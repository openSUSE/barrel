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


#include <fnmatch.h>

#include "GetOpts.h"
#include "Text.h"
#include "BarrelDefines.h"


using namespace std;


namespace barrel
{

    const struct ExtOptions GetOpts::no_ext_options({}, TakeBlkDevices::NO);


    GetOpts::GetOpts(int argc, char** argv, bool glob_blk_devices, const vector<string>& all_blk_devices)
	: argc(argc), argv(argv), glob_blk_devices(glob_blk_devices), all_blk_devices(all_blk_devices)
    {
	opterr = 0;		// disable error messages from getopt
	optind = 0;
    }


    ParsedOpts
    GetOpts::parse(const ExtOptions& ext_options)
    {
	return parse(nullptr, ext_options);
    }


    ParsedOpts
    GetOpts::parse(const char* command, const ExtOptions& ext_options)
    {
	string optstring = make_optstring(ext_options.options);
	vector<struct option> longopts = make_longopts(ext_options.options);

	map<string, string> result;
	vector<string> blk_devices;

	while (true)
	{
	    int option_index = 0;
	    int c = getopt_long(argc, argv, optstring.c_str(), &longopts[0], &option_index);

	    switch (c)
	    {
		case -1:
		{
		    if (!has_args() || !is_blk_device(argv[optind]))
			return ParsedOpts(result, blk_devices);

		    if (ext_options.take_blk_devices == TakeBlkDevices::NO)
		    {
			string opt;
			if (optopt != 0)
			    opt = string("-") + (char)(optopt);
			else
			    opt = argv[optind - 1];

			string msg;
			if (!command)
			    msg = _("No global block devices allowed.");
			else
			    msg = sformat(_("No block devices allowed for command option '%s'."), opt.c_str());

			throw OptionsException(msg);
		    }

		    bool take_original = true;
		    string original = argv[optind++];

		    if (glob_blk_devices)
		    {
			for (const string& name : all_blk_devices)
			{
			    if (fnmatch(original.c_str(), name.c_str(), 0) == 0)
			    {
				take_original = false;
				blk_devices.push_back(name);
			    }
			}
		    }

		    if (take_original)
			blk_devices.push_back(original);
		}
		break;

		case '?':
		{
		    string opt;
		    if (optopt != 0)
			opt = string("-") + (char)(optopt);
		    else
			opt = argv[optind - 1];

		    string msg;
		    if (!command)
			msg = sformat(_("Unknown global option '%s'."), opt.c_str());
		    else
			msg = sformat(_("Unknown option '%s' for command '%s'."), opt.c_str(), command);

		    throw OptionsException(msg);
		}
		break;

		case ':':
		{
		    string opt;
		    if (optopt != 0)
		    {
			vector<Option>::const_iterator it = find(ext_options.options, optopt);
			if (it == ext_options.options.end())
			    throw OptionsException("option not found");

			opt = string("--") + it->name;
		    }
		    else
		    {
			opt = argv[optind - 1];
		    }

		    string msg;
		    if (!command)
			msg = sformat(_("Missing argument for global option '%s'."), opt.c_str());
		    else
			msg = sformat(_("Missing argument for command option '%s'."), opt.c_str());

		    throw OptionsException(msg);
		}
		break;

		default:
		{
		    vector<Option>::const_iterator it = c ? find(ext_options.options, c) :
			ext_options.options.begin() + option_index;
		    if (it == ext_options.options.end())
			throw OptionsException("option not found");

		    result[it->name] = optarg ? optarg : "";
		}
		break;
	    }
	}

	return ParsedOpts(result, blk_devices);
    }


    bool
    GetOpts::has_args() const
    {
	return argc > optind;
    }


    const char*
    GetOpts::pop_arg()
    {
	return argv[optind++];
    }


    vector<Option>::const_iterator
    GetOpts::find(const vector<Option>& options, char c) const
    {
	return find_if(options.begin(), options.end(), [c](const Option& option)
	    { return option.c == c; }
	);
    }


    string
    GetOpts::make_optstring(const vector<Option>& options) const
    {
	// '+' - do not permute, stop at the 1st non-option, which is the command or an argument
	// ':' - return ':' to indicate missing arg, not '?'
	string optstring = "+:";

	for (const Option& option : options)
	{
	    if (option.c == 0)
		continue;

	    optstring += option.c;

	    switch (option.has_arg)
	    {
		case no_argument:
		    break;

		case required_argument:
		    optstring += ':';
		    break;
	    }
	}

	return optstring;
    }


    vector<struct option>
    GetOpts::make_longopts(const vector<Option>& options) const
    {
	vector<struct option> ret;

	for (const Option& option : options)
	    ret.push_back({ option.name, option.has_arg, nullptr, option.c });

	ret.push_back({ nullptr, 0, nullptr, 0 });

	return ret;
    }


    bool
    GetOpts::is_blk_device(const string& name)
    {
	return boost::starts_with(name, DEV_DIR "/");
    }


    const string&
    ParsedOpts::get(const string& name) const
    {
	map<string, string>::const_iterator it = args.find(name);

	if (it == args.end())
	{
	    cerr << "missing argument " << name << endl;
	    throw runtime_error("missing argument");
	}

	if (it->second.empty())
	{
	    cerr << "missing argument " << name << endl;
	    throw runtime_error("missing argument");
	}

	return it->second;
    }


    const optional<string>
    ParsedOpts::get_optional(const string& name) const
    {
	map<string, string>::const_iterator it = args.find(name);

	optional<string> ret;

	if (it != args.end())
	    ret = it->second;

	return ret;
    }

}
