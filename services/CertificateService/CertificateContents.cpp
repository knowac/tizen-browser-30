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

#include "CertificateContents.h"
#include "BrowserLogger.h"
#include <openssl/asn1.h>
#include <openssl/bn.h>
#include "app_i18n.h"
#include "Tools/GeneralTools.h"

#define SHA256LEN 32
#define SHA1LEN 20

#define BR_STRING_UNABLE_TO_VIEW_THE_CERTIFICATE_THE_PAGE_INFORMATION_HAS_BEEN_CHANGED  _("IDS_BR_POP_UNABLE_TO_VIEW_THE_CERTIFICATE_THE_PAGE_INFORMATION_HAS_BEEN_CHANGED")
#define BR_STRING_CERTIFICATE_SERIAL_NUMBER    _("IDS_BR_HEADER_SERIAL_NUMBER_C_ABB")
#define BR_STRING_ISSUED_ON                    _("IDS_BR_HEADER_ISSUED_ON_C")
#define BR_STRING_EXPIRES_ON_C                 _("IDS_BR_HEADER_EXPIRES_ON_C")
#define BR_STRING_FINGERPRINTS_SHA256          _("IDS_BR_BODY_SHA_256_FINGERPRINT_C")
#define BR_STRING_FINGERPRINTS_SHA1            _("IDS_BR_BODY_SHA_1_FINGERPRINT_C")
#define BR_STRING_ISSUED_BY_C                  _("IDS_BR_HEADER_ISSUED_BY_C")
#define BR_STRING_ORGANIZATION                 _("IDS_BR_HEADER_ORGANISATION_C_ABB")
#define BR_STRING_ORGANIZATION_UNIT            _("IDS_BR_HEADER_DEPARTMENT_C_ABB")
#define BR_STRING_ISSUED_TO_C                  _("IDS_BR_HEADER_ISSUED_TO_C")
#define BR_STRING_VALIDITY_C                   _("IDS_BR_HEADER_VALIDITY_C")
#define BR_STRING_FINGERPRINTS                 _("IDS_BR_BODY_FINGERPRINTS_C")
#define BR_STRING_CERTI_MESSAGE                _("IDS_BR_BODY_SECURITY_CERTIFICATE_PROBLEM_MSG")
#define BR_STRING_VALID_CERTIFICATE            _("IDS_BR_BODY_VALID_CERTIFICATE")
#define BR_STRING_UNTRUSTED_PAGE               _("IDS_BR_HEADER_SITE_NOT_TRUSTED_ABB")
#define BR_STRING_DOUBLE_TAP_VIEW_CERTIFICATE  _("IDS_BR_BODY_DOUBLE_TAP_TO_VIEW_CERTIFICATE_TTS")
#define BR_STRING_SECURITY_CERTIFICATE         _("IDS_BR_HEADER_SECURITY_CERTIFICATE")
#define BR_STRING_VIEW_CERTIFICATE             _("IDS_BR_OPT_VIEW_CERTIFICATE")
#define BR_STRING_SECURITY_WARNING_HEADER      _("IDS_BR_HEADER_SITE_NOT_TRUSTED_ABB")
#define BR_STRING_TRUSTED_AUTHORITY            _("IDS_BR_POP_THIS_CERTIFICATE_IS_FROM_A_TRUSTED_AUTHORITY")
#define BR_STRING_UNTRUSTED_AUTHORITY          _("IDS_BR_POP_THIS_CERTIFICATE_IS_NOT_FROM_A_TRUSTED_AUTHORITY")
#define BR_STRING_COMMON_NAME                  _("IDS_BR_HEADER_COMMON_NAME_C_ABB")

namespace tizen_browser
{

namespace services
{

EXPORT_SERVICE(CertificateContents, "org.tizen.browser.certificateservice")

CertificateContents::CertificateContents()
    : m_mainLayout(nullptr)
    , m_parent(nullptr)
    , m_certificate(nullptr)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("CertificateService/CertificatePopup.edj");
    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());
}

CertificateContents::~CertificateContents()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (m_genlist)
        evas_object_del(m_genlist);
    if (m_mainLayout)
        evas_object_del(m_mainLayout);
    m_genlist_callback_data_list.clear();
    X509_free(m_certificate);
}

void CertificateContents::init()
{
    auto certList = getHostCertList();
    if (certList) {
        for (auto it = (*certList)->begin(); it != (*certList)->end(); ++it) {
            addToHostCertList(it->first, static_cast<HOST_TYPE>(it->second));
        }
    } else {
        BROWSER_LOGW("[%s:%d] wrong certList value!", __PRETTY_FUNCTION__, __LINE__);
    }
}

