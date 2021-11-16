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
#include <storage/Devices/Md.h>
#include <storage/Devices/LvmVg.h>
#include <storage/Devices/LvmLv.h>
#include <storage/Devices/Gpt.h>
#include <storage/Devices/Msdos.h>
#include <storage/Devices/Luks.h>
#include <storage/Filesystems/BlkFilesystem.h>

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
	if (state.stack.empty())
	    throw runtime_error(_("stack empty during pop"));

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
	if (state.stack.empty())
	    throw runtime_error(_("stack empty during dup"));

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


    class ParsedCmdStack : public ParsedCmd
    {
    public:

	virtual bool do_backup() const override { return false; }

	virtual void doit(const GlobalOptions& global_options, State& state) const override;

    private:

	string description(const Device* device) const;

    };


    string
    ParsedCmdStack::description(const Device* device) const
    {
	if (is_md(device))
	{
	    const Md* md = to_md(device);
	    return sformat(_("RAID %s"), md->get_name().c_str());
	}

	if (is_lvm_vg(device))
	{
	    const LvmVg* lvm_vg = to_lvm_vg(device);
	    return sformat(_("LVM volume group %s"), lvm_vg->get_vg_name().c_str());
	}

	if (is_lvm_lv(device))
	{
	    const LvmLv* lvm_lv = to_lvm_lv(device);
	    return sformat(_("LVM logical volume %s"), lvm_lv->get_name().c_str());
	}

	if (is_gpt(device))
	{
	    const Gpt* gpt = to_gpt(device);
	    return sformat(_("GPT on %s"), gpt->get_partitionable()->get_name().c_str());
	}

	if (is_msdos(device))
	{
	    const Msdos* msdos = to_msdos(device);
	    return sformat(_("MS-DOS on %s"), msdos->get_partitionable()->get_name().c_str());
	}

	if (is_luks(device))
	{
	    const Luks* luks = to_luks(device);
	    return sformat(_("LUKS on %s"), luks->get_blk_device()->get_name().c_str());
	}

	if (is_blk_filesystem(device))
	{
	    const BlkFilesystem* blk_filesystem = to_blk_filesystem(device);
	    // TODO support several devices, maybe use join from libstorage-ng
	    return sformat(_("filesystem %s on %s"), get_fs_type_name(blk_filesystem->get_type()).c_str(),
			   blk_filesystem->get_blk_devices()[0]->get_name().c_str());
	}

	return "unknown";
    }


    void
    ParsedCmdStack::doit(const GlobalOptions& global_options, State& state) const
    {
	Devicegraph* staging = state.storage->get_staging();

	Table table({ _("Position"), _("Description") });
	table.set_style(Style::NONE);

	for (Stack::const_iterator it = state.stack.begin(); it != state.stack.end(); ++it)
	{
	    const sid_t sid = *it;

	    string p = it == state.stack.begin() ? "top" : "";

	    string d = "deleted";

	    if (staging->device_exists(sid))
	    {
		const Device* device = staging->find_device(sid);
		d = description(device);
	    }

	    Table::Row row(table, { p, d });
	    table.add(row);
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
	    throw runtime_error(_("backup empty during undo"));

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
