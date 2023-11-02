#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>
#include <math.h>


#ifdef __MACH__        // Mac OS X (aka macOS)
   #include <stdlib.h>
#else
   #include <malloc.h>
#endif

#ifdef __GNUC__
   // gcc compiler...
   // If you are using gcc on a Windows box, you need to undefine PENGUIN
   // to use the proper file name format and maybe the SWITCH_CHAR
   // command line option switch flag
   #define GCC
#ifndef USE_SDL
   #define USE_X11
#endif
#endif
#ifdef __GNUG__
   // g++ compiler...
   // If you are using gcc on a Windows box, you need to undefine PENGUIN
   // to use the proper file name format and maybe the SWITCH_CHAR
   // command line option switch flag
   #define GCC
#ifndef USE_SDL
   #define USE_X11
#endif
#endif
#ifdef GCC
   #define stricmp   strcasecmp    // case insensitive string comparison
   #define strnicmp  strncasecmp
   void strupr(char *);
   void strlwr(char *);
#endif

#ifdef __MACH__               // Mac OS X
   #include <mach/clock.h>
   #include <mach/mach.h>
   #include <mach-o/dyld.h>
#ifndef USE_SDL
   #define USE_X11
#endif
#endif


#ifdef _MSC_VER
  #define WINDOWS     // #define this for WIN32 environment
#endif

#define VERSION "6.14.1 Beta"

//
//   "Magic" numbers (mostly used as function args and returns)
//

#define BIG_NUM 1.0E29           // a large number

#define BROADCAST_5A   ((res_t == RES_T) ? 0x01 : 0x00)  // TSIP_RCVR: 0x01=broadcast packet 5A, 0x00=poll for packet 5A
#define DOPPLER_SMOOTH 0x02      // TSIP_RCVR: 0x02=doppler smoothed code phase  0x00=raw code phase

#define DATA_LOSS_REINIT   1     // com port data lost, OK to re-init receiver
#define DATA_LOSS_NO_INIT  2     // com port data lost, don't try to re-init receiver

#define RESET_ADEV_Q     0x01    // queue idenyifiers (for resetting queues)
#define RESET_PLOT_Q     0x02
#define RESET_MTIE_Q     0x04
#define RESET_ALL_QUEUES (RESET_PLOT_Q | RESET_ADEV_Q | RESET_MTIE_Q)
EXTERN int dont_reset_queues;    // used in TICC autotune mode
EXTERN int dont_reset_phase;     // used when reloading adev queue data

#define TRIM_Q_START     0x01    // use by trim_plot_queue
#define TRIM_Q_END       0x02

#define STOP_AT_Q_END    0       // stop searching queue at last saved point
#define STOP_AT_PLOT_END 1       // stop seaching plot queue at last displayed point

#define REVIEW_QUIET     0       // disable beeps in plot review mode
#define REVIEW_BEEP      1       // allow beeps in plot review mode

#define NO_REFRESH       0       // don't refresh screen when plot updates  NO_REFRESH MUST be 0
#define REFRESH_SCREEN   1       // refresh screen when plot updates

#define FULL_ERASE_OK    (-1)    // erase the full screen plot area disabled
#define ERASE_GRAPH_AREA (0)     // erase only the graph area
#define ERASE_PLOT_AREA  (1)     // erase the whole plot area (up to any shared plot)

#define LOG_HEX_STREAM   0x01    // log receiver data as formatted ascii hex packets
#define LOG_PACKET_ID    0x02    // log TSIP packet names
#define LOG_PACKET_START 0x04    // log TSIP packet starts
#define LOG_RAW_STREAM   0x08    // log receiver data as raw binary
#define LOG_SENT_DATA    0x10    // log packets sent
#define ECHO_RCVR_DATA   0x100   // echo receiver data to an external device
#define ECHO_TICC_DATA   0x200   // echo TICC data to an external device

#define ZOOM_SLEEP       100     // msecs to sleep after each line in ZH zoom mode



//  Values less than 0x10000 originate from TSIP receiver messages, don't change them
//  If you add any alarm values, update write_log_changes() in heathmsc.cpp
#define CRIT_ROM          0x0001 // critcal_alarms
#define CRIT_RAM          0x0002
#define CRIT_PWR          0x0004
#define CRIT_FPGA         0x0008
#define CRIT_GPS          0x0008
#define CRIT_OCXO         0x0010
#define CRIT_RTC          0x0010
#define CRIT_ALL          0xFFFF

#define MINOR_OSC_AGE     0x0001 // minor_alarms
#define MINOR_ANT_OPEN    0x0002
#define MINOR_ANT_SHORT   0x0004
#define MINOR_ANT_NO_PWR  (MINOR_ANT_OPEN | MINOR_ANT_SHORT)
#define MINOR_NO_TRACK    0x0008
#define MINOR_OSC_CTRL    0x0010
#define MINOR_SURVEY      0x0020
#define MINOR_NO_POSN     0x0040
#define MINOR_LEAP_PEND   0x0080
#define MINOR_TEST_MODE   0x0100
#define MINOR_BAD_POSN    0x0200
#define MINOR_EEPROM      0x0400
#define MINOR_ALMANAC     0x0800
#define MINOR_PPS_SKIPPED 0x1000
#define MINOR_UNKNOWN     0xE000
#define MINOR_ALL         0xFFFF

//  Additional alarm flags
#define MINOR_JL_PHASE    0x10000
#define MINOR_JL_RUNTIME  0x20000
#define MINOR_JL_HOLD     0x40000
#define MINOR_JL_FREQ     0x80000
#define MINOR_JL_CHANGE   0x100000
#define MINOR_DISCIPLINE  0x200000
#define MINOR_WARMUP      0x400000
#define MINOR_INACTIVE    0x800000
#define MINOR_HOLDOVER    0x1000000
#define MINOR_PLL         0x2000000
#define MINOR_JAMMING     0x4000000

#define GPS_FIXES         0x00   // gps_status
#define GPS_NO_TIME       0x01
#define GPS_NO_FIXES      0x02
#define GPS_PDOP_HIGH     0x03
#define GPS_NO_SATS       0x08
#define GPS_ONE_SAT       0x09
#define GPS_TWO_SATS      0x0A
#define GPS_THREE_SATS    0x0B
#define GPS_SAT_UNAVAIL   0x0C
#define GPS_TRAIM_ERR     0x10
#define GPS_UNK_STATUS    0xFF

#define DIS_LOCKED        0      // discipline (TSIP receiver status variable)
#define DIS_WARMING       1
#define DIS_FLOCK         2
#define DIS_PLACING       3
#define DIS_FINIT         4
#define DIS_COMP          5
#define DIS_OFF           6
#define DIS_RECOVERY      8

#define TFLAGS_NULL       0x0000 // time_flags - bit values match TSIP message values - don't change!
#define TFLAGS_UTC        0x0001
#define TFLAGS_INVALID    0x0004
#define TFLAGS_NO_UTC_OFS 0x0008
#define TFLAGS_USER_TIME  0x0010
#define TFLAGS_GLONASS    0x0010
#define TFLAGS_BEIDOU     0x0020
#define TFLAGS_GALILEO    0x0030
#define TFLAGS_GNSS_MASK  0x0030
#define TFLAGS_RTC        0x0100
#define TFLAGS_BAD_TIME   (TFLAGS_INVALID | TFLAGS_NO_UTC_OFS)

#define TMODE_GPS         0x00   // timing_mode
#define TMODE_UTC         0x03
#define TMODE_GLONASS     0x08   // ACU_360  ACU_GG
#define TMODE_SU          0x80   // eSIP only

#define RATE_1PPS         2      // pps_rate and set_pps_mode()
#define RATE_100PPS       0x82
#define RATE_PP2S         0x82
#define RATE_NO_TOD       0x82
#define RATE_1000PPS      0x83
#define RATE_10000PPS     0x83
#define RATE_USER         0
#define RATE_SKIP_PPS     0x80   // (res_t PP2S mode bit mask)

#define ALARM_DATE        0x01   // sound alarm at a specific date and time
#define ALARM_TIME        0x02   // sound alarm at a specific time
#define ALARM_TIMER       0x04   // sound countdown timer alarm
#define ALARM_LUXOR       0x10   // sound luxor fault alarm
#define ALARM_SET         0x80   // used for maintaining alarm at a time (without a date)

#define HOLD_SEEN_HOLD    0x01   // holdover mode seen
#define HOLD_SEEN_REC     0x02   // recovery mode seen





#define GPS_EPOCH     2444244.5  // GPS time reference epoch (julian date)
#define LINUX_EPOCH   2440587.5  // Linux time reference epoch (julian date)
#define JD2000        2451545.0  // Jan 1, 2000 at noon
#define JD_MJD        2400000.5  // convert JD to MJD
#define ROLLOVER_YEAR 2018       // if GPS year is less than this, assume it's a GPS week rollover fault


#define DEBUG_TEMP_CONTROL   // define to enable debug messages
#define DEBUG_OSC_CONTROL
#define DEBUG_PLOTS

//
//  Note: these are legacy defines left over from the days of tiny memory
//        footprints.  Undefining an option may cause compile errors due
//        to unhandled dependencies, etc.
#define ADEV_STUFF     // define this to enable ADEV calculation and plotting
#define PRECISE_STUFF  // define this to enable precise lat/lon/alt code
#define DIGITAL_CLOCK  // define this to enable the big digital clock display
#define ANALOG_CLOCK   // define this to enable the analog clock display
#define AZEL_STUFF     // define this to enable azel map drawing
#define SAT_TRAILS     // define this to plot sat trails in the az/el map
#define GIF_FILES      // define this to enable GIF screen dumps
#define FFT_STUFF      // define this to enable FFT calculations
#define GREET_STUFF    // define this to show holiday greetings
#define TEMP_CONTROL   // define this to enable unit temperature control
#define OSC_CONTROL    // define this to enable unit oscillator control
// #define PLM         // define this for power line monitoring (doesn't work)

#ifdef AZEL_STUFF
   #define AZEL_OK 1
#else
   #define AZEL_OK 0
#endif

#define HT_SIZE  (1<<13)  // 13 bit GIF hash table size

#define PI             (3.1415926535897932384626433832795)
#define E_VALUE        (2.7182818284590)
#define RAD_TO_DEG     (180.0/PI)
#define DEG_TO_RAD     (PI/180.0)
#define SECS_PER_DAY   (24 * 60 * 60)
#define SQRT3          (1.732050808)
#define FEET_PER_METER 3.280839895
#define LG_PER_METER   7.1429     // linguini per meter
#define FLICKS_PER_SEC 705600000  // a flick is a unit of time used in video editing, etc
#define LIGHTSPEED     (299792.458)    /* speed of light, km/s */
#define ABS_ZERO       (-237.15)
#define KB             (1.380649e-23)    // Boltzmann's constant
#define KH             (6.62607015e-34)  // Plank's constant

// WGS84 ellipsoid constants
#define WGS84_A          6378137.0
#define WGS84_E          0.08181919084296430236105472696748
#define eccSquared       (WGS84_E * WGS84_E)
#define eccPrimeSquared  (eccSquared / (1.0 - eccSquared))
#define dScaleFactor     0.9996


#define IABS(x)        (((x)<0)?(0-(x)):(x))
#define ABS(x)         (((x)<0.0F)?(0.0F-(x)):(x))
#define DABS(x)        (((x)<0.0)?(0.0-(x)):(x))
#define ROUND(x,to)    ((((x)+(to)-1)/(to))*to)
#define DEBUGSTR(x)    vidstr(MOUSE_ROW-2,MOUSE_COL, RED, x)
#define DEBUGSTR2(x)   vidstr(MOUSE_ROW-3,MOUSE_COL, RED, x)
#define DEBUGSTR3(x)   vidstr(MOUSE_ROW-4,MOUSE_COL, RED, x)

#define IDLE_SLEEP 25  // default idle sleep time in msecs

#define DOUBLE_DATA    // define this for double precsion data queue entries

#ifdef DOUBLE_DATA
   #define DATA_SIZE double   // double precision data queue entries
   #define OFS_SIZE  double
#else
   #define DATA_SIZE float    // single precision data queue entries
   #define OFS_SIZE  double
#endif

//#define PLUS_MINUS  0xF1
#define PLUS_MINUS  0x1F
#define SQUARED     0xFD
#define DEG_CODE    0x01   // internal char code for degrees symbol

#define PROGMEM      // (used by original Atmel AVR vector char drawing routines)
#define VCHAR_W   8  // elemental width and height of vector character patterns
#define VCHAR_H   8
#define DOT_CHARS 0  // if 1, draw characters dot-by-dot using win_vfx font definitions



// to disable sun and moon displays #define SUN_PRN 0, MOON_PRN 0, SUN_MOON_PRN MAX_PRN
#define MAX_PRN  999            // max number of sat PRNs to use
#define SUN_PRN  (MAX_PRN+1)
#define MOON_PRN (MAX_PRN+2)
#define SUN_MOON_PRN MOON_PRN   // max PRN to display: treat moon as a satellite
//#define SUN_MOON_PRN SUN_PRN  // max PRN to display: don't treat moon as a satellite
EXTERN int no_sun_moon;         // flags to control drawing sun/moon in maps and watch
EXTERN int sun_moon_shown;


#define GPS     0x0001   // bit mask positions match Venus receivers
#define GLONASS 0x0002
#define GALILEO 0x0004
#define BEIDOU  0x0008
#define IRNSS   0x0010

#define IMES    0x2000
#define QZSS    0x4000
#define SBAS    0x8000
#define MIXED   (GPS | GLONASS | GALILEO | BEIDOU | SBAS | QZSS)
EXTERN int system_mask;  // used to filter messages for GNSS system type
EXTERN int gnss_mask;
EXTERN int have_gnss_mask;
EXTERN int default_gnss;
EXTERN int gnss_sat;

EXTERN char *unit_file_name;  // base name of files to write (based upon receiver type)


#ifdef WINDOWS
   #include <assert.h>
   #include <conio.h>
   #define WIN32_LEAN_AND_MEAN
   #ifdef PLM
      #define _WIN32_WINNT 0x0500  // needed for high-res timer
   #endif
   #define WINCON         // printf() goes to TTY console if compiled with JM's modified windows.h, no effect otherwise, please leave in place
   #include <windows.h>
   #include <winbase.h>
   #include <windowsx.h>
   #include <commdlg.h>
   #include <mmsystem.h>
   #include <shellapi.h>
   #include <shlobj.h>

   #include <winsock2.h>      // TCP/IP
   #include <ws2tcpip.h>
   #include <mswsock.h>

   #include "typedefs.h"

   #define KBHIT win_kbhit
   #define GETCH win_getch
   #define SERIAL_DATA_AVAILABLE(port) (check_incoming_data(port))

   #include "sal.h"
   #include "winvfx.h"

   #include "resource.h"

   #define unlink(s)       _unlink(s)
   #define strupr(s)       _strupr(s)
   #define strlwr(s)       _strlwr(s)
   #define strnicmp(a,b,c) _strnicmp((a),(b),(c))
   #define outp(a,b)       _outp((a),(b))

   #define COORD int
   #define u08 unsigned char
   #define s08 char
   #define u16 unsigned short
   #define s16 short
   #define u32 unsigned int
   #define s32 int
   #define u48 unsigned long long
   #define u64 unsigned long long
   #define s64 long long
   #define FP80 long double

   #define DEGREES       DEG_CODE  // special chars
   #define UP_ARROW      24
   #define DOWN_ARROW    25
   #define LEFT_ARROW    27
   #define RIGHT_ARROW   26
   #define UP_DOWN_CHAR  18
   #define CHIME_CHAR    13    // char to signal cuckoo clock on
   #define SONG_CHAR     14    // char to signal singing clock on
   #define ALARM_CHAR    15    // char to signal alarm or egg timer set
   #define DUMP_CHAR     19    // char to signal screen dump timer set

   #define TEXT_X_MARGIN (TEXT_WIDTH*2)
   #define TEXT_Y_MARGIN (TEXT_HEIGHT*1)

   EXTERN HINSTANCE hInstance;
   EXTERN HWND      hWnd;
   EXTERN char root[MAX_PATH+1];
   EXTERN char heather_path[MAX_PATH+1];
   EXTERN char heather_cfg_path[MAX_PATH+1];
   EXTERN int root_len;

   extern PANE *stage;
   EXTERN u08 VFX_io_done;
   EXTERN U32 palette[16];

   EXTERN u08 downsized;  // flag set if screen has been windowed to show help dialog
   EXTERN u08 sal_ok;     // flag set once hardware has been setup

   unsigned char get_serial_char(unsigned port);
   void add_kbd(int key);
   void flush_kbd(void);
   int win_kbhit(void);
   int win_getch(void);
   void refresh_page(void);
   void do_fullscreen(void);
   void do_windowed(void);

   #define TCP_IP         // enable TCP/IP networking
   #define WIN_VFX        // use VFX for screen I/O

// #define SIN_TABLES     // use sin/cos tables for drawing circles
   #define SIG_LEVELS     // track sig levels vs azimuth
   #define BUFFER_LLA     // save lla x/y plot data in a buffer for screen redraws
#else // __linux__  __MACH__  __FreeBSD__
   #include <stdint.h>
   #include <unistd.h>
#ifdef USE_SDL
   #include <SDL2/SDL.h>
   #include <SDL2/SDL_types.h>
   #include <SDL2/SDL_endian.h>
   #include <SDL2/SDL_keyboard.h>
   #include "SDL_gfxPrimitives.h"
#else
   #include <X11/Xlib.h>
   #include <X11/Xutil.h>
   #include <X11/Xos.h>
   #include <X11/Xatom.h>
   #include <X11/keysymdef.h>
   #include <X11/cursorfont.h>
#endif
   #include <time.h>
   #include <sys/time.h>
   #include <sys/types.h>
   #include <sys/stat.h>
   #include <termios.h>
   #include <sys/ioctl.h>
   #include <sys/socket.h>
   #include <netinet/in.h>
   #include <netdb.h>
   #include <libgen.h>

#ifndef USE_SDL
   #define USE_X11
#endif
   #define SIMPLE_HELP
   #define MAX_PATH 256+1

   #define COORD int
   #define u08 unsigned char
   #define s08 char
   #define u16 uint16_t
   #define s16 int16_t
   #define u32 uint32_t
   #define s32 int32_t
   #define DWORD int32_t
   #define S32 int32_t
   #define u48 uint64_t
   #define u64 uint64_t
   #define s64 int64_t
   #define FP80 long double

#ifdef USE_SDL
   EXTERN SDL_Surface *display;
   EXTERN SDL_Window  * ne_window;
   EXTERN SDL_Texture * ne_texture;
   EXTERN SDL_Renderer * ne_renderer;
   EXTERN u32 sdl_texture_width;
   EXTERN u32 sdl_texture_height;
   EXTERN u32 sdl_window_width;
   EXTERN u32 sdl_window_height;
   EXTERN u08 sdl_scaling;

   #define KBHIT sdl_kbhit
   #define GETCH sdl_getch
   #define SERIAL_DATA_AVAILABLE(port) (check_incoming_data(port))
   #define TCP_IP         // enable TCP/IP networking

   EXTERN void kill_screen(void);
   EXTERN int sdl_getch(void);
   EXTERN int sdl_kbhit(void);
   EXTERN u32 get_sdl_color(u08 color);
   EXTERN void set_sdl_color(u08 color);
   EXTERN int get_sdl_event();

#else
   #define KBHIT x11_kbhit
   #define GETCH x11_getch
   #define SERIAL_DATA_AVAILABLE(port) (check_incoming_data(port))
   #define TCP_IP         // enable TCP/IP networking
   #define X11_sleep 100  // msecs to sleep when X11 is started / shutdown / resized
#endif

   #define TRUE  1
   #define FALSE 0


   #define DEGREES       DEG_CODE  // special chars
   #define UP_ARROW      24
   #define DOWN_ARROW    25
   #define LEFT_ARROW    27
   #define RIGHT_ARROW   26
   #define UP_DOWN_CHAR  18
   #define CHIME_CHAR    13    // char to signal cuckoo clock on
   #define SONG_CHAR     14    // char to signal singing clock on
   #define ALARM_CHAR    15    // char to signal alarm or egg timer set
   #define DUMP_CHAR     19    // char to signal screen dump timer set

   #define TEXT_X_MARGIN (TEXT_WIDTH*2)
   #define TEXT_Y_MARGIN (TEXT_HEIGHT*1)

// #define SIN_TABLES     // use sin/cos tables for drawing circles
   #define SIG_LEVELS     // track sig levels vs azimuth
   #define BUFFER_LLA     // save lla x/y plot data in a buffer for screen redraws

   EXTERN char heather_path[MAX_PATH+1];
   EXTERN char root[MAX_PATH+1];
   EXTERN int root_len;

   EXTERN char x11_io_done;
   EXTERN u08 downsized;  // flag set if screen has been windowed to show help dialog

   #define RESTORE_MAXED_WINDOW 1 // if true, attempt to restore maximized window to its original size
   EXTERN int have_root_info;                // flag set if screen size is known
   EXTERN int have_restore_size;             // used to restore window from maximized state
   EXTERN int restore_width,restore_height;  // screen size in use before maximization
   EXTERN int x11_maxed;                     // flag set if window is maximized


#ifdef USE_X11
   EXTERN Display *display;  // the X11 display to use
   EXTERN Window screen;     // the X11 window to display
   EXTERN Pixmap pix_map;    // drawing pixmap (used to double buffer the drawing)
   EXTERN Pixmap icon_map;   // pixmap for window manager (minimized) icon hint
   #define win pix_map       // win is the drawing surface - either screen (direct drawing) or pix_map (for double buffering)
   EXTERN GC gc;             // X11 graphics context
   EXTERN int screen_num;
   EXTERN char *appname;
   EXTERN int x11_left;      // screen window top left corner
   EXTERN int x11_top;
#endif

   EXTERN struct termios oldtio,newtio; // serial port config structures

   EXTERN u32 palette[16];   // our color palette

   unsigned char get_serial_char(unsigned port);  // forward ref'd functions
   void add_kbd(int key);
   void flush_kbd(void);
   int x11_kbhit(void);
   int x11_getch(void);
   void refresh_page(void);
   void do_fullscreen(void);
   void do_windowed(void);
   void Sleep(int t);
   void outp(unsigned port, unsigned val);
   void set_x11_color(u08 c);
   int get_x11_event(void);
   void flush_x11(void);
   u32 RGB_NATIVE(int r, int g, int b);
#endif  // __linux__ (and OSX)


#ifdef SIN_TABLES   // if memory is available use tables to speed up circle drawing
   #define sin360(x) sin_table[((int)((x)+360))%360]
   #define cos360(x) cos_table[((int)((x)+360))%360]
   EXTERN float sin_table[360+1];
   EXTERN float cos_table[360+1];
#else               // to save memory, DOS does not use tables to draw circles
   #define sin360(x) ((float)sin((fmod((double)(x),360.0))/RAD_TO_DEG))
   #define cos360(x) ((float)cos((fmod((double)(x),360.0))/RAD_TO_DEG))
#endif


#define SLEN  250              // standard string length

#ifdef GREET_STUFF
// EXTERN char *moons[12+1][31+2];
   EXTERN char *moons[31+2];
#endif

#ifdef SIG_LEVELS
   #define MAX_AVG_COUNT 0.0F  // max number of az or el readings to average
                               // if <0.0, don't freshen signal averages
                               // if 0.0,  freshen signal averages once a day
                               // if >0.0, freshen signal averages every n readings

   EXTERN int max_sat_prn;      // largest sat PRN seen
   EXTERN int min_el[360+1];    // minimum sat elevation seen at each azimuth
   EXTERN int max_el[360+1];    // maximum sat elevation seen at each azimuth
   EXTERN float db_az_sum[360+1];
   EXTERN float db_weighted_az_sum[360+1];
   EXTERN float db_az_count[360+1];
   EXTERN float db_el_sum[90+1];
   EXTERN float db_el_count[90+1];
   EXTERN float db_3d_sum[360+1][90+1];
   EXTERN float db_3d_count[360+1][90+1];
   EXTERN float max_sat_db[MAX_PRN+2+1];   // +2 is for sun/moon
   EXTERN float max_sig_level;
   EXTERN float min_sig_level;
   EXTERN double sig_level_sum;     // calculates average sat signal level
   EXTERN double sig_level_count;
   EXTERN int min_sig_db;           // used to label signal level map
   EXTERN int sig_level_step;
#endif // SIG_LEVELS

#define MAX_LLA_SIZE 640 //500   // max size of LLA scattergram plot
#ifdef BUFFER_LLA
   EXTERN u08 lla_data[MAX_LLA_SIZE+1][MAX_LLA_SIZE+1];
#endif

// define the second to perform certain tasks at...
// ... so a bunch of stuff does not all happen at the same time
#define REINIT_SECOND       54   // when to re-init the X11 display
#define MOON_STUFF          48   // update moon info at 48 seconds
#define AZEL_UPDATE_SECOND  42   // update azel plot at 42 seconds
#define SCPI_STATUS_SECOND  32   // request SCPI receiver status at 32 seconds
#define SUN_SECOND          26   // update sun rise/set values
#define TRAIL_UPDATE_SECOND 22   // update azel trails at 22 seconds
#define MOON_SECOND         18   // update moon rise/set values
#define LEAP_SECOND         04   // time to play the leap second song
#define CUCKOO_SECOND       00   // cuckoo clock at 00 seconds

#define SYNC_SECOND         06   // sync cpu time to gps time at 06 seconds
#define SYNC_MINUTE         05   // ... 05 minutes
#define SYNC_HOUR           04   // ... 04 hours

#define GREET_SECOND        12   // update greetings at 00:00:12
#define GREET_MINUTE        00
#define GREET_HOUR          00

#define DELTAT_SECOND       16   // update delta_t at 00:00:16
#define DELTAT_MINUTE       00
#define DELTAT_HOUR         00

#define REINIT_INTERVAL   1.0    // re-init X11 displays every 1.0 days
#define REINIT_IDLE_TIME 10.0    // only do screen re-inits if kbd idle for 10 minutes
                                 // ... set to 0 to disable keyboard idle lockout

/* screen attributes for output messages */
#define BLACK        0
#define DIM_BLUE     1
#define DIM_GREEN    2
#define DIM_CYAN     3
#define DIM_RED      4
#define DIM_MAGENTA  5
#define BROWN        6
#define DIM_WHITE    7
#define GREY         8
#define BLUE         9
#define GREEN       10
#define CYAN        11
#define RED         12
#define MAGENTA     13
#define YELLOW      14
#define WHITE       15
#define BMP_YELLOW  16


#define PPS_ADEV_COLOR   BLUE      // graph colors
#define OSC_ADEV_COLOR   BROWN
#define CHC_ADEV_COLOR   CYAN
#define CHD_ADEV_COLOR   WHITE
#define ADEV_LABEL_COLOR GREEN     // adev plot label color

#define PPS_COLOR        MAGENTA
#define OSC_COLOR        WHITE
#define DAC_COLOR        GREEN
#define TEMP_COLOR       YELLOW

#define ONE_COLOR        CYAN      // extra and DEBUG_PLOTS
#define TWO_COLOR        DIM_WHITE  //BROWN
#define THREE_COLOR      BLUE
#define FOUR_COLOR       RED   //BROWN

#define FIVE_COLOR       DIM_BLUE
#define SIX_COLOR        DIM_RED
#define SEVEN_COLOR      DIM_GREEN
#define EIGHT_COLOR      BROWN        // DIM_RED
#define NINE_COLOR       DIM_MAGENTA  // DIM_WHITE

#define TEN_COLOR        WHITE        // DIM_MAGENTA
#define ELEVEN_COLOR     DIM_CYAN
#define TWELVE_COLOR     DIM_GREEN
#define THIRTEEN_COLOR   DIM_BLUE
#define FOURTEEN_COLOR   GREY         // DIM_WHITE    // DIM_MAGENTA
#define FIFTEEN_COLOR    CYAN

#define COUNT_COLOR      CYAN
#define CONST_COLOR      CYAN
#define SKIP_COLOR       RED       // timestamp error markers
#define HOLDOVER_COLOR   RED       // holdover/temp spike markers
#define MOUSE_COLOR      CYAN      // DOS mouse cursor
#define HELP_COLOR       WHITE     // help text
#define PROMPT_COLOR     CYAN      // string editing
#define STD_TIME_COLOR   BLUE      // time while in standard time
#define DST_TIME_COLOR   CYAN      // time while in daylight savings time
#define MARKER_COLOR     CYAN      // plot marker numbers
#define ALARM_COLOR      RED       // alarm clock color
#define TIMER_COLOR      YELLOW    // egg timer marker color
#define DOP_COLOR        WHITE     // dilution of precision info
#define OSC_PID_COLOR    CYAN      // external oscillator disciplining status
#define TITLE_COLOR      WHITE     // plot title color
#define LEVEL_COLOR      BLUE      // avg signal level vs azimuth

#define KBD_GRID_COLOR   YELLOW    // touch screen keyboard color scheme
#define KBD_FG_COLOR     WHITE
#define KBD_BG_COLOR     BLACK
#define KBD_TITLE_COLOR  CYAN
#define KBD_FLASH_TIME   70        // msecs to flash pressed key for

#define HIGHLIGHT_REF   1          // put > < ticks on plot center reference line


// Where to place the various information onto the screen.
// These are in character coordinates.
#define TIME_ROW 0          // time stuff
#define TIME_COL 0

#define VAL_ROW 0           // oscillator values stuff
#define VAL_COL 15

EXTERN int VER_ROW;         // version stuff
EXTERN int VER_COL;
#define VER_LEN (INFO_COL-VER_COL-1)


#define ADEV_ROW 0          // adev text tables
#define ADEV_COL 79

#define INFO_ROW 16         // status info
EXTERN int INFO_COL;

