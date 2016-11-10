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

#include <boost/test/unit_test.hpp>
#include <Elementary.h>
#include <EWebKit.h>
#include <EWebKit_internal.h>

#include "ServiceManager.h"
#include "AbstractWebEngine.h"
#include "BrowserLogger.h"

#define TAG "[UT] WebKitEngine - "


BOOST_AUTO_TEST_SUITE(WebEngineService)

BOOST_AUTO_TEST_CASE(EwkInit)
{
    BROWSER_LOGI(TAG "EwkInit - START --> ");
    BOOST_REQUIRE(ewk_init() > 0);
    BROWSER_LOGI(TAG "--> END - EwkInit");
}

BOOST_AUTO_TEST_CASE(UriSetGet)
{
    BROWSER_LOGI(TAG "UriSetGet - START --> ");

    std::shared_ptr<tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>> webEngineService =
        std::dynamic_pointer_cast
        <tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.webengineservice"));

    BOOST_CHECK(webEngineService);

//    elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
//    Evas_Object *main_window = elm_win_util_standard_add("browserApp-ut", "browserApp-ut");
//    if (main_window == nullptr)
//        BROWSER_LOGE(TAG "Failed to create main window");
//    elm_win_autodel_set(main_window, EINA_TRUE);
    Evas_Object *main_window = nullptr;

    webEngineService->init(main_window);

    tizen_browser::basic_webengine::TabId parentTabId = webEngineService->addTab("www.test.com");

    BOOST_CHECK(!(webEngineService->getLayout()));

    webEngineService->setURI("www.test2.com");

    // URIs are different because of WebKit didn't load webpage (lack of initialization)
    BOOST_TEST_MESSAGE(TAG "Print getURI():" << webEngineService->getURI());

    BOOST_TEST_MESSAGE(TAG "Print getTitle():" << webEngineService->getTitle());

    BOOST_TEST_MESSAGE(TAG "Print closeTab():" << webEngineService->closeTab());

    BROWSER_LOGI(TAG "--> END - UriSetGet");
}

BOOST_AUTO_TEST_CASE(NavigationTest)
{
    BROWSER_LOGI(TAG "NavigationTest - START --> ");

    std::shared_ptr<tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>> webEngineService =
        std::dynamic_pointer_cast
        <tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.webengineservice"));

    BOOST_CHECK(webEngineService);

    webEngineService->init(NULL);

    tizen_browser::basic_webengine::TabId parentTabId = webEngineService->addTab("www.test2.com");

    webEngineService->stopLoading();

    webEngineService->reload();

    webEngineService->setURI("www.nextpage.com");

    BOOST_TEST_MESSAGE(TAG "Is back enabled: " << webEngineService->isBackEnabled());

    webEngineService->back();

    BOOST_TEST_MESSAGE(TAG "Is forward enabled: " << webEngineService->isForwardEnabled());

    webEngineService->forward();

    BOOST_TEST_MESSAGE(TAG "Is loading: " << webEngineService->isLoading());

    BOOST_TEST_MESSAGE(TAG "Print closeTab():" << webEngineService->closeTab());

    BROWSER_LOGI(TAG "--> END - NavigationTest");
}

BOOST_AUTO_TEST_CASE(ClearPrivateData)
{
    BROWSER_LOGI(TAG "ClearPrivateData - START --> ");

    std::shared_ptr<tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>> webEngineService =
        std::dynamic_pointer_cast
        <tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.webengineservice"));

    BOOST_CHECK(webEngineService);

    tizen_browser::basic_webengine::TabId parentTabId = webEngineService->addTab("www.clearingdata.com");

    webEngineService->clearPrivateData();

    BOOST_TEST_MESSAGE(TAG "Print closeTab():" << webEngineService->closeTab());

    BROWSER_LOGI(TAG "--> END - ClearPrivateData");
}

BOOST_AUTO_TEST_CASE(TabsCreationDeletion)
{
    BROWSER_LOGI(TAG "TabsCreationDeletion - START --> ");

    std::shared_ptr<tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>> webEngineService =
        std::dynamic_pointer_cast
        <tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.webengineservice"));

    BOOST_CHECK(webEngineService);

    BOOST_TEST_MESSAGE(TAG "Tabs count: " << webEngineService->tabsCount());

    tizen_browser::basic_webengine::TabId first = webEngineService->addTab("www.first.com");

    BOOST_CHECK(webEngineService->tabsCount() == 1);

    tizen_browser::basic_webengine::TabId second = webEngineService->addTab("www.second.com");

    BOOST_CHECK(webEngineService->tabsCount() == 2);

    BOOST_TEST_MESSAGE(TAG "Print closeTab():" << webEngineService->closeTab(first));

    BOOST_CHECK(webEngineService->tabsCount() == 1);

    BOOST_TEST_MESSAGE(TAG "Print closeTab():" << webEngineService->closeTab(second));

    BOOST_CHECK(!(webEngineService->tabsCount()));

    BROWSER_LOGI(TAG "--> END - TabsCreationDeletion");
}

