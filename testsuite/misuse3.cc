
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
    Args args({ "--dry-run", "create", "xfs", "--size", "10g" });

    vector<string> output1 = {
	"Probing... done"
    };

    vector<string> output2 = {
	"error: stack empty"
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
    Args args({ "--dry-run", "create", "xfs", "--size", "10g", "/dev/sda", "xfs" });

    vector<string> output1 = {
	"Probing... done"
    };

    vector<string> output2 = {
	"error: not a block device on stack"
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
    Args args({ "--dry-run", "create", "encryption", "--name", "cr-test", "--size", "10g", "/dev/sda" });

    vector<string> output1 = {
	// No "Probing..."
    };

    vector<string> output2 = {
	"error: encryption type missing for command 'encryption'"
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
    Args args({ "--dry-run", "create", "luks1", "--name", "cr-test", "--size", "10g", "/dev/sda",
	    "--label", "foo" });

    vector<string> output1 = {
	// No "Probing..."
    };

    vector<string> output2 = {
	"error: label only supported for LUKS2"
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
