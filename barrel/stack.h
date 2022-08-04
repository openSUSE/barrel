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

#include <storage/Devices/BlkDevice.h>
#include <storage/Devices/PartitionTable.h>

#include "Utils/Table.h"


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

	Stack() = default;
	Stack(const Stack&);
	Stack(Stack&&) = default;

	Stack operator=(Stack&&);

	bool empty() const { return objects.empty(); }
	size_t size() const { return objects.size(); }

	using objects_type = deque<unique_ptr<const StackObject::Base>>;
	using value_type = objects_type::value_type;
	using const_iterator = objects_type::const_iterator;

	const_iterator begin() const { return objects.begin(); }
	const_iterator end() const { return objects.end(); }

	void pop();
	void clear() { objects.clear(); }
	void dup();
	void exch();

	const_iterator find_mark() const;
	void open_mark();
	void close_mark();

	const StackObject::Base* top() const;
	void push(unique_ptr<const StackObject::Base>&& stack_object);

	Device* top_as_device(Devicegraph* devicegraph) const;
	void push(Device* device);

	vector<BlkDevice*> top_as_blk_devices(Devicegraph* devicegraph) const;
	vector<PartitionTable*> top_as_partition_tables(Devicegraph* devicegraph) const;

    private:

	objects_type objects;

    };


    namespace StackObject
    {

	class Base
	{
	public:

	    virtual ~Base() = default;

	    virtual unique_ptr<Base> copy() const = 0;
	    virtual string print(const Devicegraph* devicegraph) const = 0;
	    virtual void print(const Devicegraph* devicegraph, Table::Row& row) const;

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


	class Mark : public Base
	{
	public:

	    virtual unique_ptr<Base> copy() const override;
	    virtual string print(const Devicegraph* devicegraph) const override;

	};


	class Array : public Base
	{
	public:

	    Array(Stack::objects_type&& objects) : objects(std::move(objects)) {}

	    virtual unique_ptr<Base> copy() const override;
	    virtual string print(const Devicegraph* devicegraph) const override;
	    virtual void print(const Devicegraph* devicegraph, Table::Row& row) const override;

	    vector<Device*> get_devices(Devicegraph* devicegraph) const;

	private:

	    const Stack::objects_type objects;

	};

    }

}

#endif
