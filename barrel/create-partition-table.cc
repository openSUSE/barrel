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

	const vector<Option> create_partition_table_options = {
	    { "type", required_argument, 't', _("partition table type"), "type" }
	};


	const map<string, PtType> str_to_pt_type = {
	    { "gpt", PtType::GPT },
	    { "ms-dos", PtType::MSDOS }
	};


	struct Options
	{
	    Options(GetOpts& get_opts);

	    optional<PtType> type;
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

	if (state.stack.empty())
	    throw runtime_error("empty stack for create partition table");

	Partitionable* partitionable = to_partitionable(state.stack.top(staging));
	state.stack.pop();

	PtType pt_type = options.type.value();

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
	return _("Create a partition table");
    }


    const vector<Option>&
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
