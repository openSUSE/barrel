/*
 * Copyright (c) 2025 SUSE LLC
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
#include <storage/Devices/Encryption.h>
#include <storage/FreeInfo.h>

#include "Utils/GetOpts.h"
#include "Utils/Text.h"
#include "resize-device.h"


namespace barrel
{

    using namespace storage;


    namespace
    {

	const ExtOptions resize_device_options({
	    { "size", required_argument, 's', _("new size"), "size" },
	}, TakeBlkDevices::YES);


	struct Options
	{
	    Options(GetOpts& get_opts);

	    SmartSize size;

	    vector<string> blk_devices;
	};


	Options::Options(GetOpts& get_opts)
	    : size("max", true)
	{
	    ParsedOpts parsed_opts = get_opts.parse("resize", resize_device_options);

	    if (!parsed_opts.has_option("size"))
		throw OptionsException(_("Missing size option."));

	    string str = parsed_opts.get("size");
	    size = SmartSize(str, true);

	    blk_devices = parsed_opts.get_blk_devices();
	}

    }


    class ParsedCmdResizeDevice : public ParsedCmd
    {
    public:

	ParsedCmdResizeDevice(const Options& options) : options(options) {}

	virtual bool do_backup() const override { return true; }

	virtual void doit(const GlobalOptions& global_options, State& state) const override;

    private:

	const Options options;

    };


    void
    ParsedCmdResizeDevice::doit(const GlobalOptions& global_options, State& state) const
    {
	Devicegraph* staging = state.storage->get_staging();

	for (const string& name : options.blk_devices)
	{
	    BlkDevice* blk_device = BlkDevice::find_by_name(staging, name);
	    if (is_encryption(blk_device))
		blk_device = to_encryption(blk_device)->get_blk_device();

	    ResizeInfo resize_info = blk_device->detect_resize_info();
	    if (!resize_info.resize_ok)
		throw runtime_error(sformat(_("Device '%s' cannot be resized."),
					    blk_device->get_name().c_str()));

	    const unsigned long long max_size = resize_info.max_size;
	    const unsigned long long min_size = resize_info.min_size;
	    const unsigned long long old_size = blk_device->get_size();

	    unsigned long long new_size = options.size.get(max_size, old_size);

	    if (is_partition(blk_device))
	    {
		const Partition* partition = to_partition(blk_device);
		const PartitionTable* partition_table = partition->get_partition_table();

		Region region = partition->get_region();
		region.set_length(region.to_blocks(new_size));
		region = partition_table->align(region, AlignPolicy::KEEP_START_ALIGN_END);
		new_size = region.to_bytes(region.get_length());

		// The end of the existing partition can be unaligned and bigger than it
		// is after alignment.  In that case do not shrink if 'max' is requested.
		if (options.size.type == SmartSize::MAX && new_size < old_size)
		    new_size = old_size;
	    }

	    if (old_size != new_size)
	    {
		if (new_size > max_size)
		    throw runtime_error(_("New size too big."));

		if (new_size < min_size)
		    throw runtime_error(_("New size too small."));

		blk_device->set_size(new_size);
		state.modified = true;
	    }
	}
    }


    shared_ptr<ParsedCmd>
    parse_resize_device(GetOpts& get_opts)
    {
	Options options(get_opts);

	return make_shared<ParsedCmdResizeDevice>(options);
    }


    shared_ptr<ParsedCmd>
    CmdResizeDevice::parse(GetOpts& get_opts) const
    {
	Options options(get_opts);

	return make_shared<ParsedCmdResizeDevice>(options);
    }


    const char*
    CmdResizeDevice::help() const
    {
	return _("Resizes a device.");
    }


    const ExtOptions&
    CmdResizeDevice::options() const
    {
	return resize_device_options;
    }

}
