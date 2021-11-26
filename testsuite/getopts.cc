
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE barrel

#include <boost/test/unit_test.hpp>

#include "../barrel/Utils/GetOpts.h"
#include "../barrel/Utils/Args.h"


using namespace barrel;


namespace std
{
    ostream& operator<<(ostream& s, const vector<string>& strings)
    {
	s << "{";
	for (vector<string>::const_iterator it = strings.begin(); it != strings.end(); ++it)
	    s << (it == strings.begin() ? " " : ", ") << *it;
	s << " }";

	return s;
    }
}


const ExtOptions global_opts({
    { "verbose", no_argument, 'v', "be verbose" },
    { "dry-run", no_argument, 0, "dry run" },
    { "table-style", required_argument, 't', "set table style", "table-style" }
});


const ExtOptions raid_opts({
    { "size", required_argument, 's', "set size", "size" }
}, TakeBlkDevices::YES);


const ExtOptions filesystem_opts({
    { "type", required_argument, 't', "set type", "type" }
});


BOOST_AUTO_TEST_CASE(good1)
{
    Args args({ "--verbose" });
    GetOpts get_opts(args.argc(), args.argv());

    ParsedOpts parsed_global_opts = get_opts.parse(global_opts);

    BOOST_CHECK(parsed_global_opts.has_option("verbose"));
    BOOST_CHECK(!parsed_global_opts.has_option("read-my-mind"));

    BOOST_CHECK(!get_opts.has_args());
}


BOOST_AUTO_TEST_CASE(good2)
{
    Args args({ "-v", "create", "raid", "--size", "1 TiB", "/dev/sd[cd]1" });
    GetOpts get_opts(args.argc(), args.argv(), true, { "/dev/sda", "/dev/sdb", "/dev/sdc",
	    "/dev/sdc1", "/dev/sdc2", "/dev/sdd", "/dev/sdd1", "/dev/sdd2" });

    // parse global options

    ParsedOpts parsed_global_opts = get_opts.parse(global_opts);

    BOOST_CHECK(parsed_global_opts.has_option("verbose"));
    BOOST_CHECK(!parsed_global_opts.has_option("dry-run"));

    // check that command is there

    BOOST_CHECK(get_opts.has_args());
    BOOST_CHECK_EQUAL(get_opts.pop_arg(), "create");

    // check that sub command is there

    BOOST_CHECK(get_opts.has_args());
    BOOST_CHECK_EQUAL(get_opts.pop_arg(), "raid");

    // parse raid options

    ParsedOpts parsed_raid_opts = get_opts.parse("raid", raid_opts);

    BOOST_CHECK(parsed_raid_opts.has_option("size"));
    BOOST_CHECK_EQUAL(parsed_raid_opts.get("size"), "1 TiB");

    BOOST_CHECK(parsed_raid_opts.get_optional("size"));
    BOOST_CHECK_EQUAL(parsed_raid_opts.get_optional("size").value(), "1 TiB");

    BOOST_CHECK_EQUAL(parsed_raid_opts.get_blk_devices(), vector<string>({ "/dev/sdc1", "/dev/sdd1" }));

    // check that no further command is there

    BOOST_CHECK(!get_opts.has_args());
}


BOOST_AUTO_TEST_CASE(good3)
{
    Args args({ "--verbose", "create", "raid", "--size=1 TiB", "/dev/sdc1", "/dev/sdd1",
	    "filesystem", "--type=xfs" });
    GetOpts get_opts(args.argc(), args.argv());

    // parse global options

    ParsedOpts parsed_global_opts = get_opts.parse(global_opts);

    BOOST_CHECK(parsed_global_opts.has_option("verbose"));

    // check that command is there

    BOOST_CHECK(get_opts.has_args());
    BOOST_CHECK_EQUAL(get_opts.pop_arg(), "create");

    // check that first sub command is there

    BOOST_CHECK(get_opts.has_args());
    BOOST_CHECK_EQUAL(get_opts.pop_arg(), "raid");

    // parse first sub command options

    ParsedOpts parsed_raid_opts = get_opts.parse("raid", raid_opts);

    BOOST_CHECK(parsed_raid_opts.has_option("size"));
    BOOST_CHECK_EQUAL(parsed_raid_opts.get("size"), "1 TiB");

    BOOST_CHECK_EQUAL(parsed_raid_opts.get_blk_devices(), vector<string>({ "/dev/sdc1", "/dev/sdd1" }));

    // check that second sub command is there

    BOOST_CHECK(get_opts.has_args());
    BOOST_CHECK_EQUAL(get_opts.pop_arg(), "filesystem");

    // parse second sub command options

    ParsedOpts parsed_filesystem_opts = get_opts.parse("filesystem", filesystem_opts);

    BOOST_CHECK(parsed_filesystem_opts.has_option("type"));
    BOOST_CHECK_EQUAL(parsed_filesystem_opts.get("type"), "xfs");

    // check that no further command is there

    BOOST_CHECK(!get_opts.has_args());
}


