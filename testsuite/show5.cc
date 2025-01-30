
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
    setlocale(LC_ALL, "C.UTF-8");

    Args args({ "--dry-run", "show", "filesystems" });

    vector<string> output = {
	"Probing... done",
	"Type  │ Label │ Name         │       Size │  Used │ Profiles    │ Mount Point",
	"──────┼───────┼──────────────┼────────────┼───────┼─────────────┼────────────",
	"btrfs │       │ /dev/sda2    │  29.99 GiB │       │ single, dup │ /",
	"nfs   │       │ rpi5:/backup │ 915.81 GiB │ 6.26% │             │ /backup",
	"tmpfs │       │              │ 128.00 MiB │ 0.00% │             │ /playground",
	"swap  │       │ /dev/sda3    │   2.00 GiB │       │             │ swap"
    };

    Testsuite testsuite;
    testsuite.devicegraph_filename = "real8.xml";

    ostringstream buffer;
    streambuf* old = cout.rdbuf(buffer.rdbuf());
    handle(args.argc(), args.argv(), &testsuite);
    cout.rdbuf(old);

    string lhs = buffer.str();
    string rhs = accumulate(output.begin(), output.end(), ""s,
			    [](auto a, auto b) { return a + b + "\n"; });

    BOOST_CHECK_EQUAL(lhs, rhs);
}
