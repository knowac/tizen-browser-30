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

#ifndef HISTORYDAYSLISTMANAGERMOB_H_
#define HISTORYDAYSLISTMANAGERMOB_H_

#include <memory>
#include <Elementary.h>
#include <string>
#include <vector>
#include <set>
#include "HistoryDayItemDataTypedef.h"
#include "HistoryDaysListManager.h"
#include "HistoryDaysListManagerEdje.h"

namespace tizen_browser {
namespace base_ui {

class HistoryDayItemMob;
using HistoryDayItemMobPtr = std::shared_ptr<HistoryDayItemMob>;

class HistoryDaysListManagerMob : public HistoryDaysListManager
{
public:
    HistoryDaysListManagerMob();
    virtual ~HistoryDaysListManagerMob();

    void createGenlistItemClasses();

    static char* _genlist_history_day_text_get(void *data, Evas_Object *, const char *part);
    static char* _genlist_history_download_text_get(void* data, Evas_Object*, const char *part);
    static char* _genlist_history_item_text_get(void *data, Evas_Object *, const char *part);
    static void _check_state_changed(void*, Evas_Object*, void *);
    static void _tree_item_expanded(void*, Evas_Object*, void*);
    static void _tree_item_contracted(void*, Evas_Object*, void*);
    static void _tree_item_pressed(void*, Evas_Object*, void*);
    static void _item_selected(void *data, Evas_Object *obj, void *event_info);
    static Evas_Object* _genlist_history_download_content_get(void*, Evas_Object* obj, const char *part);
    static Evas_Object* _genlist_history_item_content_get(void *data, Evas_Object *, const char *part);
    static Evas_Object* _genlist_history_day_content_get(void *data, Evas_Object* obj, const char *part);

    Evas_Object* createDaysList(Evas_Object* parent, bool isRemoveMode = false) override;
    void addHistoryItems(
        const std::shared_ptr<services::HistoryItemVector>& items,
        HistoryPeriod period) override;
    void clear() override;
    void setFocusChain(Evas_Object* /*obj*/) override {}

    void onWebsiteHistoryItemVisitItemClicked(
        const WebsiteVisitItemDataPtrConst websiteVisitItemData,
        bool remove);
    void countItemsToDelete();
    void selectAllCheckboxes();
    void removeSelectedItems();
    bool isSelectAllChecked() const { return m_isSelectAllChecked == EINA_TRUE; }

    struct ItemData {
        HistoryDaysListManagerMob* self;
        WebsiteVisitItemDataPtr websiteVisitItem;
        WebsiteHistoryItemDataPtr websiteHistoryItemData;
        const char* str;
    };

private:
    void connectSignals();
    void appendDayItem(HistoryDayItemDataPtr dayItemData);
    void showNoHistoryMessage(bool show);
    bool isHistoryDayListEmpty() {return m_dayItems.empty();}

    /**
     * @brief remove item from view and from vector
     */
    void removeItem(HistoryDayItemDataPtrConst historyDayItemData);

    /**
     * @brief remove item from view and from vector
     */
    void removeItem(WebsiteHistoryItemDataPtrConst websiteHistoryItemData);

    /**
     * @brief remove item from view and from vector
     */
    void removeItem(WebsiteVisitItemDataPtrConst websiteVisitItemData);

    /**
     * @brief remove history day item from vector
     */
    void remove(HistoryDayItemMobPtr historyDayItem);

    HistoryDayItemMobPtr getItem(HistoryDayItemDataPtrConst historyDayItemData);

    HistoryDaysListManagerEdjePtr m_edjeFiles;
    std::vector<HistoryDayItemMobPtr> m_dayItems;

    Evas_Object* m_parent;
    Evas_Object* m_scrollerDays;
    Evas_Object* m_layoutScrollerDays;
    Evas_Object* m_boxDays;
    Elm_Genlist_Item_Class* m_history_day_item_class;
    Elm_Genlist_Item_Class* m_history_item_item_class;
    Elm_Genlist_Item_Class* m_history_download_item_class;
    Evas_Object* m_genlist;
    std::map<Elm_Object_Item*, HistoryDayItemDataPtr> m_itemData;
    std::map<Elm_Object_Item*, Eina_Bool> m_expandedState;
    std::map<Elm_Object_Item*, Eina_Bool> m_itemsToDelete;
    std::map<Elm_Object_Item*, WebsiteVisitItemDataPtr> m_visitItemData;
    bool m_isRemoveMode;
    std::vector<ItemData*> m_itemDataVector;
    size_t m_delete_count;
    size_t m_history_count;
    Eina_Bool m_isSelectAllChecked;
    Elm_Object_Item* m_downloadManagerItem;
    Elm_Object_Item* m_selectAllItem;
};

} /* namespace base_ui */
} /* namespace tizen_browser */

#endif /* HISTORYDAYSLISTMANAGERMOB_H_ */
