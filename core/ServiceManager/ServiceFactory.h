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

#ifndef __SERVICE_FACTORY_H__
#define __SERVICE_FACTORY_H__ 1

#include <string>
#include <map>
#include <boost/any.hpp>

#include "AbstractService.h"

namespace tizen_browser
{
namespace core
{

/**
* @brief base service factory class
*
* This class shouldn't be used directly. It is used internally by ServiceManager, and
* user service factories inherits this class, but you shouldn't have to use it directly.
*/
class ServiceFactory
{
public:


    /**
     * \brief Creates a factory for a service.
     *
     *
     */
     ServiceFactory();

    /**
     * \brief This destroys the ServiceFactory
     */
    virtual ~ServiceFactory();

    /**
     * \brief Returns service name.
     *
     * You can use this method to retrieve service name.
     * \returns Name of the service
     */
    virtual std::string serviceName() const = 0;

    /**
     * \brief Returns object instance
     * \returns Ready to use service object.
     */
    virtual AbstractService *create()=0;

};

} /* end of namespace core */
} /* end of namespace tizen_browser */

#endif //__SERVICE_FACTORY_H__