BOOST_AUTO_TEST_CASE(TabsSwitching)
{
    BROWSER_LOGI(TAG "TabsSwitching - START --> ");

    std::shared_ptr<tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>> webEngineService =
        std::dynamic_pointer_cast
        <tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.webengineservice"));

    BOOST_CHECK(webEngineService);

    BOOST_TEST_MESSAGE(TAG "Tabs count: " << webEngineService->tabsCount());

    BOOST_TEST_MESSAGE(TAG "Initial current tab: " << webEngineService->currentTabId().toString());

    tizen_browser::basic_webengine::TabId first = webEngineService->addTab("www.first.com");
    BOOST_TEST_MESSAGE(TAG "First tab: " << first.toString());

    webEngineService->switchToTab(first);

    BOOST_CHECK(webEngineService->currentTabId() == first);

    BOOST_CHECK(webEngineService->tabsCount() == 1);

    tizen_browser::basic_webengine::TabId second = webEngineService->addTab("www.second.com");
    BOOST_TEST_MESSAGE(TAG "Second tab: " << second.toString());

    BOOST_CHECK(webEngineService->currentTabId() == first);

    webEngineService->switchToTab(second);

    BOOST_CHECK(webEngineService->currentTabId() == second);

    BOOST_CHECK(webEngineService->tabsCount() == 2);

    BOOST_TEST_MESSAGE(TAG "Print closeTab():" << webEngineService->closeTab(second));

    BOOST_CHECK(webEngineService->currentTabId() == first);

    BOOST_CHECK(webEngineService->tabsCount() == 1);

    BOOST_TEST_MESSAGE(TAG "Print closeTab():" << webEngineService->closeTab(first));

    BOOST_CHECK(!(webEngineService->tabsCount()));

    BROWSER_LOGI(TAG "--> END - TabsSwitching");
}

BOOST_AUTO_TEST_CASE(Snapshots)
{
    BROWSER_LOGI(TAG "Snapshots - START --> ");

    std::shared_ptr<tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>> webEngineService =
        std::dynamic_pointer_cast
        <tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.webengineservice"));

    BOOST_CHECK(webEngineService);

    webEngineService->init(NULL);

    tizen_browser::basic_webengine::TabId parentTabId = webEngineService->addTab("www.test.com");

    webEngineService->getSnapshotData(100, 100);

    webEngineService->getSnapshotData(parentTabId, 100, 100);

    BOOST_TEST_MESSAGE(TAG "Print closeTab():" << webEngineService->closeTab());

    BROWSER_LOGI(TAG "--> END - Snapshots");
}

BOOST_AUTO_TEST_CASE(PrivateModeOnOff)
{
    BROWSER_LOGI(TAG "PrivateModeOnOff - START --> ");

    std::shared_ptr<tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>> webEngineService =
        std::dynamic_pointer_cast
        <tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.webengineservice"));

    BOOST_CHECK(webEngineService);

    webEngineService->init(NULL);

    tizen_browser::basic_webengine::TabId parentTabId = webEngineService->addTab("www.test.com");

    BOOST_CHECK(!(webEngineService->isSecretMode()));

    webEngineService->setPrivateMode(true);

    BOOST_CHECK(webEngineService->isSecretMode());

    webEngineService->setPrivateMode(false);

    BOOST_CHECK(!(webEngineService->isSecretMode()));

    BOOST_TEST_MESSAGE(TAG "Print closeTab():" << webEngineService->closeTab());

    BROWSER_LOGI(TAG "--> END - PrivateModeOnOff");
}

BOOST_AUTO_TEST_CASE(LoadErrorDefaultValue)
{
    BROWSER_LOGI(TAG "LoadErrorDefaultValue - START --> ");

    std::shared_ptr<tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>> webEngineService =
        std::dynamic_pointer_cast
        <tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.webengineservice"));

    BOOST_CHECK(webEngineService);

    webEngineService->init(NULL);

    tizen_browser::basic_webengine::TabId parentTabId = webEngineService->addTab("www.test.com");

    BOOST_CHECK(!(webEngineService->isLoadError()));

    BOOST_TEST_MESSAGE(TAG "Print closeTab():" << webEngineService->closeTab());

    BROWSER_LOGI(TAG "--> END - LoadErrorDefaultValue");
}