#define CRIT_ROW 0          // critical alarms
#define CRIT_COL INFO_COL   // ... show_satinfo() assumes CRIT_stuff is next to SAT_stuff

#define MINOR_ROW 6         // minor alarms
#define MINOR_COL INFO_COL

#define POSN_ROW 6          // lat/lon/alt info
#define POSN_COL 0

#define SURVEY_ROW 6        // self survey info / osc params
#define SURVEY_COL 22

#define DIS_ROW 6           // oscillator disciplining info
#define DIS_COL 40

#define SAT_ROW  10         // sat info (gets shifted down 2 more rows unless Ebolt at 800x600)
#define SAT_COL  0

EXTERN int FILTER_ROW;      // where to put the filter mode info
EXTERN int FILTER_COL;

EXTERN int MOUSE_ROW;       // where to put the data at the mouse cursor
EXTERN int MOUSE_COL;

#define ALL_ROW (TIME_ROW+7)  // for showing all adevs

#ifdef WINDOWS
  #define HELP_COL (PLOT_LEFT/TEXT_WIDTH)
  #define HELP_ROW (PLOT_TEXT_ROW+1)
#else // __linux__  __MACH__  __FreeBSD__
  #define HELP_COL (PLOT_LEFT/TEXT_WIDTH)
  #define HELP_ROW (PLOT_TEXT_ROW+1)
#endif
EXTERN u08 showing_help;    // flag set when keyboard help message is on screen



EXTERN int all_adev_row;        // screen row to draw the adev tables at in all_adev mode
EXTERN int adev_bottom_row;     // last row of adev tables
EXTERN int adev_page_size;      // max number of adev bins shown on screen
EXTERN int adev_table_rows;     // max number of adev bins shown on screen
EXTERN int adev_decades_shown;  // number of decades in the plot
EXTERN int left_adev_col;
EXTERN int view_row;            // the text row number just above the view info
EXTERN int queue_row;


//
//  Screen / video mode stuff
//

EXTERN u08 ENDIAN;                 // 0=INTEL byte order,  1=MOTOROLA byte order
EXTERN int text_mode;              // flag set when screen is showing just text
EXTERN S32 initial_window_mode;    // Windows version initial screen mode
EXTERN int go_fullscreen;          // if flag set, init screen in fullscreen mode
EXTERN int user_set_full_screen;   // if flag set, user specified /fu
EXTERN int need_screen_init;       // flag set if user did a /V command from the keyboard
EXTERN int jpl_clock;              // JPL wall clock display
EXTERN int calendar_entries;       // how many entries are in the greetings structure
EXTERN int greet_on;               // flag set if OK to calculate greetings


#define X11_MARGIN 6  // X11 left/right window decoration margin allowance

//#define NARROW_SCREEN (800-X11_MARGIN)  // widths below this are a narrow screen
#define NARROW_SCREEN ((800-X11_MARGIN) * vc_font_scale / 100) // widths below this are a narrow screen  piss
#define SHORT_SCREEN  600  // heights below this are a short screen

#define WIDE_SCREEN (SCREEN_WIDTH >= 1680)  // screen is wide enough for watch and (azel or lla)
#define SMALL_SCREEN ((SCREEN_WIDTH < NARROW_SCREEN) || (SCREEN_HEIGHT < SHORT_SCREEN))
//#define SMALL_SCREEN ((SCREEN_WIDTH < NARROW_SCREEN) || (SCREEN_HEIGHT < (SHORT_SCREEN-30))
#define ADEV_AZEL_THRESH  1280  // screen is wide enough to always show adevs and azel map
#define AZEL_LABEL_THRESH 160   // screen is wide enough label the azel plots values
#define AZEL_TITLE_THRESH 832   // screen is tall enough label the azel plots title


#define TINY_TINY_HEIGHT 400   // experimental ultra-small screen
#define TINY_TINY_WIDTH  450   // experimental ultra-small screen
#define TINY_HEIGHT      480   // screen heights less than this are rather restrictive in what they can show
#define MEDIUM_WIDTH     (1000-X11_MARGIN) // (1024-X11_MARGIN)   // 1024
#define MEDIUM_HEIGHT    710   // 768

#define WIDE_WIDTH    (1280-X11_MARGIN) // 1280
#define BIG_WIDTH     (1440-X11_MARGIN) // 1440

#define PLOT_TOP      480
#define MIN_HEIGHT    320  // 400
#define MIN_WIDTH     320  // 640
#define MAX_WIDTH     4096
#define MAX_AZEL      320  // max size of azel type displays on the main screen

#define FONT_SCALE  1 // vc_font_scale/100  // Note: be careful using... no parens around it
#define CORNER_SIZE (100*vc_font_scale/100) // left/right upper screen corner hot spot size



EXTERN unsigned display_width, display_height;  // screen size
EXTERN int SCREEN_WIDTH;     // screen window size in pixels
EXTERN int SCREEN_HEIGHT;
EXTERN int TEXT_COLS;        // screen size in text chars
EXTERN int TEXT_ROWS;
EXTERN int TEXT_WIDTH;       // char size in pixels
EXTERN int TEXT_HEIGHT;
EXTERN int custom_width;     // custom screen size
EXTERN int custom_height;
EXTERN int vfx_fullscreen;   // set flag to enable WIN_VFX full screen mode
EXTERN int kill_deco;        // set flag to kill window decorations and allow full screen under X11
EXTERN int rotate_screen;    // set flag to rotate screen drawing

#define SWAP(a,b)   if(rotate_screen) { swap_temp=b;  b=a;  a=swap_temp; }
#define SWAPXY(a,b) if(rotate_screen) { swap_temp=b;  b=a;  a=swap_temp; a=(SCREEN_HEIGHT-1)-a; }
EXTERN int swap_temp;

EXTERN u08 big_plot;         // flag set if plot area is large
EXTERN u08 screen_type;      // 's', 'm', 'l' = small, medium, large, etc.
EXTERN int invert_dump;      // swap black and white in screen dumps
EXTERN int force_dump;       // if flag set, force a screen dump
EXTERN int invert_display;   // swap black and white on screen
EXTERN int top_line;         // selects screen dump or plot area only
EXTERN int need_redraw;      // info for screen config update
EXTERN int no_redraw;        // set flag for config_screen() to not do a redraw
EXTERN int need_resize;
EXTERN int new_width;        // requested new screen size (from mouse dragging)
EXTERN int new_height;

EXTERN u08 user_font_size;   // set to size of font to use
EXTERN u08 small_font;       // set flag if font is < 8x16
                             // 1=proportional windows font  2=8x8 DOS font

EXTERN u08 no_x_margin;      // used to override windows screen margins
EXTERN u08 no_y_margin;
EXTERN char *print_using;    // used to print columns with VARIABLE_FONT
EXTERN u08 graphics_coords;  // set flag to pass graphics screen coords to vidstr
                             // ... (which normally uses character coords)


EXTERN int VERT_MAJOR;       // the graph axes tick mark spacing (in pixels)
EXTERN int VERT_MINOR;       // VERT_MAJOR/VERT_MINOR should be 5
EXTERN int HORIZ_MAJOR;
EXTERN int HORIZ_MINOR;
#define PLOT_LEFT 0          // left margin of plot area - should be 0 or PLOT_COL
EXTERN int PLOT_ROW;         // where the plotting area is on the screen
EXTERN int PLOT_COL;
EXTERN int PLOT_WIDTH;       // size of the plot window
EXTERN int PLOT_HEIGHT;
EXTERN int PLOT_TEXT_COL;    // PLOT_COL/TEXT_WIDTH
EXTERN int PLOT_TEXT_ROW;    // PLOT_ROW/TEXT_HEIGHT
EXTERN int PLOT_CENTER;      // center line of the plot window
EXTERN int PLOT_SCROLL;      // when graph fills, scroll it left this many pixels

EXTERN int day_plot;         // flag set to scale plot to 12/24 hours
EXTERN int day_size;         // the number of hours long the day_plot is

EXTERN int last_count_y;


EXTERN int COUNT_SCALE;      // used to scale the satellite count plot
EXTERN DATA_SIZE lux_scale;      // light sensor scale factor (lux/footcancles/candlepower)
EXTERN DATA_SIZE lum_scale;      // light sensor lumen scale factor
EXTERN u08 show_lux;
EXTERN u08 show_fc;
EXTERN u08 show_cp;
EXTERN u08 show_lumens;
EXTERN u08 show_debug_info;
EXTERN int exit_flag;        // flag set if /? seen on the command line

#define WWVB_LAT 40.677722   // lat/lon of WWVB antennas in Ft Collins
#define WWVB_LON -105.047153



// NEW_RCVR

#define TSIP_RCVR      0x0001ULL
#define NMEA_RCVR      0x0002ULL
#define UBX_RCVR       0x0004ULL
#define UCCM_RCVR      0x0008ULL     // mutant SCPI receiver
#define SCPI_RCVR      0x0010ULL
#define MOTO_RCVR      0x0020ULL
#define SIRF_RCVR      0x0040ULL
#define VENUS_RCVR     0x0080ULL
#define ZODIAC_RCVR    0x0100ULL
#define GPSD_RCVR      0x0200ULL
#define NO_RCVR        0x0400ULL
#define NVS_RCVR       0x0800ULL
#define STAR_RCVR      0x1000ULL       // Oscilloquartz STAR 4
#define LUXOR_RCVR     0x2000ULL       // luxor LED / power analyzer
#define ACRON_RCVR     0x4000ULL       // Acron Zeit WWVB receiver
#define TAIP_RCVR      0x8000ULL       // Trimble TAIP protocol
#define TICC_RCVR      0x10000ULL      // TAPR TICC
#define CS_RCVR        0x20000ULL      // HP-5071A Cesium
#define BRANDY_RCVR    0x40000ULL      // Brandywine GPS-4
#define THERMO_RCVR    0x80000ULL      // environmental sensors
#define ZYFER_RCVR     0x100000ULL     // Zyfer Nanosync 380
#define TRUE_RCVR      0x200000ULL     // TruePosition GPSDO
#define RFTG_RCVR      0x400000ULL     // Lucent RFTG-m GPSDO
#define TIDE_RCVR      0x800000ULL     // system clock with plot display
#define PRS_RCVR       0x1000000ULL    // PRS-10 rubidium
#define X72_RCVR       0x2000000ULL    // X72 rubidium
#define TERM_RCVR      0x4000000ULL    // terminal emulator
#define SRO_RCVR       0x8000000ULL    // Spectratime SRO100
#define LPFRS_RCVR     0x10000000ULL   // Spectratime LPFRS
#define FURUNO_RCVR    0x20000000ULL   // Furuno GPS
#define TSERVE_RCVR    0x40000000ULL   // TymSync 2000
#define ESIP_RCVR      0x80000000ULL   // Furuno eSIP devices
#define Z12_RCVR       0x100000000ULL  // Ashtech Z12
#define RT17_RCVR      0x200000000ULL  // Trimble RT17 (NETRS)
#define TM4_RCVR       0x400000000ULL  // Spectrum TM4 GPSDO
#define SS_RCVR        0x800000000ULL  // Novatel Superstar II GPS
#define SA35_RCVR      0x1000000000ULL // Symmetricom SA.35m rubidium

EXTERN u64 rcvr_type;       // GPS receiver message format
EXTERN u64 last_rcvr_type;  // the previous receiver type
EXTERN int calc_rcvr;       // set flag for calculator only mode

#define ASCII_RCVR      (rcvr_type & (ACRON_RCVR | BRANDY_RCVR | CS_RCVR | ESIP_RCVR | FURUNO_RCVR | GPSD_RCVR | LPFRS_RCVR | NMEA_RCVR | PRS_RCVR | SCPI_RCVR | SRO_RCVR | STAR_RCVR | THERMO_RCVR | TICC_RCVR | TM4_RCVR | TRUE_RCVR | TSERVE_RCVR | UCCM_RCVR | X72_RCVR | Z12_RCVR | ZYFER_RCVR))
#define POLLED_RCVR     (rcvr_type & (ACRON_RCVR | BRANDY_RCVR | CS_RCVR | LPFRS_RCVR | PRS_RCVR | SA35_RCVR | SCPI_RCVR | SRO_RCVR | STAR_RCVR | THERMO_RCVR | UCCM_RCVR | X72_RCVR))
#define TIMING_RCVR     (res_t || ACU_GG || ACU_360 || ACUTIME || PALISADE || (rcvr_type & (ESIP_RCVR | MOTO_RCVR | NVS_RCVR | SIRF_RCVR | UBX_RCVR | VENUS_RCVR | ZODIAC_RCVR)))
#define GPSDO           (((rcvr_type == TSIP_RCVR) && !res_t && !PALISADE && !ACUTIME && !SV6_FAMILY) || (rcvr_type & (BRANDY_RCVR | RFTG_RCVR | SCPI_RCVR | STAR_RCVR | TM4_RCVR | TRUE_RCVR | ZYFER_RCVR)) || lte_lite || saw_icm || saw_gpsdo)
#define TICC_USED       ((rcvr_type == TICC_RCVR) || ticc_port_open())    // aaattt
#define NO_ADEV_INFO    (ACU_GG || ACU_360 || ACUTIME || (rcvr_type & (TIDE_RCVR | ACRON_RCVR | ESIP_RCVR | MOTO_RCVR | NMEA_RCVR | NO_RCVR | SS_RCVR | STAR_RCVR | TAIP_RCVR | TM4_RCVR | TRUE_RCVR | TSERVE_RCVR | VENUS_RCVR)))     // receivers that dont have any (bogo) ADEVable info
#define NO_EEPROM_CMDS  ((rcvr_type & (TIDE_RCVR | CS_RCVR | NO_RCVR | RFTG_RCVR | SS_RCVR | THERMO_RCVR | TICC_RCVR | TM4_RCVR | TSERVE_RCVR)))   // receivers that dont support eeprom/nvram saving
#define NO_PPS_INFO     (((rcvr_type & (ACRON_RCVR | TIDE_RCVR | GPSD_RCVR | NMEA_RCVR | NO_RCVR | TAIP_RCVR | TRUE_RCVR | TSERVE_RCVR | ZYFER_RCVR))) || lte_lite)   // receivers that dont have any PPS state info
#define NO_SATS         ((rcvr_type & (TIDE_RCVR | CS_RCVR | LPFRS_RCVR | NO_RCVR | PRS_RCVR | SA35_RCVR | SRO_RCVR | THERMO_RCVR | TICC_RCVR | TSERVE_RCVR | X72_RCVR)) || (lte_lite && (have_sawtooth == 0)) || ((rcvr_type == RFTG_RCVR) && (rftg_unit == 0)) )   // receivers that dont use satellites
#define GPSDO_TUNE_OK   (((rcvr_type == TSIP_RCVR) && !res_t && !ACU_GG && !ACU_360 && !ACUTIME) || saw_icm)
#define TSIP_FMT_RCVR   ((rcvr_type & (NVS_RCVR | RFTG_RCVR | TSIP_RCVR)))   // receivers that use TSIP formatted packets
#define USES_PLOT_THREE ((rcvr_type & (CS_RCVR | LPFRS_RCVR | PRS_RCVR | SA35_RCVR | SRO_RCVR | THERMO_RCVR | TICC_RCVR | X72_RCVR | ZYFER_RCVR))) // receivers that use plot[THREE} for non-lla purposes
#define RAW_SAT_DATA    ((rcvr_type & (ESIP_RCVR | RT17_RCVR | NVS_RCVR | UBX_RCVR | Z12_RCVR))) // receivers that can output raw sat observation data
#define LLA_RCVR        ((rcvr_type & (BRANDY_RCVR | RFTG_RCVR | SCPI_RCVR | TM4_RCVR | TRUE_RCVR | UCCM_RCVR)))  // receivers that can plot lat/lon/alt
//#define TOW_AND_WEEK    ((rcvr_type & (ACRON_RCVR | ESIP_RCVR | NMEA_RCVR | NO_RCVR | STAR_RCVR | THERMO_RCVR | TICC_RCVR | TIDE_RCVR | TM4_RCVR | TRUE_RCVR | VENUS_RCVR | Z12_RCVR)) || ((rcvr_type == MOTO_RCVR) && (have_moto_Hr == 0)))  // fake the GPS week and tow
#define TOW_AND_WEEK    ((have_week == 0) || (have_week == 999))  // fake the GPS week and tow
#define HEX_PACKET_IDS  ((rcvr_type & (BRANDY_RCVR | LPFRS_RCVR | MOTO_RCVR | RT17_RCVR | NMEA_RCVR | PRS_RCVR | RFTG_RCVR | SA35_RCVR | SCPI_RCVR | SIRF_RCVR | SRO_RCVR | SS_RCVR | STAR_RCVR | \
                                       TRUE_RCVR | TSIP_RCVR | UBX_RCVR | UCCM_RCVR | VENUS_RCVR | ZODIAC_RCVR | ZYFER_RCVR | Z12_RCVR)))  // packet names are shown in hex log files
#define USES_JIT_PLOTS  (luxor | (rcvr_type & (CS_RCVR | PRS_RCVR | SRO_RCVR | X72_RCVR))) // receivers that use plots NINE and TEN for non-jitter measurements

#define NO_SURVEY_INFO  ((rcvr_type & (GPSD_RCVR | RT17_RCVR | NMEA_RCVR | RFTG_RCVR | SIRF_RCVR | THERMO_RCVR | TICC_RCVR | TRUE_RCVR | TSERVE_RCVR | Z12_RCVR)))   // receivers that dont have any survey state info
#define SURVEY_PROGRESS ((rcvr_type & (BRANDY_RCVR | SCPI_RCVR | UCCM_RCVR | TRUE_RCVR | TSIP_RCVR | UBX_RCVR | VENUS_RCVR | ZYFER_RCVR)))         // receivers that have survey progrss as a percentage
#define STOPABLE_SURVEY (rcvr_type & (ESIP_RCVR | FURUNO_RCVR | MOTO_RCVR | NVS_RCVR | SCPI_RCVR | TRUE_RCVR | TSIP_RCVR | UBX_RCVR | UCCM_RCVR | VENUS_RCVR | ZODIAC_RCVR | ZYFER_RCVR))

#define LOW_BYTE_FIRST  (rcvr_type & (NVS_RCVR | RFTG_RCVR | UBX_RCVR | ZODIAC_RCVR))  // little endian (Intel) message byte order
#define SER_AVAIL_RCVR  (rcvr_type & (ACRON_RCVR | BRANDY_RCVR | CS_RCVR | SA35_RCVR | LPFRS_RCVR | PRS_RCVR | RFTG_RCVR | SRO_RCVR | STAR_RCVR | X72_RCVR))     // polled receivers that have no non-polled messages
#define STATUS_RCVR     (rcvr_type & (ACRON_RCVR | BRANDY_RCVR | SCPI_RCVR | STAR_RCVR | UCCM_RCVR))   // receivers that have long pauses processing some messages (like SYST:STAT?)
#define DAC_PCT         ((rcvr_type & (BRANDY_RCVR | SCPI_RCVR | UCCM_RCVR | ZYFER_RCVR)) || lte_lite)  // receivers with DAC volatge in percent
#define CRLF_RCVR       (rcvr_type & (BRANDY_RCVR | CS_RCVR | LPFRS_RCVR | PRS_RCVR | SA35_RCVR | SCPI_RCVR | SRO_RCVR | STAR_RCVR | THERMO_RCVR | TICC_RCVR | TM4_RCVR | X72_RCVR | Z12_RCVR))   // receivers that have cr/lf terminated message that mess up terminal mode output formatting

/// polled devices that can't allow get_pending_gps() to loop
#define NO_PEND_LOOP    (((rcvr_type == STAR_RCVR) && (star_type == NEC_TYPE)) || (rcvr_type == ACRON_RCVR) || (rcvr_type == BRANDY_RCVR))

// receivers that supply some kind of ID information and use update_gps_screen() to drive the screen
#define RCVR_SENDS_ID   (rcvr_type & (CS_RCVR | ESIP_RCVR | FURUNO_RCVR | GPSD_RCVR | LPFRS_RCVR | MOTO_RCVR | NVS_RCVR | PRS_RCVR | RFTG_RCVR | SA35_RCVR | SCPI_RCVR | SIRF_RCVR | SRO_RCVR | SS_RCVR | \
                                      STAR_RCVR | TAIP_RCVR | THERMO_RCVR | TICC_RCVR | TRUE_RCVR | UBX_RCVR | UCCM_RCVR | VENUS_RCVR | X72_RCVR | Z12_RCVR | ZODIAC_RCVR | ZYFER_RCVR))

#define HIGH_TS_THRESH  ((rcvr_type & (NO_RCVR | LPFRS_RCVR | PRS_RCVR | SA35_RCVR | SRO_RCVR | THERMO_RCVR | TIDE_RCVR | X72_RCVR)))   // receivers that should not check for small time stamp skips
#define TS_CHECK_THRESHOLD 750.0  // number of msecs of time stamp error allowed for LOW_TS_CHECK devices



#define TICC_RATE   (adev_period*1.0E9)    // expected time interval (in nanoseconds) of TICC data  // aaaaaapppppp

#define HP_TYPE      '8'       // SCPI_RCVR and UCCM_RCVR sub-types
#define HP_TYPE2     '9'
#define LUCENT_TYPE  'K'
#define SAMSUNG_TYPE 'S'
#define NORTEL_TYPE  'N'
#define UCCM_TYPE    'U'
#define UCCMP_TYPE   'P'
#define SCPI_TYPE     0
EXTERN int scpi_echo_mode;     // flag set if SCPI device is echoing commands
EXTERN int scpi_type_changed;  // flag set if SCPI type was auto-changed
EXTERN int user_set_scpi_type;

#define VP_TYPE     'V'
EXTERN int moto_type;          // 1=VP series

#define FURUNO_NMEA 'F'        // NMEA receiver type
#define GARMIN_NMEA 'G'
#define VENUS_NMEA  'V'

#define STAR4_TYPE  '4'        // STAR-4 receiver type - Oscilloquartz
#define NEC_TYPE    'N'        // NEC
#define OSA_TYPE    'O'        // Oscilloquartz OSA-453x

#define ACE3_TYPE     '3'      // tsip_type TSIP_RCVR sub-types
#define ACUTIME_TYPE  'A'
#define LUXOR_TYPE    'L'
#define PALISADE_TYPE 'P'
#define STARLOC_TYPE  'S'
#define SV6_TYPE      '6'
#define TBOLT_TYPE     0

#define VENUS_RTK     'R'      // Venus receiver sub-type
#define VENUS_TIMING  'T'
#define VENUS_TYPE    0
EXTERN int venus_type;

//
//  !!!! WARNING:  SV6_FAMILY includes PALISADE and ACUTIME devices !!!!!
#define SV6_FAMILY  ((rcvr_type == TSIP_RCVR) && ((tsip_type == SV6_TYPE) || (tsip_type == ACE3_TYPE) || (tsip_type == PALISADE_TYPE) || (tsip_type == ACUTIME_TYPE)))
#define ACE3        ((rcvr_type == TSIP_RCVR) && (tsip_type == ACE3_TYPE))
#define ACUTIME     ((rcvr_type == TSIP_RCVR) && (tsip_type == ACUTIME_TYPE))
#define PALISADE    ((rcvr_type == TSIP_RCVR) && (tsip_type == PALISADE_TYPE))
#define ACU_360     (ACUTIME && ((acu_type == '3') || (acu_type == 'G')))
#define ACU_GG      (ACUTIME && (acu_type == 'G'))
#define STARLOC     ((rcvr_type == TSIP_RCVR) && (tsip_type == STARLOC_TYPE))
EXTERN int acu_type;
EXTERN int saw_sv6_time;   // saw the 0x41 GPS time message


#define find_msg_end() check_tsip_end(0)

#define RES_T      1         // res_t sub-types
#define RES_T_SMT  2
#define RES_T_RES  3
#define RES_T_360  4
#define RES_T_ICM  5
EXTERN int tbolt_e;          // flag set for newer TSIP models (like Thunderbolt-E)
EXTERN u08 res_t;            // flag set if Resolution-T message seen

#define TAPR_TICC     'T'    // TAPR TICC time interval counter
#define HP_TICC       'H'    // HP53xxx interval counter
#define COUNTER_TICC  'I'    // generic time interval counter
#define PICPET_TICC   'P'    // TVB's PICPET
#define LARS_TICC     'L'    // Lars general purpose GPSDO
EXTERN int ticc_type;        // 0 = no TIC
EXTERN int user_set_ticc_type;

#define HP5071      'H'      // HP5071A cesium oscillator
EXTERN int cs_type;          // cesium oscillator type

#define DETECT_ENVIRO  0
#define DOG_ENVIRO     1     // dogratian.com USB series devices
#define HEATHER_ENVIRO 2     // custom heather environmental sensor
#define LFS_ENVIRO     3     // LFS104BW
EXTERN int enviro_type;      // themometer type
EXTERN int user_set_enviro_type;
EXTERN int enviro_temp_pid;  // if set, use environmental sensor temp1 for temp control pid

#define ENV_TEMP1   0x01
#define ENV_TEMP2   0x02
#define ENV_HUMID   0x04
#define ENV_PRESS   0x08
#define ENV_SN      0x10
#define ENV_DP      0x20   // for future LFS device support
#define ENV_RTEMP   0x40
#define ENV_LUX     0x80
#define ENV_REFALT  0x100
#define ENV_COUNT   0x200
#define ENV_EMIS    0x400  // IR thermometer emissivity
#define ENV_ID      0x8000
#define NUM_SENSORS 11     // we support up to 10 reading types (not all are implmented)
#define ENVIRO_RATE 10.0   // max poll rate in msecs per char

EXTERN int enviro_sensors; // reading types that the device supports
EXTERN int last_enviro;    // the last reading type we requested
EXTERN int next_enviro;    // the next reading type we want


#define FAKE_ACRON_RCVR  0   // set to 1 to simulate ACRON_RCVR, 0 for real receiver


EXTERN int has_id_info;      // flag set if receiver type outputs ID info

#define MAX_ID_LINES 12
EXTERN char moto_id[MAX_ID_LINES][128+1];  // motorola receiver ID info
EXTERN int moto_id_lines;
EXTERN int furuno_moto;


#define MAX_CS_LOGS 192
#define CS_LOG_SHOW 8
#define LOG_MSG_LEN 32
EXTERN char cs_log_msg[MAX_CS_LOGS+1][LOG_MSG_LEN+2];
EXTERN int cs_log_count;

EXTERN double cs_mjd;        // HP5071A stuff
EXTERN double cs_beam;
EXTERN double cs_cfield;
EXTERN double cs_pump;
EXTERN double cs_gain;
EXTERN double cs_rfam1;
EXTERN double cs_rfam2;
EXTERN double cs_temp;
EXTERN double cs_coven;
EXTERN double cs_qoven;
EXTERN double cs_emul;
EXTERN double cs_hwi;
EXTERN double cs_msp;
EXTERN double cs_pll_dro;
EXTERN double cs_pll_saw;
EXTERN double cs_pll_87;
EXTERN double cs_pll_up;
EXTERN double cs_volt1;
EXTERN double cs_volt2;
EXTERN double cs_volt3;
EXTERN double cs_volt_avg;
EXTERN double cs_freq1;
EXTERN double cs_freq2;
EXTERN double cs_ster;
EXTERN double cs_slew;
EXTERN double cs_leapmjd;
EXTERN int cs_leapdur;
EXTERN int cs_leap_state;
EXTERN int cs_cont;
EXTERN int cs_standby;
EXTERN int cs_disp;
EXTERN int cs_remote;
EXTERN int cs_hours, cs_minutes, cs_seconds;
EXTERN int user_set_remote;
EXTERN char *cs_supply;
EXTERN char *cs_sync;

EXTERN int have_cs_time;
EXTERN int have_cs_mjd;
EXTERN int have_cs_cbtid;
EXTERN int have_cs_temp;
EXTERN char need_cs_timeset;

EXTERN int have_cs_logcount;
EXTERN int have_cs_loginfo;
EXTERN int have_cs_cont;
EXTERN int have_cs_beam;
EXTERN int have_cs_cfield;
EXTERN int have_cs_pump;
EXTERN int have_cs_gain;
EXTERN int have_cs_rfam;
EXTERN int have_cs_glob;
EXTERN int have_cs_supply;
EXTERN int have_cs_coven;
EXTERN int have_cs_qoven;
EXTERN int have_cs_emul;
EXTERN int have_cs_hwi;
EXTERN int have_cs_msp;
EXTERN int have_cs_pll;
EXTERN int have_cs_volt;
EXTERN int have_cs_freq1;
EXTERN int have_cs_freq2;
EXTERN int have_cs_ster;
EXTERN int have_cs_sync;
EXTERN int have_cs_slew;
EXTERN int have_cs_standby;
EXTERN int have_cs_remote;
EXTERN int have_cs_disp;
EXTERN int have_cs_leapdur;
EXTERN int have_cs_leapmjd;
EXTERN int have_cs_leapstate;


EXTERN int have_prs_vb;
EXTERN int have_prs_st;
EXTERN int have_prs_lm;
EXTERN int have_prs_lo;
EXTERN int have_prs_fc;
EXTERN int have_prs_fc2;
EXTERN int have_prs_ds;
EXTERN int have_prs_sf;
EXTERN int have_prs_ss;
EXTERN int have_prs_ga;
EXTERN int have_prs_ph;
EXTERN int have_prs_sp;
EXTERN int have_prs_ms;
EXTERN int have_prs_mo;
EXTERN int have_prs_mr;
EXTERN int have_prs_tt;
EXTERN int have_prs_ts;
EXTERN int have_prs_to;
EXTERN int have_prs_ps;
EXTERN int have_prs_pl;
EXTERN int have_prs_pt;
EXTERN int have_prs_pf;
EXTERN int have_prs_pi;
EXTERN int have_prs_ep;
EXTERN int have_prs_sd;
EXTERN int have_prs_ad;

