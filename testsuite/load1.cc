
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
    // So far only /dev/sda and alike can be used in the mapping file. E.g. by-path links
    // need a real system lookup (in find_by_any_name()).

    Args args({ "barrel", "--dry-run", "--yes", "--verbose", "load", "devicegraph", "--name", "load1.xml",
	    "--mapping", "mapping1.json" });

    vector<string> actions = {
	"Delete GPT on /dev/sda",
	"Create GPT on /dev/sda",
	"Set protective MBR boot flag of GPT on /dev/sda",
	"Create partition /dev/sda1 (8.00 MiB)",
	"Create partition /dev/sda2 (29.99 GiB)",
	"Create partition /dev/sda3 (2.00 GiB)",
	"Create swap on /dev/sda3 (2.00 GiB)",
	"Set UUID of swap on /dev/sda3 (2.00 GiB) to 8b753e3c-4d8e-428d-9c0f-481387f76185",
	"Mount /dev/sda3 (2.00 GiB) at swap",
	"Set legacy boot flag of partition /dev/sda2",
	"Create ext4 on /dev/sda2 (29.99 GiB)",
	"Set UUID of ext4 on /dev/sda2 (29.99 GiB) to 0bdcf9d9-fb22-4b06-bbbd-b4bc0307748a",
	"Mount /dev/sda2 (29.99 GiB) at /",
	"Add mount point swap of /dev/sda3 (2.00 GiB) to /etc/fstab",
	"Add mount point / of /dev/sda2 (29.99 GiB) to /etc/fstab",
	"Set id of partition /dev/sda1 to BIOS Boot Partition"
    };

    Testsuite testsuite;
    testsuite.devicegraph_filename = "empty1.xml";

    vector<string> tmp;
    testsuite.save_actiongraph = [&tmp](const Actiongraph* actiongraph) {
	tmp = actiongraph->get_commit_actions_as_strings();
    };

    handle(args.argc(), args.argv(), &testsuite);

    BOOST_CHECK_EQUAL(actions, tmp); // TODO sort
}


BOOST_AUTO_TEST_CASE(test2)
{
    Args args({ "barrel", "--dry-run", "--yes", "--verbose", "load", "devicegraph", "--name", "load1.xml" });

    vector<string> actions = {
	"Delete GPT on /dev/sda",
	"Create GPT on /dev/sda",
	"Set protective MBR boot flag of GPT on /dev/sda",
	"Create partition /dev/sda1 (8.00 MiB)",
	"Create partition /dev/sda2 (29.99 GiB)",
	"Create partition /dev/sda3 (2.00 GiB)",
	"Create swap on /dev/sda3 (2.00 GiB)",
	"Set UUID of swap on /dev/sda3 (2.00 GiB) to 8b753e3c-4d8e-428d-9c0f-481387f76185",
	"Mount /dev/sda3 (2.00 GiB) at swap",
	"Set legacy boot flag of partition /dev/sda2",
	"Create ext4 on /dev/sda2 (29.99 GiB)",
	"Set UUID of ext4 on /dev/sda2 (29.99 GiB) to 0bdcf9d9-fb22-4b06-bbbd-b4bc0307748a",
	"Mount /dev/sda2 (29.99 GiB) at /",
	"Add mount point swap of /dev/sda3 (2.00 GiB) to /etc/fstab",
	"Add mount point / of /dev/sda2 (29.99 GiB) to /etc/fstab",
	"Set id of partition /dev/sda1 to BIOS Boot Partition"
    };

    Testsuite testsuite;
    testsuite.devicegraph_filename = "empty1.xml";

    vector<string> tmp;
    testsuite.save_actiongraph = [&tmp](const Actiongraph* actiongraph) {
	tmp = actiongraph->get_commit_actions_as_strings();
    };

    handle(args.argc(), args.argv(), &testsuite);

    BOOST_CHECK_EQUAL(actions, tmp); // TODO sort
}
