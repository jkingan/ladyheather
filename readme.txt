Lady Heather Version 5.0 (with Linux and macOS (OS/X) support)

To use this program on Linux and macOS:

1) Create a directory to install it in and switch to that directory.

2) Unzip the zip file into the directory

   In the X11 distribution .ZIP file is a file "heathosx".  This is a
   pre-compiled executable for macOS (aka OS/X) (tested under El Capitan).
   If you are using macOS install the XQuartz package from xquartz.org.
   Then, from a terminal window:
       unzip -q heatherx11.zip
       cd heatherx11
       mv heathosx heather
       chmod +x heather
       xattr -d com.apple.quarantine heather

   Double-click the "heather" file to launch the program (depending upon your 
   system settings you might need to right-click it and select OPEN to allow it 
   to run)

   If you don't rename the file, Heather will attempt to use "heathosx.cfg" 
   and "heathosx.cal" for its' configuration and calendar files.  You could
   also just rename those files.


3) Make sure you have the g++ compiler and libx11-dev packages installed:
      sudo apt-get install g++
      sudo apt-get install libx11-dev

   For Fedora Linux edit the makefile and change "OS = g++" to "OS = gpp".
   Also the X11 development package is called libX11-devel (with a capital 'X')

   For macOS you will need to install the XQuartz package from XQuartz.org)
   (note: their installer time remaining message is rather optimistic)
   You will also need to install XCode from Apple (WARNING: it's HUGE).

4) Compile the code (the make file auto-detects Linux or macOS):
      make clean
      make

5) Edit the heather.cfg file to suit your needs.  It sets the values of the
   initial command line options to use.  Place one command line option per
   line, starting in column one.  If a line does not start with a '/' or 
   a '-', the line is considered a comment. You can also use '@' lines in
   a .cfg file to send what are normally keyboard only commands.  All the '@'
   lines are written to a temporary file which is then processed as a keyboard
   .SCRipt file once Heather has finished initializing the hardware.

6) Run it:
   ./heather -1u             (for ttyUSB0 serial input)
   ./heather -1              (for ttyS0 serial input)
   ./heather -id=device_name (for using a non-standard serial device name)
   ./heather -ip=addr:port   (for internet connected server)

   (macOS use /dev/tty.usbserial# for its USB serial port identifiers and 
    /dev/ttys# for hardware serial ports)

   You can use a different digit for the -1 or -1u options if your are using
   a different serial device.  

   Note: THE NUMBER THAT HEATHER USES FOR THE SERIAL DEVICE IS ONE GREATER THAN
   THE OPERATING SYSTEM DEVICE DEVICE NUMBER.  Sorry for the kludge... Heather
   has its roots in Windows where com port numbers start from one and Heather
   used port 0 to indicate that no serial port was to be used.


   To run heather on Linux, the user must have access permissions
   for the serial device.  You can run Heather with sudo or
   (much better idea) give the user access to the "dialout" group:
       sudo usermod -a -G dialout user_name
   You must reboot or log out and log back in for usermod to take effect.
   You only need to run usermod once. Its setting is persistent between
   boots.  Some Linux distros use the "uucp" group instead of "dialout".

   Under macOS, serial port device driver installers usually ask what users
   to give serial port access permission to.
   

Known issues:
   With some (older) Linux distros you may need to add a -lrt option to the
   linker command in the makefile.  This links in the the library for getting
   and setting the system clock.  Most distros have these functions in the
   standard C library. 

   For the system clock setting functions to work, you need to have
   root permissions.  Since most Linux boxes have NTP running, you
   probably should not be using the system time setting commands, anyway.
   If you do a TS command from the keyboard, you should hear a beep (after a
   few seconds) if the clock set command worked.

   On macOS, if you go to full screen mode by clicking the maximize button,
   clicking it again won't usually restore the window size to its previous
   value.  Also when resizing the window by dragging the lower right corner of
   the window,  make sure the mouse cursor has "caught up" with the window 
   corner before releasing the mouse button. Window resizing under XQuartz is
   a bit laggy,

   Beware of cheap Chinese USB-Serial converters.  They often use iffy "clone"
   chips that may not be compatible with standard Linux drivers.  For example, 
   a lot of "CH3xxx" clones do not seem to let you set the baud rate under 
   Linux when using the drivers shipped with the Linux distro.





