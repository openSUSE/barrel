/*
 * Copyright (c) [2021-2024] SUSE LLC
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
#include <storage/Actions/Create.h>
#include <storage/Actions/Delete.h>
#include <storage/Version.h>
#include <storage/SystemInfo/SystemInfo.h>

#include "Utils/GetOpts.h"
#include "Utils/Args.h"
#include "Utils/Text.h"
#include "Utils/Misc.h"
#include "Utils/Readline.h"
#include "Utils/Prompt.h"
#include "Utils/Colors.h"

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


    GlobalOptions::GlobalOptions(GetOpts& get_opts)
    {
	ParsedOpts parsed_opts = get_opts.parse(get_options());

	quiet = parsed_opts.has_option("quiet");
	verbose = parsed_opts.has_option("verbose");

	table_style = table_style_value(parsed_opts);

	if (parsed_opts.has_option("color"))
	    color = true;
	if (parsed_opts.has_option("no-color"))
	    color = false;

	dry_run = parsed_opts.has_option("dry-run");

	if (parsed_opts.has_option("rootprefix"))
	    rootprefix = parsed_opts.get("rootprefix");
	else if (parsed_opts.has_option("prefix"))
	    rootprefix = parsed_opts.get("prefix");

	activate = parsed_opts.has_option("activate");
	yes = parsed_opts.has_option("yes");
	help = parsed_opts.has_option("help");
	version = parsed_opts.has_option("version");
    }


    Style
    GlobalOptions::table_style_value(const ParsedOpts& opts) const
    {
	ParsedOpts::const_iterator it = opts.find("table-style");
	if (it == opts.end())
	    return Table::auto_style();

	try
	{
	    unsigned long value = stoul(it->second);

	    if (value >= Table::num_styles)
		throw exception();

	    return (Style)(value);
	}
	catch (const exception&)
	{
	    string error = sformat(_("Invalid table style '%s'."), it->second.c_str()) + '\n' +
		sformat(_("Use an integer number from %d to %d."), 0, Table::num_styles - 1);

	    throw runtime_error(error);
	}

	return Table::auto_style();
    }


    const ExtOptions&
    GlobalOptions::get_options()
    {
	static const ExtOptions options({
	    { "quiet", no_argument, 'q', _("be quiet") },
	    { "verbose", no_argument, 'v', _("be more verbose") },
	    { "table-style", required_argument, 't', _("table style"), "table-style" },
	    { "color", no_argument, 0, _("show colors") },
	    { "no-color", no_argument, 0, _("do not show colors") },
	    { "dry-run", no_argument, 0, _("do not commit anything to disk") },
	    { "rootprefix", required_argument, 0, _("run with a rootprefix"), "rootprefix" },
	    { "prefix", required_argument, 0, nullptr }, // replaced by rootprefix
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
	// GetOpts::parse explicitly. Also notice that getopt sets optind to 1 if it is 0
	// to skip the program name. For that reason argv includes a program name as first
	// element.

	get_opts.parse(GetOpts::no_ext_options);

	const char* command1 = get_opts.pop_arg();
	vector<MainCmd>::const_iterator main_cmd = sloppy_find(main_cmds, command1);

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
		const char* command2 = get_opts.pop_arg();
		vector<Parser>::const_iterator parse_cmd = sloppy_find(main_cmd->sub_cmds, command2);
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
    startup(const GlobalOptions& global_options, unique_ptr<SystemInfo>& system_info, Storage& storage)
    {
	if (global_options.activate)
	{
	    MyActivateCallbacks my_activate_callbacks(global_options);
	    storage.activate(&my_activate_callbacks);
#if LIBSTORAGE_NG_VERSION_AT_LEAST(1, 96)
	    system_info = make_unique<SystemInfo>();	// used as reset
#endif
	}

	if (global_options.probe)
	{
	    MyProbeCallbacks my_probe_callbacks(global_options);

#if LIBSTORAGE_NG_VERSION_AT_LEAST(1, 96)
	    storage.probe(*system_info, &my_probe_callbacks);
#else
	    storage.probe(&my_probe_callbacks);
#endif
	}
    }


    void
    startup_pools(const GlobalOptions& global_options, unique_ptr<SystemInfo>& system_info, State& state)
    {
	if (!state.testsuite)
	{
	    try
	    {
		cmd_load_pools(global_options, *system_info, state);
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

	    for (const Action::Base* action : actiongraph.get_commit_actions())
	    {
		cout << "  " << colorize_message(get_string(&actiongraph, action),
						 get_color(&actiongraph, action)) << '\n';
	    }
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
		    if (option.description)
			Readline::fixed_comp_names.push_back("--"s + option.name);
	    }
	    else
	    {
		for (const Parser& sub_cmd : main_cmd.sub_cmds)
		{
		    Readline::fixed_comp_names.push_back(sub_cmd.name);

		    for (const Option& option : sub_cmd.cmd->options().options)
			if (option.description)
			    Readline::fixed_comp_names.push_back("--"s + option.name);
		}
	    }
	}
    }


    bool
    interactive_ignore_line(const char* line)
    {
	size_t pos = strspn(line, " ");
	return line[pos] == '\0' || line[pos] == '#';
    }


    void
    handle_interactive(const GlobalOptions& global_options, Testsuite* testsuite)
    {
	Environment environment(false, testsuite ? ProbeMode::READ_DEVICEGRAPH : ProbeMode::STANDARD,
				TargetMode::DIRECT);

	if (global_options.rootprefix)
	    environment.set_rootprefix(global_options.rootprefix.value());

	if (testsuite)
	    environment.set_devicegraph_filename(testsuite->devicegraph_filename);

	unique_ptr<Storage> storage = make_unique<Storage>(environment);
	unique_ptr<SystemInfo> system_info = make_unique<SystemInfo>();

	startup(global_options, system_info, *storage);

	Readline readline(storage.get(), testsuite);
	make_fixed_comp_names();

	State state(global_options);
	state.storage = storage.get();
	state.testsuite = testsuite;

	startup_pools(global_options, system_info, state);

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

	    if (!interactive_ignore_line(line))
	    {
		if (testsuite)
		    cout << line << '\n';

		try
		{
		    Args args(parse_line(line));
		    GetOpts get_opts(args.argc(), args.argv(), true, possible_blk_devices(storage.get()));

		    vector<shared_ptr<ParsedCmd>> cmds = parse(get_opts);

		    bool do_backup = any_of(cmds.begin(), cmds.end(), [](const shared_ptr<ParsedCmd>& cmd) {
			return cmd->do_backup();
		    });

		    if (do_backup)
			state.backup.add(storage.get());

		    StagingGuard staging_guard(storage.get());
		    StackGuard stack_guard(state.stack);

		    for (const shared_ptr<ParsedCmd>& cmd : cmds)
		    {
			cmd->doit(global_options, state);
		    }

		    stack_guard.release();
		    staging_guard.release();

		    if (do_backup)
			state.backup.dump_last(storage.get());
		}
		catch (const exception& e)
		{
		    cerr << "error: " << e.what() << endl;
		}
	    }

	    free(line);
	}

	if (testsuite)
	{
	    testsuite->storage = std::move(storage);
	    testsuite->stack = make_unique<Stack>(std::move(state.stack));
	}
    }


    void
    handle_cmdline(const GlobalOptions& global_options, Testsuite* testsuite, GetOpts& get_opts)
    {
	// parsing must happen before probing to inform early about wrong usage
	vector<shared_ptr<ParsedCmd>> cmds = parse(get_opts);

	Environment environment(false, testsuite ? ProbeMode::READ_DEVICEGRAPH : ProbeMode::STANDARD,
				TargetMode::DIRECT);

	if (global_options.rootprefix)
	    environment.set_rootprefix(global_options.rootprefix.value());

	if (testsuite)
	    environment.set_devicegraph_filename(testsuite->devicegraph_filename);

	unique_ptr<Storage> storage = make_unique<Storage>(environment);
	unique_ptr<SystemInfo> system_info = make_unique<SystemInfo>();

	startup(global_options, system_info, *storage);

	State state(global_options);
	state.storage = storage.get();
	state.testsuite = testsuite;

	startup_pools(global_options, system_info, state);

	for (const shared_ptr<ParsedCmd>& parsed_cmd : cmds)
	{
	    parsed_cmd->doit(global_options, state);
	}

	if (state.modified)
	{
	    CmdCommit::parse()->doit(global_options, state);
	}

	if (testsuite)
	{
	    testsuite->storage = std::move(storage);
	    testsuite->stack = make_unique<Stack>(std::move(state.stack));
	}
    }


    bool
    handle(int argc, char** argv, Testsuite* testsuite)
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

		cout << "libstorage-ng " << LIBSTORAGE_NG_VERSION_STRING;
#if LIBSTORAGE_NG_VERSION_AT_LEAST(1, 92)
		if (strcmp(LIBSTORAGE_NG_VERSION_STRING, get_libversion_string()) != 0)
		    cout << " (" << get_libversion_string() << ")";
#endif
		cout << '\n';

		return true;
	    }

	    if (global_options.color.has_value())
		Colors::use_ansi_escape_codes = global_options.color.value();
	    else
		Colors::use_ansi_escape_codes = Colors::may_use_ansi_escapes_codes();

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
