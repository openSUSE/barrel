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


#include <stdio.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "Readline.h"


namespace barrel
{

    Readline::Readline(const Storage* storage, const Testsuite* testsuite)
	: testsuite(testsuite)
    {
	Readline::storage = storage;

	rl_readline_name = "barrel";

	rl_attempted_completion_function = my_completion;
	rl_completer_quote_characters = "\"'";

	using_history();
	stifle_history(1024);

	if (!testsuite)
	{
	    const char* env = getenv("HOME");
	    if (env)
		history_file = string(env) + "/.barrel_history";
	}

	if (!history_file.empty())
	    read_history(history_file.c_str());

	if (testsuite)
	    it = testsuite->readlines.begin();
    }


    Readline::~Readline()
    {
	if (!history_file.empty())
	    write_history(history_file.c_str());
    }


    char*
    Readline::readline(const string& prompt)
    {
	char* line = nullptr;

	if (!testsuite)
	    line = ::readline(prompt.c_str());
	else if (it != testsuite->readlines.end())
	    line = strdup((it++)->c_str());

	if (line)
	    add_history(line);

	return line;
    }


    string
    Readline::escape(const string& original)
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
    Readline::names_generator(const char* text, int state)
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
    Readline::my_completion(const char* text, int start, int end)
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
	comp_names.push_back("vg");
	comp_names.push_back("vgs");
	comp_names.push_back("lv");
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
	comp_names.push_back("devicegraph");
	comp_names.push_back("pop");
	comp_names.push_back("dup");
	comp_names.push_back("stack");
	comp_names.push_back("undo");
	comp_names.push_back("load");
	comp_names.push_back("save");
	comp_names.push_back("commit");
	comp_names.push_back("--size");
	comp_names.push_back("--size");
	comp_names.push_back("--devices");
	comp_names.push_back("--pool");
	comp_names.push_back("--name");
	comp_names.push_back("--devicegraph");
	comp_names.push_back("--mapping");
	comp_names.push_back("--stripes");
	comp_names.push_back("--probed");

	// TODO normally tab completion only goes upto the path component

	for (auto x : possible_blk_devices(storage))
	    comp_names.push_back(x);

	for (auto x : storage->get_pools())
	    comp_names.push_back(x.first);

	return rl_completion_matches(text, names_generator);
    }


    const Storage* Readline::storage;

    vector<string> Readline::comp_names;

}
