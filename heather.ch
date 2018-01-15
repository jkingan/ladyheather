#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>
#include <math.h>

#ifdef __GNUC__
   // gcc compiler...
   // If you are using gcc on a Windows box, you need to undefine PENGUIN
   // to use the proper file name format and maybe the SWITCH_CHAR
   // command line option switch flag
   #define GCC
   #define USE_X11
#endif
#ifdef __GNUG__
   // g++ compiler...
   // If you are using gcc on a Windows box, you need to undefine PENGUIN
   // to use the proper file name format and maybe the SWITCH_CHAR
   // command line option switch flag
   #define GCC
   #define USE_X11
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
   #define USE_X11
#endif


#ifdef _MSC_VER
  #define WINDOWS     // #define this for WIN32 environment
#endif

#define VERSION "5.00"

#define DEG_CODE    0x01       // char code for degrees symbol

#define PROGMEM
#define VCHAR_W  8   // elemental width and height of vector character patterns
#define VCHAR_H  8

#define GPS_EPOCH   2444244.5  // GPS time reference epoch (julian date)
#define LINUX_EPOCH 2440587.5  // Linux time reference epoch (julian date)
#define JD2000      2451545.0  // Jan 1, 2000 at noon
#define JD_MJD      2400000.5  // convert JD to MJD

#define ROLLOVER_YEAR 2016     // if GPS year is less than this,  assume it's a GPS week rollover fault
#define COM_TIMEOUT 5000.0     // com port loss detect timeout (in msecs)
EXTERN double com_timeout;
EXTERN int restart_count;

#define DEBUG_TEMP_CONTROL
#define DEBUG_OSC_CONTROL
#define DEBUG_PLOTS

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

#define HT_SIZE  (1<<13)  // 13 bit GIF hash table size

#ifdef AZEL_STUFF
   #define AZEL_OK 1
#else
   #define AZEL_OK 0
#endif

#define PI             (3.1415926535897932384626433832795)
#define RAD_TO_DEG     (180.0/PI)
#define DEG_TO_RAD     (PI/180.0)
#define SECS_PER_DAY   (24 * 60 * 60)
#define SQRT3          (1.732050808)
#define FEET_PER_METER 3.280839895

#define IABS(x)     (((x)<0)?(0-(x)):(x))
#define ABS(x)      (((x)<0.0F)?(0.0F-(x)):(x))
#define DABS(x)     (((x)<0.0)?(0.0-(x)):(x))
#define ROUND(x,to) ((((x)+(to)-1)/(to))*to)
#define DEBUGSTR(x)  vidstr(MOUSE_ROW-1,MOUSE_COL, RED, x)
#define DEBUGSTR2(x) vidstr(MOUSE_ROW-2,MOUSE_COL, RED, x)
#define DEBUGSTR3(x) vidstr(MOUSE_ROW-3,MOUSE_COL, RED, x)

#define DEFAULT_SLEEP 25   // default idle sleep time in msecs

#define OFS_SIZE float     // the type of the adev queue entries (float/double)

//#define PLUS_MINUS  0xF1
#define PLUS_MINUS  0x1F
#define SQUARED     0xFD

// to disable sun and moon displays #define SUN_PRN 0, MOON_PRN 0, SUN_MOON_PRN MAX_PRN
#define MAX_PRN 255  // was 32  // max number of sat PRNs to use
#define SUN_PRN  (MAX_PRN+1)
#define MOON_PRN (MAX_PRN+2)
#define SUN_MOON_PRN MOON_PRN   // max PRN to display: treat moon as a satellite
//#define SUN_MOON_PRN SUN_PRN  // max PRN to display: don't treat moon as a satellite
EXTERN int no_sun_moon;         // flags to control drawing sun/moon in maps and watch

#define GPS     0x0001   // bit mask positions match Venus receivers
#define GLONASS 0x0002
#define GALILEO 0x0004
#define BEIDOU  0x0008

#define IMES    0x2000
#define QZSS    0x4000
#define SBAS    0x8000
#define MIXED   (GPS | GLONASS | GALILEO | BEIDOU | SBAS | QZSS)
EXTERN int system_mask;  // used to filter messages for GNSS system type
EXTERN int gnss_mask;
EXTERN int have_gnss_mask;
EXTERN int default_gnss;

EXTERN char *unit_file_name;  // base name of files to write (based upon receiver type)

#define DOT_CHARS 0    // if 1, draw characters dot-by-dot using win_vfx font definitions





// #define PLM            // define this for power line monitoring

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
   #define SERIAL_DATA_AVAILABLE() (check_incoming_data())
   #define HUGE_MEM

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
   #define u16 unsigned short
   #define u32 unsigned int
   #define u48 unsigned long long
   #define s16 short
   #define s08 int
   #define LONG_LONG long long
   #define FP80 long double

   #define DEGREES     DEG_CODE  // special chars
   #define UP_ARROW    24
   #define DOWN_ARROW  25
   #define LEFT_ARROW  27
   #define RIGHT_ARROW 26
   #define CHIME_CHAR  13    // char to signal cuckoo clock on
   #define SONG_CHAR   14    // char to signal singing clock on
   #define ALARM_CHAR  15    // char to signal alarm or egg timer set
   #define DUMP_CHAR   19    // char to signal screen dump timer set

   #define TEXT_X_MARGIN (TEXT_WIDTH*2)
   #define TEXT_Y_MARGIN (TEXT_HEIGHT*1)

   EXTERN HINSTANCE hInstance;
   EXTERN HWND      hWnd;
   EXTERN char root[MAX_PATH+1];
   EXTERN char heather_path[MAX_PATH+1];
   EXTERN int root_len;
// #define BEEP(x) { if(beep_on)MessageBeep(0); if(debug_file) fprintf(debug_file, "BEEP(%d)\n", x); sprintf(debug_text, "Beep(%d)", x); }
   #define BEEP(x) if(beep_on)MessageBeep(0)

   extern PANE *stage;
   EXTERN u08 VFX_io_done;
   EXTERN U32 palette[16];

   EXTERN u08 downsized;  // flag set if screen has been windowed to show help dialog
   EXTERN u08 sal_ok;     // flag set once hardware has been setup

   unsigned char get_serial_char(void);
   void add_kbd(int key);
   int win_kbhit(void);
   int win_getch(void);
   void refresh_page(void);
   void do_fullscreen(void);
   void do_windowed(void);

   #define TCP_IP         // enable TCP/IP networking
   #define WIN_VFX        // use VFX for screen I/O

   #define SIN_TABLES     // use sin/cos tables for drawing circles
   #define SIG_LEVELS     // track sig levels vs azimuth
   #define BUFFER_LLA     // save lla x/y plot data in a buffer for screen redraws
#else // __linux__  __MACH__
   #include <stdint.h>
   #include <unistd.h>
   #include <X11/Xlib.h>
   #include <X11/Xutil.h>
   #include <X11/Xos.h>
   #include <X11/Xatom.h>
   #include <X11/keysymdef.h>
   #include <X11/cursorfont.h>
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

   #define USE_X11
   #define SIMPLE_HELP
   #define MAX_PATH 256+1

   #define KBHIT x11_kbhit
   #define GETCH x11_getch
   #define SERIAL_DATA_AVAILABLE() (check_incoming_data())
   #define BEEP(x) if(beep_on && display) XBell(display, 100);
   #define TCP_IP         // enable TCP/IP networking
   #define X11_sleep 100  // msecs to sleep when X11 is started / shutdown / resized

   #define COORD int
   #define u08 unsigned char
   #define u16 uint16_t
   #define u32 uint32_t
   #define u48 uint64_t
   #define s32 int32_t
   #define s16 int16_t
   #define s08 char
   #define S32 int32_t
// #define bool int
   #define DWORD int32_t
   #define LONG_LONG int64_t
   #define FP80 long double

   #define TRUE 1
   #define FALSE 0
   #define HUGE_MEM


   #define DEGREES     DEG_CODE  // special chars
   #define UP_ARROW    24
   #define DOWN_ARROW  25
   #define LEFT_ARROW  27
   #define RIGHT_ARROW 26
   #define CHIME_CHAR  13    // char to signal cuckoo clock on
   #define SONG_CHAR   14    // char to signal singing clock on
   #define ALARM_CHAR  15    // char to signal alarm or egg timer set
   #define DUMP_CHAR   19    // char to signal screen dump timer set

   #define TEXT_X_MARGIN (TEXT_WIDTH*2)
   #define TEXT_Y_MARGIN (TEXT_HEIGHT*1)

   #define SIN_TABLES     // use sin/cos tables for drawing circles
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

   EXTERN struct termios oldtio,newtio; // serial port config structures

   EXTERN u32 palette[16];   // our color palette

   unsigned char get_serial_char(void);  // forward ref'd functions
   void add_kbd(int key);
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

#ifdef TCP_IP
   EXTERN char   IP_addr[512];         // TCP/IP server address specified with /ip= parameter
   const S32 DEFAULT_RECV_BUF = 1024;
   const S32 DEFAULT_PORT_NUM = 45000;
#endif

#ifdef SIN_TABLES   // if memory is available use tables to speed up circle drawing
   #define sin360(x) sin_table[((int)(x))%360]
   #define cos360(x) cos_table[((int)(x))%360]
   EXTERN float sin_table[360+1];
   EXTERN float cos_table[360+1];
#else               // to save memory, DOS does not use tables to draw circles
   #define sin360(x) ((float)sin(((float)(((int)(x))%360))/(float)RAD_TO_DEG))
   #define cos360(x) ((float)cos(((float)(((int)(x))%360))/(float)RAD_TO_DEG))
#endif


#define SLEN  250              // standard string length

#ifdef GREET_STUFF
   EXTERN char *moons[31+2];
#endif

#ifdef SIG_LEVELS
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
   EXTERN int min_sig_db;
   EXTERN int sig_level_step;
#endif // SIG_LEVELS

#define MAX_LLA_SIZE 640 //500
#ifdef BUFFER_LLA
   EXTERN u08 lla_data[MAX_LLA_SIZE+1][MAX_LLA_SIZE+1];
#endif

// define the second to perform certain tasks at...
// ... so a bunch of stuff does not all happen at the same time
#define MOON_STUFF          48   // update moon info at 48 seconds
#define AZEL_UPDATE_SECOND  42   // update azel plot at 42 seconds
#define SCPI_STATUS_SECOND  33   // request SCPI receiver status at 33 seconds
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

#define DELTAT_SECOND       16   // update delta_r at 00:00:16
#define DELTAT_MINUTE       00
#define DELTAT_HOUR         00


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


#define PPS_ADEV_COLOR  BLUE      // graph colors
#define OSC_ADEV_COLOR  BROWN
#define OSC_COLOR       WHITE
#define PPS_COLOR       MAGENTA
#define DAC_COLOR       GREEN
#define TEMP_COLOR      YELLOW

