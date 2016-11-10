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

#ifndef __BROWSER_LOGGER_H__
#define __BROWSER_LOGGER_H__ 1

#include "browser_config.h"
#include <string>

#if !defined(NDEBUG) || PLATFORM(TIZEN)

#include "Logger/Logger.h"

#define BROWSER_LOGD(fmt, args...) tizen_browser::logger::Logger::getInstance().log(tizen_browser::logger::make_message(fmt, ##args))
#define BROWSER_LOGI(fmt, args...) tizen_browser::logger::Logger::getInstance().log(tizen_browser::logger::make_message(fmt, ##args))
#define BROWSER_LOGW(fmt, args...) tizen_browser::logger::Logger::getInstance().log(tizen_browser::logger::make_message(fmt, ##args))
#define BROWSER_LOGE(fmt, args...) tizen_browser::logger::Logger::getInstance().log(tizen_browser::logger::make_message(fmt, ##args))

#define BROWSER_ENABLE_LOG

#else

#define BROWSER_LOGD(fmt, args...) do { } while(0)
#define BROWSER_LOGI(fmt, args...) do { } while(0)
#define BROWSER_LOGW(fmt, args...) do { } while(0)
#define BROWSER_LOGE(fmt, args...) do { } while(0)

#endif

#endif /* __BROWSER_LOGGER_H__ */
