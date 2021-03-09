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

	struct Options
	{
	    Options(GetOpts& get_opts);

	    bool show_partitions = true;
	};


	Options::Options(GetOpts& get_opts)
	{
	    const vector<Option> options = {
		{ "no-partitions", no_argument }
	    };

	    ParsedOpts parsed_opts = get_opts.parse("disks", options);

	    show_partitions = !parsed_opts.has_option("no-partitions");
	}

    }

    class CmdShowDisks : public CmdShow
    {
    public:

	CmdShowDisks(const Options& options) : options(options) {}

	virtual bool do_backup() const override { return false; }

	virtual void doit(State& state) const override;

    private:

	const Options options;

    };


    void
    CmdShowDisks::doit(State& state) const
    {
	const Devicegraph* staging = state.storage->get_staging();

	vector<const Disk*> disks = Disk::get_all(staging);
	sort(disks.begin(), disks.end(), Disk::compare_by_name);

	Table table({ Cell(_("Name"), Id::NAME), Cell(_("Size"), Id::SIZE, Align::RIGHT),
		Cell(_("Usage"), Id::USAGE), Cell(_("Pool"), Id::POOL) });

	for (const Disk* disk : disks)
	{
	    Table::Row row(table, { disk->get_name(), format_size(disk->get_size()),
		    device_usage(disk), device_pool(state.storage, disk) });

	    if (options.show_partitions)
		insert_partitions(disk, row);

	    table.add(row);
	}

	cout << table;
    }


    shared_ptr<Cmd>
    parse_show_disks(GetOpts& get_opts)
    {
	Options options(get_opts);

	return make_shared<CmdShowDisks>(options);
    }

}
