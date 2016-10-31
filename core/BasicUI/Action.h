/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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

#ifndef ACTION_H
#define ACTION_H
#include <boost/signals2.hpp>
#include <string>


namespace tizen_browser
{
namespace base_ui
{

    /**
     * \brief Action provides an abstract user interface that can be inserted into toolbars or menubars.
     *
     * This is a support class that is delivered for convenience.
     * SE doesn't have to file all the fields, required are: text, and icon.
     *
     * By default icon is enabled and icon is displayed in menu but is not checkable.
     *
     * sugested ussage:
     * ~~~.cpp
     * //---------MyService.h---------
     * class BROWSER_EXPORT MyService: public tizen_browser::core::AbsractService
     * {
     * public:
     *  ...
     *     void showConfigWindow();
     *     void showMainWindow();
     *     void doTheJob();
     *     Action showConfigAction;
     *     Action showMainWindowAction;
     *     Action doTheJobAction;
     * }
     * //------- MyService.cpp---------
     * MyService::MyService()
     *      :showConfigAction("Configure", "path/to/icon/config.png")
     *      ,showMainWindowAction("My Service", "path/to/logo.png)
     *      ,doTheJobAction("Crypt content", "path/to/crypt.png)
     * {
     *      showConfigAction.triggered.connect(MyService::showConfigWindow, this);
     *      showMainWindowAction.triggered.connect(MyService::showMainWindow, this);
     *      doTheJobAction.triggered.connect(MyService::doTheJob, this);
     * }
     *
     * //------- using MyService actions---------
     *
     * void someUi::prepareUi{
     *      settingsMenu = new Menu("Settings");
     *      settingsMenu->addAction(myService.showConfigAction);
     *
     *      actionMenu = new Menu("Actions");
     *      actionMenu->addAction(myService.doTheJobAction);
     *
     *      toolsMenu = new Menu("Tools");
     *      toolsMenu->addAction(myService.showMainWindowAction);
     *
     *      iconBar = new IconBar();
     *      iconBar->addAction(myService.showConfigAction);
     *      iconBar->addAction(myService.doTheJobAction);
     *      iconBar->addAction(myService.showMainWindowAction)
     * }
     *
     * ~~~
     */
class Action
{
public:
    Action();
    Action(const std::string& text);
    Action(const std::string& icon, const std::string& text);

    Action(const Action& other);
    ~Action();
    std::string getText()const;
    std::string getStatusTip()const;
    std::string getToolTip()const;
    std::string getIcon()const;
    std::string getSelIcon()const;
    std::string getDisIcon()const;

    void setIconText(const std::string& iconText);
    void setText(const std::string& text);
    void setStatusTip(const std::string& statusTip);
    void setToolTip(const std::string& toolTip);
    void setIcon(const std::string& icon);
    void setSelIcon(const std::string& selIcon);
    void setDisIcon(const std::string& disIcon);

    bool isEnabled()const;
    bool isCheckable()const;
    bool isChecked()const;
    bool isIconVisibleInMenu()const;

    void setEnabled(bool enabled);
    void setCheckable(bool checkable);
    void setChecked(bool checked);
    void setIconVisibleInMenu(bool visible);

    /**
     * @brief Convenience function, switch state of checkable Actions.
     *
     */
    void toggle();

    /**
     * @brief Convenience function, manually call \see triggered() signal
     *
     */
    void trigger();

    /**
     * \brief Signale is emitted when the given action is activated by user.
     *
     * Action may be activated when user click a menu option, boolbar button
     */
    boost::signals2::signal<void ()> triggered;


    /**
     * @brief Signal is emitted if Action 'checked' state is changed.
     *
     * This signal is valid only for checkable actions
     */
    boost::signals2::signal<void (bool checked)> toggled;

    /**
     * @brief Signal is emitted if Action 'enabled' state is changed.
     */
    boost::signals2::signal<void ()> enabledChanged;

private:
    std::string m_text;     ///< Text displayed in menu.
    std::string m_statusTip;///< Text displayed in status bar.
    std::string m_toolTip;  ///< Text displayed "on mouse over" in tool tip box.
    std::string m_icon;     ///< Path to icon path.
    std::string m_selIcon;  ///< Path to selected (mouse over) icon.
    std::string m_disIcon;  ///< Path to disabled icon.
    bool m_enabled;  ///< Is action enabled.
    bool m_checkable; ///< Is action checkable.
    bool m_checked; ///< Is action checked, only checkable Actions can be checked. By default is false.
    bool m_iconVisibleInMenu; ///<Shoud icon be displayed in menu.
};

typedef std::shared_ptr<Action> sharedAction;

} /* end of namespace base_ui */
} /* end of namespace tizen_browser */

#endif // ACTION_H
