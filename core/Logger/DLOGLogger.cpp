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

/*
 * DLOGLogger.cpp
 *
 *  Created on: Apr 9, 2014
 *       Author: p.chmielewsk@samsung.com
 */

#include "browser_config.h"
#include <dlog.h>
#include "DLOGLogger.h"
#include "Logger.h"

using tizen_browser::logger::LoggerLevel;
using tizen_browser::logger::Logger;

namespace tizen_browser
{
namespace logger
{

DLOGLogger::DLOGLogger() {
}

DLOGLogger::~DLOGLogger() {
}

void DLOGLogger::init() {

}

void DLOGLogger::log(const std::string & /*timeStamp*/, const std::string & tag, const std::string & msg, bool /*errorFlag*/) {
    LoggerLevel level = Logger::getLoggerlevel();
    log_priority priority = DLOG_DEFAULT;
    switch (level) {
        case LoggerLevel::NONE:
            priority = DLOG_UNKNOWN;
            break;
        case LoggerLevel::DEBUG:
            priority = DLOG_DEBUG;
            break;
        case LoggerLevel::FATAL:
            priority = DLOG_FATAL;
            break;
        case LoggerLevel::ERROR:
            priority = DLOG_ERROR;
            break;
        case LoggerLevel::WARN:
            priority = DLOG_WARN;
            break;
        case LoggerLevel::INFO:
            priority = DLOG_INFO;
            break;
        default:
            priority = DLOG_DEFAULT;
            break;
    }
    __dlog_print(LOG_ID_SYSTEM,priority, tag.c_str(), "%s", msg.c_str());
}

} /* end namespace logger */
} /* end namespace browser */
