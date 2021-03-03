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


#include <stdarg.h>
#include <libintl.h>
#include <stdexcept>

#include "Text.h"


namespace barrel
{

    const char*
    _(const char* msgid)
    {
	return dgettext("barrel", msgid);
    }


    const char*
    _(const char* msgid, const char* msgid_plural, unsigned long int n)
    {
	return dngettext("barrel", msgid, msgid_plural, n);
    }


    string
    sformat(const char* format, ...)
    {
	char* result;
	string str;

	va_list ap;
	va_start(ap, format);
	if (vasprintf(&result, format, ap) != -1)
	{
	    str = result;
	    free(result);
	}
	va_end(ap);

	return str;
    }


    vector<string>
    parse_line(string_view line)
    {
	vector<string> ret;

	enum State { NONE, WORD, BACKSLASH, SINGLEQUOTE, DOUBLEQUOTE } state = NONE;

	string tmp;

	for (char c : line)
	{
	    switch (state)
	    {
		case NONE:
		    if (c == '\'')
		    {
			state = SINGLEQUOTE;
			tmp = "";
		    }
		    else if (c == '"')
		    {
			state = DOUBLEQUOTE;
			tmp = "";
		    }
		    else if (c == '\\')
		    {
			state = BACKSLASH;
			tmp = "";
		    }
		    else if (!isspace(c))
		    {
			state = WORD;
			tmp = c;
		    }
		    break;

		case WORD:
		    if (c == '\'')
		    {
			state = SINGLEQUOTE;
		    }
		    else if (c == '"')
		    {
			state = DOUBLEQUOTE;
		    }
		    else if (c == '\\')
		    {
			state = BACKSLASH;
		    }
		    else if (isspace(c))
		    {
			ret.push_back(tmp);
			state = NONE;
		    }
		    else
		    {
			tmp += c;
		    }
		    break;

		case BACKSLASH:
		    if (c == ' ' || c == '\\')
		    {
			state = WORD;
			tmp += c;
		    }
		    else
		    {
			throw runtime_error("unknown escape sequence");
		    }
		    break;

		case SINGLEQUOTE:
		    if (c == '\'')
		    {
			state = WORD;
		    }
		    else
		    {
			tmp += c;
		    }
		    break;

		case DOUBLEQUOTE:
		    if (c == '"')
		    {
			state = WORD;
		    }
		    else
		    {
			tmp += c;
		    }
		    break;
	    }
	}

	switch (state)
	{
	    case NONE:
		break;

	    case WORD:
		ret.push_back(tmp);
		break;

	    case BACKSLASH:
		throw runtime_error("broken escape sequence");

	    case SINGLEQUOTE:
		throw runtime_error("missing end singlequote");

	    case DOUBLEQUOTE:
		throw runtime_error("missing end doublequote");
	}

	return ret;
    }

}
