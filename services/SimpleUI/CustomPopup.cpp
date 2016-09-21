#include "CustomPopup.h"
#include "BrowserLogger.h"

CustomPopup::CustomPopup(Evas_Object* main_layout) :
    m_popup(NULL),
    m_content(NULL),
    m_mainLayout(main_layout)
{

}

CustomPopup::CustomPopup(Evas_Object *main_layout, Evas_Object *content, const char *message, char* title, char* okButtonText, char* cancelButtonText) :
    m_popup(NULL),
    m_mainLayout(main_layout),
    m_content(content),
    m_message(message),
    m_title(title),
    m_okButtonText(okButtonText),
    m_cancelButtonText(cancelButtonText)
{

}

void CustomPopup::setTitle(const std::string& title)
{
    m_title = title;
}

void CustomPopup::setMessage(const std::string& message)
{
    m_message = message;
}

void CustomPopup::setContent(Evas_Object* content)
{
    m_content = content;
}

void CustomPopup::setOkButtonText(const std::string& okButtonText)
{
    m_okButtonText = okButtonText;
}

void CustomPopup::setCancelButtonText(const std::string& cancelButtonText)
{
    m_cancelButtonText = cancelButtonText;
}

void CustomPopup::show()
{
    BROWSER_LOGD("[%s],", __func__);
    std::string edjePath = std::string(EDJE_DIR);
    edjePath.append("SimpleUI/CustomPopup.edj");
    elm_theme_extension_add(0, edjePath.c_str());
    m_popup = elm_layout_add(m_mainLayout);
    if(m_content)
        elm_layout_file_set(m_popup, edjePath.c_str(), "own_popup_long");
    else
        elm_layout_file_set(m_popup, edjePath.c_str(), "own_popup");
    BROWSER_LOGI("PATH: %s", edjePath.c_str());
    elm_object_part_text_set(m_popup, "title_text", m_title.c_str());

    Evas_Object *buttonsBox = elm_box_add(m_popup);
    elm_box_horizontal_set(buttonsBox, EINA_TRUE);

    if (!m_okButtonText.empty())
    {
        BROWSER_LOGD("Button1, %s", edjePath.c_str());
        Evas_Object *btn1 = elm_button_add(buttonsBox);
        evas_object_size_hint_weight_set(btn1, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(btn1, 0.5, 0.5);
        elm_object_style_set(btn1, "standard_button");
        elm_object_part_text_set(btn1, "elm.text", m_okButtonText.c_str());
        elm_box_pack_end(buttonsBox, btn1);
        evas_object_smart_callback_add(btn1, "clicked", popup_ok_cb, (void*)this);
        evas_object_show(btn1);
    }

    if (!m_cancelButtonText.empty())
    {
        BROWSER_LOGD("Button2");
        Evas_Object *btn2 = elm_button_add(buttonsBox);
        evas_object_size_hint_weight_set(btn2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(btn2, 0.5, 0.5);
        elm_object_style_set(btn2, "standard_button");
        elm_object_part_text_set(btn2, "elm.text", m_cancelButtonText.c_str());
        elm_box_pack_end(buttonsBox, btn2);
        evas_object_smart_callback_add(btn2, "clicked", popup_cancel_cb, (void*)this);
        evas_object_show(btn2);
    }

    if(!m_message.empty())
        elm_object_part_text_set(m_popup, "elm.text", m_message.c_str());

    evas_object_show(buttonsBox);
    elm_object_part_content_set(m_popup, "buttons", buttonsBox);
    elm_object_part_content_set(m_popup, "content", m_content);
    elm_object_part_content_set(m_mainLayout, "popup", m_popup);

    elm_object_signal_emit(m_mainLayout, "elm,state,show", "elm");
}

void CustomPopup::hide()
{
    elm_object_signal_emit(m_mainLayout, "elm,state,hide", "elm");
}

void CustomPopup::popup_ok_cb(void *data, Evas_Object *btn, void*)
{
    BROWSER_LOGD("[%s],", __func__);
    CustomPopup *ownPopup = static_cast<CustomPopup*>(data);
    ownPopup->on_ok(ownPopup->m_content);
}
void CustomPopup::popup_cancel_cb(void *data, Evas_Object *btn, void*)
{
    BROWSER_LOGD("[%s],", __func__);
    CustomPopup *ownPopup = static_cast<CustomPopup*>(data);
    ownPopup->on_cancel(ownPopup->m_content);
}

