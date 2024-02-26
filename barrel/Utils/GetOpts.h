/*
 * Copyright (c) [2011-2015] Novell, Inc.
 * Copyright (c) [2016-2021] SUSE LLC
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


#ifndef BARREL_GET_OPTS_H
#define BARREL_GET_OPTS_H


#include <getopt.h>
#include <string>
#include <map>
#include <vector>
#include <optional>
#include <iostream>
#include <boost/algorithm/string.hpp>

#include "Text.h"


namespace barrel
{

    using namespace std;


    struct OptionsException : public runtime_error
    {
	OptionsException(const string& msg) : runtime_error(msg) {}
    };


    struct Option
    {
	const char* name;
	int has_arg;
	char c;
	const char* description;
	const char* arg_name = nullptr;
    };


    enum class TakeBlkDevices
    {
	NO, YES, MAYBE
    };


    struct ExtOptions
    {
	explicit ExtOptions(const vector<Option>& options, TakeBlkDevices take_blk_devices = TakeBlkDevices::NO)
	    : options(options), take_blk_devices(take_blk_devices)
	{
	}

	const vector<Option> options;
	const TakeBlkDevices take_blk_devices;
    };


    class ParsedOpts
    {
    public:

	ParsedOpts(const map<string, string>& args, const vector<string>& blk_devices)
	    : args(args), blk_devices(blk_devices)
	{
	}

	using const_iterator = map<string, string>::const_iterator;

	bool has_option(const string& name) const { return args.find(name) != args.end(); }

	const_iterator find(const string& name) const { return args.find(name); }

	const_iterator end() const { return args.end(); }

	const string& get(const string& name) const;

	const optional<string> get_optional(const string& name) const;

	bool has_blk_devices() const { return !blk_devices.empty(); }

	const vector<string>& get_blk_devices() const { return blk_devices; }

    private:

	const map<string, string> args;
	const vector<string> blk_devices;

    };


    /**
     * A parser for command line options.
     */
    class GetOpts
    {
    public:

	static const ExtOptions no_ext_options;

	GetOpts(int argc, char** argv, bool glob_blk_devices = false, const vector<string>& all_blk_devices = {});

	ParsedOpts parse(const ExtOptions& ext_options);
	ParsedOpts parse(const char* command, const ExtOptions& ext_options);

	bool has_args() const;

	const char* pop_arg();

    private:

	int argc;
	char** argv;

	const bool glob_blk_devices;
	const vector<string> all_blk_devices;

	string make_optstring(const vector<Option>& options) const;
	vector<struct option> make_longopts(const vector<Option>& options) const;

	vector<Option>::const_iterator find(const vector<Option>& options, char c) const;

	static bool is_blk_device(const string& name);

    };


    template<typename Type>
    typename vector<Type>::const_iterator
    sloppy_find(const vector<Type>& v, const char* name)
    {
	typename vector<Type>::const_iterator best = v.end();

	for (typename vector<Type>::const_iterator it = v.begin(); it != v.end(); ++it)
	{
	    if (it->name == name)
		return it;

	    if (boost::starts_with(it->name, name))
	    {
		if (best != v.end())
		{
		    for (typename vector<Type>::const_iterator it2 = v.begin(); it2 != v.end(); ++it2)
		    {
			if (boost::starts_with(it2->name, name))
			    cerr << it2->name << " ";
		    }
		    cerr << endl;

		    throw runtime_error(_("ambiguous command or sub command"));
		}

		best = it;
	    }
	}

	if (best == v.end())
	    throw runtime_error(sformat(_("unknown command or sub command '%s'"), name));

	return best;
    }

}

#endif
