
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
    Args args({ "--dry-run", "show", "disks" });

    vector<string> output = {
	"Probing... done",
	"Name        │      Size │ Block Size │ Usage         │ Pool",
	"────────────┼───────────┼────────────┼───────────────┼─────────────",
	"/dev/sda    │ 32.00 GiB │      512 B │ GPT           │ HDDs (512 B)",
	"├─/dev/sda1 │  8.00 MiB │            │               │",
	"├─/dev/sda2 │ 29.99 GiB │            │ ext4          │",
	"└─/dev/sda3 │  2.00 GiB │            │ swap          │",
	"/dev/sdb    │ 32.00 GiB │      512 B │ GPT           │ HDDs (512 B)",
	"└─/dev/sdb1 │ 32.00 GiB │            │ RAID /dev/md0 │",
	"/dev/sdc    │ 32.00 GiB │      512 B │ GPT           │ HDDs (512 B)",
	"└─/dev/sdc1 │ 32.00 GiB │            │ RAID /dev/md0 │"
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
    Args args({ "--dry-run", "show", "raids" });

    vector<string> output = {
	"Probing... done",
	"Name     │      Size │ Level │ Metadata │ Chunk Size │ Devices │ Usage    │ Pool",
	"─────────┼───────────┼───────┼──────────┼────────────┼─────────┼──────────┼─────",
	"/dev/md0 │ 32.00 GiB │ RAID1 │ 1.0      │            │ 2       │ LVM data │"
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
    Args args({ "--quiet", "--dry-run", "show", "vgs" });

    vector<string> output = {
	"Name   │      Size │ Extent Size │ Devices │   Used │ Stripes │ Usage",
	"───────┼───────────┼─────────────┼─────────┼────────┼─────────┼──────",
	"data   │ 32.00 GiB │       4 MiB │ 1       │ 62.51% │         │",
	"└─home │ 20.00 GiB │             │         │        │ 1       │ xfs"
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


BOOST_AUTO_TEST_CASE(test4)
{
    Args args({ "--quiet", "--dry-run", "show", "vgs" });

    vector<string> output = {
	"Name        │       Size │ Extent Size │ Devices │  Used │ Stripes    │ Usage",
	"────────────┼────────────┼─────────────┼─────────┼───────┼────────────┼──────",
	"test        │ 596.17 GiB │       4 MiB │ 2       │ 5.03% │            │",
	"├─linear    │  10.00 GiB │             │         │       │ 2 (64 KiB) │",
	"└─thin-pool │  20.00 GiB │             │         │       │ 1          │",
	"  ├─thin1   │  10.00 GiB │             │         │       │            │ ext4",
	"  └─thin2   │  20.00 GiB │             │         │       │            │ xfs"
    };

    Testsuite testsuite;
    testsuite.devicegraph_filename = "real2.xml";

    ostringstream buffer;
    streambuf* old = cout.rdbuf(buffer.rdbuf());
    handle(args.argc(), args.argv(), &testsuite);
    cout.rdbuf(old);

    string lhs = buffer.str();
    string rhs = accumulate(output.begin(), output.end(), ""s,
			    [](auto a, auto b) { return a + b + "\n"; });

    BOOST_CHECK_EQUAL(lhs, rhs);
}


BOOST_AUTO_TEST_CASE(test5)
{
    Args args({ "--dry-run", "--quiet", "show", "filesystems" });

    vector<string> output = {
	"Type │ Label │ Name           │      Size │ Mount Point",
	"─────┼───────┼────────────────┼───────────┼────────────",
	"ext4 │       │ /dev/sda2      │ 29.99 GiB │ /",
	"xfs  │       │ /dev/data/home │ 20.00 GiB │ /home",
	"swap │       │ /dev/sda3      │  2.00 GiB │ swap"
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


BOOST_AUTO_TEST_CASE(test6)
{
    Args args({ "--dry-run", "--quiet", "show", "filesystems" });

    vector<string> output = {
	"Type  │ Label │ Name             │     Size │ Profile       │ Mount Point",
	"──────┼───────┼──────────────────┼──────────┼───────────────┼────────────",
	"btrfs │       │ multiple devices │          │ single, raid1 │",
	"      │       │ ├─/dev/sdc1      │ 2.98 GiB │               │",
	"      │       │ └─/dev/sdd1      │ 2.98 GiB │               │"
    };

    Testsuite testsuite;
    testsuite.devicegraph_filename = "real6.xml";

    ostringstream buffer;
    streambuf* old = cout.rdbuf(buffer.rdbuf());
    handle(args.argc(), args.argv(), &testsuite);
    cout.rdbuf(old);

    string lhs = buffer.str();
    string rhs = accumulate(output.begin(), output.end(), ""s,
			    [](auto a, auto b) { return a + b + "\n"; });

    BOOST_CHECK_EQUAL(lhs, rhs);
}


BOOST_AUTO_TEST_CASE(test7)
{
    Args args({ "--dry-run", "--quiet", "show", "dasds", "--no-partitions" });

    vector<string> output = {
	"Name       │     Size │ Block Size │   Bus ID │ Type │ Format │ Usage │ Pool",
	"───────────┼──────────┼────────────┼──────────┼──────┼────────┼───────┼───────────────────────",
	"/dev/dasdb │ 6.88 GiB │      4 KiB │ 0.0.0150 │ ECKD │ CDL    │ DASD  │ DASDs ECKD CDL (4 KiB)",
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
