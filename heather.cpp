//
// Lady Heather's GPS Disciplined Oscillator Control Program
// (now works with many different receiver types and even without a GPS receiver)
//
// Copyright (C) 2008-2016 Mark S. Sims
//
//
// Lady Heather is a monitoring and control program for GPS receivers and
// GPS Disciplined Oscillators.  It is oriented more towards the time keeping
// funtionality of GPS and less towards positioning. It supports numerous GPS
// (and Glonass, Beidou, Galileo, etc) devices.
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
// Win32 port by John Miles, KE5FX (jmiles@pop.net)
// Help with Mac OS X port by Jeff Dionne and Jay Grizzard
//   (note: OS/X version uses the XQuartz package for X11 display support)
//
// Temperature and oscillator control algorithms by Warren Sarkison
//
// Original adev code adapted from Tom Van Baak's adev1.c and adev3.c
// Incremental adev code based upon John Miles' TI.CPP
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
// The LZW .GIF encoder used in this file was derived from code written by Gershon
// Elber and Eric S. Raymond as part of the GifLib package.  And by Lachlan
// Patrick as part of the GraphApp cross-platform graphics library.
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
//   the end of the string.  DEL deletes the character at the cursor.  DOWN 
//   arrow deletes to the end of the line.  UP arrow deletes to the start
//   of the line. BACKSPACE deletes the character before the cursor.
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
//   once heather has finished initializing the hardware, etc).  See the
//   description of keyboard script files below.
//
//   You can also read in a ".cfg" file from the keyboard "R" menu or from
//   the command line:
//      /h=file  - reads a .cfg configuration file. These config files are
//                 processed after the default "heather.cfg" file has been
//                 processed.  You should not include a "/h=" command in a .cfg
//                 file since reading config files do not nest.
//
//
//   At a minimum most users will want to configure the com port, receiver
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
//      file must have an extension of ".SCR"  Script files mimic typing
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
//      To read a script file use the "R" keyboard command or "/r" command
//      line option and specify a file name with an extension of ".scr"
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
//      /2 says to use /dev/ttyS1 on Linux
//      /2 says to use /dev/ttys1 on macOS
//      /999 says to use /dev/heather
//
//   Windows treats USB connected serial adapters as standard hardware serial
//   ports.  Most Linux and macOS users will be using USB serial converters.
//   These operating systems treat the USB devices differently than hardware
//   serial ports.  For these system you can use the "/#u" command line option.
//      /1u says to use /dev/ttyUSB0 on Linux systems and
//      /1u says to use /dec/tty.usbserial on macOS.
//      /2u says to use /dev/ttyUSB1 on Linux systems and
//      /1u says to use /dec/tty.usbserial1 on macOS.
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
//   If a baud rate parameter is not given, Heather will use a default value
//   that is most commonly used for the receiver type it is using... which may
//   not be the same as what your receiver is configured for!
//
//
//
//
//
//-CONFIGURING THE RECEIVER TYPE:
//
//   Heather supports numerous receiver types.  If you do not specify a receiver
//   type Heather will attempt to automatically determine the receiver type.
//
//   Auto-detection requires the receiver to be actively sending data that
//   can be analyzed.  Some receivers power up "mute" and do not automatically
//   send data.  You must "wake up" these receivers first by specifying the
//   receiver type.  
//
//   If the receiver can work in both a NMEA and a native
//   binary format, it probably powers up in NMEA.  Use the
//   proper /rx# command show below to put the receiver into native binary
//   mode.  Binary mode offers the user full control of the receiver
//   and better monitoring options.  The "!m" keyboard command can
//   switch most receivers back to NMEA mode.  On some receivers
//   it swicthes the device to its alternate language.
//
//
//   You can force the receiver type with the "/rx" command line option.
//      /rx  - auto-detect receiver type (default)
//      /rxa - Acron Zeit WWVB receiver - 300:8:N:2
//      /rxc - UCCM - Trimble / Symmetricom GPSDOs - 57600:8:N:1
//      /rxd - DATUM STARLOC II GPSDO - inferior wannabe Thunderbolt
//      /rxe - NEC GPSDO ... STAR-4 compatible at (115200:8:N:1)
//      /rxg - GPSD interface (mainly a Linux thing - /ip=localhost:2947)
//      /rxj - Jupiter-T  (aka Zodiac)
//      /rxk - Lucent KS24361 REF0/Z3811A (19200:8:N:1)
//      /rxm - Motorola binary 
//      /rxn - NMEA
//      /rxr - Trimble Resolution T family with odd parity
//      /rxs - Sirf binary
//      /rxt - Trimble TSIP binary, 
//      /rxu - Ublox UBX binary, 
//      /rxv - Venus mixed binary / NMEA (115200:8:N:1), 
//      /rxx - No receiver, uses system clock. 
//      /rxy - SCPI - Nortel telecom GPSDOs like NTWB and NTPX
//      /rxz - SCPI (Z3801A style. 19200:7:O:1)
//      /rx5 - SCPI (HP5xxxx style)
//      /rx4 - Oscilloquartz STAR-4 GPSDO (9600:8:N:2)
//      /rx8 - NVS binary (115200:8:N:1)
//
//   /rx says to auto-detect the receiver type. This tries
//   to find the receiver type at 9600:8:N:1, then
//   115200:8:N:1, then 57600:8:N:1, then 19200:7:O:1 ...
//   unless the user specified the com port parameters
//   (with the /br=... command) before the /rx option.
//
//   /rx (auto detect) is the default mode if no /rx? command
//   is given on the command line.
//
//   Note that the auto-detect routine is a bit simplistic
//   and might occasionally mis-recognize the receiver type.
//
//
//   You can force or set the UTC leapsecond offset with the /rx? commands
//   like /rxx=17 (sets the leapsecond offset to 17 seconds)
//
//
//   If you explicity specify the receiver type, Lady Heather
//   defaults to the baud rate indicated above (or 9600:8:N:1 if
//   not shown above).  If your receiver is configured for
//   a different baud rate, specify the serial port settings
//   to use with the /br= command line option.
//
//
//   Special note for Lucent KS24361 units: the Z3811A
//   (REF1) unit talks SCPI at /br=19200:8:N:1 and the
//   Z3812A (REF0) unit talks at /br=9600:8:N:1 over the
//   DIAGNOSTIC ports.  Lady Heather's auto detect routine
//   works with the REF0, but to talk to the REF1/Z3811A 
//   you MUST force the baud rate with /br=19200:8:N:1
//   or it will try to use 19200:7:O:1. If you use the
//   /rxz command with these units you MUST first use the
//   /br= command to force the baud rate (since /rxz
//   defaults to 19200:7:O:1). For the Z3811A/REF1 you
//   should use the /rxk command to force SCPI at 19200:8:N:1
//  
//   The Datum STARLOC II cannot be auto-detected. Firmware
//   bugs cause it to stop outputting time messages. Always
//   specify "/rxd" when running STARLOC II receivers.  Also
//   note that the Datum STARLOC II has NUMEROUS firmware
//   bugs and generates lots of duplicate/missing time stamp
//   errors.
//
//   GPSD is a Linux service that provides a standardized
//   interface to numerous models of GPS receivers. It is
//   accessed via TCP/IP address "localhost:2947"  GPSD cannot be
//   auto-detected.  Use "/rxg" to force GPSD mode.
//
//   The Acron Zeit WWVB receiver cannot be auto-detected.  Always use
//   the /rxa command.  Support for this receiver is a hack.  Heather shows
//   it as tracking sat PRN 1. You should manually enter your lat/lon/altitude
//   (/po=lat,lon,alt command line option or the SL keyboard command) and 
//   the utc offset (like /rxa=17 or /uo=17 command line options). If you
//   don't enter your position, sat PRN 1 is shown at az=1, el=89.
//   Technically the Acron Zeit speaks at 300:7:O:2, but works with
//   300:8:N:2 if you strip off the received parity bit (which Heather does).
//   Also, it takes several seconds for Heather to startup and shutdown with
//   this device under Windows.
//
//
//   Special note for Trimble telecom GPSDOs like the NTWB and NTPX... these
//   devices can speak either TSIP binary (/rxt) or SCPI (/rxy).  TSIP provides
//   much more information and a much more robust interface.  Auto-detect will
//   find these units as TSIP devices.
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
//
//   The Trimble Resolution-T TEP model is configured to speak Motorola binary
//   (rather poorly).  The !M keyboard command will switch it to TSIP mode.
//   A factor reset (!H) will do a factory reset back to Motorola mode.
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
//   Also Heather needs to know your location so that the
//   sun and moon info and the astronomical time formats will work.
//   To set your lat/lon/alt use:
//      SL              - from the keyboard
//      /po=lat,lon,alt - from the command line
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
//   time zone ID is given,  the program does not do
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
//   the TZ command
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
//   "UTC offset".  This is a count of the number of leap seconds that that have
//   accumulated since the GPS system was first implemented.  It is the number
//   of seconds of difference between GPS time and UTC time.  Lady Heather
//   normally automatically gets this value from the GPS receiver.  It is part
//   of the GPS almanac data which can take over 15 minutes to arrive from the
//   GPS satellites.  Until the UTC offset is received, Heather will display
//   a "NO UTC OFFSET" warning.  
//
//   Some receiver data formats (like NMEA) do not provide the UTC offset value.
//   You can specify the value to use with the "/uo=" command line option. You 
//   can also force the UTC offset value with the "/rx#=offset" command line
//   option.  For instance with a NMEA receiver use "/rxn=17"  User specified
//   UTC offset values are shown in YELLOW.
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
//
//       MAR      - Mars date and time (official)
//       MARS     - Mars date and time (official)
//       MSD        Mars date and time (official)
//       MTC        Mars date and time (official)
//       AMT        Mars date and time (official)
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
//   Also supported are various planetary times: Mars ("MAR" or "MSD"),
//   Venus ("VEN"), Mercury ("MER"), and Pluto ("PLU").  Mars time is based
//   upon the NASA definition of Mars time.  Mercury, Venus, and Pluto time
//   are based upon the number of revolutions of the planet around the sun
//   since 1 Jan 1900 (J1900).
//
//   Sorry, no time for Uranus... or Jupiter or Saturn...
//   these are big blobs of smelly gasses with no
//   fixed rotation time.
//
//
//
//   Besides specifying your time zone you should also specify your
//   Daylight Savings Time (aka Summer Time) calculation method.
//   The default daylight savings time switching dates are 
//   the US standard.  The "/b=" command line option lets you specify
//   your daylight savings time calculation.
//
//   Use "/b=1" for USA, "/b=2" for UK/Europe, "/b=3" for
//   Australia or "/b=4" for New Zealand.  "/b=0" turns off
//   daylight savings time conversions.  
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
//
//-CONFIGURING THE VIDEO SCREEN SIZE:
//
//   Lady Heather works best with a screen size of at least 1024x768 pixels.
//   This is the default screen mode.  You can specify other fixed screen modes
//   or a custom screen size with the "/v#" command line option or via the '$'
//   keyboard menu.  The keyboard menu also allows you to tweak the size of the
//   text font (Tiny=8x8  Small=8x12  Medium=8x14  Large=8x16). 
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
//
//      /vt           - Text only Video screen
//      /vu           - Undersized (640x480) Video screen
//      /vs           - Small (800x600) Video screen
//      /vm           - Medium (1024x768) Video screen (default)
//      /vn           - Netbook (1000x540) Video screen
//      /vl           - Large (1280x1024) Video screen
//      /vv           - Very large (1440x900) Video screen
//      /vx           - eXtra large (1680x1050) Video screen
//      /vh           - Huge (1920x1080) Video screen
//      /vc=colsXrows - custom screen size (e.g. /vc=1200x800)
//      /vf           - startup in (nearly) full screen window mode - this
//                      should not be confused with true full screen mode.
//
//      /vi           - invert black and white on screen
//
//   The "$" keyboard menu also lets you change the screen resolution.  The
//   command letters are the same as used with the "/v" commands.  Note that
//   under Windows if you select a display size that is larger than your
//   screen, Heather will down-scale the screen by dropping pixels/lines...
//   which usually looks awful.  
//
//   Changing the screen size from the keyboard may cause Heather to drop / add
//   items from the display depending upon if it can find space to show them.
//
//   The "/vi" command line option and "$i" keyboard commands swap WHITE and
//   BLACK on the screen.  It looks horrible, but can be useful for doing
//   screen dumps that will be printed on paper... you remember paper, don't you?
//
//   The "G~" keyboard command lets you configure the global screen color 
//   palette. Heather uses a palette of 14 colors plus BLACK and WHITE.  
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
//   If you change the queue size from the keyboard (using the "/"
//   keyboard command) the queue data will be erased.  Changing the plot queue
//   update interval does not automatically erase the queue data, but the
//   plot time horizontal scale factor indication (like VIEW: 5 min/div) will
//   not be correct for data that was captured before the update interval was
//   changed.
//
//   You can clear the plot queue via the "C" keyboard menu.
//
//   You can disable updating of the plot queue with new data:
//      U  - from the keyboard toggles plot queue updating
//      /u - from the command line toggles plot queue updating
//
//
//
//
//
//-GPS WEEK ROLLOVER ISSUES
//   
//   The GPS system does not actually broadcast a date.  It does broadcast the
//   number of weeks that have elapsed since the GPS system was first activated.
//   However this week counter is only 1024 weeks (around 19 years) long.  Once
//   the week counter passes 1024 weeks, the date sent by the GPS receiver will
//   be wrong (like 20 years in the past wrong).  
//
//   Modern GPS receivers attempt to get around this limitation in various ways
//   which may delay when the rollover error occurs, but it will eventually
//   happen. If Lady Heather sees a date before 2016 from the receiver, it will
//   attempt to fix it by adding 1024 weeks to the GPS date/time until the date
//   appears to be reasonable.  If Lady Heather has "fixed" the GPS date, the
//   letters "ro" appear after the date in the upper left corner of the string
//   and the date will be shown in YELLOW.  Before Heather will attempt to 
//   automatically adjust the date for rollover errors, the bogus date must
//   persist for at least 15 seconds AND the GPS receiver must have the 
//   "UTC offset" value from the GPS receiver.
//
//   You can force a rollover compensation value with the "/ro=" command line
//   option.  The rollover correction value can be either the number of 1024
//   week cycles to add to the date, or the number of SECONDS to add to 
//   the date.
//
//        /ro        - apply 1 1024 week rollover to the date 
//        /ro=1*     - apply 1 1024 week rollover to the date
//        /ro=2*     - apply 2 1024 week rollovers to the date
//        /ro=12345  - add 12345 seconds to the date
//        /ro=0      - disable automatic GPS date rollover correction
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
//   receivers support all of these filters.
//
//     /fa - toggle altitude filter
//     /fi - toggle ionospheric filter (Motorola)
//     /fk - toggle Kalman filter
//     /fp - toggle PV filter (position/velocity filter)
//     /fs - toggle static filter
//     /ft - toggle tropospheric filter (Motorola)
//
//
//     /fd=# - display filter - the display filter averages # consecutive plot
//             queue values to produce a plot point.  This display filter does
//             not change the raw values stored in the plot queue so can be 
//             changed freely.
//
//             If the display filter is turned on,  the values shown
//             at the mouse cursor are the filtered values.
//
//             Be careful when using large display filter values
//             with long view time displays.  It can take a lot
//             of time to process the data on slower machines.  
//
//             If the filter count is a negative value (including -0 
//             or 0-) then the PPS and TEMPerature plots are inverted.
//             If the display filter value includes a "+" the PPS 
//             and TEMPerature plots are shown in their regular polarity.
//             If no "+" or "-" sign is included the plot polarities are 
//             not altered. Inverting the PPS and TEMPerature plots makes 
//             all the Thunderbolt plots track in the same direction.
//             You can also do this with the MI keyboard command.
//             Also,  you can append the '-' or '+' to the filter count 
//             value (i.e. -10 and 10- are treated the same).
//
//   There are also some filter settings only changeable via keyboard commands
//   in the "F" menu.  If the receiver does not support one of these filters
//   it will not appear in the "F" menu.  The "/f" filters listed above are also
//   accessible from the "F" keyboard menu.
//
//      FC - coordinate filter
//      FE - satellite elevation mask angle - satellites below this angle above
//           the horizon will not be tracked
//      FF - foliage filter
//      FI - set Motorola ionosphere filter
//      FJ - jamming mode filter
//      FL - satellite signal level mask - satellites with signals below this 
//           setting will not be used
//      FM - movement / marine filter
//      FT - set Motorola troposphere filter
//      FX - PDOP mask / switch setting
//
//   Many of the filter settings are shown in the system status column of
//   the screen display.  You can disable showing the filters:
//      GY  - toggle filter display on/off
//      /gy - toggle filter display from the command line
//
//
//     GX  - toggle the DOP (dilution of precision) value display in the 
//           receiver status information column.
//     /gx - toggle DOP display from the command line
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
//      .gpx  - a standard GPX 1.0 format file (a standardized XML file oriented
//              towards time and location data.
//      .kml  - a Google KML format location file (only stores time and 
//              lat/lon/alt)
//
//   Note that .gpx / .xml / .kml files are larger than the .log ASCII files.
//   The .xml (GPX 1.1) log format contains the most comprehensive data 
//   including pretty much the compete receiver configuration.
//
//
//   To start logging data from the keyboard:
//      WLW or LW - write a new log file from real-time data
//      WLA or LA - append real-time data to a log file
//      WLS or LS - stop logging real-time data
//
//   From the command line:
//      /w=file   - set log file name to Write to
//      /wa=file  - set log file name to Append to
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
//   Normally Heather does not write the satellite constellation data (sat PRN,
//   azimuth, elevation, signal level) to the log file.  You can enable logging
//   of the satellite constellation data:
//      WLC or LC - toggle logging of satellite constellation data
//      /ld       - from the command line
//                  Note that the sun is logged as satellite PRN 256 and 
//                  the moon as PRN 257.
//
//
//   Normally Heather writes unfiltered data to log files.  If the display
//   filter is enabled (FD keyboard command) you can toggle the writing of
//   filtered data to the log file:
//      WF  - toggle writing of filtered data to the log file.
//
//
//   Besides data logs, Heather also supports writing a debug log file:
//      WX  - write debug log file  -or-
//      /dl[=file]  - from the command line (if a file name is not given
//                    "debug.log" is used.
//
//
//   Heather also supports writing a raw receiver data capture file:
//      WY          - write debug log file from the keyboard
//      /dr[=file]  - from the command line (if a file name is not given
//                    "heather.raw" is used.
//
//
//   In ASCII format logs, the data separator can be changed from a TAB to
//   a comma with the command line option:
//      /ls  - toggle log file value separator between TABs and COMMAs
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
//             line on the first line of the file to help it recognize the file
//             as a potential log file.  XML/GPX logs don't have this limitation.
// 
//
//
//   You can disable logging of detected errors with:
//      /e   - toggle logging of detected errors in the log file
//
//
//   Lady Heather can read in log files and show the data in the plot area.
//   Reading in a log file first erases the plot queue the pauses the plot
//   queue updates from live incoming data.  You can then scroll around the
//   the logged data using the normal plot viewing commands.  You can resume
//   processing of receiver data using the "U" keyboard command.,, you might
//   want to first clear the plot queue data ("C" keyboard menu).
//  
//      R       - read a file (with .log .xml .gpx file extension)
//      /r=file - from the command line
//
//                Note that If you  read in a log file, Heather should first
//                be configured ("/rx#" command line option) for the receiver 
//                type that created the log file.
//
//
//
//   Heather can also write a raw receiver data log file... every byte from
//   the receiver is written to the raw data file:
//      WY  - write raw incoming receiver data to a file  -or-
//      /dr[=file]  - from the command line (if a file name is not given
//                    "heather.raw" is used.
//
//
//   The "/rs=filename" command line option can be used to read data from
//   a simulation file (a raw receiver data capture image). For simulation 
//   mode to work there MUST be a GPS receiver connected to the computer.
//   The receiver data is ignored, but the data stream is needed to provide 
//   pacing to Lady Heather.  The receiver can be a different type than the 
//   simulation file as long as it is providing some sort of data stream and 
//   the baud rate has been set to match the connected receiver.
//   Simulation mode may not work correctly with some receiver type (mainly
//   SCPI devices that require two-way polling to get data from the receiver)
//
//   For instance you have a Venus receiver connected at 115200 baud and want
//   to process a Ublox simulation file.  Try the command line:
//      heather /rxu /br=115200 /rs=ublox.raw
//
//   Log file can be automatically dump on a scheduled basis.  See the help
//   section on AUDIBLE CLOCKS for details.
//
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
//   receiver reports invalid time, it shows in RED.  If not UTC leapsecond
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
//      /tsz - from the command line
//      
//   
//
//
//   The watch display is located either in the upper right hand corner of
//   the screen or to the right of the plot area.  The watch outline is
//   normally shown in WHITE.  It is drawn in RED if the GPS reports that the
//   time is not valid,  and in YELLOW if no UTC leapsecond offset is available.
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
//      /tb=     - sets the watch "brand name" to the day of the week
//
//
//   You can specify the watch face style with the "O" keyboard command. 
//   Enter "O" from the keyboard and then, on the edit line, enter:
//     W0  - normal Roman numeral clock
//     W1  - decimal hours clock
//     W2  - "*" hour markers
//     W3  - 24-hour Roman numeral clock
//     W4  - 24-hour decimal hours clock
//     W5  - 24-hours "*" hour markers
//
//
//   The watch display includes a representation of the moon at its current
//   location in the sky and with its current phase.  The moon is shown in
//   YELLOW if it is visible and GREY if it is below the horizon.  The sun
//   is shown in a similar manner.  The watch display also flags when the
//   moon is new, quarter, or full.  Also "blue" and "black" moons are shown.
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
//   screen, the file name used will show up by the '\' entry)
//
//
//   The "/ta" toggle command line option shows dates in the European
//             the European dd.mm.yyyy format instead of the normal 12 Oct 2016
//             format. 
//
//
//
//
//
//--PENDING LEAPSECOND DISPLAY
//
//   Many GPS devices report when a leapsecond adjustment has been announced.
//   Heather shows this in the receiver status column. Leapsecond announcements
//   are generally made 6 months in advance.  
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
//      /ds - ISO date
//      /dt - Tzolkin calendar
//      /dv - Bolivian calendar
//      /dx - Xiuhpohualli (Aztec Haab) calendar
//      /dy - Mayan calendar
//      /dz - Aztec Tonalpohualli calendar
//      /dyyyymmdd - force date to year yyyy, month mm, day dd for testing
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
//   or the start of the next day.  If multiple calendar entries match the
//   current date, only the first match is shown.
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
//      text  - The text to display when the event occurs.
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
//    Note that the Windows versions of Heather plays sound files via the
//    PlaySound() system call,  the Linux versions use system() to spawn a 
//    /bin/sh command to the "aplay" program,  and macOs spawns a /bin/sh 
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
//  ALARM CLOCK MODE:
//    Heather has an alarm clock mode where you specify a time (and optional 
//    date) for the alarm to sound. The commands are the same as for the egg
//    timer, but you specify a specific time (and optional date) for the 
//    alarm instead of an interval.  Alarm clocks always a repeating alarm
//    amd must be manually canceled.  
//
//    TA 14:15:16   - sound the alarm continuously every day at 14:15:16
//    TA 14:15:16o  - sound the alarm once every day at 14:15:16
//    TA 8:0:12 2016/12/25  - sound the alarm at 8:00:12 on 25 December 2016
//    TA 8:0:12 12/25/2016  - sound the alarm at 8:00:12 on 25 December 2016
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
//  KEYBOARD SCRIPT FILE WAITS
//    If, in a keyboard script file, you follow and egg timer or alarm clock 
//    time with the letter "W",  the processing of the script file will
//    be paused until the alarm triggers.
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
//    The format for the scheduled log dump files defaults to ASCII.  You
//    can change this with command line options:
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
//    Screen dump mode is indicated on the screen by a '!'
//    or !! next to the time mode indicator on the first
//    line of the screen.
//
//
//  If a leap-second is observed (seconds value = 60) then Heather 
//  automatically does a screen dump to the file "leap_sec.gif"  On some 
//  receivers this might capture the previous second (xx:xx:59)... such is life.
//
//
//  Note that when a screen or log dump happens, Heather first creates the
//  the file "tblock", next the image/log file is written, and then the
//  "tblock" file is deleted.  This can be used by external scripts to 
//  minimize (but not totally eliminate) the chances of an external 
//  script/program accessing the dump file while it is being written.
//
//
//  Note that alarm and dump times are matched to the GPS receiver time down
//  to the second.  If the receiver skips the time stamp of the alarm/dump
//  event it will NOT be triggered.
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
//      TS   - from the keyboard - set the system clock once
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
//   Most operating systems require program that manipulate the system clock
//   to have administrator / root permissions. You can check if Heather can
//   can the system clock by issuing a "TS" command from the keyboard.  Within
//   a few seconds you should hear a BEEP if the time set command was accepted.
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
//   in the computer, operating system, receiver configuration, etc used can 
//   affect when the message actually arrives.
//
//   You can set the message offset time (in milliseconds) with the command
//   line option:
//
//      /tsx=milliseconds
//
//   Milliseconds can be positive (message arrives AFTER the true time value
//   encoded in the receiver time message) or negative (message arrives BEFORE 
//   the true time)
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
//   setting the time may give optimum performance.  The "TK" measurement 
//   mode also writes a ".jit" file with the histogram values and measurement
//   results.
//
//   With some receivers, Lady Heather's periodic polling
//   of the receiver for status info, etc can put spikes in
//   the timing measurements. If this is a problem use the
//   "/it" command line option to disable the sending of commands
//   to the receiver.  The spikes should not affect the
//   message offset time measurements but can affect the
//   message jitter standard deviation measurements. Issue
//   the "/it" command again to re-enable status polling.
//
//
//   Tell Heather what your system message offset is with the command line
//   option:
//      /tsx=msecs  - msecs is the value shown the the measurement results
//                    described above.  
//
//   Note that the "TK" timing measurement mode flogs the system CPU rather
//   hard.  You can see CPU usage stats approaching 100% in this mode. This can
//   produce heating that can affect the system clock rate after a while.
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
//      GLF  - show altitude in feet
//      GLM  - show altitude in meters
//
//      GLP  - private location - latitude and longitude are not displayed.
//              (so you can publish screen shots without revealing the location
//               of your evil mad scientist volcano lair to the world)
//
//      /gld , etc - set the location format from the command line.  The
//              command line option to use is the same as the keyboard commands
//              listed above with a leading "/" or "-".
//
//      /t"  - toggle between degrees.minutes.seconds / decimal location format
//             (this is a legacy command for compatibility with earlier
//              version of Lady Heather).
//
//
//   Heather also records the lat / lon / alt data in the plot queue and
//   can display the location data in the plot area or as an X-Y "scattergram"
//   of distance from a reference point.  The plot queue locations are saved 
//   as single precision floating point numbers are limited to a resolution 
//   of around 5-10 feet.
//
//     G1  - from the keyboard - toggle the latitude plot on and off
//     G2  - from the keyboard - toggle the longitude plot on and off
//     G3  - from the keyboard - toggle the altitude plot on and off
//     GV  - from the keyboard - toggle the lat/lon/alt (G1 / G2 /G3) plots 
//           on and off
//
//
//   THE LOCATION FIX SCATTERGRAM
//
//     Heather can plot a "scattergram" of the location fix data relative to
//     a fixed reference point.  The reference point is the point that was
//     active when the scattergram was activated.  The scattergram is
//     automatically enabled if a position survey is started.  Every hour the
//     color of the dots in the scattergram changes (14 colors are used).
//
//     The scattergram grid defaults to a resolution 3 meters or 10 feet 
//     per division (with +/- 5 divisions from the center reference point. 
//     You can change the scattergram grid scale factor with a command 
//     line option:
//        /tm=meters_per_division
//        /t'=feet_per_division
//
//     Normally the scattergram image is created from the double precision
//     location data as it comes in from the receiver.  If you change the 
//     resolution scale factor of the scattergram then the scattergram is image 
//     is re-created from the single precision plot queue data.  The resulting
//     scattergram image will be very "sparse" because the plot queue locations
//     have a resolution of 5-10 feet per point.
//
//     GI   - toggle the lat/lon "scattergram" on and off.
//     /gi  - toggle the lat/lon "scattergram" on and off.
//     ZL   - zoom the scattergram to full screen (ok, it doesn't actually
//            fill the screen... needs some work)
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
//     and statically processed the data using medians and averages over 
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
//     shown at the end of the file.
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
//            or the use can select a particular satellite... eventually
//            that satellite will fall below the horizon and the receiver 
//            won't be tracking anything...  Non-timing receivers are not able
//            to work in a single-satellite mode.
//
//     SX   - from the keyboard controls excluding/including of satellites.
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
//     SG   - from the keyboard controls the GNSS satellite system configuration.
//            You will be prompted to input a string of characters that 
//            indicate which systems to use.  The currently enabled systems
//            will be offered as the default entry.
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
//      Satellite PRN (that identifies the satellite)
//      Satellite azimuth angle
//      Satellite elevation angle
//      Satellite signal level
//
//   Depending upon the receiver type the information table can also show
//   extra things like:  doppler shift, tracking state, carrier phase, code
//   phase, clock bias, URA (range accuracy).
//
//   A normal GPS receiver may be able to track around 14 satellites at once.
//   More modern receiver that can also track GLONASS, GALILEO, BEIDOU, SBAS
//   and other satellites might be tracking over 32 satellites at once.
//   Normally Heather expects to show up to around 14 satellites, and
//   displaying 32 satellites at once can cause problems fitting all that data
//   on the screen.  You can control the size and format of the satellite
//   information table display:
//
//       SI    - from the keyboard
//       GCT   - from the keyboard
//       /si=# - from the command line.
//
//   Clicking the left mouse button on the satellite information display
//   will zoom the sat info it to full screen.
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
//   information it has for up to the specified number of satellites.
//
//   If you input a negative number,  Heather shows the short form of the
//   info for up to that number of satellites.  The short form display frees
//   up enough space to the right of the satellite info display to show
//   the sun / moon information display.  If the receiver type in use does
//   not report any extended information,  the sun/moon info display is 
//   automatically enabled.
//
//   If you input a number with a "+" sign, Heather shows the short form
//   information in two columns (but does not leave room for the sun / moon
//   information).  This helps with displaying information for receivers that
//   can track multiple satellite systems.
//
//   If you follow the number with a "t", only satellites that are being
//   actively TRACKED are shown in the satellite information table and
//   satellite map display.
//
//
//   The more rows of satellite information being displayed, the less space
//   there is for the plot area.  Combine lots of rows of satellite info
//   with the digital clock display on a small screen and you can wind up with
//   an unusable plot display or unreadable keyboard menus.  Turning off the
//   digital clock display (GZ) can help with the screen formatting.
//
//   If more satellites are visible that you have allocated space for, the
//   info for the excess satellites is not shown.
//
//   Satellites that are being actively tracked are shown in GREEN
//   Satellites that are visible, but not being tracked are YELLOW
//   Excluded/disabled/blocked satellites are shown in BLUE
//   Unhealthy satellites are shown in RED (if the receiver reports satellite
//   health)
//
//
//   Heather graphs the number of actively tracked satellites in the plot
//   window:
//
//      GCG  - toggles the tracked satellite count plot ON and OFF.
//             The satellite count plot is shown in CYAN at the bottom of
//             the plot window.  Each minor division equals one satellite.
//             For screen resolutions less than 800x600, each minor division
//             represents two satellites.
//   
//   
//   You can sort the satellite information table using the following
//   keyboard commands.  You can sort in descending order by including
//   "-" character.  Ascending sort is the default.
//   If you sort by one of the values, the header of that column is shown
//   in GREEN.  If you don't specify a sort, the table is sorted by PRN
//   number in ascending order.
//
//      GCA - sort by azimuth
//      GCD - sort by doppler
//      GCE - sort by elevation
//      GCP - sort by PRN (default)
//      GCS - sort by signal level
//
//
//
//   Besides the satellite information table, Heather has two other satellite
//   information displays... the satellite map and the signal level map:
//
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
//   Clicking on the map/watch display on the screen will zoom it to full 
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
//   visible and as a hollow yellow circle if it is below the horizon. The sun
//   symbol is shown surrounded by sun rays in order to distinguish it from
//   satellites.
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
//   time code errors caused by these messages.  Heather also adjusts any 
//   alarm times so that they will not occur during the interval where the
//   receiver may not be sending time codes.
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
//       SAD  - toggle raw signal level data map on and off
//
//   From the command line
//       /gq  - toggle signal level map on and off
//       /gqs - toggle signal level map on and off
//       /gqq - toggle signal level map on and off
//       /gqw - toggle signal elevation weighted level map on and off
//       /gqa - toggle signal level vs azimuth map on and off
//       /gqe - toggle signal vs elevation map on and off
//       /gqd - toggle raw signal level data map on and off
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
//    SAD - shows raw satellite signal level data points
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
//      ZX - zoom the watch, satellite, and signal quality maps to full screen
//           overlayed on each other
//      ZY - zoom the satellite map and signal quality map to full screen
//           overlayed on each other
//
//      ZS - zoom the Signal quality map to full screen
//      ZQ - zoom the signal Quality map to full screen
//      ZA - zoom the elevation weighted Azimuth map to full screen
//      ZR - zoom the Relative signal level map to full screen
//      ZE - zoom the signal level vs Elevation map to full screen
//      ZD - zoom the raw signal level Data map to full screen
//           If once of the signal quality maps is shown you can click the 
//           mouse to show all the signal quality maps (ZU command)
//
//      ZU - zoom the all signal level maps to full screen
//      ZV - zoom the watch, sat map, signals full screen
//           If the ZU or ZV screen is showing you can click the mouse
//           on one of the displayed items to zoom it to full screen.
//
//
//   Note that it can take several hours for the map to fill in.  The more
//   satellites that your receiver can track, the faster the map fills in.
//
//   Note that the satellite position and signal level maps are affected
//   by the settings of the elevation mask and signal level mask filters.
//   If you want a complete view  of your antenna system performance then first
//   set those filters to their lowest settings.
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
//      TR M  - display moon rise/transit/set times
//
//      /sr=#  - from the command line where # is the sunrise type selection
//               character shown above.  For example
//                  /sr=c says to show civil sunrise/sunset times.
//
//   Heather can also play sound files at rise/set or transit (solar noon):
//      Include an "*" after the command:
//         TR O*
//         /sr=o*
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
//   used for sun times... there are not separate moon files.
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
//
//   Heather also uses the plot title feature for displaying things like
//   calendar events, errors, and certain status information.  Setting a user
//   defined title usually inhibits these automatic titles.
//
//   Heather usually only updates the parts of the screen that have changed. 
//   You can force Heather to fully redraw the screen from the keyboard:
//      GR  - redraw the screen
//
//
//   You can zoom the plot window display to full screen:
//      ZP   - zoom the plot window to full screen
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
//   data for display.
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
//   and scrolling of the plot data is inhibited.
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
//      ]     - move the plot forward one pixel
//      [     - move the plot back one pixel
//      DEL   - exit plot review mode and return to normal scrolling
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
//   center scrolls the plot in that direction.  The further you move the mouse
//   from the center of the plot, the faster the plot will scroll.
//
//
//   You can disable the mouse from the command line:
//      /km  - toggle the mouse functionality
//
//
//   You can disable the keyboard from the command line.  The only way to
//   exit Heather when the keyboard is disabled is to click the CLOSE button
//   on the operating system title bar (or using the "kill process" command
//   of your operating system)
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
//      The GU keyboard command will clear all markers.
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
//      RED tick mark at the top of the plot.  If you place the mouse cursor
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
//        GP - usually a "PPS" (Pulse Per Second) output related value
//        GO - usually an oscillator frequency output related value
//        GO - usually an EFC DAC or PPS sawtooth correction related value
//        GT - usually a temperature related value
//
//        G1 - receiver latitude
//        G2 - receiver longitude
//        G3 - receiver altitude
//        G4 - receiver speed  or TFOM (time figure of merit)
//        G5 - receiver course or FFOM (frequency figure of merit)
//
//        G6 - average DOP (dilution of precision) value
//        G7 - (currently unused)
//        G8 - (currently unused) 
//
//        G9 - message timing offset
//        G0 - message timing jitter
//
//        GA - ADEV plots (see the section of ADEVs)
//
//
//   Selecting one of the plots listed above brings up a sub-menu of options
//   for controlling that plot.  The descriptions listed below use the "GP"
//   plot, but can be used with any of the plots.  Replace the lower-case "p"
//   shown below with the letter of the plot you want to manipulate.
//
//        Gp<cr> - toggles the plot display ON or OFF
//        GpP    - toggles the plot display ON or OFF
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
//        GpI    - toggles inversion of the plots' display ON or OFF.  Inverting
//                 a plot can make it easier to compare how one parameter (like
//                 temperature) affects another one (like DAC voltage)
//        /mi    - invert PPS and TEMPerature plots
//
//
//        Gp/ - select plot statistic selection menu. Heather can calculate
//              various statistics of the plot data shown in the plot window:
//           GP/A   - average value
//           GP/R   - RMS value
//           GP/S   - standard deviation
//           GP/V   - variation
//           GP/N   - minimum value
//           GP/X   - maximum value
//           GP/P   - span - difference between maximum and minimum values
//           GP<cr> - turns off the plots' statistic display
//
//        G/p  - you can set the statistic to show for ALL enabled plots
//               with this command
//
//        GpA  - toggles auto-scaling mode for ALL plots
//        /ma  - from the command line
//
//        GpC  - select the color to display the plot in
//
//        GpL  - toggles a linear regression trend line for the plot.  The 
//               trend line is drawn and it's equation (as a function of time)
//               is shown as the plot title.
//
//        Gp=  - removes drift from a plot by subtracting the linear regression  
//               trend line slope from the plot
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
//        /zP      - toggles plot P auto-centering from the command line
//        /zP=val  - sets plot P zero reference line value the command line
//                   (0 = auto center)
//
//        GCG  - toggles the tracked satellite count plot ON and OFF.
//               The satellite count plot is shown in CYAN at the bottom of
//               the plot window (each minor division equals one satellite)
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
//                 (1024 points if not given,  /qf is 4096 points,
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
//      You can apply an averaging filter to the displayed plots (see the
//      section on FILTERS for more details.
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
//   The steering of the oscillator frequency is usually done by a phase locked 
//   loop (PLL) circuit in the GPSDO that compares the current frequency to 
//   the GPS timing signals.  A GPSDO controls the oscillator frequency by 
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
//      HE - enable manual holdover
//      HX - exit manual holdover
//      HH - toggle holdover mode on/off
//
//
//   Besides HOLDOVER mode, some GPSDOs let the user manually control the 
//   oscillator EFC signal.  
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
//   Heather shows the amount of time of the current ot last holdover event.
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
//       &a  - autotune Thunderbolt oscillator parameters
//
//
//     Some GPSDOs (like the Nortel/Trimble NTBW and NTPx devices) let the user
//     control the GPSDO settings, but do not save them in EEPROM.  You can cause
//     Heather to configure the GPSDO at startup with these command line options:
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
//       it seem to work from the keyboard, though.
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
//     Before running auto-tune you should: 
//        1) Wait for the unit to stabilize with (relatively) 
//        steady DAC, OSC, and PPS values. 
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
//     If you run the auto-tune command on non-Thunderbolt devices then
//     only the satellite elevation mask (and perhaps the signal level mask)
//     values will be set.
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
//     Heather uses the RTS and DTR serial port modem control lines to control
//     the fan or peltier (there is some legacy code in there that could
//     be activated to work with a parallel port).
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
//        S= Scale factor (loop gain correction)
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
//   GPS timing receivers and discipline oscillators usually have one
//   or more timing output signals.  Almost all have a 1 pulse-per-second
//   (PPS) output signal.  On many devices this can be programmed for
//   other frequencies.  Other devices also have other auxiliary output
//   signals that can be controlled.
//
//   The "P" keyboard menu provides commands for controlling these output
//   signals.
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
//
//
//   PPS SIGNAL CONTROL and OUTPUT FREQUENCY CONTROL
//
//      You can turn the PPS signal ON, OFF, or toggle its state:
//        PD   - disable the PPS signal
//        PE   - enable the PPS signal
//        PS   - toggle the PPS signal ON or OFF
//        /pd  - disable PPS output signal from the command line
//        /pe  - enable PPS output signal from the command line
//        /p   - toggle PPS output signal from the command line
//
//
//      You can control the PPS signal polarity:
//        PR   - select rising edge PPS signal
//        PF   - select falling edge PPS signal
//        PP   - toggle PPS polarity
//        /-   - select rising edge PPS signal from the command line
//        /+   - select falling edge PPS signal from the command line
//
//
//      Some devices let you set the oscillator signal polarity referenced 
//      to the PPS signal:
//        PO   - toggle the oscillator signal polarity from the keyboard
//        /^f  - sync OSC signal falling edge to time
//        /^r  - sync OSC signal rising edge to time
//        /^   - toggle OSC signal edge
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
//      Some devices let you adjust the PPS timing offset in relation to 
//      UTC/GPS time (much like the antenna cable delay parameter):
//        PO   - set the PPS pulse timing offset in relation to UTC/GPS time
//               (the times are in nanoseconds and can usually be positive or
//                negative)
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
//
//
//      Some devices have one or pulse/frequency output signals that let
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
//
//
//   TRAIM CONTROL
//
//      Many GPS timing receivers support a feature called TRAIM - Time
//      Receiver Autonomous Integrity Monitoring.  TRAIM mode monitors the
//      timings from the various satellites and if inconsistencies are
//      detected can drop suspect satellites from the timing solution, set
//      alarm conditions, etc.  Consult your device manual.
//
//        PT   - configure device TRAIM mode / threshold.
//
//
//
//
//
//-NON-VOLATILE CONFIGURATION EEPROM/FLASH/SRAM memory
//
//   Some receivers support saving all or parts of the receiver configuration
//   into some form of nonvolatile memory (like EEPROM).  Configuration
//   saves are currently supported on Trimble, Ublox, and Venus devices.
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
//                show "FLASH: WRT" instead of "FLASH: OK".  Writing to flash
//                defaults to OFF for Venus receivers.
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
//-ADEV CALCULATIONS and DISPLAYS
//
//   Lady Heather can calculate and display various ADEV (Allan Variance/
//   Deviation) values from the receiver readings.  ADEVs are a way of 
//   characterizing how stable a signal is over various time intervals.
//   Heather supports ADEV, MDEV, HDEV, and TDEV calculation.  
//
//   --------------------------  WARNING --------------------------
//
//   Heather calculates ADEVs based upon the PPS and OSCillator
//   error values reported by the unit.  These values are not
//   derived from measurements made against an independent reference
//   (other than the GPS signal) and may not agree with ADEVs calculated
//   by measurements against an external reference signal. ADEVs calculated
//   when the oscillator is undisciplined are more meaningful than when
//   the oscillator is being disciplined.  For proper ADEV values, you could 
//   modify the code to support reading time intervals from a proper external
//   Time Interval Counter.
//
//   Not all devices report values that are even suitable for ADEV 
//   calculations... Mostly the Trimble GPSDO devices are usable.
// 
//   ---------------------------------------------------------------
//
//
//   SELECTING THE ADEV TYPE TO DISPLAY
//
//   Heather maintains a separate circular data buffer of the values that
//   it uses to calculates ADEVs from.  The queue contains a PPS related
//   value and an OSCillator related value.  The size of the ADEV queue
//   determines the maximum "tau" time interval that the ADEVs can show.
//   The default is 33,000 points which is suitable for values of TAU out
//   to around 10,000 seconds.  
//
//   You can set the size of the ADEV queue from the command line:
//      /a=size - sets the number of points to save in the ADEV queue.
//                A size of 0 will disable ADEV calculations.
//
//
//   Normally Heather collects a new ADEV entry every second.  You can adjust
//   the ADEV interval with:
//      /j=secs - sets the ADEV sample interval 
//
//
//   You can clear the ADEV queue from the keyboard:
//      CA - clear the ADEV queue
//      CB - clear the ADEV and plot queue
//
//   You can clear the ADEV queue and reload it from the porttion of the
//   plot queue currently being shown on the screen:
//      CR - reload the ADEV queue from the plot queue data being shown
//
//
//
//   Heather can also read ADEV information from files using the "R"
//   keyboard command.  ADEV files must have an extension of .ADV  
//   These files can have two independent values per line.  The first value
//   if the "PPS" value and the second one is the "OSC" value. These values
//   don't have to be actual PPS and OSC values, but that is how Heather
//   refers to them in the menus.
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
//
//   ADEV INFORMATION DISPLAY
//
//   Heather displays ADEV info in two ways.  First is a table of ADEV values.
//   Second is as graphs in the plot area.
//
//      GA  - from the keyboard toggles the ADEV plots on and off
//      /ga - from the command line
//
//   If the ADEV plot is enabled, then the division markers in the plot area
//   that represent decades are highlighted in CYAN.  Each highlighted
//   vertical division represents a power of 10.  Each minor division marker
//   is a linear division of that decade.  The decade value of the top line of 
//   the plot area is determined from the largest ADEV value seen in any
//   of the ADEV tables.  All ADEV types are scaled the same.
//
//   Each highlighted horizontal division represents TAU with divisions a power 
//   of 10 seconds (TAU 1,10,100,1000,10000,,..)  The horizontal major
//   divisions between the highlighted ones are a 1:2:5 division of 
//   that time decade (like 1,2,5,10,20,50,100...).
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
//   "all adevs" mode.  You should click on the upper adev table.  The lower
//   adev table might be ignored in some screen configurations.
//   Cicking again will restore the screen to its previous state.
//   Note that short clicks might be ignored... particularly on devices
//   like UCCM receivers that only output time updates every two seconds.
//   It can help to hold down the mouse button until the screen changes.
//
//
//
//   Heather always calculates all four ADEV types for both the PPS and OSC
//   values.  The "A" keyboard menu is used to configure the ADEV information
//   to display:
//      AA - Show ADEVs for the OSC and PPS values
//      AH - Show HDEVs for the OSC and PPS values 
//      AM - Show MDEVs for the OSC and PPS values 
//      AT - Show TDEVs for the OSC and PPS values 
//      AO - Show all ADEV types for the OSC value
//      AP - Show all ADEV types for the PPS value
//
//   You can also select the ADEV type(S) from the command line:
//      /oa - Show ADEVs for the OSC and PPS values
//      /oh - Show HDEVs for the OSC and PPS values 
//      /om - Show MDEVs for the OSC and PPS values 
//      /ot - Show TDEVs for the OSC and PPS values 
//      /oo - Show all ADEV types for the OSC value
//      /op - Show all ADEV types for the PPS value
//
//      The "all adev" commands (AP and AO) lets you
//      control what info to display.  All four ADEV plots
//      only, all four ADEV types and the graphs (can be a
//      bit confusing since the same colors are used for
//      two different things),  or the two regular
//      ADEV plots and graphs:
//         AOA - show all four OSC ADEVs tables and all ADEV plots
//         APA - show all four PPS ADEVs tables and all ADEV plots 
//         AOG - show all four OSC ADEVs tables and all plots
//         APG - show all four PPS ADEVs tables and all plots
//         AOR - show all four OSC ADEVs tables and regular ADEV plots
//         APR - show all four PPS ADEVs tables and regular ADEV plots 
//
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
//            serial port configuration.  In order to guard against
//            accidentally "bricking" a receiver by setting a 
//            hard-to-recover-from configuration,  
//
//            Currently Heather does not ever change the receiver serial port 
//            configuration. (someday this command might be able to configure 
//            the receiver serial port parameters)
//
//      !B  - send a 300-500 msec BREAK to the serial port
//
//
//   SENDING COMMANDS to RECEIVER
//      !U  - sends a command string to the receiver.  This is mainly for
//            NMEA and SCPI receivers that accept ASCII commands.  For NMEA
//            receivers you do not need to include the leading '$' character.
//            Also the NMEA checksum will be automatically added if it is not
//            included.
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
//            On receiver that are speaking Motorola binary data, this command 
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
//      The terminal emulator mode has several keyboard commands for
//      controlling it:
//        
//         END    - exit terminal emulator
//         HOME   - erase the screen
//         UP     - re-sends the last keyboard line to the receiver
//         F1     - toggle keystroke echo mode
//         F2     - toggle writing receiver data to a log file
//                  Heather uses the "raw data log" file name. If the raw
//                  data log file is open, it is closed.  If it not open, it
//                  is opened in "append, binary" mode.
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
//
//
//
//-MISCELLANEOUS COMMANDS
//
//   There are several commands that don't fall under any of the previous
//   sections.
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
//   to the serial port:
//      /is     - toggle read-only mode for serial port
//
//
//   Normally Heather periodically polls the receiver for various pieces of
//   data,  status,  or configuration information.  Usually a new piece of
//   information is polled for every second.  It can take around 30 seconds
//   for Heather to acquire everything from the receiver.  You can block
//   Heather from polling the receiver for data.  This is mainly useful when
//   using the "TK" command to analyze receiver message timing.
//      /it     - toggle no-polling mode for receiver data
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
//      or end in a '.',  the Heather tries to read a .LOG, .XML, .GPX, .SCR,
//      .LLA, .CAL,  .SIG,  .ADV, and .TIM files in that order.
//
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
//   /rt[=#] - use Resolution-T serial port config (9600,8,ODD,1)
//             [#=1]=force Resolution-T  [#=2]force Resolution SMT
//             (for REsolution-T devices you should now use the "/rxr" command
//              line option).
//
//
//
//   You can change the input device on the fly
//   from the keyboard with the /# or /ip keyboard command
//   just like from the command line.
//   Changing the input device from the keyboard resets
//   the satellite tracking and signal level maps.
//   You can also use these commands to re-establish
//   the a dropped connection or device.  If you had
//   previously set an IP connection address/port, you
//   don't have to type the "=addr:portnum" after the /ip
// 
//   You can also use the "/rx" commands from the keyboard to change
//   the input device type.
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
//   not be preceeded by a '/'.
//
//
//
//
//
//
//
//  Other (mostly rather obscure) keyboard selctable options:
//  Those marked with "---" are either no longer implemented or have proper
//  keyboard or command line options.
//
//    OA   - toggle AMU vs dBc signal level displays.
//    OB # - set ADEV bins per decade
//    OC   - toggle satellite constellation plot scaling mode
//    OD   - toggle FFT plot display in dB or raw values
//    OE   - toggle Thunderbolt-E display mode
//    OF   - toggle periodic refresh of adev calculations
//--- OG   - toggle plot area background highlight color
//    OH   - toggle erasing of Lat/Lon/Alt survery plot every hour
//--- OI # - set signal level display type
//    OJ   - toggle logging of serial port data stream
//    OK   - toggle logging of TSIP message faults as time skips
//    OL   - toggle live FFT mode
//    OM # - set plot magnification factor
//    ON   - toggle real-time update of trend line plot title
//    OP   - toggle plot scaling mode to peak value seen
//    OQ   - toggle plot queue sampling fast / slow mode
//    OR   - reset ADEV bins and recalculate ADEVs
//    OS   - toggle temperature spike filter mode
//    OT   - toggle alarm/dump/exit time triggers to be based upon local time
//           (default) or displayed time which can be in one of the 
//           astronomical time scales.  Previous versions used OT to toggle
//           12/24 hour clock mode which is now available from the T menu.
//--- OU # - set daylight savings time area number (0 .. 5)
//    OV   - toggle ADEV base value mode
//    OW # - select analog watch face type (0 .. 5)
//--- OX # - set receiver configuration value
//--- OY # - set tempurature-dac plot (G3 plot) scale factor
//--- OZ   - toggle auto-centering of plots
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
//  sendout(c)  - send byte 'c' to the serial port
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
//    WIN_VFX     (Windows video/keyboard/mouse stuff)
//    USE_X11     (X11 video/keyboard/mouse stuff for Linux and OS/X)
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

