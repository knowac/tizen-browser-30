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

#ifndef __SERVICE_MACROS_H__
#define __SERVICE_MACROS_H__

/**
 * @def BROWSER_EXPORT
 * @ingroup BROWSERMacros
 *
 * The BROWSER_EXPORT macro marks the symbol of the given variable
 * to be visible, so it can be used from outside the resulting library.
 *
 * \code
 * int BROWSER_NO_EXPORT foo;
 * int BROWSER_EXPORT bar;
 * \endcode
 *
 * @sa BROWSER_NO_EXPORT
 */

/**
 * @def BROWSER_IMPORT
 * @ingroup BROWSERMacros
 */

#ifdef __BROWSER_HAVE_GCC_VISIBILITY
#define BROWSER_NO_EXPORT __attribute__ ((visibility("hidden")))
#define BROWSER_EXPORT __attribute__ ((visibility("default")))
#define BROWSER_IMPORT __attribute__ ((visibility("default")))
#else
#define BROWSER_NO_EXPORT
#define BROWSER_EXPORT
#define BROWSER_IMPORT
#endif

#ifndef EXTERN_C
#  ifdef __cplusplus
#    define EXTERN_C extern "C"
#  else
#    define EXTERN_C extern
#  endif
#endif


#define SERVICE_FACTORY(name, serviceClass, ifaceName) \
    class name : public tizen_browser::core::ServiceFactory \
    { \
    public: \
        name(){}; \
        ~name(){}; \
        virtual std::string serviceName() const{return std::string(ifaceName);}; \
        tizen_browser::core::AbstractService* create(){ return new serviceClass();}; \
    };

#define SERVICE_INSTANCE(name) \
    EXTERN_C BROWSER_EXPORT void* service_factory_instance() \
    { \
        static name _instance; \
        return reinterpret_cast<void*>(&_instance); \
    }

#define SERVICE_STRING_INSTANCE(IMPLCLASS,SERVICEIFACE) \
    inline std::string IMPLCLASS::getName(){return std::string(SERVICEIFACE);}

#define FACTORY_NAME(name) name ## Factory

#define EXPORT_SERVICE(SERVICECLASS,SERVICE_STRING) \
        SERVICE_FACTORY(FACTORY_NAME(SERVICECLASS), SERVICECLASS, SERVICE_STRING) \
        SERVICE_INSTANCE(FACTORY_NAME(SERVICECLASS)) \
        SERVICE_STRING_INSTANCE(SERVICECLASS,SERVICE_STRING)


#endif // __SERVICE_MACROS_H__
