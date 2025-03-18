/*
 * Copyright (c) [2021-2025] SUSE LLC
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


#include <regex>

#include <storage/Storage.h>
#include <storage/Pool.h>
#include <storage/Devices/BlkDevice.h>
#include <storage/Devices/LvmVg.h>
#include <storage/Devices/LvmLv.h>
#include <storage/Devices/Partitionable.h>
#include <storage/Utils/HumanString.h>

#include "Utils/GetOpts.h"
#include "Utils/Misc.h"
#include "Utils/Text.h"
#include "create-lvm-lv.h"


namespace barrel
{

    using namespace storage;


    namespace
    {

	const ExtOptions create_lvm_lv_options({
	    { "vg-name", required_argument, 'v', _("name of volume group"), "vg-name" },
	    { "name", required_argument, 'n', _("set name of logical volume"), "name" },
	    { "size", required_argument, 's', _("set size of logical volume"), "size" },
	    { "stripes", required_argument, 0, _("number of stripes"), "stripes" },
	    { "stripe-size", required_argument, 0, _("size of stripes"), "stripe-size" }
	});


	struct SmartNumber
	{
	    enum Type { MAX, ABSOLUTE };

	    SmartNumber(const string& str);

	    unsigned int value(unsigned int max) const;

	    Type type = ABSOLUTE;

	    unsigned int absolute = 1;
	};


	SmartNumber::SmartNumber(const string& str)
	{
	    static const regex absolute_rx("([0-9]+)", regex::extended);

	    if (str == "max")
	    {
		type = MAX;
		return;
	    }

	    smatch match;

	    if (regex_match(str, match, absolute_rx))
	    {
		type = ABSOLUTE;

		string n1 = match[1];
		absolute = atoi(n1.c_str());
		return;
	    }

	    throw runtime_error(_("bad stripes argument"));
	}


	unsigned int
	SmartNumber::value(unsigned int max) const
	{
	    switch (type)
	    {
		case SmartNumber::MAX:
		    return max;

		case SmartNumber::ABSOLUTE:
		    return absolute;

		default:
		    throw runtime_error("unknown SmartNumber type");
	    }
	}


	struct Options
	{
	    Options(GetOpts& get_opts);

	    optional<string> vg_name;
	    string lv_name;
	    optional<SmartSize> size;
	    optional<SmartNumber> stripes;
	    optional<unsigned long long> stripe_size;

	    enum class ModusOperandi { LVM_VG, LVM_VG_FROM_STACK };

	    ModusOperandi modus_operandi;

	    void calculate_modus_operandi();
	};


	Options::Options(GetOpts& get_opts)
	{
	    ParsedOpts parsed_opts = get_opts.parse("lv", create_lvm_lv_options);

	    if (parsed_opts.has_option("vg-name"))
		vg_name = parsed_opts.get_optional("vg-name");

	    if (!parsed_opts.has_option("name"))
		throw OptionsException(_("name missing for command 'lv'"));

	    lv_name = parsed_opts.get("name");

	    if (!LvmLv::is_valid_lv_name(lv_name))
		throw OptionsException(_("invalid logical volume name for command 'lv'"));

	    if (!parsed_opts.has_option("size"))
		throw OptionsException(_("size missing for command 'lv'"));

	    string str = parsed_opts.get("size");
	    size = SmartSize(str);

	    if (parsed_opts.has_option("stripes"))
	    {
		string str = parsed_opts.get("stripes");
		stripes = SmartNumber(str);
	    }

	    if (parsed_opts.has_option("stripe-size"))
	    {
		string str = parsed_opts.get("stripe-size");
		stripe_size = humanstring_to_byte(str, false);
	    }

	    calculate_modus_operandi();
	}


	void
	Options::calculate_modus_operandi()
	{
	    if (vg_name)
		modus_operandi = ModusOperandi::LVM_VG;
	    else
		modus_operandi = ModusOperandi::LVM_VG_FROM_STACK;
	}

    }


    class ParsedCmdCreateLvmLv : public ParsedCmd
    {
    public:

	ParsedCmdCreateLvmLv(const Options& options) : options(options) {}

	virtual bool do_backup() const override { return true; }

	virtual void doit(const GlobalOptions& global_options, State& state) const override;

    private:

	const Options options;

    };


    void
    ParsedCmdCreateLvmLv::doit(const GlobalOptions& global_options, State& state) const
    {
	Devicegraph* staging = state.storage->get_staging();

	LvmVg* lvm_vg = nullptr;

	switch (options.modus_operandi)
	{
	    case Options::ModusOperandi::LVM_VG:
	    {
		lvm_vg = LvmVg::find_by_vg_name(staging, options.vg_name.value());
	    }
	    break;

	    case Options::ModusOperandi::LVM_VG_FROM_STACK:
	    {
		Device* device = state.stack.top_as_device(staging);
		if (!is_lvm_vg(device))
		    throw runtime_error(_("not a volume group on stack"));

		lvm_vg = to_lvm_vg(device);
		state.stack.pop();
	    }
	    break;
	}

	for (const LvmLv* lvm_lv : lvm_vg->get_lvm_lvs())
	{
	    if (lvm_lv->get_lv_name() == options.lv_name)
		throw runtime_error(_("name of logical volume already exists"));
	}

	unsigned int stripes = 1;

	if (options.stripes)
	    stripes = options.stripes->value(lvm_vg->get_lvm_pvs().size());

	SmartSize smart_size = options.size.value();

	unsigned long long size = 0;

	switch (smart_size.type)
	{
	    case SmartSize::MAX:
		size = lvm_vg->max_size_for_lvm_lv(LvType::NORMAL);
		break;

	    case SmartSize::ABSOLUTE:
		size = smart_size.value;
		break;

	    case SmartSize::PLUS:
	    case SmartSize::MINUS:
		throw logic_error("invalid SmartSize type");
	}

	LvmLv* lvm_lv = lvm_vg->create_lvm_lv(options.lv_name, LvType::NORMAL, size);

	if (stripes > 1)
	    lvm_lv->set_stripes(stripes);

	if (options.stripe_size)
	    lvm_lv->set_stripe_size(options.stripe_size.value());

	if (lvm_vg->is_overcommitted())
	{
	    throw runtime_error(sformat(_("volume group '%s' is over-committed"), lvm_vg->get_vg_name().c_str()));
	}

	state.stack.push(lvm_lv);
	state.modified = true;
    }


    shared_ptr<ParsedCmd>
    CmdCreateLvmLv::parse(GetOpts& get_opts) const
    {
	Options options(get_opts);

	return make_shared<ParsedCmdCreateLvmLv>(options);
    }


    const char*
    CmdCreateLvmLv::help() const
    {
	return _("Creates a new LVM logical volume.");
    }


    const ExtOptions&
    CmdCreateLvmLv::options() const
    {
	return create_lvm_lv_options;
    }

}
