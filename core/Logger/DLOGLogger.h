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
 * DLOGLogger.h
 *
 *  Created on: Apr 9, 2014
 *      Author: p.chmielewsk@samsung.com
 */


#ifndef DLOGLOGGER_H_
#define DLOGLOGGER_H_

#include "AbstractLogger.h"

namespace tizen_browser
{
namespace logger
{

class DLOGLogger: public AbstractLogger {
public:
    DLOGLogger();
    ~DLOGLogger();

    virtual void init();
    virtual void log(const std::string &timeStamp, const std::string &tag,const std::string &msg, bool errorFlag = false);
private:
};

} /* end namespace logger */
} /* end namespace browser */

#endif /* DLOGLOGGER_H_ */