#define ONE_COLOR       CYAN      // extra and DEBUG_PLOTS
#define TWO_COLOR       BROWN
#define THREE_COLOR     BLUE
#define FOUR_COLOR      BROWN
#define FIVE_COLOR      DIM_BLUE
#define SIX_COLOR       DIM_RED
#define SEVEN_COLOR     DIM_GREEN
#define EIGHT_COLOR     RED
#define NINE_COLOR      DIM_MAGENTA  // DIM_WHITE
#define TEN_COLOR       WHITE        // DIM_MAGENTA
#define ELEVEN_COLOR    DIM_RED
#define TWELVE_COLOR    DIM_GREEN
#define THIRTEEN_COLOR  DIM_BLUE
#define FOURTEEN_COLOR  DIM_WHITE    // DIM_MAGENTA

#define COUNT_COLOR     CYAN
#define CONST_COLOR     CYAN
#define SKIP_COLOR      RED       // timestamp error markers
#define HOLDOVER_COLOR  RED       // holdover/temp spike markers
#define MOUSE_COLOR     CYAN      // DOS mouse cursor
#define HELP_COLOR      WHITE     // help text
#define PROMPT_COLOR    CYAN      // string editing
#define STD_TIME_COLOR  BLUE      // time while in standard time
#define DST_TIME_COLOR  CYAN      // time while in daylight savings time
#define MARKER_COLOR    CYAN      // plot marker numbers
#define ALARM_COLOR     RED       // alarm clock color
#define TIMER_COLOR     YELLOW    // egg timer marker color
#define DOP_COLOR       WHITE     // dilution of precision info
#define OSC_PID_COLOR   CYAN      // external oscillator disciplining status
#define TITLE_COLOR     WHITE     // plot title color
#define LEVEL_COLOR     BLUE      // avg signal level vs azimuth

#define HIGHLIGHT_REF   1         // put > < ticks on plot center reference line


// Where to place the various information onto the screen.
// These are in character coordinates.
#define TIME_ROW 0          // time stuff
#define TIME_COL 0

#define VAL_ROW 0           // oscillator values stuff
#define VAL_COL 15

EXTERN int VER_ROW;         // version stuff
EXTERN int VER_COL;

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

#ifdef WINDOWS
  #define HELP_COL (PLOT_LEFT/TEXT_WIDTH)
  #define HELP_ROW (PLOT_TEXT_ROW+1)
#else // __linux__  __MACH__
  #define HELP_COL (PLOT_LEFT/TEXT_WIDTH)
  #define HELP_ROW (PLOT_TEXT_ROW+1)
#endif
EXTERN u08 showing_help;

#define ALL_ROW (TIME_ROW+7)// for showing all adevs

EXTERN int FILTER_ROW;      // where to put the filter mode info
EXTERN int FILTER_COL;

EXTERN int MOUSE_ROW;       // where to put the data at the mouse cursor
EXTERN int MOUSE_COL;

EXTERN int time_row;        // where the big digital time clock is
EXTERN int time_col;

EXTERN int all_adev_row;    // screen row to draw the adev tables at in all_adev mode
EXTERN int view_row;        // the text row number just above the view info


//
//  Screen / video mode stuff
//

EXTERN u08 ENDIAN;                 // 0=INTEL byte order,  1=other byte order
EXTERN int vmode;                  // BIOS video mode
EXTERN int text_mode;              // BIOS text mode (2,3, or 7)
EXTERN unsigned GRAPH_MODE;        // current video mode number
EXTERN unsigned user_video_mode;   // user specified video mode number
EXTERN S32 initial_window_mode;    // Windows version initial screen mode
EXTERN int go_fullscreen;          // if flag set, init screen in fullscreen mode
EXTERN u08 need_screen_init;       // flag set if user did a /V command from the keyboard
EXTERN int jpl_clock;              // JPL wall clock display
EXTERN int calendar_entries;       // how many entries are in the greetings structure

#define WIDE_SCREEN (SCREEN_WIDTH >= 1680)  // screen is wide enough for watch and (azel or lla)
#define SMALL_SCREEN ((SCREEN_WIDTH < 800) || (SCREEN_HEIGHT < 600))
#define ADEV_AZEL_THRESH 1280               // screen is wide enough to always show adevs and azel map

#define MEDIUM_WIDTH  (1000-6) // (1024-6)   // 1024
#define MEDIUM_HEIGHT  710     // 768
#define WIDE_WIDTH    (1280-6) // 1280
#define PLOT_TOP      480
#define MIN_WIDTH     640
#define MIN_HEIGHT    400

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
EXTERN u08 big_plot;         // flag set if plot area is large
EXTERN u08 screen_type;      // 's', 'm', 'l' = small, medium, large, etc.
EXTERN int invert_dump;      // swap black and white in screen dumps
EXTERN int invert_display;   // swap black and white on screen
EXTERN int top_line;         // selects screen dump or plot area only
EXTERN int need_redraw;      // info for screen config update
EXTERN int no_redraw;        // set flag for config_screen() to not do a redraw
EXTERN int need_resize;
EXTERN int new_width;        // requested new screen size (from mouse dragging)
EXTERN int new_height;
EXTERN int adevs_shown;      // flag set if adev tables are on screen

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
EXTERN float lux_scale;      // light sensor scale factor (lux/footcancles/candlepower)
EXTERN float lum_scale;      // light sensor lumen scale factor
EXTERN u08 show_lux;
EXTERN u08 show_fc;
EXTERN u08 show_cp;
EXTERN u08 show_lumens;
EXTERN u08 show_debug_info;

#define WWVB_LAT 40.677722   // lat/lon of WWVB antennas in Ft Collins
#define WWVB_LON -105.047153

#define TSIP_RCVR   0x0001
#define NMEA_RCVR   0x0002
#define UBX_RCVR    0x0004
#define UCCM_RCVR   0x0008   // mutant SCPI receiver
#define SCPI_RCVR   0x0010
#define MOTO_RCVR   0x0020
#define SIRF_RCVR   0x0040
#define VENUS_RCVR  0x0080
#define ZODIAC_RCVR 0x0100
#define GPSD_RCVR   0x0200
#define NO_RCVR     0x0400
#define NVS_RCVR    0x0800
#define STAR_RCVR   0x1000   // Oscilloquartz STAR 4
#define LUXOR_RCVR  0x2000   // luxor LED / power analyzer
#define ACRON_RCVR  0x4000   // Acron Zeit WWVB receiver

#define FAKE_ACRON_RCVR  0   // set to 1 to simulate ACRON_RCVR, 0 for real receiver

#define POLLED_RCVR ((rcvr_type == SCPI_RCVR) || (rcvr_type == STAR_RCVR) || (rcvr_type == UCCM_RCVR) || (rcvr_type == ACRON_RCVR))
#define TIMING_RCVR (res_t || (rcvr_type & (MOTO_RCVR | NVS_RCVR | SIRF_RCVR | UBX_RCVR | VENUS_RCVR | ZODIAC_RCVR)))
#define GPSDO       (((rcvr_type == TSIP_RCVR) && !res_t) || (rcvr_type == SCPI_RCVR) || (rcvr_type == STAR_RCVR) || saw_icm || saw_gpsdo)
#define find_msg_end() tsip_end(0)

#define HP_TYPE     '5'      // SCPI_RCVR sub-types
#define LUCENT_TYPE 'K'
#define NORTEL_TYPE 'N'
#define UCCM_TYPE   'U'
#define UCCMP_TYPE  'P'
#define SCPI_TYPE    0

#define GARMIN_NMEA 'G'      // NMEA receiver type
#define VENUS_NMEA  'V'

#define OSCILLO_TYPE  'O'    // STAR-4 receiver type - Oscilloquartz
#define NEC_TYPE      'N'    // NEC

#define STARLOC_RCVR 'S'     // TSIP_RCVR sub-types
#define LUXOR_DEVICE 'L'
#define TBOLT_RCVR    0

#define RES_T      1         // res_t sub-types
#define RES_T_SMT  2
#define RES_T_RES  3
#define RES_T_360  4
EXTERN int tbolt_e;          // flag set for newer TSIP models (like Thunderbolt-E)

#define MAX_ID_LINES 12
EXTERN char moto_id[MAX_ID_LINES][128+1];  // motorola receiver ID info
EXTERN int moto_id_lines;

EXTERN char nvs_id[32];      // NVS_RCVR id and serial number
EXTERN u32 nvs_sn;
EXTERN char nvs_id2[32];     // NVS_RCVR reserved id and serial number
EXTERN u32 nvs_sn2;
EXTERN char nvs_id3[32];     // NVS_RCVR reserved id and serial number
EXTERN u32 nvs_sn3;
EXTERN int nvs_chans;        // NVS_RCVR channel count

EXTERN int detect_rcvr_type; // if flag set, attempt to detect the receiver type
EXTERN int detecting;        // flag set while auto-detect is in progress
EXTERN int rcvr_type;        // GPS receiver message format
EXTERN int last_rcvr_type;   // the previous receiver type
EXTERN int tsip_type;        // flag set for DATUM GPSDO, etc
EXTERN int scpi_type;        // 0-Z3801A 5=HP5xxxx series 'U' = UCCM
EXTERN int nmea_type;        // NMEA receiver type
EXTERN int star_type;        // STAR-4 receiver type
EXTERN int dup_star_time;    // used to force keyboard check
EXTERN int saw_star_time;    // flag set if Star-4 timecode seen
EXTERN int have_uccm_loop;   // assume not a UCCM CDMA device without DIAG:LOOP command.
EXTERN int saw_uccm_dmode;   // flag set if "settling" seen in UCCM SYST:STAT? response
EXTERN int adjust_scpi_ss;   // if flag set, remap SCPI SS levels from 0..255 to 0..50ish range
EXTERN int force_mode_change;// if flag set, leave ZODIAC receiver in Motorola mode when reset
EXTERN int moto_chans;       // Motorola receiver channel count
EXTERN int rcvr_reset;       // flag set if hard-resetting Motorola reveiver
EXTERN int user_set_rcvr;
EXTERN int need_msg_init;
EXTERN int eom_flag;         // flag set when sending last byte of a message to the receiver
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
EXTERN int posn_mode;        // 0=3D, 1=posn hold,  2=2D, 3=timing auto survey


EXTERN float load_ovc;       // luxor protection thresholds
EXTERN float load_hvc;
EXTERN float load_lvc;
EXTERN float load_watts;
EXTERN float batt_ovc;
EXTERN float batt_hvc;
EXTERN float batt_lvc;
EXTERN float batt_watts;
EXTERN float auxv_hvc;
EXTERN float auxv_lvc;
EXTERN float tc1_ovt;
EXTERN float tc2_ovt;
EXTERN float msg_timeout;
EXTERN u08 prot_menu;
EXTERN u08 show_prots;
EXTERN u08 fault_seen;

EXTERN float vref_m,  vref_b;    // luxor calibration values
EXTERN float temp1_m, temp1_b;
EXTERN float temp2_m, temp2_b;
EXTERN float vcal_m,  vcal_b;
EXTERN float batti_m, batti_b;
EXTERN float ledi_m,  ledi_b;
EXTERN float lux1_m,  lux1_b;
EXTERN float lux2_m,  lux2_b;
EXTERN float adc2_m,  adc2_b;
EXTERN float rb_m,    rb_b;

