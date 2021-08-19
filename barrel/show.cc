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


#include <storage/Devices/BlkDevice.h>
#include <storage/Devices/Partitionable.h>
#include <storage/Devices/Md.h>
#include <storage/Devices/LvmVg.h>
#include <storage/Devices/LvmPv.h>
#include <storage/Pool.h>
#include <storage/Storage.h>

#include "show.h"
#include "Utils/Misc.h"


namespace barrel
{

    using namespace std;


    string
    ParsedCmdShow::device_usage(const Device* device) const
    {
	if (!is_blk_device(device))
	    return "";

	const BlkDevice* blk_device = to_blk_device(device);

	if (blk_device->has_blk_filesystem())
	{
	    const BlkFilesystem* blk_filesystem = blk_device->get_blk_filesystem();
	    return get_fs_type_name(blk_filesystem->get_type());
	}

	if (blk_device->has_encryption())
	{
	    return "encryption";
	}

	if (is_partitionable(blk_device))
	{
	    const Partitionable* partitionable = to_partitionable(blk_device);
	    if (partitionable->has_partition_table())
	    {
		const PartitionTable* partition_table = partitionable->get_partition_table();
		return get_pt_type_name(partition_table->get_type());
	    }
	}

	if (blk_device->has_children())
	{
	    const Device* child = blk_device->get_children()[0];

	    if (is_md(child))
	    {
		return "RAID " + to_md(child)->get_name();
	    }

	    if (is_lvm_pv(child))
	    {
		const LvmPv* lvm_pv = to_lvm_pv(child);

		if (lvm_pv->has_lvm_vg())
		    return "LVM " + lvm_pv->get_lvm_vg()->get_vg_name();
		else
		    return "LVM";
	    }
	}

	return "";
    }


    string
    ParsedCmdShow::device_pool(const Storage* storage, const Device* device) const
    {
	const Devicegraph* probed = storage->get_probed();

	map<string, const Pool*> pools = storage->get_pools();
	for (const map<string, const Pool*>::value_type& value : pools)
	{
	    const Pool* pool = value.second;
	    vector<const Device*> devices = pool->get_devices(probed);

	    for (const Device* tmp : devices)
		if (device->get_sid() == tmp->get_sid())
		    return value.first;
	}

	return "";
    }


    void
    ParsedCmdShow::insert_partitions(const Partitionable* partitionable, Table::Row& row) const
    {
	if (partitionable->has_partition_table())
	{
	    const PartitionTable* partition_table = partitionable->get_partition_table();

	    vector<const Partition*> partitions = partition_table->get_partitions();
	    sort(partitions.begin(), partitions.end(), Partition::compare_by_number);

	    for (const Partition* partition : partitions)
	    {
		Table::Row subrow(row.get_table());

		subrow[Id::NAME] = partition->get_name();
		subrow[Id::SIZE] = format_size(partition->get_size());
		subrow[Id::USAGE] = device_usage(partition);

		row.add_subrow(subrow);
	    }
	}
    }

}
