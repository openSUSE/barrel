/*
 * Copyright (c) [2021-2025] SUSE LLC
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
#include <storage/Utils/HumanString.h>
#include <storage/Version.h>

#include "Utils/GetOpts.h"
#include "Utils/Text.h"
#include "Utils/BarrelTmpl.h"
#include "Utils/BarrelDefines.h"
#include "create-raid.h"


namespace barrel
{

    using namespace storage;


    namespace
    {

	const ExtOptions create_raid_options({
	    { "level", required_argument, 'l', _("set RAID level"), "level" },
	    { "name", required_argument, 'n', _("set RAID name"), "name" },
	    { "pool-name", required_argument, 0, _("name of pool to use"), "name" },
	    { "size", required_argument, 's', _("set size of RAID"), "size" },
	    { "metadata", required_argument, 'm', _("set RAID metadata version"), "metadata" },
	    { "devices", required_argument, 'd', _("set number of devices"), "number" },
	    { "chunk-size", required_argument, 0, _("set chunk size"), "chunk-size" },
	    { "no-etc-mdadm", no_argument, 0, _("do not add in /etc/mdadm.conf") },
	    { "force", no_argument, 0, _("force if block devices are in use") }
	}, TakeBlkDevices::MAYBE);


	const map<string, MdLevel> str_to_md_level = {
#if LIBSTORAGE_NG_VERSION_AT_LEAST(1, 94)
	    { "linear", MdLevel::LINEAR },
#endif
	    { "0", MdLevel::RAID0 },
	    { "stripe", MdLevel::RAID0 },
	    { "1", MdLevel::RAID1 },
	    { "mirror", MdLevel::RAID1 },
	    { "4", MdLevel::RAID4 },
	    { "5", MdLevel::RAID5 },
	    { "6", MdLevel::RAID6 },
	    { "10", MdLevel::RAID10 }
	};


	struct SmartRaidNumber
	{
	    SmartRaidNumber() = default;

	    SmartRaidNumber(const string& str);

	    void fill(unsigned int max);

	    unsigned int raid = 0;
	    unsigned int spare = 0;

	    unsigned int sum() const { return raid + spare; }
	};


	SmartRaidNumber::SmartRaidNumber(const string& str)
	{
	    static const regex raid_rx("([0-9]+)", regex::extended);
	    static const regex raid_and_spare_rx("([0-9]+)\\+([0-9]+)", regex::extended);
	    static const regex spare_rx("\\+([0-9]+)", regex::extended);

	    smatch match;

	    if (regex_match(str, match, raid_rx))
	    {
		string n1 = match[1];
		raid = atoi(n1.c_str());
		if (raid < 1)
		    throw runtime_error(sformat(_("invalid devices value '%d'"), raid));

		return;
	    }

	    if (regex_match(str, match, raid_and_spare_rx))
	    {
		string n1 = match[1];
		raid = atoi(n1.c_str());
		if (raid < 1)
		    throw runtime_error(sformat(_("invalid devices value '%d'"), raid));

		string n2 = match[2];
		spare = atoi(n2.c_str());
		if (spare < 1)
		    throw runtime_error(sformat(_("invalid devices value '%d'"), spare));

		return;
	    }

	    if (regex_match(str, match, spare_rx))
	    {
		string n1 = match[1];
		spare = atoi(n1.c_str());
		if (spare < 1)
		    throw runtime_error(sformat(_("invalid devices value '%d'"), spare));

		return;
	    }

	    // TRANSLATORS: error message, the argument for the command line option
	    // 'devices' is bad
	    throw runtime_error(_("bad devices argument"));
	}


	void
	SmartRaidNumber::fill(unsigned int max)
	{
	    if (raid == 0)
		raid = max - spare;
	}


	struct Options
	{
	    Options(GetOpts& get_opts);

	    optional<MdLevel> level;
	    optional<SmartSize> size;
	    optional<string> pool_name;
	    optional<string> name;
	    optional<SmartRaidNumber> number;
	    optional<string> metadata;
	    optional<unsigned long> chunk_size;
	    bool etc_mdadm = true;
	    bool force = false;

	    vector<string> blk_devices;

	    enum class ModusOperandi { POOL, PARTITIONABLES, BLK_DEVICES };

	    ModusOperandi modus_operandi;

	    void calculate_modus_operandi();

	    void check() const;
	};


	Options::Options(GetOpts& get_opts)
	{
	    ParsedOpts parsed_opts = get_opts.parse("raid", create_raid_options);

	    if (parsed_opts.has_option("level"))
	    {
		string str = parsed_opts.get("level");

		map<string, MdLevel>::const_iterator it = str_to_md_level.find(str);
		if (it == str_to_md_level.end())
		    throw runtime_error(_("unknown raid level for command 'raid'"));

		level = it->second;
	    }

	    name = parsed_opts.get_optional("name");

	    pool_name = parsed_opts.get_optional("pool-name");

	    if (parsed_opts.has_option("devices"))
	    {
		string str = parsed_opts.get("devices");
		number = SmartRaidNumber(str);
	    }

	    if (parsed_opts.has_option("size"))
	    {
		string str = parsed_opts.get("size");
		size = SmartSize(str);
	    }

	    metadata = parsed_opts.get_optional("metadata");

	    if (parsed_opts.has_option("chunk-size"))
	    {
		string str = parsed_opts.get("chunk-size");
		chunk_size = humanstring_to_byte(str, false);
	    }

	    etc_mdadm = !parsed_opts.has_option("no-etc-mdadm");

	    force = parsed_opts.has_option("force");

	    blk_devices = parsed_opts.get_blk_devices();

	    calculate_modus_operandi();
	}


	void
	Options::calculate_modus_operandi()
	{
	    if (pool_name)
	    {
		if (!size)
		    throw runtime_error(_("size argument required for command 'raid'"));

		modus_operandi = ModusOperandi::POOL;
	    }
	    else if (size)
	    {
		if (blk_devices.empty())
		    throw runtime_error(_("block devices missing for command 'raid'"));

		modus_operandi = ModusOperandi::PARTITIONABLES;
	    }
	    else
	    {
		if (blk_devices.empty())
		    throw runtime_error(_("block devices missing for command 'raid'"));

		modus_operandi = ModusOperandi::BLK_DEVICES;
	    }
	}


	void
	Options::check() const
	{
	    if (!level)
	    {
		throw logic_error("raid level still unknown");
	    }
	}

    }


    class ParsedCmdCreateRaid : public ParsedCmd
    {
    public:

	ParsedCmdCreateRaid(const Options& options)
	    : options(options)
	{
	    options.check();
	}

	virtual bool do_backup() const override { return true; }

	virtual void doit(const GlobalOptions& global_options, State& state) const override;

    private:

	const Options options;

    };


    namespace PartitionCreator
    {
	vector<BlkDevice*>
	create_partitions(const Pool* pool, Devicegraph* devicegraph, MdLevel md_level,
			  const SmartRaidNumber& smart_number, const SmartSize& smart_size)
	{
	    if (pool->empty(devicegraph))
		throw runtime_error(_("pool is empty"));

	    unsigned long long size = 0;

	    switch (smart_size.type)
	    {
		case SmartSize::MAX:
		    size = pool->max_partition_size(devicegraph, smart_number.sum());
		    break;

		case SmartSize::ABSOLUTE:
		    size = Md::calculate_underlying_size(md_level, smart_number.raid,
							 smart_size.value);
		    break;

		case SmartSize::PLUS:
		case SmartSize::MINUS:
		    throw logic_error("invalid SmartSize type");
	    }

	    return up_cast<BlkDevice*>(pool->create_partitions(devicegraph, smart_number.sum(), size));
	}
    };


    void
    ParsedCmdCreateRaid::doit(const GlobalOptions& global_options, State& state) const
    {
	Devicegraph* staging = state.storage->get_staging();

	string name;
	if (options.name)
	{
	    name = DEV_MD_DIR "/" + options.name.value();

	    for (const Md* md : Md::get_all(staging))
	    {
		if (md->get_name() == name)
		    throw runtime_error(_("name of RAID already exists"));
	    }
	}
	else
	{
	    name = Md::find_free_numeric_name(staging);
	}

	vector<BlkDevice*> blk_devices;

	SmartRaidNumber smart_number;

	switch (options.modus_operandi)
	{
	    case Options::ModusOperandi::POOL:
	    {
		Pool* pool = state.storage->get_pool(options.pool_name.value());

		if (options.number)
		{
		    smart_number = options.number.value();
		    smart_number.fill(pool->size(staging));
		}
		else
		{
		    smart_number.raid = pool->size(staging);
		}

		blk_devices = PartitionCreator::create_partitions(pool, staging, options.level.value(),
								  smart_number, options.size.value());
	    }
	    break;

	    case Options::ModusOperandi::PARTITIONABLES:
	    {
		Pool pool;

		for (const string& device_name : options.blk_devices)
		{
		    const Partitionable* partitionable = Partitionable::find_by_name(staging, device_name);
		    pool.add_device(partitionable);
		}

		if (options.number)
		{
		    smart_number = options.number.value();
		    smart_number.fill(pool.size(staging));
		}
		else
		{
		    smart_number.raid = pool.size(staging);
		}

		blk_devices = PartitionCreator::create_partitions(&pool, staging, options.level.value(),
								  smart_number, options.size.value());
	    }
	    break;

	    case Options::ModusOperandi::BLK_DEVICES:
	    {
		for (const string& device_name : options.blk_devices)
		{
		    BlkDevice* blk_device = BlkDevice::find_by_name(staging, device_name);
		    blk_devices.push_back(blk_device);
		}

		smart_number.raid = blk_devices.size();
	    }
	    break;
	}

	if (blk_devices.empty())
	    throw runtime_error(_("block devices for RAID missing"));

	check_usable(blk_devices, options.force);

	Md* md = Md::create(staging, name);
	md->set_metadata(options.metadata ? options.metadata.value() : "default");
	md->set_md_level(options.level.value());

	if (options.chunk_size)
	    md->set_chunk_size(options.chunk_size.value());

	md->set_in_etc_mdadm(options.etc_mdadm);

	if (smart_number.raid < md->minimal_number_of_devices())
	{
	    throw runtime_error(_("too few raid devices for raid level"));
	}

	if (smart_number.spare > 0 && !md->supports_spare_devices())
	{
	    throw runtime_error(_("spare devices not allowed for raid level"));
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


    shared_ptr<ParsedCmd>
    parse_create_raid(GetOpts& get_opts, MdLevel level)
    {
	Options options(get_opts);

	if (options.level)
	    throw OptionsException(_("RAID level already set for command 'raid'"));

	options.level = level;

	return make_shared<ParsedCmdCreateRaid>(options);
    }


    shared_ptr<ParsedCmd>
    CmdCreateRaid::parse(GetOpts& get_opts) const
    {
	Options options(get_opts);

	if (!options.level)
	    throw OptionsException(_("Raid level missing for command 'raid'"));

	return make_shared<ParsedCmdCreateRaid>(options);
    }


    const char*
    CmdCreateRaid::help() const
    {
	return _("Creates a new RAID.");
    }


    const ExtOptions&
    CmdCreateRaid::options() const
    {
	return create_raid_options;
    }


    shared_ptr<ParsedCmd>
    CmdCreateRaid0::parse(GetOpts& get_opts) const
    {
	return parse_create_raid(get_opts, MdLevel::RAID0);
    }


    const char*
    CmdCreateRaid0::help() const
    {
	return _("Alias for 'create raid --level 0'");
    }


    shared_ptr<ParsedCmd>
    CmdCreateRaid1::parse(GetOpts& get_opts) const
    {
	return parse_create_raid(get_opts, MdLevel::RAID1);
    }


    const char*
    CmdCreateRaid1::help() const
    {
	return _("Alias for 'create raid --level 1'");
    }


    shared_ptr<ParsedCmd>
    CmdCreateRaid4::parse(GetOpts& get_opts) const
    {
	return parse_create_raid(get_opts, MdLevel::RAID4);
    }


    const char*
    CmdCreateRaid4::help() const
    {
	return _("Alias for 'create raid --level 4'");
    }


    shared_ptr<ParsedCmd>
    CmdCreateRaid5::parse(GetOpts& get_opts) const
    {
	return parse_create_raid(get_opts, MdLevel::RAID5);
    }


    const char*
    CmdCreateRaid5::help() const
    {
	return _("Alias for 'create raid --level 5'");
    }


    shared_ptr<ParsedCmd>
    CmdCreateRaid6::parse(GetOpts& get_opts) const
    {
	return parse_create_raid(get_opts, MdLevel::RAID6);
    }


    const char*
    CmdCreateRaid6::help() const
    {
	return _("Alias for 'create raid --level 6'");
    }


    shared_ptr<ParsedCmd>
    CmdCreateRaid10::parse(GetOpts& get_opts) const
    {
	return parse_create_raid(get_opts, MdLevel::RAID10);
    }


    const char*
    CmdCreateRaid10::help() const
    {
	return _("Alias for 'create raid --level 10'");
    }

}
