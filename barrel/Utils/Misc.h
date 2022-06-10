/*
 * Copyright (c) [2021-2022] SUSE LLC
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


#ifndef BARREL_MISC_H
#define BARREL_MISC_H


#include <string>
#include <vector>
#include <functional>

#include <storage/Storage.h>
#include <storage/Devices/BlkDevice.h>
#include <storage/Pool.h>


namespace barrel
{

    using namespace std;
    using namespace storage;


    string
    format_size(unsigned long long size, bool omit_zeroes = false);


    string
    format_percentage(unsigned long long a, unsigned long long b);


    /**
     * Class to parse a size, e.g. "2 TiB". The special value "max" is also
     * allowed.
     */
    struct SmartSize
    {
	enum Type { MAX, ABSOLUTE };

	SmartSize(const string& str);

	unsigned long long value(unsigned long long max) const;

	Type type;

	unsigned long long absolute;
    };


    template<typename Type1, typename Type2>
    vector<Type1>
    up_cast(const vector<Type2>& v)
    {
	return vector<Type1>(v.begin(), v.end());
    }


    vector<string>
    possible_blk_devices(const Storage* storage);


    struct Testsuite
    {
	string devicegraph_filename;

	vector<string> readlines;

	std::function<void(const Actiongraph*)> save_actiongraph = nullptr;
    };


    void
    remove_pools(Storage* storage);


    void
    pimp_pool(Pool* pool, const BlkDevice* blk_device);


    /**
     * This class helps to restore the staging devicegraph in the case of an exception.
     */
    class StagingGuard
    {
    public:

	StagingGuard(Storage* storage);
	~StagingGuard();

	void release();

    private:

	const string name = "barrel-staging-guard";

	Storage* storage;

    };

}

#endif
