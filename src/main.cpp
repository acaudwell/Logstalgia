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

    int video_framerate = 60;
    std::string ppm_file_name;

    std::string logfile = "";

    std::vector<std::string> groupstr;

    std::vector<std::string> arguments;

    SDLAppInit("Logstalgia", "logstalgia");

    SDLAppParseArgs(argc, argv, &width, &height, &fullscreen, &arguments);

    for (int i=0; i<arguments.size(); i++) {

        std::string args = arguments[i];

        if(args == "-h" || args == "-?" || args == "--help") {
            logstalgia_help();
        }

        if(args == "-c") {
            gSplash = 15.0f;
            continue;
        }

        if(args == "-x" || args == "--full-hostnames") {
            gMask = false;
            continue;
        }

        if(args == "-s" || args == "--speed") {

            if((i+1)>=arguments.size()) {
                logstalgia_quit("specify speed (1 to 30)");
            }

            simu_speed = atof(arguments[++i].c_str());

            if(simu_speed < 1.0f || simu_speed > 30.0f) {
                logstalgia_quit("speed should be between 1 and 30\n");
            }

            continue;
        }

        if(args == "-u" || args == "--update-rate") {

            if((i+1)>=arguments.size()) {
                logstalgia_quit("specify update rate (1 to 60)");
            }

            update_rate = atof(arguments[++i].c_str());

            if(update_rate < 1.0f || update_rate > 60.0f) {
                logstalgia_quit("update rate should be between 1 and 60\n");
            }

            continue;
        }

        //specify page url group
        if(args == "-g") {

            if((i+1)>=arguments.size()) {
                logstalgia_quit("specify group definition");
            }

            groupstr.push_back(arguments[++i].c_str());

            continue;
        }

        //dont bounce
        if(args == "--no-bounce") {
            gBounce = false;
            continue;
        }

        //dont draw response code
        if(args == "--hide-response-code") {
            gResponseCode = false;
            continue;
        }

        //no paddle
        if(args == "--hide-paddle") {
            gPaddle = false;
            continue;
        }

        if(args == "--start-position") {

            if((i+1)>=arguments.size()) {
                logstalgia_quit("specify start-position (0.0 - 1.0)");
            }

            gStartPosition = atof(arguments[++i].c_str());

            if(gStartPosition<=0.0 || gStartPosition>=1.0) {
                logstalgia_quit("start-position outside of range 0.0 - 1.0 (non-inclusive)");
            }

            continue;
        }

        //disable progress bar
        if(args == "--disable-progress") {
            gDisableProgress = true;
            continue;
        }

        if(args == "--disable-glow") {
            gDisableGlow = true;
            continue;
        }

        //enable multisampling
        if(args == "--multi-sampling") {
            multisample = true;
            continue;
        }

        if(args == "--glow-duration") {
            if((i+1)>=arguments.size()) {
                logstalgia_quit("specify glow-duration (float)");
            }

            gGlowDuration = atof(arguments[++i].c_str());

            if(gGlowDuration<=0.0 || gGlowDuration>1.0) {
                logstalgia_quit("invalid glow-intensity value");
            }

            continue;
        }

        if(args == "--glow-intensity") {
            if((i+1)>=arguments.size()) {
                logstalgia_quit("specify glow-intensity (float)");
            }

            gGlowIntensity = atof(arguments[++i].c_str());

            if(gGlowIntensity<=0.0) {
                logstalgia_quit("invalid glow-intensity value");
            }

            continue;
        }

        if(args == "--glow-multiplier") {

            if((i+1)>=arguments.size()) {
                logstalgia_quit("specify glow-multiplier (float)");
            }

            gGlowMultiplier = atof(arguments[++i].c_str());

            if(gGlowMultiplier<=0.0) {
                logstalgia_quit("invalid glow-multiplier value");
            }

            continue;
        }

        if(args == "--output-ppm-stream") {

            if((i+1)>=arguments.size()) {
                logstalgia_quit("specify ppm output file or '-' for stdout");
            }

            ppm_file_name = arguments[++i];

#ifdef _WIN32
            if(ppm_file_name == "-") {
                logstalgia_quit("stdout PPM mode not supported on Windows");
            }
#endif

            continue;
        }

        if(args == "--output-framerate") {

            if((i+1)>=arguments.size()) {
                logstalgia_quit("specify framerate (25,30,60)");
            }

            video_framerate = atoi(arguments[++i].c_str());

            if(   video_framerate != 25
               && video_framerate != 30
               && video_framerate != 60) {
                logstalgia_quit("supported framerates are 25,30,60");
            }

            continue;
        }

        //if given a non option arg treat it as a file, or if it is '-', pass that too (stdin)
        if(args == "-" || args.size() >= 1 && args[0] != '-') {
            logfile = args;
            continue;
        }

        // unknown argument
        std::string arg_error = std::string("unknown option ") + std::string(args);

        logstalgia_quit(arg_error);
    }

    if(!logfile.size()) logstalgia_quit("no file supplied");

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

    //init frame exporter
    FrameExporter* exporter = 0;

    if(ppm_file_name.size() > 0) {

        try {

            exporter = new PPMExporter(ppm_file_name);

        } catch(PPMExporterException& exception) {

            char errormsg[1024];
            snprintf(errormsg, 1024, "could not write to '%s'", exception.what());

            logstalgia_quit(errormsg);
        }
    }

    if(multisample) glEnable(GL_MULTISAMPLE_ARB);

    Logstalgia* ls = 0;

    try {
        ls = new Logstalgia(logfile, simu_speed, update_rate);

        //init frame exporter
        if(ppm_file_name.size() > 0) {
            ls->setFrameExporter(exporter, video_framerate);
        }

        for(size_t i=0;i<groupstr.size();i++) {
            ls->addGroup(groupstr[i]);
        }

        ls->run();

    } catch(ResourceException& exception) {

        char errormsg[1024];
        snprintf(errormsg, 1024, "failed to load resource '%s'", exception.what());

        logstalgia_quit(errormsg);

    } catch(SDLAppException& exception) {

        if(exception.showHelp()) {
            logstalgia_help();
        } else {
            logstalgia_quit(exception.what());
        }

    }

    if(ls!=0) delete ls;

    if(exporter!=0) delete exporter;

    display.quit();

    return 0;
}
