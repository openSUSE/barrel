
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
    Args args({ "barrel", "--dry-run", "--yes", "create", "gpt", "/dev/sdb", "--force" });

    vector<string> actions = {
	"Delete GPT on /dev/sdb",
	"Create GPT on /dev/sdb"
    };

    Testsuite testsuite;
    testsuite.devicegraph_filename = "empty2.xml";

    vector<string> tmp;
    testsuite.save_actiongraph = [&tmp](const Actiongraph* actiongraph) {
	tmp = actiongraph->get_commit_actions_as_strings();
    };

    handle(args.argc(), args.argv(), &testsuite);

    BOOST_CHECK_EQUAL(actions, tmp);
}