EXTERN int prs_vb;
EXTERN int prs_st[6];
EXTERN int prs_lm;
EXTERN int prs_lo;
EXTERN int prs_fc1, prs_fc2;
EXTERN int prs_ee_fc1;
EXTERN int prs_ee_fc2;
EXTERN int prs_pwr_cycles, prs_fc_writes;
EXTERN int prs_ds1;  // 2nd harmonic
EXTERN int prs_ds2;  // signal strength
EXTERN int prs_sf;
EXTERN int prs_ss;
EXTERN int prs_ga;
EXTERN int prs_ph;
EXTERN int prs_sp[3];
EXTERN int prs_ms;
EXTERN int prs_mo;
EXTERN int prs_mr;
EXTERN int prs_tt;
EXTERN int prs_ts;
EXTERN int prs_to;
EXTERN int prs_ps;
EXTERN int prs_pl;
EXTERN int prs_pt;
EXTERN int prs_pf;
EXTERN int prs_pi;
EXTERN int prs_ep;           // enable power
EXTERN int prs_sd[8];        // dac settings
EXTERN double prs_ad[20];    // adc readings
EXTERN char last_prs_set[3]; // the last PRS-10 parameter setting command
EXTERN double prs_fc_ppt;
EXTERN double prs_ee_fc_ppt;
EXTERN double prs_ad_pwr;    // average of power values
EXTERN double prs_ad_therm;  // average of thermistor values

#define X72_OSC  60.0E6      // assumed X72 clock freq - some units are 58982400.0
#define X72_OSC2 58982400.0  // telecom units use 58982400.0
#define X72_FREQ X72_OSC     // (for some backwards compatibility testing)

#define X72_TICS ((int) x72_osc)
#define X72_TYPE  0      // x72_type values
#define SA22_TYPE 1
#define X99_TYPE  3      // (not currently used / tested)
EXTERN double x72_osc;   // X72 master osc frequency

#define X72_HOLDOVER_SIZE  120    // how many seconds to do holdover state
#define X72_TIME_CONST     400    // 120      // DDS update time constant
#define X72_JAMSYNC_THRESH 300.0  // jamsync the PPS if greater than this many ns
#define X72_DAMPING        1.0    // default damping factor
#define MAX_X72_TC         100000 // maximum discipline time constant
#define MIN_X72_TC         5      // minimum discipline time constant

EXTERN int have_x72_creg;
EXTERN int have_x72_scont;
EXTERN int have_x72_sernum;
EXTERN int have_x72_pwrticks;
EXTERN int have_x72_lhhrs;
EXTERN int have_x72_lhticks;
EXTERN int have_x72_rhhrs;
EXTERN int have_x72_rhticks;
EXTERN int have_x72_dmp17;
EXTERN int have_x72_dmp5;
EXTERN int have_x72_dhtrvolt;
EXTERN int have_x72_plmp;
EXTERN int have_x72_pres;
EXTERN int have_x72_dlvthermc;
EXTERN int have_x72_drvthermc;
EXTERN int have_x72_dlvolt;
EXTERN int have_x72_dmvoutc;
EXTERN int have_x72_dtemplo;
EXTERN int have_x72_dtemphi;
EXTERN int have_x72_dvoltlo;
EXTERN int have_x72_dvolthi;
EXTERN int have_x72_dlvoutc;
EXTERN int have_x72_drvoutc;
EXTERN int have_x72_dmv2demavg;

EXTERN int have_x72_pps;
EXTERN int have_x72_state;
EXTERN int have_x72_dds_word;

EXTERN int have_x72_fw;
EXTERN int have_x72_type;
EXTERN int have_x72_serial;
EXTERN int have_x72_loader;
EXTERN int have_x72_res_tempofs;
EXTERN int have_x72_lamp_tempofs;
EXTERN int have_x72_crystal;
EXTERN int have_x72_acmos;
EXTERN int have_x72_sine;
EXTERN int have_x72_tune;
EXTERN int have_x72_efc;
EXTERN int have_x72_srvc;
EXTERN int have_x72_info;

EXTERN int x72_state_set;
EXTERN int have_x72_fw_dis;
EXTERN int have_x72_fw_dmode;
EXTERN int have_x72_dmode;
EXTERN int have_x72_tc;
EXTERN int have_x72_damping;
EXTERN int have_x72_jamthresh;
EXTERN int have_x72_holdover;

EXTERN int x72_loader;
EXTERN int x72_tune;
EXTERN int x72_efc;
EXTERN int x72_srvc;
EXTERN int x72_type;
EXTERN char x72_serial[32+1];
EXTERN char x72_date[32+1];
EXTERN double x72_fw;
EXTERN double x72_res_tempofs;
EXTERN double x72_lamp_tempofs;
EXTERN double x72_crystal;
EXTERN double x72_acmos;
EXTERN double x72_sine;

EXTERN int x72_creg;
EXTERN int x72_scont;
EXTERN int x72_sernum;
EXTERN int x72_pwrticks;
EXTERN int x72_lhhrs;
EXTERN int x72_lhticks;
EXTERN int x72_rhhrs;
EXTERN int x72_rhticks;
EXTERN int x72_sine_level;
EXTERN int x72_pps;
EXTERN int x72_state;

EXTERN int x72_tc_val;
EXTERN int x72_holdover_val;
EXTERN int x72_dmode_val;
EXTERN int x72_fw_dmode;
EXTERN double x72_damping_val;
EXTERN double x72_jamthresh_val;

EXTERN double x72_dmp17;
EXTERN double x72_dmp5;
EXTERN double x72_dhtrvolt;
EXTERN double x72_plmp;
EXTERN double x72_pres;
EXTERN double x72_dlvthermc;
EXTERN double x72_drvthermc;
EXTERN double x72_dlvolt;
EXTERN double x72_dmvoutc;
EXTERN double x72_dtemplo;
EXTERN double x72_dtemphi;
EXTERN double x72_dvoltlo;
EXTERN double x72_dvolthi;
EXTERN double x72_dlvoutc;
EXTERN double x72_drvoutc;
EXTERN double x72_dmv2demavg;

EXTERN double x72_dds_word;

EXTERN int user_set_x72_dds;
EXTERN int user_set_x72_osc;
EXTERN int user_set_x72_acmos_freq;
EXTERN int user_set_x72_pps_enable;
EXTERN int user_set_x72_tic;
EXTERN int user_set_x72_efc;
EXTERN int user_set_x72_srvc;

EXTERN int last_x72_acmos_freq;
EXTERN int last_x72_efc;
EXTERN int last_x72_pps_enable;
EXTERN int last_x72_tic;
EXTERN int last_x72_srvc;
EXTERN double last_x72_dds;     // last values the user set (these can't be read back from device)
EXTERN double last_x72_osc;     // master osc freq (usually 60E6)

EXTERN int x72_user_dis;

EXTERN int user_set_x72_tc;
EXTERN int user_set_x72_damping;
EXTERN int user_set_x72_jamthresh;
EXTERN int user_set_x72_holdover;
EXTERN int user_set_x72_dmode;

EXTERN double last_x72_damping;
EXTERN double last_x72_jamthresh;
EXTERN int last_x72_holdover;
EXTERN int last_x72_tc;
EXTERN int last_x72_dmode;

EXTERN int x72_pps_interval; // how many PPS samples we have accumulated
EXTERN int x72_dds_count;    // number of entries in the averaging queue
EXTERN double x72_pps_trend; // average value of the PPS error over the interval
EXTERN int x72_tic_timer;    // counts down 6 seconds from updating the TIC register
EXTERN int x72_ival;         // current TIC reading


EXTERN char nvs_id[32];      // NVS_RCVR id and serial number
EXTERN u32 nvs_sn;
EXTERN char nvs_id2[32];     // NVS_RCVR reserved id and serial number
EXTERN u32 nvs_sn2;
EXTERN char nvs_id3[32];     // NVS_RCVR reserved id and serial number
EXTERN u32 nvs_sn3;
EXTERN int nvs_chans;        // NVS_RCVR channel count

EXTERN int detect_rcvr_type; // if flag set, attempt to detect the receiver type
EXTERN int detecting;        // flag set while auto-detect is in progress
EXTERN int lte_lite;         // Jackson Labs LTE Lite with Venus receiver and STATUS message seen
                             // 0=no STATUS message seen, 1=STATUS message seen  2=STATUS and NMEA messages seen

EXTERN int tsip_type;        // flag set for DATUM GPSDO, etc
EXTERN int datum_flag;       // flag set for DATUM STAR_LOC with crappy firmware
EXTERN int sv6_flag;         // Trimble SV6 seend
EXTERN int scpi_type;        // 0-Z3801A 5=HP5xxxx series 'U' = UCCM
EXTERN int nmea_type;        // NMEA receiver type
EXTERN int star_type;        // STAR-4 receiver type
EXTERN int osa_model;        // OSA-453x model number
EXTERN int dup_star_time;    // used to force keyboard check
EXTERN int saw_star_time;    // flag set if Star-4 timecode seen
EXTERN int star_line;        // number of lines in multi-line response
EXTERN int star_msg;         // multi-line message type
EXTERN int have_uccm_loop;   // assume not a UCCM CDMA device without DIAG:LOOP command.
EXTERN int have_uccm_tcor;
EXTERN int have_uccm_pcor;
EXTERN int have_uccm_gps_phase;
EXTERN int have_uccm_ext_val;
EXTERN float uccm_ext_val;

EXTERN int saw_uccm_dmode;   // flag set if "settling" seen in UCCM SYST:STAT? response
EXTERN int adjust_scpi_ss;   // if flag set, remap SCPI SS levels from 0..255 to 0..50ish range
EXTERN int force_mode_change;// if flag set, leave ZODIAC receiver in Motorola mode when reset
EXTERN int moto_chans;       // Motorola receiver channel count
EXTERN int rcvr_reset;       // flag set if hard-resetting Motorola reveiver
EXTERN int user_set_rcvr;
EXTERN int need_msg_init;
EXTERN int hw_setup;         // flag set if the hardware has been set up
EXTERN int saw_gpsdo;
EXTERN int pkt_end1,pkt_end2;     // binary packet end flags
EXTERN int pkt_start1,pkt_start2; // binary packet start flags

EXTERN char gpsd_driver[32+1];
EXTERN char gpsd_release[32+1];
EXTERN int gpsd_major;
EXTERN int gpsd_minor;
EXTERN int saw_gpsd_pps;
EXTERN int saw_gpsd_pre;

EXTERN int plm_freq;        // nominal power line freq
EXTERN int plm_cycles;      // counter of power line cycles


EXTERN int user_set_traim;
EXTERN int traim_mode;
EXTERN int traim_threshold;
EXTERN int traim_status;
EXTERN int nvs_traim;

EXTERN int moto_active;      // 6/8 channel idle/active flag
EXTERN int have_gpgga;       // flag set if NMEA $gpgga message seen
EXTERN int have_gprmc;       // flag set if NMEA $gprmc message seen
EXTERN int have_gpgns;       // flag set if NMEA $gpgns message seen
EXTERN int have_gpzda;       // flag set if NMEA $gpzda message seen
EXTERN int have_gpgsv;       // flag set if NMEA $gpgsv message cycle seen
EXTERN int have_nmea_date;   // flag set of NMEA date has been seen
EXTERN int posn_mode;        // 0=3D, 1=posn hold,  2=2D, 3=timing auto survey


EXTERN DATA_SIZE load_ovc;       // luxor protection thresholds
EXTERN DATA_SIZE load_hvc;
EXTERN DATA_SIZE load_lvc;
EXTERN DATA_SIZE load_watts;
EXTERN DATA_SIZE batt_ovc;
EXTERN DATA_SIZE batt_hvc;
EXTERN DATA_SIZE batt_lvc;
EXTERN DATA_SIZE batt_watts;
EXTERN DATA_SIZE auxv_hvc;
EXTERN DATA_SIZE auxv_lvc;
EXTERN DATA_SIZE tc1_ovt;
EXTERN DATA_SIZE tc2_ovt;
EXTERN DATA_SIZE msg_timeout;
EXTERN u08 prot_menu;
EXTERN u08 show_prots;
EXTERN u08 fault_seen;

EXTERN DATA_SIZE vref_m,  vref_b;    // luxor calibration values
EXTERN DATA_SIZE temp1_m, temp1_b;
EXTERN DATA_SIZE temp2_m, temp2_b;
EXTERN DATA_SIZE vcal_m,  vcal_b;
EXTERN DATA_SIZE batti_m, batti_b;
EXTERN DATA_SIZE ledi_m,  ledi_b;
EXTERN DATA_SIZE lux1_m,  lux1_b;
EXTERN DATA_SIZE lux2_m,  lux2_b;
EXTERN DATA_SIZE adc2_m,  adc2_b;
EXTERN DATA_SIZE rb_m,    rb_b;

EXTERN DATA_SIZE luxor_tc1, luxor_tc2;

EXTERN double osc_integral;  // integral of oscillator error

EXTERN volatile int timer_serve;
EXTERN u08 disable_kbd;             // set flag to disable keyboard commands
EXTERN u08 kbd_flag;
EXTERN u08 touch_screen;            // flag set if touch screen is enabled
EXTERN int last_plot_key;           // flags last special plot control key pressed
EXTERN int last_null_key;           // flags last touch screen key was null
EXTERN int last_touch_key;          // flags last to touch screen key (or 999 for ?>cr)
EXTERN int force_si_cmd;            // used by royal kludge to implement G C T keyboard command
EXTERN char tide_kbd_cmd;           // used by roral kludge to implement tide plot selection keyboard commands
EXTERN u08 esc_esc_exit;            // set flag to allow ESC ESC to exit program
EXTERN u08 esc_esc_esc;
EXTERN u08 com_error_exit;          // set flag to abort on com port error
EXTERN u08 pause_data;              // set flag to pause updating queues
EXTERN u08 user_pause_data;         // flag set if user set something on the command line that needs to pause data input
EXTERN u08 no_eeprom_writes;        // set flag to disable writing EEPROM
EXTERN u08 no_easter_eggs;          // set flag to disable easter egg songs
EXTERN int ee_write_count;          // counts number of eeprom writes seen

EXTERN u08 mouse_disabled;          // if set, do not do mouse stuff
EXTERN u08 mouse_shown;             // if set,  mouse cursor is being shown
EXTERN S32 mouse_x, mouse_y;        // current mouse coordinates
EXTERN int crap_mouse;              // if set, all buttons act like left button (for PI 480x320 touchscreen)
EXTERN int user_set_crap_mouse;
EXTERN S32 last_plot_mouse_x;       // last X coord the mouse was in the plot area at
EXTERN S32 last_plot_tick;          // previous X coord the mouse was in the plot area at
EXTERN u08 mouse_plot_valid;        // flag set if mouse points to valid queue entry
EXTERN u08 last_mouse_plot_valid;   // used to minimize clearing of mouse data area
EXTERN u08 new_mouse_info;
EXTERN int this_button;             // current mouse button state
EXTERN int last_button;             // previous mouse button state
EXTERN long plot_q_col0;            // queue entry of leftmost column in the plot
EXTERN long plot_q_last_col;        // queue entry of rightmost column in the plot
EXTERN long last_mouse_q;           // the queue entry the mouse was last over
EXTERN long last_plot_glitch;       // the queue entry the last patched plot queue entry
EXTERN long last_plot_mouse_q;      // the queue entry the mouse was last over
EXTERN int last_mouse_x;
EXTERN int last_mouse_y;
EXTERN long last_q_place;           // where we were before we moved to a marker
EXTERN int cursor_time_ref;

#define MAX_MARKER 10
EXTERN long mark_q_entry[MAX_MARKER]; // the queue entry we list clicked on
EXTERN int last_was_mark;
EXTERN int last_was_adev;
EXTERN int lwa;

// bits in the sat_flags word
#define TEMP_SPIKE      0x1000
#define CONST_CHANGE    0x0800
#define TIME_SKIP       0x0400
#define UTC_TIME        0x0200
#define HOLDOVER        0x0100
#define SAT_COUNT_MASK  0x00FF  // this bit mask MUST be the low order bits
#define SAT_FLAGS       (TEMP_SPIKE | CONST_CHANGE | TIME_SKIP | UTC_TIME | HOLDOVER)

#define SENSOR_TC    10.0F          // tbolt firmware temperature sensor averaging time (in seconds)
EXTERN  u08 undo_fw_temp_filter;
EXTERN  u08 user_set_temp_filter;

// the various data that we can plot
#define PPS      0     // plots 0..3 are the standard plots.
#define OSC      1
#define DAC      2
#define TEMP     3

#define ADC3     PPS
#define ADC4     OSC
#define HUMIDITY ELEVEN
#define PRESSURE TWELVE
#define TEMP1    THIRTEEN
#define TEMP2    FOURTEEN
#define SAT_PLOT FIFTEEN

// plots 4 and above are option/extra plots
// ... they have different meanings for different devices
#define ONE      4     // ONE / TWO / THREE are usually lat/lon/alt
#define TWO      5
#define THREE    6
#define FOUR     7     // usually speed
#define FIVE     8     // usually heading
#define SIX      9     // usually average DOP
#define SEVEN    10    // THERMO_RCVR humidity
#define EIGHT    11    // THERMO_RCVR barometer
#define NINE     12    // usually end-of-message timing offset
#define TEN      13    // usually end-of-message timing jitter

#define FFT      14
#define ELEVEN   15    // plots 11 and above are usuallly from data derived from other plot data
#define TWELVE   16
#define THIRTEEN 17
#define FOURTEEN 18
#define FIFTEEN  19

#define CHC      SEVEN // TICC channel C
#define CHD      EIGHT // TICC channel D
#define MSGOFS   NINE  // timing message offset from wall clock
#define MSGJIT   TEN   // timing message jitter

// luxor plot id's
#define BATTI    OSC
#define LUX1     PPS
#define BATTV    DAC
#define LUX2     ONE
#define LEDV     TWO
#define LEDI     THREE
#define PWMHZ    FOUR
#define TC2      FIVE
#define BLUEHZ   SIX
#define GREENHZ  SEVEN
#define REDHZ    EIGHT
#define WHITEHZ  NINE
#define AUXV     TEN
#define BATTW    ELEVEN
#define LEDW     TWELVE
#define EFF      THIRTEEN
#define CCT      FOURTEEN

#ifdef FFT_STUFF
   #define NUM_PLOTS 16  // NUM_PLOTS does not include the DERVVED_PLOTS!
#else
   #define NUM_PLOTS 15
#endif

#define DERIVED_PLOTS 4       // extra plots where the data is derived from other recorded plot data
#define FIRST_EXTRA_PLOT ONE

struct PLOT_Q {    // the data we can plot
   u16 sat_flags;             // misc status and events
   double q_jd;               // time stamp (in UTC)
   DATA_SIZE data[NUM_PLOTS+DERIVED_PLOTS]; // the data values we can plot
};

EXTERN struct PLOT_Q *plot_q;
EXTERN unsigned long *hash_table;

EXTERN long plot_q_size;     // number of entries in the plot queue
EXTERN long plot_q_in;       // where next data point goes into the plot queue
EXTERN long plot_q_out;      // next data point that comes out of the queue
EXTERN long plot_q_count;    // how many points are in the plot queue
EXTERN u08  plot_q_full;     // flag set if plot queue is full
EXTERN long plot_start;      // queue entry at first point in the plot window
EXTERN u08  user_set_plot_size;


EXTERN DATA_SIZE nav_rate;        // receiver navigation fix rate (in Hz)
EXTERN DATA_SIZE user_nav_rate;   // forced startup navigation fix rate (in Hz)
EXTERN DATA_SIZE saved_nav_rate;  // nav rate in effect before log file read in
EXTERN int restore_nav_rate;      // if flag set, restore old nav rate value when pause released
EXTERN int user_set_nav_rate;

#define RAW_MSG_RATE  1      // rate to send RAW and SFRB messages (if 0, polled by standard message poll loop)
EXTERN int raw_msg_rate;     // output rate for satellite raw data messages
EXTERN int special_raw;      // if set, send undocumented messages
EXTERN int user_set_raw_rate;
EXTERN int have_raw_rate;    // flag set when receiver raw rate has been set
EXTERN int add_clk_ofs;      // if flag set add RT17 clock offset to observation timestamps

EXTERN int plot_time;        // indicates when it is time to draw a new pixel
EXTERN int view_time;        // same, but used when view_interval is not 1

EXTERN u08 interval_set;     // flag set if user did /i command to change plot queue interval
EXTERN long queue_interval;  // number of seconds between plot queue entries
                             // ... used to average each plot point over

EXTERN long view_interval;   // this can be used to set the interval that data will be
                             // extracted from the plot queue and displayed (to allow plots
                             // to be displayed at a different time scale from the queue interval)

EXTERN long log_interval;    // seconds between log file updates
EXTERN int user_set_qi;      // flag set if the user set the queue interval


EXTERN long user_view;       // the view time the user set on the command line
EXTERN int need_view_auto;   // if flag set (/va command line option) force View Auto mode
EXTERN u08 new_user_view;
EXTERN u08 view_all_data;    // set flag to view all data in the queue
EXTERN u08 slow_q_skips;     // if set, skip over sub-sampled queue entries using the slow method
EXTERN u08 set_view;
EXTERN u08 continuous_scroll;// if set, redraw plot on every incoming point

EXTERN int plot_column;             // the current pixel column we are plotting
EXTERN int plot_mag;                // magnifies plot this many times

EXTERN u08 auto_scale;              // if set, enables auto scaling of plots
EXTERN u08 auto_center;             // if set, enables auto centering of plots
EXTERN u08 peak_scale;              // if set, don't let auto scale factors get smaller
EXTERN u08 off_scale;               // flag set whenever a plot goes off scale

//   values used to center plots around
#define NEED_CENTER 99999.0         // value used to indicate uninitialized

EXTERN u08 screen_configed;         // flag set if screen is re-configured
EXTERN u08 plot_adev_data;          // flags to control whether or not to plot the
EXTERN u08 old_plot_adevs;          // used to restore screen from zoomed all_adevs screen
EXTERN u08 plot_sat_count;          // ... various parameters
EXTERN u08 small_sat_count;         // is set, use compressed sat count graph
EXTERN u08 plot_const_changes;
EXTERN u08 plot_skip_data;          // time sequence and message errors
EXTERN u08 plot_holdover_data;
EXTERN u08 need_posns;              // if set, calculate the sun and moon positions
//EXTERN u08 plot_temp_spikes;
#define plot_temp_spikes spike_mode
EXTERN u08 plot_version;            // set flag to show program version on main sreen
EXTERN u08 plot_loc;                // set flag to show lat/lon/alt
EXTERN u08 new_lla_mode;            // location display mode changed
EXTERN u08 plot_stat_info;          // set flag to show statistics value of plots
EXTERN u08 plot_digital_clock;      // set flag to show digital clock display
EXTERN int clock_show_row;          // the text row of the top of the digital clock
EXTERN u08 show_msecs;              // set flag to show milliseconds in the digital clock
EXTERN int show_elapsed_time;       // set flag to show elapsed stopwatch time
EXTERN int elapsed_time_set;
EXTERN double jd_elapsed;
EXTERN u08 show_julian_time;        // set flag to show digial clock time as julian date
EXTERN u08 show_mjd_time;           // set flag to show digital clock as Modifed Julian Date
EXTERN u08 show_unix_time;          // set flag to show digital clock as Unix epoch time
EXTERN u08 show_gps_time;           // set flag to show digital clock as GPS epoch time
EXTERN int plot_areas;              // the number of available places to draw maps and the analog watch
EXTERN u08 plot_watch;              // set flag to show watch face where the azel map goes
EXTERN int bogo_watch;              // (used to debug watch placement)
EXTERN u08 map_and_watch;           // if flag set,  draw sat map over the watch face
EXTERN u08 map_and_sigs;            // if flag set,  draw sat map over the signal level display
EXTERN u08 watch_face;              // which watch face to use
EXTERN u08 plot_azel;               // set flag to enable the az/el map
EXTERN u08 update_azel;             // if flag set, force update of azel plot
EXTERN u08 plot_signals;            // set flag to enable the signal level displays
EXTERN u08 plot_el_mask;            // set flag to show elevation mask in the az/el map
EXTERN u08 clock_12;                // set flag to show 12 hour clock
EXTERN u08 alarm_type;              // set flag to trigger alarms/dumps on displayed (vs local) time
EXTERN u08 map_trails;              // set flag to draw history trails to sats in azel map
EXTERN u08 dot_trails;              // set flag to draw time markers on satellie history trails
EXTERN u08 no_greetings;            // set flag to disable holiday greetings
EXTERN u08 erase_every_hour;        // set flag to redraw lla map every hour
EXTERN u08 user_set_bigtime;        // flag set if user specified the big clock option on the command line
EXTERN u08 user_set_signals;        // flag set if user specified the signal quality map on the command line
EXTERN u08 user_set_sat_cols;       // flag set if user specified the sat into table config
EXTERN u08 user_set_dops;           // flag set if user specified the dop display enable flag on the command line
EXTERN u08 user_set_dfilter;        // flag set if user specified the filter display enable flag on the command line
EXTERN u08 beep_on;                 // flag set to enable beeper
EXTERN int show_beep_reason;        // flag set to show reason for beeps
EXTERN u08 sound_on;                // flag set to enable sounds
EXTERN u08 blank_underscore;        // if set, a '_' on output gets blanked
EXTERN u08 show_live_fft;           // if set, display FFT on the incomming data
EXTERN u08 live_fft;                // the plot to show live data on

#define ROUND_SATS  0
#define RECT_SATS   1
#define ROUND_WINGS 2
#define RECT_WINGS  3
#define ROUND_LINES 4
#define RECT_LINES  5
EXTERN int fancy_sats;              // the shape to draw the sats in: 1=round 2=square 3=round+wings 4=square+wings;

EXTERN u08 alt_calendar;            // use alternate calendar for date display
#define GREGORIAN  0
#define HEBREW     1
#define INDIAN     2
#define ISLAMIC    3
#define PERSIAN    4
#define AFGHAN     5
#define KURDISH    6
#define MJD        7
#define JULIAN     8
#define ISO        9
#define MAYAN      10     // Mayan long count
#define HAAB       11     // Mayan Haab date
#define TZOLKIN    12     // Mayan Tzolkin date
#define AZTEC      13     // Aztec Tzolkin type date
#define AZTEC_HAAB 14     // Aztec Haab type date
#define DRUID      15     // a pseudo-Druid calendar
#define CHINESE    16     // the chinese calendar
#define BOLIVIAN   17     // the new Bolivian calendar
#define ISO_WEEK   18     // ISO yeay,week,day of week

#define MAYAN_CORR 584283L          // default mayan correlation constant
EXTERN long mayan_correlation;
EXTERN long aztec_epoch;            // day offset from default epoch
EXTERN long chinese_epoch;          // year offset from default epoch
EXTERN long druid_epoch;            // day offset from default epoch
EXTERN long bolivian_epoch;         // day offset from default epoch

EXTERN double rollover;             // offset gps time/date from default epoch (in seconds)
EXTERN int user_set_rollover;

EXTERN long filter_count;           // number of entries to average for display filter
EXTERN int disp_filter_type;
EXTERN u08 filter_log;              // if set, write filtered values to the log
                                    // when writing a log from queued data
#define FFT_TYPE  0
#define HIST_TYPE 1
EXTERN u08 fft_type;                // FFT or histogram
EXTERN u08 fft_id;                  // the currently selected signal to FFT or histogram
EXTERN int fft_row;                 // screen row for waterfall display
EXTERN int fft_col;                 // screen col for waterfall display

#define MAX_HIST 4096
EXTERN long plot_hist[MAX_HIST];
EXTERN long hist_size;
EXTERN DATA_SIZE hist_minv, hist_maxv, hist_bin_width;

EXTERN double pps_base_value;       // used to keep values in the ADEVqueues reasonable for single precision numbers
EXTERN double osc_base_value;
EXTERN double chc_base_value;
EXTERN double chd_base_value;
EXTERN int have_pps_base;           // used to keep values in the ADEV queues reasonable for single precision numbers
EXTERN int have_osc_base;
EXTERN int have_chc_base;
EXTERN int have_chd_base;
EXTERN u08 subtract_base_value;     // if set base values are subtracted/added
                                    // 1 = base values subtracted/added to queued values
                                    // 2 = base values subtarcted when read in

#define PDOP 0x01
#define HDOP 0x02
#define VDOP 0x04
#define GDOP 0x08
#define TDOP 0x10
#define NDOP 0x20
#define EDOP 0x40
#define XDOP 0x80      // GPSD
#define YDOP 0x100     // GPSD
EXTERN int have_dops;

EXTERN DATA_SIZE pdop;      // dilution of precision values
EXTERN DATA_SIZE hdop;
EXTERN DATA_SIZE vdop;
EXTERN DATA_SIZE gdop;
EXTERN DATA_SIZE tdop;
EXTERN DATA_SIZE ndop;
EXTERN DATA_SIZE edop;
EXTERN DATA_SIZE xdop;
EXTERN DATA_SIZE ydop;
EXTERN DATA_SIZE avg_dop;   // last average of all DOP values (from average_dop())
EXTERN u08 plot_dops;       // set flag to show the dops

// vector character stuff (plot markers and big digital clock display)
EXTERN int VCHAR_SCALE;
#define VCharWidth       (VCHAR_SCALE>1?(VCHAR_SCALE*8):TEXT_WIDTH)
#define VCharHeight      (VCHAR_SCALE>1?(VCHAR_SCALE*16):TEXT_HEIGHT)
//#define VCharThickness ((VCHAR_SCALE/2)?VCHAR_SCALE/2:((vc_font_scale>100)?2:1))
#define VCharThickness   vchar_stroke()

