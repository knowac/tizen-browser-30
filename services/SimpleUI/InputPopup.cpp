#include "InputPopup.h"

namespace tizen_browser {
namespace base_ui {

InputPopup::InputPopup() :
    m_parent(nullptr),
    m_layout(nullptr),
    m_buttons_box(nullptr),
    m_button_left(nullptr),
    m_button_right(nullptr),
    m_input_area(nullptr),
    m_input_cancel(nullptr),
    m_entry(nullptr),
    m_accept_right_left(true)
{
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("SimpleUI/InputPopup.edj");
    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());
    m_bad_words.push_back("");
}

InputPopup::~InputPopup()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    evas_object_smart_callback_del(m_entry, "focused", _entry_focused);
    evas_object_smart_callback_del(m_entry, "unfocused", _entry_unfocused);
    evas_object_smart_callback_del(m_entry, "changed,user", _entry_changed);
    evas_object_smart_callback_del(m_input_cancel, "clicked", _input_cancel_clicked);
    evas_object_smart_callback_del(m_button_right, "clicked", _right_button_clicked);
    evas_object_smart_callback_del(m_button_left, "clicked", _left_button_clicked);
    evas_object_del(m_input_cancel);
    evas_object_del(m_entry);
    evas_object_del(m_input_area);
    evas_object_del(m_button_right);
    evas_object_del(m_button_left);
    evas_object_del(m_buttons_box);
    evas_object_del(m_layout);
    button_clicked.disconnect_all_slots();
    popupDismissed.disconnect_all_slots();
    popupShown.disconnect_all_slots();
    m_bad_words.clear();
}

InputPopup* InputPopup::createPopup(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    InputPopup *inputPopup = new InputPopup();
    inputPopup->m_parent = parent;
    return inputPopup;
}

InputPopup* InputPopup::createPopup(Evas_Object *parent, const std::string& title, const std::string& message, const std::string& input,
                                         const std::string& rightButtonText, const std::string& leftButtonText, bool accept_right_left)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    InputPopup *inputPopup = new InputPopup();
    inputPopup->m_parent = parent;
    inputPopup->m_title = title;
    inputPopup->m_message = message;
    inputPopup->m_input = input;
    inputPopup->m_ok_button_text = rightButtonText;
    inputPopup->m_cancel_button_text = leftButtonText;
    inputPopup->m_accept_right_left = accept_right_left;
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

