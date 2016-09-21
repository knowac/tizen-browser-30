#include "BrowserAssert.h"
#include "PasswordUI.h"
#include "BrowserLogger.h"
#include <boost/format.hpp>
#include "app_i18n.h"
#include "EflTools.h"

namespace tizen_browser{
namespace base_ui{

const std::string PasswordUI::PASSWORD_FIELD = "secret_password";
const std::string PasswordUI::DECISION_MADE = "password_decision";
const int PasswordUI::PASSWORD_MINIMUM_CHARACTERS = 6;

PasswordUI::PasswordUI()
    : m_parent(nullptr)
    , m_genlist(nullptr)
    , m_entry(nullptr)
    , m_checkbox(nullptr)
    , m_state(PasswordState::CreatePassword)
{
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("TabUI/PasswordUI.edj");
    createGenlistItemClasses();
}

PasswordUI::~PasswordUI()
{
}

void PasswordUI::showUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_naviframe->getLayout());
    m_naviframe->show();
    changeState(m_state);
}

void PasswordUI::hideUI()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_naviframe->getLayout());
    m_naviframe->hide();

    elm_genlist_clear(m_genlist);
    m_genlistItemData.clear();
}

void PasswordUI::init(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(parent);
    m_parent = parent;
}

Evas_Object* PasswordUI::getContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_parent);
    if (!m_naviframe)
        createLayout();
    return m_naviframe->getLayout();
}

void PasswordUI::createGenlistItemClasses()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_password_item_class = createGenlistItemClass("entry_custom_layout", _genlist_item_text_get,
        _genlist_password_content_get);
    m_checkbox_item_class = createGenlistItemClass("type1", _genlist_item_text_get,
        _genlist_checkbox_content_get);
    m_check_on_of_item_class = createGenlistItemClass("type1", _genlist_item_text_get,
        _genlist_check_on_off_content_get);
    m_text_item_class = createGenlistItemClass("type1", _genlist_item_text_get);
}

Elm_Genlist_Item_Class* PasswordUI::createGenlistItemClass(
    const char* style, Elm_Gen_Item_Text_Get_Cb text_cb, Elm_Gen_Item_Content_Get_Cb content_cb)
{
    auto ic = elm_genlist_item_class_new();
    ic->item_style = style;
    ic->func.text_get = text_cb;
    ic->func.content_get = content_cb;
    ic->func.state_get = nullptr;
    ic->func.del = nullptr;
    ic->decorate_all_item_style = "edit_default";
    return ic;
}

char* PasswordUI::_genlist_item_text_get(void *data, Evas_Object *, const char *part)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data && part) {
        if (!strcmp(part, "elm.text")) {
            PasswordUIData *passwordData = static_cast<PasswordUIData*>(data);
            return strdup(passwordData->text.c_str());
        }
    } else {
        BROWSER_LOGE("[%s:%d] Data or part is null", __PRETTY_FUNCTION__, __LINE__);
    }
    return nullptr;
}

Evas_Object* PasswordUI::_genlist_password_content_get(void *data, Evas_Object *obj, const char *part)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data && part) {
        PasswordUIData *passwordData = static_cast<PasswordUIData*>(data);
        if (!strcmp(part, "elm.swallow.content")) {
            Evas_Object* entry_layout = elm_layout_add(obj);
            elm_layout_theme_set(entry_layout, "layout", "editfield", "multiline");
            tools::EflTools::setExpandHints(entry_layout);

            passwordData->passwordUI->m_entry = elm_entry_add(entry_layout);
            elm_entry_single_line_set(passwordData->passwordUI->m_entry, EINA_TRUE);
            elm_entry_scrollable_set(passwordData->passwordUI->m_entry, EINA_TRUE);
            tools::EflTools::setExpandHints(passwordData->passwordUI->m_entry);
            elm_entry_entry_set(passwordData->passwordUI->m_entry, elm_entry_utf8_to_markup(""));
            elm_entry_password_set(passwordData->passwordUI->m_entry, EINA_TRUE);
            elm_entry_input_panel_return_key_type_set(passwordData->passwordUI->m_entry,
                ELM_INPUT_PANEL_RETURN_KEY_TYPE_DONE);

            evas_object_smart_callback_add(passwordData->passwordUI->m_entry, "focused",
                _entry_focused, entry_layout);
            evas_object_smart_callback_add(passwordData->passwordUI->m_entry, "unfocused",
                _entry_unfocused, entry_layout);
            evas_object_smart_callback_add(passwordData->passwordUI->m_entry, "activated",
                _entry_submited, passwordData->passwordUI);

            elm_object_part_content_set(entry_layout, "elm.swallow.content", passwordData->passwordUI->m_entry);

            return entry_layout;
        }
    } else {
        BROWSER_LOGE("[%s:%d] Data or part is null", __PRETTY_FUNCTION__, __LINE__);
    }
    return nullptr;
}