EXTERN int use_vc_fonts;
EXTERN int vc_font_scale;
EXTERN int user_set_font_scale;
EXTERN int user_set_plot_row;

#define VSTRING_LEN 128+1           // max vector character string length
EXTERN char date_string[VSTRING_LEN];
EXTERN int time_color;
EXTERN int last_time_color;

EXTERN double last_stamp;       // time stamp checking stuff
EXTERN u08 time_checked;
EXTERN u08 have_last_stamp;
EXTERN int starloc_skip_delay;  // don't check time stamp after bad stamp fixed
EXTERN int bad_starloc_time;    // flag set if bad starloc time code
EXTERN long idle_sleep;         // Sleep() this long when idle
EXTERN long sim_delay;          // Sleep() this long after processing a sim_file message
EXTERN int ticc_sim_file_read;  // flag set if a log or simulation file has been read in
EXTERN int sim_file_read;       // flag set if a log or simulation file has been read in
                                // (used to switch tide plots to enviro plots is /en=0 specified)

#define CHA_MTIE 0
#define CHB_MTIE 1
#define CHC_MTIE 2
#define CHD_MTIE 3
#define MAX_MTIE_CHANS 4

#ifdef ADEV_STUFF
   #define OSC_ADEV 0    // adev_types (OSC, PPS, CHC, CHD... keep them in that order!)
   #define OSC_HDEV 1
   #define OSC_MDEV 2
   #define OSC_TDEV 3
   #define B_MTIE   4

   #define PPS_ADEV 5
   #define PPS_HDEV 6
   #define PPS_MDEV 7
   #define PPS_TDEV 8
   #define A_MTIE   9

   #define CHC_ADEV 10
   #define CHC_HDEV 11
   #define CHC_MDEV 12
   #define CHC_TDEV 13
   #define C_MTIE   14

   #define CHD_ADEV 15
   #define CHD_HDEV 16
   #define CHD_MDEV 17
   #define CHD_TDEV 18
   #define D_MTIE   19

   #define NUM_ADEV_TYPES 5       // we calculate 4 xDEVs for each channel
   #define OSC_ID  (OSC_ADEV/NUM_ADEV_TYPES)
   #define PPS_ID  (PPS_ADEV/NUM_ADEV_TYPES)
   #define CHC_ID  (CHC_ADEV/NUM_ADEV_TYPES)
   #define CHD_ID  (CHD_ADEV/NUM_ADEV_TYPES)

   EXTERN int ATYPE;              // the adev type to display
   EXTERN int last_atype;

   #define DISPLAY_ADEV    0x01
   #define DISPLAY_HDEV    0x02
   #define DISPLAY_MDEV    0x04
   #define DISPLAY_TDEV    0x08
   #define DISPLAY_MTIE    0x10
   #define DISPLAY_CHA     0x100
   #define DISPLAY_CHB     0x200
   #define DISPLAY_CHC     0x400
   #define DISPLAY_CHD     0x800
   EXTERN int adev_display_mask;      // used to mask out adev types to display in all_adev mode


   #define ADEV_DISPLAY_RATE  10  // aaahhh - update adev plots at this rate

   #define MAX_ADEV_BINS 100      // max number of adev bins to process (must be at least 32 because of MTIE support)
   struct ADEV_INFO {
      u08   adev_type;            // pps or osc and
      float adev_taus[MAX_ADEV_BINS];
      float adev_bins[MAX_ADEV_BINS];
      long  adev_on[MAX_ADEV_BINS];
      int   bin_count;
      float adev_min;
      float adev_max;
   };

   EXTERN struct ADEV_INFO pps_bins;
   EXTERN struct ADEV_INFO osc_bins;
   EXTERN struct ADEV_INFO chc_bins;
   EXTERN struct ADEV_INFO chd_bins;
   EXTERN int max_adev_rows;      // actual number of bins we use (based upon adev_q_size)

   EXTERN struct ADEV_INFO cha_mtie_bins;
   EXTERN struct ADEV_INFO chb_mtie_bins;
   EXTERN struct ADEV_INFO chc_mtie_bins;
   EXTERN struct ADEV_INFO chd_mtie_bins;

   struct BIN {
      S32    m;        // Integer tau factor (where tau = m*tau0)
      S32    n;        // # of phase points already contributing to this bin's value; calc loops run between B->n and n_points
      double sum;      // Running sum of squared variances
      double value;    // Latest final calculation result
      double tau;      // Tau factor times sample period in seconds
      double accum;    // Auxiliary sum for multiloop calculations
      S32    i;        // Auxiliary index for multiloop calculations
      S32    j;        // Auxiliary index for multiloop calculations
      S32    init;     // Flag processing needed on initial step
   };

   EXTERN struct BIN pps_adev_bins[MAX_ADEV_BINS+1];  // incremental adev info bins
   EXTERN struct BIN pps_hdev_bins[MAX_ADEV_BINS+1];
   EXTERN struct BIN pps_mdev_bins[MAX_ADEV_BINS+1];
   EXTERN struct BIN pps_tdev_bins[MAX_ADEV_BINS+1];
   EXTERN struct BIN osc_hdev_bins[MAX_ADEV_BINS+1];
   EXTERN struct BIN osc_mdev_bins[MAX_ADEV_BINS+1];
   EXTERN struct BIN osc_tdev_bins[MAX_ADEV_BINS+1];
   EXTERN struct BIN osc_adev_bins[MAX_ADEV_BINS+1];
   EXTERN struct BIN chc_adev_bins[MAX_ADEV_BINS+1];
   EXTERN struct BIN chc_hdev_bins[MAX_ADEV_BINS+1];
   EXTERN struct BIN chc_mdev_bins[MAX_ADEV_BINS+1];
   EXTERN struct BIN chc_tdev_bins[MAX_ADEV_BINS+1];
   EXTERN struct BIN chd_adev_bins[MAX_ADEV_BINS+1];
   EXTERN struct BIN chd_hdev_bins[MAX_ADEV_BINS+1];
   EXTERN struct BIN chd_mdev_bins[MAX_ADEV_BINS+1];
   EXTERN struct BIN chd_tdev_bins[MAX_ADEV_BINS+1];

   EXTERN double global_adev_max;   // max and min values found in all the adev bins
   EXTERN double global_adev_min;

   EXTERN long adev_q_in;         // aaabbb
   EXTERN long adev_q_out;
   EXTERN long adev_q_count;
   EXTERN int adev_disp;          // says when to update adev display
   EXTERN int adev_time;
   EXTERN int last_bin_count;      // how many adev bins were filled

   EXTERN OFS_SIZE *pps_adev_q;        // queue of points to calc adevs over
   EXTERN long pps_adev_q_in;
   EXTERN long pps_adev_q_out;
   EXTERN long pps_adev_q_count;
   EXTERN double pps_adev_q_overflow;  // counts how many entries have dropped out of the queue
   EXTERN double pps_adev_time;
   EXTERN int pps_adev_disp;
   EXTERN int last_pps_bin_count;      // how many adev bins were filled
   EXTERN double pps_resid;            // phase residual (derivative)
   EXTERN double last_pps_resid;
   EXTERN double pps_tie;              // Time Interval Error

   EXTERN OFS_SIZE *osc_adev_q;        // queue of points to calc adevs over
   EXTERN long osc_adev_q_in;
   EXTERN long osc_adev_q_out;
   EXTERN long osc_adev_q_count;
   EXTERN double osc_adev_q_overflow;  // counts how many entries have dropped out of the queue
   EXTERN double osc_adev_time;
   EXTERN int osc_adev_disp;           // says when to show new pps adev tables
   EXTERN int last_osc_bin_count;      // how many adev bins were filled
   EXTERN double osc_resid;            // phase residual (derivative)
   EXTERN double last_osc_resid;
   EXTERN double osc_tie;

   EXTERN OFS_SIZE *chc_adev_q;        // queue of points to calc adevs over
   EXTERN long chc_adev_q_in;
   EXTERN long chc_adev_q_out;
   EXTERN long chc_adev_q_count;
   EXTERN double chc_adev_q_overflow;  // counts how many entries have dropped out of the queue
   EXTERN double chc_adev_time;
   EXTERN int chc_adev_disp;
   EXTERN int last_chc_bin_count;      // how many adev bins were filled
   EXTERN double chc_resid;            // phase residual (derivative)
   EXTERN double last_chc_resid;
   EXTERN double chc_tie;

   EXTERN OFS_SIZE *chd_adev_q;        // queue of points to calc adevs over
   EXTERN long chd_adev_q_in;
   EXTERN long chd_adev_q_out;
   EXTERN long chd_adev_q_count;
   EXTERN double chd_adev_q_overflow;  // counts how many entries have dropped out of the queue
   EXTERN int chd_adev_disp;           // says when to show new pps adev tables
   EXTERN double chd_adev_time;
   EXTERN int last_chd_bin_count;      // how many adev bins were filled
   EXTERN double chd_resid;            // phase residual (derivative)
   EXTERN double last_chd_resid;
   EXTERN double chd_tie;

   EXTERN int pps_adevs_cleared;   // flag set if adev queue was reset
   EXTERN int osc_adevs_cleared;   // flag set if adev queue was reset
   EXTERN int chc_adevs_cleared;   // flag set if adev queue was reset
   EXTERN int chd_adevs_cleared;   // flag set if adev queue was reset
   EXTERN int adev_freshened;      // flag set if adev queue wrap caused freshening

   EXTERN int bin_scale;           // adev bin sequence (default is 1-2-5)
   EXTERN int first_show_bin;      // bin number to start displays at (0..n_bins-1)
   EXTERN int adevs_shown;         // flag set if adev tables are on screen
   EXTERN int adev_cols_shown;     // number of columns shown in all_adevs mode
   EXTERN int max_adev_width;      // max col width of the adevs we are showing
   EXTERN int max_bins_shown;      // max number of bins we have filled

   EXTERN double pps_adev_jd0;     // time of the first adev queue entry
   EXTERN double osc_adev_jd0;
   EXTERN double chc_adev_jd0;
   EXTERN double chd_adev_jd0;

   EXTERN int adev_mouse_time;     // used to keep the DOS mouse lively during long adev calculations

   EXTERN int min_points_per_bin;  // number of points needed before we display the bin
   EXTERN int n_bins;              // max number of adev bins we will calculate


   void update_adev_display(int type, int force);
   void show_adev_info(int why);
   void add_adev_point(double osc, double pps, double chc, double chd_offset);
   void add_rcvr_adev_point(int why);
   void add_pps_adev_point(double val, int phase);
   void add_osc_adev_point(double val, int phase);
   void add_chc_adev_point(double val, int phase);
   void add_chd_adev_point(double val, int phase);

   void incr_adev(u08 id, struct BIN *bins);
   void incr_hdev(u08 id, struct BIN *bins);
   void incr_mdev(u08 id, struct BIN *bins);
   void incr_tdev(u08 id, struct BIN *bins);

   int fetch_adev_info(u08 dev_id, struct ADEV_INFO *bins);
   void reset_incr_bins(struct BIN *bins);
#endif   // ADEV_STUFF

EXTERN int jitter_adev;         // if flag set calculate PPS adevs from message timing jitter
EXTERN int measure_jitter;      // flag set if /jm jitter measurement mode enabled
EXTERN int old_idle;            // used to restore settings changed by entering jitter measurement mode
EXTERN int old_poll;
EXTERN int old_jit_adev;

EXTERN long adev_q_size;         // number of entries in the adev queue
EXTERN int adev_q_allocated;
EXTERN int mtie_allocated;
EXTERN u08 user_set_adev_size;

EXTERN int show_error_bars;      // if flag is set, show ADEV error bars
EXTERN double adev_decade_height;// height of adev decades in the plot area

EXTERN double adev_period;       // adev data sample period in seconds
EXTERN double pps_adev_period;   // adev data sample period in seconds
EXTERN double osc_adev_period;   // adev data sample period in seconds
EXTERN double chc_adev_period;   // adev data sample period in seconds
EXTERN double chd_adev_period;   // adev data sample period in seconds
EXTERN int user_set_adev_period;
EXTERN int user_set_pps_period;
EXTERN int user_set_osc_period;
EXTERN int user_set_chc_period;
EXTERN int user_set_chd_period;

EXTERN int no_adev_flag;         // used to disable xDEVs on receivers that use PPS and OSC plots for non-ADEVable values
EXTERN int adev_show_time;       // if flag set, update adev plots
EXTERN u08 keep_adevs_fresh;     // if flag is set,  reset the adev bins
                                 // once the adev queue has overflowed twice

#define MIXED_NONE    0          // MIXED_NONE MUST be 0
#define MIXED_GRAPHS  1
#define MIXED_REGULAR 2
EXTERN u08 mixed_adevs;          // if flag set display normal plots along with all adev types

#define SINGLE_ADEVS 0           // adevs on the main screen (SINGLE_ADEVS MUST be 0)
#define ALL_OSC      1           // all 4 adev types for the selected channel
#define ALL_PPS      2
#define ALL_CHC      3
#define ALL_CHD      4
#define ALL_CHANS    5           // selected adev type for all 4 channels
EXTERN u08 all_adevs;            // show all 4 adev types for the indicated signal
EXTERN u08 aa_val;               // all_adev type to show



//  keyboard cursor and function key codes
#define HOME_CHAR   0x0147
#define UP_CHAR     0x0148
#define PAGE_UP     0x0149
#define LEFT_CHAR   0x014B
#define RIGHT_CHAR  0x014D
#define END_CHAR    0x014F
#define DOWN_CHAR   0x0150
#define PAGE_DOWN   0x0151
#define INS_CHAR    0x0152
#define DEL_CHAR    0x0153

#define F1_CHAR     0x013B
#define F2_CHAR     0x013C
#define F3_CHAR     0x013D
#define F4_CHAR     0x013E
#define F5_CHAR     0x013F
#define F6_CHAR     0x0140
#define F7_CHAR     0x0141
#define F8_CHAR     0x0142
#define F9_CHAR     0x0143
#define F10_CHAR    0x0144
#define F12_CHAR    0x0186

#define ESC_CHAR    0x1B

EXTERN u08  review_mode;         // flag set if scrolling through old data
EXTERN u08  review_home;         // flag set if we are at beginning of data
EXTERN long review;              // where we are looking in the queue
EXTERN long right_time;          // how long the right mouse button has been held
#define RIGHT_DELAY  10          // how many mouse checks before we start right-click mouse scrolling

EXTERN char read_log[256];       // name of log file to preload data from
EXTERN char log_name[256];       // name of log file to write
EXTERN char rinex_name[256];     // name of RINEX file to write
EXTERN char raw_name[256];       // name of raw receiver data dump file to write
EXTERN char prn_name[256];       // name of sat PRN az/el/sig level file to write
EXTERN char ticc_name[256];      // name of TICC data log file to write
EXTERN char thermo_name[256];    // name of environmental senso data log file to write
EXTERN char ticc_sim_name[256];  // name of simulation file to read
EXTERN char sim_name[256];       // name of simulation file to read
EXTERN char rpn_name[256];       // name of calculator use defined functions file
EXTERN char in_name[256];        // name of input file to read
EXTERN char x72_name[256];       // name of X72 setting file
EXTERN char log_text[256];       // text string for log file comments
EXTERN char debug_name[256];     // text string for debug output file
EXTERN char edit_file[2028+1];   // name of file to edit
EXTERN int  edit_file_type;      // if edit_name a file name (0) to edit or run (1)
EXTERN char *log_mode;           // file write mode / append mode
EXTERN FILE *log_file;           // log file I/O handle
EXTERN FILE *rinex_file;         // RINEX file I/O handle
EXTERN FILE *raw_file;           // log file of all raw receiver data
EXTERN FILE *prn_file;           // log file of all sat az/el/sig level data
EXTERN FILE *enviro_file;        // log file of all raw environmental sensor data
EXTERN FILE *ticc_file;          // log file of all raw TICC time interval counter data
EXTERN FILE *debug_file;         // debug message file
EXTERN FILE *ticc_sim_file;      // simulated auxilary TIC inpit data file
EXTERN FILE *lla_file;           // lat/lon/alt log file
EXTERN FILE *hist_file;          // message end arrival time histogram data
EXTERN FILE *temp_script;        // temporary keyboard script file written from '@' commands in heather.ch
EXTERN FILE *x72_file;           // file used to store Symmetricom X72 settings
EXTERN FILE *sim_file;           // simulated receiver input file
EXTERN FILE *rpn_file;           // file of RPN calculator user defined functions
EXTERN FILE *in_file;            // input file
EXTERN int sim_eof;              // flag set when sim file reaches EOF
EXTERN int ticc_sim_eof;         // flag set when ticc sim file reaches EOF
EXTERN int kbd_sim;              // flag set when sim file is read via the "R" keyboard command
EXTERN u08 need_raw_file;        // if flag set, open raw receiver log file on startup
EXTERN u08 need_prn_file;        // if flag set, open prn log file on startup
EXTERN u08 need_enviro_file;     // if flag set, open raw environmental sensor log file on startup
EXTERN u08 need_ticc_file;       // if flag set, open raw TICC log file on startup
EXTERN u08 need_debug_log;       // if flag set, open debug log file
EXTERN u08 need_rinex_file;      // if flag set, open rinex file
EXTERN u08 log_written;          // flag set if we wrote to the log file
EXTERN int rinex_header_written; // flag set when RINEX file header has been written
EXTERN int rinex_obs_written;    // flag set when RINEX observation are being written
EXTERN int obs_data_written;     // flag set when RINEX observations have been writen
EXTERN u08 log_header;           // write log file timestamp headers
EXTERN u08 old_log_format;       // read files written in the old log format
EXTERN u08 log_loaded;           // flag set if a log file has been read
EXTERN u08 log_errors;           // if set, log data errors
EXTERN u08 user_set_log;         // flag set if user set any of the log parametrs on the command line
EXTERN long log_file_time;       // used to determine when to write a log entry
EXTERN int doing_log_dump;       // flags that a log dump being processed
EXTERN u08 adev_log;             // flag set if file read in was adev intervals
EXTERN u08 dump_type;            // used to control what to write
EXTERN int log_stream;           // if set, write the incomming serial data to the log file
EXTERN u08 log_comments;         // set flag to write comments in the log file
EXTERN u08 log_db;               // set flag to log sat signal levels
EXTERN u08 reading_log;          // flag set while reading log file
EXTERN int prn_count;            // counts number of prn_file records written

#define FLUSH_CHAR '*'
EXTERN int raw_flush_mode;       // if flag set, flush raw file contents to disk every byte
EXTERN int log_flush_mode;       // if flag set, flush log file contents to disk every line
EXTERN int rinex_flush_mode;     // if flag set, flush RINEX file contents to disk every line
EXTERN int dbg_flush_mode;       // if flag set, flush debug file contents to disk every line
EXTERN int prn_flush_mode;       // if flag set, flush PRN file contents to disk every line

EXTERN int term_row;             // terminal cursor position
EXTERN int term_col;
EXTERN int monitor_mode;         // set flag for rcvr port monitor display
EXTERN int pause_monitor;
EXTERN int term_hex;             // if set, terminal is ascii hex dump mode
EXTERN int monitor_hex;          // if set, monitor is ascii hex dump mode

#define RINEX   302
#define KML     30
#define GPX     20
#define XML     10
#define HEATHER 0
EXTERN int log_fmt;            // log file format
EXTERN int gpx_track_number;

#define DEFAULT_NAME  "------UNKNOWN-------"
#define DEFAULT_NUM   "======UNKNOWN======="
EXTERN char marker_name[32+1];
EXTERN char marker_number[32+1];
EXTERN char marker_type[32+1];
EXTERN char antenna_type[32+1];     //
EXTERN char antenna_number[32+1];
EXTERN char rinex_site[32+1];
EXTERN double antenna_height;       //
EXTERN double antenna_ns;           //
EXTERN double antenna_ew;           //
EXTERN double rinex_fmt;            // RINEX file format version to write
EXTERN int rinex_fix;               // if flag set try and correct common RINEX data/time errors
EXTERN int use_rinex_cppr;          // if flag set, derive L1 pseudorange from carrier phase data

// user specifed RINEX observations to output
EXTERN char mixed_obs_list[SLEN+1];   // RINEX v2.11 combined GNSS system list
EXTERN char gps_obs_list[SLEN+1];     // RINEX v3.03 GNSS specific system lists
EXTERN char sbas_obs_list[SLEN+1];
EXTERN char glonass_obs_list[SLEN+1];
EXTERN char galileo_obs_list[SLEN+1];
EXTERN char beidou_obs_list[SLEN+1];

EXTERN u08 lla_log;            // flag set if file read in was lat/lon/alt values

#define SCRIPT_NEST 4          // how deep we can nest script files
#define SCRIPT_LEN  128        // how long a script name can be
struct SCRIPT_STRUCT {         // keeps track of script info
   char name[SCRIPT_LEN+1];
   FILE *file;
   int line;
   int col;
   int err;
   int fault;
   u08 pause;
};
EXTERN struct SCRIPT_STRUCT scripts[SCRIPT_NEST];

#define TEMP_SCRIPT  "heathtmp.scr"    // temporary keyboar script file written from '@' commands in heather.cfg
EXTERN char script_name[SCRIPT_LEN+1]; // current script file name
EXTERN int script_nest;         // how many script files are open
EXTERN FILE *script_file;       // script file pointer
EXTERN int script_line;         // the position we are at in the file (for error reporting)
EXTERN int script_col;
EXTERN int script_err;          // flag set if error found in the script
EXTERN int script_fault;        // flag set if scripting needs to be aborted
EXTERN u08 script_pause;        // flag set if script input paused for keyboard input
EXTERN u08 skip_comment;        // set flag to skip to end of line in script file
EXTERN u08 script_exit;         // user hit the key to abort a script
EXTERN u08 fast_script;         // if flag set, run script with delays between key strokes


EXTERN u08 first_sample;   // flag cleared after first data point has been received
EXTERN int have_pri_time;  // TSIP primary timing message seen
EXTERN u08 have_time;      // flag set when a valid time has been received
EXTERN int have_year;
EXTERN int have_osc;
EXTERN int req_num;        // used to sequentially request various minor GPS messages
EXTERN int status_prn;     // used to sequentially request satellite status
EXTERN u08 amu_mode;       // flag set if signal level is in AMU units
EXTERN int amu_flag;       // current amu/dbc mode setting for TSIP devices
EXTERN char *level_type;   // string representing the signal level units

EXTERN int track_count;    // number of sats being used (tracked)
EXTERN int vis_count;      // number of visible sats
EXTERN int drawn_sats;     // number of sats in the sat map
EXTERN int sat_count;      // number of sats being used (visible)
                           // note that show_satinfo() converts this to tracked sat count
EXTERN int jackson_sat_count;  // from Jackson Labs STATUS message

EXTERN int tracked_only;   // if flag set, only show tracked sat info
EXTERN int max_sats;       // max number of sats we can display
EXTERN int last_sat_row;   // the last text row of the sat info display
EXTERN int temp_sats;      // temporary number of sats we can display
EXTERN int ebolt;          // flag set if ThunderBolt-E message seen
EXTERN int last_ebolt;     // used to detect changes in ebolt setting
EXTERN u08 user_set_res_t; // user Forced resolution-T type
EXTERN u08 saw_icm;        // set if RES360 ICM seen


#define NO_SCPI_BREAK 0    // if set, dont sent BREAKs to SCPI receivers (used to avoid putting them into TSIP mode)
EXTERN int enable_terminal; // if flag set, enable serial port terminal mode
EXTERN int buffered_term;  // flag set to buffer keystrokes until CR pressed
EXTERN u08 mini_t;         // flag set if Mini-T message seen
EXTERN u08 nortel;         // flag set to wake up a Nortel NTGxxxx unit
EXTERN u08 saw_nortel;     // flag set if nortel format osc params worked
EXTERN u08 try_ntpx;       // flag set if detecting NTPX
EXTERN u08 saw_ntpx;       // flag set if Nortel NTPX receiver is detected
EXTERN u08 saw_mini;       // flag set if Mini-T receiver is detected
EXTERN u08 luxor;          // Luxor battery/LED analyzer message
EXTERN u08 config_set;

EXTERN int eofs;           // used to tweak things around on the screen if ebolt seen
EXTERN int hide_op_mode;   // flag set when too many items to show in the VER_ROW column

EXTERN u32 new_const;      // keep track of constellation changes

EXTERN int debug;
EXTERN int take_a_dump;
EXTERN int murray;         // testing hack for murray's weirdo GPSDO
                           // (does polled requests for timing messages if the
                           //  receiver does not broadcast them every second)

EXTERN int first_key;      // the first character of a keyboard command
                           // ...used to confirm that user wants to do something
                           // ...dangerous or prompt for the second char of a
                           // ...two char keyboard command


#define ETX 0x03          // TSIP message end code
#define DLE 0x10          // TSIP message start code and byte stuffing escape value

// TSIP message handler error codes
#define MSG_ID      0x1000
#define MSG_END     0x0300
#define MSG_TIMEOUT 0xFF00

EXTERN u16 msg_id;         // TSIP message type code
EXTERN u08 subcode;        // TSIP message type subcode

EXTERN int last_was_dle;   // used to debug dump the serial stream to the log file
EXTERN int kol;

EXTERN u08 flag_faults;
EXTERN int msg_fault;



#define BYTE_ERROR    0x08 // message processing errors (tsip_error)
#define WORD_ERROR    0x10
#define DWORD_ERROR   0x20
#define FLOAT_ERROR   0x40
#define DOUBLE_ERROR  0x80
#define TWORD_ERROR   0x100
#define FP80_ERROR    0x200
#define NAN_ERROR     0x400
#define RANGE_ERROR   0x800
#define END_ERROR     0x1000
#define EOL_ERROR     0x2000
#define CKSUM_ERROR   0x4000
#define OVFL_ERROR    0x8000

EXTERN int tsip_error;     // flags set if error detected in a receiver message
                           //   0x01: msg start seen in middle of message
                           //   0x02: msg end seen in middle of message
                           //   0x04: msg end not seen where expected
                           //   0x08: msg error in byte val
                           //   0x10: msg error in word
                           //   0x20: msg error in dword
                           //   0x40: msg error in float
                           //   0x80: msg error in double
                           //  0x100: msg error in tword (48-bit) val
                           //  0x200: msg error in FP80 val
                           //  0x400: val is a NAN  (not-a-number)
                           //  0x800: val is probably not a valid number (too big/small to be reasonable)
                           // 0x1000: message end code not seen where expected
                           // 0x2000: attempt to get value after end-of-message
                           // 0x4000: message checksum did not match
                           // 0x8000: message too long for input buffer


EXTERN u32 packet_count;   // how many receiver packets we have read
EXTERN u32 timing_seen;    // counter of number of timing messages seen
EXTERN u32 ticc_packets;   // how many TICC packets we have read
EXTERN u32 enviro_packets; // how many environmental sensor packets we have read
EXTERN u32 dac_packets;    // how many ADC packets we have read
EXTERN u32 bad_packets;    // how many packets had known errors
EXTERN u32 math_errors;    // counts known math errors
EXTERN int need_queue_reset;  // flag set when initializing a receiver
EXTERN int crlf_seen;         // flag set when a CR or LF seen from a receiver
EXTERN unsigned long wakeup_tsip_msg;  // flag set if any full receiver message has been received
EXTERN int saw_rcvr_msg;

#define MAX_SAT_DISPLAY 14 // max number of sats to display info for
EXTERN int max_sat_display;


struct SAT_INFO {  // the accumulated wisdom of the ages for each satellite
   u08 health_flag;    // packet 49

   u08 disabled;       // packet 59
   u08 forced_healthy;

   float sample_len;   // packet 5A

   double accum_range;
   double raw_time;
   int state;

   int level_msg;      // the message type that set sig_level
   float sig_level;    // (also packet 47, 5C)
   double code_phase;  // code_phase is also used for carrier phase data on non-Trimble receivers
   double doppler;
   double range;
   double last_code_phase;
   double last_cp;        // last Trimble code phase
   int iii;               // used to calculate Trimble pseudorange
   double last_doppler;   // used for ESIP changing value detection
   double last_range;
   int ca_lli;

   int l1_level_msg;      // the message type that set sig_level
   float l1_sig_level;    // (Ashtech Z12)
   double l1_code_phase;
   double l1_doppler;
   double l1_range;
   double last_l1_code_phase;
   double last_l1_doppler;
   double last_l1_range;
   int l1_lli;

   int l2_level_msg;      // the message type that set sig_level
   float l2_sig_level;    // (Ashtech Z12)
   double l2_code_phase;
   double l2_doppler;
   double l2_range;
   double last_l2_code_phase;
   double last_l2_doppler;
   double last_l2_range;
   int l2_lli;

   int l5_level_msg;      // the message type that set sig_level
   float l5_sig_level;    // (GPS/Glonass L5 data)
   double l5_code_phase;
   double l5_doppler;
   double l5_range;
   double last_l5_code_phase;
   double last_l5_doppler;
   double last_l5_range;
   int l5_lli;

   int l6_level_msg;      // the message type that set sig_level
   float l6_sig_level;    // (Galileo L6 data)
   double l6_code_phase;
   double l6_doppler;
   double l6_range;
   double last_l6_code_phase;
   double last_l6_doppler;
   double last_l6_range;
   int l6_lli;

   int l7_level_msg;      // the message type that set sig_level
   float l7_sig_level;    // (Galiloe L7 data)
   double l7_code_phase;
   double l7_doppler;
   double l7_range;
   double last_l7_code_phase;
   double last_l7_doppler;
   double last_l7_range;
   int l7_lli;

   int l8_level_msg;      // the message type that set sig_level
   float l8_sig_level;    // (Galileo L8 data)
   double l8_code_phase;
   double l8_doppler;
   double l8_range;
   double last_l8_code_phase;
   double last_l8_doppler;
   double last_l8_range;
   int l8_lli;

