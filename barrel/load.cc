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
#include <storage/Devices/Disk.h>
#include <storage/Holders/Holder.h>

#include "Utils/GetOpts.h"
#include "Utils/JsonFile.h"
#include "Utils/BarrelDefines.h"
#include "Utils/Text.h"
#include "load.h"


namespace barrel
{

    using namespace storage;


    namespace
    {

	struct Options
	{
	    Options(GetOpts& get_opts);

	    string devicegraph;
	    string mapping;
	};


	Options::Options(GetOpts& get_opts)
	{
	    const vector<Option> options = {
		{ "devicegraph", required_argument, 'f' },
		{ "mapping", required_argument, 'm' }
	    };

	    ParsedOpts parsed_opts = get_opts.parse("load", options, true);

	    if (parsed_opts.has_option("devicegraph"))
		devicegraph = parsed_opts.get("devicegraph");
	    else
		throw OptionsException("devicegraph missing");

	    if (parsed_opts.has_option("mapping"))
		mapping = parsed_opts.get("mapping");
	    else
		throw OptionsException("mapping missing");
	}

    }


    class CmdLoad : public Cmd
    {
    public:

	CmdLoad(const Options& options) : options(options) {}

	virtual bool do_backup() const override { return true; }

	virtual void doit(const GlobalOptions& global_options, State& state) const override;

    private:

	const Options options;

	map<string, string> load_mapping() const;

	const Disk* map_disk(const GlobalOptions& global_options, const Devicegraph* probed,
			     const map<string, string>& mapping, const Disk* a) const;

    };


    map<string, string>
    CmdLoad::load_mapping() const
    {
	JsonFile json_file(options.mapping);

	map<string, string> mapping;

	if (!get_child_value(json_file.get_root(), "mapping", mapping))
	    throw runtime_error(sformat("mapping not found in json file '%s'", options.mapping.c_str()));

	return mapping;
    }


    const Disk*
    CmdLoad::map_disk(const GlobalOptions& global_options, const Devicegraph* probed,
		      const map<string, string>& mapping, const Disk* a) const
    {
	map<string, string>::const_iterator it = mapping.find(a->get_name());

	if (it == mapping.end())
	{
	    for (const string& udev_path : a->get_udev_paths())
	    {
		it = mapping.find(DEV_DISK_BY_PATH_DIR "/" + udev_path);
		if (it != mapping.end())
		    break;
	    }
	}

	if (it == mapping.end())
	{
	    for (const string& udev_id : a->get_udev_ids())
	    {
		it = mapping.find(DEV_DISK_BY_ID_DIR "/" + udev_id);
		if (it != mapping.end())
		    break;
	    }
	}

	if (it == mapping.end())
	    throw Exception("disk not found in mapping");

	const BlkDevice* b = BlkDevice::find_by_any_name(probed, it->second);

	if (!is_disk(b))
	    throw Exception("mapped device is not a disk");

	if (global_options.verbose)
	{
	    cout << "mapping " << a->get_name() << " (" << it->first << ") to " << b->get_name()
		 << " (" << it->second << ")\n";
	}

	if (a->get_region().get_block_size() != b->get_region().get_block_size())
	    throw Exception("mapped disk has different block size");

	if (a->get_region().get_length() > b->get_region().get_length())
	    throw Exception("mapped disk is smaller");

	return to_disk(b);
    }


    void
    CmdLoad::doit(const GlobalOptions& global_options, State& state) const
    {
	const Devicegraph* probed = state.storage->get_probed();
	Devicegraph* staging = state.storage->get_staging();

	staging->load(options.devicegraph, false);

	map<string, string> mapping = load_mapping();

	for (Disk* a : staging->get_all_disks())
	{
	    const Disk* b = map_disk(global_options, probed, mapping, a);

	    Device* c = b->copy_to_devicegraph(staging);

	    for (Holder* holder : a->get_out_holders())
		holder->set_source(c);

	    staging->remove_device(a);

	    // TODO must anything dependent be update? e.g. name, sysfs_name, sysfs_path
	    // of partitions, topology, ...
	}

	// TODO clear uuids
	// TODO what to do with subvolumes/snapshots of snapper?

	state.modified = true;
    }


    shared_ptr<Cmd>
    parse_load(GetOpts& get_opts)
    {
	Options options(get_opts);

	return make_shared<CmdLoad>(options);
    }

}
