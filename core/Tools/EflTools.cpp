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
#include <image_util.h>
#include <BrowserAssert.h>

#include "browser_config.h"
#include "BrowserLogger.h"
#include "EflTools.h"
#include "Elementary.h"


namespace tizen_browser {
namespace tools {
namespace EflTools {

std::unique_ptr<Blob> getBlobPNG(std::shared_ptr<BrowserImage> browserImage)
{
    BROWSER_LOGD("[%s:%d]", __PRETTY_FUNCTION__, __LINE__);
    if (!browserImage) {
        BROWSER_LOGD("browserImage is null");
        return nullptr;
    }
    unsigned long long length = 0;
    void* mem_buffer = getBlobPNG(
        browserImage->getWidth(),
        browserImage->getHeight(),
        browserImage->getData(),
        &length);
    if (!mem_buffer || !length) {
        BROWSER_LOGW("Cannot create BlobPNG");
        return nullptr;
    }
    std::unique_ptr<Blob> image(new Blob(mem_buffer, length));
    return std::move(image);
}

void* getBlobPNG(int width, int height, void* image_data, unsigned long long* length)
{
    BROWSER_LOGD("[%s:%d]", __PRETTY_FUNCTION__, __LINE__);
    EINA_SAFETY_ON_NULL_RETURN_VAL(image_data, NULL);

    image_util_encode_h handler = nullptr;
    unsigned char* outputBuffer = nullptr;

    if (image_util_encode_create(IMAGE_UTIL_PNG, &handler) < 0) {
        BROWSER_LOGW("[%s:%d] image_util_encode_create: error!", __PRETTY_FUNCTION__, __LINE__);
        return nullptr;
    }

    if (image_util_encode_set_png_compression(handler, IMAGE_UTIL_PNG_COMPRESSION_6) < 0) {
        BROWSER_LOGW("[%s:%d] image_util_encode_set_png_compression: error!", __PRETTY_FUNCTION__, __LINE__);
        return nullptr;
    }

    if (image_util_encode_set_resolution(handler, width, height) < 0) {
        BROWSER_LOGW("[%s:%d] image_util_encode_set_resolution: error!", __PRETTY_FUNCTION__, __LINE__);
        return nullptr;
    }

    if (image_util_encode_set_input_buffer(handler, (const unsigned char*) image_data) < 0) {
        BROWSER_LOGW("[%s:%d] image_util_encode_set_input_buffer: error!", __PRETTY_FUNCTION__, __LINE__);
        return nullptr;
    }

    if (image_util_encode_set_output_buffer(handler, &outputBuffer) < 0) {
        BROWSER_LOGW("[%s:%d] image_util_encode_set_output_buffer: error!", __PRETTY_FUNCTION__, __LINE__);
        return nullptr;
    }

    if (image_util_encode_run(handler, length) < 0) {
        BROWSER_LOGW("[%s:%d] image_util_encode_run: error!", __PRETTY_FUNCTION__, __LINE__);
        return nullptr;
    }

    if (image_util_encode_destroy(handler) < 0) {
        BROWSER_LOGW("[%s:%d] mage_util_encode_destroy: error!", __PRETTY_FUNCTION__, __LINE__);
        return nullptr;
    }
    return outputBuffer;
}

void setExpandHints(Evas_Object* toSet)
{
    evas_object_size_hint_weight_set(toSet, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(toSet, EVAS_HINT_FILL, EVAS_HINT_FILL);
}

bool pointInObject(Evas_Object* object, int px, int py)
{
    if (!object) {
        BROWSER_LOGE("@@ [%s:%d] null object", __func__, __LINE__);
        return false;
    }
    Evas_Coord x, y, w, h;
    evas_object_geometry_get(object, &x, &y, &w, &h);
    return (px >= x && px <= x + w && py >= y && py <= y + h);
}

Evas_Object* createToastPopup(Evas_Object* parent, double timeout, const char* text)
{
    auto toast(elm_popup_add(parent));
    elm_object_style_set(toast, "toast");
    elm_popup_timeout_set(toast, timeout);
    elm_object_text_set(toast, text);
    return toast;
}

} /* end of EflTools */
} /* end of namespace tools */
} /* end of namespace tizen_browser */

