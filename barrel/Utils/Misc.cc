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


#include <regex>
#include <vector>
#include <stdexcept>
#include <algorithm>

#include <storage/Devices/Partition.h>
#include <storage/Devices/Partitionable.h>
#include <storage/Utils/HumanString.h>

#include "Misc.h"
#include "Text.h"
#include "BarrelTmpl.h"
#include "BarrelDefines.h"


namespace barrel
{

    using namespace std;
    using namespace storage;


    string
    format_size(unsigned long long size, bool omit_zeroes)
    {
	return byte_to_humanstring(size, false, 2, omit_zeroes);
    }


    string
    format_percentage(unsigned long long a, unsigned long long b, int precision)
    {
	if (b == 0)
	    return "";

	return sformat("%.*f%%", precision, 100.0 * (double)(a) / (double)(b));
    }


    SmartNumber::SmartNumber(const string& str)
    {
	static const regex absolute_rx("([0-9]+)", regex::extended);

	if (str == "max")
	{
	    type = MAX;
	}
	else
	{
	    type = ABSOLUTE;

	    smatch match;

	    if (!regex_match(str, match, absolute_rx))
		throw runtime_error(_("bad devices argument"));

	    string n1 = match[1];
	    absolute = atoi(n1.c_str());

	    if (absolute == 0)
		throw runtime_error(sformat(_("invalid devices value '%d'"), absolute));
	}
    }


    unsigned int
    SmartNumber::value(unsigned int max) const
    {
	switch (type)
	{
	    case SmartNumber::MAX:
		return max;

	    case SmartNumber::ABSOLUTE:
		return absolute;
	}

	throw logic_error("unknown SmartNumber type");
    }


    SmartSize::SmartSize(const string& str)
    {
	if (str == "max")
	{
	    type = MAX;
	}
	else
	{
	    type = ABSOLUTE;

	    try
	    {
		absolute = humanstring_to_byte(str, false);
	    }
	    catch (...)
	    {
		throw runtime_error(sformat(_("failed to parse size '%s'"), str.c_str()));
	    }

	    if (absolute == 0)
		throw runtime_error(sformat(_("invalid size '%s'"), str.c_str()));
	}
    }


    unsigned long long
    SmartSize::value(unsigned long long max) const
    {
	switch (type)
	{
	    case SmartSize::MAX:
		return max;

	    case SmartSize::ABSOLUTE:
		return absolute;
	}

	throw logic_error("unknown SmartSize type");
    }


    BlkDevice*
    PartitionCreator::create_partition(const Pool* pool, Devicegraph* devicegraph, const SmartSize& smart_size)
    {
	if (pool->empty(devicegraph))
	    throw runtime_error(_("pool is empty"));

	unsigned long long size = smart_size.value(pool->max_partition_size(devicegraph, 1));

	return pool->create_partitions(devicegraph, 1, size)[0];
    }


    vector<BlkDevice*>
    PartitionCreator::create_partitions(const Pool* pool, Devicegraph* devicegraph, DefaultNumber default_number,
					const optional<SmartNumber>& smart_number,
					const SmartSize& smart_size)
    {
	if (pool->empty(devicegraph))
	    throw runtime_error(_("pool is empty"));

	unsigned int number = default_number == ONE ? 1 : pool->size(devicegraph);

	if (smart_number)
	    number = smart_number.value().value(pool->size(devicegraph));

	unsigned long long size = 0;

	switch (smart_size.type)
	{
	    case SmartSize::MAX:
		size = pool->max_partition_size(devicegraph, number);
		break;

	    case SmartSize::ABSOLUTE:
		size = smart_size.absolute / number + 1 * MiB;
		break;
	}

	return up_cast<BlkDevice*>(pool->create_partitions(devicegraph, number, size));
    }


    vector<string>
    possible_blk_devices(const Storage* storage)
    {
	vector<string> blk_devices;

	for (const BlkDevice* blk_device : BlkDevice::get_all(storage->get_staging()))
	{
	    blk_devices.push_back(blk_device->get_name());

	    for (const string& tmp : blk_device->get_udev_paths())
		blk_devices.push_back(DEV_DISK_BY_PATH_DIR "/" + tmp);
	    for (const string& tmp : blk_device->get_udev_ids())
		blk_devices.push_back(DEV_DISK_BY_ID_DIR "/" + tmp);
	}

	sort(blk_devices.begin(), blk_devices.end());

	return blk_devices;
    }


    void
    check_usable(BlkDevice* blk_device, bool force)
    {
	if (!blk_device->is_usable_as_blk_device())
	{
	    throw runtime_error(sformat(_("block device '%s' cannot be used as a regular block device"),
					blk_device->get_name().c_str()));
	}

	if (blk_device->has_children())
	{
	    if (force)
		blk_device->remove_descendants(View::REMOVE);
	    else
		throw runtime_error(sformat(_("block device '%s' is in use"), blk_device->get_name().c_str()));
	}
    }


    void
    check_usable(vector<BlkDevice*>& blk_devices, bool force)
    {
	for (BlkDevice* blk_device : blk_devices)
	    check_usable(blk_device, force);
    }


    void
    check_usable(Partitionable* partitionable, bool force)
    {
	if (!partitionable->is_usable_as_partitionable())
	{
	    throw runtime_error(sformat(_("partitionable '%s' cannot be used as a regular partitionable"),
					partitionable->get_name().c_str()));
	}

	if (partitionable->has_children())
	{
	    if (force)
		partitionable->remove_descendants(View::REMOVE);
	    else
		throw runtime_error(sformat(_("partitionable '%s' is in use"), partitionable->get_name().c_str()));
	}
    }


    void
    remove_pools(Storage* storage)
    {
	for (const map<string, Pool*>::value_type& value : storage->get_pools())
	{
	    storage->remove_pool(value.first);
	}
    }


    void
    pimp_pool(Pool* pool, const BlkDevice* blk_device)
    {
	// TODO make this configurable

	const vector<string>& udev_ids = blk_device->get_udev_ids();
	if (!udev_ids.empty())
	{
	    string name = DEV_DISK_BY_ID_DIR "/" + udev_ids.front();

	    map<string, string> userdata = pool->get_userdata();
	    userdata[sformat("sid-%d-name", blk_device->get_sid())] = name;
	    pool->set_userdata(userdata);

	    return;
	}

	const vector<string>& udev_paths = blk_device->get_udev_paths();
	if (!udev_paths.empty())
	{
	    string name = DEV_DISK_BY_PATH_DIR "/" + udev_paths.front();

	    map<string, string> userdata = pool->get_userdata();
	    userdata[sformat("sid-%d-name", blk_device->get_sid())] = name;
	    pool->set_userdata(userdata);

	    return;
	}
    }


    StagingGuard::StagingGuard(Storage* storage)
	: storage(storage)
    {
	storage->copy_devicegraph("staging", name);
    }


    StagingGuard::~StagingGuard()
    {
	if (storage->exist_devicegraph(name))
	    storage->restore_devicegraph(name);
    }


    void
    StagingGuard::release()
    {
	storage->remove_devicegraph(name);
    }


    StackGuard::StackGuard(Stack& stack)
       : stack(stack), backup(stack)
    {
    }


    StackGuard::~StackGuard()
    {
	if (released)
	    return;

	stack = std::move(backup);
    }


    void
    StackGuard::release()
    {
	released = true;
    }

}
