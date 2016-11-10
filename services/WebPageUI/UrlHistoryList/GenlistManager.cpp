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

#include "services/HistoryService/HistoryItem.h"
#include "GenlistManager.h"
#include "GenlistManagerCallbacks.h"
#include "UrlMatchesStyler.h"
#include "GenlistItemsManager.h"
#include "Config.h"
#include <EflTools.h>
#include <Edje.h>

// Right now base scale in edc is 2.6, hence using 2.6 here to calculate proper
// ITEM_H value. Probably assumptions will change and ITEM_H will have to be
// calculated in a different way.
#define ELM_SCALE_SIZE_Z3(x) (int)(((double)(x) * elm_config_scale_get()) / 2.6)

namespace tizen_browser {
namespace base_ui {

GenlistManager::GenlistManager()
    : m_parentLayout(nullptr)
    , m_genlist(nullptr)
#if PROFILE_MOBILE
    , ITEM_H(ELM_SCALE_SIZE_Z3(boost::any_cast<int>(tizen_browser::config::Config::
            getInstance().get(CONFIG_KEY::URLHISTORYLIST_ITEM_HEIGHT))))
#else
    , ITEM_H(ELM_SCALE_SIZE(boost::any_cast<int>(tizen_browser::config::Config::
        getInstance().get(CONFIG_KEY::URLHISTORYLIST_ITEM_HEIGHT))))
#endif
    , ITEMS_VISIBLE_NUMBER_MAX(boost::any_cast<int>(tizen_browser::config::Config::
            getInstance().get(CONFIG_KEY::URLHISTORYLIST_ITEMS_VISIBLE_NUMBER_MAX)))
    , m_historyItemsVisibleCurrent(0)
    , m_historyItemClass(nullptr)
{
    m_urlMatchesStyler = make_shared<UrlMatchesStyler>();
    m_itemsManager = make_shared<GenlistItemsManager>();
    m_historyItemClass = elm_genlist_item_class_new();
    m_historyItemClass->item_style = "url_historylist_grid_item";
    m_historyItemClass->func.text_get = nullptr;
    m_historyItemClass->func.content_get = m_itemClassContentGet;
    m_historyItemClass->func.state_get = nullptr;
    m_historyItemClass->func.del = nullptr;
    GenlistManagerCallbacks::setGenlistManager(this);
    m_historyItemsVisibleCurrent = ITEMS_VISIBLE_NUMBER_MAX;
}

GenlistManager::~GenlistManager()
{
    elm_genlist_item_class_free(m_historyItemClass);
}

void GenlistManager::setParentLayout(Evas_Object* parentLayout)
{
    m_parentLayout = parentLayout;
}

Evas_Object* GenlistManager::getGenlist()
{
    if (!m_genlist)
        m_genlist = createGenlist(m_parentLayout);
    return m_genlist;
}

GenlistItemsManagerPtr GenlistManager::getItemsManager()
{
    return m_itemsManager;
}

void GenlistManager::show(const string& editedUrl,
        shared_ptr<services::HistoryItemVector> matchedEntries)
{
    clear();
    m_genlist = createGenlist(m_parentLayout);

    prepareUrlsVector(editedUrl, matchedEntries);

    m_itemsManager->setItems( { GenlistItemType::ITEM_FIRST,
            GenlistItemType::ITEM_LAST }, nullptr);
    Elm_Object_Item* itemAppended = nullptr;
    for (auto it : m_readyUrlPairs) {
        itemAppended = elm_genlist_item_append(m_genlist, m_historyItemClass,
                it.get(), nullptr, ELM_GENLIST_ITEM_NONE,
                GenlistManagerCallbacks::_item_selected, it.get());
        m_itemsManager->setItemsIfNullptr( { GenlistItemType::ITEM_FIRST },
                itemAppended);
    }
    m_itemsManager->setItems( { GenlistItemType::ITEM_LAST }, itemAppended);

    const int list_items_number = elm_genlist_items_count(m_genlist);
    if (list_items_number != 0) {
        Edje_Message_Int message;
        if (list_items_number > ITEMS_VISIBLE_NUMBER_MAX)
            message.val = ITEMS_VISIBLE_NUMBER_MAX;
        else
            message.val = list_items_number;
        message.val *= ITEM_H;
        edje_object_message_send(elm_layout_edje_get(m_parentLayout), EDJE_MESSAGE_INT, 0, &message);
    }

    evas_object_show(m_genlist);
}

void GenlistManager::hide()
{
    evas_object_hide(m_genlist);
    clear();
}

void GenlistManager::clear()
{
    if (m_genlist && elm_genlist_items_count(m_genlist)) {
        elm_genlist_clear(m_genlist);
        evas_object_del(m_genlist);
        m_genlist = nullptr;
        m_itemsManager->clear();
    }
}

string GenlistManager::getItemUrl(
        std::initializer_list<GenlistItemType> types) const
{
    for (auto t : types) {
        if (!m_itemsManager->getItem( { t }))
            continue;
        void* data = elm_object_item_data_get(m_itemsManager->getItem( { t }));
        if (!data)
            continue;
        const UrlPair* const urlPair = reinterpret_cast<UrlPair*>(data);
        if (!urlPair)
            continue;
        return urlPair->urlOriginal;
    }
    return "";
}

Evas_Object* GenlistManager::createGenlist(Evas_Object* parentLayout)
{
    Evas_Object* genlist = elm_genlist_add(parentLayout);
    evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND,
    0.0);
    evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL,
    0.0);
    elm_scroller_bounce_set(genlist, EINA_FALSE, EINA_FALSE);
    elm_scroller_movement_block_set(genlist,
            ELM_SCROLLER_MOVEMENT_BLOCK_HORIZONTAL);
    elm_scroller_policy_set(genlist, ELM_SCROLLER_POLICY_OFF,
            ELM_SCROLLER_POLICY_OFF);
    elm_object_event_callback_add(genlist,
            GenlistManagerCallbacks::_object_event, this);
    signalGenlistCreated(genlist);
    return genlist;
}

Evas_Object* GenlistManager::m_itemClassContentGet(void* data, Evas_Object* obj,
        const char* part)
{
    Evas_Object* layout = elm_layout_add(obj);
    tools::EflTools::setExpandHints(layout);
    if (strcmp(part, "matched_url") == 0) {
        const UrlPair* const item = reinterpret_cast<UrlPair*>(data);
        if (item) {
            string edjFilePath = EDJE_DIR;
            edjFilePath.append("WebPageUI/UrlHistoryList.edj");
            elm_layout_file_set(layout, edjFilePath.c_str(),
                    "layoutMatchedUrl");
            elm_object_text_set(layout, item->urlHighlighted.c_str());
        }
    }
    return layout;
}

void GenlistManager::prepareUrlsVector(const string& editedUrl,
        shared_ptr<services::HistoryItemVector> matchedEntries)
{
    // free previously used urls. IMPORTANT: it has to be assured that previous
    // genlist items are not using these pointers.
    m_readyUrlPairs.clear();
    for (auto it : *matchedEntries) {
        UrlPair newUrlPair(it->getUrl(),
                m_urlMatchesStyler->getUrlHighlightedMatches(it->getUrl(),
                        editedUrl));
        m_readyUrlPairs.push_back(make_shared < UrlPair > (newUrlPair));
    }
}

} /* namespace base_ui */
} /* namespace tizen_browser */
