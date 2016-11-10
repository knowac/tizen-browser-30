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

#ifndef SERVICELOADER_H
#define SERVICELOADER_H
#include <memory>
#include <string>

namespace tizen_browser
{
namespace core
{

class ServiceFactory;

///\todo try to replace with std::function
typedef void (*FunctionPointer)();
typedef ServiceFactory* (*FactoryFunctionPointer)();

class ServiceLoaderPrivate;

class ServiceLoader
{
public:
    /**
     * Use this constructor to load a service dynamic library.
     * Service name shouldn't have 'lib' prefix.
     *
     * \param serviceName The name of service library.
     */
    ServiceLoader(const std::string& serviceFileName);
    ~ServiceLoader();

    /**
     * User this to obtain the factory object of the service.
     * \return ServiceFactory required service.
     */
    ServiceFactory* getFactory();

    /**
     * The name of the service (as given in constructor).
     * \return The service name.
     */
    std::string serviceName() const;

    /**
     * \todo: add support for version
     */
    //uint serviceVersion() const;

private:

    std::unique_ptr<ServiceLoaderPrivate> d;
};

} /* end of namespace core */
} /* end of namespace tizen_browser */

#endif // SERVICELOADER_H