EXTERN double osc_integral;  // integral of oscillator error

EXTERN u08 disable_kbd;             // set flag to disable keyboard commands
EXTERN u08 kbd_flag;
EXTERN int force_si_cmd;            // used by royal kludge to implement G C T keyboard command
EXTERN u08 esc_esc_exit;            // set flag to allow ESC ESC to exit program
EXTERN u08 com_error_exit;          // set flag to abort on com port error
EXTERN u08 com_error;
EXTERN u08 pause_data;              // set flag to pause updating queues
EXTERN u08 user_pause_data;         // flag set if user set something on the command line that needs to pause data input
EXTERN u08 no_eeprom_writes;        // set flag to disable writing EEPROM
EXTERN u08 no_easter_eggs;          // set flag to disable easter egg songs
EXTERN int ee_write_count;          // counts number of eeprom writes seen

EXTERN u08 mouse_disabled;          // if set, do not do mouse stuff
EXTERN u08 mouse_shown;             // if set,  mouse cursor is being shown
EXTERN S32 mouse_x, mouse_y;        // current mouse coordinates
EXTERN u08 mouse_plot_valid;        // flag set if mouse points to valid queue entry
EXTERN u08 last_mouse_plot_valid;   // used to minimize clearing of mouse data area
EXTERN int this_button;             // current mouse button state
EXTERN int last_button;             // previous mouse button state
EXTERN long plot_q_col0;            // queue entry of leftmost column in the plot
EXTERN long plot_q_last_col;        // queue entry of rightmost column in the plot
EXTERN long last_mouse_q;           // the queue entry the mouse was last over
EXTERN int last_mouse_x;
EXTERN int last_mouse_y;
EXTERN long last_q_place;           // where we were before we moved to a marker

#define MAX_MARKER 10
EXTERN long mark_q_entry[MAX_MARKER]; // the queue entry we list clicked on

// bits in the sat_flags byte
#define TEMP_SPIKE   0x1000
#define CONST_CHANGE 0x0800
#define TIME_SKIP    0x0400
#define UTC_TIME     0x0200
#define HOLDOVER     0x0100
#define SAT_COUNT    0x00FF  // this bit mask MUST be the low order bits
#define SAT_FLAGS    (TEMP_SPIKE | CONST_CHANGE | TIME_SKIP | UTC_TIME | HOLDOVER)

#define SENSOR_TC    10.0F          // tbolt firmware temperature sensor averaging time (in seconds)
EXTERN  u08 undo_fw_temp_filter;
EXTERN  u08 user_set_temp_filter;

#define OSC      0     // the various data that we can plot
#define PPS      1     // plots 0..3 are the standard plots.  they are always drawn
#define DAC      2
#define TEMP     3
#define ONE      4     // plots 4 and above are option/extra plots
#define TWO      5
#define THREE    6
#define FOUR     7
#define FIVE     8
#define SIX      9
#define SEVEN    10
#define EIGHT    11
#define NINE     12
#define TEN      13
#define FFT      14
#define ELEVEN   15    // plots 11 and above are from data derived from other plot data
#define TWELVE   16
#define THIRTEEN 17
#define FOURTEEN 18

#define MSGJIT   TEN   // timing message jitter
#define MSGOFS   NINE  // timing message offset from wall clock

#define BATTI    OSC
#define LUX1     PPS
#define BATTV    DAC
#define LUX2     ONE   // luxor plot id's
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
   #define NUM_PLOTS 15  // NUM_PLOTS does not include the DERVVED_PLOTS!
#else
   #define NUM_PLOTS 14
#endif

#define DERIVED_PLOTS 4       // extra plots where the data is derived from other recorded plot data
#define FIRST_EXTRA_PLOT ONE

struct PLOT_Q {    // the data we can plot
   u16 sat_flags;         // misc status and events
   double q_jd;           // time stamp (in UTC)
   float data[NUM_PLOTS]; // the data values we can plot
};

EXTERN struct PLOT_Q HUGE_MEM *plot_q;
EXTERN unsigned long HUGE_MEM *hash_table;

EXTERN long plot_q_size;     // number of entries in the plot queue
EXTERN long plot_q_in;       // where next data point goes into the plot queue
EXTERN long plot_q_out;      // next data point that comes out of the queue
EXTERN long plot_q_count;    // how many points are in the plot queue
EXTERN u08  plot_q_full;     // flag set if plot queue is full
EXTERN long plot_start;      // queue entry at first point in the plot window
EXTERN u08  user_set_plot_size;

EXTERN u08 interval_set;     // flag set if user did /i command to change plot queue interval
EXTERN long queue_interval;  // number of seconds between plot queue entries
                             // ... used to average each plot point over

EXTERN long view_interval;   // this can be used to set the interval that data will be
                             // extracted from the plot queue and displayed (to allow plots
                             // to be displayed at a different time scale that the queue interval)
EXTERN long user_view;       // the view time the user set on the command line
EXTERN u08 new_user_view;
EXTERN u08  view_all_data;   // set flag to view all data in the queue
EXTERN u08 slow_q_skips;     // if set, skip over sub-sampled queue entries using the slow method
EXTERN u08 set_view;
EXTERN u08 continuous_scroll;// if set, redraw plot on every incoming point

EXTERN int plot_column;             // the current pixel column we are plotting
EXTERN int plot_mag;                // magnifies plot this many times
EXTERN int plot_time;               // indicates when it is time to draw a new pixel
EXTERN int view_time;               // same, but used when view_interval is not 1

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
EXTERN u08 plot_stat_info;          // set flag to show statistics value of plots
EXTERN u08 plot_digital_clock;      // set flag to show digital clock display
EXTERN u08 show_msecs;              // set flag to show milliseconds in the digital clock
EXTERN u08 show_julian_time;        // set dlag to show digial clock time as julian date
EXTERN int plot_areas;              // the number of available places to draw maps and the analog watch
EXTERN u08 plot_watch;              // set flag to show watch face where the azel map goes
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
EXTERN u08 plot_background;         // if set, highlight WINDOWS plot background
EXTERN u08 erase_every_hour;        // set flag to redraw lla map every hour
EXTERN u08 user_set_bigtime;        // flag set if user specified the big clock option on the command line
EXTERN u08 user_set_signals;        // flag set if user specified the signal quality map on the command line
EXTERN u08 user_set_dops;           // flag set if user specified the dop display enable flag on the command line
EXTERN u08 user_set_filters;        // flag set if user specified the filter display enable flag on the command line
EXTERN u08 beep_on;                 // flag set to enable beeper
EXTERN u08 sound_on;                // flag set to enable sounds
EXTERN u08 blank_underscore;        // if set, a '_' on output gets blanked
EXTERN u08 show_live_fft;           // if set, display FFT on the incomming data
EXTERN u08 live_fft;                // the plot to show live data on
EXTERN u08 fft_id;                  // the currently selected signal to FFT
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
#define BOLIVIAN   17     // the new Blovian calendar

#define MAYAN_CORR 584283L          // default mayan correlation constant
EXTERN long mayan_correlation;
EXTERN long aztec_epoch;            // day offset from default epoch
EXTERN long chinese_epoch;          // year offset from default epoch
EXTERN long druid_epoch;            // day offset from default epoch
EXTERN long bolivian_epoch;         // day offset from default epoch

EXTERN double rollover;             // offset gps time/date from default epoch (in seconds)
EXTERN int user_set_rollover;

EXTERN long filter_count;           // number of entries to average for display filter
EXTERN u08 filter_log;              // if set, write filtered values to the log
                                    // when writing a log from queued data

EXTERN double pps_base_value;       // used to keep values in the queue reasonable for single precision numbers
EXTERN double osc_base_value;
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
EXTERN int have_dops;

EXTERN float pdop;                  // dilution of precision values
EXTERN float hdop;
EXTERN float vdop;
EXTERN float gdop;
EXTERN float tdop;
EXTERN float ndop;
EXTERN float edop;
EXTERN u08 plot_dops;               // set flag to show the dops

// vector character stuff (plot markers and big digital clock display)
EXTERN int VCHAR_SCALE;
#define VCharWidth     (VCHAR_SCALE?(VCHAR_SCALE*8):8)
#define VCharHeight    (VCHAR_SCALE?(VCHAR_SCALE*16):16)
#define VCharThickness (VCHAR_SCALE?VCHAR_SCALE/2:1)

#define VSTRING_LEN 128+1           // max vector character string length
EXTERN char date_string[VSTRING_LEN];
EXTERN int time_color;
EXTERN int last_time_color;

EXTERN double last_stamp;  // time stamp checking stuff
EXTERN u08 time_checked;
EXTERN u08 have_last_stamp;
EXTERN long idle_sleep;    // Sleep() this long when idle


#ifdef ADEV_STUFF
   #define OSC_ADEV 0    // adev_types (even=OSC,  odd=PPS)
   #define PPS_ADEV 1
   #define OSC_HDEV 2
   #define PPS_HDEV 3
   #define OSC_MDEV 4
   #define PPS_MDEV 5
   #define OSC_TDEV 6
   #define PPS_TDEV 7

   EXTERN int ATYPE;              // the adev type to display

   #define ADEVS 30               // max number of adev bins to process
   EXTERN int max_adev_rows;      // actual number of bins we use (based upon adev_q_size)

   #define ADEV_DISPLAY_RATE  10  // update adev plots at this rate

   struct ADEV_INFO {
      u08   adev_type;            // pps or osc and
      float adev_taus[ADEVS];
      float adev_bins[ADEVS];
      long  adev_on[ADEVS];
      int   bin_count;
      float adev_min;
      float adev_max;
   };
   EXTERN struct ADEV_INFO pps_bins;
   EXTERN struct ADEV_INFO osc_bins;

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

   EXTERN double adev_q_overflow;   // counts how many entries have dropped out of the queue

   EXTERN struct BIN pps_adev_bins[ADEVS+1];  // incremental adev info bins
   EXTERN struct BIN osc_adev_bins[ADEVS+1];
   EXTERN struct BIN pps_hdev_bins[ADEVS+1];
   EXTERN struct BIN osc_hdev_bins[ADEVS+1];
   EXTERN struct BIN pps_mdev_bins[ADEVS+1];
   EXTERN struct BIN osc_mdev_bins[ADEVS+1];

   EXTERN double global_adev_max;   // max and min values found in all the adev bins
   EXTERN double global_adev_min;

   struct ADEV_Q {   // adev queue data values
      OFS_SIZE osc;
      OFS_SIZE pps;
   };

   EXTERN struct ADEV_Q HUGE_MEM *adev_q;  // queue of points to calc adevs over
   EXTERN long adev_q_in;
   EXTERN long adev_q_out;
   EXTERN long adev_q_count;

   #define get_adev_q(i)    adev_q[i]
   #define put_adev_q(i, q) adev_q[i] = q

   EXTERN u08 osc_adev_time;       // says when to show new osc adev tables
   EXTERN u08 pps_adev_time;       // says when to show new pps adev tables

   EXTERN int bin_scale;           // adev bin sequence (default is 1-2-5)
   EXTERN int last_bin_count;      // how many adev bins were filled

   EXTERN int adev_time;           // says when to take an adev data sample
   EXTERN int adev_mouse_time;     // used to keep the DOS mouse lively during long adev calculations

   EXTERN int min_points_per_bin;  // number of points needed before we display the bin
   EXTERN int n_bins;              // max number of adev bins we will calculate


   void update_adev_display(int type);
   void show_adev_info(void);
   void add_adev_point(double osc, double pps);

   void incr_adev(u08 id, struct BIN *bins);
   void incr_hdev(u08 id, struct BIN *bins);
   void incr_mdev(u08 id, struct BIN *bins);

   int fetch_adev_info(u08 dev_id, struct ADEV_INFO *bins);
   void reset_incr_bins(struct BIN *bins);
