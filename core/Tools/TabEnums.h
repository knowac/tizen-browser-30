/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
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

/*
 * TabEnum.h
 *
 *  Created on: Sep 8, 2016
 *      Author: m.kawonczyk@samsung.com
 */

#ifndef TABENUMS_H
#define TABENUMS_H

#include <string>

#include "app_i18n.h"

#define ADD_TRAN(x,y) static std::string x = _(y);

namespace tizen_browser
{
namespace base_ui
{
namespace Translations {
    ADD_TRAN(CreatePassword,"IDS_BR_BODY_YOUR_PASSWORD_MUST_CONTAIN_AT_LEAST_PD_CHARACTERS_INCLUDING_AT_LEAST_1_LETTER")
    ADD_TRAN(ConfirmCreatePassword, "IDS_BR_BODY_ENTER_THE_PASSWORD_AGAIN_TO_CONFRIM_IT")
    ADD_TRAN(ConfirmPassword, "IDS_BR_BODY_ENTER_YOUR_CURRENT_SECRET_MODE_PASSWORD")
    ADD_TRAN(IncorrectPassword, "IDS_BR_BODY_AN_INCORRECT_PASSWORD_HAS_BEEN_ENTERED_TRY_AGAIN")
    ADD_TRAN(ShowPassword, "IDS_BR_OPT_SHOW_PASSWORD_ABB2")
    ADD_TRAN(UsePassword, "IDS_BR_BUTTON_USE_PASSWORD_ABB")
    ADD_TRAN(ChangePassword, "IDS_BR_TMBODY_CHANGE_PASSWORD")
};

enum class PasswordAction {
    UsePassword,
    ChangePassword,
    CreatePasswordFirstTime,
    ConfirmPasswordEnterSecret
};

enum class PasswordState {
    ConfirmPassword,
    IncorrectPassword,
    CreatePassword,
    ConfirmCreatePassword,
    SecretModeData
};

}
}

#endif // TABENUMS_H
