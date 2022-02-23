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


#include <algorithm>

#include <storage/Storage.h>
#include <storage/Devices/Disk.h>

#include "Utils/GetOpts.h"
#include "Utils/Table.h"
#include "Utils/Text.h"
#include "Utils/Misc.h"
#include "show-disks.h"
#include "show.h"


namespace barrel
{

    using namespace std;
    using namespace storage;


    namespace
    {

	const ExtOptions show_disks_options({
	    // TRANSLATORS: help text
	    { "no-partitions", no_argument, 0, _("do not show partitions on disks") },
	    // TRANSLATORS: help text
	    { "probed", no_argument, 0, _("use probed instead of staging devicegraph") }
	});


	struct Options
	{
	    Options(GetOpts& get_opts);

	    bool show_partitions = true;

	    bool show_probed = false;
	};


	Options::Options(GetOpts& get_opts)
	{
	    ParsedOpts parsed_opts = get_opts.parse("disks", show_disks_options);

	    show_partitions = !parsed_opts.has_option("no-partitions");

	    show_probed = parsed_opts.has_option("probed");
	}

    }


    class ParsedCmdShowDisks : public ParsedCmdShow
    {
    public:

	ParsedCmdShowDisks(const Options& options) : options(options) {}

	virtual bool do_backup() const override { return false; }

	virtual void doit(const GlobalOptions& global_options, State& state) const override;

    private:

	const Options options;

    };


    void
    ParsedCmdShowDisks::doit(const GlobalOptions& global_options, State& state) const
    {
	const Storage* storage = state.storage;

	const Devicegraph* devicegraph = options.show_probed ? storage->get_probed() : storage->get_staging();

	vector<const Disk*> disks = Disk::get_all(devicegraph);
	sort(disks.begin(), disks.end(), Disk::compare_by_name);

	Table table({ Cell(_("Name"), Id::NAME), Cell(_("Size"), Id::SIZE, Align::RIGHT),
		Cell(_("Block Size"), Align::RIGHT), Cell(_("Usage"), Id::USAGE),
		Cell(_("Pool"), Id::POOL) });

	for (const Disk* disk : disks)
	{
	    Table::Row row(table, { disk->get_name(), format_size(disk->get_size()),
		    format_size(disk->get_region().get_block_size(), true),
		    device_usage(disk), device_pools(storage, disk) });

	    if (options.show_partitions)
		insert_partitions(disk, row);

	    table.add(row);
	}

	cout << table;
    }


    shared_ptr<ParsedCmd>
    CmdShowDisks::parse(GetOpts& get_opts) const
    {
	Options options(get_opts);

	return make_shared<ParsedCmdShowDisks>(options);
    }


    const char*
    CmdShowDisks::help() const
    {
	return _("Shows disks.");
    }


    const ExtOptions&
    CmdShowDisks::options() const
    {
	return show_disks_options;
    }

}
