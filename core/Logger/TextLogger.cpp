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
#include <cstdlib>
#include <fstream>
#include <iostream>

#include "TextLogger.h"

namespace tizen_browser
{
namespace logger
{

TextLogger::TextLogger()
    : m_fb(nullptr)
    , m_output(nullptr)
{
	m_shallRelease = false;
	m_filename.clear();
}

TextLogger::TextLogger(const char *name)
    : m_fb(nullptr)
    , m_output(nullptr)
{
	m_shallRelease = false;
	m_filename = name;
}

TextLogger::~TextLogger()
{
	if (m_shallRelease) {
		m_output->flush();
		m_fb->close();
		delete m_output;
		delete m_fb;
	}
	m_output = NULL;
}

void TextLogger::init()
{
	if (m_filename.empty()) {
		m_output = &std::cout;
		m_shallRelease = false;
	} else {
		m_fb = new std::filebuf;
		std::filebuf *result = m_fb->open(m_filename.c_str(), std::ios::out);
		if (!result) {
			Logger::getInstance().log("Cannot open file" + m_filename, true);
			exit(1);
		}
		m_output = new std::ostream(m_fb);
		m_shallRelease = true;
	}
}

void TextLogger::log(const std::string &timeStamp, const std::string &tag, const std::string &msg, bool /*errorFlag*/)
{
	if (m_output) {
		std::string str = timeStamp + "[" + tag + "] " + msg;
		(*m_output) << str << "\n";
	}
}

} /* end namespace logger */
} /* end namespace browser */
