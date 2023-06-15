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
#include <storage/Pool.h>
#include <storage/Devices/Partitionable.h>

#include "Utils/GetOpts.h"
#include "Utils/Misc.h"
#include "Utils/Table.h"
#include "Utils/Text.h"
#include "show-pools.h"


namespace barrel
{

    using namespace std;
    using namespace storage;


    namespace
    {

	const ExtOptions show_pools_options({
	    // TRANSLATORS: help text
	    { "probed", no_argument, 0, _("use probed instead of staging devicegraph") }
	});


	struct Options
	{
	    Options(GetOpts& get_opts);

	    bool show_probed = false;
	};


	Options::Options(GetOpts& get_opts)
	{
	    ParsedOpts parsed_opts = get_opts.parse("pools", show_pools_options);

	    show_probed = parsed_opts.has_option("probed");
	}

    }


    class ParsedCmdShowPools : public ParsedCmd
    {
    public:

	ParsedCmdShowPools(const Options& options) : options(options) {}

	virtual bool do_backup() const override { return false; }

	virtual void doit(const GlobalOptions& global_options, State& state) const override;

    private:

	const Options options;

    };


    void
    ParsedCmdShowPools::doit(const GlobalOptions& global_options, State& state) const
    {
	const Storage* storage = state.storage;

	const Devicegraph* devicegraph = options.show_probed ? storage->get_probed() : storage->get_staging();

	Table table({ Cell(_("Name"), Id::NAME), Cell(_("Devices"), Id::NUMBER, Align::RIGHT),
		Cell(_("Size"), Id::SIZE, Align::RIGHT), Cell(_("Used"), Id::USED, Align::RIGHT) });

	map<string, const Pool*> pools = storage->get_pools();
	for (const map<string, const Pool*>::value_type& value : pools)
	{
	    const Pool* pool = value.second;

	    Table::Row row(table, { value.first });

	    vector<const Device*> devices = pool->get_devices(devicegraph);
	    sort(devices.begin(), devices.end(), Device::compare_by_name);

	    unsigned long long total_size = 0;
	    unsigned long long total_used = 0;

	    for (const Device* device : devices)
	    {
		Table::Row subrow(row.get_table());
		subrow[Id::NAME] = device->get_displayname();

		bool usable = false;

		if (is_partitionable(device))
		{
		    const Partitionable* partitionable = to_partitionable(device);
		    if (partitionable->has_partition_table())
		    {
			usable = true;

			const PartitionTable* partition_table = partitionable->get_partition_table();

			// TODO better sum of all slots?
			unsigned long long size = partitionable->get_size();
			unsigned long long used = 0;

			for (const Partition* partition : partition_table->get_partitions())
			{
			    PartitionType partition_type = partition->get_type();

			    if (partition_type == PartitionType::PRIMARY || partition_type == PartitionType::LOGICAL)
				used += partition->get_size();
			}

			subrow[Id::SIZE] = format_size(size);
			subrow[Id::USED] = format_percentage(used, size);

			total_size += size;
			total_used += used;
		    }
		}

		if (!usable)
		    subrow[Id::NAME] += " !";

		row.add_subrow(subrow);
	    }

	    row[Id::NUMBER] = sformat("%zu", devices.size());
	    row[Id::SIZE] = format_size(total_size);
	    row[Id::USED] = format_percentage(total_used, total_size);

	    table.add(row);
	}

	cout << table;
    }


    shared_ptr<ParsedCmd>
    CmdShowPools::parse(GetOpts& get_opts) const
    {
	Options options(get_opts);

	return make_shared<ParsedCmdShowPools>(options);
    }


    const char*
    CmdShowPools::help() const
    {
	return _("Shows pools.");
    }


    const ExtOptions&
    CmdShowPools::options() const
    {
	return show_pools_options;
    }

}
