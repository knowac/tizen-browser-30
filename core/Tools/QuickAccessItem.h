/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
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

#ifndef QUICKACCESSITEM_H
#define QUICKACCESSITEM_H

#include "BrowserLogger.h"
#include "BrowserImage.h"

namespace tizen_browser{
namespace services{

class QuickAccessItem
{
public:
    QuickAccessItem();
    QuickAccessItem(
        int id,
        const std::string& url = "",
        const std::string& title = "",
        int color = 0,
        int order = 0,
        bool hasFavicon = false
        );

    void setId(int id) { m_id = id; }
    int getId() const { return m_id; }

    void setUrl(const std::string & url) { m_url = url; }
    std::string getUrl() const { return m_url; }

    void setTitle(const std::string & title) { m_title = title; }
    std::string getTitle() const { return m_title; }

    void setColor(int color) { m_color = color; }
    int getColor() const { return m_color; }

    void setOrder(int order) { m_order = order; }
    int getOrder() const { return m_order; }

    void setFavicon(tools::BrowserImagePtr favicon);
    tools::BrowserImagePtr getFavicon() const;

    bool has_favicon() const { return m_has_favicon; }

private:
    int m_id;
    std::string m_url;
    std::string m_title;
    int m_color;
    int m_order;
    bool m_has_favicon;
    tools::BrowserImagePtr m_favicon;
};

using SharedQuickAccessItem = std::shared_ptr<QuickAccessItem>;
using SharedQuickAccessItemVector = std::vector<SharedQuickAccessItem>;

}//end namespace storage
}//end namespace tizen_browser

#endif // QUICKACCESSITEM_H
