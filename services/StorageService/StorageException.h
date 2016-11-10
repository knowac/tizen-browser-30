/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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

#ifndef __STORAGE_EXCEPTIONS_H
#define __STORAGE_EXCEPTIONS_H

#include <string>
#include <sqlite3.h>
#include <stdexcept>

#include "ServiceFactory.h"
#include "service_macros.h"

namespace tizen_browser {
namespace storage {

class BROWSER_EXPORT StorageException: public std::exception
{
public:
    explicit StorageException(const std::string & message, int code = SQLITE_ERROR);
    virtual ~StorageException() throw ();
    virtual int getErrorCode() const;
    virtual const char * getMessage() const throw ();
    virtual const char* what() const throw() {return m_message.c_str();};
protected:
    int m_code;
    std::string m_message;
};

}
}

#endif
