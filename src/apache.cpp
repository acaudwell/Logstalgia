/*
    Copyright (C) 2010 Andrew Caudwell (acaudwell@gmail.com)

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

#include "apache.h"

const char* ls_apache_months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug" , "Sep", "Oct", "Nov", "Dec" };
Regex ls_apache_entry_start("^(?:([^ ]+) )?([^ ]+) +[^ ]+ +([^ ]+) +\\[(.*?)\\] +(.*)$");
Regex ls_apache_entry_date("(\\d+)/([A-Za-z]+)/(\\d+):(\\d+):(\\d+):(\\d+) ([+-])(\\d+)");
Regex ls_apache_entry_request("\"([^ ]+) +([^ ]+) +([^ ]+)\" +([^ ]+) +([^\\s+]+)(.*)");
Regex ls_apache_entry_agent("(?: +\"([^\"]+)\" +\"([^\"]+)\")?(?: +\([^ ]+))?");

ApacheLog::ApacheLog() {
}

//parse apache access.log entry into components
bool ApacheLog::parseLine(std::string& line, LogEntry& entry) {

    std::vector<std::string> matches;
    ls_apache_entry_start.match(line, &matches);

    if(matches.size()!=5) {
        return 0;
    }

    //get details
    entry.vhost    = matches[0];
    entry.hostname = matches[1];
    //entry.username = matches[1];

    //parse timestamp
    struct tm time_str;

    int day, month, year, hour, minute, second, zone;

    std::string request_str = matches[4];
    std::string datestr     = matches[3];

    matches.clear();
    ls_apache_entry_date.match(datestr, &matches);

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
        if(strcmp(matches[1].c_str(), ls_apache_months[i])==0) {
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

    entry.timestamp = mktime(&time_str);

    matches.clear();
    ls_apache_entry_request.match(request_str, &matches);

    if(matches.size() < 5) {
        return 0;
    }

//    entry.method    = matches[0];
    entry.path      = matches[1];
//    entry.protocol  = matches[2];

    entry.response_code = matches[3].c_str();
    entry.response_size = atol(matches[4].c_str());

    if(matches.size() > 5) {
        std::string agentstr = matches[5];
        matches.clear();
        ls_apache_entry_agent.match(agentstr, &matches);

        if(matches.size()==3) {
            entry.referrer   = matches[0];
            entry.user_agent = matches[1];
            entry.pid        = matches[2];
        }
    }

    //successful if response code less than 400
    //for apache
    int code = atoi(entry.response_code.c_str());

    entry.setSuccess();
    entry.setResponseColour();

    return entry.validate();
}

