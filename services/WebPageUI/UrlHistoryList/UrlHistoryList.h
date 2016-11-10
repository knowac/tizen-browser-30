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

#ifndef URLHISTORYLIST_H_
#define URLHISTORYLIST_H_

#include <memory>
#include <Evas.h>

#include "services/HistoryService/HistoryItemTypedef.h"
#include <boost/signals2/signal.hpp>
#include "EcoreTimerCaller.h"
#include "TimerCallbacks.h"

using namespace std;

namespace tizen_browser {
namespace base_ui {

class WebPageUIStatesManager;
typedef std::shared_ptr<const WebPageUIStatesManager> WPUStatesManagerPtrConst;
class GenlistManager;
typedef shared_ptr<GenlistManager> GenlistManagerPtr;

/**
 * Manages list of url matches (URL from history). Manages top layout, creates
 * widget displaying url items.
 */
class UrlHistoryList
{
public:
    UrlHistoryList(WPUStatesManagerPtrConst webPageUiStatesMgr);
    virtual ~UrlHistoryList();
    void setMembers(Evas_Object* parent, Evas_Object* chainObject);
    Evas_Object* getLayout();
    bool getGenlistVisible();

    /**
     * On uri entry widget "changed,user" signal.
     *
     * @param matchedEntries The entries matches for editedUrl
     */
    void onURLEntryEditedByUser(const string& editedUrl,
            shared_ptr<services::HistoryItemVector> matchedEntries);
    void hideWidget();
    void onBackPressed();

    /**
     * @return True if widget is focused.
     */
    bool getWidgetFocused() const;
    void onListWidgetFocusChange(bool focused);
    void listWidgetFocusedFromUri();
    void saveEntryAsEditedContent();
    void saveEntryAsURLContent();
    void restoreEntryEditedContent();
    void restoreEntryURLContent();
    int getItemsNumberMax() const;
    int getKeywordLengthMin() const;
    boost::signals2::signal<void (const std::string&)> openURL;
    boost::signals2::signal<void (const std::string&)> uriChanged;

private:
    void createLayout(Evas_Object* parentLayout);
    void onItemSelect(std::string content);

    /**
     * On genlist's item focus change.
     */
    void onItemFocusChange();

    /**
     * Main layout is not setting genlist as a swallow in the beginnging,
     * because it will cover view in WebPageUi.
     */
    void onGenlistCreated(Evas_Object*);
    static void _uri_entry_editing_changed_user(void* data, Evas_Object* obj, void* event_info);

    GenlistManagerPtr m_genlistManager;
    WPUStatesManagerPtrConst m_webPageUiStatesMgr;
    // the maximum items number on a list
    const int ITEMS_NUMBER_MAX;
    // the minimum length of the keyword used to search matches
    const int KEYWORD_LENGTH_MIN;
    Evas_Object* m_parent;
    // entry widget from which change signals are received
    Evas_Object* m_entry;
    Evas_Object* m_layout;
    string m_edjFilePath;
    // content of the edited entry, needed to restore edited value
    string m_entryEditedContent;
    // content of the entry before edition: needed to restore original URL value
    string m_entryURLContent;
    bool m_widgetFocused;
    GenlistFocused m_genlistFocusedCallback;
    tools::EflTools::EcoreTimerCaller<GenlistFocused> m_timerGenlistFocused;
};

} /* namespace base_ui */
} /* namespace tizen_browser */

#endif /* URLHISTORYLIST_H_ */
