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

#ifndef LOGGER_H
#define LOGGER_H

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <boost/noncopyable.hpp>

#include "AbstractLogger.h"

namespace tizen_browser {
namespace logger {

enum class LoggerLevel {
	NONE,
	DEBUG,
	FATAL,
	ERROR,
	WARN,
	INFO,
};

/**
 * @brief This function converts printf-like input to std::string.
 *
 * @param fmt Preformatted message, like in printf.
 * @param ... Arguments.
 *
 * @return std::string object based on input parameters,
 */
std::string make_message(const char * fmt, ...);

/**
 * @class Logger
 * @brief Singleton - public Logger interface
 *
 * This is the object providing the Logger functionality to other modules
 * of Test Framework
 */
class Logger: boost::noncopyable {
public:
	//! returns instance of the singleton
	static Logger & getInstance();

	//! returns tag string for provided logging level
	static const std::string & getLevelTag(LoggerLevel level);

	//! returns logging level based on provided character string
	static LoggerLevel parseLoggerLevel(const char* levelString);

	static const LoggerLevel & getLoggerlevel();

	//! sets project name for future usage in specialized loggers
	void setProjectName(const char * name);

	//! gets project name, used by specialized loggers to access project name
	std::string getProjectName() const;

	//! calls init() method for all registered loggers
	//! @see AbstractLogger::init()
	void init();

	/** @brief Put a \a msg to the registered loggers.
	 *
	 * This method adds a timestamp and tag, then passes the result to all
	 * registered loggers, calling their AbstractLogger::Log() method.
	 *
	 * If \a errorFlag is true, the logger is able to mark the line.
	 *
	 * This method is not thread aware. The Test Framework itself always calls
	 * the method inside the ciritcal section (see: TestCase::lockAssert()).
	 * If you are running tests in multithreaded environment and calling
	 * the Log() method on your own, you shall guard the call with critical
	 * section.
	 *
	 * @see AbstractLogger::Log()
	 */
	void log(const std::string & msg, bool errorFlag = false, LoggerLevel loggerLevel = LoggerLevel::INFO);

	void info(const std::string & msg);

	void warn(const std::string & msg);

	void error(const std::string & msg);

	void fatal(const std::string & msg);

	void debug(const std::string & msg);

	//! set task name as a tag
	//! @see clearLogTag()
	inline void setLogTag(const std::string tagName) {
		m_tag = tagName;
	}

	inline const std::string& getLogTag() {
	    return m_tag;
	}

	inline size_t getLoggersCount(){
	    return m_loggers.size();
	}

	//! clears the tag, using an empty string as a tag
	//! @see setLogTag()
	inline void clearLogTag() {
		m_tag = "";
	}

	//! returns timestamp
	static std::string timeStamp();
	//! register logger given as a pointer, you can use the method directly or via REGISTER_LOGGER() wrapper
	int registerLogger(AbstractLogger * l);
private:
	//! constructor
	Logger() :
			m_projectName("[unset project name]") {
	}
	;

	//! stores projectname
	std::string m_projectName;

	//! stores currently used tag
	std::string m_tag;

	//! table of registered loggers
	std::vector<std::shared_ptr<AbstractLogger> > m_loggers;
};

} /* end namespace logger */
} /* end namespace browser */

/// \todo: Refactor as static functions not macros.

/**
 * @def REGISTER_LOGGER(logger)
 * @brief Register logger
 *
 * Register new logger. \a logger must be constructor of the class derived from
 * AbstractLogger. Effectively, this set of macros is a wrapper for
 * new \a logger.
 */
#define REGISTER_LOGGER(logger_type) REGISTER_WITH_LINE(logger_type, __LINE__)

#define REGISTER_WITH_LINE(logger_type, line) _REGISTER_WITH_LINE(logger_type, line)

#define _REGISTER_WITH_LINE(logger_type, line) static int reg##line = tizen_browser::logger::Logger::getInstance().registerLogger(new logger_type)

#endif
