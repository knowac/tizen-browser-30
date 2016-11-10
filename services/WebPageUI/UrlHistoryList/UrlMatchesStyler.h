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

#ifndef URLMATCHESSTYLER_H_
#define URLMATCHESSTYLER_H_

#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include "BrowserLogger.h"

using namespace std;

namespace tizen_browser {
namespace base_ui {

class UrlMatchesStyler
{
public:
    UrlMatchesStyler();
    virtual ~UrlMatchesStyler();

    /**
     * @brief  Get string containing EFL tags, which are highlighting given keywords.
     * @param styledUrl url which will be styled
     * @param highlightingKeyword keywords (entered url) indicating which
     * fragments should be highlighted
     * @return styledUrl enriched with EFL tags
     */
    string getUrlHighlightedMatches(const string& styledUrl,
            const string& highlightingKeywords) const;

private:
    typedef vector<pair<int, int>> int_pairs;
    const string FONT_COLOR_HIGHLIGHT = "#4088D3";
    const string FONT_COLOR_NORMAL = "#888888";
    const string FONT_SIZE = "35";
    const string TAG_WHOLE_URL;
    const string TAG_WHOLE_URL_CLOSE;
    const string TAG_HIGHLIGHT, TAG_HIGHLIGHT_CLOSE;
    const string TAG_COLOR, TAG_COLOR_CLOSE;
    const string TAG_COMPLETE, TAG_COMPLETE_CLOSE;
    const int TAGS_COMPLETE_LEN;

    /**
     * @brief adds '/' to a tag (<a> -> </a>)
     * @param tag tag to be closed
     * @return closed tag
     */
    string closeTag(const string& tag) const;
    /**
     * @brief splits given string by removing spaces
     * @param keywordsString string to split
     * @param resultKeywords vector to which result strings are stored
     */
    void splitKeywordsString(const string& keywordsString,
            vector<string>& resultKeywords) const;
    /**
     * @brief Fills vector with ranges describing beginnings end ends of occurrences of one string in another.
     * @param checkedString the subject of search
     * @param searchedMatch match to be searched for
     * @param resultRanges vector filled with found ranges
     */
    void fillOccuranceRanges(const string& checkedString,
            const string& searchedMatch, int_pairs& resultRanges) const;
    /**
     * @brief Searches the string for positions of occurrences of another string.
     * @param checkedString the subject of search
     * @param searchedMatch string to be searched for
     * @param resultPositions vector filled with result positions
     */
    void getMatchesPositions(const string& checkedString,
            const string& searchedMatch, vector<int>& resultPositions) const;
    /**
     * @brief merges ranges
     * @param ranges vector of ranges to merge
     * @param result vector filled with merged ranges
     */
    void mergeRanges(int_pairs& ranges, int_pairs& result) const;
    /**
     * @brief get string enriched with opening and closing tags on given positions
     * @param strToHighlight string to be enriched with tags
     * @param ranges positions of opening and closing tags
     * @param tag opening tag (for every pair.first)
     * @param tagClose closing tag (for every pair.second)
     * @return string with tags
     */
    string getTaggedString(const string& strToHighlight,
            const int_pairs& ranges) const;

};

} /* namespace base_ui */
} /* namespace tizen_browser */

#endif /* URLMATCHESSTYLER_H_ */
