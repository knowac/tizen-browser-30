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
#include <unordered_map>
#include <boost/filesystem.hpp>
#include "Config.h"
#include "BrowserLogger.h"
#include "ServiceManager.h"
#include "ServiceManager_p.h"

namespace tizen_browser
{
namespace core
{

ServiceManagerPrivate::ServiceManagerPrivate()
{
    findServiceLibs();
    loadServiceLibs();
}

ServiceManagerPrivate::~ServiceManagerPrivate()
{}

void ServiceManagerPrivate::findServiceLibs() try
{
    boost::filesystem::path servicesDir(
        boost::any_cast<std::string>(tizen_browser::config::Config::getInstance().get("services/dir")));
    for (boost::filesystem::directory_iterator it(servicesDir);
        it != boost::filesystem::directory_iterator();
        ++it) {
        boost::filesystem::path item(*it);
        if (boost::filesystem::is_regular_file(item)) {
            if ((item.extension().string() == ".so" ) &&
                (item.filename().string().find("lib") != std::string::npos)) {
                try {
                    servicesLoaderMap[item.string()] = std::make_shared<ServiceLoader>(item.string());
                } catch (std::runtime_error & e) {
                    BROWSER_LOGD(e.what() );
                }
            }
        }
    }
} catch (const boost::filesystem::filesystem_error& ex) {
    BROWSER_LOGD(ex.what());
}

void ServiceManagerPrivate::loadServiceLibs()
{
    /// TODO make sure that librareis are founded.
    for (auto slm : servicesLoaderMap) {
        try {
            auto factory = slm.second->getFactory();
            servicesMap[factory->serviceName()] = factory;//do not write to map if there's an error.
        } catch (const std::runtime_error& e) {
            BROWSER_LOGD(e.what() );
        }
    }
}

void ServiceManagerPrivate::enumerateServices(){
    for (auto sm : servicesMap)
        BROWSER_LOGD("%s:%p", sm.first.c_str(), sm.second);
}

ServiceManager::ServiceManager()
    :d(new ServiceManagerPrivate)
{}

ServiceManager& ServiceManager::getInstance(void)
{
    static ServiceManager instance;
    return instance;
}

std::shared_ptr< AbstractService > ServiceManager::getService(const std::string& service)
{
    static std::unordered_map<std::string, std::shared_ptr<AbstractService>> cache;
    static std::mutex mut;


    std::lock_guard<std::mutex> hold(mut);
    auto sp = cache[service];

    if (!sp)
        cache[service] = sp = std::shared_ptr<AbstractService>(d->servicesMap[service]->create());
    return sp;
}


} /* end of namespace core */
} /* end of namespace tizen_browser */
