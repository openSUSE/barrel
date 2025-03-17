
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE barrel

#include <boost/test/unit_test.hpp>

#include "../barrel/Utils/Text.h"


using namespace barrel;


namespace std
{
    ostream& operator<<(ostream& s, const vector<string>& v)
    {
	for (vector<string>::const_iterator it = v.begin(); it != v.end(); ++it)
	{
	    if (it != v.begin())
		s << ' ';
	    s << '{' << *it << '}';
	}

	return s;
    }
}


BOOST_AUTO_TEST_CASE(test1)
{
    vector<string> res = { "pool", "--name", "HDDs (512 B)" };

    BOOST_CHECK_EQUAL(parse_line("pool --name HDDs\\ (512\\ B)"), res);

    BOOST_CHECK_EQUAL(parse_line("pool --name 'HDDs (512 B)'"), res);
    BOOST_CHECK_EQUAL(parse_line("pool --name 'HDDs'' (512 B)'"), res);

    BOOST_CHECK_EQUAL(parse_line("pool --name \"HDDs (512 B)\""), res);
    BOOST_CHECK_EQUAL(parse_line("pool --name \"HDDs\"\" (512 B)\""), res);

    BOOST_CHECK_EQUAL(parse_line("pool --name 'HDDs'\\ \"(512 B)\""), res);
}


BOOST_AUTO_TEST_CASE(test2)
{
    vector<string> res = { "a\\bc" };

    BOOST_CHECK_EQUAL(parse_line("a\\\\bc"), res);
    BOOST_CHECK_EQUAL(parse_line("'a\\bc'"), res);
    BOOST_CHECK_EQUAL(parse_line("\"a\\bc\""), res);
}
