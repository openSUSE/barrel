
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE getopt

#include <boost/test/unit_test.hpp>

#include "../barrel/Utils/Table.h"

using namespace barrel;


BOOST_AUTO_TEST_CASE(test1)
{
    Table table({ "Name", Cell("Size", Id::SIZE, Align::RIGHT), "Usage" });

    Table::Row sda(table, { "/dev/sda", "12.00 TiB", "GPT" });

    Table::Row sda1(table, { "/dev/sda1", "6.00 TiB", "xfs" });
    sda.add_subrow(sda1);

    Table::Row sda2(table, { "/dev/sda10", "1.00 TiB" });
    sda.add_subrow(sda2);

    table.add(sda);

    ostringstream o;
    o << table;

    string s =
	"Name         │      Size │ Usage\n"
	"─────────────┼───────────┼──────\n"
	"/dev/sda     │ 12.00 TiB │ GPT\n"
	"├─/dev/sda1  │  6.00 TiB │ xfs\n"
	"└─/dev/sda10 │  1.00 TiB │\n";

    BOOST_CHECK_EQUAL(o.str(), s);
}
