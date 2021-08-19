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


#include <storage/Devices/Partitionable.h>

#include "handle.h"
#include "Utils/Table.h"


namespace barrel
{

    class ParsedCmdShow : public ParsedCmd
    {

    protected:

	string device_usage(const Device* device) const;

	string device_pool(const Storage* storage, const Device* device) const;

	void insert_partitions(const Partitionable* partitionable, Table::Row& row) const;

    };

};