void CertificateContents::setCurrentTabCertData(std::string host, std::string pem, HOST_TYPE type)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    if (m_genlist) {
        elm_genlist_clear(m_genlist);
        evas_object_del(m_genlist);
    }
    m_genlist_callback_data_list.clear();
    m_hostType = type;

    if (!createCertificate(pem.c_str()))
        m_hostType = UNSECURE_HOST_UNKNOWN;

    addToHostCertList(host, m_hostType);
    m_genlist = createGenlist(m_parent);
}

bool CertificateContents::isValidCertificate(const std::string& uri)
{
    HOST_TYPE type = isCertExistForHost(tools::extractDomain(uri));
    return type == SECURE_HOST;
}

CertificateContents::HOST_TYPE CertificateContents::isCertExistForHost(const std::string& host)
{
    /*Returns the host type if a cert. exists for the host */
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    if (m_host_cert_info.find(host) == m_host_cert_info.end())
        return HOST_ABSENT;
    else
        return m_host_cert_info[host];
}

void CertificateContents::addToHostCertList(const std::string& host, HOST_TYPE type)
{
    BROWSER_LOGD("[%s:%d] host: %s type: %d", __PRETTY_FUNCTION__, __LINE__, host.c_str(), type);
    m_host_cert_info[host] = type;
}

void CertificateContents::saveCertificateInfo(const std::string& host, const std::string& pem)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    HOST_TYPE savedType = isCertExistForHost(host);
    if (savedType == HOST_ABSENT)
        savedType = SECURE_HOST;

    setCurrentTabCertData(host, pem, savedType);
    addOrUpdateCertificateEntry(pem, host, static_cast<int>(savedType));
}

void CertificateContents::saveWrongCertificateInfo(const std::string& host, const std::string& pem)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    HOST_TYPE savedType = UNSECURE_HOST_ALLOWED;
    setCurrentTabCertData(host, pem, savedType);
    addOrUpdateCertificateEntry(pem, host, static_cast<int>(savedType));
}

void CertificateContents::clear()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_host_cert_info.clear();
}

void CertificateContents::initUI(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_parent = parent;
}

Evas_Object* CertificateContents::createLabel(Evas_Object* parent, const std::string& msg)
{
    Evas_Object* layout = elm_layout_add(parent);
    elm_layout_file_set(layout, m_edjFilePath.c_str(), "label_layout");
    elm_layout_text_set(layout, "auth_text", msg.c_str());
    evas_object_show(layout);

    return layout;
}

Evas_Object* CertificateContents::getContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    if (m_hostType == HOST_ABSENT) {
        return createLabel(m_parent, BR_STRING_CERTI_MESSAGE);
    } else {
        return m_genlist;
    }
}

bool CertificateContents::createCertificate(const char* certificate_data)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    /*
     *Bring the PEM cert. data into an OpenSSL memory BIO
     *This memory BIO will be used to convert the PEM cert
     *data to X509 format
     */
    BIO *cert_mem_bio = nullptr;
    cert_mem_bio = BIO_new(BIO_s_mem());
    if (cert_mem_bio == nullptr) {
        BROWSER_LOGE("[%s:%d] Failed to create OpenSSL memory BIO", __PRETTY_FUNCTION__, __LINE__);
        BIO_free(cert_mem_bio);
        return false;
    }
    BIO_puts(cert_mem_bio, certificate_data);

    /*
     *Convert from PEM to x509
     */
    m_certificate = PEM_read_bio_X509(cert_mem_bio, nullptr, 0 , nullptr);
    BIO_free(cert_mem_bio);

    if (!m_certificate) {
        BROWSER_LOGE("[%s:%d] PEM to x509 conversion failed", __PRETTY_FUNCTION__, __LINE__);
        return false;
    }

    _parse_certificate();
    return true;
}

