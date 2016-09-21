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

#include "browser_config.h"
#include "Action.h"

namespace tizen_browser
{
namespace base_ui
{

Action::Action()
    :m_enabled(true)
    ,m_checkable(false)
    ,m_checked(false)
    ,m_iconVisibleInMenu(true)
{

}
Action::Action(const std::string& text)
    :m_text(text)
    ,m_enabled(true)
    ,m_checkable(false)
    ,m_checked(false)
    ,m_iconVisibleInMenu(true)
{
}

Action::Action(const std::string& icon, const std::string& text)
    :m_text(text)
    ,m_icon(icon)
    ,m_enabled(true)
    ,m_checkable(false)
    ,m_checked(false)
    ,m_iconVisibleInMenu(true)
{
}


Action::Action(const Action& other)
    :m_text(other.m_text)
    ,m_statusTip(other.m_statusTip)
    ,m_toolTip(other.m_toolTip)
    ,m_icon(other.m_icon)
    ,m_selIcon(other.m_selIcon)
    ,m_disIcon(other.m_disIcon)
    ,m_enabled(other.m_enabled)
    ,m_checkable(other.m_checkable)
    ,m_checked(other.m_checked)
    ,m_iconVisibleInMenu(other.m_iconVisibleInMenu)
{

}

Action::~Action()
{

}

std::string Action::getDisIcon() const
{
    return m_disIcon;
}

std::string Action::getIcon() const
{
    return m_icon;
}

std::string Action::getSelIcon() const
{
    return m_selIcon;
}

std::string Action::getStatusTip() const
{
    return m_statusTip;
}

std::string Action::getText() const
{
    return m_text;
}

std::string Action::getToolTip() const
{
    return m_toolTip;
}

void Action::setDisIcon(const std::string& disIcon)
{
    m_disIcon = disIcon;
}

void Action::setIcon(const std::string& icon)
{
    m_icon = icon;
}

void Action::setSelIcon(const std::string& selIcon)
{
    m_selIcon = selIcon;
}

void Action::setStatusTip(const std::string& statusTip)
{
    m_statusTip = statusTip;
}

void Action::setText(const std::string& text)
{
    m_text = text;
}

void Action::setToolTip(const std::string& toolTip)
{
    m_toolTip = toolTip;
}

bool Action::isCheckable() const
{
    return m_checkable;
}

bool Action::isEnabled() const
{
    return m_enabled;
}

bool Action::isIconVisibleInMenu() const
{
    return m_iconVisibleInMenu;
}

void Action::setCheckable(bool checkable)
{
    m_checkable = checkable;
}

void Action::setEnabled(bool enabled)
{
	if(m_enabled != enabled){
		m_enabled = enabled;
		enabledChanged();
	}
}

void Action::setIconVisibleInMenu(bool visible)
{
    m_iconVisibleInMenu = visible;
}

bool Action::isChecked() const
{
    return m_checked;
}

void Action::setChecked(bool checked)
{
    if(m_checkable){
        if(m_checked != checked){
            m_checked = checked;
            toggled(m_checkable);
        }
    }
}

void Action::toggle()
{
    if(m_checkable){
        m_checked = !m_checked;
        toggled(m_checked);
    }
}

void Action::trigger()
{
    if(m_enabled){
        triggered();
    }
}

} /* end of namespace base_ui */
} /* end of namespace tizen_browser */
