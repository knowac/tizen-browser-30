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
#include "browser_config.h"
#include "BrowserLogger.h"
#include "ServiceLoader.h"
#include "ServiceLoader_p.h"
#include <iostream>

namespace tizen_browser
{
namespace core
{


ServiceLoaderPrivate::ServiceLoaderPrivate(const std::string& serviceFileName)
    : serviceName(serviceFileName)
    , lib(serviceFileName)
    , instance(0)
{
}
ServiceLoaderPrivate::~ServiceLoaderPrivate()
{
}

void ServiceLoaderPrivate::loadService()
{
    instance = reinterpret_cast<FactoryFunctionPointer>(lib.resolveSymbol("service_factory_instance"));
}


ServiceLoader::ServiceLoader(const std::string& serviceFileName)
    : d(new ServiceLoaderPrivate(serviceFileName))
{
    BROWSER_LOGD( "%s %s", __PRETTY_FUNCTION__ , serviceFileName.c_str() );
}


ServiceLoader::~ServiceLoader()
{
}


std::string ServiceLoader::serviceName() const
{
    return d->serviceName;
}


ServiceFactory* ServiceLoader::getFactory()
{
    if(d->instance){
        return d->instance();
    } else{
        d->loadService();
        return d->instance();
    }
}

} /* end of namespace core */
} /* end of namespace tizen_browser */
