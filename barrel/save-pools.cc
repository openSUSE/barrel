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


#include <boost/algorithm/string.hpp>

#include <storage/Storage.h>
#include <storage/Devicegraph.h>
#include <storage/Pool.h>
#include <storage/Devices/BlkDevice.h>

#include "Utils/GetOpts.h"
#include "Utils/JsonFile.h"
#include "Utils/Text.h"
#include "Utils/BarrelDefines.h"
#include "save-pools.h"


namespace barrel
{

    using namespace storage;


    class ParsedCmdSavePools : public ParsedCmd
    {
    public:

	virtual bool do_backup() const override { return false; }

	virtual void doit(const GlobalOptions& global_options, State& state) const override;

    };


    void
    ParsedCmdSavePools::doit(const GlobalOptions& global_options, State& state) const
    {
	if (global_options.verbose)
	    cout << "Saving pools..." << endl;

	JsonFile json;

	const Storage* storage = state.storage;
	const Devicegraph* staging = storage->get_staging();

	for (const map<string, const Pool*>::value_type& value : storage->get_pools())
	{
	    const Pool* pool = value.second;
	    const map<string, string>& userdata = pool->get_userdata();

	    json_object* j_pool = json_object_new_object();

	    map<string, string>::const_iterator it = userdata.find("description");
	    if (it != userdata.end())
		json_object_object_add(j_pool, "description", json_object_new_string(it->second.c_str()));

	    json_object* j_devices = json_object_new_array();

	    for (const Device* device : pool->get_devices(staging))
	    {
		if (!is_blk_device(device))
		    continue;

		const BlkDevice* blk_device = to_blk_device(device);

		map<string, string>::const_iterator it = userdata.find(sformat("sid-%d-name", blk_device->get_sid()));
		string name = it != userdata.end() ? it->second : blk_device->get_name();

		json_object_array_add(j_devices, json_object_new_string(name.c_str()));
	    }

	    for (const map<string, string>::value_type& value : userdata)
	    {
		if (boost::starts_with(value.first, "not-found-"))
		    json_object_array_add(j_devices, json_object_new_string(value.second.c_str()));
	    }

	    json_object_object_add(j_pool, "devices", j_devices);

	    json_object_object_add(json.get_root(), value.first.c_str(), j_pool);
	}

	json.save(POOLS_FILE);

	state.pools_modified = false;
    }


    shared_ptr<ParsedCmd>
    CmdSavePools::parse()
    {
	return make_shared<ParsedCmdSavePools>();
    }


    shared_ptr<ParsedCmd>
    CmdSavePools::parse(GetOpts& get_opts) const
    {
	return make_shared<ParsedCmdSavePools>();
    }


    const char*
    CmdSavePools::help() const
    {
	return _("save pools");
    }

}
