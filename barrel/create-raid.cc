/*
 * Copyright (c) 2021 SUSE LLC
 *
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, contact SUSE LLC.
 *
 * To contact SUSE LLC about this file by physical or electronic mail, you may
 * find current contact information at www.suse.com.
 */


#include <regex>

#include <storage/Storage.h>
#include <storage/Pool.h>
#include <storage/Devices/BlkDevice.h>
#include <storage/Devices/Md.h>
#include <storage/Holders/MdUser.h>

#include "Utils/GetOpts.h"
#include "Utils/Misc.h"
#include "create-raid.h"


namespace barrel
{

    using namespace storage;


    namespace
    {

	const map<string, MdLevel> str_to_md_level = {
	    { "0", MdLevel::RAID0 },
	    { "strip", MdLevel::RAID0 },
	    { "1", MdLevel::RAID1 },
	    { "mirror", MdLevel::RAID1 },
	    { "4", MdLevel::RAID4 },
	    { "5", MdLevel::RAID5 },
	    { "6", MdLevel::RAID6 },
	    { "10", MdLevel::RAID10 }
	};


	struct SmartNumber
	{
	    SmartNumber() = default;

	    SmartNumber(const string& str);

	    unsigned int raid = 0;
	    unsigned int spare = 0;

	    unsigned int sum() const { return raid + spare; }
	};


	SmartNumber::SmartNumber(const string& str)
	{
	    static const regex raid_rx("([0-9]+)", regex::extended);
	    static const regex raid_and_spare_rx("([0-9]+)\\+([0-9]+)", regex::extended);
	    static const regex spare_rx("\\+([0-9]+)", regex::extended);

	    smatch match;

	    if (regex_match(str, match, raid_rx))
	    {
		string n1 = match[1];
		raid = atoi(n1.c_str());
		return;
	    }

	    if (regex_match(str, match, raid_and_spare_rx))
	    {
		string n1 = match[1];
		string n2 = match[2];
		raid = atoi(n1.c_str());
		spare = atoi(n2.c_str());
		return;
	    }

	    if (regex_match(str, match, spare_rx))
	    {
		string n1 = match[1];
		spare = atoi(n1.c_str());
		return;
	    }

	    throw runtime_error("bad devices argument");
	}


	struct Options
	{
	    Options(GetOpts& get_opts);

	    optional<MdLevel> level;
	    optional<SmartSize> size;
	    optional<string> pool;
	    optional<string> name;
	    optional<SmartNumber> number;
	    optional<string> metadata;

	    optional<vector<string>> blk_devices;

	    enum class ModusOperandi { POOL, PARTITIONABLES, RAW };

	    ModusOperandi modus_operandi;

	    void calculate_modus_operandi();
	};


	Options::Options(GetOpts& get_opts)
	{
	    const vector<Option> options = {
		{ "level", required_argument, 'l' },
		{ "name", required_argument, 'n' },
		{ "pool", required_argument, 'p' },
		{ "size", required_argument, 's' },
		{ "metadata", required_argument, 'm' },
		{ "devices", required_argument, 'd' }
	    };

	    ParsedOpts parsed_opts = get_opts.parse("raid", options, true);

	    if (parsed_opts.has_option("level"))
	    {
		string str = parsed_opts.get("level");

		map<string, MdLevel>::const_iterator it = str_to_md_level.find(str);
		if (it == str_to_md_level.end())
		    throw runtime_error("unknown raid level");

		level = it->second;
	    }

	    name = parsed_opts.get_optional("name");

	    pool = parsed_opts.get_optional("pool");

	    if (parsed_opts.has_option("devices"))
	    {
		string str = parsed_opts.get("devices");
		number = SmartNumber(str);
	    }

	    if (parsed_opts.has_option("size"))
	    {
		string str = parsed_opts.get("size");
		size = SmartSize(str);
	    }

	    metadata = parsed_opts.get_optional("metadata");

	    blk_devices = parsed_opts.get_blk_devices();


	    for (string x : parsed_opts.get_blk_devices())
		cout << x << endl;


	    calculate_modus_operandi();
	}


	void
	Options::calculate_modus_operandi()
	{
	    if (pool)
	    {
		if (!size)
		    throw runtime_error("size argument required");

		modus_operandi = ModusOperandi::POOL;
	    }
	    else if (size)
	    {
		if (!blk_devices)
		    throw runtime_error("devices missing");

		modus_operandi = ModusOperandi::PARTITIONABLES;
	    }
	    else
	    {
		if (!blk_devices)
		    throw runtime_error("devices missing");

		modus_operandi = ModusOperandi::RAW;
	    }
	}

    }


    class CmdCreateRaid : public Cmd
    {
    public:

	CmdCreateRaid(const Options& options) : options(options) {}

	virtual void doit(State& state) const override;

    private:

	const Options options;

    };


