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
#include <storage/Devicegraph.h>
#include <storage/Devices/Disk.h>
#include <storage/Holders/Holder.h>

#include "Utils/GetOpts.h"
#include "load.h"


namespace barrel
{

    using namespace storage;


    namespace
    {

	struct Options
	{
	    Options(GetOpts& get_opts);

	    string devicegraph;
	};


	Options::Options(GetOpts& get_opts)
	{
	    const vector<Option> options = {
		{ "devicegraph", required_argument, 'f' }
	    };

	    ParsedOpts parsed_opts = get_opts.parse("load", options, true);

	    if (parsed_opts.has_option("devicegraph"))
	    {
		devicegraph = parsed_opts.get("devicegraph");
	    }
	    else
	    {
		throw OptionsException("devicegraph missing");
	    }
	}

    }


    class CmdLoad : public Cmd
    {
    public:

	CmdLoad(const Options& options) : options(options) {}

	virtual bool do_backup() const override { return true; }

	virtual void doit(State& state) const override;

    private:

	const Options options;

    };


    void
    CmdLoad::doit(State& state) const
    {
	const Devicegraph* probed = state.storage->get_probed();
	Devicegraph* staging = state.storage->get_staging();

	staging->load(options.devicegraph, false);

	for (Disk* a : staging->get_all_disks())
	{
	    const Disk* b = Disk::find_by_name(probed, a->get_name());

	    Device* c = b->copy_to_devicegraph(staging);

	    for (Holder* holder : a->get_out_holders())
		holder->set_source(c);

	    staging->remove_device(a);

	    // TODO must anything dependent be update? e.g. name, sysfs_name, sysfs_path
	    // of partitions, topology, ...
	}

	// TODO clear uuids
	// TODO what to do with subvolumes/snapshots of snapper?

	state.modified = true;
    }


    shared_ptr<Cmd>
    parse_load(GetOpts& get_opts)
    {
	Options options(get_opts);

	return make_shared<CmdLoad>(options);
    }

}
