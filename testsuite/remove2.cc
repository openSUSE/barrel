
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE barrel

#include <boost/test/unit_test.hpp>

#include <storage/Actiongraph.h>

#include "../barrel/handle.h"
#include "../barrel/Utils/Args.h"
#include "helpers/output.h"


using namespace std;
using namespace storage;
using namespace barrel;


BOOST_AUTO_TEST_CASE(test1)
{
    // Check that removing an extended partition also removes the logical partitions.

    Args args({ "--dry-run", "--yes" });

    vector<string> actions = {
	"Delete logical partition /dev/sdd7 (59.62 GiB)",
	"Delete logical partition /dev/sdd6 (59.62 GiB)",
	"Delete logical partition /dev/sdd5 (59.62 GiB)",
	"Delete extended partition /dev/sdd3 (178.85 GiB)",
	"Delete primary partition /dev/sdd2 (59.62 GiB)",
	"Delete primary partition /dev/sdd1 (59.62 GiB)"
    };

    Testsuite testsuite;
    testsuite.devicegraph_filename = "msdos1.xml";

    testsuite.readlines = {
	"remove device /dev/sdd[1-4]",
	"commit"
    };

    handle(args.argc(), args.argv(), &testsuite);

    BOOST_CHECK_EQUAL(actions, testsuite.actions); // TODO sort
}


BOOST_AUTO_TEST_CASE(test2)
{
    // Check that removing several partitions works despite the renumbering (MS-DOS
    // logical partitions).

    Args args({ "--dry-run", "--yes" });

    vector<string> actions = {
	"Delete logical partition /dev/sdd7 (59.62 GiB)",
	"Delete logical partition /dev/sdd6 (59.62 GiB)",
	"Delete logical partition /dev/sdd5 (59.62 GiB)",
    };

    Testsuite testsuite;
    testsuite.devicegraph_filename = "msdos1.xml";

    testsuite.readlines = {
	"remove device /dev/sdd[5-7]",
	"commit"
    };

    handle(args.argc(), args.argv(), &testsuite);

    BOOST_CHECK_EQUAL(actions, testsuite.actions); // TODO sort
}


BOOST_AUTO_TEST_CASE(test3)
{
    // Check that removing several partitions works despite the renumbering (DASD
    // partitions).

    Args args({ "--dry-run", "--yes" });

    vector<string> actions = {
	"Remove mount point / of /dev/dasdb3 (5.72 GiB) from /etc/fstab",
	"Remove mount point swap of /dev/dasdb2 (990.00 MiB) from /etc/fstab",
	"Unmount /dev/dasdb2 (990.00 MiB) at swap",
	"Delete swap on /dev/dasdb2 (990.00 MiB)",
	"Remove mount point /boot/zipl of /dev/dasdb1 (199.97 MiB) from /etc/fstab",
	"Unmount /dev/dasdb1 (199.97 MiB) at /boot/zipl",
	"Unmount /dev/dasdb3 (5.72 GiB) at /",
	"Delete ext4 on /dev/dasdb3 (5.72 GiB)",
	"Delete partition /dev/dasdb3 (5.72 GiB)",
	"Delete partition /dev/dasdb2 (990.00 MiB)",
	"Delete ext2 on /dev/dasdb1 (199.97 MiB)",
	"Delete partition /dev/dasdb1 (199.97 MiB)"
    };

    Testsuite testsuite;
    testsuite.devicegraph_filename = "real3.xml";

    testsuite.readlines = {
	"remove device /dev/dasdb[1-3]",
	"commit"
    };

    handle(args.argc(), args.argv(), &testsuite);

    BOOST_CHECK_EQUAL(actions, testsuite.actions); // TODO sort
}
