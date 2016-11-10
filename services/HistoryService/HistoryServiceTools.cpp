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

#include <boost/algorithm/string.hpp>
#include "Tools/StringTools.h"
#include "HistoryServiceTools.h"
#include "HistoryItem.h"

namespace tizen_browser {
namespace services {

void removeMismatches(shared_ptr<HistoryItemVector> historyItems,
        const vector<string>& keywords)
{
    for (auto itItem = historyItems->begin(); itItem != historyItems->end();)
        if (!tools::string_tools::stringMatchesKeywords((*itItem)->getUrl(),
                keywords.begin(), keywords.end()))
            // remove url not matching all keywords
            itItem = historyItems->erase(itItem);
        else
            ++itItem;
}

bool containsDuplicates(std::shared_ptr<HistoryItemVector> vec,
        std::shared_ptr<HistoryItem> checked)
{
    bool found = false;
    for (auto& s : *vec)
        if (s->getUrl().compare(checked->getUrl()) == 0) {
            if (found)
                return true;
            found = true;
        }
    return false;
}

void removeUrlDuplicates(std::shared_ptr<HistoryItemVector> historyItems)
{
    for (auto itItem = historyItems->begin(); itItem != historyItems->end();)
        if (containsDuplicates(historyItems, *itItem))
            itItem = historyItems->erase(itItem);
        else
            ++itItem;
}

} /* namespace services */
} /* namespace tizen_browser */
