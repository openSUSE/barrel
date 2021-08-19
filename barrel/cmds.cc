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
    // TODO use some smart pointer


    const vector<Parser> show_cmds = {
	{ "commit", new CmdShowCommit },
	{ "disks", new CmdShowDisks },
	{ "filesystems", new CmdShowFilesystems },
	{ "pools", new CmdShowPools },
	{ "raids", new CmdShowRaids },
	{ "vgs", new CmdShowLvmVgs }
    };


    const vector<Parser> create_cmds = {
	{ "btrfs", new CmdCreateBtrfs },
	{ "encryption", new CmdCreateEncryption },
	{ "ext2", new CmdCreateExt2 },
	{ "ext3", new CmdCreateExt3 },
	{ "ext4", new CmdCreateExt4 },
	{ "filesystem", new CmdCreateFilesystem },
	{ "gpt", new CmdCreateGpt },
	{ "luks1", new CmdCreateLuks1 },
	{ "luks2", new CmdCreateLuks2 },
	{ "lv", new CmdCreateLvmLv },
	{ "ms-dos", new CmdCreateMsdos },
	{ "partition-table", new CmdCreatePartitionTable },
	{ "pool", new CmdCreatePool },
	{ "raid", new CmdCreateRaid },
	{ "raid0", new CmdCreateRaid0 },
	{ "raid1", new CmdCreateRaid1 },
	{ "raid4", new CmdCreateRaid4 },
	{ "raid5", new CmdCreateRaid5 },
	{ "raid6", new CmdCreateRaid6 },
	{ "raid10", new CmdCreateRaid10 },
	{ "swap", new CmdCreateSwap },
	{ "vg", new CmdCreateLvmVg },
	{ "xfs", new CmdCreateXfs }
    };


    const vector<Parser> extend_cmds = {
	{ "pool", new CmdExtendPool }
    };


    const vector<Parser> reduce_cmds = {
	{ "pool", new CmdReducePool }
    };


    const vector<Parser> remove_cmds = {
	{ "device", new CmdRemoveDevice },
	{ "pool", new CmdRemovePool }
    };


    const vector<Parser> rename_cmds = {
	{ "pool", new CmdRenamePool }
    };


    const vector<Parser> load_cmds = {
	{ "devicegraph", new CmdLoadDevicegraph },
	{ "pools", new CmdLoadPools }
    };


    const vector<Parser> save_cmds = {
	{ "devicegraph", new CmdSaveDevicegraph },
	{ "pools", new CmdSavePools }
    };


    const vector<MainCmd> main_cmds = {
	{ "commit", new CmdCommit, {} },
	{ "create", nullptr, create_cmds },
	{ "dup", new CmdDup, {} },
	{ "extend", nullptr, extend_cmds },
	{ "help", new CmdHelp, {} },
	{ "load", nullptr, load_cmds },
	{ "pop", new CmdPop, {} },
	{ "quit", new CmdQuit, {} },
	{ "reduce", nullptr, reduce_cmds },
	{ "remove", nullptr, remove_cmds },
	{ "rename", nullptr, rename_cmds },
	{ "save", nullptr, save_cmds },
	{ "show", nullptr, show_cmds },
	{ "stack", new CmdStack, {} },
	{ "undo", new CmdUndo, {} }
    };

}
