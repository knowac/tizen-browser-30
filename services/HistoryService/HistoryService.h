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

#ifndef __HISTORY_SERVICE_H
#define __HISTORY_SERVICE_H

#include <vector>
#include <memory>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/signals2/signal.hpp>

#include "ServiceFactory.h"
#include "service_macros.h"
#include "BrowserImage.h"
#include "HistoryItemTypedef.h"
#include "StorageService.h"
#include <web/web_history.h>
#define DOMAIN_HISTORY_SERVICE "org.tizen.browser.historyservice"

namespace tizen_browser
{
namespace services
{

class BROWSER_EXPORT HistoryService: public tizen_browser::core::AbstractService
{
public:
    HistoryService();
    virtual ~HistoryService();
    virtual std::string getName();
    int getHistoryId(const std::string & url);
    void addHistoryItem(const std::string & url,
                        const std::string & title,
                        tools::BrowserImagePtr favicon);

    void updateHistoryItemFavicon(const std::string & url,
                                  tools::BrowserImagePtr favicon);
    void updateHistoryItemSnapshot(const std::string & url,
                                   tools::BrowserImagePtr snapshot);
    void clearAllHistory();
    void clearURLHistory(const std::string & url);
    void deleteHistoryItem(int id);
    void setMostVisitedFrequency(int id, int frequency);
    std::shared_ptr<HistoryItem> getHistoryItem(const std::string & url);
    std::shared_ptr<HistoryItemVector> getHistoryAll();
    std::shared_ptr<HistoryItemVector> getHistoryToday();
    std::shared_ptr<HistoryItemVector> getHistoryYesterday();
    std::shared_ptr<HistoryItemVector> getHistoryLastWeek();
    std::shared_ptr<HistoryItemVector> getHistoryLastMonth();
    std::shared_ptr<HistoryItemVector> getHistoryOlder();
    std::shared_ptr<HistoryItemVector> getMostVisitedHistoryItems();
    void cleanMostVisitedHistoryItems();
    std::shared_ptr<HistoryItemVector> getHistoryItemsByKeyword(const std::string & keyword, int maxItems);
    std::shared_ptr<HistoryItemVector> getHistoryItemsByURL(const std::string & url, int maxItems);
#if PWA
    int getHistoryCnt(const int& id);
#endif

    /**
     * @brief Searches for history items matching given pattern.
     *
     * Splits pattern into words by removing spaces. History item matches
     * pattern, when its url contains all words (order not considered).
     *
     * @param keywords string containing keywords separated by spaces
     * @param maxItems searched items number will be shortened to this maxItems.
     * if -1: no shortening.
     * @param minKeywordLength minimum length of the longest keyword picked,
     * from which searching will start. If longest keyword is shorter than
     * #minKeywordLength, then search will not start.
     * @param uniqueUrls true if return should contain items with unique url
     * @return vector of shared pointers to history items matching given
     * pattern
     */
    std::shared_ptr<HistoryItemVector> getHistoryItemsByKeywordsString(
            const std::string& keywordsString, const int maxItems,
            const unsigned int minKeywordLength, bool uniqueUrls = false);

    int getHistoryItemsCount();
    void setStorageServiceTestMode(bool testmode = true);

    boost::signals2::signal<void (bool)>historyEmpty;
    boost::signals2::signal<void (const std::string& uri)> historyDeleted;
    boost::signals2::signal<void ()> historyAllDeleted;

private:
    bool m_testDbMod;;
    std::vector<std::shared_ptr<HistoryItem>> history_list;
    std::shared_ptr<tizen_browser::services::StorageService> m_storageManager;

    /**
     * Help method printing last bp_history_error_defs error.
     */
    void errorPrint(std::string method) const;

    /**
     * @throws StorageExceptionInitialization on error
     */
    void initDatabaseBookmark(const std::string & db_str);

    std::shared_ptr<HistoryItem> getHistoryItem(int* ids, int idNumber = 0);
    std::shared_ptr<HistoryItemVector> getHistoryItems(bp_history_date_defs period = BP_HISTORY_DATE_TODAY);
    bool isDuplicate(const char* url) const;
};

}
}

#endif
