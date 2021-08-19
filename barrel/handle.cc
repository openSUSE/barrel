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


#include <string>
#include <vector>
#include <iostream>

#include <storage/Environment.h>
#include <storage/Storage.h>
#include <storage/Devicegraph.h>
#include <storage/Actiongraph.h>

#include "Utils/GetOpts.h"
#include "Utils/Args.h"
#include "Utils/Text.h"
#include "Utils/Misc.h"
#include "Utils/Readline.h"

#include "handle.h"
#include "cmds.h"
#include "help.h"
#include "commit.h"
#include "create-pools.h"
#include "load-pools.h"


namespace barrel
{

    using namespace std;
    using namespace storage;


    Device*
    Stack::top(Devicegraph* devicegraph)
    {
	return devicegraph->find_device(data.back());
    }


    GlobalOptions::GlobalOptions(GetOpts& get_opts)
    {
	ParsedOpts parsed_opts = get_opts.parse(get_options());

	verbose = parsed_opts.has_option("verbose");
	dry_run = parsed_opts.has_option("dry-run");

	if (parsed_opts.has_option("prefix"))
	    prefix = parsed_opts.get("prefix");

	activate = parsed_opts.has_option("activate");
	yes = parsed_opts.has_option("yes");
	help = parsed_opts.has_option("help");
    }


    const vector<Option>&
    GlobalOptions::get_options()
    {
	static const vector<Option> options = {
	    { "quiet", no_argument, 'q' },
	    { "verbose", no_argument, 'v', _("be more verbose") },
	    { "dry-run", no_argument, 0, _("do not commit anything to disk") },
	    { "prefix", required_argument, 0, _("run with a prefix"), "prefix" },
	    { "activate", no_argument, 'a' },
	    { "yes", no_argument },
	    { "help", no_argument, 'h', _("show help and exit") }
	};

	return options;
    }


    vector<shared_ptr<ParsedCmd>>
    parse(GetOpts& get_opts)
    {
	vector<shared_ptr<ParsedCmd>> cmds;

	const char* command = get_opts.pop_arg();
	vector<MainCmd>::const_iterator main_cmd = sloppy_find(main_cmds, command);

	if (main_cmd->cmd)
	{
	    cmds.emplace_back(main_cmd->cmd->parse(get_opts));
	}
	else
	{
	    if (!get_opts.has_args())
		throw runtime_error("sub command missing");

	    while (get_opts.has_args())
	    {
		const char* command = get_opts.pop_arg();
		vector<Parser>::const_iterator parse_cmd = sloppy_find(main_cmd->sub_cmds, command);
		cmds.emplace_back(parse_cmd->cmd->parse(get_opts));
	    }
	}

	return cmds;
    }


    class MyActivateCallbacks : public ActivateCallbacksLuks
    {
    public:

	virtual void message(const string& message) const override {}

	virtual bool error(const string& message, const string& what) const override
	{
	    cerr << _("error:") << ' ' << message << endl;
	    return false;
	}

	virtual bool multipath(bool looks_like_real_multipath) const override
	{
	    return looks_like_real_multipath;
	}

	virtual pair<bool, string> luks(const string& uuid, int attempt) const override
	{
	    return make_pair(false, "");
	}

	virtual pair<bool, string> luks(const LuksInfo& info, int attempt) const override
	{
	    return make_pair(false, "");
	}

    };


    class MyProbeCallbacks : public ProbeCallbacksV3
    {
    public:

	virtual void message(const string& message) const override {}

	virtual bool error(const string& message, const string& what) const override
	{
	    cerr << _("error:") << ' ' << message << endl;
	    return false;
	}

	virtual bool missing_command(const string& message, const string& what,
				     const string& command, uint64_t used_features) const override
	{
	    cerr << _("error:") << ' ' << message << endl;
	    return false;
	}

    };


    void
    startup(const GlobalOptions& global_options, Storage& storage)
    {
	storage.set_rootprefix(global_options.prefix);

	if (global_options.activate)
	{
	    cout << _("Activating...") << flush;
	    MyActivateCallbacks my_activate_callbacks;
	    storage.activate(&my_activate_callbacks);
	    cout << " done" << endl;
	}

	if (global_options.probe)
	{
	    cout << _("Probing...") << flush;
	    MyProbeCallbacks my_probe_callbacks;
	    storage.probe(&my_probe_callbacks);
	    cout << " done" << endl;
	}
    }


    void
    startup_pools(const GlobalOptions& global_options, State& state)
    {
	if (!state.testsuite)
	{
	    try
	    {
		CmdLoadPools::parse()->doit(global_options, state);
	    }
	    catch (...)
	    {
		parse_create_pools()->doit(global_options, state);
		state.pools_modified = false;
	    }
	}
	else
	{
	    parse_create_pools()->doit(global_options, state);
	    state.pools_modified = false;
	}
    }


