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


#include <regex>

#include <storage/Storage.h>
#include <storage/Pool.h>

#include "Utils/GetOpts.h"
#include "remove-pool.h"


namespace barrel
{

    using namespace storage;


    namespace
    {

	struct Options
	{
	    Options(GetOpts& get_opts);

	    string name;
	};


	Options::Options(GetOpts& get_opts)
	{
	    const vector<Option> options = {
		{ "name", required_argument, 'n' }
	    };

	    ParsedOpts parsed_opts = get_opts.parse("pool", options);

	    name = parsed_opts.get("name");
	}

    }


    class CmdRemovePool : public Cmd
    {
    public:

	CmdRemovePool(const Options& options) : options(options) {}

	virtual bool do_backup() const override { return false; }

	virtual void doit(const GlobalOptions& global_options, State& state) const override;

    private:

	const Options options;

    };


    void
    CmdRemovePool::doit(const GlobalOptions& global_options, State& state) const
    {
	state.storage->remove_pool(options.name);

	state.pools_modified = true;
    }


    shared_ptr<Cmd>
    parse_remove_pool(GetOpts& get_opts)
    {
	Options options(get_opts);

	return make_shared<CmdRemovePool>(options);
    }

}
