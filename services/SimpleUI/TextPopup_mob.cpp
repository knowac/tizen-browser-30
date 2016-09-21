#include "TextPopup_mob.h"
#include "ServiceManager.h"
#include "AbstractMainWindow.h"

namespace tizen_browser
{

namespace base_ui
{
TextPopup* TextPopup::createPopup(Evas_Object* parent)
{
    TextPopup *raw_popup = new TextPopup(parent);
    return raw_popup;
}

TextPopup* TextPopup::createPopup(Evas_Object* parent,
                                  const std::string& title,
                                  const std::string& message)
{
    TextPopup *raw_popup = new TextPopup(parent, title, message);
    return raw_popup;
}

TextPopup::~TextPopup()
{
    buttonClicked.disconnect_all_slots();
    evas_object_del(m_layout);
}

TextPopup::TextPopup(Evas_Object* parent)
    : m_parent(parent)
    , m_layout(nullptr)
    , m_buttons_box(nullptr)
{
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("SimpleUI/TextPopup.edj");
    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());
}

TextPopup::TextPopup(Evas_Object* parent,
                     const std::string& title,
                     const std::string& message)
    : m_parent(parent)
    , m_layout(nullptr)
    , m_buttons_box(nullptr)
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

void TextPopup::dismiss(){
    popupDismissed(this);
}

void TextPopup::onBackPressed(){
    if (m_defaultBackButton != NONE)
        buttonClicked(m_defaultBackButton);
    dismiss();
}

void TextPopup::_response_cb(void* data,
                                  Evas_Object* obj,
                                  void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto self = static_cast<TextPopup*>(data);
    auto it = std::find_if(self->m_buttons.begin(),
                        self->m_buttons.end(),
                        [obj] (const Button& i)
                        { return i.m_evasObject == obj; }
                        );

    if (it == self->m_buttons.end()) {
        BROWSER_LOGW("[%s:%d] Button not found!", __PRETTY_FUNCTION__, __LINE__);
    } else {
        self->buttonClicked(it->m_type);
    }
    if (it->m_dismissOnClick)
        self->dismiss();
}

void TextPopup::setTitle(const std::string& title)
{
    this->m_title = title;
}

void TextPopup::setMessage(const std::string& message)
{
    this->m_message = message;
}

void TextPopup::setContent(Evas_Object* content)
{
    elm_object_part_content_set(m_layout, "content_swallow", content);
}

void TextPopup::addButton(const PopupButtons& button, bool dismissOnClick, bool defaultBackButton)
{
    m_buttons.push_back(Button(button, dismissOnClick));
    if (defaultBackButton)
        m_defaultBackButton = button;
}

void TextPopup::createLayout()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    M_ASSERT(m_parent);

    m_layout = elm_layout_add(m_parent);
    elm_object_tree_focus_allow_set(m_layout, EINA_FALSE);
    elm_layout_file_set(m_layout, m_edjFilePath.c_str(), "text_popup_layout");
    evas_object_size_hint_weight_set(m_layout, EVAS_HINT_EXPAND, 0);
    evas_object_size_hint_align_set(m_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    elm_object_translatable_part_text_set(m_layout, "title_text", m_title.c_str());
    elm_object_translatable_text_set(m_layout, m_message.c_str());

    m_buttons_box = elm_box_add(m_layout);
    elm_box_horizontal_set(m_buttons_box, EINA_TRUE);
    evas_object_size_hint_weight_set(m_buttons_box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_buttons_box, EVAS_HINT_FILL, EVAS_HINT_FILL);

    for (size_t i=0; i < m_buttons.size(); ++i) {
        auto btn = elm_button_add(m_buttons_box);
        m_buttons[i].m_evasObject = btn;
        elm_object_style_set(btn, "text-popup-button");
        evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, EVAS_HINT_FILL);
        elm_object_part_text_set(btn, "elm.text", _(buttonsTranslations[m_buttons[i].m_type].c_str()));
        elm_box_pack_end(m_buttons_box, btn);
        evas_object_smart_callback_add(btn, "clicked", _response_cb, (void*) this);

        evas_object_show(btn);
        if (i > 0)      // separator for more buttons than one
            elm_object_signal_emit(btn, "visible", "ui");
    }

    evas_object_show(m_buttons_box);
    elm_object_part_content_set(m_layout, "buttons_swallow", m_buttons_box);

    evas_object_show(m_layout);
    elm_object_part_content_set(m_parent, "popup_content", m_layout);
}
}

}
