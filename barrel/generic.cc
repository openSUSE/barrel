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


namespace barrel
{

    class CmdPop : public Cmd
    {
    public:

	virtual bool do_backup() const override { return false; }

	virtual void doit(const GlobalOptions& global_options, State& state) const override;

    };


    void
    CmdPop::doit(const GlobalOptions& global_options, State& state) const
    {
	if (state.stack.empty())
	    throw runtime_error("stack empty during pop");

	state.stack.pop();
    }


    shared_ptr<Cmd>
    parse_pop(GetOpts& get_opts)
    {
	get_opts.parse("pop", GetOpts::no_options);

	return make_shared<CmdPop>();
    }


    class CmdDup : public Cmd
    {
    public:

	virtual bool do_backup() const override { return false; }

	virtual void doit(const GlobalOptions& global_options, State& state) const override;

    };


    void
    CmdDup::doit(const GlobalOptions& global_options, State& state) const
    {
	if (state.stack.empty())
	    throw runtime_error("stack empty during dup");

	state.stack.dup();
    }


    shared_ptr<Cmd>
    parse_dup(GetOpts& get_opts)
    {
	get_opts.parse("dup", GetOpts::no_options);

	return make_shared<CmdDup>();
    }


    class CmdStack : public Cmd
    {
    public:

	virtual bool do_backup() const override { return false; }

	virtual void doit(const GlobalOptions& global_options, State& state) const override;

    };


    void
    CmdStack::doit(const GlobalOptions& global_options, State& state) const
    {
	Devicegraph* staging = state.storage->get_staging();

	for (sid_t sid : state.stack)
	{
	    cout << sid;

	    if (staging->device_exists(sid))
	    {
		const Device* device = staging->find_device(sid);
		cout << "  " << device->get_displayname();
	    }
	    else
	    {
		cout << "  invalid";
	    }

	    cout << endl;
	}
    }


    shared_ptr<Cmd>
    parse_stack(GetOpts& get_opts)
    {
	get_opts.parse("stack", GetOpts::no_options);

	return make_shared<CmdStack>();
    }


    class CmdUndo : public Cmd
    {
    public:

	virtual bool do_backup() const override { return false; }

	virtual void doit(const GlobalOptions& global_options, State& state) const override;

    };


    void
    CmdUndo::doit(const GlobalOptions& global_options, State& state) const
    {
	if (state.backup.empty())
	    throw runtime_error("backup empty during undo");

	state.backup.undo(state.storage);
    }


    shared_ptr<Cmd>
    parse_undo(GetOpts& get_opts)
    {
	get_opts.parse("undo", GetOpts::no_options);

	return make_shared<CmdUndo>();
    }


    class CmdQuit : public Cmd
    {
    public:

	virtual bool do_backup() const override { return false; }

	virtual void doit(const GlobalOptions& global_options, State& state) const override;

    };


    void
    CmdQuit::doit(const GlobalOptions& global_options, State& state) const
    {
	// TODO check if there are any changes at all

	if (!global_options.yes)
	{
	    if (!prompt(_("Quit?")))
		return;
	}

	state.run = false;
    }


    shared_ptr<Cmd>
    parse_quit(GetOpts& get_opts)
    {
	get_opts.parse("quit", GetOpts::no_options);

	return make_shared<CmdQuit>();
    }

}
