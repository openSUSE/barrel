
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE barrel

#include <boost/test/unit_test.hpp>

#include "../barrel/help.h"


using namespace std;
using namespace barrel;


BOOST_AUTO_TEST_CASE(test1)
{
    BOOST_CHECK_NO_THROW(help(true));
}
