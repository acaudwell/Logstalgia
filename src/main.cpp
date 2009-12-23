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

#include "main.h"

int main(int argc, char *argv[]) {

    //defaults
    int   width       = 1024;
    int   height      = 768;
    bool  fullscreen  = false;

    float simu_speed  = 1.0f;
    float update_rate = 5.0f;
    bool multisample  = false;

    std::string logfile = "";

    std::vector<std::string> groupstr;

    std::vector<std::string> arguments;

    SDLAppParseArgs(argc, argv, &width, &height, &fullscreen, &arguments);

    for (int i=0; i<arguments.size(); i++) {

        std::string args = arguments[i];

        if(args == "-h" || args == "-?" || args == "--help") {
            logstalgia_help("");
        }

        if(args == "-c") {
            gSplash = 15.0f;
            continue;
        }

        if(args == "-x") {
            gMask = false;
            continue;
        }

        if(args == "-s") {

            if((i+1)>=arguments.size()) {
                logstalgia_help("specify speed (1 to 30)");
            }

            simu_speed = atof(arguments[++i].c_str());

            if(simu_speed < 1.0f || simu_speed > 30.0f) {
                logstalgia_help("speed should be between 1 and 30\n");
            }

            continue;
        }

        if(args == "-u") {

            if((i+1)>=arguments.size()) {
                logstalgia_help("specify update rate (1 to 60)");
            }

            update_rate = atof(arguments[++i].c_str());

            if(update_rate < 1.0f || update_rate > 60.0f) {
                logstalgia_help("update rate should be between 1 and 60\n");
            }

            continue;
        }

        //specify page url group
        if(args == "-g") {

            if((i+1)>=arguments.size()) {
                logstalgia_help("specify group definition");
            }

            groupstr.push_back(arguments[++i].c_str());

            continue;
        }

        //dont bounce
        if(args == "-b") {
            gBounce = false;
            continue;
        }

        //dont draw response code
        if(args == "-r") {
            gResponseCode = false;
            continue;
        }

        //no paddle
        if(args == "-p") {
            gPaddle = false;
            continue;
        }

        if(args == "--start-position") {

            if((i+1)>=arguments.size()) {
                logstalgia_help("specify start-position (0.0 - 1.0)");
            }

            gStartPosition = atof(arguments[++i].c_str());

            if(gStartPosition<=0.0 || gStartPosition>=1.0) {
                logstalgia_help("start-position outside of range 0.0 - 1.0 (non-inclusive)");
            }

            continue;
        }

        //disable progress bar
        if(args == "--disable-progress") {
            gDisableProgress = true;
            continue;
        }

        //enable multisampling
        if(args == "--multi-sampling") {
            multisample = true;
            continue;
        }

        //if given a non option arg treat it as a file, or if it is '-', pass that too (stdin)
        if(args == "-" || args.size() >= 1 && args[0] != '-') {
            logfile = args;
            continue;
        }
    }

    //wait for data before launching
    if(logfile.compare("-") == 0) {

        char line[1024];
        while( fgets(line, sizeof(line), stdin) == NULL ) {
            SDL_Delay(100);
        }
    }

    //enable vsync
    display.enableVsync(true);

    // this causes corruption on some video drivers
    if(multisample) display.multiSample(4);

    display.init("Logstalgia", width, height, fullscreen);

    if(multisample) glEnable(GL_MULTISAMPLE_ARB);

    Logstalgia* ls = new Logstalgia(logfile, simu_speed, update_rate);

    for(size_t i=0;i<groupstr.size();i++) {
        ls->addGroup(groupstr[i]);
    }

    ls->run();

    delete ls;

    display.quit();

    return 0;
}
