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


#include <storage/Actiongraph.h>
#include <storage/Actions/Create.h>
#include <storage/Actions/Delete.h>

#include "Utils/GetOpts.h"
#include "Utils/Text.h"
#include "Utils/Colors.h"
#include "show-commit.h"


namespace barrel
{

    using namespace std;
    using namespace storage;


    class ParsedCmdShowCommit : public ParsedCmd
    {
    public:

	virtual bool do_backup() const override { return false; }

	virtual void doit(const GlobalOptions& global_options, State& state) const override;

    };


    void
    ParsedCmdShowCommit::doit(const GlobalOptions& global_options, State& state) const
    {
	try
	{
	    const Actiongraph* actiongraph = state.storage->calculate_actiongraph();

	    for (const Action::Base* action : actiongraph->get_commit_actions())
	    {
		cout << "  " << colorize_message(get_string(actiongraph, action),
						 get_color(actiongraph, action)) << '\n';
	    }
	}
	catch (const Exception& e)
	{
	    cout << _("failed to calculate actions") << endl;
	    cout << e.what() << endl;
	}

	if (state.pools_modified)
	    cout << "  " << _("Save pools") << '\n';
    }


    shared_ptr<ParsedCmd>
    CmdShowCommit::parse()
    {
	return make_shared<ParsedCmdShowCommit>();
    }


    shared_ptr<ParsedCmd>
    CmdShowCommit::parse(GetOpts& get_opts) const
    {
	get_opts.parse("commit", GetOpts::no_ext_options);

	return make_shared<ParsedCmdShowCommit>();
    }


    const char*
    CmdShowCommit::help() const
    {
	return _("Shows commit actions.");
    }

}
