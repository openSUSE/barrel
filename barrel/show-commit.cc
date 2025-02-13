/*
 * Copyright (c) [2021-2025] SUSE LLC
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


    namespace
    {

	const ExtOptions show_commit_options({
	    { "condensed", no_argument, 0, _("show condensed instead of detailed actions") }
	});


	struct Options
	{
	    Options() = default;

	    Options(GetOpts& get_opts);

	    bool show_condensed = false;
	};


	Options::Options(GetOpts& get_opts)
	{
	    ParsedOpts parsed_opts = get_opts.parse("commit", show_commit_options);

	    show_condensed = parsed_opts.has_option("condensed");
	}

    }


    class ParsedCmdShowCommit : public ParsedCmd
    {
    public:

	ParsedCmdShowCommit(const Options& options) : options(options) {}

	virtual bool do_backup() const override { return false; }

	virtual void doit(const GlobalOptions& global_options, State& state) const override;

    private:

	const Options options;

    };


    void
    ParsedCmdShowCommit::doit(const GlobalOptions& global_options, State& state) const
    {
	try
	{
	    const Actiongraph* actiongraph = state.storage->calculate_actiongraph();

	    if (!options.show_condensed)
	    {
		for (const Action::Base* action : actiongraph->get_commit_actions())
		{
		    Color color = get_color(actiongraph, action);
		    cout << "  " << colorize_message(get_string(actiongraph, action), color) << '\n';
		}
	    }
	    else
	    {
		for (const CompoundAction* compound_action : actiongraph->get_compound_actions())
		{
		    Color color = compound_action->is_delete() ? Color::RED : Color::BLACK;
		    cout << "  " << colorize_message(compound_action->sentence(), color) << '\n';
		}
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
	Options options;

	return make_shared<ParsedCmdShowCommit>(options);
    }


    shared_ptr<ParsedCmd>
    CmdShowCommit::parse(GetOpts& get_opts) const
    {
	Options options(get_opts);

	return make_shared<ParsedCmdShowCommit>(options);
    }


    const char*
    CmdShowCommit::help() const
    {
	return _("Shows commit actions.");
    }


    const ExtOptions&
    CmdShowCommit::options() const
    {
	return show_commit_options;
    }

}