Evas_Object* CertificateContents::createGenlist(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (m_genlist) {
        elm_genlist_clear(m_genlist);
        evas_object_del(m_genlist);
    }
    m_genlist = elm_genlist_add(parent);

    elm_scroller_policy_set(m_genlist, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_ON);
    elm_genlist_mode_set(m_genlist, ELM_LIST_COMPRESS);
    evas_object_size_hint_weight_set(m_genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);

    evas_object_show(m_genlist);

    Elm_Genlist_Item_Class *auth_item_class = elm_genlist_item_class_new();
    if (!auth_item_class) {
        BROWSER_LOGE("elm_genlist_item_class_new for item_class failed");
        return m_genlist;
    }
    auth_item_class->item_style = "cert_auth_text";
    auth_item_class->func.text_get = __auth_text_get_cb;
    auth_item_class->func.content_get = nullptr;
    auth_item_class->func.state_get = nullptr;
    auth_item_class->func.del = nullptr;
    Elm_Object_Item *it = elm_genlist_item_append(m_genlist, auth_item_class, this, nullptr, ELM_GENLIST_ITEM_NONE, nullptr, nullptr);
    elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

    Elm_Genlist_Item_Class *field_item_class = elm_genlist_item_class_new();
    if (!field_item_class) {
        BROWSER_LOGE("elm_genlist_item_class_new for item_class failed");
        return m_genlist;
    }
    field_item_class->item_style = "cert_field_text";
    field_item_class->decorate_item_style = nullptr;
    field_item_class->func.text_get = __field_text_get_cb;
    field_item_class->func.content_get = nullptr;
    field_item_class->func.state_get = nullptr;
    field_item_class->func.del = nullptr;

    Elm_Genlist_Item_Class *item_class = elm_genlist_item_class_new();
    if (!item_class) {
        BROWSER_LOGE("elm_genlist_item_class_new for item_class failed");
        return m_genlist;
    }
    item_class->item_style = "cert_title_value_text";
    item_class->decorate_item_style = nullptr;
    item_class->func.text_get = __title_value_text_get_cb;
    item_class->func.content_get = nullptr;
    item_class->func.state_get = nullptr;
    item_class->func.del = nullptr;

    int no_of_items = m_genlist_callback_data_list.size();
    for (int i = 0; i < no_of_items; i++) {
        Elm_Object_Item *item = nullptr;
        auto callback_data = m_genlist_callback_data_list[i];
        if (callback_data->type == ISSUED_BY_HEADER || callback_data->type == ISSUED_TO_HEADER ||
                callback_data->type == VALIDITY_HEADER || callback_data->type == FINGERPRINTS_HEADER) {
            item = elm_genlist_item_append(m_genlist, field_item_class, callback_data.get(), nullptr, ELM_GENLIST_ITEM_NONE, nullptr, nullptr);
        } else
            item = elm_genlist_item_append(m_genlist, item_class, callback_data.get(), nullptr, ELM_GENLIST_ITEM_NONE, nullptr, nullptr);
        elm_genlist_item_select_mode_set(item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
    }

    return m_genlist;
}

char* CertificateContents::__auth_text_get_cb(void* data, Evas_Object* /*obj*/, const char* part)
{
    CertificateContents* cc = static_cast<CertificateContents*>(data);
    if (!strcmp(part, "auth_text")) {
        if (cc->isValidCertificate())
            return strdup(BR_STRING_TRUSTED_AUTHORITY);
        else
            return strdup(BR_STRING_UNTRUSTED_AUTHORITY);
    }
    return strdup("");
}

char* CertificateContents::__field_text_get_cb(void* data, Evas_Object* /*obj*/, const char* part)
{
    auto callback_data = static_cast<genlist_callback_data* >(data);

    if (callback_data && !strcmp(part, "field_text")) {
        return strdup(callback_data->title);
    }

    BROWSER_LOGW("[%s:%d] callback_data is empty!", __PRETTY_FUNCTION__, __LINE__);
    return strdup("");
}

char* CertificateContents::__title_value_text_get_cb(void* data, Evas_Object* /*obj*/, const char* part)
{
    auto callback_data = static_cast<genlist_callback_data* >(data);

    if (callback_data) {
        if (!strcmp(part, "title_text")) {
            return strdup(callback_data->title);
        } else if (!strcmp(part,"value_text")) {
            return strdup(callback_data->value);
        }

        BROWSER_LOGW("[%s:%d] unknown part: \"%s\"!", __PRETTY_FUNCTION__, __LINE__, part);
        return strdup("");
    }

    BROWSER_LOGW("[%s:%d] callback_data is empty!", __PRETTY_FUNCTION__, __LINE__);
    return strdup("");
}

/*
 *This method will parse the string and fetch the data
 *which is inbetween '=' and '/' or '\0'.
 */
static const char* _get_value(char* token)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!token)
        return nullptr;

    BROWSER_LOGD("token %s", token);
    int start = 0;
    int end = 0;
    int len = strlen(token);

    while (start < len && token[start] != '=')
        start++;
    start++; //to skip '=' char
    if (start >= len) return nullptr; //couldnt find the proper value so dont add this item in genlist

    for (int i = start; i < len; i++) {
        if ((token[i] == '/') || (token[i] == '\0'))
            break;
        end++; //If no string found return the whole string
    }
    std::string token_str = token;
    std::string value = token_str.substr(start, end);
    return strdup(value.c_str());
}

