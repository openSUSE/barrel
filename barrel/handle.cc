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
#include <string_view>
#include <readline/readline.h>
#include <readline/history.h>

#include <storage/Environment.h>
#include <storage/Storage.h>
#include <storage/Devicegraph.h>
#include <storage/Actiongraph.h>

#include "Utils/GetOpts.h"
#include "Utils/Args.h"
#include "Utils/Text.h"
#include "handle.h"
#include "generic.h"
#include "show-disks.h"
#include "show-filesystems.h"
#include "show-pools.h"
#include "show-raids.h"
#include "commit.h"
#include "create-raid.h"
#include "create-encryption.h"
#include "create-partition-table.h"
#include "create-filesystem.h"
#include "create-pool.h"
#include "extend-pool.h"
#include "reduce-pool.h"
#include "remove-device.h"


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
	const vector<Option> options = {
	    { "quiet", no_argument, 'q' },
	    { "verbose", no_argument, 'v' },
	    { "dry-run", no_argument },
	    { "help", no_argument, 'h' }
	};

	ParsedOpts parsed_opts = get_opts.parse(options);

	dry_run = parsed_opts.has_option("dry-run");
    }


    typedef shared_ptr<Cmd> (*cmd_func_t)(GetOpts& get_opts);


    struct Parser
    {
	const string name;
	const cmd_func_t cmd_func;
    };


    const vector<Parser> show_cmds = {
	{ "disks", parse_show_disks },
	{ "filesystems", parse_show_filesystems },
	{ "pools", parse_show_pools },
	{ "raids", parse_show_raids }
    };


    const vector<Parser> create_cmds = {
	{ "pop", parse_pop },
	{ "dup", parse_dup },
	{ "raid", parse_create_raid },
	{ "raid0", parse_create_raid0 },
	{ "raid1", parse_create_raid1 },
	{ "raid4", parse_create_raid4 },
	{ "raid5", parse_create_raid5 },
	{ "raid6", parse_create_raid6 },
	{ "raid10", parse_create_raid10 },
	{ "encryption", parse_create_encryption },
	{ "luks1", parse_create_luks1 },
	{ "luks2", parse_create_luks2 },
	{ "partition-table", parse_create_partition_table },
	{ "gpt", parse_create_gpt },
	{ "ms-dos", parse_create_msdos },
	{ "filesystem", parse_create_filesystem },
	{ "btrfs", parse_create_btrfs },
	{ "ext2", parse_create_ext2 },
	{ "ext3", parse_create_ext3 },
	{ "ext4", parse_create_ext4 },
	{ "swap", parse_create_swap },
	{ "xfs", parse_create_xfs },
	{ "pool", parse_create_pool }
    };


    const vector<Parser> extend_cmds = {
	{ "pool", parse_extend_pool }
    };


    const vector<Parser> reduce_cmds = {
	{ "pool", parse_reduce_pool }
    };


    const vector<Parser> remove_cmds = {
	{ "device", parse_remove_device }
    };


    struct MainCmd
    {
	const string name;
	const cmd_func_t cmd_func;
	const vector<Parser>& sub_cmds;
    };


    const vector<MainCmd> main_cmds = {
	{ "pop", parse_pop, {} },
	{ "dup", parse_dup, {} },
	{ "stack", parse_stack, {} },
	{ "undo", parse_undo, {} },
	{ "quit", parse_quit, {} },
	{ "show", nullptr, show_cmds },
	{ "create", nullptr, create_cmds },
	{ "extend", nullptr, extend_cmds },
	{ "reduce", nullptr, reduce_cmds },
	{ "remove", nullptr, remove_cmds },
	{ "commit", parse_commit, {} }
    };


    vector<shared_ptr<Cmd>>
    parse(GetOpts& get_opts)
    {
	vector<shared_ptr<Cmd>> cmds;

	const char* command = get_opts.pop_arg();
	vector<MainCmd>::const_iterator main_cmd = sloppy_find(main_cmds, command);

	if (main_cmd->cmd_func)
	{
	    cmds.emplace_back((*main_cmd->cmd_func)(get_opts));
	}
	else
	{
	    if (!get_opts.has_args())
		throw runtime_error("sub command missing");

	    while (get_opts.has_args())
	    {
		const char* command = get_opts.pop_arg();
		vector<Parser>::const_iterator parse_cmd = sloppy_find(main_cmd->sub_cmds, command);
		cmds.emplace_back((*parse_cmd->cmd_func)(get_opts));
	    }
	}

	return cmds;
    }


    namespace
    {

	vector<string>
	possible_blk_devices(const Storage* storage)
	{
	    vector<string> blk_devices;

	    for (auto x : BlkDevice::get_all(storage->get_staging()))
	    {
		blk_devices.push_back(x->get_name());

		for (const string t : x->get_udev_paths())
		    blk_devices.push_back("/dev/disk/by-path/" + t);
		for (const string t : x->get_udev_ids())
		    blk_devices.push_back("/dev/disk/by-id/" + t);
	    }

	    sort(blk_devices.begin(), blk_devices.end());

	    return blk_devices;
	}


	// TODO try to make that not global
	Storage* comp_storage;
	vector<string> comp_names;


	string
	escape(const string& original)
	{
	    string escaped;

	    for (char c : original)
	    {
		if (c == ' ')
		    escaped += '\\';

		escaped += c;
	    }

	    return escaped;
	}


	char*
	names_generator(const char* text, int state)
	{
	    static size_t list_index, len;

	    if (state == 0)
	    {
		list_index = 0;
		len = strlen(text);
	    }

	    while (list_index < comp_names.size())
	    {
		const string& name = comp_names[list_index++];

		if (rl_completion_quote_character)
		{
		    if (strncmp(name.c_str(), text, len) == 0)
			return strdup(name.c_str());
		}
		else
		{
		    string tmp = escape(name);
		    if (strncmp(tmp.c_str(), text, len) == 0)
			return strdup(tmp.c_str());
		}
	    }

	    return nullptr;
	}


	char**
	my_completion(const char* text, int start, int end)
	{
	    rl_attempted_completion_over = 1;

	    comp_names.clear();

	    // TODO depending on previous arguments

	    comp_names.push_back("create");
	    comp_names.push_back("raid");
	    comp_names.push_back("raids");
	    comp_names.push_back("raid0");
	    comp_names.push_back("raid1");
	    comp_names.push_back("raid5");
	    comp_names.push_back("gpt");
	    comp_names.push_back("xfs");
	    comp_names.push_back("show");
	    comp_names.push_back("disks");
	    comp_names.push_back("filesystems");
	    comp_names.push_back("pool");
	    comp_names.push_back("pools");
	    comp_names.push_back("extend");
	    comp_names.push_back("reduce");
	    comp_names.push_back("remove");
	    comp_names.push_back("device");
	    comp_names.push_back("pop");
	    comp_names.push_back("dup");
	    comp_names.push_back("stack");
	    comp_names.push_back("undo");
	    comp_names.push_back("--size");
	    comp_names.push_back("--size");
	    comp_names.push_back("--devices");
	    comp_names.push_back("--pool");
	    comp_names.push_back("--name");

	    // TODO normally tab completion only goes upto the path component

	    for (auto x : possible_blk_devices(comp_storage))
		comp_names.push_back(x);

	    for (auto x : comp_storage->get_pools())
		comp_names.push_back(x.first);

	    return rl_completion_matches(text, names_generator);
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

	Actiongraph actiongraph(*storage, lhs, storage->get_staging());

	for (const string& action : actiongraph.get_commit_actions_as_strings())
	    cout << action << '\n';
    }


    string
    make_history_file()
    {
	const char* env = getenv("HOME");
	if (!env)
	    return "";

	return string(env) + "/.barrel_history";
    }


    void
    handle_interactive(const GlobalOptions& global_options, const Testsuite* testsuite)
    {
	rl_readline_name = "barrel";

	rl_attempted_completion_function = my_completion;
	rl_completer_quote_characters = "\"'";

	using_history();
	stifle_history(1024);

	const string history_file = make_history_file();

	if (!history_file.empty())
	    read_history(history_file.c_str());

	Environment environment(false, testsuite ? ProbeMode::READ_DEVICEGRAPH : ProbeMode::STANDARD,
				TargetMode::DIRECT);

	if (testsuite)
	    environment.set_devicegraph_filename(testsuite->devicegraph_filename);

	cout << _("Probing...") << flush;
	Storage storage(environment);
	storage.probe();
	storage.generate_pools(storage.get_probed());
	cout << " done" << endl;

	comp_storage = &storage;

	State state(global_options);
	state.storage = &storage;
	state.testsuite = testsuite;

	// TODO readline completion with proper parsing, commands, options, pools, ...

	while (state.run)
	{
	    string prompt = sformat("barrel[%ld]> ", state.stack.size());

	    char* line = readline(prompt.c_str());
	    if (!line)
	    {
		cout << endl;
		break;
	    }

	    if (*line)
	    {
		add_history(line);

		try
		{
		    Args args(parse_line(line));
		    GetOpts get_opts(args.argc(), args.argv(), true, possible_blk_devices(&storage));

		    vector<shared_ptr<Cmd>> cmds = parse(get_opts);

		    bool do_backup = any_of(cmds.begin(), cmds.end(), [](const shared_ptr<Cmd>& cmd) {
			return cmd->do_backup();
		    });

		    if (do_backup)
			state.backup.add(&storage);

		    for (const shared_ptr<Cmd> cmd : cmds)
		    {
			cmd->doit(state);
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

	if (!history_file.empty())
	    write_history(history_file.c_str());
    }


    void
    handle_cmdline(const GlobalOptions& global_options, const Testsuite* testsuite, GetOpts& get_opts)
    {
	vector<shared_ptr<Cmd>> cmds = parse(get_opts);

	Environment environment(false, testsuite ? ProbeMode::READ_DEVICEGRAPH : ProbeMode::STANDARD,
				TargetMode::DIRECT);

	if (testsuite)
	    environment.set_devicegraph_filename(testsuite->devicegraph_filename);

	cout << "Probing..." << flush;
	Storage storage(environment);
	storage.probe();
	storage.generate_pools(storage.get_probed());
	cout << " done" << endl;

	State state(global_options);
	state.storage = &storage;
	state.testsuite = testsuite;

	for (const shared_ptr<Cmd> cmd : cmds)
	{
	    cmd->doit(state);
	}

	if (state.modified)
	{
	    shared_ptr<Cmd> cmd_commit = parse_commit();
	    cmd_commit->doit(state);
	}
    }


    bool
    handle(int argc, char** argv, const Testsuite* testsuite)
    {
	GetOpts get_opts(argc, argv);

	try
	{
	    GlobalOptions global_options(get_opts);

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
