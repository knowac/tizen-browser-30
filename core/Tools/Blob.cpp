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

#include "browser_config.h"
#include <cstdlib>
#include <string.h>
#include <stdexcept>
#include <BrowserAssert.h>

#include "Blob.h"
#include "BrowserLogger.h"

namespace tizen_browser
{
namespace tools
{

Blob::Blob() :
    m_data(NULL), m_length(0)
{
}

Blob::Blob(Blob && other) : m_data(NULL), m_length(0)
{
    m_data = other.m_data;
    m_length = other.m_length;
    other.m_data = NULL;
    other.m_length = 0;
}

Blob::Blob(const void* ptr, unsigned long long length) :
    m_length(length)
{
    m_data = malloc(length);
    if(!m_data && length > 0) {
        BROWSER_LOGE("%s - Failed to allocate %d bytes", length);
    	throw std::bad_alloc();
    }
    if(m_data)
        memcpy(m_data, ptr, length);
}

Blob::~Blob()
{
    free();
}

/*private*/
Blob::Blob(const Blob &) : m_data(NULL), m_length(0)
{
}

Blob & Blob::operator=(Blob && other)
{
    if (this != &other) {
        free();
        m_data = other.m_data;
        m_length = other.m_length;
    }
    return *this;
}

/*private*/
Blob & Blob::operator=(const Blob &)
{
    return *this;
}


const void * Blob::getData() const
{
    return this->m_data;
}

void Blob::setData(void* ptr, unsigned long long length)
{
    free();

    if (ptr != NULL && length != 0) {
        this->m_data = ptr;
        this->m_length = length;
    }
}

void Blob::free()
{
    ::free(this->m_data);
    this->m_data = NULL; // nullptr
    this->m_length = 0;
}

int Blob::getLength()
{
    return this->m_length;
}

int Blob::transferData(void ** data) {
	int len = this->m_length;
	*data = m_data;

	m_length = 0;
	m_data = NULL;

	return len;
}


}
}
