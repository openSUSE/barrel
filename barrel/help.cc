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
#include "Utils/Text.h"
#include "Utils/Table.h"
#include "help.h"
#include "cmds.h"


namespace barrel
{

    using namespace std;
    using namespace storage;


    class ParsedCmdHelp : public ParsedCmd
    {
    public:

	virtual bool do_backup() const override { return false; }

	virtual void doit(const GlobalOptions& global_options, State& state) const override;

    };


    void
    options_help(const vector<Option>& options)
    {
	Table table({ Cell("Name", Id::NAME), "Description" });
	table.set_style(Style::NONE);
	table.set_global_indent(6);
	table.set_min_width(Id::NAME, 28);

	for (const Option& option : options)
	{
	    if (option.description)
	    {
		string tmp;

		if (option.c)
		    tmp += "-"s + (char)(option.c) + ", "s;
		else
		    tmp += "    ";

		tmp += "--"s + option.name;
		if (option.has_arg == required_argument)
		{
		    if (!option.arg_name)
			throw runtime_error("arg_name not set");
		    tmp += " <"s + option.arg_name + ">"s;
		}

		Table::Row row(table, { tmp, option.description });
		table.add(row);
	    }
	}

	cout << table;
    }


    void
    help()
    {
	cout << "Global options" << '\n';

	options_help(GlobalOptions::get_options());

	cout << '\n';

	for (const MainCmd& main_cmd : main_cmds)
	{
	    if (main_cmd.cmd)
	    {
		cout << main_cmd.name << '\n';

		const char* h = main_cmd.cmd->help();
		if (h)
		    cout << "    " << h << '\n';

		const vector<Option>& o = main_cmd.cmd->options();
		if (!o.empty())
		    options_help(o);

		cout << '\n';
	    }
	    else
	    {
		for (const Parser& sub_cmd : main_cmd.sub_cmds)
		{
		    if (sub_cmd.cmd)
		    {
			cout << main_cmd.name << " " << sub_cmd.name << '\n';

			const char* h = sub_cmd.cmd->help();
			if (h)
			    cout << "    " << h << '\n';

			if (!sub_cmd.cmd->is_alias())
			{
			    const vector<Option>& o = sub_cmd.cmd->options();
			    if (!o.empty())
				options_help(o);
			}
		    }

		    cout << '\n';
		}
	    }
	}
    }


    void
    ParsedCmdHelp::doit(const GlobalOptions& global_options, State& state) const
    {
	help();
    };


    shared_ptr<ParsedCmd>
    CmdHelp::parse(GetOpts& get_opts) const
    {
	get_opts.parse("help", GetOpts::no_options);

	return make_shared<ParsedCmdHelp>();
    }


    const char*
    CmdHelp::help() const
    {
	return _("help");
    }

}
