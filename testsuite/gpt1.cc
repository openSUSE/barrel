
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE barrel

#include <boost/test/unit_test.hpp>

#include <storage/Actiongraph.h>

#include "../barrel/handle.h"
#include "../barrel/Utils/Args.h"
#include "helpers/output.h"


using namespace std;
using namespace storage;
using namespace barrel;


BOOST_AUTO_TEST_CASE(test1)
{
    Args args({ "--dry-run", "--yes", "create", "gpt", "/dev/sdb", "--force" });

    vector<string> actions = {
	"Delete GPT on /dev/sdb",
	"Create GPT on /dev/sdb"
    };

    Testsuite testsuite;
    testsuite.devicegraph_filename = "empty2.xml";

    handle(args.argc(), args.argv(), &testsuite);

    BOOST_CHECK_EQUAL(actions, testsuite.actions);
}
