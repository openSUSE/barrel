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


#ifndef BARREL_JSON_FILE_H
#define BARREL_JSON_FILE_H


#include <json-c/json.h>
#include <string>
#include <boost/noncopyable.hpp>


namespace barrel
{
    using namespace std;


    class JsonFile : private boost::noncopyable
    {

    public:

	JsonFile(const string& filename);

	~JsonFile();

	json_object* get_root() { return root; }

    private:

	json_object* root;

    };

}


#endif
