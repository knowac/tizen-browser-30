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

#ifndef WEBCONFIRMATION_H_
#define WEBCONFIRMATION_H_ 1

#include <boost/uuid/uuid.hpp>
#include <memory>
#include <string>

#include "TabId.h"

namespace tizen_browser {
namespace basic_webengine {

class WebConfirmation;
typedef std::shared_ptr<WebConfirmation> WebConfirmationPtr;

class WebConfirmation
{
public:
    enum ConfirmationType {
        UserMedia,
        ContentHandler,
        ProtocolHandler,
        Geolocation,
        Notification,
        ScriptAlert,
        ScriptConfirmation,
        ScriptPrompt,
        CertificateConfirmation,
        Authentication
    };

    enum ConfirmationResult {
        None,
        Confirmed,
        Rejected
    };

    WebConfirmation(ConfirmationType type, TabId tabId, const std::string & uri, const std::string & msg);

    virtual ~WebConfirmation();

    ConfirmationType getConfirmationType() const {
        return m_confirmationType;
    }
    TabId getTabId() const {
        return m_tabId;
    }
    std::string getURI() const{
        return m_uri;
    }
    std::string getMessage() const {
        return m_message;
    }
    ConfirmationResult getResult() const {
        return m_result;
    }
    void setResult(ConfirmationResult res) {
        m_result = res;
    }

    virtual bool operator==(const WebConfirmation & n) const {
        return m_confirmationId == n.m_confirmationId;
    }
    virtual bool operator!=(const WebConfirmation & n) const {
        return m_confirmationId != n.m_confirmationId;
    }


private:
    ConfirmationType m_confirmationType;
    boost::uuids::uuid m_confirmationId;
    TabId m_tabId;
    std::string m_uri;
    std::string m_message;
    ConfirmationResult m_result;
};

class CertificateConfirmation;
typedef std::shared_ptr<CertificateConfirmation> CertificateConfirmationPtr;

class CertificateConfirmation : public WebConfirmation {
public:
    CertificateConfirmation(TabId tabId, const std::string & uri, const std::string & msg);
    void setPem(const std::string &pem) { m_pem = pem; }
    std::string getPem() const { return m_pem; }
    void setData(void* data) { m_data = data; }
    void* getData() const { return m_data; }
private:
    std::string m_pem;
    void* m_data;
};



} /* namespace basic_webengine */
} /* namespace tizen_browser */
#endif /* WEBCONFIRMATION_H_ */
