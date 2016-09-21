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
#include <cstdint>
#include <cstring>

#include "browser_config.h"
#include "BrowserImage.h"
#include "BrowserLogger.h"

namespace tizen_browser {
namespace tools {

BrowserImage::BrowserImage() :
        m_width(0),
        m_height(0),
        m_dataSize(0),
        m_isSharedData(false),
        m_imageType(ImageType::ImageTypeNoImage),
        m_imageData(nullptr),
        m_colorSpace(EVAS_COLORSPACE_ARGB8888)
{}

BrowserImage::BrowserImage(const int& w, const int& h, const long& s) :
        m_width(w),
        m_height(h),
        m_dataSize(s),
        m_isSharedData(false),
        m_imageType(ImageType::ImageTypeNoImage),
        m_imageData(nullptr),
        m_colorSpace(EVAS_COLORSPACE_ARGB8888)
{}

BrowserImage::BrowserImage(Evas_Object* image) :
        m_imageData(nullptr)
{
    evas_object_image_size_get(image, &m_width, &m_height);

    m_colorSpace = evas_object_image_colorspace_get(image);
    switch (m_colorSpace) {
        case EVAS_COLORSPACE_ARGB8888:
            m_dataSize = m_width * m_height * sizeof(uint32_t);
            break;
        case EVAS_COLORSPACE_GRY8:
            m_dataSize = m_width * m_height * sizeof(uint8_t);
            break;
        default:
            m_dataSize = 0;
            break;
    }
    void* data = evas_object_image_data_get(image, EINA_FALSE);
    setData(data, false, ImageType::ImageTypeEvasObject);
    evas_object_image_data_set(image, data);
}

BrowserImage::BrowserImage(const BrowserImage& copy) :
        m_width(copy.getWidth()),
        m_height(copy.getHeight()),
        m_dataSize(copy.getSize()),
        m_isSharedData(false),
        m_imageType(ImageType::ImageTypeNoImage),
        m_imageData(nullptr),
        m_colorSpace(copy.getColorSpace())
{
    setData(copy.getData(), copy.isSharedData(), copy.getImageType());
}



void BrowserImage::setData(void* data, bool isSharedData, ImageType type)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (m_imageData) {
        free(m_imageData);
        m_imageData = nullptr;
    }
    m_imageType = ImageType::ImageTypeNoImage;
    if (!data) {
        m_dataSize = 0;
        return;
    }
    if (isSharedData) {
        m_imageData = data;
        m_imageType = type;
    } else {
        if (m_dataSize > 0) {
            m_imageData = malloc(m_dataSize);
            if (m_imageData) {
                std::memcpy(m_imageData, data, m_dataSize);
                m_imageType = type;
            } else {
                m_dataSize = 0;
            }
        }
    }
    m_isSharedData = isSharedData;
}

Evas_Object* BrowserImage::getEvasImage(Evas_Object* parent) const
{
    switch (m_imageType) {
        case ImageType::ImageTypeEvasObject:
            return getEvas(parent);
        case ImageType::ImageTypePNG:
            return getPng(parent);
        case ImageType::ImageTypeNoImage:
        default:
            return nullptr;
    }
}

Evas_Object* BrowserImage::getEvas(Evas_Object* parent) const
{
    Evas * e = evas_object_evas_get(parent);
    Evas_Object * eo_image = evas_object_image_filled_add(e);

    evas_object_image_size_set(eo_image, m_width, m_height);
    evas_object_image_colorspace_set(eo_image, m_colorSpace);
    void* data = evas_object_image_data_get(eo_image, EINA_TRUE);
    memcpy(data, m_imageData, m_dataSize);
    evas_object_image_data_set(eo_image, data);
    evas_object_image_alpha_set(eo_image, EINA_TRUE);

    Evas_Load_Error err = evas_object_image_load_error_get(eo_image);
    if (err != EVAS_LOAD_ERROR_NONE) {
        BROWSER_LOGE("[%s:%d] Could not load image'. error: \"%s\"\n", __PRETTY_FUNCTION__, __LINE__, evas_load_error_str(err));
        evas_object_del(eo_image);
        return nullptr;
    }

    evas_object_image_fill_set(eo_image, 0, 0, m_width, m_height);
    return eo_image;
}

Evas_Object* BrowserImage::getPng(Evas_Object * parent) const
{
    if(m_dataSize && m_imageData && m_imageType == ImageType::ImageTypePNG) {
        Evas * e = evas_object_evas_get(parent);
        Evas_Object * image = evas_object_image_filled_add(e);
        char png_format[] = "png";
        evas_object_image_memfile_set(image, m_imageData, m_dataSize, png_format, NULL);
        Evas_Load_Error error = evas_object_image_load_error_get(image);
        if (EINA_UNLIKELY(error != EVAS_LOAD_ERROR_NONE)) {
            BROWSER_LOGE("[%s:%d] Can't decode image: %s", __PRETTY_FUNCTION__, __LINE__, evas_load_error_str(error));
            evas_object_del(image);
            return nullptr;
        }
        return image;
    }
    return nullptr;
}

BrowserImage::~BrowserImage()
{
    if (m_imageData && !m_isSharedData) {
        free(m_imageData);
        m_imageData = nullptr;
    }
}


} /* end of namespace tools */
} /* end of namespace tizen_browser */
