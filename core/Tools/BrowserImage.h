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

#ifndef __BROWSER_IMAGE_H__
#define __BROWSER_IMAGE_H__

#include <Evas.h>
#include <memory>
namespace tizen_browser
{
namespace tools
{

enum class ImageType {
    ImageTypeNoImage,
    ImageTypeSerializedEvas,
    ImageTypeEvasObject,
    ImageTypePNG
};

class BrowserImage
{
public:
    BrowserImage();
    BrowserImage(const int& w, const int& h, const long& s);
    BrowserImage(Evas_Object* image);
    BrowserImage(const BrowserImage& copy);
    ~BrowserImage();

    /**
     * Sets image raw data pointer, type and memory share
     *
     * If isSharedData is true that means data is in shared memory
     * and it will not be released, otherwise data will be copied
     * and free in object destructor
     *
     * @param[in] data pointer to image data
     * @param[in] isSharedData true if @p data points to shared memory
     * @param[in] type Image type
     */
    void setData(void* data, bool isSharedData, ImageType type);

    /**
     * @return image data pointer, should not be released
     */
    void* getData() const { return m_imageData; };
    ImageType getImageType() const { return m_imageType; };
    void setWidth(const int& w) { m_width = w; };
    int getWidth() const { return m_width; };
    void setHeight(const int& h) { m_height = h; };
    int getHeight() const { return m_height; };
    void setSize(const long& s) { m_dataSize = s; };
    long getSize() const { return m_dataSize; };
    bool isSharedData() const { return m_isSharedData; };
    Evas_Colorspace getColorSpace() const { return m_colorSpace; };

    /**
     * Function create new Evas_Object* representing stored image
     *
     * @param[in] parent parent view
     * @return new Evas_Object* representing image or nullptr on fail
     */
    Evas_Object* getEvasImage(Evas_Object* parent) const;

private:
    Evas_Object* getEvas(Evas_Object* parent) const;
    Evas_Object* getPng(Evas_Object* parent) const;

    int m_width;
    int m_height;
    long m_dataSize;
    bool m_isSharedData;
    ImageType m_imageType;
    void * m_imageData;
    Evas_Colorspace m_colorSpace;
};

using BrowserImagePtr = std::shared_ptr<BrowserImage>;

} /* end of namespace tools */
} /* end of namespace tizen_browser */


#endif /* __BROWSER_IMAGE_H__ */
