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

/*
 * BookmarkFolder.h
 *
 *  Created on: Nov 26, 2015
 *      Author: m.kawonczyk@samsung.com
 */


#ifndef BOOKMARKFOLDER_H
#define BOOKMARKFOLDER_H

#include "BrowserLogger.h"
#include "BrowserImage.h"

namespace tizen_browser {
namespace services {

class BookmarkFolder
{
public:
    BookmarkFolder();
    BookmarkFolder(
        unsigned int id,
        const std::string& name,
        unsigned int count
        );
    virtual ~BookmarkFolder();

    void setId(int id) { m_id = id; };
    unsigned int getId() const { return m_id; };

    void setName(const std::string & name) { m_name = name; };
    std::string getName() const { return m_name; };

    void setCount(int count) { m_count = count; };
    unsigned int getCount() const { return m_count; };

private:
    unsigned int m_id;
    std::string m_name;
    unsigned int m_count;
};

typedef std::shared_ptr<BookmarkFolder> SharedBookmarkFolder;
typedef std::vector<SharedBookmarkFolder> SharedBookmarkFolderList;

}

namespace base_ui {

typedef services::SharedBookmarkFolder SharedBookmarkFolder;
typedef std::vector<SharedBookmarkFolder> SharedBookmarkFolderList;

}
}

#endif // BOOKMARKITEM_H