   float eph_time;     // packet 5B
   u08 eph_health;
   u08 iode;
   float toe;
   u08 fit_flag;
   float sv_accuracy;

   u08 chan;           // packet 5C
   u08 acq_flag;
   u08 eph_flag;
   float time_of_week;
   float azimuth;
   float elevation;
   u08 el_dir;
   u08 az_dir;
   u08 age;
   u08 msec;
   u08 bad_flag;
   u08 collecting;
   u08 how_used;      // (packet 0x5D)
   u08 sv_type;       // (packet 0x5D)

   int tracking;       // 6D
   int visible;
   int osa_snr;
   int osa_state;

   int plot_x;         // where the sat circle was drawn in the sat map
   int plot_y;
   int plot_r;
   int plot_color;

   float sat_bias;     // 8F.A7
   float time_of_fix;
   u08 last_bias_msg;  // flag set if sat info was from last message


   int eph_valid;     // ephemeris data

   double t_ephem;
   int eph_week;
   int codeL2;
   int L2Pdata;
   int sv_accu_raw;
   int sv_health;
   int iodc;
   double tGD;
   double toc;
   double af2;
   double af1;
   double af0;
   double eph_sv_accu;
   int eph_iode;
   int fit_interval;
   double Crs;
   double delta_n;
   double M0;
   double Cuc;
   double e;
   double Cus;
   double sqrtA;
   double eph_toe;
   double Cic;
   double omega_0;
   double Cis;
   double io;
   double Crc;
   double omega;
   double omega_dot;
   double i_dot;
   double axis;
   double n;
   double r1me2;
   double omega_n;
   double odot_n;
};


#define LOCK_LOST  0x80  // lli flags
#define CYCLE_SLIP 0x40
#define Z_LOSS     0x20  // Z tracking lost on Ashtech Z12 receivers
#define P_TRACK    0x10  // tracking P code

EXTERN int user_set_short; // if flag set, only display sat az/el/snr info
EXTERN struct SAT_INFO sat[MAX_PRN+2+1];  // +2 is for sun/moon
EXTERN int max_sat_count;
EXTERN int sat_cols;       // display sat info in two columns
EXTERN int sat_rows;       // max number of rows to show sat info in
EXTERN int ms_row;         // where the "More sats..." message was last shown
EXTERN int ms_col;
EXTERN int plot_prn;       // selects a single sat prn to plot az/el/signal

#define BLINK_HIGHEST (-1)
#define BLINK_PLOT    (-2)
EXTERN int blink_prn;      // selects a single sat prn to blink in the sat map


#define SORT_AZ      'A'
#define SORT_BIAS    'B'
#define SORT_DOPPLER 'D'
#define SORT_ELEV    'E'
#define SORT_STATE   'M'
#define SORT_PRN     'P'
#define SORT_RANGE   'R'
#define SORT_SIGS    'S'
#define SORT_URA     'U'
#define SORT_CARRIER 'W'
EXTERN int sort_by;               // used to sort the sat info display
EXTERN int sort_ascend;
EXTERN int no_raw_table;          // if set don't show clock/carrier/pseudorange in the table

#define UN_ZOOM     0
#define ZOOM_SHARED 1
#define ZOOM_LLA    1
#define ZOOM_AZEL   1
#define ZOOM_INFO   2
#define ZOOM_CLOCK  3
#define ZOOM_ADEVS  4
#define ZOOM_PLOT   5
#define ZOOM_KBD    6
EXTERN u08 zoom_screen;           // full screen is taken over by a special display mode
EXTERN int zoom_fixes;
EXTERN int un_zoom;               // used to restore zoomed screen to its previous state
EXTERN int last_zoom_calc;


EXTERN double pps_offset;         // latest values from the receiver
EXTERN double osc_offset;
//EXTERN double cha_offset;       // from TICC
//EXTERN double chb_offset;
EXTERN double chc_offset;
EXTERN double chd_offset;

EXTERN double cha_ref;            // time interval zero references
EXTERN double chb_ref;
EXTERN double chc_ref;
EXTERN double chd_ref;

EXTERN double pps_phase;
EXTERN double osc_phase;
EXTERN double cha_phase;
EXTERN double chb_phase;
EXTERN double chc_phase;
EXTERN double chd_phase;
EXTERN double cha_ts_ref;
EXTERN double chb_ts_ref;
EXTERN double chc_ts_ref;
EXTERN double chd_ts_ref;
EXTERN double cha_ts_sum;
EXTERN double chb_ts_sum;
EXTERN double chc_ts_sum;
EXTERN double chd_ts_sum;
EXTERN double msgofs_phase;
EXTERN double msgjit_phase;

EXTERN double pps_count;
EXTERN double osc_count;
EXTERN double cha_count;
EXTERN double chb_count;
EXTERN double chc_count;
EXTERN double chd_count;

EXTERN double cha_time2_sum;
EXTERN double chb_time2_sum;
EXTERN double fudge_sum;
EXTERN double cha_time2_count;
EXTERN double chb_time2_count;
EXTERN double fudge_count;
EXTERN double cha_time2;
EXTERN double chb_time2;

EXTERN double last_ticc_v1;       // last channel readings from TICC_RCVR
EXTERN double last_ticc_v2;
EXTERN double last_ticc_v3;
EXTERN double last_ticc_v4;

EXTERN double last_cha_ts;        // used to process TICC debug mode time stamps
EXTERN double last_chb_ts;
EXTERN double last_chc_ts;
EXTERN double last_chd_ts;

EXTERN double last_cha_interval;  // the last time interval from each channel
EXTERN double last_chb_interval;
EXTERN double last_chc_interval;
EXTERN double last_chd_interval;


EXTERN double last_pps_offset;    // previous values from the receiver
EXTERN double last_osc_offset;
EXTERN double last_chc_offset;
EXTERN double last_chd_offset;

#define NOMINAL_FREQ 10.0E6       // default nominal signal input freq
EXTERN double nominal_cha_freq;
EXTERN double nominal_chb_freq;
EXTERN double nominal_chc_freq;
EXTERN double nominal_chd_freq;
EXTERN double cha_phase_wrap_interval;  // used to unrwap time interval phase (generic counters)
EXTERN double chb_phase_wrap_interval;  // used to unrwap time interval phase (generic counters)
EXTERN double chc_phase_wrap_interval;  // used to unrwap time interval phase (generic counters)
EXTERN double chd_phase_wrap_interval;  // used to unrwap time interval phase (generic counters)

EXTERN double cha_phase_wrap_adj;
EXTERN double cha_last_wrap_phase;
EXTERN double chb_phase_wrap_adj;
EXTERN double chb_last_wrap_phase;
EXTERN double chc_phase_wrap_adj;
EXTERN double chc_last_wrap_phase;
EXTERN double chd_phase_wrap_adj;
EXTERN double chd_last_wrap_phase;
EXTERN int user_set_wrap;

#define TS_WRAP_INTERVAL 100.0    // assume timestamps wrap at >= 100.0 seconds
EXTERN double pet_clock;          // used to config PICPET for non-10MHz clock
EXTERN double timestamp_wrap;     // time stamp wrap interval in seconds
                                  // 0 says to reduce time stamps to WRAP_INTERVAL (100.0) seconds.


EXTERN DATA_SIZE dac_voltage;
EXTERN DATA_SIZE uccm_voltage;    // manual DAC control setting for UCCM
EXTERN DATA_SIZE pps_quant;
EXTERN DATA_SIZE temperature;
EXTERN DATA_SIZE temperature2;

EXTERN double pcorr;
EXTERN double uccm_gps_phase;

EXTERN DATA_SIZE lars_pps;        // readings from Lars GPSDO when used with a GPS receiver
EXTERN DATA_SIZE lars_dac;
EXTERN DATA_SIZE lars_temp;
EXTERN int have_lars_pps;
EXTERN int have_lars_dac;
EXTERN int have_lars_temp;


EXTERN int have_sa35_telem;
EXTERN double sa35_tec;
EXTERN double sa35_rf;
EXTERN double sa35_center_freq;
EXTERN double sa35_heater;
EXTERN double sa35_dc;
EXTERN double sa35_tune;
EXTERN double sa35_efc;
EXTERN double sa35_dds;
EXTERN int sa35_efc_enable;
EXTERN int sa35_unlock;
EXTERN char sa35_fw[32+1];
EXTERN char sa35_id[32+1];
EXTERN char sa35_date[32+1];
EXTERN char sa35_model[32+1];
EXTERN char sa35_sn[32+1];


#define HG_PRESS (alt_scale[0] == 'f')  // if true, display pressure in inches of Hg
EXTERN DATA_SIZE humidity;
EXTERN DATA_SIZE pressure;
EXTERN DATA_SIZE tc0, tc1, tc2;   // external temp sensor values
EXTERN DATA_SIZE last_humidity;
EXTERN DATA_SIZE last_pressure;
EXTERN DATA_SIZE adc1,adc2,adc3,adc4;  // ADC channels

EXTERN DATA_SIZE dew_pt;
EXTERN DATA_SIZE rem_temp;
EXTERN DATA_SIZE lux;
EXTERN DATA_SIZE ref_baro_alt;
EXTERN DATA_SIZE lfs_counter;

EXTERN DATA_SIZE last_tc0, last_tc1, last_tc2;   // last external temp sensor values
EXTERN DATA_SIZE last_adc1,last_adc2,last_adc3, last_adc4;

EXTERN int tfom;                  // SCPI time figure-of-merit
EXTERN int have_tfom;
EXTERN int ffom;                  // SCPI freq figure-of-merit
EXTERN int have_ffom;
EXTERN char uccm_led_msg[32];     // response from LED:GPSL? query

EXTERN int gpsdo_ref;             // GPSDO reference source (0=GPS, 1=AUX PPS)
EXTERN int have_gpsdo_ref;

EXTERN int round_temp;            // controls temperature display
EXTERN u08 user_set_rounding;

#define DEFAULT_TICC_MODE  'I'    // default TICC parameters  aaaaaahhhhhh assume time intervals
#define TICC_SYNCMODE 'M'
#define TICC_CAL      20
#define TICC_DILAT    2500
#define TICC_TIMEOUT  5
#define TICC_SPEED    10.0        // (MHz)
#define TICC_COARSE   100.0       // (usec)

#define TICC_TUNE_TIME 1800       // seconds of data to analyze
EXTERN int ticc_tune_time;

EXTERN char ticc_fw[32];          // TICC version info
EXTERN char ticc_sn[32];
EXTERN char ticc_board[32];
EXTERN int ticc_eeprom;
EXTERN int have_ticc_eeprom;
EXTERN int have_ticc_board;

EXTERN int ticc_mode;             // TICC operating mode
EXTERN int have_ticc_mode;
EXTERN int user_set_ticc_mode;

EXTERN int ticc_syncmode;         // TICC sync mode
EXTERN int have_ticc_syncmode;

EXTERN int ticc_cal;              // TICC cal periods
EXTERN int have_ticc_cal;

EXTERN int ticc_timeout;          // TICC timeout
EXTERN int have_ticc_timeout;

EXTERN double ticc_speed;         // TICC clock speed
EXTERN int have_ticc_speed;

EXTERN double ticc_coarse;        // TICC coarse clock speed
EXTERN int have_ticc_coarse;

EXTERN double fudge_a;            // TICC fudge factors
EXTERN double fudge_b;
EXTERN double fudge_c;
EXTERN double fudge_d;
EXTERN int have_ticc_fudge;

EXTERN int edge_a;                // TICC edges
EXTERN int edge_b;
EXTERN int edge_c;
EXTERN int edge_d;
EXTERN int have_ticc_edges;

EXTERN int time2_a;               // TICC time2 values
EXTERN int time2_b;
EXTERN int time2_c;
EXTERN int time2_d;
EXTERN int have_ticc_time2;

EXTERN int dilat_a;               // TICC dilation values
EXTERN int dilat_b;
EXTERN int dilat_c;
EXTERN int dilat_d;
EXTERN int have_ticc_dilat;


#define LOW_LUX 25                // lux sensor integration times
#define MED_LUX 69
#define HI_LUX  191
#define MIN_LUX 2
#define MAX_LUX 191


EXTERN DATA_SIZE batt_v;          // luxor sensor readings
EXTERN DATA_SIZE batt_i;
EXTERN DATA_SIZE led_pos;
EXTERN DATA_SIZE led_neg;
EXTERN DATA_SIZE led_v;
EXTERN DATA_SIZE led_i;
//EXTERN DATA_SIZE adc2;
EXTERN DATA_SIZE emis1, emis2;
EXTERN DATA_SIZE pwm_hz;
EXTERN DATA_SIZE lux1, lux2;
EXTERN DATA_SIZE alt_lux1;
EXTERN u08 lux1_time, lux2_time;
EXTERN u16 batt_pwm;
EXTERN u08 batt_pwm_res;
EXTERN DATA_SIZE PWM_STEP;
EXTERN u32 unit_status;
EXTERN int luxor_hw_ver, luxor_hw_rev;
EXTERN int luxor_sn_prefix, luxor_serial_num;

EXTERN DATA_SIZE batt_w;
EXTERN DATA_SIZE led_w;
EXTERN u08 cal_mode;

#define IR_TIME 10                // 10 second internal resistance measurement cycle
EXTERN DATA_SIZE ir_v, ir_i;
EXTERN int calc_ir;

EXTERN u08 show_mah;              // battery capacity info
EXTERN DATA_SIZE batt_mah;
EXTERN DATA_SIZE batt_mwh;
EXTERN DATA_SIZE load_mah;
EXTERN DATA_SIZE load_mwh;

#define CC_VOLTS     1
#define CC_AMPS      2
#define CC_LIPO      3
#define CC_WATTS     4
#define PWM_SWEEP    5
EXTERN u08 cc_mode;               // constant current/voltage/power and lipo charge mode stuff
EXTERN u08 cc_state;
EXTERN DATA_SIZE lipo_volts;      // desired battery charge voltage
EXTERN DATA_SIZE unsafe_v;        // don't charge batts below this voltage
EXTERN DATA_SIZE cc_val;          // desired load target value
EXTERN DATA_SIZE cc_pwm;          // MOSFET pwm time (0.0 .. 1.0)
EXTERN DATA_SIZE sweep_start;     // PWM sweep limit
EXTERN DATA_SIZE sweep_end;       // PWM sweep limit
EXTERN DATA_SIZE sweep_val;       // PWM sweep current value
EXTERN DATA_SIZE sweep_stop;      // sweep from 0.0 to this value (amps/volta/watts)
EXTERN int sweep_rate;            // sweep speed (seconds per step)
EXTERN int sweep_tick;
EXTERN int update_stop;           // if set, stop screen updates after a sweep

EXTERN DATA_SIZE BLUE_SENS;       // converts Hz to uW/cm^2
EXTERN DATA_SIZE GREEN_SENS;
EXTERN DATA_SIZE RED_SENS;
EXTERN DATA_SIZE WHITE_SENS;

EXTERN DATA_SIZE blue_hz, green_hz, red_hz, white_hz;
EXTERN DATA_SIZE blue_uw, green_uw, red_uw, white_uw;
EXTERN DATA_SIZE r_over_b;
EXTERN DATA_SIZE cct;
EXTERN DATA_SIZE cct_cal, cct1_cal, cct2_cal;
EXTERN u08 cct_type;
EXTERN u08 tcs_color;
EXTERN u08 show_color_hz;
EXTERN u08 show_color_uw;
EXTERN u08 show_color_pct;
EXTERN u08 cct_dbg;
EXTERN u08 cri_flag;


EXTERN DATA_SIZE last_dac_voltage;
EXTERN DATA_SIZE last_temperature;

EXTERN DATA_SIZE last_temp_val;       // used in temp control code
EXTERN u08 temp_dir;                  // direction we are moving the temperature

EXTERN DATA_SIZE stat_count;          // how many data points we have summed
                                      // up for the statistics values


EXTERN DATA_SIZE spike_threshold; // used to filter out temperature sensor spikes
EXTERN int   spike_delay;         // ... when controlling tbolt temperature
EXTERN u08   spike_mode;          // 0=no filtering,  1=filter temp pid,  2=filter all

EXTERN char DEG_SCALE;            // 'C' or 'F'
EXTERN char deg_string[2];
EXTERN char *alt_scale;           // 'm' or 'f'
EXTERN u08 dms;                   // use deg.min.sec (or other) lla format
#define DECIMAL_FMT 0
#define DMS_FMT     1
#define GRIDSQ_FMT  2
#define UTM_FMT     3
#define NATO_FMT    4
#define GRAD_FMT    5
#define RADIAN_FMT  6
#define MIL_FMT     7             // 6400 mils in a circle
#define ECEF_FMT    8

#define DEG_PER_FOOT (2.738065e-6)// (360.0 / (WGS_A * 2.0 * PI * FEET_PER_METER))
EXTERN double ANGLE_SCALE;        // earth size in degrees/ft or meter
EXTERN char *angle_units;         // "ft" or "m "
EXTERN double cos_factor;         // cosine of reference latitude (compensates longitide distance for latitude)

#define BAD_LLA_STRING (-1)       // return values for parse_lla()
#define LLA_OK         0
#define BAD_LAT        1
#define BAD_LON        2
#define BAD_ALT        3
#define LLA_MISSING    4
#define NO_AVERAGE     5

EXTERN double lat;                // receiver position (in radians)
EXTERN double lon;
EXTERN double alt;

EXTERN double ecef_x;             // used in coversion to/from ecef and lat/lon/alt
EXTERN double ecef_y;
EXTERN double ecef_z;

EXTERN double min_q_lat;          // min an max location values seen
EXTERN double max_q_lat;
EXTERN double min_q_lon;
EXTERN double max_q_lon;

EXTERN double speed;              // meters per second
EXTERN double heading;            // degrees from north (true)

EXTERN double ugals;              // vertical gravity change due to sun/moon
EXTERN double last_ugals;
EXTERN double tide_cm;            // solid earth tide in cm
EXTERN double lat_tide;           // solid earth tide displacements in mm
EXTERN double lon_tide;
EXTERN double alt_tide;
EXTERN double last_lat_tide;
EXTERN double last_lon_tide;
EXTERN double last_alt_tide;
EXTERN int have_tides;
EXTERN int have_ugals;
EXTERN int show_tides;
EXTERN int tide_scatter;          // set to 1 when showing earth tide scattergram
EXTERN int user_show_tides;

#define LOW_RES_TIDES   0x01      // if set, use original solid.f sun/moon posn code
#define CALC_MEAN_TIDES 0x02      // if set, restore permanent earth tide offsets
EXTERN int tide_options;          // used to control earth tide calculation options


#define PV_FILTER       0x01
#define STATIC_FILTER   0x02
#define ALT_FILTER      0x04
#define KALMAN_FILTER   0x08
#define IONO_FILTER     0x10
#define TROPO_FILTER    0x20
#define FOLIAGE_FILTER  0x40
#define JAMMING_FILTER  0x80
#define MARINE_FILTER   0x100     // Motorola marine velocity filter
#define FILTER_FILTER   0x8000
EXTERN int have_filter;
EXTERN int plot_filters;          // set flag to show the filters

EXTERN u08 pv_filter;             // dynamics filters
EXTERN u08 static_filter;
EXTERN u08 alt_filter;
EXTERN u08 kalman_filter;
EXTERN u08 iono_filter;
EXTERN u08 tropo_filter;
EXTERN u08 jamming_filter;
EXTERN u08 marine_filter;
EXTERN DATA_SIZE nvs_filter;
EXTERN u08 high8_mode;
EXTERN u08 high6_mode;

EXTERN u08 user_pv;               // user requested dynamics filter settings
EXTERN u08 user_static;
EXTERN u08 user_alt;
EXTERN u08 user_kalman;
EXTERN u08 user_marine;
EXTERN int user_set_filters;

EXTERN float el_mask;             // minimum acceptable satellite elevation
EXTERN float amu_mask;            // minimum acceptable signal level
EXTERN float pdop_mask;
EXTERN float pdop_switch;
EXTERN u08 foliage_mode;          // 0=never  1=sometime  2=always
EXTERN u08 dynamics_code;         // 1=land  2=sea  3=air  4=stationary
EXTERN int smoothing_code;        // 1=quick 2=average 3=smoothed
EXTERN int have_pdop_mask;


#define RCVR_MODE_2D_3D    0
#define RCVR_MODE_SINGLE   1
#define RCVR_MODE_NO_SATS  2
#define RCVR_MODE_2D       3
#define RCVR_MODE_3D       4
#define RCVR_MODE_DGPS     5
#define RCVR_MODE_2DCLK    6
#define RCVR_MODE_HOLD     7
#define RCVR_MODE_PROP     8      // SCPI=holdover
#define RCVR_MODE_ACQUIRE  9      // acquiring
#define RCVR_MODE_BAD_GEOM 10     // bad geometry
#define RCVR_MODE_SURVEY   11     // ZODIAC receivers
#define RCVR_MODE_UNKNOWN  99

#define SET_DIS_MODE_JAMSYNC   0  // mode 0: jam sync
#define SET_DIS_MODE_RECOVER   1  // mode 1: enter recovery
#define SET_DIS_MODE_HOLDOVER  2  // mode 2: manual holdover
#define SET_DIS_MODE_NORMAL    3  // mode 3: exit manual holdover
#define SET_DIS_MODE_DISABLE   4  // mode 4: disable disciplining
#define SET_DIS_MODE_ENABLE    5  // mode 5: enable disciplining

#define DIS_MODE_NORMAL        0  // oscillator discipling modes
#define DIS_MODE_POWERUP       1
#define DIS_MODE_AUTO_HOLD     2
#define DIS_MODE_MANUAL_HOLD   3
#define DIS_MODE_RECOVERY      4
#define DIS_MODE_FAST_RECOVERY 5
#define DIS_MODE_DISABLED      6

#define DIS_MODE_WARMUP        10
#define DIS_MODE_LEARNING      20
#define DIS_MODE_NO_GPS        21
#define DIS_MODE_SOFT_HOLD     22
#define DIS_MODE_ACQUIRE       23
#define DIS_MODE_WAIT          24
#define DIS_MODE_FAILED        25
#define DIS_MODE_UNLOCK        26
#define DIS_MODE_UNKNOWN       0xFF

EXTERN u08 rcvr_mode;          // receiver operating mode
EXTERN u08 last_rmode;
EXTERN u08 configed_mode;
EXTERN u08 venus_hold_mode;
EXTERN int eeprom_save;        // enable config saves to eeprom: 0=SRAM  1=SRAM+EEPROM


#define set_rcvr_mode(x)     set_config((x),  0xFF, -1.0F, -1.0F, -1.0F, -1.0F, 0xFF)
#define set_rcvr_dynamics(x) set_config(0xFF, (x),  -1.0F, -1.0F, -1.0F, -1.0F, 0xFF)
#define set_el_mask(x)       set_config(0xFF, 0xFF, ((x)/(float)RAD_TO_DEG), -1.0F, -1.0F, -1.0F, 0xFF)
#define set_amu_mask(x)      set_config(0xFF, 0xFF, -1.0F, (x),   -1.0F, -1.0F, 0xFF)
#define set_el_amu(el,amu)   set_config(0xFF, 0xFF, ((el)/(float)RAD_TO_DEG), (amu), -1.0F, -1.0F, 0xFF)
#define set_amu_mask(x)      set_config(0xFF, 0xFF, -1.0F, (x),   -1.0F, -1.0F, 0xFF)
#define set_pdop_mask(x)     set_config(0xFF, 0xFF, -1.0F, -1.0F, (x), ((x)*0.75F), 0xFF)
#define set_foliage_mode(x)  set_config(0xFF, 0xFF, -1.0F, -1.0F, -1.0F, -1.0F, (x))

EXTERN int single_sat;
EXTERN int single_sat_prn;   // prn if single sat tracking forced
EXTERN int have_single_prn;
EXTERN u32 sats_enabled;     // bitmap of sats that are enabled
EXTERN u32 excluded_sbas;

EXTERN u08 osc_params;                // flag set if we are monkeying with the osc parameters
EXTERN DATA_SIZE time_constant;       // oscillator control values
EXTERN DATA_SIZE real_time_constant;  // for STAR_RCVR
EXTERN DATA_SIZE damping_factor;
EXTERN DATA_SIZE osc_gain;
EXTERN DATA_SIZE lars_gain;
EXTERN DATA_SIZE log_osc_gain;
EXTERN DATA_SIZE min_volts, max_volts;
EXTERN DATA_SIZE min_dac_v, max_dac_v;
EXTERN DATA_SIZE jam_sync;
EXTERN DATA_SIZE max_freq_offset;
EXTERN DATA_SIZE initial_voltage;
EXTERN DATA_SIZE lars_initv;
EXTERN DATA_SIZE osc_pid_initial_voltage;
EXTERN int gain_color;
EXTERN int pullin_range;    // UCCM device pullin range
EXTERN int have_pullin;

EXTERN DATA_SIZE user_time_constant;  // oscillator control values
EXTERN DATA_SIZE user_damping_factor;
EXTERN DATA_SIZE user_osc_gain;
EXTERN DATA_SIZE user_min_volts, user_max_volts;
EXTERN DATA_SIZE user_min_range, user_max_range;
EXTERN DATA_SIZE user_jam_sync;
EXTERN DATA_SIZE user_max_freq_offset;
EXTERN DATA_SIZE user_initial_voltage;
EXTERN int       user_holdover_time;
EXTERN int user_pullin;

EXTERN int critical_alarms;       // receiver alarms
EXTERN int last_critical;
EXTERN int minor_alarms;
EXTERN int last_minor;
EXTERN int leap_pending;
EXTERN int have_pll;
EXTERN u08 have_alarms;
EXTERN u08 leaped;                // flag set when leapsecond pending flag clears
EXTERN int leap_days;             // number of days until leap second
EXTERN int have_leap_days;
EXTERN char guessed_leap_days;
EXTERN int have_moto_Ba;          // flag set if @@Ba message seen
EXTERN int have_moto_Ea;          // flag set if @@Ea message seen
EXTERN int have_moto_Bg;          // flag set if @@Bg message seen
EXTERN int have_moto_Eg;          // flag set if @@Eg message seen
EXTERN int have_moto_Bk;          // flag set if @@Bk message seen
EXTERN int have_moto_Ek;          // flag set if @@Ek message seen
EXTERN int have_moto_Bn;          // flag set if @@Bn message seen
EXTERN int have_moto_En;          // flag set if @@En message seen
EXTERN int have_moto_Gj;          // flag set if @@Gj message seen
EXTERN int have_moto_Hr;          // flag set if @@Hr message seen
EXTERN int have_moto_range;       // flag set if @@Bg or @@Eg seen
EXTERN int have_scpi_hex_leap;    // flag set if GPST leapdate message seen (on Z3812's is usually correct)
EXTERN int leap_sixty;            // flag set if receiver seconds == 60
                                  // ... used to restore seconds to xx:xx:60 after
                                  // ... converting julian back to gregorian

EXTERN int have_antenna;          // flags that indicate what status into we have
EXTERN int have_osc_age;
EXTERN int have_osc_offset;
EXTERN int have_pps_offset;
EXTERN int have_chc_offset;
EXTERN int have_chd_offset;
EXTERN int have_rcvr_pps;
EXTERN int have_rcvr_osc;

EXTERN int have_temperature;
EXTERN int have_temperature0;  // external sensor readings
EXTERN int have_temperature1;
EXTERN int have_temperature2;
EXTERN int have_humidity;
EXTERN int have_pressure;
EXTERN int have_adc1;
EXTERN int have_adc2;
EXTERN int have_adc3;
EXTERN int have_adc4;

EXTERN int have_dp;
EXTERN int have_rem_temp;
EXTERN int have_ref_baro_alt;
EXTERN int have_lux;
EXTERN int have_lfs_counter;
EXTERN int have_lfs_emis;

EXTERN int have_saved_posn;
EXTERN int have_tracking;
EXTERN int have_sat_azel;
EXTERN int have_leap_info;
EXTERN int have_op_mode;
EXTERN int have_sec_timing;
EXTERN int have_almanac;
EXTERN int have_critical_alarms;
EXTERN int have_eeprom;
EXTERN int have_gps_status;
EXTERN int have_cable_delay;
EXTERN int have_pps_delay;
EXTERN int have_rf_delay;
EXTERN int have_lla_queue;
EXTERN int have_valid_lla;
EXTERN int have_lifetime;
EXTERN int have_nav_rate;
EXTERN int have_pps_rate;
EXTERN int have_z12_pps;
EXTERN int have_z12_ext;
EXTERN int have_pps_enable;
EXTERN int have_pps_polarity;
EXTERN int have_pps_threshold;
EXTERN int have_osc_polarity;
EXTERN int have_pps_mode;
EXTERN int have_tc;
EXTERN int have_damp;
EXTERN int have_gain;
EXTERN int have_dac;
EXTERN int have_initv;
EXTERN int have_minv;
EXTERN int have_maxv;
EXTERN int have_dac_range;
EXTERN int have_jam_sync;
EXTERN int have_freq_ofs;
EXTERN int have_count;
EXTERN int have_utc_ofs;
EXTERN int last_utc_ofs;
EXTERN int have_sirf_pps;
EXTERN int have_tow;
EXTERN int have_week;
EXTERN int have_kalman;
EXTERN int have_heading;
EXTERN int have_speed;
EXTERN int have_state;

EXTERN int have_doppler;       // RINEX file observations
EXTERN int have_phase;
EXTERN int have_range;
EXTERN int have_snr;

EXTERN int have_l1_doppler;
EXTERN int have_l1_phase;
EXTERN int have_l1_range;
EXTERN int have_l1_snr;

EXTERN int have_l2_doppler;
EXTERN int have_l2_phase;
EXTERN int have_l2_range;
EXTERN int have_l2_snr;

EXTERN int have_l5_doppler;
EXTERN int have_l5_phase;
EXTERN int have_l5_range;
EXTERN int have_l5_snr;

