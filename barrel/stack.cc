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
#include <storage/Devices/Md.h>
#include <storage/Devices/LvmVg.h>
#include <storage/Devices/LvmLv.h>
#include <storage/Devices/Gpt.h>
#include <storage/Devices/Msdos.h>
#include <storage/Devices/Luks.h>
#include <storage/Filesystems/BlkFilesystem.h>

#include "Utils/Text.h"
#include "stack.h"


namespace barrel
{

    using namespace std;
    using namespace storage;


    Stack::Stack(const Stack& stack)
    {
	for (const Stack::value_type& object : stack.objects)
	    objects.push_back(object->copy());
    }


    Stack
    Stack::operator=(Stack&& stack)
    {
	objects = std::move(stack.objects);

	return *this;
    }


    void
    Stack::pop()
    {
	if (objects.empty())
	    throw runtime_error(_("stack empty during pop"));

	objects.pop_front();
    }


    void
    Stack::dup()
    {
	if (objects.empty())
	    throw runtime_error(_("stack empty during dup"));

	objects.push_front(objects.front().get()->copy());
    }


    void
    Stack::exch()
    {
	if (objects.size() < 2)
	    throw runtime_error(_("stackunderflow during exch"));

	swap(objects[0], objects[1]);
    }


    const StackObject::Base*
    Stack::top() const
    {
	if (objects.empty())
	    throw runtime_error(_("stack empty"));

	return objects.front().get();
    }


    void
    Stack::push(unique_ptr<const StackObject::Base>&& stack_object)
    {
	objects.push_front(std::move(stack_object));
    }


    Device*
    Stack::top_as_device(Devicegraph* devicegraph) const
    {
	const StackObject::Base* base = top();

	const StackObject::Sid* sid = dynamic_cast<const StackObject::Sid*>(base);
	if (!sid)
	    throw runtime_error(_("not a device on stack"));

	return sid->get_device(devicegraph);
    }


    void
    Stack::push(Device* device)
    {
	objects.push_front(make_unique<StackObject::Sid>(device->get_sid()));
    }


    vector<BlkDevice*>
    Stack::top_as_blk_devices(Devicegraph* devicegraph) const
    {
	const StackObject::Base* base = top();

	const StackObject::Sid* sid = dynamic_cast<const StackObject::Sid*>(base);
	if (sid)
	{
	    Device* device = sid->get_device(devicegraph);
	    if (!is_blk_device(device))
		throw runtime_error(_("not a block device on stack"));

	    return { to_blk_device(device) };
	}

	const StackObject::Array* array = dynamic_cast<const StackObject::Array*>(base);
	if (array)
	{
	    vector<BlkDevice*> blk_devices;

	    for (Device* device : array->get_devices(devicegraph))
	    {
		if (!is_blk_device(device))
		    throw runtime_error(_("not a block device in array"));

		blk_devices.push_back(to_blk_device(device));
	    }

	    return blk_devices;
	}

	throw runtime_error(_("not a block device or array of block devices on stack"));
    }


    vector<PartitionTable*>
    Stack::top_as_partition_tables(Devicegraph* devicegraph) const
    {
	const StackObject::Base* base = top();

	const StackObject::Sid* sid = dynamic_cast<const StackObject::Sid*>(base);
	if (sid)
	{
	    Device* device = sid->get_device(devicegraph);
	    if (!is_partition_table(device))
		throw runtime_error(_("not a partition table on stack"));

	    return { to_partition_table(device) };
	}

	const StackObject::Array* array = dynamic_cast<const StackObject::Array*>(base);
	if (array)
	{
	    vector<PartitionTable*> partition_tables;

	    for (Device* device : array->get_devices(devicegraph))
	    {
		if (!is_partition_table(device))
		    throw runtime_error(_("not a partition table in array"));

		partition_tables.push_back(to_partition_table(device));
	    }

	    return partition_tables;
	}

	throw runtime_error(_("not a partition table or array of partition tables on stack"));
    }


    Stack::const_iterator
    Stack::find_mark() const
    {
	return find_if(begin(), end(), [](const value_type& tmp) {
	    return dynamic_cast<const StackObject::Mark*>(tmp.get());
	});
    }


    void
    Stack::open_mark()
    {
	objects.push_front(make_unique<StackObject::Mark>());
    }