    void
    CmdCreateRaid::doit(State& state) const
    {
	Devicegraph* staging = state.storage->get_staging();

	string name;
	if (options.name)
	    name = "/dev/md/" + options.name.value();
	else
	    name = Md::find_free_numeric_name(staging);

	vector<BlkDevice*> blk_devices;

	SmartNumber smart_number;

	switch (options.modus_operandi)
	{
	    case Options::ModusOperandi::POOL:
	    {
		Pool* pool = state.storage->get_pool(options.pool.value());

		if (options.number)
		    smart_number = options.number.value();
		else
		    smart_number.raid = pool->size(staging);

		SmartSize smart_size = options.size.value();

		unsigned long long size = 0;

		switch (smart_size.type)
		{
		    case SmartSize::MAX:
			size = smart_size.value(pool->max_partition_size(staging, smart_number.sum()));
			break;

		    case SmartSize::ABSOLUTE:
			size = Md::calculate_underlying_size(options.level.value(), smart_number.raid,
							     smart_size.absolute);
			break;
		}

		blk_devices = up_cast<BlkDevice*>(pool->create_partitions(staging, smart_number.sum(), size));
	    }
	    break;

	    case Options::ModusOperandi::PARTITIONABLES:
	    {
		Pool pool;

		for (const string& device_name : options.blk_devices.value())
		{
		    Partitionable* partitionable = Partitionable::find_by_name(staging, device_name);
		    pool.add_device(partitionable);
		}

		if (options.number)
		    smart_number = options.number.value();
		else
		    smart_number.raid = pool.size(staging);

		SmartSize smart_size = options.size.value();

		unsigned long long size = 0;

		switch (smart_size.type)
		{
		    case SmartSize::MAX:
			size = smart_size.value(pool.max_partition_size(staging, smart_number.sum()));
			break;

		    case SmartSize::ABSOLUTE:
			size = Md::calculate_underlying_size(options.level.value(), smart_number.raid,
							     smart_size.absolute);
			break;
		}

		blk_devices = up_cast<BlkDevice*>(pool.create_partitions(staging, pool.size(staging), size));
	    }
	    break;

	    case Options::ModusOperandi::RAW:
	    {
		for (const string& device_name : options.blk_devices.value())
		{
		    BlkDevice* blk_device = BlkDevice::find_by_name(staging, device_name);
		    blk_devices.push_back(blk_device);
		}

		smart_number.raid = blk_devices.size();
	    }
	    break;
	}

	Md* md = Md::create(staging, name);
	md->set_md_level(options.level.value());

	if (options.metadata)
	    md->set_metadata(options.metadata.value());

	if (smart_number.raid < md->minimal_number_of_devices())
	{
	    throw runtime_error("too few raid devices for raid level");
	}

	if (smart_number.spare > 0 && !md->supports_spare_devices())
	{
	    throw runtime_error("spare devices not allowed for raid level");
	}

	unsigned int cnt = 0;

	// TODO somehow allow to specify which devices are spares

	for (BlkDevice* blk_device : blk_devices)
	{
	    MdUser* md_user = md->add_device(blk_device);
	    md_user->set_spare(smart_number.raid < ++cnt);

	    if (is_partition(blk_device))
	    {
		Partition* partition = to_partition(blk_device);
		partition->set_id(ID_RAID);
	    }
	}

	state.stack.push(md);
	state.modified = true;
    }


    shared_ptr<Cmd>
    parse_create_raid(GetOpts& get_opts)
    {
	Options options(get_opts);

	if (!options.level)
	    throw OptionsException("raid level missing");

	return make_shared<CmdCreateRaid>(options);
    }


    shared_ptr<Cmd>
    parse_create_raid(GetOpts& get_opts, MdLevel level)
    {
	Options options(get_opts);

	if (options.level)
	    throw OptionsException("raid level already set");

	options.level = level;

	return make_shared<CmdCreateRaid>(options);
    }


    shared_ptr<Cmd>
    parse_create_raid0(GetOpts& get_opts)
    {
	return parse_create_raid(get_opts, MdLevel::RAID0);
    }


    shared_ptr<Cmd>
    parse_create_raid1(GetOpts& get_opts)
    {
	return parse_create_raid(get_opts, MdLevel::RAID1);
    }


    shared_ptr<Cmd>
    parse_create_raid4(GetOpts& get_opts)
    {
	return parse_create_raid(get_opts, MdLevel::RAID4);
    }


    shared_ptr<Cmd>
    parse_create_raid5(GetOpts& get_opts)
    {
	return parse_create_raid(get_opts, MdLevel::RAID5);
    }


    shared_ptr<Cmd>
    parse_create_raid6(GetOpts& get_opts)
    {
	return parse_create_raid(get_opts, MdLevel::RAID6);
    }


    shared_ptr<Cmd>
    parse_create_raid10(GetOpts& get_opts)
    {
	return parse_create_raid(get_opts, MdLevel::RAID10);
    }

}
