/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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

#ifndef STRING_TOOLS_H_
#define STRING_TOOLS_H_

#include <vector>
#include <string>

using namespace std;

namespace tizen_browser {
namespace tools {
namespace string_tools {

/**
 * @brief convert all strings to downcase strings
 * @param resultKeywords converted strings
 */
void downcase(vector<string>& toDowncase);

/**
 * @brief splits given string by removing whitespaces
 * @param keywordsString string to split
 * @param resultKeywords vector to which result strings are stored
 */
void splitString(const string& keywordsString, vector<string>& resultKeywords);

/**
 * @brief searches for the longest string
 * @param strings vector with strings
 * @return position of the longest string in vector
 */
unsigned getLongest(const vector<string>& strings);

/**
 * @brief checks if string contains given pattern
 * E.g. (abcd, bc) -> true, (abcd, ad) -> false
 *
 * @param text checked text
 * @param pattern checked pattern
 */
bool simplePatternMatch(const string &text, const string &pattern);

/**
 * @brief checks if given string contains all given keywords
 * @param stringChecked checked string
 * @param itKeywordsBegin keywords vector's begin iterator
 * @param itKeywordsEnd keywords vector's end iterator
 */
bool stringMatchesKeywords(const string& stringChecked,
        const vector<string>::const_iterator itKeywordsBegin,
        const vector<string>::const_iterator itKeywordsEnd);

} /* namespace tizen_browser  */
} /* namespace tools */
} /* namespace string */

#endif /* STRING_TOOLS_H_ */
