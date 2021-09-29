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
#include <storage/Devices/Luks.h>
#include <storage/Devices/BlkDevice.h>

#include "Utils/GetOpts.h"
#include "Utils/Text.h"
#include "create-encryption.h"


namespace barrel
{

    using namespace storage;


    namespace
    {

	const vector<Option> create_encryption_options = {
	    { "type", required_argument, 't', _("encryption type"), "type" },
	    { "name", required_argument, 'n', _("set name of device"), "name" },
	    { "password", required_argument, 'p', "set password", "password" } // TODO drop, read from stdin
	};


	const map<string, EncryptionType> str_to_encryption_type = {
	    { "luks1", EncryptionType::LUKS1 },
	    { "luks2", EncryptionType::LUKS2 }
	};


	struct Options
	{
	    Options(GetOpts& get_opts);

	    optional<EncryptionType> type;
	    string name;
	    string password;
	};


	Options::Options(GetOpts& get_opts)
	{
	    ParsedOpts parsed_opts = get_opts.parse("encryption", create_encryption_options);

	    if (parsed_opts.has_option("type"))
	    {
		string str = parsed_opts.get("type");

		map<string, EncryptionType>::const_iterator it = str_to_encryption_type.find(str);
		if (it == str_to_encryption_type.end())
		    throw runtime_error("unknown encryption type");

		type = it->second;
	    }

	    if (!parsed_opts.has_option("name"))
		throw OptionsException("name missing");

	    name = parsed_opts.get("name");

	    if (!parsed_opts.has_option("password"))
		throw OptionsException("password missing");

	    password = parsed_opts.get("password");
	}

    }


    class ParsedCmdCreateEncryption : public ParsedCmd
    {
    public:

	ParsedCmdCreateEncryption(const Options& options) : options(options) {}

	virtual bool do_backup() const override { return true; }

	virtual void doit(const GlobalOptions& global_options, State& state) const override;

    private:

	const Options options;

    };


    void
    ParsedCmdCreateEncryption::doit(const GlobalOptions& global_options, State& state) const
    {
	Devicegraph* staging = state.storage->get_staging();

	EncryptionType type = options.type.value();

	string dm_name = options.name;

	string password = options.password;

	BlkDevice* blk_device = to_blk_device(state.stack.top(staging));
	state.stack.pop();

	Encryption* encryption = blk_device->create_encryption(dm_name, type);
	encryption->set_password(password);

	state.stack.push(encryption);
	state.modified = true;
    }


    shared_ptr<ParsedCmd>
    parse_create_encryption(GetOpts& get_opts, EncryptionType type)
    {
	Options options(get_opts);

	if (options.type)
	    throw OptionsException("encryption type already set");

	options.type = type;

	return make_shared<ParsedCmdCreateEncryption>(options);
    }


    shared_ptr<ParsedCmd>
    CmdCreateEncryption::parse(GetOpts& get_opts) const
    {
	Options options(get_opts);

	if (!options.type)
	    throw OptionsException("encryption type missing");

	return make_shared<ParsedCmdCreateEncryption>(options);
    }


    const char*
    CmdCreateEncryption::help() const
    {
	return _("Create an encryption");
    }


    const vector<Option>&
    CmdCreateEncryption::options() const
    {
	return create_encryption_options;
    }


    shared_ptr<ParsedCmd>
    CmdCreateLuks1::parse(GetOpts& get_opts) const
    {
	return parse_create_encryption(get_opts, EncryptionType::LUKS1);
    }


    const char*
    CmdCreateLuks1::help() const
    {
	return _("Alias for 'create encryption --type luks1'");
    }


    shared_ptr<ParsedCmd>
    CmdCreateLuks2::parse(GetOpts& get_opts) const
    {
	return parse_create_encryption(get_opts, EncryptionType::LUKS2);
    }


    const char*
    CmdCreateLuks2::help() const
    {
	return _("Alias for 'create encryption --type luks2'");
    }

}
