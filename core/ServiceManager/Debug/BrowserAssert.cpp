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
#include <cerrno>
#include <utility>

#include "BrowserAssert.h"

namespace tizen_browser
{
namespace debug
{

void EFailedAssertion::swap(EFailedAssertion & other)
{
    using std::swap;
    if (&other != this)
        swap(_what, other._what);
    return;
}

void failed_assert(char const * const fileName_,
                   int line_, char const * const functionName_,
                   char const * const message_)
{
    /*
     * Add logging facility and log assertion.
     */
#if 0
    log << "Failed assertion: " << message_ << " -> " << fileName_ << "(" << line_ << "): " << functionName_ << endl;
#endif
    fprintf(stderr, "Failed assertion: `%s' at: %s: %4d : %s\n",
            message_, fileName_, line_, functionName_);
    if (!errno)
        ++errno;
    if (::getenv("ABORT_ON_ASSERT"))
        ::abort();
    /**
     * \todo: Implement process introspections and dump stack
     */
#if 0
    static int const DUMP_DEPTH = 64;
    dump_call_stack(clog, DUMP_DEPTH);
#endif
    throw EFailedAssertion(message_);
}

} /* end of namespace debug */
} /* end of namespace tizen_browser */
