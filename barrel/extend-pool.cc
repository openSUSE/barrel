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
#include <storage/Pool.h>
#include <storage/Devices/BlkDevice.h>

#include "Utils/GetOpts.h"
#include "Utils/Text.h"
#include "extend-pool.h"


namespace barrel
{

    using namespace storage;


    namespace
    {

	const ExtOptions extend_pool_options({
	    { "name", required_argument, 'n', _("name of pool"), "name" }
	}, TakeBlkDevices::YES);


	struct Options
	{
	    Options(GetOpts& get_opts);

	    string name;

	    vector<string> blk_devices;
	};


	Options::Options(GetOpts& get_opts)
	{
	    ParsedOpts parsed_opts = get_opts.parse("pool", extend_pool_options);

	    name = parsed_opts.get("name");

	    blk_devices = parsed_opts.get_blk_devices();
	}

    }


    class ParsedCmdExtendPool : public ParsedCmd
    {
    public:

	ParsedCmdExtendPool(const Options& options) : options(options) {}

	virtual bool do_backup() const override { return false; }

	virtual void doit(const GlobalOptions& global_options, State& state) const override;

    private:

	const Options options;

    };


    void
    ParsedCmdExtendPool::doit(const GlobalOptions& global_options, State& state) const
    {
	Devicegraph* staging = state.storage->get_staging();

	Pool* pool = state.storage->get_pool(options.name);

	for (const string& blk_device_name : options.blk_devices)
	{
	    BlkDevice* blk_device = BlkDevice::find_by_name(staging, blk_device_name);
	    pool->add_device(blk_device);

	    pimp_pool(pool, blk_device);
	}

	state.pools_modified = true;
    }


    shared_ptr<ParsedCmd>
    CmdExtendPool::parse(GetOpts& get_opts) const
    {
	Options options(get_opts);

	return make_shared<ParsedCmdExtendPool>(options);
    }


    const char*
    CmdExtendPool::help() const
    {
	return _("Adds devices to a pool.");
    }


    const ExtOptions&
    CmdExtendPool::options() const
    {
	return extend_pool_options;
    }

}
