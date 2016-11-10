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

#include <cassert>
#include <sstream>
#include <string>

#include <boost/test/unit_test.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/algorithm/string/trim.hpp>

// Dirty hack used do get access for private class interiors
// for test porpouses.
#define private public
#define protected public

#include "Logger.h"
#include "AbstractLogger.h"
#include "BrowserLogger.h"


#define COLOR "\033[41m\033[37m"

BOOST_AUTO_TEST_SUITE(logger)

struct StubAbstractLogger: tizen_browser::logger::AbstractLogger {
	StubAbstractLogger() :
			initialized(false) {
	}
	~StubAbstractLogger() {
	}
	void init() {
		ss.flush();
		initialized = true;
	}
	void log(const std::string &timeStamp, const std::string &tag,
			const std::string &msg, bool /*errorFlag*/ = false) {
		std::string str = timeStamp + tag + msg;
		ss << str << '\n';
	}

	std::string getLog() {
		std::string ret = ss.str();
		ss.flush();
		return ret;
	}

	bool initialized;
	std::stringstream ss;
};

// simple tests for inherited interface
BOOST_AUTO_TEST_CASE(logger_abstract_logger) {
        BROWSER_LOGI("[UT] LOGGER - logger_abstract_logger - START --> ");

	std::stringstream ss;
	ss << boost::unit_test::framework::current_test_case().p_name;

	StubAbstractLogger sl;

	sl.init();
	BOOST_CHECK(sl.getLog().empty());

	sl.log("a", "b", "c");
	BOOST_CHECK_EQUAL(sl.getLog().compare("abc\n"), 0);
	BROWSER_LOGI("[UT] --> END - LOGGER - logger_abstract_logger");
}

class StubLogger: public tizen_browser::logger::Logger {
public:
	virtual ~StubLogger() {
	}
	static StubLogger& getInstance() {
		static StubLogger instance;
		return instance;
	}

private:
	StubLogger() { }
};

BOOST_AUTO_TEST_CASE(logger_init) {
        BROWSER_LOGI("[UT] LOGGER - logger_init - START --> ");

	std::stringstream ss;
	ss << boost::unit_test::framework::current_test_case().p_name;

	// checks for support functions
	StubLogger::Logger::getInstance().setProjectName("bla");
	BOOST_CHECK_EQUAL(StubLogger::Logger::getInstance().m_projectName.compare("bla"), 0);
	BOOST_CHECK_EQUAL(StubLogger::Logger::getInstance().getProjectName(), "bla");

	// checks for registering logger functionality
	StubAbstractLogger *l1 = new StubAbstractLogger;
	StubAbstractLogger *l2 = new StubAbstractLogger;

	BOOST_CHECK_EQUAL(l1->initialized, false);
	BOOST_CHECK_EQUAL(l2->initialized, false);

	StubLogger::getInstance().registerLogger(l1);
	StubLogger::getInstance().registerLogger(l2);
	StubLogger::getInstance().init();

	BOOST_CHECK_EQUAL(l1->initialized, true);
	BOOST_CHECK_EQUAL(l2->initialized, true);

	// checks for logging process
	std::string msg("Bla");
	StubLogger::getInstance().log(msg);

	std::string checker = l1->getLog();
	checker.erase(checker.begin(), std::find(checker.begin(), checker.end(), ']') + 1);
	boost::algorithm::trim(checker);
	BOOST_CHECK_EQUAL(checker, msg);

	checker = l2->getLog();
	checker.erase(checker.begin(), std::find(checker.begin(), checker.end(), ']') + 1);
	boost::algorithm::trim(checker);
	BOOST_CHECK_EQUAL(checker, msg);

        BROWSER_LOGI("[UT] --> END - LOGGER - logger_init");
}

///\todo p.chmielewski
/*
BOOST_AUTO_TEST_CASE(logger_levels_getting) {
    using tizen_browser::logger::Logger;
    using tizen_browser::logger::LoggerLevel;

	std::string infoTag = Logger::getLevelTag(LoggerLevel::INFO);
	BOOST_CHECK_EQUAL(infoTag, "Info");

	std::string warningTag = Logger::getLevelTag(LoggerLevel::WARN);
	BOOST_CHECK_EQUAL(warningTag, "Warning");

	std::string errorTag = Logger::getLevelTag(LoggerLevel::ERROR);
	BOOST_CHECK_EQUAL(errorTag, "Error");

	std::string fatalTag = Logger::getLevelTag(LoggerLevel::FATAL);
	BOOST_CHECK_EQUAL(fatalTag, "Fatal");

	std::string debugTag = Logger::getLevelTag(LoggerLevel::DEBUG);
	BOOST_CHECK_EQUAL(debugTag, "Debug");

	std::string noneTag = Logger::getLevelTag(LoggerLevel::NONE);
	BOOST_CHECK_EQUAL(noneTag, "");
}

BOOST_AUTO_TEST_CASE(logger_levels_parsing) {
	using tizen_browser::logger::Logger;
	using tizen_browser::logger::LoggerLevel;

	LoggerLevel loggerLevel;

	loggerLevel = Logger::parseLoggerLevel("INFO");
	BOOST_CHECK_EQUAL(loggerLevel, LoggerLevel::INFO);

	loggerLevel = Logger::parseLoggerLevel("WARN");
	BOOST_CHECK_EQUAL(loggerLevel, LoggerLevel::WARN);

	loggerLevel = Logger::parseLoggerLevel("ERROR");
	BOOST_CHECK_EQUAL(loggerLevel, LoggerLevel::ERROR);

	loggerLevel = Logger::parseLoggerLevel("FATAL");
	BOOST_CHECK_EQUAL(loggerLevel, LoggerLevel::FATAL);

	loggerLevel = Logger::parseLoggerLevel("DEBUG");
	BOOST_CHECK_EQUAL(loggerLevel, LoggerLevel::DEBUG);

	loggerLevel = Logger::parseLoggerLevel("NONE");
	BOOST_CHECK_EQUAL(loggerLevel, LoggerLevel::NONE);
}
*/
BOOST_AUTO_TEST_SUITE_END()
