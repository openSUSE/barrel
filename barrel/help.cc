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


    // TODO just a quick hack


    namespace
    {
	struct Options
	{
	    Options(GetOpts& get_opts);

	    optional<string> main_cmd;
	    optional<string> sub_cmd;
	};


	Options::Options(GetOpts& get_opts)
	{
	    if (get_opts.has_args())
		main_cmd = get_opts.pop_arg();

	    if (get_opts.has_args())
		sub_cmd = get_opts.pop_arg();
	}


	void
	print_options_help(const ExtOptions& ext_options)
	{
	    Table table({ Cell(_("Name"), Id::NAME), _("Description") });
	    table.set_style(Style::NONE);
	    table.set_global_indent(6);
	    table.set_min_width(Id::NAME, 28);

	    for (const Option& option : ext_options.options)
	    {
		if (option.description)
		{
		    string tmp;

		    if (option.c)
			tmp += "-"s + (char)(option.c) + ", "s;
		    else
			tmp += "    ";

		    tmp += "--"s + option.name;

		    if (option.has_arg != no_argument)
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
	print_cmd_help(const MainCmd& main_cmd)
	{
	    cout << main_cmd.name << '\n';

	    cout << "    " << main_cmd.cmd->help() << '\n';

	    print_options_help(main_cmd.cmd->options());
	}


	void
	print_cmd_help(const MainCmd& main_cmd, const Parser& sub_cmd)
	{
	    const ExtOptions& ext_options = sub_cmd.cmd->options();

	    switch (ext_options.take_blk_devices)
	    {
		case TakeBlkDevices::NO:
		    cout << main_cmd.name << " " << sub_cmd.name << '\n';
		    break;

		case TakeBlkDevices::YES:
		    cout << main_cmd.name << " " << sub_cmd.name << " devices\n";
		    break;

		case TakeBlkDevices::MAYBE:
		    cout << main_cmd.name << " " << sub_cmd.name << " [devices]\n";
		    break;
	    }

	    cout << "    " << sub_cmd.cmd->help() << '\n';

	    if (!sub_cmd.cmd->is_alias())
		print_options_help(sub_cmd.cmd->options());
	}

    };


    class ParsedCmdHelp : public ParsedCmd
    {
    public:

	ParsedCmdHelp(const Options& options) : options(options) {}

	virtual bool do_backup() const override { return false; }

	virtual void doit(const GlobalOptions& global_options, State& state) const override;

    private:

	const Options options;

    };


    void
    help(bool global)
    {
	if (global)
	{
	    cout << _("Global options:") << '\n';

	    print_options_help(GlobalOptions::get_options());

	    cout << '\n';

	    cout << _("Commands:") << '\n';

	    cout << '\n';
	}

	for (const MainCmd& main_cmd : main_cmds)
	{
	    if (main_cmd.cmd)
	    {
		print_cmd_help(main_cmd);
		cout << '\n';
	    }
	    else
	    {
		for (const Parser& sub_cmd : main_cmd.sub_cmds)
		{
		    print_cmd_help(main_cmd, sub_cmd);
		    cout << '\n';
		}
	    }
	}
    }


    void
    ParsedCmdHelp::doit(const GlobalOptions& global_options, State& state) const
    {
	if (options.main_cmd)
	{
	    vector<MainCmd>::const_iterator main_cmd =
		sloppy_find(main_cmds, options.main_cmd.value().c_str());

	    if (!options.sub_cmd)
	    {
		if (main_cmd->cmd)
		{
		    if (main_cmd->name == "help")
		    {
			// TRANSLATORS: just a funny remark without further meaning nor context
			cout << _("Keep smiling, 'cause that's the most important thing.") << '\n';
			return;
		    }

		    print_cmd_help(*main_cmd);
		}
		else
		{
		    cout << _("The command has the following subcommands:") << '\n';

		    for (const Parser& sub_cmd : main_cmd->sub_cmds)
			cout << sub_cmd.name << '\n';
		}
	    }
	    else
	    {
		vector<Parser>::const_iterator sub_cmd =
		    sloppy_find(main_cmd->sub_cmds, options.sub_cmd.value().c_str());

		print_cmd_help(*main_cmd, *sub_cmd);
	    }
	}
	else
	{
	    help(false);
	}
    };


    shared_ptr<ParsedCmd>
    CmdHelp::parse(GetOpts& get_opts) const
    {
	Options options(get_opts);

	return make_shared<ParsedCmdHelp>(options);
    }


    const char*
    CmdHelp::help() const
    {
	return _("Prints help.");
    }

}
