
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
    // Check that no RAID is created in case of the specific error. Tests the StagingGuard
    // in handle_interactive() (at least right now since surely the problem could also be
    // handled somewhere else).

    Args args({ "--dry-run", "--yes" });

    vector<string> output = {
	"Probing... done",
	"create raid6 --pool-name \"HDDs (512 B)\" --size max",
	"show raids",
	"Name │ Size │ Level │ Metadata │ Chunk Size │ Devices │ Usage │ Pool",
	"─────┼──────┼───────┼──────────┼────────────┼─────────┼───────┼─────",
	"quit"
    };

    Testsuite testsuite;
    testsuite.devicegraph_filename = "empty1.xml";

    testsuite.readlines = {
	"create raid6 --pool-name \"HDDs (512 B)\" --size max",
	"show raids",
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


BOOST_AUTO_TEST_CASE(test2)
{
    // Check that the RAID is still on the stack after the exception of the second 'create
    // vg' command. Tests the StackGuard in handle_interactive() (at least right now since
    // surely the problem could also be handled somewhere else).

    Args args({ "--dry-run", "--yes" });

    Testsuite testsuite;
    testsuite.devicegraph_filename = "empty2.xml";

    testsuite.readlines = {
	"create raid1 --pool-name \"HDDs (512 B)\" --size max --devices 2",
	"dup",
	"create vg --name test1",
	"pop",
	"create vg --name test2"
    };

    handle(args.argc(), args.argv(), &testsuite);

    const Devicegraph* staging = testsuite.storage->get_staging();
    const Stack* stack = testsuite.stack.get();

    BOOST_CHECK(stack->size() == 1);
    BOOST_CHECK(stack->top()->print(staging) == "RAID /dev/md0");
}
