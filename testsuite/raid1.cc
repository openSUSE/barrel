
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE barrel

#include <boost/test/unit_test.hpp>

#include <storage/Actiongraph.h>
#include <storage/Devices/Md.h>

#include "../barrel/handle.h"
#include "../barrel/Utils/Args.h"
#include "helpers/output.h"


using namespace std;
using namespace storage;
using namespace barrel;


BOOST_AUTO_TEST_CASE(test1)
{
    Args args({ "--dry-run", "--yes", "create", "raid", "--level", "5", "--pool-name=HDDs (512 B)",
	    "--devices", "3+1", "--size", "8 GiB" });

    vector<string> actions = {
	"Create partition /dev/sdb1 (4.07 GiB)",
	"Set id of partition /dev/sdb1 to Linux RAID",
	"Create partition /dev/sdc1 (4.07 GiB)",
	"Set id of partition /dev/sdc1 to Linux RAID",
	"Create partition /dev/sdd1 (4.07 GiB)",
	"Set id of partition /dev/sdd1 to Linux RAID",
	"Create partition /dev/sde1 (4.07 GiB)",
	"Set id of partition /dev/sde1 to Linux RAID",
	"Create MD RAID5 /dev/md0 (8.00 GiB) from /dev/sdb1 (4.07 GiB), /dev/sdc1 (4.07 GiB), /dev/sdd1 (4.07 GiB) and /dev/sde1 (4.07 GiB)",
	"Add /dev/md0 to /etc/mdadm.conf"
    };

    Testsuite testsuite;
    testsuite.devicegraph_filename = "empty2.xml";

    handle(args.argc(), args.argv(), &testsuite);

    BOOST_CHECK_EQUAL(actions, testsuite.actions); // TODO sort

    const Devicegraph* staging = testsuite.storage->get_staging();

    const Md* md0 = Md::find_by_name(staging, "/dev/md0");
    BOOST_CHECK_EQUAL(md0->get_metadata(), "default");
}


BOOST_AUTO_TEST_CASE(test2)
{
    // Here the shell does the expansion of /dev/sd[b-e].
    // barrel --dry-run create raid5 /dev/sd[b-e] --size "8 GiB"

    Args args({ "--dry-run", "--yes", "create", "raid5", "/dev/sdb", "/dev/sdc", "/dev/sdd", "/dev/sde",
		"--size", "8 GiB" });

    vector<string> actions = {
	"Create partition /dev/sdb1 (2.71 GiB)",
	"Set id of partition /dev/sdb1 to Linux RAID",
	"Create partition /dev/sdc1 (2.71 GiB)",
	"Set id of partition /dev/sdc1 to Linux RAID",
	"Create partition /dev/sdd1 (2.71 GiB)",
	"Set id of partition /dev/sdd1 to Linux RAID",
	"Create partition /dev/sde1 (2.71 GiB)",
	"Set id of partition /dev/sde1 to Linux RAID",
	"Create MD RAID5 /dev/md0 (8.00 GiB) from /dev/sdb1 (2.71 GiB), /dev/sdc1 (2.71 GiB), /dev/sdd1 (2.71 GiB) and /dev/sde1 (2.71 GiB)",
	"Add /dev/md0 to /etc/mdadm.conf"
    };

    Testsuite testsuite;
    testsuite.devicegraph_filename = "empty2.xml";

    handle(args.argc(), args.argv(), &testsuite);

    BOOST_CHECK_EQUAL(actions, testsuite.actions); // TODO sort
}


BOOST_AUTO_TEST_CASE(test3)
{
    Args args({ "--dry-run", "--yes" });

    vector<string> actions = {
	"Create partition /dev/sdb1 (4.07 GiB)",
	"Set id of partition /dev/sdb1 to Linux RAID",
	"Create partition /dev/sdc1 (4.07 GiB)",
	"Set id of partition /dev/sdc1 to Linux RAID",
	"Create partition /dev/sdd1 (4.07 GiB)",
	"Set id of partition /dev/sdd1 to Linux RAID",
	"Create partition /dev/sde1 (4.07 GiB)",
	"Set id of partition /dev/sde1 to Linux RAID",
	"Create MD RAID6 /dev/md0 (8.00 GiB) from /dev/sdb1 (4.07 GiB), /dev/sdc1 (4.07 GiB), /dev/sdd1 (4.07 GiB) and /dev/sde1 (4.07 GiB)",
	"Add /dev/md0 to /etc/mdadm.conf"
    };

    Testsuite testsuite;
    testsuite.devicegraph_filename = "empty2.xml";

    // Here barrel does the expansion of /dev/sd[b-e] (in GetOpts).
    testsuite.readlines = {
	"create raid6 /dev/sd[b-e] --size=8g --metadata=1.2",
	"commit"
    };

    handle(args.argc(), args.argv(), &testsuite);

    BOOST_CHECK_EQUAL(actions, testsuite.actions); // TODO sort

    const Devicegraph* staging = testsuite.storage->get_staging();

    const Md* md0 = Md::find_by_name(staging, "/dev/md0");
    BOOST_CHECK_EQUAL(md0->get_metadata(), "1.2");
}


BOOST_AUTO_TEST_CASE(test4)
{
    Args args({ "--dry-run", "--yes", "create", "raid", "--level", "mirror", "--name", "test",
	    "/dev/sdb", "/dev/sdc", "--force" });

    vector<string> actions = {
	"Delete GPT on /dev/sdc",
	"Delete GPT on /dev/sdb",
	"Create MD RAID1 /dev/md/test (31.84 GiB) from /dev/sdb (32.00 GiB) and /dev/sdc (32.00 GiB)",
	"Add /dev/md/test to /etc/mdadm.conf"
    };

    Testsuite testsuite;
    testsuite.devicegraph_filename = "empty2.xml";

    handle(args.argc(), args.argv(), &testsuite);

    BOOST_CHECK_EQUAL(actions, testsuite.actions); // TODO sort
}
