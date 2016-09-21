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

#ifndef GENLISTMANAGER_H_
#define GENLISTMANAGER_H_

#include <Elementary.h>
#include "services/HistoryService/HistoryItemTypedef.h"
#include <boost/signals2/signal.hpp>

using namespace std;

namespace tizen_browser {
namespace base_ui {

class GenlistItemsManager;
enum class GenlistItemType;
typedef shared_ptr<GenlistItemsManager> GenlistItemsManagerPtr;
class UrlMatchesStyler;
typedef shared_ptr<UrlMatchesStyler> UrlMatchesStylerPtr;

typedef struct UrlPair_s
{
    UrlPair_s(string a, string b) :
            urlOriginal(a), urlHighlighted(b)
    {
    }
    string urlOriginal;
    /**
     * Url plus styling tags.
     */
    string urlHighlighted;
} UrlPair;

class GenlistManager
{
public:
    GenlistManager();
    ~GenlistManager();
    void setParentLayout(Evas_Object* parentLayout);
    Evas_Object* getGenlist();
    GenlistItemsManagerPtr getItemsManager();
    void show(const string& editedUrl,
            shared_ptr<services::HistoryItemVector> matchedEntries);
    void hide();

    /**
     * Clear genlist elements, delete genlist.
     */
    void clear();

    /**
     * Get url from item of a given type.
     * @param types The types of list items: url will be searched in these item types.
     * @return Url from the first item from the list, which has valid url. Empty if neither of items has url assigned.
     */
    string getItemUrl(std::initializer_list<GenlistItemType> types) const;
    boost::signals2::signal<void(string)> signalItemSelected;
    boost::signals2::signal<void()> signalItemFocusChange;

    /// sent to UrlHistoryList.
    boost::signals2::signal<void(Evas_Object*)> signalGenlistCreated;
private:
    Evas_Object* createGenlist(Evas_Object* parentLayout);
    static Evas_Object* m_itemClassContentGet(void *data, Evas_Object *obj,
            const char *part);
    void prepareUrlsVector(const string& editedUrl,
            shared_ptr<services::HistoryItemVector> matchedEntries);

    Evas_Object* m_parentLayout;
    Evas_Object* m_genlist;

    // don't know how to get from edc:
    const int ITEM_H;
    const int ITEMS_VISIBLE_NUMBER_MAX;
    // currently visible items number
    int m_historyItemsVisibleCurrent;

    Elm_Gengrid_Item_Class* m_historyItemClass;
    GenlistItemsManagerPtr m_itemsManager;

    /*
     * keeps shared pointers to strings, which are ready to be displayed, so they can be
     * passed through EFL, until they're not needed. IMPORTANT: it has to be
     * assured, that list is not cleared until all EFL items has created their
     * labels from these pointers in m_contentGet(). in case of segfaults, delete copy of pointers
     * manually in m_contentGet().
     */
    vector<shared_ptr<UrlPair>> m_readyUrlPairs;
    UrlMatchesStylerPtr m_urlMatchesStyler;
};

} /* namespace base_ui */
} /* namespace tizen_browser */

#endif /* GENLISTMANAGER_H_ */
