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

#include "generic.h"
#include "Utils/Prompt.h"
#include "Utils/Text.h"
#include "Utils/Table.h"


namespace barrel
{

    class ParsedCmdPop : public ParsedCmd
    {
    public:

	virtual bool do_backup() const override { return false; }

	virtual void doit(const GlobalOptions& global_options, State& state) const override;

    };


    void
    ParsedCmdPop::doit(const GlobalOptions& global_options, State& state) const
    {
	state.stack.pop();
    }


    shared_ptr<ParsedCmd>
    CmdPop::parse(GetOpts& get_opts) const
    {
	get_opts.parse("pop", GetOpts::no_ext_options);

	return make_shared<ParsedCmdPop>();
    }


    const char*
    CmdPop::help() const
    {
	return _("Removes the top object from the stack.");
    }


    class ParsedCmdClear : public ParsedCmd
    {
    public:

	virtual bool do_backup() const override { return false; }

	virtual void doit(const GlobalOptions& global_options, State& state) const override;

    };


    void
    ParsedCmdClear::doit(const GlobalOptions& global_options, State& state) const
    {
	state.stack.clear();
    }


    shared_ptr<ParsedCmd>
    CmdClear::parse(GetOpts& get_opts) const
    {
	get_opts.parse("clear", GetOpts::no_ext_options);

	return make_shared<ParsedCmdClear>();
    }


    const char*
    CmdClear::help() const
    {
	return _("Clears the stack.");
    }


    class ParsedCmdDup : public ParsedCmd
    {
    public:

	virtual bool do_backup() const override { return false; }

	virtual void doit(const GlobalOptions& global_options, State& state) const override;

    };


    void
    ParsedCmdDup::doit(const GlobalOptions& global_options, State& state) const
    {
	state.stack.dup();
    }


    shared_ptr<ParsedCmd>
    CmdDup::parse(GetOpts& get_opts) const
    {
	get_opts.parse("dup", GetOpts::no_ext_options);

	return make_shared<ParsedCmdDup>();
    }


    const char*
    CmdDup::help() const
    {
	return _("Duplicates the top object of the stack.");
    }


    class ParsedCmdExch : public ParsedCmd
    {
    public:

	virtual bool do_backup() const override { return false; }

	virtual void doit(const GlobalOptions& global_options, State& state) const override;

    };


    void
    ParsedCmdExch::doit(const GlobalOptions& global_options, State& state) const
    {
	state.stack.exch();
    }


    shared_ptr<ParsedCmd>
    CmdExch::parse(GetOpts& get_opts) const
    {
	get_opts.parse("exch", GetOpts::no_ext_options);

	return make_shared<ParsedCmdExch>();
    }


    const char*
    CmdExch::help() const
    {
	return _("Exchanges the top two elements of the stack.");
    }


    class ParsedCmdStack : public ParsedCmd
    {
    public:

	virtual bool do_backup() const override { return false; }

	virtual void doit(const GlobalOptions& global_options, State& state) const override;

    };


    void
    ParsedCmdStack::doit(const GlobalOptions& global_options, State& state) const
    {
	const Stack& stack = state.stack;
	Devicegraph* staging = state.storage->get_staging();

	Table table({ _("Position"), _("Description") });
	table.set_style(Style::NONE);

	for (Stack::const_iterator it = stack.begin(); it != stack.end(); ++it)
	{
	    string p = it == stack.begin() ? "top" : "";

	    const StackObject::Base* stack_object = it->get();

	    table.add(Table::Row(table, { p, stack_object->print(staging) }));
	}

	cout << table;
    }


    shared_ptr<ParsedCmd>
    CmdStack::parse(GetOpts& get_opts) const
    {
	get_opts.parse("stack", GetOpts::no_ext_options);

	return make_shared<ParsedCmdStack>();
    }


    const char*
    CmdStack::help() const
    {
	return _("Prints the stack.");
    }


    class ParsedCmdUndo : public ParsedCmd
    {
    public:

	virtual bool do_backup() const override { return false; }

	virtual void doit(const GlobalOptions& global_options, State& state) const override;

    };


    void
    ParsedCmdUndo::doit(const GlobalOptions& global_options, State& state) const
    {
	if (state.backup.empty())
	    // TRANSLATORS: error message
	    throw runtime_error(_("no backup available for undo"));

	state.backup.undo(state.storage);
    }


    shared_ptr<ParsedCmd>
    CmdUndo::parse(GetOpts& get_opts) const
    {
	get_opts.parse("undo", GetOpts::no_ext_options);

	return make_shared<ParsedCmdUndo>();
    }


    const char*
    CmdUndo::help() const
    {
	return _("Restores the staging devicegraph to the last backup.");
    }


    class ParsedCmdQuit : public ParsedCmd
    {
    public:

	virtual bool do_backup() const override { return false; }

	virtual void doit(const GlobalOptions& global_options, State& state) const override;

    };


    void
    ParsedCmdQuit::doit(const GlobalOptions& global_options, State& state) const
    {
	// TODO check if there are any changes at all

	if (!global_options.yes)
	{
	    if (!prompt(_("Quit?")))
		return;
	}

	state.run = false;
    }


    shared_ptr<ParsedCmd>
    CmdQuit::parse()
    {
	return make_shared<ParsedCmdQuit>();
    }


    shared_ptr<ParsedCmd>
    CmdQuit::parse(GetOpts& get_opts) const
    {
	get_opts.parse("quit", GetOpts::no_ext_options);

	return make_shared<ParsedCmdQuit>();
    }


    const char*
    CmdQuit::help() const
    {
	return _("Quits barrel.");
    }

}
