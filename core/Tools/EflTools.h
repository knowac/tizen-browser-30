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

/*
 *
 * Created on: May, 2014
 *     Author: k.dobkowski
 */

#ifndef __EFL_TOOLS_H__
#define __EFL_TOOLS_H__ 1

#include <Evas.h>
#include "BrowserImageTypedef.h"
#include "BrowserImage.h"
#include "Blob.h"

// counts size exactly as is in Z3 device
#define Z3_SCALE_SIZE(x) (int)(((double)(x) * elm_config_scale_get()) / 2.6)

namespace tizen_browser {
namespace tools {
namespace EflTools {

    std::unique_ptr<Blob> getBlobPNG(BrowserImagePtr browserImage);
    void* getBlobPNG(int width, int height, void * image_data, unsigned long long* length);

    void setExpandHints(Evas_Object* toSet);

    /**
     * Check if coordinates are inside of a given object.
     */
    bool pointInObject(Evas_Object* object, int x, int y);

    Evas_Object* createToastPopup(Evas_Object* parent, double timeout, const char* text);
} /* end of namespace EflTools */
} /* end of namespace tools */
} /* end of namespace tizen_browser */


#endif /* __EFL_TOOLS_H__ */
