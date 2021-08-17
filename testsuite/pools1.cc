
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE barrel

#include <numeric>
#include <boost/test/unit_test.hpp>

#include "../barrel/handle.h"
#include "../barrel/Utils/Args.h"


using namespace std;
using namespace storage;
using namespace barrel;


BOOST_AUTO_TEST_CASE(test1)
{
    Args args({ "barrel", "--dry-run", "--yes" });

    vector<string> output = {
	"Probing... done",
	"Name       │ Devices │      Size │  Used",
	"───────────┼─────────┼───────────┼──────",
	"HDDs       │       3 │ 96.00 GiB │ 0.00%",
	"├─/dev/sdb │         │ 32.00 GiB │ 0.00%",
	"├─/dev/sdc │         │ 32.00 GiB │ 0.00%",
	"└─/dev/sdd │         │ 32.00 GiB │ 0.00%",
	"Test       │       3 │ 96.00 GiB │ 0.00%",
	"├─/dev/sdb │         │ 32.00 GiB │ 0.00%",
	"├─/dev/sdc │         │ 32.00 GiB │ 0.00%",
	"└─/dev/sde │         │ 32.00 GiB │ 0.00%",

    };

    Testsuite testsuite;
    testsuite.devicegraph_filename = "empty2.xml";

    testsuite.readlines = {
	"remove pool --name \"SSDs (512 B)\"",
	"rename pool --old-name \"HDDs (512 B)\" --new-name HDDs",
	"reduce pool --name HDDs /dev/sde",
	"create pool --name Test /dev/sd[bc]",
	"extend pool --name Test /dev/sde",
	"show pools",
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
