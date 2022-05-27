/*
 * Copyright (c) [2021-2022] SUSE LLC
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


#include "config.h"

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
#include "Utils/Prompt.h"

#include "handle.h"
#include "cmds.h"
#include "help.h"
#include "generic.h"
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
	return devicegraph->find_device(data.front());
    }


    GlobalOptions::GlobalOptions(GetOpts& get_opts)
    {
	ParsedOpts parsed_opts = get_opts.parse(get_options());

	quiet = parsed_opts.has_option("quiet");
	verbose = parsed_opts.has_option("verbose");
	dry_run = parsed_opts.has_option("dry-run");

	if (parsed_opts.has_option("prefix"))
	    prefix = parsed_opts.get("prefix");

	activate = parsed_opts.has_option("activate");
	yes = parsed_opts.has_option("yes");
	help = parsed_opts.has_option("help");
	version = parsed_opts.has_option("version");
    }


    const ExtOptions&
    GlobalOptions::get_options()
    {
	static const ExtOptions options({
	    { "quiet", no_argument, 'q', _("be quiet") },
	    { "verbose", no_argument, 'v', _("be more verbose") },
	    { "dry-run", no_argument, 0, _("do not commit anything to disk") },
	    { "prefix", required_argument, 0, _("run with a prefix"), "prefix" },
	    { "activate", no_argument, 'a', _("activate storage systems at startup") },
	    { "yes", no_argument, 0, _("answer all questions with yes") },
	    { "help", no_argument, 'h', _("show help and exit") } ,
	    { "version", no_argument, 0, _("show version and exit") }
	});

	return options;
    }


    vector<shared_ptr<ParsedCmd>>
    parse(GetOpts& get_opts)
    {
	vector<shared_ptr<ParsedCmd>> cmds;

	// This is tricky. getopt needs optind = 0 to reinit itself in interactive
	// mode. Without that strange things can happen, esp. with short options. So call
	// GetOpts::parse explicitely. Also notice that getopt sets optind to 1 if it is 0
	// to skip the program name. For that reason argv includes a program name as first
	// element.

	get_opts.parse(GetOpts::no_ext_options);

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


    class MyActivateCallbacks : public ActivateCallbacksV3
    {
    public:

	MyActivateCallbacks(const GlobalOptions& global_options)
	    : global_options(global_options)
	{
	}

	virtual void begin() const override
	{
	    if (global_options.verbose || global_options.quiet)
		return;

	    cout << _("Activating...") << flush;
	    beginning_of_line = false;
	}

	virtual void end() const override
	{
	    if (global_options.verbose || global_options.quiet)
		return;

	    if (!beginning_of_line)
		cout << " ";

	    cout << _("done") << endl;
	    beginning_of_line = true;
	}

	virtual void message(const string& message) const override
	{
	    if (!global_options.verbose)
		return;

	    if (!beginning_of_line)
		cout << '\n';
	    beginning_of_line = true;

	    cout << message << endl;
	}

	virtual bool error(const string& message, const string& what) const override
	{
	    if (!beginning_of_line)
		cout << '\n';
	    beginning_of_line = true;

	    cerr << _("error:") << ' ' << message << endl;

	    return false;
	}

	virtual bool multipath(bool looks_like_real_multipath) const override
	{
	    return looks_like_real_multipath;
	}

	virtual pair<bool, string> luks(const string& uuid, int attempt) const override
	{
	    return make_pair(false, "");	// unused
	}

	virtual pair<bool, string> luks(const LuksInfo& luks_info, int attempt) const override
	{
	    string message;

	    if (luks_info.is_dm_table_name_generated())
	    {
		message = sformat(_("Activate LUKS on %1$s (%2$s) with UUID %3$s..."),
				  luks_info.get_device_name().c_str(),
				  format_size(luks_info.get_size(), false).c_str(),
				  luks_info.get_uuid().c_str());
	    }
	    else
	    {
		message = sformat(_("Activate LUKS %1$s on %2$s (%3$s) with UUID %4$s..."),
				  luks_info.get_dm_table_name().c_str(),
				  luks_info.get_device_name().c_str(),
				  format_size(luks_info.get_size(), false).c_str(),
				  luks_info.get_uuid().c_str());
	    }

	    return password(message);
	}

	virtual pair<bool, string> bitlocker(const BitlockerInfo& bitlocker_info, int attempt) const override
	{
	    string message;

	    if (bitlocker_info.is_dm_table_name_generated())
	    {
		message = sformat(_("Activate BitLocker on %1$s (%2$s) with UUID %3$s..."),
				  bitlocker_info.get_device_name().c_str(),
				  format_size(bitlocker_info.get_size(), false).c_str(),
				  bitlocker_info.get_uuid().c_str());
	    }
	    else
	    {
		message = sformat(_("Activate BitLocker %1$s on %2$s (%3$s) with UUID %4$s..."),
				  bitlocker_info.get_dm_table_name().c_str(),
				  bitlocker_info.get_device_name().c_str(),
				  format_size(bitlocker_info.get_size(), false).c_str(),
				  bitlocker_info.get_uuid().c_str());
	    }

	    return password(message);
	}

    private:

	pair<bool, string> password(const string& message) const
	{
	    if (!beginning_of_line)
		cout << '\n';
	    beginning_of_line = true;

	    cout << message << endl;

	    string password = prompt_password(false);

	    return make_pair(!password.empty(), password);
	}

	const GlobalOptions& global_options;

	mutable bool beginning_of_line = true;

    };


    class MyProbeCallbacks : public ProbeCallbacksV3
    {
    public:

	MyProbeCallbacks(const GlobalOptions& global_options)
	    : global_options(global_options)
	{
	}

	virtual void begin() const override
	{
	    if (global_options.verbose || global_options.quiet)
		return;

	    cout << _("Probing...") << flush;
	    beginning_of_line = false;
	}

	virtual void end() const override
	{
	    if (global_options.verbose || global_options.quiet)
		return;

	    if (!beginning_of_line)
		cout << " ";

	    cout << _("done") << endl;
	    beginning_of_line = true;
	}

	virtual void message(const string& message) const override
	{
	    if (!global_options.verbose)
		return;

	    cout << message << endl;
	}

	virtual bool error(const string& message, const string& what) const override
	{
	    if (!beginning_of_line)
		cout << '\n';
	    beginning_of_line = true;

	    cerr << _("error:") << ' ' << message << endl;

	    return prompt(_("Continue?"));
	}

	virtual bool missing_command(const string& message, const string& what,
				     const string& command, uint64_t used_features) const override
	{
	    return error(message, what);
	}

    private:

	const GlobalOptions& global_options;

	mutable bool beginning_of_line = true;

    };


    void
    startup(const GlobalOptions& global_options, Storage& storage)
    {
	storage.set_rootprefix(global_options.prefix);

	if (global_options.activate)
	{
	    MyActivateCallbacks my_activate_callbacks(global_options);
	    storage.activate(&my_activate_callbacks);
	}

	if (global_options.probe)
	{
	    MyProbeCallbacks my_probe_callbacks(global_options);
	    storage.probe(&my_probe_callbacks);
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
		for (const Option& option : main_cmd.cmd->options().options)
		    Readline::fixed_comp_names.push_back("--"s + option.name);
	    }
	    else
	    {
		for (const Parser& sub_cmd : main_cmd.sub_cmds)
		{
		    Readline::fixed_comp_names.push_back(sub_cmd.name);

		    for (const Option& option : sub_cmd.cmd->options().options)
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
	    string prompt = "barrel";
	    if (!state.stack.empty())
		prompt += sformat("[%ld]", state.stack.size());
	    prompt += "> ";

	    char* line = readline.readline(prompt);
	    if (!line)
	    {
		cout << endl;
		CmdQuit::parse()->doit(global_options, state);

		continue;
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
		help(true);
		return true;
	    }

	    if (global_options.version)
	    {
		cout << "barrel " VERSION << '\n';
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