    void
    Stack::close_mark()
    {
	const_iterator mark = find_mark();
	if (mark == end())
	    throw runtime_error(_("no mark on stack"));

	Stack::objects_type tmp;

#if __GNUC__ >= 11
	std::move(begin(), mark, front_inserter(tmp));
#else
	for (const_iterator it = begin(); it != mark; ++it)
	    tmp.push_front(it->get()->copy());
#endif

	objects.erase(begin(), ++mark);

	objects.push_front(make_unique<StackObject::Array>(std::move(tmp)));
    }


    void
    StackObject::Base::print(const Devicegraph* devicegraph, Table::Row& row) const
    {
	row[Id::DESCRIPTION] = print(devicegraph);
    }


    unique_ptr<StackObject::Base>
    StackObject::Sid::copy() const
    {
	return make_unique<Sid>(sid);
    }


    string
    StackObject::Sid::print(const Devicegraph* devicegraph) const
    {
	if (!devicegraph->device_exists(sid))
	    return _("deleted device");

	const Device* device = get_device(devicegraph);

	if (is_md(device))
	{
	    const Md* md = to_md(device);
	    return sformat(_("RAID %s"), md->get_name().c_str());
	}

	if (is_lvm_vg(device))
	{
	    const LvmVg* lvm_vg = to_lvm_vg(device);
	    return sformat(_("LVM volume group %s"), lvm_vg->get_vg_name().c_str());
	}

	if (is_lvm_lv(device))
	{
	    const LvmLv* lvm_lv = to_lvm_lv(device);
	    return sformat(_("LVM logical volume %s"), lvm_lv->get_name().c_str());
	}

	if (is_gpt(device))
	{
	    const Gpt* gpt = to_gpt(device);
	    return sformat(_("GPT on %s"), gpt->get_partitionable()->get_name().c_str());
	}

	if (is_msdos(device))
	{
	    const Msdos* msdos = to_msdos(device);
	    return sformat(_("MS-DOS on %s"), msdos->get_partitionable()->get_name().c_str());
	}

	if (is_luks(device))
	{
	    const Luks* luks = to_luks(device);
	    return sformat(_("LUKS on %s"), luks->get_blk_device()->get_name().c_str());
	}

	if (is_blk_filesystem(device))
	{
	    const BlkFilesystem* blk_filesystem = to_blk_filesystem(device);
	    // TODO support several devices, maybe use join from libstorage-ng
	    return sformat(_("filesystem %s on %s"), get_fs_type_name(blk_filesystem->get_type()).c_str(),
			   blk_filesystem->get_blk_devices()[0]->get_name().c_str());
	}

	return "unknown device type";
    }


    Device*
    StackObject::Sid::get_device(Devicegraph* devicegraph) const
    {
	return devicegraph->find_device(sid);
    }


    const Device*
    StackObject::Sid::get_device(const Devicegraph* devicegraph) const
    {
	return devicegraph->find_device(sid);
    }


    unique_ptr<StackObject::Base>
    StackObject::Integer::copy() const
    {
	return make_unique<Integer>(i);
    }


    string
    StackObject::Integer::print(const Devicegraph* devicegraph) const
    {
	return to_string(i);
    }


    unique_ptr<StackObject::Base>
    StackObject::Mark::copy() const
    {
	return make_unique<Mark>();
    }


    string
    StackObject::Mark::print(const Devicegraph* devicegraph) const
    {
	return "mark";
    }


    unique_ptr<StackObject::Base>
    StackObject::Array::copy() const
    {
	Stack::objects_type tmp;

	for (const Stack::value_type& object : objects)
	    tmp.push_back(object->copy());

	return make_unique<Array>(std::move(tmp));
    }


    string
    StackObject::Array::print(const Devicegraph* devicegraph) const
    {
	size_t n = objects.size();

	return sformat(_("array with %d object", "array with %d objects", n), n);
    }


    void
    StackObject::Array::print(const Devicegraph* devicegraph, Table::Row& row) const
    {
	row[Id::DESCRIPTION] = print(devicegraph);

	for (const Stack::value_type& object : objects)
	{
	    Table::Row subrow(row.get_table());
	    object->print(devicegraph, subrow);
	    row.add_subrow(subrow);
	}
    }


    vector<Device*>
    StackObject::Array::get_devices(Devicegraph* devicegraph) const
    {
	vector<Device*> ret;

	for (const Stack::value_type& object : objects)
	{
	    const StackObject::Sid* sid = dynamic_cast<const StackObject::Sid*>(object.get());
	    if (!sid)
		throw runtime_error(_("not a device in array"));

	    ret.push_back(sid->get_device(devicegraph));
	}

	return ret;
    }

}
