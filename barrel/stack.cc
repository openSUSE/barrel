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


#include <storage/Devicegraph.h>

#include "Utils/Text.h"
#include "stack.h"


namespace barrel
{

    using namespace std;
    using namespace storage;


    void
    Stack::pop()
    {
	if (data.empty())
	    throw runtime_error(_("stack empty during pop"));

	data.pop_front();
    }


    void
    Stack::dup()
    {
	if (data.empty())
	    throw runtime_error(_("stack empty during dup"));

	data.push_front(data.front());
    }


    void
    Stack::exch()
    {
	if (data.size() < 2)
	    throw runtime_error(_("stackunderflow during exch"));

	swap(data[0], data[1]);
    }


    Device*
    Stack::top(Devicegraph* devicegraph)
    {
	if (data.empty())
	    throw runtime_error(_("stack empty"));

	return devicegraph->find_device(data.front());
    }


    void
    Stack::push(Device* device)
    {
	data.push_front(device->get_sid());
    }

}