#endif   // ADEV_STUFF

EXTERN int jitter_adev;         // if flag set calculate PPS adevs from message timing jitter
EXTERN int measure_jitter;      // flag set if /jm jitter measurement mode enabled
EXTERN int old_idle;            // used to restore settings changed by entering jitter measurement mode
EXTERN int old_poll;
EXTERN int old_jit_adev;

EXTERN long adev_q_size;         // number of entries in the adev queue
EXTERN u08 user_set_adev_size;

EXTERN float adev_period;        // adev data sample period in seconds
EXTERN u08 keep_adevs_fresh;     // if flag is set,  reset the adev bins
                                 // once the adev queue has overflowed twice

EXTERN u08 mixed_adevs;          // if flag set display normal plots along with all adev types
EXTERN u08 aa_val;               // all_adev type to show
EXTERN u08 all_adevs;            //    0=normal display mode
                                 //    1=display all types of OSC adevs
                                 //    2=display all types of PPS adevs



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
#define F12_CHAR    0x0186

#define ESC_CHAR    0x1B

EXTERN u08  review_mode;       // flag set if scrolling through old data
EXTERN u08  review_home;       // flag set if we are at beginning of data
EXTERN long review;            // where we are looking in the queue
EXTERN long right_time;        // how long the right mouse button has been held
#define RIGHT_DELAY  10        // how many mouse checks before we start right-click mouse scrolling

EXTERN char read_log[256];     // name of log file to preload data from
EXTERN char log_name[256];     // name of raw receiver data dump file to write
EXTERN char raw_name[256];     // name of log file to write
EXTERN char sim_name[256];     // name of simulation file to read
EXTERN char log_text[256];     // text string for log file comments
EXTERN char debug_name[256];   // text string for debug output file
EXTERN char *log_mode;         // file write mode / append mode
EXTERN FILE *log_file;         // log file I/O handle
EXTERN FILE *raw_file;         // log file of all receiver data
EXTERN FILE *debug_file;       // debug message file
EXTERN FILE *sim_file;         // simulated receiver input file
EXTERN FILE *lla_file;         // lat/lon/alt log file
EXTERN FILE *hist_file;        // message end arrival time histogram data
EXTERN FILE *temp_script;      // temporary keyboard script file written from '@' commands in heather.ch
EXTERN u08 need_raw_file;      // if flag set, open raw receiver file
EXTERN u08 need_debug_log;     // if flag set, open debug log file
EXTERN u08 log_written;        // flag set if we wrote to the log file
EXTERN u08 log_header;         // write log file timestamp headers
EXTERN u08 old_log_format;     // read files written in the old log format
EXTERN u08 log_loaded;         // flag set if a log file has been read
EXTERN u08 log_errors;         // if set, log data errors
EXTERN u08 user_set_log;       // flag set if user set any of the log parametrs on the command line
EXTERN long log_interval;      // seconds between log file entries
EXTERN long log_file_time;     // used to determine when to write a log entry
EXTERN u08 adev_log;           // flag set if file read in was adev intervals
EXTERN u08 dump_type;          // used to control what to write
EXTERN u08 log_stream;         // if set, write the incomming serial data to the log file
EXTERN u08 log_comments;       // set flag to write comments in the log file
EXTERN u08 log_db;             // set flag to log sat signal levels
EXTERN u08 reading_log;        // flag set while reading log file

#define KML     30
#define GPX     20
#define XML     10
#define HEATHER 0
EXTERN int log_fmt;            // log file format
EXTERN int gpx_track_number;

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


EXTERN u08 first_sample;   // flag cleared after first data point has been received
EXTERN u08 have_time;      // flag set when a valid time has been received
EXTERN int have_year;
EXTERN int have_osc;
EXTERN int req_num;        // used to sequentially request various minor GPS messages
EXTERN int status_prn;     // used to sequentially request satellite status
EXTERN u08 amu_mode;       // flag set if signal level is in AMU units
EXTERN char *level_type;   // string representing the signal level units

EXTERN int track_count;    // number of sats being used (tracked)
EXTERN int vis_count;      // number of visible sats
EXTERN int sat_count;      // number of sats being used (visible)
                           // note that show_satinfo() converts this to tracked sat count

EXTERN int tracked_only;   // if flag set, only show tracked sat info
EXTERN int max_sats;       // max number of sats we can display
EXTERN int last_sat_row;   // the last text row of the sat info display
EXTERN int temp_sats;      // temporary number of sats we can display
EXTERN int ebolt;          // flag set if ThunderBolt-E message seen
EXTERN int last_ebolt;     // used to detect changes in ebolt setting
EXTERN u08 res_t;          // flag set if Resolution-T message seen
EXTERN u08 user_set_res_t; // user Forced resolution-T type
EXTERN u08 saw_icm;        // set if RES360 ICM seen


#define NO_PAR    0
#define ODD_PAR   1
#define EVEN_PAR  2
#define LUXOR_PAR ODD_PAR
EXTERN int baud_rate;      // serial port baud rate
EXTERN int parity;         // 0=none 1=odd 2=even
EXTERN int stop_bits;
EXTERN int data_bits;
EXTERN int user_set_baud;

#define NO_SCPI_BREAK 0    // if set, dont sent BREAKs to SCPI receivers (used to avoid putting them into TSIP mode)
EXTERN int enable_terminal; // if flag set, enable serial port terminal mode
EXTERN u08 mini_t;         // flag set if Mini-T message seen
EXTERN u08 nortel;         // flag set to wake up a Nortel NTGxxxx unit
EXTERN u08 saw_nortel;     // flag set if nortel format osc params worked
EXTERN u08 try_ntpx;       // flag set if detecting NTPX
EXTERN u08 saw_ntpx;       // flag set if Nortel NTPX receiver is detected
EXTERN u08 saw_mini;       // flag set if Mini-T receiver is detected
EXTERN u08 luxor;          // Luxor battery/LED analyzer message
EXTERN u08 config_set;

EXTERN int eofs;           // used to tweak things around on the screen if ebolt seen

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
EXTERN u16 tsip_error;     // flag set if error detected in the message
                           //   0x01 bit: msg start seen in middle of message
                           //   0x02 bit: msg end seen in middle of message
                           //   0x04 bit: msg end not seen where expected
                           //   0x08 bit: msg error in tsip byte
                           //   0x10 bit: msg error in tsip word
                           //   0x20 bit: msg error in tsip dword
                           //   0x40 bit: msg error in tsip float
                           //   0x80 bit: msg error in tsip double


EXTERN u32 packet_count;   // how many TSIP packets we have read
EXTERN u32 bad_packets;    // how many packets had known errors
EXTERN u32 math_errors;    // counts known math errors
EXTERN u08 rcv_error;      // com port receive error detected

#define MAX_SAT_DISPLAY 14 // max number of sats to display info for
EXTERN int max_sat_display;

struct SAT_INFO {  // the accumulated wisdom of the ages
   u08 health_flag;    // packet 49

   u08 disabled;       // packet 59
   u08 forced_healthy;


   float sample_len;   // packet 5A
   float sig_level;    // (also packet 47, 5C)
   u08 level_msg;      // the message type that set sig_level
   double code_phase;
   double doppler;
   double range;
   double raw_time;

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
   u08 age;
   u08 msec;
   u08 bad_flag;
   u08 collecting;
   u08 how_used;      // (packet 0x5D)
   u08 sv_type;       // (packet 0x5D)

   int tracking;       // 6D
   int visible;

   float sat_bias;     // 8F.A7
   float time_of_fix;
   u08 last_bias_msg;  // flag set if sat info was from last message
};
EXTERN int user_set_short; // if flag set, only display sat az/el/snr info
EXTERN struct SAT_INFO sat[MAX_PRN+2+1];  // +2 is for sun/moon
EXTERN int max_sat_count;
EXTERN int sat_cols;       // display sat info in two columns
EXTERN int sat_rows;       // max number of rows to show sat info in

#define SORT_ELEV    'E'
#define SORT_AZ      'A'
#define SORT_SIGS    'S'
#define SORT_PRN     'P'
#define SORT_DOPPLER 'D'
EXTERN int sort_by;               // used to sort the sat info display
EXTERN int sort_ascend;

#define UN_ZOOM     0
#define ZOOM_SHARED 1
#define ZOOM_LLA    1
#define ZOOM_AZEL   1
#define ZOOM_INFO   2
#define ZOOM_CLOCK  3
#define ZOOM_ADEVS  4
#define ZOOM_PLOT   5
EXTERN u08 zoom_screen;           // full screen is taken over by a special display mode
EXTERN int zoom_fixes;
EXTERN int un_zoom;               // used to restore zoomed screen to its previous state


EXTERN double pps_offset;         // latest values from the receiver
EXTERN double osc_offset;
EXTERN float dac_voltage;
EXTERN float uccm_voltage;        // manual DAC control setting for UCCM
EXTERN float temperature;
EXTERN float pps_quant;
EXTERN int tfom;                  // SCPI time figure-of-merit
EXTERN int have_tfom;
EXTERN int ffom;                  // SCPI freq figure-of-merit
EXTERN int have_ffom;
EXTERN char uccm_led_msg[32];     // response from LED:GPSL? query

EXTERN int gpsdo_ref;             // GPSDO reference source (0=GPS, 1=AUX PPS)
EXTERN int have_gpsdo_ref;

EXTERN int round_temp;            // controls temperature display
EXTERN u08 user_set_rounding;

#define LOW_LUX 25                // lux sensor integration times
#define MED_LUX 69
#define HI_LUX  191
#define MIN_LUX 2
#define MAX_LUX 191


EXTERN float batt_v;              // luxor sensor readings
EXTERN float batt_i;
EXTERN float led_pos;
EXTERN float led_neg;
EXTERN float led_v;
EXTERN float led_i;
EXTERN float adc2;
EXTERN float tc1, tc2;
EXTERN float emis1, emis2;
EXTERN float pwm_hz;
EXTERN float lux1, lux2;
EXTERN float alt_lux1;
EXTERN u08 lux1_time, lux2_time;
EXTERN u16 batt_pwm;
EXTERN u08 batt_pwm_res;
EXTERN float PWM_STEP;
EXTERN u32 unit_status;
EXTERN int luxor_hw_ver, luxor_hw_rev;
EXTERN int luxor_sn_prefix, luxor_serial_num;

