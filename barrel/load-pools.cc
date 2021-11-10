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
#include <storage/Devices/Partitionable.h>

#include "Utils/GetOpts.h"
#include "Utils/JsonFile.h"
#include "Utils/Text.h"
#include "Utils/BarrelDefines.h"
#include "load-pools.h"


namespace barrel
{

    using namespace storage;


    class ParsedCmdLoadPools : public ParsedCmd
    {
    public:

	virtual bool do_backup() const override { return false; }

	virtual void doit(const GlobalOptions& global_options, State& state) const override;

    };


    void
    ParsedCmdLoadPools::doit(const GlobalOptions& global_options, State& state) const
    {
	if (global_options.verbose)
	    cout << _("Loading pools...") << endl;

	Storage* storage = state.storage;

	JsonFile json(POOLS_FILE);

	remove_pools(storage);

	const Devicegraph* probed = storage->get_probed();

	SystemInfo system_info;

	json_object_object_foreach(json.get_root(), k, j_pool)
	{
	    Pool* pool = storage->create_pool(k);
	    map<string, string> userdata;

	    json_object* j_description = json_object_object_get(j_pool, "description");
	    if (j_description)
	    {
		if (!json_object_is_type(j_description, json_type_string))
		    throw runtime_error(_("value of description entry not a string"));

		userdata["description"] = json_object_get_string(j_description);
	    }

	    json_object* j_devices = json_object_object_get(j_pool, "devices");
	    if (j_devices)
	    {
		if (!json_object_is_type(j_devices, json_type_array))
		    throw runtime_error(_("value of devices entry not an array"));

		for (size_t i = 0; i < json_object_array_length(j_devices); ++i)
		{
		    json_object* j_name = json_object_array_get_idx(j_devices, i);
		    if (!json_object_is_type(j_name, json_type_string))
			throw runtime_error(sformat(_("element of devices array for '%s' not string"), k));

		    string name = json_object_get_string(j_name);

		    if (BlkDevice::exists_by_any_name(probed, name, system_info))
		    {
			const BlkDevice* blk_device = BlkDevice::find_by_any_name(probed, name, system_info);
			pool->add_device(blk_device);

			userdata[sformat("sid-%d-name", blk_device->get_sid())] = name;
		    }
		    else
		    {
			userdata[sformat("not-found-%zd", i)] = name;
		    }
		}
	    }

	    pool->set_userdata(userdata);
	}

	state.pools_modified = false;
    }


    shared_ptr<ParsedCmd>
    CmdLoadPools::parse()
    {
	return make_shared<ParsedCmdLoadPools>();
    }


    shared_ptr<ParsedCmd>
    CmdLoadPools::parse(GetOpts& get_opts) const
    {
	return make_shared<ParsedCmdLoadPools>();
    }


    const char*
    CmdLoadPools::help() const
    {
	return _("Loads pools.");
    }

}
