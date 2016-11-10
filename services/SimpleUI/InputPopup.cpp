#include "InputPopup.h"
#include "Tools/EflTools.h"

namespace tizen_browser {
namespace base_ui {

InputPopup::InputPopup() :
    m_parent(nullptr),
    m_popup(nullptr),
    m_icon(nullptr),
    m_button_left(nullptr),
    m_button_right(nullptr),
    m_entry(nullptr),
    m_box(nullptr),
    m_label(nullptr)
{
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("SimpleUI/InputPopup.edj");
    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());
    m_bad_words.push_back("");
}

InputPopup::~InputPopup()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    if (m_icon) {
        evas_object_smart_callback_del(m_icon, "clicked", _input_cancel_clicked);
        evas_object_del(m_icon);
    }
    if (m_entry) {
        evas_object_smart_callback_del(m_entry, "changed,user", _entry_changed);
        evas_object_del(m_entry);
    }
    if (m_button_right) {
        evas_object_smart_callback_del(m_button_right, "clicked", _right_button_clicked);
        evas_object_del(m_button_right);
    }
    if (m_button_left) {
        evas_object_smart_callback_del(m_button_left, "clicked", _left_button_clicked);
        evas_object_del(m_button_left);
    }
    if (m_label)
        evas_object_del(m_label);
    if (m_box)
        evas_object_del(m_box);
    if (m_popup)
        evas_object_del(m_popup);

    button_clicked.disconnect_all_slots();
    popupDismissed.disconnect_all_slots();
    popupShown.disconnect_all_slots();
    m_bad_words.clear();
}

InputPopup* InputPopup::createPopup(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto inputPopup = new InputPopup();
    inputPopup->m_parent = parent;
    return inputPopup;
}

InputPopup* InputPopup::createPopup(
    Evas_Object *parent,
    const std::string& title,
    const std::string& message,
    const std::string& input,
    const std::string& rightButtonText,
    const std::string& leftButtonText)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    auto inputPopup = new InputPopup();
    inputPopup->m_parent = parent;
    inputPopup->m_title = title;
    inputPopup->m_message = message;
    inputPopup->m_input = input;
    inputPopup->m_ok_button_text = rightButtonText;
    inputPopup->m_cancel_button_text = leftButtonText;
    return inputPopup;
}

void InputPopup::setInput(const std::string& input)
{
    m_input = input;
}

void InputPopup::setTitle(const std::string& title)
{
    m_title = title;
}

void InputPopup::setMessage(const std::string& message)
{
    m_message = message;
}

void InputPopup::setTip(const std::string& tip)
{
    m_tip = tip;
}

void InputPopup::setOkButtonText(const std::string& okButtonText)
{
    m_ok_button_text = okButtonText;
}

void InputPopup::setCancelButtonText(const std::string& cancelButtonText)
{
    m_cancel_button_text = cancelButtonText;
}

void InputPopup::addBadWord(const std::string &word)
{
    m_bad_words.push_back(word);
}

void InputPopup::show()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    createLayout();
    popupShown(this);

    elm_object_disabled_set(
        m_button_right,
        std::find(
            m_bad_words.begin(),
            m_bad_words.end(),
            elm_entry_utf8_to_markup(m_input.c_str())) != m_bad_words.end());
}

void InputPopup::dismiss()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    popupDismissed(this);
}

void InputPopup::onBackPressed()
{
    dismiss();
}

