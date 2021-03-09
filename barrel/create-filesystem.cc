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


#include <boost/algorithm/string.hpp>

#include <storage/Storage.h>
#include <storage/Pool.h>
#include <storage/Devices/BlkDevice.h>
#include <storage/Devices/PartitionTable.h>
#include <storage/Devices/Partitionable.h>
#include <storage/Devices/Partition.h>
#include <storage/Filesystems/MountPoint.h>

#include "Utils/GetOpts.h"
#include "Utils/Misc.h"
#include "create-filesystem.h"


namespace barrel
{

    using namespace storage;


    namespace
    {

	const map<string, FsType> str_to_fs_type = {
	    { "btrfs", FsType::BTRFS },
	    { "exfat", FsType::EXFAT },
	    { "ext2", FsType::EXT2 },
	    { "ext3", FsType::EXT3 },
	    { "ext4", FsType::EXT4 },
	    { "swap", FsType::SWAP },
	    { "vfat", FsType::VFAT },
	    { "xfs", FsType::XFS }
	};


	struct Options
	{
	    Options(GetOpts& get_opts);

	    optional<FsType> type;
	    optional<string> label;
	    optional<string> path;
	    optional<vector<string>> mount_options;
	    optional<string> pool;
	    optional<SmartSize> size;

	    enum class ModusOperandi { BLK_DEVICE, POOL, PARTITION_TABLE };

	    ModusOperandi modus_operandi;

	    void calculate_modus_operandi();
	};


	Options::Options(GetOpts& get_opts)
	{
	    const vector<Option> options = {
		{ "type", required_argument, 't' },
		{ "label", required_argument, 'l' },
		{ "path", required_argument, 'p' },
		{ "mount-options", required_argument, 'o' },
		{ "pool", required_argument },
		{ "size", required_argument, 's' }
	    };

	    ParsedOpts parsed_opts = get_opts.parse("filesystem", options);

	    if (parsed_opts.has_option("type"))
	    {
		string str = parsed_opts.get("type");

		map<string, FsType>::const_iterator it = str_to_fs_type.find(str);
		if (it == str_to_fs_type.end())
		    throw runtime_error("unknown filesystem type");

		type = it->second;
	    }

	    label = parsed_opts.get_optional("label");

	    path = parsed_opts.get_optional("path");

	    if (parsed_opts.has_option("mount-options"))
	    {
		string str = parsed_opts.get("mount-options");

		vector<string> tmp;
		boost::split(tmp, str, boost::is_any_of(","), boost::token_compress_on);
		mount_options = tmp;
	    }

	    pool = parsed_opts.get_optional("pool");

	    if (parsed_opts.has_option("size"))
	    {
		string str = parsed_opts.get("size");
		size = SmartSize(str);
	    }

	    calculate_modus_operandi();
	}


	void
	Options::calculate_modus_operandi()
	{
	    if (!size)
	    {
		if (pool)
		    throw runtime_error("pool argument not allowed");

		modus_operandi = ModusOperandi::BLK_DEVICE;
	    }
	    else
	    {
		if (pool)
		    modus_operandi = ModusOperandi::POOL;
		else
		    modus_operandi = ModusOperandi::PARTITION_TABLE;
	    }
	}

    }


    class CmdCreateFilesystem : public Cmd
    {
    public:

	CmdCreateFilesystem(const Options& options) : options(options) {}

	virtual bool do_backup() const override { return true; }

	virtual void doit(State& state) const override;

    private:

	const Options options;

    };


    void
    CmdCreateFilesystem::doit(State& state) const
    {
	Devicegraph* staging = state.storage->get_staging();

	BlkDevice* blk_device = nullptr;

	switch (options.modus_operandi)
	{
	    case Options::ModusOperandi::BLK_DEVICE:
	    {
		// The block device is taken from the state.

		if (state.stack.empty() || !is_blk_device(state.stack.top(staging)))
		    throw runtime_error("not a block device on stack");

		blk_device = to_blk_device(state.stack.top(staging));
		state.stack.pop();
	    }
	    break;

	    case Options::ModusOperandi::POOL:
	    {
		// Create a block device from the pool.

		Devicegraph* staging = state.storage->get_staging();

		Pool* pool = state.storage->get_pool(options.pool.value());

		SmartSize smart_size = options.size.value();

		unsigned long long size = smart_size.value(pool->max_partition_size(staging, 1));

		blk_device = pool->create_partitions(staging, 1, size)[0];
	    }
	    break;

	    case Options::ModusOperandi::PARTITION_TABLE:
	    {
		if (state.stack.empty() || !is_partition_table(state.stack.top(staging)))
		    throw runtime_error("not a partition table on stack");

		PartitionTable* partition_table = to_partition_table(state.stack.top(staging));
		state.stack.pop();

		Devicegraph* staging = state.storage->get_staging();

		Pool pool;
		pool.add_device(partition_table->get_partitionable());

		SmartSize smart_size = options.size.value();

		unsigned long long size = smart_size.value(pool.max_partition_size(staging, 1));

		blk_device = pool.create_partitions(staging, 1, size)[0];
	    }
	    break;
	}

	FsType fs_type = options.type.value();

	BlkFilesystem* blk_filesystem = blk_device->create_blk_filesystem(fs_type);

	if (options.label)
	{
	    blk_filesystem->set_label(options.label.value());
	}

	if (options.path)
	{
	    string path = options.path.value();
	    MountPoint* mount_point = blk_filesystem->create_mount_point(path);

	    if (options.mount_options)
	    {
		mount_point->set_mount_options(options.mount_options.value());
	    }
	}

	state.stack.push(blk_filesystem);
	state.modified = true;
    }


    shared_ptr<Cmd>
    parse_create_filesystem(GetOpts& get_opts)
    {
	Options options(get_opts);

	if (!options.type)
	    throw runtime_error("filesystem type missing");

	return make_shared<CmdCreateFilesystem>(options);
    }


    shared_ptr<Cmd>
    parse_create_filesystem(GetOpts& get_opts, FsType type)
    {
	Options options(get_opts);

	if (options.type)
	    throw runtime_error("filesystem type already set");

	options.type = type;

	return make_shared<CmdCreateFilesystem>(options);
    }


    shared_ptr<Cmd>
    parse_create_btrfs(GetOpts& get_opts)
    {
	return parse_create_filesystem(get_opts, FsType::BTRFS);
    }


    shared_ptr<Cmd>
    parse_create_ext2(GetOpts& get_opts)
    {
	return parse_create_filesystem(get_opts, FsType::EXT2);
    }


    shared_ptr<Cmd>
    parse_create_ext3(GetOpts& get_opts)
    {
	return parse_create_filesystem(get_opts, FsType::EXT3);
    }


    shared_ptr<Cmd>
    parse_create_ext4(GetOpts& get_opts)
    {
	return parse_create_filesystem(get_opts, FsType::EXT4);
    }


    shared_ptr<Cmd>
    parse_create_swap(GetOpts& get_opts)
    {
	return parse_create_filesystem(get_opts, FsType::SWAP);
    }


    shared_ptr<Cmd>
    parse_create_xfs(GetOpts& get_opts)
    {
	return parse_create_filesystem(get_opts, FsType::XFS);
    }

}
