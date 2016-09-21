/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
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

#ifndef CERTIFICATECONTENTS_H
#define CERTIFICATECONTENTS_H

#include <Evas.h>
#include <Elementary.h>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <boost/signals2/signal.hpp>

#include <openssl/bio.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>


#include "AbstractService.h"
#include "ServiceFactory.h"
#include "service_macros.h"

namespace tizen_browser
{

namespace services
{

class BROWSER_EXPORT CertificateContents : public tizen_browser::core::AbstractService
{
public:
    CertificateContents();
    ~CertificateContents();

    /**
     * @brief Returns current service's name.
     */
    virtual std::string getName();

    enum HOST_TYPE {
        SECURE_HOST = 1,
        UNSECURE_HOST_ALLOWED,
        UNSECURE_HOST_UNKNOWN,
        HOST_ABSENT = -1
    };

    void init();
    void initUI(Evas_Object* parent);
    Evas_Object* getContent();
    void unsecureHostAllowed();

    void setCurrentTabCertData(std::string host, std::string pem, HOST_TYPE type);
    Eina_Bool isValidCertificate() const { return m_hostType == SECURE_HOST; }
    bool isValidCertificate(const std::string& uri);

    HOST_TYPE isCertExistForHost(const std::string& host);
    void saveCertificateInfo(const std::string& host, const std::string& pem);
    void saveWrongCertificateInfo(const std::string& host, const std::string& pem);
    void clear();

    boost::signals2::signal<std::shared_ptr<std::vector<std::pair<std::string, int> > > ()> getHostCertList;
    boost::signals2::signal<void (const std::string&, const std::string&, int)> addOrUpdateCertificateEntry;

private:

    typedef enum _certificate_field{
        ISSUED_TO_HEADER = 0,
        ISSUED_TO_CN,
        ISSUED_TO_ORG,
        ISSUED_TO_ORG_UNIT,
        ISSUED_TO_SERIAL_NO,
        ISSUED_BY_HEADER,
        ISSUED_BY_CN,
        ISSUED_BY_ORG,
        ISSUED_BY_ORG_UNIT,
        VALIDITY_HEADER,
        VALIDITY_ISSUED_ON,
        VALIDITY_EXPIRES_ON,
        FINGERPRINTS_HEADER,
        FINGERPRINTS_SHA_256_FP,
        FINGERPRINTS_SHA_1_FP,
        FIELD_END
    } certificate_field;

    struct genlist_callback_data {
        certificate_field type;
        const char *title;
        const char *value;
    };

    void addToHostCertList(const std::string& host, HOST_TYPE type);
    bool createCertificate(const char *cert_data);
    Evas_Object* createGenlist(Evas_Object* parent);
    Evas_Object* createLabel(Evas_Object* parent,  const std::string& msg);
    void _parse_certificate();
    void _populate_certificate_field_data(char *data, certificate_field field);
    void _generate_genlist_data(certificate_field type, const char *title, const char *value);

    static char* __auth_text_get_cb(void* data, Evas_Object* obj, const char *part);
    static char* __field_text_get_cb(void* data, Evas_Object* obj, const char *part);
    static char* __title_value_text_get_cb(void* data, Evas_Object* obj, const char *part);

    Evas_Object* m_mainLayout;
    Evas_Object* m_genlist;
    Evas_Object* m_parent;
    std::vector<std::shared_ptr<genlist_callback_data> > m_genlist_callback_data_list;
    std::string m_edjFilePath;
    std::map<std::string, HOST_TYPE> m_host_cert_info;

    X509 *m_certificate;
    HOST_TYPE m_hostType;

};

}
}

#endif  // CERTIFICATECONTENTS_H