Evas_Object* PasswordUI::_genlist_checkbox_content_get(void *data, Evas_Object *obj, const char *part)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data && part) {
        PasswordUIData *passwordData = static_cast<PasswordUIData*>(data);
        if (!strcmp(part, "elm.swallow.end")) {
            passwordData->passwordUI->m_checkbox = elm_check_add(obj);
            evas_object_propagate_events_set(passwordData->passwordUI->m_checkbox, EINA_FALSE);
            elm_check_state_set(passwordData->passwordUI->m_checkbox, EINA_FALSE);
            evas_object_smart_callback_add(passwordData->passwordUI->m_checkbox, "changed",
                _show_password_state_changed, passwordData->passwordUI);
            evas_object_show(passwordData->passwordUI->m_checkbox);
            return passwordData->passwordUI->m_checkbox;
        }
    } else {
        BROWSER_LOGE("[%s:%d] Data or part is null", __PRETTY_FUNCTION__, __LINE__);
    }
    return nullptr;
}

Evas_Object* PasswordUI::_genlist_check_on_off_content_get(void *data, Evas_Object *obj, const char *part)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (part) {
        PasswordUIData *passwordData = static_cast<PasswordUIData*>(data);
        if (!strcmp(part, "elm.swallow.end")) {
            auto checkbox = elm_check_add(obj);
            elm_object_style_set(checkbox, "on&off");
            elm_check_state_set(checkbox,
                passwordData->passwordUI->getDBPassword().empty() ? EINA_FALSE : EINA_TRUE);
            evas_object_propagate_events_set(checkbox, EINA_TRUE);
            return checkbox;
        }
    } else {
        BROWSER_LOGE("[%s:%d] Part is null", __PRETTY_FUNCTION__, __LINE__);
    }
    return nullptr;
}

void PasswordUI::createLayout()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_parent);

    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());
    m_naviframe = std::make_unique<NaviframeWrapper>(m_parent);

    m_genlist = elm_genlist_add(m_naviframe->getLayout());
    elm_scroller_policy_set(m_genlist, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
    elm_genlist_multi_select_set(m_genlist, EINA_FALSE);
    elm_genlist_select_mode_set(m_genlist, ELM_OBJECT_SELECT_MODE_ALWAYS);
    elm_genlist_mode_set(m_genlist, ELM_LIST_COMPRESS);
    tools::EflTools::setExpandHints(m_genlist);

    m_naviframe->setContent(m_genlist);
    evas_object_show(m_genlist);

    m_naviframe->addPrevButton(_close_clicked, this);
    m_naviframe->setPrevButtonVisible(true);
}

void PasswordUI::changeState(PasswordState state)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    elm_genlist_clear(m_genlist);
    m_state = state;

    BROWSER_LOGD("___________ %d", m_state);
    if (m_state != PasswordState::SecretModeData) {
        //Add password item
        auto passwordData = std::make_shared<PasswordUIData>();
        m_genlistItemData.push_back(passwordData);
        std::string text = "";

        switch (m_state) {
        case PasswordState::ConfirmPassword:
            text = _(Translations::ConfirmPassword.c_str());
            m_naviframe->setTitle("Confirm password");
            break;
        case PasswordState::IncorrectPassword:
            text = _(Translations::IncorrectPassword.c_str());
            break;
        case PasswordState::CreatePassword:
            text = (boost::format(_(Translations::CreatePassword.c_str()))
                % PASSWORD_MINIMUM_CHARACTERS).str();
            m_naviframe->setTitle("Create password");
            break;
        case PasswordState::ConfirmCreatePassword:
            text = _(Translations::ConfirmCreatePassword.c_str());
            break;
        default:
            break;
        }
        BROWSER_LOGD("___________ %s", text.c_str());
        passwordData->text = text;
        passwordData->passwordUI = this;
        m_password_item = elm_genlist_item_append(m_genlist, m_password_item_class, passwordData.get(),
            nullptr, ELM_GENLIST_ITEM_NONE, nullptr, passwordData.get());

        //Add ShowPassword checkbox
        auto checkboxData = std::make_shared<PasswordUIData>();
        m_genlistItemData.push_back(checkboxData);
        checkboxData->text = _(Translations::ShowPassword.c_str());
        checkboxData->passwordUI = this;
        elm_genlist_item_append(m_genlist, m_checkbox_item_class, checkboxData.get(), nullptr,
            ELM_GENLIST_ITEM_NONE, _show_password_clicked, checkboxData.get());
    } else {
        m_naviframe->setTitle("Secret mode security");
        //Add UsePassword check_on_off
        auto checkOnOffData = std::make_shared<PasswordUIData>();
        m_genlistItemData.push_back(checkOnOffData);
        checkOnOffData->text = _(Translations::UsePassword.c_str());
        checkOnOffData->passwordUI = this;
        elm_genlist_item_append(m_genlist, m_check_on_of_item_class, checkOnOffData.get(), nullptr,
            ELM_GENLIST_ITEM_NONE, _use_password_clicked, checkOnOffData.get());

        //Add ChangePassword
        if (!getDBPassword().empty()) {
            auto changeData = std::make_shared<PasswordUIData>();
            m_genlistItemData.push_back(changeData);
            changeData->text = _(Translations::ChangePassword.c_str());
            changeData->passwordUI = this;
            elm_genlist_item_append(m_genlist, m_text_item_class, changeData.get(), nullptr,
                ELM_GENLIST_ITEM_NONE, _change_password_clicked, changeData.get());
        }
    }
}

