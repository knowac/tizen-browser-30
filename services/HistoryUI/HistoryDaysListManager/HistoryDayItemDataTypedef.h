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

using WebsiteVisitItemData = struct WebsiteVisitItemData_;
using WebsiteVisitItemDataPtr = std::shared_ptr<WebsiteVisitItemData>;
using WebsiteVisitItemDataPtrConst = std::shared_ptr<const WebsiteVisitItemData>;

using WebsiteHistoryItemData = struct WebsiteHistoryItemData_;
using WebsiteHistoryItemDataPtr = std::shared_ptr<WebsiteHistoryItemData>;
using WebsiteHistoryItemDataPtrConst = std::shared_ptr<const WebsiteHistoryItemData>;

using HistoryDayItemData = struct HistoryDayItemData_;
using HistoryDayItemDataPtr = std::shared_ptr<HistoryDayItemData>;
using HistoryDayItemDataPtrConst = std::shared_ptr<const HistoryDayItemData>;
}
}

#endif /* HISTORYDAYITEMDATATYPEDEF_H_ */
