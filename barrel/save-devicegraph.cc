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
#include <storage/Devicegraph.h>

#include "Utils/GetOpts.h"
#include "Utils/Text.h"
#include "save-devicegraph.h"


namespace barrel
{

    using namespace storage;


    namespace
    {

	const ExtOptions save_devicegraph_options({
	    { "name", required_argument, 'n', _("name of devicegraph"), "name" },
	    { "probed", no_argument, 0, _("use probed instead of staging devicegraph") }
	});


	struct Options
	{
	    Options(GetOpts& get_opts);

	    string name;

	    bool show_probed = false;
	};


	Options::Options(GetOpts& get_opts)
	{
	    ParsedOpts parsed_opts = get_opts.parse("devicegraph", save_devicegraph_options);

	    if (!parsed_opts.has_option("name"))
		throw OptionsException(_("name missing for command 'devicegraph'"));

	    name = parsed_opts.get("name");

	    show_probed = parsed_opts.has_option("probed");
	}

    }


    class ParsedCmdSaveDevicegraph : public ParsedCmd
    {
    public:

	ParsedCmdSaveDevicegraph(const Options& options) : options(options) {}

	virtual bool do_backup() const override { return true; }

	virtual void doit(const GlobalOptions& global_options, State& state) const override;

    private:

	const Options options;

    };


    void
    ParsedCmdSaveDevicegraph::doit(const GlobalOptions& global_options, State& state) const
    {
	const Storage* storage = state.storage;

	const Devicegraph* devicegraph = options.show_probed ? storage->get_probed() : storage->get_staging();

	devicegraph->save(options.name);
    }


    shared_ptr<ParsedCmd>
    CmdSaveDevicegraph::parse(GetOpts& get_opts) const
    {
	Options options(get_opts);

	return make_shared<ParsedCmdSaveDevicegraph>(options);
    }


    const char*
    CmdSaveDevicegraph::help() const
    {
	return _("Saves the devicegraph to a file.");
    }


    const ExtOptions&
    CmdSaveDevicegraph::options() const
    {
	return save_devicegraph_options;
    }

}
