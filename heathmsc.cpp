#define EXTERN extern
#include "heather.ch"

// Trimble Thunderbolt GPSDO TSIP monitor program (and now a bunch of other devices)
//
// Original Win32 port by John Miles, KE5FX
//
// (Temperature and oscillator control algorithms by Warren Sarkison
//
// Adev code adapted from Tom Van Baak's adev1.c and adev3.c
// Incremental adev code based upon John Miles' TI.CPP
//
// Easter and moon illumination code from voidware.com
//
// Equation of time code adapted from code by Mike Chirico and NOAA
//
// Sun position code is based upon Grena's Algorithm 5
//
// Moon position from code by Paul Schlyter at
//   http://hotel04.ausys.se/pausch/comp/ppcomp.html
//
// Moon image code derived from code by Mostafa Kaisoun.
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
// The LZW encoder used in this file was derived from code written by Gershon
// Elber and Eric S. Raymond as part of the GifLib package.  And by Lachlan
// Patrick as part of the GraphApp cross-platform graphics library.
//
//
// Note: This program calculates adevs based upon the pps and oscillator
//       error values reported by the unit.  These values are not
//       derived from measurements made against an independent reference
//       (other than the GPS signal) and may not agree with adevs calculated
//       by measurements against an external reference signal (particularly
//       at smaller values of tau).
// 
//
//  This file contains code for various major functional groups:
//     Log file I/O
//     ADEVs
//     Precise position survey
//     AZ/EL map display
//     Daylight savings time support and other time related stuff
//     Digital clock display
//     Analog watch display
//     Calendars and greetings
//     Astronomical time scales and time zone conversions
//     GIF file output
//     Active temperature control
//     Alternate oscillator control algorithm
//     FFT calculations
//     Whatever else I crammed in here
//     

extern char *months[];
extern u08 bmp_pal[];

#define USE_L2  1              // if set, output L2 data in RINEX .obs files
#define USE_L5  1              // if set, output L5 data in RINEX .obs files
#define USE_L6  1              // if set, output L6 data in RINEX .obs files
#define USE_L7  1              // if set, output L7 data in RINEX .obs files
#define USE_L8  1              // if set, output L8 data in RINEX .obs files
#define RAW_CHANGES rinex_fix  // if flag set ignore raw values that are not changing

#define LOG_GPS_CHECK   5000   // check for pending GPS messages every this many entries when reading logs
#define LOG_SCREEN_RFSH 10000  // refresh screen every this many entries when reading logs

#define MTIE_SCALE 1.010       // increase/decrease global_adev_max/min by this when plotting MTIE for margins


#define MOON_COLOR YELLOW      // default moon image color
#define TRUE_MOON              // define this to draw the moon on the watch face at its celestial position
#define MESS 0                 // if 1,  allow watch, map, and signals to overlay

#define MOON_SIZE  (moon_size/2)
#define MAX_MARKER_SIZE  14
#define MIN_MARKER_SIZE  3

int moon_color = MOON_COLOR;   // the color to draw the moon in
int moon_x, moon_y;            // where to draw the moon on the watch face
int moon_size;                 // size to draw the moon at

int have_moon_el;
double moon_phase_acc;  // a better moon phase number


#define SOLAR_YEAR 365.2421897 // mean length of solar year in days

double sun_hlon;        // heliocentric longitude in degrees
int sunset;
int sunrise;

int lla_xi, lla_yi;     // location of last plotted LLA point


int last_minute = 99;

#define LEVEL_COLORS 11      // how many signal level steps are available
#define LOW_SIGNAL   GREY    // show signals below min_sig_db in this color


int level_color[] = {        // converts signal level step to dot color
   BLACK,
   // LOW_SIGNAL is at this step
   DIM_RED,      // 1 = 30
   RED,
   BROWN,
   DIM_MAGENTA,  // actually grotty yellow
   YELLOW,       // 5 = 38
   DIM_BLUE,     // 6 = 40
   BLUE,
   CYAN,
   DIM_GREEN,
   GREEN,
   DIM_WHITE     // 11 = 50
};

#define MAX_SAT_COLOR     14
#define GREY_SAT_COLOR    11
#define YELLOW_SAT_COLOR  14   // this should be YELLOW - the sun
int sat_colors[] = {
   BROWN,
   BLUE,
   GREEN,
   CYAN,
   DIM_BLUE,
   MAGENTA,
   WHITE,      
   DIM_GREEN,
   DIM_CYAN,
   DIM_RED,
   DIM_MAGENTA,
   GREY,
   RED,
   DIM_WHITE,
   YELLOW        // should be last color - used to draw the sun
};


// used for both precise survey and oscillator autotuning
int hour_lats;
int hour_lons;
double lat_hr_bins[SURVEY_BIN_COUNT+1];
double lon_hr_bins[SURVEY_BIN_COUNT+1];

char *days[] = {
   "Sunday",
   "Monday",
   "Tuesday",
   "Wednesday",
   "Thursday",
   "Friday",
   "Saturday",
};

int dim[] = {   // days in the month (gets patched for leap years)
   0,
   31,   //jan
   28,   //feb
   31,   //mar
   30,   //apr
   31,   //may
   30,   //jun
   31,   //jul
   31,   //aug
   30,   //sep
   31,   //oct
   30,   //nov
   31    //dec
};

char *months[] = { // convert month to its ASCII abbreviation
   "???",
   "Jan", "Feb", "Mar", "Apr", "May", "Jun",
   "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

char *full_months[] = {
   "???",
   "January",
   "February",
   "March",
   "April",
   "May",
   "June",
   "July",
   "August",
   "September",
   "October",
   "November",
   "December"
};

#define LLA_THRESH 1.05    // LLA span must increase by this amount before re-scaling
char lla_msg[MAX_TEXT_COLS+1];
int sp_count;  // used to verify optimization of solar noon/new moon calculations

float bottom_adev_decade;

void zoom_all_signals(void);
void zoom_stuff(void);
void zoom_sat_info(void);

void write_rinex_header(FILE *file);


//
//
// Log stuff
//
//
void show_log_state()
{
char *mode;
char c;
int row;
int shown;
int needed;
int avail;
char s[32];
#define MAX_LOG_NAMES 2


   if(text_mode && first_key) return;
   if(zoom_screen) return;

   show_serial_info();

   if(luxor) row = VER_ROW+6;
   else      row = VER_ROW+5;

   shown = 0;
   hide_op_mode = 0;
   avail = (6-4) + 1;
///if(all_adevs && (all_adev_row > row)) avail = all_adev_row - row;

   needed = 0;
   if(log_file) ++needed;
   else if(log_loaded) ++needed;
   else if(log_stream & LOG_HEX_STREAM) ++needed;

   if(raw_file) ++needed;
   if(prn_file) ++needed;
   if(ticc_file) ++needed;
   if(debug_file) ++needed;
if(ticc_port_open()) ++needed;
if(enviro_port_open()) ++needed;

   if(needed > avail) 

   vidstr(row, VER_COL, WHITE, "                       ");
   if(log_file) {
      ++shown;

      if(log_wrap) {
         vidstr(row, VER_COL, WHITE, "Log: on queue wrap");
      }
      else if(log_date || log_time) {
         vidstr(row, VER_COL, WHITE, "Log: timed");
      }
      else {
         if(log_mode[0] == 'a')               strcpy(s, "Cat");
         else if(log_stream & LOG_HEX_STREAM) strcpy(s, "Hex");
         else                                 strcpy(s, "Log");
         if(log_flush_mode) strupr(s);
         mode = &s[0];

         if((int)strlen(log_name) > 16) {
            sprintf(out, "%s:...%s", mode, &log_name[strlen(log_name)-16]);
         }
         else {
            sprintf(out, "%s: %s", mode, log_name);
         }
         vidstr(row, VER_COL, WHITE, out);
      }
      ++row;
   }
   else if(log_stream & LOG_HEX_STREAM) {  // debug packet dump mode
      ++shown;
      if(log_loaded) vidstr(row, VER_COL, WHITE, "Hex: loaded       ");
      else           vidstr(row, VER_COL, WHITE, "Hex: OFF          ");
      ++row;
   }
   else if(log_loaded) {
      ++shown;
      vidstr(row, VER_COL, WHITE, "Log: loaded       ");
      ++row;
   }
   else if(needed <= avail) {
      ++shown;
      vidstr(row, VER_COL, WHITE, "Log: OFF          ");
      ++row;
   }

   // show raw/debug file names
   // We only have room to show two file names.  If more than one is needed, 
   // the name is followed by an '*'
   if(shown >= avail) {
      ++hide_op_mode;
      return;
   }

   vidstr(row, VER_COL, WHITE, "                       ");
   if(rinex_file && rinex_name[0]) {
      strcpy(s, "Rnx");
      if(rinex_flush_mode) strupr(s);
      c = ':';
      ++shown;
      if((shown == avail) && (raw_file || prn_file || debug_file || ticc_file)) c = '+';

      if(strlen(rinex_name) > 16) {
         sprintf(out, "%s%c...%s", s,c, &rinex_name[strlen(rinex_name)-16]);
      }
      else {
         sprintf(out, "%s%c %s", s,c, rinex_name);
      }

      vidstr(row, VER_COL, WHITE, out);
      ++row;
   }
   if(shown >= avail) {
      ++hide_op_mode;
      return;
   }

   vidstr(row, VER_COL, WHITE, "                       ");
   if(ticc_file && ticc_name[0]) {
      mode = "TIC";
      c = ':';
      ++shown;
      if((shown == avail) && (raw_file || prn_file || debug_file)) c = '+';

      if(strlen(ticc_name) > 16) {
         sprintf(out, "%s%c...%s", mode,c, &ticc_name[strlen(ticc_name)-16]);
      }
      else {
         sprintf(out, "%s%c %s", mode,c, ticc_name);
      }

      vidstr(row, VER_COL, WHITE, out);
      ++row;
   }
   if(shown >= avail) {
      ++hide_op_mode;
      return;
   }

   vidstr(row, VER_COL, WHITE, "                       ");
   if(raw_file && raw_name[0]) {
      if(raw_flush_mode) mode = "CAP";
      else               mode = "Cap";
      c = ':';
      ++shown;
      if((shown == avail) && (prn_file || debug_file)) c = '+';

      if(strlen(raw_name) > 16) {
         sprintf(out, "%s%c...%s", mode,c, &raw_name[strlen(raw_name)-16]);
      }
      else {
         sprintf(out, "%s%c %s", mode,c, raw_name);
      }

      vidstr(row, VER_COL, WHITE, out);
      ++row;
   }
   if(shown >= avail) {
      ++hide_op_mode;
      return;
   }

   vidstr(row, VER_COL, WHITE, "                       ");
   if(debug_file && debug_name[0]) {
      if(dbg_flush_mode) mode = "DBG";
      else mode = "Dbg";
      c = ':';
      ++shown;
      if((shown == avail) && (prn_file)) c = '+';

      if((int)strlen(debug_name) > 16) {
         sprintf(out, "%s%c...%s", mode,c, &debug_name[strlen(debug_name)-16]);
      }
      else {
         sprintf(out, "%s%c %s", mode,c, debug_name);
      }

      vidstr(row, VER_COL, WHITE, out);
      ++row;
   }
   if(shown >= avail) {
      ++hide_op_mode;
      return;
   }

   vidstr(row, VER_COL, WHITE, "                       ");
   if(prn_file && prn_name[0]) {
      if(prn_flush_mode) mode = "PRN";
      else mode = "Prn";
      c = ':';
      ++shown;

      if((int)strlen(prn_name) > 16) {
         sprintf(out, "%s%c...%s", mode,c, &prn_name[strlen(prn_name)-16]);
      }
      else {
         sprintf(out, "%s%c %s", mode,c, prn_name);
      }

      vidstr(row, VER_COL, WHITE, out);
      ++row;
   }
   if(shown >= avail) {
      ++hide_op_mode;
      return;
   }

   vidstr(row, VER_COL, WHITE, "                       ");
   if(ticc_port_open()) {  // TICC is an auxiliary input device
      ++shown;

      if(ticc_sim_file && ticc_sim_eof)                 sprintf(out, "TIC: end of sim file  "); // simulation file eof
      else if(ticc_sim_file)                            sprintf(out, "TIC: simulation file  "); // simulation file
      else if(com[TICC_PORT].usb_port == USE_IDEV_NAME) sprintf(out, "TIC: %-17.17s", com[TICC_PORT].com_dev); // user specified device
      else if(com[TICC_PORT].usb_port == USE_DEV_NAME)  sprintf(out, "TIC: %-17.17s", com[TICC_PORT].com_dev); // symlink to /dev/heather
      else if(com[TICC_PORT].usb_port >= 10)            sprintf(out, "TIC: %-2d               ", com[TICC_PORT].usb_port-1);
      else if(com[TICC_PORT].usb_port)                  sprintf(out, "TIC: %-2d               ", com[TICC_PORT].usb_port-1);
#ifdef WINDOWS
      else if(com[TICC_PORT].com_port >= 10) sprintf(out, "TIC: %-2d               ", com[TICC_PORT].com_port);
      else if(com[TICC_PORT].com_port)       sprintf(out, "TIC: %-2d               ", com[TICC_PORT].com_port);
#else   // __linux__  __MACH__  __FreeBSD__
      else if(com[TICC_PORT].com_port >= 10) sprintf(out, "TIC: %-2d               ", com[TICC_PORT].com_port-1);
      else if(com[TICC_PORT].com_port)       sprintf(out, "TIC: %-2d               ", com[TICC_PORT].com_port-1);
#endif
#ifdef TCP_IP 
      else                              sprintf(out," TIC:%-17.17s", com[TICC_PORT].IP_addr); // TCP, no room for sernum
#else
      else                              sprintf(out, "TIC:none              ");
#endif
      if(strlen(out) > 25) strcpy(&out[25-3],"...");   // don't hit 'Power' label
      vidstr(row, VER_COL, WHITE, out);
      ++row;
   }
   if(shown >= avail) {
      ++hide_op_mode;
      return;
   }

   vidstr(row, VER_COL, WHITE, "                       ");
   if(enviro_port_open()) {  // envionmental sensor is an auxiliary input device
      ++shown;

      if(com[THERMO_PORT].usb_port == USE_IDEV_NAME)     sprintf(out, "ENV: %-17.17s", com[THERMO_PORT].com_dev); // user specified device
      else if(com[THERMO_PORT].usb_port == USE_DEV_NAME) sprintf(out, "ENV: %-17.17s", com[THERMO_PORT].com_dev); // symlink to /dev/heather
      else if(com[THERMO_PORT].usb_port >= 10)           sprintf(out, "ENV: %-2d               ", com[THERMO_PORT].usb_port-1);
      else if(com[THERMO_PORT].usb_port)                 sprintf(out, "ENV: %-2d               ", com[THERMO_PORT].usb_port-1);
#ifdef WINDOWS
      else if(com[THERMO_PORT].com_port >= 10) sprintf(out, "ENV: %-2d               ", com[THERMO_PORT].com_port);
      else if(com[THERMO_PORT].com_port)       sprintf(out, "ENV: %-2d               ", com[THERMO_PORT].com_port);
#else   // __linux__  __MACH__  __FreeBSD__
      else if(com[THERMO_PORT].com_port >= 10) sprintf(out, "ENV: %-2d               ", com[THERMO_PORT].com_port-1);
      else if(com[THERMO_PORT].com_port)       sprintf(out, "ENV: %-2d               ", com[THERMO_PORT].com_port-1);
#endif
#ifdef TCP_IP 
      else                              sprintf(out," ENV:%-17.17s", com[TICC_PORT].IP_addr); // TCP, no room for sernum
#else
      else                              sprintf(out, "ENV:none              ");
#endif
      if(strlen(out) > 25) strcpy(&out[25-3],"...");   // don't hit 'Power' label
      vidstr(row, VER_COL, WHITE, out);
      ++row;
   }
   if(shown >= avail) {
      ++hide_op_mode;
      return;
   }
}


void start_log_track()
{
int color;

   if(log_file == 0) return;

   ++gpx_track_number;
   if((log_fmt == GPX) || (log_fmt == XML)) {
      fprintf(log_file, "  <trk>\n");
      fprintf(log_file, "    <name>Lady Heather data</name>\n");
      fprintf(log_file, "    <number>%d</number>\n", gpx_track_number);
      fprintf(log_file, "    <trkseg>\n\n");
   }
   else if(log_fmt == KML) {
      color = (gpx_track_number % 15) + 8;  // 1..15
      if(color > 15) color = color - 15;

      fprintf(log_file, "\n");
      fprintf(log_file, "    <Folder>\n");
      fprintf(log_file, "      <name>Track %04d/%02d/%02d  %02d:%02d:%02d</name>\n", pri_year,pri_month,pri_day, pri_hours,pri_minutes,pri_seconds);
      fprintf(log_file, "      <description>Lady Heather Track</description>\n");
      fprintf(log_file, "      <visibility>1</visibility>\n");
      fprintf(log_file, "      <open>0</open>\n");
      fprintf(log_file, "\n");
      fprintf(log_file, "      <Placemark>\n");
      fprintf(log_file, "        <visibility>0</visibility>\n");
      fprintf(log_file, "        <open>0</open>\n");
      fprintf(log_file, "        <styleUrl>color%d</styleUrl>\n", color);
      fprintf(log_file, "        <name>Track %d</name>\n", gpx_track_number);
      fprintf(log_file, "        <description>Track coordinates</description>\n");
      fprintf(log_file, "\n");
      fprintf(log_file, "        <LineString>\n");
      fprintf(log_file, "        <extrude>1</extrude>\n");
      fprintf(log_file, "        <tessellate>1</tessellate>\n");
//    fprintf(log_file, "        <altitudeMode>clampToGround</altitudeMode>\n");
      fprintf(log_file, "        <altitudeMode>absolute</altitudeMode>\n");
      fprintf(log_file, "          <coordinates>\n");
   }
   if(log_flush_mode && log_file) fflush(log_file);
}

void end_log_track()
{
   if(log_file == 0) return;

   if((log_fmt == GPX) || (log_fmt == XML)) {
      fprintf(log_file, "    </trkseg>\n");
      fprintf(log_file, "  </trk>\n\n");
   }
   else if(log_fmt == KML) {
      fprintf(log_file, "          </coordinates>\n");
      fprintf(log_file, "        </LineString>\n");
      fprintf(log_file, "      </Placemark>\n");
      fprintf(log_file, "    </Folder>\n");
   }
   if(log_flush_mode && log_file) fflush(log_file);
}

void log_rcvr_type()
{
char *s;

   // NEW_RCVR

   if(log_file == 0) return;

   log_text[0] = 0;

   if     (rcvr_type == ACRON_RCVR)  sprintf(log_text, "  <name>Acron Zeit WWVB Receiver</name>");
   else if(rcvr_type == BRANDY_RCVR) sprintf(log_text, "  <name>Brandywine GPS-4 GPSDO</name>");
   else if(rcvr_type == CS_RCVR)     sprintf(log_text, "  <name>HP5071A cesium beam oscillator</name>");
   else if(rcvr_type == ESIP_RCVR)   sprintf(log_text, "  <name>Furuno eSIP receiver</name>");
   else if(rcvr_type == FURUNO_RCVR) sprintf(log_text, "  <name>Furuno PFEC receiver</name>");
   else if(rcvr_type == GPSD_RCVR)   sprintf(log_text, "  <name>GPSD client</name>");
   else if(rcvr_type == LPFRS_RCVR)  sprintf(log_text, "  <name>LPFRS rubidium oscillator</name>");
   else if(rcvr_type == MOTO_RCVR)   sprintf(log_text, "  <name>Motorola receiver</name>");
   else if(rcvr_type == NMEA_RCVR)   sprintf(log_text, "  <name>NMEA receiver</name>");
   else if(rcvr_type == NO_RCVR)     sprintf(log_text, "  <name>System clock</name>");
   else if(rcvr_type == RT17_RCVR)   sprintf(log_text, "  <name>Trimble RT17 receiver</name>");
   else if(rcvr_type == NVS_RCVR)    sprintf(log_text, "  <name>NVS receiver</name>");
   else if(rcvr_type == PRS_RCVR)    sprintf(log_text, "  <name>PRS-10 rubidium oscillator</name>");
   else if(rcvr_type == RFTG_RCVR)   {
      if((have_rftg_unit) && (rftg_unit == 0)) sprintf(log_text, "  <name>RFTG-m RB</name>");
      else if((have_rftg_unit) && (rftg_unit == 1)) sprintf(log_text, "  <name>RFTG-m XO</name>");
      else sprintf(log_text, "  <name>RFTG-m</name>");
   }
   else if(rcvr_type == SA35_RCVR)   sprintf(log_text, "  <name>SA.3xM rubidium oscillator</name>");
   else if(rcvr_type == SCPI_RCVR) {
      if(scpi_type == HP_TYPE)       sprintf(log_text, "  <name>HP58xxx SCPI GPSDO</name>");
      else if(scpi_type == HP_TYPE2) sprintf(log_text, "  <name>HP59xxx SCPI GPSDO</name>");
      else                           sprintf(log_text, "  <name>SCPI GPSDO</name>");
   }
   else if(rcvr_type == SIRF_RCVR)   sprintf(log_text, "  <name>SIRF receiver</name>");
   else if(rcvr_type == SRO_RCVR)    sprintf(log_text, "  <name>SRO100 rubidium oscillator</name>");
   else if(rcvr_type == SS_RCVR)     sprintf(log_text, "  <name>Novatel SuperStar II/name>");
   else if(rcvr_type == STAR_RCVR) {
      if(star_type == NEC_TYPE) sprintf(log_text, "  <name>NEC STAR-4 GPSDO</name>"); 
      else if(star_type == OSA_TYPE) sprintf(log_text, "  <name>OSA 453x GPSDO</name>"); 
      else sprintf(log_text, "  <name>Oscilloquartz STAR-4 GPSDO</name>"); 
   }
   else if(res_t)                    sprintf(log_text, "  <name>Trimble Resolution-T receiver</name>");
   else if(rcvr_type == TAIP_RCVR)   sprintf(log_text, "  <name>Trimble TAIP receiver</name>"); 
   else if(rcvr_type == THERMO_RCVR) {
      sprintf(log_text, "  <name>Environmental</name>");
   }
   else if(rcvr_type == TICC_RCVR) {
      if     (ticc_type == TAPR_TICC)   sprintf(log_text, "  <name>TAPR TICC</name>");
      else if(ticc_type == HP_TICC)     sprintf(log_text, "  <name>HP53xxx interval counter</name>");
      else if(ticc_type == PICPET_TICC) sprintf(log_text, "  <name>PICPET timestamp counter</name>");
      else if(ticc_type == LARS_TICC)   sprintf(log_text, "  <name>LARS GPSDO TIC</name>");
      else                              sprintf(log_text, "  <name>Generic interval counter</name>");
   }
   else if(rcvr_type == TIDE_RCVR)    sprintf(log_text, "  <name>Gravity clock</name>");
   else if(rcvr_type == TM4_RCVR)     sprintf(log_text, "  <name>Spectrum TM4 GPSDO</name>");
   else if(rcvr_type == TRUE_RCVR) {
      sprintf(log_text, "  <name>TruePosition GPSDO</name>");
   }
   else if(rcvr_type == TSERVE_RCVR) {
      sprintf(log_text, "  <name>TymServe 2000</name>");
   }
   else if(rcvr_type == TSIP_RCVR) {
      if(tsip_type == STARLOC_TYPE)  sprintf(log_text, "  <name>STARLOC GPSDO</name>");
      else                           sprintf(log_text, "  <name>TSIP receiver</name>");
   }
   else if(rcvr_type == UBX_RCVR)  {
      if(saw_timing_msg)             sprintf(log_text, "  <name>Ublox timing receiver</name>");
      else                           sprintf(log_text, "  <name>Ublox receiver</name>");
   }
   else if(rcvr_type == UCCM_RCVR) {
      if(scpi_type == UCCMP_TYPE)    sprintf(log_text, "  <name>Symmetricom UCCM GPSDO</name>");
      else                           sprintf(log_text, "  <name>Trimble UCCM GPSDO</name>");
   }
   else if(rcvr_type == VENUS_RCVR) {
       if(lte_lite)                  sprintf(log_text, "  <name>Jackson Labs LTE</name>");
       else if(saw_venus_raw)        sprintf(log_text, "  <name>Venus RTK receiver</name>");
       else if(saw_timing_msg)       sprintf(log_text, "  <name>Venus timing receiver</name>");
       else                          sprintf(log_text, "  <name>Venus receiver</name>");
   }
   else if(rcvr_type == X72_RCVR)    sprintf(log_text, "  <name>X72 rubidium</name>");
   else if(rcvr_type == Z12_RCVR)    sprintf(log_text, "  <name>Ashtech Z12</name>");
   else if(rcvr_type == ZODIAC_RCVR) sprintf(log_text, "  <name>Zodiac receiver</name>");
   else if(rcvr_type == ZYFER_RCVR) {
      sprintf(log_text, "  <name>Zyfer Nanosync 380 GPSDO</name>");
   }
   else                              sprintf(log_text, "  <name>Unknown receiver type</name>");

   if(log_fmt == HEATHER) {
      s = strstr(log_text, "</name>");
      if(s) *s = 0;

      s = strstr(log_text, "<name>");
      if(s) {
         sprintf(out, "   Device: %s", &s[6]);
         strcpy(log_text, out);
      }

      fprintf(log_file, "#");
      write_log_comment(1);
   }
   else if(log_fmt == RINEX) {
   }
   else {
      fprintf(log_file, "%s\n", log_text);
   }
   if(log_flush_mode && log_file) fflush(log_file);
}


FILE *open_log_file(char *mode)
{
u32 color;
int i;
char *s;

   if(log_file) return 0;  // log already open
   if(mode == 0) return 0;

   s = strchr(log_name, FLUSH_CHAR);
   if(s) {
      *s = 0;
      log_flush_mode = 1;
   }
   else log_flush_mode = 0;

   log_file = topen(log_name, mode);
   if(log_file == 0) return 0;
   if(debug_file) fprintf(debug_file, "! log file %s opened\n", log_name);

   if(log_header) log_written = 0;
   log_loaded = 0;
   show_log_state();
   log_file_time = (log_interval+1);

   if     (strstr(log_name, ".xml")) log_fmt = XML;
   else if(strstr(log_name, ".XML")) log_fmt = XML;
   else if(strstr(log_name, ".gpx")) log_fmt = GPX;
   else if(strstr(log_name, ".GPX")) log_fmt = GPX;
   else if(strstr(log_name, ".kml")) log_fmt = KML;
   else if(strstr(log_name, ".KML")) log_fmt = KML;
   else if(strstr(log_name, ".obs")) log_fmt = RINEX;
   else if(strstr(log_name, ".OBS")) log_fmt = RINEX;
   else log_fmt = HEATHER;

   if(log_fmt != HEATHER) {
      last_log_hours = hours;
      if(log_fmt == GPX) {
         fprintf(log_file, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n\n");
         fprintf(log_file, "<gpx version=\"1.0\"  creator=\"Lady Heather V%s - %s\" >\n", VERSION, date_string);
         log_rcvr_type();
         start_log_track();
      }
      else if(log_fmt == KML) {
         fprintf(log_file, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");
         fprintf(log_file, "<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n");
         fprintf(log_file, "\n"); 
         fprintf(log_file, "  <Document>\n");
         fprintf(log_file, "  "); log_rcvr_type();
         fprintf(log_file, "    <visibility>1</visibility>\n"); 
         fprintf(log_file, "    <open>1</open>\n"); 
         fprintf(log_file, "\n"); 

         for(i=0; i<16; i++) {
            color = get_bgr_palette(i);
            color = (color & 0x00FF00) | ((color & 0xFF) << 16) | ((color & 0xFF0000) >> 16); 
            color |= 0x64000000;

            fprintf(log_file, "    <Style id=\"color%d\">\n", i);
            fprintf(log_file, "      <LineStyle>\n");
            fprintf(log_file, "        <color>%08X</color>\n", color);  // opacity(0..100) BB GG RR
            fprintf(log_file, "        <width>1</width>\n");
            fprintf(log_file, "      </LineStyle>\n");
            fprintf(log_file, "    </Style>\n");
            fprintf(log_file, "\n");
         }
         start_log_track();
      }
      else if(log_fmt == RINEX) {
      }
      else if(log_fmt == XML) {
         fprintf(log_file, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n\n");
         fprintf(log_file, "<gpx version=\"1.1\"  creator=\"Lady Heather V%s - %s\" >\n", VERSION, date_string);
         log_rcvr_type();
         start_log_track();
      }
   }
   if(log_flush_mode && log_file) fflush(log_file);

   return log_file;
}

void close_log_file()
{
   if(log_file) {
      if(log_fmt == GPX) {
         end_log_track();
         fprintf(log_file, "</gpx>\n");
         gpx_track_number = 0;
      }
      else if(log_fmt == KML) {
         end_log_track();

         if(1) {
            fprintf(log_file, "\n");
            fprintf(log_file, "    <LookAt>\n");
            fprintf(log_file, "      <longitude>%.9f</longitude>\n", lon*180.0/PI);            
            fprintf(log_file, "      <latitude>%.9f</latitude>\n", lat*180.0/PI);             
            fprintf(log_file, "      <altitude>%.2f</altitude>\n", alt);               
            fprintf(log_file, "      <altitudeMode>absolute</altitudeMode>\n");
//               <heading>0</heading>               
//               <tilt>45</tilt>
//               <range>175</range>                    
            fprintf(log_file, "    </LookAt>\n");
         }

         fprintf(log_file, "\n");
         fprintf(log_file, "  </Document>\n");
         fprintf(log_file, "</kml>\n");
         gpx_track_number = 0;
      }
      else if(log_fmt == RINEX) {
         rinex_header_written = 0;
         rinex_obs_written = 0;
      }
      else if(log_fmt == XML) {
         end_log_track();
         fprintf(log_file, "</gpx>\n");
         gpx_track_number = 0;
      }

      if(log_fmt != RINEX) {
         fprintf(log_file, "\n");
         fprintf(log_file, "\n");
      }

      fclose(log_file);
   }

   log_file = 0;
   if(log_header) log_written = 0;
   show_log_state();
}

void sync_log_file()
{
   if(log_file == 0) return;

   if(log_fmt != HEATHER) {
      end_log_track();
      sync_file(log_file);
      start_log_track();
   }
   else {
      sync_file(log_file);
   }
}

int filter_log_text()
{
char *s;

   // remove potentially corrupting chars from log_text lines
   // returns 0 if resulting line is blank, 1 otherwise

   s = strchr(log_text, '!');
   while(s) {
      *s = ' ';
      s = strchr(s+1, '!');
   }

   s = strchr(log_text, '#');
   while(s) {
      *s = ' ';
      s = strchr(s+1, '#');
   }

   if(log_text[0] == 0) return 0;  // empty line
   if((log_text[0] == ' ') && (log_text[1] == 0)) return 0;  // blank line
   return 1;
}


void write_rinex_line(FILE *file, char *s, char *type)
{
char buf[SLEN+1];

   // output a 60:20 character line to a RINEX file

   if(file == 0) return;
   if(s == 0) return;
   if(type == 0) return;

   sprintf(buf, "%-60.60s%-20.20s", s,type);
   buf[80] = 0;
   fprintf(file, "%s\n", buf);
}

void write_log_comment(int spaces)
{
   if(log_file == 0) return;
   if(log_comments == 0) return;

   if(log_fmt == HEATHER) {
      fprintf(log_file, "%s", log_text);
      while(spaces--) fprintf(log_file, "\n");
   }
   else if(log_fmt == KML) {
      if(filter_log_text()) {
         fprintf(log_file, "<!-- %s -->", log_text);
      }
      while(spaces--) fprintf(log_file, "\n");
   }
   else if(log_fmt == RINEX) {  // don't output anything until the RINEX header has been output
      if(rinex_header_written) {
         write_rinex_line(log_file, log_text, "COMMENT");
      }
   }
   else if(log_fmt == XML) {  // !!!! does this work when sending to a GPX viewer?
      if(filter_log_text()) {
         fprintf(log_file, "<!-- %s -->", log_text);
      }
      else {  // blank line
//       fprintf(log_file, "<!--  -->");
      }

      while(spaces--) {
         fprintf(log_file, "\n");
      }
   }

   if(debug_file) {
      if(log_text[1] == '!') fprintf(debug_file, "Event: %s\n", log_text);
      fflush(debug_file);
   }

   if(log_flush_mode && log_file) fflush(log_file);
}

void log_enviro_info()
{
   strcpy(log_text, "#");
   write_log_comment(1);

   if(enviro_type == HEATHER_ENVIRO) {
      sprintf(log_text, "#   Enviro type:   Heather Environmental Sensor");
   }
   else if(enviro_type == DOG_ENVIRO) {
      if(enviro_dev[0]) sprintf(log_text, "#   Enviro type:   %s", enviro_dev);
      else              sprintf(log_text, "#   Envito type:   Dogratian USB");
   }
   else if(enviro_type == LFS_ENVIRO) {
      if(enviro_dev[0]) sprintf(log_text, "#   Enviro type:   %s", enviro_dev);
      else              sprintf(log_text, "#   Enviro type:   LFS10xx");
   }
   else {
      sprintf(log_text, "#   Enviro type:   Environmental sensor");
   }
   write_log_comment(1);

   if(enviro_sn[0]) {
      sprintf(log_text, "#   Enviro serial: %s", enviro_sn);
      write_log_comment(1);
   }

   strcpy(log_text, "#");
   write_log_comment(1);
}

void log_ticc_info()
{
   strcpy(log_text, "#");
   write_log_comment(1);

   if(ticc_type == TAPR_TICC) {
      sprintf(log_text, "#   Unit type:     TAPR TICC");
      write_log_comment(1);
      sprintf(log_text, "#   Serial number: %s", ticc_sn);
      write_log_comment(1);
      sprintf(log_text, "#   Firmware ver:  %s", ticc_fw);
      write_log_comment(1);
      sprintf(log_text, "#   Board ver:     %s", ticc_board);
      write_log_comment(1);
      sprintf(log_text, "#   EEPROM ver:    %d", ticc_eeprom);
      write_log_comment(1);
      sprintf(log_text, "#");
      write_log_comment(1);
   }
   else if(ticc_type == HP_TICC) {
      sprintf(log_text, "#   Unit type:     HP-53xxx interval counter");
      write_log_comment(1);
   }
   else if(ticc_type == PICPET_TICC) {
      sprintf(log_text, "#   Unit type:     PICPET timestamp  counter");
      write_log_comment(1);
   }
   else if(ticc_type == LARS_TICC) {
      sprintf(log_text, "#   Unit type:     LARS GPSDO TIC");
      write_log_comment(1);
   }
   else {
      sprintf(log_text, "#   Unit type:     Generic interval counter");
      write_log_comment(1);
   }

   if(have_ticc_mode) {
      if     (ticc_mode == 'F') sprintf(log_text, "#   Mode:          Frequency");
      else if(ticc_mode == 'H') sprintf(log_text, "#   Mode:          Phase");
      else if(ticc_mode == 'I') sprintf(log_text, "#   Mode:          Interval");
      else if(ticc_mode == 'P') sprintf(log_text, "#   Mode:          Period");
      else if(ticc_mode == 'T') sprintf(log_text, "#   Mode:          Timestamp");
      else if(ticc_mode == 'D') sprintf(log_text, "#   Mode:          Debug");
      else if(ticc_mode == 'L') sprintf(log_text, "#   Mode:          Timelab");
      else                      sprintf(log_text, "#   Mode:          Unknown: %c", ticc_mode);
      write_log_comment(1);
   }
   if(have_ticc_syncmode) {
      sprintf(log_text, "#   Sync mode:     %c", ticc_syncmode);
      write_log_comment(1);
   }
   if(have_ticc_cal) {
      sprintf(log_text, "#   Cal periods:   %d", ticc_cal);
      write_log_comment(1);
   }
   if(have_ticc_timeout) {
      sprintf(log_text, "#   Timeout:       %d", ticc_timeout);
      write_log_comment(1);
   }
   if(have_ticc_speed) {
      sprintf(log_text, "#   Clock speed:   %f MHz", ticc_speed);
      write_log_comment(1);
   }
   if(have_ticc_coarse) {
      sprintf(log_text, "#   Coarse speed:  %f usec", ticc_coarse);
      write_log_comment(1);
   }

   if(have_ticc_edges & 0x01) {
      sprintf(log_text, "#   chA edge:      %c", edge_a);
      write_log_comment(1);
   }
   if(have_ticc_edges & 0x02) {
      sprintf(log_text, "#   chB edge:      %c", edge_b);
      write_log_comment(1);
   }
   if(have_ticc_edges & 0x04) {
      sprintf(log_text, "#   chC edge:      %c", edge_c);
      write_log_comment(1);
   }
   if(have_ticc_edges & 0x08) {
      sprintf(log_text, "#   chD edge:      %c", edge_d);
      write_log_comment(1);
   }

   if(have_ticc_fudge & 0x01) {
      sprintf(log_text, "#   chA Fudge:     %g ps", fudge_a);
      write_log_comment(1);
   }
   if(have_ticc_fudge & 0x02) {
      sprintf(log_text, "#   chB Fudge:     %g ps", fudge_b);
      write_log_comment(1);
   }
   if(have_ticc_fudge & 0x04) {
      sprintf(log_text, "#   chC Fudge:     %g ps", fudge_c);
      write_log_comment(1);
   }
   if(have_ticc_fudge & 0x08) {
      sprintf(log_text, "#   chD Fudge:     %g ps", fudge_d);
      write_log_comment(1);
   }

   if(have_ticc_time2 & 0x01) {
      sprintf(log_text, "#   chA TIME2:     %d", time2_a);
      write_log_comment(1);
   }
   if(have_ticc_time2 & 0x02) {
      sprintf(log_text, "#   chB TIME2:     %d", time2_b);
      write_log_comment(1);
   }
   if(have_ticc_time2 & 0x04) {
      sprintf(log_text, "#   chC TIME2:     %d", time2_c);
      write_log_comment(1);
   }
   if(have_ticc_time2 & 0x08) {
      sprintf(log_text, "#   chD TIME2:     %d", time2_d);
      write_log_comment(1);
   }

   if(have_ticc_dilat & 0x01) {
      sprintf(log_text, "#   chA Dilation   %d", dilat_a);
      write_log_comment(1);
   }
   if(have_ticc_dilat & 0x02) {
      sprintf(log_text, "#   chB Dilation   %d", dilat_b);
      write_log_comment(1);
   }
   if(have_ticc_dilat & 0x04) {
      sprintf(log_text, "#   chC Dilation   %d", dilat_c);
      write_log_comment(1);
   }
   if(have_ticc_dilat & 0x08) {
      sprintf(log_text, "#   chD Dilation   %d", dilat_d);
      write_log_comment(1);
   }

   strcpy(log_text, "#");
   write_log_comment(1);
   strcpy(log_text, "#");
   write_log_comment(1);
}


void write_log_tsip_vers()
{
   sprintf(log_text, "#   Serial number: %u.%lu", sn_prefix, (unsigned long) serial_num);
   write_log_comment(1);
   sprintf(log_text, "#   Case s/n:      %u.%lu", case_prefix, (unsigned long) case_sn);
   write_log_comment(1);
   sprintf(log_text, "#   Prodn number:  %lu.%u", (unsigned long) prodn_num, prodn_extn);
   write_log_comment(1);
   sprintf(log_text, "#   Prodn options: %u", prodn_options);        // from prodn_params message
   write_log_comment(1);
   sprintf(log_text, "#   Machine id:    %u", machine_id);
   write_log_comment(1);
   if(ebolt) {
      sprintf(log_text, "#   Hardware code: %u", hw_code);
      write_log_comment(1);
   }

   sprintf(log_text, "#   App firmware:  %2d.%-2d  %02d %s %02d", 
      ap_major, ap_minor,  ap_day, months[ap_month], ap_year);   //!!! docs say 1900
   write_log_comment(1);
   sprintf(log_text, "#   GPS firmware:  %2d.%-2d  %02d %s %02d", 
      core_major, core_minor,  core_day, months[core_month], core_year);  //!!! docs say 1900
   write_log_comment(1);
   sprintf(log_text, "#   Mfg time:      %02d:00  %02d %s %04d", 
      build_hour, build_day, months[build_month], build_year);
   write_log_comment(1);
}

void write_log_id()
{
int i;

   // write the receiver ID info to the log file
   // NEW_RCVR

   if((rcvr_type != THERMO_RCVR) && (enviro_sn[0] || enviro_dev[0])){
      log_enviro_info();
   }

   if((rcvr_type != TICC_RCVR) && ticc_port_open()) {
      log_ticc_info();
   }

   if(unit_name[0] && (rcvr_type != SCPI_RCVR) && (rcvr_type != UCCM_RCVR) && (rcvr_type != CS_RCVR)) {
      sprintf(log_text, "#   Unit type:     Trimble %s", unit_name);
      write_log_comment(1);
      write_log_tsip_vers();
   }
   else if(luxor) {
      sprintf(log_text, "#   Unit type:     Luxor");
      write_log_comment(1);
      sprintf(log_text, "#   HW version:    %u.%lu", (unsigned)luxor_hw_ver, (unsigned long)luxor_hw_rev);
      write_log_comment(1);
      sprintf(log_text, "#   Serial number: %u.%lu", (unsigned)luxor_sn_prefix, (unsigned long)luxor_serial_num);
      write_log_comment(1);
   }
   else if(rcvr_type == ACRON_RCVR) {
      sprintf(log_text, "#   Unit type:     Acron Zeit");
      write_log_comment(1);
   }
   else if(rcvr_type == BRANDY_RCVR) {
      sprintf(log_text, "#   Unit type:     Brandywine GPS-4");
      write_log_comment(1);
      sprintf(log_text, "#   Firmware:      %s", brandy_fw);
      write_log_comment(1);
      sprintf(log_text, "#   Engine:        %s", brandy_engine);
      write_log_comment(1);
      sprintf(log_text, "#   Test:          %s", brandy_test);
      write_log_comment(1);
      sprintf(log_text, "#   Osc type:      %s", brandy_osc);
      write_log_comment(1);
   }
   else if(rcvr_type == ESIP_RCVR) {
      sprintf(log_text, "#   Unit type:     Furuno eSIP GPS receiver");
      write_log_comment(1);
      sprintf(log_text, "#   Device:        %s", esip_device);
      write_log_comment(1);
      sprintf(log_text, "#   Version:       %s", esip_version);
      write_log_comment(1);
      sprintf(log_text, "#   Query:         %s", esip_query);
      write_log_comment(1);
      sprintf(log_text, "#   Model:         %s", esip_model);
      write_log_comment(1);
   }
   else if(rcvr_type == FURUNO_RCVR) {
      sprintf(log_text, "#   Unit type:     Furuno PFEC GPS receiver");
      write_log_comment(1);
      sprintf(log_text, "#   Pgm:           %d", furuno_pgm);
      write_log_comment(1);
      sprintf(log_text, "#   Ver:           %d", furuno_ver);
      write_log_comment(1);
   }
   else if(rcvr_type == GPSD_RCVR) {
      sprintf(log_text, "#   Unit type:     GPSD %d.%d interfaced receiver", gpsd_major, gpsd_minor);
      write_log_comment(1);
      sprintf(log_text, "#   Driver:        %s", gpsd_driver);
      write_log_comment(1);
      sprintf(log_text, "#   Release:       %s", gpsd_release);
      write_log_comment(1);
   }
   else if(rcvr_type == LPFRS_RCVR) {
      sprintf(log_text, "#   Unit type:     LPFRS rubidium oscillator");
      write_log_comment(1);
      sprintf(log_text, "#   Hardware:      %s", lpfrs_id);
      write_log_comment(1);
      sprintf(log_text, "#   Version:       %s", lpfrs_fw);
      write_log_comment(1);
      sprintf(log_text, "#   Checksum:      %s", lpfrs_ck);
      write_log_comment(1);
   }
   else if(rcvr_type == MOTO_RCVR) {
      sprintf(log_text, "#   Unit type:     Motorola %d channel GPS receiver", moto_chans);
      write_log_comment(1);
      for(i=1; i<=10; i++) {
         if(moto_id[i][0]) {
            sprintf(log_text, "#                  %s", &moto_id[i][0]);
            write_log_comment(1);
         }
      }
   }
   else if(rcvr_type == RT17_RCVR) {
      sprintf(log_text, "#   Unit type:     Trimble RT17 receiver");
      write_log_comment(1);
   }
   else if(rcvr_type == NMEA_RCVR) {
      sprintf(log_text, "#   Unit type:     NMEA GPS receiver");
      write_log_comment(1);
   }
   else if(rcvr_type == NO_RCVR) {
      sprintf(log_text, "#   Unit type:     System clock");
      write_log_comment(1);
   }
   else if(rcvr_type == NVS_RCVR) {
      sprintf(log_text, "#   Unit type:     NVS receiver - %d channels", nvs_chans);
      write_log_comment(1);

      if(nvs_sn || nvs_id[0]) {
         sprintf(log_text, "#   Firmware:      %s", nvs_id);
         write_log_comment(1);
         sprintf(log_text, "#   Serial number: %u (%08X)", nvs_sn, nvs_sn);
         write_log_comment(1);
      }

      if(nvs_sn2 || nvs_id2[0]) {
         sprintf(log_text, "#   Info 2:        %s", nvs_id2);
         write_log_comment(1);
         sprintf(log_text, "#   Number 2:      %u (%08X)", nvs_sn2, nvs_sn2);
         write_log_comment(1);
      }

      if(nvs_sn3 || nvs_id3[0]) {
         sprintf(log_text, "#   Info 3:        %s", nvs_id3);
         write_log_comment(1);
         sprintf(log_text, "#   Number 3:      %u (%08X)", nvs_sn3, nvs_sn3);
         write_log_comment(1);
      }
   }
   else if(rcvr_type == PRS_RCVR) {
      sprintf(log_text, "#   Unit type:     PRS-10 rubidium oscillator");
      write_log_comment(1);
      sprintf(log_text, "#   FW version:    %s", prs_fw);
      write_log_comment(1);
      sprintf(log_text, "#   Serial number: %s", prs_sn);
      write_log_comment(1);
   }
   else if(rcvr_type == RFTG_RCVR) {
      sprintf(log_text, "#   Unit type:     System clock");
      if((have_rftg_unit) && (rftg_unit == 0))      sprintf(log_text, "#   Unit type:     RFTG-m RB");
      else if((have_rftg_unit) && (rftg_unit == 1)) sprintf(log_text, "#   Unit type:     RFTG-m XO");
      else sprintf(log_text, "#   Unit type:     RFTG-m");
      write_log_comment(1);
      sprintf(log_text,"#   Firmware:      %s", rftg_fw);
      write_log_comment(1);
      sprintf(log_text,"#   HW version:    %d", rftg_hw);
      write_log_comment(1);
   }
   else if(rcvr_type == SA35_RCVR) {
      sprintf(log_text, "#   Unit type:     SA.3xM rubidium oscillator");
      if(have_sa35_telem) {
         sprintf(log_text, "#   Firmware:      %s", sa35_fw);
         write_log_comment(1);
         sprintf(log_text, "#   Date:          %s", sa35_date);
         write_log_comment(1);
         sprintf(log_text, "#   Model:         %s", sa35_model);
         write_log_comment(1);
         sprintf(log_text, "#   Serial number: %s", sa35_sn);
         write_log_comment(1);
      }
      write_log_comment(1);
   }
   else if((rcvr_type == SCPI_RCVR) || (rcvr_type == UCCM_RCVR) || (rcvr_type == CS_RCVR)) {
      sprintf(log_text, "#   Unit type:     %s", scpi_model);
      write_log_comment(1);
      sprintf(log_text, "#   Manufacturer:  %s", scpi_mfg);
      write_log_comment(1);
      sprintf(log_text, "#   Serial number: %s", scpi_sn);
      write_log_comment(1);
      sprintf(log_text, "#   Firmware:      %s", scpi_fw);
      write_log_comment(1);
//      if((rcvr_type == CS_RCVR) && cs_cbtid[0]) {
      if(rcvr_type == CS_RCVR) {
         sprintf(log_text, "#   Tube:          %s", cs_cbtid);
         write_log_comment(1);
      }
   }
   else if(rcvr_type == SIRF_RCVR) {
      sprintf(log_text, "#   Unit type:     SIRF GPS receiver");
      write_log_comment(1);
      sprintf(log_text, "#   SW:            %s", sirf_sw_id);
      write_log_comment(1);
   }
   else if(rcvr_type == SRO_RCVR) {
      sprintf(log_text, "#   Unit type:     SRO100 rubidium oscillator");
      write_log_comment(1);
      sprintf(log_text, "#   Hardware:      %s", sro_id);
      write_log_comment(1);
      sprintf(log_text, "#   Serial number: %s", sro_sn);
      write_log_comment(1);
   }
   else if(rcvr_type == SS_RCVR) {
      sprintf(log_text, "#   Unit type:     Novatel SuperStar II");
      sprintf(log_text, "#   Software:       %s", ss_sw_pn);
      sprintf(log_text, "#   Model:          %s", ss_model);
      sprintf(log_text, "#   Type::          %d", ss_type);
      sprintf(log_text, "#   Boot:           %s", ss_boot_pn);
      sprintf(log_text, "#   Serial number:  %s", ss_psn);
      sprintf(log_text, "#   Reserved 1:     %s", ss_rsvd);
      sprintf(log_text, "#   Reserved 2:     %s", ss_rsvd2);
      sprintf(log_text, "#   Model checksum: %s", ss_model_cksum);
      sprintf(log_text, "#   Boot checksum:  %04X", ss_boot_cksum);
      sprintf(log_text, "#   Op checksum:    %04X", ss_op_cksum);
      write_log_comment(1);
   }
   else if(rcvr_type == STAR_RCVR) {
      if(star_type == NEC_TYPE)      sprintf(log_text, "#   Unit type:     NEC GPSDO");
      else if(star_type == OSA_TYPE) sprintf(log_text, "#   Unit type:     OSA-453x GPSDO");
      else                           sprintf(log_text, "#   Unit type:     STAR-4 GPSDO");
      write_log_comment(1);

      sprintf(log_text, "#   Module:        %s", star_module);
      write_log_comment(1);

      sprintf(log_text, "#   Article:       %s", star_article);
      write_log_comment(1);

      sprintf(log_text, "#   SN:            %s", star_sn);
      write_log_comment(1);

      if(star_type != OSA_TYPE) {
         sprintf(log_text, "#   HW version:    %s", star_hw_ver);
         write_log_comment(1);

         sprintf(log_text, "#   FW article:    %s", star_fw_article);
         write_log_comment(1);
      }

      sprintf(log_text, "#   FW:            %s", star_fw);
      write_log_comment(1);

      sprintf(log_text, "#   Test date:     %s", star_test_date);
      write_log_comment(1);

      sprintf(log_text, "#   Test version:  %s", star_test_version);
      write_log_comment(1);

      sprintf(log_text, "#   Osc type:      %s", star_osc);
      write_log_comment(1);

      if(star_type == OSA_TYPE) {
         sprintf(log_text, "#   Layout:        %s", star_layout);
         write_log_comment(1);
         sprintf(log_text, "#   Commercial:    %s", star_commercial);
         write_log_comment(1);
      }
      else {
         sprintf(log_text, "#   FPGA:          %s", star_fpga);
         write_log_comment(1);
         sprintf(log_text, "#   Family:        %s", star_family);
         write_log_comment(1);
         sprintf(log_text, "#   Variant:       %s", star_variant);
         write_log_comment(1);
      }
   }
   else if(rcvr_type == TAIP_RCVR) {
      sprintf(log_text, "#   Product:       %s", taip_product);
      write_log_comment(1);
      sprintf(log_text, "#   Version:       %s", taip_version);
      write_log_comment(1);
      sprintf(log_text, "#   Core:          %s", taip_core);
      write_log_comment(1);
      sprintf(log_text, "#   Copyright:     %s", taip_copr);
      write_log_comment(1);
      sprintf(log_text, "#   Unit id:       %04d", taip_id);
      write_log_comment(1);
   }
   else if(rcvr_type == THERMO_RCVR) {
      log_enviro_info();
   }
   else if(rcvr_type == TICC_RCVR) {
      log_ticc_info();
   }
   else if(rcvr_type == TIDE_RCVR) {
      sprintf(log_text, "#   Unit type:     Gravity clock");
      write_log_comment(1);
   }
   else if(rcvr_type == TM4_RCVR) {
      sprintf(log_text, "#   Unit type:     Spectrum TM4 GPSDO");
      write_log_comment(1);
      sprintf(log_text, "#   Version:       %s", tm4_id);
      write_log_comment(1);
      sprintf(log_text, "#   HEX file:      %s", tm4_hex);
      write_log_comment(1);
      sprintf(log_text, "#   OBJ file:      %s", tm4_obj);
      write_log_comment(1);
      sprintf(log_text, "#   String:        %s", tm4_string);
      write_log_comment(1);
   }
   else if(rcvr_type == TRUE_RCVR) {
      sprintf(log_text, "#   Unit type:     TruePosition GPSDO");
      write_log_comment(1);
      sprintf(log_text, "#   SW:            %s", tp_sw);
      write_log_comment(1);
      sprintf(log_text, "#   GPS Engine:    %s", tp_boot);
      write_log_comment(1);
      sprintf(log_text, "#   FPGA VERSION:  %s", tp_num1);
      write_log_comment(1);
      sprintf(log_text, "#   SW CKSUM:      %s", tp_num2);
      write_log_comment(1);
      sprintf(log_text, "#   FPGA CKSUM:    %s", tp_num3);
      write_log_comment(1);
      sprintf(log_text, "#   SN:            %s", tp_sn);
      write_log_comment(1);
   }
   else if(rcvr_type == TSERVE_RCVR) {
      sprintf(log_text, "#   Unit type:     TymServe 2000");
      write_log_comment(1);
   }
   else if(rcvr_type == UBX_RCVR) {
      if(saw_timing_msg) sprintf(log_text, "#   Unit type:     Ublox GPS receiver");
      else               sprintf(log_text, "#   Unit type:     Ublox timing receiver");
      write_log_comment(1);

      sprintf(log_text, "#   SW:            %s", ubx_sw);
      write_log_comment(1);
      sprintf(log_text, "#   HW:            %s", ubx_hw);
      write_log_comment(1);
      for(i=0; i<UBX_ROM_INFO; i++) {
         if(ubx_rom[i][0]) {
            if(i > 9) sprintf(log_text, "#   ROM %d:        %s", i+1, &ubx_rom[i][0]);
            else      sprintf(log_text, "#   ROM %d:         %s", i+1, &ubx_rom[i][0]);
            write_log_comment(1);
         }
      }
   }
   else if(rcvr_type == VENUS_RCVR) {
      if(lte_lite)            sprintf(log_text, "#   Unit type:     Jackson Labs LTE GPSDO");
      else if(saw_venus_raw)  sprintf(log_text, "#   Unit type:     Venus RTK receiver");
      else if(saw_timing_msg) sprintf(log_text, "#   Unit type:     Venus timing receiver");
      else                    sprintf(log_text, "#   Unit type:     Venus GPS receiver");
      write_log_comment(1);
      sprintf(log_text, "#   Kernel:        %s", venus_kern);
      write_log_comment(1);
      sprintf(log_text, "#   ODM:           %s", venus_odm);
      write_log_comment(1);
      sprintf(log_text, "#   Rev:           %s", venus_rev);
      write_log_comment(1);
   }
   else if(rcvr_type == X72_RCVR) {
      if(x72_type == SA22_TYPE) {
         sprintf(log_text, "#   Unit type:         SA22 rubidium oscillator");
      }
      else if(x72_type == X99_TYPE) {
         sprintf(log_text, "#   Unit type:         X99 rubidium oscillator");
      }
      else {
         sprintf(log_text, "#   Unit type:         X72 rubidium oscillator");
      }
      write_log_comment(1);

      sprintf(log_text, "#   FW version:        %.2f", x72_fw);
      write_log_comment(1);
      sprintf(log_text, "#   Date:              %s", x72_date);
      write_log_comment(1);
      sprintf(log_text, "#   Loader version:    %d", x72_loader);
      write_log_comment(1);
      sprintf(log_text, "#   Serial number:     %s", x72_serial);
      write_log_comment(1);
      sprintf(log_text, "#   Tuning state:      %d", x72_tune);
      write_log_comment(1);
      sprintf(log_text, "#   Osc freq:          %.1f Hz", x72_osc);
      write_log_comment(1);
      sprintf(log_text, "#   Crystal freq:      %g Hz", x72_crystal);
      write_log_comment(1);
      sprintf(log_text, "#   ACMOS freq:        %g Hz", x72_acmos);
      write_log_comment(1);
      sprintf(log_text, "#   Sine freq:         %g Hz", x72_sine);
      write_log_comment(1);
      sprintf(log_text, "#   Res temp offset:   %g", x72_res_tempofs);
      write_log_comment(1);
      sprintf(log_text, "#   Lamp temp offset:  %g", x72_lamp_tempofs);
      write_log_comment(1);
   }
   else if(rcvr_type == Z12_RCVR) {
      sprintf(log_text, "#   Unit type:     Ashtech Z12 receiver");
      write_log_comment(1);
      sprintf(log_text, "#   Receiver type:   %s", z12_type);
      write_log_comment(1);
      sprintf(log_text, "#   Channel option:  %s", z12_chan_opt);
      write_log_comment(1);
      sprintf(log_text, "#   Nav version:     %s", z12_nav_ver);
      write_log_comment(1);
      sprintf(log_text, "#   Reserved:        %s", z12_rsvd);
      write_log_comment(1);
      sprintf(log_text, "#   Channel version: %s", z12_chan_ver);
      write_log_comment(1);
      if(z12_site[0]) {
         sprintf(log_text, "#   Site name:       %s", z12_site);
         write_log_comment(1);
      }
   }
   else if(rcvr_type == ZODIAC_RCVR) {
      sprintf(log_text, "#   Unit type:     Zodiac GPS receiver");
      write_log_comment(1);
      sprintf(log_text, "#   Chans:         %s", zod_chans);
      write_log_comment(1);
      sprintf(log_text, "#   SW:            %s", zod_sw);
      write_log_comment(1);
      sprintf(log_text, "#   Date:          %s", zod_date);
      write_log_comment(1);
      sprintf(log_text, "#   Opt            %s", zod_opt);
      write_log_comment(1);
      sprintf(log_text, "#   Rsvd:          %s", zod_rsvd);
      write_log_comment(1);
   }
   else if(rcvr_type == ZYFER_RCVR) {
      sprintf(log_text, "#   Unit type:     Zyfer Nanosync");
      write_log_comment(1);
      sprintf(log_text, "#   ID:            %s", zy_id);
      write_log_comment(1);
      sprintf(log_text, "#   SN:            %s", zy_sn);
      write_log_comment(1);
      sprintf(log_text, "#   PN:            %s", zy_pn);
      write_log_comment(1);
      sprintf(log_text, "#   FW:            %s", zy_fw);
      write_log_comment(1);
      sprintf(log_text, "#   Mfg:           %s", zy_mfgid);
      write_log_comment(1);
      sprintf(log_text, "#   Rev:           %s", zy_rev);
      write_log_comment(1);
      sprintf(log_text, "#   GPS:           %s", zy_gps);
      write_log_comment(1);
      sprintf(log_text, "#   Prodn:         %s", zy_prod);
      write_log_comment(1);
      sprintf(log_text, "#   Date:          %s", zy_date);
      write_log_comment(1);
      sprintf(log_text, "#   App version:   %s", zy_app_vers);
      write_log_comment(1);
      sprintf(log_text, "#   App prog:      %s", zy_app_prog);
      write_log_comment(1);
      sprintf(log_text, "#   App date:      %s", zy_app_date);
      write_log_comment(1);
      sprintf(log_text, "#   Option:        %s", zy_option);
      write_log_comment(1);
      sprintf(log_text, "#   OSC type:      %s", zy_osc_type);
      write_log_comment(1);
   }
   //
   //   Trimble receivers follow:
   //
   else if(saw_mini) {
      sprintf(log_text, "#   Unit type:     Trimble Mini-T");
      write_log_comment(1);
      write_log_tsip_vers();
   }
   else if(saw_ntpx) {
      sprintf(log_text, "#   Unit type:     Nortel NTPX");
      write_log_comment(1);
      write_log_tsip_vers();
   }
   else if(saw_nortel) {
      sprintf(log_text, "#   Unit type:     Nortel NTGS/NTBW");
      write_log_comment(1);
      write_log_tsip_vers();
   }
   else if(res_t) {
      sprintf(log_text, "#   Unit type:     Unknown Resolution-T type receiver");
      write_log_comment(1);
      write_log_tsip_vers();
   }
   else if((rcvr_type == TSIP_RCVR) && (tsip_type == STARLOC_TYPE)) {
      sprintf(log_text, "#   Unit type:     Datum Starloc");
      write_log_comment(1);
      write_log_tsip_vers();
   }
   else if(ACE3) {
      sprintf(log_text, "#   Unit type:     Trimble ACE-III");
      write_log_comment(1);
      write_log_tsip_vers();
   }
   else if(ACU_GG) {
      sprintf(log_text, "#   Unit type:     Trimble Acutime GG");
      write_log_comment(1);
      write_log_tsip_vers();
   }
   else if(ACU_360) {
      sprintf(log_text, "#   Unit type:     Trimble Acutime 360");
      write_log_comment(1);
      write_log_tsip_vers();
   }
   else if(ACUTIME) {
      sprintf(log_text, "#   Unit type:     Trimble Acutime");
      write_log_comment(1);
      write_log_tsip_vers();
   }
   else if(PALISADE) {
      sprintf(log_text, "#   Unit type:     Trimble Palisade");
      write_log_comment(1);
      write_log_tsip_vers();
   }
   else if(SV6_FAMILY) {
      sprintf(log_text, "#   Unit type:     Trimble SV6/SV8");
      write_log_comment(1);
      write_log_tsip_vers();
   }
   else if(rcvr_type == TSIP_RCVR) {
      sprintf(log_text, "#   Unit type:     Trimble Thunderbolt%s", ebolt?"-E":"");
      write_log_comment(1);
      write_log_tsip_vers();
   }
   else  {
      sprintf(log_text, "#   Unit type:     Unknown");
      write_log_comment(1);
   }
}

void write_log_header(FILE *file, int why)
{
FILE *old_file;

   // write the log file header comments

   if(file == 0) return;
   if(log_comments == 0) return;

   old_file = log_file;
   log_file = file;

   sprintf(log_text, "#");
   write_log_comment(1);

   if(log_fmt == HEATHER) {
      log_rcvr_type();
   }
   else if(log_fmt == RINEX) {
      write_rinex_header(file);
   }

   if((have_info & INFO_LOGGED) == 0) {
      write_log_id();
      have_info |= INFO_LOGGED;
   }

   sprintf(log_text, "#");
   write_log_comment(1);

   if(plot_title[0] && (title_type == USER)) {
      format_plot_title(plot_title);
      sprintf(log_text, "#TITLE: %s", out);
      write_log_comment(1);
      sprintf(log_text, "#");
      write_log_comment(1);
   }

   if(debug_text[0]) {
      format_plot_title(debug_text);
      sprintf(log_text, "#DEBUG1: %s", out);
      write_log_comment(1);
      sprintf(log_text, "#");
      write_log_comment(1);
   }
   if(debug_text2[0]) {
      format_plot_title(debug_text2);
      sprintf(log_text, "#DEBUG2: %s", out);
      write_log_comment(1);
      sprintf(log_text, "#");
      write_log_comment(1);
   }
   if(debug_text3[0]) {
      format_plot_title(debug_text3);
      sprintf(log_text, "#DEBUG3: %s", out);
      write_log_comment(1);
      sprintf(log_text, "#");
      write_log_comment(1);
   }
   if(debug_text4[0]) {
      format_plot_title(debug_text4);
      sprintf(log_text, "#DEBUG4: %s", out);
      write_log_comment(1);
      sprintf(log_text, "#");
      write_log_comment(1);
   }
   if(debug_text5[0]) {
      format_plot_title(debug_text5);
      sprintf(log_text, "#DEBUG5: %s", out);
      write_log_comment(1);
      sprintf(log_text, "#");
      write_log_comment(1);
   }
   if(debug_text6[0]) {
      format_plot_title(debug_text6);
      sprintf(log_text, "#DEBUG6: %s", out);
      write_log_comment(1);
      sprintf(log_text, "#");
      write_log_comment(1);
   }
   if(debug_text7[0]) {
      format_plot_title(debug_text7);
      sprintf(log_text, "#DEBUG7: %s", out);
      write_log_comment(1);
      sprintf(log_text, "#");
      write_log_comment(1);
   }

   log_file = old_file;
   return;
}

void log_sky_data(FILE *file)
{
int i;
int sky_shown;

   // write satellite constellation info to the log file

   if(file == 0) return;

   sky_shown = 0;

   for(i=1; i<=SUN_MOON_PRN; i++) {   // mooooo
      if((i == SUN_PRN) && (no_sun_moon & 0x01)) continue;
      if((i == MOON_PRN) && (no_sun_moon & 0x02)) continue;

      if(sat[i].level_msg && ((i == SUN_PRN) || (i == MOON_PRN) || ((sat[i].azimuth >= 0.0) && (sat[i].elevation > 0.0) && (sat[i].tracking > 0)))) {
         if(log_fmt != HEATHER) {
            if(sky_shown == 0) {
               fprintf(file, "        <sky>\n");
               sky_shown = 1;
            }

            fprintf(file, "          <sv prn=\"%d\"", i);
            if     (i < 10)  fprintf(file, "  ");
            else if(i < 100) fprintf(file, " ");

            fprintf(file, "  az=\"%.2f\"", sat[i].azimuth);
            if(sat[i].azimuth < 10.0) fprintf(file, "  ");
            else if(sat[i].azimuth < 100.0) fprintf(file, " ");

            if(0 && (i == SUN_PRN) && (sat[i].sig_level < 0.0)) {
               fprintf(file, "  el=\"%.2f\"", 0.0-sat[i].elevation);
            }
            else if(0 && (i == MOON_PRN) && (sat[i].sig_level < 0.0)) {
               fprintf(file, "  el=\"%.2f\"", 0.0-sat[i].elevation);
            }
            else {
               fprintf(file, "  el=\"%.2f\"", sat[i].elevation);
            }
            if(sat[i].elevation < 10.0) fprintf(file, " ");


            if((i != SUN_PRN) && (i != MOON_PRN)) {
               fprintf(file, "  sig=\"%.2f\"", sat[i].sig_level);
               if(sat[i].sig_level < 10.0) fprintf(file, " ");

               if(have_doppler) {
                  fprintf(file, "  doppler=\"%.3f\"", sat[i].doppler);
                  if     (sat[i].doppler >= 10000.0) fprintf(file, " ");
                  else if(sat[i].doppler >= 1000.0)  fprintf(file, "  ");
                  else if(sat[i].doppler >= 100.0)   fprintf(file, "   ");
                  else if(sat[i].doppler >= 10.0)    fprintf(file, "    ");
                  else if(sat[i].doppler >= 0.0)     fprintf(file, "     ");
                  else if(sat[i].doppler <= -10000.0) ;
                  else if(sat[i].doppler <= -1000.0) fprintf(file, " ");
                  else if(sat[i].doppler <= -100.0)  fprintf(file, "  ");
                  else if(sat[i].doppler <= -10.0)   fprintf(file, "   ");
                  else if(sat[i].doppler <= -0.0)    fprintf(file, "    ");
               }
               if(have_phase) {
                  fprintf(file, "  phase=\"%.3f\"", sat[i].code_phase);
                  if     (sat[i].code_phase >= 100000.0) ;
                  else if(sat[i].code_phase >= 10000.0)  fprintf(file, " ");
                  else if(sat[i].code_phase >= 1000.0)   fprintf(file, "  ");
                  else if(sat[i].code_phase >= 100.0)    fprintf(file, "   ");
                  else if(sat[i].code_phase >= 10.0)     fprintf(file, "   ");
                  else if(sat[i].code_phase >= 0.0)      fprintf(file, "    ");
               }
               if(have_range) {
                  fprintf(file, "  range=\"%.3f\"", sat[i].range);
               }
               if(have_bias) {
                  fprintf(file, "  bias=\"%.10g\"", sat[i].sat_bias);
               }

               if(have_l2_doppler || have_l2_phase || have_l2_range) {
                  fprintf(file, "  l2_sig=\"%.2f\"", sat[i].l2_sig_level);
                  if(sat[i].l2_sig_level < 10.0) fprintf(file, " ");
               }

               if(have_l2_doppler) {
                  fprintf(file, "  l2_doppler=\"%.3f\"", sat[i].l2_doppler);
                  if     (sat[i].l2_doppler >= 10000.0) fprintf(file, " ");
                  else if(sat[i].l2_doppler >= 1000.0)  fprintf(file, "  ");
                  else if(sat[i].l2_doppler >= 100.0)   fprintf(file, "   ");
                  else if(sat[i].l2_doppler >= 10.0)    fprintf(file, "    ");
                  else if(sat[i].l2_doppler >= 0.0)     fprintf(file, "     ");
                  else if(sat[i].l2_doppler <= -10000.0) ;
                  else if(sat[i].l2_doppler <= -1000.0) fprintf(file, " ");
                  else if(sat[i].l2_doppler <= -100.0)  fprintf(file, "  ");
                  else if(sat[i].l2_doppler <= -10.0)   fprintf(file, "   ");
                  else if(sat[i].l2_doppler <= -0.0)    fprintf(file, "    ");
               }
               if(have_l2_phase) {
                  fprintf(file, "  l2_phase=\"%.3f\"", sat[i].l2_code_phase);
                  if     (sat[i].l2_code_phase >= 100000.0) ;
                  else if(sat[i].l2_code_phase >= 10000.0)  fprintf(file, " ");
                  else if(sat[i].l2_code_phase >= 1000.0)   fprintf(file, "  ");
                  else if(sat[i].l2_code_phase >= 100.0)    fprintf(file, "   ");
                  else if(sat[i].l2_code_phase >= 10.0)     fprintf(file, "   ");
                  else if(sat[i].l2_code_phase >= 0.0)      fprintf(file, "    ");
               }
               if(have_l2_range) {
                  fprintf(file, "  l2_range=\"%.3f\"", sat[i].l2_range);
               }
               if(have_bias) {
                  fprintf(file, "  bias=\"%.10g\"", sat[i].sat_bias);
               }
            }

            fprintf(file, "> </sv>\n");
         }
         else if(i == SUN_PRN) {
            if(0 && (sat[i].sig_level < 0.0)) {
               fprintf(file, "#SUN  %04d  %5.1f  %-4.1f  %5.1f", 
                 i, sat[i].azimuth, 0.0-sat[i].elevation, sat[i].sig_level);
            }
            else {
               fprintf(file, "#SUN  %04d  %5.1f  %-4.1f  %5.1f", 
                 i, sat[i].azimuth, sat[i].elevation, sat[i].sig_level);
            }
            fprintf(file, "\n");
         }
         else if(i == MOON_PRN) {
            if(0 && (sat[i].sig_level < 0.0)) {
               fprintf(file, "#MOON %04d  %5.1f  %-4.1f  %5.1f", 
                 i, sat[i].azimuth, 0.0-sat[i].elevation, sat[i].sig_level);
            }
            else {
               fprintf(file, "#MOON %04d  %5.1f  %-4.1f  %5.1f", 
                 i, sat[i].azimuth, sat[i].elevation, sat[i].sig_level);
            }
            fprintf(file, "\n");
         }
         else {
            fprintf(file, "#SIG %02d  %5.1f  %-4.1f  %5.1f", 
              i, sat[i].azimuth, sat[i].elevation, sat[i].sig_level);
            if(have_doppler || have_phase || have_range || have_bias) {
               fprintf(file, "  %10.3f  %13.3f  %12.3f  %.10g", sat[i].doppler, sat[i].code_phase, sat[i].range, sat[i].sat_bias);
            }
            fprintf(file, "\n");
         }
      }
   }

   if(sky_shown) {
      fprintf(file, "        </sky>\n");
   }
}

char *idlwr(int id)
{
   strcpy(out, plot[id].plot_id);
   strlwr(out);
   return &out[0];
}


int show_lpfrs_info(int row,int col, int max_row)
{
int col2;
int color;

   col2 = col + 40;
   color = WHITE;

   if(SCREEN_HEIGHT <= TINY_HEIGHT) {
      no_y_margin = 1;
   }
   if(max_row < 999) {
      if(SCREEN_HEIGHT <= 480) {
         ++row;
         ++max_row;
      }
      fill_rectangle(0,(row+1)*TEXT_HEIGHT, (VER_COL+1)*TEXT_WIDTH-1,(max_row-row)*TEXT_HEIGHT, BLACK);
      goto info_table;
   }

   erase_screen();
     
   if(SCREEN_WIDTH >= NARROW_SCREEN) {
//    show_prs_status(row,col2+40-1, MOUSE_ROW); 
   }

   info_table:
// sprintf(out, "Freq adj:             %g PPB", lpfrs_freq*1.0E9);
   sprintf(out, "Freq adj: (C:%02X F:%02X) %g PPB", lpfrs_c_rval, lpfrs_f_rval, lpfrs_freq*1.0E9);
   vidstr(row++,col, WHITE, out);
   if(row >= max_row) goto done;

   sprintf(out, "HH - photocell:       %7.5f V", lpfrs_hh);
   vidstr(row++,col, WHITE, out);
   if(row >= max_row) goto done;

   sprintf(out, "GG - Rb signal:       %7.5f V", lpfrs_gg);
   vidstr(row++,col, WHITE, out);
   if(row >= max_row) goto done;

   sprintf(out, "EE - varactor:        %7.5f V", lpfrs_ee);
   vidstr(row++,col, WHITE, out);
   if(row >= max_row) goto done;

   sprintf(out, "DD - EFC input:       %7.5f V", lpfrs_dd);
   vidstr(row++,col, WHITE, out);
   if(row >= max_row) goto done;

   sprintf(out, "CC - lamp current:    %7.5f mA", lpfrs_cc);
   vidstr(row++,col, WHITE, out);
   if(row >= max_row) goto done;

   sprintf(out, "BB - heater current:  %7.5f mA", lpfrs_bb);
   vidstr(row++,col, WHITE, out);
   if(row >= max_row) goto done;

   sprintf(out, "AA - 90 MHz AGC:      %7.5f V", lpfrs_aa);
   vidstr(row++,col, WHITE, out);
   if(row >= max_row) goto done;

   sprintf(out, "FF - reserved:        %7.5f", lpfrs_ff);
   vidstr(row++,col, WHITE, out);
   if(row >= max_row) goto done;

   done:
   return row;
}


int show_sa35_info(int row,int col, int max_row)
{
int col2;
int color;

   // !!!!! placeholder
   col2 = col + 40;
   color = WHITE;

   if(SCREEN_HEIGHT <= TINY_HEIGHT) {
      no_y_margin = 1;
   }
   if(max_row < 999) {
      if(SCREEN_HEIGHT <= 480) {
         ++row;
         ++max_row;
      }
      fill_rectangle(0,(row+1)*TEXT_HEIGHT, (VER_COL+1)*TEXT_WIDTH-1,(max_row-row)*TEXT_HEIGHT, BLACK);
      goto info_table;
   }

   erase_screen();
     
   if(SCREEN_WIDTH >= NARROW_SCREEN) {
//    show_prs_status(row,col2+40-1, MOUSE_ROW); 
   }

   info_table:
   sprintf(out, "Freq adj: (C:%02X F:%02X) %g PPB", lpfrs_c_rval, lpfrs_f_rval, lpfrs_freq*1.0E9);
   vidstr(row++,col, WHITE, out);
   if(row >= max_row) goto done;

   sprintf(out, "TEC temperature:      %7.5f C", sa35_tec);
   vidstr(row++,col, WHITE, out);
   if(row >= max_row) goto done;

   sprintf(out, "RF voltage:           %7.5f V", sa35_rf);
   vidstr(row++,col, WHITE, out);
   if(row >= max_row) goto done;

   sprintf(out, "Cell heater current:  %7.5f mA", sa35_heater);
   vidstr(row++,col, WHITE, out);
   if(row >= max_row) goto done;

   sprintf(out, "DC signal:            %7.5f V", sa35_dc);
   vidstr(row++,col, WHITE, out);
   if(row >= max_row) goto done;

   sprintf(out, "EFC input:            %7.5f V", sa35_efc);
   vidstr(row++,col, WHITE, out);
   if(row >= max_row) goto done;

// sa35_dds
// sa35_tune
// sa35_center_freq
// sa35_efc_enable

   done:
   return row;
}


int show_sro_info(int row,int col, int max_row)
{
int col2;
int color;

   col2 = col + 40;
   color = WHITE;

   if(SCREEN_HEIGHT <= TINY_HEIGHT) {
      no_y_margin = 1;
   }
   if(max_row < 999) {
      if(SCREEN_HEIGHT <= 480) {
         ++row;
         ++max_row;
      }
      fill_rectangle(0,(row+1)*TEXT_HEIGHT, (VER_COL+1)*TEXT_WIDTH-0,(max_row-row)*TEXT_HEIGHT, BLACK);
      goto info_table;
   }

   erase_screen();
     
   if(SCREEN_WIDTH >= NARROW_SCREEN) {
//    show_prs_status(row,col2+40-1, MOUSE_ROW); 
   }

   info_table:
   if(sro_r4F & 0x03) {
      sprintf(out, "R4F- DDS limited:     %d", sro_r4F);
      vidstr(row++,col, RED, out);
   }

   if(row >= max_row) goto done;
   sprintf(out, "HH - EFC input:       %7.5f V", sro_hh);
   vidstr(row++,col, WHITE, out);
   if(row >= max_row) goto done;

   sprintf(out, "FF - Rb signal:       %7.5f V", sro_ff);
   vidstr(row++,col, WHITE, out);
   if(row >= max_row) goto done;

   sprintf(out, "EE - photocell:       %7.5f V", sro_ee);
   vidstr(row++,col, WHITE, out);
   if(row >= max_row) goto done;

   sprintf(out, "DD - varactor:        %7.5f V", sro_dd);
   vidstr(row++,col, WHITE, out);
   if(row >= max_row) goto done;

   sprintf(out, "CC - lamp current:    %7.5f %%", sro_cc);
   vidstr(row++,col, WHITE, out);
   if(row >= max_row) goto done;

   sprintf(out, "BB - heater current:  %7.5f %%", sro_bb);
   vidstr(row++,col, WHITE, out);
   if(row >= max_row) goto done;

   sprintf(out, "AA - reserved:        %7.5f", sro_aa);
   vidstr(row++,col, WHITE, out);
   if(row >= max_row) goto done;

   sprintf(out, "GG - reserved:        %7.5f", sro_gg);
   vidstr(row++,col, WHITE, out);
   if(row >= max_row) goto done;


   sprintf(out, "VS - PPSref sigma:    %.1f ns", sro_vs);
   vidstr(row++,col, WHITE, out);
   if(row >= max_row) goto done;

   sprintf(out, "VT - loop time const: %d sec", sro_vt);
   vidstr(row++,col, WHITE, out);
   if(row >= max_row) goto done;

   sprintf(out, "TC - time constant:   %d sec", sro_tc);
   vidstr(row++,col, WHITE, out);
   if(row >= max_row) goto done;

   sprintf(out, "TR - track mode:      %d", sro_tr);
   vidstr(row++,col, WHITE, out);
   if(row >= max_row) goto done;

   sprintf(out, "SY - sync mode:       %d", sro_sy);
   vidstr(row++,col, WHITE, out);
   if(row >= max_row) goto done;

   sprintf(out, "RA - phase adjust:    %d", sro_ra);
   vidstr(row++,col, WHITE, out);
   if(row >= max_row) goto done;

   sprintf(out, "CO - fine phase ofs:  %d ns", sro_co);
   vidstr(row++,col, WHITE, out);
   if(row >= max_row) goto done;

   sprintf(out, "DE - PPS delay:       %.1f ns", (double) sro_de*SRO_TICK);
   vidstr(row++,col, WHITE, out);
   if(row >= max_row) goto done;

   sprintf(out, "PW - PPS width:       %.1f us", (double) sro_pw*SRO_TICK/1000.0);
   vidstr(row++,col, WHITE, out);
   if(row >= max_row) goto done;

   sprintf(out, "FC - freq correction: %g PPT", ((double)sro_fc)*5.12E-13*1.0E12);
   vidstr(row++,col, WHITE, out);
   if(row >= max_row) goto done;

   sprintf(out, "FS - freq save mode:  %d", sro_fs);
   vidstr(row++,col, WHITE, out);
   if(row >= max_row) goto done;

   sprintf(out, "TW - tracking window  %.1f ns", (double) sro_tw*SRO_TICK);
   vidstr(row++,col, WHITE, out);
   if(row >= max_row) goto done;

   sprintf(out, "AW - alarm window:    %.1f ns", (double) sro_aw*SRO_TICK);
   vidstr(row++,col, WHITE, out);
   if(row >= max_row) goto done;

   sprintf(out, "GF - go fast mode:    %d", sro_gf);
   vidstr(row++,col, WHITE, out);
   if(row >= max_row) goto done;

   sprintf(out, "Freq limitation:      %.3f PPT", sro_freq_lim*1.0E12);
   vidstr(row++,col, WHITE, out);
   if(row >= max_row) goto done;

   sprintf(out, "Startup freq corr:    %.3f PPT", sro_poweron_fc*1.0E12);
   vidstr(row++,col, WHITE, out);
   if(row >= max_row) goto done;

   done:
   no_y_margin = 0;
   return row;
}


int show_prs_info(int row,int col, int max_row)
{
int col2;
int color;
int temp;

   col2 = col + 40;
   color = WHITE;

   if(SCREEN_HEIGHT <= TINY_HEIGHT) {
      no_y_margin = 1;
   }
   if(max_row < 999) goto info_table;

   erase_screen();
     
   if(SCREEN_WIDTH >= NARROW_SCREEN) {
      show_prs_status(row,col2+40-1, MOUSE_ROW); 
   }

   if(have_prs_lm) {
      sprintf(out, "LM - lock mode:      %d", prs_lm);
      vidstr(row++,col, WHITE, out);
   }

   if(have_prs_lo) {
      sprintf(out, "LO - lock:           %d", prs_lo);
      vidstr(row++,col, WHITE, out);
   }

   if(have_prs_ep) {
      sprintf(out, "EP - enable power:   %d", prs_ep);
      vidstr(row++,col, WHITE, out);
   }

   if(have_prs_fc) {
      sprintf(out, "FC - freq control:   %d %d", prs_fc1,prs_fc2);
      vidstr(row++,col, WHITE, out);
      sprintf(out, "                     %g", prs_fc_ppt);
      vidstr(row++,col, WHITE, out);
   }

   if(have_prs_fc2) {
      sprintf(out, "FC!- eeprom FC vals: %d %d", prs_ee_fc1,prs_ee_fc2);
      vidstr(row++,col, WHITE, out);
      sprintf(out, "                     %g", prs_ee_fc_ppt);
      vidstr(row++,col, WHITE, out);
   }

   if(have_prs_ds & 0x01) {
      sprintf(out, "DS1- 2nd harmonic:   %d", prs_ds1);
      vidstr(row++,col, WHITE, out);
   }
   if(have_prs_ds & 0x02) {
      sprintf(out, "DS2- sig level:      %d", prs_ds2);
      vidstr(row++,col, WHITE, out);
   }

   if(have_prs_sf) {
      sprintf(out, "SF - set frequency:  %d", prs_sf);
      vidstr(row++,col, WHITE, out);
   }

   if(have_prs_ss) {
      sprintf(out, "SS - set slope:      %d", prs_ss);
      vidstr(row++,col, WHITE, out);
   }

   if(have_prs_ph) {
      sprintf(out, "PH - FLL phase:      %d", prs_ph);
      vidstr(row++,col, WHITE, out);
   }

   if(have_prs_sp) {
      sprintf(out, "SP - FLL params:     %d  %d  %d", prs_sp[0],prs_sp[1],prs_sp[2]);
      vidstr(row++,col, WHITE, out);
   }

   if(have_prs_ms) {
      sprintf(out, "MS - mag switching:  %d", prs_ms);
      vidstr(row++,col, WHITE, out);
   }

   if(have_prs_mo) {
      sprintf(out, "MO - mag offset:     %d", prs_mo);
      vidstr(row++,col, WHITE, out);
   }

   if(have_prs_mr) {
      sprintf(out, "MR - mag read:       %d", prs_mr);
      vidstr(row++,col, WHITE, out);
   }

   if(have_prs_tt) {
      sprintf(out, "TT - time tag:       %d", prs_tt);
      vidstr(row++,col, WHITE, out);
   }

   if(have_prs_ts) {
      sprintf(out, "TS - time slope:     %d", prs_ts);
      vidstr(row++,col, WHITE, out);
   }

   if(have_prs_to) {
      sprintf(out, "TO - time offset:    %d", prs_to);
      vidstr(row++,col, WHITE, out);
   }

   if(have_prs_ps) {
      sprintf(out, "PS - pulse slope:    %d", prs_ps);
      vidstr(row++,col, WHITE, out);
   }



   if(have_prs_pl) {
      sprintf(out, "PL - phase locking:  %d", prs_pl);
      vidstr(row++,col, WHITE, out);
   }

   if(have_prs_ga) {
      sprintf(out, "GA - FLL gain:       %d", prs_ga);
      vidstr(row++,col, WHITE, out);
   }

   if(have_prs_pt) {
      sprintf(out, "PT - PLL TC:         %d", prs_pt);
      vidstr(row++,col, WHITE, out);
   }

   if(have_prs_pf) {
      sprintf(out, "PF - PLL stability:  %d", prs_pf);
      vidstr(row++,col, WHITE, out);
   }

   if(have_prs_pi) {
      sprintf(out, "PI - PLL integrator: %d", prs_pi);
      vidstr(row++,col, WHITE, out);
   }


   if(have_prs_st) {
      ++row;
      sprintf(out, "ST - Status byte 1:  0x%02X", prs_st[0]);
      vidstr(row++,col, WHITE, out);
      sprintf(out, "ST - Status byte 2:  0x%02X", prs_st[1]);
      vidstr(row++,col, WHITE, out);
      sprintf(out, "ST - Status byte 3:  0x%02X", prs_st[2]);
      vidstr(row++,col, WHITE, out);
      sprintf(out, "ST - Status byte 4:  0x%02X", prs_st[3]);
      vidstr(row++,col, WHITE, out);
      sprintf(out, "ST - Status byte 5:  0x%02X", prs_st[4]);
      vidstr(row++,col, WHITE, out);
      sprintf(out, "ST - Status byte 6:  0x%02X", prs_st[5]);
      vidstr(row++,col, WHITE, out);
   }



   row = 1;
   col = col2;
   if(have_prs_ad) {
      sprintf(out, "AD0  - Spare ADC:         %.3f V", prs_ad[0]);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;

      info_table:
      sprintf(out, "AD1  - Heater supply:     %.3f V", prs_ad[1]);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;

      sprintf(out, "AD2  - +24V supply:       %.3f V", prs_ad[2]);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;

      sprintf(out, "AD3  - Lamp FET drain:    %.3f V", prs_ad[3]);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;

      sprintf(out, "AD4  - Lamp FET gate:     %.3f V", prs_ad[4]);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;

      sprintf(out, "AD5  - OCXO heater:       %.3f V", prs_ad[5]);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;

      sprintf(out, "AD6  - Cell heater:       %.3f V", prs_ad[6]);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;

      sprintf(out, "AD7  - Lamp heater:       %.3f V", prs_ad[7]);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;

      sprintf(out, "AD8  - Photo signal:      %.3f", prs_ad[8]);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;

      sprintf(out, "AD9  - I/V converter      %.3f V", prs_ad[9]);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;


      temp = round_temp;
      round_temp = 2;
      sprintf(out, "AD10 - Case temperature:  %s", fmt_temp(prs_ad[10]));
      round_temp = temp;
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;

      sprintf(out, "AD11 - XTAL thermistors:  %.3f", prs_ad[11]);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;

      sprintf(out, "AD12 - Cell thermistors:  %.3f", prs_ad[12]);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;

      sprintf(out, "AD13 - Lamp thermistors:  %.3f", prs_ad[13]);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;

      sprintf(out, "AD14 - Freq cal pot:      %.3f", prs_ad[14]);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;

      sprintf(out, "AD15 - Analog ground:     %.3f V", prs_ad[15]);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;

      sprintf(out, "AD16 - TCXO varactor:     %.3f V", prs_ad[16]);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;

      sprintf(out, "AD17 - VCO varactor:      %.3f V", prs_ad[17]);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;

      sprintf(out, "AD18 - Multiplier gain:   %.3f V", prs_ad[18]);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;

      sprintf(out, "AD19 - RF synth lock:     %.3f V", prs_ad[19]);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;
      if(max_row < 999) goto done;
   }

   if(have_prs_sd) {
      ++row;
      sprintf(out, "SD0  - RF amplitude DAC:  %d", prs_sd[0]);
      vidstr(row++,col, WHITE, out);
      sprintf(out, "SD1  - PPS delay DAC:     %d", prs_sd[1]);
      vidstr(row++,col, WHITE, out);
      sprintf(out, "SD2  - Lamp drain DAC:    %d", prs_sd[2]);
      vidstr(row++,col, WHITE, out);
      sprintf(out, "SD3  - Lamp temp DAC:     %d", prs_sd[3]);
      vidstr(row++,col, WHITE, out);
      sprintf(out, "SD4  - XTAL temp DAC:     %d", prs_sd[4]);
      vidstr(row++,col, WHITE, out);
      sprintf(out, "SD5  - Cell temp DAC:     %d", prs_sd[5]);
      vidstr(row++,col, WHITE, out);
      sprintf(out, "SD6  - OSC amplitude DAC: %d", prs_sd[6]);
      vidstr(row++,col, WHITE, out);
      sprintf(out, "SD7  - RF deviation DAC:  %d", prs_sd[7]);
      vidstr(row++,col, WHITE, out);
   }

   done:
   no_y_margin = 0;
   return row;
}


int show_x72_info(int row,int col, int max_row)
{
int col2;
int color;
int temp;
int last_status_row;

   col2 = col + 40 - 1;
   color = WHITE;
   temp = 0;
   last_status_row = row;

   if(SCREEN_HEIGHT <= TINY_HEIGHT) {
      no_y_margin = 1;
   }
   if(max_row < 999) goto info_table;

   erase_screen();
     
   last_status_row = show_x72_status(row,col+40-1, MOUSE_ROW); 

   if(have_x72_fw) {
      sprintf(out, "FW version:       %.2f  ", x72_fw);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;
   }
   if(x72_date[0]) {
      sprintf(out, "Date:             %s  ", x72_date);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;
   }
   if(have_x72_loader) {
      sprintf(out, "Loader version:   %d  ", x72_loader);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;
   }
   if(x72_serial[0]) {
      sprintf(out, "Serial number:    %s  ", x72_serial);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;
   }
   if(have_x72_tune) {
      sprintf(out, "Tuning state:     %d  ", x72_tune);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;
   }

   sprintf(out, "OSC freq:         %.1f Hz  ", x72_osc);
   vidstr(row++,col, WHITE, out);
   if(row >= max_row) goto done;
   if(have_x72_crystal) {
      sprintf(out, "Crystal freq:     %g Hz  ", x72_crystal);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;
   }
   if(have_x72_acmos) {
      sprintf(out, "ACMOS freq:       %g Hz  ", x72_acmos);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;
   }
   if(have_x72_sine) {
      sprintf(out, "Sine freq:        %g Hz  ", x72_sine);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;
   }

   if(have_x72_res_tempofs) {
      sprintf(out, "Res temp offset:  %g   ", x72_res_tempofs);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;
   }
   if(have_x72_lamp_tempofs) {
      sprintf(out, "Lamp temp offset: %g   ", x72_lamp_tempofs);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;
   }

   if(have_x72_dds_word) {
      sprintf(out, "User DDS offset:  %g   ", x72_dds_word);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;

      sprintf(out, "                  %g   ", x72_dds_word*1.0E-11);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;
   }
   if(user_set_x72_acmos_freq && last_x72_acmos_freq) {
      sprintf(out, "User ACOMS freq:  %g Hz   ", (x72_osc / 2.0) / (double) last_x72_acmos_freq);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;
   }

   ++row;
   if(row >= max_row) goto done;


   info_table:
   if(row >= last_status_row) last_status_row = row;
   row = last_status_row + 1;

   if(have_x72_dmp17) {
      sprintf(out, "dMP17:          %g   ", x72_dmp17);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;
   }

   if(have_x72_dmp5) {
      sprintf(out, "dMP5:           %g   ", x72_dmp5);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;
   }

   if(have_x72_plmp) {
      sprintf(out, "PLmp:           %g   ", x72_plmp);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;
   }

   if(have_x72_pres) {
      sprintf(out, "PRes:           %g   ", x72_pres);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;
   }

   if(have_x72_dlvthermc) {
      sprintf(out, "dLVthermc:      %g   ", x72_dlvthermc);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;
   }

   if(have_x72_drvthermc) {
      sprintf(out, "dRVthermc:      %g   ", x72_drvthermc);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;
   }

   if(have_x72_dlvolt) {
      sprintf(out, "dLVolt:         %g   ", x72_dlvolt);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;
   }

   if(have_x72_dmvoutc) {
      sprintf(out, "dMVoutC:        %g   ", x72_dmvoutc);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;
   }

   if(have_x72_dlvoutc) {
      sprintf(out, "dLVoutC:        %g   ", x72_dlvoutc);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;
   }

   if(have_x72_drvoutc) {
      sprintf(out, "dRVoutC:        %g   ", x72_drvoutc);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;
   }

   if(have_x72_dmv2demavg) {
      sprintf(out, "dMV2demAvg:     %g   ", x72_dmv2demavg);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;
   }

   if(have_x72_dhtrvolt) {
      sprintf(out, "dHtrVolt:       %g V   ", x72_dhtrvolt);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;
   }

   temp = round_temp;
   round_temp = 2;
   if(have_x72_dtemplo) {
      sprintf(out, "dTempLo:        %s   ", fmt_temp(x72_dtemplo));
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;
   }
   round_temp = 3;
   if(have_x72_dtemphi) {
      sprintf(out, "dTempHi:        %s   ", fmt_temp(x72_dtemphi));
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;
   }
   round_temp = temp;

   if(have_x72_dvoltlo) {
      sprintf(out, "dVoltLo:        %g V   ", x72_dvoltlo);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;
   }
   if(have_x72_dvolthi) {
      sprintf(out, "dVoltHi:        %g V   ", x72_dvolthi);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;
   }

   if(max_row >= 999) {    // show zoomed data screen in two columns
      col = col2;
      row = last_status_row+1;
   }

   if(have_x72_creg) {
      sprintf(out, "iFpgaCtl:       0x%04X   ", x72_creg);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;
   }

   if(have_x72_pps) {
      sprintf(out, "1PPS Delta:     %-8d ", x72_pps);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;
   }

   if(have_x72_scont) {
      sprintf(out, "SCont:          %d   ", x72_scont);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;
   }
   if(have_x72_sernum) {
      sprintf(out, "SerNum:         %d   ", x72_sernum);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;
   }

   if(have_x72_lhhrs) {
      sprintf(out, "LHHrs:          %d   ", x72_lhhrs);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;
   }
   if(have_x72_lhticks) {
      sprintf(out, "LHTicks         %d   ", x72_lhticks);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;
   }

   if(have_x72_rhhrs) {
      sprintf(out, "RHHrs:          %d   ", x72_rhhrs);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;
   }
   if(have_x72_rhticks) {
      sprintf(out, "RHTicks         %d   ", x72_rhticks);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;
   }

   if(have_lifetime) {
      sprintf(out, "PwrHrs:         %d   ", scpi_life);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;
   }
   if(have_x72_pwrticks) {
      sprintf(out, "PwrTicks        %d   ", x72_pwrticks);
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;
   }

   temp = round_temp;
   round_temp = 2;
   if(have_temperature) {
      sprintf(out, "dCurTemp:       %s   ", fmt_temp(temperature));
      vidstr(row++,col, WHITE, out);
      if(row >= max_row) goto done;
   }
   round_temp = temp;

   done:
   no_y_margin = 0;
   return row;
}



void show_cs_log(int row,int col, int extended)
{
int i;
int count;

   vidstr(row,col, GREEN, "Log: Message");
   ++row;
   count = 0;
   for(i=cs_log_count-1; i>=0; i--) {
      if(cs_log_msg[i][0]) {
         vidstr(row,col, WHITE, &blanks[TEXT_COLS-LOG_MSG_LEN]);
         vidstr(row,col, WHITE, cs_log_msg[i]);
         ++row;
         ++count;
         if(count >= CS_LOG_SHOW) break;
      }
   }
}


void show_cs_val(int row,int col, int color)
{
char *s;
int i;

   s = strchr(out, ':');
   if(s == 0) return;

   i = s - &out[0] + 1;
   out[i] = 0;
   i = strlen(out);

   vidstr(row,col, WHITE, &blanks[TEXT_COLS-40]);
   vidstr(row,col, GREEN, out);
   vidstr(row,col+i+1, color, &out[i+1]);
}


void show_cs_info()
{
int row, col, col2;
int color;

   row = 1;
   col = 3;
   col2 = 40;
   color = WHITE;
   if(SCREEN_HEIGHT <= TINY_HEIGHT) {
      no_y_margin = 1;
   }

   erase_screen();
   show_cs_log(row+1,col2, 0);

   if(unit_name[0]) {
      sprintf(out,  "Device:   %s", unit_name);
      show_cs_val(row,col, color);
      ++row;
   }
   if(scpi_mfg[0]) {
      sprintf(out,  "Mfg:      %s", scpi_mfg);
      show_cs_val(row,col, color);
      ++row;
   }
   if(scpi_sn[0]) {
      sprintf(out,  "Serial:   %s", scpi_sn);
      show_cs_val(row,col, color);
      ++row;
   }
   if(scpi_fw[0]) {
      sprintf(out,  "Firmware: %s", scpi_fw);
      show_cs_val(row,col, color);
      ++row;
   }
   if(have_cs_cbtid) {
      sprintf(out,  "CBT ID:   %s", cs_cbtid);
      show_cs_val(row,col, color);
      ++row;
   }
   ++row;


   sprintf(out, "Time:        %g   %02d:%02d:%02d", cs_mjd, cs_hours,cs_minutes,cs_seconds);
   show_cs_val(row,col, color);
   ++row;

   if(have_cs_glob) {
      sprintf(out, "Status:      %s", cs_glob);
      show_cs_val(row,col, color);
      ++row;
   }
   if(have_cs_cont)    { 
      if     (cs_cont == 2) sprintf(out,  "Operation:   Continuous");
      else if(cs_cont == 1) sprintf(out,  "Operation:   Enabled");
      else if(cs_cont == 0) sprintf(out,  "Operation:   OFF");
      else                  sprintf(out,  "Operation:   %d?", cs_cont);
      show_cs_val(row,col, color);
      ++row;
   }
   if(no_y_margin == 0) ++row;

   if(have_cs_leapstate) {
      if(cs_leap_state) sprintf(out, "Leap second: Enabled");
      else              sprintf(out, "Leap second: Disabled");
      show_cs_val(row,col, color);
      ++row;
   }
   if(have_cs_leapdur) { 
      sprintf(out,  "Leap dur:    %d seconds", cs_leapdur);
      show_cs_val(row,col, color);
      ++row;
   }
   if(have_cs_leapmjd) { 
      sprintf(out,  "Leap MJD:    %g", cs_leapmjd);
      show_cs_val(row,col, color);
      ++row;
   }
   if(no_y_margin == 0) ++row;

   if(have_cs_sync)    { 
      sprintf(out,  "Sync:        %s", cs_sync);
      show_cs_val(row,col, color);
      ++row;
   }
   if(have_cs_disp)    { 
      if(cs_disp) sprintf(out,  "Display:     ON");
      else        sprintf(out,  "Display:     OFF");
      show_cs_val(row,col, color);
      ++row;
   }
   if(have_cs_standby) { 
      if(cs_standby) sprintf(out,  "Mode:        Standby");
      else           sprintf(out,  "Mode:        Operating");
      show_cs_val(row,col, color);
      ++row;
   }
   if(have_cs_remote)  { 
      if(cs_remote) sprintf(out, "Interface:   Remote");
      else          sprintf(out, "Interface:   Local");
      show_cs_val(row,col, color);
      ++row;
   }

   if(have_cs_slew)    { 
      sprintf(out, "Slew:        %g", cs_slew);
      show_cs_val(row,col, color);
      ++row;
   }


   if(have_cs_supply)    { 
      sprintf(out,  "Power:       %s", cs_supply);
      show_cs_val(row,col, color);
      ++row;
   }
   ++row;

   if(have_cs_freq1)   { 
      sprintf(out,  "Freq1:       %.1f MHz", cs_freq1/1.0E6);
      show_cs_val(row,col, color);
   }
   if(have_cs_freq2)   { 
      sprintf(out,  "Freq2:       %.1f MHz", cs_freq2/1.0E6);
      show_cs_val(row,col2, color);
   }
   ++row;
   if(no_y_margin == 0) ++row;

   if(have_cs_ster)    { 
      if(cs_ster == 0) sprintf(out,  "Steering:    0E-15");
      else             sprintf(out,  "Steering:    %g", cs_ster);
      show_cs_val(row,col, color);
   }
   if(have_dac) {
      sprintf(out,  "EFC:         %g %%", dac_voltage);
      show_cs_val(row,col2, color);
   }
   ++row;

   if(have_cs_rfam & 0x01)    { 
      sprintf(out,  "RF Ampl 1:   %g %%", cs_rfam1);
      show_cs_val(row,col, color);
   }
   if(have_cs_rfam & 0x02)    { 
      sprintf(out,  "RF Ampl 2:   %g %%", cs_rfam2);
      show_cs_val(row,col2, color);
   }
   ++row;

   if(have_cs_emul)    { 
      sprintf(out,  "E-mult:      %g V", cs_emul);
      show_cs_val(row,col, color);
   }
   if(have_cs_gain)    { 
      sprintf(out,  "Gain:        %g %%", cs_gain);
      show_cs_val(row,col2, color);
   }
   ++row;
   if(no_y_margin == 0) ++row;


   if(have_cs_beam)    { 
      sprintf(out,  "Beam:        %g nA", cs_beam);
      show_cs_val(row,col, color);
   }
   if(have_cs_cfield)  { 
      sprintf(out,  "C-field:     %g mA", cs_cfield);
      show_cs_val(row,col2, color);
   }
   ++row;


   if(have_cs_coven)   { 
      sprintf(out,  "Cesium oven: %g V", cs_coven);
      show_cs_val(row,col, color);
   }
   ++row;

   if(have_cs_qoven)   { 
      sprintf(out,  "Quartz oven: %g V", cs_qoven);
      show_cs_val(row,col, color);
   }
   if(have_cs_pump)    { 
      sprintf(out,  "Ion pump:    %g uA", cs_pump);
      show_cs_val(row,col2, color);
   }
   ++row;

   if(have_cs_hwi)     { 
      sprintf(out,  "HWI:         %g V", cs_hwi);
      show_cs_val(row,col, color);
   }
   if(have_cs_msp)     { 
      sprintf(out,  "Mass spec:   %g V", cs_msp);
      show_cs_val(row,col2, color);
   }
   ++row;


   if(have_cs_pll & 0x01) { 
      sprintf(out,  "SAW pll:     %g V", cs_pll_saw);
      show_cs_val(row,col, color);
   }
   if(have_cs_pll & 0x02) { 
      sprintf(out,  "DRO pll:     %g V", cs_pll_dro);
      show_cs_val(row,col2, color);
   }
   ++row;

   if(have_cs_pll & 0x04) { 
      sprintf(out,  "87MHz PLL:   %g V", cs_pll_87);
      show_cs_val(row,col, color);
   }
   if(have_cs_pll & 0x08)     { 
      sprintf(out,  "uP PLL:      %g V", cs_pll_up);
      show_cs_val(row,col2, color);
   }
   ++row;


   if(have_cs_volt & 0x02)    { 
      sprintf(out,  "+12V supply: %g V", cs_volt2);
      show_cs_val(row,col, color);
   }
   if(have_cs_volt & 0x04)    { 
      sprintf(out,  "-12V supply: %g V", cs_volt3);
      show_cs_val(row,col2, color);
   }
   ++row;

   if(have_cs_volt & 0x01)    { 
      sprintf(out,  "+5V supply:  %g V", cs_volt1);
      show_cs_val(row,col, color);
   }
   if(have_temperature)    { 
      sprintf(out,  "Temperature: %s", fmt_temp(temperature));
      show_cs_val(row,col2, color);
   }
   ++row;

   no_y_margin = 0;
}




void log_cs_xml(FILE *file)
{
   if(file == 0) return;

   if(have_cs_beam)    fprintf(file, "        <beam> %g </beam>\n", cs_beam);
   if(have_cs_cfield)  fprintf(file, "        <cfield> %g </cfield>\n", cs_cfield);
   if(have_cs_pump)    fprintf(file, "        <pump> %g </pump>\n", cs_pump);
   if(have_cs_gain)    fprintf(file, "        <gain> %g </gain>\n", cs_gain);
   if(have_cs_emul)    fprintf(file, "        <emult> %g </emult>\n", cs_emul);
   if(have_cs_hwi)     fprintf(file, "        <hwi> %g </hwi>\n", cs_hwi);
   if(have_cs_msp)     fprintf(file, "        <mspec> %g </mspec>\n", cs_msp);
   if(have_cs_coven)   fprintf(file, "        <coven> %g </coven>\n", cs_coven);
   if(have_cs_qoven)   fprintf(file, "        <qoven> %g </qoven>\n", cs_qoven);

   if(have_cs_rfam & 0x01)    fprintf(file, "        <rfampl1> %g </rfampl1>\n", cs_rfam1);
   if(have_cs_rfam & 0x02)    fprintf(file, "        <rfampl2> %g </rfampl2>\n", cs_rfam2);

   if(have_cs_pll & 0x01)     fprintf(file, "        <dro> %g </dro>\n", cs_pll_dro);
   if(have_cs_pll & 0x02)     fprintf(file, "        <saw> %g </saw>\n", cs_pll_saw);
   if(have_cs_pll & 0x04)     fprintf(file, "        <87mhz> %g </87mhz>\n", cs_pll_87);
   if(have_cs_pll & 0x08)     fprintf(file, "        <up> %g </up>\n", cs_pll_up);

   if(have_cs_volt & 0x01)    fprintf(file, "        <volt1> %g </volt1>\n", cs_volt1);
   if(have_cs_volt & 0x02)    fprintf(file, "        <volt2> %g </volt2>\n", cs_volt2);
   if(have_cs_volt & 0x04)    fprintf(file, "        <volt3> %g </volt3>\n", cs_volt3);

   if(have_cs_freq1)   fprintf(file, "        <freq1> %.1f </freq1>\n", cs_freq1);
   if(have_cs_freq2)   fprintf(file, "        <freq2> %.1f </freq2>\n", cs_freq2);
   if(have_cs_ster)    fprintf(file, "        <ster> %g </ster>\n", cs_ster);
   if(have_cs_slew)    fprintf(file, "        <slew> %g </slew>\n", cs_slew);

   if(have_cs_supply)  fprintf(file, "        <supply> %s </supply>\n", cs_supply);

   if(have_cs_leapdur) fprintf(file, "        <leapdur> %d </leapdur>\n", cs_leapdur);
   if(have_cs_leapmjd) fprintf(file, "        <leapmjd> %g </leapmjd>\n", cs_leapmjd);

   if(have_cs_cont)    fprintf(file, "        <cont> %d </cont>\n", cs_cont);
   if(have_cs_sync)    fprintf(file, "        <sync> %s </sync>\n", cs_sync);
   if(have_cs_disp)    fprintf(file, "        <disp> %d </disp>\n", cs_disp);
   if(have_cs_standby) fprintf(file, "        <standby> %d </standby>\n", cs_standby);
   if(have_cs_remote)  fprintf(file, "        <remote> %d </remote>\n", cs_remote);
}


void log_lpfrs_xml(FILE *file)
{
   fprintf(file, "        <aa>%g</aa>\n", lpfrs_aa);
   fprintf(file, "        <bb>%g</bb>\n", lpfrs_bb);
   fprintf(file, "        <cc>%g</cc>\n", lpfrs_cc);
   fprintf(file, "        <dd>%g</dd>\n", lpfrs_dd);
   fprintf(file, "        <ee>%g</ee>\n", lpfrs_ee);
   fprintf(file, "        <ff>%g</ff>\n", lpfrs_ff);
   fprintf(file, "        <gg>%g</gg>\n", lpfrs_gg);
   fprintf(file, "        <hh>%g</hh>\n", lpfrs_hh);
}


void log_sa35_xml(FILE *file)
{
   // !!!!! placeholder
}


void log_sro_xml(FILE *file)
{
   fprintf(file, "        <aa>%g</aa>\n", sro_aa);
   fprintf(file, "        <bb>%g</bb>\n", sro_bb);
   fprintf(file, "        <cc>%g</cc>\n", sro_cc);
   fprintf(file, "        <dd>%g</dd>\n", sro_dd);
   fprintf(file, "        <ee>%g</ee>\n", sro_ee);
   fprintf(file, "        <ff>%g</ff>\n", sro_ff);
   fprintf(file, "        <gg>%g</gg>\n", sro_gg);
   fprintf(file, "        <hh>%g</hh>\n", sro_hh);
}


void log_prs_xml(FILE *file)
{
   if(file == 0) return;

   if(have_prs_lm)             fprintf(file, "        <lm> %d </lm>\n", prs_lm);
   if(have_prs_lo)             fprintf(file, "        <lo> %d </lo>\n", prs_lo);

   if(have_prs_fc & 0x01)      fprintf(file, "        <fc1> %d </fc1>\n", prs_fc1);
   if(have_prs_fc & 0x02)      fprintf(file, "        <fc2> %d </fc2>\n", prs_fc2);

   if(have_prs_ds & 0x01)      fprintf(file, "        <ds1> %d </ds1>\n", prs_ds1);
   if(have_prs_ds & 0x02)      fprintf(file, "        <ds2> %d </ds2>\n", prs_ds2);

   if(have_prs_sf)             fprintf(file, "        <sf> %d </sf>\n", prs_sf);
   if(have_prs_ss)             fprintf(file, "        <ss> %d </ss>\n", prs_ss);
   if(have_prs_ph)             fprintf(file, "        <ph> %d </ph>\n", prs_ph);

   if(have_prs_sp & (1 << 0))  fprintf(file, "        <sp1> %d </sp1>\n", prs_sp[0]);
   if(have_prs_sp & (1 << 1))  fprintf(file, "        <sp2> %d </sp2>\n", prs_sp[1]);
   if(have_prs_sp & (1 << 2))  fprintf(file, "        <sp3> %d </sp3>\n", prs_sp[2]);

   if(have_prs_ms)             fprintf(file, "        <ms> %d </ms>\n", prs_ms);
   if(have_prs_mo)             fprintf(file, "        <mo> %d </mo>\n", prs_mo);
   if(have_prs_mr)             fprintf(file, "        <mr> %d </mr>\n", prs_mr);

   if(have_prs_tt)             fprintf(file, "        <tt> %d </tt>\n", prs_tt);
   if(have_prs_ts)             fprintf(file, "        <ts> %d </ts>\n", prs_ts);
   if(have_prs_to)             fprintf(file, "        <to> %d </to>\n", prs_to);

   if(have_prs_ps)             fprintf(file, "        <ps> %d </ps>\n", prs_ps);
   if(have_prs_pl)             fprintf(file, "        <pl> %d </pl>\n", prs_pl);
   if(have_prs_ep)             fprintf(file, "        <ep> %d </ep>\n", prs_ep);

   if(have_prs_ga)             fprintf(file, "        <ga> %d </ga>\n", prs_ga);
   if(have_prs_pt)             fprintf(file, "        <pt> %d </pt>\n", prs_pt);
   if(have_prs_pf)             fprintf(file, "        <pf> %d </pf>\n", prs_pf);
   if(have_prs_pi)             fprintf(file, "        <pi> %d </pi>\n", prs_pi);

   if(have_prs_ad & (1 << 0))  fprintf(file, "        <ad0> %.3f </ad0>\n", prs_ad[0]);
   if(have_prs_ad & (1 << 1))  fprintf(file, "        <ad1> %.3f </ad1>\n", prs_ad[1]);
   if(have_prs_ad & (1 << 2))  fprintf(file, "        <ad2> %.3f </ad2>\n", prs_ad[2]);
   if(have_prs_ad & (1 << 3))  fprintf(file, "        <ad3> %.3f </ad3>\n", prs_ad[3]);
   if(have_prs_ad & (1 << 4))  fprintf(file, "        <ad4> %.3f </ad4>\n", prs_ad[4]);
   if(have_prs_ad & (1 << 5))  fprintf(file, "        <ad5> %.3f </ad5>\n", prs_ad[5]);
   if(have_prs_ad & (1 << 6))  fprintf(file, "        <ad6> %.3f </ad6>\n", prs_ad[6]);
   if(have_prs_ad & (1 << 7))  fprintf(file, "        <ad7> %.3f </ad7>\n", prs_ad[7]);
   if(have_prs_ad & (1 << 8))  fprintf(file, "        <ad8> %.3f </ad8>\n", prs_ad[8]);
   if(have_prs_ad & (1 << 9))  fprintf(file, "        <ad9> %.3f </ad9>\n", prs_ad[9]);
   if(have_prs_ad & (1 << 10)) fprintf(file, "        <ad10> %.3f </ad10>\n", prs_ad[10]);
   if(have_prs_ad & (1 << 11)) fprintf(file, "        <ad11> %.3f </ad11>\n", prs_ad[11]);
   if(have_prs_ad & (1 << 12)) fprintf(file, "        <ad12> %.3f </ad12>\n", prs_ad[12]);
   if(have_prs_ad & (1 << 13)) fprintf(file, "        <ad13> %.3f </ad13>\n", prs_ad[13]);
   if(have_prs_ad & (1 << 14)) fprintf(file, "        <ad14> %.3f </ad14>\n", prs_ad[14]);
   if(have_prs_ad & (1 << 15)) fprintf(file, "        <ad15> %.3f </ad15>\n", prs_ad[15]);
   if(have_prs_ad & (1 << 16)) fprintf(file, "        <ad16> %.3f </ad16>\n", prs_ad[16]);
   if(have_prs_ad & (1 << 17)) fprintf(file, "        <ad17> %.3f </ad17>\n", prs_ad[17]);
   if(have_prs_ad & (1 << 18)) fprintf(file, "        <ad18> %.3f </ad18>\n", prs_ad[18]);
   if(have_prs_ad & (1 << 19)) fprintf(file, "        <ad19> %.3f </ad19>\n", prs_ad[19]);

   if(have_prs_sd & (1 << 0))  fprintf(file, "        <sd0> %d </sd0>\n", prs_sd[0]);
   if(have_prs_sd & (1 << 1))  fprintf(file, "        <sd1> %d </sd1>\n", prs_sd[1]);
   if(have_prs_sd & (1 << 2))  fprintf(file, "        <sd2> %d </sd2>\n", prs_sd[2]);
   if(have_prs_sd & (1 << 3))  fprintf(file, "        <sd3> %d </sd3>\n", prs_sd[3]);
   if(have_prs_sd & (1 << 4))  fprintf(file, "        <sd4> %d </sd4>\n", prs_sd[4]);
   if(have_prs_sd & (1 << 5))  fprintf(file, "        <sd5> %d </sd5>\n", prs_sd[5]);
   if(have_prs_sd & (1 << 6))  fprintf(file, "        <sd6> %d </sd6>\n", prs_sd[6]);
   if(have_prs_sd & (1 << 7))  fprintf(file, "        <sd7> %d </sd7>\n", prs_sd[7]);
                            
   if(have_prs_st & (1 << 0))  fprintf(file, "        <st1> %d </st1>\n", prs_st[0]);
   if(have_prs_st & (1 << 1))  fprintf(file, "        <st2> %d </st2>\n", prs_st[1]);
   if(have_prs_st & (1 << 2))  fprintf(file, "        <st3> %d </st3>\n", prs_st[2]);
   if(have_prs_st & (1 << 3))  fprintf(file, "        <st4> %d </st4>\n", prs_st[3]);
   if(have_prs_st & (1 << 4))  fprintf(file, "        <st5> %d </st5>\n", prs_st[4]);
   if(have_prs_st & (1 << 5))  fprintf(file, "        <st6> %d </st6>\n", prs_st[5]);
}

void log_x72_xml(FILE *file)
{
   if(file == 0) return;
                                                                         
   if(have_x72_dmp17)      fprintf(file, "        <dmp17>%f</dmp17>\n", x72_dmp17);     
   if(have_x72_dmp5)       fprintf(file, "        <dmp5>%f</dmp5>\n", x72_dmp5);      
   if(have_x72_dhtrvolt)   fprintf(file, "        <dhtrvolt>%f</dhtrvolt>\n", x72_dhtrvolt);  
   if(have_x72_plmp)       fprintf(file, "        <plmp>%f</plmp>\n", x72_plmp);      
   if(have_x72_pres)       fprintf(file, "        <pres>%f</pres>\n", x72_pres);      
   if(have_x72_dlvthermc)  fprintf(file, "        <dlvthermc>%f</dlvthermc>\n", x72_dlvthermc); 
   if(have_x72_drvthermc)  fprintf(file, "        <drvthermc>%f</drvthermc>\n", x72_drvthermc); 
   if(have_x72_dlvolt)     fprintf(file, "        <dlvolt>%f</dlvolt>\n", x72_dlvolt);    
   if(have_x72_dmvoutc)    fprintf(file, "        <dmvoutc>%f</dmvoutc>\n", x72_dmvoutc);   
   if(have_x72_dtemplo)    fprintf(file, "        <dtemplo>%f</dtemplo>\n", x72_dtemplo);   
   if(have_x72_dtemphi)    fprintf(file, "        <dtemphi>%f</dtemphi>\n", x72_dtemphi);   
   if(have_x72_dvoltlo)    fprintf(file, "        <dvoltlo>%f</dvoltlo>\n", x72_dvoltlo);   
   if(have_x72_dvolthi)    fprintf(file, "        <dvolthi>%f</dvolthi>\n", x72_dvolthi);   
   if(have_x72_dlvoutc)    fprintf(file, "        <dlvoutc>%f</dlvoutc>\n", x72_dlvoutc);   
   if(have_x72_drvoutc)    fprintf(file, "        <drvoutc>%f</drvoutc>\n", x72_drvoutc);   
   if(have_x72_dmv2demavg) fprintf(file, "        <dmv2demavg>%f</dmv2demavg>\n", x72_dmv2demavg);

   if(have_x72_creg)       fprintf(file, "        <creg>%d</creg>\n", x72_creg);      
   if(have_x72_scont)      fprintf(file, "        <scont>%d</scont>\n", x72_scont);     
   if(have_x72_sernum)     fprintf(file, "        <sernum>%d</sernum>\n", x72_sernum);    
   if(have_x72_pwrticks)   fprintf(file, "        <pwrticks>%d</pwrticks>\n", x72_pwrticks);  
   if(have_x72_lhhrs)      fprintf(file, "        <lhhrs>%d</lhhrs>\n", x72_lhhrs);     
   if(have_x72_lhticks)    fprintf(file, "        <lhticks>%d</lhticks>\n", x72_lhticks);   
   if(have_x72_rhhrs)      fprintf(file, "        <rhhrs>%d</rhhrs>\n", x72_rhhrs);     
   if(have_x72_rhticks)    fprintf(file, "        <rhticks>%d</rhticks>\n", x72_rhticks);   
}

void write_xml_log(FILE *file)
{
int i;
DATA_SIZE dop_sum;
double old_lat, old_lon;

   // write readings to GPX/XML format log files
   // NEW_RCVR

   if(file == 0) return;

   old_lat = lat;
   old_lon = lon;
   if(log_fmt == GPX) {
      if(plot_loc == 0) {
         lat = 90.0*PI/180.0;
         lon = 180.0*PI/180.0;
      }
      i = (int) (raw_frac*1000.0);
      fprintf(file, "      <trkpt lat=\"%.9f\" lon=\"%.9f\">\n", lat*180.0/PI,lon*180.0/PI);
      fprintf(file, "        <ele>%.3f</ele>\n", alt);
      fprintf(file, "        <time>%04d-%02d-%02dT%02d:%02d:%02d.%03dZ</time>\n", year,month,day,hours,minutes,seconds, i);
      old_lat = lat;
      old_lon = lon;

      if(have_heading) {
         fprintf(file, "        <course>%.3f</course>\n", heading);
      }
      if(have_speed) {
         fprintf(file, "        <speed>%.3f</speed>\n", speed);
      }

      if(have_dops) {
         if(have_dops & HDOP) fprintf(file, "        <hdop>%.2f</hdop>\n", hdop); 
         if(have_dops & VDOP) fprintf(file, "        <vdop>%.2f</vdop>\n", vdop); 
         if(have_dops & PDOP) fprintf(file, "        <pdop>%.2f</pdop>\n", pdop); 
      }

      if(luxor) ;
      else if(rcvr_type == CS_RCVR) ;
      else if(rcvr_type == NO_RCVR) ;
      else if(rcvr_type == TICC_RCVR) ;
      else if(rcvr_type == TIDE_RCVR) ;
      else if(rcvr_mode == RCVR_MODE_2D)       fprintf(file, "        <fix>2d</fix>\n"); 
      else if(rcvr_mode == RCVR_MODE_3D)       fprintf(file, "        <fix>3d</fix>\n"); 
      else if(rcvr_mode == RCVR_MODE_UNKNOWN)  fprintf(file, "        <fix>none</fix>\n"); 
      else if(rcvr_mode == RCVR_MODE_ACQUIRE)  fprintf(file, "        <fix>none>/fix>\n"); 
      else if(rcvr_mode == RCVR_MODE_BAD_GEOM) fprintf(file, "        <fix>none</fix>\n"); 

      if(have_count) fprintf(file, "        <sat>%d</sat>\n", sat_count);
      fprintf(file, "      </trkpt>\n\n");
   }
   else if(log_fmt == XML) {
      if(plot_loc == 0) {
         lat = 90.0*PI/180.0;
         lon = 180.0*PI/180.0;
      }
      i = (int) (raw_frac*1000.0);
      fprintf(file, "      <trkpt lat=\"%.9f\" lon=\"%.9f\">\n", lat*180.0/PI,lon*180.0/PI);
      fprintf(file, "        <ele>%.3f</ele>\n", alt);
      fprintf(file, "        <time>%04d-%02d-%02dT%02d:%02d:%02d.%03dZ</time>\n", year,month,day,hours,minutes,seconds, i);
      old_lat = lat;
      old_lon = lon;

      if(have_week)    fprintf(file, "        <week>%d</week>\n", gps_week);
      if(have_tow)     fprintf(file, "        <tow>%d</tow>\n", pri_tow);
      if(have_utc_ofs) fprintf(file, "        <utcofs>%d</utcofs>\n", utc_offset);
      if(have_utc_ofs) fprintf(file, "        <deltat>%f</deltat>\n", utc_delta_t()*(24.0*60.0*60.0));
      fprintf(file, "\n");

      if(have_heading) {
         fprintf(file, "        <course>%.3f</course>\n", heading);
      }
      if(have_speed) {
         fprintf(file, "        <speed>%.3f</speed>\n", speed);
      }
      if(have_datum) {
         fprintf(file, "        <datum>%d</datum>\n", datum);
      }

      if((rcvr_type == UCCM_RCVR) && (scpi_type == UCCMP_TYPE) && (have_temperature == 0)) {  // samsung
         fprintf(file, "        <tcor>%.6f</tcor>\n", temperature);
      }

      if(have_dac || have_temperature || have_sawtooth || have_osc_offset || 
         have_pps_offset || have_chc_offset || have_chd_offset || 
         have_tfom || have_ffom || have_true_eval || have_star_led || 
         have_temperature2 || have_humidity || have_pressure || 
         have_adc1 || have_adc2 || have_adc3 || have_adc4 || 
         have_ant_v1 || have_ant_v2 || have_ant_ma || 
         have_lars_dac || have_lars_temp || have_lars_pps
      ) {
         if(rcvr_type != THERMO_RCVR) {
            if(have_temperature)  fprintf(file, "        <temp>%.6f</temp>\n", temperature);
         }
         if(have_temperature0) fprintf(file, "        <temp0>%.6f</temp0>\n", tc0);
         if(have_temperature1) fprintf(file, "        <temp1>%.6f</temp1>\n", tc1);
         if(have_temperature2) fprintf(file, "        <temp2>%.6f</temp2>\n", tc2);
         if(have_lars_temp)    fprintf(file, "        <templ>%.6f</templ>\n", lars_temp);

         if(have_humidity)     fprintf(file, "        <rh>%.6f</rh>\n", humidity);
         if(have_pressure)     fprintf(file, "        <mb>%.6f</mb>\n", pressure);
         if(have_adc1)         fprintf(file, "        <adc1>%.6f</adc1>\n", adc1);
         if(have_adc2)         fprintf(file, "        <adc2>%.6f</adc2>\n", adc2);
         if(have_adc3)         fprintf(file, "        <adc3>%.6f</adc3>\n", adc3);
         if(have_adc4)         fprintf(file, "        <adc4>%.6f</adc4>\n", adc4);

         if(have_dac)          fprintf(file, "        <%s>%.6f</%s>\n", idlwr(DAC), dac_voltage, idlwr(DAC));
         if(have_lars_dac)     fprintf(file, "        <%s>%.6f</%s>\n", idlwr(DAC), lars_dac, idlwr(DAC));
         if(have_sawtooth)     fprintf(file, "        <sawtooth>%.6f</sawtooth>\n", dac_voltage);

         if(have_lars_pps) {
             fprintf(file, "        <%s>%.6f</%s>\n", idlwr(PPS), lars_pps,  idlwr(PPS));
         }
         else if(have_pps_offset) {
             fprintf(file, "        <%s>%.6f</%s>\n", idlwr(PPS), pps_offset,  idlwr(PPS));
         }

         if(have_osc_offset) {
             if(rcvr_type == X72_RCVR) {
                fprintf(file, "        <%s>%.6g</%s>\n", idlwr(OSC), osc_offset,  idlwr(OSC));
             }
             else {
                fprintf(file, "        <%s>%.6f</%s>\n", idlwr(OSC), osc_offset,  idlwr(OSC));
             }
         }
         if(have_chc_offset)    fprintf(file, "        <%s>%.6f</%s>\n", idlwr(CHC), chc_offset,  idlwr(CHC));
         if(have_chd_offset)    fprintf(file, "        <%s>%.6f</%s>\n", idlwr(CHD), chd_offset,  idlwr(CHD));

         if(have_esip_pps_type) fprintf(file, "        <gclk>%d</gclk>\n", esip_pps_type);
         if(have_true_eval)     fprintf(file, "        <eval>%.6f</eval>\n", true_eval); 
         if(have_tfom)          fprintf(file, "        <tfom>%d</tfom>\n", tfom); 
         if(have_ffom) {
            if(rcvr_type == BRANDY_RCVR) fprintf(file, "        <pll>%d</pll>\n", ffom); 
            else                         fprintf(file, "        <ffom>%d</ffom>\n", ffom); 
         }
         if(have_star_led) {
            fprintf(file, "        <led>%d</led>\n", star_led); 
         }

         if(luxor) {
            fprintf(file, "        <lumens>%.6f</lumens>\n", lux2);
            fprintf(file, "        <ledi>%.6f</ledi>\n", led_i);
            fprintf(file, "        <ledv>%.6f</ledv>\n", led_v);
            fprintf(file, "        <temp2>%.6f</temp2>\n", luxor_tc2);
            fprintf(file, "        <blue>%.6f</blue>\n", blue_hz);
            fprintf(file, "        <green>%.6f</green>\n", green_hz);
            fprintf(file, "        <red>%.6f</red>\n", red_hz);
            fprintf(file, "        <white>%.6f</white>\n", white_hz);
            fprintf(file, "        <pwm>%.6f</pwm>\n", pwm_hz);
            fprintf(file, "        <auxv>%.6f</auxv>\n", adc2);
         }
         else if(rcvr_type == CS_RCVR) ;
         else if(rcvr_type == TICC_RCVR) ;
         else if((rcvr_type == RFTG_RCVR) && have_rftg_unit && (have_ant_v1 || have_ant_v2 || have_ant_ma)) {
            if(rftg_unit == 0) {
               if(have_ant_v1) fprintf(file, "        <lampv>%.6f</lampv>\n", ant_v1);
               if(have_ant_v2) fprintf(file, "        <ocxov>%.6f</ocxov>\n", ant_v2);
            }
            else {
               if(have_ant_v1) fprintf(file, "        <antv1>%.6f</antv1>\n", ant_v1);
               if(have_ant_v2) fprintf(file, "        <antv2>%.6f</antv2>\n", ant_v2);
               if(have_ant_ma) fprintf(file, "        <antma>%.6f</antma>\n", ant_ma);
            }
            fprintf(file, "        <oscv>%.6f</oscv>\n", rftg_oscv);
         }
         else if(have_ant_v1 || have_ant_v2 || have_ant_ma) {
            if(have_ant_v1) fprintf(file, "        <antv1>%.6f</antv1>\n", ant_v1);
            if(have_ant_v2) fprintf(file, "        <antv2>%.6f</antv2>\n", ant_v2);
            if(have_ant_ma) fprintf(file, "        <antma>%.6f</antma>\n", ant_ma);
         }
         else fprintf(file, "\n");
      }

      if(rcvr_type == CS_RCVR) {
         log_cs_xml(file);
      }
      else if(rcvr_type == LPFRS_RCVR) {
         log_lpfrs_xml(file);
      }
      else if(rcvr_type == PRS_RCVR) {
         log_prs_xml(file);
      }
      else if(rcvr_type == SA35_RCVR) {
         log_sa35_xml(file);
      }
      else if(rcvr_type == SRO_RCVR) {  // !!!!!SRO
         log_sro_xml(file);
      }
      else if(rcvr_type == X72_RCVR) {
         log_x72_xml(file);
      }

      if(have_tc | have_damp | have_gain | have_initv) {
         if(have_tc)    fprintf(file, "        <tc>%.6f</tc>\n", time_constant); 
         if(have_damp)  fprintf(file, "        <damp>%.6f</damp>\n", damping_factor); 
         if(have_gain)  fprintf(file, "        <oscgain>%.6f</oscgain>\n", osc_gain); 
         if(have_initv) fprintf(file, "        <initv>%.6f</initv>\n", initial_voltage); 
         fprintf(file, "\n");
      }

      i = 0;
      if(have_pps_enable) {
         if(pps_enabled) fprintf(file, "        <pulse>on</pulse>\n"); 
         else            fprintf(file, "        <pulse>off</pulse>\n"); 
         ++i;
      }

      if(have_pps_rate) {
         if((rcvr_type == UBX_RCVR) && (pps_rate == RATE_100PPS)) {
            fprintf(file, "        <ppsrate>100.0</ppsrate>\n"); 
         }
         else if((rcvr_type == UBX_RCVR) && (pps_rate == RATE_1000PPS)) {
            fprintf(file, "        <ppsrate>1000.0</ppsrate>\n"); 
         }
         else if((rcvr_type == UBX_RCVR) && (pps_rate == RATE_USER)) {  // !!!!! uuuuu user rate
            fprintf(file, "        <ppsrate>-1.0</ppsrate>\n"); 
         }
         else if((rcvr_type == MOTO_RCVR) && (pps_rate != RATE_USER)) {
            fprintf(file, "        <ppsrate>100.0</ppsrate>\n"); 
         }
         else if(pps_rate == RATE_PP2S) {
            fprintf(file, "        <ppsrate>0.50</ppsrate>\n"); 
         }
         else if(rcvr_type == ZODIAC_RCVR) {
            fprintf(file, "        <ppsrate>1.0</ppsrate>\n"); 
         }
         else {
            fprintf(file, "        <ppsrate>1.0</ppsrate>\n"); 
         }
         ++i;
      }

      if(have_pps_polarity) {
         if(pps_polarity) fprintf(file, "        <ppsedge>fall</ppsedge>\n"); 
         else             fprintf(file, "        <ppsedge>rise</ppsedge>\n"); 
         ++i;
      }

      if(have_osc_polarity) {
         if(osc_polarity) fprintf(file, "        <oscedge>fall</oscedge>\n"); 
         else             fprintf(file, "        <oscedge>rise</oscedge>\n"); 
         ++i;
      }

      if(have_cable_delay) {
         fprintf(file, "        <cable>%f</cable>\n", cable_delay*1.0E9); 
         ++i;
      }
      if(have_rf_delay) {
         fprintf(file, "        <rfdelay>%f</rfdelay>\n", rf_delay*1.0E9); 
         ++i;
      }
      if(have_pps_delay & 0x01) {
         fprintf(file, "        <pps1delay>%f</pps1delay>\n", pps1_delay); 
         ++i;
      }
      if(have_pps_delay & 0x02) {
         fprintf(file, "        <pps2delay>%f</pps2delay>\n", pps2_delay); 
         ++i;
      }

      if(i) fprintf(file, "\n");


      dop_sum = hdop + vdop + pdop + gdop + tdop + ndop + edop;
      if(have_dops && (dop_sum != 0.0)) {
         if(have_dops & HDOP) fprintf(file, "        <hdop>%.2f</hdop>\n", hdop); 
         if(have_dops & VDOP) fprintf(file, "        <vdop>%.2f</vdop>\n", vdop); 
         if(have_dops & PDOP) fprintf(file, "        <pdop>%.2f</pdop>\n", pdop); 
         if(have_dops & GDOP) fprintf(file, "        <gdop>%.2f</gdop>\n", gdop); 
         if(have_dops & TDOP) fprintf(file, "        <tdop>%.2f</tdop>\n", tdop); 
         if(have_dops & NDOP) fprintf(file, "        <ndop>%.2f</ndop>\n", ndop); 
         if(have_dops & EDOP) fprintf(file, "        <edop>%.2f</edop>\n", edop); 
         fprintf(file, "\n");
      }

      i = 1;
      if(luxor) i = 0;
      else if(rcvr_type == CS_RCVR) i = 0;
      else if(rcvr_type == LPFRS_RCVR) i = 0;
      else if(rcvr_type == NO_RCVR) i = 0;
      else if(rcvr_type == PRS_RCVR) i = 0;
      else if(rcvr_type == SRO_RCVR) i = 0;
      else if(rcvr_type == SA35_RCVR) i = 0;
      else if(rcvr_type == TICC_RCVR) i = 0;
      else if(rcvr_type == TIDE_RCVR) i = 0;
      else if(rcvr_type == X72_RCVR) i = 0;
      else if(rcvr_mode == RCVR_MODE_2D_3D)    fprintf(file, "        <fix>2d3d</fix>\n"); 
      else if(rcvr_mode == RCVR_MODE_SINGLE)   fprintf(file, "        <fix>single</fix>\n"); 
      else if(rcvr_mode == RCVR_MODE_2D)       fprintf(file, "        <fix>2d</fix>\n"); 
      else if(rcvr_mode == RCVR_MODE_3D)       fprintf(file, "        <fix>3d</fix>\n"); 
      else if(rcvr_mode == RCVR_MODE_DGPS)     fprintf(file, "        <fix>dgps</fix>\n"); 
      else if(rcvr_mode == RCVR_MODE_2DCLK)    fprintf(file, "        <fix>clk2d</fix>\n"); 
      else if(rcvr_mode == RCVR_MODE_HOLD)     fprintf(file, "        <fix>posnhold</fix>\n"); 
      else if(rcvr_mode == RCVR_MODE_PROP) {
         if((rcvr_type == SCPI_RCVR) || (rcvr_type == UCCM_RCVR)) {
            fprintf(file, "        <fix>holdover</fix>\n"); 
         }
         else {
            fprintf(file, "        <fix>propogate</fix>\n"); 
         }
      }
      else if(rcvr_mode == RCVR_MODE_ACQUIRE)  fprintf(file, "        <fix>acquire</fix>\n"); 
      else if(rcvr_mode == RCVR_MODE_BAD_GEOM) fprintf(file, "        <fix>badgeom</fix>\n"); 
      else if(rcvr_mode == RCVR_MODE_SURVEY)   fprintf(file, "        <fix>survey</fix>\n"); 
      else                                     fprintf(file, "        <fix>unknown</fix>\n"); 
      if(i) fprintf(file, "\n");

      if(have_amu)     fprintf(file, "        <snrmask>%.1f</snrmask>\n", amu_mask);
      if(have_el_mask) fprintf(file, "        <elmask>%.1f</elmask>\n", el_mask);
      if(have_gnss_mask && gnss_mask) {
         out[0] = 0;
         if(gnss_mask & GPS)     strcat(out, "G");
         if(gnss_mask & GLONASS) strcat(out, "R");
         if(gnss_mask & BEIDOU)  strcat(out, "C");
         if(gnss_mask & GALILEO) strcat(out, "E");
         if(gnss_mask & SBAS)    strcat(out, "S");
         if(gnss_mask & QZSS)    strcat(out, "Q");
         if(gnss_mask & IMES)    strcat(out, "I");
         fprintf(file, "        <gnss>%s", out);
         fprintf(file, "</gnss>\n");
      }
      if(have_count)   fprintf(file, "        <sat>%d</sat>\n", sat_count);
      if(log_db) log_sky_data(file);

      if(lat || lon) {
         if(have_tides) {
            fprintf(file, "\n");
            fprintf(file, "        <lattide>%.2f</lattide>\n", lat_tide); 
            fprintf(file, "        <lontide>%.2f</lontide>\n", lon_tide); 
            fprintf(file, "        <alttide>%.2f</alttide>\n", alt_tide); 
         }
         if(have_ugals) {
            if(have_tides == 0) fprintf(file, "\n");
            fprintf(file, "        <ugals>%.2f</ugals>\n", ugals); 
         }
      }

      fprintf(file, "      </trkpt>\n\n");
   }
   else if(log_fmt == KML) {
      fprintf(file, "            %.9f,%.9f,%.3f\n", lon*180.0/PI,lat*180.0/PI,alt);
   }
}

void write_log_readings(FILE *file, long x)
{
int all_info_avail;
char *s;

   // write readings to ASCII format log file
   // NEW_RCVR

   if(file == 0) return;  // log not open
   if(tow == 0) return;   // no data from GPS yet
   if(log_stream & LOG_HEX_STREAM) return; // dumping raw tsip data

   // see if all available device info has been collected
   all_info_avail = 1;
   if(have_info & INFO_LOGGED) ;
   else if((rcvr_type == THERMO_RCVR) || (rcvr_type == ZYFER_RCVR)) {
      if     ((have_info & MANUF_PARAMS) == 0) all_info_avail = 0;
      else if((have_info & PRODN_PARAMS) == 0) all_info_avail = 0;
      else if((have_info & VERSION_INFO) == 0) all_info_avail = 0;
      else if((have_info & MFG_INFO) == 0)     all_info_avail = 0;
   }
   else {
      if     ((have_info & MANUF_PARAMS) == 0) all_info_avail = 0;
      else if((have_info & PRODN_PARAMS) == 0) all_info_avail = 0;
      else if((have_info & VERSION_INFO) == 0) all_info_avail = 0;
   }

   if(sim_file) ;  // sim capture files may not have ID info in them
   else if((no_log_id_wait == 0) && (all_info_avail == 0)) {
      return;   // wait for receiver id info before logging data
   }


   if((no_log_id_wait || sim_file) && (all_info_avail == 0)) {
       // log data without having seen receiver id info
   }
   else if(log_stream & (LOG_HEX_STREAM | LOG_SENT_DATA | LOG_PACKET_START)) {
       // always log info if logging other than raw stream
   }
   else if(have_info & INFO_LOGGED) {
       // device id info already written to log
   }
   else if(have_info) {
      write_log_header(file, 1);
      have_info |= INFO_LOGGED;
   }


   if(hours != last_log_hours) {  // sync log data to disk every hour
      if(file == log_file) {
         sync_log_file();
         file = log_file;  // in case sync_log_file changed the file descriptor
      }
      last_log_hours = hours;
      if(log_header) log_written = 0;
   }

   if(log_written == 0) {  // write header each time log opens
      log_written = 1;
      month = fix_month(month);
      if(log_comments && (log_fmt == HEATHER)) {
         fprintf(file,"#\n");

         //!!!! NOTE: IF YOU CHANGE THE CONTENT OR SPACING IF THIS LUNE,  
         //           YOU MUST CHANGE reload_log() TO MATCH !!!!
         month = fix_month(month);
         if((nav_rate != 1.0) || (rcvr_type == BRANDY_RCVR)) {
            fprintf(file,"#  %02d:%02d:%02d.%03d %s   %02d %s %04d - interval %.3f seconds\n", 
               hours,minutes,seconds,(int)(raw_frac*1000.0+0.50), (time_flags & TFLAGS_UTC)?"UTC":"GPS", day,months[month],year, (double)log_interval/(double)nav_rate);
         }
         else {
            fprintf(file,"#  %02d:%02d:%02d.%03d %s   %02d %s %04d - interval %ld seconds\n", 
               hours,minutes,seconds,(int)(raw_frac*1000.0+0.50), (time_flags & TFLAGS_UTC)?"UTC":"GPS", day,months[month],year, log_interval);
         }

         fprintf(file,"#\n");

         if(luxor) {
            fprintf(file,"# time\t  tow   \t  LUX1\t LUMENS\t  BATTi\t  BATTv\t  LEDi \t  LEDv \t TEMP1\t TEMP2\t   Blue\t  Green\t    Red\t  White\t   PWM\t  AUXv\n");
         }
         else if(rcvr_type == NO_RCVR) {
            fprintf(file,"# time\t  tow\n");
         }
         else {
            if(tow >= 1000000) s = "# tow ";
            else               s = "# tow";
            if(nav_rate != 1.0) {
                if(rcvr_type == TIDE_RCVR) fprintf(file, "%s\t\t\t", s);
                else                       fprintf(file, "%s\t\t\t\t", s);
            }
            else if(rcvr_type == TIDE_RCVR)  fprintf(file, "%s\t\t", s);
            else                             fprintf(file, "%s\t\t\t", s);

            if(rcvr_type == ACRON_RCVR) {
               fprintf(file,"\n");
            }
            else if(rcvr_type == BRANDY_RCVR) {
               fprintf(file,"phase(sec) \tosc(%s)\tdac(%s)  \ttemp(C)\t\tsats\n", ppb_string, plot[DAC].units);
            }
            else if(rcvr_type == CS_RCVR) {
               fprintf(file,"pump(uA) \temult(%s)\tdac(%s)  \ttemp(C)\t\tsats\n", "V", plot[DAC].units);
            }
            else if(rcvr_type == FURUNO_RCVR) {
               fprintf(file,"lat        \tlon     \talt (m)\t\tsats\n");
            }
            else if(rcvr_type == GPSD_RCVR) {
               fprintf(file,"lat        \tlon     \talt (m)\t\tsats\n");
            }
            else if(rcvr_type == LPFRS_RCVR) {  // !!!!!LPFRS
               fprintf(file,"TT(sec) \tFC(%s)\tEFC(%s)  \ttemp(C)\t\tsats\n", ppb_string, "V");
            }
            else if(rcvr_type == NMEA_RCVR) {
               fprintf(file,"tlat        \tlon     \talt (m)\t\tsats\n");
            }
            else if(rcvr_type == PRS_RCVR) {
               fprintf(file,"TT(sec) \tFC(%s)\tdac(%s)  \ttemp(C)\t\tsats\n", ppb_string, "V");
            }
            else if(rcvr_type == RFTG_RCVR) {
               fprintf(file,"phase(sec) \tosc(%s)\tdac(%s)  \ttemp(C)\t\tsats\n", ppb_string, plot[DAC].units);
            }
            else if(rcvr_type == SCPI_RCVR) {
               fprintf(file,"pps(ns)  \tunc(%s) \tdac(%s)  \ttemp(C)\t\tsats\n", "us", plot[DAC].units);
            }
            else if(rcvr_type == SA35_RCVR) {  // !!!!!SA35
               fprintf(file,"TT(sec) \tFC(%s)\tEFC(%s)  \ttemp(C)\t\tsats\n", ppb_string, "V");
            }
            else if(rcvr_type == SRO_RCVR) {
               fprintf(file,"pps(sec) \tosc(%s)\tsig(%s)  \tVT(s)\n", "PPB", "V");
            }
            else if(rcvr_type == THERMO_RCVR) {
               fprintf(file,"temp1(C)\ttemp2(C)\tmb\trh\tadc3\tadc4\n");
            }
            else if(rcvr_type == TICC_RCVR) {
               fprintf(file,"chA(ps)  \t\tchB(ps) \t\tchC(ps)\t\tchD(ps)\n");
            }
            else if(rcvr_type == TIDE_RCVR) {  // ckckck
               fprintf(file,"  lat(mm)\tlon(mm)  \talt(mm)  \tgrav(uGals)\n");
            }
            else if((rcvr_type == UCCM_RCVR) && (scpi_type == UCCMP_TYPE) && (have_temperature == 0)) { // samsung
               fprintf(file,"tpps(sec) \tosc(%s)\tdac(%s)  \ttcor(ppt)\t\tsats\n", ppb_string, plot[DAC].units);
            }
            else if(rcvr_type == UCCM_RCVR) {
               fprintf(file,"pps(sec) \tosc(%s)\tdac(%s)  \ttemp(C)\t\tsats\n", ppb_string, plot[DAC].units);
            }
            else if(rcvr_type == UBX_RCVR) {
               fprintf(file,"accu(ns)   \tfrac(ns)\tsawtooth \ttemp(C)\t\tsats\n");
            }
            else if(TIMING_RCVR) {
               if(rcvr_type == ESIP_RCVR) {
                  fprintf(file,"accu(ns)   \tcofs(ns)\tsawtooth \ttemp(C)\t\tsats\n");
               }
               else if(rcvr_type == MOTO_RCVR) {
                  fprintf(file,"accu(ns)   \tcofs(ns)\tsawtooth \ttemp(C)\t\tsats\n");
               }
               else if(rcvr_type == NVS_RCVR) {
                  fprintf(file,"rgen(ns/s) \tbias(ns)\tsawtooth \ttemp(C)\t\tsats\n");
               }
               else if(rcvr_type == SIRF_RCVR) {
                  fprintf(file,"Drift(ns)  \tcofs(%s)\tsawtooth \ttemp(C)\t\tsats\n", ppb_string);
               }
               else if(rcvr_type == ZODIAC_RCVR) {
                  fprintf(file,"PPS(ns)    \tcofs(ns)\tsawtooth \ttemp(C)\t\tsats\n");
               }
               else if(lte_lite) {
                  fprintf(file,"pps(ns)    \tosc(%s) \tDAC      \ttemp(C)\t\tsats\n", ppb_string);
               }
               else {
                  fprintf(file,"bias(ns)   \trate(%s)\tsawtooth \ttemp(C)\t\tsats\n", ppb_string);
               }
            }
            else if(rcvr_type == TM4_RCVR) {
               fprintf(file,"\n");
            }
            else if(rcvr_type == TSERVE_RCVR) {
               fprintf(file,"\n");
            }
            else if(rcvr_type == Z12_RCVR) {
               fprintf(file,"tlat        \tlon     \talt (m)\t\tsats\n");
            }
            else if(rcvr_type == TSIP_RCVR) {
               fprintf(file,"pps(sec) \tosc(%s)\tdac(%s)  \ttemp(C)\t\tsats\n", ppb_string, "V");
            }
            else {
               fprintf(file,"pps(sec) \tosc(%s)\tdac(%s)  \ttemp(C)\t\tsats\n", ppb_string, plot[DAC].units);
            }
         }
      }
      else if(log_comments && ((log_fmt == XML) || (log_fmt == GPX) || (log_fmt == KML))) {
         sprintf(log_text,"#  %02d:%02d:%02d.%03d %s   %02d %s %04d - interval %.2f seconds", 
            hours,minutes,seconds,(int)(raw_frac*1000.0+0.50), (time_flags & TFLAGS_UTC)?"UTC":"GPS", day,months[month],year, (double)log_interval/(double)nav_rate);
         write_log_comment(2);
      }
   }

   if(spike_threshold && last_temperature && (ABS(temperature - last_temperature) >= spike_threshold)) {
//    sprintf(log_text,"#! temperature spike: %f at tow %lu", (temperature-last_temperature), tow);
//    write_log_comment(1);
   }


   if(log_fmt == RINEX) {
      write_rinex_obs(file);
      return;
   }
   else if(log_fmt != HEATHER) {
      write_xml_log(file);
      return;
   }

//fprintf(file, "%ld: ", x);
   if((nav_rate != 1.0) || (rcvr_type == BRANDY_RCVR)) {
      fprintf(file, "%02d:%02d:%02d.%03d", hours,minutes,seconds,(int)(raw_frac*1000.0+0.5));
      if(csv_char == '\t') {
         fprintf(file, "  ");
      }
      else {
         fprintf(file, "%c", csv_char);
      }
      if(rcvr_type == TIDE_RCVR)    fprintf(file, "%6lu.%03d%c",   (unsigned long) tow, (int) (raw_frac*1000.0+0.5), ' '); 
      else if(rcvr_type == NO_RCVR) fprintf(file, "%6lu.%03d%c",   (unsigned long) tow, (int) (raw_frac*1000.0+0.5), ' '); 
      else                          fprintf(file, "%6lu.%03d%c",   (unsigned long) tow, (int) (raw_frac*1000.0+0.5), csv_char);
   }
   else {
      fprintf(file, "%02d:%02d:%02d", hours,minutes,seconds);
      if(csv_char == '\t') {
         fprintf(file, "  ");
      }
      else {
         fprintf(file, "%c", csv_char);
      }
      if(rcvr_type == TIDE_RCVR)    fprintf(file, "%6lu%c",   (unsigned long) tow, ' '); 
      else if(rcvr_type == NO_RCVR) fprintf(file, "%6lu%c",   (unsigned long) tow, ' '); 
      else                          fprintf(file, "%6lu%c",   (unsigned long) tow, csv_char); 
   }

   if(luxor) {
      fprintf(file, "%7.3f%c%7.3f%c%7.3f%c%7.3f%c%7.3f%c%7.3f%c%7.3f%c%7.3f%c%7.0f%c%7.0f%c%7.0f%c%7.0f%c%7.0f%c%7.3f \n", 
         pps_offset, csv_char, // lux1
         lux2, csv_char,               // lux2
         osc_offset, csv_char, // batti
         dac_voltage, csv_char,        // battv
         led_i, csv_char,              // ledi
         led_v, csv_char,              // ledv
         temperature, csv_char, 
         luxor_tc2, csv_char, 
         blue_hz, csv_char,
         green_hz, csv_char,
         red_hz, csv_char,
         white_hz, csv_char,
         pwm_hz, csv_char,
         adc2
      );
   }
   else if(rcvr_type == BRANDY_RCVR) {
      fprintf(file, "%-12g%c%f%c%f%c%f%c%d \n", 
         pps_offset/1.0E9, csv_char, 
         osc_offset, csv_char, 
         dac_voltage, csv_char, 
         temperature, csv_char, 
         sat_count
      );
   }
   else if(rcvr_type == FURUNO_RCVR) {
      fprintf(file, "%f%c%f%c%f%c%d \n", 
         lat*180.0/PI, csv_char,
         lon*180.0/PI,  csv_char,
         alt, csv_char,
         sat_count
      );
   }
   else if(rcvr_type == LPFRS_RCVR) {  //!!!!!LPFRS
      fprintf(file, "%g%c%f%c%f%c%f%c%d \n", 
         pps_offset/1.0E9, csv_char, 
         osc_offset, csv_char, 
         lpfrs_dd, csv_char, 
         lpfrs_gg, csv_char, 
         sat_count
      );
   }
   else if((rcvr_type == NMEA_RCVR) || (rcvr_type == GPSD_RCVR) || (rcvr_type == Z12_RCVR)) {
      fprintf(file, "%f%c%f%c%f%c%d \n", 
         lat*180.0/PI, csv_char,
         lon*180.0/PI,  csv_char,
         alt, csv_char,
         sat_count
      );
   }
   else if((rcvr_type == NO_RCVR) || (rcvr_type == ACRON_RCVR) || (rcvr_type == TSERVE_RCVR)) {
      fprintf(file, "\n");
   }
   else if(rcvr_type == SA35_RCVR) {  //!!!!!SA35
      fprintf(file, "%g%c%f%c%f%c%f%c%d \n", 
         pps_offset/1.0E9, csv_char, 
         osc_offset, csv_char, 
         lpfrs_dd, csv_char, 
         lpfrs_gg, csv_char, 
         sat_count
      );
   }
   else if(rcvr_type == SCPI_RCVR) {
      fprintf(file, "%f%c%f%c%f%c%f%c%d \n", 
         pps_offset, csv_char,
         osc_offset*1000.0,  csv_char,
         dac_voltage, csv_char,
         temperature, csv_char, 
         sat_count
      );
   }
   else if(rcvr_type == SRO_RCVR) {
      fprintf(file, "%g%c%f%c%f%c%d%c%d \n", 
         pps_offset/1.0E9, csv_char, 
         osc_offset*1000.0, csv_char, 
         sro_ff, csv_char, 
         sro_vt, csv_char, 
         sat_count
      );
   }
   else if(rcvr_type == THERMO_RCVR) {
      fprintf(file, "%7.3f%c%7.3f  %c%7.3f%c%7.3f%c%7.4f%c%7.4f \n", 
         tc1,csv_char, tc2,csv_char, pressure,csv_char, humidity,csv_char, adc3,csv_char, adc4   // aaaahhhh  
      );
   }
   else if(rcvr_type == TICC_RCVR) {
      fprintf(file, "%14.12f%c%14.12f%c%14.12f%c%14.12f \n", 
         (pps_phase), csv_char,    // aaaahhhh
         (osc_phase),  csv_char,
         (chc_phase), csv_char,
         (chd_phase)
      );
   }
   else if(rcvr_type == TIDE_RCVR) {  // ckckck
      fprintf(file, "%f%c%f%c%f%c%f \n", 
         lat_tide, csv_char,
         lon_tide, csv_char,
         alt_tide, csv_char,
         ugals
      );
   }
   else if(TIMING_RCVR) {
      fprintf(file, "%f%c%f%c%f%c%f%c%d \n", 
         pps_offset, csv_char,
         osc_offset,  csv_char,
         dac_voltage, csv_char,
         temperature, csv_char, 
         sat_count
      );
   }
   else if(rcvr_type == TM4_RCVR) {
      fprintf(file, "\n");
   }
   else {
      fprintf(file, "%g%c%f%c%f%c%f%c%d \n", 
         pps_offset/1.0E9, csv_char, 
         osc_offset, csv_char, 
         dac_voltage, csv_char, 
         temperature, csv_char, 
         sat_count
      );
   }

   if(file && log_comments && log_db && (file == log_file)) {
      log_sky_data(file);
   }

   if(file && log_flush_mode) fflush(file);
}


// #define SCI_LOG "sci.log"  // for testing SCILAB code that calcs oscillator params
FILE *sci_file;   
long nnn;

u32 fake_tow(double jd)
{
u32 tow;

   // fake the time of week time stamp

   jd = jd - GPS_EPOCH;
   jd *= (24.0*60.0*60.0);

   tow = (u32) (jd+0.50);
// tow = tow % (u32) (24L*60L*60L*7L);
   return tow;
}


void write_q_entry(FILE *file, long i)
{
struct PLOT_Q q;
int j;
int hh,mm,ss,dd,mo,yy;
double frac;
u32 tow_save;
int count_save;
u08 flag_save;
DATA_SIZE lux2_save;
DATA_SIZE ledi_save;
DATA_SIZE ledv_save;
DATA_SIZE adc2_save;
DATA_SIZE t_save, t2_save;
DATA_SIZE blue_save, green_save, red_save, white_save;
DATA_SIZE pwm_save;
DATA_SIZE lux_save;
DATA_SIZE lum_save;
u08 hz_save, uw_save, pct_save;

DATA_SIZE d_save;
double o_save;
double p_save;

double  save_cs_pump;          
double  save_cs_emul;          
double  save_cs_volt_avg;         
double  save_pps_tie;          
double  save_osc_tie;          
double  save_chc_tie;          
double  save_chd_tie;          
double  save_cs_rfam1;         
double  save_cs_rfam2;         
double  save_cs_gain;          
double  save_cs_coven;         
double  save_cs_qoven;         
double  save_cs_pll_dro;       
double  save_cs_pll_saw;       
double  save_cs_pll_87;        
double  save_cs_pll_up;        
double  save_cs_beam;          
double  save_cs_cfield;        
double  save_cs_hwi;           
double  save_cs_msp;           
double  save_lpfrs_dd;         
double  save_lpfrs_gg;         
double  save_lpfrs_hh;         
double  save_lpfrs_ee;         
double  save_lpfrs_cc;         
double  save_lpfrs_bb;         
double  save_lpfrs_aa;         
double  save_lpfrs_ff;         
double  save_prs_ad5;        
double  save_prs_ad6;        
double  save_prs_ad7;        
double  save_prs_ad8;        
double  save_prs_ad9;        
double  save_prs_ad14;        
double  save_prs_ad_pwr;        
double  save_prs_ad_therm;        
double  save_prs_ds1;          
double  save_prs_ds2;          
double  save_sro_hh;           
double  save_sro_ff;           
double  save_sro_ee;           
double  save_sro_dd;           
double  save_sro_cc;           
double  save_sro_bb;           
double  save_sro_vt;           
double  save_sro_gg;           
double  save_sro_aa;           
double  save_sro_fc;           
double  save_pps_phase;        
double  save_osc_phase;        
double  save_chc_phase;        
double  save_chd_phase;        
double  save_x72_dmvoutc;      
double  save_x72_dlvoutc;      
double  save_x72_drvoutc;      
double  save_x72_dmv2demavg;   
double  save_x72_dlvolt;   
double  save_x72_dmp17;        
double  save_x72_dmp5;         
double  save_x72_pres;         
double  save_x72_plmp;         
double  save_x72_dhtrvolt;     
double  save_zyfer_hefe;       
double  save_zyfer_hete;       
double  save_zyfer_hest;       
double  save_zyfer_drift;      
double  save_zyfer_tdev;       
double  save_zyfer_essn;       
double  save_true_debug;       
double  save_true_eval;        
double  save_tfom;             
double  save_ffom;             
double  save_avg_phase;        
double  save_rftg_oscv;        
double  save_rftg_ant1;        
double  save_rftg_ant2;        
double  save_rftg_ma;          
//double  save_jitter;           
double  save_msg_ofs;          
double  save_speed;            
double  save_heading;          
double  save_datum;          
double  save_cha_time2;        
double  save_chb_time2;        

double  save_lat;         
double  save_lon;         
double  save_alt;         
double  save_freq_trend;
double  save_avg_dop;

double  save_lat_tide;         
double  save_lon_tide;         
double  save_alt_tide;         
double  save_ugals;            

double  save_humidity;         
double  save_pressure;        
double  save_temp1;
double  save_temp2;
double  save_adc1;
double  save_adc2;
double  save_adc3;
double  save_adc4;
double  save_lars_dac;
double  save_lars_pps;
double  save_lars_temp;




    // write queue entry "i"'s data to the log file
    if(queue_interval <= 0) return;
    if(file == 0) return;

    hh = hours;                // save readings of live incoming values
    mm = minutes; 
    ss = seconds; 
    frac = raw_frac;
    dd = day;     
    mo = month;   
    yy = year;    

    o_save = osc_offset;
    p_save = pps_offset;
    d_save = dac_voltage;
    t_save = temperature;

    tow_save = tow;
    lux2_save = lux2;
    ledi_save = led_i;
    ledv_save = led_v;
    adc2_save = adc2;
    t2_save = luxor_tc2;        
    count_save = sat_count;
    flag_save = time_flags;

    blue_save = blue_hz;         // color sensor readings
    green_save = green_hz;
    red_save = red_hz;
    white_save = white_hz;
    pwm_save = pwm_hz;

    lux_save = lux_scale;
    lum_save = lum_scale;
    lux_scale = 1.0F;
    lum_scale = 1.0F;

    pct_save = show_color_pct;
    uw_save = show_color_uw;
    hz_save = show_color_hz;
    show_color_hz = 1;           // always log Hz
    show_color_pct= 0;
    show_color_uw = 0;


    // save the current raw values       
    save_cs_pump =        cs_pump; 
    save_cs_emul =        cs_emul;          
    save_cs_volt_avg =    cs_volt_avg;          
    save_pps_tie =        pps_tie;          
    save_osc_tie =        osc_tie;          
    save_chc_tie =        chc_tie;          
    save_chd_tie =        chd_tie;          
    save_cs_rfam1 =       cs_rfam1;         
    save_cs_rfam2 =       cs_rfam2;         
    save_cs_gain =        cs_gain;          
    save_cs_coven =       cs_coven;         
    save_cs_qoven =       cs_qoven;         
    save_cs_pll_dro =     cs_pll_dro;       
    save_cs_pll_saw =     cs_pll_saw;       
    save_cs_pll_87 =      cs_pll_87;        
    save_cs_pll_up =      cs_pll_up;        
    save_cs_beam =        cs_beam;          
    save_cs_cfield =      cs_cfield;        
    save_cs_hwi =         cs_hwi;           
    save_cs_msp =         cs_msp;           
    save_lpfrs_dd =       lpfrs_dd;         
    save_lpfrs_gg =       lpfrs_gg;         
    save_lpfrs_hh =       lpfrs_hh;         
    save_lpfrs_ee =       lpfrs_ee;         
    save_lpfrs_cc =       lpfrs_cc;         
    save_lpfrs_bb =       lpfrs_bb;         
    save_lpfrs_aa =       lpfrs_aa;         
    save_lpfrs_ff =       lpfrs_ff;         
    save_prs_ad5 =        prs_ad[5];        
    save_prs_ad6 =        prs_ad[6];        
    save_prs_ad7 =        prs_ad[7];        
    save_prs_ad8 =        prs_ad[8];        
    save_prs_ad9 =        prs_ad[9];        
    save_prs_ad14 =       prs_ad[14];        
    save_prs_ad_pwr =     prs_ad_pwr;        
    save_prs_ad_therm =   prs_ad_therm;        
    save_prs_ds1 =        prs_ds1;          
    save_prs_ds2 =        prs_ds2;          
    save_sro_hh =         sro_hh;           
    save_sro_ff =         sro_ff;           
    save_sro_ee =         sro_ee;           
    save_sro_dd =         sro_dd;           
    save_sro_cc =         sro_cc;           
    save_sro_bb =         sro_bb;           
    save_sro_vt =         sro_vt;           
    save_sro_gg =         sro_gg;           
    save_sro_aa =         sro_aa;           
    save_sro_fc =         sro_fc;           
    save_pps_phase =      pps_phase;        
    save_osc_phase =      osc_phase;        
    save_chc_phase =      chc_phase;        
    save_chd_phase =      chd_phase;        
    save_x72_dmvoutc =    x72_dmvoutc;      
    save_x72_dlvoutc =    x72_dlvoutc;      
    save_x72_drvoutc =    x72_drvoutc;      
    save_x72_dmv2demavg = x72_dmv2demavg;   
    save_x72_dlvolt =     x72_dlvolt;   
    save_x72_dmp17 =      x72_dmp17;        
    save_x72_dmp5 =       x72_dmp5;         
    save_x72_pres =       x72_pres;         
    save_x72_plmp =       x72_plmp;         
    save_x72_dhtrvolt =   x72_dhtrvolt;     
    save_zyfer_hefe =     zyfer_hefe;       
    save_zyfer_hete =     zyfer_hete;       
    save_zyfer_hest =     zyfer_hest;       
    save_zyfer_drift =    zyfer_drift;      
    save_zyfer_tdev =     zyfer_tdev;       
    save_zyfer_essn =     zyfer_essn;       
    save_true_debug =     true_debug;       
    save_true_eval =      true_eval;        
    save_tfom =           tfom;             
    save_ffom =           ffom;             
    save_avg_phase =      avg_phase;        
    save_rftg_oscv =      rftg_oscv;        
    save_rftg_ant1 =      ant_v1;        
    save_rftg_ant2 =      ant_v2;        
    save_rftg_ma =        ant_ma;          
//  save_jitter =         jitter;           
    save_msg_ofs =        msg_ofs;          
    save_speed =          speed;            
    save_heading =        heading;          
    save_datum =          datum;
    save_cha_time2 =      cha_time2;        
    save_chb_time2 =      chb_time2;        

    save_lat =            lat;         
    save_lon =            lon;         
    save_alt =            alt;         
    save_freq_trend =     freq_trend;         
    save_avg_dop =        avg_dop;
                                        
    save_lat_tide =       lat_tide;         
    save_lon_tide =       lon_tide;         
    save_alt_tide =       alt_tide;         
    save_ugals =          ugals;            

    save_humidity =       humidity;         
    save_pressure =       pressure;        
    save_temp1 =          tc1;         
    save_temp2 =          tc2;            
    save_adc1 =           adc1;
    save_adc2 =           adc2;
    save_adc3 =           adc3;
    save_adc4 =           adc4;
    save_lars_dac =       lars_dac;
    save_lars_pps =       lars_pps;
    save_lars_temp =      lars_temp;

    
    if(filter_log && filter_count) q = filter_plot_q(i);
    else                           q = get_plot_q(i);

    // regenerate the raw values from the plot queue entries so that
    // write_log_readings() writes the queued values
    cs_pump =           (DATA_SIZE)  q.data[PPS] / (DATA_SIZE) queue_interval;     
    cs_emul =           (DATA_SIZE)  q.data[OSC] / (DATA_SIZE) queue_interval;     
    pps_tie =           (DATA_SIZE)  q.data[PPS] / (DATA_SIZE) queue_interval;     
    osc_tie =           (DATA_SIZE)  q.data[OSC] / (DATA_SIZE) queue_interval;     
    chc_tie =           (DATA_SIZE)  q.data[CHC] / (DATA_SIZE) queue_interval;     
    chd_tie =           (DATA_SIZE)  q.data[CHD] / (DATA_SIZE) queue_interval;     
    cs_rfam1 =          (DATA_SIZE)  q.data[ONE] / (DATA_SIZE) queue_interval;     
    cs_rfam2 =          (DATA_SIZE)  q.data[TWO] / (DATA_SIZE) queue_interval;     
    cs_gain =           (DATA_SIZE)  q.data[THREE] / (DATA_SIZE) queue_interval;   
    cs_coven =          (DATA_SIZE)  q.data[FOUR] / (DATA_SIZE) queue_interval;    
    cs_qoven =          (DATA_SIZE)  q.data[FIVE] / (DATA_SIZE) queue_interval;    
    cs_pll_dro =        (DATA_SIZE)  q.data[SIX] / (DATA_SIZE) queue_interval;     
    cs_pll_saw =        (DATA_SIZE)  q.data[SEVEN] / (DATA_SIZE) queue_interval;   
    cs_pll_87 =         (DATA_SIZE)  q.data[EIGHT] / (DATA_SIZE) queue_interval;   
    cs_pll_up =         (DATA_SIZE)  q.data[NINE] / (DATA_SIZE) queue_interval;    
    cs_beam =           (DATA_SIZE)  q.data[TEN] / (DATA_SIZE) queue_interval;     
    cs_cfield =         (DATA_SIZE)  q.data[ELEVEN] / (DATA_SIZE) queue_interval;  
    cs_hwi =            (DATA_SIZE)  q.data[TWELVE] / (DATA_SIZE) queue_interval;  
    cs_msp =            (DATA_SIZE)  q.data[THIRTEEN] / (DATA_SIZE) queue_interval;
    cs_volt_avg =       (DATA_SIZE)  q.data[FOURTEEN] / (DATA_SIZE) queue_interval;
    lpfrs_dd =          (DATA_SIZE)  q.data[ONE] / (DATA_SIZE) queue_interval;     
    lpfrs_gg =          (DATA_SIZE)  q.data[TWO] / (DATA_SIZE) queue_interval;     
    lpfrs_hh =          (DATA_SIZE)  q.data[THREE] / (DATA_SIZE) queue_interval;   
    lpfrs_ee =          (DATA_SIZE)  q.data[FOUR] / (DATA_SIZE) queue_interval;    
    lpfrs_cc =          (DATA_SIZE)  q.data[FIVE] / (DATA_SIZE) queue_interval;    
    lpfrs_bb =          (DATA_SIZE)  q.data[SIX] / (DATA_SIZE) queue_interval;     
    lpfrs_aa =          (DATA_SIZE)  q.data[SEVEN] / (DATA_SIZE) queue_interval;   
    lpfrs_ff =          (DATA_SIZE)  q.data[EIGHT] / (DATA_SIZE) queue_interval;   
    prs_ad[5] =         (DATA_SIZE)  q.data[ONE] / (DATA_SIZE) queue_interval;     
    prs_ad[6] =         (DATA_SIZE)  q.data[TWO] / (DATA_SIZE) queue_interval;     
    prs_ad[7] =         (DATA_SIZE)  q.data[THREE] / (DATA_SIZE) queue_interval;   
    prs_ad[8] =         (DATA_SIZE)  q.data[EIGHT] / (DATA_SIZE) queue_interval;   
    prs_ad[9] =         (DATA_SIZE)  q.data[SEVEN] / (DATA_SIZE) queue_interval;   
    prs_ad[14] =        (DATA_SIZE)  q.data[FOUR] / (DATA_SIZE) queue_interval;    
    prs_ad_pwr =        (DATA_SIZE)  q.data[SIX] / (DATA_SIZE) queue_interval;    
    prs_ad_therm =      (DATA_SIZE)  q.data[FIVE] / (DATA_SIZE) queue_interval;    
    prs_ds1 =           (int)  (q.data[NINE] / (DATA_SIZE) queue_interval);    
    prs_ds2 =           (int)  (q.data[TEN] / (DATA_SIZE) queue_interval);     
    sro_hh =            (DATA_SIZE)  q.data[ONE] / (DATA_SIZE) queue_interval;     
    sro_ff =            (DATA_SIZE)  q.data[TWO] / (DATA_SIZE) queue_interval;     
    sro_ee =            (DATA_SIZE)  q.data[THREE] / (DATA_SIZE) queue_interval;   
    sro_dd =            (DATA_SIZE)  q.data[FOUR] / (DATA_SIZE) queue_interval;    
    sro_cc =            (DATA_SIZE)  q.data[FIVE] / (DATA_SIZE) queue_interval;    
    sro_bb =            (DATA_SIZE)  q.data[SIX] / (DATA_SIZE) queue_interval;     
    sro_vt =            (int)  (q.data[SEVEN] / (DATA_SIZE) queue_interval);   
    sro_gg =            (DATA_SIZE)  q.data[EIGHT] / (DATA_SIZE) queue_interval;   
    sro_aa =            (DATA_SIZE)  q.data[NINE] / (DATA_SIZE) queue_interval;    
    pps_phase =         (DATA_SIZE)  q.data[ONE] / (DATA_SIZE) queue_interval;     
    osc_phase =         (DATA_SIZE)  q.data[TWO] / (DATA_SIZE) queue_interval;     
    chc_phase =         (DATA_SIZE)  q.data[THREE] / (DATA_SIZE) queue_interval;   
    chd_phase =         (DATA_SIZE)  q.data[FOUR] / (DATA_SIZE) queue_interval;    
    x72_dmvoutc =       (DATA_SIZE)  q.data[ONE] / (DATA_SIZE) queue_interval;     
    x72_dlvoutc =       (DATA_SIZE)  q.data[TWO] / (DATA_SIZE) queue_interval;     
    x72_drvoutc =       (DATA_SIZE)  q.data[THREE] / (DATA_SIZE) queue_interval;   
    x72_dmv2demavg =    (DATA_SIZE)  q.data[FOUR] / (DATA_SIZE) queue_interval;    
    x72_dlvolt =        (DATA_SIZE)  q.data[FIVE] / (DATA_SIZE) queue_interval;    
    x72_dmp17 =         (DATA_SIZE)  q.data[SIX] / (DATA_SIZE) queue_interval;     
    x72_dmp5 =          (DATA_SIZE)  q.data[SEVEN] / (DATA_SIZE) queue_interval;   
    x72_pres =          (DATA_SIZE)  q.data[EIGHT] / (DATA_SIZE) queue_interval;   
    x72_plmp =          (DATA_SIZE)  q.data[NINE] / (DATA_SIZE) queue_interval;    
    x72_dhtrvolt =      (DATA_SIZE)  q.data[TEN] / (DATA_SIZE) queue_interval;     
    zyfer_hefe =        (DATA_SIZE)  q.data[ONE] / (DATA_SIZE) queue_interval;     
    zyfer_hete =        (DATA_SIZE)  q.data[TWO] / (DATA_SIZE) queue_interval;     
    zyfer_hest =        (DATA_SIZE)  q.data[THREE] / (DATA_SIZE) queue_interval;   
    zyfer_drift =       (DATA_SIZE)  q.data[FIVE] / (DATA_SIZE) queue_interval;    
    zyfer_tdev =        (DATA_SIZE)  q.data[SEVEN] / (DATA_SIZE) queue_interval;   
    zyfer_essn =        (DATA_SIZE)  q.data[EIGHT] / (DATA_SIZE) queue_interval;   
    true_debug =        (int)  (q.data[FIVE] / (DATA_SIZE) queue_interval);    
    true_eval =         (DATA_SIZE)  q.data[EIGHT] / (DATA_SIZE) queue_interval;   
    tfom =              (int)  (q.data[FOUR] / (DATA_SIZE) queue_interval);    
    ffom =              (int)  (q.data[FIVE] / (DATA_SIZE) queue_interval);    
    avg_phase =         (DATA_SIZE)  q.data[EIGHT] / (DATA_SIZE) queue_interval;   
    rftg_oscv =         (DATA_SIZE)  q.data[FIVE] / (DATA_SIZE) queue_interval;    
    ant_v1 =            (DATA_SIZE)  q.data[SEVEN] / (DATA_SIZE) queue_interval;   
    ant_v2 =            (DATA_SIZE)  q.data[EIGHT] / (DATA_SIZE) queue_interval;   
    ant_ma =            (DATA_SIZE)  q.data[SIX] / (DATA_SIZE) queue_interval;     
//  jitter =            (DATA_SIZE)  q.data[MSGJIT] / (DATA_SIZE) queue_interval;  
    msg_ofs =           (DATA_SIZE)  q.data[MSGOFS] / (DATA_SIZE) queue_interval;  
    speed =             (DATA_SIZE)  q.data[FOUR] / (DATA_SIZE) queue_interval;    
    heading =           (DATA_SIZE)  q.data[FIVE] / (DATA_SIZE) queue_interval;    
    cha_time2 =         (DATA_SIZE)  q.data[FIVE] / (DATA_SIZE) queue_interval;    
    chb_time2 =         (DATA_SIZE)  q.data[SIX] / (DATA_SIZE) queue_interval;     
                                                     
    lat_tide =          (DATA_SIZE)  q.data[ELEVEN] / (DATA_SIZE) queue_interval;  
    lon_tide =          (DATA_SIZE)  q.data[TWELVE] / (DATA_SIZE) queue_interval;  
    alt_tide =          (DATA_SIZE)  q.data[THIRTEEN] / (DATA_SIZE) queue_interval;
    ugals =             (DATA_SIZE)  q.data[FOURTEEN] / (DATA_SIZE) queue_interval;
                                                     
    humidity =          (DATA_SIZE)  q.data[HUMIDITY] / (DATA_SIZE) queue_interval;  
    pressure =          (DATA_SIZE)  q.data[PRESSURE] / (DATA_SIZE) queue_interval;  
    tc1 =               (DATA_SIZE)  q.data[TEMP1] / (DATA_SIZE) queue_interval;
    tc2 =               (DATA_SIZE)  q.data[TEMP2] / (DATA_SIZE) queue_interval;
    adc3 =              (DATA_SIZE)  q.data[ADC3] / (DATA_SIZE) queue_interval;
    adc4 =              (DATA_SIZE)  q.data[ADC4] / (DATA_SIZE) queue_interval;

    // !!!! these queue entries have values that are scaled from the raw data values... unscale them
    lat =               (DATA_SIZE)  (q.data[ONE] / (DATA_SIZE) queue_interval / (DATA_SIZE) RAD_TO_DEG);  
    lon =               (DATA_SIZE)  (q.data[TWO] / (DATA_SIZE) queue_interval / (DATA_SIZE) RAD_TO_DEG);
    alt =               (DATA_SIZE)  (q.data[THREE] / (DATA_SIZE) queue_interval);
    freq_trend =        (DATA_SIZE)  (q.data[SEVEN] / (DATA_SIZE) queue_interval / (DATA_SIZE) 1.0E12);
    sro_fc =            (int)        (q.data[OSC] / (DATA_SIZE) queue_interval / (SRO_DDS_STEP*1.0E12/1000.0));
    avg_dop =           (DATA_SIZE)  (q.data[SIX] / (DATA_SIZE) queue_interval / (DATA_SIZE) 1.0E12);
    lars_dac =          (DATA_SIZE)  (q.data[DAC] / (DATA_SIZE) queue_interval);
    lars_pps =          (DATA_SIZE)  (q.data[PPS] / (DATA_SIZE) queue_interval);
    lars_temp =         (DATA_SIZE)  (q.data[TEMP] / (DATA_SIZE) queue_interval);


    gregorian(0, q.q_jd);

    hours    = g_hours;
    minutes  = g_minutes;
    seconds  = g_seconds;
    raw_frac = g_frac;
    day      = g_day;
    month    = g_month;
    year     = g_year;
       
    tow = fake_tow(q.q_jd);  // % (7L*24L*60L*60L); 

    if(luxor) {
       osc_offset  = (OFS_SIZE) q.data[BATTI] / (OFS_SIZE) queue_interval;
       pps_offset  = (OFS_SIZE) q.data[LUX1] / (OFS_SIZE) queue_interval;
       dac_voltage = (DATA_SIZE) q.data[BATTV] / (DATA_SIZE) queue_interval;
    }
    else {
       osc_offset  = (OFS_SIZE) q.data[OSC] / (OFS_SIZE) queue_interval;
       pps_offset  = (OFS_SIZE) q.data[PPS] / (OFS_SIZE) queue_interval;
       dac_voltage = (DATA_SIZE) q.data[DAC] / (DATA_SIZE) queue_interval;
    }
    temperature = (DATA_SIZE) q.data[TEMP] / (DATA_SIZE) queue_interval;
    luxor_tc2   = (DATA_SIZE) q.data[TC2] / (DATA_SIZE) queue_interval;
    lux2        = (DATA_SIZE) q.data[LUX2] / (DATA_SIZE) queue_interval;
    led_v       = (DATA_SIZE) q.data[LEDV] / (DATA_SIZE) queue_interval;
    led_i       = (DATA_SIZE) q.data[LEDI] / (DATA_SIZE) queue_interval;
    pwm_hz      = (DATA_SIZE) q.data[PWMHZ] / (DATA_SIZE) queue_interval;
    blue_hz     = (DATA_SIZE) q.data[BLUEHZ] / (DATA_SIZE) queue_interval;
    green_hz    = (DATA_SIZE) q.data[GREENHZ] / (DATA_SIZE) queue_interval;
    red_hz      = (DATA_SIZE) q.data[REDHZ] / (DATA_SIZE) queue_interval;
    white_hz    = (DATA_SIZE) q.data[WHITEHZ] / (DATA_SIZE) queue_interval;
    adc2        = (DATA_SIZE) q.data[AUXV] / (DATA_SIZE) queue_interval;

    sat_count = q.sat_flags & SAT_COUNT_MASK;
    if(q.sat_flags & UTC_TIME) time_flags = TFLAGS_UTC;
    else                       time_flags = TFLAGS_NULL;

#ifdef SCI_LOG
if(sci_file == 0) sci_file = topen(SCI_LOG, "w");
if(sci_file) fprintf(sci_file, "%ld %12.6f %10.6f %10.6f\n", nnn, osc_offset*100.0, temperature, dac_voltage);
nnn += queue_interval;
#endif

    if(file && log_comments) {
       for(j=0; j<MAX_MARKER; j++) {
          if(i && (mark_q_entry[j] == i)) {
             if(log_fmt == HEATHER) fprintf(file, "#MARKER %d\n", j);
             else                   fprintf(file, "<!-- #MARKER %d -->\n", j);
          }
       }
    }
    
    write_log_readings(file, i);

    hours       = hh;          // restore saved values
    minutes     = mm;
    seconds     = ss;
    raw_frac    = frac;
    day         = dd;
    month       = mo;
    year        = yy;
    tow         = tow_save;

    // restore the raw values
    osc_offset  = o_save; 
    pps_offset  = p_save;
    dac_voltage = d_save; 
    temperature = t_save; 
    luxor_tc2   = t2_save;

    sat_count   = count_save;
    time_flags  = flag_save;
    lux2        = lux2_save;

    led_i       = ledi_save;
    led_v       = ledv_save;
    adc2        = adc2_save;
    blue_hz     = blue_save;
    green_hz    = green_save;
    red_hz      = red_save;
    white_hz    = white_save;
    pwm_hz      = pwm_save;
    lux_scale   = lux_save;
    lum_scale   = lum_save;
    show_color_pct = pct_save;
    show_color_uw = uw_save; 
    show_color_hz = hz_save; 


    cs_pump =        save_cs_pump;          
    cs_emul =        save_cs_emul;          
    cs_volt_avg =    save_cs_volt_avg;          
    pps_tie =        save_pps_tie;          
    osc_tie =        save_osc_tie;          
    chc_tie =        save_chc_tie;          
    chd_tie =        save_chd_tie;          
    cs_rfam1 =       save_cs_rfam1;         
    cs_rfam2 =       save_cs_rfam2;         
    cs_gain =        save_cs_gain;          
    cs_coven =       save_cs_coven;         
    cs_qoven =       save_cs_qoven;         
    cs_pll_dro =     save_cs_pll_dro;       
    cs_pll_saw =     save_cs_pll_saw;       
    cs_pll_87 =      save_cs_pll_87;        
    cs_pll_up =      save_cs_pll_up;        
    cs_beam =        save_cs_beam;          
    cs_cfield =      save_cs_cfield;        
    cs_hwi =         save_cs_hwi;           
    cs_msp =         save_cs_msp;           
    lpfrs_dd =       save_lpfrs_dd;         
    lpfrs_gg =       save_lpfrs_gg;         
    lpfrs_hh =       save_lpfrs_hh;         
    lpfrs_ee =       save_lpfrs_ee;         
    lpfrs_cc =       save_lpfrs_cc;         
    lpfrs_bb =       save_lpfrs_bb;         
    lpfrs_aa =       save_lpfrs_aa;         
    lpfrs_ff =       save_lpfrs_ff;         
    prs_ad[5] =      save_prs_ad5;        
    prs_ad[6] =      save_prs_ad6;        
    prs_ad[7] =      save_prs_ad7;        
    prs_ad[8] =      save_prs_ad8;        
    prs_ad[9] =      save_prs_ad9;        
    prs_ad[14] =     save_prs_ad14;        
    prs_ad_pwr =     save_prs_ad_pwr;        
    prs_ad_therm =   save_prs_ad_therm;        
    prs_ds1 =        (int) save_prs_ds1;          
    prs_ds2 =        (int) save_prs_ds2;          
    sro_hh =         save_sro_hh;           
    sro_ff =         save_sro_ff;           
    sro_ee =         save_sro_ee;           
    sro_dd =         save_sro_dd;           
    sro_cc =         save_sro_cc;           
    sro_bb =         save_sro_bb;           
    sro_vt =         (int) save_sro_vt;           
    sro_gg =         save_sro_gg;           
    sro_aa =         save_sro_aa;           
    pps_phase =      save_pps_phase;        
    osc_phase =      save_osc_phase;        
    chc_phase =      save_chc_phase;        
    chd_phase =      save_chd_phase;        
    x72_dmvoutc =    save_x72_dmvoutc;      
    x72_dlvoutc =    save_x72_dlvoutc;      
    x72_drvoutc =    save_x72_drvoutc;      
    x72_dmv2demavg = save_x72_dmv2demavg;   
    x72_dlvolt =     save_x72_dlvolt;   
    x72_dmp17 =      save_x72_dmp17;        
    x72_dmp5 =       save_x72_dmp5;         
    x72_pres =       save_x72_pres;         
    x72_plmp =       save_x72_plmp;         
    x72_dhtrvolt =   save_x72_dhtrvolt;     
    zyfer_hefe =     save_zyfer_hefe;       
    zyfer_hete =     save_zyfer_hete;       
    zyfer_hest =     save_zyfer_hest;       
    zyfer_drift =    (DATA_SIZE) save_zyfer_drift;      
    zyfer_tdev =     (DATA_SIZE) save_zyfer_tdev;       
    zyfer_essn =     (DATA_SIZE) save_zyfer_essn;       
    true_debug =     (int) save_true_debug;       
    true_eval =      save_true_eval;        
    tfom =           (int) save_tfom;             
    ffom =           (int) save_ffom;             
    avg_phase =      save_avg_phase;        
    rftg_oscv =      save_rftg_oscv;        
    ant_v1 =         save_rftg_ant1;        
    ant_v2 =         save_rftg_ant2;        
    ant_ma =         save_rftg_ma;          
//  jitter =         save_jitter;           
    msg_ofs =        save_msg_ofs;          
    speed =          save_speed;            
    heading =        save_heading;          
    datum =          (int) save_datum;          
    cha_time2 =      save_cha_time2;        
    chb_time2 =      save_chb_time2;        
                                        
    lat_tide =       save_lat_tide;         
    lon_tide =       save_lon_tide;         
    alt_tide =       save_alt_tide;         
    ugals =          save_ugals;            

    humidity =       (DATA_SIZE) save_humidity;         
    pressure =       (DATA_SIZE) save_pressure;        
    tc1 =            (DATA_SIZE) save_temp1;         
    tc2 =            (DATA_SIZE) save_temp2;            
    adc1 =           (DATA_SIZE) save_adc1;            
    adc2 =           (DATA_SIZE) save_adc2;            
    adc3 =           (DATA_SIZE) save_adc3;            
    adc4 =           (DATA_SIZE) save_adc4;            

    lars_dac =       (DATA_SIZE) save_lars_dac;
    lars_pps =       (DATA_SIZE) save_lars_pps;
    lars_temp =      (DATA_SIZE) save_lars_temp;

    lat =            save_lat;         
    lon =            save_lon;         
    alt =            save_alt;         
    freq_trend =     save_freq_trend;         
    avg_dop =        (DATA_SIZE) save_avg_dop;         
    sro_fc =         (int) save_sro_fc;           
}

void log_stats()
{
int old_stat;
DATA_SIZE val;
int id;
char units[SLEN+1];
char *s;
int old_interval;
int old_col0;
int old_last_col;

   // write plot queue statistics (of the plot window when the log file was
   // closed) when the log file is closed

   if(log_file == 0) return;
   if(log_comments == 0) return;

if(1) {
   old_interval = view_interval;   // don't downsample the queue data
   old_col0 = plot_q_col0;
   old_last_col = plot_q_last_col;

   view_interval = 1;
   plot_q_col0 = plot_q_out;
   calc_queue_stats(STOP_AT_Q_END);

   view_interval = old_interval;
   plot_q_col0 = old_col0;
   plot_q_last_col = old_last_col;
}

   sprintf(log_text, "#");
   write_log_comment(1);
   write_log_comment(1);
   write_log_comment(1);
   sprintf(log_text,    "#  Plot statistics:");
   write_log_comment(1);

   for(id=0; id<=FIFTEEN; id++) {
      plot[id].show_stat = SHOW_SPAN;
      val = (DATA_SIZE) fabs(get_stat_val(id));
      plot[id].show_stat = SDEV;
      val += (DATA_SIZE) fabs(get_stat_val(id));
      plot[id].show_stat = RMS;
      val += (DATA_SIZE) fabs(get_stat_val(id));
      if(val == 0.0) continue;    // plot has no data

      sprintf(log_text, "#");
      write_log_comment(1);
      sprintf(log_text, "#     %s:", plot[id].plot_id);
      write_log_comment(1);
      strcpy(units, plot[id].plot_id);
      strupr(units);
      if((plot_loc == 0) && (!strcmp(units, "LAT") || !strcmp(units, "LON"))) {
         sprintf(log_text, "#       (private)");
         write_log_comment(1);
         continue;
      }
      
      old_stat = plot[id].show_stat;
      strcpy(units, plot[id].units);
      s = strchr(units, DEGREES);
      if(s) *s = ' ';

      plot[id].show_stat = RMS;
      val = get_stat_val(id);
      sprintf(log_text, "#       %s %.9f %s", stat_id, val, units);
      write_log_comment(1);

      plot[id].show_stat = AVG;
      val = get_stat_val(id);
      sprintf(log_text, "#       %s %.9f %s", stat_id, val, units);
      write_log_comment(1);

      plot[id].show_stat = SDEV;
      val = get_stat_val(id);
      sprintf(log_text, "#       %s %.9f %s", stat_id, val, units);
      write_log_comment(1);

      plot[id].show_stat = VAR;
      val = get_stat_val(id);
      sprintf(log_text, "#       %s %.9f %s", stat_id, val, units);
      write_log_comment(1);

      plot[id].show_stat = SHOW_MIN;
      val = get_stat_val(id);
      sprintf(log_text, "#       %s %.9f %s", stat_id, val, units);
      write_log_comment(1);

      plot[id].show_stat = SHOW_MAX;
      val = get_stat_val(id);
      sprintf(log_text, "#       %s %.9f %s", stat_id, val, units);
      write_log_comment(1);

      plot[id].show_stat = SHOW_SPAN;
      val = get_stat_val(id);
      sprintf(log_text, "#       %s %.9f %s", stat_id, val, units);
      write_log_comment(1);

      plot[id].show_stat = old_stat;
   }

   sprintf(log_text, "#");
   write_log_comment(1);
}


void dump_log(char *name, u08 dump_size)
{
FILE *file;
FILE *lock_file;
char temp_name[128];
long temp_interval;
u08 temp_flags;
u08 temp_pause;
u08 temp_info;
u08 temp_written;
FILE *temp_file;
int temp_fmt;
long i;
long counter;
long val;
int row;
char *s;
char filter[32];

   if(queue_interval <= 0) return;

   doing_log_dump = 1;
   lock_file = topen(lock_name, "w");  // create file tblock to signify dump file is being written
   if(lock_file) {
      fprintf(lock_file, "2");
      fclose(lock_file);
   }

   row = PLOT_TEXT_ROW+4;
   if(dump_size == 'p') s = "plot area";
   else                 s = "queue";

   filter[0] = 0;
   if(filter_log && filter_count) sprintf(filter, "%ld point filtered ", disp_filter_size()); 

   temp_interval = log_interval;
   temp_flags = time_flags;
   temp_pause = pause_data;
   temp_info = have_info;
   temp_written = log_written;
   temp_fmt = log_fmt;
   temp_file = log_file;

   log_written = 0;
   strcpy(temp_name, log_name);

   log_interval = queue_interval;
   strcpy(log_name, name);
   if(!strstr(log_name, ".")) strcat(log_name, ".log");

   if     (strstr(log_name, ".xml")) log_fmt = XML;
   else if(strstr(log_name, ".XML")) log_fmt = XML;
   else if(strstr(log_name, ".gpx")) log_fmt = GPX;
   else if(strstr(log_name, ".GPX")) log_fmt = GPX;
   else if(strstr(log_name, ".kml")) log_fmt = KML;
   else if(strstr(log_name, ".KML")) log_fmt = KML;
//   else if(strstr(log_name, ".obs")) log_fmt = RINEX;  // cannot write RINEX file from queue data
//   else if(strstr(log_name, ".OBS")) log_fmt = RINEX;
   else log_fmt = HEATHER;

   erase_help();
   if(log_fmt != HEATHER) {
      log_file = 0;
      file = open_log_file(log_mode);
   }
   else {
      file = topen(log_name, log_mode);
   }

   if(file == 0) {
      sprintf(out, "Cannot dump log file: %s", log_name);
      edit_error(out);
      goto dump_exit;
   }

   log_file = file;
write_log_header(file, 2);
   if(log_comments) {
      sprintf(log_text, "#TITLE: From log file %s (%s%s data)", log_name, filter, s);
      write_log_comment(1);

      if((luxor == 0) && have_gain) {
         sprintf(log_text, "#OSC_GAIN %f", osc_gain);
         write_log_comment(1);
      }
   }

if(log_fmt != HEATHER) {
   log_file = 0;
}

   if(log_mode[0] == 'a') sprintf(out, "Appending %s%s data to file: %s", filter, s, log_name);
   else                   sprintf(out, "Writing %s%s data to file: %s", filter, s, log_name);
   vidstr(row, PLOT_TEXT_COL, PROMPT_COLOR, out);

   pause_data = 1;
   counter = 0;
   if(dump_size == 'p') {
      i = plot_q_col0;  // dumping the plot area's data
   }
   else {
      i = plot_q_out;   // dumping the full queue
//plot_q_col0 = plot_q_out;

   }
   while((i != plot_q_in) || (dump_size == 'p')) {
      write_q_entry(file, i);
      have_info |= INFO_LOGGED;
      update_pwm();   // if doing pwm temperature control

      ++counter;
      if((counter % LOG_GPS_CHECK) == 0) {   // keep serial data from overruning
         get_pending_gps(10);  //!!!! possible recursion
      }
//    if((counter % 1000L) == 1L) {
      if((counter % LOG_SCREEN_RFSH) == 1L) {
         sprintf(out, "Line %ld", counter-1L);
         vidstr(row+2, PLOT_TEXT_COL, PROMPT_COLOR, out);
         refresh_page();
      }

      if(dump_size == 'p') {
         val = view_interval * (long) PLOT_WIDTH;
         val /= (long) plot_mag;
         if(counter >= val) break;
         if(counter >= (plot_q_count-1)) break;
      }
      if(++i >= plot_q_size) i = 0;
   }

   #ifdef SCI_LOG
      if(sci_file) fclose(sci_file);
      sci_file = 0;
   #endif

   log_file = file;
   #ifdef ADEV_STUFF
      log_adevs();
   #endif

   log_stats();

   if(log_fmt != HEATHER) {
      close_log_file();
   }
   else {
      fclose(file);      // close this log and restore original one
   }

   dump_exit:
   log_interval = temp_interval;
   log_written = temp_written;
   time_flags = temp_flags;
   pause_data = temp_pause;
   have_info = temp_info;
have_info |= INFO_LOGGED;
   log_fmt = temp_fmt;
   log_file = temp_file;
   strcpy(log_name, temp_name);
   show_log_state();

   doing_log_dump = 0;
   path_unlink(lock_name);
}

void write_log_tow(int spaces)
{
   
   sprintf(out, ": at tow %u", tow);
   strcat(log_text, out);
   write_log_comment(spaces);
}

void write_log_changes()
{
int change;

   // write receiver state changes to the log file

   if(log_file == 0) return;
   if(log_errors == 0) return;

   if(rcvr_type == CS_RCVR) ;
   else if(NO_SATS) ;
   else if(log_comments && (rcvr_mode != last_rmode) && (check_precise_posn == 0)) {
      sprintf(log_text, "#! new reciever mode: ");

      if(rcvr_type == ZODIAC_RCVR) {
         if(single_sat_prn)                     strcat(log_text, "Single sat mode");
         else if(rcvr_mode == RCVR_MODE_3D)     strcat(log_text, "Navigation mode");
         else if(rcvr_mode == RCVR_MODE_HOLD)   strcat(log_text, "Position hold mode");
         else if(rcvr_mode == RCVR_MODE_SURVEY) strcat(log_text, "Survey mode");
         else {
            sprintf(out, "Receiver mode ?%02X?", rcvr_mode);
            strcat(log_text, out);
         }
      }
      else if(single_sat_prn)                 strcat(log_text, "Single sat mode");
      else if(rcvr_mode == RCVR_MODE_2D_3D)   strcat(log_text, "2D/3D positioning");
      else if(rcvr_mode == RCVR_MODE_SINGLE)  strcat(log_text, "Single satellite");
      else if(rcvr_mode == RCVR_MODE_NO_SATS) strcat(log_text, "No sats");
      else if(rcvr_mode == RCVR_MODE_2D)      strcat(log_text, "2D positioning");
      else if(rcvr_mode == RCVR_MODE_3D)      strcat(log_text, "3D positioning");
      else if(rcvr_mode == RCVR_MODE_DGPS)    strcat(log_text, "DGPS reference");
      else if(rcvr_mode == RCVR_MODE_2DCLK)   strcat(log_text, "2D clock hold");
      else if(rcvr_mode == RCVR_MODE_HOLD)  {
         if(rcvr_type == TSIP_RCVR) strcat(log_text, "Overdetermined clock"); 
         else                       strcat(log_text, "Position hold mode");
      }
      else if(rcvr_mode == RCVR_MODE_PROP)  {
         if((rcvr_type == SCPI_RCVR) || (rcvr_type == UCCM_RCVR)) {
            strcat(log_text, "Holdover mode");
         }
         else {
            strcat(log_text, "Propogate mode");
         }
      }
      else if(rcvr_mode == RCVR_MODE_ACQUIRE)  strcat(log_text, "Acquiring sats");
      else if(rcvr_mode == RCVR_MODE_BAD_GEOM) strcat(log_text, "Bad sat geometry");
      else if(rcvr_mode == RCVR_MODE_SURVEY)   strcat(log_text, "Survey mode");
      else {
         sprintf(out, "Receiver mode ?%02X?", rcvr_mode);
         strcat(log_text, out);
      }

      write_log_tow(1);
   }

   last_rmode = rcvr_mode;


   if(log_comments && (gps_status != last_status) && (check_precise_posn == 0)) {
      sprintf(log_text,"#! new gps status: ");
      if     (gps_status == GPS_FIXES)       strcat(log_text, "Doing fixes");
      else if(gps_status == GPS_NO_TIME)     strcat(log_text, "No GPS time");
      else if(gps_status == GPS_PDOP_HIGH)   strcat(log_text, "PDOP too high");
      else if(gps_status == GPS_NO_SATS)     strcat(log_text, "No usable sats");
      else if(gps_status == GPS_ONE_SAT)     strcat(log_text, "1 usable sat");
      else if(gps_status == GPS_TWO_SATS)    strcat(log_text, "2 usable sats");
      else if(gps_status == GPS_THREE_SATS)  strcat(log_text, "3 usable sats");
      else if(gps_status == GPS_SAT_UNAVAIL) strcat(log_text, "chosen sat unusable");
      else if(gps_status == GPS_TRAIM_ERR)   strcat(log_text, "TRAIM rejected fix");
      else {
         sprintf(out, "?%02X?", gps_status);
         strcat(log_text, out);                  
      }
      write_log_tow(1);
   }
   last_status = gps_status;


   if(log_comments && (discipline_mode != last_dmode) && (check_precise_posn == 0)) { 
      // !!!!!!!! if you change any of this text,  you must also change reload_log();
      sprintf(log_text,"#! new discipline mode: ");
      if     (discipline_mode == DIS_MODE_NORMAL)        strcat(log_text, "Normal");
      else if(discipline_mode == DIS_MODE_POWERUP)       strcat(log_text, "Power-up");
      else if(discipline_mode == DIS_MODE_AUTO_HOLD)     strcat(log_text, "Auto holdover");
      else if(discipline_mode == DIS_MODE_MANUAL_HOLD)   strcat(log_text, "Manual holdover");
      else if(discipline_mode == DIS_MODE_RECOVERY)      strcat(log_text, "Recovery mode");
      else if(discipline_mode == DIS_MODE_FAST_RECOVERY) strcat(log_text, "Fast recovery");
      else if(discipline_mode == DIS_MODE_DISABLED)      strcat(log_text, "Disabled");
      else if(discipline_mode == DIS_MODE_LEARNING)      strcat(log_text, "Learning");
      else if(discipline_mode == DIS_MODE_NO_GPS)        strcat(log_text, "No GPS");
      else if(discipline_mode == DIS_MODE_SOFT_HOLD)     strcat(log_text, "Soft holdover");
      else if(discipline_mode == DIS_MODE_ACQUIRE)       strcat(log_text, "Acquire");
      else if(discipline_mode == DIS_MODE_WAIT)          strcat(log_text, "Wait");
      else if(discipline_mode == DIS_MODE_UNLOCK)        strcat(log_text, "Unlocked");
      else if(discipline_mode == DIS_MODE_UNKNOWN)       strcat(log_text, "Unknown");
      else if(discipline_mode == DIS_MODE_WARMUP) {
         if (saw_uccm_dmode == 1) strcat(log_text, "Settling  ");
         else                     strcat(log_text, "Warming up");
      }
      else if(discipline_mode == DIS_MODE_FAILED) {
         if     (rcvr_type == LPFRS_RCVR) strcat(log_text, "RB unlocked");
         else if(rcvr_type == RFTG_RCVR)  strcat(log_text, "Module failed");
         else if(rcvr_type == SA35_RCVR)  strcat(log_text, "RB unlocked");
         else if(rcvr_type == SRO_RCVR)   strcat(log_text, "RB unlocked");
         else                             strcat(log_text, "Module failed");
      }
      else {
         sprintf(out, "?%u?", discipline_mode);
         strcat(log_text, out);
      }
      write_log_tow(1);
   }
   last_dmode = discipline_mode;


   if(log_comments && (discipline != last_discipline) && (check_precise_posn == 0)) { 
      sprintf(log_text,"#! new discipline state: ");
      if     (discipline == DIS_LOCKED)   strcat(log_text, "Phase locking");
      else if(discipline == DIS_WARMING)  strcat(log_text, "Warming up");
      else if(discipline == DIS_FLOCK)    strcat(log_text, "Frequency locking");
      else if(discipline == DIS_PLACING)  strcat(log_text, "Placing PPS");
      else if(discipline == DIS_FINIT)    strcat(log_text, "Initializing loop filter");
      else if(discipline == DIS_COMP)     strcat(log_text, "Compensating OCXO");
      else if(discipline == DIS_OFF)      strcat(log_text, "Inactive");
      else if(discipline == DIS_RECOVERY) strcat(log_text, "Recovery");
      else {
         sprintf(out, "?%02X?", discipline);
         strcat(log_text, out);
      }
      write_log_tow(1);
   }
   last_discipline = discipline;


   if(log_comments && (timing_mode != last_tmode)) { 
      sprintf(log_text,"#! new timing mode: %04X", timing_mode);
      write_log_tow(1);
   }
   last_tmode = timing_mode;

   if(log_comments && (time_flags != last_tflags)) { 
      sprintf(log_text,"#! new time flags: %04X", time_flags);
      write_log_tow(1);
   }
   last_tflags = time_flags;


   if(log_comments && (last_critical != critical_alarms)) {
      sprintf(log_text, "#! new critical alarm state %04X:  ", critical_alarms);

      change = last_critical ^ critical_alarms;
      if(change & CRIT_ROM) {
         if(critical_alarms & CRIT_ROM)   strcat(log_text, "  ROM:BAD");
         else                             strcat(log_text, "  ROM:OK");
      }
      if(change & CRIT_RAM) {
         if(critical_alarms & CRIT_RAM)   strcat(log_text, "  RAM:BAD");
         else                             strcat(log_text, "  RAM:OK");
      }
      if(change & CRIT_PWR) {
         if(critical_alarms & CRIT_PWR)   strcat(log_text, "  Power:BAD");
         else                             strcat(log_text, "  Power:OK ");
      }
      if(change & CRIT_FPGA) {
         if(critical_alarms & CRIT_FPGA)  strcat(log_text, "  FPGA:BAD");
         else                             strcat(log_text, "  FPGA:OK ");
      }
      if(change & CRIT_OCXO) {
         if(critical_alarms & CRIT_OCXO)  strcat(log_text, "  OSC: BAD");
         else                             strcat(log_text, "  OSC: OK ");
      }
      write_log_tow(1);
   }
   last_critical = critical_alarms;


   change = last_minor ^ minor_alarms;
   change &= (~MINOR_PPS_SKIPPED);  // PP2S pulse skip is probably not an error
   if(log_comments && change && (check_precise_posn == 0)) {
      sprintf(log_text, "#! new minor alarm state %04X:  ", minor_alarms);
      if(change & MINOR_OSC_AGE) {
         if(minor_alarms & MINOR_OSC_AGE) strcat(log_text, "OSC age alarm   ");
         else                              strcat(log_text, "OSC age normal   ");
      }
      if(change & (MINOR_ANT_OPEN | MINOR_ANT_SHORT)) {
         if((minor_alarms & MINOR_ANT_OPEN) && (minor_alarms & MINOR_ANT_SHORT)) {
            strcat(log_text, "Antenna power  ");
         }
         else if(minor_alarms & MINOR_ANT_OPEN)  strcat(log_text, "Antenna open   ");
         else if(minor_alarms & MINOR_ANT_SHORT) strcat(log_text, "Antenna short   ");
         else                                    strcat(log_text, "Antenna OK   ");
      }
      if(change & MINOR_NO_TRACK) {
         if(minor_alarms & MINOR_NO_TRACK)  {
            if(rcvr_type == BRANDY_RCVR) strcat(log_text, "Positioning interrupted   ");
            else                         strcat(log_text, "No sats usable   ");
         }
         else strcat(log_text, "Tracking sats   ");
      }
      if(change & MINOR_OSC_CTRL) {
         if(minor_alarms & MINOR_OSC_CTRL)  strcat(log_text, "Undisciplined   ");
         else                               strcat(log_text, "Discipline OK   ");
      }
      if(change & MINOR_SURVEY) {
         if(minor_alarms & MINOR_SURVEY)    strcat(log_text, "Survey started  ");
         else                               strcat(log_text, "Survey stopped  ");
      }
      if(change & MINOR_NO_POSN) {
         if(minor_alarms & MINOR_NO_POSN)   strcat(log_text, "No saved posn   ");
         else                               strcat(log_text, "Position saved   ");
      }
      if(change & MINOR_LEAP_PEND) {
         if(minor_alarms & MINOR_LEAP_PEND) strcat(log_text, "LEAP PENDING!    ");
         else                               strcat(log_text, "No leap second   ");
      }
      if(change & MINOR_TEST_MODE) {
         if(minor_alarms & MINOR_TEST_MODE) strcat(log_text, "Test mode set    ");
         else                               strcat(log_text, "Normal op mode   ");
      }
      if(change & MINOR_BAD_POSN) {
         if(minor_alarms & MINOR_BAD_POSN)  strcat(log_text, "Saved posn BAD   ");
         else                               strcat(log_text, "Saved posn OK    ");
      }
      if(change & MINOR_EEPROM) {
         if(minor_alarms & MINOR_EEPROM)    strcat(log_text, "EEPROM corrupt   ");
         else                               strcat(log_text, "EEPROM data OK   ");
      }
      if(change & MINOR_ALMANAC) {
         if(minor_alarms & MINOR_ALMANAC)   strcat(log_text, "No almanac    ");
         else                               strcat(log_text, "Almanac OK    ");
      }
      if((change & MINOR_PPS_SKIPPED) && (res_t == 0)) {
         if(minor_alarms & MINOR_PPS_SKIPPED)  strcat(log_text, "PPS error     ");
         else                                  strcat(log_text, "PPS OK        ");
      }

      if(change & MINOR_JL_PHASE) {
         if(minor_alarms & MINOR_JL_PHASE)   strcat(log_text, "Phase >250 ns ");
         else                                strcat(log_text, "Phase OK      ");
      }
      if(change & MINOR_JL_RUNTIME) {
         if(minor_alarms & MINOR_JL_RUNTIME) strcat(log_text, "Runtime <300s ");
         else                                strcat(log_text, "Runtime OK    ");
      }
      if(change & MINOR_JL_HOLD) {
         if(minor_alarms & MINOR_JL_HOLD)    strcat(log_text, "Holdover >60s ");
         else                                strcat(log_text, "Holdover OK   ");
      }
      if(change & MINOR_JL_FREQ) {
         if(minor_alarms & MINOR_JL_FREQ)    strcat(log_text, "Freq error    ");
         else                                strcat(log_text, "Freq OK       ");
      }
      if(change & MINOR_JL_CHANGE) {
         if(minor_alarms & MINOR_JL_CHANGE)  strcat(log_text, "Phase reset   ");
         else                                strcat(log_text, "Phase set     ");
      }

      if(change & MINOR_DISCIPLINE) {
         if(minor_alarms & MINOR_DISCIPLINE) strcat(log_text, "Large PPS error");
         else                                strcat(log_text, "PPS error OK   ");
      }
      if(change & MINOR_WARMUP) {
         if(minor_alarms & MINOR_WARMUP)     strcat(log_text, "Warmup error  ");
         else                                strcat(log_text, "Warmup OK     ");
      }
      if(change & MINOR_INACTIVE)  {
         if(minor_alarms & MINOR_INACTIVE)   strcat(log_text, "Inactive      ");
         else                                strcat(log_text, "Active        ");
      }
      if(change & MINOR_HOLDOVER)  {
         if(minor_alarms & MINOR_HOLDOVER)   strcat(log_text, "Holdover>8hrs ");
         else                                strcat(log_text, "Holdover OK   ");
      }
      if(change & MINOR_PLL)  {
         if(minor_alarms & MINOR_PLL)        strcat(log_text, "PLL error     ");
         else                                strcat(log_text, "PLL OK        ");
      }
      if(change & MINOR_JAMMING)  {
         if(minor_alarms & MINOR_JAMMING)    strcat(log_text, "Jamming seen  ");
         else                                strcat(log_text, "Jamming OK    ");
      }
      write_log_tow(1);
   }
   last_minor = minor_alarms;

   if(eclipse_flag != last_eclipse) {
      if(eclipse_flag) {
         if(eclipse_flag == 1) {
            sprintf(log_text, "#! solar eclipse started");
            write_log_tow(1);
         }
         else if(0) {
            sprintf(log_text, "#! lunar eclipse started");
            write_log_tow(1);
         }
      }
      else {
         if(last_eclipse == 1) {
            sprintf(log_text, "#! solar eclipse ended");
            write_log_tow(1);
         }
         else if(0) {
            sprintf(log_text, "#! lunar eclipse ended");
            write_log_tow(1);
         }
      }

      last_eclipse = eclipse_flag;
   }

   if(eclipse_flag != 1) {
      eclipse_d1 = BIG_NUM;
      eclipse_d2 = BIG_NUM;
      eclipse_d3 = BIG_NUM;
   }
   else if(eclipse_d1 == BIG_NUM) ;
   else if(eclipse_d2 == BIG_NUM) ;
   else if(eclipse_d3 == BIG_NUM) ;
   else if((eclipse_d1 > eclipse_d2) && (eclipse_d2 < eclipse_d3)) {
      sprintf(log_text, "#! solar eclipse closest sun-moon distance");
      write_log_tow(1);
   }
}

void write_log_error(char *s, u32 val)
{
   if(log_file == 0) return;
   if(log_errors == 0) return;
   if(log_comments == 0) return;

   sprintf(log_text, "#! %s error: %u", s, (u32) val);
   write_log_comment(1);
}

void write_log_utc(int utc_offset)
{
   // write UTC offset changes to the log file

   if(log_file == 0) return;
   if(log_comments == 0) return;
   if((have_info & INFO_LOGGED) == 0) { // log file header not written yet   
      if(user_set_log) return;   // keeps bogus UTC entry out of log file
   }

   sprintf(log_text,"#");
   write_log_comment(1);
   sprintf(log_text,"#! New UTC offset: %d seconds", utc_offset);
   write_log_comment(1);
   sprintf(log_text,"#");
   write_log_comment(1);
}

void write_log_leapsecond()
{
   // write leapsecond occurance in the log file

   sprintf(log_text,"#");
   write_log_comment(1);

   month = fix_month(month);
   sprintf(log_text,"#! Leapsecond: %02d:%02d:%02d %s   %02d %s %04d - interval %ld secs", 
      hours,minutes,seconds, (time_flags & TFLAGS_UTC)?"UTC":"GPS", day,months[month],year, log_interval);
   write_log_comment(1);

   sprintf(log_text,"#");
   write_log_comment(1);
}

void log_saved_posn(int type)
{
double d_lat, d_lon, d_alt;
float x;

   if(log_file == 0) return;  
   if(log_comments == 0) return;

   if(type < 0) {  // position saved via repeated single point surveys
      sprintf(log_text, "#Position saved via repeated single point surveys.");
      write_log_comment(1);
   }
   else {  // position saved via TSIP message
      x = (float) precise_lat;   lat = (double) x;
      x = (float) precise_lon;   lon = (double) x;
      x = (float) precise_alt;   alt = (double) x;
      if     (type == 1) sprintf(log_text, "#User stopped precise save of manually entered position.  TSIP message used.");
      else if(type == 2) sprintf(log_text, "#User stopped precise save of surveyed position.  TSIP message used.");
      else if(type == 3) sprintf(log_text, "#User stopped precise survey.  Averaged position saved using TSIP message.");
      else if(type)      sprintf(log_text, "#Precise survey stopped.  Reason=%d.", type);
      else {
         sprintf(log_text, "#Position saved via TSIP message.  Roundoff error is small enough.\n");
      }
      write_log_comment(1);
   }

   // log how and why we saved a receiver position
   d_lat = (lat-precise_lat)*RAD_TO_DEG/ANGLE_SCALE;
   d_lon = (lon-precise_lon)*RAD_TO_DEG/ANGLE_SCALE*cos_factor;
   d_alt = (alt-precise_alt);

   sprintf(log_text, "#DESIRED POSITION: %.9f  %.9f  %.9f",
                        precise_lat*RAD_TO_DEG, precise_lon*RAD_TO_DEG, precise_alt);
   write_log_comment(1);

   sprintf(log_text, "#SAVED POSITION:   %.9f  %.9f  %.9f",
                        lat*RAD_TO_DEG, lon*RAD_TO_DEG, alt);
   write_log_comment(1);

   sprintf(log_text, "#ROUNDOFF ERROR:   lat: %.9f %s   lon: %.9f %s   rms: %.9f %s", 
      d_lat,angle_units,  d_lon,angle_units,  sqrt(d_lat*d_lat + d_lon*d_lon),angle_units);
   write_log_comment(1);
}


void restore_plot_config()
{
int i;

   if(showing_adv_file) { // restore all plots
      showing_adv_file = 0;
      for(i=0; i<NUM_PLOTS+DERIVED_PLOTS; i++) {
         plot[i].show_plot = plot[i].old_show;
      }
      plot_sat_count = old_sat_plot;
      plot_adev_data = old_adev_plot;
      if(plot_adev_data == 0) adev_decades_shown = 0;
      adev_period = xxx_adev_period;       // aaaaaapppppp other periods
      keep_adevs_fresh = old_keep_fresh;
   }
}

FILE *open_it(char *line, char *fn)
{
FILE *file;

    if(line == 0) return 0;
    if(fn == 0) return 0;

    strcpy(line, fn);
    file = topen(line, "r");
    if(file) return file;

    if(strstr(fn, ".")) return file; // extension given,  we are done trying

    // no extension give. try to open file with default extensions
    if(rcvr_type == CS_RCVR) {
       strcpy(line, fn);
       strcat(line, ".xml");
       file = topen(line, "r");
       if(file) return file;

       strcpy(line, fn);
       strcat(line, ".log");
       file = topen(line, "r");
       if(file) return file;
    }
    else {
       strcpy(line, fn);
       strcat(line, ".log");
       file = topen(line, "r");
       if(file) return file;

       strcpy(line, fn);
       strcat(line, ".xml");
       file = topen(line, "r");
       if(file) return file;
    }

    strcpy(line, fn);
    strcat(line, ".gpx");
    file = topen(line, "r");
    if(file) return file;

    strcpy(line, fn);
    strcat(line, ".kml");
    file = topen(line, "r");
    if(file) return file;

    strcpy(line, fn);         // RINEX
    strcat(line, ".obs");
    file = topen(line, "r");
    if(file) return file;


    // try non-log file names
    strcpy(line, fn);
    strcat(line, ".scr");
    file = topen(line, "r");
    if(file) return file;

    strcpy(line, fn);
    strcat(line, ".lla");
    file = topen(line, "r");
    if(file) return file;

    strcpy(line, fn);
    strcat(line, ".cal");
    file = topen(line, "r");
    if(file) return file;

    strcpy(line, fn);
    strcat(line, ".sig");
    file = topen(line, "r");
    if(file) return file;

    strcpy(line, fn);
    strcat(line, ".prn");
    file = topen(line, "r");
    if(file) return file;

    strcpy(line, fn);
    strcat(line, ".adv");
    file = topen(line, "r");
    if(file) return file;

    strcpy(line, fn);
    strcat(line, ".tim");
    file = topen(line, "r");
    if(file) return file;

    strcpy(line, fn);
    strcat(line, ".tie");
    file = topen(line, "r");
    if(file) return file;

    strcpy(line, fn);
    strcat(line, ".rpn");
    file = topen(line, "r");
    if(file) return file;

    return 0;
}


int time_check(int reading_log, DATA_SIZE interval, int yy,int mon,int dd,  int hours,int minutes,int seconds,double frac)
{
double t;
int warn;
COORD row, col;
struct PLOT_Q q;
double nav_step;
double delta;

   // This routine verfies that time stamps are sequential
   // NEW_RCVR
   if(log_errors == 0) return 0;

   t = jdate(yy, mon, dd) - jdate(2012,0,0);
   t += jtime(hours,minutes,seconds,frac);
   t *= (24.0*60.0*60.0*1000.0);
   t = (double) (s64) (t+0.5);

   if(!have_last_stamp) {
      last_stamp = t;
      have_last_stamp = 1;
      return 0;
   }

   // see if we have a skip in the time stamp sequence
   warn = 0;   // assume properly consecutive time stamps

   nav_step = 1000.0;
   if(reading_log && interval) nav_step =  (double) (int) (((DATA_SIZE) 1000.0 * interval)+(DATA_SIZE) 0.50);
   else if(nav_rate) nav_step = (double) (int) ((1000.0 / nav_rate)+0.50);

   delta = fabs((t - last_stamp) - nav_step);

   if((rcvr_type == ACRON_RCVR) && status_second(seconds)) ;  // seconds value is when a long status message is being processed
   else if((rcvr_type == BRANDY_RCVR) && (delta < 800.0)) warn = 0;   // !!!! brandywine receiver can send multiple time stamps per second with millisecond fractions
   else if((rcvr_type == ESIP_RCVR) && (delta < 100.0)) warn = 0;
   else if((rcvr_type == LPFRS_RCVR) && (delta < 1400.0)) warn = 0;
   else if((rcvr_type == SA35_RCVR) && (delta < 1400.0)) warn = 0;
   else if((rcvr_type == RT17_RCVR) && (delta < 100.0)) warn = 0;  // toots
   else if((rcvr_type == SRO_RCVR) && (delta < 1400.0)) warn = 0;
   else if(STARLOC && starloc_skip_delay) --starloc_skip_delay;
   else if(STARLOC && bad_starloc_time && (delta == 1000.0)) warn = 0;
   else if((rcvr_type == TSIP_RCVR) && (tsip_type == SV6_TYPE) && (delta < 1100.0)) warn = 0;
   else if((rcvr_type == TSIP_RCVR) && (res_t == RES_T) && (raw_msg_rate >= 3) && (delta < 2100.0)) warn = 0;
   else if((rcvr_type == Z12_RCVR) && (delta < 750.0)) warn = 0;   // !!!! Z12 can send 0.5 sec backwards time stamps
//OSA   else if((rcvr_type == STAR_RCVR) && (star_type == OSA_TYPE) && (delta < 2200.0)) warn = 0;  // OSA
   else if(rcvr_type == TICC_RCVR) ;
   else if(lte_lite && (have_sawtooth == 0)) ;
   else if(time_checked == 0) warn = 0;   // it's the first time 
   else if((t-last_stamp) == 0.0) {  // duplicate time stamp
      warn = 1;
       if((rcvr_type == STAR_RCVR) && (star_type == OSA_TYPE)) warn = 0;  // OSA
   }
   else if(HIGH_TS_THRESH && (delta < TS_CHECK_THRESHOLD)) { // allow larger time stamp differences
      warn = 0;
   }
   else if(nav_rate && (reading_log == 0) && (delta > 1.0)) {  // more than 1msec err in time stamp sequence
      warn = 2;  // checking live readings
   }
   else if(reading_log && (delta > 1.0)) {  // time stamp sequence error > 1 msec
      warn = 3;   // checking log readings
   }
   bad_starloc_time = 0;

   if(0 && warn) {
      sprintf(debug_text2, "unt:%f step:%f last_stamp:%f  t:%f  delta:%f nav:%f warn:%d", 
      interval, nav_step, last_stamp, t, delta, 1.0/nav_rate, warn);
   }

//if(log_file) fprintf(log_file, "#! (%d) pri=%02d:%02d:%02d  adj:%02d:%02d:%02d\n", warn, pri_hours,pri_minutes,pri_seconds, hours,minutes,seconds);

   if(warn) {  // we have a duplicate or missing time stamp
      q = get_plot_q(plot_q_in);
      q.sat_flags |= TIME_SKIP;  
      put_plot_q(plot_q_in, q);

      if(log_errors && (reading_log == 0)) {
         if     (warn == 1) sprintf(log_text, "#! time stamp duplicated.  t=%.3f last=%.3f  delta=%.1f", t, last_stamp, delta);
         else if(warn == 2) sprintf(log_text, "#! time stamp skipped.  t=%.1f  last=%.1f  err:%.0f ms  %02d:%02d:%02d  delta=%.1f", t, last_stamp, t-last_stamp, hours,minutes,seconds, delta);
         else if(warn == 3) sprintf(log_text, "#! time stamp skipped in log file  delta=%.1f", delta);
         else               sprintf(log_text, "#! time stamp sequence error type: %d  delta=%.1f", warn, delta);
if(debug_file) fprintf(debug_file, "%s\n", log_text);
         write_log_comment(1);
      }

      if(log_comments && reading_log && log_errors) {
         if(text_mode) {
            row = EDIT_ROW;
            col = EDIT_COL;
         }
         else {
            row = PLOT_TEXT_ROW;
            col = PLOT_TEXT_COL;
         }
         sprintf(out, "#! time stamp skip %d in log.  interval=%.3f  step:%.1f  prev=%.3f this=%.3f  ", 
            warn, interval, nav_step, last_stamp, t);
         vidstr(row+4, col, RED, out);
         refresh_page();
      }
   }

   last_stamp = t;
   if(warn) time_checked = 0;
   else     time_checked = have_time;

   return warn;
}


DATA_SIZE llt;  // last log temperature;
DATA_SIZE tvb;

double vp, vo, vc, vd;
double vdac, vtemp, vtemp2;
double vpressure, vhumidity;
double vadc1, vadc2, vadc3, vadc4;
int log_hhh,log_mmm;
double log_sss;
long log_tow;
int log_mark_number;
int xml_log_fmt;
DATA_SIZE read_log_interval;
int valid_read_log;
int saw_log_title;
double log_pps_scale, log_osc_scale, log_chc_scale, log_chd_scale;
double log_freq;
int saw_timelab_tic;
double read_log_jd;


DATA_SIZE xml_val(char *line)
{
char *s;

   // get a numeric value from an XML data point line

   if(line == 0) return 0.0;

   s = strchr(line, '>');

   if(s == 0) return 0.0;
   else if(s[0] == 0) return 0.0;
   else return (DATA_SIZE) atof(s+1);
}


char *xml_string(char *line)
{
char *s;
char *s2;

   // get a string value from an XML data point line

   if(line == 0) return "";

   s = strchr(line, '>');
   if(s == 0) return "";
   else if(s[0] == 0) return "";

   s2 = strchr(s, '<');
   if(s2) *s2 = 0;

   return s+1;
}


int parse_xml_value2(char *line)
{
   // process XML log data point values - second half of if() chain
   // NEW_RCVR

   if(line == 0) return 1;

   // PRS-10 values
   if(strstr(line, "<lm>")) {
      prs_lm = (int) xml_val(line);
      have_prs_lm = 1;
   }
   else if(strstr(line, "<lo>")) {
      prs_lo = (int) xml_val(line);
      have_prs_lo = 1;
   }
   else if(strstr(line, "<fc1>")) {
      prs_fc1 = (int) xml_val(line);
      have_prs_fc |= 0x01;
   }
   else if(strstr(line, "<fc2>")) {
      prs_fc2 = (int) xml_val(line);
      have_prs_fc |= 0x02;
   }
   else if(strstr(line, "<ds1>")) {
      prs_ds1 = (int) xml_val(line);
      have_prs_ds |= 0x01;
   }
   else if(strstr(line, "<fc2>")) {
      prs_ds2 = (int) xml_val(line);
      have_prs_ds |= 0x02;
   }
   else if(strstr(line, "<ss>")) {
      prs_ss = (int) xml_val(line);
      have_prs_ss = 1;
   }
   else if(strstr(line, "<sf>")) {
      prs_sf = (int) xml_val(line);
      have_prs_sf = 1;
   }
   else if(strstr(line, "<ph>")) {
      prs_ph = (int) xml_val(line);
      have_prs_ph = 1;
   }
   else if(strstr(line, "<sp1>")) {
      prs_sp[0] = (int) xml_val(line);
      have_prs_sp |= (1 << 0);
   }
   else if(strstr(line, "<sp2>")) {
      prs_sp[1] = (int) xml_val(line);
      have_prs_sp |= (1 << 1);
   }
   else if(strstr(line, "<sp3>")) {
      prs_sp[2] = (int) xml_val(line);
      have_prs_sp |= (1 << 2);
   }
   else if(strstr(line, "<mo>")) {
      prs_mo = (int) xml_val(line);
      have_prs_mo = 1;
   }
   else if(strstr(line, "<ms>")) {
      prs_ms = (int) xml_val(line);
      have_prs_ms = 1;
   }
   else if(strstr(line, "<mr>")) {
      prs_mr = (int) xml_val(line);
      have_prs_mr = 1;
   }
   else if(strstr(line, "<tt>")) {
      prs_tt = (int) xml_val(line);
      have_prs_tt = 1;
   }
   else if(strstr(line, "<ts>")) {
      prs_ts = (int) xml_val(line);
      have_prs_ts = 1;
   }
   else if(strstr(line, "<to>")) {
      prs_to = (int) xml_val(line);
      have_prs_to = 1;
   }
   else if(strstr(line, "<ps>")) {
      prs_ps = (int) xml_val(line);
      have_prs_ps = 1;
   }
   else if(strstr(line, "<pl>")) {
      prs_pl = (int) xml_val(line);
      have_prs_pl = 1;
   }
   else if(strstr(line, "<ep>")) {
      prs_ep = (int) xml_val(line);
      have_prs_ep = 1;
   }
   else if(strstr(line, "<ga>")) {
      prs_ga = (int) xml_val(line);
      have_prs_ga = 1;
   }
   else if(strstr(line, "<pt>")) {
      prs_pt = (int) xml_val(line);
      have_prs_pt = 1;
   }
   else if(strstr(line, "<pf>")) {
      prs_pf = (int) xml_val(line);
      have_prs_pf = 1;
   }
   else if(strstr(line, "<pi>")) {
      prs_pi = (int) xml_val(line);
      have_prs_pi = 1;
   }
   else if(strstr(line, "<ad0>")) {
      prs_ad[0] = (double) xml_val(line);
      have_prs_ad |= (1 << 0);
   }
   else if(strstr(line, "<ad1>")) {
      prs_ad[1] = (double) xml_val(line);
      have_prs_ad |= (1 << 1);
   }
   else if(strstr(line, "<ad2>")) {
      prs_ad[2] = (double) xml_val(line);
      have_prs_ad |= (1 << 2);
   }
   else if(strstr(line, "<ad3>")) {
      prs_ad[3] = (double) xml_val(line);
      have_prs_ad |= (1 << 3);
   }
   else if(strstr(line, "<ad4>")) {
      prs_ad[4] = (double) xml_val(line);
      have_prs_ad |= (1 << 4);
   }
   else if(strstr(line, "<ad5>")) {
      prs_ad[5] = (double) xml_val(line);
      have_prs_ad |= (1 << 5);
   }
   else if(strstr(line, "<ad6>")) {
      prs_ad[6] = (double) xml_val(line);
      have_prs_ad |= (1 << 6);
   }
   else if(strstr(line, "<ad7>")) {
      prs_ad[7] = (double) xml_val(line);
      have_prs_ad |= (1 << 7);
   }
   else if(strstr(line, "<ad8>")) {
      prs_ad[8] = (double) xml_val(line);
      have_prs_ad |= (1 << 8);
   }
   else if(strstr(line, "<ad9>")) {
      prs_ad[9] = (double) xml_val(line);
      have_prs_ad |= (1 << 9);
   }
   else if(strstr(line, "<ad10>")) {
      prs_ad[10] = (double) xml_val(line);
      have_prs_ad |= (1 << 10);
   }
   else if(strstr(line, "<ad11>")) {
      prs_ad[11] = (double) xml_val(line);
      have_prs_ad |= (1 << 11);
   }
   else if(strstr(line, "<ad12>")) {
      prs_ad[12] = (double) xml_val(line);
      have_prs_ad |= (1 << 12);
   }
   else if(strstr(line, "<ad13>")) {
      prs_ad[13] = (double) xml_val(line);
      have_prs_ad |= (1 << 13);
   }
   else if(strstr(line, "<ad14>")) {
      prs_ad[14] = (double) xml_val(line);
      have_prs_ad |= (1 << 14);
   }
   else if(strstr(line, "<ad15>")) {
      prs_ad[15] = (double) xml_val(line);
      have_prs_ad |= (1 << 15);
   }
   else if(strstr(line, "<ad16>")) {
      prs_ad[16] = (double) xml_val(line);
      have_prs_ad |= (1 << 16);
   }
   else if(strstr(line, "<ad17>")) {
      prs_ad[17] = (double) xml_val(line);
      have_prs_ad |= (1 << 17);
   }
   else if(strstr(line, "<ad18>")) {
      prs_ad[18] = (double) xml_val(line);
      have_prs_ad |= (1 << 18);
   }
   else if(strstr(line, "<ad19>")) {
      prs_ad[19] = (double) xml_val(line);
      have_prs_ad |= (1 << 19);
   }
   else if(strstr(line, "<sd0>")) {
      prs_sd[0] = (int) xml_val(line);
      have_prs_sd |= (1 << 0);
   }
   else if(strstr(line, "<sd1>")) {
      prs_sd[1] = (int) xml_val(line);
      have_prs_sd |= (1 << 1);
   }
   else if(strstr(line, "<sd2>")) {
      prs_sd[2] = (int) xml_val(line);
      have_prs_sd |= (1 << 2);
   }
   else if(strstr(line, "<sd3>")) {
      prs_sd[3] = (int) xml_val(line);
      have_prs_sd |= (1 << 3);
   }
   else if(strstr(line, "<sd4>")) {
      prs_sd[4] = (int) xml_val(line);
      have_prs_sd |= (1 << 4);
   }
   else if(strstr(line, "<sd5>")) {
      prs_sd[5] = (int) xml_val(line);
      have_prs_sd |= (1 << 5);
   }
   else if(strstr(line, "<sd6>")) {
      prs_sd[6] = (int) xml_val(line);
      have_prs_sd |= (1 << 6);
   }
   else if(strstr(line, "<sd7>")) {
      prs_sd[7] = (int) xml_val(line);
      have_prs_sd |= (1 << 7);
   }
   else if(strstr(line, "<st1>")) {
      prs_st[0] = (int) xml_val(line);
      have_prs_st |= (1 << 0);
   }
   else if(strstr(line, "<st2>")) {
      prs_st[1] = (int) xml_val(line);
      have_prs_st |= (1 << 1);
   }
   else if(strstr(line, "<st3>")) {
      prs_st[2] = (int) xml_val(line);
      have_prs_st |= (1 << 2);
   }
   else if(strstr(line, "<st4>")) {
      prs_st[3] = (int) xml_val(line);
      have_prs_st |= (1 << 3);
   }
   else if(strstr(line, "<st5>")) {
      prs_st[4] = (int) xml_val(line);
      have_prs_st |= (1 << 4);
   }
   else if(strstr(line, "<st6>")) {
      prs_st[5] = (int) xml_val(line);
      have_prs_st |= (1 << 5);
   }

   // X72 values
   else if(strstr(line, "<creg>")) {
       x72_creg = (int) xml_val(line);
       have_x72_creg = 1;
   }
   else if(strstr(line, "<dmp17>")) {
       x72_dmp17 = (double) xml_val(line);
       have_x72_dmp17 = 1;
   }
   else if(strstr(line, "<dmp5>")) {
       x72_dmp5 = (double) xml_val(line);
       have_x72_dmp5 = 1;
   }
   else if(strstr(line, "<dhtrvolt>")) {
       x72_dhtrvolt = (double) xml_val(line);
       have_x72_dhtrvolt = 1;
   }
   else if(strstr(line, "<plmp>")) {
       x72_plmp = (double) xml_val(line);
       have_x72_plmp = 1;
   }
   else if(strstr(line, "<pres>")) {
       x72_pres = (double) xml_val(line);
       have_x72_pres = 1;
   }
   else if(strstr(line, "<dlvthermc>")) {
       x72_dlvthermc = (double) xml_val(line);
       have_x72_dlvthermc = 1;
   }
   else if(strstr(line, "<drvthermc>")) {
       x72_drvthermc = (double) xml_val(line);
       have_x72_drvthermc = 1;
   }
   else if(strstr(line, "<dlvolt>")) {
       x72_dlvolt = (double) xml_val(line);
       have_x72_dlvolt = 1;
   }
   else if(strstr(line, "<dmvoutc>")) {
       x72_dmvoutc = (double) xml_val(line);
       have_x72_dmvoutc = 1;
   }
   else if(strstr(line, "<dtemplo>")) {
       x72_dtemplo = (double) xml_val(line);
       have_x72_dtemplo = 1;
   }
   else if(strstr(line, "<dtemphi>")) {
       x72_dtemphi = (double) xml_val(line);
       have_x72_dtemphi = 1;
   }
   else if(strstr(line, "<dvoltlo>")) {
       x72_dvoltlo = (double) xml_val(line);
       have_x72_dvoltlo = 1;
   }
   else if(strstr(line, "<dvolthi>")) {
       x72_dvolthi = (double) xml_val(line);
       have_x72_dvolthi = 1;
   }
   else if(strstr(line, "<dlvoutc>")) {
       x72_dlvoutc = (double) xml_val(line);
       have_x72_dlvoutc = 1;
   }
   else if(strstr(line, "<drvoutc>")) {
       x72_drvoutc = (double) xml_val(line);
       have_x72_drvoutc = 1;
   }
   else if(strstr(line, "<dmv2demavg>")) {
       x72_dmv2demavg = (double) xml_val(line);
       have_x72_dmv2demavg = 1;
   }
   else if(strstr(line, "<scont>")) {
       x72_scont = (int) xml_val(line);
       have_x72_scont = 1;
   }
   else if(strstr(line, "<sernum>")) {
       x72_sernum = (int) xml_val(line);
       have_x72_sernum = 1;
   }
   else if(strstr(line, "<pwrticks>")) {
       x72_pwrticks = (int) xml_val(line);
       have_x72_pwrticks = 1;
   }
   else if(strstr(line, "<lhhrs>")) {
       x72_lhhrs = (int) xml_val(line);
       have_x72_lhhrs = 1;
   }
   else if(strstr(line, "<lhticks>")) {
       x72_lhticks = (int) xml_val(line);
       have_x72_lhticks = 1;
   }
   else if(strstr(line, "<rhhrs>")) {
       x72_rhhrs = (int) xml_val(line);
       have_x72_rhhrs = 1;
   }
   else if(strstr(line, "<rhticks>")) {
       x72_rhticks = (int) xml_val(line);
       have_x72_rhticks = 1;
   }

   // misc values
   else if(strstr(line, "<led>")) {
       star_led = (int) xml_val(line);
       have_star_led = 1;
   }
   else if(rcvr_type == LPFRS_RCVR) {
      if(strstr(line, "<aa>")) {
         lpfrs_aa = xml_val(line);
      }
      else if(strstr(line, "<bb>")) {
         lpfrs_bb = xml_val(line);
      }
      else if(strstr(line, "<cc>")) {
         lpfrs_cc = xml_val(line);
      }
      else if(strstr(line, "<dd>")) {
         lpfrs_dd = xml_val(line);
      }
      else if(strstr(line, "<ee>")) {
         lpfrs_ee = xml_val(line);
      }
      else if(strstr(line, "<ff>")) {
         lpfrs_ff = xml_val(line);
      }
      else if(strstr(line, "<gg>")) {
         lpfrs_gg = xml_val(line);
      }
      else if(strstr(line, "<hh>")) {
         lpfrs_hh = xml_val(line);
      }
   }
   else if(rcvr_type == SA35_RCVR) {
      // !!!!! placeholder
   }
   else if(rcvr_type == SRO_RCVR) {
      if(strstr(line, "<aa>")) {
         sro_aa = xml_val(line);
      }
      else if(strstr(line, "<bb>")) {
         sro_bb = xml_val(line);
      }
      else if(strstr(line, "<cc>")) {
         sro_cc = xml_val(line);
      }
      else if(strstr(line, "<dd>")) {
         sro_dd = xml_val(line);
      }
      else if(strstr(line, "<ee>")) {
         sro_ee = xml_val(line);
      }
      else if(strstr(line, "<ff>")) {
         sro_ff = xml_val(line);
      }
      else if(strstr(line, "<gg>")) {
         sro_gg = xml_val(line);
      }
      else if(strstr(line, "<hh>")) {
         sro_hh = xml_val(line);
      }
   }

   return 1;
}

int parse_xml_value(char *line)
{
char *s;
char ti[20];
static char supply[32];
static char sync[32];

   // process XML log data point values (first half of if() chain
   // NEW_RCVR

   if(line == 0) return 1;

   if(strstr(line, "</trkpt>")) {  // end of data point's values
      return 0;
   }

   if(strstr(line, "<trkpt")) {  // start of a data point entry
      have_dops = 0;
      s = strstr(line, "lat=");         // lat and lon
      if(s) lat = atof(s+5)*PI/180.0;
      s = strstr(line, "lon=");
      if(s) lon = atof(s+5)*PI/180.0;
   }
   else if(strstr(line, "<ele>")) {     // altitude
      alt = xml_val(line);
   }
   else if(strstr(line, "<course>")) {  // heading
      heading = xml_val(line);
      have_heading = 1;
   }
   else if(strstr(line, "<speed>")) {   // speed
      speed = xml_val(line);
      have_speed = 1;
   }
   else if(strstr(line, "<datum>")) {   // datum
      datum = (int) xml_val(line);
      have_datum = 1;
   }
   else if(strstr(line, "<time>")) {    // time code (in UTC)
      s = strstr(line, "<time>");
      if(s) {
         sscanf(s+5+1, "%d%c%d%c%d%c%d%c%d%c%lf",
            &year,&ti[0],&month,&ti[0],&day,&ti[0],
            &log_hhh,&ti[0],&log_mmm,&ti[0],&log_sss
         );
      }
   }
   else if(strstr(line, "<tow>")) {   // time-of-week
      log_tow = (long) xml_val(line);
      have_tow = 1;
   }
   else if(strstr(line, "<temp>")) {  // things stored in the [TEMP] data queue variable 
      temperature = (DATA_SIZE) xml_val(line);
      have_temperature = 1;
   }
   else if(strstr(line, "<temp0>")) {  // things stored in the [TC2] data queue variable 
      tc0 = (DATA_SIZE) xml_val(line);
      have_temperature0 = 1;
   }
   else if(strstr(line, "<temp1>")) {  // things stored in the [TC2] data queue variable 
      tc1 = (DATA_SIZE) xml_val(line);
      have_temperature1 = 1;
   }
   else if(strstr(line, "<temp2>")) {  // things stored in the [TC2] data queue variable 
      tc2 = (DATA_SIZE) xml_val(line);
      have_temperature2 = 2;
   }
   else if(strstr(line, "<rh>")) { 
      humidity = (DATA_SIZE) xml_val(line);
      have_humidity = 1;
   }
   else if(strstr(line, "<mb>")) { 
      pressure = (DATA_SIZE) xml_val(line);
      have_pressure = 1;
   }
   else if(strstr(line, "<adc1>")) { 
      adc1 = (DATA_SIZE) xml_val(line);
      have_adc1 = 1;
   }
   else if(strstr(line, "<adc2>")) { 
      adc2 = (DATA_SIZE) xml_val(line);
      have_adc2 = 1;
   }
   else if(strstr(line, "<adc3>")) { 
      adc3 = (DATA_SIZE) xml_val(line);
      have_adc3 = 1;
   }
   else if(strstr(line, "<adc4>")) { 
      adc4 = (DATA_SIZE) xml_val(line);
      have_adc4 = 1;
   }
   else if(strstr(line, "<tcor>")) {
      temperature = (DATA_SIZE) xml_val(line);
      have_temperature = 1;
   }
   else if(strstr(line, "<dac>")) {   // things stored in the [DAC] data queue variable 
      dac_voltage = (DATA_SIZE) xml_val(line);
      have_dac = 1;
   }
   else if(strstr(line, "<sawtooth>")) {
      dac_voltage = (DATA_SIZE) (xml_val(line));
      have_sawtooth = 1;
   }
   else if(strstr(line, "<sawt>")) {
      dac_voltage = (DATA_SIZE) (xml_val(line));
      have_sawtooth = 1;
   }
   else if(strstr(line, "<dacl>")) {   // Lars GPSDO data
      lars_dac = (DATA_SIZE) xml_val(line);
      have_lars_dac = 1;
   }
   else if(strstr(line, "<templ>")) {  
      lars_temp = (DATA_SIZE) xml_val(line);
      have_lars_temp = 1;
   }
   else if(strstr(line, "<tiel>")) {  
      lars_pps = (DATA_SIZE) xml_val(line);
      have_lars_pps = 1;
   }
   else if(strstr(line, "<gclk>")) {
      esip_pps_type = (int) (xml_val(line));
      have_esip_pps_type = 1;
   }
   else if(strstr(line, "<eval>")) {
      true_eval = (DATA_SIZE) (xml_val(line));
      have_true_eval = 1;
   }
   else if(strstr(line, "<pps>")) {   // things stored in the [PPS] data queue variable 
      vp = xml_val(line);             // aaalll
      if(!TIMING_RCVR) vp /= 1.0E9;
      have_pps_offset = 1;
   }
   else if(strstr(line, "<tt>")) {    // things stored in the [PPS] data queue variable 
      vp = xml_val(line);             // aaalll
      if(!TIMING_RCVR) vp /= 1.0E9;
      have_prs_tt = 1;
   }
   else if(strstr(line, "<phase>")) { // things stored in the [PPS] data queue variable 
      vp = xml_val(line);             // aaalll
      if(!TIMING_RCVR) vp /= 1.0E9;
      have_phase = 1;
   }
   else if(strstr(line, "<osc>")) {   // things stored in the [OSC] data queue variable
      vo = xml_val(line);    // aaalll 
      have_osc_offset = 1;
   }
   else if(strstr(line, "<freq>")) {   // things stored in the [OSC] data queue variable
      vo = xml_val(line);    // aaalll 
   }
   else if(strstr(line, "<dds>")) {   // things stored in the [OSC] data queue variable
      vo = xml_val(line);    // aaalll 
   }
   else if(strstr(line, "<fc>")) {   // things stored in the [OSC] data queue variable
      vo = xml_val(line);    // aaalll 
   }
   else if(strstr(line, "<cha>")) {   // things stored in the [PPS] data queue variable 
      vp = xml_val(line);    // aaalll 
      pps_phase += vp;
   }
   else if(strstr(line, "<chb>")) {   // things stored in the [OSC] data queue variable
      vo = xml_val(line);    // aaalll 
      osc_phase += vo;
   }
   else if(strstr(line, "<chc>")) {   // things stored in the [CHC] data queue variable
      vc = xml_val(line);    // aaalll 
      chc_phase = vc;
   }
   else if(strstr(line, "<chd>")) {   // things stored in the [CHD] data queue variable
      vd = xml_val(line);
      chd_phase += vd;
   }
   else if(strstr(line, "<clk>")) {   // things stored in the [PPS] data queue variable 
      vp = xml_val(line);   // GPS clock value  // aaalll 
   }
   else if(strstr(line, "<accu>")) {
      vp = xml_val(line);    // aaalll 
   }
   else if(strstr(line, "<bias>")) {
      vp = xml_val(line);    // aaalll 
   }
   else if(strstr(line, "<rgen>")) {
      vp = xml_val(line);    // aaalll 
   }
   else if(strstr(line, "<drft>")) {
      vp = xml_val(line);    // aaalll 
   }
   else if(strstr(line, "<unc>")) {
      vo = xml_val(line);   // aaalll
   }
   else if(strstr(line, "<frac>")) {
      vo = xml_val(line);   // aaalll 
   }
   else if(strstr(line, "<cofs>")) {
      vo = xml_val(line);   // aaalll 
   }
   else if(strstr(line, "<rate>")) {
      vo = xml_val(line);   // aaalll 
   }
   else if(strstr(line, "<sat>")) {   // sat count
      sat_count = (int) xml_val(line);
   }
   else if(strstr(line, "<tfom>")) {  // figure of merits
      tfom = (int) xml_val(line);
      have_tfom = 1;
   }
   else if(strstr(line, "<ffom>")) {
      ffom = (int) xml_val(line);
      have_ffom = 1;
   }
   else if(strstr(line, "<pll>")) {
      ffom = (int) xml_val(line);
      have_pll = 1;
   }
   else if(strstr(line, "<pdop>")) {  // dops
      pdop = (DATA_SIZE) xml_val(line);
      have_dops |= PDOP;
   }
   else if(strstr(line, "<hdop>")) {
      hdop = (DATA_SIZE) xml_val(line);
      have_dops |= HDOP;
   }
   else if(strstr(line, "<vdop>")) {
      vdop = (DATA_SIZE) xml_val(line);
      have_dops |= VDOP;
   }
   else if(strstr(line, "<tdop>")) {
      tdop = (DATA_SIZE) xml_val(line);
      have_dops |= TDOP;
   }
   else if(strstr(line, "<gdop>")) {
      gdop = (DATA_SIZE) xml_val(line);
      have_dops |= GDOP;
   }
   else if(strstr(line, "<ndop>")) {
      ndop = (DATA_SIZE) xml_val(line);
      have_dops |= NDOP;
   }
   else if(strstr(line, "<edop>")) {
      edop = (DATA_SIZE) xml_val(line);
      have_dops |= EDOP;
   }
   else if(strstr(line, "<oscgain>")) {
      osc_gain = (DATA_SIZE) xml_val(line);
      user_osc_gain = osc_gain;
      log_osc_gain = osc_gain;
      gain_color = YELLOW;
      have_gain = 1;
   }
   else if(strstr(line, "<beam>")) { 
      cs_beam = xml_val(line);
      have_cs_beam = 1;
   }
   else if(strstr(line, "<cfield>")) {
      cs_cfield = xml_val(line);
      have_cs_cfield = 1;
   }
   else if(strstr(line, "<pump>")) { 
      cs_pump = xml_val(line);
      have_cs_pump = 1;
   }
   else if(strstr(line, "<gain>")) { 
      cs_gain = xml_val(line);
      have_cs_gain = 1;
   }
   else if(strstr(line, "<emult>")) {
      cs_emul = xml_val(line);
      have_cs_emul = 1;
   }
   else if(strstr(line, "<hwi>")) { 
      cs_hwi = xml_val(line);
      have_cs_hwi = 1;
   }
   else if(strstr(line, "<mspec>")) {
      cs_msp = xml_val(line);
      have_cs_msp = 1;
   }
   else if(strstr(line, "<coven>")) {
      cs_coven = xml_val(line);
      have_cs_coven = 1;
   }
   else if(strstr(line, "<qoven>")) {
      cs_qoven = xml_val(line);
      have_cs_qoven = 1;
   }
   else if(strstr(line, "<rfampl1>")) { 
      cs_rfam1 = xml_val(line);
      have_cs_rfam |= 0x01;
   }
   else if(strstr(line, "<rfampl2>")) { 
      cs_rfam2 = xml_val(line);
      have_cs_rfam = 0x02;
   }
   else if(strstr(line, "<dro>")) { 
      cs_pll_dro = xml_val(line);
      have_cs_pll |= 0x01;
   }
   else if(strstr(line, "<saw>")) { 
      cs_pll_saw = xml_val(line);
      have_cs_pll |= 0x02;
   }
   else if(strstr(line, "<87mhz>")) { 
      cs_pll_87 = xml_val(line);
      have_cs_pll |= 0x04;
   }
   else if(strstr(line, "<up>")) { 
      cs_pll_up = xml_val(line);
      have_cs_pll |= 0x08;
   }
   else if(strstr(line, "<freq1>")) { 
      cs_freq1 = xml_val(line);
      have_cs_freq1 = 1;
   }
   else if(strstr(line, "<freq2>")) { 
      cs_freq2 = xml_val(line);
      have_cs_freq2 = 1;
   }
   else if(strstr(line, "<ster>")) { 
      cs_ster = xml_val(line);
      have_cs_ster = 1;
   }
   else if(strstr(line, "<slew>")) { 
      cs_slew = xml_val(line);
      have_cs_slew = 1;
   }
   else if(strstr(line, "<volt1>")) {
      cs_volt1 = xml_val(line);
      have_cs_volt |= 0x01;
   }
   else if(strstr(line, "<volt2>")) {
      cs_volt2 = xml_val(line);
      have_cs_volt |= 0x02;
   }
   else if(strstr(line, "<volt3>")) {
      cs_volt3 = xml_val(line);
      have_cs_volt |= 0x04;
   }
   else if(strstr(line, "<leapdur>")) { 
      cs_leapdur = (int) xml_val(line);
      have_cs_leapdur = 1;
   }
   else if(strstr(line, "<leapmjd>")) { 
      cs_leapmjd = xml_val(line);
      have_cs_leapmjd = 1;
   }
   else if(strstr(line, "<supply>")) {  // %s
      strcpy(out, xml_string(line));
      out[30] = 0;
      strcpy(supply, out);
      cs_supply = &supply[0];
      have_cs_supply = 1;
   }
   else if(strstr(line, "<sync>")) {   // %s
      strcpy(out, xml_string(line));
      out[30] = 0;
      strcpy(sync, out);
      cs_sync = &sync[0];
      have_cs_sync = 1;
   }
   else if(strstr(line, "<cont>")) { 
      cs_cont = (int) xml_val(line); 
      have_cs_cont = 1;
   }
   else if(strstr(line, "<disp>")) { 
      cs_disp = (int) xml_val(line); 
      have_cs_disp = 1;
   }
   else if(strstr(line, "<standby>")) {
      cs_standby = (int) xml_val(line) ;
      have_cs_standby = 1;
   }
   else if(strstr(line, "<remote>")) {
      cs_remote = (int) xml_val(line);
      have_cs_remote = 1;
   }
   else if(strstr(line, "<antv1>")) {
      ant_v1 = (double) xml_val(line);
      have_ant_v1 = 1;
   }
   else if(strstr(line, "<antv2>")) {
      ant_v2 = (double) xml_val(line);
      have_ant_v2 = 1;
   }
   else if(strstr(line, "<antma>")) {
      ant_ma = (double) xml_val(line);
      have_ant_v1 = 1;
   }
   else if(strstr(line, "<lampv>")) {
      ant_v1 = (double) xml_val(line);
   }
   else if(strstr(line, "<ocxov>")) {
      ant_v2 = (double) xml_val(line);
   }
   else if(strstr(line, "<oscv>")) {
      rftg_oscv = (double) xml_val(line);
   }
   else if(strstr(line, "<lattide>")) {
      lat_tide = (double) xml_val(line);
      have_tides = 1;
   }
   else if(strstr(line, "<lontide>")) {
      lon_tide = (double) xml_val(line);
      have_tides = 1;
   }
   else if(strstr(line, "<alttide>")) {
      alt_tide = (double) xml_val(line);
      have_tides = 1;
   }
   else if(strstr(line, "<ugals>")) {
      ugals = (double) xml_val(line);
      have_ugals = 1;
   }
   else {  // break up if() chain into two pieces - reached compiler limit
      return parse_xml_value2(line);
   }

   return 1;
}

void set_log_title(char *plot_title)
{
int i;

   // set the plot title string from info in a log file
   if(plot_title == 0) return;

   i = strlen(plot_title);

   if(i && (plot_title[i-1] == 0x0D)) plot_title[i-1] = 0;
   if(i && (plot_title[i-1] == 0x0A)) plot_title[i-1] = 0;
   if(plot_title[0]) title_type = USER;
   else              title_type = NONE;

   saw_log_title = 1;
   show_title();
   refresh_page();
}


int parse_log_comment(char *line)
{
char *s;
char ti[120];

   // process comment line in a log file

   // !!! This is a VERY crude parser that depends upon fixed spacing.
   // !!! If you change any of the log file output formats,  you will need
   // !!! to make changes in this code also.

   if(line == 0) return 1;

   if(xml_log_fmt && strstr(line, "<!--")) {  // XML comment
      if(strlen(line) < 6) return 1;

      if((line[10] == ':') && (line[13] == ':')) goto time_cmt;  // time comment
      if(!strnicmp(&line[6], "TITLE:", 6)) {
         strcpy(plot_title, &line[6+6]);
         s = strstr(plot_title, "-->");
         if(s) *s = 0;
         goto set_title;
      }
      else if(!strnicmp(&line[6], "DEBUG1:", 7)) {
         strcpy(debug_text, &line[7+6]);
         s = strstr(debug_text, "-->");
         if(s) *s = 0;
         goto set_title1;
      }
      else if(!strnicmp(&line[6], "DEBUG2:", 7)) {
         strcpy(debug_text2, &line[7+6]);
         s = strstr(debug_text2, "-->");
         if(s) *s = 0;
         goto set_title2;
      }
      else if(!strnicmp(&line[6], "DEBUG3:", 7)) {
         strcpy(debug_text3, &line[7+6]);
         s = strstr(debug_text3, "-->");
         if(s) *s = 0;
         goto set_title3;
      }
      else if(!strnicmp(&line[6], "DEBUG4:", 7)) {
         strcpy(debug_text4, &line[7+6]);
         s = strstr(debug_text4, "-->");
         if(s) *s = 0;
         goto set_title4;
      }
      else if(!strnicmp(&line[6], "DEBUG5:", 7)) {
         strcpy(debug_text5, &line[7+6]);
         s = strstr(debug_text5, "-->");
         if(s) *s = 0;
         goto set_title5;
      }
      else if(!strnicmp(&line[6], "DEBUG6:", 7)) {
         strcpy(debug_text6, &line[7+6]);
         s = strstr(debug_text6, "-->");
         if(s) *s = 0;
         goto set_title6;
      }
      else if(!strnicmp(&line[6], "DEBUG7:", 7)) {
         strcpy(debug_text7, &line[7+6]);
         s = strstr(debug_text7, "-->");
         if(s) *s = 0;
         goto set_title7;
      }
      else if(!strnicmp(&line[6], "MARKER", 6)) {
         log_mark_number = atoi(&line[6+6]);
         if((log_mark_number >= 0) && (log_mark_number < MAX_MARKER)) {
            mark_q_entry[log_mark_number] = plot_q_in;
         }
      }
      else if(!strnicmp(&line[6], "OSC_GAIN", 8)) {
         osc_gain = (DATA_SIZE) atof(&line[6+8]);
         user_osc_gain = osc_gain;
         lars_gain = osc_gain;
         log_osc_gain = osc_gain;
         gain_color = YELLOW;
      }

      return 1;
   }
   else if((line[0] == '#') || (line[0] == ';') || (line[0] == '*') || (line[0] == '/')) { 
      if((line[5] == ':') && (line[8] == ':')) {  // time comment
         time_cmt:
         if(strstr(line, "seconds") == 0) return 1;

         s = strstr(line, "interval");
         if(s == 0) return 1;

         read_log_interval = (DATA_SIZE) atof(s+8);
if(read_log_interval && (user_set_qi == 0)) queue_interval = (long) read_log_interval;
         if(read_log_interval && pause_data) {
            if(!restore_nav_rate) {            // save current nav rate
               saved_nav_rate = nav_rate;
               restore_nav_rate = 1;
            }
            if(0 && read_log_interval) {
               nav_rate = ((DATA_SIZE) 1.0 / read_log_interval); // set nav rate to what's in the log so plots scale properly
            }
            else nav_rate = 1.0;
         }

         if(strstr(line, "GPS")) {
            time_flags = TFLAGS_NULL;
            s = strstr(line, "GPS");
         }
         else if(strstr(line, "UTC")) {
            time_flags = TFLAGS_UTC;
            s = strstr(line, "UTC");
         }

         if(s) sscanf(s+3, "%d %s %d", &day, &ti[0], &year);
         for(month=1; month<=12; month++) {
            if(!strcmp(months[month], &ti[0])) goto got_month;
         }
         month = 0;

         got_month:
         valid_read_log = 1;
      }
      else if((line[1] == '!') && strstr(line, "discipline mode")) {  // discipline mode comment
         discipline_mode = DIS_MODE_NORMAL;
         if     (strstr(line, "Normal"))          discipline_mode = DIS_MODE_NORMAL;
         else if(strstr(line, "Power-up"))        discipline_mode = DIS_MODE_POWERUP;
         else if(strstr(line, "Auto holdover"))   discipline_mode = DIS_MODE_AUTO_HOLD;
         else if(strstr(line, "Manual holdover")) discipline_mode = DIS_MODE_MANUAL_HOLD;
         else if(strstr(line, "Recovery mode"))   discipline_mode = DIS_MODE_RECOVERY;
         else if(strstr(line, "Fast recovery"))   discipline_mode = DIS_MODE_FAST_RECOVERY;
         else if(strstr(line, "Disabled"))        discipline_mode = DIS_MODE_DISABLED;
         else if(strstr(line, "Warming up"))      discipline_mode = DIS_MODE_WARMUP;
         else if(strstr(line, "Settling"))        discipline_mode = DIS_MODE_WARMUP;
      }
      else if(!strnicmp(line, "#TITLE", 6)) {
         strcpy(plot_title, &line[7]);
         set_title:
         set_log_title(plot_title);
      }
      else if(!strnicmp(line, "#DEBUG1", 7)) {
         strcpy(debug_text, &line[8]);
         set_title1:
         set_log_title(debug_text);
      }
      else if(!strnicmp(line, "#DEBUG2", 7)) {
         strcpy(debug_text2, &line[8]);
         set_title2:
         set_log_title(debug_text2);
      }
      else if(!strnicmp(line, "#DEBUG3", 7)) {
         strcpy(debug_text3, &line[8]);
         set_title3:
         set_log_title(debug_text3);
      }
      else if(!strnicmp(line, "#DEBUG4", 7)) {
         strcpy(debug_text4, &line[8]);
         set_title4:
         set_log_title(debug_text4);
      }
      else if(!strnicmp(line, "#DEBUG5", 7)) {
         strcpy(debug_text5, &line[8]);
         set_title5:
         set_log_title(debug_text5);
      }
      else if(!strnicmp(line, "#DEBUG6", 7)) {
         strcpy(debug_text6, &line[8]);
         set_title6:
         set_log_title(debug_text6);
      }
      else if(!strnicmp(line, "#DEBUG7", 7)) {
         strcpy(debug_text7, &line[8]);
         set_title7:
         set_log_title(debug_text7);
      }
      else if(!strnicmp(line, "#MARKER", 7)) {
         log_mark_number = 0;
         sscanf(&line[8], "%d", &log_mark_number);
         if((log_mark_number >= 0) && (log_mark_number < MAX_MARKER)) {
            mark_q_entry[log_mark_number] = plot_q_in;
         }
      }
      else if(!strnicmp(line, "#OSC_GAIN", 9)) {
         osc_gain = (-3.5);
         osc_gain = (DATA_SIZE) atof(&line[10]);
         user_osc_gain = osc_gain;
         lars_gain = osc_gain;
         log_osc_gain = osc_gain;
         gain_color = YELLOW;
      }
      else if(!strnicmp(line, "#SCALE", 6)) {
         log_pps_scale = log_osc_scale = 1.0;
         sscanf(&line[7], "%lf %lf", &log_pps_scale, &log_osc_scale);
      }
      else if(!strnicmp(line, "#INTERVAL", 9)) {
         adev_period = 1.0;
         sscanf(&line[10], "%lf", &adev_period);   // aaaaaapppppp other periods 
      }
      else if(!strnicmp(line, "#PERIOD", 7)) {
         adev_period = 1.0;
         sscanf(&line[8], "%lf", &adev_period);    // aaaaaapppppp other periods 
      }
      else if(!strnicmp(line, "#LLA", 4)) {
         sscanf(&line[5], "%lf %lf %lf", &precise_lat, &precise_lon, &precise_alt);
         precise_lon /= RAD_TO_DEG;
         precise_lat /= RAD_TO_DEG;
         cos_factor = cos(precise_lat);
         if(cos_factor == 0.0) cos_factor = 0.001;
      }
      return 1;
   }

   return 0;
}


int tim_command(char *line)
{
   if(line == 0) return 1;

   if(!strnicmp(line, "BLN", 3)) {  // new format file
      return 1;
   }
   else if(!strnicmp(line, "DBL", 3)) {
      return 1;
   }
   else if(!strnicmp(line, "S32", 3)) {
      return 1;
   }
   else if(!strnicmp(line, "STR", 3)) {
      return 1;
   }

   else if(!strnicmp(line, "CAP", 3)) {
      if(strlen(line) > 4) {
         if(saw_log_title) strcat(plot_title, " | ");
         strcat(plot_title, &line[4]);
         set_log_title(plot_title);
         return 1;
      }
   }
   else if(!strnicmp(line, "DRIVER", 6)) {
      if(strlen(line) > 6+2) {
         if(saw_log_title) strcat(plot_title, " | ");
         strcat(plot_title, &line[6+2]);
         set_log_title(plot_title);
         return 1;
      }
   }
   else if(!strnicmp(line, "IMO", 3)) {
      if(strlen(line) > 4) {
         if(saw_log_title) strcat(plot_title, " | ");
         strcat(plot_title, &line[4]);
         set_log_title(plot_title);
         return 1;
      }
   }
   else if(!strnicmp(line, "INSTRUMENT", 10)) {
      if(strlen(line) > 10+2) {
         if(saw_log_title) strcat(plot_title, " | ");
         strcat(plot_title, &line[10+2]);
         set_log_title(plot_title);
         return 1;
      }
   }
   else if(!strnicmp(line, "MJD", 3)) {  // modified Julian date
      if(strlen(line) > 4) {
         sscanf(&line[4], "%lf", &read_log_jd);
         read_log_jd += JD_MJD;
         return 1;
      }
   }
   else if(!strnicmp(line, "NOM", 3)) {  // nominal freq
      if(strlen(line) > 4) {
         log_freq = NOMINAL_FREQ;
         sscanf(&line[4], "%lf", &log_freq);
         if(log_freq > 0.0) cha_phase_wrap_interval = 1.0 / log_freq;
         else               cha_phase_wrap_interval = 1.0 / NOMINAL_FREQ;
         return 1;
      }
   }           
   else if(!strnicmp(line, "PER", 3)) {
      if(strlen(line) > 4) {
         adev_period = 1.0;
         sscanf(&line[4], "%lf", &adev_period);   // aaaaaapppppp other periods 
         pps_adev_period = adev_period;
         osc_adev_period = adev_period;
         chc_adev_period = adev_period;
         chd_adev_period = adev_period;
         return 1;
      }
   }
   else if(!strnicmp(line, "SCA", 3)) {
      if(strlen(line) > 4) {
         log_pps_scale = log_osc_scale = 1.0;
         sscanf(&line[4], "%lf %lf", &log_pps_scale, &log_osc_scale);
         return 1;
      }
   }
   else if(!strnicmp(line, "TIC", 3)) {
      saw_timelab_tic = 1;
      return 1;
   }
   else if(!strnicmp(line, "TIM", 3)) {
      if(strlen(line) > 4) {
         if(saw_log_title) strcat(plot_title, " | ");
         strcat(plot_title, &line[4]);
         set_log_title(plot_title);
         return 1;
      }
   }
   else {
      saw_timelab_tic = 0;
      return 1;
   }

   return 0;
}


void read_tie_file(char *s)
{
FILE *tie_file;
double cha, chb, chc, chd;
int n;
int count;

   // stop recording live signal strength data and load in signal strength 
   // data from a log file

   tie_file = topen(s, "r");
   if(tie_file == 0) return;

   ATYPE = A_MTIE;
   if(rcvr_type == TICC_RCVR) all_adevs = ALL_CHANS;
   else                       all_adevs = SINGLE_ADEVS;
   alloc_mtie(10);

   count = 0;

   while(fgets(out, sizeof out, tie_file) != NULL) {
      if((out[0] == '*') || (out[0] == ';') || (out[0] == '/') || (out[0] == '#')) {
         continue;  // comment line
      }
      n = sscanf(out, "%lg %lg %lg %lg", &cha, &chb, &chc, &chd);
      if(n >= 1) add_mtie_point(CHA_MTIE, cha);
      if(n >= 2) add_mtie_point(CHB_MTIE, chb);
      if(n >= 3) add_mtie_point(CHC_MTIE, chc);
      if(n >= 4) add_mtie_point(CHD_MTIE, chd);
      if(++count > adev_q_size) break;
   }

   fclose(tie_file);

   find_global_max();
   force_adev_redraw(10);
   config_screen(108);
   if(adevs_active(0)) last_was_adev = 'I';

// show_all_adevs();
}


int read_rawfile(char *s)
{
long seek_addr;

   // load a .raw capture file and treat it as a simulation file

   if(s == 0) return 0;
   strcpy(sim_name, s);

   s = strchr(sim_name, ',');
   if(s) {  // seek offset set
      *s = 0;
      seek_addr = (long) atoi(s+1);
   }
   else seek_addr = 0;

   sim_file = topen(sim_name, "rb");
   if(sim_file == 0) return 1;
   if(rcvr_type != CS_RCVR) sim_file_read |= 0x01;

   kbd_sim = 1;  // flag that a keyboard command opened the simulation file
   fseek(sim_file, seek_addr, SEEK_SET);
   sim_eof = 0;

   return 2;
}


int reload_log(char *fn, u08 cmd_line)
{
FILE *file;
char line[SLEN];
int append_log;
char ti[120];
u32 i, j;
int k;
double frac;
u08 temp_pause;
long counter;
COORD row, col;
int color;
int err;

u08 temp_dmode;
u08 temp_tflags;
int temp_day, temp_month, temp_year;
double temp_utc;

double pps_val, osc_val, chc_val, chd_val;
double pps_ref, osc_ref, chc_ref, chd_ref;
double pps_phase, osc_phase, chc_phase, chd_phase;
static double last_pps;

u08 lla_seen;
u08 have_ref;
u08 old_fixes;
u08 tim_file;
FILE *afile;
double old_jd;

   // returns 1=bad file name
   //         2=bad file format
   //         3=lla file
   //         4=script file opened
   //         0=all other file types
   //
   //  PS: Sorry for this overly long horrible kludge of a function...
   //
   // NEW_RCVR

   if(fn == 0) return 1;
   afile = 0;  // topen("ADEV.XXX", "w");

   color = 0;
   adev_log = lla_log = lla_seen = have_ref = 0;
   log_pps_scale = log_osc_scale = log_chc_scale = log_chd_scale = 1.0;
   pps_val = osc_val = chc_val = chd_val = 0.0;
   pps_ref = osc_ref = chc_ref = chd_ref = 0.0;
   pps_phase = osc_phase = chc_phase = chd_phase = 0.0;
   pps_count = osc_count = chc_count = chd_count = 0.0;  // aaalll
   vp = vo = vc = vd = 0.0;
   have_pps_offset = have_osc_offset = have_chc_offset = have_chd_offset = 0;
   saw_log_title = 0;
   tim_file = 0;
   xml_log_fmt = 0;
   saw_timelab_tic = 0;
   log_freq = nominal_cha_freq;  // NOMINAL_FREQ  !!!!! independent channel values?


   get_clock_time();       // initialize TICC simulation file time
   read_log_jd = jdate(clk_year,clk_month,clk_day) + jtime(clk_hours,clk_minutes,clk_seconds,0.0);
    

    temp_utc = jd_utc;
    temp_pause = pause_data;
    if(text_mode) {
       row = EDIT_ROW;
       col = EDIT_COL;
    }
    else {
       row = PLOT_TEXT_ROW+4;
       col = PLOT_TEXT_COL;
    }
    erase_help();

    
    if(fn[0] == '+') {
       file = open_it(line, &fn[1]);
       append_log = 1;
    }
    else {
       file = open_it(line, fn);
       append_log = 0;
    }

    llt = 0.0F;
    if(file == 0) {  // file not found
       sprintf(out, "Cannot open file: %s", fn);
       edit_error(out);
       pause_data = temp_pause;
       return 1;
    }

    strcpy(fn, line);
    if(cmd_line && (strstr(read_log, ".scr") == 0) && (strstr(read_log, ".SCR") == 0)) {
       pause_data ^= 1;  
       temp_pause ^= 1;
    }

    if(strstr(line, ".adv") || strstr(line, ".ADV")) {  // file is an adev file
       sprintf(out, "Reading adev file: %s", line);
       read_adev_file:
       adev_log = 3;
       if(showing_adv_file == 0) {
          xxx_adev_period = adev_period;
          old_keep_fresh = keep_adevs_fresh;
       }
       adev_period = 1.0;       // aaaaaapppppp other periods 
       keep_adevs_fresh = 0;
// hours = minutes = seconds = day = month = year = 0;
// afile = topen("adev.adv", "w");
//if(afile) fprintf(afile, "#\n");
    }
#ifdef GREET_STUFF
    else if(strstr(line, ".cal")  || strstr(line, ".CAL")) { // file is a calendar file
       read_calendar(line, 0);
       return 0;
    }
#endif
    else if(strstr(line, ".cfg") || strstr(line, ".CFG")) { // file is a .CFG config file
       fclose(file);
       read_config_file(fn, 0, 0);
       if(temp_script) {  // heather.cfg created a temporary keyboard script file, process it
          fclose(temp_script);
          temp_script = 0;
          open_script(TEMP_SCRIPT);
       }
       return 6;
    }
    else if(strstr(line, ".gpx") || strstr(line, ".GPX")) {  // file is a GPX format log
       xml_log_fmt = GPX;
       if(rcvr_type != CS_RCVR) sim_file_read |= 0x02;
if(time_flags == 0) time_flags = TFLAGS_UTC;  //lfs
       goto do_log;
    }
    else if(strstr(line, ".kml") || strstr(line, ".KML")) {  // file is a KML format log
       xml_log_fmt = KML;
       goto do_log;
    }
    else if(strstr(line, ".lla") || strstr(line, ".LLA")) { // file is a lat/lon/altitude file
       lla_log = 3;
       plot_lla = 1;
       change_zoom_config(1);
       zoom_screen = 'L'; 
       reading_lla = 1;
       if(rcvr_type == TICC_RCVR) all_adevs = ALL_CHANS;
       else                       all_adevs = SINGLE_ADEVS;
       reset_first_key(10);
       prot_menu = 0;
       plot_signals = 0;
       if((shared_plot == 0) && (WIDE_SCREEN == 0)) plot_azel = 0;
       if(SCREEN_WIDTH < NARROW_SCREEN) {
          shared_plot = plot_azel = 1;
          update_azel = 1;
       }
       config_screen(40);

       old_fixes = show_fixes;
       show_fixes = 1;
       redraw_screen();
       show_fixes = old_fixes;

       sprintf(out, "Reading lat/lon/alt file: %s", line);
//start_precision_survey(1);  // medsurv
    }
    else if(strstr(line, ".raw") || strstr(line, ".RAW")) {  // file is a raw capture file, read as a simulation input
       fclose(file);
       read_rawfile(fn);
       return 4;
    }
    else if(strstr(line, ".rpn") || strstr(line, ".RPN")) {  // file is calculator user defined operations
       fclose(file);
       err = read_rpn_file(fn, 0);
       return 4;
    }
    else if(strstr(line, ".scr") || strstr(line, ".SCR")) {  // file is a script file
       fclose(file);
       open_script(fn);
       return 4;
    }
#ifdef SIG_LEVELS
    else if(strstr(line, ".sig") || strstr(line, ".SIG")) { // file is a signal level file
       read_signals(line);
       return 0;
    }
#endif
    else if(strstr(line, ".prn") || strstr(line, ".PRN")) { // file is a sat az/el/signal level file
       read_prn(line);
       return 0;
    }
    else if(strstr(line, ".tim") || strstr(line, ".TIM")) {  // file is a TI.EXE .TIM file
       tim_file = 1;
       sprintf(out, "Reading TI.EXE .tim file: %s", line);
       goto read_adev_file;
    }
    else if(strstr(line, ".tie") || strstr(line, ".TIE")) {  // file is a Time Interval Error file
       read_tie_file(line);
       return 5;
    }
    else if(strstr(line, ".wav") || strstr(line, ".WAV")) {  // file is a audio file
       fclose(file);
       play_user_sound(fn);
       return 5;
    }
    else if(strstr(line, ".xml") || strstr(line, ".XML")) {  // file is a XML format log
       xml_log_fmt = XML;
       goto do_log;
    }
    else {    // file is a data log file
       do_log:
       sprintf(out, "Reading log file: %s", line);
       if(rcvr_type != CS_RCVR) sim_file_read |= 0x02;
if(time_flags == 0) time_flags = TFLAGS_UTC;  //lfs
       if(append_log == 0) {  // erase current markers
          for(log_mark_number=0; log_mark_number<MAX_MARKER; log_mark_number++) {
             mark_q_entry[log_mark_number] = 0;
          }
       }
    }

    if(reading_lla && (zoom_screen == 'L')) {
       vidstr(TEXT_ROWS-5, TEXT_COLS/2, PROMPT_COLOR, out);
    }
    else {
       vidstr(row, col, PROMPT_COLOR, out);
    }
    refresh_page();

    log_loaded = 1;
    if(log_comments) valid_read_log = 0;
    else             valid_read_log = 1;

    new_const = 0;
    read_log_interval = (DATA_SIZE) 1.0;
    have_time = 3;
    counter = 0;
    pause_data = 1;

    restore_plot_config();
    if(append_log == 0) {
       reset_queues(RESET_ALL_QUEUES, 1100);    // clear out the old data if not appending logs
    }

    reading_log = 1;
    time_checked = 0;
    while(fgets(line, sizeof line, file) != NULL) {  // for each line in the log file
        update_pwm();     // if doing pwm temperature control
        serve_os_queue(); // so keypress check works

        if(script_file) ;
        else if(KBHIT()) {  // user pressed a key, pause or stop reading the log
           GETCH();
           i = edit_error("Reading paused...  press ESC to stop");
           if(i == ESC_CHAR) break;
           k = TEXT_COLS-80;
           if(k < 0) k = 0;
           if(reading_lla && (zoom_screen == 'L')) {
              vidstr(TEXT_ROWS-4, TEXT_COLS/2, PROMPT_COLOR, &blanks[k]);
           }
           else {
              vidstr(EDIT_ROW+3, EDIT_COL, PROMPT_COLOR, &blanks[k]);
           }
           refresh_page();
        }

        if((counter == 0) && xml_log_fmt) {  // validate the log file by checking the first line
           if(!strstr(line, "<?xml")) {
              edit_error(".XML file format not recognized.  First line must start with '<?xml'.");
              goto not_log;
           }
           valid_read_log = 1;
        }
        else if((counter == 0) && (line[0] != '#') && (tim_file == 0) && log_comments) {  // it ain't no log file
           edit_error("File format not recognized.  First line must start with '#'.");

           not_log:
           fclose(file);
           pause_data = temp_pause;
           lla_log = 0;
           reading_lla = 0;
           reading_log = 0;
           time_checked = 0;
           return 2;
        }

        if((++counter % LOG_SCREEN_RFSH) == 1L) {  // show progress info
           if(counter == 1) sprintf(out, "Line %ld", counter);
           else             sprintf(out, "Line %ld", counter-1L);
           if(reading_lla && (zoom_screen == 'L')) {
              vidstr(TEXT_ROWS-2, TEXT_COLS/2, PROMPT_COLOR, out);
           }
           else {
              vidstr(row+3, col, PROMPT_COLOR, out);
           }
           refresh_page();
        }

        // Parse the log file comment lines for relevent data.
        if(parse_log_comment(line)) continue;

        // filter out or parse lines that don't start with numbers
        if(xml_log_fmt) goto data_line;

        j = strlen(line);
        for(i=0; i<j; i++) {  // scan the line for a number or command
           if((line[i] == ' ') || (line[i] == '\t')) {  // skip whitespace
              continue;
           }
           else if((line[i] >= '0') && (line[i] <= '9')) {
if(tim_file && (saw_timelab_tic == 0)) break;  // we are not in the TIC section
              goto data_line;
           }
           else if((line[i] == '.') || (line[i] == '+') || (line[i] == '-')) {
if(tim_file && (saw_timelab_tic == 0)) break;  // we are not in the TIC section
              goto data_line;
           }
           else if(tim_file) {
              if(tim_command(&line[i])) break;
           }
           else break;  // line does not start with something we are interested in
        }
        continue;  // get next log line

        data_line:  // the line has data we want to process on it
        if(lla_log) {  // read lat/lon/altitude info
           #ifdef PRECISE_STUFF
              sscanf(line, "%u %X %lf %lf %lf", &this_tow, &gps_status, &lat, &lon, &alt);
              if(gps_status == GPS_FIXES) {
                 if(lla_seen == 0) plot_lla_axes(1);
                 lla_seen = 1;
                 lat /= RAD_TO_DEG;
                 lon /= RAD_TO_DEG;
                 color = counter / (60L*60L);
                 color %= 12;
                 plot_lla_point(1, color+3);  // ckckckck - what about earth tides?
                 survey_tow = this_tow;
              }
           #endif  // PRECISE_STUFF
        }
        else if(adev_log) {  // read adev values
           pps_val = osc_val = chc_val = chd_val = 0.0;
           sscanf(line, "%le %le %le %le", &pps_val, &osc_val, &chc_val, &chd_val);
           if(have_ref == 0) {  // we can remove the first data point as a constant offset from all points
              pps_ref = pps_val;
              osc_ref = osc_val;
              chc_ref = chc_val;
              chd_ref = chc_val;
              have_ref = 1;
           }

           if(pps_val != 0.0) have_pps_offset = 1;
           if(osc_val != 0.0) have_osc_offset = 1;
           if(chc_val != 0.0) have_chc_offset = 1;
           if(chd_val != 0.0) have_chd_offset = 1;

           if(subtract_base_value == 2) {
              pps_val -= pps_ref;
              osc_val -= osc_ref;
              chc_val -= chc_ref;
              chd_val -= chd_ref;
           }

           if(luxor) ;
           else pps_val /= (1.0e-9);           // convert to ns
           pps_offset = (pps_val * log_pps_scale);

           if(luxor) ;
           else osc_val /= (100.0 * 1.0e-9);   // aaahhh
           osc_offset = (osc_val * log_osc_scale);

           if(luxor) ;
           else chc_val /= (1.0e-9);           // convert to ns
           chc_offset = (chc_val * log_chc_scale);

           if(luxor) ;
           else chd_val /= (1.0e-9);           // convert to ns
           chd_offset = (chd_val * log_chd_scale);


           #ifdef ADEV_STUFF
              add_rcvr_adev_point(2);
           #endif

           if(1) {
//            dac_voltage = 0.0F;
//            temperature = 0.0F;
              bump_time();
//if(afile) fprintf(afile, "%.11le %.11le\n", pps_offset, osc_offset);
              old_jd = jd_utc;
if(tim_file) {
   pps_tie = pps_offset;
   osc_tie = osc_offset;
   chc_tie = chc_offset;
   chd_tie = chd_offset;
   pps_offset += (pps_adev_period * 1.0E9);   // TICC_RATE
   osc_offset += (osc_adev_period * 1.0E9);
   chc_offset += (chc_adev_period * 1.0E9);
   chd_offset += (chd_adev_period * 1.0E9);
   read_log_jd += jtime(0,0,0, adev_period);
   jd_utc = read_log_jd;
}
              update_plot(NO_REFRESH);
              jd_utc = old_jd;
           }
        }
        else {  // read log info
           if(valid_read_log == 0) goto not_log;

           if(xml_log_fmt) {   // .GPX and .XML logs
              if(parse_xml_value(line)) continue;
           }
           else if(luxor) {
              #ifdef DOUBLE_DATA
                 if(old_log_format) {   // old log format
                    sscanf(line, "%02d%c%02d%c%lf%c%ld%c%lf%c%lf%c%lf%c%lf%c%lf%c%lf%c%lf%c%lf", 
                      &log_hhh,&ti[0],&log_mmm,&ti[0],&log_sss,&ti[0], 
                      &log_tow,&ti[0], 
                      &vp,&ti[0],               // aaalll
                      &vo,&ti[0],
                      &dac_voltage,&ti[0],
                      &temperature,&ti[0],
                      &blue_hz,&ti[0],
                      &green_hz,&ti[0],
                      &red_hz,&ti[0],
                      &white_hz
                    );
                 }
                 else {
                    sscanf(line, "%02d%c%02d%c%lf%c%ld%c%lf%c%lf%c%lf%c%lf%c%lf%c%lf%c%lf%c%lf%c%lf%c%lf%c%lf%c%lf%c%lf%c%lf", 
                      &log_hhh,&ti[0],&log_mmm,&ti[0],&log_sss,&ti[0], 
                      &log_tow,&ti[0], 
                      &vp,&ti[0],               // aaalll 
                      &lux2,&ti[0],
                      &vo,&ti[0],
                      &dac_voltage,&ti[0],
                      &led_i,&ti[0],
                      &led_v,&ti[0],
                      &temperature,&ti[0],
                      &luxor_tc2,&ti[0],
                      &blue_hz,&ti[0],
                      &green_hz,&ti[0],
                      &red_hz,&ti[0],
                      &white_hz,&ti[0],
                      &pwm_hz,&ti[0],
                      &adc2
                    );
                 }
              #else
                 if(old_log_format) {   // old log format
                    sscanf(line, "%02d%c%02d%c%lf%c%ld%c%lf%c%lf%c%f%c%f%c%f%c%f%c%f%c%f", 
                      &log_hhh,&ti[0],&log_mmm,&ti[0],&log_sss,&ti[0], 
                      &log_tow,&ti[0], 
                      &vp,&ti[0],               // aaalll
                      &vo,&ti[0],
                      &dac_voltage,&ti[0],
                      &temperature,&ti[0],
                      &blue_hz,&ti[0],
                      &green_hz,&ti[0],
                      &red_hz,&ti[0],
                      &white_hz
                    );
                 }
                 else {
                    sscanf(line, "%02d%c%02d%c%lf%c%ld%c%lf%c%f%c%lf%c%f%c%f%c%f%c%f%c%f%c%f%c%f%c%f%c%f%c%f%c%f", 
                      &log_hhh,&ti[0],&log_mmm,&ti[0],&log_sss,&ti[0], 
                      &log_tow,&ti[0], 
                      &vp,&ti[0],               // aaalll 
                      &lux2,&ti[0],
                      &vo,&ti[0],
                      &dac_voltage,&ti[0],
                      &led_i,&ti[0],
                      &led_v,&ti[0],
                      &temperature,&ti[0],
                      &luxor_tc2,&ti[0],
                      &blue_hz,&ti[0],
                      &green_hz,&ti[0],
                      &red_hz,&ti[0],
                      &white_hz,&ti[0],
                      &pwm_hz,&ti[0],
                      &adc2
                    );
                 }
              #endif   // DOUBLE_DATA
              cct = calc_cct(cct_type, 0, (double) red_hz, (double) green_hz, (double) blue_hz);
           }
           else if(rcvr_type == FURUNO_RCVR) {
              sscanf(line, "%02d%c%02d%c%lf%c%ld%c%lf%c%lf%c%lf%c%d", 
                &log_hhh,&ti[0],&log_mmm,&ti[0],&log_sss,&ti[0], 
                &log_tow,&ti[0], 
                &lat,&ti[0],
                &lon,&ti[0],
                &alt,&ti[0],
                &sat_count
              );
              lat = lat * PI / 180.0;
              lon = lon * PI / 180.0;
           }
           else if(rcvr_type == LPFRS_RCVR) {  //!!!!!LPFRS
              sscanf(line, "%02d%c%02d%c%lf%c%ld%c%lf%c%lf%c%lf%c%lf%c%d", 
                &log_hhh,&ti[0],&log_mmm,&ti[0],&log_sss,&ti[0], 
                &log_tow,&ti[0], 
                &vp,&ti[0],             // aaalll 
                &vo,&ti[0],
                &lpfrs_dd,&ti[0],
                &lpfrs_gg,&ti[0],
                &sat_count
              );
           }
           else if((rcvr_type == NMEA_RCVR) || (rcvr_type == GPSD_RCVR) || (rcvr_type == Z12_RCVR)) {
              sscanf(line, "%02d%c%02d%c%lf%c%ld%c%lf%c%lf%c%lf%c%d", 
                &log_hhh,&ti[0],&log_mmm,&ti[0],&log_sss,&ti[0], 
                &log_tow,&ti[0], 
                &lat,&ti[0],
                &lon,&ti[0],
                &alt,&ti[0],
                &sat_count
              );
              lat = lat * PI / 180.0;
              lon = lon * PI / 180.0;
           }
           else if((rcvr_type == NO_RCVR) || (rcvr_type == ACRON_RCVR)) { 
              sscanf(line, "%02d%c%02d%c%lf%c%ld", 
                &log_hhh,&ti[0],&log_mmm,&ti[0],&log_sss,&ti[0], 
                &log_tow 
              );
           }
           else if(rcvr_type == SCPI_RCVR) {
              sscanf(line, "%02d%c%02d%c%lf%c%ld%c%lf%c%lf%c%lf%c%lf%c%d", 
                &log_hhh,&ti[0],&log_mmm,&ti[0],&log_sss,&ti[0], 
                &log_tow,&ti[0], 
                &vp,&ti[0],             // aaalll 
                &vo,&ti[0],
                &vdac,&ti[0],
                &vtemp,&ti[0],
                &sat_count
              );
              dac_voltage = (DATA_SIZE) vdac;
              temperature = (DATA_SIZE) vtemp;
              vp /= 1.0E9;
              vo /= 1000.0;
           }
           else if(rcvr_type == SA35_RCVR) {  //!!!!!SA35
              sscanf(line, "%02d%c%02d%c%lf%c%ld%c%lf%c%lf%c%lf%c%lf%c%d", 
                &log_hhh,&ti[0],&log_mmm,&ti[0],&log_sss,&ti[0], 
                &log_tow,&ti[0], 
                &vp,&ti[0],             // aaalll 
                &vo,&ti[0],
                &lpfrs_dd,&ti[0],
                &lpfrs_gg,&ti[0],
                &sat_count
              );
           }
           else if(rcvr_type == SRO_RCVR) {
              sscanf(line, "%02d%c%02d%c%lf%c%ld%c%lf%c%lf%c%lf%c%d%c%d", 
                &log_hhh,&ti[0],&log_mmm,&ti[0],&log_sss,&ti[0], 
                &log_tow,&ti[0], 
                &vp,&ti[0],             // aaalll 
                &vo,&ti[0],
                &sro_ff,&ti[0],
                &sro_vt,&ti[0],
                &sat_count
              );
           }
           else if(rcvr_type == THERMO_RCVR) {
              sscanf(line, "%02d%c%02d%c%lf%c%ld%c%lf%c%lf%c%lf%c%lf%c%lf%c%lf", 
                &log_hhh,&ti[0],&log_mmm,&ti[0],&log_sss,&ti[0], 
                &log_tow,&ti[0], 
                &vtemp,&ti[0],
                &vtemp2,&ti[0],
                &vpressure,&ti[0],
                &vhumidity,&ti[0],
                &vadc3,&ti[0],
                &vadc4
              );
              tc1 = (DATA_SIZE) vtemp;
              tc2 = (DATA_SIZE) vtemp2;
              pressure = (DATA_SIZE) vpressure;
              humidity = (DATA_SIZE) vhumidity;
              adc3 = (DATA_SIZE) vadc3;
              adc4 = (DATA_SIZE) vadc4;
           }
           else if(rcvr_type == TICC_RCVR) {
              sscanf(line, "%02d%c%02d%c%lf%c%ld%c%lf%c%lf%c%lf%c%lf", 
                &log_hhh,&ti[0],&log_mmm,&ti[0],&log_sss,&ti[0], 
                &log_tow,&ti[0], 
                &vp,&ti[0],
                &vo,&ti[0],
                &vc,&ti[0],
                &vd
              );

              pps_phase = vp;   // aaalll
              osc_phase = vo;
              chc_phase = vc;
              chd_phase = vd;

              vp /= 1.0E9;
              vo /= 1.0E9;
              vc /= 1.0E9;
              vd /= 1.0E9;
           }
           else if(rcvr_type == TIDE_RCVR) { // ckckck
              sscanf(line, "%02d%c%02d%c%lf%c%ld%c%lf%c%lf%c%lf%c%lf", 
                &log_hhh,&ti[0],&log_mmm,&ti[0],&log_sss,&ti[0], 
                &log_tow,&ti[0], 
                &lat_tide,&ti[0],
                &lon_tide,&ti[0],
                &alt_tide,&ti[0],
                &ugals
              );
           }
           else if(rcvr_type == TSIP_RCVR) {
              sscanf(line, "%02d%c%02d%c%lf%c%ld%c%lf%c%lf%c%lf%c%lf%c%d", 
                &log_hhh,&ti[0],&log_mmm,&ti[0],&log_sss,&ti[0], 
                &log_tow,&ti[0], 
                &vp,&ti[0],             // aaalll 
                &vo,&ti[0],
                &vdac,&ti[0],
                &vtemp,&ti[0],
                &sat_count
              );
              dac_voltage = (DATA_SIZE) vdac;
              temperature = (DATA_SIZE) vtemp;

              pps_phase += ((vp * 1.0E9) / 100.0);   // aaalll
              osc_phase += ((vo * 1.0E0) / 100.0);
           }
           else {
              sscanf(line, "%02d%c%02d%c%lf%c%ld%c%lf%c%lf%c%lf%c%lf%c%d", 
                &log_hhh,&ti[0],&log_mmm,&ti[0],&log_sss,&ti[0], 
                &log_tow,&ti[0], 
                &vp,&ti[0],             // aaalll 
                &vo,&ti[0],
                &vdac,&ti[0],
                &vtemp,&ti[0],
                &sat_count
              );
              dac_voltage = (DATA_SIZE) vdac;
              temperature = (DATA_SIZE) vtemp;
           }

if(llt == (DATA_SIZE) 0.0) llt = temperature;
if(undo_fw_temp_filter) {
   tvb = (SENSOR_TC * temperature) - ((SENSOR_TC-1.0F) * llt);
   llt = temperature;
   temperature = tvb;
}

           osc_offset = vo;

           if(luxor) {
              pps_offset  = vp;
              batt_w = (DATA_SIZE) (dac_voltage * osc_offset);
              led_w = led_v * led_i;
           }
           else if(TIMING_RCVR) {
              pps_offset = vp;
           }
           else {
              pps_offset = vp * 1.0E9;
           }

           if(pps_offset) have_pps_offset = 1;
           if(osc_offset) have_osc_offset = 1;
           if(chc_offset) have_chc_offset = 1;
           if(chd_offset) have_chd_offset = 1;

           hours = log_hhh;
           minutes = log_mmm;
           seconds = (int) log_sss;
           frac = (log_sss - (double) seconds);

           tsip_error = msg_fault = 0;
           time_check(1, read_log_interval, year,month,day, hours,minutes,seconds,frac);
           jd_utc = jdate(year,month,day) + jtime(log_hhh,log_mmm,0,log_sss);  // set time code for log entry

           if(read_log_interval >= 1.0) j = (int) read_log_interval;
           else j = 1;

           old_jd = jd_utc;
           for(i=0; i<j; i++) {  // duplicate missing log entries
              update_plot(NO_REFRESH);
              #ifdef ADEV_STUFF
                 add_rcvr_adev_point(3);
              #endif
              jd_utc += jtime(0,0,0,1.0);  // !!!!!! what if not 1Hz queue?
           }
           jd_utc = old_jd;
        }

        // every so often,  process any pending GPS messages 
        // to keep the serial port buffer from overflowing
        if((counter % LOG_GPS_CHECK) == 0) {  
           temp_dmode = discipline_mode;
           temp_tflags = time_flags;
           temp_day = day;   temp_month = month;   temp_year = year;

           get_pending_gps(20);  //!!!! possible recursion

           day = temp_day;   month = temp_month;   year = temp_year;
           time_flags = temp_tflags;
           discipline_mode = temp_dmode;
        }
    }

    pause_data = temp_pause;
    jd_utc = temp_utc;

    reading_log = 0;
    time_checked = 0;
    reading_lla = 0;

    if(saw_log_title == 0) {
       sprintf(plot_title, "From file: %s", fn);
       title_type = USER;
       show_title();
       refresh_page();
    }

    #ifdef ADEV_STUFF
       find_global_max();
    #endif

    if(file) fclose(file);

    if(lla_log) {
       lla_log = 0;
       return 3;
    }
    else if(adev_log) {
       adev_log = 0;
       if(showing_adv_file == 0) {  // remember the current plot setup
          showing_adv_file = 1;
          for(i=0; i<NUM_PLOTS+DERIVED_PLOTS; i++) {
             plot[i].old_show = plot[i].show_plot;
          }
          old_sat_plot = plot_sat_count;
          old_adev_plot = plot_adev_data;
       }
       for(i=0; i<NUM_PLOTS+DERIVED_PLOTS; i++) {
          plot[i].show_plot = 0;
       }
       plot[PPS].show_plot = 1;
       plot[OSC].show_plot = have_osc_offset;
       plot_sat_count = 0;
       plot_adev_data = 1;
       keep_adevs_fresh = 0;  // allow adevs to be calculated over all points
       return 0;
    }
    return 0;
}


//
//
// RINEX file stuff
//
//

int gps_prn(int prn)
{
   if((prn >= 1) && (prn <= 32)) return prn;
   return 0;
}

int gps_sat(int prn)
{
   // return RINEX GNSS system id character if sat is a GPS satellite
   if(gps_prn(prn)) return 'G';
   return 0;
}


int sbas_prn(int prn)
{
   // convert receiver PRN id to RINEX standard values
   // Ublox IMES is 183 .. 192
   // Ublox QZSS is 193 .. 197

   if(rcvr_type == GPSD_RCVR) {  // we don't know the actual receiver type GPSD is using
      return 0;
   }
   else if(rcvr_type == ESIP_RCVR) {
      if((prn >= 33) && (prn < 64)) {
         return prn + (120-33) - 100;
      }
   }
   else if(rcvr_type == NMEA_RCVR) {  
      if((prn >= 33) && (prn < 64)) {
         return (prn+87) - 100;
      }
   }
   else if(rcvr_type == TSIP_RCVR) {
      return 0;
   }
   else if(rcvr_type == VENUS_RCVR) {  // ???? is this correct
      if((prn >= 33) && (prn < 64)) {
         return (prn+87) - 100;
      }
   }
   else if((prn >= 120) && (prn <= 158))  {
      return prn-100;
   }

   return 0;
}

int sbas_sat(int prn)
{
   // return RINEX GNSS system id character if sat is a SBAS satellite
   if(sbas_prn(prn)) return 'S';
   return 0;
}


int glonass_prn(int prn)
{
   if(rcvr_type == GPSD_RCVR) { // we don't know the actual receiver type GPSD is using 
      return 0;
   }
   else if(rcvr_type == ESIP_RCVR) {
      if(prn < 65) return 0;
      if(prn > 92) return 0;
   }
   else {
      if(prn < 65) return 0;
      if(prn > 96) return 0;
   }
   return (prn-64);
}

int glonass_sat(int prn) 
{
   // return RINEX GNSS system id character if sat is a Glonass satellite
   if(glonass_prn(prn)) return 'R';
   return 0;
}


int galileo_prn(int prn)
{
   if(rcvr_type == UBX_RCVR) {
      if((prn >= 211) && (prn <= 246)) return prn-210;
   }
   else if(rcvr_type == VENUS_RCVR) {
      if((prn >= 301) && (prn <= 350)) return prn-300;
   }
   else if(0 && (rcvr_type == TSIP_RCVR)) {
      // 183, 192,193,200
      if((prn >= 97) && (prn <= 133)) return prn-96;
   }
   return 0;  // not yet supported
}

int galileo_sat(int prn) 
{
   // return RINEX GNSS system id character if sat is a Galileo satellite
   if(galileo_prn(prn)) return 'E'; 
   return 0;
}


int beidou_prn(int prn)
{
   // not supported in RINEX before v3.0
   if(1 && (rcvr_type == UBX_RCVR)) {
      if((prn >= 159) && (prn <= 163)) return prn-158;
      else if((prn >= 33) && (prn <= 64)) return 6+(prn-33);
   }
   else if(rcvr_type == VENUS_RCVR) {
      if((prn >= 201) && (prn <= 237)) return prn-200;
   }
   else if(0 && (rcvr_type == TSIP_RCVR)) {
      if((prn >= 201) && (prn <= 237)) return prn-200;
   }
   return 0;  // not yet supported
}

int beidou_sat(int prn)
{
   // return RINEX GNSS system id character if sat is a Beidou satellite
   if(beidou_prn(prn)) return 'C'; 
   return 0;
}


int qzss_prn(int prn)
{
   // returns the RINEX PRN id code if sat is a QZSS satellite
   if(rcvr_type == ESIP_RCVR) {
      if((prn >= 93) && (prn < 199)) return prn-92;
   }
   else if(rcvr_type == TSIP_RCVR) {
      // 183, 192, 193, 200
   }
   else if(rcvr_type == UBX_RCVR){
      if((prn >= 193) && (prn <= 197)) return prn-192;
   }
   else if(rcvr_type == VENUS_RCVR){
      if((prn >= 193) && (prn <= 199)) return prn-192;
   }

   return 0;
}

int qzss_sat(int prn)
{
   // return RINEX GNSS system id character if sat is a QZSS satellite
   if(qzss_prn(prn)) return 'J'; 
   return 0;
}


int irnss_prn(int prn)
{
   // returns the RINEX PRN id code if sat is a IRNSS satellite
   // VENUS IRNSS PRNs -> PRN+240 .. PRN+???
   if(rcvr_type == VENUS_RCVR){
      if((prn >= 240) && (prn <= 254)) return prn-240;
   }

   return 0;
}

int irnss_sat(int prn)
{
   // return RINEX GNSS system id character if sat is a IRNSS satellite
   if(irnss_prn(prn)) return 'I'; 
   return 0;
}


int rinex_prn(int prn) 
{
   // convert Heather's PRN to the RINEX standard PRN and set gnss_sat to
   // Heather's GNSS id flag for the PRN
   gnss_sat = 0;
   if(gps_sat(prn)) {
      gnss_sat = GPS;
      return prn;
   }
   else if(sbas_sat(prn)) {     // sat is an SBAS satellite
      prn = sbas_prn(prn); // convert GPS SBAS prn to RINEX SBAS prn value
      gnss_sat = SBAS;
      if(rcvr_type == ESIP_RCVR) return 0;  // does not do SBAS raw measurements
      if(rcvr_type == NVS_RCVR) return 0;   // does not do SBAS raw measurements
      if(rcvr_type == VENUS_RCVR) return 0; // does not do SBAS raw measurements
      return prn;
   }
   else if(glonass_sat(prn)) {
      prn = glonass_prn(prn);
      gnss_sat = GLONASS;
      if(rcvr_type == ESIP_RCVR) return 0;  // does not do GLONASS raw measurements 
      if(rcvr_type == TSIP_RCVR) return 0;
      return prn;
   }
   else if(galileo_sat(prn)) {
      prn = galileo_prn(prn);
      gnss_sat = GALILEO;
      if(rcvr_type == TSIP_RCVR) return 0;
      return prn;
   }
   else if(beidou_sat(prn)) {
      prn = beidou_prn(prn);
      gnss_sat = BEIDOU;
      if(rcvr_type == TSIP_RCVR) return 0;
      return prn;
   }
   else if(qzss_sat(prn)) {
      prn = qzss_prn(prn);
      gnss_sat = QZSS;
      if(rcvr_type == TSIP_RCVR) return 0;
      return 0; //  return prn;  // we don't do IRNSS raw data yet
   }
   else if(irnss_sat(prn)) {
      prn = irnss_prn(prn);
      gnss_sat = IRNSS;
      if(rcvr_type == TSIP_RCVR) return 0;
      return 0; //  return prn;  // we don't do IRNSS raw data yet
   }

   return 0;  // default for GPS
}

int rinex_gnss(int prn)
{
   // returns Heather's GNSS id flag for the PRN
   rinex_prn(prn);
   return gnss_sat;
}

char gnss_letter(int prn)
{
char c;

   rinex_gnss(prn);
   if(rcvr_type == GPSD_RCVR)  c = ' ';
   else if(gnss_sat & GPS)     c = 'G';
   else if(gnss_sat & BEIDOU)  c = 'C';
   else if(gnss_sat & GALILEO) c = 'E';
   else if(gnss_sat & IRNSS)   c = 'I';
   else if(gnss_sat & QZSS)    c = 'J';
   else if(gnss_sat & IMES)    c = 'M';
   else if(gnss_sat & GLONASS) c = 'R';
   else if(gnss_sat & SBAS)    c = 'S';
   else                        c = ' ';

   return c;
}

char *gnss_id(int prn)
{
char *s;

   rinex_gnss(prn);
   if(rcvr_type == GPSD_RCVR)  s = "PRN";
   else if(gnss_sat & GPS)     s = "GPS";
   else if(gnss_sat & BEIDOU)  s = "BEIDOU";
   else if(gnss_sat & GALILEO) s = "GALILEO";
   else if(gnss_sat & IRNSS)   s = "IRNSS";
   else if(gnss_sat & QZSS)    s = "QZSS";
   else if(gnss_sat & IMES)    s = "IMES";
   else if(gnss_sat & GLONASS) s = "GLONASS";
   else if(gnss_sat & SBAS)    s = "SBAS";
   else                        s = "PRN";

   return s;
}


int good_raw(int prn)
{
   // return the RINEX system id char if it's a satellite we support
   if(sat[prn].level_msg == 0) return 0;  // sat is not tracked
   if(sat[prn].tracking <= 0) return 0;

   if(rinex_prn(prn)) {
      if     (gps_sat(prn))     return gps_sat(prn);      // it's a GPS sat
      else if(sbas_sat(prn))    return sbas_sat(prn);     // it's a SBAS sat
      else if(glonass_sat(prn)) return glonass_sat(prn);  // 
      else if(galileo_sat(prn)) return galileo_sat(prn);  // 
      else if(beidou_sat(prn))  return beidou_sat(prn);   // 
   }

   return 0;  // it's not OK to output the sat's RAW measurements
}


#define S1_OBS 1  // SNR
#define S2_OBS 2
#define S5_OBS 3
#define S6_OBS 4
#define S7_OBS 5
#define S8_OBS 6

#define D1_OBS 7  // DOPPLER
#define D2_OBS 8
#define D5_OBS 9
#define D6_OBS 10
#define D7_OBS 11
#define D8_OBS 12

#define C1_OBS 13 // PSEUDORANGE
#define P1_OBS 14
#define P2_OBS 15
#define P5_OBS 16
#define P6_OBS 17
#define P7_OBS 18
#define P8_OBS 19

#define L1_OBS 20 // CARRIER PHASE
#define L2_OBS 21
#define L5_OBS 22
#define L6_OBS 23
#define L7_OBS 24
#define L8_OBS 25

#define MAX_OBS 27
int mixed_obs_type[MAX_OBS+1];   // RINEX v2.11 mixed GNSS observations
int mixed_num_obs;

int gps_obs_type[MAX_OBS+1];     // RINEX v3.01 sat system specific observations
int gps_num_obs;

int sbas_obs_type[MAX_OBS+1];
int sbas_num_obs;

int glonass_obs_type[MAX_OBS+1];
int glonass_num_obs;

int galileo_obs_type[MAX_OBS+1];
int galileo_num_obs;

int beidou_obs_type[MAX_OBS+1];
int beidou_num_obs;


void add_obs_val(FILE *file, char *meas, int *obs_count)
{
int max_obs;

   // add the observation value to the output string

   if(file == 0) return;
   if(meas == 0) return;
   if(obs_count == 0) return;

   max_obs = 5;
   if(rinex_fmt >= 3.0) max_obs = 100;

   fprintf(file, "%s", meas);
   *obs_count = (*obs_count)+1;
   if(*obs_count >= max_obs) {
      fprintf(file, "\n");
      out[0] = 0;
      *obs_count = 0;
      obs_data_written = 1;
   }
}


int add_obs_type(int *obs_type, int count, int obs)
{
int i;

   // add an observation ID to the observation type list

   if(obs_type == 0) return count;
   if(count >= MAX_OBS) return count;

   for(i=0; i<count; i++) {  // filter out duplicates
      if(obs_type[i] == obs) return count;
   }

   obs_type[count++] = obs;
   return obs;
}


int get_obs_types(FILE *file, char *s, int system, int *obs_type)
{
int i;
int len;
char obs[5+1];
char *sys;
unsigned ti;
int num_obs;
int max_obs; // max observation types per line

   // parse list of observation types to output and build the RINEX file
   // observation type list

   // !!!!!!!!!! needs changes for RINEX v3 !!!!!!!!!!!!!!

   num_obs = 0;
   if(file == 0) return num_obs;

   if(s == 0) s = "";

   len = strlen(s);
   if(len) ++len;

   ti = 0;
   obs[0] = 0;
   for(i=0; i<len; i++) {  // build obs list from user specified measurments
      if(obs[0] && (s[i] == 0)) goto got_obs;  // null observation type list

      if(isalpha(s[i]) || isdigit(s[i])) {    // build the next obs type string
         if(ti >= (sizeof(obs)-1)) continue;  // obs name is too long, clip it
         obs[ti++] = s[i];
         obs[ti] = 0;
      }
      else if(obs[0]) {  // got the obs type string, parse it
         got_obs:
         strupr(obs);

         // RINEX v2.11
         if     (strstr(obs, "S1")) num_obs = add_obs_type(obs_type, num_obs, S1_OBS);  // signal strength
         else if(strstr(obs, "S2")) num_obs = add_obs_type(obs_type, num_obs, S2_OBS);
         else if(strstr(obs, "S5")) num_obs = add_obs_type(obs_type, num_obs, S5_OBS);
         else if(strstr(obs, "S6")) num_obs = add_obs_type(obs_type, num_obs, S6_OBS);
         else if(strstr(obs, "S7")) num_obs = add_obs_type(obs_type, num_obs, S7_OBS);
         else if(strstr(obs, "S8")) num_obs = add_obs_type(obs_type, num_obs, S8_OBS);
                                                                                     
         else if(strstr(obs, "D1")) num_obs = add_obs_type(obs_type, num_obs, D1_OBS);  // doppler
         else if(strstr(obs, "D2")) num_obs = add_obs_type(obs_type, num_obs, D2_OBS);
         else if(strstr(obs, "D5")) num_obs = add_obs_type(obs_type, num_obs, D5_OBS);
         else if(strstr(obs, "D6")) num_obs = add_obs_type(obs_type, num_obs, D6_OBS);
         else if(strstr(obs, "D7")) num_obs = add_obs_type(obs_type, num_obs, D7_OBS);
         else if(strstr(obs, "D8")) num_obs = add_obs_type(obs_type, num_obs, D8_OBS);
                                                                                     
         else if(strstr(obs, "C1")) num_obs = add_obs_type(obs_type, num_obs, C1_OBS);  // pseudorange
         else if(strstr(obs, "L1")) num_obs = add_obs_type(obs_type, num_obs, L1_OBS);
         else if(strstr(obs, "L2")) num_obs = add_obs_type(obs_type, num_obs, L2_OBS);
         else if(strstr(obs, "L5")) num_obs = add_obs_type(obs_type, num_obs, L5_OBS);
         else if(strstr(obs, "L6")) num_obs = add_obs_type(obs_type, num_obs, L6_OBS);
         else if(strstr(obs, "L7")) num_obs = add_obs_type(obs_type, num_obs, L7_OBS);
         else if(strstr(obs, "L8")) num_obs = add_obs_type(obs_type, num_obs, L8_OBS);
                                                                                     
         else if(strstr(obs, "P1")) num_obs = add_obs_type(obs_type, num_obs, P1_OBS);  // carrier phase
         else if(strstr(obs, "P2")) num_obs = add_obs_type(obs_type, num_obs, P2_OBS);
         else if(strstr(obs, "P5")) num_obs = add_obs_type(obs_type, num_obs, P5_OBS);
         else if(strstr(obs, "P6")) num_obs = add_obs_type(obs_type, num_obs, P6_OBS);
         else if(strstr(obs, "P7")) num_obs = add_obs_type(obs_type, num_obs, P7_OBS);
         else if(strstr(obs, "P8")) num_obs = add_obs_type(obs_type, num_obs, P8_OBS);

         // RINEX v3.03  - !!!!! These need work !!!!!
         else if(strstr(obs, "S1C")) num_obs = add_obs_type(obs_type, num_obs, S1_OBS);  // signal strength
         else if(strstr(obs, "S2C")) num_obs = add_obs_type(obs_type, num_obs, S2_OBS);
         else if(strstr(obs, "S5C")) num_obs = add_obs_type(obs_type, num_obs, S5_OBS);
         else if(strstr(obs, "S6C")) num_obs = add_obs_type(obs_type, num_obs, S6_OBS);
         else if(strstr(obs, "S7C")) num_obs = add_obs_type(obs_type, num_obs, S7_OBS);
         else if(strstr(obs, "S8C")) num_obs = add_obs_type(obs_type, num_obs, S8_OBS);
                                                                                       
         else if(strstr(obs, "D1C")) num_obs = add_obs_type(obs_type, num_obs, D1_OBS);  // doppler
         else if(strstr(obs, "D2C")) num_obs = add_obs_type(obs_type, num_obs, D2_OBS);
         else if(strstr(obs, "D5C")) num_obs = add_obs_type(obs_type, num_obs, D5_OBS);
         else if(strstr(obs, "D6C")) num_obs = add_obs_type(obs_type, num_obs, D6_OBS);
         else if(strstr(obs, "D7C")) num_obs = add_obs_type(obs_type, num_obs, D7_OBS);
         else if(strstr(obs, "D8C")) num_obs = add_obs_type(obs_type, num_obs, D8_OBS);
                                                                                       
         else if(strstr(obs, "C1C")) num_obs = add_obs_type(obs_type, num_obs, C1_OBS);  // pseudorange
         else if(strstr(obs, "C1P")) num_obs = add_obs_type(obs_type, num_obs, L1_OBS);
         else if(strstr(obs, "C2P")) num_obs = add_obs_type(obs_type, num_obs, L2_OBS);
         else if(strstr(obs, "C5P")) num_obs = add_obs_type(obs_type, num_obs, L5_OBS);
         else if(strstr(obs, "C6P")) num_obs = add_obs_type(obs_type, num_obs, L6_OBS);
         else if(strstr(obs, "C7P")) num_obs = add_obs_type(obs_type, num_obs, L7_OBS);
         else if(strstr(obs, "C8P")) num_obs = add_obs_type(obs_type, num_obs, L8_OBS);
                                                                                       
         else if(strstr(obs, "L1C")) num_obs = add_obs_type(obs_type, num_obs, P1_OBS);  // carrier phase
         else if(strstr(obs, "L2C")) num_obs = add_obs_type(obs_type, num_obs, P2_OBS);
         else if(strstr(obs, "L5C")) num_obs = add_obs_type(obs_type, num_obs, P5_OBS);
         else if(strstr(obs, "L6C")) num_obs = add_obs_type(obs_type, num_obs, P6_OBS);
         else if(strstr(obs, "L7C")) num_obs = add_obs_type(obs_type, num_obs, P7_OBS);
         else if(strstr(obs, "L8C")) num_obs = add_obs_type(obs_type, num_obs, P8_OBS);

         else continue;

         ++num_obs;
         ti = 0;
         obs[0] = 0;
         if(num_obs >= MAX_OBS) break;
      }
      else obs[0] = 0;  // skip whitespace

      if(s[i] == 0) break;
   }

   if(num_obs == 0) {  // build obs list from data type seen
      if(have_range & system)       obs_type[num_obs++] = C1_OBS;
      else if((have_phase & system) && use_rinex_cppr) {
         obs_type[num_obs++] = C1_OBS;
      }
      if(have_l1_range & system)    obs_type[num_obs++] = P1_OBS;
      if(have_l2_range & system)    obs_type[num_obs++] = P2_OBS;
      if(have_l5_range & system)    obs_type[num_obs++] = P5_OBS;
      if(have_l6_range & system)    obs_type[num_obs++] = P6_OBS;
      if(have_l7_range & system)    obs_type[num_obs++] = P7_OBS;
      if(have_l8_range & system)    obs_type[num_obs++] = P8_OBS;

      if((have_phase & system) || (have_l1_phase & system)) obs_type[num_obs++] = L1_OBS;
      if(have_l2_phase & system)    obs_type[num_obs++] = L2_OBS;
      if(have_l5_phase & system)    obs_type[num_obs++] = L5_OBS;
      if(have_l6_phase & system)    obs_type[num_obs++] = L6_OBS;
      if(have_l7_phase & system)    obs_type[num_obs++] = L7_OBS;
      if(have_l8_phase & system)    obs_type[num_obs++] = L8_OBS;

      if(rcvr_type == MOTO_RCVR) ;        // doppler value is unusable for RINEX 
      else if(rcvr_type == VENUS_RCVR) ;  // doppler value is unusable for RINEX
      else {
         if((have_doppler & system) || (have_l1_doppler & system)) {
            obs_type[num_obs++] = D1_OBS;
         }
         if(have_l2_doppler & system)  obs_type[num_obs++] = D2_OBS;
         if(have_l5_doppler & system)  obs_type[num_obs++] = D5_OBS;
         if(have_l6_doppler & system)  obs_type[num_obs++] = D6_OBS;
         if(have_l7_doppler & system)  obs_type[num_obs++] = D7_OBS;
         if(have_l8_doppler & system)  obs_type[num_obs++] = D8_OBS;
      }

      if(have_snr & system)         obs_type[num_obs++] = S1_OBS;
      else if(have_l1_snr & system) obs_type[num_obs++] = S1_OBS;
      if(have_l2_snr & system)      obs_type[num_obs++] = S2_OBS;
      if(have_l5_snr & system)      obs_type[num_obs++] = S5_OBS;
      if(have_l6_snr & system)      obs_type[num_obs++] = S6_OBS;
      if(have_l7_snr & system)      obs_type[num_obs++] = S7_OBS;
      if(have_l8_snr & system)      obs_type[num_obs++] = S8_OBS;
   }

   if(rinex_fmt >= 3.0) {
      if     (system & GPS)     sys = "G";
      else if(system & SBAS)    sys = "S";
      else if(system & GLONASS) sys = "R";
      else if(system & GALILEO) sys = "E";
      else if(system & BEIDOU)  sys = "C";
      else                      sys = "?";
      sprintf(out, "%s%5d", sys, num_obs);

      max_obs = 13;
      sys = "SYS / # / OBS TYPES";

      len = 0;
      for(i=0; i<num_obs; i++) {  // build the RINEX file observation list
         // !!!!!!!! THESE ARE TEMPORARY TEST VALUES... NEED TO BE UPDATED TO REAL ONES !!!!!!!!!
         if     (obs_type[i] == S1_OBS) { strcat(out, " S1C"); ++len; }  // sig level
         else if(obs_type[i] == S2_OBS) { strcat(out, " S2C"); ++len; } 
         else if(obs_type[i] == S5_OBS) { strcat(out, " S5C"); ++len; } 
         else if(obs_type[i] == S6_OBS) { strcat(out, " S6C"); ++len; } 
         else if(obs_type[i] == S7_OBS) { strcat(out, " S7C"); ++len; } 
         else if(obs_type[i] == S8_OBS) { strcat(out, " S8C"); ++len; } 

         else if(obs_type[i] == D1_OBS) { strcat(out, " D1C"); ++len; }  // dopple
         else if(obs_type[i] == D2_OBS) { strcat(out, " D2C"); ++len; } 
         else if(obs_type[i] == D5_OBS) { strcat(out, " D5C"); ++len; } 
         else if(obs_type[i] == D6_OBS) { strcat(out, " D6C"); ++len; } 
         else if(obs_type[i] == D7_OBS) { strcat(out, " D7C"); ++len; } 
         else if(obs_type[i] == D8_OBS) { strcat(out, " D8C"); ++len; } 

         else if(obs_type[i] == C1_OBS) { strcat(out, " C1C"); ++len; }  // pseudorange
         else if(obs_type[i] == P1_OBS) { strcat(out, " C1P"); ++len; } 
         else if(obs_type[i] == P2_OBS) { strcat(out, " C2P"); ++len; } 
         else if(obs_type[i] == P5_OBS) { strcat(out, " C5P"); ++len; } 
         else if(obs_type[i] == P6_OBS) { strcat(out, " C6P"); ++len; } 
         else if(obs_type[i] == P7_OBS) { strcat(out, " C7P"); ++len; } 
         else if(obs_type[i] == P8_OBS) { strcat(out, " C8P"); ++len; } 

         else if(obs_type[i] == L1_OBS) { strcat(out, " L1C"); ++len; }  // carrier phase
         else if(obs_type[i] == L2_OBS) { strcat(out, " L2C"); ++len; } 
         else if(obs_type[i] == L5_OBS) { strcat(out, " L5C"); ++len; } 
         else if(obs_type[i] == L6_OBS) { strcat(out, " L6C"); ++len; } 
         else if(obs_type[i] == L7_OBS) { strcat(out, " L7C"); ++len; } 
         else if(obs_type[i] == L8_OBS) { strcat(out, " L8C"); ++len; } 


         if(len >= max_obs) {  // time for continuation line
            write_rinex_line(file, out, sys); 
            strcpy(out,"      ");
            len = 0;
         }
      }

      if(len) {  // flush the last observation list line
         write_rinex_line(file, out, sys); 
      }
   }
   else {
      sprintf(out, "%6d", num_obs);

      max_obs = 9;
      sys = "# / TYPES OF OBSERV";

      len = 0;
      for(i=0; i<num_obs; i++) {  // build the RINEX file observation list
         if     (obs_type[i] == S1_OBS) { strcat(out, "    S1"); ++len; }
         else if(obs_type[i] == S2_OBS) { strcat(out, "    S2"); ++len; } 
         else if(obs_type[i] == S5_OBS) { strcat(out, "    S5"); ++len; } 
         else if(obs_type[i] == S6_OBS) { strcat(out, "    S6"); ++len; } 
         else if(obs_type[i] == S7_OBS) { strcat(out, "    S7"); ++len; } 
         else if(obs_type[i] == S8_OBS) { strcat(out, "    S8"); ++len; } 

         else if(obs_type[i] == D1_OBS) { strcat(out, "    D1"); ++len; } 
         else if(obs_type[i] == D2_OBS) { strcat(out, "    D2"); ++len; } 
         else if(obs_type[i] == D5_OBS) { strcat(out, "    D5"); ++len; } 
         else if(obs_type[i] == D6_OBS) { strcat(out, "    D6"); ++len; } 
         else if(obs_type[i] == D7_OBS) { strcat(out, "    D7"); ++len; } 
         else if(obs_type[i] == D8_OBS) { strcat(out, "    D8"); ++len; } 

         else if(obs_type[i] == C1_OBS) { strcat(out, "    C1"); ++len; } 
         else if(obs_type[i] == P1_OBS) { strcat(out, "    P1"); ++len; } 
         else if(obs_type[i] == P2_OBS) { strcat(out, "    P2"); ++len; } 
         else if(obs_type[i] == P5_OBS) { strcat(out, "    P5"); ++len; } 
         else if(obs_type[i] == P6_OBS) { strcat(out, "    P6"); ++len; } 
         else if(obs_type[i] == P7_OBS) { strcat(out, "    P7"); ++len; } 
         else if(obs_type[i] == P8_OBS) { strcat(out, "    P8"); ++len; } 


         else if(obs_type[i] == L1_OBS) { strcat(out, "    L1"); ++len; } 
         else if(obs_type[i] == L2_OBS) { strcat(out, "    L2"); ++len; } 
         else if(obs_type[i] == L5_OBS) { strcat(out, "    L5"); ++len; } 
         else if(obs_type[i] == L6_OBS) { strcat(out, "    L6"); ++len; } 
         else if(obs_type[i] == L7_OBS) { strcat(out, "    L7"); ++len; } 
         else if(obs_type[i] == L8_OBS) { strcat(out, "    L8"); ++len; } 


         if(len >= max_obs) {  // time for continuation line
            write_rinex_line(file, out, sys); 
            strcpy(out,"      ");
            len = 0;
         }
      }

      if(len) {  // flush the last observation list line
         write_rinex_line(file, out, sys); 
      }
   }

   return num_obs;
}



void get_obs_time(double jd_obs, double obs_tow)
{
double o_frac;
double delta;
static double max_delta = 0.0;

   // Convert jd_obs to gregorian time values.  Since the gregorian() function
   // can have round-off errors the the range of 0.0001 seconds we do some
   // tom-foolery to attempt to use obs_tow value to get the seconds exactly
   // what the receiver sent (jd_obs is used to get the rest of the date/time)

   gregorian(1, jd_obs);

   o_frac = obs_tow - (double) (int) obs_tow;
   delta = fabs(g_frac - o_frac);
   if((delta > max_delta) && (delta < 0.5)) max_delta = delta;
   if(delta > (1.0-JD_OBS_ROUND)) delta = (1.0-delta);

if(show_debug_info) sprintf(debug_text4, "g_frac:%.7f  o_frac:%.7f  delta:%.7f", g_frac,o_frac, delta);
   if(delta < (JD_OBS_ROUND*1.0)) { // gregorian fraction is close to observation fraction
      // if the two fractions are on each side of the 1 second threshold we
      // cant substute the observation fraction for the gregorian fraction since
      // that could cause the seconds value to be wrong and cause a backwards
      // time step.
      if((o_frac < JD_OBS_ROUND) && (g_frac > (1.0-JD_OBS_ROUND))) {
         //  bump the year/month/day hours/minutes/seconds and use o_frac
         gregorian(1, jd_obs+jtime(0,0,0,0.1));  // bump time past any roundoff error
         g_frac = o_frac;
//if(rinex_file) write_rinex_line(rinex_file, "time bump!", "COMMENT");  //rnx
      }
      else if((g_frac < JD_OBS_ROUND) && (o_frac > (1.0-JD_OBS_ROUND))) {
         gregorian(1, jd_obs-jtime(0,0,0,0.1)); // do a negative time bump
         g_frac = o_frac;
//if(rinex_file) write_rinex_line(rinex_file, "negative time bump!", "COMMENT");  //rnx
      }
      else {  // it's safe to replace gregorian() value with obs_tow value
         g_frac = o_frac; 
      }
   }
if(show_debug_info) sprintf(debug_text3, "g_frac:%.7f  o_frac:%.7f  delta:%.7f  maxd:%.7f", g_frac,o_frac, delta,max_delta);
}

void write_rinex_header(FILE *file)
{
char *type;
char fmt[SLEN+1];
char core[SLEN+1];
int mask;
int i;
FILE *old_log;
int old_fmt;

   // write the RINEX .OBS file header info
   if(rinex_header_written) return;
   obs_data_written = 0;
   if(time_flags & TFLAGS_BAD_TIME) return;
   if(jd_obs == 0.0) return;
   if(first_obs) {
      first_obs = 0;
      return;
   }

   mask = gnss_mask;
   if(rcvr_type == ESIP_RCVR) mask = GPS;  // only does GPS raw measurements

   if     (mask == GPS)     type = "G (GPS)";
   else if(mask == GLONASS) type = "R (GLONASS)";
   else if(mask == SBAS)    type = "S (SBAS)";
   else if(mask == GALILEO) type = "E (GALILEO)";
   else if(mask == BEIDOU)  type = "C (BEIDOU)";
   else if(have_gnss_mask)  type = "M (MIXED)";
   else                     type = "M (MIXED)";  // "G (GPS)";
   sprintf(out,"     %4.2f           OBSERVATION DATA    %-20.20s", rinex_fmt, type);
   write_rinex_line(file, out, "RINEX VERSION / TYPE");

   gregorian(1, jd_utc);
   if(rinex_fmt >= 3.00) {
      sprintf(out,          "HEATHER %-9.9s                       %04d%02d%02d %02d%02d%02d UTC", VERSION, g_year,g_month,g_day, g_hours,g_minutes,g_seconds);
   }
   else {
      sprintf(out,          "HEATHER %-9.9s                       %04d%02d%02d %02d:%02d:%02dUTC", VERSION, g_year,g_month,g_day, g_hours,g_minutes,g_seconds);
   }
   write_rinex_line(file, out, "PGM / RUN BY / DATE"); 

   if((marker_name[0] == 0) || strstr(marker_name, DEFAULT_NAME)) {  // create a marker name
      sprintf(fmt, "%s ", unit_file_name);
      if(mask & GPS)     strcat(fmt, "G+");
      if(mask & SBAS)    strcat(fmt, "S+");
      if(mask & GLONASS) strcat(fmt, "R+");
      if(mask & GALILEO) strcat(fmt, "E+");
      if(mask & BEIDOU)  strcat(fmt, "C+");
//    if(mask & QZSS)    strcat(fmt, "J+");
//    if(mask & IRNSS)   strcat(fmt, "I+");
      i = strlen(fmt);
      if(i) {
         if(fmt[i-1] == '+') fmt[i-1] = ' ';
      }
      strupr(fmt);
   }
   else strcpy(fmt, marker_name);

   sprintf(out, "%-20.20s", fmt);
   write_rinex_line(file, out, "MARKER NAME");      

   sprintf(out, "%-20.20s", marker_number);
   write_rinex_line(file, out,  "MARKER NUMBER");      

   sprintf(out, "%-20.20s", marker_type);
   if(rinex_fmt >= 3.00) {
      write_rinex_line(file, out, "MARKER TYPE");      
   }

   write_rinex_line(file, "", "OBSERVER / AGENCY");      

   strcpy(fmt, unit_file_name);
   strupr(fmt);
   if(rcvr_type == ESIP_RCVR) {
      sprintf(out, "ID:%-17.17s%-20.20sVER:%-4.4s", esip_model, fmt, esip_version);
   }
   else if(rcvr_type == NVS_RCVR) {
      sprintf(out, "ID:%-17.17s%-20.20sFW:%-17.17s", nvs_id2, fmt, nvs_id);
   }
   else if(rcvr_type == TSIP_RCVR) {
      sprintf(core, "FW:%2d.%-2d %02d %s %02d", 
         core_major, core_minor,  core_day, months[core_month], core_year);  //!!! docs say 1900
         core[20] = 0;
      if(unit_name[0]) {
         sprintf(out, "%-20.20s%-20.20s%-20.20s", core, fmt, unit_name);
      }
      else {
         sprintf(out, "%-20.20s%-20.20s%-20.20s", core, fmt, "TRIMBLE TSIP");
      }
   }
   else if(rcvr_type == UBX_RCVR) {
      sprintf(out, "HW:%-17.17s%-20.20sFW:%-17.17s", ubx_hw, fmt, ubx_sw);
   }
   else if(rcvr_type == VENUS_RCVR) {
      sprintf(out, "%-20.20s%-20.20s%-20.20s", venus_odm, fmt, venus_rev);
   }
   else if(rcvr_type == Z12_RCVR) {
      sprintf(out, "TYPE:%-15.15s%-20.20sVER:%-4.4s %-4.4s-%-4.4s", z12_type, fmt, z12_chan_opt, z12_nav_ver,z12_chan_ver);
   }
   else {
      sprintf(out, "%-20.20s%-20.20s%-20.20s", "", fmt, "");
   }
   write_rinex_line(file, out, "REC # / TYPE / VERS"); 

   sprintf(out, "%-20.20s%-20.20s", antenna_number, antenna_type);
   write_rinex_line(file, out, "ANT # / TYPE");        
   sprintf(out, "  LAT:%14.9f  LON:%14.9f  ALT:%10.4f", lat*180.0/PI,lon*180.0/PI,alt), 
   write_rinex_line(file, out,  "COMMENT"); 
   lla_to_ecef(lat, lon, alt);
   sprintf(out, "%14.4f%14.4f%14.4f", ecef_x,ecef_y,ecef_z), 
   write_rinex_line(file, out,  "APPROX POSITION XYZ"); 

   sprintf(out, "%14.4f%14.4f%14.4f", antenna_height, antenna_ew, antenna_ns);
   if(rinex_fmt >= 3.0) {
      write_rinex_line(file, out, "ANTENNA:DELTA H/E/N");
   }
   else {
      write_rinex_line(file, out, "ANTENNA: DELTA H/E/N");
   }
   write_rinex_line(file, "     1     1", "WAVELENGTH FACT L1/2");

   if(0 && mixed_obs_list[0]) {  // user specifed observations
      mixed_num_obs = get_obs_types(file, mixed_obs_list, MIXED, &mixed_obs_type[0]);
   }
   else if(1) { // build observation list from data type seen
      if(rinex_fmt >= 3.0) {
         gps_num_obs     = get_obs_types(file, gps_obs_list,     GPS,     &gps_obs_type[0]);
         sbas_num_obs    = get_obs_types(file, sbas_obs_list,    SBAS,    &sbas_obs_type[0]);
         glonass_num_obs = get_obs_types(file, glonass_obs_list, GLONASS, &glonass_obs_type[0]);
         galileo_num_obs = get_obs_types(file, galileo_obs_list, GALILEO, &galileo_obs_type[0]);
         beidou_num_obs  = get_obs_types(file, beidou_obs_list,  BEIDOU,  &beidou_obs_type[0]);
      }
      else {
         mixed_num_obs = get_obs_types(file, mixed_obs_list, MIXED, &mixed_obs_type[0]);
      }
   }
   else if(USE_L2 && (rcvr_type == Z12_RCVR)) {
///   mixed_num_obs = get_obs_types(file, "C1    L1    D1    S1    P1    P2    L2    D2    S2", MIXED, mixed_obs_type);
      mixed_num_obs = get_obs_types(file, "L1    L2    C1    P1    P2    S1    S2", MIXED, &mixed_obs_type[0]);
   }
   else if(have_phase == 0) {  // (rcvr_type == ESIP_RCVR) 
     mixed_num_obs = get_obs_types(file, "C1    D1    S1", MIXED, &mixed_obs_type[0]); // pseudorange, doppler, snr
   }
   else {
      mixed_num_obs = get_obs_types(file, "C1    L1    D1    S1", MIXED, &mixed_obs_type[0]);
   }

   get_obs_time(jd_obs, obs_tow);
   sprintf(out, "%6d%6d%6d%6d%6d%13.7f%s", g_year,g_month,g_day, g_hours,g_minutes,(double)g_seconds+g_frac, "     GPS");
   write_rinex_line(file, out, "TIME OF FIRST OBS");

   if((rinex_fmt >= 3.02) && glonass_num_obs) {  // !!!!! do this properly
      fmt[0] = 0;
      if(1) sprintf(fmt, " C1C %8.3f", 0.0);
      else  sprintf(fmt, "             ");
      strcat(out, fmt);

      if(1) sprintf(fmt, " C1P %8.3f", 0.0);
      else  sprintf(fmt, "             ");
      strcat(out, fmt);

      if(1) sprintf(fmt, " C2C %8.3f", 0.0);
      else  sprintf(fmt, "             ");
      strcat(out, fmt);

      if(1) sprintf(fmt, " C2P %8.3f", 0.0);
      else  sprintf(fmt, "             ");
      strcat(out, fmt);
      write_rinex_line(file, out, "GLONASS COD/PHS/BIS");
   }


   write_rinex_line(file, " ",  "COMMENT"); 

   if(el_mask || amu_mask) {
      sprintf(out, "  Elevation mask: %4.1f    Signal mask: %.1f", el_mask, amu_mask);
      write_rinex_line(file, out,  "COMMENT"); 
   }

   if(have_tc) {
      sprintf(out, "  Time constant: %.3f seconds", time_constant);
      write_rinex_line(file, out,  "COMMENT"); 
   }
   if(have_damp) {
      sprintf(out, "  Damping: %.3f", damping_factor);
      write_rinex_line(file, out,  "COMMENT"); 
   }
   if(have_gain) {
      sprintf(out, "  Osc EFC gain: %.3f Hz/V", osc_gain);
      write_rinex_line(file, out,  "COMMENT"); 
   }
   if(have_initv) {
      sprintf(out, "  Initial EFC voltage : %.3f V", initial_voltage);
      write_rinex_line(file, out,  "COMMENT"); 
   }
   if(use_rinex_cppr) {
      sprintf(out, "  GPS/SBAS/GALILEO L1 pseudorange derived from carrier phase data");
      write_rinex_line(file, out,  "COMMENT"); 
   }

   rinex_header_written = 1;
   obs_data_written = 0;
   if(rinex_file) {
      old_log = log_file;
      old_fmt = log_fmt;

      log_file = rinex_file;
      log_fmt = RINEX;
      write_log_id();

      log_file = old_log;
      log_fmt = old_fmt;
   }

   write_rinex_line(file, "", "END OF HEADER");

   if(rinex_file && (file == rinex_file) && rinex_flush_mode) {
     fflush(file);
   }
}

static double last_jd_obs = 0.0;

int write_rinex_obs_header(FILE *file, int no_obs_output)
{
int i;
int no_obs;
int prn;
int p;
char gnss;
int frac;
int col_count;
int out_count;
static int last_hr = (-1);
static int last_min = (-1);
static int last_sec = (-1);
static double last_frac;
static int last_year = 0, last_month = 0,last_day = (-1);
static int last_hours = 0, last_minutes = 0 ,last_seconds = 0;

   // write observation data header for tracked sats
//NEW_RCVR

   if(file == 0) return 0;
   if(rinex_header_written == 0) return 0;

   i = 0;
   for(prn=1; prn<MAX_PRN; prn++) {  // count number of tracked sats
      if(good_raw(prn) == 0) continue;
      ++i;
   }
// if(i == 0) return 0;

   get_obs_time(jd_obs, obs_tow);

   if(g_frac >= 1.0) {
      goto jd_fault;
   }
   frac = (int) (g_frac * 1.0E7);

   if(last_hr != g_hours) ;
   else if(last_min != g_minutes) ;
   else if(last_sec != g_seconds) ;
   else if(last_frac != g_frac) ;
   else if(rinex_fix) {
      sprintf(out, "#! skipped duplicate RINEX observation timestamp!");
      write_rinex_line(file, out, "COMMENT");

      jd_fault:
      gregorian(1, last_jd_obs);
      if(debug_file) fprintf(debug_file, "#! skipped duplicate RINEX observation:  last: %02d:%02d:%02d g_frac:%f\n", g_hours,g_minutes,g_seconds, g_frac);
      gregorian(1, jd_obs);
      if(debug_file) fprintf(debug_file, "                                         this: %02d:%02d:%02d g_frac:%f\n", g_hours,g_minutes,g_seconds, g_frac);
      return 0;
   }

   last_hr = g_hours;     // another duplicate data block prevention check
   last_min = g_minutes;
   last_sec = g_seconds;
   last_frac = g_frac;


#define GOOD_LLI ' '  // '4'
#define BAD_LLI  '1'  // '5'
#define P_MODE   '4'  // tracking P code

   if(have_l1_phase || have_l2_phase || have_l5_phase || have_l6_phase || have_l7_phase || have_l8_phase || 
      have_l1_doppler || have_l2_doppler || have_l5_doppler || have_l6_doppler || have_l7_doppler || have_l8_doppler ||
      have_l1_range || have_l2_range || have_l5_range || have_l6_range || have_l7_range || have_l8_range) {  // dual freq observations
      rinex_obs_written = 1;  // OPUS does not like comments within observation records
   }

   if(rinex_fmt >= 3.0) { // observation header
      fprintf(file, "> %04d %02d %02d %02d %02d%11.7f%3d%3d\n", g_year,g_month,g_day, g_hours,g_minutes,(double)g_seconds+g_frac, 0, i);
      if(no_obs_output) return 2;
      no_obs = 0;
   }
   else {
      if(no_obs_output) i = 0;
      fprintf(file,  " %02d %02d %02d %02d %02d%11.7f%3d%3d", g_year%100,g_month,g_day, g_hours,g_minutes,(double)g_seconds+g_frac, 0, i);
      if(no_obs_output) {
         fprintf(file, "\n");
         return 2;
      }

      no_obs = 1;
      col_count = 0;
      out_count = 0;
      for(prn=1; prn<MAX_PRN; prn++) {  // output list of sats in the observation
         gnss = good_raw(prn);
         if(gnss == 0) continue;
         no_obs = 0;

         p = rinex_prn(prn);  // convert receiver PRN number to RINEX prn value

         fprintf(file, "%c%02d", gnss, p);
         ++col_count;
         ++out_count;
         if(out_count == i) break;
         if((i > 12) && (col_count == 12)) {  // we need a continuation line
            fprintf(file, "\n                                ");
            col_count = 0;
         }
      }
      if(col_count || no_obs) fprintf(file, "\n");
   }

   return 1;
}


double rinex_cppr(int prn, double range, double carrier)
{
double cp_range;
int gnss;

  // convert carrier phase to pseudorange
  // (testing shows this actually makes things worse)
  if(use_rinex_cppr == 0) return range;
  if(have_phase == 0) return range;

  gnss = rinex_gnss(prn);
  if(gnss & GPS) ;           // sats in 1575.426 Mhz L1
  else if(gnss & SBAS) ;
  else if(gnss & GALILEO) ;
  else return range;

  cp_range = carrier * ((LIGHTSPEED*1000.0)/1575.42E6);
if(0 &&debug_file) fprintf(debug_file, "cppr(%d,%f): %f -> %f\n", prn,carrier, range,cp_range);
  return cp_range;
}


void write_rinex_obs_data(FILE *file)
{
double phase, phase1, phase2, phase5, phase6, phase7, phase8;
double range, range1, range2, range5, range6, range7, range8;
double doppler, doppler1, doppler2, doppler5, doppler6, doppler7, doppler8;
double snr, snr1, snr2, snr5, snr6, snr7, snr8;
int lli, lli1, lli2, lli5, lli6, lli7, lli8;  // loss-of-lock indicator
int first_obs;
int prn;
int i, j;
char buf[SLEN+1];
int *obs_type;
int obs_count;
int count;

   // write the RAW data observations
   if(rinex_header_written == 0) return;

   j = 0;
   for(prn=1; prn<MAX_PRN; prn++) {  // output observation data for tracked sats
      if(good_raw(prn) == 0) continue;

      // validate values
      lli = lli1 = lli2 = lli5 = lli6 = lli7 = lli8 = GOOD_LLI;
      phase = phase1 = phase2 = phase5 = phase6 = phase7 = phase8 = 0.0;
      range = range1 = range2 = range5 = range6 = range7 = range8 = 0.0;
      doppler = doppler1 = doppler2 = doppler5 = doppler6 = doppler7 = doppler8 = 0.0;
      snr = snr1 = snr2 = snr5 = snr6 = snr7 = snr8 = 0.0;

      if(have_phase) {  // make sure pseudo range is valid
         count = 0;
         phase = sat[prn].code_phase;
         if(RAW_CHANGES && (phase == sat[prn].last_code_phase)) phase = 0.0;
         sat[prn].last_code_phase = sat[prn].code_phase;
         while(phase >= 1.0E10) {
            phase -= 1.0E10;
            lli = BAD_LLI;
            if(++count > 10) {
               phase = 0.0;
               break;
            }
         }
         count = 0;
         while(phase <= (-1.0E10)) {
            phase += 1.0E10;
            lli = BAD_LLI;
            if(++count > 10) {
               phase = 0.0;
               break;
            }
         }
         if(phase >= 1.0E10) phase = 0.0;
         else if(phase <= -1.0E10) phase = 0.0;
if(sat[prn].ca_lli & (LOCK_LOST | CYCLE_SLIP)) lli = BAD_LLI;
      }


      if(have_range || (use_rinex_cppr && have_phase)) {
         range = sat[prn].range;
         if(RAW_CHANGES && (range == sat[prn].last_range)) range = 0.0;
         sat[prn].last_range = sat[prn].range;
         if(range < 0.0) range = 0.0;
         else if(range >= 1.0E9) range = 0.0;
         if(use_rinex_cppr && have_phase) {  // derive pseudorange from carrier phase data
            range = rinex_cppr(prn, range, phase);
         }
      }

      if(have_doppler) {
         doppler = sat[prn].doppler;
         if(RAW_CHANGES && (doppler == sat[prn].last_doppler)) doppler = 0.0;
         if(rcvr_type == MOTO_RCVR) doppler = 0.0;  // does not output usable doppler
         sat[prn].last_doppler = sat[prn].doppler;
         if(doppler <= -100000.0) doppler = 0.0;
         else if(doppler >= 100000.0) doppler = 0.0;
      }

      snr = sat[prn].sig_level;
      if((snr < 0.0) || (snr >= 100.0)) snr = 0;



      if(have_l1_phase) {  // make sure pseudo range is valid
         count = 0;
         phase1 = sat[prn].l1_code_phase;
         if(RAW_CHANGES && (phase1 == sat[prn].last_l1_code_phase)) phase1 = 0.0;
         sat[prn].last_l1_code_phase = sat[prn].l1_code_phase;
         while(phase1 >= 1.0E10) {
            phase1 -= 1.0E10;
            lli1 = BAD_LLI;
            if(++count > 10) {
               phase1 = 0.0;
               break;
            }
         }
         count = 0;
         while(phase1 <= (-1.0E10)) {
            phase1 += 1.0E10;
            lli1 = BAD_LLI;
            if(++count > 10) {
               phase1 = 0.0;
               break;
            }
         }
         if(phase1 >= 1.0E10) phase1 = 0.0;
         else if(phase1 <= -1.0E10) phase1 = 0.0;
if(sat[prn].l1_lli & (LOCK_LOST | CYCLE_SLIP | Z_LOSS)) lli1 = BAD_LLI;
      }

      if(have_l1_range) {
         range1 = sat[prn].l1_range;
         if(RAW_CHANGES && (range1 == sat[prn].last_l1_range)) range1 = 0.0;
         sat[prn].last_l1_range = sat[prn].l1_range;
         if(range1 < 0.0) range1 = 0.0;
         else if(range1 >= 1.0E9) range1 = 0.0;
      }

      if(have_l1_doppler) {
         doppler1 = sat[prn].l1_doppler;
         if(RAW_CHANGES && (doppler1 == sat[prn].last_l1_doppler)) doppler1 = 0.0;
         sat[prn].last_l1_doppler = sat[prn].l1_doppler;
         if(doppler1 <= -100000.0) doppler1 = 0.0;
         else if(doppler1 >= 100000.0) doppler1 = 0.0;
      }

      snr1 = sat[prn].l1_sig_level;
      if((snr1 < 0.0) || (snr1 >= 100.0)) snr2 = 0;


      if(have_l2_phase) {  // make sure pseudo range is valid
         count = 0;
         phase2 = sat[prn].l2_code_phase;
         if(RAW_CHANGES && (phase2 == sat[prn].last_l2_code_phase)) phase2 = 0.0;
         sat[prn].last_l2_code_phase = sat[prn].l2_code_phase;
         while(phase2 >= 1.0E10) {
            phase2 -= 1.0E10;
            lli2 = BAD_LLI;
            if(++count > 10) {
               phase2 = 0.0;
               break;
            }
         }
         count = 0;
         while(phase2 <= (-1.0E10)) {
            phase2 += 1.0E10;
            lli2 = BAD_LLI;
            if(++count > 10) {
               phase2 = 0.0;
               break;
            }
         }
         if(phase2 >= 1.0E10) phase2 = 0.0;
         else if(phase2 <= -1.0E10) phase2 = 0.0;
if(sat[prn].l2_lli & (LOCK_LOST | CYCLE_SLIP | Z_LOSS)) lli2 = BAD_LLI;
if(sat[prn].l2_lli & P_TRACK) {
   if(lli2 == GOOD_LLI) lli2 = P_MODE;
   else if(lli2 == BAD_LLI) lli2 = '5';  // toots
}
      }

      if(have_l2_range) {
         range2 = sat[prn].l2_range;
         if(RAW_CHANGES && (range2 == sat[prn].last_l2_range)) range2 = 0.0;
         sat[prn].last_l2_range = sat[prn].l2_range;
         if(range2 < 0.0) range2 = 0.0;
         else if(range2 >= 1.0E9) range2 = 0.0;
      }

      if(have_l2_doppler) {
         doppler2 = sat[prn].l2_doppler;
         if(RAW_CHANGES && (doppler2 == sat[prn].last_l2_doppler)) doppler2 = 0.0;
         sat[prn].last_l2_doppler = sat[prn].l2_doppler;
         if(doppler2 <= -100000.0) doppler2 = 0.0;
         else if(doppler2 >= 100000.0) doppler2 = 0.0;
      }

      snr2 = sat[prn].l2_sig_level;
      if((snr2 < 0.0) || (snr2 >= 100.0)) snr2 = 0;

      if(rcvr_type == Z12_RCVR) { // undo SNR scaling
         snr  *= Z12_SNR_SCALE;
         snr1 *= Z12_SNR_SCALE;
         snr2 *= Z12_SNR_SCALE;
      }



      if(have_l5_phase) {  // make sure pseudo range is valid
         count = 0;
         phase5 = sat[prn].l5_code_phase;
         if(RAW_CHANGES && (phase5 == sat[prn].last_l5_code_phase)) phase5 = 0.0;
         sat[prn].last_l5_code_phase = sat[prn].l5_code_phase;
         while(phase5 >= 1.0E10) {
            phase5 -= 1.0E10;
            lli5 = BAD_LLI;
            if(++count > 10) {
               phase5 = 0.0;
               break;
            }
         }
         count = 0;
         while(phase5 <= (-1.0E10)) {
            phase5 += 1.0E10;
            lli5 = BAD_LLI;
            if(++count > 10) {
               phase5 = 0.0;
               break;
            }
         }
         if(phase5 >= 1.0E10) phase5 = 0.0;
         else if(phase5 <= -1.0E10) phase5 = 0.0;
if(sat[prn].l5_lli & (LOCK_LOST | CYCLE_SLIP | Z_LOSS)) lli5 = BAD_LLI;
      }

      if(have_l5_range) {
         range5 = sat[prn].l5_range;
         if(RAW_CHANGES && (range5 == sat[prn].last_l5_range)) range5 = 0.0;
         sat[prn].last_l5_range = sat[prn].l5_range;
         if(range5 < 0.0) range5 = 0.0;
         else if(range5 >= 1.0E9) range5 = 0.0;
      }

      if(have_l5_doppler) {
         doppler5 = sat[prn].l5_doppler;
         if(RAW_CHANGES && (doppler5 == sat[prn].last_l5_doppler)) doppler5 = 0.0;
         sat[prn].last_l5_doppler = sat[prn].l5_doppler;
         if(doppler5 <= -100000.0) doppler5 = 0.0;
         else if(doppler5 >= 100000.0) doppler5 = 0.0;
      }

      snr5 = sat[prn].l5_sig_level;
      if((snr5 < 0.0) || (snr5 >= 100.0)) snr5 = 0;



      if(have_l6_phase) {  // make sure pseudo range is valid
         count = 0;
         phase6 = sat[prn].l6_code_phase;
         if(RAW_CHANGES && (phase6 == sat[prn].last_l6_code_phase)) phase6 = 0.0;
         sat[prn].last_l6_code_phase = sat[prn].l6_code_phase;
         while(phase6 >= 1.0E10) {
            phase6 -= 1.0E10;
            lli6 = BAD_LLI;
            if(++count > 10) {
               phase6 = 0.0;
               break;
            }
         }
         count = 0;
         while(phase6 <= (-1.0E10)) {
            phase6 += 1.0E10;
            lli6 = BAD_LLI;
            if(++count > 10) {
               phase6 = 0.0;
               break;
            }
         }
         if(phase6 >= 1.0E10) phase6 = 0.0;
         else if(phase6 <= -1.0E10) phase6 = 0.0;
if(sat[prn].l6_lli & (LOCK_LOST | CYCLE_SLIP | Z_LOSS)) lli6 = BAD_LLI;
      }

      if(have_l6_range) {
         range6 = sat[prn].l6_range;
         if(RAW_CHANGES && (range6 == sat[prn].last_l6_range)) range6 = 0.0;
         sat[prn].last_l6_range = sat[prn].l6_range;
         if(range6 < 0.0) range6 = 0.0;
         else if(range6 >= 1.0E9) range6 = 0.0;
      }

      if(have_l6_doppler) {
         doppler6 = sat[prn].l6_doppler;
         if(RAW_CHANGES && (doppler6 == sat[prn].last_l6_doppler)) doppler6 = 0.0;
         sat[prn].last_l6_doppler = sat[prn].l6_doppler;
         if(doppler6 <= -100000.0) doppler6 = 0.0;
         else if(doppler6 >= 100000.0) doppler6 = 0.0;
      }

      snr6 = sat[prn].l6_sig_level;
      if((snr6 < 0.0) || (snr6 >= 100.0)) snr6 = 0;



      if(have_l7_phase) {  // make sure pseudo range is valid
         count = 0;
         phase7 = sat[prn].l7_code_phase;
         if(RAW_CHANGES && (phase7 == sat[prn].last_l7_code_phase)) phase7 = 0.0;
         sat[prn].last_l7_code_phase = sat[prn].l7_code_phase;
         while(phase7 >= 1.0E10) {
            phase7 -= 1.0E10;
            lli7 = BAD_LLI;
            if(++count > 10) {
               phase7 = 0.0;
               break;
            }
         }
         count = 0;
         while(phase7 <= (-1.0E10)) {
            phase7 += 1.0E10;
            lli7 = BAD_LLI;
            if(++count > 10) {
               phase7 = 0.0;
               break;
            }
         }
         if(phase7 >= 1.0E10) phase7 = 0.0;
         else if(phase7 <= -1.0E10) phase7 = 0.0;
if(sat[prn].l7_lli & (LOCK_LOST | CYCLE_SLIP | Z_LOSS)) lli7 = BAD_LLI;
      }

      if(have_l7_range) {
         range7 = sat[prn].l7_range;
         if(RAW_CHANGES && (range7 == sat[prn].last_l7_range)) range7 = 0.0;
         sat[prn].last_l7_range = sat[prn].l7_range;
         if(range7 < 0.0) range7 = 0.0;
         else if(range7 >= 1.0E9) range7 = 0.0;
      }

      if(have_l7_doppler) {
         doppler7 = sat[prn].l7_doppler;
         if(RAW_CHANGES && (doppler7 == sat[prn].last_l7_doppler)) doppler7 = 0.0;
         sat[prn].last_l7_doppler = sat[prn].l7_doppler;
         if(doppler7 <= -100000.0) doppler7 = 0.0;
         else if(doppler7 >= 100000.0) doppler7 = 0.0;
      }

      snr7 = sat[prn].l7_sig_level;
      if((snr7 < 0.0) || (snr7 >= 100.0)) snr7 = 0;



      if(have_l8_phase) {  // make sure pseudo range is valid
         count = 0;
         phase8 = sat[prn].l8_code_phase;
         if(RAW_CHANGES && (phase8 == sat[prn].last_l8_code_phase)) phase8 = 0.0;
         sat[prn].last_l8_code_phase = sat[prn].l8_code_phase;
         while(phase8 >= 1.0E10) {
            phase8 -= 1.0E10;
            lli8 = BAD_LLI;
            if(++count > 10) {
               phase8 = 0.0;
               break;
            }
         }
         count = 0;
         while(phase8 <= (-1.0E10)) {
            phase8 += 1.0E10;
            lli8 = BAD_LLI;
            if(++count > 10) {
               phase8 = 0.0;
               break;
            }
         }
         if(phase8 >= 1.0E10) phase8 = 0.0;
         else if(phase8 <= -1.0E10) phase8 = 0.0;
if(sat[prn].l8_lli & (LOCK_LOST | CYCLE_SLIP | Z_LOSS)) lli8 = BAD_LLI;
      }

      if(have_l8_range) {
         range8 = sat[prn].l8_range;
         if(RAW_CHANGES && (range8 == sat[prn].last_l8_range)) range8 = 0.0;
         sat[prn].last_l8_range = sat[prn].l8_range;
         if(range8 < 0.0) range8 = 0.0;
         else if(range8 >= 1.0E9) range8 = 0.0;
      }

      if(have_l8_doppler) {
         doppler8 = sat[prn].l8_doppler;
         if(RAW_CHANGES && (doppler8 == sat[prn].last_l8_doppler)) doppler8 = 0.0;
         sat[prn].last_l8_doppler = sat[prn].l8_doppler;
         if(doppler8 <= -100000.0) doppler8 = 0.0;
         else if(doppler8 >= 100000.0) doppler8 = 0.0;
      }

      snr8 = sat[prn].l8_sig_level;
      if((snr8 < 0.0) || (snr8 >= 100.0)) snr8 = 0;


      // output the observation record  

      out[0] = 0;
      if(rinex_fmt >= 3.0) {
         if(gps_sat(prn)) {
            obs_type = &gps_obs_type[0];
            count = gps_num_obs;
         }
         else if(sbas_sat(prn)) {
            obs_type = &sbas_obs_type[0];
            count = sbas_num_obs;
         }
         else if(glonass_sat(prn)) {
            obs_type = &glonass_obs_type[0];
            count = glonass_num_obs;
         }
         else if(galileo_sat(prn)) {
            obs_type = &galileo_obs_type[0];
            count = galileo_num_obs;
         }
         else if(beidou_sat(prn)) {
            obs_type = &beidou_obs_type[0];
            count = beidou_num_obs;
         }
      }
      else {
         obs_type = &mixed_obs_type[0];
         count = mixed_num_obs;
      }

      obs_count = 0;
      first_obs = 1;
      for(i=0; i<count; i++) {  // ouput the observation values
         // signal level
         if((rinex_fmt >= 3.0) && (obs_count == 0)) { // RINEX v3 doesn't have continuation lines
            if(first_obs) fprintf(file, "%c%02d",  good_raw(prn), rinex_prn(prn));
            else          fprintf(file, "   ");
            first_obs = 0;
         }

         // output the obervation values
         if(obs_type[i] == S1_OBS) {
            strcpy(buf, "                ");
            if(snr1 > 0.0)     sprintf(buf, "%14.3f  ", snr1);
            else if(snr > 0.0) sprintf(buf, "%14.3f  ", snr);
            buf[16] = 0;
            add_obs_val(file, buf, &obs_count);
         }
         if(obs_type[i] == S2_OBS) {
            strcpy(buf, "                ");
            if(snr2 > 0.0)     sprintf(buf, "%14.3f  ", snr2);
            buf[16] = 0;
            add_obs_val(file, buf, &obs_count);
         }
         if(obs_type[i] == S5_OBS) {
            strcpy(buf, "                ");
            if(snr5 > 0.0)     sprintf(buf, "%14.3f  ", snr5);
            buf[16] = 0;
            add_obs_val(file, buf, &obs_count);
         }
         if(obs_type[i] == S6_OBS) {
            strcpy(buf, "                ");
            if(snr6 > 0.0)     sprintf(buf, "%14.3f  ", snr6);
            buf[16] = 0;
            add_obs_val(file, buf, &obs_count);
         }
         if(obs_type[i] == S7_OBS) {
            strcpy(buf, "                ");
            if(snr7 > 0.0)     sprintf(buf, "%14.3f  ", snr7);
            buf[16] = 0;
            add_obs_val(file, buf, &obs_count);
         }
         if(obs_type[i] == S8_OBS) {
            strcpy(buf, "                ");
            if(snr8 > 0.0)     sprintf(buf, "%14.3f  ", snr8);
            buf[16] = 0;
            add_obs_val(file, buf, &obs_count);
         }

         // doppler
         if(obs_type[i] == D1_OBS) {
            strcpy(buf, "                ");
            if(doppler1)      sprintf(buf, "%14.3f  ", doppler1);
            else if(doppler)  sprintf(buf, "%14.3f  ", doppler);
            buf[16] = 0;
            add_obs_val(file, buf, &obs_count);
         }
         if(obs_type[i] == D2_OBS) {
            strcpy(buf, "                ");
            if(doppler2) sprintf(buf, "%14.3f  ", doppler2);
            buf[16] = 0;
            add_obs_val(file, buf, &obs_count);
         }
         if(obs_type[i] == D5_OBS) {
            strcpy(buf, "                ");
            if(doppler5) sprintf(buf, "%14.3f  ", doppler5);
            buf[16] = 0;
            add_obs_val(file, buf, &obs_count);
         }
         if(obs_type[i] == D6_OBS) {
            strcpy(buf, "                ");
            if(doppler6) sprintf(buf, "%14.3f  ", doppler6);
            buf[16] = 0;
            add_obs_val(file, buf, &obs_count);
         }
         if(obs_type[i] == D7_OBS) {
            strcpy(buf, "                ");
            if(doppler7) sprintf(buf, "%14.3f  ", doppler7);
            buf[16] = 0;
            add_obs_val(file, buf, &obs_count);
         }
         if(obs_type[i] == D8_OBS) {
            strcpy(buf, "                ");
            if(doppler8) sprintf(buf, "%14.3f  ", doppler8);
            buf[16] = 0;
            add_obs_val(file, buf, &obs_count);
         }


         // pseudorange
         if(obs_type[i] == C1_OBS) {
            strcpy(buf, "                ");
            if(range) sprintf(buf, "%14.3f  ", range);
            buf[16] = 0;
            add_obs_val(file, buf, &obs_count);
         }
         if(obs_type[i] == P1_OBS) {
            strcpy(buf, "                ");
            if(range1 && USE_L2) sprintf(buf, "%14.3f  ", range1);
            buf[16] = 0;
            add_obs_val(file, buf, &obs_count);
         }
         if(obs_type[i] == P2_OBS) {
            strcpy(buf, "                ");
            if(range2 && USE_L2) sprintf(buf, "%14.3f  ", range2);
            buf[16] = 0;
            add_obs_val(file, buf, &obs_count);
         }
         if(obs_type[i] == P5_OBS) {
            strcpy(buf, "                ");
            if(range5 && USE_L5) sprintf(buf, "%14.3f  ", range5);
            buf[16] = 0;
            add_obs_val(file, buf, &obs_count);
         }
         if(obs_type[i] == P6_OBS) {
            strcpy(buf, "                ");
            if(range6 && USE_L6) sprintf(buf, "%14.3f  ", range6);
            buf[16] = 0;
            add_obs_val(file, buf, &obs_count);
         }
         if(obs_type[i] == P7_OBS) {
            strcpy(buf, "                ");
            if(range7 && USE_L7) sprintf(buf, "%14.3f  ", range7);
            buf[16] = 0;
            add_obs_val(file, buf, &obs_count);
         }
         if(obs_type[i] == P8_OBS) {
            strcpy(buf, "                ");
            if(range8 && USE_L8) sprintf(buf, "%14.3f  ", range8);
            buf[16] = 0;
            add_obs_val(file, buf, &obs_count);
         }

         // carrier phase
         if(obs_type[i] == L1_OBS) {
            strcpy(buf, "                ");
            if(phase1 && USE_L2) sprintf(buf, "%14.3f%c ", phase1,lli1);
            else if(phase)       sprintf(buf, "%14.3f%c ", phase,lli);
            buf[16] = 0;
            add_obs_val(file, buf, &obs_count);
         }
         if(obs_type[i] == L2_OBS) {
            strcpy(buf, "                ");
            if(phase2 && USE_L2) sprintf(buf, "%14.3f%c ", phase2,lli2);
            buf[16] = 0;
            add_obs_val(file, buf, &obs_count);
         }
         if(obs_type[i] == L5_OBS) {
            strcpy(buf, "                ");
            if(phase5 && USE_L5) sprintf(buf, "%14.3f%c ", phase5,lli5);
            buf[16] = 0;
            add_obs_val(file, buf, &obs_count);
         }
         if(obs_type[i] == L6_OBS) {
            strcpy(buf, "                ");
            if(phase6 && USE_L6) sprintf(buf, "%14.3f%c ", phase6,lli6);
            buf[16] = 0;
            add_obs_val(file, buf, &obs_count);
         }
         if(obs_type[i] == L7_OBS) {
            strcpy(buf, "                ");
            if(phase7 && USE_L7) sprintf(buf, "%14.3f%c ", phase7,lli7);
            buf[16] = 0;
            add_obs_val(file, buf, &obs_count);
         }
         if(obs_type[i] == L8_OBS) {
            strcpy(buf, "                ");
            if(phase8 && USE_L8) sprintf(buf, "%14.3f%c ", phase8,lli8);
            buf[16] = 0;
            add_obs_val(file, buf, &obs_count);
         }
      }

      if(obs_count) fprintf(file, "\n");
   }

   if(log_flush_mode && file && (file == log_file)) fflush(file);
   if(rinex_flush_mode && file && (file == rinex_file)) fflush(file);
}

void write_rinex_obs(FILE *file)
{
double delta;
double skipped_jd;
double old_obs;
int count;

   if(jd_obs == last_jd_obs) return;  // raw data has not updated
   if(rinex_header_written == 0) return;
   skipped_jd = last_jd_obs;

   delta = jd_obs - last_jd_obs;
   delta = delta - (double) (int) delta;
   delta *= 24.0*60.0*60.0;
   if(1 && have_raw_rate && rinex_fix && obs_data_written && last_jd_obs && (delta > (((double) raw_msg_rate) * 1.10))) {  // rnx3
      // time stamp skipped, output null observation headers for missing data
      old_obs = jd_obs;
      count = 0;
      while(skipped_jd < (old_obs-jtime(0,0,raw_msg_rate,JD_OBS_ROUND))) {  // output dummy obs headers
         skipped_jd += jtime(0,0,0, (double) raw_msg_rate);
         jd_obs = skipped_jd;
write_rinex_line(file, "#! missing observation", "COMMENT");  //rnx3
         write_rinex_obs_header(file, 1);
         if(++count > (raw_msg_rate*100)) break;  // prevent runaway loop
      }
      jd_obs = old_obs;
   }

   if(write_rinex_obs_header(file, 0)) {
      write_rinex_obs_data(file);
   }

   last_jd_obs = jd_obs;
}



//
// 
//  Date and time related stuff
//
//
int dst_hour = 2;     // the hour to switch the time at
int dst_start_day;    // day and month of daylight savings time start
int dst_start_month;
int dst_end_day;      // day and month of daylight savings time end
int dst_end_month;
int down_under;       // start and stop times are reversed in the southern hemisphere

// string decribes when daylight savings time occurs
char *dst_list[] = {  // start: day_count,day_of_week number,start_month,
                      // end:   day_count,day_of_week number,end_month,
                      //        switchover_hour
                      //
                      //   day_count = nth occurence of day 
                      //               (if > 0, from start of month)
                      //               (if < 0, from end of month)
                      //   day_of_week 0=SUN, 1=MON, 2=TUE, ... 6=SAT
                      //   month 1=JAN ... 12=DEC
                      //   hour that the time switches over

   "",                  // dst zone 0 = no dst
   " 2,0,3,1,0,11,2",   // dst zone 1 = USA
   "-1,0,3,-1,0,10,2",  // dst zone 2 = Europe
   " 1,0,10,1,0,4,2",   // dst zone 3 = Australia
   "-1,0,9,1,0,4,2",    // dst zone 4 = New Zealand
   custom_dst           // dst zone 5 = custom zone definition goes here
};

int leap_year(int year)
{
   if(year % 4) return 0;        // not a leap year
   if((year % 100) == 0) {       // most likely,  not a leap year
      if(year % 400) return 0;   // not a leap year
   }
   return 1;
}

void init_dsm(int year)
{
int month;

   // days to start of month (and days in month)
   dsm[0]  = 0;
   dsm[1]  = 0;         // Jan
   dsm[2]  = 0+31;      // Feb
   dsm[3]  = 0+31+28;   // March ...
   dsm[4]  = 0+31+28+31;
   dsm[5]  = 0+31+28+31+30;
   dsm[6]  = 0+31+28+31+30+31;
   dsm[7]  = 0+31+28+31+30+31+30;
   dsm[8]  = 0+31+28+31+30+31+30+31;
   dsm[9]  = 0+31+28+31+30+31+30+31+31;
   dsm[10] = 0+31+28+31+30+31+30+31+31+30;
   dsm[11] = 0+31+28+31+30+31+30+31+31+30+31;
   dsm[12] = 0+31+28+31+30+31+30+31+31+30+31+30;

   dim[2] = 28;

   if(leap_year(year)) {
      for(month=2; month<=12; month++) dsm[month] += 1;
      dim[2] += 1;
   }
}


u08 day_of_week(int d, int m, int y)      /* 0=Sunday  1=Monday ... */
{
static u08 dow_info[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};

   if(m < 3) y -= 1;
   return (y + y/4 - y/100 + y/400 + dow_info[m-1] + d) % 7;
}

int nth_dow_in(int nth, int dow, int month, int year)
{
int sundays;
int d;

   sundays = 0;
   month = fix_month(month);
   if(nth < 0) {  // nth day of week from end of month
      nth = 0 - nth;
      for(d=dim[month]; d>=1; d--) {
         if(day_of_week(d, month, year) == dow) {
            ++sundays;
            if(sundays == nth) return d;  // last sunday in month, etc
         }
      }
   }
   else { // nth day of week from start of month
      for(d=1; d<=dim[month]; d++) {
         if(day_of_week(d, month, year) == dow) {
            ++sundays;
            if(sundays == nth) return d;  // 2nd sunday in month, etc
         }
      }
   }
   return 0;
}


void bump_time(void)
{
  // add one second to current time value
  month = fix_month(month);
  if(++seconds >= 60) {
     seconds = 0;
     if(++minutes >= 60) {
        minutes = 0;
        if(++hours >= 24) {
           hours = 0;
           if(++day > dim[month]) {
              day = 1;
              if(++month > 12) {
                 month = 1;
                 ++year;
              }
           }
        }
     }
  }
}


void gregorian(int no_tweak, double jd)
{
long z;
long w;
long x;
long a,b,c,d,e,f;
double t;
double tweak;
int i;


   // convert Julian date to Gregorian
   // The results are returned in the g_xxxx global variables.

   tweak = 0.0;  // used to compensate for subtle round-off errors;
   for(i=0; i<2; i++) {
      if(tweak) jd += jtime(0,0,0, tweak);  // tweak time to prevent round-down errors

      z = (long) (jd + 0.50);
      w = (long) (((double) z - 1867216.25) / 36524.25);
      x = w / 4;
      a = z + 1 + w - x;
      b = a + 1524;
      c = (long) ((((double) b) - 122.1) / 365.25);
      d = (long) (((double) c) * 365.25);
      e = (long) (((double) (b-d)) / 30.6001);
      f = (long) (((double) e) * 30.6001);

      g_day   = (int) (b - d - f);
      g_month = (int) (e - 1);
      if(g_month > 12) g_month = (int) (e - 13);

      if(g_month < 3) g_year = (int) (c - 4715);
      else            g_year = (int) (c - 4716);

      t = jd - jdate(g_year,g_month,g_day); 

      t *= 24.0;          // convert fractional day to hours
      g_hours = (int) t;  // convert decimal hours to hh:mm:ss

      t = (t - g_hours);
      g_minutes = (int) (t * 60.0);

      t = (t * 60.0) - g_minutes;
      g_seconds = (int) (t * 60.0);

      g_frac = (t * 60.0) - g_seconds;

      if(no_tweak) return;
      else if(g_frac >= 0.999) tweak += 0.001;
      else break;
   }

   if(g_month < 1) g_month = 0;        // invalid month, protect against possible array indexing errors
   else if(g_month > 12) g_month = 0;
   if(g_day < 1) g_day = 0;            // invalid day, protect against possible array indexing errors
   else if(g_day > 31) g_day = 0;
}


double jdate(int y, int m, int d)
{
long j;
long j1;
long j2;
long j3;
long c;
long x;

   // return the julian date of the start of a given day

   if(m < 3) {
      m = m + 12;
      j = y - 1;
   }
   else j = y;

   c = (j / 100L);
   x = j - (100L * c);
   j1 = (146097L * c) / 4L;
   j2 = (36525L * x) / 100L;
   j3 = ((153L*(long)m) - 457L) / 5L;
   return (1721119.5-1.0) + (double) j1 + (double) j2 + (double) j3 + (double) d;
}

double jtime(int hh, int mm, int ss, double frac)
{
double h;

   // return time in Julian days

   h = ((double) hh) + (((double) mm)/60.0) + ((((double) ss)+frac)/(60.0*60.0));
   return (h / 24.0);
}


double sun_mean_anomaly(double jd)
{
double T, T2, T3;

   // return mean anomaly of the sun in radians at time JD
   T = (jd - JD2000) / 36525.0;
   T2 = T * T;
   T3 = T * T2;

   return (357.52772 + 35999.05034*T - 0.0001603*T2 - (T3 / 300000.0)) * (PI/180.0);
}


double nutation(double JD)
{
double T, T2, T3, DegToRad;
double w, w1, w2, w3, w4, w5;
   
   // Code derived from code by Jay Tanner

   // ---------------------------------
   // Compute (T) in Julian centuries
   // from J2000.0 and powers of (T).

   T = (JD - JD2000) / 36525.0;
   T2 = T * T;
   T3 = T * T2;

   // ------------------------------------------------
   // Define conversion factor for degrees to radians.
   // Multiply degrees by this factor to get radians.

   DegToRad = PI / 180.0;

   // -----------------------------------------------
   // Compute mean elongation of the moon in radians.

   w1 = DegToRad*(297.85036 + 445267.11148*T
      - 0.0019142*T2 + (T3 / 189474.0));

   // -------------------------------------------
   // Compute mean anomaly of the sun in radians.

   w2 = DegToRad*(357.52772 + 35999.05034*T
      - 0.0001603*T2 - (T3 / 300000.0));

   // --------------------------------------------
   // Compute mean anomaly of the moon in radians.

   w3 = DegToRad*(134.96298 + 477198.867398*T
      + 0.0086972*T2 + (T3 / 56250.0));

   // -----------------------------------------------
   // Compute moon's argument of latitude in radians.

   w4 = DegToRad*(93.27191 + 483202.017538*T
      - 0.0036825*T2 + (T3 / 327270.0));

   // ------------------------------------------------------
   // Compute longitude of moon's ascending node in radians.

   w5 = DegToRad*(125.04452 - 1934.136261*T
      + 0.0020708*T2 + (T3 / 450000.0));

   // ---------------------------------------------
   // Compute the nutation in obliquity in terms of
   // arc seconds * 10000

   w  = cos(w5)*(92025.0 + 8.9*T);

   w += cos(2.0*(w4 - w1 + w5))*(5736.0 - 3.1*T);
   w += cos(2.0*(w4 + w5))*(977.0 - 0.5*T);
   w += cos(2.0*w5)*(0.5*T - 895.0);
   w += cos(w2)*(54.0 - 0.1*T);
   w -= 7.0*cos(w3);
   w += cos(w2 + 2.0*(w4 - w1 + w5))*(224.0 - 0.6*T);
   w += 200.0*cos(2.0*w4 + w5);
   w += cos(w3 + 2.0*(w4 + w5))*(129.0 - 0.1*T);
   w += cos(2.0*(w4 - w1 + w5) - w2)*(0.3*T - 95.0);
   w -= 70.0*cos(2.0*(w4 - w1) + w5);
   w -= 53.0*cos(2.0*(w4 + w5) - w3);
   w -= 33.0*cos(w3 + w5);
   w += 26.0*cos(2.0*(w1 + w4 + w5) - w3);
   w += 32.0*cos(w5 - w3);
   w += 27.0*cos(w3 + 2.0*w4 + w5);
   w -= 24.0*cos(2.0*(w4 - w3) + w5);
   w += 16.0*cos(2.0*(w1 + w4 + w5));
   w += 13.0*cos(2.0*(w3 + w4 + w5));
   w -= 12.0*cos(w3 + 2.0*(w4 - w1 + w5));
   w -= 10.0*cos(2.0*w4 + w5 - w3);
   w -= 8.0*cos(2.0*w1 - w3 + w5);
   w += 7.0*cos(2.0*(w2 - w1 + w4 + w5));
   w += 9.0*cos(w2 + w5);
   w += 7.0*cos(w3 + w5 - 2.0*w1);
   w += 6.0*cos(w5 - w2);
   w += 5.0*cos(2.0*(w1 + w4) - w3 + w5);
   w += 3.0*cos(w3 + 2.0*(w4 + w1 + w5));
   w -= 3.0*cos(w2 + 2.0*(w4 + w5));
   w += 3.0*cos(2.0*(w4 + w5) - w2);
   w += 3.0*cos(2.0*(w1 + w4) + w5);
   w -= 3.0*cos(2.0*(w3 + w4 + w5 - w1));
   w -= 3.0*cos(w3 + 2.0*(w4 - w1) + w5);
   w += 3.0*cos(2.0*(w1 - w3) + w5);
   w += 3.0*cos(2.0*w1 + w5);
   w += 3.0*cos(2.0*(w4 - w1) + w5 - w2);
   w += 3.0*cos(w5 - 2.0*w1);
   w += 3.0*cos(2.0*(w3 + w4) + w5);

   // -----------------------------------
   // Done.  Return nutation in obliquity
   // expressed in decimal degrees.

   return w / 36000000.0;

}

double obliquity(double JD, int nutate)
{
double t, p, w, EpsMeanDeg;

   // Code derived from code by Jay Tanner
   // Algorithm by JPL?

   // -----------------------------------------------------------
   // Compute the (t) value in Julian decamillennia corresponding
   // to the JD argument and reckoned from J2000.
   t = (JD - JD2000) / 3652500.0;

   // --------------------------------------
   // Compute mean obliquity in arc seconds.
   w  = 84381.448;  p  = t;
   w -=  4680.93*p; p *= t;
   w -=     1.55*p; p *= t;
   w +=  1999.25*p; p *= t;
   w -=    51.38*p; p *= t;
   w -=   249.67*p; p *= t;
   w -=    39.05*p; p *= t;
   w +=     7.12*p; p *= t;
   w +=    27.87*p; p *= t;
   w +=     5.79*p; p *= t;
   w +=     2.45*p;

   // ----------------------------------
   // Compute mean ecliptic obliquity in
   // degrees from arc seconds value.
   EpsMeanDeg = w / 3600.0;
   if(nutate) EpsMeanDeg += nutation(JD);   // adjust for nutation
   EpsMeanDeg *= (PI/180.0);     // convert to radians

   return EpsMeanDeg;
}

int dst_disabled();

void calc_dst_times(int yy, char *s)
{
int start_n, end_n;
int start_dow, end_dow;
int no_dst;

   // calculate when daylight savings time starts and ends

   dst_start_month = dst_end_month = 0;
   dst_start_day = dst_end_day = 0;
   down_under = 0;
   need_sunrise = 1;

   no_dst = 0;
   if(dst_area == 0) no_dst |= 0x01;       // no dst area set
   if(s == 0) no_dst |= 0x02;              // no dst area definition string
   if(s[0] == 0) no_dst |= 0x04;           // empty dst area definition string
   if(dst_string[0] == 0) no_dst |= 0x08;  // no DST time zone name given

   if(no_dst) {
      dst_ofs = dst_offset();
      jd_local = jd_utc + time_zone() + jtime(0,0,0,TSX_TWEAK);

      #ifdef GREET_STUFF
         setup_calendars(1);   // recalc calendars for the new time zone
      #endif
      return;
   }

   start_n = end_n = 0;
   start_dow = end_dow = 0;
   dst_hour = 2;
   sscanf(s, "%d,%d,%d,%d,%d,%d,%d", &start_n,&start_dow,&dst_start_month,
                                     &end_n,&end_dow,&dst_end_month, &dst_hour);
   if(dst_start_month > dst_end_month) { // start month>end_month means southern hemisphere
      down_under = 1;                    // re-scan the string to swap the values
      dst_start_month = dst_end_month = 0;
      start_n = end_n = 0;
      start_dow = end_dow = 0;
      dst_hour = 2;
      sscanf(s, "%d,%d,%d,%d,%d,%d,%d", &end_n,&end_dow,&dst_end_month,
                                        &start_n,&start_dow,&dst_start_month, &dst_hour);
   }

// yy = pri_year;
// yy = this_year;
   dst_start_day = nth_dow_in(start_n, start_dow, dst_start_month, yy);
   dst_end_day   = nth_dow_in(end_n,   end_dow,   dst_end_month,   yy);

   dst_start_jd = jdate(yy,dst_start_month,dst_start_day) + jtime(dst_hour,0,0,0.0) - tz_adjust;
// dst_end_jd = jdate(yy,dst_end_month,dst_end_day) + jtime(dst_hour,0,0,0.0) - tz_adjust;
   dst_end_jd = jdate(yy,dst_end_month,dst_end_day) + jtime(dst_hour+1,0,0,0.0) - tz_adjust;
   dst_ofs = dst_offset();
   jd_local = jd_utc + time_zone() + jtime(0,0,0,TSX_TWEAK);
///gregorian(0, dst_start_jd);
///sprintf(debug_text2, "dst start: %04d/%02d/%02d  %02d:%02d:%02d", g_year,g_month,g_day, g_hours,g_minutes,g_seconds);
///gregorian(0, dst_end_jd);
///sprintf(debug_text3, "dst end:   %04d/%02d/%02d  %02d:%02d:%02d", g_year,g_month,g_day, g_hours,g_minutes,g_seconds);
///sprintf(debug_text, "dst ofs:%d", dst_ofs);

   #ifdef GREET_STUFF
      setup_calendars(2);   // recalc calendars for the new time zone
   #endif

///sprintf(debug_text6, "DST(%s): %02d/%02d .. (%s) %02d/%02d  invert=%d -> %d.  area=%d: %s", 
///  dst_string, dst_start_month, dst_start_day, std_string, dst_end_month, dst_end_day, down_under, dst_offset(), dst_area, dst_list[dst_area]);
}

int dst_disabled()
{
   if(time_zone_set == 0)   return 1;
   if(dst_start_day == 0)   return 2;
   if(dst_start_month == 0) return 3;
   if(dst_end_day == 0)     return 4;
   if(dst_end_month == 0)   return 5;
   if(dst_string[0] == 0)   return 6;
   if(dst_area == 0)        return 7;

   return 0;
}


int dst_offset()
{
int before_switch_hour;
int after_switch_hour;

   // calculate time zone adjustment for daylight savings time settings
   strcpy(tz_string, std_string);  // assume dst not in effect

   if(dst_disabled()) {   // dst switchover times not set, use standard time
      return 0;
   }

   if(down_under) {  // southern hemisphere spins backwards
      if(dst_string[0]) strcpy(tz_string, dst_string);
      before_switch_hour = dst_hour-1;
      after_switch_hour = dst_hour;
   }
   else {
      before_switch_hour = dst_hour;
      after_switch_hour = dst_hour-1;
   }

   // see if it is before dst start time
   gregorian(0, jd_local);
//sprintf(debug_text,  "local:%04d/%02d/%02d %02d:", g_year,g_month,g_day, g_hours);
//sprintf(debug_text2, "start:%04d/%02d/%02d %02d:", g_year,dst_start_month,dst_start_day,before_switch_hour);
//sprintf(debug_text3, "end:  %04d/%02d/%02d %02d:", g_year,dst_end_month,dst_end_day,after_switch_hour);


   if(g_month < dst_start_month) {
      return down_under;
   }
   else if(g_month == dst_start_month) {
      if(g_day < dst_start_day)  {
         return down_under; 
      }
      else if(g_day == dst_start_day) {
         if(g_hours < before_switch_hour) {
            return down_under;
         }
      }
   }
   
   // it is after dst start time,  is it before dst end time?
   if(g_month > dst_end_month) {
      return down_under; 
   }
   else if(g_month == dst_end_month) {
      if(g_day > dst_end_day) {
         return down_under; 
      }
      else if(g_day == dst_end_day) {
         if(g_hours > after_switch_hour) {
//sprintf(debug_text4, "x6  gh:%d  ash:%d", g_hours,after_switch_hour);
            return down_under; 
         }
      }
   }
//sprintf(debug_text5, "down under:%d", down_under);

   if(down_under) { // daylight savings time is not in effect
      if(std_string[0]) strcpy(tz_string, std_string);
//sprintf(debug_text6, "dst off");
      return 0;
   }
   else {           // daylight savings time is in effect
      if(dst_string[0]) strcpy(tz_string, dst_string);
//sprintf(debug_text6, "dst on");
      return 1;
   }
}



double perihelion_jd[] =
{
   // Julian date (fractional, UTC) of perihelion for years 2016..2100
   // derived from table by Fred Espenak, www.Astropixels.com
   2457390.4506944446,
   2457758.0958333332,
   2458121.7326388890,
   2458486.7222222220,
   2458853.8250000002,
   2459217.0770833334,
   2459583.7881944445,
   2459949.1784722223,
   2460312.5270833331,
   2460680.0611111112,
   2461044.2194444444,
   2461408.6062500002,
   2461776.0194444442,
   2462139.2590277777,
   2462504.9249999998,
   2462871.3666666667,
   2463234.7159722224,
   2463601.9937499999,
   2463966.6993055558,
   2464330.5375000001,
   2464698.0951388888,
   2465061.6666666665,
   2465426.7090277779,
   2465793.7784722224,
   2466156.9812500002,
   2466523.4111111113,
   2466888.8798611113,
   2467252.4270833335,
   2467620.0361111113,
   2467984.1222222224,
   2468348.5402777777,
   2468715.9888888891,
   2469079.2534722220,
   2469444.9354166668,
   2469811.3159722220,
   2470174.7305555558,
   2470541.8875000002,
   2470906.4291666667,
   2471270.2493055556,
   2471638.0166666666,
   2472001.6555555556,
   2472366.6326388889,
   2472733.6666666665,
   2473096.9472222221,
   2473463.4555555554,
   2473828.8145833332,
   2474192.3861111109,
   2474560.0701388889,
   2474923.9958333331,
   2475288.1319444445,
   2475655.7395833335,
   2476019.2118055555,
   2476385.1034722221,
   2476751.2673611110,
   2477114.6187499999,
   2477481.9055555556,
   2477846.3763888888,
   2478210.1118055554,
   2478577.9756944445,
   2478941.5930555556,
   2479306.4812500002,
   2479673.3701388887,
   2480036.6826388887,
   2480403.5104166665,
   2480768.8298611110,
   2481132.2888888889,
   2481500.0770833334,
   2481863.9777777777,
   2482228.2006944446,
   2482595.6895833332,
   2482959.1402777778,
   2483325.1930555557,
   2483691.1243055556,
   2484054.2090277779,
   2484421.6979166665,
   2484786.3069444443,
   2485150.1451388891,
   2485517.9277777779,
   2485881.5118055558,
   2486246.6638888889,
   2486613.4138888889,
   2486976.5361111113,
   2487343.6069444446,
   2487708.7625000002,
   2488072.0791666666
};

double time_since_perihelion(double jd)
{
#define JD_DELTA_EOT 3.5884  // 3.52719  // typical offset time of perihelion from 1 Jan in days

   // return days (fractional) since perihelion (based upon local time)
   // returns estimated value if year not between 2016 and 2100
   gregorian(0, jd);
   if(g_year < 2016) return jd - jdate(g_year, 1, 1) - JD_DELTA_EOT;
   else if(g_year > 2100) return jd - jdate(g_year, 1, 1) - JD_DELTA_EOT;

   jd = jd - perihelion_jd[g_year-2016];
// jd -= 1.363;  // fudge factor to get alignment with NREL SPA code
jd -= 1.60;
   return jd;
}

double orbit_eccentricity(double jd)
{
double t;
double ecc;

   t = (jd - JD2000)/365250.0;
   ecc = 0.0167086342 - 0.0004203654*t - 0.0000126734*t*t + 0.0000001444*t*t*t
         -0.0000000002*t*t*t*t + 0.0000000003*t*t*t*t*t;
   return ecc;
}

double solar_year(double jd)
{
double t;

   // more precise calculation of length of solar year
// return SOLAR_YEAR;
   t = (JD2000);
   t = jd - t;
   t /= 36525.0;
   t = 365.2421896698 - (6.15359E-6*t) - (7.29E-10*t*t) - (2.64E-10*t*t*t);
   if(t == 0.0) return SOLAR_YEAR;
   return t;
}


double calcObliquityCorrection(double t)
{
  double seconds = 21.448 - t*(46.8150 + t*(0.00059 - t*(0.001813)));
  double e0 = 23.0 + (26.0 + (seconds/60.0))/60.0;

  double omega = 125.04 - 1934.136 * t;
  return e0 + 0.00256 * cos(omega*PI/180.0);
}

double eot(double jd)
{
double t;
double lon_ofs;
double m;
double e;

   // calculate the equation of time
   // adapted from code by Mike Chirico and NOAA

   t = jd;
///t += jtime(3,12,30, 0.0);       // !!!!! fudge factor to get closer to SPA results
   t += jtime(2,36,00, 0.0);       // !!!!! fudge factor to get closer to SPA results
   t = (t - JD2000) / 36525.0;

   double epsilon = calcObliquityCorrection(t);               
   double l0 = 280.46646 + t * (36000.76983 + 0.0003032 * t);
   while(l0 > 360.0) l0 -= 360.0;
   while(l0 < 0.0) l0 += 360.0;
   e = orbit_eccentricity(jd);
   m = (357.52772 + (35999.05034*t) - (0.0001603*t*t) - (t*t*t / 300000.0));

   double y = tan((epsilon*PI/180.0)/2.0);
   y *= y;

   double sinm   = sin(m * PI/180.0);
   double sin2m  = sin(2.0 * m*PI/180.0);
   double sin2l0 = sin(2.0 * l0*PI/180.0);
   double cos2l0 = cos(2.0 * l0*PI/180.0);
   double sin4l0 = sin(4.0 * l0*PI/180.0);
   eot_ofs = y * sin2l0 - 2.0 * e * sinm + 4.0 * e * y * sinm * cos2l0
           - 0.5 * y * y * sin4l0 - 1.25 * e * e * sin2m;

   eot_ofs = (eot_ofs * 180.0 / PI) * 4.0;
///eot_ofs -= 0.008343;  // more fudge factor to match SPA results
   lon_ofs = (-lon*RAD_TO_DEG*4.0); // + (time_zone()*60.0);

   return (eot_ofs - lon_ofs);
}




/*
 * heliocentric_ra_dec
 *   - procedure to calculate heliocentric right ascension and
 *     declination of the earth at a given date.
 *
 * Inputs
 *   jd  - the Julian date
 *
 * Returns
 *   *ra  - the right ascension of the earth
 *   *dec - the declination of the earth
 *
 * Heliocentric Julian Date code adapted from code by Richard Ogley.
 */

void heliocentric_ra_dec(double jd, double *ra, double *dec)
{
//const double eccentricity = 0.016718;  /* Eccentricity of the Earth's orbit*/
const double ecliptic_long = 278.833540; /* The longitude of the ecliptic at 1 Jan 1980 0:00 UT */
const double perigee_long = 282.596403;  /* The longitude of perigee at 1 Jan 1980 00:00 UT */
const double deg_to_rad = PI / 180.0;    /* A degrees to radians converion */
const double mjd_1980 = 44238.0;         /* The MJD on 1 Jan 1980 00:00 UT */
double tropical_year = 365.242190402;    /* The length of the tropical year in days */
double obliq = 23.441884*PI/180.0;       /* The obliquity of the orbit */

double mean_anomoly;        /* The mean anomoly of the sun */
double days_from_1980;      /* The number of days from 1 Jan 1980 */
double solar_longitude;     /* The longitude of the sun */
double number_of_deg;       /* The number of degrees in longitude the sun has travelled */
double equation_of_centres; /* The value for the equation of centres */
double x;                   /* An X position */
double y;                   /* A  Y position */
double beta;                /* The ecliptic longitude */ 
double number_of_rotations; /* An integer number of solar orbits */
double mjd;

   /* Calculate the number of days from 1 Jan 1980 */
   
   obliq = obliquity(jd, 1);
   mjd = jd - JD_MJD;  // modified julian date
   days_from_1980 = (mjd - mjd_1980);

   /* Calculate the number of degrees around in the orbit travelled in
      this time */

   tropical_year = solar_year(jd);
   number_of_deg = (360.0 / tropical_year) * days_from_1980;

   /* Adjust so the number of degrees is between 0 and 360 */
   
   if((number_of_deg < 0.0) || (number_of_deg > 360.0)) {
     number_of_rotations = number_of_deg / 360.0;
     number_of_rotations = floor(number_of_rotations);
     number_of_deg -= number_of_rotations * 360.0;
   }

   /* Calculate the mean anomoly */

   mean_anomoly = number_of_deg - perigee_long + ecliptic_long;

   /* Since the orbit is elliptical and not circular, calculate the
      equation of centres */

   equation_of_centres = (360.0 / PI) * orbit_eccentricity(jd) * sin(mean_anomoly * PI/180.0);

   /* Calculate the solar longitude */

   solar_longitude = number_of_deg + equation_of_centres + ecliptic_long;
   if(solar_longitude > 360.0) solar_longitude -= 360.0;
   solar_longitude *= (PI/180.0);

   /* The ecliptic latitude is zero for the Sun. */

   beta = 0.0*PI/180.0;

   /* Calculate the RA and Dec of the sun */
   
   *dec = asin( (sin(beta) * cos(obliq)) +
                (cos(beta) * sin(obliq) *
                 sin(solar_longitude)) );
   
   *dec /= deg_to_rad;
   x = cos(solar_longitude);
   y = (sin(solar_longitude) * cos(obliq)) -
       (tan(beta) * sin(obliq));
   *ra = atan(y/x);
   *ra /= deg_to_rad;

   if(*ra < 0.0) *ra += 360.0;
   *ra /= 15.0;

   /* Convert from geocentric to heliocentric co-ordinates for the Earth*/

   *dec *= -1.0;
   *ra  -= 12.0;
   if(*ra < 0.0) *ra += 24.0;
}

double get_hjd(double jd)
{
const double ausec = 499.01265;     /* Time it takes light to travel 1 AU */
const double degtorad = (PI/180.0); /* Conversion factor from degrees to radians */

struct coordinates {
  double ra;   /* The right ascension in degrees */
  double dec;  /* The declination in degrees */
  double x;    /* The X position in AU */
  double y;    /* The Y position in AU */
  double z;    /* The Z position in AU */
};

struct coordinates earth;  /* Earth co-ordinates */
struct coordinates source; /* Source co-ordinates */

double correction_secs; /* Correction factor in seconds from MJD to HMJD */
double cel;   /* intermediate calculation from spherical to cartesian co-ordinates */
  
   /* Defaults */
  
   earth.ra = earth.dec = 0.0;

   /* Enter the co-ordinates of the source */
   // !!!! Lady Heather uses 0,0 assuming the source is the sun


   /* Calculate the RA and declination in decimal degree notation */

   source.ra = 0.0 * 15.0;
   source.dec = 0.0;

   /* Attempt to find the RA and Dec of the earth using the astronomical
      calculator book. */

   heliocentric_ra_dec(jd, &earth.ra, &earth.dec);

   /* Calculate the heliocentric co-ordinates as X, Y and Z terms */
   cel = cos(earth.dec * degtorad);
   earth.x = cos(earth.ra * degtorad) * cel;
   earth.y = sin(earth.ra * degtorad) * cel;
   earth.z = sin(earth.dec * degtorad);

   /* Calculate the X,Y,Z co-ordinates of the source */
   cel = cos(source.dec * degtorad);
   source.x = cos(source.ra * degtorad) * cel;
   source.y = sin(source.ra * degtorad) * cel;
   source.z = sin(source.dec * degtorad);

   /* Calculate the correction in seconds for the light travel time
      between the source and the earth vectors in a heliocentric
      reference frame. */

   correction_secs = ausec * (earth.x * source.x + earth.y * source.y + earth.z * source.z);

   /* Modify the JD in a heliocentric reference frame */

   jd = jd + (correction_secs / (24.0*60.0*60.0));

   return jd;
}

double utc_delta_t()
{
double dt;

   // return UTC delta T (in Julian days!)

   if(user_set_delta_t) dt = user_delta_t;
   else dt = jtime(0, 0, 51+utc_offset, 0.184);  // TDT is 51.184 seconds ahead of GPS

   return dt;
}


double mars_date(double jd_tt)
{
double md;

   // return Mars date from earth date in terrestrial time
   md = ((jd_tt-2451549.5) / 1.0274912517) + 44796.0 - 0.0009626;
   return md;
}

double planet_date(double vd)
{
   // return planetary date (referenced to GPS epoch)
   vd -= jdate(1899, 12, 31);  // J1900 start time
   return vd;
}

double bessel_date(double jd)
{
   // !!!!! Note: add 1900.0 to get actual besselian date
   // we don't include that here in order to get a few more bits of resoluion
   return ((jd-2415020.31352) / 365.242198781);
}


double gmst(double jd, int apparent)
{
double st;
double d;    // oh, my
double omega;
double epsilon;
double l;
double delta;
double eqeq;
double jc;

   // calculate Greenwich mean or apparent sidereal time

   d = jd - JD2000; 

   // from NREL SPA code
   jc = d / 36525.0;  // julian century
   st = (280.46061837 + 360.98564736629 * (jd - JD2000) +
         jc*jc*(0.000387933 - jc/38710000.0));
   st = fmod(st, 360.0);
   st /= 15.0;

   if(apparent) { // calculate nutation for apparent sidereal time
      omega = 125.04 - (0.052954 * d);
      l = 280.47 + (0.98565 * d); 
      epsilon = 23.4393 - (0.0000004 * d); 
      delta = (-0.000319*sin(omega)) - (0.000024*sin(l+l));
      eqeq = delta * cos(epsilon);
      st += eqeq;
      st = fmod(st, 24.0);
   }

   while(st < 0.0)   st += 24.0;
   while(st >= 24.0) st -= 24.0;
   return st;
}

double lmst(double jd, int apparent)
{
double st;

   // calculate Greenwich mean or apparent local time

   if(lon > 0.0) st = 360.0 - lon*RAD_TO_DEG;  // st = lon west of Greenwich
   else          st = 0.0 - lon*RAD_TO_DEG;
   st = st * 24.0 / 360.0;   // st is the hour offset based upon longitude

   st = gmst(jd, apparent) - st;

   while(st < 0.0)   st += 24.0;
   while(st >= 24.0) st -= 24.0;
   return st;
}

double tai_time(double jd_gps)
{
   return jd_gps + jtime(0, 0, 19, TSX_TWEAK);  // TAI is 19 seconds ahead of GPS
}


double tcg_time(double jd_tt)
{
   return jd_tt + (6.969290134E-10 * (jd_tt-2443144.5003725));  // convert TT to TCG
}


double tdb_time(double jd_tt)
{
double g;

   g = (357.53 + (0.9856003 * (jd_tt - JD2000))) * PI/180.0; // radians
   return jd_tt + (((0.001658 * sin(g)) + (0.000014 * sin(2.0*g))) / (24.0*60.0*60.0)); // jd is TDB
}

double tcb_time(double jd_tt)
{
double g;
double jd;

   g = (357.53 + (0.9856003 * (jd_tt - JD2000))) * PI/180.0; // radians
   jd = jd_tt + (((0.001658 * sin(g)) + (0.000014 * sin(2.0*g))) / (24.0*60.0*60.0)); // jd is TDB

   jd -= 2443144.5003725;         // convert TDB to TCB
   jd /= (1.0 - 1.550519768e-8);
   jd += 2443144.5003725;
   return jd;
}

double loran_time(double jd_gps)
{
   return jd_gps + jtime(0, 0, 9, TSX_TWEAK);  // Loran is 9 seconds ahead of gps
}



int adjust_tz(int why)
{
double st;
double jd;
int round_time;
static int last_why;

   // This routine adjusts the receiver time for the time zone and
   // various astronomical time scales. It sets the pri_xxxx variables to
   // the desired local time / astronomical time.
   //
//jd_utc = jdate(1873,12,29) + 0.50;
//jd_gps = jd_utc + jtime(0,0,utc_offset,0.0);


// dst_ofs = 0;
   jd = 0.0;
   round_time = 0;
   have_local_time = 0;

   jd_astro = jd_display = jd_local;
   if(use_gmst || use_lmst || use_merc || use_msd || use_pluto || use_ven || use_bessel) {
      if(use_msd) {
         st = jd_astro = mars_date(jd_tt);
         st = st - (int) st;
         st *= 24.0;
      }
      else if(use_ven || use_merc || use_pluto) {
         st = jd_astro = planet_date(jd_gps+jtime(0,0,0,TSX_TWEAK));
         st = st - (int) st;
         st *= 24.0;
      }
      else if(use_bessel) {
         st = bessel_date(jd_utc);  // bessel_date returns bessel date offset by year 1900
         jd_astro = 1900.0 + st;    // true bessel date

         st = st - (int) st;
         gregorian(0, jd_utc);
         st = jdate(g_year, 1, 1) + (st * 365.242198781);

         st = st - (int) st;
         st *= 24.0;
      }
      else if(use_gmst) st = gmst(jd_utc+jtime(0,0,0,TSX_TWEAK), use_gmst-1);
      else              st = lmst(jd_utc+jtime(0,0,0,TSX_TWEAK), use_lmst-1);

      pri_hours = (int) st;
      st = (st - pri_hours);       // decimal hour
      st *= 60.0;

      pri_minutes = (int) st;
      st = (st - pri_minutes);     // decimal minutes
      st *= 60.0;

      pri_seconds = (int) st;
      st = (st - pri_seconds);     // decimal seconds
      st_secs = st;                // remainder seconds

      pri_frac = st_secs;

      strcpy(tz_string, std_string);
      round_time = 1;
   }
   else {
//pri_year = 2006;  // Astronomical time scale test data from SOFA
//pri_month= 1;
//pri_day= 15;
//pri_hours = 21;
//pri_minutes = 24;
//pri_seconds = 37+14;
//pri_frac = 0.50;
      if(use_tai) {
         jd = tai_time(jd_gps);
         strcpy(tz_string, std_string);
      }
      else if(use_tt) {
         jd = jd_tt;  // TT is 51.184 seconds ahead of GPS
         strcpy(tz_string, std_string);
         round_time = 1;
      }
      else if(use_ut1) {
         delta_ut1 = (jtime(0, 0, 51+utc_offset, 0.184) - utc_delta_t());
//sprintf(debug_text3, "delta_ut1:%f", delta_ut1*24.0*3600.0);
         jd = jd_utc + delta_ut1;
         strcpy(tz_string, std_string);
         round_time = 1;
      }
      else if(use_tcg) {
         jd = tcg_time(jd_tt);
         strcpy(tz_string, std_string);
         round_time = 1;
      }
      else if(use_tdb) {  // accurate to < 50 uS
         jd = tdb_time(jd_tt);
         strcpy(tz_string, std_string);
         round_time = 1;
      }
      else if(use_tcb) {
         jd = tcb_time(jd_tt);

//jd = jd + (1.550519768e-8*(now - 2443144.5003725)*86400.0 + (-6.55e-5)); // calc TCB
         strcpy(tz_string, std_string);
         round_time = 1;
      }
      else if(use_hjd) {
         jd = jd_astro = get_hjd(jd_utc+jtime(0,0,0,TSX_TWEAK));
         strcpy(tz_string, std_string);
         round_time = 1;
      }
      else if(use_loran) {
         jd = loran_time(jd_gps);
         strcpy(tz_string, std_string);
         have_local_time = 1;
      }
      else if(use_gps) {
         jd = jd_gps;
         strcpy(tz_string, std_string);
         have_local_time = 2;
      }
      else if(use_utc) {
         jd = jd_utc;
         strcpy(tz_string, std_string);
         have_local_time = 3;
      }
      else if(solar_time) {
         jd = jd_utc + jtime(0,0,0,TSX_TWEAK);  // adjust time for receiver message delays
         jd += (eot(jd_utc) / (24.0*60.0));
         round_time = 1;
      }
      else {
         if(time_flags & TFLAGS_UTC) jd = jd_utc;  // we want UTC based times
         else                        jd = jd_gps;  // displaying GPS based times

         if((leap_time || leap_sixty) && (TSX_TWEAK < 0.0)) ;  // negative TSX_TWEAK would mess up leap second display
         else jd += jtime(0,0,0,TSX_TWEAK);    // adjust time for receiver message delays

         if(!solar_time) jd += time_zone();    // adjust time for the time zone

         have_local_time = 4;
      }


      if(leap_sixty) {  // receiver sent a time of xx:59:60
         gregorian(0, jd - jtime(0,0,1,0.0));   // convert adjusted Julian back to Gregorian
         g_seconds = 60;  // display leapsecond time to xx:59:60
      }
      else {
         gregorian(0, jd);
      }

round_time = 0;  // we no longer do time rounding
      if(round_time) {       // round time up to nearest second;
         jd += jtime(0,0,0, 0.5);
      }

      jd_display = jd;
      pri_day = g_day;         // set local time values to the adjusted time
      pri_month = g_month;
      pri_year = g_year;
      pri_hours = g_hours;        
      pri_minutes = g_minutes;
      pri_seconds = g_seconds;
      pri_frac = g_frac;       // note: usually 1msec fast due to rounding compensation in gregorian(0, )
      if(round_time) {
         pri_frac = g_frac = 0;
      }
   }

   last_why = why;
   return 1;
}


double get_alarm_time()
{

   // Get the time to trigger an alarm into the g_ variables and also return
   // it as a jdate double.
   //
   // This time can either be the local time or the displayed time (if it is
   // one of the astronomical time scales) depending upon the setting of the
   // alarm_type option.

   if(alarm_type == 0) {  // alarm/dump/exit on local time
      gregorian(0, jd_local);
      return jd_local;
   }
   else {  // alarm/dump/exit on displayed (possibly astronomical time)
      g_year    = pri_year;   
      g_month   = pri_month;
      g_day     = pri_day;
      g_hours   = pri_hours;
      g_minutes = pri_minutes;
      g_seconds = pri_seconds;
      g_frac    = pri_frac;

      return jdate(g_year,g_month,g_day) + jtime(g_hours,g_minutes,g_seconds,0.0);
   }
}

void reset_alarm()
{
   // reset the alarm, turn off modem control signals if enabled
   if(modem_alarms) {  // turn off modem control signal that indicates alarm
      Sleep(250);  // allow minimum modem control signal width
      // !!!!!! manipulate modem control signals here
      if(com[ALARM_PORT].com_port > 0) { // COM port in use
         SetDtrLine(ALARM_PORT, 1);  // drive +12V
         SetRtsLine(ALARM_PORT, 0);  // drive -12V
      }
   }
}

void enable_alarm()
{
   if(modem_alarms) {
      // !!!!!! manipulate modem control signals here
      if(com[ALARM_PORT].com_port > 0) { // COM port in use
         SetDtrLine(ALARM_PORT, 0);  // drive -12V
         SetRtsLine(ALARM_PORT, 1);  // drive +12V
      }
   }
}


void check_end_times()
{
double this_jd;
static double last_jd = 1E100;

   // check to see if it is time to do a scheduled screen or log dump or to exit
   if(time_flags & TFLAGS_BAD_TIME) return;

   this_jd = get_alarm_time();

   // if time only specified, add in the current date...
   // plus 1 day if less than the current date/time
   if(alarm_jd < 1.0) {
      alarm_jd += jdate(pri_year,pri_month,pri_day);
      if(alarm_jd < this_jd) alarm_jd += 1.0;
   }
   if(dump_jd < 1.0) {
      dump_jd += jdate(pri_year,pri_month,pri_day);
      if(dump_jd < this_jd) dump_jd += 1.0;
   }
   if(exec_jd < 1.0) {
      exec_jd += jdate(pri_year,pri_month,pri_day);
      if(exec_jd < this_jd) exec_jd += 1.0;
   }
   if(exit_jd < 1.0) {
      exit_jd += jdate(pri_year,pri_month,pri_day);
      if(exit_jd < this_jd) exit_jd += 1.0;
   }
   if(log_jd < 1.0) {
      log_jd += jdate(pri_year,pri_month,pri_day);
      if(log_jd < this_jd) log_jd += 1.0;
   }
   if(script_jd < 1.0) {
      script_jd += jdate(pri_year,pri_month,pri_day);
      if(script_jd < this_jd) script_jd += 1.0;
   }


   if(exit_timer) {  // countdown timer set
      if(--exit_timer == 0) goto time_exit;
   }

   // see if user wants us to exit at this time every day
   if(end_time || end_date) {    
      if((last_jd < exit_jd) && (this_jd >= exit_jd)) {
         if(end_date) {
            goto time_exit;
         }
         else {
            time_exit:
            printf("\nUser set exit time has been reached\n");
            shut_down(100);
         }
      }
   }


   if(alarm_time || alarm_date) {  // alarm clock has been set
      if((last_jd < alarm_jd) && (this_jd >= alarm_jd)) {
         if(alarm_date) {  // sound alarm at a given date and time
            sound_alarm |= ALARM_DATE;
            enable_alarm();
            alarm_wait = 0;
         }
         else {  // sound alarm at a given time
            sound_alarm |= (ALARM_TIME | ALARM_SET);
            enable_alarm();
            alarm_wait = 0;
         }
      }
   }

   if(dump_time || dump_date) {  // screen dump clock has been set
      if((last_jd < dump_jd) && (this_jd >= dump_jd)) {
         if(dump_date) {  // do screen dump at a given date and time
            do_screen_dump |= ALARM_DATE;
         }
         else {  // dump screen at a given time
            do_screen_dump |= ALARM_TIME;
         }
      }
   }

   if(log_time || log_date) {  // log dump clock has been set
      if((last_jd < log_jd) && (this_jd >= log_jd)) {
         if(log_date) {
            do_log_dump |= ALARM_DATE;
         }
         else {
            do_log_dump |= ALARM_TIME;
         }
      }
   }

   if(script_time || script_date) {  // script run clock has been set
      if((last_jd < script_jd) && (this_jd >= script_jd)) {
         if(script_date) {
            do_script_run |= ALARM_DATE;
         }
         else {
            do_script_run |= ALARM_TIME;
         }
      }
   }

   if(exec_time || exec_date) {  // program run clock has been set
      if((last_jd < exec_jd) && (this_jd >= exec_jd)) {
         if(exec_date) {
            do_exec_run |= ALARM_DATE;
         }
         else {
            do_exec_run |= ALARM_TIME;
         }
      }
   }

   last_jd = this_jd;
}

void get_delta_t()
{
FILE *file;

   // periodically try to get delta_t from a file
   gregorian(0, jd_utc);
   if((g_hours == DELTAT_HOUR) && (g_minutes == DELTAT_MINUTE) && (g_seconds == DELTAT_SECOND)) {
      need_delta_t = 1;
   }

   if(need_delta_t && (user_set_delta_t != 1)) { // time to get delta_t from a file
      if(0) {
         file = topen(lock_name, "r");     // check for lock file
         if(file) {        // deltat.dat is being updated
            fclose(file);
            return;
         }
      }

      file = topen(DELTAT_FILE, "r");
      if(file) {
         if(fgets(out, sizeof(out)-1, file) != NULL) {
            user_delta_t = atof(out);
            user_delta_t /= (24.0*60.0*60.0);  // convert seconds to days
            user_set_delta_t = 2;
         }
         fclose(file);
      }
      need_delta_t = 0;
   }
}


char *get_tz_value(char *s)
{
int c;
u08 saw_colon;

   if(s == 0) return s;

// tz_sign = 1;
   time_zone_hours = 0;
   time_zone_minutes = 0;
   time_zone_seconds = 0;
   tz_adjust = 0.0;

   saw_colon = 0;
   while(1) { // get time zone hour offset value
      c = *s;
      if(c == 0) {
         s = 0;
         break;
      }
      if     (c == '+') tz_sign = (0+tz_sign); 
      else if(c == '-') tz_sign = (0-tz_sign);
      else if(c == ':') ++saw_colon;
      else if(isdigit(c)) {
         if     (saw_colon == 0) time_zone_hours = (time_zone_hours * 10) + (c-'0');
         else if(saw_colon == 1) time_zone_minutes = (time_zone_minutes * 10) + (c-'0');
         else if(saw_colon == 2) time_zone_seconds = (time_zone_seconds * 10) + (c-'0'); 
      }
      else break;
      ++s;
   }
   time_zone_hours %= 24;


   time_zone_hours *= tz_sign;
   time_zone_minutes %= 60;
   time_zone_minutes *= tz_sign;
   time_zone_seconds %= 60;
   time_zone_seconds *= tz_sign;
   tz_adjust = jtime(time_zone_hours,time_zone_minutes,time_zone_seconds, 0.0);

   return s;
}

char *get_std_zone(char *s, int digit_stop)
{
int c;
int j;

   if(s == 0) return s;
   if(*s == 0) return 0;

   c = 0;
   std_string[0] = 0;
   j = 0;
   while(j < 20) {
      c = *s;
      if(c == '/') break;
      if(c == '-') break;
      if(c == '+') break;
      if(digit_stop && isdigit(c)) break;

      ++s;
      if(c == ' ') continue;
      if(c == '\t') continue;

      std_string[j++] = toupper(c);
      std_string[j] = 0;

      if(j >= TZ_NAME_LEN) break;
      else if(*s == 0)    { s=0; break; }
      else if(*s == 0x0D) { s=0; break; }
      else if(*s == 0x0A) { s=0; break; }
   }

   if(std_string[0] == 0) strcpy(std_string, tz_string);
   return s;
}

char *get_dst_zone(char *s)
{
int c;
int j;

   if(dst_area == 0) dst_area = USA;
   dst_string[0] = 0;
   j = 0;
   while(j < 20) {
      c = *s;
      if(c == 0) { s=0; break; }
      ++s;

      if(c == '/') break;
      if(c == ' ') continue;
      if(c == '\t') continue;

      dst_string[j++] = toupper(c);
      dst_string[j] = 0;

      if     (j >= TZ_NAME_LEN) break;
      else if(*s == 0)    { s=0; break; }
      else if(*s == 0x0D) { s=0; break; }
      else if(*s == 0x0A) { s=0; break; }
   }

   if(1 && (user_set_dst == 0)) {  // user has not set the DST area
      strcpy(out, dst_string);
      strupr(out);
      if(strcmp(out, "BST") == 0) {  // UK/euro time
         dst_area = 2;  // set the DST area to Euro-land
      }
      else {  // assume USA
         dst_area = 1;
      }
      calc_dst_times(this_year, dst_list[dst_area]);
      dst_ofs = dst_offset();
   }

   return s;
}

void set_time_zone(char *s)
{
char c;

   time_zone_set = 0;

   use_loran = use_gps = use_utc = 0;
   use_gmst = use_lmst = 0;
   use_tai = use_tt = use_ut1 = use_tcg = use_tdb = use_tcb = use_hjd = use_bessel = 0;
   use_msd = use_ven = use_merc = use_pluto = 0;

   tz_string[0] = dst_string[0] = 0;
   if(s == 0) return;

   if((s[0] == 0) || (s[0] == 0x0A) || (s[0] == 0x0D)) {  // clear time zone
      return;
   }

   if(s[0] == ':') return;  // Linux :... time zone string

   // set time zone
   strcpy(std_string, "LOCAL");
   dst_string[0] = 0;
   strcpy(tz_string, dst_string);

   time_zone_set = 1;
   tz_sign = 1;
   time_zone_hours = 0;
   time_zone_minutes = 0;
   time_zone_seconds = 0;
   tz_adjust = 0.0;

   while(*s) {  // find first char of time zone string
      c = *s;
      if(c == 0)    { break; }
      if(c == '>')  { break; }
      if(c == '<')  { ++s; continue; }
      if(c == ' ')  { ++s; continue; }
      if(c == '\t') { ++s; continue; }
      if(c == '=')  { ++s; continue; }

      if((c == '+') || (c == '-') || isdigit(c)) {  // tz string is -6CST/CDT format
         s = get_tz_value(s);
         if(s) s = get_std_zone(s, 0);
         if(s) {
            if(*s == '/') get_dst_zone(s+1);
         }
      }
      else {   // string is in CST6CDT format
         s = get_std_zone(s, 1);
         if(s) {
            tz_sign = (-1);
            s = get_tz_value(s);
         }
         if(s) get_dst_zone(s);
      }
      break;
   }

   if     (!strcmp(std_string, "SST"))   solar_time = 1;
   else if(!strcmp(dst_string, "SDT"))   solar_time = 1;
   else if(!strcmp(std_string, "SOL"))   solar_time = 1;
   else if(!strcmp(dst_string, "SOL"))   solar_time = 1;
   else if(!strcmp(std_string, "SOLAR")) solar_time = 1;
   else if(!strcmp(dst_string, "SOLAR")) solar_time = 1;
   else                                  solar_time = 0;

   if(user_set_watch_name == 0) {
      if(solar_time) {
         set_watch_name("Solar");
         label_watch_face = 1;
      }
      else {
         watch_name[0] = 0;
         label_watch_face = 1;
      }
   }

   if     (!strcmp(std_string, "LMST"))  use_lmst = 1;   // sidereal times
   else if(!strcmp(std_string, "LAST"))  use_lmst = 2;
   else if(!strcmp(std_string, "GMST"))  use_gmst = 1;
   else if(!strcmp(std_string, "GAST"))  use_gmst = 2;

   else if(!strcmp(std_string, "TAI"))   use_tai = 1;    // astronomical times
   else if(!strcmp(std_string, "TCG"))   use_tcg = 1;
   else if(!strcmp(std_string, "TCB"))   use_tcb = 1;
   else if(!strcmp(std_string, "TDB"))   use_tdb = 1;
   else if(!strcmp(std_string, "TDT"))   use_tt = 1;
   else if(!strcmp(std_string, "TT"))    use_tt = 1;
   else if(!strcmp(std_string, "UT"))    use_ut1 = 1;
   else if(!strcmp(std_string, "UT1"))   use_ut1 = 1;
   else if(!strcmp(std_string, "HJD"))   use_hjd = 1;
   else if(!strcmp(std_string, "BES"))   use_bessel= 1;

   else if(!strcmp(std_string, "UTC"))   use_utc = 1;    // various nav system times
   else if(!strcmp(std_string, "GPS"))   use_gps = 1;
   else if(!strcmp(std_string, "GPST"))  use_gps = 1;
   else if(!strcmp(std_string, "LOR"))   use_loran = 1;
   else if(!strcmp(std_string, "LORAN")) use_loran = 1;

   else if(!strcmp(std_string, "MAR"))   use_msd = 1;    // various names of Mars time
   else if(!strcmp(std_string, "MARS"))  use_msd = 1;  
   else if(!strcmp(std_string, "MARSD")) use_msd = 1;  
   else if(!strcmp(std_string, "MSD"))   use_msd = 1;  
   else if(!strcmp(std_string, "MTC"))   use_msd = 1;  
   else if(!strcmp(std_string, "AMT"))   use_msd = 1;  

   else if(!strcmp(std_string, "MER"))   use_merc = 1;   // planetary times
   else if(!strcmp(std_string, "PLU"))   use_pluto = 1;
   else if(!strcmp(std_string, "VEN"))   use_ven = 1;
}


double time_zone()
{
   // return the time zone adjustment factor (with dst correction) in Julian days

// dst_ofs = dst_offset();   // we could do this in primary_misc() when the hour changes
   return tz_adjust + jtime(dst_ofs,0,0,0.0);
}

int ships_bells()
{
int hh;
   
   // ships bells (simplified, no dog watch)

   hh = (g_hours % 4) * 2;
   if(g_minutes == 0) return hh;
   else if(g_minutes == 30) return hh+1;
   else return (-1);
}


void silly_clocks()
{
char s[32];
FILE *lock_file;
static int last_silly_sec = (-99);
static int last_year = (-9999);
char *ext;

   gregorian(0, jd_local);   // for the following functions that always trigger on local time
   if(nav_rate != 1) {
      if(g_seconds == last_silly_sec) return;
      last_silly_sec = g_seconds;
   }

   #ifdef GREET_STUFF
      if(greet_ok || ((g_hours == GREET_HOUR) && (g_minutes == GREET_MINUTE) && (g_seconds == GREET_SECOND))) {
         show_greetings();
         greet_ok = 0;
         gregorian(0, jd_local);
      }
   #endif



   if(sun_file && play_sun_song && !no_easter_eggs) {  // sunrise/sunset file
      if((have_sun_times & 0x01) && (g_hours == rise_hh) && (g_minutes == rise_mm) && (g_seconds == rise_ss)) {
         play_tune(SUNRISE_FILE, 1);
      }
      else if((have_sun_times & 0x04) && (g_hours == set_hh) && (g_minutes == set_mm) && (g_seconds == set_ss)) {
         play_tune(SUNRISE_FILE, 1);
      }
   }

   if(noon_file && play_sun_song && !no_easter_eggs) {  // solar noon file
      if((have_sun_times & 0x02) && (g_hours == noon_hh) && (g_minutes == noon_mm) && (g_seconds == noon_ss)) {
         play_tune(NOON_FILE, 1);
      }
   }

   if(tick_clock) {  // ticking clock in use
      if(fine_tick_clock) {  // try to tick more closely to actual time, not message arival time
         // the fine tick clock extends the tick to the true next second,
         // so we need to trigger the minute sound when the message time is
         // at :59.xxx seconds (!!!!!!! is this true for receivers with negative /tsx values?)
if(fine_tick_msec >= 0.0) { // a pending tick was not played by fine_tick_check()
  sound_tick_clock(); 
}
         fine_tick_msec = GetMsecs() + (1000.0 - (pri_frac*1000.0) - TICK_SOUND_DELAY);  // used for millisecond accurate tick clock
         if(fine_tick_msec < 0.0) fine_tick_msec = GetMsecs();  

         minute_tick = 0;
         if(time_sync_offset >= 0.0) {  // time message arrives after actual time
            if(pri_seconds == 59) minute_tick = 1;
         }
         else {  // time message arrives before actual time
            if(pri_seconds == 59) minute_tick = (-1);
         }
      }
      else {  // tick when message comes in
         if(pri_seconds || (tick_clock == 1)) {  // sound the seconds tick sound
            if(tick_clock != 2) {
               if(seconds_file) play_tune(SECONDS_FILE, 1);
               else if(click_file) play_tune(CLICK_FILE, 1);
               else BEEP(200);
            }
         }
         else if(minute_file) play_tune(MINUTE_FILE, 1); // minute tick sound        
         else BEEP(201);
      }
   }

   // This routine plays a jaunty tune shortly after a leap second

   if(0 && leap_file && leap_pending && ((minor_alarms & MINOR_LEAP_PEND) == 0)) { 
      leaped = 1;   // the leap second flag just cleared
   }

   leap_pending = (minor_alarms & MINOR_LEAP_PEND);
   if(leaped && (g_seconds == LEAP_SECOND)) {
      leaped = 0;
      if(!no_easter_eggs) play_tune(LEAP_FILE, 1);
   }
   else if(1 && ((pri_year - last_year) == 1)) {
      if((pri_month == 1) && (pri_day == 1) && (pri_seconds > 0)) {
         play_tune(NEW_YEAR_NAME, 1);
         last_year = pri_year;
      }
   }
   else last_year = pri_year;


   // gratuitously silly cuckcoo clock mode (this triggers on displayed time)
   get_alarm_time();  // for functions that can trigger on local or displayed (pri_xxxx times)

   if(tick_clock) ;
   else if(ships_clock && (g_seconds == CUCKOO_SECOND)) {
      if((g_minutes == 0) || (g_minutes == 30)) {
         ring_bell = ships_bells();
         if(ring_bell >= 0) bell_number = 0;
      }
   }
   else if(cuckoo && (g_seconds == CUCKOO_SECOND)) { 
      // note that the singing_clock is handled as a special case of the cuckoo clock
      if(cuckoo_hours && (g_minutes == 0)) {  // cuckoo the hour on the hour
         cuckoo_beeps = (g_hours % 12);
         if(cuckoo_beeps == 0) cuckoo_beeps = 12;
      }
      else if((g_minutes%(60/cuckoo)) == 0) {  // cuckoo on the marks
         if     (cuckoo_hours)    cuckoo_beeps = 1;
         else if(g_minutes == 0)  cuckoo_beeps = 3;
         else if(g_minutes == 30) cuckoo_beeps = 2;
         else                     cuckoo_beeps = 1;
      }
   }

   if(egg_timer) {
      if(--egg_timer == 0) {
         sound_alarm |= ALARM_TIMER;
         enable_alarm();
         alarm_wait = 0;
         if(repeat_egg) egg_timer = egg_val;
      }
   }

   if(dump_timer) {
      if(--dump_timer == 0) {
         do_screen_dump |= ALARM_TIMER;
         if(repeat_dump) dump_timer = dump_val;
      }
   }

   if(log_timer) {
      if(--log_timer == 0) {
         do_log_dump |= ALARM_TIMER;
         if(repeat_log) log_timer = log_val;
      }
   }

   if(script_timer) {
      if(--script_timer == 0) {
         do_script_run |= ALARM_TIMER;
         if(repeat_script) script_timer = script_val;
      }
   }

   if(exec_timer) {
      if(--exec_timer == 0) {
         do_exec_run |= ALARM_TIMER;
         if(repeat_exec) exec_timer = exec_val;
      }
   }

   if(sound_alarm) {  // sounding the alarm clock
      if(sound_alarm & ALARM_TIME) {
         alarm_jd += 1.0;  // trigger again tomorrow
         if(sound_alarm == ALARM_TIME) {
            reset_alarm();
         }
         sound_alarm &= (~ALARM_TIME);
      }

      alarm_wait = 0;
      alarm_clock();  
      if(single_alarm) {  // sound alarm once,  useful if playing long sound file
         if(sound_alarm & ALARM_DATE) {
            alarm_time = alarm_date = 0;
            alarm_jd = 0.0;
            egg_timer = egg_val = 0;
            repeat_egg = 0;
         }

         reset_alarm();
         sound_alarm = 0;
      }

      if(luxor) {  // turn off battery when alarm sounds
         cc_mode = 0;
         sweep_stop = 0.0F;
         set_batt_pwm(0x0000);
      }
   }


   if(do_script_run) {  // running a sceduled script
      if(do_script_run & ALARM_TIME) script_jd += 1.0;  // trigger again tomorrow
      open_script("timer.scr");  
      do_script_run = 0;
   }


   if(do_exec_run) {  // running a scheduled program
      if(do_exec_run & ALARM_TIME) exec_jd += 1.0;  // trigger again tomorrow
      exec_program();
      do_exec_run = 0;
   }
   
   if(do_screen_dump) {  // perform a scheduled screen dump
      if(do_screen_dump & ALARM_TIME) dump_jd += 1.0;  // trigger again tomorrow
      ++dump_number;
      get_alarm_time();
      if(single_dump) {
         sprintf(s, "%sdump", dump_prefix);
      }
      else {
        sprintf(s, "%s%04d-%02d-%02d-%ld", dump_prefix, g_year,g_month,g_day,dump_number);
      }

      lock_file = topen(lock_name, "w");  // create file tblock to signify dump file is being written
      if(lock_file) {
         fprintf(lock_file, "1");
         fclose(lock_file);
      }
      dump_screen(invert_dump, top_line, s);
      path_unlink(lock_name);

      do_screen_dump = 0;
   }
   
   if(do_log_dump && (doing_log_dump == 0)) {   // scheduled write log data to a file
      if(do_log_dump & ALARM_TIME) log_jd += 1.0;  // trigger again tomorrow
      ++log_number;
      get_alarm_time();

      if(dump_xml) ext = ".xml";
      else if(dump_gpx) ext = ".gpx";
      else ext = ".log";   // RINEX can't be written from queue data

      if(single_log) {
         if(!strcmp(dump_prefix, "tb")) sprintf(s, "%s%s", unit_file_name, ext);
         else sprintf(s, "%s%s%s", dump_prefix, unit_file_name, ext);
      }
      else {
         sprintf(s, "%s%04d-%02d-%02d-%ld%s", dump_prefix, g_year,g_month,g_day,log_number, ext);
      }

      dump_log(s, 'q');

      do_log_dump = 0;
   }

   if(tick_clock) {  // ticking clock
   }
   else if(ships_clock && (ring_bell >= 0) && (bell_number >= 0) && (bell_number < 12)) {
      cuckoo_clock();
   }
   else if(cuckoo_beeps) { // cuckoo clock
      cuckoo_clock(); 
      if(cuckoo_beeps) --cuckoo_beeps;
   }
}

void need_time_set()
{
   set_system_time = 2;
   time_set_char = '*';
}

#define CLK_SORT 11
double clk_sort[CLK_SORT+1];
int clk_sort_count;

double time_sync_median(double delta)
{
int i;
int k, m;

   // This routine calculates the median of a string of system clock vs 
   // receiver time samples.  It returns 0 if median is not available,
   // otherwise it returns the value of the median.  This value is used
   // to determine when the system clock has diverged enough from
   // the reseiver time to cause a system clock reset.

   if(clk_sort_count < 0) clk_sort_count = 0;

   if(clk_sort_count >= CLK_SORT-1) {  // we have enough samples, get the median value
      delta = clk_sort[clk_sort_count/2];
      delta += clk_sort[clk_sort_count/2-1];
      delta += clk_sort[clk_sort_count/2+1];
      delta /= 3.0;
      clk_sort_count = 0;
      return fabs(delta);
   }

   if(clk_sort_count == 0) {   // first entry
      clk_sort[clk_sort_count] = delta;
      ++clk_sort_count;
      goto show_sort;
   }

   if(delta >= clk_sort[clk_sort_count-1]) {  // insert at end of list
      clk_sort[clk_sort_count] = delta;
      ++clk_sort_count;
      goto show_sort;
   }

   for(i=0; i<=clk_sort_count; i++) {  // insert new value into sorted list
      for(k=0; k<clk_sort_count; k++) {  // see where this value lies in the list so far
         if(delta > clk_sort[k]) continue;  // it's greater than the current value

         for(m=clk_sort_count; m>k; m--) {  // move lower value entries down one position
            clk_sort[m] = clk_sort[m-1];
         }

         clk_sort[k] = delta;   // insert new value in the list
         ++clk_sort_count;
         goto show_sort;
      }
   }
   return 0.0;

   show_sort:
// sprintf(plot_title, "sort %d: ", clk_sort_count);  // rrrrr
// for(i=0; i<clk_sort_count; i++) {
//    sprintf(out, "%.1f ", clk_sort[i]);
//    strcat(plot_title, out);
// }

   return 0.0;
}


//
//
//   Calendar stuff
//

double spring;          // Julian date of the seasons
double summer;
double fall;
double winter;

struct CHINA {
   u16 month_mask;
   u08 leap_month;
   float jd;
} china_data[] = {
   { 0x1A93,  5, 2454857.5 },  // 4706 2009
   { 0x0A95,  0, 2455241.5 },  // 4707 2010
   { 0x052D,  0, 2455595.5 },  // 4708 2011
   { 0x0AAD,  4, 2455949.5 },  // 4709 2012
   { 0x0AB5,  0, 2456333.5 },  // 4710 2013
   { 0x15AA,  9, 2456688.5 },  // 4711 2014
   { 0x05D2,  0, 2457072.5 },  // 4712 2015
   { 0x0DA5,  0, 2457426.5 },  // 4713 2016
   { 0x1D4A,  6, 2457781.5 },  // 4714 2017
   { 0x0D4A,  0, 2458165.5 },  // 4715 2018
   { 0x0C95,  0, 2458519.5 },  // 4716 2019
   { 0x152E,  4, 2458873.5 },  // 4717 2020
   { 0x0556,  0, 2459257.5 },  // 4718 2021
   { 0x0AB5,  0, 2459611.5 },  // 4719 2022
   { 0x11B2,  2, 2459966.5 },  // 4720 2023
   { 0x06D2,  0, 2460350.5 },  // 4721 2024
   { 0x0EA5,  6, 2460704.5 },  // 4722 2025
   { 0x0725,  0, 2461088.5 },  // 4723 2026
   { 0x064B,  0, 2461442.5 },  // 4724 2027
   { 0x0C97,  5, 2461796.5 },  // 4725 2028
   { 0x0CAB,  0, 2462180.5 },  // 4726 2029
   { 0x055A,  0, 2462535.5 },  // 4727 2030
   { 0x0AD6,  3, 2462889.5 },  // 4728 2031
   { 0x0B69,  0, 2463273.5 },  // 4729 2032
   { 0x1752, 11, 2463628.5 },  // 4730 2033
   { 0x0B52,  0, 2464012.5 },  // 4731 2034
   { 0x0B25,  0, 2464366.5 },  // 4732 2035
   { 0x1A4B,  6, 2464720.5 },  // 4733 2036
   { 0x0A4B,  0, 2465104.5 },  // 4734 2037
   { 0x04AB,  0, 2465458.5 },  // 4735 2038
   { 0x055D,  5, 2465812.5 },  // 4736 2039
   { 0x05AD,  0, 2466196.5 },  // 4737 2040
   { 0x0B6A,  0, 2466551.5 },  // 4738 2041
   { 0x1B52,  2, 2466906.5 },  // 4739 2042
   { 0x0D92,  0, 2467290.5 },  // 4740 2043
   { 0x1D25,  7, 2467644.5 },  // 4741 2044
   { 0x0D25,  0, 2468028.5 },  // 4742 2045
   { 0x0A55,  0, 2468382.5 },  // 4743 2046
   { 0x14AD,  5, 2468736.5 },  // 4744 2047
   { 0x04B6,  0, 2469120.5 },  // 4745 2048
   { 0x05B5,  0, 2469474.5 },  // 4746 2049
   { 0x0DAA,  3, 2469829.5 },  // 4747 2050
   { 0x0EC9,  0, 2470213.5 },  // 4748 2051
   { 0x1E92,  8, 2470568.5 },  // 4749 2052
   { 0x0E92,  0, 2470952.5 },  // 4750 2053
   { 0x0D26,  0, 2471306.5 },  // 4751 2054
   { 0x0A56,  6, 2471660.5 },  // 4752 2055
   { 0x0A57,  0, 2472043.5 },  // 4753 2056
   { 0x0556,  0, 2472398.5 },  // 4754 2057
   { 0x06D5,  4, 2472752.5 },  // 4755 2058
   { 0x0755,  0, 2473136.5 },  // 4756 2059
   { 0x0749,  0, 2473491.5 },  // 4757 2060
   { 0x0E93,  3, 2473845.5 },  // 4758 2061
   { 0x0693,  0, 2474229.5 },  // 4759 2062
   { 0x152B,  7, 2474583.5 },  // 4760 2063
   { 0x052B,  0, 2474967.5 },  // 4761 2064
   { 0x0A5B,  0, 2475321.5 },  // 4762 2065
   { 0x155A,  5, 2475676.5 },  // 4763 2066
   { 0x056A,  0, 2476060.5 },  // 4764 2067
   { 0x0B65,  0, 2476414.5 },  // 4765 2068
   { 0x174A,  4, 2476769.5 },  // 4766 2069
   { 0x0B4A,  0, 2477153.5 },  // 4767 2070
   { 0x1A95,  8, 2477507.5 },  // 4768 2071
   { 0x0A95,  0, 2477891.5 },  // 4769 2072
   { 0x052D,  0, 2478245.5 },  // 4770 2073
   { 0x0AAD,  6, 2478599.5 },  // 4771 2074
   { 0x0AB5,  0, 2478983.5 },  // 4772 2075
   { 0x05AA,  0, 2479338.5 },  // 4773 2076
   { 0x0BA5,  4, 2479692.5 },  // 4774 2077
   { 0x0DA5,  0, 2480076.5 },  // 4775 2078
   { 0x0D4A,  0, 2480431.5 },  // 4776 2079
   { 0x1C95,  3, 2480785.5 },  // 4777 2080
   { 0x0C96,  0, 2481169.5 },  // 4778 2081
   { 0x194E,  7, 2481523.5 },  // 4779 2082
   { 0x0556,  0, 2481907.5 },  // 4780 2083
   { 0x0AB5,  0, 2482261.5 },  // 4781 2084
   { 0x15B2,  5, 2482616.5 },  // 4782 2085
   { 0x06D2,  0, 2483000.5 },  // 4783 2086
   { 0x0EA5,  0, 2483354.5 },  // 4784 2087
   { 0x0E4A,  4, 2483709.5 },  // 4785 2088
   { 0x068B,  0, 2484092.5 },  // 4786 2089
   { 0x0C97,  8, 2484446.5 },  // 4787 2090
   { 0x04AB,  0, 2484830.5 },  // 4788 2091
   { 0x055B,  0, 2485184.5 },  // 4789 2092
   { 0x0AD6,  6, 2485539.5 },  // 4790 2093
   { 0x0B6A,  0, 2485923.5 },  // 4791 2094
   { 0x0752,  0, 2486278.5 },  // 4792 2095
   { 0x1725,  4, 2486632.5 },  // 4793 2096
   { 0x0B45,  0, 2487016.5 },  // 4794 2097
   { 0x0A8B,  0, 2487370.5 },  // 4795 2098
   { 0x149B,  2, 2487724.5 },  // 4796 2099
   { 0x04AB,  0, 2488108.5 },  // 4797 2100 
   { 0x095B,  7, 2488462.5 },  // 4798 2101 
   { 0x05AD,  0, 2488846.5 },  // 4799 2102 
   { 0x0BAA,  0, 2489201.5 },  // 4800 2103 
   { 0x1B52,  5, 2489556.5 },  // 4801 2104 
   { 0x0D92,  0, 2489940.5 },  // 4802 2105 
   { 0x0D25,  0, 2490294.5 },  // 4803 2106 
   { 0x1A4B,  4, 2490648.5 },  // 4804 2107 
   { 0x0A55,  0, 2491032.5 },  // 4805 2108 
   { 0x14AD,  9, 2491386.5 }   // 4806 2109 
};

double ctimes[40];
int cyear[40];
int cmonth[40];


double chinese_ny()
{
double jd;
double p, last_p;
double li_chun;
int hour;
int new_moons;
int rising;
double first_new;
  
   // calculate the date of the Chinese new year as the closest new moon
   // to the midpoint between winter and spring (li chun)

   if((this_year >= 2009) && (this_year <= 2100)) {  // we cheat, get date from table
      return china_data[this_year-2009].jd;
   }

   jd = winter - 365.24274306;      // previous winter solstice
   li_chun = (spring + jd) / 2.0;   // li chun is midway between winter and spring

   rising  = 0;
   new_moons = 0;
   first_new = 0.0;
   for(hour= (-24*32); hour<24*32; hour++) {  // look for new moon closet to li chun
      jd = li_chun + ((double) hour)/24.0;
      if(new_moons == 0) last_p = moon_phase(jd);
      new_moons = 1;
      p = moon_phase(jd);

      if((rising < 0) && (p > last_p)) {  // new moon
         if(jd < li_chun) first_new = jd; // before li_chun
         else if(jd > li_chun) break;     // first moon after li chun
      }

      if     (p > last_p) rising = (+1);
      else if(p < last_p) rising = (-1);
      last_p = p;
   }

   if     (this_year == 2034) p = jd; 
   else if(this_year == 2053) p = jd;
   else if((jd-li_chun) > (li_chun - first_new)) p = first_new;
   else                                          p = jd;

   p += 7.0/24.0;   // china time ahead of GMT
   return p;
}
//

#ifdef GEN_CHINA_DATA
   void load_china_data()
   {
   FILE *f, *o;
   s16 nyears;
   s16 index;
   int i;
   u32 val;
   u32 jd;
   long yy;
   double x,y;

      f = topen("chinese.dat", "rb");
      if(f == 0) return;
      o = topen("ccc", "w");
      if(o == 0) return;

      fread(&nyears, 2, 1, f);
      fread(&index, 2, 1, f);
      fprintf(o, "ny=%d  ndx=%d\n", (int) nyears, (int) index);

      val = 0;
      for(i=0; i<nyears; i++) {
         yy = index+i;

         fread(&val, 3, 1, f);
         jd = val >> 13;

         fprintf(o, "   { 0x%04lX, %2ld, ", val&0x1FFFL, jd%14L);
         jd = (yy*365L) + (yy/4L) + 757862L + jd/14L;
         fprintf(o, "%.1f },  // %ld %ld \n", ((float) jd)-0.5F, yy+60L, yy-4635L+1998L);
      }

      for(i=2009; i<=2100; i++) {
         pri_year = i;
         calc_seasons();
         x = chinese_ny();
         y = china_data[i-2009].jd;
         fprintf(o, "%d %f %f %f:  ", i, x, y, x-y);
         gregorian(0, x);
         fprintf(o, "ms:%d/%d  ", g_day,g_month);
         gregorian(0, y);
         fprintf(o, "pp:%d/%d\n", g_day,g_month);
      }

      fclose(o);
      fclose(f);
   }

   void load_hk_data()
   {
   FILE *f, *o;
   int yy,mm,dd;
   char c;
   int cday;
   int last_cday;
   int last_cmonth;
   int leap;
   int mask;
   double jd, last_jd;
   int months;

      f = topen("hk_data.txt", "r");
      if(f == 0) return;
      o = topen("hhh", "w");
      if(o == 0) return;

      last_cday = 0;
      last_cmonth = 1;
      months = 0;
      mask = 0x0001;
      jd = last_jd = 0.0;
      leap = 0;
      while(fgets(out, sizeof out, f) != NULL) {
         strupr(out);
         sscanf(out, "%d%c%d%c%d %d", &yy,&c,&mm,&c,&dd, &cday);
         if(strstr(out, "LUNAR")) {
            if(last_cday) {
               if(last_cmonth == cday) leap = cday;
               last_cmonth = cday;
               if(last_cday == 30) months |= mask;
               mask <<= 1;
            }

            if(strstr(out, "1ST LUNAR")) {
               jd = jdate(yy,mm,dd);
               fprintf(o, "   { 0x%04X, %2d, %.1f },  // %d %d\n", months, leap, last_jd, yy-1+4706-2009, yy-1);
               mask = 0x0001;
               months = 0x0000;
               last_jd = jd;
               leap = 0;
            }
         }
         last_cday = cday;
      }

      fclose(o);
      fclose(f);
   }
#endif // GEN_CHINA_DATA

#ifdef GREET_STUFF

char spring_s[80];      // season greeting strings
char summer_s[80];
char autumn_s[80];
char winter_s[80];
char china_year[80];

#define SPRING       100+0   // greetings that must be calculated
#define SUMMER       100+1
#define AUTUMN       100+2
#define WINTER       100+3
#define TAX_DAY      100+4 
#define ELECTION     100+5 
#define CLOCK_FWD    100+6 
#define CLOCK_BACK   100+7 
#define ASH          100+8 
#define PALM_SUNDAY  100+9 
#define GOOD_FRIDAY  100+10
#define EASTER       100+11
#define GRANNY       100+12
#define ROSH         100+13
#define YOM          100+14
#define PASSOVER     100+15
#define HANUKKAH     100+16
#define PURIM        100+17
#define MARDI_GRAS   100+18
#define AL_HIJRA     100+19
#define ASURA        100+20
#define RAMADAN      100+21
#define EID          100+22
#define CHINA_NY     100+23
#define MAYAN_NY     100+24
#define AZTEC_NY     100+25
#define DOOMSDAY     100+26
#define SUKKOT       100+27
#define SHAVUOT      100+28
#define Q4_TAXES     100+29
#define Q2_TAXES     100+30
#define Q3_TAXES     100+31
#define FINAL_TAXES  100+32
#define UNIX_CLOCK   100+33

#define MAX_HOLIDAYS 366

struct HOLIDAY {
   int nth;
   int day;
   int month;
   char *text;
} holiday[MAX_HOLIDAYS] = {
    { SPRING,       0,  3,  &spring_s[0] },  // the first few holidays are calculated
    { SUMMER,       0,  6,  &summer_s[0] },
    { AUTUMN,       0,  9,  &autumn_s[0] },
    { WINTER,       0, 12,  &winter_s[0] },
    { TAX_DAY,      0,  4,  "It's time to pay your dear Uncle Sam his pound of flesh!" },
    { Q2_TAXES,     0,  6,  "Quarterly taxes are due! Uncle Sam wants all your quarters!" },
    { Q3_TAXES,     0,  9,  "Quarterly taxes are due! Uncle Sam wants all your quarters!" }, 
    { Q4_TAXES,     0,  1,  "Quarterly taxes are due! Uncle Sam wants all your quarters!" }, 
    { FINAL_TAXES,  0, 10,  "Your income tax extension expires today!  Pay up!" }, 
    { ELECTION,     0, 11,  "Vote early, vote often!  It's Throw the Bastards Out Day!" },
    { CLOCK_FWD,    0,  3,  "Set your clocks forward one hour tonight!" },
    { CLOCK_BACK,   0,  3,  "Set your clocks back one hour tonight!" },
    { ASH,          0,  3,  "It's Ash Wedneday..." },
    { PALM_SUNDAY,  0,  3,  "It's Palm Sunday..." },
    { GOOD_FRIDAY,  0,  3,  "It's Good Friday..." },
    { EASTER,       0,  3,  "Happy Easter!" },
    { GRANNY,       0,  9,  "It's Grandparent's Day..." },
    { ROSH,         0,  9,  "Rosh Hashanha starts tonight..." },
    { YOM,          0,  9,  "Yom Kippur starts tonight..." },
    { PASSOVER,     0,  3,  "Passover starts tonight..." },
    { HANUKKAH,     0, 11,  "Chappy Chanukah!" },
    { PURIM,        0,  3,  "Purim starts tonight!" },
    { MARDI_GRAS,   0,  3,  "It's Mardi Gras... time to party hardy!" },
    { AL_HIJRA,     0,  1,  "Happy Islamic New Year!" },
    { ASURA,        0,  1,  "It's Asura!" },
    { RAMADAN,      0,  1,  "Happy Ramadan!" },
    { EID,          0,  1,  "Eid mubarak!" },
    { CHINA_NY,     0,  3,  &china_year[0] },
    { MAYAN_NY,     0,  1,  "Happy Mayan New Year!" },
    { AZTEC_NY,     0,  1,  "Happy Aztec New Year!  Sacrifice your enemies!  Eat their hearts!" },
    { DOOMSDAY,     0,  1,  "The world ends tomorrow...  wear clean underwear" },
    { SUKKOT,       0,  1,  "Sukkot starts tonight..." },
    { SHAVUOT,      0,  1,  "Shavuot starts tonight..." },
    { UNIX_CLOCK,   0,  1,  "Tomorrow is UNIX time doomstime...  wear a clean pocket protector" },

    { 0,  1,  1,  "Happy New Year!" },
    { 3,  1,  1,  "Happy Birthday, Martin!" },
    { 0,  2,  2,  "Happy Groundhog Day!" },
    { 1,  0,  2,  "Hope you didn't bet on the losers..." },
    { 0, 14,  2,  "Happy Valentine's Day!" },
    { 3,  1,  2,  "Greetings Mr. Presidents!  Happy Family Day!" },
    { 0, 18,  2,  "Happy birthday Pluto!  You're still our favorite planet.  The IAU sucks!" },
    { 0, 14,  3,  "Happy pi day!" },
    { 0, 15,  3,  "Beware the Ides of March!" },
    { 0, 17,  3,  "Happy Saint Patrick's Day!" },
    { 0,  1,  4,  "April Fools!" },
    { 1,  0,  4,  "May your Kanamara Matsuri day have a happy ending!" },
    {-1,  5,  4,  "It's Arbor Day... go hug a tree!" },
    { 0, 22,  4,  "Happy Earth Day..." },
    { 0, 25,  4,  "+++ Divide by Cucumber Error. Please Reinstall Universe and Reboot +++" },
    { 0,  4,  5,  "May the fourth be with you!" },
    { 0,  5,  5,  "Happy Cinco de Mayo!" },
    { 0, 25,  5,  "Happy Towel Day!  All you hoopy froods remember to wear your towel!" },
    { 2,  0,  5,  "Happy Mother's Day, Mom!" },
    { 3,  6,  5,  "It's Armed Forces Day!" },
    {-1,  1,  5,  "Happy Memorial Day!" },
    { 1,  1,  6,  "Happy Birthday, Queenie!" },
    { 1,  5,  6,  "It's National Donut Day!  Mmmmmm... forbidden donut!" },
    { 0,  8,  6,  "It's World Oceans Day!" },
    { 0, 14,  6,  "Happy Flag Day!" },
    { 3,  0,  6,  "Happy Father's Day, Dad!" },
    { 0,  1,  7,  "Canada, Oh Canada..." },
    { 0,  4,  7,  "When in the Course of human events..." },
    { 0, 14,  7,  "Happy Bastille Day!" },
    { 0, 15,  8,  "Happy Independence Day, India!" },
    { 1,  1,  9,  "Happy Labor Day!" },
    { 1,  6,  9,  "It's Vulture Awareness Day!" },
    { 0, 11,  9,  "Another day that will live in infamy..." },
    { 0, 17,  9,  "We the People of the United Sates of America..." },
    { 0, 19,  9,  "Yarrr matey... it be talk like a pirate day!" },
    { 0, 26,  9,  "It's Stanislav Petrov Day!  He saved your sorry ass." },
    { 0, 28,  9,  "It's National Drink Beer Day!" },
    { 0,  2, 10,  "Happy Birthday, Mr. Ghandi..." },
    { 0,  9, 10,  "Way to go, Leif!" },
    { 2,  1, 10,  "In 1492, Columbus sailed the ocean blue..." },
    { 3,  4, 10,  "Woohoo! It's New Jersey Credit Union Day!"},
    { 0, 24, 10,  "It's United Nations Day!" },
    { 0, 31, 10,  "Happy Halloween!" },
    { 0,  1, 11,  "Buenos Dias Los Muertos!" },
    { 0, 11, 11,  "Thanks to all the world's veterans..." },
    { 4,  4, 11,  "Happy Thanksgiving!" },
    { 0,  5, 11,  "Remember, remember the Fifth of November!" },
    { 0, 19, 11,  "It's World Toilet Day!" },
    { 0,  7, 12,  "A day that will live in infamy..." },
    { 0, 17, 12,  "Congratulations to the Brothers Wright..." },
    { 0, 24, 12,  "Hey kids,  he's checking his list!" },
    { 0, 25, 12,  "Merry Christmas!" },
    { 0, 26, 12,  "Happy Boxing Day!" },
    { 0, 28, 12,  "Farewell Lars Walineus, you are mourned by all." },
    { 0, 31, 12,  "Stay home tonight,  there are too many crazy people out there..." },
    { 0,-13,  5,  "Today is Friday the 13th...  stay home!" },
    { 0,  0,  0,  "" }
};

char *zodiac[] = {  // Chinese years
   "Monkey",
   "Rooster",
   "Dog",
   "Pig",
   "Mouse",
   "Ox",
   "Tiger",
   "Rabbit",
   "Dragon",
   "Snake",
   "Horse",
   "Goat" 
};

char *jmonths[] = {  // convert jewish month to its ASCII abbreviation
   "Tev", "Ad1", "Adr", "Nis", "Iyr", "Siv", 
   "Tam", "Av ", "Elu", "Tis", "Che", "Kis", 
   "Tev", "Shv"
};
double jtimes[14+1];  // start jdate of each month

char *imonths[] = {  // convert Indian month to its ASCII abbreviation
   "Pau",
   "Mag", "Pha", "Cai", "Vai", "Jya", "Asa",
   "Sra", "Bha", "Asv", "Har", "Pau", "Mag", "Pha"
};
double itimes[13+1];

char *mmonths[] = {  // convert Muslim month to its ASCII abbreviation
   "Muh", "Saf", "Raa", "Rat", "JiU",  "JaT", 
   "Raj", "Sbn", "Ram", "Swl", "DiQ",  "DiH",
   "Muh", "Saf", "Raa", "Rat", "JiU",  "JaT", 
   "Raj", "Sbn", "Ram", "Swl", "DiQ",  "DiH",
   "Muh", "Saf", "Raa", "Rat", "JiU",  "JaT", 
   "Raj", "Sbn", "Ram", "Swl", "DiQ",  "DiH"
};
double mtimes[36+1];
int myears[36+1];

char *pmonths[] = {  // Persian months
   "Far", "Ord", "Kho", "Tir", "Mor", "Sha",
   "Meh", "Aba", "Aza", "Dey", "Bah", "Esf",
   "Far", "Ord", "Kho", "Tir", "Mor", "Sha",
   "Meh", "Aba", "Aza", "Dey", "Bah", "Esf"
   "Far", "Ord", "Kho", "Tir", "Mor", "Sha",
   "Meh", "Aba", "Aza", "Dey", "Bah", "Esf"
};
double ptimes[36+1];
int pyears[36+1];

char *amonths[] = {  // Afghan months
   "Ham", "Saw", "Jaw", "Sar", "Asa", "Sun",
   "Miz", "Aqr", "Qaw", "Jad", "Dal", "Hou",
   "Ham", "Saw", "Jaw", "Sar", "Asa", "Sun",
   "Miz", "Aqr", "Qaw", "Jad", "Dal", "Hou",
   "Ham", "Saw", "Jaw", "Sar", "Asa", "Sun",
   "Miz", "Aqr", "Qaw", "Jad", "Dal", "Hou" 
};

char *kmonths[] = {  // Kurdish months
   "Xak", "Gol", "Joz", "Pos", "Glw", "Xer",
   "Rez", "Glz", "Ser", "Bef", "Reb", "Res",
   "Xak", "Gol", "Joz", "Pos", "Glw", "Xer",
   "Rez", "Glz", "Ser", "Bef", "Reb", "Res",
   "Xak", "Gol", "Joz", "Pos", "Glw", "Xer",
   "Rez", "Glz", "Ser", "Bef", "Reb", "Res" 
};

char *dmonths[] = {  // Druid months
   "Beth    ", "Luis    ", "Nuin    ", "Fearn   ",  "Saille  ", "Huath   ",
   "Duir    ", "Tinne   ", "Coll    ", "Muin    " , "Gort    ", "Ngetal  ",
   "Ruis    ", "Nuh     ", "Beth    ", "Luis    ",  "Nuin    ", "Fearn   "
};
double dtimes[18+1];
int dyears[18+1];

int kin;      // mayan date
int uinal;
int tun;
int katun;
int baktun;
int pictun;

char *tzolkin[] = {  // Mayan Tzolkin months
   "Ahau   ",  "Imix   ",  "Ik     ",  "Akbal  ",  "Kan    ",
   "Cicchan",  "Mimi   ",  "Manik  ",  "Lamat  ",  "Muluc  ",
   "Oc     ",  "Chuen  ",  "Eb     ",  "Ben    ",  "Ix     ",
   "Min    ",  "Cib    ",  "Caban  ",  "Eiznab ",  "Caunac "
};

char *haab[] = {  // Mayan haab months
   "Pop   ", "Uo    ", "Zip   ", "Zotz  ",  "Tzec  ", "Xul   ",
   "Yakin ", "Mol   ", "Chen  ", "Yax   ",  "Zac   ", "Ceh   ",
   "Mac   ", "Kankin", "Muan  ", "Pax   ",  "Kayab ", "Cumku ", 
   "Uayeb "
};

char *aztec[] = {  // Aztec months (Tzolkin)
   "Monster",  "Wind   ", "House  ",  "Lizard ",
   "Snake  ",  "Death  ", "Rabbit ",  "Deer   ",
   "Water  ",  "Dog    ", "Monkey ",  "Grass  ",
   "Reed   ",  "Jaguar ", "Eagle  ",  "Vulture",
   "Quake  ",  "Flint  ", "Rain   ",  "Flower "
};

char *aztec_haab[] = {  // Aztec (haab) months
   "Izcalli ",
   "Cuauhitl",   "Tlacaxip",   "Tozozton",
   "Huey Toz",   "Toxcatl ",   "Etzalcua",
   "Tecuilhu",   "Huey Tec",   "Miccailh",
   "Huey Mic",   "Ochpaniz",   "Teotleco",
   "Tepeilhu",   "Quecholl",   "Panquetz",
   "Atemoztl",   "Tititl  ",   
   "Nemontem" 
};


int set_holiday(int index, int m, int d)
{
int i;

   // search the holiday table for the specially calculated event
   // and set the month and day that it happens on in this year
//printf("set holiday: %d %d %d\n", index, m, d);
   for(i=0; i<MAX_HOLIDAYS; i++) {
      if(holiday[i].nth == index) {
         if(holiday[i].month) {  // do not enable a message that has been disabled
            holiday[i].month = m;
            holiday[i].day = d;
         }
         return i;
      }
   }
   return (-1);
}

int calendar_count()
{
int start;

   // see how many dates are in the greetings list
   start = 0;
   while(start < MAX_HOLIDAYS) {
      if((holiday[start].month == 0) && (holiday[start].day == 0) && (holiday[start].nth == 0)) break;
      ++start;
   }
   return start;
}

void clear_calendar()
{
int start;

   // clear the greetings list
   start = 0;
   while(start < MAX_HOLIDAYS) {
      holiday[start].month = 0;
      holiday[start].day = 0;
      holiday[start].nth = 0;
      ++start;
   }
   calendar_entries = 0;
}


int read_calendar(char *s, int erase)
{
FILE *cal_file;
int i;
int nth, day, month;
char text[128+1];
int len;
char *buf;
int field;
int text_ptr;
int val;
int sign;
int skipping;
char c;

   // Replace default greetings with data from HEATHER.CAL
   cal_file = topen(s, "r");
   if(cal_file == 0) return 1;

   if(erase) clear_calendar();   // clear out the old calendar

   while(fgets(out, sizeof out, cal_file) != NULL) {
      if((out[0] == '*') || (out[0] == ';') || (out[0] == '/') || (out[0] == '#')) {
         if(!strnicmp(out, "#CLEAR", 6)) clear_calendar();
         continue;  // comment line
      }

      field = 0;
      text[0] = 0;
      text_ptr = 0;
      val = 0;
      skipping = 1;
      sign = (+1);

      len = strlen(out);
      for(i=0; i<len; i++) {
         c = out[i];
         if     (c == 0x00) break;
         else if(c == 0x0D) break;
         else if(c == 0x0A) break;
         else if(field < 4) {  // getting a number value
            if     (c == '-') { sign = (-1); skipping=0; }
            else if(c == '+') { sign = (+1);  skipping=0; }
            else if((c >= '0') && (c <= '9')) {
               val = (val*10) + (c-'0');
               skipping = 0;
            }
            else if((c == ' ') || (c == '\t') || (c == ',')) {
               if(skipping) continue;
               ++field;
               if     (field == 1) {
                  nth = val*sign;
               }
               else if(field == 2) day = val*sign;
               else if(field == 3) { month = val*sign; ++field; }
               skipping = 1;
               sign = 1;
               val = 0;
            }
            else {  // invalid char in a number, ignore the line
               break;
            }
         }
         else if(((c == ' ') || (c == '\t')) && skipping) {
            continue;
         }
         else {  // get the text string
            text[text_ptr++] = c;
            text[text_ptr] = 0;
            skipping = 0;
            field = 4;
         }
      }
      if(field != 4) continue;

      buf = (char *) calloc(len+1, 1);
      if(buf == 0) break;

      strcpy(buf, text);
      holiday[calendar_entries].nth = nth;
      holiday[calendar_entries].day = day;
      holiday[calendar_entries].month = month;
      holiday[calendar_entries].text = buf;

      if(++calendar_entries >= MAX_HOLIDAYS) break;
   }

   fclose(cal_file);

   calc_greetings();
   show_greetings();
   return 0;
}

long adjust_mayan(long mayan)
{
   // adjsut mayan date for different correlation factor
   mayan += (MAYAN_CORR-mayan_correlation);  // adjust for new correlation constant
   while(mayan < 0) mayan += (20L*20L*20L*18L*20L);  // make result positive
   return mayan;
}

void get_mayan_date()
{
long mayan;

   gregorian(0, jd_local);
   mayan = (long) (jdate(g_year,g_month,g_day)-jdate(1618,9,18));
   mayan = adjust_mayan(mayan);

   kin = (int) (mayan % 20L);  mayan /= 20L;
   uinal = (int) (mayan % 18L);  mayan /= 18L;
   tun = (int) (mayan % 20L);  mayan /= 20L;
   katun = (int) (mayan % 20L);  mayan /= 20L;
   baktun = (int) ((mayan % 20L)+12L);  mayan /= 20L;  // Baktun 12 started in 1618, 13 started in 2012
   pictun = (int) (mayan % 20L);  mayan /= 20L;
}

void adjust_season(double season)
{
int temp_hours;
int temp_minutes;
int temp_seconds;
int temp_day;
int temp_month;
int temp_year;
double temp_jd_utc;
double temp_jd_gps;

   // adjust season Julian date for local time zone
   temp_hours   = pri_hours;    // save current time
   temp_minutes = pri_minutes;
   temp_seconds = pri_seconds;
   temp_day     = pri_day;
   temp_month   = pri_month;
   temp_year    = pri_year;
   temp_jd_utc  = jd_utc;
   temp_jd_gps  = jd_gps;

   gregorian(0, season);           // convert Julian season value to Gregorian

   jd_utc = jdate(g_year,g_month,g_day) + jtime(g_hours,g_minutes,g_seconds,g_frac);
   jd_gps = jd_utc + jtime(0,0,utc_offset,0.0);
   adjust_tz(20);               // convert UTC season values to local time zone
   strcpy(out, time_zone_set?tz_string:(time_flags & TFLAGS_UTC)?"UTC":"GPS");

   g_year    = pri_year;        // save season time values in the Gregorian variables
   g_month   = pri_month;
   g_day     = pri_day;
   g_hours   = pri_hours;
   g_minutes = pri_minutes;
   g_seconds = pri_seconds;
   g_frac    = pri_frac;

   pri_hours   = temp_hours;    // restore the current time
   pri_minutes = temp_minutes;
   pri_seconds = temp_seconds;
   pri_day     = temp_day;
   pri_month   = temp_month;
   pri_year    = temp_year;
   jd_utc      = temp_jd_utc;
   jd_gps      = temp_jd_gps;
}



#define         SMALL_FLOAT     (1e-12)

double sun_position(double jd)
{
double n,x,e,l,dl,v;
int i;
double ecc;

//  ecc = 0.016718;
    ecc = orbit_eccentricity(jd);
    jd -= 2444238.5;

    n = (360.0/SOLAR_YEAR) * jd;
    i = (int) (n / 360.0);
    n = n - 360.0 * (double) i;
    x = n - 3.762863;
    if(x < 0.0) x += 360.0;
    x *= DEG_TO_RAD;
    e = x;

    i = 0;
    do {  // solve Keppler's equation
        dl = e - (ecc*sin(e)) - x;
        e  = e - (dl / (1.0 - (ecc*cos(e))));
        if(++i > 10) break;
    } while (fabs(dl) >= SMALL_FLOAT);

    v = (360.0 / PI) * atan(1.01686011182*tan(e/2));
    l = v + 282.596403;
    i = (int) (l / 360.0);
    l = l - 360.0 * (double) i;
    return l;
}

double moon_position(double jd, double ls)
{
double ms,l,mm,n,ev,sms,ae,ec;
int i;

    jd -= 2444238.5;

    /* ls = sun_position(jd) */
    ms = 0.985647332099*jd - 3.762863;
    if(ms < 0) ms += 360.0;
    l = 13.176396*jd + 64.975464;
    i = (int) (l / 360.0);
    l = l - (360.0*(double) i);
    if(l < 0) l += 360.0;
    mm = l - 0.1114041*jd - 349.383063;
    i = (int) (mm / 360.0);
    mm -= 360.0 * (double) i;
    n = 151.950429 - 0.0529539*jd;
    i = (int) (n / 360.0);
    n -= 360.0 * (double) i;
    ev = 1.2739 * sin((2*(l-ls)-mm)*DEG_TO_RAD);
    sms = sin(ms*DEG_TO_RAD);
    ae = 0.1858 * sms;
    mm += ev - ae - 0.37*sms;
    ec = 6.2886 * sin(mm*DEG_TO_RAD);
    l += ev + ec - ae + 0.214*sin(2.0*mm*DEG_TO_RAD);
    l= 0.6583 * sin(2*(l-ls)*DEG_TO_RAD)+l;

    return l;
}


/*  PHASE  --  Calculate phase of moon as a fraction:

    The  argument  is  the  time  for  which  the  phase is requested,
    expressed as a Julian date and fraction.  Returns  the  terminator
    phase  angle  as a percentage of a full circle (i.e., 0 to 1), and
    stores into pointer arguments  the  illuminated  fraction  of  the
    Moon's  disc, the Moon's age in days and fraction, the distance of
    the Moon from the centre of the Earth, and  the  angular  diameter
    subtended  by the Moon as seen by an observer at the centre of the
    Earth.
*/

/*  Astronomical constants  */

#define MOON_EPOCH  2444238.5      /* 1980 January 0.0 */

/*  Constants defining the Sun's apparent orbit  */

#define elonge      278.833540     /* Ecliptic longitude of the Sun
                                      at epoch 1980.0 */
#define elongp      282.596403     /* Ecliptic longitude of the Sun at
                                      perigee */
// #define eccent      0.016718       /* Eccentricity of Earth's orbit */
#define sunsmax     1.49598023e8   /* Semi-major axis of Earth's orbit, km */
#define sunangsiz   0.533128       /* Sun's angular size, degrees, at
                                      semi-major axis distance */

/*  Elements of the Moon's orbit, epoch 1980.0  */

#define mmlong      64.975464      /* Moon's mean longitude at the epoch */
#define mmlongp     349.383063     /* Mean longitude of the perigee at the epoch */
#define mlnode      151.950429     /* Mean longitude of the node at the epoch */
#define minc        5.145396       /* Inclination of the Moon's orbit */
#define mecc        0.054900       /* Eccentricity of the Moon's orbit */
#define mangsiz     0.5181         /* Moon's angular size at distance a from Earth */
#define msmax       384748.0       /* Semi-major axis of Moon's orbit in km */
#define mparallax   0.9507         /* Parallax at distance a from Earth */
#define synmonth    29.53058868    /* Synodic month (new Moon to new Moon) */
#define lunatbase   2423436.0      /* Base date for E. W. Brown's numbered
                                      series of lunations (1923 January 16) */

/*  Properties of the Earth  */

#define earthrad    6378.137       /* Radius of Earth in kilometres */


/*  Handy mathematical functions  */

#define sgn(x) (((x) < 0) ? -1 : ((x) > 0 ? 1 : 0))       /* Extract sign */
#define abs(x) ((x) < 0 ? (-(x)) : (x))                   /* Absolute val */
#define fixangle(a) ((a) - 360.0 * (floor((a) / 360.0)))  /* Fix angle    */
#define torad(d) ((d) * (PI / 180.0))                     /* Deg->Rad     */
#define todeg(d) ((d) * (180.0 / PI))                     /* Rad->Deg     */
#define dsin(x) (sin(torad((x))))                         /* Sin from deg */
#define dcos(x) (cos(torad((x))))                         /* Cos from deg */


/*  KEPLER  --   Solve the equation of Kepler.  */

static double kepler(double m, double ecc)
{
int i;
double e, delta;
#define KEP_EPSILON 1E-12

    e = m = torad(m);
    i = 0;
    do {
        delta = e - ecc * sin(e) - m;
        e -= delta / (1.0 - ecc * cos(e));
        if(++i > 10) break;
    } while (abs(delta) > KEP_EPSILON);
    return e;
}

double xmoon_info(double jd)
{
double jd_new, jd_next;

   ++sp_count;

   jd_new = new_moon(jd);
   jd_next = new_moon(jd+31.0);
   MoonSynod = (jd_next-jd_new);
   if(MoonSynod == 0.0) MoonSynod = synmonth;

   MoonAge = ((jd - jd_new) / MoonSynod) * 360.0; // in degrees
   MoonAge = fmod(MoonAge, 360.0);
   MoonPhase = (1.0 - cos(MoonAge*PI/180.0)) / 2.0;

   MoonAge = MoonSynod * (MoonAge / 360.0);  // in days

   lunation = (long) floor(((jd_new + 7.0) - lunatbase) / MoonSynod) + 1;
//sprintf(debug_text, "jd:%f  jd_new:%f  next:%f  synod:%f  age:%f  lunat:%d  phase:%f", 
//jd,jd_new, jd_next, MoonSynod, MoonAge, lunation, MoonPhase);
   return MoonPhase;
}


double moon_info(double jd)
{
double Day, N, M, Ec, Lambdasun, ml, MM, MN, Ev, Ae, A3, MmP,
       mEc, A4, lP, V, lPP, NP, y, x, Lambdamoon, BetaM, F;
double MoonDFrac;
double mage2;
double eccent;


    // calculate moon info based upon John Walker's moontool.c

    /* Calculation of the Sun's position */
    ++sp_count;

    MoonSynod = synmonth;
    eccent = orbit_eccentricity(jd);

    Day = jd - MOON_EPOCH;                  /* Date within epoch */
//  N = fixangle((360.0 / 365.2422) * Day);       /* Mean anomaly of the Sun */
    N = fixangle((360.0 / solar_year(jd)) * Day); /* Mean anomaly of the Sun */
    M = fixangle(N + elonge - elongp);      /* Convert from perigee co-ordinates to epoch 1980.0 */
    Ec = kepler(M, eccent);                 /* Solve equation of Kepler */
    Ec = sqrt((1.0 + eccent) / (1.0 - eccent)) * tan(Ec / 2.0);
    Ec = 2.0 * todeg(atan(Ec));               /* True anomaly */
    Lambdasun = fixangle(Ec + elongp);      /* Sun's geocentric ecliptic longitude */

    /* Orbital distance factor */
    F = ((1 + eccent * cos(torad(Ec))) / (1.0 - eccent * eccent));
    if(F == 0.0) F = 1.0;
    SunDist = sunsmax / F;                  /* Distance to Sun in km */
    SunDisk = F * sunangsiz;                /* Sun's angular size in degrees */

    /* Calculation of the Moon's position */

    /* Moon's mean longitude */
    ml = fixangle(13.1763966 * Day + mmlong);

    /* Moon's mean anomaly */
    MM = fixangle(ml - 0.1114041 * Day - mmlongp);

    /* Moon's ascending node mean longitude */
    MN = fixangle(mlnode - 0.0529539 * Day);

    /* Evection */
    Ev = 1.2739 * sin(torad(2 * (ml - Lambdasun) - MM));

    /* Annual equation */
    Ae = 0.1858 * sin(torad(M));

    /* Correction term */
    A3 = 0.37 * sin(torad(M));

    /* Corrected anomaly */
    MmP = MM + Ev - Ae - A3;

    /* Correction for the equation of the centre */
    mEc = 6.2886 * sin(torad(MmP));

    /* Another correction term */
    A4 = 0.214 * sin(torad(2 * MmP));

    /* Corrected longitude */
    lP = ml + Ev + mEc - Ae + A4;

    /* Variation */
    V = 0.6583 * sin(torad(2 * (lP - Lambdasun)));

    /* True longitude */
    lPP = lP + V;

    /* Corrected longitude of the node */
    NP = MN - 0.16 * sin(torad(M));

    /* Y inclination coordinate */
    y = sin(torad(lPP - NP)) * cos(torad(minc));

    /* X inclination coordinate */
    x = cos(torad(lPP - NP));

    /* Ecliptic longitude */
    Lambdamoon = todeg(atan2(y, x));
    Lambdamoon += NP;

    /* Ecliptic latitude */
    BetaM = todeg(asin(sin(torad(lPP - NP)) * sin(torad(minc))));

    /* Calculation of the phase of the Moon */

    /* Age of the Moon in degrees */
    MoonAge = lPP - Lambdasun;

    /* Phase of the Moon */
    MoonPhase = (1.0 - cos(torad(MoonAge))) / 2.0;

    /* Calculate distance of moon from the centre of the Earth */

    MoonDist = (msmax * (1 - mecc * mecc)) /
               (1.0 + mecc * cos(torad(MmP + mEc)));

    /* Calculate Moon's angular diameter */

    MoonDFrac = MoonDist / msmax;
    if(MoonDFrac == 0.0) MoonDFrac = 1.0;
    MoonDisk = mangsiz / MoonDFrac;

    /* Calculate Moon's parallax */

    MoonPar = mparallax / MoonDFrac;

    mage2 = fixangle(MoonAge) / 360.0;
    MoonAge = synmonth * (fixangle(MoonAge) / 360.0);

    lunation = (long) floor(((new_moon_jd + 7.0) - lunatbase) / synmonth) + 1;

//gregorian(0, new_moon_jd);
//sprintf(plot_title, "new moon: %04d/%02d/%02d  %02d:%02d:%02d", g_year,g_month,g_day, g_hours,g_minutes,g_seconds);

// sprintf(debug_text2, "phasex:%f  sdiam:%f  sdist:%f  synod:%f  luna:%ld", moon_phase(jd), sun_diam(jd), earth_sun_dist(jd), synmonth, lunation);
///sprintf(debug_text, "phase:%f mage:%f %f  mdist:%f mang:%f sdist:%f sang:%f",
///MoonPhase, MoonAge,mage2, MoonDist,MoonDisk, SunDist,SunDisk);
    return MoonPhase;
}

double moon_phase(double jd)
{
double ls;
double lm;
double p;

   //
   //  Calculates the phase of the moon at the given julian date.
   //  returns the moon phase as a real number (0-1)
   //
   ++sp_count;

   ls = sun_position(jd);       // geocentric (?) longitude of sun
   lm = moon_position(jd, ls);  // geocentric (?) longitude of moon

   p = (1.0 - cos((lm-ls)*DEG_TO_RAD))/2.0;
   return p;
}


//
//   Get the days to Dec 31st 0h 2000 - note, this is NOT same
//   as J2000.  h is UT in decimal hours
//   FNday only works between 1901 to 2099 - see Meeus chapter 7
//
// #define FNday (y, m, d, h)  (367*y - 7*(y + (m+9) \ 12) \ 4 + 275 * m \ 9 + d - 730530 + h / 24)

double sign(double x)
{
   if(x == 0.0) return 1.0;
   else if(x > 0.0) return 1.0;
   else return (-1);
}

//
#define FNipart(x)  (sign(x) * (int)(fabs(x)))
//
//   the function below returns an angle in the range
//   0 to two pi
//

double FNrange(double x)
{
double a, b;

    b = x / (PI+PI);
    a = (PI+PI) * (b - FNipart(b));
    if(a < 0) a = (PI+PI) + a;
    return a;
}


//#define FNrange(x)  ((x) - (int)((x) / (PI+PI)) * (PI+PI))

double FNatn2(double y, double x)
{
double a;

    a = atan(y/x);
    if(x < 0.0) a = a + PI;
    if((y < 0.0) && (x > 0.0)) a = a + PI + PI;
    return a;
}


// Declaration of some constants 
#define dEarthMeanRadius   6371.008       // In km
#define dAstronomicalUnit  149597870.691  // In km

#define POLE_RADIUS 6356.752     // km
#define EQU_RADIUS  6378.137     // km
#define SUN_DIAM    1391400.0372 // km
#define MOON_DIAM   1737.1       // km

double moon_lat,moon_lon,moon_dist;

void moon_posn(double jd_tt)
{
//************************************************************************************
//   Moon positions to a few minutes of arc
//   Derived from code by Paul Schlyter at
//       http://hotel04.ausys.se/pausch/comp/ppcomp.html
//
double Nm;
double im;
double wm;
double am;
double ecm;
double Mm;

double Ns;
double isun;
double ws;
double asun;
double ecs;
double Ms;

double Em;
double xv;
double yv;
double vm;
double rm;
double xh;
double yh;
double zh;

double mlon;
double mlat;

double dlon;
double dlat;

double Ls;
double Lm;
double dm;
double F;

double xg;
double yg;
double zg;

double ecl;
double xe;
double ye;
double ze;

double geora, geodec;        // geocentric ra and dec
double ra, dec;              // topocentric ra and dec
double xtop,ytop,ztop, rtop;

double cosha;     // for converting RA/DEC to az/el
double sinha;
double cosd;
double sind;
double cosl;
double sinl;


//
//   the time should be TT/TDT, but UT will do
//

double d;
double h;

double lst;
double ha;
double jd;

   // NOTE: Epoch used here is NOT J2000
   if(0) {
      h = jtime(hours,minutes,seconds,raw_frac);
      h += utc_delta_t();
      d = jd = jdate(year,month,day) + h;
   }
   else {
      d = jd = jd_tt;
      h = (d-0.50) - (int) (d-0.50);
   }

   d = jd - jdate(1999,12,31);
   h *= 24.0;  // convert fractional days to hours


//   moon elements

   Nm = FNrange((125.1228 - .0529538083 * d) * DEG_TO_RAD);
   im = 5.1454 * DEG_TO_RAD;
   wm = FNrange((318.0634 + .1643573223 * d) * DEG_TO_RAD);
   am = 60.2666;  // (Earth radii)
   ecm = .0549;
   Mm = FNrange((115.3654 + 13.0649929509 * d) * DEG_TO_RAD);

//   sun elements
   Ns = 0.0;
   isun = 0.0;
   ws = FNrange((282.9404 + 4.70935E-05 * d) * DEG_TO_RAD);
   asun = 1.0;        //(AU)
   ecs = .016709 - 1.151E-09 * d;
   Ms = FNrange((356.047 + .9856002585 * d) * DEG_TO_RAD);

//   position of Moon
   Em = Mm + ecm * sin(Mm) * (1.0 + ecm * cos(Mm));
   xv = am * (cos(Em) - ecm);
   yv = am * (sqrt(1.0 - ecm * ecm) * sin(Em));
   vm = FNatn2(yv, xv);
   rm = sqrt(xv * xv + yv * yv);
   xh = rm * (cos(Nm) * cos(vm + wm) - sin(Nm) * sin(vm + wm) * cos(im));
   yh = rm * (sin(Nm) * cos(vm + wm) + cos(Nm) * sin(vm + wm) * cos(im));
   zh = rm * (sin(vm + wm) * sin(im));

//   moons geocentric lon and lat
   mlon = FNatn2(yh, xh);
   mlat = FNatn2(zh, sqrt(xh*xh + yh*yh));

//   perturbations:
//   first calculate arguments below, which should be in radians
//   Ms, Mm   Mean Anomaly of the Sun and the Moon
//   Nm       Longitude of the Moon's node
//   ws, wm   Argument of perihelion for the Sun and the Moon
   Ls = Ms + ws;       //Mean Longitude of the Sun  (Ns=0)
   Lm = Mm + wm + Nm;  //Mean longitude of the Moon
   dm = Lm - Ls;       //Mean elongation of the Moon
   F = Lm - Nm;        //Argument of latitude for the Moon

// then add the following terms to the longitude
// note amplitudes are in degrees, convert at end
   dlon = -1.274 * sin(Mm - 2.0 * dm);        //(the Evection)
   dlon = dlon + .658 * sin(2.0 * dm);        //(the Variation)
   dlon = dlon - .186 * sin(Ms);            //(the Yearly Equation)
   dlon = dlon - .059 * sin(2.0 * Mm - 2.0 * dm);
   dlon = dlon - .057 * sin(Mm - 2.0 * dm + Ms);
   dlon = dlon + .053 * sin(Mm + 2.0 * dm);
   dlon = dlon + .046 * sin(2.0 * dm - Ms);
   dlon = dlon + .041 * sin(Mm - Ms);
   dlon = dlon - .035 * sin(dm);            //(the Parallactic Equation)
   dlon = dlon - .031 * sin(Mm + Ms);
   dlon = dlon - .015 * sin(2.0 * F - 2.0 * dm);
   dlon = dlon + .011 * sin(Mm - 4.0 * dm);
   mlon = dlon * DEG_TO_RAD + mlon;

//   latitude terms
   dlat = -.173 * sin(F - 2.0 * dm);
   dlat = dlat - .055 * sin(Mm - F - 2.0 * dm);
   dlat = dlat - .046 * sin(Mm + F - 2.0 * dm);
   dlat = dlat + .033 * sin(F + 2.0 * dm);
   dlat = dlat + .017 * sin(2.0 * Mm + F);
   mlat = dlat * DEG_TO_RAD + mlat;

   moon_phase_acc = (1.0 - cos((mlon*180.0/PI-sun_hlon)*DEG_TO_RAD))/2.0;

//   distance terms earth radii
   rm = rm - .58 * cos(Mm - 2.0 * dm);
   rm = rm - .46 * cos(2.0 * dm);

   moon_lat = fmod(mlat*180.0/PI, 360.0);
   if(moon_lat < 0.0) moon_lat += 360.0;
   moon_lon = fmod(mlon*180.0/PI, 360.0);
   if(moon_lon < 0.0) moon_lon += 360.0;
   moon_dist = rm * earth_radius(lat, alt)*1000.0;
//sprintf(debug_text, "moon lla: %.9f %.9f %.9f", moon_lat, moon_lon, moon_dist);

//   next find the cartesian coordinates
//   of the geocentric lunar position
   xg = rm * cos(mlon) * cos(mlat);
   yg = rm * sin(mlon) * cos(mlat);
   zg = rm * sin(mlat);
//sprintf(debug_text2, "xyzg: %.9f %.9f %.9f", xg*EQU_RADIUS,yg*EQU_RADIUS,zg*EQU_RADIUS);

//   rotate to equatorial coords
//   obliquity of ecliptic of date
// ecl = (23.4393 - 3.563E-07 * d) * DEG_TO_RAD;
   ecl = obliquity(jd, 1);
   xe = xg;
   ye = yg * cos(ecl) - zg * sin(ecl);
   ze = yg * sin(ecl) + zg * cos(ecl);
//sprintf(plot_title, "xyze: %.9f %.9f %.9f", xe*EQU_RADIUS,ye*EQU_RADIUS,ze*EQU_RADIUS);

//   geocentric RA and Dec
   geora = FNatn2(ye,xe) * RAD_TO_DEG / 15.0;
   if(xe || ye) geodec = atan(ze / sqrt(xe*xe + ye*ye)) * RAD_TO_DEG;
   else geodec = 1.0;


   // Calculate local sidereal time.
   // We adjust d by -1.5 days since d's epoch is NOT J2000 and this equation
   // assumes J2000
   lst = 100.46 + (.985647352 * (d-1.50)) + (h * 15.0) + lon*RAD_TO_DEG;
   lst = FNrange(lst*DEG_TO_RAD)*RAD_TO_DEG;
//lst =  lmst(jd_tt, 0)*15.0;
//// while(lst < 0.0) lst += 360.0;
//// while(lst > 360.0) lst -= 360.0;

   
   // calculate the moon's topocentric coords
   xtop = xe - cos(lat) * cos(lst*DEG_TO_RAD);
   ytop = ye - cos(lat) * sin(lst*DEG_TO_RAD);
   ztop = ze - sin(lat);
//sprintf(debug_text, "xyztop: %.9f %.9f %.9f", xtop,ytop,ztop);
   rtop = sqrt(xtop * xtop + ytop * ytop + ztop * ztop);
   ra = FNatn2(ytop, xtop) * RAD_TO_DEG;
   if(xtop || ytop) dec = atan(ztop / sqrt(xtop * xtop + ytop * ytop));
   else dec = 1.0;

   // convert ra and dec to az and el 
   ha = (lst - ra) * DEG_TO_RAD;
   cosha = cos(ha);
   sinha = sin(ha);
   cosd = cos(dec);
   sind = sin(dec);
   cosl = cos(lat);
   sinl = sin(lat);

   moon_az = atan2(-cosd*sinha, cosl*sind - sinl*cosd*cosha) * RAD_TO_DEG;
   if(moon_az < 0.0) moon_az = 360.0 + moon_az;

   moon_el = asin(sinl*sind + cosl*cosd*cosha);
   moon_el *= RAD_TO_DEG;

   set_sat_azel(MOON_PRN, (float) moon_az, (float) moon_el);
}




double sun_peak(double jd, int do_moon)
{
double peak;
#define PEAK_THRESH1 (5.0*PI/180.0)  // prevents search instability near the equator
#define PEAK_THRESH2 (80.0*PI/180.0) // longitude/azimuth gets ambiguous near the poles 

   // used to find the "peak" solar position.  Near the equator and poles this is
   // the actual solar elevation angle.  Otherwise it is where the sun crosses
   // crosses 180.0 degrees azimuth (northern hemisphere) or 0 degrees (southern)

   peak = sun_posn(jd, do_moon);

   // looking for peak in sun elevation
   if(0 && do_moon) return peak;
   else if((lat > (0.0-PEAK_THRESH1)) && (lat < PEAK_THRESH1)) return peak;
   else if(lat > PEAK_THRESH2) return peak;
   else if(lat < (0.0-PEAK_THRESH2)) return peak;
// else return peak;  // always use sun elevation

   // look for sun azimuth nearest 180.0 (or 0.0 below the equator)
   peak = sun_az;
   if(lat < 0.0) peak = fmod(peak+180.0, 360.0);
   if(peak > 180.0) return 360.0-peak;
   else return peak;
}


double interp_solar_noon(double start, double end, int do_moon)
{
double start_el, mid_el, end_el;
double mid;
double pct, delta;

// return (start+end) / 2.0;

   if((lat > (0.0-PEAK_THRESH1)) && (lat < PEAK_THRESH1)) return (start+end)/2.0;
   else if(lat > PEAK_THRESH2) return (start+end)/2.0; 
   else if(lat < (0.0-PEAK_THRESH2)) return (start+end)/2.0; 

   start_el = sun_peak(start, do_moon);
   end_el = 360.0 - sun_peak(end, do_moon);
   delta = end_el - start_el;
   if(delta == 0.0) return (start+end) / 2.0;

   pct = (180.0 - start_el) / delta;
   mid = start + (end-start) * pct;


if(0 && debug_file) {
   mid_el = sun_peak(mid, do_moon);
   fprintf(debug_file, "\ninterp noon: start:%f mid:%f end:%f\n", start,mid,end);
   fprintf(debug_file, "start_el:%f mid_el:%f end_el:%f  pct:%f\n", start_el,mid_el,end_el, pct);
   gregorian(0, mid+time_zone());
   fprintf(debug_file, "noon (utc): %02d:%02d:%02d %f  %04d/%02d/%02d  tz:%f\n\n", g_hours,g_minutes,g_seconds,g_frac, g_year,g_month,g_day, time_zone());
}
   return mid;
}


void refine_solar_noon(int do_moon)
{
int i;
double start, mid, end;
double start_step, end_step;
double start_el,mid_el,end_el;
#define X_SUN_SEARCH (do_moon?4000:600)

   // search ten minutes on each side of estimated solar noon for the peak elevation
   // if peak not found, use the estimated value

   start = solar_noon - jtime(0,0,X_SUN_SEARCH,0.0);
   mid = solar_noon;
   end = solar_noon + jtime(0,0,X_SUN_SEARCH,0.0);

   start_el = sun_peak(start, do_moon);
   mid_el = sun_peak(mid, do_moon);
   end_el = sun_peak(end, do_moon);

   start_step = X_SUN_SEARCH / 2.0;
   end_step = X_SUN_SEARCH / 2.0;

if(0 && debug_file) fprintf(debug_file, "refine noon %d:%6.2f  ", sp_count, (end-start)*(24.0*60.0*60.0));
   for(i=0; i<40; i++) {
      if((start_el < mid_el) && (mid_el < end_el)) {    // sun is rising at end time, move end forward
if(0 && debug_file) fprintf(debug_file, "  fix end:   start:%f end:%f start_step:%f, end_step:%f\n", mid-start,end-mid,start_step,end_step);
         end += end_step / 2.0;
         end_el = sun_peak(end, do_moon);
      }
      else if((start_el > mid_el) && (mid_el > end_el)) {  // sun is setting at start time, back uo start
if(0 && debug_file) fprintf(debug_file, "  fix start: start:%f end:%f start_step:%f, end_step:%f\n", mid-start,end-mid,start_step,end_step);
         start -= start_step / 2.0;
         start_el = sun_peak(start, do_moon);
      }
      else {  // noon is somewhere between start and end, narrow the range
         start_step = (mid - start) / 1.25;
         end_step = (end - mid) / 1.25;
if(0 && debug_file) fprintf(debug_file, "  fix both:  start:%f end:%f start_step:%f, end_step:%f\n", mid-start,end-mid,start_step,end_step);
         start += start_step;
         end -= end_step;
         start_el = sun_peak(start, do_moon);
         end_el = sun_peak(end, do_moon);
      }
if(0 && debug_file) fprintf(debug_file, "refine noon %d :%6.2f  ", sp_count, (end-start)*(24.0*60.0*60.0));

      if(fabs(start-end) < (1.0/(24.0*60.0*60.0))) {  // time lies with a 1 second interval
         solar_noon = (start + end) / 2.0;
if(0 && debug_file) fprintf(debug_file, "\nnoon %d: %.9f", sp_count, solar_noon);
         break;  // start end end are within a second
      }

      mid = (start + end) / 2.0;
      mid_el = sun_peak(mid, do_moon);
   }

   solar_noon = interp_solar_noon(start, end, do_moon);
if(0 && debug_file) fprintf(debug_file, "\nnoon %d: %.9f", sp_count, solar_noon);

if(0 && debug_file) {
   gregorian(0, solar_noon+time_zone());   
   sun_el = sun_posn(solar_noon, do_moon);
   if(do_moon) fprintf(debug_file, "\ntransit az:%f el:%f  (utc): %02d:%02d:%02d %g   tz:%f\n", sun_el,sun_az, g_hours,g_minutes,g_seconds,g_frac, time_zone());
   else fprintf(debug_file, "\nnoon az:%f el:%f  (utc): %02d:%02d:%02d %g  tz:%f\n", sun_el,sun_az, g_hours,g_minutes,g_seconds,g_frac, time_zone());
}
}


double interp_sun_posn(double start, double end, double zenith, int do_moon)
{
double start_el, mid_el, end_el;
double mid;
double pct, delta;

//   return (start+end) / 2.0;

   start_el = sun_posn(start, do_moon);
   end_el = sun_posn(end, do_moon);
   delta = (end_el - start_el);
   if(delta == 0.0) return ((start+end) / 2.0);

   pct = (zenith - start_el) / delta;
   mid = start + (end-start) * pct;

if(0 && debug_file) {
   mid_el = sun_posn(mid, do_moon);
   fprintf(debug_file, "interp: start:%f mid:%f end:%f\n", start,mid,end);
   fprintf(debug_file, "start_el:%f mid_el:%f end_el:%f  pct:%f\n", start_el,mid_el,end_el, pct);
   gregorian(0, mid+time_zone());
   fprintf(debug_file, "time (utc): %02d:%02d:%02d %f  %04d/%02d/%02d  tz:%f\n\n", g_hours,g_minutes,g_seconds,g_frac, g_year,g_month,g_day, time_zone());
}
   return mid;
}


void log_sun_times()
{
   if(0 && debug_file) {
      if(do_moonrise) fprintf(debug_file, "\n\nMoon info for %04d/%02d/%02d\n", pri_year,pri_month,pri_day);
      else            fprintf(debug_file, "\n\nSun info for %04d/%02d/%02d\n", pri_year,pri_month,pri_day);
//    fprintf(debug_file, "day-1: jd:%.9f  r:%.9f t:%.9f s:%.9f\n", jd0, r0,n0,s0);   
//    fprintf(debug_file, "day+0: jd:%.9f  r:%.9f t:%.9f s:%.9f\n", jd1, r1,n1,s1);   
//    fprintf(debug_file, "day+1: jd:%.9f  r:%.9f t:%.9f s:%.9f\n", jd2, r2,n2,s2);   

      gregorian(0, sunrise_time+time_zone());
      fprintf(debug_file, "rise:    %02d:%02d:%02d %.3f", g_hours,g_minutes,g_seconds,g_frac);
      if(have_sun_times & 0x01) fprintf(debug_file, "\n");
      else                      fprintf(debug_file, " - bad\n");

      gregorian(0, solar_noon+time_zone());
      fprintf(debug_file, "transit: %02d:%02d:%02d %.3f", g_hours,g_minutes,g_seconds,g_frac);
      if(have_sun_times & 0x02) fprintf(debug_file, "\n");
      else                      fprintf(debug_file, " - bad\n");

      gregorian(0, sunset_time+time_zone());
      fprintf(debug_file, "set:     %02d:%02d:%02d %.3f", g_hours,g_minutes,g_seconds,g_frac);
      if(have_sun_times & 0x04) fprintf(debug_file, "\n");
      else                      fprintf(debug_file, " - bad\n");

      fprintf(debug_file, "\n");
   }
}


int calc_moonrise()
{
double old_rise_horizon;
double old_set_horizon;
double r0,n0,s0;
double r1,n1,s1;
double r2,n2,s2;
int f0,f1,f2;
double jd0,jd1,jd2;
double old_az, old_el;

   old_az = moon_az;
   old_el = moon_el;

   old_rise_horizon = sunrise_horizon;
   old_set_horizon = sunset_horizon;
   sunrise_horizon = sunset_horizon = (-50.0/60.0);

   gregorian(0, jd_local);
   jd1 = jdate(g_year,g_month,g_day) - time_zone();   // jd1 is start (midnight) of the local day
// jd1 = jdate(pri_year,pri_month,pri_day) - time_zone();
   jd0 = jd1 - 1.0;
   jd2 = jd1 + 1.0;

   f0 = calc_sunrise(-1.0, 1);   // get info for previous day
   r0 = sunrise_time;
   n0 = solar_noon;
   s0 = sunset_time;

   f2 = calc_sunrise(1.0, 2);    // get info for next day
   r2 = sunrise_time;
   n2 = solar_noon;
   s2 = sunset_time;

   f1 = calc_sunrise(0.0, 3);    // get info for today
   r1 = sunrise_time;
   n1 = solar_noon;
   s1 = sunset_time;

   have_sun_times = 0x07;       // assume all is well

   if(r1 < jd1)  {  // usually an ok condition
if(debug_file) fprintf(debug_file, "# rise prev day!\n");
      sunrise_time = r2;
   }
   if(r1 >= jd2) {
if(debug_file)      fprintf(debug_file, "#rise next day!\n");
      sunrise_time = r0;
      have_sun_times &= (~0x01);
   }

   if(n1 < jd1) {
if(debug_file)      fprintf(debug_file, "#noon prev day!\n");
      solar_noon = n2;
      have_sun_times &= (~0x02);
   }
   if(n1 >= jd2) {
if(debug_file)      fprintf(debug_file, "#noon next day!\n");
      solar_noon = n0;
      have_sun_times &= (~0x02);
   }

   if(s1 < jd1)  {
if(debug_file)      fprintf(debug_file, "#set prev day!\n");
      sunset_time = s2;
      have_sun_times &= (~0x04);
   }
   if(s1 >= jd2) {    // usually an ok condition
if(debug_file)      fprintf(debug_file, "#set next day!\n");
      sunset_time = s0;
   }

   gregorian(0, sunrise_time+time_zone()+jtime(0,0,0,0.5));
   rise_hh = g_hours;
   rise_mm = g_minutes;
   rise_ss = g_seconds;
   rise_frac = g_frac;

   gregorian(0, solar_noon+time_zone()+jtime(0,0,0,0.5)); 
   noon_hh = g_hours;
   noon_mm = g_minutes;
   noon_ss = g_seconds;
   noon_frac = g_frac;

   gregorian(0, sunset_time+time_zone()+jtime(0,0,0,0.5)); 
   set_hh = g_hours;
   set_mm = g_minutes;
   set_ss = g_seconds;
   set_frac = g_frac;

   log_sun_times();

   sunrise_horizon = old_rise_horizon;
   sunset_horizon = old_set_horizon;

   moon_az = old_az;
   moon_el = old_el;

   return have_sun_times;
}


int calc_sunrise(double delta, int why)
{
double zenith;
int hh;
double jd, jd0;
double start, mid, end;
double start_el, mid_el, end_el;
double old_az, old_el;
double old_maz, old_mel;
#define SUN_RES 5 //was 14 - 5 approximation steps gets us within 2 minutes, then we interpolate

   // A more accurate sunrise / sunset calculator than the old one...
   //
   // Sunrise and sunset times are calculated by finding the hours that the
   // sun elevation crosses the desired horizon angles.  Then a 
   // successive-approximation binary seach of those hours is used to
   // refine the times to 2 second levels. The we interpolate to get to 100 msec.
   //
   // Solar noon is first appoximated as midway between sunrise and sunset.
   // Then the time interval +/- 600 seconds from that point is checked for
   // being the peak solar elevation.  If no peak is found the mid-point value
   // between sunrise and sunset is used.

sp_count = 0;
if(debug_file) fprintf(debug_file, "#calc_sunrise(%d)\n", why);

   old_az = sun_az;
   old_el = sun_el;
   old_maz = moon_az;
   old_mel = moon_el;

   have_sun_times = 0x00;
   rise_hh = rise_mm = rise_ss = 0;         
   rise_frac = 0.0;
   set_hh = set_mm =  set_ss = 0;
   set_frac = 0.0;
   noon_hh = noon_mm = noon_ss = 0;         
   noon_frac = 0.0;

   if((sunrise_type == 0) || (lat == 0.0) || (lon == 0.0)) {  // assume GPS not tracking yet
      goto no_sun;
   }

   zenith = sunrise_horizon;

   gregorian(0, jd_local);
   jd0 = start = jdate(g_year,g_month,g_day) + delta;

   jd = jd0; // + jtime(5,0,0,0.0);  // start sunrise search at 5:00 in the morning local time
   start_el = sun_posn(jd, do_moonrise);

   for(hh=1; hh<25; hh++) {  // find the hour of the rising zenith crossing
      end = jd + jtime((hh%25),0,0,0.0);
      end_el = sun_posn(end, do_moonrise);
if(0 && debug_file) fprintf(debug_file, "rise hr:%d  spc:%d  s_el:%f  e_el:%f\n", hh, sp_count, start_el,end_el);
      if((start_el <= zenith) && (end_el >= zenith)) {
         goto got_rise_hh;
      }
      start_el = end_el;
      start = end;
   }
   start = jd0;
   goto no_rise;

   got_rise_hh:
   for(hh=0; hh<SUN_RES; hh++) {  // do a binary search for the rising zenith crossing
      mid = (start + end) / 2.0;
      mid_el = sun_posn(mid, do_moonrise);
      if(mid_el <= zenith) start = mid;
      else if(mid_el >= zenith) end = mid;
if(0 && debug_file) fprintf(debug_file, "refine rise %d:  spc:%d  rise_el:%f  diff:%f\n", hh, sp_count, mid_el, ((end-start) * 24.0*60.0*60.0));
      if(((end-start) * 24.0*60.0*60.0) < 1.0) break;
   }
   sunrise_time = interp_sun_posn(start,end, zenith, do_moonrise);
   sunrise_el = sun_posn(sunrise_time, do_moonrise);
   sunrise_az = sun_az;

   gregorian(0, sunrise_time + time_zone() + jtime(0,0,0,0.50));
   rise_hh = g_hours;
   rise_mm = g_minutes;
   rise_ss = g_seconds;
   rise_frac = g_frac;
   have_sun_times |= 0x01;  // we have a sun rise

   // now calculate the sunset time
   if(0 && do_moonrise) start = jd0; // moon can set before it rises!
   else start = sunrise_time;   // jd0;

   no_rise:
   zenith = sunset_horizon;
   start_el = sun_posn(start, do_moonrise);
   jd = start;

   for(hh=1; hh<25; hh++) {  // find the hour of the setting zenith crossing
      end = jd + jtime(hh%25,0,0,0.0);
      end_el = sun_posn(end, do_moonrise);
      if((start_el >= zenith) && (end_el <= zenith)) {
         goto got_set_hh;
      }
if(0 && debug_file) fprintf(debug_file, "set hr:%d  spc:%d  s_el:%f  e_el:%f\n", hh, sp_count, start_el,end_el);
      start_el = end_el;
      start = end;
   }
   goto no_sun;   // sun never sets, we cant calculate a sun rise

   got_set_hh:
   for(hh=0; hh<SUN_RES; hh++) {  // do a binary search for the setting zenith crossing
      mid = (start + end) / 2.0;
      mid_el = sun_posn(mid, do_moonrise);
      if(mid_el >= zenith) start = mid;
      else if(mid_el <= zenith) end = mid;
if(0 && debug_file) fprintf(debug_file, "refine set %d:  spc:%d  set_el:%f  diff:%f\n", hh, sp_count, mid_el, ((end-start) * 24.0*60.0*60.0));
      if(((end-start) * 24.0*60.0*60.0) < 1.0) break;
   }
   sunset_time = interp_sun_posn(start,end, zenith, do_moonrise);
   sunset_el = sun_posn(sunset_time, do_moonrise);
   sunset_az = sun_az;

   gregorian(0, sunset_time + time_zone() + jtime(0,0,0,0.50));
   set_hh = g_hours;
   set_mm = g_minutes;
   set_ss = g_seconds;
   set_frac = g_frac;
   have_sun_times |= 0x04;

   solar_noon = (sunrise_time + sunset_time) / 2.0;
   if(sunset_time < sunrise_time) {
      solar_noon += 0.50;
   }
   refine_solar_noon(do_moonrise); // a better succesive-approximation refinement over a longer interval

   noon_el = sun_posn(solar_noon, do_moonrise);
   noon_az = sun_az;

   gregorian(0, solar_noon + time_zone() + jtime(0,0,0,0.50));
   noon_hh = g_hours;
   noon_mm = g_minutes;
   noon_ss = g_seconds;
   noon_frac = g_frac;
   if((have_sun_times & 0x01) && (have_sun_times & 0x04)) {  // we have a rise and set
      have_sun_times |= 0x02;
   }

   if(STATUS_RCVR) { // make sure we can trigger alarms during status message
      if((rise_ss >= SCPI_STATUS_SECOND) && (rise_ss <= SCPI_STATUS_SECOND+5)) {
         rise_ss = SCPI_STATUS_SECOND-1;
      }
      if((set_ss >= SCPI_STATUS_SECOND) && (set_ss <= SCPI_STATUS_SECOND+5)) {
         set_ss = SCPI_STATUS_SECOND-1;
      }
   }

   no_sun:
   if(0 && !have_local_time) have_sun_times = 0;  // we don't need to do this anymore since we use jd_local to calc sunrise/sunset
   if(!do_moonrise) log_sun_times();

   sun_az = old_az;
   sun_el = old_el;
   moon_az = old_maz;
   moon_el = old_mel;
   return have_sun_times;
}



double earth_radius(double lat, double alt)
{
double r1,r2,r3,r4;
double r;

   // return earth radius in km at latitude, alt (in meters)

   r1 = (EQU_RADIUS*EQU_RADIUS) * cos(lat);
   r2 = (POLE_RADIUS*POLE_RADIUS) * sin(lat);
   r3 = EQU_RADIUS * cos(lat);
   r4 = POLE_RADIUS * sin(lat);

   r = (r3*r3) + (r4*r4);
   if(r == 0.0) r = dEarthMeanRadius;
   else {
      r = ((r1*r1) + (r2*r2)) / r;
      if(r > 0.0) r = sqrt(r);
      else r = dEarthMeanRadius;
   }

   return r + (alt / 1000.0);
}

double earth_sun_dist(double jd)
{
double d;
double e;
double theta;

   // in km

   e = orbit_eccentricity(jd);
   theta = ((PI*2.0)/365.2563630) * time_since_perihelion(jd);

   d = (1.0-e*e) / (1.0 + (e*cos(theta)));

   d = d*dAstronomicalUnit;
   return d;
}


double sun_refraction(double el)
{
double r;
double htan;
double r2;

   // calculation sun refraction angle (in degrees) given its 
   // elevation "el" in degrees

   r = r2 = (-1.0);
   if(0) {
      htan = el + 5.11;
      if(htan == 0.0) return (-1.0);
      htan = el + (10.3 / htan);
      htan = tan(htan*PI/180.0);
      if(htan == 0.0) return (-1.0);
      r2 = 1.02 / htan; // refraction in minutes
      r2 /= 60.0;       // refraction in degrees
      r = r2;
   }
   else {
      if(el >= 85.0) r = 0.0;
      else if(el >= 5.0) {
         htan = tan(el*PI/180.0);
         if(htan == 0.0) return (-1.0);
         r = (58.1 / htan) - (0.07 / (htan*htan*htan)) + (0.000086 / (htan*htan*htan*htan*htan));
      }
      else if(el >= (-0.575)) {
         r = 1735.0;
         r -= (518.2*el);
         r += (103.4*el*el);
         r -= (12.79*el*el*el);
         r += (0.711*el*el*el*el);
      }
      else {
         htan = tan(el*PI/180.0);
         if(htan == 0.0) return (-1.0);
         r = (-20.774/htan);
      }
      r /= 3600.0;
   }
   return r;
}


double sun_diam(double jd)
{
double diam;

   // return apparent topocentric sun disk diameter in degrees.
   // earth radius correction only valid for daylight hours

   diam = 2.0 * asin(SUN_DIAM / ((earth_sun_dist(jd)-earth_radius(lat,alt)) * 2.0)) * 180.0 / PI;
   return diam;
}


double moon_diam(double jd)
{
double diam;

   // return apparent topocentric moon disk diameter in degrees

   if(MoonDisk) diam = MoonDisk;
   else         diam = (31.7167/60.0);

   return diam;
}



double sun_posn(double jd, int moon_flag)
{
double RightAscension;
double Declination;
double HourAngle;
double Zenith;
double Azimuth;
double t, te, wte, s1, c1, s2, c2, s3, c3;
double sp, cp, sd, cd, sH, cH, se0, ep, De, lambda, epsi;
double sl, cl, se, ce, L, nu, Dlam;
double Temperature = 15.0;  // degrees C
double Pressure = 1.0;      // atmospheres
double tt;

   // Calculate sun position using Grena's Algorithm 5
   // Very accurate results (<0.0027 degrees) for years 2010 to 2110.
   // jd is jd_utc.

++sp_count;

   if(moon_flag) {       // royal kludge to calculate moon rise/transit/set
      moon_posn(jd);     // ... this kludge lets us use the calc_sunrise()
      sun_az = moon_az;  // ... code to do the moon, but the results are
      sun_el = moon_el;  // ... saved in the sun related variables !!!!!
      have_sun_el = 0;
      return moon_el;
   }

   
   tt = utc_delta_t();

   t = jd - jdate(2060,1,1);    // year 2060 is middle of algorithm validity range
   te = t + (tt / (24.0*60.0*60.0));  // convert to terresrial time

   wte = 0.0172019715*te;

   s1 = sin(wte);
   c1 = cos(wte);
   s2 = 2.0*s1*c1;
   c2 = (c1+s1)*(c1-s1);
   s3 = s2*c1 + c2*s1;
   c3 = c2*c1 - s2*s1;

   // Heliocentric longitude
   L = 1.7527901 + 1.7202792159e-2*te + 3.33024e-2*s1 - 2.0582e-3*c1 
     + 3.512e-4*s2 - 4.07e-5*c2 + 5.2e-6*s3 - 9e-7*c3 
     - 8.23e-5*s1*sin(2.92e-5*te) + 1.27e-5*sin(1.49e-3*te - 2.337)
     + 1.21e-5*sin(4.31e-3*te + 3.065) + 2.33e-5*sin(1.076e-2*te - 1.533)
     + 3.49e-5*sin(1.575e-2*te - 2.358) + 2.67e-5*sin(2.152e-2*te + 0.074)
     + 1.28e-5*sin(3.152e-2*te + 1.547) + 3.14e-5*sin(2.1277e-1*te - 0.488);

   sun_hlon = (L*180.0/PI);                     // used as possible alternate input to gravity tide sun position code
   sun_hlon = fmod(sun_hlon, 360.0) + 180.0;
   sun_hlon = fmod(sun_hlon, 360.0);
//sprintf(debug_text2, "hlon: %.9f  r:%.9f", sun_hlon, (earth_sun_dist(jd)+EQU_RADIUS*0.0)*1000.0);

   nu = 9.282e-4*te - 0.8;  // nutation
   Dlam = 8.34e-5*sin(nu);
   lambda = L + PI + Dlam;

   epsi = 4.089567e-1 - 6.19e-9*te + 4.46e-5*cos(nu);

   sl = sin(lambda);
   cl = cos(lambda);
   se = sin(epsi);
   ce = sqrt(1.0-se*se);

   RightAscension = atan2(sl*ce, cl);
   if(RightAscension < 0.0) RightAscension += (PI*2.0);
   sun_ra = RightAscension * (180.0 / PI);

   Declination = asin(sl*se);
   sun_decl = Declination * (180.0 / PI);

   HourAngle = 1.7528311 + 6.300388099*t + lon - RightAscension + 0.92*Dlam;
   HourAngle = fmod(HourAngle + PI, (PI*2.0)) - PI;
   if(HourAngle < (-PI)) HourAngle += (PI*2.0);

   sp = sin(lat);
   cp = sqrt((1.0-sp*sp));
   sd = sin(Declination);
   cd = sqrt(1.0-sd*sd);
   sH = sin(HourAngle);
   cH = cos(HourAngle);
   se0 = sp*sd + cp*cd*cH;
   ep = asin(se0) - 4.26e-5*sqrt(1.0-se0*se0);
   if(cd) Azimuth = atan2(sH, cH*sp - sd*cp/cd);
   else   Azimuth = atan2(sH, cH*sp);

   De = 0.0;
   if(apply_refraction && (ep > 0.0)) {  // apply refraction correction
      De = (0.08422*Pressure) / ((273.0+Temperature)*tan(ep + 0.003138/(ep + 0.08919)));
   }

   // convert results to our desired orientation in degrees
   sun_az = (Azimuth * (180.0/PI)) + 180.0;
   if(sun_az < 0.0) sun_az += 360.0;
   sun_az = fmod(sun_az, 360.0);

   Zenith = (PI/2.0) - ep - De;
   sun_el = 90.0 - ((Zenith * (180.0/PI)));

   return sun_el;
}  


#define MOON_PHASE(x) moon_phase(x)   // could use moon_phase() moon_info() or moon_age()

double moon_age(double x)
{
   if(new_moon_jd && (x >= new_moon_jd)) return (x - new_moon_jd);

   moon_info(x);
   return MoonAge;
}


double new_moon_table[] = {  // table of jd's of new moons from 2016..2100
   // year 2016
   2457397.563194444, 2457427.110416667, 2457456.579166667, 2457485.975000000, 
   2457515.312500000, 2457544.625000000, 2457573.959027778, 2457603.364583334, 
   2457632.877083333, 2457662.508333333, 2457692.234722222, 2457722.012500000, 
   2457751.786805556, 
   // year 2017
   2457781.504861111, 2457811.123611111, 2457840.622916667, 2457870.011111111, 
   2457899.322222222, 2457928.604861111, 2457957.906944444, 2457987.270833334, 
   2458016.729166667, 2458046.300000000, 2458075.987500000, 2458105.771527778, 
   // year 2018
   2458135.595138889, 2458165.378472222, 2458195.050000000, 2458224.581250000, 
   2458253.991666667, 2458283.321527778, 2458312.616666667, 2458341.915277778, 
   2458371.250694444, 2458400.657638889, 2458430.168055556, 2458459.805555556, 
   // year 2019
   2458489.561111111, 2458519.377777778, 2458549.169444445, 2458578.868055556, 
   2458608.447916667, 2458637.918055556, 2458667.302777778, 2458696.633333333, 
   2458725.942361111, 2458755.268055555, 2458784.651388889, 2458814.129166667, 
   2458843.717361111, 
   // year 2020
   2458873.404166667, 2458903.147222222, 2458932.894444444, 2458962.601388889, 
   2458992.235416667, 2459021.778472222, 2459051.231250000, 2459080.612500000, 
   2459109.958333334, 2459139.313194444, 2459168.713194444, 2459198.178472222, 
   // year 2021
   2459227.708333334, 2459257.295833333, 2459286.931250000, 2459316.604861111, 
   2459346.291666667, 2459375.953472222, 2459405.553472222, 2459435.076388889, 
   2459464.536111111, 2459493.961805556, 2459523.385416667, 2459552.821527778, 
   // year 2022
   2459582.273611111, 2459611.740277778, 2459641.232638889, 2459670.766666667, 
   2459700.352777778, 2459729.979166667, 2459759.619444444, 2459789.246527778, 
   2459818.845138889, 2459848.412500000, 2459877.950694445, 2459907.456250000, 
   2459936.928472222, 
   // year 2023
   2459966.370138889, 2459995.795833333, 2460025.224305556, 2460054.675000000, 
   2460084.161805556, 2460113.692361111, 2460143.272222222, 2460172.901388889, 
   2460202.569444445, 2460232.246527778, 2460261.893750000, 2460291.480555556, 
   // year 2024
   2460320.997916667, 2460350.457638889, 2460379.875000000, 2460409.264583333, 
   2460438.640277778, 2460468.026388889, 2460497.456250000, 2460526.967361111, 
   2460556.580555555, 2460586.284027778, 2460616.032638889, 2460645.765277778, 
   2460675.435416667, 
   // year 2025
   2460705.025000000, 2460734.531250000, 2460763.956944444, 2460793.313194444, 
   2460822.626388889, 2460851.938194444, 2460881.299305555, 2460910.754166667, 
   2460940.329166667, 2460970.017361111, 2460999.782638889, 2461029.571527778, 
   // year 2026
   2461059.327777778, 2461089.000694444, 2461118.557638889, 2461147.994444444, 
   2461177.334027778, 2461206.620833333, 2461235.905555556, 2461265.234027778, 
   2461294.643750000, 2461324.159722222, 2461353.793055556, 2461383.536111111, 
   // year 2027
   2461413.350000000, 2461443.163888889, 2461472.895138889, 2461502.493750000, 
   2461531.957638889, 2461561.319444445, 2461590.626388889, 2461619.920138889, 
   2461649.236805555, 2461678.608333333, 2461708.066666667, 2461737.641666667, 
   2461767.341666667, 
   // year 2028
   2461797.134027778, 2461826.942361111, 2461856.688194444, 2461886.324305556, 
   2461915.844444444, 2461945.269444444, 2461974.626388889, 2462003.947222222, 
   2462033.266666667, 2462062.622916667, 2462092.054166667, 2462121.587500000, 
   // year 2029
   2462151.225000000, 2462180.938194444, 2462210.679861111, 2462240.402777778, 
   2462270.070833333, 2462299.660416667, 2462329.160416667, 2462358.580555555, 
   2462387.947222222, 2462417.301388889, 2462446.683333333, 2462476.119444444, 
   // year 2030
   2462505.617361111, 2462535.172222222, 2462564.774305556, 2462594.418055556, 
   2462624.091666667, 2462653.764583333, 2462683.398611111, 2462712.965972222, 
   2462742.463194444, 2462771.913194445, 2462801.345138889, 2462830.781944444, 
   2462860.230555556, 
   // year 2031
   2462889.688194444, 2462919.159027778, 2462948.659027778, 2462978.206250000, 
   2463007.803472222, 2463037.434027778, 2463067.069444445, 2463096.688888889, 
   2463126.282638889, 2463155.847916667, 2463185.381944445, 2463214.879166667, 
   // year 2032
   2463244.338194444, 2463273.766666667, 2463303.184027778, 2463332.610416667, 
   2463362.066666667, 2463391.563888889, 2463421.112500000, 2463450.716666667, 
   2463480.372916667, 2463510.060416667, 2463539.739583334, 2463569.370138889, 
   // year 2033
   2463598.928472222, 2463628.416666667, 2463657.849305556, 2463687.244444444, 
   2463716.615277778, 2463745.983333333, 2463775.379861111, 2463804.842361111, 
   2463834.402777778, 2463864.069444445, 2463893.811805556, 2463923.568750000, 
   2463953.282638889, 
   // year 2034
   2463982.918055556, 2464012.465277778, 2464041.926388889, 2464071.309722222, 
   2464100.634027778, 2464129.934722222, 2464159.260416667, 2464188.661805556, 
   2464218.176388889, 2464247.814583333, 2464277.552777778, 2464307.343055556, 
   // year 2035
   2464337.127083333, 2464366.848611111, 2464396.464583333, 2464425.956944444, 
   2464455.336111111, 2464484.639583333, 2464513.915972222, 2464543.216666667, 
   2464572.582638889, 2464602.046527778, 2464631.624305556, 2464661.318055556, 
   2464691.104861111, 
   // year 2036
   2464720.928472222, 2464750.707638889, 2464780.372916667, 2464809.897916667, 
   2464839.303472222, 2464868.631944445, 2464897.928472222, 2464927.232638889, 
   2464956.577083333, 2464985.993055556, 2465015.509722222, 2465045.148611111, 
   // year 2037
   2465074.898611111, 2465104.704166667, 2465134.483333333, 2465164.172222222, 
   2465193.745833333, 2465223.215277778, 2465252.605555556, 2465281.945138889, 
   2465311.267361111, 2465340.606944445, 2465370.002083333, 2465399.484722222, 
   // year 2038
   2465429.070138889, 2465458.744444444, 2465488.468750000, 2465518.196527778, 
   2465547.888888889, 2465577.516666667, 2465607.063888889, 2465636.527777778, 
   2465665.925694444, 2465695.289583333, 2465724.661805556, 2465754.074305556, 
   2465783.543055556, 
   // year 2039
   2465813.066666667, 2465842.637500000, 2465872.250000000, 2465901.899305556, 
   2465931.568055556, 2465961.222916667, 2465990.829166667, 2466020.368055556, 
   2466049.849305556, 2466079.297916667, 2466108.740277778, 2466138.188888889, 
   // year 2040
   2466167.642361111, 2466197.100000000, 2466226.573611111, 2466256.083333334, 
   2466285.644444444, 2466315.252083333, 2466344.885416667, 2466374.518055555, 
   2466404.134722222, 2466433.726388889, 2466463.288888889, 2466492.814583333, 
   // year 2041
   2466522.297222222, 2466551.738194444, 2466581.152083333, 2466610.561805556, 
   2466639.990277778, 2466669.455555555, 2466698.970138889, 2466728.543055556, 
   2466758.177777778, 2466787.861805555, 2466817.562500000, 2466847.234027778, 
   2466876.837500000, 
   // year 2042
   2466906.362500000, 2466935.818750000, 2466965.224305556, 2466994.596527778, 
   2467023.954861111, 2467053.325000000, 2467082.744444444, 2467112.250694444, 
   2467141.868055556, 2467171.585416667, 2467201.352777778, 2467231.104166667, 
   // year 2043
   2467260.786805556, 2467290.380555556, 2467319.881250000, 2467349.295833333, 
   2467378.639583333, 2467407.940972222, 2467437.243750000, 2467466.599305556, 
   2467496.053472222, 2467525.633333333, 2467555.331250000, 2467585.109027778, 
   2467614.908333333, 
   // year 2044
   2467644.669444445, 2467674.341666667, 2467703.893055555, 2467733.320833333, 
   2467762.652777778, 2467791.933333333, 2467821.215277778, 2467850.545833333, 
   2467879.960416667, 2467909.483333333, 2467939.123611111, 2467968.870138889, 
   // year 2045
   2467998.684027778, 2468028.493750000, 2468058.218750000, 2468087.810416667, 
   2468117.268055555, 2468146.628472222, 2468175.936111111, 2468205.235416667, 
   2468234.561111111, 2468263.942361111, 2468293.409027778, 2468322.986805555, 
   // year 2046
   2468352.683333333, 2468382.465277778, 2468412.260416667, 2468441.994444444, 
   2468471.622222222, 2468501.140277778, 2468530.568750000, 2468559.934027778, 
   2468589.267361111, 2468618.600694445, 2468647.970138889, 2468677.409722222, 
   2468706.943750000, 
   // year 2047
   2468736.572222222, 2468766.268055555, 2468795.988888889, 2468825.694444445, 
   2468855.352083333, 2468884.941666667, 2468914.450694445, 2468943.886111111, 
   2468973.271527778, 2469002.644444444, 2469032.040972222, 2469061.484722222, 
   // year 2048
   2469090.980555556, 2469120.521527778, 2469150.102777778, 2469179.722222222, 
   2469209.373611111, 2469239.034722222, 2469268.669444445, 2469298.249305556, 
   2469327.767361111, 2469357.239583334, 2469386.693055556, 2469416.145833334, 
   // year 2049
   2469445.600000000, 2469475.052777778, 2469504.507638889, 2469533.985416667, 
   2469563.507638889, 2469593.083333334, 2469622.701388889, 2469652.338194444, 
   2469681.971527778, 2469711.586805556, 2469741.177083334, 2469770.733333333, 
   2469800.244444444, 
   // year 2050
   2469829.706250000, 2469859.127083333, 2469888.528472222, 2469917.934722222, 
   2469947.368750000, 2469976.848611111, 2470006.386805556, 2470035.990972222, 
   2470065.659027778, 2470095.367361111, 2470125.070138889, 2470154.720833333, 
   // year 2051
   2470184.290277778, 2470213.778472222, 2470243.202777778, 2470272.582638889, 
   2470301.936805556, 2470331.288888889, 2470360.672916667, 2470390.128472222, 
   2470419.689583333, 2470449.365972222, 2470479.124305556, 2470508.900694444, 
   // year 2052
   2470538.628472222, 2470568.270833334, 2470597.816666667, 2470627.268750000, 
   2470656.639583333, 2470685.951388889, 2470715.243055556, 2470744.563194444, 
   2470773.963194444, 2470803.481250000, 2470833.127083333, 2470862.876388889, 
   2470892.677083334, 
   // year 2053
   2470922.466666667, 2470952.188194444, 2470981.799305555, 2471011.283333333, 
   2471040.654861111, 2471069.952083333, 2471099.226388889, 2471128.528472222, 
   2471157.900000000, 2471187.370833333, 2471216.954861111, 2471246.652777778, 
   // year 2054
   2471276.440277778, 2471306.259722222, 2471336.031944444, 2471365.688888889, 
   2471395.209027778, 2471424.611111111, 2471453.940277778, 2471483.241666667, 
   2471512.554166667, 2471541.909027778, 2471571.334027778, 2471600.856944445, 
   2471630.494444444, 
   // year 2055
   2471660.235416667, 2471690.027083333, 2471719.792361111, 2471749.470138889, 
   2471779.039583333, 2471808.511111111, 2471837.908333333, 2471867.260416667, 
   2471896.597222222, 2471925.951388889, 2471955.356944445, 2471984.843750000, 
   // year 2056
   2472014.424305555, 2472044.083333334, 2472073.786111111, 2472103.493750000, 
   2472133.170833333, 2472162.794444445, 2472192.347222222, 2472221.825000000, 
   2472251.240972222, 2472280.625694444, 2472310.014583333, 2472339.438194444, 
   // year 2057
   2472368.909027778, 2472398.424305555, 2472427.975694445, 2472457.563194444, 
   2472487.188888889, 2472516.840972222, 2472546.490972222, 2472576.105555556, 
   2472605.663194445, 2472635.166666667, 2472664.638194445, 2472694.099305556, 
   2472723.557638889, 
   // year 2058
   2472753.009722222, 2472782.456250000, 2472811.909722222, 2472841.395138889, 
   2472870.932638889, 2472900.524305556, 2472930.152777778, 2472959.793750000, 
   2472989.429166667, 2473019.045138889, 2473048.631250000, 2473078.175000000, 
   // year 2059
   2473107.664583333, 2473137.102083333, 2473166.504166667, 2473195.895138889, 
   2473225.302083334, 2473254.747916667, 2473284.249305556, 2473313.818055556, 
   2473343.459027778, 2473373.159722222, 2473402.883333333, 2473432.575694445, 
   // year 2060
   2473462.194444445, 2473491.724305556, 2473521.175000000, 2473550.568055556, 
   2473579.924305555, 2473609.266666667, 2473638.623611111, 2473668.034722222, 
   2473697.538888889, 2473727.162500000, 2473756.893055555, 2473786.677777778, 
   2473816.444444445, 
   // year 2061
   2473846.136111111, 2473875.730555556, 2473905.224305556, 2473934.628472222, 
   2473963.960416667, 2473993.252083333, 2474022.549305555, 2474051.902777778, 
   2474081.359722222, 2474110.945833333, 2474140.652777778, 2474170.439583333, 
   // year 2062
   2474200.245138889, 2474230.007638889, 2474259.675694444, 2474289.220138889, 
   2474318.640972222, 2474347.966666667, 2474377.245138889, 2474406.528472222, 
   2474435.863194444, 2474465.284722222, 2474494.814583333, 2474524.459027778, 
   2474554.206250000, 
   // year 2063
   2474584.015972222, 2474613.818055556, 2474643.534722222, 2474673.119444444, 
   2474702.574305556, 2474731.934722222, 2474761.246527778, 2474790.554166667, 
   2474819.890277778, 2474849.282638889, 2474878.756944445, 2474908.336111111, 
   // year 2064
   2474938.025694444, 2474967.793750000, 2474997.572916667, 2475027.293055556, 
   2475056.913194445, 2475086.431250000, 2475115.865277778, 2475145.242361111, 
   2475174.590972222, 2475203.940277778, 2475233.322916667, 2475262.770138889, 
   // year 2065
   2475292.302083334, 2475321.918055556, 2475351.593750000, 2475381.292361111, 
   2475410.979861111, 2475440.628472222, 2475470.219444444, 2475499.740277778, 
   2475529.194444445, 2475558.600694445, 2475587.992361111, 2475617.402777778, 
   2475646.852777778, 
   // year 2066
   2475676.343055556, 2475705.868750000, 2475735.426388889, 2475765.020833334, 
   2475794.652083333, 2475824.302777778, 2475853.940277778, 2475883.534722222, 
   2475913.075000000, 2475942.571527778, 2475972.045833333, 2476001.512500000, 
   // year 2067
   2476030.970138889, 2476060.415277778, 2476089.853472222, 2476119.308333333, 
   2476148.806250000, 2476178.361805555, 2476207.970138889, 2476237.609027778, 
   2476267.256250000, 2476296.894444444, 2476326.509722222, 2476356.086805556, 
   // year 2068
   2476385.609722222, 2476415.072916667, 2476444.484722222, 2476473.869444444, 
   2476503.254861111, 2476532.669444445, 2476562.133333333, 2476591.663194445, 
   2476621.270138889, 2476650.950694445, 2476680.678472222, 2476710.404861111, 
   2476740.072916667, 
   // year 2069
   2476769.650694444, 2476799.136805556, 2476828.551388889, 2476857.915972222, 
   2476887.254166667, 2476916.593055556, 2476945.967361111, 2476975.418750000, 
   2477004.983333333, 2477034.669444445, 2477064.443055556, 2477094.234722222, 
   // year 2070
   2477123.974305556, 2477153.620138889, 2477183.161805556, 2477212.604861111, 
   2477241.964583333, 2477271.267361111, 2477300.552083334, 2477329.869444444, 
   2477359.270138889, 2477388.793055556, 2477418.446527778, 2477448.204166667, 
   // year 2071
   2477478.011111111, 2477507.802777778, 2477537.522222222, 2477567.127777778, 
   2477596.604861111, 2477625.970138889, 2477655.264583333, 2477684.539583333, 
   2477713.845138889, 2477743.223611111, 2477772.701388889, 2477802.291666667, 
   2477831.990972222, 
   // year 2072
   2477861.774305556, 2477891.586111111, 2477921.348611111, 2477950.997916667, 
   2477980.513194445, 2478009.915277778, 2478039.247222222, 2478068.556250000, 
   2478097.879861111, 2478127.247222222, 2478156.681944444, 2478186.207638889, 
   // year 2073
   2478215.840972222, 2478245.570138889, 2478275.344444444, 2478305.093055556, 
   2478334.761111111, 2478364.327777778, 2478393.803472222, 2478423.211111111, 
   2478452.578472222, 2478481.931944444, 2478511.300694444, 2478540.716666667, 
   2478570.205555555, 
   // year 2074
   2478599.776388889, 2478629.417361111, 2478659.097222222, 2478688.783333333, 
   2478718.447916667, 2478748.068750000, 2478777.629861111, 2478807.125000000, 
   2478836.561805556, 2478865.966666667, 2478895.372222222, 2478924.805555556, 
   // year 2075
   2478954.275694444, 2478983.778472222, 2479013.309722222, 2479042.872222222, 
   2479072.474305556, 2479102.111111111, 2479131.758333333, 2479161.382638889, 
   2479190.960416667, 2479220.488888889, 2479249.983333333, 2479279.461111111, 
   // year 2076
   2479308.927083334, 2479338.376388889, 2479367.809027778, 2479397.241666667, 
   2479426.702777778, 2479456.218750000, 2479485.795138889, 2479515.420833333, 
   2479545.072222222, 2479574.727777778, 2479604.368750000, 2479633.978472222, 
   2479663.537500000, 
   // year 2077
   2479693.031944444, 2479722.463194444, 2479751.850694445, 2479781.222916667, 
   2479810.609722222, 2479840.038194445, 2479869.529166667, 2479899.095833333, 
   2479928.745138889, 2479958.463888889, 2479988.209027778, 2480017.921527778, 
   // year 2078
   2480047.552083334, 2480077.082638889, 2480106.526388889, 2480135.906250000, 
   2480165.247916667, 2480194.576388889, 2480223.922916667, 2480253.328472222, 
   2480282.832638889, 2480312.462500000, 2480342.206250000, 2480372.006250000, 
   // year 2079
   2480401.785416667, 2480431.483333333, 2480461.075694445, 2480490.563194444, 
   2480519.956944444, 2480549.279166667, 2480578.563888889, 2480607.856944445, 
   2480637.210416667, 2480666.671527778, 2480696.263888889, 2480725.979166667, 
   2480755.772222222, 
   // year 2080
   2480785.580555555, 2480815.341666667, 2480845.004861111, 2480874.541666667, 
   2480903.956250000, 2480933.278472222, 2480962.556944444, 2480991.843055556, 
   2481021.184722222, 2481050.614583334, 2481080.151388889, 2481109.798611111, 
   // year 2081
   2481139.543750000, 2481169.345833333, 2481199.136805556, 2481228.844444444, 
   2481258.422916667, 2481287.876388889, 2481317.239583334, 2481346.559027778, 
   2481375.876388889, 2481405.225000000, 2481434.628472222, 2481464.109027778, 
   2481493.686805556, 
   // year 2082
   2481523.365972222, 2481553.117361111, 2481582.879166667, 2481612.585416667, 
   2481642.200000000, 2481671.720138889, 2481701.163194445, 2481730.554861111, 
   2481759.919444445, 2481789.285416667, 2481818.680555556, 2481848.132638889, 
   // year 2083
   2481877.660416667, 2481907.261111111, 2481936.914583333, 2481966.590277778, 
   2481996.260416667, 2482025.901388889, 2482055.496527778, 2482085.031944444, 
   2482114.505555556, 2482143.933333333, 2482173.344444444, 2482202.767361111, 
   // year 2084
   2482232.220833333, 2482261.703472222, 2482291.211111111, 2482320.745138889, 
   2482350.314583333, 2482379.926388889, 2482409.568750000, 2482439.211805556, 
   2482468.823611111, 2482498.386805556, 2482527.908333333, 2482557.402777778, 
   2482586.880555556, 
   // year 2085
   2482616.338194444, 2482645.772916667, 2482675.193055556, 2482704.625000000, 
   2482734.100000000, 2482763.638194445, 2482793.238888889, 2482822.882638889, 
   2482852.547222222, 2482882.208333334, 2482911.847916667, 2482941.443750000, 
   // year 2086
   2482970.975694445, 2483000.436111111, 2483029.836805556, 2483059.204166667, 
   2483088.570833333, 2483117.967361111, 2483147.418750000, 2483176.943750000, 
   2483206.554166667, 2483236.247916667, 2483265.995833333, 2483295.742361111, 
   // year 2087
   2483355.007638889, 2483384.490277778, 2483413.893750000, 2483443.244444444, 
   2483472.568750000, 2483501.897222222, 2483531.264583333, 2483560.714583333, 
   2483590.283333333, 2483619.979166667, 2483649.766666667, 2483679.571527778, 
   // year 2088
   2483709.318750000, 2483738.965277778, 2483768.500694444, 2483797.934722222, 
   2483827.284722222, 2483856.579861111, 2483885.860416667, 2483915.177777778, 
   2483944.582638889, 2483974.111111111, 2484003.772916667, 2484033.536805556, 
   // year 2089
   2484063.345833333, 2484093.136805556, 2484122.850694445, 2484152.448611111, 
   2484181.919444445, 2484211.281250000, 2484240.575000000, 2484269.853472222, 
   2484299.165972222, 2484328.552777778, 2484358.038888889, 2484387.633333333, 
   2484417.331944444, 
   // year 2090
   2484447.107638889, 2484476.907638889, 2484506.659027778, 2484536.300694444, 
   2484565.812500000, 2484595.216666667, 2484624.555555556, 2484653.875000000, 
   2484683.211111111, 2484712.590277778, 2484742.034027778, 2484771.562500000, 
   // year 2091
   2484801.188888889, 2484830.902083333, 2484860.657638889, 2484890.389583333, 
   2484920.047222222, 2484949.612500000, 2484979.094444444, 2485008.515972222, 
   2485037.899305556, 2485067.270833334, 2485096.654861111, 2485126.079861111, 
   // year 2092
   2485155.568055556, 2485185.127777778, 2485214.747916667, 2485244.404166667, 
   2485274.069444445, 2485303.720833333, 2485333.341666667, 2485362.913194445, 
   2485392.427083334, 2485421.886111111, 2485451.312500000, 2485480.734027778, 
   2485510.174305555, 
   // year 2093
   2485539.640972222, 2485569.129861111, 2485598.638194445, 2485628.175694444, 
   2485657.755555556, 2485687.379166667, 2485717.025694444, 2485746.662500000, 
   2485776.261805556, 2485805.815277778, 2485835.332638889, 2485864.825694445, 
   // year 2094
   2485894.295833333, 2485923.738888889, 2485953.156250000, 2485982.568055556, 
   2486012.006944445, 2486041.502777778, 2486071.068055556, 2486100.693055556, 
   2486130.355555556, 2486160.031250000, 2486189.696527778, 2486219.327083333, 
   // year 2095
   2486248.898611111, 2486278.395833334, 2486307.819444445, 2486337.192361111, 
   2486366.546527778, 2486395.915972222, 2486425.329861111, 2486454.812500000, 
   2486484.379166667, 2486514.038194445, 2486543.772916667, 2486573.538194445, 
   2486603.267361111, 
   // year 2096
   2486632.906944444, 2486662.436805556, 2486691.872222222, 2486721.238888889, 
   2486750.566666667, 2486779.884027778, 2486809.223611111, 2486838.625694444, 
   2486868.132638889, 2486897.770138889, 2486927.525694444, 2486957.338194444, 
   // year 2097
   2486987.125694444, 2487016.826388889, 2487046.415277778, 2487075.893750000, 
   2487105.278472222, 2487134.593750000, 2487163.874305556, 2487193.167361111, 
   2487222.524305556, 2487251.990277778, 2487281.590277778, 2487311.311111111, 
   // year 2098
   2487341.106250000, 2487370.913194445, 2487400.669444445, 2487430.325000000, 
   2487459.856250000, 2487489.266666667, 2487518.588194444, 2487547.869444444, 
   2487577.162500000, 2487606.512500000, 2487635.951388889, 2487665.494444444, 
   2487695.142361111, 
   // year 2099
   2487724.880555556, 2487754.671527778, 2487784.450000000, 2487814.146527778, 
   2487843.720833333, 2487873.174305555, 2487902.543055556, 2487931.872222222, 
   2487961.202777778, 2487990.564583333, 2488019.979861111, 2488049.465277778, 
   // year 2100
   2488079.039583333, 2488108.705555555, 2488138.437500000, 2488168.179166667, 
   2488197.871527778, 2488227.481250000, 2488257.005555556, 2488286.460416667, 
   2488315.868750000, 2488345.252777778, 2488374.635416667, 2488404.043055556, 
   2488433.498611111 
};

int lookup_new_moon(double jd)
{
int i, j;
int i0;

   gregorian(0, jd);
   if((g_year >= 2016) && (g_year < 2100)) {  // look up accurate new moon info in table
      i0 = (g_year-2016) * 12;  // approx place to start looking in the table
      j = sizeof(new_moon_table) / sizeof(new_moon_table[0]);
      for(i=i0; i<j; i++) {
         if(jd <= new_moon_table[i]) {
            return i-1;
         }
      }
   }
   return (-1);
}

double moon_synod(double jd)
{
int i;
double d1,d2,d3;
double delta;
double pct,mid;
double synod;

   // return moon synod (in days) for the month jd lies in
   // the value is linearly interpolated between the two adjacent synods.

   i = lookup_new_moon(jd);
   if(i < 0) { // jd is outside of the table range, calculate the synod
      calc_synod:
      synod = (jd - JD2000) / 36525.0;
      synod = 29.5305888531 + 0.00000021621*synod - 3.64E-10*synod*synod;  // average length of synodic month
      return synod;
   }

   d3 = new_moon_table[i+2];
   d2 = new_moon_table[i+1];
   d1 = new_moon_table[i+0];

   d1 = d2 - d1;  // this months synod
   d2 = d3 - d2;  // next months synod

   delta = (d2 - d1);
   if(delta == 0.0) goto calc_synod;
   if(d1 == 0.0) goto calc_synod;

   pct = (jd - new_moon_table[i]) / d1;
   mid = d1 + (delta * pct);

// sprintf(debug_text2, "d1:%f d2:%f pct:%f  delta:%f  mid:%f", d1,d2, pct, delta, mid);
   return mid;
}


double new_moon(double jd)
{
double start, mid, end;
double p1, p2, p3;
int i;
double start_step, end_step;

   sp_count = 0;
   if(0 && debug_file) {
      gregorian(0, jd);
      fprintf(debug_file, "New moon pinfo for %04d/%02d/%02d\n", pri_year,pri_month,pri_day);
      fprintf(debug_file, "New moon ginfo for %04d/%02d/%02d\n", g_year,g_month,g_day);
   }

   i = lookup_new_moon(jd);
   if(i >= 0) {
      return new_moon_table[i];
   }


   // calculates the julian date of the last new moon (to around 1 second)
   // but probably accurate to around 10 minutes... maybe... perhaps

   mid = jd;
   start = mid - 1.0; // yesterday
   end = mid + 1.0;   // tomorrow

   p1 = MOON_PHASE(start);  // get the moon illumination of the three consecutive days
   p2 = MOON_PHASE(mid);
   p3 = MOON_PHASE(end);

   for(i=0; i<31; i++) {   // check back a little more than a lunar month's worth of days
      if((p3 >= p2) && (p1 >= p2)) goto refine_new_moon;  // illumination is at a minimum this day

      end = mid;           // back up a day and try again
      mid = start;
      start = start - 1.0;
      p3 = p2;
      p2 = p1;
      p1 = MOON_PHASE(start);
   }

if(0 && debug_file) fprintf(debug_file, "No new moon solution at jd:%f\n", start);
   return 0.0;        // something went wrong, no minimum found

   refine_new_moon:

   start_step = 0.6;  // start seaching +/- 0.6 days from daily minimum
   end_step = 0.6;

if(0 && debug_file) fprintf(debug_file, "refine moon %d:%6.2f  ", sp_count, (end-start)*(24.0*60.0*60.0));
   for(i=0; i<40; i++) {  // binary seach for the minimum illumnation down to the second
      if((p1 >= p2) && (p2 >= p3)) {    // move end forward
if(0 && debug_file) fprintf(debug_file, "  fix end:   start:%f end:%f start_step:%f, end_step:%f\n", mid-start,end-mid,start_step,end_step);
         end += end_step / 2.0;
         p3 = MOON_PHASE(end);
      }
      else if((p1 <= p2) && (p2 <= p3)) {  // back uo start
if(0 && debug_file) fprintf(debug_file, "  fix start: start:%f end:%f start_step:%f, end_step:%f\n", mid-start,end-mid,start_step,end_step);
         start -= start_step / 2.0;
         p1 = MOON_PHASE(start);
      }
      else {  // minimum is somewhere between start and end, narrow the range
         start_step = (mid - start) / 1.25;
         end_step = (end - mid) / 1.25;
if(0 && debug_file) fprintf(debug_file, "  fix both:  start:%f end:%f start_step:%f, end_step:%f\n", mid-start,end-mid,start_step,end_step);
         start += start_step;
         end -= end_step;
         p1 = MOON_PHASE(start);
         p3 = MOON_PHASE(end);
      }
if(0 && debug_file) fprintf(debug_file, "refine moon %d :%6.2f  ", sp_count, (end-start)*(24.0*60.0*60.0));

      if(fabs(start-end) < (1.0/(24.0*60.0*60.0))) {  // time lies with a 1 second interval
         mid = (start + end) / 2.0;
if(0 && debug_file) fprintf(debug_file, "\nnoon %d: %.9f\n", sp_count, mid);
         break;  // start end end are within a second
      }

      mid = (start + end) / 2.0;
      p2 = MOON_PHASE(mid);
   }
   return mid;
}


int day_of_year(int month, int day)
{
   month = fix_month(month);
   return dsm[month] + day;
}

void calc_moons(double jd, int why)
{
double p, last_p;
double last_jd, jd_end;
int rising;
int blue_moon;
int black_moon;
int new_event;
int i;

   // Find all primary moon phase events for the month and flag them in
   // the array moons[].  These are used to label the analog watch face.

   for(i=0; i<32; i++) moons[i] = "";  // initalize moon phase names

   new_moon_jd = 0.0;
   full_moon_jd = 0.0;
   blue_moon_jd = 0.0;
   black_moon_jd = 0.0;
   blue_moon  = 0;       // counts full moons
   black_moon = 0;       // counts new moons

   gregorian(0, jd);
   last_jd = jdate(g_year,g_month,1) - jtime(1,0,0,1.0);
//last_jd = jdate(year,month,1) - jtime(1,0,0,0.0); 
   g_month = fix_month(g_month);
   jd_end = last_jd + (double) (dim[g_month]);

   last_p  = MOON_PHASE(last_jd);
   rising  = 0;

   while(last_jd < jd_end) {  // search the month for interesting lunar events
      jd = last_jd + jtime(1,0,0,0.0);
      p = MOON_PHASE(jd);
////  gregorian(0, jd + time_zone() + jtime(0,0,0,TSX_TWEAK));  // calculate moon time in local time zone
      gregorian(0, jd);  // calculate moon time in utc
      if((g_day < 0) || (g_day > 31)) g_day = 0;

      new_event = 0;

      if((last_p < 0.50) && (p >= 0.50)) {  // waxing quarter moon
         moons[g_day] = "Qtr Moon ";
         new_event = 1;
      }
      if((last_p >= 0.50) && (p < 0.50)) {  // waning quarter moon
         moons[g_day] = "Qtr Moon ";
         new_event = 2;
      }

      if(rising > 0) {
         if(p < last_p) {
            if(blue_moon) {  // moon started falling
               moons[g_day] = "Blue Moon";
               blue_moon_jd = jd;
            }
            else {
               moons[g_day] = "Full Moon";
               full_moon_jd = jd;
            }
            ++blue_moon;
            new_event = 3;

         }
      }
      else if(rising < 0) {  // moon started rising
         if(p > last_p) {
            if(black_moon) {
               moons[g_day] = "Black Moon";
               black_moon_jd = jd;
            }
            else {  // new moon 
               moons[g_day] = "New Moon";
               new_moon_jd = jd;
            }
            ++black_moon;
            new_event = 4;
         }
      }

      if     (p > last_p) rising = (+1);
      else if(p < last_p) rising = (-1);

      last_p = p;
      if(new_event) last_jd = jd + 6.00;  // next possible event averages 7.35 days away, we use 6 for safety
      else          last_jd = jd;         // keep seaching hour-by-hour
   }

   if(0 && debug_file) { 
      if(new_moon_jd) {
         gregorian(0, new_moon_jd + time_zone());
         fprintf(debug_file, "new moon:  %04d/%02d/%02d  %02d:%02d:%02d\n", g_year,g_month,g_day, g_hours,g_minutes,g_seconds);
      }
      if(full_moon_jd) {
         gregorian(0, full_moon_jd + time_zone());
         fprintf(debug_file, "full moon: %04d/%02d/%02d  %02d:%02d:%02d\n", g_year,g_month,g_day, g_hours,g_minutes,g_seconds);
      }
      if(blue_moon_jd) {
         gregorian(0, blue_moon_jd + time_zone());
         fprintf(debug_file, "blue moon: %04d/%02d/%02d  %02d:%02d:%02d\n", g_year,g_month,g_day, g_hours,g_minutes,g_seconds);
      }
      if(black_moon_jd) {
         gregorian(0, black_moon_jd + time_zone());
         fprintf(debug_file, "black moon: %04d/%02d/%02d  %02d:%02d:%02d\n", g_year,g_month,g_day, g_hours,g_minutes,g_seconds);
      }
   }
}

//
//
//  Propogation delay calculations
//
//

#define RADPERDEG       (PI/180.0)      /* radians per degree */
#define KM_PER_MILE     (5280.0/(FEET_PER_METER*1000.0))      /* km in a mile */

/*
 * greatcircle - compute the great circle distance in kilometers
 */
double greatcircle(double lat1,double long1, double lat2,double long2)
{
double dg;
double l1r, l2r;

   l1r = lat1;
   l2r = lat2;
   dg = earth_radius(lat,alt) * acos(
           (cos(l1r) * cos(l2r) * cos(long2-long1))
           + (sin(l1r) * sin(l2r)));
   if(debug_file && show_debug_info) {
       fprintf(debug_file,
               "greatcircle lat1 %.9f long1 %.9f lat2 %.9f long2 %.9f dist %.9f\n",
               lat1, long1, lat2, long2, dg);
       fflush(debug_file);
   }

   dg = check_double(dg);  // NAN check
   return dg;
}


double az_angle(double lat1,double lon1, double lat2,double lon2)
{
double x,y;
double az;

   y = sin(lon2-lon1) * cos(lat2);
   x = cos(lat1)*sin(lat) - sin(lat1)*cos(lat2)*cos(lon2-lon1);
   az = atan2(y, x) * 180.0/PI;
   az = fmod(az + 360.0, 360.0);
   return az;
}

/*
 * waveangle - compute the wave angle for the given distance, virtual
 *             height and number of hops.
 */
double waveangle(double dg, double h, int n)
{
double theta;
double delta;

   theta = dg / (earth_radius(lat,alt) * (double)(2 * n));
   delta = atan((h / (earth_radius(lat,alt) * sin(theta))) + tan(theta/2)) - theta;
   if(0 && debug_file) {
       fprintf(debug_file, "waveangle dist %g height %g hops %d angle %g\n",
              dg, h, n, delta / RADPERDEG);
       fflush(debug_file);
   }
   return delta;
}


/*
 * propdelay - compute the propagation delay
 */
static double propdelay(double dg, double h, int n)
{
double phi;
double theta;
double td;

     theta = dg / (earth_radius(lat,alt) * (double)(2 * n));
     phi = (PI/2.0) - atan((h / (earth_radius(lat,alt) * sin(theta))) + tan(theta/2));
     td = dg / (LIGHTSPEED * sin(phi));
     if(debug_file) {
         fprintf(debug_file, "propdelay dist %.9f height %.9f hops %d time %.9f\n",
                dg, h, n, td);
         fflush(debug_file);
     }
     return td;
}


/*
 * finddelay - find the propagation delay
 */
int find_delay(double lat1,double long1,double lat2,double long2,double h,  double u_dist, double *dist,double *delay)
{
double dg;      /* great circle distance */
double delta;   /* wave angle */
int n;          /* number of hops */

   if(u_dist) dg = u_dist;
   else       dg = greatcircle(lat1, long1, lat2, long2);
   if(dg == 0.0) dg = 0.000001;
   if(debug_file) {
       fprintf(debug_file, "great circle distance %.9f km %.9f miles\n", dg, dg/KM_PER_MILE);
       fflush(debug_file);
   }
   
   n = 1;
   while((delta = waveangle(dg, h, n)) < 0.0) {
      if(0 && debug_file) {
          fprintf(debug_file, "tried %d hop%s, no good\n", n, n>1?"s":"");
          fflush(debug_file);
      }

      n++;
      if(n > 1000) break;  // should never happen
   }
   if(debug_file) {
       fprintf(debug_file, "%d hop%s okay, wave angle is %g\n", n, n>1?"s":"",
              delta / RADPERDEG);
       fflush(debug_file);
   }

   *dist = dg;
   *delay = propdelay(dg, h, n);
   return n;
}


//
//
//   Gravity and solid earth tide calculations
//
//


#define EARTH_MASS 5.973600e24 // kg
#define MOON_MASS  7.347673e22 // kg
#define SUN_MASS   1.989100e30 // kg
#define GRAVITY    6.67408e-11 // m^3 kg^-1 s^-2
#define GRAV_FORCE(m1,m2,d) ((2.0*GRAVITY*m1*m2*earth_radius(lat,alt)*1000.0)/(d*d*d))

// Days since 1900 Jan 0.5 // 1900-01-00 12h = 1899-12-31 12h = MJD 15019.5
#define fMJD_TO_J1900(n) ( (n) - 15019.5 )

// Seconds from 1970-01-01
#define MJD_TO_UNIX(n) ( ((n) - 40587) * 86400 )

#define SQ2(x) ( (x) * (x) )
#define SQ3(x) ( (x) * (x) * (x) )
#define SQ4(x) ( (x) * (x) * (x) * (x) )
#define POLY(t,a,b,c,d) ( (a) + (b) * (t) + (c) * SQ2(t) + d * SQ3(t) )
#define RAD(x) ( (x) / 180.0 * PI )

// Constants.

double mu = 6.67e-08;
double m = 7.3537e+25;
double s = 1.993e+33;
double il = 0.08979719;
double omega = 0.4093146162;
double ml = 0.074804;
double el = 0.0549;
double cl1 = 1.495e+13;
double cl = 3.84402e+10;
double al = 6.37827e+08;

// Love Numbers.

double love_h2 = 0.612;
double love_k2 = 0.303;

//   Calculate lunar/solar tidal gravitational correction in uGal units
//   given fractional MJD and position (latitude/longitude/altitude).
//   Returns units of micro-gals. Typical corrections are +/- 100 uGals.
//   g is about 980 Gals.
//
//   Value calculated is the UPWARD pull due to the sun and moon. To use
//   as correction to measured gravity data, you would need to ADD these
//   numbers, not subtract them.  When the moon is overhead, for example
//   this program predicts a relatively large positive number, indicating
//   a large upward pull due to the moon. This would result in a DECREASE
//   in a gravity meter reading. Thus the tide value would be ADDED to
//   correct for this effect.

double calc_gals(double jd, double lat, double lon, double alt)
{
double tl0, j1900, t, n, el1, sl, pl, hl, pl1, i, nu, tl;
double chi, chi1, ll1, cosalf, sinalf, alf, xi, sigma, ll, lm;
double costht, cosphi, c, rl, ap, ap1, dl, D, gm, gs, g0, love;
double fmjd;

    fmjd = jd - JD_MJD;

    tl0 = fmod(fmjd, 1.0) * 24.0;
    j1900 = fMJD_TO_J1900(fmjd);
    t = j1900 / 36525.0;
    n = POLY(t, 4.523601612, -33.75715303, 0.0000367488, 0.0000000387);
    el1 = POLY(t, 0.01675104, -0.0000418, 0.000000126, 0);
    sl = POLY(t, 4.720023438, 8399.7093, 0.0000440695, 0.0000000329);
    pl = POLY(t, 5.835124721, 71.01800936, -0.0001805446, -0.0000002181);
    hl = POLY(t, 4.881627934,628.3319508, 0.0000052796, 0);
    pl1 = POLY(t, 4.908229467, 0.0300052641, 7.9024e-06, 0.0000000581);
    i = acos(0.9136975738 - 0.0356895353 * cos(n));
    if(i) nu = asin(0.0896765581 * sin(n) / sin(i));
else  nu = 0.0;
    tl = RAD(15.0 * tl0 + lon);
    chi = tl + hl - nu;
    chi1 = tl + hl;
    ll1 = hl + 2.0 * el1 * sin(hl - pl1);
    cosalf = cos(n) * cos(nu) + sin(n) * sin(nu) * 0.9173938078;
    sinalf = 0.3979806546 * sin(n) / sin(i);
    alf = 2.0 * atan(sinalf / (1 + cosalf));
    xi = n - alf;
    sigma = sl - xi;
    ll = sigma + 0.1098 * sin(sl - pl)
        + 0.0037675125 * sin(2 * (sl - pl))
        + 0.0154002735 * sin(sl - 2 * hl + pl)
        + 0.0076940028 * sin(2 * (sl - hl));
    lm = RAD(lat);   // lamda
    costht = sin(lm) * sin(i) * sin(ll) + cos(lm) *
        ( SQ2(cos(0.5 * i)) * cos(ll - chi) + SQ2(sin(0.5 * i)) * cos(ll + chi) );
    cosphi = sin(lm) * 0.3979806546 * sin(ll1) + cos(lm) *
        ( 0.9586969039 * cos(ll1 - chi1) + 0.0413030961 * cos(ll1 + chi1) );
    c = 1 / sqrt(1 + 0.006738 * SQ2(sin(lm)));
    rl = 6.37827e+08 * c + alt * 100.0; // meters to cm
    ap = 2.60930776e-11;
    ap1 = 1.0 / (1.495e+13 * (1.0 - el1 * el1));
    dl = 1.0 / (1.0 / cl
        + ap * el * cos(sl - pl)
        + ap * el * el * cos(2.0 * (sl - pl))
        + 1.875 * ap * ml * el * cos(sl - 2.0 * hl + pl)
        + ap * ml * ml * cos(2.0 * (sl - hl)) );
    D = 1.0 / (1.0 / cl1
        + ap1 * el1 * cos(hl - pl1));
    gm = mu * m * rl * (3.0 * SQ2(costht) - 1.0) / SQ3(dl)
        + 1.5 * mu * m * rl * rl * (5 * SQ3(costht) - 3 * costht) / SQ4(dl);
    gs = mu * s * rl * (3.0 * SQ2(cosphi) - 1.0) / SQ3(D);
    love = (1.0 + love_h2 - 1.5 * love_k2);
    g0 = (gm + gs) * love;

#if 0
    sprintf(plot_title, "%.6f  %.9e  %.3e  %.2f  %.2f  %.2f",
           fmjd,
           (980 - g0) / 100.0,  // g in m/s2 units
           -g0 / 980.0,         // pendulum clock rate
           g0 * 1e6,            // delta g in ugal units
           gm * love * 1e6, gs * love * 1e6);
#endif

    last_ugals = ugals;
    ugals = g0 * 1.0E6;
    have_ugals = 1;
    tide_cm = (ugals * 0.17785) - 0.9766;  // emprical conversion from uGal to cm of displacement

    return ugals;
}


//
//  Note: the solid earth tide code was originally written in Fortran and
//        was converted to C using F2C... hence the slightly ugly code
//        and function parameter passing by address.
//
//
//  If (tide_options & 0x01) use solid.f routines,  else, use Heather's
//  more accurate routines calculate sun and moon positions.  These 
//  accurate positions are calculated based upon jd_utc and jd_tt and
//  NOT the times passed to moonxyz_() and sunxyz_()
//  The way Heather works, these times should be the same.
//  Heather's routines are more accurate than solid.f
//  and have already been called, so there is no need
//  to duplicate the work.
//
//  If (tide_options & 0x02) restore the deformation due to permanent
//  earth tides to the results.
//

#define d_mod(a,b) fmod(*a,*b)

double datdi[] = { // 9x31 Fortran array
   -3., 0., 2., 0., 0.,-0.01,-0.01, 0.0 , 0.0,
   -3., 2., 0., 0., 0.,-0.01,-0.01, 0.0 , 0.0,
   -2., 0., 1.,-1., 0.,-0.02,-0.01, 0.0 , 0.0,
   -2., 0., 1., 0., 0.,-0.08, 0.00, 0.01, 0.01,      
   -2., 2.,-1., 0., 0.,-0.02,-0.01, 0.0 , 0.0,
   -1., 0., 0.,-1., 0.,-0.10, 0.00, 0.00, 0.00,      
   -1., 0., 0., 0., 0.,-0.51, 0.00,-0.02, 0.03,      
   -1., 2., 0., 0., 0., 0.01, 0.0 , 0.0 , 0.0,
    0.,-2., 1., 0., 0., 0.01, 0.0 , 0.0 , 0.0,
    0., 0.,-1., 0., 0., 0.02, 0.01, 0.0 , 0.0,
    0., 0., 1., 0., 0., 0.06, 0.00, 0.00, 0.00,      
    0., 0., 1., 1., 0., 0.01, 0.0 , 0.0 , 0.0,
    0., 2.,-1., 0., 0., 0.01, 0.0 , 0.0 , 0.0,
    1.,-3., 0., 0., 1.,-0.06, 0.00, 0.00, 0.00,      
    1.,-2., 0., 1., 0., 0.01, 0.0 , 0.0 , 0.0,
    1.,-2., 0., 0., 0.,-1.23,-0.07, 0.06, 0.01,      
    1.,-1., 0., 0.,-1., 0.02, 0.0 , 0.0 , 0.0,
    1.,-1., 0., 0., 1., 0.04, 0.0 , 0.0 , 0.0,
    1., 0., 0.,-1., 0.,-0.22, 0.01, 0.01, 0.00,      
    1., 0., 0., 0., 0.,12.00,-0.78,-0.67,-0.03,      
    1., 0., 0., 1., 0., 1.73,-0.12,-0.10, 0.00,      
    1., 0., 0., 2., 0.,-0.04, 0.0 , 0.0 , 0.0,
    1., 1., 0., 0.,-1.,-0.50,-0.01, 0.03, 0.00,      
    1., 1., 0., 0., 1., 0.01, 0.0 , 0.0 , 0.0,
    1., 1., 0., 1.,-1.,-0.01, 0.0 , 0.0 , 0.0,       
    1., 2.,-2., 0., 0.,-0.01, 0.0 , 0.0 , 0.0,
    1., 2., 0., 0., 0.,-0.11, 0.01, 0.01, 0.00,      
    2.,-2., 1., 0., 0.,-0.01, 0.0 , 0.0 , 0.0,
    2., 0.,-1., 0., 0.,-0.02, 0.02, 0.0 , 0.01,
    3., 0., 0., 0., 0., 0.0 , 0.01, 0.0 , 0.01,
    3., 0., 0., 1., 0., 0.0 , 0.01, 0.0 , 0.0
};

/* Common Block Declarations */

struct STUFF_ {
   double rad, pi, pi2;
} stuff_;

#define stuff_1 stuff_

int mjd0;

/* Table of constant values */

static double c_b56 = 360.;
static double c_b88 = 86400.;

/* subroutine prototypes */
int zero_vec8__(double *);
double enorm8_(double *);
int moonxyz_(int *, double *, double *);
int detide_(double *, int *, double *, double *, double *, double *);
int geoxyz_(double *, double *, double *, double *, double *, double *);
int sunxyz_(int *, double *, double *);
int rge_(double *, double *, double *, double *, double *, double *, double *, double *);
int step2diu_(double *, double *, double *, double *);
int step2lon_(double *, double *,double *, double *);
int st1l1_(double *, double *, double *, double *, double *, double *);
int sprod_(double *, double *, double *, double *, double *);
int getghar_(int *, double *, double *);
int st1idiu_(double *, double *, double *, double *, double *, double *);
int st1isem_(double *, double *, double *, double *, double *, double *);
int rot1_(double *, double *, double *, double *, double *, double *, double *);
int rot3_(double *, double *, double *, double *, double *, double *, double *);
double gps2tt_(double *);
double gps2utc_(double *);
double gpsleap_(double *);


void calc_earth_tide(double jd_gps)
{
    static double glad, fmjd, glod, tsec, xsta[3];
    static int lout;
    static double rsun[3], tdel2, etide[3];
    static int iloop;
    static double rmoon[3], x0, y0, z0;
    static double ut, vt, wt;
    static double xt, yt, zt;
    static double sec;
    static int mjd;
    static int ihr, imn, imo, idy, iyr;
    static double gla0, eht0, glo0;

    if((lat == 0.0) && (lon == 0)) return;

    stuff_1.pi = PI;
    stuff_1.pi2 = (PI+PI);
    stuff_1.rad = (180. / PI);

    fmjd = jd_gps - JD_MJD;
    mjd0 = (int) (jd_gps - JD_MJD);
    fmjd = fmjd - (double) mjd0;
    mjd = mjd0;

    gla0 = lat;
    glo0 = lon;
    eht0 = alt; // 0.0;

    geoxyz_(&gla0, &glo0, &eht0, &x0, &y0, &z0);
//sprintf(plot_title, "geo:%f %f %f  mjd0:%d  fmjd:%f", x0,y0,z0, mjd0,fmjd);
    xsta[0] = x0;
    xsta[1] = y0;
    xsta[2] = z0;

    sunxyz_(&mjd, &fmjd, rsun);
//sprintf(debug_text, "rsun:%f %f %f", rsun[0],rsun[1],rsun[2]);
    moonxyz_(&mjd, &fmjd, rmoon);
//sprintf(debug_text2, "rmoon:%f %f %f", rmoon[0],rmoon[1],rmoon[2]);
    detide_(xsta, &mjd, &fmjd, rsun, rmoon, etide);
//sprintf(debug_text3, "etide:%f %f %f", etide[0],etide[1],etide[2]);
    xt = etide[0];
    yt = etide[1];
    zt = etide[2];
    /* determine local geodetic horizon components (topocentric) */
    rge_(&gla0, &glo0, &ut, &vt, &wt, &xt, &yt, &zt);

    last_lat_tide = lat_tide;
    last_lon_tide = lon_tide;
    last_alt_tide = alt_tide;

    lat_tide = ut * 1000.0;  // millimeters
    lon_tide = vt * 1000.0;
    alt_tide = wt * 1000.0;
    have_tides = 1;
}


int detide_(double *xsta, int *mjd, double *fmjd,
         double *xsun, double *xmon, double *dxtide)
{
    /* Initialized data */

    static double h20 = .6078;
    static double l20 = .0847;
    static double h3 = .292;
    static double l3 = .015;

    /* F2C generated locals */
    double d__1, d__2;

    /* Local variables */
    static double mass_ratio_moon__;
    static double rsta, rmon, rsun, p2mon, p3mon, x2mon, x3mon, p2sun, p3sun;
    static int i;
    static double x2sun, x3sun;
    static double t, scmon;
    static double h2, scsun, l2;
    static double re, cosphi, dmjdtt, fmjdtt, tsectt;
    static double fac2mon, fac3mon, fac2sun, fac3sun;
    static double mass_ratio_sun__;
    static double fhr, scm, scs, tsecgps, xcorsta[3];

    static double sinphi;
    static double cosla, sinla;
    static double dr, dn;

    if(xsta == 0) return (-1);
    if(mjd == 0) return (-1); 
    if(fmjd == 0) return (-1); 
    if(xsun == 0) return (-1); 
    if(xmon == 0) return (-1); 
    if(dxtide == 0) return (-1); 

/* computation of tidal corrections of station displacements caused */
/*    by lunar and solar gravitational attraction */
/* step 1 (here general degree 2 and 3 corrections + */
/*         call st1idiu + call st1isem + call st1l1) */
/*   + step 2 (call step2diu + call step2lon + call step2idiu) */
/* it has been decided that the step 3 un-correction for permanent tide */
/* would *not* be applied in order to avoid jump in the reference frame */
/*** (this step 3 must added in order to get the mean tide station position*/
/* and to be conformed with the iag resolution.) */
/* inputs */
/***   xsta(i),i=1,2,3   -- geocentric position of the station (ITRF/ECEF)
*/
/*   xsun(i),i=1,2,3   -- geoc. position of the sun (ECEF) */
/*   xmon(i),i=1,2,3   -- geoc. position of the moon (ECEF) */
/***   mjd,fmjd          -- modified julian day (and fraction) (in GPS time)*/
/****old calling sequence****************************************************/
/***   dmjd               -- time in mean julian date (including day fract
ion)*/
/*   fhr=hr+zmin/60.+sec/3600.   -- hr in the day */
/* outputs */
/*   dxtide(i),i=1,2,3           -- displacement vector (ITRF) */
/* author iers 1996 :  v. dehant, s. mathews and j. gipson */
/*    (test between two subroutines) */
/* author iers 2000 :  v. dehant, c. bruyninx and s. mathews */
/*    (test in the bernese program by c. bruyninx) */
/* created:  96/03/23 (see above) */
/* modified from dehanttideinelMJD.f by Dennis Milbert 2006sep10 */
/* bug fix regarding fhr (changed calling sequence, too) */
/* modified to reflect table 7.5a and b IERS Conventions 2003 */
/* modified to use TT time system to call step 2 functions */
/* sign correction by V.Dehant to match eq.16b, p.81, Conventions */
/* applied by Dennis Milbert 2007may05 */
/*** nominal second degree and third degree love numbers and shida numbers
*/
    /* Function Body */
/* internal support for new calling sequence */
/* also convert GPS time into TT time */
    tsecgps = *fmjd * 86400.;
/* *** GPS time (sec of */
    tsectt = gps2tt_(&tsecgps);
/* *** TT  time (sec of */
    fmjdtt = tsectt / 86400.;
/* *** TT  time (fract. */
    dmjdtt = *mjd + fmjdtt;
/* commented line was live code in dehanttideinelMJD.f */
/* changed on the suggestion of Dr. Don Kim, UNB -- 09mar21 */
/* Julian date for 2000 January 1 00:00:00.0 UT is  JD 2451544.5 */
/* MJD         for 2000 January 1 00:00:00.0 UT is MJD   51544.0 */
/***** t=(dmjdtt-51545.d0)/36525.d0                !*** days to centuries,
 TT*/
/* *** float MJD in TT */
    t = (dmjdtt - 51544.) / 36525.;
/* *** days to centuries */
    fhr = (dmjdtt - (int) dmjdtt) * 24.;
/* scalar product of station vector with sun/moon vector */
/* *** hours in the day, */
    sprod_(&xsta[0], &xsun[0], &scs, &rsta, &rsun);
    sprod_(&xsta[0], &xmon[0], &scm, &rsta, &rmon);
    scsun = scs / rsta / rsun;
    scmon = scm / rsta / rmon;
/* computation of new h2 and l2 */
    cosphi = sqrt(xsta[0] * xsta[0] + xsta[1] * xsta[1]) / rsta;
    h2 = h20 - (1. - cosphi * 1.5 * cosphi) * 6e-4;
    l2 = l20 + (1. - cosphi * 1.5 * cosphi) * 2e-4;
/* p2-term */
    p2sun = (h2 / 2. - l2) * 3. * scsun * scsun - h2 / 2.;
    p2mon = (h2 / 2. - l2) * 3. * scmon * scmon - h2 / 2.;
/* p3-term */
/* Computing 3rd power */
    d__1 = scsun, d__2 = d__1;
    p3sun = (h3 - l3 * 3.) * 2.5 * (d__2 * (d__1 * d__1)) + (l3 - h3) * 1.5 * 
            scsun;
/* Computing 3rd power */
    d__1 = scmon, d__2 = d__1;
    p3mon = (h3 - l3 * 3.) * 2.5 * (d__2 * (d__1 * d__1)) + (l3 - h3) * 1.5 * 
            scmon;
/* term in direction of sun/moon vector */
    x2sun = l2 * 3. * scsun;
    x2mon = l2 * 3. * scmon;
    x3sun = l3 * 3. / 2. * (scsun * 5. * scsun - 1.);
    x3mon = l3 * 3. / 2. * (scmon * 5. * scmon - 1.);
/* factors for sun/moon */
    mass_ratio_sun__ = 332945.943062;
    mass_ratio_moon__ = .012300034;
    re = 6378136.55;
/* Computing 3rd power */
    d__1 = re / rsun, d__2 = d__1;
    fac2sun = mass_ratio_sun__ * re * (d__2 * (d__1 * d__1));
/* Computing 3rd power */
    d__1 = re / rmon, d__2 = d__1;
    fac2mon = mass_ratio_moon__ * re * (d__2 * (d__1 * d__1));
    fac3sun = fac2sun * (re / rsun);
    fac3mon = fac2mon * (re / rmon);
/* total displacement */
    for(i=0; i<=2; ++i) {
        dxtide[i] = fac2sun * (x2sun * xsun[i] / rsun + p2sun * xsta[i] / 
                rsta) + fac2mon * (x2mon * xmon[i] / rmon + p2mon * xsta[i] / 
                rsta) + fac3sun * (x3sun * xsun[i] / rsun + p3sun * xsta[i] / 
                rsta) + fac3mon * (x3mon * xmon[i] / rmon + p3mon * xsta[i] / 
                rsta);
    }
    zero_vec8__(xcorsta);

/* corrections for the out-of-phase part of love numbers */
/*     (part h_2^(0)i and l_2^(0)i ) */
/* first, for the diurnal band */
    st1idiu_(&xsta[0], &xsun[0], &xmon[0], &fac2sun, &fac2mon, xcorsta);
    dxtide[0] += xcorsta[0];
    dxtide[1] += xcorsta[1];
    dxtide[2] += xcorsta[2];
/* second, for the semi-diurnal band */
    st1isem_(&xsta[0], &xsun[0], &xmon[0], &fac2sun, &fac2mon, xcorsta);
    dxtide[0] += xcorsta[0];
    dxtide[1] += xcorsta[1];
    dxtide[2] += xcorsta[2];
/*** corrections for the latitude dependence of love numbers (part l^(1) ) */
    st1l1_(&xsta[0], &xsun[0], &xmon[0], &fac2sun, &fac2mon, xcorsta);
    dxtide[0] += xcorsta[0];
    dxtide[1] += xcorsta[1];
    dxtide[2] += xcorsta[2];

/* consider corrections for step 2 */
/* corrections for the diurnal band: */
/*  first, we need to know the date converted in julian centuries */
/***  this is now handled at top of code   (also convert to TT time system) */
/* **** t=(dmjd-51545.)/36525. */
/***** fhr=dmjd-int(dmjd)             !*** this is/was a buggy line (day vs. hr) */
/*  second, the diurnal band corrections, */
/*   (in-phase and out-of-phase frequency dependence): */
    step2diu_(&xsta[0], &fhr, &t, xcorsta);
    dxtide[0] += xcorsta[0];
    dxtide[1] += xcorsta[1];
    dxtide[2] += xcorsta[2];
/*  corrections for the long-period band, */
/*   (in-phase and out-of-phase frequency dependence): */
    step2lon_(&xsta[0], &fhr, &t, xcorsta);
    dxtide[0] += xcorsta[0];
    dxtide[1] += xcorsta[1];
    dxtide[2] += xcorsta[2];

/* consider corrections for step 3 */
/* ----------------------------------------------------------------------- */
/* The code below is commented to prevent restoring deformation */
/* due to permanent tide.  All the code above removes */
/* total tidal deformation with conventional Love numbers. */
/* The code above realizes a conventional tide free crust (i.e. ITRF). */
/* This does NOT conform to Resolution 16 of the 18th General Assembly */
/* of the IAG (1983).  This resolution has not been implemented by */
/* the space geodesy community in general (c.f. IERS Conventions 2003). */

/* ----------------------------------------------------------------------- */
/*** uncorrect for the permanent tide  (only if you want mean tide system)
*/
   if(tide_options & CALC_MEAN_TIDES) {
       sinphi = xsta[2] / rsta;
       cosphi = sqrt((xsta[0]*xsta[0])  + (xsta[1]*xsta[1])) / rsta;
       cosla = xsta[0] / cosphi / rsta;
       sinla = xsta[0] / cosphi / rsta;
       dr = -sqrt(5./4./PI) * h2 * 0.31460 * (3./2. * (sinphi*sinphi) - 0.5);
       dn = -sqrt(5./4./PI) * l2 * 0.31460 * 3. * cosphi*sinphi;
       dxtide[0] = dxtide[0]-dr*cosla*cosphi+dn*cosla*sinphi;
       dxtide[1] = dxtide[1]-dr*sinla*cosphi+dn*sinla*sinphi;
       dxtide[2] = dxtide[2]-dr*sinphi-dn*cosphi;
    }

    return 0;
}


/* ----------------------------------------------------------------------- */
int st1l1_(double *xsta, double *xsun, double *xmon,
           double *fac2sun, double *fac2mon, double *xcorsta)
{
    /* Initialized data */

    static double l1d = .0012;
    static double l1sd = .0024;

    /* F2C generated locals */
    double d__1, d__2, d__3, d__4;

    /* Local variables */
    static double rsta, rmon, rsun, costwola, sintwola, cosla, demon, 
                  sinla, dnmon, desun, dnsun, l1;
    static double de, dn, cosphi, sinphi;

/*** this subroutine gives the corrections induced by the latitude depende
nce*/
/* given by l^(1) in mahtews et al (1991) */
/*  input: xsta,xsun,xmon,fac3sun,fac3mon */
/* output: xcorsta */

    if(xsta == 0) return (-1);
    if(xsun == 0) return (-1); 
    if(xmon == 0) return (-1); 
    if(fac2sun == 0) return (-1); 
    if(fac2mon == 0) return (-1); 
    if(xcorsta == 0) return (-1); 

    /* Function Body */
    rsta = enorm8_(&xsta[0]);
    sinphi = xsta[2] / rsta;
/* Computing 2nd power */
    d__1 = xsta[0];
/* Computing 2nd power */
    d__2 = xsta[1];
    cosphi = sqrt(d__1 * d__1 + d__2 * d__2) / rsta;
    sinla = xsta[1] / cosphi / rsta;
    cosla = xsta[0] / cosphi / rsta;
    rmon = enorm8_(&xmon[0]);
    rsun = enorm8_(&xsun[0]);
/* for the diurnal band */
    l1 = l1d;
/* Computing 2nd power */
    d__1 = sinphi;
/* Computing 2nd power */
    d__2 = rsun;
    dnsun = -l1 * (d__1 * d__1) * *fac2sun * xsun[2] * (xsun[0] * cosla + 
            xsun[1] * sinla) / (d__2 * d__2);
/* Computing 2nd power */
    d__1 = sinphi;
/* Computing 2nd power */
    d__2 = rmon;
    dnmon = -l1 * (d__1 * d__1) * *fac2mon * xmon[2] * (xmon[0] * cosla + 
            xmon[1] * sinla) / (d__2 * d__2);
/* Computing 2nd power */
    d__1 = cosphi;
/* Computing 2nd power */
    d__2 = sinphi;
/* Computing 2nd power */
    d__3 = rsun;
    desun = l1 * sinphi * (d__1 * d__1 - d__2 * d__2) * *fac2sun * xsun[2] * (
            xsun[0] * sinla - xsun[1] * cosla) / (d__3 * d__3);
/* Computing 2nd power */
    d__1 = cosphi;
/* Computing 2nd power */
    d__2 = sinphi;
/* Computing 2nd power */
    d__3 = rmon;
    demon = l1 * sinphi * (d__1 * d__1 - d__2 * d__2) * *fac2mon * xmon[2] * (
            xmon[0] * sinla - xmon[1] * cosla) / (d__3 * d__3);
    de = (desun + demon) * 3.;
    dn = (dnsun + dnmon) * 3.;
    xcorsta[0] = -de * sinla - dn * sinphi * cosla;
    xcorsta[1] = de * cosla - dn * sinphi * sinla;
    xcorsta[2] = dn * cosphi;
/* for the semi-diurnal band */
    l1 = l1sd;
/* Computing 2nd power */
    d__1 = cosla;
/* Computing 2nd power */
    d__2 = sinla;
    costwola = d__1 * d__1 - d__2 * d__2;
    sintwola = cosla * 2. * sinla;
/* Computing 2nd power */
    d__1 = xsun[0];
/* Computing 2nd power */
    d__2 = xsun[1];
/* Computing 2nd power */
    d__3 = rsun;
    dnsun = -l1 / 2. * sinphi * cosphi * *fac2sun * ((d__1 * d__1 - d__2 * 
            d__2) * costwola + xsun[0] * 2. * xsun[1] * sintwola) / (d__3 * 
            d__3);
/* Computing 2nd power */
    d__1 = xmon[0];
/* Computing 2nd power */
    d__2 = xmon[1];
/* Computing 2nd power */
    d__3 = rmon;
    dnmon = -l1 / 2. * sinphi * cosphi * *fac2mon * ((d__1 * d__1 - d__2 * 
            d__2) * costwola + xmon[0] * 2. * xmon[1] * sintwola) / (d__3 * 
            d__3);
/* Computing 2nd power */
    d__1 = sinphi;
/* Computing 2nd power */
    d__2 = xsun[0];
/* Computing 2nd power */
    d__3 = xsun[1];
/* Computing 2nd power */
    d__4 = rsun;
    desun = -l1 / 2. * (d__1 * d__1) * cosphi * *fac2sun * ((d__2 * d__2 - 
            d__3 * d__3) * sintwola - xsun[0] * 2. * xsun[1] * costwola) / (
            d__4 * d__4);
/* Computing 2nd power */
    d__1 = sinphi;
/* Computing 2nd power */
    d__2 = xmon[0];
/* Computing 2nd power */
    d__3 = xmon[1];
/* Computing 2nd power */
    d__4 = rmon;
    demon = -l1 / 2. * (d__1 * d__1) * cosphi * *fac2mon * ((d__2 * d__2 - 
            d__3 * d__3) * sintwola - xmon[0] * 2. * xmon[1] * costwola) / (
            d__4 * d__4);
    de = (desun + demon) * 3.;
    dn = (dnsun + dnmon) * 3.;
    xcorsta[0] = xcorsta[0] - de * sinla - dn * sinphi * cosla;
    xcorsta[1] = xcorsta[1] + de * cosla - dn * sinphi * sinla;
    xcorsta[2] += dn * cosphi;
    return 0;
}


/* ----------------------------------------------------------------------- */
int step2diu_(double *xsta, double *fhr, double *t, double *xcorsta)
{
    /* Initialized data */

    static double deg2rad = .017453292519943295769;

    /* F2C generated locals */
    double d__1, d__2, d__3, d__4;

    /* Local variables */
    static double rsta, h;
    static int i, j;
    static double p, s, cosla, sinla, de, 
            dn, dr, pr, ps, thetaf, cosphi, sinphi, zla, tau, zns;

   if(xsta == 0) return (-1);
   if(fhr == 0) return (-1);
   if(t == 0) return (-1);
   if(xcorsta == 0) return (-1);

/* last change:  vd   17 may 00   1:20 pm */
/* these are the subroutines for the step2 of the tidal corrections. */

/* they are called to account for the frequency dependence */
/* of the love numbers. */

    /* Function Body */
/*** note, following table is derived from dehanttideinelMJD.f (2000oct30 16:10) */
/*** has minor differences from that of dehanttideinel.f (2000apr17 14:10) */
/*** D.M. edited to strictly follow published table 7.5a (2006aug08 13:46) */
/* cf. table 7.5a of IERS conventions 2003 (TN.32, pg.82) */
/* columns are s,h,p,N',ps, dR(ip),dR(op),dT(ip),dT(op) */
/* units of mm */
/*   data datdi/ */
/*  * -3., 0., 2., 0., 0.,-0.01,-0.01, 0.0 , 0.0, */
/*  * -3., 2., 0., 0., 0.,-0.01,-0.01, 0.0 , 0.0, */
/*  * -2., 0., 1.,-1., 0.,-0.02,-0.01, 0.0 , 0.0, */
/*  * -2., 0., 1., 0., 0.,-0.08, 0.00, 0.01, 0.01,      !*** table 7.5a
 */
/*  * -2., 2.,-1., 0., 0.,-0.02,-0.01, 0.0 , 0.0, */
/*  * -1., 0., 0.,-1., 0.,-0.10, 0.00, 0.00, 0.00,      !*** table 7.5a
 */
/*  * -1., 0., 0., 0., 0.,-0.51, 0.00,-0.02, 0.03,      !*** table 7.5a
 */
/*  * -1., 2., 0., 0., 0., 0.01, 0.0 , 0.0 , 0.0, */
/*  *  0.,-2., 1., 0., 0., 0.01, 0.0 , 0.0 , 0.0, */
/*  *  0., 0.,-1., 0., 0., 0.02, 0.01, 0.0 , 0.0, */
/*  *  0., 0., 1., 0., 0., 0.06, 0.00, 0.00, 0.00,      !*** table 7.5a
 */
/*  *  0., 0., 1., 1., 0., 0.01, 0.0 , 0.0 , 0.0, */
/*  *  0., 2.,-1., 0., 0., 0.01, 0.0 , 0.0 , 0.0, */
/*  *  1.,-3., 0., 0., 1.,-0.06, 0.00, 0.00, 0.00,      !*** table 7.5a
 */
/*  *  1.,-2., 0., 1., 0., 0.01, 0.0 , 0.0 , 0.0, */
/*  *  1.,-2., 0., 0., 0.,-1.23,-0.07, 0.06, 0.01,      !*** table 7.5a
 */
/*  *  1.,-1., 0., 0.,-1., 0.02, 0.0 , 0.0 , 0.0, */
/*  *  1.,-1., 0., 0., 1., 0.04, 0.0 , 0.0 , 0.0, */
/*  *  1., 0., 0.,-1., 0.,-0.22, 0.01, 0.01, 0.00,      !*** table 7.5a
 */
/***  *  1., 0., 0., 0., 0.,12.00,-0.78,-0.67,-0.03,      !**** table 7.5a */
/*  *  1., 0., 0., 1., 0., 1.73,-0.12,-0.10, 0.00,      !*** table 7.5a */
/*  *  1., 0., 0., 2., 0.,-0.04, 0.0 , 0.0 , 0.0, */
/*  *  1., 1., 0., 0.,-1.,-0.50,-0.01, 0.03, 0.00,      !*** table 7.5a */
/*  *  1., 1., 0., 0., 1., 0.01, 0.0 , 0.0 , 0.0, */
/***  *  1., 1., 0., 1.,-1.,-0.01, 0.0 , 0.0 , 0.0,       !*** v.dehant 2007 */
/*  *  1., 2.,-2., 0., 0.,-0.01, 0.0 , 0.0 , 0.0, */
/*  *  1., 2., 0., 0., 0.,-0.11, 0.01, 0.01, 0.00,      !*** table 7.5a */
/*  *  2.,-2., 1., 0., 0.,-0.01, 0.0 , 0.0 , 0.0, */
/*  *  2., 0.,-1., 0., 0.,-0.02, 0.02, 0.0 , 0.01, */
/*  *  3., 0., 0., 0., 0., 0.0 , 0.01, 0.0 , 0.01, */
/*  *  3., 0., 0., 1., 0., 0.0 , 0.01, 0.0 , 0.0/ */
/* Computing 3rd power */
    d__1 = *t, d__2 = d__1;
    s = *t * 481267.88194 + 218.31664563 - *t * .0014663889 * *t + d__2 * (
            d__1 * d__1) * 1.85139e-6;
/* Computing 3rd power */
    d__1 = *t, d__2 = d__1;
    tau = *fhr * 15. + 280.4606184 + *t * 36000.7700536 + *t * 3.8793e-4 * *t 
            - d__2 * (d__1 * d__1) * 2.58e-8 - s;
/* Computing 3rd power */
    d__1 = *t, d__2 = d__1;
/* Computing 4th power */
    d__3 = *t, d__3 *= d__3;
    pr = *t * 1.396971278f + *t * 3.08889e-4f * *t + d__2 * (d__1 * d__1) * 
            2.1e-8f + d__3 * d__3 * 7e-9f;
    s += pr;
/* Computing 3rd power */
    d__1 = *t, d__2 = d__1;
/* Computing 4th power */
    d__3 = *t, d__3 *= d__3;
    h = *t * 36000.7697489 + 280.46645 + *t * 3.0322222e-4 * *t + d__2 * (
            d__1 * d__1) * 2e-8f - d__3 * d__3 * 6.54e-9f;
/* Computing 3rd power */
    d__1 = *t, d__2 = d__1;
/* Computing 4th power */
    d__3 = *t, d__3 *= d__3;
    p = *t * 4069.01363525 + 83.35324312 - *t * .01032172222 * *t - d__2 * (
            d__1 * d__1) * 1.24991e-5 + d__3 * d__3 * 5.263e-8;
/* Computing 3rd power */
    d__1 = *t, d__2 = d__1;
/* Computing 4th power */
    d__3 = *t, d__3 *= d__3;
    zns = *t * 1934.13626197 + 234.95544499 - *t * .00207561111 * *t - d__2 * 
            (d__1 * d__1) * 2.13944e-6 + d__3 * d__3 * 1.65e-8;
/* Computing 3rd power */
    d__1 = *t, d__2 = d__1;
/* Computing 4th power */
    d__3 = *t, d__3 *= d__3;
    ps = *t * 1.71945766667 + 282.93734098 + *t * 4.5688889e-4 * *t - d__2 * (
            d__1 * d__1) * 1.778e-8 - d__3 * d__3 * 3.34e-9;
/* reduce angles to between 0 and 360 */
    s = d_mod(&s, &c_b56);
    tau = d_mod(&tau, &c_b56);
    h = d_mod(&h, &c_b56);
    p = d_mod(&p, &c_b56);
    zns = d_mod(&zns, &c_b56);
    ps = d_mod(&ps, &c_b56);
/* Computing 2nd power */
    d__1 = xsta[0];
/* Computing 2nd power */
    d__2 = xsta[1];
/* Computing 2nd power */
    d__3 = xsta[2];
    rsta = sqrt(d__1 * d__1 + d__2 * d__2 + d__3 * d__3);
    sinphi = xsta[2] / rsta;
/* Computing 2nd power */
    d__1 = xsta[0];
/* Computing 2nd power */
    d__2 = xsta[1];
    cosphi = sqrt(d__1 * d__1 + d__2 * d__2) / rsta;
    cosla = xsta[0] / cosphi / rsta;
    sinla = xsta[1] / cosphi / rsta;
    zla = atan2(xsta[1], xsta[0]);
    for(i=0; i<=2; ++i) {
        xcorsta[i] = 0.;
    }
    for(j=1; j<=31; ++j) {
        thetaf = (tau + datdi[j * 9 - 9] * s + datdi[j * 9 - 8] * h + datdi[j 
                * 9 - 7] * p + datdi[j * 9 - 6] * zns + datdi[j * 9 - 5] * ps)
                 * deg2rad;
        dr = datdi[j * 9 - 4] * 2. * sinphi * cosphi * sin(thetaf + zla) + 
                datdi[j * 9 - 3] * 2. * sinphi * cosphi * cos(thetaf + zla);
/* Computing 2nd power */
        d__1 = cosphi;
/* Computing 2nd power */
        d__2 = sinphi;
/* Computing 2nd power */
        d__3 = cosphi;
/* Computing 2nd power */
        d__4 = sinphi;
        dn = datdi[j * 9 - 2] * (d__1 * d__1 - d__2 * d__2) * sin(thetaf + 
                zla) + datdi[j * 9 - 1] * (d__3 * d__3 - d__4 * d__4) * cos(
                thetaf + zla);
/***** following correction by V.Dehant to match eq.16b, p.81, 2003 Co
nventions*/
/* ****   de=datdi(8,j)*sinphi*cos(thetaf+zla)+ */
        de = datdi[j * 9 - 2] * sinphi * cos(thetaf + zla) - datdi[j * 9 - 1] 
                * sinphi * sin(thetaf + zla);
        xcorsta[0] = xcorsta[0] + dr * cosla * cosphi - de * sinla - dn * 
                sinphi * cosla;
        xcorsta[1] = xcorsta[1] + dr * sinla * cosphi + de * cosla - dn * 
                sinphi * sinla;
        xcorsta[2] = xcorsta[2] + dr * sinphi + dn * cosphi;
    }
    for(i=0; i<= 2; ++i) {
        xcorsta[i] /= 1e3;
    }
    return 0;
}


/* ----------------------------------------------------------------------- */
int step2lon_(double *xsta, double *fhr, double *t, double *xcorsta)
{
    /* Initialized data */

    static double deg2rad = .017453292519943295769;
    static double datdi[45] /* was [9][5] */ = { 0.,0.,0.,1.,0.,.47,.23,
            .16,.07,0.,2.,0.,0.,0.,-.2,-.12,-.11,-.05,1.,0.,-1.,0.,0.,-.11,
            -.08,-.09,-.04,2.,0.,0.,0.,0.,-.13,-.11,-.15,-.07,2.,0.,0.,1.,0.,
            -.05,-.05,-.06,-.03 };

    /* F2C generated locals */
    double d__1, d__2, d__3;

    /* Local variables */
    static double rsta, h;
    static int i, j;
    static double p, s, cosla, sinla, de, dn, dr, pr, ps, thetaf, cosphi, 
            dn_tot__, sinphi, dr_tot__, zns;

    if(xsta == 0) return (-1);
    if(fhr == 0) return (-1);
    if(t == 0) return (-1);
    if(xcorsta == 0) return (-1);

    /* Function Body */
/* cf. table 7.5b of IERS conventions 2003 (TN.32, pg.82) */
/* columns are s,h,p,N',ps, dR(ip),dT(ip),dR(op),dT(op) */
/* IERS cols.= s,h,p,N',ps, dR(ip),dR(op),dT(ip),dT(op) */
/* units of mm */
/* Computing 3rd power */
    d__1 = *t, d__2 = d__1;
    s = *t * 481267.88194 + 218.31664563 - *t * .0014663889 * *t + d__2 * (
            d__1 * d__1) * 1.85139e-6;
/* Computing 3rd power */
    d__1 = *t, d__2 = d__1;
/* Computing 4th power */
    d__3 = *t, d__3 *= d__3;
    pr = *t * 1.396971278f + *t * 3.08889e-4f * *t + d__2 * (d__1 * d__1) * 
            2.1e-8f + d__3 * d__3 * 7e-9f;
    s += pr;
/* Computing 3rd power */
    d__1 = *t, d__2 = d__1;
/* Computing 4th power */
    d__3 = *t, d__3 *= d__3;
    h = *t * 36000.7697489 + 280.46645 + *t * 3.0322222e-4 * *t + d__2 * (
            d__1 * d__1) * 2e-8f - d__3 * d__3 * 6.54e-9f;
/* Computing 3rd power */
    d__1 = *t, d__2 = d__1;
/* Computing 4th power */
    d__3 = *t, d__3 *= d__3;
    p = *t * 4069.01363525 + 83.35324312 - *t * .01032172222 * *t - d__2 * (
            d__1 * d__1) * 1.24991e-5 + d__3 * d__3 * 5.263e-8;
/* Computing 3rd power */
    d__1 = *t, d__2 = d__1;
/* Computing 4th power */
    d__3 = *t, d__3 *= d__3;
    zns = *t * 1934.13626197 + 234.95544499 - *t * .00207561111 * *t - d__2 * 
            (d__1 * d__1) * 2.13944e-6 + d__3 * d__3 * 1.65e-8;
/* Computing 3rd power */
    d__1 = *t, d__2 = d__1;
/* Computing 4th power */
    d__3 = *t, d__3 *= d__3;
    ps = *t * 1.71945766667 + 282.93734098 + *t * 4.5688889e-4 * *t - d__2 * (
            d__1 * d__1) * 1.778e-8 - d__3 * d__3 * 3.34e-9;
/* Computing 2nd power */
    d__1 = xsta[0];
/* Computing 2nd power */
    d__2 = xsta[1];
/* Computing 2nd power */
    d__3 = xsta[2];
    rsta = sqrt(d__1 * d__1 + d__2 * d__2 + d__3 * d__3);
    sinphi = xsta[2] / rsta;
/* Computing 2nd power */
    d__1 = xsta[0];
/* Computing 2nd power */
    d__2 = xsta[1];
    cosphi = sqrt(d__1 * d__1 + d__2 * d__2) / rsta;
    cosla = xsta[0] / cosphi / rsta;
    sinla = xsta[1] / cosphi / rsta;
/* reduce angles to between 0 and 360 */
    s = d_mod(&s, &c_b56);
/* **** tau=dmod(tau,360.d0)       !*** tau not used here--09jul28 */
    h = d_mod(&h, &c_b56);
    p = d_mod(&p, &c_b56);
    zns = d_mod(&zns, &c_b56);
    ps = d_mod(&ps, &c_b56);
    dr_tot__ = 0.;
    dn_tot__ = 0.;
    for(i=0; i<= 2; ++i) {
        xcorsta[i] = 0.;
    }
/*             1 2 3 4   5   6      7      8      9 */
/* columns are s,h,p,N',ps, dR(ip),dT(ip),dR(op),dT(op) */
    for(j=1; j<=5; ++j) {
        thetaf = (datdi[j * 9 - 9] * s + datdi[j * 9 - 8] * h + datdi[j * 9 - 
                7] * p + datdi[j * 9 - 6] * zns + datdi[j * 9 - 5] * ps) * 
                deg2rad;
/* Computing 2nd power */
        d__1 = sinphi;
/* Computing 2nd power */
        d__2 = sinphi;
        dr = datdi[j * 9 - 4] * (d__1 * d__1 * 3. - 1.) / 2.f * cos(thetaf) + 
                datdi[j * 9 - 2] * (d__2 * d__2 * 3. - 1.) / 2.f * sin(thetaf)
                ;
        dn = datdi[j * 9 - 3] * (cosphi * sinphi * 2.) * cos(thetaf) + datdi[
                j * 9 - 1] * (cosphi * sinphi * 2.) * sin(thetaf);
        de = 0.;
        dr_tot__ += dr;
        dn_tot__ += dn;
        xcorsta[0] = xcorsta[0] + dr * cosla * cosphi - de * sinla - dn * 
                sinphi * cosla;
        xcorsta[1] = xcorsta[1] + dr * sinla * cosphi + de * cosla - dn * 
                sinphi * sinla;
        xcorsta[2] = xcorsta[2] + dr * sinphi + dn * cosphi;
    }
    for(i=0; i<= 2; ++i) {
        xcorsta[i] /= 1e3;
    }
    return 0;
}


/* ----------------------------------------------------------------------- */
int st1idiu_(double *xsta, double *xsun, double *xmon,
             double *fac2sun, double *fac2mon, double *xcorsta)
{
    /* Initialized data */

    static double dhi = -.0025;
    static double dli = -7e-4;

    /* F2C generated locals */
    double d__1, d__2;

    /* Local variables */
    static double rsta, rmon, rsun, cosla, demon, sinla, dnmon, desun, 
            drmon, dnsun, drsun;
    static double de, dn, dr, cosphi, sinphi, cos2phi;

    if(xsta == 0) return (-1);
    if(xsun == 0) return (-1);
    if(xmon == 0) return (-1);
    if(fac2sun == 0) return (-1);
    if(fac2mon == 0) return (-1);
    if(xcorsta == 0) return (-1);

/* this subroutine gives the out-of-phase corrections induced by */
/* mantle inelasticity in the diurnal band */
/*  input: xsta,xsun,xmon,fac2sun,fac2mon */
/* output: xcorsta */

    /* Function Body */
    rsta = enorm8_(&xsta[0]);
    sinphi = xsta[2] / rsta;
/* Computing 2nd power */
    d__1 = xsta[0];
/* Computing 2nd power */
    d__2 = xsta[1];
    cosphi = sqrt(d__1 * d__1 + d__2 * d__2) / rsta;
/* Computing 2nd power */
    d__1 = cosphi;
/* Computing 2nd power */
    d__2 = sinphi;
    cos2phi = d__1 * d__1 - d__2 * d__2;
    sinla = xsta[1] / cosphi / rsta;
    cosla = xsta[0] / cosphi / rsta;
    rmon = enorm8_(&xmon[0]);
    rsun = enorm8_(&xsun[0]);
/* Computing 2nd power */
    d__1 = rsun;
    drsun = dhi * -3. * sinphi * cosphi * *fac2sun * xsun[2] * (xsun[0] * 
            sinla - xsun[1] * cosla) / (d__1 * d__1);
/* Computing 2nd power */
    d__1 = rmon;
    drmon = dhi * -3. * sinphi * cosphi * *fac2mon * xmon[2] * (xmon[0] * 
            sinla - xmon[1] * cosla) / (d__1 * d__1);
/* Computing 2nd power */
    d__1 = rsun;
    dnsun = dli * -3. * cos2phi * *fac2sun * xsun[2] * (xsun[0] * sinla - 
            xsun[1] * cosla) / (d__1 * d__1);
/* Computing 2nd power */
    d__1 = rmon;
    dnmon = dli * -3. * cos2phi * *fac2mon * xmon[2] * (xmon[0] * sinla - 
            xmon[1] * cosla) / (d__1 * d__1);
/* Computing 2nd power */
    d__1 = rsun;
    desun = dli * -3. * sinphi * *fac2sun * xsun[2] * (xsun[0] * cosla + xsun[
            1] * sinla) / (d__1 * d__1);
/* Computing 2nd power */
    d__1 = rmon;
    demon = dli * -3. * sinphi * *fac2mon * xmon[2] * (xmon[0] * cosla + xmon[
            1] * sinla) / (d__1 * d__1);
    dr = drsun + drmon;
    dn = dnsun + dnmon;
    de = desun + demon;
    xcorsta[0] = dr * cosla * cosphi - de * sinla - dn * sinphi * cosla;
    xcorsta[1] = dr * sinla * cosphi + de * cosla - dn * sinphi * sinla;
    xcorsta[2] = dr * sinphi + dn * cosphi;
    return 0;
}


/* ----------------------------------------------------------------------- */
int st1isem_(double *xsta, double *xsun, double *xmon,
             double *fac2sun, double *fac2mon, double *xcorsta)
{
    /* Initialized data */

    static double dhi = -.0022;
    static double dli = -7e-4;

    /* F2C generated locals */
    double d__1, d__2, d__3, d__4;

    /* Local variables */
    static double rsta, rmon, rsun, costwola, sintwola, cosla, demon, 
                  sinla, dnmon, desun, drmon, dnsun, drsun;
    static double de, dn, dr, cosphi, sinphi;

    if(xsta == 0) return (-1);
    if(xsun == 0) return (-1);
    if(xmon == 0) return (-1);
    if(fac2sun == 0) return (-1);
    if(fac2mon == 0) return (-1);
    if(xcorsta == 0) return (-1);

/* this subroutine gives the out-of-phase corrections induced by */
/* mantle inelasticity in the diurnal band */
/*  input: xsta,xsun,xmon,fac2sun,fac2mon */
/* output: xcorsta */

    /* Function Body */
    rsta = enorm8_(&xsta[0]);
    sinphi = xsta[2] / rsta;
/* Computing 2nd power */
    d__1 = xsta[0];
/* Computing 2nd power */
    d__2 = xsta[1];
    cosphi = sqrt(d__1 * d__1 + d__2 * d__2) / rsta;
    sinla = xsta[1] / cosphi / rsta;
    cosla = xsta[0] / cosphi / rsta;
/* Computing 2nd power */
    d__1 = cosla;
/* Computing 2nd power */
    d__2 = sinla;
    costwola = d__1 * d__1 - d__2 * d__2;
    sintwola = cosla * 2. * sinla;
    rmon = enorm8_(&xmon[0]);
    rsun = enorm8_(&xsun[0]);
/* Computing 2nd power */
    d__1 = cosphi;
/* Computing 2nd power */
    d__2 = xsun[0];
/* Computing 2nd power */
    d__3 = xsun[1];
/* Computing 2nd power */
    d__4 = rsun;
    drsun = dhi * -.75 * (d__1 * d__1) * *fac2sun * ((d__2 * d__2 - d__3 * 
            d__3) * sintwola - xsun[0] * 2.f * xsun[1] * costwola) / (d__4 * 
            d__4);
/* Computing 2nd power */
    d__1 = cosphi;
/* Computing 2nd power */
    d__2 = xmon[0];
/* Computing 2nd power */
    d__3 = xmon[1];
/* Computing 2nd power */
    d__4 = rmon;
    drmon = dhi * -.75 * (d__1 * d__1) * *fac2mon * ((d__2 * d__2 - d__3 * 
            d__3) * sintwola - xmon[0] * 2.f * xmon[1] * costwola) / (d__4 * 
            d__4);
/* Computing 2nd power */
    d__1 = xsun[0];
/* Computing 2nd power */
    d__2 = xsun[1];
/* Computing 2nd power */
    d__3 = rsun;
    dnsun = dli * 1.5 * sinphi * cosphi * *fac2sun * ((d__1 * d__1 - d__2 * 
            d__2) * sintwola - xsun[0] * 2. * xsun[1] * costwola) / (d__3 * 
            d__3);
/* Computing 2nd power */
    d__1 = xmon[0];
/* Computing 2nd power */
    d__2 = xmon[1];
/* Computing 2nd power */
    d__3 = rmon;
    dnmon = dli * 1.5 * sinphi * cosphi * *fac2mon * ((d__1 * d__1 - d__2 * 
            d__2) * sintwola - xmon[0] * 2. * xmon[1] * costwola) / (d__3 * 
            d__3);
/* Computing 2nd power */
    d__1 = xsun[0];
/* Computing 2nd power */
    d__2 = xsun[1];
/* Computing 2nd power */
    d__3 = rsun;
    desun = dli * -1.5 * cosphi * *fac2sun * ((d__1 * d__1 - d__2 * d__2) * 
            costwola + xsun[0] * 2.f * xsun[1] * sintwola) / (d__3 * d__3);
/* Computing 2nd power */
    d__1 = xmon[0];
/* Computing 2nd power */
    d__2 = xmon[1];
/* Computing 2nd power */
    d__3 = rmon;
    demon = dli * -1.5 * cosphi * *fac2mon * ((d__1 * d__1 - d__2 * d__2) * 
            costwola + xmon[0] * 2. * xmon[1] * sintwola) / (d__3 * d__3);
    dr = drsun + drmon;
    dn = dnsun + dnmon;
    de = desun + demon;
    xcorsta[0] = dr * cosla * cosphi - de * sinla - dn * sinphi * cosla;
    xcorsta[1] = dr * sinla * cosphi + de * cosla - dn * sinphi * sinla;
    xcorsta[2] = dr * sinphi + dn * cosphi;
    return 0;
}


/* ----------------------------------------------------------------------- */
int sprod_(double *x, double *y, double *scal, double *r1, double *r2)
{

   if(x == 0) return (-1); 
   if(y == 0) return (-1); 
   if(scal == 0) return (-1); 
   if(r1 == 0) return (-1); 
   if(r2 == 0) return (-1); 

/*  computation of the scalar-product of two vectors and their norms */

/*  input:   x(i),i=1,2,3  -- components of vector x */
/*           y(i),i=1,2,3  -- components of vector y */
/*  output:  scal          -- scalar product of x and y */
/*           r1,r2         -- lengths of the two vectors x and y */

    /* Function Body */
    *r1 = sqrt(x[0] * x[0] + x[1] * x[1] + x[2] * x[2]);
    *r2 = sqrt(y[0] * y[0] + y[1] * y[1] + y[2] * y[2]);
    *scal = x[0] * y[0] + x[1] * y[1] + x[2] * y[2];
    return 0;
}


/* ----------------------------------------------------------------------- */
double enorm8_(double *a)
{
    /* F2C generated locals */
    double ret_val;
    if(a == 0) return 0.0;  // !!!!!!

    /* compute euclidian norm of a vector (of length 3) */

    /* Function Body */
    ret_val = sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
    return ret_val;
}

/* ----------------------------------------------------------------------- */

int zero_vec8__(double *v)
{
    /* initialize a vector (of length 3) to zero */
    /* Function Body */
    if(v == 0) return (-1); 

    v[0] = 0.;
    v[1] = 0.;
    v[2] = 0.;
    return 0;
} 

int moonxyz_(int *mjd, double *fmjd, double *rm)
{
    /* F2C generated locals */
    double d__1;

    /* Local variables */
    static double ghar, d, f, q, t, oblir, tjdtt, t1, t2, t3, el;
    static double cselat, selatd, cselon, selond, fmjdtt, sselat, el0, 
                  sselon, tsectt, rm1, rm2, rm3, elp, rse;
    static double tsecgps;

    if(mjd == 0) return (-1); 
    if(fmjd == 0) return (-1); 
    if(rm == 0) return (-1); 

/* get low-precision, geocentric coordinates for moon (ECEF) */
/*** input:  mjd/fmjd, is Modified Julian Date (and fractional) in GPS time*/
/* output: rm, is geocentric lunar position vector [m] in ECEF */
/*** 1."satellite orbits: models, methods, applications" montenbruck & gill(2000)*/
/* section 3.3.2, pg. 72-73 */
/*** 2."astronomy on the personal computer, 4th ed." montenbruck & pfleger (2005)*/
/* section 3.2, pg. 38-39  routine MiniMoon */
/* use TT for lunar ephemerides */

    /* Function Body */
    if((tide_options & LOW_RES_TIDES) == 0) {  // use more accurate values from moon_posn()
       selatd = moon_lat;
       selond = moon_lon;
       rse = moon_dist;
       oblir = obliquity(jd_utc, 1);
    }
    else {
       tsecgps = *fmjd * 86400.;
   /* *** GPS time (sec of */
       tsectt = gps2tt_(&tsecgps);
   /* *** TT  time (sec of */
       fmjdtt = tsectt / 86400.;
   /* julian centuries since 1.5 january 2000 (J2000) */
   /*   (note: also low precision use of mjd --> tjd) */
   /* *** TT  time (fract. */
       tjdtt = *mjd + fmjdtt + 2400000.5;
   /* *** Julian Date, TT */
       t = (tjdtt - 2451545.) / 36525.;
   /* el0 -- mean longitude of Moon (deg) */
   /* el  -- mean anomaly of Moon (deg) */
   /* elp -- mean anomaly of Sun  (deg) */
   /* f   -- mean angular distance of Moon from ascending node (deg) */
   /* d   -- difference between mean longitudes of Sun and Moon (deg) */
   /* equations 3.47, p.72 */
   /* *** julian centuries, */
       el0 = t * 481267.88088 + 218.31617 - t * 1.3972f;
       el = t * 477198.86753 + 134.96292;
       elp = t * 35999.04944 + 357.52543;
       f = t * 483202.01873 + 93.27283;
       d = t * 445267.11135 + 297.85027;
   /* longitude w.r.t. equinox and ecliptic of year 2000 */
       selond = el0 + sin(el / stuff_1.rad) * 6.288888888888889 + sin((el + el) /
                stuff_1.rad) * .2136111111111111 - sin((el - d - d) / 
               stuff_1.rad) * 1.273888888888889 + sin((d + d) / stuff_1.rad) * 
               .6583333333333333 - sin(elp / stuff_1.rad) * .1855555555555556 - 
               sin((f + f) / stuff_1.rad) * .1144444444444444 - sin((el + el - d 
               - d) / stuff_1.rad) * .05888888888888889 - sin((el + elp - d - d) 
               / stuff_1.rad) * .05722222222222222 + sin((el + d + d) / 
               stuff_1.rad) * .05333333333333334 - sin((elp - d - d) / 
               stuff_1.rad) * .04583333333333333 + sin((el - elp) / stuff_1.rad) 
               * .04111111111111111 - sin(d / stuff_1.rad) * .03472222222222222 
               - sin((el + elp) / stuff_1.rad) * .03055555555555555 - sin((f + f 
               - d - d) / stuff_1.rad) * .01527777777777778;
   /* latitude w.r.t. equinox and ecliptic of year 2000 */
   /* *** eq 3.48, p.72 */
       q = sin((f + f) / stuff_1.rad) * .1144444444444444 + sin(elp / 
               stuff_1.rad) * .1502777777777778;
   /* *** temporary ter */
       selatd = sin((f + selond - el0 + q) / stuff_1.rad) * 5.144444444444445 - 
               sin((f - d - d) / stuff_1.rad) * .1461111111111111 + sin((el + f 
               - d - d) / stuff_1.rad) * .01222222222222222 - sin((-el + f - d - 
               d) / stuff_1.rad) * .008611111111111111 - sin((-el - el + f) / 
               stuff_1.rad) * .006944444444444444 - sin((elp + f - d - d) / 
               stuff_1.rad) * .006388888888888889 + sin((-el + f) / stuff_1.rad) 
               * .005833333333333334 + sin((-elp + f - d - d) / stuff_1.rad) * 
               .003055555555555556;
   /* distance from Earth center to Moon (m) */
   /* *** eq 3.49, p.72 */
       rse = 3.85e8 - cos(el / stuff_1.rad) * 2.0905e7 - cos((d + d - el) / 
               stuff_1.rad) * 3.699e6 - cos((d + d) / stuff_1.rad) * 2.956e6 - 
               cos((el + el) / stuff_1.rad) * 5.7e5 + cos((el + el - d - d) / 
               stuff_1.rad) * 2.46e5 - cos((elp - d - d) / stuff_1.rad) * 2.05e5 
               - cos((el + d + d) / stuff_1.rad) * 1.71e5 - cos((el + elp - d - 
               d) / stuff_1.rad) * 1.52e5;
       selond += t * 1.3972;
       oblir = 23.43929111 / stuff_1.rad;
    }
//sprintf(debug_text2, "seella:   %.9f %.9f %.9f", fmod(selatd+360.0, 360.0),fmod(selond,360.0), rse);
/* convert spherical ecliptic coordinates to equatorial cartesian */
/* precession of equinox wrt. J2000   (p.71) */
/* *** eq 3.50, p.72 */


/*** position vector of moon (mean equinox & ecliptic of J2000) (EME2000, ICRF)*/
/***                         (plus long. advance due to precession -- eq. above)*/
/* *** degrees */
/* *** obliquity of the J2000 eclipti */
    sselat = sin(selatd / stuff_1.rad);
    cselat = cos(selatd / stuff_1.rad);
    sselon = sin(selond / stuff_1.rad);
    cselon = cos(selond / stuff_1.rad);
    t1 = rse * cselon * cselat;
/* *** meters          !*** eq. 3.51, */
    t2 = rse * sselon * cselat;
/* *** meters          !*** eq. 3.51, */
    t3 = rse * sselat;
//sprintf(debug_text3, "xyzt: %.9f %.9f %.9f  rse:%.9f", t1,t2,t3, rse);
/* *** meters          !*** eq. 3.51, */
    d__1 = -oblir;
    rot1_(&d__1, &t1, &t2, &t3, &rm1, &rm2, &rm3);
/* convert position vector of moon to ECEF  (ignore polar motion/LOD) 
*/
/* *** eq. 3.51, */
    getghar_(mjd, fmjd, &ghar);
/* *** sec 2.3.1, */
    rot3_(&ghar, &rm1, &rm2, &rm3, &rm[0], &rm[1], &rm[2]);
/* *** eq. 2.89, */
    return 0;
} 


/*********************************************************************************/
int getghar_(int *mjd, double *fmjd, double *ghar)
{
    static double ghad, d;
    static int i;
    static double fmjdutc, tsecgps, tsecutc;

    if(mjd == 0) return (-1); 
    if(fmjd == 0) return (-1); 
    if(ghar == 0) return (-1); 

/* convert mjd/fmjd in GPS time to Greenwich hour angle (in radians) */

/*** "satellite orbits: models, methods, applications" montenbruck & gill(2000) */
/* section 2.3.1, pg. 33 */
/*** need UT to get sidereal time  ("astronomy on the personal computer", 4th ed) 2005) */
    tsecgps = *fmjd * 86400.;
/* *** GPS time (sec of */
    tsecutc = gps2utc_(&tsecgps);
/* *** UTC time (sec of */
    fmjdutc = tsecutc / 86400.;
/* **** d = MJD - 51544.5d0                               !*** footnote */

/* *** UTC time (fract. */
    d = *mjd - 51544 + (fmjdutc - .5);
/* greenwich hour angle for J2000  (12:00:00 on 1 Jan 2000) */
/***** ghad = 100.46061837504d0 + 360.9856473662862d0*d  !*** eq. 2.85 (+d
igits)*/
/* *** days since */
    ghad = d * 360.9856473662862 + 280.46061837504;
/* *** normalize to 0-360 and convert to radians */
/* *** corrn.   (+ */
    i = (int) (ghad / 360.);
    *ghar = (ghad - i * 360.) / stuff_1.rad;
    while(*ghar > stuff_1.pi2) {
       *ghar -= stuff_1.pi2;
    }
    while(*ghar < 0.) {
       *ghar += stuff_1.pi2;
    }
    return 0;
} 


/***************************************************************************/
int sunxyz_(int *mjd, double *fmjd, double *rs)
{
    /* Local variables */
    static double cobe, ghar, sobe, opod, slon, emdeg, r, t, cslon, slond,
                  tjdtt, sslon, em;
    static double fmjdtt, em2, tsectt, rs1, rs2, rs3, obe;
    static double tsecgps;

    if(mjd == 0) return (-1); 
    if(fmjd == 0) return (-1); 
    if(rs == 0) return (-1); 

    /* get low-precision, geocentric coordinates for sun (ECEF) */
    /*** input, mjd/fmjd, is Modified Julian Date (and fractional) in GPS time */
    /* output, rs, is geocentric solar position vector [m] in ECEF */
    /*** 1."satellite orbits: models, methods, applications" montenbruck & gill(2000) */
    /* section 3.3.2, pg. 70-71 */
    /*** 2."astronomy on the personal computer, 4th ed." montenbruck & pfleger (2005) */
    /* section 3.2, pg. 39  routine MiniSun */
    /* mean elements for year 2000, sun ecliptic orbit wrt. Earth */

    /* Function Body */
    if((tide_options & LOW_RES_TIDES) == 0) {  // use more accurate values from moon_posn()
       slond = sun_hlon;
       r = earth_sun_dist(jd_utc) * 1000.0;
       obe = obliquity(jd_utc, 1);
       cobe = cos(obe);
       sobe = sin(obe);
    }
    else {
       obe = 23.43929111 / stuff_1.rad;
       /* *** obliquity of the J2000 ecliptic */
       sobe = sin(obe);
       cobe = cos(obe);
       opod = 282.94;

       /* use TT for solar ephemerides */
       /* *** RAAN + arg.peri.  (deg.) */
       tsecgps = *fmjd * 86400.;
       /* *** GPS time (sec of */
       tsectt = gps2tt_(&tsecgps);
       /* *** TT  time (sec of */
       fmjdtt = tsectt / 86400.;
       /* julian centuries since 1.5 january 2000 (J2000) */
       /*   (note: also low precision use of mjd --> tjd) */
       /* *** TT  time (fract. */
       tjdtt = *mjd + fmjdtt + 2400000.5;
       /* *** Julian Date, TT */
       t = (tjdtt - 2451545.) / 36525.;
       /* *** julian centuries, */
       emdeg = t * 35999.049 + 357.5256;
       /* *** degrees */
       em = emdeg / stuff_1.rad;
       /* *** radians */
       em2 = em + em;
       /* series expansions in mean anomaly, em   (eq. 3.43, p.71) */
       /* *** radians */
       r = (149.619 - cos(em) * 2.499 - cos(em2) * .021) * 1.0e9;
       /* *** m. */
       slond = opod + emdeg + (sin(em) * 6892. + sin(em2) * 72.) / 3600.;
       /* precession of equinox wrt. J2000   (p.71) */
       slond += t * 1.3972;
    }

    /*** position vector of sun (mean equinox & ecliptic of J2000) (EME2000, ICRF)*/
    /***                        (plus long. advance due to precession -- eq. above)*/
    /* *** degrees */

    slon = slond / stuff_1.rad;
//sprintf(plot_title, "slond:%.9f  r:%.9f  sunhlon:%.9f  erad:%.9f  mjd:%d fmjd:%f", 
//fmod(slond,360.0),r, sun_hlon,earth_sun_dist(jd_utc)*1000.0, *mjd,*fmjd);
    /* *** radians */
    sslon = sin(slon);
    cslon = cos(slon);
    rs1 = r * cslon;
    /* *** meters             !*** eq. 3.46, */
    rs2 = r * sslon * cobe;
    /* *** meters             !*** eq. 3.46, */
    rs3 = r * sslon * sobe;
    /* convert position vector of sun to ECEF  (ignore polar motion/LOD) */

    /* *** meters             !*** eq. 3.46, */
    getghar_(mjd, fmjd, &ghar);
    /* *** sec 2.3.1, */
    rot3_(&ghar, &rs1, &rs2, &rs3, &rs[0], &rs[1], &rs[2]);
    /* *** eq. 2.89, */
    return 0;
} 


/***************************************************************************/
int lhsaaz_(double *u, double *v, double *w, 
        double *ra, double *az, double *va)
{
    /* Local variables */
    static double s, r2, s2;

    if(u == 0) return (-1); 
    if(v == 0) return (-1); 
    if(w == 0) return (-1); 
    if(ra == 0) return (-1); 
    if(az == 0) return (-1); 
    if(va == 0) return (-1); 

    /* determine range,azimuth,vertical angle from local horizon coord. */
    s2 = *u * *u + *v * *v;
    r2 = s2 + *w * *w;
    s = sqrt(s2);
    *ra = sqrt(r2);
    *az = atan2(*v, *u);
    *va = atan2(*w, s);
    return 0;
} 

/* ----------------------------------------------------------------------- */
int geoxyz_(double *gla, double *glo, double *eht, double *x, double *y, double *z)
{
    /* Local variables */
    static double w, w2, en, cla, sla;

    /* convert geodetic lat, long, ellip ht. to x,y,z */
    sla = sin(*gla);
    cla = cos(*gla);
    w2 = 1. - eccSquared * sla * sla;
    w = sqrt(w2);
    en = WGS84_A / w;
    *x = (en + *eht) * cla * cos(*glo);
    *y = (en + *eht) * cla * sin(*glo);
    *z = (en * (1. - eccSquared) + *eht) * sla;
    return 0;
}

/* ----------------------------------------------------------------------- */
int rge_(double *gla, double *glo, double *u, 
        double *v, double *w, double *x, double *y, double *z)
{
    /* Local variables */
    static double cb, cl, sb, sl;

    /* given a rectangular cartesian system (x,y,z) */
    /* compute a geodetic h cartesian sys   (u,v,w) */
    sb = sin(*gla);
    cb = cos(*gla);
    sl = sin(*glo);
    cl = cos(*glo);
    *u = -sb * cl * *x - sb * sl * *y + cb * *z;
    *v = -sl * *x + cl * *y;
    *w = cb * cl * *x + cb * sl * *y + sb * *z;
    return 0;
}

/* ----------------------------------------------------------------------- */
int rot1_(double *theta, double *x, double *y, 
        double *z, double *u, double *v, double *w)
{
    /* Local variables */
    static double c, s;

    /* rotate coordinate axes about 1 axis by angle of theta radians */
    /* x,y,z transformed into u,v,w */
    s = sin(*theta);
    c = cos(*theta);
    *u = *x;
    *v = c * *y + s * *z;
    *w = c * *z - s * *y;
    return 0;
}


/* ----------------------------------------------------------------------- */
int rot3_(double *theta, double *x, double *y, 
        double *z, double *u, double *v, double *w)
{
    /* Local variables */
    static double c, s;

    /* rotate coordinate axes about 3 axis by angle of theta radians */
    /* x,y,z transformed into u,v,w */

    s = sin(*theta);
    c = cos(*theta);
    *u = c * *x + s * *y;
    *v = c * *y - s * *x;
    *w = *z;
    return 0;
} 



/* *********************************************************************** */
/* supplemental time functions **************************************** */
/* *********************************************************************** */
double gps2tt_(double *tsec)
{
    /* F2C generated locals */
    double ret_val;

    if(tsec == 0) return (-1); 

    /* convert tsec in GPS to tsec in TT */
    ret_val = *tsec + 51.184;
    /* *** fixed offset */
    return ret_val;
}


double gps2utc_(double *tsec)
{
    /* F2C generated locals */
    double ret_val;

    if(tsec == 0) return (-1);

    /* convert tsec in GPS to tsec in UTC */
    /* GPS is ahead of UTC  (c.f. USNO) */
    /* UTC is behind GPS */
    /* gpsleap() is (so far) positive (and increasing) */
    /* so, must subtract gpsleap from GPS to get UTC */

    ret_val = *tsec - utc_offset;
    return ret_val;
}


double grav_force(double jd)
{
double d;

   // jd is utc time
   d = 0.0;
   if(lat && lon) {  // only do this if lat/lon available
      d = calc_gals(jd, lat*180.0/PI, lon*180.0/PI, alt);

      calc_earth_tide(jd + jtime(0,0,utc_offset,0.0));  // uses GPS time
      d = alt_tide;
   }

   return d;
}


//
//
//   Calendar conversions
//
//

void easter(int y)
{
int g, c, d, h, k, i, j;
int m;
double jd;

    // Oudin's algorithm for calculating easter
    g = y % 19;
    c = y / 100;
    d = c - c/4;
    h = (d - ((8*c+13)/25) + (19*g) + 15) % 30;
    k = h / 28;
    i = h - k*(1 - k*(29/(h+1))*(21-g)/11);
    j = (y + y/4 + i + 2 - d) % 7;
    d = 28 + i - j;

    if(d > 31) {
       d -= 31;
       m = 4;
    }
    else m = 3;

    set_holiday(EASTER, m, d);

    // calculate Ash Wednesday
    jd = jdate(y, m, d);
    gregorian(0, jd-46.0);
    set_holiday(ASH, g_month, g_day);

    gregorian(0, jd-47.0);
    set_holiday(MARDI_GRAS, g_month, g_day);

    // calculate Palm Sunday
    gregorian(0, jd-7.0);
    set_holiday(PALM_SUNDAY, g_month, g_day);

    // calculate Good Friday
    gregorian(0, jd-2.0);
    set_holiday(GOOD_FRIDAY, g_month, g_day);
}

double rosh_jd(int y)
{
int g;
double r;
double jd;
int i, j;
int day, month;

   // return Julian date of Rosh Hashanha
   g = (y % 19) + 1;
   j = (12 * g) % 19;
   i = (y/100) - (y/400) - 2;
   r = (double) (int) (y%4);
   r = (765433.0/492480.0*(double) j) + (r/4.0) - ((313.0 * ((double) y) + 89081.0)/98496.0);
   r += (double) i;
// r = 6.057778996 + (1.554241797*(double) j) + 0.25*r - 0.003177794*(double)y,

   day = (int) r;
   r -= (double) day;
   month = 9;
   if(day > 30) { day-=30; ++month; }

   i = day_of_week(day, month, y);
   jd = jdate(y, month, day);

   // apply the postponements
   if((i == 0) || (i == 3) || (i == 5)) {
      jd += 1.0;
   }
   else if(i == 1) {
      if((r >= 23269.0/25920.0) && (j > 6)) jd += 1.0;
   }
   else if(i == 2) {
      if((r >= 1367.0/2160.0) && (j > 6)) jd += 2.0;
   }
   return jd;
}

void init_hebrew(int year)
{
double t;
double rosh;

   rosh = rosh_jd(year);        // our reference is Rosh Hashahna 
                                // once you know Rosh Hashahna,  all is revealed

   t = rosh_jd(year+1) - rosh;  // number of days in the Jewish year

   jtimes[9] = rosh;
   jtimes[10] = jtimes[9]+30.0; // che
   if((DABS(t-355.0) < 0.1) || (DABS(t-385.0) < 0.1)) {  // shalim years
      rosh += 1.0;  // kis - the month of Cheshvan has an extra day in shalim years
      jtimes[11] = jtimes[10]+30.0;
   }
   else jtimes[11] = jtimes[10]+29.0;

   if((DABS(t-353.0) < 0.1) || (DABS(t-383.0) < 0.1)) {  // hasser years
      jtimes[12] = jtimes[11]+29.0; // tev - Kislev has one less day in hasser years
   }
   else jtimes[12] = jtimes[11]+30.0;
   jtimes[13] = jtimes[12]+29.0;    // shv
   jtimes[14] = jtimes[13]+30.0;    // end of shv
   jtimes[8]  = jtimes[9]-29.0;     // elu
   jtimes[7]  = jtimes[8]-30.0;     // av
   jtimes[6]  = jtimes[7]-29.0;     // tam
   jtimes[5]  = jtimes[6]-30.0;     // siv
   jtimes[4]  = jtimes[5]-29.0;     // iyr
   jtimes[3]  = jtimes[4]-30.0;     // nis
   jtimes[2]  = jtimes[3]-29.0;     // adr
   if(DABS(t) < 370.0) {            // previous year not a leap year
      jtimes[1]  = jtimes[2]-30.0;  // Adar I in leap years
      jmonths[1] = "Ad1";
      jmonths[2] = "Ad2";
   }
   else {                           // previous year was a leap year, this one is not
      jtimes[1] = 0.0F;             // no Adar I
      jtimes[1]  = jtimes[2]-30.0;     
      jmonths[1] = "Shv";
      jmonths[2] = "Adr";
   }
   jtimes[0] = jtimes[1]-29.0;
}

void islam_to_greg(int h, int m, int d)
{
long jd, al, be, b;
long c, d1, e1;

    // convert Islamic date to gregorian
    long n = (59*(m-1)+1)/2 + d;
    long q = h / 30;
    long r = h - q*30;
    long a = (11*r + 3) / 30;
    long w = 404*q + 354*r + 208+a;
    long q1 = w / 1461;
    long q2 = w - 1461*q1;
    long g = 621 + 4*(7*q+q1);
    long k = (long) (q2 / SOLAR_YEAR);
    long e = (long) (k * SOLAR_YEAR);
    long j = q2 - e + n - 1;
    long x = g + k;
    
    if ((j > 366) && !(x&3)) { j -= 366; ++x; }
    if ((j > 365) && (x&3))  { j -= 365; ++x; }
    
    jd = (long) (365.25*(x-1));
    jd += 1721423 + j;
    al = (long) ((jd - 1867216.25) / 36524.25);
    be = jd + 1 + al - al/4;
    if (jd < 2299161) be = jd;
    b = be + 1524;
    c = (long) ((b - 122.1) / 365.25);
    d1 = (long) (365.25 * c);
    e1 = (long) ((b - d1) / 30.6001);
    
    g_day = b - d1;
    g_day -= (long) floor(30.6001*e1);
    if(e1 < 14) g_month = e1 - 1;
    else        g_month = e1 - 13;
    
    if(g_month > 2) g_year = c - 4716;
    else            g_year = c - 4715;
    return;
}

int get_islam_day(int yy, int mm, int dd)
{
// islam_to_greg(yy-581, mm, dd);
// if(g_year == yy) return 581;

   islam_to_greg(yy-580, mm, dd);
   if(g_year == yy) return 580;

   islam_to_greg(yy-579, mm, dd);
   if(g_year == yy) return 579; // not in this year, try next year

   islam_to_greg(yy-578, mm, dd);
   if(g_year == yy) return 578; // not in this year, try next year

   return 0;
}

void init_islamic(int year)
{
int mm;
int iyear;

   // calculate start of each Islamic month
   iyear = year - 578;
   while(iyear > 0) {  // find Islamic year that starts on or before this gregorian year
      islam_to_greg(iyear, 1, 1);
      if(g_year < this_year) break;
      else if((g_year == this_year) && (g_month == 1) && (g_day == 1)) break;
      --iyear;
   }

   for(mm=0; mm<=35; mm++) {  // find start of the next 36 islam months
      islam_to_greg(iyear+(mm/12), (mm%12)+1, 1);
      myears[mm] = iyear+(mm/12);
      mtimes[mm] = jdate(g_year, g_month, g_day);
   }
}

void init_druid(int year)
{
double jd;
int mm;

   jd = jdate(year-1, 12, 24);  // Druid year starts on 24 Dec
   jd += (double) druid_epoch;  // kludge to allow tweak to start of calendar
   for(mm=0; mm<=13; mm++) {    // the Juilan time of the start of each month
      dyears[mm] = year;        // assume druid year is the Gregorian year
      dtimes[mm] = jd;          // ... that holds most of the Druid year
      jd += 28.0;               // Druid months are 28 days long
   }

   dtimes[14] = jdate(year, 12, 24);
   dtimes[14] += (double) druid_epoch; 
   dyears[14] = year+1;
   dtimes[15] = dtimes[14]+28.0;
   dyears[15] = year+1;
}


void init_persian(int year)
{
double jd;
int leap;
int mm;

// gregorian(0, spring-365.24238426);    // time of spring in GMT of last year;
// jd = jdate(g_year, g_month, g_day);   // jdate of start of spring
// if((jd - (3.5/24.0) + (12.0/24.0)) > spring) {  // it's after noon in Tehran
//    jd += 1.0;  //!!! kludge - we should be using solar noon in Tehran
// }

   leap = year-621-1;               // start calculating from last year
   leap %= 33;                      // leap years on 33 year cycles
   if     (leap == 1)  leap = 20;   // leap years start on 20 March
   else if(leap == 5)  leap = 20;
   else if(leap == 9)  leap = 20;
   else if(leap == 13) leap = 20;
   else if(leap == 17) leap = 20;
   else if(leap == 22) leap = 20;
   else if(leap == 26) leap = 20;
   else if(leap == 30) leap = 20;
   else                leap = 21;

   jd = jdate(year-1, 3, leap);  // first day of the previous persian year

   ptimes[0] = jd;
   pyears[0] = year-621-1;

   for(mm=0+1; mm<=36; mm++) { // calculate next 36 months
      if     ((mm%12) <= 6)  ptimes[mm] = ptimes[mm-1] + 31.0;
      else if((mm%12) <= 11) ptimes[mm] = ptimes[mm-1] + 30.0;
      if((mm == 12) || (mm == 24) || (mm == 36)) {  // first month of the next year
         if(leap == 20)  ptimes[mm] = ptimes[mm-1] + 30.0;  // set length of the previous month
         else            ptimes[mm] = ptimes[mm-1] + 29.0;
      }
      pyears[mm] = (year-621-1) + (mm/12);
   }
}

void init_indian(int year)
{
   // India civil calendar
   itimes[0] = jdate(year-1, 12, 22);  // start of Pau the year before
   itimes[1] = itimes[0] + 30.0;           // Mag
   itimes[2] = itimes[1] + 30.0;           // Pha
   itimes[3] = itimes[2] + 30.0;           // start of Cai
   if(leap_year(year-79+78)) itimes[3] -= 1.0;  // adjust for leap years
   itimes[4] = itimes[3] + 31.0;           // Vai
   itimes[5] = itimes[4] + 31.0;           // Jya
   itimes[6] = itimes[5] + 31.0;           // Asa
   itimes[7] = itimes[6] + 31.0;           // Sra
   itimes[8] = itimes[7] + 31.0;           // Bha
   itimes[9] = itimes[8] + 31.0;           // Asv

   itimes[10] = itimes[9]  + 30.0;         // Kar
   itimes[11] = itimes[10] + 30.0;         // Agr
   itimes[12] = itimes[11] + 30.0;         // Pau
   itimes[13] = itimes[12] + 30.0;         // Mag
}

int load_china_year(int year, int mm)
{
int i;
int index;
int month, months;
double days;

   index = year - 2009;
   if(china_data[index].leap_month) months = 13;
   else                             months = 12;
   year = year + 4635 - 1998 + 60;  // note +60 years is common in the US

   ctimes[mm] = china_data[index].jd;
   month = 1;
   for(i=mm; i<mm+months; i++) {
      cyear[i]  = year;
      cmonth[i] = month;
      if(china_data[index].leap_month) {  // this year has a leap month
         if((i-mm+1) != china_data[index].leap_month) ++month;
      }
      else {
         ++month;
      }

      if(china_data[index].month_mask & (1 << (i-mm))) days = 30.0;
      else                                             days = 29.0;
      ctimes[i+1] = ctimes[i] + days;
   }
   if(china_data[index].leap_month) {
      cmonth[china_data[index].leap_month] *= (-1);
   }

   return months;
}

void init_chinese(int year)
{
int month;

   if((year < 2010) || (year > 2100)) return;
   month = 0;
   month += load_china_year(year-1, month);
   month += load_china_year(year+0, month);
   month += load_china_year(year+1, month);
//FILE *f;
//int i;
//f = topen("abcd", "w");
//for(i=0; i<40; i++) {
//   fprintf(f, "%.1f %d %d\n", ctimes[i], cyear[i], cmonth[i]);
//}
//fclose(f);
}

int tax_day(int year, int month, int day)
{
int gd;

   // return next tax due day - first weekday on or after the 15th of a month
   gd = day;
   if     (day_of_week(day, month, year) == 6) gd += 2;  // the 15th is on Sat
   else if(day_of_week(day, month, year) == 0) gd += 1;  // the 15th is on Sun
   return gd;
}

void calc_greetings()
{
double t;
int gd;
int i;
double rosh, po, purim;
int m_year;

   // calculate a few useful greetings dates 

   // convert Julian times to Gregorian and update the greetings table
   adjust_season(spring);
   sprintf(spring_s, "Spring starts today around %02d:%02d %s", g_hours,g_minutes, out);
   i = set_holiday(SPRING, g_month, g_day);
   if(i >= 0) holiday[i].text = &spring_s[0];

   adjust_season(summer);
   sprintf(summer_s, "Summer starts today around %02d:%02d %s", g_hours,g_minutes, out);
   i = set_holiday(SUMMER, g_month, g_day);
   if(i >= 0) holiday[i].text = &summer_s[0];

   adjust_season(fall);
   sprintf(autumn_s, "Autumn starts today around %02d:%02d %s", g_hours,g_minutes,out);
   i = set_holiday(AUTUMN, g_month, g_day);
   if(i >= 0) holiday[i].text = &autumn_s[0];

   adjust_season(winter);
   sprintf(winter_s, "Winter starts today around %02d:%02d %s", g_hours,g_minutes,out);
   i = set_holiday(WINTER, g_month, g_day);
   if(i >= 0) holiday[i].text = &winter_s[0];

   t = chinese_ny();
   gregorian(0, t);
   sprintf(china_year, "Happy Chinese New Year!  It's the Year of the %s.", zodiac[this_year%12]);
// sprintf(china_year, "Happy Chinese New Year!  It's the Year of the %s.", zodiac[g_year%12]);
   i = set_holiday(CHINA_NY, g_month, g_day);
   if(i >= 0) holiday[i].text = &china_year[0];

   easter(this_year);     // a few easter based holidays

   if(dst_disabled() == 0) {    // daylight savings time days
      t = jdate(this_year, dst_start_month, dst_start_day) - 1.0;
      gregorian(0, t);
      set_holiday(CLOCK_FWD, g_month, g_day);

      t = jdate(this_year, dst_end_month, dst_end_day) - 1.0;
      gregorian(0, t);
      set_holiday(CLOCK_BACK, g_month, g_day);
   }
   else {  // disable DST message
      set_holiday(CLOCK_FWD, 3, 0);
      set_holiday(CLOCK_BACK, 11, 0);
   }

   // grandprents day
   gd = nth_dow_in(1, 1, 9, this_year);  // first Monday in sept = Labor Day
   t = jdate(this_year, 9, gd) + 6.0;    // the next sunday
   gregorian(0, t);
   set_holiday(GRANNY, g_month, g_day);

   // election day
   gd = nth_dow_in(1, 1, 11, this_year) + 1;
   set_holiday(ELECTION, 11, gd);

   // tax days
   gd = tax_day(this_year,   4,  15);
   set_holiday(TAX_DAY,      4,  gd);
   gd = tax_day(this_year,   6,  15);
   set_holiday(Q2_TAXES,     6,  gd);
   gd = tax_day(this_year,   9,  15);
   set_holiday(Q3_TAXES,     9,  gd);
   gd = tax_day(this_year,   1,  15);
   set_holiday(Q4_TAXES,     1,  gd);
   gd = tax_day(this_year,   10, 15);
   set_holiday(FINAL_TAXES,  10, gd);

   // calculate approximate Islamic holidays (approximate since they can only
   // be offically determined by visual observation of the crescent moon)
   if(get_islam_day(this_year, 1, 1)) {    // Al-Hijra - new year
      set_holiday(AL_HIJRA, g_month, g_day);
   }

   if(get_islam_day(this_year, 1, 10)) {   // Asura
      set_holiday(ASURA, g_month, g_day);
   }

   if(get_islam_day(this_year, 9, 1)) {    // Ramadan - new year
      set_holiday(RAMADAN, g_month, g_day);
   }

   if(get_islam_day(this_year, 12, 10)) {  // Eid al Addha
      set_holiday(EID, g_month, g_day);
   }

   // calculate some jewish holidays
   rosh = rosh_jd(this_year);        // our reference is Rosh Hashahna 
                                     // once you know Rosh Hashahna,  all is revealed

   gregorian(0, rosh-1.0);              // Rosh Hashahna (party starts the night before)
   set_holiday(ROSH, g_month, g_day);

   gregorian(0, rosh+9.0-1.0);          // Yom Kipper is nine days after Rosh Hashanah
   set_holiday(YOM, g_month, g_day);

   po = rosh - 29.0 - 30.0 - 29.0 - 30.0 - 29.0 - 30.0;  // Julian date of Nisan 1
   purim = po - 29.0 + 13.0;   // Purim is Adar 14

   purim -= 1.0;               // start observance the night before
   gregorian(0, purim);
   set_holiday(PURIM, g_month, g_day);

   po += 14.0;                 // Passover is Nisan 15
   po -= 1.0;                  // but starts the night before
   gregorian(0, po);              // convert Julian passover to Gregorian terms
   set_holiday(PASSOVER, g_month, g_day);

   rosh += 30.0 + 29.0 + 24.0;   // Hanukkah is Kislev 25
   rosh -= 1.0;                  // start observance the night before
   gregorian(0, rosh);
   set_holiday(HANUKKAH, g_month, g_day);

   t = jtimes[9] + 14.0 - 1.0;   // 15 Tishrei
// rosh -= 1.0;                  // start observance the night before
   gregorian(0, t);
   set_holiday(SUKKOT, g_month, g_day);

   t = jtimes[5] + 5.0 - 1.0;    // 6 Sivan
// rosh -= 1.0;                  // start observance the night before
   gregorian(0, t);
   set_holiday(SHAVUOT, g_month, g_day);

// get_mayan_date();  // Mayan Tzolkin new year
// t = (360 - ((uinal * 20) + kin)) % 360;
// t += jdate(pri_year, pri_month, pri_day);
// gregorian(0, t);
// set_holiday(MAYAN_NY, g_month, g_day);

   t = jdate(2010,5,27);      // t = known Tzolkin new year
   t += (MAYAN_CORR-mayan_correlation);
   gregorian(0, jd_local);
   po = jdate(g_year,g_month,g_day);  // today
   m_year = 2010;
   while(m_year <= (this_year+2)) {  // find Mayan Tzolkin new year that starts on or after today
      if(t >= po) break;
      t += 260.0;
      ++m_year;
   }
   gregorian(0, t);
   set_holiday(MAYAN_NY, g_month, g_day);

   t = jdate(2012, 12, 20);
   t += (MAYAN_CORR-mayan_correlation);
   gregorian(0, t);
   if(g_year == this_year) set_holiday(DOOMSDAY, g_month, g_day);
   else                    set_holiday(DOOMSDAY, 0, 0);

   t = jdate(2009,10,7);      // t = known Aztec new year
   t += (double) aztec_epoch;
   gregorian(0, jd_local);
   po = jdate(g_year,g_month,g_day);  // today
   g_year = 2009;
   while(g_year <= (this_year+2)) {   // find Aztec new year that starts on or after today
      if(t >= po) break;
      t += 365.0;
      ++g_year;
   }
   gregorian(0, t);
   set_holiday(AZTEC_NY, g_month, g_day);

   if(this_year == 2038) set_holiday(UNIX_CLOCK, 1, 18);
   else                  set_holiday(UNIX_CLOCK, 0, 0);
}

struct SOL_SUMS {
   double a;
   double b;
   double c;
} sol[24] = {
  { 485.0,     324.96*DEG_TO_RAD,          1934.136*DEG_TO_RAD},
  { 203.0,     337.23*DEG_TO_RAD,         32964.467*DEG_TO_RAD},
  { 199.0,     342.08*DEG_TO_RAD,            20.186*DEG_TO_RAD},
  { 182.0,      27.85*DEG_TO_RAD,        445267.112*DEG_TO_RAD},
  { 156.0,      73.14*DEG_TO_RAD,         45036.886*DEG_TO_RAD},
  { 136.0,     171.52*DEG_TO_RAD,         22518.443*DEG_TO_RAD},
  {  77.0,     222.54*DEG_TO_RAD,         65928.934*DEG_TO_RAD},
  {  74.0,     296.72*DEG_TO_RAD,          3034.906*DEG_TO_RAD},
  {  70.0,     243.58*DEG_TO_RAD,          9037.513*DEG_TO_RAD},
  {  58.0,     119.81*DEG_TO_RAD,         33718.147*DEG_TO_RAD},
  {  52.0,     297.17*DEG_TO_RAD,           150.678*DEG_TO_RAD},
  {  50.0,      21.02*DEG_TO_RAD,          2281.226*DEG_TO_RAD},
  {  45.0,     247.54*DEG_TO_RAD,         29929.562*DEG_TO_RAD},
  {  44.0,     325.15*DEG_TO_RAD,         31555.956*DEG_TO_RAD},
  {  29.0,      60.93*DEG_TO_RAD,          4443.417*DEG_TO_RAD},
  {  18.0,     155.12*DEG_TO_RAD,         67555.328*DEG_TO_RAD},
  {  17.0,     288.79*DEG_TO_RAD,          4562.452*DEG_TO_RAD},
  {  16.0,     198.04*DEG_TO_RAD,         62894.029*DEG_TO_RAD},
  {  14.0,     199.76*DEG_TO_RAD,         31436.921*DEG_TO_RAD},
  {  12.0,      95.39*DEG_TO_RAD,         14577.848*DEG_TO_RAD},
  {  12.0,     287.11*DEG_TO_RAD,         31931.756*DEG_TO_RAD},
  {  12.0,     320.81*DEG_TO_RAD,         34777.259*DEG_TO_RAD},
  {   9.0,     227.73*DEG_TO_RAD,          1222.114*DEG_TO_RAD},
  {   8.0,      15.45*DEG_TO_RAD,         16859.074*DEG_TO_RAD}
};

double calc_vs(double jdme)
{
int i;
double t;
double s;
double w, jd, l;

   t = (jdme - JD2000) / 36525.0;    // Julian centuries from 2000 (of equ/sol).

   s = 0.0;
   for(i=0; i<=23; i++) {
      s = s + sol[i].a*cos(sol[i].b + sol[i].c*t);
   }

   w = (35999.373*t - 2.47)*DEG_TO_RAD; // radians
   l = 1.0 + 0.0334*cos(w) + 0.0007*cos(2.0*w);
   jd = jdme + (0.00001*s/l);           // the final result in Julian Dynamical Days.
   return jd;
}

void calc_seasons()
{
double m;
double t;

   // calculate/Julian date/time when each season starts
   m = ((double) (this_year - 2000)) / 1000.0;
   spring = calc_vs(2451623.80984 + 365242.37404*m + 0.05169*m*m - 0.00411*m*m*m - 0.00057*m*m*m*m);
   summer = calc_vs(2451716.56767 + 365241.62603*m + 0.00325*m*m + 0.00888*m*m*m - 0.00030*m*m*m*m);
   fall   = calc_vs(2451810.21715 + 365242.01767*m - 0.11575*m*m + 0.00337*m*m*m + 0.00078*m*m*m*m);
   winter = calc_vs(2451900.05952 + 365242.74049*m - 0.06223*m*m - 0.00823*m*m*m + 0.00032*m*m*m*m);

   if(lat < 0.0) {   // down under the world is bass ackwards
      t = fall;
      fall = spring;
      spring = t;

      t = winter;
      winter = summer;
      summer = t;
   }
}

int is_holiday(int pri_year,int pri_month,int pri_day)
{
int i;
int dow;
int day;

   if(no_greetings) return 0;

   for(i=0; i<calendar_entries; i++) { // is it a special day?
      if(holiday[i].month == 0) continue;

      day = holiday[i].day;
      if(day < 0) ;   // day number for dates like Friday the 13th (would be -13)
      else if(pri_month != holiday[i].month) continue;

      if((holiday[i].nth != 0) && (holiday[i].nth < 100)) {
         day = nth_dow_in(holiday[i].nth, day, pri_month, pri_year);
      }
      if(day < 0) {
         day = 0 - day;
         dow = day_of_week(pri_day,pri_month,pri_year);
         if(dow != holiday[i].month) continue;
      }
      if(day != pri_day) continue;
      return 1;
   }
   return 0;
}


int show_greetings()
{
int i;
unsigned j;
char c;
int day;
int dow;
int shown;
char alarm[SLEN+1];

   // see if today is a day of greetings
   if(no_greetings) return 0;         // user is a grinch...
   if(greet_on == 0) return 0;        // not all info available for calculating greetings
   if(title_type == USER) return 0;   // don't override the user's title
   if(!have_local_time) return 0;     // time is in an astronomical scale
//printf("greetings %d\n", calendar_entries);
//for(i=0; i<calendar_entries; i++) { // is it a special day?
//printf("i:%-3d  nth:%d  day:%d  mo:%d   text:%s\n", i, holiday[i].nth,holiday[i].day,holiday[i].month,holiday[i].text);
//}

   shown = 0;
   for(i=0; i<calendar_entries; i++) { // is it a special day?
      if(holiday[i].month == 0) continue;

      day = holiday[i].day;
      if(day < 0) ;   // day number for dates like Friday the 13th (would be -13)
      else if(pri_month != holiday[i].month) continue;

      if((holiday[i].nth != 0) && (holiday[i].nth < 100)) {
         day = nth_dow_in(holiday[i].nth, day, pri_month, pri_year);
      }
      if(day < 0) {
         day = 0 - day;
         dow = day_of_week(pri_day,pri_month,pri_year);
         if(dow != holiday[i].month) continue;
      }
      if(day != pri_day) continue;
//if(debug_file) fprintf(debug_file, "toots pri:%02d/%02d/%04d  on:%d  day:%d  i:%d  hlt:%d  tz:%.9f  holiday:%d (%s)\n", 
//pri_day,pri_month,pri_year, greet_on, day,i,have_local_time,time_zone(),holiday[i].month, holiday[i].text);

      j = 0;
      if(alarm_jd) ;  // alarm already set, don't set calendar alarm
      else if(holiday[i].text[0] == '@') {  // calendar entry has an alarm time set
         alarm[0] = 0;
         for(j=1; j<strlen(holiday[i].text); j++) {
            c = holiday[i].text[j];
            if(c == 0) break;
            if(isdigit(c)) ;
            else if(c == ':') ;
            else break;
            alarm[j-1] = c;
            alarm[j] = 0;
         }
         if(alarm[0]) {
            sprintf(out, " %04d/%02d/%02d", pri_year,pri_month,pri_day);
            strcat(alarm, out);
         }
         set_alarm(alarm);
      }

      ++shown;
      title_type = GREETING;
//    plot_title[0] = 0;
      debug_text[0] = 0;
      debug_text2[0] = 0;
      debug_text3[0] = 0;
      debug_text4[0] = 0;
      if     (shown == 1) sprintf(plot_title,  "%s", &holiday[i].text[j]);
      else if(shown == 2) sprintf(debug_text,  "%s", &holiday[i].text[j]);
      else if(shown == 3) sprintf(debug_text2, "%s", &holiday[i].text[j]);
      else if(shown == 4) sprintf(debug_text3, "%s", &holiday[i].text[j]);
      else if(shown == 5) sprintf(debug_text4, "%s", &holiday[i].text[j]);
   }
   if(shown) return shown;

   if(title_type == GREETING) {  // erase old greetings
      plot_title[0] = 0;
      debug_text[0] = 0;
      debug_text2[0] = 0;
      debug_text3[0] = 0;
      debug_text4[0] = 0;
      title_type = NONE;
   }
   return shown;
}

void setup_calendars(int why)
{
  #ifdef GEN_CHINA_DATA
     void load_china_data();
     void load_hk_data();
     load_china_data();              // process data from projectpluto.com "chinese.dat"
     load_hk_data();                 // process data from Hong Kong Observatory files
  #endif

   calc_seasons();                   // calc jdates of the equinoxes
   calc_moons(jd_local, 1);          // moon phases for the current month

   init_chinese(this_year);          // setup chinese calendar
   init_hebrew(this_year);           // setup Hebrew calendar (oy, vey! it be complicated)
   init_islamic(this_year);          // setup (appoximate) Islamic calendar
   init_persian(this_year);          // setup Persian/Afghan/Kurdish calendars
   init_indian(this_year);           // setup Indian civil calendar
   init_druid(this_year);            // setup pseudo-Druid calendar

   calc_greetings();                 // calculate seasonal holidays and insert into table
   show_greetings();                 // display any current greeting
}
#endif   // GREET_STUFF



char *fmt_date(int big_flag)
{
static char date[32];
double jd;
long x;
int month;
int day;
int year;
int d, iso_year, iso_week, iso_day;
char *date_flag;

   // return a string of the current date formatted for the selected calendar

   if(big_flag) date_flag = "";
   else if(rolled || user_set_rollover) date_flag = " ro";
   else date_flag = "   ";

#ifdef GREET_STUFF
   if(alt_calendar == HEBREW) {  // convert current date to Jewish Standard Time
      jd = jdate(pri_year,pri_month,pri_day);
      for(month=0; month<=13; month++) {
         if(jtimes[month] == 0.0) continue;  // its not a leap year
         if((jd >= jtimes[month]) && (jd < jtimes[month+1])) {
            day = ((int) (jd - jtimes[month])) + 1;
            year = 5770 + (pri_year-2010);
            if(month >= 9) ++year;
            sprintf(date, "%02d %s %04d%s", day, jmonths[month], year, date_flag);
            return &date[0];
         }
      }
   }
   else if(alt_calendar == INDIAN) {  // convert date to Indian civil calendar
      jd = jdate(pri_year,pri_month,pri_day);
      for(month=0; month<=12; month++) {
         if((jd >= itimes[month]) && (jd < itimes[month+1])) {
            day = ((int) (jd - itimes[month])) + 1;
            year = (pri_year-78-1);
            if(jd >= itimes[3]) ++year;
            sprintf(date, "%02d %s %04d%s", day, imonths[month], year, date_flag); 
            return &date[0];
         }
      }
   }
   else if(alt_calendar == ISLAMIC) {
      jd = jdate(pri_year,pri_month,pri_day);
      for(month=0; month<36; month++) {
         if((jd >= mtimes[month]) && (jd < mtimes[month+1])) {
            day = ((int) (jd - mtimes[month])) + 1;
            year = myears[month];
            sprintf(date, "%02d %s %04d%s", day, mmonths[month], year, date_flag); 
            return &date[0];
         }
      }
   }
   else if(alt_calendar == CHINESE) {
      jd = jdate(pri_year,pri_month,pri_day);
      for(month=0; month<36; month++) {
         if((jd >= ctimes[month]) && (jd < ctimes[month+1])) {
            day = ((int) (jd - ctimes[month])) + 1;
            year = cyear[month] + (int) chinese_epoch;
            if(cmonth[month] < 0)  sprintf(out, "%d*", 0-cmonth[month]);
            else                   sprintf(out, "%d", cmonth[month]);
            sprintf(date, "%04d/%s/%02d%s   ", year, out, day, date_flag); 
            return &date[0];
         }
      }
   }
   else if((alt_calendar == PERSIAN) || (alt_calendar == AFGHAN) || (alt_calendar == KURDISH)) {
      jd = jdate(pri_year,pri_month,pri_day);
      for(month=0; month<36; month++) {
         if((jd >= ptimes[month]) && (jd < ptimes[month+1])) {
            day = ((int) (jd - ptimes[month])) + 1;
            year = pyears[month];
            if     (alt_calendar == PERSIAN) sprintf(date, "%02d %s %04d%s", day, pmonths[month], year, date_flag); 
            else if(alt_calendar == KURDISH) sprintf(date, "%02d %s %04d%s", day, kmonths[month], year, date_flag); 
            else                             sprintf(date, "%02d %s %04d%s", day, amonths[month], year, date_flag); 
            return &date[0];
         }
      }
   }
   else if(alt_calendar == DRUID) {  // pseudo-Druid calendar
      jd = jdate(pri_year,pri_month,pri_day);
      for(month=0; month<15; month++) {
         if((jd >= dtimes[month]) && (jd < dtimes[month+1])) {
            day = ((int) (jd - dtimes[month])) + 1;
            year = dyears[month];
//          sprintf(date, "%02d %s %04d", day, dmonths[month], year);
            sprintf(date, "%02d %s%s", day, dmonths[month], date_flag); 
            return &date[0];
         }
      }
   }
   else if(alt_calendar == MJD) {
      jd = jdate(pri_year,pri_month,pri_day) - JD_MJD;
      sprintf(date, "MJD:  %5ld%s", (long) (jd+0.5), date_flag); 
      return &date[0];
   }
   else if(alt_calendar == JULIAN) {
      jd = jdate(pri_year,pri_month,pri_day);
      sprintf(date, "JD%9.1f%s", jd, date_flag); 
      return &date[0];
   }
   else if(alt_calendar == ISO) {
      jd = jdate(pri_year,pri_month,pri_day)-jdate(pri_year,1,1)+1;
      sprintf(date, "%04d-%03d%s   ", pri_year,(int)jd, date_flag); 
      return &date[0];
   }
   else if(alt_calendar == ISO_WEEK) {
      d = nth_dow_in(1, 1, 1, pri_year); // first monday in the year
      jd = jdate(pri_year,pri_month,pri_day) - jdate(pri_year,1,d);
      iso_week = ((int) jd) / 7;
      iso_day = ((int) jd) % 7;
      iso_year = pri_year;
      if(iso_day < 0) {
         iso_day += 7;
         iso_week += 51;
         iso_year -= 1;
      }
//sprintf(debug_text, "doy:%g  pri:%d/%d/%d  iy:%d iw:%d  id:%d  d:%d", jd, pri_year,pri_month,pri_day, iso_year,iso_week,iso_day, d);
      sprintf(date, "%04d-W%02d-%d%s", iso_year, iso_week+1,iso_day+1, date_flag); 

      return &date[0];
   }
   else if(alt_calendar == MAYAN) {  // long count
      get_mayan_date();
      sprintf(date, "%02d.%02d.%02d.%02d.%02d%s", baktun,katun,tun,uinal,kin, date_flag); 
      return &date[0];
   }
   else if(alt_calendar == HAAB) {
      x = (long) (jdate(pri_year, pri_month, pri_day) - jdate(2009, 4, 3));
      x = adjust_mayan(x);
      x = (x % 365L);
      day = (int) (x % 20L);
      month = (int) ((x / 20L) % 19L);
      sprintf(date, "%02d %s%s  ", day, haab[month], date_flag); 
      return &date[0];
   }
   else if(alt_calendar == TZOLKIN) {
      get_mayan_date();
      x = (long) (jdate(pri_year, pri_month, pri_day) - jdate(2001, 4, 3));
      x = adjust_mayan(x);
      x %= 13L;
      day = (int) (x+1);
      sprintf(date, "%02d %s%s ", day, tzolkin[kin], date_flag); 
      return &date[0];
   }
   else if(alt_calendar == AZTEC) {  // Tzolkin type dates
      x = (long) (jdate(pri_year, pri_month, pri_day) - jdate(2009,9,9));
      x += aztec_epoch;
      x %= (260L);
      day   = (int) (x % 13);
      month = (int) (x % 20L);
      sprintf(date, "%02d %s%s ", day+1, aztec[month], date_flag); 
      return &date[0];
   }
   else if(alt_calendar == AZTEC_HAAB) {  // Aztec haab style dates
      x = (long) (jdate(pri_year, pri_month, pri_day) - jdate(2009, 10, 7));
      x += aztec_epoch;
      x = (x % 365L);
      day = (int) (x % 20L);
      month = (int) ((x / 20L) % 19L);
      sprintf(date, "%02d %s%s  ", day+1, aztec_haab[month], date_flag); 
      return &date[0];
   }
   else if(alt_calendar == BOLIVIAN) {
      jd = (jdate(pri_year+5001, pri_month, pri_day) - jdate(1492, 6, 21));
      jd += (double) bolivian_epoch;
      year = (int) (jd/365.25);
      pri_month = fix_month(pri_month);
      sprintf(date, "%02d %s %04d%s", pri_day, months[pri_month], year, date_flag);
      return &date[0];
   }
   else if(use_hjd) {
      jd = jd_astro - JD_MJD;  // modified Julian date
      if((zoom_screen == 'C') || (rcvr_type == NO_RCVR)) {
         sprintf(date, "HJD:%ld%s", (long) (jd+0.5), date_flag); 
      }
      else {
         sprintf(date, "HJD:  %5ld%s", (long) (jd+0.5), date_flag); 
      }
      return &date[0];
   }
   else if(use_msd) {  // Mars timess
      planet:
      if((zoom_screen == 'C') || (rcvr_type == NO_RCVR)) {
         sprintf(date, "%s:%ld%s", std_string, (long) jd_astro, date_flag); 
      }
      else {
         sprintf(date, "%s:  %5ld%s", std_string, (long) jd_astro, date_flag); 
      }
      return &date[0];
   }
   else if(use_merc) { // Mercury time
      jd_astro = jd_astro / 87.968;
      goto planet;
   }
   else if(use_pluto) { // Pluto time
      jd_astro = jd_astro / 90560.0;
      goto planet;
   }
   else if(use_ven) { // Venus time
      jd_astro = jd_astro / 224.695;
      goto planet;
   }
   else if(use_bessel) {
      goto planet;
   }
#endif

   if(show_euro_dates) {
      sprintf(date, "%02d.%02d.%04d%s ", pri_day, pri_month, pri_year, date_flag);
   }
   else if(show_iso_dates) {
      sprintf(date, "%04d.%02d.%02d%s ", pri_year, pri_month, pri_day, date_flag);
   }
   else {
      pri_month = fix_month(pri_month);
      sprintf(date, "%02d %s %04d%s", pri_day, months[pri_month], pri_year, date_flag);
   }
   return &date[0];
}



#ifdef DIGITAL_CLOCK

char *tz_info()
{
   if(time_zone_set && tz_string[0]) return &tz_string[0];
   else if(time_flags & TFLAGS_UTC) return "UTC";
   else return"GPS";
}


int show_digital_clock(int row, int col)
{
int time_exit;
int width, height;
int www;
int ofs;
int min_scale;
int top_line;
int len;
int i;
int hh;
char s[32];
char *am_pm;
int old_vs;
int time_top;
int old_time_color;
int date_row, date_scale;
int hhh;
#define TIME_CHARS len

   // show the big digital clock if we can find a place for it

   old_vs = VCHAR_SCALE;
   time_exit = 0;     // assume there is room for the sat info and clock
   www = 0;
   ofs = 2;

   if(pri_hours >= 12) am_pm = "PM";
   else                am_pm = "AM";

   if((rcvr_type == NO_RCVR) && (zoom_screen != 'C')) { 
      if(text_mode) return 0;  // no room on screen to do this
      if(first_key) return 0;
      if(zoom_screen) return 0;
   }

// if(all_adevs && (SCREEN_WIDTH < MEDIUM_WIDTH)) return 0;
   if(all_adevs && (SCREEN_WIDTH < NARROW_SCREEN)) return 0;

   old_time_color = time_color;
   if(eclipsed) time_color = YELLOW;

   // format the time display string
   ofs = 2;
   if(show_elapsed_time && jd_elapsed) { // must be first item in this if statement
      sprintf(out, "%.3f secs", (jd_utc-jd_elapsed)*24.0*60.0*60.0+0.00001);
   }
   else if(show_julian_time) {
      sprintf(out, "%.6f", jd_display);  // 14.
      ofs = 1;
   }
   else if(show_mjd_time) {
      sprintf(out, "%.6f", jd_display-JD_MJD);  // 12.
      ofs = 1;
   }
   else if(show_unix_time) { // 12.
      sprintf(out, "%.3f", (jd_display-LINUX_EPOCH)*24.0*60.0*60.0);
      ofs = 1;
   }
   else if(show_gps_time) {  // 12.
      sprintf(out, "%.3f", (jd_display-GPS_EPOCH)*24.0*60.0*60.0);
      ofs = 1;
   }
   else if(show_msecs) {
      sprintf(out, "%02d:%02d:%02d.%03d", clock_12?(pri_hours%12):pri_hours, pri_minutes, pri_seconds, (int)(pri_frac*1000.0));
      ofs = 3;
   }
   else if(clock_12) {
      sprintf(out, "%02d:%02d:%02d %s", pri_hours%12, pri_minutes, pri_seconds, am_pm);
      ofs = 3;
   }
   else {
      sprintf(out, "%02d:%02d:%02d", pri_hours, pri_minutes, pri_seconds);
   }
   len = strlen(out)+1;

   if((zoom_screen == 'C') || (rcvr_type == NO_RCVR)) { 
      if(0 && show_euro_dates) {
         sprintf(out, "%02d.%02d.%02d", pri_day, pri_month, pri_year%100);
      }
      else {
         strcpy(out, fmt_date(1));
      }
      len = strlen(out);

      if(len < 14) len = 14;
      VCHAR_SCALE = ((SCREEN_WIDTH/8) / len);
      if(rcvr_type == NO_RCVR) {
         if(zoom_screen != 'C') {  // tweak scale factor for wide/short screens
            VCHAR_SCALE = (SCREEN_WIDTH / len);
            VCHAR_SCALE = (((VCHAR_SCALE*SCREEN_HEIGHT*1024)/768)) / (SCREEN_WIDTH*8);
            if(SCREEN_WIDTH < TINY_TINY_WIDTH) {
               VCHAR_SCALE = (VCHAR_SCALE*vc_font_scale/100);
            }
         }
      }

      if(jpl_clock) {
         i = ((SCREEN_HEIGHT) / (6*VCHAR_H));
         if(i < VCHAR_SCALE) VCHAR_SCALE = i;
         if(SCREEN_WIDTH > WIDE_WIDTH) VCHAR_SCALE = ((SCREEN_HEIGHT) / (11*VCHAR_H));

         time_row = VCHAR_SCALE/4;
         date_row = time_row;
         date_scale = VCHAR_SCALE;

         center_vstring(time_row, VCHAR_SCALE, time_color, out);

         time_row += VCHAR_SCALE+VCHAR_SCALE;
         sprintf(out, "%02d:%02d:%02d UTC", clock_12?(hours%12):hours, minutes, seconds);
         center_vstring(time_row, VCHAR_SCALE, time_color, out);

         time_row += VCHAR_SCALE+VCHAR_SCALE/2;
         hh = pri_hours+3;
         if(hh < 0) hh += 24;
         else if(hh > 23) hh -= 24;
         strcpy(s, tz_string);
         s[0] = 'E';
         sprintf(out, "%02d:%02d:%02d %s", clock_12?(hh%12):hh, pri_minutes, pri_seconds, s);
         center_vstring(time_row, VCHAR_SCALE, time_color, out);

         time_row += VCHAR_SCALE+VCHAR_SCALE/2;
         sprintf(out, "%02d:%02d:%02d %s", clock_12?(pri_hours%12):pri_hours, pri_minutes, pri_seconds, tz_string);
         center_vstring(time_row, VCHAR_SCALE, time_color, out);
      }
      else {
         time_row = ((SCREEN_HEIGHT/TEXT_HEIGHT)/2) - ((VCHAR_SCALE*16)/(2*TEXT_HEIGHT));
         time_row += (VCHAR_SCALE*16/2)/TEXT_HEIGHT;

         if((rcvr_type == NO_RCVR) && (zoom_screen != 'C')) {
            time_row = SCREEN_HEIGHT / TEXT_HEIGHT;
            time_row +=2;
            time_row -= (VCHAR_SCALE*16*3/2)/TEXT_HEIGHT;

            time_top = (time_row * TEXT_HEIGHT);  // top line of digital clock display
            time_top -= (VCHAR_SCALE*16*3/2);

            ACLOCK_SIZE = time_top - (TEXT_HEIGHT*1);
            if(ACLOCK_SIZE > (SCREEN_WIDTH/3)) ACLOCK_SIZE = (SCREEN_WIDTH/3);
            aclock_y = (ACLOCK_SIZE/2) + (TEXT_HEIGHT*1);
            aclock_x = SCREEN_WIDTH - (ACLOCK_SIZE/2) - (TEXT_WIDTH*2);
         }

         date_row = time_row;
         date_scale = VCHAR_SCALE;
         center_vstring(time_row, VCHAR_SCALE, time_color, out);

         time_row -= (VCHAR_SCALE*16*3/2)/TEXT_HEIGHT;
         if(show_elapsed_time && jd_elapsed) { // must be first item in this if statement
            sprintf(out, "%.3f secs", (jd_utc-jd_elapsed)*24.0*60.0*60.0+0.00001);
            len = strlen(out)+1;
            center_vstring(time_row, VCHAR_SCALE, time_color, out);
         }
         else if(show_julian_time) {
            sprintf(out, "%14.6f %s", jd_local, tz_info());
            len = strlen(out)+1;
            center_vstring(time_row, VCHAR_SCALE*14/len, time_color, out);
         }
         else if(show_mjd_time) {
            sprintf(out, "%12.6f %s", jd_local-JD_MJD, tz_info());
            len = strlen(out)+1;
            center_vstring(time_row, VCHAR_SCALE*14/len, time_color, out);
         }
         else if(show_unix_time) {
            sprintf(out, "%14.3f %s", (jd_local-LINUX_EPOCH)*24.0*60.0*60.0, tz_info());
            len = strlen(out)+1;
            center_vstring(time_row, VCHAR_SCALE*14/len, time_color, out);
         }
         else if(show_gps_time) {
            sprintf(out, "%14.3f %s", (jd_local-GPS_EPOCH)*24.0*60.0*60.0, tz_info());
            len = strlen(out)+1;
            center_vstring(time_row, VCHAR_SCALE*14/len, time_color, out);
         }
         else if(show_msecs) {
            if(clock_12) {
               sprintf(out, "%02d:%02d:%02d.%03d %s", pri_hours%12,pri_minutes,pri_seconds,(int)(pri_frac*1000.0), tz_info());
               len = strlen(out)+1;
               center_vstring(time_row, VCHAR_SCALE*14/len, time_color, out);
            }
            else {
               sprintf(out, "%02d:%02d:%02d.%03d %s", pri_hours,pri_minutes,pri_seconds,(int)(pri_frac*1000.0), tz_info());
               len = strlen(out)+1;
               center_vstring(time_row, VCHAR_SCALE*14/len, time_color, out);
            }
         }
         else if(clock_12){
            VCHAR_SCALE = (VCHAR_SCALE * 12) / 15;
            sprintf(out, "%02d:%02d:%02d %s %s", pri_hours%12,pri_minutes,pri_seconds, am_pm, tz_info());
            center_vstring(time_row, VCHAR_SCALE, time_color, out);
         }
         else {
            sprintf(out, "%02d:%02d:%02d %s", pri_hours,pri_minutes,pri_seconds, tz_info());
            center_vstring(time_row, VCHAR_SCALE, time_color, out);
         }
      }

      if(1 && ((zoom_screen == 'C') || (rcvr_type == NO_RCVR)) && enviro_mode()) {  // show environmental sensor data on the clock screen
         #define ENV_COL (VCharWidth*3)
         if(zoom_screen == 'C') {
            date_row += (date_scale*1);

            hhh = (SCREEN_HEIGHT - date_row*date_scale) / 8;
            VCHAR_SCALE = (hhh/16);

            date_row = (SCREEN_HEIGHT-VCharHeight*4);
            col = ENV_COL;
         }
         else {
            VCHAR_SCALE = 3; // (hhh/16);
            date_row = (VCharHeight*50)/19;
            col = (VCharWidth*2);
         }


         i = round_temp;
         round_temp = 2;
         if     (have_temperature0) sprintf(out, "Temp0:%7.2f%c%c",  scale_temp(tc0),DEGREES,DEG_SCALE);
         else if(have_temperature1) sprintf(out, "Temp1:%7.2f%c%c",  scale_temp(tc1),DEGREES,DEG_SCALE); 
         else if(have_temperature2) sprintf(out, "Temp2:%7.2f%c%c",  scale_temp(tc2),DEGREES,DEG_SCALE); 
         else if(have_temperature)  sprintf(out, "Temp:  %7.2f%c%c", scale_temp(temperature),DEGREES,DEG_SCALE); 
         else                       sprintf(out, "                ");

         graphics_coords = 1;

         round_temp = i;
         vchar_string(date_row, col, time_color, out);

         date_row += VCharHeight; //VCHAR_SCALE;
         sprintf(out, "Baro: %7.2f mb", pressure);
         vchar_string(date_row, col, time_color, out);

         date_row += VCharHeight; //VCHAR_SCALE;
         sprintf(out, "RH:   %7.2f %%", humidity);
         vchar_string(date_row, col, time_color, out);

         graphics_coords = 0;
      }

      digital_clock_shown = 1;
      show_title();

      time_color = old_time_color;
      return time_exit;
   }
   else if(zoom_screen) {
      time_color = old_time_color;
      return 1;
   }
   else if(all_adevs) { 
      if(small_font == 2) VCHAR_SCALE = 5;  // DOS 8x8 font
      else                VCHAR_SCALE = 6;
      if(rcvr_type == TICC_RCVR) VCHAR_SCALE -= 1;

      if(SCREEN_WIDTH <= 640) { 
         VCHAR_SCALE = 2;
      }
      else if(SCREEN_WIDTH < MEDIUM_WIDTH) {
         VCHAR_SCALE = 3;
         col -= 6;
      }

      if(WIDE_SCREEN) {  // center digital clock over right hand adev tables
         width = VCharWidth;
      }
      else {   // center digital clock in the upper right hand corner space
         width = ((SCREEN_WIDTH/TEXT_WIDTH)-col)*TEXT_WIDTH;
         if(clock_12) i = 3;
         else         i = 2;
         width -= ((TIME_CHARS+ofs) * VCharWidth);
         width /= 2;
      }
      www = width;
      time_col = col + (width/TEXT_WIDTH);
      time_row = row;
   }
   else {  // clock goes in or below sat info area
      if(temp_sats < 8) top_line = (row+1+8);
      else              top_line = (row+1+temp_sats);
      if(sat_cols > 1)  top_line = (row+1+sat_rows);

//    width = (INFO_COL - TIME_CHARS) * TEXT_WIDTH;
      width = (INFO_COL - 0*TIME_CHARS) * TEXT_WIDTH;
      www = width;
      height = (MOUSE_ROW-top_line-1) * TEXT_HEIGHT;

      VCHAR_SCALE = 1;

      height = (height / VCharHeight);
      width = (width / (VCharWidth*TIME_CHARS));
      if(use_vc_fonts && (vc_font_scale < 100)) width = (width*vc_font_scale) / 100;
      if(width < 0) width = 1;
      if(height < 0) height = 1;

      if(width > height) VCHAR_SCALE = height;
      else               VCHAR_SCALE = width;

      if(SCREEN_HEIGHT < TINY_TINY_HEIGHT) {
//       if(VCHAR_SCALE < 2) VCHAR_SCALE = 1;
//       else                VCHAR_SCALE = 2;
         VCHAR_SCALE = 2;
      }

#define MIN_SCALE 2 
      min_scale = MIN_SCALE;

      if(VCHAR_SCALE < MIN_SCALE) {  // no room for time under the sat info
         time_exit = 1;      // so dont draw the sat info table
         top_line = row;
         VCHAR_SCALE = 6;
      }

      time_col = (INFO_COL - 0 - ((TIME_CHARS*VCharWidth)/TEXT_WIDTH)) / 2;
      time_row = (MOUSE_ROW - top_line - (VCharHeight/TEXT_HEIGHT)) / 2;
      time_row += top_line+1;
      if((MOUSE_ROW-time_row) > 1) ++time_row;
   }
   if(time_col < 0) time_col = 0;
   clock_show_row = time_row;
//sprintf(plot_title, "r:%d c:%d  w:%d h:%d www:%d  top:%d  vscale:%d  mscale:%d", time_row,time_col, width,height, www, top_line, VCHAR_SCALE, min_scale);

   if((MOUSE_ROW-time_row) <= 1) {  // very small area for clock display
      min_scale = 1;                // ... use unscaled text format
      VCHAR_SCALE = 1;
   }

   // show the time
   i = VCHAR_SCALE;
   if(all_adevs) {
      VCHAR_SCALE = i;
   }
   else {
      VCHAR_SCALE = (VCHAR_SCALE) * 8 / len;
      if(VCHAR_SCALE < min_scale) VCHAR_SCALE = min_scale;
   }

   vchar_string(time_row, time_col, time_color, out);

   VCHAR_SCALE = i;

   if(old_vs != VCHAR_SCALE) {  // screen config changed
      need_redraw = 1100;
   }

   digital_clock_shown = 1;
   time_color = old_time_color;
   return time_exit;
}
#endif  // DIGITAL_CLOCK


#ifdef ANALOG_CLOCK
//
// stuff for drawing an analog clock (watch)
//
char *roman[] = {
   " ",
   "I",     "II",    "III",
   "IIII",  "V",     "VI", 
   "VII",   "VIII",  "IX",
   "X",     "XI",    "XII",
   "XIII",  "XIV",   "XV",   
   "XVI",   "XVII",  "XVIII",
   "IXX",   "XX",    "XXI",  
   "XXII",  "XXIII", "*"
};
char *arabic[] = {
  " ",
  "1",    "2",   "3",
  "4",    "5",   "6",
  "7",    "8",   "9",
  "10",   "11",  "12",
  "13",   "14",  "15",
  "16",   "17",  "18",
  "19",   "20",  "21",
  "22",   "23",  "0" 
};
char *stars[] = {
   " ",
   "*",   "*",    "*",
   "*",   "*",    "*",
   "*",   "*",    "*",
   "*",   "*",    "*",
   "*",   "*",    "*",
   "*",   "*",    "*",
   "*",   "*",    "*",
   "*",   "*",    "!"
};

#define NUM_FACES 3
char **face[] = {  // select the watch face style
   roman,
   arabic,
   stars
};

void calc_moon_posn(int watch)
{
double maz,mel;
int r;

   if(map_and_watch && (watch == 0)) {
      r = AZEL_RADIUS;
      moon_size = (AZEL_RADIUS/4);
   }
   else {
      r = ACLOCK_R;
      moon_size = (ACLOCK_R/4);
   }

   mel = moon_el;
   if(mel < 0.0) {
      mel = 0.0 - mel;
      moon_color = GREY;
   }
   mel = (mel - 90.0F) / 90.0F;

   maz = moon_az;
   maz += 90.0F;
   if(maz >= 360.0) maz -= 360.0;
   if(maz <= 360.0) maz += 360.0;
   moon_x = (int) (mel * cos360((int)maz) * r);
   moon_y = (int) (mel * sin360((int)maz) * r);
//sprintf(plot_title, "ACLOCK:%d  AZEL:%d  maw:%d", ACLOCK_R,AZEL_RADIUS, map_and_watch);
}


void draw_watch_face()
{
int x, y;
int hh;
int lasth;
char **wf;
float hr;
u08 bottom_blocked, top_blocked;
int i;
int old_time_color;

   old_time_color = time_color;
   if(eclipsed) time_color = YELLOW;

   wf = face[watch_face%NUM_FACES];
   graphics_coords = 1;

   // find the best place to put the watch brand name
   bottom_blocked = top_blocked = 0;
   hh = (pri_hours / WATCH_MULT) % 12;
   hh *= 100;
   hh += pri_minutes;
   if((pri_minutes >= 20) && (pri_minutes <= 40)) bottom_blocked |= 1;
   if((hh >= 400) && (hh <= 800))                 bottom_blocked |= 2;
   if((pri_minutes >= 10) && (pri_minutes <= 50)) top_blocked |= 0;
   else                                           top_blocked |= 5;
   if((hh >= 200) && (hh <= 1000))                top_blocked |= 0;
   else                                           top_blocked |= 6;

   moon_x = 0;
   if     (top_blocked == 0)    { y = aclock_y-ACLOCK_SIZE/4; moon_y = (+ACLOCK_SIZE/4); }
   else if(bottom_blocked == 0) { y = aclock_y+ACLOCK_SIZE/4; moon_y = (-ACLOCK_SIZE/4); }
   else if(top_blocked & 1)     { y = aclock_y-ACLOCK_SIZE/4; moon_y = (+ACLOCK_SIZE/4); }
   else                         { y = aclock_y+ACLOCK_SIZE/4; moon_y = (-ACLOCK_SIZE/4); }

// sprintf(plot_title, "lwf:%d  wn[%02X]:%s  hlt:%d  r:%d  xy:%d,%d  gc:%d", label_watch_face, watch_name[0], watch_name, have_local_time, ACLOCK_R, aclock_x,y, graphics_coords);
   if(label_watch_face) {
      out[0] = 0;
      if(eclipsed) strcpy(out, "ECLIPSE!");
      else if(watch_name[0]) strcpy(out, watch_name);
      else if(!have_local_time) ;
      else if(ACLOCK_R < 70) ;
      else strcpy(out, days[day_of_week(pri_day, pri_month, pri_year)]);
      vidstr(y+TEXT_HEIGHT*0,aclock_x-strlen(out)*TEXT_WIDTH/2, time_color, out);

      out[0] = 0;
      if(watch_name2[0]) strcpy(out, watch_name2);
      else if(ACLOCK_R < 120) ;
      else strcpy(out, fmt_date(1));
      i = strlen(out);
      while(i--) {
         if(out[i] == ' ') out[i] = 0;
         else break;
      }
      vidstr(y+TEXT_HEIGHT*1,aclock_x-strlen(out)*TEXT_WIDTH/2, time_color, out);

      #ifdef GREET_STUFF
         if(moons[pri_day] && (watch_name[0] != ' ')) {
            if(y < aclock_y) vidstr(y+TEXT_HEIGHT*2,aclock_x-strlen(moons[pri_day])*TEXT_WIDTH/2, time_color, moons[pri_day]);
            else             vidstr(y-TEXT_HEIGHT*1,aclock_x-strlen(moons[pri_day])*TEXT_WIDTH/2, time_color, moons[pri_day]);
         }
      #endif
//sprintf(debug_text, "asize:%d", ACLOCK_R);
   }


   for(lasth=1; lasth<=WATCH_HOURS; lasth++) {  // draw the watch numerals
      hr = (float) lasth;
      hr *= (60.0F/(float)WATCH_HOURS);
      if(lasth == 24) x = 0;
      else if(lasth == (6*WATCH_MULT)) x = 0;
      else x = (int) (ACLOCK_R * cos(hand_angle(hr))); 
      if(x > 0) x -= strlen(wf[lasth])*TEXT_WIDTH;
      else if(x == 0) x -= (strlen(wf[lasth])*TEXT_WIDTH/2-(TEXT_WIDTH/2));
      x += aclock_x;
      x -= TEXT_WIDTH/2;

      y = (int) (ACLOCK_R * sin(hand_angle(hr))); 
      if(y > 0) y -= TEXT_HEIGHT;
      else if(y < 0) y += TEXT_HEIGHT;
      y += aclock_y;
      y -= TEXT_HEIGHT/2;


      // tweak text positions for more natural goodness
      hh = lasth;
      if((hh >= (2*WATCH_MULT)) && (hh <= (4*WATCH_MULT))) {
         x -= TEXT_WIDTH/2;
         if(hh == (3*WATCH_MULT)) x -= TEXT_WIDTH/2;
      }
      else if((hh >= (8*WATCH_MULT)) && (hh <= (10*WATCH_MULT))) {
         x += TEXT_WIDTH;  // *3/4;
         if(hh == (9*WATCH_MULT)) x += TEXT_WIDTH/2;
      }

      if(WATCH_HOURS == 24) {
         if     (hh == 1)  {                     x += TEXT_WIDTH/2;   }
         else if(hh == 7)  { y += TEXT_HEIGHT;                        }
         else if(hh == 3)  { y += TEXT_HEIGHT/2; x += TEXT_WIDTH/2;   }
         else if(hh == 9)  { y += TEXT_HEIGHT/2; x -= TEXT_WIDTH/2;   }
         else if(hh == 19) { y -= TEXT_HEIGHT;   x += TEXT_WIDTH/2;   }
         else if(hh == 21) { y -= TEXT_HEIGHT/2; x += TEXT_WIDTH;     }
         else if(hh == 13) { 
            if((watch_face%NUM_FACES) == 0) {  // special tweak for roman numerals
               y -= TEXT_HEIGHT/4; x -= TEXT_WIDTH*3/2;
            }
            else {
               x -= TEXT_WIDTH*3/4;
            }
         }
      }

      if(SCREEN_WIDTH <= NARROW_SCREEN) {  // tweaks for small screens
         if(hh == (1*WATCH_MULT)) {           // 1:00
            x += TEXT_WIDTH;
            if(PLOT_HEIGHT < 160) y += TEXT_HEIGHT/4;
         }
         else if(hh == (11*WATCH_MULT)) {     // 11:00
            if(PLOT_HEIGHT < 160) y += TEXT_HEIGHT/4;
         }
         else if(hh == (5*WATCH_MULT)) {      // 5:00
            x += TEXT_WIDTH;
            if(PLOT_HEIGHT < 160) y -= TEXT_HEIGHT/4;
         }
         else if(hh == (7*WATCH_MULT)) {      // 7:00
            if(PLOT_HEIGHT < 160) {
               x += TEXT_WIDTH/2;
               y -= TEXT_HEIGHT/4;
            }
         }
         else if(hh == (2*WATCH_MULT)) {
            y -= TEXT_HEIGHT/2;
         }
         else if(hh == (10*WATCH_MULT)) {
            y -= TEXT_HEIGHT/2;
         }

         if(WATCH_HOURS == 24) {
            if     (hh == 1)  { x += TEXT_WIDTH;                       }
            else if(hh == 3)  { x += TEXT_WIDTH/2; y -= TEXT_HEIGHT/2; }
            else if(hh == 4)  { x += TEXT_WIDTH/2;                     }
            else if(hh == 5)  { x += TEXT_WIDTH/2; y -= TEXT_HEIGHT/2; }
            else if(hh == 6)  { x += TEXT_WIDTH/2;                     }
            else if(hh == 8)  { x += TEXT_WIDTH/2; y += TEXT_HEIGHT/2; }
            else if(hh == 9)  { x += TEXT_WIDTH/2;                     }
            else if(hh == 11) { x += TEXT_WIDTH/2;                     }
//          else if(hh == 13) { x -= TEXT_WIDTH/1; y -= TEXT_HEIGHT/2; }
            else if(hh == 14) { x += TEXT_WIDTH/4;                     }
            else if(hh == 16) { x -= TEXT_WIDTH/4; y += TEXT_HEIGHT/4; }
            else if(hh == 17) {                    y += TEXT_HEIGHT/4; }
            else if(hh == 22) { x += TEXT_WIDTH*0; y -= TEXT_HEIGHT/4; }
            else if(hh == 23) { x += TEXT_WIDTH/2; y -= TEXT_HEIGHT/4; }
         }
      }

      vidstr(y,x, time_color, wf[lasth]);
   }

   time_color = old_time_color;
   graphics_coords = 0;
}


void draw_watch_outline()
{
int x, y;
int x0, y0;
int theta;
int alarm_theta;
int alarm_ticks;
int timer_theta;
int timer_ticks;
double jd_timer;
int color;
int stem_size;
#define THETA_STEP 1
int old_time_color;

   // draw the watch outline
   old_time_color = time_color;
   if(eclipsed) time_color = YELLOW;

   alarm_ticks = 0;
   alarm_theta = (999);  // shut up compiler warnings

   if(1) ;  // use edge ticks
   else if(alarm_date || alarm_time) {  // highlight the alarm clock time
      alarm_theta = ((alarm_hh%WATCH_HOURS)*WATCH_STEP*2)+alarm_mm;
      alarm_theta /= 2;
      alarm_theta -= 90;
      alarm_theta -= (ALARM_WIDTH/2);
      while(alarm_theta < 0) alarm_theta += 360;
      alarm_theta %= 360;
      alarm_theta /= THETA_STEP;      // round to clockface drawing increment
      alarm_theta *= THETA_STEP;
      if(alarm_theta < ALARM_WIDTH) { // alarm spans 0/360 degrees
         alarm_ticks = ALARM_WIDTH;
      }
   }    

   timer_ticks = 0;
   timer_theta = (999);  // shut up compiler warnings 

   if(1) ;  // use edge ticks
   else if(egg_timer) {  // highlight the egg time time
      jd_timer = jd_local + jtime(0,0,0, (double) egg_timer);
      jd_timer = jd_timer - (double) (int) jd_timer; 
      jd_timer = (jd_timer * 720.0);
      timer_theta = (int) jd_timer;
      timer_theta -= 90;
      timer_theta -= (ALARM_WIDTH/2);
      while(timer_theta < 0) timer_theta += 360;
      timer_theta %= 360;
      timer_theta /= THETA_STEP;      // round to clockface drawing increment
      timer_theta *= THETA_STEP;
      if(timer_theta < ALARM_WIDTH) { // timer spans 0/360 degrees
         timer_ticks = ALARM_WIDTH;
      }
   }


   if(luxor && (alarm_time || alarm_date || egg_timer)) {
      x = ((aclock_x + ACLOCK_R) / TEXT_WIDTH) - 2;
      y = ((aclock_y - ACLOCK_R) / TEXT_HEIGHT) + 2;
      out[0] = ALARM_CHAR;
      out[1] = 0;
      vidstr(y,x, WHITE, out);
   }

   x0 = (int) (ACLOCK_R * cos360(0));
   y0 = (int) (ACLOCK_R * sin360(0));

   for(theta=THETA_STEP; theta<=360; theta+=THETA_STEP) { 
      if(egg_timer && (theta == timer_theta)) {
         timer_ticks = ALARM_WIDTH*2;
      }
      if((alarm_date || alarm_time) && (theta == alarm_theta)) {
         alarm_ticks = ALARM_WIDTH*2;
      }

      color = time_color;
      if(timer_ticks) {
         color = TIMER_COLOR;
         --timer_ticks;
         if(THETA_STEP > 1) timer_ticks = 0;
      }
      if(alarm_ticks) {
         color = ALARM_COLOR;
         --alarm_ticks;
         if(THETA_STEP > 1) alarm_ticks = 0;
      }
      if(alarm_ticks && timer_ticks) color = BLACK;

      if(alarm_ticks || timer_ticks) {
         x = (int) ((ACLOCK_R+1) * cos360(theta));
         y = (int) ((ACLOCK_R+1) * sin360(theta));
         thick_line(aclock_x+x0,aclock_y+y0, aclock_x+x,aclock_y+y, color, 3);
      }
      else {
         x = (int) (ACLOCK_R * cos360(theta));
         y = (int) (ACLOCK_R * sin360(theta));
         line(aclock_x+x0,aclock_y+y0, aclock_x+x,aclock_y+y, color);
      }

      x0 = x;
      y0 = y;
   }

   stem_size = ACLOCK_SIZE / 50;      // draw watch winding stem
   if(stem_size < 3) stem_size = 3;
   line(aclock_x-stem_size, aclock_y-ACLOCK_R-stem_size, aclock_x+stem_size, aclock_y-ACLOCK_R-stem_size, time_color);
   line(aclock_x-stem_size, aclock_y-ACLOCK_R-stem_size, aclock_x-stem_size, aclock_y-ACLOCK_R, time_color);
   line(aclock_x+stem_size, aclock_y-ACLOCK_R-stem_size, aclock_x+stem_size, aclock_y-ACLOCK_R, time_color);

   time_color = old_time_color;
}


#define FANCY_ANGLE 2.50   // half width of hands in degrees
#define FANCY_STEP  0.25   // degrees step of hand color fill lines
#define FANCY_BREAK 0.66   // length of hands where the angle changes

void draw_watch_hands(void) 
{
int thickness;
int x, y;
int xx,yy;
int xxx,yyy;
double ang;
double hr;
double a;
int old_time_color;

   // Draw new hands.  If zoom_screen == 'X', try and outline the hand in black
   // so that they show up better agains the signal level map.
   old_time_color = time_color;
   if(eclipsed) time_color = YELLOW;

   thickness = 2;
   if((zoom_screen == 'X') || (zoom_screen == 'Y')) thickness = 3;

   a = hand_angle((double) pri_seconds + pri_frac);
   x = (int) ((ACLOCK_R-AA) * cos(a));
   y = (int) ((ACLOCK_R-AA) * sin(a));  
   if(zoom_screen == 'X') thick_line(aclock_x,aclock_y,  aclock_x+x,aclock_y+y, BLACK, thickness+2);
   thick_line(aclock_x,aclock_y,  aclock_x+x,aclock_y+y, time_color, thickness);


   if((zoom_screen == 'X') || (zoom_screen == 'Y')) thickness = 5;
   else if(ACLOCK_R > 100) thickness = 3;
   else               thickness = 2+1;


   hr = (double) pri_minutes;
// hr += ((double) pri_seconds) / 60.0F;
   a = hand_angle(hr);
   x = (int) ((ACLOCK_R-AA) * cos(a));  
   y = (int) ((ACLOCK_R-AA) * sin(a));  

   if(fancy_hands) {  // use trapazoidal hands
      if(fancy_hands > 1) ang = FANCY_ANGLE;  // hollow hands
      else ang = 0.0;  // filled hands

      while(ang<=FANCY_ANGLE) {
         xx = (int) ((ACLOCK_R-AA)*FANCY_BREAK * cos(a+ang*PI/180.0)); 
         yy = (int) ((ACLOCK_R-AA)*FANCY_BREAK * sin(a+ang*PI/180.0));

         xxx = (int) ((ACLOCK_R-AA)*FANCY_BREAK * cos(a-(ang*PI/180.0))); 
         yyy = (int) ((ACLOCK_R-AA)*FANCY_BREAK * sin(a-(ang*PI/180.0)));

         thick_line(aclock_x,aclock_y,  aclock_x+xx,aclock_y+yy, time_color, thickness);
         thick_line(aclock_x,aclock_y,  aclock_x+xxx,aclock_y+yyy, time_color, thickness);
         thick_line(aclock_x+x,aclock_y+y,  aclock_x+xx,aclock_y+yy, time_color, thickness);
         thick_line(aclock_x+x,aclock_y+y,  aclock_x+xxx,aclock_y+yyy, time_color, thickness);
         ang += FANCY_STEP;
      }
   }
   else {  // linear hands
      if(zoom_screen == 'X') thick_line(aclock_x,aclock_y,  aclock_x+x,aclock_y+y, BLACK, thickness+2);
      thick_line(aclock_x,aclock_y,  aclock_x+x,aclock_y+y, time_color, thickness);
   }



   hr = (double) pri_hours;
   hr += ((double)pri_minutes/60.0);
   hr *= (60.0/(double)WATCH_HOURS);  // convert hour angle to minute angle
   a = hand_angle(hr);
   x = (int) ((ACLOCK_R-AA*3) * cos(a)); 
   y = (int) ((ACLOCK_R-AA*3) * sin(a)); 

   if(fancy_hands) {  // use trapazoidal hands
      if(fancy_hands > 1) ang = FANCY_ANGLE;  // hollow hands
      else ang = 0.0;  // filled hands

      while(ang<=FANCY_ANGLE) {
         xx = (int) ((ACLOCK_R-AA*3)*FANCY_BREAK * cos(a+ang*PI/180.0)); 
         yy = (int) ((ACLOCK_R-AA*3)*FANCY_BREAK * sin(a+ang*PI/180.0));

         xxx = (int) ((ACLOCK_R-AA*3)*FANCY_BREAK * cos(a-(ang*PI/180.0))); 
         yyy = (int) ((ACLOCK_R-AA*3)*FANCY_BREAK * sin(a-(ang*PI/180.0)));

         thick_line(aclock_x,aclock_y,  aclock_x+xx,aclock_y+yy, time_color, thickness);
         thick_line(aclock_x,aclock_y,  aclock_x+xxx,aclock_y+yyy, time_color, thickness);
         thick_line(aclock_x+x,aclock_y+y,  aclock_x+xx,aclock_y+yy, time_color, thickness);
         thick_line(aclock_x+x,aclock_y+y,  aclock_x+xxx,aclock_y+yyy, time_color, thickness);
         ang += FANCY_STEP;
      }
   }
   else {  // linear hands
     if(zoom_screen == 'X') thick_line(aclock_x,aclock_y,  aclock_x+x,aclock_y+y, BLACK, thickness+2);
     thick_line(aclock_x,aclock_y,  aclock_x+x,aclock_y+y, time_color, thickness);
   }

   time_color = old_time_color;
}



void draw_moon(double jd, int x0,int y0, int r, int watch)
{
int Xpos, Ypos, Rpos;
int Xpos1, Xpos2;
int y, x1,x2;
int t;
double age;
double synod;  // length of the synodic month
int moon_r;

    // This routine draws an image of the moon phase on the analog watch display
    // This code derived from code by Mostafa Kaisoun.
    if(MOON_PRN == 0) return;
    if(no_sun_moon & 0x02) return;
//  if(zoom_screen == 'I') return;

    if(0) {
       age = moon_age(jd);           // calc the moon phase age in days since last new moon

//     synod = (jd - JD2000) / 36525.0;
//     synod = 29.5305888531 + 0.00000021621*synod - 3.64E-10*synod*synod;  // average length of synodic month
       synod = moon_synod(jd);
sprintf(plot_title, "synod %f: %f  calc:%f age:%f  mage2:%f",
jd, synod, moon_synod(jd), age, age/synod);
       if(synod) age = age / synod;  // normalize moon age to 0.0 .. 1.0
       else age = 0.0;
                                                                         // ... which may be +/- 6 hours of the actual value
    }
    else if(MoonPhase) {   // we got MoonPhase from moon_info() call in primary_misc()
       age = MoonPhase / 2.0;
    }
    else {
       age = moon_phase(jd) / 2.0;
    }

    if(age < 0.5) age = age + 0.50;
    else                age = age - 0.50;
    if(age > 1.0) age = 1.0;
    else if(age < 0.0) age = 0.0;
    age *= 2.0;

//  x0 = aclock_x;
//  y0 = aclock_y;
//  r = ACLOCK_SIZE;

    if(watch == 0) {
       sat[MOON_PRN].plot_x = x0 + moon_x;
       sat[MOON_PRN].plot_y = y0 + moon_y;
       sat[MOON_PRN].plot_r = MOON_SIZE;
       sat[MOON_PRN].plot_color = moon_color;
    }

    moon_r = 0;
    for(Ypos=0; Ypos<=MOON_SIZE; Ypos++) {  // draw the moon        
       Xpos = (int)(sqrt((MOON_SIZE*MOON_SIZE) - ((double) Ypos * (double) Ypos)));

       // determine the edges of the lighted part of the moon       
       Rpos = 2 * Xpos;
       if(age < 1.0) {
          Xpos1 = 0 - Xpos;
          Xpos2 = (int)(Rpos - Xpos - (int) (age*(double)Rpos));
       }
       else {
          Xpos1 = Xpos;
          Xpos2 = (int)(Rpos + Xpos - (int) (age*(double)Rpos));
       }       

       // Draw the lighted part of the moon       

       x1 = x0+moon_x+Xpos1;
       x2 = x0+moon_x+Xpos2;
       if(x1 > x2) { 
          t = x1;
          x1 = x2;
          x2 = t;
       }
       if(zoom_screen) ;
       else {  // clip off any part outside the drawing area
          if(x1 <= (x0-r+1)) x1 = x0-r+2;
          if(x1 >= (x0+r-1)) x1 = x0+r-2;
          if(x2 <= (x0-r+1)) x2 = x0-r+2;
          if(x2 >= (x0+r-1)) x2 = x0+r-2;
       }

       if(x1 > x2) {
          t = x1;
          x1 = x2;
          x2 = t;
       }

       y = y0+moon_y-Ypos;
       if(y > (y0-r)) {
          line(x1,y, x2,y, moon_color);
          dot(x1-2,y, BLACK);   // outline moon in black
          dot(x1-1,y, BLACK);
          dot(x2+1,y, BLACK);
          dot(x2+2,y, BLACK);
       }

       y = y0+moon_y+Ypos;
       if(y < (y0+r)) {
          line(x1,y, x2,y, moon_color);
          dot(x1-2,y, BLACK);   // outline moon in black
          dot(x1-1,y, BLACK);
          dot(x2+1,y, BLACK);
          dot(x2+2,y, BLACK);
       }
    }    
}


#define SUN_STRING "\002"

int small_azel()
{
   // see if azel plot is too small to label 
   if((AZEL_SIZE < AZEL_LABEL_THRESH) && (zoom_screen == 0)) { 
      return 1;
   }
   return 0;
}

void draw_sun_rays(int x,int y, int marker_size, int watch)
{
int k;
int thickness;
int ray_len;
int ray_gap;
#define RAY_LEN 4
#define RAY_GAP (marker_size / 3)
int color;
int clip_left_x, clip_right_x;
int clip_top_y, clip_bottom_y;
int NO_CLIP;

   color = sat_colors[YELLOW_SAT_COLOR];

   last_sun_x = x;
   last_sun_y = y;
   last_sun_r = marker_size;

   thickness = 1;
   if(!small_azel()) thickness = 2;

   // erase the satellite map area on the screen
   if((zoom_screen == 'W') || ((zoom_screen == 0) && NO_SATS)) {
      clip_left_x = aclock_x-ACLOCK_R;
      clip_right_x = aclock_x+ACLOCK_R;
      clip_top_y = aclock_y-ACLOCK_R;
      clip_bottom_y = aclock_y+ACLOCK_R;
   }
   else if(watch && ((zoom_screen == 0) || (zoom_screen == 'V'))) {
      clip_left_x = aclock_x-ACLOCK_R;
      clip_right_x = aclock_x+ACLOCK_R;
      clip_top_y = aclock_y-ACLOCK_R;
      clip_bottom_y = aclock_y+ACLOCK_R;
   }
   else {
      clip_left_x = AZEL_X-AZEL_RADIUS;
      clip_right_x = AZEL_X+AZEL_RADIUS;
      clip_top_y = AZEL_Y-AZEL_RADIUS;
      clip_bottom_y = AZEL_Y+AZEL_RADIUS;
   }
//draw_rectangle(clip_left_x,clip_top_y, clip_right_x-clip_left_x,clip_bottom_y-clip_top_y, WHITE);
   clip_right_x -= thickness;
   clip_left_x += thickness;
   clip_top_y += thickness;
   clip_bottom_y -= thickness;
//draw_rectangle(clip_left_x,clip_top_y, clip_right_x-clip_left_x,clip_bottom_y-clip_top_y, RED);

   ray_gap = RAY_GAP;
   ray_len = RAY_LEN;

   k = marker_size + ray_gap;

NO_CLIP = 0;

   if(NO_CLIP || (x+k+ray_len < clip_right_x))  {
      thick_line(x+k,y, x+k+ray_len,y, color, thickness);
   }
   if(NO_CLIP || (x-k-ray_len > clip_left_x)) {
      thick_line(x-k,y, x-k-ray_len,y, color, thickness);
   }
   if(NO_CLIP || (y-k-ray_len > clip_top_y)) {
      thick_line(x,y-k, x,y-k-ray_len, color, thickness);
   }
   if(NO_CLIP || (y+k+ray_len < clip_bottom_y)) {
      thick_line(x,y+k, x,y+k+ray_len, color, thickness);
   }

   k = (int) (((double) k) * 0.707);
   if(NO_CLIP || ((x+k+ray_len < clip_right_x) && (y+k+ray_len < clip_bottom_y))) {
      thick_line(x+k,y+k, x+k+ray_len,y+k+ray_len, color, thickness);
   }
   if(NO_CLIP || ((x-k-ray_len > clip_left_x) && (y+k+ray_len < clip_bottom_y))) {
      thick_line(x-k,y+k, x-k-ray_len,y+k+ray_len, color, thickness);
   }
   if(NO_CLIP || ((x+k+ray_len < clip_right_x) && (y-k-ray_len > clip_top_y))) {
       thick_line(x+k,y-k, x+k+ray_len,y-k-ray_len, color, thickness);
   }
   if(NO_CLIP || ((x-k-ray_len > clip_left_x) && (y-k-ray_len > clip_top_y))) {
      thick_line(x-k,y-k, x-k-ray_len,y-k-ray_len, color, thickness);
   }
}


int set_marker_size(int prn)
{
int marker_size;

   if(prn < 0) return 0;
   if(prn > SUN_MOON_PRN) return 0;

   marker_size = (int) sat[prn].sig_level;
if((rcvr_type == UBX_RCVR) && (prn != SUN_PRN)) marker_size += (marker_size / 3);  // UBX receivers have smaller sig level values
   if(marker_size < 0) marker_size = 0 - marker_size;
   if(amu_mode == 0) {
       marker_size -= 30;  // 30 dBc represents our minimum marker size
   }

   if(marker_size > MAX_MARKER_SIZE) marker_size = MAX_MARKER_SIZE;   // and 44 dBc is our max size
   else if(marker_size < MIN_MARKER_SIZE) marker_size = MIN_MARKER_SIZE;
   marker_size = (int) (((float) marker_size) / (float) MAX_MARKER_SIZE * (float) AZEL_SIZE/20.0F);
   if(marker_size < MIN_MARKER_SIZE) marker_size = MIN_MARKER_SIZE;

   return marker_size;
}

void draw_watch_sun()
{
double az, el;
int x0,y0;
int r;
int x, y;

   if(have_sun_el == 0) return;
   if(zoom_screen == 'B') return;   // use the sun from the sat map
   if(zoom_screen == 'W') ;
   else if(zoom_screen == 'V') ;
   else if(map_and_watch) return;   // sun was part of the satellite map display
   if(SUN_PRN == 0) return;
   if(no_sun_moon & 0x01) return;

   x0 = aclock_x;
   y0 = aclock_y;
   r = ACLOCK_R;

   el = sun_el;
   if(el < 0.0) el = 0.0 - el;
   if((el < 0.0F) || (el > 90.0F)) return;
   el = (el - 90.0F) / 90.0F;

   az = sun_az;
   if((az < 0.0F) || (az >= 360.0F)) return;
   az += 90.0F;
   if(az > 360.0) az -= 360.0;

   x = x0 + (int) (el * cos360(az) * r);
   if(x > x0) x -= 1;   // clipping tweak
   else       x += 1;
   y = y0 + (int) (el * sin360(az) * r);

   if(sun_el >= 0.0) draw_circle(x,y, r/16, YELLOW, 1);
   else              draw_circle(x,y, r/16, YELLOW, 0);

   draw_sun_rays(x,y, r/16, 1);
// draw_sun_rays(x,y, set_marker_size(SUN_PRN), 1);
}


void draw_watch_alarms()
{
int x0,y0;
int x1,y1;
int len;
int timer_theta, alarm_theta;
double jd_timer;

   // draw tick marks on the watch for alarm and egg timer

   len = ACLOCK_R / 20;
   if(len < 5) len = 5;
   else if(len > 15) len = 15;

   if(alarm_date || alarm_time) {  // highlight the alarm clock time
      alarm_theta = ((alarm_hh%WATCH_HOURS)*WATCH_STEP*2)+alarm_mm;
      alarm_theta /= 2;
      alarm_theta -= 90;
      while(alarm_theta < 0) alarm_theta += 360;
      alarm_theta %= 360;

      x0 = (int) ((ACLOCK_R-0) * cos360(alarm_theta));
      y0 = (int) ((ACLOCK_R-0) * sin360(alarm_theta));

      x1 = (int) ((ACLOCK_R+len) * cos360(alarm_theta));
      y1 = (int) ((ACLOCK_R+len) * sin360(alarm_theta));
      thick_line(aclock_x+x0,aclock_y+y0, aclock_x+x1,aclock_y+y1, ALARM_COLOR, 3);
   }    


   if(egg_timer) {  // highlight the egg time time
      jd_timer = jd_local + jtime(0,0,0, (double) egg_timer);
      jd_timer = jd_timer - (double) (int) jd_timer; 
      jd_timer = (jd_timer * 720.0);
      timer_theta = (int) jd_timer;
      timer_theta -= 90;
      while(timer_theta < 0) timer_theta += 360;
      timer_theta %= 360;

      x0 = (int) ((ACLOCK_R-0) * cos360(timer_theta));
      y0 = (int) ((ACLOCK_R-0) * sin360(timer_theta));

      x1 = (int) ((ACLOCK_R+len) * cos360(timer_theta));
      y1 = (int) ((ACLOCK_R+len) * sin360(timer_theta));
      thick_line(aclock_x+x0,aclock_y+y0, aclock_x+x1,aclock_y+y1, TIMER_COLOR, 3);
   }
}


int draw_watch(int why)
{
   if(text_mode) return 1;
   if(measure_jitter) return 1;
//sprintf(plot_title, "draw watch:%d", why);

   if((zoom_screen == 'W') || (zoom_screen == 'B') || (zoom_screen == 'X')) ;
   else if(zoom_screen == 'Y') ;
   else if(zoom_screen == 'V') ;
   else if(plot_watch == 0) return 2;
   else if(first_key && shared_plot && (shared_item & SHARE_WATCH)) return 66;
   else if(all_adevs && plot_lla && (WIDE_SCREEN == 0)) return 3;
   else if(plot_lla && lla_showing) {
//.     if(SMALL_SCREEN) return 15;
////  if((shared_item & SHARE_WATCH) == 0) return 15;
if(SMALL_SCREEN && ((shared_item & SHARE_WATCH) == 0)) {
////return 15;
}
   }

if(zoom_screen == 'B') {
   ACLOCK_SIZE = AZEL_SIZE;
   aclock_x = AZEL_X;        //SCREEN_WIDTH/2 - TEXT_WIDTH*2;
   aclock_y = AZEL_Y;        //TEXT_HEIGHT*2 + ACLOCK_SIZE/2;
}

   if((zoom_screen == 'W') || (zoom_screen == 'B') || (zoom_screen == 'X')) ;
   else if(zoom_screen == 'Y') ;
   else if(zoom_screen == 'V') ;
   else if(map_and_watch) ;
   else if(shared_plot) {
      if(plot_signals && zoom_screen) return 4; 
      if(plot_lla && (SCREEN_WIDTH < NARROW_SCREEN)) return 5;
   }
   else {  // see if no room for both watch and lla
      if(plot_signals) return 6;
      if(plot_lla && (WIDE_SCREEN == 0)) return 7;
      if(SCREEN_WIDTH < NARROW_SCREEN) return 8; 
   }

   if(first_key && (aclock_y > PLOT_ROW)) return 9;  // watch would be in active help screen
   if(aclock_y > PLOT_ROW) shared_item = SHARE_WATCH;


   if(MESS && map_and_watch && map_and_sigs && (plot_areas == 1)) ;
   else if(zoom_screen == 'X') ;
   else if(zoom_screen == 'Y') ;
   else erase_watch();

   draw_watch_face();
   draw_watch_outline();
   draw_watch_alarms();

   if(zoom_screen == 'X') ;
   else if((lat != 0.0) || (lon != 0.0)) {
      moon_color = YELLOW;
      #ifdef TRUE_MOON
         calc_moon_posn(1);
      #endif

      // don't draw moon twice in the watch/map areas because
      // they have slighly different sizes and locations.
      if(zoom_screen) goto watch_moon;
      if(map_and_watch && map_and_sigs && (plot_areas == 1)) ;
      else if(map_and_watch && (map_and_sigs == 0) && (plot_areas == 2)) ;
      else if(map_and_watch && (map_and_sigs == 0) && (plot_areas == 1)) ;
      else {  // watch and map do not overlap, ok to draw watch's moon
         watch_moon:
         draw_moon(jd_utc, aclock_x,aclock_y, ACLOCK_SIZE/2, 1);
         draw_watch_sun();
      }

      if(0 && ((zoom_screen == 'W') || ((zoom_screen == 'B') && (SCREEN_WIDTH >= MEDIUM_WIDTH)))) {
         show_sun_moon(TIME_ROW-1, TIME_COL);
      }
   }

   draw_watch_hands();
   return 0;
}
#endif // analog_clock


//
//
//  Zoomed calendar
//
//

void zoom_cal(int year, int month)
{
int day;
int dow;
int row, col;
int top;
int color;
int i, j;
int CAL_WIDTH;
int CAL_LEFT;
int pre_month;
double jd;
char *days;

   // Show a zoomed full screen calendar
   
   CAL_WIDTH = 30;
   CAL_LEFT = 2;
   i = 4;
   days = "SUN MON TUE WED THU FRI SAT";
   if(SMALL_SCREEN) {
      CAL_WIDTH = 24;
      CAL_LEFT = 2;
      i = 3;
      days = "SU MO TU WE TH FR SA";
   }

   CAL_LEFT = (TEXT_COLS - ((CAL_WIDTH*3) + (i*2))) / 2;

   if(month < 1) month = 1;
   if(month > 12) month = 12;

   erase_screen();

   col = CAL_LEFT;
   top = 0;
   if(SCREEN_HEIGHT < SHORT_SCREEN) pre_month = 1; // start with 1 previous months
   else pre_month = 4;  // 4 previous months
   month = month - pre_month;
   if(month <= 0) { 
      --year;
      month += 12;
   }

   for(j=1; j<=48; j++) {  // display up to 48 months
      init_dsm(year);
      calc_dst_times(year, dst_list[dst_area]);  // find daylight savings change times

      row = 0;

      dow = day_of_week(1, month, year);
      if(year < 0) sprintf(out, "%s %d BC", full_months[month], 0-year);
      else         sprintf(out, "%s %04d",  full_months[month], year);
      i = ((CAL_WIDTH-3) - strlen(out)) / 2;
      color = WHITE;
      if((month == pri_month) && (year == pri_year)) color = GREEN;
      else if(j <= pre_month) color = DIM_WHITE;
      vidstr(top+row, col+i, color, out);  // center year/month string
      ++row;

      vidstr(top+row, col, color, days);
      ++row;

      jd = jdate(year,month,1);
      calc_moons(jd, 3); 

      for(day=1; day<=dim[month]; day++) {  // show the day values
         dow = day_of_week(day, month, year);

         if(SMALL_SCREEN) sprintf(out, "%-3d", day);
         else             sprintf(out, "%-4d", day);
         i = col + (dow * strlen(out));

         if((month == dst_start_month) && (day == dst_start_day)) color = BLUE;
         else if((month == dst_end_month) && (day == dst_end_day)) color = BLUE;
         else if(moons[day][0] && (moons[day][0] != 'Q')) {
            if(strstr(moons[day], "Full")) color = YELLOW;     // full moon
            else if(strstr(moons[day], "Blue")) color = YELLOW; // blue moon
            else color = BROWN;  // new moon / black moon
         }
         else if(is_holiday(year,month,day)) color = CYAN;
         else if((month == pri_month) && (year == pri_year)) color = GREEN;
         else if(j <= pre_month) color = DIM_WHITE;
         else color = WHITE;
         if((pri_year == year) && (pri_month == month) && (pri_day == day) && (pri_seconds & 1)) {
            color = BLACK;  // blink today's date
         }
         vidstr(top+row, i, color, out);

         if(dow >= 6) {  // end of week, go to next row
            ++row;
         }
      }


      col += CAL_WIDTH;  // setup for next month
      if(col > ((CAL_WIDTH*2)+CAL_LEFT)) {
         col = CAL_LEFT;
         top += 8;
      }
      if(top >= (TEXT_ROWS-8)) break;

      ++month;
      if(month > 12) {
         month = 1;
         year += 1;
      }
   }

   init_dsm(this_year);
   calc_dst_times(this_year, dst_list[dst_area]);  // find daylight savings change times
   calc_moons(jd_local, 5); 
}


#ifdef AZEL_STUFF

//
//
// AZIMUTH/ELEVATION plots
//
//

void tick_radial(int grid_x,int grid_y, int grid_r, int max_el, int az)
{
int x0,y0;
int x1,y1;
int x2,y2;
int el;
double len;
double r;

   x0 = grid_x;
   y0 = grid_y;
   r = (double) grid_r;
   if(plot_signals == 3) {
      x0 -= grid_r;
      y0 += grid_r;
      r *= 2.0F;
   }

   for(el=0; el<max_el; el+=5) {
      len = (r * ((double)(max_el-el)/(double) max_el));
      x1 = (int) (len * cos360(az-2));
      y1 = (int) (len * sin360(az-2));

      x2 = (int) (len * cos360(az+2));
      y2 = (int) (len * sin360(az+2));

      line(x0+x1,y0+y1, x0+x2,y0+y2, azel_grid_color);
   }
}

void draw_azel_grid(int signals, int grid_x,int grid_y, int grid_r)
{
int az, el;
int x0, y0;
float x1, y1;
float x2, y2;
float elx;
float len;
float r;
int max_el, el_grid;

   x0 = grid_x;
   y0 = grid_y;
   len = (float) grid_r;
   if(signals && (plot_signals == 3)) {
      x0 -= grid_r;
      y0 += grid_r;
      len *= 2.0F;
   }

   x1 = cos360(0) * len;
   y1 = sin360(0) * len;

   for(az=AZ_INCR; az<=360; az+=AZ_INCR) {
      x2 = cos360(az) * len;
      y2 = sin360(az) * len;

      if(signals && (plot_signals == 3) && ((az-AZ_INCR) > 90) && ((az-AZ_INCR) < 360)) {
         goto skip_it;
      }

      if(1 || (plot_signals == 0) || (signals && ((plot_signals >= 4)))) {
         r = 1.03F;  // extend radials out a smidge
         max_el = 90;
         el_grid = 30;
      }
      else {
         r = 1.0F;
         max_el = 100;
         el_grid = 25;
      }

      if(((az-AZ_INCR)%AZ_GRID) == 0) {  // draw azimuth vectors
         line(x0+0,y0-0, 
              x0+(int)(x1*r),y0-(int)(y1*r), azel_grid_color);
         if(signals && plot_signals) {
            if(plot_signals >= 4) tick_radial(grid_x,grid_y,grid_r, 90, (az-AZ_INCR)+270);
            else                  tick_radial(grid_x,grid_y,grid_r,100, (az-AZ_INCR)+270);
         }
      }

      // tick mark the 0 degree elevation circle
      line(x0+(int)(x1*0.99F),y0-(int)(y1*0.99F),  
           x0+(int)(x1*1.03F),y0-(int)(y1*1.03F), azel_grid_color); 

      // dot the satelite elevation mask angle circle
      if(plot_el_mask == 0) ;
      else if((signals == 0) && (map_and_watch || (map_and_sigs == 0) || (zoom_screen == 'B'))) { 
         elx = ((90.0F-el_mask) / 90.0F);
         dot(x0+(int)(x1*elx),y0-(int)(y1*elx), azel_grid_color); 
      }
      else if((plot_signals == 0) || (signals && ((plot_signals > 3)))) { 
         elx = ((90.0F-el_mask) / 90.0F);
         dot(x0+(int)(x1*elx),y0-(int)(y1*elx), azel_grid_color); 
      }

      // draw the elevation circles
//      for(el=0; el<max_el; el+=el_grid) {  
      for(el=0; el<=max_el; el+=el_grid) {  
         if(max_el == 0) break;
         elx = r = ((((float) max_el) - (float) el) / (float) max_el);

         if(plot_el_mask && (el == (int) el_mask) && (el_mask != 0.0F)) {
            continue;  
         }
         if((zoom_screen == 'B') && (el == 0)) continue;
         if(zoom_screen == 'M') ;
         else if(zoom_screen == 'S') ;
         else if(zoom_screen == 'Y') ;
         else if(zoom_screen == 'U') ;
         else if(zoom_screen == 'V') ;
         else if(zoom_screen == 'I') ;
         else if(map_and_watch && (el == 0)) continue;
         if(signals && (plot_signals == 3) && ((az-AZ_INCR) == 90)) continue;

         line(x0+(int)(x1*elx),y0-(int)(y1*elx), 
              x0+(int)(x2*elx),y0-(int)(y2*elx), azel_grid_color);
      }

      skip_it:
      x1 = x2;  y1 = y2;
   }
}




#ifdef SAT_TRAILS
#define TRAIL_INTERVAL 5   // minutes between trail reference points (must divide into 60)

#define TRAIL_POINTS   ((6*60)/TRAIL_INTERVAL)  // number of points for 6 hours of data (the max a sat can be above the horizon)
#define TRAIL_MARKER   30                       // dot trail every 30 minutes (must divide into 60)

int trail_count;    // how many reference points that we have accumulated

struct SAT_TRAIL {  // the satellite location history (newest to oldest)
   float az[TRAIL_POINTS+1];  // (az<<6) | (lower 6 bits are minutes)
   float el[TRAIL_POINTS+1];
   int mins[TRAIL_POINTS+1];
} sat_trail[MAX_PRN+2+1];   // one member for each sat plus sun and moon

void clear_sat_trails()
{
int i;
int prn;

   for(prn=0; prn<=MOON_PRN; prn++) {
      for(i=0; i<TRAIL_POINTS; i++) {  // clear the sat's position list
         sat_trail[prn].az[i] = 0.0F;
         sat_trail[prn].el[i] = 0.0F;
         sat_trail[prn].mins[i] = 0;
      }
   }
   trail_count = 0;
}


void update_sat_trails()
{
int prn;
int i;
float az,el;

   if(pri_seconds != TRAIL_UPDATE_SECOND) return;      // keeps too many things from happening at the same time
   if((pri_minutes % TRAIL_INTERVAL) != 0) return;

   for(prn=1; prn<=SUN_MOON_PRN; prn++) {       // !!!! mooooo for each satellite
      az = (sat[prn].azimuth);      // get current position
      el = (sat[prn].elevation);
      if((az < 0.0F) || (az > 360.0F)) az = el = 0.0F;  // validate it
      else if((prn == SUN_PRN) || (prn == MOON_PRN)) {
         if((el < -90.0F) || (el > 90.0F)) az = el = 0.0F;
      }
      else if((el < 0.0F) || (el > 90.0F)) az = el = 0.0F;

      // duplicate or zero entry means sat is no longer tracked
      if(((az == 0.0F) && (el == 0.0F)) || ((sat_trail[prn].az[0] == az) && (sat_trail[prn].el[0] == el) && (i != SUN_PRN) && (i != MOON_PRN))) {  
         sat[prn].azimuth = 0.0F;
         sat[prn].elevation = 0.0F;
         for(i=0; i<TRAIL_POINTS; i++) {  // clear the sat's position list
            sat_trail[prn].az[i] = 0.0F;
            sat_trail[prn].el[i] = 0.0F;
            sat_trail[prn].mins[i] = 0;
         }
      }
      else if(1) {   // sat is being tracked
         for(i=trail_count; i>0; i--) {  // shift the old positions down one slot
            sat_trail[prn].az[i]   = sat_trail[prn].az[i-1];
            sat_trail[prn].el[i]   = sat_trail[prn].el[i-1];
            sat_trail[prn].mins[i] = sat_trail[prn].mins[i-1];
         }

         // insert current position at top of the list
         sat_trail[prn].az[0] = az;
         sat_trail[prn].el[0] = el;
         sat_trail[prn].mins[0] = (int) pri_minutes;
      }
   }

   ++trail_count;
   if(trail_count >= TRAIL_POINTS) {  // the trail list is now full
      trail_count = TRAIL_POINTS-1;
   }
}

void plot_sat_trail(int grid_x,int grid_y, int grid_r,  int prn, float az,float el,  int color, int dots)
{
int x1,y1;
int x2,y2;
int i;
int row;
u08 m;

   // Draw the trail of previous sat positions.  The trail is marked with colored
   // on the hour and black dots otherwise  as time markers.  

   if(map_trails == 0) return;  // trails disabled
   if((prn == SUN_PRN) && (no_sun_moon & 0x01)) return;
   if((prn == MOON_PRN) && (no_sun_moon & 0x02)) return;

   m = 0;
   row = 0;

   // the current location of the sat
   if(az < 0.0F) az += 360.0F;
   if(az >= 360.0F) az -= 360.0F;
   x1 = grid_x + (int) ((el * cos360(az) * grid_r)+0.50);
   y1 = grid_y + (int) ((el * sin360(az) * grid_r)+0.50);
   if(x1 < 0) return;
   if(y1 < 0) return;
   if(x1 >= SCREEN_WIDTH) return;
   if(y1 >= SCREEN_WIDTH) return;

   for(i=0; i<trail_count; i++) { // check the old locations of the sat
      el = (float) sat_trail[prn].el[i];
      az = (float) sat_trail[prn].az[i];
      if((az == 0.0F) && (el == 0.0F)) break;  // end of valid locations
      if((prn == MOON_PRN) || (prn == SUN_PRN)) {  // moon color depends upon elevation
         if(el < 0.0F) color = GREY;  // !!!! this does not work since saved el is always positive
         else color = MOON_COLOR;
//sprintf(debug_text2, "moon: el:%f  moon_el:%f  color:%d", el, moon_el, color);
      }
      if(el < 0.0F) el = 0.0F - el;

      el = (el - 90.0F) / 90.0F;
      az += 90.0F;
      if(az < 0.0F) az += 360.0F;
      if(az >= 360.0F) az -= 360.0F;

      x2 = grid_x + (int) ((el * cos360(az) * grid_r)+0.50);
      y2 = grid_y + (int) ((el * sin360(az) * grid_r)+0.50);
      if(x2 < 0) continue;
      if(y2 < 0) continue;
      if(x2 >= SCREEN_WIDTH) continue;
      if(y2 >= SCREEN_WIDTH) continue;

      if(dots) {  // mark the trail with time markers
         if((sat_trail[prn].mins[i] % TRAIL_MARKER) == 0) {    // it's time to mark the trail
            if(sat_trail[prn].mins[i] != 0) {  // it's not the hour... dot the dot
               draw_circle(x2,y2, 2, color, 0);  // hollow circle
            }
            else {
               draw_circle(x2,y2, 2, color, 1);  // filled circle
            }
         }
      }
      else {   // draw the trail line
         line(x1,y1, x2,y2, color); 
      }

      x1 = x2;
      y1 = y2;
   }
}
#endif

void log_sun_posn()
{
   // record the position of the sun (as satellite PRN 1000)

   sunrise = sunset = 0;
   if(have_sun_el == 0) ;
   else if((last_sun_el < 0.0) && (sun_el >= 0.0)) sunrise = 1;
   else if((last_sun_el >= 0.0) && (sun_el < 0.0)) sunset = 1;
   last_sun_el = sun_el;

   sat[SUN_PRN].level_msg = 1;
   sat[SUN_PRN].sig_level = (-1.0);
   sat[SUN_PRN].azimuth = (float) sun_az;
   if(sun_el >= 0.0) {
      sat[SUN_PRN].elevation = (float) sun_el;
      sat[SUN_PRN].sig_level = (40.0F);
      sat[SUN_PRN].tracking = 1;
   }
   else {
      sat[SUN_PRN].elevation = 0.0F - (float) sun_el;
      sat[SUN_PRN].sig_level = (-40.0F);
      sat[SUN_PRN].tracking = 0;
   }
sat[SUN_PRN].elevation = (float) sun_el;
   if(sat[SUN_PRN].elevation > 90.0) sat[SUN_PRN].elevation = 90.0;
   if(sat[SUN_PRN].elevation < -90.0) sat[SUN_PRN].elevation = -90.0;
   if(sat[SUN_PRN].azimuth > 360.0) sat[SUN_PRN].azimuth -= 360.0;


   sat[MOON_PRN].level_msg = 1;
   sat[MOON_PRN].sig_level = (-1.0);
   sat[MOON_PRN].azimuth = (float) moon_az;
   if(moon_el >= 0.0) {
      sat[MOON_PRN].elevation = (float) moon_el;
      sat[MOON_PRN].sig_level = (40.0F);
      sat[MOON_PRN].tracking = 1;
   }
   else {
      sat[MOON_PRN].elevation = 0.0F - (float) moon_el;
      sat[MOON_PRN].sig_level = (-40.0F);
      sat[MOON_PRN].tracking = 0;
   }

sat[MOON_PRN].elevation = (float) moon_el;
   if(sat[MOON_PRN].elevation > 90.0) sat[MOON_PRN].elevation = 90.0;
   if(sat[MOON_PRN].elevation < -90.0) sat[MOON_PRN].elevation = -90.0;
   if(sat[MOON_PRN].azimuth > 360.0) sat[MOON_PRN].azimuth -= 360.0;
}

int blink_sat()
{
   if(blink_prn >= 0) return blink_prn;    // user specified sat
   else if(blink_prn == BLINK_HIGHEST) return highest_sat();
   else if(blink_prn == BLINK_PLOT) return plot_prn;

   return 0;
}

void draw_sat(int prn, int x, int y, int r, int color, int fill)
{
int i,j;
int clip_left_x, clip_right_x;
int clip_top_y, clip_bottom_y;
int thickness;

   // draw the satellite on the map

   if(blink_sat() && (prn == blink_sat()) && (seconds & 1)) {  // blink the specified sat
      color = BLACK;
   }

   thickness = 1;
   if(!small_azel()) thickness = 2;

   if((zoom_screen == 'W') || ((zoom_screen == 0) && NO_SATS)) {
      clip_left_x = aclock_x-ACLOCK_R;
      clip_right_x = aclock_x+ACLOCK_R;
      clip_top_y = aclock_y-ACLOCK_R;
      clip_bottom_y = aclock_y+ACLOCK_R;
   }
   else {
      clip_left_x = AZEL_X-AZEL_RADIUS;
      clip_right_x = AZEL_X+AZEL_RADIUS;
      clip_top_y = AZEL_Y-AZEL_RADIUS;
      clip_bottom_y = AZEL_Y+AZEL_RADIUS;
   }
clip_right_x -= thickness;
clip_left_x += thickness;
clip_top_y += thickness;
clip_bottom_y -= thickness;


   if(fancy_sats == ROUND_SATS) {  // simple circular sats
      draw_circle(x,y, r, color, fill);
   }
   else if(fancy_sats == RECT_SATS) {  // simple rectangular sats
      i = j = r;
      if(fill) fill_rectangle(x-i,y-j, i+i,j+j, color);
      else     draw_rectangle(x-i,y-j, i+i,j+j, color);
   }
   else { // fancy sats
      if((fancy_sats == RECT_WINGS) || (fancy_sats == RECT_LINES)) {  // rectangle with wings
         i = (r*4) / 10;
         if(i < 2) i = 2;

         j = (r*3)/4;
         if(j < 3) j = 3;

         if(fill) fill_rectangle(x-i,y-j, i+i,j+j, color);
         else     draw_rectangle(x-i,y-j, i+i,j+j, color);
      }
      else if(fancy_sats == ROUND_LINES) {
         i = r*2/3;
         draw_circle(x,y, r*2/3, color, fill);
      }
      else {  // circle with wings
         draw_circle(x,y, r*2/3, color, fill);
      }

      // draw the wings
      if((fancy_sats == RECT_WINGS) || (fancy_sats == ROUND_WINGS)) {
         line(x+r,y+r, x+r,y-r, color);
         line(x-r,y+r, x-r,y-r, color);

         if(fill) {
            for(i=0-r; i<r; i++) {
               line(x+i,y+i, x+i,y-i, color);
            }
         }
         if(color) {
            line(x-r,y+r, x+r,y-r, color);
            line(x+r,y+r, x-r,y-r, color);
         }
      }
      else if(fancy_sats == RECT_LINES) {
         j = 3;
         i += 2;
         if(zoom_screen) j = 4;
         else if((y+r) <= clip_top_y) return;
         else if((y-r) >= clip_bottom_y) return;

         if(zoom_screen || ((x+r-2) <= clip_right_x)) {
            thick_line(x+r-2,y+r, x+r-2,y-r, color,j);
         }
         if(zoom_screen || ((x-r) >= (clip_left_x))) {
            thick_line(x-r,y+r,   x-r,y-r,   color,j);
         }

         if(zoom_screen) {
            thick_line(x+r-2,y, x+r-2-i-j,  y, color,j);
            thick_line(x-r,y,   x-r+i+(j-1),y, color,j);
         }
         else {
            if((x+r-1) <= clip_right_x) {
               thick_line(x+r-1,y, x+r-i-j-0,y, color,j);
            }
            if((x-r) >= clip_left_x) {
               thick_line(x-r,y,   x-r+i+j-1,y, color,j);
            }
         }
      }
      else if(fancy_sats == ROUND_LINES) {
         j = 3;
         if(zoom_screen) j = 4;
         else if((y+r) <= clip_top_y) return;
         else if((y-r) >= clip_bottom_y) return;

         if(zoom_screen || ((x+r-2) <= clip_right_x)) {
            thick_line(x+r-2,y+r, x+r-2,y-r, color,j);
         }
         if(zoom_screen || ((x-r) >= clip_left_x)) {
            thick_line(x-r,y+r,   x-r,y-r,   color,j);
         }

         if(zoom_screen || ((x+r-2) <= clip_right_x)) {
            thick_line(x+r-2,y, x+r-i/2,y,   color,j);
         }
         if(zoom_screen || ((x-r) >= clip_left_x)) {
            thick_line(x-r,y,   x-r+i/2-1,y, color,j);
         }
      }
   }
}

void draw_azel_sats(int grid_x,int grid_y, int grid_r)
{
float az,el;
int x, y;
int marker_size;
int row;
int i, j;
int color;
u08 m;

   // Draw the sats on the sat position map.  Sats that are being used are
   // drawn colored in.  Sats that are not being used are drawn as hollow
   // circles.

   m = 0;
   j = 0;
   row = 0;

   map_x = grid_x;
   map_y = grid_y;
   map_r = grid_r;

   if(AZEL_Y >= PLOT_ROW) shared_item = SHARE_MAP;
   if(map_and_watch && (shared_item == SHARE_MAP)) {
      if(0 && plot_lla && lla_showing) ;
      else shared_item = (SHARE_MAP | SHARE_WATCH);
   }

   log_sun_posn();
   if(1 || (shared_plot == 0) || (aclock_y < (SCREEN_HEIGHT/3)) || (zoom_screen == 'M') || (zoom_screen == 'X') || (zoom_screen == 'Y')) {
      #ifdef TRUE_MOON
         calc_moon_posn(0);
      #endif
      if((lat != 0.0) || (lon != 0.0)) {
         draw_moon(jd_utc, grid_x,grid_y, grid_r, 0);
      }
   }
   
   drawn_sats = 0;
   for(i=1; i<=SUN_MOON_PRN; i++) { // !!!!! mooooo  now draw the sat vectors
      if(sat[i].level_msg == 0x00) continue;
      if((i == SUN_PRN) && (no_sun_moon & 0x01)) continue;
      if((i == MOON_PRN) && (no_sun_moon & 0x02)) continue;

if(tracked_only & (sat[i].tracking <= 0) && (i != SUN_PRN) && (i != MOON_PRN)) continue;
      if(i == SUN_PRN) ;
      else if(i == MOON_PRN);
      else if(sat[i].sig_level <= 0.0) continue;  // zzzzzzzzzzzzzzz

      if((i == SUN_PRN) && (sun_el == 0.0F) && (sun_az == 0.0F)) continue;
      if((i == MOON_PRN) && (moon_el == 0.0F) && (moon_az == 0.0F)) continue;

      // filter out potentially bogus data
      el = sat[i].elevation;
      az = sat[i].azimuth;

      if((az == 0.0F) && (el == 0.0F)) continue;
      if((az < 0.0F) || (az >= 360.0F)) continue;
      if((el < -90.0F) || (el > 90.0F)) continue;

      if((i == SUN_PRN) || (i == MOON_PRN)) {
         if(el < 0.0F) el = 0.0F - el;
      }
      if((el < 0.0F) || (el > 90.0F)) continue;

      el = (el - 90.0F) / 90.0F;
      az += 90.0F;
      if(az < 0.0F) az += 360.0F;
      if(az >= 360.0F) az -= 360.0F;

      x = grid_x + (int) (el * cos360(az) * grid_r);
      y = grid_y + (int) (el * sin360(az) * grid_r);
      if(x < 0) continue;
      if(y < 0) continue;
      if(x >= SCREEN_WIDTH) continue;
      if(y >= SCREEN_HEIGHT) continue;

      // the size of the sat marker is based upon the signal strength
      marker_size = set_marker_size(i);
  
      
      color = j;
      if(i == SUN_PRN) {
         color = YELLOW_SAT_COLOR;  // !!!! kludge: force YELLOW for the sun
      }
      else if(i == MOON_PRN) {
         if(moon_el < 0) color = GREY_SAT_COLOR;
         else            color = YELLOW_SAT_COLOR;  // !!!! kludge: force YELLOW for the moon
      }


      #ifdef SAT_TRAILS
         // we draw the line first,  then the time marker dots
         // since the dots can be hollow and erase part of the line
         plot_sat_trail(grid_x,grid_y, grid_r,  i, az,el, sat_colors[color], 0);   // plot the satellite's path line 
         if(dot_trails) {
            plot_sat_trail(grid_x,grid_y, grid_r,  i, az,el, sat_colors[color], 1);   // dot the satellite's path with time markers
         }
      #endif

      if(i == MOON_PRN) ;  // we draw the moon separately
      else if((i == SUN_PRN) && (fancy_sats >= RECT_SATS)) {
         if(sat[i].tracking > 0) {  //  mark sats that are actively tracked with a filled box
            draw_circle(x,y, marker_size, sat_colors[color], 1);     
         }
         else {
            draw_circle(x,y, marker_size, sat_colors[color], 0);     
         }
         sat[i].plot_x = x;
         sat[i].plot_y = y;
         sat[i].plot_r = marker_size;
         sat[i].plot_color = sat_colors[color];
      }
      else if(sat[i].tracking > 0) {  //  mark sats that are actively tracked with a filled box
         draw_sat(i, x,y, marker_size, sat_colors[color], 1);     
         ++drawn_sats;
         sat[i].plot_x = x;
         sat[i].plot_y = y;
         sat[i].plot_r = marker_size;
         sat[i].plot_color = sat_colors[color];

         if((fancy_sats <= RECT_SATS) && (map_and_sigs || (zoom_screen == 'X') || (zoom_screen == 'Y'))) {  // outline the sats in black
            draw_sat(i, x,y, marker_size+1, BLACK, 0);
            draw_sat(i, x,y, marker_size+2, BLACK, 0);
         }
      }
      else {  // mark inactive sats with a hollow box
         draw_sat(i, x,y, marker_size, sat_colors[color], 0);          
         ++drawn_sats;
         sat[i].plot_x = x;
         sat[i].plot_y = y;
         sat[i].plot_r = marker_size;
         sat[i].plot_color = sat_colors[color];

         if((fancy_sats <= RECT_SATS) && (map_and_sigs || (zoom_screen == 'X') || (zoom_screen == 'Y'))) {  // outline the sats in black
            draw_sat(i, x,y, marker_size+1, BLACK, 0);
            draw_sat(i, x,y, marker_size+2, BLACK, 0);
         }
         if((fancy_sats <= RECT_SATS) && (marker_size > 1+1)) {
            draw_sat(i, x,y, marker_size-1-1, BLACK, 1);
         }
      }

      if(i == SUN_PRN) {
         draw_sun_rays(x,y, marker_size, 0);
      }
      else if(i == MOON_PRN) {
      }

      ++j;
      if(j > MAX_SAT_COLOR) j = 0;
   }
}

void draw_azel_prns(int signals)
{
int i, j;
int kl, kr;
char el_dir;
float az,el;
int q1_row,q2_row,q3_row,q4_row;
int q3_q4_col, q1_q2_col;
int l, step;
int color;

   // label the round maps

   q4_row = q1_row = ((AZEL_Y - AZEL_SIZE/2) / TEXT_HEIGHT) + 1;
   q3_row = q2_row = ((AZEL_Y + AZEL_SIZE/2) / TEXT_HEIGHT) - 1;
   q3_q4_col = (AZEL_X-AZEL_SIZE/2)/TEXT_WIDTH+2;
   q1_q2_col = (AZEL_X+AZEL_SIZE/2)/TEXT_WIDTH-2;
   if(SCREEN_WIDTH <= NARROW_SCREEN) {
      if(all_adevs || shared_plot) {  // plot area shared with az/el map
         if(SCREEN_HEIGHT < SHORT_SCREEN) {
            ++q4_row;  ++q1_row;
            ++q3_row;  ++q2_row;
         }
      }
      else {
         ++q3_row; ++q2_row;
      }
      --q4_row; --q1_row;
   }
   else if(all_adevs || shared_plot) {  // az/el map shared with plot area
      q1_q2_col -= 2;
      q3_q4_col += 1;
   }
   else {
      q1_q2_col -= 1;
   }

   if(0 & (SCREEN_HEIGHT < SHORT_SCREEN)) {
      --q2_row;
      --q3_row;
   }

   if(q1_q2_col < 0) q1_q2_col = 0;
   if(q3_q4_col < 0) q3_q4_col = 0;
   if(q1_row < 0) q1_row = 0;
   if(q2_row < 0) q2_row = 0;
   if(q3_row < 0) q3_row = 0;
   if(q4_row < 0) q4_row = 0;

   if((zoom_screen == 'U') || (zoom_screen == 'V')) {
      if(SCREEN_HEIGHT > SHORT_SCREEN) show_title();
   }
   else if(zoom_screen) {
      show_title();
   }

   if(small_azel()) return;  // map is too small to label

   no_x_margin = no_y_margin = 1;

   #ifdef SIG_LEVELS 
      if(zoom_screen || ((SCREEN_HEIGHT >= AZEL_TITLE_THRESH) && (all_adevs == SINGLE_ADEVS)) || ((SCREEN_WIDTH >= MEDIUM_WIDTH) && (AZEL_ROW < PLOT_ROW))) {
         if(zoom_screen == 'S')      label_circles(signals, q2_row+3); 
         else if(zoom_screen == 'U') label_circles(signals, q2_row+3); 
         else if((zoom_screen == 'V') || (zoom_screen == 'I')) {
            if(plot_signals) label_circles(signals, q2_row+3); 
            else             label_circles(0, q2_row+3); 
         }
         else if(map_and_watch) label_circles(0, q2_row+3);
         else                   label_circles(signals, q2_row+3);
      }

      l = min_sig_db;
      step = sig_level_step;

      if((zoom_screen == 'X') || (zoom_screen == 'Y')) {
         if(SCREEN_WIDTH < TINY_TINY_WIDTH) {
            q1_q2_col += 4;
            q3_q4_col -= 4;
         }
         else {
            q1_q2_col += 9;
            q3_q4_col -= 9;
         }
         goto xxxx;
      }
      else if((zoom_screen == 'U') || (zoom_screen == 'V') || (zoom_screen == 'I')) {
         q1_q2_col += 3;
         q3_q4_col -= 3;
         if(plot_signals == 4) goto xxxx;
         else if(plot_signals == 0) goto yyyy;
no_x_margin = no_y_margin = 0;
         return;
      }
      else if((zoom_screen == 'M') || (zoom_screen == 'B')) {
         q1_q2_col += 4;
         q3_q4_col -= 4;
      }

      if(map_and_watch && (zoom_screen != 'S')) ;
      else if((signals || (zoom_screen == 'S')) && (plot_signals >= 4)) {
         if(zoom_screen || ((SCREEN_WIDTH >= MEDIUM_WIDTH) && (AZEL_ROW < PLOT_ROW))) {
            xxxx:
            q1_q2_col-=2;
            q3_q4_col-=2;
            if(zoom_screen && (SCREEN_WIDTH >= (NARROW_SCREEN-24)) && (SCREEN_WIDTH <= (NARROW_SCREEN+24))) {  // !!!!! == NARROW_SCREEN?
               ++q1_row;  ++q4_row;
            }
//sprintf(debug_text, "q1r:%d  q2r:%d  q3r:%d  q4r:%d  q34c:%d  q12c:%d", q1_row,q2_row,q3_row,q4_row, q3_q4_col, q1_q2_col);
//DEBUGSTR(debug_text);

            sprintf(out, "<%d %s", l, level_type);
            vidstr(q4_row+0, q3_q4_col, LOW_SIGNAL,      out);
            sprintf(out, ">%d %s", l, level_type); l+=step;
            vidstr(q4_row+1, q3_q4_col, level_color[1],  out);
            sprintf(out, ">%d %s", l, level_type); l+=step;
            vidstr(q4_row+2, q3_q4_col, level_color[2],  out);

            sprintf(out, ">%d %s", l, level_type); l+=step;
            vidstr(q1_row+0, q1_q2_col, level_color[3],  out);
            sprintf(out, ">%d %s", l, level_type); l+=step;
            vidstr(q1_row+1, q1_q2_col, level_color[4],  out);
            sprintf(out, ">%d %s", l, level_type); l+=step;
            vidstr(q1_row+2, q1_q2_col, level_color[5],  out);

            sprintf(out, ">%d %s", l, level_type); l+=step;
            vidstr(q3_row-2, q3_q4_col, level_color[6],  out);
            sprintf(out, ">%d %s", l, level_type); l+=step;
            vidstr(q3_row-1, q3_q4_col, level_color[7],  out);
            sprintf(out, ">%d %s", l, level_type); l+=step;
            vidstr(q3_row-0, q3_q4_col, level_color[8],  out);

            sprintf(out, ">%d %s", l, level_type); l+=step;
            vidstr(q2_row-2, q1_q2_col, level_color[9],  out);
            sprintf(out, ">%d %s", l, level_type); l+=step;
            vidstr(q2_row-1, q1_q2_col, level_color[10], out);
            sprintf(out, ">%d %s", l, level_type); l+=step;
            vidstr(q2_row-0, q1_q2_col, level_color[11], out);
         }
         else {
            sprintf(out, "<%d", l);
            vidstr(q4_row+0, q3_q4_col, LOW_SIGNAL,      out);
            sprintf(out, ">%d", l); l+=step;
            vidstr(q4_row+1, q3_q4_col, level_color[1],  out);   
            sprintf(out, ">%d", l); l+=step;
            vidstr(q4_row+2, q3_q4_col, level_color[2],  out);   

            sprintf(out, ">%d", l); l+=step;
            vidstr(q1_row+0, q1_q2_col, level_color[3],  out);   
            sprintf(out, ">%d", l); l+=step;
            vidstr(q1_row+1, q1_q2_col, level_color[4],  out);   
            sprintf(out, ">%d", l); l+=step;
            vidstr(q1_row+2, q1_q2_col, level_color[5],  out);   

            sprintf(out, ">%d", l); l+=step;
            vidstr(q3_row-2, q3_q4_col, level_color[6],  out);   
            sprintf(out, ">%d", l); l+=step;
            vidstr(q3_row-1, q3_q4_col, level_color[7],  out);   
            sprintf(out, ">%d", l); l+=step;
            vidstr(q3_row-0, q3_q4_col, level_color[8],  out);   

            sprintf(out, ">%d", l); l+=step;
            vidstr(q2_row-2, q1_q2_col, level_color[9],  out);
            sprintf(out, ">%d", l); l+=step;
            vidstr(q2_row-1, q1_q2_col, level_color[10], out);
            sprintf(out, ">%d", l); l+=step;
            vidstr(q2_row-0, q1_q2_col, level_color[11], out);
         }
         no_x_margin = no_y_margin = 0;
if((zoom_screen == 'X') || (zoom_screen == 'Y')) {
   q1_q2_col -= 9;
   q3_q4_col += 9;

   --q1_row;
   --q4_row;
}
         else return;
      }
      else if(signals && plot_signals) {
         no_x_margin = no_y_margin = 0;
         return;
      }
   #endif

   yyyy:
   j = 0;
   for(i=1; i<=SUN_MOON_PRN; i++) {  // !!!!!! mooooo first draw the sat PRNs in use
      if(sat[i].level_msg == 0x00) continue;
      if((i == SUN_PRN) && (no_sun_moon & 0x01)) continue;
      if((i == MOON_PRN) && (no_sun_moon & 0x02)) continue;

if(tracked_only & (sat[i].tracking <= 0) && (i != SUN_PRN) && (i != MOON_PRN)) continue;
      if(i == SUN_PRN) ;
      else if(i == MOON_PRN);
      else if(sat[i].sig_level <= 0.0) continue; // zzzzzzzzzzzzzzzzzzzz

      el = sat[i].elevation;
      az = sat[i].azimuth;

      if((az == 0.0F) && (el == 0.0F)) continue;
      if((az < 0.0F) || (az >= 360.0F)) continue;
      if(el > 90.0F) continue;
      if(el < 0.0F) {
         if(i == SUN_PRN) ;
         else if(i == MOON_PRN) ;
         else continue;
         el = 0.0F - el;
      }

//sprintf(debug_text2, "Q1r:%d  Q2r:%d  Q3r:%d  Q4r:%d  Q34c:%d  Q12c:%d", q1_row,q2_row,q3_row,q4_row, q3_q4_col, q1_q2_col);
//DEBUGSTR3(debug_text2);

      if(sat[i].tracking > 0) {  //  we are actively tracking at least one satellite
         if(azel_grid_color == AZEL_ALERT) azel_grid_color = AZEL_COLOR;  
      } 
      el_dir = sat[i].el_dir;
      if(el_dir == 0) el_dir = ' ';

      kl = kr = 0;
//gns if(SCREEN_WIDTH > NARROW_SCREEN) {
      if((SCREEN_WIDTH > 800) || zoom_screen) {
         if(i == SUN_PRN)       sprintf(out, "SUN "); 
         else if(i == MOON_PRN) sprintf(out, "MOON"); 
         else if(i >= 100)      sprintf(out, "%03d%c", i,el_dir);
         else                   sprintf(out, "%02d%c ", i,el_dir);
         kl = 1;
         kr = 0;
      }
      else {
         if(i == SUN_PRN)       sprintf(out, "SUN "); 
         else if(i == MOON_PRN) sprintf(out, "MOON"); 
         else if(i >= 100)      sprintf(out, "%03d ", i);
         else                   sprintf(out, "%02d  ", i);
//gns    kl = 0;
//gns    kr = 1;
//       kl = 2;
         kl = 1;  // helps with small screens with no critical info
         kr = 1;
      }

      color = j;
      if(i == SUN_PRN) color = YELLOW_SAT_COLOR;  // !!!! kludge: force YELLOW for the sun
      if(i == MOON_PRN) {
         if(moon_el < 0) color = GREY_SAT_COLOR;
         else            color = YELLOW_SAT_COLOR;  // !!!! kludge: force YELLOW for the moon
      //sprintf(debug_text, "moon: el:%f  moon_el:%f  color:%d", el, moon_el, color);
      }
      color = sat_colors[color];
      if(blink_sat() && (i == blink_sat()) && (seconds & 1)) {  // blink the specified sat
         color = BLACK;
      }

      if(az < 90.0F) {  // figure out what quadrant they are in
         vidstr(q1_row, q1_q2_col+kr, color, out);
         ++q1_row;
      }
      else if(az < 180.0F) {
         vidstr(q2_row, q1_q2_col+kr, color, out);
         --q2_row;
      }
      else if(az < 270.0F) {
         vidstr(q3_row, q3_q4_col-kl, color, out);
         --q3_row;
      }
      else {
         vidstr(q4_row, q3_q4_col-kl, color, out);
         ++q4_row;
      }

      ++j;
      if(j > MAX_SAT_COLOR) j = 0;
   }

   no_x_margin = no_y_margin = 0;
   return;
}


#ifdef SIG_LEVELS

float amu_2_dbc[] = {  // indexed with amu*10.0
   20.0F, 20.4F, 20.8F, 21.2F, 21.6F, 22.0F, 22.4F, 22.8F, 23.2F, 23.6F, 
   24.0F, 24.4F, 24.8F, 25.2F, 25.6F, 26.0F, 26.4F, 26.8F, 27.2F, 27.6F, 
   28.0F, 28.4F, 28.8F, 29.2F, 29.6F, 30.0F, 30.3F, 30.6F, 30.9F, 31.2F, 
   31.6F, 31.9F, 32.2F, 32.5F, 32.8F, 33.1F, 33.4F, 33.7F, 34.0F, 34.3F, 
   34.6F, 34.8F, 35.0F, 35.2F, 35.4F, 35.6F, 35.8F, 36.0F, 36.2F, 36.4F, 
   36.6F, 36.8F, 37.0F, 37.2F, 37.4F, 37.6F, 37.8F, 38.0F, 38.2F, 38.4F, 
   38.6F, 38.8F, 39.0F, 39.2F, 39.4F, 39.6F, 39.8F, 39.9F, 40.0F, 40.1F, 
   40.2F, 40.3F, 40.4F, 40.5F, 40.6F, 40.7F, 40.8F, 40.9F, 41.0F, 41.1F, 
   41.2F, 41.3F, 41.4F, 41.5F, 41.6F, 41.7F, 41.8F, 41.9F, 42.0F, 42.1F, 
   42.2F, 42.3F, 42.4F, 42.5F, 42.6F, 42.7F, 42.8F, 42.9F, 43.0F, 43.1F, 
   43.2F, 43.3F, 43.4F, 43.5F, 43.6F, 43.7F, 43.7F, 43.8F, 43.9F, 44.0F, 
   44.1F, 44.2F, 44.3F, 44.4F, 44.5F, 44.6F, 44.7F, 44.8F, 44.9F, 45.0F, 
   45.1F, 45.2F, 45.3F, 45.4F, 45.4F, 45.5F, 45.5F, 45.6F, 45.6F, 45.7F, 
   45.7F, 45.8F, 45.8F, 45.9F, 46.0F, 46.0F, 46.1F, 46.1F, 46.2F, 46.2F, 
   46.3F, 46.3F, 46.4F, 46.5F, 46.6F, 46.7F, 46.8F, 46.8F, 46.9F, 46.9F, 
   47.0F, 47.0F, 47.1F, 47.1F, 47.2F, 47.2F, 47.3F, 47.3F, 47.4F, 47.4F, 
   47.5F, 47.5F, 47.6F, 47.6F, 47.7F, 47.7F, 47.8F, 47.8F, 47.9F, 47.9F, 
   48.0F, 48.0F, 48.1F, 48.1F, 48.2F, 48.2F, 48.3F, 48.3F, 48.4F, 48.4F, 
   48.5F, 48.5F, 48.6F, 48.6F, 48.7F, 48.7F, 48.8F, 48.8F, 48.9F, 48.9F, 
   49.0F, 49.0F, 49.0F, 49.0F, 49.1F, 49.1F, 49.1F, 49.2F, 49.2F, 49.2F, 
   49.3F, 49.3F, 49.3F, 49.4F, 49.4F, 49.4F, 49.5F, 49.5F, 49.5F, 49.6F, 
   49.6F, 49.6F, 49.7F, 49.7F, 49.7F, 49.8F, 49.8F, 49.8F, 49.9F, 49.9F, 
   49.9F, 50.0F, 50.0F, 50.0F, 50.1F, 50.1F, 50.1F, 50.2F, 50.2F, 50.2F, 
   50.3F, 50.3F, 50.3F, 50.4F, 50.4F, 50.4F, 50.5F, 50.5F, 50.5F, 50.6F, 
   50.6F, 50.6F, 50.7F, 50.7F, 50.7F, 50.8F, 50.8F, 50.8F, 50.9F, 50.9F, 
   50.9F, 51.0F, 51.0F, 51.0F, 51.1F, 51.1F
};

float amu_to_dbc(float sig_level)
{
   if     (sig_level < 0.0F)   return 0.0F;
   else if(sig_level >= 25.1F) return 51.0F + ((sig_level - 25.1F) / 3.0F);
   else                        return amu_2_dbc[(int) (sig_level*10.0F)];
}

float dbc_to_amu(float sig_level)
{
int i;

   if(sig_level <= 0.0F)  return 0.0F;
   if(sig_level >= 51.0F) return 25.1F + ((sig_level - 51.0F) * 3.0F);

   for(i=0; i<256; i++) {
      if(amu_2_dbc[i] > sig_level) return ((float) i) / 10.0F;
   }
   return 0.0F;
}



void small_screen_watch()
{
int medium_touch;  // for 1024x600 touch screen

if(plot_lla && lla_showing) return;

   medium_touch = ((SCREEN_WIDTH > MEDIUM_WIDTH) && (SCREEN_WIDTH <= (MEDIUM_WIDTH+X11_MARGIN)) && (SCREEN_HEIGHT <= SHORT_SCREEN) && (all_adevs == SINGLE_ADEVS));

   if(medium_touch || (SMALL_SCREEN && (SCREEN_WIDTH > NARROW_SCREEN) && (all_adevs == SINGLE_ADEVS))) {
      if(rcvr_type == NO_RCVR) {
         aclock_y = WATCH_ROW + ACLOCK_R + TEXT_WIDTH;
         aclock_x = WATCH_COL + TEXT_WIDTH;
      }
      else {
         ACLOCK_SIZE = WATCH_SIZE;
         aclock_y = WATCH_ROW + ACLOCK_R + TEXT_WIDTH;
         aclock_x = WATCH_COL + ACLOCK_R + TEXT_WIDTH;
      }
   }
}


void draw_maps()
{
u08 show_watch;
u08 show_map;
u08 show_sigs;

   // This routine controls the display of the lat/lon/alt map,  sat position
   // map, signal level map,  and analog watch displays.  
//sprintf(debug_text, "plot_lla:%d  fixes:%d  x:%d  row:%d  showing:%d  shared:%02X  aclock:%d,%d %d  WATCH:%d,%d %d", 
//plot_lla, show_fixes, LLA_Y, LLA_ROW, lla_showing, shared_item, aclock_x,aclock_y,ACLOCK_R, WATCH_COL,WATCH_ROW,WATCH_SIZE);
#define XXXX (NARROW_SCREEN)

   if(zoom_screen == '`') {
      zoom_calc();
      return;
   }
   else if(zoom_screen == 'Q') {
      zoom_cal(cal_year, cal_month);
      return;
   }

   azel_erased = 0;
   if(text_mode) plot_areas = 0;
   else if(just_read) plot_areas = 0;
   else if(zoom_screen == 'L') plot_areas = 0;
   else if(zoom_screen) plot_areas = 1;
   else if(SCREEN_WIDTH < XXXX) {
      if(shared_plot) plot_areas = 1;
      else if(rcvr_type == NO_RCVR) plot_areas = 1;
////else if(rcvr_type == TIDE_RCVR) plot_areas = 1;  // ckckck
      else plot_areas = 0;
   }
   else if(shared_plot == 0) plot_areas = 1;
   else if(all_adevs) plot_areas = 1;
   else plot_areas = 2;

   show_watch = plot_watch;
   show_map = plot_azel;
   if(zoom_screen == 'X') show_watch = show_map = 1;
   if(zoom_screen == 'Y') show_map = 1;
   if(zoom_screen == 'I') show_map = 1;
   show_sigs = plot_signals;

   if(first_key && plot_areas) {
      if(SCREEN_WIDTH < XXXX) plot_areas = 0;
      else if(all_adevs) plot_areas = 0;
//ssssssss      else if(shared_plot) plot_areas = 0;
      else plot_areas = 1;
   }

   map_and_watch = 0;
   map_and_sigs = 0; 
   if(show_watch && show_map && ((plot_areas == 1) || show_sigs || plot_lla)) {
      map_and_watch = 1;
      if(zoom_screen && (zoom_screen != 'K')) {
         aclock_x = AZEL_COL + AZEL_SIZE/2 - 2;
         aclock_y = AZEL_ROW + AZEL_SIZE/2 - 2;
      }
   }

   if(zoom_screen == 'S') ;
   else if(zoom_screen == 'X') ;
   else if(zoom_screen == 'Y') ;
   else if(zoom_screen == 'I') ;
   else if(show_map && show_sigs && (show_sigs != 3) && ((plot_areas == 1) || plot_lla)) { 
      map_and_sigs = 1;
   }

   if(plot_areas == 0) return;

   if(plot_lla && (zoom_screen == 0)) {  // we are drawing a LatLonAlt plot
      if(screen_configed) {   // screen re-configured,  so redraw LLA grid
         plot_lla_axes(2);
         screen_configed = 0;
      }
      --plot_areas;
      if(plot_areas <= 0) return;  // LLA plot is updated where packets are read in
   }

   if(zoom_screen == 'B') goto maw;
   else if(zoom_screen == 'D') {  // zoom to receiver data info
      if     (rcvr_type == BRANDY_RCVR) show_brandy_info(1,4);
      else if(rcvr_type == CS_RCVR)     show_cs_info();
      else if(rcvr_type == LPFRS_RCVR)  show_lpfrs_info(1,3, 999);
      else if(rcvr_type == PRS_RCVR)    show_prs_info(1,3, 999);
      else if(rcvr_type == SA35_RCVR)   show_sa35_info(1,3, 999);
      else if(rcvr_type == SRO_RCVR)    show_sro_info(1,3, 999);
      else if(rcvr_type == RFTG_RCVR)   show_rftg_info(1,4);
      else if(rcvr_type == X72_RCVR)    show_x72_info(1,3, 999);
      return;
   }
   else if(zoom_screen == 'F') {  // FFT waterfall
      // image drawn in process_signal()
      return;
   }
   else if(zoom_screen == 'I') {
      zoom_sat_info();
      return;
   }
   else if(zoom_screen == 'U') {
      zoom_all_signals();
      return;
   }
   else if(zoom_screen == 'V') {
      zoom_stuff();
      return;
   }
   else if(zoom_screen == 'W') goto maw;
   else if((zoom_screen == 'X') || (zoom_screen == 'Y')) {
      draw_azel_plot(1);
      return;
   }
   else if(zoom_screen == '`') {
      zoom_calc();
      return;
   }
   else if(zoom_screen) ;
   else if(map_and_watch) {
      maw:
      if(MESS && map_and_sigs && (plot_areas == 1)) {
         draw_signal_map(1);
         show_sigs = 0;
      }

      #ifdef SAT_TRAILS
//       update_sat_trails(); // update az/el position array
      #endif
      if(zoom_screen == 'W') draw_watch(1);
      else {
         if(rotate_screen && (azel_erased == 0) && (AZEL_ROW >= PLOT_ROW)) {
            erase_azel(20);
         }
         draw_azel_plot(2);
      }
      --plot_areas;
      show_watch = 0; 
      show_map = 0;
      map_and_watch = 0;
   }

   if(zoom_screen) ;
   else if(plot_areas <= 0) return;
//sprintf(out, "maw:%d  mas:%d  show_map:%d  show_sigs:%d  areas:%d", map_and_watch,map_and_sigs, show_map,show_sigs, plot_areas);
//DEBUGSTR(out);

   #ifdef SIG_LEVELS
      if((show_sigs && (plot_areas > 0)) || (zoom_screen == 'S')) {
         draw_signal_map(2);      // draw signal level map
         show_sigs = 0;

         if(map_and_sigs) {
            goto do_map;
         }
         --plot_areas;
      }
   #endif

//sprintf(debug_text3, "areas:%d  watch:%d  map:%d sigs:%d lla:%d  azel:%d,%d",
//plot_areas, show_watch,show_map, show_sigs, plot_lla, AZEL_COL,AZEL_ROW);

   #ifdef AZEL_STUFF
      if((zoom_screen == 'M') || (zoom_screen == 'B')) goto do_map;
      else if(zoom_screen) ;
      else if(show_map && (plot_areas > 0)) {
         do_map:
         #ifdef SAT_TRAILS
//          update_sat_trails(); // update az/el position array
         #endif
         draw_azel_plot(3);
         show_map = 0;
         --plot_areas;
      }
   #endif
   map_and_sigs = 0;

   #ifdef ANALOG_CLOCK
      if((zoom_screen == 'W') && (zoom_screen == 'B')) {
         goto anaw;
      }
      else if(zoom_screen) ;
      else if((plot_areas > 0) || (rcvr_type == NO_RCVR) || (rcvr_type == TIDE_RCVR)) {
         if(luxor && luxor_fault()) {
            --plot_areas;
         }
         else if(show_watch) {
            anaw:
small_screen_watch();
            draw_watch(2);      // draw analog watch
            show_watch = 0;
            --plot_areas;
         }
      }
   #endif
}

int center_sat;
int cursor_sat_color;

void find_cursor_sat()
{
int prn;
int close_prn;
double close_dist;
double dist;
double dx, dy;
double az, el;
char az_dir;
char el_dir;

   // show info of the satellite the mouse cursor is over in the sat map

   if(drawn_sats == 1) sprintf(out, "%d Satellite Position ", drawn_sats);
   else if(drawn_sats) sprintf(out, "%d Satellite Positions ", drawn_sats);
   else                sprintf(out, "Satellite Positions ");

   if(map_r == 0) return;

   if(mouse_x < map_x-map_r) return;
   if(mouse_x > map_x+map_r) return;
   if(mouse_y < map_y-map_r) return;
   if(mouse_y > map_y+map_r) return;

   close_prn = 0;
   close_dist = 1E10;
   for(prn=1; prn<=MOON_PRN; prn++) {
//if(prn == SUN_PRN) ;
//else if(prn == MOON_PRN) ;
      if(sat[prn].level_msg == 0) continue;

      if(mouse_x < sat[prn].plot_x - sat[prn].plot_r) continue;
      if(mouse_x > sat[prn].plot_x + sat[prn].plot_r) continue;
      if(mouse_y < sat[prn].plot_y - sat[prn].plot_r) continue;
      if(mouse_y > sat[prn].plot_y + sat[prn].plot_r) continue;

      dx = (double) (sat[prn].plot_x - mouse_x);
      dy = (double) (sat[prn].plot_y - mouse_y);
      dist = sqrt(dx*dx + dy*dy);
      if(dist < close_dist) {
         close_dist = dist;
         close_prn = prn;
      }
   }

   if(close_prn) {  // show info of the sat at the mouse cursor
      az_dir = (char) sat[close_prn].az_dir;
      if(az_dir == 0) az_dir = ' ';
      else if(sat[close_prn].azimuth == 0.0F) az_dir = ' ';
      el_dir = (char) sat[close_prn].el_dir;
      if(el_dir == 0) el_dir = ' ';
      else if(sat[close_prn].elevation == 0.0F) el_dir = ' ';

      if(close_prn == SUN_PRN) {
         if(sat[close_prn].elevation >= 0) cursor_sat_color = YELLOW;
         else                              cursor_sat_color = GREY;
         sprintf(out, "SUN: az:%.2f%c el:%.2f%c", sat[close_prn].azimuth,az_dir,  sat[close_prn].elevation,el_dir);
      }
      else if(close_prn == MOON_PRN) {
         if(sat[close_prn].elevation >= 0) cursor_sat_color = YELLOW;
         else                              cursor_sat_color = GREY;
         sprintf(out, "MOON: az:%.0f%c el:%.0f%c phase:%.0f%%", sat[close_prn].azimuth,az_dir, sat[close_prn].elevation,el_dir,MoonPhase*100.0);
      }
      else {
         if     (sat[close_prn].disabled)         cursor_sat_color = BLUE;
         else if(sat[close_prn].health_flag == 1) cursor_sat_color = RED;
         else if(sat[close_prn].tracking <= 0)    cursor_sat_color = YELLOW;
         else                                     cursor_sat_color = GREEN;
         sprintf(out, "%s:%d az:%.0f%c el:%.0f%c sig:%.0f", gnss_id(close_prn),close_prn, sat[close_prn].azimuth,az_dir, sat[close_prn].elevation,el_dir,sat[close_prn].sig_level);
      }
//    cursor_sat_color = sat[close_prn].plot_color;
//    center_sat = 0;
   }
   else {   // show az/el at the mouse cursor
      dx = mouse_x - AZEL_X;
      dy = mouse_y - AZEL_Y;
      az = 90.0 + atan2(dy,dx) * 180.0/PI;
      if(az < 0.0) az += 360.0;
      else if(az >= 360.0) az -= 360.0;

      el = sqrt(dx*dx+dy*dy) / AZEL_RADIUS;
      el = 90.0 - (90.0 * el);
      if(el >= 0.0) {
         sprintf(out, "az:%.0f  el:%.0f", az,el);
      }
   }
}

void label_circles(int signals, int row)
{
int col;
int x, y;
char *s;

   // label the various az/el plots

   center_sat = 1;
   cursor_sat_color = WHITE;

   if(rcvr_type == CS_RCVR) return;
   else if(rcvr_type == NO_RCVR) return;
   else if(rcvr_type == THERMO_RCVR) return;
   else if(rcvr_type == TICC_RCVR) return;
////else if(rcvr_type == TIDE_RCVR) return;  // ckckck
   else if(signals && (plot_signals == 1)) {
      s = "Relative strength vs azimuth";
      if(zoom_screen == 'U') row -= 2;
   }
   else if(signals && (plot_signals == 2)) {
      s = "1/EL weighted strength vs azimuth";
      if(zoom_screen == 'U') row -= 2;
      if(zoom_screen == 'V') row -= 2;
      if(zoom_screen == 'I') row -= 2;
   }
   else if(signals && (plot_signals == 3)) s = "Relative strength vs elevation";
   else if(signals && (plot_signals == 4)) {
      s = "Signal strength vs az/el";
      if(zoom_screen == 'U') row -= 2;
      if(zoom_screen == 'V') row -= 2;
      if(zoom_screen == 'I') row -= 2;
   }
   else if(signals && (plot_signals == 5)) {
      s = "Raw signal level data";
   }
   else {
      find_cursor_sat();
      s = &out[0];
      if(zoom_screen == 'U') row -= 2;
      else if(zoom_screen == 'V') row -= 2;
      else if(zoom_screen == 'I') row -= 2;
      else if((zoom_screen == 'B') || (zoom_screen == 'M')) {
         if(SCREEN_HEIGHT < TINY_TINY_HEIGHT) row -= 1;
      }
   }

   if(zoom_screen) ++row;
   col = ((AZEL_X-AZEL_RADIUS)/TEXT_WIDTH);
   x = (AZEL_RADIUS*2) / TEXT_WIDTH;
   vidstr(row,col, WHITE, &blanks[TEXT_COLS-x]);

   if(center_sat) col = (AZEL_X/TEXT_WIDTH) - (strlen(s)/2);
   else           col = ((AZEL_X-AZEL_RADIUS)/TEXT_WIDTH);
   vidstr(row,col, cursor_sat_color, s);

   if(small_azel()) return;  // map is too small to label

   graphics_coords = 1;   // these labels need fine positioning

   if(signals && (plot_signals == 3)) {
      x = AZEL_X;
      y = AZEL_Y;  // AZEL_RADIUS;
      if(zoom_screen) y += TEXT_HEIGHT;
      y += AZEL_RADIUS+TEXT_HEIGHT/2;
      vidstr(y, x-AZEL_RADIUS*1-2*TEXT_WIDTH/2, WHITE, "0%");
      vidstr(y, x-AZEL_RADIUS/2-3*TEXT_WIDTH/2, WHITE, "25%");
      vidstr(y, x-AZEL_RADIUS*0-3*TEXT_WIDTH/2, WHITE, "50%");
      vidstr(y, x+AZEL_RADIUS/2-3*TEXT_WIDTH/2, WHITE, "75%");
      vidstr(y, x+AZEL_RADIUS*1-4*TEXT_WIDTH/2, WHITE, "100%");

      x = AZEL_X+AZEL_RADIUS+TEXT_WIDTH*2;
      y = AZEL_Y+AZEL_RADIUS-TEXT_HEIGHT/2;
      if(zoom_screen) x += TEXT_WIDTH;
      sprintf(out, "0%c", DEGREES);
      vidstr(y,x, WHITE, out);

      x = AZEL_X-AZEL_RADIUS+TEXT_WIDTH*3  + (int) ((float)(AZEL_RADIUS*2)*cos360(30));
      y = AZEL_Y+AZEL_RADIUS-TEXT_HEIGHT*2 - (int) ((float)(AZEL_RADIUS*2)*sin360(30));
      if(zoom_screen == 0) y += TEXT_HEIGHT;
      if(zoom_screen == 0) x -= TEXT_WIDTH;
      sprintf(out, "30%c", DEGREES);
      vidstr(y,x, WHITE, out);

      x = AZEL_X-AZEL_RADIUS+TEXT_WIDTH*1  + (int) ((float)(AZEL_RADIUS*2)*cos360(60));
      y = AZEL_Y+AZEL_RADIUS-TEXT_HEIGHT*3 - (int) ((float)(AZEL_RADIUS*2)*sin360(60));
      if(zoom_screen == 0) y += TEXT_HEIGHT;
      sprintf(out, "60%c", DEGREES);
      vidstr(y,x, WHITE, out);

      x = AZEL_X-AZEL_RADIUS-(TEXT_WIDTH*3/2);
      y = AZEL_Y-AZEL_RADIUS-TEXT_HEIGHT;
      if(zoom_screen) y -= TEXT_HEIGHT;
      sprintf(out, "90%c", DEGREES);
      vidstr(y,x, WHITE, out);
   }
   else if((signals && plot_signals) || (zoom_screen == 'M') || (zoom_screen == 'I')) {
      x = AZEL_X-TEXT_WIDTH/2;
      y = AZEL_Y;  // AZEL_RADIUS;
      vidstr(y-AZEL_RADIUS-TEXT_HEIGHT-TEXT_HEIGHT/2,x, WHITE, "N");

      if((zoom_screen == 'U') || (zoom_screen == 'V')) ;
      else vidstr(y+AZEL_RADIUS+TEXT_HEIGHT/2,x, WHITE, "S");

      x = AZEL_X;
      y = AZEL_Y - TEXT_HEIGHT/2;
      vidstr(y,x-AZEL_RADIUS-TEXT_WIDTH*2-TEXT_WIDTH/2, WHITE, "W");

      x = x+AZEL_RADIUS+TEXT_WIDTH*1+TEXT_WIDTH/2;
      if((plot_signals >= 4) || (zoom_screen == 'M') || (zoom_screen == 'I')) {
         vidstr(y,x, WHITE, "E");
      }
      else {
         if((SCREEN_WIDTH == 1280) && (zoom_screen == 0)) ;
         else x += TEXT_WIDTH;
         vidstr(y,x, WHITE, "E");

         x = AZEL_X;
         y = AZEL_Y+TEXT_HEIGHT/4;  // AZEL_RADIUS;
         vidstr(y, x+AZEL_RADIUS*1/4-3*TEXT_WIDTH/2, WHITE, "25%");
         vidstr(y, x+AZEL_RADIUS*2/4-3*TEXT_WIDTH/2, WHITE, "50%");
         vidstr(y, x+AZEL_RADIUS*3/4-3*TEXT_WIDTH/2, WHITE, "75%");
         vidstr(y, x+AZEL_RADIUS*4/4-4*TEXT_WIDTH/2, WHITE, "100%");

         vidstr(y, x-AZEL_RADIUS*4/4-3*TEXT_WIDTH/2, WHITE, "100%");
         vidstr(y, x-AZEL_RADIUS*3/4-2*TEXT_WIDTH/2, WHITE, "75%");
         vidstr(y, x-AZEL_RADIUS*2/4-2*TEXT_WIDTH/2, WHITE, "50%");
         vidstr(y, x-AZEL_RADIUS*1/4-2*TEXT_WIDTH/2, WHITE, "25%");
      }
   }

   graphics_coords = 0;
}

void clear_signals()
{
int x, y;

   // clear out the signal level data structures

   for(x=0; x<=360; x++) {
      db_az_sum[x] = 0.0F;
      db_weighted_az_sum[x] = 0.0F;
      db_az_count[x] = 0.0F;
      max_el[x] = 0;
      min_el[x] = 90;

      for(y=0; y<=90; y++) {
         db_3d_sum[x][y] = 0.0F;
         db_3d_count[x][y] = (-1.0F);
      }
   }

   for(y=0; y<=90; y++) {
      db_el_sum[y]= 0.0F;
      db_el_count[y] = 0.0F;
   }

   max_sig_level = 0.0F;
   min_sig_level = 999.0F;
   sig_level_sum = 0.0;
   sig_level_count = 0.0;
   max_sat_prn = 0;
   for(x=0; x<=32; x++) {
      max_sat_db[x] = 0.0F;
   }

   reading_signals = 0;  // re-enable live data recording
   signal_length = 0L;
}



void freshen_sig_info()
{
int az, ell;

   // keep signal level maps "fresh" by periodically re-starting the averaging
   // of readings from the current avaerage.

   for(az=0; az<=360; az++) {
      if(db_az_count[az]) {
         db_weighted_az_sum[az] = (db_weighted_az_sum[az] / db_az_count[az]);
         db_az_sum[az] = (db_az_sum[az] / db_az_count[az]);
         db_az_count[az] = 1;
      }
   }

   for(ell=0; ell<=90; ell++) {
      if(db_el_count[ell]) {
         db_el_sum[ell] = (db_el_sum[ell] / db_el_count[ell]);
         db_el_count[ell] = 1;
      }
   }
}


void log_signal(float azf, float elf, float sig_level, int amu_flag)
{
int az, ell;

   // record sat signal strength data in the various arrays

   az  = (int) (azf+0.50F);
   ell = (int) (elf+0.50F);
   if((az < 0) || (az > 360)) return;
   if((ell < 0) || (ell > 90)) return;
   if((az == 0) && (ell == 0)) return;  // probably a bogus fix, ignore

   if(sig_level < 1.00F) {  // don't let zero signal levels spoil the average
return;  // fixes problems at high latitudes with receivers that report sigs of sats below the elevation mask
      if(db_3d_count[az][ell] > 0.0) return; // only log one of them
   }

   if((rcvr_type == TSIP_RCVR) && amu_flag) {  // convert AMU values to dBc values
      sig_level = amu_to_dbc(sig_level);
   }

   if((MAX_AVG_COUNT > 0.0F) && (db_az_count[az] > MAX_AVG_COUNT)) {
      db_weighted_az_sum[az] -= (db_weighted_az_sum[az] / db_az_count[az]);
      db_weighted_az_sum[az] += (sig_level * ((90.0F-(float)ell)/90.0F));

      db_az_sum[az] -= (db_az_sum[az] / db_az_count[az]);
      db_az_sum[az] += sig_level;
   }
   else {
      db_weighted_az_sum[az] += (sig_level * ((90.0F-(float)ell)/90.0F));
      db_az_sum[az] += sig_level;
      db_az_count[az] += 1.0F;
   }

   if((MAX_AVG_COUNT > 0.0) && (db_el_count[ell] > MAX_AVG_COUNT)) {
      db_el_sum[ell] -= (db_el_sum[ell] / db_el_count[ell]);
      db_el_sum[ell] += sig_level;
   }
   else {
      db_el_sum[ell] += sig_level;
      db_el_count[ell] += 1.0F;
   }

   if(ell > max_el[az]) max_el[az] = ell;
   if(ell < min_el[az]) min_el[az] = ell;

   if(0) {  // track peak signal
      if(sig_level > db_3d_sum[az][ell]) db_3d_sum[az][ell] = sig_level;
      db_3d_count[az][ell] = 1.0F;
   }
   else {   // track average signal
      if(db_3d_count[az][ell] < 1.0F) {   // first signal level seen at this location
         db_3d_sum[az][ell] = sig_level;
         db_3d_count[az][ell] = 1.0F;
      }
      else if((MAX_AVG_COUNT > 0.0F) && (db_3d_count[az][ell] > MAX_AVG_COUNT)) {  // keep map fresh by averaging last 10 readings
         db_3d_sum[az][ell] -= (db_3d_sum[az][ell] / db_3d_count[az][ell]);  // remove current average
         db_3d_sum[az][ell] += sig_level;                                    // add in new reading
      }
      else {  // calculate average signal level
         db_3d_sum[az][ell] += sig_level;
         db_3d_count[az][ell] += 1.0F;
      }
   }

   ++signal_length;
}

void dump_signals(char *fn)
{
FILE *f;
int az, el;

   // write signal strength data to a log file

   if(!strstr(fn, ".")) {
      strcat(fn, ".sig");
   }
   f = topen(fn, "w");
   if(f == 0) return;

   write_log_header(f, 3);
   month = fix_month(month);
   fprintf(f,"#   %02d:%02d:%02d %s   %02d %s %04d\n", 
      hours,minutes,seconds, (time_flags & TFLAGS_UTC)?"UTC":"GPS", day,months[month],year);
   fprintf(f, "#\n");
   fprintf(f, "#az el dBc\n");

   for(el=0; el<=90; el++) {
      for(az=0; az<=360; az++) {
         if(db_3d_count[az][el] > 0.0F) {
            fprintf(f, "%03d %02d %5.2f\n", az, el, db_3d_sum[az][el]/db_3d_count[az][el]);
         }
      }
   }

   fclose(f);
}


void read_signals(char *s)
{
FILE *sig_file;
float az, el, sig;

   // stop recording live signal strength data and load in signal strength 
   // data from a log file

   sig_file = topen(s, "r");
   if(sig_file == 0) return;
   clear_signals();       // flush the old data
   
   plot_signals = 4;      // show the pretty color map

   while(fgets(out, sizeof out, sig_file) != NULL) {
      if((out[0] == '*') || (out[0] == ';') || (out[0] == '/') || (out[0] == '#')) {
         continue;  // comment line
      }
      sscanf(out, "%f %f %f", &az, &el, &sig);
      log_signal(az, el, sig, 0);
   }

   fclose(sig_file);
}


void read_prn(char *s)
{
FILE *prn_file;
double jd;
int prn;
float az, el, sig;

   // load in satellie time, prn, az, el, signal strength 
   // data from a .prn file

   prn_file = topen(s, "r");
   if(prn_file == 0) return;

   while(fgets(out, sizeof out, prn_file) != NULL) {
      if((out[0] == '*') || (out[0] == ';') || (out[0] == '/') || (out[0] == '#')) {
         continue;  // comment line
      }
      sscanf(out, "%lf %d %f %f %f", &jd, &prn, &az, &el, &sig);
      log_signal(az, el, sig, 0);   // !!!!! do something with this data?
   }

   fclose(prn_file);
}


void plot_raw_signals(int el)
{
int az;
int color;
int thick;
int x, y;
float elf;

   // draw the raw signal strength data seen at an elevation angle
   thick = (AZEL_RADIUS/90)+1;

   for(az=0; az<=360; az++) {
      if(db_3d_count[az][el] <= 0.0F) continue;
      if(sig_level_step == 0) continue;

      elf = db_3d_sum[az][el] / db_3d_count[az][el];
      elf -= (float) min_sig_db;
      color = (int) (elf / (float) sig_level_step);
      if     (color > LEVEL_COLORS) color = LEVEL_COLORS;
      else if(color <= BLACK)       color = LOW_SIGNAL;
      else                          color = level_color[color];

      elf = ((float)(el - 90)) / 90.0F;

      x = AZEL_X + (int) (elf * cos360(az+90) * AZEL_RADIUS) - thick/2;
      y = AZEL_Y + (int) (elf * sin360(az+90) * AZEL_RADIUS) - thick/2;

      thick_line(x,y, x,y, color, thick);
   }
}

int min_az_clip[360+1];
int max_az_clip[360+1];

void calc_az_clip()
{
int az, el;

return;

   // find the min and max elevation angle that has signals at each az point
   for(az=0; az<=360; az++) {
      min_az_clip[az] = 90;
      max_az_clip[az] = 0;
   }

   for(az=360; az>0; az--) {  // find starting point
      if(min_el[az] <= max_el[az]) break;
   }
   if(min_el[az] > max_el[az]) return;  // no data to plot

   el = max_el[az];
   for(az=0; az<=360; az++) {
      if(min_el[az] <= max_el[az]) el = max_el[az];
      max_az_clip[az] = el;
   }

   // now find the min elevation bounds
   for(az=360; az>0; az--) {
      if(min_el[az] <= max_el[az]) break;
   }

   el = min_el[az];
   for(az=0; az<=360; az++) {
      if(min_el[az] <= max_el[az]) el = min_el[az];
      min_az_clip[az] = el;
   }
}

void db_arc(int start, int end, int el, int arc_color)
{
int x1, y1;
int x2, y2;
float elf;
int az;
int color;
int thickness;

   // color in an arc of azimuth at the given elevation.

   thickness = 1;
   if(el) {
      thickness = (AZEL_SIZE+99) / 100;
      if(thickness <= 0) thickness = 1;
   }

   if(start < 0) start += 360;
   elf = ((float)(90 - el)) / 90.0F;
   az = start + 90;
   x1 = AZEL_X - (int) (elf * cos360(az) * AZEL_RADIUS) - (thickness/2)-1;
   y1 = AZEL_Y - (int) (elf * sin360(az) * AZEL_RADIUS) - (thickness/2)-0;

   while(start <= end) {
      az = start + 90;

      if     (el < min_el[start]) color = BLACK;
      else if(el > max_el[start]) color = BLACK;
      else                        color = arc_color;

//    if     (el < min_az_clip[start]) color = BLACK;
//    else if(el > max_az_clip[start]) color = BLACK;
//    else                             color = arc_color;

      if(1) {  // use large dots to draw the arc
         x2 = AZEL_X - (int) (elf * cos360(az) * AZEL_RADIUS); 
         y2 = AZEL_Y - (int) (elf * sin360(az) * AZEL_RADIUS); 
         if(color != BLACK) draw_circle(x2,y2, thickness, color, 1);
      }
      else {   // use thick lines to draw the arc
         x2 = AZEL_X - (int) (elf * cos360(az) * AZEL_RADIUS) - (thickness/2)-1; 
         y2 = AZEL_Y - (int) (elf * sin360(az) * AZEL_RADIUS) - (thickness/2)-0; 
         thick_line(x1,y1, x2,y2, color, thickness);
      }

      x1 = x2;
      y1 = y2;
      ++start;
   }
}

int start_color;

#define HIGH_LATS 55.0

int next_az_point(int start_az, int el)
{
   if(start_az >= 360) return 360;
   while(++start_az < 360) {
      if(db_3d_count[start_az][el] > 0.0F) break;
      if((lat < 0.0) || (DABS(lat*RAD_TO_DEG) > HIGH_LATS)) {
         if(start_az == 180) break;
      }
   }
   return start_az;
}

void plot_3d_signals(int el)
{
int color;
float elf;
int start_az, center_az, end_az;
int saz, eaz;
#define MIN_ARC 1   // minimum pixel expansion size
#define MAX_ARC 44  // maximum pixel exapansion in degrees azimuth
int max_arc;  
int last_max;

   // draw the signal level vs az/el map, filling in sparse data areas as
   // best we can.

   max_arc = last_max = MAX_ARC;
   start_az = 0;
   color = 0;
   while(start_az < 360) {
      center_az = next_az_point(start_az, el);
      end_az = next_az_point(center_az, el);

      // satellite sky coverage has holes towards the poles
      // we better define them by shrinking the max size of the arcs
      // when they are where the holes might be
      last_max = max_arc;
      max_arc = MAX_ARC;
      if((lat < 0.0) || ((lat*RAD_TO_DEG) >= HIGH_LATS)) {
         if     (start_az == 180)  max_arc = MIN_ARC;
         else if(center_az == 180) max_arc = MIN_ARC;
         else if(end_az == 180)    max_arc = MIN_ARC;
      }
      if((lat >= 0.0) || ((lat*RAD_TO_DEG) <= (-HIGH_LATS))) {
         if     (start_az == 0) max_arc = MIN_ARC;
         else if(end_az == 360) max_arc = MIN_ARC;
      }
      end_az = (center_az + end_az) / 2;

      if(db_3d_count[center_az][el] > 0.0F) {
         elf = db_3d_sum[center_az][el] / db_3d_count[center_az][el];
         elf -= min_sig_db;
         if(sig_level_step)            color = (int) (elf / (float) sig_level_step);
         if     (color > LEVEL_COLORS) color = LEVEL_COLORS;
         else if(color <= BLACK)       color = LOW_SIGNAL;
         else                          color = level_color[color];
      }
      else {
         color = BLACK;
      }

      if((start_az == 0) && (color != BLACK)) start_color = color;

      if((center_az - start_az) > max_arc)           saz = center_az - max_arc;
      else if((lat < 0.0F) && (last_max <= MIN_ARC)) saz = center_az - MIN_ARC; 
      else                                           saz = start_az;
      if((end_az - center_az) > max_arc)             eaz = center_az + max_arc;
      else if((lat < 0.0F) && (last_max <= MIN_ARC)) eaz = center_az + MIN_ARC;
      else                                           eaz = end_az;

      if(1 && (saz >= 359)) {
//if(debug_file) fprintf(debug_file, "saz:%d eaz:%d el:%d  color:%d\n", saz,eaz,el,color);
      }
      else if(color != BLACK) {
         db_arc(saz, eaz, el, color);
      }
      else if(end_az >= 360) {
         if(start_color != BLACK) db_arc(saz, eaz, el, start_color);
      }

      start_az = end_az;
   }
}


void draw_3d_signals()
{
int el;

   // draw signal level map as a function of az/el
   start_color = BLACK;
   if(plot_signals == 4) {  // find min and max elevations seen at each az angle
      calc_az_clip();
   }
   else if(plot_signals) ;
   else if(zoom_screen == 'S') {
      calc_az_clip();
   }

   for(el=0; el<90; el++) {
      if(plot_signals >= 5) {  // just show raw data points
         plot_raw_signals(el);
      }
      else {                   // show fully expanded color data plot
         plot_3d_signals(el);
      }
   }

   azel_grid_color = WHITE;  // AZEL_COLOR;
   draw_azel_grid(1, AZEL_X,AZEL_Y, AZEL_RADIUS);
}

void draw_az_levels()
{
int first_az;
int az;
int x0, y0;
int x1, y1;
int x2, y2;
float r;
float max_db;
float max_weighted_db;
int color;

   // show signal level vesus azimuth
   first_az = (-1);
   max_db = max_weighted_db = 0.0F;

   for(x0=0; x0<360; x0++) {  // find max db values
      if(db_az_sum[x0] && db_az_count[x0]) {
         r = db_az_sum[x0] / db_az_count[x0];
         if(r > max_db) {
            max_db = r;
            if(first_az < 0) first_az = x0;
         }
      }
      if(db_weighted_az_sum[x0] && db_az_count[x0]) {
         r = db_weighted_az_sum[x0] / db_az_count[x0];
         if(r > max_weighted_db) {
            max_weighted_db = r;
         }
      }
   }
   if(first_az < 0) return;
   if(max_db == 0.0F) return;
   if(max_weighted_db == 0.0F) return;

   db_az_count[360] = db_az_count[0];
   db_az_sum[360] = db_az_sum[0];

   if(db_az_count[first_az] == 0) r = 0.0;
   else if(plot_signals == 1)     r = (db_az_sum[first_az] / db_az_count[first_az]) / max_db;
   else                           r = (db_weighted_az_sum[first_az] / db_az_count[first_az]) / max_weighted_db;
   if(r > 1.0F) r = 1.0F;
   x0 = x1 = AZEL_X - (int) (r * cos360(0+90) * AZEL_RADIUS);
   y0 = y1 = AZEL_Y - (int) (r * sin360(0+90) * AZEL_RADIUS);

   for(az=0; az<=360; az+=1) {
      if(db_az_count[az] && db_az_sum[az]) {
         if(plot_signals == 1) r = (db_az_sum[az] / db_az_count[az]) / max_db;
         else                  r = (db_weighted_az_sum[az] / db_az_count[az]) / max_weighted_db;
         if(r >= 1.0F) r = 1.0F;
         color = GREEN;
      }
      else color = BLUE;

      x2 = AZEL_X - (int) (r * cos360(az+90) * AZEL_RADIUS);
      y2 = AZEL_Y - (int) (r * sin360(az+90) * AZEL_RADIUS);

      line(x1,y1, x2,y2, color);
      x1 = x2;
      y1 = y2;
   }

   line(x0,y0, x1,y1, LEVEL_COLOR);
}

void draw_el_levels()
{
int first_el;
int el;
int x1, y1;
int x2, y2;
float max_db;
float len;
float r;
int color;

   // show signal level versus elevation
   for(first_el=0; first_el<=90; first_el++) {
      if(db_el_count[first_el] != 0.0F) break;
   }
   if(first_el > 89) return;   // no data recorded

   max_db = 0.0F;
   for(el=0; el<=90; el++) {  // find max db value
      if(db_el_count[el]) {
         r = db_el_sum[el] / db_el_count[el];
         if(r > max_db) max_db = r;
      }
   }
   if(max_db == 0.0F) return;

   r = (db_el_sum[first_el] / db_el_count[first_el]) / max_db;
   if(r > 1.0F) r = 1.0F;

   len = (float) (AZEL_RADIUS*2);
   x1 = AZEL_X-AZEL_RADIUS + (int) (r * cos360(0+90) * len);
   y1 = AZEL_Y+AZEL_RADIUS - (int) (r * sin360(0+90) * len);

   for(el=0; el<=90; el++) {
      if(db_el_count[el] && db_el_sum[el]) {
         r = (db_el_sum[el] / db_el_count[el]) / max_db;
         if(r > 1.0F) r = 1.0F;
         color = GREEN;
      }
      else color = BLUE;

      x2 = AZEL_X-AZEL_RADIUS + (int) (r * cos360(el) * len);
      y2 = AZEL_Y+AZEL_RADIUS - (int) (r * sin360(el) * len);

      if(el != 0) line(x1,y1, x2,y2, color);
      x1 = x2;
      y1 = y2;
   }

   el = good_el_level();
   x1 = AZEL_X-AZEL_RADIUS + (int) (0.99F * cos360(el) * len);
   y1 = AZEL_Y+AZEL_RADIUS - (int) (0.99F * sin360(el) * len);
   x2 = AZEL_X-AZEL_RADIUS + (int) (1.03F * cos360(el) * len);
   y2 = AZEL_Y+AZEL_RADIUS - (int) (1.03F * sin360(el) * len);
   thick_line(x1,y1, x2,y2, YELLOW, 3);

   if(have_el_mask) {
      el = (int) el_mask;
      x1 = AZEL_X-AZEL_RADIUS + (int) (0.99F * cos360(el) * len);
      y1 = AZEL_Y+AZEL_RADIUS - (int) (0.99F * sin360(el) * len);
      x2 = AZEL_X-AZEL_RADIUS + (int) (1.03F * cos360(el) * len);
      y2 = AZEL_Y+AZEL_RADIUS - (int) (1.03F * sin360(el) * len);
      thick_line(x1,y1, x2,y2, BLUE, 3);
   }
}

#define GOOD_EL_LEVEL  (30)    // if no good signal found, use this elevation
#define GOOD_SIG_LEVEL (20.0F) // if no good signal level found, use this sig level mask
#define EL_THRESHOLD   ((res_t && (res_t != RES_T)) ? 0.750F:0.875F)  // "good" is this percent of max signal
// #define EL_THRESHOLD   ((res_t && (res_t != RES_T)) ? 0.750F:0.750F)  // "good" is this percent of max signal

int good_el_level()
{
int el;
float max_db;
float r;
#define MAX_GOOD_EL 75

   // find lowest EL level that has good signal
   max_db = 0.0F;
   for(el=0; el<=MAX_GOOD_EL; el++) {  // find max db value
      if(db_el_count[el]) {
         r = db_el_sum[el] / db_el_count[el];
         if(r > max_db) max_db = r;
      }
   }
   if(max_db == 0.0F) return GOOD_EL_LEVEL;

   for(el=0; el<=MAX_GOOD_EL; el++) {
      if(db_el_count[el] && db_el_sum[el]) {
         r = (db_el_sum[el] / db_el_count[el]) / max_db;
         if(r > EL_THRESHOLD) {
            return el;
         }
      }
   }

   return GOOD_EL_LEVEL;
}


int good_sig_level()
{
float count, sum;
int prn;

   // find reasonable signal level mask setting (0.4 * the average max sig level)

   count = sum = 0.0;
   for(prn=1; prn<=MAX_PRN; prn++) {
      if(max_sat_db[prn]) {
         sum += max_sat_db[prn];
         ++count;
      }
   }

   if(count) {
      sum /= count;
      sum *= 0.40F;
   }
   else {
      sum = GOOD_SIG_LEVEL;
   }

   return (int) (sum+0.50F);
}

void draw_signal_map(int why)
{
   if(text_mode) return;
   if(rcvr_type == NO_RCVR) return;
   if(rcvr_type == TIDE_RCVR) return;  // ckckck
   if(zoom_screen == 'L') return;

   if     (zoom_screen == 'S') ;
   else if(zoom_screen == 'X') ;
   else if(zoom_screen == 'Y') ;
   else if(zoom_screen == 'U') ;
   else if(zoom_screen == 'V') ;
   else if(zoom_screen == 'I') ;
   else if(zoom_screen) return;
   else if(plot_signals == 0) return;
if(first_key && shared_plot && (shared_item & SHARE_SIGNALS)) return;

   AZEL_X = (AZEL_COL+AZEL_SIZE/2-1);   // center point of plot
   AZEL_Y = (AZEL_ROW+AZEL_SIZE/2-1);   

   if(zoom_screen == 'X') ;
   else if(zoom_screen == 'Y') ;
   else if(zoom_screen == 'I') ;
   else {
      erase_azel(1);       // erase the old az/el plot
   }

   if(AZEL_Y >= PLOT_ROW) shared_item = SHARE_SIGNALS;

   azel_grid_color = AZEL_COLOR;
   draw_azel_prns(1);
   draw_azel_grid(1, AZEL_X,AZEL_Y, AZEL_RADIUS); // now draw the az/el grid

   if(plot_signals >= 4) {  // sky map
      draw_3d_signals();
   }
   else if(plot_signals == 3) {
      draw_el_levels(); // signal level vs elevation
   }
   else if((plot_signals == 1) || (plot_signals == 2)) {
      draw_az_levels(); // signal level vs azimuth
   }
   else if((zoom_screen == 'S') || (zoom_screen == 'X') || (zoom_screen == 'Y') || (zoom_screen == 'I')) {
      draw_3d_signals();
   }
}


void zoom_all_signals()
{
int old_sigs;
int old_row, old_col;
int old_size;
int w;

   old_sigs = plot_signals;
   old_row = AZEL_ROW;
   old_col = AZEL_COL;
   old_size = AZEL_SIZE;

   if((SCREEN_WIDTH < TINY_TINY_WIDTH) || (SCREEN_WIDTH < SCREEN_HEIGHT)) {
      w = SCREEN_WIDTH;
   }
   else {
      w = SCREEN_HEIGHT;
   }
   AZEL_SIZE = ((w / 2) * 8) / 10;

   plot_signals = 1;
   AZEL_COL = 0 + TEXT_WIDTH*8;
   AZEL_ROW = TEXT_HEIGHT*2;
   erase_azel(2);
   draw_signal_map(3);

   plot_signals = 2;
   AZEL_COL = SCREEN_WIDTH/2;
   erase_azel(3);
   draw_signal_map(4);

   plot_signals = 3;
   AZEL_COL = 0 + TEXT_WIDTH*8;
   AZEL_ROW = (SCREEN_HEIGHT/2) + (TEXT_HEIGHT/2);
   erase_azel(4);
   draw_signal_map(5);

   plot_signals = 4;
   AZEL_COL = SCREEN_WIDTH/2;
   erase_azel(5);
   draw_signal_map(6);

   AZEL_COL = old_col;
   AZEL_ROW = old_row;
   AZEL_SIZE = old_size;
   plot_signals = old_sigs;
}


void zoom_stuff()
{
int old_sigs;
int old_row, old_col;
int old_size;

   old_sigs = plot_signals;
   old_row = AZEL_ROW;
   old_col = AZEL_COL;
   old_size = AZEL_SIZE;

   if((SCREEN_WIDTH < TINY_TINY_WIDTH) || (SCREEN_WIDTH < SCREEN_HEIGHT)) { 
      AZEL_SIZE = ((SCREEN_WIDTH / 2) * 8) / 10;
   }
   else {
      AZEL_SIZE = ((SCREEN_HEIGHT / 2) * 8) / 10;
   }

   erase_screen();

   plot_signals = 0;
   ACLOCK_SIZE = AZEL_SIZE;
   aclock_x = 0 + TEXT_WIDTH*8 + ACLOCK_SIZE/2;
   aclock_y = TEXT_HEIGHT*2 + ACLOCK_SIZE/2;
   draw_watch(9);

   AZEL_ROW = TEXT_HEIGHT*2;
   AZEL_COL = SCREEN_WIDTH/2;
// AZEL_RADIUS = AZEL_SIZE/2;
   erase_azel(6);
   draw_azel_plot(4);

   plot_signals = 2;
   AZEL_COL = 0 + TEXT_WIDTH*8;
   AZEL_ROW = (SCREEN_HEIGHT/2) + (TEXT_HEIGHT/2);
   erase_azel(7);
   draw_signal_map(7);

   plot_signals = 4;
   AZEL_COL = SCREEN_WIDTH/2;
   erase_azel(8);
   draw_signal_map(8);

   AZEL_COL = old_col;
   AZEL_ROW = old_row;
   AZEL_SIZE = old_size;
   plot_signals = old_sigs;
}


void zoom_sat_info()
{
int old_sigs;
int old_row, old_col;
int old_size;
int old_clk_r;

   old_sigs = plot_signals;
   old_row = AZEL_ROW;
   old_col = AZEL_COL;
   old_size = AZEL_SIZE;
   old_clk_r = ACLOCK_SIZE;

   if(SCREEN_HEIGHT < TINY_HEIGHT) AZEL_SIZE = ((SCREEN_WIDTH-((MINOR_COL+4)*TEXT_WIDTH)) * 6) / 10;
   else AZEL_SIZE = ((SCREEN_WIDTH-((MINOR_COL+4)*TEXT_WIDTH)) * 8) / 10;

   if(AZEL_SIZE > (SCREEN_HEIGHT/2)) AZEL_SIZE = ((SCREEN_HEIGHT/2) * 8) / 10;
   if(AZEL_SIZE > MAX_AZEL) AZEL_SIZE = MAX_AZEL;

   plot_signals = 0;
   AZEL_ROW = TEXT_HEIGHT*2;
   AZEL_COL = (MINOR_COL+4)*TEXT_WIDTH;
// AZEL_RADIUS = AZEL_SIZE/2;
   ACLOCK_SIZE = AZEL_SIZE;
   erase_azel(9);
   draw_azel_plot(5);

   plot_signals = 4;
   AZEL_ROW = (SCREEN_HEIGHT/2);// + TEXT_WIDTH*8;
   AZEL_COL = (MINOR_COL+4)*TEXT_WIDTH;
   erase_azel(10);
   draw_signal_map(9);

   AZEL_COL = old_col;
   AZEL_ROW = old_row;
   AZEL_SIZE = old_size;
   ACLOCK_SIZE= old_clk_r;
   plot_signals = old_sigs;
}

#endif // SIG_LEVELS

void clear_maps()
{
   // clear the signal level at sat position data

#ifdef SAT_TRAILS
   clear_sat_trails();
   need_redraw = 1101;
#endif

#ifdef SIG_LEVELS
   clear_signals();
   need_redraw = 1102;
#endif

   need_posns = 2;
}

void draw_azel_plot(int why)
{
int save_row, save_col, save_size;
int did_watch;

   // draw satellite position map, analog watch, and/or signal level map

   if(text_mode) return;
   did_watch = 99;
   if(zoom_screen == 'L') return;
   if(zoom_screen == 'C') return;
   if((zoom_screen == 'M') || (zoom_screen == 'B') || (zoom_screen == 'X')) ;
   else if(zoom_screen == 'Y') ;
   else if(zoom_screen == 'V') ;
   else if(zoom_screen == 'I') ;
   else if(plot_azel == 0) return;

   if     (zoom_screen == 'W') ;
   else if(zoom_screen == 'B') erase_screen();
   else if(zoom_screen == 'M') ;
   else if(zoom_screen == 'S') ;
   else if(zoom_screen == 'X') ;
   else if(zoom_screen == 'Y') ;
   else if(zoom_screen == 'V') ;
   else if(zoom_screen == 'I') ;
   else if(zoom_screen == 'N') {
      erase_screen();
      return;
   }
   else if(zoom_screen) return;

// if(all_adevs && plot_signals && shared_plot) return;  //zzzzzzz
   if(plot_lla && (shared_plot == 0)) return;
   if(first_key && shared_plot && (shared_item & SHARE_MAP)) return;

   save_row = AZEL_ROW;
   save_col = AZEL_COL;
   save_size = AZEL_SIZE;

   AZEL_X = (AZEL_COL+AZEL_SIZE/2-1);   // center point of plot
   AZEL_Y = (AZEL_ROW+AZEL_SIZE/2-1);   


   if(zoom_screen == 'N') {  // blank (mostly) screen
      erase_screen();
      return;
   }

if(last_sun_x && last_sun_y && last_sun_r && (show_fixes == 0) && (zoom_screen != 'V')) {  // erase the old sun disk
   draw_circle(last_sun_x,last_sun_y, last_sun_r, BLACK, 1);
}
   if((zoom_screen == 'W') || (zoom_screen == 'B')) {
      if(luxor && luxor_fault()) ;
      else draw_watch(3);
   }
   else if(zoom_screen) ;
   else if(map_and_watch) {
      small_screen_watch();
      if(luxor && luxor_fault()) ;
      else did_watch = draw_watch(4);
   } 
   else if((plot_watch && (plot_signals == 0)) || (zoom_screen == 'B')) {  // both azel and watch want to be on the screen
      if(all_adevs) {
         if(WIDE_SCREEN == 0) return;
      }
      else if(WIDE_SCREEN && (shared_plot == 0) && plot_lla) return;  // the watch wins
      else if(WIDE_SCREEN && plot_lla) ;
      else if(WIDE_SCREEN && (shared_plot == 0) && (plot_lla == 0)) {
         AZEL_SIZE = LLA_SIZE;
         AZEL_ROW = LLA_ROW;
         AZEL_COL = LLA_COL;
      }
      else if((shared_plot == 0) || all_adevs || plot_lla) {
         return;  // the watch wins
      }
   }
   else if(all_adevs && WIDE_SCREEN && (plot_lla == 0)) {
   }

   if(zoom_screen == 'V') ;
   else if(zoom_screen == 'I') ;
   else if((plot_signals == 0) && (plot_areas == 2)) ;
   else if(map_and_watch && plot_lla && (plot_areas == 1)) ;
   else if(map_and_sigs && plot_lla && (plot_areas == 1)) ;
   else if(map_and_watch && (SCREEN_WIDTH >= XXXX) && (all_adevs == SINGLE_ADEVS)) {
      AZEL_ROW = WATCH_ROW;
      AZEL_COL = WATCH_COL;
      AZEL_SIZE = WATCH_SIZE;
   }
// else if(plot_signals && plot_azel && (SCREEN_WIDTH >= 1280)) {
   else if(plot_signals && plot_azel && (SCREEN_WIDTH >= WIDE_WIDTH) && (all_adevs == SINGLE_ADEVS)) {
      AZEL_ROW = WATCH_ROW;
      AZEL_COL = WATCH_COL;
      AZEL_SIZE = WATCH_SIZE;
   }
   else if(plot_signals && plot_azel && shared_plot && (all_adevs == SINGLE_ADEVS) && (SCREEN_WIDTH >= XXXX)) {
      AZEL_ROW = WATCH_ROW;
      AZEL_COL = WATCH_COL;
      AZEL_SIZE = WATCH_SIZE;
   }
// else if((zoom_screen == 'X') || (zoom_screen == 'Y')) {
//    AZEL_ROW = WATCH_ROW;
//    AZEL_COL = WATCH_COL;
//    AZEL_SIZE = WATCH_SIZE;
// }

   AZEL_X = (AZEL_COL+AZEL_SIZE/2-1);   // center point of plot
   AZEL_Y = (AZEL_ROW+AZEL_SIZE/2-1);   

   if(rcvr_type == NO_RCVR) return;
//// if(rcvr_type == TIDE_RCVR) return;  // ckckck

   if(zoom_screen == 'B') ;
   else if(zoom_screen) ;     
   else if(map_and_watch || map_and_sigs) ;
// else if((did_watch == 0) && (AZEL_ROW == WATCH_ROW) && (AZEL_COL == WATCH_COL)) ;
   else erase_azel(11);       // erase the old az/el plot


   azel_grid_color = AZEL_COLOR;
   if(zoom_screen == 'M') {
      erase_azel(12);
      draw_azel_prns(0);   // label the azel plot
      draw_azel_grid(0, AZEL_X,AZEL_Y, AZEL_RADIUS); // now draw the az/el grid
   }
   else if(zoom_screen == 'X') {
      erase_azel(13);
      draw_azel_prns(0);   // label the azel plot
      draw_azel_grid(0, AZEL_X,AZEL_Y, AZEL_RADIUS); // now draw the az/el grid
      draw_signal_map(10);
      draw_azel_sats(AZEL_X,AZEL_Y, AZEL_RADIUS);   // fill in the satellites
      draw_watch(66);
   }
   else if(zoom_screen == 'Y') {
      erase_azel(14);
      draw_azel_prns(11);   // label the azel plot
      draw_azel_grid(0, AZEL_X,AZEL_Y, AZEL_RADIUS); // now draw the az/el grid
      draw_signal_map(12);
      draw_azel_sats(AZEL_X,AZEL_Y, AZEL_RADIUS);   // fill in the satellites
   }
   else if(zoom_screen == 'B') {
      draw_azel_prns(0);   // label the azel plot
      draw_azel_grid(0, AZEL_X,AZEL_Y, AZEL_RADIUS); // now draw the az/el grid
   }
   else if(zoom_screen == 'S') {
      draw_azel_prns(1);   // label the azel plot
      draw_azel_grid(1, AZEL_X,AZEL_Y, AZEL_RADIUS); // now draw the az/el grid
   }
   else if(zoom_screen == 'V') {
      draw_azel_prns(1);   // label the azel plot
      draw_azel_grid(1, AZEL_X,AZEL_Y, AZEL_RADIUS); // now draw the az/el grid
      draw_azel_sats(AZEL_X,AZEL_Y, AZEL_RADIUS);    // fill in the satellites
   }
   else if(zoom_screen == 'I') {
      draw_azel_prns(1);   // label the azel plot
      draw_azel_grid(1, AZEL_X,AZEL_Y, AZEL_RADIUS); // now draw the az/el grid
      draw_azel_sats(AZEL_X,AZEL_Y, AZEL_RADIUS);   // fill in the satellites
   }
   else if(zoom_screen) ;
   else if(map_and_watch || (map_and_sigs == 0)) { 
      draw_azel_prns(0);   // label the azel plot
      draw_azel_grid(0, AZEL_X,AZEL_Y, AZEL_RADIUS); // now draw the az/el grid
   }

   if(0 && (rcvr_type == NO_RCVR)) ;  // wwwwww
   else if(zoom_screen == 'X') ;
   else if(zoom_screen == 'Y') ;
   else if(zoom_screen == 'I') ;
   else if(plot_azel || (zoom_screen == 'M') || (zoom_screen == 'B')) {
      draw_azel_sats(AZEL_X,AZEL_Y, AZEL_RADIUS);   // fill in the satellites
   }

   AZEL_ROW = save_row;
   AZEL_COL = save_col;
   AZEL_SIZE = save_size;
}

#endif  // AZEL_STUFF


//
//
//   Precision survey stuff
//
//
#ifdef PRECISE_STUFF

u08 debug_lla = 1;
#define SURVEY_COLOR        BLUE

#define LAT_REF (precise_lat*RAD_TO_DEG)
#define LON_REF (precise_lon*RAD_TO_DEG)
#define ALT_REF precise_alt
#define COS_FACTOR(x) cos((x)/RAD_TO_DEG)

long start_tow;
long minute_tow;
long hour_tow;

double interp;

double xlat, xlon, xalt;
double lat_sum, lon_sum, alt_sum;
double best_lat, best_lon, best_alt;
double best_count;

double lat_bins[SURVEY_BIN_COUNT+1];
double lon_bins[SURVEY_BIN_COUNT+1];
double alt_bins[SURVEY_BIN_COUNT+1];
int second_lats, second_lons, second_alts;

double lat_min_bins[SURVEY_BIN_COUNT+1];
double lon_min_bins[SURVEY_BIN_COUNT+1];
double alt_min_bins[SURVEY_BIN_COUNT+1];
int minute_lats, minute_lons, minute_alts;

double alt_hr_bins[SURVEY_BIN_COUNT+1];
int hour_alts;

#define LAT_THRESH (ANGLE_SCALE/RAD_TO_DEG)  //radian per foot
#define LON_THRESH (ANGLE_SCALE/RAD_TO_DEG)
#define ALT_THRESH (1.0)


// empirically determined weights for median calculation
#define WEIGHT_MEDIAN 0
#define LAT_SEC_TURN ((WEIGHT_MEDIAN && (rcvr_type == TSIP_RCVR)) ? 0.457374 : 0.50)
#define LON_SEC_TURN ((WEIGHT_MEDIAN && (rcvr_type == TSIP_RCVR)) ? 0.469305 : 0.50)
#define ALT_SEC_TURN ((WEIGHT_MEDIAN && (rcvr_type == TSIP_RCVR)) ? 0.461097 : 0.50)

#define LAT_MIN_TURN ((WEIGHT_MEDIAN && (rcvr_type == TSIP_RCVR)) ? 0.371318 : 0.50)
#define LON_MIN_TURN ((WEIGHT_MEDIAN && (rcvr_type == TSIP_RCVR)) ? 0.414684 : 0.50)
#define ALT_MIN_TURN ((WEIGHT_MEDIAN && (rcvr_type == TSIP_RCVR)) ? 0.405716 : 0.50)

#define LAT_HR_TURN  ((WEIGHT_MEDIAN && (rcvr_type == TSIP_RCVR)) ? 0.469675 : 0.50)
#define LON_HR_TURN  ((WEIGHT_MEDIAN && (rcvr_type == TSIP_RCVR)) ? 0.439141 : 0.50)
#define ALT_HR_TURN  ((WEIGHT_MEDIAN && (rcvr_type == TSIP_RCVR)) ? 0.416180 : 0.50)


double float_error(double val)
{
float x;

   // calculate the error between the single and double precision value
   x = (float) val;
   val = val - x;
   if(val < 0.0) val = 0.0 - val;
   return val;
}

void open_lla_file(int why)
{
   if(lla_file == 0) {
//    strcpy(out, LLA_FILE);
      sprintf(out, "%s.lla", unit_file_name);
      lla_file = topen(out, "w");
   }
   if(lla_file == 0) return;
//fprintf(lla_file, "### open_lla_file(%d)\n", why);

   month = fix_month(month);
   fprintf(lla_file, "#TITLE: LLA log %s: %02d %s %04d  %02d:%02d:%02d - receiver mode %d\n", 
      out, day, months[month], year, hours,minutes,seconds, rcvr_mode);
   fprintf(lla_file, "#LLA: %.9f %.9f %.4f\n", precise_lat*RAD_TO_DEG, precise_lon*RAD_TO_DEG, precise_alt);
}

void close_lla_file(int cleanup)
{
   if(lla_file == 0) return;
//fprintf(lla_file, "### close_lla_file(%d)\n", cleanup);
// if(cleanup) return; // "###
   fclose(lla_file);
   lla_file = 0;
}


void start_3d_fixes(int mode, int why)
{
   #ifdef BUFFER_LLA
      if(mode >= 0) clear_lla_points(0);
   #endif
// open_lla_file(1);

   if(mode >= 0) set_rcvr_mode(mode);  // 0=auto 2D/3D  or  4=3D only mode
   request_sat_list();

   plot_lla = 2;
   plot_azel = 1;
   update_azel = 1;
   if(WIDE_SCREEN == 0) {
      shared_plot = 1; 
      if(rcvr_type == TICC_RCVR) all_adevs = ALL_CHANS;
      else                       all_adevs = SINGLE_ADEVS;
   }
if(mode >= 0) precision_samples = 0L;
}

void start_precision_survey(int why)
{
   stop_self_survey(1);  // abort standard survey
   if(do_median_survey <= 0) { 
      do_median_survey = 48; 
      survey_why = 1;
   }
   else if(do_median_survey > SURVEY_BIN_COUNT) { 
      do_median_survey = 48; 
      survey_why = 2;
   }

   do_survey = do_median_survey; // medsurv
   set_rcvr_mode(RCVR_MODE_3D);  // put receiver into 3D mode

   if(log_comments && log_file) {
      sprintf(log_text, "# Precision %ld hour survey started.", do_median_survey);
      write_log_comment(1);
   }

   open_lla_file(2);
   if(lla_file) {
      fprintf(lla_file, "# Precision %ld hour survey started.\n", do_median_survey);
      fprintf(lla_file, "#\n");
      fprintf(lla_file, "#tow  status  lat  lon  alt\n");
      fprintf(lla_file, "#     status  0: doing fixes\n");
      fprintf(lla_file, "#     status  1: no time\n");
      fprintf(lla_file, "#     status  2: no fixes\n");
      fprintf(lla_file, "#     status  3: PDOP too high\n");
      fprintf(lla_file, "#     status  8: no sats\n");
      fprintf(lla_file, "#     status  9: one sat\n");
      fprintf(lla_file, "#     status  A: two sats\n");
      fprintf(lla_file, "#     status  B: three sats\n");
      fprintf(lla_file, "#     status  C: sat unavailable\n");
      fprintf(lla_file, "#     else:      unknown error\n");
      fprintf(lla_file, "#\n");
   }

   precision_survey = 1;   // config screen to show survey map
if(rcvr_type == NO_RCVR) ;
else if(1) {
   show_fixes = 1;
   change_fix_config(show_fixes);
   start_3d_fixes(-1, 555);
   #ifdef BUFFER_LLA
      clear_lla_points(0);
   #endif
}
else {
   show_fixes = 0;   // zzzzzz
   change_fix_config(show_fixes);
   
   plot_lla = 3;
   plot_azel = 1;
}
   update_azel = 1;
   if(WIDE_SCREEN == 0) {
      shared_plot = 1; 
      if(rcvr_type == TICC_RCVR) all_adevs = ALL_CHANS;
      else                       all_adevs = SINGLE_ADEVS;
   }
   config_screen(41);

   start_tow = tow;
//start_tow = survey_tow; //"###
   minute_tow = start_tow + 60L;
   hour_tow = start_tow + 60L*60L;
//if(debug_file) fprintf(debug_file, "surv_tow:%d  start_tow:%d  min_tow:%d  hr_tow:%d  psh:%d\n", survey_tow,start_tow,minute_tow,hour_tow, PRECISE_SURVEY_HOURS); //plugh

   lat_sum = lon_sum = alt_sum = 0.0;
   precision_samples = 0;
   survey_minutes = 0L;

   second_lats = second_lons = second_alts = 0;
   minute_lats = minute_lons = minute_alts = 0;
   hour_lats = hour_lons = hour_alts = 0;

   best_lat = best_lon = best_alt = 0.0;
   best_count = 0.0;

   plot_lla_axes(3);
}

void stop_precision_survey(int why)
{
   if(precision_survey == 0) return;
   precision_survey = 0;

//if(debug_file) fprintf(debug_file, "stop precision survey:%d\n", why); //plugh
   sprintf(log_text, "# Precision survey stopped.  why:%d", why);
   write_log_comment(1);

   show_fixes = 0;
   change_fix_config(show_fixes);
   set_rcvr_mode(RCVR_MODE_HOLD);     // overdetermined clock mode
   if(lla_file) { //!!!! kludge - add a extra entry so external processor program works correctly
      fprintf(lla_file, "# time filler kludge\n");
      fprintf(lla_file, "%-6ld %d %13.8f  %13.8f  %8.3f\n", 
      tow+60L, 0, lat*RAD_TO_DEG, lon*RAD_TO_DEG, alt);
   }

}

void precise_check()
{
double d_lat, d_lon, d_alt;

   if(surveying) return;
   if(check_precise_posn == 0) return;
   if(check_delay) {  // ignore first few survey points - they could be bogus filter remnants
      --check_delay;
      return;
   }

   d_lat = (lat-precise_lat)*RAD_TO_DEG/ANGLE_SCALE;
   d_lon = (lon-precise_lon)*RAD_TO_DEG/ANGLE_SCALE*cos_factor;
   d_alt = (alt-precise_alt);
// if((DABS(lat-precise_lat) < LAT_THRESH) && (DABS(lon-precise_lon) < LON_THRESH) && (DABS(alt-precise_alt) < ALT_THRESH)) {

   // see if surveyed position error is small enough
   if((sqrt(d_lat*d_lat + d_lon*d_lon) <= 1.0) && (DABS(d_alt) < ALT_THRESH)) {
      save_segment(7, 10);      // yes,  save the position in EEPROM
      check_precise_posn = 0;   // we are done
      log_saved_posn(-1);
      if(SCREEN_WIDTH > NARROW_SCREEN) lla_header("Precise survey complete    ", WHITE);
      else                             lla_header("Survey complete            ", WHITE);
      close_lla_file(1);
      precise_survey_done = 1;
      trimble_save = 0;
   }
   else { // try single point survey again
      start_self_survey(0x00, 2);
   }
}


void save_precise_posn(int quick_save)
{
   // see if single precision TSIP message position will be close enough 
   // !!!! resolution_SMT does not update lla values during survey, can't do precise save
   if(lla_file) {
      fprintf(lla_file, "### save_precise_position(%d)\n", quick_save);
      fprintf(lla_file, "### precise lla:%.9f %.9f %.3f\n", precise_lat*180.0/PI, precise_lon*180.0/PI, precise_alt);
      fprintf(lla_file, "### precise lat:%s\n", dms_fmt(precise_lat*180.0/PI));
      fprintf(lla_file, "### precise lon:%s\n", dms_fmt(precise_lon*180.0/PI));

      lla_to_ecef(precise_lat,precise_lon,precise_alt);
      fprintf(lla_file, "### precise ecef x:%.5f y:%.5f z:%.5f\n", ecef_x, ecef_y, ecef_z);
   }

// medsurv
   if((res_t && (res_t != RES_T))) quick_save = 1;
   else if(rcvr_type != TSIP_RCVR) quick_save = 1;

   if((quick_save >= 0) || (1 && (float_error(precise_lat) < LAT_THRESH) && (float_error(precise_lon) < LON_THRESH))) {
      stop_self_survey(2);  // abort standard survey
      check_precise_posn = 0;
      set_lla(precise_lat, precise_lon, precise_alt);

      if(NO_EEPROM_CMDS) ;
      else {
         save_segment(7, 11);     // save the position in EEPROM
         log_saved_posn(quick_save);
         sprintf(log_text, "# Saved precise position directly");
         if(lla_file) fprintf(lla_file, "%s\n", log_text);
         write_log_comment(1);
         if(SCREEN_WIDTH > NARROW_SCREEN) lla_header("Precise survey complete  ", WHITE);
         else                             lla_header("Survey complete  ", WHITE);
      }
      close_lla_file(2);
      precise_survey_done = 1;
      trimble_save = 0;
   }
   else {  // single/double precision difference is too big, do single point surveys until we get close
      set_survey_params(1, 0, 1L);  // config for single fix survey, don't save position
      start_self_survey(0x00, 3);   // start the survey

      check_precise_posn = 1;       // we are looking for a fix very close to where we are
      check_delay = 4;
      plot_lla = 4;                 // prepare to plot the search attempts
      plot_azel = 1;
      update_azel = 1;
      if(WIDE_SCREEN == 0) {
         shared_plot = 1; 
         if(rcvr_type == TICC_RCVR) all_adevs = ALL_CHANS;
         else                       all_adevs = SINGLE_ADEVS;
      }
      config_screen(42);
      if(SCREEN_WIDTH > NARROW_SCREEN) lla_header("Saving precise position  ", YELLOW);
      else                             lla_header("Saving position    ", YELLOW);
      refresh_page();
   }
}

void abort_precise_survey(int why)
{
float flat, flon, falt;

   if(check_precise_posn) {  // precise position found
      flat = (float) precise_lat;
      flon = (float) precise_lon;
      falt = (float) precise_alt;

      lat = (double) flat;
      lon = (double) flon;
      alt = (double) falt;
      save_precise_posn(2);  // save it using low res TSIP message
   }
   else if(precision_survey) {  // precise survey is incomplete
      if(precision_samples) {   // so save the average position
         precise_lat = lat_sum / (double)precision_samples;
         precise_lon = lon_sum / (double)precision_samples;
         precise_alt = alt_sum / (double)precision_samples;
         have_precise_lla = 1;
         ref_lat = precise_lat;
         ref_lon = precise_lon;
         ref_alt = precise_alt;
         cos_factor = cos(ref_lat);
         if(cos_factor == 0.0) cos_factor = 0.001;

         flat = (float) precise_lat;
         flon = (float) precise_lon;
         falt = (float) precise_alt;

         lat = (double) flat;
         lon = (double) flon;
         alt = (double) falt;
         save_precise_posn(3);
      }
   }
//if(lla_file) {
//   fprintf(lla_file, "### abort_precise_survey(%d): cpp:%d  ps:%d  samples:%d\n", why, check_precise_posn, precision_survey, precision_samples);
//   fprintf(lla_file, "### lla:%.9f %.9f %.9f\n", lat*180.0/PI,lon*180.0/PI,alt);
//}
}

//
//
//  Lat/lon/alt scattergram stuff
//
//

#define TIDE_UNITS    "mm"
#define TIDE_LLA_SPAN 15.0

double mouse_lat, mouse_lon;
double ref_lat_span, ref_lon_span;

int find_mouse_lla()
{
int map_r;
int x,y;

   // set mouse_lat and mouse_lon variables to the lat/lon in the scattergram
   // where the mouse cursor is located.  Returns true is mouse values are
   // valid.

   mouse_lat = mouse_lon = 0.0;
   if(autoscale_lla == 0) return 0;
   if(have_valid_lla == 0) return 0;
   
   map_r = LLA_SIZE / 2;
   if(mouse_x < LLA_X-map_r) return 0;
   if(mouse_x > LLA_X+map_r) return 0;
   if(mouse_y < LLA_Y-map_r) return 0;
   if(mouse_y > LLA_Y+map_r) return 0;

   x = mouse_x - LLA_X;
   y = LLA_Y - mouse_y;

   if(cos_factor == 0.0) cos_factor = 0.001;
   mouse_lat = (ref_lat*RAD_TO_DEG) + ( (((double) y) / (double) LLA_SIZE) * ref_lat_span ); 
   mouse_lon = (ref_lon*RAD_TO_DEG) + ( (((double) x) / (double) LLA_SIZE) * (ref_lon_span / cos_factor));
   return 1;
}

void label_lla_header()
{
int moused;
char s[SLEN];

   // draw the lat/lon scattergram label

   sprintf(s, "%s/div", angle_units);
   moused = 0;
   if(autoscale_lla && have_valid_lla) {
      strupr(s);
      moused = find_mouse_lla();
   }

if(autoscale_lla && show_debug_info) sprintf(debug_text5, "lla span: %.9f  ref:%.9f %.9f", LLA_SPAN,  ref_lat*RAD_TO_DEG,ref_lon*RAD_TO_DEG);  //llascale
   if(zoom_screen && (zoom_screen != 'L')) ;
   else if(text_mode || (SCREEN_WIDTH > NARROW_SCREEN)) {
      if(check_precise_posn) sprintf(out, "Saving precise position  ");
      else if(show_fixes) {
         if(rcvr_type == TIDE_RCVR)  sprintf(out, "Earth tide (%.1f %s/div from):   ", (TIDE_LLA_SPAN), TIDE_UNITS);   // ckckck
         else if(show_tides) sprintf(out, "Earth tide (%.1f %s/div from):   ", (TIDE_LLA_SPAN), TIDE_UNITS);   // ckckckck
         else if(moused) sprintf(out, "Mouse lat/lon:               ");
         else if(rcvr_mode == 4)     sprintf(out, "3D Fixes (%d %s from):     ", ((int)LLA_SPAN), s);
         else if(rcvr_mode == 3)     sprintf(out, "2D Fixes (%d %s from):     ", ((int)LLA_SPAN), s);
         else if(rcvr_mode == 0)     sprintf(out, "2D/3D Fixes (%d %s from):  ", ((int)LLA_SPAN), s);
         else if(configed_mode == 5) sprintf(out, "Mode %d Fixes (%d %s from):", configed_mode, ((int)LLA_SPAN), s);
         else                        sprintf(out, "Mode %d fixes (%d %s from):", rcvr_mode, ((int)LLA_SPAN), s);
      }
      else if(reading_lla)           sprintf(out, "Read LLA (%d %s from):     ", ((int)LLA_SPAN), s);
      else if((lla_file == 0) && lla_msg[0]) sprintf(out, "%s", lla_msg);
      else                           sprintf(out, "Surveying (%d %s from):    ", ((int)LLA_SPAN), s);
   }
   else {
      if(check_precise_posn) sprintf(out, "Saving position    ");
      else if((lla_file == 0) && lla_msg[0]) sprintf(out, "%s", lla_msg);
      else                   sprintf(out, "%d %s from: ", ((int)LLA_SPAN), s);
   }

   lla_header(out, YELLOW);
}

void show_mouse_lla()
{
int xx;

   // label the lat/lon scattergram

   if(text_mode && first_key) return;
   if(plot_lla == 0) return;

   xx = LLA_COL/TEXT_WIDTH+1;
   if(SCREEN_WIDTH > (NARROW_SCREEN+X11_MARGIN)) xx += 5;
   else                                          xx -= 2;

   if(find_mouse_lla()) {
      label_lla_header();
      format_lla(mouse_lat*DEG_TO_RAD,mouse_lon*DEG_TO_RAD,ref_alt, (LLA_ROW+LLA_SIZE)/TEXT_HEIGHT+1, xx, 0);
   }
   else {
      label_lla_header();
      format_lla(ref_lat,ref_lon,ref_alt, (LLA_ROW+LLA_SIZE)/TEXT_HEIGHT+1, xx, 0);
   }
}

double calc_lla_span(int force_show)
{
double lat_dist, lon_dist;
double max_dist;
static int last_sec = (-99);

   // return the maximum span distance of lat or lon in meters or feet

if(0) {
   if(lat == 0.0) return LLA_SPAN;
   if(lon == 0.0) return LLA_SPAN;
   if(min_q_lat == 0.0) return LLA_SPAN;
   if(min_q_lon == 0.0) return LLA_SPAN;
   if(max_q_lat == 0.0) return LLA_SPAN;
   if(max_q_lon == 0.0) return LLA_SPAN;
}

   lat_dist = greatcircle(min_q_lat,lon, max_q_lat,lon) * DEG_TO_RAD * 1000.0;  // lat span in meters
   lon_dist = greatcircle(lat,min_q_lon, lat,max_q_lon) * DEG_TO_RAD * 1000.0;  // lon span in meters
   lat_dist = fabs(lat_dist);
   lon_dist = fabs(lon_dist);

   max_dist = lat_dist;
   if(lon_dist > max_dist) max_dist = lon_dist;  // max span dist of lat/lon
   if(angle_units) {
      if(angle_units[0] == 'f') max_dist *= FEET_PER_METER;
      else if(angle_units[0] == 'l') max_dist *= LG_PER_METER;  // linguini per meter
   }
   max_dist /= ((double) LLA_DIVISIONS / 2.0);  // scale to LLA plot size
   max_dist = (double) (int) (max_dist+1.0);    // round up to next integer

   if(force_show || (last_sec != pri_seconds)) {  // rate limit the screen updates
      last_sec = pri_seconds;
if(autoscale_lla && show_debug_info) {
   sprintf(debug_text, "lat_d: %.9f  lon_d:%.9f  span:%.9f  %s/div", lat_dist,lon_dist,max_dist, alt_scale); //llascale
   sprintf(debug_text2, "lat:%.9f %.9f (%.9f)   lon:%.9f %.9f (%.9f)", min_q_lat,max_q_lat,max_q_lat-min_q_lat,  min_q_lon,max_q_lon,max_q_lon-min_q_lon); //llascale
}
      show_mouse_lla();
   }
   return max_dist;
}

void change_lla_scale()
{
double lw;
double radius;

   // re-scale the lat/lon scattergram if the lat/lon span changes

   LLA_SPAN = calc_lla_span(1);  // meters or feet per division
   user_set_ref_lla = 2;
   ref_lat = ((min_q_lat + max_q_lat) / 2.0) * DEG_TO_RAD;
   ref_lon = ((min_q_lon + max_q_lon) / 2.0) * DEG_TO_RAD;
   ref_alt = alt;

   ref_lat_span = (max_q_lat - min_q_lat);  // in degrees
   ref_lon_span = (max_q_lon - min_q_lon);

   cos_factor = cos(ref_lat);
   if(cos_factor == 0.0) cos_factor = 0.001;
   lw = ref_lon_span = ((double)LLA_SPAN * (double) LLA_DIVISIONS);  // scattergram width in meters or feet
   if(angle_units) {
      if(angle_units[0] == 'f') lw /= FEET_PER_METER;  // lw now in meters
      else if(angle_units[0] == 'l') lw /= LG_PER_METER;  // lw now in meters
   }

   radius = (2.0*PI*earth_radius(0.0,ref_alt)*1000.0);    // divided by earth circumference in meters
   if(radius > 0.0) ref_lon_span /= radius;
   else ref_lon_span = 0.0;
   ref_lon_span *= 360.0;  // scattergram width in degrees
if(show_debug_info) sprintf(debug_text3, "lla_span:%f  lla_divs:%d  lw:%.9f  ref_lon_span: %.9f degrees", LLA_SPAN,LLA_DIVISIONS,lw,ref_lon_span);  //llascale
   ref_lat_span = ref_lon_span;
if(show_debug_info) {
   sprintf(debug_text4, "new span,divs %d,%.2f:  last:%.2f  ref:%.9f %.9f  dist:%.9f %.9f",   //llascale
   LLA_DIVISIONS, LLA_SPAN, last_lla_span, ref_lat*RAD_TO_DEG,ref_lon*RAD_TO_DEG, ref_lat_span,ref_lon_span);
}
   last_lla_span = LLA_SPAN * LLA_THRESH;  //llascale
}


void plot_lla_point(int draw, int color)
{
double x,y;
double xi, yi;
double half_span;
double old_lat, old_lon, old_alt;
double old_ref_lat, old_ref_lon, old_ref_alt;
double old_span, old_scale, old_cos;
double span;

   // add a point to the lat/lon scattergram plot

   if(draw) {
      if(text_mode || (first_key && (SCREEN_HEIGHT < SHORT_SCREEN))) return;
      if(zoom_screen && (zoom_screen != 'L')) return; 
      if(0 && all_adevs && (WIDE_SCREEN == 0)) return;
      if((SCREEN_HEIGHT < SHORT_SCREEN) && (shared_plot == 0)) return;
   }

   old_lat = lat;
   old_lon = lon;
   old_alt = alt;
   old_ref_lat = ref_lat;
   old_ref_lon = ref_lon;
   old_ref_alt = ref_alt;
   old_span = LLA_SPAN;
   old_scale = ANGLE_SCALE;
   old_cos = cos_factor;

   if((rcvr_type == TIDE_RCVR) || show_tides) { // ckckck adjust scale factors for showing lat/lon earth tides
      LLA_SPAN = TIDE_LLA_SPAN;    // !!!!! 10? 15?
      ANGLE_SCALE = 1.0;
      cos_factor = 1.0;
      lat = 0.0 - (lat_tide / RAD_TO_DEG);
      lon = 0.0 - (lon_tide / RAD_TO_DEG);
      alt = alt_tide;
      ref_lat = 0.0;
      ref_lon = 0.0;
      tide_scatter = 1;
   }
   else if(1 && ((lat == 0.0) || (lon == 0.0))) {  // ignore likely invald location
      goto no_lla;
   }
   else {
      if(1 && autoscale_lla && have_valid_lla) {
         span = calc_lla_span(0);
         if(((lla_points < 2) && (span != last_lla_span)) || (span > last_lla_span)) {  // the plot scale has changed
//          autoscale_lla = 0;  // toggle autoscale off
//          change_lla_scale();

            autoscale_lla = 1;        // toggle autoscale back on
            change_lla_scale();

            if(in_rebuild_lla) ;      //llascale - prevent recursion with rebuild_lla_plot()
            else rebuild_lla_plot(0);
            need_redraw = 4321;

            old_ref_lat = ref_lat;
            old_ref_lon = ref_lon;
            old_ref_alt = alt;
            old_cos = cos_factor;
            old_span = LLA_SPAN;
            user_set_ref_lla = 3;
         }
      }
      tide_scatter = 0;
   }


   y = (ref_lat - lat) * RAD_TO_DEG / ANGLE_SCALE;   // x,y = meters (or feet) from center
   x = ((lon - ref_lon) * RAD_TO_DEG / ANGLE_SCALE) * cos_factor;

   half_span = (LLA_SPAN*(double)LLA_DIVISIONS/2.0); // meters each side of center
   if(half_span == 0.0) return;

   if(x > half_span) x = half_span;
   if(x < (-half_span)) x = (-half_span);

   if(y > half_span) y = half_span;
   if(y < (-half_span)) y = (-half_span);

   yi = (y * ((double)lla_width / 2.0)) / half_span;
   xi = (x * ((double)lla_width / 2.0)) / half_span;
   lla_xi = LLA_X + (int) xi;
   lla_yi = LLA_Y + (int) yi;
   if(draw) dot(lla_xi, lla_yi, color);

#ifdef BUFFER_LLA
   // save lla point in the buffer so we can redraw the screen later
   ++lla_points;
   color = (lla_points / (60*60));
   color = (color % 12) + 3;

   xi += ((double) lla_width/2.0);
   yi += ((double) lla_width/2.0);

   xi = ((double) xi * (double) MAX_LLA_SIZE / (double) lla_width);
   yi = ((double) yi * (double) MAX_LLA_SIZE / (double) lla_width);
   if((xi >= 0.0) && (xi < (double) MAX_LLA_SIZE) && (yi >= 0.0) && (yi < (double) MAX_LLA_SIZE)) {
      lla_data[(int)xi][(int)yi] = (u08) color;
   }
#endif

   no_lla:
   lat = old_lat;
   lon = old_lon;
   alt = old_alt;
   ref_lat = old_ref_lat;
   ref_lon = old_ref_lon;
   ref_alt = old_ref_alt;
   LLA_SPAN = old_span;
   ANGLE_SCALE = old_scale;
   cos_factor = old_cos;
}


void rebuild_lla_plot(int draw)
{
struct PLOT_Q q;
double old_lat, old_lon, old_alt;
double old_lat_tide, old_lon_tide, old_alt_tide;
long i;
long count;
int color;

   // rebuild the LLA scattergram image from the plot queue data

   old_lat = lat;
   old_lon = lon;
   old_alt = alt;
   old_lat_tide = lat_tide;
   old_lon_tide = lon_tide;
   old_alt_tide = alt_tide;

   plot_lla_axes(10);    // erase the old scattergram
   #ifdef BUFFER_LLA
      clear_lla_points(1);
   #endif
   count = 0;

   i = plot_q_out;   // dumping the full queue
   while(i != plot_q_in) {  // rebuilt the scattergram from plot queue lat/lon data
      q = get_plot_q(i);

      lat = (double) q.data[ONE] * DEG_TO_RAD;
      lon = (double) q.data[TWO] * DEG_TO_RAD;
      alt = (double) q.data[THREE];

      lat_tide = (double) q.data[ELEVEN];
      lon_tide = (double) q.data[TWELVE];
      alt_tide = (double) q.data[THIRTEEN];
 
      color = lla_points / (60L*60L);
      color %= 12;
      in_rebuild_lla = 1;   // prevent recursion with plot_lla_point()
      plot_lla_point(draw, color+3); 
      in_rebuild_lla = 0;

      if(0 && (count == 0)) {  // !!!! this does not work = draw X at start of scattergram data
         dot(lla_xi+1,lla_yi+1, color+3);
         dot(lla_xi+2,lla_yi+2, color+3);
         dot(lla_xi+1,lla_yi-1, color+3);
         dot(lla_xi+2,lla_yi-2, color+3);
         dot(lla_xi-1,lla_yi+1, color+3);
         dot(lla_xi-2,lla_yi+2, color+3);
         dot(lla_xi-1,lla_yi-1, color+3);
         dot(lla_xi-2,lla_yi-2, color+3);
      }

      if(++i >= plot_q_size) i = 0;
      if(++count >= plot_q_size) break;
   }

   lat = old_lat;
   lon = old_lon;
   alt = old_alt;
   lat_tide = old_lat_tide;
   lon_tide = old_lon_tide;
   alt_tide = old_alt_tide;
}

void lla_header(char *s, int color)
{
int xx;
int yy;

   // label the lat/lon/alt data block

   if(text_mode) {
      yy = MOUSE_ROW-1;
      xx = MOUSE_COL;
   }
   else {
      xx = (LLA_COL/TEXT_WIDTH)+1;
      yy = (LLA_ROW+LLA_SIZE)/TEXT_HEIGHT+0;
      if(SCREEN_WIDTH > (NARROW_SCREEN+X11_MARGIN)) xx += 5;
      else                                          xx -= 2;
   }

   vidstr(yy, xx, color, s);
   strcpy(lla_msg, s);
}

#ifdef BUFFER_LLA

void redraw_lla_points()
{
int x, y;
int xs, ys;
int xx,yy;

   // redraw splatter plot from the saved data points

   for(x=0; x<MAX_LLA_SIZE; x++) {
      xs = (int) ((float) x * (float) lla_width / (float) MAX_LLA_SIZE);
      if((xs < 0) || (xs >= MAX_LLA_SIZE)) continue;

      for(y=0; y<MAX_LLA_SIZE; y++) {
         ys = (int) ((float) y * (float) lla_width / (float) MAX_LLA_SIZE);
         if((ys < 0) || (ys >= MAX_LLA_SIZE)) continue;
         if(lla_data[x][y]) {
            xx = LLA_X+xs-(lla_width/2);
            yy = LLA_Y+ys-(lla_width/2);
            if(xx < 0) continue;
            if(yy < 0) continue;
            if(xx >= SCREEN_WIDTH) continue;
            if(yy >= SCREEN_HEIGHT) continue;
            dot(xx,yy, lla_data[x][y]);
         }
      }
   }
}

void clear_lla_points(int force_clear)
{
int x, y;

   // clear splatter plot data buffer

   if(force_clear) ;    // llascale
   else if(zoom_screen == 'L') return;

   for(x=0; x<MAX_LLA_SIZE; x++) {
      for(y=0; y<MAX_LLA_SIZE; y++) {
         lla_data[x][y] = 0;
      }
   }

   if(user_set_ref_lla == 0) {
      ref_lat = lat;
      ref_lon = lon;
      ref_alt = alt;
   }
   cos_factor = cos(ref_lat);
   if(cos_factor == 0.0) cos_factor = 0.001;
   lla_points = 0;
}
#endif  // BUFFER_LLA


void plot_lla_axes(int why)
{
int x,y;
int xx,yy;
int color;

   // draw the lat/lon scattergram grid and label it

   lla_showing = 0;
   if(plot_lla == 0) return;
   if(text_mode || (first_key && (SCREEN_HEIGHT < SHORT_SCREEN))) return;
if(first_key && shared_plot && (shared_item & SHARE_LLA)) return;
   if(zoom_screen && (zoom_screen != 'L')) return;

   lla_showing = 1;
   erase_lla();

   label_lla_header();
   if(text_mode) return;  // plot area is in use for help/warning message

   x = LLA_X - LLA_SIZE/2 + LLA_MARGIN;
   y = LLA_Y - LLA_SIZE/2 + LLA_MARGIN;
   if(LLA_ROW >= PLOT_ROW) shared_item = SHARE_LLA;

   for(yy=y; yy<=y+lla_step*LLA_DIVISIONS; yy+=lla_step) {  // horizontals
     color = GREY;
     if(yy == y) color = WHITE;
     if(yy == y+lla_step*LLA_DIVISIONS) color = WHITE;
     line(x,yy, x+lla_step*LLA_DIVISIONS,yy, color);
   }

   for(xx=x; xx<=x+lla_step*LLA_DIVISIONS; xx+=lla_step) {  // verticals
     color = GREY;
     if(xx == x) color = WHITE;
     if(xx == x+lla_step*LLA_DIVISIONS) color = WHITE;
     line(xx,y+1,    xx,y+lla_step*LLA_DIVISIONS-1, color);
   }

   // draw the center cross
   xx = LLA_X;
   yy = LLA_Y;
   dot(xx+0, yy-0, WHITE);
   dot(xx+0, yy-1, WHITE);
   dot(xx+0, yy-2, WHITE);
   dot(xx+0, yy-3, WHITE);
   dot(xx+0, yy-4, WHITE);
   dot(xx+0, yy-5, WHITE);
   dot(xx+0, yy+0, WHITE);
   dot(xx+0, yy+1, WHITE);
   dot(xx+0, yy+2, WHITE);
   dot(xx+0, yy+3, WHITE);
   dot(xx+0, yy+4, WHITE);
   dot(xx+0, yy+5, WHITE);

   dot(xx+1, yy+0, WHITE);
   dot(xx+2, yy+0, WHITE);
   dot(xx+3, yy+0, WHITE);
   dot(xx+4, yy+0, WHITE);
   dot(xx+5, yy+0, WHITE);
   dot(xx-1, yy+0, WHITE);
   dot(xx-2, yy+0, WHITE);
   dot(xx-3, yy+0, WHITE);
   dot(xx-4, yy+0, WHITE);
   dot(xx-5, yy+0, WHITE);

   xx = LLA_COL/TEXT_WIDTH+1;
   if(SCREEN_WIDTH > (NARROW_SCREEN+X11_MARGIN)) xx += 5;
   else                                          xx -= 2;

   show_mouse_lla();

   #ifdef BUFFER_LLA
      redraw_lla_points();
   #endif
}


void calc_precise_lla()
{
u08 have_lla;

   // calculate simple average of all surveyed points

   have_lla = 0;
   if(precision_samples) {
      xlat = lat_sum / (double)precision_samples;
      xlon = lon_sum / (double)precision_samples;
      xalt = alt_sum / (double)precision_samples;
      precise_lat = xlat;
      precise_lon = xlon;
      precise_alt = xalt;
      have_precise_lla = 2;
      ref_lat = precise_lat;
      ref_lon = precise_lon;
      ref_alt = precise_alt;
      cos_factor = cos(ref_lat);
      if(cos_factor == 0.0) cos_factor = 0.001;
      lla_to_ecef(xlat,xlon,xalt);

      xlat *= RAD_TO_DEG;
      xlon *= RAD_TO_DEG;
      if(lla_file) {
         fprintf(lla_file, "# Simple average of %.0f points:\n", (double)precision_samples);
         fprintf(lla_file, "#   lat:%14.10f  %.10f  ecef_x:%.5f  dms:%s\n",  xlat, (xlat-LAT_REF)/ANGLE_SCALE, ecef_x, dms_fmt(xlat));
         fprintf(lla_file, "#   lon:%14.10f  %.10f  ecef_y:%.5f  dms:%s\n",  xlon, (xlon-LON_REF)/ANGLE_SCALE*COS_FACTOR(xlat), ecef_y, dms_fmt(xlon));
         fprintf(lla_file, "#   alt:%14.10f  %.10f  ecef_z:%.5f\n",  xalt, (xalt-ALT_REF), ecef_z);
         fprintf(lla_file, "#\n");
      }
      have_lla |= 0x01;
   }


   // calculate average of all 24 hour median intervals
   if(best_count && best_lat && best_lon && best_alt) {
      xlat = best_lat / best_count;
      xlon = best_lon / best_count;
      xalt = best_alt / best_count;
      precise_lat = xlat;
      precise_lon = xlon;
      precise_alt = xalt;
      have_precise_lla = 3;
      ref_lat = xlat;
      ref_lon = xlon;
      ref_alt = xalt;
      cos_factor = cos(ref_lat);
      if(cos_factor == 0.0) cos_factor = 0.001;
      lla_to_ecef(xlat,xlon,xalt);

      xlat *= RAD_TO_DEG;
      xlon *= RAD_TO_DEG;
      if(lla_file) {
         fprintf(lla_file, "# REF: %.10f  %.10f  %.10f  cos=%.10f\n", LAT_REF, LON_REF, ALT_REF, COS_FACTOR(xlat));
         fprintf(lla_file, "# Average of %.0f 24 hour medians:\n", best_count);
         fprintf(lla_file, "#   lat:%14.10f  %.10f  ecef_x:%.5f  dms:%s\n",  xlat, (xlat-LAT_REF)/ANGLE_SCALE, ecef_x, dms_fmt(xlat));
         fprintf(lla_file, "#   lon:%14.10f  %.10f  ecef_y:%.5f  dms:%s\n",  xlon, (xlon-LON_REF)/ANGLE_SCALE*COS_FACTOR(xlat), ecef_y, dms_fmt(xlon));
         fprintf(lla_file, "#   alt:%14.10f  %.10f  ecef_z:%.5f\n",  xalt, (xalt-ALT_REF), ecef_z);
      }
      have_lla |= 0x02;
   }

   if(have_lla) {            // we have a good idea where we are
      save_precise_posn(-1);  // save it in the receiver (directly or via repeated single point surveys)
   }
//if(lla_file) {
//   fprintf(lla_file, "### calc_precise_lla: have:%d  ps:%d bc:%d bla:%d blo:%d bal:%d\n", 
//   have_lla, precision_samples, best_count,best_lat,best_lon,best_alt);
//}
}                             


int add_to_bin(double val,  double bin[],  int bins_filled)
{
int i;

   if(bins_filled >= SURVEY_BIN_COUNT) {
     return bins_filled;
   }

   bin[bins_filled] = val;
   for(i=bins_filled-1; i>=0; i--) {
      if(bin[i] > val) {
         bin[i+1] = bin[i];
         bin[i] = val;
      }
      else break;
   }

   bins_filled += 1;
   return bins_filled;
}


void show_precise_info(int flag)
{
int i;
double xlat,xlon,xalt;

   if(debug_lla) {
      for(i=flag; i<hour_lats; i++) {
         xlat = lat_hr_bins[i];
         xlon = lon_hr_bins[i];
         xalt = alt_hr_bins[i];

         xlat *= RAD_TO_DEG;
         xlon *= RAD_TO_DEG;
         if(lla_file) {
            fprintf(lla_file, "#\n");
            fprintf(lla_file, "# Hour %2d: lat:%14.9f  %.9f\n",  i+1, xlat, (xlat-LAT_REF)/ANGLE_SCALE);
            fprintf(lla_file, "# Hour %2d: lon:%14.9f  %.9f\n",  i+1, xlon, (xlon-LON_REF)/ANGLE_SCALE*COS_FACTOR(xlat));
            fprintf(lla_file, "# Hour %2d: alt:%14.9f  %.9f\n",  i+1, xalt, (xalt-ALT_REF)*FEET_PER_METER);
            fprintf(lla_file, "#\n");
         }
      }
   }
}


void analyze_hours()
{
int i;
int j;
int k;
int interval;

   if(hour_lats == 0) return;

   // copy hourly medians to working array
   for(i=0; i<hour_lats; i++) {
      lat_bins[i] = lat_hr_bins[i];
      lon_bins[i] = lon_hr_bins[i];
      alt_bins[i] = alt_hr_bins[i];
   }

   minute_lats = minute_lons = minute_alts = 0;

   k = hour_lats;
   if(k > 24) k = 24;   // the size of the intervals we are analyzing (should be 24 hours)
   j = hour_lats - 24;
   if(j < 1) j = 1;     // the number of overlapping 24 hour intervals we have

   if(lla_file) fprintf(lla_file, "#\n# Processing %d %d hour intervals:\n", j, k);

   for(interval=0; interval<j; interval++) {
      if(lla_file) fprintf(lla_file, "#\n# Interval %d:\n", interval+1);

      hour_lats = hour_lons = hour_alts = 0;
      for(i=0; i<k; i++) {    // sort the interval's data into the median arrays
         hour_lats = add_to_bin(lat_bins[interval+i], lat_hr_bins, hour_lats);
         hour_lons = add_to_bin(lon_bins[interval+i], lon_hr_bins, hour_lons);
         hour_alts = add_to_bin(alt_bins[interval+i], alt_hr_bins, hour_alts);
      }

      show_precise_info(0);

      // calculate the weighted median of the 24 hour lat/lon/alt data
      interp = (double) hour_lats * LAT_HR_TURN;
      i = (int) interp;
      if(i < (hour_lats-1)) {
         xlat = (lat_hr_bins[i+1]-lat_hr_bins[i]) * (interp-(double)i);
         xlat += lat_hr_bins[i];
         minute_lats = add_to_bin(xlat, lat_min_bins, minute_lats);
         best_lat += xlat;
         if(lla_file) fprintf(lla_file, "# BEST %d: lat:%14.9f  %.9f\n",  i, xlat*RAD_TO_DEG, (xlat*RAD_TO_DEG-LAT_REF)/ANGLE_SCALE);
      }

      interp = (double) hour_lons * LON_HR_TURN;
      i = (int) interp;
      if(i < (hour_lons-1)) {
         xlon = (lon_hr_bins[i+1]-lon_hr_bins[i]) * (interp-(double)i);
         xlon += lon_hr_bins[i];
         minute_lons = add_to_bin(xlon, lon_min_bins, minute_lons);
         best_lon += xlon;
         if(lla_file) fprintf(lla_file, "# BEST %d: lon:%14.9f  %.9f\n",  i, xlon*RAD_TO_DEG, (xlon*RAD_TO_DEG-LON_REF)/ANGLE_SCALE*COS_FACTOR(xlat*RAD_TO_DEG));
      }

      interp = (double) hour_alts * ALT_HR_TURN;
      i = (int) interp;
      if(i < (hour_alts-1)) {
         xalt = (alt_hr_bins[i+1]-alt_hr_bins[i]) * (interp-(double)i);
         xalt += alt_hr_bins[i];
         minute_alts = add_to_bin(xalt, alt_min_bins, minute_alts);
         best_alt += xalt;
         if(lla_file) fprintf(lla_file, "# BEST %d: alt:%14.9f  %.9f\n",  i, xalt,  (xalt-ALT_REF)*FEET_PER_METER);
      }
      if(lla_file) fprintf(lla_file, "#\n");

      ++best_count;
   }
}


int analyze_minutes()
{
int i;
int fault;

   fault = 0;

   interp = (double) minute_lats * LAT_MIN_TURN;
   i = (int) interp;
   if(i >= (minute_lats-1)) ++fault;
   else {
      xlat = (lat_min_bins[i+1]-lat_min_bins[i]) * (interp-(double)i);
      xlat += lat_min_bins[i];
   }

   interp = (double) minute_lons * LON_MIN_TURN;
   i = (int) interp;
   if(i >= (minute_lons-1)) ++fault;
   else {
      xlon = (lon_min_bins[i+1]-lon_min_bins[i]) * (interp-(double)i);
      xlon += lon_min_bins[i];
   }

   interp = (double) minute_alts * ALT_MIN_TURN;
   i = (int) interp;
   if(i >= (minute_alts-1)) ++fault;
   else {
      xalt = (alt_min_bins[i+1]-alt_min_bins[i]) * (interp-(double)i);
      xalt += alt_min_bins[i];
   }

   minute_lats = minute_lons = minute_alts = 0;
   while(hour_tow <= survey_tow) {
      hour_tow += 60L*60L;
   }
//if(debug_file) fprintf(debug_file, "anal_mins surv_tow:%d  hour_tow:%d  hr_lats:%d  sbc:%d  psh:%d\n", survey_tow,hour_tow,hour_lats,SURVEY_BIN_COUNT,PRECISE_SURVEY_HOURS);  //plugh

   if((fault == 0) && (hour_lats < SURVEY_BIN_COUNT)) {
      lat_hr_bins[hour_lats++] = xlat;
      lon_hr_bins[hour_lons++] = xlon;
      alt_hr_bins[hour_alts++] = xalt;
      if(hour_lats >= PRECISE_SURVEY_HOURS) {  // precise survey is complete
//if(debug_file) fprintf(debug_file, "hour_lats:%d  psh:%d\n", hour_lats, PRECISE_SURVEY_HOURS);  //plugh
         return 3;
      }

      if(erase_every_hour) {  // clear screen to prepare for the next hour of data
         if(precision_samples) {  // calculate new center point of plot based upon sample average
            precise_lat = lat_sum / precision_samples;
            precise_lon = lon_sum / precision_samples;
            precise_alt = alt_sum / precision_samples;
            have_precise_lla = 4;
         }
         else {   // or based upon last hour's median point
            precise_lat = xlat;
            precise_lon = xlon;
            precise_alt = xalt;
            have_precise_lla = 5;
         }
         ref_lat = precise_lat;
         ref_lon = precise_lon;
         ref_alt = precise_alt;
         cos_factor = cos(ref_lat);
         if(cos_factor == 0.0) cos_factor = 0.001;

         need_redraw = 1103;
      }

      if(!zoom_screen) {
         sprintf(out, "hr  %2d: %.8f %.8f %.3f     ", 
            hour_lats, xlat*RAD_TO_DEG, xlon*RAD_TO_DEG,xalt);
         vidstr(MOUSE_ROW-2, MOUSE_COL, SURVEY_COLOR, out);  //plugh +2
      }

      show_precise_info(hour_lats-1);
   }

   return 0;
}


int analyze_seconds()
{
int i;
int fault;

   fault = 0;

   interp = (double) second_lats * LAT_SEC_TURN;
   i = (int) interp;
   if((i+1) >= second_lats) ++fault;
   else {
      xlat = (lat_bins[i+1]-lat_bins[i]) * (interp-(double)i);
      xlat += lat_bins[i];
   }

   interp = (double) second_lons * LON_SEC_TURN;
   i = (int) interp;
   if((i+1) >= second_lats) ++fault;
   else {
      xlon = (lon_bins[i+1]-lon_bins[i]) * (interp-(double)i);
      xlon += lon_bins[i];
   }

   interp = (double) second_alts * ALT_SEC_TURN;
   i = (int) interp;
   if((i+1) >= second_lats) ++fault;
   else {
      xalt = (alt_bins[i+1]-alt_bins[i]) * (interp-(double)i);
      xalt += alt_bins[i];
   }

   if((fault == 0) && (second_lats >= 60)) {  // we have a full minute of data
      minute_lats = add_to_bin(xlat, lat_min_bins, minute_lats);
      minute_lons = add_to_bin(xlon, lon_min_bins, minute_lons);
      minute_alts = add_to_bin(xalt, alt_min_bins, minute_alts);
      if(!zoom_screen) {
         sprintf(out, "min %2d: %.8f %.8f %.3f     ", minute_lats, xlat*RAD_TO_DEG, xlon*RAD_TO_DEG, xalt);  //plugh +1
         vidstr(MOUSE_ROW-1, MOUSE_COL, SURVEY_COLOR, out);
      }
   }

   // prepare for the next minute
   second_lats = second_lons = second_alts = 0;
   while(minute_tow <= survey_tow) {
      minute_tow += 60L;
   }
//if(debug_file) fprintf(debug_file, "anal_secs surv_tow:%d  minute_tow:%d  hr_tow:%d  psh:%d\n", survey_tow,minute_tow,hour_tow, PRECISE_SURVEY_HOURS);  //plugh

   if(survey_tow >= hour_tow) {  // we have 60 minutes of data
      i = analyze_minutes();
//if(debug_file) fprintf(debug_file, "analyze_minutes: %d  surv_mins:%d  psh:%d\n", i, survey_minutes, PRECISE_SURVEY_HOURS); //plugh
      return i;
   }

   return 0;
}

int add_survey_point()
{
int i;

   if(gps_status != GPS_FIXES) return 0;

   lat_sum += lat;
   lon_sum += lon;
   alt_sum += alt;
   ++precision_samples;
//++survey_tow; //"###
//if(precision_samples > do_median_survey) return 1;  //!!! debug statement

   second_lats = add_to_bin(lat, lat_bins, second_lats);
   second_lons = add_to_bin(lon, lon_bins, second_lons);
   second_alts = add_to_bin(alt, alt_bins, second_alts);
   if(!zoom_screen) {
      sprintf(out, "sec %2d: %.8f %.8f %.3f     ", second_lats, lat*RAD_TO_DEG, lon*RAD_TO_DEG, alt);
      vidstr(MOUSE_ROW+0, MOUSE_COL, SURVEY_COLOR, out);
   }

   while(survey_tow < start_tow) {
      survey_tow += (7L*24L*60L*60L);
   }
//if(debug_file) fprintf(debug_file, "asp surv_tow:%d  start_tow:%d  min_tow:%d  psh:%d\n", survey_tow,start_tow,minute_tow, PRECISE_SURVEY_HOURS);  //plugh

   if(survey_tow >= minute_tow) {  // we have 60 seconds of data
      ++survey_minutes;
      i = analyze_seconds();
//if(debug_file) fprintf(debug_file, "analyze_seconds: %d  surv_mins:%d  psh:%d\n", i, survey_minutes, PRECISE_SURVEY_HOURS); //plugh
      return i;
   }

   return 0;
}

void update_precise_survey()
{
int color;
int plotted;
int i;

   plotted = 0;
   color = precision_samples / (60L*60L);
   color %= 12;

   if(precision_survey || check_precise_posn || show_fixes) {  // plot the points
      if((gps_status == GPS_FIXES) && plot_lla) {  // plot data
         plot_lla_point(1, color+3);
         plotted = 1;
      }
   }
   if(plotted == 0) { // just add point to lla map buffer
      plot_lla_point(0, color+3);
   }

   if(precision_survey) {
      i = add_survey_point();
      if(i) {                      // precision survey has completed
         stop_precision_survey(1); // stop doing the precison survey
         analyze_hours();          // analyze the hourly data
         calc_precise_lla();       // figure up where we are
      }
   }
   else {
      if(show_fixes) ++precision_samples;
   }
}
#endif  // PRECISE_STUFF


//
//
//   MTIE stuff - Maximum Time Interval Error
//
//

struct MTIE_VALS {
   double *min_vals;
   double *max_vals;
};

struct MTIE_TAU {
   struct MTIE_VALS *vals;
};

int pow2[MAX_ADEV_BINS+1];                           // powers of 2
double mtie[MAX_MTIE_CHANS][MAX_ADEV_BINS+1];        // calculated MTIE values for each channel
double *mtie_buf[MAX_MTIE_CHANS];                    // incomming TIE values
int mtie_q_count[MAX_MTIE_CHANS];                    // number incomming of TIE values saved
int mtie_intervals[MAX_MTIE_CHANS][MAX_ADEV_BINS+1]; // number of intervals acquired for each tau

double mtie_max[MAX_MTIE_CHANS];
double mtie_min[MAX_MTIE_CHANS];
double mtie0[MAX_MTIE_CHANS];

struct MTIE_TAU *mtie_taus[MAX_MTIE_CHANS];

int max_mtie_samples;        // max number of samples we will process (a power of 2)
int mtie_kmax;               // max number of taus we will process
int mtie_q_in;
int mtie_q_out;


void free_mtie_data(int id)
{
int i;

   if(id < 0) return;
   if(id > MAX_MTIE_CHANS) return;
   if(mtie_taus[id] == 0) return;

   for(i=0; i<mtie_kmax; i++) {
      if(mtie_taus[id][i].vals) {
         if(mtie_taus[id][i].vals->min_vals) free(mtie_taus[id][i].vals->min_vals);
         if(mtie_taus[id][i].vals->max_vals) free(mtie_taus[id][i].vals->max_vals);
      }
   }

   for(i=0; i<mtie_kmax; i++) {
      if(mtie_taus[id][i].vals) {
         free(mtie_taus[id][i].vals);
      }
   }

   free(mtie_taus[id]);
   mtie_taus[id] = 0;

   if(mtie_buf[id]) {
      free(mtie_buf[id]);
      mtie_buf[id] = 0;
   }

   mtie_q_count[id] = 0;
   mtie_q_in = mtie_q_out = 0;
}


int alloc_mtie_data(int id, int mtie_size)
{
int tau;
int i;

   // alloc mtie tau array, initialize it, and return number of bins
   if(id < 0) return (-10);
   if(id > MAX_MTIE_CHANS) return (-11);

   free_mtie_data(id); // free old info

   tau = 1;
   for(i=0; i<32; i++) {    // calculate powers of 2 table
      pow2[i] = tau;
      tau *= 2;
      mtie_intervals[id][i] = 0;   // init MTIE results table
      mtie[id][i] = (-BIG_NUM);  // init MTIE results table
      mtie0[id] = (-BIG_NUM);
      mtie_max[id] = (-BIG_NUM);
      mtie_min[id] = (BIG_NUM);
   }

   tau = 1;
   mtie_kmax = 0;
   while(1) { // calculate mtie bin count
      if(tau > mtie_size) break;
if(0 && debug_file) fprintf(debug_file, "MTIE_TAU %d: %d\n", mtie_kmax, tau);
      tau *= 2;
      ++mtie_kmax;
   }
   max_mtie_samples = tau/2;
if(0 && debug_file) fprintf(debug_file, "MAX_MTIE_SAMPLES: %d\n", max_mtie_samples);

   mtie_buf[id] = (double *) calloc(max_mtie_samples, sizeof(double));
   if(mtie_buf[id] == 0) {
      sprintf(out, "Could not allocate MTIE data buffer");
      error_exit(50, out);
      return 0;
   }

   if(mtie_kmax) {  // allocate mtie bins
      mtie_taus[id] = (struct MTIE_TAU *) calloc(mtie_kmax, sizeof(struct MTIE_TAU));
   }
   if(mtie_taus[id] == 0) {
      sprintf(out, "Could not allocate MTIE data arrays");
      error_exit(50, out);
      return 0;
   }

   tau = 1;
   i = 0;
   while(i < mtie_kmax) { // allocate and initialize mtie bins
//    mtie_taus[id][i].tau = tau;
if(0 && debug_file) fprintf(debug_file, "MTIE_TAU[%d] %d\n", i, tau);
//    mtie_taus[id][i].val = 0.0;
      mtie_taus[id][i].vals = (struct MTIE_VALS *) calloc(mtie_kmax, sizeof(struct MTIE_VALS));
      if(mtie_taus[id][i].vals == 0) {
         sprintf(out, "Could not allocate MTIE value arrays");
         error_exit(50, out);
         return -1;
      }

      mtie_taus[id][i].vals->min_vals = (double *) calloc(max_mtie_samples, sizeof(double));
      if(mtie_taus[id][i].vals->min_vals == 0) {
         sprintf(out, "Could not allocate MTIE min value arrays");
         error_exit(50, out);
         return (-2);
      }

      mtie_taus[id][i].vals->max_vals = (double *) calloc(max_mtie_samples, sizeof(double));
      if(mtie_taus[id][i].vals->max_vals == 0) {
         sprintf(out, "Could not allocate MTIE max value arrays");
         error_exit(50, out);
         return (-2);
      }

      tau *= 2;
      ++i;
   }

   return mtie_kmax;
}

void alloc_mtie(int why)
{
   alloc_mtie_data(CHA_MTIE, adev_q_size);
   alloc_mtie_data(CHB_MTIE, adev_q_size);
   alloc_mtie_data(CHC_MTIE, adev_q_size);
   alloc_mtie_data(CHD_MTIE, adev_q_size);
   mtie_allocated = 1;
}

void free_mtie()
{
   free_mtie_data(CHA_MTIE);
   free_mtie_data(CHB_MTIE);
   free_mtie_data(CHC_MTIE);
   free_mtie_data(CHD_MTIE);
   mtie_allocated = 0;
}


#define MAX(a,b) (((a) > (b)) ? (a):(b))
#define MIN(a,b) (((a) < (b)) ? (a):(b))

double mtie_phase(int id, int ndx)
{
   // get a value from the MTIE data buffer
   if(id < 0) return 0.0;
   if(id > MAX_MTIE_CHANS) return 0.0;

   if(ndx < 0) return 0.0;
   if(ndx >= max_mtie_samples) return 0.0;

   return mtie_buf[id][ndx];
}

void save_mtie_phase(int id, double val)
{
   // put a value into the MTIE data buffer

   if(id < 0) return;
   if(id > MAX_MTIE_CHANS) return;
   if(mtie_q_count[id] >= max_mtie_samples) return;

   mtie_buf[id][mtie_q_count[id]] = val;
   ++mtie_q_count[id];
}


void add_mtie_point(int id, double val)
{
int i;
int j;
int k;
int p;
int kidx;
int imax;
struct MTIE_VALS *v;
struct MTIE_VALS *vm;
double x;
//double t0;

   if(id < 0) return;
   if(id > MAX_MTIE_CHANS) return;

   if(mtie_taus[id] == 0) alloc_mtie(20);   // mtie memory not allocated
   else if(mtie_buf[id] == 0) alloc_mtie(30);
   if(mtie_taus[id] == 0) return;   // mtie memory not allocated
   if(mtie_buf[id] == 0) return;

//t0 = GetMsecs();
   if((mtie_q_count[id] == 0) && (val == 0.0)) return;  // if first data point is 0.0, it is probably bogus
   save_mtie_phase(id, val);  // save new data point in the MTIE data buffer

   mtie0[id] = MAX(val, mtie0[id]);   // !!!! need ID
   if(mtie_q_count[id] > 1) {
      mtie_max[id] = MAX(val-mtie_phase(id, mtie_q_count[id]-2), mtie_max[id]);
      mtie_min[id] = MIN(val-mtie_phase(id, mtie_q_count[id]-2), mtie_min[id]);
//    mtie_max[id] = MAX(val, mtie_max[id]);
//    mtie_min[id] = MIN(val, mtie_min[id]);
   }
   mtie[id][0] = MAX(fabs(mtie_max[id]), fabs(mtie_min[id]));
   mtie_intervals[id][0] = mtie_q_count[id];

   for(kidx=0; kidx<mtie_kmax; kidx++) {
      k = kidx + 1;
      imax = mtie_q_count[id] - pow2[k] + 1;
      v = mtie_taus[id][kidx].vals;
      if(1) {  // fast "stop after buffer full mode"
         j = mtie_intervals[id][kidx+1] - 2;
         if(j < 0) j = 0;
      }
      else {  // much slower "circular buffer mode"
         j = 0;
      }

      for(i=j; i<imax; i++) {
         if(k <= 1) {
            v->max_vals[i] = MAX(mtie_phase(id, i), mtie_phase(id, i+1));
            v->min_vals[i] = MIN(mtie_phase(id, i), mtie_phase(id, i+1));
         }
         else {
            vm = mtie_taus[id][kidx-1].vals;
            p = pow2[k-1];

            v->max_vals[i] = MAX(vm->max_vals[i],
                                 vm->max_vals[i+p]);

            v->min_vals[i] = MIN(vm->min_vals[i],
                                 vm->min_vals[i+p]);
         }

         mtie_intervals[id][kidx+1] = i;
         x = v->max_vals[i] - v->min_vals[i];

         if(x > mtie[id][kidx+1]) mtie[id][kidx+1] = x;  // save current mtie value
      }
   }
//t0 = GetMsecs() - t0;
//sprintf(debug_text, "mtie_q_count %d: kmax:%d  %.12f  msecs:%f\n", mtie_q_count[id], mtie_kmax, val, t0);
}


void dump_mtie(int id, FILE *file)
{
int i;
char *s;

   if(id < 0) return;
   if(id > MAX_MTIE_CHANS) return;
   if(file == 0) return;

   if(mtie_q_count[id] == 0) return;

   if     (id == 0) s = "chA";
   else if(id == 1) s = "chB";
   else if(id == 2) s = "chC";
   else if(id == 3) s = "chD";
   else             s = "???";
   
   fprintf(file, "TIE values for %s.  intervals:%d\n", s, mtie_q_count[id]);
   for(i=0; i<mtie_q_count[id]; i++) {
fprintf(file, "i:%-6d tie(ns):%.12f\n", i, mtie_phase(id, i));
   }
   fprintf(file, "\n");
//   fprintf(file, "mtie_min:%.12f  mtie_max:%.12f  mtie0: %.12f\n", mtie_min[id], mtie_max[id], mtie0[id]);
   fprintf(file, "\n");

   fprintf(file, "tau: %-6d   mtie(ns):%.12f   intervals:%d\n", pow2[0], mtie[id][0], mtie_q_count[id]);

   for(i=0; i<mtie_kmax; i++) {
      if(mtie[id][i+1] == (-BIG_NUM)) continue;
      fprintf(file, "tau: %-6d   mtie(ns):%.12f   intervals:%d\n", pow2[i+1], mtie[id][i+1], mtie_intervals[id][i+1]);
   }
   fprintf(file, "\n\n\n");
}


int fetch_mtie_info(int id, struct ADEV_INFO *bins)
{
int i;
int count;
double val;
double period;
double max_val, min_val;

   // copy MTIE info into an ADEV_INFO type array for display

   max_val = (-(BIG_NUM));
   min_val = (BIG_NUM);

   bins->adev_type = PPS_ADEV;
   for(i=0; i<32; i++) {
      bins->adev_taus[i] = (float) 0;
      bins->adev_on[i] = (long) 0;
      bins->adev_bins[i] = (float) 0;
      bins->bin_count = 0;
      bins->adev_min = (float) min_val;
      bins->adev_max = (float) max_val;
   }

   if(id < 0) return 0;
   if(id > MAX_MTIE_CHANS) return 0;

   period = 1.0;
   if(id == CHA_MTIE) {
      bins->adev_type = A_MTIE;   //PPS_ADEV;
      period = pps_adev_period;
   }
   else if(id == CHB_MTIE) {
      bins->adev_type = B_MTIE;   //OSC_ADEV;
      period = osc_adev_period;
   }
   else if(id == CHC_MTIE) {
      bins->adev_type = C_MTIE;   //CHC_ADEV;
      period = chc_adev_period;
   }
   else if(id == CHD_MTIE) {
      bins->adev_type = D_MTIE;   //CHD_ADEV;
      period = chd_adev_period;
   }
   else return 0;

   count = 0;
   for(i=0; i<32; i++) {
      if(mtie[id][i] == (-BIG_NUM)) break;
      val = (float) (mtie[id][i] / 1.0E9);
if((i == 0) && (mtie[id][0] > mtie[id][1])) val = (float) (mtie[id][1] / 1.0E9); 

      ++count;
      bins->bin_count = count;
      bins->adev_taus[i] = ((float) pow2[i]) * (float) period;
      bins->adev_on[i] = mtie_intervals[id][i];
if(bins->adev_on[i] == 0) bins->adev_on[i] = 1;
      bins->adev_bins[i] = (float) val;
      if(val > max_val) max_val = val;
      if(val < min_val) min_val = val;
   }
   bins->adev_min = (float) min_val;
   bins->adev_max = (float) max_val;

   return count;
}

void scan_mtie_bins(int id)
{
int i;
double val;

   // find min/max values in the MTIE data

   if(id < 0) return;
   if(id > MAX_MTIE_CHANS) return;

   for(i=first_show_bin; i<32; i++) {  // !!!!! should start at
      if(mtie[id][i] == (-BIG_NUM)) continue;
      val = (mtie[id][i] / 1.0E9);

      if(val > global_adev_max) global_adev_max = val;
      if(val < global_adev_min) global_adev_min = val;
   }
}




//
//
//   Allan deviation stuff
//
//


int adevs_active(int check_enable)
{
int i;
static int last_ticc_i = 0;

   // NEW_RCVR

   // return true if we might have adevs to display
   //   0x01 : pps_offset available
   //   0x02 : osc_offset available
   //   0x04 : TICC channel C available
   //   0x08 : TICC channel D available
   //   0x80 : flags that bits 0x01 and 0x02 are guessed

   if(no_adev_flag) return 0;

   if(check_enable) {
      if     (pps_adev_period > 0.0) ;        // aaaallll
      else if(osc_adev_period > 0.0) ;
      else if(chc_adev_period > 0.0) ;
      else if(chd_adev_period > 0.0) ;
      else if(adev_period <= 0.0) {
         return 0;
      }
   }

   if(jitter_adev) return 0x03;

   i = 0;
   if(rcvr_type == TICC_RCVR) {
      if(have_pps_offset) i |= 0x01;
      if(have_osc_offset) i |= 0x02;
      if(have_chc_offset) i |= 0x04;
      if(have_chd_offset) i |= 0x08;
      if(mtie_q_count[CHA_MTIE]) i |= 0x01;
      if(mtie_q_count[CHB_MTIE]) i |= 0x02;
      if(mtie_q_count[CHC_MTIE]) i |= 0x04;
      if(mtie_q_count[CHD_MTIE]) i |= 0x08;
      if(sim_file && (i == 0)) return last_ticc_i;
      last_ticc_i = i;
      return i;
   }
   else if(TICC_USED) {
      if(ticc_mode == 'I') return 0x01;   // time interval
      if(ticc_mode == 'F') return 0x01;   // frequency counter
      return 0x03;   // aaaacccc
   }
   else {
      if(luxor) return 0;
      if(NO_ADEV_INFO) return 0;
   }

   if(rcvr_type == PRS_RCVR) ;   // SRO_RCVR?  LPFRS_RCVR  SA35_RCVR?
   else if(GPSDO) ;
   else if(!TIMING_RCVR) return 0;
///   else if(TIMING_RCVR) return 0;

   if(rcvr_type == BRANDY_RCVR) {
      if(have_pps_offset) return 0x01;
      else                return 0x01;
   }

   if(have_pps_offset && have_osc_offset) return 0x03;
   else if(have_pps_offset) return 0x01;
   else if(have_osc_offset) return 0x02;

   return 0x83;
}


#ifdef ADEV_STUFF

//
// Get next tau to calculate. Some tools calculate Allan deviation
// for only one point per decade; others two or three. Below are
// several unique alternatives that produce cleaner-looking plots.
//

void calc_adev_display_size()
{
   adev_bottom_row  = PLOT_TEXT_ROW - 1;
   adev_bottom_row -= (TEXT_Y_MARGIN+TEXT_HEIGHT-1)/TEXT_HEIGHT;
   if(extra_plots) adev_bottom_row -= 2;
   if(all_adevs && (adev_bottom_row >= MOUSE_ROW+1)) adev_bottom_row = MOUSE_ROW+1; // mmmmmm 

   if(all_adevs) adev_bottom_row = MOUSE_ROW+1;
   else          adev_bottom_row = MOUSE_ROW+2+1;  // mmmmmm
}

long next_tau(long tau, int bin_scale)
{
long pow10;
long n;

    switch (bin_scale) {
       case 0 :    // all tau (not practical)
           return tau + 1;

       case 1 :    // one per decade
           return tau * 10;

       case 2 :    // one per octave  (1-2-4-8-16...)
           return tau * 2;

       case 3 :    // 3 dB
       case 4 :    // 1-2-4 decade
       case 5 :    // 1-2-5 decade
       case 8 :    // 1-2-4-8 decade
       case 10 :   // ten per decade
           space_bins:
           pow10 = 1;
           while(tau >= 10) {  // reduce the decade
               pow10 *= 10;
               tau /= 10;
           }

           if(bin_scale == 3) {
               if(tau == 3) return 10 * pow10;
               else         return 3 * pow10;
           }
           if((bin_scale == 4) && (tau == 4)) {
               return 10 * pow10;
           }
           if((bin_scale == 5) && (tau == 2)) {
               return 5 * pow10;
           }
           if((bin_scale == 8) && (tau == 8)) {
               return 10 * pow10;
           }
           if(bin_scale == 10) {
               return (tau + 1) * pow10;
           }
           return tau * 2 * pow10;

       case 29 :    // 29 nice round numbers per decade
           pow10 = 1;
           while(tau > 100) {  // reduce the decade
               pow10 *= 10;
               tau /= 10;
           }

           if(tau < 22) {
               return (tau + 1) * pow10;
           } 
           else if(tau < 40) {
               return (tau + 2) * pow10;
           } 
           else if(tau < 60) {
               return (tau + 5) * pow10;
           } 
           else {
               return (tau + 10) * pow10;
           }

       case 99 :   // logarithmically evenly spaced divisions
           n = (long) (log10((double) tau) * (double)bin_scale + 0.5) + 1;
           n = (long) pow(10.0, (double)n / (double)bin_scale);
           return (n > tau) ? n : tau + 1;

       default : 
          bin_scale = 5;    // 1-2-5 sequence
          goto space_bins;
    }
}

void reset_incr_bins(struct BIN *bins, double period)
{
int b;
S32 m;
struct BIN *B;

   if(bins == 0) return;

   m = 1L;
   for(b=0; b<MAX_ADEV_BINS; b++) {  // flush adev bin data
      B = &bins[b];
      if(B == 0) continue;

      B->m     = m;
      B->n     = 0;
      B->sum   = 0.0;
      B->value = 0.0;
      B->tau   = ((double) m) * period;
      B->accum = 0.0;
      B->i     = 0;
      B->j     = 0;
      B->init  = 0;

      m = next_tau(m, bin_scale);
   }
}

void reset_pps_bins()
{
   pps_adev_q_overflow = 0.0;
   reset_incr_bins(&pps_adev_bins[0], pps_adev_period);
   reset_incr_bins(&pps_hdev_bins[0], pps_adev_period);
   reset_incr_bins(&pps_mdev_bins[0], pps_adev_period);
   reset_incr_bins(&pps_tdev_bins[0], pps_adev_period);
}

void reset_osc_bins()
{
   osc_adev_q_overflow = 0.0;
   reset_incr_bins(&osc_adev_bins[0], osc_adev_period);
   reset_incr_bins(&osc_hdev_bins[0], osc_adev_period);
   reset_incr_bins(&osc_mdev_bins[0], osc_adev_period);
   reset_incr_bins(&osc_tdev_bins[0], osc_adev_period);
}

void reset_chc_bins()
{
   chc_adev_q_overflow = 0.0;
   reset_incr_bins(&chc_adev_bins[0], chc_adev_period);
   reset_incr_bins(&chc_hdev_bins[0], chc_adev_period);
   reset_incr_bins(&chc_mdev_bins[0], chc_adev_period);
   reset_incr_bins(&chc_tdev_bins[0], chc_adev_period);
}

void reset_chd_bins()
{
   chd_adev_q_overflow = 0.0;
   reset_incr_bins(&chd_adev_bins[0], chd_adev_period);
   reset_incr_bins(&chd_hdev_bins[0], chd_adev_period);
   reset_incr_bins(&chd_mdev_bins[0], chd_adev_period);
   reset_incr_bins(&chd_tdev_bins[0], chd_adev_period);
}

void reset_adev_bins()
{
   // reset the incremental adev bins
   reset_pps_bins();
   reset_osc_bins();
   reset_chc_bins();
   reset_chd_bins();

   // !!!!! This code works for 1-2-5 adev decades.  Should be generalized to
   //       work with any bin density. !!!!!!    aaaahhhh
   if     (adev_q_size <= 10L)       max_adev_rows = 2;
   else if(adev_q_size <= 20L)       max_adev_rows = 3;
   else if(adev_q_size <= 40L)       max_adev_rows = 4;
   else if(adev_q_size <= 100L)      max_adev_rows = 5;
   else if(adev_q_size <= 200L)      max_adev_rows = 6;
   else if(adev_q_size <= 400L)      max_adev_rows = 7;
   else if(adev_q_size <= 1000L)     max_adev_rows = 8;
   else if(adev_q_size <= 2000L)     max_adev_rows = 9; 
   else if(adev_q_size <= 4000L)     max_adev_rows = 10;
   else if(adev_q_size <= 10000L)    max_adev_rows = 11;
   else if(adev_q_size <= 20000L)    max_adev_rows = 12;
   else if(adev_q_size <= 40000L)    max_adev_rows = 13;
   else if(adev_q_size <= 100000L)   max_adev_rows = 14;
   else if(adev_q_size <= 200000L)   max_adev_rows = 15;
   else if(adev_q_size <= 400000L)   max_adev_rows = 16;
   else if(adev_q_size <= 1000000L)  max_adev_rows = 17;
   else if(adev_q_size <= 2000000L)  max_adev_rows = 18;
   else if(adev_q_size <= 4000000L)  max_adev_rows = 19;
   else if(adev_q_size <= 10000000L) max_adev_rows = 20;
   else if(adev_q_size <= 20000000L) max_adev_rows = 21;
   else if(adev_q_size <= 40000000L) max_adev_rows = 22;
   else                              max_adev_rows = 23;

   calc_adev_display_size();
}

float adev_decade(float val)
{
float decade;

    // convert an ADEV value to the decade it is in
    for(decade=1.0F; decade>=1.0E-19F; decade/=10.0F) {
       if(val > decade) break;
    }
    return decade*10.0F;
}


float scale_adev(float val)
{
double check;
double mant;
double expo;
double v;

   // convert adev value into an decade_exponent.mantissa value
   // Warning: due to subtle rounding/truncation issues,  change
   //          anything in this routine at your own risk...
if(ATYPE == A_MTIE) return val;

   if(val > 1.0e0F) {
      val = (float) (0.0F);
      return val;
   }
   else if(val > 1.0e-1F) {
      val = (float) ((-1.0F) + (val/1.0e-0F));
   }
   else if(val > 1.0e-2F) {
      val = (float) ((-2.0F) + (val/1.0e-1F));
   }
   else if(val > 1.0e-3F) {
      val = (float) ((-3.0F) + (val/1.0e-2F));
   }
   else if(val > 1.0e-4F) {
      val = (float) ((-4.0F) + (val/1.0e-3F));
   }
   else if(val > 1.0e-5F) {
      val = (float) ((-5.0F) + (val/1.0e-4F));
   }
   else if(val > 1.0e-6F) {
      val = (float) ((-6.0F) + (val/1.0e-5F));
   }
   else if(val > 1.0e-7F) {
      val = (float) ((-7.0F) + (val/1.0e-6F));
   }
   else if(val > 1.0e-8F) {
      val = (float) ((-8.0F) + (val/1.0e-7F));
   }
   else if(val > 1.0e-9F) {
      val = (float) ((-9.0F) + (val/1.0e-8F));
   }
   else if(val > 1.0e-10F) {
      val = (float) ((-10.0F) + (val/1.0e-9F));
   }
   else if(val > 1.0e-11F) {
      val = (float) ((-11.0F) + (val/1.0e-10F));
   }
   else if(val > 1.0e-12F) {
      val = (float) ((-12.0F) + (val/1.0e-11F));
   }
   else if(val > 1.0e-13F) {
      val = (float) ((-13.0F) + (val/1.0e-12F));
   }
   else if(val > 1.0e-14F) {
      val = (float) ((-14.0F) + (val/1.0e-13F));
   }
   else if(val > 1.0e-15F) {
      val = (float) ((-15.0F) + (val/1.0e-14F));
   }
   else if(val > 1.0e-16F) {
      val = (float) ((-16.0F) + (val/1.0e-15F));
   }
   else if(val > 1.0e-17F) {
      val = (float) ((-17.0F) + (val/1.0e-16F));
   }
   else if(val > 1.0e-18F) {
      val = (float) ((-18.0F) + (val/1.0e-17F));
   }
   else if(val > 1.0e-19F) {
      val = (float) ((-19.0F) + (val/1.0e-18F));
   }
   else if(val > 1.0e-20F) {
      val = (float) ((-20.0F) + (val/1.0e-19F));
   }
   else if(val > 1.0e-21F) {
      val = (float) ((-21.0F) + (val/1.0e-20F));
   }
   else if(val > 1.0e-22F) {
      val = (float) ((-22.0F) + (val/1.0e-21F));
   }
   else if(val > 1.0e-23F) {
      val = (float) ((-23.0F) + (val/1.0e-22F));
   }
   else if(val > 1.0e-24F) {
      val = (float) ((-24.0F) + (val/1.0e-23F));
   }
   else if(val > 1.0e-25F) {
      val = (float) ((-25.0F) + (val/1.0e-24F));
   }
   else if(val > 1.0e-26F) {
      val = (float) ((-26.0F) + (val/1.0e-25F));
   }
   else if(val > 1.0e-27F) {
      val = (float) ((-27.0F) + (val/1.0e-26F));
   }
   else if(val > 1.0e-28F) {
      val = (float) ((-28.0F) + (val/1.0e-27F));
   }
   else if(val > 1.0e-29F) {
      val = (float) ((-29.0F) + (val/1.0e-28F));
   }
   else val = (float) (-30.0F);

   return val;

   // this code has the rounding issues mentioned above...
   check = 1.0e-0;
   mant = (-1.0);
   expo = 1.0e+1;

   v = val;
   while(mant > (-30.0)) {
      if(v > check) {
         v = (mant + (val/expo));
         return (float) v;
      }
      check /= 10.0;
      mant -= 1.0;
      expo /= 10.0;
   }

   return (float) (-30.0);
}


void label_adev_grid(int top)
{
int row, col;
int row_step;
int top_row;
int i,j;
int k;
double val;
double range;
double steps;
int adev_top_row;   // calculate where the adev decades are in the plot area
int vert_scale;
int vert_ofs;
long m;

   if(all_adevs == SINGLE_ADEVS) {
      if(plot_adev_data == 0) return;   // no adevs on screen, don't label the plots
   }
   if(plot_adev_data == 0) return;   // no adevs on screen, don't label the plots

   graphics_coords = 1;

   m = 1L;
   for(i=0; i<first_show_bin; i++) {  // find tau of the first displayed bin
      if(ATYPE == A_MTIE) m *= 2;
      else                m = next_tau(m, bin_scale);
   }

   i = k = 0;
   adev_decades_shown = 0;
   for(col=0; col<PLOT_WIDTH; col+=HORIZ_MAJOR) {  // aaaaaapppppp
//      sprintf(out, "%ld", m);
      sprintf(out, "%g", adev_period * (double) m);

      if(col == 0) j = 0;
      else         j = (strlen(out)*TEXT_WIDTH) / 2;

      if((col-j) > (PLOT_WIDTH-(HORIZ_MAJOR*2))) break;
      else if((HORIZ_MAJOR < (6*TEXT_WIDTH)) && (k != 0)) ;
      else {
         vidstr(PLOT_ROW+2, 1+PLOT_COL+col-j, ADEV_LABEL_COLOR, out); // xyzzy
      }
      ++adev_decades_shown;

      k = (k + 1) % 3;
      if(ATYPE == A_MTIE) m *= 2;
      else                m = next_tau(m, bin_scale);
      if(m > 100000) break;
   }

   range = 0.0;
   val = 1.0;
   for(i=0; i<20; i++) {  // find decade value of the largest xDEV value
      if(global_adev_max >= val) break;
      val /= 10.0;
   }
   val *= 10.0;

   top_row = PLOT_ROW; // top;
   vert_scale = (PLOT_HEIGHT / (VERT_MAJOR*2)) * (VERT_MAJOR*2);  // two major divisions per decade
   vert_ofs = (PLOT_HEIGHT - vert_scale) / 2;
   if(((PLOT_HEIGHT / (VERT_MAJOR*2)) & 0x01) == 0x00) {  // align adevs to cyan dotted lines
      vert_ofs += VERT_MAJOR;
   }
   adev_top_row = PLOT_ROW + vert_ofs;
   if(adev_top_row && (adev_top_row > top_row)) top_row = adev_top_row;
//sprintf(debug_text, "plot:%d  top:%d  hmajor:%d  verts:%d  atr:%d  gam:%g  val:%g", PLOT_ROW, top, HORIZ_MAJOR, PLOT_HEIGHT/VERT_MAJOR, adev_top_row, global_adev_max, val);

   row_step = VERT_MAJOR;
   if(SCREEN_HEIGHT > SHORT_SCREEN) {
      row_step *= 2;
   }
   else if(zoom_screen == 'P') {  // aaaahhhh
      row_step *= 2;
   }
   adev_decade_height = (double) row_step;

   if(ATYPE == A_MTIE) {  // calculate vertical scale
      steps = 0.0;
      for(row=top_row; row<SCREEN_HEIGHT; row+=row_step) {
         steps += 1.0;
      }
      val = global_adev_max * MTIE_SCALE;
      range = val - (global_adev_min / MTIE_SCALE);
      if(steps) range /= steps;
   //sprintf(debug_text, "steps:%g", steps);
   }

   for(row=top_row; row<SCREEN_HEIGHT; row+=row_step) {
      if(ATYPE == A_MTIE) {
         sprintf(out, "%.3g", val);   // aaahhh %le
         val -= range;
      }
      else {
         sprintf(out, "%g", val);   // aaahhh %le
         val /= 10.0;
      }

      i = (strlen(out)+1) * TEXT_WIDTH;
      if((row == top_row) && (PLOT_ROW == adev_top_row)) {
         vidstr(row+2, PLOT_COL+PLOT_WIDTH-i, ADEV_LABEL_COLOR, out);
      }
      else {
         vidstr(row-TEXT_HEIGHT/2, PLOT_COL+PLOT_WIDTH-i, ADEV_LABEL_COLOR, out);
      }
   }

   bottom_adev_decade = (float) val;
   graphics_coords = 0;
}


void adev_mouse()
{
u08 old_disable_kbd;

// return;    // this routine breaks "keep_adevs_fresh"  unless timer_serve is set when calling get_mouse_info()

   // keep mouse lively during long periods of thinking
   if((++adev_mouse_time & 0xFFFF) != 0x0000) return;

   update_pwm();
   if(mouse_shown == 0) return;

   old_disable_kbd = disable_kbd;
   if(kbd_flag) disable_kbd = 2; // (so that do_kbd() won't do anything when it's called by WM_CHAR during get_pending_gps())

   ++timer_serve;
   get_mouse_info();
   --timer_serve;

   disable_kbd = old_disable_kbd;
}


void do_incr_pps_adevs()
{
   // update each of the PPS adev bins with the latest queue data
   incr_adev(PPS_ADEV, &pps_adev_bins[0]);
   incr_hdev(PPS_HDEV, &pps_hdev_bins[0]);
   incr_mdev(PPS_MDEV, &pps_mdev_bins[0]);
   incr_tdev(PPS_TDEV, &pps_tdev_bins[0]);
}

void do_incr_osc_adevs()
{
   // update each of the OSC adev bins with the latest queue data
   incr_adev(OSC_ADEV, &osc_adev_bins[0]);
   incr_hdev(OSC_HDEV, &osc_hdev_bins[0]);
   incr_mdev(OSC_MDEV, &osc_mdev_bins[0]);
   incr_tdev(OSC_TDEV, &osc_tdev_bins[0]);
}

void do_incr_chc_adevs()
{
   // update each of the CHC adev bins with the latest queue data
   incr_adev(CHC_ADEV, &chc_adev_bins[0]);
   incr_hdev(CHC_HDEV, &chc_hdev_bins[0]);
   incr_mdev(CHC_MDEV, &chc_mdev_bins[0]);
   incr_tdev(CHC_TDEV, &chc_tdev_bins[0]);
}

void do_incr_chd_adevs()
{
   // update each of the CHD adev bins with the latest queue data
   incr_adev(CHD_ADEV, &chd_adev_bins[0]);
   incr_hdev(CHD_HDEV, &chd_hdev_bins[0]);
   incr_mdev(CHD_MDEV, &chd_mdev_bins[0]);
   incr_tdev(CHD_TDEV, &chd_tdev_bins[0]);
}

void recalc_adev_info()
{
double msec, msec2;
msec = GetMsecs();
   // recalculate all the adevs from scratch
   reset_adev_bins();    // reset the bins

   extend_com_timeout(ADEV_TIMEOUT); // aaaahhhh
   do_incr_pps_adevs();  // recalculate all the adevs from the queued data
   do_incr_osc_adevs();
   do_incr_chc_adevs();
   do_incr_chd_adevs();
msec2 = GetMsecs();        // aaaahhhh
//sprintf(debug_text2, "recalc adev msecs:%f %f %f", msec,msec2,msec2-msec);
}

void incr_pps_overflow()
{
int b;

   // tweek bin data counts when the PPS/chA adev queue is full
   pps_adev_q_overflow += 1.0;

   for(b=0; b<n_bins; b++) {  
      pps_adev_bins[b].n--; 
      pps_adev_bins[b].j--;
      pps_hdev_bins[b].n--;
      pps_hdev_bins[b].j--;
      pps_mdev_bins[b].n--;
      pps_mdev_bins[b].j--;
      pps_tdev_bins[b].n--;
      pps_tdev_bins[b].j--;
   }
}

void incr_osc_overflow()
{
int b;

   // tweek bin data counts when the OSC/chB adev queue is full
   osc_adev_q_overflow += 1.0;

   for(b=0; b<n_bins; b++) {  
      osc_adev_bins[b].n--;
      osc_adev_bins[b].j--;
      osc_hdev_bins[b].n--;
      osc_hdev_bins[b].j--;
      osc_mdev_bins[b].n--;
      osc_mdev_bins[b].j--;
      osc_tdev_bins[b].n--;
      osc_tdev_bins[b].j--;
   }
}

void incr_chc_overflow()
{
int b;

   // tweek bin data counts when the chC adev queue is full
   chc_adev_q_overflow += 1.0;

   for(b=0; b<n_bins; b++) {  
      chc_adev_bins[b].n--;
      chc_adev_bins[b].j--;
      chc_hdev_bins[b].n--;
      chc_hdev_bins[b].j--;
      chc_mdev_bins[b].n--;
      chc_mdev_bins[b].j--;
      chc_tdev_bins[b].n--;
      chc_tdev_bins[b].j--;
   }
}

void incr_chd_overflow()
{
int b;

   // tweek bin data counts when the chD adev queue is full
   chd_adev_q_overflow += 1.0;

   for(b=0; b<n_bins; b++) {  
      chd_adev_bins[b].n--;
      chd_adev_bins[b].j--;
      chd_hdev_bins[b].n--;
      chd_hdev_bins[b].j--;
      chd_mdev_bins[b].n--;
      chd_mdev_bins[b].j--;
      chd_tdev_bins[b].n--;
      chd_tdev_bins[b].j--;
   }
}

void calc_resids(int plot, double resid_val, double period)
{
double x,y;

   if(plot < 0) return;
   if(plot >= (NUM_PLOTS+DERIVED_PLOTS)) return;

   resid[plot].resid_count += 1.0;
   y = resid_val;
   x = resid[plot].resid_count * period;

   resid[plot].sum_x  += x;
   resid[plot].sum_y  += y;
   resid[plot].sum_xx += (x*x);
   resid[plot].sum_yy += (y*y);
   resid[plot].sum_xy += (x*y);
//   resid[plot].a0 = 
}


void add_pps_adev_point(double val, int phase)
{
   if(pps_adev_period <= 0.0) return;
   if(adev_q_allocated == 0) {
      alloc_adev();
   }

if(rcvr_type == BRANDY_RCVR) phase = 1;  // receiver outputs phase instead of time for the pps_offset
if(rcvr_type == RFTG_RCVR) phase = 1;
   if(phase) pps_phase = val;
   else      pps_phase += val;  // convert time interval to phase

   if(pps_adev_q_count == 0) {
      pps_resid = 0.0;
      pps_adev_jd0 = jd_utc;
   }
   else pps_resid = (pps_phase - last_pps_resid);
   last_pps_resid = pps_phase;

   calc_resids(PPS, pps_phase, pps_adev_period);

//sprintf(debug_text, "phase:%.12f  last:%.12f  diff:%.12f %.12f", pps_phase,last_pps_phase, (pps_phase-last_pps_phase), (pps_phase-last_pps_phase)/1.0E9);

   if((have_pps_base == 0) && (subtract_base_value == 1) && (pps_adev_q_count <= 1)) {  // aaaaa
      pps_base_value = pps_phase;
      have_pps_base = 1;
   }

   if(pps_adev_q) {
      pps_adev_q[pps_adev_q_in] = (OFS_SIZE) (pps_phase - pps_base_value);
   }

   if(++pps_adev_q_in >= adev_q_size) {  // queue has wrapped
      pps_adev_q_in = 0;
   }

   if(pps_adev_q_in == pps_adev_q_out) {  // queue is full
      ++pps_adev_q_out;           // drop oldest entry from the queue
      pps_adev_q_overflow += 1.0;
      incr_pps_overflow();        // tweek counts in the adev bins

      // Once the adev queue fills up,  the adev results begin to get stale
      // because the incremental adevs are based upon all the points seen in
      // the past.  To keep the adev numbers fresh, we preiodically reset
      // the incremental adev bins.  This causes the adevs to be
      // recalculated from just the values stored in the queue.
      if(pps_adev_q_overflow >= adev_q_size) {  // !!!! wrap >= was >0
         if(keep_adevs_fresh) {
            reset_pps_bins();              
            adev_freshened = 1;
         }
      }
   }
   else ++pps_adev_q_count;   // keep count of number of entries in the adev queue
   if(pps_adev_q_out >= adev_q_size) pps_adev_q_out = 0;

   // incrementally update the adev bin values with the new data point
   do_incr_pps_adevs();    // recalculate all the adevs from the queued data

   if(adev_freshened) {
      show_adev_info(1);
      adev_freshened = 0;
   }
}

void add_osc_adev_point(double val, int phase)
{
   if(osc_adev_period <= 0.0) return;
   if(adev_q_allocated == 0) {
      alloc_adev();
   }

   if(phase) osc_phase = val;
   else      osc_phase += val;  // convert time interval to phase

   if(osc_adev_q_count == 0) {
      osc_resid = 0.0;
      osc_adev_jd0 = jd_utc;
   }
   else osc_resid = ((val) - last_osc_resid);
   last_osc_resid = val;

   calc_resids(OSC, osc_phase, osc_adev_period);

   if((have_osc_base == 0) && (subtract_base_value == 1) && (osc_adev_q_count <= 1)) {  // aaaaa
      osc_base_value = osc_phase;
      have_osc_base = 1;
   }

   if(osc_adev_q) osc_adev_q[osc_adev_q_in] = (OFS_SIZE) (osc_phase - osc_base_value);

   if(++osc_adev_q_in >= adev_q_size) {  // queue has wrapped
      osc_adev_q_in = 0;
   }

   if(osc_adev_q_in == osc_adev_q_out) {  // queue is full
      ++osc_adev_q_out;               // drop oldest entry from the queue
      osc_adev_q_overflow += 1.0;
      incr_osc_overflow();        // tweek counts in the adev bins

      // Once the adev queue fills up,  the adev results begin to get stale
      // because the incremental adevs are based upon all the points seen in
      // the past.  To keep the adev numbers fresh, we preiodically reset
      // the incremental adev bins.  This causes the adevs to be
      // recalculated from just the values stored in the queue.
      if(osc_adev_q_overflow >= adev_q_size) {  // !!!! wrap >= was >0 
         if(keep_adevs_fresh) {
            reset_osc_bins();              
            adev_freshened = 1;
         }
      }
   }
   else ++osc_adev_q_count;   // keep count of number of entries in the adev queue
   if(osc_adev_q_out >= adev_q_size) osc_adev_q_out = 0;

   // incrementally update the adev bin values with the new data point
   do_incr_osc_adevs();    // recalculate all the adevs from the queued data

   if(adev_freshened) {
      show_adev_info(2);
      adev_freshened = 0;
   }
}

void add_chc_adev_point(double val, int phase)
{
   if(chc_adev_period <= 0.0) return;
   if(adev_q_allocated == 0) {
      alloc_adev();
   }

   if(phase) chc_phase = val;
   else      chc_phase += val;  // convert time interval to phase

   if(chc_adev_q_count == 0) {
      chc_resid = 0.0;
      chc_adev_jd0 = jd_utc;
   }
   else chc_resid = (float) ((val-last_chc_resid));
   last_chc_resid = val;

   calc_resids(CHC, chc_phase, chc_adev_period);

   if((have_chc_base == 0) && (subtract_base_value == 1) && (chc_adev_q_count <= 1)) {  // aaaaa
      chc_base_value = chc_phase;
      have_chc_base = 1;
   }

   if(chc_adev_q) chc_adev_q[chc_adev_q_in] = (OFS_SIZE) (chc_phase - chc_base_value);

   if(++chc_adev_q_in >= adev_q_size) {  // queue has wrapped
      chc_adev_q_in = 0;
   }

   if(chc_adev_q_in == chc_adev_q_out) {  // queue is full
      ++chc_adev_q_out;               // drop oldest entry from the queue
      chc_adev_q_overflow += 1.0;
      incr_chc_overflow();        // tweek counts in the adev bins

      // Once the adev queue fills up,  the adev results begin to get stale
      // because the incremental adevs are based upon all the points seen in
      // the past.  To keep the adev numbers fresh, we preiodically reset
      // the incremental adev bins.  This causes the adevs to be
      // recalculated from just the values stored in the queue.
      if(chc_adev_q_overflow >= adev_q_size) {  // !!!! wrap >= was >0 
         if(keep_adevs_fresh) {
            reset_chc_bins();              
            adev_freshened = 1;
         }
      }
   }
   else ++chc_adev_q_count;   // keep count of number of entries in the adev queue
   if(chc_adev_q_out >= adev_q_size) chc_adev_q_out = 0;

   // incrementally update the adev bin values with the new data point
   do_incr_chc_adevs();    // recalculate all the adevs from the queued data

   if(adev_freshened) {
      show_adev_info(3);
      adev_freshened = 0;
   }
}

void add_chd_adev_point(double val, int phase)
{
   if(chd_adev_period <= 0.0) return;
   if(adev_q_allocated == 0) {
      alloc_adev();
   }

   if(phase) chd_phase = val;
   else      chd_phase += val;  // convert time interval to phase

   if(chd_adev_q_count == 0) {
      chd_resid = 0.0;
      chd_adev_jd0 = jd_utc;
   }
   else chd_resid = (float) ((val-last_chd_resid));
   last_chd_resid = val;

   calc_resids(CHD, chd_phase, chd_adev_period);

   if((have_chd_base == 0) && (subtract_base_value == 1) && (chd_adev_q_count <= 1)) {  // aaaaa
      chd_base_value = chd_phase;
      have_chd_base = 1;
   }

   if(chd_adev_q) chd_adev_q[chd_adev_q_in] = (OFS_SIZE) (chd_phase - chd_base_value);

   if(++chd_adev_q_in >= adev_q_size) {  // queue has wrapped
      chd_adev_q_in = 0;
   }

   if(chd_adev_q_in == chd_adev_q_out) {  // queue is full
      ++chd_adev_q_out;               // drop oldest entry from the queue
      chd_adev_q_overflow += 1.0;
      incr_chd_overflow();        // tweek counts in the adev bins

      // Once the adev queue fills up,  the adev results begin to get stale
      // because the incremental adevs are based upon all the points seen in
      // the past.  To keep the adev numbers fresh, we preiodically reset
      // the incremental adev bins.  This causes the adevs to be
      // recalculated from just the values stored in the queue.
      if(chd_adev_q_overflow >= adev_q_size) {  // !!!! wrap >= was >0 
         if(keep_adevs_fresh) {
            reset_chd_bins();              
            adev_freshened = 1;
         }
      }
   }
   else ++chd_adev_q_count;   // keep count of number of entries in the adev queue
   if(chd_adev_q_out >= adev_q_size) chd_adev_q_out = 0;

   // incrementally update the adev bin values with the new data point
   do_incr_chd_adevs();    // recalculate all the adevs from the queued data

   if(adev_freshened) {
      show_adev_info(4);
      adev_freshened = 0;
   }
}


void add_adev_point(double osc_phase, double pps_phase, double chc_phase, double chd_phase)
{
   if(adev_period <= 0) return;
   if(adev_q_allocated == 0) {
      alloc_adev();
   }

   add_pps_adev_point(pps_phase, 0);
   add_osc_adev_point(osc_phase, 0);
   add_chc_adev_point(chc_phase, 0);
   add_chd_adev_point(chd_phase, 0);
   return;
}


double get_adev_point(u08 id, long i)
{
double val;
#define INTERVAL ((OFS_SIZE) 0.0)  //1.0

   if(adev_q_allocated == 0) return 0.0;

   id /= NUM_ADEV_TYPES;

   val = 0;
   if(id == PPS_ID) {   // return PPS value
      i = pps_adev_q_out + i;
//      if(i >= adev_q_size) i -= adev_q_size;
      while(i >= adev_q_size) i -= adev_q_size;
      if(i < 0) return 0.0;
      if(pps_adev_q == 0) return 0.0;
      val = pps_adev_q[i];
      if(luxor) val = INTERVAL + (val+pps_base_value);
      else      val = INTERVAL + (1.0e-9 * (val+pps_base_value));
   }
   else if(id == CHC_ID) {   // return CHC value
      i = chc_adev_q_out + i;
//      if(i >= adev_q_size) i -= adev_q_size;
      while(i >= adev_q_size) i -= adev_q_size;
      if(i < 0) return 0.0;
      if(chc_adev_q == 0) return 0.0;
      val = chc_adev_q[i];
      if(luxor) val = INTERVAL + (val+chc_base_value);
      else      val = INTERVAL + (1.0e-9 * (val+chc_base_value));
   }
   else if(id == CHD_ID) {   // return CHD value
      i = chd_adev_q_out + i;
//      if(i >= adev_q_size) i -= adev_q_size;
      while(i >= adev_q_size) i -= adev_q_size;
      if(i < 0) return 0.0;
      if(chd_adev_q == 0) return 0.0;
      val = chd_adev_q[i];
      if(luxor) val = INTERVAL + (val+chd_base_value);
      else      val = INTERVAL + (1.0e-9 * (val+chd_base_value));
   }
   else {      // return OSC value
      i = osc_adev_q_out + i;
//      if(i >= adev_q_size) i -= adev_q_size;
      while(i >= adev_q_size) i -= adev_q_size;
      if(i < 0) return 0.0;
      if(osc_adev_q == 0) return 0.0;
      val = osc_adev_q[i];
      if(luxor) val = INTERVAL + (val+osc_base_value);
      else if(TICC_USED) val = INTERVAL + (1.0e-9 * (val+osc_base_value));
      else if(0) val = INTERVAL + (1.0e-9 * (val+osc_base_value));  // aaattttt
      else val = INTERVAL + ((100.0 * 1.0e-9) * (val+osc_base_value));
   }

   return val;
}

//
// For given sample interval (tau) compute the Allan deviation.
// Nole: all Allan deviations are of the overlapping type.
//

void incr_adev(u08 id, struct BIN *bins)
{
S32 b;
S32 t1,t2;
struct BIN *B;
double v;
int vis_bins;
long adev_q_count;
double adev_q_overflow;

   if     (id == PPS_ADEV) { adev_q_count = pps_adev_q_count; adev_q_overflow = pps_adev_q_overflow; }
   else if(id == OSC_ADEV) { adev_q_count = osc_adev_q_count; adev_q_overflow = osc_adev_q_overflow; } 
   else if(id == CHC_ADEV) { adev_q_count = chc_adev_q_count; adev_q_overflow = chc_adev_q_overflow; } 
   else if(id == CHD_ADEV) { adev_q_count = chd_adev_q_count; adev_q_overflow = chd_adev_q_overflow; } 
   else return;

   vis_bins = 0;

   for(b=0; b<n_bins; b++) {
      B = &bins[b];
      if(B->n < 0) break;

      t1 = B->m;
      t2 = t1 + t1;

      if((B->n+t2) >= adev_q_count) break;
//tvb if((B->n+t2) > adev_q_count) break;

      while((B->n+t2) < adev_q_count) {
         v =  get_adev_point(id, B->n+t2);
         v -= get_adev_point(id, B->n+t1) * 2.0;
         v += get_adev_point(id, B->n);

         B->sum += (v * v);
         B->n++;
         adev_mouse();
      }

      if(B->n >= min_points_per_bin) {
         if(B->n && B->tau) {
            B->value = sqrt(B->sum / (2.0 * ((double) B->n + adev_q_overflow))) / B->tau;
            ++vis_bins;
         }
      }
   }

   if(vis_bins > max_adev_rows) max_adev_rows = vis_bins;
}

void incr_hdev(u08 id, struct BIN *bins)
{
S32 b;
S32 t1,t2,t3;
struct BIN *B;
double v;
int vis_bins;
long adev_q_count;
double adev_q_overflow;

   if     (id == PPS_HDEV) { adev_q_count = pps_adev_q_count; adev_q_overflow = pps_adev_q_overflow; }
   else if(id == OSC_HDEV) { adev_q_count = osc_adev_q_count; adev_q_overflow = osc_adev_q_overflow; }
   else if(id == CHC_HDEV) { adev_q_count = chc_adev_q_count; adev_q_overflow = chc_adev_q_overflow; }
   else if(id == CHD_HDEV) { adev_q_count = chd_adev_q_count; adev_q_overflow = chd_adev_q_overflow; }
   else return;

   vis_bins = 0;

   for(b=0; b<n_bins; b++) {
      B = &bins[b];
      if(B->n < 0) break;

      t1 = B->m;
      t2 = t1 + t1;
      t3 = t1 + t1 + t1;

      if((B->n+t3) >= adev_q_count) break;

      while((B->n+t3) < adev_q_count) {
         v =  get_adev_point(id, B->n+t3);
         v -= get_adev_point(id, B->n+t2) * 3.0;
         v += get_adev_point(id, B->n+t1) * 3.0;
         v -= get_adev_point(id, B->n);

         B->sum += (v * v);
         B->n++;
         adev_mouse();
      }

      if(B->n >= min_points_per_bin) {
         if(B->n && B->tau) {
            B->value = sqrt(B->sum / (6.0 * ((double) B->n + adev_q_overflow))) / B->tau;
            ++vis_bins;
         }
      }
   }

   if(vis_bins > max_adev_rows) max_adev_rows = vis_bins;
}

void incr_mdev(u08 id, struct BIN *bins)
{
S32 b;
S32 t1,t2,t3;
struct BIN *B;
double divisor;
int vis_bins;
long adev_q_count;
double adev_q_overflow;

   if     (id == PPS_MDEV) { adev_q_count = pps_adev_q_count; adev_q_overflow = pps_adev_q_overflow; }
   else if(id == OSC_MDEV) { adev_q_count = osc_adev_q_count; adev_q_overflow = osc_adev_q_overflow; }
   else if(id == CHC_MDEV) { adev_q_count = chc_adev_q_count; adev_q_overflow = chc_adev_q_overflow; }
   else if(id == CHD_MDEV) { adev_q_count = chd_adev_q_count; adev_q_overflow = chd_adev_q_overflow; }
   else return;

   vis_bins = 0;

   for(b=0; b<n_bins; b++) {
      B = &bins[b];
      if(B->n < 0) break;
      if(B->j < 0) break;
      if(B->i < 0) break;

      t1 = B->m;
      t2 = t1 + t1;
      t3 = t1 + t1 + t1;

//    if((B->j+t3) >= adev_q_count) break;
      if((B->j+t3) > adev_q_count) break;

      while(((B->i+t2) < adev_q_count) && (B->i < t1)) {
         B->accum += get_adev_point(id, B->i+t2);
         B->accum -= get_adev_point(id, B->i+t1) * 2.0;
         B->accum += get_adev_point(id, B->i);
         B->i++;
         adev_mouse();
      }

      if(B->init == 0) {
         B->sum += (B->accum * B->accum);
         B->n++;
         B->init = 1;
      }

      while((B->j+t3) < adev_q_count) {
         B->accum += get_adev_point(id, B->j+t3);
         B->accum -= get_adev_point(id, B->j+t2) * 3.0;
         B->accum += get_adev_point(id, B->j+t1) * 3.0;
         B->accum -= get_adev_point(id, B->j);

         B->sum += (B->accum * B->accum);
         B->j++;
         B->n++;
         adev_mouse();
      }

      if(B->n >= min_points_per_bin) {
         divisor = (double) B->m * B->tau;
         if(divisor != 0.0) {
            B->value = sqrt(B->sum / (2.0 * ((double) B->n + adev_q_overflow))) / divisor;
         }
         ++vis_bins;
      }
   }

   if(vis_bins > max_adev_rows) max_adev_rows = vis_bins;
}

void incr_tdev(u08 id, struct BIN *bins)
{
S32 b;
S32 t1,t2,t3;
struct BIN *B;
double divisor;
int vis_bins;
long adev_q_count;
double adev_q_overflow;

   if     (id == PPS_TDEV) { adev_q_count = pps_adev_q_count; adev_q_overflow = pps_adev_q_overflow; }
   else if(id == OSC_TDEV) { adev_q_count = osc_adev_q_count; adev_q_overflow = osc_adev_q_overflow; }
   else if(id == CHC_TDEV) { adev_q_count = chc_adev_q_count; adev_q_overflow = chc_adev_q_overflow; }
   else if(id == CHD_TDEV) { adev_q_count = chd_adev_q_count; adev_q_overflow = chd_adev_q_overflow; }
   else return;

   vis_bins = 0;

   for(b=0; b<n_bins; b++) {
      B = &bins[b];
      if(B->n < 0) break;
      if(B->j < 0) break;
      if(B->i < 0) break;

      t1 = B->m;
      t2 = t1 + t1;
      t3 = t1 + t1 + t1;

//    if((B->j+t3) >= adev_q_count) break;
      if((B->j+t3) > adev_q_count) break;

      while(((B->i+t2) < adev_q_count) && (B->i < t1)) {
         B->accum += get_adev_point(id, B->i+t2);
         B->accum -= get_adev_point(id, B->i+t1) * 2.0;
         B->accum += get_adev_point(id, B->i);
         B->i++;
         adev_mouse();
      }

      if(B->init == 0) {
         B->sum += (B->accum * B->accum);
         B->n++;
         B->init = 1;
      }

      while((B->j+t3) < adev_q_count) {
         B->accum += get_adev_point(id, B->j+t3);
         B->accum -= get_adev_point(id, B->j+t2) * 3.0;
         B->accum += get_adev_point(id, B->j+t1) * 3.0;
         B->accum -= get_adev_point(id, B->j);

         B->sum += (B->accum * B->accum);
         B->j++;
         B->n++;
         adev_mouse();
      }

      if(B->n >= min_points_per_bin) {
         divisor = (double) B->m * B->tau;
         if(divisor != 0.0) {
            B->value = sqrt(B->sum / (2.0 * ((double) B->n + adev_q_overflow))) / divisor;
            B->value = B->value * B->tau / SQRT3;
         }
         ++vis_bins;
      }
   }

   if(vis_bins > max_adev_rows) max_adev_rows = vis_bins;
}

int fetch_adev_info(u08 dev_id, struct ADEV_INFO *bins)
{
double adev;
double tau;
long on;
struct BIN *table;

    if     (dev_id == PPS_ADEV) table = &pps_adev_bins[0];
    else if(dev_id == OSC_ADEV) table = &osc_adev_bins[0];
    else if(dev_id == CHC_ADEV) table = &chc_adev_bins[0];
    else if(dev_id == CHD_ADEV) table = &chd_adev_bins[0];

    else if(dev_id == PPS_HDEV) table = &pps_hdev_bins[0];
    else if(dev_id == OSC_HDEV) table = &osc_hdev_bins[0];
    else if(dev_id == CHC_HDEV) table = &chc_hdev_bins[0];
    else if(dev_id == CHD_HDEV) table = &chd_hdev_bins[0];

    else if(dev_id == PPS_MDEV) table = &pps_mdev_bins[0];
    else if(dev_id == OSC_MDEV) table = &osc_mdev_bins[0];
    else if(dev_id == CHC_MDEV) table = &chc_mdev_bins[0];
    else if(dev_id == CHD_MDEV) table = &chd_mdev_bins[0];

    else if(dev_id == PPS_TDEV) table = &pps_tdev_bins[0];
    else if(dev_id == OSC_TDEV) table = &osc_tdev_bins[0];
    else if(dev_id == CHC_TDEV) table = &chc_tdev_bins[0];
    else if(dev_id == CHD_TDEV) table = &chd_tdev_bins[0];

    else if(dev_id == A_MTIE) return fetch_mtie_info(CHA_MTIE, bins);
    else if(dev_id == B_MTIE) return fetch_mtie_info(CHB_MTIE, bins);
    else if(dev_id == C_MTIE) return fetch_mtie_info(CHC_MTIE, bins);
    else if(dev_id == D_MTIE) return fetch_mtie_info(CHD_MTIE, bins);
    else {
       sprintf(out, "Bad dev_id in calc_adevs: %d\n", dev_id);
       error_exit(92, out);
    }

    bins->adev_min = 1.0e29F;
    bins->adev_max = (-1.0e29F);
    bins->bin_count = 0;
    bins->adev_type = dev_id;

    // convert data from the incremental adev bins into the
    // old style adev tables (so we don't have to mess with changing
    // all that old well-debugged display code).
    while(bins->bin_count < MAX_ADEV_BINS) {
        on = table[bins->bin_count].n;
        if(on < min_points_per_bin) {
           if(1) {  // zap unused entries   // aaaahhhh
              bins->adev_on[bins->bin_count] = 0;
              bins->adev_taus[bins->bin_count] = (float) 0.0;
              bins->adev_bins[bins->bin_count] = (float) 0.0;
           }
////bins->bin_count += 1;
////continue;
           break;
        }

        tau = (double) table[bins->bin_count].tau;

        adev = table[bins->bin_count].value;

        bins->adev_on[bins->bin_count] = on;
        bins->adev_taus[bins->bin_count] = (float) tau;
        bins->adev_bins[bins->bin_count] = (float) adev;

        bins->bin_count += 1;
        if(adev > bins->adev_max) bins->adev_max = (float) adev;
        if(adev < bins->adev_min) bins->adev_min = (float) adev;
    }

    if(1 && global_adev_max) {  // give all adev plots the same top decade
       bins->adev_max = (float) global_adev_max;
    }

    bins->adev_max = adev_decade(bins->adev_max);

    // !!!!!! force tidy ADEV graph scale factors
    // aaaahhhh - we need to do this better... what if one VERT_MAJOR per decade
    if((PLOT_HEIGHT/VERT_MAJOR) <= 6) {  // three decades (/VS - makes for a cramped plot) 
       bins->adev_min = bins->adev_max * 1.0e-6F;
    }
    else if((PLOT_HEIGHT/VERT_MAJOR) <= 8) {  // four decades
       bins->adev_min = bins->adev_max * 1.0e-4F;
    }
    else if((PLOT_HEIGHT/VERT_MAJOR) <= 10) {  // five decades (/VM)
       bins->adev_min = bins->adev_max * 1.0e-5F;
    }
    else if((PLOT_HEIGHT/VERT_MAJOR) <= 12) {  // six decades
       bins->adev_min = bins->adev_max * 1.0e-6F;
    }
    else if((PLOT_HEIGHT/VERT_MAJOR) <= 14) {  // seven decades
       bins->adev_min = bins->adev_max * 1.0e-7F;
    }
    else if((PLOT_HEIGHT/VERT_MAJOR) <= 16) {  // eight decades (/VL)
       bins->adev_min = bins->adev_max * 1.0e-8F;
    }
    else if((PLOT_HEIGHT/VERT_MAJOR) <= 18) {  // nine decades
       bins->adev_min = bins->adev_max * 1.0e-9F;
    }
    else if((PLOT_HEIGHT/VERT_MAJOR) <= 20) {  // ten decades
       bins->adev_min = bins->adev_max * 1.0e-10F;
    }
    else if((PLOT_HEIGHT/VERT_MAJOR) <= 22) {  // eleven decades
       bins->adev_min = bins->adev_max * 1.0e-11F;
    }
    else if((PLOT_HEIGHT/VERT_MAJOR) <= 24) {  // twelve decades
       bins->adev_min = bins->adev_max * 1.0e-12F;
    }
    else if((PLOT_HEIGHT/VERT_MAJOR) <= 26) {  // 13 decades
       bins->adev_min = bins->adev_max * 1.0e-13F;
    }
    else if((PLOT_HEIGHT/VERT_MAJOR) <= 28) {  // 14 decades
       bins->adev_min = bins->adev_max * 1.0e-14F;
    }
    else if((PLOT_HEIGHT/VERT_MAJOR) <= 30) {  // 15 decades
       bins->adev_min = bins->adev_max * 1.0e-15F;
    }
    else if((PLOT_HEIGHT/VERT_MAJOR) <= 32) {  // 16 decades
       bins->adev_min = bins->adev_max * 1.0e-16F;
    }
    else if((PLOT_HEIGHT/VERT_MAJOR) <= 34) {  // 17 decades
       bins->adev_min = bins->adev_max * 1.0e-17F;
    }
    else if((PLOT_HEIGHT/VERT_MAJOR) <= 36) {  // 18 decades
       bins->adev_min = bins->adev_max * 1.0e-18F;
    }
    else if((PLOT_HEIGHT/VERT_MAJOR) <= 38) {  // 19 decades
       bins->adev_min = bins->adev_max * 1.0e-19F;
    }
    else if((PLOT_HEIGHT/VERT_MAJOR) <= 40) {  // 20 decades
       bins->adev_min = bins->adev_max * 1.0e-20F;
    }
    else { // ten decades?
       bins->adev_min = bins->adev_max * 1.0e-10F;
    }

    if((PLOT_HEIGHT/VERT_MAJOR) & 0x01) {  // odd number of decade bins are off-centered
       bins->adev_max /= 2.0F;
       bins->adev_min *= 5.0F;
    }

    return bins->bin_count;
}


void reload_adev_queue(int reset_phase)
{
long i;
long counter;
long max_count;
struct PLOT_Q q;
int dump_size;
int have_cha_tie, have_chb_tie, have_chc_tie, have_chd_tie;
double cha_tie, chb_tie, chc_tie, chd_tie;
int have_cha_ofs, have_chb_ofs, have_chc_ofs, have_chd_ofs;
double cha_ofs, chb_ofs, chc_ofs, chd_ofs;
double old_cha_ofs, old_chb_ofs, old_chc_ofs, old_chd_ofs;
double cha_tick, chb_tick, chc_tick, chd_tick;
OFS_SIZE old_pps_offset, old_osc_offset;
int old_have_pps_offset, old_have_osc_offset;
long pcount;

   have_cha_tie = have_chb_tie = have_chc_tie = have_chd_tie = 0;
   have_cha_ofs = have_chb_ofs = have_chc_ofs = have_chd_ofs = 0;

   old_have_pps_offset = have_pps_offset;  // receiver derived adev related values
   old_have_osc_offset = have_osc_offset;

   if(reset_phase) dont_reset_phase = 0;
   else            dont_reset_phase = 1;
   if(rcvr_type == TICC_RCVR) {
      if(mtie_q_count[CHA_MTIE]) have_cha_tie = 1;
      if(mtie_q_count[CHB_MTIE]) have_chb_tie = 1;
      if(mtie_q_count[CHC_MTIE]) have_chc_tie = 1;
      if(mtie_q_count[CHD_MTIE]) have_chd_tie = 1;
      reset_queues(RESET_MTIE_Q, 1101);
   }

   if(pps_adev_q_count) have_cha_ofs = 1;
   if(osc_adev_q_count) have_chb_ofs = 1;
   if(chc_adev_q_count) have_chc_ofs = 1;
   if(chd_adev_q_count) have_chd_ofs = 1;


   reset_queues(RESET_ADEV_Q, 1102);   // clear the adev queue
   dont_reset_phase = 0;
////   pause_data = 1;

   max_count = view_interval * (long) PLOT_WIDTH;
   max_count /= (long) plot_mag;
   counter = 0;

   dump_size = 'p';
   if(dump_size == 'p') i = plot_q_col0;  // reloading from the displayed plot area
   else                 i = plot_q_out;   // reloading from the full queue

   if(queue_interval == 0) return;

   cha_tick = pps_adev_period; //  / (double) queue_interval;
   chb_tick = osc_adev_period; //  / (double) queue_interval;
   chc_tick = chc_adev_period; //  / (double) queue_interval;
   chd_tick = chd_adev_period; //  / (double) queue_interval;

   pcount = plot_q_size;
   while(pcount) {
      --pcount;
      q = get_plot_q(i);

      old_cha_ofs = pps_phase;
      old_chb_ofs = osc_phase;
      old_chc_ofs = chc_phase;
      old_chd_ofs = chd_phase;
      old_osc_offset = osc_offset;
      old_pps_offset = pps_offset;

      cha_ofs = cha_tie = 0.0;
      chb_ofs = chb_tie = 0.0;
      chc_ofs = chc_tie = 0.0;
      chd_ofs = chd_tie = 0.0;

      if(queue_interval) {
         cha_ofs = cha_tie = (OFS_SIZE) q.data[PPS] / (OFS_SIZE) queue_interval;
         chb_ofs = chb_tie = (OFS_SIZE) q.data[OSC] / (OFS_SIZE) queue_interval;
         chc_ofs = chc_tie = (OFS_SIZE) q.data[CHC] / (OFS_SIZE) queue_interval;
         chd_ofs = chd_tie = (OFS_SIZE) q.data[CHD] / (OFS_SIZE) queue_interval;
      }

      if(rcvr_type == TICC_RCVR) {  // MTIE data
         --cha_tick;
         --chb_tick;
         --chc_tick;
         --chd_tick;

         if(have_cha_tie && (cha_tick <= 0.0)) add_mtie_point(CHA_MTIE, cha_tie); 
         if(have_chb_tie && (chb_tick <= 0.0)) add_mtie_point(CHB_MTIE, chb_tie); 
         if(have_chc_tie && (chc_tick <= 0.0)) add_mtie_point(CHC_MTIE, chc_tie); 
         if(have_chd_tie && (chd_tick <= 0.0)) add_mtie_point(CHD_MTIE, chd_tie);

         cha_ofs = (OFS_SIZE) q.data[ONE] / (OFS_SIZE) queue_interval;
         chb_ofs = (OFS_SIZE) q.data[TWO] / (OFS_SIZE) queue_interval;
         chc_ofs = (OFS_SIZE) q.data[THREE] / (OFS_SIZE) queue_interval;
         chd_ofs = (OFS_SIZE) q.data[FOUR] / (OFS_SIZE) queue_interval;

         if(have_cha_ofs && (cha_tick <= 0.0)) add_pps_adev_point(cha_ofs, 1);   // adev values
         if(have_chb_ofs && (chb_tick <= 0.0)) add_osc_adev_point(chb_ofs, 1); 
         if(have_chc_ofs && (chc_tick <= 0.0)) add_chc_adev_point(chc_ofs, 1); 
         if(have_chd_ofs && (chd_tick <= 0.0)) add_chd_adev_point(chd_ofs, 1);  

         if(cha_tick <= 0.0) cha_tick = pps_adev_period; // / (double) queue_interval;
         if(cha_tick <= 0.0) chb_tick = osc_adev_period; // / (double) queue_interval;
         if(cha_tick <= 0.0) chc_tick = chc_adev_period; // / (double) queue_interval;
         if(cha_tick <= 0.0) chd_tick = chd_adev_period; // / (double) queue_interval;
      }
      else {  // adevs from receiver data or external TICC
         pps_offset = cha_ofs;
         osc_offset = chb_ofs;
         have_pps_offset = have_cha_ofs;
         have_osc_offset = have_chb_ofs;

         add_rcvr_adev_point(1);

         pps_offset = old_pps_offset;
         osc_offset = old_osc_offset;
         have_pps_offset = old_have_pps_offset;
         have_osc_offset = old_have_osc_offset;
      }

      ++counter;
      if((counter % LOG_GPS_CHECK) == 0) {   // keep serial data from overruning
         get_pending_gps(30);  //!!!! possible recursion
      }
//    if((counter % 1000L) == 1L) {
      if((counter % LOG_SCREEN_RFSH) == 1L) {
         sprintf(out, "Line %ld", counter-1L);
         vidstr(PLOT_TEXT_ROW+4+2, PLOT_TEXT_COL, PROMPT_COLOR, out);
         refresh_page();
      }

      if(dump_size == 'p') {  // reloading from the displayed plot area
         if(counter >= max_count) break;
      }
      if(++i >= plot_q_size) i = 0;  // queue pointer wrapped
   }

   if(rcvr_type == TICC_RCVR) {
      find_global_max();
   }

   force_adev_redraw(11);
   redraw_screen();
}

//
//   Allan deviation table output and plotting stuff
//

int fperiod(double period)
{
   // returns true if period is not an integer
   if(((double) (int) period) != period) return 1;
   return 0;
}

int show_adev_table(struct ADEV_INFO *bins, int adev_row, int adev_col, u08 color, int why)
{
int i;
int id;
char *d;
char *t;
long adev_q_count;
char c;
int count;
char adev_id[32];
int mask;
double tau;
double period;
char pstring[128];

   left_adev_col = adev_col;

   if(text_mode) {
      if(first_key) return 0;
      if(all_adevs == SINGLE_ADEVS) return 0;     // no room on screen to do this
   }
   if(zoom_screen) return 0;
   if(bins->bin_count <= 0) return 0;              // nothing to show
   if(adevs_active(1) == 0) return 0;
   if((SCREEN_WIDTH < NARROW_SCREEN) && (all_adevs == SINGLE_ADEVS)) {  // no room for ADEV tables
      return 0;
   }
   if(osc_params && (rcvr_type == TICC_RCVR)) return 0;

   mask = 0xFFFF;
   period = adev_period;
   if     (bins->adev_type == PPS_ADEV) { t = "ADEV"; adev_q_count = pps_adev_q_count+(long) pps_adev_q_overflow; mask = DISPLAY_ADEV; period = pps_adev_period; } 
   else if(bins->adev_type == OSC_ADEV) { t = "ADEV"; adev_q_count = osc_adev_q_count+(long) osc_adev_q_overflow; mask = DISPLAY_ADEV; period = osc_adev_period; } 
   else if(bins->adev_type == CHC_ADEV) { t = "ADEV"; adev_q_count = chc_adev_q_count+(long) chc_adev_q_overflow; mask = DISPLAY_ADEV; period = chc_adev_period; } 
   else if(bins->adev_type == CHD_ADEV) { t = "ADEV"; adev_q_count = chd_adev_q_count+(long) chd_adev_q_overflow; mask = DISPLAY_ADEV; period = chd_adev_period; } 

   else if(bins->adev_type == PPS_HDEV) { t = "HDEV"; adev_q_count = pps_adev_q_count+(long) pps_adev_q_overflow; mask = DISPLAY_HDEV; period = pps_adev_period; } 
   else if(bins->adev_type == OSC_HDEV) { t = "HDEV"; adev_q_count = osc_adev_q_count+(long) osc_adev_q_overflow; mask = DISPLAY_HDEV; period = osc_adev_period; } 
   else if(bins->adev_type == CHC_HDEV) { t = "HDEV"; adev_q_count = chc_adev_q_count+(long) chc_adev_q_overflow; mask = DISPLAY_HDEV; period = chc_adev_period; } 
   else if(bins->adev_type == CHD_HDEV) { t = "HDEV"; adev_q_count = chd_adev_q_count+(long) chd_adev_q_overflow; mask = DISPLAY_HDEV; period = chd_adev_period; } 

   else if(bins->adev_type == PPS_MDEV) { t = "MDEV"; adev_q_count = pps_adev_q_count+(long) pps_adev_q_overflow; mask = DISPLAY_MDEV; period = pps_adev_period; } 
   else if(bins->adev_type == OSC_MDEV) { t = "MDEV"; adev_q_count = osc_adev_q_count+(long) osc_adev_q_overflow; mask = DISPLAY_MDEV; period = osc_adev_period; } 
   else if(bins->adev_type == CHC_MDEV) { t = "MDEV"; adev_q_count = chc_adev_q_count+(long) chc_adev_q_overflow; mask = DISPLAY_MDEV; period = chc_adev_period; } 
   else if(bins->adev_type == CHD_MDEV) { t = "MDEV"; adev_q_count = chd_adev_q_count+(long) chd_adev_q_overflow; mask = DISPLAY_MDEV; period = chd_adev_period; } 

   else if(bins->adev_type == PPS_TDEV) { t = "TDEV"; adev_q_count = pps_adev_q_count+(long) pps_adev_q_overflow; mask = DISPLAY_TDEV; period = pps_adev_period; } 
   else if(bins->adev_type == OSC_TDEV) { t = "TDEV"; adev_q_count = osc_adev_q_count+(long) osc_adev_q_overflow; mask = DISPLAY_TDEV; period = osc_adev_period; } 
   else if(bins->adev_type == CHC_TDEV) { t = "TDEV"; adev_q_count = chc_adev_q_count+(long) chc_adev_q_overflow; mask = DISPLAY_TDEV; period = chc_adev_period; } 
   else if(bins->adev_type == CHD_TDEV) { t = "TDEV"; adev_q_count = chd_adev_q_count+(long) chd_adev_q_overflow; mask = DISPLAY_TDEV; period = chd_adev_period; } 

   else if(bins->adev_type == A_MTIE)   { t = "MTIE"; adev_q_count = mtie_intervals[CHA_MTIE][0]; mask = DISPLAY_MTIE; period = pps_adev_period; } 
   else if(bins->adev_type == B_MTIE)   { t = "MTIE"; adev_q_count = mtie_intervals[CHB_MTIE][0]; mask = DISPLAY_MTIE; period = osc_adev_period; } 
   else if(bins->adev_type == C_MTIE)   { t = "MTIE"; adev_q_count = mtie_intervals[CHC_MTIE][0]; mask = DISPLAY_MTIE; period = chc_adev_period; } 
   else if(bins->adev_type == D_MTIE)   { t = "MTIE"; adev_q_count = mtie_intervals[CHD_MTIE][0]; mask = DISPLAY_MTIE; period = chd_adev_period; } 
   else { t="????"; adev_q_count = 0; }
   if(period <= 0.0) return 0;      // aaaapppp other periods 
   
   if(SCREEN_WIDTH < ADEV_AZEL_THRESH) {  // no room for both a map and adev tables
      if(all_adevs == SINGLE_ADEVS) {
         if(plot_watch) {
            if(shared_plot && (shared_item & SHARE_WATCH)) ;
            else return 0;
         }
         if(plot_azel) {
            if(shared_plot && (shared_item & SHARE_MAP)) ;
            else return 0;
         }
         if(plot_signals) {
            if(shared_plot && (shared_item & SHARE_SIGNALS)) ;
            else return 0;
         }
         if(plot_lla) {
            if(shared_plot && (shared_item & SHARE_LLA)) ;
            else return 0;
         }

         if(precision_survey) return 0;
         if(check_precise_posn) return 0;
         if(precise_survey_done) return 0;
         if(show_fixes) return 0;
      }
   }

   if((bins->adev_bins[0] == 0.0) && (bins->adev_bins[1] == 0.0)) return 0;

   calc_adev_display_size();
      
   id = bins->adev_type / NUM_ADEV_TYPES;

   if(id == CHD_ID) {
      d = "chD";
   }
   else if(id == CHC_ID) {
      d = "chC";
   }
   else if(id == PPS_ID) {
      if(jitter_adev)    d = "Msg jit"; //"PPS";
      else if(TICC_USED) d = "chA";
      else               d = plot[PPS].plot_id; //"PPS";
   }
   else {
      if(jitter_adev)    d = "Msg ofs"; //"OSC";
      else if(TICC_USED) d = "chB";
      else               d = plot[OSC].plot_id; //"OSC";
   }

if(ATYPE == A_MTIE) {
// if((mask & adev_display_mask) == 0) return 0;
}

   strcpy(adev_id, t);
   if(TICC_USED == 0) strlwr(adev_id);  // show bogo-adevs in lower case
   else if(ticc_type == LARS_TICC) strlwr(adev_id);

   blank_underscore = 1; // '_' char is used for formatting columns,  convert to blank

   if(all_adevs && (rcvr_type == TICC_RCVR)) {
      vidstr(adev_row,adev_col, WHITE, "                       ");
   }

   if(last_was_adev) c = UP_DOWN_CHAR;  // show adev scrolling is active
   else              c = ':';           // show plot scrolling is active

//sprintf(debug_text3, "%s  lwa:%d(%d)  fsb:%d  mbs:%d  atr:%d  aps:%d  abr:%d  aar:%d  bc:%d", 
//t, last_was_adev,lwa, first_show_bin, max_bins_shown, adev_table_rows, adev_page_size, adev_bottom_row, all_adev_row, bins->bin_count);

if(0) {  // try to fix mystery bug where adev display starts at the wrong bin
   for(i=0; i<bins->bin_count; i++) {
      if(bins->adev_taus[i] == 0) {  // we should never see a bin with tau=0
         first_show_bin = 0;
         break;
      }
   }
}

   count = 0;
   if(all_adevs || (SCREEN_WIDTH < MEDIUM_WIDTH)) {  // 800x600 - short version of tables
      left_adev_col = adev_col;
       sprintf(out, "%s %s%c %lu pts", d, adev_id,c, adev_q_count);
///   sprintf(out, "%s %s%c %lu pts bc:%d", d, adev_id,c, adev_q_count,bins->bin_count);
      vidstr(adev_row, left_adev_col, color, &blanks[TEXT_COLS-max_adev_width]);
      vidstr(adev_row, left_adev_col, color, out);
      adevs_shown = strlen(out);
      if(bins->bin_count > max_bins_shown) max_bins_shown = bins->bin_count;

      for(i=first_show_bin; i<bins->bin_count; i++) {
         ++adev_row;
         if(all_adevs && (left_adev_col < INFO_COL)) {
            if(adev_row > (MOUSE_ROW+0)) break;  // mmmmmm
         }
         else if(adev_row > (MOUSE_ROW+1)) break;  // mmmmmm

         if(max_adev_width > 5) {  // erase the old data
//          fill_rectangle (left_adev_col*TEXT_WIDTH+TEXT_X_MARGIN,adev_row*TEXT_HEIGHT+TEXT_Y_MARGIN, (max_adev_width)*TEXT_WIDTH,TEXT_HEIGHT, RED);
            erase_rectangle(left_adev_col*TEXT_WIDTH+TEXT_X_MARGIN,adev_row*TEXT_HEIGHT+TEXT_Y_MARGIN, (max_adev_width)*TEXT_WIDTH,TEXT_HEIGHT);
         }
         if(++count >= adev_table_rows) break;
         if(ATYPE == A_MTIE) {
            if(bins->adev_on[i] < 0.0) break;
         }
         else if(bins->adev_on[i] <= 0.0) break;

         tau = bins->adev_taus[i];
         strcpy(pstring, "??????");

         if(fperiod(period)) {  // non-integer period
            if     (tau < 100.0)    sprintf(pstring, "%6.3f", tau);
            else if(tau < 1000.0)   sprintf(pstring, "%6.2f", tau);
            else if(tau < 10000.0)  sprintf(pstring, "%6.1f", tau);
            else                    sprintf(pstring, "%6.0f", tau);
         }
         else {
            sprintf(pstring, "%6ld", (long) tau);
         }

         if(all_adevs && (SCREEN_WIDTH >= MEDIUM_WIDTH) && (text_mode == 0)) {
            if(tau >= 1000000.0) {
               sprintf(out, "%st %.4le (n=%ld)", pstring, bins->adev_bins[i], bins->adev_on[i]);
            }
            else {
               sprintf(out, "%s t %.4le (n=%ld)", pstring, bins->adev_bins[i], bins->adev_on[i]);
            }
            adevs_shown = strlen(out);
         }
         else {
            if(1 || (SCREEN_WIDTH < NARROW_SCREEN)) {
               sprintf(out, "%st %.3le", pstring, bins->adev_bins[i]);
            }
            else {
               sprintf(out, "%s t %.3le", pstring, bins->adev_bins[i]);
            }
            adevs_shown = strlen(out);
            adevs_shown += 6;  // make sure view info erases
         }
         vidstr(adev_row, left_adev_col, color, out);

         if(adevs_shown > max_adev_width) max_adev_width = adevs_shown+1;
      }
   }
   else {   // long version of tables will fit
      left_adev_col = adev_col+4;
      sprintf(out, "%s %s%c for %lu points", d, adev_id,c, adev_q_count);
///   sprintf(out, "%s %s%c %lu pts bc::%d", d, adev_id,c, adev_q_count,bins->bin_count);
      vidstr(adev_row, adev_col, color, &blanks[TEXT_COLS-max_adev_width]);
      vidstr(adev_row, adev_col+7, color, out);
      adevs_shown = 7 + 5 + strlen(out) + 2;
      adevs_shown = strlen(out);
      if(bins->bin_count > max_bins_shown) {
         max_bins_shown = bins->bin_count;
      }

      for(i=first_show_bin; i<bins->bin_count; i++) {  // draw the adev table entries
         ++adev_row;
         if(all_adevs && (left_adev_col < INFO_COL)) {
            if(adev_row > (MOUSE_ROW+0)) break;  // mmmmmm
         }
         else if(adev_row > (MOUSE_ROW+1)) break;  // mmmmmm

         if(max_adev_width > 5) {   // erase the old data
//          fill_rectangle (left_adev_col*TEXT_WIDTH+TEXT_X_MARGIN,adev_row*TEXT_HEIGHT+TEXT_Y_MARGIN, max_adev_width*TEXT_WIDTH,TEXT_HEIGHT, RED);
            erase_rectangle(left_adev_col*TEXT_WIDTH+TEXT_X_MARGIN,adev_row*TEXT_HEIGHT+TEXT_Y_MARGIN, max_adev_width*TEXT_WIDTH,TEXT_HEIGHT);
         }
         if(++count >= adev_table_rows) break;
         if(ATYPE == A_MTIE) {
            if(bins->adev_on[i] < 0.0) break;
         }
         else if(bins->adev_on[i] <= 0.0) break;

         tau = bins->adev_taus[i];
         strcpy(pstring, "????????");

         if(fperiod(period)) {
            if     (tau < 1000.0)    sprintf(pstring, "%8.4f", tau);
            else if(tau < 10000.0)   sprintf(pstring, "%8.3f", tau);
            else if(tau < 100000.0)  sprintf(pstring, "%8.2f", tau);
            else if(tau < 1000000.0) sprintf(pstring, "%8.1f", tau);
            else                     sprintf(pstring, "%8.0f", tau);
         }
         else {
            sprintf(pstring, "%8ld", (long) tau);
         }

         sprintf(out, "%s tau  %.3le (n=%ld)", pstring, bins->adev_bins[i], bins->adev_on[i]);
         vidstr(adev_row, left_adev_col, color, out);

         adevs_shown = strlen(out);
         if(adevs_shown > max_adev_width) max_adev_width = adevs_shown+1;
      }
   }

   ++adev_row;
//sprintf(debug_text, "maw:%d  left:%d  arow:%d  bot:%d  geight:%d  mouse:%d", 
//max_adev_width, left_adev_col, adev_row, adev_bottom_row, (adev_bottom_row-adev_row+1), MOUSE_ROW);
   i = (adev_bottom_row-adev_row+1);       // i=number of rows to erase
   if(all_adevs && (left_adev_col < INFO_COL)) {
      if(adev_bottom_row >= (MOUSE_ROW+0)) {  // mmmmm would eat into plot titles
         i -= (adev_bottom_row - (MOUSE_ROW+0));  // mmmmm adjust it so it won't
      }
   }
   else if(adev_bottom_row >= (MOUSE_ROW+1)) {  // mmmmm would eat into plot titles
      i -= (adev_bottom_row - (MOUSE_ROW+1));  // mmmmm adjust it so it won't
   }
   if(i > 0) {  // erase the rest of the adev table
//    fill_rectangle(left_adev_col*TEXT_WIDTH+TEXT_X_MARGIN, adev_row*TEXT_HEIGHT+TEXT_Y_MARGIN, max_adev_width*TEXT_WIDTH,i*TEXT_HEIGHT, BLUE);
      erase_rectangle(left_adev_col*TEXT_WIDTH+TEXT_X_MARGIN,adev_row*TEXT_HEIGHT+TEXT_Y_MARGIN, max_adev_width*TEXT_WIDTH,i*TEXT_HEIGHT);
   }

   blank_underscore = 0;
   return 1;
}


void plot_adev_curve(struct ADEV_INFO *bins, u08 color, int mask)
{
int i;
int x1, x2;
float y1, y2;
float max_scale;
float min_scale;
float adev_range;
int vert_scale;
int vert_ofs;

double n;
float m;
float eb1,eb2;


   if(text_mode) return;  // no room on screen to do this
   if(rcvr_type == NO_RCVR) return;  // no room on screen to do this
   if(rcvr_type == TIDE_RCVR) return;  // ckckck has no adevs to show

   if(first_key) return;
   if(plot_adev_data == 0) return;
   if(bins == 0) return;

   if(zoom_screen == 'P') ;
   else if(zoom_screen) return;

   if(bins->adev_bins[0] == 0.0) return;

if((mask & adev_display_mask) == 0) return;

   max_scale = scale_adev(bins->adev_max);
   if(bottom_adev_decade) min_scale = scale_adev(bottom_adev_decade);
   else                   min_scale = scale_adev(bins->adev_min);
if(ATYPE == A_MTIE) {  // offset plots a little from the top and bottom of the plot area
   min_scale = scale_adev(bins->adev_min);
max_scale = (float) (global_adev_max*MTIE_SCALE);
min_scale = (float) (global_adev_min/MTIE_SCALE);
}

   adev_range = max_scale - min_scale;
   adev_range = (float) ABS(adev_range);
//sprintf(debug_text2, "ascale min:%g  max:%g  range:%g", min_scale, max_scale, adev_range);
   if(adev_range == 0.0) return;


   // adev plots are scaled to major vertical divisions
   vert_scale = (PLOT_HEIGHT / (VERT_MAJOR*2)) * (VERT_MAJOR*2);  // two major divisions per decade
   vert_ofs = (PLOT_HEIGHT - vert_scale) / 2;
   if(((PLOT_HEIGHT / (VERT_MAJOR*2)) & 0x01) == 0x00) {  // align adevs to cyan dotted lines
      vert_ofs += VERT_MAJOR;
   }
//sprintf(plot_title, "bot:%g  max(%g):%g min(%g):%g range:%g", bottom_adev_decade, bins->adev_max,max_scale, bins->adev_min,min_scale, adev_range);

   for(i=first_show_bin; i<bins->bin_count; i++) {
      x1 = (i-first_show_bin-1) * HORIZ_MAJOR;   // make sure line fits in the plot
      x2 = (i-first_show_bin) * HORIZ_MAJOR;
if(x1 > PLOT_WIDTH) continue; // aaaahhhh 
if(x2 > PLOT_WIDTH) continue;
if(x1 < 0) continue;
if(x2 < 0) continue;
      if(x1 < 0) x1 = 0;
      if(x2 > PLOT_WIDTH) x2 = PLOT_WIDTH;

      y1 = (0.0F - scale_adev(bins->adev_bins[i-1]));
      y1 += max_scale;
      y1 /= adev_range;
      y1 *= (float) vert_scale;
      y1 += (float) (PLOT_ROW + vert_ofs);
      if(y1 <= PLOT_ROW) y1 = (float) (PLOT_ROW+2);
      if(y1 >= (PLOT_ROW+PLOT_HEIGHT-1)) y1 = (float) (PLOT_ROW+PLOT_HEIGHT-2);

      y2 = eb1 = eb2 = bins->adev_bins[i];
      y2 = (0.0F - scale_adev(y2));
      y2 += max_scale;
      y2 /= adev_range;
      y2 *= (float) vert_scale;
      y2 += (float) (PLOT_ROW+vert_ofs);
      if(y2 <= PLOT_ROW) y2 = (float) (PLOT_ROW+2);
      if(y2 >= (PLOT_ROW+PLOT_HEIGHT-1)) y2 = (float) (PLOT_ROW+PLOT_HEIGHT-2);

      if(x1 < PLOT_WIDTH) {
         line(PLOT_COL+x1, (int) y1, PLOT_COL+x2, (int) y2, color);
      }
   }

   // draw error bars from last bin to first bin
   if(show_error_bars == 0) return;  // bars disabled
if(ATYPE == A_MTIE) return;

   for(i=bins->bin_count-1; i>=first_show_bin; i--) {
      eb1 = eb2 = bins->adev_bins[i];

      x1 = (i-first_show_bin-1) * HORIZ_MAJOR;   // make sure line fits in the plot
      x2 = (i-first_show_bin) * HORIZ_MAJOR;
if(x1 > PLOT_WIDTH) continue;
if(x2 > PLOT_WIDTH) continue;
if(x1 < 0) continue;
if(x2 < 0) continue;
      if(x1 < 0) x1 = 0;
      if(x2 > PLOT_WIDTH) x2 = PLOT_WIDTH;

      if(1 || (x1 < PLOT_WIDTH)) {
         m = (float) bins->adev_taus[i];
         if(m == 0) continue;

         n = (double) bins->adev_on[i];
         n = sqrt(n/(double) m);
         if(n == 0) continue;

         eb1 -= (eb1 / (float) n);
         eb1 = (0.0F - scale_adev(eb1));
         eb1 += max_scale;
         eb1 /= adev_range;
         eb1 *= vert_scale;
         eb1 += (PLOT_ROW+vert_ofs);
         if(eb1< PLOT_ROW) eb1 = (float) PLOT_ROW;
         if(eb1 >= (PLOT_ROW+PLOT_HEIGHT)) eb1 = (float) (PLOT_ROW+PLOT_HEIGHT);

         eb2 += (eb2 / (float) n);
         eb2 = (0.0F - scale_adev(eb2));
         eb2 += max_scale;
         eb2 /= adev_range;
         eb2 *= vert_scale;
         eb2 += (PLOT_ROW+vert_ofs);
         if(eb2< PLOT_ROW) eb2 = (float) PLOT_ROW;
         if(eb2 >= (PLOT_ROW+PLOT_HEIGHT)) eb2 = (float) (PLOT_ROW+PLOT_HEIGHT);


         if(n < 1.0) {  // error bars are too large to be meaningful - limit size and use arrow endcaps
            eb1 = y2 - (float) (adev_decade_height / 2.0);
            eb2 = y2 + (float) ( adev_decade_height / 2.0);

            if(eb1< PLOT_ROW) eb1 = (float) PLOT_ROW;
            if(eb1 >= (PLOT_ROW+PLOT_HEIGHT)) eb1 = (float) (PLOT_ROW+PLOT_HEIGHT);

            if(eb2< PLOT_ROW) eb2 = (float) PLOT_ROW;
            if(eb2 >= (PLOT_ROW+PLOT_HEIGHT)) eb2 = (float) (PLOT_ROW+PLOT_HEIGHT);

            line(PLOT_COL+x2,(int) eb1, PLOT_COL+x2,(int)eb2, color);

            line(PLOT_COL+x2-HORIZ_MINOR/3,(int) eb1+VERT_MINOR, PLOT_COL+x2,(int)eb1, color);
            line(PLOT_COL+x2+HORIZ_MINOR/3,(int) eb1+VERT_MINOR, PLOT_COL+x2,(int)eb1, color);

            line(PLOT_COL+x2-HORIZ_MINOR/3,(int) eb2-VERT_MINOR, PLOT_COL+x2,(int)eb2, color);
            line(PLOT_COL+x2+HORIZ_MINOR/3,(int) eb2-VERT_MINOR, PLOT_COL+x2,(int)eb2, color);
         }
         else if(0 || (fabs(eb2-eb1) >= (double) (VERT_MINOR*1))) { // good bars, use flat end caps
            if(0) {  // center error bars on the adev line
               y2 = bins->adev_bins[i];
               y2 = (0.0F - scale_adev(y2));
               y2 += max_scale;
               y2 /= adev_range;
               y2 *= (float) vert_scale;
               y2 += (float) (PLOT_ROW+vert_ofs);
               if(y2 < PLOT_ROW) y2 = (float) PLOT_ROW;
               if(y2 >= (PLOT_ROW+PLOT_HEIGHT)) y2 = (float) (PLOT_ROW+PLOT_HEIGHT);

               m = (float) (fabs(eb1-eb2) / 2.0);
               eb1 = y2 - m;
               eb2 = y2 + m;
            }

            line(PLOT_COL+x2,(int) eb1, PLOT_COL+x2,(int)eb2, color);

            line(PLOT_COL+x2-HORIZ_MINOR/3,(int) eb1, PLOT_COL+x2+HORIZ_MINOR/3,(int)eb1, color);
            line(PLOT_COL+x2-HORIZ_MINOR/3,(int) eb2, PLOT_COL+x2+HORIZ_MINOR/3,(int)eb2, color);
         }
         else {  // stop on last non-short error bar
            break;
         }
      }
   }

}

void scan_bins(struct BIN *bin)
{
int i;

   // find the min and max adev values in the specified adev bins
// for(i=0; i<MAX_ADEV_BINS; i++) {
   for(i=first_show_bin; i<MAX_ADEV_BINS; i++) {
// aaaahhhh    if(bin[i].n < min_points_per_bin) break;
      if(bin[i].n < min_points_per_bin) continue;
      if(bin[i].value == 0.0) continue;
      if(bin[i].value > global_adev_max) global_adev_max = bin[i].value;
      if(bin[i].value < global_adev_min) global_adev_min = bin[i].value;
   }
}

void find_global_max()
{
   // find the min and max adev values in all the displayed xDEV bins

   global_adev_max = (-(BIG_NUM));
   global_adev_min = (BIG_NUM);

   if(all_adevs == SINGLE_ADEVS) {  // main screen adev display of PPS/chA and OSC/chB values
      if(ATYPE == OSC_ADEV) {
         if(adev_display_mask & DISPLAY_ADEV) {
            if(adev_display_mask & DISPLAY_CHA) scan_bins(&pps_adev_bins[0]);
            if(adev_display_mask & DISPLAY_CHB) scan_bins(&osc_adev_bins[0]);
         }
      }
      else if(ATYPE == OSC_HDEV) {
         if(adev_display_mask & DISPLAY_HDEV) {
            if(adev_display_mask & DISPLAY_CHA) scan_bins(&pps_hdev_bins[0]);
            if(adev_display_mask & DISPLAY_CHB) scan_bins(&osc_hdev_bins[0]);
         }
      }
      else if(ATYPE == OSC_MDEV) {
         if(adev_display_mask & DISPLAY_MDEV) {
            if(adev_display_mask & DISPLAY_CHA) scan_bins(&pps_mdev_bins[0]);
            if(adev_display_mask & DISPLAY_CHB) scan_bins(&osc_mdev_bins[0]);
         }
      }
      else if(ATYPE == OSC_TDEV) {
         if(adev_display_mask & DISPLAY_TDEV) {
            if(adev_display_mask & DISPLAY_CHA) scan_bins(&pps_tdev_bins[0]);
            if(adev_display_mask & DISPLAY_CHB) scan_bins(&osc_tdev_bins[0]);
         }
      }
      else if(ATYPE == A_MTIE) { 
         if(adev_display_mask & DISPLAY_MTIE) {
            if(adev_display_mask & DISPLAY_CHA) scan_mtie_bins(CHA_MTIE);
            if(adev_display_mask & DISPLAY_CHB) scan_mtie_bins(CHB_MTIE);
         }
      }
   }
   else if(all_adevs == ALL_CHANS) {  // show selected adev type for all channels
      if(ATYPE == OSC_ADEV) {
         if(adev_display_mask & DISPLAY_ADEV) {
            if(adev_display_mask & DISPLAY_CHA) scan_bins(&pps_adev_bins[0]);
            if(adev_display_mask & DISPLAY_CHB) scan_bins(&osc_adev_bins[0]);
            if(adev_display_mask & DISPLAY_CHC) scan_bins(&chc_adev_bins[0]);
            if(adev_display_mask & DISPLAY_CHD) scan_bins(&chd_adev_bins[0]);
         }
      }
      else if(ATYPE == OSC_HDEV) {
         if(adev_display_mask & DISPLAY_HDEV) {
            scan_bins(&pps_hdev_bins[0]);
            scan_bins(&osc_hdev_bins[0]);
            scan_bins(&chc_hdev_bins[0]);
            scan_bins(&chd_hdev_bins[0]);
         }
      }
      else if(ATYPE == OSC_MDEV) {
         if(adev_display_mask & DISPLAY_MDEV) {
            scan_bins(&pps_mdev_bins[0]);
            scan_bins(&osc_mdev_bins[0]);
            scan_bins(&chc_mdev_bins[0]);
            scan_bins(&chd_mdev_bins[0]);
         }
      }
      else if(ATYPE == OSC_TDEV) {
         if(adev_display_mask & DISPLAY_TDEV) {
            scan_bins(&pps_tdev_bins[0]);
            scan_bins(&osc_tdev_bins[0]);
            scan_bins(&chc_tdev_bins[0]);
            scan_bins(&chd_tdev_bins[0]);
         }
      }
      else if(ATYPE == A_MTIE) {
         if(adev_display_mask & DISPLAY_MTIE) {
            if(adev_display_mask & DISPLAY_CHA) scan_mtie_bins(CHA_MTIE);
            if(adev_display_mask & DISPLAY_CHB) scan_mtie_bins(CHB_MTIE);
            if(adev_display_mask & DISPLAY_CHC) scan_mtie_bins(CHC_MTIE);
            if(adev_display_mask & DISPLAY_CHD) scan_mtie_bins(CHD_MTIE);
         }
      }
   }
   else {  // show all adev types for the selected channel
      if(all_adevs == ALL_PPS) {
         if(adev_display_mask & DISPLAY_ADEV) scan_bins(&pps_adev_bins[0]);
         if(adev_display_mask & DISPLAY_HDEV) scan_bins(&pps_hdev_bins[0]);
         if(adev_display_mask & DISPLAY_MDEV) scan_bins(&pps_mdev_bins[0]);
         if(adev_display_mask & DISPLAY_TDEV) scan_bins(&pps_tdev_bins[0]);
      }

      if(all_adevs == ALL_OSC) {
         if(adev_display_mask & DISPLAY_ADEV) scan_bins(&osc_adev_bins[0]);
         if(adev_display_mask & DISPLAY_HDEV) scan_bins(&osc_hdev_bins[0]);
         if(adev_display_mask & DISPLAY_MDEV) scan_bins(&osc_mdev_bins[0]);
         if(adev_display_mask & DISPLAY_TDEV) scan_bins(&osc_tdev_bins[0]);
      }

      if(all_adevs == ALL_CHC) {
         if(adev_display_mask & DISPLAY_ADEV) scan_bins(&chc_adev_bins[0]);
         if(adev_display_mask & DISPLAY_HDEV) scan_bins(&chc_hdev_bins[0]);
         if(adev_display_mask & DISPLAY_MDEV) scan_bins(&chc_mdev_bins[0]);
         if(adev_display_mask & DISPLAY_TDEV) scan_bins(&chc_tdev_bins[0]);
      }

      if(all_adevs == ALL_CHD) {
         if(adev_display_mask & DISPLAY_ADEV) scan_bins(&chd_adev_bins[0]);
         if(adev_display_mask & DISPLAY_HDEV) scan_bins(&chd_hdev_bins[0]);
         if(adev_display_mask & DISPLAY_MDEV) scan_bins(&chd_mdev_bins[0]);
         if(adev_display_mask & DISPLAY_TDEV) scan_bins(&chd_tdev_bins[0]);
      }
   }
//if(global_adev_min) {
//   sprintf(debug_text, "gamax:%g  gamin:%g  gdecades:%g", 
//adev_decade(global_adev_max),adev_decade(global_adev_min),adev_decade(global_adev_max)/adev_decade(global_adev_min));
//}
///global_adev_max = 1.0E-7;
}

void show_all_adevs()
{
struct ADEV_INFO bins;
COORD row,col;
char *s;
int i;
int color;
int mask;

   // show all adev tables on the screen at the same time
   adevs_shown = 0;
   adev_cols_shown = 0;
   if(adevs_active(1) == 0) return;
   find_global_max();
   mask = 0xFFFF;

   if(TEXT_HEIGHT <= 8) row = ALL_ROW+5;
   else if((TEXT_HEIGHT <= 16) && (PLOT_ROW >= 576)) row = ALL_ROW+5;
   else if((TEXT_HEIGHT <= 14) && big_plot) row = ALL_ROW+5;
   else if((TEXT_HEIGHT <= 12) && (SCREEN_WIDTH >= WIDE_WIDTH)) row = ALL_ROW+5;
   else if((TEXT_HEIGHT <= 14) && (SCREEN_WIDTH >= MEDIUM_WIDTH)) row = ALL_ROW+3;
   else {
      row = ALL_ROW;
//    if(osc_params || (osc_discipline == 0)) ++row;
      if(osc_params) ++row;
   }

   all_adev_row = row;
   adev_page_size = adev_table_rows = (adev_bottom_row - all_adev_row);
   if(adev_decades_shown && (adev_decades_shown < adev_page_size)) adev_page_size = adev_decades_shown;
//sprintf(debug_text, "arow:%d  abr:%d  aps:%d  ads:%d", ADEV_ROW, adev_bottom_row, adev_page_size, adev_decades_shown);

   col = 0;

   i = 0;
   s = "ADEV";
   if     (all_adevs == ALL_PPS)   { i = 0x01; s = "chA"; }
   else if(all_adevs == ALL_OSC)   { i = 0x02; s = "chB"; }
   else if(all_adevs == ALL_CHC)   { i = 0x04; s = "chC"; }
   else if(all_adevs == ALL_CHD)   { i = 0x08; s = "chD"; }
   else if(all_adevs == ALL_CHANS) { i = 0x0F; s = "TICC"; }

   if(text_mode) ;
   else if(rcvr_type == TICC_RCVR) {
      if((i & adevs_active(1)) == 0) {
         sprintf(out, "No %s ADEV data available   ", s);
      }
      else {
         sprintf(out, "Collecting %s data          ", s);
      }
   }

   if(zoom_screen == 0) {
      vidstr(row, col, RED, out);
   }

   // all_adevs == 1  -  OSC
   // all_adevs == 2  -  PPS
   // all_adevs == 3  -  CHC
   // all_adevs == 4  -  CHD
   // all_adevs == 5  -  all channels

   mask = DISPLAY_ADEV;
   if(all_adevs == ALL_OSC) {  // showing OSC adevs
      fetch_adev_info(OSC_ADEV, &bins);
      adev_cols_shown += show_adev_table(&bins, row, col, OSC_ADEV_COLOR, 1);
      if(mixed_adevs != MIXED_REGULAR) {
         if(adev_display_mask & DISPLAY_CHB) plot_adev_curve(&bins, OSC_ADEV_COLOR, mask);
      }
   }
   else if(all_adevs == ALL_PPS) {   // showing PPS adevs
      fetch_adev_info(PPS_ADEV, &bins);
      adev_cols_shown += show_adev_table(&bins, row, col, PPS_ADEV_COLOR, 2);
      if(mixed_adevs != MIXED_REGULAR) {
         if(adev_display_mask & DISPLAY_CHA) plot_adev_curve(&bins, PPS_ADEV_COLOR, mask);
      }
   }
   else if(all_adevs == ALL_CHC) {   // showing CHC adevs
      fetch_adev_info(CHC_ADEV, &bins);
      adev_cols_shown += show_adev_table(&bins, row, col, CHC_ADEV_COLOR, 3);
      if(mixed_adevs != MIXED_REGULAR) {
         if(adev_display_mask & DISPLAY_CHC) plot_adev_curve(&bins, CHC_ADEV_COLOR, mask);
      }
   }
   else if(all_adevs == ALL_CHD) {   // showing selected adev type for all channels
      fetch_adev_info(CHD_ADEV, &bins);
      adev_cols_shown += show_adev_table(&bins, row, col, CHD_ADEV_COLOR, 4);
      if(mixed_adevs != MIXED_REGULAR) {
         if(adev_display_mask & DISPLAY_CHD) plot_adev_curve(&bins, CHD_ADEV_COLOR, mask);
      }
   }
   else if(all_adevs == ALL_CHANS) {
      if     (ATYPE == OSC_ADEV) { fetch_adev_info(PPS_ADEV, &bins); mask = DISPLAY_ADEV; }
      else if(ATYPE == OSC_HDEV) { fetch_adev_info(PPS_HDEV, &bins); mask = DISPLAY_HDEV; } 
      else if(ATYPE == OSC_MDEV) { fetch_adev_info(PPS_MDEV, &bins); mask = DISPLAY_MDEV; } 
      else if(ATYPE == OSC_TDEV) { fetch_adev_info(PPS_TDEV, &bins); mask = DISPLAY_TDEV; } 
//    else if(ATYPE == A_MTIE)   { fetch_adev_info(A_MTIE, &bins);   mask = DISPLAY_MTIE; } 
else if(ATYPE == A_MTIE)   { fetch_adev_info(A_MTIE, &bins);   mask = DISPLAY_CHA; } 
      adev_cols_shown += show_adev_table(&bins, row, col, PPS_ADEV_COLOR, 5);
      if(mixed_adevs != MIXED_REGULAR) {
         if(adev_display_mask & DISPLAY_CHA) plot_adev_curve(&bins, PPS_ADEV_COLOR, mask);
      }
   }

   color = GREEN;
   if(SCREEN_WIDTH < NARROW_SCREEN) col = (TEXT_COLS*1)/4;
   else if(SCREEN_WIDTH < MEDIUM_WIDTH) col += 25;
   else                                 col += 32;

   mask = DISPLAY_HDEV;
   if     (all_adevs == ALL_OSC) fetch_adev_info(OSC_HDEV, &bins);
   else if(all_adevs == ALL_PPS) fetch_adev_info(PPS_HDEV, &bins);
   else if(all_adevs == ALL_CHC) fetch_adev_info(CHC_HDEV, &bins);
   else if(all_adevs == ALL_CHD) fetch_adev_info(CHD_HDEV, &bins);
   else if(all_adevs == ALL_CHANS) {  // channel B
      if     (ATYPE == OSC_ADEV) { fetch_adev_info(OSC_ADEV, &bins); mask = DISPLAY_ADEV; }  
      else if(ATYPE == OSC_HDEV) { fetch_adev_info(OSC_HDEV, &bins); mask = DISPLAY_HDEV; }  
      else if(ATYPE == OSC_MDEV) { fetch_adev_info(OSC_MDEV, &bins); mask = DISPLAY_MDEV; }  
      else if(ATYPE == OSC_TDEV) { fetch_adev_info(OSC_TDEV, &bins); mask = DISPLAY_TDEV; }  
//    else if(ATYPE == A_MTIE)   { fetch_adev_info(B_MTIE, &bins);   mask = DISPLAY_MTIE; } 
else if(ATYPE == A_MTIE)   { fetch_adev_info(B_MTIE, &bins);   mask = DISPLAY_CHB; } 
      color = OSC_ADEV_COLOR;
   }
   adev_cols_shown += show_adev_table(&bins, row, col, color, 6);
   if(mixed_adevs != MIXED_REGULAR) {
      if(adev_display_mask & DISPLAY_CHB) plot_adev_curve(&bins, color, mask);
   }

   if(SCREEN_WIDTH >= WIDE_WIDTH) col = FILTER_COL+20;
   else                           col = (TEXT_COLS*2)/4;
   mask = DISPLAY_MDEV;
   if     (all_adevs == ALL_OSC) fetch_adev_info(OSC_MDEV, &bins);
   else if(all_adevs == ALL_PPS) fetch_adev_info(PPS_MDEV, &bins);
   else if(all_adevs == ALL_CHC) fetch_adev_info(CHC_MDEV, &bins);
   else if(all_adevs == ALL_CHD) fetch_adev_info(CHD_MDEV, &bins);
   else if(all_adevs == ALL_CHANS) {
      if     (ATYPE == OSC_ADEV) { fetch_adev_info(CHC_ADEV, &bins); mask = DISPLAY_ADEV; }  
      else if(ATYPE == OSC_HDEV) { fetch_adev_info(CHC_HDEV, &bins); mask = DISPLAY_HDEV; }  
      else if(ATYPE == OSC_MDEV) { fetch_adev_info(CHC_MDEV, &bins); mask = DISPLAY_MDEV; }  
      else if(ATYPE == OSC_TDEV) { fetch_adev_info(CHC_TDEV, &bins); mask = DISPLAY_TDEV; }  
//    else if(ATYPE == A_MTIE)   { fetch_adev_info(C_MTIE, &bins);   mask = DISPLAY_MTIE; } 
      else if(ATYPE == A_MTIE)   { fetch_adev_info(C_MTIE, &bins);   mask = DISPLAY_CHC; } 
   }
   adev_cols_shown += show_adev_table(&bins, row, col, MAGENTA, 7);
   if(mixed_adevs != MIXED_REGULAR) {
      if(adev_display_mask & DISPLAY_CHC) plot_adev_curve(&bins, MAGENTA, mask);
   }

   if(SCREEN_WIDTH >= WIDE_WIDTH) col += 32;
   else                           col = (TEXT_COLS*3)/4;
   mask = DISPLAY_TDEV;
   if     (all_adevs == ALL_OSC) fetch_adev_info(OSC_TDEV, &bins);
   else if(all_adevs == ALL_PPS) fetch_adev_info(PPS_TDEV, &bins);
   else if(all_adevs == ALL_CHC) fetch_adev_info(CHC_TDEV, &bins);
   else if(all_adevs == ALL_CHD) fetch_adev_info(CHD_TDEV, &bins);
   else if(all_adevs == ALL_CHANS) {
      if     (ATYPE == OSC_ADEV) { fetch_adev_info(CHD_ADEV, &bins); mask = DISPLAY_ADEV; }  
      else if(ATYPE == OSC_HDEV) { fetch_adev_info(CHD_HDEV, &bins); mask = DISPLAY_HDEV; }  
      else if(ATYPE == OSC_MDEV) { fetch_adev_info(CHD_MDEV, &bins); mask = DISPLAY_MDEV; }  
      else if(ATYPE == OSC_TDEV) { fetch_adev_info(CHD_TDEV, &bins); mask = DISPLAY_TDEV; }  
//    else if(ATYPE == A_MTIE)   { fetch_adev_info(D_MTIE, &bins);   mask = DISPLAY_MTIE; } 
      else if(ATYPE == A_MTIE)   { fetch_adev_info(D_MTIE, &bins);   mask = DISPLAY_CHD; } 
   }
   adev_cols_shown += show_adev_table(&bins, row, col, YELLOW, 8);

   if(mixed_adevs != MIXED_REGULAR) {
      if(adev_display_mask & DISPLAY_CHD) plot_adev_curve(&bins, YELLOW, mask);
   }
}

void show_adev_info(int why)
{
int row;
int mask;

   // draw the adev tables and graphs
   adev_show_time = 0;
   adevs_shown = 0;
   if(adevs_active(1) == 0) return;
   if(luxor) return;

   mask = 0xFFFF;
   if(all_adevs) {
      if(mixed_adevs == MIXED_REGULAR) {  // show 4 adev tables, PPS and OSC curves
         show_all_adevs();
         if(adev_display_mask & DISPLAY_CHA) plot_adev_curve(&pps_bins, PPS_ADEV_COLOR, mask);
         if(adev_display_mask & DISPLAY_CHB) plot_adev_curve(&osc_bins, OSC_ADEV_COLOR, mask);
      }
      else {  // show all 4 adev tables and matching curves
         show_all_adevs();
      }
   }
   else {  // show OSC and PPS tables and curves
      find_global_max();
      row = ADEV_ROW+0;
      if(pps_adev_q_count && osc_adev_q_count) ;
      else adev_page_size = adev_table_rows = (adev_bottom_row - row) - 1;

      if(pps_adev_q_count) {  // we have pps adevs
         if(continuous_scroll) {
            if     (ATYPE == OSC_ADEV) { fetch_adev_info(PPS_ADEV, &pps_bins); mask = DISPLAY_ADEV; }
            else if(ATYPE == OSC_HDEV) { fetch_adev_info(PPS_HDEV, &pps_bins); mask = DISPLAY_HDEV; } 
            else if(ATYPE == OSC_MDEV) { fetch_adev_info(PPS_MDEV, &pps_bins); mask = DISPLAY_MDEV; } 
            else if(ATYPE == OSC_TDEV) { fetch_adev_info(PPS_TDEV, &pps_bins); mask = DISPLAY_TDEV; } 
            else if(ATYPE == A_MTIE)   { fetch_adev_info(A_MTIE, &pps_bins); mask = DISPLAY_MTIE; } 
         }
         show_adev_table(&pps_bins, row, ADEV_COL, PPS_ADEV_COLOR, 9);
         if(adev_display_mask & DISPLAY_CHA) plot_adev_curve(&pps_bins, PPS_ADEV_COLOR, mask);
      }


      if(pps_adev_q_count && osc_adev_q_count) {  // we have osc and pps adevs
         row = (adev_bottom_row - ADEV_ROW);  // split the adev area in half adev_page_size
         row = ADEV_ROW + (row/2);
         adev_page_size = adev_table_rows = (adev_bottom_row - row);
         if(adev_decades_shown && (adev_decades_shown < adev_page_size)) adev_page_size = adev_decades_shown;
         adev_page_size &= (~1);
         adev_table_rows &= (~1);
      }
//sprintf(debug_text, "arow:%d  abr:%d  aps:%d  ads:%d", ADEV_ROW, adev_bottom_row, adev_page_size, adev_decades_shown);

      if(osc_adev_q_count) {  // we have osc adevs to show
         if(continuous_scroll) {
            if     (ATYPE == OSC_ADEV) { fetch_adev_info(OSC_ADEV, &osc_bins); mask = DISPLAY_ADEV; }  
            else if(ATYPE == OSC_HDEV) { fetch_adev_info(OSC_HDEV, &osc_bins); mask = DISPLAY_HDEV; }  
            else if(ATYPE == OSC_MDEV) { fetch_adev_info(OSC_MDEV, &osc_bins); mask = DISPLAY_MDEV; }  
            else if(ATYPE == OSC_TDEV) { fetch_adev_info(OSC_TDEV, &osc_bins); mask = DISPLAY_TDEV; }  
            else if(ATYPE == A_MTIE)   { fetch_adev_info(B_MTIE,   &osc_bins); mask = DISPLAY_MTIE; }  
         }
         show_adev_table(&osc_bins, row, ADEV_COL, OSC_ADEV_COLOR, 10);
         if(adev_display_mask & DISPLAY_CHB) plot_adev_curve(&osc_bins, OSC_ADEV_COLOR, mask);
      }
   }
//sprintf(debug_text, "gamax:%g  gamin:%g    ", global_adev_max,global_adev_min);
}

void update_adev_display(int type, int force)
{
int bin_count;
u08 new_adev_info;
u08 need_new_plot;

   // redraw the adev displays if it is time to do so
   if(adevs_active(1) == 0) return;

// if(type == A_MTIE) type = OSC_ADEV;

   new_adev_info = 0;
   need_new_plot = 0;

   if(osc_adev_q_count && (++osc_adev_disp >= ADEV_DISPLAY_RATE)) {
      osc_adev_disp = 0;
      bin_count = fetch_adev_info(type+NUM_ADEV_TYPES*0, &osc_bins);

      if(bin_count > last_osc_bin_count) {  // force redraw whenever a new bin fills
         need_new_plot |= 0x02;
         last_osc_bin_count = bin_count;
      }
      new_adev_info |= 0x02;
   }

   if(pps_adev_q_count && (++pps_adev_disp >= ADEV_DISPLAY_RATE)) {
      pps_adev_disp = 0;
      if(all_adevs) new_adev_info = 1;

      bin_count = fetch_adev_info(type+NUM_ADEV_TYPES*1, &pps_bins);

      if(bin_count > last_pps_bin_count) {  // force redraw whenever a new bin fills
         need_new_plot |= 0x01;
         last_pps_bin_count = bin_count;
      }
      new_adev_info |= 0x01;
   }


   if(chc_adev_q_count && (++chc_adev_disp >= ADEV_DISPLAY_RATE)) {   // aaabbb what about chc
      chc_adev_disp = 0;
      bin_count = fetch_adev_info(type+NUM_ADEV_TYPES*2, &chc_bins);

      if(bin_count > last_chc_bin_count) {  // force redraw whenever a new bin fills
         need_new_plot |= 0x04;
         last_chc_bin_count = bin_count;
      }
      new_adev_info |= 0x04;
   }


   if(chd_adev_q_count && (++chd_adev_disp >= ADEV_DISPLAY_RATE)) {   // aaabbb what about chc
      chd_adev_disp = 0;
      bin_count = fetch_adev_info(type+NUM_ADEV_TYPES*3, &chd_bins);

      if(bin_count > last_chd_bin_count) {  // force redraw whenever a new bin fills
         need_new_plot |= 0x08;
         last_chd_bin_count = bin_count;
      }
      new_adev_info |= 0x08;
   }

   if(need_new_plot || force) {
      draw_plot(NO_REFRESH);
   }

   if(new_adev_info || force || adev_freshened) { 
      if(force || (continuous_scroll == 0)) {
         show_adev_info(5);      // aaaahhhh
      }
      adev_freshened = 0;
   }
}

void force_adev_redraw(int why)
{
    // force a redraw of the the adev tables and graphs
    have_time = 0;
    pps_adev_disp = ADEV_DISPLAY_RATE;
    osc_adev_disp = ADEV_DISPLAY_RATE;
    chc_adev_disp = ADEV_DISPLAY_RATE;
    chd_adev_disp = ADEV_DISPLAY_RATE;

    max_adev_width = 0;    // recalc table sizes
    adev_page_size = 0;
    adev_table_rows = 0;

    update_adev_display(ATYPE, 1);
}


void write_log_adevs(struct ADEV_INFO *bins)
{
int i;
char *d;
char *t;
long adev_q_count;
char adev_id[32];
double period;

   // write an adev table to the log file
   if(log_file == 0) return;
   if(log_comments == 0) return;

   i = bins->adev_type / NUM_ADEV_TYPES;

   if(i == CHD_ID) {
      d = "chD";
   }
   else if(i == CHC_ID) { 
      d = "chC";
   }
   else if(i == PPS_ID) { 
      if(jitter_adev)    d = "Mag jit"; //"PPS";
      else if(TICC_USED) d = "chA";
      else               d = plot[PPS].plot_id; //"PPS";
   }
   else {
      if(jitter_adev)    d = "Msg ofs"; //"OSC";
      else if(TICC_USED) d = "chB";
      else               d = plot[OSC].plot_id; //"OSC";
   }

   period = adev_period;
   if     (bins->adev_type == PPS_ADEV) { t = "ADEV"; adev_q_count = pps_adev_q_count+(long) pps_adev_q_overflow; period = pps_adev_period; } 
   else if(bins->adev_type == OSC_ADEV) { t = "ADEV"; adev_q_count = osc_adev_q_count+(long) osc_adev_q_overflow; period = osc_adev_period; } 
   else if(bins->adev_type == CHC_ADEV) { t = "ADEV"; adev_q_count = chc_adev_q_count+(long) chc_adev_q_overflow; period = chc_adev_period; } 
   else if(bins->adev_type == CHD_ADEV) { t = "ADEV"; adev_q_count = chd_adev_q_count+(long) chd_adev_q_overflow; period = chd_adev_period; } 

   else if(bins->adev_type == PPS_MDEV) { t = "MDEV"; adev_q_count = pps_adev_q_count+(long) pps_adev_q_overflow; period = pps_adev_period; } 
   else if(bins->adev_type == OSC_MDEV) { t = "MDEV"; adev_q_count = osc_adev_q_count+(long) osc_adev_q_overflow; period = osc_adev_period; } 
   else if(bins->adev_type == CHC_MDEV) { t = "MDEV"; adev_q_count = chc_adev_q_count+(long) chc_adev_q_overflow; period = chc_adev_period; } 
   else if(bins->adev_type == CHD_MDEV) { t = "MDEV"; adev_q_count = chd_adev_q_count+(long) chd_adev_q_overflow; period = chd_adev_period; } 

   else if(bins->adev_type == PPS_HDEV) { t = "HDEV"; adev_q_count = pps_adev_q_count+(long) pps_adev_q_overflow; period = pps_adev_period; } 
   else if(bins->adev_type == OSC_HDEV) { t = "HDEV"; adev_q_count = osc_adev_q_count+(long) osc_adev_q_overflow; period = osc_adev_period; } 
   else if(bins->adev_type == CHC_HDEV) { t = "HDEV"; adev_q_count = chc_adev_q_count+(long) chc_adev_q_overflow; period = chc_adev_period; } 
   else if(bins->adev_type == CHD_HDEV) { t = "HDEV"; adev_q_count = chd_adev_q_count+(long) chd_adev_q_overflow; period = chd_adev_period; } 

   else if(bins->adev_type == PPS_TDEV) { t = "TDEV"; adev_q_count = pps_adev_q_count+(long) pps_adev_q_overflow; period = pps_adev_period; } 
   else if(bins->adev_type == OSC_TDEV) { t = "TDEV"; adev_q_count = osc_adev_q_count+(long) osc_adev_q_overflow; period = osc_adev_period; } 
   else if(bins->adev_type == CHC_TDEV) { t = "TDEV"; adev_q_count = chc_adev_q_count+(long) chc_adev_q_overflow; period = chc_adev_period; } 
   else if(bins->adev_type == CHD_TDEV) { t = "TDEV"; adev_q_count = chd_adev_q_count+(long) chd_adev_q_overflow; period = chd_adev_period; } 

   else if(bins->adev_type == A_MTIE)   { t = "MTIE"; adev_q_count = mtie_intervals[CHA_MTIE][0]; period = pps_adev_period; } 
   else if(bins->adev_type == B_MTIE)   { t = "MTIE"; adev_q_count = mtie_intervals[CHB_MTIE][0]; period = osc_adev_period; } 
   else if(bins->adev_type == C_MTIE)   { t = "MTIE"; adev_q_count = mtie_intervals[CHC_MTIE][0]; period = chc_adev_period; } 
   else if(bins->adev_type == D_MTIE)   { t = "MTIE"; adev_q_count = mtie_intervals[CHD_MTIE][0]; period = chd_adev_period; } 
   else { t="????"; adev_q_count = 0; }

   strcpy(adev_id, t);
   if(TICC_USED == 0) strlwr(adev_id);  // show bogo-adevs in lower case
   else if(ticc_type == LARS_TICC) strlwr(adev_id);

   sprintf(log_text, "#");
   write_log_comment(1);

   sprintf(log_text, "#  %s %s for %lu points - sample period=%.3f secs  - bin count:%d", 
      d, adev_id, adev_q_count, period, bins->bin_count);   // aaaapppp other periods 
   write_log_comment(1);

   for(i=0; i<bins->bin_count; i++) {
      sprintf(log_text, "# %10.3f tau  %.4le (n=%ld)", 
      bins->adev_taus[i], bins->adev_bins[i], bins->adev_on[i]);
      write_log_comment(1);
   }
}

void log_adevs()
{
struct ADEV_INFO bins;

   // write all adev tables to the log file
   if(luxor) return;
   if(log_file == 0) return;

   if(adevs_active(1) == 0) return;

   if((log_stream & LOG_HEX_STREAM) && (kol > 0)) {
      fprintf(log_file, "\n");
      if(log_flush_mode && log_file) fflush(log_file);
   }

   if(adevs_active(1) & 0x01) {
      fetch_adev_info(PPS_ADEV, &bins);
      write_log_adevs(&bins);  

      fetch_adev_info(PPS_HDEV, &bins);
      write_log_adevs(&bins);  

      fetch_adev_info(PPS_MDEV, &bins);
      write_log_adevs(&bins);  

      fetch_adev_info(PPS_TDEV, &bins);
      write_log_adevs(&bins);  

      sprintf(log_text, "#");
      write_log_comment(1);

      sprintf(log_text, "#");
      write_log_comment(1);
   }


   if(adevs_active(1) & 0x02) {
      fetch_adev_info(OSC_ADEV, &bins);
      write_log_adevs(&bins);

      fetch_adev_info(OSC_HDEV, &bins);
      write_log_adevs(&bins);

      fetch_adev_info(OSC_MDEV, &bins);
      write_log_adevs(&bins);

      fetch_adev_info(OSC_TDEV, &bins);
      write_log_adevs(&bins);

      sprintf(log_text, "#");
      write_log_comment(1);

      sprintf(log_text, "#");
      write_log_comment(1);
   }


   if(adevs_active(1) & 0x04) {
      fetch_adev_info(CHC_ADEV, &bins);
      write_log_adevs(&bins);

      fetch_adev_info(CHC_HDEV, &bins);
      write_log_adevs(&bins);

      fetch_adev_info(CHC_MDEV, &bins);
      write_log_adevs(&bins);

      fetch_adev_info(CHC_TDEV, &bins);
      write_log_adevs(&bins);

      sprintf(log_text, "#");
      write_log_comment(1);

      sprintf(log_text, "#");
      write_log_comment(1);
   }


   if(adevs_active(1) & 0x08) {
      fetch_adev_info(CHD_ADEV, &bins);
      write_log_adevs(&bins);

      fetch_adev_info(CHD_HDEV, &bins);
      write_log_adevs(&bins);

      fetch_adev_info(CHD_MDEV, &bins);
      write_log_adevs(&bins);

      fetch_adev_info(CHD_TDEV, &bins);
      write_log_adevs(&bins);

      sprintf(log_text, "#");
      write_log_comment(1);

      sprintf(log_text, "#");
      write_log_comment(1);
   }

   if(mtie_q_count[CHA_MTIE]) {
      fetch_mtie_info(CHA_MTIE, &bins);
      write_log_adevs(&bins);
   }

   if(mtie_q_count[CHB_MTIE]) {
      fetch_mtie_info(CHB_MTIE, &bins);
      write_log_adevs(&bins);
   }

   if(mtie_q_count[CHC_MTIE]) {
      fetch_mtie_info(CHC_MTIE, &bins);
      write_log_adevs(&bins);
   }

   if(mtie_q_count[CHD_MTIE]) {
      fetch_mtie_info(CHD_MTIE, &bins);
      write_log_adevs(&bins);
   }

//fprintf(log_file, "active:%d  qin:%d qout:%d count:%d\n", adevs_active(1), adev_q_in,adev_q_out,adev_q_count);  // aaaaaa
//int i;
//for(i=0; i<adev_q_count; i++) {
//   fprintf(log_file, "%d: %.12f %.12f %.12f\n", i, adev_q[i].osc, adev_q[i].pps, adev_q[i].chc);
//}
}

#endif // ADEV_STUFF

#ifdef GIF_FILES

//
// .GIF stuff from GraphApp
//
#pragma pack(1)  // Do NOT allow compiler to reorder structs!

typedef unsigned char      byte;
typedef unsigned long      Char;

typedef struct Colour      Color;
typedef struct Colour      Colour;

struct Colour {
 byte  alpha;    /* transparency, 0=opaque, 255=transparent */
 byte  red;      /* intensity, 0=black, 255=bright red */
 byte  green;    /* intensity, 0=black, 255=bright green */
 byte  blue;     /* intensity, 0=black, 255=bright blue */
};

typedef struct {
    int      length;
    Colour * colours;
} GifPalette;

typedef struct {
    int          width, height;
    int          has_cmap, color_res, sorted, cmap_depth;
    int          bgcolour, aspect;
    GifPalette * cmap;
} GifScreen;

typedef struct {
    int              left, top, width, height;
    int              has_cmap, interlace, sorted, reserved, cmap_depth;
    GifPalette *     cmap;
} GifPicture;

typedef struct {
    int             byte_count;
    unsigned char * bytes;
} GifData;

typedef struct {
    int        marker;
    int        data_count;
    GifData ** data;
} GifExtension;

typedef struct {
    int            intro;
    GifPicture *   pic;
    GifExtension * ext;
} GifBlock;

typedef struct {
    char        header[8];
    GifScreen * screen;
    int         block_count;
    GifBlock ** blocks;
} Gif;

/*
 *  Gif internal definitions:
 */

#define LZ_MAX_CODE     4095    /* Largest 12 bit code */
#define LZ_BITS         12

#define FLUSH_OUTPUT    4096    /* Impossible code = flush */
#define FIRST_CODE      4097    /* Impossible code = first */
#define NO_SUCH_CODE    4098    /* Impossible code = empty */

#define HT_KEY_MASK     0x1FFF  /* 13 bit key mask */

#define IMAGE_LOADING   0       /* file_state = processing */
#define IMAGE_SAVING    0       /* file_state = processing */
#define IMAGE_COMPLETE  1       /* finished reading or writing */

typedef struct {
    FILE *file;
    int depth,
        clear_code, eof_code,
        running_code, running_bits,
        max_code_plus_one,
        prev_code, current_code,
        stack_ptr,
        shift_state;
    unsigned long shift_data;
    unsigned long pixel_count;
    int           file_state, position, bufsize;
    unsigned char buf[256];
} GifEncoder;

//****************************************************************************
//
// GIF routines from GraphApp
//
//****************************************************************************

#define rgb(r,g,b)  app_new_rgb((r),(g),(b))

int  write_byte(FILE *file, int ch);
void write_gif_byte(FILE *file, GifEncoder *encoder, int ch);
void write_gif_int(FILE *file, int output);
void write_gif_palette(FILE *file, GifPalette *cmap);
void write_gif_header(FILE *file, Gif *gif);
void write_gif_code(FILE *file, GifEncoder *encoder, int code);
void init_gif_encoder(FILE *file, GifEncoder *encoder, int depth);
void write_gif_line(FILE *file, GifEncoder *encoder, int line, int length);
void flush_gif_encoder(FILE *file, GifEncoder *encoder);
void write_gif_picture(FILE *file, int top_line, GifPicture *pic);
void write_gif(FILE *file, Gif *gif);
int  write_gif_file(char *filename, Gif *gif);

//
//  Gif data structures  (no longer dynamically allocated/freed)
//
GifEncoder  encoder_data;
GifPalette  palette_data;
GifScreen   screen_data;
GifPicture  pic_data;
GifBlock    block_data;
Gif         gif_data;

/*
 *  Hash table:
 */

/*
 *  The 32 bits contain two parts: the key & code:
 *  The code is 12 bits since the algorithm is limited to 12 bits
 *  The key is a 12 bit prefix code + 8 bit new char = 20 bits.
 */
#define HT_GET_KEY(x)   ((x) >> 12)
#define HT_PUT_KEY(x)   ((x) << 12)
#define HT_GET_CODE(x)  ((x) & 0x0FFF)
#define HT_PUT_CODE(x)  ((x) & 0x0FFF)

#define get_hash_table(i)   hash_table[i]
#define put_hash_table(i,j) hash_table[i]=(j)

/*
 *  Generate a hash key from the given unique key.
 *  The given key is assumed to be 20 bits as follows:
 *    lower 8 bits are the new postfix character,
 *    the upper 12 bits are the prefix code.
 */
static int gif_hash_key(unsigned long key)
{
   return ((key >> 12) ^ key) & HT_KEY_MASK;
}

/*
 *  Reset the hash_table to an empty state.
 */
static void clear_gif_hash_table()
{
int i;

   for(i=0; i<HT_SIZE; i++) {
      put_hash_table(i, 0xFFFFFFFFL);
   }
}

/*
 *  Insert a new item into the hash_table.
 *  The data is assumed to be new.
 */
static void add_gif_hash_entry(unsigned long key, int code)
{
int hkey;

   hkey = gif_hash_key(key);
   while(HT_GET_KEY(get_hash_table(hkey)) != 0xFFFFFL) {
      hkey = (hkey + 1) & HT_KEY_MASK;
   }
   put_hash_table(hkey, HT_PUT_KEY(key) | HT_PUT_CODE(code));
}

/*
 *  Determine if given key exists in hash_table and if so
 *  returns its code, otherwise returns -1.
 */
static int lookup_gif_hash(unsigned long key)
{
int hkey;
unsigned long htkey;

   hkey = gif_hash_key(key);
   while((htkey = HT_GET_KEY(get_hash_table(hkey))) != 0xFFFFFL) {
      if(key == htkey) return HT_GET_CODE(get_hash_table(hkey));
      hkey = (hkey + 1) & HT_KEY_MASK;
   }
   return (-1);
}

/*
 *   Initialise the encoder, given a GifPalette depth.
 */
void init_gif_encoder(FILE *file, GifEncoder *encoder, int depth)
{
int lzw_min;

   lzw_min = depth = (depth < 2 ? 2 : depth);
   encoder->file_state   = IMAGE_SAVING;
   encoder->position     = 0;
   encoder->bufsize      = 0;
   encoder->buf[0]       = 0;
   encoder->depth        = depth;
   encoder->clear_code   = (1 << depth);
   encoder->eof_code     = encoder->clear_code + 1;
   encoder->running_code = encoder->eof_code + 1;
   encoder->running_bits = depth + 1;
   encoder->max_code_plus_one = 1 << encoder->running_bits;
   encoder->current_code = FIRST_CODE;
   encoder->shift_state  = 0;
   encoder->shift_data   = 0;

   /* Write the LZW minimum code size: */
   write_byte(file, lzw_min);

   /* Clear hash table, output Clear code: */
   clear_gif_hash_table();
   write_gif_code(file, encoder, encoder->clear_code);
}

void flush_gif_encoder(FILE *file, GifEncoder *encoder)
{
   write_gif_code(file, encoder, encoder->current_code);
   write_gif_code(file, encoder, encoder->eof_code);
   write_gif_code(file, encoder, FLUSH_OUTPUT);
}

/*
 *  Write a Gif code word to the output file.
 *
 *  This function packages code words up into whole bytes
 *  before writing them. It uses the encoder to store
 *  codes until enough can be packaged into a whole byte.
 */
void write_gif_code(FILE *file, GifEncoder *encoder, int code)
{
   if(code == FLUSH_OUTPUT) {
      /* write all remaining data */
      while(encoder->shift_state > 0) {
         write_gif_byte(file, encoder, encoder->shift_data & 0xff);
         encoder->shift_data >>= 8;
         encoder->shift_state -= 8;
      }
      encoder->shift_state = 0;
      write_gif_byte(file, encoder, FLUSH_OUTPUT);
   }
   else {
      encoder->shift_data |= ((long) code) << encoder->shift_state;
      encoder->shift_state += encoder->running_bits;

      while(encoder->shift_state >= 8) { /* write full bytes */
         write_gif_byte(file, encoder, encoder->shift_data & 0xff);
         encoder->shift_data >>= 8;
         encoder->shift_state -= 8;
      }
   }

   /* If code can't fit into running_bits bits, raise its size.
    * Note that codes above 4095 are for signalling. */
   if((encoder->running_code >= encoder->max_code_plus_one) && (code < 4096)) {
      encoder->max_code_plus_one = (1 << ++encoder->running_bits);
   }
}


//
//
//   Basic item I/O routines
//
//
int write_byte(FILE *file, int ch)
{
   if(file == 0) return 0;
   return putc(ch, file);
}

int write_stream(FILE *file, unsigned char buffer[], int length)
{
   if(file == 0) return 0;
   return fwrite(buffer, 1, length, file);
}

void write_gif_int(FILE *file, int output)
{
   if(file == 0) return;
   putc((output & 0xff), file);
   putc((((unsigned int) output) >> 8) & 0xff, file);
}


/*
 *  Write a byte to a Gif file.
 *
 *  This function is aware of Gif block structure and buffers
 *  chars until 255 can be written, writing the size byte first.
 *  If FLUSH_OUTPUT is the char to be written, the buffer is
 *  written and an empty block appended.
 */
void write_gif_byte(FILE *file, GifEncoder *encoder, int ch)
{
unsigned char *buf;

   buf = encoder->buf;

   if(encoder->file_state == IMAGE_COMPLETE) return;

   if(ch == FLUSH_OUTPUT) {
      if(encoder->bufsize) {
         write_byte(file, encoder->bufsize);
         write_stream(file, buf, encoder->bufsize);
         encoder->bufsize = 0;
      }
      /* write an empty block to mark end of data */
      write_byte(file, 0);
      encoder->file_state = IMAGE_COMPLETE;
   }
   else {
      if(encoder->bufsize == 255) {
         /* write this buffer to the file */
         write_byte(file, encoder->bufsize);
         write_stream(file, buf, encoder->bufsize);
         encoder->bufsize = 0;
      }
      buf[encoder->bufsize++] = ch;
   }
}

/*
 *  Write one scanline of pixels out to the Gif file,
 *  compressing that line using LZW into a series of codes.
 */
void write_gif_line(FILE *file, GifEncoder *encoder, int line, int length)
{
int i, current_code, new_code;
unsigned long new_key;
unsigned char pixval;

    i = 0;

    if(encoder->current_code == FIRST_CODE) current_code = get_pixel(i++, line);
    else current_code = encoder->current_code;

    while(i<length) {
       pixval = get_pixel(i++, line); /* Fetch next pixel from screen */

       /* Form a new unique key to search hash table for the code
        * Combines current_code as prefix string with pixval as
        * postfix char */
       new_key = (((unsigned long) current_code) << 8) + pixval;
       if((new_code = lookup_gif_hash(new_key)) >= 0) {
           /* This key is already there, or the string is old,
            * so simply take new code as current_code */
           current_code = new_code;
       }
       else {
           /* Put it in hash table, output the prefix code,
            * and make current_code equal to pixval */
           write_gif_code(file, encoder, current_code);
           current_code = pixval;

           /* If the hash_table if full, send a clear first
            * then clear the hash table: */
           if(encoder->running_code >= LZ_MAX_CODE) {
              write_gif_code(file, encoder, encoder->clear_code);
              encoder->running_code = encoder->eof_code + 1;
              encoder->running_bits = encoder->depth + 1;
              encoder->max_code_plus_one = 1 << encoder->running_bits;
              clear_gif_hash_table();
           }
           else {
              /* Put this unique key with its relative code in hash table */
              add_gif_hash_entry(new_key, encoder->running_code++);
           }
       }
    }

    /* Preserve the current state of the compression algorithm: */
    encoder->current_code = current_code;
}

/*
 *  GifPicture:
 */

void write_gif_header(FILE *file, Gif *gif)
{
unsigned char info;
GifScreen *screen;
GifPalette *cmap;
int i;
Colour c;

   if(file == 0) return;
   fprintf(file, "%s", gif->header);  // header info

   // image description
   screen = gif->screen;
   cmap = screen->cmap;
   write_gif_int(file, screen->width);
   write_gif_int(file, screen->height);

   info = 0;
   info = info | (screen->has_cmap ? 0x80 : 0x00);
   info = info | ((screen->color_res - 1) << 4);
// info = info | (screen->sorted ? 0x08 : 0x00);
   if(screen->cmap_depth > 0) {
      info = info | ((screen->cmap_depth) - 1);
   }
   write_byte(file, info);

   write_byte(file, screen->bgcolour);
   write_byte(file, screen->aspect);

   if(screen->has_cmap) {   // palette data
      for(i=0; i<cmap->length; i++) {
         c = cmap->colours[i];
         write_byte(file, c.red);
         write_byte(file, c.green);
         write_byte(file, c.blue);
      }
   }
}

void write_gif_picture(FILE *file, int top_line, GifPicture *pic)
{
unsigned char info;
GifEncoder *encoder;
int row;

   write_byte(file, 0x2C);   // intro code

   write_gif_int(file, pic->left);
   write_gif_int(file, pic->top);
   write_gif_int(file, pic->width);
   write_gif_int(file, pic->height);

   info = 0;
// info = info | (pic->has_cmap    ? 0x80 : 0x00);
// info = info | (pic->interlace   ? 0x40 : 0x00);
// info = info | (pic->sorted      ? 0x20 : 0x00);
   info = info | ((pic->reserved << 3) & 0x18);
   if(pic->has_cmap) info = info | (pic->cmap_depth - 1);

   write_byte(file, info);

   encoder = &encoder_data;
   init_gif_encoder(file, encoder, pic->cmap_depth);

   row = 0;
   while(row < pic->height) {  // !!!!! kludge: top_line should be passed to this routine
     update_pwm();
     write_gif_line(file, encoder, row+top_line, pic->width);
     row += 1;
   }

   flush_gif_encoder(file, encoder);

   write_byte(file, 0x3B);   // end code
}

Colour app_new_rgb(int r, int g, int b)
{
Colour c;

   c.alpha  = 0;
   c.red    = r;
   c.green  = g;
   c.blue   = b;

   return c;
}

//****************************************************************************
//
// Write contents of screen to .GIF file, returning 1 if OK or 0 on error 
//
//****************************************************************************


int dump_gif_file(int invert, int top_line, FILE *file)
{
S32 i;
Gif *gif;
GifPalette *cmap;
GifPicture *pic;
#define BPP 4   // 4 bits per pixel = 16 colors
u08 ctable[(1 << BPP) * sizeof(Colour)];

    // setup GIF header
    gif = &gif_data;
    strcpy(gif->header, "GIF87a");
    gif->blocks      = NULL;
    gif->block_count = 0;
    gif->screen      = &screen_data;

    gif->screen->width      = SCREEN_WIDTH;
    gif->screen->height     = SCREEN_HEIGHT-top_line;
    gif->screen->has_cmap   = 1;
    gif->screen->color_res  = BPP;
    gif->screen->cmap_depth = BPP;
   
    // setup palette color map
    screen_data.cmap = &palette_data;
    cmap = gif->screen->cmap;
    cmap->length = (1 << BPP);
    cmap->colours = (Colour *) &ctable[0];

    for(i=0; i<(1 << BPP); i++) {  // clear all the palette table
       cmap->colours[i] = rgb(0, 0, 0);
    }
    for(i=0; i<16; i++) {   // define the colors that we use
       cmap->colours[i] = rgb(bmp_pal[i*4+2], bmp_pal[i*4+1], bmp_pal[i*4+0]);
       if(invert) {
          if     (i == BLACK)  cmap->colours[i] = rgb(bmp_pal[WHITE*4+2], bmp_pal[WHITE*4+1], bmp_pal[WHITE*4+0]);
          else if(i == WHITE)  cmap->colours[i] = rgb(bmp_pal[BLACK*4+2], bmp_pal[BLACK*4+1], bmp_pal[BLACK*4+0]);
          else if(i == YELLOW) cmap->colours[i] = rgb(bmp_pal[BMP_YELLOW*4+2], bmp_pal[BMP_YELLOW*4+1], bmp_pal[BMP_YELLOW*4+0]);
       }
    }

    // picture description
    pic = &pic_data;
    pic->cmap       = &palette_data;
    pic->width      = SCREEN_WIDTH;
    pic->height     = SCREEN_HEIGHT-top_line;
    pic->interlace  = 0; 
    pic->has_cmap   = 0;
    pic->cmap_depth = BPP;

    write_gif_header(file, gif);
    write_gif_picture(file, top_line, pic);

    return 1;
}
#endif  // GIF_FILES

//
//
//   Automatic oscillator parameter configuration routine
//
//


#define STABLE_TIME    5     // how many secs to stabilize after changing DAC before averaging
#define GAIN_AVG_TIME 60     // how many secs to average osc reading over
                             // must be less than SURVEY_BIN_COUNT

#define HIGH_STATE   100     // state when measuring with high dac setting
#define WAIT_STATE  1000     // state when waiting for dac to settle
#define LOW_STATE  10000     // state when measuring with low dac setting

#define OSC_TUNE_GAIN 10.0
#define DAC_STEP      0.005  // dac voltage step (+ and -)

void tune_ticc()
{
int i;
static int ticc_time = 0;
static int old_ticc_mode;
static int fudge_val = 0;
static int cha_time2_val = 0;
static int chb_time2_val = 0;
unsigned port;

   if(dac_dac == 0) {
      return;
   }

   if(rcvr_type == TICC_RCVR) port = RCVR_PORT;
   else if(ticc_port_open())  port = TICC_PORT;
   else {
      return;
   }

   if(dac_dac == 1) {  // setup ticc to calculate new FUDGE and TIME2 parameters
      for(i=NUM_PLOTS+DERIVED_PLOTS-1; i>=0; i--) {
         plot[i].show_plot = 0;
      }
      plot_adev_data = 0;
      adev_decades_shown = 0;
      plot_sat_count = 0;
      plot[FOUR].show_plot = 1;
      plot[FIVE].show_plot = 1;
      plot[SIX].show_plot = 1;
      pause_data = 0;

      old_ticc_mode = ticc_mode;

      sprintf(plot_title, "Restting TICC fudge factors.  Mode:%c", ticc_mode);
      redraw_screen();
      strcpy(edit_buffer, "0 0");
      edit_ticc_fudge();

      sprintf(plot_title, "Restting TICC TIME2 factors");
      redraw_screen();
      strcpy(edit_buffer, "0 0");
      edit_ticc_time2();

      sprintf(plot_title, "Setting TICC debug mode");
      redraw_screen();
      strcpy(edit_buffer, "D");
      edit_ticc_mode();
      redraw_screen();

      dac_dac = 2;
      ticc_time = 0;

      fudge_val = (int) fudge_a;
      cha_time2_val = (int) time2_a;
      chb_time2_val = (int) time2_b;

   }
   else if(dac_dac == 2) {   // collect data
      ++ticc_time;
      if(ticc_time < 5) {
         cha_time2_sum = chb_time2_sum = 0.0;
         cha_time2_count = chb_time2_count = 0.0;
         fudge_sum = fudge_count = 0.0;
      }
      else {
if(ticc_time == 10) {   // keep plots from bouncing around
   plot[FOUR].plot_center = (float) cha_time2_val;
   plot[FOUR].float_center = 0;
   plot[FIVE].plot_center = (float) chb_time2_val;
   plot[FIVE].float_center = 0;
   plot[SIX].plot_center = (float) fudge_val;
   plot[SIX].float_center = 0;
}
         if(fudge_count) fudge_val = (int) ((fudge_sum / fudge_count)*1.0E12);
         if(cha_time2_count) cha_time2_val = (int) ((cha_time2_sum / cha_time2_count) + 0.50);
         if(chb_time2_count) chb_time2_val = (int) ((chb_time2_sum / chb_time2_count) + 0.50);
      }

      sprintf(plot_title, "Gathering data %d of %d secs: fudge:%d    chA TIME2:%d    chB TIME2b:%d", 
         ticc_time,ticc_tune_time, fudge_val, cha_time2_val,chb_time2_val);
      if(ticc_time >= ticc_tune_time) dac_dac = 3;
   }
   else if(dac_dac >= 3) {
pause_data = 1;         // stop capturing queue data
dont_reset_queues = 1;  // keep old queue data so we can review the autotune data
      sprintf(plot_title, "Setting chA fudge to %d ps", fudge_val);
      redraw_screen();
      sprintf(edit_buffer, "%d 0", fudge_val);
      edit_ticc_fudge();

      sprintf(plot_title, "Setting TIME2 values to chA:%d  chB:%d", cha_time2_val,chb_time2_val);
      redraw_screen();
      sprintf(edit_buffer, "%d %d", cha_time2_val,chb_time2_val);
      edit_ticc_time2();
      
      sprintf(plot_title, "Restoring TICC mode to %c", old_ticc_mode);
      redraw_screen();
      sprintf(edit_buffer, "%c", old_ticc_mode);
      edit_ticc_mode();

      dac_dac = 0;
   }
}

void tune_x72()
{
int i;
static double slope;
#define DDS_RESET_DELAY 5

   if(dac_dac == 0) {  // not tuning
      return;
   }

   if(dac_dac == 1) {  // setup to calculate tuning word
      for(i=NUM_PLOTS+DERIVED_PLOTS-1; i>=0; i--) {
         plot[i].show_plot = 0;
      }
      plot_adev_data = 0;
      adev_decades_shown = 0;
      plot_sat_count = 0;
      plot[PPS].show_plot = 1;
      pause_data = 0;

      sprintf(plot_title, "Reseting DAC tune word to 0.0E-12 and TIC to 0");
      if(pps_enabled == 0) {  // PPS output must be enabled in order for TIC command to work
         user_pps_enable = 1;
         set_pps(user_pps_enable, pps_polarity,  delay_value, pps1_delay,  300.0, 4);
      }
      set_x72_discipline(0);
      set_x72_dds(0.0);
      set_x72_tic(x72_pps);

      dac_dac = 2;  // now wait for TIC change to take effect
   }
   else if(dac_dac == 2) {   // wait for TIC set to complete
      sprintf(plot_title, "Waiting for TIC set: %d", x72_tic_timer);
      if(x72_tic_timer > 0) {
         --x72_tic_timer;
      }
      else {  // start collecting data
         new_x72_interval(5);
         dac_dac = 3;
         new_queue(RESET_PLOT_Q, 36);
      }
   }
   else if(dac_dac == 3) {   // collecting data
      if(x72_pps_interval > ticc_tune_time) {  // analyze data and set DDS word
         dac_dac = 4;
      }
      else {
         i = fix_x72_tics(x72_ival);  // fixup possible wrap near 0
         x72_update_pps_list(i);      // add fixed up ival to pps list
         x72_pps_stats(0);            // calculate trend line
         slope = x72_pps_trend;
         slope /= (double) x72_pps_interval;
         slope /= (double) x72_osc;

         sprintf(plot_title, "Gathering data: %d of %d secs.  Slope:%g", 
            x72_pps_interval, ticc_tune_time, slope);
      }
   }
   else if(dac_dac == 4) {  // set the DDS tuning word
      sprintf(plot_title, "Setting DDS tune word to %g", last_x72_dds+slope);
      set_x72_dds(last_x72_dds + slope);
      set_x72_tic(x72_pps);
      mark_q_entry[1] = plot_q_in;
      BEEP(202);
      redraw_screen();

      dac_dac = 0;
   }
   else {  // should never happen
      dac_dac = 0;
   }
}

void calc_osc_gain()
{
int old_read_only;
float el_filter;

   // dac_dac is the current gain measurement state
   if(dac_dac == 0) return;  // we are not measuring

   if(rcvr_type == TICC_RCVR) {
      tune_ticc();
      return;
   }
   else if(rcvr_type == X72_RCVR) {
      tune_x72();
      return;
   }

   if(dac_dac == 1) {        // initialize dac gain test
      if(!GPSDO) goto tune_res_t;
      if(!GPSDO_TUNE_OK) goto tune_res_t;

      #ifdef OSC_CONTROL
         disable_osc_control();
      #endif
      #ifdef PRECISE_STUFF
         stop_precision_survey(2);
      #endif

      old_read_only = read_only;
      read_only = 1;
      set_el_mask(0.0F);
      read_only = old_read_only;

      hour_lats = 0;         // setup to determine median osc offset values
      hour_lons = 0;
      gain_voltage = dac_voltage; // initial (current) DAC voltage
      set_discipline_mode(SET_DIS_MODE_DISABLE);     // disable disciplining
      set_dac_voltage(gain_voltage+(DATA_SIZE) (DAC_STEP*OSC_TUNE_GAIN), 1);  // bump the osc voltage up
      ++dac_dac;
      sprintf(plot_title, "Determining OSC gain: initializing");
      title_type = OTHER;
   }
   else if(dac_dac <= STABLE_TIME) {    // wait a little while for things to settle
      ++dac_dac;
      sprintf(plot_title, "Determining OSC gain: stabilizing high");
      title_type = OTHER;
   }
   else if(dac_dac == (STABLE_TIME+1)) {    // prepare to start taking data
      dac_dac = HIGH_STATE;
   }
   else if(dac_dac < (HIGH_STATE+1+GAIN_AVG_TIME)) {   // find median osc offset value with dac set high
      hour_lats = add_to_bin((double)osc_offset, lat_hr_bins, hour_lats);
      sprintf(plot_title, "Determining OSC gain: high osc(%d)=%f", hour_lats, (float) lat_hr_bins[hour_lats/2]);
      title_type = OTHER;
      ++dac_dac;
   }
   else if(dac_dac == (HIGH_STATE+1+GAIN_AVG_TIME)) {  // prepare to measure osc with dac at low value
      sprintf(plot_title, "Determining OSC gain: stabilizing low");
      title_type = OTHER;
      set_dac_voltage(gain_voltage - (DATA_SIZE) (DAC_STEP*OSC_TUNE_GAIN), 2);
      dac_dac = WAIT_STATE;
   }
   else if(dac_dac <= (WAIT_STATE+STABLE_TIME)) {  // wait for things to settle down some
      ++dac_dac;
   }
   else if(dac_dac == (WAIT_STATE+1+STABLE_TIME)) {  // prepare to start measuring osc with dac voltage low
      dac_dac = LOW_STATE;
   }
   else if(dac_dac < (LOW_STATE+GAIN_AVG_TIME+1)) {   // calculate the median osc value with dac set low
      ++dac_dac;
      hour_lons = add_to_bin((double)osc_offset, lon_hr_bins, hour_lons);
      sprintf(plot_title, "Determining OSC gain: low(%d)=%f", hour_lons, (float) lon_hr_bins[hour_lons/2]);
      title_type = OTHER;
   }
   else {  // we are done,  calc osc params and store into eeprom
      user_osc_gain  = (float) lon_hr_bins[hour_lons/2];
      user_osc_gain -= (float) lat_hr_bins[hour_lats/2];
      user_osc_gain /= OSC_TUNE_GAIN;

      user_time_constant   = 500.0F;
      user_damping_factor  = 1.0F;    //// !!!! 0.707F;  0.800F
      user_min_volts = min_volts;
      user_max_volts = max_volts;
      user_jam_sync = jam_sync;
      user_max_freq_offset = max_freq_offset;
      user_initial_voltage = gain_voltage;
      if(saw_ntpx) initial_voltage = gain_voltage;

//    sprintf(plot_title, "OSC gain: %f Hz/V", user_osc_gain);
sprintf(plot_title, "gain:%.3f iv=%.3f min=%.3f  max=%.3f  tc=%.3f  damp=%.3f", 
user_osc_gain, user_initial_voltage, user_min_volts, user_max_volts, user_time_constant, user_damping_factor);
title_type = OTHER;

      set_discipline_params(1);       // save params into eeprom
      set_dac_voltage(gain_voltage, 3);  // restore dac voltage
      set_discipline_mode(SET_DIS_MODE_ENABLE);   // re-enable disciplining
      set_discipline_mode(SET_DIS_MODE_JAMSYNC);  // jam sync the pps back in line
      request_all_dis_params();       // display the newest settings

      tune_res_t:
//    set_el_level();                 // set elevation mask level 
//    !!!! res-t does not like back to back config commands, set both params with one operation

      el_filter = (el_mask + (float) good_el_level()) / 2.0F;

      if(res_t && (res_t != RES_T))   set_el_amu(el_filter, 25.0F);  // set signal level mask level in dBc  // !!!!! RES_T_360, etc?
      else if(rcvr_type == TSIP_RCVR) set_el_amu(el_filter, 1.0F);   // set signal level mask level in AMU
      else                            set_el_amu(el_filter, (float) good_sig_level());

      redraw_screen();
      dac_dac = 0;                    // we are done dac-ing around
   }

   show_title();
}



#ifdef TEMP_CONTROL   // stuff
//
// Active tempeature stabilization of the unit temperature:
// An easy way to do this is to use choose a control temperature above
// maximum ambient temperature and below the self heating temperature
// of the unit in a closed box.  Use the apply_cool() function to turn 
// on a fan and the apply_heat() function to turn it off.

// Heat control via the parallel port uses the upper four bits to
// enable/disable the heat controller.  A value of 9 in the upper 4
// bits says to enable the controller.  A value of 6 says to disable it.
//
// The lower 4 bits are used to select the control state:
//    9=apply heat  A=apply cool  6=hold temperature
//
// These values were chosen so that both polarities of control signal
// are available.  Also the enable/disable patterns were chosen so that
// they are unlikely to occur due to the BIOS initializing the parallel
// port (which ususally leaves a AA/55/00/FF in the port register at
// power up).

// Heat control via the serial port uses the DTR line for fan speed control
// and the RTS line for heat controller enable/disable.

#define HEAT 0     // serial port DTR line values  (-12V)
#define COOL 1     // (+12V)
#define ENABLE  0  // serial port RTS line values  (-12V)
#define DISABLE 1  // (+12V)

u08 a_heating;     // the current heating/cooling/holding state (only one should be true)
u08 a_cooling;
u08 a_holding;

DATA_SIZE this_temperature; // the current temperature reading
DATA_SIZE last_temp;        // the previous temperature reading
DATA_SIZE delta;            // the temperature error
DATA_SIZE last_delta;       // the previous temperature error
DATA_SIZE spike_temp;       // the temperature reading before a sensor misread spike

double heat_off_time, cool_off_time;   // when to update the pwm output
DATA_SIZE heat_time, cool_time;        // how many milliseconds to heat or cool for

// stuff used to analyze the raw controller response (in bang-bang mode)
double heat_on_tick, cool_on_tick, hold_on_tick;
double cycle_time, avg_cycle_time, ct_samples;
double this_cycle, last_cycle;
double fall_cycle, fall_time;
DATA_SIZE heat_sum, heat_ticks;
DATA_SIZE cool_sum, cool_ticks;

DATA_SIZE heat_rate, cool_rate;            // heating and cooling rate (deg/sec)
DATA_SIZE rate_sum, rate_count;

long low_ok, high_ok;
DATA_SIZE HEAT_OVERSHOOT;
DATA_SIZE COOL_UNDERSHOOT;



unsigned init_lpt()   /* init LPT port,  return base I/O address */
{
   if(lpt_port == 1) return 0x378;
   if(lpt_port == 2) return 0x278;
   if(lpt_port == 3) return 0x3BC;
   return 0;
}


// Warren's filter info
DATA_SIZE PID_out;          // new filter output
DATA_SIZE last_PID_out;     // previous filter output
DATA_SIZE PID_error;        // new temperature error
DATA_SIZE last_PID_error;   // previous temperature error
DATA_SIZE last_PID_display; // previous PID filter value (for debug display)

DATA_SIZE integral_step;    // integrator update factor
DATA_SIZE integrator;       // the current integrator value

// internally used PID filter constants (derived from user friendly ones)
DATA_SIZE k1;      // the proportional gain
DATA_SIZE k2;      // the derivitive time constant
DATA_SIZE k3;      // the filter time constant (scaled)
DATA_SIZE k4;      // the integrator time constant
DATA_SIZE k6;      // load distubance test tuning param
DATA_SIZE k7;      // loop gain tuning param
DATA_SIZE k8;      // integrator reset
#define loop_gain        0.0 // k7
#define integrator_reset k8

#define PID_UPDATE_RATE   1.0              // how fast the PID filter is updated in seconds
#define PWM_CYCLE_TIME    (1002.5/2.0)    // milliseconds
#define MAX_PID           0.99
#define MAX_INTEGRAL      2.0               // max integrator update step
#define SCALE_FACTOR      (1.0+loop_gain)  // !!!! ////

#define TUNE_CYCLES   7       // end autotune after this many waveform cycles
int tune_cycles;
double tune_amp[TUNE_CYCLES+1];
int    amp_ptr;
double tune_cyc[TUNE_CYCLES+1];
int    cyc_ptr;
int bang_settle_time;         // how long to wait for PID filter to settle before starting analysis
int BANG_INITS;               // number of cycles to wait before starting distubance measurments

void calc_k_factors()
{
   // This routine converts the user friendly input PID constants into the
   // "k" values used by the PID filter algorithm.  It is called whenever
   // a PID filter constant is changed by the user.
   //
   // P_GAIN        -> k1   (PID gain)
   // D_TC          -> k2   (derivative time constant)
   // FILTER_TC     -> k3   (filter time constant)
   // I_TC          -> k4   (integrator time constant)
   // FILTER_OFFSET         (temperature setpoint offset)

   if(I_TC) k4 = ((DATA_SIZE)1.0/I_TC) / (DATA_SIZE) PID_UPDATE_RATE * P_GAIN;
   else     k4 = (DATA_SIZE) 0.0;

   if(FILTER_TC > 0.0) {
      k3 = (DATA_SIZE) 1.0 - ((DATA_SIZE) 1.0/((FILTER_TC*(DATA_SIZE) PID_UPDATE_RATE)+(DATA_SIZE) 1.0));
      k2 = ((DATA_SIZE) -1.0 * D_TC * P_GAIN) / (FILTER_TC*(DATA_SIZE) PID_UPDATE_RATE+(DATA_SIZE) 1.0) * ((DATA_SIZE) PID_UPDATE_RATE/(DATA_SIZE) SCALE_FACTOR);
   }
   else {
      k3 = FILTER_TC;
      k2 = ((DATA_SIZE) -1.0 * D_TC * P_GAIN) * ((DATA_SIZE) PID_UPDATE_RATE/(DATA_SIZE) SCALE_FACTOR);
   }

   k1 = ((P_GAIN/(DATA_SIZE) SCALE_FACTOR) * ((DATA_SIZE) 1.0 - k3)) - k2;

   if(pid_debug || bang_bang) {  // set a marker each time a PID value changes
      if(++test_marker >= MAX_MARKER) test_marker = 1;
      if(test_marker < 0) test_marker = 1;
      mark_q_entry[test_marker] = plot_q_in;
   }
}

void set_default_pid(int pid_num)
{
   // standard filter constants
   FILTER_OFFSET = 0.0F;
   FILTER_TC = 2.0F;
   k6 = 0.0F;
   k7 = 0.0F;
   k8 = 1.0F;
   integrator = 0.0F;

   if(pid_num == 0) {        // Warren's tortise slow default values
      P_GAIN = 1.50F; 
      D_TC = 45.0F;
      I_TC = 180.0F;
   }
   else if(pid_num == 1) {   // Mark's medium fast hare values
      P_GAIN = 3.5F;  // 5.0F;
      D_TC = 25.0F;   // 32.0F; 
      I_TC = 100.0F;  // 120.0F 
   }
   else if(pid_num == 2) {   // Mark's fast hare values
      P_GAIN = 10.0F;  
      D_TC = 28.0;     
      I_TC = 112.0F;   
   }
   else if(pid_num == 3) {   // Mark's fast hare-on-meth values
      P_GAIN = 24.0F;  
      D_TC = 12.0;     
      I_TC = 48.0F;   
   }

   calc_k_factors();
}

void show_pid_values()
{
   if(dac_dac) return;

   sprintf(plot_title, "kP(%.2f %c %.4f) kD(%.2f %c %.4f) kF(%.2f %c %.4f) kI(%.1f %c %.3f)  kO=%.3f kL=%.3f kS=%.3f kR=%.3f k9=%.3f", 
      P_GAIN,RIGHT_ARROW,k1, 
      D_TC,RIGHT_ARROW,k2, 
      FILTER_TC,RIGHT_ARROW,k3, 
      I_TC,RIGHT_ARROW,k4, 
      FILTER_OFFSET, k6, k7, k8, KL_TUNE_STEP);
   title_type = OTHER;
}

int calc_autotune()
{
double val;

   if(HEAT_OVERSHOOT == 0.0F) return 1;
   if(COOL_UNDERSHOOT == 0.0F) return 2;
   if(ct_samples == 0.0) return 3;
   if(amp_ptr < 2) return 4;
   if(cyc_ptr < 2) return 5;

   val  = tune_amp[amp_ptr/2];
   if(amp_ptr > 2) {
      val += tune_amp[amp_ptr/2-1];
      val += tune_amp[amp_ptr/2+1];
      val /= 3.0;
   }
   if(val == 0.0) return 6;
   P_GAIN = 0.625F * (KL_TUNE_STEP+KL_TUNE_STEP) / (float) val;

   val  = tune_cyc[cyc_ptr/2];
   if(cyc_ptr > 2) {
      val += tune_cyc[cyc_ptr/2-1];
      val += tune_cyc[cyc_ptr/2+1];
      val /= 3.0;
   }
   I_TC   = 0.75F * (float) val;
   D_TC   = 0.1875F * (float) val; 

   k6 = 0.0F;
// integrator = 0.0F;

   calc_k_factors();
   clear_pid_display();
   show_pid_values();

   bang_bang = 0;    // pid autotune analysis done
   return 0;
}


void temp_pid_filter()
{
DATA_SIZE pwm_width;
char cmd[SLEN];

   #ifdef DEBUG_TEMP_CONTROL
      if(pid_debug || bang_bang) {
         show_pid_values();
      }
   #endif

   last_PID_error = PID_error;

   // PID_error is the curent temperature error (desired_temp - current_temp)
   // Negative values indicate the temperature is too high and needs to be 
   // lowered.  Positive values indicate the temperature is too low and needs
   // to be raised.
   //
   // FILTER_OFFSET is a temperature offset constant: negative values raise 
   // the temperature curve,  positive values lower it
   PID_error = (desired_temp + FILTER_OFFSET) - this_temperature;

   // integral_step is the clipped temperature error
   integral_step = PID_error; 
   if(integral_step > MAX_INTEGRAL) integral_step = MAX_INTEGRAL;  
   else if(integral_step < (-MAX_INTEGRAL)) integral_step = (-MAX_INTEGRAL);

   if((PID_out >= MAX_PID) && (integral_step > 0.0)) integral_step *= (-integrator_reset);
   else if((PID_out <= (-MAX_PID)) && (integral_step < 0.0)) integral_step *= (-integrator_reset);

   integrator += (integral_step * k4);
   if((integrator+k6) > (DATA_SIZE) 1.0) integrator = ((DATA_SIZE) 1.0 - k6);    // integrator value is maxed out
   else if((integrator+k6) < ((DATA_SIZE) -1.0)) integrator = ((DATA_SIZE) -1.0 - k6);

   PID_out = (k1*PID_error) + (k2*last_PID_error) + (k3*last_PID_out);
   last_PID_out = PID_out;
   if(PID_out > (DATA_SIZE) 0.0) PID_out -= k7;   // autotune debug - manual step size
   else                          PID_out += k7;
   PID_out += (integrator + k6);

   // clamp the filter response
   if(PID_out > (DATA_SIZE) MAX_PID) PID_out = (DATA_SIZE) MAX_PID;
   else if(PID_out < (DATA_SIZE) (-MAX_PID)) PID_out = (DATA_SIZE) (-MAX_PID);

   // convert filter output to PWM duty cycle times
   pwm_width = (-PID_out) * (DATA_SIZE) PWM_CYCLE_TIME;
   cool_time = (DATA_SIZE) PWM_CYCLE_TIME + pwm_width;
   heat_time = (DATA_SIZE) PWM_CYCLE_TIME - pwm_width;

//sprintf(debug_text, "width:%f  heat:%f(%d)  cool:%f(%d)", pwm_width, heat_time,(int)heat_time,cool_time, (int) cool_time);
   sprintf(cmd, "$CTRL %04d", (int) ((pwm_width/PWM_CYCLE_TIME)*(-1000.0)));
   send_fan_cmd(cmd);

   #ifdef DEBUG_TEMP_CONTROL
      if(pid_debug || bang_bang) {
         sprintf(debug_text,  "pwm=%.2f  heat:%.1f ms  cool:%.1f ms   PID(%.5f)  err(%.5f)  int=%.5f",
            pwm_width, heat_time,cool_time,  
            PID_out,  PID_error*((DATA_SIZE) -1.0),  integrator);
         last_PID_display =  PID_out;
      }
   #endif
}

void analyze_bang_bang()
{
char *s;
double ct;
double val;

   ct = 0.0;
   if(bang_bang == 0) {  // we are not banging around
      return;   
   }
   else if(bang_bang == 1) {    // waiting for a temp zero crossing (temp near setpoint)
      if(((PID_error > 0.0) && (last_PID_error < 0.0)) ||
         ((PID_error < 0.0) && (last_PID_error > 0.0))) { 
         bang_bang = 2;         // now wait for pid to settle
         bang_settle_time = (int) (I_TC * 5.0F);  // wait for 5 integrator time constants
      }
      s = "ZEROING";
      goto tuned_up;
   }
   else if(bang_bang == 2) {  // waiting for pid filter to settle
      if(bang_settle_time > 0) --bang_settle_time;
      else bang_bang = 3;     // start analyzing pid disturbance cycles
      ct_samples = bang_settle_time;
      s = "SETTLING";
      goto tuned_up;
   }
   else if(bang_bang == 3) {  // first bang time,  initialize variables
      s = "INITIALIZING";
      BANG_INITS = 2;
      tune_cycles = TUNE_CYCLES;
      goto start_banging;
   }
   else if(bang_bang == 4) { // quick bang test
      s = "QUICK TEST";
      BANG_INITS = 0;
      tune_cycles = 1;

      start_banging:
      low_ok = high_ok = 0;
      heat_rate = cool_rate = 0.0F;
      heat_sum = cool_sum = 0.0F;
      heat_ticks = cool_ticks = 0.0F;
      cycle_time = avg_cycle_time = fall_time = ct_samples = 0.0;
      this_cycle = last_cycle = fall_cycle = GetMsecs();
      cyc_ptr = amp_ptr = 0;
      tune_cyc[0] = tune_amp[0] = 0.0;

      OLD_P_GAIN = P_GAIN;
      P_GAIN = 0.0F;
      if(PID_error > 0.0F) k6 = KL_TUNE_STEP;
      else                 k6 = (-KL_TUNE_STEP);
      calc_k_factors();

      bang_bang = 5;
      goto tuned_up;
   }


   // PID filter should now be near a steady state
   // Start disturbance testing
   last_delta = delta;
   delta = this_temperature - desired_temp;

   if(delta > 0.0F) {
      heat_sum += delta;     // calculate average peak values
      heat_ticks += 1.0F;
   }
   else if(delta < 0.0F) {
      cool_sum += delta;
      cool_ticks += 1.0F;
   }

   if(PID_error > 0.0F) k6 = KL_TUNE_STEP;
   else                 k6 = (-KL_TUNE_STEP);

   if((low_ok > BANG_INITS) && (high_ok > BANG_INITS)) {  // we are past the disturbance startup wobblies
      s = "TUNING";
   }
   else {
      s = "DISTURBING";
   }

   if((last_PID_error > 0.0F) && (PID_error <= 0.0F)) {  // rising zero crossing
      this_cycle = GetMsecs();
      cycle_time = this_cycle - last_cycle;
      cycle_time /= 1000.0;    // waveform cycle time in seconds
      fall_time = this_cycle - fall_cycle;
      fall_time /= 1000.0;

      if((low_ok > BANG_INITS) && (high_ok > BANG_INITS)) { // it's not the first rising crossing
         if(cycle_time > 10.0) {  // crossing is probably not noise
            avg_cycle_time += cycle_time;
            ct_samples += 1.0;
            if(cyc_ptr <= tune_cycles) cyc_ptr = add_to_bin(cycle_time,  tune_cyc,  cyc_ptr);

            if(heat_ticks) HEAT_OVERSHOOT = (heat_sum / heat_ticks) * 1.414F;
            if(cool_ticks) COOL_UNDERSHOOT = (cool_sum / cool_ticks) * 1.414F;
            val = HEAT_OVERSHOOT - COOL_UNDERSHOOT;
            if(amp_ptr <= tune_cycles) amp_ptr = add_to_bin(val,  tune_amp,  amp_ptr);
            heat_sum = cool_sum = 0.0F;
            heat_ticks = cool_ticks = 0.0F;

            if(cyc_ptr >= tune_cycles) {
               calc_autotune();
               s = "DONE";
               last_cycle = this_cycle;
               goto tuned_up;
            }
         }
      }
      else { // first rising zero crossing
         heat_sum = cool_sum = 0.0F;
         heat_ticks = cool_ticks = 0.0F;
         avg_cycle_time = ct_samples = 0.0;
      }

      last_cycle = this_cycle;
      rate_sum = delta;
      rate_count = 1.0F;

      ++low_ok;
   }
   else if((last_PID_error < 0.0F) && (PID_error >= 0.0F)) {  // falling zero crossing
      fall_cycle = GetMsecs();
      rate_sum = delta;     //!!!! should we filter test the cycle time here?
      rate_count = 1.0F;
      ++high_ok;
   }
   else {  // calculate the heating and cooling rates near the zero crossing
      rate_sum += delta;
      if(++rate_count == 5.0) {
         rate_sum /= rate_count;
         if(delta > 0.0) heat_rate = rate_sum;
         else            cool_rate = rate_sum;
      }
   }

   tuned_up:
   #ifdef DEBUG_TEMP_CONTROL
      ct = cycle_time;
      sprintf(debug_text2, "%s: cycle(%.0f)=%.3f+%.3f=%.3f sec   us(%.0f)=%.4f os(%.0f)=%.4f  [%.3f,%.4f]", 
             s, ct_samples, ct-fall_time, fall_time, ct,  
             cool_ticks, COOL_UNDERSHOOT,  heat_ticks, HEAT_OVERSHOOT,
             tune_cyc[cyc_ptr/2], tune_amp[amp_ptr/2]
      );
   #endif
}


#define SPIKE_FILTER_TIME 45  // seconds

void filter_spikes()
{
// This routine helps filter out false temperature sensor reading spikes.
// It sends the last valid temperature reading to the PID until the reading  
// either increases in value,  falls below the starting value, or times out.

//temperature += KL_TUNE_STEP;    // simulate a temperature spike
//if(KL_TUNE_STEP > 0.0F) KL_TUNE_STEP -= 0.01F;
//else KL_TUNE_STEP = 0.0F;

   if(enviro_temp_pid || ((have_temperature == 0) && enviro_mode())) {  // receiver does not send temperature
      this_temperature = tc1;
      return;
   }
   else {  // use receiver temperature sensor
      this_temperature = temperature;
   }

   if(spike_mode == 0) {
      spike_delay = 0;
      return;  // not filtering spikes
   }

   if(spike_threshold && last_temp) { // filter out temperature spikes
      if(spike_delay > 0) {  // hold temperature while spike settles out
         if(this_temperature < spike_temp) {  // temp has fallen below where the spike started
            spike_delay = 0; 
         }
         else if((this_temperature > last_temp_val) && (spike_mode < 2)) {  // temp is now rising
            spike_delay = 0; 
         }
         else {  // countdown max spike filter time
            --spike_delay;
            this_temperature = spike_temp;
         }
      }
      else if((this_temperature - last_temp) > spike_threshold) {  // temp spike seen
         if(undo_fw_temp_filter) spike_delay = 3;                  // de-filtered temps cause one second spikes
         else                    spike_delay = SPIKE_FILTER_TIME;  // filtered temps cause 30 second spikes
         spike_temp = last_temp;
         this_temperature = spike_temp;
      }
   }

   if(spike_mode > 1) temperature = this_temperature;
}

void control_temp()
{
char cmd[SLEN];

   if(temp_control_on == 0) return;

   if((test_heat > 0.0F) || (test_cool > 0.0F)) {  // manual pwm control is in effect
      heat_time = test_heat;
      cool_time = test_cool;

      sprintf(cmd, "$CTRL %04d", (int) (heat_time-cool_time));
      send_fan_cmd(cmd);

      #ifdef DEBUG_TEMP_CONTROL
         if(pid_debug || bang_bang) {
            sprintf(debug_text,  "heat:%.1f ms  cool:%.1f ms", heat_time, cool_time);
         }
      #endif
   }
   else {   // we are doing the PID filter stuff
      temp_pid_filter();     // update the PID and fan speed
      analyze_bang_bang();   // do autotune analysis
   }

   last_temp = this_temperature;
}

void apply_heat(void)
{
   if(lpt_addr) {  // using parallel port to control temp
      lpt_val = (lpt_val & 0xF0) | 0x09;
      outp(lpt_addr, lpt_val);
   }
   else if(com[fan_port].com_port > 0) { // using serial port DTR line to control temp
      SetDtrLine(fan_port, HEAT);
      SetRtsLine(fan_port, ENABLE);
   }

   if(a_heating == 0) {
      heat_on_tick = GetMsecs(); 
   }
   a_heating = 1;
   a_cooling = 0;
   a_holding = 0;
   temp_dir = UP_ARROW;

}

void apply_cool() 
{
   if(lpt_addr) {  // using parallel port to control temp
      lpt_val = (lpt_val & 0xF0) | 0x0A;
      outp(lpt_addr, lpt_val);
   }
   else if(com[fan_port].com_port > 0) { // using serial port DTR line to control temp
      SetDtrLine(fan_port, COOL);
      SetRtsLine(fan_port, ENABLE);
   }

   if(a_cooling == 0) {
      cool_on_tick = GetMsecs(); 
   }
   a_cooling = 1;
   a_heating = 0;
   a_holding = 0;
   temp_dir = DOWN_ARROW;
}

void hold_temp()
{
   if(lpt_addr) {  // we are using the parallel port to control temp
      lpt_val = (lpt_val & 0xF0) | 0x06;
      outp(lpt_addr, lpt_val);
   }
   else if(com[fan_port].com_port > 0) { // using serial port DTR line to control temp
      SetDtrLine(fan_port, COOL);
      SetRtsLine(fan_port, DISABLE);
   }
   send_fan_cmd("$HOLD");

   if(a_holding == 0) {
      hold_on_tick = GetMsecs(); 
   }
   a_holding = 1;
   a_heating = 0;
   a_cooling = 0;
   temp_dir = '=';
}

void enable_temp_control()
{
   if(lpt_port) {  // get lpt port address from bios area
      lpt_addr = init_lpt();
   }
   else lpt_addr = 0;

   if(lpt_addr) {  // turn on parallel port temp control unit
      lpt_val = (lpt_val & 0x0F) | 0x90;
      outp(lpt_addr, lpt_val);
   }
   else if(com[fan_port].com_port > 0) { // using serial port lines to control temp
      SetRtsLine(fan_port, ENABLE);
   }
   send_fan_cmd("$INIT");

   low_ok = high_ok = 0;
   HEAT_OVERSHOOT = COOL_UNDERSHOOT = 0.0F;

   temp_control_on = 1;
   hold_temp();
}

void disable_temp_control()
{
   if(temp_control_on == 0) return;

   hold_temp();
   if(lpt_addr) {  // using parallel port to control temp
      lpt_val = (lpt_val & 0x0F) | 0x60;
      outp(lpt_addr, lpt_val);
   }
   else if(com[fan_port].com_port > 0) { // using serial port to control temp
     SetRtsLine(fan_port, DISABLE);
   }
   send_fan_cmd("$STOP");

   temp_control_on = 0;
}


void update_pwm(void)
{
   // This routine is called whenever the system is idle or waiting in a loop.
   // It checks to see if it is time to change the temperature contoller PWM
   // output.
   this_msec = GetMsecs();
   if(temp_control_on == 0) return;

   if(this_msec < last_msec) {  // millisecond clock has wrapped
      cool_off_time = heat_off_time = 0.0F;
   }
   last_msec = this_msec;

   if(a_cooling) {   // we are currently cooling
      if(this_msec > cool_off_time) {  // it's time to stop cooling
         if(heat_time) {               // so turn on the heat
            apply_heat();
            heat_off_time = this_msec + (double) heat_time;  // set when to stop heating
         }
      }
   }
   if(a_heating) {  // we are currently heating
      if(this_msec > heat_off_time) {   // it's time to stop heating
         if(cool_time) {                // so turn on the cooler
            apply_cool();
            cool_off_time = this_msec + (double) cool_time;  // set when to stop cooling
         }
      }
   }

   // failsafe code to make sure controller does not lock up!!!!
   if((cool_time == 0.0) && (heat_time >= (PWM_CYCLE_TIME*2.0)) && (a_heating == 0)) {
      apply_heat();
      heat_off_time = this_msec + (double) heat_time;  // set when to stop heating
   }
   if((heat_time == 0.0) && (cool_time >= (PWM_CYCLE_TIME*2.0)) && (a_cooling == 0)) {
      apply_cool();
      cool_off_time = this_msec + (double) cool_time;  // set when to stop heating
   }
   if((a_heating == 0) && (a_cooling == 0) && (a_holding == 0)) {
      apply_cool();
      cool_off_time = this_msec + 100.0;
   }
   if(a_holding) {  // we are currently holding !!!!
      apply_cool();
      heat_off_time = this_msec + 100.0;  // set when to stop heating
   }
}
#endif  // TEMP_CONTROL


#ifdef OSC_CONTROL

// Warren's filter info
double osc_PID_out;          // new filter output
double last_osc_PID_out;     // previous filter output
double osc_PID_error;        // new osc ture error
double last_osc_PID_error;   // previous osc error
double last_osc_PID_display; // previous PID filter value (for debug display)

double osc_integral_step;    // integrator update factor
double osc_integrator;       // the current integrator value

// internally used PID filter constants (derived from user friendly ones)
double osc_k1;      // the proportional gain
double osc_k2;      // the derivitive time constant
double osc_k3;      // the filter time constant (scaled)
double osc_k4;      // the integrator time constant
double osc_k5;   // note that the k5 pid param is FILTER_OFFSET
double osc_k6;      // load distubance test tuning param
double osc_k7;      // loop gain tuning param
double osc_k8;      // integrator reset
#define osc_loop_gain        0.0 // osc_k7
#define osc_integrator_reset osc_k8

#define OSC_PID_UPDATE_RATE   1.0              // how fast the PID filter is updated in seconds
#define OSC_MAX_PID           100.0  // 0.99F
#define OSC_MAX_INTEGRAL      10.0    //2.0               // max integrator update step
#define OSC_SCALE_FACTOR      (1.0+osc_loop_gain)  // !!!! ////
#define MAX_PID_HZ            0.1    // saturated pid moves freq this many Hz


void calc_osc_k_factors()
{
double i_tc;
double f_tc;

   // This routine converts the user friendly input PID constants into the
   // "k" values used by the PID filter algorithm.  It is called whenever
   // a PID filter constant is changed by the user.
   //
   // OSC_P_GAIN        -> osc_k1   (PID gain)
   // OSC_D_TC          -> osc_k2   (derivative time constant)
   // OSC_FILTER_TC     -> osc_k3   (filter time constant)
   // OSC_I_TC          -> osc_k4   (integrator time constant)
   // OSC_FILTER_OFFSET             (osc setpoint offset)

   if(osc_rampup && OSC_I_TC && (osc_rampup < OSC_I_TC)) {
      i_tc = OSC_I_TC      * (osc_rampup / OSC_I_TC);
      f_tc = OSC_FILTER_TC * (osc_rampup / OSC_I_TC);
      osc_rampup += 1.0;
   }
   else {
      i_tc = OSC_I_TC;
      f_tc = OSC_FILTER_TC;
   }

   if(i_tc) osc_k4 = (1.0/i_tc) / OSC_PID_UPDATE_RATE * OSC_P_GAIN;
   else     osc_k4 = 0.0;

   if(f_tc > 0.0) {
      osc_k3 = 1.0 - (1.0/((f_tc*OSC_PID_UPDATE_RATE)+1.0));
      osc_k2 = (-1.0 * OSC_D_TC * OSC_P_GAIN) / (f_tc*OSC_PID_UPDATE_RATE+1.0) * (OSC_PID_UPDATE_RATE/OSC_SCALE_FACTOR);
   }
   else {
      osc_k3 = OSC_FILTER_TC;
      osc_k2 = (-1.0 * OSC_D_TC * OSC_P_GAIN) * (OSC_PID_UPDATE_RATE/OSC_SCALE_FACTOR);
   }

   osc_k1 = ((OSC_P_GAIN/OSC_SCALE_FACTOR) * (1.0-osc_k3)) - osc_k2;

   if(osc_pid_debug && (osc_rampup == 0.0)) {  // set a marker each time a PID value changes
      if(++test_marker >= MAX_MARKER) test_marker = 1;
      if(test_marker < 0) test_marker = 1;
      mark_q_entry[test_marker] = plot_q_in;
   }

   if(osc_pid_debug) {
      show_osc_pid_values();
   }
}

void reset_osc_pid(int kbd_cmd)
{
   disable_osc_control();

   osc_PID_out = 0.0;          // new filter output
   last_osc_PID_out = 0.0;     // previous filter output
   osc_PID_error = 0.0;        // new osc error
   last_osc_PID_error = 0.0;   // previous osc error
   last_osc_PID_display = 0.0; // previous PID filter value (for debug display)
   osc_integral_step = 0.0;    // integrator update factor
   osc_integrator = 0.0;       // integral value

   post_q_count = 0;
   new_postfilter();

// if(kbd_cmd) enable_osc_control(); 
}

void set_default_osc_pid(int pid_num)
{
   // standard filter constants
   OSC_FILTER_OFFSET = 0.0;
   OSC_FILTER_TC = 2.0;
   osc_k6 = 0.0;
   osc_k7 = 0.0;
   osc_k8 = 0.0;
   osc_integrator = 0.0;
   osc_postfilter = 0;

   if(pid_num == 0) {        // slower PPS recovery
      OSC_P_GAIN = 0.1;     
      OSC_D_TC = 0.0;         
      OSC_FILTER_TC = 100.0; 
      OSC_I_TC = 100.0;      
      OSC_FILTER_OFFSET = 50.0;
   }
   else if(pid_num == 1) {   // faster, noiser PPS recovery
      OSC_P_GAIN = 0.1;     
      OSC_D_TC = 0.0;         
      OSC_FILTER_TC = 100.0; 
      OSC_I_TC = 100.0;      
      OSC_FILTER_OFFSET = 100.0;
   }
   else if(pid_num == 2) {   // John's fancy pants oscillator
      OSC_P_GAIN = 0.03;    
      OSC_D_TC = 0.0;         
      OSC_FILTER_TC = 100.0; 
      OSC_I_TC = 500.0;      
      OSC_FILTER_OFFSET = 40.0;
      osc_postfilter = 100;
   }
   else if(pid_num == 3) {   // TBD
      OSC_P_GAIN = 0.06;    
      OSC_D_TC = 0.0;         
      OSC_FILTER_TC = 100.0; 
      OSC_I_TC = 250.0;      
      OSC_FILTER_OFFSET = 70.0;
   }

   new_postfilter();
   calc_osc_k_factors();
}

void show_osc_pid_values()
{
   if(dac_dac) return;

   sprintf(plot_title, "bP(%.2f %c %.4f) bD(%.2f %c %.4f) bF(%.2f %c %.4f) bI(%.1f %c %.3f)  bO=%.3f bL=%.3f bS=%.3f bR=%.3f b9=%-3d", 
      OSC_P_GAIN,RIGHT_ARROW,osc_k1, 
      OSC_D_TC,RIGHT_ARROW,osc_k2, 
      OSC_FILTER_TC,RIGHT_ARROW,osc_k3, 
      OSC_I_TC,RIGHT_ARROW,osc_k4, 
      OSC_FILTER_OFFSET, osc_k6, osc_k7, osc_k8, osc_postfilter);
   title_type = OTHER;
}

double osc_pps_q[PRE_Q_SIZE+1];
double osc_osc_q[PRE_Q_SIZE+1];

double osc_post_q[POST_Q_SIZE+1];
double post_q_sum;

void new_postfilter()
{
double post_val;
int i;

   if(1 || (osc_postfilter < post_q_count)) {  // changing filter size,  rebuild filter queue
      if(post_q_count) post_val = post_q_sum / (double) post_q_count;
      else             post_val = osc_PID_out;
      post_q_sum = 0.0;
      for(i=0; i<osc_postfilter; i++) {
         osc_post_q[i] = post_val;
         post_q_sum += post_val;
      }
      post_q_count = osc_postfilter;
      post_q_in = 0;
   }
}

void osc_pid_filter()
{
double dac_ctrl;
double pps_tweak;
double osc_PID_val;
int i;

   if(0 && osc_prefilter) {
      if(rcvr_type == X72_RCVR) {
         osc_osc_q[opq_in] = osc_offset;
         osc_pps_q[opq_in] = pps_offset * 1.0E-9;
      }
      else {
         osc_osc_q[opq_in] = osc_offset;
         osc_pps_q[opq_in] = pps_offset;
      }
      if(++opq_in >= osc_prefilter) opq_in = 0;
      if(++opq_count > osc_prefilter) opq_count = osc_prefilter;

      pps_bin_count = 0;
      avg_pps = avg_osc = 0.0;
      for(i=0; i<opq_count; i++) {  // !!! we should calc this more efficiently by tracking what is in the queue
         avg_osc += osc_osc_q[i];
         avg_pps += osc_pps_q[i];
         ++pps_bin_count;
      }
      avg_osc /= (double) pps_bin_count;
      avg_pps /= (double) pps_bin_count;
   }
   else if(rcvr_type == X72_RCVR) {
      avg_osc = x72_xxx_dds(pps_offset);
sprintf(debug_text4, "avg osc1:%g  count:%d", avg_osc, x72_dds_count);
      avg_osc = avg_osc * (1.0 + (((double) (x72_dds_count - x72_tc_val)) / (2.0*(double) x72_tc_val)) );
sprintf(debug_text3, "avg osc2:%g", avg_osc);
      if(x72_dds_count) avg_osc /= x72_dds_count;
      avg_osc /= x72_osc;
      if(x72_dds_count < 60) avg_osc = last_x72_dds;
sprintf(debug_text2, "avg osc3:%g", avg_osc);
      avg_pps = pps_offset * 1.0E-9;
   }
   else {
      avg_osc = osc_offset;
      avg_pps = pps_offset;
   }

   if(osc_rampup && (osc_rampup < OSC_I_TC)) {
      calc_osc_k_factors();
   }

   #ifdef DEBUG_OSC_CONTROL
      if(osc_pid_debug) {
         show_osc_pid_values();
      }
   #endif

   last_osc_PID_error = osc_PID_error;

   // osc_PID_error is the curent oscillator error 
   // Negative values indicate the osc is too high and needs to be 
   // lowered.  Positive values indicate the osc too high and needs
   // to be raised.
   //
   // OSC_FILTER_OFFSET is a offset constant: negative values raise 
   // the osc curve,  positive values lower it
// pps_tweak = ((float) pps_offset) * OSC_FILTER_OFFSET * (OSC_P_GAIN/1000.0F);
// osc_PID_error = ((float) osc_offset) + pps_tweak;  // OSC_FILTER_OFFSET;
   pps_tweak = avg_pps * OSC_FILTER_OFFSET * (OSC_P_GAIN/1000.0);
   osc_PID_error = avg_osc + pps_tweak;  // OSC_FILTER_OFFSET;

   // osc_integral_step is the clipped osc error
   osc_integral_step = osc_PID_error; 
   if(osc_integral_step > OSC_MAX_INTEGRAL) osc_integral_step = OSC_MAX_INTEGRAL;  
   else if(osc_integral_step < (-OSC_MAX_INTEGRAL)) osc_integral_step = (-OSC_MAX_INTEGRAL);

   if((osc_PID_out >= OSC_MAX_PID) && (osc_integral_step > 0.0)) osc_integral_step *= (-osc_integrator_reset);
   else if((osc_PID_out <= (-OSC_MAX_PID)) && (osc_integral_step < 0.0)) osc_integral_step *= (-osc_integrator_reset);

   osc_integrator += (osc_integral_step * osc_k4);
   if((osc_integrator+osc_k6) > 1.0F) osc_integrator = (1.0F-osc_k6);    // integrator value is maxed out
   else if((osc_integrator+osc_k6) < (-1.0F)) osc_integrator = (-1.0F-osc_k6);

   osc_PID_out = (osc_k1*osc_PID_error) + (osc_k2*last_osc_PID_error) + (osc_k3*last_osc_PID_out);
   last_osc_PID_out = osc_PID_out;
   if(osc_PID_out > 0.0F) osc_PID_out -= osc_k7;   // autotune debug - manual step size
   else                   osc_PID_out += osc_k7;
   osc_PID_out += (osc_integrator + osc_k6);

   // clamp the filter response
   if(osc_PID_out > OSC_MAX_PID) osc_PID_out = OSC_MAX_PID;
   else if(osc_PID_out < (-OSC_MAX_PID)) osc_PID_out = (-OSC_MAX_PID);

   if(osc_postfilter) {
      if(post_q_count >= osc_postfilter) post_q_sum -= osc_post_q[post_q_in];
      post_q_sum += osc_PID_out;
      osc_post_q[post_q_in] = osc_PID_out;
      if(++post_q_in >= osc_postfilter)   post_q_in = 0;
      if(++post_q_count > osc_postfilter) post_q_count = osc_postfilter;
      osc_PID_val = post_q_sum / (double) post_q_count;
   }
   else {
      osc_PID_val = osc_PID_out;
   }

   // convert filter output to DAC volatge
   if(rcvr_type == X72_RCVR) {
      if(osc_gain) dac_ctrl = (osc_PID_val*0.10) / (double) osc_gain;  // 4.0E-8 = MAX_DDS_STEP
      else dac_ctrl = 0.0;
      dac_ctrl += (double) osc_pid_initial_voltage;
      set_x72_dds(dac_ctrl);
sprintf(debug_text5, "set dds: %g  opv:%g  avg_osc:%g  avg_pps:%g  pps_tweak:%g", dac_ctrl,osc_PID_val, avg_osc,avg_pps,pps_tweak);
   }
   else {
      if(osc_gain) dac_ctrl = (osc_PID_val*MAX_PID_HZ) / (double) osc_gain;
      else dac_ctrl = 0.0;
      dac_ctrl += (double) osc_pid_initial_voltage;
      set_dac_voltage((float) dac_ctrl, 4);
   }

   #ifdef DEBUG_OSC_CONTROL
      if(osc_pid_debug) {
         sprintf(debug_text,  " PID(%g)  post(%g,%d)  last_out(%g)  err(%g)  last_err(%g)  int=%g  ramp=%g",
            osc_PID_out, osc_PID_val,post_q_count,  last_osc_PID_out, osc_PID_error*(-1.0F),  last_osc_PID_error, osc_integrator, osc_rampup);
         last_osc_PID_display =  osc_PID_out;
      }
   #endif
}

void enable_osc_control()
{
   osc_discipline = 0;
   set_discipline_mode(SET_DIS_MODE_DISABLE);

   if(rcvr_type == X72_RCVR) {
      set_x72_dds(last_x72_dds);
      osc_pid_initial_voltage = (DATA_SIZE) last_x72_dds;
   }
   else {
      set_dac_voltage(dac_voltage, 5);
      osc_pid_initial_voltage = dac_voltage;
   }

   osc_control_on = 1;
   osc_rampup = 1.0;
}

void disable_osc_control()
{
   if(osc_control_on == 0) return;

   osc_discipline = 1;
   if(rcvr_type == X72_RCVR) {
   }
   else {
      set_discipline_mode(SET_DIS_MODE_ENABLE);
   }

   osc_control_on = 0;
}


int trick_tick;
#define MAX_TRICK 1000.0

void control_osc()
{
double true_pps;

   // calculate integral of osc signal
   if(trick_scale) {
      if(trick_tick) --trick_tick;
      else {
         true_pps = (pps_offset + trick_value);
         trick_value = (true_pps * trick_scale);

         if(trick_value > MAX_TRICK) trick_value = MAX_TRICK;
         else if(trick_value <= (-MAX_TRICK)) trick_value = (-MAX_TRICK);

         set_pps(user_pps_enable, pps_polarity,  trick_value*1.0E-9, pps1_delay, 300.0, 0);
         request_pps_info();
         sprintf(plot_title,  "Cable trick:  scale=%f  ofs=%f", trick_scale, trick_value);
         title_type = OTHER;
         trick_tick = 5;
      }
   }

   if(osc_control_on) {  // we are controlling the oscillator disciplining
      osc_pid_filter();
   }

   if(dac_dac) {         // we are doing a osc param autotune
      calc_osc_gain();
   }
}

#endif  // OSC_CONTROL




#ifdef FFT_STUFF

/* function prototypes for dft and inverse dft functions */
void fft(COMPLEX *,int);
void rfft(float *,COMPLEX *,int);
int  logg2(long);


/**************************************************************************

fft - In-place radix 2 decimation in time FFT

Requires pointer to complex array, x and power of 2 size of FFT, m
(size of FFT = 2**m).  Places FFT output on top of input COMPLEX array.

void fft(COMPLEX *x, int m)

*************************************************************************/

void fft(COMPLEX *x, int m)
{
static int mstore = 0;       /* stores m for future reference */
static int n = 1;            /* length of fft stored for future */

COMPLEX u,temp,tm;
COMPLEX *xi, *xip, *xj, *wptr;

int i,j,k,l,le,windex;

double arg,w_real,w_imag,wrecur_real,wrecur_imag,wtemp_real;

    if(m != mstore) {  // fft size changed,  redo w array
        /* free previously allocated storage and set new m */
        mstore = m;
        if(m == 0) return;       /* if m=0 then done */

        /* n = 2**m = fft length */
        n = 1 << m;
        le = n/2;

        /* calculate the w values recursively */
        arg = 4.0*atan(1.0)/le;         /* PI/le calculation */
        wrecur_real = w_real = cos(arg);
        wrecur_imag = w_imag = -sin(arg);
        xj = w;
        for(j=1; j<le; j++) {
            xj->real = (float) wrecur_real;
            xj->imag = (float) wrecur_imag;
            xj++;
            wtemp_real = wrecur_real*w_real - wrecur_imag*w_imag;
            wrecur_imag = wrecur_real*w_imag + wrecur_imag*w_real;
            wrecur_real = wtemp_real;
        }
    }

    /* start fft */
    le = n;
    windex = 1;
    for(l=0; l<m; l++) {
       le = le/2;

       /* first iteration with no multiplies */
       for(i=0; i<n; i=i+2*le) {
           xi = x + i;
           xip = xi + le;
           temp.real = xi->real + xip->real;
           temp.imag = xi->imag + xip->imag;
           xip->real = xi->real - xip->real;
           xip->imag = xi->imag - xip->imag;
           *xi = temp;
       }

       /* remaining iterations use stored w */
       wptr = w + windex - 1;
       for(j=1; j<le; j++) {
          u = *wptr;
          for(i=j; i<n; i=i+2*le) {
             xi = x + i;
             xip = xi + le;
             temp.real = xi->real + xip->real;
             temp.imag = xi->imag + xip->imag;
             tm.real = xi->real - xip->real;
             tm.imag = xi->imag - xip->imag;
             xip->real = tm.real*u.real - tm.imag*u.imag;
             xip->imag = tm.real*u.imag + tm.imag*u.real;
             *xi = temp;
          }
          wptr = wptr + windex;
       }
       windex = 2*windex;
    }

    /* rearrange data by bit reversing */
    j = 0;
    for(i=1; i<(n-1); i++) {
        k = n/2;
        while(k <= j) {
           j = j - k;
           k = k/2;
        }
        j = j + k;
        if(i < j) {
           xi = x + i;
           xj = x + j;
           temp = *xj;
           *xj = *xi;
           *xi = temp;
        }
    }
}

/************************************************************

rfft - trig recombination real input FFT

Requires real array pointed to by x, pointer to complex
output array, y and the size of real FFT in power of
2 notation, m (size of input array and FFT, N = 2**m).
On completion, the COMPLEX array pointed to by y
contains the lower N/2 + 1 elements of the spectrum.

void rfft(float *x, COMPLEX *y, int m)

***************************************************************/

void rfft(float *x, COMPLEX *y, int m)
{
static    int      mstore = 0;
int       p,num,k;
float     Realsum, Realdif, Imagsum, Imagdif;
double    factor, arg;
COMPLEX   *ck, *xk, *xnk, *cx;

   /* First call the fft routine using the x array but with
      half the size of the real fft */

    p = m - 1;
    cx = (COMPLEX *) x;
    fft(cx, p);

    /* Next create the coefficients for recombination, if required */
    num = (1 << p);    /* num is half the real sequence length.  */

    if(m != mstore) { // fft size changed,  redo cf array
       mstore = m;  

       factor = 4.0*atan(1.0)/num;
       for (k = 1; k < num; k++){
         arg = factor*k;
         cf[k-1].real = (float)cos(arg);
         cf[k-1].imag = (float)sin(arg);
       }
    }

    /* DC component, no multiplies */
    y[0].real = cx[0].real + cx[0].imag;
    y[0].imag = 0.0;

    /* other frequencies by trig recombination */
    ck = cf;
    xk = cx + 1;
    xnk = cx + num - 1;
    for (k = 1; k < num; k++){
      Realsum = ( xk->real + xnk->real ) / 2.0F;
      Imagsum = ( xk->imag + xnk->imag ) / 2.0F;
      Realdif = ( xk->real - xnk->real ) / 2.0F;
      Imagdif = ( xk->imag - xnk->imag ) / 2.0F;

      y[k].real = Realsum + ck->real * Imagsum
                          - ck->imag * Realdif ;

      y[k].imag = Imagdif - ck->imag * Imagsum
                          - ck->real * Realdif ;
      ck++;
      xk++;
      xnk--;
    }
}

/**************************************************************************

logg2 - base 2 logarithm

Returns base 2 log such that i = 2**ans where ans = logg2(i).
if logg2(i) is between two values, the larger is returned.

int logg2(unsigned int x)

*************************************************************************/

int logg2(long x)
{
    unsigned long mask,i;

    if(x == 0) return(-1L);     /* zero is an error, return -1 */
    x--;                        /* get the max index, x-1 */

    for(mask=1,i=0; ; mask*=2,i++) {
        if(x == 0) return(i);   /* return logg2 if all zero */
        x = x & (~mask);        /* AND off a bit */
    }
}



float fft_max, fft_min;

long process_signal(long length, int id)
{
long i;
long last_i;
long j;
int k;
struct PLOT_Q q;
DATA_SIZE show_time;
float a;
float tempflt;
float max_db, min_db;
int color;

   last_i = 0;
   plot_column = 0;

   j = 0;
   i = plot_q_col0;
   fft_queue_0 = i;
   while(i != plot_q_in) {  // copy queue data to FFT buffers
      if(filter_count) q = filter_plot_q(i);
      else             q = get_plot_q(i);

      tsignal[j] = (float) q.data[id];  // * window[j];
      if(queue_interval) tsignal[j] /= queue_interval;
      if(++j >= length) {
         break;  // buffer is full
      }

      i = next_q_point(i, STOP_AT_Q_END);
      if(i < 0) {
         break;
      }
   }

   if(j <= 0) {
      fft_length = 0;
      goto done;
   }

   length = (1 << logg2(j));  // adjust fft length for short data
   if(length > j) length /= 2;
   if(length > max_fft_len) length = max_fft_len;

   fft_scale = 1;
   fft_length = length;
   if(fft_length < 2) goto done;  //fft_scale = 1;
   else               fft_scale = ((view_interval * (long)SCREEN_WIDTH) / (fft_length/2L));
   if(fft_scale < 1)  fft_scale = 1;

   show_time = ((DATA_SIZE) (fft_length)) * ((DATA_SIZE) view_interval);  // seconds per screen
   show_time /= (DATA_SIZE) plot_mag;
   show_time /= nav_rate;
   if(show_time < (DATA_SIZE) (2.0*60.0*60.0)) sprintf(out, "%.1f min", show_time/(DATA_SIZE) 60.0);
   else                                        sprintf(out, "%.1f hrs", show_time/(DATA_SIZE) (60.0*60.0));
   if(title_type != USER) {
      sprintf(plot_title, "%ld point FFT of %s of%s%s data.", 
          fft_length, out, show_live_fft?" live ":" ", plot[id].plot_id);
      title_type = OTHER;
   }

   a = (float) fft_length * (float) fft_length;
   rfft(&tsignal[0], fft_out, logg2(fft_length));

   fft_max = (float) (-BIG_NUM);
   fft_min = (float) (BIG_NUM);
   for(j=1; j<fft_length/2; j++) {
      tempflt  = fft_out[j].real * fft_out[j].real;
      tempflt += fft_out[j].imag * fft_out[j].imag;
      if(tempflt > fft_max) fft_max = tempflt;
      if(tempflt < fft_min) fft_min = tempflt;
   }
   if(fft_max == 0.0F) fft_max = 1.0F;
   max_db = (float) (10.0 * log10(MAX(fft_max/a, 1.e-16))) ;
   min_db = (float) (10.0 * log10(MAX(fft_min/a, 1.e-16))) ;

   if((zoom_screen == 'F') && (fft_col < (SCREEN_WIDTH-TEXT_HEIGHT-2))) {  // FFT waterfall
      for(j=1; j<fft_length/2; j++) {
         tempflt  = fft_out[j].real * fft_out[j].real;
         tempflt += fft_out[j].imag * fft_out[j].imag;

         if(fft_db && (max_db != min_db)) {  // calc FFT in dB
           tempflt /= a;
           tempflt = (float) (10.0 * log10(MAX(tempflt, 1.e-16))) ;
           tempflt -= min_db;
           tempflt /= (max_db-min_db);  // 0..1
         }
         else if(fft_max != fft_min) {
            tempflt -= fft_min;
            tempflt /= (fft_max-fft_min);  // 0..1
         }
         else tempflt = 0.0F;
         tempflt *= 15.0F;    // 15 - number of colors used
         if(tempflt < 0.0F) tempflt = 0.0F-tempflt;
         color = ((int) tempflt) % 15;
         if(color < 0) color = 0;
         else if(color > 14) color = 14;
         dot(fft_col++, fft_row, color+1);
      }

      line(fft_col,fft_row, SCREEN_WIDTH-1, fft_row, BLACK);
      ++fft_row;
      line(0,fft_row, SCREEN_WIDTH-1, fft_row, RED);
      fft_col = 0;
      if(fft_row >= (SCREEN_HEIGHT-TEXT_HEIGHT*2)) fft_row = 0;
   }

   plot_column = 0;
   j = 0;
   i = last_i = plot_q_col0;

   // place fft results into the plot queue FFT plot data
   while(i != plot_q_in) {  
      for(k=0; k<fft_scale; k++) { // expand plot horizontally so that it is easier to read
         q = get_plot_q(i);

         if(j >= fft_length/2) q.data[FFT] = (DATA_SIZE) 0.0;  // fill out queue with 0's
         else if(j == 0)       q.data[FFT] = (DATA_SIZE) 0.0;  // drop the DC value because it messes up scaling
         else {   // insert FFT results into plot queue data
            tempflt  = fft_out[j].real * fft_out[j].real;
            tempflt += fft_out[j].imag * fft_out[j].imag;
            if(fft_db) {  // calc FFT in dB
              tempflt /= a;
              tempflt = (float) (10.0 * log10(MAX(tempflt, 1.e-16))) ;
            }
            else {
               tempflt /= fft_max;
            }
            q.data[FFT] = (DATA_SIZE) tempflt;
            last_i = i;
         }
         if(j == 1) mark_q_entry[1] = i;

         put_plot_q(i, q);
         if(++i == plot_q_in) goto done;
         while(i >= plot_q_size) i -= plot_q_size;
      }
      j++;
      if((j >= fft_length/2) && (j >= SCREEN_WIDTH*2)) break;
   }

   done:
   mark_q_entry[2] = last_i;
   return j;
}

void set_fft_scale()
{
   // configure FFT/histogram plot

   if(fft_type == HIST_TYPE) {     // doing histogram instead of FFT
      plot[FFT].plot_id = "HIST";
      if(fft_db) plot[FFT].units = "dB";
      else       plot[FFT].units = "cnt";

      plot[FFT].user_scale = 0;
      plot[FFT].scale_factor = 1.0F;
      plot[FFT].plot_center = 0.0F;
      plot[FFT].float_center = 1;

      fft_scale = 1;
   }
   else if(fft_db) {
      plot[FFT].plot_id = "FFT";          // assume we are doing an FFT
      plot[FFT].units = "dB";

      plot[FFT].user_scale = 0;
      plot[FFT].scale_factor = 1.0F;
      plot[FFT].plot_center = 0.0F;
      plot[FFT].float_center = 1;
   }
   else {
      plot[FFT].plot_id = "FFT";          // assume we are doing an FFT
      plot[FFT].units = "x ";

      plot[FFT].user_scale = 1;
      plot[FFT].scale_factor = 1.0F / ((float) PLOT_HEIGHT/(float)(VERT_MAJOR*2));
      plot[FFT].plot_center = 0.0F;
      plot[FFT].float_center = 0;
   }
}




void dump_fft_plot()
{
int j;
float tempflt;
float a;
float val;

   // dump the plot FFT data to the debug_file

   if(debug_file == 0) return;
   if(plot[FFT].show_plot == 0) return;
   if(fft_type != FFT_TYPE) return;
   if(fft_length == 0.0) return;

   if(show_live_fft) {
      fprintf(debug_file, "\nLive %s plot FFT:\n", plot[fft_id].plot_id);
   }
   else {
      fprintf(debug_file, "\nSingle %s plot FFT:\n", plot[fft_id].plot_id);
   }

   tempflt = 0.0F;
   a = (float) fft_length * (float) fft_length;

   for(j=1; j<fft_length/2; j++) {
      if(fps) val = (1.0F/ fps) / ((float) j); 
      else val = 0.0;
//sprintf(debug_text2, "fft_scale:%ld  fps:%.9f", fft_scale, fps);

      tempflt  = fft_out[j].real * fft_out[j].real;
      tempflt += fft_out[j].imag * fft_out[j].imag;
      if(fft_db) {  // calc FFT in dB
         tempflt /= a;
         tempflt = (float) (10.0 * log10(MAX(tempflt, 1.e-16))) ;
         fprintf(debug_file, "bin:%d  %.3f s   val:%.9f dB\n", j, val, tempflt);
      }
      else if(tempflt) {
         tempflt /= fft_max;
         fprintf(debug_file, "bin:%d  %.3f s   val:%.9f\n", j, val, tempflt);
      }
   }

   fprintf(debug_file, "\n");
   fflush(debug_file);
}

long calc_fft(int id)
{
long length, m;
long points;

   fft_id = id;
   set_fft_scale();

   if(fft_type == HIST_TYPE) {  // do histogram instead of an FFT
      if(id == FFT) {
//      edit_error("Cannot calculate the histogram of the histogram plot!");
//      return 0;
id = pre_fft_plot;
      }
      points = calc_plot_hist(id);
   }
   else {  // do FFT instead of histogram
      if(id == FFT) {
        edit_error("Cannot calculate the FFT of the FFT plot!");
        return 0;
      }

      length = max_fft_len;
      if(length < 2) {  /* Check for power of 2 input size */
        edit_error("FFT size must be a power of 2 greater than 1");
        return 0;
      }

      m = logg2(length);
      if((1<<m) != length) {  /* Check for power of 2 input size */
        edit_error("FFT size must be a power of 2.");
        return 0;
      }

      fps = ((1.0F/view_interval)/2.0F) / (float) (length/2);
      points = process_signal(length, id);
   }

   // set scale factors and enable the FFT plot
   if(show_live_fft == 0) {
      plot[FFT].show_stat = 1;
      if(plot[FFT].show_plot == 0) toggle_plot(FFT);
   }

   return points;
}



void dump_hist_plot()
{
int i;
DATA_SIZE val;

   // dump the plot histogram data to the debug_file

   if(debug_file == 0) return;
   if(plot[FFT].show_plot == 0) return;
   if(fft_type != HIST_TYPE) return;

   if(show_live_fft) {
      fprintf(debug_file, "\nLive %s plot histogram:\n", plot[fft_id].plot_id);
   }
   else {
      fprintf(debug_file, "\nSingle %s plot histogram:\n", plot[fft_id].plot_id);
   }

   for(i=0; i<hist_size; i++) {
      if(plot_hist[i] == 0) continue;  // empty bin
      val = hist_minv + (hist_bin_width*((DATA_SIZE) i));
      fprintf(debug_file, "bin:%d  val:%.9f  count:%ld\n", i, val, plot_hist[i]);
   }

   fprintf(debug_file, "\n");
   fflush(debug_file);
}



DATA_SIZE scale_hist_val(int id, DATA_SIZE val)
{
   // convert a raw plot queue data value to it's displayed value

//!!!!!! we need to handle more cases... see label_cursor_val()
   if(tie_plot(id)) {  // it's a time interval error plot
      if(plot[id].show_freq) {  // frequency mode
         val = (DATA_SIZE) tie_to_freq(id, (double) val);
      } 
   }
   else if(id == TEMP) {
////  val /= 1000.0;
   }

   return val;
}


long calc_plot_hist(int id)
{
long i, j, k;
long last_i;
long count;
int col;
struct PLOT_Q q;
DATA_SIZE val, aval;
int bin;
int max_bin;
long max_count;

   // calculate a histogram of a plot queue value

   if(queue_interval == 0) return 0;
   if(stat_count == 0) return 0;
   if(id < 0) return 0;
   if(id > (NUM_PLOTS+DERIVED_PLOTS)) return 0;

   last_i = 0;
   plot_column = 0;
   hist_size = plot_q_count;
   if(hist_size > PLOT_WIDTH) hist_size = PLOT_WIDTH;
   if(hist_size >= MAX_HIST) hist_size = MAX_HIST;

   fft_id = id;
   set_fft_scale();

   hist_minv = plot[id].min_disp_val;
   hist_maxv = plot[id].max_disp_val;
   aval = DATA_SIZE (fabs(hist_maxv-hist_minv) * 0.01);
   hist_minv -= aval;   // expand value range by 1% of the span
   hist_maxv += aval;

   hist_bin_width = (hist_maxv - hist_minv) / (DATA_SIZE) hist_size; 

   for(i=0; i<hist_size; i++) plot_hist[i] = 0;

   max_bin = 0;
   max_count = 0;
   count = 0;
   col = 0;
   i = plot_q_col0;   // hist displayed points
   fft_queue_0 = i;
////i = 0;            // hist all queue points
   while(i != plot_q_in) {  // scan the data that is in the plot queue
      q = get_plot_q(i);    // get next point to histogram
      val = (DATA_SIZE) q.data[id] / (DATA_SIZE) queue_interval;
      val = scale_hist_val(id, val);
      bin = (int) ((val - hist_minv) / hist_bin_width);
      if((bin >= 0) && (bin < hist_size)) {
         ++plot_hist[bin];
//       if(plot_hist[bin] > max_count) {   // find first peak bin
         if(plot_hist[bin] >= max_count) {  // find last peak bin
            max_count = plot_hist[bin];
            max_bin = bin;
         }
      }

      ++i;
      if(i >= plot_q_size) i = 0;
      if(++count > plot_q_count) break; // all points in the queue have been processed

      col += plot_mag; // end histogram at end of displayed data
      if(col >= (PLOT_WIDTH*view_interval)) break;
   }

   plot_column = 0;
   j = 0;
   i = last_i = plot_q_col0;

   fft_scale = view_interval;
   if(fft_scale < 1) fft_scale = 1;

   // place hist results into the plot queue FFT plot data
   while(i != plot_q_in) {  
      for(k=0; k<fft_scale; k++) { // expand plot horizontally so that it is easier to read
         q = get_plot_q(i);
         val = (DATA_SIZE) plot_hist[j];
         if(fft_db && (val > 0)) {
            val = (float) (10.0 * log10(MAX((double) val, 0.0))) ;
         }
         q.data[FFT] = val;
         last_i = i;
         if(j == 1) mark_q_entry[1] = i;

         put_plot_q(i, q);
         if(++i == plot_q_in) break;
         while(i >= plot_q_size) i -= plot_q_size;
      }

      ++j;
      if(j >= hist_size) break;
   }
   mark_q_entry[2] = last_i;

// dump_hist_plot();

   if(1 && (title_type != USER)) {
      if((max_bin >= 0) && (max_bin < hist_size)) {
         val = hist_minv + hist_bin_width*(DATA_SIZE) max_bin;
//sprintf(debug_text4, "bin:%d  step:%.9f  count:%ld   min:%.9f max:%.9f  val:%.9f", mouse_x, hist_bin_width, plot_hist[mouse_x], hist_minv, hist_maxv, val);
      }
      else val = 0;

      if(show_live_fft) {
         sprintf(plot_title, "%ld bin live %s plot histogram.  Max count:%ld  bin:%d  val:%f", hist_size, plot[fft_id].plot_id, max_count, max_bin, val);
      }
      else {
         sprintf(plot_title, "%ld bin single %s plot histogram.  Max count:%ld  bin:%d  val:%f", hist_size, plot[fft_id].plot_id, max_count, max_bin, val);
      }
//sprintf(debug_text3, "hist point count:%ld  scale:%ld", count, fft_scale);

      if(mouse_plot_valid && (mouse_x >= 0) && (mouse_x < hist_size)) {
         val = hist_minv + hist_bin_width*(DATA_SIZE) mouse_x;
//sprintf(debug_text, "bin:%d  step:%.9f  count:%ld   min:%.9f max:%.9f  val:%.9f", mouse_x, hist_bin_width, plot_hist[mouse_x], hist_minv, hist_maxv, val);
      }
      title_type = OTHER;
   }

   return hist_size;
}

#endif // FFT_STUFF

