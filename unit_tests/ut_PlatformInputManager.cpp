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
 *
 *
 */

#include <boost/test/unit_test.hpp>

#include "ServiceManager.h"
#include "PlatformInputManager.h"
#include "BrowserLogger.h"


BOOST_AUTO_TEST_SUITE(PlatformInputManager)

BOOST_AUTO_TEST_CASE(PointerModeSetting)
{
    BROWSER_LOGI("[UT] PlatformInputManager - PointerModeSetting - START --> ");

        std::shared_ptr<tizen_browser::services::PlatformInputManager> platformInputManager =
        std::dynamic_pointer_cast
        <tizen_browser::services::PlatformInputManager, tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.platforminputmanager"));

    BOOST_REQUIRE(platformInputManager);
//
//    BOOST_CHECK(platformInputManager->getPointerModeEnabled());
//
//    platformInputManager->setPointerModeEnabled(false);
//
//    BOOST_CHECK(!(platformInputManager->getPointerModeEnabled()));
//
//    platformInputManager->setPointerModeEnabled(true);
//
//    BOOST_CHECK(platformInputManager->getPointerModeEnabled());

    BROWSER_LOGI("[UT] --> END - PlatformInputManager - PointerModeSetting");
}

BOOST_AUTO_TEST_SUITE_END()
