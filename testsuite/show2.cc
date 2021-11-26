
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE barrel

#include <numeric>
#include <boost/test/unit_test.hpp>

#include <storage/Actiongraph.h>

#include "../barrel/handle.h"
#include "../barrel/Utils/Args.h"


using namespace std;
using namespace storage;
using namespace barrel;


namespace std
{
    ostream& operator<<(ostream& s, const vector<string>& lines)
    {
	for (const string& line : lines)
	    s << line << '\n';

	return s;
    }
}


BOOST_AUTO_TEST_CASE(test1)
{
    Args args({ "--dry-run", "show", "tree", "/dev/md0" });

    vector<string> output = {
	"Probing... done",
	"Name         │      Size │ Usage         │ Pool",
	"─────────────┼───────────┼───────────────┼─────────────",
	"/dev/md0     │ 32.00 GiB │ LVM data      │",
	"├─/dev/sdb1  │ 32.00 GiB │ RAID /dev/md0 │",
	"│ └─/dev/sdb │ 32.00 GiB │ GPT           │ HDDs (512 B)",
	"└─/dev/sdc1  │ 32.00 GiB │ RAID /dev/md0 │",
	"  └─/dev/sdc │ 32.00 GiB │ GPT           │ HDDs (512 B)"
    };

    Testsuite testsuite;
    testsuite.devicegraph_filename = "real1.xml";

    ostringstream buffer;
    streambuf* old = cout.rdbuf(buffer.rdbuf());
    handle(args.argc(), args.argv(), &testsuite);
    cout.rdbuf(old);

    string lhs = buffer.str();
    string rhs = accumulate(output.begin(), output.end(), ""s,
			    [](auto a, auto b) { return a + b + "\n"; });

    BOOST_CHECK_EQUAL(lhs, rhs);
}


BOOST_AUTO_TEST_CASE(test2)
{
    Args args({ "--quiet", "--dry-run", "show", "tree", "/dev/data/home" });

    vector<string> output = {
	"Name           │      Size │ Usage         │ Pool",
	"───────────────┼───────────┼───────────────┼─────────────",
	"/dev/data/home │ 20.00 GiB │ xfs           │",
	"└─/dev/md0     │ 32.00 GiB │ LVM data      │",
	"  ├─/dev/sdb1  │ 32.00 GiB │ RAID /dev/md0 │",
	"  │ └─/dev/sdb │ 32.00 GiB │ GPT           │ HDDs (512 B)",
	"  └─/dev/sdc1  │ 32.00 GiB │ RAID /dev/md0 │",
	"    └─/dev/sdc │ 32.00 GiB │ GPT           │ HDDs (512 B)"
    };

    Testsuite testsuite;
    testsuite.devicegraph_filename = "real1.xml";

    ostringstream buffer;
    streambuf* old = cout.rdbuf(buffer.rdbuf());
    handle(args.argc(), args.argv(), &testsuite);
    cout.rdbuf(old);

    string lhs = buffer.str();
    string rhs = accumulate(output.begin(), output.end(), ""s,
			    [](auto a, auto b) { return a + b + "\n"; });

    BOOST_CHECK_EQUAL(lhs, rhs);
}


BOOST_AUTO_TEST_CASE(test3)
{
    Args args({ "--dry-run", "--quiet", "show", "tree", "/dev/mapper/36005076305ffc73a00000000000013b4" });

    vector<string> output = {
	"Name                                          │     Size │ Usage                                            │ Pool",
	"──────────────────────────────────────────────┼──────────┼──────────────────────────────────────────────────┼────────────",
	"/dev/mapper/36005076305ffc73a00000000000013b4 │ 1.00 GiB │ GPT                                              │ MPs (512 B)",
	"├─/dev/sda                                    │ 1.00 GiB │ MP /dev/mapper/36005076305ffc73a00000000000013b4 │",
	"└─/dev/sdc                                    │ 1.00 GiB │ MP /dev/mapper/36005076305ffc73a00000000000013b4 │",
    };

    Testsuite testsuite;
    testsuite.devicegraph_filename = "real3.xml";

    ostringstream buffer;
    streambuf* old = cout.rdbuf(buffer.rdbuf());
    handle(args.argc(), args.argv(), &testsuite);
    cout.rdbuf(old);

    string lhs = buffer.str();
    string rhs = accumulate(output.begin(), output.end(), ""s,
			    [](auto a, auto b) { return a + b + "\n"; });

    BOOST_CHECK_EQUAL(lhs, rhs);
}


BOOST_AUTO_TEST_CASE(test4)
{
    Args args({ "--dry-run", "--quiet", "show", "tree", "/dev/mapper/isw_ddgdcbibhd_test1" });

    vector<string> output = {
	"Name                             │      Size │ Usage │ Pool",
	"─────────────────────────────────┼───────────┼───────┼─────",
	"/dev/mapper/isw_ddgdcbibhd_test1 │  8.00 GiB │ GPT   │",
	"├─/dev/sdb                       │ 16.00 GiB │       │",
	"└─/dev/sdc                       │ 16.00 GiB │       │",
    };

    Testsuite testsuite;
    testsuite.devicegraph_filename = "real4.xml";

    ostringstream buffer;
    streambuf* old = cout.rdbuf(buffer.rdbuf());
    handle(args.argc(), args.argv(), &testsuite);
    cout.rdbuf(old);

    string lhs = buffer.str();
    string rhs = accumulate(output.begin(), output.end(), ""s,
			    [](auto a, auto b) { return a + b + "\n"; });

    BOOST_CHECK_EQUAL(lhs, rhs);
}
