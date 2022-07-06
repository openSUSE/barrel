
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE barrel

#include <boost/test/unit_test.hpp>

#include <storage/Actiongraph.h>

#include "../barrel/handle.h"
#include "../barrel/Utils/Args.h"
#include "../barrel/Utils/Mockup.h"


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


class Fixture
{
public:

    Fixture()
    {
	mockup = true;
    }

};


BOOST_TEST_GLOBAL_FIXTURE(Fixture);


BOOST_AUTO_TEST_CASE(test1)
{
    Args args({ "--dry-run", "--yes", "create", "luks1", "--name=cr1", "/dev/sdb", "-s", "max",
	    "ext4" });

    vector<string> actions = {
	"Create partition /dev/sdb1 (32.00 GiB)",
	"Create encryption layer device on /dev/sdb1",
	"Activate encryption layer device on /dev/sdb1",
	"Create ext4 on /dev/mapper/cr1 (32.00 GiB)",
	"Add encryption layer device on /dev/sdb1 to /etc/crypttab"
    };

    Testsuite testsuite;
    testsuite.devicegraph_filename = "empty2.xml";

    handle(args.argc(), args.argv(), &testsuite);

    BOOST_CHECK_EQUAL(actions, testsuite.actions);
}


BOOST_AUTO_TEST_CASE(test2)
{
    Args args({ "--dry-run", "--yes", "create", "encryption", "--type=luks2", "--name=cr2",
	    "/dev/sdb", "--force", "ext4" });

    vector<string> actions = {
	"Delete GPT on /dev/sdb",
	"Create encryption layer device on /dev/sdb",
	"Activate encryption layer device on /dev/sdb",
	"Create ext4 on /dev/mapper/cr2 (31.98 GiB)",
	"Add encryption layer device on /dev/sdb to /etc/crypttab"
    };

    Testsuite testsuite;
    testsuite.devicegraph_filename = "empty2.xml";

    handle(args.argc(), args.argv(), &testsuite);

    BOOST_CHECK_EQUAL(actions, testsuite.actions);
}


BOOST_AUTO_TEST_CASE(test3)
{
    Args args({ "--dry-run", "--yes", "create", "luks1", "--pool", "HDDs (512 B)", "--name",
	    "cr3", "--size=12 GiB", "ext4" });

    vector<string> actions = {
	"Create partition /dev/sdb1 (12.00 GiB)",
	"Create encryption layer device on /dev/sdb1",
	"Activate encryption layer device on /dev/sdb1",
	"Create ext4 on /dev/mapper/cr3 (12.00 GiB)",
	"Add encryption layer device on /dev/sdb1 to /etc/crypttab"
    };

    Testsuite testsuite;
    testsuite.devicegraph_filename = "empty2.xml";

    handle(args.argc(), args.argv(), &testsuite);

    BOOST_CHECK_EQUAL(actions, testsuite.actions);
}
