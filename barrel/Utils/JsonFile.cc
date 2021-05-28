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


#include <functional>
#include <memory>
#include <stdexcept>
#include <fstream>

#include "JsonFile.h"


namespace barrel
{

    JsonFile::JsonFile(const string& filename)
	: root(nullptr)
    {
	std::unique_ptr<json_tokener, std::function<void(json_tokener*)>> tokener(
	    json_tokener_new(), [](json_tokener* p) { json_tokener_free(p); }
	);

	ifstream file(filename);
	if (!file.good())
	    throw runtime_error("opening json file failed");

	file.unsetf(ifstream::skipws);

	string line;
	getline(file, line);
	while (file.good())
	{
	    root = json_tokener_parse_ex(tokener.get(), line.c_str(), line.size());

	    json_tokener_error jerr = json_tokener_get_error(tokener.get());
	    if (jerr != json_tokener_success && jerr != json_tokener_continue)
		throw runtime_error("json parser failed");

	    getline(file, line);
	}
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