EXTERN int have_l6_doppler;
EXTERN int have_l6_phase;
EXTERN int have_l6_range;
EXTERN int have_l6_snr;

EXTERN int have_l7_doppler;
EXTERN int have_l7_phase;
EXTERN int have_l7_range;
EXTERN int have_l7_snr;

EXTERN int have_l8_doppler;
EXTERN int have_l8_phase;
EXTERN int have_l8_range;
EXTERN int have_l8_snr;

EXTERN int have_ubx_rawx;
EXTERN int have_ubx_tmode;
EXTERN int have_accum_range;
EXTERN int have_bias;
EXTERN int have_accu;
EXTERN int have_sawtooth;
EXTERN int have_traim;
EXTERN int have_timing_mode;
EXTERN int have_rcvr_tmode;
EXTERN int have_star_perf;
EXTERN int have_star_wtr;
EXTERN int have_star_atdc;
EXTERN int have_star_input;
EXTERN int have_star_led;
EXTERN int have_star_track;
EXTERN int fix_star_ts;      // if set, attempt to fix STAR4 timestamp errors
EXTERN int have_build;
EXTERN int build_ok;         // TSIP hw version msg build date is OK
EXTERN int have_amu;         // we have a signal level mask
EXTERN int have_el_mask;     // we have an elevation mask
EXTERN int have_scpi_test;   // SCPI rcvr test results
EXTERN int use_traim_lla;         // if flag set, report ZODIAC lla from traim message, else posn message
EXTERN int saw_version;      // flag set if version message seen
EXTERN u08 last_rcvr_mode;   // the previous receiver operating mode
EXTERN u08 have_rcvr_mode;   // flag set if we have the receiver operating mode
EXTERN int rolled;           // flag set if GPS week rollover active
EXTERN double auto_rollover; // automatic rollover compensation value in use
EXTERN int first_request;
EXTERN u08 saw_kalman_on;    // flag set if the dynamics Kalman filter is ever on
EXTERN int saw_timing_msg;   // flag set if Ublox timing receiver messages seen
EXTERN int saw_ubx_tp5;      // flag set if Ublox TP5 message seen
EXTERN int saw_ubx_tp;       // flag set if Ublox TP message seen
EXTERN int saw_diff_change;  // flag set if movement seen in UCCM freq_diff param
EXTERN int scpi_test;        // results from SCPI receiver self test
EXTERN u32 got_timing_msg;   // flag incremented whenever a timing message is processed
EXTERN u32 roll_timing_msg;  // flag incremented whenever a timing message is processed
EXTERN int have_io_options;  // TSIP io options message seen
EXTERN int need_rcvr_init;   // flag set if receiver needs to be initialized
EXTERN int eph_polled;       // bitmask of sats we have polled for ephemeris

EXTERN u08 leap_time;             // flag set if seconds is 60
EXTERN u08 leap_dump;             // do screen dump at leap-second time
EXTERN u08 leap_dumped;           // flag set if leaps image was dumped

EXTERN u08 holdover_seen;         // receiver oscillator holdover control
EXTERN u08 user_holdover;
EXTERN u32 holdover;
EXTERN int scpi_life;             // SCPI reveiver lifetime (in hours)
EXTERN int have_scpi_hold;

EXTERN int scpi_self_test;        // date from STATUS self test line
EXTERN int scpi_int_power;
EXTERN int scpi_oven_power;
EXTERN int scpi_ocxo;
EXTERN int scpi_efc;
EXTERN int scpi_gps;
EXTERN int have_scpi_self_test;
EXTERN int have_scpi_int_power;
EXTERN int have_scpi_oven_power;
EXTERN int have_scpi_ocxo;
EXTERN int have_scpi_efc;
EXTERN int have_scpi_gps;

EXTERN u08 osc_discipline;        // oscillator disciplining control
EXTERN u08 discipline;
EXTERN u08 last_discipline;
EXTERN u08 discipline_mode;
EXTERN u08 last_dmode;


EXTERN int gps_status;            // receiver signal status
EXTERN int last_status;

EXTERN int time_flags;            // receiver time status
EXTERN int last_time_flags;
EXTERN int last_tflags;

EXTERN int set_utc_mode;          // gps or utc time
EXTERN int set_gps_mode;
EXTERN int user_set_time_mode;
EXTERN u08 temp_utc_mode;         // used when setting system time from the tbolt
EXTERN u08 timing_mode;           // PPS referenced to GPS or UTC time
EXTERN int last_tmode;
EXTERN int use_tsc;               // if flag set, use the processor TSC instruction got GetMsecs()

EXTERN int seconds;               // receiver time info (in utc)
EXTERN int minutes;
EXTERN int hours;
EXTERN int day;
EXTERN int month;
EXTERN int year;
EXTERN double raw_frac;

EXTERN int user_set_delta_t;      // flag set if user set UTC delta T value
EXTERN int need_delta_t;          // flag set if time to fetch delta_t from a file
EXTERN double user_delta_t;       // user specified delta T (in days)
EXTERN double delta_ut1;          // difference between UTC and UT1

EXTERN int pri_seconds;           // reciver (UTC) time converted to local time zone
EXTERN int pri_minutes;
EXTERN int pri_hours;
EXTERN int pri_day;
EXTERN int pri_month;
EXTERN int pri_year;
EXTERN double pri_frac;
EXTERN int this_year;             // local time year value

EXTERN int cal_day;               // date of zoomed calendar
EXTERN int cal_month;
EXTERN int cal_year;

EXTERN double jd_utc;             // current Julian date in UTC.
EXTERN double jd_gps;             // current Julian date in GPS time.
EXTERN double jd_tt;              // current Julian date in terrestrial time.
EXTERN double jd_local;           // current Julian date in local time.
EXTERN double jd_display;         // currently displayed Julian date
EXTERN double jd_astro;
EXTERN double jd_leap;            // Julian date of next leapsecond (like 31 Dec 23:59:59)
EXTERN int have_jd_leap;

#define JD_OBS_ROUND 0.0005 //rnx3 // round up jd_obs values by this much, then we truncate the fraction seconds
EXTERN double jd_obs;             // GPS time of raw measurements
EXTERN double obs_tow;            // observation tow
EXTERN int first_obs;

EXTERN int stuck_clock;           // make sure device clock is ticking
EXTERN double last_jd_utc;

EXTERN double last_kbd_jd;        // time of last keypress
EXTERN double last_kbd_msec;      // time since  the keyboard was last checked
EXTERN double idle_time;          // minutes since last keystroke
EXTERN int idle_timeout;          // after this many idle miuntes, zoom screen to "idle_screen"
EXTERN u08 idle_screen;           // screen type to zoom to when idle_time passes

EXTERN int need_sunrise;          // if flag set, calculate sunrise/sunset times
EXTERN int realtime_sun;          // if flag set calclulate sunrise/sunset continuously
EXTERN int play_sun_song;         // if flag set, play the sunrise song at sunrise/sunset
EXTERN double eot_ofs;            // raw equation-of-time value (not compensated for longitude) calculated by eot()
EXTERN char *sunrise_type;        // sunrise/sunset type
EXTERN double sunrise_horizon;    // angle of sunrise from horizon
EXTERN double sunset_horizon;     // angle of sunset from horizon
EXTERN int apply_refraction;      // if flag set, apply refraction correction to sun_posn()

EXTERN double sunrise_az;
EXTERN double sunrise_el;
EXTERN double sunrise_time;       // Julian sunrise time (in UTC)
EXTERN int rise_hh;               // sunrise time (int local time)
EXTERN int rise_mm;
EXTERN int rise_ss;
EXTERN double rise_frac;

EXTERN double sunset_az;
EXTERN double sunset_el;
EXTERN double sunset_time;        // Julian sunset time (in UTC)
EXTERN int set_hh;                // sunset time (int local time)
EXTERN int set_mm;
EXTERN int set_ss;
EXTERN double set_frac;
EXTERN int have_sun_times;        // flag set if sunrise/sunset times are valid
EXTERN int have_local_time;       // flag set if time in not in an astronomical time scale

EXTERN double noon_az;
EXTERN double noon_el;
EXTERN double solar_noon;         // Julian solar zenith time (in UTC)
EXTERN int noon_hh;               // solar noon time (int local time)
EXTERN int noon_mm;
EXTERN int noon_ss;
EXTERN double noon_frac;

EXTERN int do_moonrise;           // if flag set sun_posn() actually returns the moon position

EXTERN double wall_time;          // high precision wall clock time returned by GetNsecs()
EXTERN double time_zero;          // wall_time when the program started
EXTERN int initialized_t0;        // flag set if time_zero has been initialized

EXTERN int g_month;     // Gregorian date gets saved here by gregorian(jdate)
EXTERN int g_day;
EXTERN int g_year;
EXTERN int g_hours;
EXTERN int g_minutes;
EXTERN int g_seconds;
EXTERN double g_frac;

EXTERN int clk_month;   // clock time (int UTC gets saved here by get_clock_time()
EXTERN int clk_day;
EXTERN int clk_year;
EXTERN int clk_hours;
EXTERN int clk_minutes;
EXTERN int clk_seconds;
EXTERN double clk_frac;
EXTERN double clk_jd;
EXTERN double sim_jd;
EXTERN int fake_time_stamp;  // flag set if processing faked UCCM time stamp
EXTERN double fake_msec;     // time to fake a time code update


EXTERN int last_hours;
EXTERN int last_log_hours;
EXTERN int last_second;
EXTERN double this_time_msec;
EXTERN double last_time_msec;
EXTERN double msg_sync_msec;

#define MIN_UTC_OFFSET 17    // assume values less than this are not valid
EXTERN int utc_offset;
EXTERN int last_utc_offset;
EXTERN int utc_offset_flag;  // set if non-zero utc offset seen
EXTERN int user_set_utc_ofs; // user forced the GPS-UTC leapsecond offset value

EXTERN u16 gps_week;
EXTERN u32 tow;              // GPS time of week
EXTERN u32 this_tow;
EXTERN u32 pri_tow;
EXTERN int faked_tow;
EXTERN long survey_tow;
EXTERN u08 csv_char;         // log file value separator
EXTERN double last_msec, this_msec; // millisecond tick clock values

EXTERN int force_day;
EXTERN int force_month;
EXTERN int force_year;
EXTERN u08 fraction_time;
EXTERN u08 seconds_time;
EXTERN int last_day;
EXTERN int last_month;
EXTERN double reinit_jd;          // used to periodically re-init an X11 display
                                  // (many X11 libs have a 32-bit counter that crashes when it rolls over)

EXTERN int new_moon_info;
EXTERN double new_moon_jd;        // julian date of the previous new moon (in UTC)
EXTERN double full_moon_jd;       // julian date of the full moon
EXTERN double blue_moon_jd;       // julian date of the blue moon
EXTERN double black_moon_jd;      // julian date of the black moon
EXTERN double sun_el;             // az/el location of the sun
EXTERN double last_sun_el;
EXTERN int have_sun_el;
EXTERN double sun_az;
EXTERN double sun_decl;
EXTERN double sun_ra;
EXTERN double moon_az;            // az/el location of the moon
EXTERN double moon_el;
EXTERN int last_sun_x;
EXTERN int last_sun_y;
EXTERN int last_sun_r;
EXTERN int drawing_sigs;

#define SEND_SUN     0x01
#define SEND_MOON    0x02
#define SEND_PRN     0x04
#define SEND_SATS    0x08
#define SEND_TIME    0x10
#define SEND_HIGHEST 0x20
EXTERN int track_port_info;        // what data to send out the TRACK_PORT
EXTERN int track_prn;

EXTERN int eclipsed;
EXTERN int eclipse_flag;          // set if eclipse condition detected
EXTERN int last_eclipse;          // previous state of the eclipse flag
EXTERN double eclipse_d1;
EXTERN double eclipse_d2;
EXTERN double eclipse_d3;

// stuff returned by moon_info()
EXTERN double MoonAge;            // Age of moon in days
EXTERN double MoonPhase;          // Illuminated fraction
EXTERN double MoonSynod;          // Synodic month
EXTERN double MoonDist;           // Distance in kilometres
EXTERN double MoonDisk;           // Angular diameter in degrees
EXTERN double MoonPar;            // Moon paralax
EXTERN double SunDisk;            // Sun's angular diameter
EXTERN double SunDist;            // Distance to Sun
EXTERN long lunation;             // Brown's number


EXTERN double tz_adjust;          // time zone offset (without dst correction) expressed as Julian days
EXTERN int time_zone_hours;       // offset from GMT in hours
EXTERN int time_zone_minutes;     // time zone minute offset
EXTERN int time_zone_seconds;     // time zone seconds offset
EXTERN u08 time_zone_set;         // flag set if time zone is in effect
EXTERN int tz_sign;               // the sign of the time zone offset

#define NO_DST      0
#define USA         1
#define EUROPE      2
#define AUSTRALIA   3
#define NEW_ZEALAND 4
#define CUSTOM_DST  5             // must be last entry in dst_list[] (defined in heathmsc.cpp)

#define DST_AREAS   CUSTOM_DST    // number of DST areas in use (4 standard + custom)

EXTERN int dst_area;
EXTERN int dst_ofs;               // daylight savings time correction (in hours)
EXTERN u08 user_set_dst;
EXTERN char custom_dst[64];       // custom daylight savings time zone descriptor
EXTERN double dst_start_jd;       // starting jd (in UTC!) of dst switch
EXTERN double dst_end_jd;         // ending jd (in UTC!) of dst switch

EXTERN u08 use_gmst;              // greenwich mean sidereal time
EXTERN u08 use_lmst;              // local mean siderial time
EXTERN u08 use_ut1;               // UT1 Time
EXTERN u08 use_tai;               // TAI time
EXTERN u08 use_tt;                // TT Terrerstrial Time
EXTERN u08 use_tcg;               // TGT Geocentric Terrerstrial Time
EXTERN u08 use_tdb;               // TDB Barycentric Dynamical Time
EXTERN u08 use_tcb;               // TCB Barycentric Corrdinate Time
EXTERN u08 use_hjd;               // HJD Heliocentric Julian Date
EXTERN u08 use_loran;             // Loran time
EXTERN u08 use_gps;               // GPS time zone name set
EXTERN u08 use_utc;               // UTC time zone name sey
EXTERN u08 use_msd;               // Mars time
EXTERN u08 use_merc;              // Mercury time
EXTERN u08 use_ven;               // Venus time
EXTERN u08 use_pluto;             // Pluto time
EXTERN u08 use_bessel;            // Besselian time
EXTERN u08 solar_time;            // adjust times for equation-of-time
EXTERN double st_secs;            // fractional seconds of sidereal time

#define TZ_NAME_LEN 5
EXTERN char std_string[TZ_NAME_LEN+1];   // standard time zone name
EXTERN char dst_string[TZ_NAME_LEN+1];   // daylisght savings time zone name
EXTERN char tz_string[TZ_NAME_LEN+1];    // current time zone name

EXTERN int dsm[12+1];             // number of days to start of each month

//  version and production info
#define MANUF_PARAMS   0x01
#define PRODN_PARAMS   0x02
#define VERSION_INFO   0x04
#define MFG_INFO       0x08
#define ALL_ID_INFO    (MANUF_PARAMS | PRODN_PARAMS | VERSION_INFO | MFG_INFO)
#define MISC_INFO      0x100
#define INFO_LOGGED    0x200     // flag set when device info has been written to the log

EXTERN int have_info;            // flags what receiver id info has been seen
EXTERN int no_log_id_wait;       // if set, don't wait for receiver id info before logging data
EXTERN int have_seg_info;        // EEPROM segment info

EXTERN u16 sn_prefix;            // from manuf_params message
EXTERN u32 serial_num;
EXTERN u16 build_year;
EXTERN u08 build_month;
EXTERN u08 build_day;
EXTERN u08 build_hour;
EXTERN float build_offset;

EXTERN u08 prodn_options;        // from prodn_params message
EXTERN u08 prodn_extn;
EXTERN u16 case_prefix;
EXTERN u32 case_sn;
EXTERN u32 prodn_num;
EXTERN u16 machine_id;
EXTERN u16 hw_code;

EXTERN u08 ap_major, ap_minor;   // from version_info message
EXTERN u08 ap_month, ap_day;
EXTERN u16 ap_year;
EXTERN u08 core_major, core_minor;
EXTERN u08 core_month, core_day;
EXTERN u16 core_year;

EXTERN int rftg_hw;              // rftg hw vers
EXTERN int rftg_unit;
EXTERN int have_rftg_unit;
EXTERN char rftg_fw[32+1];

EXTERN double rftg_phase_err;    // various data value from rftg device
EXTERN double rftg_phase_ref;
EXTERN double rftg_avg_phase;
EXTERN double rftg_delta_temp;

EXTERN double rftg_ape;
EXTERN double rftg_delta_ape_10m;
EXTERN double rftg_ape2;
EXTERN double rftg_delta_ape;
EXTERN double rftg_const_ape;

EXTERN double rftg_sensitivity;
EXTERN unsigned rftg_elapsed;
EXTERN int rftg_warmup_time;
EXTERN int rftg_avg_sample;
EXTERN int rftg_corr_sched;

EXTERN double rftg_time_corr;
EXTERN double rftg_freq_corr;

EXTERN double ant_v1;
EXTERN double ant_v2;
EXTERN double ant_ma;
EXTERN int have_ant_v1;
EXTERN int have_ant_v2;
EXTERN int have_ant_ma;

EXTERN double rftg_adc;
EXTERN double rftg_dac_5min;
EXTERN double rftg_oscv;

EXTERN int rftg_pll_port;
EXTERN int rftg_pld_port;
EXTERN int have_rftg_lla;


EXTERN char prs_fw[32+1];
EXTERN char prs_sn[32+1];


#define SRO_TICK (1.0 / 7.5E6 * 1.0E9)  // SRO100 internal clock is 7.5 MHz
//#define SRO_DDS_STEP (5.12E-13 * 1.0000130)
#define SRO_DDS_STEP (5.12E-13)

EXTERN char sro_id[32+1];
EXTERN char sro_sn[32+1];
EXTERN int sro_status;
EXTERN double sro_hh;   // efc input
EXTERN double sro_gg;   // reserved (but does change)
EXTERN double sro_ff;   // Rb signal peak voltage
EXTERN double sro_ee;   // photocell voltage
EXTERN double sro_dd;   // varactor voltage
EXTERN double sro_cc;   // heater current
EXTERN double sro_bb;   // cell current
EXTERN double sro_aa;   // not used
EXTERN int sro_tr;
EXTERN int sro_sy;
EXTERN int sro_de;
EXTERN int sro_pw;
EXTERN int sro_fc;
EXTERN int sro_fs;
EXTERN int sro_tw;
EXTERN int sro_aw;
EXTERN int sro_tc;
EXTERN int sro_co;
EXTERN double sro_vs;
EXTERN int sro_vt;
EXTERN int sro_gf;
EXTERN int sro_ra;

EXTERN int sro_l05, sro_l06;
EXTERN int sro_r05, sro_r06;
EXTERN int sro_r14, sro_r15;
EXTERN int sro_r48, sro_r49;
EXTERN int sro_r4F;
EXTERN int sro_countdown;     // (R48/R49) gofast mode countdown timer
EXTERN double sro_user_fc;    // (R05/R06)
EXTERN double sro_freq_lim;   // (R14/R15)
EXTERN double sro_poweron_fc; // (L05/L06)


EXTERN char lpfrs_id[32+1];
EXTERN char lpfrs_fw[32+1];
EXTERN char lpfrs_ck[32+1];

EXTERN int lpfrs_status;
EXTERN int lpfrs_c_rval;  // coarse tune
EXTERN int lpfrs_f_rval;  // fine_tune
EXTERN double lpfrs_freq;

EXTERN double lpfrs_hh;   // efc input
EXTERN double lpfrs_gg;   // reserved (but does change)
EXTERN double lpfrs_ff;   // Rb signal peak voltage
EXTERN double lpfrs_ee;   // photocell voltage
EXTERN double lpfrs_dd;   // varactor voltage
EXTERN double lpfrs_cc;   // heater current
EXTERN double lpfrs_bb;   // cell current
EXTERN double lpfrs_aa;   // not used

EXTERN double sa35_freq;

EXTERN u08 set_osc_polarity;     // command line settable options
EXTERN u08 user_osc_polarity;
EXTERN u08 osc_polarity;

EXTERN u08 set_pps_polarity;
EXTERN u08 user_pps_polarity;
EXTERN int pps_polarity;         // 1=falling  0=rising
EXTERN float pps_threshold;

EXTERN u08 pps_enabled;
EXTERN u08 user_pps_enable;
EXTERN u08 pps_mode;  // 0=off, 1=always on, 2=when tracking, 3=when traim OK

EXTERN u08 pps_rate;
EXTERN u08 user_pps_rate;
EXTERN double pps1_freq;        // programmable PPS output freq and duty cycle
EXTERN double pps1_duty;
EXTERN u32 pps1_flags;
EXTERN double pps2_freq;
EXTERN double pps2_duty;
EXTERN u32 pps2_flags;
EXTERN double zyfer_pps_width;
EXTERN int have_pps_freq;
EXTERN int have_pps_duty;
#define NVS_PPS_WIDTH 1000000
EXTERN u32 nvs_pps_width;       // in nanoseconds
EXTERN u32 last_nvs_pps_width;  // in nanoseconds

EXTERN int esip_pps_width;      // in msecs
EXTERN int esip_pps_type;       // LEGACY or GCLK
EXTERN int have_esip_pps_type;
EXTERN int esip_status;
EXTERN int have_esip_status;
EXTERN int esip_time_source;
EXTERN u32 esip_deleted_gps;
EXTERN u32 esip_deleted_glonass;
EXTERN u32 esip_deleted_galileo;
EXTERN u32 esip_deleted_qzss;
EXTERN u32 esip_deleted_sbas;
EXTERN int have_esip_fixmask;
EXTERN int furuno_type;

EXTERN double ss_pps_width;
EXTERN u32 ss_deleted_gps;
EXTERN u32 ss_deleted_sbas;
EXTERN int ss_intrinsic_delay;

#define VELOCITY_FACTOR 0.66    // assumed cable velocity factor
EXTERN double delay_value;
EXTERN double cable_delay;      // in seconds
EXTERN double rf_delay;         // in seconds
EXTERN double pps1_delay;       // in nanoseconds
EXTERN double pps2_delay;       // in nanoseconds
EXTERN int cable_nsecs;         // if set, SCPI device expects value in ns
EXTERN u08 user_set_delay;

EXTERN int datum;
EXTERN int have_datum;

#define PARAM_TC        0x0001
#define PARAM_DAMP      0x0002
#define PARAM_GAIN      0x0004
#define PARAM_INITV     0x0008
#define PARAM_MINV      0x0010
#define PARAM_MAXV      0x0020
#define PARAM_TBOLT     (PARAM_TC | PARAM_DAMP | PARAM_GAIN | PARAM_INITV | PARAM_MINV | PARAM_MAXV)
#define PARAM_NTPX      (PARAM_TC | PARAM_DAMP | PARAM_GAIN | PARAM_MINV | PARAM_MAXV)

#define PARAM_PULLIN    0x0100
#define PARAM_JAMSYNC   0x0200
#define PARAM_MAXFREQ   0x0400
#define PARAM_MINRANGE  0x0800
#define PARAM_MAXRANGE  0x1000
#define PARAM_HOLDOVER  0x2000
EXTERN int user_set_osc_param; // flags which osc params were on the command line
EXTERN int have_osc_params;    // flags which params we have from the oscillator

EXTERN DATA_SIZE cmd_tc;       // the command line osc param values
EXTERN DATA_SIZE cmd_damp;
EXTERN DATA_SIZE cmd_gain;
EXTERN int cmd_pullin;

EXTERN DATA_SIZE cmd_initdac;
EXTERN DATA_SIZE cmd_minv;
EXTERN DATA_SIZE cmd_maxv;
EXTERN DATA_SIZE cmd_minrange;
EXTERN DATA_SIZE cmd_maxrange;

EXTERN DATA_SIZE cmd_jamsync;
EXTERN DATA_SIZE cmd_maxfreq;
EXTERN int cmd_holdover;

EXTERN u08 user_set_pps_float;
EXTERN u08 user_set_osc_float;
EXTERN u08 user_set_dac_float;

EXTERN u08 user_set_pps_plot;
EXTERN u08 user_set_osc_plot;
EXTERN u08 user_set_dac_plot;
EXTERN u08 user_set_temp_plot;
EXTERN u08 user_set_adev_plot;
EXTERN u08 user_set_watch_plot;
EXTERN u08 user_set_clock_plot;
EXTERN u08 user_set_sat_plot;
EXTERN int user_set_lla_plots;

EXTERN u08 keyboard_cmd;  // flag set if command line option is being set from the keyboard
EXTERN u08 not_safe;      // flag set if user attempts to set a command line option
                          // ... from the keyboard that is not always ok change.
                          // ... 1 = option not processed immediately
                          // ... 2 = option cannot be changed safely


#define SURVEY_SIZE  2000L         // default self-survey size
#define SURVEY_BIN_COUNT 96        // number of precise survey bins
EXTERN int survey_length;          // reported survey size
EXTERN u08 survey_save;            // reported survey save flag
EXTERN long do_survey;             // user requested survey size
EXTERN long do_median_survey;      // user requested precision survey survey size
EXTERN int survey_why;
EXTERN int survey_progress;        // completion percentage of the survey
EXTERN int have_progress;
EXTERN int survey_secs;            // time / samples left in survey mode
EXTERN u08 precision_survey;       // do 48 hour precision survey
EXTERN u08 user_precision_survey;  // flag set if user requested precsion survey from command line
EXTERN u08 precise_survey_done;    // flag set after survey completes - used to keep adev tables off screen
EXTERN int trimble_save;
EXTERN u08 have_initial_posn;
EXTERN u08 surveying;
EXTERN long survey_minutes;
#define doing_survey (minor_alarms & 0x0020)
#define PRECISE_SURVEY_HOURS do_survey

EXTERN u08 time_set_char;          // char used to show time has been set
EXTERN u08 force_utc_time;         // if set, make sure to use UTC time for system clock
EXTERN u08 set_time_daily;         // set CPU clock at 04:05:06 local time every day
EXTERN u08 set_time_hourly;        // set CPU clock at xx:05:06 local time every day
EXTERN u08 set_time_minutely;      // set CPU clock at xx:xx:06 local time every day
EXTERN u08 set_time_once;
EXTERN int set_system_time;        // set flag to keep CPU clock set to GPS receiver clock
EXTERN double set_time_anytime;    // set time if CPU clock millisecond is off by over this value

#define TIME_SYNC_AVG 200.0        // typical time message end time offset
#define TSX_TWEAK (time_sync_offset/1000.0)  // used to adjust clock time for receiver end-of-message offset time
EXTERN double time_sync_offset;    // milliseconds of delay from receiver time message
EXTERN double msg_ofs;
EXTERN int user_set_tsx;           // user set the receiver message vs actual time offset
                                   // ... to clock time (rcvr_time - clock_time)

EXTERN u08 cuckoo;                 // cuckoo clock mode - #times per hour
EXTERN u08 singing_clock;          // set flag to sing songs instead of cuckoo
EXTERN u08 cuckoo_hours;           // set flag to cuckoo the hour on the hour
EXTERN u08 cuckoo_beeps;           // cuckoo clock signal counter
EXTERN u08 ticker;                 // used to flash alarm indicators

EXTERN u08 ships_clock;            // set flag to ring ships bells instead of cuckoo
EXTERN int ring_bell;
EXTERN int bell_number;

EXTERN u08 tick_clock;             // beep every second, alarm sound on the minute
EXTERN int fine_tick_clock;        // flag set if millisecond accurate tick clock enabled
EXTERN double fine_tick_msec;      // time to do a seconds tick
EXTERN int minute_tick;;           // sound the minute tick
#define TICK_SOUND_DELAY 0.0       // milliseconds delay in audio path

EXTERN long egg_timer;             // egg timer mode
EXTERN long egg_val;               // initial egg timer value
EXTERN u08 repeat_egg;             // repeat the egg timer
EXTERN u08 single_egg;
EXTERN u08 alarm_wait;             // if set, wait for timer to expire before continuing script

EXTERN int alarm_time;             // daily alarm time set
EXTERN int alarm_hh, alarm_mm, alarm_ss;
EXTERN int alarm_date;             // alarm date set
EXTERN int alarm_month, alarm_day, alarm_year;
EXTERN double alarm_jd;
EXTERN u08 sound_alarm;            // flag set to sound the alarm
EXTERN u08 single_alarm;           // set flag to play the alarm file only once
EXTERN int modem_alarms;           // flag set to manipulate modem control signals when alarm sounds

EXTERN int dump_time;              // daily screen dump time set
EXTERN int dump_hh, dump_mm, dump_ss;
EXTERN int dump_date;              // screen dump date set
EXTERN int dump_month, dump_day, dump_year;
EXTERN double dump_jd;
EXTERN u08 dump_alarm;
EXTERN long dump_timer;            // egg timer mode
EXTERN long dump_val;              // initial egg timer value
EXTERN long dump_number;           // automatic screen dump counter
EXTERN u08 repeat_dump;            // repeat the screen dump
EXTERN u08 single_dump;
EXTERN u08 do_screen_dump;         // flag set to do the screen dump
EXTERN u08 dump_xml;               // do scheduled log dumps in XML format
EXTERN u08 dump_gpx;               // do scheduled log dumps in GPX format
EXTERN int user_dump_fmt;          // user specified the automatic dump file format

