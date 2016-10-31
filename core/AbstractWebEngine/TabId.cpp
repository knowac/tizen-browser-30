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
 * TabId.cpp
 *
 *  Created on: Apr 11, 2014
 *      Author: p.rafalski
 */

#include "browser_config.h"
#include "BrowserImage.h"
#include "TabId.h"
#include "CapiWebErrorCodes.h"

namespace tizen_browser {
namespace basic_webengine {

TabId TabId::NONE(-1);

TabId::TabId(int id)
    : m_id(id)
{
}

TabId::TabId(const TabId & n)
{
    m_id = n.m_id;
}

TabId::~TabId()
{
}

std::string TabId::toString() const {
    return std::to_string(m_id);
}

TabContent::TabContent(
    TabId id,
    const std::string& url,
    const std::string& title,
    const TabOrigin& origin,
    tools::BrowserImagePtr thumbnail,
    tools::BrowserImagePtr favicon,
    bool isSecret)
    : m_id(id)
    , m_url(url)
    , m_title(title)
    , m_origin(origin)
    , m_thumbnail(thumbnail)
    , m_favicon(favicon)
    , m_isSecret(isSecret)
{
}

TabContent::TabContent(
    const TabId& id,
    const std::string& url,
    const std::string& title,
    const TabOrigin& origin,
    bool isSecret)
    : m_id(id)
    , m_url(url)
    , m_title(title)
    , m_origin(origin)
    , m_thumbnail(std::make_shared<tools::BrowserImage>())
    , m_favicon(std::make_shared<tools::BrowserImage>())
    , m_isSecret(isSecret)
{
}

TabId TabContent::getId() const
{
    return m_id;
}

std::string TabContent::getUrl() const
{
    return m_url;
}

std::string TabContent::getTitle() const
{
    return m_title;
}

TabOrigin TabContent::getOrigin() const
{
    return m_origin;
}

void TabContent::setThumbnail(tools::BrowserImagePtr thumbnail)
{
    m_thumbnail = thumbnail;
}

void TabContent::setFavicon(tools::BrowserImagePtr favicon)
{
    m_favicon = favicon;
}

tools::BrowserImagePtr TabContent::getThumbnail() const
{
    return m_thumbnail;
}

tools::BrowserImagePtr TabContent::getFavicon() const
{
    return m_favicon;
}

bool TabContent::getIsSecret() const
{
    return m_isSecret;
}

} /* end of basic_webengine */
} /* end of tizen_browser */
