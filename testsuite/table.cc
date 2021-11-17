
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE barrel

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


BOOST_AUTO_TEST_CASE(test2)
{
    Table table({ "Name", Cell("Size", Id::SIZE, Align::RIGHT) });

    Table::Row vg(table, { "/dev/vg", "12.00 TiB" });

    Table::Row pool1(table, { "/dev/vg/pool1", "3.00 TiB" });

    Table::Row thin1a(table, { "/dev/vg/thin1a", "2.00 TiB" });
    pool1.add_subrow(thin1a);

    Table::Row thin1b(table, { "/dev/vg/thin1b", "2.00 TiB" });
    pool1.add_subrow(thin1b);

    vg.add_subrow(pool1);

    Table::Row pool2(table, { "/dev/vg/pool2", "6.00 TiB" });

    Table::Row thin2a(table, { "/dev/vg/thin2a", "4.00 TiB" });
    pool2.add_subrow(thin2a);

    Table::Row thin2b(table, { "/dev/vg/thin2b", "4.00 TiB" });
    pool2.add_subrow(thin2b);

    vg.add_subrow(pool2);

    table.add(vg);

    ostringstream o;
    o << table;

    string s =
	"Name               │      Size\n"
	"───────────────────┼──────────\n"
	"/dev/vg            │ 12.00 TiB\n"
	"├─/dev/vg/pool1    │  3.00 TiB\n"
	"│ ├─/dev/vg/thin1a │  2.00 TiB\n"
	"│ └─/dev/vg/thin1b │  2.00 TiB\n"
	"└─/dev/vg/pool2    │  6.00 TiB\n"
	"  ├─/dev/vg/thin2a │  4.00 TiB\n"
	"  └─/dev/vg/thin2b │  4.00 TiB\n";

    BOOST_CHECK_EQUAL(o.str(), s);
}


BOOST_AUTO_TEST_CASE(test3)
{
    Table table({ "Name", "Level" });

    Table::Row a(table, { "a", "1" });
    Table::Row b(table, { "b", "2" });
    Table::Row c(table, { "c", "3" });
    Table::Row d(table, { "d", "4" });
    Table::Row e(table, { "e", "4" });
    Table::Row f(table, { "f", "3" });
    Table::Row g(table, { "g", "2" });
    Table::Row h(table, { "h", "1" });

    c.add_subrow(d);
    c.add_subrow(e);
    b.add_subrow(c);
    b.add_subrow(f);
    a.add_subrow(b);
    a.add_subrow(g);
    table.add(a);
    table.add(h);

    ostringstream o;
    o << table;

    string s =
	"Name    │ Level\n"
	"────────┼──────\n"
	"a       │ 1\n"
	"├─b     │ 2\n"
	"│ ├─c   │ 3\n"
	"│ │ ├─d │ 4\n"
	"│ │ └─e │ 4\n"
	"│ └─f   │ 3\n"
	"└─g     │ 2\n"
	"h       │ 1\n";

    BOOST_CHECK_EQUAL(o.str(), s);
}
