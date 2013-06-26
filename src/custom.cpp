#include "custom.h"

#include "core/regex.h"

//timestamp
//hostname
//path
//response_code
//response_size
//success (1|0)
//response_colour
//referrer
//user_agent
//virtual_host
//pid

Regex custom_entry("^([^|]*)\\|([^|]*)\\|([^|]*)\\|([^|]*)\\|([^|]*)(?:\\|([^|]*))?(?:\\|#?([^|]*))?(?:\\|([^|]*))?(?:\\|([^|]*))?(?:\\|([^|]*))?(?:\\|([^|]*))?$");

CustomAccessLog::CustomAccessLog() {
}

bool CustomAccessLog::parseLine(std::string& line, LogEntry& entry) {

    std::vector<std::string> matches;

    if(!custom_entry.match(line, &matches)) return false;

    entry.timestamp = atol(matches[0].c_str());
    entry.hostname  = matches[1];
    entry.path      = matches[2];
    entry.response_code = matches[3];
    entry.response_size = atol(matches[4].c_str());

    //optional fields

    //success 1 or 0
    if(matches.size() > 5) {
        std::string success = matches[5];

        if(success.empty() || (success.size()==1 && success[0] == ' ')) {
            entry.setSuccess();
        } else {
            entry.successful = atoi(success.c_str())==1 ? true : false;
        }
    } else entry.setSuccess();

    //response colour
    if(matches.size()>6) {

        std::string colour = matches[6];

        int r, g, b;
        if(colour.size()>0 &&
           sscanf(colour.c_str(), "%02x%02x%02x", &r, &g, &b) == 3) {
            entry.response_colour = vec3( r, g, b );
            entry.response_colour /= 255.0f;
        } else {
            entry.setResponseColour();
        }
    } else entry.setResponseColour();

    //referrer
    if(matches.size()>7) {
        entry.referrer   = matches[7];
    }

    //user agent
    if(matches.size()>8) {
        entry.user_agent = matches[8];
    }

    //vhost
    if(matches.size()>9) {
        entry.vhost = matches[9];
    }

    //pid or some other identifier
    if(matches.size()>10) {
        entry.pid = matches[10];
    }

    return entry.validate();
}
