/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * ut_Config.cpp
 * Unit test of Config component
 *
 * Created on: Mar 18, 2014
 *     Author: k.dobkowski
 */

#include <string>

#include <boost/test/unit_test.hpp>
#include <boost/any.hpp>

#include "BrowserLogger.h"
#include "Config.h"

#define TAG "[UT] Config - "

BOOST_AUTO_TEST_SUITE(config)

BOOST_AUTO_TEST_CASE(config_simple_get_set)
{
    BROWSER_LOGI(TAG "config_simple_get_set - START --> ");


    boost::any testvalue = tizen_browser::config::Config::getInstance().get(std::string("testkey"));
    BOOST_CHECK(testvalue.empty());

    int testval = 100;
    tizen_browser::config::Config::getInstance().set("intTestKey", testval);
    int retval = boost::any_cast<int>(defconf->get(std::string("intTestKey")));
    BOOST_CHECK_EQUAL(testval, retval);

    BROWSER_LOGI(TAG "Config - config_simple_get_set");
}


/*
 * This is test of boundary conditions
 */
BOOST_AUTO_TEST_CASE(config_boundary_conditions)
{
    BROWSER_LOGI(TAG "config_boundary_conditions - START --> ");

    std::string retstring;

// Wrong keys tests
    BOOST_CHECK(retstring.empty());
    boost::any retany;
    try{
    retany = tizen_browser::config::Config::getInstance().get(NULL);
    } catch(std::logic_error & e){
	/// \todo get() function expects string and cannot construct empty string from NULL
        BROWSER_LOGI(TAG "[i] Catched error, msg: %s",e.what());
        BROWSER_LOGI(TAG "[i] get() function expects string and cannot construct empty string from NULL\n");
    }

    try{
	retstring = boost::any_cast<std::string>(retany);
	BOOST_CHECK(retstring.empty());
    }catch(boost::bad_any_cast & e){
	/// \todo Need to resolve bad type (void *) from boost::any(empty_string))
        BROWSER_LOGI(TAG "[i] Catched error, msg: %s",e.what());
        BROWSER_LOGI(TAG "[i] std::map not found map[key] and returns NULL to boost::any_cast as type (void*) instead of std::string (this case)\n");
    }
    tizen_browser::config::Config::getInstance().set(std::string(""), std::string("value"));
    retstring = boost::any_cast<std::string>(tizen_browser::config::Config::getInstance().get(std::string("")));
    BOOST_CHECK_EQUAL(retstring, std::string("value"));

    tizen_browser::config::Config::getInstance().set(std::string(" "), std::string("anothervalue"));
    retstring = boost::any_cast<std::string>(tizen_browser::config::Config::getInstance().get(std::string(" ")));
    BOOST_CHECK_EQUAL(retstring, std::string("anothervalue"));

    tizen_browser::config::Config::getInstance().set(std::string("	"), std::string("value3"));
    retstring = boost::any_cast<std::string>(tizen_browser::config::Config::getInstance().get(std::string("	")));
    BOOST_CHECK_EQUAL(retstring, std::string("value3"));

// Wrong value tests
// NOTE Check that value is allowed to be empty.
    tizen_browser::config::Config::getInstance().set(std::string("TestKey"), std::string(""));
    retstring = boost::any_cast<std::string>(tizen_browser::config::Config::getInstance().get(std::string("TestKey")));
    BOOST_CHECK(retstring.empty());

    tizen_browser::config::Config::getInstance().set(std::string("TestKey"), std::string(" "));
    retstring = boost::any_cast<std::string>(tizen_browser::config::Config::getInstance().get(std::string("TestKey")));
    BOOST_CHECK(!retstring.empty());

    tizen_browser::config::Config::getInstance().set(std::string("AnotherTestKey"), std::string("	"));
    retstring = boost::any_cast<std::string>(tizen_browser::config::Config::getInstance().get(std::string("AnotherTestKey")));
    BOOST_CHECK(!retstring.empty());

// Set two the same keys or values
// NOTE Check that key or value are allowed have duplicates. This test case not allowed duplicates
    tizen_browser::config::Config::getInstance().set(std::string("SameTestKey"), std::string("valueA"));
    tizen_browser::config::Config::getInstance().set(std::string("SameTestKey"), std::string("valueB"));
    retstring = boost::any_cast<std::string>(tizen_browser::config::Config::getInstance().get(std::string("SameTestKey")));
    BOOST_CHECK_EQUAL(std::string("valueB"), retstring);
    BOOST_CHECK_PREDICATE( std::not_equal_to<std::string>(), (retstring)(std::string("valueA")) );

    BROWSER_LOGI(TAG "--> END - config_boundary_conditions");
}

BOOST_AUTO_TEST_SUITE_END()
