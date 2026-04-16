/*
 * Copyright (c) [2021-2022] SUSE LLC
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


#include <termios.h>
#include <unistd.h>
#include <iostream>

#include "Prompt.h"
#include "Text.h"
#include "Mockup.h"


namespace barrel
{

    bool
    prompt(const string& message)
    {
	while (true)
	{
	    // TRANSLATORS: Abbreviation for "yes". Translation must be different from
	    // translation for "n" ("no").
	    string y = _("y");

	    // TRANSLATORS: Abbreviation for "no". Translation must be different from
	    // translation for "y" ("yes").
	    string n = _("n");

	    cout << message << " [" << y << "/" << n << "] " << flush;

	    if (cin.eof())	// TODO
		return false;

	    string reply;
	    cin >> reply;

	    if (reply == y)
		return true;
	    else if (reply == n)
		return false;

	    cout << sformat(_("Invalid answer '%s'"), reply.c_str()) << '\n';
	}
    }


    string
    prompt_password(bool verify)
    {
	if (mockup)
	    return "mockup";

	while (true)
	{
	    struct termios old_termios;
	    const bool is_tty = tcgetattr(STDIN_FILENO, &old_termios) == 0;
	    if (is_tty)
	    {
		struct termios new_termios = old_termios;
		new_termios.c_lflag &= ~(ECHO);
		tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
	    }

	    int c;

	    cout << _("Enter password:") << " " << flush;
	    string password1;
	    while ((c = getchar()) != '\n' && c != EOF)
		password1.push_back(c);
	    cout << '\n';

	    string password2;
	    if (verify)
	    {
		cout << _("Verify password:") << " " << flush;
		while ((c = getchar()) != '\n' && c != EOF)
		    password2.push_back(c);
		cout << '\n';
	    }

	    if (is_tty)
	    {
		tcsetattr(STDIN_FILENO, TCSANOW, &old_termios);
	    }

	    if (verify && password1 != password2)
	    {
		cout << _("Passwords do not match.") << '\n';
		continue;
	    }

	    return password1;
	}
    }

}
