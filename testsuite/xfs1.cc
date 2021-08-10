
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
    Args args({ "barrel", "--dry-run", "--yes", "create", "xfs", "/dev/sdb", "-s", "8 GiB", "-p", "/test", "-o", "noauto" });

    vector<string> actions = {
	"Create partition /dev/sdb1 (8.00 GiB)",
	"Create xfs on /dev/sdb1 (8.00 GiB)",
	"Mount /dev/sdb1 (8.00 GiB) at /test",
	"Add mount point /test of /dev/sdb1 (8.00 GiB) to /etc/fstab"
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


BOOST_AUTO_TEST_CASE(test2)
{
    Args args({ "barrel", "--dry-run", "--yes", "create", "xfs", "--pool", "HDDs (512 B)", "--size", "12 GiB" });

    // TODO another disk could be selected

    vector<string> actions = {
	"Create partition /dev/sdd1 (12.00 GiB)",
	"Create xfs on /dev/sdd1 (12.00 GiB)"
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
