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
#include <stdexcept>
#include "ServiceLib.h"


namespace tizen_browser
{
namespace core
{

ServiceLib::ServiceLib(const std::string& libraryFile)
    : libraryFile(libraryFile)
    , pHnd(0,0)
{
    load();
}

ServiceLib::~ServiceLib()
{
}


void ServiceLib::load()
{

    int flags=0;
    flags = //RTLD_NOW     //resolve all symbols before dlopen returns
            RTLD_LAZY       //resolve sybols on ussage - fixes problem with cross linikg services.
           | RTLD_GLOBAL; //export or sybols to be used by other libraries.

    pHnd = unique_library_ptr(dlopen(libraryFile.c_str(), flags), dlclose);
    if(!pHnd){
        throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + "Library loading error:" + dlerror());
    }
}

FunctionPointer ServiceLib::resolveSymbol(const char* symbol)
{
    FunctionPointer address = reinterpret_cast<FunctionPointer>(dlsym(pHnd.get(),symbol));
    if (!address){
        throw std::runtime_error(std::string(__PRETTY_FUNCTION__)
                                + "Cannot resolve symbol \""
                                + symbol + "\" in "
                                + libraryFile
                                + ":" + dlerror());
    }
    return address;
}

} /* end of namespace core */
} /* end of namespace tizen_browser */