std::string PasswordUI::getDBPassword()
{
    auto password = getDBStringParamValue(PASSWORD_FIELD);
    if (password)
        return *password;
    else
        BROWSER_LOGW("[%s:%d] Wrong boost signal value!", __PRETTY_FUNCTION__, __LINE__);
    return std::string();
}

bool PasswordUI::checkIfStringContainsLetter(const std::string& s)
{
    return std::any_of(std::begin(s), std::end(s), ::isalpha);
}

void PasswordUI::_close_clicked(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto self = static_cast<PasswordUI*>(data);
    self->closeUI();
}

void PasswordUI::_entry_focused(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data)
        elm_object_signal_emit((Evas_Object*)data, "elm,state,focused", "");
    else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void PasswordUI::_entry_unfocused(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data)
        elm_object_signal_emit((Evas_Object*)data, "elm,state,unfocused", "");
    else
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
}

void PasswordUI::_entry_submited(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        auto self = static_cast<PasswordUI*>(data);
        std::string hash = std::to_string(
            std::hash<std::string>()(std::string(elm_object_text_get(self->m_entry)))
        );
        switch (self->m_state) {
        case PasswordState::ConfirmPassword:
        case PasswordState::IncorrectPassword: {
            if (self->getDBPassword() == hash)
                switch (self->m_action) {
                case PasswordAction::ChangePassword:
                    self->changeState(PasswordState::CreatePassword);
                    break;
                case PasswordAction::UsePassword:
                    self->setDBStringParamValue(PASSWORD_FIELD, "");
                    self->changeState(PasswordState::SecretModeData);
                    break;
                case PasswordAction::ConfirmPasswordEnterSecret:
                    self->changeEngineState();
                    self->closeUI();
                default:
                    BROWSER_LOGW("Action state is not supported in Password workflow");
                    break;
                }
            else
                self->changeState(PasswordState::IncorrectPassword);
            break; }
        case PasswordState::CreatePassword: {
            std::string s = std::string(elm_object_text_get(self->m_entry));
            if (s.length() < PASSWORD_MINIMUM_CHARACTERS ||
                !checkIfStringContainsLetter(s)) {
                elm_object_text_set(self->m_entry, "");
            } else {
                self->m_not_confirmed_hash = hash;
                self->changeState(PasswordState::ConfirmCreatePassword);
            }
            break; }
        case PasswordState::ConfirmCreatePassword:
            if (hash == self->m_not_confirmed_hash) {
                self->setDBStringParamValue(PASSWORD_FIELD, hash);
                if (self->m_action == PasswordAction::CreatePasswordFirstTime) {
                    self->setDBBoolParamValue(DECISION_MADE, true);
                    self->changeEngineState();
                    self->closeUI();
                } else {
                    self->changeState(PasswordState::SecretModeData);
                }
            } else {
                self->changeState(PasswordState::ConfirmCreatePassword);
            }
            break;
        default:
            BROWSER_LOGW("Password state is not supported in Password workflow");
        }
    } else {
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
    }
}

void PasswordUI::_show_password_state_changed(void* data, Evas_Object*, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        auto self = static_cast<PasswordUI*>(data);
        elm_entry_password_set(self->m_entry, !elm_check_state_get(self->m_checkbox));
    } else {
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
    }
}

void PasswordUI::_show_password_clicked(void *data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        PasswordUI* passwordUI = static_cast<PasswordUI*>(data);
        elm_check_state_set(passwordUI->m_checkbox,
            !elm_check_state_get(passwordUI->m_checkbox));
        elm_entry_password_set(passwordUI->m_entry, !elm_check_state_get(passwordUI->m_checkbox));
    } else {
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
    }
}

void PasswordUI::_use_password_clicked(void *data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        PasswordUIData* passwordData = static_cast<PasswordUIData*>(data);
        passwordData->passwordUI->setAction(PasswordAction::UsePassword);
        if (passwordData->passwordUI->getDBPassword().empty()) {    // start using password
            passwordData->passwordUI->changeState(PasswordState::CreatePassword);
        } else {                                                    // stop using password
            passwordData->passwordUI->changeState(PasswordState::ConfirmPassword);
        }
    } else {
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
    }
}

void PasswordUI::_change_password_clicked(void *data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        PasswordUIData* passwordData = static_cast<PasswordUIData*>(data);
        passwordData->passwordUI->m_action = PasswordAction::ChangePassword;
        passwordData->passwordUI->changeState(PasswordState::ConfirmPassword);
    } else {
        BROWSER_LOGW("[%s] data = nullptr", __PRETTY_FUNCTION__);
    }
}

}
}
