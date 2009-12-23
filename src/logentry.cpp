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

//LogEntry
LogEntry::LogEntry() {
}

LogEntry::LogEntry(std::string& entry) {
    parse_ok = parse(entry);

}

bool LogEntry::successful() {
    return (code<400);
}

const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug" , "Sep", "Oct", "Nov", "Dec" };
Regex entry_start("^([^ ]+) +[^ ]+ +([^ ]+) +\\[(.*?)\\] +(.*)$");
Regex entry_date("(\\d+)/([A-Za-z]+)/(\\d+):(\\d+):(\\d+):(\\d+) ([+-])(\\d+)");
Regex entry_request("\"([^ ]+) +([^ ]+) +([^ ]+)\" +([^ ]+) +([^\\s+]+)(.*)");
Regex entry_agent(" +\"([^\"]+)\" +\"([^\"]+)\" +\"([^\"]+)\"");
Regex hostname_parts("([^.]+)(?:\\.([^.]+))?(?:\\.([^.]+))?(?:\\.([^.]+))?(?:\\.([^.]+))?(?:\\.([^.]+))?(?:\\.([^.]+))?(?:\\.([^.]+))?$");

std::string LogEntry::maskHostname(std::string hostname) {

    std::vector<std::string> parts;
    hostname_parts.match(hostname, &parts);

    //if only 1-2 parts, or 3 parts and a 2 character suffix, pass through unchanged
    if(parts.size()<=2 || parts.size()==3 && parts[parts.size()-1].size()==2) return hostname;

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


//parse apache access.log entry into components
int LogEntry::parse(std::string& line) {

    std::vector<std::string> matches;
    entry_start.match(line, &matches);

    if(matches.size()!=4) {
        return 0;
    }

    //get details
    host      = matches[0];
    user      = matches[1];

    if(gMask) host = maskHostname(host);

    //parse timestamp
    struct tm time_str;

    int day, month, year, hour, minute, second, zone;

    std::string request_str = matches[3];
    std::string datestr     = matches[2];

    matches.clear();
    entry_date.match(datestr, &matches);

    if(matches.size()!=8) {
        return 0;
    }

    day    = atoi(matches[0].c_str());
    year   = atoi(matches[2].c_str());
    hour   = atoi(matches[3].c_str());
    minute = atoi(matches[4].c_str());
    second = atoi(matches[5].c_str());
    zone   = atoi(matches[7].c_str());

    //negative timezone
    if(strcmp(matches[6].c_str(), "-")==0) {
        zone = -zone;
    }

    month=0;
    for(int i=0;i<12;i++) {
        if(strcmp(matches[1].c_str(), months[i])==0) {
            month=i;
            break;
        }
    }

    time_str.tm_year = year - 1900;
    time_str.tm_mon  = month;
    time_str.tm_mday = day;
    time_str.tm_hour = hour;
    time_str.tm_min = minute;
    time_str.tm_sec = second;
    time_str.tm_isdst = -1;

    timestamp = mktime(&time_str);

    matches.clear();
    entry_request.match(request_str, &matches);

    if(matches.size() < 5) {
        debugLog("couldnt find rest!\n");
        return 0;
    }

    rtype     = matches[0];
    file      = matches[1];
    proto     = matches[2];
    code      = atoi(matches[3].c_str());
    bytes     = atol(matches[4].c_str());

    if(matches.size() > 5) {
        std::string agentstr = matches[5];
        matches.clear();
        entry_agent.match(agentstr, &matches);

        if(matches.size()>1) {
            refer     = matches[0];
            agent     = matches[1];
        }
    }

    return 1;
}

bool LogEntry::parsedOK() {
    return parse_ok;
}

void LogEntry::setHostname(std::string str) {
    host = str;
}

int         LogEntry::responseCode() { return code; }
long        LogEntry::bytesCount()   { return bytes; }
long        LogEntry::getTimestamp()   { return timestamp; }
std::string LogEntry::getHostname() { return host; }
std::string LogEntry::getUsername() { return user; }
std::string LogEntry::referrerURL() { return refer; }
std::string LogEntry::requestURL() { return file; }
std::string LogEntry::requestType() { return rtype; }
std::string LogEntry::userAgent() { return agent; }