BOOST_AUTO_TEST_CASE(Focus)
{
    BROWSER_LOGI(TAG "Focus - START --> ");

    std::shared_ptr<tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>> webEngineService =
        std::dynamic_pointer_cast
        <tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.webengineservice"));

    BOOST_CHECK(webEngineService);

    webEngineService->init(NULL);

    tizen_browser::basic_webengine::TabId parentTabId = webEngineService->addTab("www.test.com");

    BOOST_TEST_MESSAGE(TAG "Has focus at start:" << webEngineService->hasFocus());

    webEngineService->setFocus();

    BOOST_TEST_MESSAGE(TAG "Has focus after setting focus:" << webEngineService->hasFocus());

    webEngineService->clearFocus();

    BOOST_TEST_MESSAGE(TAG "Has focus after clearing focus:" << webEngineService->hasFocus());

    webEngineService->setFocus();

    BOOST_TEST_MESSAGE(TAG "Has focus after setting focus:" << webEngineService->hasFocus());

    BOOST_TEST_MESSAGE(TAG "Print closeTab():" << webEngineService->closeTab());

    BROWSER_LOGI(TAG "--> END - Focus");
}

BOOST_AUTO_TEST_CASE(ZoomAndAutofit)
{
    BROWSER_LOGI(TAG "ZoomAndAutofit - START --> ");

    std::shared_ptr<tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>> webEngineService =
        std::dynamic_pointer_cast
        <tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.webengineservice"));

    BOOST_CHECK(webEngineService);

    webEngineService->init(NULL);

    tizen_browser::basic_webengine::TabId parentTabId = webEngineService->addTab("www.test.com");

    BOOST_TEST_MESSAGE(TAG "Zoom factor at start:" << webEngineService->getZoomFactor());

    webEngineService->setZoomFactor(200);

    BOOST_TEST_MESSAGE(TAG "Zoom factor after setting 200:" << webEngineService->getZoomFactor());

    webEngineService->setZoomFactor(100);

    BOOST_TEST_MESSAGE(TAG "Zoom factor after setting 100:" << webEngineService->getZoomFactor());

    BOOST_TEST_MESSAGE(TAG "Print closeTab():" << webEngineService->closeTab());

    BROWSER_LOGI(TAG "--> END - ZoomAndAutofit");
}

BOOST_AUTO_TEST_CASE(Favicon)
{
    BROWSER_LOGI(TAG "Favicon - START --> ");

    std::shared_ptr<tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>> webEngineService =
        std::dynamic_pointer_cast
        <tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.webengineservice"));

    BOOST_CHECK(webEngineService);

    webEngineService->init(NULL);

    tizen_browser::basic_webengine::TabId parentTabId = webEngineService->addTab("www.test.com");

    webEngineService->getFavicon();

    BOOST_TEST_MESSAGE(TAG "Print closeTab():" << webEngineService->closeTab());

    BROWSER_LOGI(TAG "--> END - Favicon");
}


BOOST_AUTO_TEST_CASE(SearchOnWebsite)
{
    BROWSER_LOGI(TAG "SearchOnWebsite - START --> ");

    std::shared_ptr<tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>> webEngineService =
        std::dynamic_pointer_cast
        <tizen_browser::basic_webengine::AbstractWebEngine<Evas_Object>,tizen_browser::core::AbstractService>
        (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.webengineservice"));

    BOOST_CHECK(webEngineService);

    webEngineService->init(NULL);

    tizen_browser::basic_webengine::TabId parentTabId = webEngineService->addTab("www.test.com");

    webEngineService->searchOnWebsite("test", 0);

    webEngineService->searchOnWebsite("test2", 0);

    webEngineService->searchOnWebsite("", 0);

    BOOST_TEST_MESSAGE(TAG "Print closeTab():" << webEngineService->closeTab());

    BROWSER_LOGI(TAG "--> END - SearchOnWebsite");
}

//BOOST_AUTO_TEST_CASE(EwkShutdown)
//{
//    BROWSER_LOGI(TAG "EwkShutdown - START --> ");
//    BOOST_CHECK(ewk_shutdown() == 0);
//    BROWSER_LOGI(TAG "--> END - EwkShutdown");
//}

BOOST_AUTO_TEST_SUITE_END()
