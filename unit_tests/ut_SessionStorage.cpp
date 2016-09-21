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

#include <boost/any.hpp>
#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>
#include <iterator>
#include "core/ServiceManager/ServiceManager.h"
#include "core/ServiceManager/AbstractService.h"
#include "services/StorageService/SessionStorage.h"
#include "services/StorageService/Session.h"
#include "AbstractWebEngine/TabId.h"
#include <core/Config/Config.h>
#include "BrowserLogger.h"


BOOST_AUTO_TEST_SUITE(SessionStorage)

BOOST_AUTO_TEST_CASE(InitSession)
{
    BROWSER_LOGI("[UT] SessionStorage - InitSession - START --> ");

    std::string resourceDbDir(boost::any_cast < std::string > (tizen_browser::config::Config::getInstance().get("resourcedb/dir")));
    std::string sessionDb(boost::any_cast < std::string > (tizen_browser::config::Config::getInstance().get("DB_SESSION")));

    boost::filesystem::path dbFile(resourceDbDir + sessionDb);
    boost::filesystem::remove(dbFile);

    std::shared_ptr<tizen_browser::services::SessionStorage> sessionService =
    std::dynamic_pointer_cast
    <
        tizen_browser::services::SessionStorage,
        tizen_browser::core::AbstractService
    >(tizen_browser::core::ServiceManager::getInstance().getService(DOMAIN_SESSION_STORAE_SERVICE));

    BOOST_REQUIRE(sessionService);
    BOOST_CHECK_EQUAL(sessionService->getName(), DOMAIN_SESSION_STORAE_SERVICE);

    tizen_browser::storage::SessionStorage* storage=0;
    storage = sessionService->getStorage();

    BOOST_REQUIRE(storage);

    BROWSER_LOGI("[UT] --> END - SessionStorage - InitSession");
}

BOOST_AUTO_TEST_CASE(CreateAndPopulateSession)
{
    BROWSER_LOGI("[UT] SessionStorage - CreateAndPopulateSession - START --> ");

    std::shared_ptr<tizen_browser::services::SessionStorage> sessionService =
    std::dynamic_pointer_cast
    <
        tizen_browser::services::SessionStorage,
        tizen_browser::core::AbstractService
    >(tizen_browser::core::ServiceManager::getInstance().getService(DOMAIN_SESSION_STORAE_SERVICE));

    BOOST_REQUIRE(sessionService);

    tizen_browser::storage::SessionStorage* storage=0;
    storage = sessionService->getStorage();

    BOOST_REQUIRE(storage);

    tizen_browser::Session::Session session(storage->createSession());

    BOOST_CHECK(session.isValid());

    std::map<std::string, std::pair<std::string, std::string>> urls;

    urls["54379ff6-f9ff-4ef3-99b0-a0de00edd473"] = std::pair<std::string, std::string>("http://www.calligra.org", "Title");
    urls["7b5719d4-c2f5-4d87-89ff-9cd70da1710f"] = std::pair<std::string, std::string>("http://www.kde.org", "Title");
    urls["ce18e8e2-8d33-4ba7-9fc4-d602cdf3fa36"] = std::pair<std::string, std::string>("http://www.krita.org", "Title");

    for(auto iter = urls.begin(), end = urls.end(); iter != end; iter++){
        session.updateItem(iter->first, iter->second.first, iter->second.second);
    }


    BOOST_CHECK_EQUAL(session.items().size(), 3);

    std::string replaceUrl("https://marble.kde.org/");


    session.updateItem(std::next(urls.begin(), 2)->first, replaceUrl, std::next(urls.begin(), 2)->second.second);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        replaceUrl.begin(),
        replaceUrl.end(),
        session.items().at(std::next(urls.begin(),2)->first).first.begin(),
        session.items().at(std::next(urls.begin(),2)->first).first.end()
    );

    session.removeItem(std::next(urls.begin(),1)->first);

    BOOST_CHECK_EQUAL(session.items().size(), 2);

    BROWSER_LOGI("[UT] --> END - SessionStorage - CreateAndPopulateSession");
}

BOOST_AUTO_TEST_CASE(getLastSession)
{
    BROWSER_LOGI("[UT] SessionStorage - getLastSession - START --> ");

    std::shared_ptr<tizen_browser::services::SessionStorage> sessionService =
    std::dynamic_pointer_cast
    <
        tizen_browser::services::SessionStorage,
        tizen_browser::core::AbstractService
    >(tizen_browser::core::ServiceManager::getInstance().getService(DOMAIN_SESSION_STORAE_SERVICE));

    BOOST_REQUIRE(sessionService);

    tizen_browser::storage::SessionStorage* storage=0;
    storage = sessionService->getStorage();

    BOOST_REQUIRE(storage);

    sleep(2);
    tizen_browser::Session::Session session(storage->createSession());
    const std::string newSessionName("theLastOfUs");

    session.setSessionName("theLastOfUs");

    BOOST_CHECK(session.isValid());

    std::map<std::string, std::pair<std::string, std::string>> urls;

    urls["54379ff6-f9ff-4ef3-99b0-a0de00edd473"] = std::pair<std::string, std::string>("http://www.calligra.org", "Title");
    urls["7b5719d4-c2f5-4d87-89ff-9cd70da1710f"] = std::pair<std::string, std::string>("http://www.kde.org", "Title");
    urls["ce18e8e2-8d33-4ba7-9fc4-d602cdf3fa36"] = std::pair<std::string, std::string>("http://www.krita.org", "Title");

    for(auto iter = urls.begin(), end = urls.end(); iter != end; iter++){
        session.updateItem(iter->first, iter->second.first, iter->second.second);
    }

    BOOST_CHECK_EQUAL(session.items().size(), 3);

    tizen_browser::Session::Session lastSession(storage->getLastSession());

    BOOST_REQUIRE(lastSession.isValid());

    BOOST_CHECK_EQUAL_COLLECTIONS(
        newSessionName.begin(),
        newSessionName.end(),
        lastSession.sessionName().begin(),
        lastSession.sessionName().end()
    );

    BOOST_CHECK_EQUAL(lastSession.items().size(), 3);

    BROWSER_LOGI("[UT] --> END - SessionStorage - getLastSession");
}