/*
 *This method will format the ANS1_TIME struct to readable time format
 */
static const char* _get_formatted_time(ASN1_TIME* tm)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    char timeBuf[128] = {'\0', };
    BIO *sBio = BIO_new(BIO_s_mem());
    if (sBio) {
        int retVal = ASN1_TIME_print(sBio, tm);
        if (retVal <= 0) {
            BROWSER_LOGE("ASN1_TIME_print failed or wrote no data.\n");
            BIO_free(sBio);
            return nullptr;
        }
        retVal = BIO_gets(sBio, timeBuf, 128);
        if (retVal <= 0) {
            BROWSER_LOGE("Failed to transfer contents to TimeBuffer");
            BIO_free(sBio);
            return nullptr;
        }
        BIO_free(sBio);
    }
    return strdup(timeBuf);
}

/*
 *This method will convert the serial number in required format.
 */
static const char* _get_formatted_serial_no(ASN1_INTEGER *bs )
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    char printable[100]={'\0',};
    BIGNUM *bn = ASN1_INTEGER_to_BN(bs, nullptr);
    unsigned char* binSerial = nullptr;
    unsigned int outsz;
    outsz = BN_num_bytes(bn);
    if(bn){
        if (BN_is_negative(bn)) {
            outsz++;
            if (!(binSerial = (unsigned char* )malloc(outsz))) return 0;
            BN_bn2bin(bn, binSerial + 1);
            binSerial[0] = 0x80;
        } else {
            if (!(binSerial = (unsigned char* )malloc(outsz))) return 0;
            BN_bn2bin(bn, binSerial);
        }
    }

    for (size_t i=0; i < outsz; i++) {
        char* l = (char*) (3*i + ((intptr_t) printable));

        if (i < (outsz -1)) {
            if(binSerial != nullptr) {
                snprintf(l, 4, "%02x%c", binSerial[i],':');
            }
        }
        else {
            if(binSerial != nullptr) {
                snprintf(l, 3, "%02x", binSerial[i]);
            }
        }
    }
    free(binSerial);
    BN_free(bn);
    BROWSER_LOGD(" New Serial Number : %s",printable);
    return strdup(printable);
}

/*
 *This method is to convert binary data to hexa decimal
 */
static const char* _bin2hex(unsigned char*bin, size_t bin_size , char delimiter)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    char printable[100]={'\0',};
    for (size_t i=0; i < bin_size; i++) {
        char* l = (char*) (3*i + ((intptr_t) printable));
        snprintf(l, 4, "%02x%c", bin[i],delimiter);
    }

    return strdup(printable);
}

