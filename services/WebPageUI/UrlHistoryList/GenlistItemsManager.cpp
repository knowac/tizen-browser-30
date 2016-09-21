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

#include "GenlistItemsManager.h"
#include "BrowserLogger.h"

namespace tizen_browser {
namespace base_ui {

GenlistItemsManager::GenlistItemsManager()
{
    ptrMap = {
        {   GenlistItemType::ITEM_CURRENT,
            make_shared<Elm_Object_Item*>()},
        {   GenlistItemType::ITEM_FIRST,
            make_shared<Elm_Object_Item*>()},
        {   GenlistItemType::ITEM_LAST,
            make_shared<Elm_Object_Item*>()}
    };
    namesMap = {
            { GenlistItemType::ITEM_CURRENT, "ITEM_CURRENT" },
            { GenlistItemType::ITEM_FIRST, "ITEM_FIRST" },
            { GenlistItemType::ITEM_LAST, "ITEM_LAST" }
    };
}

GenlistItemsManager::~GenlistItemsManager()
{
}

Elm_Object_Item* GenlistItemsManager::getItem(GenlistItemType type) const
{
    return *ptrMap.at(type);
}

void GenlistItemsManager::setItems(std::initializer_list<GenlistItemType> types,
        Elm_Object_Item* item)
{
    for (auto i : types) {
        *ptrMap.at(i) = item;
    }
}

void GenlistItemsManager::setItemsIfNullptr(
        std::initializer_list<GenlistItemType> types, Elm_Object_Item* item)
{
    for (auto i : types) {
        if (!getItem(i)) {
            setItems( { i }, item);
        }
    }
}

void GenlistItemsManager::assignItem(GenlistItemType dst, GenlistItemType src)
{
    setItems( { dst }, getItem(src));
}

bool GenlistItemsManager::shiftItemDown(GenlistItemType item)
{
    if (!getItem(item))
        return false;
    Elm_Object_Item* item_next = elm_genlist_item_next_get(getItem(item));
    if (item_next) {
        setItems( { item }, item_next);
        return true;
    }
    return false;
}

bool GenlistItemsManager::shiftItemUp(GenlistItemType item)
{
    if (!getItem(item))
        return false;
    Elm_Object_Item* item_prev = elm_genlist_item_prev_get(getItem(item));
    if (item_prev) {
        setItems( { item }, item_prev);
        return true;
    }
    return false;
}

void GenlistItemsManager::clear()
{
    for (auto pair : ptrMap) {
        setItems({pair.first}, nullptr);
    }
}

std::string GenlistItemsManager::toString(GenlistItemType item) const
{
    return namesMap.find(item)->second;
}

} /* namespace base_ui */
} /* namespace tizen_browser */
