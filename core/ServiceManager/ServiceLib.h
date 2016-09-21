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

#ifndef SERVICELIB_H
#define SERVICELIB_H

#include <dlfcn.h>
#include <memory>
#include <string>

#include "ServiceLoader.h"


namespace tizen_browser
{
namespace core
{

class ServiceLib
{
public:
    explicit ServiceLib(const std::string& libraryFile);
    ~ServiceLib();

    /**
     * \brief Loads dynamic library.
     * \throw std::runtime_error, with proper error message.
     */
    void load();

    /**
     * Looks for symbol in library (or any other currently loaded)
     * \return FunctionPointer to resolved symbol 0 if not found.
     * \throw std::runtime_error with error message on error.
     */
    FunctionPointer resolveSymbol(const char* symbol);

    /**
     * @brief Typedef, RAI for library loading.
     *
     * dlopen return void*
     * dlclose return int
     */
    typedef std::unique_ptr<void, int(*)(void *)> unique_library_ptr;

    std::string libraryFile;/// Holds library file name.
    unique_library_ptr pHnd;
};

} /* end of namespace core */
} /* end of namespace tizen_browser */

#endif // SERVICELIB_H
