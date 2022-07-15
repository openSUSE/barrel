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

	data.push_front(data.front().get()->copy());
    }


    void
    Stack::exch()
    {
	if (data.size() < 2)
	    throw runtime_error(_("stackunderflow during exch"));

	swap(data[0], data[1]);
    }


    const StackObject::Base*
    Stack::top() const
    {
	if (data.empty())
	    throw runtime_error(_("stack empty"));

	return data.front().get();
    }


    void
    Stack::push(unique_ptr<const StackObject::Base>&& stack_object)
    {
	data.push_front(std::move(stack_object));
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
	data.push_front(make_unique<StackObject::Sid>(device->get_sid()));
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
	    return "deleted";

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

}
