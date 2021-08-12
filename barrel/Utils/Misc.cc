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


#include <stdexcept>
#include <algorithm>

#include <storage/Devices/BlkDevice.h>
#include <storage/Utils/HumanString.h>

#include "Misc.h"
#include "Text.h"
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
    format_percentage(unsigned long long a, unsigned long long b)
    {
	if (b == 0)
	    return "";

	return sformat("%.2f%%", 100.0 * (double)(a) / (double)(b));
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

	    absolute = humanstring_to_byte(str, false);
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

	    default:
		throw runtime_error("unknown SmartSize type");
	}
    }


    vector<string>
    possible_blk_devices(const Storage* storage)
    {
	vector<string> blk_devices;

	for (auto x : BlkDevice::get_all(storage->get_staging()))
	{
	    blk_devices.push_back(x->get_name());

	    for (const string& t : x->get_udev_paths())
		blk_devices.push_back(DEV_DISK_BY_PATH_DIR "/" + t);
	    for (const string& t : x->get_udev_ids())
		blk_devices.push_back(DEV_DISK_BY_ID_DIR "/" + t);
	}

	sort(blk_devices.begin(), blk_devices.end());

	return blk_devices;
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

}
