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
#include "create-encryption.h"


namespace barrel
{

    using namespace storage;


    namespace
    {

	const map<string, EncryptionType> str_to_encryption_type = {
	    { "luks1", EncryptionType::LUKS1 },
	    { "luks2", EncryptionType::LUKS2 }
	};


	struct Options
	{
	    Options(GetOpts& get_opts);

	    optional<EncryptionType> type;
	    optional<string> name;
	    optional<string> password;
	};


	Options::Options(GetOpts& get_opts)
	{
	    const vector<Option> options = {
		{ "type", required_argument, 't' },
		{ "name", required_argument, 'n' },
		{ "password", required_argument, 'p' } // TODO drop, read from stdin
	    };

	    ParsedOpts parsed_opts = get_opts.parse("encryption", options);

	    if (parsed_opts.has_option("type"))
	    {
		string str = parsed_opts.get("type");

		map<string, EncryptionType>::const_iterator it = str_to_encryption_type.find(str);
		if (it == str_to_encryption_type.end())
		    throw runtime_error("unknown encryption type");

		type = it->second;
	    }

	    if (parsed_opts.has_option("name"))
	    {
		name = parsed_opts.get("name");
	    }

	    if (parsed_opts.has_option("password"))
	    {
		password = parsed_opts.get("password");
	    }
	}

    }


    class CmdCreateEncryption : public Cmd
    {
    public:

	CmdCreateEncryption(const Options& options) : options(options) {}

	virtual bool do_backup() const override { return true; }

	virtual void doit(State& state) const override;

    private:

	const Options options;

    };


    void
    CmdCreateEncryption::doit(State& state) const
    {
	Devicegraph* staging = state.storage->get_staging();

	EncryptionType type = options.type.value();

	if (!options.name)
	    throw runtime_error("name missing");

	string dm_name = options.name.value();

	if (!options.password)
	    throw runtime_error("password missing");

	string password = options.password.value();

	BlkDevice* blk_device = to_blk_device(state.stack.top(staging));
	state.stack.pop();

	Encryption* encryption = blk_device->create_encryption(dm_name, type);
	encryption->set_password(password);

	state.stack.push(encryption);
	state.modified = true;
    }


    shared_ptr<Cmd>
    parse_create_encryption(GetOpts& get_opts)
    {
	Options options(get_opts);

	if (!options.type)
	    throw OptionsException("encryption type missing");

	return make_shared<CmdCreateEncryption>(options);
    }


    shared_ptr<Cmd>
    parse_create_encryption(GetOpts& get_opts, EncryptionType type)
    {
	Options options(get_opts);

	if (options.type)
	    throw OptionsException("encryption type already set");

	options.type = type;

	return make_shared<CmdCreateEncryption>(options);
    }


    shared_ptr<Cmd>
    parse_create_luks1(GetOpts& get_opts)
    {
	return parse_create_encryption(get_opts, EncryptionType::LUKS1);
    }


    shared_ptr<Cmd>
    parse_create_luks2(GetOpts& get_opts)
    {
	return parse_create_encryption(get_opts, EncryptionType::LUKS2);
    }

}