void CertificateContents::_parse_certificate()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    char issued_to[1024] = {'\0', };
    char issued_by[1024] = {'\0', };
    char sha256[SHA256LEN] = {'\0', };
    char sha1[SHA1LEN] = {'\0', };
    size_t size;

    for (int field_count = 0; field_count < FIELD_END; field_count++) {
        if (field_count == ISSUED_TO_HEADER) {
            //Issued to
            size = sizeof(issued_to);
            X509_NAME_oneline(X509_get_subject_name(m_certificate), issued_to, size);
            _populate_certificate_field_data(issued_to, ISSUED_TO_HEADER);
            //Serial no:
            ASN1_INTEGER *bs = X509_get_serialNumber(m_certificate);
            _generate_genlist_data(ISSUED_TO_SERIAL_NO, BR_STRING_CERTIFICATE_SERIAL_NUMBER,_get_formatted_serial_no(bs));
            field_count += 4;
        } else if (field_count == ISSUED_BY_HEADER) {
            //Issued by
            size = sizeof(issued_by);
            X509_NAME_oneline(X509_get_issuer_name(m_certificate), issued_by, size);
            _populate_certificate_field_data(issued_by, ISSUED_BY_HEADER);
            field_count += 3;
        } else if (field_count == VALIDITY_HEADER) {
            _populate_certificate_field_data(nullptr, VALIDITY_HEADER);
            //Issued On
            ASN1_TIME *issuedTime = X509_get_notBefore(m_certificate);
            _generate_genlist_data(VALIDITY_ISSUED_ON, BR_STRING_ISSUED_ON, _get_formatted_time(issuedTime));
            //Expires on
            ASN1_TIME *expiresTime = X509_get_notAfter(m_certificate);
            _generate_genlist_data(VALIDITY_EXPIRES_ON, BR_STRING_EXPIRES_ON_C, _get_formatted_time(expiresTime));
            field_count += 2;
        } else if (field_count == FINGERPRINTS_HEADER) {
            _populate_certificate_field_data(nullptr, FINGERPRINTS_HEADER);
            const EVP_MD *digestSHA256 = EVP_sha256();
            unsigned len1;
            int retVal = X509_digest(m_certificate, digestSHA256,(unsigned char*) sha256, &len1);
            if (retVal == 0 || len1 != SHA256LEN) {
                BROWSER_LOGE("Getting SHA256 cryptographic fingerprint failed %d",len1);
                continue;
            }
            _generate_genlist_data(FINGERPRINTS_SHA_256_FP, BR_STRING_FINGERPRINTS_SHA256, _bin2hex((unsigned char*)sha256, SHA256LEN,' '));
            const EVP_MD *digestSHA1 = EVP_sha1();
            unsigned len2;
            retVal = X509_digest(m_certificate, digestSHA1,(unsigned char*) sha1, &len2);
            if (retVal == 0 || len2 != SHA1LEN) {
                BROWSER_LOGE("Getting SHA1 cryptographic fingerprint failed %d",len2);
                continue;
            }
            _generate_genlist_data(FINGERPRINTS_SHA_1_FP, BR_STRING_FINGERPRINTS_SHA1, _bin2hex((unsigned char*)sha1, SHA1LEN,' '));
            field_count += 2;
        }
    }
}

void CertificateContents::_populate_certificate_field_data(char* data, certificate_field field)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    const char* value = nullptr;
    switch (field) {
        case ISSUED_BY_HEADER:
            _generate_genlist_data(ISSUED_BY_HEADER , BR_STRING_ISSUED_BY_C, strdup(BR_STRING_ISSUED_BY_C));
            //Get Common name
            value = _get_value(strstr(data, "CN="));
            _generate_genlist_data(ISSUED_BY_CN, BR_STRING_COMMON_NAME, value);
            //Get Orgnization
            value = _get_value(strstr(data, "O="));
            _generate_genlist_data(ISSUED_BY_ORG, BR_STRING_ORGANIZATION, value);
            //Get Orgnization UNIT
            value = _get_value(strstr(data, "OU="));
            _generate_genlist_data(ISSUED_BY_ORG_UNIT, BR_STRING_ORGANIZATION_UNIT, value);
            break;

        case ISSUED_TO_HEADER:
            _generate_genlist_data(ISSUED_TO_HEADER , BR_STRING_ISSUED_TO_C, strdup(BR_STRING_ISSUED_TO_C));
            //Get Common name
            value = _get_value(strstr(data, "CN="));
            _generate_genlist_data(ISSUED_TO_CN, BR_STRING_COMMON_NAME, value);
            //Get Orgnization
            value = _get_value(strstr(data, "O="));
            _generate_genlist_data(ISSUED_TO_ORG, BR_STRING_ORGANIZATION, value);
            //Get Orgnization UNIT
            value = _get_value(strstr(data, "OU="));
            _generate_genlist_data(ISSUED_TO_ORG_UNIT, BR_STRING_ORGANIZATION_UNIT, value);
            break;

        case VALIDITY_HEADER:
            _generate_genlist_data(VALIDITY_HEADER , BR_STRING_VALIDITY_C, strdup(BR_STRING_VALIDITY_C));
            break;

        case FINGERPRINTS_HEADER:
            _generate_genlist_data(FINGERPRINTS_HEADER , BR_STRING_FINGERPRINTS, strdup(BR_STRING_FINGERPRINTS));
            break;

        default:
            break;
    }
}

void CertificateContents::_generate_genlist_data(certificate_field field_type, const char* title, const char* value)
{
    if (!value)
        return;
    auto gl_data = std::make_shared<genlist_callback_data>();
    gl_data->type = field_type;
    gl_data->title = title;
    gl_data->value = value;
    m_genlist_callback_data_list.push_back(gl_data);
}

}
}
