
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
    Args args({ "--dry-run", "--yes" });

    vector<string> output = {
	"Probing... done",
	"show pools",
	"Name             │ Devices │       Size │  Used",
	"─────────────────┼─────────┼────────────┼──────",
	"NVMes (512 B)    │       2 │ 476.94 GiB │ 0.00%",
	"├─/dev/nvme0n1   │         │ 476.94 GiB │ 0.00%",
	"└─/dev/nvme1n1 ! │         │            │",
	"quit"
    };

    Testsuite testsuite;
    testsuite.devicegraph_filename = "empty3.xml";

    testsuite.readlines = {
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
