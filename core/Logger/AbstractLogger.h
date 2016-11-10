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

#ifndef ABSTRACT_LOGGER_H
#define ABSTRACT_LOGGER_H

#include <string>

namespace tizen_browser
{
namespace logger
{

/**
  * @brief the interface for specialized loggers
  */
class AbstractLogger {
public:
	//! constructor - nothing special
	AbstractLogger() { };
	//! destructor - nothing special
	virtual ~AbstractLogger() { };

	//! this method is called after setup of Logger parameters, any initialization of specialized loggers shall be placed in this method
	virtual void init() = 0;
	/**
	  * @brief Adds a log message as-is to the buffer
	  * @param timeStamp the message to log
	  * @param tag the message to log
	  * @param msg the message to log
	  & @param errorFlag marks the message as an error, can be used by specialized loggers
	  */
	virtual void log(const std::string &timeStamp, const std::string &tag, const std::string &msg, bool errorFlag = false) = 0;
};

} /* end namespace logger */
} /* end namespace tizen_browser */

#endif
