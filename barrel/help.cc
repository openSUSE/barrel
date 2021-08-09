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


#include "Utils/GetOpts.h"
#include "help.h"
#include "cmds.h"


namespace barrel
{

    using namespace std;
    using namespace storage;


    class CmdHelp : public Cmd
    {
    public:

	virtual bool do_backup() const override { return false; }

	virtual void doit(const GlobalOptions& global_options, State& state) const override;

    };


    void
    help()
    {
	for (const MainCmd& main_cmd : main_cmds)
	{
	    if (main_cmd.cmd_func)
	    {
		cout << main_cmd.name << '\n';
	    }
	    else
	    {
		for (const Parser& sub_cmd : main_cmd.sub_cmds)
		{
		    if (sub_cmd.cmd_func)
		    {
			cout << main_cmd.name << " " << sub_cmd.name << '\n';
		    }
		}
	    }
	}
    }


    void
    CmdHelp::doit(const GlobalOptions& global_options, State& state) const
    {
	help();
    };


    shared_ptr<Cmd>
    parse_help(GetOpts& get_opts)
    {
	get_opts.parse("help", GetOpts::no_options);

	return make_shared<CmdHelp>();
    }

}
