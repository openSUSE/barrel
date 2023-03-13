/*
 * Copyright (c) 2023 SUSE LLC
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


#include <unistd.h>
#include <string.h>

#include "Colors.h"

using namespace std;


#define ESC "\033"

#define FG_RED		ESC "[31m"
#define FG_GREEN	ESC "[32m"
#define FG_DEFAULT	ESC "[39m"


bool Colors::use_ansi_escape_codes = false;


string
Colors::red(const string& s)
{
    if (!use_ansi_escape_codes)
	return s;

    return FG_RED + s + FG_DEFAULT;
}


string
Colors::green(const string& s)
{
    if (!use_ansi_escape_codes)
	return s;

    return FG_GREEN + s + FG_DEFAULT;
}


bool
Colors::may_use_ansi_escapes_codes()
{
    if (!isatty(STDOUT_FILENO))
	return false;

    const char* term = getenv("TERM");
    if (term && strcmp(term, "dumb") != 0)
	return true;

    return false;
}


string
colorize_message(const string& s, bool green, bool red)
{
#if 0
    if (green)
	return Colors::green(s);
#endif

    if (red)
	return Colors::red(s);

    return s;
}
