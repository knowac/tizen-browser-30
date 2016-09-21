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
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <string>

#include "Logger.h"

namespace tizen_browser
{
namespace logger {

std::string make_message(const char *fmt, ...)
{
	int n;
	int size = 512;   /* Guess we need no more than 512 bytes */
	int old_size = 0;
	char *p, *np;
	va_list ap;

	try {
		p = new char[size];
	} catch (std::bad_alloc) {
		Logger::getInstance().log("Error while allocating memory!!");
		return std::string();
	}
	while (true)
	{
		/* Try to print in the allocated space */
		va_start(ap, fmt);
		n = vsnprintf(p, size, fmt, ap);
		va_end(ap);

		/* Check error code */
		if (n < 0) {
			delete [] p;
			return std::string();
        }

		/* If that worked, return the string */
		if (n < size) {
			std::string retval(p);
			delete [] p;
			return retval;
		}
		/* Else try again with more space */
		old_size = size;
		size = n + 1;       /* Precisely what is needed */

		try {
			np = new char[size];
			std::copy(p, p + old_size, np);
			delete [] p;
			p = np;
		} catch (std::bad_alloc) {
			delete [] p;
			Logger::getInstance().log("Error while allocating memory!!");
			return std::string();
		}
	}
}

} /* end namespace logger */
} /* end namespace browser */
