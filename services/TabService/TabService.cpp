/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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

#include "BrowserImage.h"
#include "EflTools.h"
#include "BrowserLogger.h"
#include "TabService.h"
#include "Blob.h"
#include "TabId.h"
#include <web/web_tab.h>
#include "CapiWebErrorCodes.h"

namespace tizen_browser {
namespace services {

EXPORT_SERVICE(TabService, DOMAIN_TAB_SERVICE)

TabService::TabService()
{
    if (bp_tab_adaptor_initialize() < 0) {
        errorPrint("bp_tab_adaptor_initialize");
    }
}

TabService::~TabService()
{
    if (bp_tab_adaptor_deinitialize() < 0) {
        errorPrint("bp_tab_adaptor_deinitialize");
    }
}

// return -1 when tab adaptor is not created
int TabService::createTabId(int tabId) const
{
    int adaptorId = tabId;
    if (bp_tab_adaptor_create(&adaptorId) < 0) {
        errorPrint("bp_tab_adaptor_create");
    }
    return adaptorId;
}

boost::optional<int> TabService::convertTabId(std::string tabId) const
{
    try {
        boost::optional<int> tabIdConverted = std::stoi(tabId);
        return tabIdConverted;
    } catch (const std::exception& /*e*/) {
        BROWSER_LOGE("%s can't convert %s to tab id", __PRETTY_FUNCTION__,
                tabId.c_str());
        return boost::none;
    }
}

void TabService::errorPrint(std::string method) const
{
    int error_code = bp_tab_adaptor_get_errorcode();
    BROWSER_LOGE("%s error: %d (%s)", method.c_str(), error_code,
            tools::capiWebError::tabErrorToString(error_code).c_str());
}

std::shared_ptr<std::vector<basic_webengine::TabContent> > TabService::getAllTabs()
{
    BROWSER_LOGD("[%s:%d]", __PRETTY_FUNCTION__, __LINE__);
    int* items = nullptr;
    int count;
    auto vec = std::make_shared<std::vector<basic_webengine::TabContent> >(std::vector<basic_webengine::TabContent>());
    if (bp_tab_adaptor_get_full_ids_p(&items, &count) < 0) {
        errorPrint("bp_tab_adaptor_get_full_ids_p");
        return vec;
    }

    for (int i = 0; i < count; ++i) {
        bp_tab_info_fmt info;
        if (bp_tab_adaptor_get_easy_all(items[i], &info) < 0) {
            errorPrint("bp_tab_adaptor_get_easy_all");
            continue;
        }
        if (!info.url) {
            BROWSER_LOGW("[%s:%d] unknown url!", __PRETTY_FUNCTION__, __LINE__);
            continue;
        }
        if (!info.title) {
            BROWSER_LOGW("[%s:%d] unknown title!", __PRETTY_FUNCTION__, __LINE__);
            continue;
        }
        if (!info.index) {
            BROWSER_LOGW("[%s:%d] unknown index!", __PRETTY_FUNCTION__, __LINE__);
            continue;
        }
        vec->push_back(basic_webengine::TabContent(basic_webengine::TabId(items[i]),
                std::string(info.url),
                std::string(info.title),
                basic_webengine::TabOrigin(info.index)));
    }
    if (count > 0)
        free(items);

    return vec;
}

tools::BrowserImagePtr TabService::getThumb(const basic_webengine::TabId& tabId)
{
    auto imageCache = getThumbCache(tabId);
    if (imageCache) {
        return *imageCache;
    }

    auto imageDatabase = getThumbDatabase(tabId);
    if(imageDatabase) {
        saveThumbCache(tabId, *imageDatabase);
        return *imageDatabase;
    }

    BROWSER_LOGD("%s [%d] generating thumb", __FUNCTION__, tabId.get());
    generateThumb(tabId);

    imageCache = getThumbCache(tabId);
    if (imageCache)
        return *imageCache;
    else
        return std::make_shared<tools::BrowserImage>();
}

boost::optional<tools::BrowserImagePtr> TabService::getThumbCache(
        const basic_webengine::TabId& tabId)
{
    if (!thumbCached(tabId)) {
        return boost::none;
    }
    return m_thumbMap.find(tabId.get())->second;
}

void TabService::removeTab(const basic_webengine::TabId& tabId)
{
    BROWSER_LOGD("[%s:%d] tab id: %d", __PRETTY_FUNCTION__, __LINE__, tabId.get());
    clearFromDatabase(tabId);
    clearFromCache(tabId);
}

void TabService::fillThumbs(
        const std::vector<basic_webengine::TabContentPtr>& tabsContents)
{
    for (auto& tc : tabsContents) {
        auto thumbPtr = getThumb(tc->getId());
        tc->setThumbnail(thumbPtr);
    }
}

void TabService::updateTabItem(const basic_webengine::TabId& tabId,
        const std::string& url,
        const std::string& title,
        const basic_webengine::TabOrigin& origin)
{
    BROWSER_LOGD("[%s:%d] tabid: %d", __PRETTY_FUNCTION__, __LINE__, tabId.get());

    if (!thumbInDatabase(tabId))
        createTabId(tabId.get());
    if (bp_tab_adaptor_set_url(tabId.get(), url.c_str()) < 0) {
        errorPrint("bp_tab_adaptor_set_url");
    }
    if (bp_tab_adaptor_set_title(tabId.get(), title.c_str()) < 0) {
        errorPrint("bp_tab_adaptor_set_title");
    }
    if (bp_tab_adaptor_set_index(tabId.get(), origin.getValue()) < 0) {     // tab origin is int saved in index bp_tab parameter
        errorPrint("bp_tab_adaptor_set_index");
    }
}

void TabService::updateTabItemSnapshot(const basic_webengine::TabId& tabId,
        tools::BrowserImagePtr imagePtr)
{
    BROWSER_LOGD("[%s:%d] tabid: %d", __PRETTY_FUNCTION__, __LINE__, tabId.get());

    if (!thumbInDatabase(tabId))
        createTabId(tabId.get());
    saveThumbCache(tabId, imagePtr);
    saveThumbDatabase(tabId, imagePtr);
}

void TabService::saveThumbCache(const basic_webengine::TabId& tabId,
        tools::BrowserImagePtr imagePtr)
{
    m_thumbMap[tabId.get()] = imagePtr;
}

bool TabService::thumbCached(const basic_webengine::TabId& tabId) const
{
    return m_thumbMap.find(tabId.get()) != m_thumbMap.end();
}

void TabService::clearFromCache(const basic_webengine::TabId& tabId) {
    m_thumbMap.erase(tabId.get());
}

void TabService::clearAll()
{
    int* items = nullptr;
    int count;
    if (bp_tab_adaptor_get_full_ids_p(&items, &count) < 0) {
        errorPrint("bp_tab_adaptor_get_full_ids_p");
    }
    for (int i = 0; i < count; ++i) {
        auto id = basic_webengine::TabId(items[i]);
        clearFromDatabase(id);
    }
    m_thumbMap.clear();
}

void TabService::clearFromDatabase(const basic_webengine::TabId& tabId)
{
    if (bp_tab_adaptor_delete(tabId.get()) < 0) {
        errorPrint("bp_tab_adaptor_delete");
    }
    if (bp_tab_adaptor_set_deleted(tabId.get()) < 0) {
        errorPrint("bp_tab_adaptor_set_deleted");
    }
    if (bp_tab_adaptor_clear_deleted_ids() < 0) {
        errorPrint("bp_tab_adaptor_clear_deleted_ids");
    }
}

void TabService::saveThumbDatabase(const basic_webengine::TabId& tabId,
        tools::BrowserImagePtr imagePtr)
{
    BROWSER_LOGD("[%s:%d] tabId: %d", __PRETTY_FUNCTION__, __LINE__, tabId.get());
    std::unique_ptr<tools::Blob> thumb_blob = tools::EflTools::getBlobPNG(
            imagePtr);
    if (!thumb_blob){
        BROWSER_LOGW("getBlobPNG failed");
        return;
    }
    unsigned char* thumbData = std::move((unsigned char*) thumb_blob->getData());
    if (bp_tab_adaptor_set_snapshot(tabId.get(), imagePtr->getWidth(),
            imagePtr->getHeight(), thumbData, thumb_blob->getLength()) < 0) {
        errorPrint("bp_tab_adaptor_set_snapshot");
    }
}

bool TabService::thumbInDatabase(const basic_webengine::TabId& tabId) const
{
    char* url;
    int result = bp_tab_adaptor_get_url(tabId.get(), &url);
    if (result == BP_TAB_ERROR_ID_NOT_FOUND) {
        return false;
    } else if (result < 0) {
        errorPrint("bp_tab_adaptor_get_full_ids_p");
        return false;
    }

    return true;
}

boost::optional<tools::BrowserImagePtr> TabService::getThumbDatabase(
        const basic_webengine::TabId& tabId)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!thumbInDatabase(tabId)) {
        return boost::none;
    }

    int w = 0, h = 0, l = 0;
    unsigned char *v = nullptr;
    if (bp_tab_adaptor_get_snapshot(tabId.get(), &w, &h, &v, &l)) {
        errorPrint("bp_tab_adaptor_get_snapshot");
        return boost::none;
    }

    tools::BrowserImagePtr image = std::make_shared<tools::BrowserImage>(w, h, l);
    // TODO check if we can use shared memory here
    image->setData((void*)v, false, tools::ImageType::ImageTypePNG);

    if (image->getSize() <= 0) {
        BROWSER_LOGW("[%s:%d] empty image!", __PRETTY_FUNCTION__, __LINE__);
        return boost::none;
    }

    return image;
}

} /* namespace base_ui */
} /* namespace tizen_browser */
