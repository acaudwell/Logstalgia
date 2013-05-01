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

#ifdef _WIN32
std::string win32LogSelector() {
    OPENFILENAME ofn;

    char filepath[_MAX_PATH];
    filepath[0] = '\0';

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = 0;
    ofn.lpstrFile = filepath;
    ofn.nMaxFile = sizeof(filepath);
    ofn.lpstrFilter = "Website Access Log\0*.log\0*.*\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = 0;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = 0;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    GetOpenFileName(&ofn);

    return std::string(filepath);
}
#endif

int main(int argc, char *argv[]) {

    //defaults
    int   width       = 1024;
    int   height      = 768;
    bool  fullscreen  = false;

#ifdef _RPI
    bcm_host_init();           
    atexit(bcm_host_deinit);
#endif

    float simu_speed  = 1.0f;
    float update_rate = 2.0f;
    bool multisample  = false;

    vec3f background = vec3f(0.0, 0.0, 0.0);

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

        if(args == "--hide-url-prefix") {
            gHideURLPrefix = true;
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

        if(args == "--paddle-mode") {

            if((i+1)>=arguments.size()) {
                logstalgia_quit("specify paddle-mode (vhost,pid)");
            }

            std::string paddle_mode = arguments[++i];

            if(paddle_mode == "single") {
                gPaddleMode = PADDLE_SINGLE;

            } else if(paddle_mode == "pid") {
                gPaddleMode = PADDLE_PID;

            } else if(paddle_mode == "vhost") {
                gPaddleMode = PADDLE_VHOST;

            } else {
                logstalgia_quit("invalid paddle-mode");

            }

            continue;
        }

        if(args == "--paddle-position") {

            if((i+1)>=arguments.size()) {
                logstalgia_quit("specify paddle-position (0.25 - 0.75)");
            }

            gPaddlePosition = atof(arguments[++i].c_str());

            if(gPaddlePosition < 0.25f || gPaddlePosition > 0.75f) {
                logstalgia_quit("paddle-position outside of range 0.25 - 0.75");
            }

            continue;
        }

        if(args == "--font-size") {

            if((i+1)>=arguments.size()) {
                logstalgia_quit("specify font-size (10 - 40)");
            }

            gFontSize = atoi(arguments[++i].c_str());

            if(gFontSize < 10 || gFontSize > 40) {
                logstalgia_quit("font-size outside of range 10 - 40");
            }

            continue;
        }

        if(args == "-b" || args == "--background") {

            if((i+1)>=arguments.size()) {
                logstalgia_quit("specify background colour (FFFFFF)");
            }

            int r,g,b;
            std::string colstring = arguments[++i];

            if(colstring.size()==6 && sscanf(colstring.c_str(), "%02x%02x%02x", &r, &g, &b) == 3) {
                background = vec3f(r,g,b);
                background /= 255.0f;
            } else {
                logstalgia_quit("invalid colour string");
            }

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
            gPaddleMode = PADDLE_NONE;
            continue;
        }
        
        //no paddle tokens
        if(args == "--hide-paddle-tokens") {
          gPaddleTokens = PADDLE_TOKENS_HIDDEN;
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

        if(args == "--stop-position") {

            if((i+1)>=arguments.size()) {
                logstalgia_quit("specify stop-position (0.0 - 1.0)");
            }

            gStopPosition = atof(arguments[++i].c_str());

            if(gStopPosition<=0.0 || gStopPosition>1.0) {
                logstalgia_quit("stop-position outside of range 0.0 - 1.0");
            }

            continue;
        }

        //disable automatic skipping of empty time periods
        if(args == "--disable-auto-skip") {
            gAutoSkip = false;
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
                logstalgia_quit("glow-duration outside of range 0.0 - 1.0");
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

        if(args == "--sync") {
            gSyncLog = true;
            if(!logfile.size()) logfile = "-";
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

#ifdef _WIN32
    if(!logfile.size()) {
        //open file dialog
        logfile = win32LogSelector();

        if(!logfile.size()) return 0;
    }
#endif

    if(!logfile.size()) logstalgia_quit("no file supplied");

    // wait for a character on the file handle if reading stdin
    if(logfile == "-") {

#ifdef _WIN32
        DWORD available_bytes;
        HANDLE stdin_handle = GetStdHandle(STD_INPUT_HANDLE);

        while(PeekNamedPipe(stdin_handle, 0, 0, 0,
            &available_bytes, 0) && available_bytes==0 && !std::cin.fail()) {
            SDL_Delay(100);
        }
#else
        while(std::cin.peek() == EOF && !std::cin.fail()) SDL_Delay(100);
#endif
        std::cin.clear();
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

        ls->setBackground(background);

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
