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

    CompletionHelper Readline::completion;

    Readline::Readline(const Storage* storage, const Testsuite* testsuite)
	: testsuite(testsuite)
    {
	completion.set_storage(storage);

	// Note: readline allocates memory that partly cannot be freed.

	rl_readline_name = "barrel";

	rl_attempted_completion_function = my_completion;
	rl_completion_display_matches_hook = my_display_matches;
	rl_filename_quoting_function = my_quote_filename;
	rl_basic_word_break_characters = " \t";
	rl_completer_quote_characters = "'\"";
	rl_filename_quote_characters = " ";
	rl_sort_completion_matches = 0;
	rl_char_is_quoted_p = my_char_is_quoted;

	using_history();
	stifle_history(1024);

	if (!testsuite)
	{
	    if (const char* env = getenv("HOME"); env)
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

	if (line && *line)
	    add_history(line);

	return line;
    }

    char*
    Readline::names_generator(const char* text, int state)
    {
	static size_t list_index;

	if (state == 0)
	    list_index = 0;

	if (list_index < completion.get_result().items.size())
	    return strdup(completion.get_result().items[list_index++].name.c_str());

	return nullptr;
    }

    char**
    Readline::my_completion(const char* text, int start, int end)
    {
	string_view line(rl_line_buffer, start);
	string u_string = CompletionHelper::unescape(string(text));
	vector<string> tokens;

	// Disable default filename completion
	rl_attempted_completion_over = 1;

	// Use filename completion mode to trigger quoting/dequoting
	rl_filename_completion_desired = 1;
	rl_filename_quoting_desired = 1;

	try {

	    if (!u_string.empty() && !line.empty()) {
		char last = line.back();
		if (last == '"' || last == '\'') {
		    line.remove_suffix(1);
		}
	    }
	    tokens = parse_line(line);
	} catch (const std::runtime_error &e) {
	    // silent ignore error, just no completion
	    return nullptr;
	}

	const CompletionHelper::CompletionResult res = completion.complete(tokens, u_string);

	return rl_completion_matches(u_string.c_str(), names_generator);
    }

    void
    Readline::my_display_matches(char** matches, int num_matches, int max_length)
    {
	completion.display_matches(matches, num_matches, max_length);
	rl_on_new_line();
    }


    int
    Readline::my_char_is_quoted(char *text, int index)
    {
	return (index > 0 && text[index - 1] == '\\');
    }


    char*
    Readline::my_quote_filename(char* s, int rtype, char* qcp)
    {
	char quote_char = rl_completer_quote_characters[0];

	if (!strchr(s, ' '))
	    return s;

	if (qcp && qcp[0])
	    quote_char = qcp[0];

	size_t len = strlen(s);
	char* quoted = (char*)malloc(len + 3);
	quoted[0] = quote_char;
	memcpy(quoted + 1, s, len);
	if (quoted[len] == ' ') {
	    quoted[len + 1] = '\0';
	} else {
	    quoted[len + 1] = quote_char;
	    quoted[len + 2] = '\0';
	}
	return quoted;
    }

}
