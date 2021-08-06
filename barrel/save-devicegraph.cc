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

#include "Utils/GetOpts.h"
#include "save-devicegraph.h"


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

	    ParsedOpts parsed_opts = get_opts.parse("devicegraph", options);

	    if (parsed_opts.has_option("name"))
		name = parsed_opts.get("name");
	    else
		throw OptionsException("name missing");
	}

    }


    class CmdSaveDevicegraph : public Cmd
    {
    public:

	CmdSaveDevicegraph(const Options& options) : options(options) {}

	virtual bool do_backup() const override { return true; }

	virtual void doit(const GlobalOptions& global_options, State& state) const override;

    private:

	const Options options;

    };


    void
    CmdSaveDevicegraph::doit(const GlobalOptions& global_options, State& state) const
    {
	const Devicegraph* staging = state.storage->get_staging();

	staging->save(options.name);
    }


    shared_ptr<Cmd>
    parse_save_devicegraph(GetOpts& get_opts)
    {
	Options options(get_opts);

	return make_shared<CmdSaveDevicegraph>(options);
    }

}
