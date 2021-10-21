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
#include <storage/Pool.h>
#include <storage/Devices/Partitionable.h>
#include <storage/Devices/PartitionTable.h>

#include "Utils/GetOpts.h"
#include "Utils/Misc.h"
#include "Utils/Text.h"
#include "create-partition-table.h"


namespace barrel
{

    using namespace storage;


    namespace
    {

	const ExtOptions create_partition_table_options({
	    { "type", required_argument, 't', _("partition table type"), "type" },
	    { "force", no_argument, 0, _("force if block device is in use") }
	}, TakeBlkDevices::MAYBE);


	const map<string, PtType> str_to_pt_type = {
	    { "gpt", PtType::GPT },
	    { "ms-dos", PtType::MSDOS }
	};


	struct Options
	{
	    Options(GetOpts& get_opts);

	    optional<PtType> type;
	    bool force = false;

	    vector<string> blk_devices;

	    enum class ModusOperandi { PARTITIONABLE, PARTITIONABLE_FROM_STACK };

	    ModusOperandi modus_operandi;

	    void calculate_modus_operandi();
	};


	Options::Options(GetOpts& get_opts)
	{
	    ParsedOpts parsed_opts = get_opts.parse("partition-table", create_partition_table_options);

	    if (parsed_opts.has_option("type"))
	    {
		string str = parsed_opts.get("type");

		map<string, PtType>::const_iterator it = str_to_pt_type.find(str);
		if (it == str_to_pt_type.end())
		    throw runtime_error("unknown partition table type");

		type = it->second;
	    }

	    force = parsed_opts.has_option("force");

	    blk_devices = parsed_opts.get_blk_devices();

	    calculate_modus_operandi();
	}


	void
	Options::calculate_modus_operandi()
	{
	    if (blk_devices.empty())
		modus_operandi = ModusOperandi::PARTITIONABLE_FROM_STACK;
	    else
		modus_operandi = ModusOperandi::PARTITIONABLE;
	}

    }


    class ParsedCmdCreatePartitionTable : public ParsedCmd
    {
    public:

	ParsedCmdCreatePartitionTable(const Options& options) : options(options) {}

	virtual bool do_backup() const override { return true; }

	virtual void doit(const GlobalOptions& global_options, State& state) const override;

    private:

	const Options options;

    };


    void
    ParsedCmdCreatePartitionTable::doit(const GlobalOptions& global_options, State& state) const
    {
	Devicegraph* staging = state.storage->get_staging();

	PtType pt_type = options.type.value();

	Partitionable* partitionable = nullptr;

	switch (options.modus_operandi)
	{
	    case Options::ModusOperandi::PARTITIONABLE_FROM_STACK:
	    {
		if (state.stack.empty() || !is_partitionable(state.stack.top(staging)))
		    throw runtime_error("not a partitionable on stack");

		partitionable = to_partitionable(state.stack.top(staging));
		state.stack.pop();
	    }
	    break;

	    case Options::ModusOperandi::PARTITIONABLE:
	    {
		if (options.blk_devices.size() != 1)
		    throw runtime_error("only one block device allowed");

		BlkDevice* blk_device = BlkDevice::find_by_name(staging, options.blk_devices.front());

		if (!is_partitionable(blk_device))
		    throw runtime_error("not a partitionable");

		partitionable = to_partitionable(blk_device);
	    }
	    break;
	}

	if (partitionable->has_children())
	{
	    if (options.force)
	    {
		partitionable->remove_descendants(View::REMOVE);
	    }
	    else
	    {
		throw runtime_error(sformat("partitionable '%s' is in use", partitionable->get_name().c_str()));
	    }
	}

	if (!partitionable->is_usable_as_partitionable())
	{
	    throw runtime_error(sformat("partitionable '%s' cannot be used as a regular partitionable",
					partitionable->get_name().c_str()));
	}

	PartitionTable* partition_table = partitionable->create_partition_table(pt_type);

	state.stack.push(partition_table);
	state.modified = true;
    }


    shared_ptr<ParsedCmd>
    parse_create_partition_table(GetOpts& get_opts, PtType type)
    {
	Options options(get_opts);

	if (options.type)
	    throw OptionsException("partition table type already set");

	options.type = type;

	return make_shared<ParsedCmdCreatePartitionTable>(options);
    }


    shared_ptr<ParsedCmd>
    CmdCreatePartitionTable::parse(GetOpts& get_opts) const
    {
	Options options(get_opts);

	if (!options.type)
	    throw OptionsException("partition table type missing");

	return make_shared<ParsedCmdCreatePartitionTable>(options);
    }


    const char*
    CmdCreatePartitionTable::help() const
    {
	return _("Creates a new partition table.");
    }


    const ExtOptions&
    CmdCreatePartitionTable::options() const
    {
	return create_partition_table_options;
    }


    shared_ptr<ParsedCmd>
    CmdCreateGpt::parse(GetOpts& get_opts) const
    {
	return parse_create_partition_table(get_opts, PtType::GPT);
    }


    const char*
    CmdCreateGpt::help() const
    {
	return _("Alias for 'create partition-table --type gpt'");
    }


    shared_ptr<ParsedCmd>
    CmdCreateMsdos::parse(GetOpts& get_opts) const
    {
	return parse_create_partition_table(get_opts, PtType::MSDOS);
    }


    const char*
    CmdCreateMsdos::help() const
    {
	return _("Alias for 'create partition-table --type ms-dos'");
    }

}
