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

#ifndef SERVICEMANAGERPRIVATE_H
#define SERVICEMANAGERPRIVATE_H 1

#include <memory>

#include "ServiceManager.h"
#include "ServiceLoader.h"

namespace tizen_browser
{
namespace core
{

class ServiceManagerPrivate{
public:
    //required by scoped_ptr
    ~ServiceManagerPrivate();
private:
    ServiceManagerPrivate();
    /**
     * Find all library files in service direcotry.
     */
    void findServiceLibs();

    /**
     * Try to load services from libraries
     */
    void loadServiceLibs();

    /**
     * debug function to display all founded and loaded services.
     */
    void enumerateServices();/// write names of all services

    //ServiceFactory is a static ServiceFactory member - no need to delete it manually (or by smart_ptr) by calling delete or free on it
    std::unordered_map<std::string, ServiceFactory*> servicesMap; /// "com.class.interface":&ServiceFactory
    std::unordered_map<std::string, std::shared_ptr<ServiceLoader>>  servicesLoaderMap; /// "path/to/library.so": &service_factory_interface

    friend class ServiceManager;
};

} /* end of namespace core */
} /* end of namespace tizen_browser */

#endif //SERVICEMANAGERPRIVATE_H
