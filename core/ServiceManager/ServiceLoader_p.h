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

#ifndef SERVICELOADERPRIVATE_H
#define SERVICELOADERPRIVATE_H
#include <string>

#include "ServiceLoader.h"
#include "ServiceFactory.h"
#include "ServiceLib.h"
namespace tizen_browser
{
namespace core
{

class ServiceLoaderPrivate
{
public:
    //required pubilc by scoped_ptr
    ~ServiceLoaderPrivate();
private:
    //only friend can crate me.
    ServiceLoaderPrivate(const std::string& serviceName);

    void loadService();

    std::string serviceName;
    /**
     * \todo add support for service version if we need this.
     */
    //uint serviceVersion;
    ServiceLib lib;
    friend class ServiceLoader;
    FactoryFunctionPointer instance;
};

} /* end of namespace core */
} /* end of namespace tizen_browser */
#endif // SERVICELOADERPRIVATE_H
