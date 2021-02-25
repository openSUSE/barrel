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


#ifndef BARREL_CREATE_FILESYSTEM_H
#define BARREL_CREATE_FILESYSTEM_H


#include "handle.h"


namespace barrel
{

    shared_ptr<Cmd>
    parse_create_filesystem(GetOpts& get_opts);

    shared_ptr<Cmd>
    parse_create_btrfs(GetOpts& get_opts);

    shared_ptr<Cmd>
    parse_create_exfat(GetOpts& get_opts);

    shared_ptr<Cmd>
    parse_create_ext2(GetOpts& get_opts);

    shared_ptr<Cmd>
    parse_create_ext3(GetOpts& get_opts);

    shared_ptr<Cmd>
    parse_create_ext4(GetOpts& get_opts);

    shared_ptr<Cmd>
    parse_create_swap(GetOpts& get_opts);

    shared_ptr<Cmd>
    parse_create_vfat(GetOpts& get_opts);

    shared_ptr<Cmd>
    parse_create_xfs(GetOpts& get_opts);

}

#endif
