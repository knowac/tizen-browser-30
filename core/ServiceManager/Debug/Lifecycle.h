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

#ifndef __LIFECYCLE_HPP__
#define __LIFECYCLE_HPP__

#include <iostream>
#include <typeinfo>

#include "BrowserLogger.h"

#ifndef NDEBUG

#define BEGIN() std::cout << "Begin of " << __PRETTY_FUNCTION__ << std::endl;
#define END() std::cout << "End of " << __PRETTY_FUNCTION__ << std::endl;

template <typename T>
struct ShowLifeCycle {
    ShowLifeCycle() {
        BROWSER_LOGD("Created object: '%s'", type<T>().c_str());
    }

    ~ShowLifeCycle() {
        BROWSER_LOGD("Destroyed object: '%s'", type<T>().c_str());
    }

};

#else

#define BEGIN()
#define END()

template <typename T>
struct ShowLifeCycle {
};

#endif

#endif // __LIFECYCLE_HPP__
