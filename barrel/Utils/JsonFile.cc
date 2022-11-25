/*
 * Copyright (c) [2017-2021] SUSE LLC
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
#include <sys/stat.h>

#include <stdexcept>

#include "JsonFile.h"
#include "Text.h"


namespace barrel
{

    JsonFile::JsonFile()
	: root(json_object_new_object())
    {
    }


    JsonFile::JsonFile(const string& filename)
    {
	FILE* fp = fopen(filename.c_str(), "r");
	if (!fp)
	    // TRANSLATORS: error message, 'open' refers to open system call
	    throw runtime_error(sformat(_("open for json file '%s' failed"), filename.c_str()));

	struct stat st;
	if (fstat(fileno(fp), &st) != 0)
	{
	    fclose(fp);
	    // TRANSLATORS: error message, 'stat' refers to stat system call
	    throw runtime_error(sformat(_("stat for json file '%s' failed"), filename.c_str()));
	}

	vector<char> data(st.st_size);
	if (fread(data.data(), 1, st.st_size, fp) != (size_t)(st.st_size))
	{
	    fclose(fp);
	    // TRANSLATORS: error message, 'read' refers to read system call
	    throw runtime_error(sformat(_("read for json file '%s' failed"), filename.c_str()));
	}

	if (fclose(fp) != 0)
	    // TRANSLATORS: error message, 'close' refers to close system call
	    throw runtime_error(sformat(_("close for json file '%s' failed"), filename.c_str()));

	json_tokener* tokener = json_tokener_new();

	root = json_tokener_parse_ex(tokener, data.data(), st.st_size);

	if (json_tokener_get_error(tokener) != json_tokener_success)
	{
	    json_tokener_free(tokener);
	    json_object_put(root);
	    throw runtime_error(sformat(_("parsing json file '%s' failed"), filename.c_str()));
	}

	if (tokener->char_offset != st.st_size)
	{
	    json_tokener_free(tokener);
	    json_object_put(root);
	    throw runtime_error(sformat(_("excessive content in json file '%s'"), filename.c_str()));
	}

	json_tokener_free(tokener);
    }


    JsonFile::~JsonFile()
    {
	json_object_put(root);
    }


    void
    JsonFile::save(const string& filename) const
    {
	FILE* fp = fopen(filename.c_str(), "w");
	if (!fp)
	    throw runtime_error(sformat(_("opening json file '%s' failed"), filename.c_str()));

	const int flags = JSON_C_TO_STRING_PRETTY | JSON_C_TO_STRING_SPACED |
	    JSON_C_TO_STRING_NOSLASHESCAPE;

	fprintf(fp, "%s\n", json_object_to_json_string_ext(root, flags));

	if (fclose(fp) != 0)
	    throw runtime_error(sformat(_("closing json file '%s' failed"), filename.c_str()));
    }

}
