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

#ifndef __SERVICE_MANAGER_HPP__
#define __SERVICE_MANAGER_HPP__

/**
 * \defgroup Using Services
 * @{
 *
 * # Creating a service
 *
 * To crate a service you could simply create a service that derives from tizen_browser::core::AbstractService,
 * or some generic ServiceInterface
 * And after that all you have to do is to use EXPORT_SERVICE macro with some "service.string"
 * (how to create "service.string" will be covered later in the doc)
 *
 * This code example shows how to crete service that inherits after AbstractService.
 *
 * Pay attention to that "service.string" is the same in EXPORT_SERVICE macro and in tizen_browser::core::ServiceManager::getService() function.
 *
 * ~~~~~{.cpp}
 * #include <AbstractService.h>
 * #include "ServiceFactory.h"
 * #include "service_macros.h"
 *
 * class BROWSER_EXPORT MyService : public ServiceInterface
 * {
 * //implement your code here
 *
 * };
 * EXPORT_SERVICE(TestServiceTwo, "org.tizen.browser.ServiceInterface")
 * ~~~~~
 *
 * # Using service
 *
 * ~~~~~{.cpp}
 * tizen_browser::core::ServiceManager *serviceManager = &tizen_browser::core::ServiceManager::getInstance();
 * td::shared_ptr<ServiceInterface> service1
 *            = std::dynamic_pointer_cast
 *               <
 *                   ServiceInterface,
 *                   tizen_browser::core::AbstractService
 *               >
 *               (serviceManager->getService("org.tizen.browser.ServiceInterface"));
 *
 * service1->runServiceFunction();
 * ~~~~~
 *
 *
 *
 * @}
 */


#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <boost/noncopyable.hpp>

#include "AbstractService.h"
#include "Lifecycle.h"

#include "ServiceFactory.h"

namespace tizen_browser
{
namespace core
{


class ServiceManagerPrivate;

/**
 * \brief Managing services.
 *
 * Loading services and creating instances.
 */
// #ifndef NDEBUG
// class ServiceManager : boost::noncopyable, ShowLifeCycle<ServiceManager>
// {
// #else
class ServiceManager : boost::noncopyable
{
// #endif
public:
    /**
     * \brief Get service instance.
     * ServiceManager is a singleton.
     */
    static ServiceManager & getInstance(void);

    /**
     * @brief Returns service instance.
     *
     * This function crates service instance with its default constructor.
     * It is up to the service author to create singletons.
     *
     * @param service ServiceString, unique string defining service API.
     * @param args Currently no used, may by omitted.
     * @return std::shared_ptr< tizen_browser::core::AbstractService >
     */
    std::shared_ptr<AbstractService> getService(const std::string& service);

private:
    /**
     * \brief This is Singleton.
     * cretate this object by calling ServiceManager::getInstance!!!
     */
    ServiceManager();

    std::unique_ptr<ServiceManagerPrivate> d;
};

} /* end of namespace core */
} /* end of namespace tizen_browser */

#endif // __SERVICE_MANAGER_HPP__
