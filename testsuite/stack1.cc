
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE barrel

#include <numeric>
#include <boost/test/unit_test.hpp>

#include "../barrel/handle.h"
#include "../barrel/Utils/Args.h"
#include "helpers/output.h"


using namespace std;
using namespace storage;
using namespace barrel;


BOOST_AUTO_TEST_CASE(test1)
{
    Args args({ "--dry-run", "--yes" });

    vector<string> output = {
	"Probing... done",
	"create vg --name test --size 4g /dev/sdb",
	"  Create partition /dev/sdb1 (4.00 GiB)",
	"  Set id of partition /dev/sdb1 to Linux LVM",
	"  Create physical volume on /dev/sdb1",
	"  Create volume group test (4.00 GiB) from /dev/sdb1 (4.00 GiB)",
	"dup",
	"create lv --name a --size 1g xfs",
	"  Create logical volume a (1.00 GiB) on volume group test",
	"  Create xfs on /dev/test/a (1.00 GiB)",
	"stack",
	"top  filesystem xfs on /dev/test/a",
	"     LVM volume group test",
	"quit"
    };

    Testsuite testsuite;
    testsuite.devicegraph_filename = "empty2.xml";

    testsuite.readlines = {
	"create vg --name test --size 4g /dev/sdb",
	"dup",
	"create lv --name a --size 1g xfs",
	"stack",
	"quit"
    };

    ostringstream buffer;
    streambuf* old = cout.rdbuf(buffer.rdbuf());
    handle(args.argc(), args.argv(), &testsuite);
    cout.rdbuf(old);

    string lhs = buffer.str();
    string rhs = accumulate(output.begin(), output.end(), ""s,
			    [](auto a, auto b) { return a + b + "\n"; });

    BOOST_CHECK_EQUAL(lhs, rhs);
}
