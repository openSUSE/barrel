
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
    Args args({ "--dry-run", "--yes", "create", "raid", "--level", "5", "--pool", "HDDs (512 B)",
	    "--devices", "3+1", "--size", "8 GiB", "xfs", "--path", "/test1" });

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
	"Create xfs on /dev/md0 (8.00 GiB)",
	"Mount /dev/md0 (8.00 GiB) at /test1",
	"Add mount point /test1 of /dev/md0 (8.00 GiB) to /etc/fstab",
	"Add /dev/md0 to /etc/mdadm.conf"
    };

    Testsuite testsuite;
    testsuite.devicegraph_filename = "empty2.xml";

    handle(args.argc(), args.argv(), &testsuite);

    BOOST_CHECK_EQUAL(actions, testsuite.actions); // TODO sort
}


BOOST_AUTO_TEST_CASE(test2)
{
    Args args({ "--dry-run", "--yes", "create", "raid5", "--pool", "HDDs (512 B)", "--devices",
	    "3+1", "--size", "8 GiB", "gpt", "xfs", "--size", "1 GiB", "--path", "/test1" });

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
	"Create GPT on /dev/md0",
	"Create partition /dev/md0p1 (1.00 GiB)",
	"Create xfs on /dev/md0p1 (1.00 GiB)",
	"Mount /dev/md0p1 (1.00 GiB) at /test1",
	"Add mount point /test1 of /dev/md0p1 (1.00 GiB) to /etc/fstab",
	"Add /dev/md0 to /etc/mdadm.conf"
    };

    Testsuite testsuite;
    testsuite.devicegraph_filename = "empty2.xml";

    handle(args.argc(), args.argv(), &testsuite);

    BOOST_CHECK_EQUAL(actions, testsuite.actions); // TODO sort
}


BOOST_AUTO_TEST_CASE(test3)
{
    Args args({ "--dry-run", "--yes", "create", "raid", "--level", "5", "--pool", "HDDs (512 B)",
	    "--devices", "3+1", "--size", "8 GiB", "vg", "--name", "test", "lv", "--name", "foo",
	    "--size", "2 GiB" });

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
	"Create physical volume on /dev/md0",
	"Create volume group test (8.00 GiB) from /dev/md0 (8.00 GiB)",
	"Create logical volume foo (2.00 GiB) on volume group test",
	"Add /dev/md0 to /etc/mdadm.conf"
    };

    Testsuite testsuite;
    testsuite.devicegraph_filename = "empty2.xml";

    handle(args.argc(), args.argv(), &testsuite);

    BOOST_CHECK_EQUAL(actions, testsuite.actions); // TODO sort
}


BOOST_AUTO_TEST_CASE(test4)
{
    Args args({ "--dry-run", "--yes", "create", "raid5", "--pool", "HDDs (512 B)", "--devices",
	    "3+1", "--size", "8 GiB", "gpt", "vg", "--name", "test", "--size", "6 GiB", "lv", "--name",
	    "foo", "--size", "2 GiB" });

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
	"Create GPT on /dev/md0",
	"Create partition /dev/md0p1 (6.00 GiB)",
	"Set id of partition /dev/md0p1 to Linux LVM",
	"Create physical volume on /dev/md0p1",
	"Create volume group test (6.00 GiB) from /dev/md0p1 (6.00 GiB)",
	"Create logical volume foo (2.00 GiB) on volume group test",
	"Add /dev/md0 to /etc/mdadm.conf"
    };

    Testsuite testsuite;
    testsuite.devicegraph_filename = "empty2.xml";

    handle(args.argc(), args.argv(), &testsuite);

    BOOST_CHECK_EQUAL(actions, testsuite.actions); // TODO sort
}
