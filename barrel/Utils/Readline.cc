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

	// TODO make completion context aware

	comp_names = fixed_comp_names;

	// TODO normally tab completion only goes upto the path component

	for (auto x : possible_blk_devices(storage))
	    comp_names.push_back(x);

	for (auto x : storage->get_pools())
	    comp_names.push_back(x.first);

	return rl_completion_matches(text, names_generator);
    }


    const Storage* Readline::storage;

    vector<string> Readline::fixed_comp_names;
    vector<string> Readline::comp_names;

}
