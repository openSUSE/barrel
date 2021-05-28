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
#include <storage/Filesystems/Mountable.h>
#include <storage/Filesystems/MountPoint.h>
#include <storage/Filesystems/Filesystem.h>
#include <storage/Filesystems/BlkFilesystem.h>
#include <storage/Storage.h>

#include "Utils/GetOpts.h"
#include "Utils/Table.h"
#include "Utils/Text.h"
#include "Utils/Misc.h"
#include "show-filesystems.h"


namespace barrel
{

    using namespace std;
    using namespace storage;


    class CmdShowFilesystems : public Cmd
    {
    public:

	virtual bool do_backup() const override { return false; }

	virtual void doit(const GlobalOptions& global_options, State& state) const override;

    };


    void
    CmdShowFilesystems::doit(const GlobalOptions& global_options, State& state) const
    {
	const Devicegraph* staging = state.storage->get_staging();

	vector<const Filesystem*> filesystems = Filesystem::get_all(staging);
	// TODO sort

	Table table({ _("Type"), _("Device"), _("Mount Point") });

	for (const Filesystem* filesystem : filesystems)
	{
	    Table::Row row(table, { get_fs_type_name(filesystem->get_type()) });

	    if (is_blk_filesystem(filesystem))
	    {
		const BlkFilesystem* blk_filesystem = to_blk_filesystem(filesystem);
		row.add(blk_filesystem->get_blk_devices()[0]->get_name());
	    }
	    else
	    {
		row.add("");
	    }

	    if (filesystem->has_mount_point())
	    {
		const MountPoint* mount_point = filesystem->get_mount_point();
		row.add(mount_point->get_path());
	    }
	    else
	    {
		row.add("");
	    }

	    table.add(row);
	}

	cout << table;
    }


    shared_ptr<Cmd>
    parse_show_filesystems(GetOpts& get_opts)
    {
	get_opts.parse("filesystems", GetOpts::no_options);

	return make_shared<CmdShowFilesystems>();
    }

}
