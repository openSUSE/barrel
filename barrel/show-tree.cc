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


#include <storage/Storage.h>
#include <storage/Devices/Disk.h>
#include <storage/Devices/Md.h>
#include <storage/Devices/Encryption.h>

#include "Utils/GetOpts.h"
#include "Utils/Table.h"
#include "Utils/Text.h"
#include "show-tree.h"
#include "show.h"


namespace barrel
{

    using namespace std;
    using namespace storage;


    namespace
    {

	enum class Direction { UP, DOWN };


	const ExtOptions show_tree_options({
	    { "up", no_argument, 'u', _("go upwards") },
	    { "down", no_argument, 'd', _("go downwards") },
	    { "probed", no_argument, 0, _("probed instead of staging") }
	}, TakeBlkDevices::YES);


	struct Options
	{
	    Options(GetOpts& get_opts);

	    Direction direction = Direction::UP;
	    bool show_probed = false;

	    vector<string> blk_devices;
	};


	Options::Options(GetOpts& get_opts)
	{
	    ParsedOpts parsed_opts = get_opts.parse("tree", show_tree_options);

	    if (parsed_opts.has_option("up"))
		direction = Direction::UP;
	    else if (parsed_opts.has_option("down"))
		direction = Direction::DOWN;

	    show_probed = parsed_opts.has_option("probed");

	    blk_devices = parsed_opts.get_blk_devices();
	}

    }


    class ParsedCmdShowTree : public ParsedCmdShow
    {
    public:

	ParsedCmdShowTree(const Options& options) : options(options) {}

	virtual bool do_backup() const override { return false; }

	virtual void doit(const GlobalOptions& global_options, State& state) const override;

    private:

	const Options options;

	void worker(const Storage* storage, const Device* device, Table::Row& row) const;

    };


    void
    ParsedCmdShowTree::worker(const Storage* storage, const Device* device, Table::Row& row) const
    {
	if (is_blk_device(device))
	{
	    const BlkDevice* blk_device = to_blk_device(device);

	    row[Id::NAME] = blk_device->get_name();
	    row[Id::SIZE] = format_size(blk_device->get_size());
	    row[Id::USAGE] = device_usage(blk_device);
	    row[Id::POOL] = device_pools(storage, blk_device);
	}

	switch (options.direction)
	{
	    case Direction::UP:
	    {
		for (const Device* parent : device->get_parents())
		{
		    if (is_blk_device(parent))
		    {
			Table::Row subrow(row.get_table());
			worker(storage, parent, subrow);
			row.add_subrow(subrow);
		    }
		    else
		    {
			worker(storage, parent, row);
		    }
		}
	    }
	    break;

	    case Direction::DOWN:
	    {
		for (const Device* child : device->get_children())
		{
		    if (is_blk_device(child))
		    {
			Table::Row subrow(row.get_table());
			worker(storage, child, subrow);
			row.add_subrow(subrow);
		    }
		    else
		    {
			worker(storage, child, row);
		    }
		}
	    }
	    break;
	}
    }


    void
    ParsedCmdShowTree::doit(const GlobalOptions& global_options, State& state) const
    {
	const Storage* storage = state.storage;

	const Devicegraph* devicegraph = options.show_probed ? storage->get_probed() : storage->get_staging();

	Table table({ Cell(_("Name"), Id::NAME), Cell(_("Size"), Id::SIZE, Align::RIGHT),
		Cell(_("Usage"), Id::USAGE), Cell(_("Pool"), Id::POOL) });

	for (const string& name : options.blk_devices)
	{
	    const BlkDevice* blk_device = BlkDevice::find_by_name(devicegraph, name);

	    Table::Row row(table);
	    worker(storage, blk_device, row);
	    table.add(row);
	}

	cout << table;
    }


    shared_ptr<ParsedCmd>
    CmdShowTree::parse(GetOpts& get_opts) const
    {
	Options options(get_opts);

	return make_shared<ParsedCmdShowTree>(options);
    }


    const char*
    CmdShowTree::help() const
    {
	return _("Shows device tree.");
    }


    const ExtOptions&
    CmdShowTree::options() const
    {
	return show_tree_options;
    }

}
