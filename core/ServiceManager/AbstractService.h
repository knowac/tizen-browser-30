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

#ifndef __ABSTRACT_SERVICE_HPP__
#define __ABSTRACT_SERVICE_HPP__

#include <unordered_map>
#include <string>
#include <boost/any.hpp>

#include "Debug/Lifecycle.h"

namespace tizen_browser
{
namespace core
{
/**
 * \brief Basic service interface.
 *
 * Every class that would like to be a service needs to inherit AbstractService.
 *
 */
#ifndef NDEBUG
struct AbstractService : ShowLifeCycle<AbstractService> {
#else
struct AbstractService {
#endif

    virtual ~AbstractService() {}

    /**
     * \brief Returns service id string.
     */
    virtual std::string getName() = 0;
};

} /* end of namespace core */
} /* end of namespace tizen_browser */

#endif // __ABSTRACT_SERVICE_HPP__
