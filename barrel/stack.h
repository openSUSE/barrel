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


    namespace StackObject
    {
	class Base;
    }


    class Stack
    {
    public:

	bool empty() const { return data.empty(); }
	size_t size() const { return data.size(); }

	using data_type = deque<unique_ptr<const StackObject::Base>>;
	using value_type = data_type::value_type;
	using const_iterator = data_type::const_iterator;

	const_iterator begin() const { return data.begin(); }
	const_iterator end() const { return data.end(); }

	void pop();
	void clear() { data.clear(); }
	void dup();
	void exch();

	const StackObject::Base* top() const;
	void push(unique_ptr<const StackObject::Base>&& stack_object);

	Device* top_as_device(Devicegraph* devicegraph) const;
	void push(Device* device);

    private:

	data_type data;

    };


    namespace StackObject
    {

	class Base
	{
	public:

	    virtual ~Base() = default;

	    virtual unique_ptr<Base> copy() const = 0;
	    virtual string print(const Devicegraph* devicegraph) const = 0;

	};


	class Sid : public Base
	{
	public:

	    Sid(sid_t sid) : sid(sid) {}

	    virtual unique_ptr<Base> copy() const override;
	    virtual string print(const Devicegraph* devicegraph) const override;

	    Device* get_device(Devicegraph* devicegraph) const;
	    const Device* get_device(const Devicegraph* devicegraph) const;

	private:

	    const sid_t sid;

	};


	class Integer : public Base
	{
	public:

	    Integer(int i) : i(i) {}

	    virtual unique_ptr<Base> copy() const override;
	    virtual string print(const Devicegraph* devicegraph) const override;

	private:

	    const int i;

	};

    }

}

#endif
