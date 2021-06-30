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


#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <stdexcept>

#include "JsonFile.h"
#include "Text.h"


namespace barrel
{

    JsonFile::JsonFile(const string& filename)
	: root(nullptr)
    {
	int fd = open(filename.c_str(), O_RDONLY);
	if (fd < 0)
	{
	    throw runtime_error(sformat("opening json file '%s' failed", filename.c_str()));
	}

	struct stat st;
	if (fstat(fd, &st) < 0)
	{
	    close(fd);
	    throw runtime_error(sformat("stat for json file '%s' failed", filename.c_str()));
	}

	void* data = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (!data)
	{
	    close(fd);
	    throw runtime_error(sformat("mmap for json '%s' file failed", filename.c_str()));
	}

	json_tokener* tokener = json_tokener_new();
	root = json_tokener_parse_ex(tokener, (const char*) data, st.st_size);

	munmap(data, st.st_size);
	close(fd);

	if (json_tokener_get_error(tokener) != json_tokener_success)
	{
	    json_tokener_free(tokener);
	    throw runtime_error(sformat("parsing json file '%s' failed", filename.c_str()));
	}

	// excessive content
	if (tokener->char_offset != st.st_size)
	{
	    json_tokener_free(tokener);
	    throw runtime_error(sformat("excessive content in json file '%s'", filename.c_str()));
	}

	json_tokener_free(tokener);
    }


    JsonFile::~JsonFile()
    {
	json_object_put(root);
    }


    bool
    get_child_value(json_object* parent, const char* name, map<string, string>& value)
    {
	json_object* tmp;
        json_object_object_get_ex(parent, name, &tmp);

	if (!json_object_is_type(tmp, json_type_object))
	    return false;

	value.clear();

	json_object_object_foreach(tmp, k, v)
	{
	    if (!json_object_is_type(v, json_type_string))
		return false;

	    value[k] = json_object_get_string(v);
	}

	return true;
    }

}
