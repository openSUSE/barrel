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
#include <storage/Actiongraph.h>

#include "Utils/GetOpts.h"
#include "Utils/Text.h"
#include "Utils/BarrelDefines.h"
#include "show-actiongraph.h"
#include "show.h"


namespace barrel
{

    using namespace std;
    using namespace storage;


    class ParsedCmdShowActiongraph : public ParsedCmdShow
    {
    public:

	virtual bool do_backup() const override { return false; }

	virtual void doit(const GlobalOptions& global_options, State& state) const override;

    };


    void
    ParsedCmdShowActiongraph::doit(const GlobalOptions& global_options, State& state) const
    {
	Storage* storage = state.storage;

	try
        {
            const Actiongraph* actiongraph = storage->calculate_actiongraph();

	    string filename_gv = "actiongraph.gv";
	    string filename_svg = "actiongraph.svg";

	    actiongraph->write_graphviz(filename_gv, get_debug_actiongraph_style_callbacks());

	    system(string(DOT_BIN " -Tsvg < " + filename_gv + " > " + filename_svg).c_str());
	    system(string(DISPLAY_BIN " " + filename_svg).c_str());
	}
        catch (const Exception& e)
        {
            cout << _("failed to calculate actions") << endl;
            cout << e.what() << endl;
        }
    }


    shared_ptr<ParsedCmd>
    CmdShowActiongraph::parse(GetOpts& get_opts) const
    {
	get_opts.parse("actiongraph", GetOpts::no_ext_options);

	return make_shared<ParsedCmdShowActiongraph>();
    }


    const char*
    CmdShowActiongraph::help() const
    {
	return _("Shows actiongraph.");
    }

}
