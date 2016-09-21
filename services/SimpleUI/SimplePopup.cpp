#include "SimplePopup.h"
#include "ServiceManager.h"

namespace tizen_browser {
namespace base_ui {

SimplePopup* SimplePopup::createPopup(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    SimplePopup *raw_popup = new SimplePopup(parent);
    return raw_popup;
}

SimplePopup* SimplePopup::createPopup(Evas_Object* parent, const std::string &title, const std::string &message)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    SimplePopup *raw_popup = new SimplePopup(parent, title, message);
    return raw_popup;
}

SimplePopup::~SimplePopup()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    for (std::map<Evas_Object*, PopupButtons>::iterator it = addedButtons.begin(); it != addedButtons.end(); ++it) {
        evas_object_smart_callback_del(it->first, "clicked", _response_cb);
    }
    evas_object_del(content);
    evas_object_del(popup);
    buttonClicked.disconnect_all_slots();
    popupDismissed.disconnect_all_slots();
    popupShown.disconnect_all_slots();
}

SimplePopup::SimplePopup(Evas_Object* parent) : m_parent(parent), popup(nullptr), content(nullptr)
{ }

SimplePopup::SimplePopup(Evas_Object* parent, const std::string &title, const std::string &message)
    : m_parent(parent)
    , popup(nullptr)
    , content(nullptr)
    , title(title)
    , message(message)
{ }

void SimplePopup::show()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    if (!popup) {
       popup = elm_popup_add(m_parent);
    }
    evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

    if (content != nullptr)
       elm_object_content_set(popup, content);
    else
       elm_object_text_set(popup, message.c_str());

    elm_popup_content_text_wrap_type_set(popup, ELM_WRAP_WORD);
    elm_object_part_text_set(popup, "title,text", title.c_str());

    int buttonsCounter = 1;
    for(std::list<PopupButtons>::iterator it = buttons.begin(); it != buttons.end(); ++it)
    {
        Evas_Object *btn1 = elm_button_add(popup);
        elm_object_text_set(btn1, buttonsTranslations[*it].c_str());
        std::string buttonName = "button";
        buttonName.append(std::to_string(buttonsCounter));
        elm_object_part_content_set(popup, buttonName.c_str(), btn1);
        addedButtons[btn1] = *it;
        evas_object_smart_callback_add(btn1, "clicked", _response_cb, this);
        ++buttonsCounter;
    }

    evas_object_show(popup);
    popupShown(this);
}

void SimplePopup::dismiss()
{
    popupDismissed(this);
}

void SimplePopup::onBackPressed()
{
    dismiss();
}

void SimplePopup::_response_cb(void *data, Evas_Object *obj, void */*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    SimplePopup *self = static_cast<SimplePopup*>(data);
    self->buttonClicked(self->addedButtons[obj], self->popupData);
    self->dismiss();
}

void SimplePopup::setTitle(const std::string &title)
{
    this->title = title;
}

void SimplePopup::setMessage(const std::string &message)
{
    this->message = message;
}

void SimplePopup::setContent(Evas_Object* content)
{
    this->content = content;
}

void SimplePopup::setData(std::shared_ptr< PopupData > popupData)
{
    this->popupData = popupData;
}

void SimplePopup::addButton(PopupButtons buttonId)
{
    buttons.push_back(buttonId);
}

} // base_ui
} // tizen_browser
