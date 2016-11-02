#include "TextPopup_mob.h"
#include "ServiceManager.h"
#include "AbstractMainWindow.h"
#include <stdio.h>
#include <stdlib.h>
#include "Tools/EflTools.h"

namespace tizen_browser
{

namespace base_ui
{
TextPopup* TextPopup::createPopup(Evas_Object* parent)
{
    auto raw_popup = new TextPopup(parent);
    return raw_popup;
}

TextPopup* TextPopup::createPopup(
    Evas_Object* parent,
    const std::string& title,
    const std::string& message)
{
    auto raw_popup = new TextPopup(parent, title, message);
    return raw_popup;
}

TextPopup::~TextPopup()
{
    buttonClicked.disconnect_all_slots();
    for (auto& button : m_buttons) {
        evas_object_smart_callback_del(button.m_evasObject, "clicked", _response_cb);
        evas_object_del(button.m_evasObject);
    }
    evas_object_del(m_popup);
}

TextPopup::TextPopup(Evas_Object* parent)
    : m_parent(parent)
    , m_popup(nullptr)
{
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("SimpleUI/TextPopup.edj");
    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());
}

TextPopup::TextPopup(
    Evas_Object* parent,
    const std::string& title,
    const std::string& message)
    : m_parent(parent)
    , m_popup(nullptr)
    , m_title(title)
    , m_message(message)
{
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("SimpleUI/TextPopup.edj");
    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());
}

void TextPopup::show()
{
    createLayout();
    popupShown(this);
}

void TextPopup::dismiss()
{
    popupDismissed(this);
}

void TextPopup::onBackPressed()
{
    if (m_defaultBackButton != NONE)
        buttonClicked(m_defaultBackButton);
    dismiss();
}

void TextPopup::_response_cb(
    void* data,
    Evas_Object* obj,
    void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        auto self = static_cast<TextPopup*>(data);
        auto it = std::find_if(
            self->m_buttons.begin(),
            self->m_buttons.end(),
            [obj] (const Button& i) {
            return i.m_evasObject == obj;
        });

        (it == self->m_buttons.end()) ?
            BROWSER_LOGW("[%s:%d] Button not found!", __PRETTY_FUNCTION__, __LINE__) :
            self->buttonClicked(it->m_type);

        if (it->m_dismissOnClick)
            self->dismiss();
    } else
        BROWSER_LOGD("data is null ");
}

void TextPopup::setTitle(const std::string& title)
{
    m_title = title;
}

void TextPopup::setMessage(const std::string& message)
{
    m_message = message;
}

void TextPopup::setContent(Evas_Object* content)
{
    elm_object_content_set(m_popup, content);
}

void TextPopup::addButton(
    const PopupButtons& button,
    bool dismissOnClick,
    bool defaultBackButton)
{
    m_buttons.push_back(Button(button, dismissOnClick));
    if (defaultBackButton)
        m_defaultBackButton = button;
}

void TextPopup::createLayout()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_parent);

    m_popup = elm_popup_add(m_parent);
    elm_popup_align_set(m_popup, ELM_NOTIFY_ALIGN_FILL, 1.0);
    evas_object_size_hint_weight_set(m_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

    elm_object_translatable_part_text_set(m_popup, "default", m_message.c_str());
    elm_object_translatable_part_text_set(m_popup, "title,text", m_title.c_str());

    for (size_t i = 0; i < m_buttons.size(); ++i) {
        auto buttonString = std::string("button") + std::to_string(i+1);
        auto btn = elm_button_add(m_popup);
        tools::EflTools::setExpandHints(btn);
        elm_object_text_set(btn, _(buttonsTranslations[m_buttons[i].m_type].c_str()));
        evas_object_smart_callback_add(btn, "clicked", _response_cb, (void*) this);
        elm_object_part_content_set(m_popup, buttonString.c_str(), btn);
        evas_object_show(btn);
        m_buttons[i].m_evasObject = btn;
    }
    evas_object_show(m_popup);
}
}

}