void InputPopup::setAcceptRightLeft(bool right_left)
{
    m_accept_right_left = right_left;
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

    m_layout = elm_layout_add(m_parent);
    elm_layout_file_set(m_layout, m_edjFilePath.c_str(), "input-popup-layout");
    evas_object_size_hint_weight_set(m_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_object_part_text_set(m_layout, "title_text", m_title.c_str());

    m_input_area = elm_layout_add(m_layout);
    elm_object_part_content_set(m_layout, "input_swallow", m_input_area);

    evas_object_size_hint_weight_set(m_input_area, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_input_area, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(m_input_area);

    elm_layout_file_set(m_input_area, m_edjFilePath.c_str(), "input-area-layout");
    elm_object_part_text_set(m_input_area, "input_message_text", m_message.c_str());

    m_entry = elm_entry_add(m_input_area);
    elm_object_style_set(m_entry, "popup-input-entry");

    elm_entry_single_line_set(m_entry, EINA_TRUE);
    elm_entry_scrollable_set(m_entry, EINA_TRUE);
    elm_entry_input_panel_layout_set(m_entry, ELM_INPUT_PANEL_LAYOUT_URL);
    elm_object_part_content_set(m_input_area, "input_text_swallow", m_entry);
    elm_object_part_text_set(m_entry, "elm.text", elm_entry_utf8_to_markup(m_input.c_str()));
    elm_object_part_text_set(m_entry, "elm.guide", elm_entry_utf8_to_markup(m_tip.c_str()));

    evas_object_smart_callback_add(m_entry, "focused", _entry_focused, (void*)this);
    evas_object_smart_callback_add(m_entry, "unfocused", _entry_unfocused, (void*)this);
    evas_object_smart_callback_add(m_entry, "changed,user", _entry_changed, (void*)this);

    m_input_cancel = elm_button_add(m_input_area);
    elm_object_style_set(m_input_cancel, "invisible_button");
    evas_object_smart_callback_add(m_input_cancel, "clicked", _input_cancel_clicked, this);

    evas_object_show(m_input_cancel);
    elm_object_part_content_set(m_input_area, "input_cancel_click", m_input_cancel);

    m_buttons_box = elm_box_add(m_layout);
    elm_box_horizontal_set(m_buttons_box, EINA_TRUE);
    evas_object_size_hint_weight_set(m_buttons_box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_buttons_box, EVAS_HINT_FILL, EVAS_HINT_FILL);

    m_button_left = elm_button_add(m_buttons_box);
    elm_object_style_set(m_button_left, "input-popup-button");
    evas_object_size_hint_weight_set(m_button_left, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_button_left, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_object_part_text_set(m_button_left, "elm.text", m_cancel_button_text.c_str());
    elm_box_pack_end(m_buttons_box, m_button_left);
    evas_object_smart_callback_add(m_button_left, "clicked", _left_button_clicked, (void*)this);

    evas_object_show(m_button_left);

    m_button_right = elm_button_add(m_buttons_box);
    elm_object_style_set(m_button_right, "input-popup-button");
    evas_object_size_hint_weight_set(m_button_right, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(m_button_right, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_object_part_text_set(m_button_right, "elm.text", m_ok_button_text.c_str());
    elm_box_pack_end(m_buttons_box, m_button_right);
    evas_object_smart_callback_add(m_button_right, "clicked", _right_button_clicked, (void*)this);

    evas_object_show(m_button_right);
    elm_object_signal_emit(m_button_right, "visible", "ui");

    evas_object_show(m_buttons_box);
    elm_object_part_content_set(m_layout, "buttons_swallow", m_buttons_box);

    evas_object_show(m_layout);
    elm_object_part_content_set(m_parent, "popup_content", m_layout);

    if (std::find(m_bad_words.begin(), m_bad_words.end(), m_input) != m_bad_words.end()) {
        elm_object_disabled_set(m_accept_right_left ? m_button_right : m_button_left, EINA_TRUE);
        elm_object_signal_emit(m_accept_right_left ? m_button_right : m_button_left, "dissabled", "ui");
    }

    elm_object_signal_emit(m_input_area, "close_icon_show", "ui");
}

void InputPopup::_entry_focused(void* data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        InputPopup*  inputPopup = static_cast<InputPopup*>(data);
        elm_object_focus_allow_set(inputPopup->m_input_cancel, EINA_TRUE);
        elm_object_signal_emit(inputPopup->m_entry, "focused", "ui");
    }
}

void InputPopup::_entry_unfocused(void* data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        InputPopup*  inputPopup = static_cast<InputPopup*>(data);
        elm_object_focus_allow_set(inputPopup->m_input_cancel, EINA_FALSE);
        elm_object_signal_emit(inputPopup->m_entry, "unfocused", "ui");
    }
}

void InputPopup::_entry_changed(void* data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        InputPopup*  inputPopup = static_cast<InputPopup*>(data);
        std::string text = elm_entry_markup_to_utf8(elm_object_part_text_get(inputPopup->m_entry, "elm.text"));

        if (text.empty())
            elm_object_signal_emit(inputPopup->m_input_area, "close_icon_hide", "ui");
        else
            elm_object_signal_emit(inputPopup->m_input_area, "close_icon_show", "ui");

        if (std::find(inputPopup->m_bad_words.begin(), inputPopup->m_bad_words.end(), text)
                != inputPopup->m_bad_words.end()) {
            elm_object_disabled_set(inputPopup->m_accept_right_left ? inputPopup->m_button_right :
                                                          inputPopup->m_button_left, EINA_TRUE);
            elm_object_signal_emit(inputPopup->m_accept_right_left ? inputPopup->m_button_right :
                                                         inputPopup->m_button_left, "dissabled", "ui");
        } else {
            elm_object_disabled_set(inputPopup->m_accept_right_left ? inputPopup->m_button_right :
                                                          inputPopup->m_button_left, EINA_FALSE);
            elm_object_signal_emit(inputPopup->m_accept_right_left ? inputPopup->m_button_right :
                                                         inputPopup->m_button_left, "enabled", "ui");
        }
    }
}

void InputPopup::_input_cancel_clicked(void * data, Evas_Object *, void *)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (data != nullptr) {
        InputPopup*  inputPopup = static_cast<InputPopup*>(data);
        elm_object_part_text_set(inputPopup->m_entry, "elm.text", "");
        elm_object_disabled_set(inputPopup->m_accept_right_left ? inputPopup->m_button_right :
                                                      inputPopup->m_button_left, EINA_TRUE);
        elm_object_signal_emit(inputPopup->m_accept_right_left ? inputPopup->m_button_right :
                                                     inputPopup->m_button_left, "dissabled", "ui");
        elm_object_signal_emit(inputPopup->m_input_area, "close_icon_hide", "ui");
    }
}

void InputPopup::_right_button_clicked(void *data, Evas_Object *, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    InputPopup *inputPopup = static_cast<InputPopup*>(data);
    if (inputPopup->m_accept_right_left)
        inputPopup->button_clicked(elm_entry_markup_to_utf8(elm_object_part_text_get(inputPopup->m_entry, "elm.text")));
    inputPopup->dismiss();
}

void InputPopup::_left_button_clicked(void* data, Evas_Object *, void*)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    InputPopup *inputPopup = static_cast<InputPopup*>(data);
    if (!inputPopup->m_accept_right_left)
        inputPopup->button_clicked(elm_entry_markup_to_utf8(elm_object_part_text_get(inputPopup->m_entry, "elm.text")));
    inputPopup->dismiss();
}

}
}
