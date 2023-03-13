/*
 * Copyright (c) [2021-2023] SUSE LLC
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


#ifndef BARREL_HANDLE_H
#define BARREL_HANDLE_H


#include <string>

#include "Utils/GetOpts.h"
#include "Utils/Misc.h"
#include "stack.h"


namespace barrel
{

    using namespace std;
    using namespace storage;


    struct GlobalOptions
    {
	GlobalOptions(GetOpts& get_opts);

	static const ExtOptions& get_options();

	bool help = false;
	bool version = false;
	bool quiet = false;
	bool verbose = false;
	optional<bool> color;
	bool dry_run = false;
	optional<string> rootprefix;
	bool activate = false;
	bool probe = true;
	bool yes = false;
    };


    class Backup
    {
    public:

	bool empty() const { return names.empty(); }

	void add(Storage* storage);
	void undo(Storage* storage);

	void dump_last(Storage* storage) const;

    private:

	vector<string> names;

	int num = 0;

    };


    struct State
    {
	State(const GlobalOptions& global_options)
	    : global_options(global_options) {}

	const GlobalOptions& global_options;
	Testsuite* testsuite = nullptr;

	bool run = true;

	bool modified = false;
	bool pools_modified = false;

	Stack stack;
	Backup backup;

	Storage* storage = nullptr;
    };


    struct ParsedCmd
    {
	virtual ~ParsedCmd() = default;

	virtual bool do_backup() const = 0;
	virtual void doit(const GlobalOptions& global_options, State& state) const = 0;
    };


    struct Cmd
    {
	virtual ~Cmd() = default;

	virtual shared_ptr<ParsedCmd> parse(GetOpts& get_opts) const = 0;
	virtual const char* help() const = 0;
	virtual bool is_alias() const { return false; }
	virtual const ExtOptions& options() const { return GetOpts::no_ext_options; }
    };


    struct Parser
    {
	const string name;
	const shared_ptr<Cmd> cmd;
    };


    struct MainCmd : public Parser
    {
	const vector<Parser>& sub_cmds;
    };


    bool
    handle(int argc, char** argv, Testsuite* testsuite = nullptr);

}

#endif
