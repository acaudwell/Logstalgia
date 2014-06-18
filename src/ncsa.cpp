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

#include "ncsa.h"
#include "core/regex.h"

#include <time.h>

const char* ls_ncsa_months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug" , "Sep", "Oct", "Nov", "Dec" };
Regex ls_ncsa_entry_start("^(?:([^ ]+) )?([^ ]+) +[^ ]+ +([^ ]+) +\\[(.*?)\\] +(.*)$");
Regex ls_ncsa_entry_date("(\\d+)/(\\d+|[A-Za-z]+)/(\\d+):(\\d+):(\\d+):(\\d+) ([+-])(\\d+)");
Regex ls_ncsa_entry_request("\"(?:([^ ]+) +([^ ]+) +([^ ]+)|(?:[^\"]*))\" +([^ ]+) +([^\\s+]+)(.*)");
Regex ls_ncsa_entry_agent("(?: +\"([^\"]+)\" +\"([^\"]+)\")?( .+)?");
Regex ls_ncsa_extra_field("^ +(\"[^\"]*\"|[^ ]+)");

NCSALog::NCSALog() {
}

//parse NCSA format access.log entry into components
bool NCSALog::parseLine(std::string& line, LogEntry& entry) {

    std::vector<std::string> matches;
    ls_ncsa_entry_start.match(line, &matches);

    if(matches.size()!=5) {
        return 0;
    }

    //get details
    entry.vhost    = matches[0];
    entry.hostname = matches[1];
    //entry.username = matches[1];

    //parse timestamp
    struct tm time_str;

    int day, month, year, hour, minute, second;

    std::string request_str = matches[4];
    std::string datestr     = matches[3];

    matches.clear();
    ls_ncsa_entry_date.match(datestr, &matches);

    if(matches.size()!=8) {
        return 0;
    }

    day    = atoi(matches[0].c_str());
    month  = atoi(matches[1].c_str());
    year   = atoi(matches[2].c_str());
    hour   = atoi(matches[3].c_str());
    minute = atoi(matches[4].c_str());
    second = atoi(matches[5].c_str());

    if(month) {
        month--;
    } else {
        //parse non numeric month
        for(int i=0;i<12;i++) {
            if(strcmp(matches[1].c_str(), ls_ncsa_months[i])==0) {
                month=i;
                break;
            }
        }
    }

    //could not parse month (range 0-11 as used by mktime)
    if(month<0 || month>11) return 0;

    //convert zone to utc offset
    int tz_hour = atoi(matches[7].substr(0,2).c_str());
    int tz_min  = atoi(matches[7].substr(2,2).c_str());

    int tz_offset = tz_hour * 3600 + tz_min * 60;

    if(matches[6] == "-") {
        tz_offset = -tz_offset;
    }

    time_str.tm_year = year - 1900;
    time_str.tm_mon  = month;
    time_str.tm_mday = day;
    time_str.tm_hour = hour;
    time_str.tm_min = minute;
    time_str.tm_sec = second;
    time_str.tm_isdst = -1;

    entry.timestamp = mktime(&time_str);

    //apply utc offset
    entry.timestamp -= tz_offset;

    matches.clear();
    ls_ncsa_entry_request.match(request_str, &matches);

    if(matches.size() < 5) {
        return 0;
    }

//    entry.method    = matches[0];
    entry.path        = (!matches[1].empty()) ? matches[1] : "???";
//    entry.protocol  = matches[2];

    entry.response_code = matches[3];
    entry.response_size = atol(matches[4].c_str());

    if(matches.size() > 5) {
        std::string agentstr = matches[5];
        matches.clear();
        ls_ncsa_entry_agent.match(agentstr, &matches);

        if(matches.size()==3) {
            entry.referrer   = matches[0];
            entry.user_agent = matches[1];

            std::string extra = matches[2];

            // NOTE: could store extra fields and allow --paddle-mode to address then via their offset
            if(!extra.empty()) {

                std::vector<std::string> extra_fields;
                if(ls_ncsa_extra_field.matchAll(extra, &extra_fields)) {

//                     for(size_t i=0;i<extra_fields.size();i++) {
//                         debugLog("extra fields %d: %s", i, extra_fields[i].c_str());
//                     }

                    if(!extra_fields.empty() && !extra_fields[0].empty()) {
                        entry.pid = extra_fields[0];

                        if(entry.pid.size()>=2 && entry.pid[0] == '"' && entry.pid[entry.pid.size()-1] == '"') {
                            entry.pid = entry.pid.substr(1, entry.pid.size()-2);
                        }
                    }
                }
            }
        }
    }

    //successful if response code less than 400
    int code = atoi(entry.response_code.c_str());

    entry.setSuccess();
    entry.setResponseColour();

    return entry.validate();
}

