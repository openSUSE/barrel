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


#include <boost/algorithm/string.hpp>

#include <storage/Storage.h>
#include <storage/Pool.h>
#include <storage/Devices/BlkDevice.h>
#include <storage/Devices/PartitionTable.h>
#include <storage/Devices/Partitionable.h>
#include <storage/Devices/Partition.h>
#include <storage/Filesystems/MountPoint.h>

#include "Utils/GetOpts.h"
#include "Utils/Text.h"
#include "Utils/Misc.h"
#include "create-filesystem.h"


namespace barrel
{

    using namespace storage;


    namespace
    {

	const ExtOptions create_filesystem_options({
	    { "type", required_argument, 't', _("set file system type"), "type" },
	    { "label", required_argument, 'l', _("set file system label"), "label" },
	    { "path", required_argument, 'p', _("mount path"), "path" },
	    { "mount-options", required_argument, 'o', _("mount options"), "options" },
	    { "mkfs-options", required_argument, 0, _("mkfs options"), "options" },
	    { "tune-options", required_argument, 0, _("tune options"), "options" },
	    { "pool-name", required_argument, 0, _("pool name"), "name" },
	    { "size", required_argument, 's', _("set size"), "size" },
	    { "force", no_argument, 0, _("force if block devices are in use") }
	}, TakeBlkDevices::MAYBE);


	const map<string, FsType> str_to_fs_type = {
	    { "btrfs", FsType::BTRFS },
	    { "exfat", FsType::EXFAT },
	    { "ext2", FsType::EXT2 },
	    { "ext3", FsType::EXT3 },
	    { "ext4", FsType::EXT4 },
	    { "f2fs", FsType::F2FS },
	    { "nilfs2", FsType::NILFS2 },
	    { "ntfs", FsType::NTFS },
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
	    optional<string> mkfs_options;
	    optional<string> tune_options;
	    optional<string> pool_name;
	    optional<SmartSize> size;
	    bool force = false;

	    vector<string> blk_devices;

	    enum class ModusOperandi { BLK_DEVICE, POOL, PARTITION_TABLE_FROM_STACK, BLK_DEVICE_FROM_STACK,
		PARTITIONABLE };

	    ModusOperandi modus_operandi;

	    void calculate_modus_operandi();

	    void check() const;
	};


	Options::Options(GetOpts& get_opts)
	{
	    ParsedOpts parsed_opts = get_opts.parse("filesystem", create_filesystem_options);

	    if (parsed_opts.has_option("type"))
	    {
		string str = parsed_opts.get("type");

		map<string, FsType>::const_iterator it = str_to_fs_type.find(str);
		if (it == str_to_fs_type.end())
		    throw runtime_error(sformat(_("unknown filesystem type '%s'"), str.c_str()));

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

	    if (parsed_opts.has_option("mkfs-options"))
		mkfs_options = parsed_opts.get("mkfs-options");

	    if (parsed_opts.has_option("tune-options"))
		tune_options = parsed_opts.get("tune-options");

	    pool_name = parsed_opts.get_optional("pool-name");

	    if (parsed_opts.has_option("size"))
	    {
		string str = parsed_opts.get("size");
		size = SmartSize(str);
	    }

	    force = parsed_opts.has_option("force");

	    blk_devices = parsed_opts.get_blk_devices();

	    calculate_modus_operandi();
	}


	void
	Options::calculate_modus_operandi()
	{
	    // TODO identical in create-lvm-vg.cc

	    if (pool_name)
	    {
		if (!size)
		    throw runtime_error(_("size argument missing for command 'filesystem'"));

		if (!blk_devices.empty())
		    throw runtime_error(_("pool argument and blk devices not allowed together for command 'filesystem'"));

		modus_operandi = ModusOperandi::POOL;
	    }
	    else
	    {
		if (size)
		{
		    if (blk_devices.empty())
			modus_operandi = ModusOperandi::PARTITION_TABLE_FROM_STACK;
		    else
			modus_operandi = ModusOperandi::PARTITIONABLE;
		}
		else
		{
		    if (blk_devices.empty())
			modus_operandi = ModusOperandi::BLK_DEVICE_FROM_STACK;
		    else
			modus_operandi = ModusOperandi::BLK_DEVICE;
		}
	    }
	}


	void
	Options::check() const
	{
	    if (tune_options)
	    {
		if (type.value() != FsType::EXT2 && type.value() != FsType::EXT3 &&
		    type.value() != FsType::EXT4)
		    throw runtime_error(sformat(_("tune options not allowed for %s"),
						get_fs_type_name(type.value()).c_str()));
	    }

	    if (path)
	    {
		if (type.value() != FsType::SWAP && !boost::starts_with(path.value(), "/"))
		{
		    throw runtime_error(sformat(_("invalid path '%s'"), path.value().c_str()));
		}

		if (type.value() == FsType::SWAP && path.value() != "swap")
		{
		    throw runtime_error(_("path must be 'swap'"));
		}
	    }

	    if (!path && mount_options)
	    {
		throw runtime_error(_("mount options require a path"));
	    }
	}

    }


