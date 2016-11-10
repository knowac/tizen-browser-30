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

#include <boost/test/included/unit_test.hpp>
#include "ServiceManager.h"
#define BOOST_TEST_NO_MAIN

::boost::unit_test::test_suite* init_unit_test_suite(int , char** )
{
	//#ifndef NDEBUG
	//Initialization of logger module
	tizen_browser::logger::Logger::getInstance().init();
	tizen_browser::logger::Logger::getInstance().setLogTag("browser");
	//#endif
	using namespace ::boost::unit_test;
	assign_op(framework::master_test_suite().p_name.value, BOOST_TEST_STRINGIZE(BOOST_TEST_MODULE).trim("\""), 0);

	return 0;
}

//int main(int argc, char * argv[]) { return ::boost::unit_test::unit_test_main( &init_unit_test_suite, argc, argv ); }
