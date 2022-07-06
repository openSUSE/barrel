
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE barrel

#include <boost/test/unit_test.hpp>

#include <storage/Actiongraph.h>
#include <storage/Devices/Partition.h>
#include <storage/Filesystems/Btrfs.h>

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

    ostream& operator<<(ostream& s, BtrfsRaidLevel btrfs_raid_level)
    {
	s << get_btrfs_raid_level_name(btrfs_raid_level);

	return s;
    }
}


BOOST_AUTO_TEST_CASE(test1)
{
    Args args({ "--dry-run", "--yes", "create", "btrfs", "/dev/sdb", "/dev/sdc", "-s", "8 GiB",
	    "-p", "/test1", "-o", "noauto" });

    vector<string> actions = {
	"Create partition /dev/sdb1 (4.00 GiB)",
	"Create partition /dev/sdc1 (4.00 GiB)",
	"Create btrfs on /dev/sdb1 (4.00 GiB) and /dev/sdc1 (4.00 GiB)",
	"Mount /dev/sdb1 (4.00 GiB) and /dev/sdc1 (4.00 GiB) at /test1",
	"Add mount point /test1 of /dev/sdb1 (4.00 GiB) and /dev/sdc1 (4.00 GiB) to /etc/fstab"
    };

    Testsuite testsuite;
    testsuite.devicegraph_filename = "empty2.xml";

    handle(args.argc(), args.argv(), &testsuite);

    BOOST_CHECK_EQUAL(actions, testsuite.actions);

    const Devicegraph* staging = testsuite.storage->get_staging();

    const Partition* sdb1 = Partition::find_by_name(staging, "/dev/sdb1");
    const Btrfs* btrfs = to_btrfs(sdb1->get_blk_filesystem());

    BOOST_CHECK_EQUAL(btrfs->get_data_raid_level(), BtrfsRaidLevel::DEFAULT);
    BOOST_CHECK_EQUAL(btrfs->get_metadata_raid_level(), BtrfsRaidLevel::DEFAULT);
}


BOOST_AUTO_TEST_CASE(test2)
{
    Args args({ "--dry-run", "--yes", "create", "btrfs", "--pool", "HDDs (512 B)", "--size=24 GiB",
	    "--devices", "max", "--profiles", "raid5,raid6" });

    vector<string> actions = {
	"Create partition /dev/sdb1 (6.00 GiB)",
	"Create partition /dev/sdc1 (6.00 GiB)",
	"Create partition /dev/sdd1 (6.00 GiB)",
	"Create partition /dev/sde1 (6.00 GiB)",
	"Create btrfs on /dev/sdb1 (6.00 GiB), /dev/sdc1 (6.00 GiB), /dev/sdd1 (6.00 GiB) and /dev/sde1 (6.00 GiB)"
    };

    Testsuite testsuite;
    testsuite.devicegraph_filename = "empty2.xml";

    handle(args.argc(), args.argv(), &testsuite);

    BOOST_CHECK_EQUAL(actions, testsuite.actions);

    const Devicegraph* staging = testsuite.storage->get_staging();

    const Partition* sdb1 = Partition::find_by_name(staging, "/dev/sdb1");
    const Btrfs* btrfs = to_btrfs(sdb1->get_blk_filesystem());

    BOOST_CHECK_EQUAL(btrfs->get_data_raid_level(), BtrfsRaidLevel::RAID5);
    BOOST_CHECK_EQUAL(btrfs->get_metadata_raid_level(), BtrfsRaidLevel::RAID6);
}


BOOST_AUTO_TEST_CASE(test3)
{
    Args args({ "--dry-run", "--yes", "create", "btrfs", "--pool", "HDDs (512 B)", "--size=24 GiB", "--devices",
	    "2", "--profiles", "raid1", "--path", "/test3" });

    vector<string> actions = {
	"Create partition /dev/sdb1 (12.00 GiB)",
	"Create partition /dev/sdc1 (12.00 GiB)",
	"Create btrfs on /dev/sdb1 (12.00 GiB) and /dev/sdc1 (12.00 GiB)",
	"Mount /dev/sdb1 (12.00 GiB) and /dev/sdc1 (12.00 GiB) at /test3",
	"Add mount point /test3 of /dev/sdb1 (12.00 GiB) and /dev/sdc1 (12.00 GiB) to /etc/fstab"
    };

    Testsuite testsuite;
    testsuite.devicegraph_filename = "empty2.xml";

    handle(args.argc(), args.argv(), &testsuite);

    BOOST_CHECK_EQUAL(actions, testsuite.actions);

    const Devicegraph* staging = testsuite.storage->get_staging();

    const Partition* sdb1 = Partition::find_by_name(staging, "/dev/sdb1");
    const Btrfs* btrfs = to_btrfs(sdb1->get_blk_filesystem());

    BOOST_CHECK_EQUAL(btrfs->get_data_raid_level(), BtrfsRaidLevel::RAID1);
    BOOST_CHECK_EQUAL(btrfs->get_metadata_raid_level(), BtrfsRaidLevel::RAID1);
}


BOOST_AUTO_TEST_CASE(test4)
{
    Args args({ "--dry-run", "--yes", "create", "btrfs", "--pool", "HDDs (512 B)", "--size", "max",
	    "--devices", "max" });

    vector<string> actions = {
	"Create partition /dev/sdb1 (32.00 GiB)",
	"Create partition /dev/sdc1 (32.00 GiB)",
	"Create partition /dev/sdd1 (32.00 GiB)",
	"Create partition /dev/sde1 (32.00 GiB)",
	"Create btrfs on /dev/sdb1 (32.00 GiB), /dev/sdc1 (32.00 GiB), /dev/sdd1 (32.00 GiB) and /dev/sde1 (32.00 GiB)"
    };

    Testsuite testsuite;
    testsuite.devicegraph_filename = "empty2.xml";

    handle(args.argc(), args.argv(), &testsuite);

    BOOST_CHECK_EQUAL(actions, testsuite.actions);

    const Devicegraph* staging = testsuite.storage->get_staging();

    const Partition* sdb1 = Partition::find_by_name(staging, "/dev/sdb1");
    const Btrfs* btrfs = to_btrfs(sdb1->get_blk_filesystem());

    BOOST_CHECK_EQUAL(btrfs->get_data_raid_level(), BtrfsRaidLevel::DEFAULT);
    BOOST_CHECK_EQUAL(btrfs->get_metadata_raid_level(), BtrfsRaidLevel::DEFAULT);
}