BOOST_AUTO_TEST_CASE(getAllSessions)
{
    BROWSER_LOGI("[UT] SessionStorage - getAllSessions - START --> ");

    std::shared_ptr<tizen_browser::services::SessionStorage> sessionService =
    std::dynamic_pointer_cast
    <
        tizen_browser::services::SessionStorage,
        tizen_browser::core::AbstractService
    >(tizen_browser::core::ServiceManager::getInstance().getService(DOMAIN_SESSION_STORAE_SERVICE));

    BOOST_REQUIRE(sessionService);

    tizen_browser::storage::SessionStorage* storage=0;
    storage = sessionService->getStorage();

    BOOST_REQUIRE(storage);

    //new session should be newer then previous one.
    tizen_browser::Session::SessionsVector sessions(storage->getAllSessions());

    BOOST_CHECK_EQUAL(sessions.size(), 2);

    BROWSER_LOGI("[UT] --> END - SessionStorage - getAllSessions");
}


BOOST_AUTO_TEST_CASE(deleteSession)
{
    BROWSER_LOGI("[UT] SessionStorage - deleteSession - START --> ");

    std::shared_ptr<tizen_browser::services::SessionStorage> sessionService =
    std::dynamic_pointer_cast
    <
        tizen_browser::services::SessionStorage,
        tizen_browser::core::AbstractService
    >(tizen_browser::core::ServiceManager::getInstance().getService(DOMAIN_SESSION_STORAE_SERVICE));

    BOOST_REQUIRE(sessionService);

    tizen_browser::storage::SessionStorage* storage=0;
    storage = sessionService->getStorage();

    BOOST_REQUIRE(storage);

    //new session should be newer then previous one.
    tizen_browser::Session::SessionsVector sessions(storage->getAllSessions());

    storage->deleteSession(sessions.at(0));

    tizen_browser::Session::SessionsVector sessionsBucket(storage->getAllSessions());

    BOOST_CHECK_EQUAL(sessionsBucket.size(),1);

    BROWSER_LOGI("[UT] --> END - SessionStorage - deleteSession");
}


BOOST_AUTO_TEST_CASE(deleteAllSessions)
{
    BROWSER_LOGI("[UT] SessionStorage - deleteAllSessions - START --> ");

    std::shared_ptr<tizen_browser::services::SessionStorage> sessionService =
    std::dynamic_pointer_cast
    <
        tizen_browser::services::SessionStorage,
        tizen_browser::core::AbstractService
    >(tizen_browser::core::ServiceManager::getInstance().getService(DOMAIN_SESSION_STORAE_SERVICE));

    BOOST_REQUIRE(sessionService);

    tizen_browser::storage::SessionStorage* storage=0;
    storage = sessionService->getStorage();

    BOOST_REQUIRE(storage);

    //new session should be newer then previous one.
    sleep(1);
    tizen_browser::Session::Session session2(storage->createSession());
    sleep(1);
    tizen_browser::Session::Session session3(storage->createSession());
    sleep(1);
    tizen_browser::Session::Session session4(storage->createSession());


    std::map<std::string, std::pair<std::string, std::string>> urls;

    urls["54379ff6-f9ff-4ef3-99b0-a0de00edd473"] = std::pair<std::string, std::string>("http://www.calligra.org", "Title");
    urls["7b5719d4-c2f5-4d87-89ff-9cd70da1710f"] = std::pair<std::string, std::string>("http://www.kde.org", "Title");
    urls["ce18e8e2-8d33-4ba7-9fc4-d602cdf3fa36"] = std::pair<std::string, std::string>("http://www.krita.org", "Title");

    for(auto iter = urls.begin(), end = urls.end(); iter != end; iter++){
        session2.updateItem(iter->first, iter->second.first, iter->second.second);
        sleep(1);
        session3.updateItem(iter->first, iter->second.first, iter->second.second);
        sleep(1);
        session4.updateItem(iter->first, iter->second.first, iter->second.second);
    }


    tizen_browser::Session::SessionsVector sessionsBucket(storage->getAllSessions());

    BOOST_CHECK_EQUAL(sessionsBucket.size(), 4);

    storage->deleteAllSessions();

    sessionsBucket = storage->getAllSessions();

    BOOST_CHECK_EQUAL(sessionsBucket.size(), 0);

    BROWSER_LOGI("[UT] --> END - SessionStorage - deleteAllSessions");
}

BOOST_AUTO_TEST_SUITE_END()
