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

#include <memory>
#include <map>
#include <BrowserAssert.h>

#include "DriverManager.h"

namespace tizen_browser {
namespace storage {

class DriverManagerInstance
{
public:
	std::map< std::string, std::shared_ptr<SQLDatabase> > _instances;

	std::shared_ptr<SQLDatabase> getDatabase(const std::string& aConn);
};

static DriverManagerInstance s_driverManager;

std::shared_ptr<SQLDatabase> DriverManagerInstance::getDatabase(const std::string& aConn)
{
	auto it = _instances.find(aConn);
	if(it != _instances.end())
		return it->second;
	std::shared_ptr<SQLDatabase> db(SQLDatabase::newInstance());
	db->open(aConn);
	_instances[aConn] = db;
	return db;
}

DriverManager::DriverManager()
{

}

DriverManager::~DriverManager()
{
}

std::shared_ptr<SQLDatabase> DriverManager::getDatabase(const std::string & aConn)
{
	return s_driverManager.getDatabase(aConn);
}

}
}