EXTERN float batt_w;
EXTERN float led_w;
EXTERN u08 cal_mode;

#define IR_TIME 10             // 10 second internal resistance measurement cycle
EXTERN float ir_v, ir_i;
EXTERN int calc_ir;

EXTERN u08 show_mah;           // battery capacity info
EXTERN float batt_mah;
EXTERN float batt_mwh;
EXTERN float load_mah;
EXTERN float load_mwh;

#define CC_VOLTS     1
#define CC_AMPS      2
#define CC_LIPO      3
#define CC_WATTS     4
#define PWM_SWEEP    5
EXTERN u08 cc_mode;           // constant current/voltage/power and lipo charge mode stuff
EXTERN u08 cc_state;
EXTERN float lipo_volts;      // desired battery charge voltage
EXTERN float unsafe_v;        // don't charge batts below this voltage
EXTERN float cc_val;          // desired load target value
EXTERN float cc_pwm;          // MOSFET pwm time (0.0 .. 1.0)
EXTERN float sweep_start;     // PWM sweep limit
EXTERN float sweep_end;       // PWM sweep limit
EXTERN float sweep_val;       // PWM sweep current value
EXTERN float sweep_stop;      // sweep from 0.0 to this value (amps/volta/watts)
EXTERN int sweep_rate;        // sweep speed (seconds per step)
EXTERN int sweep_tick;
EXTERN int update_stop;       // if set, stop screen updates after a sweep

EXTERN float BLUE_SENS;       // converts Hz to uW/cm^2
EXTERN float GREEN_SENS;
EXTERN float RED_SENS;
EXTERN float WHITE_SENS;

EXTERN float blue_hz, green_hz, red_hz, white_hz;
EXTERN float blue_uw, green_uw, red_uw, white_uw;
EXTERN float r_over_b;
EXTERN float cct;
EXTERN float cct_cal, cct1_cal, cct2_cal;
EXTERN u08 cct_type;
EXTERN u08 tcs_color;
EXTERN u08 show_color_hz;
EXTERN u08 show_color_uw;
EXTERN u08 show_color_pct;
EXTERN u08 cct_dbg;
EXTERN u08 cri_flag;


EXTERN double last_pps_offset;    // previous values from the receiver
EXTERN double last_osc_offset;
EXTERN float last_dac_voltage;
EXTERN float last_temperature;

EXTERN float last_temp_val;       // used in temp control code
EXTERN u08 temp_dir;              // direction we are moving the temperature

EXTERN float stat_count;          // how many data points we have summed
                                  // up for the statisticsvalues

EXTERN float spike_threshold;     // used to filter out temperature sensor spikes
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

EXTERN double ANGLE_SCALE;        // earth size in degrees/ft or meter
EXTERN char *angle_units;         // "ft" or "m "

EXTERN double lat;                // receiver position
EXTERN double lon;
EXTERN double alt;
EXTERN double cos_factor;         // cosine of reference latitude

EXTERN double speed;              // meters per second
EXTERN double heading;            // degrees from north (true)

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
EXTERN float nvs_filter;

EXTERN u08 user_pv;               // user requested dynamics filter settings
EXTERN u08 user_static;
EXTERN u08 user_alt;
EXTERN u08 user_kalman;
EXTERN u08 user_marine;

EXTERN float el_mask;             // minimum acceptable satellite elevation
EXTERN float amu_mask;            // minimum acceptable signal level
EXTERN float pdop_mask;
EXTERN float pdop_switch;
EXTERN u08 foliage_mode;          // 0=never  1=sometime  2=always
EXTERN u08 dynamics_code;         // 1=land  2=sea  3=air  4=stationary


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

EXTERN u08 rcvr_mode;             // receiver operating mode
EXTERN u08 last_rmode;
EXTERN u08 configed_mode;
EXTERN u08 venus_hold_mode;
EXTERN int eeprom_save;           // enable config saves to eeprom: 0=SRAM  1=SRAM+EEPROM
EXTERN float nav_rate;            // receiver navigation fix rate (in Hz)
EXTERN float user_nav_rate;       // forced startup navigation fix rate (in Hz)
EXTERN float saved_nav_rate;      // nav rate in effect before log file read in
EXTERN int restore_nav_rate;      // if flag set, restore old nav rate value when pause released
EXTERN int user_set_nav_rate;

#define set_rcvr_mode(x)     set_config((x),  0xFF, -1.0F, -1.0F, -1.0F, -1.0F, 0xFF)
#define set_rcvr_dynamics(x) set_config(0xFF, (x),  -1.0F, -1.0F, -1.0F, -1.0F, 0xFF)
#define set_el_mask(x)       set_config(0xFF, 0xFF, ((x)/(float)RAD_TO_DEG), -1.0F, -1.0F, -1.0F, 0xFF)
#define set_amu_mask(x)      set_config(0xFF, 0xFF, -1.0F, (x),   -1.0F, -1.0F, 0xFF)
#define set_el_amu(el,amu)   set_config(0xFF, 0xFF, ((el)/(float)RAD_TO_DEG), (amu), -1.0F, -1.0F, 0xFF)
#define set_amu_mask(x)      set_config(0xFF, 0xFF, -1.0F, (x),   -1.0F, -1.0F, 0xFF)
#define set_pdop_mask(x)     set_config(0xFF, 0xFF, -1.0F, -1.0F, (x), ((x)*0.75F), 0xFF)
#define set_foliage_mode(x)  set_config(0xFF, 0xFF, -1.0F, -1.0F, -1.0F, -1.0F, (x))
EXTERN u08 single_sat;
EXTERN int single_sat_prn;   // prn if single sat tracking forced
EXTERN u32 sats_enabled;     // bitmap of sats that are enabled

EXTERN u08 osc_params;            // flag set if we are monkeying with the osc parameters
EXTERN float time_constant;       // oscillator control values
EXTERN float real_time_constant;  // for STAR_RCVR
EXTERN float damping_factor;
EXTERN float osc_gain;
EXTERN float log_osc_gain;
EXTERN int gain_color;
EXTERN float min_volts, max_volts;
EXTERN float min_dac_v, max_dac_v;
EXTERN float jam_sync;
EXTERN float max_freq_offset;
EXTERN float initial_voltage;
EXTERN float osc_pid_initial_voltage;
EXTERN int pullin_range;    // UCCM device pullin range
EXTERN int have_pullin;

EXTERN float user_time_constant;  // oscillator control values
EXTERN float user_damping_factor;
EXTERN float user_osc_gain;
EXTERN float user_min_volts, user_max_volts;
EXTERN float user_min_range, user_max_range;
EXTERN float user_jam_sync;
EXTERN float user_max_freq_offset;
EXTERN float user_initial_voltage;
EXTERN int user_pullin;

EXTERN u16 critical_alarms;       // receiver alarms
EXTERN u16 minor_alarms;
EXTERN u08 have_alarms;
EXTERN u16 last_critical;
EXTERN u16 last_minor;
EXTERN u16 leap_pending;
EXTERN u08 leaped;                // flag set when leapsecond pending flag clears
EXTERN int leap_days;             // number of days until leap second
EXTERN int have_leap_days;
EXTERN char guessed_leap_days;
EXTERN int have_moto_Gj;          // flag set if @@Gj message seen
EXTERN int have_scpi_hex_leap;    // flag set if GPST leapdate message seen (on Z3812's is usually correct)
EXTERN int leap_sixty;            // flag set if receiver seconds == 60
                                  // ... used to restore seconds to xx:xx:60 after
                                  // ... converting julian back to gregorian

EXTERN int have_antenna;          // flags that indicate what status into we have
EXTERN int have_osc_age;
EXTERN int have_osc_offset;
EXTERN int have_pps_offset;
EXTERN int have_saved_posn;
EXTERN int have_tracking;
EXTERN int have_sat_azel;
EXTERN int have_leap_info;
EXTERN int have_op_mode;
EXTERN int have_almanac;
EXTERN int have_critical_alarms;
EXTERN int have_eeprom;
EXTERN int have_gps_status;
EXTERN int have_cable_delay;
EXTERN int have_pps_delay;
EXTERN int have_rf_delay;
EXTERN int have_temperature;
EXTERN int have_lifetime;
EXTERN int have_nav_rate;
EXTERN int have_pps_rate;
EXTERN int have_pps_enable;
EXTERN int have_pps_polarity;
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
EXTERN int have_doppler;
EXTERN int have_phase;
EXTERN int have_range;
EXTERN int have_bias;
EXTERN int have_accu;
EXTERN int have_sawtooth;
EXTERN int have_traim;
EXTERN int have_timing_mode;
EXTERN int have_star_perf;
EXTERN int have_star_atdc;
EXTERN int have_star_input;
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
EXTERN int first_request;
EXTERN u08 saw_kalman_on;    // flag set if the dynamics Kalman filter is ever on
EXTERN int saw_timing_msg;   // flag set if Ublox timing receiver messages seen
EXTERN int saw_ubx_tp5;      // flag set if Ublox TP5 message seen
EXTERN int saw_ubx_tp;       // flag set if Ublox TP message seen
EXTERN int saw_diff_change;  // flag set if movement seen in UCCM freq_diff param
EXTERN int scpi_test;        // results from SCPI receiver self test
EXTERN int got_timing_msg;   // flag incremented whenever a timing message is processed

EXTERN u08 leap_time;             // flag set if seconds is 60
EXTERN u08 leap_dump;             // do screen dump at leap-second time

EXTERN u08 holdover_seen;         // receiver oscillator holdover control
EXTERN u08 user_holdover;
EXTERN u32 holdover;

EXTERN u08 osc_discipline;        // oscillator disciplining control
EXTERN u08 discipline;
EXTERN u08 last_discipline;
EXTERN u08 discipline_mode;
EXTERN u08 last_dmode;


EXTERN int gps_status;            // receiver signal status
EXTERN int last_status;

EXTERN u08 time_flags;            // receiver time status
EXTERN u08 last_time_flags;

EXTERN int set_utc_mode;          // gps or utc time
EXTERN int set_gps_mode;
EXTERN int user_set_time_mode;
EXTERN u08 temp_utc_mode;         // used when setting system time from the tbolt
EXTERN u08 timing_mode;           // PPS referenced to GPS or UTC time
EXTERN int use_tsc;               // if flag set, use the processor TSC instruction got GetMsecs()

EXTERN s08 seconds;               // receiver time info (in utc)
EXTERN s08 minutes;
EXTERN s08 hours;
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

EXTERN double jd_utc;             // current Julian date in UTC.
EXTERN double jd_gps;             // current Julian date in GPS time.
EXTERN double jd_tt;              // current Julian date in terrestrial time.
EXTERN double jd_local;           // current Julian date in local time.
EXTERN double jd_display;         // currently displayed Julian date
EXTERN double jd_astro;
EXTERN double jd_leap;            // Julian date of next leapsecond (like 31 Dec 23:59:59)
EXTERN int have_jd_leap;

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
EXTERN int fake_time_stamp;  // flag set if processing faked UCCM time stamp


