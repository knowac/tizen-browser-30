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

#ifndef __FIELD_H
#define __FIELD_H

#include <memory>
#include <string>
#include <vector>

#include "Blob.h"

namespace tizen_browser {
namespace storage {

class Field;

typedef std::shared_ptr<Field> FieldPtr;
typedef std::vector<FieldPtr> Fields;

class Field
{
public:
    explicit Field();
    explicit Field(int sqlInt);
    explicit Field(double sqlDouble);
    explicit Field(const std::string & sqlText);
    explicit Field(std::shared_ptr<tizen_browser::tools::Blob> blob);

    ~Field();

    int getInt() const;
    double getDouble() const;
    std::string getString() const;
    const std::shared_ptr<tizen_browser::tools::Blob> getBlob() const;

    int getType() const;

private:
    Field(const Field &);
    Field & operator=(const Field &);

    int type;

    int sqlInt;
    double sqlDouble;
    std::string sqlText;
    std::shared_ptr<tizen_browser::tools::Blob> blob;
};

}
}

#endif
