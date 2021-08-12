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


#include "cmds.h"
#include "help.h"
#include "generic.h"
#include "show-disks.h"
#include "show-filesystems.h"
#include "show-pools.h"
#include "show-raids.h"
#include "show-lvm-vgs.h"
#include "show-commit.h"
#include "commit.h"
#include "create-raid.h"
#include "create-lvm-vg.h"
#include "create-lvm-lv.h"
#include "create-encryption.h"
#include "create-partition-table.h"
#include "create-filesystem.h"
#include "create-pool.h"
#include "remove-pool.h"
#include "rename-pool.h"
#include "extend-pool.h"
#include "reduce-pool.h"
#include "remove-device.h"
#include "load-devicegraph.h"
#include "save-devicegraph.h"
#include "load-pools.h"
#include "save-pools.h"


namespace barrel
{

    const vector<Parser> show_cmds = {
	{ "disks", parse_show_disks },
	{ "filesystems", parse_show_filesystems },
	{ "pools", parse_show_pools },
	{ "raids", parse_show_raids },
	{ "vgs", parse_show_lvm_vgs },
	{ "commit", parse_show_commit }
    };


    const vector<Parser> create_cmds = {
	{ "pop", parse_pop },
	{ "dup", parse_dup },
	{ "raid", parse_create_raid },
	{ "raid0", parse_create_raid0 },
	{ "raid1", parse_create_raid1 },
	{ "raid4", parse_create_raid4 },
	{ "raid5", parse_create_raid5 },
	{ "raid6", parse_create_raid6 },
	{ "raid10", parse_create_raid10 },
	{ "vg", parse_create_lvm_vg },
	{ "lv", parse_create_lvm_lv },
	{ "encryption", parse_create_encryption },
	{ "luks1", parse_create_luks1 },
	{ "luks2", parse_create_luks2 },
	{ "partition-table", parse_create_partition_table },
	{ "gpt", parse_create_gpt },
	{ "ms-dos", parse_create_msdos },
	{ "filesystem", parse_create_filesystem },
	{ "btrfs", parse_create_btrfs },
	{ "ext2", parse_create_ext2 },
	{ "ext3", parse_create_ext3 },
	{ "ext4", parse_create_ext4 },
	{ "swap", parse_create_swap },
	{ "xfs", parse_create_xfs },
	{ "pool", parse_create_pool }
    };


    const vector<Parser> extend_cmds = {
	{ "pool", parse_extend_pool }
    };


    const vector<Parser> reduce_cmds = {
	{ "pool", parse_reduce_pool }
    };


    const vector<Parser> remove_cmds = {
	{ "device", parse_remove_device },
	{ "pool", parse_remove_pool }
    };


    const vector<Parser> rename_cmds = {
	{ "pool", parse_rename_pool }
    };


    const vector<Parser> load_cmds = {
	{ "devicegraph", parse_load_devicegraph },
	{ "pools", parse_load_pools }
    };


    const vector<Parser> save_cmds = {
	{ "devicegraph", parse_save_devicegraph },
	{ "pools", parse_save_pools }
    };


    const vector<MainCmd> main_cmds = {
	{ "help", parse_help, {} },
	{ "pop", parse_pop, {} },
	{ "dup", parse_dup, {} },
	{ "stack", parse_stack, {} },
	{ "undo", parse_undo, {} },
	{ "quit", parse_quit, {} },
	{ "show", nullptr, show_cmds },
	{ "create", nullptr, create_cmds },
	{ "extend", nullptr, extend_cmds },
	{ "reduce", nullptr, reduce_cmds },
	{ "remove", nullptr, remove_cmds },
	{ "rename", nullptr, rename_cmds },
	{ "load", nullptr, load_cmds },
	{ "save", nullptr, save_cmds },
	{ "commit", parse_commit, {} }
    };

}