EXTERN int log_time;               // daily log dump time set
EXTERN int log_hh, log_mm, log_ss;
EXTERN int log_date;               // log dump date set
EXTERN int log_month, log_day, log_year;
EXTERN double log_jd;
EXTERN u08 log_alarm;
EXTERN long log_timer;             // egg timer mode
EXTERN long log_val;               // initial egg timer value
EXTERN long log_number;            // automatic screen dump counter
EXTERN u08 repeat_log;             // repeat the screen dump
EXTERN u08 single_log;
EXTERN u08 do_log_dump;            // flag set to do the log dump
EXTERN u08 log_wrap;               // write log on queue wrap

EXTERN int end_time;               // daily exit time set
EXTERN int end_hh, end_mm, end_ss;
EXTERN int end_date;               // exit date set
EXTERN int end_month, end_day, end_year;
EXTERN double exit_jd;
EXTERN long exit_timer;            // exit countdown timer mode
EXTERN long exit_val;              // initial exit countdown timer value
EXTERN u08 repeat_exit;
EXTERN u08 single_exit;

EXTERN int script_time;            // daily script time set
EXTERN int script_hh, script_mm, script_ss;
EXTERN int script_date;            // script date set
EXTERN int script_month, script_day, script_year;
EXTERN double script_jd;
EXTERN long script_timer;          // script countdown timer mode
EXTERN long script_val;            // initial script countdown timer value
EXTERN u08 repeat_script;
EXTERN u08 single_script;
EXTERN u08 do_script_run;          // flag set to run the script

EXTERN int exec_time;              // daily run program time set
EXTERN int exec_hh, exec_mm, exec_ss;
EXTERN int exec_date;              // run date set
EXTERN int exec_month, exec_day, exec_year;
EXTERN double exec_jd;
EXTERN long exec_timer;            // run countdown timer mode
EXTERN long exec_val;              // initial run pgm countdown timer value
EXTERN u08 repeat_exec;
EXTERN u08 single_exec;
EXTERN u08 do_exec_run;            // flag set to run the program
EXTERN int no_exec;                // set flag (/ne) to disable file execution/edit commands


EXTERN u08 show_euro_ppt;          // if flag set,  show exponents on the osc values (default is ppb/ppt)
EXTERN u08 show_euro_dates;        // if flag set,  format mm/dd/yy dates in euro format
EXTERN u08 show_iso_dates;         // if flag set,  format yyyy/mm/dd dates
EXTERN u08 digital_clock_shown;
EXTERN u08 enable_timer;           // enable windows message timer


#define SYM_EEP_FILE      "eeprom.dat"            // base file name of X72 rubidium osc setting file
#define LEAP_FILE         "heather_leapsec.wav"   // played after leapsecond flag clears
#define SUNRISE_FILE      "heather_sunrise.wav"   // played at sun (or moon) rise/set
#define NOON_FILE         "heather_noon.wav"      // played at solar noon
#define CHIME_FILE        "heather_chime.wav"     // chime (cuckoo) clock sound
#define MINUTE_FILE       "heather_minute.wav"    // tick clock minute sound
#define SECONDS_FILE      "heather_second.wav"    // tick clock second sound
#define BELL_FILE         "heather_bell.wav"      // ships bells audible clock sound
#define SONG_NAME         "heather_song%02d.wav"  // singing clock files
#define HOUR_NAME         "heather_hour%02d.wav"  // singing clock files for Big Ben hour
#define NEW_YEAR_NAME     "heather_new_year.wav"  // singing clock file for new year
#define ALARM_FILE        "heather_alarm.wav"     // played when alarm/timer triggers
#define USER_NOTIFY_FILE  "heather_notify.wav"    // alternate chime clock sound file
#define USER_CHORD_FILE   "heather_chord.wav"     // user defined sounds
#define CLICK_FILE        "heather_click.wav"     // touch screen click sound
#define LOCN_FILE         "heather.loc"           // default location
#define RPN_FILE          "heather.rpn"           // calclator user defined functions

#define CHORD_FILE        "c:\\WINDOWS\\Media\\chord.wav"      // standard Windows sound
#define NOTIFY_FILE       "c:\\WINDOWS\\Media\\notify.wav"     // standard Windows sound
EXTERN u08 chord_file;    // flag set if user chord file exists
EXTERN u08 notify_file;   // flag set if user notify file exists

#define LOCK_NAME         "tblock"                // lock file (exists while screen/log dumps in progress)
#define DELTAT_FILE       "deltat.dat"            // user specifyed UT1 delta-T
#define LLA_FILE          "lla.lla"               // precision survey lat/lon/alt fix data
EXTERN char lock_name[256];   // dump file lock file name
EXTERN char dump_prefix[256]; // prefix of timed dump file names

EXTERN u08 chime_file;    // flag set if user chime file exists
EXTERN u08 minute_file;   // flag set if user minute tick file exists
EXTERN u08 seconds_file;  // flag set if user second tick file exists
EXTERN u08 alarm_file;    // flag set if user alarm file exists
EXTERN u08 leap_file;     // flag set if user leapsecond alarm file exists
EXTERN u08 sun_file;      // flag set if user sunrise/sunset tune file exists
EXTERN u08 noon_file;     // flag set if user solar noon tune file exists
EXTERN u08 bell_file;     // flag set if ships bell file exists
EXTERN u08 click_file;    // flag set if click file exists


//
//  com port related stuff
//
#define NUM_COM_PORTS (10+9)    // max number of com ports to use
                                // ports 0 .. 9 are user defined devices
#define RCVR_PORT      10       // the receiver com device
#define ECHO_PORT      11       // echo receiver data to this port
#define NMEA_PORT      12       // echo receiver data in NMEA format to this port
#define TICC_PORT      13       // time interval counter
#define TICC_ECHO_PORT 14       // time interval counter data echo port
#define THERMO_PORT    15       // external environmental sensor (thermometer) port
#define DAC_PORT       16       // external DAC port
#define FAN_PORT       17       // external temperature control fan port
#define TRACK_PORT     18       // outputs moon, sun, and sat  az/el info

#define MONITOR_PORT  RCVR_PORT // default port for monitor mode to show
#define TERM_PORT     RCVR_PORT // default terminal emulator com device
#define ALARM_PORT    RCVR_PORT // default alarm port
EXTERN  unsigned fan_port;      // temperature control device port
EXTERN unsigned monitor_port;   // the port for monitor mode to show
EXTERN unsigned term_port;      // the port for the terminal emulator to talk to


#define RCV_BUF_SIZE  4096      // size of com port data buffers
#define XMIT_BUF_SIZE 4096      // size of com port data buffers

#define COM_TIMEOUT    5000.0   // com port loss detect timeout (in msecs)
#define DETECT_TIMEOUT 1500.0   // timeout to use when auto-detecting receiver type
#define MIN_TIMEOUT    3000.0   // minimum allowable timeout
#define ADEV_TIMEOUT   15000.0  // extend timeout when recalculating adevs

#define COM_INIT_SLEEP 200      // wait this many msecs after opening a com port
                                // ... to allow time for the OS to process it

#define RCV_ERR   0x01          // com[port].com_error flags
#define XMIT_ERR  0x02
#define INIT_ERR  0x04

#define NOT_RUNNING    (-1)     // com[port].com_running
#define UNKN_RUNNING   (0)      // should not be used
#define SERIAL_RUNNING (1)
#define TCPIP_RUNNING  (2)

#define NO_PAR    0             // serial port parity modes
#define ODD_PAR   1
#define EVEN_PAR  2
#define LUXOR_PAR ODD_PAR
EXTERN int parity_defined;

#define USE_DEV_NAME  999       // if usb_port set to this, use the "/dev/heather" name
#define USE_IDEV_NAME 9999      // if usb_port set to this, use the "input device" name
#define HEATHER_DEV   "/dev/heather"

// sendout() eom_flag codes
#define ADD_CHAR  0             // add char to buffer, sends buffer if full
#define EOM       1             // add char to buffer and send buffer
#define FLUSH_COM 2             // sends buffer without adding a new char

struct COM_INFO {
   int com_port;           // COM port number to use
   int usb_port;           // USB serial port number to use (for Linux ttyUSB#)
   int com_fd;             // linux com port file handle
   char com_dev[256+1];    // linux serial port device name
   char IP_addr[256+1];    // TCP/IP server address specified with /ip= parameter

   int baud_rate;          // serial port baud rate
   int data_bits;
   int stop_bits;
   int parity;             // 0=none 1=odd 2=even

   u08 xmit_buffer[XMIT_BUF_SIZE]; // data buffers
   u08 rcv_buffer[RCV_BUF_SIZE];
   DWORD rcv_byte_count;   // number of bytes in the receive buffer
   DWORD next_rcv_byte;    // next byte to get out of the receive buffer

   int user_disabled_com;  // flag set when /0 command line option used
   int port_used;          // 0=never opened,  1=port now closed, 2=open for serial, 3=open for TCP/IP
   int com_running;        // flag set >0 if com port has been initialized
   int process_com;        // clear flag to not process the com port data on startup
                           // useful if just reviewing a log file and no tbolt is connected

   double com_timeout;     // com port loss-of-data timeout in msecs
   double user_timeout;    // user set com timeout value
   double last_com_time;   // the last time com port data was seen
   u08 com_data_lost;      // flag set if com port data stream interrupted
   u08 com_error;          // the last com port error
   u08 rcv_error;          // com port receive error detected (Windows)
   int com_recover;        // if flag set, attempt to recover from com loss by re-initing the port/ip connection

   int user_set_baud;      // flag set if user set the port baud rate
   int user_set_ip;        // flag set if user set the IP address
};

EXTERN struct COM_INFO com[NUM_COM_PORTS];   // the com port related variables



EXTERN double user_com_timeout;  // user set com timeout - to oveeride the default timeout
EXTERN int timeout_extended;     // flag set if com timeout has been extended for special commands
EXTERN int restart_count;        // the number of times a com port restart has been attempted
EXTERN int did_init1;            // flag set if init_message(1) setup the device

const S32 DEFAULT_PORT_NUM = 45000;  // TCP/IP default port number for server connection

EXTERN u32 com_errors;         // counter of com port errors
EXTERN u08 just_read;          // if set, only read TSIP data,  don't do any processing
EXTERN u08 no_send;            // if set, block data from going out the serial port
EXTERN u08 read_only;          // if set, block TSIP commands that change the oscillator config

EXTERN int lpt_port;           // LPT port number to use for temperature control

EXTERN u08 no_poll;            // if set, blocks all polled message requests


EXTERN int break_flag;         // flag set when ctrl-break pressed
EXTERN u08 system_busy;        // flag set when system is processing lots of messages
EXTERN u08 first_msg;          // flag set when processing first message from receiver
                               // (used to get in sync to data without flagging an error)

#define USER_CMD_LEN (256+1)
EXTERN u08 user_init_cmd[USER_CMD_LEN];  // user specified init commands
EXTERN int user_init_len;
EXTERN u08 user_pps_cmd[USER_CMD_LEN];   // user_specified pps commands
EXTERN int user_pps_len;


#define MAX_TSIP 4096               // max length of a receiver message
EXTERN u08 tsip_buf[MAX_TSIP+1];    // incoming message is built here
EXTERN u16 tsip_wptr;               // incoming message write pointer
EXTERN u16 tsip_rptr;               // incoming message read pointer
EXTERN u16 tsip_sync;               // incoming message state

EXTERN u08 ticc_buf[MAX_TSIP+1];    // incoming message is built here
EXTERN int ticc_wptr;
EXTERN int ticc_rptr;
EXTERN int ticc_sync;

EXTERN u08 enviro_buf[MAX_TSIP+1];  // incoming message is built here
EXTERN int enviro_wptr;
EXTERN int enviro_rptr;
EXTERN int enviro_sync;

EXTERN u08 dac_buf[MAX_TSIP+1];     // incoming message is built here
EXTERN int dac_wptr;
EXTERN int dac_rptr;
EXTERN int dac_sync;


#define LLA_X         (LLA_COL+LLA_SIZE/2-1)   // center point of lla plot
#define LLA_Y         (LLA_ROW+LLA_SIZE/2-1)
#define LLA_DIVISIONS 10       // lla plot grid divisons

#define HIDE_LLA_ON_HOLD 0     // what to do with LLA plots in position hold mode
#define KEEP_LLA_ON_HOLD 1

#define KEEP_LLA_SHOW    0     // how to configure showing LLA plots
#define DISABLE_LLA_SHOW 1
#define ENABLE_LLA_SHOW  2

EXTERN double LLA_SPAN;        // feet or meters each side of center
EXTERN int LLA_SIZE;           // size of the lla map window (for DOS should be a multiple of TEXT_HEIGHT)
EXTERN int LLA_MARGIN;         // lla plot grid margins in pixels
EXTERN int lla_step;           // pixels per grid division
EXTERN int lla_width;          // size of data area in the grid
EXTERN int LLA_ROW;            // location of the lla map window
EXTERN int LLA_COL;
EXTERN u08 plot_lla;           // flag set when plotting the lat/lon/alt data
EXTERN int autoscale_lla;
EXTERN double last_lla_span;
EXTERN int lla_showing;        // flag set if lla scattergram is on screen
EXTERN u08 graph_lla;          // flag set when graphing the lat/lon/alt data
EXTERN int need_rebuild_lla;
EXTERN int in_rebuild_lla;

EXTERN int keep_lla_plots;     // flag set if receiver type can do lla plots
EXTERN u08 show_fixes;         // set flag to enter 3D fix mode and plot lla scatter
EXTERN u08 user_fix_set;       // flag set if command line option enabled fix map display
EXTERN u08 fix_mode;           // flag set if receiver is not in overdetermined clock mode
EXTERN u08 reading_lla;        // set flag if reading lla log file
EXTERN u08 reading_signals;    // set flag if reading signal level log file
EXTERN u32 signal_length;      // number of signal level samples logged
EXTERN u08 check_precise_posn; // set flag to do repeated single point surveys
EXTERN u08 check_delay;        // counter to skip first few survey checks
EXTERN u32 precision_samples;  // counter of number of surveyed points
EXTERN u32 lla_points;         // counter of number of points in LLA fix map
EXTERN double precise_lat;     // the precise position that we are at (or want to be at)
EXTERN double precise_lon;
EXTERN double precise_alt;
EXTERN int have_precise_lla;   // flag set >0 if precise lat/lon/alt has been calculated, <=0 if from receiver
EXTERN double ref_lat;         // the reference position for LLA scatter plots
EXTERN double ref_lon;
EXTERN double ref_alt;
EXTERN double default_lat;     // the default location from heather.loc file
EXTERN double default_lon;
EXTERN double default_alt;
EXTERN int user_set_ref_lla;
EXTERN double prop_lat;        // the reference position for propogation delay calcs
EXTERN double prop_lon;
EXTERN double prop_alt;
EXTERN int have_prop_lla;

EXTERN int WATCH_ROW;     // where the AZEL plot goes, if not in the plot area
EXTERN int WATCH_COL;     // ... also where to draw the analog watch
EXTERN int WATCH_SIZE;
EXTERN int ACLOCK_SIZE;
EXTERN int aclock_x,aclock_y;
EXTERN int time_row, time_col;  // where the digital clock is shown
EXTERN int vc_row, vc_col;      // where the last vector char string was drawn

#define ACLOCK_R     (ACLOCK_SIZE/2-TEXT_WIDTH)
#define AA           ((float)ACLOCK_R/8.0F)
#define ALARM_WIDTH  2   // how many degrees wide the alarm time marker is
#define WATCH_HOURS  (12*((watch_face/NUM_FACES)+1))  //12
#define WATCH_STEP   (360/WATCH_HOURS)
#define WATCH_MULT   (WATCH_HOURS/12)
#define hand_angle(x) ((270.0+((x)*6.0)) * PI/180.0)

EXTERN int AZEL_SIZE;     // size of the az/el map window (for DOS should be a multiple of TEXT_HEIGHT)
EXTERN int AZEL_ROW;      // location of the az/el map window
EXTERN int AZEL_COL;
EXTERN int AZEL_MARGIN;   // outer circle margin in pixels
EXTERN int azel_erased;   // flag set when azel area had been erased

#define SHARED_AZEL  (plot_azel && ((AZEL_ROW+AZEL_SIZE) >= PLOT_ROW))
#define AZEL_BOTTOM  (((AZEL_ROW+AZEL_SIZE+TEXT_HEIGHT-1)/TEXT_HEIGHT)*TEXT_HEIGHT)
#define AZEL_RADIUS  ((AZEL_SIZE/2)-AZEL_MARGIN)

EXTERN int AZEL_X, AZEL_Y;   // center point of plot
#define AZ_INCR     5        // drawing resolution of the az circles
#define EL_GRID     30       // grid spacing in degrees
#define AZ_GRID     30

#define AZEL_COLOR  GREY     // grid color (turns red if nothing tracked)
#define AZEL_ALERT  DIM_RED
EXTERN int azel_grid_color;

EXTERN int map_x;            // where the sat map was drawn
EXTERN int map_y;
EXTERN int map_r;


EXTERN u32 this_track;       // bit vector of currently tracked sats
EXTERN u32 last_track;       // bit vector of previously tracked sats
EXTERN u32 this_used;        // bit vector of currently used sats
EXTERN u32 last_used;        // bit vector of previously used sats
EXTERN u32 last_q1, last_q2, last_q3, last_q4;  // tracks sats that are in each quadrant
EXTERN u32 this_q1, this_q2, this_q3, this_q4;
EXTERN u08 shared_plot;      // set flag if sharing plot area with az/el map
EXTERN u08 share_active;     // flag set if plot area is being shared

#define SHARE_WATCH   0x01
#define SHARE_MAP     0x02
#define SHARE_SIGNALS 0x04
#define SHARE_LLA     0x08
EXTERN int shared_item;      // the item in the shared plot area



EXTERN u08 show_min_per_div;  // set flag to show display in minutes/division
EXTERN u08 showing_adv_file;  // flag set if adev file has been loaded
EXTERN u08 old_sat_plot;
EXTERN u08 old_adev_plot;
EXTERN u08 old_keep_fresh;
EXTERN double xxx_adev_period;
EXTERN double old_adev_period;
EXTERN double old_pps_adev_period;
EXTERN double old_osc_adev_period;
EXTERN double old_chc_adev_period;
EXTERN double old_chd_adev_period;
EXTERN u08 debug_screen;
EXTERN char user_view_string[MAX_PRN+2+1];  // +2 is for sun/moon

EXTERN int dac_dac;          // osc autotune state
EXTERN DATA_SIZE gain_voltage;   // initial dac voltage


#define MAX_TEXT_COLS (MAX_WIDTH/8)
#define UNIT_LEN 18            // unit name length
#define NMEA_MSG_SIZE 512

EXTERN int rpn_mode;  // -1=in rpn mode 1=parsing rpn expression  0=normal mode
EXTERN int show_rpn_help;
EXTERN char nmea_msg[NMEA_MSG_SIZE+1];     // nmea message buffer
EXTERN char msg_field[NMEA_MSG_SIZE+1];    // fields extracted from message buffer come here
EXTERN int msg_col;                        // used to get chars from the nmea buffer to the field buffer

EXTERN char brandy_fw[128];
EXTERN char brandy_engine[128];
EXTERN char brandy_test[128];
EXTERN char brandy_osc[128];

EXTERN int brandy_pps_rate;
EXTERN int have_brandy_rate;
EXTERN int brandy_pps_width;
EXTERN int have_brandy_width;

EXTERN char brandy_code1;
EXTERN char brandy_code2;
EXTERN char brandy_code3;
EXTERN char brandy_code4;
EXTERN int have_brandy_code;

EXTERN char tm4_code1;
EXTERN char tm4_code2;
EXTERN int tm4_time_baud;
EXTERN int have_tm4_time_baud;
EXTERN int have_tm4_code;
EXTERN int ett_enable;
EXTERN int have_tm4_ett_enable;
EXTERN int ett_time_format;
EXTERN int have_ett_time_format;
EXTERN int ett_code_format;
EXTERN int have_ett_code_format;
EXTERN char tm4_obj[32+1];
EXTERN char tm4_hex[32+1];
EXTERN char tm4_string[32+1];
EXTERN char tm4_id[32+1];
EXTERN int tm4_lock;
EXTERN int have_tm4_lock;
EXTERN int tm4_antenna;
EXTERN int have_tm4_antenna;
EXTERN int tm4_pps_source;
EXTERN int have_tm4_pps_source;
EXTERN int ett_polarity;
EXTERN int have_ett_polarity;
EXTERN int have_tm4_enhanced;
EXTERN int tm4_width;
EXTERN int have_tm4_width;
EXTERN int tm4_repeat;
EXTERN int have_tm4_repeat;
EXTERN int tm4_pop_mode;
EXTERN int have_tm4_pop_mode;
EXTERN int tm4_pop_polarity;
EXTERN int have_tm4_pop_polarity;
EXTERN double ett_jd;
EXTERN int have_tm4_ett_jd;
EXTERN double pop_jd;
EXTERN int have_tm4_pop_jd;

EXTERN double inst_phase;            // Brandywine freq control message data
EXTERN double phase_control_val;
EXTERN double avg_phase;
EXTERN double freq_control_val;
EXTERN double last_freq_corr;
EXTERN double freq_trend;
EXTERN char last_correction[32+1];
EXTERN int phase_status;
EXTERN int pll_constraint;

EXTERN char blanks[MAX_TEXT_COLS+1];      // 80 of them will blank a screen line
EXTERN char out[MAX_TEXT_COLS+1];         // screen output strings get formatted into here

EXTERN char gpsd_device[MAX_TEXT_COLS+1]; // GPSD device name to select

EXTERN char unit_name[UNIT_LEN+1];        // unit name identifier
EXTERN char scpi_mfg[20+1];
EXTERN char scpi_fw[128];
EXTERN char scpi_sn[128+1];
EXTERN char scpi_model[UNIT_LEN+1];
EXTERN char scpi_serno[NMEA_MSG_SIZE+1];
EXTERN char scpi_mfg_id[NMEA_MSG_SIZE+1];

EXTERN char cs_cbtid[NMEA_MSG_SIZE+1];
EXTERN char cs_glob[NMEA_MSG_SIZE+1];

EXTERN char tp_sw[32+1];
EXTERN char tp_boot[32+1];
EXTERN char tp_num1[32+1];
EXTERN char tp_num2[32+1];
EXTERN char tp_num3[32+1];
EXTERN char tp_sn[32+1];

EXTERN char ss_sw_pn[32+1];
EXTERN char ss_model[32+1];
EXTERN char ss_model_cksum[32+1];
EXTERN char ss_boot_pn[32+1];
EXTERN char ss_rsvd[32+1];
EXTERN char ss_psn[32+1];
EXTERN char ss_rsvd2[32+1];
EXTERN int ss_boot_cksum;
EXTERN int ss_op_cksum;
EXTERN int ss_type;

EXTERN char enviro_dev[UNIT_LEN+1];
EXTERN char enviro_sn[UNIT_LEN+1];

EXTERN int furuno_pgm;
EXTERN int furuno_ver;
EXTERN int furuno_traim;
EXTERN int furuno_fix_mode;
EXTERN int have_furuno_fix_mode;
EXTERN int traim_deleted;
EXTERN int traim_alarm;

EXTERN char esip_device[32+1];
EXTERN char esip_version[32+1];
EXTERN char esip_query[32+1];
EXTERN char esip_model[32+1];

EXTERN double true_eval;  // unknown value from $EXTSTATUS message
EXTERN int have_true_eval;
EXTERN double true_scalefactor;  // unknown value from $GETSCALEFACTOR message
EXTERN int have_true_scale;
EXTERN double atten_val;
EXTERN int have_atten;
EXTERN int true_pot1, true_pot2;
EXTERN int have_true_pot;
EXTERN int true_debug;
EXTERN int have_true_debug;

EXTERN char zy_id[32+1];
EXTERN char zy_prod[32+1];
EXTERN char zy_sn[32+1];
EXTERN char zy_pn[32+1];
EXTERN char zy_fw[32+1];
EXTERN char zy_rev[32+1];
EXTERN char zy_mfgid[32+1];
EXTERN char zy_date[32+1];
EXTERN char zy_gps[32+1];
EXTERN char zy_app_vers[32+1];
EXTERN char zy_app_prog[32+1];
EXTERN char zy_app_date[32+1];
EXTERN char zy_option[32+1];
EXTERN char zy_osc_type[32+1];

EXTERN DATA_SIZE zyfer_drift;
EXTERN int have_zyfer_drift;
EXTERN DATA_SIZE zyfer_tdev;
EXTERN int have_zyfer_tdev;
EXTERN DATA_SIZE zyfer_essn;
EXTERN int have_zyfer_essn;
EXTERN double zyfer_hefe;
EXTERN int have_zyfer_hefe;
EXTERN double zyfer_hest;
EXTERN int have_zyfer_hest;
EXTERN double zyfer_hete;
EXTERN int have_zyfer_hete;

EXTERN int have_zyfer_hint;
EXTERN int zy_hflag;
EXTERN int zy_pll_bad;
EXTERN int zy_gps_bad;
EXTERN int zy_fix_bad;
EXTERN int zy_bias_bad;


#define UBX_ROM_INFO   20
EXTERN char ubx_sw[30];
EXTERN char ubx_hw[10];
EXTERN char ubx_rom[UBX_ROM_INFO+1][30+1];
EXTERN float ubx_fw_ver;

EXTERN char sirf_sw_id[NMEA_MSG_SIZE+1];

#define Z12_SNR_SCALE 5.0   // scale down Z12 SNR values by this factor for Heather plot compatibility
EXTERN char z12_type[32+1];
EXTERN char z12_chan_opt[32+1];
EXTERN char z12_nav_ver[32+1];
EXTERN char z12_rsvd[32+1];
EXTERN char z12_chan_ver[32+1];
EXTERN char z12_site[4+1];


EXTERN char star_module[12+1];        // STAR_RCVR version info
EXTERN char star_article[12+1];
EXTERN char star_sn[12+1];
EXTERN char star_hw_ver[12+1];
EXTERN char star_fw_article[12+1];
EXTERN char star_fw[12+1];
EXTERN char star_layout[12+1];
EXTERN char star_commercial[12+1];
EXTERN char star_test_date[12+1];
EXTERN char star_test_version[12+1];
EXTERN char star_osc[12+1];
EXTERN char star_fpga[12+1];
EXTERN char star_family[12+1];
EXTERN char star_variant[12+1];

EXTERN int star_restart_ok;           // if flag set, send RESTART(W) if com data loss seen
EXTERN int star_wtr;
EXTERN int star_wtr_timer;
EXTERN int star_aux_timer;
EXTERN int star_led;

EXTERN int user_hbsq;         // HBSQ (holdover squelch time)
EXTERN int current_hbsq;      // time remaining before squelch activated
EXTERN int have_star_hbsq;

EXTERN int have_osa_stat;     // !!!! 4531 input status flags
EXTERN int osa_stat1;
EXTERN int osa_stat2;
EXTERN int osa_stat3;
EXTERN int have_osa_prior;    // !!!! 4531 input priority flags
EXTERN int osa_gps_prior;
EXTERN int osa_aux_prior;
EXTERN int have_osa_output;   // !!!! 4531 input priority flags
EXTERN int osa_output_type;
EXTERN int have_osa_aux;      // !!!! 4531 aux input flags
EXTERN int osa_aux_c;
EXTERN int osa_aux_t;
EXTERN int have_osa_adm;      // !!!! 4531 aux input flags
EXTERN int osa_gps_adm;
EXTERN int osa_aux_adm;

EXTERN char venus_kern[32];
EXTERN char venus_odm[32];
EXTERN char venus_rev[32];
EXTERN u32  venus_kernel;
EXTERN int  saw_venus_raw;
EXTERN int  have_venus_timing;

#define ROVER_MODE 0
#define BASE_MODE  1
EXTERN int  rtk_mode;         // Venus receiver RTK mode
EXTERN int  have_rtk_mode;

EXTERN char zod_chans[32];           // Zodiac ID info
EXTERN char zod_sw[32];
EXTERN char zod_date[32];
EXTERN char zod_opt[32];
EXTERN char zod_rsvd[32];

EXTERN char taip_product[NMEA_MSG_SIZE+1];  // TAIP ID info
EXTERN char taip_version[NMEA_MSG_SIZE+1];
EXTERN char taip_core[NMEA_MSG_SIZE+1];
EXTERN char taip_copr[NMEA_MSG_SIZE+1];
EXTERN int taip_id;

EXTERN char watch_name[30+1];        // analog watch brand name
EXTERN char watch_name2[30+1];
EXTERN u08  label_watch_face;        // if set label the analog watch
EXTERN u08  user_set_watch_name;     // if set, the user set the watch name
EXTERN int  fancy_hands;             // if set, use trapezoidal watch hands

EXTERN int user_set_title_bar;       // flag set if user set the title bar name

EXTERN char help_path[MAX_PATH+1];   // tweaked argv[0]

EXTERN char plot_title[SLEN+80+1];   // graph title string ////!!!!80
EXTERN char debug_text[SLEN+80+1];   // used to display debug info in the plot area
EXTERN char debug_text2[SLEN+80+1];  // !!!! debug_text strings must be same length as plot_title
EXTERN char debug_text3[SLEN+80+1];  // !!!! for format_plot_title() to work
EXTERN char debug_text4[SLEN+80+1];  // !!!! for format_plot_title() to work
EXTERN char debug_text5[SLEN+80+1];  // !!!! for format_plot_title() to work
EXTERN char debug_text6[SLEN+80+1];  // !!!! for format_plot_title() to work
EXTERN char debug_text7[SLEN+80+1];  // !!!! for format_plot_title() to work
EXTERN int title_type;
EXTERN u08 greet_ok;
#define NONE      0
#define GREETING  1
#define USER      2
#define OTHER     3

EXTERN char edit_buffer[SLEN+1];     // the text string the user is typing in
EXTERN char last_edit_buf[SLEN+1];   // the text string the user is typing in
EXTERN char last_user_cmd[SLEN+1];   // the last user command sent to the receiver
EXTERN char *edit_info1;             // additional edit prompt info to display
EXTERN char *edit_info2;
EXTERN char *edit_info3;
EXTERN char *edit_info4;
EXTERN char *edit_info5;

