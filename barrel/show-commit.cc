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


#include <storage/Storage.h>

#include "Utils/GetOpts.h"
#include "Utils/Text.h"
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

	    for (const string& action : actiongraph->get_commit_actions_as_strings())
		cout << "  " << action << '\n';

	    if (state.pools_modified)
		cout << "  " << _("Save pools") << '\n';
	}
	catch (const Exception& e)
	{
	    cout << "failed to calculate actions" << endl;
	    cout << e.what() << endl;
	}
    }


    shared_ptr<ParsedCmd>
    CmdShowCommit::parse()
    {
	return make_shared<ParsedCmdShowCommit>();
    }


    shared_ptr<ParsedCmd>
    CmdShowCommit::parse(GetOpts& get_opts) const
    {
	get_opts.parse("commit", GetOpts::no_options);

	return make_shared<ParsedCmdShowCommit>();
    }


    const char*
    CmdShowCommit::help() const
    {
	return _("show what would be commited");
    }

}
