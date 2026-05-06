/*
 * Copyright (c) 2026 SUSE LLC
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

#include <filesystem>
#include <iomanip>
#include <boost/algorithm/string.hpp>

#include <storage/Devices/LvmVg.h>

#include "CompletionHelper.h"


namespace barrel
{

    void
    CompletionHelper::set_storage(const Storage* r_storage)
    {
	storage = r_storage;
    }


    void
    CompletionHelper::CompletionResult::clear()
    {
	items.clear();
    }


    void
    CompletionHelper::CompletionResult::push(Category category, const string& name, const string& desc,
					     const string& display)
    {
	items.push_back({name, desc, category, display});
    }


    void
    CompletionHelper::CompletionResult::push_command(const string& name, const string& desc)
    {
	push(Category::COMMAND, name, desc);
    }


    void
    CompletionHelper::CompletionResult::push_argument(const string& name, const string& desc,
						      const string& display)
    {
	push(Category::ARGUMENT, name, desc, display);
    }


    void
    CompletionHelper::CompletionResult::push_pool(const string& name)
    {
	push(Category::POOL, name);
    }


    void
    CompletionHelper::CompletionResult::push_device(const string& name)
    {
	push(Category::DEVICE, name);
    }


    string
    CompletionHelper::escape(const string& original)
    {
	string escaped;

	for (char c : original)
	{
	    if (c == ' ')
		escaped += '\\';

	    escaped += c;
	}

	return escaped;
    }


    string
    CompletionHelper::unescape(const string& original)
    {
	string unescaped;

	for (size_t i = 0; i < original.length(); ++i)
	{
	    if (original[i] == '\\' && i + 1 < original.length() && original[i + 1] == ' ')
	    {
		unescaped += ' ';
		++i;
	    }
	    else
		unescaped += original[i];
	}

	return unescaped;
    }


    string
    CompletionHelper::removeAfterNthSlash(string str, int n) const
    {
	size_t pos = 0;
	int count = 0;

	// Find the position of the n-th '/'
	while (count < n && (pos = str.find('/', pos)) != string::npos)
	{
	    count++;
	    if (count < n)
		pos++;
	}

	if (count == n && pos != string::npos)
	    str.erase(pos + 1);

	return str;
    }


    const MainCmd*
    CompletionHelper::find_main_cmd(const string &name) const
    {
	for (const MainCmd& main_cmd : main_cmds)
	{
	    if (main_cmd.name == name)
		return &main_cmd;
	}

	return nullptr;
    }


    pair<const Parser*, size_t>
    CompletionHelper::find_sub_cmd(const MainCmd* cmd, const vector<string> &tokens) const
    {
	for (auto it = tokens.rbegin(); it != tokens.rend(); ++it)
	{
	    for (const Parser& sub_cmd : cmd->sub_cmds)
	    {
		if (sub_cmd.name == *it)
		    return pair(&sub_cmd, tokens.size() - 1 - distance(tokens.rbegin(), it));
	    }
	}

	return pair( nullptr, string::npos );
    }


    const Option*
    CompletionHelper::find_option(const Cmd* cmd, const string &text) const
    {
	if (text.size() < 2 || text[0] != '-' || !cmd)
	    return nullptr;

	// long opt case, starting with --
	if (text[1] == '-'){
	    string opt_name = text.substr(2);
	    for (const Option& opt : cmd->options().options)
	    {
		if (opt.name == opt_name)
		    return &opt;
	    }
	}

	// On short options, we only use the last letter
	char short_opt = text.back();
	for (const Option& opt : cmd->options().options)
	{
	    if (opt.c == short_opt)
		return &opt;
	}

	return nullptr;
    }


    vector<string>
    CompletionHelper::list_files_of_dir(const string &text) const
    {
	vector<string> result;
	error_code ec;
	filesystem::path p(text.empty() ? "./" : text);

	if(!filesystem::is_directory(p, ec))
	    p = p.parent_path();

	try {
	    for (const auto & entry : filesystem::directory_iterator(p))
	    {
		string name = entry.path().string();
		if (entry.is_directory())
		    name += "/";
		result.push_back(name);
	    }
	} catch (...) {
	    // ignore errors (e.g. permission denied)
	}
	return result;
    }


    void
    CompletionHelper::add_devices(CompletionResult &res, const string &text,
	    const vector<string> &all_devices) const
    {
	const ptrdiff_t counted = count(text.begin(), text.end(), '/');
	set<string> blk_matches;

	for (const string& s : all_devices)
	{
	    if (!text.empty() && !boost::starts_with(s, text))
		continue;

	    blk_matches.insert(removeAfterNthSlash(s, counted + 1));
	}

	// If only one match and it's a directory, include its children to avoid it being "taken"
	// as unique immediately, allowing the user to see the next level on subsequent TAB.
	if (blk_matches.size() == 1 && blk_matches.begin()->back() == '/')
	{
	    const string unique_match = *blk_matches.begin();
	    for (const string& s : all_devices)
	    {
		if (s.compare(0, unique_match.length(), unique_match) == 0)
		    blk_matches.insert(removeAfterNthSlash(s, counted + 2));
	    }
	}

	for (const string& m : blk_matches)
	    res.push_device(m);
    }


    const CompletionHelper::CompletionResult&
    CompletionHelper::complete(const vector<string> &tokens, const string &text)
    {
	result.clear();

	const MainCmd* main_cmd = nullptr;
	const Option* option = nullptr;
	const Cmd* cmd = nullptr;

	if (tokens.empty() || !(main_cmd = find_main_cmd(tokens[0])))
	{
	    for (const MainCmd& cmd : main_cmds)
	    {
		if (boost::starts_with(cmd.name, text))
		    result.push_command(
			    cmd.name,
			    cmd.cmd ? cmd.cmd->help() : ""
			    );
	    }
	    return result;
	}

	auto [sub_cmd, sub_cmd_idx] = find_sub_cmd(main_cmd, tokens);

	cmd = sub_cmd ? sub_cmd->cmd.get() : main_cmd->cmd.get();

	option = find_option(cmd, tokens.back());

	if (option && option->has_arg != no_argument)
	{
	    switch (option->value_type)
	    {
		case ValueType::POOL:
		    if (storage)
			for (auto x : storage->get_pools())
			    if (boost::starts_with(x.first, text))
				result.push_pool(x.first);
		    break;

		case ValueType::LVM_VG_NAME:
		    if (storage)
			for (const LvmVg* lvm_vg : LvmVg::get_all(storage->get_system()))
			    if (boost::starts_with(lvm_vg->get_vg_name(), text))
				result.push_pool(lvm_vg->get_vg_name());
		    break;

		case ValueType::STRING_LIST:
		    for (const auto& s : option->possible_values)
			if (boost::starts_with(s, text))
			    result.push_argument(s, "");
		    break;

		case ValueType::PATH:
		    add_devices(result, text, list_files_of_dir(text));
		    break;

		case ValueType::UNKNOWN:
		default:
		    string name = string("<") + (option->arg_name ? option->arg_name : "arg") + ">";
		    result.push_argument(name, option->description ? option->description : "");
		    // TODO try to avoid this extra empty value to get the
		    // completions visible but not taken
		    result.push_argument("", "");
	    }

	    return result;
	}

	for (const Parser& sub_cmd : main_cmd->sub_cmds){
	    if (boost::starts_with(sub_cmd.name, text))
		result.push_command(
			sub_cmd.name,
			sub_cmd.cmd ? sub_cmd.cmd->help() : ""
			);
	}

	if (cmd)
	{
	    for (const Option& option : cmd->options().options){
		// skip deprecated options
		if (!option.description)
		    continue;

		string name = string("--") + option.name;

		// show options only once
		if (find(tokens.begin() + (sub_cmd ? sub_cmd_idx: 1), tokens.end(), name) != tokens.end())
		    continue;

		string desc = option.description;
		stringstream display;
		if (option.c)
		    display << "-" << option.c << " ";
		else
		    display << "   ";
		display << "--" << option.name;
		if (option.has_arg)
		    display << " <" << option.arg_name << ">";

		if (boost::starts_with(name, text))
		    result.push_argument(name, desc, display.str());
	    }

	    if (storage && cmd->options().take_blk_devices != TakeBlkDevices::NO)
		add_devices(result, text.empty() ? "/dev/": text, possible_blk_devices(storage));
	}

	return result;
    }


    ostream& operator<<(ostream& os, CompletionHelper::Category category)
    {
	switch (category) {
	    case CompletionHelper::Category::NONE:     return os << "";
	    case CompletionHelper::Category::COMMAND:  return os << "COMMAND";
	    case CompletionHelper::Category::ARGUMENT: return os << "ARGUMENT";
	    case CompletionHelper::Category::POOL:     return os << "POOL";
	    case CompletionHelper::Category::DEVICE:   return os << "DEVICE";
	    default:                 return os << "Unknown";
	}
    }


    void
    CompletionHelper::display_comp_items(vector<reference_wrapper<const CompItem>> &items) const
    {
	int desc_max_len = 20;

	for (const CompItem& item: items)
	    desc_max_len = max(desc_max_len, static_cast<int>(
			item.display.empty() ? item.name.length() : item.display.length()));

	for (const CompItem& item: items)
	{
	    cout << left << setw(desc_max_len + 2)
		 << (item.display.empty() ? item.name : item.display)
		 << item.desc << endl;
	}
    }


    void
    CompletionHelper::display_matches(char** matches, int num_matches, int max_length) const
    {
	const vector<Category> order = {
	    Category::COMMAND,
	    Category::ARGUMENT,
	    Category::DEVICE,
	    Category::POOL
	};

	map<Category, vector<reference_wrapper<const CompItem>>> grouped;

	for (const auto& item : result.items)
	    grouped[item.category].push_back(cref(item));

	cout << endl;

	for (Category category : order)
	{
	    if (grouped.find(category) == grouped.end())
		continue;

	    if (grouped.size() > 1)
		cout << category << "s:" << endl;

	    display_comp_items(grouped.at(category));

	    if (grouped.size() > 1)
		cout << endl;
	}
    }
}
