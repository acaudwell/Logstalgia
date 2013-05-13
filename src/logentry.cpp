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

bool  gMask    = true;


//AccessLog

AccessLog::AccessLog() {
}

//LogEntry

LogEntry::LogEntry() {
    timestamp = 0;
    response_size = 0;
    successful = false;
    response_colour = vec3(1.0, 0.0, 0.0);
}

Regex logentry_hostname_parts("([^.]+)(?:\\.([^.]+))?(?:\\.([^.]+))?(?:\\.([^.]+))?(?:\\.([^.]+))?(?:\\.([^.]+))?(?:\\.([^.]+))?(?:\\.([^.]+))?$");

std::string LogEntry::maskHostname(std::string hostname) {

    std::vector<std::string> parts;
    logentry_hostname_parts.match(hostname, &parts);

    //if only 1-2 parts, or 3 parts and a 2 character suffix, pass through unchanged
    if( parts.size()<=2 || (parts.size()==3 && parts[parts.size()-1].size()==2))
        return hostname;

    int num = atoi(parts[parts.size()-1].c_str());

    std::string output;

    //if last element is numeric, assume it is a numbered ip address
    //(ie 192.168.0.1 => 192.168.0-)
    if(num!=0) {
        for(size_t i=0;i<parts.size()-1;i++) {
            if(i>0) output += '.';
            output += parts[i];
        }

        output += '-';
        return output;
    }

    //hide the first element
    //(ie dhcp113.web.com -> web.com
    for(size_t i=1;i<parts.size();i++) {
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

bool LogEntry::validate() {
    if(pid == "-") pid = "";
    if(referrer == "-") referrer = "";

    if(hostname.size()==0) return false;

    if(gMask) {
        hostname = maskHostname(hostname);
    }

    if(path.size()==0) return false;
    if(timestamp == 0) return false;

    return true;
}
