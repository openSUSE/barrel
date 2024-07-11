/*
 * Copyright (c) [2021-2024] SUSE LLC
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
#include <storage/Pool.h>
#include <storage/Devices/Luks.h>
#include <storage/Devices/BlkDevice.h>
#include <storage/Devices/PartitionTable.h>
#include <storage/Devices/Partitionable.h>
#include <storage/Devices/Partition.h>

#include "Utils/GetOpts.h"
#include "Utils/Text.h"
#include "Utils/Prompt.h"
#include "create-encryption.h"


namespace barrel
{

    using namespace storage;


    namespace
    {

	const ExtOptions create_encryption_options({
	    { "type", required_argument, 't', _("encryption type"), "type" },
	    { "name", required_argument, 'n', _("set name of device"), "name" },
	    { "label", required_argument, 0, _("set label of device"), "label" },
	    { "activate-by", required_argument, 0, _("activate by"), "type" },
	    { "activate-options", required_argument, 'o', _("activate options"), "options" },
	    { "pool-name", required_argument, 0, _("pool name"), "name" },
	    { "size", required_argument, 's', _("set size"), "size" },
	    { "key-file", required_argument, 0, _("set a key file"), "key-file" },
	    { "no-crypttab", no_argument, 0, _("do not add in /etc/crypttab") },
	    { "force", no_argument, 0, _("force if block devices are in use") }
	}, TakeBlkDevices::MAYBE);


	const map<string, EncryptionType> str_to_encryption_type = {
	    { "luks1", EncryptionType::LUKS1 },
	    { "luks2", EncryptionType::LUKS2 }
	};


	const map<string, MountByType> str_to_activate_by_type = {
	    { "device", MountByType::DEVICE },
	    { "path", MountByType::PATH },
	    { "id", MountByType::ID },
	    { "uuid", MountByType::UUID },
	    { "label", MountByType::LABEL },
#if 0
	    { "partuuid", MountByType::PARTUUID },
	    { "partlabel", MountByType::PARTLABEL }
#endif
	};


	struct Options
	{
	    Options(GetOpts& get_opts);

	    optional<EncryptionType> type;
	    string name;
	    optional<string> label;
	    optional<MountByType> activate_by;
	    optional<vector<string>> activate_options;
	    optional<string> pool_name;
	    optional<SmartSize> size;
	    optional<string> key_file;
	    bool crypttab = true;
	    bool force = false;

	    vector<string> blk_devices;

	    enum class ModusOperandi { BLK_DEVICE, POOL, PARTITION_TABLE_FROM_STACK, BLK_DEVICE_FROM_STACK,
		PARTITIONABLE };

	    ModusOperandi modus_operandi;

	    void calculate_modus_operandi();

	    void check() const;
	};


	Options::Options(GetOpts& get_opts)
	{
	    ParsedOpts parsed_opts = get_opts.parse("encryption", create_encryption_options);

	    if (parsed_opts.has_option("type"))
	    {
		string str = parsed_opts.get("type");

		map<string, EncryptionType>::const_iterator it = str_to_encryption_type.find(str);
		if (it == str_to_encryption_type.end())
		    throw runtime_error(_("unknown encryption type"));

		type = it->second;
	    }

	    if (!parsed_opts.has_option("name"))
		throw OptionsException(_("name missing for command 'encryption'"));

	    name = parsed_opts.get("name");

	    label = parsed_opts.get_optional("label");

	    if (parsed_opts.has_option("activate-by"))
	    {
		string str = parsed_opts.get("activate-by");

		map<string, MountByType>::const_iterator it = str_to_activate_by_type.find(str);
		if (it == str_to_activate_by_type.end())
		    throw runtime_error(sformat(_("unknown activate-by type '%s'"), str.c_str()));

		activate_by = it->second;
	    }

	    if (parsed_opts.has_option("activate-options"))
	    {
		string str = parsed_opts.get("activate-options");

		vector<string> tmp;
		boost::split(tmp, str, boost::is_any_of(","), boost::token_compress_on);
		activate_options = tmp;
	    }

	    pool_name = parsed_opts.get_optional("pool-name");

	    if (parsed_opts.has_option("size"))
	    {
		string str = parsed_opts.get("size");
		size = SmartSize(str);
	    }

	    if (parsed_opts.has_option("key-file"))
		key_file = parsed_opts.get("key-file");

	    crypttab = !parsed_opts.has_option("no-crypttab");

	    force = parsed_opts.has_option("force");

	    blk_devices = parsed_opts.get_blk_devices();

	    calculate_modus_operandi();
	}


	void
	Options::calculate_modus_operandi()
	{
	    // TODO identical in create-lvm-vg.cc

	    if (pool_name)
	    {
		if (!size)
		    throw runtime_error(_("size argument missing for command 'encryption'"));

		if (!blk_devices.empty())
		    throw runtime_error(_("pool argument and blk devices not allowed together for command 'encryption'"));

		modus_operandi = ModusOperandi::POOL;
	    }
	    else
	    {
		if (size)
		{
		    if (blk_devices.empty())
			modus_operandi = ModusOperandi::PARTITION_TABLE_FROM_STACK;
		    else
			modus_operandi = ModusOperandi::PARTITIONABLE;
		}
		else
		{
		    if (blk_devices.empty())
			modus_operandi = ModusOperandi::BLK_DEVICE_FROM_STACK;
		    else
			modus_operandi = ModusOperandi::BLK_DEVICE;
		}
	    }
	}


	void
	Options::check() const
	{
	}

    }


    class ParsedCmdCreateEncryption : public ParsedCmd
    {
    public:

	ParsedCmdCreateEncryption(const Options& options)
	    : options(options)
	{
	    options.check();
	}

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

	BlkDevice* blk_device = nullptr;

	switch (options.modus_operandi)
	{
	    case Options::ModusOperandi::BLK_DEVICE_FROM_STACK:
	    {
		Device* device = state.stack.top_as_device(staging);
		if (!is_blk_device(device))
		    throw runtime_error(_("not a block device on stack"));

		blk_device = to_blk_device(device);
		state.stack.pop();
	    }
	    break;

	    case Options::ModusOperandi::POOL:
	    {
		Pool* pool = state.storage->get_pool(options.pool_name.value());

		blk_device = PartitionCreator::create_partition(pool, staging, options.size.value());
	    }
	    break;

	    case Options::ModusOperandi::PARTITION_TABLE_FROM_STACK:
	    {
		const Device* device = state.stack.top_as_device(staging);
		if (!is_partition_table(device))
		    throw runtime_error(_("not a partition table on stack"));

		const PartitionTable* partition_table = to_partition_table(device);
		state.stack.pop();

		Pool pool;

		pool.add_device(partition_table->get_partitionable());

		blk_device = PartitionCreator::create_partition(&pool, staging, options.size.value());
	    }
	    break;

	    case Options::ModusOperandi::BLK_DEVICE:
	    {
		if (options.blk_devices.size() != 1)
		    throw runtime_error(_("only one block device allowed"));

		blk_device = BlkDevice::find_by_name(staging, options.blk_devices.front());
	    }
	    break;

	    case Options::ModusOperandi::PARTITIONABLE:
	    {
		Pool pool;

		if (options.blk_devices.size() != 1)
		    throw runtime_error(_("wrong number of partitionables"));

		for (const string& device_name : options.blk_devices)
		{
		    const Partitionable* partitionable = Partitionable::find_by_name(staging, device_name);
		    pool.add_device(partitionable);
		}

		blk_device = PartitionCreator::create_partition(&pool, staging, options.size.value());
	    }
	    break;
	}

	check_usable(blk_device, options.force);

	string password = !options.key_file ? prompt_password(true) : "";

	Encryption* encryption = blk_device->create_encryption(dm_name, type);
	if (!options.key_file)
	    encryption->set_password(password);
	else
	    encryption->set_key_file(options.key_file.value());

	encryption->set_in_etc_crypttab(options.crypttab);

	if (is_luks(encryption))
	{
	    Luks* luks = to_luks(encryption);

	    if (options.label)
		luks->set_label(options.label.value());

	    if (options.activate_by)
		luks->set_mount_by(options.activate_by.value());

	    if (options.activate_options)
		luks->set_crypt_options(options.activate_options.value());
	}

	state.stack.push(encryption);
	state.modified = true;
    }


    shared_ptr<ParsedCmd>
    parse_create_encryption(GetOpts& get_opts, EncryptionType type)
    {
	Options options(get_opts);

	if (options.type)
	    throw OptionsException(_("encryption type already set for command 'encryption'"));

	options.type = type;

	return make_shared<ParsedCmdCreateEncryption>(options);
    }


    shared_ptr<ParsedCmd>
    CmdCreateEncryption::parse(GetOpts& get_opts) const
    {
	Options options(get_opts);

	if (!options.type)
	    throw OptionsException(_("encryption type missing for command 'encryption'"));

	return make_shared<ParsedCmdCreateEncryption>(options);
    }


    const char*
    CmdCreateEncryption::help() const
    {
	return _("Creates a new encryption device.");
    }


    const ExtOptions&
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
