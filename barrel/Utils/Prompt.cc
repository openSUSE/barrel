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


#include <iostream>

#include "Prompt.h"
#include "Text.h"


namespace barrel
{

    bool
    prompt(const string& message)
    {
	while (true)
	{
	    cout << message + " ["s + _("y/n") + "] "s << flush;

	    string reply;
	    cin >> reply;

	    if (reply == "y")
		return true;
	    else if (reply == "n")
		return false;

	    cout << sformat(_("Invalid answer '%s'"), reply.c_str()) << '\n';
	}
    }

}
