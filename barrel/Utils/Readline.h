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


#ifndef BARREL_READLINE_H
#define BARREL_READLINE_H


#include "Misc.h"
#include "CompletionHelper.h"


namespace barrel
{

    // TODO lots of static members

    class Readline
    {
    public:

	Readline(const Storage* storage, const Testsuite* testsuite);
	~Readline();

	char* readline(const string& prompt);

    private:

	static CompletionHelper completion;

	const Testsuite* testsuite;

	string history_file;

	vector<string>::const_iterator it;

	static char* names_generator(const char* text, int state);

	static char** my_completion(const char* text, int start, int end);

	static void my_display_matches(char** matches, int num_matches, int max_length);

	static int my_char_is_quoted(char *text, int index);

	static char* my_quote_filename(char* s, int rtype, char* qcp);
    };

}


#endif
