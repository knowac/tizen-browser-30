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

#ifndef __ABSTRACT_ROTATABLE_H__
#define __ABSTRACT_ROTATABLE_H__

namespace tizen_browser
{
namespace interfaces
{

/**
 * @brief This interface defines rotatable object,
 * which reacts on mobile device oriantation change.
 */
class AbstractRotatable
{
public:
    virtual ~AbstractRotatable(){};
    /**
     * @brief Abstract method for receiving orientation change event.
     */
    virtual void orientationChanged() = 0;

    /**
     * @brief Boost signal, check what is current rotation state.
     * Class which implements AbstractRotatable has to connect this signal in SimpleUI class.
     * @returns True if rotation is portrait, false if rotation is landscape.
     */
    boost::signals2::signal<bool ()> isLandscape;

    /**
     * @brief Boost signal, get what is current rotation angle.
     * Class which implements AbstractRotatable has to connect this signal in SimpleUI class.
     * @returns rotation angle.
     */
    boost::signals2::signal<int ()> getRotation;
};

}//namespace interfaces
}//namespace tizen_browser

#endif /* __ABSTRACT_ROTATABLE_H__ */
