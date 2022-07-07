
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE barrel

#include <boost/test/unit_test.hpp>

#include <storage/Actiongraph.h>
#include <storage/Devices/LvmVg.h>
#include <storage/Utils/HumanString.h>

#include "../barrel/handle.h"
#include "../barrel/Utils/Args.h"
#include "helpers/output.h"


using namespace std;
using namespace storage;
using namespace barrel;


BOOST_AUTO_TEST_CASE(test1)
{
    Args args({ "--dry-run", "--yes", "create", "vg", "--name", "test", "--size", "5g", "--pool",
	    "HDDs (512 B)", "--devices", "2", "lv", "--name", "a", "--size", "2g", "--stripes", "max", "xfs",
	    "--path", "/test" });

    vector<string> actions = {
	"Create partition /dev/sdb1 (2.50 GiB)",
	"Set id of partition /dev/sdb1 to Linux LVM",
	"Create physical volume on /dev/sdb1",
	"Create partition /dev/sdc1 (2.50 GiB)",
	"Set id of partition /dev/sdc1 to Linux LVM",
	"Create physical volume on /dev/sdc1",
	"Create volume group test (5.00 GiB) from /dev/sdb1 (2.50 GiB) and /dev/sdc1 (2.50 GiB)",
	"Create logical volume a (2.00 GiB) on volume group test",
	"Create xfs on /dev/test/a (2.00 GiB)",
	"Mount /dev/test/a (2.00 GiB) at /test",
	"Add mount point /test of /dev/test/a (2.00 GiB) to /etc/fstab"
    };

    Testsuite testsuite;
    testsuite.devicegraph_filename = "empty2.xml";

    handle(args.argc(), args.argv(), &testsuite);

    BOOST_CHECK_EQUAL(actions, testsuite.actions); // TODO sort
}


BOOST_AUTO_TEST_CASE(test2)
{
    Args args({ "--dry-run", "--yes" });

    vector<string> actions = {
	"Create partition /dev/sdb1 (2.50 GiB)",
	"Set id of partition /dev/sdb1 to Linux LVM",
	"Create physical volume on /dev/sdb1",
	"Create partition /dev/sdc1 (2.50 GiB)",
	"Set id of partition /dev/sdc1 to Linux LVM",
	"Create physical volume on /dev/sdc1",
	"Create volume group test (5.00 GiB) from /dev/sdb1 (2.50 GiB) and /dev/sdc1 (2.50 GiB)",
	"Create logical volume a (2.00 GiB) on volume group test",
	"Create xfs on /dev/test/a (2.00 GiB)",
	"Mount /dev/test/a (2.00 GiB) at /test",
	"Add mount point /test of /dev/test/a (2.00 GiB) to /etc/fstab"
    };

    Testsuite testsuite;
    testsuite.devicegraph_filename = "empty2.xml";

    testsuite.readlines = {
	"create vg --name test --size 5g --pool 'HDDs (512 B)' --devices 2",
	"create lv --vg-name=test --name=a --size=2g --stripes=2",
	"create xfs --path /test",
	"commit"
    };

    handle(args.argc(), args.argv(), &testsuite);

    BOOST_CHECK_EQUAL(actions, testsuite.actions); // TODO sort
}


BOOST_AUTO_TEST_CASE(test3)
{
    Args args({ "--dry-run", "--yes", "create", "vg", "--name", "test", "/dev/sdb", "/dev/sdc",
	    "--size=max", "--extent-size=8m", "lv", "--name", "a", "--size", "max", "ext4" });

    vector<string> actions = {
	"Create partition /dev/sdb1 (32.00 GiB)",
	"Set id of partition /dev/sdb1 to Linux LVM",
	"Create physical volume on /dev/sdb1",
	"Create partition /dev/sdc1 (32.00 GiB)",
	"Set id of partition /dev/sdc1 to Linux LVM",
	"Create physical volume on /dev/sdc1",
	"Create volume group test (63.98 GiB) from /dev/sdb1 (32.00 GiB) and /dev/sdc1 (32.00 GiB)",
	"Create logical volume a (63.98 GiB) on volume group test",
	"Create ext4 on /dev/test/a (63.98 GiB)"
    };

    Testsuite testsuite;
    testsuite.devicegraph_filename = "empty2.xml";

    handle(args.argc(), args.argv(), &testsuite);

    BOOST_CHECK_EQUAL(actions, testsuite.actions); // TODO sort

    const Devicegraph* staging = testsuite.storage->get_staging();

    const LvmVg* lvm_vg = LvmVg::find_by_vg_name(staging, "test");
    BOOST_CHECK_EQUAL(lvm_vg->get_extent_size(), 8 * MiB);
}


BOOST_AUTO_TEST_CASE(test4)
{
    Args args({ "--dry-run", "--yes", "create", "vg", "--name", "test", "/dev/sdb", "--force",
	    "lv", "--name", "a", "--size", "2g", "--stripes", "max", "xfs", "--path", "/test" });

    vector<string> actions = {
	"Delete GPT on /dev/sdb",
	"Create physical volume on /dev/sdb",
	"Create volume group test (32.00 GiB) from /dev/sdb (32.00 GiB)",
	"Create logical volume a (2.00 GiB) on volume group test",
	"Create xfs on /dev/test/a (2.00 GiB)",
	"Mount /dev/test/a (2.00 GiB) at /test",
	"Add mount point /test of /dev/test/a (2.00 GiB) to /etc/fstab"
    };

    Testsuite testsuite;
    testsuite.devicegraph_filename = "empty2.xml";

    handle(args.argc(), args.argv(), &testsuite);

    BOOST_CHECK_EQUAL(actions, testsuite.actions); // TODO sort
}
