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

#include <sqlite3.h>

#include "Field.h"
#include "Blob.h"

namespace tizen_browser {
namespace storage {

Field::Field()
    : sqlInt(0)
    , sqlDouble(0)
{
    this->type = SQLITE_NULL;
}

/*private*/Field::Field(const Field& f)
    : type(f.getType())
    , sqlInt(f.getInt())
    , sqlDouble(f.getDouble())
{
}
/*private*/Field & Field::operator=(const Field &)
{
    return *this;
}

Field::Field(int sqlInt)
    : type(SQLITE_INTEGER)
    , sqlInt(sqlInt)
    , sqlDouble(0)
{
}

Field::Field(double sqlDouble)
    : type(SQLITE_FLOAT)
    , sqlInt(0)
    , sqlDouble(sqlDouble)
{
}

Field::Field(const std::string & sqlText)
{
    this->sqlText = sqlText;
    this->type = SQLITE3_TEXT;
}

Field::Field(std::shared_ptr<tizen_browser::tools::Blob> blob)
{
    this->blob = blob;
    this->type = SQLITE_BLOB;
}

Field::~Field()
{
}

int Field::getInt() const
{
    return this->sqlInt;
}

double Field::getDouble() const
{
    return this->sqlDouble;
}

std::string Field::getString() const
{
    return this->sqlText;
}

const std::shared_ptr<tizen_browser::tools::Blob> Field::getBlob() const
{
    return this->blob;
}

int Field::getType() const
{
    return this->type;
}

}
}
