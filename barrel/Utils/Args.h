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


#include <string>
#include <vector>
#include <initializer_list>


using namespace std;


namespace barrel
{

    class Args
    {

    public:

	Args(std::initializer_list<string> init)
	{
	    tmp.push_back(strdup("barrel"));

	    for (const string& s : init)
		tmp.push_back(strdup(s.c_str()));

	    tmp.push_back(nullptr);
	}

	Args(const vector<string>& init)
	{
	    tmp.push_back(strdup("barrel"));

	    for (const string& s : init)
		tmp.push_back(strdup(s.c_str()));

	    tmp.push_back(nullptr);
	}

	~Args()
	{
	    for (char* p : tmp)
		free(p);
	}

	int argc() const { return tmp.size() - 1; }
	char** argv() { return tmp.data(); }

    private:

	vector<char*> tmp;

    };

}
