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

#include "UrlMatchesStyler.h"

namespace tizen_browser {
namespace base_ui {

UrlMatchesStyler::UrlMatchesStyler() :
        TAG_WHOLE_URL(
                "<align=left><color=" + FONT_COLOR_NORMAL + "><font_size="
                        + FONT_SIZE + ">"), TAG_WHOLE_URL_CLOSE(
                "</color></font></align>"), TAG_HIGHLIGHT("<font_weight=bold>"),
                TAG_HIGHLIGHT_CLOSE("</font_weight>"), TAG_COLOR(
                "<color=" + FONT_COLOR_HIGHLIGHT + ">"), TAG_COLOR_CLOSE(
                closeTag(TAG_COLOR)), TAG_COMPLETE(TAG_HIGHLIGHT + TAG_COLOR), TAG_COMPLETE_CLOSE(
                TAG_HIGHLIGHT_CLOSE + TAG_COLOR_CLOSE), TAGS_COMPLETE_LEN(
                TAG_COMPLETE.length() + TAG_COMPLETE_CLOSE.length())
{
}

UrlMatchesStyler::~UrlMatchesStyler()
{
}

string UrlMatchesStyler::closeTag(const string& tag) const
{
    string closedTag(tag);
    return string(closedTag.insert(1, "/"));
}

string UrlMatchesStyler::getUrlHighlightedMatches(const string& styledUrl,
        const string& highlightingKeywords) const
{
    vector<string> keywords;
    splitKeywordsString(highlightingKeywords, keywords);

    int_pairs rangesHighlight;
    for (auto key : keywords) {
        fillOccuranceRanges(styledUrl, key, rangesHighlight);
    }

    int_pairs mergedRangesHighlight;
    mergeRanges(rangesHighlight, mergedRangesHighlight);
    return getTaggedString(styledUrl, mergedRangesHighlight);
}

void UrlMatchesStyler::splitKeywordsString(const string& keywordsString,
        vector<string>& resultKeywords) const
{
    boost::algorithm::split(resultKeywords, keywordsString,
            boost::is_any_of("\t "), boost::token_compress_on);
    // remove empty elements
    for (auto it = resultKeywords.begin(); it != resultKeywords.end();) {
        if ((*it).empty()) {
            it = resultKeywords.erase(it);
        } else {
            ++it;
        }
    }
}

void UrlMatchesStyler::fillOccuranceRanges(const string& _checkedString,
        const string& _searchedMatch, int_pairs& resultRanges) const
{
    if (_checkedString.empty() || _searchedMatch.empty())
        return;

    string checkedString(_checkedString);
    string searchedMatch(_searchedMatch);
    boost::algorithm::to_lower(checkedString);
    boost::algorithm::to_lower(searchedMatch);

    int len = searchedMatch.length();
    vector<int> positions;
    getMatchesPositions(checkedString, searchedMatch, positions);
    for (auto pos : positions) {
        resultRanges.push_back( { pos, pos + len - 1 });
    }
}

void UrlMatchesStyler::getMatchesPositions(const string& checkedString,
        const string& searchedMatch, vector<int>& resultPositions) const
{
    boost::regex match_regex(searchedMatch);
    for (auto it = boost::sregex_iterator(checkedString.begin(),
            checkedString.end(), match_regex); it != boost::sregex_iterator();
            ++it) {
        resultPositions.push_back(it->position());
    }
}

void UrlMatchesStyler::mergeRanges(int_pairs& ranges, int_pairs& result) const
{
    if (ranges.size() == 0)
        return;
    sort(ranges.begin(), ranges.end());

    auto current = *(ranges.begin());
    for (auto it = ranges.begin() + 1; it != ranges.end(); ++it) {
        if (current.second >= it->first) {
            current.second = max(current.second, it->second);
        } else {
            result.push_back(current);
            current = *it;
        }
    }
    result.push_back(current);
}

string UrlMatchesStyler::getTaggedString(const string& strToHighlight,
        const int_pairs& ranges) const
{
    string strResult(strToHighlight);
    int insertOffset = 0;
    for (auto pair : ranges) {
        strResult.insert(pair.second + insertOffset + 1, TAG_COMPLETE_CLOSE);
        strResult.insert(pair.first + insertOffset, TAG_COMPLETE);
        insertOffset += TAGS_COMPLETE_LEN;
    }
    strResult.insert(strResult.length(), TAG_WHOLE_URL_CLOSE);
    strResult.insert(0, TAG_WHOLE_URL);
    return strResult;
}

} /* namespace base_ui */
} /* namespace tizen_browser */
