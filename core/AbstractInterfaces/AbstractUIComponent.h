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

#ifndef __ABSTRACT_UI_COMPONENT_H__
#define __ABSTRACT_UI_COMPONENT_H__ 1

#include <boost/signals2/signal.hpp>

namespace tizen_browser
{
namespace interfaces
{

/**
 * @brief This interface defines UI Component API.
 */
class AbstractUIComponent
{
public:
    virtual ~AbstractUIComponent(){};
/**
 * @brief Sets the parent which will be used when getContent is used.
 */
    virtual void init(Evas_Object* parent) = 0;

/**
 * @brief If UI content is not created creates it.
 *        Call init betore calling this function.
 * @return UI content.
 */
    virtual Evas_Object* getContent() = 0;

/**
 * @brief This makes element visible and clickable.
 *        Recreates genlists and gengrids (due to elm_genlist_clear and elm_gengrid_clear isue)
 */
    virtual void showUI() = 0;

/**
 * @brief This makes UI element invisible and unclickable.
 *        Deletes genlists and gengrids (due to elm_genlist_clear and elm_gengrid_clear isue)
 */
    virtual void hideUI() = 0;

/**
 * @brief Signal which has to be connected to view manager pop from stack function.
 */
    boost::signals2::signal<void ()> closeUI;
};

}//namespace interfaces
}//namespace tizen_browser

#endif /* __ABSTRACT_UI_COMPONENT_H__ */
