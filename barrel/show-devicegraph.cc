/*
 * Copyright (c) 2022 SUSE LLC
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
#include "Utils/BarrelDefines.h"
#include "show-devicegraph.h"
#include "show.h"


namespace barrel
{

    using namespace std;
    using namespace storage;


    namespace
    {

        const ExtOptions show_devicegraph_options({
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
            ParsedOpts parsed_opts = get_opts.parse("devicegraph", show_devicegraph_options);

            show_probed = parsed_opts.has_option("probed");
        }

    }


    class ParsedCmdShowDevicegraph : public ParsedCmdShow
    {
    public:

	ParsedCmdShowDevicegraph(const Options& options) : options(options) {}

	virtual bool do_backup() const override { return false; }

	virtual void doit(const GlobalOptions& global_options, State& state) const override;

    private:

        const Options options;

    };


    void
    ParsedCmdShowDevicegraph::doit(const GlobalOptions& global_options, State& state) const
    {
	const Storage* storage = state.storage;

        const Devicegraph* devicegraph = options.show_probed ? storage->get_probed() : storage->get_staging();

	string filename_gv = "devicegraph.gv";
	string filename_svg = "devicegraph.svg";

	devicegraph->write_graphviz(filename_gv, get_debug_devicegraph_style_callbacks(), View::CLASSIC);

	system(string(DOT_BIN " -Tsvg < " + filename_gv + " > " + filename_svg).c_str());
	system(string(DISPLAY_BIN " " + filename_svg).c_str());
    }


    shared_ptr<ParsedCmd>
    CmdShowDevicegraph::parse(GetOpts& get_opts) const
    {
	Options options(get_opts);

	return make_shared<ParsedCmdShowDevicegraph>(options);
    }


    const char*
    CmdShowDevicegraph::help() const
    {
	return _("Shows devicegraph.");
    }

}