    void
    Backup::add(Storage* storage)
    {
	string name = sformat("backup-%d", num++);
	storage->copy_devicegraph("staging", name);
	names.push_back(name);
    }


    void
    Backup::undo(Storage* storage)
    {
	storage->restore_devicegraph(names.back());
	names.pop_back();
    }


    void
    Backup::dump_last(Storage* storage) const
    {
	Devicegraph* lhs = storage->get_devicegraph(names.back());

	try
	{
	    Actiongraph actiongraph(*storage, lhs, storage->get_staging());

	    for (const string& action : actiongraph.get_commit_actions_as_strings())
		cout << "  " << action << '\n';
	}
	catch (const Exception& e)
	{
	    cout << "failed to calculate actions" << endl;
	    cout << e.what() << endl;
	}
    }


    void
    make_fixed_comp_names()
    {
	// this is only a workaround until completion is context aware

	for (const MainCmd& main_cmd : main_cmds)
	{
	    Readline::fixed_comp_names.push_back(main_cmd.name);

	    if (main_cmd.cmd)
	    {
		for (const Option& option : main_cmd.cmd->options())
		    Readline::fixed_comp_names.push_back("--"s + option.name);
	    }
	    else
	    {
		for (const Parser& sub_cmd : main_cmd.sub_cmds)
		{
		    Readline::fixed_comp_names.push_back(sub_cmd.name);

		    for (const Option& option : sub_cmd.cmd->options())
			Readline::fixed_comp_names.push_back("--"s + option.name);
		}
	    }
	}
    }


    void
    handle_interactive(const GlobalOptions& global_options, const Testsuite* testsuite)
    {
	Environment environment(false, testsuite ? ProbeMode::READ_DEVICEGRAPH : ProbeMode::STANDARD,
				TargetMode::DIRECT);

	if (testsuite)
	    environment.set_devicegraph_filename(testsuite->devicegraph_filename);

	Storage storage(environment);
	startup(global_options, storage);

	Readline readline(&storage, testsuite);
	make_fixed_comp_names();

	State state(global_options);
	state.storage = &storage;
	state.testsuite = testsuite;

	startup_pools(global_options, state);

	// TODO readline completion with proper parsing, commands, options, pools, ...

	while (state.run)
	{
	    string prompt = sformat("barrel[%ld]> ", state.stack.size());

	    char* line = readline.readline(prompt);
	    if (!line)
	    {
		cout << endl;
		break;
	    }

	    if (*line)
	    {
		try
		{
		    Args args(parse_line(line));
		    GetOpts get_opts(args.argc(), args.argv(), true, possible_blk_devices(&storage));

		    vector<shared_ptr<ParsedCmd>> cmds = parse(get_opts);

		    bool do_backup = any_of(cmds.begin(), cmds.end(), [](const shared_ptr<ParsedCmd>& cmd) {
			return cmd->do_backup();
		    });

		    if (do_backup)
			state.backup.add(&storage);

		    for (const shared_ptr<ParsedCmd>& cmd : cmds)
		    {
			cmd->doit(global_options, state);
		    }

		    if (do_backup)
			state.backup.dump_last(&storage);
		}
		catch (const exception& e)
		{
		    // TODO undo?

		    cerr << "error: " << e.what() << endl;
		}
	    }

	    free(line);
	}
    }


    void
    handle_cmdline(const GlobalOptions& global_options, const Testsuite* testsuite, GetOpts& get_opts)
    {
	// parsing must happen before probing to inform early about wrong usage
	vector<shared_ptr<ParsedCmd>> cmds = parse(get_opts);

	Environment environment(false, testsuite ? ProbeMode::READ_DEVICEGRAPH : ProbeMode::STANDARD,
				TargetMode::DIRECT);

	if (testsuite)
	    environment.set_devicegraph_filename(testsuite->devicegraph_filename);

	Storage storage(environment);
	startup(global_options, storage);

	State state(global_options);
	state.storage = &storage;
	state.testsuite = testsuite;

	startup_pools(global_options, state);

	for (const shared_ptr<ParsedCmd>& parsed_cmd : cmds)
	{
	    parsed_cmd->doit(global_options, state);
	}

	if (state.modified)
	{
	    CmdCommit::parse()->doit(global_options, state);
	}
    }


    bool
    handle(int argc, char** argv, const Testsuite* testsuite)
    {
	GetOpts get_opts(argc, argv);

	try
	{
	    GlobalOptions global_options(get_opts);

	    if (global_options.help)
	    {
		help();
		return true;
	    }

	    bool interactive = !get_opts.has_args();

	    if (interactive)
	    {
		handle_interactive(global_options, testsuite);
	    }
	    else
	    {
		handle_cmdline(global_options, testsuite, get_opts);
	    }
	}
	catch (const exception& e)
	{
	    cerr << "error: " << e.what() << endl;
	    return false;
	}

	return true;
    }

}
