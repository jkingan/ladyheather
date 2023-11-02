//
// Lady Heather's GPS Disciplined Oscillator Control Program
// (now works with many different receiver types and even without a GPS
//  receiver.  Also works with precision rubidium/cesium oscillators and
//  frequency / time interval counters).
//
// Copyright (C) 2008-2018 Mark S. Sims
//
//
// Lady Heather is a monitoring and control program for GPS receivers, GPS
// Disciplined Oscillators, precision oscillators (rubidium and cesiem), and
// frequency / time interval counters.  Heather is oriented more towards the
// time keeping functionality of GPS and less towards positioning. It supports
// numerous GPS (and Glonass, Beidou, Galileo, etc) devices including:
//
//      GPS Receivers:
//         Ashtech Z12 receiver
//         Furuno GT-8031 (uses $PFEC commands)
//         Furuno GT-87xx (using eSIP commands)
//         GPSD interface (mainly a Linux/macOS/FreeBSD thing - GPSD provides
//              a shared read-only interface to numerous GPS devices.
//         Jupiter-T  (aka Zodiac)
//         Motorola binary
//         NMEA
//         NVS BINR binary (115200:8:N:1)
//         Sirf binary
//         Trimble TSIP binary receivers
//         Trimble TAIP receivers
//         Trimble SV6/SV8/ACE-III
//         Trimble Accutime / Palisade receivers
//         Trimble RT17 format receivers (like the NetRS)
//         TymServe 2000 (not yet tested)
//         Ublox UBX binary
//         Venus/Navspark (115200:8:N:1)
//
//      GPSDO's (GPS disciplined oscillators):
//         Brandywine GPS-4 receiver
//         DATUM STARLOC II GPSDO - inferior wannabe Thunderbolt
//         Jackson Labs LTE Lite
//         Lars simple GPSDO controller (used with any 1PPS output GPS receiver)
//         Lucent RFTG-m GPSDO
//         Lucent KS24361 REF0/Z3811A  Z3812A
//         NEC GPSDO ... STAR-4 compatible at 115,200 baud
//         Oscilloquartz STAR-4 GPSDO (management interface)
//         Oscilloquartz OSA-453x GPSDO
//         SCPI - Nortel telecom GPSDOs like NTWB and NTPX in SCPI mode
//         SCPI (Z3801A/Z38015/Z3816/etc style)
//         SCPI (HP5xxxx style)
//         Spectrum TM4  (not fully tested)
//         Trimble TSIP binary GPSDOs (like the Thunderbolt and numerous
//           "telecom" GPSDOs.
//         TruePosition GPS
//         UCCM - small Trimble / Symmetricom / Samsung telecom GPSDOs
//           (57600 baud)
//         Zyfer Nanosync 380 (19200:8:N:1)
//
//     Atomic frequency references:
//        HP 5071A cesium beam oscillator
//        Spectratime/Temex LPFRS rubidium
//        Spectratime/Temex RMO rubidium  (same as LPFRS)
//        Spectratime SRO100/SRO70 rubidium
//        SRS PRS-10 rubidium oscillator
//        Symmetricom SA22 rubidium (60 Mhz and 58.9824 MHz ref freq)
//        Symmetricom X72 rubidium
//        Symmetricom X99 rubidium
//
//     Clocks:
//        Acron Zeit WWVB receiver
//        Gravity/solid earth tide clock (uses system clock to display
//          solid earth tides and gravity offset,  Requires manual entry
//          of latitude/longitude/altitude)
//        No receiver, uses system clock.
//
//     Time and frequency counters:
//        Generic frequency/time interval counters
//        HP531xx counters
//        Lars simple GPSDO controller
//        PICPET simple timestamping interval counter chip
//        TAPR TICC time interval counter
//
//     Misc:
//        Simple terminal emulator
//
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
//
// Original Win32 port by John Miles, KE5FX (john@miles.io)
// Help with Mac OS X port by Jeff Dionne and Jay Grizzard
//   (note: OS/X version uses the XQuartz package for X11 display support)
// FreeBSD support by Daniel Lawrence
//
// Temperature and oscillator control algorithms by Warren Sarkison
//
// Original adev code adapted from Tom Van Baak's adev1.c and adev3.c
// Incremental adev code based upon John Miles' TI.CPP
//
// uGal gravity calculation code slightly adapted from Tom Van Baak's
// tides.c which was derived from code by J.L. Ahern which was based upon
// equations by P. Schureman
//
// Solid earth tide code translated from Fortran solid.f using the F2C
// translator program.  solid.f was written by Dennis Milbert and is based
// upon the dehandtideinel/MJD code by V. Dehandt,  S. Matherws,  J. Gipson,
// and C. Bruynix.
//
// Easter and moon illumination phase code from voidware.com
//
// moon_info() derived from code in John Walker's moontool.c
//
// Equation of time code adapted from code by Mike Chirico and NOAA
//
// Sun position code is Grena's Algorithm 5
//
// Moon position from code by Paul Schlyter at
//   http://hotel04.ausys.se/pausch/comp/ppcomp.html
//
// Heliocentric Julian Date code adapted from code by Richard Ogley.
//
// Nutation and obliquity code derived from code by Jay Tanner and JPL
//
// Equinox/Solstice equations derived from Jan Meeus "Astronomical Algorithms"
//    via Simon Cassidy.
//
// Time of perihelion derived from table by Fred Espenak, www.Astropixels.com
//
// New moon table derived from table by Fred Espenak, www.Astropixels.com
//
// Moon image code derived from code by Mostafa Kaisoun.
//
// Nortel NTGS55A recever wakeup research by Sam Sergi
//
// Linux IPv6 support by Kasper Pedersen
//
// The LZW .GIF encoder used in this program was derived from code written by
// Gershon Elber and Eric S. Raymond as part of the GifLib package.  And by
// Lachlan Patrick as part of the GraphApp cross-platform graphics library.
//
// Singing clock excerpted from Palistrina, Missa Assumpta by the Tallis
// Scholars...  you should buy some of their stuff... they are quite good.
//
//
//
// This file contains most of the operating system dependent routines,
// initialization routines,  serial port I/O,  and the screen I/O and
// plotting routines.
//
//
//
//
//
//
//
//-PROGRAM CONFIGURATION AND HELP INFORMATION
//
//   Heather has two types of user commands.  The first is the command line
//   options.  You can get a list of command line options by starting Heather
//   with an invalid command line option such as "/?".  This will bring up
//   a scroll box of commands on Windows and several pages of commands on
//   other operating systems.  You can also enter "?" from the keyboard to
//   bring up the command line help information.
//
//   Command line options can begin with either a '/' (Windows standard)
//   or a '-' (Linux / macOS standard).  Either is acceptable no matter what
//   operating system you are using.  Options must be separated by a space.
//   Note that file names may not contain spaces and quoted options are not
//   supported!
//
//   Once Heather has started, you can enter command line options from the
//   keyboard using the "/"  keyboard command.  There are a few command line
//   options that cannot be changed from the keyboard once Heather has started.
//
//
//
//   The second set of commands are the keyboard commands. You can get a list
//   of the primary keyboard commands and menus by pressing SPACE.  Note that
//   not all receivers support all of the keyboard commands.  Keyboard menu
//   commands are shown in this documentation in upper case characters, but
//   either upper case or lower case are acceptable.
//
//   Pressing the first key of a command menu will show a sub-menu of commands
//   available under that menu.  Pressing the second key of a menu command
//   will either execute the command or, more commonly, prompt for a value
//   to enter.  The default value offered as the input is either the current
//   setting or a common value to use.  You can press ENTER to accept the
//   offered value, ESC or ESC ESC to abort the command, enter a new value,
//   or edit the offered value.
//
//   LEFT and RIGHT arrows move the cursor within the string. INSERT
//   toggles insert mode.  HOME moves to the start of the string.  END moves
//   to the end of the string.  DEL deletes the character at the cursor.
//   DOWN arrow deletes to the end of the line.  UP arrow deletes to the
//   start of the line. BACKSPACE deletes the character before the cursor.
//   PAGE_UP retreives the last command entered. This lets you repeat the
//   last entered command value or recover from a botched line edit.
//
//   If a keyboard command suggests an input value and the first character
//   that you enter is a not an  editing character,  the offered value is
//   erased so that you don't have to backspace over it to enter a new value.
//
//
//
//   Heather's configuration comes from three places:
//
//      First is a hard coded configuration in the program.
//      Next is from the file "heather.cfg"
//      Finally is from the command line that starts the program.
//
//   The location of "heather.cfg" depends upon the operating system and how
//   Heather was started (from a command line or from a desktop icon).  You
//   can determine where to place the file by bringing up the command line
//   help information (described above) and scrolling down to the bottom of
//   the help information.  There will be a line that says:
//     "Put heather.cfg file in directory ..."
//   The location of the file is also shown (in GREEN) when you press SPACE
//   to get the keyboard help menu.
//
//   Note that if you rename the "heather" executable file,  the .cfg and .cal
//   file names that Heather uses will also change to match the new executable
//   file name.
//
//   Note that this directory is also the default directory that Heather uses
//   for all of its support files (sound files, log files, screen dumps, etc).
//
//   If you launch Heather from a WINDOWS desktop icon, you can set the command
//   line options by right clicking on the icon and selecting PROPERTIES.
//   The TARGET field will show the command command line to use.
//
//   Note that in this file a lot of command line options are described as
//   "toggle" options.  Toggle options are like on/off switches.  For instance:
//      /gw  - will toggle the watch display between on and off and back on
//             each time it is seen.
//
//   You can override this toggle action and explictly set the state.
//   For instance:
//      /gw0 - forces the watch display OFF
//      /gw1 - forces the watch display ON
//
//
//
//   LADY HEATHER'S CONFIGURATION FILE "heather.cfg" :
//
//   Place the command line options that you want to use in this file
//   with one command line option per line.  Each option MUST start in column
//   one with a '/', '-', '@', or '$' otherwise the line will be treated as
//   a comment.
//
//   Lines that begin with '/' or '-' set command line option values.
//
//   Lines that begin with '$' send hex values to the receiver.
//
//   Lines that begin with '@' send keyboard commands (all '@' lines are copied
//   to temporary keyboard script file "heathtmp.scr" which is then processed
//   once Heather has finished initializing the hardware, etc).  See the
//   description of keyboard script files below.
//
//   You can also read in a ".cfg" file from the keyboard "R" menu or from
//   the command line:
//      /r=file.cfg  - reads a .cfg configuration file. These config files are
//                     processed after the default "heather.cfg" file has been
//                     processed.  You should not include a "/r=file.cfg"
//                     command in a .cfg file since reading config files do
//                     not nest!
//      Note: earlier versions of Lady Heather used the /h command to read
//      .cfg files.   The /h command now displays command line help.
//
//
//   At a minimum, most users will want to configure the com port, receiver
//   type, and time zone in their default configuration.
//
//   If an error is detected in a command line option in the .cfg file,
//   processing of the file is stopped and the command line help screen is
//   shown.  The offending option is listed at the end of the help info.
//
//
//
//   KEYBOARD SCRIPT FILES:
//
//      Lady Heather has the ability to read keyboard script files.  These
//      file must have an extension of ".scr"  Script files mimic typing
//      from the keyboard.  Script file names must be less than 128 characters
//      long.
//
//      Commands that would normally suggest an input do not
//      do it when read from a script file.  They behave
//      like you first entered ESC to erase the suggestion.
//
//      Commands that normally toggle a flag may be followed
//      by a "1" or "0" to set the flag value directly. e.g.
//      you can use GW0 in a script file to force the watch display
//      OFF. A few commands (like GS) do not toggle a specific value
//      and cannot be used this way.
//
//      You can put more than one command on a line (separated
//      by spaces) until you do a command that expects a
//      parameter value or string.
//
//      A '~' in a script file pauses reading from the script
//      and starts reading from the keyboard until a carriage
//      return is seen.  This is useful for entering a parameter
//      value or string.  e.g.  GG~ will pause the script and
//      prompt for a graph title.
//
//      Any text following a ';' or a '#' in a script file
//      is a comment and is ignored.
//
//      Script files abort upon the first error detected or any
//      unrecognized command is seen.
//
//      Script files can be nested up to 5 levels deep.
//      Scripts can be stopped by pressing any key.
//
//      You can cause processing of a keyboard script file to wait for a
//      time interval or a specific time.  See the section on ALARMS for
//      a description of how to do this.
//
//      To run a script file use the "R" keyboard command or "/r" command
//      line option and specify a file name with an extension of ".scr"
//
//      The function keys F1..F10 (F1..F9 on Windows) cause keyboard script
//      files "f1.scr" to "f10.scr" to be read.  This allows F1..F10 to be
//      used as programmable function keys.
//
//      Heather normally puts a short pause between each character read from
//      a script file.  You can toggle "fast script" mode with the /nf command
//      line option.
//
//
//
//
//
//-CONFIGURING THE COM PORT:
//
//   Lady Heather talks to the GPS device via a serial port (hardware or USB)
//   or an internet TCP/IP connection.  You need to tell Heather which
//   communications device to use.  For serial ports you use the "/#" command
//   line option (where # is the com port number to use):
//      /1 says to use COM1
//      /2 says to use COM2, etc
//      /0 says to not use a communications link.  This can be useful if you
//         are just reading a log file, etc and do not want to talk to a receiver.
//
//   On Linux systems the # number you specify IS 1 MORE THAN THE SERIAL PORT
//   NUMBER:
//      /1 says to use /dev/ttyS0 on Linux
//      /1 says to use /dev/ttys0 on macOS
//      /1 says to use /dev/cuaU0 on FreeBSD
//
//      /2 says to use /dev/ttyS1 on Linux
//      /2 says to use /dev/ttys1 on macOS
//      /2 says to use /dev/cuaU1 on FreeBSD
//
//      /999 says to use /dev/heather
//
//   Windows treats USB connected serial adapters as standard hardware serial
//   ports.  Most Linux and macOS users will be using USB serial converters.
//   These operating systems treat the USB devices differently than hardware
//   serial ports.  For these system you can use the "/#u" command line option.
//
//      /1u says to use /dev/ttyUSB0 on Linux systems and
//      /1u says to use /dec/tty.usbserial on macOS.
//      /1u says to use /dev/cuaU0 on FreeBSD
//
//      /2u says to use /dev/ttyUSB1 on Linux systems and
//      /2u says to use /dec/tty.usbserial1 on macOS.
//      /2u says to use /dev/cuaU1 on FreeBSD
//
//    You can also follow the com port number with an "a" to use a /dev/ttyACM
//    device:
//      /1a says to use /dev/tty/ACM0
//      /2a says to use /dev/tty/ACM1  ,etc.
//
//   If your system does not use these standard device names, you can specify
//   the input device name to use with the "/id=" command line option:
//      /id=/dev/your_device_name
//
//   Heather also supports using a TCP/IP connection to talk to your receiver.
//      /ip=addr:port
//      addr can be be either a numeric address like 192.168.10.20 or a web
//      address like ke5fx.dyndns.org
//
//      If the :port number is not given Heather uses port 45000
//
//   Note that if the specified IP address is not valid or cannot be reached
//   Heather may appear to hang for quite a while until/if the connection
//   attempt times out.  This is particularly true for numeric IP addresses.
//
//   On Linux and macOS Heather supports IPv6 addresses. If the specified
//   IP address is a "bracketed" IP address [....] you can specify the port
//   number to use like [....]:45000  otherwise you can force an IPv6
//   connection by using a ';' before the port number.
//
//
//   For hardware serial port and USB serial connections you can specify the
//   serial port baud rate and data format with the "/br=" command line option:
//      /br=9600:8:N:1   (9600 baud, 8 data bits, no parity, 1 stop bit)
//      /br=19200:7:E:1  (19200 baud, 7 data bits even parity, one stop bit)
//      /br=57600:8:O:2  (57600 baud, 8 data bits odd parity, two stop bits)
//
//   Many receivers that connect via a native USB port (not a USB-serial
//   converter cable) use a "CDC" driver that effectively ignores the baud
//   rate, etc parameters.  You can Heather's port config to anything and
//   they will still work.
//
//   There is another parameter (:Px) that you can add to the /br parameters
//   to select which external device (see the following section on external
//   devices for more information) that you want to configure.
//
//   If a baud rate parameter is not given in the "/br=" string, that
//   parameter is not changed.  Previous versions of Heather set that
//   parameter to a default value.
//
//   If a baud rate command is not given, Heather will use a default value
//   that is most commonly used for the receiver type it is using... which
//   may not be the same as what your receiver is configured for!
//
//   Another way to specify the baud rate is to follow the serial port number
//   with an ',' and the baud rate info like:
//       /1,9600:8:N:1
//
//
//   If com data is lost or the com port disappears Heather will attempt to
//   re-init the com port every "com_timeout" interval (5-25 seconds
//   depending upon the receiver type and the operation being done).
//   You can disable com port error recovery with the /ce command line option.
//   Heather can recover from things like yanked ethernet cables and USB
//   dongles and dropped WIFI connections.
//
//   You can specify the com port data loss timeout (in milliseconds) with
//   the "/ct=#" or "/ct?=#" command line option.  /ct sets the timeout for
//   all devices ports.  /ct? sets the timeout for just port '?' where '?'
//   is the port identifier for the port.  See the section below on the
//   external/extra ports for the supported port identifier characters.
//
//   The timeout value must be at least 3000 msecs. The default com timeout
//   is 5 seconds.  A few devices extend the com timeout values to 10 to 25
//   seconds while doing commands that take a long time to do (like receiver
//   diagnostics and hard resets).  If you don't use those commands,
//   the "/ct" commands allow you to set a shorter timeout.  Shorter timeouts
//   make automatic recovery from dropped com links faster.
//
//   Note that under Windows if you drag the window for more than the
//   com timeout period you will get a data loss error!
//
//   Heather now has the "!x" keyboard command for setting the receiver
//   baud rate for devices that support raw satellite data.
//   IF YOU SET THE RECEIVER TO A NON-DEFAULT VALUE YOU WILL NEED TO SPECIFY
//   THE BAUD RATE ON THE COMMAND LINE (/br=...) WHENEVER YOU START HEATHER.
//   If the receiver type can be auto-detected and the baud rate is one of
//   the supported auto-detect rates (9600,19200,38400,57600,115200 baud),
//   tnen you might not need to specify the baud rate.
//
//   You can also use the "brx=baud_rate" command line option to set the
//   receiver baud rate from the command line.  The /brr command MUST come
//   after the /rx?? command line option that sets the receiver type (so that
//   Heather knows which command to send to the device).
//
//   The !x and /brr commands currently only change the receiver baud rate.
//   They do NOT change the data_bits, stop_bits, or parity settings!
//
//
//
//-EXTERNAL / EXTRA DEVICE SUPPORT
//
//   Besides the main "receiver" device, Heather has support for and hooks
//   for adding extra devices (such as counters, environmental sensors, and
//   echoing device data to other devices).  These external device ports are
//   configured using the /e? command.
//
//   /ed  DAC/ADC port
//   /ee  receiver data echo port
//   /ef  temperature control device
//   /ei  secondary interval/frequency counter
//   /ek  NMEA echo port
//   /en  environmental sensor port
//   /er  the standard "receiver" device
//   /et  moon / sun / sat position tracking info
//   /e0 .. /e9 - for user defineable devices
//
//   The syntax for the /e? command is shown below using /ei as an example:
//
//      /ei=device
//
//   Where "device" can be a serial port descriptor like:
//      /ei=1   (com1)
//      /ei=1u  (usb0)
//      /ei=3,9600:8:N:1   (com3 at 9600 baud, 8 bits, no parity, 1 stop bit)
//
//      You can also configure the baud rate for the external devices by
//      using the ":P#" parameter on the /br= command like this example for
//      the NMEA echo (/ek) port:
//         /br=19200:8:N:1:PI
//
//   Where "device" can be an IP address AND a non-optional port number.
//   Heather uses the presence of a ':' in the device parameter to recognize
//   that it is a IP address.  The IP address must lead to a connection that
//   is lisenting for data, otherwise Heather can lock up waiting for the
//   connection or a timeout occurs.
//       /ei=123.135.146.157:45000
//       /ei=example.com:45000
//
//   Where "device" is a Linux style device name:
//       /ei=/dev/ttyUSB0
//       /ei=/dev/ttyUSB1,9600:8:N:1
//
//   The only external devices that Heather currently has full/tested support
//   are:
//
//       /ee - receiver echo port.  This port echoes the receiver data
//             stream to the port.  The default data format of the echoed
//             data is whatever the main "receiver" device is configured for.
//             Any data sent to the echo port by the connected device is
//             currently ignored by Heather.
//
//       /ef - temperature control device (fan) port.  Heather defaults to
//             using the receiver data port modem control DTR/RTS lines
//             for its temperature control function.  The /ef command lets
//             you specify a different port.  This can be useful if the
//             main receiver port does not have modem control signals (like
//             some USB interfaced receivers).
//
//       /ei - the secondary time/frequency counter port.  If the main
//             "receiver" is a counter (/rxi or /rx1), the /ei command lets
//             you use a second counter for processing up to two more channels
//             of time/frequency data (or four channels with a TAPR TICC).
//             Note that Heather has no way of configuring the secondary
//             counter configuration operating mode.  Also the user must
//             configure the main and secondary counters with the same
//             settings and operating mode.  To configure the second TICC
//             start Heather with the second device's port specified as the
//             main input device, configure it, shut down Heather, then
//             restart Heather with the first TICC specified as the main
//             device and the second TICC as the extra device.
//
//             If the main receiver device is a GPS receiver or GPSDO, the
//             data from the "secondary" /ei counter device is used to
//             calculate true adevs instead of the "bogo-adevs" that are
//             derived from the GPSDO self-reported loop control statistics.
//             When Heather is calculating "bogo-adevs" the adev type is
//             shown in lower case.  Upper case is used for "TRUE" counter
//             derived adevs.
//
//             See the description of the /im#, /if#, and /it# commands for
//             specifying what type of data the counter is sending, the
//             frequecy of the oscillator being tested, and forcing the
//             counter type.
//
//       /ek - the NMEA echo port.  This port echoes the current receiver
//             position, time, and satellite infomation in standard NMEA
//             format.  This lets you send the receiver data to another
//             program that understands NMEA but does not understand the
//             data the the receiver is sending.
//
//       /en - to read data from an external environmental monitor device
//             that provides temperatures (up to two), humidity, and air
//             pressure.  Currently supported sensors are the USB sensors
//             from dogratian.com  amd the LFS10x devices from
//             LookingForSolutions.com (like the LFS104BW).  A future
//             Heather-specific/optimized sensor is planned.
//
//       /er - the main receiver data port.  This is the same as the port
//             set by the /1, /1u, /id, /ip etc commands.  The /er command
//             just provides an alternate way of specifying the receiver
//             data connection.
//
//       /et - outputs the selected sun / moon / sat position info
//             out the TRACK_PORT.  Can also output the time in UTC.
//             This can be useful for implementing moon/sun/satellite trackers.
//             Once the TRACK_PORT is open and outputing data the SUN AZ:
//             SUN EL: / MOON AZ: / MOON EL: labels on the screen will be
//             shown in upper case if that info is selected for output.
//
//
//   Future device support:
//
//      /ed - to provide control and monitoring of an external device that
//            has DACs and/or ADCs for monitoring and controlling analog
//            devices.
//
//
//
//
//-USING A SIMULATION INPUT FILE:
//
//   You can force Heather to get its input data from a file.  These are
//   typically receiver data capture files captured by Heather using the WY
//   keyboard option.  Simulation files do not work with most of the "polled"
//   receiver types such as SCPI that require two-way polling to get data
//   from the receiver.  They mostly work with "UCCM" type receivers that
//   echo the commands sent to the receiver, but there will be a "time skip"
//   indication every second.
//
//   To use a simulation input file, use the /rs command line option:
//     /rs=filename  -or-
//     /rs=filename,seek_addr
//
//     ",seek_addr" is optional and if specified seeks to that byte in the
//     simulation file before reading.  This lets you skip over data you are
//     not interested in.
//
//   You should also specify "/0" on the command line to disable the com
//   port, but this is not usually necessary.
//
//   You can also read .raw simulation files with the keyboard "R" command.
//   This does not require the /0 command to disable the serial port.
//
//   If the simulation file was read in via the /rs= command line option,
//   the WU keyboard command will pause reading the file.  If the file was
//   read in via the "R" keyboard command, WU will close the simulation file
//   and resume normal processing of the receiver data stream.
//
//   You can use the /sw= command line option to control how fast the
//   simulation file is processed.  The value is the delay in milliseconds
//   to wait after each message is read.  Using the /sw command can make
//   keyboard response rather sluggish.  You can slightly speed up reading
//   of a simulation file by doing something that causes the plot area to
//   not be drawn like pressing the "G" key to bring up a keyboard menu.
//   As long as the menu is being shown, the simulation runs a little faster.
//   Also, if the mouse cursor is in the plot area of the screen reading the
//   simulation file can slow down quite a bit.
//
//
//   You can also simulate a time interval counter connected as an "extra"
//   input device using the /ri command.  If you do this you MUST also
//   specify the time interval counter type using the /it? command.
//     /ri=filename  -or-
//     /ri=filename,seek_addr
//
//
//
//-CONFIGURING THE RECEIVER TYPE:
//
//   Heather supports numerous receiver types.  If you do not specify a
//   receiver type Heather will attempt to automatically determine
//   the receiver type.
//
//   Auto-detection requires the receiver to be actively sending data that
//   can be analyzed.  Some receivers power up "mute" and do not automatically
//   send data.  You must "wake up" these receivers first by specifying the
//   receiver type.  Also some receivers ("polled receivers") only send data
//   in response to a query message.
//
//   If the receiver can work in both a NMEA and a native
//   binary format, it probably powers up in NMEA.  Use the
//   proper /rx# command shown below to put the receiver into native binary
//   mode.  Binary mode offers the user full control of the receiver
//   and better monitoring options.  The "!m" keyboard command can
//   switch most receivers back to NMEA mode.  On some receivers
//   it swicthes the device to its alternate language.
//
//   Some Motorola receivers power up in NMEA mode, sometimes at 4800 baud.
//   Receiver type auto-detect does not try 4800 baud.  If you are using a
//   Motorola receiver and it does not auto-detect, try starting Heather with
//   the /br=4800 command line option.   You can switch Motorola receivers
//   that are in NMEA mode to binary mode using the "!n" keyboard command.
//
//
//   You can force the receiver type with the "/rx" command line option.
//      /rx   - auto-detect receiver type (default)
//      /rxa  - Acron Zeit WWVB receiver (300:8:N:2)
//      /rxai - Trimble ACE-III (9600:8:O:1)
//      /rxag - Trimble Acutime GG (9600:8:O:1)
//      /rxat - Trimble Acutime Gold and earlier versions (9600:8:O:1)
//      /rxa3 - Trimble Acutime 360  (115200:8:O:1)
//      /rxb  - Brandywine GPS-4 receiver - 4800:8:N:1
//      /rxca - calculator only mode
//      /rxc  - UCCM - Trimble / Symmetricom / Samsung GPSDOs - 57600:8:N:1
//              (auto-detect sub-type)
//      /rxcp - UCCM-P - Trimble / Symmetricom GPSDOs - 57600:8:N:1
//      /rxcs - UCCM LPK/L8 - Samsung GPSDOs - 57600:8:N:1
//      /rxcu - UCCM - Trimble / Symmetricom GPSDOs - 57600:8:N:1
//      /rxd  - DATUM STARLOC II GPSDO - inferior wannabe Thunderbolt
//      /rxe  - NEC GPSDO ... STAR-4 compatible at (115200:8:N:1)
//      /rxen - Environmental sensors
//      /rxes - Furuno eSIP receivers
//      /rxf  - Lucent RFTG-m GPSDO
//      /rxfu - Furuno GT-8031 (using $PFEC commands)
//      /rxg  - GPSD interface (not really a Windows thing - /ip=localhost:2947)
//      /rxh  - HP 5071A cesium beam oscillator
//      /rxi  - TAPR TICC time interval counter (115200:8:N:1)
//      /rxj  - Jupiter-T  (aka Zodiac)
//      /rxk  - Lucent KS24361 REF0/Z3811A (19200:8:N:1)
//      /rxl  - Jackson Labs LTE Lite (38400,8,N,1)
//      /rxlp - Temex/Specratime LPFRS or RMO rubidium  (1200:8:N:1)
//      /rxm  - Motorola binary
//      /rxn  - NMEA
//      /rxo  - TruePosition GPS
//      /rxp  - Trimble TAIP receivers (4800:8:N:1)
//      /rxpa - Trimble Palisade (9600:8:O:1)
//      /rxpp - Tom Van Baak's PICPET timestamping counter chip
//      /rxpr - SRS PRS-10 rubidium oscillator
//      /rxr  - Trimble Resolution T family with odd parity
//      /rxs  - Sirf binary
//      /rxsa - Symmetricom SA22.c rubidium with 60 Mhz osc (forced model type)
//      /rxsb - Symmetricom SA22.c rubidium with 58982400.0 Hz osc (forced model type)
//      /rxsy - Symmetricom X72/X99/SA22 rubidium (60 Mhz osc) (auto detect model)
//      /rxss - Novatel SuperStar II (19200,8,N,1)  - currently untested
//      /rxsr - Spectratime SRO100/SRO70 rubidium
//      /rxt  - Trimble TSIP binary,
//      /rxtb - terminal emulator only (with buffered keyboard).
//      /rxte - terminal emulator only.
//      /rxtm - Spectrum TM4 GPSDO (setting features untested)
//      /rxts - TymServe 2000.
//      /rxu  - Ublox UBX binary,
//      /rxut - Ublox UBX binary (with TP5 timing messages),
//      /rxv  - Venus/Navspark mixed binary / NMEA (115200:8:N:1)
//              (auto-detects timing/RTK models)
//      /rxvb - Venus/Navspark RTK receiver in BASE mode (115200:8:N:1)
//      /rxvp - Motorol Oncore (VP) receiver
//      /rxvr - Venus/Navspark RTK receiver in ROVER mode (115200:8:N:1)
//      /rxvt - Venus/Navspark timing receivers (115200:8:N:1)
//      /rxx  - No receiver, uses system clock.
//      /rxx7 - Symmetricom X72 rubidium (forced model type)
//      /rxx9 - Symmetricom X99 rubidium (forced model type)
//      /rxy  - SCPI - Nortel telecom GPSDOs like NTWB and NTPX
//      /rxz  - SCPI (Z3801A style. 19200:7:O:1)
//      /rx5  - SCPI (HP5xxxx style)
//      /rx0  - Gravity/solid earth tide clock (uses system clock to display
//              solid earth tides and gravity offset,  requires manual entry
//              of latitude/longitude/altitude)
//      /rx1  - HP531xx (or other) counter (9600:8:N:1)
//      /rx12 - Ashtech Z12 L1/L2 GPS receiver
//      /rx17 - Trimble RT17 format GPS receivers (115200,8,N,1)
//      /rx3  - Zyfer Nanosync 380 (19200:8:N:1)
//      /rx35 - Symmetricom SA.35m rubidium(57600:8:N:2) - code not completed
//      /rx4  - Oscilloquartz STAR-4 GPSDO (9600:8:N:2)
//      /rx45 - Oscilloquartz OSA-453X GPSDO (9600:8:N:2)
//      /rx6  - Trimble SV6/SV8 (9600:8:O:1) (also ACE-III)
//      /rx8  - NVS BINR binary (115200:8:N:1)
//
//   /rx says to auto-detect the receiver type. This tries
//   to find the receiver type at 9600:8:N:1, then
//   115200:8:N:1, then 57600:8:N:1, then 19200:8:O:1, then 38400:8:N:1
//   unless the user specified the com port parameters
//   (with the /br=... command) before the /rx option. Note that for Z3801A
//   receivers that use 19200:7:O:1,  the auto-detect routine detects the
//   incorrect data size/parity combination and uses the proper settings).
//
//   /rx (auto detect) is the default mode if no /rx? command
//   is given on the command line.
//
//   Note that the auto-detect routine is a bit simplistic
//   and might occasionally mis-recognize the receiver type.
//
//
//   You can force or set the UTC leapsecond offset with the /rx? commands
//   like /rxx=18 (sets the leapsecond offset to 18 seconds).  This is
//   useful for devices that do not report the leapsecond offset or report
//   an invalid value.
//
//
//   If you explicity specify the receiver type, Lady Heather
//   defaults to the baud rate indicated above (or 9600:8:N:1 if
//   not shown above).  If your receiver is configured for
//   a different baud rate, specify the serial port settings
//   to use with the /br= command line option.
//
//
//
//   Special notes for Trimble Resolution-T (and other) TSIP devices:
//   Resolution-T devices default to ODD parity.  Heather can recognize the
//   TSIP data stream even if set for EVEN/NO parity, but the Res-T will not
//   recognize commands sent to the device.  If you let Heather auto-detect
//   the receiver type and did not specify a baud rate then, after around
//   30 seconds and the receiver ID message has not been seen (which the
//   receiver sends only if it accepts a query command) Heather will
//   automatically try different parity settings.  This feature may not
//   always work and is a bit of a hack.  Also, the screen and some receiver
//   options may not be properly configured for the new receiver type.  The
//   screen can usually be configured properly by issuing a screen size
//   command from the $ keyboard menu) once Heather has properly detected
//   the receiver serial port settings and has determined the actual receiver
//   type.
//
//   If you are using a Trimble Resolution-T type of receiver, you can
//   force the receiver model and/or ODD parity with the /rt command line
//   option.  The /rt command line option should be used with the /rxt and
//   /rxr command line options or if the device is not properly auto-detected.
//      /rt   - use ODD parity on the serial port and config for Res-T devices.
//      /rt=1 - Resolution-T
//      /rt=2 - Resolution-T SMT
//      /rt=3 - Resolution-T RES
//      /rt=4 - Resolution-T 360
//      /rt=5 - Resolution-T ICM
//
//
//
//   Special notes for Trimble RT17 format receivers (like the NetRS):
//   Receiver ports configured for RT17/Trimcom format outputs seem to only
//   stream data.  I have been unable to send configuration, etc commands
//   to the receiver.  Thus Heather is currently a monitoring only program
//   for these receivers.  You cannot change the receiver configuration or
//   request information that the receiver does not automatically stream.
//   You must make all configuration changes via the NetRS built-in http
//   web server.
//
//   Heather can auto-detect RT17 receivers as Zodiac receivers.  It is
//   best to force the receiver type with the /rx17 command line option.
//
//   It can take a few seconds to connect to the receiver over a TCP/IP
//   connection.  You can configure the device's IP address by connecting
//   a serial port (115200,8,N,1) to the RS-232 connector on the front panel.
//   Use a terminal program (or Heather with the /rxtt /br=115200 command
//   line option) and power up the receiver.  You should see lots of messages
//   fly by.  Near the end of the boot sequence it will ask if you want to
//   change the IP address.  Note that some of the serial ports on the rear
//   panel are straight-through connections and others are null-modem
//   connections.  Some NetRS manuals are incorrect in specifying which are
//   which.  It is suggested to use the front panel connector for boot-up
//   monitoring and use a rear panel connector to talk to Heather.
//
//   It is suggested to update NetRS receiver firmware to the latest available
//   (ver 1.3-2)  This firmware was a "security" upgrade and includes several
//   fixes to earlier versions.  It will work on all receivers regardless of
//   their "warrenty" date.   UNAVCO.COM has lots of info on using the NetRS
//   receiver.
//
//   Suggested port message confiuration is RT17 data with:
//      1 Hz epochs, position and measurement data
//      Concise format
//      Send RAW GPS data
//      Send Ephmeris data data (not used by Heather, may be useful to other
//      programs).
//      Send IODE and cycle slip counts.
//      Smooth code phase
//      Smooth carrier phase
//
//   If you connect an external frequency reference to the NetRS, you must
//   enable it.  Leaving it connected but not enabled can affect performance
//   or interfere with satellite tracking.
//
//   Trimcomm messsage format configured ports do not send location data,
//   so use RT17 format.  Trimcom data is only available over the TCP/IP
//   port (default 5018).
//
//   Heather supports writing L1/L2 RINEX files from the RT17 data stream.
//   Note that the RT17 format does not output a GPS week number.  Heather
//   creates a week number from the system clock.  This is used in conjunction
//   with the GPS time-of-week value in the receiver raw data packet to
//   generate the time display and RINEX files timestamps. If the system clock
//   is inaccurate, the date will be wrong close to the GPS end-of-week.
//   This can cause glitches in the RINEX output... so don't collect RINEX
//   files that span the GPS week rollover period.  For RINEX output don't
//   select L2C data.  RINEX v2.xx does not know about L2C and Heather's
//   RINEX v3.xx support is currently incomplete (the observation list does
//   not output the correct v3.xx observation codes... you could manually
//   edit the file, though).
//
//
//
//   The Acron Zeit WWVB receiver cannot be auto-detected.  Always use
//   the /rxa command.  Support for this receiver is a hack.  Heather shows
//   it as tracking sat PRN 1. You should manually enter your lat/lon/altitude
//   (/po=lat,lon,alt command line option or the SL keyboard command or the
//   "heather.loc" file) and the utc offset (like /rxa=17 or /uo=17 command
//   line options). If you don't enter your position, sat PRN 1 is shown
//   at az=1, el=89. Technically the Acron Zeit speaks at 300:7:O:2, but it
//   also works with 300:8:N:2 if you strip off the received parity bit
//   (which Heather does). Also, it takes several seconds for Heather to
//   startup and shutdown with this device under Windows.
//
//
//
//   Special note for Acutime 2000 and earlier receivers... also the Palsiade:
//   You MUST specify the /rxat (or /rxpa) and /rxag  receiver type command
//   line option. If you auto-detect the device or specify an incorrect
//   TSIP receiver device, the Acutime can be put into a mdde where it
//   sends TSIP messages without proper end-of-message indicators.  If this
//   happens, the screen data areas will go blank and you must power-cycle
//   the receiver to recover!  Also, note that changing most of the settings
//   on these receivers cause them to do a restart and the seetings changes
//   do not show up for several seconds.
//
//
//
//   Special note for the Ashtech Z12 receiver:  If the Z12 is configured to
//   send the NMEA GPZDA time code message, the time code messages will
//   corrupt the raw satellite data observation message... so Heather uses the
//   system clock to display the time.  This can make it appear that Heather
//   is actually receiving data from the Z12.  To verify that Heather is
//   actually talking to the Z12, make sure that the receiver ID, firmware,
//   and option info is being displayed.  If raw observation messages are
//   enabled you should also see satellite positions, signal strength, and
//   doppler data being displayed.
//
//   The Z12 has no way to read back the satellite elevation mask filter.
//   If you are unsure of the current setting, set the filter (FE command).
//
//
//
//   The Brandywine GPS-4 cannot be auto-detected.  Also the baud rate
//   must be set to 4800:8:N:1  since the Brandywine receiver cannot accept
//   more than 1 char per millisecond.  The Brandywine receiver will not
//   work with most serial cables.  Pin 4 is a receiver reset signal and if
//   connected will prevent the receiver from working.  Pins 6, 8, and 9
//   also have non-standard uses.  YOU SHOULD USE A CABLE WITH ONLY
//   PINS 2, 3, and 5 CONNECTED TO THE PC!  The Brandywine receiver does
//   not work well with the audible "tick clock" mode.
//
//
//
//   The Datum STARLOC II cannot be auto-detected. Firmware
//   bugs cause it to stop outputting time messages. Always
//   specify "/rxd" when running STARLOC II receivers.  Also
//   note that the Datum STARLOC II has NUMEROUS firmware
//   bugs and generates lots of duplicate/missing time stamp errors
//   and does not output satellite azimuth/elevation values.  The
//   firmware bugs cause many messages to be corrupted (the firmware does not
//   properly "escape" any 0x10 bytes in most of the output messages, etc),
//   thus any messages that contain a 0x10 byte are likely to be corruped!
//   There might be firmware out there that fixes these issues, but I have
//   not seen any.  The STARLOC is the most miserabe excuse for a GPSDO that
//   i have ever seen... the firmware seems to have been written/tested by a
//   drunk chimanzee... I have fw App:1.10 16 Mar 2000, GPS:1.2 07 Jan 2000.
//
//
//
//   Special note for Furuno (like GT-8031 and eSIP) receivers:  Auto-detect
//   may detect these as NMEA receivers, particularly if Heather has not
//   previously enabled them.  YOU SHOULD NOT RELY ON AUTO-DETECTING THESE
//   RECEIVERS!  Also, these receivers may not output any data until
//   they begin tracking satellites and have an almanac.  They may be
//   unusable/undetectable for a long time (an hour?) after power-up.
//
//   The Furuno PFEC model and version info is only available from the
//   self-test results message.  If the diagnostics have not been run they
//   will show up as "0". Running the diagnostics will affect the receiver
//   operation for a few seconds.  The diagnostics are run whenever
//   Heather starts up, so for the first few seconds you may see
//   "No stats usable" and "Time invalid" messages.
//
//   For ESIP receivers, the SU keyboard command can be used to set the
//   default UTC (leapsecond) offset to use when the receiver does not have
//   the current value from the satellies.  This value is saved in EEPROM.
//
//   Furuno ESIP receivers (like the GT87xx series) have some satellite
//   data messages (that send things like satellite doppler, pseudoranges,
//   and the raw navigation message that are only avialable if the baud rate
//   is 115200 baud or greater.
//   You can set the ESIP baud rate by using the "!u" keyboard command to
//   send the following string (in uppercase!):
//      PERDCFG,UART1,115200
//   Note that the baud rate, cable delay, signal level mask, elevation
//   mask, etc settings are NOT stored in flash memory and will be lost
//   after a power cycle (unless a backup battery is in use).  Also after
//   changing the baud rate, you will need to specify the baud rate from
//   the command line (because Heather assumes ESIP receivers run
//   at 38400 baud):
//       /rxes /br=115200
//
//
//
//   GPSD is a Linux/macOS/FreeBSD service that provides a standardized
//   interface to numerous models of GPS receivers. It is accessed
//   via TCP/IP address "localhost:2947"  GPSD cannot be auto-detected.
//   Use "/rxg" to force GPSD mode.  You can specify a particular
//   GPSD device to use with the /ig=gpsd_device_name command.
//   The /ig command will also do the /rxg command.
//   If your GPSD system is not on "localhost:2947", specify the TCP/IP
//   address with the /ip= option after the /rxg or /ig= option.
//
//
//
//   The Jackson Labs LTE GPSDO device has a Venus timing receiver in it that
//   runs at 38400:8:N:1 and normally generates the standard Venus messages.
//   It also has a switch that enables a proprietary Jackson Labs STATUS
//   message.  On earlier versions of the LTE firmware enabling the STATUS
//   messages disables all the Venus messages and you lose the ability to
//   show satellite info, automatically get the UTC.  etc.  Later versions
//   of the LTE firmware allow the STATUS message to be mixed in with the
//   regular Venus messages.leapseoncd offset, etc.  In STATUS only mode
//   Heather gets the time from the receiver clock.  Note that the LTE does
//   not let you control the Venus receiver and you cannot change any
//   settings or request data from the receiver.  Because of this, Heather
//   cannot get the UTC leapsecond offset.
//
//
//
//   Special note for Lucent KS24361 units: the Z3811A
//   (REF1) unit talk SCPI at /br=19200:8:N:1 and the
//   Z3812A (REF0) unit talks at /br=9600:8:N:1 over the
//   DIAGNOSTIC ports.
//
//
//
//   Special note for Lucent RFTG-m units: the RFTG-m units output
//   control and staus messages in TSIP format and a time code in ASCII.
//   Heather attempts to disable the TSIP messages when it exits so that
//   the device can be auto-detected when Heather starts.  If a program
//   (such as the Lucent RFTG control program) left the device outputting
//   TSIP, it will be mis-identified.  If this happens, you should use
//   the /rxf command line option to force the receiver type to RFTG-m.
//
//   You can use the !D and !R commands to force disable or re-enable a unit.
//   If you disable a unit,  the other unit the other unit will be in
//   control.  If both units are enabled, then the RFTG firmware decides
//   which unit is in control (usually the Rb unit).  If both units are
//   disabled, well... not much happens.  Heather will not see any data
//   on the serial port and after a few seconds will try to re-establish
//   communications... which won't suceed.  Try !R to re-enable the unit.
//   Or you may have to power-cycle the device.
//
//   Also, The RFTG units do not output satellite positions so the normal
//   signal level maps are not available. On the RFTG-XO unit Heather will
//   show  the satellites being tracked and the signal levels, but the
//   satellite positions are fixed at AZ=(prn*10) and EL=20 degrees.
//
//   If you enter "& ESC" the rather useless satellite info table will be
//   replaced by a much more intersting paramter info table.  If you press
//   ESC or SPACE, the display will revert to the sat info table.  Clicking
//   on the sat info table or issuing the ZD keyboard command will zoom the
//   screen to show the device paramter info screen.
//
//   Also, you will need to run two instances of Heather to monitor both
//   sides (RB and XO) of the unit at the same time.
//
//
//
//   Some Motorola receivers power up in NMEA mode, sometimes at 4800 baud.
//   Receiver type auto-detect does not try 4800 baud.  If you are using a
//   Motorola receiver and it does not auto-detect, try starting Heather with
//   the /br=4800 command line option.   You can switch Motorola receivers
//   that are in NMEA mode to binary mode using the "!n" keyboard command.
//
//
//
//   Some NMEA receivers use 4800 baud baud.  Receiver type auto-detect does
//   not try 4800 baud.  If you are using a NMEA receiver and it does not
//   auto-detect, try starting Heather with the /br=4800 command line option.   You can switch Motorola receivers
//
//
//
//   The NV-08C receiver powers up "mute".  It cannot be auto-detected until
//   it has been initialized (start Heather with the /rx8 command line
//   oprion).  Also the auto-detect routine might possible identify
//   the NVS-08 as a Trimble TSIP receiver... their comminications protocols
//   are virtually identical.  The NV-08C has two serial ports.  One outputs
//   NMEA data, the other uses NVS BINR binary format.  You should use the
//   binary format port.  Some NVS receiver modules have a switch for seleting
//   the NMEA or BINR port.
//
//
//
//   Special note for OSA-453x rubidium oscillators:  these cannot be auto
//   detected.  Also, some versions of the firmware time out after 175 msecs
//   between keystrokes... you can't manually send commands to the device
//   with a standard terminal program!  The !t terminal emulator command
//   attempts to buffer all keystrokes and sends the buffer when CR is
//   pressed. The /rxtt terminal-only mode does not currently support this
//   feature... use /rxtb for a buffered terminal-only mode keyboard.
//
//   Due to the slow baud rate and the length of the response to the "V"
//   version info command, the version info is only requested when Heather
//   starts up (otherwise you would see lots of time stamp errors).  Speaking
//   of which, the 453x devices have some firmware issues... the message
//   that outputs position/time regularly gets corrupted.  Heather can usually
//   patch things up.  Also the 453x's also regularly stops responding to
//   queries, so you will probably see DATA LOSS RESETS.
//
//
//
//   Note that many PRS-10's have the serial data and 1PPS input pins
//   configured to output analog data.  To configure the pins for serial
//   data and 1PPS inputs you must change some resistors inside the unit
//   (Hey SRS! ever hear of jumpers and solder blobs for doing config
//   changes?  Bsstards!)
//   You might want to disable the digital clock display... you will then
//   be able to see all the ADxx values on the main screen.
//   The ZD command will zoom the screen to show all values read from the
//   device.
//
//
//
//   Special note for SRS PRS-10 rubidium oscillators: the PRS-10 cannot be
//   auto detected.  It takes around 25 seconds to request and receive all
//   of the information it can provide.  The plot queue is automatically
//   reset the first time all of the available infomation is received.  This
//   is done because the plot queue will contain bogus zero values for any
//   data that had not yet been received.
//
//
//
//   Special note for the Oscilloquartz Star-4: When the Star-4 is first
//   powered up, it does not send time/position info until it receives a
//   warm-restart command.  This causes the receiver to go do a self-survey
//   and start sending time/position information.  Heather detects that
//   the device has not been initialized by detecting the loss of time
//   messages for over 16 seconds.  The first time you start Heather after
//   poweing up the Star-4, it can take around 20 seconds before normal
//   operation is established and the screen starts updating.
//
//
//   Special note for Samsung UCCM receivers like the -LPK and -L8:
//   These receiver will report "Power Bad" unless the input voltage is
//   within a rather narrow range around 5.5V.  I power mine from a 6.0V
//   supply fed through a (5A) Schottky power diode.  The device does seem
//   to work properly from 5V .. 6V even though it reports "Power Bad".
//   Some people have reported locking issues at 5.0V.
//   These devices will do a self survey every time they power up... this
//   takes around 40 minutes to complete.  Settings such as cable delay
//   and antenna elevation mask are saved in EEPROM.
//
//
//   Special note for the Spectratime SRO100/SRO70 rubidium oscillators:
//   these cannot be auto detected.  The plot queue is automatically reset
//   the first time all of the available information is received.  This
//   is done because the plot queue will contain bogus zero values for any
//   data that had not yet been received.
//   You might want to disable the digital clock display... you will then
//   be able to see more of the SRO100 status values on the main screen.
//   The ZD command will zoom the screen to show all values read from the
//   device.
//
//
//   Special note for the Spectrum TM4.  Support for this device has not been
//   fully tested... I don't have one.   If you set the cable delay value
//   with the PC keyboard command it will set the MUX1 output for PPS output
//   (enabled or disabled).  If you had the output configured for a frequency
//   or time code (P1 command) you will need to redo that setting.  Also note
//   that the TM4 does not send satellite positions... the satellite position
//   and signal level maps are not available...  boo! hiss!
//
//
//
//   The TAPR TICC must have the latest firmware to work with Lady Heather...
//   the firmware shipped with the first batch of TICCs (and maybe the
//   second production run?) will not work. Auto-detecting the TAPR TICC
//   can be unreliable, particularly on non-Windows machines.  Other counter
//   types (except the PICPET) cannot be auto-detected... you must use
//   the /rxi command line option.
//
//   For TAPR TICC devices.  Heather defaults to the "all adev" display
//   display for channel A data.  The GP plot is the channel A time interval
//   deviation from 1 second (in nanosecods).  GO is channel B.  G7 is channel
//   C. G8 is channel D.  The current TAPR TICC is a two channel device and
//   does not ouput chC and chD data but Heather allows two TICC devices to be
//   used.  The first one is the main "receiver" device (the /rxi device) and
//   the second one is the "external" device (specified the the /ei= command
//   line option).  In this mode the data from the secondary TICC chA and chB
//   are processed as chC and chD.
//
//   Although the current TAPR TICC can output channel C timestamps when set
//   to "TIMELAB" mode, these are meaningless for Heather's ADEV analysis and
//   support is mainly for curiosity.
//
//
//   If the Trimble SV6/SV8/ACE-III is auto-detected,  it will first be
//   recognized as a TSIP device but the parity will be wrong and it won't
//   accept commands. Also, the special commands the SV6 expects won't be
//   used.  After 30 seconds or so, Heather will automatically try odd
//   parity (you should hear a beep) and the receiver should mostly work.
//   For the most reliable operation, use /rx6 or /rxai  with these devices.
//
//
//   The Trimble SV6 and related devices do not output data until they
//   are tracking satellites.  They must have an antenna connected and may
//   need to be powered up for 20+ minutes before they can be used.  Also
//   they do not accept a lot of the standard TSIP receiver control commands
//   (even though they may be shown in the various keyboard menus).
//
//
//   Trimble TAIP receivers cannot be auto-detected if the baud rate is
//   the standard 4800 baud.  TAIP receivers do not output satellite
//   positions or signal levels.  The sats show up in the sat maps at
//   az=10*prn, el=30 degrees,  sig level=40.0
//
//
//
//   Special note for Trimble Resolution-T devices:
//   These are commonly available configured for ODD parity. Heather attempts
//   to detect this when it thinks it sees a Resolution-T by issuing commands
//   and if it does not see expected responses, it toggles the parity mode and
//   tries again.  This repeats every 30 seconds or so.
//   The "/rxr" command sets parity to ODD.
//   Also Resolution-T devices can have firmware that makes identifying the
//   exact model number impossible. If you have problems, you can force the
//   model type:
//      /rxr=1  - original Resolution-T
//      /rxr=2  - Resolution-T-SMT
//      /rxr=3  - Resolution-T-RES
//      /rxr=4  - Resolution-T-360
//      /rxr=5  - ICM
//
//   The Trimble Resolution-T TEP model is configured to speak Motorola binary
//   (rather poorly).  The !M keyboard command will switch it to TSIP mode.
//   A hard reset (!H) will do a factory reset back to Motorola mode.
//
//
//
//   Special note for Trimble telecom GPSDOs like the NTWB and NTPX: these
//   devices can speak either TSIP binary (/rxt) or SCPI (/rxy).  TSIP
//   provies much more information and a much more robust interface.
//   Auto-detect will find these units as TSIP devices.
//
//
//
//   The TruePosition GPSDO has factory and warm reset commands, but these
//   dont seem to restore things like cable delay and attenuator settings
//   to their default values.
//
//
//
//   Special note for Ublox receivers:  Later models of the Ublox receivers
//   use the CFG_TP5 message to config the PPS and TIMEPULSE outputs.  Heather
//   can automatically detect the receivers and use the proper message. But
//   if you use the '@P...' command in a heather.cfg file to config the pulse
//   outputs on startup Heather will not yet know the exact receiver type.
//   You can use the /rxut command line option to force Heather to use the
//   CFG_TP5 commands.  Also Ublox receivers can send mixed NMEA and binary
//   messages.  This can confuse the auto-detect routine into reporting the
//   receiver as a NMEA receiver (particulalry if the receiver is configure
//   for multiple GNSS systems) .  Use the /rxu command line option to force
//   Ublox mode and prevent this.
//
//
//
//   Special note for Venus/Navspark RTK receivers.  These receivers need to
//   be put into base or rover mode before they will send RTK/RAW data. A
//   RTK receiver that is in a RTK mode will show the device as "Venus-R" (the
//   Venus timing receivers show as "Venus-T").  Heather tries to enable
//   base mode when it initializes the receiver.  If this does not work
//   (sometimes the receiver seems to ignore the command) you can try
//   the !k keyboard command which will re-send the base mode command.  If
//   the receiver is in ROVER mode when Heather is started, it cannot be
//   auto-detected.  If you are using a Venus RTK receiver it is best
//   to start Heather using one of the following commands.  Heather will
//   force ROVER mode if you dont use one of these commands.
//      /rxvb - for BASE mode
//      /rxvr - for ROVER mode
//
//   The !k command lets you enable binary RTCM output mode.  If this is
//   selected then Heather will stop seeing valid messages, timeout, and
//   re-start the receiver.  The re-start will disable RTCM output mode.
//   If you wish to keep RTCM output enabled you MUST exit the program before
//   the timeout (5 seconds).  Whenever Heather starts up, RTCM output is
//   disabled.  Heather cannot auto-detect a receiver that is in RTCM
//   output mode.
//
//
//
//
//   Special note for X72/SA22 rubidium oscillators: these cannot be
//   auto detected.  The plot queue is automatically reset the
//   first time all of the available infomation is received.  This
//   is done because the plot queue will contain bogus zero values for any
//   data that had not yet been received.
//   You might want to disable the digital clock display... you will then
//   be able to see more of the X72 status values on the main screen.
//   The ZD command will zoom the screen to show all values read from the
//   device.
//
//   Most X72/SA22.c devices use a 60 MHz oscillator and generate a 10 MHz
//   output.  Some telecom models (ussualy SA22.c devices... use a 58.982400
//   oscillator and generate a 9.8304 MHz output.  The /rxsb command line
//   option configures Heather for these units.  If you happen to have a
//   X72 with these "telecom" frequencies try /rxsb /rxsy.  Note the "i"
//   status command reports clock frequencies that have no relation to
//   reality!
//
//   The X72/SA22 firmware does not have a way to read back a lot of the
//   current configuration settings.  Heather implements a "software eeprom"
//   that stores the current settings in the file "tbeeprom.dat".  If you
//   use more than one X72 and/or SA22 device you should start up Heather
//   with the "/wp=xxx" (write prefix) command line option that specifies
//   which software eeprom file to use with that device.  For instance
//   "/wp=sym" will use the file "symeeprom.dat", /wp=xyz will use the file
//   "xyzeeprom.dat", etc.
//
//
//
//   Special note for Zyfer Nanosync units: these power-up "mute" and do not
//   send any data until commanded.  The unit cannot be auto-detected. You
//   must use the /rx3 command line option to use them.  Once they have been
//   initialized with the /rx3 command they should be able to be auto-detected.
//
//
//
//   If no recognizable device is auto-detected,
//   Heather assumes that no receiver is connected and
//   defaults to a time-only mode using the system clock.
//   This will also happen if you specify the wrong com device or
//   it is in use by another program.
//
//
//   /rxx system clock mode does not need a GPS receiver, it uses
//   the system clock to drive Lady Heather as a time display.
//   Use /rxx=17, etc to set the UTC/GPS leap second offset
//   (needed only for some of the astronomical time formats)
//
//
//   The /rx0 "gravity clock" that calculates solid earth tides and vertical
//   gravity offstes also uses the system clock to drive the program.  To
//   use the gravity clock display, you must set your location using the
//   /po command line or SL keyboard commands.
//
//   Also Heather likes to know your location so that the
//   sun and moon info and the astronomical time formats will work.
//   To set your lat/lon/alt use:
//      SL              - from the keyboard
//      /po=lat,lon,alt - from the command line
//
//   Heather can also get the default location from the file "heather.loc"
//   This file is read until a line is found that has a valid lat/lon/alt
//   position. The file should be saved in the heather installation
//   directory.  It is normally only used if the input device does not report
//   a location (like for rubidium oscillators, etc). It is also used if you
//   use the string "loc" when Heather requests a lat/lon/alt setiing.
//
//   The lat/lon values can either be decimal values or degrees/minutes/seconds
//   format like: 30d40m50.2s   Negative values are west longitude, southern
//   latitude.
//
//   The altitude value is normally in meters, but can be in feet if the value
//   is followed by an "f" or "'" like  500f or 500'
//
//   In "/rxx" receiver-less mode most of the keyboard commands are
//   meaningless (mostly the the "T" and "G" menus have relevant options)
//
//
//   You can force the Heather display window title to an indication of the
//   receiver type with the /pt command line option.  /pt=title_string will
//   set the title to whatever you want ('_' chars are replaced with blanks).
//   /pt with no title string uses the the receiver type as the title.
//
//
//
//
//-CONFIGURING THE TIME ZONE:
//
//   You should tell Heather what time zone you are in so that the
//   proper local time can be displayed. You can set the time zone
//   with the command line option:
//       "/tz=#sssss/ddddd" where # is the standard
//   time zone offset in hours from GMT and "sssss" is up to 5
//   characters of standard time zone ID and "ddddd"
//   is the daylight savings time zone id.
//
//   Fractional time zone offsets can be specified
//   like "/tz=9:30ACST".  NOTE THAT WESTERN HEMISPHERE
//   TIME ZONES ARE NEGATIVE NUMBERS! (i.e. /tz=-6CDT/CST)
//
//   The time zone ID strings can be up to 5 characters
//   long (default=LOCAL).  If no "ddddd" daylight savings
//   time zone ID is given, the program does not do
//   any daylight savings time conversions.
//
//   If a standard time zone in use,  the time shows in
//   BLUE.  If a daylight savings time is in use, it shows
//   in CYAN.  If no local time zone is in use, it shows
//   in WHITE.
//
//   The time zone string can also be specified in the standard
//   Linux time zone format like CST6CDT (note: western
//   hemisphere time zone offset values are POSITIVE values in
//   this format)
//
//   Time zones can also be set from the keyboard with
//   the TZ command.
//
//
//   If the user does not specify a time zone on the command line or in the
//   "heather.cfg" file, Heather attempts to get the time zone from the "TZ="
//   environment variable. Heather only checks the "TZ=" environment variable
//   when starting up.  If you change it while Heather is running, the change
//   will not be seen.
//
//
//   Many of Lady Heather's time calculations depend upon having the correct
//   "UTC offset".  This is a count of the number of leap seconds that that
//   have accumulated since the GPS system was first implemented.  It is the
//   number of seconds of difference between GPS time and UTC time.
//   Lady Heather normally automatically gets this value from the GPS receiver.
//   It is part of the GPS almanac data which can take over 15 minutes to
//   arrive from the GPS satellites.  Until the UTC offset is received,
//   Heather will display a "NO UTC OFFSET" warning.
//
//   Some receiver data formats (like NMEA) do not provide the UTC offset
//   value. You can specify the value to use with the "/uo=" command line
//   option. You  can also force the UTC offset value with the "/rx#=offset"
//   command line option that sets the receiver type.  For instance with
//   a NMEA receiver use "/rxn=18"  User specified UTC offset values are
//   shown in YELLOW.
//
//   If no UTC offset value has been set, Heather attempts to guess an
//   approximate value.  Guessed values will be shown in RED.
//
//   Heather uses the availability of the UTC offset value from the GPS
//   receiver as an indicator that the date/time values reported are valid.
//   Until the UTC offset has been received, the date/time and other info
//   is assumed to be incorrect.  When the UTC offset value has been received,
//   the plot data queue is flushed and normal operation is started.
//
//
//   There are several special time zone names. These special time zones
//   only affect the displayed time unless you specify OT from the keyboard.
//   Note that there can be multiple names for the same special time zone.
//
//       GPS      - GPS time
//       GPST     - GPS time
//       UTC      - UTC time
//       UT1      - UT1 time
//       UT       - UT1 time
//
//       LOR      - Loran time (9 seconds ahead of GPS)
//       LORAN    - Loran time (9 seconds ahead of GPS)
//
//       SST/SDT  - Solar time
//       SOL      - Solar time
//       SOLAR    - Solar time
//
//       GMST     - Greenwich Mean Sidereal Time
//       GAST     - Greenwich Apparent Sidereal Time
//       LMST     - Local Mean Sidereal Time
//       LAST     - Local Apparent Sidereal Time
//
//       TAI      - TAI time (19 seconds ahead of UTC)
//       TT       - Terrestrial time
//       TDT      - same as Terrestrial time
//       TCG      - Geocentric Terrestrial Time
//       TCB      - Barycentric Coordinate Time
//       TDB      - Barycentric Dynamical Time
//       BES      - Besselian time
//       HJD      - Heliocentric Julian Date
//
//       MAR      - Mars date and time (official)
//       MARS     - Mars date and time (official)
//       MARSD    - Mars date and time (official)
//       MSD      - Mars date and time (official)
//       MTC      - Mars date and time (official)
//       AMT      - Mars date and time (official)
//
//       VEN      - Venus date and time (referenced to J1900 epoch, GPS time scale)
//       MER      - Mercury date and time (referenced to J1900 epoch, GPS time scale)
//       PLU      - Pluto date and time (referenced to J1900 epoch, GPS time scale)
//
//   Note that when setting one of these astronomical time
//   scales you should still specify your local time zone
//   offset (e.g.  /tz=-6LAST/LAST) so that the various
//   features (like the audible clocks and alarms) that work
//   with local time will know the correct local time.
//   You can use the (rather obscure) OT keyboard command
//   to cause Heather to use the displayed time for these
//   features.  See the description of the audible clocks for more details.
//
//   You can also start up in Solar Time mode with the "/bs" command line
//   option.
//
//
//   The astronomical time scales UT1/TT/TCG/TDB/TCB/MARS,etc
//   depend upon the current utc "delta-T" value.  This
//   is normally derived +/- 0.5 seconds from the receiver
//   UTC time using the current leap second offset:
//      delta-T = UTC offset + 51.184 seconds.
//
//   Delta-T actually changes constantly in an unpredictable manner
//   and if a more precise value is desired, you can specify it
//   with the "/uc=" command line option or TE keyboard command.
//   Heather can also read the delta-T value from the file
//   "deltat.dat". It tries to do this on startup and at
//   00:00:16 UTC every day.
//
//   If you follow the delta-T value in the TE keyboard command with an "*"
//   then the deltat.dat file will be updated with the new value.
//
//   You can set up a operating system chron job or script to fetch
//   the current delta-T value from the net and write it to
//   the file "deltat.dat".  On startup or at 00:00:16 UTC every day
//   Heather will try to read the current delta-T value from the file.
//
//   The special time zone name "UT1" can only be set
//   using the "-6UT1/UT1" format... the Linux format cannot
//   be used (since the "1" int "UT1" looks like a time zone offset).
//   Use the equivalent time zone name "UT" instead.
//
//   Also supported are various planetary times: Mars ("MAR", "MSD", etc)
//   Venus ("VEN"), Mercury ("MER"), and Pluto ("PLU").  Mars time is based
//   upon the NASA definition of Mars time.  Mercury, Venus, and Pluto time
//   are based upon the number of revolutions of the planet around the sun
//   since 1 Jan 1900 (J1900).
//
//   Sorry, no time for Uranus... or Jupiter or Saturn...
//   these are big blobs of smelly gasses with no
//   fixed rotation time.
//
//   Besides specifying your time zone you should also specify your
//   Daylight Savings Time (aka Summer Time) calculation method.
//   The default daylight savings time switching dates are
//   the US standard.  The "/b=" command line option lets you specify
//   your daylight savings time calculation.
//
//   Use "/b=1" for USA, "/b=2" for UK/Europe, "/b=3" for
//   Australia or "/b=4" for New Zealand.  "/b=0" turns off
//   daylight savings time conversions.  If you have not specified a DST zone
//   and set the DST time zone name to "BST" then zone 2 is automatically
//   set (otheriwse zone 1 is set).
//
//   If the rules change or you live in a backwater,  you can
//   specify a custom daylight saving time rule:
//       /b=nth1,start_dow,start_month,nth2,end_dow,end_month,hour
//
//       nth1 = start DST on nth occurance of the day-of-week
//            if nth1 > 0,  count day-of-week from start of month
//            if nth1 < 0,  count from end of month
//
//       start_dow - start DST day-of-week (0=Sunday, 1=Monday, ... 6=Saturday)
//
//       start_month - 1..12 = January .. December
//
//       nth2 = end DST on nth occurance of the day-of-week
//            if nth2 > 0,  count day-of-week from start of month
//            if nth2 < 0,  count from end of month
//
//       end_dow - end DST day-of-week (0=Sunday, 1=Monday, ... 6=Saturday)
//
//       end_month - 1..12 = January..December
//
//       hour - local time hour of switchover (default = 2)
//
//     Example:  /b=-1,0,9,2,3,4,6 says to start daylight
//               savings time on the last Sunday in September
//               and return to standard time on the second
//               Wednesday in April at 6:00 local time.
//
//
//
//
//-CONFIGURING THE UTC OFFSET (LEAPSECOND COUNT):
//
//   The difference between GPS and UTC time is known as the UTC offet.  It
//   is the number of leapseconds that has accumulated since the GPS system
//   was started.  Most GPS receivers get this from the satellites (after
//   the almanac and ephemeris data has been received).  For devices that do
//   not send a UTC offset, you can set the value with the /uo= command
//   line option:
//       /uo=seconds
//   or you can set it with the /rx## receiver type command line option
//   like:
//       /rxt=18
//
//   Furuno ESIP receivers let you store a default UTC offset value in
//   EEPROM.  This value is used until the receiver can provide the current
//   value.  The SU keyboard command lets you set the default value.
//
//   Until Heather has a UTC offset value, it is assumed that the time
//   is not valid.  If no leapseoncd value is available, Heather estimates
//   a leapsecond value to use based upon the date.
//
//   Heather shows UTC offset values obtained from the satellites in WHITE.
//   User specified UTC offsets are shown in YELLOW.  Esitmated UTC offsets
//   are shown in RED.
//
//
//
//
//-CONFIGURING THE VIDEO SCREEN SIZE:
//
//   Lady Heather works best with a screen size of at least 1024x768 pixels.
//   This is the default screen mode.  You can specify other fixed screen
//   sizes or a custom screen size with the "/v#" command line option or
//   via the '$' keyboard menu.  The keyboard menu also allows you to tweak
//   the size of the text font (Tiny=8x8  Small=8x12  Medium=8x14  Large=8x16).
//
//   Heather also now has support for an EXPERIMENTAL scaled vector font that
//   allows larger, more readable text on big screens.
//
//
//   WARNING: under Windows the F11 key is used to maximize the display
//            true full screen mode (no title bar or edge decoration).
//            This will ONLY work if your MONITOR size is supported by
//            old-style DirectDraw (i.e. 640x480, 800x600,1024x768,1280x1024).
//
//            If your system does not support DirectDraw and/or your monitor
//            size is not supported, HEATHER WILL CRASH.  If this happens,
//            Use CTRL-ALT_DEL to bring up the Windows Task Manager and
//            kill the heather.exe process.  If your system does support it,
//            you can start Heather in true full-screen mode with the /fu
//            command line option. The MAXIMIZE button on the Windows title
//            bar is only enabled in "/fu" mode.
//
//            Pressing F11 in maximized true full screen mode will restore
//            the screen size.
//
//
//   WARNING: The macOS version of Lady Heather using XQuartz for the display
//            manager does not let you restore the window size to its previous
//            size if you maximize the window size.  You must manually resize
//            the window by dragging the lower right corner of the window.
//
//   Linux can use the F11 key to toggle to/from full screen mode.  Under
//   macOS, F11 will switch to full screen mode (but may not be able to
//   restore the previous screen size).
//
//   Screen resolutions marked ** are designed for small-ish touchscreen LCD
//   panels.
//
//   In Linux/macOS/FreeBSD specifying these screen resolutions automatically
//   enables true fullscreen mode.  If your monitor screen size does
//   not match the selected resolution, the screen might go bonkers or
//   go to full screen mode for your monitor size. You can disable the
//   automatic setting of full screen mode for these video modes by
//   using "/fu /fu" commands before the /v? command.
//
//   These modes also default to mapping all mouse buttons to the left button
//   (to get around potential bugs in the touchscreen driver). To disable this
//   use "/mb /mb" on the command line.
//
//      /vt           - Text only Video screen
//      /vu           - Undersized (640x480) Video screen
//      /vp **        - Raspberry PI (800x480) touchscreen screen
//      /vs           - Small (800x600) Video screen
//      /vn           - Netbook (1000x540) Video screen
//      /vr **        - Reduced (1024x600) Video screen
//      /vm           - Medium (1024x768) Video screen (default)
//      /vj **        - Large (1280x800) Video screen
//      /vk           - Large (1280x960) Video screen
//      /vl           - Large (1280x1024) Video screen
//      /vv           - Very large (1440x900) Video screen
//      /vx           - eXtra large (1680x1050) Video screen
//      /vh           - Huge (1920x1080) Video screen
//      /vz           - 2048x1536 Video screen
//      /vc=colsXrows - custom screen size (e.g. /vc=1200x800)
//                      Heather supports screen widths up to 4096 pixels.
//
//      /vd **        - experimental 480x320 ultra-small screen mode
//                      (for things like Raspberry PI touchscreens).
//                      This mode uses vector fonts and the srceen formatting
//                      can be rather poor!
//
//                      Note that for several small Raspberry PI SPI interfaced
//                      touchscreens the touchscreen driver does not work
//                      well (or at all).  It ignores touches!.  You can
//                      improve this by using the /mb command line option.
//                      This maps the scroll wheel and RIGHT buttons to be
//                      the same as the LEFT button.  The touch response will
//                      be rather slow... you have to hold the touch for 2-3
//                      seconds before it will be recognized.
//
//      /ve **        - experimental 320x480 ultra-small screen mode
//                      (for things like Raspberry PI touchscreens).
//                      This mode uses vector fonts and the srceen formatting
//                      can be rather poor!
//
//      /vf           - startup in (nearly) full screen window mode - this
//                      should not be confused with true full screen mode.
//      /vq=scale     - uses a scaled vector font.  "scale" is the desired
//                      scale factor (50 to 500 percent).  This allows for
//                      better readability on large screens.
//                      Suggested values are 150 and 200 percent.
//                      VECTOR FONTS ARE CURRENTLY EXPERIMENTAL AND MAY CAUSE
//                      SOME SCREEN FORMATTING ISSUES!  They also take
//                      sligtly mode CPU power to draw.
//
//      /vi           - invert black and white on screen
//
//      /vo           - rotate image in screen window - can be useful with
//                      small LCD displays, etc.  Generally, you should
//                      specify /vo before any /v? screen size command line
//                      option.
//
//
//   The "$" keyboard menu also lets you change the screen resolution.  The
//   command letters are the same as used with the "/v" commands.  Note that
//   under Windows if you select a display size that is larger than your
//   screen, Heather will down-scale the screen by dropping pixels/lines...
//   which usually looks awful.
//
//   Changing the screen size from the keyboard may cause Heather to
//   drop / add items from the display depending upon if it can find space
//   to show them.
//
//   The /vu command (640x400 screen) will usually disable showing of DOP
//   (dilution of precision) values.  Use the GX keybord command or the /gx
//   command line option to re-enable showing the DOP values.
//
//   The "/vi" command line option and "$i" keyboard commands swap WHITE and
//   BLACK on the screen.  It looks horrible, but can be useful for doing
//   screen dumps that will be printed on paper... you remember paper,
//   don't you?
//
//   The "G~" keyboard command lets you configure the global screen color
//   palette. Heather uses a palette of 14 colors plus BLACK and WHITE. This
//   command lets you edit the RGB values for any of the 16 colors.  The
//   GxC keyboard command lets you assign one of the 16 colors to plot "x".
//
//   The color palette entry assigned to each of the the various plots can be
//   changed on a plot-by-plot basis.  See the PLOTTING section for details...
//
//
//
//
//
//-CONFIGURING DATA CAPTURE SIZES:
//
//   Lady Heather records several data values from the GPS receiver into
//   a circular buffer (called the "plot queue").  Once the plot queue
//   fills up, the oldest values are replaced with the newest values.
//   The default value for the plot queue size is 3 days of data (at one
//   second per point).
//
//   You can change the plot queue size with the "/q=" command line option.
//      /q=100000   - 100,000 second queue size
//      /q=2000m    - 2000 minute queue size
//      /q=300h     - 300 hour queue size
//      /q=40d      - 40 day queue size
//      /q=5w       - 5 week queue size
//
//   Each day of plot queue data (at one entry per second) uses around 15
//   megabytes of memory. Heather's basic memory usage (on Windows) is around.
//   10 megabytes. The plot queue and adev queues are the largest users of
//   of memory (in addition to the basic memory footprint).  Heather's default
//   configuration of a 3 day plot queue and 43,200 ADEV queue entries uses
//   around 90 megabytes of memory.
//
//   Normally the plot queue is updated once a second.  You can change the
//   queue update interval with the "/i=" command line option.  For queue
//   update intervals longer than 1 second, the GPS receiver data values are
//   averaged over the queue update interval and the averaged value is stored.
//     /i=60       - averages 60 GPS device readings and stores the result
//                   in the plot queue.
//
//   These values assume that your GPS receiver outputs data once per second.
//   If your GPS device outputs data faster than once per second (see the "!r"
//   keyboard command),  then these values must be scaled.  For instance, if
//   your receiver is sending 10 updates per second, "/q=100000" would hold
//   10000 seconds of data and "/i=60" would update every six seconds.
//
//   If you change the queue size from the keyboard (using the "/q"
//   keyboard command) the queue data will be erased.  Changing the plot queue
//   update interval does not automatically erase the queue data, but the
//   plot time horizontal scale factor indication (like VIEW: 5 min/div) will
//   not be correct for data that was captured before the update interval was
//   changed.
//
//
//   You can clear the plot queue via the "C" keyboard menu.
//
//
//   You can disable updating of the plot queue with new data:
//      UU  - from the keyboard toggles plot queue updating
//      /u  - from the command line toggles plot queue updating
//
//      Note that if a timestamping time interval counter is in use, the
//      ADEV queues will be reset because pausing the data stream disrupts the
//      timestamp sequence which will cause the ADEV values to be corrupted.
//
//
//
//
//
//-GPS WEEK ROLLOVER ISSUES
//
//   The GPS system does not actually broadcast a date.  It does broadcast
//   the number of weeks that have elapsed since the GPS system was first
//   activated. However this week counter is only 1024 weeks (around 19 years)
//   long.  Once the week counter passes 1024 weeks, the date sent by the
//   GPS receiver will be wrong (like 20 years in the past wrong).
//
//   Modern GPS receivers attempt to get around this limitation in various ways
//   which may delay when the rollover error occurs, but it will eventually
//   happen. If Lady Heather sees a date before 2018 from the receiver, it will
//   attempt to fix it by adding 1024 weeks to the GPS date/time until the date
//   appears to be reasonable.  If Lady Heather has "fixed" the GPS date, the
//   letters "ro" appear after the date in the upper left corner of the screen
//   and the date will be shown in YELLOW.
//
//   Note that if an automatic rollover correction is applied, the time will
//   jump.  Data in the plot queue that was collected before the rollover
//   correction will have the uncorrected time stamps.  This can cause the
//   the time interval data (shown as "<interval>" in the plot header to be
//   off from the true interval.  It is recommended that if you are using a
//   receiver with known rollover issues to use the "/ro" command line option
//   to force the rollover correction befor data collection starts.
//
//   Heather resets the plot queues if it detects a rollover fix within
//   20 seconds of starting the program.
//
//
//   You can force a rollover compensation value with the "/ro=" command line
//   option.  The rollover correction value can be either the number of 1024
//   week cycles to add to the date, or the number of SECONDS to add to
//   the date.  You can also specify an adjustment time in
//   weeks/days/hours/minutes/seconds by including w,d,h,m, and/or s values.
//   This feature was implemented for testing the calendar function, but
//   might have other uses.
//
//        /ro=0      - disable automatic GPS date rollover correction
//        /ro        - apply 1 1024 week rollover to the date
//        /ro=1*     - apply 1 1024 week rollover to the date
//        /ro=2*     - apply 2 1024 week rollovers to the date
//        /ro=12345  - add 12345 seconds to the date
//        /ro=-6w4d2h30m20s - subtract 6 weeks,4 days, 2 hours, 30 mintues,
//                     20 seconds from the date.
//
//   Two other options are available for testing calendar/DST code.  These
//   adjust the current rollover adjustment by the indicated value.  These
//   should normally only be used from the keyboard.
//        /ro-30m20s - subtract  30 mintues, 20 seconds from the current
//                     rollover value
//        /ro+20s    - add 20 seconds to the current rollover value
//
//
//
//
//
//-CONFIGURING FILTERS:
//
//   Many GPS receivers have options for filtering their position data in
//   various ways and optimizing them for the expected movement environment.
//
//   These command line options allow configuration of the
//   various receiver filters from the command line.  Note that not all
//   receivers support all of these filters.  These filters are also
//   usually changeable from the "F" keyboard menu.
//
//     /fa - toggle altitude filter
//     /fi - toggle ionospheric filter (Motorola)
//     /fk - toggle Kalman filter
//     /fp - toggle PV filter (position/velocity filter)
//     /fs - toggle static filter
//     /ft - toggle tropospheric filter (Motorola)
//
//           For STAR-4 receivers the FT command controls fixing timestamp
//           errors.  The STAR-4 occasionally skips a time stamp.  Setting
//           this flag will attempt to compensate for the missing timestamp,
//           but this can cause the time to actuaaly be off by one second
//           until the receiver requests satellite info at time hh:mm:33
//
//
//     /fd=# - display filter - the display filter averages # consecutive plot
//             queue values to produce a plot point.  This display filter does
//             not change the values stored in the plot queue so can be
//             changed freely.
//
//             When you are viewing a section of the plot queue that is
//             longer than the plot area in pixels, the extra plot values are
//             generated by skipping plot queue values.  This can hide short
//             "glitches" in the data and things like the time skip, holdover,
//             and satellite changes.  The display filter helps these missing
//             values become visible (particularly using the "FD -1" setting).
//
//             If the display filter is turned on,  the values shown
//             at the mouse cursor are the filtered values.
//
//             Be careful when using large display filter values
//             with long view time displays.  It can take a lot
//             of time to process the data on slower machines.
//
//             If the filter count is a negative value a "per-pixel" filter
//             count is automatically determined based upon the plot view
//             interval and the plot display width.  If the "per-pixel" filter
//             is selected the calculated filter count in shown in square
//             brackets like [30].
//             A value of -1 averages (view_interval / PLOT_WIDTH) values
//             A value of -2 averages 2*(view_interval / PLOT_WIDTH) values
//             A value of -3 averages 3*(view_interval / PLOT_WIDTH) values
//             etc...
//
//             The display filter averages the "n/2" data points each side
//             of the queue entry to create the displayed data point.  If the
//             queue entry being averaged is less than "n/2" points from
//             the end of the captured data, the previous points in the
//             queue are averaged.  If there are less than "n" data points in
//             the queue then all the data points available are averaged.
//
//             The display filter can generate some discontinuities in the
//             plots at points within "n" points of the plot queue start
//             and end values.
//
//
//   There are also some filter settings only changeable via keyboard commands
//   in the "F" menu.  If the receiver does not support one of these filters
//   it will not appear in the "F" menu.  The "/f" filters listed above are
//   also accessible from the "F" keyboard menu.
//
//      FC - coordinate filter
//      FE - satellite elevation mask angle - satellites below this angle
//           above the horizon will not be tracked
//      FF - foliage filter
//      FI - set ionosphere filter (Motorola receivers can do this)
//      FJ - jamming mode filter
//      FL - satellite signal level mask - satellites with signals below this
//           setting will not be used
//      FM - movement / marine filter
//      FT - set troposphere filter (Motorola receivers can do this)
//           (or filter timestamp errors for STAR-4 receivers).
//      FX - PDOP mask / switch setting (causes receiver to switch to 2D mode
//           if PDOP is less than the specified value).
//
//   Many of the filter settings are shown in the system status column of
//   the screen display.  You can disable showing the filters:
//      GY  - toggle filter display on/off
//      /gy - toggle filter display from the command line
//
//
//      GX  - toggle the DOP (dilution of precision) value display in the
//            receiver status information column.
//      /gx - toggle DOP display from the command line
//
//
//   The Thunderbolt GPSDO firmware smooths the output of the
//   temperature sensor with a filtering algorithm. This
//   filtering can mask and prolong the occasional single
//   point glitches that the temperature sensor produces.  Lady
//   Heather's default action is to reverse the filtering
//   that the Thunderbolt firmware does.
//   Removal of the temperature sensor filter makes the sensor
//   glitches much more obvious in the temperature plots and
//   minimizes their effect in the active temperature
//   control mode.  It also makes the temperature display
//   a little less smooth since you now see the raw sensor
//   readings that have around 0.01C increments (0.16C on receivers
//   that have the newer, but lower resolution, temperature sensors).
//   The "/tj" command line option toggles the temperature sensor
//   un-filtering mode and the smoothed temperature data is used.
//
//   On Thunderbolts and other Trimble receivers you can save the filter
//   configuration into EEPROM using the "EE" keyboard command.
//   This will write the complete current receiver configuration into EEPROM.
//
//
//
//
//
//-LOG FILES
//
//   Lady Heather can write log files of the receiver data and program status.
//   The log files can be written in real-time as the data comes in from
//   the receiver or from data saved in the plot queue.
//
//   Log files are automatically closed and re-opened once per hour.  This
//   provides some protection from data loss if your system crashes or loses
//   power.
//
//   The contents of the log files depends upon the receiver type.
//
//   Lady Heather supports several different log file formats.  The file format
//   is determined by .EXTension of the log file name:
//      .log  - a simple ASCII format log
//      .xml  - a GPX 1.1 format file that supports extended / user defined
//              data types.
//      .gpx  - a standard GPX 1.0 format file (a standardized XML file
//              oriented towards time and location data.
//      .kml  - a Google KML format location file (only stores time and
//              lat/lon/alt)
//      .obs  - RINEX format raw observation data (pseduorange, carrier phase,
//              doppler, signal level) output of receivers that can supply this
//              data.  RINEX data can be post-proceesed to provide precise
//              locations.  See the section below on RINEX files.
//
//   Note that .gpx / .xml / .kml / .obsfiles are larger than the .log ASCII
//   files.  The .xml (GPX 1.1) log format contains the most comprehensive
//   data including pretty much the complete receiver configuration.
//
//
//   To start logging data from the keyboard:
//      WLW or LW - write a new log file from real-time data
//      WLA or LA - append real-time data to a log file
//      WLS or LS - stop logging real-time data
//      WLH or LH - write a new log file from real-time data in formatted
//                  HEX/ASCII packet format
//
//   From the command line:
//      /w=file   - set log file name to Write to
//      /wa=file  - set log file name to Append to
//
//      /wh       - toggles writimg the log file as a formatted  HEX/ASCII
//                  packet log file instead of a data log.
//
//
//   You can also write RINEX files as a separate file.  This allows you to
//   write a standard log file and a RINEX file at the same time:
//      MW - write a separate RINEX file  (from the keyboard).
//      /mw=file  - set RINEX file name to write to (from the command line)
//
//   If you add an "*" to the end of the log file name, the log file write
//   buffer is flushed to disk on every write.  If log flush mode is enabled
//   the "Log:" file name indicator is shown as "LOG:"  If the log file is
//   already open then the /wq command line option will toggle log flush mode.
//
//
//   You can also write a file of tracked satellite time, PRN, azimuth,
//   elevation, and signal level data:
//      WLP or LP - write a sat .prn log file from real-time data.
//
//   If you add an "*" to the end of the PRN file name, the prn log file write
//   buffer is flushed to disk on every write.  If prn flush mode is enabled
//   the "Prn:" file name indicator is shown as "PRN:"
//
//
//   You can write a log file from data in the plot queue:
//      WA  - writes ALL data in the plot queue to a log file
//      WP  - writes the data from the area of the plot queue that is
//            being displayed on the screen.
//
//            Note that the *tow* time-of-week field in logs generated from
//            queue data is not the official GPS time-of-week,  but is just
//            a sequential number.
//
//            If writing the plot window queue data to a log file
//            AND the plot queue is full and warping AND the queue
//            updates are not paused AND the plot window covers
//            all of the queue data,  then there may be a glitch
//            at the beginning of the log output file.  Several
//            seconds of the latest data can appear at the start
//            of the log data.
//
//            You cannot write a RINEX file from plot queue data.
//
//
//   You can delete files:
//      WD  - deletes a file
//
//
//   Normally Heather updates the log file every second (or every time a
//   new receiver data point come in).  You can configure the log update
//   interval.
//      WLI or LI     - set the log update interval  -or-
//      /l[=seconds]  - from the command line
//
//
//   Normally Heather does not write the satellite constellation data (sat
//   PRN, azimuth, elevation, signal level) to the log file.  You can enable
//   logging of the satellite constellation data:
//      WLC or LC - toggle logging of satellite constellation data
//      /ld       - from the command line
//                  Note that the sun is logged as satellite PRN 1000 and
//                  the moon as PRN 1001.  (WAS 256 AND 257 IN PREVIOUS
//                  RELEASES)
//
//
//   Normally Heather writes unfiltered data to log files.  If the display
//   filter is enabled (FD keyboard command) you can toggle the writing of
//   filtered data to the log file:
//      WF  - toggle writing of filtered data to the log file.
//
//
//   Normally Heather waits for any receiver id information before writing
//   data values to log files. You can configure heather to write log data
//   brfore the receiver id information has arrived.
//   This command can be useful if Heather thinks that a receiver should be
//   sending id information, but the receiver has non-standard firmware that
//   does not send id information.
//      /li - from the command line - toggles the "log id wait" flag.
//
//
//   Besides data logs, Heather also supports writing a debug log file:
//      WX  - write debug log file  -or-
//      /dl[=file]  - from the command line (if a file name is not given
//                    "debug.log" is used.
//      The content of the debug log is device dependent and subject to
//      change.
//
//
//   Heather also supports writing a receiver data capture file:
//      WY          - write receiver data capture file from the keyboard
//      /dr[=file]  - from the command line (if a file name is not given
//                    "heather.raw" is used.)
//
//      If you add a '*' to the end of the data capture file name, then the
//      data is flushed to disk every byte (default is to let the OS write the
//      data to disk whenever its internal buffer is full.  Flushing to disk
//      every byte is OS resource intensive but helps make sure all data
//      has been written to disk if an unexpected program crash occurs.
//      In raw flush mode the "Cap:" log file name is shown as "CAP:".
//
//
//
//   If a time interval counter is being used Heather also supports writing
//   a raw counter data capture file:
//      WT          - write counter data log file from the keyboard
//      /dq[=file]  - from the command line (if a file name is not given
//                    "ticc.raw" is used.)
//
//
//   In ASCII format logs, the data separator can be changed from a TAB to
//   a comma with the command line option:
//      /ls   - toggle log file value separator between TABs and COMMAs
//      /ls=c - set separator char to "c".
//
//
//   You can disable comments in log files from with the command line option:
//      /lc  - toggle writing any comments to the log file
//             Note that disabling log comments means Heather cannot
//             calculate the date of a log entry or the log data interval
//             because these values are stored as special comments.
//
//             ALSO: to read a log file that was written without comments,
//             the /lc command must be in effect,  otherwise the file will not
//             be recognized as a log file because Heather uses the "#" comment
//             line on the first line of the file to help it recognize the
//             file is a potential log file.  XML/GPX logs don't have this
//             limitation.
//
//
//
//   You can disable logging of detected errors with:
//      /e   - toggle logging of detected errors in the log file
//
//
//   Lady Heather can read in log files and show the data in the plot area.
//   Reading in a log file first pauses the plot queue updates from live
//   incoming data.  The current plot queue data is cleared out unless
//   you preceed the file name with a '+' to append the log data to the
//   current data. You can then scroll around the the log file data
//   using the normal plot viewing commands.  You can resume processing
//   of receiver data using the "U" keyboard command... you might want
//   to first clear the plot queue data ("C" keyboard menu).  RINEX files
//   cannot be read in.
//
//      R       - read a file (with .log .xml .gpx file extension)
//      /r=file - from the command line
//
//                Note that If you  read in a log file, Heather should first
//                be configured ("/rx#" command line option) for the receiver
//                type that created the log file.
//
//                Note that only one log file can be read using the "/r="
//                command line option.  You cannot use multiple /r= options
//                to read and append multiple log files.
//
//
//
//   Log files can be automatically written on a scheduled basis.  See the
//   section on SCHEDULED EVENTS for details.
//
//
//
//
//-RINEX files
//
//   Some receivers can supply "raw" data observations from the GPS satellites
//   (not to be confused with Heather's "raw receiver data capture files" which
//    record all data sent from the receiver to a file).
//   Raw observation data includes pseudoranges, carrier phase data, doppler,
//   and signal levels.  Heather can write this data to a RINEX format
//   observation file.  RINEX is an insustry standard file format used to
//   transfer raw data observations to post-processing services to get a
//   precise location.
//
//   Currently supported receivers that can supply "raw" observations include:
//      Ashtech Z12 (L1 and L2 observations)
//      Furuno ESIP receivers  (if baud rate is >= 115200 baud).
//      Motorola Oncore VPZ receiver (with raw measurement data firmware)
//      NVS-08 receiver
//      Ublox timing receivers
//      Most Trimble TSIP receivers and GPSDOs (SV6 does not work).
//      Trimble RT17 format receivers like the NetRS
//      Venus/Navspark RTK capable receivers
//
//      The ESIP and TSIP receivers output only pseudorange, doppler, and
//      signal level observations.
//
//      The Trimble telecom GPSDOs (NTBW,NTPX,NTGS,etc) can produce rather
//      noisy results and may not yield a usable solution.  This may be
//      related to their tendency to cause data jumps when satellites enter
//      and leave the tracked constellation.  Increasing the satellite
//      elevation and/or signal level filters may help... but settings that
//      are too high can cause less than 4 satellites to be tracked... which
//      is a bad thing...  (Update: recent changes to Heather has greatly
//      improved the resuluts with these receivers).
//
//      The Trimble receivers can take around 30 seconds before all satellite
//      ephemeris data has been collected and is available for generating
//      RINEX files.  The original Resolution-T takes around 90 seconds to
//      gather all the ephemeris data.  Also this reveiver requires thet the
//      RINEX data rate be set to 3 seconds.  Note that since CSRS-PPP updated
//      their processing system, performance with Trimble receivers was
//      greatly reduced!!!  Typically over 50% of observations are rejected.
//
//      The Ashtech Z12 receiver gets its time display from the system clock.
//      It also uses the system clock for determining the GPS week number
//      rollover correction value.  If the system clock is not accurate you
//      should avoid collecting data that includes the GPS week number change
//      (00:00 GPS time on Sunday).  This can cause a glitch in the calculated
//      observation timestamps and most RINEX processing services will reject
//      the file.
//
//      You can verify that the receiver can generate a RINEX file by clicking
//      on the satellite information table.  If you see a PSEUDORANGE or
//      CARRIER column, you shouild be able to generate a RINEX file.
//
//   Before a RINEX file will be written, the receiver must have a valid time
//   and UTC offset value.
//
//   A lot of receivers seem to have a problem with their raw data observations
//   around the time of the GPS week rollover (00:00 UTC on Sunday).  It is
//   best to not collect data that spans this time.
//
//   For best results the receiver should be in position hold mode with a
//   self-surveyed or reasonably accurate position, but this is just a
//   suggestion.
//
//   Note that there can be a LOT of raw data.  You will probably need to
//   configure the receiver to a high baud rate to prevent over-running the
//   receiver transmit buffer.  Heather now has the "!x" and MX keyboard
//   commands for setting the receiver baud rate for devices that support raw
//   data.  IF YOU SET THE RECEIVER TO A NON-DEFAULT VALUE YOU WILL NEED TO
//   SPECIFY THE BAUD RATE ON THE COMMAND LINE (/br=...) WHENEVER YOU START
//   HEATHER.  If the receiver type can be auto-detected and the baud rate is
//   one of the supported auto-detect rates (9600,19200,38400,57600,115200
//   baud) then you may not need to force the baud rate setting.
//
//   You can write the RINEX file using log file commands and specify an
//   file name extension of .obs or you can write the RINEX file as a separate
//   file (this allows you to write a normal log file and a RINEX file at
//   the same time.  See the section above on the commands used to enable
//   RINEX format files.  Update: Writing RINEX files as a log file is now
//   deprecated and should not be used.
//
//   The receiver should be configured for a navigation message output rate
//   of one Hz (!r command).  Heather does have some code for working
//   with receivers that are outputing position data at faster rates... but
//   it is a bit wonky and may not work properly in all cases.  Also many
//   receivers seem to mess up / skip raw data messages at faster nav rates.
//
//
//   Heather has several commands for configuring RINEX files.  You should NOT
//   use any of these commands while a RINEX file is being written.
//
//     /rr=secs -  sets the receiver raw data mesage output rate (in seconds).
//                 RINEX files can be very large.  The /rr command lets you
//                 specify the raw data observation rate.  Faster rates can
//                 produce more accurate results, but generally rates of
//                 10-30 seconds are about as good as faster rates.  Heather
//                 does not currentlt support fractional second raw message
//                 rates or rates faster than 1Hz.
//      MR         - from the keyboard
//
//                 For the Ublox M8N receiver, a negative value for raw rate
//                 enables the undocumented TRK_MEAS and TRK_SRFBX messages.
//                 These messages are only available for M8N receivers with
//                 early versions of the firmware.  Heather does not parse
//                 them, but the messages will be written to the reciver "raw"
//                 data capture file (WY command) if it is enabled.  RTKLIB
//                 can be used to process the capture file into a RINEX file
//                 (don't confuse receiver raw data capture files with RINEX
//                 obervattion "raw" measurements / RINEX files).
//
//                 The original Trimble Resolution-T receiver has firmware
//                 issues that cause it to only be able to output RINEX data
//                 every 3 seconds.  Normally Heather does not enable the
//                 raw data for this receiver, but if you set the raw rate
//                 to 3 seconds, it will do it... but the clock will only tick
//                 every 3 seconds. Setting the rate < 3 seconds  disables
//                 raw data.  Setting values over 3 seconds will use 3 seconds.
//
//                 For all all the Trimble receivers you should click on the
//                 satellite information table and verify that the receiver
//                 is showing PSEDORANGE data for all tracked satelliites
//                 before starting to write a RINEX file.  This usually
//                 takes around 30 seconds or less.
//
//                 Some receivers only output observations at 1 Hz.  For
//                 thse receivers, when you set the raw rate to other than
//                 1 Hz,  Heather drops unwanted observations by calculating
//                 (GPS time-of-week mod raw_message rate).  You must specify
//                 a raw message rate that divides evenly into 604800.
//
//
//      /mw=name   - from the command line (this is not recommended because
//                   not all satellite information may be available before
//                   the file starts writing)
//      MW         - from the keyboard - opens/closes the RINEX file to write.
//
//
//      MX         - from the keyboard - sets the receiver baud rate.
//                 See the warning above about the implications of setting
//                 the receiver baud rate to its non-default value.
//
//      MI         - set bad observation data fixup mode (default is enabled).
//                 If enabled, Heather attempts to fix potential errors in
//                 the RINEX data.  Values that are "stuck" and not changing
//                 are blanked.  Duplicate observation time stamps are deleted,
//                 missing observations are replaced with null records, etc.
//
//
//      /at=antenna - sets the antenna type to include in the RINEX file.
//                 Many survey grade antennas have precisely measured
//                 characteristics (like their phase center) and if these
//                 are known they can be used to improve the post-processing
//                 results.  Most post-processing services have a library
//                 of these antenna types which can be selected using an
//                 antenna type code.  The /at or MA commands lets you specify
//                 the antenna type code.  Or, you can manually edit the
//                 RINEX file to include the proper antenna type code.
//                 Any '_' in the antenna name are replaced with a ' '. If
//                 the offical antenna name has an '_' in it, you will need
//                 to manually edit the RINEX file antenna number/type record
//                 in the file header.
//      MA         - from the keyboard  (the MA command does not have any
//                   limittations on '_' or ' ' in the antenna name).
//
//
//      /ah=antenna_height - Lets you specify the height of the antenna above
//                 the ground.  If you don't specify an antenna height, the
//                 post processing services will calculate the location of
//                 the antenna (which is what you probably want if you
//                 are configuring a timing receiver).  If you specify the
//                 antenna height then the services will return the location
//                 of the point on the ground directly below the antenna.
//                 Besides the antenna height displacement you can also set
//                 the east-west and north-south displacements:
//      /ah=height,ew,ns - sets the antenna height and east-west and
//                 north/south displacements (in meters).
//      MH         - from the keyboard
//
//
//
//   The following antenna and marker commands provide descriptive information
//   to be included in the RINEX file.  This information is optional and does
//   not alter the the location results.
//
//      /an=number - sets the antenna number to include in the RINEX file.
//                The number can be alphanumeric.
//      MU         - from the keyboard
//
//
//
//      /ak=marker - sets the marker name
//      MK         - from the keyboard
//                 If a marker name is not given, Heather creates one from
//                 the receiver type (and a string of characters that
//                 represent the satellite systems that are being used (if
//                 available and the receiver supports multiple GNSS systems).
//
//      /av=marker - sets the marker number
//      MV         - from the keyboard
//
//
//
//
//      /rm=raw_meaurements - Heather defaults to including all available
//                 raw measurements in the RINEX file.  The /rm command
//                 lets you control which measurements that you want to
//                 include in the RINEX file (and also the order they are
//                 written in.  The /rm command is used for RINEX v2.11 files.
//                 RINEX v3.xx has separate observaion lists for each satellite
//                 system and uses different (3 character) measurement type
//                 codes.
//
//                 Supported v2.11 measurments are:
//                    C1 - C/A code pseudorange
//                    L1 - L1 pseudorange
//                    L2 - L2 pseudorange
//                    L5 - L5 pseudorange (GPS/GLONASS/GALILEO)
//                    L6 - L6 pseudorange (Galileo)
//                    L7 - L7 pseudorange (Galileo)
//                    L8 - L8 pseudorange (Galileo)
//
//                    P1 - L1 carrier phase
//                    P2 - L2 carrier phase
//                    P5 - L5 carrier phase (GPS/GLONASS/GALILEO)
//                    P6 - L6 carrier phase (Galileo)
//                    P7 - L7 carrier phase (Galileo)
//                    P8 - L8 carrier phase (Galileo)
//
//                    D1 - L1 doppler
//                    D2 - L2 doppler
//                    D5 - L5 doppler (GPS/GLONASS/GALILEO)
//                    D6 - L6 doppler (Galileo)
//                    D7 - L7 doppler (Galileo)
//                    D8 - L8 doppler (Galileo)
//
//                    S1 - L1 signal level
//                    S2 - L2 signal level
//                    S5 - L5 signal level (GPS/GLONASS/GALILEO)
//                    S6 - L6 signal level (Galileo)
//                    S7 - L7 signal level (Galileo)
//                    S8 - L8 signal level (Galileo)
//                  Note: Heather curently does not support any receivers
//                  with L5/L6/L7/L8 signals.
//
//         /rm       - limits measurements to the L1 data
//         /rm=list  - 'list' is a comma separated list of the measurement
//                     types to include like:
//                     /rm=C1,D1 - put only C1 and D1 data in the RINEX file
//         MM        - from the keyboard
//
//   For RINEX v3.xx the various GNSS satellite systems have separate
//   independent observation lists.  You can set the observation types to
//   be used for each system:
//         /rmb=list - BEIDOU
//         MB        - from the keyboard
//         /rmg=list - GPS
//         MG          from the keyboard
//         /rmn=list - GLONASS
//         MN          from the keyboard
//         /rml=list - GALILEO
//         ML        - from the keyboard
//         /rms=list - SBAS
//         MS          from the keyboard
//
//   Heather normally writes a RINEX v2.11 format file.  There is also
//   support (currently incomplete) for writing v3.xx files.  You can select
//   the RINEX format with the /rf command line option.  Note that none of
//   the standard online post-processing services seems to accept RINEX v3.xx
//   files.
//      /rf         - toggle between v2.12 and v3.03
//      /rf=version - set specific version type.
//      MF          - set RINEX version from the keyboard
//
//   Heather includes raw measurement data from all the satellites that it
//   sees.  If the receiver supports multiple satellite systems (GPS/SBAS/
//   GLONASS/GALILEO) and you want to exclude a particular satellite system's
//   data from the RINEX file use the SG keyboard command to disable tracking
//   that system.
//
//   Another way to get RINEX files from Lady Heather is to write a raw
//   receiver data capture file (WX command) and use a program like RTKLIB
//   or TEQC to process the raw receiver data capture file into a RINEX file.
//
//   Once you have a RINEX file you will need to process it to get precise
//   position data.  The easiest way to do this is to submit it to an
//   online processing service.  There are several free services available
//   but most only accept data from dual frequency (L1/L2) receivers.  You
//   should probaly .ZIP the file before submitting it since they can be
//   very large.  AUSPOS does not accept .ZIPed files.
//
//   The CSRS-PPP service from NRCAN (Natural Resouces Canada) can handle
//   L1 only data.  Also, they probably have the best output reports. They
//   are the currently recommended processing service and the only one
//   that currently handles L1 only data.  Their results are available
//   2-3 hours after the last record in the file was writen.  If the file
//   contains GLONASS data it can take a bit longer for them to get the
//   GLONASS orbits.  If you  submit a file before the GLONASS orbits are
//   available, they will process the GPS data and send a report.  Later
//   they will send another report with the GPS and GLONASS data.  NOTE:
//   for L1 only receivers, CSRS-PPP currently seems to only process the
//   pseudorange data!!!  Carrier phase and dopper measurements are
//   ignored!!!   CSRS-PPP's current (upgraded?) processing system does
//   NOT work well with Trimble receivers... typically over 50% of the
//   observations are rejected!
//
//   AUSPOS in Australia seems to report the smallest error ellipse sizes,
//   but the actual positions seem to differ from the results of other
//   processing services (perhaps due to the long base-lines to the referecnce
//   stations that they use). Note that you have to wait 24 hours after the
//   RINEX file was written before submitting the file.  AUSPOS does not
//   seem to like .ZIPed files.  They do support FTP uploads.
//
//   OPUS in the United States seems to be VERY finicky about the input files.
//   They seem to reject a lot of files that CSRS-PPP and AUSPOS have no
//   problem with.  Note that if you use a Z12 receiver, OPUS does not work
//   with the Z12's default 20 second raw data rate.  You will need to use
//   the /rr= or MR command to set the raw data message interval to something
//   it can use.  OPUS does not work with L1 only data.  Although OPUS says
//   they can process files up to 48 hours long, but those 48 hours can only
//   span two calendar days. So, if you start a 48 hour run in the middle of
//   the day, OPUS will not process the data... bastards!
//
//   Also note that all the post processing services have VERY poor error
//   reporting if something goes wrong.  The typical error message is along
//   the lines of "Error in file" with very little hint as to what or where
//   the error is.
//
//   The post-processing services use precise satellite orbit measurements,
//   etc to massage the receiver RINEX data to get precise locations.  There
//   are three main types of orbit data that they use:
//      Ultra-rapid - based upon current and predicted measurements of the
//                    satellite orbits.  The ultra-rapid results are the
//                    least accurate, but still are pretty good.  Typical
//                    results may be a few centimeters worse than the
//                    more precise orbits.
//      Rapid       - The rapid orbits are based upon data from worldwide
//                    monitoring stations.  They are available around 24
//                    hours after the RINEX file was written.
//      Precise     - the precise orbits are available 1-2 weeks after the
//                    RINEX file was written.  They give the most accurate
//                    results (but usually not significantly better than
//                    the rapid orbits).
//
//   Note that some receivers (like the Furuno ESIP receivers) report/want
//   altitude as "Orthometric" height / height above geoid.  Most receivers
//   use GPS (geoid) altitude If you are using the post-processing results
//   to set your receiver's position hold location, make sure that you use
//   the correct altitude value (WGS84 geoid or Orthometric).
//   Also, different receivers may use different geoid models than the
//   post-processing service to get ortometric altitudes.  This can lead to
//   reduced accuracy.  Consult your receiver manual to find out if it uses
//   GPS or othometric altitude... hopeully they bother to tell you which one
//   they use... but don't count on it!  At my location the GPS and
//   orthometric altitudes differ by around 25 meters so using the correct
//   altiude that the receiver expects is important.
//
//
//
//
//-TIME DISPLAYS
//
//   Lady Heather has three main time displays:
//      1) The date/time block in the upper left hand corner of the screen
//      2) A digital clock display
//      3) An analog watch display.
//
//
//   The date/time block is always shown. It contains the time / time zone,
//   the date,  the GPS week number,  the GPS time-of-week, and the "UTC offset"
//   leap second count.  It also indicates whether the GPS receiver is running
//   in GPS time mode or UTC time mode and the validity of the receiver time.
//
//   If the receiver does not report the GPS week or time-of-week then
//   Heather will derive these values from the time and both the WEEK: and
//   TOW: values will be shown in YELLOW (instead of WHITE).
//
//   If a GPS week rollover condition has been detected and is being
//   compensated for, the date is shown in YELLOW and is followed by " ro".
//
//   The time in the time block is normally shown if BLUE if daylight savings
//   time is not being applied,  CYAN if daylight savings time is in effect,
//
//   The time can be shown in a few different formats:
//     /tp  - shows time as fraction of a day
//     /tq  - shows time as total seconds of the day
//
//
//
//   The digital clock display is located between the satellite information
//   data and the plot area,  It is normally CYAN in color.  If the GPS
//   receiver reports invalid time, it shows in RED.  If no UTC leapsecond
//   offset is available, it shows in YELLOW.
//
//      GZ  - from the keyboard toggles the digital clock on/off
//      /gz - from the command line
//
//   Clicking on the digital clock display (or just under the satellite
//   information display) will zoom the digital clock display to full screen.
//   Cicking again will restore the screen to its previous state.
//   Note that short clicks might be ignored... particularly on devices like
//   UCCM receivers that only output time updates every two seconds.  It can
//   help to hold down the mouse button until the screen changes.
//
//
//   You can control the format of the digital clock:
//      TM   - toggles between a seconds resolution clock or a millisecond clock
//      /tsz - from the command line
//
//      TW   - toggles between a 24 hour clock and a 12 hour AM/PM clock
//      /ti  - from the command line
//
//      TJ   - toggles between a Julian date.time clock and a normal clock
//      /tsj - from the command line
//
//      TQ   - toggles between a Modified Julian date.time and normal clock
//      /tsk - from the command line
//
//      TN   - normal 24 hours seconds clock
//      /tsn - from the command line
//
//      /tsu - display time in Unix epoch seconds.
//             This time is adjusted for the currently set local time zone.
//             For actual Unix time, set the time zone offset to 0.
//
//      /tsg - display time in GPS epoch seconds.
//             This time is adjusted for the currently set local time zone.
//             For actual GPS time, set the time zone offset to 0.
//
//      TI   - time interval (stopwatch)
//      /tsw - from the command line
//
//             This toggles the digital clock display between the last selected
//             clock format and the number of seconds between the current time
//             and the time the last clock format command was issued (or the
//             time Heather was started up if no clock format command was
//             ever set).  So, to reset and stop the "stopwatch", issue a time
//             format command (such as TN). Then you can issue the TI command
//             to restart the stopwatch.
//
//
//
//   The watch display is located either in the upper right hand corner of
//   the screen or to the right of the plot area.  The watch outline is
//   normally shown in BLUE or CYAN.  It is drawn in RED if the GPS reports
//   that the time is not valid,  and in YELLOW if no UTC leapsecond offset
//   is available.
//
//      GW  - from the keyboard toggles the watch on/off
//      /gw - from the command line
//
//   Clicking on the watch display on the screen will zoom it to full
//   screen.  Clicking again will restore the screen to its previous state.
//   Note that short clicks might be ignored... particularly on devices like
//   UCCM receivers that only output time updates every two seconds.  It can
//   help to hold down the mouse button until the screen changes.
//
//
//   You can label the watch face with a "brand name".  On small screens there
//   might not be room to show the brand name:
//      /tb      - toggles labeling of the watch face
//      /tb=name - sets a brand name on the watch face. Any "_" in the brand
//                 name is converted into a space.  The brand name can be two
//                 lines long.  Separate the lines with a "/".  A two line
//                 watch name does not show the date in the clock face.
//      /tb=     - sets the watch "brand name" to the day of the week.
//                 This is the default setting.
//
//
//   You can specify the watch face style with the /wf= command line option
//     /wf=0  - normal Roman numeral clock
//     /wf=1  - decimal hours clock
//     /wf=2  - "*" hour markers
//     /wf=3  - 24-hour Roman numeral clock
//     /wf=4  - 24-hour decimal hours clock
//     /wf=5  - 24-hours "*" hour markers
//
//     /wt    - straight shaped hour/minute hands
//     /wt=0  - straight shaped hour/minute hands
//     /wt=1  - filled trapazoidal shaped hour/minute hands (default)
//     /wt=2  - hollow trapazoidal shaped hour/minute hands
//
//
//
//   The watch display includes a representation of the moon at its current
//   location in the sky and with its current phase.  The moon is shown in
//   YELLOW if it is visible and GREY if it is below the horizon.  The sun
//   is shown as a filled in yellow circle with "rays" if it above the horizon
//   and as an empty yellow circle with rays if it below the horizon.
//   The watch display also flags when the moon is new, quarter, or full.
//   Also "blue" and "black" moons are shown.
//
//   You can disable the sun/moon images in the satellite map and watch
//   displays using the command line options:
//      /kj   - toggle sun/moon drawing
//      /kj=0 - disable sun/moon drawing
//      /kj=1 - disable sun drawing
//      /kj=2 - disable moon drawing
//      /kj=3 - disable sun and moon drawing
//
//
//   Note that the watch display is not updated while a keyboard command
//   menu is being displayed and the "GB" option is in effect that allows
//   maps and/or the watch to be displayed in the plot area.
//
//
//   The ZC keyboard command will zoom the clock display to full screen.
//
//   The ZW keyboard command will zoom the watch display to full screen.
//      Clicking the mouse will toggle between the zoomed watch and digital
//      clock displays.
//      Clicking in the upper left hand corner of the screen will restore
//      the screen to normal mode.
//      Note that short clicks might be ignored... particularly on devices
//      like UCCM receivers that only output time updates every two seconds.
//      It can help to hold down the mouse button until the screen changes.
//
//   The ZB keyboard command will zoom the watch and satellite map displays
//   to full screen overlayed on each other.
//      Clicking the mouse will switch to the ZV display where you can then
//      click again to select another display to zoom to full screen.
//      Clicking in the upper left hand corner of the screen will restore
//      the screen to normal mode.
//      Note that short clicks might be ignored... particularly on devices
//      like UCCM receivers that only output time updates every two seconds.
//      It can help to hold down the mouse button until the screen changes.
//
//
//   Note that the watch or clock do not have to be enabled on the main
//   screen for the zoomed displays to work.  Pressing any key (except '\')
//   will restore the main display screen if it has been zoomed. '\' will
//   dump a .GIF image of the zoomed screen to the file "xxxx.gif" where xxxx
//   is the receiver type (if you press SPACE to bring up the keyboard help
//   screen, the file name used will show up by the '\' entry).
//
//   You can also force a screen dump by left-clicking on the screen where
//   the "Receiver data" info is near the upper left hand corner of the
//   screen.  This is handy if you are using a touch-screen.
//
//
//   The "/ta" toggle command line option shows dates in the European
//             the European dd.mm.yyyy format instead of the normal 12 Oct 2016
//             format.
//
//   The "/tl" toggle command line option shows dates in the ISO
//             yyyy.mm.dd format instead of the normal 12 Oct 2016
//             format.
//
//
//
//
//
//--PENDING LEAPSECOND DISPLAY
//
//   Many GPS devices report when a leapsecond adjustment has been announced.
//   Heather shows this in the receiver status column. Leapsecond
//   announcements are generally made 6 months in advance.
//
//      If the receiver supports leapsecond announcements but no leapsecond
//      is pending it shows "No leap pend" in GREEN.  If a leapsecond
//      adjustment has been announced it shows "Leap pending" in YELLOW.
//
//      Many receivers send enough enough information to determine the date of
//      the leapsecond event.  If this information is available the leapsecond
//      status shows a countdown clock like "Leap 45 days".  If the receiver
//      does not report the exact date of the leapsecond, Heather assumes it
//      will be on the last day of June or December and indicates this guess
//      by "Leap 45 days?".
//
//      As the time of the leapsecond nears, the countdown clock starts showing
//      hours, then minutes, then seconds until the leapsecond.  When showing
//      hours, any fractional hour count is rounded up,  so the countdown
//      clock will show 2 hours then 59 minutes.
//
//      Some receivers (like the Z3801A and Z3812A have firmware bugs that
//      expect a leapsecond announcement no more than 3 months in advance.
//      These receivers will display an invalid leapsecond date while the
//      pending leapsecond is more than three months in the future and may
//      take several days to figure out the proper leapsecond date once the
//      erroneous date has passed.
//
//
//
//
//
//
//--DATE AND CALENDAR DISPLAYS
//
//   Lady Heather can display the date in various calendar formats. The
//   calendar type is selected by a command line option:
//      /d  - default Gregorian calendar
//      /da - Afghan calendar
//      /db - Mayan Haab calendar
//      /dc - Chinese calendar
//      /dd - Druid calendar
//      /dh - Hebrew calendar
//      /di - Islamic calendar
//      /dj - Julian date
//      /dk - Kurdish calendar
//      /dm - Modified Julian Date (MJD)
//      /di - Indian civil calendar
//      /dp - Persian calendar
//      /ds - ISO date (yyyy-day_of_year)
//      /dt - Tzolkin calendar
//      /dv - Bolivian calendar
//      /dw - ISO week format (yyyyWww-dow)
//      /dx - Xiuhpohualli (Aztec Haab) calendar
//      /dy - Mayan calendar
//      /dz - Aztec Tonalpohualli calendar
//      /dyyyymmdd - force date to year yyyy, month mm, day dd for testing
//                   (use a leading zero for months/days less than 10)
//
//   For several of the calendars you can specify an epoch correction factor
//   for the date calculations.  The correction factor is the number of days
//   (+ or -) to add to the default date calculation.  This is to compensate
//   for various interpretations of when the calendar should begin.
//
//      /db=days   -  AZTEC_HAAB
//      /dc=days   -  CHINESE
//      /dd=days   -  DRUID
//      /dt=days   -  TZOLKIN
//      /dv=days   -  Bolivian
//      /dx=days   -  Xiuhpohualli
//      /dy=days   -  MAYAN
//      /dz=days   -  AZTEC Tonalpohualli
//
//
//   Heather has a "greetings" calendar function.  This displays a message
//   at the bottom of the plot area whenever a special event or day occurs.
//   The greetings calendar is read from the file "heather.cal".  If no
//   calendar file is seen, Heather uses an default internal calendar file.
//
//   In addition to displaying a greeting, you can specify a time to sound
//   the alarm on the day of a greeting.
//
//
//   You can disable the greeting calendar with:
//      /gn  - toggles the greetings calendar from the command line
//
//
//   The calendar file should be placed in the same directory as the
//   "heather.cfg" file.  The proper directory is shown when you do a
//   "HEATHER /?" command line or enter ? from the keyboard.
//
//   When a calendar file date is matched to the current date, the message is
//   displayed at the bottom  of the plot window until erased (G G command)
//   or the start of the next day.  Up to 5 calendar match entries can be
//   shown for one day.
//
//   Fields on each line in "heather.cal" are:
//      nth   - event happens on the n'th occurance of DAY in the month
//              (negative values are from the end of the month)
//              (positive values are from the start of the month)
//              (zero means the event occurs on a fixed day number)
//              Values >= 100 are codes for holidays that must be specially
//              calculated.
//
//      day   - The DAY of the month if nth is 0
//              The DAY of the week if n'th is not zero
//              (0=Sunday,  1=Monday,  ...  6=Saturday)
//
//      month - (1=January ... 12=December  0=ignore this entry)
//
//              If DAY < 0 then MONTH is the day-of week code.
//              This allows for doing days like Friday the 13th (0, -13, 5)
//
//
//      text  - The text to display when the event occurs.  If the text begins
//              with "@hh:mm:ss" this will set the alarm to sound at the
//              specified time on the day of the alarm. For example a calendar
//              file enrty of:
//                 0 1 1 @00:00:00 Happy New Year
//              will sound the alarm at midnight on New Year's Day.
//
//              Note that if an alarm is alreay set, the calendar alarm will
//              not override it.
//
//   Comment lines can start with # * / or ; in column 1
//
//
//
//
//
//-AUDIBLE CLOCKS, ALARMS, and SCHEDULED EVENTS
//
//   Lady Heather has several audible clock modes that signal the time via
//   the computer's sound system.
//
//   The singing clock:
//     The singing clock signals the time by playing .WAV files at fixed
//     intervals throughout the hour.  It is activated via:
//        TH #S   - from the keyboard
//        /th=#s  - from the command line
//
//     The number (#) is the number of times per hour to play the files.
//     For instance /th=4s says to play a sound file 4 times per hour (every
//     15 minutes).   The files to play should be placed in the default
//     heather directory are named:
//        "heather_songxx.wav"  where xx indicates the minutes.
//
//     if # is 0 or not given, the singing clock is disabled.
//
//     If the singing clock is activated a double musical note symbol is
//     shown near the upper left hand corner of the screen.
//
//     At 1 second past midnight on New Years, the file "heather_new_year.wav"
//     will be played (if it exists in the current installation directory).
//
//
//   Westminster Chime (Big Ben) mode:
//       TH 4W   - from the keyboard
//      //th=4w  - from the command line
//
//      This mode is very similar to the singing clock, except on the hour
//      the file played is "heather_hourxx.wav"  NOTE THAT THE LADY HEATHER
//      DISTRIBUTION DOES NOT INCLUDE THE NEEDED .wav files.  To implement a
//      proper Big Ben clock, you would need twelve .wav hour files:
//      (heather_hour00.wav through heather_hour11.wav) with the full
//      Westminster chime sequence and gongs for each hour.  You would also
//      need heather_song15.wav, heather_song30.wav, and heather_song45.wav
//      files with the appropriate Westminster chime sequences.
//
//
//   The cuckoo chime clock:
//     The cuckoo chime clock signals the time by playing .WAV files at the
//     hours and at intervals throughout the hour.  It is activated via:
//        TH #H   - from the keyboard
//        /th=#h  - from the command line
//
//     The "heather_chime.wav" file is played x times on the hour (where x is
//     the current hour).  In addition the file is played once (#-1) times
//     during hour.  For instance /th=4h says to play the file x times at the
//     hour and  3 more times times during the hour (every 15 minutes).  The
//     file "heather_chime.wav" should be less than once second long.
//
//     If # is 0 or not given, the chime clock is disabled.
//
//     If the cuckoo chime clock is activated a single musical note symbol is
//     shown near the upper left hand corner of the screen.
//
//
//   The ships bell clock:
//     The ships bell clock signals the time in ships bells format. Ships bells
//     sound on the hour and half hour.  Ships bells mode is activated by:
//       TH 1B   - from the keyboard enables ships bell mode
//        /th=1b - from the command line enables ships bell mode
//     The bell sound is from the file "heather_bell.wav"  This file should be
//     less than two seconds long.
//
//     if 0B is used instead of 1B, the ships bell clock is disabled.
//
//     If the ships bell clock is activated a '#' symbol is shown near
//     the upper left hand corner of the screen.
//
//
//   The tick clock:
//     Entering  "1T" for the clock type enables the ticking clock.
//     This mode sounds a tick on the second.
//
//     Entering  "2T" for the clock type enables the minute clock.
//     This mode sounds a beep on the minute.
//
//     Entering  "3T" for the clock type enables the minute beep / ticking
//     second clock.  This mode sounds a beep on the minute and a tick
//     sound on the second.
//
//     Normally the tick clock sounds when the receiver time message comes
//     in.  By adding a "F" to the command, the "fine tick clock" will be
//     enabled.  This mode attempts to align the ticks to real time by
//     adjusting the ticks for the /tsx (or TO) message delay offset.
//     Note that the fine tick clock may not work well with some "polled"
//     receivers... the Brandywine GPSDO is particularly bad in tick clock
//     mode   ...
//
//     If the file "heather_minute.wav" exists in the heather installation
//     directory exists, this file is played on the minute instead of the
//     beep sound.
//
//     If the file "heather_second.wav" exists in the heather installation
//     directory exists, this file is played on the second instead of the
//     heather_click.wav sound file. If neither file exists then a BEEP
//     is used.
//
//     Entering "0T" for the singing clock type disables the tick/beep clock.
//
//
//
//    Note that the Windows versions of Heather plays sound files via the
//    PlaySound() system call,  the Linux versions use system() to spawn a
//    /bin/sh command to the "aplay" program,  and macOS spawns a /bin/sh
//    command to the "afplay" program.  If your system does not support these
//    sound file player programs,  modify the function "play_tune()"
//    in file heather.cpp
//
//
//  EGG TIMER ALARMS
//
//  Heather supports two types of alarms:
//    An "egg timer" mode where a countdown timer is started and when it
//    reaches 0, the file "heather_alarm.wav" is played. The alarm sound is
//    usually played continuously (and the screen flashes) until a key
//    is pressed which cancels the alarm.  You can set the timer to only
//    play the sound file once by following the time interval with the
//    letter "o".  You can set the timer to automatically repeat by following
//    the time interval with the letter "r"
//
//
//    To start the timer from the keyboard:
//      TA 5S    - sound the alarm continuously after 5 seconds
//      TA 10M   - sound the alarm continuously after 10 minutes
//      TA 3H    - sound the alarm continuously after 3 hours
//      TA 2D    - sound the alarm continuously after 2 days
//      TA 10MO  - sound the alarm once after 10 minutes
//      TA 20MR  - sound the alarm continously every 20 minutes
//      TA 30MOR - sound the alarm once every 30 minutes
//
//    You also set the timer from the command line with the "/na=" command
//    line option:
//      /na=5s  - (see the keyboard commands listed above for the various
//                 timer modes)
//
//    If a countdown timer is enabled and the watch display is enabled, a
//    YELLOW tick mark is shown at the edge of the watch face.
//
//
//
//  ALARM CLOCK MODE:
//    Heather has an alarm clock mode where you specify a time (and optional
//    date) for the alarm to sound. The commands are the same as for the egg
//    timer, but you specify a specific time (and optional date) for the
//    alarm instead of an interval.  Alarm clocks are always a repeating
//    alarm and must be manually canceled unless a date has been specifed.
//    If a date has been specified the alarm setting is cleared once the
//    alarm has sounded and the user stops the alarm (by pressing a key).
//
//    TA 14:15:16   - sound the alarm continuously every day at 14:15:16
//    TA 14:15:16o  - sound the alarm once every day at 14:15:16
//    TA 8:0:12 2016/12/25  - sound the alarm at 8:00:12 on 25 December 2016
//    TA 8:0:12 12/25/2016  - sound the alarm at 8:00:12 on 25 December 2016
//
//    If you set an alarm time that is before the current time, the alarm
//    sounds on the next day.
//
//    You also set the alarm from the command line with the "/na=" command
//    line option:
//      /na=14:15:16
//      /na=14:15:16,2016/12/15 (note non-space char between time and date
//      /na=14:15:16o2016/12/15  is needed on command line alarms)
//
//    Dates can be of the form yyyy/mm/dd, dd/mm/yy, or dd/mm/yyyyy.
//    If only a date is given, the time is assumed to be 00:00:00 on that date.
//
//    If an alarm clock is enabled and the watch display is shown, a
//    RED tick mark is shown at the edge of the watch face.
//
//    Note that if you have one of the astronomical time zones set (such
//    as GMST) alarms are normally based upon your LOCAL time zone time and
//    NOT the displayed astronomical time.  You can force the alarms to be
//    triggered based upon the displayed astronomical time with the "OT"
//    keyboard command. This is the reason you should include your time zone
//    hour offset when using an astronomical time scale. e.g.  /tz=GMST6GMST
//
//
//    Following the setting string with an "A" enables "modem alarms". This
//    uses the modem control signals DTR and RTS to signal when an alarm
//    is sounding.  The modem control signals default to DTR=+12V RTS=-12V.
//    When the alarm is sounding DTR will be -12V and RTS will be +12V.
//    (OK, the actual voltage is dependent upon your serial port driver).
//    For instance:
//       T4SA
//    will use the modem control signals to inform an external device that
//    an alarm or egg time has triggered.
//    YOU SHOULD NOT USE MODEM ALARMS WITH an ACRON ZEIT receiver or if
//    active temperature control is in use... these use the modem control
//    signals for other stuff.
//
//
//
//  SCHEDULED KEYBOARD SCRIPT RUN:
//    Heather has a mode where the keyboard script file "timer.scr" can be
//    automatically run after a specified time interval or at a specified
//    time and/or date.  You specify a time (and optional date) for the
//    timer script to be run. The keyboard script commands are very
//    similar to the egg timer and and alarm clock commands.
//
//    TP 14:15:16            - run the script every day at 14:15:16
//    TP 8:0:12 2016/12/25   - run the script at 8:00:12 on 25 December 2016
//    TP 8:00:12 12/25/2016  - run the script 8:00:12 on 25 December 2016
//    TP 1h                  - run the script in one hour.
//    TP 100mr               - run the script every 100 minutes.
//
//    You also set script time from the command line with the "/np=" command
//    line option:
//      /np=14:15:16
//      /np=14:15:16,2016/12/15 (note non-space char between time and date
//      /np=14:15:16o2016/12/15  is needed on command line alarms)
//
//    Dates can be of the form yyyy/mm/dd, dd/mm/yy, or dd/mm/yyyyy.
//    If only a date is given, the time is assumed to be 00:00:00 on that date.
//
//    Note that if you are typing on the keyboard while the timer script
//    is being run, the script will be interrupted.
//
//
//
//
//  KEYBOARD SCRIPT FILE ALARM WAITS
//    If, in a keyboard or timer script file, you follow an egg timer or alarm
//    clock time with the letter "W",  the processing of the script file will
//    be paused until the alarm triggers.  The command that sets the alarm
//    time MUST be the last command on the line AND the next line in the
//    script file MUST be a blank line or comment!
//
//
//
//  SCHEDULED EXTERNAL PROGRAM EXECUTION:
//    Heather has a mode where an external program can be automatically
//    run after a specified time interval or at a specified  time or date.
//    You specify a time (and optional date) for the program to be run.
//    The scheduled program execution commands are similar to the
//    egg timer and and alarm clock commands.
//
//    TC 14:15:16            - run the program every day at 14:15:16
//    TC 8:0:12 2016/12/25   - run the program at 8:00:12 on 25 December 2016
//    TC 8:00:12 12/25/2016  - run the program 8:00:12 on 25 December 2016
//    TC 1h                  - run the program in one hour.
//    TC 100mr               - run the program every 100 minutes.
//
//    You also set the program execution time from the command line with
//    the "/nc=" command
//    line option:
//      /nc=14:15:16
//      /nc=14:15:16,2016/12/15 (note non-space char between time and date
//      /nc=14:15:16o2016/12/15  is needed on command line alarms)
//
//    Dates can be of the form yyyy/mm/dd, dd/mm/yy, or dd/mm/yyyyy.
//    If only a date is given, the time is assumed to be 00:00:00 on that date.
//
//    The name of the program to run is fetched from the "HEATHER_EXEC"
//    environment variable.  If HEATHER_EXEC has not been defined, the
//    program "heather_exec" is run.
//
//    The /ne command line option disables the file editing and execution
//    commands.  Once /ne is used, it cannot be re-enabled.  This command is
//    a security measure to block possible malicious exploitation of these
//    features.  On Linux/macOS "/ne /ne" also disables playing of sound files
//    vis system() calls to APLAY.
//
//
//
//  AUTOMATIC PROGRAM EXIT
//
//    You can configure Heather to automatically quit at a given time or after
//    a given time interval.  This works just like the egg timer mode or alarm
//    clock mode.  The commands to do this are:
//       TX   - from the keyboard - see TA command for details
//       /nx  - from the command line  - see /na command for details
//
//
//
//  AUTOMATIC LOG FILE DUMPS
//
//    You can configure Heather to automatically write the plot queue data
//    to a log file at a given time or after given time interval.  This works
//    just like the egg timer mode or alarm clock mode.  The commands to do
//    this are:
//       TL   - from the keyboard  - see TA command for details
//       /nl  - from the command line  - see /na command for details
//
//    The format for the scheduled log dump files defaults to ASCII (or XML
//    for the HP-5071A).  You can change this with command line options:
//       /fg  - toggle .GPX format for scheduled log file dumps
//       /fx  - toggle .XML format for scheduled log file dumps
//
//    If the command setting contains the 'o' character
//    the file "tblog.log" is re-written each time a dump
//    occurs.  Without the 'o' character,  the file
//    "tbyyyy-mm-dd-#.log" is written (where #) is an incrementing
//    sequence number.  See the /fg and /fx commands for using .gpx and .xml
//    log formats.
//
//    Example:  to write the log dump to the file "tblog.log" every 30 minutes
//    use the command line option "/nl=30mor" or "TL 30mor" from the keyboard.
//    (the 30m says do it every 30 minutes, the "o" says to do it to
//     one file,  the "r" says to do it on a repeating basis).
//
//    You can change the "tb" prefix using in automatic log dump file names
//    with the /wp=prefix command line option.
//
//
//
//  AUTOMATIC SCREEN IMAGE FILE DUMPS
//
//    You can configure Heather to automatically write a screen dump image
//    to a .GIF file at a given time or after given time interval.  This works
//    just like the egg timer mode or alarm clock mode.  The commands to do
//    this are:
//       TD   - from the keyboard  - see TA command for details
//       /nd  - from the command line - see /na command for details
//
//    If the command setting contains the 'o' character
//    the file "tbdump.gif" is re-written each time a dump
//    occurs.  Without the 'o' character,  the file
//    "tbyyyy-mm-dd-#.gif" is written (where #) is an incrementing
//    sequence number.
//
//    Example:  to write the screen image to the
//    file "tbdump.gif" every 30 minutes use the command
//    line option "/nd=30mor" or "TD 30mor" from the keyboard.
//    (the 30m says do it every 30 minutes, the "o" says to do it to
//     one file,  the "r" says to do it on a repeating basis).
//
//    You can change the "tb" prefix using in automatic screen dump file
//    names with the /wp=prefix command line option.
//
//    Screen dump mode is indicated on the screen by a '!'
//    or !! next to the time mode indicator on the first
//    line of the screen.
//
//
//  If a leap-second is observed (seconds value = 60, duplicated xx:00:00
//  time stamp, or duplicated xx:59:59 time stamp) then Heather automatically
//  does a screen dump to the file "leap_sec.gif"  On some receivers
//  this might capture the previous second (xx:xx:59)... such is
//  life.  When a leap-second screen dump is done, the leap second value
//  in the status column shows "Leap: captured" in YELLOW.  You can clear
//  this indication with the GR keyboard command.
//
//
//  Note that when a screen or log dump happens, Heather first creates the
//  the file "tblock", next the image/log file is written, and then the
//  "tblock" file is deleted.  This can be used by external scripts to
//  minimize (but not totally eliminate) the chances of an external
//  script/program accessing the dump file while it is being written.
//
//
//
//
//
//-SETTING THE SYSTEM TIME FROM GPS
//
//   By far the best way to keep your system time accurate is to use an
//   Internet time protocol like NTP.  This can keep the system clock highly
//   accurate and monotonic (i.e. it does not jump back and forth, it smoothly
//   increments at a rate to keep the system time accurate.
//
//   However, if you do not have an internet connection available, Heather can
//   set the system time from data available from the GPS receiver.  Heather
//   can set the system clock on demand,  every minute, every hour, every day,
//   or whenever the system clock diverges from the GPS time by more than "x"
//   milliseconds.
//
//      TS   - from the keyboard - set the system clock once to the current
//             receiver time.
//      /tso - from the command line - set the system clock once when Heather
//             starts up and has valid time from the GPS receiver.
//      /tsm - from the command line - set the system clock once a minute
//             (the time is set at hh:mm:06 local time)
//      /tsh - from the command line - set the system clock once an hour
//             (the time is set at xx:00:06 local time)
//      /tsd - from the command line - set the system clock once a day
//             (the time is set at 04:05:06 local time)
//      /tsa - from the command line - set the system clock anytime the system
//             clock differs from the GPS time by over 40 milliseconds (Windows)
//             or 10 milliseconds (Linux, macOS)
//      /tsa=msecs - from the command line - set the system clock anytime the
//             system clock differs from the GPS time by over "msecs"
//             milliseconds
//
//   Most operating systems require programs that manipulate the system clock
//   to have administrator / root permissions. You can check if Heather can
//   can set the system clock by issuing a "TS" command from the keyboard.
//   Within a few seconds you should hear a BEEP if the time set command was
//   accepted.
//
//   Heather sets the system clock by "jamming" the time into the system. It
//   makes no attempt to keep the clock monotonically increasing.  This can
//   be a bad thing for some systems, but, well, beggars can't be choosers.
//
//
//   GPS receivers typically send out a message once per second that contains
//   the time.  Heather uses when the last character of this message arrives
//   to set the system clock.  One problem is that different receivers send the
//   the time message at different offsets from the true time.  Also variations
//   in the computer, operating system, receiver configuration, serial port,
//   etc used can affect when the message actually arrives.
//
//   You can set the message offset time (in milliseconds) with the command
//   line option:
//      /tsx=milliseconds  or the TO keyboard command
//
//   Milliseconds can be positive (message arrives AFTER the true time value
//   encoded in the receiver time message) or negative (message arrives BEFORE
//   the true time)  Most receivers have a 1PPS (pulse per second) output
//   signal that indicates true time.   Heather does NOT use the 1PPS signal.
//
//   If you do not set a message offset value, Heather uses a default value
//   for a typical model of that receiver.  For maximum accuracy you
//   you should set the message timing offset adjustment factor for your
//   particular system and receiver configuration.  Heather can help determine
//   the message offset adjustment to use.
//
//   For this to work, your system time must be set accurately.  If you are
//   using a network time protocol like NTP, you should be good to go.
//   Otherwise let Heather set the system time with a "TS" command from the
//   keyboard.
//
//   Then issue the "TK" keyboard command.  This starts measuring the
//   difference between the system clock and the receiver timing message times.
//   It builds a histogram table and also calculates an average value of the
//   message offset time.  For the histogram to work, the system and GPS time
//   need to be within two seconds of each other.
//
//   When measuring the message timing the timing jitter is plotted in the G0
//   plot and the message offset time is shown in the G9 plot.  Also ADEV
//   (Allan variance) tables are shown for the message offset time and jitter.
//   The G0 and G9 plots do not show up as options in the "G" keyboard menu,
//   but are available...
//
//   Let the system run for a minute or so.  Then issue the "TK" command
//   again.  This should print a message at the bottom of the plot area like:
//
//      # msg offset time: /tsx=246.00 msec   max hits:10 points:34   avg:254.35 sdev:10.16
//
//   There are two values of interest in this message:
//      /tsx=246.00   and   avg:254.35
//
//   These values should be close to each other.  The "/tsx" value is the
//   most common value in the histogram and the "avg:" value is the average
//   value seen.  Some receivers do not send the timing message in a consistent
//   manner.  If the "/tsx" value and the "avg:" value are not close or the
//   "SDEV:" (standard deviation) value is large then using the receiver for
//   setting the time may not give optimum performance.  The "TK" measurement
//   mode also writes a ".jit" file with the histogram values and measurement
//   results.
//
//   With some receivers, Lady Heather's periodic polling
//   of the receiver for status info, etc can put spikes in
//   the timing measurements. If this is a problem use the
//   "/ix" command line option to disable the sending of commands
//   to the receiver.  The spikes should not affect the
//   message offset time measurements but can affect the
//   message jitter standard deviation measurements. Issue
//   the "/ix" command again to re-enable status polling.
//
//
//   Tell Heather what your system message offset is with the command line
//   option:
//      /tsx=msecs  - msecs is the value shown the the measurement results
//                    described above.  Or use TO from the keyboard.
//
//   Note that the "TK" timing measurement mode flogs the system CPU rather
//   hard.  You can see CPU usage stats approaching 100% in this mode. This
//   can produce heating that can affect the system clock rate after a while.
//   Ideally you should do the timing measurement test when the system has
//   been running normally (for you) for a while to let it settle down at your
//   typical operating temperature. The difference is probably not much, but
//   in the world of precision timing, every little bit helps.
//
//
//
//
//
//-SETTING THE GPS RECEIVER POSITION and DOING ANTENNA LOCATION SURVEYS
//
//   First a word about ALL the various values that Lady Heather reports...
//   The various values shown by Lady Heather are reported to a decimal
//   point precision represented by the data fields that the receiver
//   messages contain.  Just because your receiver says the altitude is
//   123.4567890 meters does NOT mean the altitude is actually ACCURATE to
//   that many decimal places... Heather just shows what the receiver is
//   sending out...  garbage in, garbage out.
//
//
//   Lady Heather shows the latitude/longitude/altitude position that the
//   receiver is currently reporting.  It can show the position in various
//   formats.  From the keyboard:
//
//      GLD  - decimal format
//      GLS  - degrees/minutes/seconds format
//      GLR  - decimal radians
//      GLG  - decimal grads (400 grads in a circle)
//      GLI  - decimal mils  (6400 mils in a circle)
//      GLE  - ECEF (Earth Centered Earth Fixed) coordinates
//      GLH  - Maidenhead grid square (ham radio)
//      GLU  - UTM (Universal Transverse Mercator)
//      GLN  - NATO MGRS (Military Grid Reference System)
//
//      GLT  - toggle display of solid earth tide displacements and vertical
//             gravity offset (in uGals) instead of the normal lat/lon/alt
//             coordinates.  In GLT mode the lat/lon scattergram (GI keyboard
//             command shows the latitude/longitude earth tide displacement
//             in mm.
//
//      GLF  - show altitude in feet
//      GLM  - show altitude in meters
//      GLL  - show altitude in linguini (7.1429 standard linguini per meter)
//
//      GLP  - private location - latitude and longitude are not displayed.
//             (so you can publish screen shots without revealing the location
//              of your evil mad scientist volcano lair to the world)
//
//      GLA  - toggles autoscaling of the lat/lon/alt scattergram scale factor
//             to match the span of the lat/lon data values seen.  When
//             autoscaling is enabled the "m/div" or "ft/div" value in the
//             scattergram header is shown in UPPERCASE.  Autoscaling the
//             scattergram can be useful if you are using the receiver in a
//             moving environment.
//
//             If you are in scattergram auto-scale mode and move the mouse
//             pointer into the scattergram area, the lat/lon location of the
//             mouse pointer will be shown below the scattergram.
//
//             WARNING: LLA autoscaling is currently experimental can can be
//             a bit flakey.  Before enabling autoscale mode make sure all
//             the lat/lon data in the plot queue is valid (i.e. no bogus or
//             0.0,0.0 lat/lon values due to receiver startup).  It's best
//             to clear the plot queue after the receiver starts producing
//             valid lat/lon values and before enabling autoscale mode.
//
//      /gld, etc - set the location format from the command line.  The
//                  command line option to use is the same as the keyboard
//                  commands listed above with a leading "/" or "-".
//
//      /t"  - toggle between degrees.minutes.seconds / decimal location format
//             (this is a legacy command for compatibility with earlier
//              version of Lady Heather).
//
//
//   Heather also records the lat / lon / alt data in the plot queue and
//   can display the location data in the plot area or as an X-Y "scattergram"
//   of distance from a reference point.
//
//     G1  - from the keyboard - toggle the latitude plot on and off
//     G2  - from the keyboard - toggle the longitude plot on and off
//     G3  - from the keyboard - toggle the altitude plot on and off
//     GV  - from the keyboard - toggle the lat/lon/alt (G1 / G2 /G3) plots
//           on and off
//
//     For the Zyfer Nanosync receiver the G1/G2/G3 plots show some
//     undocumented information. These are HETE/HEFE/HEST.  These are thought
//     to be holdover estimated time error, holdover estimated frequency
//     error, and holdover estimated 24 hour error... but who knows
//
//     The G1/G2/G3 plots are also used to display other values for the
//     HP-5071A and TAPR TICC devices since these devices don't have
//     lat/lon/alt related values.
//
//
//   THE LOCATION FIX SCATTERGRAM
//
//     Heather can plot a "scattergram" of the location fix data relative to
//     a fixed reference point.  The reference point is the location  that
//     was active when the scattergram was activated.  The scattergram is
//     automatically enabled if a position survey is started.  Every hour the
//     color of the dots in the scattergram changes (14 colors are used).
//
//     You can change the scattergram reference point with the SR keyboard
//     command.  The SR command defaults to using the current lat/lon/alt.
//     If the lla scattergram reference has been manually set the lat/lon/alt
//     labels are shown in all upper case. They are normally in upper and
//     lower case.  Toggling the scattergram off and on cancels the user
//     set reference position.
//
//     If you use the letter 'A' for a location value then the current
//     average of the lat/lon/alt value will be used.  The average is
//     calculated from the data displayed in the plot area.  To use the
//     average of all data in the plot queue, first use the VA command to
//     display all plot data and "FD -1" to do a per-pixel average.
//
//     If you use '*' as a location value then the current value of the
//     lat/lon/alt value will be used.
//
//     If you specify a sign of '++' or '--' before a lat/lon/alt
//     value the current location of that value will be moved by that
//     number of meters (or feet if in /t' mode).
//
//     The scattergram grid defaults to a resolution 3 meters or 10 feet
//     per division (with +/- 5 divisions from the center reference point.
//     You can change the scattergram grid scale factor with a command
//     line option:
//        /tm=meters_per_division
//        /t'=feet_per_division
//        /tlg=linguini_per_division
//
//     You can also set the scattergram to auto-scale:
//     GLA  - toggles autoscaling of the lat/lon/alt scattergram scale factor
//            to match the span of the lat/lon data values seen.  When
//            autoscaling is enabled the "m/div" or "ft/div" value in the
//            scattergram header is shown in UPPERCASE.  Autoscaling of the
//            scattergram can be useful if you are using the receiver in a
//            moving environment.
//     /gla - does GLA from the command line
//
//     GI   - toggle the lat/lon "scattergram" on and off from the keyboard.
//     /gi  - toggle the lat/lon "scattergram" on and off from the command
//            line.
//     ZL   - zoom the scattergram to full screen
//     SR   - change the scattergram reference position
//
//            In earth tide display mode (GLT keyboard command) the lat/lon
//            scattergram shows the latitude/longitude earth tide displacement
//            in mm. The scale is fixed at 12.5 mm per division.  This display
//            is not available with the HP5071 cesium beam oscillator.
//
//   Clicking the left mouse button on the lat/lon/alt information display
//   of the scattergram will zoom the lat/lon scattergram to full screen.
//   Clicking again will restore the normal screen display,
//   Note that short clicks might be ignored... particularly on devices
//   like UCCM receivers that only output time updates every two seconds.
//   It can help to hold down the mouse button until the screen changes.
//
//
//
//   ANTENNA LOCATION SURVEYS
//
//     Most GPS timing receivers and disciplined oscillators require a fixed
//     location for the antenna and its position must be known to very high
//     accuracy for optimum operations.  These devices can usually operate
//     in a "position hold" mode or a "3D fix / navigation" mode.  Position
//     hold mode is also called "overdetermined clock" mode.
//
//     The accurate fixed location can be input manually by the user or the
//     receiver can automatically determine it by doing a "self-survey" where
//     it collects data for a period of time and averages/filters the readings
//     to come up with an accurate location.  Most receiver self-surveys last
//     for at least 20 minutes and may take hours.  Some devices let you
//     specify the time interval to survey for.
//
//     Lady Heather can also do a precision median survey to determine the
//     location.  This collects data for several hours (48 hours is the default)
//     and statisically processed the data using medians and averages over
//     several time intervals to come up with a precise location that is usually
//     better than the standard receiver position survey.  The GPS satellite
//     orbits repeat on a roughly 12 hour basis (with 6 hours of visibility
//     for each orbit).  A 48 hour survey allows for 4 repeats of the complete
//     satellite constellation and allows the precision survey to mitigate
//     a lot of signal problems such as multi-path distortion.  If you cannot
//     wait for 48 hours,  24 hours works well, and 12 hours is OK.
//
//     If a precision survey is done, Heather logs the readings and results in
//     a ".lla" file. Precision surveys can be done on receivers that do not
//     support a "position hold" mode.  After the survey completes you can
//     check the ".lla" file for the results.  The calculated position will be
//     shown at the end of the file.  Previous versions of Heather always
//     wrote the file "lla.lla".  Heather now uses ????.lla where ???? is
//     based upon the receiver type.  To avoid lla log file name conflicts
//     you should not run more than one precision survey at a time if the
//     receiver types are the same.
//
//     Trimble Thunderbolt receivers do not have a command for entering the
//     position using double precision numbers.  Heather tries to write the
//     calculated precision location or the manually set location to the
//     device by doing single point self-surveys until one is within a foot
//     of the desired location.  This process takes an indeterminate amount
//     of time!
//
//     Interrupting a standard survey or precise position survey save
//     will save the current location using the lower precision
//     TSIP command.  So will exiting the program while a survey is
//     in progress.
//
//     Some GPSDO devices (mainly telecom surplus devices) do not have a
//     "navigation" mode that lets them output 3D fixes. You cannot do a
//     precision survey on these devices.
//
//
//     To enter a location manually and set position hold mode:
//        SL   - from the keyboard.  The latitude and longitude can be in
//               decimal or degrees/minutes/seconds format like 30d40m50.60s
//               Wherever Heather expects a lat/lon/alt string you can specify
//               a value of "loc" and the location will be read from the file
//               "heather.loc" (if it exists).
//
//     The SL command can also be used with devices that do not output
//     lat/lon/alt info. Entering a location for these devices allows the
//     astronomical / sun / moon features of Heather to work.
//
//
//     The command that Trimble receivers use to set a position only uses
//     single precision floating point numbers.  These are not accurate enough
//     to resolve lat/lon to meter level precision.  The ST command can be
//     used to save a more accurate position by doing multiple single point
//     self surveys until one happens to land within 1 meter of the desired
//     location.  This process can take an indeterminate amount of time
//     (incuding forever!).  Note that the ST command does not appear in
//     the "S" keyboard menu.  To figure out which method of saving a position
//     on a Trimble receiver works best for your situation try:
//        1) set the precise location with the SL command
//        2) calculate the distance between the saved location and the entered
//           location with the SD commad (SD precice_lat precise_lon 0.0)
//        3) write down the calculated distance
//        4) do a precice location save of the precice location:
//           ST precise_lat precise_lon precise_alt
//        5) wait for the precice location save to end
//        6) calculate the distance between the new saved location and the
//           entered location with the SD command:
//               SD precice_lat precise_lon 0.0
//        7) if the new reported distance is greater than the old distance
//           from step 3, do step 1.
//
//     Heather can also get the location automatically from the file
//     "heather.loc" in the Heather installation directory.  This file is
//     only read if the main input device does not report a location (i.e.
//     is not a GPS based device) or if you specify a value of "loc" when
//     Heather requests a lat/lon/alt location value.
//
//
//     If you use the letter 'A' for a location value then the current
//     average of the lat/lon/alt value will be used.  The average is
//     calculated from the data displayed in the plot area.  To use the
//     average of all data in the plot queue, first use the VA command to
//     display all plot data and "FD -1" to do a per-pixel average.
//
//     If you use '*' as a location value then the current
//     value of the lat/lon/alt value will be used.
//
//     If you specify a sign of '++' or '--' before a lat/lon/alt
//     value the current location of that value will be moved by
//     that number of meters (or feet if in /t' mode).  Note that moving
//     the lat/lon scattergram reference point is not available if
//     scattergram auto-scaling is enabled (GLA command).
//
//
//     To start a native receiver self survey:
//        SS   - from the keyboard.  It will request a value for the length
//               of the survey.  Depending upon the receiver type the length
//               will be in samples, seconds, minutes, or hours.  A few
//               receivers can only perform a fixed length survey.  If a survey
//               is in progress, issuing SS again tries to stop the survey.
//        /ss[=length] - from the command line.  If a length is not specified
//               the survey length is device dependent (maybe 20 minutes or 1-3
//               hours).  If a survey is in progress and Heather is started with
//               /ss=0, Heather attempts to stop the survey.
//
//     To start a precision survey:
//        /sp[=hours]  - from the command line - do a precision survey,
//        SP   - from the keyboard.  It will request the length of the survey
//               in hours (up to 96 hours long)... the longer the better.  If a
//               survey is in progress, issuing SP again will stop the survey.
//
//
//
//     To force position hold mode:
//        SH   - from the keyboard - Some receivers remember their set position
//               hold locations and use that when position hold mode is
//               selected Other receivers will use the current position as
//               the hold position.
//
//     To force 3D navigation mode mode:
//        SN   - from the keyboard
//
//     Trimble Thunderbolt and related devices support several other positioning
//     modes.  See the device manual for more information (0xBB packet).
//
//        S0   - automatic 2D/3D navigation mode selection
//        S1   - single satellite mode
//        S2   - 2D mode (S2 actually sets receiver mode 3)
//        S3   - 3D mode (S3 actually sets receiver mode 4)
//        S4   - undocumented (3D?) mode (S4 actually sets receiver mode 2)
//        S5   - DGPS reference mode (disables timing functionality)
//        S6   - 2D clock hold mode
//        S7   - overdetermined clock mode (same as SH)
//
//
//
//   PROPOGATION TIME ESTIMATES (also distance to remote location)
//
//     Heather can calculate a (crude) estimate of the signal delay
//     (propagation) time between your location and a distant transmitter
//     using the SD command.
//
//        SD lat lon ionosphere
//        SD lat lon
//        SD id ionosphere
//        SD id
//        SD km ionosphere
//        SD km
//
//     lat and lon are the transmitter location,  ionosphere is the
//     height of the ionosphere in km.  If ionosphere is not given,
//     it is estimated based upon your hemisphere and the time of year.
//     Negative lat values are in the southern hemisphere, negative lon
//     values are the western hemisphere.
//
//     Or you can specify a distance in km by following the first number
//     with the letter 'k'.
//        SD 123k 100  (123 km to station, 100 km ionosphere height)
//        SD 300000k   (300,000 km to station , calculated ionosphere height)
//
//     You can also use the SD command to calculate the lat/lon great circle
//     distance to a location (ignoring the altiude difference).  Normally
//     distance is shown in km/miles.  If the distance is less than 1 km, it
//     will be shown in meters/feet.  This can be useful if you want to
//     calculate the distance between the GPS receiver reading (like the
//     position hold location) to a post-processed precise location.  To use
//     the SD command to calculate a great circle distance difference, specify
//     an ionosphere height of 0.0 meters.  Note that there can be a 1mm
//     residual offset in the calculated distance.
//
//
//     Instead of specifying the transmitter lat and lon, Heather has a list
//     of known station ID's you can use:
//        ANTHORN
//        BBC  (Droitwich)
//        BPM
//        CHU
//        DCF
//        EBC
//        HBG
//        HLA
//        JJY
//        KYUSHU
//        LOL
//        MIKES
//        MSF
//        PPE
//        RBU
//        RTZ
//        RUGBY
//        RWM
//        TDF
//        WWV
//        WWVB
//        WWVH
//        YVTO
//
//    To clear the propagation time results from the screen use the GR command.
//
//
//
//
//   INCLUDING and EXCLUDING SATELLITES
//
//     Several receivers let the user exclude certain satellites from being
//     tracked,  force or restore the inclusion of satellites, or operating
//     the receiver in a single satellite mode.
//
//     SO   - from the keyboard controls single satellite mode.  Trimble
//            receivers can be configured to track a specific satellite or
//            automatically select the highest elevation satellite.  For
//            other receivers Heather offers to track the highest satellite
//            or the user can select a particular satellite... eventually
//            that satellite will fall below the horizon and the receiver
//            won't be tracking anything...  Non-timing receivers are not able
//            to work in a single-satellite mode.
//
//     SX   - from the keyboard controls excluding/including of satellites.
//            (currently only one satellite may be excluded)
//
//
//
//   CONFIGURING THE SATELLITE CONSTELLATION TYPES TO USE
//
//     Many modern GPS receiver are actually GNSS receivers... they can work
//     with more than one satellite system: GPS/GLONASS/BEIDOU/GALILEO/SBAS/
//     QZSS/IMES, etc.  Heather lets you control which satellite systems to
//     track.  Note that most GNSS receivers have limits on which systems can
//     be enabled at the same time. Consult your receiver manual for
//     information on what it can do.
//
//
//     SG   - from the keyboard controls the GNSS satellite system
//            congiguration.  You will be prompted to input a string of
//            characters that indicate which systems to use.  The currently
//            enabled systems will be offered as the default entry.
//
//            The letters that are used to select a particular satellite
//            system were changed in v6.04 to match the international
//            standard ones used in RINEX files.
//
//
//
//
//
//-THE SATELLITE INFORMATION DISPLAYS
//
//   Heather has three main satellite information displays:
//      1) The satellite information table
//      2) The satellite position map
//      3) The satellite signal maps
//
//   The satellite information table lists the currently visible and tracked
//   tracked satellites and various information about them:
//      Satellite PRN and GNSS id char (that identifies the sat and GNSS system)
//      Satellite azimuth angle
//      Satellite elevation angle
//      Satellite signal level
//
//   GNSS system id chars (next to the PRN columns) are:
//      G - GPS     (USA)
//      S - SBAS    (USA)
//      R - GLONASS (Russia)
//      E - Galileo (Europe)
//      C - BEIDOU  (China)
//      J - QZSS    (Japan)
//      I - IRNSS   (India)
//      M - IMES    (IMES is not a RINEX supported system)
//
//   Depending upon the receiver type, the information table can also show
//   extra things like:  doppler shift, tracking state, carrier phase, code
//   phase, clock bias, URA (range accuracy).
//
//   A normal GPS receiver may be able to track around 14 satellites at once.
//   More modern receivers that can also track GLONASS, GALILEO, BEIDOU, SBAS
//   and other satellites might be tracking over 32 satellites at once.
//   Normally Heather expects to show up to around 14 satellites, and
//   displaying 32 satellites at once can cause problems fitting all that data
//   on the screen.  You can control the size and format of the satellite
//   information table display:
//
//       SI    - from the keyboard
//       /si=# - from the command line.
//
//   Clicking the left mouse button on the satellite information display
//   will zoom the sat info information table to full screen.
//   Clicking again will restore the normal screen display,
//   Note that short clicks might be ignored... particularly on devices
//   like UCCM receivers that only output time updates every two seconds.
//   It can help to hold down the mouse button until the screen changes.
//
//
//   If you don't use the SI commands to specify the maximum number of
//   satellites to show information for, Heather will show up to 14 satellites.
//   The satellite information display size automatically adjusts as more
//   satellites are seen.  If you set a value, Heather forces the display size
//   to that value.
//
//   If you input a number without a +/- sign, Heather displays all the
//   information it has for up to the specified number of satellites.  Note
//   that the +/- sign can be used either before or after the number.
//
//   If you input a negative number,  Heather shows the short form of the
//   info for up to that number of satellites.  The short form display frees
//   up enough space to the right of the satellite info display to show
//   the sun / moon information display.  If the receiver type in use does
//   not report any extended information,  the sun/moon info display is
//   automatically enabled.
//
//   The sun / moon info is shown in yellow if there is a possible eclipse
//   condition (currently only works for solar eclipses).
//
//   If you input a number with a "+" sign, Heather shows the short form
//   information in two columns (but does this not leave room for the
//   sun/moon information).  This helps with displaying information for
//   receivers that can track multiple satellite systems.
//
//   If you follow the number with a "t", only satellites that are being
//   actively TRACKED are shown in the satellite information table and
//   satellite map display.  The GCT keyboard command also lets you toggle
//   the display of un-tracked satellites.
//
//
//   The more rows of satellite information being displayed, the less space
//   there is for the plot area.  Combine lots of rows of satellite info
//   with the digital clock display on a small screen and you can wind up with
//   an unusable plot display or unreadable keyboard menus.  Turning off the
//   digital clock display (GZ) can help with the screen formatting.
//
//   If more satellites are visible that you have allocated space for, the
//   info for the excess satellites is not shown and a message is shown
//   saying to click on the satellite information table.  If you click on the
//   satellite table the screen is zoomed to show all satellite information.
//
//   Satellites that are being actively tracked are shown in GREEN
//   Satellites that are visible, but not being tracked are YELLOW
//   Excluded/disabled/blocked satellites are shown in BLUE
//   Unhealthy satellites are shown in RED (if the receiver reports satellite
//   health)
//
//
//   Heather graphs the number of actively tracked satellites in the plot
//   window and can flag whenever the sats being used for calculating
//   fixes changes:
//
//      GCG  - Enables the tracked satellite count plot menu (same as G$)
//             The sat count plot does not scale or auto-center. The satellite
//             count plot is always at the bottom of the plot area.
//             The satellite count plot is shown in CYAN at the bottom of
//             the plot window.  Each minor division equals one satellite.
//             For screen resolutions less than 800x600, each minor division
//             represents two satellites.
//
//      GCC  - toggles the display of changes in the actively USED
//             satellite constellation.  When the list of satellites being
//             actively tracked and USED FOR FIXES changes a small tick
//             mark is shown shown at the bottom of the plot area.  The
//             actively used satellites are the ones shown in GREEN in
//             the satellite list.  See the section on the FD (filter display)
//             command for information about how setting the plot view
//             interval to show more than one reading per pixel can mask
//             seeing all the satellite changes.
//
//
//   You can sort the satellite information table using the following
//   keyboard commands.  You can sort in descending order by including
//   "-" character.  Ascending sort is the default for most plots.
//   If you sort by one of the values, the header of that column is shown
//   in GREEN.  If you don't specify a sort, the table is sorted by PRN
//   number in ascending order.  Note that if a value is 0, this usually
//   means that the value is unavailable or invalid.  These values always
//   appear at the end of the satellite list.  If you attempt to sort on
//   a value that the receiver does not output, the list winds up being
//   sorted by PRN.
//
//      GCA - sort by azimuth
//      GCB - sort by clock bias
//      GCD - sort by doppler
//      GCE - sort by elevation
//      GCP - sort by PRN (default)
//      GCR - sort by pseudorange
//      GCS - sort by signal level
//      GCW - sort by carrier/code phase
//
//      GCT - toggle display of non-tracked satellites
//
//
//   The SQ keyboard command allows plotting the az/el/signal level of a
//   specified satellite, the sun, or the moon.  SQ mode is not available
//   if an environmental sensor is being used... it uses the same plot queue
//   entries.  This also means that the earth tide plots are not available
//   in SQ mode.  The SQ command also resets all the data queues whenever it
//   is executed.  The GK keyboard command is used to access the sat prn
//   plots.  For the sun and moon, the signal level is repoted as -40 or 40
//   depending upon if it as below or above the horizon.  The satellite that
//   is being plotted is identified by an '*' next to its PRN value in the
//   satellite information table.
//
//
//   The SB command causes a particular satellite in the satellite map to
//   blink once a second.
//      SB # - user specified PRN #
//      SB H - current highest satellite
//      SB P - the plot satellite specified by the SQ command
//
//
//   Besides the satellite information table, Heather has two other satellite
//   information displays... the satellite map and the signal level map:
//
//
//   THE SATELLITE POSITION MAP:
//
//   The satellite map shows a map of the currently visible / tracked
//   satellites. It is enabled by:
//       GM   - from the keyboard
//       /gm  - from the command line
//       ZM   - zoom the satellite map to full-screen
//       ZB   - zoom the satellite map and watch to full-screen
//
//   Clicking on the map or watch displays on the screen will zoom it to full
//   screen.  Clicking again will restore the screen to its previous state.
//   Note that short clicks might be ignored... particularly on devices
//   like UCCM receivers that only output time updates every two seconds.
//   It can help to hold down the mouse button until the screen changes.
//
//   The satellite map shows the satellites as circles on an azimuth/elevation
//   grid.  The size of the circles depends upon the current signal level.
//   Satellites that are currently being tracked are shown as filled in
//   colored circles.  Satellites that are visible are shown as hollow circles.
//
//   The /so=# command line option lets you change the shape of the satellites
//   in the map:
//      /so=0 - round sats
//      /so=1 - square sats
//      /so=2 - round sats with "bat-wings"
//      /so=3 - rectangular sats with "bat-wings"
//      /so=4 - rectangular sats with "linear wings"
//      /so=5 - rectangular sats with "linear wings"
//
//   If you hold the mouse cursor over a satellite circle on a map display
//   that satellites' PRN, azimuth, elevation, and signal level will be
//   shown below the map... if there is enough room on the screen for it (i.e.
//   the map has the "Satellite positions" label below it).
//
//   The position history of each satellite is shown as a "trail" behind the
//   satellite.  The trail has small time marker circles drawn on it.  Solid
//   circles indicate hours and hollow circles indicate half hours. The trail
//   shows the last 6 hours of the satellite position.  The drawing of satellite
//   position trails can be toggled with:
//      /st  - from the command line - toggle display of satellite trails
//      /sd  - from the command line - toggle display of time markers on the
//             satellite trails
//
//
//   The satellite position map includes a representation of the sun at its
//   current location in the sky.  The sun is shown in solid YELLOW if it is
//   visible and as a hollow YELLOW circle if it is below the horizon. The sun
//   symbol is shown surrounded by sun rays in order to distinguish it from
//   satellites.  The moon is also drawn showing its current phase.  It is
//   shown in YELLOW if above the horizon and GREY if below the horizon.
//
//
//
//   If both the satellite position map and the watch display are enabled they
//   will overlay each other on the screen if there are not two places
//   available to draw them (the GB and /gb commands allow one of the displays
//   to be drawn to the right of the plot area).
//
//   The GB command is labeled "Both map and adev tables" in the keyboard menu
//   but is more properly "allow map displays in the plot area."
//
//
//
//   Note that on SCPI, UCCM, STAR, and ACRON receivers the message that sends
//   the satellite position and signal data takes a long time for the receiver
//   to send.  Heather only requests it once per minute at hh:mm:33.  These
//   satellite information messages block the sending of time code messages
//   and causes time code skip errors... Heather automatically ignores any
//   time code errors caused by these messages.  Heather has some code to
//   "fake" the missing time messages on most receivers.
//
//
//   Note that the RFTG-m XO unit does not report satellite positions. Heather
//   will fake the satellite positions at AZ=(prn*10) and EL=20 degrees.  This
//   hack lets you see what sats are being tracked and their signal levels,
//   but not their true positions... such is life.
//
//
//
//   THE SIGNAL LEVEL MAPS:
//
//   The signal level maps show a map of the satellite signal levels as a
//   function of satellite azimuth/elevation.  The values used are the AVERAGE
//   value of the signal level of all satellites seen a given position in
//   the sky.  The signal level map can be shown in several different formats.
//
//   signal level map   - shows a color coded display of the average signal
//                        level seen across the sky.  Data is interpolated
//                        to fill in spaces between observed positions.
//
//   data               - this is similar to the signal level map, but the map
//                        is not interpolated between missing points.
//
//   azimuth            - shows a map of the relative signal level seen at
//                        each azimuth angle.
//
//   elevation weighted - shows a map of the relative signal level seen at
//                        each azimuth angle weighted by 1.0/elevation.
//                        Weighting the signal levels by the inverse of the
//                        satellite elevation compensates for the normal
//                        behavior of satellite signals being better at
//                        higher angles and better shows the effects of
//                        antenna obstructions.
//
//   elevation          - shows a map of the relative signal level seen at
//                        each elevation angle.  Note that a YELLOW tick mark
//                        appears at the edge of this map.  This is the
//                        elevation angle where satellite signal levels begin
//                        to rapidly drop off.  A BLUE tick mark indicates the
//                        satellite elevation mask setting.
//
//   From the keyboard:
//       GQ   - toggle signal level map on and off
//       SAS  - toggle signal level map on and off
//       SAW  - toggle signal level elevation weighted data map on and off
//       SAE  - toggle signal vs elevation map on and off
//       SAA  - toggle signal level vs azimuth map on and off
//       SAD  - toggle signal level data map on and off
//
//   From the command line
//       /gq  - toggle signal level map on and off
//       /gqs - toggle signal level map on and off
//       /gqq - toggle signal level map on and off
//       /gqw - toggle signal elevation weighted level map on and off
//       /gqa - toggle signal level vs azimuth map on and off
//       /gqe - toggle signal vs elevation map on and off
//       /gqd - toggle signal level data map on and off
//
//    SAA - displays relative signal level seen at each
//          azimuth angle
//    SAW - displays relative signal level seen at each
//          azimuth angle,  with the signal strength
//          weighted inversely with elevation angle.  This
//          highlights low elevation antenna blockages and
//          low orbit angles that have no satellites.
//    SAE - displays relative signal level seen at each
//          elevation angle
//    SAS - displays color coded map of the absolute signal
//          level seen at each azimuth/elevation point.
//    SAD - shows satellite signal level data points
//          (much like the GM satellite position map)
//    SAC - clears the old signal level data and starts
//          collecting new data. You can also do this with
//          the CM keyboard command
//
//
//   You can zoom the various satellite maps to full screen from the "Z"
//   keyboard menu.  Any keypress (except '\') will exit the zoom screen mode.
//   A '\' will dump the screen image to a .gif file.
//
//   You can also force a screen dump by left-clicking on the screen where
//   the "Receiver data" info is near the upper left hand corner of the
//   screen.  This is handy if you are using a touch-screen.
//
//   Note that it can take several hours for the map to fill in.  The more
//   satellites that your receiver can track, the faster the map fills in.
//
//   Note that the satellite position and signal level maps are affected
//   by the settings of the elevation mask and signal level mask filters.
//   If you want a complete view  of your antenna system performance then
//   first set those filters to their lowest settings.
//
//   The signal level maps show the current setting of the satellite elevation
//   mask filter with a dotted circle at the elevation mask angle.  You can
//   disable the display of the elevation mask angle circle:
//      GJ  - from the keyboard
//      /gj - from the command line
//
//
//   You can reset the satellite map and signal data with the keyboard command:
//       CM   - clear out the satellite map data
//
//   You can write the current signal level information to a ".sig" file.
//   The signal level file contains some comments and a list of az/el/snr
//   values.
//       WZ   - writes the signal level data to a file.
//
//   You can read a ".sig" file from the "R" keyboard command. Note that
//   reading a ".sig" file pauses updating the plot queue with new values.
//   You can resume plot queue updates with the "U" keyboard command.
//
//
//
//
//-ZOOMED SCREEN DISPLAYS:
//
//      ZA - zoom the elevation weighted Azimuth map to full screen
//
//      ZB - zoom Both the watch and satellite map displays to full screen
//           overlayed on each other.
//
//      ZC - zoom the Clock display to full screen.
//
//      ZD - zoom the signal level Data map to full screen.
//           If one of the signal quality maps is shown you can click the
//           mouse to show all the signal quality maps (ZU command)
//
//           For the HP-5071A, LPFRS, RMO, Lucent RFTG-m, SRO100, PRS-10, and
//           X72 devices, ZD zooms the screen to a full device Data display.
//
//      ZE - zoom the signal level vs Elevation map to full screen
//
//      ZH - brings up a receiver data monitor screen. Received message data
//           is formatted into hex+ASCII lines.  Binary data packets are
//           labeled with their name (where possible).
//
//      ZI - zoom the satellite Information displays to full screen
//
//      ZL - zoom the position fix scattergram to full screen
//
//      ZM - zoom the satellite Map to full-screen
//
//      ZN - zoom the screen do a Null (blank) display
//
//      ZO - brings up a data mOnitor screen. Received message data bytes
//           are shown in GREEN and sent data is shown in YELLOW.
//
//      ZP - zoom the Plot window to full screen
//
//      ZQ - zoom the screen to a calendar display.  The calendar shows a few
//           previous month(s) and the next "x" months.  The current month is
//           shown in GREEN.  Months before the requested staring data are
//           shown in GREY, and later months are shown in WHITE. "x" is
//           determined by what will fit on the srceen.
//
//           Clicking on the lower left corner of the calendar screen or
//           pressing the '-' key moves the calendar back a month.
//           Pressing the '<' key moves the calendar back a year.
//
//           Clicking on the lower right corner of the calendar screen or
//           pressing the '+' key moves the calendar forward a month.
//           Pressing the '>' key moves the calendar forward a year.
//
//           You can preceed the +,-,<,> commands with a number from 0..10000.
//           (note that there will not be any indication of you typing the
//           adjustment number).  The adjustment number gets cleared when
//           you execute the adjustment.
//
//           The current day will blink every second.
//           Days of daylight savings time switchover are shown in BLUE.
//           Days that have full/blue moons are shown  in YELLOW.
//           Days that have new/black moons are shown in BROWN.
//             The moon phase dates are calculated with a resolution
//             of 1-2 hours.  If the new/full moon appears around
//             midnight local time, the date shown in the calendar may be off
//             by a day.  The moon phase calendar information may not be
//             accurate outside of the years 2000..2100
//           Days that have an entry in the greetings calendar are shown
//           in CYAN.
//
//           The TB keyboard command lets you show a calendar for a specifed
//           year (and optional starting month).
//
//      ZR - zoom the Relative signal level map to full screen
//
//      ZS - zoom the Signal quality map to full screen
//
//      ZU - zoom all signal level maps to full screen
//
//      ZV - zoom the watch, sat map, signals full screen
//           If the ZU or ZV screen is showing you can click the mouse
//           on one of the displayed items to zoom it to full screen.
//
//      ZW - zoom the Watch display to full screen.
//
//      ZX - zoom the watch, satellite, and signal quality maps to full screen
//           overlayed on each other
//
//      ZY - zoom the satellite map and signal quality map to full screen
//           overlayed on each other
//
//      ZZ - rerurn the screen to the normal un-Zoomed state
//
//      ZT - sets a keyboard inactivity Timeout (in minutes).  After the
//           specified time has passed without any keyboard activity, the
//           screen switches to the specified zoomed screen display.  This
//           will not happen if the screen is aready showing a zoomed screen
//           or a keyboard menu is being shown.  The default timeout display
//           is the digital clock.
//
//
//
//
//-SUN and MOON INFORMATION DISPLAY
//
//   Besides displaying information about the GPS satellites, Lady Heather
//   calculates the position of and other relevant information about the sun
//   and moon:
//
//     Sun azimuth and elevation in the sky
//     Moon azimuth and elevation and phase (illumination percentage)
//     Sun rise/transit/set times
//     Moon rise/transit/set times
//     Current solar equation of time.
//     Current moon "age" from the start of the lunar month (new moon).
//
//   The sun/moon information table is shown to the right of the satellite
//   information table.  Enabling the sun/moon information automatically
//   selects the short form of the satellite information display.
//
//   The sun rise and set times are based upon the sun's position above the
//   horizon. There are several definitions of sun rise/set:
//      Physical - when the top edge of the sun crosses the horizon
//      Official - when the sun crosses 0.833 degrees below the horizon
//                 (this angle is for atmospheric refraction effects)
//      Civil    - when the sun crosses 6 degrees below the horizon
//      Nautical - when the sun crosses 12 degrees below the horizon
//      Astronomical - when the sun crosses 18 degrees below the horizon
//      Islamic  - when the sun crosses 18 degrees below the horizon at sunrise
//                 or 17 degrees below the horizon at sunset
//      User defined - you can specify any angle above (positive) or
//                     below (negative) the horizon.
//
//   Sun/moon times are shown as local time zone times for the current date.
//
//
//   To display the sun (or moon) rise/set times use:
//      TR    - from the keyboard
//
//      TR P  - display "Physical" sunrise/sunset times
//      TR O  - display "Official" sunrise/sunset times
//      TR C  - display "Civil" sunrise/sunset times
//      TR N  - display "Nautical" sunrise/sunset times
//      TR A  - display "Astronomical" sunrise/sunset times
//      TR I  - display "Islamic" sunrise/sunset times
//      TR #  - (where # is the user defined solar horizon angle[s]
//              negative values are for angles below the horizon, positive
//              values are for angles above the horizon)
//              You can specify independent solar rise/set horizons like:
//                 TR -18,-17
//
//      TR M  - display moon rise/transit/set times instead of the sun times.
//
//      /sr=#  - from the command line where # is the sunrise type selection
//               character shown above.  For example
//                  /sr=c says to show civil sunrise/sunset times.
//
//   Heather can also play sound files at rise/set or transit (solar noon):
//      Include an "*" after the command:
//         TR O*
//         /sr=o*
//         /sr=o    - disables sunrise/transit/set sound
//
//   Heather plays the file "heather_sunrise.wav" at sunrise/sunset.
//   Heather plays the file "heather_noon.wav" at solar noon or moon transit.
//
//   If the sun/moon files are enabled a double musical note appears next to
//   the rise/noon times.  Otherwise, the horizon type letter is used.  The
//   horizon type always appears next to the set time.
//
//   The standard rise/set sound is a rooster crowing and the noon sound is
//   a church bell.
//
//   If moon rise/set times are selected (TRM) the sound files are played at
//   moon rise/transit/set and not sun rise/noon/set.  They are the same files
//   used for sun times... there are not any separate moon files.
//
//
//   Normally Heather assumes that the GPS receiver is at a fixed position
//   and the sun/moon rise/set times are only recalculated when Heather
//   starts up or once per hour at xx:00:26 local time.  If you are
//   working with a moving receiver you can cause Heather to continuously
//   recalculate the times:
//      Include an "!" after the command.
//         TR C!
//         /sr=c!
//
//   You can use both "*" and "!" on the same command.
//
//
//
//
//
//-THE PLOT WINDOW
//
//   The receiver data gathered by Heather and stored in the plot queue
//   is shown in the plot window at the bottom of the screen.  The data
//   that is stored and shown is dependent upon the receiver type.
//
//   The plot area is divided into a header area at the top and a plot
//   grid below it.  The header shows the various plot scale factors and
//   reference (center line) values along with a statistical value for each
//   plot.
//
//   Clicking on the plot header area will zoom the plot display to full
//   screen Cicking on the zoomed plot header area will restore the screen
//   to its previous state.
//   Note that short clicks might be ignored... particularly on devices
//   like UCCM receivers that only output time updates every two seconds.
//   It can help to hold down the mouse button until the screen changes.
//
//
//   Moving the mouse cursor into the plot area will show the values of
//   the data points at the cursor and the time that the value was acquired.
//   Also the time interval from the first point shown in the plot area
//   to the mouse cursor is shown.
//
//   If a plot has not been enabled, its header entry will not be shown.
//
//   You can disable all plots with the G- keyboard command.  Disabling all
//   plots with G- makes it easier to enable just the plots that you want
//   to see.
//
//   You can enable all plots that have varying data within the plot view
//   window with with the G+ keyboard command.  Emabling all plots can
//   produce a very crowded and confusing display.
//
//   You can put a title string at the bottom of the plot window with the
//   keyboard command:
//      GG  - label the plot window with a title.
//      Titles are also saved to/loaded from log file comments.
//
//      If a plot title contains an '&&' then the '&&' is replaced
//      with the current oscillator disciplining parameters.
//
//      If a plot title contains an '&t' then the '&t' is replaced
//      with the current time
//
//      If a plot title contains an '&d' then the '&d' is replaced
//      with the current date
//
//      If a plot title begins with "&1", "&2", "&3", "&4", "&5", "&6" or &7
//      then the following text will be used as extra title lines positioned
//      above the main title.  Note that the GR screen redraw command will
//      erase the extra titles.
//
//      If a plot title begins with "&b" then the following text will be
//      placed in the program window title bar.
//
//      You can also set the program title bar with the /pt=string
//      command line option.  If you want to include any spaces in the
//      program title bar, replace them with the '_' character.  If the title
//      bar string is empty,  Heather uses the receiver type as the title.
//
//
//   Heather also uses the plot title feature for displaying things like
//   calendar events, errors, and certain status information.  Setting a user
//   defined title usually inhibits these automatic titles.
//
//   Heather usually only updates the parts of the screen that have changed.
//   You can force Heather to fully redraw the screen from the keyboard:
//      GR  - redraw the screen
//   Doing a GR will erase any "extra" title lines.
//
//
//   You can zoom the plot window display to full screen:
//      ZP   - zoom the plot window to full screen
//             Clicking the mouse/touchscreen in the upper right corner of
//             the zoomed plot takes the display out of "plot review" mode
//             just like you pressed the DEL key (but without exiting zoomed
//             plot mode)
//      Clicking on the text above the title bar with also zoom the plot
//      area to full screen.
//
//
//   Data plots can be individually configured for either fixed scale factors
//   or can be automatically scaled. Most plots default to be automatically
//   scaled and centered to fit the plot window.
//
//   If a plot has a fixed scale factor, the plot is labeled like:
//       DAC=(25 uV/div)
//
//   An automatically scaled plot is labeled like:
//       DAC~(25 uV/div)
//   Also, if auto-scaling is enabled, moving the mouse pointer into the
//
//
//
//   The "reference" value for a plot is the value at the horizontal center
//   line of the plot area (indicated by small '<' and '>' marks at the left
//   and right edges of the plot area.
//
//   Plots can be individually configured for either fixed reference
//   values or can have a "floating" reference value where the plot is
//   automatically centered in the plot area.
//
//   If a plot has a fixed reference value, the plot is labeled like:
//       ref=<100 uV)
//
//   A plot with a floating (auto centered) reference value is labeled like:
//       ref~<100 uV>
//
//
//
//   The plot grid is divided into horizonal and vertical major divisions
//   which are further divided into minor divisions.  Time is represented by
//   the horizontal axis and data values by the vertical axis.
//
//
//
//   The "V" (view) keyboard command controls the view window into the plot
//   data queue.  This sets how much data is shown in the plots.  The
//   view value can be either in minutes per (major) division or the time
//   interval to be shown.  Note that the specified view interval is
//   automatically tweaked/rounded to fit a multiple of the plot grid size.
//
//   If the selected plot view interval is larger than one plot queue sample
//   per pixel, the plot queue data is down-scaled by dropping values.
//   Note that if down-scaling is in use the display of time skip markers, etc
//   may not be accurate / complete.  Markers or other events that do not
//   happen on a displayed sample may not always be shown because all
//   non-displayed  samples are skipped over while processing the plot queue
//   data for display.  Setting the display filter to a negative value like
//      FD -1
//   or to positive values greater than (VIEW_TIME/PLOT_WIDTH) will average
//   together all polot queue entries covered by the each displayed pixel
//   and avoid dropping any missing events.
//
//
//      VA  - shows all data currently in the plot queue.  The plot horizontal
//            scale factor (time per division) is adjusted to fit the data
//            onto the screen. The data is down-scaled by dropping values.
//
//      VT  - auto scale the time axis as data comes in.  The plot view
//            interval is automatically adjusted as data come in.  Once the
//            plot window fills, it is scrolled left a few major divisions
//            and a new scale factor is calculated.
//
//            You can force startup in VT mode with the /va command line
//            option.  Doing this has a side effect of not being able
//            to automatically cancel "View Auto" mode whenever the data
//            queues are cleared or reconfigured.  This might cause some
//            problems.  You can cancel this side effect by doing a VA or V0
//            keyboard command.
//
//      V#  - set the plot time to # minutes per horizonal major division
//
//      V#S - set the plot horizontal scale factor so that # seconds of
//            data are shown
//
//      V#M - set the plot horizontal scale factor so that # minutes of
//            data are shown
//
//      V#H - set the plot horizontal scale factor so that # hours of
//            data are shown
//
//      V#D - set the plot horizontal scale factor so that # days of
//            data are shown
//
//      V#W - set the plot horizontal scale factor so that # weeks of
//            data are shown
//
//      X   - set the plot scale factor to 1 hour per division
//
//      Y   - set the plot scale factor so around 24 hours of data are shown
//            If the plot area is being shared with a map display (GB command)
//            this "day" plot view time may be reduced to less than 24 hours.
//
//
//      You can also set the plot view time with the /y command line option:
//         /y=3  - set view to 3 minutes per division
//         /y=3h - set view to 3 hours (can also use #S, #M, #D, #W as
//                 described above for seconds, minutes, days, and weeks)
//         /y    - set the view interval to 24 hour mode
//
//
//   Normally the plots show the last data acquired and scroll to the left
//   as new data comes in.  You can "review" older data using keyboard commands
//   to change the starting point in the plot.  When you are in plot review
//   mode the plot header shows:
//       "Review (DEL to stop):"  or "All (DEL to stop):"
//   and scrolling of the plot data caused by new incoming data is inhibited.
//
//
//      HOME  - move to the start (oldest) of the plot queue data
//      END   - move to the end (newest) of the plot queue data
//      LEFT  - move the plot forward one horizontal division
//      RIGHT - move the plot back one horizontal division
//      PG UP - move the plot forward one screen
//      PG DN - move the plot back one screen
//      UP    - move the plot forward one hour
//      DN    - move the plot back one hour
//      >     - move the plot forward one day
//      <     - move the plot back one day
//      ]     - move the plot forward one pixel (*)
//      [     - move the plot back one pixel (*)
//      DEL   - exit plot review mode and return to normal scrolling
//
//   (*) If the last keyboard command was an ADEV related command:
//      ]     - scrolls the ADEV plots and tables forward one bin
//      [     - scrolls the ADEV plots and tables backward one bin
//      >     - scrolls the ADEV plots and tables forward one bin
//      <     - scrolls the ADEV plots and tables backward one bin
//      HOME  - move to the start of the adev data
//      END   - move to the end of the adev data
//      LEFT  - move 5 bins forward
//      RIGHT - move 5 bins back
//      UP    - move 10 bins forward
//      RIGHT - move 10bins back
//      PG UP - move one page forward
//      PG DN - move one page back
//
//   You can force ADEV review mode with A ESC.  In ADEV review mode the
//   ADEV type in the ADEV tables is followed by a up/down arrow character
//
//   You can force PLOT review mode with G ESC.  In PLOT review mode the
//   ADEV type in the ADEV tables is followed by a ':' character.
//
//
//
//   If you are in VIEW ALL mode and click the LEFT mouse button at a point in
//   the plot, the plot will be centered at that point and the view interval
//   will be set to 1 minute per division.  The clicked point will be marked
//   with a "v" at the top of the plot area and a '^' at the bottom.  You can
//   return to the marked point by pressing '0' or '@'.
//
//   When in "auto" or "all" mode, clicking on the plot to zoom in to a point
//   on the plot will cancel that mode and set normal PLOT REVIEW mode.
//
//   If you (QUICKLY) right click on the graph it will mark the point
//   and center the display on it (without zooming).  You
//   can return to the mark with the '@' or '0' keyboard command.
//
//   You can hold the RIGHT mouse button to scroll around in the plot.  Move
//   the mouse cursor to the horizontal center of the plot and HOLD the right
//   button down.  Moving the mouse cursor to the left or right of the plot
//   center scrolls the plot in that direction.  The further you move the
//   mouse from the center of the plot, the faster the plot will scroll.
//
//
//   You can disable the mouse:
//      /km  - toggle the mouse functionality
//
//
//   You can disable the keyboard from the command line.  The only ways to
//   exit Heather when the keyboard is disabled is to click the CLOSE button
//   on the operating system title bar (or using the "kill process" command
//   of your operating system) or you can use the "ESC y" command.  The
//   "ESC !" keyboard command will re-enable the keyboard.  If you press any
//   key while the keyboard is disable a beep will sound.
//      /kq  - toggle the keyboard enable  (earlier version of Heather used
//             /kk for this, but that conflicted with the temperature control
//             PID commands)
//
//
//
//   PLOT MARKERS
//
//      If you see something interesting a a plot, you can set a marker at that
//      point.  Move the mouse cursor to that point and press "=".  A numeric
//      marker (from 1 to 9) will show at the top of the plot area.
//
//      To center the plot on a marker, press that number on the keyboard.
//      Pressing '0' or '@' centers the plot on the last place you
//      left-clicked in the plot area.
//
//      If you have just gone to a marker, pressing '+' will center the plot
//      where where the cursor was previously located.
//
//      To delete a marker, select that marker from the keyboard and press '-'
//      The GU keyboard command will clear all plot markers.
//
//      Note that the "+" and "-" keyboard commands have a different meaning
//      if you have not just selected a marker... they move the last selected
//      plot up or down and lock the plot scale factors and reference line
//      value (see the description below).
//
//      Plot markers are also saved in and reloaded from log files.
//
//
//   TIME SEQUENCE and HOLDOVER ERRORS
//
//      Heather expects to see a time message from the GPS device every
//      second.  The time code in the message should always increment
//      by one second.  If an error is detected in the time code sequence
//      (like skipped or duplicated time codes) these are flagged by a
//      RED tick mark at the top of the plot and the line that shows the
//      time is drawn in RED (instead of CYAN).  If you place the mouse cursor
//      in the plot area and press '%' the plot will jump to the next time
//      sequence error.
//         GE  - toggles display and finding of time skip markers
//         /ge - toggles display and finding of time skip markers
//
//      If a GPSDO enters a holdover state that is indicated by a solid RED
//      line at the top of the plot area.  The '%' key will also move the plot
//      display to the next holdover event.
//         GH  - toggles display and finding holdover events
//         /gh - toggles display and finding holdover events
//
//
//   CONTROLLING THE DISPLAYED PLOTS
//
//      The "G" keyboard menu controls the plots that are displayed and their
//      configuration.
//
//   These are the standard plot assignments.  Some devices may use the
//   various plots for other purposes.  The G keyboard menu will show what
//   info is assigned to the various plots for that device.
//
//        GP - usually a "PPS" (Pulse Per Second) output related value
//             (TAPR TICC chA Time Interval Error)
//        GO - usually an oscillator frequency output related value
//             (TAPR TICC chB Time Interval Error)
//        GO - usually an EFC DAC or PPS sawtooth correction related value
//        GT - usually a temperature related value
//        GD - GPSDO EFC DAC value or GPS receiver sawtooth value
//             (TAPR TICC debug mode chA FUDGE value).  Depending upon the
//             receiver type the DAC value is either a voltage or a
//             percentage from (-100% to +100% of full scale with 0% being
//             the center of the control range).  The RFTG DAC range is
//             0% .. 100%
//
//        G1 - receiver latitude
//             (TAPR TICC/counter channel A phase value)
//        G2 - receiver longitude
//             (TAPR TICC/counter channel B phase value)
//        G3 - receiver altitude
//             (TAPR TICC/counter channel C phase value)
//        G4 - receiver speed  or TFOM (time figure of merit)
//             (TAPR TICC/counter channel C phase value)
//        G5 - receiver course or FFOM (frequency figure of merit)
//             (TAPR TICC debug mode chA TIME2 value)
//
//             Note: the G4 (speed) and G5 (course) selections don't appear
//                   in the "G" keyboard menu, but do work for receivers that
//                   output heading and speed info.
//
//        G6 - average DOP (dilution of precision) value
//             (TAPR TICC debug mode chB TIME2 value)
//        G7 - the TAPR TICC channel C Time Interval Error data
//        G8 - the TAPR TICC channel D Time Interval Error data
//
//        G9 - message timing offset
//        G0 - message timing jitter
//
//        GA - ADEV plots (see the section of ADEVs)
//
//        G$ - tracked satellite count
//
//
//   The GKx commands control plotting of four different plots. Which data
//   that is being plottted depends upon the device type and which options
//   have been selected:
//
//   If an external enviromental sensor is active the tide plots and sat
//   az/el/signal level plots are not available.
//   These commands control the environmental sensor plots:
//        GKA - toggles display of all the environmenal sensor plots
//        GKH - controls humidity sensor plot
//        GKP - controls air pressure sensor plot
//        GKT - controls first temperatue sensor plot
//        GKU - controls second temperatue sensor plot
//
//   If plotting a satellite az/el/signal level is enabled (SQ command):
//        GKA - toggles display of all sat PRN related plots:
//        GKZ - controls azimuth plot
//        GKE - controls elevation plot
//        GKS - controls signal level plot
//        GKG - controls the carrier phase / pseudorange / gravity plot
//              Carrier phase is plotted if available, else the pseudorange
//              is plotted.  If neither carrier phase or pseudorange is
//              available the plot defaults to the gravity offset plot.
//
//   Otherwise:
//        GKA - toggles display of all the solid earth tide and gravity plots
//               (except for HP5071 cesium beam oscillator devices which uses
//                the eath tide plot queue data entries for other things)
//        GKX - controls longitude earth tide plot
//        GKY - controls latitude earth tide plot
//        GKZ - controls altitude earth tide plot
//        GKG - controls vertical gravity offset plot
//
//
//   Selecting one of the plots listed above brings up a sub-menu of options
//   for controlling that plot.  The descriptions listed below use the "GP"
//   plot, but can be used with any of the plots.  Replace the lower-case "p"
//   shown below with the letter or number of the plot you want to manipulate.
//
//        Gp<cr> - toggles the plot display ON or OFF
//        Gpp    - toggles the plot display ON or OFF
//        /gP    - from the command line - toggles plot "P" ON or OFF
//
//        If the previous key was not a plot marker command (where
//        a following + or - key will undo a marker)  you can
//        use the '+' or '-' keys to move the last selected
//        graph up or down 0.1 divisions.   That graph's scale
//        factor and center reference values will be "fixed".
//
//           +     move the last selected plot up 0.1 major divisions
//           -     move the last selected plot down 0.1 major divisions
//
//
//        GpI    - toggles inversion of the plots' display.
//                 Inverting a plot can make it easier to compare how one
//                 parameter (like temperature) affects another one (like
//                 DAC voltage)
//        /mi    - invert PPS and TEMPerature plots
//
//
//        Gp/ - select plot statistic selection menu. Heather can calculate
//              various statistics of the plot data shown in the plot window:
//           Gp/A   - average value
//           Gp/R   - RMS value
//           Gp/S   - standard deviation
//           Gp/V   - variation
//           Gp/N   - minimum value
//           Gp/X   - maximum value
//           Gp/P   - span - difference between maximum and minimum values
//           Gp<cr> - turns off the plots' statistic display
//           Gp/?   - toggles showing all 7 statistics for a selected plot.
//                    The values are shown as "debug" info in the plot area.
//
//        G/p  - you can set the statistic to show for ALL enabled plots
//               with this command
//
//        GpA  - toggles auto-scaling mode for ALL plots
//        /ma  - from the command line
//
//        GpC  - select the color to display the plot in
//
//        GpD  - show the derivative of the plot data.  When a plot is in
//               derivative mode, the ':' after the plot statistics type
//               is changed to an '*'.  Showing the derivative of a plot can
//               be useful for finding glitches and spikes in the data.
//
//        GpL  - toggles a linear regression trend line for the plot.  The
//               trend line is drawn and its equation (as a function of time)
//               and extrapolated 24 hour change is shown as the plot title.
//               If the selected plot has the "derivative (*)" or "frequency
//               (#)" modifier set, the trend line cannot be plotted.  The
//               trend line equation of the un-modfied plot data is shown,
//               though.
//
//               The trend line equation title also shows the extrapolated
//               trend value over (drift) 24-bours (default).  The OX obscure
//               keyboard option command allows you to change the extrapolation
//               period:
//                  OX0 - 24 hours
//                  OX1 - 1 hour
//                  OX2 - 1 minute
//                  OX3 - 1 second
//
//        Gp=  - removes drift from a plot by subtracting the linear regression
//               trend line slope from the plot.  To cancel trend removal,
//               enter a value of 0.  In trend removal mode the ':' after
//               the plot statistics value is replaced by an '='.
//
//        GpS  - set the plot scale factor in units per division  -or-
//               enable auto scaling of the plot.
//        /mP     - from command line doubles plot P scale factor
//        /mP=val - sets plot P scale factor from the command line (0 = auto)
//        /m      - doubles all plot scale factors from the command line
//        /m=val  - multiplies all plot scale factors by "val"
//
//        GpX  - toggles auto-scale mode for the selected plot
//
//        GpZ  - set the plot zero reference value (the value at the horizontal
//               center line of the plot) to a fixed value  -or=  enable auto
//               centering of the plot.
//        /zp      - toggles plot P auto-centering from the command line
//        /zp=val  - sets plot P zero reference line value the command line
//                   (0 = auto center).  "p" can be any plot selection
//                   character.
//
//        GpF  - calculate an FFT (Fast Fourier Transform) of the selected
//               plots' data.
//
//                 The data to be analyzed starts at the beginning of
//                 the plot window and end when the max fft size points
//                 or the end of the plot data are reached.  The
//                 data is sampled from the plot data at the viewing
//                 interval (seconds/pixel).
//
//                 The calculated FFT bins are shown as seconds with the
//                 lowest frequencies to the left and highest to the right.
//                 Moving the mouse cursor within the plot area shows the
//                 various FFT bin values.
//
//                 The max FFT size is set by the /qf[=#] command line option.
//                 (1024 points... if not given then /qf is 4096 points,
//                 otherwise it is whatever is set (must be a power of 2). If
//                 the number of data points in the plot queue to less than
//                 the FFT size, the FFT size is reduced accordingly.
//
//                 Note that a FFT requires the input data to be bandwidth
//                 limited to below the sample frequency or else you will.
//                 get spurious values due to aliasing.  Set the display
//                 display filter count to at least the value shown as
//                 the "view interval" in minutes/division.
//
//                 The FFT plot option defaults to doing a single
//                 FFT of the selected plots' data when the FFT command was
//                 given.  The O L keyboard command can be used
//                 to enable a "live" FFT that gets recalculated
//                 every time a new point is added to the plot.
//
//                 The O D keyboard option toggles the FFT display value
//                 format between raw values (default) and dB's.
//
//                 Whenever an FFT is calculated, the first point of the plot
//                 queue data analyzed by the FFT is marked with MARKER 1 and
//                 the last point is marked with MARKER 2.
//
//
//        GpH  - calculates a histogram of the currently displayed plot
//               queue data.  The histogram and FFT options share the
//               the plot same display code and cannot be used at the same
//               time.
//
//
//        GF   - toggle the FFT or histogram plot display on and off.
//               If the debug log file is open then whenever a FFT or
//               histogram plot is toggled on the current histogram or
//               FFT data is written to the debug file.
//
//        Note:  Histograms and the FFT use a lot of the same code.  You
//               cannot enable both a FFT and histogram at the same time.
//               You can only show histograms or FFTs for one plot at a
//               time.  You turn off a histogram with the GF command (the
//               G menu always shows F)FT for the GF command, even though it
//               can turn off the histogram plot.
//
//
//        Gp~  - applies a deglitch filter to the selected plot. The filter
//               prompts for a "sigma" value and replaces any point in the
//               plot that is more than "sigma" standard deviations away from
//               the average plot value with the previous point.  To remove
//               glitches from all plots, use the CG command.
//
//        Gp#  - for time interval counters, shows the selected plot's TIE
//               (time interval error) value as frequency.  This also works
//               for the PRS-10 FC (frequency control) plot to show the value
//               in Hz instead of PPT (parts per trillion).
//
//        GCG  - controls the tracked satellite count plot.
//               The satellite count plot is shown in CYAN at the bottom of
//               the plot window (each minor division equals one satellite)
//        /g$  - from the command line
//
//
//      You can apply an averaging filter to the displayed plots (see the
//      description on the /fd command in the FILTERS section for more details.
//         FD  -  set the display filter count.
//
//
//      Normally Heather displays the OSC related parameter values in ppb
//      (parts-per-billion) or ppt (parts-per-trillion) which can cause
//      Europeans to wander off in a daze of confusion and angst.
//      You can display the OSC related value with an exponent using:
//         /tx - toggle OSC value displays with eXponent
//
//
//
//   PATCHING GLITCHES IN THE PLOT DATA
//
//      Occasionally you may have a data glitch in the acquired plot
//      data.  Heather allows you to patch a glitched data value.
//
//      Carefully position the mouse cursor on the leading edge of the
//      offending point and press '~'  This will replace the point's data
//      with the previous data point.  Ideally you should have the display
//      filter (FD command) turned off and the view interval (V command) set
//      to one point per pixel (V 0) when patching data glitches.
//
//      If the mouse cursor is not in the plot area and over a data point
//      you will hear a warning BEEP.
//
//      Each time you press '~' the glitch edit point (but not the mouse
//      cursor) is incremented to the next point.  You can press '~' multiple
//      times to remove a wider data glitch as long as the mouse does not
//      move (by even a single pixel).  Whenever the mouse moves, the glitch
//      edit point changes to the new mouse position.
//
//      You may want to then use the CR command to recalculate xDEV
//      and MTIE values.
//
//      After patching up a plot glitch you can use the CR command to
//      recalculate xDEV and MTIE data.
//
//
//      Heather can also automatically remove all glitches from a plot with
//      the Gp~ command (where 'p' is the plot to deglitch).
//        Gp~  - applies a deglitch filter to the selected plot. The filter
//               prompts for a "sigma" value and replaces any point in the
//               plot that is more than sigma standard deviations away from
//               the average with the previous point.
//
//      The CG command will remove glitches from all plots.
//
//      The Gp~ and CG deglitch commands work on the data interval in the
//      plot queue that is currently being displayed.  To deglitch all the
//      data in the plot queue, use the VA command first to display all
//      the plot queue data.  The way these commands work is ineffective
//      when removing a glitch at the first point in the viewed plot area.
//
//      One way to remove glitches at the start of the plot area is to use
//      the CT command:
//
//        The CT command will trim (delete) all plot queue entries before
//        the first entry shown in the plot area or after the last one.
//
//        This can be a useful alternative to using the CP or CC commands
//        that clear all data from the plot queue if you want to remove
//        garbage data from the plot queue that can occur when the program
//        first starts capturing data from the receiver.  You may need to
//        follow the CT command with the CR command to recalulate ADEVs
//        from the remaining data in the plot queue.
//
//        To delete data at the start of the plot queue first position the
//        plot view so the first entry you want to keep is at the left edge
//        of the plot window.  Then enter CTS and  all previous entries will
//        be deleted.
//
//        To delete data at the end of the plot queue first position the
//        plot view so the last desired entry is at the right edge of the
//        plot window and enter CTE and all the following entries will be
//        deleted.
//
//        To delete data that is not shown in the plot window,  adjust the plot
//        view to show all the data you want to keep.  Then enter CTSE and all
//        data not shown in the plot window will be deleted.
//
//        After entering a "CT" command, the plot view will be set to "view all"
//        just like you entered "VA".
//
//      There is no way to un-do a glitch removal or plot queue trim.
//
//
//
//
//   TEMPERATURE DISPLAYS
//
//      Lady Heather can display the temperature readings in several
//      different temperature scales (I bet you didn't know there are
//      so many!) by using one of the following command line options:
//
//           /tc    - show Celcius temperatures (default)
//           /td    - show DeLisle temperatures
//           /te    - show Reaumur temperatures
//           /tf    - show Fahrenheit temperatures
//           /tk    - show Kelvin temperatures
//           /tn    - show Newton temperatures
//           /to    - show Romer temperatures
//           /tph   - show Paris Hilton temperatures - at 20C Paris is
//                    assumed to be nominally clothed (yeah, right).  Every
//                    10C above that she sheds a layer of clothing.  Below
//                    that she adds one.  Degrees H indicates how hot Paris
//                    Hilton actually is (or thinks she is).
//           /tr    - show Rankine temperatures
//           /ty=#  - show temperatures to # decimal places
//
//
//
//
//
//-WRITING SCREEN DUMPS
//
//   Heather can write .GIF or .BMP image files of either the entire screen
//   or just the plot area.  Dumps can be either as the screen appears or
//   in "reverse video" where BLACK and WHITE are swapped.  Reverse video
//   dumps are useful for printing.
//
//     WS  - write screen to image file
//     WR  - write reverse video screen to image file
//
//     WG  - write plot (graph) area of the screen to image file
//     WI  - write reverse video plot (graph) area of the screen to image file
//
//   You get a beep at the start and end of the dump.
//
//
//   The '\' keyboard command can be used to do a quick full screen .GIF dump
//   with a single keystroke.  It does not bring up a menu that prompts for a
//   file name.  This can be useful because redrawing of the plot area after
//   entering the file name from the normal screen dump menu can cause
//   problems with some parts of the screen not being updated before the
//   screen dump happens. The "quick" screen dump is done to file
//     "xxxxx.gif"
//   where "xxxxx" is dependent upon the receiver type.
//
//   You can also force a screen dump by left-clicking on the screen where
//   the "Receiver data" info is near the upper left hand corner of the
//   screen.  This is handy if you are using a touch-screen.
//
//
//   If you start to select a reverse video screen dump (even if you do not
//   complete it by using the ESC key) the reverse video attribute will
//   will be used for any following '\' initiated screen dumps. The same
//   applies to normal polarity screen dumps.
//
//   If the mouse cursor is in the plot area when a screen dump is done, an
//   "X" will be drawn in the plot area of the dump.
//
//
//
//
//
//-GPSDO and HOLDOVER CONFIGURATION
//
//   GPS Disciplined Oscillators (GPSDOs) use the GPS satellite signals to
//   steer a precision oscillator to a precise frequency (usually 10.0 MHz)
//   over long periods of time.  This "discipling" process corrects for normal
//   oscillator frequency drift, aging, and temperature effects.
//
//   The steering of the oscillator frequency is usually done by a phase
//   locked loop (PLL) circuit in the GPSDO that compares the current frequency
//   to the GPS timing signals.  A GPSDO controls the oscillator frequency by
//   using a digital to analog converter (DAC) to drive the oscillator's
//   electronic frequency control (EFC) signal.
//
//   If a GPSDO loses lock on the GPS satellites it can enter a HOLDOVER mode
//   where it attempts to guess the compensation that it needs to apply to the
//   oscillator in order to keep it on frequency.  Many GPSDOs can be
//   forced into HOLDOVER mode manually by the user for testing, etc.
//
//   Heather has some keyboard commands for controlling holdover mode and
//   oscillator disciplining:
//
//      HE - enable manual holdover (for the PRS-10 this enables 1PPS input
//           discipling mode - the same as the DE command)
//      HX - exit manual holdover (for the PRS-10 this disables the 1PPS
//           input discipling mode - the same as the DD command)
//      HH - toggle holdover mode (discipling on the PRS-10) on/off
//
//
//   Besides HOLDOVER mode, some GPSDOs and the PRS-10 rubidium oscillator
//   let the user manually control the oscillator EFC signal or turn ON/OFF
//   oscillator discipling.
//
//      DD - completely disable oscillator disciplining
//      DE - re-enable oscillator disciplining
//      DS - set the oscillator EFC DAC to a given value.
//
//
//   The plot area normally shows when the device is in holdover by a RED
//   line at the top of the plot. You can turn this off with:
//      GH  - from the keyboard toggles the holdover plot display
//      /gh - command line option toggles display of holdover events
//            in the plot area.
//
//
//   Heather shows the amount of time of the current or last holdover event.
//   The value is shown in GREEN if the holdover event ended before Heather
//   was started. It is shown in YELLOW if a holdover event occured after
//   Heather was started.  It is shown in RED if holdover mode is currently
//   active.
//
//
//
//  GPSDO DISCUPLINING CONFIGURATION PARAMETERS:
//
//     Some GPSDOs let the user configure various parameters of the oscillator
//     disciplining PLL system.
//
//     GPSDO control parameters are accessible from the "&" keyboard menu.
//     Not all GPSDOs support all of these settings.
//
//     The "&" keyboard command brings up the oscillator parameter
//     and satellite max signal level display screens.
//     To return to normal display mode you can press SPACE
//     or ESC or "&" again.
//
//     The "&" display screen shows the current GPSDO parameters and also
//     a table of the maximum signal level seen from each satellite.
//
//       &d  - sets the PLL damping factor
//       &t  - sets the PLL time constant
//       &g  - sets the PLL gain (i.e. Hz change per volt of EFC)
//       &p  - sets the pullin range (for UCCM devices)
//
//       &i  - sets the DAC initial voltage
//       &n  - sets the DAC minimum voltage
//       &x  - sets the DAC maximum voltage
//
//       &h  - sets the DAC allowable range high value
//       &l  - sets the DAC allowable range low value
//
//       &f  - sets the max frequency offset threshold
//       &j  - sets the jam sync threshold
//
//       &a  - autotune oscillator parameters
//
//
//     Some GPSDOs (like the Nortel/Trimble NTBW and NTPx devices) let the user
//     control the GPSDO settings, but do not save them in EEPROM.  You can
//     cause Heather to configure the GPSDO at startup with these command
//     line options:
//
//       /ud=val - set oscillator disciplining damping value
//       /ut=val - set oscillator disciplining time constant value
//       /ug=val - set oscillator disciplining gain value
//       /up=val - set pull-in range value (for UCCM devices)
//       /ui=val - set oscillator disciplining initial dac value
//       /un=val - set oscillator disciplining minimum dac value
//       /ux=val - set oscillator disciplining maximum dac value
//       /uh=val - sets the DAC allowable range high value
//       /ul=val - sets the DAC allowable range low value
//
//       The &L (/ul) and &H (/uh) commands use an undocumented TSIP command
//       for reading and setting the what appears to be the allowable EFC DAC.
//       range.  You can alter the low and high values with the UNDOCUMENTED
//       &L and &H keyboard commands.  These values might just be for reporting
//       the allowable DAC range to the software or they might do something
//       else!  Also, /ul and /uh on the command line may not always work...
//       it seems to work from the keyboard, though.
//
//                  CAVEAT EMPTOR IF YOU USE/CHANGE THEM!
//
//
//     When you write a log file an OSC_GAIN comment is written to the log
//     file.  This allows the computation of osc drift and tempco from logged
//     data acquired on a unit that does not have the same osc gain (or that
//     is not connected to a tbolt).  If the osc gain has been loaded from a
//     log file,  it is shown in YELLOW.
//
//
//
//  OSCILLATOR DRIFT RATE and TEMPERATURE COEFFICIENT VALUES
//
//     For Trimble GPSDO devices, the '&' screen display shows an oscillator
//     drift rate and temperature coefficient value.  These are calculated
//     from the data points shown in the plot window.
//
//     For the best results,  select a plot display interval
//     that covers a fairly long time interval with well
//     behaved DAC and/or TEMP values.
//
//     You should be able to get a decent value for the drift rate
//     with 24 hours of data. The oscillator drift rate value makes the most
//     sense if the temperature is stable so that temperature effects on the
//     oscillator frequency are minimized.
//
//     The oscillator temperature coefficient value makes the most sense if
//     the temperature is allowed to change in a linear ramp.  This is
//     easy to achieve with Heather's active temperature control mechanism.
//
//
//
//  AUTOMATICALLY SETTING GPSDO OSCILLATOR PARAMETERS:
//
//     Heather can automatically configure Thunderbolt style GPSDO parameters
//     to values better suited for precision time and frequency applications
//     that their default "telecom" values.
//
//     The "&a" keyboard command sequence starts the "auto-tune" process.
//
//     Before running auto-tune on the Thunderbolt you should:
//        1) Wait for the unit to stabilize with (relatively)
//        steady DAC, OSC, PPS, and temperaure values. Overnight is good.
//
//        2) Set your antenna elevation mask angle (F E keyboard command)
//        to 0 and signal level mask (F L keyboard command) to 1.  This allows
//        collection of signal level data across the full sky.
//
//        3) Clear your signal level history (C M keyboard command)
//
//        4) Let Heather run for at least 6-12 hours to build
//        up a new satellite signal level map.
//
//        5) Issue the "&a" auto-tune command and wait for it to
//        complete
//
//     Heather will spend a couple of minutes tickling
//     your oscillator and determine good values for the
//     osc gain and initial dac voltage.  The time constant
//     will be set to 500 seconds,  and the damping to 1.0
//     Also the AMU (signal level) mask will be set to 1.0
//     and the satellite elevation mask to midway between the horizon
//     and where your average signal level begins to fall off.
//
//     The values will be written to the configuration EEPROM.
//     You can then manually change any of the parameters
//     that may not suit your needs.
//
//     Note that non-Thunderbolt GPSDOs (such as the NTBW and NTPx telecom
//     devices) do not all support saving disciplining parameters to EEPROM
//     and some do not allow users to change the disciplining parameters.
//     You can use "/d" command line options for setting the PLL parameters
//     from the command line.
//
//     If you run the auto-tune command on non-Thunderbolt GPSDO devices then
//     only the satellite elevation mask (and perhaps the signal level mask)
//     values will be set.
//
//     You can erase the auto-tune results message in the plot title using
//     the "GG" keyboard command.
//
//
//
//     For the Symmetricom rubidium oscillators (X72, X99, SA22) the &a
//     command calculates and sets the DDS tuning word that sets the
//     oscillator output frequency (DDS).  Before issuing the "&a" command you
//     should have a stable 1PPS signal connected to the PPS input and wait
//     for the device to stabilize (recommend at least 24 hours of power on).
//
//     The &a command will prompt for the number of seconds of data to analyze
//     to determine the device drift rate... the longer the better.  The
//     minimum time is 600,  the default is 1800 seconds.  Several hours is
//     better.  First the DDS word is set to 0 and the TIC offset register
//     is set to zero out the current 1PPS offset.  Next data is collected
//     for the specified number of seconds.  Finally, the data is analyzed
//     by calculating a linear regression trend line and the DDS tuning word
//     is set and the TIC offset value is re-zeroed.
//
//     After the auto-tune interval has been reached, Heather will set the
//     DDS tuning word to the calculated value.  On some firmware versions
//     the EE keyboard command can write the tuning word (and maybe the other
//     device configuration values) to EEPROM.  The calculated tuning
//     value will be shown in the plot title line.  You can record that value
//     and use the PZ keyboard command to restore the DDS word if you lose
//     power.  Note that the X72 / SA22 has no way to read back most of the
//     current settings (more ass spanking justified).
//
//     Heather implements a "software eeprom" that records all setting
//     changes in a file (default tbeeprom.dat).  This file is used
//     to restore all the last known settings every time Heather is started.
//     If you have more than one Symmetricom rubidium oscillator, the "/wp="
//     command can be used to change the "tb" prefix string used for the
//     EEPROM file. For example /wp=sa22 will use the file "sa22eeprom.dat".
//     The /wp command also changes the prefix used for automatic log and
//     screen dumps.  When you first setup a X72/SA22 you should set all
//     the device parameters to what you want (even if they appear to
//     already be set correctly.  This will insure that Heather knows the
//     correct device configuration.
//
//
//
//  ACTIVE GPSDO TEMPERATURE CONTROL
//
//     GPS Disciplined Oscillators are very precise devices and their
//     precise oscillators and circuity can be sensitive to temperature,
//     other environmental conditions (air pressure, humidity, etc), and
//     power.  You can maximize a GPSDOs performance by minimizing changes
//     in these values.
//
//     Heather has the ability to actively control the environmental
//     temperature of GPSDO devices that regularly (like every second) report
//     their temperature to a high level of resolution (like less than 0.1C).
//     The Trimble Thunderbolt series of devices is particularly good at this.
//     Early models have a temperature sensor with a resolution of around
//     0.01C and later models used an inferior version of the sensor with
//     around 0.1C resolution.
//
//     Heather uses a PID control algorithm driven by the GPSDO temperature
//     sensor readings to PWM (pulse width modulate) a temperature control
//     device (such as a fan or a peltier device).
//
//     You specify the desired control temperature in degrees C.  A value
//     of 0 says to turn off temperature control.
//        TT          - from the keyboard
//        /tt=degrees - from the command line
//
//     When temperature control is activated an DOWN_ARROW, EQUAL_SIGN, or
//     UP_ARROW is shown next to the temperature reading at the top of the
//     screen.  They mean that the temperature control PID is calling for
//     cooling, holding temperature, or heating.
//
//
//     Normally Heather uses the receiver port RTS and DTR serial port modem
//     control lines to control the fan or peltier (there is some legacy
//     code in there that could be activated to work with a parallel port).
//
//     Heather also supports using a dedicated serial port to control the
//     "fan".  This can be enabled with the /ef= command line option.  The
//     "fan" port manipulates the modem control signals just like when using
//     the receiver port.  It also sends ASCII commands (default 115200,8,N,1)
//     out the "fan" port.  Using a dedicated "fan" port can be useful if the
//     receiver port does not support modem control signals (like some USB
//     devices or an Ethernet connection).
//        $INIT<cr><lf> - initialsize the temperature control device
//        $STOP<cr><lf> - disable the device
//        $HOLD<cr><lf> - hold the current state
//        $CTRL msecs<cr><lf> - values to PWM the device with.
//             Positive values indicate heating,  negative indicate cooling
//             The value is in millisecods (can be interpreted as percent*10)
//
//     RTS is the temperature controller enable (+12=off, -12=on)
//     DTR is the heat (-12V) / cool (+12V) line.
//     (note that some serial ports may have different voltage ranges like
//      +/- 6V)
//
//     The DTR line is updated once a second with a pulse whose width is
//     proportional to how hard to drive the fan/peltier. Use the line to
//     control a transistor or solid state relay.  The RTS line can be
//     used to enable the temperature control device.  RTS and DTR will be
//     set to +12V when Heather exits.
//
//     Simple implementation:
//        Isolate the Thunderbolt in a box (I use a corrugated cardboard box),
//        set the control temperature below the typical unit operating temp
//        but above normal room temp.  When Heather signals COOL turn
//        send power to a fan to move room air into the box.  You want the
//        fan to move enough air to cool the unit,  but not so
//        much air that the temperature drops by more than 0.01C
//        per second.  The indicator of too much airflow is a
//        temperature curve that spikes down around 0.1C several
//        times per minute.  Too little airflow shows up as a
//        curve that oscillates around 0.25C about set point over
//        a couple of minutes.  Good airflow should show a
//        temperature curve stable to with 0.01C with a period of
//        between 1 and 3 minutes.   Generally you do not want
//        the fan to blow directly on the unit (mine is surrounded
//        by foam).  You want to gently move air through the
//        box.  It can help to include a large thermal mass in
//        the box (I use a 2kg scale weight, other people use water).
//
//        Placing the GPSDO power supply in the same controlled environment
//        as the GPSDO can improve system performance.  BE AWARE OF WHAT
//        MIGHT HAPPEN IF A POWER SUPPLY OR CONTROL SYSTEM FAULT CAUSES
//        OVERHEATING OR A FIRE!
//
//        One thing to try is to disable the fan and closely monitor the
//        temperature see how warm the device becomes in the event of a fan
//        failure. Try and use an enclosure that keeps the maximum device
//        temperature to safe levels.  An enclosure with too much insulation
//        is not a good thing here...
//
//
//  TUNING THE TEMPERATURE CONTROL PID
//
//     The temperature control PID is adjusted via the "K" keyboard menu or
//     "/k" command line options.  The "K" keyboard menu works a bit
//     differently than the other keyboard menus! It does not bring up any
//     sub-menus.  It brings up an edit line where you enter a command letter
//     optionally followed by command values.  Commands that expect a value
//     (or values) are shown with an "=" sign here.  You do not type the "="
//     sign, just the letter and the value(s) separated by a space or comma.
//
//     From the command line you DO enter the "=" to set a value. For instance
//     you would set the PID parameters with:
//        K 1.5 45 2 100     from the keyboard "K" menu or
//        /kk=1.5,25,2,100   from the command line
//
//
//        A= Autotune temperature PID
//          A 0  - abort autotune
//          A 1  - full autotune with all settling delays
//          A 2  - don't wait for temp to get near the control setpoint
//          A 3  - don't wait for PID to stabilize
//
//        P= PID Proportional (gain) term
//        I= PID Integral control term
//        D= OID Derivative control term
//        F= PID Filter time constant
//        K= P D F I   (sets all foure major PID "K" terms)
//
//        W  config PID with canned slow time constant PID values (default)
//        X  config PID with canned fast time constant PID values
//        Y  config PID with canned very fast time constant PID values
//        N  config PID with canned medium time constant PID values
//
//        G= Scale factor (loop gain correction)
//        H  show PID debug info
//        L= Load disturbance test value
//        O= filter Offset term
//        R= Reset integrator value
//        S= Scale factor (loop\ gain correction)
//           (use /kg= from command line for this since /ks controls sounds!)
//        T= Test PWM output  (cool_msecs, heat_msecs)
//        Z  reset and restart temperature PID control loop
//        9= set auto-tune step size
//        0  test with crude temperature values
//
//
//
//
//   EXPERIMENTAL OSCILLATOR CONTROL PID
//
//     The Thunderbolt GPSDO is a very versatile device that offers the
//     user great flexibility and control.  You can manually control the
//     oscillator EFC DAC and measure the oscillator frequency and PPS error.
//     This provides the ability to implement a user-defined GPSDO control
//     loop.  Lady Heather has an implementation of such a control PID. It
//     is activated from the "B" keyboard command or /x command line options.
//
//     The experimental oscillator control PID commands work like the
//     temperature control PID commands described above.
//
//        K .1 0 100 100     from the keyboard "B" menu or
//        /xk=.1,0,100,100   from the command line
//
//        P= Proportional gain (P)
//        I= Intergral time constant (I)
//        D= Derivative time constant (D)
//        F= Filter time constant (F)
//        K= P D F I   (sets all four PID "B" terms (P D F I))
//
//        N  use pre-configured PID #1 values
//        W  use pre-configured PID #2 values
//        X  use pre-configured PID #3 values
//        Y  use pre-configured PID #4 values
//
//        H  show PID debug info
//        L= Load disturbance test value
//        O= Filter offset
//        Q= Post-filter depth
//        R= Reset integrator
//        S= Scale factor (loop gain compensation)
//        Z  Reset and restart oscillator control PID
//        0  Disable oscillator control PID
//        1  Enable oscillator control PID
//
//
//
//
//
//-GPSDO / TIMING RECEIVER PPS and FREQUENCY OUTPUT CONTROL
//
//   GPS timing receivers and disciplined oscillators usually have one
//   or more timing output signals.  Almost all have a 1 pulse-per-second
//   (PPS) output signal.  On many devices this can be programmed for
//   other frequencies.  Other devices also have other auxiliary output
//   signals that can be controlled.
//
//   The "P" keyboard menu provides commands for controlling these output
//   signals.  The paramters that can be set via the "P" menu depend upon
//   the device type.
//
//
//
//   ANTENNA CABLE DELAY
//
//     To precisely align the PPS output to GPS or UTC time, the receiver
//     needs to know the delay of the GPS signal through the antenna cable
//     (plus the delay through the PPS output cable).  The cable delay
//     adjustment can also be used to shift the PPS output relative to
//     GPS/UTC time for other reasons.
//
//     Note that some devices (like the Trimble Thunderbolt and the
//     OSCILLOQUARTZ STAR) use a NEGATIVE number for compensating for cable
//     delay compensation, other devices use a  positive number.  Consult
//     the device manual for details (but good look finding them!)
//
//        PC         - sets the cable delay
//        /c=75      - sets the cable delay (in nanoseconds) from the command
//                     line
//        /c=50f     - sets the cable delay to 50 feet of 0.66 velocity factor
//                     coaxial cable
//        /c=70m     - sets the cable delay in 70 meters of 0.66 velocity
//                     factor coaxial cable
//        /c=50m,.8v - sets the cable delay to 50 meters of 0.80 velocity
//                     factorcoaxial cable
//
//     The "PC" keyboard command lets you specify the cable delay in
//     nanoseconds or in cable length / velocity factor:
//       PC 100      - 100 nanoseconds
//       PC 50f      - 50 feet of 0.66 velocity factor coax
//       PC 75m .77v - 75 meters of 0.77 velocity factor coax
//
//     For the SRO100/SRO70 PC sets the clock in the device to the displayed
//     system date/time. Heather does not currently use the date/time info
//     that can be read from the SRO100/SRO70.
//
//
//
//   PPS SIGNAL CONTROL and OUTPUT FREQUENCY CONTROL
//
//      Note that some devices (mostly counters and atomic frequency
//      references) have different meanings for some of these commands.
//      Check the "P menu on those devices!
//
//      You can turn the PPS signal ON, OFF, or toggle its state:
//        PD   - disable the PPS signal
//        PE   - enable the PPS signal
//        PS   - toggle the PPS signal ON or OFF
//        /pd  - disable PPS output signal from the command line
//        /pe  - enable PPS output signal from the command line
//        /p   - toggle PPS output signal from the command line
//
//        For TAPR TICC devices PE sets the input EDGE select values.
//
//        For the PRS-10 PS sets the magnetic switching mode.
//
//        For the PRS-10 PD sets the 1PPS output offset delay (used to
//        align the 1PPS output to match another device)
//
//        For the SRO100 PD sets the PPS delay.
//        For the SRO100 PS sets the sync mode.
//        For the SRO100 PW sets the PPS pulse width.
//        For the SRO100 PK sets the PPS tracking mode.
//
//        For the SRO100 PC sets the clock in the device to the displayed
//        system date/time.
//
//        For the SRO100 PG sets the "gofast" mode countdown timer.
//        Gofast mode was implemented in device firmware > 1.96.
//
//
//      You can control the PPS signal polarity on some receivers:
//        PR   - select rising edge PPS signal
//        PF   - select falling edge PPS signal
//        PP   - toggle PPS polarity
//        /-   - select rising edge PPS signal from the command line
//        /+   - select falling edge PPS signal from the command line
//
//        For Furuno eSIP devices the PF command is only available in GCLK
//        mode. It is not avaliable in LEGACY clock mode. You can control
//        the clock mode:
//           PG - sets GCLK mode (this has noiser PPS output statistics)
//           PL - sets LEGACY clock mode
//
//        For TAPR TICC devices PF sets the channel "FUDGE" factor
//        delay compensation values.
//
//        For PRS-10 devices PF enables discipling to a 1PPS input with
//        input filtering enabled and PU selects unfiltered 1PPS input.
//        PH configures the 1PPS output to be a static "locked" signal
//        where a high level means locked.  PL selects a low level
//        "locked" indication.
//
//        For PRS-10 devices PA sets the PRS-10 frequency offset using the SF
//        command.  Note that the SF setting is temporary and is reset
//        between device power cycles and resets.  Also, it is not used if
//        the PRS-10 1PPS input is used to discupline the oscillator.
//
//        For PRS-10 devices PY sets the PRS-10 frequency synthesizer control
//        words that allow larger and permanent changes in the output
//        frequency.  There are around 100 valid combinations of synthesizer
//        settings...  see the PRS-10 manual.
//
//        For the SRO100 PR sets the raw phase adjust.
//        For the SRO100 PF sets the frequency adjustment (FC) value.
//        For the SRO100 PH sets the phase comparator offset.
//        For the SRO100 PP does nothing.
//
//
//      The Trimble receivers let you set the oscillator signal polarity
//      reference to the PPS signal:
//        PO   - toggle the oscillator signal polarity from the keyboard
//        /^f  - sync OSC signal falling edge to time
//        /^r  - sync OSC signal rising edge to time
//        /^   - toggle OSC signal edge
//
//
//      Some other devices let you adjust the PPS timing offset in relation
//      to UTC/GPS time (much like the antenna cable delay parameter). For
//      these devices the PO command is used to set the PPS offset.
//        PO   - set the PPS pulse timing offset in relation to UTC/GPS time
//               (the times are in nanoseconds and can usually be positive or
//                negative)
//
//               For the PRS-10 PO sets the magnetic offset value (used to
//               tweak the output frequency by small amounts)
//
//               For the PRS-10 PT sets the time tag offset offset value
//               (used to adjust the time tag by up to +/- 32767 ns)
//
//               For the X72 PO tells Heather what the master oscillator
//               frequency is (usually 60 MHz)
//
//
//      Some devices let you configure the PPS output between a 1 PPS mode
//      and a PP2S mode (telecom standard pulse per 2 seconds):
//        P1   - select 1 PPS mode
//        P2   - select PP2S mode
//
//      On the Oscilloquartz Star-4 GPSDO:
//        P1   - enable TOD (time of day) output
//        P2   - disable TOD output
//
//
//      Some devices let you reference the PPS pulse to either UTC time or
//      GPS time (and usually this also changes the time reported by the
//      receiver to GPS or UTC time).
//        TG   - configure the receiver for GPS time
//        /tg  - configure the receiver for GPS time from the command line
//
//        TU   - configure the receiver for UTC time
//        /tu  - configure the receiver for UTC time from the command line
//
//        TN   - configure the receiver for UTC time with PPS synced to
//               UTC(SU) (ESIP receivers only)
//
//
//
//      Some devices have one or more pulse/frequency output signals that let
//      you set the pulse rate/frequency and/or pulse duty cycle or width:
//        PA   - configure pulse output A
//        PB   - configure pulse output B
//
//        You enter the pulse frequency in Hz and optionally a pulse
//        width or duty cycle (duty cycle values are between 0 and 0.99999,
//        pulse width values are a width in microseconds >= 1.0).  If a pulse
//        width/duty cycle is not given, the default is 50% duty cycle.
//        Note that most devices have limits of the output frequency and
//        width/duty cycle that is available, and may be different for each
//        output... consult your device manual.
//
//        For the SRO100 PA sets the PPS tracking alarm window.
//
//
//      For the Symmetricom X72 and SA22 rubidium oscillators you can set
//      the ACMOS output frequency:
//         PN  - set the ACMOS output frequency or divider ratio.
//               If you follow the input number with an 'H' the frequency
//               will be set to the closest possible value in Hz (accuracy
//               is limited by the interger divider ratio)
//
//
//
//   TRAIM CONTROL
//
//      Many GPS timing receivers support a feature called TRAIM - Time
//      Receiver Autonomous Integrity Monitoring.  TRAIM mode monitors the
//      timings from the various satellites and, ifinconsistencies are
//      detected, can drop suspect satellites from the timing solution, set
//      alarm conditions, etc.  Consult your device manual.
//
//        PT   - configure device TRAIM mode / threshold.
//               For TAPR TICC devices this sets the "FIXED TIME2" values.
//               For the SRO100 PT sets the PPS tracking window half-width.
//
//
//
//
//
//-NON-VOLATILE CONFIGURATION EEPROM/FLASH/SRAM memory
//
//   Some receivers support saving all or parts of the receiver configuration
//   into some form of nonvolatile memory (like EEPROM).  Configuration
//   saves are currently supported on Trimble, Ublox, and Venus/Navspark devices.
//   Also the HP-5071A cesium beam oscillator.  The Symmetricom X72 and SA22
//   rubidium oscillators also support EEPROM, but what gets saved is not
//   documented and may vary depending upon the firmware version.  The
//   TruePosition GPSDO has an undocumented "$UPDATE FLASH" command the EE
//   sends.
//
//   Whenever EEPROM is written Heather sounds a BEEP.
//   If you hear lots of unexpected beeps,  something cold be
//   wrong and you may be wearing out the EEPROM.  Note that SCPI
//   type devices also sound a BEEP when a receiver message response does
//   not seem to match what was expected (loss of message sync).
//
//   Jupiter receivers automatically write things like almanac data to EEPROM
//   and will periodically produce EEPROM write BEEPs.  After a power-on there
//   will be quite a few during the first hour or so.
//
//      /kc     - toggles disabling of writing configuration data into EEPROM
//      /kc1    - disable writing into EEPROM
//      /kc0    - enable writing into EEPROM
//
//
//      EE      - from the keyboard, writes the complete current receiver
//                configuration to EEPROM/BBRAM/FLASH.
//
//                For Venus receivers this command toggles the enable of
//                writing configuration changes to flash memory.  If writing
//                to flash is enabled, the receiver status column will
//                show "FLASH: WRT" instead of "FLASH: OK".  Once "write flash"
//                mode is enabled any new configuration changes are saved to
//                flash.  Previous changes are not written to flash.  Writing
//                to flashdefaults to OFF for Venus receivers amd changes
//                are only saved to battery backed RAM (if available).
//
//                For the PRS-10 rubidium oscillator this command will prompt
//                you to select the parameter that should be saved.  The
//                default prompt is for the last paramter that you changed.
//                Entering "ALL" will save the current values of all the
//                save-able paramters.
//
//                Changing any of the PPS/OSC/Cable delay parameters
//                from the keyboard stores the values in EEPROM unless EEPROM
//                writes have been disabled.  Changing them from the command
//                line does not cause an EEPROM write.
//
//
//
//
//
//-ADEV (and MTIE) CALCULATIONS and DISPLAYS
//
//   Lady Heather can calculate and display various ADEV (Allan Variance/
//   Deviation) values from the receiver readings.  ADEVs are a way of
//   characterizing how stable a signal is over various time intervals.
//   Heather supports simultaneous ADEV, MDEV, HDEV, and TDEV calculation.
//
//   If the main "receiver" type is a time interval counter (like the TAPR
//   TICC) Heather can also calculate and display MTIE (Maximum Time
//   Interval Error).  In this section, MTIE can be considered to be just
//   another xDEV type,  but xDEVs and MTIE cannot be shown on the screen
//   at the same time.
//
//   The AI command enables the MTIE display.  Heather uses Bregni's algorithm
//   to calculate MTIE.  Bregni's algorithm is very fast but limits the
//   intervals to powers of 2.  The maximum time interval used is the largest
//   power of 2 <= the adev queue size value.  MTIE CALCULATION STOPS
//   WHENEVER THE MAXIMUM TIME INTERVAL IS REACHED.  The CI command will
//   clear the MTIE data and start over.
//
//
//   --------------------------  WARNING --------------------------
//
//   Unless you are using a time interval counter (like the TAPR TICC)
//   the ADEV values are based upon the PPS and OSCillator
//   error values reported by the unit.  These values (aka bogo-adevs) are
//   not derived from measurements made against an independent reference
//   (other than the GPS signal) and may not agree with ADEVs calculated
//   by measurements against an external reference signal (true adevs).
//
//   Bogo-adevs tend to diverge more from true adevs at shorter taus (say
//   less than 30 seconds) and somewhat less at longer taus.  Values at
//   the middle of the tau range can be quite close to true adevs.
//
//   ADEVs calculated when the oscillator is undisciplined are more
//   meaningful than when the oscillator is being disciplined.  For proper
//   true ADEV values, you should use a proper external Time Interval Counter.
//
//   Not all devices report values that are even suitable for ADEV
//   calculations... Mostly the Trimble GPSDO devices are usable.
//
//   If the ADEV values were calculated from receiver data, the ADEV
//   type in the table headers is shown in lower case.  ADEVs derived from
//   time interval counters are shown in upper case.
//
//   If the ADEVQ: queue size message on the screen is shown in GREY, that
//   means that the receiver potentially has data that ADEVs can be calulated
//   on, but that data is not time-related and that ADEV calculations are
//   blocked.  You can un-block them by explcitly setting the adev queue
//   size with the /a= command line option.
//
//   ---------------------------------------------------------------
//
//
//   SELECTING THE ADEV/MTIE TYPE TO DISPLAY
//
//   Heather maintains a separate circular data buffer of the values that
//   it uses to calculates ADEVs from.  The queue contains a PPS related
//   value and an OSCillator related value.  The size of the ADEV queue
//   determines the maximum "tau" time interval that the ADEVs can show.
//   The default is 33,000 points which is suitable for values of TAU out
//   to around 10,000 seconds.
//
//   You can set the size of the ADEV (and MTIE) queue from the command line:
//      /a=size - sets the number of points to save in the ADEV queue.
//                A size of 0 will disable ADEV calculations.  Every 10000
//                ADEV queue entries uses around 1.0 megabytes of memory.
//                If MTIE data is being used (with a time interval counter)
//                this increases to around 7.5 megabytes.
//
//   Normally Heather shows the xDEV queue size on the screen in WHITE.  If
//   the device does not support xDEVs and the xDEV queue memory has not
//   been allocated it will be shown in GREY.
//
//
//   Normally Heather collects a new ADEV entry every second.  You can adjust
//   the ADEV interval with:
//      /j=secs  - sets all ADEV queue sample intervals
//      /ja=secs - sets chA (PPS) ADEV queue sample interval
//      /jb=secs - sets chB (OSC) ADEV queue sample interval
//      /jc=secs - sets chc ADEV queue sample interval
//      /jd=secs - sets chd ADEV queue sample interval
//      /jo=secs - sets chB (OSC) ADEV queue sample interval
//      /jp=secs - sets chA (PPS) ADEV queue sample interval
//
//
//   You can clear the ADEV queue from the keyboard:
//      CA - clear the ADEV and MTIE queues
//      CB - clear the ADEV and plot queue
//      CI - clear the MTIE data queue
//
//   You can clear the ADEV or MTIE queue and reload it from the porttion
//   of the plot queue currently being shown on the screen:
//      CR - reload the ADEV queue from the plot queue data being shown.
//   When using the TAPR TICC (or other) time interval counter this
//   command also reloads the MTIE data from the plot queue TIE points
//   displayed. Note that the CR command can produce errors if the device
//   is actively sending data to Heather.
//
//   The CR command lets you examine the ADEVs and MTIE data of a selected
//   portion of the captured data.  Set the plot view interval to the length
//   of the data segement you want to analyze (V command).  Use the plot
//   scrolling commands to move the first point of the data to be analyzed
//   to the left edge of the plot area then issue the CR command.  Note that
//   unless plot updates are disabled (U command) new incomming data will be
//   added to the ADEV and MTIE data queues.
//
//
//
//   Heather can also read ADEV information from files using the "R"
//   keyboard command.  ADEV files must have an extension of .ADV
//   These files can have three independent values per line.  The first value
//   if the "PPS/chA" value, the second one is the "OSC/chB" value, the third
//   value is the TAPR TICC channel C value (and is mostly useless)  These
//   values don't have to be actual PPS and OSC values, but that is how
//   Heather refers to them in the menus.
//
//   The .ADV files can also can contain:
//      # text comments
//      #title: title text (note that the ': ' is required)
//      #period seconds_between_readings (default=1)
//      #scale  pps_scale_factor  osc_scale_factor
//
//   The first line of an ADV file must be a '#' line.
//
//   The scale factor value multiplied by the data values should yield
//   nanoseconds.
//
//   Note that the #period command erases any data values that appeared
//   before it in the file.
//
//   Reading an ADEV file pauses the processing of data from the receiver.
//   THe "U" keyboard command can resume receiver updates.
//
//
//   Heather can also read a time interval error (TIE) value file for
//   calculating MTIE.  TIE files must have an extension of .TIE
//   A TIE file consists of lines of up to  4 channels of TIE
//   data (chA chB chC and chD) per line.  Comment lines start with a '#'.
//
//
//
//   ADEV INFORMATION DISPLAY
//
//   Heather displays ADEV info in two ways.  First is a table of ADEV values.
//   Second is as graphs in the plot area.  If a time interval counter (like
//   the TAPR TICC) Heather can also display MTIE values.  (MTIE is Maximum
//   Time Interval Error, a common measurement used in the telecom industry).
//
//      GA  - from the keyboard toggles the ADEV plots on and off
//      /ga - from the command line
//
//   If the ADEV plot is enabled, then the division markers in the plot area
//   that represent decades are highlighted in CYAN.  Each highlighted
//   vertical division represents a power of 10.  Each minor division marker
//   is a linear division of that decade.  The decade value of the top line of
//   the plot area is determined from the largest ADEV value seen in any
//   of the ADEV tables.  All ADEV types are scaled the same.  When ADEV plots
//   are being displayed the TAU values are labeled at the top of the plot
//   and the decade values are labeled at the right side.
//
//   Each highlighted horizontal division represents TAU with divisions of
//   powers of 10 seconds (TAU 1,10,100,1000,10000,,..)  The horizontal major
//   divisions between the highlighted ones are a 1:2:5 division of
//   that time decade (like 1,2,5,10,20,50,100...).
//
//   Heather's ADEV plots differ from conventional ADEV plots.  Conventional
//   ADEV plots are drawn with a log-log scale.  Heather's plots use a linear
//   scale (this is necessary because Heather's data plots share the same plot
//   grid).  The vertical scale is logarithmic by decade, but linear within
//   the decade.
//
//   For screen resolutions 800x600 and below,  the ADEV plot decades
//   are scaled to single VERT_MAJOR divisions (not to every two major
//   divisions)
//
//
//   The ADEV table is always shown on the screen if there is space for it
//   (in the upper right corner of the screen).  The "GB" keyboard command
//   allows for one of the satellite maps or the watch display to be shown
//   in the plot area.
//
//   If more than one of the map displays and/or watch display is enabled
//   they take precedence over the ADEV table display.
//
//   Clicking on the adev table display will switch the screen to the
//   "all adevs" mode.  The screen will be configured to display all four
//   adev types of the adev table that you clicked on.
//   Cicking again will restore the screen to its previous state.
//   Note that short clicks might be ignored... particularly on devices
//   like UCCM receivers that only output time updates every two seconds.
//   It can help to hold down the mouse button until the screen changes.
//
//
//   If the adev tables have more adev bins calculated than will fit on
//   the screen,  you can scroll through them.  See the PLOT WINDOW section
//   above for information on scrolling through the ADEV tables.
//
//
//   Heather always calculates all four ADEV types for both the PPS and OSC
//   values (and the TAPR TICC in TIMELAB mode, the channel C data).
//   The "A" keyboard menu is used to configure the ADEV information to
//   display:
//      AA - Show ADEVs for the OSC and PPS values
//      AH - Show HDEVs for the OSC and PPS values
//      AI - Show MTIEs for the TICC chA and chB values (the MTIE display is
//           only available if using a frequency/time interval counter)
//      AM - Show MDEVs for the OSC and PPS values
//      AT - Show TDEVs for the OSC and PPS values
//      AP - Show all ADEV types for the PPS / channel A value
//      AO - Show all ADEV types for the OSC / channel B value
//      AC - Show all ADEV types for the TAPR TICC channel C value
//      AD - Show all ADEV types for the TAPR TICC channel D value
//
//   You can also select the ADEV type(S) from the command line:
//      /oa - Show ADEVs for the OSC and PPS values
//      /oh - Show HDEVs for the OSC and PPS values
//      /oi - Show MTIEs for the TICC chA and chB values
//      /om - Show MDEVs for the OSC and PPS values
//      /ot - Show TDEVs for the OSC and PPS values
//      /oo - Show all ADEV types for the OSC value
//      /op - Show all ADEV types for the PPS value
//      /oc - Show all ADEV types for the TAPR TICC channel C data (currently useless)
//      /od - Show all ADEV types for the TAPR TICC channel D data (currently useless)
//
//      The "all adev" commands (AP / AO / AC / AD) let you
//      control what info to plot.  All four ADEV plots
//      only, all four ADEV types and the graphs (can be a
//      bit confusing since the same colors are used for
//      two different things),  or the two regular (PPS/OSC or CHA/CHB)
//      ADEV plots and graphs:
//         AOA - show all four OSC ADEVs tables and all ADEV plots
//         APA - show all four PPS ADEVs tables and all ADEV plots
//         ACA - show all four channel C ADEVs tables and all ADEV plots
//         ADA - show all four channel D ADEVs tables and all ADEV plots
//
//         AOG - show all four OSC ADEVs tables and all plots
//         APG - show all four PPS ADEVs tables and all plots
//         ACG - show all four channel C ADEVs tables and all plots
//         ADG - show all four channel D ADEVs tables and all plots
//
//         AOR - show all four OSC ADEVs tables and regular ADEV plots
//         APR - show all four PPS ADEVs tables and regular ADEV plots
//         ACR - show all four channel C ADEVs tables and regular ADEV plots
//         ADR - show all four channel D ADEVs tables and regular ADEV plots
//
//
//      Sometimes the scaling of one (usually TDEV) or more of the xDEV/MTIE
//      plots will interfere with the display of the plots of the other
//      types.  You can selectively turn off (or on) the plots of one
//      or more of the data types with the AX keyboard command or the /ax
//      command line option:
//         AXA  - toggle display of ADEV plots
//         AXH  - toggle display of HDEV plots
//         AXM  - toggle display of MDEV plots
//         AXT  - toggle display of TDEV plots
//         AXP  - toggle display of PPS/chA xDEV/MTIE plots
//         AXO  - toggle display of OSC/chB xDEV/MTIE plots
//         AXC  - toggle display of chC xDEV/MTIE plots
//         AXD  - toggle display of chD xDEV/MTIE plots
//
//         /axa - toggle display of ADEV plots
//         /axh - toggle display of HDEV plots
//         /axm - toggle display of MDEV plots
//         /axt - toggle display of TDEV plots
//         /axp - toggle display of PPS/chA xDEV/MTIE plots
//         /axo - toggle display of OSC/chB xDEV/MTIE plots
//         /axc - toggle display of chC xDEV/MTIE plots
//         /axd - toggle display of chD xDEV/MTIE plots
//         /ax  - enable display of plots all xDEV/MTIE types
//
//
//      Heather defaults to scaling the ADEV bins into a 1-2-5 sequence.
//      The AS keyboard command or /as= command line option let you modify
//      the bin scaling sequence.
//         /as=0  all adevs (next tau = tau + 1)
//         /as=1  one bin per decade
//         /as=2  one bin per octave
//         /as=3  3dB per decade (displayed tau is rounded to an integer)
//         /as=4  1-2-4 decades
//         /as=5  1-2-5 decades
//         /as=8  1-2-4-8 decades
//         /as=10 10 bins per decade
//         /as=29 29 (linearly spaced) per decade
//         /as=99 logarithmic linearly spaced bins
//
//      Heather is limited to calculating a maximum of 100 bins.  The number
//      of ADEV bins that can be plotted or shown on the screen is limited
//      by the amount of screen space available.  The full adev info is
//      written to the log file whenever it is closed or Heather exits.
//      You can scroll through the adev tables and plots as mentioned above.
//
//      Note that changing the bin scaling sequence causes Heather to
//      recalculate the adevs from the adev queue data.  If the adev queue
//      had overflowed the new adev calculations and numbers will not use
//      data that had been accumulated before the queue had overflowed.
//
//      Recalulating the adev values (either due to manually changing the
//      bin scaling sequence or automatically due to an adev queue overflow)
//      can take a long time on slower machines.  For instance recalculating
//      all 4 adev types on 36,000 adev queue entries on a Raspberry PI3
//      can take around 25 seconds).  When recalculating the adevs, the
//      keyboard will not respond.
//
//
//      The AE keyboard command or /ae= command line option toggles the
//      display of ADEV error bars in the plots.  Error bars that are shown
//      with arrow points on the ends indicate an adev bin with not enough
//      data to produce a meaningful error estimate.  Error bars smaller than
//      one minor plot division are not shown.
//
//
//
//
//-USING HEATHER WITH FREQUENCY AND TIME INTERVAL COUNTERS
//
//   Heather can be used with frequency and time interval counters in two
//   different ways:
//
//     1) As the primary "receiver" device
//     2) As an auxiliary (extra) input device.  When used as an auxiliary
//        device the counter readings are used to calculate true ADEV values
//        (instead of the "bogo-adevs" calculated from the receiver
//         self-reported disciplining loop parameters).
//
//    To use a counter as the primary "receiver" device use:
//       /rxi - input device is a counter
//       /rx1 - input device is an HP-531xx counter.
//              /rxi will usually work with the HP counters, but /rx1 can be
//              used to force the issue.
//
//    Heather does a pretty good job for determining the counter type from
//    the data that it see, but you can also use the following commands to
//    force the counter type when /rx1 or /rxi has been specified:
//       /itc - force generic counter mode
//       /ith - force HP counter mode
//       /itl - force Lars GPSDO controller counter mode
//       /itp - force PICPET counter mode
//       /itt - force TAPR TICC counter mode
//
//    To use a counter as an auxiliary input device, see the section on
//    the /ei command command.
//
//    If the primary input device is a counter, the /ei command allows you
//    to connect a second counter to add more channels of data (two TAPR
//    TICCs allows four channels of data to be analyzed).
//
//
//    When using Heather with a counter as the primary input device, Heather
//    enters ADEV mode and displays ADEV/HDEV/MDEV/TDEV and/or MTIE of the
//    input signals.  It has most of the functionality of the popular
//    TIMELAB progam, but none of the cool slickness... but it does run under
//    Linux, macOS, and FreeBSD.  See the section of ADEVs for info on
//    configuring and controlling the ADEV displays.  TAPR TICC data is best
//    displayed on screens with a width of 1280 pixels.
//
//
//    For generic counters, Heather does not send commands to the counter or
//    change the counter configuration.  With the TAPR TICC there are several
//    commands that can configure the device.  Consult the TAPR TICC manual for
//    more info on these settings.
//       PM - set the data type mode
//       PE - set the input signal edge to use
//       PE - set the channel "fudge" factor settings
//       PT - set the channel "TIME2" factor settings
//       PS - set the channel "time dilation" settings
//       PS - set the device sync mode
//       PC - set the device calibration period
//       PR - set the device ref clock
//       PK - set the device coarse clock
//       &a - auto-tune TIME2 and fudge factor values
//
//    The user must configure the main and secondary counters with the same
//    settings and operating mode.  To configure the second TICC
//    start Heather with the second device's port specified as the
//    main input device, configure it, shut down Heather, then
//    restart Heather with the first TICC specified as the main
//    device and the second TICC as the extra device.
//
//
//
//    Heather needs to know what kind of data the counter is providing.
//    All counter channels must be using the same data type.
//       /imi - time interval data
//       /imf - frequency data
//       /imp - period data
//       /imt - timestamp data
//       /imd - TAPR TICC debug data
//       /iml - TAPR TICC Timelab data
//
//       PM from the keyboard also sets the counter data type.
//
//    Note that the TAPR TICC supplies data type information, but these
//    commands let you override the TICC info, but this will probably cause
//    errors.
//
//
//    Heather converts and proceees all counter readings into time intervals.
//    The data is displayed as Time Interval Errors (TIE data).  TIE is the
//    difference from the reading to the nominal frequency of the oscillator
//    being analyzed.
//
//    You should configure frequency counters with a 1 second gate time.
//    For the TAPR TICC, etc  use a divider to drive the counter with a
//    1 PPS input signal.
//
//
//    Heather needs to know the nominal frequency of the data source being
//    analyzed.  The nominal frequency of all channels defaults to 10 MHz.
//    Setting the nominal frequency also sets the phase unwrap interval used
//    when processing frequency data.
//    You can specify the nominal frequency of any channel like:
//       /if=5E6  - set all channels to a 5 MHz nominal frequency
//       /ifa=15E6  - set channel A to 15 MHz nominal frequency
//       /ifb=10E6  - set channel B to 10 MHz nominal frequency
//       /ifc=1E6   - set channel C to 1 MHz nominal frequency
//       /ifd=10000000  - set channel D to 10 MHz nominal frequency
//       /ifo=15E6  - set channel A to 15 MHz nominal frequency
//       /ifp=10E6  - set channel B to 10 MHz nominal frequency
//
//      PN from the keyboard also sets channel nominal frequencies.
//      The PN command is not available when using Symmetricom Rb
//      oscillators.
//
//
//  Heather can also display the TIE data as frequency error data:
//      GP# - Display channel A data as frequency errors
//      GO# - Display channel B data as frequency errors
//      G7# - Display channel C data as frequency errors
//      G8# - Display channel D data as frequency errors
//
//
//  Heather can also display phase data of the input signals:
//      G1  - Display channel A phase data
//      G2  - Display channel B phase data
//      G3  - Display channel C phase data
//      G4  - Display channel D phase data
//      GV  - Display phase data for all channels
//
//  Instead of raw phase data, Heather can display phase data residuals of
//  the input signals.  Phase resiuals are the least squares inear trend line
//  of the phase data subtracted from the phase data.
//      G1=  - Display channel A phase residuals
//      G2=  - Display channel B phase residuals
//      G3=  - Display channel C phase residuals
//      G4=  - Display channel D phase residuals
//
//
//  When operating on frequency data, Heather needs to "unwrap" phase data.
//  To do this, it needs to know what phase wrap interval to use.  Heather
//  normally bases this upon the nominal frequency setting using the formula:
//  (1.0 / nominal_frequency)... the actual value used to unwrap phase data is
//  half that value.  You can override this setting using:
//      /pwa=seconds   - for channel A data
//      /pwb=seconds   - for channel B data
//      /pwc=seconds   - for channel C data
//      /pwd=seconds   - for channel D data
//      /pwp=seconds   - for channel A data
//      /pwp=seconds   - for channel B data
//      /pw=seconds    - for all channels
//      PW             - from the keyboard.  You can specify up to 4 phase
//                       wrap intervals for CHA, CHB, CHC, and CHD separated
//                       by spaces.  The PW command is not available when using
//                       SRO100 rubidium oscillators or STAR4 GPSDOs.
//
//      Values of 0 seconds says to disable phase un-wrapping.
//      Negative values say to use the nominal frequency to set the phase
//      wrap interval.
//
//
//    Timestamping mode counters have a limited range that the timestamps
//    cover.  After this amount of time, the timetamps "wrap".  Heather
//    normally reduces all timestamps modulo 100 seconds.  This value is
//    good for most applications.  You can force the timestamp wrap interval
//    to any value.  All channels use the same timestamp wrap setting:
//       /twi=seconds
//       PI  - from the keyboard
//
//    The PICPET timestamp counter chip has a rather coarse resolution (100 ns
//    with a 10 MHz clock).  This can produce rather noisy plots.  Try setting
//    a long-ish display filter with the FD keyboard command... like FD 60)
//
//    The Lars GPSDO is a simple GPSDO controller that can work with any
//    GPS device that outputs a 1PPS signal.  It disciplines an external
//    10 MHz oscillator.  You can use the Lars GPSDO either as a TIC (in
//    period mode) as the main input device or you can use it as an "extra"
//    input device. In "extra" mode you use the GPS receiver as the main input
//    device and the Lars GPSDO data as the "extra" input device.  The Lars
//    GPSDO reports the PPS error deviation from 1Hz (i.e. time interval
//    error), the GPSDO DAC setiing, and the GPSDO temperature.  If the
//    receiver does not report one of these values, the Lars GPSDO data is
//    shown.  If the GPS receiver sends one of those values, the GPS receiver
//    data is shown.  The Lars DAC setting is labeled "DACL",  the Lars
//    temperature value is labeled "TEMPL", and the PPS value is labeled "PPSL".
//    Note that the xDEVs from the Lars GPSDO are calculated from the GPSDO
//    control loop values and are not referenced to an external reference.
//    They are what Heather refers to as "bogo adevs".
//
//
//
//
//-MISCELLANEOUS RECEIVER CONTROL COMMANDS
//
//
//   RESTARTING and RESETTING the RECEIVER
//
//      !W  - warm start the receiver
//      !C  - cold start the receiver
//      !H  - hard reset the receiver to factory defaults
//
//            Note that on many receivers doing warm/cold/factory resets may
//            take a long time and Heather may take a long time to
//            re-establish communications or Heather may not be able to
//            re-establish communications, or may change the receiver language
//            type.  You may need to exit and restart Heather.
//
//
//
//   SERIAL PORT CONTROL
//      !S  - re-initializes the serial port connection and receiver data
//            parser
//
//      !Z  - reset the receiver message data parser
//
//      !P  - set operating system serial port parameters (like the /br
//            command line option)  Note this changes the operating system
//            serial port configuration and NOT the receiver serial port
//            parameters.
//
//            In order to guard against accidentally "bricking" a receiver
//            by setting a hard-to-recover-from configuration,  Heather
//            does not ever change the receiver serial port configuration.
//            (someday this command might be able to configure the receiver
//             serial port parameters)
//
//      !B  - send a 300-500 msec BREAK to the serial port
//
//
//
//   EDITING FILES
//       N  - pressing N on the keyboard brings up WORDPAD (NANO on
//            Linux and macOS_ to edit/view a file.  The default file
//            is heather.cfg
//
//
//   SENDING COMMANDS to RECEIVER
//      !U  - sends a command string to the receiver.  This is mainly for
//            NMEA, SCPI, and other  receivers that accept ASCII commands.
//            For NMEA receivers you do not need to include the leading '$'
//            character.  Also the NMEA checksum will be automatically
//            added if it is not included (no '*' in the command).
//
//            On the Lucent RFTG-m GPSDO this command disables the unit and
//            causes the other unit to assume control.  Use !R to re-enable
//            the unit.
//
//
//   RUNNING RECEIVER DIAGNOSTICS
//      !D  - command the receiver to run its internal self-tests.  Note that
//            on many devices this takes a long time and/or resets the receiver
//            and Heather may take a long time (or forever) to re-establish
//            communications.
//
//
//   SWITCHING RECEIVER MESSAGE FORMATS
//      !M  - If a receiver can speak NMEA, but is currently running in its
//            native binary mode,  this will switch the receiver back to NMEA.
//            On receivers that are speaking Motorola binary data, this command
//            usually switches the receiver from Motorola mode to some other
//            (device dependent) language.
//
//
//   CHANGING THE RECEIVER NAVIGATION / MESSAGE OUTPUT RATE
//      /nr=Hz - from the command line
//      !R=Hz  - many modern receivers can output data at greater than 1 Hz.
//               This command lets you change the "navigation rate" of the
//               receiver.
//
//               Note that most timing receivers cannot provide an accurate
//               1 PPS signal unless they are running at 1 Hz navigation rate.
//
//               High navigation rates produce a lot of data and serial port
//               traffic.  If the receiver serial port is set to a low speed,
//               they usually drop messages to lower the data rate. Some
//               receivers just go bonkers.  This condition usually shows up
//               as lots of time-skip indications in the plot area and/or
//               jittery watch or digital clock displays.
//
//               Sometimes a high navigation rate produces enough serial port
//               traffic that Heather never sees an idle time in the data
//               stream where it can process keyboard commands.  The keyboard
//               will stop responding.  You can recover from this by killing
//               the program and restarting it with the /nr=1 command line
//               option to lower the navigation rate back to 1 Hz.
//
//               Also with high navigation rates and plot auto-show mode (the
//               VT keyboard command) you might not be able to exit the
//               program from the keyboard (particularly with a lot of data
//               in the plot queue).  You can exit the program by clicking on
//               the exit button in the title bar or turning off VT mode (try
//               V0) before exiting.
//
//
//
//   TERMINAL EMULATOR
//
//      Heather has a built in terminal emulator program that is handy
//      for monitoring data from the receiver and sending ASCII commands to it.
//      Receiver data is shown in GREEN and user keystrokes are shown in
//      YELLOW.
//
//         !T  - enter terminal emulator from the keyboard
//         /bt - start Heather in terminal emulator mode from the command line
//
//      If you use /rxx (no receiver type, use system clock) along with /bt
//      Heather will work as a simple terminal emulator.  The program will
//      exit when you press the END key.
//
//      You can also use the /rxte (or /rxtt) command line option to operate
//      heather as a terminal emulator program.  You can set the port
//      configuration with /rxte=...   where ... is the device desciptor
//      string as documented for the external device commands ("/ei" command).
//
//      Normally the terminal emulator sends keystrokes immediately when a
//      key is pressed.  Use /rxtb for a stand-alone terminal emulator with
//      a buffered keyboard that only sends keystrokes when CR is pressed.
//      This is needed for the OSA-453x devices with firmware that times
//      out after 175 msecs betweem keystrokes (who was the bastard that
//      thought that was a good idea?)
//
//      The terminal emulator mode has several keyboard commands for
//      controlling it:
//
//         END    - exit terminal emulator
//         HOME   - erase the screen
//         UP     - re-sends the last keyboard line to the receiver
//         F1     - toggle keystroke echo mode
//         F2     - toggle writing receiver data to a receiver data capture
//                  capture file.  Heather uses the "raw data capture log"
//                  file name. If the data capture file is open, it is closed.
//                  If it not open, it is opened in "append, binary" mode.
//         F3     - toggle HEX binary data display mode.  In HEX mode
//                  bytes that could be part of a start/end-of-message
//                  sequence are shown in YELLOW
//         F4     - toggle writing to the screen
//         F5     - CR-LF mode - adds a CR after any LF or add an LF after
//                  any CR (this option does not show up the terminal emulator
//                  help info header!)
//         F8     - send a BREAK
//
//
//     The !O keyboard command brings up a serial port data monitor screen.
//     This shows all data sent to or from a given serial port.  Pressing any
//     key will exit the monitor mode.
//
//     The ZO command also brings up a data monitor screen for the main
//     input device. Received message data bytes are shown in GREEN and
//     sent messagee bytes are shown in YELLOW.
//
//
//
//   RPN CALCULATOR
//
//      Heather has a built-in RPN calculator mode.  RPN calculators are
//      based upon a postfix stack based architecture as popularized by
//      the Hewlett Packard scientific calculators.  Heather's calculator
//      defaults to a six level stack.  The first entry on the stack is
//      usually called "X" and the next entry is "Y".  Heather also implements
//      100 memory registers (0 .. 99).
//
//      Calculator mode is entered with the "`" (backwards quote) key.
//      Multiple values and operations can be entered on a line.
//      To exit calculator mode enter a blank line or ESC when the calculator
//      prompt is being shown  or enter a '`' (backward quote) or ESC when the
//      calculator prompt is not being shown.
//
//      The calculator is also available in a full screen zoomed mode via
//      the Z` keyboard command.
//
//      Heather can also be run in "calculator only" mode by starting it
//      with the /rxca command line option.  The only way to exit
//      "calculator only" mode is with the EXIT calculator command or by
//      clicking on the exit icon in the program title bar.
//
//      The current stack values are shown in the plot area.  The "X" value
//      is at the bottom of the list).  After exiting calculator mode
//      the GR keyboard command (redraw screen) will erase the stack
//      display from the plot area.
//
//      Operations marked with an '*' work with angles. The input and output
//      values will be in either degrees or radians (depending upon the
//      current RAD or DEG setting).
//
//      Numbers can be in entered in either fixed point (3.1459) or exponent
//      (12.34e-5) format.  Hexadecimal integers can be entered like 0xABCD.
//      Binary integers can be entered like 0b1101.  Octal integers can be
//      entered like 0o123.   Note that binary/octal/hex numbers cannot be
//      signed.  Use the CHS command to make them negative.
//
//      As an example a calculator command line:
//         45 sin 2 * sqr
//      Will results in a X value of 2.0000:  calculates sin(45), multiples
//      the value (0.707...) by 2, then squares the result.
//
//
//      Basic math operations:
//         +    - add X and Y
//         -    - subtract X from Y
//         *    - multiply X and Y
//         /    - divide Y by X
//         inv  - 1.0 / X
//         mod  - fmod(Y,X)
//         max  - maximum value of X and Y
//         min  - minimum value of X and Y
//         chs  - change sign of X
//         abs  - absolute value of X
//         int  - integer part of X
//         frac - fractional part of X
//         nop  - no operation
//
//         ctof - centigrade to farenheit
//         ftoc - farenheit to centegrade
//         ftom - feet to meters
//         mtof - meters to feet
//         rn   - resistor noise in nV/sqrt(Hz)  (input Y=degrees C   X=ohms)
//
//      Logical operations (on 64 bit integers, but note that the numbers
//      are processed via double precision values so only the lower 54 bits
//      may be valid due to the double precision exponent bits).
//         ~    - complment X
//         &    - AND X and Y
//         |    - OR X and Y
//         ^    - XOR X and Y
//         >>#  - shift X right #places (1 place if # not given)
//         <<#  - shift X left #places (1 place if # not given)
//
//      Trig functions:
//         deg   - treat/display values as degrees
//         rad   - treat/display values as radians
//         rtod  - radians to degrees (X * 180.0 / PI)
//         dtor  - degrees to degrees (X * PI / 180.0)
//         sin   * sine(x)
//         cos   * cosine(x)
//         tan   * tangent(x)
//         asin  * arcsine(x)
//         acos  * arccosine(x)
//         atan  * arctangent(x)
//         atan2 * atan2(Y,X)
//         rtop  * rectangular to polar (theta->Y, r->X)
//         ptor  * polar to rectangular (Y input is theta, X is R)
//         dms   * break up angle in X to degrees minutes seconds
//         dec   * convert degrees minutes seconds (Z Y X) to decimal
//         dist  - linear distance between two points (X Y and Z R)
//         diag  - diagonal distance - sqrt(X*X + Y*Y);
//         gcd   * great circle distance and azimuth between two locations
//                 (X=LON1 Y=LAT1 [destination])   (Z=LON2 R=LAT2 [source])
//         bearing * azimuth between two locations (X=LON1 Y=LAT1 [destination])
//                   (Z=LON2 R=LAT2 [source])
//
//      Powers and logarithms:
//         sqrt  - square root X
//         sqr   - X*X
//         ln    - ln(X)
//         log   - log base 10 (X)
//         exp   - e^X
//         powe  - e^X
//         pow10 - 10.0^X
//         pow   - Y^X
//         **    - Y^X
//
//   Time and date maniipulation:
//         date      - put display date (yyyy mm dd onto stack Z Y X)
//         time      - put display time (hh mm ss onto stack Z Y X)
//         greg      - convert X (JD value) to gregorian onto the stack)
//         local     - local time as a Julian date
//         utc       - UTC time as a Julian date
//         gps       - GPS time as a Julian date
//         mjd       - UTC time as a Modified Julian Date
//         gtime     - time since GPS epoch (in days)
//         utime     - time since Unix epoch (in days)
//         tzjd      - time zone offset as a Julian time (fraction of a day)
//         tz        - time zone offset in hours
//         leap      - current leapsecond count (UTC offset)
//         week      - gps week
//         tow       - gps time of week
//         epoch     - gps epoch as a julian date
//         secs      - convert hours minutes seconds (Z Y X) to seconds
//         dow       - convert hours min secs (Z Y X) to day of week (0..6)
//         ti        - stopwatch timer value (TI keyboard command value)
//
//         rise      - sun/moon rise time as a Julian date (local time)
//         noon      - sun/moon noon/moon transit as a Julian date (local time)
//         set       - sun/moon set time as a Julian date (local time)
//                       The rise/noon/set times will be for either the sun
//                       or moon... depending upon how the main screen is
//                       configured (with the TR M keyboard command).
//
//      Display formatting: # is the number of decimal point digits. If #
//      is not given, use the current setting.  Note that there is no space
//      before the #.
//         fix#   - fixed point notation (unless value is too large)
//         sci#   - scientific notation (mantissa and exponent)
//         exp#   - engineering notation (exponents are a multiple of 3)
//         hex#   - ASCII hex format (integer only!)
//         oct#   - ASCII octal format (integer only!)
//         bin#   - ASCII octal format (integer only!)
//         comma# - toggle formatting the integer part with "comma" every 4
//                  digits.  # is the option "comma" character.  If # is
//                  a '.' then also replace the decimal point with
//                  a ',' (european style mumbers)
//         zoom   - switch to zoomed calculator screen mode
//
//      Stack manipulation:
//         ex    - exchange X and Y value
//         ent   - push X onto stack
//         enter - push X onto stack
//         clx   - clear X value
//         clear - clear stack and memory registers
//         cls   - clear stack
//         clm   - clear memory registers
//
//         roll   - rotate the stack down 1 position
//         roll#  - rotate the stack down # positions
//         down   - rotate the stack down 1 position  (same as roll command)
//         down#  - rotate the stack down # positions (same as roll command)
//         up     - rotate the stack up 1 position
//         up#    - rotate the stack up # positions
//         drop#  - remove # entries from the stack
//         stack# - push stack entry# onto the stack
//         swap#  - swap X with stack entry#
//
//      Memory operations:  # is the register to use (0..99).  If # is not
//      given, register 0 is used.
//         rcl#  - push reg # onto the stack
//         rcl+# - add reg # to X
//         rcl-# - subtract reg # from X
//         rcl*# - multiple X by reg #
//         rcl/# - divide X by reg #
//         rcl@# - push reg[reg[#]] onto the stack (indirect recall)
//
//         sto#  - store X in reg #
//         sto+# - add X to reg #
//         sto-# - subtract X from reg #
//         sto*# - multiply reg # by X
//         sto/# - divide reg # by X
//         sto@# - store X in reg[reg[#]]  (indirect store)
//
//      Constants and values:  These commands push the indicated value onto
//      the stack.
//         last      - the previous X value
//         lastx     - the previous X value
//         pi        - 3.14159...
//         \         - 3.14159...
//         e         - 2.81828...
//         c         - speed of light in meters per second
//         h         - Plank's constant (6.62607015E-34)
//         k         - Boltzmann's constant (1.380649E-23)
//         t0        - Absolute zero (-273.15)
//         spd       - seconds per day (24*24*60)
//         spw       - seconds per week (24*24*60*7)
//
//         lat       * current latitude
//         lon       * current longitude
//         alt       * current altitude
//         lla       * current lat, lon, alt
//
//         dac       - current DAC/Sawtooth setting
//         temp      - current temperature setting
//         pps       - current PPS plot value
//         osc       - current OSC plot value
//
//         cable     - current cable delay value in ns
//         elmask    * current elevation mask filter setting
//         amu       - current signal level filter setting
//
//         dop       - average DOP value
//         pdop      - current PDOP
//         hdop      - HDOP
//         vdop      - HDOP
//         gdop      - GDOP
//         tdop      - TDOP
//         edop      - EDOP
//         xdop      - XDOP
//         ydop      - YDOP
//         tfom      - time figure of merit
//         ffom      - frequency figure of merit
//
//         tc        - GPSDO time constant
//         damp      - GPSDO damping factor
//         gain      - GPSDO oscillator gain in Hz/Volt
//         initv     - GPSDO initial voltage
//
//
//   For these values, # indicates the desired satellite.  Note that there
//   is no space before the #. If # is not given then the highest elevation
//   satellite is used.  #=1000 is the SUN, #1001 is the MOON
//         az#       * azimuth of satellite PRN #
//         el#       * elevation of satellite PRN #
//         sig#      - signal level of satellite PRN #
//         doppler#  - doppler of satellite PRN #
//         range#    - pseudorange of satellite PRN #
//         phase#    - code or carrier phase of satellite PRN #
//         prn#      - place all sat measurements on the stack
//
//
//   Conditionals operations: If the comparison is true the next operation on
//   the line is processed and the second operation is ignored.  If the
//   comparison is false the next operation is ignored and the second
//   operation is processed.  This is similar to the "compare ? true : false"
//   operation in the C language.
//         x=y
//         x<>y
//         x<y
//         x>y
//         x>=y
//         x<=y
//
//         x=#   - (# is a numeric constant, 0 if not given)
//         x<>#
//         x<#
//         x>#
//         x>=#
//         x<=#
//
//         dsz   - decrement x and skip next instruntion if 0
//         isz   - increment x and skip next instruntion if 0
//
//         nop   - no operation
//         break - skip the rest of the line
//         again - loop back to the start of the line
//
//   User defined operations:
//      You can create user defined opeations using the DEFINE command:
//          define name op1 op2 ... opn
//
//      Once a command is DEFINEd you can invoke it by name just like any
//      built in command.  DEFINE names must start with a letter.  You
//      can DEFINE up to 100 new commands.
//
//      You can re-DEFINE built-in operations that start with a letter (but
//      this is not generally a good idea).
//
//      You can also edit user defined operations:
//          edit name
//      or you can simply re-DEFINE an existing DEFINE.  If you don't specify
//      any string after the define name, the DEFINE is deleted.
//
//      User defined operations can use other user defined operations.  The
//      maximum execution nesting level is 10.
//
//      You can save all your DEFINEd operations with the "savedefs" command.
//      The file extension should be .rpn if you want to re-load it using
//      the "R" keyboard command.  Or the calculator "run" command can also
//      be used ("run" accepts any file name extension).
//         savedefs filename.rpn
//
//      You can save all DEFINEs and the current memory and stack using
//      the "saveall filename" command.
//
//      At startup, Heather can read calculator commands from the file
//      "heather.rpn"  The entries in the file should start in column 1.
//      The file can contain any calculator commands, but is mainly
//      intended to load user DEFINEd calculator commands.
//
//      You can read in calculator RPN files using the "R" keyboard
//      command (file extension must be .rpn) or with the calculator
//      "run" command.  Lines that begin with a blank or tab are
//      ignored.  This feature lets you use the .RPN files to implement
//      more complex functions than will fit on a single line.
//
//      The "again" operation should only be used after a conditional
//      comparison that you KNOW can be reached.  Otherwise the calculator
//      will loop forever!   You can break out of a "hung" AGAIN loop by
//      pressing any key.
//
//
//
//   EXECUTING AN EXTERNAL PROGRAM
//      !E  - This command prompts for a command line for the operating
//            system to execute.  It spwans a shell to run the specified
//            program,  For this command to work under Linux/macOS/FreeBSD
//            the "xterm" program must be installed (it is part of most
//            Linux distros, but it is not part of the standard Raspberry Pi
//            distro, but can be installed).
//
//
//
//
//-ON SCREEN KEYBOARD and TOUCH SCREEN SUPPORT
//
//   Heather now supports an on-screen "touch screen" keyboard.  It is
//   accessed by touching or clicking the mouse in the upper left-hand corner
//   of the screen.  The touch screen must emulate a mouse for it to work.
//   Enabling of the on-screen keyboard may be toggled with the /kv
//   command line option.
//
//   The on-screen keyboard has a "CAP" key... this actually does a
//   shift-lock function, not a caps-lock function. The key label changes
//   lower case to upper case when caps-lock is on.
//
//   The on-screen keyboard also has a "MOU" key.  When on (key is labeled
//   in upper case) then whenever the mouse is in the plot area the left
//   mouse button is treated as being the right mouse button.  This
//   allows the plot scrolling function to be used with a touch screen.
//
//   If the file "heather_click.wav" exists in the heather installation
//   directory, it is played whenever an on-screen keyboard key is pressed
//   or the screen is clicked to zoom an item to full screen.
//
//   Note that for several small Raspberry PI SPI interfaced
//   touchscreens the touchscreen driver does not work
//   well (or at all).  It ignores touches!.  You can
//   improve this by using the /mb command line option.
//   This maps the SCROLL WHEEL and RIGHT buttons to be
//   the same as the LEFT button.  The touch response will
//   be rather slow... you have to hold the touch for 2-3
//   seconds before it will be recognized... sucks, but
//   such is life...
//
//
//
// SCREEN CLICK HOTSPOTS
//
//   In addition to the standard mouse clicks in the plot area actions,
//   clicking the mouse (or pressing a touch screen) can cause various other
//   actions.  These special "hot spots" are:
//
//   Upper left 100x100 pixels on the main screen (the time/date information)
//   brings up the touchscreen keyboard if it has been enabled (/kv command)
//
//   Upper left 100x100 pixels on a "zoomed" screen - exits the zooomed screen
//
//   The "receiver data" area at the top of the screen next to the time/date
//   info block - does an immediate screen image capture just like you pressed
//   the "\" key.   This also works on zoomed screen displays, even though
//   the "receiver data" is not being shown.
//
//   The LAT/LON/ALT table on the main screen - zooms the lat/lon/alt
//   scattergram to full screen.
//
//   The satellite info table - zooms the screen to the full satellite info
//   table,  satellite position map,  and satellite signal level map.  If the
//   satellite info table is not on the screen, clicking to the left of the
//   first digit of the digital clock will also do this.
//
//   The area under the satellite info table - zooms the digital clock to
//   full screen.
//
//   The digital clock - zooms the screen to the digital clock display.
//
//   The analog watch or satellite maps - zooms the clicked item to full
//   screen.
//
//   The plot area header text - zooms the plot to full screen.  If the plot
//   is zoomed to full screen,  clicking on the upper right 100x100 pixel of
//   the screen will simulate pressing the "DEL" key to get out of
//   "plot review" mode.
//
//   If the keyboard "Z" command has been used to enable a zoomed screen
//   display,  clicking on various items in the zoomed display will zoom that
//   item to full screen.  Clicking on items on a zoomed display that was
//   created by clicking on a main screen item will NOT zoom that item... it
//   will return to the main display screen.
//
//
//
//
//-MISCELLANEOUS COMMANDS
//
//   There are several commands that don't fall under any of the previous
//   sections.
//
//
//   Heather uses a color palette of 16 colors: 0..15   With 0=black and
//   15=bright white.  You can modify entries in the color palette map:
//      /cm=color,r,g,b    where color is the color number (0..15) to
//                         change and r,g, and b are the RED GREEN and
//                         BLUE values (0..255).
//
//
//
//
//   Whenever Heather is idle and not doing something or receiving data
//   from the GPS receiver, it does a short Sleep() that returns control
//   to the operating system and greatly reduces the CPU usage.  You can
//   specify how long to Sleep() for in milliseconds:
//      /tw[=#] - Sleep() for # milliseconds when idle (default=10)
//
//
//
//   You can control the audio functions of Lady Heather:
//      /kb     - toggle Beeps from the command line
//      /ks     - toggle playing Sound files from the command line
//      /rb     - toggle showing (in the plot title) reason code for beeps
//
//      GS      - toggle all sounds from the keyboard.  If beeps OR sound
//                files are enabled, they will both be disabled.  If beeps
//                AND sound files were disabled, they will both be enabled
//                and a BEEP will be sounded as confirmation.
//
//
//   Normally you exit Heather from the keyboard by pressing "ESC y".  You
//   can allow ESC ESC to exit the program with:
//      /ke     - toggle quick exit with ESC key
//
//
//   You can put Heather into a "read-only" mode that blocks sending commands
//   to the receiver over the serial port.  This mode can interfere with
//   functions or displays that require commands to be sent to the receiver:
//      /ir     - toggle read-only mode for receiver commands
//
//
//   This is similar to the "/ir" command, except it blocks sending ALL data
//   out the serial port:
//      /is     - toggle read-only mode for serial port
//
//
//   Normally Heather periodically polls the receiver for various pieces of
//   data,  status,  or configuration information.  Usually a new piece of
//   information is polled for every second.  It can take around 30 seconds
//   for Heather to acquire everything from the receiver.  You can block
//   Heather from polling the receiver for data.  This is mainly useful when
//   using the "TK" command to analyze receiver message timing.
//      /ix     - toggle no-polling mode for receiver data
//
//
//   Heather can output some debug information (mainly on Linux and macOS).
//   This debug information is independent of the debug log file and most
//   goes to stdout. This command line option sets the level of debug
//   information that is shown.
//      /de[=#] - set debug information level
//
//
//
//   The "R" keyboard command can read some other type of files. Many of these
//   file formats are also readable from the command line using the "/r="
//   command line option.
//
//      You can read lat/lon/alt logs.  These files
//      must have an extension of ".LLA"   Data lines require:
//        time-of-week  flag  lat  lon  alt
//        flag=0 means good reading,  otherwise skip it
//      This format can be generated by TBOLTMON log command.
//      Other .LLA file commands supported are:
//         # text comments
//         #title: Title text (note that the ': ' is required)
//         #lla: ref_lat  ref_lon  ref_alt (your assumed exact location)
//      The first line of an LLA file must be a '#' line.
//
//      You can play/test sound files.  These files must have an file name
//      extension of ".WAV"
//
//      You can read greetings calendar files.  These files must have an file
//      name extension of ".CAL"
//
//      Can also read ".TIM" files from John Miles' TI.EXE
//      program.  The .TIM file CAP/TIM/IMO, PER, and SCA commands are
//      used.
//
//      If you attempt to read a file that does not have a filename .EXTension
//      or end in a '.', Heather tries to read a .ADV, .CAL, .CFG, .GPX,
//      .KML, .LLA, .RAW, .SCR, .SIG, .TIM, .TIE, .WAV, and .XML files
//      in that order.
//
//
//   The "N" keyboard command brings up a text editor  The default editor
//   used is NOTEPAD in Windows, NANO on Linux and macOS, and EE on FreeBSD.
//   You can specify an editor to use by setting the "HEATHER_EDITOR" or
//   "EDITOR" environment variables.  You can view/edit any file... the
//   default is "heather.cfg"  To use this command on Linux/macOS/FreeBSD
//   the "xterm" program must be installed (it is not part of the standard
//   Raspberry Pi distro, but can be installed).
//
//   The /ne command line option disables the file editing and execution
//   commands.  Once /ne is used, it cannot be re-enabled.  This command is
//   a security measure to block possible malicious exploitation of these
//   features.  On Linux/macOS "/ne /ne" also disables playing of sound files
//   vis system() calls to APLAY.
//
//
//
//   A few legacy command line options... nothing to see here, move
//   along...
//
//      /lo     - enable reading of old format LUXOR log files
//      /lh     - don't write timestamp headers in the log file when the file
//                is periodically synced to the disk.
//
//   In WINDOWS, both the TCP/IP link and help dialog features use
//   a windows message timer to keep the program running
//   if the command line help dialog box is active or the screen is
//   being dragged, etc.  At one time, this timer had the potential to
//   cause very intermittent, random "unhandled exception"
//   aborts, but this seems to have been fixed.  You can still disable the
//   timer feature with the:
//      /kt     - toggle Windows dialog/message timer
//
//
//   The "/bs" command line option will adjust the displayed time by the
//   solar Equation of Time.  The only indication that solar time is in use
//   is on the analog watch display. The watch name/day of week will
//   be "Solar".  Using solar time is now best done with the "TZ" and "/tz"
//   time zone commands.
//
//
//   /nt  - attempt to wake up Nortel "telecom" GPSDOs  (this is now
//          done automatically)
//
//
//
//   You can change the input device on the fly
//   from the keyboard with the /# or /ip keyboard command
//   just like from the command line.
//   Changing the input device from the keyboard resets
//   the satellite tracking and signal level maps.
//   You can also use these commands to re-establish communications
//   to a dropped connection or device.  If you had
//   previously set an IP connection address/port, you
//   don't have to type the "=addr:portnum" after the /ip
//
//   You can also use the "/rx" commands from the keyboard to change
//   the input device type.  Note that Heather attempts to configure itself
//   for the new input device, but sometimes not all the settings and data
//   will be updated for the new device.  Changing the device on the fly
//   may not always work properly.
//
//
//
//   Added '$' and '=' command line/.CFG file  options for
//   building lists of hex bytes to send to the receiver
//   The '=' list is sent when the program starts.  The '$'
//   list is sent when a primary timing message is received
//   (i.e. once each second).  For example with a TSIP receiver:
//       $10,3a,00,10,03   would request doppler/code phase
//                         data every second.
//       $10,3C,00,10,03   would request satellite status
//                         and position data every second.
//   Note that the '$' and '=' command line options should
//   not be preceeded by a '/' or '-'.
//
//
//
//
//  Revision History (from version 5.0):
//
//    6.14.1 Modified to include SDL graphics support that can be used instead
//           of X11 on macOS and Linux (see makefile, set USE_SDL=1)
//           Added /sc command line option to use scaling instead of resize in
//           SDL builds
//
//           Added /hd command line option to set the "HOME" directory where
//           config options lie. Useful if you are running multiple versions
//           simultaneously.
//
//           Fixed support for "No Receiver" which uses the system clock to
//           display time, etc. For some reason this was removed but it's not
//           clear to me why
//
//           This version, 6.14.1, is unofficial and all modifications were done
//           by Jason Kingan, jasonk@toast442.org, jkingan on GitHub,
//           https://kg7nux.org or https://toast442.org
//
//
//    6.14 - Modified "n" keyboard command to allow editing of arbitrary files.
//           Under Windows it spawns the "notepad" editor.
//           Under Linux and macOS it spawns the "nano" editor.
//           You can also specify an editor to use with the "EDITOR" or
//           "HEATHER_EDITOR" environment variables.
//
//           Added PAGE_UP editing character to retreive the last line entered
//           so you can recover from a botched edit or easily repeat the last
//           command value.
//
//           Changed default plot statistic to show to SPAN (was RMS).  SPAN
//           is the difference between the maximum and minimum values shown
//           in the plot area.  Be aware that not all plotted values may
//           be available immediately after Heather has started collecting
//           data.  Those values will be assumed to be 0 and this can affect
//           the SPAN value.  It can help to clear the plot queue (CC or CP
//           keyboard command once Heather has valid data from all the
//           plotted variables.
//
//           If the user specifies the Trimble TSIP receiver type with the
//           /rx? command line option the base file name for output files was
//           changed from "tbolt" to "starloc","acutime", "sv6", or "res_t".
//           If the receiver type was autodetected (or is a Thunderbolt)
//           the base file name is "tbolt".
//
//           If the user specifies the Lucent SCPI receiver type with the /rxf
//           command line option the base file name for output files was
//           changed from "scpi" to "lucent". If the receiver type was
//           autodetected the base file name is "scpi".
//
//           If the user specifies the Nortel SCPI receiver type with the /rxy
//           command line option the base file name for output files was
//           changed from "scpi" to "nortel".  If the receiver type was
//           autodetected the base file name is "scpi".
//
//           Fixed "n" keyboard command to not break other two letter keyboard
//           commands that end in "n".
//
//           Added the Gp/? command to show a snapshot of all 7 plot
//           statistics for plot "p".
//
//           Fixed an issue where some SCPI receivers were not displaying
//           latitudes in the southern hemisphere properly.
//
//           Modified code to allocat ADEV or MTIE data queues only if
//           these features are being used.
//
//           Modified /gb command to toggle shared_plot... previous versions
//           always enabled shared_plot.   Also if /gb is issued from the
//           keyboard the screen is re initialized in order for the new
//           setting to take effect.
//
//           Modified satellite info table to allow sat PRN numbers up to 999.
//           This is because some receivers don't use the industry standard
//           PRN numbers for Galileo and Beidou, etc GNSS satellite systems.
//           THIS MODIFICATION CHANGES THE SUN and MOON PRN NUMBERS USED IN
//           LOG FILES FROM 256/257 to 1000/1001!
//
//           Added support for the Motorola @@Bg and @@Eg raw observation
//           data messages (used for the VPZ carrier phase capable Oncore
//           receivers.
//
//           Fixed issues with early model Motorola receivers displaying the
//           signal level.  These Motorola receivers report the signal level
//           as a value between 0 and 255.  Heather likes signal levels in
//           the 0 .. 50ish range, so the sat and signal level maps were not
//           properly displayed.  Heather now convets the sig level using
//           20*log10(sig)... the resulting value agrees quite well with
//           what later model Mototola receivers report in dBc.  This formula
//           is also now used with SCPI receivers that report "SS" signal
//           levels in the 0..255 range (like the Z3801A).
//
//           Fixed some potential issues with displaying the command line
//           help info on Linux/macOS/FreeBSD machines.  Allowed mouse/
//           touchscreen click to skip to the next page of help info.
//
//           Allowed screen heights/widths down to 320 pixels.  Sizes less than
//           640x400 require use of scaled vector fonts (/vq or $Q commands).
//
//           Added the EXPERIMENTAL /vq and $Q commands to draw characters
//           using a scaled vector font instead of the normal dot matrix font.
//           The font size can be scaled from 50 to 500 percent.  Scaled
//           vector fonts can help the readability of text on large screens
//           or to increase the plot area size on small screens.
//
//           Added EXPERIMENTAL /vd and $D commands to use a 480x320 screen
//           size (this use 75% scaled vector fonts).  Expect some srceen
//           formatting issues and you may need to change some display item
//           selections (watch, clock, shared mode, etc) particularly if you
//           change the screen res from the keyboard.
//
//           Added EXPERIMENTAL /ve and $E commands to use a 320x480 screen
//           size (this use 60% scaled vector fonts).  Expect some srceen
//           formatting issues and you may need to change some display item
//           selections (watch, clock, shared mode, etc) particularly if you
//           change the screen res from the keyboard.
//
//           Added EPERIMENTAL /vo and $O commands for rotating the image
//           in the screen window.  This can be useful when used with something
//           like using a 480x320 LCD as a 320x480 vertical format display.
//
//           Added the /pt= command line option to set the program title
//           bar text.  You can also set the title bar text from the keyboard
//           with the GG command.  Use &b as the first two characters of
//           the title.  If the title bar string is empty, Heather uses the
//           receiver type as the title.
//
//           Added "!e" keyboard command to execute an external program.
//           You enter the command line to start the program and it will run
//           in another window.
//
//           If an invalid two character keyboard command is entered you
//           get a beep and the srceen flashes an error message.
//
//           Reworked processing of simulation input files.  You no longer
//           need a receiver to be connected to process a simulation file.
//
//           Added /sw= command line option to control pacing of message
//           processing when reading a simulation file.  The value specifies
//           how long to delay in milliseconds after each message is read.
//
//           Fixed a spacing issue in the "/rx" command line help dialog.
//
//           Added the ability to sound the alarm at a specifed time on
//           the day that a calendar file greeting is triggered.
//
//           Allowed up to 5 greetings to match and be displayed on any
//           day.
//
//           Fixed HP5xxxx/Z3816/Z3817 receiver (/rx5) initialization control
//           message that turns off receiver full duplex mode.  Heather now
//           turns off full-duplex mode for all SCPI receivers.
//
//           Added support for a touch screen keyboard.  Clicking on the
//           UPPER LFFT corner of the screen brings up a touch screen keyboard.
//           The keyboard is automatically hidden whenever a keyboard command
//           is completed or one of the blank "keys" is clicked.  The "/kv"
//           command line option toggles enabling of the touch screen feature.
//
//           Added support for the numeric keypad on Linux/macOS/FreeBSD.
//
//           Added support for "heather_click.wav" sound file that plays
//           whenever a touch screen key or item is touched.  It is also
//           played every second in "tick clock" audible clock mode.
//
//           Added TH #T keyboard command and /th=#t command line options for
//           enabling the audible "tick clock" that ticks each second and
//           beeps each minute.  The sounds can be changed by supplying
//           the files "heather_second.wav" and "heather_minute.wav"
//
//           Added the /tl command line option for showing dates in the ISO
//           yyyy.mm.dd format
//
//           Fixed an issue on X11 systems where the mouse was responding
//           to clicks on a window sitting on top of the Heather window.
//
//           Fixed an issue on Windows systems to force the screen width
//           to even values when setting a custom screen size. WIN_VFX
//           barfs if screen width is an odd number.
//
//           Improved screen formatting for NO_RCVR mode on small screens
//           and screens with large width:height ratios.
//
//           Modified screen formatting for custom screen sizes so no wasted
//           blank space appears below the plot area.
//
//           Added -vp and $P commands for enabling a 800x480 screen display
//           (like the official Raspberry PI touchscreen display)
//           For X11 based systems "-vp" also enables full screen mode.
//
//           Added /vp and $P (800x480 - Raspberry PI) screen size command (*)
//           Added /vr and $R (1024x600) - Reduced screen size command (*)
//           Added /vj and $J (1280x800) screen resolution. (*)
//           Added /vk and $K (1280x960) screen resolution.
//           The screen types marked (*) can open in full screen mode on
//           Linux type systems... they are standard touchscreen sizes for
//           devices like the Raspberry PI.
//
//           Added full screen display mode for (X11 Linux / macOS displays)
//           using the -fu command line option. This must be used in
//           combination with an explicit screen size command that EXACTLY
//           matches the monitor size... e.g.  "/vc=800x480 /fu" or
//           "/vm /fu".  If the specified screen size does not match the
//           monitor size, all sorts of not-good things can happen. XQuartz
//           for macOS has problems with full screen mode.
//
//
//           Improved SCPI receiver recovery from commands that reset
//           the receiver.  It no longer does an auto_detect() which messed
//           up Lucent devices because of their different com port parity
//           than the Z3801A.  Also resets the SCPI command queue.
//
//           Improved the auto-detect of SCPI receivers.  Lucent KS devices
//           are now auto-detected.  The auto-detect routine now uses
//           19200:8:O:1 and if the device appears to be a SCPI device with
//           ODD parity, the setting is automatcially changed to 19200:7:O:1.
//           Also improved the overall reliabilty of the auto-detect routine.
//
//           Added support for the F1..F9 function keys to read keyboard
//           script files "f1.scr" to "f9.scr".  This allows F1..F9
//           (also F10 on X11 systems) to be used as programmable function
//           keys.
//
//           Clicking on the UPPER RIGHT corner of the zoomed plot display
//           exits plot review mode without exiting zoomed plot mode.
//
//           Tweaked some screen size display arrangement thresholds to allow
//           better displays on X11 systems where a few pixels are lost to
//           the window borders.
//
//           Added code so X11 systems to keep the screen updating while
//           scrolling the plot using the righthand mouse button.
//           Modified the signal level maps to use "fresher" data.  Previous
//           versions kept averages of every reading seen at a given az/el
//           position since the program started.  Now the averages are reset
//           to their current value once a day.  This makes it easier to
//           spot changes in the antenna system performance... they won't be
//           masked by potentailly weeks of old data.
//
//           Modifed clicking on an adev table to show all four adev values
//           for the selected adev table type.  Previous versions only
//           supported clicking on the upper adev table and the adev tables
//           for the value selected by the "AO" or "AP" keyboard command was
//           used.
//
//           Added support for touch screen control of plot markers and
//           skipping to the next error event.  Heather remembers the last
//           column that the mouse cursor was in the plot area at.  This is
//           shown by a small GREEN dot at the top of the plot area. If the
//           mouse cursor is outside of the plot area (like in the touch
//           screen keyboard) the plot marker control keyboard commands
//           (0..9, @, =, +, -) use this position.
//
//           Heather now remembers the last plot scrolling/positioning keyboard
//           command ([, ], {, }, <. >, +, -)  This command can be replayed
//           by clicking/touching in the "Receiver data" area near the top left
//           corner of the screen.  This is useful on touch screens so you
//           can scroll or move the plots without having to bring up the
//           touch keyboard for each desired movement. For instance, you want
//           to fine tune the plot position a few pixels to the left.  Press
//           the '[' key.  This moves the plot one pixel to the left.  Now
//           click in the "Receiver data" area.  Each click show shift the
//           plot left another pixel
//
//           Added support for FreeBSD (note: FreeBSD has no standard
//           sound system so sound effects are not available (unless you can
//           implment the "aplay" program and modify the play_tune() function).
//
//           Modified automatic leap second screen capture to work with
//           devices that report the leap second as a duplicated xx:59:59 or
//           xx:00:00.  Also the status column now shows that the leap second
//           screen was captured.  Fixed screen capture for receivers that
//           have a negative value for the /tsx receiver message time offset
//           (like SCPI receivers).
//
//           Modifed the "/br=" command line option to allow "," separators.
//           Also, if a field is not given, that value is not changed. Previous
//           versions set the missing value to a default value.
//
//           Reworked the com port data loss timeout code to only use extended
//           timeout intervals while processing commands (like diagnostics and
//           device resets) that take a long time.
//
//           Added /ct= command line option to allow setting all the com port
//           data loss timeouts (in msecs >= 3000). Normally Heather uses a
//           5000 msec com timeout.
//
//           Added /ct?= command line option to allow setting a specfic port
//           data loss timeout (in msecs >= 3000). Normally Heather uses a
//           5000 msec com timeout.  Replace the '?' character with the port
//           identifier character.
//
//           Added /ce command line option to toggle com port data / port
//           loss recovery.  If com data is lost or the com port disappears
//           Heather will attempt to re-init the com port every "com_timeout"
//           interval.  This new feature defaults to ON.
//
//           Modified scheduled events (screen dump, log dump, alarm, exit)
//           to not require an exact time match to trigger.  They now trigger
//           on the next time message at or after the specified time.  This
//           allows them to work if the receiver has a time skip at the
//           specified time.
//
//           Added TQ and /tsk commands for showing the digital clock in
//           Modified Julian date.time format.
//
//           Added TI and /tsw commands for showing the elapsed time interval
//           (stopwatch time) on the digital clock.
//
//           Added /tph command line option for showing temperature as
//           degrees Paris Hilton.
//
//           Added the SR keyboard command for changing the lat/lon/alt
//           scattergram reference point.
//
//           Modified the plot display filter commands so that a negative
//           value selects a "per-pixel" filter count based upon the plot
//           view interval and plot width.  Negative values for the filter
//           count no longer invert the temperature plot like previous
//           versions did.
//
//           Added the /va command line option to start up in "VT" View-Auto
//           scale plot view mode.
//
//           Modifed the display filter code to average the points each side
//           of the queue entry.  Previous versions averaged the points
//           after the queue entry. The new method keeps the "phase" of the
//           averaged display point aligned with the unaveraged queue data.
//
//           Added the ability to display the derivative of a plot's data.
//           For instance GPD will change the PPS plot to the derivative of
//           the plot.  In derivative mode, the ':' after the  plot statistic
//           label in the plot header is replaced by an '*'.
//
//           Added support for allowing multiple serial and TCP/IP ports to
//           be open at the same time.  This allows for future support of
//           things like external time interval counters and other devices
//           and echoing receiver data to an external device.
//
//           Added receiver data "echo" ports.  These ports echo the data from
//           the receiver to an external device.  The /ee[=device] command
//           echos the receiver data in its' native format.  The /ek]=device]
//           command echos the receiver data in NMEA format.
//
//           Added support for environmental temperature/humidity/pressure
//           sensors (/en command).  Currently only the dogtatian.com
//           and lookingforsolutions.com USB sensorr are supported.
//
//           Added /et command to send the moon, sun, and sat position(s) out a
//           port.  This data can be used to implement an antenna / solar
//           tracker.
//
//           Added support for using a dedicated serial port for the
//           temperature control device.  The /ef= command enables the
//           external temperature control port.
//
//           Modifed the way that "missing" odd numbered seconds are simulated
//           on receivers that only send time every other second.  The
//           simulated seconds are now generated "on-time". Before, they were
//           generated immediately after the even numbered seconds time
//           message.  This also works for simulating missing seconds that
//           occur while polled receivers (like SCPI and Star-4 devices) are
//           processing long commands like the SYST:STAT command.
//
//           Added support for ancient Trimble SV6/SV8 receivers (/rx6 command
//           line option)
//
//           Added support for ancient Trimble ACE-III receivers (/rxai
//           command line option... these are currently treated as a SV6/SV8)
//
//           Added support for ancient Trimble Palisade receivers (/rxpr
//           command line option) - not yet fully tested
//
//           Added support for Trimble Acutime (/rxag, /rxan, and /rxat
//           command line options) - not yet fully tested. /rxan is ACUTIME 360,
//           multi-gnss systems.  /rxag is Acutime GG, /rxat is for older
//           models like the Acutime Gold and Acutime 1000/2000.
//
//           Added support for ancient Trimble TAIP receivers (/rxp command
//           line option)
//
//           Added support for Datum TymServe 2000 receivers (/rxts command
//           line option)
//
//           Added support for the Lars simple GPSDO controller time interval
//           data.  The Lars device can either be used as the main input device
//           (as a time interval counter device) or as an "extra" time interval
//           counter device.
//
//           Added support for ancient Lucent RFTG-m GPSDOs (/rxf command
//           line option)
//
//           Added support for Novatel SuperStar II (/rxss command
//           line option) - currently untested
//
//           Added support for the SRS PRS-10 rubidium oscillator (/rxpr
//           command line option)
//
//           Added support for the Spectratime SRO100 rubidium oscillator
//           (/rxsr command line option)
//
//           Added support for Spectrum TM4 GPSDOs (/rxtm command line
//           option)  - Note I don't have one to test the command that set
//           options and values!
//
//           Added support for the Symmetricom rubidium oscillators.  These
//           cannot be auto-detected, but Heather does try to determine the
//           model from the devide responses. Use /rxsy for auto detect model,
//           /rxx7 to force X72,  /rxx9 to force X99,  /rxsa to force SA22).
//           /rxsb allows use with telecom versions that have a 58.9824 master
//           oscillator instead of the normal 60 MHz ones.
//
//           Added support for the Temex/Spectratime LPFRS and RMO rubidium
//           oscillators (/rxlp command line option)
//
//           Added support for TruePosition GPSDOs (/rxo command line
//           option)
//
//           Added support for Venus/Navspark RTK receivers.
//           Added !k keyboard command for setting Venus RTK base/rover mode
//           and RTCM output.
//
//           Added support for Zyfer Nanosync 380m GPSDOs (/rx3 command
//           line option)
//
//           Added support for the TAPR TICC (and some other) time interval
//           counters for dong ADEV analysis. The /rxi command line option
//           specifies time interval analyzer mode. Also /rx1 can be used
//           for other generic counters if /rxi does not properly identify
//           the attached device.  Note that /rx1 defaults to 9600 baud
//           and /rxi defaults to 115,200 baud.
//
//           Added support for Tom Van Baak's PICPET simple timestamping
//           time interval counter chips for dong ADEV analysis. The /rxpp
//           command line option specifies time interval analyzer mode.
//           The PICPET sends data at 19200,8,N,1 unless driven by a
//           non-10MHz clock.  Heather works with 2.5, 5, 10, and 20 MHz clocks
//           or 15 MHz on Windows systems (because most Linux systems don't
//           support the resulting 28800 baud serial data).  With a 10 MHz
//           clock the PICPET resolution is 100 nsec.  This can produce
//           rather noisy plots.  It is best to set a long-ish display filter
//           time (FD command - try FD 60).
//
//           Added support for a terminal emulator only mode (/rxte or /rxtt)
//           command line option).  This works like the /bt command line option
//           but when you exit terminal emulator mode, Heather exits.  Also
//           you can specify the device / parameters like /rxte=... where
//           "..." is the device settings info as documented for external
//           device command line options (like /ei).
//
//           Added /np and TP commands for running a keyboard script at
//           a specified time or interval.
//
//           Added /nc and TC commands for running an external program
//           at a specified time or interval.  The program to run is
//           deterined by the "HEATHER_EXEC" environment variable.  If it has
//           not been set, then the default program "heather_exec" is run.
//
//           Added /ne command line option to disable the file editing and
//           execution commands.
//
//           Added /bm and !o commands for enabling a port data monitoring
//           screen.  Data from the port is shown in GREEN. Data sent to the
//           port is shown in YELLOW.  The monitor screen can be in either
//           hex or ascii.  You can specify the port to monitor.
//           To exit port monitoring mode press any key.
//
//           Added ZO command to monitor receiver port traffic and show it as
//           a zoomed screen display.
//
//           Added ZH command to monitor receiver port traffic and show it as
//           a formatted hex/ascii zoomed screen display.
//
//           Modified ZN command to display a blank screen.  On previous versions
//           ZN would cancel the current zoomed screen display.  This is now
//           done with the ZZ command.
//
//           Added ZT command to set a keyboard inactivity timeout (in
//           minutes).  After the specified time has passed without any
//           keyboard activity, the screen switches to the specified zoomed
//           screen display.
//
//           Added ZQ command for zoomed full screen calendar display.  (ZQ
//           used to be the same as ZS zoomed signal display).
//
//           Added TB keyboard command to show calendar for a specifed year.
//
//           Added /zs? to start up with zoomed display '?'
//
//           Modifed !t and /bt commands to allow connecting the termnial
//           emulator to any port.
//
//           Modified NMEA handler to prioritize the message used to generate
//           the time code and update the screen:  GxZDA, GxRMC, GxGGA, GxGNS
//           The GGA and GNS messages do not have a date in them.  If the date
//           has not been seen in a NMEA message, the system clock date is
//           used.  Once a higher priority message has been seen, the lower
//           priority ones are not used for time keeping.
//
//           If adev values are being plotted, the plot area is labeled with
//           the taus at the top of the plots and the decades at the right
//           side of the plots.  The adev labels are show in GREEN.  Turning
//           off the adev plots removes the adev labels.
//
//           Modifed default ADEV queue size to 43200 points (12 hours).  It
//           was 36000 points.
//
//           Added /ja (or /jp),  /jb (or /jo), /jc, and /jd command line
//           options for individually setting the ADEV queue period.
//           /j sets all queue periods to the same value.
//
//
//           Added /im? command for forcing the time interval counter data
//           type to ?  (default for /im is Timestamp mode).  If the mode is
//           not forced for non TAPR TICC counters, the mode is set to
//           'I' (time interval). Other counter modes include:
//              F - frequency
//              I - interval
//              P - period
//              T - timestamp
//              D - debug (TAPR TICC)
//              L - Timelab (TAPR TICC)
//
//
//           For TAPR TICC devices the "P" menu is used to configure the
//           device.  Any changes to the device configuration take around
//           5 seconds to process and will reset the plot and adev queues.
//           Selecting the "P" menu will also change the display mode to
//           show the current device configuration.  While the device
//           configuration is being shown any keypress will restore the
//           screen to normal operation.
//
//           For TAPR TICC devices the PC command sets the device calibration
//           periods value.
//
//           For TAPR TICC devices the PD command sets the "Time Dilation"
//           values.
//
//           For TAPR TICC devices the PK command sets the device coarse
//           clock interbal (in microseconds).
//
//           For TAPR TICC devices the PM command sets the device operting
//           mode.
//
//           For TAPR TICC devices the PE command sets the input edge
//           polarity for the channels.
//
//           For TAPR TICC devices the PF command sest the channel delay
//          "FUDGE" values.
//
//           For TAPR TICC devices the PR command sets the device reference
//           clock frequency (in MHz).
//
//           For TAPR TICC devices the PS command sets the sync mode
//           value.  WRANING: don't enable slave mode unless a slave device
//           is connected.  To recover from a missing slave device, start
//           Heather with terminal emulator mode enabled (/bt) and used the
//           TICC menu to set the sync mode to master.
//
//           For TAPR TICC devices the PO command sets the device timeout
//           value.
//
//           For TAPR TICC devices the PT command sets the "FIXED TIME2"
//           values
//
//           For TAPR TICC devices the &a command starts an "autotune"
//           proceedure that sets the FUDGE and TIME2 parameters.  For
//           autotune to work you must have a 1PPS signal connected to the
//           TICC inputs via cables with matched lengths (use a "T" adaper
//           with matched cable between the "T" outputs and the TICC inputs).
//           You specify the number of seconds to analyze.  It must be
//           greater than 30 seconds, but values over 1800 seconds are
//           recommened.
//
//           For TAPR TICC devices, selecting the '&' keyboard menu will
//           show the TICC parameters... type "& ESC".  This will show
//           the paramter settings and stop showing the ADEV tables.  Press
//           any key to resume the normal display.  On wide screen monitors
//           (at least 1280 pixels wide) the TICC config info is part of the
//           normal display.
//
//
//           Added /twi= command line option for setting the time interval
//           (in seconds) that timestamps wrap at.  The default setting is to
//           reduce all timestamp values modulo 100.0 seconds.
//
//           Added PW keyboard and /pw# command line options for setting the
//           time interval counter phase wrap time (default = 100.0E-9 for a
//           10.0 MHz oscillator)
//
//
//           For Symmetricom X72 compatible devices the &a command calculates
//           the DDS tuning value that will set the 10 MHz output to the
//           correct frequency.
//
//           Added a software disciplining algorithm for the X72 and SA22
//           rubidium oscillators that do not have firmware that supports
//           discipling with a 1PPS input.
//
//
//           Added support for displaying MTIE (Maximum Time Interval Error)
//           of TICC / interval counter data.  The AI command enables the
//           MTIE display.  The maximum time interval used is the largest
//           power of 2 <= the adev queue size value.  MTIE calculation
//           stops whenever the maximum time interval is reached.  The CI
//           command will clear the MTIE data and start over.
//
//           The WM command writes the current MTIE data to a file. The MTIE
//           results are also written when a log file is closed.
//
//
//           Added /dq=filename command line option and WT keyboard commands
//           for writing TICC data to a file when both a receiver and and
//           a TICC are in use at the same time.  Receiver data goes to the
//           /dr (or WY) receiver data capture file and TICC data goes to the
//           /dq (or WT) TICC data capture file.
//
//           Added /as= command line option for setting the ADEV bin spacing.
//           also AS keyboard command, Heather is limited to 100 adev bins.
//               /as=0  all tau (not practical)
//               /as=1  one bin per decade
//               /as=2  one bin per octave
//               /as=3  3dB per decade
//               /as=4  1-2-4 decade
//               /as=5  1-2-5 decade
//               /as=10 10 per decade
//               /as=29 29 (linearly spaced) per decade
//               /as=99 log spaced
//
//           Added /ae command line option and AE keyboard option for toggling
//           the display of ADEV error bars in the plots.
//
//           Added the '~' keyboard option to patch a point in the plot queue
//           that has a data glitch.  Position the mouse cursor on the
//           offending point and press '~'.  This will replace the point's
//           data with the previous data point.  You may want to then use
//           CR command to recalulate xDEV and MTIE values from the patched
//           queue data.
//
//           Added the Gp~ command to automatically remove all glitches from
//           a selected plot (where p is the plot to de-glitch).
//
//           Added calculation of solid earth tide displacements (in mm) and
//           the net vertical gravity change (in uGals) due to the effects of
//           the sun and moon.  The GLT keyboard command toggles the display
//           on and off.  The values are also shown in the ZI zoom screen
//           display.  The GKx keyboard command controls the earth tide
//           displacement and grvity offset plots.  If an external
//           environmental sensor is used the GKx commands control the
//           sensor plots.  If a satellite is selected for plotting the GKx
//           commands control the sat az/el/signal level plots.
//
//           Added the /rx0 "gravity clock" operating mode.  This uses the
//           system clock as the time source.  It is optimized for showing
//           solid earth tide displacements and vertical gravity offsets.
//           To use this mode you must specify your location (via SL from
//           the keyboard or the /po command line option or the "heather.loc"
//           file).
//
//           The color palette used for the lat/lon scattergram now repeats
//           every 12 hours (was 14 hours).  Colors 0, 1, and 2 are not used.
//           Using a 12 hour cycle is more appropriate since since it meshes
//           better with 24 hour days.  Also colors 1 and 2 are rather dim and
//           hard to see against a black background.
//
//           Added /cm=color,r,g,b command line option to change color map
//           table entries.
//
//           Added GpH command (where "p" is the plot selection character)
//           to calculate a histogram of the selected plot's value.  The
//           histogram is calculated over the values being displayed in the
//           plot window.  The histogram and FFT functions share much of the
//           same code and cannot be used at the same time.  The GF (FFT)
//           keyboard command is also used to disable a histogram plot.
//
//           Added /tsu command line fpr displaying the digital clock time
//           as Unix seconds.  The time is for the currently set local time
//           zone.  For actual Unix time, set the time zone offset to 0.
//
//           Added /tsg command line fpr displaying the digital clock time
//           as GPS seconds.  The time is for the currently set local time
//           zone.  For actual GPS time, set the time zone offset to 0.
//
//           Modified the automatic week number rollover correction code
//           to flush the plot queues if the rollover is detected within
//           20 seconds of starting the program.  This is so the bogus time
//           codes do not pollute the plot cursor time calculations.  The
//           20 second threshold allows receivers that just had a rollover
//           occur to be analyzed... rollovers within the first 20 seconds
//           are assumed to be from a receiver that has had long standing
//           rollover issues.
//
//           Modified sun/moon position code to show the positions in YELLOW
//           if they are aligned well enough (0.75 degrees) to be an eclipse.
//           (currently only solar eclipses are detected).
//           Also modified sun/moon position history trail to show when the
//           sun/moon was below the horizon in GREY.
//
//           Fixed issue with end of daylight savings time possibly triggering
//           one hour early.
//
//           Added /dw date format command for showing dates in ISO week
//           format (yyyyWww-d)
//
//           Added SD command to calculate appoximate radio signal delay
//           between the GPS receiver and a remote station.  You enter the
//           station lat and lon and the ionosphere height (approx 250 km
//           in the winter and 350 km in the summer).  The SD command can also
//           be used to calculate the great circle distance and bearing
//           between the current receiver lat/lon and a user specified
//           location.
//
//           Fixed issues parsing the RFTG-m status bytes.  Three of the
//           status bytes were not being converted from hex and the first
//           status byte value was being interpreted for those status
//           bytes... D'oh!
//
//           Added support for a "Big Ben" mode Westminster chime clock.
//           You will need to supply your own .WAV files for the hour
//           sequences and the quarters.
//
//           Added SU keyboard command to set the default UTC (leapsecond)
//           offset value that the receiver will use until it has the
//           current value from the satellites.  This setting will be saved
//           in flash memory. This command is not supported for most receivers.
//
//           Modifed displays that show the sun with rays so that rays that
//           fall outside of the clipping area for the display are not
//           shown. This minimizes trash left of the screen when the sun
//           moves away from the horizon.
//
//           Fixed zoomed displays that show satellite PRNs to not show PRNs
//           that are no longer active.  The previous version did not always
//           erase the old PRNs.
//
//           Modified satellite / sun / moon position history trail code to
//           use floating point az/el values (old version used integers).  If
//           the device reports satellite position as floating point numbers
//           the satellite trails will be a lot smoother.
//
//           Fixed an issue where the sun/moon trails on Raspberry Pi's
//           were corrupted when the sun/moon was below the horizon (this
//           issues seems to be due to a Ras Pi compiler bug).
//
//           Fixed an issue where if the log file name was given on the
//           command line (/wl=) before the receiver type (/rx) was specifed
//           the default file name for the receiver type overrode the user
//           specified log name.
//
//           Changed the way the automatic GPS date rollover correction works.
//           Before, once a rollover condition was detected (GPS repoted a
//           year <2016) the rollover correction was calculated and used from
//           then on.  The rollover code now does not "latch onto" a single
//           rollover correction value and will update the rollover correction
//           continuously.  This change helps if Heather is started before
//           the GPS receiver has finished initializing and is sending invalid
//           dates.  Once it starts sending valid dates, the old (bogus)
//           rollover correction is canceled.  Also, Heather no longer waits
//           for the receiver to have a UTC leapsecond offset and it reports
//           a valid time for rollover compensation to be enabled.
//
//           Fixed the -1a, etc command line options that select Linux ACM/CDC
//           devices to use /dev/ttyACM0  (was using /dev/ACM0 which is not
//           available on a lot of distros)
//
//           Fixed configuring the GNSS system on Trimble multi-gnss devices.
//           Before it was not altering the GNSS system.
//
//           Changed Trimble ACE-III receiver type selector to /rxai.  Before
//           it was /rx3.  /rx3 now selects the Acutime 360 devices.
//
//           Modifed XML/GPX log files to set lat to 90.0 and lon to 90.0
//           if the location mode is set to "private" (GLP keyboard command)
//
//           When log files are closed all the statistics of the currently
//           displayed plot window's plot queue data are now written to the
//           log file as comments.
//
//           Added support for getting the lat/lon/alt from the file
//           "heather.loc" if the receiver type does not have a GPS that
//           reports the device location.  The heather.loc file location can
//           also be used if you enter a value of "loc" when Heather requests
//           a lat/lon/alt location.
//
//           Changed NMEA echo port command to /ek (was /en)  and environmental
//           sensor (thermometer) port command to /en (was /et)... sorry for
//           any inconvienience.
//
//           Added display of tracked sat count number at the plot cursor
//           to the plot header (if it will fit on the screen)
//
//           If the mouse is placed over a satellite circle in a map display
//           the sat az/el/signal level will be shown below the map display
//           (but only if there is enough space on the screen to do it)
//
//           Added /so command line option to set the outline shape to draw
//           the sats in on the satellite maps.
//
//           Addded ability to simulate input from an "extra" interval counter
//           device using the /ri=filename command.
//
//           The /h command line option now shows the command line help.
//           Earlier versions of Heather used /h=file to read in a .cfg
//           config file.  You now must use the /r=file.cfg command line
//           option to read config files.
//
//           Modified the order of the lines in the plot header info area.
//           All plot values are now on contiguos lines.  Note that if a
//           FFT or histogram plot is enabled and the mouse cursor is over
//           FFT or histogram data, and a plot review mode is active, it is
//           posible that the top line of the histogram/FFT info can oveflow
//           into the plot review line.
//
//           Modifed the code that shows the plot data at the mouse cursor
//           to show the time in RED if it is at a time stamp sequence error.
//           The mouse cursor data is normally show in CYAN.
//
//           Modified Ublox code to automatically send RAW data messages every
//           second or at the requested raw data rate (previously these were
//           only requested by the message polling loop).
//
//           Fixed an issue where a precision survey would be aborted if the
//           receiver had a com message timeout during the survey.
//
//
//           Changed the satellite information table display routines.
//           If more satellies are visible than can fit on the screen, a
//           message is shown.
//
//           The satellite PRN is followed by a character that indicates
//           which GNSS system the satellite belongs to.  They follow the
//           RINEX standard for designating satellite systems.
//
//           Changed the GCT command to control display of only tracked sats.
//           Earlier version of Heather used GCT as an alias of the SI
//           satellite info table format command.
//
//           Added ability to sort the table on carrier phase, pseudorange,
//           clock bias, and state.  Invalid or missing values are always
//           sorted to the bottom of the table.
//
//           Made the satellite count plot a standard plot with plot
//           statisics, etc available.  You can control it with the G$ or
//           GCG keyboard commands.  The sat count plot does not scale or
//           center.  It is always at the bottom of the plot area.
//
//           Changed the auto-detect data collection timeout code to work
//           better with X11 systems.  It now completes the data collection
//           after the intended 3 seconds.  It was taking much longer due to
//           continuous screen updates.
//
//           Modified time zone / daylight savings time code to automatically
//           set the DST zone to 2 (Uk/Europe) if the time zone name for
//           daylight savings time is set to "BST" and the DST zone has not
//           been specified by the user with the /b= command line option.
//
//
//
//           Added support for writing RINEX format log files if the receiver
//           supports raw satellite observation data output (doppler, carrier
//           phase, pseudorange).  To write a RINEX file, specify a log file
//           name with the ".obs" extension.  RINEX files can be submitted to
//           GPS data processing services to get much more precise location
//           information.  Currently the best processing service seems to
//           be CSRS-PPP (in Canada).  They can handle single frequency
//           receiver data files (but they only seem to use the pseudorange
//           measurements).
//
//           Added /rr command line option for controlling output rate of
//           raw data messages.  Defaults to 1 second, but raw data can cause
//           a lot of traffic from the receiver.  Raw data messages are
//           are mainly used by post-processing programs for calculating
//           very precise position and time solutions.
//
//           Added /rm command line option for setting the raw measurment
//           types to include in RINEX files.
//
//           Added /at command line option for setting the antenna type
//
//           Added /an command line option for setting the antenna number
//
//           Added /ah command line option for setting the antenna height
//           (and optionally e/w and n/s displacements) to include in RINEX
//           files.
//
//           Added /ak command line option for setting the marker name
//           string to include in RINEX files.
//
//           Added /av command line option for setting the marker number
//           string to include in RINEX files.
//
//           Modified default plots for the TruePosition receivers to show
//           the EFC DAC value (GD) and hide the undocumented EVAL value (G8).
//           Also changed the scale factor for the DAC plot to be in uV/div.
//
//           Added the GLA command to autoscale the LLA scattergram scale
//           factor.  This can be useful if you are using the receiver in
//           a moving environment.
//
//           Added the SQ command to enable plotting of a single satellite's
//           or sun or moon azimuth, elevation, and signal level.  If SQ mode
//           is enabled, the earth tide plots are not available.  The GK
//           command is used to enable/disable plots.  This SQ mode is not
//           available when an envronmental sensor is being used.
//
//           Added the LP/WLP command to enable writing a satellite PRN info
//           log file of the tracked satellites time/prn/azimuth/elevation
//           info (the time is in JD UTC format).
//
//           Fixed LLA scattergram rendering code to invert north/south and
//           east/west directions.
//
//           Modified the code that guesses the UTC offset (leapsecond count)
//           for receivers that don't support leapsecond output to assume the
//           next leapsecond (#19) will be on 1 JUL 2019.
//
//           If the receiver does not send gps week or time-of-week data
//           then these values are calculated from the time.  If this is done
//           the WEEK: and TOW: values will be shown in YELLOW.
//
//           Changed UCCM GPSDO message parser to (hopefully) work with the
//           Ublox LEA-6T / Samsung oscillator versions of the GPSDO.
//
//           Fixed (hopefully) the CT plot queue trim commands.  CTE and  CTS
//           were not always working properly.
//
//           Reworked the code that shows the alarms and receiver state column
//           to maximize the amount of information that can fit on the screen.
//
//           Modifed the satellite PRN maximum signal level table (shown when
//           the '&' menu is active) to show all sats.  Previously only PRNs
//           1..32 were shown.  If no sats in a row have signal levels
//           detected then that row is not shown.
//
//           Modified the format of the "#SIG" satellite constellation lines
//           in ASCII log files to include: PRN, AZ, EL, SIG, DOPPLER,
//           CODE or CARRIER PHASE, PSEUDORANGE, CLOCK BIAS.  I'm sorry if
//           this change breaks any of your current log readers... .XML log
//           files are much better about indicating what the various data
//           values are (but, alas, are larger and harder to read).
//
//           Modified the default title bar string to include the executable
//           file name along with the receiver type.
//
//           Added a RPN calculator mode: "`" (backwards quote) keyboard
//           keyboard command.  Also Z` will bring up the calculator in a
//           zoomed full screen mode.  The /rxca command line option selects
//           a "calculator only" mode.
//
//           Added support for New Years song that plays at 00:00:01 on
//           1 Jan.
//
//           Added ability to show altitude in standard linguini (7.1429 lg
//           per meter) with /tlg command line option or GLL keyboard command,
//
//           Added /wt[=#] command line option to select the type of hands
//           to draw the analog watch with.  Default hand shape is now
//           filled trapazoidal hands.
//
//
//  Other (mostly rather obscure) keyboard selctable options:
//  Those marked with "---" are either no longer implemented or have proper
//  keyboard or command line options to do what they did.  Their function
//  is very likely to be changed in later versions.
//
//    OA   - toggle AMU vs dBc signal level displays.
//    OB # - set ADEV bins per decade
//    OC   - toggle continuous plot scrolling mode.  In non-scrolling mode
//           the plot queue arrows are shown in CYAN instead of WHITE)
//           The (dafault) continuous scroll mode scrolls the plot left one
//           pixel as each new point comes in.  In non-scrolling mode, the
//           plot scrolls left two major divisions whenever it reaches the
//           right edge.  Also the ADEV tables are only updated every 10
//           seconds.  Non-scrolling mode was intended for use on very
//           slow systems.
//    OD   - toggle FFT plot display in dB or raw values
//    OE   - toggle Thunderbolt-E display mode
//    OF   - toggle periodic refresh of adev calculations
//    OG   - set solid earth tide options:  bit 0x01 set = use original solid.f
//           sun/moon position   bit 0x02 set = calculate mean tides by restoring
//           permament earth tide deformations.
//    OH   - toggle erasing of Lat/Lon/Alt survery plot every hour
//--- OI # - set signal level display type
//    OJ # - toggle logging of serial port data stream
//    OK   - toggle logging of TSIP message faults as time skips
//    OL   - toggle live FFT / Histogram mode
//    OM # - set plot magnification factor
//    ON   - toggle real-time update of trend line plot title
//    OP   - toggle plot scaling mode to peak value seen
//    OQ   - toggle plot queue sampling fast / slow mode
//--- OR   - reset ADEV bins and recalculate ADEVs
//    OS # - toggle temperature spike filter mode
//    OT   - toggle alarm/dump/exit time triggers to be based upon local time
//           (default) or displayed time which can be in one of the
//           astronomical time scales.  Previous versions used OT to toggle
//           12/24 hour clock mode which is now available from the T menu.
//--- OU # - set daylight savings time area number (0 .. 5)
//    OV # - toggle ADEV base value mode
//--- OW # - select analog watch face type (0 .. 5)
//    OX # - set trend line rate display time scale (0=units per day, 1=per hour, 2=per minute, 3=per second)
//--- OY # - set tempurature-dac plot (G3 plot) scale factor
//    OZ # - toggle plot cursor time reference between start of capture <hms>
//           and start displayed data [hms]  You can also force the time
//           to be in seconds by setting the OZ value to 2 for <seconds> or
//           to 3 for [seconds]
//
//
//
//
//  This program requires the following operating system dependent routines:
//
//  init_hardware() - put screen in a high res graphics mode or open a
//                    graphics window and open the com port (9600,8,N,1)
//
//  dot(x,y, color) - draw a colored dot
//
//  get_pixel(x,y) - read a pixel from the screen
//
//  kill_screen() - close graphics screen or window
//
//  sendout()  - send byte 'c' to the serial port
//
//  SERIAL_DATA_AVAILABLE() - a routine or macro that returns true if
//                            there is serial port data available
//
//  get_serial_char()  - get a character from the serial port.  You should check
//                       SERIAL_DATA_AVAILABLE() befor calling this.
//
//  kill_com()    - close down serial port
//
//  SetDtrLine(state) - used for PWM fan control for temerature control PID
//  SetRtsLine(state) - used for PWM fan control for temerature control PID
//  SetBreak(state)   - used to wake up Nortel GPSDO's by sending a BREAK condition
//
//  KBHIT() - a routine or macro that returns true if a keyboard key has
//            been pressed
//
//  GETCH() - a routine or macro that returns the keyboard character
//
//  refresh_page() - copy any virtual screen buffer to the screen
//                   or flip pages if doing screen buffering.  Can be
//                   a null routine if writing directly to the screen.
//
//  BEEP() - sound a beep if beep_on is set
//
//
//  set_cpu_clock() - has OS dependent code for setting the system time
//
//  GetMsecs() - returns a double precision count of milliseconds. Can
//               be since program started,  os boot, or any base time
//               reference (used to do pwm control of the heat/cool cycle
//               if doing precise temp control and checking for loss of
//               serial port communications. Also sets wall_time variable
//               to system clock time with high resolution.
//
//  GetNsecs() - return high resolution timer count in nanoseconds.
//
//  Also the get_mouse_info() routine will need to be updated to handle
//  the system mouse.  You need to get the current mouse coordinates into
//  variables mouse_x and mouse_y and the button state into this_button.
//
//  play_tune() - play a sound file asynchronously (non-blocking)
//
//  Also, you can improve performance with operating system dependent
//  line and circle drawing and area erasing functions.
//
//  To see where you might need to add support for a new operating system
//  search the code for the strings:
//    WINDOWS     (Windows related stuff)
//    __linux__   (Linux related stuff)
//    __MACH__    (macOS OS/X related stuff)
//    __FreeBSD__ (FreeBSD related stuff)
//    WIN_VFX     (Windows video/keyboard/mouse stuff)
//    USE_X11     (X11 video/keyboard/mouse stuff for Linux and OS/X)
//
//  If adding a new receiver type search for NEW_RCVR.  This flags routine
//  that will almost certainly need to be updated for the new receiver type.
//  (there are also lots of other places that will probably need updating)
//
//
// Note: stupid DOS linker does not allow initialized variables to be declared
//       in more than one file (even if defined as externs).  Therefore any
//       variables that are used in more than one file are initialized
//       to non-zero values in the routine set_defaults();
//
//

#define EXTERN
#include "heather.ch"
#include "heathfnt.ch"

char szAppName[256+1] = "Lady Heather's Disciplined Oscillator Control Program - " VERSION "";
char exe_name[MAX_PATH] = "";


#ifdef __MACH__    // OSX time functions
   #include <mach/clock.h>
   #include <mach/mach.h>
   char *editor = "nano";
   char *heather_exec = "heather_exec";
   int mac_os = 1;
#else
   int mac_os = 0;
#endif


#ifdef WINDOWS
   u08 timer_set;  // flag set if dialog timer has been set
   u08 path_help;  // flag set if help message has been seen before
   BOOL Win32Exec(LPSTR lpCmdLine, BOOL bWait);
   char *editor = "notepad";
   char *heather_exec = "heather_exec";
#endif


#ifdef __linux__
   char *editor = "nano";
   char *heather_exec = "heather_exec";
#endif


#ifdef __FreeBSD__
   char *editor = "ee";
   char *heather_exec = "heather_exec";
#endif


unsigned char *dot_font;  // pointer to character font in use
int x11_mouse;            // flags x11 mouse motion detected
int my_mouse;             // flag set if mouse is in the Heather X11 window
int no_mouse_review;      // used to inhibit recursive calls to get_mouse_info
int com_tick;             // used to fake SERIAL_CHAR_AVAILABLE() if no receiver detected


#ifdef WIN_VFX            // using WIN_VFX library for screen/mouse/keyboard

   VFX_WINDOW *stage_window = NULL;
   PANE       *stage = NULL;

   VFX_WINDOW *screen_window = NULL;
   PANE       *screen = NULL;

   VFX_FONT *vfx_font = (VFX_FONT *) (void *) &h_font_12[0];

   S32 font_height;

   U16 transparent_font_CLUT[256];
#endif

char degc[] = { DEGREES, 'C', 0 };
char degk[] = { DEGREES, 'K', 0 };
char degs[] = { DEGREES, 0 };

//#define CURSOR_TIME_ROW  (MOUSE_ROW+2)
//#define CURSOR_VAL_ROW   (MOUSE_ROW+3)
//#define STAT_ROW         (MOUSE_ROW+4)
//#define REVIEW_ROW       (MOUSE_ROW+5)
//#define REF_ROW          (MOUSE_ROW+6)
//#define SCALE_ROW        (MOUSE_ROW+7)
//#define PLOT_INFO_ROW    (MOUSE_ROW+7)   // the row just above the plot area
//#define FFT_VAL_ROW      CURSOR_TIME_ROW
#define CURSOR_TIME_ROW  (MOUSE_ROW+2)
#define REVIEW_ROW       (MOUSE_ROW+3)
#define CURSOR_VAL_ROW   (MOUSE_ROW+4)
#define STAT_ROW         (MOUSE_ROW+5)
#define REF_ROW          (MOUSE_ROW+6)
#define SCALE_ROW        (MOUSE_ROW+7)
#define PLOT_INFO_ROW    (MOUSE_ROW+7)   // the row just above the plot area
#define FFT_VAL_ROW      REVIEW_ROW

struct PLOT_DATA plot[NUM_PLOTS+DERIVED_PLOTS] = {  // plot configurations
//  ID        units   ref scale  show  float  plot color
   {"PPS",    "ns",   1.0F,      1,    0,     PPS_COLOR    },
   {"OSC",    "ppt",  1000.0F,   0,    0,     OSC_COLOR    },
// {"DAC",    "uV",   1.0E6F,    1,    1,     DAC_COLOR    },
   {"DAC",    "V",    1.0E6F,    1,    1,     DAC_COLOR    },
   {"Temp",   degc,   1000.0F,   1,    1,     TEMP_COLOR   },
   {"D\032H", "mHz",  1.0F,      0,    1,     ONE_COLOR    },   // G1
   {"DIF",    "x",    1.0F,      0,    1,     TWO_COLOR    },   // G2
   {"D-T",    "x",    1.0F,      0,    1,     THREE_COLOR  },   // G3

   {"[4]",    "x",    1.0F,      0,    1,     FOUR_COLOR   },
   {"[5]",    "x",    1.0F,      0,    1,     FIVE_COLOR   },
   {"[6]",    "x",    1.0F,      0,    1,     SIX_COLOR    },
   {"chC",    "ns",   1.0F,      0,    1,     SEVEN_COLOR  },
   {"chD",    "ns",   1.0F,      0,    1,     EIGHT_COLOR  },
// {"[9]",    "x",    1.0F,      0,    1,     NINE_COLOR   },
// {"[0]",    "x",    1.0F,      0,    1,     TEN_COLOR    },
   {"MsgOfs", "msec", 1.0F,      0,    1,     NINE_COLOR   },
   {"MsgJit", "msec", 1.0F,      0,    1,     TEN_COLOR    },
   #ifdef FFT_STUFF
   {"FFT",    "x",    1.0F,      0,    0,     RED  },
   #endif
   {"[11]",   "x",    1.0F,      0,    0,     ELEVEN_COLOR    }, // DERIVED_PLOTS
   {"[12]",   "x",    1.0F,      0,    1,     TWELVE_COLOR    },
   {"[13]",   "x",    1.0F,      0,    1,     THIRTEEN_COLOR  },
   {"[14]",   "x",    1.0F,      0,    1,     FOURTEEN_COLOR  },
   {"Sats",   "",     1.0F,      0,    1,     FIFTEEN_COLOR   },
};


#define MAX_BELLS 12
int bells[8][MAX_BELLS] = {   // for ships clock: 1=play bell file, 0=silent
   { 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0 }, // 8 bells
   { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 1 bells
   { 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 2 bells
   { 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 }, // 3 bells
   { 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0 }, // 4 bells
   { 1, 1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0 }, // 5 bells
   { 1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0 }, // 6 bells
   { 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 0 }  // 7 bells
};



extern char *dst_list[];  // daylight savings time definitions
extern int first_request;
int f11_flag;             // set if F11 pressed


#ifdef GCC  // gcc compiler
   void strlwr(char *s)     // convert string to lower case
   {
      while(*s) {
         *s = tolower(*s);
         ++s;
      }
   }

   void strupr(char *s)     // convert string to upper case
   {
      while(*s) {
         *s = toupper(*s);
         ++s;
      }
   }
#endif


u32 atohex(char *s)
{
u32 val;

   // return 32-bit integer value of a hex ascii string (can have leading 0x)

   if(s == 0) return 0;
   if((s[0] == '0') && (s[1] == 'x')) s += 2;
   else if((s[0] == '0') && (s[1] == 'X')) s += 2;

   val = 0;
   sscanf(s, "%x", &val);
   return val;
}


double atosecs(char *s)
{
double val;

   // return integer value of a hex ascii string
   if(s == 0) return 0;

   val = 0;
   val = atof(s);

   if(strchr(s, 'm') || strchr(s, 'M')) val *= 60.0;
   else if(strchr(s, 'h') || strchr(s, 'H')) val *= 60.0*60.0;
   else if(strchr(s, 'd') || strchr(s, 'D')) val *= 24.0*60.0*60.0;

   return val;
}




int tune_exists(char *file, int add_path)
{
char tune[MAX_PATH+1];
FILE *f;

   // This routine sees if a sound file exists

   if(add_path) sprintf(tune, "%s%s", heather_path, file);
   else         sprintf(tune, "%s", file);

   f = topen(tune, "r");
   if(f) {
      fclose(f);
      return 1;
   }
   return 0;
}

void find_sound_files()
{
   // See if user sound files exist for the cuckoo clock and alarm clock
   // If so, we play them.  Otherwise we use the default sounds.

   chime_file   = tune_exists(CHIME_FILE, 1);
   alarm_file   = tune_exists(ALARM_FILE, 1);    // see if alarm sound file exists
   leap_file    = tune_exists(LEAP_FILE, 1);     // see if leapsecond sound file exists
   sun_file     = tune_exists(SUNRISE_FILE, 1);  // see if sunrise tune sound file exists
   noon_file    = tune_exists(NOON_FILE, 1);     // see if solar noon tune sound file exists
   bell_file    = tune_exists(BELL_FILE, 1);     // see if ships bell sound file exists
   click_file   = tune_exists(CLICK_FILE, 1);    // see if touchscreen click sound file exists
   minute_file  = tune_exists(MINUTE_FILE, 1);   // ticking clock minute sound
   seconds_file = tune_exists(SECONDS_FILE, 1);  // ticking clock seconds sound

   if(tune_exists(USER_CHORD_FILE, 1)) chord_file = 2;
   else if(tune_exists(CHORD_FILE, 0)) chord_file = 1;

   if(tune_exists(USER_NOTIFY_FILE, 1)) notify_file = 2;
   else if(tune_exists(NOTIFY_FILE, 0)) notify_file = 1;

printf("chime_file:%d\n", chime_file);  // zork - show_debug_info
printf("alarm_file:%d\n", alarm_file);
printf("leap_file:%d\n", leap_file);
printf("sunrise_file:%d\n", sun_file);
printf("noon_file:%d\n", noon_file);
printf("chord_file:%d\n", chord_file);
printf("noiify_file:%d\n", notify_file);
printf("bell_file:%d\n", bell_file);
printf("click_file:%d\n", click_file);
printf("minute_file:%d\n", minute_file);
printf("second_files:%d\n", seconds_file);
}

void play_tune(char *file, int add_path)
{
char tune[MAX_PATH+1];

   // This routine plays a sound file asynchronously (non-blocking)

   if(!sound_on) return;

   if(add_path) sprintf(tune, "%s%s", heather_path, file);
   else         sprintf(tune, "%s", file);


   #ifdef WINDOWS
      PlaySoundA(tune, NULL, SND_ASYNC);
   #endif

   #ifdef __linux__
      if(no_exec > 1) return;
      char shell_cmd[MAX_PATH+64];
      sprintf(shell_cmd, "/bin/sh -c \"aplay -q %s\" &", tune);
      system(shell_cmd);
   #endif

   #ifdef __MACH__
      if(no_exec > 1) return;
      char shell_cmd[MAX_PATH+64];
      sprintf(shell_cmd, "afplay %s &", tune);
      system(shell_cmd);
   #endif

   #ifdef __FreeBSD__
      //  FreeBSD does not have a standard sound system
   #endif
}


void play_user_sound(char *fn)
{
int add_path;

   // play a user specified sound file.  Adds directory path if not givem.

   add_path = 1;

   #ifdef WINDOWS
      if(strchr(fn, '\\')) add_path = 0;
      if(strchr(fn, ':')) add_path = 0;
   #endif

   #if defined(__linux__) || defined(__MACH__) || defined(__FreeBSD__)
      if(strchr(fn, '/')) add_path = 0;
   #endif

   play_tune(fn, add_path);
}


void BEEP(int why)
{
   #ifdef WINDOWS
      if(beep_on) MessageBeep(0);
   #else   // __linux__  __MACH__  __FreeBSD__
      #if USE_SDL
      #elif defined(USE_X11)
      	if(beep_on && display) XBell(display, 100);
      #endif
#endif

   if(show_beep_reason) sprintf(plot_title, "BEEP reason: %d", why);
}

void alarm_clock()
{
   if(alarm_file)           play_tune(ALARM_FILE, 1);
   else if(chord_file == 2) play_tune(USER_CHORD_FILE, 1);
   else if(chord_file)      play_tune(CHORD_FILE, 0);
   else                     BEEP(1);
}

void cuckoo_clock()
{
char fn[MAX_PATH+1];

   if(singing_clock) {  // sing a song file
      get_alarm_time();
      if((singing_clock == 2) && (g_minutes == 0)) {  // Big Ben mode - play a different file each hour on the hour
         sprintf(fn, HOUR_NAME, g_hours%12);
      }
      else {
         sprintf(fn, SONG_NAME, g_minutes);
      }
      play_tune(fn, 1);
      cuckoo_beeps = 0;
   }
   else if(ships_clock && (ring_bell >= 0) && (bell_number >= 0) && (bell_number < MAX_BELLS)) {
      if(bell_file && (bells[ring_bell][bell_number])) play_tune(BELL_FILE, 1);
      if(++bell_number >= MAX_BELLS) bell_number = (-1);
   }
   else if(chime_file) play_tune(CHIME_FILE, 1);
   else if(notify_file == 2) play_tune(USER_NOTIFY_FILE, 1);
   else if(notify_file) play_tune(NOTIFY_FILE, 0);
   else BEEP(2);
}

void click_sound(int why)
{
   // play a key click sound file

   // why=0 -> keyboard keys  why!=0 -> screen item clicks
   if(click_file && touch_screen) {  // key click sound
      play_tune(CLICK_FILE, 1);
   }
}



char *trim_whitespace(char *p)
{
char *s;

   // return pointer to first non-whitespace char in string p
   // returns 0 for null or empty string

   if(p == 0) return 0;

   s = p;
   while(*s) {
      if(*s == ' ') ++s;
      else if(*s == '\t') ++s;
      else return s;
   }
   return 0;
}


void run_program(char *pgm, int no_path)
{
char name[2048+1];
char edit_name[2048+1];      // edit_buffer with leading whitespace trimmed off
char shell_cmd[2048+64+1];
char *s;

   // spawn a system call to invoke a program (like a text editor)
   if(pgm == 0) return;
   if(no_exec) return;

   s = trim_whitespace(edit_buffer);
   if(s) {
      strcpy(edit_file, s);
      strcpy(name, s);
   }
   else {
      edit_file[0] = 0;
      name[0] = 0;
   }
   strcpy(edit_name, edit_file);

   #ifdef WINDOWS
      if(edit_name[0] == '\\') ;      // user gave a path
      else if(edit_name[0] == '/') ;  // user gave a path
      else if(edit_name[0] == 0) ;    // user gave no file name
      else if(no_path) ;
      else if(isalpha(edit_name[0]) && (edit_name[1] == ':')) ;
      else {  // add path to heather directory
         strcpy(name, heather_path);
         strcat(name, edit_name);
      }

      if(name[0] && (no_path == 0)) {
         _snprintf(shell_cmd,sizeof(shell_cmd)-1,"%s \"%s\"", pgm, name);
      }
      else {
         _snprintf(shell_cmd,sizeof(shell_cmd)-1,"%s", pgm);
      }

printf("run:(%s)\n", shell_cmd);
//sprintf(debug_text, "run:(%s)", shell_cmd);
      Win32Exec(shell_cmd, FALSE);
   #endif

   #ifdef __linux__ // __linux__
      if(edit_name[0] == '.') ;       // user gave a path
      else if(edit_name[0] == '/') ;  // user gave a path
      else if(edit_name[0] == 0) ;    // user gave no file name
      else if(no_path) ;
      else {  // add path to heather directory
         strcpy(name, heather_path);
         strcat(name, edit_name);
      }

      if(name[0] && (no_path == 0)) {
         sprintf(shell_cmd, "/bin/sh -c \"xterm -e %s %s\" &", pgm, name);
      }
      else {
         sprintf(shell_cmd, "/bin/sh -c \"xterm -e %s\" &", pgm);
      }

printf("run:(%s)\n", shell_cmd);
      system(shell_cmd);
   #endif

   #ifdef __MACH__ // __MACH__
      if(edit_name[0] == '.') ;       // user gave a path
      else if(edit_name[0] == '/') ;  // user gave a path
      else if(edit_name[0] == 0) ;    // user gave no file name
      else if(no_path) ;
      else {  // add path to heather directory
         strcpy(name, heather_path);
         strcat(name, edit_name);
      }

      if(name[0] && (no_path == 0)) {
         sprintf(shell_cmd, "/usr/x11/bin/xterm -e %s \"%s\" &", pgm, name);
      }
      else {
         sprintf(shell_cmd, "/usr/x11/bin/xterm -e %s &", pgm);
      }

printf("run:(%s)\n", shell_cmd);
      system(shell_cmd);
   #endif

   #ifdef __FreeBSD__ // __FreeBSD__
      if(edit_name[0] == '.') ;       // user gave a path
      else if(edit_name[0] == '/') ;  // user gave a path
      else if(edit_name[0] == 0) ;    // user gave no file name
      else if(no_path) ;
      else {  // add path to heather directory
         strcpy(name, heather_path);
         strcat(name, edit_name);
      }

      if(name[0] && (no_path == 0)) {
         sprintf(shell_cmd, "xterm -e %s \"%s\" &", pgm, name);
      }
      else {
         sprintf(shell_cmd, "xterm -e %s &", pgm);
      }

printf("run:(%s)\n", shell_cmd);
      system(shell_cmd);
   #endif
}

void text_editor()
{
char *e;

   // run a text editor program
   if(no_exec) return;

   e = getenv("HEATHER_EDITOR");
   if(!e || !*e) {
      e = getenv("EDITOR");
   }
   if(!e || !*e) {
      e = editor;
   }

   run_program(e, 0);
}

void exec_program()
{
char *e;

   // run a user defined program on a schedule
   if(no_exec) return;

   e = getenv("HEATHER_EXEC");
   if(!e || !*e) {
      e = heather_exec;
   }
   strcpy(edit_buffer, e);
   strcpy(out, e);

   run_program(out, 0);
}


int path_unlink(char *fn)
{
char name[2048+1];
char *f;

   // delete a file, add path name if none given
   f = trim_whitespace(fn);
   if(f == 0) return 1;

   strcpy(name, f);

#ifdef WINDOWS
#else // __linux__  __MACH__  __FreeBSD__
   if(f[0] == '.') ;       // user gave a path
   else if(f[0] == '/') ;  // user gave a path
   else {  // add path to heather directory
      strcpy(name, heather_path);
      strcat(name, f);
   }
#endif

   return unlink(name);
}

FILE *path_open(char *fn, char *m)
{
char name[2048+1];
char *f;
FILE *fp;

   // open a file, add path name if none given
   if(m == 0) return 0;
   f = trim_whitespace(fn);
   if(f == 0) return 0;

   strcpy(name, f);

#ifdef WINDOWS
#else // __linux__  __MACH__  __FreeBSD__
   if(f[0] == '.') ;       // user gave a path
   else if(f[0] == '/') ;  // user gave a path
   else {  // add path to heather directory
      strcpy(name, heather_path);
      strcat(name, f);
   }
#endif

   fp = fopen(name, m);
   if(fp) {
      if(debug_file) fprintf(debug_file, "! file %s opened\n", name);
   }
if(show_debug_info) printf("path_open(%s, %s) -> %p\n", name, m, fp);
   return fp;
}


FILE *topen(char *fn, char *m)
{
char *f;

   // open a file,  trim leading whitespace from the name
   if(m == 0) return 0;
   f = trim_whitespace(fn);
   if(f == 0) return 0;

if(show_debug_info) printf("topen(%s, %s)\n", f,m);  //zorky
   return path_open(f, m);
}

FILE *open_debug_file(char *f)
{
char *s;
char *star;

   s = trim_whitespace(f);
   if(s == 0) return 0;

   if(debug_file) {
      fclose(debug_file);
      debug_file = 0;
   }

   star = strchr(s, FLUSH_CHAR);
   if(star) {
      *star = 0;
      dbg_flush_mode = 1;
   }
   else dbg_flush_mode = 0;

   debug_file = topen(s, "w");
   strcpy(debug_name, s);
   return debug_file;
}


void set_rinex_name()
{
   // create an IGS format RINEX observation file name

   get_clock_time();   // get system clock time in UTC
   sprintf(out, "rinx%03d%c.%02do", day_of_year(clk_month,clk_day), 'a'+(clk_hours%24), clk_year%100);
}

FILE *open_rinex_file(char *f)
{
char *s;
char *star;

   s = trim_whitespace(f);
   if(s == 0) return 0;

   if(rinex_file) {
      fclose(rinex_file);
      rinex_file = 0;
   }

   star = strchr(s, FLUSH_CHAR);
   if(star) {
      *star = 0;
      rinex_flush_mode = 1;
   }
   else rinex_flush_mode = 0;

   rinex_file = topen(s, "w");
   if(rinex_file) {
      if(debug_file) fprintf(debug_file, "! RINEX file %s opened\n", s);
   }
   strcpy(rinex_name, s);
   rinex_header_written = 0;
   first_obs = 1;
   return rinex_file;
}

FILE *open_raw_file(char *name, char *mode)
{
char *s;
FILE *file;

   if(name == 0) return 0;
   if(mode == 0) return 0;

   if(raw_file) {
      fclose(raw_file);
      raw_file = 0;
   }

   s = strchr(name, FLUSH_CHAR);
   if(s) {
      *s = 0;
      raw_flush_mode = 1;
   }
   else raw_flush_mode = 0;

//lfs drain_port(RCVR_PORT);
   wakeup_tsip_msg = 0;        // cause wait for full receiver message before logging raw data
   saw_rcvr_msg = 0;
   file = topen(name, mode);
   if(file) {
      if(debug_file) fprintf(debug_file, "! raw capture file %s opened\n", name);
   }
   wakeup_tsip_msg = 0;        // cause wait for full receiver message before logging raw data
   saw_rcvr_msg = 0;

   return file;
}

FILE *open_prn_file(char *name, char *mode)
{
char *s;
FILE *file;

   if(name == 0) return 0;
   if(mode == 0) return 0;

   if(prn_file) {
      fclose(prn_file);
      prn_file = 0;
      prn_count = 0;
   }

   s = strchr(name, FLUSH_CHAR);
   if(s) {
      *s = 0;
      prn_flush_mode = 1;
   }
   else prn_flush_mode = 0;

//lfs drain_port(RCVR_PORT);
   file = topen(name, mode);
   if(file) {
      fprintf(file, "#JD UTC         PRN   AZ     EL     SIG\n");
      fprintf(file, "# 1\n");
      if(prn_flush_mode) fflush(file);
      prn_count = 1;
      if(debug_file) fprintf(debug_file, "! PRN file %s opened\n", name);
   }

   return file;
}

void sync_file(FILE *file)
{
   // make sure file buffer contents are written to disk

   if(file == 0) return;

   fflush(file);

   #ifdef WINDOWS
   #else // __linux__  __MACH__  __FreeBSD__
      fsync(fileno(file));
   #endif
}


int get_clock_time()
{
   // get the system clock into the clk_ variables

   #ifdef WINDOWS
      SYSTEMTIME t;
      GetSystemTime(&t);
      clk_year = t.wYear;
      clk_month = t.wMonth;
      clk_day = t.wDay;
      clk_hours = t.wHour;
      clk_minutes = t.wMinute;
      clk_seconds = t.wSecond;
      clk_frac = ((double) t.wMilliseconds) / 1000.0;

      clk_jd = jdate(clk_year,clk_month,clk_day);
      clk_jd += jtime(clk_hours,clk_minutes,clk_seconds,clk_frac);

      return 1;
   #endif

   #if defined(__linux__) || defined(__MACH__) || defined(__FreeBSD__)
      double t0;
      GetNsecs();   // get wall_time as seconds since epoch (with high-res fractional seconds)

      t0 = wall_time / (24.0*60.0*60.0); // convert seconds to days
      t0 += LINUX_EPOCH;  // add in Linux epoch

      gregorian(0, t0);          // convert to gregorian
      clk_year = g_year;
      clk_month = g_month;
      clk_day = g_day;
      clk_hours = g_hours;
      clk_minutes = g_minutes;
      clk_seconds = g_seconds;
      clk_frac = g_frac;

      clk_jd = jdate(clk_year,clk_month,clk_day);
      clk_jd += jtime(clk_hours,clk_minutes,clk_seconds,clk_frac);
      return 1;
   #endif

   return 0;
}



void set_cpu_clock()
{
int hhh, mmm, sss;
double milli;
double rcvr_jd;
double delta_jd;
int time_set;

   // This routine sets the system clock to the GPS receiver time
   // It can do this when requested, on a regular interval or whenever
   // the two clocks diverge by a specified amount.
   //
   // On Linux and OSX Heather must have root privledges to set the system clock.

   if(have_time == 0) return;         // we have no GPS receiver time
   if(have_timing_mode == 0) return;  // we don't know if the receiver time is UTC or GPS
   if(saw_icm) {
      if(time_flags & TFLAGS_BAD_TIME) return;   // GPS receiver time is not valid
   }
   else if(time_flags & (TFLAGS_BAD_TIME | TFLAGS_GLONASS)) return; // GPS receiver time is not valid  // !!!! saw_icm?

   if(fake_time_stamp) return;          // dont sync on faked UCCM time stamps

   if(set_system_time) ;        // a time set is scheduled
   else if(set_time_anytime) ;  // set time if clock has drifted
   else return;                 // nothing to do here, move along

   rcvr_jd = jd_utc;

   if(set_system_time > 1) {    // waiting for system to stabilze before setting time
      --set_system_time;
      if(set_system_time == 0) goto set_clock;
      return;
   }
   else if(set_system_time < 0) {  // dont resync time if we just did it
      ++set_system_time;
      return;
   }

   if(time_flags & TFLAGS_UTC) {  // receiver time is in UTC
      if(force_utc_time == 0) {  // we want to set system clock to GPS time
         rcvr_jd += ((double) utc_offset) / (24.0*60.0*60.0);
      }
   }
   else {  // receiver time is in GPS time
      if(force_utc_time) {  // we want to set system clock to UTC time
         rcvr_jd -= ((double) utc_offset) / (24.0*60.0*60.0);
      }
   }

   // adjust the receiver message time for the offset between when it arrived
   // and the true time
   rcvr_jd += ((time_sync_offset / 1000.0) / (24.0*60.0*60.0));

   // now we set the system time from the GPS receiver
   milli = 0.0;
   if(set_time_anytime) {  // set system clock anytime x milliseconds of drift is seen
      get_clock_time();    // get the system clock in UTC

      delta_jd = (rcvr_jd - clk_jd) * 24.0*60.0*60.0 * 1000.0;
      milli = fabs(time_sync_median(delta_jd));
//    milli = fabs(delta_jd);  // milliseconds of divergence between the system and receiver

//sprintf(debug_text2, "tset %.1f: delta:%f  tflags:%02X  futc:%d  tso:%g  jdo:%g",
//set_time_anytime, milli, time_flags,force_utc_time, time_sync_offset, delta_jd);  // rrrrrrr

      if(milli >= set_time_anytime) {  // we need a time set
         need_time_set();
         return;
      }
   }

   if(set_system_time == 0) return;   // we are not setting the time

   set_clock:
   time_set = 0;

   gregorian(0, rcvr_jd);   // convert needed to to broken down format

   #ifdef WINDOWS
      hhh = g_hours;
      mmm = g_minutes;
      sss = g_seconds;
   #endif
   #if defined(__linux__) || defined(__MACH__) || defined(__FreeBSD__)
      hhh = g_hours;
      mmm = g_minutes;
      sss = g_seconds;
   #endif

   // adjust receiver time for message offset delay to get true time
   #ifdef WINDOWS
      SYSTEMTIME t;
      milli = (long) (g_frac * 1000.0);

      t.wYear = g_year;
      t.wMonth = g_month;
      t.wDay = g_day;
      t.wHour = hhh;
      t.wMinute = mmm;
      t.wSecond = sss;
      t.wMilliseconds = (int) (milli + 0.50);
      time_set = (int) SetSystemTime(&t);
   #endif

   #if defined(__linux__) || defined(__MACH__) || defined(__FreeBSD__)
      struct timeval tv;
      time_t rawtime;
      double gmt;
      struct tm *tt;

      rcvr_jd -= LINUX_EPOCH;       // convert receiver time to Linux epoch
      rcvr_jd *= (24.0*60.0*60.0);  // convert days to seconds

      tv.tv_sec = (time_t) rcvr_jd;
      rcvr_jd -= floor(rcvr_jd);
      tv.tv_usec = (suseconds_t) (rcvr_jd * 1.0E6);

      time_set = settimeofday(&tv, 0);
      if(time_set == 0) time_set = 1;
      else time_set = 0;
   #endif

   if(time_set) {  // clock has been set
      if(!set_time_anytime) { // don't annoy if doing periodic time sets
         BEEP(3);
      }
   }

   set_system_time = (-3);  // time has been set, don't set it again for at least three seconds
}


int sim_kbd_check()
{
    // this routine is used to determine how often to let Heather check
    // for a keystroke while reading a simulation file.

    if(pause_data) return 0;

    if(rcvr_type == TICC_RCVR) {
       if(1 || sim_delay) com_tick = (com_tick + 1) % 100;  // aaaaahhhhh
       else               com_tick = (com_tick + 1) % 20000;
    }
    else com_tick = (com_tick + 1) % 100;
//  else com_tick = (com_tick + 1) % 20000;

    if(com_tick > 2) return 1;
    else             return 0;
}


u08 get_sim_char(unsigned port)
{
int flag;
u08 c;

   // get a character from a simulation file.
   if(sim_file == 0) return 0;  // simulation file not being used

   if(sim_eof) {
      return 0;
   }

   flag = fread(&c, 1, 1, sim_file);
   if(flag <= 0) {  // just reached the end of file
      BEEP(4);
      sim_eof = 1;

      sprintf(plot_title, "Reached end of simulation file: %s", sim_name);
      refresh_page();
      return 0;
   }

   if((zoom_screen == 'O') && (port == monitor_port)) {   // send char to "terminal" format monitor screen
      echo_term(monitor_port, c, monitor_hex, 0);
   }
   return c;
}


#define KBD_MSECS 250.0
void reset_kbd_timer()
{
   // used to make sure the keyboard gets checked during periods of heavy
   // traffic from the receiver

   last_kbd_msec = GetMsecs() + KBD_MSECS;
}


#ifdef WINDOWS     // WINDOWS OS dependent I/O routines

#include "timeutil.cpp"
#include "ipconn.cpp"

struct LH_IPCONN : public IPCONN
{
   // for TCP/IP connection

   C8 message[1024];

   virtual void message_sink(IPMSGLVL level,  C8 *text)
   {
      memset(message, 0, sizeof(message));      // copy error/notice message to an array where app can see it
      strncpy(message, text, sizeof(message)-1);
      message[1023] = 0;

      printf("%s\n", message);
   }

   virtual void on_lengthy_operation(void)
   {
      update_pwm();  // e.g., while send() is blocking
   }
};

LH_IPCONN *IPC[NUM_COM_PORTS];

HANDLE hSerial[NUM_COM_PORTS];
DCB    dcb[NUM_COM_PORTS];

void SetDtrLine(unsigned port, u08 on)
{
   if(port >= NUM_COM_PORTS) return;

   if(hSerial[port] != INVALID_HANDLE_VALUE) {
      EscapeCommFunction(hSerial[port], on ? SETDTR : CLRDTR);
   }
}

void SetRtsLine(unsigned port, u08 on)
{
   if(port >= NUM_COM_PORTS) return;

   if(hSerial[port] != INVALID_HANDLE_VALUE) {
      EscapeCommFunction(hSerial[port], on ? SETRTS : CLRRTS);
   }
}

void SetBreak(unsigned port, u08 on)
{
   if(port >= NUM_COM_PORTS) return;

   if(hSerial[port] != INVALID_HANDLE_VALUE) {
      EscapeCommFunction(hSerial[port], on ? SETBREAK : CLRBREAK);
   }
}


void kill_com(unsigned port, int why)
{
   if(port >= NUM_COM_PORTS) return;
// if(debug_file) fprintf(debug_file, "kill com %d: %d\n", port,why);
   com[port].port_used = 1;  // port has been closed

   if(com[port].com_port > 0) {       // COM port in use: close it
      SetDtrLine(port, 0);
      if(hSerial[port] != INVALID_HANDLE_VALUE) CloseHandle(hSerial[port]);
      hSerial[port] = INVALID_HANDLE_VALUE;
   }
   else {                    // TCP connection: close the connection
      if(hSerial[port] != INVALID_HANDLE_VALUE) CloseHandle(hSerial[port]);
      hSerial[port] = INVALID_HANDLE_VALUE;
      if(IPC[port] != NULL) {
         delete IPC[port];
         IPC[port] = NULL;
      }
   }

   com[port].com_running = (-1000-why); //NOT_RUNNING;
}

void init_tcpip(unsigned port)
{
   if(port >= NUM_COM_PORTS) return;

   IPC[port] = new LH_IPCONN();

   IPC[port]->connect(com[port].IP_addr, DEFAULT_PORT_NUM);

   if(!IPC[port]->status()) {
      com[port].process_com = 0;
      com[port].com_error |= INIT_ERR;
      return;
      error_exit(2, IPC[port]->message);
   }
   com[port].port_used = 3;  // port open for TCP/IP
}

void init_com(unsigned port, int why)
{
   // open communications to the receiver
   if(port >= NUM_COM_PORTS) return;

   kill_com(port, why);   // in case COM port already open
// if(debug_file) fprintf(debug_file, "! init_com: port:%d  why:%d  rcvr_run:%d\n", port,why, com[RCVR_PORT].com_running);
   com[port].com_error = 0;
   if((port == RCVR_PORT) && (rcvr_type == NO_RCVR) && (enable_terminal == 0)) {
      return;
   }
if((port == RCVR_PORT) && (rcvr_type == TIDE_RCVR) && (enable_terminal == 0)) {
   return;
}

   crlf_seen = 0;
   if((com[port].com_port == 0) && com[port].IP_addr[0]) {
      //
      // TCP: In Windows, COM0 with process_com=TRUE means we're using TCP/IP
      //
      if(com[port].process_com) {
         init_tcpip(port);
         com[port].com_running = TCPIP_RUNNING;
         com[port].com_data_lost = 0;
         com[port].last_com_time = this_msec = GetMsecs();
         if(port == RCVR_PORT) first_request = 1;
         Sleep(COM_INIT_SLEEP);
      }
      return;
   }
if(com[port].com_port == 0) return;

   // kd5tfd hack to handle comm ports > 9
   // see http://support.microsoft.com/default.aspx?scid=kb;%5BLN%5D;115831
   // for the reasons for the bizarre comm port syntax

   char com_name[256+1];
   sprintf(com_name, "\\\\.\\COM%d", com[port].com_port);
   hSerial[port] = CreateFile(com_name,
                        GENERIC_READ | GENERIC_WRITE,
                        0,
                        0,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        0
   );

   if(hSerial[port] == INVALID_HANDLE_VALUE) {
      sprintf(out, "Can't open com port: %s", com_name);
      com[port].process_com = 0;
      com[port].com_error |= INIT_ERR;
      kill_com(port, 2000);
      return;
      error_exit(10001, out);
   }

   dcb[port].DCBlength = sizeof(dcb[RCVR_PORT]);
   if(!GetCommState(hSerial[port], &dcb[port])) {
      error_exit(10002, "Can't GetCommState()");
   }

   dcb[port].BaudRate         = com[port].baud_rate;

   dcb[port].ByteSize         = com[port].data_bits;

   if     (com[port].parity == ODD_PAR)  dcb[port].Parity = ODDPARITY;  // !!!! (parity == 1)
   else if(com[port].parity == EVEN_PAR) dcb[port].Parity = EVENPARITY;
   else                                  dcb[port].Parity = NOPARITY;

   if(com[port].stop_bits == 2) dcb[port].StopBits = TWOSTOPBITS;
   else                         dcb[port].StopBits = ONESTOPBIT;

   dcb[port].fBinary          = TRUE;
   dcb[port].fOutxCtsFlow     = FALSE;
   dcb[port].fOutxDsrFlow     = FALSE;
   dcb[port].fDtrControl      = DTR_CONTROL_ENABLE;
   dcb[port].fDsrSensitivity  = FALSE;
   dcb[port].fOutX            = FALSE;
   dcb[port].fInX             = FALSE;
   dcb[port].fErrorChar       = FALSE;
   dcb[port].fNull            = FALSE;
   dcb[port].fRtsControl      = RTS_CONTROL_ENABLE;
   dcb[port].fAbortOnError    = FALSE;

   if(!SetCommState(hSerial[port], &dcb[port])) {
      error_exit(10003, "Can't SetCommState()");
   }

   // set com port timeouts so we return immediately if no serial port
   // character is available
   COMMTIMEOUTS cto = { 0, 0, 0, 0, 0 };
   cto.ReadIntervalTimeout = MAXDWORD;
   cto.ReadTotalTimeoutConstant = 0;
   cto.ReadTotalTimeoutMultiplier = 0;

   if(!SetCommTimeouts(hSerial[port], &cto)) {
      error_exit(10004, "Can't SetCommTimeouts()");
   }

   SetDtrLine(port, 1);
   if((rcvr_type == ACRON_RCVR) && (port == RCVR_PORT)) {  // power up the RS-232 interface
      SetDtrLine(port, 1);  // drive +12V
      SetRtsLine(port, 0);  // drive -12V
      Sleep(500);
   }
   com[port].com_running = SERIAL_RUNNING;
   if(port == RCVR_PORT) first_request = 1;

   if(modem_alarms) reset_alarm();
   com[port].port_used = 2;   // serial port open

   Sleep(COM_INIT_SLEEP);
//eee sprintf(debug_text4, "init port(%d):%d  msecs:%.0f  process:%d  running:%d",
//eee com[port].com_port, port, GetMsecs(), com[port].process_com, com[port].com_running);
}


int check_incoming_data(unsigned port)
{
   // return true if com device has a character available
   if(port >= NUM_COM_PORTS) return FALSE;

   if((port == RCVR_PORT) && (rcvr_type == NO_RCVR) && (enable_terminal == 0)) { // alternate ready/not ready each check
      return (++com_tick & 0x01);
   }
if((port == RCVR_PORT) && (rcvr_type == TIDE_RCVR) && (enable_terminal == 0)) {
   return (++com_tick & 0x01);
}

   if((port == RCVR_PORT) && sim_file) {  // using a simulation file
      return sim_kbd_check();
   }

   if(com[port].com_error & RCV_ERR) return TRUE; // !!!! return 0;
   if(com[port].next_rcv_byte < com[port].rcv_byte_count) return TRUE;  // we have already read and buffered the char

   com[port].rcv_byte_count = 0;
   com[port].next_rcv_byte = 0;

   if(com[port].com_port != 0) {       // COM port in use: read bytes from serial port
if(hSerial[port] == INVALID_HANDLE_VALUE) return FALSE;
      ReadFile(hSerial[port], &com[port].rcv_buffer[0], RCV_BUF_SIZE, &com[port].rcv_byte_count, NULL);
      if(com[port].rcv_byte_count == 0) return FALSE;  // no serial port data is available
   }
   else {                     // TCP connection: read a byte from Winsock
      if(!IPC[port]->status()) {
          error_exit(22, IPC[port]->message);
      }

      com[port].rcv_byte_count = IPC[port]->read_block(com[port].rcv_buffer, RCV_BUF_SIZE);

      if(com[port].rcv_byte_count == 0) return FALSE;
   }

   return TRUE;
}

u08 get_serial_char(unsigned port)
{
int flag;
u08 c;
DWORD err;

   // return the next byte from a com device

   if(port >= NUM_COM_PORTS) return 0;

   if((port == RCVR_PORT) && sim_file) {  // get char from a simulation file
      return get_sim_char(port);
   }

   if((port == RCVR_PORT) && (rcvr_type == NO_RCVR) && (enable_terminal == 0)) { // alternate ready/not ready each check
      return 0;
   }
if((port == RCVR_PORT) && (rcvr_type == TIDE_RCVR) && (enable_terminal == 0)) {
   return 0;
}

   if(com[port].process_com == 0) return 0;  //lfs
   if(com[port].com_error & RCV_ERR) return 0;

   if(com[port].next_rcv_byte < com[port].rcv_byte_count) {   // return byte previously fetched by check_incoming_data()
      c = com[port].rcv_buffer[com[port].next_rcv_byte++];
      if((c == 0x0D) || (c == 0x0A)) crlf_seen = 1;

      if((zoom_screen == 'O') && (port == monitor_port)) {   // send char to "terminal" format monitor screen
         echo_term(monitor_port, c, monitor_hex, 0);
      }
      return c;
   }

   com[port].rcv_byte_count = 0;
   com[port].next_rcv_byte = 0;

   while(com[port].rcv_byte_count == 0) {  // wait until we have a character
      if(com[port].com_port != 0)  {       // COM port in use: read a byte from serial port
if(hSerial[port] == INVALID_HANDLE_VALUE) return FALSE;
         flag = ReadFile(hSerial[port], &com[port].rcv_buffer[0], RCV_BUF_SIZE, &com[port].rcv_byte_count, NULL);
         // !!!!!! test for parity errors, etc and set rcv_error
         ClearCommError(hSerial[port], &err, NULL);
         if(err & (CE_FRAME | CE_OVERRUN | CE_RXPARITY | CE_RXOVER)) {
            com[port].rcv_error |= err;
         }
      }
      else {                     // TCP connection: read a byte from Winsock
         if(!IPC[port]->status()) {
            error_exit(23, IPC[port]->message);
         }

         flag = com[port].rcv_byte_count = IPC[port]->read_block(com[port].rcv_buffer, RCV_BUF_SIZE);
      }

      update_pwm();   // if doing pwm temperature control
   }

   if(flag && com[port].rcv_byte_count) {  // succesful read
      c = com[port].rcv_buffer[com[port].next_rcv_byte++];
      if((c == 0x0D) || (c == 0x0A)) crlf_seen = 1;

      if((zoom_screen == 'O') && (port == monitor_port)) {   // send char to "terminal" format monitor screen
         echo_term(monitor_port, c, monitor_hex, 0);
      }
      return c;
   }

   if(com_error_exit) {
      error_exit(222, "Serial receive error");
   }
   else {
      com[port].com_error |= RCV_ERR;
      ++com_errors;
   }
   return 0;
}

void sendout(unsigned port, unsigned char val, int eom_flag)
{
DWORD written;
int flag;
static S32 x = 0;

   // Ssend data to the receiver.
   // We queue up data until end of message to reduce per-packet overhead.
   // If eom_flag == FLUSH_COM then send buffered chars without adding a new one
   // If eom_flag == 0, add char to buffer (sends buffer if full)
   // If eom_flag != 0, add char to buffer and send it

   if(port >= NUM_COM_PORTS) return;

   if((port == RCVR_PORT) && (rcvr_type == NO_RCVR) && (enable_terminal == 0)) { // alternate ready/not ready each check
      return;
   }
if((port == RCVR_PORT) && (rcvr_type == TIDE_RCVR) && (enable_terminal == 0)) { // alternate ready/not ready each check
   return;
}

   if(com[port].process_com == 0) return;
   if(com[port].com_error & XMIT_ERR) return;
   if(just_read && (port == RCVR_PORT)) return;
   if(no_send && (port == RCVR_PORT)) return;
   if(sim_file) return;  // don't send anything out if reading a simulation file

   if(FAKE_ACRON_RCVR && (rcvr_type == ACRON_RCVR) && (port == RCVR_PORT)) {  // simulate echo by sticking char in rcv buffer
      com[port].rcv_buffer[com[port].next_rcv_byte] = val;
      ++com[port].rcv_byte_count;
      return;
   }


   if((zoom_screen == 'O') && (port == monitor_port)) {  // echo data to "terminal" monitor screen
      echo_term(port, (unsigned) val, monitor_hex, 1);
   }

   // we buffer up output chars until buffer full or end of message flag seen
   if(eom_flag == FLUSH_COM) {  // just send buffered chars
      if(x == 0) return;   // nothing to flush
   }
   else com[port].xmit_buffer[x++] = val;  // add char to buffer
   if((x < (S32) XMIT_BUF_SIZE) && (eom_flag == ADD_CHAR)) return;

   update_pwm();   // if doing pwm temperature control

   if(com[port].com_port != 0) {       // COM port in use: send bytes via serial port
      if(hSerial[port] == INVALID_HANDLE_VALUE) return;
      flag = WriteFile(hSerial[port], com[port].xmit_buffer, x, &written, NULL);
      if(written == x) written = 1;
      else             written = 0;
   }
   else {                    // TCP connection: send byte via Winsock
      IPC[port]->send_block(com[port].xmit_buffer, x);

      if(!IPC[port]->status()) {
         error_exit(24, IPC[port]->message);
      }

      flag = 1;      // success
      written = 1;
   }

   x = 0;

   if((flag == 0) || (written != 1)) {
      if(com_error_exit) {
         error_exit(3, "Serial transmit error");
      }
      else {
         com[port].com_error |= XMIT_ERR;
         ++com_errors;
      }
   }
}


void SendBreak(unsigned port)
{
   // This routine sends a 300 msec BREAK signal to the serial port
   if(port >= NUM_COM_PORTS) return;

   SetBreak(port, 1);
   Sleep(300);
   SetBreak(port, 0);
   Sleep(100);
}

void init_hardware(void)
{
   init_com(RCVR_PORT, 1);
   init_screen(9102);

   if(NO_SCPI_BREAK && (rcvr_type == SCPI_RCVR)) ;
   else if(nortel == 0) {    // To wake up a Nortel NTGS55A receiver
      SendBreak(RCVR_PORT);
   }

   // Arrange to call serve_gps() 5x/second while dragging or displaying the
   // command line help dialog to maintain screen/com updates.
   //
   // !!!  note:  this seems to cause random unhandled exception aborts
   //             maybe due to recursive calls to serve_gps().  Use the /kt
   //             command line option to not use this timer.
   if(enable_timer) {
      SetTimer(hWnd, 0, 200, NULL);
      timer_set = 1;
   }
   sal_ok = 1;
   need_screen_init = 0;
}


double GetNsecs()
{
SYSTEMTIME t;

   // set wall_time to system clock time with millisecond accuracy
   // and return elapsed nanosecond count

   GetSystemTime(&t);
   wall_time = ((double) t.wHour) * (24.0*60.0*60.0);
   wall_time += (((double) t.wMinute) * 60.0);
   wall_time += ((double) t.wSecond);
   wall_time += (((double) t.wMilliseconds) / 1000.0);

   return GetMsecs() * 1.0E6;
}

double GetMsecs()
{
LARGE_INTEGER ElapsedMicroseconds;
LARGE_INTEGER Frequency;
double t,f;

   // we normally use the high resolution peformance counter here
   // but the standard tick counter works fine if your system has TSC issues

   if(use_tsc == 0) {  // don't use the performance counter for high-res interval clock
      return (double) (unsigned long) GetTickCount();
   }
   else {
      QueryPerformanceFrequency(&Frequency);
      QueryPerformanceCounter(&ElapsedMicroseconds);

      //
      // We now have the elapsed number of ticks, along with the
      // number of ticks-per-second. We use these values
      // to convert to the number of elapsed milliseconds (in double precision).
      //
      f = (double) (unsigned long long) Frequency.QuadPart;
      t = (double) (unsigned long long) ElapsedMicroseconds.QuadPart;

      if(f) t = (t*1000.0) / f;
      return t;
   }
}


//****************************************************************************
//
// Window message receiver procedure for application
//
// We implement a small keystroke queue so that the DOS kbhit()/getch()
// keyboard i/o model works with the Windows message queue i/o model.
// Keystrokes are put into the queue by the Windows message reciever proc.
// They are drained from the queue by the program keyboard handler.
//
//****************************************************************************

#define get_kbd() win_getch()
#define KBD_Q_SIZE 16
int kbd_queue[KBD_Q_SIZE+1];
int kbd_in, kbd_out;

void add_kbd(int key)
{
   // add keystroke value to the keyboard queue

   last_kbd_jd = jd_utc;
   if(++kbd_in >= KBD_Q_SIZE) kbd_in = 0;
   if(kbd_in == kbd_out) {  // kbd queue is full
      if(--kbd_in < 0) kbd_in = KBD_Q_SIZE-1;
   }
   else {  // put keystoke in the queue
      kbd_queue[kbd_in] = key;
   }
}

void flush_kbd()
{
   kbd_in = kbd_out = 0;
   edit_buffer[0] = 0;
   getting_string = 0;
}

int win_kbhit(void)
{
   // return true if a keystroke is available
   reset_kbd_timer();
   return (kbd_in != kbd_out);  // return true if anything is in the queue
}

int win_getch(void)
{
int key;

   // return the next get from the keystroke queue
   if(kbd_in == kbd_out) return 0;            // no keys in the queue

   if(++kbd_out >= KBD_Q_SIZE) kbd_out = 0;   // get to next queue entry
   key = kbd_queue[kbd_out];                  // get keystroke from queue
   return key;                                // return it
}


//****************************************************************************
// Win32Exec: Create process and return immediately, without doing a
//            WaitForInputIdle() call like WinExec()
//
// Optionally, wait for process to terminate
//****************************************************************************

BOOL Win32Exec(LPSTR lpCmdLine, BOOL bWait)
{
   STARTUPINFO         StartInfo;
   PROCESS_INFORMATION ProcessInfo;
   BOOL                result;

   memset(&StartInfo,   0, sizeof(StartInfo));
   memset(&ProcessInfo, 0, sizeof(ProcessInfo));

   StartInfo.cb = sizeof(StartInfo);

   printf("Win32Exec(%s)\n",lpCmdLine);

   result = CreateProcess(NULL,         // Image name (if NULL, module name must be first whitespace-delimited token on lpCmdLine or ambiguous results may occur)
                          lpCmdLine,    // Command line
                          NULL,         // Process security
                          NULL,         // Thread security
                          FALSE,        // Do not inherit handles
                          0,            // Creation flags
                          NULL,         // Inherit parent environment
                          NULL,         // Keep current working directory
                         &StartInfo,    // Startup info structure
                         &ProcessInfo); // Process info structure

   if (bWait)
      {
      WaitForSingleObject(ProcessInfo.hProcess,
                          INFINITE);
      }

   return result;
}


#else // __linux__  __MACH__  __FreeBSD__ OS dependent I/O routines


struct addrinfo hints = { 0 };  // for TCP/IP connection
struct addrinfo *res;


void outp(unsigned port, unsigned val)
{
   // place holder for doing fan control via I/O port bits
   // zork!!!
}

void SetDtrLine(unsigned port, u08 on)
{
int val;

   if(port >= NUM_COM_PORTS) return;

   if(com[port].com_fd < 0) return;  // !!!! zork what if TCPIP connected

   val = TIOCM_DTR;
   if(on) ioctl(com[port].com_fd, TIOCMBIS, &val);
   else   ioctl(com[port].com_fd, TIOCMBIC, &val);
}

void SetRtsLine(unsigned port, u08 on)
{
int val;

   if(port >= NUM_COM_PORTS) return;

   if(com[port].com_fd < 0) return;  // !!!! zork what if TCPIP connected

   val = TIOCM_RTS;
   if(on) ioctl(com[port].com_fd, TIOCMBIS, &val);
   else   ioctl(com[port].com_fd, TIOCMBIC, &val);
}

void SetBreak(unsigned port, u08 on)
{
   if(port >= NUM_COM_PORTS) return;

   if(com[port].com_fd < 0) return;  // !!!! zork what if TCPIP connected

   if(on) ioctl(com[port].com_fd, TIOCSBRK, 0);
   else   ioctl(com[port].com_fd, TIOCCBRK, 0);
}


void SendBreak(unsigned port)
{
   // This routine sends a 300 msec BREAK signal to the serial port
   if(port >= NUM_COM_PORTS) return;
   if(com[port].com_fd < 0) return;  // !!!! should also return if IP connected

// tcsendbreak(com[port].com_fd, 0);  // send a 200..500 ms (supposedly) break
   SetBreak(port, 1);  // a more reliable way to do the BREAK
   Sleep(500);
   SetBreak(port, 0);
   Sleep(100);
}


void kill_com(unsigned port, int why)
{
   // close the serial communications device
   if(port >= NUM_COM_PORTS) return;
   com[port].port_used = 1;   // port has been closed

   if(com[port].com_fd < 0) return;

   tcsetattr(com[port].com_fd, TCSANOW, &oldtio);
   close(com[port].com_fd);

   com[port].com_fd = (-1);
   com[port].com_running = (-11); //NOT_RUNNING;
}


void init_tcpip(unsigned port)
{
static int portno = 45000;
static int use_v6 = 0;
char ip_string[512];
char port_string[80];
char *s,*eb;
int x;
int gai_err;
int af_given;

   // establish a TCPIP connection to a remote device server
   if(port >= NUM_COM_PORTS) return;
portno = 45000;
use_v6 = 0;

printf("init tcpip[%d] to %s\n", port, com[port].IP_addr);  // zork - show_debug_info
   strcpy(ip_string, com[port].IP_addr);

   //see if this is a bracketed IPv6 address
   eb = strstr(ip_string, "]");
   if(eb && (ip_string[0]=='[')) { //bracketed IPv6 address.
printf("bracketed IPv6 address\n");      // zork - show_debug_info
      //Remove the brackets and leave a zero terminated address.
      *eb=0;
      memmove(ip_string,ip_string+1,eb-ip_string);
      //check for the : after the ]
      s = strstr(eb+1, ":");
      use_v6 = 1;
      af_given = 1;
   } else { // see if IPv6 is forced by the ';' portnumber separator
      s = strstr(ip_string, ";");
      if(s) {  // IPv6 address detected
         use_v6 = 1;
         af_given = 1;
      }
      else {   // no ';' - assume IPv4 (we try that first)
         s = strstr(ip_string, ":");
         use_v6 = 0; // This sets the default address space.
         af_given = 0;
      }
   }


   if(s) {  // we have a port number string
      *s = 0;                         // chop off the port number string
      if(*(s+1)) portno = atoi(s+1);  // get the port number as an integer
   }
   if(portno < 0) portno = 45000;
   sprintf(port_string, "%d", portno);
printf("ip:%s  port:%s  use_v6:%d\n", ip_string, port_string, use_v6); // zork - show_debug_info

   retry_resolve:
   if(use_v6) hints.ai_family = AF_INET6;
   else       hints.ai_family = AF_INET;
   hints.ai_socktype = SOCK_STREAM;

   //see if we can get an address in the chosen family
   gai_err = getaddrinfo(ip_string, port_string, &hints, &res);
   if(gai_err && !af_given) {
        //we could not get an address in the default address space.
        //try the other address space
printf("No address in the chosen ai_family, trying the other.\n");  // zork - show_debug_info
        use_v6 = !use_v6;
        af_given = 1;
        if(use_v6) hints.ai_family = AF_INET6;
        else       hints.ai_family = AF_INET;
        gai_err = getaddrinfo(ip_string, port_string, &hints, &res);
   }

   if(gai_err) {
printf("ERROR, could not get IP addr info:%s  port:%s. err=%d (%s)\n", ip_string, port_string, gai_err, gai_strerror(gai_err));
      com[port].process_com = 0;
      com[port].usb_port = 0;
      com[port].com_port = 0;
      com[port].com_error |= INIT_ERR;
      return;
   }

   com[port].com_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
   if(com[port].com_fd < 0) {
      if(!af_given) { // The local machine can not handle this address family. Try the other address family.
        use_v6 = !use_v6;
        af_given = 1;
        close(com[port].com_fd);
        com[port].com_fd = (-1);
        goto retry_resolve;
      }
      error_exit(20000, "ERROR could not open IP socket");
   }
printf("%s socket opened!\n", use_v6?"IPv6":"IPv4");
printf("Attempting to connect() to socket\n");

   if(connect(com[port].com_fd, res->ai_addr, res->ai_addrlen) < 0) {
      if(!af_given) {
        // we can not connect using that address family. Try the other.
        // this happens when the machine has link local IPv6 or IPv4 only, and we are trying
        // to reach a remote IPv6/IPv4 address. In this case we get an immediate error,
        // and can retry using the other protocol if the other address is dual-stacked.
        use_v6 = !use_v6;
        af_given = 1;
        close(com[port].com_fd);
        com[port].com_fd = (-1);
        goto retry_resolve;
      }

      error_exit(20002, "ERROR connecting to IP socket.");
   }

printf("connected!\n");
   x = fcntl(com[port].com_fd, F_GETFL, 0);
   fcntl(com[port].com_fd, F_SETFL, x | O_NONBLOCK);
   com[port].port_used = 3;  // port open for TCP/IP
printf("set ip connection to non-blocking\n");
}



void init_com(unsigned port, int why)
{
int col, i;
int sync;

   // Initialize the serial device for asynchronous non-blocking communication
   // The device can be a serial port or ethernet connection
   if(port >= NUM_COM_PORTS) return;

   kill_com(port, why);   // in case COM port already open
   com[port].com_error = 0;
   if((port == RCVR_PORT) && (rcvr_type == NO_RCVR) && (enable_terminal == 0)) { // alternate ready/not ready each check
      return;
   }
if((port == RCVR_PORT) && (rcvr_type == TIDE_RCVR) && (enable_terminal == 0)) {
   return;
}

   crlf_seen = 0;
   if((com[port].com_port == 0) && (com[port].usb_port == 0) && com[port].IP_addr[0]) {
      //
      // TCP: In Windows, COM0 with process_com=TRUE means we're using TCP/IP
      //
      if(com[port].process_com) {
         init_tcpip(port);
         com[port].com_running = TCPIP_RUNNING;
         com[port].com_data_lost = 0;
         com[port].last_com_time = this_msec = GetMsecs();
         if(port == RCVR_PORT) first_request = 1;
         Sleep(COM_INIT_SLEEP);
      }
      return;
   }

   if((com[port].usb_port == USE_IDEV_NAME) && com[port].com_dev[0]) {  // user set com device name with -id=xxx command line option
   }
   else if(com[port].usb_port == USE_DEV_NAME) {  // use symlink to /dev/heather
      strcpy(com[port].com_dev, HEATHER_DEV);
   }
   else if(com[port].usb_port) {    // USB serial port
      #ifdef __MACH__
         if(com[port].usb_port <= 1) sprintf(com[port].com_dev, "/dev/tty.usbserial");
         else                        sprintf(com[port].com_dev, "/dev/tty.usbserial%d", com[port].usb_port-1);
      #elif defined(__linux__)
         sprintf(com[port].com_dev, "/dev/ttyUSB%d", com[port].usb_port-1);
      #elif defined(__FreeBSD__)
         sprintf(com[port].com_dev, "/dev/cuaU%d", com[port].usb_port-1);
      #else
         strcpy(com[port].com_dev, HEATHER_DEV);
      #endif
   }
   else if(com[port].com_port) {    // hardware serial port
      #ifdef __MACH__
         sprintf(com[port].com_dev, "/dev/ttys%d", com[port].com_port-1);
      #elif defined(__linux__)
         sprintf(com[port].com_dev, "/dev/ttyS%d", com[port].com_port-1);
      #elif defined(__FreeBSD__)
        sprintf(com[port].com_dev, "/dev/cuau%d", com[port].com_port-1);
      #else
        strcpy(com[port].com_dev, HEATHER_DEV);
      #endif
   }
   else {  // should never happen
      com[port].com_running = (-12); //NOT_RUNNING;
      com[port].process_com = 0;
      com[port].com_error |= INIT_ERR;
      if(port == RCVR_PORT) first_request = 1;
      return;
   }
printf("opening com[%d] device:%s\n", port, com[port].com_dev); // zork - show_debug_info
fflush(stdout);

//
// we init the com device here
//

/*
   #ifndef O_RSYNC
      sync = O_SYNC;
   #else
      sync = O_RSYNC;
   #endif
   sync = O_DSYNC | sync;
*/
sync = 0;

   com[port].com_fd = open(com[port].com_dev, O_RDWR | O_NOCTTY | O_NONBLOCK | sync);
   if(com[port].com_fd < 0) {
      printf("Com[%d] open failed:%d\n", port, com[port].com_fd);
      printf("\n");
      printf("Do you have permission to access the serial device?\n");
      printf("try:  sudo usermod -a -G dialout user_name\n");
      printf("      and then re-boot the system\n");
      com[port].process_com = 0;
      com[port].com_running = (-13); //NOT_RUNNING;
      com[port].com_error |= INIT_ERR;
      if(port == RCVR_PORT) first_request = 1;
      return;
   }

   tcgetattr(com[port].com_fd, &oldtio); /* save current port settings */

   /* set new port settings for non-canonical input processing */
   bzero(&newtio, sizeof(newtio));
   if     (com[port].data_bits == 7) newtio.c_cflag = CS7 | CLOCAL | CREAD;
   else if(com[port].data_bits == 6) newtio.c_cflag = CS6 | CLOCAL | CREAD;
   else if(com[port].data_bits == 5) newtio.c_cflag = CS5 | CLOCAL | CREAD;
   else                              newtio.c_cflag = CS8 | CLOCAL | CREAD;

   if(com[port].stop_bits == 2) newtio.c_cflag |= (CSTOPB);

   if(com[port].parity == ODD_PAR) {   // odd parity
      newtio.c_cflag |= (PARENB | PARODD);
   }
   else if(com[port].parity == EVEN_PAR) {  // even parity
      newtio.c_cflag |= (PARENB);
   }
   else {  // no parity
   }

   newtio.c_iflag = IGNPAR;   // ignore parity errors on reads
   newtio.c_oflag = 0;
   newtio.c_lflag = 0;
   newtio.c_cc[VMIN] = 0;
   newtio.c_cc[VTIME] = 0;

   if     (com[port].baud_rate == 150)    i = B150;
   else if(com[port].baud_rate == 300)    i = B300;
   else if(com[port].baud_rate == 600)    i = B600;
   else if(com[port].baud_rate == 1200)   i = B1200;
   else if(com[port].baud_rate == 2400)   i = B2400;
   else if(com[port].baud_rate == 4800)   i = B4800;
   else if(com[port].baud_rate == 9600)   i = B9600;
   else if(com[port].baud_rate == 19200)  i = B19200;
   else if(com[port].baud_rate == 38400)  i = B38400;
   else if(com[port].baud_rate == 57600)  i = B57600;
   else if(com[port].baud_rate == 115200) i = B115200;
   else if(com[port].baud_rate == 230400) i = B230400;
   else i = B9600;
   cfsetispeed(&newtio, i);
   cfsetospeed(&newtio, i);

   tcflush(com[port].com_fd, TCIFLUSH);
   tcsetattr(com[port].com_fd, TCSANOW, &newtio);
   fcntl(com[port].com_fd, F_SETFL, FNDELAY); // make sure port is in non-blocking mode

   SetDtrLine(port, 1);
   if((rcvr_type == ACRON_RCVR) && (port == RCVR_PORT)) {  // power up the RS-232 interface
      SetDtrLine(port, 1);  // drive +12V
      SetRtsLine(port, 0);  // drive -12V
      Sleep(500);
   }
   com[port].com_running = SERIAL_RUNNING;
   com[port].port_used = 2;  // serial port open
   if(port == RCVR_PORT) first_request = 1;
   Sleep(COM_INIT_SLEEP);

   if(modem_alarms) reset_alarm();
}

int check_incoming_data(unsigned port)
{
int flag;

   // return true if we have a serial port byte available to read
   if(port >= NUM_COM_PORTS) return 0;

   if((port == RCVR_PORT) && (rcvr_type == NO_RCVR) && (enable_terminal == 0)) { // alternate ready/not ready each check
      return(++com_tick & 0x01);
   }
if((port == RCVR_PORT) && (rcvr_type == TIDE_RCVR) && (enable_terminal == 0)) {
   return (++com_tick & 0x01);
}

   if((port == RCVR_PORT) && sim_file) {  // using a simulation file
      return sim_kbd_check();
   }

   if(com[port].com_fd < 0) return 0;

   if(com[port].com_error & RCV_ERR) {
//printf("com port %d avail err:%02X\n", port, com[port].com_error);
      return -100; // !!!! return 0;
   }
   if(com[port].next_rcv_byte < com[port].rcv_byte_count) return 2000|com[port].next_rcv_byte;  // we have already read and buffered the char

   com[port].rcv_byte_count = 0;
   com[port].next_rcv_byte = 0;

   if(1 || (com[port].com_port != 0) || (com[port].usb_port != 0)) {  // zork COM port in use: read bytes from serial port
      flag = read(com[port].com_fd, &com[port].rcv_buffer[0], RCV_BUF_SIZE);
      if(flag < 0) {  // !!!! error or no serial data available - we should check errno()
         com[port].rcv_byte_count = 0;
         return 0;
      }
      else {
         com[port].rcv_byte_count = flag;
         if(com[port].rcv_byte_count == 0) return FALSE;  // no serial port data is available
      }
   }

   return (0x8000 | com[port].rcv_byte_count);
}

u08 get_serial_char(unsigned port)
{
int flag;
u08 c;

   // return the next byte from a com device
   if(port >= NUM_COM_PORTS) return FALSE;

   if((port == RCVR_PORT) && (rcvr_type == NO_RCVR) && (enable_terminal == 0)) { // alternate ready/not ready each check
      return 0;
   }
if((port == RCVR_PORT) && (rcvr_type == TIDE_RCVR) && (enable_terminal == 0)) {
   return 0;
}


   if((port == RCVR_PORT) && sim_file) {  // get char from a simulation file
      return get_sim_char(port);
   }

   if(com[port].process_com == 0) return 0;  //lfs
   if(com[port].com_fd < 0) return 0;
   if(com[port].com_error & RCV_ERR) return 0;

   if(com[port].next_rcv_byte < com[port].rcv_byte_count) {   // return byte previously fetched by check_incoming_data()
      c = com[port].rcv_buffer[com[port].next_rcv_byte++];
      if((c == 0x0D) || (c == 0x0A)) crlf_seen = 1;

      if((zoom_screen == 'O')  && (port == monitor_port)) {   // send char to "terminal" format monitor screen
         echo_term(monitor_port, c, monitor_hex, 0);
      }
      return c;
   }

   com[port].rcv_byte_count = 0;
   com[port].next_rcv_byte = 0;

   while(com[port].rcv_byte_count == 0) {  // wait until we have a character
      if(1 || (com[port].com_port != 0) || (com[port].usb_port != 0)) {  // zork COM port in use: read a byte from serial port
         flag = read(com[port].com_fd, &com[port].rcv_buffer[0], RCV_BUF_SIZE);
         if(flag < 0) {  // !!!! com error or no serial data available - we should check errno()
            flag = com[port].rcv_byte_count = 0;
         }
         else com[port].rcv_byte_count = flag;
      }

      update_pwm();   // if doing pwm temperature control
   }

   if(flag && com[port].rcv_byte_count) {  // succesful read
      c = com[port].rcv_buffer[com[port].next_rcv_byte++];
      if((c == 0x0D) || (c == 0x0A)) crlf_seen = 1;

      if((zoom_screen == 'O') && (port == monitor_port)) {   // send char to "terminal" format monitor screen
         echo_term(monitor_port, c, monitor_hex, 0);
      }
      return c;
   }

   if(com_error_exit) {
      error_exit(222, "Serial receive error");
   }
   else {
      com[port].com_error |= RCV_ERR;
      ++com_errors;
   }
   return 0;
}

void sendout(unsigned port, unsigned char val, int eom_flag)
{
unsigned long written;
int flag;
static long x = 0;

   // Ssend data to the receiver.
   // We queue up data until end of message to reduce per-packet overhead.
   // If eom_flag == FLUSH_COM then send buffered chars without adding a new one
   // If eom_flag == 0, add char to buffer (sends buffer if full)
   // If eom_flag != 0, add char to buffer and send buffer

   if((port == RCVR_PORT) && (rcvr_type == NO_RCVR) && (enable_terminal == 0)) { // alternate ready/not ready each check
      return;
   }
if((port == RCVR_PORT) && (rcvr_type == TIDE_RCVR) && (enable_terminal == 0)) {
      return;
}

   if(com[port].com_fd < 0) return;

   if(com[port].process_com == 0) return;
   if(com[port].com_error & XMIT_ERR) return;
   if(just_read && (port == RCVR_PORT)) return;
   if(no_send && (port == RCVR_PORT)) return;
   if(sim_file) return;  // don't send anything out if reading a simulation file

   if(FAKE_ACRON_RCVR && (rcvr_type == ACRON_RCVR) && (port == RCVR_PORT)) {  // simulate echo by sticking char in rcv buffer
      com[port].rcv_buffer[com[port].next_rcv_byte] = val;
      ++com[port].rcv_byte_count;
      return;
   }

   if((zoom_screen == 'O') && (port == monitor_port)) {  // echo data to "terminal" monitor screen
      echo_term(port, (unsigned) val, monitor_hex, 1);
   }

   // we buffer up output chars until buffer full or end of message flag seen
   if(eom_flag == FLUSH_COM) {  // just send buffered chars
      if(x == 0) return;   // nothing to flush
   }
   else com[port].xmit_buffer[x++] = val;  // add char to send buffer

   if((x < (long) XMIT_BUF_SIZE) && (eom_flag == ADD_CHAR)) return;

   update_pwm();   // if doing pwm temperature control

   if(1 || (com[port].com_port != 0) || (com[port].usb_port != 0)) {   // zork COM port in use: send bytes via serial port
      flag = write(com[port].com_fd, com[port].xmit_buffer, x);
if((eom_flag == EOM) || (eom_flag == FLUSH_COM)) {
   fsync(com[port].com_fd);
}
      if(flag == x) written = 1;
      else          written = 0;
   }

   x = 0;

   if((flag == 0) || (written != 1)) {
      if(com_error_exit) {
         error_exit(3, "Serial transmit error");
      }
      else {
         com[port].com_error |= XMIT_ERR;
         ++com_errors;
      }
   }
}


double GetNsecs()
{
struct timespec t;
double nsecs;

   // get nanosecond resolution timer value (in seconds)
   //
   // This funtion would ideally use  CLOCK_MONOTONIC instead of CLOCK_REALTIME
   // but the clock_get_time() funtion uses this to get a high-res value for
   // the wall clock time.  !!!! Perhaps we should use two functions to do this?

#ifdef __MACH__ // OSX does not have clock_gettime, use clock_get_time
   clock_serv_t cclock;
   mach_timespec_t mts;
//gggg host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &cclock);  // for CLOCK_MONOTONIC
   host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
   clock_get_time(cclock, &mts);
   mach_port_deallocate(mach_task_self(), cclock);
   t.tv_sec = mts.tv_sec;
   t.tv_nsec = mts.tv_nsec;
#else
//gggg   clock_gettime(CLOCK_MONOTONIC, &t);
   clock_gettime(CLOCK_REALTIME, &t);
#endif

   wall_time = (double) t.tv_sec;
   wall_time += (((double) t.tv_nsec) / 1.0E9);

   if(initialized_t0 == 0) {  // used to reduce the range of wall_time to
      time_zero = wall_time;  // ... time since program start
      initialized_t0 = 1;
   }

//   nsecs = ((double) t.tv_sec) - time_zero;
//   nsecs *= 1000.0;
//   nsecs += (((double) t.tv_nsec));
   nsecs = (wall_time - time_zero);

   return nsecs;
}

double GetMsecs()
{
// (used for PWMing the temperature control fan and measuring message jitter
   return GetNsecs() * 1.0E3;
}

void Sleep(int time)
{
   time *= 1000;   // convert msecs to usecs
   while(time > 1000000) {
      usleep(1000000);
      time -= 1000000;
   }
   usleep(time);
}


void init_hardware(void)
{
   init_com(RCVR_PORT, 2);
   init_screen(9103);

   if(NO_SCPI_BREAK && (rcvr_type == SCPI_RCVR)) ;
   else if(nortel == 0) {    // To wake up a Nortel NTGS55A, etc receiver
      SendBreak(RCVR_PORT);
   }

   need_screen_init = 0;
}
#endif  // WINDOWS

#ifdef USE_SDL
#define get_kbd() sdl_getch()
#endif // SDL

#ifdef USE_X11  // X11 / Linux / macOS OS video / keyboard routines

u08 last_x11_color = 0x80;
XImage *capture_image;          // screen capture image created here
XSizeHints *size_hints;
XWMHints *wm_hints;
XClassHint *class_hints;
XWindowAttributes wattr;
Atom wmDeleteMessage;


#define get_kbd() x11_getch()
#define KBD_Q_SIZE 16
int kbd_queue[KBD_Q_SIZE+1];
int kbd_in, kbd_out;


void refresh_page(void)
{
   // if double buffering the screen, copy background pixmap to display window
   // also flush the X11 drawing queue

   if(display == 0) return;

if(0 && sim_file && (sim_eof == 0)) {  // this can double the sim file rate
   if((com_tick % 200) != 10) return;
}

   if(pix_map && x11_io_done) {
      SWAP(SCREEN_WIDTH,SCREEN_HEIGHT);
      XCopyArea(display,pix_map,screen,gc, 0,0, SCREEN_WIDTH,SCREEN_HEIGHT, 0,0);
      SWAP(SCREEN_WIDTH,SCREEN_HEIGHT);
   }

   XFlush(display);

   x11_io_done = 0;
}

void flush_x11(void)
{
   // flag that some X11 calls were made - does not actually do an XFlush()
   x11_io_done = 1;
}


void kill_screen(void)
{
   // shutdown the X11 server and screen

   if(display == 0) return;
   XFlush(display);

   if(pix_map) XFreePixmap(display, pix_map);
   pix_map = 0;

   if(icon_map) XFreePixmap(display, icon_map);
   icon_map = 0;

   XDestroyWindow(display, screen);
   screen = 0;
   XFlush(display);

   XFreeGC(display, gc);
   gc = 0;
   XFlush(display);

   if(size_hints) XFree(size_hints);
   size_hints = 0;
   if(wm_hints) XFree(wm_hints);
   wm_hints = 0;
   if(class_hints) XFree(class_hints);
   class_hints = 0;

   XFlush(display);
   XCloseDisplay(display);
   display = 0;

   x11_io_done = 0;
   last_x11_color = 0x80;
   Sleep(X11_sleep);
}


int read_pixmap_file()
{
FILE *file;
int state;
char buf[MAX_PATH+1];
int i;
int j;
int cols,rows;
int num_colors;
int ndx;
int cpc;
u32 val;
int x,y;
int r,g,b;
char name[4+1];

struct COLOR {
   char name[4+1];
   int r,g,b;
} *colors;

   // read a .XPM pixmap icon file and create an X11 pixmap
   if(display == 0) return 1;

   strcpy(buf, heather_path);
   if(luxor) strcat(buf, "luxor.xpm");
   else      strcat(buf, "heather.xpm");
   file = topen(buf, "r");
   if(file == 0) {
printf("Could not open icon pixmap file:%s\n", buf);
      return 1;
   }
   state = 0;
   ndx = 0;
   x = y = 0;

   while(fgets(buf, sizeof(buf), file) != NULL) {
      for(i=0; i<(int)strlen(buf); i++) {  // skip leading whitespace
         if((buf[i] == ' ') || (buf[i] == '\t')) continue;
         break;
      }
      if(buf[i] != '"') continue;  // not a line what we are looking for
      if(buf[i] == 0) continue;    // blank line
      if(buf[i] == 0x0D) continue; // blank line
      if(buf[i] == 0x0A) continue; // blank line

//printf("line %d:%s\n", i, &buf[i]);

      if(state == 0) {  // get pixmap size info
         sscanf(&buf[i+1], "%d %d %d %d", &cols,&rows, &num_colors, &cpc);
printf("icon pixmap  cols:%d  rows:%d  colors:%d  cpc:%d\n", cols,rows, num_colors, cpc);
         if((num_colors <= 0) || (cols <= 0) || (rows <= 0) || (cpc <= 0) || (cpc >= 4)) {
printf("invalid color count:%d\n", num_colors);
            fclose(file);
            return 1;
         }

         colors = (struct COLOR *) (void *) calloc(num_colors, sizeof(struct COLOR));
         if(colors == 0) {
printf("could not allocate color table\n");
            fclose(file);
            return 8;
         }
         ++state;
      }
      else if(state == 1) { // get color definitions
          if(ndx >= (num_colors-1)) {  // end of color definitions
             state = 2;
             icon_map = XCreatePixmap(display,screen, cols,rows, wattr.depth); // winz
             if(icon_map == 0) {
                if(colors) free(colors);
                fclose(file);
                return 4;
             }
          }

          colors[ndx].name[0] = 0;
          for(j=0; j<cpc; j++) {  // get color name string
             ++i;
             colors[ndx].name[j] = buf[i];
             colors[ndx].name[j+1] = 0;
             if(j > 4) {
printf("color code too long\n");
                if(colors) free(colors);
                if(icon_map) XFreePixmap(display, icon_map);
                icon_map = 0;
                fclose(file);
                return 2;
             }
          }
          if(colors[ndx].name[0] == 0) {  // no color name
printf("invalid color name\n");
             if(colors) free(colors);
             if(icon_map) XFreePixmap(display, icon_map);
             icon_map = 0;
             fclose(file);
             return 3;
          }

          val = 0;
          while(i < (int)strlen(buf)-1) {  // find pixel value (hex only)
             ++i;
             if(buf[i] != '#') continue;

             val = atohex(&buf[i+1]);
             colors[ndx].r = (val >> 16) & 0xFF;
             colors[ndx].g = (val >> 8) & 0xFF;
             colors[ndx].b = val & 0xFF;
          }
//printf("%d:  (%s) %02X %02X %02X\n", ndx, colors[ndx].name, colors[ndx].r,colors[ndx].g,colors[ndx].b);
          ++ndx;
      }
      else if(state == 2) { // get pixmap pixels
//printf("cpc:%d  pixel row %d:%s\n", cpc, y, &buf[i+1]);
         for(x=0; x<cols; x++) {
            name[0] = 0;
            for(j=0; j<cpc; j++) {   // get pixel color code
               name[j] = buf[++i];
               name[j+1] = 0;
            }

            r = g = b = 0;
            for(j=0; j<num_colors; j++) {  // lookup pixel color RGB value
               if(!strcmp(name, colors[j].name)) {
                  r = colors[j].r;
                  g = colors[j].g;
                  b = colors[j].b;
                  goto found_color;
               }
            }
printf("Bad color code (%s) in pixmap! Using black.\n", name);

            found_color:
//printf("%d: %s(%02X,%02X,%02X) \n", x, name, r,g,b);
            XSetForeground(display,gc, RGB_NATIVE(r,g,b));
            XDrawPoint(display,icon_map, gc, x,y);

         }
//printf("\n");
         if(++y > rows) break;
      }
   }

printf("Icon pixmap created\n");
   if(colors) free(colors);
   colors = 0;

   fclose(file);
   return 0;
}


u32 RGB_NATIVE(int r, int g, int b)
{
u32 color;

   // convert 8-bit R,G,B values to X11 pixel color
   // !!!!!! we need to use the proper X11 way to do this... whatever that is

   if     (wattr.depth == 15) color = (((u32) (r>>3))<<10)  | (((u32) (g>>3))<<5) | ((u32) (b>>3));
   else if(wattr.depth == 16) color = (((u32) (r>>3))<<11)  | (((u32) (g>>2))<<5) | ((u32) (b>>3));
   else if(wattr.depth == 18) color = (((u32) (r>>2))<<12)  | (((u32) (g>>2))<<6) | ((u32) (b>>2));
   else if(wattr.depth == 24) color = (((u32) r)<<16)       | (((u32) g)<<8)      | ((u32) b);
   else if(wattr.depth == 30) color = (((u32) r)<<20)       | (((u32) g)<<10)     | ((u32) (b<<2));
   else                       color = (((u32) r)<<16)       | (((u32) g)<<8)      | ((u32) b) | 0xFF000000;
   return color;
}

int screen_active()
{
   // return true if screen has been initialized

   if(display) return 1;
   return 0;
}

void init_screen(int why)
{
Cursor cur;
unsigned char *vfx_font;
int font_height;
int i;
unsigned int j;
int old_sw, old_sh;

struct DECO_HINTS {  // stuff for disabling window decorations
   u32 flags;
   u32 functions;
   u32 decorations;
   u32 input_mode;
   u32 status;
} deco_hints;
Atom deco;

   // Initialize the graphics screen
printf("\nInit screen %d: %dx%d (potentially rotated)\n", why, SCREEN_WIDTH,SCREEN_HEIGHT);

   config_screen(3);  // initialize screen rendering variables for potentially rotated size
// VFX_io_done = 1;
   kill_screen();    // release any screen data structures currently in use

   old_sw = SCREEN_WIDTH;
   old_sh = SCREEN_HEIGHT;
   SWAP(SCREEN_WIDTH,SCREEN_HEIGHT);  // physical screen size
printf("Physical screen size: %dx%d\n", SCREEN_WIDTH,SCREEN_HEIGHT);

   // setup text drawing using WIN_VFX fonts
   if(user_font_size == 0)  {
      if(big_plot && (ebolt == 0)) {
         vfx_font = &h_medium_font[0];
         user_font_size = 14;
      }
      else {
         vfx_font = &h_font_12[0];
         user_font_size = 12;
      }
   }
   else if(user_font_size <= 8)  {
      vfx_font = &h_small_font[0];
      user_font_size = 8;
   }
   else if(user_font_size <= 12) {
      vfx_font = &h_font_12[0];
      user_font_size = 12;
   }
   else if(user_font_size <= 14) {
      vfx_font = &h_medium_font[0];
      user_font_size = 14;
   }
   else if(user_font_size <= 16) {
      vfx_font = &h_large_font[0];
      user_font_size = 16;
   }
   else {
      vfx_font = &h_font_12[0];
      user_font_size = 12;
   }

   font_height = user_font_size;
   TEXT_HEIGHT = font_height;
   TEXT_WIDTH  = 8;  // !!!!!!!
   if(font_height <= 12) {
      small_font = 2;
   }
   else small_font = 0;

   dot_font = (unsigned char *) (void *) vfx_font;



   /*  Window variables  */

   int          x, y;
   unsigned int width, height;
   char *window_name;
   char *icon_name;

   if(luxor) {
      window_name = "Luxor X11";
      icon_name   = "Luxor";
   }
   else {
      window_name = &szAppName[0];  // "Lady Heather X11";
      icon_name   = "Heather";
   }


   /*  Display variables  */

   char *       display_name = NULL;


   /*  Miscellaneous X11 variables  */

   XTextProperty windowName, iconName;
   XGCValues     values;

   if(luxor) appname = "LUXOR";  // !!!argv[0];
   else      appname = "HEATHER";  // !!!argv[0];


   /*  Allocate memory for X11 structures  */

   if ( !( size_hints  = XAllocSizeHints() ) ||
        !( wm_hints    = XAllocWMHints()   ) ||
        !( class_hints = XAllocClassHint() )    ) {
       sprintf(out, "%s: couldn't allocate memory.\n", appname);
       error_exit(10005, out);
   }


   /*  Connect to X11 server  */

   display = XOpenDisplay(display_name);
   if(display == NULL) {
      sprintf(out, "%s: couldn't connect to X server %s\n", appname, display_name);
      error_exit(10006, out);
   }


   /*  Get screen size from display structure macro  */

   screen_num     = DefaultScreen(display);
   display_width  = DisplayWidth(display, screen_num);
   display_height = DisplayHeight(display, screen_num);
   have_root_info = 1;

printf("Config disp: %dx%d  screen:%dx%d  kill_decorations:%d  go_full:%d\n", display_width,display_height, SCREEN_WIDTH,SCREEN_HEIGHT, kill_deco, go_fullscreen);
   if(go_fullscreen && (display_width >= MIN_WIDTH) && (display_height >= MIN_HEIGHT)) {
      SCREEN_WIDTH  = display_width;
      SCREEN_HEIGHT = display_height;
      go_fullscreen = 0;
   }
if(kill_deco) go_fullscreen = 1;
printf("Display size: %dx%d  screen:%dx%d  kill_decorations:%d  go_full:%d\n", display_width,display_height, SCREEN_WIDTH,SCREEN_HEIGHT, kill_deco, go_fullscreen);


   if(!RESTORE_MAXED_WINDOW) ; // don't attempt to restore maximized window size
   else if(((unsigned) SCREEN_WIDTH >= display_width) || ((unsigned) SCREEN_HEIGHT >= display_height)) {  // request for large screen
      if(x11_maxed) {
         if(have_restore_size) {
            SCREEN_WIDTH = restore_width;   //1024;
            SCREEN_HEIGHT = restore_height; // 768
         }
         x11_maxed = 0;
printf("have restore size %d: %d %d   new:%d %d\n", have_restore_size, restore_width,restore_height, new_width,new_height);
      }
      else {
         x11_maxed = 1;
printf("maximize window\n");
      }
   }




   /*  Set initial window size and position, and create it  */

   x = x11_left;
   y = x11_top;
   width  = SCREEN_WIDTH;
   height = SCREEN_HEIGHT;
printf("creating %dx%d simple window at %d,%d\n", width,height, x,y);
   screen = XCreateSimpleWindow(display, RootWindow(display,screen_num),
                             x,y, width,height, 0,
                             WhitePixel(display, screen_num),
                             BlackPixel(display, screen_num));

   if(screen == 0) {
       error_exit(10010, "Could not create X11 simple window");
   }

printf("Get window attributes\n");
   XGetWindowAttributes(display,screen, &wattr);  // winz
   x11_left = wattr.x;
   x11_top = wattr.y;
   if(x11_left < 0) x11_left = 0;
   if(x11_top < 0) x11_top = 0;
printf("Window color depth: %d  left/top:%dx%d\n", wattr.depth, x11_left,x11_top);


   // setup screen palette
   memset(palette,0xff,sizeof(palette));
   setup_palette();

//for(xx=0; xx<16; xx++) printf("pal %2d: %08lX\n", xx,palette[xx]);
//printf("white:  %08lX\n", WhitePixel(display,screen_num));

printf("Palette setup\n");
   /*  Set hints for window manager before mapping window  */

   if(XStringListToTextProperty(&window_name, 1, &windowName) == 0 ) {
      sprintf(out, "%s: structure allocation for windowName failed.\n", appname);
      error_exit(10007, out);
   }
printf("Text properties set.\n");
   if(XStringListToTextProperty(&icon_name, 1, &iconName) == 0 ) {
      sprintf(out, "%s: structure allocation for iconName failed.\n", appname);
      error_exit(10008, out);
   }
printf("Icon name set\n");
   /*  Create graphics context  */
printf("creating gc\n");
fflush(stdout);
   gc = XCreateGC(display,screen, 0, &values); // winz
   if(gc == 0) {
      sprintf(out, "Could not create graphics context");
      error_exit(10018, out);
   }

   // Create the icon pixmap
   i = read_pixmap_file();
   if(i) {  // XPM pixmap not loaded, try a monochrome XBM bitmap
      if(luxor) i = XReadBitmapFile(display,screen, "luxor.xbm", &j,&j, &icon_map, &i,&i);
      else      i = XReadBitmapFile(display,screen, "heather.xbm", &j,&j, &icon_map, &i,&i);
   }
   else {  // XPM pixmap created
      i = BitmapSuccess;
   }

   if(i != BitmapSuccess) {
printf("No icon image found.\n");
      icon_map = 0;
   }

   size_hints->flags       = PPosition | PSize | PMinSize;
   size_hints->min_width   = MIN_WIDTH;
   size_hints->min_height  = MIN_HEIGHT;

   if(icon_map) wm_hints->flags = StateHint | InputHint | IconPixmapHint;
   else         wm_hints->flags = StateHint | InputHint;
   wm_hints->initial_state = NormalState;
   wm_hints->input         = True;
   wm_hints->icon_pixmap   = icon_map;

   class_hints->res_name   = appname;
   if(luxor) class_hints->res_class  = "luxor";
   else      class_hints->res_class  = "heather";
printf("setting properties\n");
   XSetWMProperties(display,screen, &windowName, &iconName, 0,0, // winz- argv, argc,
                    size_hints, wm_hints, class_hints);

   wmDeleteMessage = XInternAtom(display, "WM_DELETE_WINDOW",  0);
   XSetWMProtocols(display,screen, &wmDeleteMessage, 1);


   if(kill_deco) {  // disable window decorations to get a full screen display
      deco_hints.flags = 2;
      deco_hints.functions = 0;
      deco_hints.decorations = 0;
      deco_hints.input_mode = 0;
      deco_hints.status = 0;

      deco = XInternAtom(display, "_MOTIF_WM_HINTS", 1);

      if(deco) {
printf("Killing window decorations!\n");
         touch_screen = 1;  // make sure virtual keyboard is available.
                            // If the window size does not match the monitor size
                            // you can lose access to the real keyboard.
         XChangeProperty(display,screen, deco,deco, 32, PropModeReplace, (unsigned char *) &deco_hints, 5);
      }
   }


   /*  Choose which events we want to handle  */

   XSelectInput(display,screen, ExposureMask | KeyPressMask | PointerMotionMask |
                ButtonPressMask | ButtonReleaseMask | StructureNotifyMask | EnterWindowMask | LeaveWindowMask);   // winz


   cur = XCreateFontCursor(display, 132);
   if(cur) XDefineCursor(display,screen, cur);    // winz


   XSetForeground(display,gc, WhitePixel(display, screen_num));


   /*  Display Window  */

   XMapWindow(display,screen);  // winz
printf("window mapped    Pixmap wh:%dx%d\n", width,height);
   pix_map = XCreatePixmap(display,screen, width,height, wattr.depth); // winz
   if(pix_map == 0) {
      error_exit(30000, "Could not create pix_map");
   }
printf("pixmap created\n");
   Sleep(X11_sleep);
   XFlush(display);

printf("Display size2: %dx%d\n", SCREEN_WIDTH,SCREEN_HEIGHT);
   SWAP(SCREEN_WIDTH,SCREEN_HEIGHT);
//SCREEN_WIDTH = old_sw;   //piss
//SCREEN_HEIGHT = old_sh;
   config_screen(why);  // re-initialize screen rendering variables
                        // to reflect any changes due to font size
   XFlush(display);

   #ifdef BUFFER_LLA
//lla clear_lla_points(0);
   #endif

printf("screen configured. why=%d  %dx%d (potentially rotated)\n", why, SCREEN_WIDTH,SCREEN_HEIGHT);
   erase_rectangle(0,0, SCREEN_WIDTH,SCREEN_HEIGHT);
printf("screen init done\n\n");
fflush(stdout);
}

void set_x11_color(u08 color)
{
   // set the graphics context color

   if(color != last_x11_color) {  // color has changed, update context
      last_x11_color = color;
      if(color == 0xFF) XSetForeground(display, gc, RGB_NATIVE(0,0,35)); // special plot area background color highlight
      else              XSetForeground(display, gc, palette[color]);
   }
}


//****************************************************************************
//
// X11 / Linux message receiver procedure for application
//
// We implement a small keystroke queue so that the DOS kbhit()/getch()
// keyboard i/o model works with the Linux message queue i/o model.
// Keystrokes are put into the queue by the Linux message reciever proc.
// They are drained from the queue by the program keyboard handler.
//
//****************************************************************************

void add_kbd(int key)
{
   // add a keystroke to the keyboard queue

   last_kbd_jd = jd_utc;
   if(++kbd_in >= KBD_Q_SIZE) kbd_in = 0;
   if(kbd_in == kbd_out) {  // kbd queue is full
      if(--kbd_in < 0) kbd_in = KBD_Q_SIZE-1;
   }
   else {  // put keystoke in the queue
      kbd_queue[kbd_in] = key;
   }
}

void flush_kbd()
{
   kbd_in = kbd_out = 0;
   edit_buffer[0] = 0;
   getting_string = 0;
}

int x11_kbhit(void)
{
   // return true if a keystoke is available

   reset_kbd_timer();
   return (kbd_in != kbd_out);  // return true if anything is in the keyboard queue
}

int x11_getch(void)
{
int key;

   // get a keystroke from the keyboard queue

   if(kbd_in == kbd_out) return 0;            // no keys in the queue

   if(++kbd_out >= KBD_Q_SIZE) kbd_out = 0;   // get to next queue entry
   key = kbd_queue[kbd_out];                  // get keystroke from queue
   return key;                                // return it
}

int enter_mouse;
int leave_mouse;


//#define PRESLEY   // define to use x11_event for mouse clicks (does not work!)

int get_x11_event()
{
XEvent report;
KeySym key;
char buf[20];
int button;
int i;

   // This is the X11 event handler.

   if(display == 0) return 0;
   button = 0;

   if(!XEventsQueued(display, QueuedAlready)) {  // no events available to process
      return 0;
   }
   XNextEvent(display, &report);

   switch(report.type) {  // act on the event
      case Expose:
          if(report.xexpose.count != 0 ) break;
if(1 && show_debug_info) printf("Expose event seen\n");  // zork - show_debug_info
          need_redraw = 1000;
          return 1;
          break;


      case ConfigureNotify:
          /*  Store new window desired window width/height  and top/left  */
          new_width = report.xconfigure.width;
          new_height = report.xconfigure.height;
          x11_left = report.xconfigure.x;
          x11_top = report.xconfigure.y;
          if(x11_left < 0) x11_left = 0;
          if(x11_top < 0) x11_top = 0;
          need_resize = 1;
if(1 && show_debug_info) printf("configure:  wh%d %d  tl:%d %d  mouse:%d\n", new_width,new_height, x11_top,x11_left, this_button);  // zork - show_debug_info
          return 2;
          break;


      case KeyPress:
         buf[0] = 0;
         i = XLookupString(&report.xkey, &buf[0],20, &key, 0);
         if     (key ==  XK_Home)      add_kbd(HOME_CHAR);
         else if(key ==  XK_Up)        add_kbd(UP_CHAR);
         else if(key ==  XK_Page_Up)   add_kbd(PAGE_UP);
         else if(key ==  XK_Left)      add_kbd(LEFT_CHAR);
         else if(key ==  XK_Right)     add_kbd(RIGHT_CHAR);
         else if(key ==  XK_End)       add_kbd(END_CHAR);
         else if(key ==  XK_Down)      add_kbd(DOWN_CHAR);
         else if(key ==  XK_Page_Down) add_kbd(PAGE_DOWN);
         else if(key ==  XK_Insert)    add_kbd(INS_CHAR);
         else if(key ==  XK_Delete)    add_kbd(DEL_CHAR);
         else if(key ==  XK_BackSpace) add_kbd(0x08);
         else if(key ==  XK_Tab)       add_kbd(0x09);
         else if(key ==  XK_Linefeed)  add_kbd(0x0A);
         else if(key ==  XK_Clear)     add_kbd(0x0B);
         else if(key ==  XK_Return)    add_kbd(0x0D);
         else if(key ==  XK_Escape)    add_kbd(ESC_CHAR);
         else if(i && (key >= ' ') && (key < 0x7F)) add_kbd(buf[0]);
         else if(key ==  XK_Break)     break_flag = 1;
         else if(key ==  XK_F1)        add_kbd(F1_CHAR);
         else if(key ==  XK_F2)        add_kbd(F2_CHAR);
         else if(key ==  XK_F3)        add_kbd(F3_CHAR);
         else if(key ==  XK_F4)        add_kbd(F4_CHAR);
         else if(key ==  XK_F5)        add_kbd(F5_CHAR);
         else if(key ==  XK_F6)        add_kbd(F6_CHAR);
         else if(key ==  XK_F7)        add_kbd(F7_CHAR);
         else if(key ==  XK_F8)        add_kbd(F8_CHAR);
         else if(key ==  XK_F9)        add_kbd(F9_CHAR);
         else if(key ==  XK_F10)       add_kbd(F10_CHAR);
         else if(key ==  XK_F11) {  // piss
            f11_flag = 1;
            need_screen_init = 99999;
            not_safe = 1;
            config_options();

         }
         else if(key ==  XK_F12)         add_kbd(0);
         else if(key ==  XK_Help)        add_kbd(' ');

         else if(key ==  XK_KP_Space)    add_kbd(' ');      // keypad keys
         else if(key ==  XK_KP_Tab)      add_kbd(0x08);
         else if(key ==  XK_KP_Enter)    add_kbd(0x0D);
         else if(key ==  XK_KP_F1)       add_kbd(F1_CHAR);
         else if(key ==  XK_KP_F2)       add_kbd(F2_CHAR);
         else if(key ==  XK_KP_F3)       add_kbd(F3_CHAR);
         else if(key ==  XK_KP_F4)       add_kbd(F4_CHAR);
         else if(key ==  XK_KP_Equal)    add_kbd('=');
         else if(key ==  XK_KP_Multiply) add_kbd('*');
         else if(key ==  XK_KP_Add)      add_kbd('+');
         else if(key ==  XK_KP_Subtract) add_kbd('-');
         else if(key ==  XK_KP_Decimal)  add_kbd('.');
         else if(key ==  XK_KP_Divide)   add_kbd('/');
         else if(key ==  XK_KP_0)        add_kbd('0');
         else if(key ==  XK_KP_1)        add_kbd('1');
         else if(key ==  XK_KP_2)        add_kbd('2');
         else if(key ==  XK_KP_3)        add_kbd('3');
         else if(key ==  XK_KP_4)        add_kbd('4');
         else if(key ==  XK_KP_5)        add_kbd('5');
         else if(key ==  XK_KP_6)        add_kbd('6');
         else if(key ==  XK_KP_7)        add_kbd('7');
         else if(key ==  XK_KP_8)        add_kbd('8');
         else if(key ==  XK_KP_9)        add_kbd('9');

         return 3;
         break;


      case EnterNotify:
         my_mouse = 1;
         enter_mouse = report.xcrossing.subwindow;
         break;


      case LeaveNotify:
         my_mouse = 0;
         leave_mouse = report.xcrossing.subwindow;
         break;


//    case MotionNotify:
//       mouse_x = report.xmotion.x_root;
//       mouse_y = report.xmotion.y_root;
//       mouse_y -= 48;
//       x11_mouse = 1;
//       return 4;
//       break;

#ifdef PRESLEY       // we now use XQueryPointer in get_mouse_info to do this
      case ButtonPress:            /*  Fall through  */
          button = (int) report.xbutton.button;
          if(button == 1) {
             last_button = this_button;
             this_button = 1;
          }
          else if(button == 3) {
             last_button = this_button;
             this_button = 2;
          }
          else {
             last_button = this_button;
             this_button = 0;
          }
if(0 && show_debug_info) printf("Button press: this:%d  last:%d!\n", this_button, last_button);
          return 5;
          break;

       case ButtonRelease:
if(0 && show_debug_info) printf("Button release\n");
//        last_button = this_button;
//        this_button = 0;
          return 6;
          break;
#endif


    case ClientMessage:
if(0 && show_debug_info) printf("Client message:%ld\n", report.xclient.data.l[0]); //zork - show_debug_info
        if((unsigned long) report.xclient.data.l[0] == (unsigned long) wmDeleteMessage) {
           shut_down(0);
        }
        return 6;
        break;
   }

   return 0;
}


#endif  // USE_X11




#ifdef WIN_VFX   // WINDOWS video drawing routines

void serve_vfx()
{
   // process the Windows event handler via WIN_VFX

   SAL_serve_message_queue();
   Sleep(0);
}

void kill_screen(void)
{
   // gracefully close the graphics window here

   VFX_io_done = 1;

   if(stage_window) {
      VFX_window_destroy(stage_window);
      stage_window = 0;
   }
   if(stage) {
      VFX_pane_destroy(stage);
      stage = 0;
   }
   if(screen_window) {
      VFX_window_destroy(screen_window);
      screen_window = 0;
   }
   if(screen) {
      VFX_pane_destroy(screen);
      screen = 0;
   }
}

void do_windowed()
{
   initial_window_mode = VFX_WINDOW_MODE;
   change_screen_res();
   downsized = 1;
}

void do_fullscreen()
{
return;  // this is broken in the VFX library
   initial_window_mode = VFX_TRY_FULLSCREEN;
   change_screen_res();
   downsized = 0;
}

void init_VFX_text()
{
   if(user_font_size == 0)  {
      if(big_plot && (ebolt == 0)) vfx_font = (VFX_FONT *) (void *) &h_medium_font[0];
      else vfx_font = (VFX_FONT *) (void *) &h_font_12[0];
   }
   else if(user_font_size <= 8)  vfx_font = (VFX_FONT *) (void *) &h_small_font[0];
   else if(user_font_size <= 12) vfx_font = (VFX_FONT *) (void *) &h_font_12[0];
   else if(user_font_size <= 14) vfx_font = (VFX_FONT *) (void *) &h_medium_font[0];
   else if(user_font_size <= 16) vfx_font = (VFX_FONT *) (void *) &h_large_font[0];
   else                          vfx_font = (VFX_FONT *) (void *) &h_font_12[0];

   font_height = VFX_font_height(vfx_font);
   TEXT_HEIGHT = font_height;
   TEXT_WIDTH  = VFX_character_width(vfx_font, 'm');
   if(font_height <= 12) {
      small_font = 2;
   }
   else small_font = 0;

   dot_font = (unsigned char *) (void *) vfx_font;
}

#define SCREEN_BORDER_WIDTH  10
#define SCREEN_BORDER_HEIGHT 32

void get_screen_size(int init)
{
RECT r;

   if(SystemParametersInfo(SPI_GETWORKAREA, 0, &r, 0)) {
      display_width = (unsigned) (r.right - r.left);
      display_height = (unsigned) (r.bottom - r.top);

      if(go_fullscreen && init) {
         go_fullscreen = 0;
         SCREEN_WIDTH = (int) (display_width - SCREEN_BORDER_WIDTH);
         SCREEN_HEIGHT = (int) (display_height - SCREEN_BORDER_HEIGHT);

         if(SCREEN_WIDTH <= MIN_WIDTH) SCREEN_WIDTH = MIN_WIDTH;
         if(SCREEN_HEIGHT <= MIN_HEIGHT) SCREEN_HEIGHT = MIN_HEIGHT;

if(screen_type == 'p');       // ras pi full screen touchscreen sizes (kill_deco ?)
else if(screen_type == 'r');
else if(screen_type == 'j');
else
         screen_type = 'c';
         adjust_screen_options();
      }
   }
}

int screen_active()
{
   // return true if screen has been initialized

   if(screen) return 1;
   return 0;
}

void init_screen(int why)
{
   get_screen_size(1);

   config_screen(6);  // initialize screen rendering variables (for potentially rotated screen)
   VFX_io_done = 1;
   kill_screen();     // release any screen data structures currently in use

   //set new video mode
   SWAP(SCREEN_WIDTH,SCREEN_HEIGHT);  // init screen with physical size
   if(!VFX_set_display_mode(SCREEN_WIDTH,
                            SCREEN_HEIGHT,
                            16,
                            initial_window_mode,
                            TRUE))
   {  // big problem
      error_exit(4, "VFX cannot set video display mode");
   }


   // setup screen palette
   memset(palette,0xFF,sizeof(palette));

   setup_palette();

   transparent_font_CLUT[0] = (U16) RGB_TRANSPARENT;
   transparent_font_CLUT[1] = (U16) RGB_NATIVE(0,0,0);


   //
   // Setup text fonts
   //
   init_VFX_text();


   // allocate screen windows and buffers
   stage_window = VFX_window_construct(SCREEN_WIDTH, SCREEN_HEIGHT);

   stage = VFX_pane_construct(stage_window,
                              0,
                              0,
                              SCREEN_WIDTH-1,
                              SCREEN_HEIGHT-1);

   VFX_assign_window_buffer(stage_window, NULL, -1);

   screen_window = VFX_window_construct(SCREEN_WIDTH, SCREEN_HEIGHT);

   screen = VFX_pane_construct(screen_window,
                               0,
                               0,
                               SCREEN_WIDTH-1,
                               SCREEN_HEIGHT-1);

   SWAP(SCREEN_WIDTH,SCREEN_HEIGHT);  // back to potentially rotated sizw

   config_screen(why);  // re-initialize screen rendering variables
                        // to reflect any changes due to font size

   #ifdef BUFFER_LLA
//lla clear_lla_points(0);
   #endif

   erase_rectangle(0,0, SCREEN_WIDTH,SCREEN_HEIGHT);
   refresh_page();
}

void refresh_page(void)
{
   // copies the virtual screen buffer to the physical screen
   // (or othwewise flips pages, etc for double buffered graphics)

if(0 && sim_file && (sim_eof == 0)) {  // this can double the sim file rate
   if((com_tick % 200) != 10) return;
}
   if(stage == 0) return;

   if(VFX_io_done == 0) { // nothing touched the screen since the last call
      return;             // so don't waste resources redrawing the page
   }
   VFX_io_done = 0;

   //
   // Lock the buffer and validate the VFX_WINDOW
   //
   VFX_lock_window_surface(screen_window,VFX_BACK_SURFACE);

   //
   // Copy entire staging pane to screen_window
   //
   VFX_pane_copy(stage,0,0,screen,0,0,NO_COLOR);

   //
   // Release surface and perform page flip
   //
   VFX_unlock_window_surface(screen_window, TRUE);
}

#endif // WIN_VFX


//
//
//  OS independent character drawing routines
//
//

void set_title_bar(int why)
{
unsigned i;

   // set program title bar to the string szAppName
   // use executable file name and unit_file_name if no szAppName string.

   if((szAppName[0] == 0) && unit_file_name) {
      strcpy(szAppName, unit_file_name);  // the device name
      szAppName[32] = 0;
      strupr(szAppName);

      if(1) {   // include the executable file name
         strcat(szAppName, " - ");
         strcpy(out, exe_name);
         out[255-32-3] = 0;
         strcat(szAppName, out);
      }
   }

   for(i=0; i<strlen(szAppName); i++) {
      if(szAppName[i] == '_') szAppName[i] = ' ';
   }

   #ifdef WINDOWS
      SetWindowText(hWnd, &szAppName[0]);
   #endif

   #ifdef USE_X11
      init_screen(3498);
   #endif
}


int character_width(u08 c)
{
int h,w;
int p;

   // get the width of a (mono-spaced) VFX font character

   if(dot_font == 0) dot_font = &h_font_12[0];
   if(c > 127) c = ' ';  // char out of range

   h = dot_font[8];      // char height

   p = (4*4) + (c*4);    // index of offset to char definition
   p = (dot_font[p+1] * 256) + dot_font[p+0];  // pointer to char definition
   w = dot_font[p];      // char width
   return w;
}


int dot_char(COORD x, COORD y, u08 c, u08 attr)
{
int h,w;
int p;
int xx,yy;
int xl;

   // draw a VFX font character using the line drawing routine
   // (useful if porting Heather to a differnt operating system)
   if(x > SCREEN_WIDTH) x = SCREEN_WIDTH;
   if(y > SCREEN_HEIGHT) y = SCREEN_HEIGHT;

   if(dot_font == 0) dot_font = &h_font_12[0];
   if(use_vc_fonts) {
      p = (4*4) + (0x20*4);    // index of offset to char definition of a valid char
      if(c == DEG_CODE) c = 0xF8;
      else if(c == PLUS_MINUS) c = 0xF1;
   }
   else if(c & 0x80) {
      c = ' '; // char out of range
      p = (4*4) + (c*4);    // index of offset to char definition
   }
   else p = (4*4) + (c*4);    // index of offset to char definition

   h = dot_font[8];      // char height

   p = (dot_font[p+1] * 256) + dot_font[p+0];  // pointer to char definition
   w = dot_font[p];      // char width
   p += 4;               // pointer to char pattern

   if(use_vc_fonts) {  // draw all text using vector characters
      int old_scale;
      old_scale = VCHAR_SCALE;
      VCHAR_SCALE = 1;
      TEXT_WIDTH = (w*vc_font_scale) / 100;
      TEXT_HEIGHT = (h*vc_font_scale) / 100;
      vchar(x,y, 1, attr, c);  // draw a vector character
      if(0 && VCharThickness) w = (TEXT_WIDTH+VCharThickness-1);
      else w = (TEXT_WIDTH+VCharThickness*0);
      VCHAR_SCALE = old_scale;
      return w;
   }

   erase_rectangle(x,y, TEXT_WIDTH,TEXT_HEIGHT);

   for(yy=0; yy<h; yy+=1) {    // for each row in a character
      xl = (-1);               // left-most pixel of a run of dots
      for(xx=0; xx<w; xx+=1) { // for each pixel in a character row
         if(dot_font[p]) {       // character has a visible dot
            if(xl < 0) xl = xx;  // it's the first dot of a run

            if(xx >= (w-1)) {    // this is the last dot in the character row
               if(xl >= 0) {     // we have a run of dots pending, draw it
                  line(x+xl,y+yy, x+xx,y+yy, attr);
               }
            }
         }
         else { // it's a background color dot
            if(xl >= 0) {  // this pixel ends a pending run of visible dots
               line(x+xl,y+yy, x+(xx-1),y+yy, attr);
               xl = (-1);
            }
         }

         ++p;  // next pixel in the char
      }
   }

   return w;
}


int plusminus_char(COORD row, COORD col, u08 color)
{
   // erase the old character cell
   erase_rectangle(col,row, TEXT_WIDTH,TEXT_HEIGHT);

   // draw the +/- symbol (which is not in the normal font)
   dot(col+3, row+0, color);  // plus
   dot(col+3, row+1, color);
   dot(col+3, row+2, color);
   dot(col+3, row+3, color);
   dot(col+3, row+4, color);
   dot(col+3, row+5, color);
   dot(col+4, row+0, color);
   dot(col+4, row+1, color);
   dot(col+4, row+2, color);
   dot(col+4, row+3, color);
   dot(col+4, row+4, color);
   dot(col+4, row+5, color);

   dot(col+1, row+2, color);
   dot(col+2, row+2, color);
   dot(col+5, row+2, color);
   dot(col+6, row+2, color);
   dot(col+1, row+3, color);
   dot(col+2, row+3, color);
   dot(col+5, row+3, color);
   dot(col+6, row+3, color);

   dot(col+1, row+7, color);    // minus
   dot(col+2, row+7, color);
   dot(col+3, row+7, color);
   dot(col+4, row+7, color);
   dot(col+5, row+7, color);
   dot(col+6, row+7, color);
   dot(col+1, row+8, color);
   dot(col+2, row+8, color);
   dot(col+3, row+8, color);
   dot(col+4, row+8, color);
   dot(col+5, row+8, color);
   dot(col+6, row+8, color);

   return 8;  // width
}


#define DEG_WIDTH (5+1)
int deg_char(COORD row, COORD col, u08 color)
{
   // erase the old character cell (and probably a little extra)
   erase_rectangle(col,row, TEXT_WIDTH,TEXT_HEIGHT);

   // draw the degrees symbol (which is not in the normal font)
   dot(col+2, row+0, color);
   dot(col+3, row+0, color);
   dot(col+1, row+1, color);
   dot(col+4, row+1, color);
   dot(col+1, row+2, color);
   dot(col+4, row+2, color);
   dot(col+2, row+3, color);
   dot(col+3, row+3, color);

   return DEG_WIDTH;  // width
}

void vidstr(COORD row, COORD col, u08 attr, char *s)
{
char *p;
S32 px, py;
char work_str[MAX_TEXT_COLS+1];
char *chp;
char ch;
S32 word_x[UNIT_LEN+1];
S32 n_words;
S32 ux;
char last_uch;
char last_ch;
char uch;
S32 w;
S32 x;

   // this routine recognizes some global variables:
   //   blank_underscore: if set, converts underscore char to blanks
   //   graphics_coords:  if set, row and col are in pixels coordinates
   //                     if clear, they are in character coordinates
   //   no_x_margin:      if set, do not apply the "x" character margin offset
   //   no_y_margin:      if set, do not apply the "y" character margin offset
   //   print_using:      if not null, points to column alignment string
   //                     (note: Heather does not use this functionality, it can be deleted)

   if(graphics_coords) {  // row/col in pixels
      px = col;
      py = row;
   }
   else { // row/col in text positions
      px = col * TEXT_WIDTH;
      py = row * TEXT_HEIGHT;

      if((zoom_screen && (zoom_screen != 'P') && (zoom_screen != 'K')) || (py < (((PLOT_ROW/TEXT_HEIGHT)*TEXT_HEIGHT) - (TEXT_HEIGHT*2)))) {
         if(no_x_margin == 0) px += TEXT_X_MARGIN;   // looks better with margins
         if(no_y_margin == 0) py += TEXT_Y_MARGIN;
      }
   }


   if(print_using == NULL) {  // we are using a monospace font and not aligning columns
      strcpy(work_str, s);

      chp = work_str;
      while(*chp) {
         if(blank_underscore) {
            if(*chp == '_') *chp = ' ';
         }

         if(use_vc_fonts) {
            px += dot_char(px,py, *chp, attr);
         }
         else if(*chp == DEGREES) {
            px += deg_char(py,px, attr);
         }
         else if(*chp == PLUS_MINUS) {
            px += plusminus_char(py,px, attr);
         }
         else {  // dot-by-dot character rendering
            px += dot_char(px,py, *chp, attr);
         }
         ++chp;
      }

      return;
   }

   //
   // At each space->nonspace transition in *s, position cursor at equivalent
   // space->nonspace transition in *print_using
   //
   // This allows table-cell alignment with VFX's proportional font
   //
   // New kludge: the first column is right justified one space before the
   //             second column.
   //

   n_words = 0;
   ux = px;
   last_uch = '\0';

   while(*print_using) {
      uch = *print_using++;

      if(((uch != ' ') && (last_uch == ' ')) || (last_uch == '\0')) {
         word_x[n_words++] = ux;
         if(n_words >= UNIT_LEN) break;
      }

      last_uch = uch;
      if(blank_underscore) {
         if(uch == '_') uch = ' ';  // _ prints as a blank,  but does not cause a space transition
      }

      if(uch == DEGREES) ux += DEG_WIDTH;  //!!! kludge:  hardcoded char width
      else if(uch == PLUS_MINUS) ux += 8;  //!!! kludge:  hardcoded char width
      else ux += 8; // !!!!!! VFX_character_width(vfx_font, uch);
   }

   w = 0;
   x = px;
   last_ch = '\0';

   while(*s) {
      ch = *s++;

      if((ch != ' ') && (last_ch == ' ') && (w < n_words)) {
         x = word_x[w++];
         if(w == 1) {  // right justify the first column
            x = word_x[w];   // x = where the second column starts
            x -= 8; // !!!! VFX_character_width(vfx_font, ' ');  // back up a space
            p = s-1;
            while((*p != ' ') && (*p != '\0')) {  // back up the width of the text
               x -= 8; // !!!!VFX_character_width(vfx_font, *p);
               ++p;
            }
         }
      }

      last_ch = ch;
      if(blank_underscore) {
         if(ch == '_') ch = ' ';
      }

      if(use_vc_fonts) {
         x += dot_char(x,py, ch, attr);
      }
      else if(ch == DEGREES) {
         x += deg_char(py,x, attr);
      }
      else if(ch == PLUS_MINUS) {
         x += plusminus_char(py,x, attr);
      }
      else if(1) { // !!!!! testing dot-by-dot character rendering
         x += dot_char(x,py, ch, attr);
      }
   }
}


//
//
//  Vector char stuff for showing big digital clock, etc
//  (Note: the PROGMEM keyword is a left over from a AVR processor implementation)
//
//
u08 vg_00[] PROGMEM = { 0xFF };
u08 vg_01[] PROGMEM = { 0x01,0x90,0xD0,0xE1,0xE6, 0x01,0x86,0x97,0xD7,0xE6, 0x22,
                         0x24,0xB5,0xC4, 0x4A };
u08 vg_02[] PROGMEM = { 0x01,0x90,0xC3, 0x01,0xE1,0xE6, 0x01,0x86,0x97,0xD7,0xE6,
                         0x02,0xA0,0xB1,0xC0,0xE2, 0x03,0xE3, 0x04,0xB7,0xE4, 0x05,0xA7,
                         0x05,0x95,0x97, 0x06,0xE6, 0x10,0xD0,0xE1, 0x10,0x91, 0x20,0xA1,0xB0,0xC1,
                         0x23,0xD0,0xD1, 0x26,0xA7, 0x30,0xB3, 0x36,0xB7, 0x40,0xC1,
                         0x46,0xC7,0xE5, 0x55,0xE5, 0x55,0xDF };
u08 vg_03[] PROGMEM = { 0x01,0x90,0xC3, 0x01,0xB4, 0x01,0xC1,0xC3, 0x01,0x83,0xA5,0xC3,
                         0x02,0xA4,0xC2, 0x02,0xC2, 0x03,0xB0,0xC1, 0x03,0xC3, 0x10,0x94,0xC1,
                         0x11,0xB3, 0x13,0xB1, 0x14,0xB4, 0x21,0xA6, 0x30,0xBC };
u08 vg_04[] PROGMEM = { 0x03,0xB6,0xE3, 0x03,0xB0,0xE3, 0x03,0xE3, 0x12,0xC5, 0x12,0xD2,0xD4,
                         0x12,0x94,0xC1,0xC5, 0x13,0xB5,0xD3, 0x13,0xB1,0xD3, 0x14,0xD4,
                         0x21,0xD4, 0x21,0xC1, 0x21,0xA5,0xD2, 0x22,0xC4, 0x24,0xC2,
                         0x25,0xC5, 0x30,0xBE };
u08 vg_05[] PROGMEM = { 0x03,0xC7, 0x03,0xD3,0xD4, 0x03,0x84,0xB7, 0x04,0xB1, 0x04,0xD4,
                         0x11,0xA0,0xB1, 0x11,0xC4, 0x11,0xC1, 0x13,0xB5, 0x13,0x95,0xB3,
                         0x14,0xC1, 0x15,0xC5, 0x17,0xD3, 0x17,0xC7, 0x20,0xB0,0xC1,
                         0x20,0xA7,0xD4, 0x21,0xB0,0xB7, 0x21,0xD4, 0x22,0xB2, 0x23,0xC5,
                         0x25,0xC3,0xC5, 0x26,0xBE };
u08 vg_06[] PROGMEM = { 0x03,0xC7, 0x03,0xB0,0xE3,0xE5, 0x03,0xE3, 0x03,0x85,0xC1,0xC5,
                         0x04,0xB1,0xE4, 0x04,0xE4, 0x12,0xC5, 0x12,0xD2,0xD4, 0x12,0x94,
                         0x13,0xB5,0xD3, 0x21,0xE5, 0x21,0xC1, 0x21,0xA5,0xD2, 0x22,0xC4,
                         0x24,0xC2, 0x25,0xC5, 0x27,0xE3, 0x27,0xC7, 0x30,0xBF };
u08 vg_07[] PROGMEM = { 0x23,0xB4,0xC3,0xC4, 0x23,0xB2,0xC3, 0x23,0xC3, 0x23,0xA4,0xB3,0xC4,
                         0x24,0xB5,0xC4, 0x24,0xC4, 0x32,0xBD };
u08 vg_08[] PROGMEM = { 0x00,0xE0,0xE7, 0x00,0x87,0xE7, 0x01,0x90,0x92, 0x01,0xE1,
                         0x02,0xA0,0xA1, 0x02,0x92, 0x03,0xB0,0xE3, 0x04,0xB7,0xE4,
                         0x05,0xA7, 0x05,0x95,0x97, 0x06,0x97, 0x06,0xE6, 0x26,0xA7,
                         0x30,0xB1, 0x36,0xB7, 0x40,0xE2, 0x40,0xC1, 0x46,0xC7,0xE5,
                         0x50,0xE1, 0x50,0xD2,0xE2, 0x55,0xE5, 0x55,0xD7,0xEE };
u08 vg_09[] PROGMEM = { 0x03,0xA1,0xC1,0xE3,0xE4, 0x03,0x84,0xA6,0xC6,0xEC };
u08 vg_0A[] PROGMEM = { 0x00,0xE0,0xE7, 0x00,0x87,0xE7, 0x23,0xB4,0xC3,0xC4, 0x23,0xB2,0xC3,
                         0x23,0xC3, 0x23,0xA4,0xB3,0xC4, 0x24,0xB5,0xC4, 0x24,0xC4,
                         0x32,0xBD };
u08 vg_0B[] PROGMEM = { 0x04,0x93,0xB3,0xC4,0xC6, 0x04,0x86,0x97,0xB7,0xC6, 0x33,0xE0,0xE2,
                         0x40,0xE8 };
u08 vg_0C[] PROGMEM = { 0x11,0xA0,0xC0,0xD1,0xD3, 0x11,0x93,0xA4,0xC4,0xD3, 0x16,0xD6,
                         0x34,0xBF };
u08 vg_0D[] PROGMEM = { 0x06,0x95,0xA5, 0x06,0x87,0x97,0xA6, 0x20,0xE0,0xE2, 0x20,0xA6,
                         0x22,0xEA };
u08 vg_0E[] PROGMEM = { 0x05,0x94,0xA4, 0x05,0x86,0x96,0xA5, 0x20,0xE0,0xE6, 0x20,0xA5,
                         0x22,0xE2, 0x46,0xD5,0xE5, 0x46,0xC7,0xD7,0xEE };
u08 vg_0F[] PROGMEM = { 0x00,0xA2, 0x03,0x93,0xB5,0xD3,0xE3, 0x06,0xA4, 0x13,0xB1,0xD3,
                         0x30,0xB1, 0x35,0xB6, 0x42,0xE0, 0x44,0xEE };
u08 vg_10[] PROGMEM = { 0x00,0xB3, 0x00,0x86,0xB3, 0x01,0xA3, 0x01,0x91,0x95, 0x02,0xA4,
                         0x02,0xA2,0xA4, 0x03,0xB3, 0x04,0xA2, 0x04,0xA4, 0x05,0xA3,
                         0x05,0x9D };
u08 vg_11[] PROGMEM = { 0x23,0xD6, 0x23,0xD0,0xD6, 0x23,0xD3, 0x32,0xD4, 0x32,0xD2,
                         0x32,0xB4,0xD2, 0x33,0xD5, 0x33,0xD1, 0x34,0xD4, 0x41,0xD1,
                         0x41,0xC5,0xDD };
u08 vg_12[] PROGMEM = { 0x12,0xB0,0xD2, 0x15,0xB7,0xD5, 0x30,0xBF };
u08 vg_13[] PROGMEM = { 0x10,0x94, 0x16, 0x50,0xD4, 0x5E };
u08 vg_14[] PROGMEM = { 0x01,0x90,0xE0,0xE6, 0x01,0x82,0x93,0xB3, 0x30,0xBE };
u08 vg_15[] PROGMEM = { 0x06,0x97,0xB7,0xC6, 0x11,0xA2,0xB2,0xC3,0xC4, 0x11,0xA0,0xC0,0xD1,
                         0x13,0xA2, 0x13,0x94,0xA5,0xB5,0xC6, 0x35,0xCC };
u08 vg_16[] PROGMEM = { 0x14,0xB6,0xD4,0xD6, 0x14,0xD4, 0x14,0x96,0xB4,0xD6, 0x15,0xA6,0xC4,0xD5,
                         0x15,0xA4,0xC6,0xD5, 0x15,0xD5, 0x16,0xD6, 0x24,0xA6, 0x34,0xB6,
                         0x44,0xCE };
u08 vg_17[] PROGMEM = { 0x12,0xB0,0xD2, 0x14,0xB6,0xD4, 0x17,0xD7, 0x30,0xBE };
u08 vg_18[] PROGMEM = { 0x12,0xB0,0xD2, 0x30,0xBE };
u08 vg_19[] PROGMEM = { 0x14,0xB6,0xD4, 0x30,0xBE };
u08 vg_1A[] PROGMEM = { 0x03,0xD3, 0x31,0xD3, 0x35,0xDB };
u08 vg_1B[] PROGMEM = { 0x03,0xA5, 0x03,0xA1, 0x03,0xDB };
u08 vg_1C[] PROGMEM = { 0x02,0x85,0xDD };
u08 vg_1D[] PROGMEM = { 0x03,0xA5, 0x03,0xA1, 0x03,0xE3, 0x41,0xE3, 0x45,0xEB };
u08 vg_1E[] PROGMEM = { 0x04,0xB1,0xE4, 0x04,0xE4, 0x13,0xD3,0xD4, 0x13,0x94,0xB2,0xD4,
                         0x22,0xC4, 0x22,0xC2,0xC4, 0x22,0xA4,0xC2, 0x31,0xBC };
u08 vg_1F[] PROGMEM = { 0x02,0xB5,0xE2, 0x02,0xE2, 0x12,0xB4,0xD2,0xD3, 0x12,0x93,0xD3,
                         0x22,0xC4, 0x22,0xA4,0xC2,0xC4, 0x24,0xC4, 0x32,0xBD };
u08 vg_20[] PROGMEM = { 0xFF };
u08 vg_21[] PROGMEM = { 0x11,0xA0,0xB1,0xB3, 0x11,0xB3, 0x11,0xB1, 0x11,0x93,0xA4,0xB3,
                         0x12,0xB2, 0x13,0xB1, 0x13,0xB3, 0x20,0xA4, 0x2E };
u08 vg_22[] PROGMEM = { 0x10,0x92, 0x40,0xCA };
u08 vg_23[] PROGMEM = { 0x02,0xD2, 0x04,0xD4, 0x10,0x96, 0x40,0xCE };
u08 vg_24[] PROGMEM = { 0x02,0x93,0xB3,0xC4, 0x02,0x91,0xC1, 0x05,0xB5,0xC4, 0x20,0xA1,
                         0x25,0xAE };
u08 vg_25[] PROGMEM = { 0x01,0x91,0x92, 0x01,0x82,0x92, 0x06,0xD1, 0x45,0xD5,0xD6,
                         0x45,0xC6,0xDE };
u08 vg_26[] PROGMEM = { 0x04,0xA2,0xB2,0xD4,0xE3, 0x04,0x85,0x96,0xC6,0xD5,0xE6,
                         0x11,0xA2, 0x11,0xA0,0xB0,0xC1, 0x32,0xC1, 0x54,0xDD };
u08 vg_27[] PROGMEM = { 0x02,0x91, 0x10,0x99 };
u08 vg_28[] PROGMEM = { 0x12,0xB0, 0x12,0x94,0xBE };
u08 vg_29[] PROGMEM = { 0x10,0xB2,0xB4, 0x16,0xBC };
u08 vg_2A[] PROGMEM = { 0x03,0xE3, 0x11,0xD5, 0x15,0xD1, 0x30,0xBE };
u08 vg_2B[] PROGMEM = { 0x03,0xC3, 0x21,0xAD };
u08 vg_2C[] PROGMEM = { 0x17,0xA6, 0x25,0xAE };
u08 vg_2D[] PROGMEM = { 0x03,0xCB };
u08 vg_2E[] PROGMEM = { 0x25,0xAE };
u08 vg_2F[] PROGMEM = { 0x05,0xD8 };
u08 vg_30[] PROGMEM = { 0x01,0x90,0xC0,0xD1,0xD5, 0x01,0x85,0x96,0xC6,0xD5, 0x05,0xD9 };
u08 vg_31[] PROGMEM = { 0x11,0xA0,0xA6, 0x16,0xBE };
u08 vg_32[] PROGMEM = { 0x01,0x90,0xB0,0xC1,0xC2, 0x05,0xA3,0xB3,0xC2, 0x05,0x86,0xCE };
u08 vg_33[] PROGMEM = { 0x01,0x90,0xB0,0xC1,0xC2, 0x05,0x96,0xB6,0xC5, 0x23,0xB3,0xC4,0xC5,
                         0x33,0xCA };
u08 vg_34[] PROGMEM = { 0x03,0xB0,0xC0,0xC6, 0x03,0x84,0xDC };
u08 vg_35[] PROGMEM = { 0x00,0xC0, 0x00,0x82,0xB2,0xC3,0xC5, 0x05,0x96,0xB6,0xCD };
u08 vg_36[] PROGMEM = { 0x02,0xA0,0xB0, 0x02,0x85,0x96,0xB6,0xC5, 0x03,0xB3,0xC4,0xCD };
u08 vg_37[] PROGMEM = { 0x00,0xC0,0xC2, 0x00,0x81, 0x24,0xC2, 0x24,0xAE };
u08 vg_38[] PROGMEM = { 0x01,0x90,0xB0,0xC1,0xC2, 0x01,0x82,0x93,0xB3,0xC4,0xC5,
                         0x04,0x93, 0x04,0x85,0x96,0xB6,0xC5, 0x33,0xCA };
u08 vg_39[] PROGMEM = { 0x01,0x90,0xB0,0xC1,0xC4, 0x01,0x82,0x93,0xC3, 0x16,0xA6,0xCC };
u08 vg_3A[] PROGMEM = { 0x21,0xA2, 0x25,0xAE };
u08 vg_3B[] PROGMEM = { 0x17,0xA6, 0x21,0xA2, 0x25,0xAE };
u08 vg_3C[] PROGMEM = { 0x03,0xB6, 0x03,0xB8 };
u08 vg_3D[] PROGMEM = { 0x02,0xC2, 0x05,0xCD };
u08 vg_3E[] PROGMEM = { 0x10,0xC3, 0x16,0xCB };
u08 vg_3F[] PROGMEM = { 0x01,0x90,0xB0,0xC1,0xC2, 0x24,0xC2, 0x24,0xA5, 0x2F };
u08 vg_40[] PROGMEM = { 0x01,0x90,0xC0,0xD1,0xD4, 0x01,0x85,0x96,0xB6, 0x32,0xB4,0xDC };
u08 vg_41[] PROGMEM = { 0x02,0xA0,0xC2,0xC6, 0x02,0x86, 0x04,0xCC };
u08 vg_42[] PROGMEM = { 0x00,0xB0,0xC1,0xC2, 0x00,0x86,0xB6,0xC5, 0x03,0xB3,0xC4,0xC5,
                         0x33,0xCA };
u08 vg_43[] PROGMEM = { 0x02,0xA0,0xC0,0xD1, 0x02,0x84,0xA6,0xC6,0xDD };
u08 vg_44[] PROGMEM = { 0x00,0xA0,0xC2,0xC4, 0x00,0x86,0xA6,0xCC };
u08 vg_45[] PROGMEM = { 0x10,0xD0, 0x10,0x96,0xD6, 0x13,0xBB };
u08 vg_46[] PROGMEM = { 0x10,0xD0, 0x10,0x96, 0x13,0xBB };
u08 vg_47[] PROGMEM = { 0x02,0xA0,0xC0,0xD1, 0x02,0x84,0xA6,0xD6, 0x44,0xD4,0xDE };
u08 vg_48[] PROGMEM = { 0x00,0x86, 0x03,0xC3, 0x40,0xCE };
u08 vg_49[] PROGMEM = { 0x10,0xB0, 0x16,0xB6, 0x20,0xAE };
u08 vg_4A[] PROGMEM = { 0x04,0x85,0x96,0xB6,0xC5, 0x30,0xD0, 0x40,0xCD };
u08 vg_4B[] PROGMEM = { 0x00,0x86, 0x03,0xA3,0xC5,0xC6, 0x23,0xC1, 0x40,0xC9 };
u08 vg_4C[] PROGMEM = { 0x10,0x96,0xDE };
u08 vg_4D[] PROGMEM = { 0x00,0xB3,0xE0,0xE6, 0x00,0x86, 0x33,0xBC };
u08 vg_4E[] PROGMEM = { 0x00,0xD5, 0x00,0x86, 0x50,0xDE };
u08 vg_4F[] PROGMEM = { 0x02,0xA0,0xB0,0xD2,0xD4, 0x02,0x84,0xA6,0xB6,0xDC };
u08 vg_50[] PROGMEM = { 0x10,0xC0,0xD1,0xD2, 0x10,0x96, 0x13,0xC3,0xDA };
u08 vg_51[] PROGMEM = { 0x01,0x90,0xB0,0xC1,0xC5, 0x01,0x85,0x96,0xB6,0xC5, 0x24,0xDF };
u08 vg_52[] PROGMEM = { 0x10,0xC0,0xD1,0xD2, 0x10,0x96, 0x13,0xC3,0xD2, 0x33,0xD5,0xDE };
u08 vg_53[] PROGMEM = { 0x01,0x90,0xB0,0xC1, 0x01,0x82,0x93,0xB3,0xC4,0xC5, 0x05,0x96,0xB6,0xCD };
u08 vg_54[] PROGMEM = { 0x00,0xC0, 0x20,0xAE };
u08 vg_55[] PROGMEM = { 0x00,0x85,0x96,0xB6,0xC5, 0x40,0xCD };
u08 vg_56[] PROGMEM = { 0x00,0x84,0xA6,0xC4, 0x40,0xCC };
u08 vg_57[] PROGMEM = { 0x00,0x86,0xB3,0xE6, 0x32,0xB3, 0x60,0xEE };
u08 vg_58[] PROGMEM = { 0x00,0x81,0xD6, 0x06,0xD1, 0x50,0xD9 };
u08 vg_59[] PROGMEM = { 0x00,0x82,0xA4,0xC2, 0x16,0xB6, 0x24,0xA6, 0x40,0xCA };
u08 vg_5A[] PROGMEM = { 0x10,0xD0,0xD1, 0x15,0xD1, 0x15,0x96,0xDE };
u08 vg_5B[] PROGMEM = { 0x10,0xB0, 0x10,0x96,0xBE };
u08 vg_5C[] PROGMEM = { 0x00,0xEE };
u08 vg_5D[] PROGMEM = { 0x10,0xB0,0xB6, 0x16,0xBE };
u08 vg_5E[] PROGMEM = { 0x03,0xB0,0xEB };
u08 vg_5F[] PROGMEM = { 0x07,0xEF };
u08 vg_60[] PROGMEM = { 0x20,0xA1,0xBA };
u08 vg_61[] PROGMEM = { 0x05,0x96,0xB6,0xC5,0xD6, 0x05,0x94,0xC4, 0x12,0xB2,0xC3,0xCD };
u08 vg_62[] PROGMEM = { 0x06,0x95,0xA6,0xC6,0xD5, 0x10,0x95, 0x13,0xC3,0xD4,0xDD };
u08 vg_63[] PROGMEM = { 0x03,0x92,0xB2,0xC3, 0x03,0x85,0x96,0xB6,0xCD };
u08 vg_64[] PROGMEM = { 0x04,0x93,0xC3, 0x04,0x85,0x96,0xB6,0xC5,0xD6, 0x40,0xCD };
u08 vg_65[] PROGMEM = { 0x03,0x92,0xB2,0xC3,0xC4, 0x03,0x85,0x96,0xB6, 0x04,0xCC };
u08 vg_66[] PROGMEM = { 0x03,0xA3, 0x06,0xA6, 0x11,0xA0,0xB0,0xC1, 0x11,0x9E };
u08 vg_67[] PROGMEM = { 0x03,0x92,0xB2,0xC3,0xD2, 0x03,0x84,0x95,0xC5, 0x07,0xB7,0xC6,
                         0x43,0xCE };
u08 vg_68[] PROGMEM = { 0x10,0x96, 0x14,0xB2,0xC2,0xD3,0xDE };
u08 vg_69[] PROGMEM = { 0x12,0xA2,0xA6, 0x16,0xB6, 0x28 };
u08 vg_6A[] PROGMEM = { 0x05,0x86,0x97,0xB7,0xC6, 0x40, 0x42,0xCE };
u08 vg_6B[] PROGMEM = { 0x10,0x96, 0x14,0xB4,0xD6, 0x34,0xDA };
u08 vg_6C[] PROGMEM = { 0x10,0xA0,0xA6, 0x16,0xBE };
u08 vg_6D[] PROGMEM = { 0x02,0x86, 0x04,0xA2,0xB3,0xC2,0xD3,0xD6, 0x33,0xBD };
u08 vg_6E[] PROGMEM = { 0x02,0x86, 0x04,0xA2,0xB2,0xC3,0xCE };
u08 vg_6F[] PROGMEM = { 0x03,0x92,0xB2,0xC3,0xC5, 0x03,0x85,0x96,0xB6,0xCD };
u08 vg_70[] PROGMEM = { 0x02,0x93,0xA2,0xC2,0xD3,0xD4, 0x07,0xA7, 0x13,0x97, 0x15,0xC5,0xDC };
u08 vg_71[] PROGMEM = { 0x03,0x92,0xB2,0xC3,0xD2, 0x03,0x84,0x95,0xC5, 0x37,0xD7,
                         0x43,0xCF };
u08 vg_72[] PROGMEM = { 0x02,0x93,0xA2,0xB2,0xC3, 0x06,0xA6, 0x13,0x9E };
u08 vg_73[] PROGMEM = { 0x03,0x94,0xB4,0xC5, 0x03,0x92,0xC2, 0x06,0xB6,0xCD };
u08 vg_74[] PROGMEM = { 0x12,0xC2, 0x20,0xA5,0xB6,0xCD };
u08 vg_75[] PROGMEM = { 0x02,0x85,0x96,0xB6,0xC5,0xD6, 0x42,0xCD };
u08 vg_76[] PROGMEM = { 0x02,0x84,0xA6,0xC4, 0x42,0xCC };
u08 vg_77[] PROGMEM = { 0x02,0x85,0x96,0xB4,0xD6,0xE5, 0x33,0xB4, 0x62,0xED };
u08 vg_78[] PROGMEM = { 0x02,0xA4,0xB4,0xD6, 0x06,0xA4, 0x34,0xDA };
u08 vg_79[] PROGMEM = { 0x02,0x84,0x95,0xC5, 0x07,0xB7,0xC6, 0x42,0xCE };
u08 vg_7A[] PROGMEM = { 0x02,0xC2, 0x06,0xC2, 0x06,0xCE };
u08 vg_7B[] PROGMEM = { 0x03,0x93,0xA4,0xA5,0xB6,0xC6, 0x13,0xA2, 0x21,0xB0,0xC0,
                         0x21,0xAA };
u08 vg_7C[] PROGMEM = { 0x30,0xB2, 0x34,0xBE };
u08 vg_7D[] PROGMEM = { 0x00,0x90,0xA1,0xA2,0xB3,0xC3, 0x06,0x96,0xA5, 0x24,0xB3,
                         0x24,0xAD };
u08 vg_7E[] PROGMEM = { 0x01,0x90,0xA0,0xB1,0xC1,0xD8 };
u08 vg_7F[] PROGMEM = { 0x04,0xB1,0xE4,0xE6, 0x04,0x86,0xEE };
u08 vg_80[] PROGMEM = { 0x01,0x90,0xC0,0xD1, 0x01,0x83,0x94,0xC4,0xD3, 0x27,0xC7,0xD6,
                         0x34,0xDE };
u08 vg_81[] PROGMEM = { 0x01, 0x03,0x85,0x96,0xB6,0xC5,0xD6, 0x41, 0x43,0xCD };
u08 vg_82[] PROGMEM = { 0x03,0x92,0xB2,0xC3,0xC4, 0x03,0x85,0x96,0xB6, 0x04,0xC4,
                         0x30,0xC8 };
u08 vg_83[] PROGMEM = { 0x01,0x90,0xD0,0xE1, 0x15,0xA6,0xC6,0xD5,0xE6, 0x15,0xA4,0xD4,
                         0x22,0xC2,0xD3,0xDD };
u08 vg_84[] PROGMEM = { 0x05,0x96,0xB6,0xC5,0xD6, 0x05,0x94,0xC4, 0x10, 0x12,0xB2,0xC3,0xC5, 0x48 };
u08 vg_85[] PROGMEM = { 0x00,0x90, 0x05,0x96,0xB6,0xC5,0xD6, 0x05,0x94,0xC4, 0x12,0xB2,0xC3,0xCD };
u08 vg_86[] PROGMEM = { 0x05,0x96,0xB6,0xC5,0xD6, 0x05,0x94,0xC4, 0x12,0xB2,0xC3,0xC5,
                         0x20,0xB0,0xB1, 0x20,0xA1,0xB9 };
u08 vg_87[] PROGMEM = { 0x03,0x92,0xC2,0xD3, 0x03,0x84,0x95,0xC5, 0x07,0x97,0xBD };
u08 vg_88[] PROGMEM = { 0x01,0x90,0xD0,0xE1, 0x13,0xA2,0xC2,0xD3,0xD4, 0x13,0x95,0xA6,0xC6,
                         0x14,0xDC };
u08 vg_89[] PROGMEM = { 0x00, 0x03,0x92,0xB2,0xC3,0xC4, 0x03,0x85,0x96,0xB6, 0x04,0xC4, 0x48 };
u08 vg_8A[] PROGMEM = { 0x00,0xA0, 0x03,0x92,0xB2,0xC3,0xC4, 0x03,0x85,0x96,0xB6,
                         0x04,0xCC };
u08 vg_8B[] PROGMEM = { 0x00, 0x12,0xA2,0xA6, 0x16,0xB6, 0x48 };
u08 vg_8C[] PROGMEM = { 0x01,0x90,0xC0,0xD1, 0x22,0xB2,0xB6, 0x26,0xCE };
u08 vg_8D[] PROGMEM = { 0x00,0xA0, 0x12,0xA2,0xA6, 0x16,0xBE };
u08 vg_8E[] PROGMEM = { 0x00, 0x03,0xA1,0xC3,0xC6, 0x03,0x86, 0x04,0xC4, 0x48 };
u08 vg_8F[] PROGMEM = { 0x04,0x93,0xB3,0xC4,0xC6, 0x04,0x86, 0x05,0xC5, 0x20,0xB0,0xB1,
                         0x20,0xA1,0xB9 };
u08 vg_90[] PROGMEM = { 0x12,0xC2, 0x12,0x96,0xC6, 0x14,0xB4, 0x20,0xC8 };
u08 vg_91[] PROGMEM = { 0x15,0xA4,0xE4, 0x15,0x96,0xE6, 0x22,0xE2, 0x42,0xCE };
u08 vg_92[] PROGMEM = { 0x02,0xA0,0xE0, 0x02,0x86, 0x03,0xD3, 0x30,0xB6,0xEE };
u08 vg_93[] PROGMEM = { 0x01,0x90,0xB0,0xC1, 0x04,0x93,0xB3,0xC4,0xC5, 0x04,0x85,0x96,0xB6,0xCD };
u08 vg_94[] PROGMEM = { 0x01, 0x04,0x93,0xB3,0xC4,0xC5, 0x04,0x85,0x96,0xB6,0xC5, 0x49 };
u08 vg_95[] PROGMEM = { 0x01,0xA1, 0x04,0x93,0xB3,0xC4,0xC5, 0x04,0x85,0x96,0xB6,0xCD };
u08 vg_96[] PROGMEM = { 0x01,0x90,0xB0,0xC1, 0x03,0x85,0x96,0xB6,0xC5,0xD6, 0x43,0xCD };
u08 vg_97[] PROGMEM = { 0x01,0xA1, 0x03,0x85,0x96,0xB6,0xC5,0xD6, 0x43,0xCD };
u08 vg_98[] PROGMEM = { 0x01, 0x03,0x84,0x95,0xC5, 0x07,0xB7,0xC6, 0x41, 0x43,0xCE };
u08 vg_99[] PROGMEM = { 0x00, 0x13,0xB1,0xD3,0xD4, 0x13,0x94,0xB6,0xD4, 0x68 };
u08 vg_9A[] PROGMEM = { 0x00, 0x02,0x85,0x96,0xB6,0xC5, 0x40, 0x42,0xCD };
u08 vg_9B[] PROGMEM = { 0x03,0x92,0xD2, 0x03,0x84,0x95,0xD5, 0x30,0xBF };
u08 vg_9C[] PROGMEM = { 0x03,0xA3, 0x06,0xC6,0xD5, 0x11,0xA0,0xB0,0xC1,0xC2, 0x11,0x9E };
u08 vg_9D[] PROGMEM = { 0x00,0x81,0xA3,0xC1, 0x04,0xC4, 0x06,0xC6, 0x12, 0x23,0xA7,
                         0x32, 0x40,0xC9 };
u08 vg_9E[] PROGMEM = { 0x00,0xB0,0xC1,0xC2, 0x00,0x87, 0x03,0xB3,0xC2, 0x54,0xF4,
                         0x63,0xE6,0xF7, 0xFF };
u08 vg_9F[] PROGMEM = { 0x06,0x97,0xA7,0xB6, 0x23,0xC3, 0x31,0xC0,0xD0,0xE1, 0x31,0xBE };
u08 vg_A0[] PROGMEM = { 0x05,0x96,0xB6,0xC5,0xD6, 0x05,0x94,0xC4, 0x12,0xB2,0xC3,0xC5,
                         0x20,0xC8 };
u08 vg_A1[] PROGMEM = { 0x12,0xA2,0xA6, 0x16,0xB6, 0x20,0xC8 };
u08 vg_A2[] PROGMEM = { 0x04,0x93,0xB3,0xC4,0xC5, 0x04,0x85,0x96,0xB6,0xC5, 0x21,0xC9 };
u08 vg_A3[] PROGMEM = { 0x03,0x85,0x96,0xB6,0xC5,0xD6, 0x21,0xC1, 0x43,0xCD };
u08 vg_A4[] PROGMEM = { 0x01,0x90,0xA0,0xB1,0xC1,0xD0, 0x03,0x86, 0x04,0x93,0xB3,0xC4,0xCE };
u08 vg_A5[] PROGMEM = { 0x00,0xC0, 0x02,0xC6, 0x02,0x86, 0x42,0xCE };
u08 vg_A6[] PROGMEM = { 0x11,0xA0,0xC0,0xC3, 0x11,0x92,0xA3,0xD3, 0x15,0xDD };
u08 vg_A7[] PROGMEM = { 0x11,0xA0,0xB0,0xC1,0xC2, 0x11,0x92,0xA3,0xB3,0xC2, 0x15,0xCD };
u08 vg_A8[] PROGMEM = { 0x15,0xA6,0xC6,0xD5, 0x15,0xB3, 0x30, 0x32,0xBB };
u08 vg_A9[] PROGMEM = { 0x03,0xC3, 0x03,0x8D };
u08 vg_AA[] PROGMEM = { 0x03,0xC3,0xCD };
u08 vg_AB[] PROGMEM = { 0x00,0x82, 0x05,0xD0, 0x34,0xC3,0xD3,0xE4, 0x46,0xE4, 0x46,0xC7,0xEF };
u08 vg_AC[] PROGMEM = { 0x00,0x82, 0x05,0xD0, 0x44,0xE2,0xE7, 0x44,0xC5,0xED };
u08 vg_AD[] PROGMEM = { 0x13,0xA2,0xB3,0xB5, 0x13,0xB5, 0x13,0xB3, 0x13,0x95,0xA6,0xB5,
                         0x14,0xB4, 0x15,0xB3, 0x15,0xB5, 0x20, 0x22,0xAE };
u08 vg_AE[] PROGMEM = { 0x03,0xA5, 0x03,0xA1, 0x43,0xE5, 0x43,0xE9 };
u08 vg_AF[] PROGMEM = { 0x01,0xA3, 0x05,0xA3, 0x41,0xE3, 0x45,0xEB };
u08 vg_B0[] PROGMEM = { 0x01, 0x03, 0x05, 0x07, 0x20, 0x22, 0x24, 0x26, 0x41, 0x43, 0x45,
                         0x47, 0x60, 0x62, 0x64, 0x6E };
u08 vg_B1[] PROGMEM = { 0x01,0xE7,0xF6, 0x01,0x90,0xF6, 0x03,0xC7,0xF4, 0x03,0xB0,0xF4,
                         0x05,0xA7,0xF2, 0x05,0xD0,0xF2, 0x07,0xF8 };
u08 vg_B2[] PROGMEM = { 0x01,0x91,0xA2,0xB2,0xC3,0xD3,0xE4,0xF4, 0x03,0x93,0xA4,0xB4,0xC5,0xD5,0xE6,0xF6,
                         0x05,0x95,0xA6,0xB6,0xC7,0xD7,0xE6, 0x07,0x97,0xA6, 0x11,0xA0,0xB0,0xC1,0xD1,0xE2,0xF2,
                         0x13,0xA2, 0x15,0xA4, 0x32,0xC1, 0x34,0xC3, 0x36,0xC5, 0x51,0xE0,0xF0,
                         0x53,0xE2, 0x55,0xEC };
u08 vg_B3[] PROGMEM = { 0x30,0xBF };
u08 vg_B4[] PROGMEM = { 0x04,0xB4, 0x30,0xBF };
u08 vg_B5[] PROGMEM = { 0x02,0xB2, 0x04,0xB4, 0x30,0xBF };
u08 vg_B6[] PROGMEM = { 0x04,0xB4, 0x30,0xB7, 0x60,0xEF };
u08 vg_B7[] PROGMEM = { 0x04,0xE4,0xE7, 0x34,0xBF };
u08 vg_B8[] PROGMEM = { 0x02,0xB2,0xB7, 0x04,0xBC };
u08 vg_B9[] PROGMEM = { 0x02,0xB2, 0x04,0xB4,0xB7, 0x30,0xB2, 0x60,0xEF };
u08 vg_BA[] PROGMEM = { 0x30,0xB7, 0x60,0xEF };
u08 vg_BB[] PROGMEM = { 0x02,0xE2,0xE7, 0x04,0xB4,0xBF };
u08 vg_BC[] PROGMEM = { 0x02,0xB2, 0x04,0xE4, 0x30,0xB2, 0x60,0xEC };
u08 vg_BD[] PROGMEM = { 0x04,0xE4, 0x30,0xB4, 0x60,0xEC };
u08 vg_BE[] PROGMEM = { 0x02,0xB2, 0x04,0xB4, 0x30,0xBC };
u08 vg_BF[] PROGMEM = { 0x04,0xB4,0xBF };
u08 vg_C0[] PROGMEM = { 0x30,0xB4,0xFC };
u08 vg_C1[] PROGMEM = { 0x04,0xF4, 0x30,0xBC };
u08 vg_C2[] PROGMEM = { 0x04,0xF4, 0x34,0xBF };
u08 vg_C3[] PROGMEM = { 0x30,0xB7, 0x34,0xFC };
u08 vg_C4[] PROGMEM = { 0x04,0xFC };
u08 vg_C5[] PROGMEM = { 0x04,0xF4, 0x30,0xBF };
u08 vg_C6[] PROGMEM = { 0x30,0xB7, 0x32,0xF2, 0x34,0xFC };
u08 vg_C7[] PROGMEM = { 0x30,0xB7, 0x60,0xE7, 0x64,0xFC };
u08 vg_C8[] PROGMEM = { 0x30,0xB4,0xF4, 0x60,0xE2,0xFA };
u08 vg_C9[] PROGMEM = { 0x32,0xF2, 0x32,0xB7, 0x64,0xF4, 0x64,0xEF };
u08 vg_CA[] PROGMEM = { 0x02,0xB2, 0x04,0xF4, 0x30,0xB2, 0x60,0xE2,0xFA };
u08 vg_CB[] PROGMEM = { 0x02,0xF2, 0x04,0xB4,0xB7, 0x64,0xF4, 0x64,0xEF };
u08 vg_CC[] PROGMEM = { 0x30,0xB7, 0x60,0xE2,0xF2, 0x64,0xF4, 0x64,0xEF };
u08 vg_CD[] PROGMEM = { 0x02,0xF2, 0x04,0xFC };
u08 vg_CE[] PROGMEM = { 0x02,0xB2, 0x04,0xB4,0xB7, 0x30,0xB2, 0x60,0xE2,0xF2, 0x64,0xF4,
                         0x64,0xEF };
u08 vg_CF[] PROGMEM = { 0x02,0xF2, 0x04,0xF4, 0x40,0xCA };
u08 vg_D0[] PROGMEM = { 0x04,0xF4, 0x30,0xB4, 0x60,0xEC };
u08 vg_D1[] PROGMEM = { 0x02,0xF2, 0x04,0xF4, 0x34,0xBF };
u08 vg_D2[] PROGMEM = { 0x04,0xF4, 0x34,0xB7, 0x64,0xEF };
u08 vg_D3[] PROGMEM = { 0x30,0xB4,0xF4, 0x60,0xEC };
u08 vg_D4[] PROGMEM = { 0x30,0xB4,0xF4, 0x32,0xFA };
u08 vg_D5[] PROGMEM = { 0x32,0xF2, 0x32,0xB7, 0x34,0xFC };
u08 vg_D6[] PROGMEM = { 0x34,0xF4, 0x34,0xB7, 0x64,0xEF };
u08 vg_D7[] PROGMEM = { 0x04,0xF4, 0x30,0xB7, 0x60,0xEF };
u08 vg_D8[] PROGMEM = { 0x02,0xF2, 0x04,0xF4, 0x30,0xBF };
u08 vg_D9[] PROGMEM = { 0x04,0xB4, 0x30,0xBC };
u08 vg_DA[] PROGMEM = { 0x34,0xF4, 0x34,0xBF };
u08 vg_DB[] PROGMEM = { 0x00,0xF0,0xF7, 0x00,0x87,0xF7, 0x01,0xF1, 0x02,0xF2, 0x03,0xF3,
                         0x04,0xF4, 0x05,0xF5, 0x06,0xF6, 0x10,0x97, 0x20,0xA7, 0x30,0xB7,
                         0x40,0xC7, 0x50,0xD7, 0x60,0xEF };
u08 vg_DC[] PROGMEM = { 0x04,0xF4,0xF7, 0x04,0x87,0xF7, 0x05,0xF5, 0x06,0xF6, 0x14,0x97,
                         0x24,0xA7, 0x34,0xB7, 0x44,0xC7, 0x54,0xD7, 0x64,0xEF };
u08 vg_DD[] PROGMEM = { 0x00,0xB0,0xB7, 0x00,0x87,0xB7, 0x01,0xB1, 0x02,0xB2, 0x03,0xB3,
                         0x04,0xB4, 0x05,0xB5, 0x06,0xB6, 0x10,0x97, 0x20,0xAF };
u08 vg_DE[] PROGMEM = { 0x40,0xF0,0xF7, 0x40,0xC7,0xF7, 0x41,0xF1, 0x42,0xF2, 0x43,0xF3,
                         0x44,0xF4, 0x45,0xF5, 0x46,0xF6, 0x50,0xD7, 0x60,0xEF };
u08 vg_DF[] PROGMEM = { 0x00,0xF0,0xF3, 0x00,0x83,0xF3, 0x01,0xF1, 0x02,0xF2, 0x10,0x93,
                         0x20,0xA3, 0x30,0xB3, 0x40,0xC3, 0x50,0xD3, 0x60,0xEB };
u08 vg_E0[] PROGMEM = { 0x03,0x92,0xA2,0xB3,0xC3,0xD2, 0x03,0x85,0x96,0xA6,0xB5,0xC5,0xD6,
                         0x33,0xBD };
u08 vg_E1[] PROGMEM = { 0x02,0x91,0xB1,0xC2, 0x02,0x87, 0x03,0xB3,0xC4, 0x05,0xB5,0xC4,
                         0x33,0xCA };
u08 vg_E2[] PROGMEM = { 0x01,0xC1,0xC2, 0x01,0x8E };
u08 vg_E3[] PROGMEM = { 0x02,0xD2, 0x12,0x96, 0x42,0xCE };
u08 vg_E4[] PROGMEM = { 0x00,0xC0,0xC1, 0x00,0x81,0xA3, 0x05,0xA3, 0x05,0x86,0xC6,
                         0x45,0xCE };
u08 vg_E5[] PROGMEM = { 0x03,0x92,0xD2, 0x03,0x85,0x96,0xA6,0xB5, 0x32,0xBD };
u08 vg_E6[] PROGMEM = { 0x07,0xA5,0xC5,0xD4, 0x11,0x94,0xA5, 0x51,0xDC };
u08 vg_E7[] PROGMEM = { 0x02,0x91,0xA1,0xB2,0xD2,0xE1, 0x32,0xBE };
u08 vg_E8[] PROGMEM = { 0x03,0x92,0xB2,0xC3,0xC4, 0x03,0x84,0x95,0xB5,0xC4, 0x10,0xB0,
                         0x17,0xB7, 0x20,0xAF };
u08 vg_E9[] PROGMEM = { 0x02,0xA0,0xB0,0xD2,0xD4, 0x02,0x84,0xA6,0xB6,0xD4, 0x03,0xDB };
u08 vg_EA[] PROGMEM = { 0x02,0xA0,0xB0,0xD2,0xD3, 0x02,0x83,0x94,0x96, 0x06,0x96,
                         0x44,0xD3, 0x44,0xC6,0xDE };
u08 vg_EB[] PROGMEM = { 0x04,0x93,0xC3, 0x04,0x85,0x96,0xB6,0xC5, 0x31,0xC2,0xC5,
                         0x31,0xC0,0xD8 };
u08 vg_EC[] PROGMEM = { 0x03,0x92,0xA2,0xB3,0xC2,0xD2,0xE3,0xE4, 0x03,0x84,0x95,0xA5,0xB4,0xC5,0xD5,0xE4,
                         0x33,0xBC };
u08 vg_ED[] PROGMEM = { 0x02,0x91,0xD1,0xE2,0xE4, 0x02,0x84,0x95,0xD5,0xE4, 0x06,0xE8 };
u08 vg_EE[] PROGMEM = { 0x02,0xA0,0xB0, 0x02,0x84,0xA6,0xB6, 0x03,0xBB };
u08 vg_EF[] PROGMEM = { 0x01,0x90,0xB0,0xC1,0xC6, 0x01,0x8E };
u08 vg_F0[] PROGMEM = { 0x11,0xD1, 0x13,0xD3, 0x15,0xDD };
u08 vg_F1[] PROGMEM = { 0x12,0xD2, 0x16,0xD6, 0x30,0xBC };
u08 vg_F2[] PROGMEM = { 0x06,0xC6, 0x10,0xB2, 0x14,0xBA };
u08 vg_F3[] PROGMEM = { 0x06,0xC6, 0x12,0xB4, 0x12,0xB8 };
u08 vg_F4[] PROGMEM = { 0x31,0xC0,0xD0,0xE1,0xE2, 0x31,0xBF };
u08 vg_F5[] PROGMEM = { 0x05,0x86,0x97,0xA7,0xB6, 0x30,0xBE };
u08 vg_F6[] PROGMEM = { 0x13,0xD3, 0x30,0xB1, 0x35,0xBE };
u08 vg_F7[] PROGMEM = { 0x02,0x91,0xA1,0xB2,0xC2,0xD1, 0x05,0x94,0xA4,0xB5,0xC5,0xDC };
u08 vg_F8[] PROGMEM = { 0x11,0xA0,0xB0,0xC1,0xC2, 0x11,0x92,0xA3,0xB3,0xCA };
u08 vg_F9[] PROGMEM = { 0x33,0xBC };
u08 vg_FA[] PROGMEM = { 0x3C };
u08 vg_FB[] PROGMEM = { 0x15,0xB7,0xC7, 0x40,0xE0, 0x40,0xCF };
u08 vg_FC[] PROGMEM = { 0x00,0x91,0xA0,0xB0,0xC1,0xC4, 0x11,0x9C };
u08 vg_FD[] PROGMEM = { 0x01,0x90,0xA0,0xB1, 0x04,0xB1, 0x04,0xBC };
u08 vg_FE[] PROGMEM = { 0x22,0xC4, 0x22,0xC2,0xC5, 0x22,0xA5,0xC3, 0x23,0xC5, 0x23,0xC3,
                         0x24,0xC2, 0x24,0xC4, 0x25,0xC5, 0x32,0xBD };
u08 vg_FF[] PROGMEM = { 0x01,0xD1,0xD6, 0x01,0x86,0xDE };

//
// Table of pointers to the character strokes
//
u08 *vgen[256] PROGMEM = {
   &vg_00[0],   &vg_01[0],   &vg_02[0],   &vg_03[0],
   &vg_04[0],   &vg_05[0],   &vg_06[0],   &vg_07[0],
   &vg_08[0],   &vg_09[0],   &vg_0A[0],   &vg_0B[0],
   &vg_0C[0],   &vg_0D[0],   &vg_0E[0],   &vg_0F[0],
   &vg_10[0],   &vg_11[0],   &vg_12[0],   &vg_13[0],
   &vg_14[0],   &vg_15[0],   &vg_16[0],   &vg_17[0],
   &vg_18[0],   &vg_19[0],   &vg_1A[0],   &vg_1B[0],
   &vg_1C[0],   &vg_1D[0],   &vg_1E[0],   &vg_1F[0],
   &vg_20[0],   &vg_21[0],   &vg_22[0],   &vg_23[0],
   &vg_24[0],   &vg_25[0],   &vg_26[0],   &vg_27[0],
   &vg_28[0],   &vg_29[0],   &vg_2A[0],   &vg_2B[0],
   &vg_2C[0],   &vg_2D[0],   &vg_2E[0],   &vg_2F[0],
   &vg_30[0],   &vg_31[0],   &vg_32[0],   &vg_33[0],
   &vg_34[0],   &vg_35[0],   &vg_36[0],   &vg_37[0],
   &vg_38[0],   &vg_39[0],   &vg_3A[0],   &vg_3B[0],
   &vg_3C[0],   &vg_3D[0],   &vg_3E[0],   &vg_3F[0],
   &vg_40[0],   &vg_41[0],   &vg_42[0],   &vg_43[0],
   &vg_44[0],   &vg_45[0],   &vg_46[0],   &vg_47[0],
   &vg_48[0],   &vg_49[0],   &vg_4A[0],   &vg_4B[0],
   &vg_4C[0],   &vg_4D[0],   &vg_4E[0],   &vg_4F[0],
   &vg_50[0],   &vg_51[0],   &vg_52[0],   &vg_53[0],
   &vg_54[0],   &vg_55[0],   &vg_56[0],   &vg_57[0],
   &vg_58[0],   &vg_59[0],   &vg_5A[0],   &vg_5B[0],
   &vg_5C[0],   &vg_5D[0],   &vg_5E[0],   &vg_5F[0],
   &vg_60[0],   &vg_61[0],   &vg_62[0],   &vg_63[0],
   &vg_64[0],   &vg_65[0],   &vg_66[0],   &vg_67[0],
   &vg_68[0],   &vg_69[0],   &vg_6A[0],   &vg_6B[0],
   &vg_6C[0],   &vg_6D[0],   &vg_6E[0],   &vg_6F[0],
   &vg_70[0],   &vg_71[0],   &vg_72[0],   &vg_73[0],
   &vg_74[0],   &vg_75[0],   &vg_76[0],   &vg_77[0],
   &vg_78[0],   &vg_79[0],   &vg_7A[0],   &vg_7B[0],
   &vg_7C[0],   &vg_7D[0],   &vg_7E[0],   &vg_7F[0],
   &vg_80[0],   &vg_81[0],   &vg_82[0],   &vg_83[0],
   &vg_84[0],   &vg_85[0],   &vg_86[0],   &vg_87[0],
   &vg_88[0],   &vg_89[0],   &vg_8A[0],   &vg_8B[0],
   &vg_8C[0],   &vg_8D[0],   &vg_8E[0],   &vg_8F[0],
   &vg_90[0],   &vg_91[0],   &vg_92[0],   &vg_93[0],
   &vg_94[0],   &vg_95[0],   &vg_96[0],   &vg_97[0],
   &vg_98[0],   &vg_99[0],   &vg_9A[0],   &vg_9B[0],
   &vg_9C[0],   &vg_9D[0],   &vg_9E[0],   &vg_9F[0],
   &vg_A0[0],   &vg_A1[0],   &vg_A2[0],   &vg_A3[0],
   &vg_A4[0],   &vg_A5[0],   &vg_A6[0],   &vg_A7[0],
   &vg_A8[0],   &vg_A9[0],   &vg_AA[0],   &vg_AB[0],
   &vg_AC[0],   &vg_AD[0],   &vg_AE[0],   &vg_AF[0],
   &vg_B0[0],   &vg_B1[0],   &vg_B2[0],   &vg_B3[0],
   &vg_B4[0],   &vg_B5[0],   &vg_B6[0],   &vg_B7[0],
   &vg_B8[0],   &vg_B9[0],   &vg_BA[0],   &vg_BB[0],
   &vg_BC[0],   &vg_BD[0],   &vg_BE[0],   &vg_BF[0],
   &vg_C0[0],   &vg_C1[0],   &vg_C2[0],   &vg_C3[0],
   &vg_C4[0],   &vg_C5[0],   &vg_C6[0],   &vg_C7[0],
   &vg_C8[0],   &vg_C9[0],   &vg_CA[0],   &vg_CB[0],
   &vg_CC[0],   &vg_CD[0],   &vg_CE[0],   &vg_CF[0],
   &vg_D0[0],   &vg_D1[0],   &vg_D2[0],   &vg_D3[0],
   &vg_D4[0],   &vg_D5[0],   &vg_D6[0],   &vg_D7[0],
   &vg_D8[0],   &vg_D9[0],   &vg_DA[0],   &vg_DB[0],
   &vg_DC[0],   &vg_DD[0],   &vg_DE[0],   &vg_DF[0],
   &vg_E0[0],   &vg_E1[0],   &vg_E2[0],   &vg_E3[0],
   &vg_E4[0],   &vg_E5[0],   &vg_E6[0],   &vg_E7[0],
   &vg_E8[0],   &vg_E9[0],   &vg_EA[0],   &vg_EB[0],
   &vg_EC[0],   &vg_ED[0],   &vg_EE[0],   &vg_EF[0],
   &vg_F0[0],   &vg_F1[0],   &vg_F2[0],   &vg_F3[0],
   &vg_F4[0],   &vg_F5[0],   &vg_F6[0],   &vg_F7[0],
   &vg_F8[0],   &vg_F9[0],   &vg_FA[0],   &vg_FB[0],
   &vg_FC[0],   &vg_FD[0],   &vg_FE[0],   &vg_FF[0]
};



void vchar(int xoffset,int yoffset, u08 erase, u08 color, u08 c)  // draw a vector character
{
int x1,y1, x2,y2;
u08 VByte;
u08 *VIndex;  // char gen table offset

   // draw a vector graphics character

   if(erase) erase_vchar(xoffset, yoffset);
   if(c == ' ') return;
if(c == DEG_CODE) c = 0xF8;
else if(c == PLUS_MINUS) c = 0xF1;

   x2 = y2 = 0;

   VIndex = vgen[c];  // pointer to strokes for the char

   while(1) {  // draw the character strokes
      VByte = *VIndex;
      if(VByte == 0xFF) break;
      ++VIndex;

      x1 = (VByte >> 4) & 0x07;
      y1 = (VByte & 0x07);

      x1 *= VCharWidth;
      if((c >= 0xB0) && (c <= 0xDF)) {  // make sure line drawing chars touch
         if(((VByte>>4) & 0x07) == (VCHAR_W-1)) x1 += (VCharWidth-1);
      }
      x1 /= VCHAR_W;

      y1 *= VCharHeight;
      if((c >= 0xB0) && (c <= 0xDF)) { // make sure line drawing chars touch
         if((VByte & 0x07) == (VCHAR_H-1)) y1 += (VCharHeight-1);
      }
      y1 /= VCHAR_H;

      if((VByte & 0x80) == 0x00) {  // move to point and draw a dot
         x2 = x1;   // we do dots as a single point line
         y2 = y1;
      }

/////      thick_line(x1+xoffset,y1+yoffset, x2+xoffset,y2+yoffset, color, VCharThickness);
      xthick_line(x1+xoffset,y1+yoffset, x2+xoffset,y2+yoffset, color, VCharThickness);

      if(VByte & 0x08) break;  // end of list

      x2 = x1;  // prepare for next stroke in the character
      y2 = y1;
   }
}


void vchar_string(int row, int col, u08 color, char *s)
{
u08 c;
int i;

   // draw a text string using vector graphics characters

   if(1 && graphics_coords) {
   }
   else {
      row *= TEXT_HEIGHT;  //!!!!!!
      col *= TEXT_WIDTH;
      row += TEXT_Y_MARGIN;
      col += TEXT_X_MARGIN;
row += 2;
   }
   vc_col = col;
   vc_row = row;

   i = 0;
   while((c = *s++)) {  // to save time,  we only draw the chars that changed
      vchar(col, row, 1, color, c);
      if(i<(VSTRING_LEN-1)) ++i;
      col += VCharWidth;
   }
}

void center_vstring(int row, int scale, int color, char *s)
{
int col;
int len;
int i;

   // draw a scaled vector graphics text string that is centered on the screen

   i = VCHAR_SCALE;
   VCHAR_SCALE = scale;

   len = strlen(s);
   col = ((SCREEN_WIDTH-((VCHAR_W*VCHAR_SCALE)*len)) / TEXT_WIDTH) / 2;
   vchar_string(row, col, color, s);

   VCHAR_SCALE = i;
}


void xthick_line(COORD x1,COORD y1,  COORD x2,COORD y2, u08 color, u08 thickness)
{
COORD x,y;

   // draw a thick line on the screen (using single pixel wide lines)

   if(thickness <= 1) {
      return line(x1,y1, x2,y2, color);
   }

   for(x=0; x<thickness; x++) {
      for(y=0; y<thickness; y++) {
         line(x1+x,y1+y, x2+x,y2+y, color);
      }
   }
}

void thick_line(COORD x1,COORD y1,  COORD x2,COORD y2, u08 color, u08 thickness)
{
COORD y;
int i1, i2;
int d1, d2, d;

   // draw a thick line on the screen (using single pixel wide lines)
   // (this version tends to look a little better than xthick_line() defined
   // above but is slower

   if(thickness <= 1) {
      line(x1,y1, x2,y2, color);
   }
   else if(1) {   // more efficient way to draw thick lines
      return xthick_line(x1,y1, x2,y2, color, thickness);
   }
   else {
      i1 = i2 = 1;

      d1 = (int) x2 - (int) x1;
      if(d1 < 0) {
         i1 = (-1);
         d1 = 0 - d1;
      }

      d2 = (int) y2 - (int) y1;
      if(d2 < 0) {
         i2 = (-1);
         d2 = 0 - d2;
      }

      if(d1 > d2) {
         d = d2 + d2 - d1;
         while(1) {
            for(y=y1; y<y1+thickness; y++) {
               line(x1,y, x1+thickness-1,y, color);
            }
            if(x1 == x2) break;
            if(d >= 0) {
               d = d - d1 - d1;
               y1 += i2;
            }
            d = d + d2 + d2;
            x1 += i1;
         }
      }
      else {
         d = d1 + d1 - d2;
         while (1) {
            for(y=y1; y<y1+thickness; y++) {
               line(x1,y, x1+thickness-1,y, color);
            }
            if(y1 == y2) break;
            if(d >= 0) {
               d = d - d2 - d2;
               x1 += i1;
            }
            d = d + d1 + d1;
            y1 += i2;
         }
      }
   }
}


void erase_vchar(int x, int y)
{
int i,j;

   // erase a vector graphics character

//   i = 0;
//   if(VCharThickness > 1) i = 1;
//   if(VCharThickness > 1) i = VCharThickness;
   i = VCharThickness;
   j = 0;
++i;

// if(VCharThickness > 1) ++j;
   erase_rectangle(x,y-j, VCharWidth+VCharThickness,VCharHeight+VCharThickness-i);
}

void erase_help()
{
   // erase the help area on the screen

   if(text_mode) {
      erase_screen();
      return;
   }
   erase_rectangle(PLOT_LEFT,PLOT_ROW, SCREEN_WIDTH-PLOT_LEFT,SCREEN_HEIGHT-PLOT_ROW);
}

int vchar_stroke()
{
   // return vchar line width in pixels

   if(VCHAR_SCALE <= 1) return 1;
   else if(VCHAR_SCALE <= 3) return 2;
   else if(vc_font_scale > 100) return 2;
   else return (VCHAR_SCALE / 2);
}


void erase_lla()
{
   // erase the lat/lon scattergram area on the screen

   if(text_mode) return;
   if(zoom_screen == 'P');
   else if(zoom_screen) return;

   erase_rectangle(LLA_COL,LLA_ROW, LLA_SIZE,LLA_SIZE);
}


void erase_watch()
{
   // erase the analog watch area on the screen
   erase_rectangle(aclock_x-ACLOCK_SIZE/2,aclock_y-ACLOCK_SIZE/2,
                                ACLOCK_SIZE,ACLOCK_SIZE);
}


void erase_azel(int why)
{
   // erase the satellite map area on the screen

   if((zoom_screen == 0) && ((AZEL_ROW+AZEL_SIZE) >= PLOT_ROW)) {  // az/el map is in the normal plot window
      erase_rectangle(PLOT_COL+PLOT_WIDTH,AZEL_ROW, SCREEN_WIDTH-(PLOT_COL+PLOT_WIDTH),AZEL_SIZE);
   }
   else {
      erase_rectangle(AZEL_COL,AZEL_ROW, AZEL_SIZE,AZEL_SIZE);
   }

   azel_erased = 1;
   azel_grid_color = GREY;
}


void erase_screen()
{
   // erase the screen
   erase_rectangle(0,0, SCREEN_WIDTH,SCREEN_HEIGHT);
   show_touch_kbd(0);
   lla_showing = 0;
   ms_row = ms_col = 0;

   #ifdef PRECISE_STUFF
      plot_lla_axes(10);
   #endif
}


void show_last_mouse_x(int color)
{
  // highlight what column the mouse was last in the plot area at

  if(first_key) return;
  if(text_mode) return;
  if(zoom_screen && (zoom_screen != 'P')) return;

  if(last_plot_tick >= 1) {  // erase the old tick mark
     line(PLOT_COL+last_plot_tick-1,PLOT_ROW+0, PLOT_COL+last_plot_tick+1,PLOT_ROW+0, WHITE); // lmq
     line(PLOT_COL+last_plot_tick-1,PLOT_ROW-1, PLOT_COL+last_plot_tick+1,PLOT_ROW-1, BLACK); // lmq
     last_plot_tick = (-1);
  }

  if(last_plot_mouse_x < 1) return;
  if(last_plot_mouse_x == last_plot_tick) return;

  line(PLOT_COL+last_plot_mouse_x-1,PLOT_ROW+0, PLOT_COL+last_plot_mouse_x+1,PLOT_ROW+0, color); // lmq
  line(PLOT_COL+last_plot_mouse_x-1,PLOT_ROW-1, PLOT_COL+last_plot_mouse_x+1,PLOT_ROW-1, color); // lmq
  last_plot_tick = last_plot_mouse_x;
}


void erase_plot(int full_plot)
{
   // erase the plot area on the screen
   //   if full_plot < 0 and plots are disabled, erase the full screen
   //   if full_plot = 0 erase only the graph area
   //   if full_plot > 0 erase the whole plot area

   if(text_mode) return;

   if(((rcvr_type == NO_RCVR) || no_plots) && (full_plot < 0)) {
      erase_screen();
      return;
   }

   if(full_plot < 0) ;
   else if(rcvr_type == NO_RCVR) return;
   else if(no_plots) return;

   if(zoom_screen == 'P');
   else if(zoom_screen) return;

   if(PLOT_COL > PLOT_LEFT) {
      erase_rectangle(PLOT_LEFT,PLOT_ROW, PLOT_COL-PLOT_LEFT,SCREEN_HEIGHT-PLOT_ROW);
   }

   erase_rectangle(PLOT_COL,PLOT_ROW, PLOT_WIDTH,SCREEN_HEIGHT-PLOT_ROW);

   // erase area to the right of the plot area
   if(full_plot && ((PLOT_COL+PLOT_WIDTH) < SCREEN_WIDTH)) {
      erase_rectangle(PLOT_COL+PLOT_WIDTH,PLOT_ROW, SCREEN_WIDTH-(PLOT_COL+PLOT_WIDTH),SCREEN_HEIGHT-PLOT_ROW);
   }
}



//
//
//  Screen drawing primitives
//
//

#ifndef USE_SDL

void dot(int x, int y, u08 color)
{
   // draw a pixel on the screen

   SWAPXY(x,y);

#ifdef WIN_VFX
   if(stage == 0) return;

   VFX_io_done = 1;
   VFX_pixel_write(stage, x,y, palette[color]);
#endif

#ifdef USE_X11
   if(display == 0) return;

   set_x11_color(color);
   XDrawPoint(display,win, gc, x,y);
   flush_x11();
#endif

}


u08 get_pixel(COORD x,COORD y)
{
unsigned long pixel;
int i;

   // gets a pixel from the current screen image

   SWAPXY(x,y);

#ifdef WIN_VFX
   if(stage == 0) return 0;

   VFX_io_done = 1;
   pixel = (unsigned long) VFX_pixel_read(stage, x,y) & 0xFFFF;
   for(i=0; i<16; i++) {  // convert screen value to color code
      if(((u32) pixel) == (palette[i]&0xFFFF)) return i;
   }
#endif

#ifdef USE_X11
   // X11 uses the screen capture image buffer

   if(capture_image == 0) return 0;

   pixel = XGetPixel(capture_image, x,y);
   for(i=0; i<16; i++) {  // convert screen value to color code
      if(pixel == palette[i]) return i;
   }
#endif

   return 0;
}

#endif

void draw_circle(int x,int y, int r, int color, int fill)
{
   // draw a filled or hollow circle on the screen

   SWAPXY(x,y);

   #ifdef WIN_VFX
      if(stage == 0) return;

      if(fill) VFX_ellipse_fill(stage, x,y, r,r, palette[color]);
      else     VFX_ellipse_draw(stage, x,y, r,r, palette[color]);
   #endif

   #ifdef USE_SDL
      if(fill)  filledCircleColor(display,x,y,r, get_sdl_color(color));
      circleColor(display,x,y,r, get_sdl_color(color));
   #endif

   #ifdef USE_X11
      if(display == 0) return;

      r *= 2;
      x = x - (r/2);  // upper left corner of bounding box
      y = y - (r/2);
      set_x11_color(color);
      if(fill) XFillArc(display,win,gc, x,y, r,r, 0,(360*64));
      else     XDrawArc(display,win,gc, x,y, r,r, 0,(360*64));
      flush_x11();
   #endif
}


void line(COORD x1,COORD y1, COORD x2,COORD y2, u08 color)
{
   // draw a single pixel width line on the screen

   SWAPXY(x1,y1);
   SWAPXY(x2,y2);

#ifdef WIN_VFX
   if(stage == 0) return;

   VFX_io_done = 1;
   VFX_line_draw(stage, x1,y1, x2,y2, LD_DRAW, palette[color]);
#endif


#ifdef USE_X11
   if(display == 0) return;

   set_x11_color(color);
   XDrawLine(display,win,gc, x1,y1, x2,y2);
   flush_x11();
#endif

#ifdef USE_SDL
   lineColor(display, x1,y1,x2,y2,get_sdl_color(color));
#endif
}


void draw_rectangle(int x,int y, int width,int height, int color)
{
   // draw a rectangle outline

   line(x,y,              x+width,y,        color);
   line(x+width,y,        x+width,y+height, color);
   line(x+width,y+height, x,y+height,       color);
   line(x,y+height,       x,y,              color);
}


void fill_rectangle(int x,int y, int width,int height, int color)
{
   // fill a rectangle on the screen with color

   #ifdef WIN_VFX
      if(stage == 0) return;
      SWAPXY(x,y);
      SWAP(width,height);
      if(rotate_screen) width = 0-width;

      VFX_io_done = 1;
      VFX_rectangle_fill(stage, x,y, x+width,y+height, LD_DRAW, palette[color]);
   #endif

   #ifdef USE_X11
      if(display == 0) return;
      SWAPXY(x,y);
      SWAP(width,height);
      if(rotate_screen) {
         x -= width;
      }

      set_x11_color(color);
      XFillRectangle(display,win,gc, x,y, width+1,height+1);
      flush_x11();
   #endif

   #ifdef USE_SDL
      SDL_Rect r;
      r.x = x;
      r.y = y;
      r.w = width;
      r.h = height;
      //set_sdl_color(BLACK);
      //SDL_RenderFillRect(ne_renderer, &r);
      SDL_FillRect(display, &r, palette[BLACK]);
   #endif
}


void erase_rectangle(int x,int y, int width,int height)
{
   // erase a rectangle on the screen by filling it with BLACK

   fill_rectangle(x,y, width,height, BLACK);
}



//
//
//  color palette code
//
//


struct CMAP {   // Lady Heather standard color map (R,G,B)
  int r, g, b;
} cmap[16] = {
   {0,     0,   0 },      //0  - black
   {64,   64, 255 },      //1  - dim blue (now a little brighter for better visibility)
   {0,    96,   0 },      //2  - dim green
   {0,   144, 140 },      //3  - dim cyan
   {128,   0,   0 },      //4  - dim red
// {192,   0, 192 },      //5  - dim magenta
   {140,  96,   0 },      //5  - dim magenta is now grotty yellow
   {255,  64,  64 },      //6  - brown
   {128, 128, 128 },      //7  - dim white (was 160)
   {80,   80,  80 },      //8  - grey (was 96)
   {128, 128, 255 },      //9  - blue (now purple, for better visibility)
   {0,   255,   0 },      //10 - green
   {0,   255, 255 },      //11 - cyan
   {255,   0,   0 },      //12 - red
   {255,   0, 255 },      //13 - magenta
   {255, 255,   0 },      //14 - yellow
   {255, 255, 255 }       //15 - white
};

u08 bmp_pal[256];  // .BMP file color palette blue, green, red, filler for 16+1 colors

void edit_cmap(int color, int r, int g, int b)
{
   // change a palette color map entry

   if(color < 0) return;
   if(color > 15) return;

   if(r < 0) r = 0;
   if(g < 0) g = 0;
   if(b < 0) b = 0;
   if(r > 255) r = 255;
   if(g > 255) g = 255;
   if(b > 255) b = 255;

   cmap[color].r = r;
   cmap[color].g = g;
   cmap[color].b = b;

   setup_palette();
   need_redraw = 8878;
}

void setup_palette()
{
int i;
int j;

   // initialize working palettes from the standard palette table

   j = 0;
   for(i=0; i<16; i++) {  // setup native device and .BMP file palettes
      palette[i] = RGB_NATIVE(cmap[i].r, cmap[i].g, cmap[i].b);

      bmp_pal[j++] = cmap[i].b;   // bitmap palette is BGRx format
      bmp_pal[j++] = cmap[i].g;
      bmp_pal[j++] = cmap[i].r;
      bmp_pal[j++] = 0;
   }

   bmp_pal[j++] = 0;    // bmp_yellow - special color used to print yellow on white background
   bmp_pal[j++] = 168;
   bmp_pal[j++] = 92;
   bmp_pal[j++] = 0;
}

u32 get_bgr_palette(int i)
{
u32 color;

   // return BGR color palette table entry i

   color = cmap[i].b;
   color <<= 16;
   color |= ((u32) cmap[i].g) << 8;
   color |= cmap[i].r;
   return color;
}

void invert_screen()
{
   // toggle BLACK and WHITE

   invert_display ^= 1;
   if(invert_display) {  // inverted
      cmap[YELLOW].b = 96;  // bmp_yellow - special color used to print yellow on white background
      cmap[YELLOW].g = 96;
      cmap[YELLOW].r = 255;

      cmap[BLACK].b = 255;
      cmap[BLACK].g = 255;
      cmap[BLACK].r = 255;

      cmap[WHITE].b = 0;
      cmap[WHITE].g = 0;
      cmap[WHITE].r = 0;
   }
   else {  // normal
      cmap[YELLOW].b = 0;    // bmp_yellow - special color used to print yellow on white background
      cmap[YELLOW].g = 255;
      cmap[YELLOW].r = 255;

      cmap[BLACK].b = 0;
      cmap[BLACK].g = 0;
      cmap[BLACK].r = 0;

      cmap[WHITE].b = 255;
      cmap[WHITE].g = 255;
      cmap[WHITE].r = 255;
   }

   setup_palette();
   need_redraw = 8877;
}

void change_palette(char *s)
{
int r,g,b;
int i;
int n;

   // change a color palette table BGR entry

   if(s == 0) return;

   n = sscanf(s, "%d %d %d %d", &i, &r,&g,&b);
   if(n != 4) {
      edit_error("Must specify four value: COLOR R G B");
      return;
   }
   if((i < 0) || (i > 15)) {
      edit_error("COLOR value must be 0 .. 15");
      return;
   }
   if((r < 0) || (r > 255)) {
      edit_error("RED value must be 0 .. 255");
      return;
   }
   if((g < 0) || (g > 255)) {
      edit_error("GREEN value must be 0 .. 255");
      return;
   }
   if((b < 0) || (b > 255)) {
      edit_error("BLUE value must be 0 .. 255");
      return;
   }

   cmap[i].r = r;
   cmap[i].g = g;
   cmap[i].b = b;

   setup_palette();
   need_redraw = 8899;
}


//
//   Screen dump routines
//   !!! Note:  This code might not be non-Intel byte ordering ENDIAN compatible
//
//

#define BM_HEADER_SIZE (0x0036 + 16*4)

FILE *bmp_file;

void dump_byte(u08 b)
{
   fwrite(&b, 1, 1, bmp_file);
}

void dump_word(u16 w)
{
   dump_byte((u08) w);
   dump_byte((u08) (w>>8));
}

void dump_dword(u32 d)
{
   dump_byte(d);
   dump_byte(d>>8);
   dump_byte(d>>16);
   dump_byte(d>>24);
}

int dump_bmp_file(int invert, int top_line)
{
int x, y, c;

   // main header
   dump_byte('B');   dump_byte('M');                          // magic number
   dump_dword((SCREEN_HEIGHT-top_line)*SCREEN_WIDTH/2L+BM_HEADER_SIZE);  // file size
   dump_dword(0L);              // reserved
   dump_dword(BM_HEADER_SIZE);  // offset of bitmap data in the file

   // info header
   dump_dword(40L);             // info header length
   dump_dword(SCREEN_WIDTH);    // screen size
   dump_dword(SCREEN_HEIGHT-top_line);
   dump_word(1);                // number of planes
   dump_word(4);                // bits per pixel
   dump_dword(0L);              // no compression
   dump_dword(SCREEN_WIDTH*(SCREEN_HEIGHT-top_line)/2L);// picture size in bytes
   dump_dword(0L);              // horizontal resolution
   dump_dword(0L);              // vertical resolution
   dump_dword(16L);             // number of used colors
   dump_dword(16L);             // number of important colors

   // palette
   for(c=0; c<16; c++) {
      x = (c << 2);
      if(invert) {
         if     (c == WHITE)  x = (BLACK << 2);
         else if(c == BLACK)  x = (WHITE << 2);
         else if(c == YELLOW) x = (BMP_YELLOW << 2);
      }
      dump_byte(bmp_pal[x++]);
      dump_byte(bmp_pal[x++]);
      dump_byte(bmp_pal[x++]);
      dump_byte(bmp_pal[x]);
   }

   for(y=SCREEN_HEIGHT-1; y>=top_line; y--) {
      update_pwm();
      for(x=0; x<SCREEN_WIDTH; x+=2) {
         c = (get_pixel(x, y) & 0x0F) << 4;
         c |= (get_pixel(x+1, y) & 0x0F);
         dump_byte(c);
      }
   }

   return 1;
}

int dump_screen(int invert, int top_line, char *fn)
{
int x, y;
int redraw;
int do_gif;

   strcpy(out, fn);
   if(strstr(out, ".") == 0) {
      #ifdef GIF_FILES
         strcat(out, ".gif");
      #else
         strcat(out, ".bmp");
      #endif
   }

   bmp_file = topen(out, "wb");
   if(bmp_file == 0) return 0;

   strlwr(out);
   if(strstr(out, ".gif") || strstr(out, ".GIF")) do_gif = 1;
   else do_gif = 0;

   redraw = 0;
   if(mouse_plot_valid && (last_mouse_x >= 0)) {
      if(0) {
         x = PLOT_COL+last_mouse_x;
         y = PLOT_ROW+8;
         line(x-1,PLOT_ROW, x-1,y, WHITE);
         line(x+0,PLOT_ROW, x+0,y, WHITE);
         line(x+1,PLOT_ROW, x+1,y, WHITE);
         line(x-3,y+1,  x+3,y+1, WHITE);
         line(x-2,y+2,  x+2,y+2, WHITE);
         line(x-1,y+3,  x+1,y+3, WHITE);
         line(x-0,y+4,  x+0,y+4, WHITE);
      }
      else {
         x = PLOT_COL+last_mouse_x;
         y = last_mouse_y;
         thick_line(x-5,y-5, x+5,y+5, WHITE, 2);
         thick_line(x+5,y-5, x-5,y+5, WHITE, 2);

//       line(x-5,y-5, x+5,y-5, WHITE);
//       line(x-5,y+5, x+5,y+5, WHITE);
//       line(x-5,y-5, x-5,y+5, WHITE);
//       line(x+5,y-5, x+5,y+5, WHITE);
      }
      redraw = 1;
   }

   if(do_screen_dump == 0) {  // keyboard commanded dump
      BEEP(5);                // beep because this can take a while to do
   }

#ifdef USE_X11   // capture the screen image into a buffer
   if(display == 0) return 0;
   if(capture_image) XDestroyImage(capture_image);
   SWAP(SCREEN_WIDTH, SCREEN_HEIGHT);
   capture_image = XGetImage(display,win, 0,0, SCREEN_WIDTH,SCREEN_HEIGHT, AllPlanes,XYPixmap);
   SWAP(SCREEN_WIDTH, SCREEN_HEIGHT);
#endif

   #ifdef GIF_FILES
      if(do_gif) {
         dump_gif_file(invert, top_line, bmp_file);
      }
      else
   #endif
   {
      dump_bmp_file(invert, top_line);
   }

   fclose(bmp_file);
   bmp_file = 0;

#ifdef USE_X11  // free uo the image capture buffer
   if(capture_image) XDestroyImage(capture_image);
   capture_image = 0;
#endif

   if(do_screen_dump == 0) {
      BEEP(6);
   }

   if(redraw) redraw_screen();
   return 1;
}



//
//
//   Touch screen keyboard
//
//

struct KEY_CAP {
   int row;     // key positon in the keyboard matrix
   int col;
   char *label; // key cap label (max 3 chars to allow for small screens)
   int val;     // the char code to send when the key cap is clicked
};

#define KBD_ROWS 6   // size of the keyboard matrix
#define KBD_COLS 15

#define TOUCH_MARGIN (TEXT_HEIGHT*2) // keyboard top/left offset from screen corner

int key_width;          // the size of a screen key cap in pixels
int key_height;         // the size of a screen key cap in pixels
int caps_lock;          // flag set if caps lock is on
int button_lock;        // flag set if mouse right click mode enabled
int lc_cap_lock_index;  // key table indexes of the CAPS LOCK key
int uc_cap_lock_index;  // ... used to change its label
int lc_but_lock_index;  // key table indexes of the BUTTON LOCK key
int uc_but_lock_index;  // ... used to change its label
int lc_shift_index;     // key table indexes of the SHIFT key
int uc_shift_index;     // ... used to change its label
int shifted;            // flag set if SHIFT key pressed and next key will be shifted

#define TOUCH_CAP (-1)  // special key code values - CAPS LOCK
#define TOUCH_SH  (-2)  // special key code values - SHIFT
#define TOUCH_BUT (-3)  // special key code values - mouse right click enable

struct KEY_CAP normal_lc_table[] = {      // lower case standard keyboard
   { 0,0,  0,      0          },
   { 0,1,  "F1",   F1_CHAR    },
   { 0,2,  "F2",   F2_CHAR    },
   { 0,3,  0,      0          },
   { 0,4,  "F3",   F3_CHAR    },
   { 0,5,  "F4",   F4_CHAR    },
   { 0,6,  0,      0          },
   { 0,7,  "F5",   F5_CHAR    },
   { 0,8,  "F6",   F6_CHAR    },
   { 0,9,  0,      0          },
   { 0,10, "F7",   F7_CHAR    },
   { 0,11, "F8",   F8_CHAR    },
   { 0,12, 0,      0          },
   { 0,13, "F9",   F9_CHAR    },
#ifdef WIN_VFX
   { 0,14, 0,      0          },
#else
   { 0,14, "F10",  F10_CHAR   },
#endif
   { 0,15, 0,      0          },

   { 1,0,  0,      0          },
   { 1,1,  "PUP",  PAGE_UP    },
   { 1,2,  "PDN",  PAGE_DOWN  },
   { 1,3,  0,      0          },
   { 1,4,  "UP",   UP_CHAR    },
   { 1,5,  "DN",   DOWN_CHAR  },
   { 1,6,  0,      0          },
   { 1,7,  "LFT",  LEFT_CHAR  },
   { 1,8,  "RGT",  RIGHT_CHAR },
   { 1,9,  0,      0          },
   { 1,10, "HOM",  HOME_CHAR  },
   { 1,11, "END",  END_CHAR   },
   { 1,12, 0,      0          },
   { 1,13, "INS",  INS_CHAR   },
   { 1,14, "DEL",  DEL_CHAR   },
   { 1,15, 0,      0          },

   { 2,0,  "ESC",  ESC_CHAR   },
   { 2,1,  "`",    '`'        },
   { 2,2,  "1",    '1'        },
   { 2,3,  "2",    '2'        },
   { 2,4,  "3",    '3'        },
   { 2,5,  "4",    '4'        },
   { 2,6,  "5",    '5'        },
   { 2,7,  "6",    '6'        },
   { 2,8,  "7",    '7'        },
   { 2,9,  "8",    '8'        },
   { 2,10, "9",    '9'        },
   { 2,11, "0",    '0'        },
   { 2,12, "-",    '-'        },
   { 2,13, "=",    '='        },
   { 2,14, "BS",   0x08       },
   { 2,15, 0,      0,         },

   { 3,0,  "TAB",  0x09       },
   { 3,1,  "q",    'q'        },
   { 3,2,  "w",    'w'        },
   { 3,3,  "e",    'e'        },
   { 3,4,  "r",    'r'        },
   { 3,5,  "t",    't'        },
   { 3,6,  "y",    'y'        },
   { 3,7,  "u",    'u'        },
   { 3,8,  "i",    'i'        },
   { 3,9,  "o",    'o'        },
   { 3,10, "p",    'p'        },
   { 3,11, "[",    '['        },
   { 3,12, "]",    ']'        },
   { 3,13, "\\",   '\\'       },
   { 3,14, 0,      0,         },
   { 3,15, 0,      0,         },

   { 4,0,  "cap",  TOUCH_CAP  },
   { 4,1,  "mou",  TOUCH_BUT  },
   { 4,2,  "a",    'a'        },
   { 4,3,  "s",    's'        },
   { 4,4,  "d",    'd'        },
   { 4,5,  "f",    'f'        },
   { 4,6,  "g",    'g'        },
   { 4,7,  "h",    'h'        },
   { 4,8,  "j",    'j'        },
   { 4,9,  "k",    'k'        },
   { 4,10, "l",    'l'        },
   { 4,11, ";",    ';'        },
   { 4,12, "'",    '\''       },
   { 4,13, 0,      0          },
   { 4,14, "CR",   0x0D       },
   { 4,15, 0,      0          },

   { 5,0,  "sh",   TOUCH_SH   },
   { 5,1,  0,      0          },
   { 5,2,  "z",    'z'        },
   { 5,3,  "x",    'x'        },
   { 5,4,  "c",    'c'        },
   { 5,5,  "v",    'v'        },
   { 5,6,  "b",    'b'        },
   { 5,7,  "n",    'n'        },
   { 5,8,  "m",    'm'        },
   { 5,9,  ",",    ','        },
   { 5,10, ".",    '.'        },
   { 5,11, "/",    '/'        },
   { 5,12, "SPC",  ' '        },
   { 5,13, 0,      0          },
   { 5,14, "sh",   TOUCH_SH   },
   { 5,15, 0,      0          }
};


struct KEY_CAP calc_lc_table[] = {      // lower case calculator keyboard
   { 0,0,  0,      0          },
   { 0,1,  "F1",   F1_CHAR    },
   { 0,2,  "F2",   F2_CHAR    },
   { 0,3,  0,      0          },
   { 0,4,  "F3",   F3_CHAR    },
   { 0,5,  "F4",   F4_CHAR    },
   { 0,6,  0,      0          },
   { 0,7,  "F5",   F5_CHAR    },
   { 0,8,  "F6",   F6_CHAR    },
   { 0,9,  0,      0          },
   { 0,10, "F7",   F7_CHAR    },
   { 0,11, "F8",   F8_CHAR    },
   { 0,12, 0,      0          },
   { 0,13, "F9",   F9_CHAR    },
#ifdef WIN_VFX
   { 0,14, 0,      0          },
#else
   { 0,14, "F10",  F10_CHAR   },
#endif
   { 0,15, 0,      0          },

   { 1,0,  0,      0          },
   { 1,1,  "PUP",  PAGE_UP    },
   { 1,2,  "PDN",  PAGE_DOWN  },
   { 1,3,  0,      0          },
   { 1,4,  "UP",   UP_CHAR    },
   { 1,5,  "DN",   DOWN_CHAR  },
   { 1,6,  0,      0          },
   { 1,7,  "LFT",  LEFT_CHAR  },
   { 1,8,  "RGT",  RIGHT_CHAR },
   { 1,9,  0,      0          },
   { 1,10, "HOM",  HOME_CHAR  },
   { 1,11, "END",  END_CHAR   },
   { 1,12, 0,      0          },
   { 1,13, "INS",  INS_CHAR   },
   { 1,14, "DEL",  DEL_CHAR   },
   { 1,15, 0,      0          },

   { 2,0,  "ESC",  ESC_CHAR   },
   { 2,1,  "`",    '`'        },
   { 2,2,  "1",    '1'        },
   { 2,3,  "2",    '2'        },
   { 2,4,  "3",    '3'        },
   { 2,5,  "4",    '4'        },
   { 2,6,  "5",    '5'        },
   { 2,7,  "6",    '6'        },
   { 2,8,  "7",    '7'        },
   { 2,9,  "8",    '8'        },
   { 2,10, "9",    '9'        },
   { 2,11, "0",    '0'        },
   { 2,12, "-",    '-'        },
   { 2,13, "*",    '*'        },  // !!!normally =
   { 2,14, "BS",   0x08       },
   { 2,15, 0,      0,         },

   { 3,0,  "TAB",  0x09       },
   { 3,1,  "q",    'q'        },
   { 3,2,  "w",    'w'        },
   { 3,3,  "e",    'e'        },
   { 3,4,  "r",    'r'        },
   { 3,5,  "t",    't'        },
   { 3,6,  "y",    'y'        },
   { 3,7,  "u",    'u'        },
   { 3,8,  "i",    'i'        },
   { 3,9,  "o",    'o'        },
   { 3,10, "p",    'p'        },
   { 3,11, "=",    '='        },  // !!! normally [
   { 3,12, "+",    '+'        },  // !!! normally ]
   { 3,13, "/",    '/'        },  // !!! normally /
   { 3,14, 0,      0,         },
   { 3,15, 0,      0,         },

   { 4,0,  "cap",  TOUCH_CAP  },
   { 4,1,  "mou",  TOUCH_BUT  },
   { 4,2,  "a",    'a'        },
   { 4,3,  "s",    's'        },
   { 4,4,  "d",    'd'        },
   { 4,5,  "f",    'f'        },
   { 4,6,  "g",    'g'        },
   { 4,7,  "h",    'h'        },
   { 4,8,  "j",    'j'        },
   { 4,9,  "k",    'k'        },
   { 4,10, "l",    'l'        },
   { 4,11, ";",    ';'        },
   { 4,12, "'",    '\''       },
   { 4,13, "?",    '?'        },
   { 4,14, "CR",   0x0D       },
   { 4,15, 0,      0          },

   { 5,0,  "sh",   TOUCH_SH   },
   { 5,1,  0,      0          },
   { 5,2,  "z",    'z'        },
   { 5,3,  "x",    'x'        },
   { 5,4,  "c",    'c'        },
   { 5,5,  "v",    'v'        },
   { 5,6,  "b",    'b'        },
   { 5,7,  "n",    'n'        },
   { 5,8,  "m",    'm'        },
   { 5,9,  ",",    ','        },
   { 5,10, ".",    '.'        },
   { 5,11, "pi",   '\\'       }, // !!! was /
   { 5,12, "SPC",  ' '        },
   { 5,13, 0,      0          },
   { 5,14, "sh",   TOUCH_SH   },
   { 5,15, 0,      0          }
};


struct KEY_CAP uc_table[] = {     // upper case keyboard
   { 0,0,  0,      0          },
   { 0,1,  "F1",   F1_CHAR    },
   { 0,2,  "F2",   F2_CHAR    },
   { 0,3,  0,      0          },
   { 0,4,  "F3",   F3_CHAR    },
   { 0,5,  "F4",   F4_CHAR    },
   { 0,6,  0,      0          },
   { 0,7,  "F5",   F5_CHAR    },
   { 0,8,  "F6",   F6_CHAR    },
   { 0,9,  0,      0          },
   { 0,10, "F7",   F7_CHAR    },
   { 0,11, "F8",   F8_CHAR    },
   { 0,12, 0,      0          },
   { 0,13, "F9",   F9_CHAR    },
#ifdef WIN_VFX
   { 0,14, 0,      0          },
#else
   { 0,14, "F10",  F10_CHAR   },
#endif
   { 0,15, 0,      0          },

   { 1,0,  0,      0          },
   { 1,1,  "PUP",  PAGE_UP    },
   { 1,2,  "PDN",  PAGE_DOWN  },
   { 1,3,  0,      0          },
   { 1,4,  "UP",   UP_CHAR    },
   { 1,5,  "DN",   DOWN_CHAR  },
   { 1,6,  0,      0          },
   { 1,7,  "LFT",  LEFT_CHAR  },
   { 1,8,  "RGT",  RIGHT_CHAR },
   { 1,9,  0,      0          },
   { 1,10, "HOM",  HOME_CHAR  },
   { 1,11, "END",  END_CHAR   },
   { 1,12, 0,      0          },
   { 1,13, "INS",  INS_CHAR   },
   { 1,14, "DEL",  DEL_CHAR   },
   { 1,15, 0,      0          },

   { 2,0,  "ESC",  ESC_CHAR   },
   { 2,1,  "~",    '~'        },
   { 2,2,  "!",    '!'        },
   { 2,3,  "@",    '@'        },
   { 2,4,  "#",    '#'        },
   { 2,5,  "$",    '$'        },
   { 2,6,  "%",    '%'        },
   { 2,7,  "^",    '^'        },
   { 2,8,  "&",    '&'        },
   { 2,9,  "*",    '*'        },
   { 2,10, "(",    '('        },
   { 2,11, ")",    ')'        },
   { 2,12, "_",    '_'        },
   { 2,13, "+",    '+'        },
   { 2,14, "BS",   0x08       },
   { 2,15, 0,      0,         },

   { 3,0,  "TAB",  0x09       },
   { 3,1,  "Q",    'Q'        },
   { 3,2,  "W",    'W'        },
   { 3,3,  "E",    'E'        },
   { 3,4,  "R",    'R'        },
   { 3,5,  "T",    'T'        },
   { 3,6,  "Y",    'Y'        },
   { 3,7,  "U",    'U'        },
   { 3,8,  "I",    'I'        },
   { 3,9,  "O",    'O'        },
   { 3,10, "P",    'P'        },
   { 3,11, "{",    '{'        },
   { 3,12, "}",    '}'        },
   { 3,13, "|",    '|'        },
   { 3,14, 0,      0,         },
   { 3,15, 0,      0,         },

   { 4,0,  "cap",  TOUCH_CAP  },
   { 4,1,  "mou",  TOUCH_BUT  },
   { 4,2,  "A",    'A'        },
   { 4,3,  "S",    'S'        },
   { 4,4,  "D",    'D'        },
   { 4,5,  "F",    'F'        },
   { 4,6,  "G",    'G'        },
   { 4,7,  "H",    'H'        },
   { 4,8,  "J",    'J'        },
   { 4,9,  "K",    'K'        },
   { 4,10, "L",    'L'        },
   { 4,11, ":",    ':'        },
   { 4,12, "\"",   '"'        },
   { 4,13, "?",    '?'        },
   { 4,14, "CR",   0x0D       },
   { 4,15, 0,      0          },

   { 5,0,  "SH",   TOUCH_SH   },
   { 5,1,  0,      0          },
   { 5,2,  "Z",    'Z'        },
   { 5,3,  "X",    'X'        },
   { 5,4,  "C",    'C'        },
   { 5,5,  "V",    'V'        },
   { 5,6,  "B",    'B'        },
   { 5,7,  "N",    'N'        },
   { 5,8,  "M",    'M'        },
   { 5,9,  "<",    '<'        },
   { 5,10, ">",    '>'        },
   { 5,11, "?",    '?'        },
   { 5,12, "SPC",  ' '        },
   { 5,13, 0,      0          },
   { 5,14, "SH",   TOUCH_SH   },
   { 5,15, 0,      0          }
};



void show_key(int row, int col, int invert)
{
int num_keys;
int i;
int shift;
int x,y;
int fg, bg;
unsigned char *old_font;
unsigned char old_height;
unsigned char old_width;
struct KEY_CAP *table;
char *lab;

   // Draw a touch screen keyboard key.  Invert allows flashing the key when
   // it is touched.

   if(invert) {            // set keycap colors
      bg = KBD_FG_COLOR;
      fg = KBD_BG_COLOR;
   }
   else {
      fg = KBD_FG_COLOR;
      bg = KBD_BG_COLOR;
   }

   x = (col * key_width) + TOUCH_MARGIN;  // where to draw the key cap
   y = (row * key_height) + TOUCH_MARGIN;

   if(rotate_screen) i = 1;  // rotate_screen
   else i = 0;
   draw_rectangle(x,y, key_width,key_height+i, KBD_GRID_COLOR);   // key outline
   fill_rectangle(x+1,y+1, key_width-2,key_height-2, bg);         // key interior

   old_font = dot_font;       // switch to large font for key labels
   old_height = TEXT_HEIGHT;
   old_width = TEXT_WIDTH;

   dot_font = (unsigned char *) &h_large_font[0];
   TEXT_HEIGHT = dot_font[8];         // char height
   TEXT_WIDTH = character_width('M'); // character_width('M');

   shift = (shifted ^ caps_lock);

   if(shift) {
      table = &uc_table[0];
      num_keys = sizeof(uc_table) / sizeof(struct KEY_CAP);
   }
   else if(rpn_mode || last_zoom_calc) {
      table = &calc_lc_table[0];
      num_keys = sizeof(calc_lc_table) / sizeof(struct KEY_CAP);
   }
   else {
      table = &normal_lc_table[0];
      num_keys = sizeof(normal_lc_table) / sizeof(struct KEY_CAP);
   }

   for(i=0; i<num_keys; i++) {  // find the key in the table and display it
      lab = table[i].label;
      if(lab == 0) ;
      else if((table[i].row == row) && (table[i].col == col)) {
         ++graphics_coords;
         x += ((key_width - (strlen(lab)*TEXT_WIDTH*vc_font_scale/100)) / 2);
         y += ((key_height - TEXT_HEIGHT*vc_font_scale/100) / 2);
         vidstr(y,x, fg, lab);
         --graphics_coords;
         break;
      }
   }

   dot_font = old_font;       // restore font size
   TEXT_HEIGHT = old_height;
   TEXT_WIDTH = old_width;
}


void show_touch_kbd(int erase)
{
int t,l, b,r;  // top,left, bottom,right
int row,col;
int key_size;
unsigned i;

   // draw the touch screen keyboard (upper or lower case)

   if(zoom_screen != 'K') return;

   t = 0+TOUCH_MARGIN;   // allowable keyboard outline
   l = 0+TOUCH_MARGIN;

   b = PLOT_ROW-TOUCH_MARGIN;
   r = SCREEN_WIDTH-TOUCH_MARGIN;

   if(0) {  // always square keys
     key_size = (b-t)/KBD_ROWS;
     if(((r-l)/KBD_COLS) < key_size) key_size = ((r-l)/KBD_COLS);
     r = TOUCH_MARGIN + (KBD_COLS*key_size);
     key_width = key_height = key_size;
   }
   else {   // possible rectangular keys to fill the keyboard space
      key_height = (b-t)/KBD_ROWS;
      key_width = (r-l)/KBD_COLS;
   }

   if(erase) {  // screen erase needed
      erase_screen();
   }

   for(row=0; row<KBD_ROWS; row++) {     // draw the keyboard grid
      for(col=0; col<KBD_COLS; col++) {
         show_key(row,col, 0);
      }

      t += key_height;
   }

   ++graphics_coords;
   vidstr(t+TEXT_HEIGHT,l, KBD_TITLE_COLOR, "Touch screen keyboard.  Enter a command or touch any blank key to exit.");
   --graphics_coords;

   // get the keyboard table indexes of the SHIFT and CAPS LOCK keys
   // so we can change the labels depending upon their state
   for(i=0; i<((sizeof(normal_lc_table) / sizeof(struct KEY_CAP))); i++) {
      if(normal_lc_table[i].val == TOUCH_SH) lc_shift_index = i;
      if(normal_lc_table[i].val == TOUCH_CAP) uc_cap_lock_index = i;
      if(normal_lc_table[i].val == TOUCH_BUT) lc_but_lock_index = i;
   }
   for(i=0; i<((sizeof(uc_table) / sizeof(struct KEY_CAP))); i++) {
      if(uc_table[i].val == TOUCH_SH) uc_shift_index = i;
      if(uc_table[i].val == TOUCH_CAP) uc_cap_lock_index = i;
      if(uc_table[i].val == TOUCH_BUT) uc_but_lock_index = i;
   }
}


void hide_kbd(int why)
{
   // turn off the touch screen keyboard display
   if(zoom_screen != 'K') return;  // keyboard already off

   getting_string = 0;
   need_redraw = 3399;
   reset_first_key(2);

   if(0) {   // remove the keyboard from the screen
      add_kbd('Z');  // simulate keystrokes (interfers with clearing plot markers
      add_kbd('N');  // since this resets "last_was_mark")
   }
   else {            // remove keyboard without key strokes
      remove_zoom();
   }
}


int touch_key(int x,int y)
{
int num_keys;
int shift;
int i;
struct KEY_CAP *table;

   // Convert a mouse click on the touch screen keyboard to a simulated
   // keystroke.  Return the simulated keystoke value or 0 if blank/no key
   // was pressed.

   if(key_width <= 0) return 0;
   if(key_height <= 0) return 0;
   if(x < TOUCH_MARGIN) return 0;
   if(x > (TOUCH_MARGIN + (key_width*KBD_COLS))) return 0;
   if(y < TOUCH_MARGIN) return 0;
   if(y > (TOUCH_MARGIN + (key_height*KBD_ROWS))) return 0;

   x -= TOUCH_MARGIN;
   x /= key_width;
   y -= TOUCH_MARGIN;
   y /= key_height;

   shift = (shifted ^ caps_lock);

   if(x >= KBD_COLS) return 0;
   if(y >= KBD_ROWS) return 0;


   click_sound(0);  // keyboard typing key clicks

   if(shift) {  // upper case/shiftet keys
      table = &uc_table[0];
      num_keys = sizeof(uc_table) / sizeof(struct KEY_CAP);
   }
   else if(rpn_mode || last_zoom_calc) {
      table = &calc_lc_table[0];
      num_keys = sizeof(calc_lc_table) / sizeof(struct KEY_CAP);
   }
   else {
      table = &normal_lc_table[0];
      num_keys = sizeof(normal_lc_table) / sizeof(struct KEY_CAP);
   }

   for(i=0; i<num_keys; i++) {  // search the key table and look up keystroke value
      if(table[i].label == 0) ;
      else if((table[i].row == y) && (table[i].col == x)) {
         show_key(y,x, 1);        // flash the touched key
         refresh_page();
         Sleep(KBD_FLASH_TIME);
         show_key(y,x, 0);

         return table[i].val;
      }
   }

   return 0;
}



//
//
//   Some misc functions
//
//

void find_endian(void)
{
u08 s[2];
u16 v;

   // determine the system byte ordering (and adjust for receiver message
   // byte ordering)

   s[0] = 0x34;
   s[1] = 0x12;
   v = *((u16 *) (void *) &s[0]);
   v &= 0xFFFF;
   if((v&0xFFFF) != (0x1234&0xFFFF)) ENDIAN = 1;
   else ENDIAN = 0;  // intel byte ordering

   if(LOW_BYTE_FIRST) {  // receiver messages use little endian format
      ENDIAN ^= 1;
   }
}

void end_log()
{
   // close out the log file if it is open

   if(log_file == 0) return;

   if(log_comments) {
      sprintf(log_text, "#");
      write_log_comment(1);
      write_log_comment(1);
   }

   if(com_errors) {
      printf("*** %lu COM PORT ERRORS!!!\n", (unsigned long) com_errors);
      sprintf(log_text, "#*** %lu COM PORT ERRORS!!!", (unsigned long) com_errors);
      write_log_comment(1);
   }
   if(math_errors) {
      printf("*** %lu MATH ERRORS!!!\n", (unsigned long) math_errors);
      sprintf(log_text, "#*** %lu MATH ERRORS!!!", (unsigned long) math_errors);
      write_log_comment(1);
   }
   if(1 || bad_packets) {
      printf("%lu Rvcr packets processed.  %lu bad packets.\n", (unsigned long) packet_count, (unsigned long) bad_packets);
      sprintf(log_text, "#*** %lu Rcvr packets processed.  %lu bad packets.", (unsigned long) packet_count, (unsigned long) bad_packets);
      write_log_comment(1);
   }

   close_log_file();  // close the log file
}


void shut_down(int reason)
{
unsigned port;

   // clean up in preparation for program exit
#ifdef WINDOWS
   if(timer_set) KillTimer(hWnd, 0);
#endif

   if(rcvr_type == RFTG_RCVR) {
      set_rftg_normal();
      start_rftg_request(0);
      Sleep(500);
   }

#ifdef ADEV_STUFF
   log_adevs();          // write adev info to the log file
#endif

   log_stats();

   end_log();            // close out the ASCII log file

   if(raw_file) {        // close out raw receiver data dump file
      fclose(raw_file);
      raw_file = 0;
   }

   if(prn_file) {        // close out sat az/el/sig level data dump file
      fclose(prn_file);
      prn_file = 0;
   }

   if(rinex_file) {      // close out RINEX raw receiver measurement data dump file
      fclose(rinex_file);
      rinex_file = 0;
   }

   if(ticc_file) {        // close out TICC data dump file
      fclose(ticc_file);
      ticc_file = 0;
   }

   if(sim_file) {
      fclose(sim_file);
      sim_file = 0;
   }

   if(in_file) {
      fclose(in_file);
      in_file = 0;
   }

   if(hist_file) {
      fclose(hist_file);
      hist_file = 0;
   }

   if(temp_script) {
      fclose(temp_script);
      temp_script = 0;
   }

   if(x72_file) {
      fclose(x72_file);
      x72_file = 0;
   }

   #ifdef PRECISE_STUFF
      close_lla_file(0);  // close the lat/lon/alt file
   #endif

   #ifdef TEMP_CONTROL
      disable_temp_control();
   #endif

   #ifdef OSC_CONTROL
      disable_osc_control();
   #endif

   for(port=0; port<NUM_COM_PORTS; port++) {  // shut down open com ports
      kill_com(port, 2100);
   }
   kill_screen();     // kill graphics mode/window, return to text

   free_fft();        // free up any allocated memory
   free_gif();
   free_adev_queues();
   free_mtie();
   free_plot();

   exit(reason);
}

void error_exit(int num,  char *s)
{
char err[1024];

   if(s) sprintf(err, "Error exit %d: %s", num, s);
   else  sprintf(err, "Error exit %d!", num);

   break_flag = 1;

   #ifdef WIN_VFX
      SAL_alert_box("Error", err);
   #else   // __linux__  __MACH__  __FreeBSD__
      printf("*** ERROR: %s\n", err);
   #endif

   if(num) shut_down(num);
}



//
//
//  Data queue support
//
//

struct PLOT_Q get_plot_q(long i)
{
static struct PLOT_Q q;
DATA_SIZE color_sum;

   // returns entry "i" from the plot queue.  If "i" is out of range, return
   // the last queue entry that we fetched.  This should show up as a duplicate
   // time stamp error...


   if(plot_q == 0) return q;
   if(i >= plot_q_size) return q;
   if(i < 0) return q;

   q = plot_q[i];

   if(luxor) {
      if(show_color_pct) {  // scale color values to percentages
         color_sum = (DATA_SIZE) ((q.data[REDHZ] + q.data[GREENHZ] + q.data[BLUEHZ]) / (DATA_SIZE) 100.0);
         if(color_sum) {
            q.data[REDHZ] /= color_sum;
            q.data[GREENHZ] /= color_sum;
            q.data[BLUEHZ] /= color_sum;
            q.data[WHITEHZ] /= color_sum;
         }
         else {
            q.data[REDHZ] = 0.0;
            q.data[GREENHZ] = 0.0;
            q.data[BLUEHZ] = 0.0;
            q.data[WHITEHZ] = 0.0;
         }
      }
      else if(show_color_uw) {
         q.data[BLUEHZ] /= (DATA_SIZE) BLUE_SENS;     // blue
         q.data[GREENHZ] /= (DATA_SIZE)  GREEN_SENS;  // green
         q.data[REDHZ] /= (DATA_SIZE) RED_SENS;       // red
         q.data[WHITEHZ] /= (DATA_SIZE) WHITE_SENS;   // white
      }
   }
   else if(USES_PLOT_THREE) ;
   else if(graph_lla == 0) {
      q.data[THREE] = q.data[TEMP] - (q.data[DAC] * d_scale);  // !!!!
   }

   return q;
}


void put_plot_q(long i, struct PLOT_Q q)
{
   // save a structure entry in the plot queue

   if(i >= plot_q_size) return;
   if(i < 0) return;

   plot_q[i] = q;

   return;
}

void replace_plot_q(long i)
{
struct PLOT_Q q0, q1, q2;
int j;

   // replace a plot queue point's data with the average of the adjacent points

   q0 = get_plot_q(i-1);
   q1 = get_plot_q(i);
   q2 = get_plot_q(i+1);
   for(j=0; j<NUM_PLOTS+DERIVED_PLOTS; j++) {
      q1.data[j] = q0.data[j];  // replace with previous point's data
//    q1.data[j] = (q0.data[j] + q2.data[j]) / (DATA_SIZE) 2.0;  // replace with average of adjacent points
//    q1.data[j] = q2.data[j];  // replace with next point's data
   }
   put_plot_q(i, q1);
}

void deglitch_queue_point()
{
   // replace a plot queue mouse point data with the average of the adjacent points

   if(mouse_plot_valid) {
      replace_plot_q(last_plot_glitch);
      ++last_plot_glitch;
      need_redraw = 5832;
   }
   else {
      BEEP(7);
   }
}

void deglitch_plot_queue(int pid, DATA_SIZE sigma)
{
long i;
long count;
long dg_count;
long pcount;
int id, start, end;
int col;
struct PLOT_Q q0, q;
DATA_SIZE sdev, avg;
DATA_SIZE val;

   // replace all queue points that are more than 3 standard deviations away
   // from the average value.  If pid < 0 all plots are deglitched, otherwise
   // only the specified plot is deglitched.


   if(queue_interval == 0) return;
   if(stat_count == 0) return;
   if(pid > (NUM_PLOTS+DERIVED_PLOTS)) return;

   if(pid < 0) {  // do all plots
      start = 0;
      end = (NUM_PLOTS+DERIVED_PLOTS-1);
if(debug_file) {
   fprintf(debug_file, "deglitch all plots: sigma:%g\n", sigma);
}
   }
   else {  // do single specified plot
      start = end = pid;
if(debug_file) {
   fprintf(debug_file, "deglitch plot %s: sigma:%g\n", plot[pid].plot_id,sigma);
}
   }

   count = 0;
   dg_count = 0;
   col = 0;
   i = plot_q_col0;   // deglitch displayed points
   pcount = plot_q_size;
////i = 0;            // deglitch all queue points
   while(pcount) {  // scan the data that is in the plot queue
      --pcount;
      if(i > 0) q0 = get_plot_q(i-1); // previous point
      else      q0 = get_plot_q(i);
      q = get_plot_q(i);              // point to deglitch

      for(id=start; id<=end; id++) {  // deglitch each plots' entries
         if(rcvr_type == CS_RCVR) ;        // don't deglitch message offset/jitter data
         else if(rcvr_type == TICC_RCVR) ;
         else if(id == NINE) continue;
         else if(id == TEN) continue;

         sdev = ((plot[id].sum_yy/stat_count) - ((plot[id].sum_y/stat_count)*(plot[id].sum_y/stat_count)) );
         if(sdev < (DATA_SIZE) 0.0) sdev = (DATA_SIZE) 0.0 - sdev;
         sdev = (DATA_SIZE) sqrt(sdev);

         avg = (DATA_SIZE) (plot[id].sum_y/stat_count);

         val = (DATA_SIZE) q.data[id]/(DATA_SIZE) queue_interval;

         if(val < (avg - (sdev*sigma))) {
            q.data[id] = q0.data[id];
            ++dg_count;
//sprintf(debug_text, "deglitch < %s:  #%ld  queue %ld   sdev:%g  val:%g  avg:%g", plot[id].plot_id, dg_count, i, sdev, val, avg);
if(debug_file) fprintf(debug_file, "deglitch < %s:  #%ld   queue %ld   sdev:%g  val:%g  avg:%g\n", plot[id].plot_id, dg_count, i, sdev, val, avg);
            put_plot_q(i, q);

         }
         else if(val > (avg + (sdev*sigma))) {
            q.data[id] = q0.data[id];
            ++dg_count;
//sprintf(debug_text, "deglitch > %s:  #%ld  queue %ld   sdev:%g  val:%g  avg:%g", plot[id].plot_id, dg_count, i, sdev, val, avg);
if(debug_file) fprintf(debug_file, "deglitch > %s:  #%ld   queue %ld   sdev:%g  val:%g  avg:%g\n", plot[id].plot_id, dg_count, i, sdev, val, avg);
            put_plot_q(i, q);
         }
      }

      ++i;
      if(i >= plot_q_size) i = 0;
      if(++count > plot_q_count) break;  // deglitch all points

      col += plot_mag;    // deglitch displayed points
      if(col >= (PLOT_WIDTH*view_interval)) break;
   }
// sprintf(debug_text3, "count:%ld  col:%d", count,col);
}


void trim_plot_queue(int trim)
{
long count;

   // trim leading (the entries before the first displayed point)
   // entries from the plot queue

   if(trim & TRIM_Q_START) {
      count = plot_q_col0 - plot_q_out;
      if(count < 0) count += plot_q_size;  // number of entries to remove

      if(count > plot_q_size) ;   // can't trim the start
      else if(count > plot_q_count) ;
      else {
         plot_q_out = plot_q_col0;
         plot_q_count -= count;

         if(trim & TRIM_Q_END) {  //trimming both ends from the queue
            count = plot_q_in - plot_q_last_col;
            if(count < 0) count += plot_q_size;  // number of entries to remove
            plot_q_col0 = plot_q_in - count;
            if(plot_q_col0 < 0) plot_q_col0 += plot_q_size;
            goto both_ends;
         }
      }
   }

   if(trim & TRIM_Q_END) {
      count = plot_q_last_col - plot_q_col0;
      if(count < 0) count += plot_q_size;  // number of entries to remove

      both_ends:
      if(count > plot_q_size) ;
      else if(count > plot_q_count) ;
      else if(count < 0) ;
      else {
         plot_q_in = plot_q_col0;
         plot_q_count -= count;
      }
   }
}


void clear_plot_entry(long i)
{
struct PLOT_Q q;
int j;

   // clear out the data in a plot queue entry

// for(j=0; j<NUM_PLOTS; j++) q.data[j] = (DATA_SIZE) 0.0;  // derived_plots OK
   for(j=0; j<NUM_PLOTS+DERIVED_PLOTS; j++) q.data[j] = (DATA_SIZE) 0.0;  // derived_plots OK

   q.sat_flags = 0;
   q.q_jd = 0.0;

   put_plot_q(i, q);

   if(i) {   // clear q entry marker, if set
      for(j=0; j<MAX_MARKER; j++) {
         if(mark_q_entry[j] == i) mark_q_entry[j] = 0;
      }
   }
}

void free_adev_queues()
{
   // release the adev queue memory
   if(pps_adev_q) {
      free(pps_adev_q);
      pps_adev_q = 0;
   }

   if(osc_adev_q) {
      free(osc_adev_q);
      osc_adev_q = 0;
   }

   if(chc_adev_q) {
      free(chc_adev_q);
      chc_adev_q = 0;
   }

   if(chd_adev_q) {
      free(chd_adev_q);
      chd_adev_q = 0;
   }

   adev_q_allocated = 0;
   return;
}

void alloc_adev()
{
#ifdef ADEV_STUFF
long i;

   // allocate memory for the adev data queues

   free_adev_queues(); // adev queue memory already allocated, free it

//   adev_q_size++;  // add an extra entry for overflow protection
   i = 0L;

   pps_adev_q = (OFS_SIZE *) calloc(adev_q_size+10L, sizeof (OFS_SIZE));
   if(pps_adev_q == 0) {
      sprintf(out, "Could not allocate %lu x %lu byte PPS adev queue",
                    (unsigned long) (adev_q_size), (unsigned long) sizeof(OFS_SIZE));
      error_exit(50, out);
   }

   osc_adev_q = (OFS_SIZE *) calloc(adev_q_size+10L, sizeof (OFS_SIZE));
   if(osc_adev_q == 0) {
      sprintf(out, "Could not allocate %lu x %lu byte OSC adev queue",
                    (unsigned long) (adev_q_size), (unsigned long) sizeof(OFS_SIZE));
      error_exit(50, out);
   }

   chc_adev_q = (OFS_SIZE *) calloc(adev_q_size+10L, sizeof (OFS_SIZE));
   if(chc_adev_q == 0) {
      sprintf(out, "Could not allocate %lu x %lu byte chC adev queue",
                    (unsigned long) (adev_q_size), (unsigned long) sizeof(OFS_SIZE));
      error_exit(50, out);
   }

   chd_adev_q = (OFS_SIZE *) calloc(adev_q_size+10L, sizeof (OFS_SIZE));
   if(chd_adev_q == 0) {
      sprintf(out, "Could not allocate %lu x %lu byte chD adev queue",
                    (unsigned long) (adev_q_size), (unsigned long) sizeof(OFS_SIZE));
      error_exit(50, out);
   }

   adev_q_allocated = 1;
#endif
}


void free_plot()
{
   // release the plot queue memory

   if(plot_q == 0) return;
   free(plot_q);
   plot_q = 0;
   return;
}

void alloc_plot()
{
long i;

   // allocate memory for the plot data queue

   free_plot();
   if(plot_q) return; // plot queue memory already allocated

   plot_q = (struct PLOT_Q *) calloc(plot_q_size+1L, sizeof(struct PLOT_Q));
   if(plot_q == 0) i = 0;
   else            i = 1;

   if(plot_q == 0) {
      sprintf(out, "Could not allocate %lu x %lu byte plot queue",
                    (unsigned long) (plot_q_size+1L), (unsigned long) sizeof(struct PLOT_Q));
      error_exit(51, out);
   }
}

void free_gif()
{
#ifdef GIF_FILES
   // free the gif hash table memory

   if(hash_table) free(hash_table);
   hash_table = 0;
#endif
}

void alloc_gif()
{
#ifdef GIF_FILES
long i;

   // allocate memory for the .GIF file encoder

   if(hash_table) {  // memory alrady allocated
      free_gif();
   }

   hash_table = (unsigned long *) calloc(HT_SIZE+1L, sizeof (unsigned long));
   if(hash_table == 0) i = 0;
   else                i = 1;

   if(hash_table == 0) {
      sprintf(out, "Could not allocate %lu x %lud byte GIF hash table",
                    (unsigned long) (HT_SIZE+1L), (unsigned long) sizeof (unsigned long));
      error_exit(52, out);
   }
#endif
}



void free_fft()
{
#ifdef FFT_STUFF
   if(tsignal) free(tsignal);
   if(fft_out) free(fft_out);
   if(w) free(w);
   if(cf) free(cf);

   tsignal = 0;
   fft_out = 0;
   w = 0;
   cf = 0;
#endif
}


void alloc_fft()
{
#ifdef FFT_STUFF
long i;

   // allocate memory for the FFT routine
   free_fft();
   if(max_fft_len == 0) max_fft_len = 1024;

   if(tsignal == 0) {
      i = sizeof(float);
      i *= (max_fft_len+2);
      tsignal = (float *) (void *) calloc(i, 1);
   }
   if(fft_out == 0) {
      i = sizeof(COMPLEX);
      i *= (max_fft_len/2+2);
      fft_out = (COMPLEX *) (void *) calloc(i, 1);
   }
   if(w == 0) {
      i = sizeof(COMPLEX);
      i *= (max_fft_len/2+2);
      w = (COMPLEX *) (void *) calloc(i, 1);
   }
   if(cf == 0) {
      i = sizeof(COMPLEX);
      i *= (max_fft_len/2+2);
      cf = (COMPLEX *) (void *) calloc(i, 1);
   }
   if((tsignal == 0) || (fft_out == 0) || (w == 0) || (cf == 0)) {
      sprintf(out, "Could not allocate FFT tables");
      error_exit(53, out);
   }
#endif
}

void alloc_memory()
{
   printf("\nInitializing memory...");

//!!!! alloc_adev();
   alloc_plot();
   alloc_gif();
   alloc_fft();

   reset_queues(RESET_ALL_QUEUES, 1000);

   printf("\nDone...\n\n");
}

void reset_marks(void)
{
int i;

   for(i=0; i<MAX_MARKER; i++) {  // clear all the plot markers
      mark_q_entry[i] = 0;
   }
}

void reset_queues(int queue_type, int why)
{
#ifdef ADEV_STUFF
int i;
int k;

// sprintf(debug_text2, "reset queue:%04x  why:%d", queue_type, why);

   if(queue_type & RESET_ADEV_Q) { // reset adev queue
      adev_q_in = 0;
      adev_q_out = 0;
      adev_q_count = 0;
      adev_time = 0;
      last_bin_count = 0;

      pps_adev_q_in = 0;
      pps_adev_q_out = 0;
      pps_adev_q_count = 0;
      pps_adev_q_overflow = 0.0;
      pps_adev_disp = 0; // ADEV_DISPLAY_RATE;

      osc_adev_q_in = 0;
      osc_adev_q_out = 0;
      osc_adev_q_count = 0;
      osc_adev_q_overflow = 0.0;
      osc_adev_disp = 0; // ADEV_DISPLAY_RATE;

      chc_adev_q_in = 0;
      chc_adev_q_out = 0;
      chc_adev_q_count = 0;
      chc_adev_q_overflow = 0.0;
      chc_adev_disp = 0; // ADEV_DISPLAY_RATE;

      chd_adev_q_in = 0;
      chd_adev_q_out = 0;
      chd_adev_q_count = 0;
      chd_adev_q_overflow = 0.0;
      chd_adev_disp = 0; // ADEV_DISPLAY_RATE;

      last_pps_bin_count = 0;
      last_osc_bin_count = 0;
      last_chc_bin_count = 0;
      last_chd_bin_count = 0;

      pps_bins.adev_type = 0;
      pps_bins.bin_count = 0;
      pps_bins.adev_min = 0.0;
      pps_bins.adev_max = 0.0;

      osc_bins.adev_type = 0;
      osc_bins.bin_count = 0;
      osc_bins.adev_min = 0.0;
      osc_bins.adev_max = 0.0;

      chc_bins.adev_type = 0;
      chc_bins.bin_count = 0;
      chc_bins.adev_min = 0.0;
      chc_bins.adev_max = 0.0;

      chd_bins.adev_type = 0;
      chd_bins.bin_count = 0;
      chd_bins.adev_min = 0.0;
      chd_bins.adev_max = 0.0;

      for(i=0; i<MAX_ADEV_BINS; i++) {
         pps_bins.adev_on[i] = 0;
         pps_bins.adev_taus[i] = 0.0;
         pps_bins.adev_bins[i] = 0.0;

         osc_bins.adev_on[i] = 0;
         osc_bins.adev_taus[i] = 0.0;
         osc_bins.adev_bins[i] = 0.0;

         chc_bins.adev_on[i] = 0;
         chc_bins.adev_taus[i] = 0.0;
         chc_bins.adev_bins[i] = 0.0;

         chd_bins.adev_on[i] = 0;
         chd_bins.adev_taus[i] = 0.0;
         chd_bins.adev_bins[i] = 0.0;
      }

      global_adev_max = (-(BIG_NUM));
      global_adev_min = (BIG_NUM);

      pps_base_value = 0.0;
      osc_base_value = 0.0;
      chc_base_value = 0.0;
      chd_base_value = 0.0;

      have_pps_base = 0;
      have_osc_base = 0;
      have_chc_base = 0;
      have_chd_base = 0;

      cha_ts_ref = chb_ts_ref = chc_ts_ref = chd_ts_ref = 0.0;
      cha_ts_sum = chb_ts_sum = chc_ts_sum = chd_ts_sum = 0.0;

      pps_phase = osc_phase = cha_phase = chb_phase = chc_phase = chd_phase = 0.0;
      pps_count = osc_count = cha_count = chb_count = chc_count = chd_count = 0.0;
      if(dont_reset_phase == 0) {  // reloaing adev queue from plot queue data
         last_cha_ts = last_chb_ts = last_chc_ts = last_chd_ts = 0.0;
         last_ticc_v1 = last_ticc_v2 = last_ticc_v3 = last_ticc_v4 = 0.0;
      }
      if((dont_reset_phase == 0) || (dont_reset_phase == 2)) {
         cha_phase_wrap_adj = 0.0;
         cha_last_wrap_phase = 0.0;
         chb_phase_wrap_adj = 0.0;
         chb_last_wrap_phase = 0.0;
         chc_phase_wrap_adj = 0.0;
         chc_last_wrap_phase = 0.0;
         chd_phase_wrap_adj = 0.0;
         chd_last_wrap_phase = 0.0;
      }

      cha_time2_sum = chb_time2_sum = 0.0;
      cha_time2_count = chb_time2_count = 0.0;
      msgofs_phase = msgjit_phase = 0.0;


      fudge_sum = fudge_count = 0.0;

      first_show_bin = 0;
      max_adev_width = 0;
      max_bins_shown = 0;

      reset_adev_bins();
      pps_adevs_cleared = 1;
      osc_adevs_cleared = 1;
      chc_adevs_cleared = 1;
      chd_adevs_cleared = 1;

      pps_tie = 0.0;           // Time Interval Errors
      osc_tie = 0.0;
      chc_tie = 0.0;
      chd_tie = 0.0;

      pps_resid = 0.0;         // phase residuals
      osc_resid = 0.0;
      chc_resid = 0.0;
      chd_resid = 0.0;
      last_pps_resid = 0.0;
      last_osc_resid = 0.0;
      last_chc_resid = 0.0;
      last_chd_resid = 0.0;

      for(k=0; k<NUM_PLOTS+DERIVED_PLOTS; k++) {  // reset plot queue residuals
         resid[k].sum_x        = (DATA_SIZE) 0.0;
         resid[k].sum_y        = (DATA_SIZE) 0.0;
         resid[k].sum_xx       = (DATA_SIZE) 0.0;
         resid[k].sum_yy       = (DATA_SIZE) 0.0;
         resid[k].sum_xy       = (DATA_SIZE) 0.0;
         resid[k].a0           = (DATA_SIZE) 0.0;
         resid[k].a1           = (DATA_SIZE) 0.0;
         resid[k].resid_count  = (DATA_SIZE) 0.0;
      }

      if(rcvr_type == TICC_RCVR) {  // aaaacccc
////     dac_voltage = 0.0;
         temperature = 0.0;
         have_osc_offset = 0;
         have_pps_offset = 0;
         have_chc_offset = 0;
         have_chd_offset = 0;
      }
///if(dont_reset_queues == 0) ticc_packets = 0;
      if(screen_active()) {
         force_adev_redraw(666);
         need_redraw = 666;
      }
   }

   if(queue_type & RESET_MTIE_Q) {
      if(TICC_USED || mtie_allocated) alloc_mtie(1);  // reset MTIE data structures by re-allocating them
   }
#endif

   if(queue_type & RESET_PLOT_Q) {  // reset plot queue
      plot_q_in = plot_q_out = 0;
      plot_q_count = 0;
      plot_q_full = 0;
      plot_time = 0;
      plot_start = 0;
      plot_column = 0;
      stat_count = 0.0F;
      clear_plot_entry((long) plot_q_in);

      min_q_lat = 90.0;
      max_q_lat = (-90.0);
      min_q_lon = 180.0;
      max_q_lon = (-180.0);
      have_valid_lla = 0;
      last_lla_span = 1.0;

      if(sim_file == 0) plot_title[0] = 0;  // reset plot title
      title_type = NONE;
      greet_ok = 1;

      reset_marks();
      rebuild_lla_plot(0);
      if(dont_reset_queues == 0) {
         ticc_packets = 0;
      }
   }
}

void new_queue(int queue_type, int why)
{
   // flush the queue contents and start up a new data capture
//sprintf(debug_text, "new queue:%04x  why:%d", queue_type, why);
   reset_queues(queue_type, why);
   log_loaded = 0;
   if(need_view_auto && (view_all_data == 2)) ;
   else end_review(1);
   redraw_screen();
}

long find_event(long i, u16 flags)
{
struct PLOT_Q q;
long count;

   // locate the next holdover event or time sequence error in the plot queue
   if(i < 0) return (-1);  // should never happen

   count = 0;
   while(i >= plot_q_size) i -= plot_q_size;  // !!! perhaps we should return (-1)

   while(i != plot_q_in) {  // skip over leading queue entries that match the flags
      q = get_plot_q(i);
      if((q.sat_flags & flags) == 0) break;
      ++i;
      if(i >= plot_q_size) i = 0;       // wrap the queue
      if(++count > plot_q_count) break; // prevent run-away loop
   }
   if(i == plot_q_in) {  // event not found
      return i;
   }

   count = 0;
   while(i != plot_q_in) {  // find the next occurance of the event
      q = get_plot_q(i);
      if((q.sat_flags & flags) != 0) break;
      ++i;
      if(i >= plot_q_size) i = 0;
      if(++count > plot_q_count) break;
   }

   return i;
}

int go_count;

long goto_event(u16 flags)
{
long val;

   // move plot to the next holdover event or time sequence error
++go_count;

   val = find_event(last_mouse_q, flags);
   if(val < 0) return val;

//   if(1 || !mouse_plot_valid) ; // mouse is out of the plot area
//   else if(plot_mag) val -= ((last_plot_mouse_x * view_interval) / (long) plot_mag); // center point on mouse

   if(val > plot_q_count) val = plot_q_count - 1;
   if(val < 0) val = 0;

   if(val != last_mouse_q) last_plot_glitch = val; // mouse was moved
   last_mouse_q = val;
   last_plot_glitch = last_mouse_q;
   last_plot_mouse_q = val;
   if(1 || !mouse_plot_valid) {  // mouse is out of the plot area
      if(plot_mag) val -= ((last_plot_mouse_x * view_interval) / (long) plot_mag); // center point on mouse
   }
   zoom_review(val, REVIEW_BEEP);

   return val;
}


long next_q_point(long i, int stop_flag)
{
long j;
int wrap;

   // locate the next data point in the plot queue based upon the display
   // view interval

   plot_column += plot_mag;
   if((stop_flag == STOP_AT_PLOT_END) && (plot_column >= PLOT_WIDTH)) {  // we are at end of plot area
      plot_column -= plot_mag;
      return (-2L);
   }
   wrap = 0;

   if(slow_q_skips) { //!!! ineffcient way to step over multiple queue entries,  but bug resistant
      j = view_interval;
      while(j--) {
         if(++i == plot_q_in) return (-1L);
         while(i >= plot_q_size) i -= plot_q_size;
      }
   }
   else {  // more efficent way to do it,  but there might be bugs...
      if(i <= plot_q_in) {
         i += view_interval;
         if(i >= plot_q_in) return (-1L);
      }
      else if(i > plot_q_in) {
         i += view_interval;
         if(i <= plot_q_in) return (-1L);
      }
      while(i >= plot_q_size) {
         i -= plot_q_size;
         ++wrap;
      }
   }
   if(1 && wrap && (i >= plot_q_out)) return (-1L);
   if(i == plot_q_out) return (-1L);

   return i;
}

long disp_filter_size()
{
DATA_SIZE show_time;

   // return the length of the display averaging filter

   if(filter_count < 0) {   // filter size based up display config
      show_time = (DATA_SIZE) (view_interval*queue_interval*PLOT_WIDTH);  // seconds per screen
show_time *= (DATA_SIZE) (0-filter_count);
      show_time /= plot_mag;
      show_time /= nav_rate;
      show_time /= PLOT_WIDTH;
      show_time += (DATA_SIZE) 0.5;
      if(show_time < 1.0) show_time = (DATA_SIZE) 1.0;
      if(show_time > PLOT_WIDTH) show_time = (DATA_SIZE) PLOT_WIDTH;

      return (long) show_time;
   }
   else {  // fixed filter size
      if(filter_count > PLOT_WIDTH) return PLOT_WIDTH;
      return filter_count;
   }
}

double filter_jd;   // jd at the plot queue point
u16 filter_flags;   // logical or of all sat_flags within the filter interval


struct PLOT_Q back_filter(long point)
{
struct PLOT_Q avg;
struct PLOT_Q q;
DATA_SIZE count;
DATA_SIZE max[NUM_PLOTS+DERIVED_PLOTS];
DATA_SIZE min[NUM_PLOTS+DERIVED_PLOTS];
long i;
int j;

   // average the previous "filter_count" queue entries

   if(plot_q_count < filter_count) i = plot_q_count;
   else                            i = disp_filter_size();

   point -= i;
   if(point < 0) point += plot_q_size;
//   if(point >= plot_q_count) point = 0;

   avg = get_plot_q(point);
   filter_jd = avg.q_jd;
   filter_flags = avg.sat_flags;
   count = 1.0F;

   if(disp_filter_type == 'P') {  // min/max filter
      for(j=0; j<NUM_PLOTS+DERIVED_PLOTS; j++) {
         if(j != FFT) {
            max[j] = min[j] = avg.data[j];
         }
      }
   }

   while(--i > 0) {
      ++point;
      while(point >= plot_q_size) point -= plot_q_size;

      q = get_plot_q(point);
      filter_jd = q.q_jd;
      filter_flags |= q.sat_flags;
      count += (DATA_SIZE) 1.0;

      for(j=0; j<NUM_PLOTS+DERIVED_PLOTS; j++) {
         if(j != FFT) {
            if(disp_filter_type == 'P') {
               if(q.data[j] > max[j]) max[j] = q.data[j];
               if(q.data[j] < min[j]) min[j] = q.data[j];
            }

            avg.data[j] += q.data[j];
         }
      }
   }

   for(j=0; j<NUM_PLOTS+DERIVED_PLOTS; j++) {  // derived_plots OK
      if(j != FFT) {
         if(disp_filter_type == 'P') {
            if(fabs(max[j]) >= fabs(min[j])) avg.data[j] = max[j];
            else                             avg.data[j] = min[j];
         }
         else {
            avg.data[j] /= count;
         }
      }
   }

   return avg;
}


struct PLOT_Q center_filter(long point)
{
struct PLOT_Q avg;
struct PLOT_Q q;
DATA_SIZE count;
DATA_SIZE max[NUM_PLOTS+DERIVED_PLOTS];
DATA_SIZE min[NUM_PLOTS+DERIVED_PLOTS];
long i;
long f_count;
int j;

   // average "filter_count / 2" queue entries each side of the specified queue entry

   f_count = disp_filter_size();

   avg = get_plot_q(point);
   filter_jd = avg.q_jd;
   filter_flags = avg.sat_flags;

   point = point - (f_count / 2);
   if(point < 0) point += plot_q_size;
//if(point >= plot_q_count) point = 0;

   avg = get_plot_q(point);
   filter_jd = avg.q_jd;
   filter_flags |= avg.sat_flags;
   count = 1.0F;

// if(point == plot_q_in) return avg;

   if(disp_filter_type == 'P') {  // min/max filter
      for(j=0; j<NUM_PLOTS+DERIVED_PLOTS; j++) {
         if(j != FFT) {
            max[j] = min[j] = avg.data[j];
         }
      }
   }

   for(i=1; i<f_count; i++) {
      if(++point == plot_q_in) {  // at the end of the plot data
         break;
      }
      while(point >= plot_q_size) point -= plot_q_size;

      q = get_plot_q(point);
      filter_flags |= q.sat_flags;
      count += (DATA_SIZE) 1.0;

      for(j=0; j<NUM_PLOTS+DERIVED_PLOTS; j++) {
         if(j != FFT) {
            if(disp_filter_type == 'P') {
               if(q.data[j] > max[j]) max[j] = q.data[j];
               if(q.data[j] < min[j]) min[j] = q.data[j];
            }

            avg.data[j] += q.data[j];
         }
      }
   }

   for(j=0; j<NUM_PLOTS+DERIVED_PLOTS; j++) {  // derived_plots OK
      if(j != FFT) {
         if(disp_filter_type == 'P') {
            if(fabs(max[j]) >= fabs(min[j])) avg.data[j] = max[j];
            else                             avg.data[j] = min[j];
         }
         else {
            avg.data[j] /= count;
         }
      }
   }

   return avg;
}


struct PLOT_Q forward_filter(long point)
{
struct PLOT_Q avg;
struct PLOT_Q q;
DATA_SIZE count;
DATA_SIZE max[NUM_PLOTS+DERIVED_PLOTS];
DATA_SIZE min[NUM_PLOTS+DERIVED_PLOTS];
long i;
long f_count;
int j;

   // average the next "filter_count" queue entries
   avg = get_plot_q(point);
   filter_jd = avg.q_jd;
   filter_flags = avg.sat_flags;
   count = 1.0F;

   if(point == plot_q_in) return avg;

   if(disp_filter_type == 'P') {  // min/max filter
      for(j=0; j<NUM_PLOTS+DERIVED_PLOTS; j++) {
         if(j != FFT) {
            max[j] = min[j] = avg.data[j];
         }
      }
   }

   f_count = disp_filter_size();
   for(i=1; i<f_count; i++) {
      if(++point == plot_q_in) {  // at the end of the plot data
         break;
      }
      while(point >= plot_q_size) point -= plot_q_size;

      q = get_plot_q(point);
      filter_flags |= q.sat_flags;
      count += (DATA_SIZE) 1.0;

      for(j=0; j<NUM_PLOTS+DERIVED_PLOTS; j++) {
         if(j != FFT) {
            if(disp_filter_type == 'P') {
               if(q.data[j] > max[j]) max[j] = q.data[j];
               if(q.data[j] < min[j]) min[j] = q.data[j];
            }

            avg.data[j] += q.data[j];
         }
      }
   }

   for(j=0; j<NUM_PLOTS+DERIVED_PLOTS; j++) {  // derived_plots OK
      if(j != FFT) {
         if(disp_filter_type == 'P') {
            if(fabs(max[j]) >= fabs(min[j])) avg.data[j] = max[j];
            else                             avg.data[j] = min[j];
         }
         else {
            avg.data[j] /= count;
         }
      }
   }

   return avg;
}


struct PLOT_Q filter_plot_q(long point)
{
long i;
long f_count;
int back_ok, center_ok, fwd_ok;

   if     (disp_filter_type == 'B') return back_filter(point);    // user forced filter type
   else if(disp_filter_type == 'C') return center_filter(point);
   else if(disp_filter_type == 'F') return forward_filter(point);

   f_count = disp_filter_size();

   back_ok = center_ok = fwd_ok = 1;

   i = point - f_count;
   if(i < 0) i += plot_q_size;
   if(i > plot_q_count) {  // too near the start of the plot to do a back filter
      back_ok = 0;
   }

   i = point - (f_count / 2);
   if(i < 0) i += plot_q_size;
   if(i > plot_q_count) {  // too near the start of the plot to do a center filter
      center_ok = 0;
   }

   i = point + f_count;
   if(i > plot_q_size) i -= plot_q_size;
   if(i > plot_q_count) {  // too near the end of the plot to do a forward filter
      fwd_ok = 0;
   }

   if(center_ok) {  // filter points each side of the point
      return center_filter(point);
   }
   else if(fwd_ok) {  // filter points after the point
      return forward_filter(point);
   }
   else if(back_ok) {  // filter points before the point
      return back_filter(point);
   }
   else {  // not enough data to do a proper filter
      return forward_filter(point);
   }
}


//
//
//   Screen format configuration stuff
//
//

void config_text()
{
int i;

   // text drawing stuff
   PLOT_TEXT_COL = (PLOT_COL / TEXT_WIDTH);
   PLOT_TEXT_ROW = (PLOT_ROW / TEXT_HEIGHT);

   MOUSE_COL = (PLOT_TEXT_COL)+2;
   MOUSE_ROW = ((PLOT_TEXT_ROW)-6-eofs);
   MOUSE_ROW -= ((TEXT_Y_MARGIN+TEXT_HEIGHT-1)/TEXT_HEIGHT);
   if((TEXT_HEIGHT == 16) && (SCREEN_HEIGHT <= SHORT_SCREEN)) --MOUSE_ROW;
   if(MOUSE_ROW < 0) MOUSE_ROW = 0;

   TEXT_COLS = ((SCREEN_WIDTH+TEXT_WIDTH-1)/TEXT_WIDTH);
   if(TEXT_COLS >= (int)sizeof(blanks)) TEXT_COLS = sizeof(blanks)-1;
   TEXT_ROWS = ((SCREEN_HEIGHT+TEXT_HEIGHT-1)/TEXT_HEIGHT);
   if(text_mode) MOUSE_ROW = TEXT_ROWS;

   EDIT_ROW = PLOT_TEXT_ROW+2;   // where to put the string input dialog
   if(SCREEN_HEIGHT <= TINY_HEIGHT) EDIT_ROW = PLOT_TEXT_ROW+1;  // piss3
   EDIT_COL = PLOT_TEXT_COL;

   if(text_mode) {
      EDIT_ROW = EDIT_COL = 0;
      PLOT_ROW += (TEXT_HEIGHT*3);
      PLOT_TEXT_ROW = 100;
   }

   for(i=0; i<TEXT_COLS; i++) blanks[i] = ' ';
   blanks[TEXT_COLS] = 0;
}

int scale_plot_row()
{
   // adjust plot row for scaled up text size
   if(use_vc_fonts) {
      PLOT_ROW = (PLOT_ROW * vc_font_scale) / 100;
if(vc_font_scale < 100) PLOT_ROW += TEXT_HEIGHT*2;  // piss
      config_text();
      vidstr(0,0, WHITE, " ");
   }
   return PLOT_ROW;
}

void config_undersized()
{
   // setup for 640x480 screen
   if(user_font_size > 12) user_font_size = 12;

   HORIZ_MAJOR = 30;
   HORIZ_MINOR = 5;
   VERT_MAJOR = 20;
   VERT_MINOR = 4;
   COUNT_SCALE = (VERT_MAJOR/2);

   if(rcvr_type == NO_RCVR) {  // size of the analog watch area
      AZEL_SIZE = LLA_SIZE = (SCREEN_WIDTH / 4); // piss (160);
   }

   if(user_set_plot_row == 0) {
      if(user_font_size && (user_font_size <= 8)) PLOT_ROW = SCREEN_HEIGHT-VERT_MAJOR*8;
      else PLOT_ROW = SCREEN_HEIGHT-VERT_MAJOR*6;
      scale_plot_row();
   }
}

void config_small()
{
   // setup for 800x600 screen
   if(user_set_plot_row == 0) {
      PLOT_ROW = (400+8);
      scale_plot_row();
   }

   FILTER_ROW = 17;
   FILTER_COL = INFO_COL;

   COUNT_SCALE = (VERT_MAJOR/2);

   AZEL_SIZE = LLA_SIZE = (160);       // size of the az/el map area

   if(TEXT_HEIGHT >= 16) eofs = 0;
}


void config_azel()
{
int i, j;

   // how and where to draw the azimuth/elevation map (and analog watch)
   // assume azel plot will be in the adev table area
   last_track = last_used = 0L;
   share_active = 0;
   shared_item = 0;
   left_adev_col = ADEV_COL;

   AZEL_ROW = ADEV_ROW*TEXT_HEIGHT;
   AZEL_COL = (ADEV_COL*TEXT_WIDTH);
   bogo_watch = 1;
   if(SCREEN_WIDTH >= ADEV_AZEL_THRESH) {  // adev tables and azel both fit
      if(adevs_active(1)) AZEL_COL += (TEXT_WIDTH*44);
      else                AZEL_COL += (TEXT_WIDTH*12);
      AZEL_ROW += TEXT_HEIGHT;
   }
   else if(SCREEN_WIDTH >= MEDIUM_WIDTH) {
      AZEL_ROW += TEXT_HEIGHT;
      AZEL_COL += (TEXT_WIDTH*4);
      if(SCREEN_HEIGHT <= SHORT_SCREEN) AZEL_COL += (TEXT_WIDTH*6);
   }
   else if((rcvr_type == NO_RCVR) && (SCREEN_WIDTH <= NARROW_SCREEN)) {
      AZEL_COL -= 160; // analog watch size
   }
   else {
      AZEL_COL += (TEXT_WIDTH*1);
      AZEL_ROW += TEXT_HEIGHT*2;
   }
   AZEL_SIZE = (SCREEN_WIDTH-AZEL_COL);

   if((SCREEN_WIDTH >= WIDE_WIDTH) && (SCREEN_WIDTH < 1400)) AZEL_SIZE -= (TEXT_WIDTH*2);
   else if(small_font && (SCREEN_WIDTH >= MEDIUM_WIDTH)) AZEL_COL += TEXT_WIDTH;

   if((AZEL_ROW+AZEL_SIZE) >= (PLOT_ROW-128)) {  // az/el map is in plot area
      AZEL_SIZE = (PLOT_ROW-AZEL_ROW-128);
   }
   if(SCREEN_WIDTH > NARROW_SCREEN) AZEL_MARGIN = 8;
   else                             AZEL_MARGIN = 4;

   if((zoom_screen) && (zoom_screen != 'K')) {
      if((SCREEN_WIDTH < TINY_TINY_WIDTH) || (SCREEN_WIDTH < SCREEN_HEIGHT)) {  //piss
         AZEL_SIZE = SCREEN_WIDTH-(SCREEN_WIDTH/20);
      }
      else {
         AZEL_SIZE = SCREEN_HEIGHT-(SCREEN_HEIGHT/20);
      }
      AZEL_SIZE = (AZEL_SIZE/TEXT_HEIGHT)*TEXT_HEIGHT;
      AZEL_SIZE -= (TEXT_HEIGHT*2)*2;

      AZEL_ROW = (SCREEN_HEIGHT-AZEL_SIZE)/2;
      AZEL_ROW = (AZEL_ROW/TEXT_HEIGHT)*TEXT_HEIGHT;
      if(SCREEN_HEIGHT <= SHORT_SCREEN) AZEL_ROW -= TEXT_HEIGHT;
      else                              AZEL_ROW -= (TEXT_HEIGHT*2);

      AZEL_COL = (SCREEN_WIDTH-AZEL_SIZE)/2;
      AZEL_COL = (AZEL_COL/TEXT_WIDTH)*TEXT_WIDTH;
      bogo_watch = 2;
   }

   // where to put the analog clock
   if(all_adevs && plot_lla && WIDE_SCREEN && ((zoom_screen == 0) || (zoom_screen == 'P'))) {
      WATCH_ROW = AZEL_ROW;   // we could be drawing the watch here
      WATCH_COL = (FILTER_COL+20+32+32)*TEXT_WIDTH;  //AZEL_COL;
      WATCH_SIZE = (SCREEN_WIDTH-WATCH_COL)/2;
      if(WATCH_SIZE > (PLOT_ROW-TEXT_HEIGHT*4)) WATCH_SIZE = PLOT_ROW-TEXT_HEIGHT*4;
   }
   else {
      WATCH_ROW = AZEL_ROW;   // we could be drawing the watch here
      WATCH_COL = AZEL_COL;
if((SCREEN_WIDTH >= MEDIUM_WIDTH) && (SCREEN_WIDTH < WIDE_WIDTH)) {
   WATCH_COL -= (TEXT_WIDTH*2);
   AZEL_COL -= (TEXT_WIDTH*2);
}
      WATCH_SIZE = AZEL_SIZE;
   }

   if(zoom_screen == 'P') ;
   else if(zoom_screen == 'K') ;
   else if(zoom_screen) return;

   if(all_adevs || shared_plot) {  // all_adevs or share az/el space with normal plot area
      if(all_adevs && WIDE_SCREEN && (plot_watch == 0) && (plot_lla == 0)) {
         AZEL_COL = SCREEN_WIDTH-AZEL_SIZE;   // and azel goes in the corner
      }
      else {
         AZEL_SIZE = PLOT_HEIGHT;     // default size of the az/el map area
if((PLOT_WIDTH/4) < AZEL_SIZE) AZEL_SIZE = (PLOT_WIDTH / 4);  // piss
         if(AZEL_SIZE > MAX_AZEL) {   // it's just too big to look at
            AZEL_SIZE = MAX_AZEL;
            AZEL_ROW = ((PLOT_ROW+TEXT_HEIGHT-1)/TEXT_HEIGHT)*TEXT_HEIGHT;
         }
         else {
            i = j = (SCREEN_HEIGHT-AZEL_SIZE);  // bottom justify
j = ((PLOT_ROW+TEXT_HEIGHT-1)/TEXT_HEIGHT)*TEXT_HEIGHT;  // toots - top justify
            AZEL_ROW = (i+j) / 2;  // center justify
         }
         AZEL_COL = (SCREEN_WIDTH-AZEL_SIZE);
if(0 && (bogo_watch == 1)) {
   WATCH_ROW = AZEL_ROW;
   WATCH_COL = AZEL_COL;
   WATCH_SIZE = AZEL_SIZE;               // watch goes in the adev area
   aclock_x = (WATCH_COL+ACLOCK_SIZE/2);   // and azel goes in the plot area
   aclock_y = (WATCH_ROW+ACLOCK_SIZE/2);
}
         bogo_watch = 3;
      }
      AZEL_MARGIN = 4;

      if((shared_plot == 0) && (PLOT_WIDTH < SCREEN_WIDTH)) {  // center plot area on the screen
         PLOT_COL = (SCREEN_WIDTH - PLOT_WIDTH) / 2;
      }
   }
   else {   // draw az/el map in the adev table area
      if(PLOT_WIDTH < SCREEN_WIDTH) {  // center plot area on the screen
         PLOT_COL = (SCREEN_WIDTH - PLOT_WIDTH) / 2;
      }
   }

// if(plot_azel || plot_signals || plot_watch) {
   if(zoom_screen == 'P') ;
   else if(plot_azel || plot_signals || plot_watch || plot_lla) {
      if((AZEL_ROW+AZEL_SIZE) >= PLOT_ROW) {  // az/el map is in the normal plot window
         PLOT_WIDTH -= AZEL_SIZE;             // make room for it...
         PLOT_WIDTH += (HORIZ_MAJOR-1);
         PLOT_WIDTH = (PLOT_WIDTH/HORIZ_MAJOR) * HORIZ_MAJOR;
         if((PLOT_COL+PLOT_WIDTH+AZEL_SIZE) > SCREEN_WIDTH) PLOT_WIDTH -= HORIZ_MAJOR;
         AZEL_COL -= ((AZEL_COL-(PLOT_COL+PLOT_WIDTH)) / 2);
         AZEL_COL = ((AZEL_COL+TEXT_WIDTH-1)/TEXT_WIDTH)*TEXT_WIDTH;
         share_active = 1;
      }
      else if((plot_azel || plot_signals) && plot_watch && (plot_lla == 0) && (all_adevs == SINGLE_ADEVS) && WIDE_SCREEN) {
         AZEL_SIZE = (SCREEN_WIDTH - AZEL_COL) / 2;
         if(AZEL_SIZE > MAX_AZEL) AZEL_SIZE = MAX_AZEL;
         WATCH_SIZE = AZEL_SIZE;
      }
      else if((plot_signals == 3) && (SCREEN_WIDTH >= WIDE_WIDTH)) {
         AZEL_COL -= TEXT_WIDTH*2;
      }
   }

}

void config_watch()
{
   // calculate where we can draw the watch and how big to make it
   ACLOCK_SIZE = AZEL_SIZE;
   aclock_x = (AZEL_COL+ACLOCK_SIZE/2);
   aclock_y = (AZEL_ROW+ACLOCK_SIZE/2);

   if(zoom_screen && (zoom_screen != 'K')) {
      if(map_and_watch) {
         aclock_x = AZEL_COL + AZEL_SIZE/2 - 2;
         aclock_y = AZEL_ROW + AZEL_SIZE/2 - 2;
      }
      else {
         aclock_x = SCREEN_WIDTH/2;
         aclock_y = SCREEN_HEIGHT/2;
      }
   }
   else if(all_adevs) {
      if(WIDE_SCREEN) {
         if(plot_lla) {
            goto watch_it;
         }
         else {
            ACLOCK_SIZE = (SCREEN_WIDTH-WATCH_COL);
            if(ACLOCK_SIZE > (PLOT_ROW-TEXT_HEIGHT*4)) {
               ACLOCK_SIZE = PLOT_ROW-TEXT_HEIGHT*4;
            }
            if(ACLOCK_SIZE > MAX_AZEL) ACLOCK_SIZE = MAX_AZEL;
            aclock_x = (SCREEN_WIDTH-ACLOCK_SIZE/2-TEXT_WIDTH*2);   // and azel goes in the plot area
            aclock_y = (0+ACLOCK_SIZE/2+TEXT_HEIGHT*2);
         }
      }
   }
   else if((plot_azel || plot_signals) && plot_lla && WIDE_SCREEN) {
      goto watch_it;
   }
   else if((rcvr_type == NO_RCVR) && (SCREEN_WIDTH < NARROW_SCREEN)) {
      goto watch_it;
   }
   else if((plot_azel || plot_signals) && (plot_lla == 0) && (SCREEN_HEIGHT >= SHORT_SCREEN)) {  // both azel and watch want to be on the sceen
      watch_it:
      ACLOCK_SIZE = WATCH_SIZE;               // watch goes in the adev area
      aclock_x = (WATCH_COL+ACLOCK_SIZE/2);   // and azel goes in the plot area
      aclock_y = (WATCH_ROW+ACLOCK_SIZE/2);
   }
}


void config_lla()
{
int lla_margin;
int adev_flag;

   // how and where to draw the lat/lon/altitude map

   adev_flag = 1;
   if     (pps_adev_period > 0.0) ;        // aaaapppp
   else if(osc_adev_period > 0.0) ;
   else if(chc_adev_period > 0.0) ;
   else if(chd_adev_period > 0.0) ;
   else if(adev_period <= 0.0) adev_flag = 0;

   LLA_ROW  = (ADEV_ROW*TEXT_HEIGHT);  // on small screens the lla map takes the place of the adev tables
   LLA_COL  = (ADEV_COL*TEXT_WIDTH);
   lla_margin = 1;
   if(zoom_screen == 'L') {
      if(reading_lla) lla_margin = 20;
      LLA_ROW = lla_margin;
      LLA_COL = lla_margin;
   }
   else if(WIDE_SCREEN) {
      if(all_adevs) {
         if(plot_lla) LLA_COL = WATCH_COL + WATCH_SIZE;
         else         LLA_COL = SCREEN_WIDTH - AZEL_SIZE - (TEXT_WIDTH*2);
      }
      else if(shared_plot) {
         if(plot_watch && (plot_azel || plot_signals)) LLA_COL = WATCH_COL + WATCH_SIZE + (TEXT_WIDTH*2);
         else                                          LLA_COL = SCREEN_WIDTH - AZEL_SIZE - (TEXT_WIDTH*2);
      }
      else {
         LLA_COL = AZEL_COL + AZEL_SIZE + (TEXT_WIDTH*2);
      }
   }
   else if(all_adevs) {
      LLA_ROW = AZEL_ROW;
      LLA_COL = AZEL_COL;
   }
   else if(SCREEN_WIDTH > BIG_WIDTH) {  // both lla map and azel map will fit on big screens
      if(shared_plot) LLA_COL = SCREEN_WIDTH - AZEL_SIZE - (TEXT_WIDTH*2);
      else            LLA_COL = AZEL_COL + AZEL_SIZE + (TEXT_WIDTH*2);
   }
// else if(SCREEN_WIDTH >= WIDE_WIDTH) {
   else if((SCREEN_WIDTH >= WIDE_WIDTH) && adev_flag) {
      if(adevs_active(1)) LLA_COL += (TEXT_WIDTH*44); // adevs and lla will fit
      else                LLA_COL += (TEXT_WIDTH*12);
   }
   else if(SCREEN_WIDTH >= MEDIUM_WIDTH) {
      LLA_COL += (4*TEXT_WIDTH);
   }
   else if(SCREEN_WIDTH >= NARROW_SCREEN) {
      LLA_COL += (2*TEXT_WIDTH);
   }
   else if(0 && all_adevs) {
      LLA_COL = AZEL_COL + AZEL_SIZE + (TEXT_WIDTH*2);
   }
   else {  // undersized screens
      LLA_ROW = AZEL_ROW;
      LLA_COL = AZEL_COL;
   }

   if(zoom_screen == 'L') {
      if((SCREEN_WIDTH < TINY_TINY_WIDTH) || (SCREEN_WIDTH < SCREEN_HEIGHT)) {  //piss
         LLA_SIZE = (SCREEN_WIDTH-LLA_ROW-TEXT_HEIGHT*4-lla_margin);
      }
      else {
         LLA_SIZE = (SCREEN_HEIGHT-LLA_ROW-TEXT_HEIGHT*4-lla_margin);
      }
   }
   else {
      LLA_SIZE = (SCREEN_WIDTH-LLA_COL);
      if(SCREEN_WIDTH >= NARROW_SCREEN) { // lla map maight be in the plot area
         if(all_adevs) ;
         else if((LLA_ROW+LLA_SIZE) >= (PLOT_ROW-128)) LLA_SIZE = (PLOT_ROW-LLA_ROW-128);
      }
   }
//sprintf(debug_text3, "share:%d  LLA: %d,%d,%d  XY=%d,%d  AZEL:%d,%d %d  WSIZE:%d  ADEV:%d,%d  SCRW:%d",
//shared_plot, LLA_COL,LLA_ROW,LLA_SIZE,  LLA_X,LLA_Y, AZEL_COL,AZEL_ROW,AZEL_SIZE, WATCH_SIZE, ADEV_COL,ADEV_ROW, SCREEN_WIDTH);

   if(LLA_SIZE > MAX_LLA_SIZE) LLA_SIZE = MAX_LLA_SIZE;
   LLA_MARGIN = 10;              // border width in pixels
   LLA_SIZE -= (LLA_MARGIN*2);   // tweek size so it matches the grid
   LLA_SIZE /= LLA_DIVISIONS;    // round size to a multiple of the grid size
   LLA_SIZE *= LLA_DIVISIONS;
   lla_step = LLA_SIZE / LLA_DIVISIONS;  // pixels per grid division
   lla_width = lla_step * LLA_DIVISIONS;
   if(lla_width > MAX_LLA_SIZE) lla_width = MAX_LLA_SIZE;
   LLA_SIZE += (LLA_MARGIN*2);   // add a margin around the lla circles
   if(LLA_SIZE > MAX_LLA_SIZE) LLA_SIZE = MAX_LLA_SIZE;
   if(zoom_screen == 'L') {
      LLA_COL = (SCREEN_WIDTH - LLA_SIZE) / 2;
   }
}

void config_zoom()
{
   if(zoom_screen == 0) return;
   if(plot_signals) return;
   if(plot_watch) return;
   if(plot_digital_clock) return;
   if(plot_azel) return;
   if(plot_lla && (zoom_screen == 'L')) return;

   if(text_mode) cancel_zoom(3);
   if(zoom_screen == 'L') cancel_zoom(4);
//zzzzzzz   zoom_lla = 0;
}

void config_res()
{
   // setup the srceen size values based upon the screen type letter

   if(screen_type == 'd') {  // very small horizontal LCD (full screen mode)
      SCREEN_WIDTH = 480;
      SCREEN_HEIGHT = 320;
      use_vc_fonts = 1;
      config_undersized();
      if(0 && (user_set_font_scale == 0)) {  // piss
         if(plot_digital_clock) vc_font_scale = 60;
         else                   vc_font_scale = 75;
      }

      if(user_set_plot_row == 0) {
         if(rotate_screen) PLOT_ROW = 240; // piss
         else PLOT_ROW = 240;
      }

      if(day_plot >= 24) {
         HORIZ_MAJOR = 24;
         HORIZ_MINOR = 4;
      }
      else if(day_plot >= 12) {
         HORIZ_MAJOR = 48;
         HORIZ_MINOR = 8;
      }
      else day_plot = 0;
      #ifdef USE_X11
         if(user_set_full_screen == 0) {
            kill_deco = 1;
            go_fullscreen = 1;
         }
         if(user_set_crap_mouse == 0) {
            crap_mouse = 1;
         }
      #endif

      if(user_set_sat_cols == 0) {  // 8 col short sat info display
         max_sat_display = 8;
         user_set_short = 1;
         sat_cols = 1;
         ms_row = ms_col = 0;

         max_sats = max_sat_display;  // used to format the sat_info data
         max_sat_count = max_sat_display;
         temp_sats = max_sat_display;
         config_sat_rows();
      }
   }
   else if(screen_type == 'e') {  // very small vertical LCD (full screen mode)
      SCREEN_WIDTH = 320;
      SCREEN_HEIGHT = 480;
      use_vc_fonts = 1;

      config_undersized();

      if(day_plot >= 24) {
         HORIZ_MAJOR = 24;
         HORIZ_MINOR = 4;
      }
      else if(day_plot >= 12) {
         HORIZ_MAJOR = 48;
         HORIZ_MINOR = 8;
      }
      else day_plot = 0;
      #ifdef USE_X11
         if(user_set_full_screen == 0) {
            kill_deco = 1;
            go_fullscreen = 1;
         }
         if(user_set_crap_mouse == 0) {
            crap_mouse = 1;
         }
      #endif
   }
   else if(screen_type == 'p') {  // Raspberry PI 800x480 LCD (full screen mode)
      custom_width = 800;
      custom_height = 480;
      #ifdef USE_X11
         if(user_set_full_screen == 0) {
            kill_deco = 1;
            go_fullscreen = 1;
         }
         if(user_set_crap_mouse == 0) {
            crap_mouse = 1;
         }
      #endif
      goto customize;
   }
   else if(screen_type == 'r') {  // Reduced 1024x600 LCD (full screen mode)
      custom_width = 1024;
      custom_height = 600;
      #ifdef USE_X11
         if(user_set_full_screen == 0) {
            kill_deco = 1;
            go_fullscreen = 1;
         }
         if(user_set_crap_mouse == 0) {
            crap_mouse = 1;
         }
      #endif
      goto customize;
   }
   else if(screen_type == 'j') {   // 1280x800 LCD (full screen mode)
      custom_width = 1280;
      custom_height = 800;
      #ifdef USE_X11
         if(user_set_full_screen == 0) {
            kill_deco = 1;
            go_fullscreen = 1;
         }
         if(user_set_crap_mouse == 0) {
            crap_mouse = 1;
         }
      #endif
      goto customize;
   }

   else if(screen_type == 'c') { // custom screen screen sizw
      customize:
      screen_type = 'c';
      if(custom_width <= 0) custom_width = SCREEN_WIDTH;
      if(custom_height <= 0) custom_height = SCREEN_HEIGHT;

      if(custom_width <= MIN_WIDTH) custom_width = MIN_WIDTH;
      if(custom_width >= MAX_WIDTH) custom_width = MAX_WIDTH;
      if(custom_height <= MIN_HEIGHT) custom_height = MIN_HEIGHT;

      SCREEN_WIDTH = custom_width;
      SCREEN_HEIGHT = custom_height;

      if(user_set_plot_row == 0) {
         if     (SCREEN_HEIGHT > 1050) { PLOT_ROW = 576;  scale_plot_row(); }
         else if(SCREEN_HEIGHT > 960)  { PLOT_ROW = PLOT_TOP+(2*VERT_MAJOR);  scale_plot_row(); }
         else if(SCREEN_HEIGHT >= MEDIUM_HEIGHT) { PLOT_ROW = 468;  scale_plot_row(); }
         else if(SCREEN_HEIGHT >= SHORT_SCREEN) config_small();
         else                                   config_undersized();
      }

      if(day_plot) {
         HORIZ_MAJOR = SCREEN_WIDTH / day_plot;
         HORIZ_MAJOR /= 4;
         HORIZ_MAJOR *= 4;
         HORIZ_MINOR = (HORIZ_MAJOR / 4);
      }
      else day_plot = 0;
////  day_plot = 0;
   }
   else if(screen_type == 'h') { // huge screen
      SCREEN_WIDTH = 1920;
      SCREEN_HEIGHT = 1080;

      if(user_set_plot_row == 0) {
         PLOT_ROW = 576;
         scale_plot_row();
      }

      if(day_plot >= 24) {
         HORIZ_MAJOR = 75;
         HORIZ_MINOR = 15;
      }
      else if(day_plot >= 16) {
         HORIZ_MAJOR = 150;
         HORIZ_MINOR = 30;
      }
      else day_plot = 0;
   }
   else if(screen_type == 'k') {
      custom_width = 1280;
      custom_height = 960;
      goto customize;
   }
   else if(screen_type == 'l') { // large screen
      SCREEN_WIDTH = 1280;
      SCREEN_HEIGHT = 1024; // 800, 900, 960, 1024

      if(user_set_plot_row == 0) {
         PLOT_ROW = (PLOT_TOP+VERT_MAJOR*2);
         scale_plot_row();
      }

      if(day_plot >= 24) {
         HORIZ_MAJOR = 48;
         HORIZ_MINOR = 12;
      }
      else if(day_plot >= 12) {
         HORIZ_MAJOR = 96;
         HORIZ_MINOR = 24;
      }
      else day_plot = 0;
   }
   else if(screen_type == 'n') {  // netbook screen
      custom_width = 1000;
      custom_height = 540;
      goto customize;
   }
   else if(screen_type == 's') {  // small screen
      SCREEN_WIDTH = 800;
      SCREEN_HEIGHT = 600;

      config_small();

      if(day_plot >= 24) {
         HORIZ_MAJOR = 30;
         HORIZ_MINOR = 5;
      }
      else if(day_plot >= 12) {
         HORIZ_MAJOR = 60;
         HORIZ_MINOR = 15;
      }
      else day_plot = 0;
   }
   else if(screen_type == 't') {  // text only mode
      // In DOS we do this mode as a true text mode
      // In Windows and X11, this is actually a 640x480 graphics mode
      SCREEN_WIDTH = 640;
      SCREEN_HEIGHT = 480;
      if(user_set_plot_row == 0) {
         PLOT_ROW = (SCREEN_HEIGHT/TEXT_HEIGHT)*TEXT_HEIGHT;  // -VERT_MAJOR*2;
         scale_plot_row();
      }
      PLOT_COL = 0;
      day_plot = 0;
   }
   else if(screen_type == 'u') {  // undersized, very small screen
      SCREEN_WIDTH = 640;
      SCREEN_HEIGHT = 480;

      config_undersized();

      if(day_plot >= 24) {
         HORIZ_MAJOR = 24;
         HORIZ_MINOR = 4;
      }
      else if(day_plot >= 12) {
         HORIZ_MAJOR = 48;
         HORIZ_MINOR = 8;
      }
      else day_plot = 0;
   }
   else if(screen_type == 'v') { // very large screen
      SCREEN_WIDTH = 1400;  //1440;
      SCREEN_HEIGHT = 1050; // 900;

      if(user_set_plot_row == 0) {
         PLOT_ROW = (PLOT_TOP+VERT_MAJOR*2);
         scale_plot_row();
      }

      if(day_plot >= 24) {
         HORIZ_MAJOR = 55;
         HORIZ_MINOR = 11;
      }
      else if(day_plot >= 12) {
         HORIZ_MAJOR = 110;
         HORIZ_MINOR = 22;
      }
      else day_plot = 0;
   }
   else if(screen_type == 'x') { // extra large screen
      SCREEN_WIDTH = 1680;
      SCREEN_HEIGHT = 1050;

      if(user_set_plot_row == 0) {
         PLOT_ROW = (PLOT_TOP+VERT_MAJOR*2);
         scale_plot_row();
      }

      if(day_plot >= 24) {
         HORIZ_MAJOR = 60;
         HORIZ_MINOR = 12;
      }
      else if(day_plot >= 12) {
         HORIZ_MAJOR = 120;
         HORIZ_MINOR = 24;
      }
      else day_plot = 0;
   }
   else if(screen_type == 'z') { // really huge screen
      SCREEN_WIDTH = 2048;
      SCREEN_HEIGHT = 1536;

      if(user_set_plot_row == 0) {
         PLOT_ROW = (576);
         scale_plot_row();
      }

      if(day_plot >= 24) {
         HORIZ_MAJOR = 80;
         HORIZ_MINOR = 16;
      }
      else if(day_plot >= 16) {
         HORIZ_MAJOR = 160;
         HORIZ_MINOR = 32;
      }
      else day_plot = 0;
   }
   else {   // 'm' medium/normal (1024x768) screen
      SCREEN_WIDTH = 1024;
      SCREEN_HEIGHT = 768;

      if(user_set_plot_row == 0) {
         PLOT_ROW = 468;
         scale_plot_row();
      }
      PLOT_COL = 0;

      if(day_plot >= 24) {
         HORIZ_MAJOR = 40;
         HORIZ_MINOR = 10;
      }
      else if(day_plot >= 12) {
         HORIZ_MAJOR = 80;
         HORIZ_MINOR = 20;
      }
      else day_plot = 0;
   }
}

void config_screen(int why)
{
int i;

   // setup variables related to the video screen size
   no_x_margin = no_y_margin = 0;

   VERT_MAJOR = 30;    // the graph axes tick mark spacing (in pixels)
   VERT_MINOR  = 6;    // VERT_MAJOR/VERT_MINOR should be 5

   HORIZ_MAJOR = 60;
   HORIZ_MINOR = 10;

   INFO_COL = 65;
   if(text_mode) INFO_COL -= 1;
   else if(SCREEN_WIDTH <= 480) {
      INFO_COL -= 3;  // piss
   }
   else if(SCREEN_WIDTH <= 640) INFO_COL -= 2;
   else if(1 && (SCREEN_HEIGHT <= TINY_HEIGHT) && (SCREEN_WIDTH > NARROW_SCREEN-20) && (SCREEN_WIDTH <= NARROW_SCREEN+X11_MARGIN)) {
      INFO_COL -= 2;  // kludge for small touch screens (800x480)
   }
   else if(SCREEN_WIDTH <= NARROW_SCREEN) INFO_COL -= 1;
   else if(small_font == 1) INFO_COL -= 1;

   FILTER_ROW = 18;
   FILTER_COL = INFO_COL;

   AZEL_SIZE = LLA_SIZE = (MAX_AZEL); // size of the az/el map area
   eofs = 1;

   PLOT_COL = 0;

   config_res();   // set the height/width, etc.
   SWAP(SCREEN_WIDTH,SCREEN_HEIGHT);

//sprintf(plot_title, "PLOT_ROW:%d  VM:%d  texth:%d", PLOT_ROW, VERT_MAJOR, TEXT_HEIGHT);
   if(zoom_screen == 'P') {
      if(user_set_plot_row == 0) {
         if((SCREEN_WIDTH < TINY_TINY_WIDTH) || (SCREEN_WIDTH < SCREEN_HEIGHT)) {  //piss
            PLOT_ROW = TEXT_HEIGHT*12;
         }
         else {
            PLOT_ROW = TEXT_HEIGHT*8;
         }
         scale_plot_row();
      }
   }
   else if(1 && luxor) {
//      PLOT_ROW = SCREEN_HEIGHT / 2;
//      scale_plot_row();
//if(PLOT_ROW > TEXT_HEIGHT*26) PLOT_ROW = TEXT_HEIGHT*26;
//      PLOT_ROW /= TEXT_HEIGHT;
//      PLOT_ROW *= TEXT_HEIGHT;
      if(user_set_plot_row == 0) {
         if(plot_digital_clock == 0) {
            PLOT_ROW -= VERT_MAJOR*2;
            scale_plot_row();
         }
      }
   }
   else if((all_adevs == SINGLE_ADEVS) && (SCREEN_HEIGHT > SHORT_SCREEN) && (max_sats > 8) && plot_digital_clock) {
      i = (max_sats-8) * TEXT_HEIGHT;
      if(sat_cols > 1) i = (sat_rows-8) * TEXT_HEIGHT;
      i += ((VERT_MAJOR*2) - 1);
      i /= (VERT_MAJOR*2);  // makes graph prettier to have things a multiple of VERT_MAJOR*2
      i *= (VERT_MAJOR*2);

      if(user_set_plot_row == 0) {
         PLOT_ROW = PLOT_ROW + i;
         scale_plot_row();
      }
   }

   if(small_sat_count) {  // compress sat count to VERT_MINOR ticks per sat
      COUNT_SCALE = VERT_MINOR;
   }
   else {                 // expand sat count to VERT_MAJOR ticks per sat
      COUNT_SCALE = VERT_MAJOR;
   }

   if(no_plots) {
      if(user_set_plot_row == 0) {
         PLOT_ROW = SCREEN_HEIGHT;
      }
//    scale_plot_row();
   }

   // graphs look best if PLOT_HEIGHT is a multiple of (2*VERT_MAJOR)
   // but small screens don't have room to waste
   PLOT_HEIGHT = (SCREEN_HEIGHT-PLOT_ROW);
   PLOT_HEIGHT /= (VERT_MAJOR*2);  // makes graph prettier to have things a multiple of VERT_MAJOR*2
   PLOT_HEIGHT *= (VERT_MAJOR*2);

   PLOT_CENTER = (PLOT_HEIGHT/2-1);

if(1 && ((PLOT_ROW+PLOT_HEIGHT) < SCREEN_HEIGHT)) {  // shift plot down to bottom of the screen
   PLOT_ROW += (SCREEN_HEIGHT - (PLOT_ROW + PLOT_HEIGHT) - 0);
}

   PLOT_WIDTH = (SCREEN_WIDTH/HORIZ_MAJOR) * HORIZ_MAJOR;
   if(day_plot) {
      PLOT_WIDTH = HORIZ_MAJOR * day_plot;
      if(interval_set) {
         queue_interval = (60L*60L) / HORIZ_MAJOR;
         interval_set = 0;
      }
   }

   // when graph fills, scroll it left this many pixels
   if(continuous_scroll) PLOT_SCROLL = 1;  // live update mode for fast processors
   else                  PLOT_SCROLL = (HORIZ_MAJOR*2);

   if(rcvr_type == NO_RCVR) ;
   else if(no_plots) ;
   else if(0 && SMALL_SCREEN) {  // undersized screen
      if     (plot_azel)     shared_plot = 1;
      else if(plot_signals)  shared_plot = 1;
      else if(plot_watch)    shared_plot = 1;
      else if(plot_lla)      shared_plot = 1;
      else                   shared_plot = 0;
   }

   if(fix_mode && plot_lla && (plot_azel || plot_signals) && (WIDE_SCREEN == 0)) {
      shared_plot = 1;
   }
   if(all_adevs && plot_lla) {
      shared_plot = 1;
   }

   config_zoom();  // see if Z zoom command can be used
   config_azel();  // setup for drawing azel map (and analog watch)
   config_lla();   // setup for drawing lat/lon/alt plot
   config_text();  // setup for drawing text
   config_watch(); // setup for drawing the analog watch

// sprintf(plot_title, "share:%d azel:%d,%d  lla:%d,%d  watch:%d,%d  adev:%d,%d", share_active, AZEL_ROW,AZEL_COL, LLA_ROW,LLA_COL, WATCH_ROW,WATCH_COL, ADEV_ROW,ADEV_COL);

   if(PLOT_ROW >= (PLOT_TOP+2*VERT_MAJOR)) big_plot = 1;
   else                                    big_plot = 0;

   if(SCREEN_HEIGHT <= TINY_HEIGHT) {
      if((rcvr_type == SCPI_RCVR) || (rcvr_type == UCCM_RCVR) || (rcvr_type == TRUE_RCVR) || (rcvr_type == ZYFER_RCVR)) ;
      else if(have_ffom || have_tfom) ;
      else if(!user_set_dops) plot_dops = 0;
   }

   screen_configed = 1;

   if(no_redraw == 0) need_redraw = why;
   force_adev_redraw(1);
}


//
//
//   Data plotting stuff
//
//

DATA_SIZE scale_temp(double t)
{
double x;

   // convert degrees C into user specified measurement system
   x = (double) t;
   if     (DEG_SCALE == 'C') ;
   else if(DEG_SCALE == 'D') x = (100.0-x) * 1.5;
   else if(DEG_SCALE == 'E') x = (x * (21.0 / 40.0)) + 7.5;
   else if(DEG_SCALE == 'F') x = (x * 1.8) + 32.0;
   else if(DEG_SCALE == 'H') x = (x - 20.0) / 10.0;   // degrees Hilton (as in Paris Hilton)
   else if(DEG_SCALE == 'N') x = x * 0.33F;
   else if(DEG_SCALE == 'O') x = (x * (21.0 / 40.0)) + 7.5;
   else if(DEG_SCALE == 'R') x = (x + 273.15) * 1.8;
   else if(DEG_SCALE == 'K') x = x + 273.15;

   return (DATA_SIZE) x;
}

char *fmt_temp(double temp)
{
static char buf[SLEN];
double val;

   val = scale_temp(temp);
   if     (round_temp == 1) sprintf(buf, "%.1f %c%c", val, DEGREES, DEG_SCALE);
   else if(round_temp == 2) sprintf(buf, "%.2f %c%c", val, DEGREES, DEG_SCALE);
   else if(round_temp == 3) sprintf(buf, "%.3f %c%c", val, DEGREES, DEG_SCALE);
   else if(round_temp == 4) sprintf(buf, "%.4f %c%c", val, DEGREES, DEG_SCALE);
   else if(round_temp == 5) sprintf(buf, "%.5f %c%c", val, DEGREES, DEG_SCALE);
   else {
      if((rcvr_type == THERMO_RCVR) || (enviro_mode() && (have_temperature == 0))) {  // flag external temp sensor with upper case
         sprintf(buf, "%.5f %c%c", val, DEGREES, DEG_SCALE);
      }
      else {
         sprintf(buf, "%.6f %c%c", val, DEGREES, DEG_SCALE);
      }
   }
   return &buf[0];
}


DATA_SIZE scale_pressure(double t)
{
double x;

   // convert mb to inches of mercury
   x = (double) t;
   if(HG_PRESS) x *= 0.029533327;

   return (DATA_SIZE) x;
}

char *fmt_pressure(double pressure)
{
static char buf[SLEN];
double val;

   val = scale_pressure(pressure);
   if(HG_PRESS) sprintf(buf, "%.5f %s", val, "Hg");
   else         sprintf(buf, "%.5f %s", val, "mb");
   return &buf[0];
}



char *fmt_secs(double val)
{
static char buf[SLEN];
long secs;
char c1,c2;
int nav_flag;

   if(cursor_time_ref & 0x01) {  // time since start of displayed data
      c1 = '[';
      c2 = ']';
   }
   else {                 // time since start of capture
      c1 = '<';
      c2 = '>';
   }

   secs = (int) (val + 0.50);
   nav_flag = (((DATA_SIZE) (int) nav_rate) != nav_rate);

   if(cursor_time_ref & 0x02) {  // always show time as seconds
      if(nav_flag) {
         sprintf(buf, "%c%.1fs%c", c1, val, c2);
      }
      else {
         sprintf(buf, "%c%lds%c", c1, secs, c2);
      }
   }
   else if(nav_flag) {
      sprintf(buf, "%c%.1fs%c", c1, val, c2);
   }
   else if(secs < 60L) {
      sprintf(buf, "%c%lds%c", c1, secs, c2);
   }
   else if(secs < 60L*60L) {
      sprintf(buf, "%c%ldm%lds%c", c1, secs/60L, secs%60L, c2);
   }
   else {
      sprintf(buf, "%c%ldh%ldm%lds%c", c1, (secs/(60L*60L)), (secs/60L)%60L, secs%60L, c2);
   }

   return &buf[0];
}


void fmt_osc_val(int stat, char *id, double val)
{
char *s;

   if(stat) s = "";
   else if(plot[OSC].show_deriv) s = "*";
   else if(plot[OSC].show_freq) s = "#";
   else if(plot[OSC].drift_rate) s = "=";
   else s = ":";

   if(luxor) {
      if((val >= 100000.0) || (val <= (-10000.0))) {
         if(stat) sprintf(out, "%s%s%8.3f%s", id,s, val, plot[OSC].units);
         else     sprintf(out, "%s%s%7.3f%s", id,s, val, plot[OSC].units);
      }
      else if((val >= 10000.0) || (val <= (-1000.0))) {
         if(stat) sprintf(out, "%s%s%8.3f%s", id,s, val, plot[OSC].units);
         else     sprintf(out, "%s%s%7.3f%s", id,s, val, plot[OSC].units);
      }
      else if((val >= 1000.0) || (val <= (-100.0))) {
         if(stat) sprintf(out, "%s%s%8.3f%s", id,s, val, plot[OSC].units);
         else     sprintf(out, "%s%s%7.3f%s", id,s, val, plot[OSC].units);
      }
      else {
         if(stat) sprintf(out, "%s%s%8.3f%s", id,s, val, plot[OSC].units);
         else     sprintf(out, "%s%s%7.3f%s", id,s, val, plot[OSC].units);
      }
   }
   else if(rcvr_type == CS_RCVR) {
      if((val >= 100.0) || (val <= (-10.0))) {
         sprintf(out, "%s%s%6.1f%s", id,s, val, ppt_string);
      }
      else if((val >= 10.0) || (val <= (-1.0))) {
         sprintf(out, "%s%s%6.2f%s", id,s, val, ppt_string);
      }
      else {
         sprintf(out, "%s%s%6.3f%s", id,s, val, ppt_string);
      }
   }
   else if((rcvr_type == TICC_RCVR) || (rcvr_type == ZODIAC_RCVR)) {
      if((val >= 100000.0) || (val <= (-10000.0))) {
         sprintf(out, "%s%s%9.1f%s", id,s, val, ppt_string);
      }
      else if((val >= 10000.0) || (val <= (-1000.0))) {
         sprintf(out, "%s%s%9.2f%s", id,s, val, ppt_string);
      }
      else if((val >= 1000.0) || (val <= (-100.0))) {
         sprintf(out, "%s%s%9.3f%s", id,s, val, ppt_string);
      }
      else if((val >= 100.0) || (val <= (-10.0))) {
         sprintf(out, "%s%s%9.4f%s", id,s, val, ppt_string);
      }
      else if((val >= 10.0) || (val <= (-1.0))) {
         sprintf(out, "%s%s%9.5f%s", id,s, val, ppt_string);
      }
      else {
         sprintf(out, "%s%s%9.6f%s", id,s, val, ppt_string);
      }
   }
   else if((rcvr_type == VENUS_RCVR) && saw_venus_raw) {
      sprintf(out, "%s%s%.5f %s", id,s, val, plot[OSC].units);
   }
   else if(TIMING_RCVR) {
      if((val >= 100000.0) || (val <= (-10000.0))) {
         sprintf(out, "%s%s%11.3f%s", id,s, val, ppt_string);
      }
      else if((val >= 10000.0) || (val <= (-1000.0))) {
         sprintf(out, "%s%s%11.4f%s", id,s, val, ppt_string);
      }
      else if((val >= 1000.0) || (val <= (-100.0))) {
         sprintf(out, "%s%s%11.5f%s", id,s, val, ppt_string);
      }
      else if((val >= 100.0) || (val <= (-10.0))) {
         sprintf(out, "%s%s%11.6f%s", id,s, val, ppt_string);
      }
      else if((val >= 10.0) || (val <= (-1.0))) {
         sprintf(out, "%s%s%11.7f%s", id,s, val, ppt_string);
      }
      else {
         sprintf(out, "%s%s%11.8f%s", id,s, val, ppt_string);
      }
   }
   else if(rcvr_type == X72_RCVR) {  // xxxxx
      sprintf(out, "%s%s%.5g %s", id,s, val, plot[OSC].units);
   }
   else {
      val *= 1000.0;

      if((val >= 100000.0) || (val <= (-10000.0))) {
         sprintf(out, "%s%s%11.3f%s", id,s, val, ppt_string);
      }
      else if((val >= 10000.0) || (val <= (-1000.0))) {
         sprintf(out, "%s%s%11.4f%s", id,s, val, ppt_string);
      }
      else if((val >= 1000.0) || (val <= (-100.0))) {
         sprintf(out, "%s%s%11.5f%s", id,s, val, ppt_string);
      }
      else if((val >= 100.0) || (val <= (-10.0))) {
         sprintf(out, "%s%s%11.6f%s", id,s, val, ppt_string);
      }
      else if((val >= 10.0) || (val <= (-1.0))) {
         sprintf(out, "%s%s%11.7f%s", id,s, val, ppt_string);
      }
      else {
         sprintf(out, "%s%s%11.8f%s", id,s, val, ppt_string);
      }
   }
}


DATA_SIZE get_stat_val(int id)
{
DATA_SIZE val;

   // get the plot statistic value to show for the plot

   if(stat_count == (DATA_SIZE) 0.0) return (DATA_SIZE) 0.0;

   if(plot[id].show_stat == RMS) {
      strcpy(stat_id, "rms");
      val = sqrt(plot[id].sum_yy/stat_count);
   }
   else if(plot[id].show_stat == AVG) {
      strcpy(stat_id, "avg");
      val = plot[id].sum_y/stat_count;
   }
   else if(plot[id].show_stat == SDEV) {
      strcpy(stat_id, "sdv");
      val = ((plot[id].sum_yy/stat_count) - ((plot[id].sum_y/stat_count)*(plot[id].sum_y/stat_count)) );
//    val = ( plot[id].sum_yy - ((plot[id].sum_y*plot[id].sum_y)/stat_count) ) / stat_count;
      if(val < (DATA_SIZE) 0.0) val = (DATA_SIZE) 0.0 - val;
      val = sqrt(val);
   }
   else if(plot[id].show_stat == VAR) {
      strcpy(stat_id, "var");
      val = ( (plot[id].sum_yy/stat_count) - ((plot[id].sum_y/stat_count)*(plot[id].sum_y/stat_count)) );
//    val = ( plot[id].sum_yy - ((plot[id].sum_y*plot[id].sum_y)/stat_count) ) / stat_count;
   }
   else if(plot[id].show_stat == SHOW_MIN) {
      strcpy(stat_id, "min");
      val = plot[id].min_disp_val;
   }
   else if(plot[id].show_stat == SHOW_MAX) {
      strcpy(stat_id, "max");
      val = plot[id].max_disp_val;
   }
   else if(plot[id].show_stat == SHOW_SPAN) {
      strcpy(stat_id, "span");
      val = plot[id].max_disp_val - plot[id].min_disp_val;
   }
   else {
      strcpy(stat_id, "???");
      val = (DATA_SIZE) 0.0;
   }

   if(plot[id].show_deriv) strcat(stat_id, "*");
   else if(plot[id].show_freq) strcat(stat_id, "#");
   else if(plot[id].drift_rate) strcat(stat_id, "=");
   else strcat(stat_id, ":");

   return val;
}


DATA_SIZE trend_value(int plot, double t)
{
double r;
double a0, a1;
double sxx,syy,sxy;

   // calculate linear regression trend line slope and offset

   sxy = resid[plot].sum_xy - ((resid[plot].sum_x*resid[plot].sum_y)/resid[plot].resid_count);
   syy = resid[plot].sum_yy - ((resid[plot].sum_y*resid[plot].sum_y)/resid[plot].resid_count);
   sxx = resid[plot].sum_xx - ((resid[plot].sum_x*resid[plot].sum_x)/resid[plot].resid_count);

   if(sxx == 0.0F) {
      a1 = a0 = 0.0;
   }
   else {
      a1 = (sxy / sxx);
      a0 = (resid[plot].sum_y/resid[plot].resid_count) - (resid[plot].a1*(resid[plot].sum_x/resid[plot].resid_count));
   }

//if(plot == PPS) sprintf(debug_text2, "t:%f  a0:%.12f a1:%.12f", t,a0,a1);

   resid[plot].a0 = a0;
   resid[plot].a1 = a1;

   r = (a0 + (a1 * t));
   return (DATA_SIZE) r;
}


int tie_plot(int id)
{
   // return true if plot *id* is a time interval error plot

   if(rcvr_type == TICC_RCVR) {
      if((id == PPS) || (id == OSC) || (id == SEVEN) || (id == EIGHT)) return 1;
   }
   else if(((have_pps_offset == 0) || (have_pps_offset == 55)) && have_lars_pps) {
      if(id == PPS) return 1;
   }
   else if(rcvr_type == PRS_RCVR) {
      if(id == OSC) return 1;
   }
else if(0 && (rcvr_type == X72_RCVR)) {
   if(id == PPS) return 1;
}

   return 0;
}


int phase_plot(int id)
{
   // return true if plot *id* is a phase plot

   if(rcvr_type != TICC_RCVR) return 0;

   if((id == ONE) || (id == TWO) || (id == THREE) || (id == FOUR)) return 1;
   return 0;
}


DATA_SIZE remove_trend(int id, int phase_flag, double jd, DATA_SIZE val)
{
double jd0;
DATA_SIZE r;
double a0, a1;

   // remove linear regression trend line value from reading

   if(id <= 0) return val;
   if(id >= (NUM_PLOTS+DERIVED_PLOTS)) return val;

   if(plot[id].show_deriv) return val;       // plot derivative enabled
   if(plot[id].show_freq) return val;        // plot frequency enabled
   if(plot[id].drift_rate == 0) return val;  // trend removal not active for the plot

   if(phase_flag && phase_plot(id)) {  // it's a TICC_RCVR phase plot
      if(id == ONE) {
         jd0 = jd - pps_adev_jd0;    // time of first queue entry
         jd0 *= (24.0*60.0*60.0);
         r = trend_value(PPS, jd0);
//r -= resid[PPS].a0;    // for better match with TimeLab zero-based values
         val = val - r;
      }
      else if(id == TWO) {
         jd0 = jd - osc_adev_jd0;
         jd0 *= (24.0*60.0*60.0);
         r = trend_value(OSC, jd0);
//r -= resid[OSC].a0;
         val = val - r;
      }
      else if(id == THREE) {
         jd0 = jd - chc_adev_jd0;
         jd0 *= (24.0*60.0*60.0);
         r = trend_value(CHC, jd0);
//r -= resid[CHC].a0;
         val = val - r;
      }
      else if(id == FOUR) {
         jd0 = jd - chd_adev_jd0;
         jd0 *= (24.0*60.0*60.0);
         r = trend_value(CHD, jd0);
//r -= resid[CHD].a0;
         val = val - r;
      }
   }
   else {
      if(dynamic_trend_line) {   // dynamic trend removal
         a1 = plot[id].a1;
         a0 = plot[id].a0;
      }
      else {                     // static trend removal
         a1 = plot[id].drift_rate;
         a0 = 0.0;
      }
      val -= (DATA_SIZE) (a0 + (a1 * jd));
   }

   return val;
}


int right_arrow;  // signals plot queue has more data than is shown on the screen

int config_extra_plots()
{
int i;
int old_extra;

   old_extra = extra_plots;

   extra_plots = 0;  // this flag gets set if any extra plots are enabled
   for(i=FIRST_EXTRA_PLOT; i<NUM_PLOTS+DERIVED_PLOTS; i++) {
      extra_plots |= 1;  // plot[i].show_plot;
   }

   if(old_extra != extra_plots) {
      need_redraw = 1;
      if(rcvr_type == TICC_RCVR) plot_adev_data = 1;
      else if(extra_plots) plot_adev_data = 0;
      return 1;
   }
   return 0;
}


// NEW_RCVR may apply to the "label_plot_xxxx" routines.

int label_plot_scale(int id, int col)
{
char c;
double val;
DATA_SIZE sf;
char *s;

   // format the plot header scale factor line entries

   if(plot[id].user_scale == 0) c = '~';
   else c = '=';

   sf = (DATA_SIZE) fabs(plot[id].scale_factor);
   val = (double) plot[id].scale_factor * plot[id].invert_plot;
   if(id == OSC) val /= plot[id].ref_scale;

   if(tie_plot(id) && plot[id].show_freq) {
      sprintf(out, "%s%c(%.6f %s/div)",
                    plot[id].plot_id, c, val, "Hz");
   }
   else if(phase_plot(id)) {
      sprintf(out, "%s%c(%.6f %s/div)",
                    plot[id].plot_id, c, val, plot[id].units);
   }
   else if((id == ADC3) && (rcvr_type == THERMO_RCVR)) {  // PPS
         sprintf(out, "%s%c(%.6f %s/div)",
                       plot[id].plot_id, c, val, plot[id].units);
   }
   else if((id == ADC4) && (rcvr_type == THERMO_RCVR)) {  // OSC
         sprintf(out, "%s%c(%.6f %s/div)",
                       plot[id].plot_id, c, val, plot[id].units);
   }
   else if((id == DAC) && ((rcvr_type == CS_RCVR) || (rcvr_type == RFTG_RCVR) || (rcvr_type == SCPI_RCVR) || (rcvr_type == UCCM_RCVR) || (rcvr_type == ZYFER_RCVR))) {
      if(sf < (DATA_SIZE) 0.01) {
         sprintf(out, "%s%c(%.5f %s/div)",
                       plot[id].plot_id, c, val, plot[id].units);
      }
      else if(sf < (DATA_SIZE) 0.10) {
         sprintf(out, "%s%c(%.4f %s/div)",
                       plot[id].plot_id, c, val, plot[id].units);
      }
      else if(sf < (DATA_SIZE) 1.0) {
         sprintf(out, "%s%c(%.3f %s/div)",
                       plot[id].plot_id, c, val, plot[id].units);
      }
      else if(sf < (DATA_SIZE) 10.0) {
         sprintf(out, "%s%c(%.2f %s/div)",
                       plot[id].plot_id, c, val, plot[id].units);
      }
      else {
         sprintf(out, "%s%c(%.1f %s/div)",
                       plot[id].plot_id,  c, val, plot[id].units);
      }
   }
   else if(id == SAT_PLOT) {
      sprintf(out, "%s%c(%.0f/tick)",
                    plot[id].plot_id, c, 1.0);
   }
   else if(id == TEMP) {
      if(luxor) {
         if(sf >= (DATA_SIZE) 100.0) {
            sprintf(out, "%s%c(%ld %c%c/div)",
                         plot[id].plot_id, c, (long) (val+0.5), DEGREES,DEG_SCALE);
         }
         else if(sf < (DATA_SIZE) 1.0) {
            sprintf(out, "%s%c(%.2f %c%c/div)",
                          plot[id].plot_id, c, val, DEGREES,DEG_SCALE);
         }
         else {
            sprintf(out, "%s%c(%.1f %c%c/div)",
                          plot[id].plot_id, c, val, DEGREES,DEG_SCALE);
         }
      }
      else if((rcvr_type == UCCM_RCVR) && (scpi_type == UCCMP_TYPE) && (have_temperature == 0)) {  // samsung
         val /= 1000.0;
         if(sf >= (DATA_SIZE) 100.0) {
            sprintf(out, "%s%c(%ld %s/div)",
                         "TCOR", c, (long) (val+0.5), ppt_string);
         }
         else if(sf < (DATA_SIZE) 1.0) {
            sprintf(out, "%s%c(%.2f %s/div)",
                          "TCOR", c, val, ppt_string);
         }
         else {
            sprintf(out, "%s%c(%.1f %s/div)",
                          "TCOR", c, val, ppt_string);
         }
      }
      else {
         if(sf >= (DATA_SIZE) 100.0) {
            sprintf(out, "%s%c(%ld m%c%c/div)",
                         plot[id].plot_id, c, (long) (val+0.5), DEGREES,DEG_SCALE);
         }
         else if(sf < (DATA_SIZE) 1.0) {
            sprintf(out, "%s%c(%.2f m%c%c/div)",
                          plot[id].plot_id, c, val, DEGREES,DEG_SCALE);
         }
         else {
            sprintf(out, "%s%c(%.1f m%c%c/div)",
                          plot[id].plot_id, c, val, DEGREES,DEG_SCALE);
         }
      }
   }
   else if((id == PRESSURE) && enviro_mode()) {
      sprintf(out, "%s%c(%s/div)",
                    plot[id].plot_id, c, fmt_pressure(val));
   }
   else if(luxor) {
      if(sf >= (DATA_SIZE) 100.0) {
         sprintf(out, "%s%c(%ld %c%c/div)",
                      plot[id].plot_id, c, (long) (val+0.5), DEGREES,DEG_SCALE);
      }
      else if(sf < (DATA_SIZE) 1.0) {
         sprintf(out, "%s%c(%.2f %c%c/div)",
                       plot[id].plot_id, c, val, DEGREES,DEG_SCALE);
      }
      else {
         sprintf(out, "%s%c(%.1f %c%c/div)",
                       plot[id].plot_id, c, val, DEGREES,DEG_SCALE);
      }
   }
   else if(id == OSC) {
      if(luxor) {
         if(sf >= (DATA_SIZE) 100.0) {
            sprintf(out, "%s%c(%ld %s/div)",
                         plot[id].plot_id, c, (long) (val+0.5), plot[id].units);
         }
         else if(sf < (DATA_SIZE) 1.0) {
            sprintf(out, "%s%c(%.3f %s/div)",
                          plot[id].plot_id, c, val, plot[id].units);
         }
         else {
            sprintf(out, "%s%c(%.2f %s/div)",
                          plot[id].plot_id, c, val, plot[id].units);
         }
      }
      else if(rcvr_type == X72_RCVR) {  // xxxxxx
         sprintf(out, "%s%c(%.5g %s/div)",
                       plot[id].plot_id, c, val, plot[id].units);
      }
      else if(rcvr_type == ZODIAC_RCVR) {
         if(sf >= (DATA_SIZE) 100.0) {
            sprintf(out, "%s%c(%ld %s/div)",
                         plot[id].plot_id, c, (long) (val+0.5), plot[id].units);
         }
         else if(sf < (DATA_SIZE) 1.0) {
            sprintf(out, "%s%c(%.3f %s/div)",
                          plot[id].plot_id, c, val, plot[id].units);
         }
         else {
            sprintf(out, "%s%c(%.2f %s/div)",
                          plot[id].plot_id, c, val, plot[id].units);
         }
      }
      else {
         if(luxor) ;
         else if(rcvr_type == CS_RCVR) ;
         else if((rcvr_type == TICC_RCVR) || (rcvr_type == ZODIAC_RCVR)) ;
         else if(ACUTIME || PALISADE || ACU_360 || ACU_GG) ;
         else if(TIMING_RCVR) ;
         else if(rcvr_type == TSIP_RCVR) {
            val *= 1000.0;
         }
         else val *= 1000.0;

         if(sf >= (DATA_SIZE) 100.0) {
            sprintf(out, "%s%c(%ld %s/div)",
                         plot[id].plot_id, c, (long) (val+0.5), plot[id].units);
         }
         else if(sf < (DATA_SIZE) 1.0) {
            sprintf(out, "%s%c(%.3f %s/div)",
                          plot[id].plot_id, c, val, plot[id].units);
         }
         else {
            sprintf(out, "%s%c(%.2f %s/div)",
                          plot[id].plot_id, c, val, plot[id].units);
         }
      }
   }
   else {
      s = plot[id].units;
      if((id == DAC) && (rcvr_type == TSIP_RCVR)) s = "uV";
      else if((id == DAC) && (rcvr_type == TRUE_RCVR)) s = "uV";

      if(luxor) {
         if(sf >= (DATA_SIZE) 100.0) {
            sprintf(out, "%s%c(%ld %s/div)",
                         plot[id].plot_id, c, (long) (val+0.5), s);
         }
         else if(sf < (DATA_SIZE) 1.0) {
            sprintf(out, "%s%c(%.3f %s/div)",
                          plot[id].plot_id, c, val, s);
         }
         else {
            sprintf(out, "%s%c(%.2f %s/div)",
                          plot[id].plot_id, c, val, s);
         }
      }
      else if(rcvr_type == CS_RCVR) {
            sprintf(out, "%s%c(%.3f %s/div)",
                          plot[id].plot_id, c, val, s);
      }
      else if(rcvr_type == PRS_RCVR) {
            sprintf(out, "%s%c(%.3f %s/div)",
                          plot[id].plot_id, c, val, s);
      }
      else if(rcvr_type == X72_RCVR) {
            sprintf(out, "%s%c(%.3f %s/div)",
                          plot[id].plot_id, c, val, s);
      }
      else if((id == ONE) || (id == TWO) || (id == THREE)) {
         if(rcvr_type == ZYFER_RCVR) {
            sprintf(out, "%s%c(%.3f %s/div)",
                          plot[id].plot_id, c, val, s);
         }
         else {
            sprintf(out, "%s%c(%.5f %s/div)",
                          plot[id].plot_id, c, val, s);
         }
      }
      else {
          if(sf >= (DATA_SIZE) 100.0) {
             sprintf(out, "%s%c(%ld %s/div)",
                          plot[id].plot_id, c, (long) (val+0.5), s);
          }
          else if(rcvr_type == PRS_RCVR) {
                sprintf(out, "%s%c(%.3f %s/div)",
                              plot[id].plot_id, c, val, s);
          }
          else if(enviro_mode()) {
                sprintf(out, "%s%c(%.3f %s/div)",
                              plot[id].plot_id, c, val, s);
          }
          else if(sf < (DATA_SIZE) 1.0) {
             sprintf(out, "%s%c(%.3f %s/div)",
                           plot[id].plot_id, c, val, s);
          }
          else {
             sprintf(out, "%s%c(%.2f %s/div)",
                           plot[id].plot_id, c, val, s);
          }
      }
   }

   no_x_margin = no_y_margin = 1;
   vidstr(SCALE_ROW, col, plot[id].plot_color, out);
   no_x_margin = no_y_margin = 0;
   return strlen(out);
}

int label_plot_ref(int id, int col)
{
double val;
char c;

   // format the plot header reference value line entries

   val = (double) plot[id].plot_center;
   if(val == NEED_CENTER) val = 0.0;
   if(plot[id].float_center) c = '~';
   else                      c = '=';

   if(tie_plot(id) && plot[id].show_freq) {
      sprintf(out, "ref%c<%.7f %s>", c, val, "Hz");
   }
   else if(phase_plot(id)) {
      sprintf(out, "ref%c<%.7f %s>", c, val, plot[id].units);
   }
   else if((id == ADC3) && (rcvr_type == THERMO_RCVR)) {  // PPS
      sprintf(out, "ref%c<%.6f %s>", c, val, plot[id].units);
   }
   else if((id == ADC4) && (rcvr_type == THERMO_RCVR)) {  // OSC
      sprintf(out, "ref%c<%.6f %s>", c, val, plot[id].units);
   }
   else if(id == DAC)  {
      if(luxor)      sprintf(out, "ref%c<%.3f %s>", c, val, plot[DAC].units);
      else if(res_t) sprintf(out, "ref%c<%.6f ns>", c, val);
      else if(TIMING_RCVR) sprintf(out, "ref%c<%.6f ns>", c, val);
      else sprintf(out, "ref%c<%.6f %s>", c, val, plot[id].units);
   }
   else if(id == OSC)  {
      if(luxor)      sprintf(out, "ref%c<%.3f%s>", c, val, plot[id].units);
      else if(rcvr_type == CS_RCVR)   sprintf(out, "ref%c<%.1f %s>", c, val, plot[id].units);
      else if(rcvr_type == TICC_RCVR) sprintf(out, "ref%c<%.1f %s>", c, val, plot[id].units);
      else if(rcvr_type == X72_RCVR) {  // xxxxxx
         sprintf(out, "ref%c<%.5g %s>", c, val, plot[id].units);
      }
      else if(rcvr_type == ZODIAC_RCVR) sprintf(out, "ref%c<%.1f %s>", c, val, plot[id].units);
      else if(TIMING_RCVR) sprintf(out, "ref%c<%.1f %s>", c, val, plot[id].units);
      else sprintf(out, "ref%c<%.1f %s>", c, val*1000.0, plot[id].units);
   }
   else if(id == PPS) {
      if(res_t) sprintf(out, "ref%c<%.1f %s>", c, val/1000.0, plot[id].units);
      else if(rcvr_type == GPSD_RCVR)   sprintf(out, "ref%c<%.1f %s>", c, val/1.0E6F, plot[id].units);
      else if(rcvr_type == ESIP_RCVR)   sprintf(out, "ref%c<%.1f %s>", c, val, plot[id].units);
      else if(rcvr_type == FURUNO_RCVR) sprintf(out, "ref%c<%.1f %s>", c, val, plot[id].units);
      else if(ACU_GG)                   sprintf(out, "ref%c<%.1f %s>", c, val, plot[id].units);
      else if(ACU_360)                  sprintf(out, "ref%c<%.1f %s>", c, val, plot[id].units);
      else if(TIMING_RCVR)              sprintf(out, "ref%c<%.1f %s>", c, val/1000.0, plot[id].units);
      else sprintf(out, "ref%c<%.1f %s>", c, val, plot[id].units);
   }
   else if(id == SAT_PLOT) {
      sprintf(out, " ");
   }
   else if(id == TEMP) {
      if((rcvr_type == UCCM_RCVR) && (scpi_type == UCCMP_TYPE)) {
         sprintf(out, "ref%c<%.3f %s>", c, val, ppt_string);
      }
      else sprintf(out, "ref%c<%.3f %c%c>", c, scale_temp(val), DEGREES,DEG_SCALE);
   }
else if((id == PRESSURE) && enviro_mode()) {
   if(HG_PRESS) sprintf(out, "ref%c<%.3f %s>", c, scale_pressure(val), "Hg");
   else         sprintf(out, "ref%c<%.3f %s>", c, scale_pressure(val), "mb");
}
   else if(luxor && (id == TC2)) {
      sprintf(out, "ref%c<%.3f %c%c>", c, val, DEGREES,DEG_SCALE);
   }
   else if(rcvr_type == CS_RCVR) {
      sprintf(out, "ref%c<%.3f %s>", c, val, plot[id].units);
   }
   else if(rcvr_type == X72_RCVR) {
      sprintf(out, "ref%c<%.4f %s>", c, val, plot[id].units);
   }
   else if((id == ONE) || (id == TWO)) {
      if(rcvr_type == TIDE_RCVR) {  // ckckck
         sprintf(out, "ref%c<%.3f %s>", c, val, plot[id].units);
      }
      else if(rcvr_type == ZYFER_RCVR) {
         sprintf(out, "ref%c<%.4f %s>", c, val, plot[id].units);
      }
      else if(plot_loc) {
         sprintf(out, "ref%c<%.7f %s>", c, val, plot[id].units);
      }
      else sprintf(out, "ref%c<private %s>", c, plot[id].units);
   }
   else if(id == THREE) {
      sprintf(out, "ref%c<%.3f %s>", c, val, plot[id].units);
   }
   else if(rcvr_type == THERMO_RCVR) {
      sprintf(out, "ref%c<%.3f %s>", c, val, plot[id].units);
   }
   else sprintf(out, "ref%c<%.1f %s>", c, val, plot[id].units);

   no_x_margin = no_y_margin = 1;
   vidstr(REF_ROW, col, plot[id].plot_color, out);
   no_x_margin = no_y_margin = 0;

   return strlen(out);
}


int label_plot_stats(int id, int col)
{
double val;

   // format the plot header statistic value line entries

//   no_x_margin = 1;   // mmmmmmmmm
   val = get_stat_val(id);

   if(tie_plot(id) && plot[id].show_freq) {
      if((plot[id].show_stat == SHOW_MIN) || (plot[id].show_stat == SHOW_MAX) || (plot[id].show_stat == SHOW_SPAN)) ;
      else {
         val = tie_to_freq(id, val);
      }
      sprintf(out, "%s %.6f %s", stat_id, val, "Hz");
   }
   else if(phase_plot(id)) {
      sprintf(out, "%s %.6f %s", stat_id, val, plot[id].units);
   }
   else if((id == ADC3) && (rcvr_type == THERMO_RCVR)) {  // PPS
      sprintf(out, "%s %.6f %s", stat_id, val, plot[id].units);
   }
   else if((id == ADC4) && (rcvr_type == THERMO_RCVR)) {  // OSC
      sprintf(out, "%s %.6f %s", stat_id, val, plot[id].units);
   }
   else if(id == OSC) {  // xxxxxx
      fmt_osc_val(1, stat_id, val);
   }
   else if(id == PPS) {
      if(luxor) {
         val *= (double) lux_scale;
         if(val <= (-10000.0))     sprintf(out, "%s  %.3f %s", stat_id, val, plot[PPS].units);
         else if(val <= (-1000.0)) sprintf(out, "%s  %.3f %s", stat_id, val, plot[PPS].units);
         else if(val >= 10000.0)   sprintf(out, "%s  %.3f %s", stat_id, val, plot[PPS].units);
         else                      sprintf(out, "%s  %.3f %s", stat_id, val, plot[PPS].units);
      }
      else if(rcvr_type == CS_RCVR) {
         sprintf(out, "%s  %.1f %s", stat_id, val, plot[PPS].units);
      }
      else if(rcvr_type == ESIP_RCVR) {
         if(val <= (-10000.0))     sprintf(out, "%s %.4f ns", stat_id, val);
         else if(val <= (-1000.0)) sprintf(out, "%s %.5f ns", stat_id, val);
         else if(val >= 10000.0)   sprintf(out, "%s %.4f ns", stat_id, val);
         else                      sprintf(out, "%s %.6f ns", stat_id, val);
      }
      else if(rcvr_type == GPSD_RCVR) {
         val /= 1.0E6;  // convert ns to ms
         if(val <= (-10000.0))     sprintf(out, "%s %.4f ms", stat_id, val);
         else if(val <= (-1000.0)) sprintf(out, "%s %.5f ms", stat_id, val);
         else if(val >= 10000.0)   sprintf(out, "%s %.4f ms", stat_id, val);
         else                       sprintf(out, "%s %.6f ms", stat_id, val);
      }
      else if(rcvr_type == NVS_RCVR) { // 11.?f
         if(val <= (-10000.0))     sprintf(out, "%s %.4f %s", stat_id, val, plot[id].units);
         else if(val <= (-1000.0)) sprintf(out, "%s %.5f %s", stat_id, val, plot[id].units);
         else if(val >= 10000.0)   sprintf(out, "%s %.4f %s", stat_id, val, plot[id].units);
         else                      sprintf(out, "%s %.6f %s", stat_id, val, plot[id].units);
      }
      else if(rcvr_type == UBX_RCVR) {
         if(val <= (-10000.0))     sprintf(out, "%s %.4f ns", stat_id, val);
         else if(val <= (-1000.0)) sprintf(out, "%s %.5f ns", stat_id, val);
         else if(val >= 10000.0)   sprintf(out, "%s %.4f ns", stat_id, val);
         else                      sprintf(out, "%s %.6f ns", stat_id, val);
      }
      else if((rcvr_type == VENUS_RCVR) && saw_venus_raw) {
         sprintf(out, "%s %.1f m", stat_id, val);
      }
      else if(rcvr_type == ZODIAC_RCVR) {
         if(val <= (-10000.0))     sprintf(out, "%s %.4f ns", stat_id, val);
         else if(val <= (-1000.0)) sprintf(out, "%s %.5f ns", stat_id, val);
         else if(val >= 10000.0)   sprintf(out, "%s %.4f ns", stat_id, val);
         else                      sprintf(out, "%s %.6f ns", stat_id, val);
      }
      else if(lte_lite || ACU_GG || ACU_360) {
         if(val <= (-10000.0))     sprintf(out, "%s %.4f ns", stat_id, val);
         else if(val <= (-1000.0)) sprintf(out, "%s %.5f ns", stat_id, val);
         else if(val >= 10000.0)   sprintf(out, "%s %.4f ns", stat_id, val);
         else                      sprintf(out, "%s %.6f ns", stat_id, val);
      }
      else if(TIMING_RCVR) {  // 11.?f
         val /= 1000.0;
         if(val <= (-10000.0))     sprintf(out, "%s %.4f us", stat_id, val);
         else if(val <= (-1000.0)) sprintf(out, "%s %.5f us", stat_id, val);
         else if(val >= 10000.0)   sprintf(out, "%s %.4f us", stat_id, val);
         else                      sprintf(out, "%s %.6f us", stat_id, val);
      }
      else { // 11.?f
         if(val <= (-10000.0))     sprintf(out, "%s %.4f %s", stat_id, val, plot[id].units);
         else if(val <= (-1000.0)) sprintf(out, "%s %.5f %s", stat_id, val, plot[id].units);
         else if(val >= 10000.0)   sprintf(out, "%s %.4f %s", stat_id, val, plot[id].units);
         else                      sprintf(out, "%s %.6f %s", stat_id, val, plot[id].units);
      }
   }
   else if(id == SAT_PLOT) {
      sprintf(out, "%s  %.2f %s", stat_id, val, plot[id].units);
   }
   else if(id == TEMP) {
      if((rcvr_type == UCCM_RCVR) && (scpi_type == UCCMP_TYPE)) {
         sprintf(out, "%s %.6f %s", stat_id, val, ppt_string);
      }
      else {
         sprintf(out, "%s  %s", stat_id, fmt_temp(val));
      }
   }
else if((id == PRESSURE) && enviro_mode()) {
   sprintf(out, "%s  %s", stat_id, fmt_pressure(val));
}
   else if(luxor && (id == TC2)) {
      sprintf(out, "%s  %s", stat_id, fmt_temp(val));
   }
   else if(id == DAC) {
      if(luxor) {
         sprintf(out, "%s  %.3f %s", stat_id, val, plot[id].units);
      }
      else if(TIMING_RCVR) {
         sprintf(out, "%s %.6f ns", stat_id, val);
      }
      else {
         sprintf(out, "%s %.6f %s", stat_id, val, plot[id].units);
      }
   }
   else if(rcvr_type == CS_RCVR) {
      sprintf(out, "%s %.3f %s", stat_id, val, plot[id].units);
   }
   else if(rcvr_type == X72_RCVR) {
      sprintf(out, "%s %.5f %s", stat_id, val, plot[id].units);
   }
   else if((id == ONE) || (id == TWO)) {
      if(rcvr_type == TIDE_RCVR) {  // ckckck
         sprintf(out, "%s %.3f %s", stat_id, val, plot[id].units);
      }
      else if(rcvr_type == ZYFER_RCVR) {
         sprintf(out, "%s %.4f %s", stat_id, val, plot[id].units);
      }
      else if(plot_loc) {
         sprintf(out, "%s %.8f %s", stat_id, val, plot[id].units);
      }
      else sprintf(out, "%s (private) %s", stat_id, plot[id].units);
   }
   else if(id == THREE) {
      sprintf(out, "%s %.3f %s", stat_id, val, plot[id].units);
   }
   else if(id == SIX) {
      sprintf(out, "%s %.3f %s", stat_id, val, plot[id].units);
   }
   else {
      if(luxor) {
         if(id == LUX2) val *= (double) lum_scale;
         sprintf(out, "%s %.3f %s", stat_id, val, plot[id].units);
      }
      else {
         sprintf(out, "%s %.6f %s", stat_id, val, plot[id].units);
      }
   }

   if(plot[id].show_plot && plot[id].show_stat) {
      no_x_margin = no_y_margin = 1;
      vidstr(STAT_ROW, col, plot[id].plot_color, out);
      no_x_margin = no_y_margin = 0;
      return strlen(out);
   }
   else return 0;
}


int label_plot_cursor_val(int id, int col, struct PLOT_Q q)
{
double val;
char c;

   // format the plot header cursor value line entries

   if(plot[id].show_deriv) c = '*';
   else if(plot[id].show_freq) c = '#';
   else if(plot[id].drift_rate) c = '=';
   else c = ':';

   val = (double) q.data[id]/(double) queue_interval;
   out[0] = 0;

   if(tie_plot(id)) {  // it's a time interval error plot
      if(plot[id].show_freq) {  // frequency mode
         val = tie_to_freq(id, val);
         sprintf(out, "%s%c %.6f %s", plot[id].plot_id,c, val, "Hz");
      }
      else {  // time mode
         val = remove_trend(id, 1, q.q_jd, (DATA_SIZE) val);
         sprintf(out, "%s%c %.6f %s", plot[id].plot_id,c, val, plot[id].units);
      }
   }
   else if(phase_plot(id)) {
      val = remove_trend(id, 1, q.q_jd, (DATA_SIZE) val);
      sprintf(out, "%s%c %.6f %s", plot[id].plot_id,c, val, plot[id].units);
   }
   else if((id == ADC3) && (rcvr_type == THERMO_RCVR)) {  // PPS
      sprintf(out, "%s%c %.6f %s", plot[id].plot_id,c, val, plot[id].units);
   }
   else if((id == ADC4) && (rcvr_type == THERMO_RCVR)) {  // OSC
      sprintf(out, "%s%c %.6f %s", plot[id].plot_id,c, val, plot[id].units);
   }
   else if(id == DAC) {
      if(luxor) {
         sprintf(out, "%s%c %.3f V", plot[id].plot_id,c, val);
      }
      else if(TIMING_RCVR) {
         sprintf(out, "%s%c %.6f ns", plot[id].plot_id,c, val);
      }
      else {
         sprintf(out, "%s%c %.6f %s", plot[id].plot_id,c, val, plot[id].units);
      }
   }
   else if(id == TEMP) {
      if((rcvr_type == UCCM_RCVR) && (scpi_type == UCCMP_TYPE)) {
         sprintf(out, "%s%c %.6f %s", "TCOR", c, val, ppt_string);
      }
      else {
         sprintf(out, "%s%c %s", plot[id].plot_id,c, fmt_temp(val));
      }
   }
else if((id == PRESSURE) && enviro_mode()) {
   sprintf(out, "%s%c %s", plot[id].plot_id,c, fmt_pressure(val));
}
   else if(luxor && (id == TC2)) {
      sprintf(out, "%s%c %s", plot[id].plot_id,c, fmt_temp(val));
   }
   else if(id == OSC) {
      fmt_osc_val(0, plot[id].plot_id, val);
   }
   else if(id == PPS) {
      if(luxor) {
         val = val * (double) lux_scale;
         if(val <= (-10000.0))     sprintf(out, "%s%c %.3f %s", plot[id].plot_id,c, val, plot[id].units);
         else if(val <= (-1000.0)) sprintf(out, "%s%c %.3f %s", plot[id].plot_id,c, val, plot[id].units);
         else if(val >= 10000.0)   sprintf(out, "%s%c %.3f %s", plot[id].plot_id,c, val, plot[id].units);
         else                      sprintf(out, "%s%c %.3f %s", plot[id].plot_id,c, val, plot[id].units);
      }
      else if(rcvr_type == CS_RCVR) {
         sprintf(out, "%s%c %.1f uA", plot[id].plot_id,c, val);
      }
      else if(rcvr_type == ESIP_RCVR) {
         if(val <= (-10000.0))     sprintf(out, "%s%c %.4f ns", plot[id].plot_id,c, val);
         else if(val <= (-1000.0)) sprintf(out, "%s%c %.5f ns", plot[id].plot_id,c, val);
         else if(val >= 10000.0)   sprintf(out, "%s%c %.4f ns", plot[id].plot_id,c, val);
         else                      sprintf(out, "%s%c %.6f ns", plot[id].plot_id,c, val);
      }
      else if(rcvr_type == GPSD_RCVR) {
         val = val / 1.0E6;
         if(val <= (-10000.0))     sprintf(out, "%s%c %.4f ms", plot[id].plot_id,c, val);
         else if(val <= (-1000.0)) sprintf(out, "%s%c %.5f ms", plot[id].plot_id,c, val);
         else if(val >= 10000.0)   sprintf(out, "%s%c %.4f ms", plot[id].plot_id,c, val);
         else                      sprintf(out, "%s%c %.6f ms", plot[id].plot_id,c, val);
      }
      else if(rcvr_type == NVS_RCVR) {
         if(val <= (-10000.0))     sprintf(out, "%s%c %.4f ns/s", plot[id].plot_id,c, val);
         else if(val <= (-1000.0)) sprintf(out, "%s%c %.5f ns/s", plot[id].plot_id,c, val);
         else if(val >= 10000.0)   sprintf(out, "%s%c %.4f ns/s", plot[id].plot_id,c, val);
         else                      sprintf(out, "%s%c %.6f ns/s", plot[id].plot_id,c, val);
      }
      else if(rcvr_type == UBX_RCVR) {
         if(val <= (-10000.0))     sprintf(out, "%s%c %.4f ns", plot[id].plot_id,c, val);
         else if(val <= (-1000.0)) sprintf(out, "%s%c %.5f ns", plot[id].plot_id,c, val);
         else if(val >= 10000.0)   sprintf(out, "%s%c %.4f ns", plot[id].plot_id,c, val);
         else                      sprintf(out, "%s%c %.6f ns", plot[id].plot_id,c, val);
      }
      else if((rcvr_type == VENUS_RCVR) && saw_venus_raw) {
         sprintf(out, "%s%c %.1f m", plot[id].plot_id,c, val);
      }
      else if(rcvr_type == ZODIAC_RCVR) {
         if(val <= (-10000.0))     sprintf(out, "%s%c %.4f ns", plot[id].plot_id,c, val);
         else if(val <= (-1000.0)) sprintf(out, "%s%c %.5f ns", plot[id].plot_id,c, val);
         else if(val >= 10000.0)   sprintf(out, "%s%c %.4f ns", plot[id].plot_id,c, val);
         else                      sprintf(out, "%s%c %.6f ns", plot[id].plot_id,c, val);
      }
      else if(lte_lite) {
         if(val <= (-10000.0))     sprintf(out, "%s%c %.4f ns", plot[id].plot_id,c, val);
         else if(val <= (-1000.0)) sprintf(out, "%s%c %.5f ns", plot[id].plot_id,c, val);
         else if(val >= 10000.0)   sprintf(out, "%s%c %.4f ns", plot[id].plot_id,c, val);
         else                      sprintf(out, "%s%c %.6f ns", plot[id].plot_id,c, val);
      }
      else if(ACU_GG || ACU_360) {
         if(val <= (-10000.0))     sprintf(out, "%s%c %.4f ns", plot[id].plot_id,c, val);
         else if(val <= (-1000.0)) sprintf(out, "%s%c %.5f ns", plot[id].plot_id,c, val);
         else if(val >= 10000.0)   sprintf(out, "%s%c %.4f ns", plot[id].plot_id,c, val);
         else                      sprintf(out, "%s%c %.6f ns", plot[id].plot_id,c, val);
      }
      else if(TIMING_RCVR) {
         val = val / 1000.0F;
         if(val <= (-10000.0))     sprintf(out, "%s%c %.4f us", plot[id].plot_id,c, val);
         else if(val <= (-1000.0)) sprintf(out, "%s%c %.5f us", plot[id].plot_id,c, val);
         else if(val >= 10000.0)   sprintf(out, "%s%c %.4f us", plot[id].plot_id,c, val);
         else                      sprintf(out, "%s%c %.6f us", plot[id].plot_id,c, val);
      }
      else {
         if(val <= (-10000.0))     sprintf(out, "%s%c %.4f ns", plot[id].plot_id,c, val);
         else if(val <= (-1000.0)) sprintf(out, "%s%c %.5f ns", plot[id].plot_id,c, val);
         else if(val >= 10000.0)   sprintf(out, "%s%c %.4f ns", plot[id].plot_id,c, val);
         else                      sprintf(out, "%s%c %.6f ns", plot[id].plot_id,c, val);
      }
   }
   else if(id == SAT_PLOT) {
      sprintf(out, "%s%c %.0f %s", plot[id].plot_id,c, val, plot[id].units);
   }
   else if(rcvr_type == CS_RCVR) {
      sprintf(out, "%s%c %.3f %s", plot[id].plot_id,c, val, plot[id].units);
   }
   else if(rcvr_type == X72_RCVR) {
      sprintf(out, "%s%c %.5f %s", plot[id].plot_id,c, val, plot[id].units);
   }
   else if(id >= NUM_PLOTS) {  // derived plots DERIVED_PLOTS !!!!!
      if(luxor && (id == BATTW)) {        // battery watts
         val = (double) q.data[BATTI]*(double) q.data[BATTV] / (double) queue_interval;
      }
      else if(luxor && (id == LEDW)) {   // LED watts
         val = (double) q.data[LEDI]*(double) q.data[LEDV] / (double) queue_interval;
      }
      else if(luxor && (id == EFF)) { // driver efficency
         val = (double) q.data[BATTI]*(double) q.data[BATTV];
         if(val) val = (double) q.data[LEDI]*(double) q.data[LEDV] / val;
         val = val * 100.0;
      }
      else if(luxor && (id == CCT)) { // color temo
         cct_dbg = 1;
         val = (double) calc_cct(cct_type, 1, (double) q.data[REDHZ]/(double)queue_interval, (double) q.data[GREENHZ]/(double)queue_interval, (double) q.data[BLUEHZ]/(double)queue_interval);
         cct_dbg = 0;
      }
      else {
//       val = 0.0;
      }

      if(luxor) {
         sprintf(out, "%s%c %.3f %s", plot[id].plot_id,c, val, plot[id].units);
      }
      else {
         sprintf(out, "%s%c %.6f %s", plot[id].plot_id,c, val, plot[id].units);
      }
   }
   else if((id == ONE) || (id == TWO)) {
      if(rcvr_type == TIDE_RCVR) {  // ckckck
         sprintf(out, "%s%c %.3f %s", plot[id].plot_id,c, val, plot[id].units);
      }
      else if(rcvr_type == ZYFER_RCVR) {
         sprintf(out, "%s%c %.4f %s", plot[id].plot_id,c, val, plot[id].units);
      }
      else if(plot_loc) {
         sprintf(out, "%s%c %.8f %s", plot[id].plot_id,c, val, plot[id].units);
      }
      else sprintf(out, "%s%c (private) %s", plot[id].plot_id,c, plot[id].units);
   }
   else if(id == THREE) {
      sprintf(out, "%s%c %.3f %s", plot[id].plot_id,c, val, plot[id].units);
   }
   else if(id == SIX) {
      sprintf(out, "%s%c %.3f %s", plot[id].plot_id,c, val, plot[id].units);
   }
   else {
      if(luxor) {
         if(id == LUX2) val = val * (double) lum_scale;
         sprintf(out, "%s%c %.3f %s", plot[id].plot_id,c, val, plot[id].units);
      }
      else {
         sprintf(out, "%s%c %.6f %s", plot[id].plot_id,c, val, plot[id].units);
      }
   }

   no_x_margin = no_y_margin = 1;
   vidstr(CURSOR_VAL_ROW, col, plot[id].plot_color, out);
   no_x_margin = no_y_margin = 0;
   return strlen(out);
}


void label_plots()
{
COORD row, col;
COORD fft_col;
struct PLOT_Q q, q0, qq;
char *t;
char *s;
long i;
int id;
double val;
int len;
int max_len;
int month, day, year;
int hours, minutes, seconds;
double frac;
double old_utc, old_gps;
double q0_jd;
DATA_SIZE show_time;
int arrow_color;
int cursor_time_col;
int color;
u16 sat_flags;

   // label the plot with scale factors, etc
   if(text_mode) return;
   if(no_plots) return;
   if(rcvr_type == NO_RCVR) return;
   if(zoom_screen == 'P') ;
   else if(zoom_screen) return;
   if(first_key && SMALL_SCREEN) return;

   if(continuous_scroll) arrow_color = WHITE;
   else                  arrow_color = CYAN;


   fft_col = MOUSE_COL+40;
   no_x_margin = no_y_margin = 1;
   if(0) {  // erase the plot header area
      vidstr(CURSOR_TIME_ROW, 0, WHITE, &blanks[0]);  // was [TEXT_COLS-INFO_COL]);
      vidstr(CURSOR_VAL_ROW, 0,  WHITE, &blanks[0]);
      vidstr(STAT_ROW, 0,        WHITE, &blanks[0]);
      vidstr(REVIEW_ROW, 0,      WHITE, &blanks[0]);
      vidstr(REF_ROW, 0,         WHITE, &blanks[0]);
      vidstr(SCALE_ROW, 0,       WHITE, &blanks[0]);
   }
   else if((SCREEN_HEIGHT < TINY_TINY_HEIGHT) || (SCREEN_WIDTH < TINY_TINY_WIDTH)) {
      fill_rectangle(0,(MOUSE_ROW+3)*TEXT_HEIGHT, TEXT_COLS*TEXT_WIDTH,5*TEXT_HEIGHT, BLACK);
   }
   else {  // faster way to erase the plot header area
      fill_rectangle(0,(MOUSE_ROW+2)*TEXT_HEIGHT, TEXT_COLS*TEXT_WIDTH,6*TEXT_HEIGHT, BLACK);
   }

   t = "";
   row = PLOT_INFO_ROW;
   col = PLOT_TEXT_COL+0;

   if(review_mode) {
      if(review_home)  sprintf(out, "  ");
      else             sprintf(out, "%c ", LEFT_ARROW);
   }
   else {
      if(((plot_q_count*(long)plot_mag)/view_interval) >= PLOT_WIDTH) sprintf(out, "%c ", LEFT_ARROW);
      else sprintf(out, "  ");
   }
   vidstr(row, col, arrow_color, out);
   no_x_margin = no_y_margin = 0;


   hours   = pri_hours;     // save time
   minutes = pri_minutes;
   seconds = pri_seconds;
   frac    = pri_frac;
   day     = pri_day;
   month   = pri_month;
   year    = pri_year;
   old_utc = jd_utc;
   old_gps = jd_gps;



   // see if mouse is in the plot area,  if so get data point to display
   cursor_time_col = TEXT_COLS;
   col = PLOT_TEXT_COL+2;
   if(mouse_plot_valid) {
      if(filter_count) {
         q = filter_plot_q(last_mouse_q);
         jd_utc = filter_jd;
         sat_flags = filter_flags; // q.sat_flags;
      }
      else {
         q = get_plot_q(last_mouse_q);
         jd_utc = q.q_jd;
         sat_flags = q.sat_flags;
      }

      jd_gps = jd_utc + jtime(0,0,utc_offset,0.0);
      adjust_tz(1);  // tweak pri_ time for time zone

      if(top_line) {
         show_time = (DATA_SIZE) (view_interval*queue_interval*PLOT_WIDTH);  //seconds per screen
         show_time /= plot_mag;
         show_time /= (DATA_SIZE) 60.0;  // minutes/screen
         show_time /= (((DATA_SIZE) PLOT_WIDTH) / (DATA_SIZE) HORIZ_MAJOR);
         show_time /= nav_rate;
         sprintf(out, "Cursor time: %s %02d:%02d:%02d.%03d %s   View: %.1f min/div    ", fmt_date(0),
             pri_hours,pri_minutes,pri_seconds, (int) ((pri_frac*1000.0)+0.5),
             time_zone_set?tz_string:((q.sat_flags & UTC_TIME) ? "UTC":"GPS"),
             show_time
         );
      }
      else if(nav_rate && plot_mag) {
         if(cursor_time_ref) { // show cursor time interval from start of plot area
            val = ((double) (view_interval * mouse_x) / nav_rate) / (double) plot_mag;
         }
         else { // show cursor time interval from start of queue data
            q0 = get_plot_q(plot_q_out);
            q0_jd = (jd_utc - q0.q_jd) * (double) SECS_PER_DAY;
            val = q0_jd;
         }

         sprintf(out, "Cursor time: %s %02d:%02d:%02d.%03d %s  %s       ", fmt_date(0),
             pri_hours,pri_minutes,pri_seconds,  (int) ((pri_frac*1000.0)+0.5),
             time_zone_set?tz_string:((q.sat_flags & UTC_TIME) ? "UTC":"GPS"),
             fmt_secs(val)
         );
      }
      no_x_margin = no_y_margin = 1;
      color = MOUSE_COLOR;
      if(sat_flags & TIME_SKIP) color = SKIP_COLOR;
      if((SCREEN_HEIGHT < TINY_TINY_HEIGHT) || (SCREEN_WIDTH < TINY_TINY_WIDTH)) { // piss
         vidstr(CURSOR_TIME_ROW+1, PLOT_TEXT_COL+2, color, out);
      }
      else {
         vidstr(CURSOR_TIME_ROW, PLOT_TEXT_COL+2, color, out);
      }
      cursor_time_col = PLOT_TEXT_COL+2+strlen(out);
      no_x_margin = no_y_margin = 0;

      pri_hours   = hours;      // restore time
      pri_minutes = minutes;
      pri_seconds = seconds;
      pri_frac    = frac;
      pri_day     = day;
      pri_month   = month;
      pri_year    = year;
      jd_utc      = old_utc;
      jd_gps      = old_gps;
   }

   if(review_mode) {  // we are scrolling around in the plot queue data
      i = plot_q_out + plot_start;
      while(i >= plot_q_size) i -= plot_q_size;
      qq = get_plot_q(i);

      jd_utc = qq.q_jd;
      jd_gps = jd_utc + jtime(0,0,utc_offset,0.0);
      adjust_tz(2);  // tweak pri_ time for time zone

      if(view_all_data == 1) s = "All";
      else if(view_all_data == 2) s = "Auto";
      else s = "Review";

      sprintf(out, "%s (DEL to stop): %s %02d:%02d:%02d.%03d %s         ", s, fmt_date(0),
          pri_hours,pri_minutes,pri_seconds, (int) ((pri_frac*1000.0)+0.5),
          time_zone_set?tz_string:(qq.sat_flags & UTC_TIME) ? "UTC":"GPS");

      pri_hours   = hours;      // restore time
      pri_minutes = minutes;
      pri_seconds = seconds;
      pri_frac    = frac;
      pri_day     = day;
      pri_month   = month;
      pri_year    = year;
      jd_utc      = old_utc;
      jd_gps      = old_gps;

      no_x_margin = no_y_margin = 1;
      vidstr(REVIEW_ROW, PLOT_TEXT_COL+2,  CYAN, out);
      no_x_margin = no_y_margin = 0;
   }

   vidstr(CURSOR_VAL_ROW, 0, WHITE, &blanks[0]);
   for(id=0; id<NUM_PLOTS+DERIVED_PLOTS; id++) {  // show the plot values
      if(plot[id].show_plot == 0) continue;
      if(id == FFT) fft_col = col;

      max_len = 0;

      // show the plot scale factors
      len = label_plot_scale(id, col);
      if(len > max_len) max_len = len;


      // now show the plot zero reference line value
      len = label_plot_ref(id, col);
      if(len > max_len) max_len = len;


      // now we show the statistics values
      if(stat_count == 0.0) goto no_stats;
      if(all_adevs && (mixed_adevs == MIXED_NONE)) goto no_stats;
      len = label_plot_stats(id, col);
      if(len > max_len) max_len = len;


      // now display the cursor info
      no_stats:
      if(mouse_plot_valid == 0) goto no_mouse_info;
      if(queue_interval <= 0) goto no_mouse_info;
      len = label_plot_cursor_val(id, col, q);

      if(len > max_len) max_len = len;

      no_mouse_info:
      col += (max_len+1);
      if(col >= TEXT_COLS) break;
   }

   if(NO_SATS) ;
   else if(0 && mouse_plot_valid && plot_sat_count) {
      no_x_margin = no_y_margin = 1;
      sprintf(out, "Sats:%d", q.sat_flags & 0xFF);
      if(((unsigned) col) < (TEXT_COLS-strlen(out)-2)) {
         vidstr(CURSOR_VAL_ROW, col, CYAN, out);
      }
      no_x_margin = no_y_margin = 0;
   }

#ifdef FFT_STUFF
   if(plot[FFT].show_plot && fft_scale) {  // hist_plot
      out[0] = 0;
      if(fft_type == FFT_TYPE) {
         i = last_mouse_q - fft_queue_0;
         if(i < 0) i += plot_q_size;
         i /= (S32) fft_scale;
         val = (double) i * fps;
         if(i > (fft_length/2)) sprintf(out, "              ");
         else if(val) {
            val = 1.0/val;
            sprintf(out, "%s: %-.1f sec     ", plot[fft_id].plot_id, val);
         }
         else sprintf(out, "%s: DC blocked    ", plot[fft_id].plot_id);
      }
      else {  // histogram
         if(mouse_plot_valid && (mouse_x >= 0) && (mouse_x < hist_size)) {
            val = hist_minv + hist_bin_width*(DATA_SIZE) mouse_x;
//          val *= plot[fft_id].ref_scale;  // !!!!! ref_scale not always correct
            if(tie_plot(fft_id) && plot[fft_id].show_freq) {  // frequency mode
               sprintf(out, "%s: %f %s", plot[fft_id].plot_id, val, "Hz");
            }
            else if((rcvr_type == X72_RCVR) && (fft_id == OSC)) {
               sprintf(out, "%s: %.6g %s", plot[fft_id].plot_id, val, plot[fft_id].units);
            }
            else {
               sprintf(out, "%s: %f %s", plot[fft_id].plot_id, val, plot[fft_id].units);
            }
         }
      }

      no_x_margin = no_y_margin = 1;
//    if(fft_col < cursor_time_col) fft_col = cursor_time_col;
      if(mouse_plot_valid) {
         vidstr(FFT_VAL_ROW, fft_col, WHITE, &blanks[TEXT_COLS-fft_col]);  // erase the plot info area
         vidstr(FFT_VAL_ROW, fft_col, plot[FFT].plot_color, out);
      }
      no_x_margin = no_y_margin = 0;
   }
#endif



   row = PLOT_INFO_ROW;
   config_extra_plots();
   if((extra_plots == 0) && (SCREEN_WIDTH >= MEDIUM_WIDTH)) {
      no_x_margin = no_y_margin = 1;
      #ifdef ADEV_STUFF
         if((all_adevs == SINGLE_ADEVS) || (mixed_adevs != MIXED_GRAPHS)) {
            if     (ATYPE == OSC_ADEV) t = "ADEV";
            else if(ATYPE == OSC_HDEV) t = "HDEV";
            else if(ATYPE == OSC_MDEV) t = "MDEV";
            else if(ATYPE == OSC_TDEV) t = "TDEV";
            else if(ATYPE == PPS_ADEV) t = "ADEV";
            else if(ATYPE == PPS_HDEV) t = "HDEV";
            else if(ATYPE == PPS_MDEV) t = "MDEV";
            else if(ATYPE == PPS_TDEV) t = "TDEV";
            else if(ATYPE == CHC_ADEV) t = "ADEV";
            else if(ATYPE == CHC_HDEV) t = "HDEV";
            else if(ATYPE == CHC_MDEV) t = "MDEV";
            else if(ATYPE == CHC_TDEV) t = "TDEV";
            else if(ATYPE == CHD_ADEV) t = "ADEV";
            else if(ATYPE == CHD_HDEV) t = "HDEV";
            else if(ATYPE == CHD_MDEV) t = "MDEV";
            else if(ATYPE == CHD_TDEV) t = "TDEV";
            else if(ATYPE == A_MTIE)   t = "MTIE";
            else if(ATYPE == B_MTIE)   t = "MTIE";
            else if(ATYPE == C_MTIE)   t = "MTIE";
            else if(ATYPE == D_MTIE)   t = "MTIE";
            else                       t = "?DEV";

            sprintf(out, "%s %s       ", plot[PPS].plot_id, t);
            if(plot_adev_data) vidstr(row, PLOT_TEXT_COL+85,  PPS_ADEV_COLOR, out);
            else               vidstr(row, PLOT_TEXT_COL+85,  GREY,           out);

            sprintf(out, "%s %s       ", plot[OSC].plot_id, t);
            if(plot_adev_data) vidstr(row, PLOT_TEXT_COL+100, OSC_ADEV_COLOR, out);
            else               vidstr(row, PLOT_TEXT_COL+100, GREY,           out);

            // aaaahbbb - what about chC and chD? !!!!!!!
         }
      #endif
      no_x_margin = no_y_margin = 0;
   }

   if(right_arrow) {
      sprintf(out, " %c", RIGHT_ARROW);
      no_x_margin = no_y_margin = 1;
      vidstr(PLOT_INFO_ROW, (SCREEN_WIDTH/TEXT_WIDTH)-2, arrow_color, out);
      no_x_margin = no_y_margin = 0;
   }

   no_x_margin = no_y_margin = 0;
}




void mark_row(int y)
{
int x;
int color;

   // draw > < at the edges of the specified plot row

   if(0 && continuous_scroll) color = CYAN;
   else                       color = WHITE;

   y += PLOT_ROW;
   x = PLOT_COL;
   dot(x+0, y-5, color);   dot(x+0, y+5, color);
   dot(x+1, y-4, color);   dot(x+1, y+4, color);
   dot(x+2, y-3, color);   dot(x+2, y+3, color);
   dot(x+3, y-2, color);   dot(x+3, y+2, color);
   dot(x+4, y-1, color);   dot(x+4, y+1, color);
   dot(x+5, y-0, color);

   x = PLOT_COL + PLOT_WIDTH - 1;
   dot(x-0, y-5, color);   dot(x-0, y+5, color);
   dot(x-1, y-4, color);   dot(x-1, y+4, color);
   dot(x-2, y-3, color);   dot(x-2, y+3, color);
   dot(x-3, y-2, color);   dot(x-3, y+2, color);
   dot(x-4, y-1, color);   dot(x-4, y+1, color);
   dot(x-5, y-0, color);
}


void plot_mark(int symbol)
{
int x;
int y;
int temp;
char c;

   // draw the plot marker chars at the top of the plot area

   if(symbol < 0) symbol = 1;
   else if(symbol >= MAX_MARKER) symbol = (MAX_MARKER - 1);

   if(symbol < 10) c = ('0' + symbol);
   else            c = ('A' + symbol - 10);

   // draw user set data markers in the plot
   y = PLOT_ROW;
   x = PLOT_COL+plot_column;

   if(symbol == 0) {   // the mouse click marker
      temp = MARKER_COLOR;
if(button_lock && touch_screen) temp = YELLOW;
      dot(x+0, y+5, temp);
      dot(x+1, y+4, temp);    dot(x-1, y+4, temp);
      dot(x+2, y+3, temp);    dot(x-2, y+3, temp);
      dot(x+3, y+2, temp);    dot(x-3, y+2, temp);
      dot(x+4, y+1, temp);    dot(x-4, y+1, temp);
      dot(x+5, y+0, temp);    dot(x-5, y+0, temp);

      y = PLOT_ROW+PLOT_HEIGHT-1;
      dot(x+0, y-5, temp);
      dot(x+1, y-4, temp);    dot(x-1, y-4, temp);
      dot(x+2, y-3, temp);    dot(x-2, y-3, temp);
      dot(x+3, y-2, temp);    dot(x-3, y-2, temp);
      dot(x+4, y-1, temp);    dot(x-4, y-1, temp);
      dot(x+5, y-0, temp);    dot(x-5, y-0, temp);
   }
   else {   // the numeric markers
      #ifdef DIGITAL_CLOCK
         // we can use the vector character code to draw the markers
         x -= VCHAR_SCALE/2;
         if(x < 0) x = 0;
         temp = VCHAR_SCALE;
         if(SCREEN_HEIGHT < MEDIUM_HEIGHT) VCHAR_SCALE = 0;
         else VCHAR_SCALE = 1;
if(plot_adev_data) y += TEXT_HEIGHT;
         vchar(x,y+2, 0, MARKER_COLOR, c);
         VCHAR_SCALE = temp;
      #else
         // use text chars for the markers (may not go exactly where we want in DOS)
         #ifdef WIN_VFX
            graphics_coords = 1;
            x -= TEXT_WIDTH/2;
            if(x < 0) x = 0;
            y += 2;
         #endif
if(plot_adev_data) y += TEXT_HEIGHT;
         sprintf(out, "%c", c);
         vidstr(y,x, MARKER_COLOR, out);
         graphics_coords = 0;
      #endif
   }
}

#define ZOOM_LEFT  0   // margins for zoomed plot display
#define ZOOM_SPACE 20

void show_queue_info()
{
int row, col;
int j;
int adev_flag;
int color;
DATA_SIZE queue_time;

   // show the plot queue size and view interval settings
   if(first_key) return;
   if(zoom_screen == 'O') return;
   if(zoom_screen == 'H') return;

   col = FILTER_COL;
   if(text_mode) {
      row = TEXT_ROWS - 2;  // FILTER_ROW+3+eofs+1+3;
   }
   else if(zoom_screen == 'P') {
      col = ZOOM_LEFT + ZOOM_SPACE*2;
      row = 0;
   }
   else if(luxor) {
     row = VER_ROW+12;
     col = VER_COL;
   }
   else if(SCREEN_HEIGHT <= TINY_HEIGHT) {
      if(all_adevs && (adev_cols_shown > 2)) {
         row = MOUSE_ROW+2;
         if(pause_data) --row;
      }
      else row = MOUSE_ROW+1;
   }
   else {
      row = MOUSE_ROW+1;
   }

   j = row;
   if(luxor == 0) --j;
   queue_row = j;

   if(all_adevs && (adev_cols_shown > 2) && (SCREEN_WIDTH < WIDE_WIDTH)) {
       // aaawww
       sprintf(out, "                 ");
   }
   else if(zoom_screen == 'P') {
      if(pause_data) vidstr(j, col+ZOOM_SPACE*2, YELLOW, "UPDATES PAUSED   ");
      else           vidstr(j, col+ZOOM_SPACE*2, YELLOW, "                 ");
   }
   else if(pause_data) {
      vidstr(j, col, YELLOW, "UPDATES PAUSED   ");
      --j;
   }
   else {
      vidstr(j, col, YELLOW, "                 ");
      --j;
   }
   queue_row = j;

   #ifdef ADEV_STUFF
      if(all_adevs && (adev_cols_shown > 2) && (SCREEN_WIDTH < WIDE_WIDTH)) {
         // aaawww
         sprintf(out, "                    ");
      }
      else {
         adev_flag = 0;  // set to 0 if all adev periods are the same
         if(pps_adev_period != adev_period) adev_flag |= 0x01; // different channels have different adev periods
         if(osc_adev_period != adev_period) adev_flag |= 0x02;
         if(chc_adev_period != adev_period) adev_flag |= 0x04;
         if(chc_adev_period != adev_period) adev_flag |= 0x08;

         if(no_adev_flag) color = GREY;
         else if(adev_q_allocated == 0) color = GREY;
         else color = WHITE;

         if((adev_period <= 0.0) && (adev_flag == 0)) {  // aaaapppp
            if(luxor) sprintf(out, "                    ");
            else      sprintf(out, "ADEV:  OFF          ");
         }
         else if(adev_flag) {
            sprintf(out, "ADEVP: multiple   ");
         }
         else if(adev_period == 1.0) {
            sprintf(out, "ADEVQ: %ld pts ", adev_q_size);
         }
         else if(adev_period < 1.0)  sprintf(out, "ADEVP: %.3f secs    ", adev_period);
         else                        sprintf(out, "ADEVP: %.1f sec   ", adev_period);
         vidstr(j, col, color, out);
         if(zoom_screen == 'P') {
            col += ZOOM_SPACE;
         }
         else {
            --j;
            queue_row = j;
         }
      }
   #endif

   queue_time = (DATA_SIZE) (plot_q_size * queue_interval);
   queue_time /= nav_rate;
   if(all_adevs && (adev_cols_shown > 2) && (SCREEN_WIDTH < WIDE_WIDTH)) {
      // aaawww
      sprintf(out, "                    ");
   }
   else {
      if(queue_time >= (DATA_SIZE) (60.0*60.0*24.0)) sprintf(out, "PLOTQ: %.1f day  ", (DATA_SIZE) queue_time/(60.0*60.0*24.0));
      else if(queue_time >= (DATA_SIZE)(60.0*60.0))  sprintf(out, "PLOTQ: %.1f hr   ", (DATA_SIZE) queue_time/(60.0*60.0));
      else if(queue_time >= (DATA_SIZE)60.0)         sprintf(out, "PLOTQ: %.1f min  ", (DATA_SIZE) queue_time/60.0);
      else if(queue_time > (DATA_SIZE)0.0)           sprintf(out, "PLOTQ: %.1f sec  ", (DATA_SIZE) queue_time);
      else                                           sprintf(out, "PLOTQ: OFF          ");
      vidstr(j, col, WHITE, out);
   }

   if(luxor && (lat || lon)) show_sun_moon(j+2, col);

   if(zoom_screen == 'P') ;
   else --j;

   view_row = j;
   queue_row = j;
}

void show_view_info()
{
DATA_SIZE show_time;
int col;
int color;

   // tuck the diaplay interval data into whatever nook or crannie
   // we can find on the screen

   if((all_adevs == SINGLE_ADEVS) || mixed_adevs) {
      if(zoom_screen == 'P') col = ZOOM_LEFT;
      else if(luxor) col = VER_COL;
      else col = FILTER_COL;

      if(all_adevs && (adev_cols_shown > 2) && (SCREEN_WIDTH < WIDE_WIDTH)) {
         // aaawww
         sprintf(out, "                    ");
      }
      else if(show_min_per_div) {
         show_time = (DATA_SIZE) (view_interval*queue_interval*PLOT_WIDTH);  // seconds per screen
         show_time /= plot_mag;
         show_time /= (DATA_SIZE) 60.0;  // minutes/screen
         show_time /= (((DATA_SIZE) PLOT_WIDTH) / (DATA_SIZE) HORIZ_MAJOR);
         show_time /= nav_rate;
         if(SMALL_SCREEN) {
            sprintf(out, "%.1f min/div ", show_time);
         }
         else if((SCREEN_WIDTH <= NARROW_SCREEN) && (small_font != 1)) {
            sprintf(out, "%.1f min/div ", show_time);
         }
         else {
            if(show_time < (DATA_SIZE) 1.0) sprintf(out, "VIEW: %.1f sec/div  ", show_time*(DATA_SIZE) 60.0);
            else                            sprintf(out, "VIEW: %.1f min/div  ", show_time);
         }

         if(view_interval != 1) color = BLUE;
         else                   color = WHITE;

         if(all_adevs && (SCREEN_HEIGHT <= TINY_HEIGHT) && (adev_cols_shown > 2)) {
            // aaawww
            sprintf(out, "                    ");
         }
         else {
            vidstr(view_row, col, color, out);
            if(zoom_screen == 'P') col += ZOOM_SPACE;
            else --view_row;
            queue_row = view_row;
         }

         show_time = (DATA_SIZE) (view_interval*queue_interval*PLOT_WIDTH);  // seconds per screen
         show_time /= plot_mag;
         show_time /= nav_rate;
         if(all_adevs && (SCREEN_HEIGHT <= TINY_HEIGHT) && (adev_cols_shown > 2)) {
            // aaawww
            sprintf(out, "                    ");
         }
         else {
            if(show_time < (DATA_SIZE) (2.0*60.0*60.0)) sprintf(out, "VIEW: %.1f min     ", show_time/60.0F);
            else sprintf(out, "VIEW: %.1f hrs     ", show_time/(DATA_SIZE)(60.0*60.0));
            vidstr(view_row, col, color, out);
            if(zoom_screen == 'P') ;
            else --view_row;
            queue_row = view_row;
         }
      }

      label_plots();
   }
}


void show_plot_grid()
{
int x, y;
int color;
int col;
int j;
int top_adev_row;
int adev_decade_width;  // HORIZ_MAJOR divisions per decade

   if(bin_scale == 8) adev_decade_width = 4;  // 1-2-4-8 decades
   else if(bin_scale == 1) adev_decade_width = 1;  // 1 per decade
//   else if(bin_scale == 2) adev_decade_width = 4;  // 1 per octave
   else adev_decade_width = 3;  // 1-2-5 decades

   if(rcvr_type == NO_RCVR) return;
   top_adev_row = SCREEN_HEIGHT;


   for(x=0; x<PLOT_WIDTH; x++) {  // draw vertical features
      col = PLOT_COL + x;
      if((x % HORIZ_MAJOR) == 0) {  // major tick marks
         dot(col, PLOT_ROW+1, WHITE);               // along top horizontal axis
         dot(col, PLOT_ROW+2, WHITE);
         dot(col, PLOT_ROW+3, WHITE);

         dot(col, PLOT_ROW+PLOT_HEIGHT-2, WHITE);   // along bottom horizontal axis
         dot(col, PLOT_ROW+PLOT_HEIGHT-3, WHITE);
         dot(col, PLOT_ROW+PLOT_HEIGHT-4, WHITE);

         for(j=0; j<=PLOT_CENTER; j+=VERT_MAJOR) {
            dot(col,   PLOT_ROW+PLOT_CENTER+j-1, WHITE);  // + at intersections
            dot(col,   PLOT_ROW+PLOT_CENTER+j,   WHITE);
            dot(col,   PLOT_ROW+PLOT_CENTER+j+1, WHITE);
            if(col > PLOT_COL) dot(col-1, PLOT_ROW+PLOT_CENTER+j,WHITE);
            dot(col+1, PLOT_ROW+PLOT_CENTER+j,   WHITE);

            dot(col,  PLOT_ROW+PLOT_CENTER-j-1, WHITE);  // + at intersections
            dot(col,  PLOT_ROW+PLOT_CENTER-j,   WHITE);
            dot(col,  PLOT_ROW+PLOT_CENTER-j+1, WHITE);
            if(col > PLOT_COL) dot(col-1,PLOT_ROW+PLOT_CENTER-j, WHITE);
            dot(col+1,PLOT_ROW+PLOT_CENTER-j,    WHITE);
         }

         color = GREY;  // WHITE;
         if(plot_adev_data) {
            if((x % (HORIZ_MAJOR*adev_decade_width)) == 0) color = CYAN;  // ADEV 1-2-5 bins
         }

         for(j=VERT_MINOR; j<=PLOT_CENTER; j+=VERT_MINOR) {
            dot(col, PLOT_ROW+PLOT_CENTER+j, color);
            dot(col, PLOT_ROW+PLOT_CENTER-j, color);
         }
      }
      else if((x % HORIZ_MINOR) == 0) {  // minor tick marks
         dot(col, PLOT_ROW+1, WHITE);
         dot(col, PLOT_ROW+PLOT_CENTER, GREY);  // center line tick marks
         dot(col, PLOT_ROW+PLOT_HEIGHT-2, WHITE);

         if(plot_adev_data) color = CYAN;  // subtly highlight ADEV decades
         else               color = GREY;

         for(j=VERT_MAJOR; j<=PLOT_CENTER; j+=VERT_MAJOR) {
            dot(col, PLOT_ROW+PLOT_CENTER+j, color);
            dot(col, PLOT_ROW+PLOT_CENTER-j, color);
            if((PLOT_ROW+PLOT_CENTER-j) < top_adev_row) {
               if(color == CYAN) top_adev_row = (PLOT_ROW+PLOT_CENTER-j);
            }
            if(SCREEN_HEIGHT > SHORT_SCREEN) {
               if(plot_adev_data && (color != CYAN)) color = CYAN;
               else color = GREY;  // WHITE;
            }
            else if(zoom_screen == 'P') {
               if(plot_adev_data && (color != CYAN)) color = CYAN;
               else color = GREY;  // WHITE;
            }
            else {
               dot(col, PLOT_ROW+PLOT_CENTER, color);
            }
         }
      }
   }

   for(y=0; y<=PLOT_CENTER; y++) {  // draw horizontal features
      if((y % VERT_MAJOR) == 0) {
         if((HIGHLIGHT_REF == 0) || (y != 0)) {
            dot(PLOT_COL+1,              PLOT_ROW+PLOT_CENTER+y, WHITE);
            dot(PLOT_COL+1,              PLOT_ROW+PLOT_CENTER-y, WHITE);
            dot(PLOT_COL+PLOT_WIDTH-1-1, PLOT_ROW+PLOT_CENTER+y, WHITE);
            dot(PLOT_COL+PLOT_WIDTH-1-1, PLOT_ROW+PLOT_CENTER-y, WHITE);
            dot(PLOT_COL+2,              PLOT_ROW+PLOT_CENTER+y, WHITE);
            dot(PLOT_COL+2,              PLOT_ROW+PLOT_CENTER-y, WHITE);
            dot(PLOT_COL+PLOT_WIDTH-1-2, PLOT_ROW+PLOT_CENTER+y, WHITE);
            dot(PLOT_COL+PLOT_WIDTH-1-2, PLOT_ROW+PLOT_CENTER-y, WHITE);
            dot(PLOT_COL+3,              PLOT_ROW+PLOT_CENTER+y, WHITE);
            dot(PLOT_COL+3,              PLOT_ROW+PLOT_CENTER-y, WHITE);
            dot(PLOT_COL+PLOT_WIDTH-1-3, PLOT_ROW+PLOT_CENTER+y, WHITE);
            dot(PLOT_COL+PLOT_WIDTH-1-3, PLOT_ROW+PLOT_CENTER-y, WHITE);
         }
      }
      else if((y % VERT_MINOR) == 0) {
         dot(PLOT_COL+1,             PLOT_ROW+PLOT_CENTER+y,  WHITE);
         dot(PLOT_COL+1,             PLOT_ROW+PLOT_CENTER-y,  WHITE);
         dot(PLOT_COL+PLOT_WIDTH-1-1,PLOT_ROW+PLOT_CENTER+y,  WHITE);
         dot(PLOT_COL+PLOT_WIDTH-1-1,PLOT_ROW+PLOT_CENTER-y,  WHITE);
      }
   }

   // draw plot border
   draw_rectangle(PLOT_COL,PLOT_ROW+1, PLOT_WIDTH-1,PLOT_HEIGHT-1-1, WHITE);

   if(HIGHLIGHT_REF) {   // highlight plot center reference line with > ,,, < markers
      mark_row(PLOT_CENTER);
   }

   label_adev_grid(top_adev_row);  // label the ADEV grid lines if adevs being shown
}


void format_plot_title(char *title)
{
int len;
int i, j;
char c;

   if(title == 0) return;

   j = 0;
   i = 0;
   out[0] = 0;
   len = strlen(title);

   for(i=0; i<len; i++) {
      if((unsigned) j > (sizeof(plot_title)-2)) break;

      c = title[i];
      if(c == '&') {
         if(title[i+1] == '&') {  // && -> disciplining parameters
            if((unsigned)j > (sizeof(plot_title)-45)) break;
            if(osc_control_on) {
               sprintf(&out[j], "(OSC PID: P=%f D=%f F=%f I=%f)", OSC_P_GAIN, OSC_D_TC, OSC_FILTER_TC, OSC_I_TC);
            }
            else {
               sprintf(&out[j], "(TC=%.3f  DAMPING=%.3f  GAIN=%.3f Hz/V  INITV:%.3f)",
                   time_constant, damping_factor, osc_gain, initial_voltage);
            }
            j = strlen(out);
            ++i;
         }
         else if(title[i+1] == 'd') {  // &d -> date
            if((unsigned)j > (sizeof(plot_title)-20)) break;
            sprintf(&out[j], "%s", fmt_date(0));
            j = strlen(out);
            ++i;
         }
         else if(title[i+1] == 't') {  // &t -> time
            if((unsigned)j > (sizeof(plot_title)-20)) break;
            sprintf(&out[j], "%02d:%02d:%02d %s", pri_hours,pri_minutes,pri_seconds, tz_string);
            j = strlen(out);
            ++i;
         }
         else {
            out[j++] = '&';
            out[j] = 0;
         }
      }
      else {
         out[j++] = c;
         out[j] = 0;
         if(c == 0) break;
      }
   }
}

void show_title()
{
int row, col;
int color;

   if(showing_help) return;
   if(zoom_screen == 'O') return;
   if(zoom_screen == 'H') return;

   if(text_mode) {
      row = TEXT_ROWS-1;
      col = 0;
      no_x_margin = no_y_margin = 1;  // mmmmmmmm
   }
   else {
      if(zoom_screen) {
         row = (SCREEN_HEIGHT-1-(TEXT_HEIGHT-1));
      }
      else {
         row = (PLOT_ROW+PLOT_HEIGHT);
      }

      row /= TEXT_HEIGHT;
      row *= TEXT_HEIGHT;
      row = (row / TEXT_HEIGHT) - 1;
//    if(zoom_screen == 'P') row -= 1;
      col = PLOT_TEXT_COL + 1;
   }

   if(plot_title[0]) {
      format_plot_title(plot_title);
      vidstr(row, col, TITLE_COLOR, out);
      if(rcvr_type == NO_RCVR) ;
      else if((text_mode == 0) && (zoom_screen == 0)) {
         line(PLOT_COL,PLOT_ROW+PLOT_HEIGHT-1,  PLOT_COL+PLOT_WIDTH,PLOT_ROW+PLOT_HEIGHT-1, WHITE);
      }
   }

   if(0 && rpn_mode && (zoom_screen == 0)) {
      rpn_show();
   }

   if(text_mode) return;
   if(zoom_screen == 'P');
   else if(zoom_screen) return;
   else if(first_key) return;

   if(debug_text[0]) {  // !!!!! should we format_plot_title() debug messages?
      if(show_mah) vidstr(row-1, col, WHITE, debug_text);
      else         vidstr(row-1, col, GREEN, debug_text);
   }
   if(debug_text2[0]) {
      vidstr(row-2, col, YELLOW, debug_text2);
   }
   if(debug_text3[0]) {
      vidstr(row-3, col, CYAN, debug_text3);
   }
   if(debug_text4[0]) {
      vidstr(row-4, col, BLUE, debug_text4);
   }
   if(debug_text5[0]) {
      vidstr(row-5, col, MAGENTA, debug_text5);
   }
   if(debug_text6[0]) {
      vidstr(row-6, col, RED, debug_text6);
   }
   if(debug_text7[0]) {
      vidstr(row-7, col, BROWN, debug_text7);
   }

   if(filter_count) {
      if(disp_filter_type == 'P') {
         if(filter_count < 0) sprintf(out, "Peak filter: [%ld]", disp_filter_size());
         else                 sprintf(out, "Peak filter: %ld", disp_filter_size());
      }
      else if(disp_filter_type == 'B') {
         if(filter_count < 0) sprintf(out, "Back filter: [%ld]", disp_filter_size());
         else                 sprintf(out, "Back filter: %ld", disp_filter_size());
      }
      else if(disp_filter_type == 'C') {
         if(filter_count < 0) sprintf(out, "Center filter: [%ld]", disp_filter_size());
         else                 sprintf(out, "Center filter: %ld", disp_filter_size());
      }
      else if(disp_filter_type == 'F') {
         if(filter_count < 0) sprintf(out, "Fwd filter: [%ld]", disp_filter_size());
         else                 sprintf(out, "Fwd filter: %ld", disp_filter_size());
      }
      else {
         if(filter_count < 0) sprintf(out, "Filter: [%ld]", disp_filter_size());
         else                 sprintf(out, "Filter: %ld", disp_filter_size());
      }

      graphics_coords = 1;
      color = WHITE;
      if(plot_adev_data) {
         vidstr((PLOT_ROW+TEXT_HEIGHT), PLOT_COL+2, color, out);
      }
      else {
         vidstr((PLOT_ROW+2), PLOT_COL+2, color, out);
      }
      graphics_coords = 0;
   }
}


void plot_axes()
{
   // draw the plot background grid and label info
   if(first_key) return;   // plot area is in use for help/warning message

   erase_plot(ERASE_GRAPH_AREA);          // erase plot area
   if(read_only || just_read || no_send) {
      show_version_header();
   }

   if(no_plots) ;
   else if(rcvr_type == NO_RCVR) ;
   else {
      show_queue_info();      // show the queue stats
      if((text_mode == 0) && ((zoom_screen == 0) || (zoom_screen == 'P'))) {
         show_view_info();    // display the plot view settings
         show_plot_grid();    // display the plot grid
      }
   }
   show_title();           // display the plot title
}



DATA_SIZE round_scale(DATA_SIZE val)
{
double decade;

   // round the scale factor to a 1 2 (2.5) 5  sequence
   for(decade=1.0E-15; decade<=1.0E15; decade*=10.0) {
      if(val <= decade)       return (DATA_SIZE) decade;
      if(val <= (decade*2.0)) return (DATA_SIZE) (decade*2.0);
//    if(val <= (decade*2.5)) if(decade >= 10.0) return (DATA_SIZE) (decade*2.5);
      if(val <= (decade*2.5)) return (DATA_SIZE) (decade*2.5);
      if(val <= (decade*5.0)) return (DATA_SIZE) (decade*5.0);
   }

   return val;
}


void scale_plots()
{
long i;
struct PLOT_Q q;
int k;
DATA_SIZE scale[NUM_PLOTS+DERIVED_PLOTS];
DATA_SIZE val;

   // Calculate plot scale factors and center line reference points.
   // This routine uses the min and max values collected by calc_plot_statistics()

// if((auto_scale == 0) && (auto_center == 0)) return;
   if(queue_interval <= 0) return;

   // calc_queue_stats() already scanned the queue data to find mins and maxes
   // now we calculate good looking scale factors and center points for the plots

   if(plot_q_last_col) {  // we have data to calculate good graph scale factors from
      i = (PLOT_HEIGHT / VERT_MAJOR) & 0xFFFE;  //even number of major vertical divisions
      //  if(SCREEN_HEIGHT >= MEDIUM_HEIGHT) i -= 1;  // prevents multiple consecutive rescales
      i -= 1;  //!!!!
      if(i <= 0) i = 1;

      // scale graphs to the largest side of the curve above/below
      // the graph center reference line
      for(k=0; k<NUM_PLOTS+DERIVED_PLOTS; k++) {
         if(plot[k].float_center) {
            plot[k].plot_center = (plot[k].max_disp_val+plot[k].min_disp_val) / 2.0F;
         }

         if((plot[k].max_disp_val >= plot[k].plot_center) && (plot[k].min_disp_val < plot[k].plot_center)) {
            if((plot[k].max_disp_val-plot[k].plot_center) >= (plot[k].plot_center-plot[k].min_disp_val)) {
               scale[k] = (plot[k].max_disp_val-plot[k].plot_center);
            }
            else {
               scale[k] = plot[k].plot_center - plot[k].min_disp_val;
            }
         }
         else if(plot[k].max_disp_val >= plot[k].plot_center) {
            scale[k] = (plot[k].max_disp_val - plot[k].plot_center);
         }
         else {
            scale[k] = (plot[k].plot_center - plot[k].min_disp_val);
         }

         if(scale[k] == 0.0) { // force a non-zero scale factor so plot shows correctly
            scale[k] = (DATA_SIZE) 1.0E-9;
         }

         if(i > 1) scale[k] = (scale[k] / (DATA_SIZE) (i/2));
         scale[k] *= plot[k].ref_scale;
//if(k == OSC) sprintf(debug_text, "osc scale: %g ->   %g  min:%g max:%g", scale[k], round_scale(scale[k]), plot[k].min_disp_val, plot[k].max_disp_val);

         // round scale factors to nice values
         scale[k] = round_scale(scale[k]);
      }

      // set the working scale factors to the new values
      if(peak_scale) {  // don't ever let scale factors get smaller
         for(k=0; k<NUM_PLOTS+DERIVED_PLOTS; k++) {
            if(scale[k] > plot[k].scale_factor) {
               if(plot[k].user_scale == 0) {
                  plot[k].scale_factor = scale[k];
               }
            }
         }
      }
      else {  // scale factors can change to whatever they are
         for(k=0; k<NUM_PLOTS+DERIVED_PLOTS; k++) {
            if(plot[k].user_scale == 0) plot[k].scale_factor = scale[k];
         }
      }

      if(1 || auto_center) {  // center the plots
         for(k=0; k<NUM_PLOTS+DERIVED_PLOTS; k++) {
            if(plot[k].float_center) {
               plot[k].plot_center = (plot[k].max_disp_val+plot[k].min_disp_val) / 2.0F;
            }
         }
         last_dac_voltage = plot[DAC].plot_center;
//!!!!   last_temperature = plot[TEMP].plot_center;
      }
   }
   else if(plot_q_last_col >= 1) {  // center these graphs around the last recorded value
      if(filter_count) q = filter_plot_q(plot_q_last_col-1);
      else             q = get_plot_q(plot_q_last_col-1);

      for(k=0; k<NUM_PLOTS+DERIVED_PLOTS; k++) {
         if(plot[k].float_center) {
            if(k == TEMP) plot[k].plot_center = scale_temp((double) q.data[TEMP] / (double) queue_interval);
else if(enviro_mode() && (k == PRESSURE)) plot[k].plot_center = scale_pressure((double) q.data[PRESSURE] / (double) queue_interval);
            else if(luxor && (k == TC2)) plot[k].plot_center = scale_temp((double) q.data[k] / (double) queue_interval);
            else if(luxor && (k == LUX1))  plot[k].plot_center = (q.data[k] / (DATA_SIZE) queue_interval) * (DATA_SIZE) lux_scale;
            else if(luxor && (k == LUX2))  plot[k].plot_center = (q.data[k] / (DATA_SIZE) queue_interval) * (DATA_SIZE) lum_scale;
            else if(k >= NUM_PLOTS) {  // derived plots DERIVED_PLOTS !!!!!
               if(luxor && (k == BATTW)) {        // battery watts
                  plot[k].plot_center = q.data[BATTI]*q.data[BATTV] / (DATA_SIZE) queue_interval;
               }
               else if(luxor && (k == LEDW)) {   // LED watts
                  plot[k].plot_center = q.data[LEDI]*q.data[LEDV] / (DATA_SIZE) queue_interval;
               }
               else if(luxor && (k == EFF)) { // driver efficency
                  val = q.data[BATTI]*q.data[BATTV];
                  if(val) val = q.data[LEDI]*q.data[LEDV] / val;
                  val *= 100.0F;
                  plot[k].plot_center = val;
               }
               else if(luxor && (k == CCT)) { // color temo
                  val = calc_cct(cct_type, 1, (double)q.data[REDHZ]/(double)queue_interval, (double)q.data[GREENHZ]/(double)queue_interval, (double)q.data[BLUEHZ]/(double)queue_interval);
                  plot[k].plot_center = val;
               }
               else {
//                plot[k].plot_center = (DATA_SIZE) 0.0;
                  plot[k].plot_center = q.data[k] / (DATA_SIZE) queue_interval;
               }
            }
            else plot[k].plot_center = q.data[k] / (DATA_SIZE) queue_interval;
         }
      }
   }

   // finally we round the center line reference values to multiples of
   // the scale factor (note that this tweak can allow a plot to go slightly
   // off scale)
   if(1 || auto_center) {
      for(k=0; k<NUM_PLOTS+DERIVED_PLOTS; k++) {
         if(plot[k].float_center && plot[k].scale_factor) {
            plot[k].plot_center *= plot[k].ref_scale;
            plot[k].plot_center = (DATA_SIZE) (s64) ((plot[k].plot_center / plot[k].scale_factor)) * plot[k].scale_factor;
            plot[k].plot_center /= plot[k].ref_scale;
         }
      }
   }
}


int last_plot_col;
u08 plot_dot;

int plot_y(DATA_SIZE val, int last_y, int color)
{
int py;

   // draw a data point on the plot
   py = (int) (val * (DATA_SIZE) VERT_MAJOR);
   if(py >= PLOT_CENTER) {  // point is off the top of the plot area
      off_scale |= 0x01;
      py = PLOT_ROW;
   }
   else if(py <= (-PLOT_CENTER)) {  // point is off the bottom of the plot area
      off_scale |= 0x02;
      py = PLOT_ROW+PLOT_CENTER+PLOT_CENTER;
   }
   else {  // point is in the plot area
      py = (PLOT_ROW+PLOT_CENTER) - py;
   }

   if(plot_dot) dot(PLOT_COL+plot_column,py, color);
   else         line(PLOT_COL+last_plot_col,last_y,  PLOT_COL+plot_column,py, color);

   return py;
}


void plot_entry(long i)
{
DATA_SIZE x;
DATA_SIZE y;
DATA_SIZE val;
DATA_SIZE data;
double t;
struct PLOT_Q q;
struct PLOT_Q q2;
int col;
int py;
int k;
u16 sat_flags;

   // draw all data points for the next column in the plot

   if(first_key) return;    // plot area is in use for help/warning message
   if(text_mode) return;    // no graphics available in text mode
   if(rcvr_type == NO_RCVR) return;
   if(no_plots) return;
   if(zoom_screen == 'P');
   else if(zoom_screen) return;
   if(queue_interval <= 0) return;  // no queue data to plot

   if(filter_count) {
      q = filter_plot_q(i);   // plot filtered data
      sat_flags = filter_flags; // q.sat_flags;
   }
   else {
      q = get_plot_q(i);      // plot raw data
      sat_flags = q.sat_flags;
   }


   batt_mah += q.data[BATTI] * (DATA_SIZE) (queue_interval*view_interval/(DATA_SIZE) 3600.0);
   batt_mwh += q.data[BATTI] * (DATA_SIZE) q.data[BATTV] * (queue_interval*view_interval/(DATA_SIZE) 3600.0);
   load_mah += q.data[LEDI] * (DATA_SIZE) (queue_interval*view_interval/(DATA_SIZE) 3600.0);
   load_mwh += q.data[LEDI] * (DATA_SIZE) q.data[LEDV] * (queue_interval*view_interval/(DATA_SIZE) 3600.0);

   if(show_mah) {
      sprintf(debug_text, " Battery: Ah=%.3f  Wh=%.3f     LED: Ah=%.3f  Wh=%.3f", batt_mah,batt_mwh, load_mah,load_mwh);
   }


   for(k=0; k<NUM_PLOTS+DERIVED_PLOTS; k++) {     // compensate for linear trend line slope (drift rate)
      t = (double) plot_column*view_interval;
      q.data[k] = remove_trend(k, 0, t, q.data[k]);
   }

   col = PLOT_COL + plot_column;    // where we will be plotting
   if(plot_column <= 0) {           // no data or only one point to plot
      last_plot_col = 0;
      plot_dot = 1;
   }
   else {  // with more than one point available, connect the dots with lines
      last_plot_col = plot_column-plot_mag;
      if(last_plot_col < 0) last_plot_col = 0;
      plot_dot = 0;
   }

   if(sat_flags & TIME_SKIP) {  // flag skips and stutters in the time stamps
      if(plot_skip_data) {        // use small skip markers
         line(col,PLOT_ROW, col,PLOT_ROW+8, SKIP_COLOR);
      }
   }


   if(plot_holdover_data && (sat_flags & HOLDOVER)) {  // flag holdover events
      if(plot_dot) {
         dot(col,PLOT_ROW+0, HOLDOVER_COLOR);
         dot(col,PLOT_ROW+1, HOLDOVER_COLOR);
      }
      else {
         line(PLOT_COL+last_plot_col,PLOT_ROW+0, col,PLOT_ROW+0, HOLDOVER_COLOR);
         line(PLOT_COL+last_plot_col,PLOT_ROW+1, col,PLOT_ROW+1, HOLDOVER_COLOR);
      }
   }

   show_last_mouse_x(GREEN);

   // flag satellite constellation changes
   if(plot_const_changes && (sat_flags & CONST_CHANGE)) {
      line(col,PLOT_ROW+PLOT_HEIGHT, col,PLOT_ROW+PLOT_HEIGHT-8, CONST_COLOR);
//    plot_dot = 1;  // highlights satellite change discontinuites
   }

   if(plot[SAT_PLOT].show_plot) {  // show_sat_plot
      py = (q.sat_flags & SAT_COUNT_MASK) * COUNT_SCALE;
      if(py > PLOT_HEIGHT) py = PLOT_HEIGHT;
      py = (PLOT_ROW+PLOT_HEIGHT) - py;
      if(plot_dot) dot(col,py, COUNT_COLOR);
      else         line(PLOT_COL+last_plot_col,last_count_y, col,py, plot[SAT_PLOT].plot_color);
      last_count_y = py;
   }

   // draw each of the data plots
   for(k=0; k<NUM_PLOTS+DERIVED_PLOTS; k++) {
      if(plot[k].show_plot) {
         data = q.data[k];

         if(plot[k].show_deriv) {  // showing plot derivative
            if(filter_count) q2 = filter_plot_q(i-1);   // plot filtered data
            else             q2 = get_plot_q(i-1);      // plot raw data
            data -= q2.data[k];
         }
         else if(tie_plot(k) && plot[k].show_freq) {
            data = (DATA_SIZE) tie_to_freq(k, data);
         }

         if(k == TEMP) y = (DATA_SIZE) ((scale_temp((double) data / (double) queue_interval) - plot[k].plot_center) * plot[k].ref_scale);  // y = ppt
else if(enviro_mode() && (k == PRESSURE)) y = (DATA_SIZE) ((scale_pressure((double) data / (double) queue_interval) - plot[k].plot_center) * plot[k].ref_scale);
         else if(luxor && (k == TC2)) y = (DATA_SIZE) (((double) scale_temp(data / (double) queue_interval) - plot[k].plot_center) * plot[k].ref_scale);  // y = ppt
         else if(luxor && (k == LUX1)) y = (DATA_SIZE) (((data / queue_interval * lux_scale) - plot[k].plot_center) * plot[k].ref_scale);
         else if(luxor && (k == LUX2)) y = (DATA_SIZE) (((data / queue_interval * lum_scale) - plot[k].plot_center) * plot[k].ref_scale);
         else if(k >= NUM_PLOTS) {  // derived data DERIVED_PLOTS !!!!!
            if(luxor && (k == BATTW)) {        // battery watts
               y = (DATA_SIZE) (((q.data[BATTI]*q.data[BATTV] / queue_interval) - plot[k].plot_center) * plot[k].ref_scale);
            }
            else if(luxor && (k == LEDW)) {   // LED watts
               y = (DATA_SIZE) (((q.data[LEDI]*q.data[LEDV] / queue_interval) - plot[k].plot_center) * plot[k].ref_scale);
            }
            else if(luxor && (k == EFF)) { // driver efficency
               val = q.data[BATTI]*q.data[BATTV];
               if(val) val = q.data[LEDI]*q.data[LEDV] / val;
               val *= (DATA_SIZE) 100.0;
               y = (DATA_SIZE) ((val - plot[k].plot_center) * plot[k].ref_scale);
            }
            else if(luxor && (k == CCT)) { // color temo
               val = (DATA_SIZE) calc_cct(cct_type, 1, (double)q.data[REDHZ]/(double)queue_interval, (double)q.data[GREENHZ]/(double)queue_interval, (double)q.data[BLUEHZ]/(double)queue_interval);
               y = (DATA_SIZE) ((val - plot[k].plot_center) * plot[k].ref_scale);
            }
            else {
//             y = (DATA_SIZE) 0.0;
               y = (DATA_SIZE) (((data / queue_interval) - plot[k].plot_center) * plot[k].ref_scale);  // y = ppt
            }
         }
         else {
            y = (DATA_SIZE) (((data / queue_interval) - plot[k].plot_center) * plot[k].ref_scale);  // y = ppt
         }

         y /= (DATA_SIZE) (plot[k].scale_factor*plot[k].invert_plot);
         if(k != SAT_PLOT) {
            plot[k].last_y = plot_y(y, plot[k].last_y, plot[k].plot_color);
         }
      }

      if(plot[k].show_freq) ;        // can't plot trend line of freq plots
      else if(plot[k].show_deriv) ;  // can't plot trend line of derivative plots
      else if(plot[k].show_trend) {  // draw the trend line plot
         x = view_interval * queue_interval * (DATA_SIZE) plot_column;
         if(plot_mag) x /= (DATA_SIZE) plot_mag;
         y = (DATA_SIZE) (((plot[k].a0 + (plot[k].a1*x)) - plot[k].plot_center) * plot[k].ref_scale);  // y = ppt
         y /= (DATA_SIZE) (plot[k].scale_factor*plot[k].invert_plot);
         plot[k].last_trend_y = plot_y(y, plot[k].last_trend_y, plot[k].plot_color);
      }
   }
}

struct PLOT_Q last_q;

void add_stat_point(struct PLOT_Q *q)
{
int k;
DATA_SIZE val;
DATA_SIZE x;

   // calculate statistics of the points in the plot window
   if(queue_interval <= 0) return;

   x = (stat_count * view_interval * queue_interval);
   for(k=0; k<NUM_PLOTS+DERIVED_PLOTS; k++) {
      if(k >= NUM_PLOTS) {   // derived data DERIVED_PLOTS !!!!
         if(luxor && (k == BATTW)) {        // battery watts
            val = q->data[BATTI]*q->data[BATTV] / (DATA_SIZE) queue_interval;
         }
         else if(luxor && (k == LEDW)) {   // LED watts
            val = q->data[LEDI]*q->data[LEDV] / (DATA_SIZE) queue_interval;
         }
         else if(luxor && (k == EFF)) { // driver efficency
            val = q->data[BATTI]*q->data[BATTV];
            if(val) val = q->data[LEDI]*q->data[LEDV] / val;
            val *= (DATA_SIZE) 100.0;
         }
         else if(luxor && (k == CCT)) { // color temo  // !!!!!!! wrong! need raw data here
            val = (DATA_SIZE) calc_cct(cct_type, 1, (double)q->data[REDHZ]/(double)queue_interval, (double)q->data[GREENHZ]/(double)queue_interval, (double)q->data[BLUEHZ]/(double)queue_interval);
         }
         else {
//          val = (DATA_SIZE) 0.0;
            val = q->data[k] / queue_interval;
            plot[k].sum_change += (val - (last_q.data[k]/queue_interval));
         }
         // !!!!! sum_change
      }
      else {
         val = q->data[k] / queue_interval;
         plot[k].sum_change += (val - (last_q.data[k]/queue_interval));
      }

      plot[k].sum_y  += val;
      plot[k].sum_yy += (val * val);
      plot[k].sum_xy += (x * val);
      plot[k].sum_xx += (x * x);
      plot[k].sum_x  += x;
      plot[k].stat_count += 1.0F;
   }

   ++stat_count;
}

void calc_queue_stats(int stop)
{
long i;
int k;
struct PLOT_Q q;
struct PLOT_Q q2;
DATA_SIZE sxx, syy, sxy;
DATA_SIZE qi;
DATA_SIZE val;
double jd0;
double t;
int have_deriv;

   // prepare to calculate the statistics values of the plots
   for(k=0; k<NUM_PLOTS+DERIVED_PLOTS; k++) {
      plot[k].sum_x        = (DATA_SIZE) 0.0;
      plot[k].sum_y        = (DATA_SIZE) 0.0;
      plot[k].sum_xx       = (DATA_SIZE) 0.0;
      plot[k].sum_yy       = (DATA_SIZE) 0.0;
      plot[k].sum_xy       = (DATA_SIZE) 0.0;
      plot[k].stat_count   = (DATA_SIZE) 0.0;
      plot[k].sum_change   = (DATA_SIZE) 0.0;
      plot[k].max_disp_val = (DATA_SIZE) (-BIG_NUM);
      plot[k].min_disp_val = (DATA_SIZE) (BIG_NUM);
   }

   qi = (DATA_SIZE) queue_interval;

   if(1 || auto_center) {  // set min/max values in case auto scaling is off
      for(k=0; k<NUM_PLOTS+DERIVED_PLOTS; k++) {
         if(plot[k].float_center == 0) {
            plot[k].min_disp_val = plot[k].max_disp_val = plot[k].plot_center;
            if(k == TEMP) {      // !!!! ref_scale?
               if(plot[k].plot_center == NEED_CENTER) {
                  if(last_temperature) plot[k].min_disp_val = plot[k].max_disp_val = plot[k].plot_center = scale_temp((double) last_temperature);
               }
            }
else if(enviro_mode() && (k == PRESSURE)) {      // !!!! ref_scale?
    if(plot[k].plot_center == NEED_CENTER) {
       if(last_pressure) plot[k].min_disp_val = plot[k].max_disp_val = plot[k].plot_center = scale_temp((double) last_pressure);
    }
 }
            else if(luxor && (k == TC2)) {      // !!!! ref_scale?
               if(plot[k].plot_center == NEED_CENTER) {
                  plot[k].min_disp_val = plot[k].max_disp_val = plot[k].plot_center = scale_temp((double) tc2);
               }
            }
            else if(k == DAC) {  // !!!! ref_scale?
               if(plot[k].plot_center == NEED_CENTER) {
                  plot[k].min_disp_val = plot[k].max_disp_val = plot[k].plot_center = last_dac_voltage;
               }
            }
            else if(luxor && (k == PPS)) {
               // !!!!!! luxor do something?
            }
         }
      }
   }

   plot_column = 0;
   stat_count = 0.0F;
   plot_q_last_col = 0;

   have_deriv = 0;
   for(k=0; k<NUM_PLOTS+DERIVED_PLOTS; k++) {  // see if any derivative plots are enabled
      if(plot[k].show_deriv) {
         have_deriv = 1;
         break;
      }
   }

   i = plot_q_col0;
   jd0 = jd_utc;
   while(i != plot_q_in) {  // scan the data that is in the plot window
      if(filter_count) q = filter_plot_q(i);
      else             q = get_plot_q(i);

      if(have_deriv) {  // showing plot derivative  // get previous queue entry
         if(filter_count) q2 = filter_plot_q(i-1);   // plot filtered data
         else             q2 = get_plot_q(i-1);      // plot raw data
      }

      if(i == plot_q_col0) {
         jd0 = q.q_jd;
         last_q = q;
q2 = q;
      }

      add_stat_point(&q);   // update statistics values
      last_q = q;

      if((queue_interval > 0) && qi) {  // find plot min and max value
         for(k=0; k<NUM_PLOTS+DERIVED_PLOTS; k++) {
            if(k == TEMP) val = scale_temp((double) q.data[TEMP]/(double) qi);
else if(enviro_mode() && (k == PRESSURE)) val = scale_pressure((double) q.data[PRESSURE]/(double) qi);
            else if(luxor && (k == TC2)) val = scale_temp((double)q.data[k]/(double)qi);
            else if(luxor && (k == LUX1)) val = q.data[k]/qi*(DATA_SIZE) lux_scale;
            else if(luxor && (k == LUX2)) val = q.data[k]/qi*(DATA_SIZE) lum_scale;
            else if(k >= NUM_PLOTS) {  // derived plots DERIVED_PLOTS !!!!!
               if(luxor && (k == BATTW)) {        // battery watts
                  val = q.data[BATTI]*q.data[BATTV] / qi;
               }
               else if(luxor && (k == LEDW)) {   // LED watts
                  val = q.data[LEDI]*q.data[LEDV] / qi;
               }
               else if(luxor && (k == EFF)) { // driver efficency
                  val = q.data[BATTI]*q.data[BATTV];
                  if(val) val = q.data[LEDI]*q.data[LEDV] / val;
                  val *= (DATA_SIZE) 100.0;
               }
               else if(luxor && (k == CCT)) { // color temo
                  val = (DATA_SIZE) calc_cct(cct_type, 1, (double)q.data[REDHZ]/(double)qi, (double)q.data[GREENHZ]/(double)qi, (double)q.data[BLUEHZ]/(double)qi);
               }
               else {
//                val = (DATA_SIZE) 0.0;
                  val = q.data[k] / qi;
               }
            }
            else val = q.data[k] / qi;

            if(plot[k].show_deriv) {   // calculate plot derivative value
               val = q.data[k] - q2.data[k];
            }
            else if(tie_plot(k) && plot[k].show_freq) {
               val = (DATA_SIZE) tie_to_freq(k, val);
            }
            else if(phase_plot(k)) {  // calculate TICC_RCVR phase value with trend line removed
               val = remove_trend(k, 1, q.q_jd, val);
            }
            else if(plot[k].drift_rate) {  // calculate value with trend line removed
               t = q.q_jd - jd0;
               t *= (24.0*60.0*60.0);
               val = remove_trend(k, 0, t, val);
            }

            if(val > plot[k].max_disp_val) plot[k].max_disp_val = val;
            if(val < plot[k].min_disp_val) plot[k].min_disp_val = val;
         }
      }

      plot_q_last_col = i;
      i = next_q_point(i, stop);
      if(i < 0) break;      // end of plot data reached
   }

   if(stat_count == (DATA_SIZE) 0.0) return;

   for(k=0; k<NUM_PLOTS+DERIVED_PLOTS; k++) { // calculate linear regression values
      sxy = plot[k].sum_xy - ((plot[k].sum_x*plot[k].sum_y)/stat_count);
      syy = plot[k].sum_yy - ((plot[k].sum_y*plot[k].sum_y)/stat_count);
      sxx = plot[k].sum_xx - ((plot[k].sum_x*plot[k].sum_x)/stat_count);
      plot[k].stat_count = stat_count;
      if(sxx == 0.0F) {
         plot[k].a1 = plot[k].a0 = (DATA_SIZE) 0.0;
      }
      else {
         plot[k].a1 = (sxy / sxx);
         plot[k].a0 = (plot[k].sum_y/stat_count) - (plot[k].a1*(plot[k].sum_x/stat_count));
      }
   }
}


void plot_queue_data()
{
long i;
long last_i;
int j;
u08 ticker[MAX_MARKER];

   //  plot the data points
   plot_column = 0;

   batt_mah = batt_mwh = 0.0F;
   load_mah = load_mwh = 0.0F;

   if((all_adevs == SINGLE_ADEVS) || mixed_adevs) {
      for(j=0; j<MAX_MARKER; j++) { // prepare to find what marked points are shown
         ticker[j] = 1;
      }

      i = plot_q_col0;
      while(i != plot_q_in) {  // plot the data that is in the queue
         plot_entry(i);        // plot the data values

         last_i = i;           // go to next point to plot
         i = next_q_point(i, STOP_AT_PLOT_END);
         if(i < 0) break;      // end of plot data reached

         for(j=0; j<MAX_MARKER; j++) {  // see if the queue entry is marked
            if(ticker[j] && mark_q_entry[j] && (mark_q_entry[j] >= last_i) && (mark_q_entry[j] <= i)) {
               plot_mark(j);
               ticker[j] = 0;
            }
         }
      }
      plot_q_last_col = last_i;

      // see if we have more data in the queue that can be plotted
      if((plot_q_count && (i == plot_q_in)) || (i == (-2L))) {  // more data is available
         sprintf(out, " %c", RIGHT_ARROW);
         right_arrow = 1;
      }
      else {
         sprintf(out, "  ");
         right_arrow = 0;
      }
      vidstr(PLOT_TEXT_ROW-1, (SCREEN_WIDTH/TEXT_WIDTH)-2, WHITE, out);
   }
}


void draw_plot(u08 refresh_ok)
{
   // draw everything related to the data plots
   if(first_key) return;   // plot area is in use for help/warning message

   if(text_mode || (rcvr_type == NO_RCVR) || no_plots) {   // plot area is not available,  only draw text stuff
      plot_axes();
      #ifdef ADEV_STUFF
         if(all_adevs && (refresh_ok != 2)) {
            show_adev_info(10);
         }
      #endif
      if(com[RCVR_PORT].process_com == 0) {
         show_satinfo();
         show_param_values(1); //simeof
      }
      if(refresh_ok) refresh_page();
      return;   // no plotting in text modes
   }

   // locate the first queue entry we will be plotting
   plot_q_col0 = plot_q_out + plot_start;
   while(plot_q_col0 >= plot_q_size) plot_q_col0 -= plot_q_size;

   if(zoom_screen == 'P') ;
   else if(zoom_screen == 'F') ; // FFT waterfall
   else if(zoom_screen) return;

   if(queue_interval <= 0) {
      plot_axes();
      return;  // we have no queued data to plot
   }

   #ifdef FFT_STUFF
      if(plot[FFT].show_plot && show_live_fft && (live_fft != FFT)) {    // calc FFT over the live data
         calc_fft(live_fft);
      }
   #endif
   if(zoom_screen == 'F') return;

   calc_queue_stats(STOP_AT_PLOT_END);  // calc stat info data that will be displayed

   scale_plots();       // find scale factors and the values to center the graphs around
   plot_axes();         // draw and label the plot grid
   plot_queue_data();   // plot the queue data

   #ifdef ADEV_STUFF
      if(refresh_ok != 2) {   // draw the adev info
         show_adev_info(11);
      }
   #endif

   if(com[RCVR_PORT].process_com == 0) {
      show_satinfo();
      show_param_values(1); //simeof
   }
   if(refresh_ok) {     // copy buffered screen image to the screen
      refresh_page();
   }
}


void update_plot(int draw_flag)
{
struct PLOT_Q q;
double v;
DATA_SIZE jitter;
DATA_SIZE sig;

   // add current data values to the plot data queue
   // NEW_RCVR
   //
   // !!!! IF YOU ADD A NEW DEVICE AND STORE OTHER VALUES IN THE PLOT QUEUE
   // YOU MUST ALSO MODIFY write_q_entry() TO SAVE/CALCULATE/RESTORE THEM
   // WHEN IT WRITES A LOG FILE ENTRY !!!!

   q = get_plot_q(plot_q_in);

   // update the plot queue with the current data
   if((have_temperature == 0) && have_lars_temp) {
      q.data[TEMP] += lars_temp;
      plot[TEMP].plot_id = "TEMPL";
   }
   else {
      q.data[TEMP] += temperature;
   }
   q.data[SAT_PLOT] = (DATA_SIZE) sat_count;

   if((have_dac == 0) && (have_sawtooth == 0) && have_lars_dac) {
      q.data[DAC] += lars_dac;
      plot[DAC].units = "";
      plot[DAC].plot_id = "DACL";
      plot[DAC].ref_scale = (DATA_SIZE) 1.0;
   }
   else {
      q.data[DAC] += dac_voltage;
   }

   if(rcvr_type == TICC_RCVR) {
      q.data[PPS]  += (DATA_SIZE) pps_tie;
      q.data[OSC]  += (DATA_SIZE) osc_tie;
      q.data[CHC]  += (DATA_SIZE) chc_tie;  // [SEVEN]
      q.data[CHD]  += (DATA_SIZE) chd_tie;  // [EIGHT]
   }
   else if(((have_pps_offset == 0) || (have_pps_offset == 55)) && have_lars_pps) {
      q.data[PPS]  += (DATA_SIZE) lars_pps;
      q.data[OSC]  += (DATA_SIZE) osc_offset;
      plot[PPS].plot_id = "PPSL";
   }
   else if(rcvr_type == CS_RCVR) {
      q.data[PPS]  += (DATA_SIZE) cs_pump;
      q.data[OSC]  += (DATA_SIZE) cs_emul;
   }
   else if(rcvr_type == LPFRS_RCVR) {
      q.data[PPS]  += (DATA_SIZE) pps_offset;
      q.data[OSC]  += (DATA_SIZE) osc_offset;
   }
   else if(rcvr_type == SRO_RCVR) {
      q.data[PPS]  += (DATA_SIZE) pps_offset;
      q.data[OSC]  += (DATA_SIZE) (((double)sro_fc)*SRO_DDS_STEP*1.0E12/1000.0);
   }
   else if(rcvr_type == THERMO_RCVR) {
      q.data[ADC3]  += (DATA_SIZE) adc3;
      q.data[ADC4]  += (DATA_SIZE) adc4;
   }
   else if((rcvr_type == TIDE_RCVR) && enviro_mode()) {
      q.data[ADC3]  += (DATA_SIZE) adc3;
      q.data[ADC4]  += (DATA_SIZE) adc4;
   }
   else if((rcvr_type == NO_RCVR) && enviro_mode()) {
      q.data[ADC3]  += (DATA_SIZE) adc3;
      q.data[ADC4]  += (DATA_SIZE) adc4;
   }
   else if(luxor) {
      q.data[PPS]  += (DATA_SIZE) pps_offset;
      q.data[OSC]  += (DATA_SIZE) osc_offset;
   }
   else {
      q.data[PPS]  += (DATA_SIZE) pps_offset;
      q.data[OSC]  += (DATA_SIZE) osc_offset;
//    q.data[CHC]  += (DATA_SIZE) chc_offset;  // !!!! not for non TICC_RCVR
//    q.data[CHD]  += (DATA_SIZE) chd_offset;
   }

   if(rcvr_type == CS_RCVR) {
      if(ONE < NUM_PLOTS)   q.data[ONE] += (DATA_SIZE) cs_rfam1;
      if(TWO < NUM_PLOTS)   q.data[TWO] += (DATA_SIZE) cs_rfam2;
      if(THREE < NUM_PLOTS) q.data[THREE] += (DATA_SIZE) cs_gain;

      if(FOUR < NUM_PLOTS)  q.data[FOUR] += (DATA_SIZE) cs_coven;
      if(FIVE < NUM_PLOTS)  q.data[FIVE] += (DATA_SIZE) cs_qoven;

      if(SIX < NUM_PLOTS)   q.data[SIX] += (DATA_SIZE) cs_pll_dro;
      if(SEVEN < NUM_PLOTS) q.data[SEVEN] += (DATA_SIZE) cs_pll_saw;
      if(EIGHT < NUM_PLOTS) q.data[EIGHT] += (DATA_SIZE) cs_pll_87;
      if(NINE  < NUM_PLOTS) q.data[NINE] += (DATA_SIZE) cs_pll_up;

      q.data[TEN] += (DATA_SIZE) cs_beam;

      q.data[ELEVEN] += (DATA_SIZE) cs_cfield;
      q.data[TWELVE] += (DATA_SIZE) cs_hwi;
      q.data[THIRTEEN] += (DATA_SIZE) cs_msp;
      q.data[FOURTEEN] += (DATA_SIZE) cs_volt_avg;
   }
   else if(rcvr_type == LPFRS_RCVR) {
      if(ONE < NUM_PLOTS) q.data[ONE] += (DATA_SIZE) (lpfrs_dd);
      if(TWO < NUM_PLOTS) q.data[TWO] += (DATA_SIZE) (lpfrs_gg);
      if(THREE < NUM_PLOTS) q.data[THREE] += (DATA_SIZE) (lpfrs_hh);

      if(FOUR < NUM_PLOTS)  q.data[FOUR] += (DATA_SIZE) (lpfrs_ee);
      if(FIVE < NUM_PLOTS)  q.data[FIVE] += (DATA_SIZE) (lpfrs_cc);
      if(SIX < NUM_PLOTS)   q.data[SIX] += (DATA_SIZE) (lpfrs_bb);
      if(SEVEN < NUM_PLOTS) q.data[SEVEN] += (DATA_SIZE) (lpfrs_aa);
      if(EIGHT < NUM_PLOTS)  q.data[EIGHT] += (DATA_SIZE) (lpfrs_ff);
   }
   else if(rcvr_type == PRS_RCVR) {
      if(ONE < NUM_PLOTS) q.data[ONE] += (DATA_SIZE) (prs_ad[5]);
      if(TWO < NUM_PLOTS) q.data[TWO] += (DATA_SIZE) (prs_ad[6]);
      if(THREE < NUM_PLOTS) q.data[THREE] += (DATA_SIZE) (prs_ad[7]);

      if(FOUR < NUM_PLOTS)  q.data[FOUR] += (DATA_SIZE) (prs_ad[14]);
      if(FIVE < NUM_PLOTS)  q.data[FIVE] += (DATA_SIZE) (prs_ad_therm);
      if(SIX < NUM_PLOTS)   q.data[SIX] += (DATA_SIZE) (prs_ad_pwr);
      if(SEVEN < NUM_PLOTS) q.data[SEVEN] += (DATA_SIZE) (prs_ad[9]);
      if(EIGHT < NUM_PLOTS) q.data[EIGHT] += (DATA_SIZE) (prs_ad[8]);
      if(NINE  < NUM_PLOTS) q.data[NINE] += (DATA_SIZE) prs_ds1;
      if(TEN  < NUM_PLOTS)  q.data[TEN] += (DATA_SIZE) prs_ds2;
   }
   else if(rcvr_type == SA35_RCVR) {
      if(ONE < NUM_PLOTS) q.data[ONE] += (DATA_SIZE) (sa35_tec);
      if(TWO < NUM_PLOTS) q.data[TWO] += (DATA_SIZE) (sa35_dc);
      if(THREE < NUM_PLOTS) q.data[THREE] += (DATA_SIZE) (sa35_rf);

      if(FOUR < NUM_PLOTS)  q.data[FOUR] += (DATA_SIZE) (sa35_heater);
   }
   else if(rcvr_type == SRO_RCVR) {
      if(ONE < NUM_PLOTS) q.data[ONE] += (DATA_SIZE) (sro_hh);
      if(TWO < NUM_PLOTS) q.data[TWO] += (DATA_SIZE) (sro_ff);
      if(THREE < NUM_PLOTS) q.data[THREE] += (DATA_SIZE) (sro_ee);

      if(FOUR < NUM_PLOTS)  q.data[FOUR] += (DATA_SIZE) (sro_dd);
      if(FIVE < NUM_PLOTS)  q.data[FIVE] += (DATA_SIZE) (sro_cc);
      if(SIX < NUM_PLOTS)   q.data[SIX] += (DATA_SIZE) (sro_bb);
      if(SEVEN < NUM_PLOTS) q.data[SEVEN] += (DATA_SIZE) (sro_vt);
      if(EIGHT < NUM_PLOTS) q.data[EIGHT] += (DATA_SIZE) (sro_gg);
      if(NINE < NUM_PLOTS)  q.data[NINE] += (DATA_SIZE) (sro_aa); // !!!! reserved value
   }
   else if(rcvr_type == TICC_RCVR) {
      if(ONE < NUM_PLOTS)   q.data[ONE] += (DATA_SIZE) pps_phase;
      if(TWO < NUM_PLOTS)   q.data[TWO] += (DATA_SIZE) osc_phase;
      if(THREE < NUM_PLOTS) q.data[THREE] += (DATA_SIZE) chc_phase;
      if(FOUR < NUM_PLOTS)  q.data[FOUR] += (DATA_SIZE) chd_phase;

      if(FIVE < NUM_PLOTS)  q.data[FIVE] += (DATA_SIZE) cha_time2;
      if(SIX < NUM_PLOTS)   q.data[SIX] += (DATA_SIZE) chb_time2;
   }
   else if(1 && (rcvr_type == TIDE_RCVR)) {  // we also store tide data in lat/lon/alt/dop plots (along with plots 11/12/13/14)
      if(ONE < NUM_PLOTS)   q.data[ONE] += (DATA_SIZE) lat_tide;
      if(TWO < NUM_PLOTS)   q.data[TWO] += (DATA_SIZE) lon_tide;
      if(THREE < NUM_PLOTS) q.data[THREE] += (DATA_SIZE) alt_tide;
      if(SIX < NUM_PLOTS)   q.data[SIX] += (DATA_SIZE) ugals;
      have_lla_queue = 1;
   }
   else if(rcvr_type == UCCM_RCVR) {
      if(EIGHT < NUM_PLOTS) q.data[EIGHT] += (DATA_SIZE) pcorr;
   }
   else if(rcvr_type == X72_RCVR) {
      if(ONE < NUM_PLOTS) q.data[ONE] += (DATA_SIZE) x72_dmvoutc;
      if(TWO < NUM_PLOTS) q.data[TWO] += (DATA_SIZE) x72_dlvoutc;
      if(THREE < NUM_PLOTS) q.data[THREE] += (DATA_SIZE) x72_drvoutc;

      if(FOUR < NUM_PLOTS)  q.data[FOUR] += (DATA_SIZE) x72_dmv2demavg;
      if(FIVE < NUM_PLOTS)  q.data[FIVE] += (DATA_SIZE) x72_dlvolt;
      if(SIX < NUM_PLOTS)   q.data[SIX] += (DATA_SIZE) x72_dmp17;
      if(SEVEN < NUM_PLOTS) q.data[SEVEN] += (DATA_SIZE) x72_dmp5;
      if(EIGHT < NUM_PLOTS) q.data[EIGHT] += (DATA_SIZE) x72_pres;
      if(NINE < NUM_PLOTS)  q.data[NINE] += (DATA_SIZE) x72_plmp;
      if(TEN < NUM_PLOTS)   q.data[TEN]  += (DATA_SIZE) x72_dhtrvolt;
   }
   else if(rcvr_type == ZYFER_RCVR) {
      if(ONE < NUM_PLOTS)   q.data[ONE] += (DATA_SIZE) zyfer_hefe;
      if(TWO < NUM_PLOTS)   q.data[TWO] += (DATA_SIZE) zyfer_hete;
      if(THREE < NUM_PLOTS) q.data[THREE] += (DATA_SIZE) zyfer_hest;
      if(FOUR < NUM_PLOTS)  q.data[FOUR] += (DATA_SIZE) tfom;
      if(FIVE < NUM_PLOTS)  q.data[FIVE] += (DATA_SIZE) zyfer_drift;
      if(SIX < NUM_PLOTS)   q.data[SIX] += average_dop();
      if(SEVEN < NUM_PLOTS) q.data[SEVEN] += (DATA_SIZE) zyfer_tdev;
//    if(EIGHT < NUM_PLOTS) q.data[EIGHT] += (DATA_SIZE) zyfer_essn;
   }
   else if(graph_lla || TIMING_RCVR || LLA_RCVR) {
      if(ONE < NUM_PLOTS)   q.data[ONE] += (DATA_SIZE) (lat*RAD_TO_DEG);
      if(TWO < NUM_PLOTS)   q.data[TWO] += (DATA_SIZE) (lon*RAD_TO_DEG);
      if(THREE < NUM_PLOTS) q.data[THREE] += (DATA_SIZE) alt;

      if((lat*RAD_TO_DEG) < min_q_lat) min_q_lat = (lat*RAD_TO_DEG);
      if((lat*RAD_TO_DEG) > max_q_lat) max_q_lat = (lat*RAD_TO_DEG);
      if((lon*RAD_TO_DEG) < min_q_lon) min_q_lon = (lon*RAD_TO_DEG);
      if((lon*RAD_TO_DEG) > max_q_lon) max_q_lon = (lon*RAD_TO_DEG);
      have_lla_queue = 1;

      if((rcvr_type == SCPI_RCVR) || (rcvr_type == UCCM_RCVR) || (rcvr_type == TRUE_RCVR)) {
         if(FOUR < NUM_PLOTS)  q.data[FOUR] += (DATA_SIZE) tfom;
         if(rcvr_type == TRUE_RCVR) {
            if(FIVE < NUM_PLOTS)  q.data[FIVE] += (DATA_SIZE) true_debug;
            if(EIGHT < NUM_PLOTS) q.data[EIGHT] += (DATA_SIZE) true_eval;
         }
         else {
            if(FIVE < NUM_PLOTS)  q.data[FIVE] += (DATA_SIZE) ffom;
         }

         if(rcvr_type == UCCM_RCVR) {
            if(SEVEN < NUM_PLOTS) q.data[SEVEN] += (DATA_SIZE) (ant_v1);
         }
      }
      else if(rcvr_type == BRANDY_RCVR) {
         if(SEVEN < NUM_PLOTS) q.data[SEVEN] += (DATA_SIZE) (freq_trend*1.0E12);
         if(EIGHT < NUM_PLOTS) q.data[EIGHT] += (DATA_SIZE) (avg_phase);
      }
      else if(rcvr_type == RFTG_RCVR) {
         if(FIVE < NUM_PLOTS)  q.data[FIVE] += (DATA_SIZE) rftg_oscv;
         if(SEVEN < NUM_PLOTS) q.data[SEVEN] += (DATA_SIZE) (ant_v1);
         if(EIGHT < NUM_PLOTS) q.data[EIGHT] += (DATA_SIZE) (ant_v2);
      }
      else {
         if(FOUR < NUM_PLOTS)  q.data[FOUR] += (DATA_SIZE) speed;
         if(FIVE < NUM_PLOTS)  q.data[FIVE] += (DATA_SIZE) heading;
      }

      if((rcvr_type == RFTG_RCVR) || (rcvr_type == UCCM_RCVR)) {  // we use DOP plot for antenna current
         if(SIX < NUM_PLOTS) q.data[SIX] += (DATA_SIZE) ant_ma;
      }
      else {
         if(SIX < NUM_PLOTS) q.data[SIX] += average_dop();
      }
   }
   else {
      if(initial_voltage == (DATA_SIZE) 0.0) v = dac_voltage;
      else                        v = initial_voltage;
      if(luxor) {  // DERIVED_PLOTS !!!!
         if(LUX2 < NUM_PLOTS)    q.data[LUX2] += (DATA_SIZE) lux2;        // ONE
         if(LEDV < NUM_PLOTS)    q.data[LEDV] += (DATA_SIZE) led_v;       // TWO
         if(LEDI < NUM_PLOTS)    q.data[LEDI] += (DATA_SIZE) led_i;       // THREE
         if(PWMHZ < NUM_PLOTS)   q.data[PWMHZ] += (DATA_SIZE) pwm_hz;     // FOUR
         if(TC2 < NUM_PLOTS )    q.data[TC2] += (DATA_SIZE) luxor_tc2;    // FIVE

         if(BLUEHZ < NUM_PLOTS)  q.data[BLUEHZ] += (DATA_SIZE) blue_hz;   // SIX  // color sensor
         if(GREENHZ < NUM_PLOTS) q.data[GREENHZ] += (DATA_SIZE) green_hz; // SEVEN
         if(REDHZ < NUM_PLOTS)   q.data[REDHZ] += (DATA_SIZE) red_hz;     // EIGHT
         if(WHITEHZ < NUM_PLOTS) q.data[WHITEHZ] += (DATA_SIZE) white_hz; // NINE
         if(AUXV < NUM_PLOTS)    q.data[AUXV] += (DATA_SIZE) adc2;        // TEN
         // ELEVEN = BATTW
         // TWELVE = LEDW
         // THIRTEEN = EFF
         // FOURTEEN = CCT
      }
      else {
         if(ONE < NUM_PLOTS) q.data[ONE] += (DATA_SIZE) ((dac_voltage-v)*osc_gain*((DATA_SIZE)1000.0));
         if(TWO < NUM_PLOTS) q.data[TWO] = (DATA_SIZE) ((q.data[OSC]*(DATA_SIZE) 1000.0)-q.data[ONE]);
      }
   }

   if(luxor) ;
   else if(rcvr_type == CS_RCVR) ;
   else if(USES_JIT_PLOTS) goto tide_plots; // device uses plots [NINE] and [TEN]
   else {
      jitter = (DATA_SIZE) (this_time_msec - last_time_msec);  // timing message jitter
      if((rcvr_type == SCPI_RCVR) || (rcvr_type == UCCM_RCVR)) {  // !!!! kludge around STATUS messaage
         if(jitter > (DATA_SIZE) 1800.0) jitter = (DATA_SIZE) 1000.0;
      }

      q.data[MSGJIT] += jitter;                // [NINE]
      q.data[MSGOFS] += (DATA_SIZE) msg_ofs;   // [TEN]

      tide_plots:
      if(enviro_mode() && (rcvr_type != TIDE_RCVR)) {
         q.data[HUMIDITY] += (DATA_SIZE) humidity;
         q.data[PRESSURE] += (DATA_SIZE) pressure;
         q.data[TEMP1] += (DATA_SIZE) tc1;
         q.data[TEMP2] += (DATA_SIZE) tc2;
      }
      else if(plot_prn) {
         q.data[ELEVEN] += (DATA_SIZE) sat[plot_prn].azimuth;
         q.data[TWELVE] += (DATA_SIZE) sat[plot_prn].elevation;
         sig = sat[plot_prn].sig_level;
         if((sig < 0.0) && (plot_prn <= MAX_PRN)) sig = (-sig);
         q.data[THIRTEEN] += sig;
         if(have_phase)      q.data[FOURTEEN] += (DATA_SIZE) sat[plot_prn].code_phase;
         else if(have_range) q.data[FOURTEEN] += (DATA_SIZE) sat[plot_prn].range;
         else                q.data[FOURTEEN] += (DATA_SIZE) ugals;
      }
      else {
         q.data[ELEVEN] += (DATA_SIZE) lat_tide;
         q.data[TWELVE] += (DATA_SIZE) lon_tide;
         q.data[THIRTEEN] += (DATA_SIZE) alt_tide;
         q.data[FOURTEEN] += (DATA_SIZE) ugals;
      }
   }

   if(FFT < NUM_PLOTS+DERIVED_PLOTS) q.data[FFT] = (DATA_SIZE) 0.0;

   q.sat_flags &= SAT_FLAGS;       // preserve flag bits
   if(sat_count > SAT_COUNT_MASK) q.sat_flags |= SAT_COUNT_MASK;   // satellite count
   else                           q.sat_flags |= sat_count;
   q.sat_flags |= (new_const & CONST_CHANGE);            // constellation change
   if(time_flags & TFLAGS_UTC) q.sat_flags |= UTC_TIME;  // UTC/GPS time flag

//if((seconds%10) == 9) q.sat_flags |= TIME_SKIP;

   if((discipline_mode == DIS_MODE_DISABLED) && osc_control_on) ;
   else if(discipline_mode != DIS_MODE_NORMAL) {
      q.sat_flags |= HOLDOVER;
   }

   if(spike_delay) q.sat_flags |= TEMP_SPIKE;

   if(flag_faults && (msg_fault || tsip_error)) q.sat_flags |= TIME_SKIP;

   q.q_jd = jd_utc;

   put_plot_q(plot_q_in, q);

   new_const = 0;

   if(++plot_time < queue_interval) {  // not yet time to draw
      if(draw_flag) refresh_page();    // make sure text parts of screen keep updating
      return;
   }
   plot_time = 0;

   if(review_mode == 0) {
      review = plot_q_count;
   }

   ++view_time;
//!!!!!!!!!! if((view_time >= view_interval) && ((review_mode == 0) || (plot_column < (PLOT_WIDTH-plot_mag)))) {
//!!!!!!!!!! if((view_time >= view_interval) && ((review_mode == 0) || view_all_data || (plot_column < (PLOT_WIDTH-plot_mag)))) {
   if(((view_time >= view_interval) || (view_all_data == 2)) && ((review_mode == 0) || (view_all_data == 2) || (plot_column < (PLOT_WIDTH-plot_mag)))) {
      if(plot_q_in == plot_q_col0) last_q = q;
      add_stat_point(&q);
      last_q = q;
      if(view_all_data == 2) ;
      else if(draw_flag == NO_REFRESH) goto scroll_it;

      plot_entry(plot_q_in);  // plot latest queue entry
      plot_column += plot_mag;
      if(plot_column >= PLOT_WIDTH) {   // it's time to scroll the plot left
         if(view_all_data == 2) {
            view_all(1);
         }
         else {
            plot_column -= (PLOT_SCROLL*plot_mag);
            plot_start = plot_q_in - (((PLOT_WIDTH - PLOT_SCROLL)*view_interval)/plot_mag) - plot_q_out;
            if(plot_start < 0) {
                if(plot_q_full) {
                   plot_start += plot_q_size;
                   if(plot_start < 0) plot_start = 0;
                }
                else plot_start = 0;
            }
            draw_plot(NO_REFRESH);
         }
      }
      else if(off_scale) {  // a graph is now off scale,  redraw the plots to rescale it
         draw_plot(NO_REFRESH);
         off_scale = 0;
      }
      else if(continuous_scroll) draw_plot(NO_REFRESH);
   }
   else {
      scroll_it:
      if(continuous_scroll) draw_plot(NO_REFRESH);
   }
   if(view_time >= view_interval) view_time = 0;


   // prepare queue for next point
   if(++plot_q_count >= plot_q_size) plot_q_count = plot_q_size;
   if(++plot_q_in >= plot_q_size) {
      plot_q_in = 0;
   }
   if(plot_q_in == plot_q_out) {  // plot queue is full
      if(++plot_q_out >= plot_q_size) plot_q_out = 0;
      plot_q_count = plot_q_size;
      plot_q_full = 1;
   }
   clear_plot_entry((long) plot_q_in);

   if(draw_flag) {
      show_title();    // rrrrr
      refresh_page();
   }
}


void redraw_screen()
{
   // redraw everything on the screen
   if(first_key) {  // we are showing a keyboard menu show hold off on the redraw
      return;
   }

   need_redraw = 0;
   last_plot_tick = (-1);

   precise_survey_done = 0;
   if((check_precise_posn == 0) && (precision_survey == 0) && (show_fixes == 0)) {
      plot_lla = 0;
////  if(doing_survey == 0) plot_lla = 0;
   }

   erase_screen();
   show_log_state();
//!!!!! request_rcvr_info(202);
   request_version();
   if(plot_azel) update_azel = 1;

   draw_plot(REFRESH_SCREEN);  // redraw the plot area
}






void zoom_zoomed()
{
   erase_screen();

   if(zoom_screen == 'P') {
      goto unzoom;
   }
   else if(zoom_screen == '`') {
      show_rpn_help = 0;
      goto unzoom;
   }
   else if((rcvr_type == CS_RCVR) && (zoom_screen == 'D')) {
      goto unzoom;
   }
   else if((rcvr_type == RFTG_RCVR) && (zoom_screen == 'D')) {
      goto unzoom;
   }
   else if((rcvr_type == X72_RCVR) && (zoom_screen == 'D')) {
      goto unzoom;
   }
   else if((un_zoom == ZOOM_SHARED) || (un_zoom == ZOOM_LLA) || (un_zoom == ZOOM_AZEL) || (un_zoom == ZOOM_PLOT)) {
      unzoom:
      if(un_zoom == ZOOM_LLA) erase_screen();
      un_zoom = UN_ZOOM;
      change_zoom_config(987);
      cancel_zoom(5);
      zoom_fixes = show_fixes;
      config_screen(117);
      return;
   }
   else if(un_zoom == ZOOM_INFO) {
      goto unzoom;
   }
   else if(un_zoom == ZOOM_CLOCK) {
      show_fixes = 0;
      config_fix_display();
      goto unzoom;
   }
   else if(un_zoom == ZOOM_ADEVS) {
//sprintf(debug_text2, "plot watch unzoom:%d", plot_watch);
      if(rcvr_type == TICC_RCVR) all_adevs = ALL_CHANS;
      else                       all_adevs = SINGLE_ADEVS;
      plot_adev_data = old_plot_adevs;
      goto unzoom;
   }
   else if(un_zoom && (un_zoom != 'L')) {
      zoom_screen = un_zoom;
      un_zoom = UN_ZOOM;
      return;
   }

   un_zoom = zoom_screen;
   if(zoom_screen == 'U') {  // all signals
//un_zoom = UN_ZOOM;
      zoom_screen = 'S';
      if(mouse_x > SCREEN_WIDTH/2) {
         if(mouse_y >= (SCREEN_HEIGHT/2)) plot_signals = 4;
         else                             plot_signals = 2;
      }
      else {
         if(mouse_y >= (SCREEN_HEIGHT/2)) plot_signals = 3;
         else                             plot_signals = 1;
      }
      config_screen(7904);
   }
   else if(zoom_screen == 'V') { // signals and maps
//un_zoom = UN_ZOOM;
      if(mouse_y > SCREEN_HEIGHT/2) {
         zoom_screen = 'S';
         if(mouse_x >= (SCREEN_WIDTH/2)) plot_signals = 4;
         else                            plot_signals = 2;
      }
      else {
         if(mouse_x >= (SCREEN_WIDTH/2)) {
            zoom_screen = 'M';
         }
         else {
            zoom_screen = 'W';
         }
         config_screen(7903);
      }
   }
   else if(zoom_screen == 'I') { // signals and maps
//un_zoom = UN_ZOOM;
      if(mouse_x < (MINOR_COL*TEXT_WIDTH)) {
         zoom_screen = 'U';
      }
      else if(mouse_y < (SCREEN_HEIGHT/2)) {
         zoom_screen = 'M';
      }
      else {
         plot_signals = 4;
         zoom_screen = 'S';
      }
      config_screen(7903);
   }
   else if(zoom_screen == 'S') {  // signals
      zoom_screen = 'U';
   }
   else if(zoom_screen == 'M') {  // sat map
      zoom_screen = 'V';
   }
   else if(zoom_screen == 'C') {  // digital clock
      zoom_screen = 'W';
   }
   else if(zoom_screen == 'W') {  // watch
      zoom_screen = 'C';
      config_screen(7903);
   }
   else if(zoom_screen == 'B') {  // map and watch
      zoom_screen = 'V';
   }
   else if(zoom_screen == 'Y') {  // map and sigs
      zoom_screen = 'V';
   }
   else if(zoom_screen == 'L') {  // fix map
      change_zoom_config(-3);
      cancel_zoom(6);      //zkzk
      show_fixes = 0;
if(rcvr_type != NO_RCVR) show_fixes = zoom_fixes;  // zzzzzz
      change_fix_config(show_fixes);
//    plot_lla = 0;

      no_redraw = 1;
      config_screen(128);
      erase_screen();
      no_redraw = 0;

need_redraw = 5791;
   }
}


void config_lla_zoom(int why)
{
    if(rcvr_type == CS_RCVR) un_zoom = UN_ZOOM;

    change_zoom_config(4);

    if(rcvr_type == CS_RCVR) {
       zoom_screen = 'D';
    }
    else {
       zoom_screen = 'L';
       zoom_fixes = show_fixes;
       show_fixes = 1;
       change_fix_config(show_fixes);
       if(show_fixes && (precision_survey == 0)) {  // set reference position for fix map
          precise_lat = lat;
          precise_lon = lon;
          precise_alt = alt;
          have_precise_lla = (-1);
          if(user_set_ref_lla == 0) {
             ref_lat = lat;
             ref_lon = lon;
             ref_alt = alt;
          }
          cos_factor = cos(ref_lat);
          if(cos_factor == 0.0) cos_factor = 0.001;
       }
       start_3d_fixes(-1, 3);  // !!!!! only if show_fixes == 1?
    }

    config_screen(113);
}


int do_touch_key()
{
int key;

    // process a touch keyboard key click
    key = touch_key(mouse_x,mouse_y);
    if(key == TOUCH_SH) {  // shift key pressed - shift next keystroke
       shifted ^= 1;
    }
    else if(key == TOUCH_BUT) {  // right click mode enabled
       button_lock ^= 1;
       if(button_lock) {
          if(lc_but_lock_index) normal_lc_table[lc_but_lock_index].label = "MOU";
          if(lc_but_lock_index) calc_lc_table[lc_but_lock_index].label = "MOU";
          if(uc_but_lock_index) uc_table[uc_but_lock_index].label = "MOU";
       }
       else {
          if(lc_but_lock_index) normal_lc_table[lc_but_lock_index].label = "mou";
          if(lc_but_lock_index) calc_lc_table[lc_but_lock_index].label = "mou";
          if(uc_but_lock_index) uc_table[uc_but_lock_index].label = "mou";
       }
    }
    else if(key == TOUCH_CAP) { // caps lock pressed
       caps_lock ^= 1;
       if(caps_lock) {
          if(lc_cap_lock_index) normal_lc_table[lc_cap_lock_index].label = "CAP";
          if(lc_cap_lock_index) calc_lc_table[lc_cap_lock_index].label = "CAP";
          if(uc_cap_lock_index) uc_table[uc_cap_lock_index].label = "CAP";
       }
       else {
          if(lc_cap_lock_index) normal_lc_table[lc_cap_lock_index].label = "cap";
          if(lc_cap_lock_index) calc_lc_table[lc_cap_lock_index].label = "cap";
          if(uc_cap_lock_index) uc_table[uc_cap_lock_index].label = "cap";
       }
       shifted = 0;
    }
    else if(key) {    // text key pressed
       add_kbd(key);  // add key to keystroke buffer
       shifted = 0;   // reset shift state
    }
    else {   // touched a blank key
       last_null_key = last_zoom_calc;
       hide_kbd(99);
       add_kbd(0);
    }

    if(key == '?') last_touch_key = key;
    else if((key == 0x0D) && (last_touch_key == '?')) last_touch_key = 999;
    else last_touch_key = 0;

    show_touch_kbd(0);
    return key;
}


void do_plot_click(int click)
{
static int mouse_sleeps = 0;
int right_button;
S32 i;

#define MOUSE_SLEEP  10    // msec delay between testing for next mouse state while right_click scrolling
#define SCROLL_SERVE 200   // serve_gps() every this many msecs while right_click scrolling

   // process a click in the plot area

   mouse_x -= PLOT_COL;
   if((mouse_x >= 0) && (mouse_x < plot_column) && (mouse_y >= PLOT_ROW) && (mouse_y < (PLOT_ROW+PLOT_HEIGHT))) { // mouse is in the plot area
//    show_last_mouse_x(WHITE);      // lmq
      last_plot_mouse_x = mouse_x;
      show_last_mouse_x(GREEN);      // lmq
      if(last_mouse_x != mouse_x) new_mouse_info |= 0x01;
      else if(last_mouse_y != mouse_y) new_mouse_info |= 0x01;

      i = plot_q_col0 + ((view_interval * mouse_x)/plot_mag);
      while(i >= plot_q_size) i -= plot_q_size;
      if(i != last_mouse_q) last_plot_glitch = i;  // mouse was moved
      last_mouse_q = last_plot_mouse_q = i;
      mouse_plot_valid = 1;  // mouse is in the plot area

      right_button = 2;
      if(touch_screen && button_lock) right_button = 1;  // change left button to right button

      if((this_button == right_button) && (last_button == 0)) {  // mark plot and center on it
         view_all_data = 0;
         need_view_auto = 0;
         right_time = 0;

         right_click:
         mark_q_entry[0] = last_mouse_q;  // mark the point

         no_mouse_review = 1; // inhibit recursive call to get_mouse_info
         goto_mark(0);        // center plot on the marked point
         no_mouse_review = 0;
      }
      else if((this_button == right_button) && (last_button == right_button)) {  // scroll plot with right mouse button
         Sleep(MOUSE_SLEEP);
         ++mouse_sleeps;

         // keep gps processing alive approx every 200 msecs while right mouse button scrolling
         if(enable_timer == 0) ;
         else if(timer_serve);
         else if((mouse_sleeps % (SCROLL_SERVE/MOUSE_SLEEP)) == 0) {
            #ifdef WINDOWS
               // windows timer proc does this for us
            #else
               ++timer_serve;
               serve_gps(55);
               --timer_serve;
            #endif  // __linux__  __MACH__   __FreeBSD__
         }

         if(++right_time >= RIGHT_DELAY) goto right_click;
      }
      else if(click) {  // zoom
         view_all_data = 0;
         need_view_auto = 0;
         right_time = 0;
         kbd_zoom();
      }
      else right_time = 0;
   }
}


int showing_cmd_help;


void do_mouse_click()
{
int click;
int c;

   // process mouse clicks on the screen

   if(mouse_x < 0) return;
   else if(mouse_x >= SCREEN_WIDTH) return;
   else if(mouse_y < 0) return;
   else if(mouse_y >= SCREEN_HEIGHT) return;
// else if(first_key && (zoom_screen != 'K')) return;  // no zooms if keyboard menu showing

//if(zoom_screen == 'K') ;     // inhibt mouse clicks if menu shown
//else if(first_key) return;

   if((this_button == 1) && (last_button == 0)) click = 1;  // left mouse button clicked
   else click = 0;

   if(click && (text_mode == 2)) {  // any click exits keyboard help menu
      click_sound(1);
      add_kbd(' ');
   }
   else if(click && showing_cmd_help) { // any click exits command line help
      click_sound(1);
      add_kbd(' ');
   }
   else if((zoom_screen == 'K') && click && (mouse_y < PLOT_ROW)) {
      // click on touch screen keyboard
      do_touch_key();
   }
   else if((zoom_screen == '`') && click && (mouse_y < CORNER_SIZE) && (mouse_x < CORNER_SIZE)) {
      // switch from zoomed calculator to touch screen keyboard
      click_sound(333);
      rpn_mode = (-1);
      zoom_screen = 'K';
      show_touch_kbd(1);
      last_zoom_calc = 1;
      show_rpn_help = 0;
   }
   else if(zoom_screen && click && (mouse_y < CORNER_SIZE) && (mouse_x < CORNER_SIZE)) {
      // un-zoom plot display
      un_lla:
      click_sound(2);
      add_kbd(ESC_CHAR);
   }
   else if(touch_screen && click && (mouse_y < CORNER_SIZE) && (mouse_x < CORNER_SIZE)) {
       // show touch screen keyboard
       click_sound(3);
       if(0 && (first_key == 0)) {
          add_kbd('z');
          add_kbd('k');
       }
       else {
          zoom_screen = 'K';
          show_touch_kbd(1);
       }
   }
// else if(touch_screen && last_plot_key && click && (mouse_y < CORNER_SIZE) && (mouse_x > CORNER_SIZE) && (mouse_x < (VER_COL*TEXT_WIDTH))) {
   else if(touch_screen && click && (mouse_y < CORNER_SIZE) && (mouse_x > CORNER_SIZE) && (mouse_x < (VER_COL*TEXT_WIDTH*vc_font_scale/100))) {
      // repeat last plot control key
      if(last_plot_key) {   // repeat last plot control key
         click_sound(22);
         add_kbd(last_plot_key);
      }
      else if(unit_file_name) {   // do a screen dump
         dump_screen(invert_dump, 0, unit_file_name);

         erase_screen();
         sprintf(out, "Captured screen to file: %s.gif", unit_file_name);
         vidstr(0,0, YELLOW, out);
         refresh_page();
         need_redraw = 6586;
         Sleep(1000);
      }
   }
   else if((zoom_screen == 'F') && click) {
      // click anywhere on zoomed FFT waterfall screen
      goto un_lla;
   }
   else if((zoom_screen == 'L') && click) {
      // click anywhere on zoomed LLA screen
      goto un_lla;
   }
   else if((zoom_screen == 'O') && click) {
      // click anywhere on monitor mode screen
      goto un_lla;
   }
   else if((zoom_screen == 'Q') && click) {
      if(mouse_y > (SCREEN_HEIGHT-CORNER_SIZE)) {  // bottom left corner
         if(mouse_x < CORNER_SIZE) {
            click_sound(66);
            --cal_month;
            if(cal_month < 1) {
               cal_month = 12;
               --cal_year;
            }
         }
         else if(mouse_x > (SCREEN_WIDTH-CORNER_SIZE)) {  // bottom right corner
            click_sound(67);
            ++cal_month;
            if(cal_month > 12) {
               cal_month = 1;
               ++cal_year;
            }
         }
      }
      else goto un_lla;
   }
   else if((zoom_screen == '`') && click) {
      // click anywhere on calculator mode screen
      click_sound(222);
      rpn_mode = 0;
      getting_string = 0;
      zoom_zoomed();   // remove_zoom();
      if(edit_buffer[0]) {
         add_kbd(ESC_CHAR);
         add_kbd(0x0D);
      }
      add_kbd(' ');
//    add_kbd('z');   // boobs
//    add_kbd('z');
//    add_kbd(0x0D);
   }
   else if((un_zoom == ZOOM_ADEVS) && click) {  // un-zoom all_adevs display
      click_sound(4);
      zoom_zoomed();
      if(adevs_active(0)) last_was_adev = 5;
   }
   else if(zoom_screen && click && (zoom_screen != 'P')) {  // un-zoom display
      click_sound(5);
      zoom_zoomed();
   }
   else if((zoom_screen == 'P') && click && (mouse_y < (PLOT_ROW-TEXT_HEIGHT))) {
      // un-zoom plot display
      click_sound(6);
      if(mouse_x > (SCREEN_WIDTH-CORNER_SIZE)) {
//       add_kbd(DEL_CHAR);  // causes exit plot review and zoom screen modes
         end_review(1);      // just exit plot review mode
      }
      else zoom_zoomed();
   }
   else if((zoom_screen == 0) && show_fixes && click && (mouse_y < LLA_SIZE) && (mouse_x > (SCREEN_WIDTH-LLA_SIZE))) {
      // zoom fix map
      click_sound(7);
      if(first_key) {  // keyboard menu is on screen, cancel it
         first_key = 0;
         getting_string = 0;
         return;
      }
      goto fixes;
   }
   else if(rcvr_type == NO_RCVR) {
      if(click) {
         click_sound(8);
         if(first_key) {  // keyboard menu is on screen, cancel it
            first_key = 0;
            getting_string = 0;
            return;
         }

         if((mouse_y > (SCREEN_HEIGHT/2))) {  // zoom clock
            un_zoom = ZOOM_CLOCK;
            zoom_screen = 'C';
            zoom_fixes = show_fixes;
            config_screen(124);
         }
         else if(mouse_x > (SCREEN_WIDTH/2)) {
            change_zoom_config(89);
            un_zoom = ZOOM_AZEL;
            zoom_screen = 'W';
            zoom_fixes = show_fixes;
            config_screen(124);
         }
      }
   }
   else if((zoom_screen == 0) && share_active && shared_item && click && (mouse_y >= PLOT_ROW) && (mouse_x > (PLOT_COL+PLOT_WIDTH))) {
      // zoom shared plot item
      click_sound(9);
      if(first_key) {  // keyboard menu is on screen, cancel it
         first_key = 0;
         getting_string = 0;
         return;
      }

      if(mac_os && (mouse_x >= (SCREEN_WIDTH-48))) ;        // needed for XQuartz resize box
      else if(mac_os && (mouse_y >= (SCREEN_HEIGHT-48))) ;
      else {
         change_zoom_config(88);
         un_zoom = ZOOM_SHARED;
         if(shared_item == SHARE_WATCH) zoom_screen = 'W';
         else if(shared_item == SHARE_MAP) zoom_screen = 'M';
         else if(shared_item == (SHARE_MAP | SHARE_WATCH)) zoom_screen = 'B';
         else if(shared_item == SHARE_SIGNALS) zoom_screen = 'S';
         else if(shared_item == SHARE_LLA) zoom_screen = 'L';
         else cancel_zoom(7);
         zoom_fixes = show_fixes;
         config_screen(124);
      }
   }
   else if((zoom_screen == 0) && click && adevs_shown && (mouse_y < (ADEV_ROW+(max_adev_rows+2)*2)*TEXT_HEIGHT) && (mouse_x > (left_adev_col*TEXT_WIDTH)) && (mouse_x < ((ADEV_COL+adevs_shown)*TEXT_WIDTH)) ) {
      // zoom adev tables to all_adevs screen
      click_sound(19);
      if(first_key) {  // keyboard menu is on screen, cancel it
         first_key = 0;
         getting_string = 0;
         return;
      }

      change_zoom_config(88);
      un_zoom = ZOOM_ADEVS;
      if(osc_adev_q_count == 0) all_adevs = ALL_PPS;
      else if(pps_adev_q_count == 0) all_adevs = ALL_OSC;
      else if(mouse_y < ((ADEV_ROW+max_adev_rows+2)*TEXT_HEIGHT)) all_adevs = ALL_PPS; // PPS
      else all_adevs = ALL_OSC;  // OSC adevs  // aaaahhhh what about ALL_CHC and ALL_CHD

      old_plot_adevs = plot_adev_data;
      plot_adev_data = 1;
      config_screen(1124);
      if(adevs_active(0)) last_was_adev = 6;
   }
// else if((zoom_screen == 0) && click && (mouse_y < (ACLOCK_R*2)) && (mouse_x > ((ADEV_COL+adevs_shown)*TEXT_WIDTH))) {
   else if((zoom_screen == 0) && click && (mouse_y < (SCREEN_HEIGHT/2)) && (mouse_x > ((ADEV_COL+adevs_shown)*TEXT_WIDTH))) {
      // zoom map/watch
      click_sound(10);
      if(first_key) {  // keyboard menu is on screen, cancel it
         first_key = 0;
         getting_string = 0;
         return;
      }

      fixes:
      if(all_adevs == SINGLE_ADEVS) {
         change_zoom_config(888);
         un_zoom = ZOOM_AZEL;

         if(show_fixes && (shared_item != SHARE_LLA)) zoom_screen = 'L';  // zoom the scattergram
         else if(plot_watch && plot_azel) {      // zoom watch and/or map
            if(shared_plot && (shared_item == SHARE_MAP)) zoom_screen = 'W';
            else if(shared_plot && (shared_item == SHARE_WATCH)) zoom_screen = 'M';
            else zoom_screen = 'B';
         }
         else if(plot_watch && (shared_item != SHARE_WATCH)) zoom_screen = 'W';
         else if(plot_azel && (shared_item != SHARE_MAP)) zoom_screen = 'M';
         else if(plot_signals && (shared_item != SHARE_SIGNALS)) zoom_screen = 'S';
         else cancel_zoom(8);
         zoom_fixes = show_fixes;
         config_screen(124);
      }
   }
   else if((zoom_screen == 0) && click && (mouse_y < ((MOUSE_ROW+4)*TEXT_HEIGHT)) && (mouse_y >= ((POSN_ROW+2)*TEXT_HEIGHT)) && (mouse_x < (MINOR_COL*TEXT_WIDTH))) {
      // zoom clock or sat info

      if(all_adevs) ;
      else if(mouse_y <= ((POSN_ROW+6)*TEXT_HEIGHT)) {
         // lat/lon/alt clicked, show scattergram
         if(mouse_x <= ((SURVEY_COL+2)*TEXT_WIDTH)) {
            click_sound(111);
            if(first_key) {  // keyboard menu is on screen, cancel it
               first_key = 0;
               getting_string = 0;
               return;
            }

            if(show_fixes) un_zoom = ZOOM_LLA;
            else           un_zoom = ZOOM_CLOCK;
            config_lla_zoom(1);
         }
      }
      else if(sun_moon_shown && (mouse_x >= ((sun_moon_shown+2)*TEXT_WIDTH)) && (mouse_y < ((last_sat_row-0)*TEXT_HEIGHT))) {
         // mouse is past the sat info data
      }
      else {  // mouse in in the sat info table
         click_sound(112);
         if(first_key) {    // keyboard menu is on screen, cancel it
            first_key = 0;
            getting_string = 0;
            return;
         }

         un_zoom = ZOOM_INFO;
         change_zoom_config(88);

         c = 'I';
         if(rcvr_type == RFTG_RCVR) c = 'D';
         if((mouse_x < vc_col) && plot_digital_clock) zoom_screen = c;
         else if(mouse_y < ((last_sat_row-0)*TEXT_HEIGHT)) zoom_screen = c;
         else zoom_screen = 'C';

         if((rcvr_type == CS_RCVR) && (zoom_screen == 'I')) {
            zoom_screen = 'D';
         }
         else {
            zoom_fixes = show_fixes;
         }
         config_screen(124);
      }
   }
   else if((zoom_screen == 0) && click && (mouse_y >= ((MOUSE_ROW+4)*TEXT_HEIGHT)) && (mouse_y < ((PLOT_ROW-TEXT_HEIGHT)))) {
      // zoom plot
      click_sound(12);
      if(first_key) {  // keyboard menu is on screen, cancel it
         first_key = 0;
         getting_string = 0;
         return;
      }

      un_zoom = 'P';
      change_zoom_config(89);
      zoom_screen = 'P';
      zoom_fixes = show_fixes;
      config_screen(124);
      force_adev_redraw(2);
   }
   else {
      do_plot_click(click);
   }
}


int get_mouse_info()
{
u08 new_button;
float val;

   // this routine shows the queue data at the mouse cursor in the plot window
   if(queue_interval <= 0) return 0;
   if(timer_serve) return 0;  // don't let windows timer access this routine

// mouse_plot_valid = 0;
   new_mouse_info = new_button = 0;

   // get mouse coordinates here
#ifdef WIN_VFX
   serve_vfx();

   if(SAL_is_app_active()) {
      SAL_WINAREA wndrect;
      SAL_client_area(&wndrect);    // Includes menu if any

      POINT cursor;
      GetCursorPos(&cursor);

      mouse_x = cursor.x - wndrect.x;
      mouse_y = cursor.y - wndrect.y;
      // adjust position in case screen area is scaled down
      if(wndrect.w && wndrect.h) {
         SWAP(wndrect.w,wndrect.h);
         val = ((float) mouse_x * (float) SCREEN_WIDTH) / (float) wndrect.w;
         mouse_x = (int) val;
         val = ((float) mouse_y * (float) SCREEN_HEIGHT) / (float) wndrect.h;
         mouse_y = (int) val;
      }
      SWAPXY(mouse_y,mouse_x);  // note: y,x and not x,y

      last_button = this_button;
      if     (GetKeyState(VK_LBUTTON) & 0x8000) this_button = 1;
      else if(GetKeyState(VK_RBUTTON) & 0x8000) this_button = 2;
      else                                      this_button = 0;

      if(mouse_disabled) return 0;
#endif


#ifdef USE_X11
   {
Window root,child;
int rx,ry;
int wx,wy;
unsigned bmask;

      if(mouse_disabled) return 0;
      if(display == 0) return 0;
      if(my_mouse == 0) return 0;

      if(!XQueryPointer(display,screen, &root,&child, &rx,&ry, &wx,&wy, &bmask)) return 0;
      mouse_x = wx;
      mouse_y = wy;
      SWAPXY(mouse_y,mouse_x);  // note: y,x and not x,y

      #ifndef PRESLEY
         last_button = this_button;
         if(bmask & Button1Mask) this_button = 1;
else if(crap_mouse && (bmask & (Button2Mask | Button3Mask))) this_button = 1;    // testing PI 480x320 touchscreen
         else if(bmask & Button3Mask) this_button = 2;
         else this_button = 0;

         if(last_button != this_button) {
            new_button |= 0x02;
         }
      #endif

      val = 0.0F;
#endif

#ifdef USE_SDL
   {
      	if(mouse_disabled) return 0;
        int button = SDL_GetMouseState(&mouse_x, &mouse_y);

        float scale_x = ((float)sdl_texture_width / (float)sdl_window_width);
        float scale_y = ((float)sdl_texture_height / (float)sdl_window_height);

        mouse_x = ((float)mouse_x) * scale_x;
        mouse_y = ((float)mouse_y) * scale_y;

	    last_button = this_button;

   		if(button & SDL_BUTTON(SDL_BUTTON_LEFT)) {
   			this_button = 1;
   		} else if(button & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
   			this_button = 2;
   		} else {
   			this_button = 0;
   		}


        if(last_button != this_button) {
           new_button |= 0x02;
	    }

	    val = 0.0F;
#endif // USE_SDL

      if(this_button && (touch_screen == 0)) {
         view_all_data = 0;
         need_view_auto = 0;
      }

//sprintf(plot_title, "mx:%d  my:%d  pcol:%d  prow:%d", mouse_x,mouse_y, plot_column,PLOT_ROW);
//sprintf(debug_text, "mouse:%d %d   azel_size:%d  aclock_r:%d  maw:%d", mouse_x,mouse_y, AZEL_SIZE, ACLOCK_R, map_and_watch);
//sprintf(debug_text, "adevs shown: %d", adevs_shown);

      mouse_plot_valid = 0;  // assume mouse is not in plot area
      do_mouse_click();      // check mouse position and button clicks

      last_mouse_x = mouse_x;
      last_mouse_y = mouse_y;
   }


   if(new_mouse_info || new_button || (last_mouse_plot_valid != mouse_plot_valid)) {
      label_plots();
if(!mouse_plot_valid) {   // mouse moved out of the plot area, update the last mouse position marker
   draw_plot(NO_REFRESH);
   show_last_mouse_x(GREEN);
}
      refresh_page();
      last_mouse_plot_valid = mouse_plot_valid;
      return 1;
   }

   last_mouse_plot_valid = mouse_plot_valid;
   return 0;
}


//
//
//  stuff to position plot queue data onto the screen
//
//
void plot_review(long i)
{
   // start viewing queue data at point *i*
   if(i <= 0) review_home = 1;
   else       review_home = 0;

   if(i >= plot_q_count) {  // we are past the end of the data, back up a minor tick
      while((i >= plot_q_count) && plot_mag) {
         i = plot_q_count - (HORIZ_MINOR*view_interval) / (long) plot_mag;
      }
      if(i < 0) i = 0;
      BEEP(8);
   }
   else if(i < 0) {  // we are at the start of the data in the queue
      i = 0;
      BEEP(9);
   }

   review = i;
   review_mode = 1;
   plot_start = review;

   draw_plot(REFRESH_SCREEN);
}

void zoom_review(long i, u08 beep_ok)
{
   i -= plot_q_out;

   if(i >= plot_q_count) {  // we are past the end of the data, back up a minor tick
      i -= plot_q_count;
      if(beep_ok) BEEP(10);
   }
   if(i < 0) {  // we are at the start of the data in the queue
      i = 0;
      if(beep_ok) BEEP(11);
   }

   review = i;
   review_mode = 1;
   plot_start = review;

   draw_plot(NO_REFRESH);

   if(no_mouse_review) ;
   else get_mouse_info();  // update cursor info in case we are not doing live data

   refresh_page();
}

void kbd_zoom()
{
u32 val;

   // toggle view interval from 1 sec/pixel to longer view
   if(mouse_plot_valid && plot_mag) {
      if((view_interval == 1L) && (queue_interval > 0)) {
         if(user_view) view_interval = user_view;
////     else if(0 && (plot_q_count < ((24L*60L*60L) / queue_interval))) {  // less than 24 hours of data available
         else if(0) {  // less than 24 hours of data available
            view_all(0);
            mark_q_entry[0] = last_mouse_q;  // marker 0 is mouse click marker
         }
         else {
            val = ((60L*60L)/HORIZ_MAJOR)/queue_interval;  // 1 hr per division
            view_interval = val;
         }
      }
      else {
         day_plot = 0;
         view_interval = 1L;
      }

      mark_q_entry[0] = last_mouse_q;  // marker 0 is mouse click marker

      val = last_mouse_q;
      val -= (((PLOT_WIDTH/2)*view_interval) / (long) plot_mag);  // center point on screen
      if(last_mouse_q < plot_q_out) val += plot_q_size;
      zoom_review(val, REVIEW_BEEP);
   }
}

void goto_mark(int i)
{
long val;

   // center plot window on the marked point
   if((i < 0) || (i >= MAX_MARKER)) return;
   if(mark_q_entry[i] && plot_mag) { // a queue entry is marked
      last_q_place = last_mouse_q;
      val = mark_q_entry[i];
      val -= (((PLOT_WIDTH/2)*view_interval) / (long) plot_mag);  // center point on screen
      if(mark_q_entry[i] < plot_q_out) val += plot_q_size;
      zoom_review(val, REVIEW_QUIET);
   }
}


void end_review(u08 draw_flag)
{
   // exit plot review mode
   review_mode = 0;
   review = plot_q_count;

   plot_start = plot_q_in - (((PLOT_WIDTH - PLOT_SCROLL)*view_interval)/(long)plot_mag) - plot_q_out;
   if(plot_start < 0) {
       if(plot_q_full) {
          plot_start += plot_q_size;
          if(plot_start < 0) plot_start = 0;
       }
       else plot_start = 0;
   }

//!!!!- pause_data = 0;
   restore_plot_config();
   if(draw_flag) draw_plot(REFRESH_SCREEN);
}


void adev_review(int c, int why)
{
   // this routine lets you scroll around the adev tables and plots

   if(c == HOME_CHAR)  {  // go to start of adev data
      first_show_bin = 0;
   }
   else if(c == END_CHAR) { // go to end of adev data
      first_show_bin = max_bins_shown - adev_table_rows;
      if(first_show_bin < 0) first_show_bin = 0;
   }
   else if(c == LEFT_CHAR) {  // scroll adevs by 5 bins
      first_show_bin += 5;
      goto scroll_adevs;
   }
   else if(c == RIGHT_CHAR) {
      first_show_bin -= 5;
      goto scroll_adevs;
   }
   else if(c == PAGE_UP) {  // scroll adevs by one screen
      first_show_bin += (adev_page_size - 2);
      goto scroll_adevs;
   }
   else if(c == PAGE_DOWN)  {
      first_show_bin -= (adev_page_size - 2);
      goto scroll_adevs;
   }
   else if(c == UP_CHAR) {  // scroll adevs by 10 bins
first_show_bin += 10;
goto scroll_adevs;
   }
   else if(c == DOWN_CHAR) {
first_show_bin -= 10;
goto scroll_adevs;
   }
   else if(c == '<') {      // scroll adves one bin
goto scroll_left;
   }
   else if(c == '>') {
goto scroll_right;
   }
   else if(c == '[') {      // scroll adevs one bin
      scroll_left:
      ++first_show_bin;

      scroll_adevs:
      if(first_show_bin >= max_bins_shown) {
         first_show_bin = max_bins_shown-1;
         if(first_show_bin < 0) first_show_bin = 0;
         BEEP(12);
      }
      else if(first_show_bin < 0) {
         first_show_bin = 0;
         BEEP(13);
      }
      force_adev_redraw(3);
   }
   else if(c == ']') {
      scroll_right:
      --first_show_bin;
      goto scroll_adevs;
   }
   else {  // terminate adev scroll mode
      last_was_adev = 0;
      lwa = 1;
      force_adev_redraw(4);
      end_review(1);
   }
}

void do_review(int c, int why)
{
   // move the view around in the plot queue
   if(last_was_adev) {
      adev_review(c, why);
   }
   else if(c == HOME_CHAR)  {  // start of plot data
      plot_review(0L);
   }
   else if(c == END_CHAR) { // end of plot data
      review_end:
      review = plot_q_count - ((PLOT_WIDTH*view_interval)/(long)plot_mag) + 1;
      if(review < 0) review = 0;
      plot_review(review);
   }
   else if(c == LEFT_CHAR) {  // scroll one major division
      if(review_mode == 0) goto review_end;
      plot_review(review + ((HORIZ_MAJOR*view_interval)/(long)plot_mag));
   }
   else if(c == RIGHT_CHAR) {
      if(review_mode == 0) goto review_end;
      plot_review(review - ((HORIZ_MAJOR*view_interval)/(long)plot_mag));
   }
   else if(c == PAGE_UP) {  // scroll one screen
      if(review_mode == 0) goto review_end;
      plot_review(review + ((PLOT_WIDTH*view_interval)/(long)plot_mag));
   }
   else if(c == PAGE_DOWN)  {
      if(review_mode == 0) goto review_end;
      plot_review(review - ((PLOT_WIDTH*view_interval)/(long)plot_mag));
   }
   else if(c == UP_CHAR) {  // scroll one hour
      if(review_mode == 0) goto review_end;
      if(queue_interval > 0) plot_review(review + (3600L/queue_interval));
   }
   else if(c == DOWN_CHAR) {
      if(review_mode == 0) goto review_end;
      if(queue_interval > 0) plot_review(review - (3600L/queue_interval));
   }
   else if(c == '<') {      // scroll one day
      if(review_mode == 0) goto review_end;
      if(queue_interval > 0) plot_review(review+(long) ((24L*3600L)/queue_interval));
   }
   else if(c == '>') {
      if(review_mode == 0) goto review_end;
      if(queue_interval > 0) plot_review(review-(long) ((24L*3600L)/queue_interval));
   }
   else if(c == '[') {      // scroll one pixel
      if(review_mode == 0) goto review_end;
      if(queue_interval > 0) plot_review(review+view_interval);
   }
   else if(c == ']') {
      if(review_mode == 0) goto review_end;
      if(queue_interval > 0) plot_review(review-view_interval);
   }
   else {  // terminate scroll mode
      end_review(1);
   }
}

void new_view()
{
   // set up display for the new view parameters
   if((view_interval == 1L) && (queue_interval > 0) && nav_rate) {
      view_interval = ((60L*60L)/HORIZ_MAJOR)/queue_interval;  // 1 hr per division
      view_interval = (int) (((float) view_interval) / nav_rate);
   }
   else {
      day_plot = 0;
      view_interval = 1L;
      config_screen(8);
   }
   if(view_interval <= 0L) view_interval = 1L;
   user_view = 0;

   redraw_screen();
}

void adjust_view()
{
   // configure screen for the view the user specified on the command line
   if(set_view == 1) {
      config_screen(9);
      day_plot = SCREEN_WIDTH / HORIZ_MAJOR;

      if((view_interval == 1L) && (queue_interval > 0)) {
         view_interval = ((60L*60L)/HORIZ_MAJOR)/queue_interval;  // 1 hr per division
      }
      else {
         day_plot = 0;
         view_interval = 1L;
         config_screen(10);
      }
      if(view_interval <= 0L) view_interval = 1L;
      user_view = 0;
//    user_view = day_plot;
   }
}

//
//
//  Keypress / wait for key routines
//
//

void serve_os_queue()
{
   // service the operating system GUI message queue

   #ifdef WIN_VFX
      serve_vfx();
   #endif

   #ifdef USE_X11
      get_x11_event();
      XFlush(display);
   #endif
   #ifdef USE_SDL
      get_sdl_event();
   #endif
}


void key_wait(char *s)
{
// debug routine - display message and wait for key
strcpy(plot_title, s);
title_type = OTHER;
show_title();
refresh_page();
BEEP(14);

   while(1) {
      serve_os_queue();

      if(zoom_screen == 'K') get_mouse_info();
      if(KBHIT()) break;
   }
   GETCH();
BEEP(15);
}


void wait_for_key(int serve)
{
     if(SER_AVAIL_RCVR) serve = 0;  // prevent possible lockup
//   if(POLLED_RCVR) serve = 0;  // prevent possible lockup

   if(timer_serve) {
      return; // don't let windows timer access this routine
   }

   first_key = ' ';  // make sure plot area is not disturbed
   while(break_flag == 0) {
      serve_os_queue();

if((zoom_screen == 'K') || showing_cmd_help) get_mouse_info();
      if(KBHIT()) break;

      if(serve && (com[RCVR_PORT].process_com || sim_file || (rcvr_type == NO_RCVR) || (rcvr_type == TIDE_RCVR))) {
         get_pending_gps(0);   //!!!! possible recursion
      }
   }
}


void abort_wakeup()
{
   // see if user says to stop the receiver wakeup loop
   serve_os_queue();

   if(KBHIT()) {
      if(GETCH() == ESC_CHAR) {
         error_exit(6666, "User pressed ESC during abort_wakeup()");
      }
   }
}

void set_restore_size()
{
   // remember the current window size so we can restore the screen to it
#ifdef USE_X11
   have_restore_size = 2;
   restore_width = SCREEN_WIDTH;
   restore_height= SCREEN_HEIGHT;
#endif
}


//
//
//  Script and config file processing
//
//

void close_script(u08 close_all)
{
int i;

   // close script file and un-nest a level if script is nested

   if(script_file == 0) return;

   if(close_all) {
      for(i=0; i<script_nest; i++) fclose(scripts[i].file);
      script_file = 0;
      script_pause = 0;
      script_nest = 0;
   }
   else if(script_nest) {
      fclose(script_file);
      --script_nest;
      strcpy(script_name, scripts[script_nest].name);
      script_file   = scripts[script_nest].file;
      script_line   = scripts[script_nest].line;
      script_col    = scripts[script_nest].col;
      script_err    = scripts[script_nest].err;
      script_fault  = scripts[script_nest].fault;
      script_pause  = scripts[script_nest].pause;
   }
   else {
     fclose(script_file);
     script_file  = 0;
     script_pause = 0;
   }
}

int open_script(char *fn)
{
   // open a keyboard script file (they can be nested)

   if(script_file) { // nested script files
      if(script_nest < SCRIPT_NEST) {
         strcpy(scripts[script_nest].name, script_name);
         scripts[script_nest].file  = script_file;
         scripts[script_nest].line  = script_line;
         scripts[script_nest].col   = script_col;
         scripts[script_nest].fault = script_fault;
         scripts[script_nest].err   = script_err;
         scripts[script_nest].pause = script_pause;
         ++script_nest;
      }
      else {
         edit_error("Script files nested too deep");
         close_script(1);
         return 2;
      }
   }

   strncpy(script_name, fn, SCRIPT_LEN);
   script_file  = topen(fn, "r");
   script_line  = 1;
   script_col   = 0;
   script_fault = 0;
   script_pause = 0;
   skip_comment = 0;
   if(script_file) return 0;
   else            return 1;
}


int get_script()
{
int i;

   // read a simulated keystroke from a script file
   i = fgetc(script_file);
   if(i < 0) {  // end of file
      close_script(0);
      return 0;
   }
   else if((i == 0x00) || (i == 0x01)) { // two byte cursor char
      i = fgetc(script_file);
      if(i < 0) {
         close_script(0);
         return 0;
      }
      i += 0x0100;
   }
   else if((skip_comment == 0) && (i == '~')) { // switch input to keyboard
      script_pause = 1;
      return 0;
   }

   if(i == 0x0A) i = 0x0D; // CR and LF are the same
   if(i == 0x0D) {         // end of line
      ++script_line;
      script_col = 0;
      skip_comment = 0;
   }
   else ++script_col;

   if(skip_comment) return 0;
   return i;
}

int process_script_file()
{
int i;

   if(script_fault) {  // error in script,  abort
      close_script(1);
      getting_string = 0;
      alarm_wait = 0;
      return 0;
   }
   else if(KBHIT()) {  // key pressed,  abort script
      i = edit_error("Script aborted...  key pressed.");
      close_script(1);
      getting_string = 0;
      alarm_wait = 0;
      return 0;
   }
   else if(alarm_wait && (alarm_time || alarm_date || egg_timer || egg_val)) { // script is waiting for an alarm to trigger
      return 0;
   }
   else {   // get keyboard command from the script file  // !!!!!! what about force_si_cmd?
      i = get_script();
      if(i) do_kbd(i);
      return i;
   }
}


int read_config_file(char *name, u08 local_flag, u08 add_path)
{
char fn[MAX_PATH+1];
char msg[MAX_PATH+80];
char buf[SLEN+1];
char *dot;
FILE *file;
int error;
int i;

   // read command line options from a .CFG file
   if(name == 0) return 0;

printf("read_config_file(name:%s  local:%d  add:%d)  path:%s\n", name,(int) local_flag, (int) add_path, heather_path);
   fn[0] = 0;
   if(add_path) {
      strcat(fn, heather_path);
   }
   strcat(fn, name);

   dot = strstr(&fn[1], ".");
   if(dot) {   // file name has a .EXTension
               // replace with .CFG,  unless user gave the name on the command line
      if(local_flag == 1) {  // cfg file name is based upon the .EXE file name
         strcpy(dot, ".cfg");
      }
      else if(local_flag == 2) {
         strcpy(dot, ".cal");
      }
   }
   else {
      if(local_flag == 1) strcat(fn, ".cfg");  // no extension given,  use .CFG
      else                strcat(fn, ".cal");  // no extension given,  use .CAL
   }

   if(local_flag) {  // cfg file name is based upon the .EXE file name
      dot = strstr(fn, ".");
      while(dot >= &fn[0]) {  // look in current directory first
#ifdef WINDOWS
         if(*dot == '\\') break;
#else  // __linux__  __MACH__  __FreeBSD__
         if(*dot == '/') break;
#endif
         --dot;
      }
printf("Attempting to open config file(%s):(%s)\n", fn, dot+1);
      file = topen(dot+1, "r");
      if(file) {
         if(local_flag == 2) printf("Reading calendar file:%s\n", dot+1);
         else                printf("Reading config file:%s\n", dot+1);
      }
   }
   else file = 0;

   if(file == 0) {  // now look in .EXE file directory
      file = topen(fn, "r");
printf("Attempting to open config file::%s\n", fn);
      if(file == 0) { // config file not found
         if(local_flag) return 1;
         sprintf(msg, "\nConfig file %s not found!", fn);
         buf[0] = 0;
         command_help(msg, buf, 0);
         return 1;
//qqq    exit(10);
      }
      printf("Reading config file: %s\n", fn);
   }

   if(local_flag == 2) {
      fclose(file);
      #ifdef GREET_STUFF
         read_calendar(fn, 1);
      #endif
      return 0;
   }
#ifdef WINDOWS
   else
   {
   memset(heather_cfg_path, 0, sizeof(heather_cfg_path));
   _snprintf(heather_cfg_path,sizeof(heather_cfg_path)-1,"%s", fn);
   }
#endif

   while(fgets(buf, sizeof buf, file) != NULL) {
      if(buf[0] == '@') {       // @keyboard command - copy to temp script file
         if(local_flag != 2) {  // it's not from a calendar file
            if(temp_script == 0) {
               temp_script = topen(TEMP_SCRIPT, "w");
            }
            if(temp_script && buf[0] && buf[1]) {  // copy line to temporary script file for later execution
               fputs(&buf[1], temp_script);
            }
         }
      }
      else {
         if((buf[0] != '/') && (buf[0] != '-') && (buf[0] != '=') && (buf[0] != '$')) continue;
         for(i=strlen(buf)-1; i>=0; i--) {       // trim trailing garbage from line
            if     (buf[i] == 0x0A) buf[i] = 0;
            else if(buf[i] == 0x0D) buf[i] = 0;
            else if(buf[i] == ' ')  buf[i] = 0;
            else if(buf[i] == '\t') buf[i] = 0;
            else break;
         }

         error = option_switch(buf);

         if(error) {
            sprintf(blanks, "in config file %s: ", fn);
            command_help(blanks, buf, 0);
            return 1;
//qqq       exit(10);
         }
      }
   }

   fclose(file);
   return 0;
}


void config_options(void)
{
int k;
char old_buf[SLEN+1];     // the text string the user is typing in

   // configure system for any screen related command line options that were set
   old_buf[0] = 0;

   if(need_screen_init) {
#ifdef USE_X11
      if(need_screen_init == 99999) {  // piss go to full screen mode (/fu from keyboard)
         need_screen_init = 0;
         have_restore_size = 0;
         have_root_info = 0;
         x11_maxed = 0;
         SCREEN_WIDTH = display_width;
         SCREEN_HEIGHT = display_height;
         restore_width = display_width;
         restore_height = display_height;

         go_fullscreen = 1;   // piss   go_full()
         kill_deco = go_fullscreen; // disable X11 window decorations

         screen_type = 'c';
         strcpy(old_buf, edit_buffer);
         sprintf(edit_buffer, "%dx%d",SCREEN_WIDTH,SCREEN_HEIGHT);
         if(display) XFlush(display);
         keyboard_cmd = 1;
         edit_screen_res();
         strcpy(edit_buffer, old_buf);
         keyboard_cmd = 0;
go_fullscreen = kill_deco = 0;
      }
      else {
         need_screen_init = 0;
         init_screen(9106);
      }
#else
      need_screen_init = 0;
      init_screen(9106);
#endif
   }

   if(keyboard_cmd == 0) {
      if(SCREEN_WIDTH < MEDIUM_WIDTH) {  // blank clock name for small screens
         if(watch_name[0] == 0)  strcpy(watch_name,  " ");
         if(watch_name2[0] == 0) strcpy(watch_name2, " ");
      }
      else if(0) {
         if((watch_name[0] == ' ') && (watch_name[1] == 0)) watch_name[0] = 0;
         if((watch_name2[0] == ' ') && (watch_name2[1] == 0)) watch_name2[0] = 0;
      }
   }

   // set where the first point in the graphs will be
   last_count_y = PLOT_ROW + (PLOT_HEIGHT - VERT_MINOR);
   for(k=0; k<NUM_PLOTS+DERIVED_PLOTS; k++) {
      plot[k].last_y = PLOT_ROW+PLOT_CENTER;
      plot[k].last_trend_y = PLOT_ROW+PLOT_CENTER;
   }

   if(user_view_string[0]) {  // user set a plot view interval on the command line
      edit_user_view(user_view_string);
   }
   else if(new_user_view && (keyboard_cmd == 0) && user_view && (queue_interval > 0)) {   // set user specified plot window time (in hours)
      view_interval = (user_view * 60L*60L) / queue_interval;
      view_interval /= PLOT_WIDTH;
      if(view_interval <= 1L) view_interval = 1L;
      user_view = view_interval;
   }
   new_user_view = 0;

   // user set a daylight savings time definition
   if(user_set_dst && have_year) {
      calc_dst_times(this_year, dst_list[dst_area]);
   }

   // if user did not enable/disable the big clock,  turn it on if it will fit
   if(user_set_clock_plot == 0) {
      if(SCREEN_WIDTH > NARROW_SCREEN) plot_digital_clock = 1;
      else if(rcvr_type == NO_RCVR) plot_digital_clock = 1;
      else if((TEXT_HEIGHT <= 8) && (text_mode == 0)) plot_digital_clock = 1; ////
      else if((small_font == 1) && text_mode) plot_digital_clock = 1;
   }

   #ifdef TEMP_CONTROL
      if(do_temp_control) {  // user wants to actively stabilize the unit temperature
         if(desired_temp) enable_temp_control();
         else             disable_temp_control();
      }
   #endif

   if(not_safe) {
      if(not_safe == 3) ;   // xyzzy
      else if(com[RCVR_PORT].process_com == 0) ;  // xyzzy
      else if(com[RCVR_PORT].com_running <= 0) ;  // xyzzy
      else if(did_init1) init_messages(1, 0);
      else               init_messages(1, 1);
      did_init1 = 1;
   }
}



void set_defaults()
{
double angle;
int i;
int j;
unsigned port;

   // This routine is used to give initial values to variables.
   // The QuickC linker does not let one initialize variables from more than
   // one file (even if they are declared extern).

   touch_screen = 1;     // enable mouse driven touch screen
   my_mouse = 1;
   invert_display = 0;
   setup_palette();
   idle_screen = 'C';    // idle keyboard mode zoom screen type

   unit_file_name = "tbolt";
   strcpy(dump_prefix, "tb");
   strcpy(lock_name, LOCK_NAME);

   detect_rcvr_type = 1;
   rcvr_type = last_rcvr_type = TSIP_RCVR;
   nav_rate = 1.0;               // assume receiver running at 1Hz
   default_gnss = MIXED;
   need_rcvr_init = 1;

   sunrise_type = "Official";
   sunrise_horizon = sunset_horizon = (-50.0 / 60.0);
   play_sun_song = 0;

   track_port_info = (SEND_SUN | SEND_MOON | SEND_TIME | SEND_SATS | SEND_HIGHEST);

   last_eclipse = (-1);
   eclipse_d1 = BIG_NUM;
   eclipse_d2 = BIG_NUM;
   eclipse_d3 = BIG_NUM;

   min_sig_db = 30;      // low sig level threshold for sig level map
   sig_level_step = 2;   // sig level map signal steo size
   amu_flag = 0x08;      // assume dBc mode for TSIP devices
   amu_mode = 0;

   use_tsc = 1;          // use the TSC instruction for nanosecond counter
   last_time_msec = GetMsecs();  // message jitter ellapsed millisecond counter
   this_time_msec = msg_sync_msec = last_time_msec + 1000.0;
   time_zero = 0.0;
   time_sync_offset = TIME_SYNC_AVG;  // average of all tested receivers
   timing_mode = TMODE_UTC;  // assume UTC time

   traim_threshold = 1; // bogus value which should be rejected when initing the receiver
   furuno_traim = 100;
   sats_enabled = 0xFFFFFFFF;
   update_disable_list(sats_enabled);
   system_mask = MIXED;
   did_init1 = 0;

   this_msec = GetMsecs();

   #ifdef WINDOWS
      continuous_scroll = 1;  // windows defaults to continuous scroll mode
   #endif
continuous_scroll = 1;
   enable_timer = 1;      // enable windows dialog timer or __linux__ mouse right click timer

   angle = 0.0;
   #ifdef SIN_TABLES
      // precalculate sin and cos values - uses memory DOS version can't spare
      for(i=0; i<=360; i+=1) {
         angle = ((double) i) * DEG_TO_RAD;
         sin_table[i] = (float) sin(angle);
         cos_table[i] = (float) cos(angle);
      }
   #endif

   TEXT_WIDTH = 8;        // font size
   TEXT_HEIGHT = 16;
   VCHAR_SCALE = 6;       // default vector char size multiplier
   use_vc_fonts = 0;      // if not 0, draw fonts using vector chars
   vc_font_scale = 100;

   TEXT_COLS = 80;        // screen size (in text charactrers)
   TEXT_ROWS = 25;
   INFO_COL = 65;

   VER_ROW = 0;
   VER_COL = 38;
   if(SCREEN_WIDTH <= MIN_WIDTH) --VER_COL;

   screen_type = 'm';     // medium size screen
   SCREEN_WIDTH = 1024;
   SCREEN_HEIGHT = 768;
   PLOT_ROW = 468;
   scale_plot_row();

   beep_on = 1;           // allow beeps
   sound_on = 1;          // disable sound files
   fast_script = 0;       // no delay between keyboard script keystrokes

   for(port=0; port<NUM_COM_PORTS; port++) {  // init com port info
      com[port].com_fd = (-1);
      com[port].com_running = (-14); //NOT_RUNNING;
      com[port].process_com = 1;
      com[port].com_recover = 1;    // attempt com port recovery on receiver data loss
      com[port].user_disabled_com = 0;
      com[port].IP_addr[0] = 0;     // TCP/IP address

      com[port].baud_rate = 9600;
      com[port].data_bits = 8;
      com[port].parity = NO_PAR;    // no parity (0=none 1=odd, 2=even)
      com[port].stop_bits = 1;

      com[port].com_timeout = COM_TIMEOUT;
      com[port].user_timeout = COM_TIMEOUT;

      #ifdef WINDOWS
         hSerial[port] = INVALID_HANDLE_VALUE;
         IPC[port] = NULL;   // TCP
      #endif
   }
   com[TICC_PORT].baud_rate = 115200; // aaattt
   com[TICC_ECHO_PORT].baud_rate = 115200; // aaattt
   com[THERMO_PORT].baud_rate = 115200; // aaattt
   com[FAN_PORT].baud_rate = 115200; // aaattt
   parity_defined = NO_PAR;

   fan_port = RCVR_PORT;   // temp control device, assume connected to receiver port

   ticc_type = TAPR_TICC;
   ticc_mode = DEFAULT_TICC_MODE;
   ticc_syncmode = TICC_SYNCMODE;
   ticc_cal = TICC_CAL;
   ticc_timeout = TICC_TIMEOUT;
   ticc_speed = TICC_SPEED;
   ticc_coarse = TICC_COARSE;
   ticc_tune_time = TICC_TUNE_TIME;
   pet_clock = 1.0;  // * 10MHz
   lars_gain = 1.0;
   lars_initv = 32768.0;

   nominal_cha_freq = NOMINAL_FREQ;
   nominal_chb_freq = NOMINAL_FREQ;
   nominal_chc_freq = NOMINAL_FREQ;
   nominal_chd_freq = NOMINAL_FREQ;
   cha_phase_wrap_interval = (1.0 / NOMINAL_FREQ);  // used to unrwap time interval phase (generic counters)
   chb_phase_wrap_interval = (1.0 / NOMINAL_FREQ);
   chc_phase_wrap_interval = (1.0 / NOMINAL_FREQ);  // used to unrwap time interval phase (generic counters)
   chd_phase_wrap_interval = (1.0 / NOMINAL_FREQ);

   dilat_a = dilat_b = dilat_c = dilat_d = TICC_DILAT;  // TAPR_TICC time dilation

   cs_supply = "UNKN";
   cs_sync = "UNKN";
   cs_log_count = (-1);

   fix_star_ts = 1;

   monitor_port = MONITOR_PORT;     // the default port for monitor mode to show
   term_port = TERM_PORT;           // the default port for the terminal emulator to use

#ifdef WINDOWS
   com[RCVR_PORT].com_port = 1;     // use COM1 for receiver serial I/O
   com[RCVR_PORT].usb_port = 0;
#else   // __linux__  __MACH__  __FreeBSD__
   com[RCVR_PORT].com_port = 0;     // use /dev/ttyUSB0 for receiver serial I/O
   com[RCVR_PORT].usb_port = 1;
#endif

   first_msg = 1;
   first_request = 1;
   restart_count = 0;

   moto_chans = 12;
   max_sat_display = MAX_SAT_DISPLAY;
   max_sats = 8;        // used to format the sat_info data
   max_sat_count = 8;
   sat_cols = 1;
   temp_sats = 8;
   if(max_sat_count > max_sat_display) {
      max_sats = max_sat_display;         // used to format the sat_info data
      max_sat_count = max_sat_display;
      temp_sats = max_sat_display;
   }
   fancy_sats = ROUND_LINES;   // round sats with linear wings
   sat_rows = max_sat_display;
   ms_row = ms_col = 0;
   eofs = 1;
   sort_by = 0; // !!!! SORT_PRN;
   sort_ascend = 1;

   pv_filter = 1;         // default power-up dynamics filters
   static_filter = 1;
   alt_filter = 1;
   kalman_filter = 0;
   marine_filter = 100;
   have_kalman = 1;

   user_pv = 1;           // user requested dynamics filters
   user_static = 1;
   user_alt = 1;
   user_kalman = 1;
   user_marine = 100;

   cmd_tc = 500.0F;       // default disciplining params
   cmd_damp = 1.000F;
   cmd_gain = 1.400F;
   cmd_pullin = 300;

   cmd_initdac = 3.000F;
   cmd_minv = 0.0F;
   cmd_maxv = 5.0F;
   cmd_minrange = (-5.0);
   cmd_maxrange = (5.0);

   cmd_jamsync = 300.0;
   cmd_maxfreq = 50.0;

   foliage_mode = 1;      // sometimes
   dynamics_code = 4;     // stationary
   pdop_mask = 8.0F;
   pdop_switch = pdop_mask * 0.75F;

   user_pps_enable = 1;   // output signal controls
   pps_polarity = 0;
   osc_polarity = 0;
   osc_discipline = 1;
   pps1_freq = 1.0;
   pps1_duty = 0.50;
   pps1_flags = 0;
   pps2_freq = 1.0;
   pps2_duty = 0.50;
   pps2_flags = 0;

   min_q_lat = 90.0;
   max_q_lat = (-90.0);
   min_q_lon = 180.0;
   max_q_lon = (-180.0);
   have_valid_lla = 0;
   last_lla_span = 1.0;

   nvs_pps_width = NVS_PPS_WIDTH;  // in nanoseconds
   last_nvs_pps_width = NVS_PPS_WIDTH;  // in nanoseconds
   pps_threshold = 300.0F;
   esip_pps_width = 200;

//   set_utc_mode = 1;      // default to UTC mode
//   set_gps_mode = 0;

   delay_value = 50.0F * (1.0F / (186254.0F * 5280.0F)) / VELOCITY_FACTOR;  // 50 feet of 0.66 vp coax
   dac_drift_rate = 0.0F; // units per second

   last_rmode = RCVR_MODE_UNKNOWN;
   last_hours = last_log_hours = 99;
   last_second = 99;
   last_utc_offset = (-9999);
   need_delta_t = 1;
   force_utc_time = 1;
   first_sample = 1;       // flag cleared after first data point has been received
   time_color = RED;
   time_set_char = ' ';

   get_clock_time();       // initialize TICC simulation file time
   sim_jd = jdate(clk_year,clk_month,clk_day) + jtime(clk_hours,clk_minutes,clk_seconds,0.0);

   queue_interval = 1;     // seconds between queue updates
   log_interval = 1;       // seconds between log file entries
   log_header = 1;         // write timestamp headers to the log file
   view_interval = 1;      // plot window view time
   show_min_per_div = 1;
   day_size = 24;          // assumed day plot size
   plot_mag = 1;           // plot magnifier (for more than one pixel per second)
   plot_column = 0;
   stat_count = 0.0F;
   csv_char = '\t';        // log file separator char
   log_comments = 1;       // write comments in the log file
   leap_dump = 1;          // do screen dump at leap-second

   if(luxor) {
      strcpy(log_name,  "luxor.log");
      strcpy(debug_name,"debug.log");
      strcpy(raw_name,  "luxor.raw");
      strcpy(ticc_name, "ticc.raw");
   }
   else {
      strcpy(log_name,  "tbolt.log");
      strcpy(debug_name,"debug.log");
      strcpy(raw_name,  "tbolt.raw");
      strcpy(prn_name,  "tbolt.prn");
      strcpy(ticc_name, "ticc.raw");
   }
   strcpy(rpn_name, RPN_FILE);

   log_mode = "w";  // write mode (append mode = "a")
   log_errors = 1;  // if set, log data errors
   log_stream = 0;  // don't write serial data stream to the log file
// log_stream = (LOG_HEX_STREAM | LOG_PACKET_ID | LOG_PACKET_START | LOG_SENT_DATA);
if(0) {  // gggg enable raw receiver data logging   xyzzy
   log_stream = LOG_RAW_STREAM;
   raw_file = open_raw_file(raw_name, "wb");
}

   #ifdef ADEV_STUFF
      ATYPE = last_atype = OSC_ADEV;
      adev_period = 1.0;
      pps_adev_period = 1.0;
      osc_adev_period = 1.0;
      chc_adev_period = 1.0;
      chd_adev_period = 1.0;
      save_adev_period();

      bin_scale = 5;        // 1-2-5 adev bin sequence
      n_bins = MAX_ADEV_BINS;
      min_points_per_bin = 4;
      keep_adevs_fresh = 1;
      adev_display_mask = (DISPLAY_ADEV | DISPLAY_HDEV | DISPLAY_MDEV | DISPLAY_TDEV | DISPLAY_MTIE);
      adev_display_mask |= (DISPLAY_CHA | DISPLAY_CHB | DISPLAY_CHC | DISPLAY_CHD);

      jitter_adev = 0;
      pps_adevs_cleared = 1;
      osc_adevs_cleared = 1;
      chc_adevs_cleared = 1;
      chd_adevs_cleared = 1;
subtract_base_value = 0;
   #endif
   nominal_cha_freq = NOMINAL_FREQ;
   nominal_chb_freq = NOMINAL_FREQ;
   nominal_chc_freq = NOMINAL_FREQ;
   nominal_chd_freq = NOMINAL_FREQ;
   aa_val = ALL_PPS;             // show all PPS adevs
   mixed_adevs = MIXED_GRAPHS;   // graphs and adev plots
   left_adev_col = ADEV_COL;

   // flags to control what and what not to draw
   plot_adev_data = 1;      // adevs
   plot_skip_data = 1;      // time sequence and message errors
   plot_sat_count = 1;      // satellite count
   small_sat_count = 1;     // used compressed sat count plot
   plot_const_changes = 0;  // if set, flag changes of satellites being used for fixes
   plot_holdover_data = 1;  // holdover status
   plot_digital_clock = 0;  // big digital clock
   plot_loc = 1;            // actual lat/lon/alt
   strcpy(stat_id, "RMS:");
   plot_dops = 0;           // dilution of precision (in place of filters on small screens)
   plot_filters = 1;        // if set, plot receiver filter settings
   plot_azel = 0;           // satellite azimuth/elevation map
   plot_signals = 0;        // satellite signal levels
   plot_el_mask = 1;        // show elevation angle mask in the azel plot
   map_trails = 1;          // draw satellite history trails
   dot_trails = 1;          // draw time markers on satellite history trails
   dynamic_trend_line = 1;  // if set, update trend line info dynamically
   tide_options = 0;        // solid earth tide calculation options
   shared_plot = 0;         // share az/el or lla maps with the plot area
   need_posns = 2;          // calculate sun and moon positions

   lux_scale = 1.0F;        // lux reading scale factor
   lum_scale = 1.0F;        // lumen reading scale factor
   show_lux = 1;
   show_fc = 0;
   show_cp = 0;
   show_lumens = 0;


   BLUE_SENS  = 123.94F;    // 371.0F  // converts Hz to uW/cm^2
   GREEN_SENS = 144.80F;    // 386.0F
   RED_SENS   = 177.74F;    // 474.0F
   WHITE_SENS = ((RED_SENS+BLUE_SENS+GREEN_SENS)/3.0F);
   cct_cal = 1.0F;
   cct1_cal = 1.07F;
   cct2_cal = 1.07F;
   cct_type = 0;

   lipo_volts = 3.60F;
   batt_pwm_res = 10;
   PWM_STEP = (64.0F / 65536.0F);
   sweep_rate = sweep_tick = 1;

   show_color_pct = 0;      // use raw color values (not scaled to percentages)
   show_color_hz = 0;
   show_color_uw = 1;

   show_euro_ppt = 0;
   set_osc_units();

   extra_plots = 0;
   num_plots = NUM_PLOTS+DERIVED_PLOTS;
   plot_stat_info = 0;
   for(i=0; i<NUM_PLOTS+DERIVED_PLOTS; i++) {     // initialize default plot parameters
      plot[i].invert_plot  = 1.0F;      // normal graph polarity
      plot[i].scale_factor = 1.0F;      // unity data scale factor
      plot[i].show_stat = SHOW_SPAN;    // was RMS - statistic to show
      if(i >= FIRST_EXTRA_PLOT) {
         extra_plots |= plot[i].show_plot;
      }
      plot_stat_info |= plot[i].show_stat;
   }

   plot[OSC].units = ppt_string;
   plot[DAC].plot_center = plot[TEMP].plot_center = NEED_CENTER;
   if(luxor) {
      plot[TC2].plot_center = NEED_CENTER;
   }


   plot[MSGJIT].show_stat = SDEV; // SHOW_SPAN
   plot[MSGOFS].show_stat = AVG;  // SHOW_SPAN

   auto_scale = 1;             // auto scale the plots
   auto_center = 1;            // and auto center them

   deg_string[0] = DEGREES;
   deg_string[1] = 0;
   DEG_SCALE = 'C';            // Celcius
   alt_scale = "m";            // meters
   level_type = "SIG";         // AMU or dB
   dms = 0;                    // decimal degrees
   strcpy(tz_string,  "LOC");  // current time zone name
   strcpy(std_string, "LOC");  // normal time zone name
   strcpy(dst_string, "LOC");  // daylight savings time zone name

//   LLA_SPAN = 10.0;            // lla plot scale in feet per division
//   ANGLE_SCALE = DEG_PER_FOOT; // earth angle in degrees per foot
//   angle_units = "ft";

   LLA_SPAN = 3.0;
   ANGLE_SCALE = ((DEG_PER_FOOT)*FEET_PER_METER); // degrees per meter
   angle_units = "m";

   cos_factor = 0.82;          // cosine of latitude
// LLA_SPAN = 20.0;
// ANGLE_SCALE = (DEG_PER_FOOT*FEET_PER_METER); // degrees per meter
// angle_units = "m";
   last_lla_span = LLA_SPAN;

   adev_q_size = (12L*3600L);      // 12 hours - good for 10000 tau
   plot_q_size = (60L*60L*24L*3L); // 72 hours of data
   mouse_shown = 1;
   last_plot_mouse_x = (-1);
   last_plot_tick = (-1);
   last_plot_mouse_q = (0);

   // convert __DATE__ into Lady Heather's offical date format
   i = j = 0;
   while(__DATE__[i] && (__DATE__[i] != ' ')) ++i;
   while(__DATE__[i] && (__DATE__[i] == ' ')) ++i;
   while(__DATE__[i] && (__DATE__[i] != ' ') && (j < VSTRING_LEN-8)) {
      date_string[j++] = __DATE__[i++];
   }
   if(j == 1) {
      date_string[j++] = date_string[0];
      date_string[0] = '0';
   }
   date_string[j++] = ' ';
   date_string[j++] = __DATE__[0];
   date_string[j++] = __DATE__[1];
   date_string[j++] = __DATE__[2];
   date_string[j++] = ' ';
   while(__DATE__[i] && (__DATE__[i] == ' ')) ++i;
   while(__DATE__[i] && (__DATE__[i] != ' ') && (j < VSTRING_LEN-2)) {
      date_string[j++] = __DATE__[i++];
   }
   date_string[j++] = 0;
   label_watch_face = 1;
   fancy_hands = 1;

   #ifdef TEMP_CONTROL
      do_temp_control = 0;     // don't attempt temperature control
      temp_control_on = 0;
      desired_temp = 40.0;     // in degrees C
      lpt_port = 0;
   #endif
   mayan_correlation = MAYAN_CORR;

   ring_bell = (-1);
   bell_number = 0;

   get_clock_time();
   this_year = clk_year;
   init_dsm(this_year); // set up the days to start of month table

   plot_title[0] = 0;
   title_type = NONE;
   greet_ok = 1;

   flag_faults = 1;   // show message errors as time skips


   debug_text[0] = 0;
   debug_text2[0] = 0;
   debug_text3[0] = 0;
   debug_text4[0] = 0;
   debug_text5[0] = 0;
   debug_text6[0] = 0;
   debug_text7[0] = 0;

   test_heat = test_cool = (-1.0F);
   test_marker = 0;
   spike_threshold = 0.04F;
   spike_mode = 1;    // filter temp spikes from pid data

   set_default_pid(0);
   KL_TUNE_STEP = 0.20F;

   #ifdef OSC_CONTROL
      set_default_osc_pid(0);
      OSC_KL_TUNE_STEP = 0.0;
   #endif

   x72_tc_val = X72_TIME_CONST;
   x72_damping_val = X72_DAMPING;
   x72_jamthresh_val = X72_JAMSYNC_THRESH;
   x72_holdover_val = X72_HOLDOVER_SIZE;
   x72_osc = X72_OSC;

   user_hbsq = (-1);    // Star-4 HBSQ (holdover squelch time)
   current_hbsq = (-1); // time remaining before squelch activated

   #ifdef SIG_LEVELS
      clear_signals();
   #endif

   #ifdef GREET_STUFF
      calendar_entries = calendar_count();
   #endif

	#ifdef USE_SDL
	   sdl_scaling = 0;
	#endif

   d_scale = 1.0F;
   osc_gain = user_osc_gain = (-3.5);
   gain_color = GREY;  // was yellow

   hist_size = 512;

   undo_fw_temp_filter = 0;
   idle_sleep = IDLE_SLEEP;

// sort_by = SORT_PRN;
   sort_ascend = 1;

   rtk_mode = BASE_MODE;
   raw_msg_rate = RAW_MSG_RATE;  // how often to send receiver raw data messages
   special_raw = 0;
   rinex_fix = 1;  // fix stuck observations, missing time stamps, duplicate time stamps, etc

   strcpy(marker_name,    DEFAULT_NAME);
   strcpy(marker_number,  DEFAULT_NAME);
   strcpy(marker_type,    "GEODETIC");
   strcpy(antenna_type,   DEFAULT_NAME);
   strcpy(antenna_number, DEFAULT_NUM);
   rinex_fmt = 2.11;
//rinex_fmt = 3.03;  // RINEX v3 not yet implemented
}



//
//
//   Process the GPS receiver data
//
//

void process_extra_ports(int why)
{
   // process any data from the various extra input data ports

   if(enviro_port_open()) {  // monitoring a external environmental sensor
      get_enviro_message(THERMO_PORT);  // aaattt
   }
   else if(com[THERMO_PORT].port_used > 0) {  // for data loss recovery
      get_enviro_message(THERMO_PORT);  // aaattt
   }


   if(ticc_port_open()) {  // monitoring a GPS on the RCVR_PORT and a TICC on the TICC_PORT
      get_ticc_message(TICC_PORT);  // aaattt
   }
   else if(com[TICC_PORT].port_used > 0) {  // for data loss recovery
      get_ticc_message(TICC_PORT);  // aaattt
   }


   if(dac_port_open()) {  // monitoring external ADC
      get_dac_message(DAC_PORT);
   }
   else if(com[DAC_PORT].port_used > 0) {  // for data loss recovery
      get_dac_message(DAC_PORT);  // aaattt
   }
}


void get_device_messages(int serial_avail)
{
   // process any data from the various input devices

   if(serial_avail && (((rcvr_type == NO_RCVR) || (rcvr_type == TIDE_RCVR)) || com[RCVR_PORT].process_com || sim_file)) {  // we have receiver serial data to process
      get_rcvr_message();  // process the incoming GPS messages
      system_busy = 2;
   }
   process_extra_ports(1);  // process the extra devices
}


void get_pending_gps(int why)
{
u08 old_disable_kbd;

   // process all the data currently in the com port input buffers
   old_disable_kbd = disable_kbd;
   disable_kbd = 2; // (so that do_kbd() won't do anything when it's called by WM_CHAR during get_pending_gps())
   system_busy = 1; // we are doing other things besides waiting for serial data

   while((break_flag == 0) && (take_a_dump == 0) && (com[RCVR_PORT].process_com || sim_file || ((rcvr_type == NO_RCVR) || (rcvr_type == TIDE_RCVR))) && SERIAL_DATA_AVAILABLE(RCVR_PORT)) {
      serve_os_queue();

      get_rcvr_message();
      process_extra_ports(2);

      if(NO_PEND_LOOP) break;
   }
   check_com_timer(RCVR_PORT);

   disable_kbd = old_disable_kbd;
}


int in_terminal;     // flag set if running debug terminal

int recover_com(unsigned port)
{
    // attempt to restart a port after a receive timeout

   if(port >= NUM_COM_PORTS) return (-1);         // bad port number
   if(com[port].user_disabled_com)  return (-2);  // disabled port
   if(com[port].com_running <= 0) return (-3);    // port not init'd
   if(com[port].com_data_lost != DATA_LOSS_REINIT) return (-4);  // not timeout out or recovery failed
   if(com[port].com_recover == 0) return (-5);    // com recovery disabled on the port
   if(com[port].com_port || com[port].usb_port || com[port].IP_addr[0]) ;
   else return (-6); // port not assigned to a device or IP address

   com[port].com_data_lost = DATA_LOSS_NO_INIT;  // prevents futile re-init attempts
   if(com[port].process_com) {
      init_com(port, 3);
      return 1;
   }
   else {
      com[port].process_com = 2;
      if(com[port].com_running) {
         init_com(port, 4);
         return 2;
      }
   }
   return 0;
}

int serial_input_check()
{
int i;
static int count = 0;
#define THROTTLE 20

   // returns true if a character is available from the serial port
   // But, we might need to occasionally allow the keyboard to be
   // checked for some polled devices or high nav rate modes.


   if(GetMsecs() > last_kbd_msec) {  // we haven't checked the keyboard in a while
//sprintf(out, "GetMsecs:%.0f  last:%.0f  %.0f    ", GetMsecs(), last_kbd_msec, GetMsecs()-last_kbd_msec);
//DEBUGSTR(out);
//refresh_page();
//    reset_kbd_timer();
      return 0;
   }

   if((nav_rate > 10) && (got_timing_msg % 5) == 4) i = 0;
   else if(rcvr_type == NO_RCVR) i = SERIAL_DATA_AVAILABLE(RCVR_PORT);
   else if(rcvr_type == TIDE_RCVR) i = SERIAL_DATA_AVAILABLE(RCVR_PORT);
   else if(sim_file) i = SERIAL_DATA_AVAILABLE(RCVR_PORT);
   else if(com[RCVR_PORT].process_com == 0) i = 0;
   else if(1 && (rcvr_type == CS_RCVR)) {
      if((++count % THROTTLE) == 0) i = 0;
      else i = SERIAL_DATA_AVAILABLE(RCVR_PORT);
   }
   else if(1 && (rcvr_type == LPFRS_RCVR)) {
      if((++count % THROTTLE) < (THROTTLE/2)) i = 0;
      else i = SERIAL_DATA_AVAILABLE(RCVR_PORT);
   }
   else if(1 && (rcvr_type == PRS_RCVR)) {
      if((++count % THROTTLE) < (THROTTLE/2)) i = 0;
      else i = SERIAL_DATA_AVAILABLE(RCVR_PORT);
   }
   else if(1 && (rcvr_type == SA35_RCVR)) {
      if((++count % THROTTLE) < (THROTTLE/2)) i = 0;
      else i = SERIAL_DATA_AVAILABLE(RCVR_PORT);
   }
   else if(1 && (rcvr_type == SRO_RCVR)) {
      if((++count % THROTTLE) < (THROTTLE/2)) i = 0;
      else i = SERIAL_DATA_AVAILABLE(RCVR_PORT);
   }
   else if(1 && (rcvr_type == STAR_RCVR) && (star_type == OSA_TYPE)) {
//OSA i = SERIAL_DATA_AVAILABLE(RCVR_PORT);
      if((++count % 50) < (3)) i = 0;
      else i = SERIAL_DATA_AVAILABLE(RCVR_PORT);
   }
   else if(1 && ((++count % THROTTLE) == 0)) {  // mainly for NEC_TYPE GPSDO and ACRON_RCVR
      if(SER_AVAIL_RCVR) {   // polled receivers that need some access to the keyboard
         dup_star_time = 0;
         i = 0;
      }
      else {
         i = SERIAL_DATA_AVAILABLE(RCVR_PORT);
      }
   }
   else i = SERIAL_DATA_AVAILABLE(RCVR_PORT); // check if serial port has data (and stay in loop until it's exhausted)

   return i;
}

void sound_tick_clock()
{
   // play the tick clock sounds

   if(minute_tick && (tick_clock & 0x02)) {
      if(minute_file) play_tune(MINUTE_FILE, 1); // minute tick sound
      else BEEP(16);
   }
   else if(tick_clock & 0x01) {
      if(seconds_file) play_tune(SECONDS_FILE, 1);
      else if(click_file) play_tune(CLICK_FILE, 1);
      else BEEP(17);
   }
}


void fine_tick_check()
{
   // This routine attempts to provide an audible tick clock where the ticks
   // occur the the "true" second instead of when the receiver time message
   // arrives.  It does this by having silly_clocks() set a triiger
   // variable (in milliseconds). When this number of milliseconds has elapsed
   // the tick sound(s) are played.  The trigger variable is checked in
   // wherever the serial port is polled for input characters.

   if(fine_tick_clock == 0) return;   // fine tick clock not active
   if(tick_clock == 0) return;        // singing clock not in tick clock mode
   if(fine_tick_msec < 0.0) return;   // trigger already used
   if(GetMsecs() < fine_tick_msec) return;  // trigger time not elasped

   fine_tick_msec = (-1.0);   // flag trigger has been used
   sound_tick_clock();
}


S32 serve_gps(int why)  // called from either foreground loop or WM_TIMER (e.g., while dragging)
{
S32 i;
int served;
int old_survey;

   if(break_flag) return 0;        // ctrl-break pressed, so exit the program
   if(take_a_dump) return 1;
   if(in_terminal) return 1;

   while(1) {

      served = 0;
      #ifdef WIN_VFX
         if(!timer_serve) {
            served = 1;
            serve_vfx();
         }
      #endif
      #ifdef USE_X11
         if(!timer_serve) {
            get_x11_event();
            served = 1;
         }
      #endif
      #ifdef USE_SDL
         if(!timer_serve) {
    	     get_sdl_event();
	         served = 1;
	     }
      #endif
fake_second_check(1);
fine_tick_check();


      i = serial_input_check();  // see if OK to process the receiver serial port data

      if(i == 0) {    // no data available,  we are not too busy
if(rcvr_type == Z12_RCVR) {
   get_rcvr_message();
}
         // no data available,  we are not too busy, or taking a breather from high nav-rate updates
         system_busy = 0;
         check_com_timer(RCVR_PORT);
         get_mouse_info();

         if(rcvr_type == NO_RCVR) ;
         else if(rcvr_type == TIDE_RCVR) ;
         else if(com[RCVR_PORT].user_disabled_com) ;
         else if(com[RCVR_PORT].com_data_lost == DATA_LOSS_REINIT) { // attempt to restart the receiver
            com[RCVR_PORT].com_data_lost = DATA_LOSS_NO_INIT;  // prevents futile re-init attempts
            if(com[RCVR_PORT].process_com) {
               vidstr(0,0, RED, "NO DATA SEEN ON COM DEVICE      ");
               refresh_page();
               ++restart_count;
               if((com[RCVR_PORT].com_recover && !com[RCVR_PORT].user_disabled_com) || rcvr_reset || POLLED_RCVR || (rcvr_type == ZODIAC_RCVR)) {    // recover from hard reset
                  com_retry:
                  old_survey = do_survey;
                  do_survey = 0;
                  rcvr_reset = 0;
                  tsip_sync = 0;
                  tsip_wptr = 0;
                  tsip_rptr = 0;
                  if(com[RCVR_PORT].com_recover && !com[RCVR_PORT].user_disabled_com) {
                     init_com(RCVR_PORT, 5);
                     if(first_key == 0) Sleep(1000);
                  }

                  if(debug_file) {
                     fprintf(debug_file, "!!! RCVR_PORT DATA LOSS RESET #%d: %d !!!\n", restart_count, rcvr_reset);
                  }
                  init_messages(0, 0);
                  do_survey = old_survey;
               }
reset_kbd_timer();
               refresh_page();
            }
            else {
               vidstr(0,0, RED, "COM DATA DEVICE UNAVAILABLE     ");
               refresh_page();
               if((com[RCVR_PORT].com_recover && !com[RCVR_PORT].user_disabled_com) && com[RCVR_PORT].com_running) {
                  if(com[RCVR_PORT].com_port || com[RCVR_PORT].usb_port || com[RCVR_PORT].IP_addr[0]) com[RCVR_PORT].process_com = 3;
                  goto com_retry;
               }
            }
         }

         if(rcvr_type == NO_RCVR) get_rcvr_message();
         else if(rcvr_type == TIDE_RCVR) ;
         else if(com[RCVR_PORT].process_com == 0) refresh_page();

process_extra_ports(3);
         break;
      }
      else {
         reset_com_timer(RCVR_PORT);
      }

      get_device_messages(i);  // process any data from the input devices
      process_extra_ports(4);   // process the extra devices

      if(get_mouse_info() == 0) {     // show queue data at the mouse cursor
         if(i == 0) refresh_page();   // keep refreshing page in case no serial data
      }
   }
process_extra_ports(5);

   return 1;
}

void show_com_state()
{
   if(com[RCVR_PORT].process_com) {       // we are using the serial port
      if((com[RCVR_PORT].com_port == 0) && (com[RCVR_PORT].usb_port == 0) && com[RCVR_PORT].IP_addr[0]) {   // TCP
         sprintf(out, "WAITING FOR CONNECTION TO:%s", com[RCVR_PORT].IP_addr);
      }
      else if(nortel && com[RCVR_PORT].com_port)   sprintf(out, "Waking up Nortel receiver on COM%d", com[RCVR_PORT].com_port);
      else if(nortel && com[RCVR_PORT].usb_port)   sprintf(out, "Waking up Nortel receiver on USB%d", com[RCVR_PORT].usb_port);
      else if(com[RCVR_PORT].com_port) sprintf(out, "NO COM%d SERIAL PORT DATA SEEN", com[RCVR_PORT].com_port);
      else if(com[RCVR_PORT].usb_port) {
         if(com[RCVR_PORT].usb_port == USE_IDEV_NAME) {
            sprintf(out, "NO %s SERIAL PORT DATA SEEN", com[RCVR_PORT].com_dev);
         }
         else if(com[RCVR_PORT].usb_port == USE_DEV_NAME) {
            sprintf(out, "NO %s SERIAL PORT DATA SEEN", com[RCVR_PORT].com_dev);
         }
         else {
            sprintf(out, "NO USB%d SERIAL PORT DATA SEEN", com[RCVR_PORT].usb_port);
         }
      }
      else                             sprintf(out, "NO INPUT DEVICE FOUND");
      vidstr(0,0, RED, out);
      refresh_page();
if(first_key == 0) Sleep(1000);

//    set_single_sat(0x00);
      if(!did_init1) init_messages(3, 1);     // send init messages and request various data

#ifdef USE_X11
      while(0 && !SERIAL_DATA_AVAILABLE(RCVR_PORT)) {  // !!!!
         if(IDLE_SLEEP) Sleep(IDLE_SLEEP);
         check_com_timer(RCVR_PORT);
         get_x11_event();
         if(display) XFlush(display);
         if(KBHIT()) {
            break;
         }
      }
#endif
#ifdef USE_SDL
      while(0 && !SERIAL_DATA_AVAILABLE(RCVR_PORT)) {  // !!!!
         if(IDLE_SLEEP) Sleep(IDLE_SLEEP);
         check_com_timer(RCVR_PORT);
         get_sdl_event();
         if(KBHIT()) {
            break;
         }
      }
#endif
      if(SERIAL_DATA_AVAILABLE(RCVR_PORT)) {
         reset_com_timer(RCVR_PORT);
         find_msg_end();  // skip data till we see an end of message code
      }
   }
   else if(sim_file) {
      vidstr(0,0, YELLOW, "SIMULATION FILE IN USE         ");
   }
   else {                  // user has disabled the serial port
      vidstr(0,0, YELLOW, "SERIAL PORT PROCESSING DISABLED");
      refresh_page();
if(first_key == 0) Sleep(1000);
   }
}

void get_log_file()
{
int i;

   if(read_log[0]) {       // user specified a log file to read in
      first_key = ' ';
      i = reload_log(read_log, 1);
      if(i == 0) {         // log file found
         plot_review(0L);  // enter plot review mode to start plot at first point
         #ifdef ADEV_STUFF
            force_adev_redraw(5);    // and make sure adev tables are showing
         #endif
         pause_data = user_pause_data^1;
      }
      reset_first_key(1);
      prot_menu = 0;
      draw_plot(REFRESH_SCREEN);
   }
}


void clean_up()
{
   // leave receiver in a nice stand-alone state
   #ifdef PRECISE_STUFF
      abort_precise_survey(1);  // shut down any surveys in progress,  save best position
   #endif

   if(rcvr_type == VENUS_RCVR) {
      set_venus_mode(2);  // !!!!!!! vvvvvv put receiver into binary output mode so we can auto_detect it next time
   }

   if(debug_file) {
      fclose(debug_file);
      debug_file = 0;
      debug_name[0] = 0;
   }

   if(luxor) set_batt_pwm(0);

   #ifdef PLM
     kill_pl_timer();
   #endif
}


//
//
//   Serial terminal emulator stuff
//
//


#define MAX_TERM_ROWS     64
#define MAX_TERM_COLS     132
#define TERM_HEADER_LINES 5
#define TERM_CURSOR       '_'

char term_screen[MAX_TERM_ROWS][MAX_TERM_COLS+1];  // screen image buffer
int term_rows;     // screen size in chars
int term_cols;
int term_changed;  // flag set if screen buffer has been updated
int term_header_lines;  // number of lines in the screen help header

int term_echo;     // if set, echo keyboard chars to screen
int term_pause;    // if set, stop updating srceen
int term_crlf;     // if set, treat all CR or LF chars as CR LF


char term_kbd[MAX_TERM_COLS+1];  // keystroke buffer used to resend last kbd command
int tkbd_col;
int last_was_keybuf;
int blinker;            // used to blink keyboard cursor char


void show_term_screen(unsigned port)
{
int row;
int col;
int c;
int left;

   // copy the teminal screen image buffer to the screen

   left = 0;
   erase_screen();
   for(row=0; row<term_rows; row++) {
      for(col=0; col<term_cols; col++) {
         c = term_screen[row][col];
         c &= 0xFF;
         if((col == term_col) && (row == term_row)) {  // blinking cursor
            if(blinker) dot_char((col+left)*TEXT_WIDTH, row*TEXT_HEIGHT, TERM_CURSOR, WHITE);  // header lines
            else        dot_char((col+left)*TEXT_WIDTH, row*TEXT_HEIGHT, TERM_CURSOR, BLACK);  // header lines
         }
         if(c == 0) break;

         if(row < term_header_lines) dot_char((col+left)*TEXT_WIDTH, row*TEXT_HEIGHT, c&0x7F, WHITE);  // header lines
         else if(c & 0x80) {
            dot_char((col+left)*TEXT_WIDTH, row*TEXT_HEIGHT, c&0x7F, YELLOW);   // user input chars
         }
         else dot_char((col+left)*TEXT_WIDTH, row*TEXT_HEIGHT, c&0x7F, GREEN);  // received chars
      }
   }
   refresh_page();

   reset_com_timer(port);
   term_changed = 0;
}

void scroll_up(unsigned port)
{
int row;

   // scroll the terminal image buffer up a line and send it to the screen

   if(term_row >= (term_rows-1)) {  // scroll up a line
      for(row=term_header_lines; row<term_rows-1; row++) {
         strcpy(&term_screen[row][0], &term_screen[row+1][0]);
      }
      term_screen[term_rows-1][0] = 0;
   }

   ++term_row;
   if(term_row >= term_rows) {  // scroll up a line
      term_row = term_rows-1;
   }
   term_screen[term_row][0] = 0;

   show_term_screen(port);
// if((zoom_screen == 'H') && ZOOM_SLEEP) Sleep(ZOOM_SLEEP);
}

void init_term()
{
int row;

   // initialize the terminal image buffer

   term_rows = (SCREEN_HEIGHT/TEXT_HEIGHT) - 1;
   if(term_rows > MAX_TERM_ROWS) term_rows = MAX_TERM_ROWS;

   term_cols = (SCREEN_WIDTH/TEXT_WIDTH);
   if(term_cols > MAX_TERM_COLS) term_cols = MAX_TERM_COLS;

   for(row=0; row<term_rows; row++) term_screen[row][0] = 0;
}


void echo_term(unsigned port, int i, int hex, int highlight)
{
char s[UNIT_LEN+1];
int color;

   // put a character into the terminal image buffer.  Handle CR and LF chars
   if(port >= NUM_COM_PORTS) return;

   if(pause_monitor) return;

   color = 0x00;
   if((zoom_screen == 'O') || (zoom_screen == 'H')) {
      if(highlight) color = 0x80;
if(hex == 0) i &= 0x7F;
   }

   if(hex) {  // hex terminal display
      sprintf(s, "%02X", i);

      if(zoom_screen == 'O') ;
      else if(zoom_screen == 'H') ;
      else if(i == pkt_end1)   color = 0x80;  // highlight possible packet boundaries
      else if(i == pkt_end2)   color = 0x80;
      else if(i == pkt_start1) color = 0x80;
      else if(i == pkt_start2) color = 0x80;

      term_screen[term_row][term_col+0] = s[0] | color;
      term_screen[term_row][term_col+1] = s[1] | color;
      term_screen[term_row][term_col+2] = ' ';
      term_screen[term_row][term_col+3] = 0;
      term_col += 3;
      term_changed = 1;

      if(SCREEN_WIDTH < (36*3*TEXT_WIDTH)) {  // small screen - 16 vals per line
         if(term_col > (16*3)) {
            term_col = 0;
            scroll_up(port);
         }
      }
      else { // wide screen
         if(term_col == (16*3)) { // two groups of 16 vals per line
            term_screen[term_row][term_col++] = ' ';
            term_screen[term_row][term_col] = 0;
         }

         if(term_col > (16*6+1)) {
            term_col = 0;
            scroll_up(port);
         }
      }
   }
   else if((i & 0x7F) == 0x0D) {  // carriage return
      term_col = 0;
      if(term_crlf) scroll_up(port);
else if(rcvr_type == UCCM_RCVR) scroll_up(port);  // piss
else if(CRLF_RCVR) scroll_up(port);
else if(port == THERMO_PORT) scroll_up(port);
   }
   else if((i & 0x7F) == 0x0A) {  // line feed
      if(term_crlf) term_col = 0;
//else if(rcvr_type == UCCM_RCVR) term_col = 0;  // piss
else if(CRLF_RCVR) term_col = 0;
else if(port == THERMO_PORT) term_col = 0;
else if(port == TRACK_PORT) term_col = 0;
else if(zoom_screen == 'H') term_col = 0;
      scroll_up(port);
   }
   else if(((i & 0x7F) >= ' ') && ((i & 0x7F) < 0x7F)) { // add char to terminal image buffser
      term_screen[term_row][term_col+0] = ((i & 0xFF) | color);
      term_screen[term_row][term_col+1] = 0;
      term_changed = 1;

      if(++term_col >= term_cols) {
         term_col = 0;
         scroll_up(port);
      }
   }
}

void save_term_kbd(int i)
{
   // save a keyboard char into the last command buffer

   if(last_was_keybuf) {  // the last command we sent was the buffered keyboard command
      tkbd_col = 0;             // start buffering this command
      term_kbd[tkbd_col] = 0;
      last_was_keybuf = 0;
   }

   if(tkbd_col < (MAX_TERM_COLS-1)) {  // add kbd char to last command buffer
      term_kbd[tkbd_col++] = i;
      term_kbd[tkbd_col] = 0;
   }
}

void set_term_header(unsigned port)
{
   // setup the terminal screen header lines

                     strcpy(&term_screen[0][0], "=========================================================================");
   if((zoom_screen == 'O') || (zoom_screen == 'H')) {
      if(port == RCVR_PORT) {
                     strcpy(&term_screen[1][0], "   Lady Heather RCVR Port Data Monitor   Press any key to exit.");
      }
      else if(port == TICC_PORT) {
                     strcpy(&term_screen[1][0], "   Lady Heather TICC Port Data Monitor   Press any key to exit.");
      }
      else if(port == ECHO_PORT) {
                     strcpy(&term_screen[1][0], "   Lady Heather DATA ECHO Port Data Monitor   Press any key to exit.");
      }
      else if(port == NMEA_PORT) {
                     strcpy(&term_screen[1][0], "   Lady Heather NMEA ECHO Port Data Monitor   Press any key to exit.");
      }
      else if(port == TICC_ECHO_PORT) {
                     strcpy(&term_screen[1][0], "   Lady Heather TICC ECHO Port Data Monitor   Press any key to exit.");
      }
      else if(port == THERMO_PORT) {
                     strcpy(&term_screen[1][0], "   Lady Heather Environmental Port Data Monitor   Press any key to exit.");
      }
      else if(port == TRACK_PORT) {
                     strcpy(&term_screen[1][0], "   Lady Heather Tracking Port Data Monitor   Press any key to exit.");
      }
      else if(port == DAC_PORT) {
                     strcpy(&term_screen[1][0], "   Lady Heather DAC/ADC Port Data Monitor   Press any key to exit.");
      }
      else if(port == FAN_PORT) {
                     strcpy(&term_screen[1][0], "   Lady Heather Fan Port Data Monitor   Press any key to exit.");
      }
      else {
                     sprintf(out,               "   Lady Heather Port %d Data Monitor   Press any key to exit.", port);
                     strcpy(&term_screen[1][0], out);
      }
      if(pause_monitor) {
         strcpy(&term_screen[2][0], "        GREEN=from  YELLOW=to device    SPACE to continue) ");
      }
      else {
         strcpy(&term_screen[2][0], "        GREEN=from  YELLOW=to device    SPACE to pause) ");
      }
   }
   else {
                     strcpy(&term_screen[1][0], "   Lady Heather Terminal   END to exit   HOME to clear   F8 send break");
      if(term_pause) strcpy(&term_screen[2][0], "        F4 resume output   ");
      else           strcpy(&term_screen[2][0], "         F4 pause output   ");
      if(term_echo)  strcat(&term_screen[2][0], "F1 echo off   ");
      else           strcat(&term_screen[2][0], "F1 echo on    ");
      if(raw_file)   strcat(&term_screen[2][0], "F2 log off      ");
      else           strcat(&term_screen[2][0], "F2 log append   ");
      if(term_hex)   strcat(&term_screen[2][0], "F3 ASCII mode ");
      else           strcat(&term_screen[2][0], "F3 HEX mode   ");
   }
   strcpy(&term_screen[3][0], "=========================================================================");
   strcpy(&term_screen[4][0], "");

   term_header_lines = 4+1;
}

void set_monitor_mode(int kbd_flag)
{
   // enable monitor port screen

   change_zoom_config(26);
   zoom_screen = 'O';
   pause_monitor = 0;

   init_term();
   set_term_header(monitor_port);
   term_row = term_header_lines;
   term_col = 0;
   show_term_screen(monitor_port);  // draw the terminal screen image
}

void do_term(unsigned port)
{
int i;
double old_cto;

   // Interactive video terminal.  (added mainly to play with SCPI receivers)

   if(port >= NUM_COM_PORTS) return;
   if(com[port].process_com == 0) enable_terminal = 0;
   if(com[port].com_running < 0) enable_terminal = 0;
   if(enable_terminal == 0) return;

   old_cto = com[port].com_timeout;
   com[port].com_timeout = 250.0;
   in_terminal = 1;

   if(rcvr_type == CS_RCVR) strcpy(term_kbd, "SYST:PRINT?");
   else if((rcvr_type == SCPI_RCVR) || (rcvr_type == UCCM_RCVR)) {
      if(scpi_echo_mode) strcpy(term_kbd, ":SYST:COMM:SER1:FDUP 0");
      else               strcpy(term_kbd, "SYST:STAT?");
   }
   else term_kbd[tkbd_col] = 0;
   tkbd_col = strlen(term_kbd);
   last_was_keybuf = 0;

   term_echo = 1;   // echo input to the screen (in yellow)
// term_hex = 0;    // ascii mode
   term_pause = 0;  // send all data to the screeb
   term_header_lines = TERM_HEADER_LINES;  // length of the header area
   monitor_hex = 1;

   erase:
   init_term();

   new_header:
   set_term_header(port);

   term_row = term_header_lines; // start at top of screen
// term_row = term_rows-1;       // start at the screen bottom
   term_col = 0;
   show_term_screen(port);  // draw the screen image

   while(1) {    // the main processing loop...  repeat until exit requested
      serve_os_queue();

      if(KBHIT()) {       // key pressed
          i = get_kbd();  // get the keyboard character

          if     (i == END_CHAR)  break;        // exit the terminal
          else if(i == HOME_CHAR) goto erase;   // erase screen
          else if(i == F8_CHAR)   SendBreak(port);  // send a break
          else if(i == F1_CHAR) {               // toggle echo
             term_echo ^= 1;
             goto new_header;
          }
          else if(i == F2_CHAR) {               // toggle log
             if(raw_file) {
                fclose(raw_file);
                raw_file = 0;
                log_stream &= (~LOG_RAW_STREAM);
             }
             else {
                raw_file = open_raw_file(raw_name, "ab");
                log_stream |= LOG_RAW_STREAM;
             }
             goto new_header;
          }
          else if(i == F3_CHAR) {  // toggle hex display mode
             term_hex ^= 1;
             goto erase;
          }
          else if(i == F4_CHAR) {  // pause screen
             term_pause ^= 1;
             goto new_header;
          }
          else if(i == F5_CHAR) {  // crlf mode
             term_crlf ^= 1;
             goto new_header;
          }
          else if(i == UP_CHAR) {  // resend last keyboard command
             send_line:
             for(i=0; i<tkbd_col; i++) {
                sendout(port, term_kbd[i], EOM);
                echo_term(port, term_kbd[i] | 0x80, 0, 0);
             }
             last_was_keybuf = 1;
          }
#define BUFFERED_KBD buffered_term
//#define BUFFERED_KBD (rcvr_type == LPFRS_RCVR)
          else if(BUFFERED_KBD) {   // don't send line to com port until CR
             if(i == 0x0D) {        // since OSA4530 times out after 175 msecs between keystrokes
                save_term_kbd(0x0D);
                save_term_kbd(0x0A);
                goto send_line;
             }
             else goto buffer_kbd;
          }
          else {
             sendout(port, i, EOM);

             buffer_kbd:
             if(i == 0x08) {      // backspace key pressed
                if(tkbd_col) {    // remove keystroke from keyboard buffer
                   --tkbd_col;
                }
                term_kbd[tkbd_col] = 0;
                last_was_keybuf = 0;

                if(term_col) --term_col; // remove from screen image
                term_screen[term_row][term_col] = 0;
                show_term_screen(port);
             }
             else if(i == 0x0D) { // carriage return
                term_col = 0;
                scroll_up(port);
                if(rcvr_type != STAR_RCVR) last_was_keybuf = 1;
                else save_term_kbd(i);
             }
             else {
                save_term_kbd(i);  // save last keystroke in replay buffer
                i |= 0x80;         // echo input in yellow
                if(term_echo) echo_term(port, i, 0, 0);  // echo keystroke if echo enabled
             }
          }
      }
      else if(SERIAL_DATA_AVAILABLE(port)) {  // we have a char from the receiver
         i = get_com_char();       // get the char (and write to raw file if opened)  aaaahhhh - this needs to be passed term_port
         if(term_pause == 0) {     // send it to the screen unless screen is paused
            echo_term(port, i, term_hex, 0);
         }

      }
      else {  // nothing going on, Take a short snooze
         Sleep(10);
      }

      check_com_timer(port); // update screen every 250 msec
      if(com[port].com_data_lost) {         // time to copy screen buffer to the screen
         blinker ^= 1;            // blinking cursor
         show_term_screen(port);
         reset_com_timer(port);
      }
   }

   com[port].com_timeout = old_cto;
   reset_com_timer(port);
   in_terminal = 0;

   if((rcvr_type == SCPI_RCVR) || (rcvr_type == CS_RCVR)) {
      com[port].last_com_time = 0.0;   // force com timeout re-init
   }

   if(TICC_USED) {
      new_queue(RESET_ALL_QUEUES, 1);
   }

   erase_screen();
   sprintf(out, "Restarting... this may take around %d seconds...", (int) ((com[RCVR_PORT].com_timeout+0.999)/1000.0));
   vidstr(0,0, YELLOW, out);
   refresh_page();
   need_redraw = 1234;
}


void dump_stream(int init)
{
int row, col;
unsigned i;

   again:
   erase_screen();
   vidstr(0,0, GREEN, "Stream dump:");
   refresh_page();
   if(init) init_messages(4, 0);

   row = 2;
   col = 0;
   while(1) {
      serve_os_queue();

      if(SERIAL_DATA_AVAILABLE(RCVR_PORT)) {
         reset_com_timer(RCVR_PORT);
         i = get_serial_char(RCVR_PORT);
//       sprintf(out, "%02X(%c) ", i,i);
         sprintf(out, "%02X", i);
         vidstr(row,col, WHITE, out);
         refresh_page();
         col += 3;
         if(col >= 48) {
            col = 0;
            ++row;
            if(row > 32) {
               row = 2;
            }
         }
      }
      else {
         check_com_timer(RCVR_PORT);
      }

      serve_os_queue();
      if(KBHIT()) {
         i = GETCH();
         if(i == ESC_CHAR) {
            clean_up();
            error_exit(6666, "User pressed ESC during dump_stream()");
         }
         else goto again;
      }
   }
}


void x11_resize_check()
{
int mach;

   // if X11 screen, check if screen has been re-sized
   mach = 0;
   #ifdef __MACH__
      mach = 0;     // !!!! testing
   #endif

#ifdef USE_X11
   if(have_restore_size == 0) {
      SWAP(SCREEN_WIDTH,SCREEN_HEIGHT);
      restore_width = SCREEN_WIDTH;
      restore_height= SCREEN_HEIGHT;
      have_restore_size = 1;   // size came from initial screen init
      SWAP(SCREEN_WIDTH,SCREEN_HEIGHT);
   }

   if(need_resize && (this_button == 0) && (last_button == 0)) {
if(show_debug_info) printf("resizing screen:%dx%d\n", SCREEN_WIDTH,SCREEN_HEIGHT);
      SWAP(SCREEN_WIDTH,SCREEN_HEIGHT);
if(show_debug_info) printf("RESIZE to %dx%d  now:%dx%d  have_root:%d  mach:%d\n", new_width,new_height, SCREEN_WIDTH,SCREEN_HEIGHT, have_root_info, mach); // zork - show_debug_info
      need_resize = 0;
      if(new_width < MIN_WIDTH) new_width = MIN_WIDTH;
      if(new_width > MAX_WIDTH) new_width = MAX_WIDTH;
      if(new_height < MIN_HEIGHT) new_height = MIN_HEIGHT;


      if(mach || (new_width != SCREEN_WIDTH) || (new_height != SCREEN_HEIGHT)) {
         if(have_root_info && ((unsigned)(new_width+2) < display_width) && ((unsigned) (new_height+0) < display_height)) { // new window size is not near the screen size
            restore_width = new_width;
            restore_height = new_height;
            have_restore_size = 3;  // size came from resize event
            x11_maxed = 0;
if(show_debug_info) printf("new restore size 3: %dx%d\n", restore_width,restore_height); // zork - show_debug_infi
         }

         SCREEN_WIDTH = custom_width = new_width;
         SCREEN_HEIGHT = custom_height = new_height;
if(show_debug_info) printf("Resizing to %d x %d.\n", SCREEN_WIDTH,SCREEN_HEIGHT); // zork - show_debug_info
         screen_type = 'c';
//       strcpy(edit_buffer, "c");
         sprintf(edit_buffer, "%dx%d",SCREEN_WIDTH,SCREEN_HEIGHT);
         if(display) XFlush(display);
         keyboard_cmd = 1;
         edit_screen_res();
         keyboard_cmd = 0;
      }
      else SWAP(SCREEN_WIDTH,SCREEN_HEIGHT);
   }
#endif  // USE_X11
#if USE_SDL
       if(0 == sdl_scaling && need_resize && (this_button == 0) && (last_button == 0)) {
           printf("RESIZE to %dx%d  now:%dx%d  have_root:%d  mach:%d\n", new_width,new_height, SCREEN_WIDTH,SCREEN_HEIGHT, have_root_info, mach); // zork - show_debug_info
           need_resize = 0;
           restore_width = new_width;
           restore_height = new_height;
           have_restore_size = 3;  // size came from resize event
           SCREEN_WIDTH = custom_width = new_width;
           SCREEN_HEIGHT = custom_height = new_height;
           if(show_debug_info) printf("Resizing to %d x %d.\n", SCREEN_WIDTH,SCREEN_HEIGHT); // zork - show_debug_info
           screen_type = 'c';
           sprintf(edit_buffer, "%dx%d",SCREEN_WIDTH,SCREEN_HEIGHT);
           keyboard_cmd = 1;
           edit_screen_res();
           keyboard_cmd = 0;
       }
#endif
}

void do_gps()
{
int i;

   while(1) {    // the main processing loop...  repeat until exit requested
      x11_resize_check();     // see if screen has been re-sized

      fake_second_check(2);   // see if we should fake a missing time stamp second
      fine_tick_check();      // doing synchonized audible tick clock

      if(first_key) ;         // dont redraw screen while keyboard menu active
      else if(need_redraw) {
//sprintf(plot_title, "need redraw:%d", need_redraw);
         redraw_screen();
      }

      if(force_dump && unit_file_name) {   // forced screen dump
         dump_screen(invert_dump, 0, unit_file_name);
      }
      force_dump = 0;

      if(!serve_gps(0)) {  // time to exit the progaam
         break;
      }
      got_timing_msg = 0;  // used to prevent keyboard lockout if data comming in at high nav rates

      if(f11_flag) {  //// !!!! debug
         f11_flag = 0;
      }

      if(script_file && (script_pause == 0)) {  // reading chars from script file
         system_busy = 3;
         while(script_file) {  // process a full line of commands
            i = process_script_file();
            if((i == 0) || (i == 0x0D)) break;
            update_pwm();
         }
      }
      else if(force_si_cmd) {
         system_busy = 4;
         do_kbd('a');
      }
      else if(tide_kbd_cmd) {
         system_busy = 5;
         do_kbd('a');
      }
      else if(KBHIT()) {   // key pressed
         system_busy = 6;
         i = get_kbd();    // get the keyboard character
         i = do_kbd(i);    // process the character
         if(i) break;      // it's time to stop this madness
      }
      else if(sim_file && (sim_eof == 0)) {  // allow fast simulation file processing... use /sw if throttling or sleep needed
      }
      else if(idle_sleep && (set_system_time == 0)) {  // sleep a while when we are not busy to keep cpu usage down
         Sleep(idle_sleep);
      }

      process_extra_ports(6);  // handle data from extra serial ports

      // screen saver keyboard idle time check
      idle_time = (jd_utc - last_kbd_jd) * (24.0 * 60.0);
// sprintf(debug_text, "idle_timeout:%d  idle_time:%g  screen:%c  fk:%d", idle_timeout, idle_time*60.0, idle_screen, first_key);
      if(idle_timeout && idle_screen  && (zoom_screen == 0) && (first_key == 0)) {
         if((int) idle_time >= idle_timeout) {  // zoom to digital clock display
            add_kbd('Z');
            add_kbd(idle_screen);
         }
      }
   }
}


#ifdef PLM
   // This code was a failed attempt to monitor long term power line
   // frequency changes.  There does not appear to be a viable way to have a
   // callback routine called when the modem signal changes state.  So
   // this works by polling the RI modem control line for changes.
   // It uses a high-frequency (1KHz) timer event to poll the signal
   // for changes.  Unfortunately Windoze cannot guarantee the required
   // latency.
   HANDLE hTimer = NULL;
   HANDLE hTimerQueue = NULL;
   int    pl_arg = 1;
   DWORD  last_ring;

   VOID CALLBACK TimerRoutine(PVOID lpParam, BOOLEAN TimerOrWaitFired)
   {
   DWORD ring;
   DWORD dwModemStatus;

      if(!GetCommModemStatus(hSerial[RCVR_PORT], &dwModemStatus)) {
         return;  // Error in GetCommModemStatus;
      }

      ring = MS_RING_ON & dwModemStatus;
      if(ring != last_ring) {
         last_ring = ring;
         ++pl_counter;
      }
   //++pl_counter;
   }

   void kill_pl_timer()
   {
       // Delete all timers in the timer queue.
       if(hTimerQueue) {
          DeleteTimerQueue(hTimerQueue);
          timeEndPeriod(1);
       }
   }

   void init_pl_timer()
   {
       if(monitor_pl <= 0) return;

       timeBeginPeriod(1);

       hTimerQueue = CreateTimerQueue();
       if(hTimerQueue == NULL) {
          monitor_pl = (-1);
          return;
       }

       // Set a timer to call the timer routine every 5 msecs starting in 1 second.
       if(!CreateTimerQueueTimer( &hTimer, hTimerQueue,
               (WAITORTIMERCALLBACK)TimerRoutine, &pl_arg , 1000, 1, WT_EXECUTEINTIMERTHREAD)) {
          kill_pl_timer();
          monitor_pl = (-2);
       }
       return;
   }
#endif //PLM


int check_com_timer(unsigned port)
{
double to;

   // see if no data from com port for last com_timeout msecs

   update_pwm();
   if(port >= NUM_COM_PORTS) return 0;

   to = com[port].last_com_time + com[port].com_timeout;
   if(to < this_msec) {  // com data lost
      com[port].last_com_time = this_msec;
      if(com[port].com_data_lost == DATA_LOSS_REINIT) com[port].com_data_lost = DATA_LOSS_NO_INIT;
      else                                            com[port].com_data_lost = DATA_LOSS_REINIT;
   }

   return com[port].com_data_lost;
}

void reset_com_timer(unsigned port)
{
   // reset the com port data loss timer

   update_pwm();
   if(port >= NUM_COM_PORTS) return;

   com[port].last_com_time = this_msec;
   com[port].com_data_lost = 0;
}

void option_handle_hd(int argc, char ** argv)
{
    int argIndex;
    for(argIndex = 1; argIndex < argc; argIndex++) {
        char * arg = argv[argIndex];
        if((*arg == '/' || *arg == '-') && 0 == strncmp(arg+1,"hd",2) &&
           (*(arg+3) == '=' || *(arg+3) == ':')) {
            realpath(arg+4, heather_path);
            if(*(heather_path + strlen(heather_path)-1) != '/') {
                strcat(heather_path,"/");
            }
            printf("New heather_path of %s\n", heather_path);
        }
    }
}

int config_program(int argc, char *argv[])
{
int i;
int j;
int add_path;
int error;
char path[1024+1];

   set_defaults();     // initialize global variables

   add_path = 0;
   j = 0;
   path[j] = 0;

#ifdef WINDOWS
      strcpy(help_path, &argv[0][0]);
#else  // __linux__  __MACH__  __FreeBSD__
   strcpy(path, argv[0]);
   strcpy(help_path, basename(path));
   if(strstr(help_path,"luxor")) {
//    luxor = 2;
   }

   // get program name (help path) and executable path (heather_path)
   #ifdef __MACH__
      char temp[1024+1];
      j = sizeof(temp);
      i = _NSGetExecutablePath(temp, (unsigned int *) &j);
      if(i == 0) {
         realpath(path, temp);
      }
   #else
      i = readlink("/proc/self/exe", path, sizeof(path));
   #endif

   if(i < 0) readlink("/proc/curproc/file", path, sizeof(path));
   if(i < 0) readlink("/proc/self/path/a.out", path, sizeof(path));
   if(i >= 0) {
      strcpy(heather_path, dirname(path));
      strcat(heather_path, "/");
   }
   add_path = 1;
#endif

   // Look for /hd option for setting the home directory
   // We need to do this before looking for all the other files
   // and so we have to do it before other option parsing. We'll
   // call a special routine to do this one thing for us.

    option_handle_hd(argc, argv);

	printf("excecutable path:%s\n", heather_path);  // zork - show_debug_info
	printf("program:%s\n", help_path);

   find_sound_files();

   read_config_file(help_path, 1, add_path);  // process options from default config file
   read_config_file(help_path, 2, add_path);  // process calendar file

   for(i=1; i<argc; i++) {          // process the command line
      if((argv[i][0] == '/') || (argv[i][0] == '-') || (argv[i][0] == '=') || (argv[i][0] == '$')) {
         error = option_switch(argv[i]);
      }
      else error = 1;

      if(error) {
         command_help("on command line: ", argv[i], help_path);
         return 1;
//qqq    exit(10);
      }
   }

   return 0;
}


void config_tz_info()
{
char *s;

   // If user did not set the time zone, try to get time zone info from
   // system environment variable.

   if(time_zone_set) return;  // user set the string

   s = getenv("TZ");
   if(s == 0) s = getenv("tz");
   if(s == 0) return;

   strncpy(out, s, 128);
   out[127] = 0;

   set_time_zone(out);
}

int read_default_lla()
{
FILE *file;
int i;

   // get default location from a file if the device does not have a GPS

   lat = 0.0;
   lon = 0.0;
   alt = 0.0;
   i = (-1);
   file = fopen(LOCN_FILE, "r");
   if(file) {
      while(fgets(out, sizeof out, file) != NULL) {
         strupr(out);
         if(strstr(out, "LOC")) ;
         else {
            i = parse_lla(out);
            if(i == LLA_OK) {
               lat = precise_lat;
               lon = precise_lon;
               alt = precise_alt;
               default_lat = lat;
               default_lon = lon;
               default_alt = alt;
               ref_lat = lat;
               ref_lon = lon;
               ref_alt = alt;
               cos_factor = cos(ref_lat);
               if(cos_factor == 0.0) cos_factor = 0.001;
               break;
            }
         }
      }

      fclose(file);
      file = 0;
   }

   return i;
}

int main(int argc, char *argv[])
{
   #ifdef USE_X11
      if(argv[0]) strcpy(exe_name, argv[0]);
      printf("argv[0] (%s)\n", argv[0]);
   #endif

   config_program(argc, argv);  // process command line arguments

   if(1) ;
   else if(((rcvr_type == NO_RCVR) || (rcvr_type == TIDE_RCVR)) && (com[RCVR_PORT].process_com == 0)) {
      com[RCVR_PORT].process_com = 1;
      com[RCVR_PORT].user_disabled_com = 0;
   }

   #ifdef WIN_VFX
     get_screen_size(0);
     if(display_width == 1280) {
        vfx_full:
        if(vfx_fullscreen) {
           SAL_set_preference(SAL_MAXIMIZE_TO_FULLSCREEN, YES);
           initial_window_mode = VFX_TRY_FULLSCREEN;
        }
     }
     else if(display_width == 1024) goto vfx_full;
     else if(display_width == 800)  goto vfx_full;
     else if(display_width == 640)  goto vfx_full;
   #endif

   if(jpl_clock) {
      if(dst_ofs)            time_color = DST_TIME_COLOR;
      else if(time_zone_set) time_color = DST_TIME_COLOR;  // !!! std_time_color is rather dim for the big clock
      else                   time_color = WHITE;
   }

   if(luxor) {
      config_luxor_plots();
   }

   find_endian();     // determine machine byte ordering
   alloc_memory();    // allocate memory for plot and adev queues, etc
   adjust_view();     // tweak screen for user selected view interval, etc


   init_hardware();   // initialize the com port, screen, and any other hardware
   hw_setup = 1;
   update_pwm();

   if(temp_script) {  // heather.cfg created a temporary keyboard script file, process it
      fclose(temp_script);
      temp_script = 0;
      open_script(TEMP_SCRIPT);
   }

   if(take_a_dump) {
      dump_stream(1);
   }

   #ifdef PLM
      init_pl_timer();// setup to monitor power line freq
   #endif
   config_tz_info();  // try to get time zone info from the system
   config_options();  // configure system for command line options

   // user set a log param on the command line, start writing the log file
   if(user_set_log) {
      log_file_time = log_interval + 1;
      open_log_file(log_mode);
      rinex_header_written = 0;
   }

   if(need_debug_log) {
      need_debug_log = 0;
      if(debug_file) fclose(debug_file);
      debug_file = 0;
      debug_name[0] = 0;

      sprintf(out, "debug.log");
      open_debug_file(out);
   }

   if(need_rinex_file) {
      need_rinex_file = 0;
      if(rinex_file) fclose(rinex_file);
      rinex_file = 0;
      rinex_name[0] = 0;

      set_rinex_name();  // into string "out"
      if(out[0]) open_rinex_file(out);
   }

   if(need_raw_file) {  // open raw receiver log file specified on the command line
      need_raw_file = 0;
      if(raw_file) fclose(raw_file);
      raw_file = 0;
      sprintf(raw_name, "%s.raw", "heather");
      raw_file = open_raw_file(raw_name, "wb");
      if(raw_file) log_stream |= LOG_RAW_STREAM;
   }

   if(need_prn_file) {  // open raw receiver log file specified on the command line
      need_prn_file = 0;
      if(prn_file) fclose(prn_file);
      prn_file = 0;
      sprintf(prn_name, "%s.prn", "heather");
      prn_file = open_prn_file(prn_name, "wb");
   }

   if(need_ticc_file) {  // open ticc log file specified on the command line
      need_ticc_file = 0;
      if(ticc_file) fclose(ticc_file);
      ticc_file = 0;
      sprintf(ticc_name, "%s.raw", "ticc");
      ticc_file = topen(ticc_name, "wb");
// aaattt      if(raw_file) log_stream |= LOG_RAW_STREAM;
   }

   if(detect_rcvr_type) {
      auto_detect(0);
      if(rcvr_type == TICC_RCVR) {  // xyzzy  disable watch/azel/signals
         reset_com_timer(RCVR_PORT);
         init_screen(5395);
         reset_com_timer(RCVR_PORT);
not_safe = 3;
      }
   }

   if(ticc_type == PICPET_TICC) {  // scale PICPET wrap interval to 10 MHz clock equivalent
      if(rcvr_type == TICC_RCVR) {
         pet_clock = (double) com[RCVR_PORT].baud_rate / 19200.0;
         if(timestamp_wrap == 0.0) timestamp_wrap = 100.0 / pet_clock;
      }
      else if(ticc_port_open()) {
         pet_clock = (double) com[TICC_PORT].baud_rate / 19200.0;
         if(timestamp_wrap == 0.0) timestamp_wrap = 100.0 / pet_clock;
      }
   }

   if(NO_SATS || (rcvr_type == TIDE_RCVR)) {  // try to get default user position from file
      read_default_lla();
   }

   if(enviro_mode()) {
      config_enviro_plots(1);
   }

   if(user_set_title_bar) { // set title bar text
      set_title_bar(0);
   }
   else if(unit_file_name == 0) {
   }
   else if(unit_file_name[0]) {
      strcpy(szAppName, unit_file_name);
      strupr(szAppName);
szAppName[0] = 0;
      user_set_title_bar = 2;
      set_title_bar(2);
   }

   if(TICC_USED) no_adev_flag = 0;
   else if(0 && no_adev_flag) {
      free_mtie();
      free_adev_queues();

      adev_q_size = 0;
      // disable adevs... if you re-enable them you will need to use
      // the /j command to reset the adev period to a non-zero value!!!!!
      adev_q_size = 1L;
      adev_period = 0.0;
      pps_adev_period = adev_period;
      osc_adev_period = adev_period;
      chc_adev_period = adev_period;
      chd_adev_period = adev_period;
      save_adev_period();
   }

   plot_axes();         // draw graph axes
   if(luxor) reset_luxor_wdt(1);
   show_version_header();
   show_log_state();    // show logging state
   show_com_state();    // show com port state and flush buffer
   get_log_file();      // read in any initial log file
   read_rpn_file(rpn_name, 1);  // read in any calculator user defined functions

   if(monitor_mode) {
      set_monitor_mode(0); // port monitor
   }
   else if(enable_terminal) {
      do_term(term_port);  // debug terminal
      if(rcvr_type == NO_RCVR) goto exit_pgm;
      if(rcvr_type == TERM_RCVR) goto exit_pgm;
   }

   reset_com_timer(RCVR_PORT);

   if(need_view_auto) {     // force a VT auto view command
      add_kbd('v');
      add_kbd('t');
      add_kbd(0x0D);
   }

   if(calc_rcvr) {
      start_calc_zoom(1);
   }

   do_gps();            // run the receiver until something says stop

   exit_pgm:
   clean_up();          // leave the receiver in a nice stand-alone state
   shut_down(0);        // clean up remaining details and exit
   return 0;
}


#ifdef WINDOWS       // startup code

//****************************************************************************
//
// Exit handlers must be present in every SAL application
//
// These routines handle exits under different conditions (exit() call,
// user request via GUI, etc.)
//
//****************************************************************************

static int exit_handler_active = 0;

void WINAPI WinClean(void)
{
   if(exit_handler_active) {
      return;
   }

   exit_handler_active = 1;

   SAL_shutdown();
}

void WINAPI WinExit(void)
{
   if(!exit_handler_active) {
      WinClean();
   }
   exit(0);
}

void AppExit(void)
{
   if(!exit_handler_active) {
      WinClean();
   }
   return;
}

long FAR PASCAL WindowProc(HWND   hWnd,   UINT   message,   //)
                           WPARAM wParam, LPARAM lParam)
{
static int count = 0;

   switch(message) {
      case WM_KEYDOWN:
         {
         switch (wParam) {
            case VK_HOME:   add_kbd(HOME_CHAR);  break;
            case VK_UP:     add_kbd(UP_CHAR);    break;
            case VK_PRIOR:  add_kbd(PAGE_UP);    break;
            case VK_LEFT:   add_kbd(LEFT_CHAR);  break;
            case VK_RIGHT:  add_kbd(RIGHT_CHAR); break;
            case VK_END:    add_kbd(END_CHAR);   break;
            case VK_DOWN:   add_kbd(DOWN_CHAR);  break;
            case VK_NEXT:   add_kbd(PAGE_DOWN);  break;
            case VK_INSERT: add_kbd(INS_CHAR);   break;
            case VK_DELETE: add_kbd(DEL_CHAR);   break;
            case VK_CANCEL: break_flag = 1;      break;
            case VK_F1:     add_kbd(F1_CHAR);    break;
            case VK_F2:     add_kbd(F2_CHAR);    break;
            case VK_F3:     add_kbd(F3_CHAR);    break;
            case VK_F4:     add_kbd(F4_CHAR);    break;
            case VK_F5:     add_kbd(F5_CHAR);    break;
            case VK_F6:     add_kbd(F6_CHAR);    break;
            case VK_F7:     add_kbd(F7_CHAR);    break;
            case VK_F8:     add_kbd(F8_CHAR);    break;
            case VK_F9:     add_kbd(F9_CHAR);    break;
            case VK_F10:    add_kbd(F10_CHAR);   break;  // note: F10 and F11 intercepted by WIN_VFX
//          case VK_F10:
            case VK_F11:
               f11_flag = 1;
               break;
            case VK_F12:
               add_kbd(0);
               break;
            }

            break;
         }

      case WM_CHAR:
         {
            add_kbd(wParam);
            break;
         }

      case WM_CLOSE:
         {
            break_flag = 1;
            return 0;
         }


      case WM_TIMER:
         {
            timer_serve++;
            ++count;
            if(0 && ((count % 1) == 0) && ((rcvr_type == STAR_RCVR) && (star_type == NEC_TYPE))) ;  // NEC_RCVR has issues with serve_gps() here
            else if(0 && ((count % 1) == 0) && (rcvr_type == ACRON_RCVR)) ;  // NEC_RCVR has issues with serve_gps() here
            else if((timer_serve == 1) && (reading_log == 0) && (detecting == 0))  {
               serve_gps(1);
            }
            timer_serve--;
            break;
         }
   }

   return DefWindowProc(hWnd, message, wParam, lParam);
}

//****************************************************************************
//
// Windows main() function
//
//****************************************************************************
int main(int argc, char *argv[]);

int PASCAL WinMain(HINSTANCE hInst,
                   HINSTANCE hPrevInst,
                   LPSTR     lpCmdLine,
                   int       nCmdShow)
{
#define MAX_ARGS 60
char *s;

   //
   // Initialize system abstraction layer -- must succeed in order to continue
   //

   VFX_io_done = 1;
   hInstance = hInst;


   //
   // Get current working directory and make sure it's not the Windows desktop
   // (We don't want to drop our temp files there)
   //
   // If it is, try to change the CWD to the current user's My Documents folder
   // Otherwise, leave the CWD alone to permit use of the "Start In" field on a
   // desktop shortcut
   //
   // (Also do this if the current working directory contains "Program Files")
   //

   C8 docs[MAX_PATH] = "";
   C8 desktop[MAX_PATH] = "";

   SHGetSpecialFolderPath(HWND_DESKTOP,
                          desktop,
                          CSIDL_DESKTOPDIRECTORY,
                          FALSE);

   SHGetSpecialFolderPath(HWND_DESKTOP,
                          docs,
                          CSIDL_PERSONAL,
                          FALSE);

   C8 CWD[MAX_PATH] = "";

   if(GetCommandLine()) {
      if(strlen(GetCommandLine()) < MAX_PATH) {
         strcpy(CWD, GetCommandLine());
         strcpy(exe_name, CWD);
         s = strchr(exe_name, ' ');
         if(s) *s = 0;

         strlwr(CWD);
         if(strstr(CWD,"luxor")) {
//          luxor = 3;
         }
      }
   }

   if(GetCurrentDirectory(sizeof(CWD), CWD)) {
       _strlwr(CWD);
       _strlwr(desktop);

      if((!_stricmp(CWD,desktop)) ||
            (strstr(CWD,"program files") != NULL)) {
         SetCurrentDirectory(docs);
         strcpy(CWD, docs);
      }
      if(strstr(CWD,"luxor")) {
//       luxor = 4;
      }
   }

   //
   // Pass Windows command line as DOS argv[] array
   //

   static char *argv[MAX_ARGS];

   S32 argc = 1;
   strcpy(heather_path, CWD);
   strcat(heather_path, "\\");
   strcpy(root, CWD);
   root_len = strlen(root);

   strcat(root, "\\heather.exe");
   argv[0] = &root[0];
//  argv[0] = _strdup("heather.exe");

   C8 cli[MAX_PATH];
   strcpy(cli, lpCmdLine);

   C8 *src = cli;


   for(argc=1; (argc < MAX_ARGS) && (src != NULL); argc++) {  // build command line arg vector
      if(*src == 0) break;
      while(*src == ' ') ++src;

      C8 *next = NULL;
      C8 *term = strchr(src,' ');

      if(term != 0) {
         *term++ = 0;
         next = term;
      }

      argv[argc] = _strdup(src);
      src = next;
   }

   if(luxor) {
      strcpy(szAppName, "Luxor Power/LED Analyzer Control Program - "VERSION"");
   }


   SAL_set_preference(SAL_USE_PARAMON, NO);

   if(!SAL_startup(hInstance, szAppName, TRUE, WinExit)) {
      return 0;
   }

   //
   // Create application window
   //

   SAL_set_preference(SAL_ALLOW_WINDOW_RESIZE, YES);
   SAL_set_preference(SAL_MAXIMIZE_TO_FULLSCREEN, NO);
   SAL_set_preference(SAL_USE_DDRAW_IN_WINDOW, NO);

   SAL_set_application_icon((C8 *) IDI_ICON);

   hWnd = SAL_create_main_window();

   if(hWnd == NULL) {
      SAL_shutdown();
      return 0;
   }

   initial_window_mode = VFX_WINDOW_MODE;

   //
   // Register window procedure
   //

   SAL_register_WNDPROC(WindowProc);
   SAL_show_system_mouse();

   //
   // Register exit handler and validate command line
   //

   atexit(AppExit);


   return main(argc, argv);   // go do it
}

#endif  // WINDOWS


#ifdef WINDOWS
BOOL CALLBACK CLIHelpDlgProc (HWND   hDlg,
                              UINT   message,
                              WPARAM wParam,
                              LPARAM lParam)
{
   switch (message) {
      case WM_INITDIALOG:
         {
         //
         // Center dialog on screen
         //

         SAL_FlipToGDISurface();

         S32 screen_w = GetSystemMetrics(SM_CXSCREEN);
         S32 screen_h = GetSystemMetrics(SM_CYSCREEN);

         LPNMHDR pnmh = (LPNMHDR) lParam;

         RECT r;

         GetWindowRect(hDlg, &r);

         r.right  -= r.left;
         r.bottom -= r.top;

         r.left = (screen_w - r.right)  / 2;
         r.top  = (screen_h - r.bottom) / 2;

         MoveWindow(hDlg, r.left, r.top, r.right, r.bottom, TRUE);

         //
         // Set caption for window
         //

         SetWindowText(hDlg, "Help for Command Line Options");

         //
         // Set initial control values
         //

         HWND hDT = GetDlgItem(hDlg, IDC_CMDHELP);

         SetWindowText(hDT, (C8 *) lParam);

         SendMessage(hDT, WM_SETFONT, (WPARAM) GetStockFont(ANSI_FIXED_FONT), (LPARAM) true);

         return TRUE;
         }

      case WM_COMMAND:
         {
         S32 ID = LOWORD(wParam);

         switch (ID) {
            case IDOK:
               {
                  EndDialog(hDlg, 1);
                  #ifdef WIN_VFX
                     if(downsized) do_fullscreen();
                  #endif
                  return TRUE;
               }

            case IDREADME:
               {
                  EndDialog(hDlg, 0);
                  #ifdef WIN_VFX
                     if(downsized) do_fullscreen();
                  #endif
                  return TRUE;
               }
            }
            break;
         }
   }

   return FALSE;
}

//****************************************************************************
//
// Launch HTML page from program directory
//
//****************************************************************************

void WINAPI launch_page(C8 *filename)
{
   C8 path[MAX_PATH];

   GetModuleFileName(NULL, path, sizeof(path)-1);

   _strlwr(path);

   C8 *exe = strstr(path,"heather.exe");

   if(exe != NULL) {
      strcpy(exe, filename);
   }
   else {
      strcpy(path, filename);
   }

   ShellExecute(NULL,    // hwnd
               "open",   // verb
                path,    // filename
                NULL,    // parms
                NULL,    // dir
                SW_SHOWNORMAL);
}

#endif


int help_line;
int show_escape;

void show(char *s)
{
int old_text_mode;
int old_zoom;
int old_plot_text_row;
int old_edit_row;
char xout[MAX_TEXT_COLS+1];    // screen output strings get formatted into here


   if(show_escape) return;  // user interrupted the show with ESC
   strcpy(xout, s);

   if(keyboard_cmd) {
      old_text_mode = text_mode;
      old_zoom = zoom_screen;
      old_plot_text_row = PLOT_TEXT_ROW;
      old_edit_row = EDIT_ROW;
      if(old_text_mode) {
         PLOT_TEXT_ROW = 20;
         EDIT_ROW = 20+2;
      }
      text_mode = 2;
      no_x_margin = no_y_margin = 1;
      cancel_zoom(9);

      if(help_line > (PLOT_TEXT_ROW-4)) {
         showing_cmd_help = 1;  // so touchscreen can work
         if(edit_error("") == ESC_CHAR) show_escape = 1;
         showing_cmd_help = 0;
         erase_screen();
         help_line = 1;
      }
      else if(help_line == 0) erase_screen();
      vidstr(help_line, 0, WHITE, xout);

      ++help_line;
      text_mode = old_text_mode;
      if(text_mode == 0) {
         no_x_margin = no_y_margin = 0;
      }
      zoom_screen = old_zoom;
      PLOT_TEXT_ROW = old_plot_text_row;
      EDIT_ROW = old_edit_row;
   }
   else printf("%s\n", xout);
}


void show_help_msg(char *s)
{
unsigned i;
char c;

   // show() the help_msg[] line by line

   i = 0;
   out[i] = 0;
   while(*s) {
      c = *s++;
      if(c == 0) {
         break;
      }
      if(c == '\r') c = ' ';

      if(c == '\n') {
         show(out);
         i = 0;
      }
      else if(i < (sizeof(out)-2)){
         out[i++] = c;
         out[i] = 0;
      }
   }
}

void command_help(char *where, char *s, char *cfg_path)
{
unsigned long x;
int fs_help;

// need to add:  /tsc /aj /pw /pwa /pwb /im? /it? /if? commands !!!!!!!!

    x = 0;
    help_line = 0;
    show_escape = 0;
    showing_help = 1;
    downsized = 0;
    fs_help = 0;

#ifdef WIN_VFX
    if(sal_ok) {  // SAL package has been initialized
       if(SAL_window_status() == SAL_FULLSCREEN) {
          // (it's extremely painful to make GDI dialogs work properly in DirectDraw fullscreen mode, so we don't try)
//        do_windowed();  // try to go windowed
          if(SAL_window_status() == SAL_FULLSCREEN) {
             fs_help = 1;
//             downsized = 0;
//             erase_plot(ERASE_PLOT_AREA);
//             edit_error("Sorry, startup command line help dialog is not available in fullscreen mode");
//             showing_help = 0;
//             return;
          }
       }
    }
#endif

    static char help_msg[32768] = {
         "Lady Heather's GPS Disciplined Oscillator Control Program\r\n"
         "Version " VERSION " - " __DATE__ " " __TIME__ "\r\n"
         "\r\n"
         "Copyright (C) 2008-2018 Mark S. Sims - Released under MIT License.\r\n"
         "This program is offered AS-IS with NO WARRANTY as to its accuracy,\r\n"
         "usability, functionality, or fitness for any purpose... enjoy!\r\n"
         "\r\n"
         "Original Windows port from DOS version and TCP/IP support by John Miles.\r\n"
         "Adev code from Tom Van Baak's ADEV3.C\r\n"
         "Incremental ADEV mods to ADEV3.C by John Miles\r\n"
         "Temperature and oscillator control algorithms by Warren Sarkison.\r\n"
         "See the comments at the start of the file heather.cpp for other contributors\r\n"
         "algorithm sources, and program command descriptions.\r\n"
         "\r\n"
#ifdef WINDOWS
         "Startup command line options should be placed on the TARGET line in the\r\n"
         "Lady Heather program PROPERTIES or in the HEATHER.CFG file.  Most can also\r\n"
         "be executed with the keyboard / command.\r\n"
         "\r\n"
         "The directory to use for Heather's log and configuration files can\r\n"
         "be changed with the \"Start In\" property.  This can be useful if\r\n"
         "Heather was installed in the system directory path on Windows versions\r\n"
         "that implement enhanced system security features that prevent writing\r\n"
         "to files in a system directory.\r\n"
         "\r\n"
#else //__linux__  __MACH__   __FreeBSD__
         "Most command line options can also be executed with the keyboard / command.\r\n"
#endif
         "\r\n"

         "Valid command line options are:\r\n"
         "   /0               - disable com port processing (if just reading a log file)\r\n"

#ifdef WINDOWS
         "   /1..99           - use COM1 .. COM99 for receiver i/o\r\n"
#endif
#ifdef __MACH__
         "   /1..99           - use /dev/ttys(#-1) for receiver i/o\r\n"
         "   /1u..99u         - use /dev/tty.usbserial(#-1) for receiver i/o\r\n"
         "   /999u            - use /dev/heather for receiver i/o\r\n"
         "   /id=dev_name     - use macOS device dev_name for receiver i/o\r\n"
#endif
#ifdef __linux__
         "   /1..99           - use /dev/ttyS(#-1) for receiver i/o\r\n"
         "   /1u..99u         - use /dev/ttyUSB(#-1) for receiver i/o\r\n"
         "   /999u            - use /dev/heather for receiver i/o\r\n"
         "   /id=dev_name     - use Linux device dev_name for receiver i/o\r\n"
#endif
#ifdef __FreeBSD__
         "   /1..99           - use /dev/cuau(#-1) for receiver i/o\r\n"
         "   /1u..99u         - use /dev/cuaU(#-1) for receiver i/o\r\n"
         "   /999u            - use /dev/heather for receiver i/o\r\n"
         "   /id=dev_name     - use FreeBSD device dev_name for receiver i/o\r\n"
#endif
         "   /ig=gpsd_device  - set the specific GPSD device name to access\r\n"

#ifdef TCP_IP
         "   /ip=addr[:port#] - connect to TCP/IP server instead of local COM port\r\n"
#endif

         "   /a[=#]           - number of points to calc Adevs over (default=432000)\r\n"
         "                      If 0,  then all adev calculations are disabled.\r\n"
         "   /ae              - toggles display of error bars in the ADEV plots\r\n"
         "   /ah=meters       - set RINEX file antenna height (also e/w and n/s displacements)\r\n"
         "   /ak=marker       - set RINEX file marker name\r\n"
         "   /an=number       - set RINEX file antenna number (can be alphanumeric)\r\n"
         "   /at=antenna      - set RINEX file antenna type\r\n"
         "   /av=antenna      - set RINEX file marker number (can be alphanumeric)\r\n"
         "   /as[=#]          - set ADEV calculation bin spacing sequence\r\n"
         "   /ax              - enable display of all adev type plots\r\n"
         "   /axa             - toggle display of ADEV plots\r\n"
         "   /axh             - toggle display of HDEV plots\r\n"
         "   /axm             - toggle display of MDEV plots\r\n"
         "   /axt             - toggle display of TDEV plots\r\n"
         "   /axp             - toggle display of PPS/chA xDEV plots\r\n"
         "   /axp             - toggle display of OSC/chB xDEV plots\r\n"
         "   /axc             - toggle display of chC  xDEV plots\r\n"
         "   /axd             - toggle display of chD  xDEV plots\r\n"
         "   /b[=#]           - set daylight savings time area (1=USA,2=EURO,3=AUST,4=NZ)\r\n"
         "   /b=nth,start_day,month,nth,end_day,month,hour - set custom DST rule\r\n"
         "                      day:0=Sun..6=Sat  month:1=Jan..12=Dec\r\n"
         "                      nth>0 = from start of month  nth<0 = from end of month\r\n"
         "   /br[=#]          - set serial port configuration. (default 9600:8:n:1)\r\n"
         "   /bs              - set time display to solar time\r\n"
         "   /bt              - start up in terminal emulator mode\r\n"
         "   /c[=#]           - set Cable delay to # ns (default=77.03 ns (50 ft std coax))\r\n"
         "   /c=#f              set Cable delay in feet of 0.66Vp coax\r\n"
         "   /c=#m              set Cable delay in meters of 0.66Vp coax\r\n"
         "   /ce              - toggle automatic com port data loss error recovery mode\r\n"
         "   /ct[=#]          - set all com port data loss timeout thresholds to # msecs (min=3000)\r\n"
         "   /ct?[=#]         - set port '?' data loss timeout threshold to # msecs (min=3000)\r\n"
         "   /cm=color,r,g,b  - set color palette table entry 'color' (0..15)\r\n"
         "   /co              - toggle adding clock offset to RT17 observation time stamps\n"
#ifdef GREET_STUFF
         "   /d#              - show dates in calendar #\r\n"
         "                      A)fghan   haaB)     C)hinese  D)ruid   H)ebrew\r\n"
         "                      I)slamic  J)ulian   K)urdish  M)jd    iN)dian\r\n"
         "                      P)ersian  iS)o      T)zolkin  boliV)isn   W)ISO week format\r\n"
         "                      X)iuhpohualli       maY)an    aZ)tec Tonalpohualli\r\n"
#endif
         "   /de[=#]          - set debug information level\r\n"
         "   /dl[=file]       - write debug information log file\r\n"
         "   /dr[=file]       - write raw receiver data capture file (default heather.raw)\r\n"
         "   /dq[=file]       - write TICC device raw data capture file (default ticc.raw)\r\n"
         "   /e?[=port]       - enable extra com port\r\n"
         "                      ?: D=DAC  E=ECHO  F=FAN  I=TICC  K=NMEA  N=ENVIRONMENTAL sensor\r\n"
         "                         T=TRACKING     R=RECEIVER\r\n"
         "   /e               - do not log message/time Errors and state changes\r\n"
#ifdef USE_X11
         "   /f               - start in Fullscreen mode\r\n"
#endif
         "   /f[psakit]       - toggle Pv,Static,Altitude,Kalman,Ionosphere,Troposphere filter\r\n"
         "   /fd[=#]          - set display filter count to # seconds (default=10)\r\n"
         "   /fg              - toggle GPX format for scheduled log dumps\r\n"
         "   /fx              - toggle XML format for scheduled log dumps\r\n"
         "   /g[#]            - toggle Graph enable (#= a,b,c,d,e,h,l,m,o,p,s,t,u,x,z)\r\n"
         "                      (Adevs  Both map and adev tables  sat_Count  Dac  Errors\r\n"
         "                       Holdover  K(constallation changes)    hide Location\r\n"
         "                       Map,no adev tables   Osc  Pps  R(RMS info)   Sound  Temperature\r\n"
         "                       Watch  X(dops)  Z(clock)  J(el mask)  \r\n"
         "                       N(disable holiday greetings)  Q(signal quality map)\r\n"
         "                       0) .. 9) toggle plots G0..G9   V)toggle LLA plots\r\n"
         "   /hd=directory    - use specified \"home\" directory instead of location of executable\r\n"
         "   /h               - show command line help\r\n"
         "   /i[=#]           - set plot Interval to # seconds (default=24 hour mode)\r\n"
         "   /id=#            - set Linux/macOS/FreeBSD input device\r\n"
         "   /ip=#            - set TCP/IP input device address\r\n"
         "   /ir              - set read-only mode for receiver commands\r\n"
         "   /is              - set read-only mode for serial port\r\n"
         "   /ix              - set no-pollng mode for receiver data\r\n"
         "   /if[=freq]       - set time interval counter nominal freq (all channels)\r\n"
         "   /ifa[=freq]      - set time interval counter channel A nominal freq\r\n"
         "   /ifb[=freq]      - set time interval counter channel B nominal freq\r\n"
         "   /ifc[=freq]      - set time interval counter channel C nominal freq\r\n"
         "   /ifd[=freq]      - set time interval counter channel D nominal freq\r\n"
         "   /im#             - set time interval counter data mode\r\n"
         "                      #=F Frequency  I=interval  P=period  T=timestamp\r\n"
         "   /it#             - set time interval counter type\r\n"
         "                      #=C generic counter  H=HP531xx  L=Lars GPSDO  P=PICPET  T=TAPR TICC\r\n"
         "   /j[=#]           - set all ADEV queue sample periods to # seconds (default=10 seconds)\r\n"
         "   /ja[=#]          - set chA (PPS) ADEV queue sample period to # seconds (default=10 seconds)\r\n"
         "   /jb[=#]          - set chB (OSC) ADEV queue sample period to # seconds (default=10 seconds)\r\n"
         "   /jc[=#]          - set chC ADEV queue sample period to # seconds (default=10 seconds)\r\n"
         "   /jd[=#]          - set chD ADEV queue sample period to # seconds (default=10 seconds)\r\n"
//       "   /jw              - enable JPL wall clock mode on start-up\r\n"
         "   /kb              - toggle Beep sounds\r\n"
         "   /kc              - toggle writes to Config EEPROM\r\n"
         "   /ke              - toggle quick exit with ESC key\r\n"
         "   /kj              - toggle showing sun and moon in satellite maps\r\n"
         "   /km              - toggle mouse enable\r\n"
         "   /kq              - toggle Keyboard enable\r\n"
         "   /ks              - toggle Sound files\r\n"
         "   /kt              - toggle Windows dialog/message timer\r\n"
         "   /kv              - toggle touch screen keyboard enable\r\n"
         "   /k?[=#]          - set temp control PID parameter '?'\r\n"
         "   /l[=#]           - write Log file every entry # seconds (default=1)\r\n"
         "   /lc              - don't write any comments in the log file\r\n"
         "   /ld              - write signal level comments in the log file\r\n"
         "   /lo              - enable reading of old format log files\r\n"
         "   /lh              - don't write timestamp headers in the log file\r\n"
         "   /li              - don't wait for receiver id before logging data\r\n"
         "   /ls              - change log file value separator from tab to a comma\r\n"
         "   /m[=#]           - Multiply all plot scale factors by # (default is to double)\r\n"
         "   /ma              - toggle Auto scaling\r\n"
         "   /mb              - toggle mapping of all mouse buttons to left-click\r\n"
         "   /md[=#]          - set DAC plot scale factor (microvolts/divison)\r\n"
         "   /mi              - invert pps and temperature plots\r\n"
         "   /mo[=#]          - set OSC plot scale factor (parts per trillion/divison)\r\n"
         "   /mp[=#]          - set PPS plot scale factor (nanoseconds/divison)\r\n"
         "   /mt[=#]          - set TEMPERATURE plot scale factor (millidegrees/divison)\r\n"
         "   /m0../m9[=#]     - set scale factors of the other plots (units/divison)\r\n"
         "   /n=hh:mm:ss      - exit program at specified time (optional: /n=month/day/year)\r\n"
         "   /na=hh:mm:ss     - sound alarm at specified time (optional: /na=month/day/year)\r\n"
         "   /na=#?           - sound alarm every #s secs,  #m mins,  #h hours  #d=days\r\n"
         "   /na=#?o          - sound alarm Once in #so secs,  #mo mins,  #ho hours  #d=days\r\n"
         "   /nc=hh:mm:ss     - run program at specified time (optional: /n=month/day/year)\r\n"
         "   /nc=#?           - run program every #s secs,  #m mins,  #h hours  #d=days\r\n"
         "   /nc=#?o          - run program Once in #so secs,  #mo mins,  #ho hours  #d=days\r\n"
         "   /nd=hh:mm:ss     - dump screen at specified time (optional: /n=month/day/year)\r\n"
         "   /nd=#?           - dump screen every #s secs,  #m mins,  #h hours  #d=days\r\n"
         "   /nd=#?o          - dump screen Once in #so secs,  #mo mins,  #ho hours  #d=days\r\n"
         "   /ne              - disable file editing and execution commands\r\n"
         "   /nl=hh:mm:ss     - dump log at specified time (optional: /n=month/day/year)\r\n"
         "   /nl=#?           - dump log every #s secs,  #m mins,  #h hours  #d=days\r\n"
         "   /nl=#?o          - dump log Once in #so secs,  #mo mins,  #ho hours  #d=days\r\n"
         "   /np=hh:mm:ss     - run timer.scr at specified time (optional: /np=month/day/year)\r\n"
         "   /np=#?           - run timer.scr every #s secs,  #m mins,  #h hours  #d=days\r\n"
         "   /nr[=#]          - force receiver navigation rate to # Hz (default=1)\r\n"
         "   /nf              - toggle fast keyboard script mode\r\n"
         "   /nt              - attempt to wake up Nortel NTGxxxx receivers\r\n"
         "   /nx=hh:mm:ss     - exit program at specified time (optional: /n=month/day/year)\r\n"
         "   /nx=#?o          - exit program in #s secs,  #m mins,  #h hours  #d=days\r\n"
#ifdef ADEV_STUFF
         "   /o[#]            - select ADEV type (#=A,H,M,T, O,P)\r\n"
         "                      Adev  Hdev  Mdev  Tdev  O=all osc types  P=all pps types\r\n"
#endif
         "   /p               - toggle PPS output signal enable\r\n"
         "   /pd              - disable PPS output signal\r\n"
         "   /pe              - enable PPS output signal\r\n"
         "   /po=lat,lon,alt  - force posiiton to lat,lon,altitude\r\n"
         "   /pt[=title]      - set title bar string (use '_' for spaces)\r\n"
         "   /pw[=#]          - set time interbal counter phase wrap interval (default=100.0E-9 seconds)\r\n"
         "   /q[=#]           - set size of plot Queue in seconds (default=3 days)\r\n"
         "   /qf[=#]          - set max size of FFT (default=4096)\r\n"
         "   /r[=file]        - Read file (default=tbolt.log)\r\n"
         "                      .log   .xml  .gpx (log files)\r\n"
         "                      .scr=script  .lla=lat/lon/altitude\r\n"
         "                      .adv=adev    .tim=ti.exe time file\r\n"
         "   /rb              - toggle showing of reason code for beeps\r\n"
         "   /rm[=types]      - set receiver raw measurment types to write to RINEX files\r\n"
         "                      (C1,P1,P2,L1,L2,D1,D2,S1,S2)\r\n"
         "   /rr[=secs]       - set receiver raw satellite data message output rates (seconds)\r\n"
         "   /ro[=#]          - add # seconds to the GPS receiver date/time\r\n"
         "                      or /ro says 1024 weeks,  /ro=2* says 2048 weeks, etc\r\n"
         "   /ri=file[,seek]  - get input from counter data simulation file\r\n"
         "   /rs=file[,seek]  - get input from raw receiver data simulation file\r\n"
         "   /rt[=#]          - use Resolution-T serial port config (9600,8,ODD,1)\r\n"
         "                      [#=1]=force Resolution-T  [#=2]force Resolution SMT\r\n"
         "   /rx#[=leapsecs]  - set receiver type # (A=Acron  B=Brandywine  D=Datum  E=NEC  F=RFTG-m\r\n"
         "                      C=UCCM     CP=UCCM-P    CS=UCCM Samsung           CU=UCCM\r\n"
         "                      G=GPSD     I=TAPR TICC  H=5071A  J=Jupiter        K=Z3811A        L=LTE Lite\r\n"
         "                      M=Motorola   N=NMEA     O=TruePosition   P=TAIP   R=Resolution-T  S=SIRF\r\n"
         "                      T=TSIP  U=Ublox V=Venus VB=Venus Base    VR=Venus Rover  VP=Motorola VP\r\n"
         "                      X=system clock  Y=SCPI-(NORTEL)  Z=SCPI-(Z3801A)  0=Gravity clock\r\n"
         "                      1=HP531xx       3=Zyfer Nanosync 380     4=Oscilloquartz Star-4     12=Z12\r\n"
         "                      45=OSA453x      5=SCPI-(Z3816A, HP5xxxx, etc)    6=SV6/8    8=NVS   17=RT17 fmt\r\n"
         "                      AI=ACE-III      AT=Acutime      AG=Acutime GG    A3=Acutime 360     PA=Palisade\r\n"
         "                      ES=Furuno ESIP  FU=Furuno PFEC    LP=LPFRS/RMO   TM=Spectrum TM4    TS=TymServe\r\n"
         "                      PR=PRS10        SA=60 Mhz SA22.c  SB=58.9824 MHz SA22.c   SY=X72/SA22.c\r\n"
         "                      SS=Novatel Superstar II           CS=Samsung UCCM         EN=Environmental\r\n"
         "                      CA=Calculator   TB=buffered keyboard terminal emulator    TE=terminal emulator\r\n"
         "                      For devices that do not report a valid leapsecond\r\n"
         "                      count you can specify the value to use (e.g. /rxlp=18)\r\n"
#ifdef USE_SDL
         "   /sc              - use window scaling instead of resizing\r\n"
#endif
//       "   /sf              - enter 2D/3D fix mode and map fixes\r\n"
         "   /si[=#]          - set maximum number of displayed satellites to #\r\n"
         "   /so[=#]          - set outline shape to show sat map sats in\r\n"
         "                      (0=round  1=square  2=round with wings  3=rectangular with wings)\r\n"
         "   /sp[=#]          - do Precison Survey (# hours,  default=48/max=96)\r\n"
         "   /sr[=#]          - enable sunrise/set display\r\n"
         "   /ss[=#]          - do Self Survey (# fixes,  default=2000)\r\n"
         "   /st              - toggle drawing of satellite position trails\r\n"
         "   /sw[=#]          - Sleep() for # milliseconds after proceesing\r\n"
         "                      each message when reading a simulation file (default=100)\r\n"
         "   /ta              - show dates in European dd.mm.yyyy format.\r\n"
         "   /tl              - show dates in ISO yyyy.mm.dd format.\r\n"
         "   /tb              - do not label the analog watch face.\r\n"
         "   /tb=string       - set analog watch face brand name.\r\n"
         "   /tc              - show Celcius temperatures\r\n"
         "   /td              - show DeLisle temperatures\r\n"
         "   /te              - show Reaumur temperatures\r\n"
         "   /tf              - show Fahrenheit temperatures\r\n"
         "   /tg              - sync outputs to GPS Time (default is UTC time)\r\n"
         "   /th[=#]          - chime clock mode.  Chimes # times per hour.\r\n"
         "                      Tries to play heather_chime.WAV,  else uses alarm sound.\r\n"
         "   /th[=#H]         - cuckoo clock mode.  Sings .WAV files # times per hour.\r\n"
         "                      Tries to play heather_chime.WAV,  else uses alarm sound.\r\n"
         "                      Chimes the hour number on the hour,  one chime at other times.\r\n"
         "   /th[=#S]         - singing chime clock mode.  Sings .WAV files # times per hour.\r\n"
         "                      Tries to play souns file heather_songxx.WAV (xx=minute)\r\n"
         "   /th=1B           - enables ships bells clock mode.\r\n"
         "   /th=0B           - disables ships bells clock mode.\r\n"
         "   /th=3T           - enables ticking second / minute beep clock mode.\r\n"
         "   /th=2T           - enables minute beep clock mode.\r\n"
         "   /th=1T           - enables ticking second clock mode.\r\n"
         "   /th=0T           - disables ticking clock modes.\r\n"
         "   /ti              - toggle 12/24 hour digital clock mode\r\n"
         "   /tj              - don't remove effects of tbolt firmware temperature smoothing\r\n"
         "   /tk              - show Kelvin temperatures\r\n"
         "   /tlg             - show altitude in linguini\r\n"
         "   /tm              - show altitude in meters\r\n"
         "   /tn              - show Newton temperatures\r\n"
         "   /to              - show Romer temperatures\r\n"
         "   /tp              - show time as fraction of a day\r\n"
         "   /tph             - show temperature in degrees Paris Hilton\r\n"
         "   /tq              - show time as total seconds of the day\r\n"
         "   /tr              - show Rankine temperatures\r\n"
         "   /ts[odhm]        - set operating system time to UTC (once,daily,hourly,every minute)\r\n"
         "   /tsa[=#]         - set operating system time to UTC anytime difference exceeds # msecs\r\n"
         "   /tsx[=#]         - compensate for delay between 1PPS output and receiver timing message\r\n"
         "                      Value is in milliseconds.  Default is a +45 millisecond delay\r\n"
         "   /tsg             - toggle show digital clock as GPS epoch seconds\r\n"
         "   /tsj             - toggle show digital clock Julian date.time\r\n"
         "   /tsk             - toggle show digital clock Modifed Julian date.time\r\n"
         "   /tsn             - show digital clock in 24 hours mode\r\n"
         "   /tsu             - toggle show digital clock as Unix epoch seconds\r\n"
         "   /tsz             - toggle show digital clock with milliseconds\r\n"
         "   /tt=#            - set active temp control setpoint to # degrees C\r\n"
         "   /tu              - sync outputs to UTC Time (default is UTC time)\r\n"
         "   /twi[=secs]      - Set timestamping counter reading wrap interval in seconds\r\n"
         "   /tw[=#]          - Sleep() for # milliseconds when idle (default=10)\r\n"
         "   /tx              - show osc values with eXponent (default is ppb/ppt)\r\n"
         "   /ty[=#]          - show temperatures to # decimal places\r\n"
         "   /tz=#SSSSS/DDDDD - show time at local time zone - #=gmt offset\r\n"
         "                      SSSSS=standard time zone id   DDDDD=daylight time zone id\r\n"
         "                      (note:  western hemisphere # is negative:  /T=-6CST/CDT)\r\n"
         "   /t'              - show altitude in feet\r\n"
         "   /t\"              - use degrees.minutes.seconds for lat/lon\r\n"
         "   /u               - toggle plot/adev queue Updates\r\n"
         "   /uc=val          - force UTC delatT values\r\n"
         "   /ud=val          - set oscillator disciplining damping value\r\n"
         "   /ut=val          - set oscillator disciplining time constant value\r\n"
         "   /ug=val          - set oscillator disciplining gain value\r\n"
         "   /up=val          - set UCCM GPSDO pullin range\r\n"
         "   /ui=val          - set oscillator disciplining initial dac value\r\n"
         "   /uh=val          - set oscillator maximum allowed DAC value\r\n"
         "   /ul=val          - set oscillator minimum allowed DAC value\r\n"
         "   /un=val          - set oscillator disciplining minimum dac value\r\n"
         "   /uo=val          - force UTC-GPS leapsecond offset\r\n"
         "   /ux=val          - set oscillator disciplining maximum dac value\r\n"
         "   /vt              - Text only Video screen\r\n"
         "   /vd              - Ultra-small experimental (480x320) Video screen\r\n"
         "   /ve              - Ultra-small experimental (320x480) Video screen\r\n"
         "   /vu              - Undersized (640x480) Video screen\r\n"
         "   /vp              - PI (800x480) Video screen\r\n"
         "   /vs              - Small (800x600) Video screen\r\n"
         "   /vr              - Reduced (1024x600) Video screen\r\n"
         "   /vm              - Medium (1024x768) Video screen (default)\r\n"
         "   /vn              - Netbook (1000x540) Video screen\r\n"
         "   /vj              - (1280x800) Video screen\r\n"
         "   /vk              - (1280x960) Video screen\r\n"
         "   /vl              - Large (1280x1024) Video screen\r\n"
         "   /vv              - Very large (1400x900) Video screen\r\n"
         "   /vx              - eXtra large (1680x1050) Video screen\r\n"
         "   /vh              - Huge (1920x1080) Video screen\r\n"
         "   /vz              - 2048x1536 Video screen\r\n"
         "   /vc=colsXrows    - custom screen size (e.g. /vc=1200x800)\r\n"
         "   /va              - start in VT (auto-scale plot view) mode\r\n"
         "   /vf              - start in Fullscreen mode\r\n"
         "   /vi              - invert black and white on screen\r\n"
         "   /vo              - rotate screen image in window\r\n"
         "   /vq[=scale]      - enable scaled vector font mode (scale=50.500%)\r\n"
         "   /w=file          - set log file name to Write (default=tbolt.log)\r\n"
         "   /wa=file         - set log file name to Append to (default=tbolt.log)\r\n"
         "   /wf=[#]          - set watch face type (0=Roman  1=Arabic  2=Stars\r\n"
         "                      add 3 for 24 hour mode)\r\n"
         "   /wh              - toggle writing log file as an ASCII hex dump file)\r\n"
         "   /wp=prefix       - set prefix to use on scheduled log/screen dump file names\r\n"
         "   /wq              - toggles log flush mode for open log files\r\n"
         "   /wt[=#]          - watch fance hand shape (0=straight, 1=filled, 2=hollow)\r\n"
         "   /x=#             - set experimental oscillator disiplining PID value\r\n"
         "   /y               - optimize plot grid for 24 hour display (/y /y = 12hr)\r\n"
         "   /y=#             - set plot view time to # minutes/division\r\n"
         "   /zs?             - start up with zoomed full screen display '?'\r\n"
         "   /z[#][=val]      - toggle or set graph zero line ref value (#=d,o,p,t,0..9)\r\n"
         "                      (D)ac volts  (O)sc   (P)ps ns  (T)temp deg  (0)..(9)other plots\r\n"
         "   /+               - sync PPS signal rising edge to time\r\n"
         "   /-               - sync PPS signal falling edge to time\r\n"
         "   /^               - toggle OSC signal edge referenced to the PPS signal\r\n"
         "   /^f              - set OSC signal falling edge referenced to the PPS signal\r\n"
         "   /^r              - set OSC signal rising edge referenced to the PPS signal\r\n"
    };

#ifdef WINDOWS   // show help in dialog box
   if(path_help == 0) {
      if(strchr(s,'?') == NULL) {
         strcat(help_msg, "\r\nInvalid option seen: ");
         strcat(help_msg, where);
         strcat(help_msg, s);
         strcat(help_msg, "\r\n");
      }

      if(cfg_path) {
         strcat(help_msg, "\r\nPut heather.cfg file in directory ");
         cfg_path[root_len] = 0;
         strcat(help_msg, cfg_path);
         strcat(help_msg, "\r\n");
      }
      path_help = 1;
   }

   if(fs_help) {  // showing help in full screen mode - can't use dialog box
      show_help_msg(help_msg);
      if(keyboard_cmd) {
         BEEP(18);
         help_line = 999;  // hacky way to wait for a key-press
         show("");
      }
   }
   else if(!DialogBoxParam(hInstance,
                       MAKEINTRESOURCE(IDD_CMDHELP),
                       hWnd,
                       CLIHelpDlgProc,
                       (LPARAM) help_msg)) {
      launch_page("readme.htm");
   }
#else // __linux__ __MACH__  __FreeBSD__ show help as simple text screens
   show_help_msg(help_msg);

   if(strchr(s,'?') == NULL) {
     sprintf(out, "Invalid option seen: %s%s", where, s);
     show(out);
   }

   if(cfg_path) {
      show("");
      sprintf(out, "Put heather.cfg file in directory: %s", cfg_path);
      show(out);
   }
   if(keyboard_cmd) {
      BEEP(19);
      help_line = 999;
      show("");
      redraw_screen();
   }
   else {
      shut_down(999);
   }
#endif // WINDOWS

   showing_help = 0;

   if(exit_flag) {
      shut_down(9999);
   }
}
