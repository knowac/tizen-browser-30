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

#ifndef HISTORYDAYITEMDATATYPEDEF_H_
#define HISTORYDAYITEMDATATYPEDEF_H_

#include <memory>
#include <Elementary.h>

namespace tizen_browser {
namespace base_ui {

typedef struct WebsiteVisitItemData_ WebsiteVisitItemData;
typedef std::shared_ptr<WebsiteVisitItemData> WebsiteVisitItemDataPtr;
typedef std::shared_ptr<const WebsiteVisitItemData> WebsiteVisitItemDataPtrConst;

typedef struct WebsiteHistoryItemData_ WebsiteHistoryItemData;
typedef std::shared_ptr<WebsiteHistoryItemData> WebsiteHistoryItemDataPtr;
typedef std::shared_ptr<const WebsiteHistoryItemData> WebsiteHistoryItemDataPtrConst;

typedef struct HistoryDayItemData_ HistoryDayItemData;
typedef std::shared_ptr<HistoryDayItemData> HistoryDayItemDataPtr;
typedef std::shared_ptr<const HistoryDayItemData> HistoryDayItemDataPtrConst;

}
}

#endif /* HISTORYDAYITEMDATATYPEDEF_H_ */
