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

#ifndef HISTORYMATCHFINDER_H_
#define HISTORYMATCHFINDER_H_

#include <string>
#include <vector>

#include "HistoryItemTypedef.h"

using namespace std;

namespace tizen_browser {
namespace services {

/**
 * @brief Removes history items not matching given keywords
 * @param historyItems Vector from which mismatching items will be removed
 * @param keywords Keywords (history item is a match, when all keywords are
 * matching)
 */
void removeMismatches(std::shared_ptr<HistoryItemVector> historyItems,
        const vector<string>& keywords);

/**
 * @brief Returns true, if vector contains at least two items with the same url.
 */
bool containsDuplicates(std::shared_ptr<HistoryItemVector> vec,
        std::shared_ptr<HistoryItem> checked);

/**
 * @brief Removes history items with urls duplicating other items.
 * In the end, vector has items with unique URLs.
 */
void removeUrlDuplicates(std::shared_ptr<HistoryItemVector> historyItems);

} /* namespace services */
} /* namespace tizen_browser */

#endif /* HISTORYMATCHFINDER_H_ */
