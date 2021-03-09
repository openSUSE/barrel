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
#include <storage/Devicegraph.h>
#include <storage/Devices/Md.h>
#include <storage/Holders/MdUser.h>

#include "Utils/GetOpts.h"
#include "Utils/Table.h"
#include "Utils/Text.h"
#include "Utils/Misc.h"
#include "show-pools.h"
#include "show.h"


namespace barrel
{

    using namespace std;
    using namespace storage;


    class CmdShowRaids : public CmdShow
    {
    public:

	virtual bool do_backup() const override { return false; }

	virtual void doit(State& state) const override;

    private:

	string devices(const Devicegraph* devicegraph, const Md* md) const;

    };


    string
    CmdShowRaids::devices(const Devicegraph* devicegraph, const Md* md) const
    {
	// TODO faulty, journal

	unsigned int raid = 0;
	unsigned int spare = 0;

	for (const BlkDevice* blk_device : md->get_devices())
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
    CmdShowRaids::doit(State& state) const
    {
	// TODO show pool if all underlying devices are in the same pool
	// TODO show underlying devices

	const Devicegraph* staging = state.storage->get_staging();

	vector<const Md*> mds = Md::get_all(staging);
	sort(mds.begin(), mds.end(), Md::compare_by_name);

	Table table({ Cell(_("Name"), Id::NAME), Cell(_("Size"), Id::SIZE, Align::RIGHT), _("Level"),
		_("Metadata"), _("Devices"), Cell(_("Usage"), Id::USAGE) });

	for (const Md* md : mds)
	{
	    Table::Row row(table, { md->get_name(), format_size(md->get_size()),
		    get_md_level_name(md->get_md_level()), md->get_metadata(), devices(staging, md),
		    device_usage(md) });

	    insert_partitions(md, row);

	    table.add(row);
	}

	cout << table;
    }


    shared_ptr<Cmd>
    parse_show_raids(GetOpts& get_opts)
    {
	get_opts.parse("raids", GetOpts::no_options);

	return make_shared<CmdShowRaids>();
    }

}
