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

#ifndef CONFIGKEY_H_
#define CONFIGKEY_H_

enum class CONFIG_KEY
{
    HISTORY_TAB_SERVICE_THUMB_HEIGHT,
    HISTORY_TAB_SERVICE_THUMB_WIDTH,
    FAVORITESERVICE_THUMB_WIDTH,
    FAVORITESERVICE_THUMB_HEIGHT,
    URLHISTORYLIST_ITEMS_NUMBER_MAX,
    URLHISTORYLIST_ITEMS_VISIBLE_NUMBER_MAX,
    URLHISTORYLIST_KEYWORD_LENGTH_MIN,
    URLHISTORYLIST_ITEM_HEIGHT,
    WEB_ENGINE_PAGE_OVERVIEW,
    WEB_ENGINE_LOAD_IMAGES,
    WEB_ENGINE_ENABLE_JAVASCRIPT,
    WEB_ENGINE_REMEMBER_FROM_DATA,
    WEB_ENGINE_REMEMBER_PASSWORDS,
    WEB_ENGINE_AUTOFILL_PROFILE_DATA,
    WEB_ENGINE_SCRIPTS_CAN_OPEN_PAGES,
    CACHE_ENABLE_VALUE,
    CACHE_INTERVAL_VALUE,
    CACHE_FONT_VALUE,
    CACHE_IMAGE_VALUE,
    SAVE_CONTENT_LOCATION,
    DEFAULT_SEARCH_ENGINE,
    CURRENT_HOME_PAGE
};

#endif /* CONFIGKEY_H_ */
