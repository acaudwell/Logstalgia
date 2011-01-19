/*
    Copyright (c) 2009 Andrew Caudwell (acaudwell@gmail.com)
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:
    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
    3. The name of the author may not be used to endorse or promote products
       derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
    OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
    IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
    NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
    THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "seeklog.h"

long gSeekLogMaxBufferSize = 104857600;

//StreamLog

StreamLog::StreamLog() {
    this->stream = &std::cin;

    fcntl_fail = false;

#ifdef _WIN32
    stdin_handle = GetStdHandle(STD_INPUT_HANDLE);
#else
    int ret = fcntl(STDIN_FILENO, F_GETFL, 0);

    if (fcntl (STDIN_FILENO, F_SETFL, ret | O_NONBLOCK) < 0) {
        debugLog("fcntl(stdin) failed\n");
        fcntl_fail = true;
    }
#endif
}

StreamLog::~StreamLog() {
}

bool StreamLog::getNextLine(std::string& line) {

    //try and fix the stream
    if(isFinished()) stream->clear();

#ifdef _WIN32

    DWORD available_bytes;

    if (!PeekNamedPipe(stdin_handle, 0, 0, 0,
        &available_bytes, 0)) return false;

    if(available_bytes==0) return false;

#endif

    std::getline(*stream, line);

    //remove carriage returns
    if (line.size() > 0 && line[line.size()-1] == '\r') {
        line.resize(line.size() - 1);
    }

    if(isFinished()) {
        return false;
    }

    return true;
}

bool StreamLog::isFinished() {

    if(fcntl_fail || stream->fail() || stream->eof()) {
        return true;
    }

    return false;
}

// SeekLog

SeekLog::SeekLog(std::string logfile) {
    this->logfile = logfile;

    this->stream = 0;

    if(!readFully()) {
        throw SeekLogException(logfile);
    }
}

bool SeekLog::readFully() {

    if(stream!=0) delete stream;

    std::ifstream* file = new std::ifstream(logfile.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
    file_size = file->tellg();

    if(!file->is_open()) return false;

    file->seekg (0, std::ios::beg);

    //dont load into memory if larger than
    if(file_size > gSeekLogMaxBufferSize) {
        stream = file;
        return true;
    }

    //buffer entire file into memory
    char* filebuffer = new char[file_size+1];

    if(!file->read(filebuffer, file_size)) {
        file->close();
        delete file;
        return false;
    }
    filebuffer[file_size] = '\0';

    file->close();
    delete file;

    stream = new std::istringstream(std::string(filebuffer));

    delete[] filebuffer;

    return true;
}

SeekLog::~SeekLog() {
    if(stream!=0) delete stream;
}

float SeekLog::getPercent() {
    return current_percent;
}

void SeekLog::setPointer(std::streampos pointer) {
    stream->seekg(pointer);
}

std::streampos SeekLog::getPointer() {
    return stream->tellg();
}

void SeekLog::seekTo(float percent) {

    if(isFinished()) stream->clear();

    std::streampos mem_pointer = (std::streampos) (percent * file_size);

    setPointer(mem_pointer);
    
    //throw away end of line
    if(mem_pointer!=0) {
        std::string eol;
        getNextLine(eol);
    }
}

bool SeekLog::getNextLine(std::string& line) {

    //try and fix the stream
    if(isFinished()) stream->clear();

    std::getline(*stream, line);
    
    //remove carriage returns
    if (line.size() > 0 && line[line.size()-1] == '\r') {
        line.resize(line.size() - 1);
    }

    if(stream->fail()) {
        return false;
    }

    current_percent = (float) stream->tellg() / file_size;
    //debugLog("current_percent = %.2f\n", current_percent);

    return true;
}

// temporarily move the file pointer to get a line somewhere else in the file
bool SeekLog::getNextLineAt(std::string& line, float percent) {
    stream->clear();
    
    std::streampos currpointer = getPointer();

    seekTo(percent);

    bool success = getNextLine(line);

    stream->clear();

    //set the pointer back
    setPointer(currpointer);
    
    return success;
}

bool SeekLog::isFinished() {
    bool finished = false;

    if(stream->fail() || stream->eof()) {
        debugLog("stream is finished\n");
        finished=true;
    }

    return finished;
}
