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
#include <sstream>
#include <cstring>
#include <sys/time.h>

#include "Logger.h"

namespace tizen_browser {
namespace logger {

static const std::string LEVEL_NONE_TAG = "None";
static const std::string LEVEL_DEBUG_TAG = "Debug";
static const std::string LEVEL_FATAL_TAG = "Fatal";
static const std::string LEVEL_ERROR_TAG = "Error";
static const std::string LEVEL_WARN_TAG = "Warning";
static const std::string LEVEL_INFO_TAG = "Info";

static const std::string LEVEL_TAG_ERROR = "TAG_ERROR";

#ifdef LOG_LEVEL
static const LoggerLevel globalLoggerLevel = Logger::parseLoggerLevel(LOG_LEVEL);
#else
static const LoggerLevel globalLoggerLevel = LoggerLevel::DEBUG;
#endif

Logger &Logger::getInstance() {
	static Logger instance;
	return instance;
}

const LoggerLevel & Logger::getLoggerlevel(){
    return globalLoggerLevel;
}

LoggerLevel Logger::parseLoggerLevel(const char* levelString){
	LoggerLevel level;
		if (strcmp(levelString, "NONE") == 0) {
			level = LoggerLevel::NONE;
		} else if (strcmp(levelString, "DEBUG") == 0) {
			level = LoggerLevel::DEBUG;
		} else if (strcmp(levelString, "FATAL") == 0) {
			level = LoggerLevel::FATAL;
		} else if (strcmp(levelString, "ERROR") == 0) {
			level = LoggerLevel::ERROR;
		} else if (strcmp(levelString, "WARN") == 0) {
			level = LoggerLevel::WARN;
		} else if (strcmp(levelString, "INFO") == 0) {
			level = LoggerLevel::INFO;
		} else {
			level = LoggerLevel::INFO;
		}
		return level;
}

const std::string & Logger::getLevelTag(LoggerLevel level) {
	switch (level) {
	case LoggerLevel::NONE:
		return LEVEL_NONE_TAG;
	case LoggerLevel::DEBUG:
		return LEVEL_DEBUG_TAG;
	case LoggerLevel::FATAL:
		return LEVEL_FATAL_TAG;
	case LoggerLevel::ERROR:
		return LEVEL_ERROR_TAG;
	case LoggerLevel::WARN:
		return LEVEL_WARN_TAG;
	case LoggerLevel::INFO:
		return LEVEL_INFO_TAG;
	default:
		return LEVEL_TAG_ERROR;
	}
}

void Logger::setProjectName(const char *name) {
	m_projectName = name;
}

std::string Logger::getProjectName() const {
	return m_projectName;
}

void Logger::init() {
	std::vector<std::shared_ptr<AbstractLogger> >::iterator it =
			m_loggers.begin(), end = m_loggers.end();
	for (; it != end; ++it) {
		(*it)->init();
	}
}

void Logger::log(const std::string &msg, bool errorFlag,
		LoggerLevel ) {
	std::vector<std::shared_ptr<AbstractLogger> >::iterator it =
			m_loggers.begin(), end = m_loggers.end();
	for (; it != end; ++it) {
		(*it)->log(timeStamp(), m_tag, msg, errorFlag);
	}
}

void Logger::info(const std::string & msg) {
	if (globalLoggerLevel > LoggerLevel::INFO) {
		log(msg, false, LoggerLevel::INFO);
	}
}

void Logger::warn(const std::string & msg) {
	if (globalLoggerLevel > LoggerLevel::WARN) {
		log(msg, false, LoggerLevel::WARN);
	}
}

void Logger::error(const std::string & msg) {
	if (globalLoggerLevel > LoggerLevel::ERROR) {
		log(msg, true, LoggerLevel::ERROR);
	}
}

void Logger::fatal(const std::string & msg) {
	if (globalLoggerLevel > LoggerLevel::FATAL) {
		log(msg, true, LoggerLevel::FATAL);
	}
}

void Logger::debug(const std::string & msg) {
	if (globalLoggerLevel > LoggerLevel::DEBUG) {
		log(msg, false, LoggerLevel::DEBUG);
	}
}

int Logger::registerLogger(AbstractLogger *l) {
	m_loggers.push_back(std::shared_ptr<AbstractLogger>(l));
	return 1;
}

std::string Logger::timeStamp() {
        time_t initializer = time(NULL);
        struct tm b;
        if(localtime_r(&initializer,&b)==NULL){
            return std::string("");
        }

        struct timeval detail_time;
        gettimeofday(&detail_time,NULL);

        char buf[80];
//	strftime(buf, sizeof(buf), "%d/%m/%y,%T ", brokenTime, detail_time.tv_usec/1000);
        snprintf(buf,  sizeof(buf),"[%d/%d/%d,%d:%d:%d.%ld]", b.tm_year, b.tm_mon, b.tm_mday, b.tm_hour, b.tm_min, b.tm_sec, detail_time.tv_usec/1000);
        return std::string(buf);
}

} /* end namespace logger */
} /* end namespace browser */
