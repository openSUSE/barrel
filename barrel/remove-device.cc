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

#include "Utils/GetOpts.h"
#include "remove-device.h"


namespace barrel
{

    using namespace storage;


    namespace
    {

	struct Options
	{
	    Options(GetOpts& get_opts);

	    vector<string> blk_devices;
	};


	Options::Options(GetOpts& get_opts)
	{
	    ParsedOpts parsed_opts = get_opts.parse("remove", GetOpts::no_options, true);

	    blk_devices = parsed_opts.get_blk_devices();
	}

    }


    class CmdRemoveDevice : public Cmd
    {
    public:

	CmdRemoveDevice(const Options& options) : options(options) {}

	virtual void doit(State& state) const override;

    private:

	const Options options;

    };


    void
    CmdRemoveDevice::doit(State& state) const
    {
	Devicegraph* staging = state.storage->get_staging();

	for (const string& name : options.blk_devices)
	{
	    BlkDevice* blk_device = BlkDevice::find_by_name(staging, name);

	    if (is_md(blk_device))
	    {
		Md* md = to_md(blk_device);

		for (BlkDevice* parent : md->get_devices())
		{
		    if (is_partition(parent))
			staging->remove_device(parent);
		}
	    }

	    for (Device* descendant : blk_device->get_descendants(false, View::REMOVE))
		staging->remove_device(descendant);

	    staging->remove_device(blk_device);
	}

	state.modified = true;
    }


    shared_ptr<Cmd>
    parse_remove_device(GetOpts& get_opts)
    {
	Options options(get_opts);

	return make_shared<CmdRemoveDevice>(options);
    }

}
