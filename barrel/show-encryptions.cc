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
#include <storage/Devices/Encryption.h>

#include "Utils/GetOpts.h"
#include "Utils/Table.h"
#include "Utils/Text.h"
#include "Utils/Misc.h"
#include "show-encryptions.h"
#include "show.h"


namespace barrel
{

    using namespace std;
    using namespace storage;


    namespace
    {

	const ExtOptions show_encryptions_options({
	    { "probed", no_argument, 0, _("probed instead of staging") }
	});


	struct Options
	{
	    Options(GetOpts& get_opts);

	    bool show_probed = false;
	};


	Options::Options(GetOpts& get_opts)
	{
	    ParsedOpts parsed_opts = get_opts.parse("encryptions", show_encryptions_options);

	    show_probed = parsed_opts.has_option("probed");
	}

    }


    class ParsedCmdShowEncryptions : public ParsedCmdShow
    {
    public:

	ParsedCmdShowEncryptions(const Options& options) : options(options) {}

	virtual bool do_backup() const override { return false; }

	virtual void doit(const GlobalOptions& global_options, State& state) const override;

    private:

	const Options options;

	string devices(const Devicegraph* devicegraph, const Md* md) const;

    };


    void
    ParsedCmdShowEncryptions::doit(const GlobalOptions& global_options, State& state) const
    {
	const Storage* storage = state.storage;

	const Devicegraph* devicegraph = options.show_probed ? storage->get_probed() : storage->get_staging();

	vector<const Encryption*> encryptions = Encryption::get_all(devicegraph);
	sort(encryptions.begin(), encryptions.end(), Encryption::compare_by_dm_table_name);

	Table table({ Cell(_("Name"), Id::NAME), Cell(_("Size"), Id::SIZE, Align::RIGHT), _("Type"),
		_("Underlying Device"), Cell(_("Usage"), Id::USAGE) });

	for (const Encryption* encryption : encryptions)
	{
	    Table::Row row(table, { encryption->get_dm_table_name(), format_size(encryption->get_size()),
		    get_encryption_type_name(encryption->get_type()),
		    encryption->get_blk_device()->get_name(), device_usage(encryption) });

	    table.add(row);
	}

	cout << table;
    }


    shared_ptr<ParsedCmd>
    CmdShowEncryptions::parse(GetOpts& get_opts) const
    {
	Options options(get_opts);

	return make_shared<ParsedCmdShowEncryptions>(options);
    }


    const char*
    CmdShowEncryptions::help() const
    {
	return _("Shows encryption devices.");
    }


    const ExtOptions&
    CmdShowEncryptions::options() const
    {
	return show_encryptions_options;
    }

}
