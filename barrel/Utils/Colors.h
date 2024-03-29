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


#ifndef BARREL_COLORS_H
#define BARREL_COLORS_H


#include <string>

#include <storage/Actions/Base.h>


using namespace std;
using namespace storage;


class Colors
{
public:

    static string red(const string& s);
    static string green(const string& s);

    static bool may_use_ansi_escapes_codes();

    static bool use_ansi_escape_codes;

};


string
colorize_message(const string& s, Color color);


#endif
