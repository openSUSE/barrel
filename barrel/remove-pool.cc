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

#include "Utils/GetOpts.h"
#include "Utils/Text.h"
#include "remove-pool.h"


namespace barrel
{

    using namespace storage;


    namespace
    {

	const ExtOptions remove_pool_options({
	    { "name", required_argument, 'n', _("name of pool"), "name" }
	});


	struct Options
	{
	    Options(GetOpts& get_opts);

	    string name;
	};


	Options::Options(GetOpts& get_opts)
	{
	    ParsedOpts parsed_opts = get_opts.parse("pool", remove_pool_options);

	    name = parsed_opts.get("name");
	}

    }


    class ParsedCmdRemovePool : public ParsedCmd
    {
    public:

	ParsedCmdRemovePool(const Options& options) : options(options) {}

	virtual bool do_backup() const override { return false; }

	virtual void doit(const GlobalOptions& global_options, State& state) const override;

    private:

	const Options options;

    };


    void
    ParsedCmdRemovePool::doit(const GlobalOptions& global_options, State& state) const
    {
	state.storage->remove_pool(options.name);

	state.pools_modified = true;
    }


    shared_ptr<ParsedCmd>
    CmdRemovePool::parse(GetOpts& get_opts) const
    {
	Options options(get_opts);

	return make_shared<ParsedCmdRemovePool>(options);
    }


    const char*
    CmdRemovePool::help() const
    {
	return _("Removes a pool.");
    }


    const ExtOptions&
    CmdRemovePool::options() const
    {
	return remove_pool_options;
    }

}
