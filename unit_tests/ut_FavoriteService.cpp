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
 *
 * Created on: Apr, 2014
 *     Author: k.dobkowski
 */

#include <string>

#include <boost/test/unit_test.hpp>
#include <boost/any.hpp>

#include "ServiceManager.h"
#include "BrowserLogger.h"
#include "BookmarkService.h"
#include "BookmarkItem.h"

#define TAG "[UT] Bookmarks - "

BOOST_AUTO_TEST_SUITE(bookmarks)

bool item_is_empty(std::shared_ptr<tizen_browser::services::BookmarkItem> item) { return item->getAddress() == std::string(); };

BOOST_AUTO_TEST_CASE(bookmark_add_remove)
{
    BROWSER_LOGI(TAG "bookmark_add_remove - START --> ");


    /// \todo: clean casts, depends on ServiceManager
    std::shared_ptr<tizen_browser::services::BookmarkService> fs =
    std::dynamic_pointer_cast
    <
        tizen_browser::services::BookmarkService,
        tizen_browser::core::AbstractService
    >
    (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.favoriteservice"));

//    fs->setStorageServiceTestMode();

    int bookcount = -1;
    int bookcount2 = -1;
    bool resultflag = false;

//  getBookmarks method test
    std::shared_ptr<tizen_browser::services::BookmarkItem> bitem;
    std::vector<std::shared_ptr<tizen_browser::services::BookmarkItem>> bookmarks_list = fs->getBookmarks();
    while(!bookmarks_list.empty()) {
	bitem = bookmarks_list.back();
        BROWSER_LOGI(TAG "Element from cached bookmark list: id: %d, title: %s, URL: %s", bitem->getId(),
			    bitem->getTitle().c_str(), bitem->getAddress().c_str());
	bookmarks_list.pop_back();
    }

    BROWSER_LOGI(TAG "Above - current stored bookmarks (recently adder order");

//  clean all bookmarks
    resultflag = fs->deleteAllBookmarks();
    BOOST_CHECK(resultflag);
    fs->getBookmarks();
    BROWSER_LOGI(TAG "Above - current stored bookmarks after deleteAll, deleting resultflag: %d", resultflag);

//  Empty bookmark test
    bookcount = fs->countBookmarks();
    BOOST_CHECK(item_is_empty(fs->addToBookmarks("","")));
    bookcount2 = fs->countBookmarks();
    BOOST_CHECK_EQUAL(bookcount, bookcount2);
    BROWSER_LOGI(TAG "Add empty bookmark test summary - number of bookmarks before: %d, after: %d", bookcount ,bookcount2);
    fs->getBookmarks();

//  Add bookmark with the same title
    BOOST_CHECK(!item_is_empty(fs->addToBookmarks("www.thisis.url1","Title")));
    BOOST_CHECK(!item_is_empty(fs->addToBookmarks("www.thisis.url4","Title")));
    std::shared_ptr<tizen_browser::services::BookmarkItem> item_to_delete = fs->addToBookmarks("www.thisis.url5","Title");
    BOOST_CHECK(!item_is_empty(item_to_delete));
    fs->getBookmarks();
    BROWSER_LOGI(TAG "Before delete last bookmark (%s)", item_to_delete->getAddress().c_str());
    BOOST_CHECK(fs->deleteBookmark(item_to_delete->getAddress()));
    BROWSER_LOGI(TAG "After delete bookmark");
    fs->getBookmarks();

//  Add duplicated url
    BROWSER_LOGI(TAG "Add duplicated url");
    BOOST_CHECK(item_is_empty(fs->addToBookmarks("www.thisis.url4","Not duplicateTitle")));
    fs->getBookmarks();

//  check existing url
    resultflag = fs->bookmarkExists("www.not_existing.url");
    BROWSER_LOGI(TAG "Check not existing url (%s) resultflag: %d", "www.not_existing.url", resultflag);
    BOOST_CHECK(!resultflag);
    resultflag = fs->bookmarkExists("www.thisis.url4");
    BROWSER_LOGI(TAG "Check existing url (%s) resultflag: %d", "www.thisis.url4", resultflag);
    BOOST_CHECK(resultflag);

    BROWSER_LOGI(TAG "--> END - bookmark_add_remove");
}

BOOST_AUTO_TEST_CASE(bookmark_synchro)
{
    /// \todo: clean casts, depends on ServiceManager
    BROWSER_LOGI(TAG "bookmark_synchro - START --> ");

    std::shared_ptr<tizen_browser::services::BookmarkService> fs =
    std::dynamic_pointer_cast
    <
        tizen_browser::services::BookmarkService,
        tizen_browser::core::AbstractService
    >
    (tizen_browser::core::ServiceManager::getInstance().getService("org.tizen.browser.favoriteservice"));

    BROWSER_LOGI(TAG "Bookmarks synchronize test");
    fs->synchronizeBookmarks();
    BOOST_CHECK(!fs->getBookmarks().empty());

    BROWSER_LOGI(TAG "--> END - bookmark_synchro");
}

BOOST_AUTO_TEST_SUITE_END()