void InputPopup::createLayout()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    m_popup = elm_popup_add(m_parent);
    elm_popup_align_set(m_popup, ELM_NOTIFY_ALIGN_FILL, 1.0);
    evas_object_size_hint_weight_set(m_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

    elm_object_translatable_part_text_set(m_popup, "default", m_message.c_str());
    elm_object_translatable_part_text_set(m_popup, "title,text", m_title.c_str());

    m_box = elm_box_add(m_popup);
    tools::EflTools::setExpandHints(m_box);
    elm_object_content_set(m_popup, m_box);

    if (!m_message.empty()) {
        m_label = elm_entry_add(m_box);
        elm_entry_editable_set(m_label, EINA_FALSE);
        elm_entry_single_line_set(m_label, EINA_TRUE);
        elm_entry_scrollable_set(m_label, EINA_FALSE);
        elm_entry_input_panel_layout_set(m_entry, ELM_INPUT_PANEL_LAYOUT_NORMAL);
        elm_object_text_set(m_label, elm_entry_utf8_to_markup(m_message.c_str()));
        elm_box_pack_end(m_box, m_label);
        evas_object_show(m_label);
    }

    m_entry = elm_entry_add(m_box);
    tools::EflTools::setExpandHints(m_entry);
    elm_entry_single_line_set(m_entry, EINA_TRUE);
    elm_entry_scrollable_set(m_entry, EINA_TRUE);
    elm_entry_input_panel_layout_set(m_entry, ELM_INPUT_PANEL_LAYOUT_NORMAL);
    elm_box_pack_end(m_box, m_entry);

    m_icon = elm_icon_add(m_entry);
    elm_icon_standard_set(m_icon, "close");
    evas_object_size_hint_min_set(
        m_icon,
        ELM_SCALE_SIZE(40),
        ELM_SCALE_SIZE(40));
    evas_object_size_hint_max_set(
        m_icon,
        ELM_SCALE_SIZE(40),
        ELM_SCALE_SIZE(40));
    elm_object_part_content_set(m_entry, "end", m_icon);

    elm_object_part_text_set(m_entry, "elm.text", elm_entry_utf8_to_markup(m_input.c_str()));
    elm_object_part_text_set(m_entry, "elm.guide", elm_entry_utf8_to_markup(m_tip.c_str()));

    m_button_left = elm_button_add(m_popup);
    tools::EflTools::setExpandHints(m_button_left);
    elm_object_text_set(m_button_left, m_cancel_button_text.c_str());
    elm_object_part_content_set(m_popup, "button1", m_button_left);

    m_button_right = elm_button_add(m_popup);
    tools::EflTools::setExpandHints(m_button_right);
    elm_object_text_set(m_button_right, m_ok_button_text.c_str());
    elm_object_part_content_set(m_popup, "button2", m_button_right);

    evas_object_smart_callback_add(m_entry, "changed,user", _entry_changed, this);
    evas_object_smart_callback_add(m_icon, "clicked", _input_cancel_clicked, this);
    evas_object_smart_callback_add(m_button_left, "clicked", _left_button_clicked, this);
    evas_object_smart_callback_add(m_button_right, "clicked", _right_button_clicked, this);

    evas_object_show(m_box);
    evas_object_show(m_icon);
    evas_object_show(m_entry);
    evas_object_show(m_button_left);
    evas_object_show(m_button_right);
    evas_object_show(m_popup);
}

void InputPopup::_entry_changed(void* data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        auto inputPopup = static_cast<InputPopup*>(data);
        std::string text =
            elm_entry_markup_to_utf8(
                elm_object_part_text_get(inputPopup->m_entry, "elm.text"));

        elm_object_disabled_set(
            inputPopup->m_button_right,
            std::find(
                inputPopup->m_bad_words.begin(),
                inputPopup->m_bad_words.end(),
                text) != inputPopup->m_bad_words.end());
    } else
        BROWSER_LOGD("data is null ");
}

void InputPopup::_input_cancel_clicked(void* data, Evas_Object*, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        auto inputPopup = static_cast<InputPopup*>(data);
        elm_object_part_text_set(inputPopup->m_entry, "elm.text", "");
        elm_object_disabled_set(inputPopup->m_button_right, EINA_TRUE);
    } else
        BROWSER_LOGD("data is null ");
}

void InputPopup::_right_button_clicked(void *data, Evas_Object *, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        auto inputPopup = static_cast<InputPopup*>(data);
        inputPopup->button_clicked(
            elm_entry_markup_to_utf8(
                elm_object_part_text_get(inputPopup->m_entry, "elm.text")));
        inputPopup->dismiss();
    } else
        BROWSER_LOGD("data is null ");
}

void InputPopup::_left_button_clicked(void* data, Evas_Object *, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data) {
        auto inputPopup = static_cast<InputPopup*>(data);
        inputPopup->dismiss();
    } else
        BROWSER_LOGD("data is null ");
}

}
}
