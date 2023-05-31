/*
 * Copyright (c) [2021-2023] SUSE LLC
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
#include <storage/Devices/Partition.h>
#include <storage/Devices/Dasd.h>
#include <storage/Devices/Multipath.h>
#include <storage/Devices/DmRaid.h>
#include <storage/Holders/Holder.h>
#include <storage/SystemInfo/SystemInfo.h>

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

	const ExtOptions load_devicegraph_options({
	    { "name", required_argument, 'n', _("name of devicegraph file"), "name" },
	    { "mapping", required_argument, 'm', _("name of mapping file"), "name" }
	});


	struct Options
	{
	    Options(GetOpts& get_opts);

	    string name;
	    optional<string> mapping;
	};


	Options::Options(GetOpts& get_opts)
	{
	    ParsedOpts parsed_opts = get_opts.parse("devicegraph", load_devicegraph_options);

	    if (!parsed_opts.has_option("name"))
		throw OptionsException(_("name missing for command 'devicegraph'"));

	    name = parsed_opts.get("name");

	    if (parsed_opts.has_option("mapping"))
		mapping = parsed_opts.get("mapping");
	}

    }


    class ParsedCmdLoadDevicegraph : public ParsedCmd
    {
    public:

	ParsedCmdLoadDevicegraph(const Options& options) : options(options) {}

	virtual bool do_backup() const override { return true; }

	virtual void doit(const GlobalOptions& global_options, State& state) const override;

    private:

	const Options options;

	using mapping_t = map<string, vector<string>>;

	mapping_t load_mapping() const;

	/**
	 * Make an automatic mapping for disks and DASDs. Either the udev path or id must be
	 * identical. For multipath and DM RAID the name must be identical.
	 */
	mapping_t make_mapping(const Devicegraph* probed, const Devicegraph* staging) const;

	/**
	 * Return true iff the block devices have a common udev path or id.
	 */
	bool same_udev(const BlkDevice* a, const BlkDevice* b) const;

	const BlkDevice* map_blk_device(const GlobalOptions& global_options, const Devicegraph* probed,
					SystemInfo& system_info, const mapping_t& mapping,
					const BlkDevice* a) const;

	const Disk* map_disk(const GlobalOptions& global_options, const Devicegraph* probed,
			     SystemInfo& system_info, const mapping_t& mapping, const Disk* a) const;

	const Dasd* map_dasd(const GlobalOptions& global_options, const Devicegraph* probed,
			     SystemInfo& system_info, const mapping_t& mapping, const Dasd* a) const;

	const Multipath* map_multipath(const GlobalOptions& global_options, const Devicegraph* probed,
				       SystemInfo& system_info, const mapping_t& mapping, const Multipath* a) const;

	const DmRaid* map_dm_raid(const GlobalOptions& global_options, const Devicegraph* probed,
				  SystemInfo& system_info, const mapping_t& mapping, const DmRaid* a) const;

	bool skip_disk(const Disk* a) const;

	void copy_to_staging(Devicegraph* staging, BlkDevice* a, const BlkDevice* b) const;

    };


    ParsedCmdLoadDevicegraph::mapping_t
    ParsedCmdLoadDevicegraph::load_mapping() const
    {
	const string& filename = options.mapping.value();

	JsonFile json_file(filename);

	json_object* tmp1;
	json_object_object_get_ex(json_file.get_root(), "mapping", &tmp1);
	if (!json_object_is_type(tmp1, json_type_object))
	    throw runtime_error(sformat(_("mapping not found in json file '%s'"), filename.c_str()));

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
			throw runtime_error(sformat(_("element of array for '%s' not string"), k));

		    mapping[k].push_back(json_object_get_string(tmp2));
		}
	    }
	    else
	    {
		throw runtime_error(sformat(_("value for '%s' neither string nor array"), k));
	    }
	}

	return mapping;
    }


    bool
    ParsedCmdLoadDevicegraph::same_udev(const BlkDevice* a, const BlkDevice* b) const
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


    ParsedCmdLoadDevicegraph::mapping_t
    ParsedCmdLoadDevicegraph::make_mapping(const Devicegraph* probed, const Devicegraph* staging) const
    {
	mapping_t mapping;

	for (const Disk* a : Disk::get_all(staging))
	{
	    vector<const Disk*> matches;

	    for (const Disk* b : Disk::get_all(probed))
	    {
		if (same_udev(a, b))
		    matches.push_back(b);
	    }

	    if (matches.size() == 0)
		throw runtime_error(sformat(_("no mapping disk for '%s' found"), a->get_name().c_str()));

	    if (matches.size() >= 2)
		throw runtime_error(sformat(_("several mapping disks for '%s' found"), a->get_name().c_str()));

	    mapping[a->get_name()] = { matches.front()->get_name() };
	}

	for (const Dasd* a : Dasd::get_all(staging))
	{
	    vector<const Dasd*> matches;

	    for (const Dasd* b : Dasd::get_all(probed))
	    {
		if (same_udev(a, b))
		    matches.push_back(b);
	    }

	    if (matches.size() == 0)
		throw runtime_error(sformat(_("no mapping DASD for '%s' found"), a->get_name().c_str()));

	    if (matches.size() >= 2)
		throw runtime_error(sformat(_("several mapping DASDs for '%s' found"), a->get_name().c_str()));

	    mapping[a->get_name()] = { matches.front()->get_name() };
	}

	for (const Multipath* a : Multipath::get_all(staging))
	{
	    vector<const Multipath*> matches;

	    for (const Multipath* b : Multipath::get_all(probed))
	    {
		if (a->get_name() == b->get_name())
		    matches.push_back(b);
	    }

	    if (matches.size() == 0)
		throw runtime_error(sformat(_("no mapping multipath for '%s' found"), a->get_name().c_str()));

	    if (matches.size() >= 2)
		throw runtime_error(sformat(_("several mapping multipath for '%s' found"), a->get_name().c_str()));

	    mapping[a->get_name()] = { matches.front()->get_name() };
	}

	for (const DmRaid* a : DmRaid::get_all(staging))
	{
	    vector<const DmRaid*> matches;

	    for (const DmRaid* b : DmRaid::get_all(probed))
	    {
		if (a->get_name() == b->get_name())
		    matches.push_back(b);
	    }

	    if (matches.size() == 0)
		throw runtime_error(sformat(_("no mapping DM RAID for '%s' found"), a->get_name().c_str()));

	    if (matches.size() >= 2)
		throw runtime_error(sformat(_("several mapping DM RAIDs for '%s' found"), a->get_name().c_str()));

	    mapping[a->get_name()] = { matches.front()->get_name() };
	}

	return mapping;
    }


    const BlkDevice*
    ParsedCmdLoadDevicegraph::map_blk_device(const GlobalOptions& global_options, const Devicegraph* probed,
					     SystemInfo& system_info, const mapping_t& mapping,
					     const BlkDevice* a) const
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
	    throw runtime_error(sformat(_("device '%s' not found in mapping"), name.c_str()));

	const BlkDevice* b = nullptr;

	for (const string& tmp : it->second)
	{
	    if (!BlkDevice::exists_by_any_name(probed, tmp, system_info))
		continue;

	    b = BlkDevice::find_by_any_name(probed, tmp, system_info);
	    break;
	}

	if (!b)
	    throw runtime_error(sformat(_("mapped device for '%s' not found"), name.c_str()));

	if (global_options.verbose)
	    cout << sformat(_("mapping %s to %s"), a->get_name().c_str(), b->get_name().c_str()) << '\n';

	if (a->get_region().get_block_size() != b->get_region().get_block_size())
	    throw runtime_error(sformat(_("mapped device for '%s' has different block size"), a->get_name().c_str()));

	if (a->get_region().get_length() > b->get_region().get_length())
	    throw runtime_error(sformat(_("mapped device for '%s' is smaller"), a->get_name().c_str()));

	return b;
    }


    const Disk*
    ParsedCmdLoadDevicegraph::map_disk(const GlobalOptions& global_options, const Devicegraph* probed,
				       SystemInfo& system_info, const mapping_t& mapping, const Disk* a) const
    {
	const BlkDevice* b = map_blk_device(global_options, probed, system_info, mapping, a);

	if (!is_disk(b))
	    throw runtime_error(sformat(_("mapped device for '%s' is not a disk"), a->get_name().c_str()));

	return to_disk(b);
    }


    const Dasd*
    ParsedCmdLoadDevicegraph::map_dasd(const GlobalOptions& global_options, const Devicegraph* probed,
				       SystemInfo& system_info, const mapping_t& mapping, const Dasd* a) const
    {
	const BlkDevice* b = map_blk_device(global_options, probed, system_info, mapping, a);

	if (!is_dasd(b))
	    throw runtime_error(sformat(_("mapped device for '%s' is not a DASD"), a->get_name().c_str()));

	const Dasd* dasd = to_dasd(b);

	if (a->get_type() != dasd->get_type())
	    throw runtime_error(sformat(_("mapped DASD for '%s' has different type"), a->get_name().c_str()));

	if (a->get_format() != dasd->get_format())
	    throw runtime_error(sformat(_("mapped DASD for '%s' has different format"), a->get_name().c_str()));

	return dasd;
    }


    const Multipath*
    ParsedCmdLoadDevicegraph::map_multipath(const GlobalOptions& global_options, const Devicegraph* probed,
					    SystemInfo& system_info, const mapping_t& mapping, const Multipath* a) const
    {
	const BlkDevice* b = map_blk_device(global_options, probed, system_info, mapping, a);

	if (!is_multipath(b))
	    throw runtime_error(sformat(_("mapped device for '%s' is not a multipath"), a->get_name().c_str()));

	return to_multipath(b);
    }


    const DmRaid*
    ParsedCmdLoadDevicegraph::map_dm_raid(const GlobalOptions& global_options, const Devicegraph* probed,
					  SystemInfo& system_info, const mapping_t& mapping, const DmRaid* a) const
    {
	const BlkDevice* b = map_blk_device(global_options, probed, system_info, mapping, a);

	if (!is_dm_raid(b))
	    throw runtime_error(sformat(_("mapped device for '%s' is not a DM RAID"), a->get_name().c_str()));

	return to_dm_raid(b);
    }


    bool
    ParsedCmdLoadDevicegraph::skip_disk(const Disk* a) const
    {
	for (const Device* child : a->get_children())
	{
	    if (is_multipath(child) || is_dm_raid(child))
		return true;
	}

	return false;
    }


    void
    ParsedCmdLoadDevicegraph::copy_to_staging(Devicegraph* staging, BlkDevice* a, const BlkDevice* b) const
    {
	// copy device to staging and attach children

	Device* c = b->copy_to_devicegraph(staging);

	for (Holder* holder : a->get_out_holders())
	    holder->set_source(c);

	// copy parents with holders to staging (e.g. for multipath)

	for (const Holder* holder : b->get_in_holders())
	{
	    holder->get_source()->copy_to_devicegraph(staging);
	    holder->copy_to_devicegraph(staging);
	}

	// remove old (loaded) device and old (loaded) parents from staging

	for (Holder* holder : a->get_in_holders())
	    staging->remove_device(holder->get_source());

	staging->remove_device(a);
    }


    void
    ParsedCmdLoadDevicegraph::doit(const GlobalOptions& global_options, State& state) const
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

	for (Disk* a : Disk::get_all(staging))
	{
	    if (skip_disk(a))
		continue;

	    const Disk* b = map_disk(global_options, probed, system_info, mapping, a);

	    if (b->exists_in_devicegraph(staging))
		throw runtime_error(sformat(_("mapped disk for '%s' mapped twice"), a->get_name().c_str()));

	    copy_to_staging(staging, a, b);
	}

	for (Dasd* a : Dasd::get_all(staging))
	{
	    const Dasd* b = map_dasd(global_options, probed, system_info, mapping, a);

	    if (b->exists_in_devicegraph(staging))
		throw runtime_error(sformat(_("mapped DASD for '%s' mapped twice"), a->get_name().c_str()));

	    copy_to_staging(staging, a, b);
	}

	for (Multipath* a : Multipath::get_all(staging))
	{
	    const Multipath* b = map_multipath(global_options, probed, system_info, mapping, a);

	    if (b->exists_in_devicegraph(staging))
		throw runtime_error(sformat(_("mapped multipath for '%s' mapped twice"), a->get_name().c_str()));

	    copy_to_staging(staging, a, b);
	}

	for (DmRaid* a : DmRaid::get_all(staging))
	{
	    const DmRaid* b = map_dm_raid(global_options, probed, system_info, mapping, a);

	    if (b->exists_in_devicegraph(staging))
		throw runtime_error(sformat(_("mapped DM RAID for '%s' mapped twice"), a->get_name().c_str()));

	    copy_to_staging(staging, a, b);
	}

	for (Partitionable* partitionable : Partitionable::get_all(staging))
	{
	    if (!partitionable->has_partition_table())
		continue;

	    PartitionTable* partition_table = partitionable->get_partition_table();

	    for (Partition* partition : partition_table->get_partitions())
	    {
		unsigned int id = partition->get_id();

		if (id == ID_UNKNOWN || !partition_table->is_partition_id_supported(id))
		{
		    cout << sformat(_("Setting id of partition %s from unsupported value to %s."),
				    partition->get_name().c_str(), get_partition_id_name(ID_LINUX).c_str()) << '\n';
		    partition->set_id(ID_LINUX);
		}
	    }
	}

	// TODO must anything dependent be update? libstorage-ng already takes are of some
	// stuff

	// TODO clear uuids
	// TODO what to do with subvolumes/snapshots of snapper?

	state.modified = true;
    }


    shared_ptr<ParsedCmd>
    CmdLoadDevicegraph::parse(GetOpts& get_opts) const
    {
	Options options(get_opts);

	return make_shared<ParsedCmdLoadDevicegraph>(options);
    }


    const char*
    CmdLoadDevicegraph::help() const
    {
	return _("Loads the staging devicegraph from a file.");
    }


    const ExtOptions&
    CmdLoadDevicegraph::options() const
    {
	return load_devicegraph_options;
    }

}
