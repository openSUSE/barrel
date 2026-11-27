#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE barrel

#include <boost/test/unit_test.hpp>

#include "../barrel/handle.h"
#include "../barrel/Utils/Args.h"
#include "helpers/run-and-capture.h"


using namespace std;
using namespace barrel;



static string
error_of(const vector<string>& cmd)
{
    Args args(cmd);

    Testsuite testsuite;
    testsuite.devicegraph_filename = "msdos1.xml";

    return run_and_capture(args.argc(), args.argv(), &testsuite).second;
}


/*
 * Run cmd followed by devices, once with and once without --force, and
 * check that both runs fail with sane error message, because child depends
 * on parent.
 */

static void
check_dependent(const vector<string>& cmd, const vector<string>& devices,
		const string& child, const string& parent)
{
    const string expected = "error: block device '" + child + "' depends on block device '" +
	parent + "'\n";

    for (bool force : { true, false })
    {
	BOOST_TEST_CONTEXT("force: " << force)
	{
	    vector<string> args = { "--dry-run", "--yes" };

	    args.insert(args.end(), cmd.begin(), cmd.end());
	    if (force)
		args.push_back("--force");
	    args.insert(args.end(), devices.begin(), devices.end());

	    BOOST_CHECK_EQUAL(error_of(args), expected);
	}
    }
}


BOOST_AUTO_TEST_CASE(raid_disk_before_partition)
{
    check_dependent({ "create", "raid1" }, { "/dev/sdd", "/dev/sdd1" }, "/dev/sdd1", "/dev/sdd");
}


BOOST_AUTO_TEST_CASE(raid_partition_before_disk)
{
    check_dependent({ "create", "raid1" }, { "/dev/sdd1", "/dev/sdd" }, "/dev/sdd1", "/dev/sdd");
}


BOOST_AUTO_TEST_CASE(lvm_vg)
{
    check_dependent({ "create", "vg", "--name", "test" }, { "/dev/sdd", "/dev/sdd5" },
		    "/dev/sdd5", "/dev/sdd");
}


BOOST_AUTO_TEST_CASE(btrfs)
{
    check_dependent({ "create", "btrfs" }, { "/dev/sdd", "/dev/sdd6" }, "/dev/sdd6", "/dev/sdd");
}
