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

#ifndef __ABSTRACT_MAIN_WINDOW_HPP__
#define __ABSTRACT_MAIN_WINDOW_HPP__ 1

#include <memory>
#include <functional>

#include <boost/noncopyable.hpp>

#include "BrowserAssert.h"
#include "Lifecycle.h"
#include "../ServiceManager/AbstractService.h"

namespace tizen_browser
{
namespace base_ui
{

/**
 * @brief This class is placeholder for generic AbstractMainWindow object. It will
 * handle resource deallocation.
 */
template <typename T>
#ifndef NDEBUG
class AbstractMainWindow : public tizen_browser::core::AbstractService, boost::noncopyable , ShowLifeCycle<AbstractMainWindow<T>>
#else
class AbstractMainWindow : public tizen_browser::core::AbstractService, boost::noncopyable
#endif
{
public:
    AbstractMainWindow()
        : m_window()
    {}

    virtual int exec(
        const std::string& url,
        const std::string& caller,
        const std::string& operation) = 0;
    std::shared_ptr<T> getMainWindow(){return m_window;};

    /**
     * \todo:
     * Extend API of main window placeholder class.
     */
    void setMainWindow(T *rawPtr){ m_window=std::shared_ptr<T>(rawPtr);}
    virtual void destroyUI() { }

    virtual void suspend() = 0;
    virtual void resume() = 0;

protected:
    std::shared_ptr<T> m_window;
};

} /* end of namespace base_ui */
} /* end of namespace tizen_browser */

#endif /* __ABSTRACT_MAIN_WINDOW_HPP__ */
