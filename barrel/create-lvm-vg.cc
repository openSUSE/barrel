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
#include <storage/Devices/LvmVg.h>
#include <storage/Devices/Partitionable.h>
#include <storage/Utils/HumanString.h>

#include "Utils/GetOpts.h"
#include "Utils/Misc.h"
#include "create-lvm-vg.h"


namespace barrel
{

    using namespace storage;


    namespace
    {

	struct SmartNumber
	{
	    enum Type { MAX, ABSOLUTE };

	    SmartNumber(const string& str);

	    unsigned int value(unsigned int max) const;

	    Type type = ABSOLUTE;

	    unsigned int absolute = 1;
	};


	SmartNumber::SmartNumber(const string& str)
	{
	    static const regex absolute_rx("([0-9]+)", regex::extended);

	    if (str == "max")
	    {
		type = MAX;
		return;
	    }

	    smatch match;

	    if (regex_match(str, match, absolute_rx))
	    {
		type = ABSOLUTE;

		string n1 = match[1];
		absolute = atoi(n1.c_str());
		return;
	    }

	    throw runtime_error("bad devices argument");
	}


	unsigned int
	SmartNumber::value(unsigned int max) const
	{
	    switch (type)
	    {
		case SmartNumber::MAX:
		    return max;

		case SmartNumber::ABSOLUTE:
		    return absolute;

		default:
		    throw runtime_error("unknown SmartNumber type");
	    }
	}


	struct Options
	{
	    Options(GetOpts& get_opts);

	    string vg_name;
	    optional<SmartSize> size;
	    optional<string> pool;
	    optional<SmartNumber> number;
	    optional<unsigned long long> extent_size;

	    vector<string> blk_devices;

	    enum class ModusOperandi { POOL, PARTITIONABLES, RAW };

	    ModusOperandi modus_operandi;

	    void calculate_modus_operandi();
	};


	Options::Options(GetOpts& get_opts)
	{
	    const vector<Option> options = {
		{ "name", required_argument, 'n' },
		{ "pool", required_argument, 'p' },
		{ "size", required_argument, 's' },
		{ "devices", required_argument, 'd' },
		{ "extent-size", required_argument }
	    };

	    ParsedOpts parsed_opts = get_opts.parse("vg", options, true);

	    if (!parsed_opts.has_option("name"))
		throw OptionsException("name missing for command 'vg'");

	    vg_name = parsed_opts.get("name");

	    if (!LvmVg::is_valid_vg_name(vg_name))
		throw OptionsException("invalid volume group name for command 'vg'");

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

	    if (parsed_opts.has_option("extent-size"))
	    {
		string str = parsed_opts.get("extent-size");
		extent_size = humanstring_to_byte(str, false);
	    }

	    blk_devices = parsed_opts.get_blk_devices();

	    calculate_modus_operandi();
	}


	void
	Options::calculate_modus_operandi()
	{
	    if (pool)
	    {
		if (!size)
		    throw runtime_error("size argument required for command 'vg'");

		modus_operandi = ModusOperandi::POOL;
	    }
	    else if (size)
	    {
		if (blk_devices.empty())
		    throw runtime_error("block devices missing for command 'vg'");

		modus_operandi = ModusOperandi::PARTITIONABLES;
	    }
	    else
	    {
		if (blk_devices.empty())
		    throw runtime_error("block devices missing for command 'vg'");

		modus_operandi = ModusOperandi::RAW;
	    }
	}

    }


    class CmdCreateLvmVg : public Cmd
    {
    public:

	CmdCreateLvmVg(const Options& options) : options(options) {}

	virtual bool do_backup() const override { return true; }

	virtual void doit(const GlobalOptions& global_options, State& state) const override;

    private:

	const Options options;

    };


    void
    CmdCreateLvmVg::doit(const GlobalOptions& global_options, State& state) const
    {
	Devicegraph* staging = state.storage->get_staging();

	for (const LvmVg* lvm_vg : LvmVg::get_all(staging))
	{
	    if (lvm_vg->get_vg_name() == options.vg_name)
		throw runtime_error("name of volume group already exists");
	}

	vector<BlkDevice*> blk_devices;

	switch (options.modus_operandi)
	{
	    case Options::ModusOperandi::POOL:
	    {
		Pool* pool = state.storage->get_pool(options.pool.value());

		unsigned int number = 1;

		if (options.number)
		    number = options.number->value(pool->size(staging));

		SmartSize smart_size = options.size.value();

		unsigned long long size = 0;

		switch (smart_size.type)
		{
		    case SmartSize::MAX:
			size = smart_size.value(pool->max_partition_size(staging, number));
			break;

		    case SmartSize::ABSOLUTE:
			size = smart_size.absolute / number + 1 * MiB;
			break;
		}

		blk_devices = up_cast<BlkDevice*>(pool->create_partitions(staging, number, size));
	    }
	    break;

	    case Options::ModusOperandi::PARTITIONABLES:
	    {
		Pool pool;

		for (const string& device_name : options.blk_devices)
		{
		    Partitionable* partitionable = Partitionable::find_by_name(staging, device_name);
		    pool.add_device(partitionable);
		}

		unsigned int number = pool.size(staging);

		SmartSize smart_size = options.size.value();

		unsigned long long size = 0;

		switch (smart_size.type)
		{
		    case SmartSize::MAX:
			size = smart_size.value(pool.max_partition_size(staging, number));
			break;

		    case SmartSize::ABSOLUTE:
			size = smart_size.absolute / number + 1 * MiB;
			break;
		}

		blk_devices = up_cast<BlkDevice*>(pool.create_partitions(staging, pool.size(staging), size));
	    }
	    break;

	    case Options::ModusOperandi::RAW:
	    {
		for (const string& device_name : options.blk_devices)
		{
		    BlkDevice* blk_device = BlkDevice::find_by_name(staging, device_name);
		    blk_devices.push_back(blk_device);
		}
	    }
	    break;
	}

	LvmVg* lvm_vg = LvmVg::create(staging, options.vg_name);

	if (options.extent_size)
	    lvm_vg->set_extent_size(options.extent_size.value());

	for (BlkDevice* blk_device : blk_devices)
	{
	    lvm_vg->add_lvm_pv(blk_device);

	    if (is_partition(blk_device))
	    {
		Partition* partition = to_partition(blk_device);
		partition->set_id(ID_LVM);
	    }
	}

	state.stack.push(lvm_vg);
	state.modified = true;
    }


    shared_ptr<Cmd>
    parse_create_lvm_vg(GetOpts& get_opts)
    {
	Options options(get_opts);

	return make_shared<CmdCreateLvmVg>(options);
    }

}
