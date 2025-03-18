
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE barrel

#include <boost/test/unit_test.hpp>

#include <storage/Utils/HumanString.h>

#include "../barrel/Utils/Misc.h"


using namespace barrel;


BOOST_AUTO_TEST_CASE(test1)
{
    SmartSize s(" max ");

    BOOST_CHECK_EQUAL(s.type, SmartSize::MAX);
    BOOST_CHECK_EQUAL(s.value, 0);

    BOOST_CHECK_EQUAL(s.get(10 * GiB), 10 * GiB);
}


BOOST_AUTO_TEST_CASE(test2)
{
    SmartSize s(" 1 GiB ");

    BOOST_CHECK_EQUAL(s.type, SmartSize::ABSOLUTE);
    BOOST_CHECK_EQUAL(s.value, 1 * GiB);

    BOOST_CHECK_EQUAL(s.get(10 * GiB), 1 * GiB);
}


BOOST_AUTO_TEST_CASE(test3)
{
    BOOST_CHECK_EXCEPTION(SmartSize s(" 0 B "), runtime_error, [](const exception& e) {
	return strcmp(e.what(), "invalid size ' 0 B '") == 0;
    });
}


BOOST_AUTO_TEST_CASE(test4)
{
    BOOST_CHECK_EXCEPTION(SmartSize s(" + 1 GiB "), runtime_error, [](const exception& e) {
	return strcmp(e.what(), "neither + nor - allowed in size") == 0;
    });
}


BOOST_AUTO_TEST_CASE(test5)
{
    SmartSize s(" + 2 GiB ", true);

    BOOST_CHECK_EQUAL(s.type, SmartSize::PLUS);
    BOOST_CHECK_EQUAL(s.value, 2 * GiB);

    BOOST_CHECK_EQUAL(s.get(10 * GiB, 5 * GiB), 7 * GiB);

    BOOST_CHECK_EQUAL(s.get(10 * GiB, 16 * EiB - 1 * GiB), 16 * EiB - 1);	// overflow
}


BOOST_AUTO_TEST_CASE(test6)
{
    SmartSize s(" - 2 GiB ", true);

    BOOST_CHECK_EQUAL(s.type, SmartSize::MINUS);
    BOOST_CHECK_EQUAL(s.value, 2 * GiB);

    BOOST_CHECK_EQUAL(s.get(10 * GiB, 5 * GiB), 3 * GiB);

    BOOST_CHECK_EQUAL(s.get(10 * GiB, 1 * GiB), 0 * GiB);	// underflow
}
