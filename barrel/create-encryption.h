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


#ifndef BARREL_CREATE_ENCRYPTION_H
#define BARREL_CREATE_ENCRYPTION_H


#include "handle.h"


namespace barrel
{

    struct CmdCreateEncryption : public Cmd
    {
	virtual shared_ptr<ParsedCmd> parse(GetOpts& get_opts) const override;
	virtual const char* help() const override;
	virtual const vector<Option>& options() const override;
    };


    struct CmdCreateLuks1 : public CmdCreateEncryption
    {
	virtual shared_ptr<ParsedCmd> parse(GetOpts& get_opts) const override;
	virtual const char* help() const override;
	virtual bool is_alias() const override { return true; }
    };


    struct CmdCreateLuks2 : public CmdCreateEncryption
    {
	virtual shared_ptr<ParsedCmd> parse(GetOpts& get_opts) const override;
	virtual const char* help() const override;
	virtual bool is_alias() const override { return true; }
    };

}

#endif
