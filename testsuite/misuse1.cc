
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE barrel

#include <numeric>
#include <boost/test/unit_test.hpp>

#include <storage/Actiongraph.h>

#include "../barrel/handle.h"
#include "../barrel/Utils/Args.h"
#include "helpers/output.h"
#include "helpers/run-and-capture.h"


using namespace std;
using namespace storage;
using namespace barrel;


BOOST_AUTO_TEST_CASE(test1)
{
    // The block devices are missing. That is detected before probing.

    Args args({ "--dry-run", "create", "raid5" });

    vector<string> output1 = {
	// No "Probing..."
    };

    vector<string> output2 = {
	"error: block devices missing for command 'raid'"
    };

    Testsuite testsuite;
    testsuite.devicegraph_filename = "empty1.xml";

    pair<string, string> lhs = run_and_capture(args.argc(), args.argv(), &testsuite);

    string rhs1 = accumulate(output1.begin(), output1.end(), ""s,
			     [](auto a, auto b) { return a + b + "\n"; });

    BOOST_CHECK_EQUAL(lhs.first, rhs1);

    string rhs2 = accumulate(output2.begin(), output2.end(), ""s,
			     [](auto a, auto b) { return a + b + "\n"; });

    BOOST_CHECK_EQUAL(lhs.second, rhs2);
}


BOOST_AUTO_TEST_CASE(test2)
{
    // The --size option is missing. That is detected before probing.

    Args args({ "--dry-run", "create", "vg", "--name", "test", "--pool", "HDDs (512 B)" });

    vector<string> output1 = {
	// No "Probing..."
    };

    vector<string> output2 = {
	"error: size argument required for command 'vg'"
    };

    Testsuite testsuite;
    testsuite.devicegraph_filename = "empty1.xml";

    pair<string, string> lhs = run_and_capture(args.argc(), args.argv(), &testsuite);

    string rhs1 = accumulate(output1.begin(), output1.end(), ""s,
			     [](auto a, auto b) { return a + b + "\n"; });

    BOOST_CHECK_EQUAL(lhs.first, rhs1);

    string rhs2 = accumulate(output2.begin(), output2.end(), ""s,
			     [](auto a, auto b) { return a + b + "\n"; });

    BOOST_CHECK_EQUAL(lhs.second, rhs2);
}


BOOST_AUTO_TEST_CASE(test3)
{
    // The --devices option is not allowed for xfs. That is detected before probing.

    Args args({ "--dry-run", "create", "xfs", "--size", "10g", "--pool-name", "HDDs (512 B)",
	    "--devices", "2" });

    vector<string> output1 = {
	// No "Probing..."
    };

    vector<string> output2 = {
	"error: option --devices not allowed for xfs"
    };

    Testsuite testsuite;
    testsuite.devicegraph_filename = "empty1.xml";

    pair<string, string> lhs = run_and_capture(args.argc(), args.argv(), &testsuite);

    string rhs1 = accumulate(output1.begin(), output1.end(), ""s,
			     [](auto a, auto b) { return a + b + "\n"; });

    BOOST_CHECK_EQUAL(lhs.first, rhs1);

    string rhs2 = accumulate(output2.begin(), output2.end(), ""s,
			     [](auto a, auto b) { return a + b + "\n"; });

    BOOST_CHECK_EQUAL(lhs.second, rhs2);
}


BOOST_AUTO_TEST_CASE(test4)
{
    // Instructing barrel to remove both the extended and the logical partitions results
    // in an error since removing the extended partition already removed the logical
    // partitions. Thus the later lookup for the logical partitions fails.

    Args args({ "--dry-run", "--yes" });

    vector<string> output1 = {
	"Probing... done",
	"remove device /dev/sdd[1-7]",
	"commit",
	"dry run"
    };

    vector<string> output2 = {
	"error: device not found, name:/dev/sdd5"
    };

    Testsuite testsuite;
    testsuite.devicegraph_filename = "msdos1.xml";

    testsuite.readlines = {
        "remove device /dev/sdd[1-7]",
        "commit"
    };

    pair<string, string> lhs = run_and_capture(args.argc(), args.argv(), &testsuite);

    string rhs1 = accumulate(output1.begin(), output1.end(), ""s,
			     [](auto a, auto b) { return a + b + "\n"; });

    BOOST_CHECK_EQUAL(lhs.first, rhs1);

    string rhs2 = accumulate(output2.begin(), output2.end(), ""s,
			     [](auto a, auto b) { return a + b + "\n"; });

    BOOST_CHECK_EQUAL(lhs.second, rhs2);
}
