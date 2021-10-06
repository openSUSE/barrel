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
#include <storage/Devices/Dasd.h>

#include "Utils/GetOpts.h"
#include "Utils/Table.h"
#include "Utils/Text.h"
#include "Utils/Misc.h"
#include "show-dasds.h"
#include "show.h"


namespace barrel
{

    using namespace std;
    using namespace storage;


    namespace
    {

	const ExtOptions show_dasds_options({
	    { "no-partitions", no_argument, 0, _("do not show partitions on DASDs") },
	    { "probed", no_argument, 0, _("probed instead of staging") }
	});


	struct Options
	{
	    Options(GetOpts& get_opts);

	    bool show_partitions = true;

	    bool show_probed = false;
	};


	Options::Options(GetOpts& get_opts)
	{
	    ParsedOpts parsed_opts = get_opts.parse("dasds", show_dasds_options);

	    show_partitions = !parsed_opts.has_option("no-partitions");

	    show_probed = parsed_opts.has_option("probed");
	}

    }


    class ParsedCmdShowDasds : public ParsedCmdShow
    {
    public:

	ParsedCmdShowDasds(const Options& options) : options(options) {}

	virtual bool do_backup() const override { return false; }

	virtual void doit(const GlobalOptions& global_options, State& state) const override;

    private:

	const Options options;

    };


    void
    ParsedCmdShowDasds::doit(const GlobalOptions& global_options, State& state) const
    {
	const Storage* storage = state.storage;

	const Devicegraph* devicegraph = options.show_probed ? storage->get_probed() : storage->get_staging();

	vector<const Dasd*> dasds = Dasd::get_all(devicegraph);
	sort(dasds.begin(), dasds.end(), Dasd::compare_by_name);

	Table table({ Cell(_("Name"), Id::NAME), Cell(_("Size"), Id::SIZE, Align::RIGHT),
		Cell(_("Block Size"), Align::RIGHT), Cell(_("Bus ID"), Align::RIGHT),
		Cell(_("Type")), Cell(_("Format")), Cell(_("Usage"), Id::USAGE),
		Cell(_("Pool"), Id::POOL) });

	for (const Dasd* dasd : dasds)
	{
	    Table::Row row(table, { dasd->get_name(), format_size(dasd->get_size()),
		    format_size(dasd->get_region().get_block_size(), true), dasd->get_bus_id(),
		    get_dasd_type_name(dasd->get_type()),
		    dasd->get_type() == DasdType::ECKD ? get_dasd_format_name(dasd->get_format()) : "",
		    device_usage(dasd), device_pools(storage, dasd) });

	    if (options.show_partitions)
		insert_partitions(dasd, row);

	    table.add(row);
	}

	cout << table;
    }


    shared_ptr<ParsedCmd>
    CmdShowDasds::parse(GetOpts& get_opts) const
    {
	Options options(get_opts);

	return make_shared<ParsedCmdShowDasds>(options);
    }


    const char*
    CmdShowDasds::help() const
    {
	return _("Shows DASDs.");
    }


    const ExtOptions&
    CmdShowDasds::options() const
    {
	return show_dasds_options;
    }

}