#ifdef __MACH__    // OSX time functions
   #include <mach/clock.h>
   #include <mach/mach.h>
#endif


#ifdef WINDOWS
   C8 szAppName[256+1] = "Lady Heather's Disciplined Oscillator Control Program - "VERSION"";

   u08 timer_set;  // flag set if dialog timer has been set
   u08 path_help;  // flag set if help message has been seen before
#endif


unsigned char *dot_font;  // pointer to character font in use
int x11_mouse;            // flags x11 mouse motion detected
int no_mouse_review;      // used to inhibit recursive calls to get_mouse_info
int com_tick;             // used to fake SERIAL_CHAR_AVAILABLE() if no receiver detected
int did_init1;            // flag set if init_message(1) setup the device


#ifdef WIN_VFX            // using WIN_VFX library for screen/mouse/keyboard

   VFX_WINDOW *stage_window = NULL;
   PANE       *stage = NULL;

   VFX_WINDOW *screen_window = NULL;
   PANE       *screen = NULL;

   VFX_FONT *vfx_font = (VFX_FONT *) (void *) &h_font_12[0];

   S32 font_height;

   U16 transparent_font_CLUT[256];
#endif

static char degc[] = { DEGREES, 'C', 0 };
static char degk[] = { DEGREES, 'K', 0 };
static char degs[] = { DEGREES, 0 };

struct PLOT_DATA plot[NUM_PLOTS+DERIVED_PLOTS] = {  // plot configurations 
//  ID        units   ref scale  show  float  plot color
   {"OSC",    "ppt",  1000.0F,   0,    0,     OSC_COLOR    },
   {"PPS",    "ns",   1.0F,      1,    0,     PPS_COLOR    },
   {"DAC",    "uV",   1.0E6F,    1,    1,     DAC_COLOR    },
   {"TEMP",   degc,   1000.0F,   1,    1,     TEMP_COLOR   },
   {"D\032H", "mHz",  1.0F,      0,    1,     ONE_COLOR    },   // G1
   {"DIF",    "x",    1.0F,      0,    1,     TWO_COLOR    },   // G2
   {"D-T",    "x",    1.0F,      0,    1,     THREE_COLOR  },   // G3

