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


#ifndef BARREL_TEXT_H
#define BARREL_TEXT_H


#include <string>
#include <vector>


namespace barrel
{

    using namespace std;


    const char*
    _(const char* msgid);

    const char*
    _(const char* msgid, const char* msgid_plural, unsigned long int n);


    string sformat(const char* format, ...) __attribute__ ((format(printf, 1, 2)));


    vector<string> parse_line(string_view line);

}

#endif
