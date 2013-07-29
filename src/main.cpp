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

#include "logstalgia.h"
#include "settings.h"

#ifdef _WIN32
std::string win32LogSelector() {

    //get original directory
    char cwd_buff[1024];

    if(getcwd(cwd_buff, 1024) != cwd_buff) {
        SDLAppQuit("error getting current working directory");
    }

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

    //change back to original directory
    if(chdir(cwd_buff) != 0) {
        SDLAppQuit("error changing directory");
    }

    return std::string(filepath);
}
#endif

int main(int argc, char *argv[]) {

    ConfFile conf;
    std::vector<std::string> files;

    SDLAppInit("Logstalgia", "logstalgia");

#ifdef _WIN32
        SDLApp::initConsole();
#endif

    try {
        settings.parseArgs(argc, argv, conf, &files);

        //set log level
        Logger::getDefault()->setLevel(settings.log_level);

#ifdef _WIN32
        // hide console if not needed
        if(settings.log_level == LOG_LEVEL_OFF && !SDLApp::existing_console) {
            SDLApp::showConsole(false);
        }
#endif
        //load config
        if(!settings.load_config.empty()) {
            conf.clear();
            conf.load(settings.load_config);

            //apply args to loaded conf file
            settings.parseArgs(argc, argv, conf);
        }

        if(!files.empty()) {
            std::string path = files[files.size()-1];

            ConfSectionList* sectionlist = conf.getSections("logstalgia");

            if(sectionlist!=0) {
                for(ConfSectionList::iterator sit = sectionlist->begin(); sit != sectionlist->end(); sit++) {
                    (*sit)->setEntry("path", path);
                }
            } else {
                conf.setEntry("logstalgia", "path", path);
            }
        }

        //apply the config / see if its valid
        settings.importDisplaySettings(conf);
        settings.importLogstalgiaSettings(conf);

        //save config
        if(!settings.save_config.empty()) {
            conf.save(settings.save_config);
            exit(0);
        }


    } catch(ConfFileException& exception) {

        SDLAppQuit(exception.what());
    }

#ifdef _WIN32
    if(settings.path.empty()) {

        //open file dialog
        settings.path = win32LogSelector();

        //TODO chdir back to original directory

        if(settings.path.empty()) return 0;
    }
#endif

    if(settings.path.empty()) SDLAppQuit("no file supplied");

    //enable vsync
    display.enableVsync(settings.vsync);

    // this causes corruption on some video drivers
    if(settings.multisample) display.multiSample(4);

    if(settings.resizable && settings.output_ppm_filename.empty()) {
        display.enableResize(true);
    }

    display.init("Logstalgia", settings.display_width, settings.display_height, settings.fullscreen);

    // Don't minimize when alt-tabbing so you can fullscreen logstalgia on a second monitor
#if SDL_VERSION_ATLEAST(2,0,0)
     SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");
#endif
     
    //disable OpenGL 2.0 functions if not supported
    if(!GLEW_VERSION_2_0) settings.ffp = true;
    
    //init frame exporter
    FrameExporter* exporter = 0;

    if(!settings.output_ppm_filename.empty()) {

        try {

            exporter = new PPMExporter(settings.output_ppm_filename);

        } catch(PPMExporterException& exception) {

            char errormsg[1024];
            snprintf(errormsg, 1024, "could not write to '%s'", exception.what());

            SDLAppQuit(errormsg);
        }
    }

    if(settings.multisample) glEnable(GL_MULTISAMPLE_ARB);

    Logstalgia* ls = 0;

    try {
        ls = new Logstalgia(settings.path);

        if(exporter != 0) {
            ls->setFrameExporter(exporter);
        }

        for(const std::string& group : settings.groups) {
            ls->addGroup(group);
        }

        ls->setBackground(settings.background_colour);

        ls->run();

    } catch(ResourceException& exception) {

        char errormsg[1024];
        snprintf(errormsg, 1024, "failed to load resource '%s'", exception.what());

        SDLAppQuit(errormsg);

    } catch(SDLAppException& exception) {

        if(exception.showHelp()) {
            settings.help();
        } else {
            SDLAppQuit(exception.what());
        }

    }

    if(ls!=0) delete ls;

    if(exporter!=0) delete exporter;

    display.quit();

    return 0;
}
