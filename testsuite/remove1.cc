
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE barrel

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
    Args args({ "--dry-run", "--yes" });

    vector<string> actions = {
	"Create partition /dev/sdb1 (8.13 GiB)",
	"Set id of partition /dev/sdb1 to Linux RAID",
	"Create partition /dev/sdc1 (8.13 GiB)",
	"Set id of partition /dev/sdc1 to Linux RAID"
    };

    Testsuite testsuite;
    testsuite.devicegraph_filename = "empty2.xml";

    testsuite.readlines = {
	"create raid0 /dev/sd[bc] --size=16g",
	"create raid1 /dev/sd[de] --size=16g --metadata=1.2",
	"remove device /dev/md0 --keep-partitions",
	"remove device /dev/md1",
	"commit"
    };

    handle(args.argc(), args.argv(), &testsuite);

    BOOST_CHECK_EQUAL(actions, testsuite.actions); // TODO sort
}


BOOST_AUTO_TEST_CASE(test2)
{
    // Even on GPT partitions can be renumber due to the inability of parted to create
    // partitions with a defined number.

    Args args({ "--dry-run", "--yes" });

    vector<string> actions = {
	"Create partition /dev/sdb1 (2.03 GiB)",
	"Set id of partition /dev/sdb1 to Linux RAID",
	"Create partition /dev/sdc1 (2.03 GiB)",
	"Set id of partition /dev/sdc1 to Linux RAID",
	"Create MD RAID1 /dev/md1 (2.00 GiB) from /dev/sdb1 (2.03 GiB) and /dev/sdc1 (2.03 GiB)",
	"Add /dev/md1 to /etc/mdadm.conf"
    };

    Testsuite testsuite;
    testsuite.devicegraph_filename = "empty2.xml";

    testsuite.readlines = {
	"create raid0 /dev/sd[bc] --size=1g",
	"create raid1 /dev/sd[bc] --size=2g",
	"remove device /dev/md0",
	"commit"
    };

    handle(args.argc(), args.argv(), &testsuite);

    BOOST_CHECK_EQUAL(actions, testsuite.actions); // TODO sort
}
