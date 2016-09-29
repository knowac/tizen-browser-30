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

#ifndef TABSERVICE_H_
#define TABSERVICE_H_

#define DOMAIN_TAB_SERVICE "org.tizen.browser.tabservice"

#include "ServiceFactory.h"
#include "service_macros.h"
#include <memory>
#include <map>
#include <boost/signals2/signal.hpp>
#include <boost/optional.hpp>
#include <web/web_tab.h>
#include "TabIdTypedef.h"
#include "BrowserImageTypedef.h"
#include "AbstractWebEngine/TabOrigin.h"

namespace tizen_browser {
namespace services {

/**
 * Manages tab data by using web_tab.h API.
 */
class BROWSER_EXPORT TabService: public tizen_browser::core::AbstractService
{
public:
    TabService();
    virtual ~TabService();
    virtual std::string getName();

    /**
     * @brief Insert or update item related to tab of given id.
     * @param tabId Id of the tab from web engine
     * @param url URL of the tab
     * @param title Ttle of the tab
     */
    void updateTabItem(const basic_webengine::TabId& tabId,
            const std::string& url,
            const std::string& title,
            const basic_webengine::TabOrigin& origin);

    /**
     * @brief Get all tab in vector.
     * @return Shared pointer to vector of tabs (TabContents)
     */
    std::shared_ptr<std::vector<basic_webengine::TabContent> > getAllTabs();

    /**
     * Remove image thumb for given id from the cache and database.
     */
    void removeTab(const basic_webengine::TabId& tabId);

    /**
     * Remove all items from the cache and database.
     */
    void clearAll();

    /**
     * Set thumb images for given TabContent objects: get them from
     * cache or database or generate them by taking screenshots.
     */
    void fillThumbs(
            const std::vector<basic_webengine::TabContentPtr>& tabsContents);

    /**
     * Invoke bp_tab_adaptor_create.
     *
     * @param tabId If -1, new id will be created. Otherwise entry in database
     * will have given id.
     * @return id The id created by bp_tab_adaptor_create.
     */
    int createTabId(int tabId = -1) const;

    /**
     * Convert tab id (string) to int.
     *
     * @return boost::none if string cannot be converted
     */
    boost::optional<int> convertTabId(std::string tabId) const;

    boost::signals2::signal<void(basic_webengine::TabId)> generateThumb;

    void updateTabItemSnapshot(const basic_webengine::TabId& tabId,
            tools::BrowserImagePtr imagePtr);

private:
    /**
     * Help method printing last bp_tab_error_defs error.
     */
    void errorPrint(std::string method) const;

    /**
     * Get image thumb for given id (from cache or database).
     * Create one, if it does not exist.
     */
    tools::BrowserImagePtr getThumb(const basic_webengine::TabId& tabId);

    /**
     * Get cached thumb for given tab id.
     *
     * @return Image or boost::none.
     */
    boost::optional<tools::BrowserImagePtr> getThumbCache(
            const basic_webengine::TabId& tabId);
    /**
     * Cache given thumb image with given tab id.
     */
    void saveThumbCache(const basic_webengine::TabId& tabId,
            tools::BrowserImagePtr imagePtr);
    /**
     * Check if thumb for given id is in a map.
     */
    bool thumbCached(const basic_webengine::TabId& tabId) const;
    /**
     * Remove image from cache for given tab id.
     */
    void clearFromCache(const basic_webengine::TabId& tabId);

    /**
     * Get thumb from database for given tab id.
     *
     * @return Image or boost::none.
     */
    boost::optional<tools::BrowserImagePtr> getThumbDatabase(
            const basic_webengine::TabId& tabId);
    /**
     * Save given thumb image with given tab id in a database.
     */
    void saveThumbDatabase(const basic_webengine::TabId& tabId,
            tools::BrowserImagePtr imagePtr);
    /**
     * Check if thumb for given id is in a database.
     */
    bool thumbInDatabase(const basic_webengine::TabId& tabId) const;
    /**
     * Remove image from a database for given tab id.
     *
     * @param ID created earlier by bp_tab_adaptor_create()
     */
    void clearFromDatabase(const basic_webengine::TabId& tabId);

    /**
     * Map caching images. Keys: tab ids, values: thumb images.
     */
    std::map<int, tools::BrowserImagePtr> m_thumbMap;
};

} /* namespace base_ui */
} /* namespace tizen_browser */

#endif /* TABSERVICE_H_ */
