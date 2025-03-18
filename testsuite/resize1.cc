
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
    Args args({ "--dry-run", "--yes" });

    vector<string> actions = {
	"Grow partition /dev/sdc1 from 100.00 GiB to 200.00 GiB",
	"Grow ext4 on /dev/sdc1 from 100.00 GiB to 200.00 GiB"
    };

    Testsuite testsuite;
    testsuite.devicegraph_filename = "real9.xml";

    testsuite.readlines = {
	"resize device /dev/sdc1 --size '+100 GiB'",
	"commit"
    };

    handle(args.argc(), args.argv(), &testsuite);

    BOOST_CHECK_EQUAL(actions, testsuite.actions); // TODO sort
}


BOOST_AUTO_TEST_CASE(test2)
{
    Args args({ "--dry-run", "--yes" });

    vector<string> actions = {
	"Grow partition /dev/sdc1 from 100.00 GiB to 298.09 GiB",
	"Grow ext4 on /dev/sdc1 from 100.00 GiB to 298.09 GiB"
    };

    Testsuite testsuite;
    testsuite.devicegraph_filename = "real9.xml";

    testsuite.readlines = {
	"resize device /dev/sdc1 --size max",
	"commit"
    };

    handle(args.argc(), args.argv(), &testsuite);

    BOOST_CHECK_EQUAL(actions, testsuite.actions); // TODO sort
}


/*
 * The partition sdd1 has already the "absolute" max size (the end is not aligned).
 * Check that barrel does not align the end and thus shrinks the partition.
 */
BOOST_AUTO_TEST_CASE(test3)
{
    Args args({ "--dry-run", "--yes" });

    vector<string> actions = {
    };

    Testsuite testsuite;
    testsuite.devicegraph_filename = "real9.xml";

    testsuite.readlines = {
	"resize device /dev/sdd1 --size max",
	"commit"
    };

    handle(args.argc(), args.argv(), &testsuite);

    BOOST_CHECK_EQUAL(actions, testsuite.actions);
}


BOOST_AUTO_TEST_CASE(test4)
{
    Args args({ "--dry-run", "--yes" });

    vector<string> actions = {
    };

    Testsuite testsuite;
    testsuite.devicegraph_filename = "real9.xml";

    testsuite.readlines = {
	"resize device /dev/sdc1 --size '100 GiB'",
	"commit"
    };

    handle(args.argc(), args.argv(), &testsuite);

    BOOST_CHECK_EQUAL(actions, testsuite.actions);
}


BOOST_AUTO_TEST_CASE(test5)
{
    Args args({ "--dry-run", "--yes" });

    vector<string> output1 = {
        "Probing... done",
	"resize device /dev/sdc1 --size '10 TiB'",
	"commit",
	"dry run"
    };

    vector<string> output2 = {
        "error: New size too big."
    };

    Testsuite testsuite;
    testsuite.devicegraph_filename = "real9.xml";

    testsuite.readlines = {
	"resize device /dev/sdc1 --size '10 TiB'",
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
