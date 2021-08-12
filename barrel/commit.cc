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
#include <storage/Actiongraph.h>

#include "commit.h"
#include "Utils/Text.h"
#include "Utils/Prompt.h"
#include "show-commit.h"
#include "save-pools.h"


namespace barrel
{

    class CmdCommit : public Cmd
    {
    public:

	virtual bool do_backup() const override { return false; } // TODO ???

	virtual void doit(const GlobalOptions& global_options, State& state) const override;

    };


    void
    CmdCommit::doit(const GlobalOptions& global_options, State& state) const
    {
	parse_show_commit()->doit(global_options, state);

	if (!global_options.yes)
	{
	    if (!prompt(_("Commit changes?")))
		return;
	}

	const Actiongraph* actiongraph = state.storage->calculate_actiongraph();

	if (state.testsuite && state.testsuite->save_actiongraph)
	    state.testsuite->save_actiongraph(actiongraph);

	if (state.global_options.dry_run)
	{
	    cout << "dry run" << endl;
	}
	else
	{
	    CommitOptions commit_options(false);
	    state.storage->commit(commit_options);

	    if (state.pools_modified)
		parse_save_pools()->doit(global_options, state);
	}

	state.run = false;
	state.modified = false;
    }


    shared_ptr<Cmd>
    parse_commit(GetOpts& get_opts)
    {
	get_opts.parse("commit", GetOpts::no_options);

	return make_shared<CmdCommit>();
    }


    shared_ptr<Cmd>
    parse_commit()
    {
	return make_shared<CmdCommit>();
    }

}