    class ParsedCmdCreateFilesystem : public ParsedCmd
    {
    public:

	ParsedCmdCreateFilesystem(const Options& options)
	    : options(options)
	{
	    options.check();
	}

	virtual bool do_backup() const override { return true; }

	virtual void doit(const GlobalOptions& global_options, State& state) const override;

    private:

	const Options options;

    };


    void
    ParsedCmdCreateFilesystem::doit(const GlobalOptions& global_options, State& state) const
    {
	Devicegraph* staging = state.storage->get_staging();

	BlkDevice* blk_device = nullptr;

	switch (options.modus_operandi)
	{
	    case Options::ModusOperandi::BLK_DEVICE_FROM_STACK:
	    {
		if (state.stack.empty() || !is_blk_device(state.stack.top(staging)))
		    throw runtime_error(_("not a block device on stack"));

		blk_device = to_blk_device(state.stack.top(staging));
		state.stack.pop();
	    }
	    break;

	    case Options::ModusOperandi::POOL:
	    {
		Devicegraph* staging = state.storage->get_staging();

		Pool* pool = state.storage->get_pool(options.pool_name.value());

		SmartSize smart_size = options.size.value();

		unsigned long long size = smart_size.value(pool->max_partition_size(staging, 1));

		blk_device = pool->create_partitions(staging, 1, size)[0];
	    }
	    break;

	    case Options::ModusOperandi::PARTITION_TABLE_FROM_STACK:
	    {
		if (state.stack.empty() || !is_partition_table(state.stack.top(staging)))
		    throw runtime_error(_("not a partition table on stack"));

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

	    case Options::ModusOperandi::BLK_DEVICE:
	    {
		if (options.blk_devices.size() != 1)
		    throw runtime_error(_("only one block device allowed"));

		blk_device = BlkDevice::find_by_name(staging, options.blk_devices.front());

		if (blk_device->has_children())
		{
		    if (options.force)
		    {
			blk_device->remove_descendants(View::REMOVE);
		    }
		    else
		    {
			throw runtime_error(sformat(_("block device '%s' is in use"), blk_device->get_name().c_str()));
		    }
		}
	    }
	    break;

	    case Options::ModusOperandi::PARTITIONABLE:
	    {
		Pool pool;

		if (options.blk_devices.size() != 1)
		    throw runtime_error(_("wrong number of partitionables"));

		for (const string& device_name : options.blk_devices)
		{
		    Partitionable* partitionable = Partitionable::find_by_name(staging, device_name);
		    pool.add_device(partitionable);
		}

		SmartSize smart_size = options.size.value();

		unsigned long long size = smart_size.value(pool.max_partition_size(staging, 1));

		blk_device = pool.create_partitions(staging, 1, size)[0];
	    }
	    break;
	}

	if (!blk_device->is_usable_as_blk_device())
	    throw runtime_error(sformat(_("block device '%s' cannot be used as a regular block device"),
					blk_device->get_name().c_str()));

	FsType fs_type = options.type.value();

	if (fs_type != FsType::SWAP && options.path)
	{
	    string path = options.path.value();

	    if (!MountPoint::find_by_path(staging, path).empty())
		throw runtime_error(sformat(_("path '%s' already used"), path.c_str()));
	}

	BlkFilesystem* blk_filesystem = blk_device->create_blk_filesystem(fs_type);

	if (options.label)
	{
	    blk_filesystem->set_label(options.label.value());
	}

	if (options.mkfs_options)
	{
	    blk_filesystem->set_mkfs_options(options.mkfs_options.value());
	}

	if (options.tune_options)
	{
	    blk_filesystem->set_tune_options(options.tune_options.value());
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

	if (is_partition(blk_device))
	{
	    unsigned int id = ID_LINUX;

	    if (fs_type == FsType::SWAP)
	    {
		id = ID_SWAP;
	    }
	    else if (options.path)
	    {
		// Requires SUSE parted 3.5 or higher.

#if 0
		const string& path = options.path.value();

		if (path == "/home")
		    id = ID_LINUX_HOME;
		else if (path == "/srv")
		    id = ID_LINUX_SERVER_DATA;
#endif
	    }

	    Partition* partition = to_partition(blk_device);
	    partition->set_id(id);
	}

	state.stack.push(blk_filesystem);
	state.modified = true;
    }


    shared_ptr<ParsedCmd>
    parse_create_filesystem(GetOpts& get_opts, FsType type)
    {
	Options options(get_opts);

	if (options.type)
	    throw runtime_error(_("filesystem type already set for command 'filesystem'"));

	options.type = type;

	return make_shared<ParsedCmdCreateFilesystem>(options);
    }


    shared_ptr<ParsedCmd>
    CmdCreateFilesystem::parse(GetOpts& get_opts) const
    {
	Options options(get_opts);

	if (!options.type)
	    throw runtime_error(_("filesystem type missing for command 'filesystem'"));

	return make_shared<ParsedCmdCreateFilesystem>(options);
    }


    const char*
    CmdCreateFilesystem::help() const
    {
	return _("Creates a new file system.");
    }


    const ExtOptions&
    CmdCreateFilesystem::options() const
    {
	return create_filesystem_options;
    }


    shared_ptr<ParsedCmd>
    CmdCreateBtrfs::parse(GetOpts& get_opts) const
    {
	return parse_create_filesystem(get_opts, FsType::BTRFS);
    }


    const char*
    CmdCreateBtrfs::help() const
    {
	return _("Alias for 'create filesystem --type btrfs'");
    }


    shared_ptr<ParsedCmd>
    CmdCreateExfat::parse(GetOpts& get_opts) const
    {
	return parse_create_filesystem(get_opts, FsType::EXFAT);
    }


    const char*
    CmdCreateExfat::help() const
    {
	return _("Alias for 'create filesystem --type exfat'");
    }


    shared_ptr<ParsedCmd>
    CmdCreateExt2::parse(GetOpts& get_opts) const
    {
	return parse_create_filesystem(get_opts, FsType::EXT2);
    }


    const char*
    CmdCreateExt2::help() const
    {
	return _("Alias for 'create filesystem --type ext2'");
    }


    shared_ptr<ParsedCmd>
    CmdCreateExt3::parse(GetOpts& get_opts) const
    {
	return parse_create_filesystem(get_opts, FsType::EXT3);
    }


    const char*
    CmdCreateExt3::help() const
    {
	return _("Alias for 'create filesystem --type ext3'");
    }


    shared_ptr<ParsedCmd>
    CmdCreateExt4::parse(GetOpts& get_opts) const
    {
	return parse_create_filesystem(get_opts, FsType::EXT4);
    }


    const char*
    CmdCreateExt4::help() const
    {
	return _("Alias for 'create filesystem --type ext4'");
    }


    shared_ptr<ParsedCmd>
    CmdCreateF2fs::parse(GetOpts& get_opts) const
    {
	return parse_create_filesystem(get_opts, FsType::F2FS);
    }


    const char*
    CmdCreateF2fs::help() const
    {
	return _("Alias for 'create filesystem --type f2fs'");
    }


    shared_ptr<ParsedCmd>
    CmdCreateNilfs2::parse(GetOpts& get_opts) const
    {
	return parse_create_filesystem(get_opts, FsType::NILFS2);
    }


    const char*
    CmdCreateNilfs2::help() const
    {
	return _("Alias for 'create filesystem --type nilfs2'");
    }


    shared_ptr<ParsedCmd>
    CmdCreateNtfs::parse(GetOpts& get_opts) const
    {
	return parse_create_filesystem(get_opts, FsType::NTFS);
    }


    const char*
    CmdCreateNtfs::help() const
    {
	return _("Alias for 'create filesystem --type ntfs'");
    }


    shared_ptr<ParsedCmd>
    CmdCreateSwap::parse(GetOpts& get_opts) const
    {
	return parse_create_filesystem(get_opts, FsType::SWAP);
    }


    const char*
    CmdCreateSwap::help() const
    {
	return _("Alias for 'create filesystem --type swap'");
    }


    shared_ptr<ParsedCmd>
    CmdCreateVfat::parse(GetOpts& get_opts) const
    {
	return parse_create_filesystem(get_opts, FsType::VFAT);
    }


    const char*
    CmdCreateVfat::help() const
    {
	return _("Alias for 'create filesystem --type vfat'");
    }


    shared_ptr<ParsedCmd>
    CmdCreateXfs::parse(GetOpts& get_opts) const
    {
	return parse_create_filesystem(get_opts, FsType::XFS);
    }


    const char*
    CmdCreateXfs::help() const
    {
	return _("Alias for 'create filesystem --type xfs'");
    }

}
