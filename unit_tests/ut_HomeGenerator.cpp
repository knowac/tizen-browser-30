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

#include <string>

#include <boost/signals2.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/concept_check.hpp>

#include "services/HomeGenerator/HomeGenerator.h"

BOOST_AUTO_TEST_SUITE(HomeGenerator)

BOOST_AUTO_TEST_CASE(getHomePageUrl)
{
    tizen_browser::services::HomeGenerator hg;
    BOOST_CHECK_EQUAL(hg.getHomePageUrl(), std::string("/tmp/HomePage.html"));
}

BOOST_AUTO_TEST_SUITE_END()
