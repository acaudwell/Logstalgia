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

#include "core/vectors.h"

#include <string>
#include <vector>
#include <map>

class LogEntry {

private:
    static std::vector<std::string> fields;
    static std::vector<std::string> default_fields;
    static std::map<std::string, std::string> field_titles;

    std::string maskHostname(const std::string& hostname);
public:
    LogEntry();
    bool validate();

    void setSuccess();
    void setResponseColour();

    bool getValue(const std::string& field, std::string& value) const;

    std::string log_entry;

    time_t timestamp;

    std::string hostname;
    std::string vhost;

    std::string path;

    std::string pid;
    std::string method;
    std::string protocol;

    std::string response_code;
    long response_size;

    std::string referrer;
    std::string user_agent;

    vec3 response_colour;

    bool successful;

    static const std::vector<std::string>& getFields();
    static const std::vector<std::string>& getDefaultFields();
    static const std::string& getFieldTitle(const std::string& field);
};

class AccessLog {

public:
    AccessLog();
    virtual ~AccessLog() {};
    virtual bool parseLine(std::string& line, LogEntry& entry) = 0;

};

#endif
