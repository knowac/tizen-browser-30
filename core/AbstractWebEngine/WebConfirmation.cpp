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

/*
 * WebConfirmation.cpp
 *
 *  Created on: Apr 11, 2014
 *      Author: p.rafalski
 */

#include "browser_config.h"
#include "WebConfirmation.h"

#include <boost/uuid/uuid_generators.hpp>


namespace tizen_browser {
namespace basic_webengine {

WebConfirmation::WebConfirmation(ConfirmationType type, TabId tabId, const std::string & uri, const std::string & msg)
    : m_confirmationType(type)
    , m_confirmationId(boost::uuids::random_generator()())
    , m_tabId(tabId)
    , m_uri(uri)
    , m_message(msg)
{
}

WebConfirmation::~WebConfirmation()
{
}

CertificateConfirmation::CertificateConfirmation(TabId tabId, const std::string & uri, const std::string & msg)
    : WebConfirmation(ConfirmationType::CertificateConfirmation, tabId, uri, msg)
{
}


} /* end of basic_webengine */
} /* end of tizen_browser */