EXTERN s08 last_hours;
EXTERN s08 last_log_hours;
EXTERN s08 last_second;
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
EXTERN long survey_tow;
EXTERN u08 csv_char;         // log file value separator
EXTERN double last_msec, this_msec; // millisecond tick clock values

EXTERN u32 pri_tow;
EXTERN int force_day;
EXTERN int force_month;
EXTERN int force_year;
EXTERN u08 fraction_time;
EXTERN u08 seconds_time;
EXTERN int last_day;
EXTERN int last_month;

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
#define MISC_INFO      0x08
#define INFO_LOGGED    0x80
EXTERN u08 have_info;

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


EXTERN u08 set_osc_polarity;     // command line settable options
EXTERN u08 user_osc_polarity;
EXTERN u08 osc_polarity;

EXTERN u08 set_pps_polarity;
EXTERN u08 user_pps_polarity;
EXTERN int pps_polarity;         // 1=falling  0=rising

EXTERN u08 pps_enabled;
EXTERN u08 user_pps_enable;
EXTERN u08 pps_mode;  // 0=off, 1=always on, 2=when tracking, 3=when traim OK

EXTERN u08 pps_rate;
EXTERN u08 user_pps_rate;
EXTERN double pps1_freq;   // programmable PPS output freq and duty cycle
EXTERN double pps1_duty;
EXTERN u32 pps1_flags;
EXTERN double pps2_freq;
EXTERN double pps2_duty;
EXTERN u32 pps2_flags;
EXTERN int have_pps_freq;
EXTERN int have_pps_duty;
#define NVS_PPS_WIDTH 1000000
EXTERN u32 nvs_pps_width;       // in nanoseconds
EXTERN u32 last_nvs_pps_width;  // in nanoseconds

EXTERN double delay_value;
EXTERN double cable_delay; // in seconds
EXTERN double rf_delay;    // in seconds
EXTERN double pps1_delay;  // in nanoseconds
EXTERN double pps2_delay;  // in nanoseconds
EXTERN u08 user_set_delay;

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
EXTERN int user_set_osc;   // flags which osc params were on the command line
EXTERN int have_osc_params;// flags which params we have from the oscillator

EXTERN float cmd_tc;       // the command line osc param values
EXTERN float cmd_damp;
EXTERN float cmd_gain;
EXTERN int cmd_pullin;

EXTERN float cmd_initdac;
EXTERN float cmd_minv;
EXTERN float cmd_maxv;
EXTERN float cmd_minrange;
EXTERN float cmd_maxrange;

EXTERN float cmd_jamsync;
EXTERN float cmd_maxfreq;

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

EXTERN u08 keyboard_cmd;  // flag set of command line option is being set from the keyboard
EXTERN u08 not_safe;      // flag set if user attempts to set a command line option
                          // ... from the keyboard that is not ok change.
                          // ... 1 = option not processed immediately
                          // ... 2 = option cannot be changed safely


#define SURVEY_SIZE  2000L         // default self-survey size
#define SURVEY_BIN_COUNT 96        // number of precise survey bins
EXTERN int survey_length;          // reported survey size
EXTERN u08 survey_save;            // reported survey save flag
EXTERN long do_survey;             // user requested survey size
EXTERN int survey_why;
EXTERN int survey_progress;        // completion percentage of the survey
EXTERN u08 precision_survey;       // do 48 hour precision survey
EXTERN u08 user_precision_survey;  // flag set if user requested precsion survey from command line
EXTERN u08 survey_done;            // flag set after survey completes - used to keep adev tables off screen
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

EXTERN int alarm_time;             // daily alarm time set
EXTERN int alarm_hh, alarm_mm, alarm_ss;
EXTERN int alarm_date;             // alarm date set
EXTERN int alarm_month, alarm_day, alarm_year;
EXTERN u08 sound_alarm;            // flag set to sound the alarm
EXTERN u08 single_alarm;           // set flag to play the alarm file only once

EXTERN u08 cuckoo;                 // cuckoo clock mode - #times per hour
EXTERN u08 singing_clock;          // set flag to sing songs instead of cuckoo
EXTERN u08 cuckoo_hours;           // set flag to cuckoo the hour on the hour
EXTERN u08 cuckoo_beeps;           // cuckoo clock signal counter
EXTERN u08 ticker;                 // used to flash alarm indicators

EXTERN u08 ships_clock;            // set flag to ring ships bells instead of cuckoo
EXTERN int ring_bell;
EXTERN int bell_number;

EXTERN long egg_timer;             // egg timer mode
EXTERN long egg_val;               // initial egg timer value
EXTERN u08 repeat_egg;             // repeat the egg timer
EXTERN u08 single_egg;
EXTERN u08 alarm_wait;             // if set, wait for timer to expire before continuing script

EXTERN int dump_time;              // daily screen dump time set
EXTERN int dump_hh, dump_mm, dump_ss;
EXTERN int dump_date;              // screen dump date set
EXTERN int dump_month, dump_day, dump_year;
EXTERN u08 dump_alarm;
EXTERN long dump_timer;            // egg timer mode
EXTERN long dump_val;              // initial egg timer value
EXTERN long dump_number;           // automatic screen dump counter
EXTERN u08 repeat_dump;            // repeat the screen dump
EXTERN u08 single_dump;
EXTERN u08 do_dump;                // flag set to do the screen dump
EXTERN u08 dump_xml;               // do scheduled log dumps in XML format
EXTERN u08 dump_gpx;               // do scheduled log dumps in GPX format

EXTERN int log_time;               // daily log dump time set
EXTERN int log_hh, log_mm, log_ss;
EXTERN int log_date;               // log dump date set
EXTERN int log_month, log_day, log_year;
EXTERN u08 log_alarm;
EXTERN long log_timer;             // egg timer mode
EXTERN long log_val;               // initial egg timer value
EXTERN long log_number;            // automatic screen dump counter
EXTERN u08 repeat_log;             // repeat the screen dump
EXTERN u08 single_log;
EXTERN u08 do_log;                 // flag set to do the screen dump
EXTERN u08 log_wrap;               // write log on queue wrap

EXTERN int end_time;               // daily exit time set
EXTERN int end_hh, end_mm, end_ss;
EXTERN int end_date;               // exit date set
EXTERN int end_month, end_day, end_year;
EXTERN long exit_timer;            // exit countdown timer mode
EXTERN long exit_val;              // initial exit countdown timer value
EXTERN u08 repeat_exit;
EXTERN u08 single_exit;


EXTERN u08 show_euro_ppt;          // if flag set,  show exponents on the osc values (default is ppb/ppt)
EXTERN u08 show_euro_dates;        // if flag set,  format mm/dd/yy dates in euro format
EXTERN u08 digital_clock_shown;
EXTERN u08 enable_timer;           // enable windows message timer


#define LEAP_FILE         "heather_leapsec.wav"   // played after leapsecond flag clears
#define SUNRISE_FILE      "heather_sunrise.wav"   // played at sun (or moon) rise/set
#define NOON_FILE         "heather_noon.wav"      // played at solar noon
#define CHIME_FILE        "heather_chime.wav"     // chime (cuckoo) clock sound
#define BELL_FILE         "heather_bell.wav"      // ships bells audible clock sound
#define SONG_NAME         "heather_song%02d.wav"  // singing clock files
#define ALARM_FILE        "heather_alarm.wav"     // played when alarm/timer triggers
#define USER_NOTIFY_FILE  "heather_notify.wav"    // alternate chime clock sound file
#define USER_CHORD_FILE   "heather_chord.wav"     // user defined sounds

#define CHORD_FILE        "c:\\WINDOWS\\Media\\chord.wav"      // standard Windows sound
#define NOTIFY_FILE       "c:\\WINDOWS\\Media\\notify.wav"     // standard Windows sound
EXTERN u08 chord_file;    // flag set if user chord file exists
EXTERN u08 notify_file;   // flag set if user notify file exists

#define LOCK_FILE         "tblock"                // lock file (exists while screen/log dumps in progress)
#define DELTAT_FILE       "deltat.dat"            // user specifyed UT1 delta-T
#define LLA_FILE          "lla.lla"               // lat/lon/alt fix data

EXTERN u08 chime_file;    // flag set if user chime file exists
EXTERN u08 alarm_file;    // flag set if user alarm file exists
EXTERN u08 leap_file;     // flag set if user leapsecond alarm file exists
EXTERN u08 sun_file;      // flag set if user sunrise/sunset tune file exists
EXTERN u08 noon_file;     // flag set if user solar noon tune file exists
EXTERN u08 bell_file;     // flag set if ships bell file exists



EXTERN int com_port;           // COM port number to use
EXTERN int usb_port;           // USB serial port number to use (for Linux ttyUSB#)
EXTERN int lpt_port;           // LPT port number to use for temperature control
EXTERN int com_fd;             // linux com port file handle
EXTERN unsigned port_addr;     // com chip I/O address (for DOS mode)
EXTERN int int_mask;           // and interrupt level mask (for DOS mode)
EXTERN u08 com_running;        // flag set if com port has been initialized
EXTERN char com_dev[256+1];    // linux serial port device name

EXTERN u08 com_data_lost;      // flag set if com port data stream interrupted
EXTERN double last_com_time;

EXTERN int com_q_in;           // serial port data queue (for DOS mode)
EXTERN int com_q_out;
EXTERN u32 com_errors;

EXTERN u08 read_only;          // if set, block TSIP commands that change the oscillator config
EXTERN u08 just_read;          // if set, only read TSIP data,  don't do any processing
EXTERN u08 no_send;            // if set, block data from going out the serial port
EXTERN u08 no_poll;            // if set, blocks all polled message requests
EXTERN u08 process_com;        // clear flag to not process the com port data on startup
                               // useful if just reviewing a log file and no tbolt is connected
                               // and your serial port has other data comming in

EXTERN int break_flag;         // flag set when ctrl-break pressed
EXTERN u08 system_busy;        // flag set when system is processing lots of messages
EXTERN u08 first_msg;          // flag set when processing first message from com port
                               // (used to get in sync to data without flagging an error)

#define USER_CMD_LEN (256+1)
EXTERN u08 user_init_cmd[USER_CMD_LEN];  // user specified init commands
EXTERN int user_init_len;
EXTERN u08 user_pps_cmd[USER_CMD_LEN];   // user_specified pps commands
EXTERN int user_pps_len;


#define MAX_TSIP 2048             // max length of a TSIP message
EXTERN u08 tsip_buf[MAX_TSIP+1];  // incoming message is built here
EXTERN u16 tsip_wptr;             // incoming message write pointer
EXTERN u16 tsip_rptr;             // incoming message read pointer
EXTERN u16 tsip_sync;             // incoming message state


#define RCVR_BUF_SIZE 1024     // size of com port data buffers
EXTERN u08 rcvr_buf[RCVR_BUF_SIZE];
EXTERN DWORD rcvr_byte_count;
EXTERN DWORD next_serial_byte;


