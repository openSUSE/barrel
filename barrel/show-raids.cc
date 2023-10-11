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


#include <algorithm>

#include <storage/Storage.h>
#include <storage/Devicegraph.h>
#include <storage/Devices/Md.h>
#include <storage/Holders/MdUser.h>
#include <storage/Devices/DmRaid.h>
#include <storage/Version.h>

#include "Utils/GetOpts.h"
#include "Utils/Table.h"
#include "Utils/Text.h"
#include "Utils/Misc.h"
#include "show-raids.h"
#include "show.h"


namespace barrel
{

    using namespace std;
    using namespace storage;


    namespace
    {

	const ExtOptions show_raids_options({
	    { "no-partitions", no_argument, 0, _("do not show partitions on RAIDs") },
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
	    ParsedOpts parsed_opts = get_opts.parse("raids", show_raids_options);

	    show_partitions = !parsed_opts.has_option("no-partitions");
	    show_probed = parsed_opts.has_option("probed");
	}

    }


    class ParsedCmdShowRaids : public ParsedCmdShow
    {
    public:

	ParsedCmdShowRaids(const Options& options) : options(options) {}

	virtual bool do_backup() const override { return false; }

	virtual void doit(const GlobalOptions& global_options, State& state) const override;

    private:

	const Options options;

	string devices(const Devicegraph* devicegraph, const Md* md) const;

    };


    string
    ParsedCmdShowRaids::devices(const Devicegraph* devicegraph, const Md* md) const
    {
	// TODO faulty, journal

	unsigned int raid = 0;
	unsigned int spare = 0;

	for (const BlkDevice* blk_device : md->get_blk_devices())
	{
	    const MdUser* md_user = to_md_user(devicegraph->find_holder(blk_device->get_sid(), md->get_sid()));

	    if (md_user->is_spare())
		++spare;
	    else
		++raid;
	}

	if (spare == 0)
	    return sformat("%d", raid);
	else
	    return sformat("%d+%d", raid, spare);
    }


    void
    ParsedCmdShowRaids::doit(const GlobalOptions& global_options, State& state) const
    {
	const Storage* storage = state.storage;

	const Devicegraph* devicegraph = options.show_probed ? storage->get_probed() : storage->get_staging();

	vector<const Md*> mds = Md::get_all(devicegraph);
	sort(mds.begin(), mds.end(), Md::compare_by_name);

	vector<const DmRaid*> dm_raids = DmRaid::get_all(devicegraph);
	sort(dm_raids.begin(), dm_raids.end(), DmRaid::compare_by_name);

	Table table({ Cell(_("Name"), Id::NAME), Cell(_("Size"), Id::SIZE, Align::RIGHT), _("Level"),
		_("Metadata"), Cell(_("Chunk Size"), Align::RIGHT), _("Devices"),
		Cell(_("Usage"), Id::USAGE), Cell(_("Pool"), Id::POOL) });

	for (const Md* md : mds)
	{
#if LIBSTORAGE_NG_VERSION_AT_LEAST(1, 94)
	    string t1 = md->is_chunk_size_meaningful() ? format_size(md->get_chunk_size(), true) : "";
#else
	    string t1 = md->get_md_level() != MdLevel::RAID1 ? format_size(md->get_chunk_size(), true) : "";
#endif

	    Table::Row row(table, { md->get_name(), format_size(md->get_size()),
		    get_md_level_name(md->get_md_level()), md->get_metadata(),
		    t1, devices(devicegraph, md), device_usage(md), device_pools(storage, md) });

	    if (options.show_partitions)
		insert_partitions(md, row);

	    table.add(row);
	}

	for (const DmRaid* dm_raid : dm_raids)
	{
	    Table::Row row(table, { dm_raid->get_name(), format_size(dm_raid->get_size()),
		    "", "", "", "", device_usage(dm_raid), device_pools(storage, dm_raid) });

	    if (options.show_partitions)
		insert_partitions(dm_raid, row);

	    table.add(row);
	}

	cout << table;
    }


    shared_ptr<ParsedCmd>
    CmdShowRaids::parse(GetOpts& get_opts) const
    {
	Options options(get_opts);

	return make_shared<ParsedCmdShowRaids>(options);
    }


    const char*
    CmdShowRaids::help() const
    {
	return _("Shows RAIDs.");
    }


    const ExtOptions&
    CmdShowRaids::options() const
    {
	return show_raids_options;
    }

}
