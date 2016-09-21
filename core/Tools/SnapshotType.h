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

/*
 * SnapshotType.h
 *
 *  Created on: May 18, 2016
 *      Author: m.kawonczyk@samsung.com
 */

#ifndef __SNAPSHOT_TYPE_H__
#define __SNAPSHOT_TYPE_H__ 1

namespace tizen_browser
{
namespace tools
{

/**
 * @brief Snapshot type enumerator. Used to recognize why methed was created
 */
enum class SnapshotType {
    ASYNC_LOAD_FINISHED,
    ASYNC_TAB,
    ASYNC_BOOKMARK,
    SYNC
};

}
}

#endif /* __SNAPSHOT_TYPE_H__ */
