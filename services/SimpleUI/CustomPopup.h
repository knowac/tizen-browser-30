#ifndef CUSTOMPOPUP_H
#define CUSTOMPOPUP_H

#include <Elementary.h>
#include <boost/signals2.hpp>

class CustomPopup{

public:
    CustomPopup(Evas_Object *main_layout);
    CustomPopup(Evas_Object *main_layout, Evas_Object *content, const char *message, char *title, char* okButtonText, char* cancelButtonText);

    /**
     * Theese setters should be called before showing popup.
     */
    void setContent(Evas_Object *content);
    void setMessage(const std::string &message);
    void setTitle(const std::string &title);
    void setOkButtonText(const std::string &okButtonText);
    void setCancelButtonText(const std::string &cancelButtonText);

    void show();
    void hide();
    boost::signals2::signal<void (Evas_Object *)> on_ok;
    boost::signals2::signal<void (Evas_Object *)> on_cancel;

private:
    static void popup_ok_cb(void *data, Evas_Object *btn, void*);
    static void popup_cancel_cb(void *data, Evas_Object *btn, void*);
    Evas_Object *m_popup;
    Evas_Object *m_content;
    std::string m_message, m_title, m_okButtonText, m_cancelButtonText;

protected:
    Evas_Object *m_mainLayout;
};

#endif
