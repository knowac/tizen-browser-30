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

#ifndef __HISTORY_ITEM_H
#define __HISTORY_ITEM_H

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <memory>
#include <vector>

#include "BrowserImage.h"

namespace tizen_browser {
namespace services {

class HistoryItem {
public:
    HistoryItem(int id,
                const std::string & url,
                const std::string & title,
                tools::BrowserImagePtr favicon,
                tools::BrowserImagePtr thumbnail = std::make_shared<tools::BrowserImage>());
    HistoryItem(HistoryItem && other) noexcept;
    HistoryItem(int id, const std::string& url);
    HistoryItem(const HistoryItem& source);
    virtual ~HistoryItem();

    HistoryItem & operator=(HistoryItem && other) throw();

    /**
    * @brief compares two HisoryItems, only "url" are checked no other elements are checked.
    *
    * @return bool
    */
    bool operator==(const HistoryItem& other);
    bool operator!=(const HistoryItem& other);

    void setUrl(const std::string & url);
    std::string getUrl() const;

    void setTitle(const std::string & title);
    std::string getTitle() const;

    void setLastVisit(boost::posix_time::ptime visitDate);
    boost::posix_time::ptime getLastVisit() const;
    time_t getLastVisitAsTimeT() const;

    void setVisitCounter(int visitCounter);
    int getVisitCounter();

    void setThumbnail(tools::BrowserImagePtr thumbnail);
    tools::BrowserImagePtr getThumbnail() const;

    void setFavIcon(tools::BrowserImagePtr favIcon);
    tools::BrowserImagePtr getFavIcon();

    void setUriFavicon(const std::string & uri);
    std::string getUriFavicon();

    int getId() const { return m_primaryKey; };
private:
    int m_primaryKey;
    std::string m_url;
    std::string m_title;
    boost::gregorian::date m_visitDate;
    boost::posix_time::ptime m_lastVisit;
    tools::BrowserImagePtr m_thumbnail;
    tools::BrowserImagePtr m_favIcon;
    std::string m_urifavicon;
    int m_visitCounter;
};

}
}

#endif
