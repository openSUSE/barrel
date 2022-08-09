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


#include "cmds.h"
#include "commit.h"
#include "create-encryption.h"
#include "create-filesystem.h"
#include "create-lvm-lv.h"
#include "create-lvm-vg.h"
#include "create-partition-table.h"
#include "create-pool.h"
#include "create-raid.h"
#include "extend-pool.h"
#include "generic.h"
#include "help.h"
#include "load-devicegraph.h"
#include "load-pools.h"
#include "reduce-pool.h"
#include "remove-device.h"
#include "remove-pool.h"
#include "rename-pool.h"
#include "save-devicegraph.h"
#include "save-pools.h"
#include "show-commit.h"
#include "show-dasds.h"
#include "show-disks.h"
#include "show-multipaths.h"
#include "show-filesystems.h"
#include "show-encryptions.h"
#include "show-lvm-vgs.h"
#include "show-pools.h"
#include "show-raids.h"
#include "show-tree.h"


namespace barrel
{

    const vector<Parser> show_cmds = {
	{ "commit", make_shared<CmdShowCommit>() },
	{ "dasds", make_shared<CmdShowDasds>() },
	{ "disks", make_shared<CmdShowDisks>() },
	{ "encryptions", make_shared<CmdShowEncryptions>() },
	{ "filesystems", make_shared<CmdShowFilesystems>() },
	{ "multipaths", make_shared<CmdShowMultipaths>() },
	{ "pools", make_shared<CmdShowPools>() },
	{ "raids", make_shared<CmdShowRaids>() },
	{ "tree", make_shared<CmdShowTree>() },
	{ "vgs", make_shared<CmdShowLvmVgs>() }
    };


    const vector<Parser> create_cmds = {
	{ "btrfs", make_shared<CmdCreateBtrfs>() },
	{ "encryption", make_shared<CmdCreateEncryption>() },
	{ "exfat", make_shared<CmdCreateExfat>() },
	{ "ext2", make_shared<CmdCreateExt2>() },
	{ "ext3", make_shared<CmdCreateExt3>() },
	{ "ext4", make_shared<CmdCreateExt4>() },
	{ "f2fs", make_shared<CmdCreateF2fs>() },
	{ "filesystem", make_shared<CmdCreateFilesystem>() },
	{ "gpt", make_shared<CmdCreateGpt>() },
	{ "jfs", make_shared<CmdCreateJfs>() },
	{ "luks1", make_shared<CmdCreateLuks1>() },
	{ "luks2", make_shared<CmdCreateLuks2>() },
	{ "lv", make_shared<CmdCreateLvmLv>() },
	{ "ms-dos", make_shared<CmdCreateMsdos>() },
	{ "nilfs2", make_shared<CmdCreateNilfs2>() },
	{ "ntfs", make_shared<CmdCreateNtfs>() },
	{ "partition-table", make_shared<CmdCreatePartitionTable>() },
	{ "pool", make_shared<CmdCreatePool>() },
	{ "raid", make_shared<CmdCreateRaid>() },
	{ "raid0", make_shared<CmdCreateRaid0>() },
	{ "raid1", make_shared<CmdCreateRaid1>() },
	{ "raid4", make_shared<CmdCreateRaid4>() },
	{ "raid5", make_shared<CmdCreateRaid5>() },
	{ "raid6", make_shared<CmdCreateRaid6>() },
	{ "raid10", make_shared<CmdCreateRaid10>() },
	{ "reiserfs", make_shared<CmdCreateReiserfs>() },
	{ "swap", make_shared<CmdCreateSwap>() },
	{ "udf", make_shared<CmdCreateUdf>() },
	{ "vfat", make_shared<CmdCreateVfat>() },
	{ "vg", make_shared<CmdCreateLvmVg>() },
	{ "xfs", make_shared<CmdCreateXfs>() }
    };


    const vector<Parser> extend_cmds = {
	{ "pool", make_shared<CmdExtendPool>() }
    };


    const vector<Parser> reduce_cmds = {
	{ "pool", make_shared<CmdReducePool>() }
    };


    const vector<Parser> remove_cmds = {
	{ "device", make_shared<CmdRemoveDevice>() },
	{ "pool", make_shared<CmdRemovePool>() }
    };


    const vector<Parser> rename_cmds = {
	{ "pool", make_shared<CmdRenamePool>() }
    };


    const vector<Parser> load_cmds = {
	{ "devicegraph", make_shared<CmdLoadDevicegraph>() },
	{ "pools", make_shared<CmdLoadPools>() }
    };


    const vector<Parser> save_cmds = {
	{ "devicegraph", make_shared<CmdSaveDevicegraph>() },
	{ "pools", make_shared<CmdSavePools>() }
    };


    const vector<MainCmd> main_cmds = {
	{ "[", make_shared<CmdOpenMark>(), {} },
	{ "]", make_shared<CmdCloseMark>(), {} },
	{ "clear", make_shared<CmdClear>(), {} },
	{ "commit", make_shared<CmdCommit>(), {} },
	{ "create", nullptr, create_cmds },
	{ "dup", make_shared<CmdDup>(), {} },
	{ "exch", make_shared<CmdExch>(), {} },
	{ "extend", nullptr, extend_cmds },
	{ "help", make_shared<CmdHelp>(), {} },
	{ "load", nullptr, load_cmds },
	{ "pop", make_shared<CmdPop>(), {} },
	{ "quit", make_shared<CmdQuit>(), {} },
	{ "reduce", nullptr, reduce_cmds },
	{ "remove", nullptr, remove_cmds },
	{ "rename", nullptr, rename_cmds },
	{ "save", nullptr, save_cmds },
	{ "show", nullptr, show_cmds },
	{ "stack", make_shared<CmdStack>(), {} },
	{ "undo", make_shared<CmdUndo>(), {} }
    };

}
