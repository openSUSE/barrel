
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

    Args args({ "--dry-run", "show", "encryptions" });

    vector<string> output = {
	"Probing... done",
	"Name                                           │      Size │ Type  │ Cipher          │ Key Size │ PBKDF  │ Label │ Usage",
	"───────────────────────────────────────────────┼───────────┼───────┼─────────────────┼──────────┼────────┼───────┼───────────",
	"cr_ata-VBOX_HARDDISK_VB098dbc19-95da593f-part2 │ 31.98 GiB │ luks2 │ aes-xts-plain64 │ 512 bits │ pbkdf2 │       │ LVM system"
    };

    Testsuite testsuite;
    testsuite.devicegraph_filename = "real7.xml";

    ostringstream buffer;
    streambuf* old = cout.rdbuf(buffer.rdbuf());
    handle(args.argc(), args.argv(), &testsuite);
    cout.rdbuf(old);

    string lhs = buffer.str();
    string rhs = accumulate(output.begin(), output.end(), ""s,
			    [](auto a, auto b) { return a + b + "\n"; });

    BOOST_CHECK_EQUAL(lhs, rhs);
}
