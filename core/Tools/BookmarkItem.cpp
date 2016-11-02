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

#include "browser_config.h"
#include "BookmarkItem.h"

#include <string>
#include <Evas.h>

namespace tizen_browser{
namespace services{

BookmarkItem::BookmarkItem()
: m_saved_id(0)
, m_url()
, m_title()
, m_note()
, m_parent(0)
, m_order(0)
, m_has_thumbnail(false)
, m_has_favicon(false)
, m_is_folder(false)
, m_is_editable(true)
{
}

BookmarkItem::BookmarkItem(
                int id,
                const std::string& url,
                const std::string& title,
                const std::string& note,
                int parent,
                int order
                        )
: m_saved_id(id)
, m_url(url)
, m_title(title)
, m_note(note)
, m_parent(parent)
, m_order(order)
, m_has_thumbnail(false)
, m_has_favicon(false)
{

}

BookmarkItem::~BookmarkItem()
{
}

void BookmarkItem::setFavicon(std::shared_ptr<tizen_browser::tools::BrowserImage> fav)
{
    m_has_favicon = true;
    m_favicon = fav;
};

std::shared_ptr<tizen_browser::tools::BrowserImage> BookmarkItem::getFavicon() const
{
    return m_favicon;
};

void BookmarkItem::setThumbnail(std::shared_ptr<tizen_browser::tools::BrowserImage> thumbnail)
{
    m_has_thumbnail = true;
    m_thumbnail = thumbnail;
};

std::shared_ptr<tizen_browser::tools::BrowserImage> BookmarkItem::getThumbnail() const
{
    return m_thumbnail;
};

}
}
