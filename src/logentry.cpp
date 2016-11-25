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

#include "logentry.h"
#include "settings.h"
#include "core/regex.h"

#include <time.h>
#include <algorithm>

//AccessLog

AccessLog::AccessLog() {
}

//LogEntry

std::vector<std::string> LogEntry::fields;
std::vector<std::string> LogEntry::default_fields;
std::map<std::string, std::string> LogEntry::field_titles;

LogEntry::LogEntry() {
    timestamp = 0;
    response_size = 0;
    successful = false;
    response_colour = vec3(1.0, 0.0, 0.0);
}

Regex logentry_ipv6("(?i)^[a-f0-9:]+$");

Regex logentry_hostname_parts("([^.]+)(?:\\.([^.]+))?(?:\\.([^.]+))?(?:\\.([^.]+))?(?:\\.([^.]+))?(?:\\.([^.]+))?(?:\\.([^.]+))?(?:\\.([^.]+))?$");

std::string LogEntry::maskHostname(const std::string& hostname) {

    std::vector<std::string> parts;

    // could be an ipv6 address
    if(logentry_ipv6.match(hostname) && hostname.find(":") != std::string::npos) {

        size_t c = 0;
        size_t last     = hostname.size(); 

        size_t colon_count = std::count(hostname.begin(), hostname.end(), ':');
        size_t padding     = 7 - colon_count;

        if(colon_count <= 7) {
            
            size_t previous = 0;

            while( (last = hostname.rfind(":",last-1)) != std::string::npos) {

                c++;

                if(last == (previous-1)) c += padding;
                
                if(c >= 4) {
                    std::string output = hostname.substr(0, last);
                    output += '-';
                    return output;
                }

                previous = last;
            }
        }
    }
    
    logentry_hostname_parts.match(hostname, &parts);

    size_t part_count = parts.size();
    
    //if only 1-2 parts, or 3 parts and a 2 character suffix, pass through unchanged
    if( part_count <= 2 || (part_count == 3 && parts[part_count-1].size()==2))
        return hostname;

    int num = atoi(parts[part_count-1].c_str());

    std::string output;

    //if last element is numeric, assume it is a numbered ip address
    //(ie 192.168.0.1 => 192.168.0-)

    if(num != 0) {
        for(size_t i=0;i<part_count-1;i++) {
            if(i>0) output += '.';
            output += parts[i];
        }

        output += '-';
        return output;
    }

    //hide the first element
    //(ie dhcp113.web.com -> web.com
    for(size_t i=1;i<part_count;i++) {
            if(i>1) output += '.';
            output += parts[i];
    }

    return output;
}

void LogEntry::setSuccess() {

    int code = atoi(response_code.c_str());

    successful = (code<400) ? true : false;
}

void LogEntry::setResponseColour() {

    int code = atoi(response_code.c_str());

    //set response colour
    if(code<200) {
        response_colour = vec3(0.0f, 1.0f, 0.5f);
    }
    else if(code>= 200 && code < 300) {
        response_colour = vec3(1.0f, 1.0f, 0.0f);
    }
    else if(code>= 300 && code < 400) {
        response_colour = vec3(1.0f, 0.5f, 0.0f);
    }
    else {
        response_colour = vec3(1.0f, 0.0f, 0.0f);
    }
}

const std::vector<std::string>& LogEntry::getFields() {

    if(fields.empty()) {
        fields.push_back("pid");
        fields.push_back("path");
        fields.push_back("vhost");
        fields.push_back("hostname");
        fields.push_back("response_code");
        fields.push_back("response_size");
        fields.push_back("referrer");
        fields.push_back("user_agent");
        fields.push_back("timestamp");
        fields.push_back("log_entry");
    }

    return fields;
}

const std::vector<std::string>& LogEntry::getDefaultFields() {

    if(default_fields.empty()) {
        default_fields.push_back("vhost");
        default_fields.push_back("path");
        default_fields.push_back("timestamp");
        default_fields.push_back("hostname");
        default_fields.push_back("response_code");
        default_fields.push_back("response_size");
        default_fields.push_back("referrer");
        default_fields.push_back("user_agent");
    }

    return default_fields;
}

const std::string& LogEntry::getFieldTitle(const std::string& field) {

    if(field_titles.empty()) {
        field_titles["pid"]           = "PID";
        field_titles["path"]          = "Path";
        field_titles["vhost"]         = "Virtual Host";
        field_titles["hostname"]      = "Hostname";
        field_titles["response_size"] = "Response Size";
        field_titles["response_code"] = "Response Code";
        field_titles["referrer"]      = "Referrer";
        field_titles["user_agent"]    = "User Agent";
        field_titles["timestamp"]     = "Timestamp";
        field_titles["log_entry"]     = "Log Entry";
    }

    auto it = field_titles.find(field);

    assert(it != field_titles.end());

    return it->second;
}

bool LogEntry::getValue(const std::string& field, std::string& value) const {

    if(field == "pid") {
        value = pid;
        return true;
    }

    if(field == "path") {
        value = path;
        return true;
    }

    if(field == "hostname") {
        value = hostname;
        return true;
    }

    if(field == "vhost") {
        value = vhost;
        return true;
    }

    if(field == "response_size") {
        value = std::to_string(response_size);
        return true;
    }

    if(field == "response_code") {
        value = response_code;
        return true;
    }

    if(field == "referrer") {
        value = referrer;
        return true;
    }

    if(field == "user_agent") {
        value = user_agent;
        return true;
    }

    if(field == "timestamp") {
        struct tm* timeinfo = localtime ( &timestamp );
        char timestamp_buff[256];
        strftime(timestamp_buff, 256, "%Y-%m-%d %H:%M:%S", timeinfo);
        value = std::string(timestamp_buff);
        return true;
    }

    if(field == "log_entry") {
        value = log_entry;
        return true;
    }

    value = "";
    return false;
}

bool LogEntry::validate() {
    if(pid == "-") pid = "";
    if(referrer == "-") referrer = "";

    if(hostname.empty()) return false;

    if(settings.mask_hostnames) {
        hostname = maskHostname(hostname);
    }

    if(path.empty()) return false;
    if(timestamp == 0) return false;

    return true;
}
