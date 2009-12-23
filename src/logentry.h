/*
    Copyright (C) 2008 Andrew Caudwell (acaudwell@gmail.com)

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version
    3 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef LOGENTRY_H
#define LOGENTRY_H

#include <string>
#include <string.h>

#include <vector>
#include <time.h>

#include "core/sdlapp.h"
#include "core/regex.h"

extern bool  gMask;

class LogEntry {

private:
    bool parse_ok;

    std::string host;
    std::string user;
    long timestamp;
    std::string rtype;
    std::string file;
    std::string proto;
    int code;
    long bytes;
    std::string refer;
    std::string agent;

    int parse(std::string& line);
    std::string maskHostname(std::string hostname);
public:
    LogEntry();
    LogEntry(std::string& entry);
    int  responseCode();
    long  bytesCount();
    long  getTimestamp();
    std::string getHostname();
    void setHostname(std::string str);
    std::string getUsername();
    std::string referrerURL();
    std::string requestURL();
    std::string requestType();
    std::string userAgent();
    bool successful();

    bool parsedOK();

};

#endif
