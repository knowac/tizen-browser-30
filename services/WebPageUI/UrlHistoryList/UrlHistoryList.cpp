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

#include <Elementary.h>
#include <ecore-1/Ecore.h>
#include "UrlHistoryList.h"
#include "GenlistManager.h"
#include "BrowserLogger.h"
#include "GenlistItemsManager.h"
#include "WebPageUI/WebPageUIStatesManager.h"
#include "Config.h"
#include <EflTools.h>

namespace tizen_browser {
namespace base_ui {

UrlHistoryList::UrlHistoryList(WPUStatesManagerPtrConst webPageUiStatesMgr)
    : m_genlistManager(make_shared<GenlistManager>())
    , m_webPageUiStatesMgr(webPageUiStatesMgr)
    , ITEMS_NUMBER_MAX(boost::any_cast<int>(config::Config::
            getInstance().get(CONFIG_KEY::URLHISTORYLIST_ITEMS_NUMBER_MAX)))
    , KEYWORD_LENGTH_MIN(boost::any_cast<int>(tizen_browser::config::Config::
            getInstance().get(CONFIG_KEY::URLHISTORYLIST_KEYWORD_LENGTH_MIN)))
    , m_parent(nullptr)
    , m_entry(nullptr)
    , m_layout(nullptr)
    , m_widgetFocused(false)
{
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("WebPageUI/UrlHistoryList.edj");
    m_genlistManager->signalItemSelected.connect(
            boost::bind(&UrlHistoryList::onItemSelect, this, _1));
    m_genlistManager->signalItemFocusChange.connect(
            boost::bind(&UrlHistoryList::onItemFocusChange, this));
    m_genlistManager->signalGenlistCreated.connect(
            boost::bind(&UrlHistoryList::onGenlistCreated, this, _1));
    m_genlistFocusedCallback.set(this);
}

UrlHistoryList::~UrlHistoryList()
{
}

void UrlHistoryList::setMembers(Evas_Object* parent, Evas_Object* editedEntry)
{
    m_parent = parent;
    m_entry = editedEntry;
    evas_object_smart_callback_add(m_entry, "changed,user",
            UrlHistoryList::_uri_entry_editing_changed_user, this);
}

void UrlHistoryList::createLayout(Evas_Object* parentLayout)
{
    m_layout = elm_layout_add(parentLayout);
    tools::EflTools::setExpandHints(m_layout);
    elm_layout_file_set(m_layout, m_edjFilePath.c_str(), "url_history_list");
    m_genlistManager->setParentLayout(m_layout);
}

Evas_Object* UrlHistoryList::getLayout()
{
    if (!m_layout)
        createLayout(m_parent);
    return m_layout;
}

void UrlHistoryList::saveEntryAsEditedContent()
{
    m_entryEditedContent = elm_entry_entry_get(m_entry);
}

void UrlHistoryList::restoreEntryEditedContent()
{
    elm_entry_entry_set(m_entry, m_entryEditedContent.c_str());
    elm_entry_cursor_end_set(m_entry);
}

int UrlHistoryList::getItemsNumberMax() const
{
    return ITEMS_NUMBER_MAX;
}

int UrlHistoryList::getKeywordLengthMin() const
{
    return KEYWORD_LENGTH_MIN;
}

bool UrlHistoryList::getGenlistVisible()
{
    return evas_object_visible_get(m_genlistManager->getGenlist());
}

void UrlHistoryList::hideWidget()
{
    m_genlistManager->hide();
}

void UrlHistoryList::onBackPressed()
{
    hideWidget();
}

bool UrlHistoryList::getWidgetFocused() const
{
    return m_widgetFocused;
}

void UrlHistoryList::onURLEntryEditedByUser(const string& editedUrl,
        shared_ptr<services::HistoryItemVector> matchedEntries)
{
    if (matchedEntries->size() == 0) {
        hideWidget();
    } else {
        m_genlistManager->show(editedUrl, matchedEntries);
        evas_object_show(m_layout);
    }
}

void UrlHistoryList::onItemFocusChange()
{
    elm_entry_entry_set(m_entry, m_genlistManager->getItemUrl( {
            GenlistItemType::ITEM_CURRENT }).c_str());
}

void UrlHistoryList::onGenlistCreated(Evas_Object* genlist)
{
    elm_object_part_content_set(m_layout, "list_swallow", genlist);
    evas_object_show(m_layout);
}

void UrlHistoryList::onItemSelect(std::string content)
{
    if (m_webPageUiStatesMgr->equals(WPUState::QUICK_ACCESS)) {
        openURL(content);
    } else {
        uriChanged(content);
    }
}

void UrlHistoryList::onListWidgetFocusChange(bool focused)
{
    m_widgetFocused = focused;
    if (focused) {
        string itemUrl = m_genlistManager->getItemUrl( {
                GenlistItemType::ITEM_CURRENT, GenlistItemType::ITEM_FIRST });
        elm_entry_entry_set(m_entry, itemUrl.c_str());
    } else {
        restoreEntryEditedContent();
    }
}

void UrlHistoryList::listWidgetFocusedFromUri()
{
    m_widgetFocused = true;
    m_timerGenlistFocused.addTimer(m_genlistFocusedCallback);
}

void UrlHistoryList::_uri_entry_editing_changed_user(void* data,
        Evas_Object* /* obj */, void* /* event_info */)
{
    UrlHistoryList* self = reinterpret_cast<UrlHistoryList*>(data);
    self->saveEntryAsEditedContent();
}

}/* namespace base_ui */
} /* namespace tizen_browser */
