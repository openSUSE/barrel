
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE barrel

#include <numeric>
#include <boost/test/unit_test.hpp>

#include <storage/Actiongraph.h>
#include <storage/Version.h>

#include "../barrel/handle.h"
#include "../barrel/Utils/Args.h"
#include "helpers/output.h"
#include "helpers/run-and-capture.h"


using namespace std;
using namespace storage;
using namespace barrel;


BOOST_AUTO_TEST_CASE(test1)
{
    // Early check for encryption name before probing.

#if LIBSTORAGE_NG_VERSION_AT_LEAST(1, 103)

    Args args({ "--dry-run", "--yes", "create", "luks1", "--name", "a/b" });

    Testsuite testsuite;
    testsuite.devicegraph_filename = "empty2.xml";

    ostringstream buffer;
    streambuf* old = cout.rdbuf(buffer.rdbuf());
    handle(args.argc(), args.argv(), &testsuite);
    cout.rdbuf(old);

    vector<string> output1 = {
	// No "Probing..."
    };

    vector<string> output2 = {
	"error: invalid name for command 'encryption'"
    };

    pair<string, string> lhs = run_and_capture(args.argc(), args.argv(), &testsuite);

    string rhs1 = accumulate(output1.begin(), output1.end(), ""s,
			     [](auto a, auto b) { return a + b + "\n"; });

    BOOST_CHECK_EQUAL(lhs.first, rhs1);

    string rhs2 = accumulate(output2.begin(), output2.end(), ""s,
			     [](auto a, auto b) { return a + b + "\n"; });

    BOOST_CHECK_EQUAL(lhs.second, rhs2);

#endif
}
