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
#include "load-devicegraph.h"


namespace barrel
{

    using namespace storage;


    namespace
    {

	struct Options
	{
	    Options(GetOpts& get_opts);

	    string name;
	    optional<string> mapping;
	};


	Options::Options(GetOpts& get_opts)
	{
	    const vector<Option> options = {
		{ "name", required_argument, 'n' },
		{ "mapping", required_argument, 'm' }
	    };

	    ParsedOpts parsed_opts = get_opts.parse("load", options, true);

	    if (parsed_opts.has_option("name"))
		name = parsed_opts.get("name");
	    else
		throw OptionsException("name missing");

	    if (parsed_opts.has_option("mapping"))
		mapping = parsed_opts.get("mapping");
	}

    }


    class CmdLoadDevicegraph : public Cmd
    {
    public:

	CmdLoadDevicegraph(const Options& options) : options(options) {}

	virtual bool do_backup() const override { return true; }

	virtual void doit(const GlobalOptions& global_options, State& state) const override;

    private:

	const Options options;

	using mapping_t = map<string, vector<string>>;

	mapping_t load_mapping() const;

	/**
	 * Make an automatic mapping for disks. Either the udev path or id must be
	 * identical.
	 */
	mapping_t make_mapping(const Devicegraph* probed, const Devicegraph* staging) const;

	/**
	 * Return true iff the block devices have a common udev path or id.
	 */
	bool same_udev(const BlkDevice* a, const BlkDevice* b) const;

	const Disk* map_disk(const GlobalOptions& global_options, const Devicegraph* probed,
			     SystemInfo& system_info, const mapping_t& mapping, const Disk* a) const;

    };


    CmdLoadDevicegraph::mapping_t
    CmdLoadDevicegraph::load_mapping() const
    {
	const string& filename = options.mapping.value();

	JsonFile json_file(filename);

	json_object* tmp1;
	json_object_object_get_ex(json_file.get_root(), "mapping", &tmp1);
	if (!json_object_is_type(tmp1, json_type_object))
	    throw runtime_error(sformat("mapping not found in json file '%s'", filename.c_str()));

	mapping_t mapping;

	json_object_object_foreach(tmp1, k, v)
	{
	    // check for duplicate key not possible since already removed by json parser

	    if (json_object_is_type(v, json_type_string))
	    {
		mapping[k] = { json_object_get_string(v) };
	    }
	    else if (json_object_is_type(v, json_type_array))
	    {
		for (size_t i = 0; i < json_object_array_length(v); ++i)
		{
		    json_object* tmp2 = json_object_array_get_idx(v, i);
		    if (!json_object_is_type(tmp2, json_type_string))
			throw runtime_error(sformat("element of array for '%s' not string", k));

		    mapping[k].push_back(json_object_get_string(tmp2));
		}
	    }
	    else
	    {
		throw runtime_error(sformat("value for '%s' neither string nor array", k));
	    }
	}

	return mapping;
    }


    bool
    CmdLoadDevicegraph::same_udev(const BlkDevice* a, const BlkDevice* b) const
    {
	// could use something like set_intersection but needs sorting

	for (const string& udev_path_a : a->get_udev_paths())
	    for (const string& udev_path_b : b->get_udev_paths())
		if (udev_path_a == udev_path_b)
		    return true;

	for (const string& udev_id_a : a->get_udev_ids())
	    for (const string& udev_id_b : b->get_udev_ids())
		if (udev_id_a == udev_id_b)
		    return true;

	return false;
    }


    CmdLoadDevicegraph::mapping_t
    CmdLoadDevicegraph::make_mapping(const Devicegraph* probed, const Devicegraph* staging) const
    {
	mapping_t mapping;

	for (const Disk* a : staging->get_all_disks())
	{
	    vector<const Disk*> matches;

	    for (const Disk* b : probed->get_all_disks())
	    {
		if (same_udev(a, b))
		    matches.push_back(b);
	    }

	    if (matches.size() == 0)
		throw runtime_error(sformat("no mapping disk for '%s' found", a->get_name().c_str()));

	    if (matches.size() >= 2)
		throw runtime_error(sformat("several mapping disk for '%s' found", a->get_name().c_str()));

	    mapping[a->get_name()] = { matches.front()->get_name() };
	}

	return mapping;
    }


    const Disk*
    CmdLoadDevicegraph::map_disk(const GlobalOptions& global_options, const Devicegraph* probed,
				 SystemInfo& system_info, const mapping_t& mapping, const Disk* a) const
    {
	const string name = a->get_name();

	mapping_t::const_iterator it = mapping.find(name);

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
	    throw runtime_error(sformat("disk '%s' not found in mapping", name.c_str()));

	const BlkDevice* b = nullptr;

	for (const string& tmp : it->second)
	{
	    if (!BlkDevice::exists_by_any_name(probed, tmp, system_info))
		continue;

	    b = BlkDevice::find_by_any_name(probed, tmp, system_info);
	    break;
	}

	if (!b)
	    throw runtime_error(sformat("mapped disk for '%s' not found", name.c_str()));

	if (!is_disk(b))
	    throw runtime_error(sformat("mapped device for '%s' is not a disk", name.c_str()));

	if (global_options.verbose)
	{
	    cout << "mapping " << name << " to " << b->get_name() << '\n';
	}

	if (a->get_region().get_block_size() != b->get_region().get_block_size())
	    throw runtime_error(sformat("mapped disk for '%s' has different block size", name.c_str()));

	if (a->get_region().get_length() > b->get_region().get_length())
	    throw runtime_error(sformat("mapped disk for '%s' is smaller", name.c_str()));

	return to_disk(b);
    }


    void
    CmdLoadDevicegraph::doit(const GlobalOptions& global_options, State& state) const
    {
	const Devicegraph* probed = state.storage->get_probed();
	Devicegraph* staging = state.storage->get_staging();

	staging->load(options.name, false);

	const mapping_t mapping = options.mapping ? load_mapping() : make_mapping(probed, staging);

	if (global_options.verbose)
	{
	    for (const mapping_t::value_type& v : mapping)
	    {
		cout << v.first << " = [ ";
		for (const string& x : v.second)
		    cout << x << " ";
		cout << "]\n";
	    }
	}

	SystemInfo system_info;

	for (Disk* a : staging->get_all_disks())
	{
	    const Disk* b = map_disk(global_options, probed, system_info, mapping, a);

	    if (b->exists_in_devicegraph(staging))
	    {
		throw runtime_error(sformat("mapped disk for '%s' mapped twice", a->get_name().c_str()));
	    }

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
    parse_load_devicegraph(GetOpts& get_opts)
    {
	Options options(get_opts);

	return make_shared<CmdLoadDevicegraph>(options);
    }

}