BOOST_AUTO_TEST_CASE(good4)
{
    Args args({ "--verbose", "create", "raid", "--size=1 TiB", "/dev/sd[cd]1" });
    GetOpts get_opts(args.argc(), args.argv(), true, { "/dev/sdc", "/dev/sdd" });

    // parse global options

    ParsedOpts parsed_global_opts = get_opts.parse(global_opts);

    BOOST_CHECK(parsed_global_opts.has_option("verbose"));

    // check that command is there

    BOOST_CHECK(get_opts.has_args());
    BOOST_CHECK_EQUAL(get_opts.pop_arg(), "create");

    // check that first sub command is there

    BOOST_CHECK(get_opts.has_args());
    BOOST_CHECK_EQUAL(get_opts.pop_arg(), "raid");

    // parse first sub command options

    ParsedOpts parsed_raid_opts = get_opts.parse("raid", raid_opts);

    BOOST_CHECK(parsed_raid_opts.has_option("size"));
    BOOST_CHECK_EQUAL(parsed_raid_opts.get("size"), "1 TiB");

    BOOST_CHECK_EQUAL(parsed_raid_opts.get_blk_devices(), vector<string>({ "/dev/sd[cd]1" }));

    BOOST_CHECK(!get_opts.has_args());
}


BOOST_AUTO_TEST_CASE(error1)
{
    Args args({ "--table-style" });
    GetOpts get_opts(args.argc(), args.argv());

    BOOST_CHECK_EXCEPTION(get_opts.parse(global_opts), runtime_error, [](const exception& e) {
	return strcmp(e.what(), "Missing argument for global option '--table-style'.") == 0;
    });
}


BOOST_AUTO_TEST_CASE(error2)
{
    Args args({ "create", "raid", "--size" });
    GetOpts get_opts(args.argc(), args.argv());

    get_opts.parse(global_opts);
    get_opts.pop_arg();
    get_opts.pop_arg();

    BOOST_CHECK_EXCEPTION(get_opts.parse("raid", raid_opts), runtime_error, [](const exception& e) {
	return strcmp(e.what(), "Missing argument for command option '--size'.") == 0;
    });
}


BOOST_AUTO_TEST_CASE(error3)
{
    Args args({ "--read-my-mind" });
    GetOpts get_opts(args.argc(), args.argv());

    BOOST_CHECK_EXCEPTION(get_opts.parse(global_opts), runtime_error, [](const exception& e) {
	return strcmp(e.what(), "Unknown global option '--read-my-mind'.") == 0;
    });
}


BOOST_AUTO_TEST_CASE(error4)
{
    Args args({ "create", "raid", "--read-my-mind" });
    GetOpts get_opts(args.argc(), args.argv());

    get_opts.parse(global_opts);
    get_opts.pop_arg();
    get_opts.pop_arg();

    BOOST_CHECK_EXCEPTION(get_opts.parse("raid", raid_opts), runtime_error, [](const exception& e) {
	return strcmp(e.what(), "Unknown option '--read-my-mind' for command 'raid'.") == 0;
    });
}


BOOST_AUTO_TEST_CASE(error5)
{
    Args args({ "/dev/sdz", "show" });
    GetOpts get_opts(args.argc(), args.argv());

    BOOST_CHECK_EXCEPTION(get_opts.parse(global_opts), runtime_error, [](const exception& e) {
	return strcmp(e.what(), "No global block devices allowed.") == 0;
    });
}


BOOST_AUTO_TEST_CASE(error6)
{
    Args args({ "show", "disks", "/dev/sdz" });
    GetOpts get_opts(args.argc(), args.argv());

    get_opts.parse(GetOpts::no_ext_options);
    get_opts.pop_arg();
    get_opts.pop_arg();

    BOOST_CHECK_EXCEPTION(get_opts.parse("disks", GetOpts::no_ext_options), runtime_error, [](const exception& e) {
	return strcmp(e.what(), "No block devices allowed for command option 'disks'.") == 0;
    });
}
