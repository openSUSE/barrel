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
#include <storage/Devices/LvmVg.h>

#include "Utils/GetOpts.h"
#include "Utils/Table.h"
#include "Utils/Text.h"
#include "Utils/Misc.h"
#include "show-lvm-vgs.h"
#include "show.h"


namespace barrel
{

    using namespace std;
    using namespace storage;


    namespace
    {

	const ExtOptions show_lvm_vgs_options({
	    { "probed", no_argument, 0, _("probed instead of staging") }
	});


	struct Options
	{
	    Options(GetOpts& get_opts);

	    bool show_probed = false;
	};


	Options::Options(GetOpts& get_opts)
	{

	    ParsedOpts parsed_opts = get_opts.parse("vgs", show_lvm_vgs_options);

	    show_probed = parsed_opts.has_option("probed");
	}

    }


    class ParsedCmdShowLvmVgs : public ParsedCmdShow
    {
    public:

	ParsedCmdShowLvmVgs(const Options& options) : options(options) {}

	virtual bool do_backup() const override { return false; }

	virtual void doit(const GlobalOptions& global_options, State& state) const override;

    private:

	const Options options;

	void insert_lvm_lvs(const LvmVg* lvm_vg, Table::Row& row) const;
	void insert_lvm_lvs(const LvmLv* lvm_lv, Table::Row& row) const;

    };


    void
    ParsedCmdShowLvmVgs::insert_lvm_lvs(const LvmVg* lvm_vg, Table::Row& row) const
    {
	vector<const LvmLv*> lvm_lvs = lvm_vg->get_lvm_lvs();
	sort(lvm_lvs.begin(), lvm_lvs.end(), LvmLv::compare_by_name);

	for (const LvmLv* lvm_lv : lvm_lvs)
	{
	    Table::Row subrow(row.get_table());

	    subrow[Id::NAME] = lvm_lv->get_lv_name();
	    subrow[Id::SIZE] = format_size(lvm_lv->get_size());

	    subrow[Id::STRIPES] = sformat("%u", lvm_lv->get_stripes());
	    if (lvm_lv->get_stripes() > 1)
		subrow[Id::STRIPES] += " (" + format_size(lvm_lv->get_stripe_size(), true) + ")";

	    subrow[Id::USAGE] = device_usage(lvm_lv);

	    insert_lvm_lvs(lvm_lv, subrow);

	    row.add_subrow(subrow);
	}
    }


    void
    ParsedCmdShowLvmVgs::insert_lvm_lvs(const LvmLv* lvm_lv, Table::Row& row) const
    {
	vector<const LvmLv*> sub_lvm_lvs = lvm_lv->get_lvm_lvs();
	sort(sub_lvm_lvs.begin(), sub_lvm_lvs.end(), LvmLv::compare_by_name);

	for (const LvmLv* sub_lvm_lv : sub_lvm_lvs)
	{
	    Table::Row subrow(row.get_table());

	    subrow[Id::NAME] = sub_lvm_lv->get_lv_name();
	    subrow[Id::SIZE] = format_size(sub_lvm_lv->get_size());

	    subrow[Id::USAGE] = device_usage(sub_lvm_lv);

	    row.add_subrow(subrow);
	}
    }


    void
    ParsedCmdShowLvmVgs::doit(const GlobalOptions& global_options, State& state) const
    {
	// TODO show pool if all underlying devices are in the same pool?
	// TODO show underlying devices

	const Storage* storage = state.storage;

	const Devicegraph* devicegraph = options.show_probed ? storage->get_probed() : storage->get_staging();

	vector<const LvmVg*> lvm_vgs = LvmVg::get_all(devicegraph);
	sort(lvm_vgs.begin(), lvm_vgs.end(), LvmVg::compare_by_name);

	Table table({ Cell(_("Name"), Id::NAME), Cell(_("Extent Size"), Align::RIGHT),
		_("Devices"), Cell(_("Size"), Id::SIZE, Align::RIGHT),
		Cell(_("Used"), Id::USED, Align::RIGHT), Cell(_("Stripes"), Id::STRIPES),
		Cell(_("Usage"), Id::USAGE) });

	for (const LvmVg* lvm_vg : lvm_vgs)
	{
	    Table::Row row(table, { lvm_vg->get_vg_name(), format_size(lvm_vg->get_extent_size(), true),
		    sformat("%lu", lvm_vg->get_lvm_pvs().size()), format_size(lvm_vg->get_size()) });

	    unsigned long long total_size = lvm_vg->number_of_extents();
	    unsigned long long total_used = lvm_vg->number_of_used_extents();
	    row[Id::USED] = format_percentage(total_used, total_size);

	    insert_lvm_lvs(lvm_vg, row);

	    table.add(row);
	}

	cout << table;
    }


    shared_ptr<ParsedCmd>
    CmdShowLvmVgs::parse(GetOpts& get_opts) const
    {
	Options options(get_opts);

	return make_shared<ParsedCmdShowLvmVgs>(options);
    }


    const char*
    CmdShowLvmVgs::help() const
    {
	return _("Shows LVM volume groups.");
    }


    const ExtOptions&
    CmdShowLvmVgs::options() const
    {
	return show_lvm_vgs_options;
    }

}