#define LLA_X         (LLA_COL+LLA_SIZE/2-1)   // center point of lla plot
#define LLA_Y         (LLA_ROW+LLA_SIZE/2-1)
#define LLA_DIVISIONS 10       // lla plot grid divisons
EXTERN double LLA_SPAN;        // feet or meters each side of center
EXTERN int LLA_SIZE;           // size of the lla map window (for DOS should be a multiple of TEXT_HEIGHT)
EXTERN int LLA_MARGIN;         // lla plot grid margins in pixels
EXTERN int lla_step;           // pixels per grid division
EXTERN int lla_width;          // size of data area in the grid
EXTERN int LLA_ROW;            // location of the lla map window
EXTERN int LLA_COL;
EXTERN u08 plot_lla;           // flag set when plotting the lat/lon/alt data
EXTERN u08 graph_lla;          // flag set when graphing the lat/lon/alt data
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
EXTERN double ref_lat;         // the reference position for LLA scatter plots
EXTERN double ref_lon;
EXTERN double ref_alt;

EXTERN int WATCH_ROW;     // where the AZEL plot goes, if not in the plot area
EXTERN int WATCH_COL;     // ... also where to draw the analog watch
EXTERN int WATCH_SIZE;
EXTERN int ACLOCK_SIZE;
EXTERN int aclock_x,aclock_y;

#define ACLOCK_R     (ACLOCK_SIZE/2-TEXT_WIDTH)
#define AA           ((float)ACLOCK_R/8.0F)
#define ALARM_WIDTH  2   // how many degrees wide the alarm time marker is
#define WATCH_HOURS  (12*((watch_face/NUM_FACES)+1))  //12
#define WATCH_STEP   (360/WATCH_HOURS)
#define WATCH_MULT   (WATCH_HOURS/12)
#define hand_angle(x) ((270.0F+((x)*6.0F)) * PI/180)

EXTERN int AZEL_SIZE;     // size of the az/el map window (for DOS should be a multiple of TEXT_HEIGHT)
EXTERN int AZEL_ROW;      // location of the az/el map window
EXTERN int AZEL_COL;
EXTERN int AZEL_MARGIN;   // outer circle margin in pixels

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


EXTERN u32 this_track;       // bit vector of currently tracked sats
EXTERN u32 last_track;       // bit vector of previously tracked sats
EXTERN u32 this_used;        // bit vector of currently used sats
EXTERN u32 last_used;        // bit vector of previously used sats
EXTERN u32 last_q1, last_q2, last_q3, last_q4;  // tracks sats that are in each quadrant
EXTERN u32 this_q1, this_q2, this_q3, this_q4;
EXTERN u08 shared_plot;      // set flag if sharing plot area with az/el map
EXTERN u08 share_active;     // flag set if plot area is being shared
EXTERN int shared_item;      // the item in the shared plot area


EXTERN u08 show_min_per_div;  // set flag to show display in minutes/division
EXTERN u08 showing_adv_file;  // flag set if adev file has been loaded
EXTERN u08 old_sat_plot;
EXTERN u08 old_adev_plot;
EXTERN u08 old_keep_fresh;
EXTERN float old_adev_period;
EXTERN u08 debug_screen;
EXTERN char user_view_string[MAX_PRN+2+1];  // +2 is for sun/moon

EXTERN int dac_dac;          // osc autotune state
EXTERN float gain_voltage;   // initial dac voltage


#define MAX_TEXT_COLS (2560/8)
#define UNIT_LEN 21            // unit name length
#define NMEA_MSG_SIZE 512

EXTERN char blanks[MAX_TEXT_COLS+1]; // 80 of them will blank a screen line
EXTERN char out[MAX_TEXT_COLS+1];    // screen output strings get formatted into here
EXTERN char unit_name[UNIT_LEN+1];   // unit name identifier
EXTERN char scpi_mfg[20+1];
EXTERN char scpi_fw[128];
EXTERN char scpi_sn[128+1];
EXTERN char scpi_model[UNIT_LEN+1];
EXTERN char scpi_serno[NMEA_MSG_SIZE+1];
EXTERN char scpi_mfg_id[NMEA_MSG_SIZE+1];
EXTERN int scpi_msg_id;

EXTERN char ubx_sw[30];
EXTERN char ubx_hw[10];
EXTERN char ubx_rom[30];
EXTERN float ubx_fw_ver;

EXTERN char sirf_sw_id[NMEA_MSG_SIZE+1];


EXTERN char star_module[12+1];        // STAR_RCVR version info
EXTERN char star_article[12+1];
EXTERN char star_sn[12+1];
EXTERN char star_hw_ver[12+1];
EXTERN char star_fw_article[12+1];
EXTERN char star_fw[12+1];
EXTERN char star_test_date[12+1];
EXTERN char star_test_version[12+1];
EXTERN char star_osc[12+1];
EXTERN char star_fpga[12+1];
EXTERN char star_family[12+1];
EXTERN char star_variant[12+1];
EXTERN int star_restart_ok;           // if flag set, send RESTART(W) if com data loss seen

EXTERN char venus_kern[32];
EXTERN char venus_odm[32];
EXTERN char venus_rev[32];
EXTERN u32  venus_kernel;

EXTERN char zod_chans[32];           // Zodiac ID info
EXTERN char zod_sw[32];
EXTERN char zod_date[32];
EXTERN char zod_opt[32];
EXTERN char zod_rsvd[32];

EXTERN char watch_name[30+1];        // analog watch brand name
EXTERN char watch_name2[30+1];
EXTERN u08  label_watch_face;        // if set label the analog watch
EXTERN u08  user_set_watch_name;     // if set, the user set the watch name

EXTERN char help_path[MAX_PATH+1];   // tweaked argv[0]

EXTERN char plot_title[SLEN+80+1];   // graph title string ////!!!!80
EXTERN char debug_text[SLEN+80+1];   // used to display debug info in the plot area
EXTERN char debug_text2[SLEN+80+1];
EXTERN char debug_text3[SLEN+80+1];
EXTERN int title_type;
EXTERN u08 greet_ok;
#define NONE      0
#define GREETING  1
#define USER      2
#define OTHER     3

EXTERN char edit_buffer[SLEN+1];     // the text string the user is typing in
EXTERN char last_user_cmd[SLEN+1];   // the last user command sent to the receiver
EXTERN char *edit_info1;             // additional edit prompt info to display
EXTERN char *edit_info2;
EXTERN char *edit_info3;
EXTERN char *edit_info4;
EXTERN u16 getting_string;           // the command that will use the stuff we are typing
EXTERN u08 edit_ptr;                 // the edit cursor column
EXTERN int EDIT_ROW, EDIT_COL;       // where to show the edit string

EXTERN char *ppb_string;             // parts per billion identifier
EXTERN char *ppt_string;             // parts per trillion identifier


//
// function definitions
//
void draw_circle(int x,int y, int r, int color, int fill);
void erase_rectangle(int x,int y, int width,int height);
void line(COORD x1,COORD y1, COORD x2,COORD y2, u08 color);
void thick_line(COORD x1,COORD y1, COORD x2,COORD y2, u08 color, u08 thickness);
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
void init_com(void);
int auto_detect(void);
void scpi_init(int type);
int status_second(int seconds);

void sendout(u08 val);
void bios_char(u08 c, u08 attr);
void scr_rowcol(COORD row, COORD col);
int check_incoming_data(void);

void init_screen(void);
void config_screen(int why);
void new_screen(u08 c);
void change_screen_res();
void edit_screen_res();
void config_sat_count(int sat_count);
void config_sat_rows();
void config_lla_zoom(int why);
void set_restore_size();

int dump_screen(int invert, int top_line, char *fn);
int dump_bmp_file(int invert, int top_line);
int dump_gif_file(int invert, int top_line, FILE *file);
void erase_cursor_info(void);
void play_tune(char *file, int add_root);


void draw_plot(u08 refresh_ok);
void update_plot(int draw_flag);
void erase_plot(int full_plot);
void erase_help(void);
void erase_screen(void);
void erase_vchar(int x, int y);
void scale_plots(void);
int  get_mouse_info(void);
void vchar(int xoffset,int yoffset, u08 erase, u08 color, u08 c);
void vchar_string(int row, int col, u08 color, char *s);
void center_vstring(int row, int scale, int color, char *s);
void reset_vstring(void);
float scale_temp(float t);
char *fmt_temp(float t);
void restore_plot_config(void);
void filter_spikes(void);
void adev_mouse(void);
void view_all(void);

void alloc_queues(void);
void alloc_plot(void);
void alloc_adev(void);
void alloc_fft(void);
void alloc_gif(void);
void reset_queues(int queue_type);
void new_queue(int queue_type);
void clear_all_data(void);
void write_q_entry(FILE *file, long i);
long find_event(long i, u16 flags);
long goto_event(u16 flags);
void put_plot_q(long i, struct PLOT_Q q);
long next_q_point(long i, int stop_flag);
struct PLOT_Q get_plot_q(long i);
struct PLOT_Q filter_plot_q(long i);
float calc_cct(int type, int undo_scale, float red, float green, float blue);
void set_cct_id(void);
void update_check(void);

void do_gps(void);
void get_pending_gps(void);
void key_wait(char *s);
void wait_for_key(int serve);
void abort_wakeup(void);
void init_messages(int why);
void get_rcvr_message(void);
u08 tsip_end(u08 report_err);
void unknown_msg(u16 msg_id);
void debug_stream(unsigned c);

void set_survey_params(u08 enable_survey,  u08 save_survey, u32 survey_len);
void start_self_survey(u08 val, int why);
void stop_self_survey(void);
void request_rcvr_info(int why);
void set_filter_config(u08 pv, u08 stat, u08 alt, u08 kalman, u08 marine, int save);
void set_filter_factor(float val);
void set_nav_rate(float hz);
void request_cold_reset(void);
void set_discipline_mode(u08 mode);
void update_osc_params(void);
void save_segment(u08 segment, int why);
void request_timing_mode(void);
void set_timing_mode(u08 mode);
void get_timing_mode(void);
void request_factory_reset(void);
void request_self_tests(void);
void request_version(void);
void set_pps(u08 pps_enable,  u08 pps_polarity,  double cable_delay, double pps_delay, float threshold, int save);
void set_pps_freq(int chan, double freq, double duty, int polarity, double cable_delay, double pps_delay, int save);
void set_ref_input(int source);
void request_warm_reset(void);
void enable_moto_binary(void);
void enable_moto_nmea(void);
void set_osc_sense(u08 mode, int save);
void set_dac_voltage(float volts, int why);
void calc_osc_gain(void);
void request_discipline_params(u08 type);
void request_all_dis_params(void);
void set_discipline_params(int save);
void request_survey_params(void);
void set_pullin_range(int i);
void request_sat_list(void);
void request_primary_timing(void);
void request_secondary_timing(void);
void request_pps_info(void);
void send_byte(u08 val);
void wakey_wakey(int force_sirf);
void do_fixes(int mode);
void change_fix_config(int save);
void change_zoom_config(int save);
void start_3d_fixes(int mode, int why);
void config_fix_display(void);
void set_config(u08 mode, u08 dynamics, float elev, float amu, float pdop_mask, float pdop_switch, u08 foliage);
void request_rcvr_config(int why);
void set_single_sat(u08 prn);
void exclude_sat(u08 prn);
void update_disable_list(u32 val);
int  highest_sat();
void set_io_options(u08 posn, u08 vel, u08 timing, u08 aux);
void start_self_survey(u08 val);
void save_segment(u08 segment);
void saw_ebolt(void);
void set_cal_mode(u08 cal_mode);
void set_gnss_system(int system);
void request_gnss_system();
int get_clock_time();

