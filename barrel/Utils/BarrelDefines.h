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
 * with this program; if not, contact Novell, Inc.
 *
 * To contact SUSE LLC about this file by physical or electronic mail, you may
 * find current contact information at www.suse.com.
 */


#ifndef BARREL_BARREL_DEFINES_H
#define BARREL_BARREL_DEFINES_H


// paths

#define DEV_DIR "/dev"
#define DEV_MD_DIR "/dev/md"

#define DEV_DISK_BY_ID_DIR DEV_DIR "/disk/by-id"
#define DEV_DISK_BY_PATH_DIR DEV_DIR "/disk/by-path"

#define SYSCONF_DIR "/etc"


// files

#define POOLS_FILE SYSCONF_DIR "/barrel/pools.json"


#endif
