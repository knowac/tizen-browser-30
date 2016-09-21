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

#ifndef __BLOB_H__
#define __BLOB_H__

namespace tizen_browser
{
namespace tools
{

class Blob
{
public:
    explicit Blob();
    explicit Blob(Blob && other);
    explicit Blob(const void* ptr, unsigned long long length);
    ~Blob();
    Blob & operator=(Blob && other);
    const void * getData() const;
    void setData(void* ptr, unsigned long long length);
    void free();
    int getLength();
    int transferData(void ** data);

private:
    Blob(const Blob &);
    Blob & operator=(const Blob &);

    void * m_data;
    unsigned long long m_length;
};

}
}

#endif
