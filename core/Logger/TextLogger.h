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

#ifndef TEXT_LOGGER_H
#define TEXT_LOGGER_H

#include <fstream>
#include <iostream>

#include "Logger.h"

namespace tizen_browser
{
namespace logger
{

/**
  * @class TextLogger
  * @brief Implementation of AbstractLogger with plain text output
  *
  * This class can log both to stdout and to file, depending on the constructor
  * you use. You can instantiate and register both variants for logging
  * to stdout and to the file simultaneously.
  */
class TextLogger : public AbstractLogger
{
public:
	//! constructor used for logging plain text to stdout
	TextLogger();
	//! constructor used for logging plain text to file
	//! @param name filename to output
	TextLogger(const char *name);
	//! Destructor
	~TextLogger();
	//! Initialize the output stream to stdout or file, depending on used constructor
	virtual void init();
	// documentation inherited from AbstractLogger::Log()
	virtual void log(const std::string &timeStamp, const std::string &tag, const std::string &msg, bool errorFlag = false);
private:
	//! if logging to file, stores the filename, used later in init() method
	std::string m_filename;
	//! if logging to file, this is the file buffer used later in std::ostream object
	std::filebuf *m_fb;
	//! pointer to stdout or to std::ostream object using the \a fb variable
	std::ostream *m_output;
	//! flag set if logging to file, used later in ~TextLogger() to check if resources need to be released
	bool m_shallRelease;
};

} /* end namespace logger */
} /* end namespace browser */

#endif
