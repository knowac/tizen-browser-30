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

#ifndef HISTORYITEMTYPEDEF_H_
#define HISTORYITEMTYPEDEF_H_

#include <memory>
#include <vector>
#include <string>
#include <map>

namespace tizen_browser {
namespace services {

class HistoryItem;

using HistoryItemVector = std::vector<std::shared_ptr<HistoryItem>>;
using HistoryItemVectorIter = std::vector<std::shared_ptr<HistoryItem>>::iterator;
using HistoryItemVectorConstIter = std::vector<std::shared_ptr<HistoryItem>>::const_iterator;
using HistoryItemVectorMap = std::map<std::string, services::HistoryItemVector>;

}
}

#endif /* HISTORYITEMTYPEDEF_H_ */
