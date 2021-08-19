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
#include "rename-pool.h"


namespace barrel
{

    using namespace storage;


    namespace
    {

	const vector<Option> rename_pool_options = {
	    { "old-name", required_argument, 'o', _("old name"), "name" },
	    { "new-name", required_argument, 'n', _("new name"), "name" }
	};


	struct Options
	{
	    Options(GetOpts& get_opts);

	    string old_name;
	    string new_name;
	};


	Options::Options(GetOpts& get_opts)
	{
	    ParsedOpts parsed_opts = get_opts.parse("pool", rename_pool_options);

	    old_name = parsed_opts.get("old-name");
	    new_name = parsed_opts.get("new-name");
	}

    }


    class ParsedCmdRenamePool : public ParsedCmd
    {
    public:

	ParsedCmdRenamePool(const Options& options) : options(options) {}

	virtual bool do_backup() const override { return false; }

	virtual void doit(const GlobalOptions& global_options, State& state) const override;

    private:

	const Options options;

    };


    void
    ParsedCmdRenamePool::doit(const GlobalOptions& global_options, State& state) const
    {
	state.storage->rename_pool(options.old_name, options.new_name);

	state.pools_modified = true;
    }


    shared_ptr<ParsedCmd>
    CmdRenamePool::parse(GetOpts& get_opts) const
    {
	Options options(get_opts);

	return make_shared<ParsedCmdRenamePool>(options);
    }


    const char*
    CmdRenamePool::help() const
    {
	return _("rename pool");
    }


    const vector<Option>&
    CmdRenamePool::options() const
    {
	return rename_pool_options;
    }

}
