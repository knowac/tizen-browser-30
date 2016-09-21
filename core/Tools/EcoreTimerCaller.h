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

#ifndef TOOLS_ECORETIMERCALLER_H_
#define TOOLS_ECORETIMERCALLER_H_

#include <Elementary.h>

namespace tizen_browser {
namespace tools {
namespace EflTools {

/**
 * Invokes () operator of the given type with ecore_timer_add. Could be used
 * to ensure, that code is invoked in the end of EFL loop (callback interval
 * 0.0) or after other fixed interval. Template only simplifies the use of
 * ecore_timer_add.
 */
template<typename T>
class EcoreTimerCaller
{
public:
    EcoreTimerCaller();
    virtual ~EcoreTimerCaller()
    {
    }

    /**
     * Call T() after given interval.
     */
    void addTimer(const T& callback, double in = 0.0);
private:
    static Eina_Bool timerHandle(void *data);

    T m_callback;
    bool m_handled;
};

template<typename T>
EcoreTimerCaller<T>::EcoreTimerCaller() :
        m_handled(true)
{
}

template<typename T>
void EcoreTimerCaller<T>::addTimer(const T& callback, double in)
{
    if (!m_handled)
        return;
    m_callback = callback;
    ecore_timer_add(in, EcoreTimerCaller<T>::timerHandle, this);
}

template<typename T>
Eina_Bool EcoreTimerCaller<T>::timerHandle(void* data)
{
    auto self = reinterpret_cast<EcoreTimerCaller<T>*>(data);
    if (!self)
        return ECORE_CALLBACK_CANCEL;
    self->m_callback();
    // delete timer
    return ECORE_CALLBACK_CANCEL;
}

}
}
}

#endif /* TOOLS_ECORETIMERCALLER_H_ */
