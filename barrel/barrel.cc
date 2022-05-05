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


#include <iostream>

#include "config.h"
#include "handle.h"
#include "Utils/BarrelDefines.h"


using namespace barrel;


int
main(int argc, char** argv)
{
    try
    {
	locale::global(locale(""));
    }
    catch (const runtime_error& e)
    {
	cerr << "Failed to set locale." << endl;
    }

    set_logger(get_logfile_logger(LOG_FILE));

    get_logger()->write(LogLevel::MILESTONE, "barrel", __FILE__, __LINE__, __FUNCTION__,
			"barrel version " VERSION);

    if (!handle(argc, argv))
	return EXIT_FAILURE;

    return EXIT_SUCCESS;
}
