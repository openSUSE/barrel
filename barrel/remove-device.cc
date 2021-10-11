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


#include <storage/Storage.h>
#include <storage/Devicegraph.h>
#include <storage/Actiongraph.h>
#include <storage/Devices/Md.h>
#include <storage/Devices/Encryption.h>
#include <storage/FreeInfo.h>

#include "Utils/GetOpts.h"
#include "Utils/Text.h"
#include "remove-device.h"


namespace barrel
{

    using namespace storage;


    namespace
    {

	const ExtOptions remove_device_options({
	    { "keep-partitions", no_argument, 0, _("keep underlying partitions") }
	}, TakeBlkDevices::YES);


	struct Options
	{
	    Options(GetOpts& get_opts);

	    bool keep_partitions = false;

	    vector<string> blk_devices;
	};


	Options::Options(GetOpts& get_opts)
	{
	    ParsedOpts parsed_opts = get_opts.parse("remove", remove_device_options);

	    keep_partitions = parsed_opts.has_option("keep-partitions");

	    blk_devices = parsed_opts.get_blk_devices();
	}

    }


    class ParsedCmdRemoveDevice : public ParsedCmd
    {
    public:

	ParsedCmdRemoveDevice(const Options& options) : options(options) {}

	virtual bool do_backup() const override { return true; }

	virtual void doit(const GlobalOptions& global_options, State& state) const override;

    private:

	const Options options;

	static void remove_blk_device(Devicegraph* devicegraph, BlkDevice* blk_device);

    };


    void
    ParsedCmdRemoveDevice::remove_blk_device(Devicegraph* devicegraph, BlkDevice* blk_device)
    {
	// TODO this special handling should be done by the libstorage-ng

	if (is_partition(blk_device))
	{
	    Partition* partition = to_partition(blk_device);
	    PartitionTable* partition_table = partition->get_partition_table();
	    partition_table->delete_partition(partition);
	}
	else
	{
	    devicegraph->remove_device(blk_device);
	}
    }


    void
    ParsedCmdRemoveDevice::doit(const GlobalOptions& global_options, State& state) const
    {
	Devicegraph* staging = state.storage->get_staging();

	for (const string& name : options.blk_devices)
	{
	    BlkDevice* blk_device = BlkDevice::find_by_name(staging, name);

	    if (!blk_device->detect_remove_info().remove_ok)
		throw runtime_error(sformat("block device '%s' cannot be removed",
					    blk_device->get_name().c_str()));

	    vector<Partition*> partitions;

	    if (!options.keep_partitions)
	    {
		if (is_md(blk_device))
		{
		    Md* md = to_md(blk_device);

		    for (BlkDevice* tmp : md->get_devices())
			if (is_partition(tmp))
			    partitions.push_back(to_partition(tmp));
		}

		if (is_encryption(blk_device))
		{
		    Encryption* encryption = to_encryption(blk_device);

		    BlkDevice* tmp = encryption->get_blk_device();
		    if (is_partition(tmp))
			partitions.push_back(to_partition(tmp));
		}
	    }

	    for (Device* descendant : blk_device->get_descendants(false, View::REMOVE))
		staging->remove_device(descendant);

	    remove_blk_device(staging, blk_device);

	    for (Partition* partition : partitions)
	    {
		if (partition->detect_remove_info().remove_ok)
		    remove_blk_device(staging, partition);
	    }
	}

	state.modified = true;
    }


    shared_ptr<ParsedCmd>
    parse_remove_device(GetOpts& get_opts)
    {
	Options options(get_opts);

	return make_shared<ParsedCmdRemoveDevice>(options);
    }

    shared_ptr<ParsedCmd>
    CmdRemoveDevice::parse(GetOpts& get_opts) const
    {
	Options options(get_opts);

	return make_shared<ParsedCmdRemoveDevice>(options);
    }


    const char*
    CmdRemoveDevice::help() const
    {
	return _("Removes a device.");
    }


    const ExtOptions&
    CmdRemoveDevice::options() const
    {
	return remove_device_options;
    }

}
