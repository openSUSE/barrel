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


#ifndef BARREL_STACK_H
#define BARREL_STACK_H


#include <deque>

#include <storage/Devices/Device.h>


namespace barrel
{

    using namespace std;
    using namespace storage;


    class Stack
    {
    public:

	bool empty() const { return data.empty(); }
	size_t size() const { return data.size(); }

	using data_type = deque<sid_t>;
	using const_iterator = data_type::const_iterator;

	const_iterator begin() const { return data.begin(); }
	const_iterator end() const { return data.end(); }

	void pop() { data.pop_front(); }
	void clear() { data.clear(); }
	void dup() { data.push_front(data.front()); }

	Device* top(Devicegraph* devicegraph);
	void push(Device* device);

    private:

	data_type data;

    };

}

#endif
