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

#include "Utils/GetOpts.h"
#include "extend-pool.h"


namespace barrel
{

    using namespace storage;


    namespace
    {

	struct Options
	{
	    Options(GetOpts& get_opts);

	    string name;

	    vector<string> blk_devices;
	};


	Options::Options(GetOpts& get_opts)
	{
	    const vector<Option> options = {
		{ "name", required_argument, 'n' }
	    };

	    ParsedOpts parsed_opts = get_opts.parse("pool", options, true);

	    name = parsed_opts.get("name");

	    blk_devices = parsed_opts.get_blk_devices();
	}

    }


    class CmdExtendPool : public Cmd
    {
    public:

	CmdExtendPool(const Options& options) : options(options) {}

	virtual void doit(State& state) const override;

    private:

	const Options options;

    };


    void
    CmdExtendPool::doit(State& state) const
    {
	Devicegraph* staging = state.storage->get_staging();

	Pool* pool = state.storage->get_pool(options.name);

	for (const string& blk_device_name : options.blk_devices)
	{
	    BlkDevice* blk_device = BlkDevice::find_by_name(staging, blk_device_name);
	    pool->add_device(blk_device);
	}

	state.modified = true;
    }


    shared_ptr<Cmd>
    parse_extend_pool(GetOpts& get_opts)
    {
	Options options(get_opts);

	return make_shared<CmdExtendPool>(options);
    }

}
