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


#ifndef BARREL_HANDLE_H
#define BARREL_HANDLE_H


#include <string>

#include <storage/Actiongraph.h>
#include <storage/Devices/BlkDevice.h>

#include "Utils/GetOpts.h"
#include "Utils/Misc.h"


namespace barrel
{

    using namespace std;
    using namespace storage;


    struct GlobalOptions
    {
	GlobalOptions(GetOpts& get_opts);

	bool verbose = false;
	bool dry_run = false;
	string prefix;
	bool activate = false;
	bool probe = true;
	bool yes = false;
    };


    class Stack
    {
    public:

	size_t size() const { return data.size(); }
	bool empty() const { return data.empty(); }

	vector<sid_t>::const_iterator begin() const { return data.begin(); }
	vector<sid_t>::const_iterator end() const { return data.end(); }

	void pop() { data.pop_back(); }
	void dup() { data.push_back(data.back()); }

	Device* top(Devicegraph* devicegraph);
	void push(Device* device) { data.push_back(device->get_sid()); }

    private:

	vector<sid_t> data;

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
	const Testsuite* testsuite = nullptr;

	bool run = true;

	bool modified = false;

	Stack stack;
	Backup backup;

	Storage* storage = nullptr;
    };


    struct Cmd
    {
	virtual ~Cmd() = default;

	virtual bool do_backup() const = 0;

	virtual void doit(const GlobalOptions& global_options, State& state) const = 0;
    };


    bool
    handle(int argc, char** argv, const Testsuite* testsuite = nullptr);

}

#endif
