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

#include <memory>
#include <string>
#include <boost/test/unit_test.hpp>

#include "ServiceManager.h"
#include "BrowserLogger.h"
#include "StorageService.h"
#include "StorageException.h"
//#include "HistoryItem.h"

#define CHANNEL_AUTH01 "Gall Anonim 1"
#define CHANNEL_DESCR01 "Test channel - description 1"
#define CHANNEL_DATE01 "2001-01-20 23:59:59.000"
#define CHANNEL_URL01 "www.gallanonim-test1.com"
#define CHANNEL_TITLE01 "Test CHANNEL title 1"

#define CHANNEL_AUTH02 "Gall Anonim 2"
#define CHANNEL_DESCR02 "Test channel - description 2"
#define CHANNEL_DATE02 "2002-02-22 23:59:59.000"
#define CHANNEL_URL02 "www.gallanonim-test2.com"
#define CHANNEL_TITLE02 "Test CHANNEL title 2"

#define CHANNEL_AUTH11 "Gall Anonim 11"
#define CHANNEL_DESCR11 "Test channel - description 11"
#define CHANNEL_DATE11 "2011-01-20 23:59:59.000"
#define CHANNEL_URL11 "www.gallanonim-test11.com"
#define CHANNEL_TITLE11 "Test CHANNEL title 11"


#define ITEM_DESCRIPTION01 "Test item - description 1"
#define ITEM_DATE01 "1987-01-20 23:59:59.000"
#define ITEM_URL01 "www.item01.com"
#define ITEM_TITLE01 "Test ITEM title 1"

#define ITEM_DESCRIPTION02 "Test item - description 2"
#define ITEM_DATE02 "1988-01-20 23:59:59.000"
#define ITEM_URL02 "www.item02.com"
#define ITEM_TITLE02 "Test ITEM title 2"

#define ITEM_DESCRIPTION11 "Test item - description 11"
#define ITEM_DATE11 "1988-01-20 23:59:59.000"
#define ITEM_URL11 "www.item11.com"
#define ITEM_TITLE11 "Test ITEM title 11"

#define ITEM_DESCRIPTION12 "Test item - description 12"
#define ITEM_DATE12 "1989-01-20 23:59:59.000"
#define ITEM_URL12 "www.item12.com"
#define ITEM_TITLE12 "Test ITEM title 12"

#define ITEM_DESCRIPTION13 "Test item - description 13"
#define ITEM_DATE13 "1990-01-20 23:59:59.000"
#define ITEM_URL13 "www.item13.com"
#define ITEM_TITLE13 "Test ITEM title 13"

#define ITEM_DESCRIPTION14 "Test item - description 14"
#define ITEM_DATE14 "1991-01-20 23:59:59.000"
#define ITEM_URL14 "www.item14.com"
#define ITEM_TITLE14 "Test ITEM title 14"



BOOST_AUTO_TEST_SUITE(StorageService)


BOOST_AUTO_TEST_CASE(storage_settings)
{
    BROWSER_LOGI("[UT] StorageService - storage_settings - START --> ");

    std::shared_ptr<tizen_browser::services::StorageService> storageManager = std::dynamic_pointer_cast <
                                                                              tizen_browser::services::StorageService,
                                                                              tizen_browser::core::AbstractService > (
                                                                                  tizen_browser::core::ServiceManager::getInstance().getService(
                                                                                          DOMAIN_STORAGE_SERVICE));

    storageManager->init(true);

    const std::string keyInt = "keyINT";
    storageManager->setSettingsInt(keyInt, 12);
    auto i12 = storageManager->getSettingsInt(keyInt, 0);
    BOOST_CHECK(12 == i12);

    const std::string keyDouble = "keyDouble";
    storageManager->setSettingsDouble(keyDouble, 22.45);
    auto d22_45 = storageManager->getSettingsDouble(keyDouble, 0.0);
    BOOST_CHECK(22.45 == d22_45);

    const std::string keyString = "keyString";
    storageManager->setSettingsString(keyString, "String");
    auto sString = storageManager->getSettingsText(keyString, "0.0");
    BOOST_CHECK("String" == sString);

    BROWSER_LOGI("[UT] --> END - StorageService - storage_settings");
}

// Should it be moved to ut_historyService ????
//BOOST_AUTO_TEST_CASE(storage_history)
//{
//    BROWSER_LOGI("StorageService - history - START --> ");
//
//    std::shared_ptr<tizen_browser::services::StorageService> storageManager = std::dynamic_pointer_cast <
//                                                                              tizen_browser::services::StorageService,
//                                                                              tizen_browser::core::AbstractService > (
//                                                                                  tizen_browser::core::ServiceManager::getInstance().getService(
//                                                                                          DOMAIN_STORAGE_SERVICE));
//
//    storageManager->init(true);
//
//    std::shared_ptr<tizen_browser::tools::BrowserImage> bi = std::make_shared<tizen_browser::tools::BrowserImage>();
//    std::shared_ptr<tizen_browser::services::HistoryItem> hi = std::make_shared<tizen_browser::services::HistoryItem>("URL", "Title", bi);
//    storageManager->addHistoryItem(hi);
//
//    std::shared_ptr<tizen_browser::services::HistoryItem> hi2 = std::make_shared<tizen_browser::services::HistoryItem>("URL2", "Title2", bi);
//    storageManager->addHistoryItem(hi2);
//
//    auto countItems_2 = storageManager->getHistoryItems(100, 2);
//    auto countItems_1 = storageManager->getHistoryItems(100, 1);
//    BOOST_CHECK(countItems_2.size() == 2);
//    BOOST_CHECK(countItems_1.size() == 1);
//    storageManager->deleteHistory(hi2->getUrl());
//
//    auto hiauto = storageManager->getHistoryItem("URL");
//    BOOST_CHECK(hiauto->getTitle() == "Title");
//
//    auto iHistCount = storageManager->getHistoryItemsCount();
//    BROWSER_LOGD("iHistCount = %d", iHistCount);
//    BOOST_CHECK(iHistCount == 1);
//
//    auto iVisitCounter = storageManager->getHistoryVisitCounter(hiauto->getUrl());
//    BROWSER_LOGD("iVisitCounter = %d", iVisitCounter);
//    BOOST_CHECK(iVisitCounter == 1);
//
//    hi->setTitle("New Title");
//    storageManager->insertOrRefresh(hi);
//    auto newHistItem = storageManager->getHistoryItem("URL");
//    BOOST_CHECK(hi->getTitle() == "New Title");
//    auto iNewVisitCounter = storageManager->getHistoryVisitCounter(hi->getUrl());
//    BROWSER_LOGD("iVisitCounter = %d", iNewVisitCounter);
//    BOOST_CHECK(iNewVisitCounter == 2);
//
//    storageManager->deleteHistory();
//    iHistCount = storageManager->getHistoryItemsCount();
//    BROWSER_LOGD("iHistCount = %d", iHistCount);
//    BOOST_CHECK(iHistCount == 0);
//
//    BROWSER_LOGI("<-- StorageService - history - END");
//}


BOOST_AUTO_TEST_SUITE_END()