   {"[4]",    "x",    1.0F,      0,    1,     FOUR_COLOR   },
   {"[5]",    "x",    1.0F,      0,    1,     FIVE_COLOR   },
   {"[6]",    "x",    1.0F,      0,    1,     SIX_COLOR    },
   {"[7]",    "x",    1.0F,      0,    1,     SEVEN_COLOR  },
   {"[8]",    "x",    1.0F,      0,    1,     EIGHT_COLOR  },
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


void config_rcvr_plots()
{
   // Configure the plots for the receiver type in use
   // note: we should not change settings the user gave on the command line

plot[FOUR].plot_id = "Speed";
plot[FOUR].units = "m/s";
plot[FOUR].ref_scale = 1.0F;
plot[FOUR].show_plot = 0;
plot[FOUR].float_center = 1;

plot[FIVE].plot_id = "Course";
plot[FIVE].units = degs;
plot[FIVE].ref_scale = 1.0F;
plot[FIVE].show_plot = 0;
plot[FIVE].float_center = 1;

   if(res_t) {
      plot[DAC].plot_id = "Sawt";
      plot[DAC].units = "ns";
      plot[DAC].ref_scale = 1.0F;
      if(user_set_dac_plot == 0) plot[DAC].show_plot = 0;
      if(user_set_dac_float == 0) plot[DAC].float_center = 1;

      plot[PPS].plot_id = "Bias";
      plot[PPS].units = "us";
      plot[PPS].ref_scale = 1.0F/1000.0F;
      if(user_set_pps_plot == 0) plot[PPS].show_plot = 1;
      if(user_set_pps_float == 0) plot[PPS].float_center = 1;

      plot[OSC].plot_id = "Rate";
      plot[OSC].units = "ppb";
      plot[OSC].ref_scale = 1.0F;
      if(user_set_osc_plot == 0) plot[OSC].show_plot = 1;
      if(user_set_osc_float == 0) plot[OSC].float_center = 1;

      if(user_set_adev_plot == 0) plot_adev_data = 0;
//    have_sawtooth = 1;
   }
   else if(rcvr_type == MOTO_RCVR) {
      plot[DAC].plot_id = "Sawt";
      plot[DAC].units = "ns";
      plot[DAC].ref_scale = 1.0F;
      if(user_set_dac_plot == 0) plot[DAC].show_plot = 0;
      if(user_set_dac_float == 0) plot[DAC].float_center = 1;

      plot[PPS].plot_id = "Accu";
      plot[PPS].units = "us";
      plot[PPS].ref_scale = 1.0F/1000.0F;
      if(user_set_pps_plot == 0) plot[PPS].show_plot = 1;
      if(user_set_pps_float == 0) plot[PPS].float_center = 1;

      plot[OSC].plot_id = "Cofs";
      plot[OSC].units = "ns";
      plot[OSC].ref_scale = 1.0F;
      if(user_set_osc_plot == 0) plot[OSC].show_plot = 1;
      if(user_set_osc_float == 0) plot[OSC].float_center = 1;
      ppt_string = " ns";
      ppb_string = " ns";
   }
   else if(rcvr_type == NVS_RCVR) {
      plot[DAC].plot_id = "Sawt";
      plot[DAC].units = "ns";
      plot[DAC].ref_scale = 1.0F;
      if(user_set_dac_plot == 0) plot[DAC].show_plot = 0;
      if(user_set_dac_float == 0) plot[DAC].float_center = 1;

      plot[PPS].plot_id = "Rgen";
      plot[PPS].units = "ns/s";
      plot[PPS].ref_scale = 1.0F;
      if(user_set_pps_plot == 0) plot[PPS].show_plot = 1;
      if(user_set_pps_float == 0) plot[PPS].float_center = 1;
   }
   else if(rcvr_type == SIRF_RCVR) {
      plot[PPS].plot_id = "Drft";
      plot[PPS].units = "us";
      plot[PPS].ref_scale = 1.0F/1000.0F;
      if(user_set_pps_plot == 0) plot[PPS].show_plot = 1;
      if(user_set_pps_float == 0) plot[PPS].float_center = 1;

      plot[OSC].plot_id = "Cofs";
      plot[OSC].units = "ns";
      plot[OSC].ref_scale = 1.0F;
      if(user_set_osc_plot == 0) plot[OSC].show_plot = 0;
      if(user_set_osc_float == 0) plot[OSC].float_center = 1;

      if(user_set_dac_plot == 0) plot[DAC].show_plot = 0;
      ppt_string = " ns";
      ppb_string = " ns";
   }
   else if(rcvr_type == UBX_RCVR) {
      plot[DAC].plot_id = "Sawt";
      plot[DAC].units = "ns";
      plot[DAC].ref_scale = 1.0F;
      if(user_set_dac_plot == 0) plot[DAC].show_plot = 0;
      if(user_set_dac_float == 0) plot[DAC].float_center = 1;

      plot[PPS].plot_id = "Accu";
      plot[PPS].units = "ns";  
      plot[PPS].ref_scale = 1.0; // !!!!! 1.0F/1000.0F;
      if(user_set_pps_plot == 0) plot[PPS].show_plot = 1;
      if(user_set_pps_float == 0) plot[PPS].float_center = 1;

      plot[OSC].plot_id = "Frac";
      plot[OSC].units = "ns";
      plot[OSC].ref_scale = 1.0F;
      if(user_set_osc_plot == 0) plot[OSC].show_plot = 0;
      if(user_set_osc_float == 0) plot[OSC].float_center = 1;
      ppt_string = " ns";
      ppb_string = " ns";
   }
   else if(rcvr_type == VENUS_RCVR) {
      plot[DAC].plot_id = "Sawt";
      plot[DAC].units = "ns";
      plot[DAC].ref_scale = 1.0F;
      if(user_set_dac_plot == 0) plot[DAC].show_plot = 0;
      if(user_set_dac_float == 0) plot[DAC].float_center = 1;
   }
   else if(rcvr_type == ZODIAC_RCVR) {
      plot[DAC].show_plot = 0;

      plot[PPS].plot_id = "PPS";
      plot[PPS].units = "ns";  // "us"!!!!!
      plot[PPS].ref_scale = 1.0; // !!!!! 1.0F/1000.0F;
      if(user_set_pps_plot == 0) plot[PPS].show_plot = 1;
      if(user_set_pps_float == 0) plot[PPS].float_center = 1;

      plot[OSC].plot_id = "Cofs";
      plot[OSC].units = "ns";
      plot[OSC].ref_scale = 1.0F;
      if(user_set_osc_plot == 0) plot[OSC].show_plot = 1;
      if(user_set_osc_float == 0) plot[OSC].float_center = 1;
      ppt_string = " ns";
      ppb_string = " ns";
   }
   else {  // Resolution-T, etc receiver
      plot[PPS].plot_id = "Bias";
      plot[PPS].units = "us";
      plot[PPS].ref_scale = 1.0F/1000.0F;
      if(user_set_pps_plot == 0) plot[PPS].show_plot = 1;
      if(user_set_pps_float == 0) plot[PPS].float_center = 1;

      plot[OSC].plot_id = "Rate";
      plot[OSC].units = "ppb";
      plot[OSC].ref_scale = 1.0F;
      if(user_set_osc_plot == 0) plot[OSC].show_plot = 1;
      if(user_set_osc_float == 0) plot[OSC].float_center = 1;
   }

   plot[MSGJIT].plot_id = "MsgJit";   // timing message jitter
   plot[MSGJIT].units = "msec";
   plot[MSGJIT].ref_scale = 1.0F;
   plot[MSGJIT].float_center = 1;
   plot[MSGJIT].show_stat = SDEV;

   plot[MSGOFS].plot_id = "MsgOfs";   // timing message offset from clock
   plot[MSGOFS].units = "msec";
   plot[MSGOFS].ref_scale = 1.0F;
   plot[MSGOFS].float_center = 1;
   plot[MSGOFS].show_stat = AVG;      // SHOW_SPAN;
   

   if(user_set_adev_plot == 0) plot_adev_data = 0;

// plot_signals = 4;
   if(0 && (SCREEN_HEIGHT >= MEDIUM_HEIGHT)) {
      plot_azel = AZEL_OK;
      update_azel = 1;
      if(plot_azel == 0) plot_signals = 4;
      if(user_set_clock_plot == 0) plot_digital_clock = 0;
   }
   else if(rcvr_type == NO_RCVR) {
      if(user_set_watch_plot == 0) plot_watch = 1;
      if(user_set_clock_plot == 0) plot_digital_clock = 1;
   }
   else {
      if(user_set_watch_plot == 0) plot_watch = 1;
      if(user_set_clock_plot == 0) plot_digital_clock = 0;
   }

   if(user_set_temp_filter == 0) undo_fw_temp_filter = 0;
   config_set = 1;
}

void config_normal_plots()
{
    // restore G1 G2 and G3 plots to their normal state
return;  // we now use these for other stuff

    // These plots were for Warren S.'s temperature studies
    plot[ONE].plot_center = (float) 0.0;
    plot[ONE].float_center = 1;
    plot[ONE].plot_id = "D\032H";
    plot[ONE].units = "mHz";

    plot[TWO].plot_center = (float) 0.0;
    plot[TWO].float_center = 1;
    plot[TWO].plot_id = "DIF";
    plot[TWO].units = "x";

    plot[THREE].plot_center = (float) 0.0;
    plot[THREE].float_center = 1;
    plot[THREE].plot_id = "D-T";
    plot[THREE].units = "x";

    plot[SIX].plot_center = (float) (0.0);
    plot[SIX].float_center = 1;
    plot[SIX].plot_id = "[4]";
    plot[SIX].units = "x";

    graph_lla = 0;
}

void config_lla_plots(int keep, int show_lla)
{
    // config G1 G2 and G3 plots to show lat/lon/alt, G6 for DOP
    plot[ONE].plot_center = (float) (lat * RAD_TO_DEG);
    plot[ONE].float_center = 1;
    plot[ONE].plot_id = "Lat";
    plot[ONE].units = deg_string;

    plot[TWO].plot_center = (float) (lon * RAD_TO_DEG);
    plot[TWO].float_center = 1;
    plot[TWO].plot_id = "Lon";
    plot[TWO].units = deg_string;

    plot[THREE].plot_center = (float) alt;
    plot[THREE].float_center = 1;
    plot[THREE].plot_id = "Alt";
    plot[THREE].units = alt_scale;

    if(show_lla == 0) {  // keep old show values
    }
    else if(show_lla == 1) {   // disable
       plot[ONE].show_plot = 0;
       plot[TWO].show_plot = 0;
       plot[THREE].show_plot = 0;
    }
    else if(show_lla == 2) {   // enable
       plot[ONE].show_plot = 1;
       plot[TWO].show_plot = 1;
       plot[THREE].show_plot = 1;
    }

    plot[SIX].plot_center = 0.0F;
    plot[SIX].float_center = 1;
    plot[SIX].plot_id = "DOP";
    plot[SIX].units = " ";

    graph_lla = 1;
    if(keep) keep_lla_plots = keep;
}


void set_cct_id()
{
   // luxor color temperature plot
   if(cct_type == 1)      plot[CCT].plot_id = "CCT1";
   else if(cct_type == 2) plot[CCT].plot_id = "CCT2";
   else                   plot[CCT].plot_id = "CCT";
}

void config_luxor_plots()
{
int i;

   // setup plots for luxor LED/POWER analyzer

   VER_COL = 55;
   if(SCREEN_WIDTH <= MIN_WIDTH) --VER_COL;

   last_rmode = rcvr_mode = RCVR_MODE_HOLD;
   tow = 0;
   time_flags = 0x0001;     // UTC
   plot_watch = 1;
   plot_adev_data = 0;      // adevs
   plot_sat_count = 0;      // satellite count
   adev_period = (-1.0);
   osc_gain = user_osc_gain = (69.0);
   if(user_set_rounding == 0) {
      round_temp = 3;
   }

   for(i=0; i<NUM_PLOTS+DERIVED_PLOTS; i++) {     // initialize default plot parameters
      plot[i].show_stat = AVG;          // statistic to show
   }

   plot[TEMP].plot_id = "TEMP1";
   plot[TEMP].ref_scale = 1.0F;

   plot[BATTV].plot_id = "BATv";
   plot[BATTV].units = "V";
   plot[BATTV].ref_scale = 1.0F;
// if(user_set_dac_plot == 0) plot[DAC].show_plot = 1;
   if(user_set_dac_float == 0) plot[BATTV].float_center = 1;

   plot[BATTI].plot_id = "BATi";
   plot[BATTI].units = "A";
   plot[BATTI].ref_scale = 1.0F;
   if(user_set_osc_plot == 0)  plot[BATTI].show_plot = 1;
   if(user_set_osc_float == 0) plot[BATTI].float_center = 1;

   plot[LUX1].plot_id = "LUX";
   plot[LUX1].ref_scale = 1.0F;
   plot[LUX1].ref_scale = 1;
plot[LUX1].show_stat = SHOW_SPAN;
// if(user_set_pps_plot == 0) plot[PPS].show_plot = 1;
   if(user_set_pps_float == 0) plot[LUX1].float_center = 1;
   if(show_fc) plot[LUX1].units = "fc";
   else        plot[LUX1].units = "lux";

   plot[LUX2].plot_id = "LUM";    // !!!!!
   plot[LUX2].ref_scale = 1.0F;
   plot[LUX2].ref_scale = 1;
plot[LUX2].show_stat = SHOW_SPAN;
   if(user_set_pps_float == 0) plot[LUX2].float_center = 1;
   if(show_cp) plot[LUX2].units = "cp";
   else        plot[LUX2].units = "lum";

   plot[LEDV].plot_id = "LEDv";
   plot[LEDV].units = "V";
   plot[LEDV].ref_scale = 1.0F;
   plot[LEDV].float_center = 1;

   plot[LEDI].plot_id = "LEDi";
   plot[LEDI].units = "A";
   plot[LEDI].ref_scale = 1.0F;
   plot[LEDI].float_center = 1;

   plot[PWMHZ].plot_id = "PWM";
   plot[PWMHZ].units = "Hz";
   plot[PWMHZ].ref_scale = 1.0F;
   plot[PWMHZ].float_center = 1;

   plot[TC2].plot_id = "TEMP2";
   plot[TC2].units = degc;
   plot[TC2].ref_scale = 1.0F;
   plot[TC2].float_center = 1;

   plot[BLUEHZ].plot_id = "BLU";
   plot[BLUEHZ].ref_scale = 1.0F;
   plot[BLUEHZ].float_center = 1;

   plot[GREENHZ].plot_id = "GRN";
   plot[GREENHZ].ref_scale = 1.0F;
   plot[GREENHZ].float_center = 1;

   plot[REDHZ].plot_id = "RED";
   plot[REDHZ].ref_scale = 1.0F;
   plot[REDHZ].float_center = 1;

   plot[WHITEHZ].plot_id = "WHT";
   plot[WHITEHZ].ref_scale = 1.0F;
   plot[WHITEHZ].float_center = 1;

   plot[BLUEHZ].units = "Hz";     // default color values to pct
   plot[GREENHZ].units = "Hz";
   plot[REDHZ].units = "Hz";
   plot[WHITEHZ].units = "Hz";
   if(show_color_pct) {        // color values are in percent
      plot[BLUEHZ].units = "%";
      plot[GREENHZ].units = "%";
      plot[REDHZ].units = "%";
      plot[WHITEHZ].units = "%";
   }
   else if(show_color_uw) {   // color values are in uW/cm^2
      plot[BLUEHZ].units = "uW";
      plot[GREENHZ].units = "uW";
      plot[REDHZ].units = "uW";
      plot[WHITEHZ].units = "uW";
   }
                   
   plot[AUXV].plot_id = "AUXv";
   plot[AUXV].units = "V";
   plot[AUXV].ref_scale = 1.0F;
   plot[AUXV].float_center = 1;

   plot[BATTW].plot_id = "BATw";
   plot[BATTW].units = "W";
   plot[BATTW].ref_scale = 1.0F;
   plot[BATTW].float_center = 1;

   plot[LEDW].plot_id = "LEDw";
   plot[LEDW].units = "W";
   plot[LEDW].ref_scale = 1.0F;
   plot[LEDW].float_center = 1;

   plot[EFF].plot_id = "Eff";
   plot[EFF].units = "%";
   plot[EFF].ref_scale = 1.0F;
   plot[EFF].float_center = 1;

   set_cct_id();
   plot[CCT].units = degk;
   plot[CCT].ref_scale = 1.0F;
   plot[CCT].float_center = 1;

   if(user_set_temp_filter == 0) undo_fw_temp_filter = 0;
   if(user_set_clock_plot == 0) { plot_digital_clock = 0; user_set_clock_plot = 1; }

   unit_file_name = "luxor";
   if(log_file == 0) sprintf(log_name, "%s.log", unit_file_name);
   config_set = 1;
}



int tune_exists(char *file, int add_root)
{
char tune[MAX_PATH+1];
FILE *f;

   // This routine sees if a sound file exists

   if(add_root) sprintf(tune, "%s%s", heather_path, file);
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

   chime_file = tune_exists(CHIME_FILE, 1);
   alarm_file = tune_exists(ALARM_FILE, 1);    // see if alarm sound file exists
   leap_file  = tune_exists(LEAP_FILE, 1);     // see if leapsecond sound file exists
   sun_file   = tune_exists(SUNRISE_FILE, 1);  // see if sunrise tune sound file exists
   noon_file  = tune_exists(NOON_FILE, 1);     // see if solar noon tune sound file exists
   bell_file  = tune_exists(BELL_FILE, 1);     // see if ships bell sound file exists

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
}

void play_tune(char *file, int add_root)
{
char tune[MAX_PATH+1];

   // This routine plays a sound file asynchronously (non-blocking)

   if(!sound_on) return;

   if(add_root) sprintf(tune, "%s%s", heather_path, file);
   else         sprintf(tune, "%s", file);


   #ifdef WINDOWS
      PlaySoundA(tune, NULL, SND_ASYNC);
   #endif

   #ifdef __linux__
      char shell_cmd[MAX_PATH+64];
      sprintf(shell_cmd, "/bin/sh -c \"aplay -q %s\" &", tune);
      system(shell_cmd); 
   #endif
   
   #ifdef __MACH__
      char shell_cmd[MAX_PATH+64];
      sprintf(shell_cmd, "afplay %s &", tune);
      system(shell_cmd); 
   #endif
}


void alarm_clock()
{
   if(alarm_file) play_tune(ALARM_FILE, 1);
   else if(chord_file == 2) play_tune(USER_CHORD_FILE, 1);
   else if(chord_file) play_tune(CHORD_FILE, 0);
   else BEEP(1);
}

void cuckoo_clock()
{
char fn[MAX_PATH+1];

   if(singing_clock) {  // sing a song file
      get_alarm_time();
      sprintf(fn, SONG_NAME, g_minutes);
      play_tune(fn, 1);
      cuckoo_beeps = 0;
   }
   else if(ships_clock && (ring_bell >= 0) && (bell_number >= 0) && (bell_number < MAX_BELLS)) {
      if(bells[ring_bell][bell_number]) play_tune(BELL_FILE, 1);
      if(++bell_number >= MAX_BELLS) bell_number = (-1);
   }
   else if(chime_file) play_tune(CHIME_FILE, 1);
   else if(notify_file == 2) play_tune(USER_NOTIFY_FILE, 1);
   else if(notify_file) play_tune(NOTIFY_FILE, 0);
   else BEEP(2);
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

LH_IPCONN *IPC;

HANDLE hSerial = INVALID_HANDLE_VALUE;
DCB dcb = { 0 };

void SetDtrLine(u08 on)
{   
   if(hSerial != INVALID_HANDLE_VALUE) {
      EscapeCommFunction(hSerial, on ? SETDTR : CLRDTR);
   }
}

void SetRtsLine(u08 on)
{   
   if(hSerial != INVALID_HANDLE_VALUE) {
      EscapeCommFunction(hSerial, on ? SETRTS : CLRRTS);
   }
}

void SetBreak(u08 on)
{   
   if(hSerial != INVALID_HANDLE_VALUE) {
      EscapeCommFunction(hSerial, on ? SETBREAK : CLRBREAK);
   }
}


void kill_com(void)
{
   if(com_port > 0) {       // COM port in use: close it
      SetDtrLine(0);
      if(hSerial != INVALID_HANDLE_VALUE) CloseHandle(hSerial);
      hSerial = INVALID_HANDLE_VALUE;
   }
   else {                    // TCP connection: close the connection
if(hSerial != INVALID_HANDLE_VALUE) CloseHandle(hSerial);
hSerial = INVALID_HANDLE_VALUE;
       if(IPC != NULL) {
          delete IPC;
          IPC = NULL;
       }
   }
}

void init_tcpip()
{
   IPC = new LH_IPCONN();

   IPC->connect(IP_addr, DEFAULT_PORT_NUM);

   if(!IPC->status()) {
process_com = 0;
return;
       error_exit(2, IPC->message);
   }
}

void init_com(void)
{
   // open communications to the receiver

   kill_com();   // in case COM port already open
   if(rcvr_type == NO_RCVR) { //rxx
      return;
   }

   if((com_port == 0) && IP_addr[0]) {
      //
      // TCP: In Windows, COM0 with process_com=TRUE means we're using TCP/IP 
      //
      if(process_com) {
         init_tcpip();
         com_running = 2;
         first_request = 1;
         com_data_lost = 0;
         last_com_time = this_msec = GetMsecs();
      }
      return;    
   }
if(com_port == 0) return;

   // kd5tfd hack to handle comm ports > 9
   // see http://support.microsoft.com/default.aspx?scid=kb;%5BLN%5D;115831 
   // for the reasons for the bizarre comm port syntax

   char com_name[20];
   sprintf(com_name, "\\\\.\\COM%d", com_port);
   hSerial = CreateFile(com_name,
                        GENERIC_READ | GENERIC_WRITE, 
                        0,
                        0,
                        OPEN_EXISTING, 
                        FILE_ATTRIBUTE_NORMAL, 
                        0
   );

   if(hSerial == INVALID_HANDLE_VALUE) {
      sprintf(out, "Can't open com port: %s", com_name);
process_com = 0;
kill_com();
return;
      error_exit(10001, out);
   }

   dcb.DCBlength = sizeof(dcb);
   if(!GetCommState(hSerial, &dcb)) {
      error_exit(10002, "Can't GetCommState()");
   }

   dcb.BaudRate         = baud_rate;

   dcb.ByteSize         = data_bits;

   if     (parity == ODD_PAR)  dcb.Parity = ODDPARITY;  // !!!! (parity == 1)
   else if(parity == EVEN_PAR) dcb.Parity = EVENPARITY;
   else                        dcb.Parity = NOPARITY;

   if(stop_bits == 2) dcb.StopBits = TWOSTOPBITS;
   else               dcb.StopBits = ONESTOPBIT;

   dcb.fBinary          = TRUE;
   dcb.fOutxCtsFlow     = FALSE;
   dcb.fOutxDsrFlow     = FALSE;
   dcb.fDtrControl      = DTR_CONTROL_ENABLE;
   dcb.fDsrSensitivity  = FALSE;
   dcb.fOutX            = FALSE;
   dcb.fInX             = FALSE;
   dcb.fErrorChar       = FALSE;
   dcb.fNull            = FALSE;
   dcb.fRtsControl      = RTS_CONTROL_ENABLE;
   dcb.fAbortOnError    = FALSE;

   if(!SetCommState(hSerial, &dcb)) {
      error_exit(10003, "Can't SetCommState()");
   }

   // set com port timeouts so we return immediately if no serial port
   // character is available
   COMMTIMEOUTS cto = { 0, 0, 0, 0, 0 };
   cto.ReadIntervalTimeout = MAXDWORD;
   cto.ReadTotalTimeoutConstant = 0;
   cto.ReadTotalTimeoutMultiplier = 0;

   if(!SetCommTimeouts(hSerial, &cto)) {
      error_exit(10004, "Can't SetCommTimeouts()");
   }

   SetDtrLine(1);
   if(rcvr_type == ACRON_RCVR) {  // power up the RS-232 interface
      SetDtrLine(1);  // drive +12V
      SetRtsLine(0);  // drive -12V
      Sleep(500);
   }
   com_running = 1;
   first_request = 1;
}

int check_incoming_data(void)
{
   // return true if com device has a character available

   if(rcvr_type == NO_RCVR) { // alternate ready/not ready each check
      return (++com_tick & 0x01);
   }

   if(com_error & 0x01) return TRUE; // !!!! return 0;
   if(next_serial_byte < rcvr_byte_count) return TRUE;  // we have already read and buffered the char

   rcvr_byte_count = 0;
   next_serial_byte = 0;

   if(com_port != 0) {       // COM port in use: read bytes from serial port
if(hSerial == INVALID_HANDLE_VALUE) return FALSE;
      ReadFile(hSerial, &rcvr_buf[0], sizeof(rcvr_buf), &rcvr_byte_count, NULL);
      if(rcvr_byte_count == 0) return FALSE;  // no serial port data is available
   }
   else {                     // TCP connection: read a byte from Winsock 
      if(!IPC->status()) {
          error_exit(22, IPC->message);
      }

      rcvr_byte_count = IPC->read_block(rcvr_buf, sizeof(rcvr_buf));

      if(rcvr_byte_count == 0) return FALSE;
   }

   return TRUE;
}

u08 get_serial_char(void)
{
int flag;
u08 c;
DWORD err;

   // return the next byte from the com device

   if(rcvr_type == NO_RCVR) { // no receiver in use, just return something
      return 0;
   }

   if(com_error & 0x01) return 0;

   if(next_serial_byte < rcvr_byte_count) {   // return byte previously fetched by check_incoming_data()
      c = rcvr_buf[next_serial_byte++];
      return c;
   }

   rcvr_byte_count = 0;
   next_serial_byte = 0;

   while(rcvr_byte_count == 0) {  // wait until we have a character
      if(com_port != 0)  {       // COM port in use: read a byte from serial port
if(hSerial == INVALID_HANDLE_VALUE) return FALSE;
         flag = ReadFile(hSerial, &rcvr_buf[0], sizeof(rcvr_buf), &rcvr_byte_count, NULL);
         // !!!!!! test for parity errors, etc and set rcv_errpr
         ClearCommError(hSerial, &err, NULL);
         if(err & (CE_FRAME | CE_OVERRUN | CE_RXPARITY | CE_RXOVER)) {
            rcv_error |= err;
//            sprintf(plot_title, "Err=%ld", err);
//            vidstr(0,0, RED, plot_title);
//            refresh_page();
         }
      }
      else {                     // TCP connection: read a byte from Winsock 
         if(!IPC->status()) {
            error_exit(23, IPC->message);
         }

         flag = rcvr_byte_count = IPC->read_block(rcvr_buf, sizeof(rcvr_buf));
      }

      update_pwm();   // if doing pwm temperature control
   }

   if(flag && rcvr_byte_count) {  // succesful read
      c = rcvr_buf[next_serial_byte++];
      return c;
   }

   if(com_error_exit) {
      error_exit(222, "Serial receive error");
   }
   else {
      com_error |= 0x01;
      ++com_errors;
   }
   return 0;
}

void sendout(unsigned char val)
{
DWORD written;
int flag;
static U8 xmit_buffer[RCVR_BUF_SIZE]; 
static S32 x = 0;    

   // send data to the receiver.  
   // we queue up data until end of message to reduce per-packet overhead 

   if(rcvr_type == NO_RCVR) { //rxx
      return;
   }

   if(process_com == 0) return;
   if(com_error & 0x02) return;
   if(just_read) return;
   if(no_send) return;

if(FAKE_ACRON_RCVR && (rcvr_type == ACRON_RCVR)) {  // simulate echo by sticking char in rcv buffer
   rcvr_buf[next_serial_byte] = val;
   ++rcvr_byte_count;
   return;
}

   // we buffer up output chars until buffer full or end of message flag seen
   xmit_buffer[x++] = val;
   if((x < (S32)sizeof(xmit_buffer)) && (eom_flag == 0)) return;
   eom_flag = 0;

   update_pwm();   // if doing pwm temperature control

   if(com_port != 0) {       // COM port in use: send bytes via serial port
      if(hSerial == INVALID_HANDLE_VALUE) return;
      flag = WriteFile(hSerial, xmit_buffer, x, &written, NULL);
      if(written == x) written = 1;
      else             written = 0;
   }
   else {                    // TCP connection: send byte via Winsock 
      IPC->send_block(xmit_buffer, x);

      if(!IPC->status()) {
         error_exit(24, IPC->message);
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
         com_error |= 0x02;
         ++com_errors;
      }
   }
}


void SendBreak()
{
   // This routine sends a 300 msec BREAK signal to the serial port

   SetBreak(1);
   Sleep(300);
   SetBreak(0);
   Sleep(100);
}

void init_hardware(void) 
{
   init_com();
   init_screen();

   if(NO_SCPI_BREAK && (rcvr_type == SCPI_RCVR)) ; else 
   if(nortel == 0) {    // To wake up a Nortel NTGS55A receiver
      SendBreak(); 
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

   if(++kbd_in >= KBD_Q_SIZE) kbd_in = 0;
   if(kbd_in == kbd_out) {  // kbd queue is full
      if(--kbd_in < 0) kbd_in = KBD_Q_SIZE-1;
   }
   else {  // put keystoke in the queue
      kbd_queue[kbd_in] = key;
   }
}

int win_kbhit(void)
{
   // return true if a keystroke is available
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


#else // __linux__  __MACH__  OS dependent I/O routines


struct addrinfo hints = { 0 };  // for TCP/IP connection
struct addrinfo *res;


void outp(unsigned port, unsigned val)
{
   // place holder for doing fan control via I/O port bits
   // zork!!!
}

void SetDtrLine(u08 on)
{   
int val;

   if(com_fd < 0) return;  // !!!! zork what if TCPIP connected

   val = TIOCM_DTR;
   if(on) ioctl(com_fd, TIOCMBIS, &val);
   else   ioctl(com_fd, TIOCMBIC, &val);
}

void SetRtsLine(u08 on)
{   
int val;

   if(com_fd < 0) return;  // !!!! zork what if TCPIP connected

   val = TIOCM_RTS;
   if(on) ioctl(com_fd, TIOCMBIS, &val);
   else   ioctl(com_fd, TIOCMBIC, &val);
}

void SetBreak(u08 on)
{   
   if(com_fd < 0) return;  // !!!! zork what if TCPIP connected

   if(on) ioctl(com_fd, TIOCSBRK, 0);
   else   ioctl(com_fd, TIOCCBRK, 0);
}


void SendBreak()
{
   // This routine sends a 300 msec BREAK signal to the serial port
   if(com_fd < 0) return;  // !!!! should also return if IP connected

// tcsendbreak(com_fd, 0);  // send a 200..500 ms (supposedly) break
   SetBreak(1);  // a more reliable way to do the BREAK
   Sleep(500);
   SetBreak(0);
   Sleep(100);
}


void kill_com(void)
{
   // close the serial communications device

   if(com_fd < 0) return;

   tcsetattr(com_fd, TCSANOW, &oldtio);
   close(com_fd);

   com_fd = (-1);
}


void init_tcpip()
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

printf("init tcpip to %s\n", IP_addr);  // zork - show_debug_info
   strcpy(ip_string, IP_addr);

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
printf("ERROR, could not get IP addr info:%s  port:%s. err=%d\n", ip_string, port_string, gai_err);
      process_com = 0;
      usb_port = 0;
      com_port = 0;
      return;
   }

   com_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
   if(com_fd < 0) {
      if(!af_given) { // The local machine can not handle this address family. Try the other address family.
        use_v6 = !use_v6;
        af_given = 1;
        close(com_fd);
        com_fd = (-1);
        goto retry_resolve;
      }
      error_exit(20000, "ERROR could not open IP socket");
   }
printf("%s socket opened!\n", use_v6?"IPv6":"IPv4");
printf("Attempting to connect() to socket\n");

   if(connect(com_fd, res->ai_addr, res->ai_addrlen) < 0) {
      if(!af_given) {
        // we can not connect using that address family. Try the other.
        // this happens when the machine has link local IPv6 or IPv4 only, and we are trying
        // to reach a remote IPv6/IPv4 address. In this case we get an immediate error,
        // and can retry using the other protocol if the other address is dual-stacked.
        use_v6 = !use_v6;
        af_given = 1;
        close(com_fd);
        com_fd = (-1);
        goto retry_resolve;
      }

      error_exit(20002, "ERROR connecting to IP socket.");
   }

printf("connected!\n");
   x = fcntl(com_fd, F_GETFL, 0);
   fcntl(com_fd, F_SETFL, x | O_NONBLOCK);
printf("set ip connection to non-blocking\n");   
}



void init_com(void)
{
int col, i;
int sync;

   // Initialize the serial device for asynchronous non-blocking communication
   // The device can be a serial port or ethernet connection

   kill_com();   // in case COM port already open
   if(rcvr_type == NO_RCVR) { //rxx
      return;
   }

   if((com_port == 0) && (usb_port == 0) && IP_addr[0]) {
      //
      // TCP: In Windows, COM0 with process_com=TRUE means we're using TCP/IP 
      //
      if(process_com) {
         init_tcpip();
         com_running = 2;
         first_request = 1;
         com_data_lost = 0;
         last_com_time = this_msec = GetMsecs();
      }
      return;    
   }

   if((usb_port == 9999) && com_dev[0]) {  // user set com device name with -id=xxx command line option
   }
   else if(usb_port == 999) {  // use symlink to /dev/heather
      strcpy(com_dev, "/dev/heather");
   }
   else if(usb_port) {    // USB serial port
      #ifdef __MACH__
         if(usb_port <= 1) sprintf(com_dev, "/dev/tty.usbserial");
         else              sprintf(com_dev, "/dev/tty.usbserial%d", usb_port-1);
      #else
         sprintf(com_dev, "/dev/ttyUSB%d", usb_port-1);
      #endif
   }
   else if(com_port) {    // hardware serial port
      #ifdef __MACH__
         sprintf(com_dev, "/dev/ttys%d", com_port-1);
      #else
         sprintf(com_dev, "/dev/ttyS%d", com_port-1);
      #endif
   }
   else {  // should never happen
      com_running = 1;
      first_request = 1;
      process_com = 0;
      return;
   }
printf("opening com device:%s\n", com_dev); // zork - show_debug_info
fflush(stdout);

// 
// we init the com device here
// 

   #ifndef O_RSYNC
      sync = O_SYNC;
   #else
      sync = O_RSYNC;
   #endif 
   sync = O_DSYNC | sync;
sync = 0;

   com_fd = open(com_dev, O_RDWR | O_NOCTTY | O_NONBLOCK | sync);
   if(com_fd < 0) {
      printf("Com open failed:%d\n", com_fd);
      printf("Do you have permission to access the serial device?\n");
      printf("try:  sudo usermod -a -G dialout user_name\n");
      process_com = 0;
      com_running = 1;
      first_request = 1;
      return;
   }

   tcgetattr(com_fd, &oldtio); /* save current port settings */

   /* set new port settings for non-canonical input processing */
   bzero(&newtio, sizeof(newtio));
   if     (data_bits == 7) newtio.c_cflag = CS7 | CLOCAL | CREAD;
   else if(data_bits == 6) newtio.c_cflag = CS6 | CLOCAL | CREAD;
   else if(data_bits == 5) newtio.c_cflag = CS5 | CLOCAL | CREAD;
   else                    newtio.c_cflag = CS8 | CLOCAL | CREAD;

   if(stop_bits == 2) newtio.c_cflag |= (CSTOPB);

   if(parity == ODD_PAR) {   // odd parity
      newtio.c_cflag |= (PARENB | PARODD);
   }
   else if(parity == EVEN_PAR) {  // even parity
      newtio.c_cflag |= (PARENB);
   }
   else {  // no parity
   }

   newtio.c_iflag = IGNPAR;   // ignore parity errors on reads
   newtio.c_oflag = 0;
   newtio.c_lflag = 0;
   newtio.c_cc[VMIN] = 0;
   newtio.c_cc[VTIME] = 0;

   if     (baud_rate == 150)    i = B150;
   else if(baud_rate == 300)    i = B300;
   else if(baud_rate == 600)    i = B600;
   else if(baud_rate == 1200)   i = B1200;
   else if(baud_rate == 2400)   i = B2400;
   else if(baud_rate == 4800)   i = B4800;
   else if(baud_rate == 9600)   i = B9600;
   else if(baud_rate == 19200)  i = B19200;
   else if(baud_rate == 38400)  i = B38400;
   else if(baud_rate == 57600)  i = B57600;
   else if(baud_rate == 115200) i = B115200;
   else if(baud_rate == 230400) i = B230400;
   else i = B9600;
   cfsetispeed(&newtio, i);
   cfsetospeed(&newtio, i);

   tcflush(com_fd, TCIFLUSH);
   tcsetattr(com_fd, TCSANOW, &newtio);
   fcntl(com_fd, F_SETFL, FNDELAY); // make sure port is in non-blocking mode

   SetDtrLine(1);
   if(rcvr_type == ACRON_RCVR) {  // power up the RS-232 interface
      SetDtrLine(1);  // drive +12V
      SetRtsLine(0);  // drive -12V
      Sleep(500);
   }
   com_running = 1;
   first_request = 1;
}

int check_incoming_data(void)
{
int flag;

   // return true if we have a serial port byte available to read

   if(rcvr_type == NO_RCVR) {  //rxx
      return(++com_tick & 0x01);
   }

   if(com_fd < 0) return 0;

   if(com_error & 0x01) {
printf("com avail err:%02X\n", com_error);
      return -100; // !!!! return 0;
   }
   if(next_serial_byte < rcvr_byte_count) return 2000|next_serial_byte;  // we have already read and buffered the char

   rcvr_byte_count = 0;
   next_serial_byte = 0;

   if(1 || (com_port != 0) || (usb_port != 0)) {  // zork COM port in use: read bytes from serial port
      flag = read(com_fd, &rcvr_buf[0], sizeof(rcvr_buf));
      if(flag < 0) {  // !!!! error or no serial data available - we should check errno()
         rcvr_byte_count = 0;
         return 0;
      }
      else {
         rcvr_byte_count = flag;
         if(rcvr_byte_count == 0) return FALSE;  // no serial port data is available
      }
   }

   return (1000|rcvr_byte_count);
}


u08 get_serial_char(void)
{
int flag;
u08 c;

   // get a character from the serial port
   if(rcvr_type == NO_RCVR) { //rxx
      return 0;
   }

   if(com_fd < 0) return 0;

   if(com_error & 0x01) return 0;

   if(next_serial_byte < rcvr_byte_count) {   // return byte previously fetched by check_incoming_data()
      c = rcvr_buf[next_serial_byte++];
      return c;
   }

   rcvr_byte_count = 0;
   next_serial_byte = 0;

   while(rcvr_byte_count == 0) {  // wait until we have a character
      if(1 || (com_port != 0) || (usb_port != 0)) {  // zork COM port in use: read a byte from serial port
         flag = read(com_fd, &rcvr_buf[0], sizeof(rcvr_buf));
         if(flag < 0) {  // !!!! com error or no serial data available - we should check errno()
            flag = rcvr_byte_count = 0;
         }
         else rcvr_byte_count = flag;
      }

      update_pwm();   // if doing pwm temperature control
   }

   if(flag && rcvr_byte_count) {  // succesful read
      c = rcvr_buf[next_serial_byte++];
      return c;
   }

   if(com_error_exit) {
      error_exit(222, "Serial receive error");
   }
   else {
      com_error |= 0x01;
      ++com_errors;
   }
   return 0;
}

void sendout(unsigned char val)
{
unsigned long written;
int flag;
static unsigned char xmit_buffer[RCVR_BUF_SIZE]; 
static long x = 0; 

   // buffer up serial output chars until end of message is seen, then send the buffer

   if(rcvr_type == NO_RCVR) { //rxx
      return;
   }

   if(com_fd < 0) return;

   if(process_com == 0) return;
   if(com_error & 0x02) return;
   if(just_read) return;
   if(no_send) return;

if(FAKE_ACRON_RCVR && (rcvr_type == ACRON_RCVR)) {  // simulate echo by sticking char in rcv buffer
   rcvr_buf[next_serial_byte] = val;
   ++rcvr_byte_count;
   return;
}

   // we buffer up output chars until buffer full or end of message flag seen
   xmit_buffer[x++] = val;
   if((x < (long)sizeof(xmit_buffer)) && (eom_flag == 0)) return;
   eom_flag = 0;

   update_pwm();   // if doing pwm temperature control

   if(1 || (com_port != 0) || (usb_port != 0)) {   // zork COM port in use: send bytes via serial port
      flag = write(com_fd, xmit_buffer, x);
      if(flag == x) written = 1;
      else          written = 0;
   }

   x = 0;

   if((flag == 0) || (written != 1)) {
      if(com_error_exit) {
         error_exit(3, "Serial transmit error");
      }
      else {
         com_error |= 0x02;
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
   init_com();
   init_screen();

   if(NO_SCPI_BREAK && (rcvr_type == SCPI_RCVR)) ; 
   else if(nortel == 0) {    // To wake up a Nortel NTGS55A, etc receiver
      SendBreak(); 
   }

   need_screen_init = 0;
}
#endif  // WINDOWS



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

   if(pix_map && x11_io_done) {
      XCopyArea(display,pix_map,screen,gc, 0,0, SCREEN_WIDTH,SCREEN_HEIGHT, 0,0);   
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
int val;
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
             sscanf(&buf[i+1], "%X", &val);
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


void init_screen(void)
{
Cursor cur;
unsigned char *vfx_font;
int font_height;
int i;
unsigned int j;

   // Initialize the graphics screen

   config_screen(3);  // initialize screen rendering variables
// VFX_io_done = 1;
   kill_screen();    // release any screen data structures currently in use

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
      window_name = "Lady Heather X11";
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
printf("Display size: %dx%d\n", display_width,display_height);

   if(go_fullscreen && (display_width >= 640) && (display_height >= 480)) {
      SCREEN_WIDTH = display_width;
      SCREEN_HEIGHT= display_height;
      go_fullscreen = 0;
   }


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
printf("creating %dx%d window at %d,%d\n", width,height, x,y);
   screen = XCreateSimpleWindow(display, RootWindow(display, screen_num),
                             x,y, width,height, 0,
                             WhitePixel(display, screen_num),
                             BlackPixel(display, screen_num));

   if(screen == 0) {
       error_exit(10010, "Could not create X11 window");
   }

printf("Get window attributes\n");
   XGetWindowAttributes(display,screen, &wattr);  // winz
x11_left = wattr.x;
x11_top = wattr.y;
if(x11_left < 0) x11_left = 0;
if(x11_top < 0) x11_top = 0;
printf("Window color depth: %d  xy:%dx%d\n", wattr.depth, x11_left,x11_top);


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


   /*  Choose which events we want to handle  */

   XSelectInput(display,screen, ExposureMask | KeyPressMask | PointerMotionMask |
                ButtonPressMask | ButtonReleaseMask | StructureNotifyMask);   // winz


   cur = XCreateFontCursor(display, 132);
   if(cur) XDefineCursor(display,screen, cur);    // winz


   XSetForeground(display,gc, WhitePixel(display, screen_num));


   /*  Display Window  */

   XMapWindow(display,screen);  // winz
printf("window mapped\n");
   pix_map = XCreatePixmap(display,screen, width,height, wattr.depth); // winz
   if(pix_map == 0) {
      error_exit(30000, "Could not create pix_map");
   }
printf("pixmap created\n");
   Sleep(X11_sleep);
   XFlush(display);

   config_screen(4);  // re-initialize screen rendering variables
                     // to reflect any changes due to font size
   XFlush(display);

   #ifdef BUFFER_LLA
//lla clear_lla_points();
   #endif

printf("screen configured\n");   
   erase_rectangle(0,0, SCREEN_WIDTH,SCREEN_HEIGHT);
printf("screen init done\n");   
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

void dot(int x, int y, u08 color)
{
   // draw a dot on the screen

   if(display == 0) return;
   set_x11_color(color);
   XDrawPoint(display,win, gc, x,y);
   flush_x11();
}  

u08 get_pixel(COORD x,COORD y)
{
unsigned long pixel;
int i;

   // get a pixel from the screen capture image buffer

   if(capture_image == 0) return 0;

   pixel = XGetPixel(capture_image, x,y);
   for(i=0; i<16; i++) {  // convert screen value to color code
      if(pixel == palette[i]) return i;
   }
   return 0;
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

   if(++kbd_in >= KBD_Q_SIZE) kbd_in = 0;
   if(kbd_in == kbd_out) {  // kbd queue is full
      if(--kbd_in < 0) kbd_in = KBD_Q_SIZE-1;
   }
   else {  // put keystoke in the queue
      kbd_queue[kbd_in] = key;
   }
}

int x11_kbhit(void)
{
   // return true if a keystoke is available

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


int get_x11_event()
{
XEvent report;
KeySym key;
char buf[20];
int i;

   // This is the X11 event handler.

   if(display == 0) return 0;

   if(!XEventsQueued(display, QueuedAlready)) {  // no events available to process
      return 0;
   }
   XNextEvent(display, &report);

   switch(report.type) {  // act on the event
      case Expose:
          if(report.xexpose.count != 0 ) break;
if(show_debug_info) printf("Expose event seen\n");  // zork - show_debug_info
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
if(show_debug_info) printf("configure:  wh%d %d  tl:%d %d  mouse:%d\n", new_width,new_height, x11_top,x11_left, this_button);  // zork - show_debug_info
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
         else if(key ==  XK_Escape)    add_kbd(0x1B);
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
         else if(key ==  XK_F11) {
            f11_flag = 1;
//          go_fullscreen = 1;
            need_resize = 1;
            new_width = display_width;
            new_height= display_height;
         }
         else if(key ==  XK_F12)       add_kbd(0);
         return 3;
         break;

if(0) {  // we now use XQueryPointer to do this
      case MotionNotify:
         mouse_x = report.xmotion.x_root;
         mouse_y = report.xmotion.y_root;
         mouse_y -= 48;
         x11_mouse = 1;
         return 4;
         break;
}

#ifdef PRESLEY       // we now use XQueryPointer in get_mouse_info to do this
      case ButtonPress:            /*  Fall through  */
          button = report.xbutton.button;
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
if(show_debug_info) printf("Press: this:%d  last:%d!\n", this_button, last_button);      
          return 5;
          break;
          
       case ButtonRelease:
if(show_debug_info) printf("Release\n"); 
          return 6;      
          break; 
#endif


    case ClientMessage:
if(show_debug_info) printf("Client message:%ld\n", report.xclient.data.l[0]); //zork - show_debug_info
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

void dot(int x, int y, u08 color)
{
   if(stage == 0) return;

   VFX_io_done = 1;
   VFX_pixel_write(stage, x,y, palette[color]);
}

u08 get_pixel(COORD x,COORD y)
{
S32 pixel;
int i;

   if(stage == 0) return 0;
   VFX_io_done = 1;

   pixel = VFX_pixel_read(stage, x,y) & 0xFFFF;
   for(i=0; i<16; i++) {  // convert screen value to color code
      if(((u32) pixel) == (palette[i]&0xFFFF)) return i;
   }
   return 0;
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

void get_screen_size(int init)
{
RECT r;

   if(SystemParametersInfo(SPI_GETWORKAREA, 0, &r, 0)) {
      display_width = (unsigned) (r.right - r.left);
      display_height = (unsigned) (r.bottom - r.top);

      if(go_fullscreen && init) {
         go_fullscreen = 0;
         SCREEN_WIDTH = (int) (display_width - 10);
         SCREEN_HEIGHT = (int) (display_height- 32);

         if(SCREEN_WIDTH <= 640) SCREEN_WIDTH = 640;
         if(SCREEN_HEIGHT <= 480) SCREEN_HEIGHT = 480;

         screen_type = 'c';
         adjust_screen_options();
      }
   }
}

void init_screen(void)
{
   get_screen_size(1);

   config_screen(6);  // initialize screen rendering variables
   VFX_io_done = 1;
   kill_screen();     // release any screen data structures currently in use

   //set new video mode
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


   config_screen(7);  // re-initialize screen rendering variables
                      // to reflect any changes due to font size

   #ifdef BUFFER_LLA
//lla clear_lla_points();
   #endif

   erase_rectangle(0,0, SCREEN_WIDTH,SCREEN_HEIGHT);
   refresh_page();
}

void refresh_page(void)
{  
   // copies the virtual screen buffer to the physical screen
   // (or othwewise flips pages, etc for double buffered graphics)

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

   if(dot_font == 0) dot_font = &h_font_12[0];
   if(c & 0x80) c = ' '; // char out of range

   h = dot_font[8];      // char height

   p = (4*4) + (c*4);    // index of offset to char definition
   p = (dot_font[p+1] * 256) + dot_font[p+0];  // pointer to char definition
   w = dot_font[p];      // char width
   p += 4;               // pointer to char pattern

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
char work_str[256];
char *chp;
char ch;
S32 word_x[32];
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

      if((zoom_screen && (zoom_screen != 'P')) || (py < (((PLOT_ROW/TEXT_HEIGHT)*TEXT_HEIGHT) - (TEXT_HEIGHT*2)))) {
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

         if(*chp == DEGREES) {
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

      if(ch == DEGREES) {
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
       
      thick_line(x1+xoffset,y1+yoffset, x2+xoffset,y2+yoffset, color, VCharThickness);

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

   row *= TEXT_HEIGHT;  //!!!!!!
   col *= TEXT_WIDTH;
   i = 0;
   while((c = *s++)) {  // to save time,  we only draw the chars that changed
      vchar(col+TEXT_X_MARGIN, row+TEXT_Y_MARGIN, 1, color, c);
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

   // draw a thick line on the screen (using single pixel lines)

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

   // draw a thick line on the screen (using single pixel lines)
   // (this version tends to look better than xthick_line() defined above

   if(thickness <= 1) {
      line(x1,y1, x2,y2, color);
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
   // erase a vector graphics character

   erase_rectangle(x,y, VCharWidth+VCharThickness,VCharHeight+VCharThickness);
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


void erase_azel() 
{
   // erase the satellite map area on the screen
   if((zoom_screen == 0) && ((AZEL_ROW+AZEL_SIZE) >= PLOT_ROW)) {  // az/el map is in the normal plot window
      erase_rectangle(PLOT_COL+PLOT_WIDTH,AZEL_ROW, SCREEN_WIDTH-(PLOT_COL+PLOT_WIDTH),AZEL_SIZE);
   }
   else {
      erase_rectangle(AZEL_COL,AZEL_ROW, AZEL_SIZE,AZEL_SIZE);
   }

   azel_grid_color = GREY;
}


void erase_screen()
{
   // erase the screen
   erase_rectangle(0,0, SCREEN_WIDTH,SCREEN_HEIGHT);

   #ifdef PRECISE_STUFF
      plot_lla_axes(10);
   #endif
}


void erase_plot(int full_plot) 
{
   // erase the plot area on the screen

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
//  for increased performance these routines are OS dependent
//

void draw_circle(int x,int y, int r, int color, int fill)
{
   // draw a filled/hollow circle on the screen 

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

   #ifdef WIN_VFX
      if(stage == 0) return;

      if(fill) VFX_ellipse_fill(stage, x,y, r,r, palette[color]);
      else     VFX_ellipse_draw(stage, x,y, r,r, palette[color]);
   #endif
}


void line(COORD x1,COORD y1, COORD x2,COORD y2, u08 color)
{
   // draw a single pixel width line on the screen

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

}


void erase_rectangle(int x,int y, int width,int height)
{
   // erase a rectangle on the screen

   #ifdef WIN_VFX
      if(stage == 0) return;

      VFX_io_done = 1;
      VFX_rectangle_fill(stage, x,y, x+width-1,y+height-1, LD_DRAW, palette[BLACK]);
   #endif

   #ifdef USE_X11
      if(display == 0) return;

      set_x11_color(BLACK);
      XFillRectangle(display,win,gc, x,y, width,height);
      flush_x11();
   #endif
}





struct CMAP {   // Lady Heather standard color map (R,G,B)
  int r, g, b;
} cmap[16] = {
   {0,     0,   0 },      //0  - black                              
   {64,   64, 255 },      //1  - dim blue (now a little brighter for better visibility)
   {0,    96,   0 },      //2  - dim green                          
   {0,   192, 192 },      //3  - dim cyan                           
   {128,   0,   0 },      //4  - dim red                            
// {192,   0, 192 },      //5  - dim magenta                        
   {140,  96,   0 },      //5  - dim magenta is now grotty yellow   
   {255,  64,  64 },      //6  - brown                                     
   {192, 192, 192 },      //7  - dim white                          
   {96,   96,  96 },      //8  - grey                                    
   {128, 128, 255 },      //9  - blue (now purple, for better visibility)
   {0,   255,   0 },      //10 - green                                    
   {0,   255, 255 },      //11 - cyan                                    
   {255,   0,   0 },      //12 - red                                    
   {255,   0, 255 },      //13 - magenta                                    
   {255, 255,   0 },      //14 - yellow                                    
   {255, 255, 255 }       //15 - white                              
};

u08 bmp_pal[256];  // .BMP file color palette blue, green, red, filler for 16+1 colors

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


#define BM_HEADER_SIZE (0x0036 + 16*4)

//
//   !!! Note:  This code might not be non-Intel byte ordering ENDIAN compatible
//

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

   if(do_dump == 0) {  // keyboard commanded dump
      BEEP(5);         // beep because this can take a while to do
   }

#ifdef USE_X11   // capture the screen image into a buffer
   if(display == 0) return 0;
   if(capture_image) XDestroyImage(capture_image);
   capture_image = XGetImage(display,win, 0,0, SCREEN_WIDTH,SCREEN_HEIGHT, AllPlanes,XYPixmap);
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

   if(do_dump == 0) {
      BEEP(6);
   }

   if(redraw) redraw_screen();
   return 1;
}





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

   if(rcvr_type == UBX_RCVR) {  // messages use little endian format
      ENDIAN ^= 1;
   }
   else if(rcvr_type == ZODIAC_RCVR) {  // messages use little endian format
      ENDIAN ^= 1;
   }
   else if(rcvr_type == NVS_RCVR) {  // messages use little endian format
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
   // clean up in preparation for program exit
#ifdef WINDOWS
   if(timer_set) KillTimer(hWnd, 0);
#endif

#ifdef ADEV_STUFF
   log_adevs();          // write adev info to the log file
#endif

   end_log();            // close out the ASCII log file

   if(raw_file) {        // close out raw receiver data dump file
      fclose(raw_file);
      raw_file = 0;
   }

   if(sim_file) {
      fclose(sim_file);
      sim_file = 0;
   }

   if(hist_file) {
      fclose(hist_file);
      hist_file = 0;
   }

   if(temp_script) {
      fclose(temp_script);
      temp_script = 0;
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

   kill_com();        // turn off incoming interrupts
   kill_screen();     // kill graphics mode/window, return to text

   exit(reason);
}

void error_exit(int num,  char *s)
{
char err[1024];

   if(s) sprintf(err, "Error exit %d: %s", num, s);
   else  sprintf(err, "Error exit %d!", num);

   #ifdef __MACH__
      break_flag = 1; 
      printf("*** ERROR: %s\n", err);
   #endif

   #ifdef __linux__
      break_flag = 1; 
      printf("*** ERROR: %s\n", err);
   #endif

   #ifdef WIN_VFX
      break_flag = 1;  // inhibit cascading error messages, since WM_TIMER keeps running during SAL_alert_box()
      SAL_alert_box("Error", err);
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
float color_sum;

   // returns entry "i" from the plot queue.  If "i" is out of range, return
   // the last queue entry that we fetched.  This should show up as a duplicate
   // time stamp error...


   if(plot_q == 0) return q;
   if(i >= plot_q_size) return q;
   if(i < 0) return q;

   q = plot_q[i];

   if(luxor) {
      if(show_color_pct) {  // scale color values to percentages
         color_sum = (q.data[REDHZ] + q.data[GREENHZ] + q.data[BLUEHZ]) / 100.0F;
         if(color_sum) {
            q.data[REDHZ] /= color_sum;
            q.data[GREENHZ] /= color_sum;
            q.data[BLUEHZ] /= color_sum;
            q.data[WHITEHZ] /= color_sum;
         }
         else {
            q.data[REDHZ] = 0.0F;
            q.data[GREENHZ] = 0.0F;             
            q.data[BLUEHZ] = 0.0F;             
            q.data[WHITEHZ] = 0.0F;             
         }
      }
      else if(show_color_uw) {
         q.data[BLUEHZ] /= BLUE_SENS;    // blue
         q.data[GREENHZ] /= GREEN_SENS;  // green
         q.data[REDHZ] /= RED_SENS;      // red
         q.data[WHITEHZ] /= WHITE_SENS;  // white
      }
   }
   else {
      if(graph_lla == 0) q.data[THREE] = q.data[TEMP] - (q.data[DAC]*d_scale);  // !!!!
   }
   return q;
}


void put_plot_q(long i, struct PLOT_Q q)
{
   if(i >= plot_q_size) return;
   if(i < 0) return;

   plot_q[i] = q;
   return;
}

void clear_plot_entry(long i)
{
struct PLOT_Q q;
int j;

   for(j=0; j<NUM_PLOTS; j++) q.data[j] = 0.0;  // derived_plots OK

   q.sat_flags = 0;
   q.q_jd = 0.0;

   put_plot_q(i, q);

   if(i) {   // clear q entry marker, if set
      for(j=0; j<MAX_MARKER; j++) { 
         if(mark_q_entry[j] == i) mark_q_entry[j] = 0;
      }
   }
}

void free_adev()
{
   if(adev_q == 0) return;

   free(adev_q);
   adev_q = 0;
   return;
}

void alloc_adev()
{
#ifdef ADEV_STUFF
long i;

   // allocate memory for the adev data queue 
   free_adev();
   if(adev_q) return;  // memory already allocated

   ++adev_q_size;  // add an extra entry for overflow protection
   i = 0L;
   adev_q = (struct ADEV_Q *) calloc(adev_q_size+1L, sizeof (struct ADEV_Q));

   if(adev_q == 0) {
      sprintf(out, "Could not allocate %lu x %lu byte adev queue",
                    (unsigned long) (adev_q_size+1L), (unsigned long) sizeof(struct ADEV_Q));
      error_exit(50, out);
   }
#endif
}


void free_plot()
{
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

void alloc_gif()
{
#ifdef GIF_FILES
long i;

   // allocate memory for the .GIF file encoder
   if(hash_table) return;  // memory alrady allocated

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
      i = sizeof (float);
      i *= (max_fft_len+2);
      tsignal = (float *) (void *) calloc(i, 1);
   }
   if(fft_out == 0) {  
      i = sizeof (COMPLEX);
      i *= (max_fft_len/2+2);
      fft_out = (COMPLEX *) (void *) calloc(i, 1);
   }
   if(w == 0) {  
      i = sizeof (COMPLEX);
      i *= (max_fft_len/2+2);
      w = (COMPLEX *) (void *) calloc(i, 1);
   }
   if(cf == 0) {  
      i = sizeof (COMPLEX);
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

   alloc_adev();
   alloc_plot();
   alloc_gif();
   alloc_fft();

   reset_queues(0x03);

   printf("\nDone...\n\n");
}

void reset_marks(void)
{
int i;

   for(i=0; i<MAX_MARKER; i++) {  // clear all the plot markers
      mark_q_entry[i] = 0;
   }
}

void reset_queues(int queue_type)
{
#ifdef ADEV_STUFF
struct ADEV_Q q;

   if(queue_type & 0x01) { // reset adev queue
      adev_q_in = 0;
      adev_q_out = 0;
      adev_q_count = 0;
      adev_time = 0;
      pps_adev_time = 0; // ADEV_DISPLAY_RATE;
      osc_adev_time = 0; // ADEV_DISPLAY_RATE;
      last_bin_count = 0;
      pps_bins.bin_count = 0;
      osc_bins.bin_count = 0;

      pps_base_value = 0.0;
      osc_base_value = 0.0;

      q.pps = (OFS_SIZE) (0.0 - pps_base_value); // clear first adev queue entry
      q.osc = (OFS_SIZE) (0.0 - osc_base_value);
      if(adev_q) put_adev_q(adev_q_in, q);  // clear first adev queue entry

      reset_adev_bins();
   }
#endif

   if(queue_type & 0x02) {  // reset plot queue
      plot_q_in = plot_q_out = 0;
      plot_q_count = 0;
      plot_q_full = 0;
      plot_time = 0;
      plot_start = 0;
      plot_column = 0;
      stat_count = 0.0F;
      clear_plot_entry((long) plot_q_in);

      plot_title[0] = 0;
      title_type = NONE;
      greet_ok = 1;

      reset_marks();
   }
}

void new_queue(int queue_type)
{
   // flush the queue contents and start up a new data capture
   reset_queues(queue_type);
   log_loaded = 0;
   end_review(1);
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

long goto_event(u16 flags)
{
long val;

   // move plot to the next holdover event or time sequence error

   val = find_event(last_mouse_q, flags);
   if(val < 0) return val;

// val -= (PLOT_WIDTH/2)*view_interval;  // center point on screen
   if(plot_mag) val -= ((mouse_x * view_interval) / (long) plot_mag); // center point on mouse
   if(val > plot_q_count) val = plot_q_count - 1;
   if(val < 0) val = 0;
   last_mouse_q = val;
   zoom_review(val, 1);

   return val;
}


long next_q_point(long i, int stop_flag)
{
long j;
int wrap;

   // locate the next data point in the plot queue 
   // based upon the display view interval

   plot_column += plot_mag;
   if(stop_flag && (plot_column >= PLOT_WIDTH)) {  // we are at end of plot area
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



#ifdef BACK_FILTER
struct PLOT_Q filter_plot_q(long point)
{
struct PLOT_Q avg;
struct PLOT_Q q;
float count;
long i;
int j;

   // average the previous "filter_count" queue entries

   if(plot_q_count < filter_count) i = plot_q_count;
   else                            i = filter_count;

   point -= i;
   if(point < 0) point += plot_q_size;
   if(point >= plot_q_count) point = 0;

   avg = get_plot_q(point);
   count = 1.0F;

   while(--i) {
      ++point;
      while(point >= plot_q_size) point -= plot_q_size;

      q = get_plot_q(point);

      for(j=0; j<NUM_PLOTS; j++) {
         if(j != FFT) {
            avg.data[j] += q.data[j];
         }
      }

      count += 1.0F;
   }

   for(j=0; j<NUM_PLOTS; j++) {
      if(j != FFT) {
         avg.data[j] /= count;
      }
   }

   return avg;
}
#else
struct PLOT_Q filter_plot_q(long point)
{
struct PLOT_Q avg;
struct PLOT_Q q;
float count;
long i;
int j;

   // average the next "filter_count" queue entries
   avg = get_plot_q(point);
   if(point == plot_q_in) return avg;

   count = 1.0F;
   for(i=1; i<filter_count; i++) {
      if(++point == plot_q_in) break;
      while(point >= plot_q_size) point -= plot_q_size;

      q = get_plot_q(point);

      for(j=0; j<NUM_PLOTS; j++) {
         if(j != FFT) {
            avg.data[j] += q.data[j];
         }
      }

      count += 1.0F;
   }
      
   for(j=0; j<NUM_PLOTS; j++) {  // derived_plots OK
      if(j != FFT) {
         avg.data[j] /= count;
      }
   }

   return avg;
}
#endif


//
//
//   Screen format configuration stuff
//
//

void config_undersized()
{
   // setup for 640x480 screen
   if(user_font_size > 12) user_font_size = 12;

   HORIZ_MAJOR = 30;
   HORIZ_MINOR = 5;
   VERT_MAJOR = 20;
   VERT_MINOR = 4;
   COUNT_SCALE = (VERT_MAJOR/2);

   if(rcvr_type == NO_RCVR) {  // nnnnnn size of the analog watch area
      AZEL_SIZE = LLA_SIZE = (160); 
   }

   if(user_font_size && (user_font_size <= 8)) PLOT_ROW = SCREEN_HEIGHT-VERT_MAJOR*8;
   else PLOT_ROW = SCREEN_HEIGHT-VERT_MAJOR*6;
}

void config_small()
{
   // setup for 800x600 screen
   PLOT_ROW = (400+8);

   FILTER_ROW = 17;
   FILTER_COL = INFO_COL;

   COUNT_SCALE = (VERT_MAJOR/2);

   AZEL_SIZE = LLA_SIZE = (160);       // size of the az/el map area

   if(TEXT_HEIGHT >= 16) eofs = 0;
}


void config_azel()
{
   // how and where to draw the azimuth/elevation map (and analog watch)
   // assume azel plot will be in the adev table area
   last_track = last_used = 0L;
   share_active = 0;
   shared_item = 0;

   AZEL_ROW = ADEV_ROW*TEXT_HEIGHT;
   AZEL_COL = (ADEV_COL*TEXT_WIDTH);
   if(SCREEN_WIDTH >= ADEV_AZEL_THRESH) {  // adev tables and azel both fit
      if(adevs_active(1)) AZEL_COL += (TEXT_WIDTH*44);
      else                AZEL_COL += (TEXT_WIDTH*12);
      AZEL_ROW += TEXT_HEIGHT;
   }
   else if(SCREEN_WIDTH >= MEDIUM_WIDTH) {
      AZEL_COL += (TEXT_WIDTH*4);
      AZEL_ROW += TEXT_HEIGHT;
   }
   else if((rcvr_type == NO_RCVR) && (SCREEN_WIDTH <= 800)) {
      AZEL_COL -= 160; // nnnnnn  analog watch size
   }
   else {
      AZEL_COL += (TEXT_WIDTH*1);
      AZEL_ROW += TEXT_HEIGHT*2;
   }
   AZEL_SIZE = (SCREEN_WIDTH-AZEL_COL);

   if((SCREEN_WIDTH >= 1280) && (SCREEN_WIDTH < 1400)) AZEL_SIZE -= (TEXT_WIDTH*2);
   else if(small_font && (SCREEN_WIDTH >= MEDIUM_WIDTH)) AZEL_COL += TEXT_WIDTH;

   if((AZEL_ROW+AZEL_SIZE) >= (PLOT_ROW-128)) {  // az/el map is in plot area
      AZEL_SIZE = (PLOT_ROW-AZEL_ROW-128);
   }
   if(SCREEN_WIDTH > 800) AZEL_MARGIN = 8;
   else                   AZEL_MARGIN = 4;

   if(zoom_screen) {
      AZEL_SIZE = SCREEN_HEIGHT-(SCREEN_HEIGHT/20);
      AZEL_SIZE = (AZEL_SIZE/TEXT_HEIGHT)*TEXT_HEIGHT;
      AZEL_SIZE -= (TEXT_HEIGHT*2)*2;

      AZEL_ROW = (SCREEN_HEIGHT-AZEL_SIZE)/2;
      AZEL_ROW = (AZEL_ROW/TEXT_HEIGHT)*TEXT_HEIGHT;
      if(SCREEN_HEIGHT <= 600) AZEL_ROW -= TEXT_HEIGHT;
      else                     AZEL_ROW -= (TEXT_HEIGHT*2);

      AZEL_COL = (SCREEN_WIDTH-AZEL_SIZE)/2;
      AZEL_COL = (AZEL_COL/TEXT_WIDTH)*TEXT_WIDTH;
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
      WATCH_SIZE = AZEL_SIZE;
// if(WATCH_SIZE > (PLOT_ROW-TEXT_HEIGHT*4)) WATCH_SIZE = PLOT_ROW-TEXT_HEIGHT*4; // wwwww
   }

   if(zoom_screen == 'P');
   else if(zoom_screen) return;

   if(all_adevs || shared_plot) {  // all_adevs or share az/el space with normal plot area
      if(all_adevs && WIDE_SCREEN && (plot_watch == 0) && (plot_lla == 0)) {
         AZEL_COL = SCREEN_WIDTH-AZEL_SIZE;   // and azel goes in the corner
      }
      else {
         AZEL_SIZE = PLOT_HEIGHT;     // default size of the az/el map area
         if(AZEL_SIZE > 320) {        // it's just too big to look at
            AZEL_SIZE = 320;
            AZEL_ROW = ((PLOT_ROW+TEXT_HEIGHT-1)/TEXT_HEIGHT)*TEXT_HEIGHT;
         }
         else {
            AZEL_ROW = (SCREEN_HEIGHT-AZEL_SIZE);
         }
         AZEL_COL = (SCREEN_WIDTH-AZEL_SIZE);
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
      else if((plot_azel || plot_signals) && plot_watch && (plot_lla == 0) && (all_adevs == 0) && WIDE_SCREEN) {
         AZEL_SIZE = (SCREEN_WIDTH - AZEL_COL) / 2;
         if(AZEL_SIZE > 320) AZEL_SIZE = 320;
         WATCH_SIZE = AZEL_SIZE;
      }
      else if((plot_signals == 3) && (SCREEN_WIDTH >= 1280)) {
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

   if(zoom_screen) {
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
            if(ACLOCK_SIZE > 320) ACLOCK_SIZE = 320;
            aclock_x = (SCREEN_WIDTH-ACLOCK_SIZE/2-TEXT_WIDTH*2);   // and azel goes in the plot area
            aclock_y = (0+ACLOCK_SIZE/2+TEXT_HEIGHT*2);
         }
      }
   }
   else if((plot_azel || plot_signals) && plot_lla && WIDE_SCREEN) {
      goto watch_it;
   }
   else if((rcvr_type == NO_RCVR) && (SCREEN_WIDTH < 800)) { // nnnnnn
      goto watch_it;
   }
   else if((plot_azel || plot_signals) && (plot_lla == 0) && (SCREEN_HEIGHT >= 600)) {  // both azel and watch want to be on the sceen
      watch_it:
      ACLOCK_SIZE = WATCH_SIZE;               // watch goes in the adev area
      aclock_x = (WATCH_COL+ACLOCK_SIZE/2);   // and azel goes in the plot area
      aclock_y = (WATCH_ROW+ACLOCK_SIZE/2);
   }
}


void config_lla()
{
int lla_margin;

   // how and where to draw the lat/lon/altitude map
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
   else if(SCREEN_WIDTH > 1440) {  // both lla map and azel map will fit on big screens
      if(shared_plot) LLA_COL = SCREEN_WIDTH - AZEL_SIZE - (TEXT_WIDTH*2);
      else            LLA_COL = AZEL_COL + AZEL_SIZE + (TEXT_WIDTH*2);
   }
// else if(SCREEN_WIDTH >= 1280) {
   else if((SCREEN_WIDTH >= 1280) && (adev_period > 0)) {
      if(adevs_active(1)) LLA_COL += (TEXT_WIDTH*44); // adevs and lla will fit
      else                LLA_COL += (TEXT_WIDTH*12);
   }
   else if(SCREEN_WIDTH >= MEDIUM_WIDTH) {
      LLA_COL += (4*TEXT_WIDTH); 
   }
   else if(SCREEN_WIDTH >= 800) {
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
      LLA_SIZE = (SCREEN_HEIGHT-LLA_ROW-TEXT_HEIGHT*4-lla_margin);
   }
   else {
      LLA_SIZE = (SCREEN_WIDTH-LLA_COL);
      if(SCREEN_WIDTH >= 800) { // lla map maight be in the plot area
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

void config_text()
{
int i;

   // text drawing stuff
   PLOT_TEXT_COL = (PLOT_COL / TEXT_WIDTH);
   PLOT_TEXT_ROW = (PLOT_ROW / TEXT_HEIGHT);

   MOUSE_COL = (PLOT_TEXT_COL)+2;
   MOUSE_ROW = ((PLOT_TEXT_ROW)-6-eofs);
   MOUSE_ROW -= ((TEXT_Y_MARGIN+TEXT_HEIGHT-1)/TEXT_HEIGHT);
   if((TEXT_HEIGHT == 16) && (SCREEN_HEIGHT <= 600)) --MOUSE_ROW;
   if(MOUSE_ROW < 0) MOUSE_ROW = 0;

   TEXT_COLS = ((SCREEN_WIDTH+TEXT_WIDTH-1)/TEXT_WIDTH);
   if(TEXT_COLS >= (int)sizeof(blanks)) TEXT_COLS = sizeof(blanks)-1;
   TEXT_ROWS = ((SCREEN_HEIGHT+TEXT_HEIGHT-1)/TEXT_HEIGHT);
   if(text_mode) MOUSE_ROW = TEXT_ROWS;

   EDIT_ROW = PLOT_TEXT_ROW+2;   // where to put the string input dialog
   EDIT_COL = PLOT_TEXT_COL;

   if(text_mode) {
      EDIT_ROW = EDIT_COL = 0;
      PLOT_ROW += (TEXT_HEIGHT*3);
      PLOT_TEXT_ROW = 100;
   }

   for(i=0; i<TEXT_COLS; i++) blanks[i] = ' ';
   blanks[TEXT_COLS] = 0;
}

void config_zoom()
{
   if(zoom_screen == 0) return;
   if(plot_signals) return;
   if(plot_watch) return;
   if(plot_digital_clock) return;
   if(plot_azel) return;
   if(plot_lla && (zoom_screen == 'L')) return; 

   if(text_mode) zoom_screen = 0;
   if(zoom_screen == 'L') zoom_screen = 0;
//zzzzzzz   zoom_lla = 0;
}

void config_screen(int why)
{
int i;

   // setup variables related to the video screen size

   VERT_MAJOR = 30;    // the graph axes tick mark spacing (in pixels)
   VERT_MINOR  = 6;    // VERT_MAJOR/VERT_MINOR should be 5

   HORIZ_MAJOR = 60;
   HORIZ_MINOR = 10;

   INFO_COL = 65;
   if(text_mode) INFO_COL -= 1;
   else if(SCREEN_WIDTH <= MIN_WIDTH) INFO_COL -= 2;
   else if(SCREEN_WIDTH <= 800) INFO_COL -= 1;
   else if(small_font == 1) INFO_COL -= 1;

   FILTER_ROW = 18;
   FILTER_COL = INFO_COL;

   AZEL_SIZE = LLA_SIZE = (320);       // size of the az/el map area
   eofs = 1;

   PLOT_COL = 0;

   if(screen_type == 'u') {  // undersized, very small screen
      GRAPH_MODE = 18;
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
   else if(screen_type == 's') {  // small screen
      GRAPH_MODE  = 258;
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
   else if(screen_type == 'n') {  // netbook screen
      custom_width = 1000;
      custom_height = 540;
      goto customize;
   }
   else if(screen_type == 'l') { // large screen
      GRAPH_MODE  = 262;
      SCREEN_WIDTH = 1280;
      SCREEN_HEIGHT = 1024; // 800, 900, 960, 1024

      PLOT_ROW = (PLOT_TOP+VERT_MAJOR*2);

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
   else if(screen_type == 'v') { // very large screen
      GRAPH_MODE  = 327;        //!!!!!
      SCREEN_WIDTH = 1400;  //1440;
      SCREEN_HEIGHT = 1050; // 900; 

      PLOT_ROW = (PLOT_TOP+VERT_MAJOR*2);

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
      GRAPH_MODE  = 304;     // 325
      SCREEN_WIDTH = 1680;  
      SCREEN_HEIGHT = 1050; 

      PLOT_ROW = (PLOT_TOP+VERT_MAJOR*2);

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
   else if(screen_type == 'h') { // huge screen
      GRAPH_MODE  = 319;   
      SCREEN_WIDTH = 1920;
      SCREEN_HEIGHT = 1080;

      PLOT_ROW = 576;

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
   else if(screen_type == 'z') { // really huge screen
      GRAPH_MODE  = 338;   
      SCREEN_WIDTH = 2048;
      SCREEN_HEIGHT = 1536;

      PLOT_ROW = (576);

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
   else if(screen_type == 'c') { // custom screen
      customize:
      screen_type = 'c';
      if(custom_width <= 0) custom_width = SCREEN_WIDTH;
      if(custom_height <= 0) custom_height = SCREEN_HEIGHT;

      if(custom_width <= MIN_WIDTH) custom_width = MIN_WIDTH;
      if(custom_height <= MIN_HEIGHT) custom_height = MIN_HEIGHT;

      SCREEN_WIDTH = custom_width;
      SCREEN_HEIGHT = custom_height;

      if     (SCREEN_HEIGHT > 1050) PLOT_ROW = 576;
      else if(SCREEN_HEIGHT > 960)  PLOT_ROW = PLOT_TOP+(2*VERT_MAJOR);
      else if(SCREEN_HEIGHT >= MEDIUM_HEIGHT) PLOT_ROW = 468;
      else if(SCREEN_HEIGHT >= 600) config_small();
      else                          config_undersized();

      if(day_plot) {
         HORIZ_MAJOR = SCREEN_WIDTH / day_plot;
         HORIZ_MAJOR /= 4;
         HORIZ_MAJOR *= 4;
         HORIZ_MINOR = (HORIZ_MAJOR / 4);
      }
      else day_plot = 0;
////  day_plot = 0;
   }
   else if(screen_type == 't') {  // text only mode
      // In DOS we do this mode as a true text mode
      // In Windows, this is actually a 640x480 graphics mode
      GRAPH_MODE = 18;
      SCREEN_WIDTH = 640;
      SCREEN_HEIGHT = 480;
      PLOT_ROW = (SCREEN_HEIGHT/TEXT_HEIGHT)*TEXT_HEIGHT;  // -VERT_MAJOR*2;
      PLOT_COL = 0;
      day_plot = 0;
   }
   else {   // normal (1024x768) screen
      GRAPH_MODE  = 260;
      SCREEN_WIDTH = 1024;
      SCREEN_HEIGHT = 768;

      PLOT_ROW = 468;
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

//sprintf(plot_title, "PLOT_ROW:%d  VM:%d  texth:%d", PLOT_ROW, VERT_MAJOR, TEXT_HEIGHT);
   if(zoom_screen == 'P') {
      PLOT_ROW = TEXT_HEIGHT*8;
   }
   else if(1 && luxor) {
//      PLOT_ROW = SCREEN_HEIGHT / 2;
//if(PLOT_ROW > TEXT_HEIGHT*26) PLOT_ROW = TEXT_HEIGHT*26;
//      PLOT_ROW /= TEXT_HEIGHT;
//      PLOT_ROW *= TEXT_HEIGHT;
      if(plot_digital_clock == 0) {
         PLOT_ROW -= VERT_MAJOR*2;
      }
   }
//   else if(1 && (SCREEN_HEIGHT > 600) && (max_sats > 8) && plot_digital_clock) {
   else if((all_adevs == 0) && (SCREEN_HEIGHT > 600) && (max_sats > 8) && plot_digital_clock) {
      i = (max_sats-8) * TEXT_HEIGHT;
if(sat_cols > 1) i = (sat_rows-8) * TEXT_HEIGHT;
      i += ((VERT_MAJOR*2) - 1);
      i /= (VERT_MAJOR*2);  // makes graph prettier to have things a multiple of VERT_MAJOR*2
      i *= (VERT_MAJOR*2);

      PLOT_ROW = PLOT_ROW + i;
   }

   if(user_video_mode) {  // user specified the video mode to use
      GRAPH_MODE = user_video_mode;
   }

   if(small_sat_count) {  // compress sat count to VERT_MINOR ticks per sat
      COUNT_SCALE = VERT_MINOR; 
   }
   else {                 // expand sat count to VERT_MAJOR ticks per sat
      COUNT_SCALE = VERT_MAJOR;
   }

   if(no_plots) PLOT_ROW = SCREEN_HEIGHT;

   // graphs look best if PLOT_HEIGHT is a multiple of (2*VERT_MAJOR)
   // but small screens don't have room to waste
   PLOT_HEIGHT = (SCREEN_HEIGHT-PLOT_ROW);
   PLOT_HEIGHT /= (VERT_MAJOR*2);  // makes graph prettier to have things a multiple of VERT_MAJOR*2
   PLOT_HEIGHT *= (VERT_MAJOR*2);

   PLOT_CENTER = (PLOT_HEIGHT/2-1);

   PLOT_WIDTH = (SCREEN_WIDTH/HORIZ_MAJOR) * HORIZ_MAJOR;
   if(day_plot) {
      PLOT_WIDTH = HORIZ_MAJOR * day_plot;
      if(interval_set) {
         queue_interval = 3600L / HORIZ_MAJOR;
         interval_set = 0;
      }
   }

   // when graph fills, scroll it left this many pixels
   if(continuous_scroll) PLOT_SCROLL = 1;  // live update mode for fast processors
   else                  PLOT_SCROLL = (HORIZ_MAJOR*2);

   if(rcvr_type == NO_RCVR) ;
   else if(no_plots) ;
   else if(SMALL_SCREEN) {  // undersized screen
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
   else                               big_plot = 0;

   screen_configed = 1;

   if(no_redraw == 0) need_redraw = why;
}


//
//
//   Data plotting stuff
//
//

float scale_temp(float t)
{
   // convert degrees C into user specified measurement system
   if     (DEG_SCALE == 'C') ;
   else if(DEG_SCALE == 'F') t = (t * 1.8F) + 32.0F;
   else if(DEG_SCALE == 'R') t = (t + 273.15F) * 1.8F;
   else if(DEG_SCALE == 'K') t = t + 273.15F;
   else if(DEG_SCALE == 'D') t = (100.0F-t) * 1.5F;
   else if(DEG_SCALE == 'N') t = t * 0.33F;
   else if(DEG_SCALE == 'O') t = (t * (21.0F / 40.0F)) + 7.5F;
   else if(DEG_SCALE == 'E') t = (t * (21.0F / 40.0F)) + 7.5F;

   return t;
}

char *fmt_temp(float temp)
{
static char buf[SLEN];
float val;

   val = scale_temp(temp);
   if     (round_temp == 1) sprintf(buf, "%.1f %c%c", val, DEGREES, DEG_SCALE);
   else if(round_temp == 2) sprintf(buf, "%.2f %c%c", val, DEGREES, DEG_SCALE);
   else if(round_temp == 3) sprintf(buf, "%.3f %c%c", val, DEGREES, DEG_SCALE);
   else if(round_temp == 4) sprintf(buf, "%.4f %c%c", val, DEGREES, DEG_SCALE);
   else if(round_temp == 5) sprintf(buf, "%.5f %c%c", val, DEGREES, DEG_SCALE);
   else                     sprintf(buf, "%.6f %c%c", val, DEGREES, DEG_SCALE);
   return &buf[0];
}

char *fmt_secs(float val)
{
static char buf[SLEN];
long secs;

   secs = (int) (val + 0.50);
   if(nav_rate != 1.0) {
      sprintf(buf, "[%.1fs]", val);
   }
   else if(secs < 60L) {
      sprintf(buf, "[%lds]", secs);
   }
   else if(secs < 60L*60L) {
      sprintf(buf, "[%ldm%lds]", secs/60L, secs%60L);
   }
   else {
      sprintf(buf, "[%ldh%ldm%lds]", (secs/(60L*60L)), (secs/60L)%60L, secs%60L);
   }

   return &buf[0];
}


void fmt_osc_val(int stat, char *id, float val)
{
   if(luxor) {
      if((val >= 100000.0F) || (val <= (-10000.0F))) {
         if(stat) sprintf(out, "%s:%8.3f%s", id, val, plot[OSC].units);
         else     sprintf(out, "%s:%7.3f%s", id, val, plot[OSC].units);
      }
      else if((val >= 10000.0F) || (val <= (-1000.0F))) {
         if(stat) sprintf(out, "%s:%8.3f%s", id, val, plot[OSC].units);
         else     sprintf(out, "%s:%7.3f%s", id, val, plot[OSC].units);
      }
      else if((val >= 1000.0F) || (val <= (-100.0F))) {
         if(stat) sprintf(out, "%s:%8.3f%s", id, val, plot[OSC].units);
         else     sprintf(out, "%s:%7.3f%s", id, val, plot[OSC].units);
      }
      else {
         if(stat) sprintf(out, "%s:%8.3f%s", id, val, plot[OSC].units);
         else     sprintf(out, "%s:%7.3f%s", id, val, plot[OSC].units);
      }
   }
   else if(rcvr_type == ZODIAC_RCVR) {
      if((val >= 10000.0F) || (val <= (-10000.0F))) {
         sprintf(out, "%s:%9.1f%s", id, val, ppt_string);
      }
      else if((val >= 10000.0F) || (val <= (-1000.0F))) {
         sprintf(out, "%s:%9.2f%s", id, val, ppt_string);
      }
      else if((val >= 1000.0F) || (val <= (-100.0F))) {
         sprintf(out, "%s:%9.3f%s", id, val, ppt_string);
      }
      else {
         sprintf(out, "%s:%9.4f%s", id, val, ppt_string);
      }
   }
   else if(TIMING_RCVR) {
      if((val >= 10000.0F) || (val <= (-10000.0F))) {
         sprintf(out, "%s:%11.3f%s", id, val, ppt_string);
      }
      else if((val >= 10000.0F) || (val <= (-1000.0F))) {
         sprintf(out, "%s:%11.4f%s", id, val, ppt_string);
      }
      else if((val >= 1000.0F) || (val <= (-100.0F))) {
         sprintf(out, "%s:%11.5f%s", id, val, ppt_string);
      }
      else {
         sprintf(out, "%s:%11.6f%s", id, val, ppt_string);
      }
   }
   else {
      val *= 1000.0F;

      if((val >= 100000.0F) || (val <= (-10000.0F))) {
         sprintf(out, "%s:%10.3f%s", id, val, ppt_string);
      }
      else if((val >= 10000.0F) || (val <= (-1000.0F))) {
         sprintf(out, "%s:%10.4f%s", id, val, ppt_string);
      }
      else if((val >= 1000.0F) || (val <= (-100.0F))) {
         sprintf(out, "%s:%10.5f%s", id, val, ppt_string);
      }
      else {
         sprintf(out, "%s:%10.6f%s", id, val, ppt_string);
      }
   }
}


float get_stat_val(int id)
{
float val;

   // get the plot statistic value to show for the plot

   if(stat_count == 0.0F) return 0.0F;

   if(plot[id].show_stat == RMS) {
      stat_id = "rms";
      return sqrt(plot[id].sum_yy/stat_count);
   }
   else if(plot[id].show_stat == AVG) {
      stat_id = "avg";
      return plot[id].sum_y/stat_count;
   }
   else if(plot[id].show_stat == SDEV) {
      stat_id = "sdv";
      val = ((plot[id].sum_yy/stat_count) - ((plot[id].sum_y/stat_count)*(plot[id].sum_y/stat_count)) );
//    val = ( plot[id].sum_yy - ((plot[id].sum_y*plot[id].sum_y)/stat_count) ) / stat_count;
      if(val < 0.0F) val = 0.0F - val;
      return sqrt(val);
   }
   else if(plot[id].show_stat == VAR) {
      stat_id = "var";
      val = ( (plot[id].sum_yy/stat_count) - ((plot[id].sum_y/stat_count)*(plot[id].sum_y/stat_count)) );
//    val = ( plot[id].sum_yy - ((plot[id].sum_y*plot[id].sum_y)/stat_count) ) / stat_count;
      return val;
   }
   else if(plot[id].show_stat == SHOW_MIN) {
      stat_id = "min";
      return plot[id].min_val;
   }
   else if(plot[id].show_stat == SHOW_MAX) {
      stat_id = "max";
      return plot[id].max_val;
   }
   else if(plot[id].show_stat == SHOW_SPAN) {
      stat_id = "span";
      return plot[id].max_val - plot[id].min_val;
   }
   else {
      stat_id = "???";
      return 0.0F;
   }
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
      if(extra_plots) plot_adev_data = 0;
      return 1;
   }
   return 0;
}

void label_plots()
{
COORD row, col;
COORD fft_col;
struct PLOT_Q q;
char *t;
long i;
int id;
char c;
float val;
int len;
int max_len;
int month, day, year;
int hours, minutes, seconds;
double frac;
double old_utc, old_gps;
float show_time;

   // label the plot with scale factors, etc
   if(text_mode) return;
   if(no_plots) return;
   if(rcvr_type == NO_RCVR) return;
   if(zoom_screen == 'P') ;
   else if(zoom_screen) return;
   if(first_key && SMALL_SCREEN) return;


   fft_col = MOUSE_COL+40;
   no_x_margin = no_y_margin = 1;
   vidstr(MOUSE_ROW+2, 0, WHITE, &blanks[0]);  // erase the plot info area
   vidstr(MOUSE_ROW+3, 0, WHITE, &blanks[0]);
   vidstr(MOUSE_ROW+4, 0, WHITE, &blanks[0]);
   vidstr(MOUSE_ROW+5, 0, WHITE, &blanks[0]);
   vidstr(MOUSE_ROW+6, 0, WHITE, &blanks[0]);
   vidstr(MOUSE_ROW+7, 0, WHITE, &blanks[0]);

   t = "";
   row = MOUSE_ROW+7;
   col = PLOT_TEXT_COL+0;

   if(review_mode) {
      if(review_home)  sprintf(out, "  ");
      else             sprintf(out, "%c ", LEFT_ARROW); 
   }
   else {
      if(((plot_q_count*(long)plot_mag)/view_interval) >= PLOT_WIDTH) sprintf(out, "%c ", LEFT_ARROW);
      else sprintf(out, "  ");
   }
   vidstr(row, col, WHITE, out);
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
   col = PLOT_TEXT_COL+2;
   if(mouse_plot_valid) {
      if(filter_count) q = filter_plot_q(last_mouse_q);
      else             q = get_plot_q(last_mouse_q);

      jd_utc = q.q_jd;
      jd_gps = jd_utc + jtime(0,0,utc_offset,0.0);
      adjust_tz(1);  // tweak pri_ time for time zone

      if(top_line) {
         show_time = (float) (view_interval*queue_interval*PLOT_WIDTH);  //seconds per screen
         show_time /= plot_mag;
         show_time /= 60.0F;  // minutes/screen
         show_time /= (((float) PLOT_WIDTH) / (float) HORIZ_MAJOR);
         show_time /= nav_rate;
         sprintf(out, "Cursor time: %02d:%02d:%02d.%03d %s  %s     View: %.1f min/div    ",
             pri_hours,pri_minutes,pri_seconds, (int) ((pri_frac*1000.0)+0.5),
             time_zone_set?tz_string:((q.sat_flags & UTC_TIME) ? "UTC":"GPS"), 
             fmt_date(0), show_time 
         );
      }
      else if(nav_rate && plot_mag) {
//         sprintf(out, "Cursor time: %02d:%02d:%02d %s  %s           ",
//             pri_hours,pri_minutes,pri_seconds, 
//             time_zone_set?tz_string:((q.sat_flags & UTC_TIME) ? "UTC":"GPS"), 
//             fmt_date(0) 
//         );
         val = ((float) (view_interval * mouse_x) / nav_rate) / (float) plot_mag;
         sprintf(out, "Cursor time: %02d:%02d:%02d.%03d %s  %s  %s       ",
             pri_hours,pri_minutes,pri_seconds,  (int) ((pri_frac*1000.0)+0.5), 
             time_zone_set?tz_string:((q.sat_flags & UTC_TIME) ? "UTC":"GPS"), 
             fmt_date(0), 
             fmt_secs(val)
         );
      }
      no_x_margin = no_y_margin = 1;
      vidstr(MOUSE_ROW+2, 0, WHITE, &blanks[0]);  // erase the plot info area
      vidstr(MOUSE_ROW+2, PLOT_TEXT_COL+2, MOUSE_COLOR, out);
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

   vidstr(MOUSE_ROW+3, 0, WHITE, &blanks[0]);
   for(id=0; id<NUM_PLOTS+DERIVED_PLOTS; id++) {  // show the plot values
      if(plot[id].show_plot == 0) continue;
      if(id == FFT) fft_col = col;

      max_len = 0;
      val = plot[id].scale_factor * plot[id].invert_plot;


      // show the plot scale factors
      if(auto_scale && (plot[id].user_scale == 0)) c = '~';
      else c = '=';
      if(id == TEMP) {
         if(luxor) {
            if(plot[id].scale_factor >= 100.0F) {
               sprintf(out, "%s%c(%ld %c%c/div)",  
                            plot[id].plot_id, c, (long) (val+0.5F), DEGREES,DEG_SCALE);
            }
            else if(plot[id].scale_factor < 1.0F) {
               sprintf(out, "%s%c(%.2f %c%c/div)", 
                             plot[id].plot_id, c, val, DEGREES,DEG_SCALE);
            }
            else {
               sprintf(out, "%s%c(%.1f %c%c/div)", 
                             plot[id].plot_id, c, val, DEGREES,DEG_SCALE);
            }
         }
         else if((rcvr_type == UCCM_RCVR) && (scpi_type == 'P')) {
            val /= 1000.0;
            if(plot[id].scale_factor >= 100.0F) {
               sprintf(out, "%s%c(%ld %s/div)",  
                            "TCOR", c, (long) (val+0.5F), ppt_string);
            }
            else if(plot[id].scale_factor < 1.0F) {
               sprintf(out, "%s%c(%.2f %s/div)", 
                             "TCOR", c, val, ppt_string);
            }
            else {
               sprintf(out, "%s%c(%.1f %s/div)", 
                             "TCOR", c, val, ppt_string);
            }
         }
         else {
            if(plot[id].scale_factor >= 100.0F) {
               sprintf(out, "%s%c(%ld m%c%c/div)",  
                            plot[id].plot_id, c, (long) (val+0.5F), DEGREES,DEG_SCALE);
            }
            else if(plot[id].scale_factor < 1.0F) {
               sprintf(out, "%s%c(%.2f m%c%c/div)", 
                             plot[id].plot_id, c, val, DEGREES,DEG_SCALE);
            }
            else {
               sprintf(out, "%s%c(%.1f m%c%c/div)", 
                             plot[id].plot_id, c, val, DEGREES,DEG_SCALE);
            }
         }
      }
      else if(luxor && (id == TC2)) {
         if(plot[id].scale_factor >= 100.0F) {
            sprintf(out, "%s%c(%ld %c%c/div)",  
                         plot[id].plot_id, c, (long) (val+0.5F), DEGREES,DEG_SCALE);
         }
         else if(plot[id].scale_factor < 1.0F) {
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
            if(plot[id].scale_factor >= 100.0F) {
               sprintf(out, "%s%c(%ld%s/div)", 
                            plot[id].plot_id, c, (long) (val+0.5F), plot[id].units);
            }
            else if(plot[id].scale_factor < 1.0F) {
               sprintf(out, "%s%c(%.3f%s/div)", 
                             plot[id].plot_id, c, val, plot[id].units);
            }
            else {
               sprintf(out, "%s%c(%.2f%s/div)", 
                             plot[id].plot_id, c, val, plot[id].units);
            }
         }
         else if(rcvr_type == ZODIAC_RCVR) {
            if(plot[id].scale_factor >= 100.0F) {
               sprintf(out, "%s%c(%ld %s/div)", 
                            plot[id].plot_id, c, (long) (val+0.5F), plot[id].units);
            }
            else if(plot[id].scale_factor < 1.0F) {
               sprintf(out, "%s%c(%.2f %s/div)", 
                             plot[id].plot_id, c, val, plot[id].units);
            }
            else {
               sprintf(out, "%s%c(%.1f %s/div)", 
                             plot[id].plot_id, c, val, plot[id].units);
            }
         }
         else {
            if(plot[id].scale_factor >= 100.0F) {
               sprintf(out, "%s%c(%ld%s/div)", 
                            plot[id].plot_id, c, (long) (val+0.5F), plot[id].units);
            }
            else if(plot[id].scale_factor < 1.0F) {
               sprintf(out, "%s%c(%.2f%s/div)", 
                             plot[id].plot_id, c, val, plot[id].units);
            }
            else {
               sprintf(out, "%s%c(%.1f%s/div)", 
                             plot[id].plot_id, c, val, plot[id].units);
            }
         }
      }
      else if((id == DAC) && ((rcvr_type == SCPI_RCVR) || (rcvr_type == UCCM_RCVR))) {
         if(plot[id].scale_factor < 0.01) {
            sprintf(out, "%s%c(%.5f %s/div)", 
                          plot[id].plot_id, c, val, plot[id].units);
         }
         else if(plot[id].scale_factor < 1.0F) {
            sprintf(out, "%s%c(%.3f %s/div)", 
                          plot[id].plot_id, c, val, plot[id].units);
         }
         else {
            sprintf(out, "%s%c(%.1f %s/div)", 
                          plot[id].plot_id, c, val, plot[id].units);
         }
      }
      else {
         if(luxor) {
            if(plot[id].scale_factor >= 100.0F) {
               sprintf(out, "%s%c(%ld %s/div)", 
                            plot[id].plot_id, c, (long) (val+0.5F), plot[id].units);
            }
            else if(plot[id].scale_factor < 1.0F) {
               sprintf(out, "%s%c(%.3f %s/div)", 
                             plot[id].plot_id, c, val, plot[id].units);
            }
            else {
               sprintf(out, "%s%c(%.2f %s/div)", 
                             plot[id].plot_id, c, val, plot[id].units);
            }
         }
         else if((id == ONE) || (id == TWO) || (id == THREE)) {
            sprintf(out, "%s%c(%.5f %s/div)", 
                          plot[id].plot_id, c, val, plot[id].units);
         }
         else {
             if(plot[id].scale_factor >= 100.0F) {
                sprintf(out, "%s%c(%ld %s/div)", 
                             plot[id].plot_id, c, (long) (val+0.5F), plot[id].units);
             }
             else if(plot[id].scale_factor < 1.0F) {
                sprintf(out, "%s%c(%.2f %s/div)", 
                              plot[id].plot_id, c, val, plot[id].units);
             }
             else {
                sprintf(out, "%s%c(%.1f %s/div)", 
                              plot[id].plot_id, c, val, plot[id].units);
             }
         }
      }
      no_x_margin = no_y_margin = 1;
      vidstr(MOUSE_ROW+7, col, plot[id].plot_color, out);
      no_x_margin = no_y_margin = 0;
      len = strlen(out);
      if(len > max_len) max_len = len;



      // now show the plot zero reference line value
      val = plot[id].plot_center;
      if(val == NEED_CENTER) val = 0.0F;
      if(auto_center && plot[id].float_center) c = '~';
      else                                     c = '=';
      if(id == OSC)  {
         if(luxor)      sprintf(out, "ref%c<%.3f%s>", c, val, plot[id].units); 
         else if(rcvr_type == ZODIAC_RCVR) sprintf(out, "ref%c<%.1f %s>", c, val, plot[id].units); 
         else if(TIMING_RCVR) sprintf(out, "ref%c<%.1f%s>", c, val, plot[id].units); 
         else sprintf(out, "ref%c<%.1f%s>", c, val*1000.0F, plot[id].units); 
      }
      else if(id == PPS) {
         if(res_t) sprintf(out, "ref%c<%.1f %s>", c, val/1000.0F, plot[id].units);
         else if(TIMING_RCVR) sprintf(out, "ref%c<%.1f %s>", c, val/1000.0F, plot[id].units);
         else sprintf(out, "ref%c<%.1f %s>", c, val, plot[id].units);
      }
      else if(id == DAC)  {
         if(luxor)      sprintf(out, "ref%c<%.3f%s>", c, val, plot[DAC].units);
         else if(res_t) sprintf(out, "ref%c<%.6f ns>", c, val);
         else if(TIMING_RCVR) sprintf(out, "ref%c<%.6f ns>", c, val);
         else if(rcvr_type == TSIP_RCVR) sprintf(out, "ref%c<%.6f %s>", c, val, "V");
         else sprintf(out, "ref%c<%.6f %s>", c, val, plot[id].units);
      }
      else if(id == TEMP) {
         if((rcvr_type == UCCM_RCVR) && (scpi_type == 'P')) {
            sprintf(out, " ref%c<%.3f %s>", c, val, ppt_string);
         }
         else sprintf(out, " ref%c<%.3f %c%c>", c, val, DEGREES,DEG_SCALE);
      }
      else if(luxor && (id == TC2)) sprintf(out, " ref%c<%.3f %c%c>", c, val, DEGREES,DEG_SCALE);
      else if((id == ONE) || (id == TWO)) {
         if(plot_loc) sprintf(out, "ref%c<%.7f %s>", c, val, plot[id].units);
         else         sprintf(out, "ref%c<private %s>", c, plot[id].units);
      }
      else if(id == THREE) {
         sprintf(out, "ref%c<%.3f %s>", c, val, plot[id].units);
      }
      else sprintf(out, "ref%c<%.1f %s>", c, val, plot[id].units);
      no_x_margin = no_y_margin = 1;
      vidstr(MOUSE_ROW+6, col, plot[id].plot_color, out);
      no_x_margin = no_y_margin = 0;
      len = strlen(out);
      if(len > max_len) max_len = len;


      // now we show the statistics values
      if(stat_count == 0.0) goto no_stats;
      if(all_adevs && (mixed_adevs == 0)) goto no_stats;
      no_x_margin = 1;

      if(id == OSC) {
         val = get_stat_val(OSC);
         fmt_osc_val(1, stat_id, val);
      }
      else if(id == PPS) {
         val = get_stat_val(PPS);
         if(luxor) {
            val *= (float) lux_scale;
            if(val <= (-10000.0F))     sprintf(out, "%s:  %.3f %s", stat_id, val, plot[PPS].units);
            else if(val <= (-1000.0F)) sprintf(out, "%s:  %.3f %s", stat_id, val, plot[PPS].units); 
            else if(val >= 10000.0F)   sprintf(out, "%s:  %.3f %s", stat_id, val, plot[PPS].units); 
            else                       sprintf(out, "%s:  %.3f %s", stat_id, val, plot[PPS].units); 
         }
         else if(rcvr_type == UBX_RCVR) {
            if(val <= (-10000.0F))     sprintf(out, "%s: %.4f ns", stat_id, val);
            else if(val <= (-1000.0F)) sprintf(out, "%s: %.5f ns", stat_id, val);
            else if(val >= 10000.0F)   sprintf(out, "%s: %.4f ns", stat_id, val);
            else                       sprintf(out, "%s: %.6f ns", stat_id, val);
         }
         else if(rcvr_type == ZODIAC_RCVR) {
            if(val <= (-10000.0F))     sprintf(out, "%s: %.4f ns", stat_id, val);
            else if(val <= (-1000.0F)) sprintf(out, "%s: %.5f ns", stat_id, val);
            else if(val >= 10000.0F)   sprintf(out, "%s: %.4f ns", stat_id, val);
            else                       sprintf(out, "%s: %.6f ns", stat_id, val);
         }
         else if(rcvr_type == NVS_RCVR) { // 11.?f
            if(val <= (-10000.0F))     sprintf(out, "%s: %.4f %s", stat_id, val, plot[id].units);
            else if(val <= (-1000.0F)) sprintf(out, "%s: %.5f %s", stat_id, val, plot[id].units);
            else if(val >= 10000.0F)   sprintf(out, "%s: %.4f %s", stat_id, val, plot[id].units);
            else                       sprintf(out, "%s: %.6f %s", stat_id, val, plot[id].units);
         }
         else if(TIMING_RCVR) {  // 11.?f
            val /= 1000.0F;
            if(val <= (-10000.0F))     sprintf(out, "%s: %.4f us", stat_id, val);
            else if(val <= (-1000.0F)) sprintf(out, "%s: %.5f us", stat_id, val);
            else if(val >= 10000.0F)   sprintf(out, "%s: %.4f us", stat_id, val);
            else                       sprintf(out, "%s: %.6f us", stat_id, val);
         }
         else { // 11.?f
            if(val <= (-10000.0F))     sprintf(out, "%s: %.4f %s", stat_id, val, plot[id].units);
            else if(val <= (-1000.0F)) sprintf(out, "%s: %.5f %s", stat_id, val, plot[id].units);
            else if(val >= 10000.0F)   sprintf(out, "%s: %.4f %s", stat_id, val, plot[id].units);
            else                       sprintf(out, "%s: %.6f %s", stat_id, val, plot[id].units);
         }
      }
      else if(id == TEMP) {
         val = get_stat_val(id);
         if((rcvr_type == UCCM_RCVR) && (scpi_type == 'P')) {
            sprintf(out, "%s: %.6f %s", stat_id, val, ppt_string);
         }
         else {
            sprintf(out, "%s:  %s", stat_id, fmt_temp(val));
         }
      }
      else if(luxor && (id == TC2)) {
         val = get_stat_val(id);
         sprintf(out, "%s:  %s", stat_id, fmt_temp(val));
      }
      else if(id == DAC) {
         val = get_stat_val(id);
         if(luxor) {
            sprintf(out, "%s:  %.3f %s", stat_id, val, plot[id].units);
         }
         else if(TIMING_RCVR) {
            sprintf(out, "%s: %.6f ns", stat_id, val);
         }
         else if(rcvr_type == TSIP_RCVR) {
            sprintf(out, "%s: %.6f %s", stat_id, val, "V");
         }
         else {
            sprintf(out, "%s: %.6f %s", stat_id, val, plot[id].units);
         }
      }
      else if((id == ONE) || (id == TWO)) {
         val = get_stat_val(id);
         if(plot_loc) sprintf(out, "%s: %.8f %s", stat_id, val, plot[id].units);
         else         sprintf(out, "%s: (private) %s", stat_id, plot[id].units);
      }
      else if(id == THREE) {
         val = get_stat_val(id);
         sprintf(out, "%s: %.3f %s", stat_id, val, plot[id].units);
      }
      else {
         val = get_stat_val(id);
         if(luxor) {
            if(id == LUX2) val *= (float) lum_scale;
            sprintf(out, "%s: %.3f %s", stat_id, val, plot[id].units);
         }
         else {
            sprintf(out, "%s: %.6f %s", stat_id, val, plot[id].units);
         }
      }

      if(plot[id].show_plot && plot[id].show_stat) {
         no_x_margin = no_y_margin = 1;
         vidstr(MOUSE_ROW+4, col, plot[id].plot_color, out);
         no_x_margin = no_y_margin = 0;
         len = strlen(out);
         if(len > max_len) max_len = len;
      }




      // now display the cursor info
      no_stats:
      if(mouse_plot_valid == 0) goto no_mouse_info;
      if(queue_interval <= 0) goto no_mouse_info;

      if(id == DAC) {
         if(luxor) {
            sprintf(out, "%s: %.3f V", plot[id].plot_id, q.data[id]/(float) queue_interval);
         }
         else if(TIMING_RCVR) {
            sprintf(out, "%s: %.6f ns", plot[id].plot_id, q.data[id]/(float) queue_interval);
         }
         else if(rcvr_type == TSIP_RCVR) {
            sprintf(out, "%s: %.6f %s", plot[id].plot_id, q.data[id]/(float) queue_interval, "V");
         }
         else {
            sprintf(out, "%s: %.6f %s", plot[id].plot_id, q.data[id]/(float) queue_interval, plot[id].units);
         }
      }
      else if(id == TEMP) {
         if((rcvr_type == UCCM_RCVR) && (scpi_type == 'P')) {
            sprintf(out, "%s: %.6f %s", "TCOR", 
                   q.data[id]/(float) queue_interval, ppt_string);
         }
         else {
            sprintf(out, "%s: %s", plot[id].plot_id, 
                   fmt_temp(q.data[id]/(float) queue_interval));
         }
      }
      else if(luxor && (id == TC2)) {
         sprintf(out, "%s: %s", plot[id].plot_id, 
                fmt_temp(q.data[id]/(float) queue_interval));
      }
      else if(id == OSC) {
         val = (float) (q.data[id]/(OFS_SIZE) queue_interval);
         fmt_osc_val(0, plot[id].plot_id, val);
      }
      else if(id == PPS) {
         val = (float) (q.data[id]/(OFS_SIZE) queue_interval);
         if(luxor) {
            val *= (float) lux_scale;
            if(val <= (-10000.0F))     sprintf(out, "%s: %.3f %s", plot[id].plot_id,val, plot[id].units);
            else if(val <= (-1000.0F)) sprintf(out, "%s: %.3f %s", plot[id].plot_id,val, plot[id].units);
            else if(val >= 10000.0F)   sprintf(out, "%s: %.3f %s", plot[id].plot_id,val, plot[id].units);
            else                       sprintf(out, "%s: %.3f %s", plot[id].plot_id,val, plot[id].units);
         }
         else if(rcvr_type == NVS_RCVR) {
            if(val <= (-10000.0F))     sprintf(out, "%s: %.4f ns/s", plot[id].plot_id,val);
            else if(val <= (-1000.0F)) sprintf(out, "%s: %.5f ns/s", plot[id].plot_id,val);
            else if(val >= 10000.0F)   sprintf(out, "%s: %.4f ns/s", plot[id].plot_id,val);
            else                       sprintf(out, "%s: %.6f ns/s", plot[id].plot_id,val);
         }
         else if(rcvr_type == UBX_RCVR) {
            if(val <= (-10000.0F))     sprintf(out, "%s: %.4f ns", plot[id].plot_id,val);
            else if(val <= (-1000.0F)) sprintf(out, "%s: %.5f ns", plot[id].plot_id,val);
            else if(val >= 10000.0F)   sprintf(out, "%s: %.4f ns", plot[id].plot_id,val);
            else                       sprintf(out, "%s: %.6f ns", plot[id].plot_id,val);
         }
         else if(rcvr_type == ZODIAC_RCVR) {
            if(val <= (-10000.0F))     sprintf(out, "%s: %.4f ns", plot[id].plot_id,val);
            else if(val <= (-1000.0F)) sprintf(out, "%s: %.5f ns", plot[id].plot_id,val);
            else if(val >= 10000.0F)   sprintf(out, "%s: %.4f ns", plot[id].plot_id,val);
            else                       sprintf(out, "%s: %.6f ns", plot[id].plot_id,val);
         }
         else if(TIMING_RCVR) {
            val /= 1000.0F;
            if(val <= (-10000.0F))     sprintf(out, "%s: %.4f us", plot[id].plot_id,val);
            else if(val <= (-1000.0F)) sprintf(out, "%s: %.5f us", plot[id].plot_id,val);
            else if(val >= 10000.0F)   sprintf(out, "%s: %.4f us", plot[id].plot_id,val);
            else                       sprintf(out, "%s: %.6f us", plot[id].plot_id,val);
         }
         else {
            if(val <= (-10000.0F))     sprintf(out, "%s: %.4f ns", plot[id].plot_id,val);
            else if(val <= (-1000.0F)) sprintf(out, "%s: %.5f ns", plot[id].plot_id,val);
            else if(val >= 10000.0F)   sprintf(out, "%s: %.4f ns", plot[id].plot_id,val);
            else                       sprintf(out, "%s: %.6f ns", plot[id].plot_id,val);
         }
      }
      else if(id >= NUM_PLOTS) {  // derived plots DERIVED_PLOTS !!!!!
         if(luxor && (id == BATTW)) {        // battery watts
            val = q.data[BATTI]*q.data[BATTV] / (float) queue_interval;
         }
         else if(luxor && (id == LEDW)) {   // LED watts
            val = q.data[LEDI]*q.data[LEDV] / (float) queue_interval;
         }
         else if(luxor && (id == EFF)) { // driver efficency
            val = q.data[BATTI]*q.data[BATTV];
            if(val) val = q.data[LEDI]*q.data[LEDV] / val;
            val *= 100.0F;
         }
         else if(luxor && (id == CCT)) { // color temo
            cct_dbg = 1;
            val = calc_cct(cct_type, 1, q.data[REDHZ]/(float)queue_interval, q.data[GREENHZ]/(float)queue_interval, q.data[BLUEHZ]/(float)queue_interval);
            cct_dbg = 0;
         }
         else {
            val = 0.0F;
         }

         if(luxor) {
            sprintf(out, "%s: %.3f %s", plot[id].plot_id, val, plot[id].units);
         }
         else {
            sprintf(out, "%s: %.6f %s", plot[id].plot_id, val, plot[id].units);
         }
      }
      else if((id == ONE) || (id == TWO)) {
         val = q.data[id]/queue_interval;
         if(plot_loc) sprintf(out, "%s: %.8f %s", plot[id].plot_id, val, plot[id].units);
         else         sprintf(out, "%s: (private) %s", plot[id].plot_id, plot[id].units);
      }
      else if(id == THREE) {
         val = q.data[id]/queue_interval;
         sprintf(out, "%s: %.3f %s", plot[id].plot_id, val, plot[id].units);
      }
      else {
         val = q.data[id]/queue_interval;
         if(luxor) {
            if(id == LUX2) val *= (float) lum_scale;
            sprintf(out, "%s: %.3f %s", plot[id].plot_id, val, plot[id].units);
         }
         else {
            sprintf(out, "%s: %.6f %s", plot[id].plot_id, val, plot[id].units);
         }
      }
      no_x_margin = no_y_margin = 1;
      vidstr(MOUSE_ROW+3, col, plot[id].plot_color, out);
      no_x_margin = no_y_margin = 0;
      len = strlen(out);
      if(len > max_len) max_len = len;

      no_mouse_info:
      col += (max_len+1);
      if(col >= TEXT_COLS) break;
   }

#ifdef FFT_STUFF
   if(plot[FFT].show_plot && fft_scale) {
      i = last_mouse_q - fft_queue_0;
      if(i < 0) i += plot_q_size;
      i /= (S32) fft_scale;
      val = (float) i * fps;
      if(i > (fft_length/2)) sprintf(out, "              ");
      else if(val) {
         val = 1.0F/val;
         sprintf(out, "%s: %-.1f sec     ", plot[fft_id].plot_id, val);
      }
      else sprintf(out, "%s: DC blocked    ", plot[fft_id].plot_id);
      no_x_margin = no_y_margin = 1;
      vidstr(MOUSE_ROW+2, 0, WHITE, &blanks[0]);  // erase the plot info area
//    if(show_live_fft) strupr(out);
//    else              strlwr(out);
      vidstr(MOUSE_ROW+2, fft_col, plot[FFT].plot_color, out);
      no_x_margin = no_y_margin = 0;
   }
#endif



   if(review_mode) {  // we are scrolling around in the plot queue data
      i = plot_q_out + plot_start;
      while(i >= plot_q_size) i -= plot_q_size;
      q = get_plot_q(i);

      jd_utc = q.q_jd;
      jd_gps = jd_utc + jtime(0,0,utc_offset,0.0);
      adjust_tz(2);  // tweak pri_ time for time zone

      if(view_all_data == 1) {
         sprintf(out, "All (DEL to stop): %02d:%02d:%02d.%03d  %s %s         ", 
             pri_hours,pri_minutes,pri_seconds, (int) ((pri_frac*1000.0)+0.5),  fmt_date(0),
             time_zone_set?tz_string:(q.sat_flags & UTC_TIME) ? "UTC":"GPS");
      }
      else if(view_all_data == 2) {
         sprintf(out, "Auto (DEL to stop): %02d:%02d:%02d.%03d  %s %s         ", 
             pri_hours,pri_minutes,pri_seconds, (int) ((pri_frac*1000.0)+0.5),  fmt_date(0),
             time_zone_set?tz_string:(q.sat_flags & UTC_TIME) ? "UTC":"GPS");
      }
      else {
         sprintf(out, "Review (DEL to stop): %02d:%02d:%02d.%03d  %s %s         ", 
             pri_hours,pri_minutes,pri_seconds, (int) ((pri_frac*1000.0)+0.5),  fmt_date(0),
             time_zone_set?tz_string:(q.sat_flags & UTC_TIME) ? "UTC":"GPS");
      }

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
      vidstr(MOUSE_ROW+5, PLOT_TEXT_COL+2,  CYAN, out);
      no_x_margin = no_y_margin = 0;
   }

   row = MOUSE_ROW+7;
   config_extra_plots();
   if((extra_plots == 0) && (SCREEN_WIDTH >= MEDIUM_WIDTH)) {
      no_x_margin = no_y_margin = 1;
      #ifdef ADEV_STUFF
         if((all_adevs == 0) || (mixed_adevs != 1)) {
            if     (ATYPE == OSC_ADEV) t = "ADEV";
            else if(ATYPE == PPS_ADEV) t = "ADEV";
            else if(ATYPE == OSC_MDEV) t = "MDEV";
            else if(ATYPE == PPS_MDEV) t = "MDEV";
            else if(ATYPE == OSC_HDEV) t = "HDEV";
            else if(ATYPE == PPS_HDEV) t = "HDEV";
            else if(ATYPE == OSC_TDEV) t = "TDEV";
            else if(ATYPE == PPS_TDEV) t = "TDEV";
            else                       t = "?DEV";

            sprintf(out, "%s %s       ", plot[PPS].plot_id, t);
            if(plot_adev_data) vidstr(row, PLOT_TEXT_COL+85,  PPS_ADEV_COLOR, out);
            else               vidstr(row, PLOT_TEXT_COL+85,  GREY,           out);

            sprintf(out, "%s %s       ", plot[OSC].plot_id, t);
            if(plot_adev_data) vidstr(row, PLOT_TEXT_COL+100, OSC_ADEV_COLOR, out);
            else               vidstr(row, PLOT_TEXT_COL+100, GREY,           out);
         }

         if(plot_sat_count) vidstr(row, PLOT_TEXT_COL+115,  COUNT_COLOR, "Sat count");
         else               vidstr(row, PLOT_TEXT_COL+115,  GREY,        "Sat count");
      #else
         if(plot_sat_count) vidstr(row, PLOT_TEXT_COL+85,  COUNT_COLOR, "Sat count");
         else               vidstr(row, PLOT_TEXT_COL+85,  GREY,        "Sat count");
      #endif
      no_x_margin = no_y_margin = 0;
   }

   if(right_arrow) {
      sprintf(out, " %c", RIGHT_ARROW);  
      no_x_margin = no_y_margin = 1;
      vidstr(MOUSE_ROW+7, (SCREEN_WIDTH/TEXT_WIDTH)-2, WHITE, out);
//    vidstr(MOUSE_ROW+6, (SCREEN_WIDTH/TEXT_WIDTH)-3, WHITE, out);
      no_x_margin = no_y_margin = 0;
   }
}




void mark_row(int y)
{
int x;
int color;

   // draw > < at the edges of the specified plot row
   if(continuous_scroll) color = CYAN;
   else                  color = WHITE;

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

   // draw user set data markers in the plot
   y = PLOT_ROW;
   x = PLOT_COL+plot_column;

   if(symbol == 0) {   // the mouse click marker
      temp = MARKER_COLOR;
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
         vchar(x,y+2, 0, MARKER_COLOR, '0'+symbol);
         VCHAR_SCALE = temp;
      #else
         // use text chars for the markers (may not go exactly where we want in DOS)
         #ifdef WIN_VFX
            graphics_coords = 1;
            x -= TEXT_WIDTH/2;
            if(x < 0) x = 0;
            y += 2;
         #endif
         sprintf(out, "%c", '0'+symbol);
         vidstr(y,x, MARKER_COLOR, out);
         graphics_coords = 0;
      #endif
   }
}

#define ZOOM_LEFT  0
#define ZOOM_SPACE 20


void show_queue_info()
{
int row, col;
int j;
float queue_time;

   // show the plot queue size and view interval settings

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
   else {
      row = MOUSE_ROW+1;
   }
   j = row;
   if(luxor == 0) --j;

   if(zoom_screen == 'P') {
      if(pause_data) vidstr(j, col+ZOOM_SPACE*2, YELLOW, "UPDATES PAUSED");
      else           vidstr(j, col+ZOOM_SPACE*2, YELLOW, "              ");
   }
   else {
      if(pause_data) vidstr(j, col, YELLOW, "UPDATES PAUSED");
      else           vidstr(j, col, YELLOW, "              ");
      --j;
   }

   #ifdef ADEV_STUFF
      if(adev_period <= 0.0F) {
         if(luxor) sprintf(out, "                    ");
         else      sprintf(out, "ADEV:  OFF          ");
      }
      else if(adev_period == 1.0F) sprintf(out, "ADEVQ: %ld pts ", adev_q_size-1);
      else if(adev_period < 1.0F)  sprintf(out, "ADEVP: %.3f secs    ", adev_period);
      else                         sprintf(out, "ADEVP: %.1f sec   ", adev_period);
      vidstr(j, col, WHITE, out);
      if(zoom_screen == 'P') {
         col += ZOOM_SPACE;
      }
      else {
         --j;
      }
   #endif

   queue_time = ((float) plot_q_size * queue_interval);
   queue_time /= nav_rate;
   if(queue_time >= (3600.0F * 24.0F)) sprintf(out, "PLOTQ: %.1f day  ", queue_time/(3600.0F*24.0F));
   else if(queue_time >= 3600.0F)      sprintf(out, "PLOTQ: %.1f hr   ", queue_time/3600.0F);
   else if(queue_time >= 60.0F)        sprintf(out, "PLOTQ: %.1f min  ", queue_time/60.0F);
   else if(queue_time > 0.0F)          sprintf(out, "PLOTQ: %.1f sec  ", queue_time);
   else                                sprintf(out, "PLOTQ: OFF          ");
   vidstr(j, col, WHITE, out);

   if(luxor && (lat || lon)) show_sun_moon(j+2, col);

   if(zoom_screen == 'P') ;
   else --j;

   view_row = j;
}

void show_view_info()
{
float show_time;
int col;
int color;

   // tuck the diaplay interval data into whatever nook or crannie 
   // we can find on the screen
   if((all_adevs == 0) || mixed_adevs) {
      if(zoom_screen == 'P') col = ZOOM_LEFT;
      else if(luxor) col = VER_COL;
      else col = FILTER_COL;

      if(show_min_per_div) {
         show_time = (float) (view_interval*queue_interval*PLOT_WIDTH);  //seconds per screen
         show_time /= plot_mag;
         show_time /= 60.0F;  // minutes/screen
         show_time /= (((float) PLOT_WIDTH) / (float) HORIZ_MAJOR);
         show_time /= nav_rate;
         if(SMALL_SCREEN) {
            sprintf(out, "%.1f min/div ", show_time);
         }
         else if((SCREEN_WIDTH <= 800) && (small_font != 1)) {
            sprintf(out, "%.1f min/div ", show_time);
         }
         else {
            if(show_time < 1.0F) sprintf(out, "VIEW: %.1f sec/div  ", show_time*60.0F);
            else                 sprintf(out, "VIEW: %.1f min/div  ", show_time);
         }

         if(view_interval != 1) color = BLUE;
         else                   color = WHITE;

         vidstr(view_row, col, color, out);
         if(zoom_screen == 'P') col += ZOOM_SPACE;
         else --view_row;
         
         show_time = (float) (view_interval*queue_interval*PLOT_WIDTH);  // seconds per screen
         show_time /= plot_mag;
         show_time /= nav_rate;
         if(show_time < (2.0F*3600.0F)) sprintf(out, "VIEW: %.1f min     ", show_time/60.0F);
         else sprintf(out, "VIEW: %.1f hrs     ", show_time/3600.0F);
         vidstr(view_row, col, color, out);
         if(zoom_screen == 'P') ;
         else --view_row;
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

   for(x=0; x<PLOT_WIDTH; x++) {  // draw vertical features
      col = PLOT_COL + x;
      dot(col, PLOT_ROW, WHITE);                // top of graph
      dot(col, PLOT_ROW+PLOT_HEIGHT-1, WHITE);  // bottom of graph

      if((x % HORIZ_MAJOR) == 0) {  // major tick marks
         dot(col, PLOT_ROW+1, WHITE);               // top horizontal axis
         dot(col, PLOT_ROW+2, WHITE);
         dot(col, PLOT_ROW+3, WHITE);

         dot(col, PLOT_ROW+PLOT_HEIGHT-2, WHITE);   // bottom horizontal axis     
         dot(col, PLOT_ROW+PLOT_HEIGHT-3, WHITE);
         dot(col, PLOT_ROW+PLOT_HEIGHT-4, WHITE);

         for(j=0; j<=PLOT_CENTER; j+=VERT_MAJOR) {
            dot(col,   PLOT_ROW+PLOT_CENTER+j-1, WHITE);  // + at intersections
            dot(col,   PLOT_ROW+PLOT_CENTER+j,   WHITE);
            dot(col,   PLOT_ROW+PLOT_CENTER+j+1, WHITE);
            if(col > PLOT_COL) dot(col-1, PLOT_ROW+PLOT_CENTER+j,   WHITE);
            dot(col+1, PLOT_ROW+PLOT_CENTER+j,   WHITE);

            dot(col,  PLOT_ROW+PLOT_CENTER-j-1, WHITE);  // + at intersections 
            dot(col,  PLOT_ROW+PLOT_CENTER-j,   WHITE);
            dot(col,  PLOT_ROW+PLOT_CENTER-j+1, WHITE);
            if(col > PLOT_COL) dot(col-1,PLOT_ROW+PLOT_CENTER-j,    WHITE);
            dot(col+1,PLOT_ROW+PLOT_CENTER-j,    WHITE);
         }

         color = GREY;  // WHITE;
         if(plot_adev_data) {
            if((x % (HORIZ_MAJOR*3)) == 0) color = CYAN;  // ADEV bins
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

         if(plot_adev_data) color = CYAN;   // subtly highlight ADEV decades
         else               color = GREY;
         for(j=VERT_MAJOR; j<=PLOT_CENTER; j+=VERT_MAJOR) {
            dot(col, PLOT_ROW+PLOT_CENTER+j, color);
            dot(col, PLOT_ROW+PLOT_CENTER-j, color);
            if(SCREEN_HEIGHT > 600) {
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
      dot(PLOT_COL, PLOT_ROW+PLOT_CENTER+y, WHITE);
      dot(PLOT_COL, PLOT_ROW+PLOT_CENTER-y, WHITE);
      dot(PLOT_COL+PLOT_WIDTH-1, PLOT_ROW+PLOT_CENTER+y, WHITE);
      dot(PLOT_COL+PLOT_WIDTH-1, PLOT_ROW+PLOT_CENTER-y, WHITE);

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

   if(HIGHLIGHT_REF) {   // highlight plot center reference line
      mark_row(PLOT_CENTER);
   }
}


void format_plot_title()
{
int len;
int i, j;
char c;

   j = 0;
   i = 0;
   out[0] = 0;
   len = strlen(plot_title);

   for(i=0; i<len; i++) {
      if((unsigned) j > (sizeof(plot_title)-2)) break;

      c = plot_title[i];
      if(c == '&') {
         if(plot_title[i+1] == '&') {  // && -> disciplining parameters
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
         else if(plot_title[i+1] == 'd') {  // &d -> date
            if((unsigned)j > (sizeof(plot_title)-20)) break;
            sprintf(&out[j], "%s", fmt_date(0));
            j = strlen(out);
            ++i;
         }
         else if(plot_title[i+1] == 't') {  // &t -> time
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

   if(showing_help) return;

   if(text_mode) {
      row = TEXT_ROWS-1;
      col = 0;
      no_x_margin = no_y_margin = 1;
   }
   else {
      row = (PLOT_ROW+PLOT_HEIGHT)/TEXT_HEIGHT-1;
//    if(zoom_screen == 'P') row -= 1;
      col = PLOT_TEXT_COL + 1;
   }

   if(plot_title[0]) {
      format_plot_title();
      vidstr(row, col, TITLE_COLOR, out);
      if((text_mode == 0) && (zoom_screen == 0)) { 
         line(PLOT_COL,PLOT_ROW+PLOT_HEIGHT-1,  PLOT_COL+PLOT_WIDTH,PLOT_ROW+PLOT_HEIGHT-1, WHITE);
      }
   }

   if(text_mode) return;
   if(zoom_screen == 'P');
   else if(zoom_screen) return;
   else if(first_key) return;

   if(debug_text[0]) {
      if(show_mah) vidstr(row-1, col, WHITE, debug_text);
      else         vidstr(row-1, col, GREEN, debug_text);
   }

   if(debug_text2[0]) {
      vidstr(row-2, col, YELLOW, debug_text2);
   }

   if(debug_text3[0]) {
      vidstr(row-3, col, CYAN, debug_text3);
   }

   if(filter_count) {
      sprintf(out, "Filter: %ld", filter_count);
      vidstr(PLOT_TEXT_ROW+1, col, WHITE, out);
   }
}


void plot_axes()
{
   // draw the plot background grid and label info
   if(first_key) return;   // plot area is in use for help/warning message

   erase_plot(0);          // erase plot area
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



float round_scale(float val)
{
double decade;

   // round the scale factor to a 1 2 (2.5) 5  sequence
   for(decade=1.0E-6; decade<=1.0E9; decade*=10.0) {
      if(val <= decade)       return (float) decade;
      if(val <= (decade*2.0)) return (float) (decade*2.0);
//    if(val <= (decade*2.5)) if(decade >= 10.0) return (float) (decade*2.5);
      if(val <= (decade*2.5)) return (float) (decade*2.5);
      if(val <= (decade*5.0)) return (float) (decade*5.0);
   }

   return val;
}


void scale_plots()
{
long i;
struct PLOT_Q q;
int k;
float scale[NUM_PLOTS+DERIVED_PLOTS];
float val;

   // Calculate plot scale factors and center line reference points.
   // This routine uses the min and max values collected by calc_plot_statistics()

   if((auto_scale == 0) && (auto_center == 0)) return;
   if(queue_interval <= 0) return;

   // calc_queue_stats() already scanned the queue data to find mins and maxes
   // now we calculate good looking scale factors and center points for the plots
   if(auto_scale && plot_q_last_col) {  // we have data to calculate good graph scale factors from
      i = (PLOT_HEIGHT / VERT_MAJOR) & 0xFFFE;  //even number of major vertical divisions
      //  if(SCREEN_HEIGHT >= MEDIUM_HEIGHT) i -= 1;  // prevents multiple consecutive rescales
      i -= 1;  //!!!!
      if(i <= 0) i = 1;

      // scale graphs to the largest side of the curve above/below
      // the graph center reference line
      for(k=0; k<NUM_PLOTS+DERIVED_PLOTS; k++) {
         if(auto_center && plot[k].float_center) {
            plot[k].plot_center = (plot[k].max_val+plot[k].min_val) / 2.0F;
         }

         if((plot[k].max_val >= plot[k].plot_center) && (plot[k].min_val < plot[k].plot_center)) { 
            if((plot[k].max_val-plot[k].plot_center) >= (plot[k].plot_center-plot[k].min_val)) {
               scale[k] = (plot[k].max_val-plot[k].plot_center);
            }
            else {
               scale[k] = plot[k].plot_center - plot[k].min_val;
            }
         }
         else if(plot[k].max_val >= plot[k].plot_center) {
            scale[k] = (plot[k].max_val - plot[k].plot_center);
         }
         else {
            scale[k] = (plot[k].plot_center - plot[k].min_val);
         }

         if(i > 1) scale[k] = (scale[k] / (float) (i/2));
         scale[k] *= plot[k].ref_scale;

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

      if(auto_center) {  // center the plots
         for(k=0; k<NUM_PLOTS+DERIVED_PLOTS; k++) {  
            if(plot[k].float_center) {
               plot[k].plot_center = (plot[k].max_val+plot[k].min_val) / 2.0F;  
            }
         }
         last_dac_voltage = plot[DAC].plot_center;
//!!!!   last_temperature = plot[TEMP].plot_center;
      }
   }
   else if(auto_center && (plot_q_last_col >= 1)) {  // center these graphs around the last recorded value
      if(filter_count) q = filter_plot_q(plot_q_last_col-1);
      else             q = get_plot_q(plot_q_last_col-1);

      for(k=0; k<NUM_PLOTS+DERIVED_PLOTS; k++) { 
         if(plot[k].float_center) {
            if(k == TEMP) plot[k].plot_center = scale_temp(q.data[TEMP] / (float) queue_interval);
            else if(luxor && (k == TC2)) plot[k].plot_center = scale_temp(q.data[k] / (float) queue_interval);
            else if(luxor && (k == LUX1))  plot[k].plot_center = (q.data[k] / (float) queue_interval) * (float) lux_scale;
            else if(luxor && (k == LUX2))  plot[k].plot_center = (q.data[k] / (float) queue_interval) * (float) lum_scale;
            else if(k >= NUM_PLOTS) {  // derived plots DERIVED_PLOTS !!!!!
               if(luxor && (k == BATTW)) {        // battery watts
                  plot[k].plot_center = q.data[BATTI]*q.data[BATTV] / (float) queue_interval;
               }
               else if(luxor && (k == LEDW)) {   // LED watts
                  plot[k].plot_center = q.data[LEDI]*q.data[LEDV] / (float) queue_interval;
               }
               else if(luxor && (k == EFF)) { // driver efficency
                  val = q.data[BATTI]*q.data[BATTV];
                  if(val) val = q.data[LEDI]*q.data[LEDV] / val;
                  val *= 100.0F;
                  plot[k].plot_center = val;
               }
               else if(luxor && (k == CCT)) { // color temo
                  val = calc_cct(cct_type, 1, q.data[REDHZ]/(float)queue_interval, q.data[GREENHZ]/(float)queue_interval, q.data[BLUEHZ]/(float)queue_interval);
                  plot[k].plot_center = val;
               }
               else {
                  plot[k].plot_center = 0.0F;
               }
            }
            else plot[k].plot_center = q.data[k] / (float) queue_interval;
         }
      }
   }

   // finally we round the center line reference values to multiples of 
   // the scale factor (note that this tweak can allow a plot to go slightly 
   // off scale)
   if(auto_center) {
      for(k=0; k<NUM_PLOTS+DERIVED_PLOTS; k++) {
         if(plot[k].float_center && plot[k].scale_factor) {
            plot[k].plot_center *= plot[k].ref_scale;
            plot[k].plot_center = (LONG_LONG) ((plot[k].plot_center / plot[k].scale_factor)) * plot[k].scale_factor;
            plot[k].plot_center /= plot[k].ref_scale;
         }
      }
   }
}


int last_plot_col;
u08 plot_dot;

int plot_y(float val, int last_y, int color)
{
int py;

   // draw a data point on the plot
   py = (int) (val * (float) VERT_MAJOR);
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
float y;
float val;
struct PLOT_Q q;
int col;
int py;
float x;
int k;

   // draw all data points for the next column in the plot

   if(first_key) return;    // plot area is in use for help/warning message
   if(text_mode) return;    // no graphics available in text mode
   if(rcvr_type == NO_RCVR) return; 
   if(no_plots) return; 
   if(zoom_screen == 'P');
   else if(zoom_screen) return;
   if(queue_interval <= 0) return;  // no queue data to plot

   if(filter_count) q = filter_plot_q(i);   // plot filtered data
   else             q = get_plot_q(i);      // plot raw data


   batt_mah += q.data[BATTI] * (queue_interval*view_interval/3600.0F);
   batt_mwh += q.data[BATTI] * q.data[BATTV] * (queue_interval*view_interval/3600.0F);
   load_mah += q.data[LEDI] * (queue_interval*view_interval/3600.0F);
   load_mwh += q.data[LEDI] * q.data[LEDV] * (queue_interval*view_interval/3600.0F);

   if(show_mah) {
      sprintf(debug_text, " Battery: Ah=%.3f  Wh=%.3f     LED: Ah=%.3f  Wh=%.3f", batt_mah,batt_mwh, load_mah,load_mwh);
   }


   for(k=0; k<NUM_PLOTS; k++) {     // compensate for drift rate
      q.data[k] -= ((plot[k].drift_rate*(float)plot_column)*view_interval); 
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

   if(q.sat_flags & TIME_SKIP) {  // flag skips and stutters in the time stamps
      if(plot_skip_data) {        // use small skip markers
         line(col,PLOT_ROW, col,PLOT_ROW+8, SKIP_COLOR); 
      }
   }

   if(plot_holdover_data && (q.sat_flags & HOLDOVER)) {  // flag holdover events
      if(plot_dot) {
         dot(col,PLOT_ROW+0, HOLDOVER_COLOR);
         dot(col,PLOT_ROW+1, HOLDOVER_COLOR);
      }
      else {
         line(PLOT_COL+last_plot_col,PLOT_ROW+0, col,PLOT_ROW+0, HOLDOVER_COLOR);
         line(PLOT_COL+last_plot_col,PLOT_ROW+1, col,PLOT_ROW+1, HOLDOVER_COLOR);
      }
   }

   // flag satellite constellation changes
   if(plot_const_changes && (q.sat_flags & CONST_CHANGE)) {  
      line(col,PLOT_ROW+PLOT_HEIGHT, col,PLOT_ROW+PLOT_HEIGHT-8, CONST_COLOR); 
//    plot_dot = 1;  // highlights satellite change discontinuites
   }

   if(plot_sat_count) {
      py = (q.sat_flags & SAT_COUNT) * COUNT_SCALE;
      if(py > PLOT_HEIGHT) py = PLOT_HEIGHT;
      py = (PLOT_ROW+PLOT_HEIGHT) - py;
      if(plot_dot) dot(col,py, COUNT_COLOR);
      else         line(PLOT_COL+last_plot_col,last_count_y, col,py, COUNT_COLOR);
      last_count_y = py;
   }

   // draw each of the data plots
   for(k=0; k<NUM_PLOTS+DERIVED_PLOTS; k++) {
      if(plot[k].show_plot) {
         if(k == TEMP) y = (float) ((scale_temp(q.data[k] / queue_interval) - plot[k].plot_center) * plot[k].ref_scale);  // y = ppt
         else if(luxor && (k == TC2)) y = (float) ((scale_temp(q.data[k] / queue_interval) - plot[k].plot_center) * plot[k].ref_scale);  // y = ppt
         else if(luxor && (k == LUX1)) y = (float) (((q.data[k] / queue_interval * lux_scale) - plot[k].plot_center) * plot[k].ref_scale);
         else if(luxor && (k == LUX2)) y = (float) (((q.data[k] / queue_interval * lum_scale) - plot[k].plot_center) * plot[k].ref_scale);
         else if(k >= NUM_PLOTS) {  // derived data DERIVED_PLOTS !!!!!
            if(luxor && (k == BATTW)) {        // battery watts
               y = (float) (((q.data[BATTI]*q.data[BATTV] / queue_interval) - plot[k].plot_center) * plot[k].ref_scale);
            }
            else if(luxor && (k == LEDW)) {   // LED watts
               y = (float) (((q.data[LEDI]*q.data[LEDV] / queue_interval) - plot[k].plot_center) * plot[k].ref_scale);
            }
            else if(luxor && (k == EFF)) { // driver efficency
               val = q.data[BATTI]*q.data[BATTV];
               if(val) val = q.data[LEDI]*q.data[LEDV] / val;
               val *= 100.0F;
               y = (float) ((val - plot[k].plot_center) * plot[k].ref_scale);
            }
            else if(luxor && (k == CCT)) { // color temo
               val = calc_cct(cct_type, 1, q.data[REDHZ]/(float)queue_interval, q.data[GREENHZ]/(float)queue_interval, q.data[BLUEHZ]/(float)queue_interval);
               y = (float) ((val - plot[k].plot_center) * plot[k].ref_scale);
            }
            else {
               y = 0.0F;
            }
         }
         else {
            y = (float) (((q.data[k] / queue_interval) - plot[k].plot_center) * plot[k].ref_scale);  // y = ppt
         }

         y /= (float) (plot[k].scale_factor*plot[k].invert_plot);
         plot[k].last_y = plot_y(y, plot[k].last_y, plot[k].plot_color);
      }

      if(plot[k].show_trend) {
         x = view_interval * queue_interval * (float) plot_column;
         if(plot_mag) x /= (float) plot_mag;
         y = (float) (((plot[k].a0 + (plot[k].a1*x)) - plot[k].plot_center) * plot[k].ref_scale);  // y = ppt
         y /= (float) (plot[k].scale_factor*plot[k].invert_plot);
         plot[k].last_trend_y = plot_y(y, plot[k].last_trend_y, plot[k].plot_color);
      }
   }

}

struct PLOT_Q last_q;

void add_stat_point(struct PLOT_Q *q)
{
int k;
float val;
float x;

   // calculate statistics of the points in the plot window
   if(queue_interval <= 0) return;

   x = (stat_count * view_interval * queue_interval);
   for(k=0; k<NUM_PLOTS+DERIVED_PLOTS; k++) { 
      if(k >= NUM_PLOTS) {   // derived data DERIVED_PLOTS !!!! 
         if(luxor && (k == BATTW)) {        // battery watts
            val = q->data[BATTI]*q->data[BATTV] / (float) queue_interval;
         }
         else if(luxor && (k == LEDW)) {   // LED watts
            val = q->data[LEDI]*q->data[LEDV] / (float) queue_interval;
         }
         else if(luxor && (k == EFF)) { // driver efficency
            val = q->data[BATTI]*q->data[BATTV];
            if(val) val = q->data[LEDI]*q->data[LEDV] / val;
            val *= 100.0F;
         }
         else if(luxor && (k == CCT)) { // color temo  // !!!!!!! wrong! need raw data here
            val = calc_cct(cct_type, 1, q->data[REDHZ]/(float)queue_interval, q->data[GREENHZ]/(float)queue_interval, q->data[BLUEHZ]/(float)queue_interval);
         }
         else {
            val = 0.0F;
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

void calc_queue_stats()
{
long i;
int k;
float sxx, syy, sxy;
struct PLOT_Q q;
float qi;
float val;

   // prepare to calculate the statistics values of the plots
   for(k=0; k<NUM_PLOTS+DERIVED_PLOTS; k++) {  
      plot[k].sum_x      = 0.0F;
      plot[k].sum_y      = 0.0F;
      plot[k].sum_xx     = 0.0F;
      plot[k].sum_yy     = 0.0F;
      plot[k].sum_xy     = 0.0F;
      plot[k].stat_count = 0.0F;
      plot[k].sum_change = 0.0F;
      plot[k].max_val    = (-1.0E30F);
      plot[k].min_val    = (1.0E30F);
   }

   qi = (float) queue_interval;

   if(auto_center) {  // set min/max values in case auto scaling is off
      for(k=0; k<NUM_PLOTS+DERIVED_PLOTS; k++) {
         if(plot[k].float_center == 0) {
            plot[k].min_val = plot[k].max_val = plot[k].plot_center;
            if(k == TEMP) {      // !!!! ref_scale? 
               if(plot[k].plot_center == NEED_CENTER) { 
                  if(last_temperature) plot[k].min_val = plot[k].max_val = plot[k].plot_center = scale_temp(last_temperature);
               }
            }
            else if(luxor && (k == TC2)) {      // !!!! ref_scale? 
               if(plot[k].plot_center == NEED_CENTER) { 
                  plot[k].min_val = plot[k].max_val = plot[k].plot_center = scale_temp(tc2);
               }
            }
            else if(k == DAC) {  // !!!! ref_scale?
               if(plot[k].plot_center == NEED_CENTER) {
                  plot[k].min_val = plot[k].max_val = plot[k].plot_center = last_dac_voltage;
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

   i = plot_q_col0;
   while(i != plot_q_in) {  // scan the data that is in the plot window
      if(filter_count) q = filter_plot_q(i);
      else             q = get_plot_q(i);

      if(i == plot_q_col0) last_q = q;
      add_stat_point(&q);   // update statistics values
      last_q = q;

      if(auto_scale && (queue_interval > 0) && qi) {  // find plot min and max value
         for(k=0; k<NUM_PLOTS+DERIVED_PLOTS; k++) {
            if(k == TEMP) val = scale_temp(q.data[TEMP]/qi);
            else if(luxor && (k == TC2)) val = scale_temp(q.data[k]/qi);
            else if(luxor && (k == LUX1)) val = q.data[k]/qi*(float) lux_scale;
            else if(luxor && (k == LUX2)) val = q.data[k]/qi*(float) lum_scale;
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
                  val *= 100.0F;
               }
               else if(luxor && (k == CCT)) { // color temo
                  val = calc_cct(cct_type, 1, q.data[REDHZ]/qi, q.data[GREENHZ]/qi, q.data[BLUEHZ]/qi);
               }
               else {
                  val = 0.0F;
               }
            }
            else val = q.data[k]/qi;

            if(val > plot[k].max_val) plot[k].max_val = val;
            if(val < plot[k].min_val) plot[k].min_val = val;
//if(k == ONE) sprintf(plot_title, "max:%.8f min:%.8f", plot[k].max_val, plot[k].min_val);
         }
      }

      plot_q_last_col = i;
      i = next_q_point(i, 1);
      if(i < 0) break;      // end of plot data reached
   }

   if(stat_count == 0.0F) return;

   for(k=0; k<NUM_PLOTS+DERIVED_PLOTS; k++) { // calculate linear regression values
      sxy = plot[k].sum_xy - ((plot[k].sum_x*plot[k].sum_y)/stat_count);
      syy = plot[k].sum_yy - ((plot[k].sum_y*plot[k].sum_y)/stat_count);
      sxx = plot[k].sum_xx - ((plot[k].sum_x*plot[k].sum_x)/stat_count);
      plot[k].stat_count = stat_count;
      if(sxx == 0.0F) {
         plot[k].a1 = plot[k].a0 = 0.0F;
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

   if((all_adevs == 0) || mixed_adevs) {
      for(j=0; j<MAX_MARKER; j++) { // prepare to find what marked points are shown
         ticker[j] = 1;
      }

      i = plot_q_col0;
      while(i != plot_q_in) {  // plot the data that is in the queue
         plot_entry(i);        // plot the data values

         last_i = i;           // go to next point to plot
         i = next_q_point(i, 1);
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

   if(text_mode || (rcvr_type == NO_RCVR) || no_plots) {   // graphics are not available,  only draw text stuff
      plot_axes();
      #ifdef ADEV_STUFF
         if(all_adevs && (refresh_ok != 2)) {
            show_adev_info();
         }
      #endif
      if(process_com == 0) {
         show_satinfo();
      }
      if(refresh_ok) refresh_page();
      return;   // no plotting in text modes
   }

   // locate the first queue entry we will be plotting
   plot_q_col0 = plot_q_out + plot_start;
   while(plot_q_col0 >= plot_q_size) plot_q_col0 -= plot_q_size;

   if(zoom_screen == 'P') ;
   else if(zoom_screen) return;

   if(queue_interval <= 0) {
      plot_axes();
      return;  // we have no queued data to plot
   }

   #ifdef FFT_STUFF
      if(show_live_fft && (live_fft != FFT)) {    // calc FFT over the live data
         calc_fft(live_fft);
      }
   #endif
   calc_queue_stats();  // calc stat info data that will be displayed
   scale_plots();       // find scale factors and the values to center the graphs around 
   plot_axes();         // draw and label the plot grid
   plot_queue_data();   // plot the queue data

   #ifdef ADEV_STUFF
      if(refresh_ok != 2) {   // draw the adev info
         show_adev_info();
      }
   #endif

   if(process_com == 0) {
      show_satinfo();
   }
   if(refresh_ok) {     // copy buffered screen image to the screen
      refresh_page();
   }
}


void update_plot(int draw_flag)
{
struct PLOT_Q q;
double v;
float dop, div;
float jitter;

   // add current data values to the plot data queue
   q = get_plot_q(plot_q_in);

   q.data[TEMP] += temperature;
   q.data[DAC]  += dac_voltage;
   q.data[OSC]  += (OFS_SIZE) osc_offset;
   q.data[PPS]  += (OFS_SIZE) pps_offset;

   if(graph_lla || TIMING_RCVR || (rcvr_type == SCPI_RCVR) || (rcvr_type == UCCM_RCVR)) {
      if(ONE < NUM_PLOTS)   q.data[ONE] += (float) (lat*RAD_TO_DEG);
      if(TWO < NUM_PLOTS)   q.data[TWO] += (float) (lon*RAD_TO_DEG);
      if(THREE < NUM_PLOTS) q.data[THREE] += (float) alt;

      if((rcvr_type == SCPI_RCVR) || (rcvr_type == UCCM_RCVR)) {
         if(FOUR < NUM_PLOTS)  q.data[FOUR] += (float) tfom;
         if(FIVE < NUM_PLOTS)  q.data[FIVE] += (float) ffom;
      }
      else {
         if(FOUR < NUM_PLOTS)  q.data[FOUR] += (float) speed;
         if(FIVE < NUM_PLOTS)  q.data[FIVE] += (float) heading;
      }

      if(SIX < NUM_PLOTS) {
         dop = 0.0;
         div = 0.0;
         if(have_dops & TDOP) { dop += tdop; ++div; }
         if(have_dops & HDOP) { dop += hdop; ++div; }
         if(have_dops & VDOP) { dop += vdop; ++div; }
         if(have_dops & PDOP) { dop += pdop; ++div; }
         if(div) dop /= div;
         q.data[SIX] += (float) dop;
      }
   }
   else {
      if(initial_voltage == 0.0F) v = dac_voltage;
      else                        v = initial_voltage;
      if(luxor) {  // DERIVED_PLOTS !!!!
         if(LUX2 < NUM_PLOTS)    q.data[LUX2] += (float) lux2;
         if(LEDV < NUM_PLOTS)    q.data[LEDV] += (float) led_v;
         if(LEDI < NUM_PLOTS)    q.data[LEDI] += (float) led_i;
         if(PWMHZ < NUM_PLOTS)   q.data[PWMHZ] += (float) pwm_hz;
         if(TC2 < NUM_PLOTS )    q.data[TC2] += (float) tc2;

         if(BLUEHZ < NUM_PLOTS)  q.data[BLUEHZ] += blue_hz;     // color sensor
         if(GREENHZ < NUM_PLOTS) q.data[GREENHZ] += green_hz;
         if(REDHZ < NUM_PLOTS)   q.data[REDHZ] += red_hz;
         if(WHITEHZ < NUM_PLOTS) q.data[WHITEHZ] += white_hz;
         if(AUXV < NUM_PLOTS)    q.data[AUXV] += adc2;
      }
      else {
         if(ONE < NUM_PLOTS) q.data[ONE] += (float) ((dac_voltage-v)*osc_gain*(+1000.0F));
         if(TWO < NUM_PLOTS) q.data[TWO] = (float) ((q.data[OSC]*1000.0F)-q.data[ONE]); 
      }
   }

   jitter = (float) (this_time_msec - last_time_msec);  // timing message jitter
   if((rcvr_type == SCPI_RCVR) || (rcvr_type == UCCM_RCVR)) {  // !!!! kludge around STATUS messaage
      if(jitter > 1800.0) jitter = 1000.0;
   }
   q.data[MSGJIT] += jitter;
   q.data[MSGOFS] += (float) msg_ofs;

   if(FFT < NUM_PLOTS) q.data[FFT] = 0.0F;

   q.sat_flags &= SAT_FLAGS;       // preserve flag bits
   if(sat_count > SAT_COUNT) q.sat_flags |= SAT_COUNT; // satellite count
   else                      q.sat_flags |= sat_count;
   q.sat_flags |= (new_const & CONST_CHANGE);          // constellation change
   if(time_flags & 0x0001) q.sat_flags |= UTC_TIME;    // UTC/GPS time flag

//if((seconds%10) == 9) q.sat_flags |= TIME_SKIP;

   if((discipline_mode == 6) && osc_control_on) ;
   else if(discipline_mode != 0) q.sat_flags |= HOLDOVER;

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
if(view_all_data == 2) ; else
      if(draw_flag == 0) goto scroll_it;

      plot_entry(plot_q_in);  // plot latest queue entry
      plot_column += plot_mag;
      if(plot_column >= PLOT_WIDTH) {   // it's time to scroll the plot left
if(view_all_data == 2) view_all(); else { // !!!!!!!! testing automatic view all
         plot_column -= (PLOT_SCROLL*plot_mag);
         plot_start = plot_q_in - (((PLOT_WIDTH - PLOT_SCROLL)*view_interval)/plot_mag) - plot_q_out;
         if(plot_start < 0) {
             if(plot_q_full) {
                plot_start += plot_q_size;
                if(plot_start < 0) plot_start = 0;
             }
             else plot_start = 0;
         }
         draw_plot(0);
}
      }
      else if(auto_scale && off_scale) {  // a graph is now off scale,  redraw the plots to rescale it
         draw_plot(0);
         off_scale = 0;
      }
      else if(continuous_scroll) draw_plot(0);
   }
   else {
      scroll_it:
      if(continuous_scroll) draw_plot(0);
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
   need_redraw = 0;

   survey_done = 0;
   if((check_precise_posn == 0) && (precision_survey == 0) && (show_fixes == 0)) {
      plot_lla = 0;
////  if(doing_survey == 0) plot_lla = 0;
   }

   erase_screen();
   show_log_state();
   request_rcvr_info(202);
   if(plot_azel) update_azel = 1;

   draw_plot(1);  // redraw the plot area
}




volatile int timer_serve;


void zoom_zoomed()
{
   erase_screen();

   if(zoom_screen == 'P') {
      goto unzoom;
   }
   else if((un_zoom == ZOOM_SHARED) || (un_zoom == ZOOM_LLA) || (un_zoom == ZOOM_AZEL) || (un_zoom == ZOOM_PLOT)) {
      unzoom:
      un_zoom = UN_ZOOM;
      change_zoom_config(987);
      zoom_screen = 0;
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
      all_adevs = 0;
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
         zoom_screen = 0;
         change_zoom_config(0);
         show_fixes = 0;
if(rcvr_type != NO_RCVR) show_fixes = zoom_fixes;  // zzzzzz
         change_fix_config(show_fixes);
         plot_lla = 0;
         no_redraw = 1;
         config_screen(128);
         erase_screen();
         no_redraw = 0;
   }
}


void config_lla_zoom(int why)
{
    change_zoom_config(4);
    zoom_screen = 'L';
    zoom_fixes = show_fixes;
    show_fixes = 1;
    change_fix_config(show_fixes);
    if(show_fixes && (precision_survey == 0)) {  // set reference position for fix map
       precise_lat = lat;
       precise_lon = lon;
       precise_alt = alt;
       ref_lat = lat;
       ref_lon = lon;
       ref_alt = alt;
       cos_factor = cos(ref_lat);
    }
    start_3d_fixes(-1, 3);  // !!!!! only if show_fixes == 1?
    config_screen(113);
}


u08 new_mouse_info;

void do_mouse_click()
{
S32 i;

   // process mouse clicks 

   if(mouse_x < 0) return;
   else if(mouse_x >= SCREEN_WIDTH) return;
   else if(mouse_y < 0) return;
   else if(mouse_y >= SCREEN_HEIGHT) return;
   else if(first_key) return;

//sprintf(out, "this:%d last:%d  x:%d y:%d ts:%d", this_button,last_button, mouse_x,mouse_y, timer_serve);
//DEBUGSTR(out);
//refresh_page();

   if(zoom_screen && (this_button == 1) && (last_button == 0) && (mouse_y < 100) && (mouse_x < 100)) {  // un-zoom plot display
      add_kbd(ESC_CHAR);
   }
   else if((un_zoom == ZOOM_ADEVS) && (this_button == 1) && (last_button == 0)) {  // un-zoom all_adevs display
      zoom_zoomed();
   }
   else if(zoom_screen && (zoom_screen != 'P') && (this_button == 1) && (last_button == 0)) {  // un-zoom display
      zoom_zoomed();
   }
   else if((zoom_screen == 'P') && (this_button == 1) && (last_button == 0) && (mouse_y < (PLOT_ROW-TEXT_HEIGHT))) {  // un-zoom plot display
      zoom_zoomed();
   }
   else if((zoom_screen == 0) && show_fixes && (this_button == 1) && (last_button == 0) && (mouse_y < LLA_SIZE) && (mouse_x > (SCREEN_WIDTH-LLA_SIZE))) {  // zoom fix map
      goto fixes;
   }
   else if(rcvr_type == NO_RCVR) {
      if((this_button == 1) && (last_button == 0)) {
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
   else if((zoom_screen == 0) && share_active && shared_item && (this_button == 1) && (last_button == 0) && (mouse_y >= PLOT_ROW) && (mouse_x > (PLOT_COL+PLOT_WIDTH))) {  // zoom shared plot item
      if(mouse_x >= (SCREEN_WIDTH-48)) ;        // needed for XQuartz resize box
      else if(mouse_y >= (SCREEN_HEIGHT-48)) ;
      else {
         change_zoom_config(88);
         un_zoom = ZOOM_SHARED;
         if(shared_item == 'W') zoom_screen = 'W';
         else if(shared_item == 'M') zoom_screen = 'M';
         else if(shared_item == 'B') zoom_screen = 'B';
         else if(shared_item == 'S') zoom_screen = 'S';
         else if(shared_item == 'L') zoom_screen = 'L';
         else zoom_screen = 0;
         zoom_fixes = show_fixes;
         config_screen(124);
      }
   }
// else if((zoom_screen == 0) && (this_button == 1) && (last_button == 0) && (mouse_y < ACLOCK_R*2) && (mouse_x > ((ADEV_COL+adevs_shown)*TEXT_WIDTH))) {  // zoom map/watch
   else if((zoom_screen == 0) && (this_button == 1) && (last_button == 0) && (mouse_y < ACLOCK_R*2) && (mouse_x > (ADEV_COL*TEXT_WIDTH))) {  // zoom map/watch
      fixes:
      if(all_adevs == 0) {
         change_zoom_config(88);
         un_zoom = ZOOM_AZEL;
         if(adevs_shown && (mouse_x < ((ADEV_COL+adevs_shown)*TEXT_WIDTH))) {
            un_zoom = ZOOM_ADEVS;
            all_adevs = aa_val;
            old_plot_adevs = plot_adev_data;
            plot_adev_data = 1;
         }
         else if(show_fixes) zoom_screen = 'L';
         else if(plot_watch && plot_azel) zoom_screen = 'B';
         else if(plot_watch) zoom_screen = 'W';
         else if(plot_azel) zoom_screen = 'M';
         else if(plot_signals) zoom_screen = 'S';
         else zoom_screen = 0;
         zoom_fixes = show_fixes;
         config_screen(124);
      }
   }
   else if((zoom_screen == 0) && (this_button == 1) && (last_button == 0) && (mouse_y < (MOUSE_ROW*TEXT_HEIGHT)) && (mouse_y >= ((POSN_ROW+2)*TEXT_HEIGHT)) && (mouse_x < (MINOR_COL*8))) {  // zoom clock or sat info
      if(all_adevs) ;
      else if(mouse_y <= ((POSN_ROW+6)*TEXT_HEIGHT)) {
         if(show_fixes) un_zoom = ZOOM_LLA;
         else un_zoom = ZOOM_CLOCK;
         config_lla_zoom(1);
      }
      else {
         un_zoom = ZOOM_INFO;
         change_zoom_config(88);
         if(mouse_y < (last_sat_row*TEXT_HEIGHT)) zoom_screen = 'I';
         else zoom_screen = 'C';
         zoom_fixes = show_fixes;
         config_screen(124);
      }
   }
   else if((zoom_screen == 0) && (this_button == 1) && (last_button == 0) && (mouse_y >= ((MOUSE_ROW+4)*TEXT_HEIGHT)) && (mouse_y < ((PLOT_ROW-TEXT_HEIGHT)))) {  // zoom plot
      un_zoom = 'P';
      change_zoom_config(89);
      zoom_screen = 'P';
      zoom_fixes = show_fixes;
      config_screen(124);
   }
   else {
      mouse_x -= PLOT_COL;
      if((mouse_x >= 0) && (mouse_x < plot_column) && (mouse_y >= PLOT_ROW) && (mouse_y < (PLOT_ROW+PLOT_HEIGHT))) { // mouse is in the plot area
         if(last_mouse_x != mouse_x) new_mouse_info |= 0x01;
         else if(last_mouse_y != mouse_y) new_mouse_info |= 0x01;
         i = plot_q_col0 + ((view_interval * mouse_x)/plot_mag);
         while(i >= plot_q_size) i -= plot_q_size;
         last_mouse_q = i;
         mouse_plot_valid = 1;  // mouse is in the plot area

         if((this_button == 1) && (last_button == 0)) {  // zoom plot
            view_all_data = 0;
            right_time = 0;
            kbd_zoom();
         }
         else if((this_button == 2) && (last_button == 0)) {  // mark plot and center on it
            view_all_data = 0;
            right_time = 0;
            right_click:
            mark_q_entry[0] = last_mouse_q;  // mark the point

            no_mouse_review = 1; // inhibit recursive call to get_mouse_info
            goto_mark(0);        // center plot on the marked point
            no_mouse_review = 0;
         }
         else if((this_button == 2) && (last_button == 2)) {
            Sleep(10);
            if(++right_time >= RIGHT_DELAY) goto right_click;
         }
         else right_time = 0;
      }
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
   SAL_serve_message_queue();
   Sleep(0);

   if(SAL_is_app_active()) {
      SAL_WINAREA wndrect;
      SAL_client_area(&wndrect);    // Includes menu if any

      POINT cursor;
      GetCursorPos(&cursor);

      mouse_x = cursor.x - wndrect.x;
      mouse_y = cursor.y - wndrect.y;
      // adjust position in case screen area is scaled down
      if(wndrect.w && wndrect.h) {
         val = ((float) mouse_x * (float) SCREEN_WIDTH) / (float) wndrect.w;
         mouse_x = (int) val;
         val = ((float) mouse_y * (float) SCREEN_HEIGHT) / (float) wndrect.h;
         mouse_y = (int) val;
      }

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
      if(!XQueryPointer(display,screen, &root,&child, &rx,&ry, &wx,&wy, &bmask)) return 0;
      mouse_x = wx;
      mouse_y = wy;

      last_button = this_button;
      if(bmask & Button1Mask) this_button = 1;
      else if(bmask & Button3Mask) this_button = 2;
      else this_button = 0;

      if(last_button != this_button) {
         new_button |= 0x02;
      }

      val = 0.0F;
#endif

      if(this_button) view_all_data = 0;

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
      BEEP(7);
   }
   else if(i < 0) {  // we are at the start of the data in the queue
      i = 0;
      BEEP(8);
   }

   review = i;
   review_mode = 1;
   plot_start = review;

   draw_plot(1);
}

void zoom_review(long i, u08 beep_ok)
{
   i -= plot_q_out;

   if(i >= plot_q_count) {  // we are past the end of the data, back up a minor tick
      i -= plot_q_count;
      if(beep_ok) BEEP(9);
   }
   if(i < 0) {  // we are at the start of the data in the queue
      i = 0;
      if(beep_ok) BEEP(10);
   }

   review = i;
   review_mode = 1;
   plot_start = review;

   draw_plot(0);

   if(no_mouse_review) ;
   else get_mouse_info();  // update cursor info in case we are not doing live data
                           // was bad idea - get_mouse_info calls goto_mark() which calls get_mouse_info
   refresh_page();
}

void kbd_zoom()
{
u32 val;

   // toggle view interval from 1 sec/pixel to longer view
   if(mouse_plot_valid && plot_mag) {
      if((view_interval == 1L) && (queue_interval > 0)) {
         if(user_view) view_interval = user_view;
         else view_interval = (3600L/HORIZ_MAJOR)/queue_interval;  // 1 hr per division
      }
      else {
         day_plot = 0;
         view_interval = 1L;
      }

      mark_q_entry[0] = last_mouse_q;  // marker 0 is mouse click marker
      val = last_mouse_q;
      val -= (((PLOT_WIDTH/2)*view_interval) / (long) plot_mag);  // center point on screen
      if(last_mouse_q < plot_q_out) val += plot_q_size;
      zoom_review(val, 1);
   }
}

void goto_mark(int i)
{
long val;

   // center plot window on the marked point
   if(mark_q_entry[i] && plot_mag) { // a queue entry is marked
      last_q_place = last_mouse_q;
      val = mark_q_entry[i];
      val -= (((PLOT_WIDTH/2)*view_interval) / (long) plot_mag);  // center point on screen
      if(mark_q_entry[i] < plot_q_out) val += plot_q_size;
      zoom_review(val, 0);
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
   if(draw_flag) draw_plot(1);
}

void do_review(int c)
{
   // move the view around in the plot queue
   if(c == HOME_CHAR)  {  // start of plot data 
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
      if(queue_interval > 0) plot_review(review+(long) (24L*3600L/queue_interval));
   }
   else if(c == '>') {  
      if(review_mode == 0) goto review_end;
      if(queue_interval > 0) plot_review(review-(long) (24L*3600L/queue_interval));
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
      view_interval = (3600L/HORIZ_MAJOR)/queue_interval;  // 1 hr per division
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
         view_interval = (3600L/HORIZ_MAJOR)/queue_interval;  // 1 hr per division
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

void key_wait(char *s)
{
// debug routine - display message and wait for key
strcpy(plot_title, s);
title_type = OTHER;
show_title();
refresh_page();
BEEP(300);
   while(1) {
      #ifdef WIN_VFX
         SAL_serve_message_queue();
         Sleep(0);
      #endif
      #ifdef USE_X11
         get_x11_event();
         XFlush(display);
      #endif
      if(KBHIT()) break;
   }
   GETCH();
BEEP(301);
}

void wait_for_key(int serve)
{
   if(timer_serve) return; // don't let windows timer access this routine

   first_key = ' ';  // make sure plot area is not disturbed
   while(break_flag == 0) {
      #ifdef WIN_VFX
         SAL_serve_message_queue();
         Sleep(0);
      #endif
      #ifdef USE_X11
         get_x11_event();
         if(display) XFlush(display);
      #endif
      if(KBHIT()) break;

      if(serve && (process_com || (rcvr_type == NO_RCVR))) { //rxx
         get_pending_gps();   //!!!! possible recursion
      }
   }
}


void abort_wakeup()
{
   // see if user says to stop the receiver wakeup loop
#ifdef WIN_VFX
   SAL_serve_message_queue();
   Sleep(0);
#endif
#ifdef USE_X11
   get_x11_event();
   XFlush(display);
#endif
   if(KBHIT()) {
      if(GETCH() == 0x1B) {
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

int get_script()
{
int i;

   // read a simulated keystroke from a script file
   i = fgetc(script_file);
   if(i < 0) {  // end of file
      close_script(0);
      return 0;
   }
   else if(i == 0x00) { // two byte cursor char
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
   else if(alarm_wait && (alarm_time || alarm_date || egg_timer || egg_val)) {
//sprintf(plot_title, "alarm waiting");
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
#else  // __linux__  __MACH__
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

   // configure system for any screen related command line options that were set

   if(need_screen_init) {
      need_screen_init = 0;
      init_screen();
   }

   if(keyboard_cmd == 0) {
      if(SCREEN_WIDTH < MEDIUM_WIDTH) {  // blank clock name for small screens
         if(watch_name[0] == 0)  strcpy(watch_name,  " ");
         if(watch_name2[0] == 0) strcpy(watch_name2, " "); 
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
      view_interval = (user_view * 3600L) / queue_interval;
      view_interval /= PLOT_WIDTH;
      if(view_interval <= 1L) view_interval = 1L;
      user_view = view_interval;
   }                                                 
   new_user_view = 0;

   // user set a log param on the command line, start writing the log file
   if(user_set_log) { 
      log_file_time = log_interval + 1;
      open_log_file(log_mode);
   }

   // user set a daylight savings time definition
   if(user_set_dst && have_year) {
      calc_dst_times(dst_list[dst_area]);
   }

   // if user did not enable/disable the big clock,  turn it on if it will fit
   if(user_set_clock_plot == 0) {  
      if(SCREEN_WIDTH > 800) plot_digital_clock = 1; 
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
      init_messages(1);
      did_init1 = 1;
   }
}



void set_defaults()
{
double angle;
int i;
int j;

   // This routine is used to give initial values to variables.
   // The QuickC linker does not let one initialize variables from more than 
   // one file (even if they are declared extern).

   invert_display = 0;
   setup_palette();

   unit_file_name = "tbolt";
   detect_rcvr_type = 1;
   rcvr_type = last_rcvr_type = TSIP_RCVR;
   nav_rate = 1.0;               // assume receiver running at 1Hz
   default_gnss = MIXED;

   sunrise_type = "Official";
   sunrise_horizon = sunset_horizon = (-50.0 / 60.0);
   play_sun_song = 0;

   min_sig_db = 30;      // low sig level threshold for sig level map
   sig_level_step = 2;   // sig level map signal steo size

   use_tsc = 1;          // use the TSC instruction for nanosecond counter
   last_time_msec = GetMsecs();  // message jitter ellapsed millisecond counter
   this_time_msec = msg_sync_msec = last_time_msec + 1000.0;
   time_zero = 0.0;
   time_sync_offset = TIME_SYNC_AVG;  // average of all tested receivers
   timing_mode = 0x03;  // assume UTC time

   traim_threshold = 1; // bogus value which should be rejected when initing the receiver
   eom_flag = 0;
   com_timeout = COM_TIMEOUT;
   restart_count = 0;
   first_request = 1;
   sats_enabled = 0xFFFFFFFF;
   update_disable_list(sats_enabled);
   system_mask = MIXED;
   did_init1 = 0;

   baud_rate = 9600;
   parity = NO_PAR;       // no parity (0=none 1=odd, 2=even)
   data_bits = 8;
   stop_bits = 1;

   this_msec = GetMsecs();

   #ifdef WINDOWS
      continuous_scroll = 1;        // windows defaults to continuous scroll mode

      IP_addr[0] = 0;     // TCP
      enable_timer = 1;   // enable windows dialog timer
   #endif

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

   beep_on = 1;           // allow beeps
   sound_on = 1;          // disable sound files

#ifdef WINDOWS
   com_port = 1;          // use COM1 for serial I/O
   usb_port = 0;
#else   // __linux__  __MACH__
   com_port = 0;          // use /dev/ttyUSB0 for serial I/O
   usb_port = 1;
#endif
   com_fd = (-1);
   process_com = 1;
com_running = (-1);
   first_msg = 1;        
   first_request = 1;

   moto_chans = 12;
   max_sat_display = MAX_SAT_DISPLAY;
   max_sats = 8;          // used to format the sat_info data
   max_sat_count = 8;
   sat_cols = 1;
   temp_sats = 8;
   if(max_sat_count > max_sat_display) {
      max_sats = max_sat_display;         // used to format the sat_info data
      max_sat_count = max_sat_display;
      temp_sats = max_sat_display;
   }
   sat_rows = max_sat_display;
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
   nvs_pps_width = NVS_PPS_WIDTH;  // in nanoseconds
    last_nvs_pps_width = NVS_PPS_WIDTH;  // in nanoseconds

//   set_utc_mode = 1;      // default to UTC mode
//   set_gps_mode = 0;

   delay_value = 50.0F * (1.0F / (186254.0F * 5280.0F)) / 0.66F;  // 50 feet of 0.66 vp coax
   dac_drift_rate = 0.0F; // volts per second

   last_rmode = 7;
   last_hours = last_log_hours = 99;
   last_second = 99;
   last_utc_offset = (-9999);
   need_delta_t = 1;
   force_utc_time = 1;
   first_sample = 1;       // flag cleared after first data point has been received
   time_color = RED;
   time_set_char = ' ';

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

   log_stream = 0;         // don't write serial data stream to the log file
   log_errors = 1;         // if set, log data errors
   if(luxor) {
      strcpy(log_name, "luxor.log"); 
      strcpy(raw_name, "luxor.raw");
   }
   else {
      strcpy(log_name, "tbolt.log");
      strcpy(raw_name, "tbolt.raw");
   }
   log_mode = "w";

if(0) {  // gggg enable raw receiver data logging
   log_stream = 0x02;
   raw_file = topen(raw_name, "wb");
}

   #ifdef ADEV_STUFF
      ATYPE = OSC_ADEV;
      adev_period = 1.0F;
      bin_scale = 5;        // 1-2-5 adev bin sequence
      n_bins = ADEVS;
      min_points_per_bin = 4;
      keep_adevs_fresh = 1;
      jitter_adev = 0;
   #endif
   aa_val = 2;      // all PPS adevs
   mixed_adevs = 1; // graphs and adev plots

   // flags to control what and what not to draw
   plot_adev_data = 1;      // adevs
   plot_skip_data = 1;      // time sequence and message errors
   plot_sat_count = 1;      // satellite count
   small_sat_count = 1;     // used compressed sat count plot
   plot_const_changes = 0;  // satellite constellation changes
   plot_holdover_data = 1;  // holdover status
   plot_digital_clock = 0;  // big digital clock
   plot_loc = 1;            // actual lat/lon/alt
   stat_id = "RMS";
   plot_dops = 0;           // dilution of precision (in place of filters on small screens)
   plot_filters = 1;        // if set, plot receiver filter settings
   plot_azel = 0;           // satellite azimuth/elevation map
   plot_signals = 0;        // satellite signal levels
   plot_el_mask = 1;        // show elevation angle mask in the azel plot
   map_trails = 1;          // draw satellite history trails
   dot_trails = 1;          // draw time markers on satellite history trails
   dynamic_trend_info = 1;  // if set, update trend line info dynamically
   plot_background = 0;     // if set, highlight plot area background color
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
      plot[i].show_stat = RMS;          // statistic to show
      if(i >= FIRST_EXTRA_PLOT) {
         extra_plots |= plot[i].show_plot;
      }
      plot_stat_info |= plot[i].show_stat;
   }

   plot[OSC].units = ppt_string;
   plot[DAC].plot_center = plot[TEMP].plot_center = NEED_CENTER;
   plot[MSGJIT].show_stat = SDEV; // SHOW_SPAN
   plot[MSGOFS].show_stat = AVG;  // SHOW_SPAN
   if(luxor) {
      plot[TC2].plot_center = NEED_CENTER;
   }

   auto_scale = 1;             // auto scale the plots
   auto_center = 1;            // and auto center them

   deg_string[0] = DEGREES;
   deg_string[1] = 0;
   DEG_SCALE = 'C';            // Celcius
   alt_scale = "m";            // meters
   level_type = "   ";         // AMU or dB
   dms = 0;                    // decimal degrees
   strcpy(tz_string,  "LOC");  // current time zone name
   strcpy(std_string, "LOC");  // normal time zone name
   strcpy(dst_string, "LOC");  // daylight savings time zone name

//   LLA_SPAN = 10.0;            // lla plot scale in feet per division
//   ANGLE_SCALE = 2.74e-6;      // degrees per foot
//   angle_units = "ft";

   LLA_SPAN = 3.0;
   ANGLE_SCALE = ((2.74e-6)*FEET_PER_METER); // degrees per meter
   angle_units = "m";

   cos_factor = 0.82;          // cosine of latitude
// LLA_SPAN = 20.0;
// ANGLE_SCALE = (2.74e-6*FEET_PER_METER); // degrees per meter
// angle_units = "m";

   adev_q_size = (33000L);         // good for 10000 tau
   plot_q_size = (3600L*24L*3L);   // 72 hours of data
   mouse_shown = 1;

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

   #ifdef TEMP_CONTROL
      do_temp_control = 0;     // don't attempt temperature control
      temp_control_on = 0;
      desired_temp = 40.0;     // in degrees C
      lpt_port = 0;
      port_addr = 0x0000;
   #endif
   mayan_correlation = MAYAN_CORR;

   ring_bell = (-1);
   bell_number = 0;

   get_clock_time();
   this_year = clk_year;
   init_dsm();        // set up the days to start of month table

   plot_title[0] = 0;
   title_type = NONE;
   greet_ok = 1;

   flag_faults = 1;   // show message errors as time skips


   debug_text[0] = debug_text2[0] = debug_text3[0] = 0;

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

   #ifdef SIG_LEVELS
      clear_signals();
   #endif

   #ifdef GREET_STUFF
      calendar_entries = calendar_count();
   #endif

   d_scale = 1.0F;
   osc_gain = user_osc_gain = (-3.5);
   gain_color = GREY;  // was yellow

   undo_fw_temp_filter = 0;
   idle_sleep = DEFAULT_SLEEP;
}



//
//
//   Process the GPS receiver data
//
//
void get_pending_gps()
{
u08 old_disable_kbd;

   // process all the data currently in the com port input buffer
   old_disable_kbd = disable_kbd;
   disable_kbd = 2; // (so that do_kbd() won't do anything when it's called by WM_CHAR during get_pending_gps())
   system_busy = 1; // we are doing other things besides waiting for serial data

   while((break_flag == 0) && (take_a_dump == 0) && (process_com || (rcvr_type == NO_RCVR)) && SERIAL_DATA_AVAILABLE()) { //rxx
      #ifdef WIN_VFX
         SAL_serve_message_queue();
         Sleep(0);
      #endif
      #ifdef USE_X11
         get_x11_event();
      #endif

      get_rcvr_message();
if((rcvr_type == STAR_RCVR) && (star_type == NEC_TYPE)) break;
if(rcvr_type == ACRON_RCVR) break;
   }
   check_com_timer();

   disable_kbd = old_disable_kbd;
}


int in_terminal;   // flag set if running debug terminal

S32 serve_gps(int why)  // called from either foreground loop or WM_TIMER (e.g., while dragging)
{
S32 i;
int served;
static int count;

   if(break_flag) return 0;        // ctrl-break pressed, so exit the program
   if(take_a_dump) return 1;
   if(in_terminal) return 1;

   while(1) {   
      served = 0;
      #ifdef WIN_VFX
         if(!timer_serve) {
            SAL_serve_message_queue();     // OK to re-enter serve_gps() via WM_TIMER invoked from this call...
            served = 1;
            Sleep(0);                      // (in fact, that will usually be the case) 
         }
      #endif
      #ifdef USE_X11
         get_x11_event();
         served = 1;
      #endif

      if((nav_rate > 10) && (got_timing_msg % 5) == 4) i = 0;
      else if(rcvr_type == NO_RCVR) i = SERIAL_DATA_AVAILABLE();  //rxx
      else if(process_com == 0) i = 0;
else if(1 && ((++count % 20) == 0)) {  // mainly for NEC_TYPE GPSDO and ACRON_RCVR
   if((rcvr_type == STAR_RCVR) || (rcvr_type == ACRON_RCVR)) {
      dup_star_time = 0;
      i = 0;
   }
}
      else i = SERIAL_DATA_AVAILABLE(); // check if serial port has data (and stay in loop until it's exhausted)

      if(i == 0) {    // no data available,  we are not too busy
         // no data available,  we are not too busy, or taking a breather from high nav-rate updates
         system_busy = 0; 
         check_com_timer();
         get_mouse_info();

         if(rcvr_type == NO_RCVR) ;  //rxx
         else if(com_data_lost == 1) {
            com_data_lost = 2;
            if(process_com) {
               vidstr(0,0, RED, "NO DATA SEEN ON COM DEVICE      ");
               ++restart_count;

               if(rcvr_reset || POLLED_RCVR || (rcvr_type == ZODIAC_RCVR)) {    // recover from hard reset
                  do_survey = 0;
                  rcvr_reset = 0;
                  tsip_sync = 0;
                  tsip_wptr = 0;
                  tsip_rptr = 0;
                  init_messages(0);
                  redraw_screen();
if(debug_file) fprintf(debug_file, "!!! DATA LOSS RESET: %d !!!\n", rcvr_reset);
               }
               refresh_page();
            }
            else {
               vidstr(0,0, RED, "COM DATA DEVICE UNAVAILABLE     ");
            }
         }

         if(rcvr_type == NO_RCVR) ;  //rxx
         else if(process_com == 0) refresh_page();
         break;
      }
      else {
         reset_com_timer();
      }

      if(((rcvr_type == NO_RCVR) && i) || (process_com && i)) {  // we have serial data to process  //rxx   
         get_rcvr_message();  // process the incoming GPS message bytes
         system_busy = 2;
      }

      if(get_mouse_info() == 0) {     // show queue data at the mouse cursor
         if(i == 0) refresh_page();   // keep refreshing page in case no serial data
      }
   }
   
   return 1;
}

void show_com_state()
{
   if(process_com) {       // we are using the serial port
      if((com_port == 0) && (usb_port == 0) && IP_addr[0]) {   // TCP
         sprintf(out, "WAITING FOR CONNECTION TO:%s", IP_addr);  
      }
      else if(nortel && com_port)   sprintf(out, "Waking up Nortel receiver on COM%d", com_port);
      else if(nortel && usb_port)   sprintf(out, "Waking up Nortel receiver on USB%d", usb_port);
      else if(com_port) sprintf(out, "NO COM%d SERIAL PORT DATA SEEN", com_port);
      else if(usb_port) sprintf(out, "NO USB%d SERIAL PORT DATA SEEN", usb_port);
      else              sprintf(out, "NO INPUT DEVICE FOUND");
      vidstr(0,0, RED, out);
      refresh_page();

//    set_single_sat(0x00);
      if(!did_init1) init_messages(3);     // send init messages and request various data

#ifdef USE_X11      
      while(0 && !SERIAL_DATA_AVAILABLE()) {  // !!!!
         Sleep(DEFAULT_SLEEP);
         check_com_timer();
         get_x11_event();
         if(display) XFlush(display);
         if(KBHIT()) {
            break;
         }
      }
#endif
      if(SERIAL_DATA_AVAILABLE()) {
         reset_com_timer();
         find_msg_end();  // skip data till we see an end of message code
      }
   }
   else {                  // user has disabled the serial port
      vidstr(0,0, YELLOW, "SERIAL PORT PROCESSING DISABLED");
      refresh_page();
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
            force_adev_redraw();    // and make sure adev tables are showing
         #endif
         pause_data = user_pause_data^1;
      }
      first_key = 0;
      prot_menu = 0;
      draw_plot(1);
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
int term_row;      // cursor position
int term_col;
int term_changed;  // flag set if screen buffer has been updated
int term_header_lines;  // number of lines in the screen help header

int term_echo;     // if set, echo keyboard chars to screen
int term_pause;    // if set, stop updating srceen
int term_hex;      // if set, srceen is ascii hex dump mode
int term_crlf;     // if set, treat all CR or LF chars as CR LF


char term_kbd[MAX_TERM_COLS+1];  // keystroke buffer used to resend last kbd command
int tkbd_col;
int last_was_keybuf;
int blinker;            // used to blink keyboard cursor char


void show_term_screen()
{
int row;
int col;
int c;

   // copy the teminal screen image buffer to the screen

   erase_screen();
   for(row=0; row<term_rows; row++) {
      for(col=0; col<term_cols; col++) {
         c = term_screen[row][col];
         c &= 0xFF;
         if((col == term_col) && (row == term_row)) {  // blinking cursor
            if(blinker) dot_char(col*TEXT_WIDTH, row*TEXT_HEIGHT, TERM_CURSOR, WHITE);  // header lines
            else        dot_char(col*TEXT_WIDTH, row*TEXT_HEIGHT, TERM_CURSOR, BLACK);  // header lines
         }
         if(c == 0) break;

         if(row < term_header_lines) dot_char(col*TEXT_WIDTH, row*TEXT_HEIGHT, c&0x7F, WHITE);  // header lines
         else if(c & 0x80) dot_char(col*TEXT_WIDTH, row*TEXT_HEIGHT, c&0x7F, YELLOW);   // user input chars
         else dot_char(col*TEXT_WIDTH, row*TEXT_HEIGHT, c&0x7F, GREEN);  // received chars
      }
   }
   refresh_page();

   reset_com_timer();
   term_changed = 0;
}

void scroll_up()
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

   show_term_screen();
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

void echo_term(int i, int hex)
{
char s[32];
int color;
   // put a character into the terminal image buffer.  Handle CR and LF chars

   if(hex) {  // hex terminal display
      sprintf(s, "%02X", i);
      if     (i == pkt_end1)   color = 0x80;  // highlight possible packet boundaries
      else if(i == pkt_end2)   color = 0x80;
      else if(i == pkt_start1) color = 0x80;
      else if(i == pkt_start2) color = 0x80;
      else                     color = 0x00;

      term_screen[term_row][term_col+0] = s[0] | color;
      term_screen[term_row][term_col+1] = s[1] | color;
      term_screen[term_row][term_col+2] = ' ';
      term_screen[term_row][term_col+3] = 0;
      term_col += 3;
      term_changed = 1;

      if(SCREEN_WIDTH < (36*3*TEXT_WIDTH)) {  // small screen - 16 vals per line
         if(term_col > (16*3)) {
            term_col = 0;
            scroll_up();
         }
      }
      else { // wide screen
         if(term_col == (16*3)) { // two groups of 16 vals per line
            term_screen[term_row][term_col++] = ' ';
            term_screen[term_row][term_col] = 0;
         }

         if(term_col > (16*6+1)) {
            term_col = 0;
            scroll_up();
         }
      }
   }
   else if((i & 0x7F) == 0x0D) {  // carriage return
      term_col = 0;
      if(term_crlf) scroll_up();
   }
   else if((i & 0x7F) == 0x0A) {  // line feed
      if(term_crlf) term_col = 0;
      scroll_up();
   }
   else if(((i & 0x7F) >= ' ') && ((i & 0x7F) < 0x7F)) { // add char to terminal image buffser
      term_screen[term_row][term_col] = (i & 0xFF);
      term_screen[term_row][term_col+1] = 0;
      term_changed = 1;

      if(++term_col >= term_cols) {
         term_col = 0;
         scroll_up();
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

void set_term_header()
{
   // setup the terminal screen header lines

                  strcpy(&term_screen[0][0], "=========================================================================");
                  strcpy(&term_screen[1][0], "   Lady Heather Terminal   END to exit   HOME to clear   F8 send break");
   if(term_pause) strcpy(&term_screen[2][0], "        F4 resume output   ");
   else           strcpy(&term_screen[2][0], "         F4 pause output   ");
   if(term_echo)  strcat(&term_screen[2][0], "F1 echo off   ");
   else           strcat(&term_screen[2][0], "F1 echo on    ");
   if(raw_file)   strcat(&term_screen[2][0], "F2 log off      ");
   else           strcat(&term_screen[2][0], "F2 log append   ");
   if(term_hex)   strcat(&term_screen[2][0], "F3 ASCII mode ");
   else           strcat(&term_screen[2][0], "F3 HEX mode   ");
   strcpy(&term_screen[3][0], "=========================================================================");
   strcpy(&term_screen[4][0], "");

   term_header_lines = 4+1;
}

void do_term()
{
int i;
double old_cto;

   // Interactive video terminal.  (added mainly to play with SCPI receivers)

   if(enable_terminal == 0) return;

   old_cto = com_timeout;
   com_timeout = 250.0;
   in_terminal = 1;

   tkbd_col = 0;
   term_kbd[tkbd_col] = 0;
   last_was_keybuf = 0;

   term_echo = 1;   // echo input to the screen (in yellow)
   term_hex = 0;    // ascii mode
   term_pause = 0;  // send all data to the screeb
   term_header_lines = TERM_HEADER_LINES;  // length of the header area

   erase:
   init_term();

   new_header:
   set_term_header();

   term_row = term_header_lines; // start at top of screen
// term_row = term_rows-1;       // start at the screen bottom
   term_col = 0;
   show_term_screen();  // draw the screen image

   while(1) {    // the main processing loop...  repeat until exit requested
      #ifdef WIN_VFX
         SAL_serve_message_queue();
         Sleep(0);
      #endif

      #ifdef USE_X11
         get_x11_event();
if(display) XFlush(display);
      #endif

      if(KBHIT()) {      // key pressed 
          i = get_kbd(); // get the keyboard character

          if     (i == END_CHAR)  break;        // exit the terminal
          else if(i == HOME_CHAR) goto erase;   // erase screen
          else if(i == F8_CHAR)   SendBreak();  // send a break
          else if(i == F1_CHAR) {               // toggle echo
             term_echo ^= 1; 
             goto new_header;
          }
          else if(i == F2_CHAR) {               // toggle log
             if(raw_file) {
                fclose(raw_file);
                raw_file = 0;
                log_stream &= (~0x02);
             }
             else {
                raw_file = topen(raw_name, "ab");
                log_stream |= 0x02;
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
             for(i=0; i<tkbd_col; i++) {
                eom_flag = 1;
                sendout(term_kbd[i]);
                echo_term(term_kbd[i] | 0x80, 0);
             }
             last_was_keybuf = 1;
          }
          else {
             eom_flag = 1;        // send keyboard char to receiver
             sendout(i);

             if(i == 0x08) {      // backspace key pressed
                if(tkbd_col) {    // remove keystroke from keyboard buffer
                   --tkbd_col;
                }
                term_kbd[tkbd_col] = 0;
                last_was_keybuf = 0;

                if(term_col) --term_col; // remove from screen image
                term_screen[term_row][term_col] = 0;
                show_term_screen();
             }
             else if(i == 0x0D) { // carriage return
                term_col = 0;
                scroll_up();
                if(rcvr_type != STAR_RCVR) last_was_keybuf = 1;
                else save_term_kbd(i);
             }
             else {
                save_term_kbd(i);  // save last keystroke in replay buffer
                i |= 0x80;         // echo input in yellow
                if(term_echo) echo_term(i, 0);  // echo keystroke if echo enabled
             }
          }
      }
      else if(SERIAL_DATA_AVAILABLE()) {  // we have a char from the receiver
         i = get_com_char();       // get the char (and write to raw file if opened)
         if(term_pause == 0) {     // send it to the screen unless screen is paused
            echo_term(i, term_hex);
         }

      }
      else {  // nothing going on, Take a short snooze
         Sleep(10);
      }

      check_com_timer();    // update screen every 250 msec
      if(com_data_lost) {   // time to copy screen buffer to the screen
         blinker ^= 1;      // blinking cursor
         show_term_screen();
         reset_com_timer();
      }
   }

   com_timeout = old_cto;
   reset_com_timer();
   in_terminal = 0;

   if(rcvr_type == SCPI_RCVR) {
      last_com_time = 0;   // force com timeout re-init
   }

   erase_screen();
   sprintf(out, "Restarting... this may take around %d seconds...", (int) (com_timeout/1000.0));
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
   if(init) init_messages(4);

   row = 2;
   col = 0;
   while(1) {
#ifdef WIN_VFX
      if(1 || init) SAL_serve_message_queue();
      Sleep(0);
#endif
#ifdef USE_X11
      get_x11_event();
if(display) XFlush(display);
#endif
      if(SERIAL_DATA_AVAILABLE()) {
         reset_com_timer();
         i = get_serial_char();
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
         check_com_timer();
      }
#ifdef WIN_VFX
      if(1 || init) SAL_serve_message_queue();
      Sleep(0);
#endif
#ifdef USE_X11
      get_x11_event();
if(display) XFlush(display);
#endif
      if(KBHIT()) {
         i = GETCH();
         if(i == 0x1B) {
            clean_up();
            error_exit(6666, "User pressed ESC during dump_strem()");
         }
         else goto again;
      }
   }
}


void do_gps()
{
int i;
int mach;

   mach = 0;
   #ifdef __MACH__
      mach = 0;     // !!!! testing
   #endif

   while(1) {    // the main processing loop...  repeat until exit requested
#ifdef USE_X11
      if(have_restore_size == 0) {
         restore_width = SCREEN_WIDTH;
         restore_height= SCREEN_HEIGHT;
         have_restore_size = 1;   // size came from initial screen init
      }

      if(need_resize && (this_button == 0) && (last_button == 0)) {
printf("RESIZE to %dx%d  now:%dx%d  have_root:%d  mach:%d\n", new_width,new_height, SCREEN_WIDTH,SCREEN_HEIGHT, have_root_info, mach); // zork - show_debug_info
         need_resize = 0;
         if(new_width < MIN_WIDTH) new_width = MIN_WIDTH;
         if(new_height < MIN_HEIGHT) new_height = MIN_HEIGHT;
         if(mach || (new_width != SCREEN_WIDTH) || (new_height != SCREEN_HEIGHT)) {
            if(have_root_info && ((unsigned)(new_width+2) < display_width) && ((unsigned) (new_height+0) < display_height)) { // new window size is not near the screen size
               restore_width = new_width;
               restore_height = new_height;
               have_restore_size = 3;  // size came from resize event
               x11_maxed = 0;
printf("new restore size: %dx%d\n", restore_width,restore_height); // zork - show_debug_infi
            }

            SCREEN_WIDTH = custom_width = new_width;
            SCREEN_HEIGHT = custom_height = new_height;
if(1 || show_debug_info) printf("Resizing to %d x %d.\n", SCREEN_WIDTH,SCREEN_HEIGHT); // zork - show_debug_info
            screen_type = 'c';
//          strcpy(edit_buffer, "c");
            sprintf(edit_buffer, "%dx%d",SCREEN_WIDTH,SCREEN_HEIGHT);
            if(display) XFlush(display);
            edit_screen_res();
         }
      }
#endif  // USE_X11


      if(first_key) ;         // dont redraw screen while keyboard menu active
      else if(need_redraw) {
         redraw_screen();
      }

      if(!serve_gps(0)) {
         break;
      }
      got_timing_msg = 0;  // used to prevent keyboard lockout if data comming in at high nav rates

      if(f11_flag) {  //// !!!! debug
//       BEEP(11);
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
      else if(KBHIT()) {   // key pressed 
         system_busy = 4;
         i = get_kbd();    // get the keyboard character
         i = do_kbd(i);    // process the character
         if(i) break;      // it's time to stop this madness
      }
      else {  // sleep a while when we are not busy to keep cpu usage down
         #ifdef WINDOWS
            if(idle_sleep && (set_system_time == 0)) Sleep(idle_sleep);
         #else  // __linux__  __MACH__ 
            if(idle_sleep && (set_system_time == 0)) Sleep(idle_sleep);
         #endif
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

      if(!GetCommModemStatus(hSerial, &dwModemStatus)) {
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


void check_com_timer()
{
double to;

   update_pwm();
   to = last_com_time + com_timeout;
   if(to < this_msec) {  // com data lost
      last_com_time = this_msec;
      if(com_data_lost == 1) com_data_lost = 2;
      else                   com_data_lost = 1;
   }
}

void reset_com_timer()
{
   update_pwm();
   last_com_time = this_msec;
   com_data_lost = 0;
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
#else  // __linux__  __MACH__
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


int main(int argc, char *argv[])
{
   config_program(argc, argv);  // process command line arguments

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

   if(need_debug_log) {
      need_debug_log = 0;
      if(debug_file) fclose(debug_file);
      debug_file = 0;
      debug_name[0] = 0;

      sprintf(out, "debug.log");
      open_debug_file(out);
   }

   if(need_raw_file) {
      need_raw_file = 0;
      if(raw_file) fclose(raw_file);
      raw_file = 0;
      sprintf(raw_name, "%s.raw", "heather");
      raw_file = topen(raw_name, "wb");
      if(raw_file) log_stream |= 0x02;
   }

   if(detect_rcvr_type) {
      auto_detect();
   }

   plot_axes();       // draw graph axes
   if(luxor) reset_luxor_wdt(1);
   show_version_header();
   show_log_state();  // show logging state
   show_com_state();  // show com port state and flush buffer
   get_log_file();    // read in any initial log file

   do_term();         // debug terminal 

   do_gps();          // run the receiver until something says stop 

   clean_up();        // leave the receiver in a nice stand-alone state
   shut_down(0);      // clean up remaining details and exit
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

   switch (message)
      {
      case WM_KEYDOWN:
         {
         switch (wParam)
            {
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
   //
   // Initialize system abstraction layer -- must succeed in order to continue
   //

   VFX_io_done = 1;
   hInstance = hInst;

   IPC = NULL;   // TCP

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
   switch (message)
      {
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

         switch (ID)
            {
            case IDOK:
               {
               EndDialog(hDlg, 1);
               if(downsized) do_fullscreen();
               return TRUE;
               }

            case IDREADME:
               {
               EndDialog(hDlg, 0);
               if(downsized) do_fullscreen();
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

// need to add:  /tsc /aj /it commands !!!!!!!!

   if(show_escape) return;  // user interrupted the show with ESC

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
      zoom_screen = 0;

      if(help_line > (PLOT_TEXT_ROW-4)) {
         if(edit_error("") == 0x1B) show_escape = 1;
         erase_screen();
         help_line = 1;
      }
      else if(help_line == 0) erase_screen();
      vidstr(help_line, 0, WHITE, s);

      ++help_line;
      text_mode = old_text_mode;
      zoom_screen = old_zoom;
      PLOT_TEXT_ROW = old_plot_text_row;
      EDIT_ROW = old_edit_row;
   }
   else printf("%s\n", s);
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
//             erase_plot(1);
//             edit_error("Sorry, startup command line help dialog is not available in fullscreen mode");
//             showing_help = 0;
//             return;
          }
       }
    }
#endif

    static char help_msg[32768] = {
         "Lady Heather's GPS Disciplined Oscillator Control Program\r\n"
         "Version "VERSION" - "__DATE__" "__TIME__"\r\n"
         "\r\n"
         "Copyright (C) 2008-2016 Mark S. Sims - Released under MIT License.\r\n"
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
#else //__linux__  __MACH__
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

#ifdef TCP_IP
         "   /ip=addr[:port#] - connect to TCP/IP server instead of local COM port\r\n"
#endif

         "   /a[=#]           - number of points to calc Adevs over (default=330000)\r\n"
         "                      If 0,  then all adev calculations are disabled.\r\n"
         "   /b[=#]           - set daylight savings time area (1=USA,2=EURO,3=AUST,4=NZ)\r\n"
         "   /b=nth,start_day,month,nth,end_day,month,hour - set custom DST rule\r\n"
         "                      day:0=Sun..6=Sat  month:1=Jan..12=Dec\r\n"
         "                      nth>0 = from start of month  nth<0 = from end of month\r\n"
         "   /br[=#]          - set serial port configuration. (default 9600:8:n:1)\r\n"
         "   /bs              - set time display to solar time\n"
         "   /bt              - start up in terminal emulator mode\r\n"
         "   /c[=#]           - set Cable delay to # ns (default=77.03 ns (50 ft std coax))\r\n"
         "   /c=#f              set Cable delay in feet of 0.66Vp coax\r\n"
         "   /c=#m              set Cable delay in meters of 0.66Vp coax\r\n"
//       "                      use NEGATIVE values to compensate for antenna cable delays!\r\n"
#ifdef GREET_STUFF
         "   /d#              - show dates in calendar #\r\n"
         "                      A)fghan   haaB)     C)hinese  D)ruid   H)ebrew\r\n"
         "                      I)slamic  J)ulian   K)urdish  M)jd    iN)dian\r\n"
         "                      P)ersian  iS)o      T)zolkin  boliV)isn\r\n"
         "                      X)iuhpohualli       maY)an    aZ)tec Tonalpohualli\r\n"
#endif
         "   /de[=#]          - set debug information level\r\n"
         "   /dl[=file]       - write debug information log file\r\n"
         "   /dr[=file]       - write raw receiver data capture file\r\n"
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
         "   /h=file          - read command line options from .CFG config file\r\n"
         "   /i[=#]           - set plot Interval to # seconds (default=24 hour mode)\r\n"
         "   /ir              - set read-only mode for receiver commands\r\n"
         "   /is              - set read-only mode for serial port\r\n"
         "   /it              - set no-pollng mode for receiver data\r\n"
         "   /j[=#]           - set ADEV sample period to # seconds (default=10 seconds)\r\n"
//       "   /jp              - enable JPL wall clock mode on start-up\r\n"
         "   /kb              - toggle Beep sounds\r\n"
         "   /kc              - toggle writes to Config EEPROM\r\n"
         "   /ke              - toggle quick exit with ESC key\r\n"
         "   /kj              - toggle showing sun and moon in satellite maps\r\n"
         "   /km              - toggle mouse enable\r\n"
         "   /kq              - toggle Keyboard enable\r\n"
         "   /ks              - toggle Sound files\r\n"
         "   /kt              - toggle Windows dialog/message timer\r\n"
         "   /k?[=#]          - set temp control PID parameter '?'\r\n"
         "   /l[=#]           - write Log file every entry # seconds (default=1)\r\n"
         "   /lc              - don't write any comments in the log file\r\n"
         "   /ld              - write signal level comments in the log file\r\n"
         "   /lo              - enable reading of old format log files\r\n"
         "   /lh              - don't write timestamp headers in the log file\r\n"
         "   /ls              - change log file value separator from tab to a comma\r\n"
         "   /m[=#]           - Multiply all plot scale factors by # (default is to double)\r\n"
         "   /ma              - toggle Auto scaling\r\n"
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
         "   /nd=hh:mm:ss     - dump screen at specified time (optional: /n=month/day/year)\r\n"
         "   /nd=#?           - dump screen every #s secs,  #m mins,  #h hours  #d=days\r\n"
         "   /nd=#?o          - dump screen Once in #so secs,  #mo mins,  #ho hours  #d=days\r\n"
         "   /nl=hh:mm:ss     - dump log at specified time (optional: /n=month/day/year)\r\n"
         "   /nl=#?           - dump log every #s secs,  #m mins,  #h hours  #d=days\r\n"
         "   /nl=#?o          - dump log Once in #so secs,  #mo mins,  #ho hours  #d=days\r\n"
         "   /nr[=#]          - force receiver navigation rate to # Hz (default=1)\r\n"
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
         "   /q[=#]           - set size of plot Queue in seconds (default=3 days)\r\n"
         "   /qf[=#]          - set max size of FFT (default=4096)\r\n"
         "   /r[=file]        - Read file (default=tbolt.log)\r\n"
         "                      .log  ,xml  .gpx (log files)\r\n"
         "                      .scr=script   .lla=lat/lon/altitude\r\n"
         "                      .log=log   .scr=script   .lla=lat/lon/altitude\r\n"
         "                      .adv=adev  .tim=ti.exe time file\r\n"
         "   /ro[=#]          - add # seconds to the GPS receiver date/time\r\n"
         "                      or /ro says 1024 weeks,  /ro=2* says 2048 weeks, etc\r\n"
         "   /rs=file         - get input from raw receiver data simulation file\r\n"
         "   /rt[=#]          - use Resolution-T serial port config (9600,8,ODD,1)\r\n"
         "                      [#=1]=force Resolution-T  [#=2]force Resolution SMT\r\n"
         "   /rx#[=leapsecs]  - set receiver type # (A=Acron  C=UCCM  D=Datum  E=NEC  G=GPSD\r\n"
         "                      J=Jupiter  K=Z3811A  M=Motorola  N=NMEA  R=Resolution-T\r\n"
         "                      S=SIRF  T=TSIPU=Ublox  V=Venus  X=system clock  8=NVS\r\n"
         "                      Y=SCPI-(NORTEL)  Z=SCPI-(Z3801A)  5=SCPI-(Z3816A, HP53xxx)r\n"
         "                      For receivers that do not report a valid leapsecond\r\n"
         "                      count you can specify the value to use\r\n"
//       "   /sf              - enter 2D/3D fix mode and map fixes\r\n"
         "   /si[=#]          - set maximum number of displayed satellites to #\r\n"
         "   /sp[=#]          - do Precison Survey (# hours,  default=48/max=96)\r\n"
         "   /sr[=#]          - enable sunrise/set display\r\n"
         "   /ss[=#]          - do Self Survey (# fixes,  default=2000)\r\n"
         "   /st              - toggle drawing of satellite position trails\r\n"
         "   /ta              - show dates in European dd.mm.yy format.\r\n"
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
         "   /ti              - toggle 12/24 hour digital clock mode\n"
         "   /tj              - don't remove effects of tbolt firmware temperature smoothing\r\n"
         "   /tk              - show Kelvin temperatures\r\n"
         "   /tm              - show altitude in meters\r\n"
         "   /tn              - show Newton temperatures\r\n"
         "   /to              - show Romer temperatures\r\n"
         "   /tp              - show time as fraction of a day\r\n"
         "   /tq              - show time as total seconds of the day\r\n"
         "   /tr              - show Rankine temperatures\r\n"
         "   /ts[odhm]        - set operating system time to UTC (once,daily,hourly,every minute)\r\n"
         "   /tsa[=#]         - set operating system time to UTC anytime difference exceeds # msecs\r\n"
         "   /tsx[=#]         - compensate for delay between 1PPS output and receiver timing message\r\n"
         "                      Value is in milliseconds.  Default is a +45 millisecond delay\r\n"
         "   /tsj             - toggle show digital clock Julian date.time\r\n"
         "   /tsz             - toggle show digital clock with milliseconds\r\n"
         "   /tt=#            - set active temp control setpoint to # degrees C\r\n"
         "   /tu              - sync outputs to UTC Time (default is UTC time)\r\n"
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
         "   /vu              - Undersized (640x480) Video screen\r\n"
         "   /vs              - Small (800x600) Video screen\r\n"
         "   /vm              - Medium (1024x768) Video screen (default)\r\n"
         "   /vn              - Netbook (1000x540) Video screen\r\n"
         "   /vl              - Large (1280x1024) Video screen\r\n"
         "   /vv              - Very large (1440x900) Video screen\r\n"
         "   /vx              - eXtra large (1680x1050) Video screen\r\n"
         "   /vh              - Huge (1920x1080) Video screen\r\n"
         "   /vc=colsXrows    - custom screen size (e.g. /vc=1200x800)\r\n"
         "   /vf              - start in Fullscreen mode\r\n"
         "   /vi              - invert black and white on screen\r\n"
         "   /wa=file         - set log file name to Append to (default=tbolt.log)\r\n"
         "   /w=file          - set log file name to Write (default=tbolt.log)\r\n"
         "   /x=#             - set experimental oscillator disiplining PID value\r\n"
         "   /y               - optimize plot grid for 24 hour display (/y /y = 12hr)\r\n"
         "   /y=#             - set plot view time to # minutes/division\r\n"
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
         BEEP(12);
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
#else // __linux__ __MACH__  show help as simple text screens
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
      BEEP(12);
      help_line = 999;
      show("");
      redraw_screen();
   }
   else {
      shut_down(999);
   }
#endif // WINDOWS

   showing_help = 0;
}

