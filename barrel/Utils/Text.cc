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
#include <string.h>
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


    int
    mbs_width_e(const string& str)
    {
	// from snapper, zypper, smpppd

	const char* ptr = str.c_str();
	size_t s_bytes = str.length();
	int s_cols = 0;
	bool in_ctrlseq = false;

	mbstate_t shift_state;
	memset(&shift_state, 0, sizeof(shift_state));

	wchar_t wc;
	size_t c_bytes;

	// mbrtowc produces one wide character from a multibyte string
	while ((c_bytes = mbrtowc(&wc, ptr, s_bytes, &shift_state)) > 0)
	{
	    if (c_bytes >= (size_t) -2) // incomplete (-2) or invalid (-1) sequence
		return -1;

	    // ignore the length of terminal control sequences in order
	    // to compute the length of colored text correctly
	    if (!in_ctrlseq && ::wcsncmp(&wc, L"\033", 1) == 0)
		in_ctrlseq = true;
	    else if (in_ctrlseq && ::wcsncmp(&wc, L"m", 1) == 0)
		in_ctrlseq = false;
	    else if (!in_ctrlseq)
		s_cols += ::wcwidth(wc);

	    s_bytes -= c_bytes;
	    ptr += c_bytes;

	    // end of string
	    if (s_bytes == 0)
		break;
	}

	return s_cols;
    }


    size_t
    mbs_width(const string& str)
    {
	int c = mbs_width_e(str);
	if (c < 0)
	    return str.length();        // fallback if there was an error
	else
	    return (size_t) c;
    }

}