void show_cursor_info(S32 i);
void show_title(void);
void show_stat_info(void);
int show_digital_clock(int row, int col);
int  show_greetings(void);
void show_log_state(void);
void show_time_info();
void show_satinfo(void);
void show_sun_moon(int row, int col);
void show_sun_azel(int row, int col);
void show_moon_azel(int row, int col);
void show_pid_values(void);
void show_osc_params(int row, int col);
void show_osc_pid_values(void);
void show_manuf_params();
void show_unit_info();
void show_version_header(void);
void show_version_info(void);
void show_gpsd_driver();
void show_ebolt_info();
int adevs_active(int check_enable);

void start_3d_fixes_survey(void);
void start_precision_survey(void);
void stop_precision_survey(void);
void abort_precise_survey(int why);
void save_precise_posn(int force_save);
int add_to_bin(double val,  double bin[],  int bins_filled);
void plot_lla_point(int draw, int color);
void rebuild_lla_plot(int draw);
void precise_check(void);
int add_survey_point(void);
void analyze_hours(void);
void calc_precise_lla(void);
void update_precise_survey(void);

void plot_lla_axes(int why);
void lla_header(char *s, int color);
void set_lla(double lat, double lon, double alt);
void format_lla(double lat, double lon,  double alt,  int row, int col);
void erase_lla(void);
void clear_signals(void);
void clear_lla_points(void);
void update_mouse(void);
void hide_mouse(void);
u08 mouse_hit(COORD x1,COORD y1,  COORD x2,COORD y2);
void open_lla_file(int why);
void close_lla_file(int cleanup);
void check_com_timer();
void reset_com_timer();

void draw_maps(void);
void clear_maps(void);
void draw_azel_plot(void);
void draw_signal_map(void);
void check_azel_changes(void);
void clear_sat_trails(void);
void update_sat_trails(void);
void erase_azel(void);
void dump_trails(void);
void plot_3d_sig(int az, int el);
void log_signal(float azf, float elf, float sig_level, int amu_flag);
void log_sun_posn(void);
void dump_signals(char *fn);
float amu_to_dbc(float sig_level);
float dbc_to_amu(float sig_level);
void label_circles(int signals, int row);

void format_plot_title(void);
FILE *open_log_file(char *mode);
void close_log_file(void);
void sync_log_file(void);
void dump_log(char *name, u08 dump_size);
void write_log_leapsecond(void);
void write_log_readings(FILE *file, long i);
void write_log_changes(void);
void write_log_error(char *s, u32 val);
void write_log_utc(int utc_offset);
void write_log_comment(int spaces);
void log_posn_bins(void);
void log_saved_posn(int type);
void log_adevs(void);
int calc_sunrise(double delta, int why);
int calc_moonrise(void);
u32 fake_tow(double jd);
int reload_log(char *fn, u08 cmd_line);
int time_check(int reading_log, float interval, int yy,int mon,int dd, int hh,int mm,int ss, double frac);
double GetMsecs(void);
double GetNsecs(void);

void need_time_set(void);
void init_dsm(void);
int leap_year(int year);
void bump_time(void);
void set_time_zone(char *s);
int adjust_tz(int why);
void calc_dst_times(char *s);
void moon_posn(double jd_tt);
double sun_posn(double jd, int do_moon);
double sun_diam(double jd);
double moon_diam(double jd);
void record_sig_levels(int prn);
int dst_offset(void);
void setup_calendars(int why);
void calc_seasons(void);
double new_moon(double jd);
void calc_moons(int why);
void erase_watch(void);
double jdate(int y, int m, int d);
double jtime(int hh, int mm, int ss, double frac);
double mars_date(int y,int m,int d, int hh,int mm,int ss, double frac);
double gmst(int yy, int mo, int dd, int hh, int mm, int ss, int app_flag);
double lmst(int yy, int mo, int dd, int hh, int mm, int ss, int app_flag);
double eot(double jd);
double utc_delta_t();
void get_alarm_time(void);
void set_watch_name(char *s);
char *tz_info();
double time_zone();

int do_kbd(int c);
void kbd_help(void);
int are_you_sure(int c);
int sure_exit(void);
u08 toggle_value(u08 x);
int option_switch(char *arg);
int read_config_file(char *name, u08 local_flag, u08 add_path);
void config_options(void);
void save_cmd_bytes(char *arg);
void command_help(char *where, char *s, char *cfg_path);
u32 get_bgr_palette(int i);
void setup_palette(void);
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
int good_el_level(void);
float set_el_level(void);
FILE *topen(char *s, char *m);
FILE *path_open(char *s, char *m);
FILE *open_debug_file(char *s);
int path_unlink(char *s);
void open_script(char *fn);

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
void silly_clocks(void);
void calc_greetings(void);
void set_cpu_clock(void);
int draw_watch(int why);
char *fmt_date(int big_flag);
void gregorian(double jd);
double moon_phase(double jd);
double moon_info(double jd);
int lookup_new_moon(double jd);
double moon_synod(double jd);
double moon_age(double jd);
double earth_radius(double lat, double alt);
double earth_sun_dist(double jd);
void debug_date(char *s, double jd);

float scale_adev(float val);
void reset_adev_bins(void);
void reload_adev_info(void);
void reload_adev_queue(void);
void force_adev_redraw(void);
long next_tau(long tau, int bins);
void find_global_max(void);

void plot_review(long i);
void do_review(int c);
void end_review(u08 draw_flag);
void zoom_review(long i, u08 beep_ok);
void kbd_zoom(void);
void goto_mark(int mark);
void reset_marks(void);
void adjust_view(void);
void new_view(void);
int edit_user_view(char *s);
void show(char *s);

void SetDtrLine(u08 on);
void SetRtsLine(u08 on);
void SetBreak(u08 on);
void SendBreak(void);
void do_term();

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
   EXTERN float  desired_temp;        // desired unit temperature
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
void config_rcvr_plots(void);
void config_rcvr_type(int set_baud);
void config_msg_ofs();
void config_normal_plots(void);
void config_lla_plots(int keep, int show);
int config_extra_plots(void);
void dump_stream(int flag);
void send_user_cmd(char *s);
void send_nmea_cmd(char *s);
void set_venus_mode(int mode);

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
void set_luxor_time();
void set_emissivity(float em1, float em2);
u08 luxor_fault(void);
int get_com_char(void);

EXTERN float test_heat, test_cool;
EXTERN int test_marker;
EXTERN int bang_bang;

EXTERN u08 pid_debug;
EXTERN int crude_temp;
EXTERN float P_GAIN;
EXTERN float D_TC;
EXTERN float FILTER_TC;
EXTERN float I_TC;
EXTERN float FILTER_OFFSET;
EXTERN float KL_TUNE_STEP;
EXTERN float OLD_P_GAIN;

#define PRE_Q_SIZE  60
EXTERN int osc_prefilter;   // OSC pid pre filter (input values to the PID) depth
EXTERN int opq_in;
EXTERN int opq_count;

#define POST_Q_SIZE 600
EXTERN int osc_postfilter;  // OSC pid post filter (between PID and DAC) depth
EXTERN int post_q_count;
EXTERN int post_q_in;


void calc_osc_k_factors(void);
void clear_osc_pid_display(void);
void set_default_osc_pid(int num);
void new_postfilter(void);

EXTERN u08 osc_pid_debug;
EXTERN double OSC_P_GAIN;
EXTERN double OSC_D_TC;
EXTERN double OSC_FILTER_TC;
EXTERN double OSC_I_TC;
EXTERN double OSC_FILTER_OFFSET;
EXTERN double OSC_KL_TUNE_STEP;
EXTERN double OLD_OSC_P_GAIN;

void enable_osc_control(void);
void disable_osc_control(void);
void osc_pid_filter(void);
void control_osc(void);
EXTERN u08 osc_control_on;
EXTERN double osc_rampup;

EXTERN double avg_pps;
EXTERN double avg_osc;
EXTERN int pps_bin_count;

EXTERN int monitor_pl;              // monitor power line freq
EXTERN unsigned long pl_counter;


EXTERN float dac_drift_rate;
EXTERN u08 show_filtered_values;
EXTERN float d_scale;

EXTERN double trick_scale;
EXTERN int    first_trick;
EXTERN double trick_value;

#define RMS       0x01
#define AVG       0x02
#define SDEV      0x04
#define VAR       0x08
#define SHOW_MIN  0x10
#define SHOW_MAX  0x20
#define SHOW_SPAN 0x40
EXTERN int stat_type;
EXTERN char *stat_id;

struct PLOT_DATA {
   char *plot_id;      // the id string for the plot
   char *units;        // the plot data units
   float ref_scale;    // scale_factor is divided by this to get the units of
                       // measurement to manipulate the scale factor in
   u08 show_plot;      // set flag to show the plot
   u08 float_center;   // set flag to allow center line ref val to float
   int plot_color;     // the color to draw the plot in

   float plot_center;  // the calculated center line reference value;
   u08   user_scale;   // flag set if user set the scale factor
   float scale_factor; // the plot scale factor
   float data_scale;   // data plot scale factor
   float invert_plot;  // if (-1.0) invert the plot

   float sum_x;        // linear regression stuff
   float sum_y;
   float sum_xx;
   float sum_yy;
   float sum_xy;
   float stat_count;
   float a0;
   float a1;
   float drift_rate;   // drift rate to remove from plot
   float sum_change;

   float min_val;      // min and max values of the display window
   float max_val;
   int   last_y;       // last plot y-axis value
   u08 old_show;       // previous value of show_plot flag (used to save/restore options when reading a log file)
   u08 show_trend;     // set flag to show trend line
   u08 show_stat;      // set flag to statistic to show
   int last_trend_y;
};
extern struct PLOT_DATA plot[];

EXTERN int num_plots;      // how many plots we are keeping data for
EXTERN int extra_plots;    // flag set if any of the extra plots are enabled
EXTERN int selected_plot;       // the currently selected plot
EXTERN int last_selected_plot;  // the last selected plot
EXTERN int no_plots;            // if set, disable plot area
EXTERN int dynamic_trend_info;  // if set, update trend line info plot title dynamically


#ifdef FFT_STUFF
   #define BIGUN HUGE_MEM

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
   EXTERN float BIGUN *tsignal;     // the fft input data
   EXTERN COMPLEX BIGUN *w;         // the fft w array
   EXTERN COMPLEX BIGUN *cf;        // the fft cf array
   EXTERN COMPLEX BIGUN *fft_out;   // the fft results
#endif

