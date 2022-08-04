/*
 * Copyright (c) [2021-2022] SUSE LLC
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


#include <storage/Storage.h>
#include <storage/Pool.h>
#include <storage/Devices/BlkDevice.h>
#include <storage/Devices/LvmVg.h>
#include <storage/Devices/Partitionable.h>
#include <storage/Utils/HumanString.h>

#include "Utils/GetOpts.h"
#include "Utils/BarrelTmpl.h"
#include "Utils/Text.h"
#include "create-lvm-vg.h"


namespace barrel
{

    using namespace storage;


    namespace
    {

	const ExtOptions create_lvm_vg_options({
	    { "name", required_argument, 'n', _("set name of volume group"), "name" },
	    { "pool-name", required_argument, 0, _("name of pool to use"), "name" },
	    { "size", required_argument, 's', _("set size of volume group"), "size" },
	    { "devices", required_argument, 'd', _("set number of devices"), "number" },
	    { "extent-size", required_argument, 0, _("set extent size"), "extent-size" },
	    { "force", no_argument, 0, _("force if block devices are in use") }
	}, TakeBlkDevices::MAYBE);


	struct Options
	{
	    Options(GetOpts& get_opts);

	    string vg_name;
	    optional<SmartSize> size;
	    optional<string> pool_name;
	    optional<SmartNumber> number;
	    optional<unsigned long long> extent_size;
	    bool force = false;

	    vector<string> blk_devices;

	    enum class ModusOperandi { POOL, PARTITIONABLES, BLK_DEVICES, BLK_DEVICES_FROM_STACK,
		PARTITION_TABLES_FROM_STACK };

	    ModusOperandi modus_operandi;

	    void calculate_modus_operandi();
	};


	Options::Options(GetOpts& get_opts)
	{
	    ParsedOpts parsed_opts = get_opts.parse("vg", create_lvm_vg_options);

	    if (!parsed_opts.has_option("name"))
		throw OptionsException(_("name missing for command 'vg'"));

	    vg_name = parsed_opts.get("name");

	    if (!LvmVg::is_valid_vg_name(vg_name))
		throw OptionsException(_("invalid volume group name for command 'vg'"));

	    pool_name = parsed_opts.get_optional("pool-name");

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

	    force = parsed_opts.has_option("force");

	    blk_devices = parsed_opts.get_blk_devices();

	    calculate_modus_operandi();
	}


	void
	Options::calculate_modus_operandi()
	{
	    // TODO identical in create-filesystem.cc

	    if (pool_name)
	    {
		if (!size)
		    throw runtime_error(_("size argument required for command 'vg'"));

		if (!blk_devices.empty())
		    throw runtime_error(_("pool argument and blk devices not allowed together for command 'vg'"));

		modus_operandi = ModusOperandi::POOL;
	    }
	    else
	    {
		if (size)
		{
		    if (blk_devices.empty())
			modus_operandi = ModusOperandi::PARTITION_TABLES_FROM_STACK;
		    else
			modus_operandi = ModusOperandi::PARTITIONABLES;
		}
		else
		{
		    if (blk_devices.empty())
			modus_operandi = ModusOperandi::BLK_DEVICES_FROM_STACK;
		    else
			modus_operandi = ModusOperandi::BLK_DEVICES;
		}
	    }
	}

    }


    class ParsedCmdCreateLvmVg : public ParsedCmd
    {
    public:

	ParsedCmdCreateLvmVg(const Options& options) : options(options) {}

	virtual bool do_backup() const override { return true; }

	virtual void doit(const GlobalOptions& global_options, State& state) const override;

    private:

	const Options options;

    };


    void
    ParsedCmdCreateLvmVg::doit(const GlobalOptions& global_options, State& state) const
    {
	Devicegraph* staging = state.storage->get_staging();

	for (const LvmVg* lvm_vg : LvmVg::get_all(staging))
	{
	    if (lvm_vg->get_vg_name() == options.vg_name)
		throw runtime_error(_("name of volume group already exists"));
	}

	vector<BlkDevice*> blk_devices;

	switch (options.modus_operandi)
	{
	    case Options::ModusOperandi::POOL:
	    {
		Pool* pool = state.storage->get_pool(options.pool_name.value());

		blk_devices = PartitionCreator::create_partitions(pool, staging, PartitionCreator::ONE,
								  options.number, options.size.value());
	    }
	    break;

	    case Options::ModusOperandi::PARTITION_TABLES_FROM_STACK:
	    {
		vector<PartitionTable*> partition_tables = state.stack.top_as_partition_tables(staging);

		Pool pool;

		for (const PartitionTable* partition_table : partition_tables)
		    pool.add_device(partition_table->get_partitionable());

		blk_devices = PartitionCreator::create_partitions(&pool, staging, PartitionCreator::POOL_SIZE,
								  options.number, options.size.value());

		state.stack.pop();
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

		blk_devices = PartitionCreator::create_partitions(&pool, staging, PartitionCreator::POOL_SIZE,
								  options.number, options.size.value());
	    }
	    break;

	    case Options::ModusOperandi::BLK_DEVICES:
	    {
		for (const string& device_name : options.blk_devices)
		{
		    BlkDevice* blk_device = BlkDevice::find_by_name(staging, device_name);
		    blk_devices.push_back(blk_device);
		}
	    }
	    break;

	    case Options::ModusOperandi::BLK_DEVICES_FROM_STACK:
	    {
		blk_devices = state.stack.top_as_blk_devices(staging);
		state.stack.pop();
	    }
	    break;
	}

	if (blk_devices.empty())
	    throw runtime_error(_("block devices for LVM volume group missing"));

	check_usable(blk_devices, options.force);

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


    shared_ptr<ParsedCmd>
    CmdCreateLvmVg::parse(GetOpts& get_opts) const
    {
	Options options(get_opts);

	return make_shared<ParsedCmdCreateLvmVg>(options);
    }


    const char*
    CmdCreateLvmVg::help() const
    {
	return _("Creates a new LVM volume group.");
    }


    const ExtOptions&
    CmdCreateLvmVg::options() const
    {
	return create_lvm_vg_options;
    }

}
