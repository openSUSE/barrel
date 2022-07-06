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


#ifndef BARREL_STORAGE_TMPL_H
#define BARREL_STORAGE_TMPL_H


namespace barrel
{

    template <typename Container, typename Value>
    bool contains(const Container& container, const Value& value)
    {
	return find(container.begin(), container.end(), value) != container.end();
    }


    template<typename Type1, typename Type2>
    vector<Type1>
    up_cast(const vector<Type2>& v)
    {
	return vector<Type1>(v.begin(), v.end());
    }

}

#endif
