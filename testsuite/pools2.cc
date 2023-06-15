
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
	"create gpt /dev/nvme1n1",
	"  Create GPT on /dev/nvme1n1",
	"create raid1 --size 100g --pool-name \"NVMes (512 B)\"",
	"  Create partition /dev/nvme0n1p1 (100.22 GiB)",
	"  Set id of partition /dev/nvme0n1p1 to Linux RAID",
	"  Create partition /dev/nvme1n1p1 (100.22 GiB)",
	"  Set id of partition /dev/nvme1n1p1 to Linux RAID",
	"  Create MD RAID1 /dev/md0 (100.00 GiB) from /dev/nvme0n1p1 (100.22 GiB) and /dev/nvme1n1p1 (100.22 GiB)",
	"  Add /dev/md0 to /etc/mdadm.conf",
	"create pool --name MDs /dev/md0",
	"show raids",
	"Name     │       Size │ Level │ Metadata │ Chunk Size │ Devices │ Usage │ Pool",
	"─────────┼────────────┼───────┼──────────┼────────────┼─────────┼───────┼─────",
	"/dev/md0 │ 100.00 GiB │ RAID1 │ default  │            │ 2       │       │ MDs",
	"show pools",
	"Name           │ Devices │       Size │   Used",
	"───────────────┼─────────┼────────────┼───────",
	"MDs            │       1 │        0 B │",
	"└─/dev/md0 !   │         │            │",
	"NVMes (512 B)  │       2 │ 953.88 GiB │ 21.01%",
	"├─/dev/nvme0n1 │         │ 476.94 GiB │ 21.01%",
	"└─/dev/nvme1n1 │         │ 476.94 GiB │ 21.01%",
	"quit"
    };

    Testsuite testsuite;
    testsuite.devicegraph_filename = "empty3.xml";

    testsuite.readlines = {
	"create gpt /dev/nvme1n1",
	"create raid1 --size 100g --pool-name \"NVMes (512 B)\"",
	"create pool --name MDs /dev/md0",
	"show raids",
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
