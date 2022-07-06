
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE barrel

#include <boost/test/unit_test.hpp>

#include <storage/Actiongraph.h>
#include <storage/Devices/Partition.h>

#include "../barrel/handle.h"
#include "../barrel/Utils/Args.h"
#include "helpers.h"


using namespace std;
using namespace storage;
using namespace barrel;


BOOST_AUTO_TEST_CASE(test1)
{
    Args args({ "--dry-run", "--yes", "create", "xfs", "/dev/sdb", "-s", "8 GiB", "-p", "/test",
	    "-o", "noauto" });

    vector<string> actions = {
	"Create partition /dev/sdb1 (8.00 GiB)",
	"Create xfs on /dev/sdb1 (8.00 GiB)",
	"Mount /dev/sdb1 (8.00 GiB) at /test",
	"Add mount point /test of /dev/sdb1 (8.00 GiB) to /etc/fstab"
    };

    Testsuite testsuite;
    testsuite.devicegraph_filename = "empty2.xml";

    handle(args.argc(), args.argv(), &testsuite);

    BOOST_CHECK_EQUAL(actions, testsuite.actions);
}


BOOST_AUTO_TEST_CASE(test2)
{
    Args args({ "--dry-run", "--yes", "create", "filesystem", "--type=xfs", "/dev/sdb", "--force" });

    vector<string> actions = {
	"Delete GPT on /dev/sdb",
	"Create xfs on /dev/sdb (32.00 GiB)"
    };

    Testsuite testsuite;
    testsuite.devicegraph_filename = "empty2.xml";

    handle(args.argc(), args.argv(), &testsuite);

    BOOST_CHECK_EQUAL(actions, testsuite.actions);
}


BOOST_AUTO_TEST_CASE(test3)
{
    Args args({ "--dry-run", "--yes", "create", "xfs", "--pool", "HDDs (512 B)", "--size=12 GiB",
	    "--mkfs-options=-m bigtime=1" });

    vector<string> actions = {
	"Create partition /dev/sdb1 (12.00 GiB)",
	"Create xfs on /dev/sdb1 (12.00 GiB)"
    };

    Testsuite testsuite;
    testsuite.devicegraph_filename = "empty2.xml";

    handle(args.argc(), args.argv(), &testsuite);

    BOOST_CHECK_EQUAL(actions, testsuite.actions);

    const Devicegraph* staging = testsuite.storage->get_staging();

    const Partition* sdb1 = Partition::find_by_name(staging, "/dev/sdb1");
    BOOST_CHECK_EQUAL(sdb1->get_id(), ID_LINUX);
}
