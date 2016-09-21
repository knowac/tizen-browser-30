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

#include <string>

#include <boost/signals2.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/concept_check.hpp>

#include "Action.h"
#include "BrowserLogger.h"


BOOST_AUTO_TEST_SUITE(action)

BOOST_AUTO_TEST_CASE(action_constructors)
{
    BROWSER_LOGI("[UT] Action - action_constructor - START --> ");

    tizen_browser::base_ui::Action action_01;
    BOOST_CHECK_EQUAL(action_01.isEnabled(), true);
    BOOST_CHECK_EQUAL(action_01.isCheckable(), false);
    BOOST_CHECK_EQUAL(action_01.isChecked(), false);
    BOOST_CHECK_EQUAL(action_01.isIconVisibleInMenu(), true);

    std::string a02_text("test02");
    tizen_browser::base_ui::Action  action_02(a02_text);
    BOOST_CHECK_EQUAL(action_02.isEnabled(), true);
    BOOST_CHECK_EQUAL(action_02.isCheckable(), false);
    BOOST_CHECK_EQUAL(action_02.isChecked(), false);
    BOOST_CHECK_EQUAL(action_02.isIconVisibleInMenu(), true);
    BOOST_CHECK_EQUAL(action_02.getText(), a02_text);

    std::string a03_text("test02");
    tizen_browser::base_ui::Action  action_03(a03_text);
    BOOST_CHECK_EQUAL(action_03.isEnabled(), true);
    BOOST_CHECK_EQUAL(action_03.isCheckable(), false);
    BOOST_CHECK_EQUAL(action_03.isChecked(), false);
    BOOST_CHECK_EQUAL(action_03.isIconVisibleInMenu(), true);
    BOOST_CHECK_EQUAL(action_03.getText(), a03_text);

    std::string a04_text("test04");
    std::string a04_icon("test04Icon");
    tizen_browser::base_ui::Action  action_04(a04_icon, a04_text);
    BOOST_CHECK_EQUAL(action_04.isEnabled(), true);
    BOOST_CHECK_EQUAL(action_04.isCheckable(), false);
    BOOST_CHECK_EQUAL(action_04.isChecked(), false);
    BOOST_CHECK_EQUAL(action_04.isIconVisibleInMenu(), true);
    BOOST_CHECK_EQUAL(action_04.getText(), a04_text);
    BOOST_CHECK_EQUAL(action_04.getIcon(), a04_icon);

    BROWSER_LOGI("[UT] --> END - Action - action_constructor");
}

BOOST_AUTO_TEST_CASE(action_get_and_set){

    BROWSER_LOGI("[UT] Action - action_get_and_set - START --> ");

    std::string iconText("iconText");
    std::string text("text");
    std::string statusTip("statusTip");
    std::string toolTip("toolTip");
    std::string icon("icon");
    std::string selIcon("selIcon");
    std::string disIcon("disabledIcon");

    tizen_browser::base_ui::Action action;

    action.setIconText(iconText);
    action.setText(text);
    action.setStatusTip(statusTip);
    action.setToolTip(toolTip);
    action.setIcon(icon);
    action.setSelIcon(selIcon);
    action.setDisIcon(disIcon);

    BOOST_CHECK_EQUAL(action.getIconText(), iconText);
    BOOST_CHECK_EQUAL(action.getText(), text);
    BOOST_CHECK_EQUAL(action.getStatusTip(), statusTip);
    BOOST_CHECK_EQUAL(action.getToolTip(), toolTip);
    BOOST_CHECK_EQUAL(action.getIcon(), icon);
    BOOST_CHECK_EQUAL(action.getSelIcon(), selIcon);
    BOOST_CHECK_EQUAL(action.getDisIcon(), disIcon);

    BROWSER_LOGI("[UT] --> END - Action - action_get_and_set");
}

BOOST_AUTO_TEST_CASE(action_bool_behaviour){

    BROWSER_LOGI("[UT] Action - action_bool_behaviour - START --> ");

    tizen_browser::base_ui::Action action_01;
    //action is not checkable by defalut,
    //this call should be ignored.
    action_01.setChecked(true);
    BOOST_CHECK_EQUAL(action_01.isCheckable(), false);
    BOOST_CHECK_EQUAL(action_01.isChecked(), false);

    tizen_browser::base_ui::Action action_02;
    action_02.setCheckable(true);
    action_02.setChecked(true);
    BOOST_CHECK_EQUAL(action_02.isCheckable(), true);
    BOOST_CHECK_EQUAL(action_02.isChecked(), true);

    //toggle test
    action_02.toggle();
    BOOST_CHECK_EQUAL(action_02.isChecked(), false);

    BROWSER_LOGI("[UT] --> END - Action - action_bool_behaviour");
}

BOOST_AUTO_TEST_CASE(action_trigger_test){

    BROWSER_LOGI("[UT] Action - action_trigger_test - START --> ");

    struct TriggerHandler{
        TriggerHandler()
            :beenCalled(false){};
        ~TriggerHandler(){
            BOOST_CHECK_EQUAL(beenCalled, true);
        };
        void operator()(){
            beenCalled = true;
        };
//     private:
        bool beenCalled;
    };


    TriggerHandler triggered;
    tizen_browser::base_ui::Action action_01;
    action_01.triggered.connect(boost::ref(triggered));
    action_01.trigger();
    BOOST_CHECK_EQUAL(triggered.beenCalled, true);

    BROWSER_LOGI("[UT] --> END - Action - action_trigger_test");
}

BOOST_AUTO_TEST_CASE(action_togle_test){

    BROWSER_LOGI("[UT] Action - action_togle_test - START --> ");

    struct ToggleHandler{
        ToggleHandler()
            :isChecked(false),beenCalled(false){};
        ~ToggleHandler(){
        };
        void operator()(bool checked){
            isChecked = checked;
            beenCalled = true;
        };
        bool isChecked;
        bool beenCalled;
    };

    ToggleHandler toggelHandler;
    tizen_browser::base_ui::Action action;

    action.setCheckable(true);
    action.toggled.connect(boost::ref(toggelHandler));

    action.toggle();
    BOOST_CHECK_EQUAL(toggelHandler.beenCalled, true);
    BOOST_CHECK_EQUAL(action.isChecked(), toggelHandler.isChecked);

    BROWSER_LOGI("[UT] --> END - Action - action_togle_test");
}

BOOST_AUTO_TEST_SUITE_END()
