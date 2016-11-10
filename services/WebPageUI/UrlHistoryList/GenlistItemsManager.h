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

#ifndef GENLISTITEMSMANAGER_H_
#define GENLISTITEMSMANAGER_H_

#include <memory>
#include <map>

#include <Elementary.h>

using namespace std;

namespace tizen_browser {
namespace base_ui {

enum class GenlistItemType
{
    ITEM_CURRENT, ITEM_FIRST, ITEM_LAST
};

/**
 * Stores and manipulated pointers on Elm_Object_Item for GenlistManager
 */
class GenlistItemsManager
{
public:
    GenlistItemsManager();
    virtual ~GenlistItemsManager();

    Elm_Object_Item* getItem(GenlistItemType type) const;
    void setItems(std::initializer_list<GenlistItemType> types,
            Elm_Object_Item* item);
    /**
     * Same as #setItems, except only nullptr value pointers are set
     */
    void setItemsIfNullptr(std::initializer_list<GenlistItemType> types,
            Elm_Object_Item* item);
    /**
     * Assign src pointer value to dst.
     */
    void assignItem(GenlistItemType dst, GenlistItemType src);
    /**
     * Assign item of a given type to a elm_genlist_item_next_get item, if
     * there is one. Return false, if value has not changed.
     */
    bool shiftItemDown(GenlistItemType item);
    bool shiftItemUp(GenlistItemType item);

    /**
     * clear all pointers
     */
    void clear();

    std::string toString(GenlistItemType item) const;


private:
    map<GenlistItemType, shared_ptr<Elm_Object_Item*>> ptrMap;
    std::map<GenlistItemType, std::string> namesMap;

};

} /* namespace base_ui */
} /* namespace tizen_browser */

#endif /* GENLISTITEMSMANAGER_H_ */
