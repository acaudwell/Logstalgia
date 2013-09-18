# Logstalgia

A website access log visualisation tool. Copyright &copy; 2008 [Andrew Caudwell](mailto:acaudwell@gmail.com). 

[code.google.com/p/logstalgia](http://code.google.com/p/logstalgia/) and [github.com/acaudwell/Logstalgia](https://github.com/acaudwell/Logstalgia)


## Contents

1. Description
2. Requirements
3. Using Logstalgia
4. Copyright


## 1. Description

Logstalgia is a visualization tool that replays or streams web server access logs as a retro arcade game simulation.


## 2. Requirements

Logstalgia's display is rendered using OpenGL and requires a 3D accelerated video card to run.

Logstalgia supports several standardized access.log formats used by web servers such as Apache and Nginx (see 'Supported Log Formats' below).

As Logstalgia is designed to playback logs in real time you will need a log from a fairly busy webserver to achieve interesting results (e.g. 100s of requests each minute).


## 3. Using Logstalgia

        $ logstalgia [options] logfile


### Options

        -f      Fullscreen.

        -WxH    Set the window size. If -f is also supplied, will attempt to set the
                video mode to this also. Add ! to make the window non-resizable.

        -b, --background FFFFFF
                Background colour in hex.

        -x, --full-hostnames
                Show full request ip/hostname.

        -s, --simulation-speed
                Simulation speed. Defaults to 1 (1 second-per-second).

        -p, --pitch-speed
                Speed balls travel across the screen (defaults to 0.15).

        -u, --update-rate
                Page Summary update speed. Defaults to 5 (5 seconds).

        -g name,(HOST|URI|CODE)=regex,percent[,colour]

                Creates a new named summarizer group for requests for which a
                specified attribute (HOST, URI or response CODE) matches a
                regular expression. Percent specifies a vertical percentage of
                screen to use.

                A colour may optionally be supplied in hexadecimal format
                (eg FF0000 for red) which will be applied to all labels
                and request balls matched to the group.

                Examples:

                 -g "HTML,URI=html?$,30"
                 -g "Lan,HOST=^192,30"
                 -g "Success,CODE=^[23],30"

                If no groups are specified, the default groups are Images
                (image files), CSS (.css files) and Scripts (.js files).

                If there is enough space remaining a catch-all group 'Misc'
                will appear as the last group.

        --paddle-mode MODE
                Paddle mode (pid, vhost, single).

                vhost  - separate paddle for each virtual host in the log file.

                pid    - separate paddle for each process id in the log file.

                single - single paddle (the default).

        --paddle-position POSITION
                Paddle position as a fraction of the view width (0.25 - 0.75).

        --sync  Read from STDIN, ignoring entries before the current time.

        --start-position POSITION
                Begin at some position in the log file (between 0.0 and 1.0).

        --stop-position POSITION
                Stop at some position.

        --no-bounce
                No bouncing.

        --hide-response-code
                Hide response code.

        --hide-paddle
                Hide paddle.
    
        --hide-paddle-tokens
                Hide paddle tokens shown in multi-paddle modes.

        --hide-url-prefix
                Hide URL protocol and hostname prefix of requests.

        --disable-auto-skip
                Disable automatic skipping of empty time periods.

        --disable-progress
                Disable the progress bar.

        --disable-glow
                Disable the glow effect.

        --font-size SIZE
                Font size (10 - 40).

        --glow-duration
                Duration of the glow (between 0.0 and 1.0).

        --glow-multiplier
                Adjust the amount of glow.

        --glow-intensity
                Intensity of the glow.

        -o, --output-ppm-stream FILE
                Write frames as PPM to a file (?-? for STDOUT).

        -r, --output-framerate FPS
                Framerate of output (used with --output-ppm-stream).

        --load-config CONFIG_FILE
                Load a config file.

        --save-config CONFIG_FILE
                Save a config file with the current options.

        logfile
                The path to the access log file to read or '-' if you wish to
                supply log entries via STDIN.


### Examples

Watch an example access.log file using the default settings:

        $ logstalgia data/example.log

Watch the live access.log, starting from the most recent batch of entries in the log (requires tail). Note than '-' at the end is required for logstalgia to know it needs to read from STDIN:

        $ tail -f /var/log/apache2/access.log | logstalgia -

To follow the log in real time, use the --sync option. This will start reading from the next entry received on STDIN:

        $ tail -f /var/log/apache2/access.log | logstalgia --sync

Watch a remote access.log via ssh:

        $ ssh user@example.com tail -f /var/log/apache2/access.log | logstalgia --sync


### Supported Log Formats

Logstalgia supports the following standardized log formats used by web servers like Apache and Nginx:

NCSA Common Log Format (CLF):

        "%h %l %u %t \"%r\" %>s %b"

NCSA Common Log Format with Virtual Host:

        "%v %h %l %u %t \"%r\" %>s %b"

NCSA extended/combined log format:

        "%h %l %u %t \"%r\" %>s %b \"%{Referer}i\" \"%{User-agent}i\""

NCSA extended/combined log format with Virtual Host:

        "%v %h %l %u %t \"%r\" %>s %b \"%{Referer}i\" \"%{User-agent}i\""

The process id (%P), or some other identifier, may be included as an additional field at the end of the entry. This can be used with '--paddle-mode pid' where a separate paddle will be created for each unique value in this field.


### Custom Log Format:

Logstalgia now supports a pipe ('|') delimited custom log file format:

        timestamp       - unix timestamp of the request date.
        hostname        - hostname of the request
        path            - path requested
        response_code   - the response code from the webserver (eg 200)
        response_size   - the size of the response in bytes

The following are optional:

        success         - 1 or 0 to indicate if successful
        response_colour - response colour in hexidecial (#FFFFFF) format
        referrer url    - the referrer url
        user agent      - the user agent
        virtual host    - the virtual host (to use with --paddle-mode vhost)
        pid             - process id or some other identifier (--paddle-mode pid)

If _success_ or _response\_colour_ are not provided, they will be derived from the _response\_code_ using the normal HTTP conventions (code < 400 = success).


### Recording Videos

See the guide on the homepage for examples of recording videos with Logstalgia: [code.google.com/p/logstalgia/wiki/Videos](http://code.google.com/p/logstalgia/wiki/Videos)


### Interface

The time shown in the top left of the screen is set initially from the first log entry read and is incremented according to the simulation speed (-s).

The counter in the bottom right hand corner shows the number of requests displayed since the start of the current session.

Pressing space at any time will pause/unpause the simulation. While paused you may use the mouse to inspect the detail of individual requests.

Interactive keyboard commands:

        (C)   Displays Logstalgia logo
        (N)   Jump forward in time to next log entry
        (+-)  Adjust simulation speed
        (<>)  Adjust pitch speed

        (Alt+Enter) Toggle Fullscreen
   
        (ESC) Quit


## 4. Copyright

Logstalgia - A website access log visualisation tool. Copyright &copy; 2008 [Andrew Caudwell](mailto:acaudwell@gmail.com).

[code.google.com/p/logstalgia](http://code.google.com/p/logstalgia/) and [github.com/acaudwell/Logstalgia](https://github.com/acaudwell/Logstalgia)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
