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
#include <storage/Pool.h>
#include <storage/Devices/BlkDevice.h>

#include "Utils/GetOpts.h"
#include "Utils/JsonFile.h"
#include "Utils/Text.h"
#include "create-pools.h"


namespace barrel
{

    using namespace storage;


    class ParsedCmdCreatePools : public ParsedCmd
    {
    public:

	virtual bool do_backup() const override { return false; }

	virtual void doit(const GlobalOptions& global_options, State& state) const override;

    };


    void
    ParsedCmdCreatePools::doit(const GlobalOptions& global_options, State& state) const
    {
	if (global_options.verbose)
	    cout << "Generating pools" << endl;

	Storage* storage = state.storage;
	const Devicegraph* devicegraph = storage->get_staging();

	storage->generate_pools(devicegraph);

	for (map<string, Pool*>::value_type& value : storage->get_pools())
	{
	    Pool* pool = value.second;

	    for (const Device* device : pool->get_devices(devicegraph))
	    {
		if (!is_blk_device(device))
		    continue;

		const BlkDevice* blk_device = to_blk_device(device);

		pimp_pool(pool, blk_device);
	    }
	}

	state.pools_modified = true;
    }


    shared_ptr<ParsedCmd>
    parse_create_pools()
    {
	return make_shared<ParsedCmdCreatePools>();
    }

}
