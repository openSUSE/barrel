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


namespace barrel
{

    // TODO lots of static members

    class Readline
    {
    public:

	Readline(const Storage* storage, const Testsuite* testsuite);
	~Readline();

	char* readline(const string& prompt);

	static const Storage* storage;

    private:

	const Testsuite* testsuite;

	string history_file;

	vector<string>::const_iterator it;

	static string escape(const string& original);

	static char* names_generator(const char* text, int state);

	static char** my_completion(const char* text, int start, int end);

	static vector<string> comp_names;

    };

}


#endif