#define CALC_CMD  0x0180  // MUST match value in heathui.cpp
EXTERN u16 getting_string;           // the command that will use the stuff we are typing
EXTERN u08 edit_ptr;                 // the edit cursor column
EXTERN int EDIT_ROW, EDIT_COL;       // where to show the edit string

EXTERN char *ppb_string;             // parts per billion identifier
EXTERN char *ppt_string;             // parts per trillion identifier




EXTERN DATA_SIZE test_heat, test_cool;
EXTERN int test_marker;
EXTERN int bang_bang;

EXTERN u08 pid_debug;
EXTERN int crude_temp;
EXTERN DATA_SIZE P_GAIN;
EXTERN DATA_SIZE D_TC;
EXTERN DATA_SIZE FILTER_TC;
EXTERN DATA_SIZE I_TC;
EXTERN DATA_SIZE FILTER_OFFSET;
EXTERN DATA_SIZE KL_TUNE_STEP;
EXTERN DATA_SIZE OLD_P_GAIN;

#define PRE_Q_SIZE  60
EXTERN int osc_prefilter;   // OSC pid pre filter (input values to the PID) depth
EXTERN int opq_in;
EXTERN int opq_count;

#define POST_Q_SIZE 600
EXTERN int osc_postfilter;  // OSC pid post filter (between PID and DAC) depth
EXTERN int post_q_count;
EXTERN int post_q_in;


EXTERN u08 osc_pid_debug;
EXTERN double OSC_P_GAIN;
EXTERN double OSC_D_TC;
EXTERN double OSC_FILTER_TC;
EXTERN double OSC_I_TC;
EXTERN double OSC_FILTER_OFFSET;
EXTERN double OSC_KL_TUNE_STEP;
EXTERN double OLD_OSC_P_GAIN;

EXTERN u08 osc_control_on;
EXTERN double osc_rampup;

EXTERN double avg_pps;
EXTERN double avg_osc;
EXTERN int pps_bin_count;

EXTERN int monitor_pl;              // monitor power line freq
EXTERN unsigned long pl_counter;


EXTERN float dac_drift_rate;
EXTERN u08 show_filtered_values;
EXTERN DATA_SIZE d_scale;

EXTERN double trick_scale;
EXTERN int    first_trick;
EXTERN double trick_value;

#define RMS       0x01     // selection codes of plot statistics to display
#define AVG       0x02
#define SDEV      0x04
#define VAR       0x08
#define SHOW_MIN  0x10
#define SHOW_MAX  0x20
#define SHOW_SPAN 0x40
EXTERN int stat_type;      // current statistic to shoe
EXTERN char stat_id[32];

struct PLOT_DATA {  // info about what's in the plot queue data entries
   char *plot_id;          // the id string for the plot
   char *units;            // the plot data units
   DATA_SIZE ref_scale;    // scale_factor is divided by this to get the units of
                           // measurement to manipulate the scale factor in
   int show_plot;          // set flag to show the plot
   int float_center;       // set flag to allow center line ref val to float
   int plot_color;         // the color to draw the plot in

   DATA_SIZE plot_center;  // the calculated center line reference value;
   int user_scale;         // flag set if user set the scale factor
   DATA_SIZE scale_factor; // the plot scale factor
   DATA_SIZE data_scale;   // data plot scale factor
   DATA_SIZE invert_plot;  // if (-1.0) invert the plot

   DATA_SIZE min_q_val;    // min and max values of the queue data
   DATA_SIZE max_q_val;

   DATA_SIZE sum_x;        // linear regression stuff
   DATA_SIZE sum_y;
   DATA_SIZE sum_xx;
   DATA_SIZE sum_yy;
   DATA_SIZE sum_xy;
   DATA_SIZE stat_count;
   DATA_SIZE a0;
   DATA_SIZE a1;
   DATA_SIZE drift_rate;   // drift rate to remove from plot
   DATA_SIZE sum_change;

   DATA_SIZE min_disp_val; // min and max values of the display window
   DATA_SIZE max_disp_val;
   int   last_y;           // last plot y-axis value

   int old_show;           // previous value of show_plot flag (used to save/restore options when reading a log file)
   int show_trend;         // set flag to show trend line
   int show_deriv;         // set flag to show plot derivative
   int show_freq;          // set flag to show frequency instead of TIE (for TICC's)
   int show_stat;          // set flag to statistic to show
   int last_trend_y;
   int user_set_float;
   int user_set_show;
};
extern struct PLOT_DATA plot[];
EXTERN struct PLOT_DATA sat_stats;
EXTERN int show_tie;

struct RESIDS {
   double sum_x;           // linear regression stuff
   double sum_y;
   double sum_xx;
   double sum_yy;
   double sum_xy;
   double a0;
   double a1;
   double resid_count;
};
EXTERN struct RESIDS resid[NUM_PLOTS+DERIVED_PLOTS];

EXTERN int num_plots;           // how many plots we are keeping data for
EXTERN int extra_plots;         // flag set if any of the extra plots are enabled
EXTERN int selected_plot;       // the currently selected plot
EXTERN int last_selected_plot;  // the last selected plot
EXTERN int show_all_stats;      // if flag set, show all plot stats in the plot window for a selected plot
EXTERN int pre_fft_plot;        // the plot selected before an FFT plot
EXTERN int no_plots;            // if set, disable plot area
EXTERN int dynamic_trend_line;  // if set, update trend line info plot title dynamically
EXTERN int trend_rate_display;  // selects units per day (0) or per hour (1) trend line rate display
                                // or per minute (2) or per second (3) trend line rate display


#ifdef FFT_STUFF
   typedef struct {   /* COMPLEX STRUCTURE */
      float real, imag;
   } COMPLEX;
   int  logg2(long);
   long calc_fft(int id);
   void set_fft_scale(void);

   EXTERN long max_fft_len;    // must be a power of two
   EXTERN long fft_length;     // must be a power of 2
   EXTERN long fft_scale;      // expand power specta bins to this many pixels wide
   EXTERN float fps;           // freqency per sample
   EXTERN u08 fft_db;          // if set, calculate FFT results in dB
   EXTERN long fft_queue_0;
   EXTERN float *tsignal;      // the fft input data
   EXTERN COMPLEX *w;          // the fft w array
   EXTERN COMPLEX *cf;         // the fft cf array
   EXTERN COMPLEX *fft_out;    // the fft results
#endif


//
// function definitions
//
void draw_circle(int x,int y, int r, int color, int fill);
void draw_rectangle(int x,int y, int width,int height, int color);
void fill_rectangle(int x,int y, int width,int height, int color);
void erase_rectangle(int x,int y, int width,int height);
void line(COORD x1,COORD y1, COORD x2,COORD y2, u08 color);
void thick_line(COORD x1,COORD y1, COORD x2,COORD y2, u08 color, u08 thickness);
void xthick_line(COORD x1,COORD y1, COORD x2,COORD y2, u08 color, u08 thickness);
void dot(COORD x,COORD y, u08 color);

int deg_char(COORD row, COORD col, u08 color);
int plusminus_char(COORD row, COORD col, u08 color);
void vidstr(COORD row, COORD col, u08 color, char *s);
u08 get_pixel(COORD x,COORD y);
void redraw_screen(void);
int dot_char(COORD x, COORD y, u08 c, u08 attr);

int config_program(int argc, char *argv[]);
void find_endian(void);
void init_hardware(void);
void shut_down(int reason);
void error_exit(int reason, char *s);
void init_com(unsigned port, int why);
int set_rcvr_baud(int baud, int data_bits, int parity, int stop_bits);
int set_trimble_protocol(int baud,int data_bits,int parity,int stop_bits, int in_prot,int out_prot);
int recover_com(unsigned port);
void kill_com(unsigned port, int why);
void init_enviro(unsigned port);
u64 auto_detect(int baud_only);
void scpi_init(int type);
int status_second(int seconds);

void set_rftg_normal(void);
void start_rftg_request(int flag);
void start_rftg_request2(int flag);
void set_rftg_offset(double val);
void rftg_enable_cpu(void);
void rftg_disable_cpu(void);
void clear_eeprom_data(void);
int show_rftg_info(int row, int col);

int show_lpfrs_info(int row,int col, int max_row);
int show_sa35_info(int row,int col, int max_row);
int show_sro_info(int row,int col, int max_row);
int show_prs_info(int row,int col, int max_row);
int show_prs_status(int row,int col, int max_row);

int show_x72_info(int row,int col, int max_row);
int show_x72_status(int row,int col, int max_row);
void set_x72_creg(int val);
void set_x72_srvc(int val);
void set_x72_efc(int val);
void set_x72_acmos_freq(int val);
void set_x72_discipline(int val);
void set_x72_tc(int val);
void set_x72_damping(double val);
void set_x72_jamthresh(double val);
void set_x72_holdover(int val);
void set_x72_dds(double val);
void set_x72_tic(int val);
void update_x72_tic(int val, int why);
double x72_pps_stats(int dump);
void x72_update_pps_list(int val);
int fix_x72_tics(int val);
void new_x72_interval(int why);
void save_x72_tune(void);
void save_x72_dmode(void);
void save_x72_state(void);
void restore_x72_state(void);
void x72_start_discipline(void);
int x72_fw_discipline(void);
void x72_sw_discipline(void);
double x72_xxx_dds(double val);
double x72_avg_dds(double val);

void train_true_ocxo(void);

void sendout(unsigned port, u08 val, int eom_flag);
void bios_char(u08 c, u08 attr);
void scr_rowcol(COORD row, COORD col);
int check_incoming_data(unsigned port);

void init_screen(int why);
void config_screen(int why);
void new_screen(u08 c);
void change_screen_res(void);
void edit_screen_res(void);
void config_sat_count(int sat_count);
void config_sat_rows(void);
void config_lla_zoom(int why);
void config_enviro_plots(int enable);
void set_restore_size(void);
void set_monitor_mode(int kbd_flag);
void set_title_bar(int why);
void config_prn_plots(int enable);
void config_tide_plots(int enable);
void config_enviro_plots(int enable);
int screen_active(void);

int dump_screen(int invert, int top_line, char *fn);
int dump_bmp_file(int invert, int top_line);
int dump_gif_file(int invert, int top_line, FILE *file);
void erase_cursor_info(void);
void play_tune(char *file, int add_root);
void play_user_sound(char *fn);
void sound_tick_clock(void);
char *trim_whitespace(char *p);


void draw_plot(u08 refresh_ok);
void update_plot(int draw_flag);
void erase_plot(int full_plot);
void erase_help(void);
void erase_screen(void);
void erase_vchar(int x, int y);
void scale_plots(void);
void show_last_mouse_x(int color);
int  get_mouse_info(void);
void trim_plot_queue(int trim);
void deglitch_queue_point(void);
void deglitch_plot_queue(int id, DATA_SIZE sigma);
long calc_plot_hist(int i);
void dump_fft_plot(void);
void dump_hist_plot(void);
int  vchar_stroke(void);
void vchar(int xoffset,int yoffset, u08 erase, u08 color, u08 c);
void vchar_string(int row, int col, u08 color, char *s);
void center_vstring(int row, int scale, int color, char *s);
void reset_vstring(void);
DATA_SIZE scale_temp(double t);
DATA_SIZE scale_pressure(double t);
char *fmt_temp(double t);
char *fmt_pressure(double pressure);
char *dms_fmt(double x);
void restore_plot_config(void);
void filter_spikes(void);
void set_alt_filter(u08 mode);
void adev_mouse(void);
void view_all(int set_user_view);

void alloc_queues(void);
void alloc_plot(void);
void alloc_adev(void);
void alloc_fft(void);
void alloc_gif(void);
void reset_queues(int queue_type, int why);
void new_queue(int queue_type, int why);
void clear_all_data(void);
void write_q_entry(FILE *file, long i);
long find_event(long i, u16 flags);
long goto_event(u16 flags);
void put_plot_q(long i, struct PLOT_Q q);
long next_q_point(long i, int stop_flag);
struct PLOT_Q get_plot_q(long i);
struct PLOT_Q filter_plot_q(long i);
long disp_filter_size(void);
DATA_SIZE calc_cct(int type, int undo_scale, double red, double green, double blue);
void set_cct_id(void);
void update_check(void);
void save_adev_period();
void restore_adev_eriod();

void do_gps(void);
S32 serve_gps(int why);
void get_pending_gps(int why);
int fake_missing_second(double delta);
void fake_second_check(int why);
void key_wait(char *s);
void wait_for_key(int serve);
void abort_wakeup(void);
void init_messages(int why, int reset_ok);
void reset_parser(void);
void get_rcvr_message(void);
void get_ticc_message(unsigned port);
int ticc_port_open(void);
int two_ticc_mode(void);
u08 check_tsip_end(u08 report_err);
void get_unknown_msg(u16 msg_id);
void echo_stream(unsigned c);
void drain_port(unsigned port);

int dac_port_open(void);
void get_dac_message(unsigned port);

void get_enviro_message(unsigned port);
int enviro_mode(void);
int enviro_port_open(void);
int two_enviro_mode(void);

void set_survey_params(u08 enable_survey,  u08 save_survey, u32 survey_len);
void start_self_survey(u08 val, int why);
void stop_self_survey(int why);
void request_rcvr_info(int why);
void set_filter_config(u08 pv, u08 stat, u08 alt, u08 kalman, u08 marine, int save);
void set_filter_factor(DATA_SIZE val);
void set_nav_rate(DATA_SIZE hz);
void request_cold_reset(void);
void set_discipline_mode(u08 mode);
void update_osc_params(void);
void save_segment(u08 segment, int why);
void revert_segment(u08 segment);
void save_prs_param(char *s);
void request_timing_mode(void);
void set_timing_mode(u08 mode, int why);
void get_timing_mode(void);
void request_factory_reset(void);
void request_self_tests(int test);
void request_version(void);
void request_gps_system_msg(void);
void set_pps(u08 pps_enable,  u08 pps_polarity,  double cable_delay, double pps_delay, float threshold, int save);
void set_pps_freq(int chan, double freq, double duty, int polarity, double cable_delay, double pps_delay, int save);
void set_ref_input(int source);
void set_time_code(char mode);
void set_pps_source(int source);
void request_warm_reset(void);
void enable_moto_binary(void);
void enable_moto_nmea(void);
void set_osc_sense(u08 mode, int save);
void set_dac_voltage(DATA_SIZE volts, int why);
void calc_osc_gain(void);
void request_discipline_params(u08 type);
void request_all_dis_params(void);
void set_prs_ep(int val);
void set_prs_ga(int);
void set_prs_mo(int val);
void set_prs_ms(int val);
void set_prs_pf(int);
void set_prs_pi(int);
void set_prs_pt(int);
void set_prs_pp(int val);
void set_prs_sf(int val);
void set_prs_sp(int r, int n, int a);
void set_prs_to(int val);

void set_sro_width(double usecs);
void set_sro_delay(double nsecs);
void set_sro_track_window(double nsecs);
void set_sro_alarm_window(double nsecs);
void set_sro_tc(int secs);
void set_sro_sy(int secs);
void set_sro_tr(int secs);
void set_sro_fs(int secs);
void set_sro_gf(int secs);
void set_sro_raw(double nsecs);
void set_sro_co(double nsecs);
void set_sro_fc(double ppb);
void set_sro_clock(void);

void set_lpfrs_freq(double freq);
void set_sa35_freq(double freq);
void set_sa35_efc(int mode);

void set_star_wtr(int secs);
void set_star_hbsq(int secs);
void set_star_prior(int p1, int p2);
void set_star_output(int mode);
void set_star_adm(int gps, int aux);
void set_star_aux_inp(int c, int t);
void clear_star_wtr();

void set_discipline_params(int save);
void request_survey_params(void);
void set_pullin_range(int i);
void request_sat_list(void);
void request_sig_levels(void);
void request_primary_timing(void);
int fix_month(int month);
void request_secondary_timing(void);
void secondary_timing(int get_tsip);
void request_pps_info(void);
void send_byte(u08 val);
void wakey_wakey(int force_sirf);
void do_fixes(int mode);
void change_fix_config(int save);
void change_zoom_config(int save);
void cancel_zoom(int why);
void remove_zoom(void);
void start_3d_fixes(int mode, int why);
void config_fix_display(void);
void set_config(u08 mode, u08 dynamics, float elev, float amu, float pdop_mask, float pdop_switch, u08 foliage);
void request_rcvr_config(int why);
void set_single_sat(int prn);
void exclude_sat(int prn);
void update_disable_list(u32 val);
int  highest_sat(void);
void set_rcvr_utc_ofs(int i);
DATA_SIZE average_dop(void);
void set_io_options(u08 posn, u08 vel, u08 timing, u08 aux);
void request_io_options(void);
void start_self_survey(u08 val);
void save_segment(u08 segment);
void saw_ebolt(void);
void set_cal_mode(u08 cal_mode);
void set_gnss_system(int system);
void request_gnss_system(void);

void set_cs_disp(int mode);
void set_cs_remote(int mode);
void set_cs_freq(int chan, double freq);
void set_cs_leap(double jd, int dur);
void set_cs_slew(double val);
void set_cs_ster(double offset);
void set_cs_standby(int mode);
void set_cs_sync(char *s);
void set_cs_time(char mode);
void clear_cs_log(void);
void show_cs_log(int row,int col, int extended);

void set_rcvr_smoothing(int val);

int get_clock_time(void);

void show_cursor_info(S32 i);
void show_title(void);
void show_stat_info(void);
int show_digital_clock(int row, int col);
int show_greetings(void);
void show_log_state(void);
void show_serial_info(void);
void show_time_info(void);
void show_satinfo(void);
void dump_prn_info(void);
void show_sun_moon(int row, int col);
void show_sun_azel(int row, int col);
int track_object(void);
void show_moon_azel(int row, int col);
void show_pid_values(void);
void show_osc_params(int row,int col);
int  show_brandy_info(int row, int col);
void show_osc_pid_values(void);
void show_manuf_params(void);
void show_unit_info(void);
void show_version_header(void);
void show_version_info(void);
void show_gpsd_driver(void);
void show_ebolt_info(void);
void show_param_values(int all);
int adevs_active(int check_enable);

void start_3d_fixes_survey(void);
void start_precision_survey(int why);
void stop_precision_survey(int why);
void abort_precise_survey(int why);
void save_precise_posn(int force_save);
int add_to_bin(double val,  double bin[],  int bins_filled);
void plot_lla_point(int draw, int color);
void rebuild_lla_plot(int draw);
double calc_lla_span(void);
void change_lla_scale(void);
void show_mouse_lla(void);
void precise_check(void);
int add_survey_point(void);
void analyze_hours(void);
void calc_precise_lla(void);
void update_precise_survey(void);

void plot_lla_axes(int why);
void lla_header(char *s, int color);
void set_lla(double lat, double lon, double alt);
int read_default_lla(void);
int read_rpn_file(char *s, int erase);
void format_lla(double lat,double lon,double alt, int row,int col, int disp_tide);
void erase_lla(void);
void clear_signals(void);
void clear_lla_points(int force_clear);
void update_mouse(void);
void hide_mouse(void);
u08 mouse_hit(COORD x1,COORD y1,  COORD x2,COORD y2);
void open_lla_file(int why);
void close_lla_file(int cleanup);
int check_com_timer(unsigned port);
void reset_com_timer(unsigned port);
void extend_com_timeout(double timeout);
void reset_kbd_timer(void);
int serial_input_check(void);

void show_plot_grid(void);
DATA_SIZE get_stat_val(int id);
void draw_maps(void);
void clear_maps(void);
void draw_azel_plot(int why);
void draw_signal_map(int why);
void check_azel_changes(void);
void clear_sat_trails(void);
void update_sat_trails(void);
void erase_azel(int why);
void dump_trails(void);
void plot_3d_sig(int az, int el);
void log_signal(float azf, float elf, float sig_level, int amu_flag);
void freshen_sig_info(void);
void log_sun_posn(void);
void dump_signals(char *fn);
float amu_to_dbc(float sig_level);
float dbc_to_amu(float sig_level);
void label_circles(int signals, int row);

void format_plot_title(char *plot_title);
FILE *open_log_file(char *mode);
void close_log_file(void);
void sync_log_file(void);
void dump_log(char *name, u08 dump_size);
void write_log_leapsecond(void);
void write_log_readings(FILE *file, long i);
void write_log_changes(void);
void write_log_tow(int spaces);
void write_log_error(char *s, u32 val);
void write_log_utc(int utc_offset);
void write_log_comment(int spaces);
void write_rinex_header(FILE *file);
void write_rinex_obs(FILE *file);
int rinex_gnss(int prn);
char gnss_letter(int prn);
double rinex_cppr(int prn, double range, double carrier);
void log_posn_bins(void);
void log_saved_posn(int type);
void log_adevs(void);
void log_stats(void);
int calc_sunrise(double delta, int why);
int calc_moonrise(void);
u32 fake_tow(double jd);
int reload_log(char *fn, u08 cmd_line);
int time_check(int reading_log, DATA_SIZE interval, int yy,int mon,int dd, int hh,int mm,int ss, double frac);
double GetMsecs(void);
double GetNsecs(void);
void serve_vfx(void);
void serve_os_queue(void);

void need_time_set(void);
void init_dsm(int year);
int leap_year(int year);
void bump_time(void);
void set_time_zone(char *s);
int adjust_tz(int why);
void calc_dst_times(int year, char *s);
void moon_posn(double jd_tt);
double sun_posn(double jd, int do_moon);
double sun_diam(double jd);
double moon_diam(double jd);
int eclipse(void);
double grav_force(double jd);
void record_sig_levels(int prn);
int dst_offset(void);
void setup_calendars(int why);
void calc_seasons(void);
double new_moon(double jd);
void calc_moons(double jd, int why);
void erase_watch(void);
double jdate(int y, int m, int d);
double jtime(int hh, int mm, int ss, double frac);
double mars_date(int y,int m,int d, int hh,int mm,int ss, double frac);
double gmst(int yy, int mo, int dd, int hh, int mm, int ss, int app_flag);
double lmst(int yy, int mo, int dd, int hh, int mm, int ss, int app_flag);
double eot(double jd);
double utc_delta_t(void);
double get_alarm_time(void);
void set_alarm(char *alarm);
void enable_alarm(void);
void reset_alarm(void);
void set_watch_name(char *s);
char *tz_info(void);
double time_zone(void);

void BEEP(int why);
int do_kbd(int c);
void show_queue_info(void);
void show_touch_kbd(int erase);
void hide_kbd(int why);
void reset_first_key(int why);
void kbd_help(void);
void text_editor(void);
void exec_program(void);
void run_program(char *pgm, int no_path);
int are_you_sure(int c);
int sure_exit(void);
u08 toggle_value(u08 x);
int option_switch(char *arg);
int read_config_file(char *name, u08 local_flag, u08 add_path);
void config_options(void);
void save_cmd_bytes(char *arg);
void go_full(void);
void command_help(char *where, char *s, char *cfg_path);
u32 get_bgr_palette(int i);
void setup_palette(void);
void edit_cmap(int color, int r, int g, int b);
void change_palette(char *s);
void invert_screen(void);
void set_sat_white(void);
void set_defaults(void);
double hp53131 (char *line);
void close_script(u08 close_all);
int get_script(void);
int adjust_screen_options(void);
void toggle_plot(int id);
int read_calendar(char *s, int erase);
int calendar_count(void);
void read_signals(char *s);
void read_prn(char *s);
int good_el_level(void);
int good_sig_level(void);
float set_el_level(void);
int parse_lla(char *s);
void set_atten(double val);
void set_antenna(int i);
FILE *topen(char *s, char *m);
FILE *open_raw_file(char *s, char *m);
FILE *open_prn_file(char *s, char *m);
FILE *path_open(char *s, char *m);
FILE *open_debug_file(char *s);
FILE *open_rinex_file(char *s);
void sync_file(FILE *file);
int path_unlink(char *s);
int open_script(char *fn);

u08 string_param(void);
void edit_ref(void);
void edit_scale(void);
int edit_error(char *s);
int start_edit(int c, char *prompt);
u08 edit_string(int c);
void set_osc_units(void);
void set_traim_mode(int i, int save);
void show_trend_info(int plot);

void check_end_times(void);
void get_delta_t(void);
void cuckoo_clock(void);
void alarm_clock(void);
void click_sound(int why);
void silly_clocks(void);
void calc_greetings(void);
void set_cpu_clock(void);
double time_sync_median(double delta);
int draw_watch(int why);
char *fmt_date(int big_flag);
void gregorian(int no_tweak, double jd);
void set_sat_azel(int prn, float az, float el);
double moon_phase(double jd);
double moon_info(double jd);
int lookup_new_moon(double jd);
double moon_synod(double jd);
double moon_age(double jd);
double earth_radius(double lat, double alt);
double earth_sun_dist(double jd);
void debug_date(char *s, double jd);
int find_delay(double lat1,double long1,double lat2,double long2,double h, double u_dist, double *dist,double *delay);
float check_float(float);
double check_double(double);
int get_msg_field(void);

float scale_adev(float val);
void reset_adev_bins(void);
void recalc_adev_info(void);
void reload_adev_queue(int reset_phase);
void force_adev_redraw(int why);
long next_tau(long tau, int bin_scale);
void find_global_max(void);

void plot_review(long i);
void do_review(int c, int why);
void end_review(u08 draw_flag);
void zoom_review(long i, u08 beep_ok);
void kbd_zoom(void);
void zoom_calc(void);
void zoom_cal(int year, int month);
u08 day_of_week(int d, int m, int y);
int is_holiday(int pri_year,int pri_month,int pri_day);
void goto_mark(int mark);
void reset_marks(void);
void adjust_view(void);
void new_view(void);
int edit_user_view(char *s);
void show(char *s);
void calc_queue_stats(int stop);

void SetDtrLine(unsigned port, u08 on);
void SetRtsLine(unsigned port, u08 on);
void SetBreak(unsigned port, u08 on);
void SendBreak(unsigned port);

void do_term(unsigned port);
void init_term(void);
void set_term_header(unsigned port);
void echo_term(unsigned port, int i, int hex, int highlight);
void show_term_screen(unsigned port);
int exit_monitor(void);

#ifdef TEMP_CONTROL
   void apply_heat(void);
   void apply_cool(void);
   void hold_temp(void);
   void enable_temp_control(void);
   void disable_temp_control(void);
   void control_temp(void);
   unsigned init_lpt(void);
   void update_pwm(void);

   EXTERN u08 do_temp_control;        // if flag is set,  attempt to control unit temperature
   EXTERN u08 temp_control_on;
   EXTERN DATA_SIZE desired_temp;        // desired unit temperature
   EXTERN int lpt_addr;
   EXTERN u08 lpt_val;
#else
   #define update_pwm(void)
#endif

void calc_k_factors(void);
void clear_pid_display(void);
void set_default_pid(int num);
int calc_autotune(void);
void analyze_bang_bang(void);
void reset_osc_pid(int kbd_cmd);
void set_pps_mode(int mode);
void request_pps_mode(void);
void set_timeport_format(int format);
void config_rcvr_plots(void);
void config_rcvr_type(int set_baud);
void config_msg_ofs(void);
void config_normal_plots(void);
void config_lla_plots(int keep, int show);
int config_extra_plots(void);
void dump_stream(int flag);
void send_user_cmd(char *s);
void send_nmea_cmd(char *s);
void send_fan_cmd(char *s);
void set_venus_mode(int mode);
void set_venus_survey(int mode, double slat,double slon,double salt);
void set_raw_rate();
void set_rinex_site(char *s);
void set_rtk_mode(int mode);
void set_rtcm_mode(int enable);
void init_receiver(int why, int reset_ok);

void config_luxor_plots(void);
void set_luxor_sens(u08 lux1, u08 lux2);
void set_luxor_config(void);
void get_luxor_config(void);
void set_luxor_cal(void);
void get_luxor_cal(void);
void set_batt_pwm(u16 pwm);
void xset_driver_mode(u08 pulses, u08 double_click);
void set_driver_mode(void);
void reset_luxor_wdt(u08 flag);
void set_luxor_time(void);
void set_emissivity(DATA_SIZE em1, DATA_SIZE em2);
u08 luxor_fault(void);
int get_com_char(void);
void label_adev_grid(int top);

void set_ticc_config(unsigned port, char *s);
void get_ticc_config(unsigned port);
void edit_ticc_mode(void);
void edit_ticc_time2(void);
void edit_ticc_fudge(void);
int show_ticc_config(int row);
double tie_to_freq(int id, double val);
int tie_plot(int id);
u32 atohex(char *s);
double atosecs(char *s);

void free_fft(void);
void free_gif(void);
void free_adev_queues(void);
void free_plot(void);
void free_mtie(void);
void dump_mtie(int id, FILE *file);
void alloc_mtie(int why);

void show_all_adevs(void);
void add_mtie_point(int id, double val);
int alloc_mtie_data(int id, int mtie_size);
int fetch_mtie_info(int id, struct ADEV_INFO *bins);
void scan_mtie_bins(int id);
DATA_SIZE round_scale(DATA_SIZE val);

void calc_osc_k_factors(void);
void clear_osc_pid_display(void);
void set_default_osc_pid(int num);
void new_postfilter(void);
void enable_osc_control(void);
void disable_osc_control(void);
void osc_pid_filter(void);
void control_osc(void);

void lla_to_ecef(double lat, double lon, double alt);
double greatcircle(double lat1,double long1, double lat2,double long2);
double az_angle(double lat1,double long1, double lat2,double long2);
int get_norcvr_message(void);
int day_of_year(int month, int day);
void set_rinex_name(void);
void start_calc(int why);
void start_calc_zoom(int why);
void rpn_show(void);
void show_plot_stats(void);

