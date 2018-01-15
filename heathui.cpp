#define EXTERN extern
#include "heather.ch"

//  Thunderbolt TSIP monitor
//
//  Copyright (C) 2008,2009 Mark S. Sims - all rights reserved
//  Win32 port by John Miles, KE5FX (jmiles@pop.net)
//
//
//  This file contains most of the user interface stuff.
//  Abandon all hope, ye mortals who dain to understand it...
//  It makes my noggin throb,  and I wrote it...
//

int last_was_mark;
u08 all_plots;
extern char *dst_list[];
extern float k6, k7, k8, integrator;
extern double osc_k6, osc_k7, osc_k8, osc_integrator;
extern int first_request;

#define GRAPH_LLA 1     // if 1,  make graphs 1,2,3 lat/lon/alt if rcvr is in 3D mode

// text editing stuff
u08 first_edit;
u08 insert_mode;
u08 edit_cursor;
u08 no_auto_erase;
u08 dos_text_mode;
int e_row;
char mode_string[SLEN+1];     // the last driver mode change string used

int getting_plot;
int amu_flag = 0x08;

u08 old_show_plots[NUM_PLOTS];  // used to save/restore plot enables when toggling jitter measurement mode
u08 old_plot_azel; 
u08 old_plot_watch; 

float scale_step;
float center_step;

void set_steps(void);

// extended codes for identifiing edit strings
#define DRVR_MODE      0x0100   // driver mode change
#define PC_CMD         0x0101   // load protection overcurrent
#define PH_CMD         0x0102   // battery HVC
#define PL_CMD         0x0103   // battery LVC
#define PM_CMD         0x0104   // message watchdog timeout
#define PO_CMD         0x0105   // battery overcurrent
#define PP_CMD         0x0106   // load overwatts
#define PR_CMD         0x0107   // protection fault reset
#define PS_CMD         0x0108   // temp2 overtemp
#define PT_CMD         0x0109   // temp1 overtemp
#define PU_CMD         0x010A   // load undervoltage
#define PV_CMD         0x010B   // load overvoltage
#define PX_CMD         0x010C   // auxv overvoltage
#define PW_CMD         0x010D   // battery overwatts
#define PZ_CMD         0x010E   // auxv undervoltage
#define WC_CMD         0x010F   // write config data to file
#define S_CMD          0x0110   // set luxor lat, lon, alt

#define AMPL_CMD       0x0120   // lux sensitivity
#define AMPU_CMD       0x0121   // lumen sensitivity
#define AMPE_CMD       0x0122   // IR1 emissivity
#define AMPI_CMD       0x0123   // IR2 emissivity
#define AMPS_CMD       0x0124   // serial number
#define AMPV_CMD       0x0125   // reference voltage
                            
#define BC_CMD         0x0130   // constant current load mode
#define BF_CMD         0x0131   // 3.60V LiFePO4 charge mode
#define BH_CMD         0x0132   // high voltage lipo charge mode
#define BL_CMD         0x0133   // 4.20V lipo charge mode
#define BP_CMD         0x0134   // battery PWM resolution (8/9/10 bit)
#define BR_CMD         0x0135   // pwm sweep rate
#define BS_CMD         0x0136   // pwm sweep
#define BV_CMD         0x0137   // constant load voltage mode
#define BW_CMD         0x0138   // constant load wattage mode

#define CAL_CMD        0x0140   // calibration constants
#define DEBUG_LOG_CMD  0x0150   // open debug log (debug_file)
#define RAW_LOG_CMD    0x0151   // open raw receiver data log (raw_file) 

#define PALETTE_CMD    0x0160   // edit color palette

#define TRAIM_CMD      0x0200   // traim threshold
#define PPS_OFS_CMD    0x0201   // simple pps offset
#define ZODIAC_RESTART 0x0202   // reset Zodiac receiver into Motorola mode
#define USER_CMD       0x0203   // send user command to receiver
#define PPS_OFS1_CMD   0x0210   // PPS1 offset delay
#define PPS_OFS2_CMD   0x0211   // PPS2 offset delay
#define PPS1_CFG_CMD   0x0212
#define PPS2_CFG_CMD   0x0213
#define REF_CMD        0x0214   // GPSDO reference source
#define SAT_IGN_CMD    0x0220   // ignore satellite command
#define GNSS_CMD       0x0230   // GNSS system select command
#define SI_CMD         0x0240   // sat info display count
#define SORT_CMD       0x0250   // sort sat info display
#define SERIAL_CMD     0x0260   // set serial com port params
#define NVS_FILTER_CMD 0x0270   // NVS_RCVR solution filtration factor
#define NAV_RATE_CMD   0x0280   // navigation update rate
#define SUN_CMD        0x0290   // sunrise/sunset calculation type
#define DELTA_T_CMD    0x02A0   // set TT-UT1 delta T
#define MARINE_CMD     0x02B0   // Motorola marine velocity filter
#define PULLIN_CMD     0x02C0   // UCCM pullin-range

void set_com_params(char *p)
{
char *s;

   if(p == 0) return;

   s = p;
   while(*s) { // skip leading white space
      if(*s == ' ') ++s;
      else if(*s == '\t') ++s;
      else break;
   }
   if(s[0] == 0) return;

   if(isdigit(*s)) {
      sscanf(s, "%d", &baud_rate);
   }
   if(baud_rate <= 0) baud_rate = 57600;

   if     (strstr(s, ":8")) data_bits = 8;
   else if(strstr(s, ":7")) data_bits = 7;
   else if(strstr(s, ":6")) data_bits = 6;
   else if(strstr(s, ":5")) data_bits = 5;
   else data_bits = 8;

   if     (strstr(s, ":1")) stop_bits = 1;
   else if(strstr(s, ":2")) stop_bits = 2;
   else stop_bits = 1;

   if     (strstr(s, ":N")) parity = 0;
   else if(strstr(s, ":n")) parity = 0;
   else if(strstr(s, ":O")) parity = 1;
   else if(strstr(s, ":0")) parity = 1;
   else if(strstr(s, ":o")) parity = 1;
   else if(strstr(s, ":E")) parity = 2;
   else if(strstr(s, ":e")) parity = 2;
   else parity = 0;
}


//
//  Keyboard processor
//

int path_unlink(char *f)
{
char name[2048+1];

   // delete a file, add path name if none given
   if(f == 0) return 1;

   strcpy(name, f);

#ifdef WINDOWS
#else // __linux__  __MACH__
   if(f[0] == '.') ;       // user gave a path
   else if(f[0] == '/') ;  // user gave a path
   else {  // add path to heather directory
      strcpy(name, heather_path);
      strcat(name, f);
   }
#endif

   return unlink(name);
}

FILE *path_open(char *f, char *m)
{
char name[2048+1];
FILE *fp;

   // open a file, add path name if none given
   if(f == 0) return 0;
   if(m == 0) return 0;

   strcpy(name, f);

#ifdef WINDOWS
#else // __linux__  __MACH__
   if(f[0] == '.') ;       // user gave a path
   else if(f[0] == '/') ;  // user gave a path
   else {  // add path to heather directory
      strcpy(name, heather_path);
      strcat(name, f);
   }
#endif

   fp = fopen(name, m);
if(show_debug_info) printf("path_open(%s, %s) -> %p\n", name, m, fp);
   return fp;
}

FILE *topen(char *f, char *m)
{
char c;

   // open a file,  trim leading whitespace from the name
   if(f == 0) return 0;
   if(m == 0) return 0;

   while(*f) {
      c = *f;
      if((c == ' ') || (c == '\t')) ++f;
      else break;
   }

if(show_debug_info) printf("topen(%s, %s)\n", f,m);  //zorky
   return path_open(f, m);
}

FILE *open_debug_file(char *s)
{
   if(s == 0) return 0;

   if(debug_file) {
      fclose(debug_file);
      debug_file = 0;
      debug_name[0] = 0;
   }

   debug_file = topen(s, "w");
   strcpy(debug_name, s);
   return debug_file;
}


#define ALWAYS_TEXT_HELP 1

void kbd_help()
{
char *s, *ems_info;
COORD help_row, help_col;
COORD row, col;
COORD help_row_1;

   // this routine displays help info about the keyboard commands 
   // until a key is pressed

   request_version(); // so the "Press space for help" message gets updated quickly
   plot_version = 1;  // once asked for help,  switch help message to version 
                      // display since the user should now know that help exists

   if(1 && osc_params) {   // if enabled, cancel the osc param display mode
      osc_params = 0;
      text_mode = 0;
      erase_screen();
      redraw_screen();
//    do_kbd(' ');
      sure_exit();
      return;
   }

   if(1 || (process_com == 0)) {
      osc_params = 0;
      erase_screen();
      redraw_screen();
   }

   // !!! kludge: for undersized screens
   if(screen_type == 't') {   // text mode
      no_x_margin = no_y_margin = 1;
   }
   else if(ALWAYS_TEXT_HELP || (SCREEN_HEIGHT <= 600)) {  // small graphics screens
      text_mode = 2;  // say we are in text mode while help is displayed
      no_x_margin = no_y_margin = 1;
   }

   if(text_mode) {   // full screen help mode
      if(SCREEN_WIDTH >= 800) {  // there is room for a margin
         help_row = 3;
         help_col = 3;
      }
      else {         // no margin space available
         help_row = 0;
         help_col = HELP_COL;
      }
      erase_screen();
   }
   else {            // help is in the plot area
      help_row = HELP_ROW;
      help_col = HELP_COL+3;
   }

   row = help_row;
   col = help_col + 0;

   s = "Control ";
   ems_info = "";
   if(TEXT_COLS <= 100) {
      if(luxor) sprintf(out, "Luxor Power/LED Analyzer %sProgram", s);
      else      sprintf(out, "Lady Heather's Disciplined Oscillator %sProgram", s);
      vidstr(row,col,  HELP_COLOR, out);
      ++row;

      sprintf(out, "Rev %s - %s %s%s", VERSION, date_string, __TIME__, ems_info);
      vidstr(row,col,  HELP_COLOR, out);
      ++row;

      vidstr(row,col,  HELP_COLOR, "Press any key to continue...");
      ++row;
      ++row;
   }
   else {
      if(luxor) sprintf(out, "Luxor Power/LED Analyzer %sProgram.  Rev %s - %s %s%s", s, VERSION, date_string, __TIME__, ems_info);
      else      sprintf(out, "Lady Heather's Disciplined Oscillator %sProgram.  Rev %s - %s %s%s", s, VERSION, date_string, __TIME__, ems_info);
      vidstr(row,col,  HELP_COLOR, out);
      ++row;

      vidstr(row,col,  HELP_COLOR, "Press any key to continue...");
      ++row;
      ++row;
   }
   help_row = row;

   if(script_file) {  // error was in a script file,  abandon script
      sprintf(out, "Unknown command seen in script file: %s line %d, col %d: %c", 
         script_name, script_line, script_col, script_err);
      vidstr(row, col,  HELP_COLOR, out);
      close_script(1);
      goto help_exit;
   }

   #ifdef ADEV_STUFF
      if(luxor == 0) {
         s = "a - select Adev type to display";
         vidstr(row++, col,  HELP_COLOR, s);
      }
   #endif

   if(luxor) {
      s = "b - Battery capacity/resistance";
      vidstr(row++, col,  HELP_COLOR, s);
   }
   else {
      s = "b - experimental OSC control PID";
      vidstr(row++, col,  HELP_COLOR, s);
   }

   s = "c - Clear collected data";
   vidstr(row++, col,  HELP_COLOR, s);

   if(luxor == 0) {
      s = "d - set osc Disciplining / DAC voltage";
      vidstr(row++, col,  HELP_COLOR, s);

      s = "e - save current config to EEPROM";
      vidstr(row++, col,  HELP_COLOR, s);
   }

   if(luxor) s = "f - change display Filter";
   else      s = "f - toggle Filter or signal/elev masks";
   vidstr(row++, col,  HELP_COLOR, s);

   s = "g - modify Graph or screen display";
   vidstr(row++, col,  HELP_COLOR, s);

   if(luxor == 0) {
      s = "h - toggle manual Holdover mode";
      vidstr(row++, col,  HELP_COLOR, s);
   }

   s = "i - set the plot queue update Interval";
   vidstr(row++, col,  HELP_COLOR, s);

   if(luxor == 0) {
      if(timing_mode & 0x02) s = "j - Jam sync PPS to UTC time";
      else                   s = "j - Jam sync PPS to GPS time";
      vidstr(row++, col,  HELP_COLOR, s);

      s = "k - temperature control PID";
      vidstr(row++, col,  HELP_COLOR, s);
   }

   s = "l - data Logging";
   vidstr(row++, col,  HELP_COLOR, s);

   if(luxor) {
      s = "m - change LED driver Mode";
      vidstr(row++, col,  HELP_COLOR, s);

      s = "o - Set misc options";
      vidstr(row++, col,  HELP_COLOR, s);

      s = "p - set Protection values";
      vidstr(row++, col,  HELP_COLOR, s);
   }
   else {
      s = "o - Set misc options";
      vidstr(row++, col,  HELP_COLOR, s);

      s = "p - PPS and TRAIM control";
      vidstr(row++, col,  HELP_COLOR, s);
   }

   s = "r - Read in a file";
   vidstr(row++, col,  HELP_COLOR, s);

   if(luxor) s = "s - enter position (lat lon alt)";
   else      s = "s - Survey/position/mode control";
   vidstr(row++, col,  HELP_COLOR, s);


   help_row_1 = row;      // switch to next column
   row = help_row;
   col = help_col + 40;


   s = "t - Time and temperature control";
   vidstr(row++, col,  HELP_COLOR, s);

   if(luxor) {
      if(pause_data) s = "u - resume plot queue Updates";
      else           s = "u - pause plot queue Updates";
      vidstr(row++, col,  HELP_COLOR, s);

      s = "v - set View time span of plot window";
      vidstr(row++, col,  HELP_COLOR, s);
   }
   else {
      if(pause_data) s = "u - resume plot/adev queue Updates";
      else           s = "u - pause plot/adev queue Updates";
      vidstr(row++, col,  HELP_COLOR, s);
   }

   if(luxor) {
      s = "w - Write data, screen, delete file";
      vidstr(row++, col,  HELP_COLOR, s);
   }
   else {
      s = "v - set View time span of plot window";
      vidstr(row++, col,  HELP_COLOR, s);

      s = "w - Write data, screen, delete file";
      vidstr(row++, col,  HELP_COLOR, s);
   }

   sprintf(out, "x - view graphs at 1 hr/div");
   vidstr(row++, col,  HELP_COLOR, out);

   if(day_size) sprintf(out, "y - view graphs at %d hr/screen", day_size);
   else         sprintf(out, "y - view graphs at %d hr/screen", 24);
   vidstr(row++, col,  HELP_COLOR, out);

   if(luxor) s = "z - Zoom fullscreen clocks";
   else      s = "z - Zoom fullscreen clocks and maps";
   vidstr(row++, col,  HELP_COLOR, s);

   s = "-+  lower/raise last selected graph";
   vidstr(row++, col,  HELP_COLOR, s);

   s = "{}  shrink/grow last selected graph";
   vidstr(row++, col,  HELP_COLOR, s);

   s = "! - device resets and diagnostics";
   vidstr(row++, col,  HELP_COLOR, s);

   if(luxor) s = "& - change calibration parameters";
   else      s = "& - change oscillator discipling parameters";
   vidstr(row++, col,  HELP_COLOR, s);

   s = "/ - startup (command line) only options";
   vidstr(row++, col,  HELP_COLOR, s);

   #ifdef GIF_FILES
      sprintf(out, "\\ - dump screen to %s.gif", unit_file_name);
   #else
      sprintf(out, "\\ - dump screen to %s.bmp", unit_file_name);
   #endif
   vidstr(row++, col,  HELP_COLOR, out);

   s = "$ - change screen resolution";
   vidstr(row++, col,  HELP_COLOR, s);

   #ifdef WINDOWS
//    s = "F11 - toggle fullscreen mode (BROKEN!)";
//    vidstr(row++, col,  HELP_COLOR, s);
   #endif

   s = "? - display command line help";
   vidstr(row++, col,  HELP_COLOR, s);



   if(text_mode) {  // full screen help
      col = help_col;
      row = help_row_1 + 1;
      help_row_1 = row;
   }
   else {           // help in the plot area
      row = help_row;
      col = help_col + 80;
   }

   s = "HOME - scroll plot to beginning of data";
   vidstr(row++, col,  HELP_COLOR, s);

   s = "END - scroll plot to end of data";
   vidstr(row++, col,  HELP_COLOR, s);

   s = "LEFT - scroll plot forward one division";
   vidstr(row++, col,  HELP_COLOR, s);

   s = "RIGHT - scroll plot back one division";
   vidstr(row++, col,  HELP_COLOR, s);

   s = "PG UP - scroll plot forward one screen";
   vidstr(row++, col,  HELP_COLOR, s);

   s = "PG DN - scroll plot back one screen";
   vidstr(row++, col,  HELP_COLOR, s);

   if(text_mode && (SCREEN_HEIGHT < 600)) {
      row = help_row_1;
      col = help_col + 40;
   }

   s = "UP - scroll plot forward one hour";
   vidstr(row++, col,  HELP_COLOR, s);

   s = "DOWN - scroll plot back one hour";
   vidstr(row++, col,  HELP_COLOR, s);

   s = "<> - scroll plot one day";
   vidstr(row++, col,  HELP_COLOR, s);

   s = "[] - scroll plot one pixel";
   vidstr(row++, col,  HELP_COLOR, s);

   s = "DEL - return plotting back to normal";
   vidstr(row++, col,  HELP_COLOR, s);

   s = "% - scroll to next error event";
   vidstr(row++, col,  HELP_COLOR, s);

   s = "0..9,=,+,- set or move to plot markers";
   vidstr(row++, col,  HELP_COLOR, s);

   ++row;
   col = help_col;
   s = "LEFT CLICK in plot area - zoom and center plot at mouse pointer";
   vidstr(row++, col, HELP_COLOR, s);
   s = "RIGHT BUTTON (at center of plot area) - scroll plot left or right";
   vidstr(row++, col, HELP_COLOR, s);
   s = "LEFT CLICK lat/lon info, sat info, plot header, adev table, maps, clock, watch";
   vidstr(row++, col, HELP_COLOR, s);
   s = "           to zoom them to full screen.  Click again to restore screen";
   vidstr(row++, col, HELP_COLOR, s);

   help_exit:
   refresh_page();
}


void second_key(int c)
{
   // process second  keystroke of a two key command
   if(c == first_key) {  // the user confirmed a dangerous single char command
      if(c != '?') BEEP(302);
   }
   else {  // single char command not confirmed
      if(script_file) {
         erase_plot(-1);
         sprintf(out, "Selection '%c' for command '%c' not recognized", c, first_key);
         edit_error(out);
      }
      if(text_mode) redraw_screen();
   }

   first_key = 0;  // clear message and resume graphing
   if(screen_type == 't') {
      no_x_margin = no_y_margin = 0;
   }
   else if(text_mode == 2) { // displaying full screen help
      text_mode = 0;
      no_x_margin = no_y_margin = 0;
if(0 && review_mode) end_review(0);
      redraw_screen();
   }

   if(text_mode) {
      redraw_screen();
   }
   else {
     erase_help();
     draw_plot(1);
   }
}


int are_you_sure(int c)
{
char *s1;
char *s2;
char *s3;
char *s4;
char *s5;
char *s6;
char s[SLEN+1];
u08 two_char;
char key[6];
COORD row, col;
int i;

   // This routine supports the two character keyboard commands.
   // It either prompts for the second character of the command or 
   // asks for confirmation on single character commands that do 
   // something semi-dangerous.

   s1 = s2 = s3 = s4 = s5 = s6 = 0;
   two_char = 0;
   if(c != '&') {   // command is not the osc parameter command
      if(osc_params) {      // we are showing the osc parameters
         redraw_screen();
         if(process_com) {
//----      osc_params = 0;    // disable the osc param display
         }
      }
   }
   else {
      show_satinfo();
   }

   if(first_key) {   // this is the second keystroke of a two char command
      second_key(c);
      return c;
   }

   // This is the first keystroke of a command.
   // Either the user wants to do something dangerous... so ask the annoying question
   // or else prompt for the second keystroke of the two character commands.

   first_key = c;    // first_key is first char of the command we are doing
   if(first_key != 'p') prot_menu = 0;
   erase_help();     // erase any old help info
   refresh_page();

   if(c == '?') {    // the user wants some help
      kbd_help();    // show the help
      return 0;
   }

   row = EDIT_ROW;
   col = EDIT_COL;
   if(text_mode) {
      erase_screen();
      if(osc_params) {  
         show_osc_params(row+8, col);
      }
   }

   if(c == ESC_CHAR) {
      s1 = "This will exit the program";
   }
#ifdef ADEV_STUFF
   else if(c == 'a') {
      if(adevs_active(0)) {
//       s1 = "Display adev type:  A)dev  H)dev  M)dev  T)dev    all O)sc  all P)ps";
         sprintf(s, "Display adev type:  A)dev  H)dev  M)dev  T)dev    all O)%s  all P)%s", plot[OSC].plot_id, plot[PPS].plot_id);
         s1 = &s[0];
         two_char = 1;
      }
      else {
         s1 = "Adev info not supported by this receiver... press ESC";
      }
   }
#endif
   else if(luxor && (c == 'b')) {
      s1 = "Battery:  I)nternal resistance measurement   P)WM resolution";
      s2 = "Constant: V)oltage  C)urrent     W)atts      S)weep pwm   R)ate";
      s3 = "Charge:   L)iPo     F)LiFePO4    H)igh voltage LiPo";
      two_char = 1;
   }
   else if(c == 'c') {
      if(luxor) {
         s1 = "Clear plot queue";
      }
      else if(rcvr_type == ACRON_RCVR) {
         s1 = "C)lear all data   P)lot queue";
      }
      else {
         s1 = "Clear A)dev queue only   B)oth plot and adev queues   C)lear all data";
         s2 = "      L)LA fix data      satellite M)ap data          P)lot queue data";
         s3 = "      R)eload adevs from displayed plot window data";
         two_char = 1;
      }
   }
   else if(c == 'd') {
      if((rcvr_type != TSIP_RCVR) && (rcvr_type != UCCM_RCVR)) {
         s1 = "Manual disciplining not supported by this receiver... press ESC";
      }
      else {
         s1 = "Oscillator discipline:  E)nable  D)isable    S)et DAC voltage";
         two_char = 1;
      }
   }
   else if(c == 'e') {
      if(((rcvr_type == TSIP_RCVR) && (tsip_type != STARLOC_RCVR)) || (rcvr_type == UBX_RCVR)) {
         if(no_eeprom_writes) s1 = "EEPROM updates disabled with /kc keyboard command.";
         else                 s1 = "This will save the current configuration into EEPROM";
         two_char = 1;
      }
      else if(rcvr_type == VENUS_RCVR) {
         if(no_eeprom_writes) s1 = "EEPROM updates disabled with /kc keyboard command.";
         else if(eeprom_save) s1 = "Press E to DISABLE configuration change writes to EEPROM (ESC ESC to abort)";
         else                 s1 = "Press E to ENABLE configuration change writes to EEPROM (ESC ESC to abort)";
         two_char = 1;
      }
      else {
         s1 = "EEPROM not supported by this receiver... press ESC";
      }
   }
   else if(c == 'f') {
      if(luxor) {
         s1 = "Enter filter to change: D)isplay";
      }
      else if(rcvr_type == ACRON_RCVR) {
         s1 = "Enter filter to change: D)isplay";
      }
      else if(rcvr_type == MOTO_RCVR) {
         s1 = "Enter filter to change: E)levation mask  D)isplay";
         s2 = "                        P)osition        M)arine velocity";
         s3 = "Corrections:            I)onosphere      T)roposphere";
      }
      else if((rcvr_type == NMEA_RCVR) || (rcvr_type == GPSD_RCVR)) {
         s1 = "Enter filter to change: D)isplay";
      }
      else if(rcvr_type == NVS_RCVR) {
         s1 = "Enter filter to change: D)isplay                 C)ordinate filter factor";
         s2 = "                        signal L)evel (in dBc)   E)levation mask (degrees)";
      }
      else if(rcvr_type == SIRF_RCVR) {
         s1 = "Enter filter to change: D)isplay";
         s2 = "                        signal L)evel (in dBc)   E)levation mask (degrees)";
      }
      else if(rcvr_type == TSIP_RCVR) {
         s1 =    "Enter filter to change: P)v  S)tatic  A)ltitude  K)alman     D)isplay";
         if(tsip_type == STARLOC_RCVR) ;
         else if((res_t && (res_t != RES_T)) || saw_icm) {
            s2 = "                        signal L)evel (in dBc)   E)levation mask (degrees)";
            s3 = "                        J)amming mode";
         }
         else {
            s2 = "                        signal L)evel (in amu)   E)levation mask (degrees)";
            s3 = "                        F)oliage mode    M)ovement   X)PDOP mask/switch";
         }
      }
      else if(rcvr_type == UBX_RCVR) {
         s1 = "Enter filter to change: D)isplay";
         s2 = "                        signal L)evel (in dBc)   E)levation mask (degrees)";
      }
      else if(rcvr_type == VENUS_RCVR) {
         s1 = "Enter filter to change: D)isplay";
         s2 = "                        signal L)evel (in dBc)   E)levation mask (degrees)";
         s3 = "                        J)amming mode";
      }
      else {
         s1 = "Enter filter to change: E)levation mask  D)isplay";
      }
      two_char = 1;
   }
   else if(c == 'g') {
      if(luxor) {
         s1 = "Display: C)apacity D)BATTv   O)BATTi   F)FT      T)emperature";
         s2 = "         P)lux     I)BATTw   J)LEDw    K)eff     L)CCT";
         s3 = "         1)lumens  2)LEDv    3)LEDi    4)PWM     5)TEMP2";
         s4 = "         6)BLUE    7)GREEN   8)RED     9)WHITE   0)AUXv";
         s6 = "         R)edraw   S)ound    W)atch    B)oth     G)raph title   A)prots";
         s5 = "         E)rrors   /)change statistic  ~)colors";
      }
      else if(rcvr_type == GPSD_RCVR) {
         s1 = "Display: A)dev  B)oth map & adev tables  C)onstellation   E)rrors   F)FT       G)raph title";
         s2 = "         H)oldover  L)ocation  M)ap,no adevs     O)bias   P)PS      Q)uality   R)edraw";
         s3 = "         S)ound  T)emperature  W)atch    X)dops  Z)clock  ~)colors  /)statistics";
         s4 = "Plot:    1)Latitude    2)Longitude   3)Altitude  V)LLA    6)dop     fI)x map";
      }
      else if(rcvr_type == MOTO_RCVR) {
         s1 = "Display: A)dev B)oth map & adev tables C)onstellation  D)sawtooth  E)rrors    F)FT   G)raph title";
         s2 = "         H)oldover  L)ocation  M)ap,no adevs  O)bias     P)accu    Q)uality   R)edraw";
         s3 = "         S)ound  T)emperature  W)atch  X)dops Y)filters  Z)clock   ~)colors   /)statistics";
         s4 = "Plot:    1)Latitude    2)Longitude   3)Altitude  V)LLA   6)dop     fI)x map";
      }
      else if(rcvr_type == NVS_RCVR) {
         s1 = "Display: A)dev B)oth map & adev tables C)onstellation  D)sawtooth E)rrors   F)FT   G)raph title";
         s2 = "         H)oldover  L)ocation  M)ap,no adevs   O)rate    P)rgen   Q)uality  R)edraw";
         s3 = "         S)ound  T)emperature  W)atch  X)dops  Y)filters Z)clock  ~)colors  /)statistics";
         s4 = "Plot:    1)Latitude   2)Longitude  3)Altitude  V)LLA     6)dop    fI)x map";
      }
      else if(rcvr_type == NMEA_RCVR) {
         s1 = "Display: A)dev  B)oth map & adev tables C)onstellation     E)rrors    F)FT    G)raph title";
         s2 = "         H)oldover  L)ocation  M)ap,no adevs    Q)uality   R)edraw";
         s3 = "         S)ound  W)atch  X)dops    Y)filters    Z)clock    ~)colors   /)statistics";
         s4 = "Plot:    1)Latitude   2)Longitude   3)Altitude  V)LLA      6)dop      fI)x map";
      }
      else if((rcvr_type == SCPI_RCVR) || (rcvr_type == UCCM_RCVR)) {
         s1 = "Display: A)dev  B)oth map & adev tables  C)onstellation  D)ac  E)rrors   F)FT   G)raph title";
         s2 = "         H)oldover  L)ocation      M)ap,no adevs   O)uncert    P)ps      Q)uality    R)edraw";
         s3 = "         S)ound  W)atch  X)dops    Y)filters       Z)clock     ~)colors  /)statistics";
         s4 = "Plot:    1)Latitude   2)Longitude  3)Altitude      V)LLA       4)tfom    5)ffom      fI)x map";
      }
      else if(rcvr_type == SIRF_RCVR) {
         s1 = "Display: A)dev  B)oth map & adev tables C)onstellation   D)sawtooth   E)rrors    F)FT   G)raph title";
         s2 = "         H)oldover     L)ocation  M)ap,no adevs   O)rate     P)drift  Q)uality   R)edraw";
         s3 = "         S)ound  T)emperature  W)atch  X)dops     Y)filters  Z)clock  ~)colors   /)statistics";
         s4 = "Plot:    1)Latitude    2)Longitude   3)Altitude   V)LLA      6)dop    fI)x map";
      }
      else if(rcvr_type == STAR_RCVR) {
         s1 = "Display: A)dev   B)oth map & adev tables   C)onstellation    E)rrors    F)FT      G)raph title";  
         s2 = "         H)oldover   L)ocation     M)ap,no adevs  Q)uality   R)edraw";
         s3 = "         S)ound      T)emperature  W)atch         Y)filters  Z)clock    ~)colors  /)statistics";
         s4 = "Plot:    1)Latitude  2)Longitude   3)Altitude     V)LLA      fI)x map";
      }
      else if(rcvr_type == UBX_RCVR) {
         s1 = "Display: A)dev B)oth map & adev tables C)onstellation  D)sawtooth  E)rrors  F)FT      G)raph title";
         s2 = "         H)oldover   L)ocation    M)ap,no adevs    O)frac    P)accu         Q)uality  R)edraw";
         s3 = "         S)ound  T)emperature  W)atch  Y)filters   Z)clock   ~)colors       /)statistics";
         s4 = "Plot:    1)Latitude   2)Longitude   3)Altitude     V)LLA     fI)x map";
      }
      else if(rcvr_type == ZODIAC_RCVR) {
         s1 = "Display: A)dev  B)oth map & adev tables C)onstellation    E)rrors   F)FT      G)raph title";
         s2 = "         H)oldover  L)ocation  M)ap,no adevs   O)bias     P)PS      Q)uality  R)edraw";
         s3 = "         S)ound  T)emperature  W)atch  X)dops  Y)filters  Z)clock   ~)colors  /)statistics";
         s4 = "Plot:    1)Latitude  2)Longitude  3)Altitude   V)LLA      6)dop     fI)x map";
      }
      else if(TIMING_RCVR) {
         s1 = "Display: A)dev B)oth map & adev tables C)onstellation  D)sawtooth   E)rrors    F)FT   G)raph title";
         s2 = "         H)oldover  L)ocation  M)ap,no adevs   O)rate     P)bias    Q)uality   R)edraw";
         s3 = "         S)ound  T)emperature  W)atch  X)dops  Y)filters  Z)clock   ~)colors   /)statistics";
         s4 = "Plot:    1)Latitude  2)Longitude  3)Altitude   V)LLA      6)dop     fI)x map";
      }
      else {
         s1 = "Display: A)dev  B)oth map & adev tables  C)onstellation   D)ac      E)rrors    F)FT  G)raph title";
         s2 = "         H)oldover  L)ocation  M)ap,no adevs   O)sc       P)ps      Q)uality   R)edraw";
         s3 = "         S)ound  T)emperature  W)atch  X)dops  Y)filters  Z)clock   ~)colors   /)statistics";
         s4 = "Plot:    1)Latitude  2)Longitude  3)Altitude   V)LLA      6)dop     fI)x map";
      }
      two_char = 1;
   }
   else if(c == 'h') {
      if(GPSDO) {
                            s1 = "E)nter holdover    eX)it holdover";
//       if(user_holdover)  s2 = "Press H again to exit user Holdover mode";
//       else               s2 = "Press H again to enter user Holdover mode";
         two_char = 1;
      }
      else {
         s1 = "Holdover control not supported by this receiver... press ESC";
         two_char = 1;
      }
   }
   else if(c == 'j') {
      if(GPSDO && (rcvr_type != STAR_RCVR) && (rcvr_type != UCCM_RCVR)) {
         if(timing_mode & 0x02)   s1 = "This will Jam sync the PPS output to UTC time";
         else                     s1 = "This will Jam sync the PPS output to GPS time";
      }
      else {
         s1 = "Jam sync not supported by this receiver... press ESC";
         two_char = 1;
      }
   }
   else if(c == 'l') {
      if(log_file) s1 = "S)top logging data to file";
      else         s1 = "Log:  A)ppend to file   W)rite to file   D)elete file   I)nterval";
      if(log_db)   s2 = "      stop C)onstellation data in log file";
      else         s2 = "      add C)onstellation data to log file";
      two_char = 1;
   }
   else if(luxor && (c == 'p')) {
      s1 = "Battery:     H)igh voltage cutoff  L)ow voltage cutoff   W)atts   C)over current";
      s2 = "LED:         V)over voltage        U)nder voltage        P)ower   O)ver current";
      s3 = "AUXV:        X)over voltage        Z)under voltage";
      s4 = "Temperature: T)emp1 overtemp       S)temp2 overtemp";
      s5 = "Misc:        M)sg timeout          R)eset protection faults";
      prot_menu = 1;
      two_char = 1;
   }
   else if(c == 'p') {
      if(res_t) {
         s1 = "PPS output:  1)PPS mode     2)PP2S mode";
         s2 = "             R)ising edge   F)alling edge  toggle PPS edge P)olarity";
         s3 = "             D)isable PPS   E)nable PPS    toggle PP(S) enable";
         s4 = "Signals:     C)able delay";           
         two_char = 1;
      }
      else if(rcvr_type == ACRON_RCVR) { 
         s1 = "PPS control not supported by this receiver... press ESC";
      }
      else if(rcvr_type == MOTO_RCVR) {
         s1 = "PPS output:  1)PPS mode     2)100 PPS mode";
         s2 = "             D)isable PPS   E)nable PPS   toggle PP(S) enable";
         s3 = "Signals:     C)able delay   PPS O)ffset";           
         s4 = "TRAIM:       T)raim threshold";
         two_char = 1;
      }
      else if(rcvr_type == NMEA_RCVR) { 
         s1 = "PPS control not supported by this receiver... press ESC";
      }
      else if(rcvr_type == NVS_RCVR) { 
         s1 = "PPS output:  R)ising edge   F)alling edge  toggle PPS edge P)olarity";
         s2 = "             A)PPS width";
         s3 = "Signals:     C)able delay";           
         s4 = "TRAIM:       T)raim control";
      }
      else if(rcvr_type == SCPI_RCVR) {
         if(scpi_type == HP_TYPE) {
            s1 = "PPS output:  R)ising edge   F)alling edge  toggle PPS edge P)olarity";
            s2 = "Signals:     C)able delay";           
         }
         else {
            s1 = "Signals:     C)able delay";           
         }
         two_char = 1;
      }
      else if(rcvr_type == SIRF_RCVR) { 
         s1 = "PPS control not supported by this receiver... press ESC";
      }
      else if(rcvr_type == STAR_RCVR) { 
         s1 = "Signals:     C)able delay";           
         if(have_pps_rate) s2 = "TOD output:  1)enable      2)disable";           
      }
      else if(rcvr_type == UCCM_RCVR) {
         s1 = "PPS output:  D)isable PPS   E)nable PPS      toggle PP(S) enable";
         s2 = "PPS rate:    1)PPS mode     2)PP2S mode";
         s3 = "Signals:     C)able delay";           
         two_char = 1;
      }
      else if(rcvr_type == UBX_RCVR) {
         s1 = "PPS output:  R)ising edge   F)alling edge    toggle PPS edge P)olarity";
         s2 = "             D)isable PPS   E)nable PPS      toggle PP(S) enable";
         s3 = "PPS rate:    1)PPS mode     2)100 PPS mode   3)1000 PPS mode";
         if(saw_ubx_tp5) {
            s4 = "             A)PPS1 freq    B)PPS2 freq";
            s5 = "Signals:     C)able delay   PPS O)ffset";           
         }
         else {
            s4 = "Signals:     C)able delay   PPS O)ffset";           
         }
         two_char = 1;
      }
      else if(rcvr_type == VENUS_RCVR) {
         if(saw_timing_msg) {
            s1 = "PPS output:  B)PPS2 freq and duty cycle";
            s2 = "Signals:     C)able delay";           
            s3 = " ";
            s4 = "Note: Max 100000 us pulse width, 19.2 Mhz freq";
         }
         else if(have_pps_freq) {
            s1 = "PPS output:  A)PPS1 freq and duty cycle";
            s2 = "Signals:     C)able delay";           
            s3 = " ";
            s4 = "Note: Max 100000 us pulse width, 10 Mhz freq";
         }
         else {
            s1 = "PPS output:  A)PPS1 duty cycle";
            s2 = "Signals:     C)able delay";           
            s3 = " ";
            s4 = "Note: Max 100000 us pulse width. Freq fixed to 1 Hz";
         }
         two_char = 1;
      }
      else if(rcvr_type == ZODIAC_RCVR) {
         s1 = "PPS output:  1)PPS mode       2)100 PPS mode";
         s2 = "             D)isable PPS     E)nable PPS     toggle PP(S) enable";
         s4 = "Signals:     C)able delay     PPS O)ffset";           
         s5 = "TRAIM:       T)raim threshold";
         two_char = 1;
      }
      else {
         s1 = "PPS output:  R)ising edge   F)alling edge  toggle PPS edge P)olarity";
         s2 = "             D)isable PPS   E)nable PPS    toggle PP(S) enable";
         s3 = "OSC output:  toggle O)sc polarity";
         s4 = "Signals:     C)able delay";           
         two_char = 1;
      }
   }
   else if(c == 's') {
      #ifdef PRECISE_STUFF
         if     (check_precise_posn) s6 = "Enter L to abort the precise lat/lon/altitude save (ESC to ignore)";
         else if(precision_survey)   s6 = "Enter P to abort the Precison survey (ESC to ignore)";
         else if(doing_survey)       s6 = "Enter S to abort the Standard survey (ESC to ignore)";
         else if(show_fixes)         s6 = "Enter F to return to overdetermined clock mode (ESC to ignore)";

         if(rcvr_type == ACRON_RCVR) { 
            s1 = "enter L)at/lon/alt   A)ntenna signal level maps";
            s2 = "sat I)nfo display";
         }
         else if(rcvr_type == MOTO_RCVR) {
            if(moto_chans < 12) {  // these don't do self-surveys
               s1 = "P)recision median survey";
            }
            else {
               s1 = "S)tandard survey     P)recision median survey";
            }
            s2 = "enter L)at/lon/alt   A)ntenna signal level maps";
            s3 = "position H)old mode  N)3d mode    O)ne sat mode   eX)clude sat";
            s4 = "do 2D/3d F)ixes      3)2D fixes only   4)3D fixes";
            s5 = "sat I)nfo display";
         }
         else if((rcvr_type == NMEA_RCVR) || (rcvr_type == GPSD_RCVR)) {
            s1 = "P)recision survey    A)ntenna signal level maps";
            s2 = "sat I)nfo display";
         }
         else if((rcvr_type == TSIP_RCVR) && (tsip_type == STARLOC_RCVR)) {
            s1 = "S)tandard survey     P)recision median survey";
            s2 = "enter L)at/lon/alt   A)ntenna signal level maps";
            s3 = "eX)clude satellite   sat I)nfo display";
         }
         else if(rcvr_type == NVS_RCVR) {
            s1 = "S)tandard survey      P)recision median survey";
            s2 = "enter L)at/lon/alt    A)ntenna signal level maps";
            s3 = "position H)old mode   N)3d mode    eX)clude sat";
            s4 = "select G)nss systems  sat I)nfo display";
         }
         else if(rcvr_type == SCPI_RCVR) {
            s1 = "S)tandard survey    (ESC to ignore)";
            s2 = "enter L)at/lon/alt   A)ntenna signal level maps";
            s3 = "O)ne sat mode        eX)clude sat";
            s4 = "sat I)nfo display";
         }
         else if(rcvr_type == SIRF_RCVR) {
            s1 = "P)recision median survey";
            s2 = "A)ntenna signal level maps";
            s3 = "sat I)nfo display";
         }
         else if(rcvr_type == STAR_RCVR) {
            s1 = "A)ntenna signal level maps";
            s2 = "sat I)nfo display";
         }
         else if(rcvr_type == UBX_RCVR) {
            if(saw_timing_msg) s1 = "S)tandard survey       P)recision median survey";
            else               s1 = "P)recision median survey";
            s2 =                    "enter L)at/lon/alt     A)ntenna signal level maps";
            s3 =                    "position H)old mode    N)avigation (3D) mode";
            s4 =                    "select G)nss systems   sat I)nfo display";
         }
         else if(rcvr_type == UCCM_RCVR) {
            s1 = "S)tandard survey     (ESC to ignore)";
            s2 = "enter L)at/lon/alt   A)ntenna signal level maps";
            s3 = "O)ne sat mode        eX)clude sat";
            s4 = "sat I)nfo display";
         }
         else if(rcvr_type == VENUS_RCVR) {
            if(saw_timing_msg) {
               s1 = "S)tandard survey        P)recision median survey";
               s2 = "enter L)at/lon/alt      A)ntenna signal level maps";
               s3 = "position H)old mode     N)3d navigation mode";
               s4 = "select G)nss systems    sat I)nfo display";
            }
            else {
               s1 = "P)recision median survey";
               s2 = "A)ntenna signal level maps";
               s3 = "select G)nss systems    sat I)nfo display";
            }
         }
         else if(rcvr_type == ZODIAC_RCVR) {
            s1 = "S)tandard survey     P)recision median survey";
            s2 = "enter L)at/lon/alt   A)ntenna signal level maps";
            s3 = "position H)old mode  N)avigation (3D) mode";
            s4 = "O)ne sat mode        eX)clude sat";
            s5 = "sat I)nfo display";
         }
         else {
            s1 = "S)tandard survey       P)recision median survey";
            s2 = "enter L)at/lon/alt     A)ntenna signal level maps";
            s3 = "position H)old mode    N)3d mode    O)ne sat mode   eX)clude sat";
            s4 = "do 2D/3d F)ixes        3)D fixes only   0)..7) set receiver mode";
            if(saw_icm || (res_t == RES_T_360)) {
              s5 = "select G)nss systems   sat I)nfo display";
            }
            else s5 = "sat I)nfo display";
         }
      #else
         s1 = "S)tandard survey     (ESC to ignore)";
         s2 = "enter L)at/lon/alt   A)ntenna signal level maps";
         s3 = "do 2D/3d F)ixes      3)D fixes only     0)..7) set receiver mode";
         s4 = "O)ne sat mode        eX)clude sat";
         s5 = "sat I)nfo display";
      #endif

      if(luxor) ;
      else two_char = 1;
   }
   else if(c == 't') {
      if(luxor) {
         s1  = "A)larm time   cH)ime clock";
         #ifdef TEMP_CONTROL
            s2 = "eX)it time    time Z)one      T)emperature control";
         #else
            s2 = "eX)it time    time Z)one";
         #endif
         s3 = "screen D)ump time    L)og dump time    set tt-ut1 dE)lta-t";
      }
      else {
         s1  =      "A)larm time   cH)ime clock    use G)PS time   use U)TC time";
         #ifdef TEMP_CONTROL
            if(have_temperature && (tsip_type != STARLOC_RCVR)) {
               s2 = "eX)it time    time Z)one      S)et system time    T)emperature control";
            }
            else {
               s2 = "eX)it time    time Z)one      S)et system time";
            }
         #else
            s2 =    "eX)it time    time Z)one      S)et system time";
         #endif
         s3 =       "screen D)ump time    L)og dump time    set tt-ut1 dE)lta-t";
         s4 =       "Digital clock:  M)illisecond   tW)elve hour   J)ulian";
         s5 =       "show sun/moon R)ise info";

         if(rcvr_type == SCPI_RCVR) {
            s6 = "CHANGING BETWEEN UTC/GPS TIME WILL RESTART THE RECEIVER!";
         }
      }
      two_char = 1;
   }
   else if(c == 'u') {
      if(pause_data) s1 = "This will resume plot/adev queue Updates";
      else           s1 = "This will pause plot/adev queue Updates";
   }
   else if(c == 'w')  {
      if(filter_log) s1 = "Write to file:  A)ll queue data   P)lot area    D)elete file   unF)iltered data";
      else           s1 = "Write to file:  A)ll queue data   P)lot area    D)elete file   F)iltered data";
      if(luxor) {
         s2 =             "                S)creen dump      R)everse video screen dump   L)og";
         s4 =             "                C)onfig data      &)calibration data";
      }
      else {
         s2 =             "                S)creen dump      R)everse video screen dump   L)og  aZ)el data";
      }
      s3 =                "                G)raph area dump  I)nverse video graph area dump";
      s4 =                "                X)debug info      Y)raw receiver data          Z)signal levels";
      two_char = 1;
   }
   else if(c == 'z') {
      if(zoom_screen == 0) {
         if(luxor) {
            s1 = "Zoom:  C)lock   W)atch    N)ormal    P)lots";
         }
         else if(rcvr_type == NO_RCVR) {
            s1 = "Zoom:  C)lock   W)atch    B)watch with sun   N)ormal";
         }
         else {
            s1 = "Zoom:  C)lock     W)atch     M)ap       B)oth watch and map";
            s2 = "       L)LA       N)ormal    P)lots     sat I)nfo";
            s3 = "       S)ignals   A)zimuth   E)levation     R)elative   D)ata   U)all";
            s4 = "       X)watch/map/signals   Y)map/signals  V)watch+sats+sigs";
         }
         two_char = 1;
      }
   }
   else if(c == '&') {
      if(luxor) {
         s1 = "Light sens:  L)ux  U)lumens";
         s2 = "IR sensor:   E)IR1 emissivity   I)IR2 emissivity";
         s3 = "Mode:        C)alibration mode";
         two_char = 1;
      }
      else if(res_t) {
         s1 = "Tune: A)utotune elevation and signal level masks";
         two_char = 1;
      }
      else if(rcvr_type == MOTO_RCVR) {
         s1 = "Tune: A)utotune elevation and signal level masks";
         two_char = 1;
      }
      else if(rcvr_type == NVS_RCVR) {
         s1 = "Tune: A)utotune elevation and signal level masks";
         two_char = 1;
      }
      else if(rcvr_type == SCPI_RCVR) {
         s1 = "Tune:        A)utotune elevation mask";
         two_char = 1;
      }
      else if(rcvr_type == SIRF_RCVR) {
         s1 = "Tune: A)utotune elevation and signal level masks";
         two_char = 1;
      }
      else if(rcvr_type == STAR_RCVR) {
         s1 = "Tune: A)utotune elevation and signal level masks";
         if(have_gpsdo_ref) s2 = "Osc:  T)ime constant   R)eference source";
         else               s2 = "Osc:  T)ime constant";
         two_char = 1;
      }
      else if(rcvr_type == UBX_RCVR) {
         s1 = "Tune: A)utotune elevation and signal level masks";
         two_char = 1;
      }
      else if(rcvr_type == UCCM_RCVR) {
         s1 = "Tune:        A)utotune elevation mask";
         s2 = "Oscillator:  P)ullin range";
         two_char = 1;
      }
      else if(rcvr_type == ZODIAC_RCVR) {
         s1 = "Tune: A)utotune elevation and signal level masks";
         two_char = 1;
      }
      else if(GPSDO && (tsip_type != STARLOC_RCVR)) {
         s1 = "EFC voltage:  I)initial  mi(N)  ma(X)   or   A)utotune osc params";
         s2 = "Oscillator:   D)amping   max F)req   G)ain   J)am thresh   T)ime const";
         two_char = 1;
      }
      else {
         s1 = "Oscillator control not supported by this receiver... press ESC";
         osc_params = 0;
      }
   }
   else if(c == '!') {
      if(luxor) s1 = "W)arm reset   C)old reset   H)ard reset to factory settings";
      else {
         if(rcvr_type == ACRON_RCVR) {
            s1 = "W) request receiver reception attempt";
         }
         else if(rcvr_type == MOTO_RCVR) {
            s1 = "H)ard reset to factory settings";
         }
         else if(rcvr_type == NVS_RCVR)  {
            s1 = "W)arm reset   H)ard reset to factory settings"; 
         }
         else if(rcvr_type == STAR_RCVR) {
            if(star_type == NEC_TYPE) s1 = 0;
            else s1 = "W)arm reset   H)ard reset to factory settings"; 
         }
         else if((rcvr_type == SCPI_RCVR) || (rcvr_type == UCCM_RCVR)) {
            s1 = "C)old reset   H)ard reset to factory settings"; 
         }
         else if((rcvr_type == TSIP_RCVR) || (rcvr_type == UBX_RCVR) || (rcvr_type == VENUS_RCVR) || (rcvr_type == ZODIAC_RCVR)) {  // moto tsip ubx zodiac
            s1 = "W)arm reset   C)old reset   H)ard reset to factory settings"; 
         }
         else s1 = "";

         s2 =    "re-init S)erial port   set serial P)ort config";
         s3 =    "send B)reak            T)erminal emulator       Z)reset parser";

         if(rcvr_type == ACRON_RCVR) {
            s4 = "send U)ser command to receiver"; 
         }
         else if(rcvr_type == GPSD_RCVR) {
            s4 = "send U)ser command to receiver"; 
         }
         else if(rcvr_type == MOTO_RCVR) {
            s4 = "switch receiver language M)ode";
            s5 = "run receiver D)iagnostics";
         }
         else if(rcvr_type == NMEA_RCVR) {
            s4 = "send U)ser command to receiver"; 
         }
         else if(rcvr_type == NO_RCVR) {
            s4 = "set clock update R)ate";
         }
         else if(rcvr_type == NVS_RCVR) {
            s4 = "run receiver D)iagnostics      set nav R)ate";
         }
         else if(rcvr_type == SCPI_RCVR) {
            s4 = "run receiver D)iagnostics"; 
            s5 = "send U)ser command to receiver"; 
         }
         else if(rcvr_type == SIRF_RCVR) {
            s4 = "switch receiver to NMEA M)ode";
         }
         else if(rcvr_type == STAR_RCVR) {
            s4 = "send U)ser command to receiver"; 
         }
         else if(rcvr_type == UBX_RCVR) {
            s4 = "set nav R)ate          switch receiver to NMEA M)ode";
         }
         else if(rcvr_type == UCCM_RCVR) {
            if(scpi_type == UCCMP_TYPE) s4 = "run receiver D)iagnostics"; 
            s5 = "send U)ser command to receiver"; 
         }
         else if(rcvr_type == VENUS_RCVR) {
            s4 = "set nav R)ate          switch receiver to NMEA M)ode";
         }
         else if(rcvr_type == ZODIAC_RCVR) {
            s4 = "switch to Motorola M)ode";
            s5 = "run receiver D)iagnostics";
            s6 = "send U)ser command to receiver"; 
         }
      }
      two_char = 1;
   }
   else if(c == '$') {
      s1 = "Select screen size:   T)ext      U)ndersized ";
      s2 = "   S)mall   M)edium   N)etbook   L)arge        V)ery large";
      s3 = "   eX)tra large       H)uge      Z)humongous   C)ustom     F)ull";
      s4 = "   I)nvert black and white on screen";
      two_char = 1;
   }

   // display the command menu options
   i = 0;
   --row;
   if(s1) vidstr(row+0, col, PROMPT_COLOR, s1);
   if(s2) vidstr(row+1, col, PROMPT_COLOR, s2);
   if(s3) vidstr(row+2, col, PROMPT_COLOR, s3);
   if(s4) { vidstr(row+3, col, PROMPT_COLOR, s4); i = 1; }
   if(s5) { vidstr(row+4, col, PROMPT_COLOR, s5); i = 2; }
   if(s6) { vidstr(row+5, col, PROMPT_COLOR, s6); i = 3; }

   if(two_char == 0) {     // it is a single character keyboard command
      if(c == ESC_CHAR) {  // user wants to exit
         if(esc_esc_exit) sprintf(out, "Press 'y' or ESC if you are sure...");
         else             sprintf(out, "Press 'y' if you are sure...");
      }
      else {
         key[0] = first_key;
         key[1] = 0;
         sprintf(out, "Press '%s' again if you are sure...", key);
      }
      vidstr(row+3+i, col, PROMPT_COLOR, out);
   }

   vidstr(row+4+i, col, PROMPT_COLOR, "Press any other key to ignore...");
   refresh_page();

   return 0;
}

u08 edit_string(int c)
{
int row, col;
int attr;
char s[2];

   // perform an edit operation on the text string that we are building

   edit_ptr = strlen(edit_buffer);

   if(c == INS_CHAR) { // toggle insert mode
      insert_mode ^=1;
      first_edit = 0;
   }
   else if(c == LEFT_CHAR) {   // move cursor left
      if(edit_cursor) --edit_cursor;
      else BEEP(303);
      first_edit = 0;
   }
   else if(c == RIGHT_CHAR) {  // move cursor right
      if(++edit_cursor >= edit_ptr) {  // if past end of string, append blanks
         c = ' ';
         goto add_char;
      }
      first_edit = 0;
   }
   else if(c == END_CHAR) {  // move end-of-line
      edit_cursor = edit_ptr;
      first_edit = 0;
   }
   else if(c == HOME_CHAR) {  // move to start of line
      edit_cursor = 0;
      first_edit = 0;
   }
   else if(c == DOWN_CHAR) {  // delete to end-of-line
      edit_buffer[edit_cursor] = 0;
      first_edit = 0;
   }
   else if(c == UP_CHAR) {  // delete to start-of-line
      strcpy(out, &edit_buffer[edit_cursor+1]);
      strcpy(edit_buffer, out);
      edit_cursor = edit_ptr = 0;  // strlen(edit_buffer);
      first_edit = 0;
   }
   else if(c == DEL_CHAR) {  // delete char at cursor
      delete_char:
      if(edit_buffer[edit_cursor]) {
         strcpy(out, edit_buffer);
         strcpy(&edit_buffer[edit_cursor], &out[edit_cursor+1]);
         edit_ptr = strlen(edit_buffer);
      }
      else if(edit_cursor) {
         --edit_cursor;
         goto delete_char;
      }
      else BEEP(304);
      first_edit = 0;
   }
   else if(c == 0x08) { // back up and delete a char
      if(edit_cursor) {
         --edit_cursor;
         goto delete_char;
      }
      else BEEP(305);
   }
   else if(c >= 0x0100) {  // ignore unused cursor and function keys
   }
   else if(c == ESC_CHAR) {  // erase edit buffer
      if(edit_ptr) {  // if something is in the buffer,  erase the buffer
         edit_ptr = edit_cursor = c = 0;
         edit_buffer[edit_ptr] = 0;
      }
      else {  // ESC with nothing in the buffer,  abandon the edit
         first_edit = 0;
         no_auto_erase = 0;
         if(luxor && (prot_menu || show_prots)) {
            prot_menu = 0;
            redraw_screen();
         }
         return ESC_CHAR;
      }
   }
   else if(c == 0x0D) {  // end of edit
      script_pause = 0;
      first_edit = 0;
   }
   else if(c) {  // normal text char - insert it into the buffer
      if(first_edit && (no_auto_erase == 0)) {  // first char typed erases old buffer contents
         edit_ptr = 0;
         edit_buffer[edit_ptr++] = c;
         edit_buffer[edit_ptr] = 0;
         edit_cursor = edit_ptr;
      }
      else if(insert_mode) {
         if(edit_ptr >= (SLEN-1)) {  // gone too far
            edit_ptr = (SLEN-1);
            edit_buffer[edit_ptr] = 0;
            BEEP(306);
         }
         else if(edit_ptr >= (TEXT_COLS-3)) {  // gone too far
            edit_ptr = (TEXT_COLS-3);
            edit_buffer[edit_ptr] = 0;
            BEEP(307);
         }
         else {
            strcpy(out, &edit_buffer[edit_cursor]);
            edit_buffer[edit_cursor] = c;
            strcpy(&edit_buffer[edit_cursor+1], out);
            ++edit_cursor;
            edit_ptr = strlen(edit_buffer);
         }
      }
      else {  // overwrite mode
         add_char:
         if(edit_cursor >= edit_ptr) {  // append char to end of string
            if(edit_ptr >= (SLEN-1)) {  // gone too far
               edit_ptr = (SLEN-1);
               edit_buffer[edit_ptr] = 0;
               edit_cursor = edit_ptr;
               BEEP(308);
            }
            else if(edit_ptr >= (TEXT_COLS-3)) {  // gone too far
               edit_ptr = (TEXT_COLS-3);
               edit_buffer[edit_ptr] = 0;
               edit_cursor = edit_ptr;
               BEEP(309);
            }
            else {
               edit_buffer[edit_ptr] = c;
               edit_buffer[++edit_ptr] = 0;
               edit_cursor = edit_ptr;
               edit_ptr = strlen(edit_buffer);
            }
         }
         else {  // replace char in the string at the cursor
            edit_buffer[edit_cursor++] = c;
         }
      }

      c = 0;
      first_edit = 0;
   }

   if(first_edit == 0) no_auto_erase = 0;

   dos_text_mode = 0;
   attr = WHITE;

   col = TEXT_COLS-SLEN-2;
   if(col < 0) col = 0;
   vidstr(e_row, EDIT_COL, WHITE, &blanks[col]);

   // kludgy,  but no-brainer way to erase old edit cursor
   col = (EDIT_COL+1+edit_cursor) * TEXT_WIDTH;
   row = e_row * TEXT_HEIGHT;
   if(row < PLOT_TEXT_ROW) {  // undo Windows margin offsets
      if(no_x_margin == 0) col += TEXT_X_MARGIN;
      if(no_y_margin == 0) row += TEXT_Y_MARGIN;
   }
   if(dos_text_mode == 0) {
      line(0,row+TEXT_HEIGHT, SCREEN_WIDTH-1,row+TEXT_HEIGHT, BLACK);
      line(0,row+TEXT_HEIGHT+2, SCREEN_WIDTH-1,row+TEXT_HEIGHT+2, BLACK);
   }

   sprintf(out, ">%s", edit_buffer);
   vidstr(e_row, EDIT_COL, WHITE, out);
   if(c != 0x0D) {  // draw the edit cursor under the char
      if(dos_text_mode) {
         if(insert_mode) attr = CYAN<<4;
         else            attr = WHITE<<4;
         s[0] = edit_buffer[edit_cursor];
         if(s[0] == 0) s[0] = ' ';
         s[1] = 0;
         blank_underscore = 0;
         vidstr(row/TEXT_HEIGHT, col/TEXT_WIDTH, attr, s);
         blank_underscore = 1;
      }
      else {
         line(col,row+TEXT_HEIGHT, col+TEXT_WIDTH-1,row+TEXT_HEIGHT, PROMPT_COLOR);
         if(insert_mode) line(col,row+TEXT_HEIGHT+2, col+TEXT_WIDTH-1,row+TEXT_HEIGHT+2, PROMPT_COLOR);
      }
   }

   if(0 && script_file && (script_pause == 0)) ;  // this makes script go faster,  but you can't see what is happening
   else refresh_page();

   return c;
}




#define BLANK_LINES_OK ((getting_string != ':') && (getting_string != '/') && (getting_string != '*') && (getting_string != '~') && (getting_string != '!') && (getting_string != '('))

int blank_lines_ok(int c)
{
   if(c == ':') return 0;
   if(c == '/') return 0;
   if(c == '*') return 0;
   if(c == '~') return 0;
   if(c == '!') return 0;
   if(c == '(') return 0;
   if((c >= 'A') && (c <= 'Z')) return 0;
   if(c == 0x0100) return 0;
   if(c == SUN_CMD) return 0;

   return 1;
}

int start_edit(int c, char *prompt)
{
int i, col;
int row;

   //
   // Start building the text string for command parameter *c* 
   //
   // Note that blank lines are allowed as input and are processed only 
   // if *c* is an upper case character (A-Z), '*', ':', '~', '!', '(',  or '/'.
   // Search for BLANK_LINES_OK to find where to add more "blank ok" lines
   //
   insert_mode = 0;
   if(script_file) edit_buffer[0] = 0;
   edit_cursor = strlen(edit_buffer);   // user can prime the buffer with data
   getting_string = c;
   if(c < 0x0100) first_key = tolower(c);
   else           first_key = c;

   if(text_mode) erase_screen();
   else          erase_help();

   // display prompt string for the command
   vidstr(EDIT_ROW, EDIT_COL, PROMPT_COLOR, prompt);

   // special cases for multiple line prompts
   e_row = EDIT_ROW+2;
   row = EDIT_ROW+1;
   if((c == 'g') || (c == PALETTE_CMD)) {  // display color palette selections
      col = EDIT_COL + strlen(prompt);
      for(i=1; i<16; i++) {
         sprintf(out, " %2d", i);
         vidstr(EDIT_ROW, col+((i-1)*3), i, out);
      }
   }

   if(edit_info1) {
      vidstr(row, EDIT_COL, PROMPT_COLOR, edit_info1);
      ++row;
      ++e_row;
      edit_info1 = 0;
   }
   if(edit_info2) {
      vidstr(row, EDIT_COL, PROMPT_COLOR, edit_info2);
      ++row;
      ++e_row;
      edit_info2 = 0;
   }
   if(edit_info3) {
      vidstr(row, EDIT_COL, PROMPT_COLOR, edit_info3);
      ++row;
      ++e_row;
      edit_info3 = 0;
   }
   if(edit_info4) {
      vidstr(row, EDIT_COL, PROMPT_COLOR, edit_info4);
      ++row;
      ++e_row;
      edit_info4 = 0;
   }

   first_edit = 1;
   edit_string(0);  // (0) param just prints >_
   return 0;
}

int edit_error(char *s)
{
int x;
char out[256+1];
int row,col,width;

   if(reading_lla && (zoom_screen == 'L')) {
      row = TEXT_ROWS-4;
      col = TEXT_COLS/2;
      width = TEXT_COLS/2;
   }
   else {
      row = EDIT_ROW+3;
      col = EDIT_COL;
      width = 80;
   }

   // show an error message and wait for a key press
   vidstr(row+0, col, PROMPT_COLOR, &blanks[TEXT_COLS-width]);
   vidstr(row+1, col, PROMPT_COLOR, &blanks[TEXT_COLS-width]);

   if(script_file) {
      sprintf(out, "Error in script file %s: line %d, col %d: %s", 
         script_name, script_line, script_col, s);
      vidstr(row+0, col, PROMPT_COLOR, out);
      vidstr(row+1, col, PROMPT_COLOR, "Press any key to stop script....");
   }
   else {
      vidstr(row+0, col, PROMPT_COLOR, s);
      vidstr(row+1, col, PROMPT_COLOR, "Press any key to continue....");
   }

   refresh_page();

   //!!! just waiting for a key here would be bad news because
   //    gps data would not be processed and the queue can overflow
//if(rcvr_type == ACRON_RCVR) Sleep(1000); else
   wait_for_key(1);

   vidstr(row+1, col, PROMPT_COLOR, "                                ");
   refresh_page();

   if(KBHIT()) x = GETCH();
   else        x = 0;
   script_fault = 1;
   return x;  
}

int build_string(int c)
{
u32 val;
int not_upper;

   val = toupper(c & 0x7F);

   // we are building a text string from the keystrokes / script input
   c = edit_string(c);     // attempt to add char to the string
   if(getting_string == SORT_CMD) {  // royal kuldge for G C G, G C Z, and G C T keyboard commands
      if(val == 'G') goto quick_cmd;  // g c g
      if(val == 'T') goto quick_cmd;  // g c t
      if(val == 'Z') goto quick_cmd;  // g c z 
   }

   if(getting_string < 0x0100) not_upper = !isupper(getting_string);
   else                        not_upper = 1;

   if(c == ESC_CHAR) {     // edit abandoned
      goto abandon_edit;
   }
// else if((c == 0x0D) && (edit_buffer[0] == 0) && !isupper(getting_string) && BLANK_LINES_OK) {
// else if((c == 0x0D) && (edit_buffer[0] == 0) && not_upper && BLANK_LINES_OK) {
   else if((c == 0x0D) && (edit_buffer[0] == 0) && blank_lines_ok(getting_string)) {
      // CR with no text in edit buffer
      abandon_edit:
      getting_string = 0;
      script_pause = 0;
      if(text_mode) redraw_screen();
      else if(rcvr_type == NO_RCVR) erase_plot(-1);
      else if((PLOT_WIDTH/TEXT_WIDTH) <= 80) erase_plot(-1);
   }
   else if(c == 0x0D) { // edit done, parameter available,  do the command
      quick_cmd:
      script_pause = 0;
      val = string_param();  // evaluate and act upon the text string
      getting_string = 0;
      if(val == 2) new_queue(0x03);
      else if(val == 1) redraw_screen();
      else if(rcvr_type == NO_RCVR) erase_plot(-1);
      else if((PLOT_WIDTH/TEXT_WIDTH) <= 80) erase_plot(-1);
   }
   else if(getting_string == 'a') {  // !!! kludge to make parsing command easier !!!
      goto quick_cmd;  
   }
   else if(getting_string == 'Q') {  // !!! kludge to make parsing command easier !!!
      goto quick_cmd;
   }
   else if(getting_string == '~') {  // !!! kludge to make parsing command easier !!!
      goto quick_cmd;  
   }
   else return 0;  // character was added to the edit string

   return sure_exit();
}


void edit_scale()
{
float val;
char *u;

   // get a plot scale factor
   if(plot[selected_plot].user_scale) val = 0.0F;
   else                               val = plot[selected_plot].scale_factor;

   if(selected_plot == TEMP) {
      if     (DEG_SCALE == 'F')  u = "milli degrees F";
      else if(DEG_SCALE == 'R')  u = "milli degrees R";
      else if(DEG_SCALE == 'K')  u = "milli degrees K";
      else if(DEG_SCALE == 'D')  u = "milli degrees D";
      else if(DEG_SCALE == 'N')  u = "milli degrees N";
      else if(DEG_SCALE == 'E')  u = "milli degrees E";
      else if(DEG_SCALE == 'O')  u = "milli degrees O";
      else                       u = "milli degrees C";
   }
   else u = plot[selected_plot].units;
   if(u[0] == ' ') ++u;

   sprintf(edit_buffer, "%.0f", val);

   sprintf(out, "Enter scale factor for %s graph (%s/div, 0=auto scale)", 
                 plot[selected_plot].plot_id, u);
   start_edit('d', out);
}

void edit_drift()
{
char *u;

   // set a plot drift rate removal factor

   if(selected_plot == TEMP) {
      if     (DEG_SCALE == 'F')  u = "degrees F";
      else if(DEG_SCALE == 'R')  u = "degrees R";
      else if(DEG_SCALE == 'K')  u = "degrees K";
      else if(DEG_SCALE == 'D')  u = "degrees D";
      else if(DEG_SCALE == 'N')  u = "degrees N";
      else if(DEG_SCALE == 'E')  u = "degrees E";
      else if(DEG_SCALE == 'O')  u = "degrees O";
      else                       u = "degrees C";
   }
   else u = plot[selected_plot].units;
   if(u[0] == ' ') ++u;
u = "units";

   sprintf(edit_buffer, "%g", plot[selected_plot].a1);

   sprintf(out, "Enter drift rate to remove from %s graph (%s/sec)", 
                 plot[selected_plot].plot_id, u);
   start_edit('=', out);
}

void edit_ref()
{
float val;
char *u;

   // get a plot center line reference value
   edit_buffer[0] = 0;
   val = plot[selected_plot].plot_center;
   if(plot[selected_plot].float_center) {
      sprintf(edit_buffer, "%.6f", val);
   }

   if(selected_plot == TEMP) {
      if     (DEG_SCALE == 'F')  u = "degrees F";
      else if(DEG_SCALE == 'R')  u = "degrees R";
      else if(DEG_SCALE == 'K')  u = "degrees K";
      else if(DEG_SCALE == 'D')  u = "degrees D";
      else if(DEG_SCALE == 'N')  u = "degrees N";
      else if(DEG_SCALE == 'E')  u = "degrees E";
      else if(DEG_SCALE == 'O')  u = "degrees O";
      else                       u = "degrees C";
   }
   else if(selected_plot == DAC) u = "V";
   else u = plot[selected_plot].units;
   if(u[0] == ' ') ++u;

   sprintf(out, "Enter center line reference for %s graph (in %s, <cr>=auto)", 
                 plot[selected_plot].plot_id, u);
   start_edit('C', out);
}


void edit_cal()
{
float m, b;
char *s;

   // get a plot center line reference value
   s = plot[selected_plot].plot_id;
   edit_buffer[0] = 0;

   if     (selected_plot == TEMP)  { m=temp1_m; b=temp1_b; }
   else if(selected_plot == TC2)   { m=temp2_m; b=temp2_b; }
   else if(selected_plot == BATTI) { m=batti_m; b=batti_b; }
   else if(selected_plot == LEDI)  { m=ledi_m;  b=ledi_b; }
   else if(selected_plot == BATTV) { m=vcal_m;  b=vcal_b; s="VCAL"; }
   else if(selected_plot == LUX1)  { m=lux1_m;  b=lux1_b; }
   else if(selected_plot == LUX2)  { m=lux2_m;  b=lux2_b; }
   else if(selected_plot == AUXV)  { m=adc2_m;  b=adc2_b; }
   else if(selected_plot == CCT)   { m=rb_m;    b=rb_b; }
   else {
      sprintf(out, "%s does not have a calibration adjustment!", s);
      edit_error(out);
      return;
   }

   if(cal_mode) {
//    sprintf(edit_buffer, "%f  %f", m,b);
      edit_buffer[0] = 0;
      sprintf(out, "Enter reading1 actual1 reading2 actual2 for %s (m=%f b=%f)", s, m,b);
   }
   else {
      sprintf(edit_buffer, "%f  %f", m,b);
      sprintf(out, "Enter m and b cal factors for %s (m=%f b=%f)", s, m,b);
   }
   start_edit(CAL_CMD, out);
}

void edit_cal_param()
{
float m, b;
float r1,a1, r2,a2;
int i;

   if(cal_mode) {
      r1 = 1.0F;
      a1 = 0.0F;
      r2 = 1.0F;
      a2 = 0.0F;
      i = sscanf(edit_buffer, "%f %f %f %f", &r1,&a1, &r2,&a2);
      if(i < 1) {
         edit_error("ERROR: No calibration numbers were given!");
         return;
      }
      if(i < 3) {
         r2 = r1;
         a2 = a1;
      }

      if(r2 == r1) {
         if(r1 == 0.0F) {
            edit_error("ERROR: reading1 and reading2 cannot both be zero!");
            return;
         }
         else {
            m = a1 / r1;
            b = 0.0F;
         }
      }
      else {
         m = (a2-a1) / (r2-r1);
         b = a2 - m*r2;
      }

      if(m == 0.0F) {
         edit_error("ERROR: calculated 'm' value cannot be 0.0!");
         return;
      }
      else {
         sprintf(plot_title, "%s cal: m=%.6f  b=%.6f", plot[selected_plot].plot_id, m,b);
      }
   }
   else {
      m = 1.0F;
      b = 0.0F;
      i = sscanf(edit_buffer, "%f %f", &m,&b);
      if(i < 1) {
         edit_error("ERROR: No calibration numbers were given!");
         return;
      }
   }

   if     (selected_plot == TEMP)  { temp1_m=m; temp1_b=b; }
   else if(selected_plot == TC2)   { temp2_m=m; temp2_b=b; }
   else if(selected_plot == BATTI) { batti_m=m; batti_b=b; }
   else if(selected_plot == BATTV) { vcal_m=m;  vcal_b=b; }
   else if(selected_plot == LEDI)  { ledi_m=m;  ledi_b=b; }
   else if(selected_plot == LUX1)  { lux1_m=m;  lux1_b=b; }
   else if(selected_plot == LUX2)  { lux2_m=m;  lux2_b=b; }
   else if(selected_plot == AUXV)  { adc2_m=m;  adc2_b=b; }
   else if(selected_plot == CCT)   { rb_m=m;    rb_b=b; }
   else return;
   set_luxor_cal();
}


void view_all()
{
float val;

   val = (float) plot_q_count;
   view_interval = 0;
   while(val > 0) {
      val -= PLOT_WIDTH;
      ++view_interval;
   }
// view_interval = (long) (((val+PLOT_WIDTH-1) / PLOT_WIDTH) + 0.5F);
   if(view_interval <= 1L) view_interval = 1L;
   user_view = view_interval;
   new_user_view = 1;
   do_review(END_CHAR);
}


int edit_user_view(char *s)
{
float val;

   // process the new plot view time setting
   val = 0.0F;
   strcpy(out, s);
   strupr(out);
   if(strstr(out, "A")) {   // view ALL data
      view_all_data = 1;
   }
   else if(strstr(out, "T")) {   // view ALL data, auto scroll
      view_all_data = 2;
   }
   else {                   // view a subset of the data
      view_all_data = 0;
      val = (float) atof(out);
      if(val < 0.0F) {
         val = 0.0F - val;
      }
      else if(val == 0.0F) {
         user_view = 0L;
         view_interval = 1L;
         return 0;
      }
   }
   val *= nav_rate;

   if(queue_interval == 0.0) ;
   else if(strstr(out, "W")) {
      view_interval = (long) ((((val * (24.0F*60.0F*60.0F)*7.0F) / (float) queue_interval) / PLOT_WIDTH) + 0.5F);
   }
   else if(strstr(out, "D")) {
      view_interval = (long) ((((val * (24.0F*60.0F*60.0F)) / (float) queue_interval) / PLOT_WIDTH) + 0.5F);
   }
   else if(strstr(out, "H")) {
      view_interval = (long) ((((val * 3600.0F) / (float) queue_interval) / PLOT_WIDTH) + 0.5F);
   }
   else if(strstr(out, "M")) {
      view_interval = (long) ((((val * 60.0F) / (float) queue_interval) / PLOT_WIDTH) + 0.5F);
   }
   else if(strstr(out, "S")) {
      view_interval = (long) (((val / (float) queue_interval) / PLOT_WIDTH) + 0.5F);
   }
   else if(strstr(out, "T")) {
      view_all();
      return 0;
   }
   else if(strstr(out, "A")) {
      view_all();
      return 0;
   }
   else if(queue_interval) {  // interval set in minutes per division
      val /= (float) queue_interval;
      val *= 60.0F;                   // seconds/division
      val /= (float) HORIZ_MAJOR;
      view_interval = (long) val;
   }

   if(view_interval <= 1L) view_interval = 1L;
   user_view = view_interval;
   new_user_view = 1;

   return 0;
}


void edit_dt(char *arg,  int err_ok)
{
int aa,bb,cc;
u08 s1,s2;
int dd,ee,ff;
u08 s3,s4;
u08 sx;
long val;
u08 timer_set;

   // parse the alarm/exit/dump date and time string
   if(arg[0] == 0) {  // no string given,  clear the timer values
      alarm_wait = 0;
      if(getting_string == ':') {      // alarm/egg timer
         alarm_time = alarm_date = 0;
         egg_timer = egg_val = 0;
         repeat_egg = 0;
      }
      else if(getting_string == '/') { // exit time/timer
         end_time = end_date = 0;
         exit_timer = exit_val = 0;
         repeat_exit = 0;
      }
      else if(getting_string == '!') { // screen dump timer
         dump_time = dump_date = 0;
         dump_timer = dump_val = 0;
         repeat_dump = 0;
      }
      else if(getting_string == '(') { // log dump timer
         log_time = log_date = 0;
         log_timer = log_val = 0;
         repeat_log = 0;
      }
      return;
   }

   strupr(arg);
   if(getting_string == '!') {
      if(strstr(arg, "O")) single_dump = 1;  // dump the screen to one file
      else                 single_dump = 0;  // dump the screen multiple files
   }
   else if(getting_string == '/') { // exit time/timer
      if(strstr(arg, "O")) single_exit = 1;  // exit program once
      else                 single_exit = 0;  // exit program peridoically
   }
   else if(getting_string == '(') {
      if(strstr(arg, "O")) single_log = 1;   // dump the log to one file
      else                 single_log = 0;   // dump the log multiple files
   }
   else {
      if(strstr(arg, "O")) single_alarm = 1; // sound the alarm tone once
      else                 single_alarm = 0; // sound the alarm until a key press
   }

   // see if a countdown timer has been set
   timer_set = 0;
   val = (long) atof(arg);
   if(strstr(arg, "S")) {      // timer is in seconds
      timer_set = 'S';
   }
   else if(strstr(arg, "M")) { // timer is in minutes 
      val *= 60L;
      timer_set = 'M';
   }
   else if(strstr(arg, "H")) { // timer is in hours 
      val *= 3600L;
      timer_set = 'H';
   }
   else if(strstr(arg, "D")) { // timer is in days 
      val *= 3600L*24L;
      timer_set = 'D';
   }

   if(strstr(arg, "W")) { // wait for alarm to trigger in script file
      alarm_wait = 1;
   }
   else {
      alarm_wait = 0;
   }


   if(timer_set) {  // a countdown timer has been set
      if(getting_string == ':') { // it is the egg timer
         egg_val = egg_timer = val;
         if(strstr(arg, "R")) repeat_egg = 1;
         else                 repeat_egg = 0;
      }
      else if(getting_string == '/') {  // it is the exit timer
         exit_val = exit_timer = val;
         if(strstr(arg, "R")) repeat_exit = 1;
         else                 repeat_exit = 0;
      }
      else if(getting_string == '!') {  // it is the screen dump timer
         dump_val = dump_timer = val;
         if(strstr(arg, "R")) repeat_dump = 1;
         else                 repeat_dump = 0;
      }
      else if(getting_string == '(') {  // it is the log dump timer
         log_val = log_timer = val;
         if(strstr(arg, "R")) repeat_log = 1;
         else                 repeat_log = 0;
      }
      return;
   }

   // setting a time or date value
   aa = bb = cc = dd = ee = ff = 0;
   s1 = s2 = s3 = s4 = 0;
   if(err_ok) {  // processing keyboard time string
      if(getting_string == '!') {
         dump_time = dump_date = 0;
         dump_hh = dump_mm = dump_ss = 0;
         dump_month = dump_day = dump_year = 0;
      }
      else if(getting_string == '(') {
         log_time = log_date = 0;
         log_hh = log_mm = log_ss = 0;
         log_month = log_day = log_year = 0;
      }
      else {
         alarm_time = alarm_date = 0;
         alarm_hh = alarm_mm = alarm_ss = 0;
         alarm_month = alarm_day = alarm_year = 0;

         end_time = end_date = 0;
         end_hh = end_mm = end_ss = 0;
         end_month = end_day = end_year = 0;
      }
   }
   sscanf(arg, "%d%c%d%c%d%c%d%c%d%c%d", &aa,&s1,&bb,&s2,&cc, &sx, &dd,&s3,&ee,&s4,&ff);

   if(s1 == ':') { // first field is a time
      if(status_second(cc)) {     // seconds field woukd occur within a long status message
         cc = status_second(cc);
      }

      if(s2 && (s2 != s1)) {
        if(err_ok) edit_error("Invalid time separator character");
      }
      else if(s3 && (s3 != '/')) {
        if(err_ok) edit_error("Invalid date separator character");
      }
      else if(getting_string == ':') {  // alarm time
         alarm_time = 1;
         alarm_hh = aa;
         alarm_mm = bb;
         alarm_ss = cc;
         if(s3 == '/') {  // second field is a date
            alarm_date = 1;
            if(dd >= 2000) {
               alarm_day = ff;
               alarm_month = ee;
               alarm_year = dd;
            }
            else {
               alarm_month = dd;
               alarm_day = ee;
               if(ff < 100) ff += 2000;
               alarm_year = ff;
            }
         }
      }
      else if(getting_string == '!') {  // screen dump time
         dump_time = 1;
         dump_hh = aa;
         dump_mm = bb;
         dump_ss = cc;
         if(s3 == '/') {  // second field is a date
            dump_date = 1;
            if(dd >= 2000) {
               dump_day = ff;
               dump_month = ee;
               dump_year = dd;
            }
            else {
               dump_month = dd;
               dump_day = ee;
               if(ff < 100) ff += 2000;
               dump_year = ff;
            }
         }
      }
      else if(getting_string == '(') {  // log dump time
         log_time = 1;
         log_hh = aa;
         log_mm = bb;
         log_ss = cc;
         if(s3 == '/') {  // second field is a date
            if(dd >= 2000) {
               log_day = ff;
               log_month = ee;
               log_year = dd;
            }
            else {
               log_date = 1;
               log_month = dd;
               log_day = ee;
               if(ff < 100) ff += 2000;
               log_year = ff;
            }
         }
      }
      else { // exit time
         end_time = 1;
         end_hh = aa;
         end_mm = bb;
         end_ss = cc;
         if(s3 == '/') {  // second field is a date
            end_date = 1;
            if(dd >= 2000) {
               end_day = ff;
               end_month = ee;
               end_year = dd;
            }
            else {
               end_month = dd;
               end_day = ee;
               if(ff < 100) ff += 2000;
               end_year = ff;
            }
         }
      }
   }
   else if(s1 == '/') {  // first field is a date
      if(status_second(ff)) {     // seconds field woukd occur within a long status message
         ff = status_second(ff);
      }

      if(s2 && (s2 != s1))       edit_error("Invalid date separator character");
      else if(s3 && (s3 != ':')) edit_error("Invalid time separator character");
      else if(getting_string == ':') {  // alarm date
         alarm_date = 1;
         if(aa >= 2000) {
            alarm_day = cc;
            alarm_month = bb;
            alarm_year = aa;
         }
         else {
            alarm_month = aa;
            alarm_day = bb;
            if(cc < 100) cc += 2000;
            alarm_year = cc;
         }

         if(s3 == ':') {  // second field is a time
            alarm_time = 1;
            alarm_hh = dd;
            alarm_mm = ee;
            alarm_ss = ff;
         }
      }
      else if(getting_string == '!') {  // screen dump date
         dump_date = 1;
         if(aa >= 2000) {
            dump_day = cc;
            dump_month = bb;
            dump_year = aa;
         }
         else {
            dump_month = aa;
            dump_day = bb;
            if(cc < 100) cc += 2000;
            dump_year = cc;
         }

         if(s3 == ':') {  // second field is a time
            dump_time = 1;
            dump_hh = dd;
            dump_mm = ee;
            dump_ss = ff;
         }
      }
      else if(getting_string == '(') {  // log dump date
         log_date = 1;
         if(aa >= 2000) {
            log_day = cc;
            log_month = bb;
            log_year = aa;
         }
         else {
            log_month = aa;
            log_day = bb;
            if(cc < 100) cc += 2000;
            log_year = cc;
         }

         if(s3 == ':') {  // second field is a time
            log_time = 1;
            log_hh = dd;
            log_mm = ee;
            log_ss = ff;
         }
      }
      else { // exit date
         end_date = 1;
         if(aa >= 2000) {
            end_day = cc;
            end_month = bb;
            end_year = aa;
         }
         else {
            end_month = aa;
            end_day = bb;
            if(cc < 100) cc += 2000;
            end_year = cc;
         }

         if(s3 == ':') {  // second field is a time
            end_time = 1;
            end_hh = dd;
            end_mm = ee;
            end_ss = ff;
         }
      }
   }
   else {
      if(err_ok) edit_error("No time or date specifier character seen");
   }
}


double parse_alt(char *s)
{
double val;

   // convert an altitude string to meters. The string can end in an f or '
   // to indicate feet.

   if(s == 0) return 0.0;

   strlwr(s);
   val = atof(s);

   if(strchr(s, 'f') || strchr(s, '\'')) {  // convert feet to meters
      val /= FEET_PER_METER;
   }
   return val;
}

double parse_coord(char *s)
{
double val;
char c1;
char c2;
char c3;
double degs, mins, secs;
double v1,v2,v3;

   // convert a lat/lon string to decimal degrees.  The string can either be
   // a decimal number or a string in the format 11d12m30s (no embeded spaces).
   // If a part of the dms format string is not given, it is assumed to be 0.

   if(s == 0) return 0.0;

   strlwr(s);

   if(strchr(s, 'd') || strchr(s, 'm') || strchr(s, 's')) {  // convert feet to meters
      c1 = c2 = c3 = 0;
      degs = mins = secs = 0.0;
      v1 = v2 = v3 = 0.0;
      sscanf(s, "%lf%c%lf%c%lf%c", &v1,&c1, &v2,&c2, &v3,&c3);

      if     (c1 == 'd') degs = v1;
      else if(c2 == 'd') degs = v2;
      else if(c3 == 'd') degs = v2;
      else               degs = 0.0;

      if     (c1 == 'm') mins = v1;
      else if(c2 == 'm') mins = v2;
      else if(c3 == 'm') mins = v3;
      else               mins = 0.0;

      if     (c1 == 's') secs = v1;
      else if(c2 == 's') secs = v2;
      else if(c3 == 's') secs = v3;
      else               secs = 0.0;

      val = degs + (mins/60.0) + (secs/3600.0);
   }
   else {
      val = atof(s);
   }

   return val;
}

int parse_lla(char *s)
{
double old_lat, old_lon, old_alt;
char lat_s[SLEN+1];
char lon_s[SLEN+1];
char alt_s[SLEN+1];
int n;
char *comma;

   // set lat/lon/altitude
   if(s == 0) return (-1);

   old_lat = precise_lat;
   old_lon = precise_lon;
   old_alt = precise_alt;

   comma = s;
   while(comma) {  // convert commas to spaces
      comma = strchr(comma, ',');
      if(comma) *comma = ' ';
      else break;
   }

   precise_lat = precise_lon = precise_alt = 1.0E6;
   n = sscanf(s, "%s %s %s", &lat_s[0],&lon_s[0],&alt_s[0]);
   if(n != 3) {
      precise_lat = old_lat;
      precise_lon = old_lon;
      precise_alt = old_alt;
      return 4;
   }

   precise_lat = parse_coord(lat_s);
   precise_lon = parse_coord(lon_s);
   precise_alt = parse_alt(alt_s);
//sprintf(plot_title, "lat:%.9f lon:%.9f alt:%.9f", precise_lat,precise_lon,precise_alt);

   if((precise_lat < -90.0) || (precise_lat > 90.0)) {
      precise_lat = old_lat;
      precise_lon = old_lon;
      precise_alt = old_alt;
      return 1;
   }
   else if((precise_lon < -180.0) || (precise_lon > 180.0)) {
      precise_lat = old_lat;
      precise_lon = old_lon;
      precise_alt = old_alt;
      return 2;
   }
   else if((precise_alt< -1000.0) || (precise_alt > 20000.0)) {
      precise_lat = old_lat;
      precise_lon = old_lon;
      precise_alt = old_alt;
      return 3;
   }
   precise_lat /= RAD_TO_DEG;
   precise_lon /= RAD_TO_DEG;

   return 0;
}

void edit_lla_cmd()
{
int err;

   err = parse_lla(&edit_buffer[0]);
   if     (err == 1) edit_error("Bad latitude value");
   else if(err == 2) edit_error("Bad longitude value");
   else if(err == 3) edit_error("Bad altitude value");
   else if(err == 4) edit_error("Must enter lat lon alt values (separated by spaces)");
   else if(err) return;
   else { // save current position with high accuracy
      // !!!!! should we set ref_lat/lon/alt? gggggggg
      if(luxor) {
         set_lla(precise_lat, precise_lon, precise_alt);
      }
      else {
         #ifdef PRECISE_STUFF
            stop_precision_survey();   // stop any precision survey
            stop_self_survey();        // stop any self survey
            save_precise_posn(0);      // do single point surveys until we get close enough
         #else
            set_lla(precise_lat, precise_lon, precise_alt);
         #endif
      }
   }
}


void edit_pps_freq(int chan)
{
double freq;
double duty;

   // set lat/lon/altitude
   if(edit_buffer[0] == 0) return;

   freq = 1.0;
   duty = 0.50;
   if(rcvr_type == NVS_RCVR) sscanf(edit_buffer, "%lf", &duty);
   else sscanf(edit_buffer, "%lf %lf", &freq, &duty);

   if((chan == 1) && ((rcvr_type == NVS_RCVR) || ((rcvr_type == VENUS_RCVR) && !saw_timing_msg)) ) {
      edit_error("Receiver does not support PPS2");
      return;
   }
   else if(freq < 0.0) {
      edit_error("Frequency must be >= 0.0");
      return;
   }
   else if(duty <= 0.0) {
      edit_error("Duty cycle / pulse width must be > 0.0");
      return;
   }
   else if(duty >= 1.0) {  // duty is a pulse width in microseconds
      if(rcvr_type == NVS_RCVR) duty = freq * (duty/1.0E9);  // convert nanoeconds to duty cycle
      else duty = freq * (duty/1.0E6);  // convert microseconds to duty cycle
      if(duty >= 1.0) {
         edit_error("Pulse width too long for specified frequency"); 
         return;
      }
   }

   if(chan == 0) set_pps_freq(chan, freq, duty, pps_polarity, cable_delay, pps1_delay, 1);
   else          set_pps_freq(chan, freq, duty, 0,            cable_delay, pps2_delay, 1); // rising edge gggg
}

void edit_option_switch()
{
int cmd_err;
char *s;
int i;
char cmd[256+1];

   // process what is normally command line only options from the keyboard
   not_safe = 0;
   keyboard_cmd = 1;
   s = &edit_buffer[0];

   while(*s) {
      while(*s) {  // skip leading white space
         if(*s == 0) break;
         else if(*s == ' ') ++s;
         else if(*s == '\t') ++s;
         else break;
      }
      if(*s == 0) break;

      i = 0;
      cmd[i] = 0;
      while(*s) {  // extract the next option string
         if(*s == ' ') break;
         else if(*s == '\t') break;
         else if(i >= 256) break;

         cmd[i++] = *s;
         cmd[i] = 0;
         ++s;
      }

      if(1) ;
      else if(cmd[0] == '-');
      else if(cmd[0] == '/');
      else if(cmd[0]) {
         strcpy(out, "/");
         strcat(out, cmd);
         strcpy(cmd, out);
      }

      cmd_err = option_switch(&cmd[0]);  // process the option

      if(cmd_err) {
         sprintf(out, "Unknown command line option %c.  Type ? for command line help.", cmd_err);
         edit_error(out);
         break;
      }
      else if(not_safe == 1) {
         config_options();
      }
      else if(not_safe == 2) {
         sprintf(out, "Option cannot be set from keyboard: %s", edit_buffer);
         edit_error(out);
         break;
      }
   }

   need_redraw = 1200;
}

void set_osc_units()
{
if(rcvr_type != TSIP_RCVR) return;

   if(show_euro_ppt) {
      ppb_string = "e-9 "; 
      ppt_string = "e-12";
   }
   else {
      ppb_string = " ppb"; 
      ppt_string = " ppt";
   }
   plot[OSC].units = ppt_string;
}


void edit_option_value()
{
float val;
u08 c;

   // set a debug/test value
   val = (float) atof(&edit_buffer[1]);
   c = tolower(edit_buffer[0]);
   if(c == 'a') { 
      amu_flag ^= 0x08;
      set_io_options(0x13, 0x03, 0x01, amu_flag);  // ECEF+LLA+DBL PRECISION, ECEF+ENU vel,  UTC, no PACKET 5A, amu
//    clear_signals();
   }
#ifdef ADEV_STUFF
   else if(c == 'b') {  // adev bin scale
      bin_scale = (int) val;
      if(bin_scale <= 0) bin_scale = 5;
      reload_adev_info();
      force_adev_redraw();
   }
#endif
   else if(c == 'c') {  // sat count size
      if(edit_buffer[1]) small_sat_count = (int) val & 0x01;
      else               small_sat_count ^= 1;
      config_screen(100);
   }
#ifdef FFT_STUFF
   else if(c == 'd') {  // do FFT in dB
      if(edit_buffer[1]) fft_db = (int) val & 0x01;
      else               fft_db ^= 1;
      if(fft_db) plot[FFT].units = "dB";
      else       plot[FFT].units = "x ";
      set_fft_scale();
   }
#endif
   else if(c == 'e') {  // force Thunderbolt-E flag
      if(edit_buffer[1]) ebolt = (int) val & 0x01;
      else               ebolt ^= 1;
      max_sats = 8;     // used to format the sat_info data
      temp_sats = 8;
      eofs = 1;
      if(ebolt) {
         last_ebolt = (-2);
         saw_ebolt();
      }
      config_screen(101);
   }
#ifdef ADEV_STUFF
   else if(c == 'f') {  // keep adevs fresh (by periodically forcing a recalc)
      if(edit_buffer[1]) keep_adevs_fresh = (int) val & 0x01;
      else               keep_adevs_fresh = 1;
   }
#endif
   else if(c == 'g') {  // plot grid background color
      if(edit_buffer[1]) plot_background = (int) val & 0x01;
      else               plot_background ^= 1;
   }
   else if(c == 'h') {  // erase lla plot every hour
      if(edit_buffer[1]) erase_every_hour = (int) val & 0x01;
      else               erase_every_hour ^= 1;
   }
   else if(c == 'i') {  // signal level displays
      if(edit_buffer[1])    plot_signals = (int) val;
      else if(plot_signals) plot_signals = 0;
      else                  plot_signals = 4;

      if(zoom_screen) {
         zoom_screen = 0;
         change_zoom_config(0);
      }
      zoom_fixes = show_fixes;
      if     (strchr(edit_buffer, 'z')) {
         change_zoom_config(10);
         zoom_screen = 'S';
      }
      else if(strchr(edit_buffer, 'Z')) {
         change_zoom_config(11);
         zoom_screen = 'S';
      }
      update_azel = 1;
      config_screen(102);
   }
   else if(c == 'j') {  // just read and log serial data
      if(edit_buffer[1]) log_stream = (int) val & 0x01;
      else               log_stream ^= 1;
//    if(edit_buffer[1]) just_read = (int) val & 0x01;
//    else               just_read ^= 1;
//    log_stream = just_read;
   }
   else if(c == 'k') {  // flag message faults as time skips
      if(edit_buffer[1]) flag_faults = (int) val & 0x01;
      else               flag_faults ^= 1;
   }
#ifdef FFT_STUFF
   else if(c == 'l') {  // live fft
      if(edit_buffer[1]) show_live_fft = (int) val & 0x01;
      else               show_live_fft ^= 1;
      live_fft = selected_plot;
      if(show_live_fft && (plot[FFT].show_plot == 0)) toggle_plot(FFT);
   }
#endif
   else if(c == 'm') {  // plot magnification factor (for less than one sec/pixel)
      plot_mag = (int) val;
      if(plot_mag <= 1) plot_mag = 1;
   }
   else if(c == 'n') {  // dynamic trend line info display
      if(edit_buffer[1]) dynamic_trend_info = (int) val & 0x01;
      else               dynamic_trend_info ^= 1;
   }
   else if(c == 'p') {  // peak scale - dont let plot scale factors get smaller
      if(edit_buffer[1]) peak_scale = (int) val & 0x01;
      else               peak_scale ^= 1;
   }
   else if(c == 'q') {  // skip over subsmapled queue entries using the slow, direct method
      if(edit_buffer[1]) slow_q_skips = (int) val & 0x01;
      else               slow_q_skips ^= 1;
   }
#ifdef ADEV_STUFF
   else if(c == 'r') {  // reset adev bins and recalc adevs
      reload_adev_info();
   }
#endif
   else if(c == 's') {  // temperature spike filter 0=off  1=for temp pid  2=for pid and plot
      if(edit_buffer[1])  spike_mode = (int) val;
      else if(spike_mode) spike_mode = 0;
      else                spike_mode = 1;
   }
   else if(c == 't') {  // alarm/dump type based upon local or displayed time
      if(edit_buffer[1]) alarm_type = (int) val & 0x01;
      else               alarm_type ^= 1;
   }
   else if(c == 'u') {  // daylight savings time area
      if(edit_buffer[1]) dst_area = (int) val;
      else               dst_area = 0;
      if((dst_area < 0) || (dst_area > DST_AREAS)) dst_area = 1;
      calc_dst_times(dst_list[dst_area]);
      dst_ofs = dst_offset();
   }
   else if(c == 'v') {  // ADEV base value mode 
      // (kludge used to increase dynamic range of single precision numbers)
      subtract_base_value = (u08) val;
      reset_queues(3);
   }
   else if(c == 'w') {  // watch face style
      watch_face = (u08) val;
      if(watch_face > 5) watch_face = 0;
   }
   else if(c == 'x') {
      if(edit_buffer[1]) set_rcvr_mode((int) val);
      else               set_rcvr_mode(RCVR_MODE_HOLD);
   }
   else if(c == 'y') {  // temp-dac dac scale factor
      if(edit_buffer[1]) d_scale = val;
      else if(plot[DAC].sum_yy && stat_count) {
         d_scale = sqrt(plot[TEMP].sum_yy/stat_count) / sqrt(plot[DAC].sum_yy/stat_count);
      }
      else d_scale = 0.0;
if(title_type != USER) {
   sprintf(plot_title, "DAC_scale=%f", d_scale);
   title_type = OTHER;
}
   }
   else if(c == 'z') {  // auto centering
      if(edit_buffer[1]) auto_center = (int) val & 0x01;
      else               auto_center ^= 1;
   }
   else if((c == '/') || (c == '-') || (c == '$') || (c == '=') || isdigit(c)) {
      edit_option_switch();
   }
   else {
      // bugs_black_blood:
      sprintf(out, "Unknown debug parameter: %c", c);
      edit_error(out);
   }

   redraw_screen();
}


void dump_cal_file()
{
FILE *file;

   if(edit_buffer[0] == 0) {
      file = topen("luxcal.scr", "w"); 
      if(file == 0) {
         sprintf(out, "ERROR: could not write file: luxcal.scr");
         edit_error(out);
         return;
      }
   }
   else {
//    strupr(edit_buffer);
      if(strstr(edit_buffer, ".") == 0) strcat(edit_buffer, ".scr");
      file = topen(edit_buffer, "w"); 
      if(file == 0) {
         sprintf(out, "ERROR: could not write file: %s", edit_buffer);
         edit_error(out);
         return;
      }
   }
   if(file == 0) return;

   fprintf(file, "&c0                     ;exit calibration mode\n");
   fprintf(file, "&s  %.4f   SERIAL     ;serial number\n", vref_b);
   fprintf(file, "&v  %.6f VREF       ;ADC reference voltage\n", vref_m);
   fprintf(file, "gd& %.6f %.6f   ;battery/led voltage calibration\n", vcal_m, vcal_b);
   fprintf(file, "go& %.6f %.6f   ;battery current calibration\n", batti_m, batti_b);
   fprintf(file, "g3& %.6f %.6f   ;led current calibration\n", ledi_m, ledi_b);
   fprintf(file, "gt& %.6f %.6f   ;temp sensor 1 calibration\n", temp1_m, temp1_b);
   fprintf(file, "g5& %.6f %.6f   ;temp sensor 2 calibration\n", temp2_m, temp2_b);
   fprintf(file, "gp& %.6f %.6f   ;lux sensor calibration\n", lux1_m, lux1_b);
   fprintf(file, "g1& %.6f %.6f   ;lumen sensor calibration\n", lux2_m, lux2_b);
   fprintf(file, "gl& %.6f %.6f   ;r/b color temperature calibration\n", rb_m, rb_b);
   fprintf(file, "g0& %.6f %.6f   ;aux ADC channel calibration\n", adc2_m, adc2_b);

   fclose(file);
}

int edit_write_cmd()
{
   // process a file write/delete command
   if(edit_buffer[0]) {
      if(dump_type == 's') {
         first_key = 0;
         prot_menu = 0;
         draw_plot(1);
         draw_maps();
         refresh_page();
         if(dump_screen(invert_dump, top_line, edit_buffer) == 0) {
            first_key = ' ';
            edit_error("Could not write screen image file");
         }
      }
      else if(dump_type == 'd') {   //delete file
         if(path_unlink(edit_buffer)) {
            sprintf(out, "Could not delete file: %s", edit_buffer);
         }
         else {
            sprintf(out, "File %s deleted", edit_buffer);
         }
         edit_error(out);
      }
      else if(dump_type == 'c') {  // luxor cal .SCR file
         dump_cal_file();
      }
      else {
         dump_log(edit_buffer, dump_type);
      }
   }
   else {
      edit_error("Bad file name");
   }

   top_line = 0;
   return 0;
}

void edit_osc_param()
{
float val;

   // change an oscillator control parameter
   if(edit_buffer[0] == 0) return;
   val = (float) atof(edit_buffer);

   if(getting_string == '1') {  // time constant
      user_time_constant = val;
   }
   else if(getting_string == '2') {  // damping factor
      user_damping_factor = val;
   }
   else if(getting_string == '3') {  // osc gain
      user_osc_gain = val;
      if(process_com == 0) osc_gain = val;
if(luxor) set_luxor_sens((u08) val, (u08) val);
   }
   else if(getting_string == '4') {  // min volts / battery pwm level
      user_min_volts = val;
if(luxor) set_batt_pwm((u16) (val*65535.0F));
   }
   else if(getting_string == '5') {  // max volts / luxor driver mode change
      user_max_volts = val;
   }
   else if(getting_string == '6') {  // jam sync theshold
      user_jam_sync = val;
   }
   else if(getting_string == '7') {  // max freq offset
      user_max_freq_offset = val;
   }
   else if(getting_string == '8') {  // initial dac voltage
      user_initial_voltage = val;
      if(saw_ntpx) initial_voltage = val;
   }
   else if(getting_string == 't') {  // min EFC range volts
      user_min_range = val;
   }
   else if(getting_string == 'T') {  // max EFC range volts
      user_max_range = val;
   }
   else if(getting_string == PULLIN_CMD) {  // max UCCM pullin range
      user_pullin = (int) val;
   }
   else return;   // unknown osc param

   set_discipline_params(1);
   request_all_dis_params();
}

float get_cable_val(char *s)
{
float val;

   if(s == 0) return 0.0F;

   while(s > &edit_buffer[0]) { // backup over leading white space
      --s;
      if((*s != ' ') && (*s != '\t')) break;
   }
   while(s > &edit_buffer[0]) { // backup to the value
      --s;
      if((*s == ' ') || (*s == '\t')) break;
   }
   if(s == 0) return 0.0F;

   sscanf(s, "%f", &val);
   return val;
}


void edit_cable_delay(int kbd_cmd)
{
float val;
float vf;
char *s, *sf, *squote;

   if(edit_buffer[0] == 0) return;

   vf = 0.66F;  // 0.66 vp coax
   strupr(edit_buffer);


   s = strchr(edit_buffer, 'V');  // look for velocity factor setting
   if(s) {
      vf = get_cable_val(s);
      if((vf <= 0.0) || (vf > 1.0)) {
         if(kbd_cmd) edit_error("Invalid velocity factor specified.");
         return;
      }
   }

   s = strchr(edit_buffer, 'M');  // look for meters
   sf = strchr(edit_buffer, 'F');
   squote = strchr(edit_buffer, '\'');
   if(s) {
      val = get_cable_val(s);
      val *= (float) FEET_PER_METER;
      val = (val / 983571056.0F) / vf;
   }
   else if(sf) {
      val = get_cable_val(sf);
      val = (val / 983571056.0F) / vf;
   }
   else if(squote) {
      val = get_cable_val(squote);
      val = (val / 983571056.0F) / vf;
   }
   else {
      val = (float) atof(edit_buffer);
      val /= 1.0E9;
   }

   delay_value = val;

   if(kbd_cmd) {
      set_pps(user_pps_enable, pps_polarity,  delay_value,  pps1_delay, 300.0, 1);
      request_pps_info();
   }
}


#ifdef TEMP_CONTROL

void clear_pid_display()
{
   if(pid_debug == 0) {
      debug_text[0] = 0;
      debug_text2[0] = 0;
      redraw_screen();
   }
}

u08 edit_temp_pid_value(char c, char *s, int kbd_cmd)
{
   if(s == 0) return 'k';

   c = toupper(c);

   if(c == 'A') {    // KA command - bang-bang autotune
      // ka0 = abort autotune
      // ka1 = do full tuneup with all settling delays
      // ka2 = don't wait for temp to get near setpoint
      // ka3 = don't wait for pid to stabilize
      if(bang_bang == 0) OLD_P_GAIN = P_GAIN;
      bang_bang = 1;
      sscanf(s, "%d", &bang_bang);
      if(kbd_cmd) enable_temp_control();
      do_temp_control = 1;

      if(bang_bang > 4) bang_bang = 1;
      else if(bang_bang == 0) {       // abort autotune
         P_GAIN = OLD_P_GAIN;
         calc_k_factors();
         if(kbd_cmd) clear_pid_display();
         if(kbd_cmd) show_pid_values();
      }
   }
   else if(c == 'D') {  // KD command - derivitive time constant
      sscanf(s, "%f", &D_TC);
      calc_k_factors();
   }
   else if(c == 'F') {  // KF command - filter time constant
      sscanf(s, "%f", &FILTER_TC);
      calc_k_factors();
   }
   else if(c == 'G') {  // KG command - scale factor (loop gain correction)
      sscanf(s, "%f", &k7);
      calc_k_factors();
   }
   else if(c == 'H') {  // KH command - PID debug (help) display
      pid_debug = toggle_value(pid_debug);
      if(pid_debug) osc_pid_debug = 0;
      if(kbd_cmd) clear_pid_display();
   }
   else if(c == 'I') {  // KI command - integral time constant
      sscanf(s, "%f", &I_TC);
      calc_k_factors();
   }
   else if(c == 'K') {  // KK command - all major PID values
      if(kbd_cmd) enable_temp_control();
      do_temp_control = 1;
      FILTER_OFFSET = 0.0F;
      sscanf(s, "%f%c%f%c%f%c%f", &P_GAIN,&c, &D_TC,&c,  &FILTER_TC,&c, &I_TC);
      calc_k_factors();
   }
   else if(c == 'L') {  // KL command - load distubance
      sscanf(s, "%f", &k6);
      calc_k_factors();
   }
   else if(c == 'N') {  // KN command - mark's medium values
      if(kbd_cmd) enable_temp_control();
      do_temp_control = 1;
      set_default_pid(1);
      if(0 && kbd_cmd) show_pid_values();
   }
   else if(c == 'O') {  // KO command - filter offset
      sscanf(s, "%f", &FILTER_OFFSET);
      calc_k_factors();
   }
   else if(c == 'P') {  // KP command - pid gain
      sscanf(s, "%f", &P_GAIN);
      calc_k_factors();
   }
   else if(c == 'R') {  // KR command - integrator reset
      sscanf(s, "%f", &k8);
      calc_k_factors();
   }
   else if(c == 'S') {  // KS command - scale factor (loop gain correction)
      sscanf(s, "%f", &k7);
      calc_k_factors();
   }
   else if(c == 'T') {  // KT command - pwm test
      sscanf(s, "%f%c%f", &test_cool, &c, &test_heat);
      if(test_heat || test_cool) {
         if(kbd_cmd) enable_temp_control();
         do_temp_control = 1;
      }
      if(++test_marker > 9) test_marker = 1;
      mark_q_entry[test_marker] = plot_q_in;
   }
   else if(c == 'W') {  // KW command - Warren's slow values
      if(kbd_cmd) enable_temp_control();
      do_temp_control = 1;
      set_default_pid(0);
      if(0 && kbd_cmd) show_pid_values();
   }
   else if(c == 'X') {  // KX command - fast PID values
      if(kbd_cmd) enable_temp_control();
      do_temp_control = 1;
      set_default_pid(2);
      if(0 && kbd_cmd) show_pid_values();
   }
   else if(c == 'Y') {  // KY command - very fast PID values
      if(kbd_cmd) enable_temp_control();
      do_temp_control = 1;
      set_default_pid(3);
      if(0 && kbd_cmd) show_pid_values();
   }
   else if(c == 'Z') {  // KZ command - reset PID
      integrator = 0.0F;
      if(kbd_cmd) enable_temp_control(); 
      do_temp_control = 1;
   }
   else if(c == '9') {  // K9 command - autotune step size
      sscanf(s, "%f", &KL_TUNE_STEP);
      calc_k_factors();
   }
   else if(c == '0') {  // K0 command - make temperature crude
      crude_temp = toggle_value(crude_temp);
   }
   else return 1;

   return 0;
}

#endif // TEMP_CONTROL


#ifdef OSC_CONTROL

void clear_osc_pid_display()
{
   if(osc_pid_debug == 0) {
      debug_text[0] = 0;
      debug_text2[0] = 0;
      redraw_screen();
   }
}

u08 edit_osc_pid_value(char c, char *s, int kbd_cmd)
{
   if(s == 0) return 'b';
   c = toupper(c);

   if((c == 'A') || (c == 'B')) {    // BA command - bang-bang autotune
   }
   else if(c == 'C') { // BC command - failed cable delay trick - does not work
      sscanf(s, "%lf", &trick_scale);
      if(trick_scale == 0.0) {
         set_pps(user_pps_enable, pps_polarity,  delay_value, pps1_delay, 300.0, 0);
         request_pps_info();
      }
      else {
         first_trick = 1;
         trick_value = 0.0;
      }
   }
   else if(c == 'D') {  // BD command - derivitive time constant
      sscanf(s, "%lf", &OSC_D_TC);
      calc_osc_k_factors();
   }
   else if(c == 'F') {  // BF command - filter time constant
      sscanf(s, "%lf", &OSC_FILTER_TC);
      calc_osc_k_factors();
   }
   else if(c == 'H') {  // BH command - PID debug (help) display
      osc_pid_debug = toggle_value(osc_pid_debug);
      if(osc_pid_debug) pid_debug = 0;
      if(kbd_cmd) clear_osc_pid_display();
   }
   else if(c == 'I') {  // BI command - integral time constant
      sscanf(s, "%lf", &OSC_I_TC);
      calc_osc_k_factors();
   }
   else if(c == 'K') {  // BK command - all major PID values
      OSC_FILTER_OFFSET = 0.0F;
      sscanf(s, "%lf%c%lf%c%lf%c%lf", &OSC_P_GAIN,&c, &OSC_D_TC,&c,  &OSC_FILTER_TC,&c, &OSC_I_TC);
      calc_osc_k_factors();
      if(kbd_cmd) enable_osc_control();
      if(0 && kbd_cmd) show_osc_pid_values();
   }
   else if(c == 'L') {  // BL command - load distubance
      sscanf(s, "%lf", &osc_k6);
      calc_osc_k_factors();
   }
   else if(c == 'N') {  // BN command - mark's values
      set_default_osc_pid(1);
      if(kbd_cmd) enable_osc_control();
      if(0 && kbd_cmd) show_osc_pid_values();
   }
   else if(c == 'O') {  // BO command - filter offset
      sscanf(s, "%lf", &OSC_FILTER_OFFSET);
      calc_osc_k_factors();
   }
   else if(c == 'P') {  // BP command - pid gain
      sscanf(s, "%lf", &OSC_P_GAIN);
      calc_osc_k_factors();
   }
   else if(c == 'Q') {  // BQ command - post-filter depth
//    sscanf(s, "%lf", &OSC_KL_TUNE_STEP);
//    sscanf(s, "%d", &osc_prefilter);
//    if(osc_prefilter > PRE_Q_SIZE) osc_prefilter = PRE_Q_SIZE;
//    opq_in = opq_count = 0;
      sscanf(s, "%d", &osc_postfilter);
      if(osc_postfilter > POST_Q_SIZE) osc_postfilter = POST_Q_SIZE;
      new_postfilter();
      calc_osc_k_factors();
   }
   else if(c == 'R') {  // BR command - integrator reset
      sscanf(s, "%lf", &osc_k8);
      calc_osc_k_factors();
   }
   else if(c == 'S') {  // BS command - scale factor (loop gain correction)
      sscanf(s, "%lf", &osc_k7);
      calc_osc_k_factors();
   }
   else if(c == 'W') {  // BW command - Warren's slow values
      set_default_osc_pid(0);
      if(kbd_cmd) enable_osc_control();
      if(0 && kbd_cmd) show_osc_pid_values();
   }
   else if(c == 'X') {  // BX command - John's values
      set_default_osc_pid(2);
      if(kbd_cmd) enable_osc_control();
      if(0 && kbd_cmd) show_osc_pid_values();
   }
   else if(c == 'Y') {  // BY command - TBD values
      set_default_osc_pid(3);
      if(kbd_cmd) enable_osc_control();
      if(0 && kbd_cmd) show_osc_pid_values();
   }
   else if(c == 'Z') {  // BZ command - reset PID
      reset_osc_pid(kbd_cmd);
   }
   else if(c == '0') {  // B0 command - turn off pid
      disable_osc_control();
   }
   else if(c == '1') {  // B1 command - turn pid on
      enable_osc_control();
   }
   else return 1;

   return 0;
}

#endif // OSC_CONTROL


int adjust_screen_options()
{
int stm;
int old_azel;

   stm = 0;   // set text mode
   old_azel = plot_azel;

// plot_signals = 0;
   if(screen_type == 't') {  // text mode
      plot_azel = 0;
      stm = 2;
   }
   else if(screen_type == 'z') ; // plot_azel = AZEL_OK;
   else if(screen_type == 'h') ; // plot_azel = AZEL_OK;
   else if(screen_type == 'x') ; // plot_azel = AZEL_OK;
   else if(screen_type == 'v') ; // plot_azel = AZEL_OK;
   else if(screen_type == 'l') ; // plot_azel = AZEL_OK;
   else if(screen_type == 'm') ; // plot_azel = 0;
   else if(screen_type == 'n') ; // plot_azel = 0;          // netbook
   else if(screen_type == 's') ; // plot_azel = 0;
   else if(screen_type == 'u') ; // plot_azel = 0;
   else if(screen_type == 'c') ; // plot_azel = 0;   //!!! custom screen
   else if(screen_type == 'f') {
      plot_azel = 0;
      go_fullscreen = 1;
      need_screen_init = 1;
   }
   else {
      screen_type = 'm';
      text_mode = stm;
      return 1;
   }
   if(plot_azel || plot_signals) update_azel = 1;
   text_mode = stm;

   if(plot_watch) {
      plot_azel = old_azel;
      if(user_set_clock_plot == 0) {
         if(rcvr_type == NO_RCVR) plot_digital_clock = 1;
         else                     plot_digital_clock = 0;
      }
      if(plot_azel || plot_signals) update_azel = 1;
   }
   else if(user_set_clock_plot == 0) {
      if(rcvr_type == NO_RCVR) plot_digital_clock = 1;
      else                     plot_digital_clock = plot_azel;
   }
// shared_plot = 0;

   if(screen_type == 'm') {
      if((TEXT_HEIGHT <= 12) && (user_set_clock_plot == 0)) plot_digital_clock = 1;
   }

   if(ebolt) {  // adjust ebolt sat count for time clock display
      last_ebolt = (-3);
      saw_ebolt();
   }
   return 0;
}

void change_screen_res()
{
int k;

   first_key = 0;
   prot_menu = 0;
   eofs = 1;

   adjust_screen_options();
   init_screen();
   if((rcvr_type != SCPI_RCVR) && (rcvr_type != UCCM_RCVR) && (rcvr_type != STAR_RCVR)) {
      init_messages(40);  // zorky
   }

   // set where the first point in the graphs will be
   last_count_y = PLOT_ROW + (PLOT_HEIGHT - VERT_MINOR);
   for(k=0; k<NUM_PLOTS+DERIVED_PLOTS; k++) {
      plot[k].last_y = PLOT_ROW+PLOT_CENTER;
   }

   redraw_screen();
}

#ifdef WINDOWS
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
#endif

void edit_screen_res()
{
int font_set;
int n;
char c;

   // change the screen size
   strupr(edit_buffer);
   if(strstr(edit_buffer, "N")) return;

   user_video_mode = (int) atof(edit_buffer);
   font_set = 0;
   if     (strstr(edit_buffer, ":")) font_set = 12;
   else if(strstr(edit_buffer, "S")) font_set = 12;
   else if(strstr(edit_buffer, "M")) font_set = 14;
   else if(strstr(edit_buffer, "L")) font_set = 16;
   else if(strstr(edit_buffer, "T")) font_set = 8;
   else                              font_set = 0;

   if(strstr(edit_buffer, "I")) invert_screen();

   if(screen_type == 'c') {  // custom screen size
      if(strstr(edit_buffer, "N")) return;

      custom_width = custom_height = (-1);
      n = sscanf(edit_buffer, "%d%c%d", &custom_width,&c,&custom_height);
      if(n != 3) return;
      if(custom_width < 0) return;
      if(custom_height < 0) return;

      if(custom_width < MIN_WIDTH) custom_width = MIN_WIDTH;
      if(custom_height < MIN_HEIGHT) custom_height = MIN_HEIGHT;
   }
   else if(screen_type == 'f') {
      custom_width = display_width;
      custom_height= display_height;
#ifdef WINDOWS
      custom_width -= 10;
      custom_height -= 32;
#endif
      screen_type = 'c';
   }

   #ifdef WINDOWS
      if(font_set == 0) {
if(screen_type == 'c') ; else
         if(strstr(edit_buffer, "Y")) ;
         else if(strstr(edit_buffer, "I")) ;
         else if(0 && strstr(edit_buffer, "F")) ;
         else return;
      }
      if(0 && strstr(edit_buffer, "F")) initial_window_mode = VFX_TRY_FULLSCREEN;
      else initial_window_mode = VFX_WINDOW_MODE;
   #endif

   if((user_font_size == 0) && (SCREEN_HEIGHT < 600)) font_set = 12;
   user_font_size = font_set;

   change_screen_res();
}

void new_screen(u08 c)
{
char *s;

   // setup for a new screen size
   screen_type = c;   // $ command
   if(c == 'u') {
      strcpy(edit_buffer, "18");
      s="640x480";
   }
   else if (c == 's') {
      strcpy(edit_buffer, "258");
      s="800x600";
   }
   else if(c == 'm') {
      strcpy(edit_buffer, "260");
      s="1024x768";
   }
   else if(c == 'l') {
      strcpy(edit_buffer, "262");
      s="1280x1024";
   }
   else if(c == 'v') {
      strcpy(edit_buffer, "327");
      s="1400x1050";
   }
   else if(c == 'x') {
      strcpy(edit_buffer, "304");  // or mode 325
      s="1680x1050";
   }
   else if(c == 'h') {
      strcpy(edit_buffer, "319");
      s="1920x1080";
   }
   else if(c == 'z') {
      strcpy(edit_buffer, "338");
      s="2048x1530";
   }
   else if(c == 't') {
      s="640x480 text only";
      strcpy(edit_buffer, "2");
   }
   else if(c == 'n') {
      s="1000x540";
      strcpy(edit_buffer, "2");
   }
   else if(c == 'c') {
      s="Custom screen size";
      strcpy(edit_buffer, "2");
   }
   else if(c == 'f') {
      s="Full screen size";
      sprintf(out, "%dx%d", display_width,display_height);
      s = &out[0];
      strcpy(edit_buffer, "2");
   }
   else              {
      screen_type = 'm';
      strcpy(edit_buffer, "260");
      s="1024x768";
   }

   if(c == 'c') {
      edit_info1 = "Enter the desired screen config like 1024x768";
      sprintf(edit_buffer,"%dx%d", SCREEN_WIDTH,SCREEN_HEIGHT);
      sprintf(out, "%s screen: optionally add T)iny  S)mall  M)edium  L)arge font:", s);
   }
   else {
      strcpy(edit_buffer, "Y");
      sprintf(out, "%s screen: Y)es  N)o  I)nvert  +  Font: T)iny  S)mall  M)edium  L)arge", s);
   }
   start_edit('$', out);
}



void update_check()
{
// if(strchr(edit_buffer, 'u') || strchr(edit_buffer, 'U')) {
   if(!strchr(edit_buffer, 'u') && !strchr(edit_buffer, 'U')) {
      pause_data = 0;    // release data pause
      new_queue(3);      // erase the data queue
      update_stop = 1;   // stop updating data at end of run
   }
}

void write_luxor_config()
{
FILE *file;

   if(!strchr(edit_buffer, '.')) strcat(edit_buffer, ".scr");

   file = topen(edit_buffer, "w");
   if(file == 0) {
      sprintf(out, "ERROR: could not write file: %s", edit_buffer);
      edit_error(out);
      return;
   }

   fprintf(file, "PL %f  ;BATT low voltage\n", batt_lvc);
   fprintf(file, "PH %f  ;BATT high voltage\n", batt_hvc);
   fprintf(file, "PC %f  ;BATT overcurrent\n", batt_ovc);
   fprintf(file, "PW %f  ;BATT over-wattage\n", batt_watts);
   fprintf(file, "PU %f  ;LED low voltage\n", load_lvc);
   fprintf(file, "PV %f  ;LED high voltage\n", load_hvc);
   fprintf(file, "PO %f  ;LED overcurrent\n", load_ovc);
   fprintf(file, "PP %f  ;LED over-wattage\n", load_watts);
   fprintf(file, "PZ %f  ;AUX low voltage\n", auxv_lvc);
   fprintf(file, "PX %f  ;AUX high voltage\n", auxv_hvc);
   fprintf(file, "PT %f  ;TEMP1 overtemp\n", tc1_ovt);
   fprintf(file, "PS %f  ;TEMP2 overtemp\n", tc2_ovt);
   fprintf(file, "PM %f  ;Msg timeout\n", msg_timeout);
   fprintf(file, "&E %f  ;IR1 temp sensor emissivity\n", emis1);
   fprintf(file, "&I %f  ;IR2 temp sensor emissivity\n", emis2);
   fprintf(file, "BP %-2d   ;BATT pwm resolution\n", batt_pwm_res);
   fprintf(file, "BR %-2d   ;BATT pwm sweep rate\n", sweep_rate);
   fprintf(file, "&L %-3d  ;LUX sensor sensitivity\n", lux1_time);
   fprintf(file, "&U %-3d  ;LUMEN sensor sensitivity\n", lux2_time);
   fclose(file);
}

void set_cc_mode(int mode, float volts)
{
   if(cc_val) {
      cc_mode = mode;
      lipo_volts = volts;
      cc_state = 0;
      cc_pwm = 0.0F;
      update_check();
   }
   else {
      cc_mode = 0;
      lipo_volts = 0.0F;
      cc_state = 0;
      sweep_stop = 0.0F;
      update_stop = 0;
   }
   cc_pwm = 0.0F;
   set_batt_pwm(0x0000);
}


void set_sunrise_type(char *s, int kbd_cmd)
{
char c;
double jd, jd_nm;
int n;

   need_sunrise = 0;
   realtime_sun = 0;
   play_sun_song = 0;
   do_moonrise = 0;
   if(s == 0) return;
// if(s[0] == 0) return;

   c = 'O';
   while(*s) {
      c = *s;
      if(c == ' ') ;
      else if(c == '\t') ;
      else if(c == '!') realtime_sun = 1;
      else if(c == '*') play_sun_song = 1;
      else break;
      ++s;
   }
   if(c == 0) c = 'O';

   need_sunrise = 1;
   apply_refraction = 0;
   user_set_short = 1;   // abbreviated sat info display
if(kbd_cmd) max_sat_display = max_sat_count;
   c = toupper(c);
   if(c == 'A') {
      sunrise_type = "Astronomical";
      sunrise_horizon = (-18.0);
      sunset_horizon = sunrise_horizon;
   }
   else if(c == 'C') {
      sunrise_type = "Civil";
      sunrise_horizon = (-6.0);
      sunset_horizon = sunrise_horizon;
   }
   else if(c == 'M') {    // Substitute moon info for sun info
      sunrise_type = "Moon";
      do_moonrise = 1;
      sunrise_horizon = (0.0 - ((35.4/60.0) + moon_diam(jd_utc)/2.0));
      sunset_horizon = sunrise_horizon;
//sprintf(plot_title, "moon diam:%f  hor:%f", MoonDisk, sunrise_horizon);
   }
   else if(c == 'N') {
      sunrise_type = "Nautical";
      sunrise_horizon = (-12.0);
      sunset_horizon = sunrise_horizon;
   }
   else if(c == 'O') {
      sunrise_type = "Official";
      sunrise_horizon = (-50.0/60.0);
      sunset_horizon = sunrise_horizon;
   }
   else if(c == 'P') {
      sunrise_type = "Physical";
      sunrise_horizon = 0.0 - (sun_diam(jd_utc)/2.0);
      sunset_horizon = sunrise_horizon;
   }
   else if((c == '+') || (c == '-') || (c == '.') || isdigit(c)) {  // user define angles
      sunrise_type = "User";

      n = sscanf(s, "%lf%c%lf", &sunrise_horizon,&c,&sunset_horizon);

      sunrise_horizon = sunset_horizon = 0.0;
      if(n == 0) sunrise_horizon = sunset_horizon = 999.0;
      else if(n < 3) sunset_horizon = sunrise_horizon;

      if((sunrise_horizon > 90.0) || (sunset_horizon > 90.0) || (sunrise_horizon < (-90.0)) || (sunset_horizon < (-90.0))) { 
         sunrise_type = 0;
         need_sunrise = 0;
         play_sun_song = 0;
         realtime_sun = 0;
         return;
      }
   }
else if(c == 'X') {  // debug info
   for(pri_month=1; pri_month<=12; pri_month++) {
      if(debug_file) {
         jd = jdate(pri_year,pri_month,pri_day) + jtime(pri_hours,pri_minutes,pri_seconds,0.0);
         gregorian(jd);
         fprintf(debug_file, "\ncheck new moon day: %02d:%02d:%02d %02d/%02d/%04d  jd:%f\n",
            g_hours,g_minutes,g_seconds, g_month,g_day,g_year, jd);
         jd_nm = new_moon(jd);  // get this month's new moon
         gregorian(jd_nm-jtime(0,0,0,0.0));
         fprintf(debug_file, "new moon day: %02d:%02d:%02d %02d/%02d/%04d  jd:%f\n",
            g_hours,g_minutes,g_seconds, g_month,g_day,g_year, jd);
      }
   }

   for(pri_month=1; pri_month<=12; pri_month++) {
      for(pri_day=1; pri_day<31; pri_day++) {
         sunrise_type = "Moon";
         do_moonrise = 1;
         sunrise_horizon = (-51.2583333/60.0);
         sunset_horizon = sunrise_horizon;
         calc_moonrise();

         sunrise_type = "Official";
         do_moonrise = 0;
         sunrise_horizon = (-50.0/60.0);
         sunset_horizon = sunrise_horizon;
         calc_sunrise(0.0, 10);
      }
   }
   need_sunrise = 0;
}
   else {
      sunrise_type = "Official";
      sunrise_horizon = (-50.0/60.0);
      sunset_horizon = sunrise_horizon;
   }

   if(strchr(s, '*')) play_sun_song = 1;    // enable sun song
   if(strchr(s, '!')) realtime_sun = 1;     // recalc times every second
   if(strchr(s, 'r')) apply_refraction = 1; // apply refraction correction
   if(strchr(s, 'R')) apply_refraction = 1; // apply refraction correction
}



u08 string_param()
{
float val;
u08 c;
int i;
int survey_scale;

   // this routine evalates the string in edit_buffer
   // and sets the appropriate parameter value based upon 'getting_string'
   //
   // returns 2 if new queue setting
   // returns 1 if screen redraw needed
   //

   if(getting_string == 'a') {  // all_adevs display mode
      strupr(edit_buffer);
      if     (edit_buffer[0] == 'A') mixed_adevs = 0;
      else if(edit_buffer[0] == 'G') mixed_adevs = 1;
      else if(edit_buffer[0] == 'R') mixed_adevs = 2;
      else if(edit_buffer[0] == 'N') mixed_adevs = 2;
      plot_adev_data = 1;
      all_adevs = aa_val;
      config_screen(103);
   }
#ifdef PRECISE_STUFF
   else if(getting_string == 'A') {  // abort precise survey
      if((tolower(edit_buffer[0]) == 'y') || (tolower(edit_buffer[0]) == 'n')) {
         check_precise_posn = 0;
         plot_lla = 0;
         stop_self_survey();        // stop any self survey
         if(tolower(edit_buffer[0]) == 'y') {
            abort_precise_survey(10);  // save current position using TSIP message
         }
         stop_precision_survey();
      }
   }
#endif
#ifdef OSC_CONTROL
   else if(getting_string == 'b') {  // osc control PID
      if(edit_osc_pid_value(edit_buffer[0], &edit_buffer[1], 1)) {
         edit_error("Unknown osc PID command");
      }
   }
#endif
   else if(getting_string == 'c') {  // DAC control voltage
      if(edit_buffer[0] == 0) {
         if(rcvr_type == UCCM_RCVR) val = uccm_voltage;
         else                       val = dac_voltage;
      }
      else val = (float) atof(edit_buffer);

      if(rcvr_type == UCCM_RCVR) ;
      else if(val < (-5.0F)) val = (-5.0F);  // !!!!! is this a proper range of limits?
      else if(val > 5.0F) val = 5.0F;
      set_discipline_mode(4);
      set_dac_voltage(val, 10);
      osc_discipline = 0;
   }
   else if(getting_string == 'C') { // center line zero reference
      if(edit_buffer[0]) {
         val = (float) atof(edit_buffer);
         plot[selected_plot].plot_center = val;
         plot[selected_plot].float_center = 0;
      }
      else plot[selected_plot].float_center = 1;
      return 0;
   }
   else if(getting_string == 'd') {  // plot scale factor
      if(edit_buffer[0] == 0) val = 0.0F;
      else                    val = (float) atof(edit_buffer);
      if(val < 0.0F) val = 0.0F-val;
      if(strstr(edit_buffer, "-")) plot[selected_plot].invert_plot = (-1.0);
      else                         plot[selected_plot].invert_plot = 1.0;
      plot[selected_plot].user_scale = 1;
      if(val == 0.0F) plot[selected_plot].user_scale = 0;
      else            plot[selected_plot].scale_factor = val;
      set_steps();
      return 0;
   }
   else if(getting_string == 'e') {  // elevation angle
      if(edit_buffer[0]) {
         val = (float) atof(edit_buffer);
         if((val < 0.0F) || (val > 90.0F)) edit_error("Invalid elevation angle");
         else set_el_mask(val);
      }
      else {   // set recommended level
         set_el_level();
         request_rcvr_config(10);
      }
      return 0;
   }
   else if(getting_string == 'f') {  // display filter
      if(strstr(edit_buffer, "-"))      plot[PPS].invert_plot = plot[TEMP].invert_plot = (-1.0);
      else if(strstr(edit_buffer, "+")) plot[PPS].invert_plot = plot[TEMP].invert_plot = (+1.0);
      if(edit_buffer[0]) {
         filter_count = (long) atof(edit_buffer);
         if(filter_count < 0) filter_count = 0 - filter_count;
      }
      else filter_count = 0;
      config_screen(104);
   }
   else if(getting_string == 'g') {       // set plot color
      if(edit_buffer[0]) {
         val = (float) (int) atof(edit_buffer);
         if((val >= 0.0F) && (val < 16.0F)) {
            plot[selected_plot].plot_color = (int) val;
         }
      }
   }
   else if(getting_string == 'h') {       // set chime interval
      strupr(edit_buffer);
      cuckoo_hours = singing_clock = ships_clock = 0;
      if(strstr(edit_buffer, "H")) cuckoo_hours  = 1;
      if(strstr(edit_buffer, "S")) singing_clock = 1;
      else if(strstr(edit_buffer, "B")) ships_clock  = 1;
      val = (float) atof(edit_buffer);
      if(val < 0.0F)  val = 0.0F - val;
      if(val > 60.0F) val = 4.0F;
//    if(ships_clock) val = 1;
      cuckoo = (u08) val;
   }
   else if(getting_string == 'H') {  // Graph title
      strcpy(plot_title, edit_buffer);
      if(plot_title[0]) title_type = USER;
      else              title_type = NONE;

      sprintf(log_text, "#TITLE: %s", plot_title);
      write_log_comment(1);
      return 1;
   }
   else if(getting_string == 'i') {       // set queue interval
      if(edit_buffer[0]) {
         val = (float) atof(edit_buffer);
         if(val < 1.0) {
            edit_error("Bad queue update interval");
         }
         else {   // !!!!!! luxor
            queue_interval = (long) (val+0.5F);
            plot[DAC].plot_center = last_dac_voltage = 1.0;
            last_temperature = 30.0F;
            plot[TEMP].plot_center = scale_temp(last_temperature);
            return 2;
         }
      }
   }
   else if(getting_string == 'j') {  // log interval
      val = (float) atof(edit_buffer);
      if(val < 0.0F) val = 0.0F - val;
      if(val == 0.0F) val = 1.0F;
      log_interval = (long) val;
      sync_log_file();
      return 1;
   }
#ifdef TEMP_CONTROL
   else if(getting_string == 'k') {  // temperature control PID
      if(edit_temp_pid_value(edit_buffer[0], &edit_buffer[1], 1)) {
         edit_error("Unknown temperature PID command");
      }
   }
#endif
   else if(getting_string == 'l') {  // enter precise lat/lon/alt
      edit_lla_cmd();
   }
#ifdef PRECISE_STUFF
   else if(getting_string == 'L') {  // abort lat/lon/alt position save search
      if((tolower(edit_buffer[0]) == 'y') || (tolower(edit_buffer[0]) == 'n')) {
         check_precise_posn = 0;
         plot_lla = 0;
         stop_self_survey();        // stop any self survey
         if(tolower(edit_buffer[0]) == 'y') {  
            save_precise_posn(1);  // save position using single precision numbers
         }
      }
   }
#endif
   else if(getting_string == 'm') {  // enter AMU mask
      if(edit_buffer[0]) {
         val = (float) atof(edit_buffer);
         if(val < 0.0F) edit_error("Invalid signal level mask");
         else set_amu_mask(val);
      }
      return 0;
   }
   else if(getting_string == 'n') {   // log file name
      if(edit_buffer[0] == 0) {
         edit_error("No file name given");
      }
      else {
         strcpy(log_name, edit_buffer);
         open_log_file(log_mode);
         log_written = 0;
         if(log_file == 0) edit_error("Could not open log file.");
      }
   }
   else if(getting_string == 'o') {  // fOliage or Jamming mode
      if((res_t && (res_t != RES_T)) || saw_icm || (rcvr_type == VENUS_RCVR)) {
         if     (tolower(edit_buffer[0]) == 'n') set_foliage_mode(0);
         else if(tolower(edit_buffer[0]) == 's') set_foliage_mode(1);
         else edit_error("Jamming mode must be S) enable  N) disable");
      }
      else {
         if     (tolower(edit_buffer[0]) == 'n') set_foliage_mode(0);
         else if(tolower(edit_buffer[0]) == 's') set_foliage_mode(1);
         else if(tolower(edit_buffer[0]) == 'a') set_foliage_mode(2);
         else edit_error("Foliage mode must be A)lways  S)ometimes  N)ever");
      }
      return 0;
   }
   else if(getting_string == 'p') {  // pdop mask
      if(edit_buffer[0]) {
         val = (float) atof(edit_buffer);
         if((val < 1.0F) || (val > 100.0F)) edit_error("PDOP make must be 1..100");
         else set_pdop_mask(val);
      }
      return 0;
   }
   else if(getting_string == 'Q') {
      strupr(edit_buffer);
      if     (edit_buffer[0] == 'A')  plot_signals = 1;
      else if(edit_buffer[0] == 'W')  plot_signals = 2;
      else if(edit_buffer[0] == 'E')  plot_signals = 3;
      else if(edit_buffer[0] == 'S')  plot_signals = 4;
      else if(edit_buffer[0] == 'D')  plot_signals = 5;
      else if(edit_buffer[0] == 'R')  clear_signals();
      else if(edit_buffer[0] == 'C')  clear_signals();
      else if(edit_buffer[0] == 0x0A) plot_signals = 0;
      else if(edit_buffer[0] == 0x0D) plot_signals = 0;
      else if(edit_buffer[0] == 0)    plot_signals = 0;
      if(plot_signals == 0) {
         if(zoom_screen) {
            change_zoom_config(0);
            zoom_screen = 0; 
         }
      }
      config_screen(105);
   }
   else if(getting_string == 'r') {   // read in a log file
      c = reload_log(edit_buffer, 0);
      if(c == 0) { // adv, tim, log file
         plot_review(0L);
         #ifdef ADEV_STUFF
            force_adev_redraw();    // and make sure adev tables are showing
         #endif
         pause_data = user_pause_data^1;
      }
      else if(c == 3) {  // lla file
         plot_review(0L);
         pause_data = user_pause_data^1;
         return 0;
      }
      else if((c ==  1) || (c == 2)) return 0;   // bad file
   }
   else if(getting_string == 's') {  // do self survey
      if(edit_buffer[0]) {
         do_survey = (long) atof(edit_buffer);
         survey_why = 30;

         strupr(edit_buffer);
         survey_scale = 1;
         if(rcvr_type == ZODIAC_RCVR) ;
         else if(strchr(edit_buffer, 'M')) survey_scale = 60;
         else if(strchr(edit_buffer, 'H')) survey_scale = 60*60;
         else if(strchr(edit_buffer, 'D')) survey_scale = 60*60*24;
         do_survey *= survey_scale;

         if(do_survey < 0L) {
            edit_error("Bad self survey size");
         }
         else {
            #ifdef PRECISE_STUFF
               stop_precision_survey();
            #endif
            set_survey_params(1, 1, (long) do_survey);
            request_survey_params();  //!!!
            start_self_survey(0x00, 1);
         }
      }
   }
   else if(getting_string == 'u') {  // receiver dynamics
      if     (tolower(edit_buffer[0]) == 'a') set_rcvr_dynamics(3);
      else if(tolower(edit_buffer[0]) == 'f') set_rcvr_dynamics(4);
      else if(tolower(edit_buffer[0]) == 'l') set_rcvr_dynamics(1);
      else if(tolower(edit_buffer[0]) == 's') set_rcvr_dynamics(2);
      else edit_error("Movement dynamics must be:  A)ir  F)ixed  L)and  S)ea");
      return 0;
   }
   else if(getting_string == MARINE_CMD) {
      if(edit_buffer[0]) {
         i = atoi(edit_buffer);
         if((i >= 10) && (i <= 100)) {
            set_filter_config(pv_filter, static_filter, alt_filter, kalman_filter, i, 1);
         }
         else edit_error("Marine filter value must be between 10 (max) and 100 (none)");
      }
      return 0;
   }
   else if(getting_string == 'v') {  // set show interval
      if(edit_buffer[0]) {
         edit_user_view(&edit_buffer[0]);
      }
   }
   else if(getting_string == 'w') {  // Write queue data to log file
      edit_write_cmd();
      return 0;
   }
   else if(getting_string == 'x') {  // single sat mode
      if(edit_buffer[0] == 0) val = 0.0F;
      else val = (float) atof(edit_buffer);
      if(val < 0) val = 0.0F;
      else if(val > MAX_PRN) val = 0.0F;
      single_sat = (u08) val;
      set_single_sat(single_sat);
//    do_fixes(1);
   }
   else if(getting_string == SAT_IGN_CMD) {  // exclude sat mode
      if(edit_buffer[0] == 0) val = 0.0F;
      else val = (float) atof(edit_buffer);
      if(val < 0) val = 0.0F;
      else if(val > MAX_PRN) val = 0.0F;
      exclude_sat((u08) val);
//    do_fixes(1);
   }
   else if(getting_string == 'Z') {  // set time zone
      set_time_zone(edit_buffer);
      calc_dst_times(dst_list[dst_area]);
   }
#ifdef PRECISE_STUFF
   else if(getting_string == '^') {  // do precision survey;
      if(edit_buffer[0] == 0) val = 48.0F;
      else val = (float) atof(edit_buffer);
      if((val < 1.0) || (val > (double) SURVEY_BIN_COUNT)) {
         sprintf(out, "Value must be between 1 and %d.  48 is good.", SURVEY_BIN_COUNT);
         edit_error(out);
      }
      else {
         do_survey = (long) val;
         survey_why = 31;
         start_precision_survey();
      }
   }
#endif
#ifdef TEMP_CONTROL
   else if(getting_string == '*') {  // set control temperature;
      if(edit_buffer[0] == 0) val = 0.0F;
      else val = (float) atof(edit_buffer);
      spike_delay = 0;
      if(val == 0.0) {
         disable_temp_control();
         desired_temp = 0.0;
      }
      else if((val < 10.0) || (val > 50.0)) {
         edit_error("Control temperature must be between 10 and 50 degrees C");
      }
      else {
         desired_temp = val;
         enable_temp_control();
      }
   }
#endif
   else if(getting_string == '$') {  // select a new screen resolution
      edit_screen_res();
      set_restore_size();
   }
   else if(getting_string == '~') {  // select a plot statistic
      c = tolower(edit_buffer[0]);
      if(all_plots) {
         for(i=0; i<NUM_PLOTS+DERIVED_PLOTS; i++) {
            if     (c == 'a') plot[i].show_stat = AVG;
            else if(c == 'r') plot[i].show_stat = RMS;
            else if(c == 's') plot[i].show_stat = SDEV;
            else if(c == 'v') plot[i].show_stat = VAR;
            else if(c == 'n') plot[i].show_stat = SHOW_MIN;
            else if(c == 'x') plot[i].show_stat = SHOW_MAX;
            else if(c == 'p') plot[i].show_stat = SHOW_SPAN;
            else if(c == ESC_CHAR) ;
            else              plot[i].show_stat = 0;
         }
      }
      else {
         if     (c == 'a') plot[selected_plot].show_stat = AVG;
         else if(c == 'r') plot[selected_plot].show_stat = RMS;
         else if(c == 's') plot[selected_plot].show_stat = SDEV;
         else if(c == 'v') plot[selected_plot].show_stat = VAR;
         else if(c == 'n') plot[selected_plot].show_stat = SHOW_MIN;
         else if(c == 'x') plot[selected_plot].show_stat = SHOW_MAX;
         else if(c == 'p') plot[selected_plot].show_stat = SHOW_SPAN;
         else if(c == ESC_CHAR) ;
         else              plot[selected_plot].show_stat = 0;
      }
      plot_stat_info = 0;
      for(c=0; c<NUM_PLOTS+DERIVED_PLOTS; c++) {
         plot_stat_info |= plot[c].show_stat;
      }
   }
   else if(getting_string == ',') {  // enter debug value
      edit_option_value();
   }
   else if(getting_string == '-') {  // do command line option
      edit_option_switch();
   }
   else if(getting_string == '=') {  // remove drift rate from plot
      if(edit_buffer[0] == 0) val = 0.0F;
      else val = (float) atof(edit_buffer);
      plot[selected_plot].drift_rate = val;
   }
   else if(getting_string == '%') {  // az/el signal level data
      if(edit_buffer[0] == 0) {
         sprintf(edit_buffer, "%s.sig", unit_file_name);
      }
      dump_signals(edit_buffer);
   }
   else if((getting_string == ':') || (getting_string == '/') || (getting_string == '!') || (getting_string == '(')) {  // set alarm or exit
      edit_dt(edit_buffer, 1);
   }
   else if(getting_string == 't') {  // efc range
      edit_osc_param();
   }
   else if(getting_string == 'T') {  // efc range
      edit_osc_param();
   }
   else if((getting_string >= '1') && (getting_string <= '8')) { // oscillator disciplinging param
      edit_osc_param();
   }
   else if(getting_string == '9') {  // cable delay
      edit_cable_delay(1);
   }
   else if(getting_string == REF_CMD) {
      strupr(edit_buffer);
      if     (edit_buffer[0] == 'G') set_ref_input(0);
      else if(edit_buffer[0] == 'P') set_ref_input(1); 
      else if(edit_buffer[0] == 'A') set_ref_input(1); 
      else  edit_error("Invalid GPSDO reference type");
   }
   else if(getting_string == PALETTE_CMD) {
      change_palette(edit_buffer);
   }
   else if(getting_string == DRVR_MODE) {
      strcpy(mode_string, edit_buffer);
      set_driver_mode();
   }
   else if(getting_string == PC_CMD) {
      if(edit_buffer[0]) batt_ovc = (float) atof(edit_buffer);
      set_luxor_config();
   }
   else if(getting_string == PH_CMD) {
      if(edit_buffer[0]) batt_hvc = (float) atof(edit_buffer);
      set_luxor_config();
   }
   else if(getting_string == PL_CMD) {
      if(edit_buffer[0]) batt_lvc = (float) atof(edit_buffer);
      set_luxor_config();
   }
   else if(getting_string == PM_CMD) {
      if(edit_buffer[0]) msg_timeout = (float) atof(edit_buffer);
      set_luxor_config();
   }
   else if(getting_string == PO_CMD) {
      if(edit_buffer[0]) load_ovc = (float) atof(edit_buffer);
      set_luxor_config();
   }
   else if(getting_string == PP_CMD) {
      if(edit_buffer[0]) load_watts = (float) atof(edit_buffer);
      set_luxor_config();
   }
   else if(getting_string == PR_CMD) {
      if(tolower(edit_buffer[0]) == 'r') reset_luxor_wdt(0x01);
   }
   else if(getting_string == PS_CMD) {
      if(edit_buffer[0]) tc2_ovt = (float) atof(edit_buffer);
      set_luxor_config();
   }
   else if(getting_string == PT_CMD) {
      if(edit_buffer[0]) tc1_ovt = (float) atof(edit_buffer);
      set_luxor_config();
   }
   else if(getting_string == PU_CMD) {
      if(edit_buffer[0]) load_lvc = (float) atof(edit_buffer);
      set_luxor_config();
   }
   else if(getting_string == PV_CMD) {
      if(edit_buffer[0]) load_hvc = (float) atof(edit_buffer);
      set_luxor_config();
   }
   else if(getting_string == PW_CMD) {
      if(edit_buffer[0]) batt_watts = (float) atof(edit_buffer);
      set_luxor_config();
   }
   else if(getting_string == PX_CMD) {
      if(edit_buffer[0]) auxv_hvc = (float) atof(edit_buffer);
      set_luxor_config();
   }
   else if(getting_string == PZ_CMD) {
      if(edit_buffer[0]) auxv_lvc = (float) atof(edit_buffer);
      set_luxor_config();
   }
   else if(getting_string == WC_CMD) {
      write_luxor_config();
   }
   else if(getting_string == S_CMD) {
      lat = lon = alt = 0.0;
      sscanf(edit_buffer, "%lf %lf %lf", &lat, &lon, &alt);
      lat /= RAD_TO_DEG;
      lon /= RAD_TO_DEG;
      precise_lat = lat;
      precise_lon = lon;
      precise_alt = alt;
      #ifdef BUFFER_LLA
//lla    clear_lla_points();
      #endif
   }
   else if(getting_string == BC_CMD) {  // constant current load
      if(edit_buffer[0]) cc_val = (float) atof(edit_buffer);
      strupr(edit_buffer);
      if(strstr(edit_buffer, "S")) sweep_stop = cc_val;
      else if(strstr(edit_buffer, "R")) sweep_stop = cc_val;
      unsafe_v = 0.0F;
      set_cc_mode(CC_AMPS, 0.00F);
   }
   else if(getting_string == BF_CMD) {  // lifepo4 battery charge mode
      if(edit_buffer[0]) cc_val = (float) atof(edit_buffer);
      unsafe_v = 2.0F;
      if(strstr(edit_buffer, "unsafe")) unsafe_v = 0.0F;
      if(led_v < unsafe_v) {
         edit_error("Battery voltage is too low to safely charge!");
      }
      else if(led_v > (3.60F-0.05F)) {
         edit_error("Battery appears to be fully charged!");
      }
      else {
         sweep_stop = 0.0F;
         set_cc_mode(CC_LIPO, 3.60F);
      }
   }
   else if(getting_string == BH_CMD) {  // high voltage lipo battery charge mode
      cc_val = 0.0F;
      lipo_volts = 0.0F;
      i = sscanf(edit_buffer, "%f %f", &cc_val, &lipo_volts);
      if(i != 2) {
         edit_error("ERROR: must specify charge curent and charge voltage!");
         return 0;
      }

      unsafe_v = lipo_volts * 0.60F;
      if(strstr(edit_buffer, "unsafe")) unsafe_v = 0.0F;
      if(led_v < unsafe_v) {
         edit_error("Battery voltage is too low to safely charge!");
      }
      else if(unsafe_v && (led_v > (lipo_volts-0.05F))) {
         edit_error("Battery appears to be fully charged!");
      }
      else {
         sweep_stop = 0.0F;
         set_cc_mode(CC_LIPO, lipo_volts);
      }
   }
   else if(getting_string == BL_CMD) {  // lipo battery charge mode
      if(edit_buffer[0]) cc_val = (float) atof(edit_buffer);
      unsafe_v = 2.50F;
      if(strstr(edit_buffer, "unsafe")) unsafe_v = 0.0F;
      if(led_v < unsafe_v) {
         edit_error("Battery voltage is too low to safely charge!");
      }
      else if(unsafe_v && (led_v > (4.20F-0.05F))) {
         edit_error("Battery appears to be fully charged!");
      }
      else {
         sweep_stop = 0.0F;
         set_cc_mode(CC_LIPO, 4.20F);
      }
   }
   else if(getting_string == BP_CMD) {  // PWM resolution
      if(strstr(edit_buffer, "8")) batt_pwm_res = 8;
      else if(strstr(edit_buffer, "9")) batt_pwm_res = 9;
      else batt_pwm_res = 10;
      set_luxor_config();
   }
   else if(getting_string == BR_CMD) {  // PWM sweep rate
      sweep_rate = 1;
      sscanf(edit_buffer, "%d", &sweep_rate);
   }
   else if(getting_string == BS_CMD) {  // PWM sweep
      sweep_start = sweep_end = 0.0F;
      sscanf(edit_buffer, "%f %f", &sweep_start, &sweep_end);
      if(sweep_start < 0.0F) sweep_start = 0.0F;
      if(sweep_start > 1.0F) sweep_start = 1.0F;
      if(sweep_end < 0.0F) sweep_end = 0.0F;
      if(sweep_end > 1.0F) sweep_end = 1.0F;
      cc_mode = PWM_SWEEP;
      update_check();
      sweep_stop = 0.0F;
      cc_state = 0;
      cc_pwm = sweep_start;
   }
   else if(getting_string == BV_CMD) {  // constant load volatge
      if(edit_buffer[0]) cc_val = (float) atof(edit_buffer);
      strupr(edit_buffer);
      if(strstr(edit_buffer, "S")) sweep_stop = cc_val;
      else if(strstr(edit_buffer, "R")) sweep_stop = cc_val;
      unsafe_v = 0.0F;
      set_cc_mode(CC_VOLTS, 0.00F);
   }
   else if(getting_string == BW_CMD) {  // constant load wattage
      if(edit_buffer[0]) cc_val = (float) atof(edit_buffer);
      strupr(edit_buffer);
      if(strstr(edit_buffer, "S")) sweep_stop = cc_val;
      else if(strstr(edit_buffer, "R")) sweep_stop = cc_val;
      unsafe_v = 0.0F;
      set_cc_mode(CC_WATTS, 0.00F);
   }
   else if(getting_string == AMPL_CMD) {
      strupr(edit_buffer);
      if     (strstr(edit_buffer, "L")) lux1_time = LOW_LUX;
      else if(strstr(edit_buffer, "M")) lux1_time = MED_LUX;
      else if(strstr(edit_buffer, "H")) lux1_time = HI_LUX;
      else {
         lux1_time = (u08) atof(edit_buffer);
         if(lux1_time < 1) lux1_time = 2;
         else if(lux1_time > MAX_LUX) lux1_time = MAX_LUX;
      }
      set_luxor_sens(lux1_time, lux2_time);
   }
   else if(getting_string == AMPU_CMD) {
      strupr(edit_buffer);
      if     (strstr(edit_buffer, "L")) lux2_time = LOW_LUX;
      else if(strstr(edit_buffer, "M")) lux2_time = MED_LUX;
      else if(strstr(edit_buffer, "H")) lux2_time = HI_LUX;
      else {
         lux2_time = (u08) atof(edit_buffer);
         if(lux2_time < 1) lux2_time = 2;
         else if(lux2_time > MAX_LUX) lux2_time = MAX_LUX;
      }
      set_luxor_sens(lux1_time, lux2_time);
   }
   else if(getting_string == USER_CMD) {
      send_user_cmd(edit_buffer);
   }
   else if(getting_string == AMPE_CMD) {
      emis1 = (float) atof(edit_buffer);
      if(emis1 < 0.1F) emis1 = 0.1F;
      else if(emis1 > 1.0F) emis1 = 1.0F;
      set_emissivity(emis1, emis2);
   }
   else if(getting_string == AMPI_CMD) {
      emis2 = (float) atof(edit_buffer);
      if(emis2 < 0.1F) emis2 = 0.1F;
      else if(emis2 > 1.0F) emis2 = 1.0F;
      set_emissivity(emis1, emis2);
   }
   else if(getting_string == AMPS_CMD) {
      strupr(edit_buffer);
      if(strstr(edit_buffer, "SERIAL")) {
         vref_b = (float) atof(edit_buffer);
         if(vref_b < 0.0F) vref_b = 0.0F;
         vref_b = vref_b * 10000.0F + 0.50F;
         vref_b = ((float) (int) vref_b) / 10000.0F;
         set_luxor_cal();
      }
      else {
         edit_error("You did not say the magic word!");
      }
   }
   else if(getting_string == AMPV_CMD) {
      strupr(edit_buffer);
      if(strstr(edit_buffer, "VREF")) {
         vref_m = (float) atof(edit_buffer);
         if(vref_m < 1.00F) vref_m = 5.0F;
         else if(vref_m > 6.50F) vref_m = 6.144F;
         set_luxor_cal();
      }
      else {
         edit_error("You did not say the magic word!");
      }
   }
   else if(getting_string == CAL_CMD) {
      edit_cal_param();
   }
   else if(getting_string == DEBUG_LOG_CMD) {
      if(debug_file) {
         strupr(edit_buffer);
         if(strchr(edit_buffer, 'Y')) {
            fclose(debug_file);
            debug_file = 0;
            debug_name[0] = 0;
            erase_screen();
            vidstr(0,0, YELLOW, "Closing debug log file.");
            refresh_page();
            Sleep(1000);
         }
      }
      else if(edit_buffer[0] == 0) {
         edit_error("No file name given for debug log file.");
      }
      else {
         open_debug_file(edit_buffer);
         if(debug_file == 0) edit_error("Could not open debug log file.");
      }
   }
   else if(getting_string == RAW_LOG_CMD) {   // log file name
      if(raw_file) {
         strupr(edit_buffer);
         if(strchr(edit_buffer, 'Y')) {
            fclose(raw_file);
            raw_file = 0;
            erase_screen();
            vidstr(0,0, YELLOW, "Closing raw receiver data log file.");
            refresh_page();
            Sleep(1000);
            log_stream &= (~0x02);
         }
      }
      else if(edit_buffer[0] == 0) {
         edit_error("No file name given for raw receiver data log file.");
      }
      else {
         strcpy(raw_name, edit_buffer);
         raw_file = topen(raw_name, "wb");
         if(raw_file == 0) edit_error("Could not open raw receiver data log file.");
         else log_stream |= 0x02;
      }
   }
   else if(getting_string == TRAIM_CMD) {
      i = atoi(edit_buffer);
      set_traim_mode(i, 1);
   }
   else if(getting_string == SI_CMD) {
      i = atoi(edit_buffer);
      if(strchr(edit_buffer, '+')) sat_cols = 2;
      else                         sat_cols = 1;
      if(strchr(edit_buffer, 't'))      tracked_only = 1;
      else if(strchr(edit_buffer, 'T')) tracked_only = 1;
      else                              tracked_only = 0;
      if((i > 0) && strchr(edit_buffer, '-')) i = 0 - i;
      if((i == 0) && (strchr(edit_buffer, '-') || strchr(edit_buffer, '+'))) {
         if(strchr(edit_buffer, '-')) user_set_short = 1;
         else user_set_short = 0;
      }
      else {
         max_sat_display = i;
         if(max_sat_display < 0) {
            max_sat_display = 0-max_sat_display;
            user_set_short = 1;
         }
         else user_set_short = 0;
      }
if((sat_cols > 1) && (max_sat_display < 16)) max_sat_display = 16;
      max_sats = max_sat_display;  // used to format the sat_info data
      max_sat_count = max_sat_display;
      temp_sats = max_sat_display;
      config_sat_rows();
      config_screen(666);
   }
   else if(getting_string == GNSS_CMD) {
      strupr(edit_buffer);
      i = 0;
      if(strchr(edit_buffer, 'A')) i |= MIXED;
      if(strchr(edit_buffer, 'B')) i |= BEIDOU;
      if(strchr(edit_buffer, 'D')) i |= default_gnss;
      if(strchr(edit_buffer, 'G')) i |= GPS;
      if(strchr(edit_buffer, 'I')) i |= IMES;
      if(strchr(edit_buffer, 'N')) i |= GLONASS;
      if(strchr(edit_buffer, 'L')) i |= GALILEO;
      if(strchr(edit_buffer, 'Q')) i |= QZSS;
      if(strchr(edit_buffer, 'S')) i |= SBAS;
      if(i == 0) edit_error("No valid GNSS system selections seen.");
      else set_gnss_system(i);
   }
   else if(getting_string == SORT_CMD) {
      strupr(edit_buffer);
      if(strchr(edit_buffer, 'G')) {
         plot_sat_count = toggle_value(plot_sat_count);
         draw_plot(1);
      }
      else if(strchr(edit_buffer, 'T')) {
         force_si_cmd = 1;  // royal kludge to implement G C T keyboard command
      }
      else if(strchr(edit_buffer, 'Z')) {  // G C Z command
         no_plots ^= 1;
         config_screen(7777);
      }
      else {
         sort_by = 0;
         sort_ascend = 1;

         if     (strchr(edit_buffer, 'P')) { sort_by = SORT_PRN;     sort_ascend = 1; }
         else if(strchr(edit_buffer, 'A')) { sort_by = SORT_AZ;      sort_ascend = 1; }
         else if(strchr(edit_buffer, 'E')) { sort_by = SORT_ELEV;    sort_ascend = 0; }
         else if(strchr(edit_buffer, 'S')) { sort_by = SORT_SIGS;    sort_ascend = 0; }
         else if(strchr(edit_buffer, 'D')) { sort_by = SORT_DOPPLER; sort_ascend = 1; }
         else edit_error("Not a valid sort selection.");

         if(sort_by) {
            if     (strchr(edit_buffer, '+')) sort_ascend = 1;
            else if(strchr(edit_buffer, '-')) sort_ascend = 0;
            if((sort_by == SORT_PRN) && sort_ascend) sort_by = 0; // don't color the PRN header
         }
      }
   }
   else if(getting_string == SERIAL_CMD) {
      set_com_params(&edit_buffer[0]);
      user_set_baud = 1;
      init_com();
      Sleep(200);
      request_rcvr_info(222);
   }
   else if(getting_string == PULLIN_CMD) {
      i = atoi(edit_buffer);
      set_pullin_range(i);
   }
   else if(getting_string == SUN_CMD) {
      set_sunrise_type(edit_buffer, 1);
   }
   else if(getting_string == NVS_FILTER_CMD) {
      val = (float) atof(edit_buffer);
      set_filter_factor(val);
      request_rcvr_config(3333);
   }
   else if(getting_string == NAV_RATE_CMD) {
      val = (float) atof(edit_buffer);
      set_nav_rate((float) val);
      request_rcvr_config(3334);
   }
   else if(getting_string == DELTA_T_CMD) {
      user_delta_t = (float) atof(edit_buffer);
      user_delta_t /= (24.0*60.0*60.0);  // convert seconds to days
      user_set_delta_t = 1;
      need_sunrise = 1;
   }
   else if(getting_string == PPS_OFS_CMD) {
      i = atoi(edit_buffer);
      set_pps(user_pps_enable, pps_polarity,  delay_value,  (double) i, 300.0, 2);
      request_pps_info();
   }
   else if(getting_string == PPS1_CFG_CMD) {
      i = atoi(edit_buffer);
      edit_pps_freq(0);
      request_pps_info();
   }
   else if(getting_string == PPS2_CFG_CMD) {
      i = atoi(edit_buffer);
      edit_pps_freq(1);
      request_pps_info();
   }
   else {
      if(getting_string >= 0x0100) sprintf(out, "Unknown edit string: 0x%04X", getting_string);
      else                         sprintf(out, "Unknown edit string: %c", getting_string);
      edit_error(out);
   }

   return 1;
}



void kbd_exit()
{
   // user told the program to exit
   break_flag = 1;
   erase_help();
   vidstr(EDIT_ROW+4, EDIT_COL, RED, "Exiting...");
   refresh_page();
}

int sure_exit()
{
   // normal keystroke processor exit
   if(rcvr_type == NO_RCVR) need_redraw = 1;  // erase any menu that was showing
   if(first_key) {    // we were processing a two-character command
      first_key = 0;  // command sequence is done
      prot_menu = 0;
      draw_plot(1);   // replace command sub-menu with the normal plot
   }
   return 0;
}

int help_exit(int c, int reason)
{
   // There was an error in the keyboard command.
   // Exit the keystroke processor with a help message
   reason = 0;       // shut up compiler warning
   script_err = c;
   prot_menu = 0;

   // if reading from a script file,  CR, LF, SPACE, TAB do not bring up help menu
   if(script_file) {   // script file is active
      if((c == 0x0D) || (c == 0x0A)) {  // end-of line reached
         script_pause = 0;
         return sure_exit();
      }
      else if(c == ' ') return sure_exit();
      else if(c == '\t') return sure_exit();
      else c = '?';
   }
   else c = '?';

   if(are_you_sure(c) != c) return 0;
   else if(script_file) return 0;

   return sure_exit();
}

int cmd_error(int c)
{
   return are_you_sure(c);
}

u08 toggle_value(u08 x)
{
int c;

   // commands that normally toggle the parameter can be forced on or off
   // by following them with a 0,1,y,n when they are in script files
   // i.e.  gu  - toggles graph update mode
   //       gu1 - sets graph update mode
   //       gun - clears graph update mode
   if(script_file && (script_pause == 0)) {
      c = get_script();
      if     ((c == '1') || (c == 'y') || (c == 'Y')) return 1;
      else if((c == '0') || (c == 'n') || (c == 'N')) return 0;
   }

   return (x & 1) ^ 1;
}

void share_it()
{
if(0) {
   if(plot_azel == 0)   {
      plot_azel = AZEL_OK;
      shared_plot = 1;
   }
   else if(shared_plot) {
      plot_azel = 0;
      shared_plot = 0;
   }
   else shared_plot = 1;
   if(WIDE_SCREEN && (plot_watch == 0)) shared_plot = 0;  // no need to share plot on wide screens
   //shared_plot = 1;
}
   shared_plot = toggle_value(shared_plot);
if(rcvr_type == NO_RCVR) shared_plot = 0;
   first_key = 0;
   prot_menu = 0;
   config_screen(106);
}

void edit_loc_format()
{
   // process the location formatting commands

   getting_plot = (-1);
   first_key = '{';
   prot_menu = 0;

   if(text_mode) erase_screen();
   else          erase_help();

   // display prompt string for the command
   sprintf(out, " Location format:  D)ecimal   dmS)   G)rads   R)adians  mI)ls   (ESC to abort)");
   vidstr(EDIT_ROW+0, EDIT_COL, PROMPT_COLOR, out);
   sprintf(out, "                   H)am       U)TM   N)ATO    E)CEF");
   vidstr(EDIT_ROW+1, EDIT_COL, PROMPT_COLOR, out);
   sprintf(out, "                   F)eet   M)eters   P)rivate");
   vidstr(EDIT_ROW+2, EDIT_COL, PROMPT_COLOR, out);

   refresh_page();
}

void edit_plot(int id, int c)
{
char *s;
char *a;

   // process the plot control commands

   last_selected_plot = selected_plot;
   selected_plot = id;
   #ifdef FFT_STUFF
      if(selected_plot != FFT) live_fft = selected_plot;
   #endif
   getting_plot = c;
   first_key = '{';
   prot_menu = 0;
   set_steps();

   if(text_mode) erase_screen();
   else          erase_help();

   // display prompt string for the command
   if(plot[selected_plot].show_plot) s = "hide";
   else                              s = "show";
   if(auto_scale) a = "disable";
   else           a = "enable";
   sprintf(out, " %4s plot: A)utoscale %s all plots   X) toggle %s plot autoscale modes",
                 plot[selected_plot].plot_id, a, plot[selected_plot].plot_id);
   vidstr(EDIT_ROW+0, EDIT_COL, PROMPT_COLOR, out);

   #ifdef FFT_STUFF
      if(selected_plot == FFT) sprintf(out, "            C)olor   I)nvert   S)cale factor   trend L)ine");
      else if(luxor)           sprintf(out, "            C)olor   I)nvert   S)cale factor   trend L)ine   &)Cal");
      else                     sprintf(out, "            C)olor   F)FT   I)nvert   S)cale factor   trend L)ine");
   #else                 
      if(luxor) sprintf(out, "            C)olor   I)nvert   S)cale factor   trend L)ine   &)Cal");
      else      sprintf(out, "            C)olor   I)nvert   S)cale factor   trend L)ine");
   #endif
   vidstr(EDIT_ROW+1, EDIT_COL, PROMPT_COLOR, out);

   sprintf(out, "            Z)ero ref   /)statistics  =)remove drift   %c or <cr>=%s", c, s);
   vidstr(EDIT_ROW+2, EDIT_COL, PROMPT_COLOR, out);

   refresh_page();
}



void set_steps()
{
   scale_step = plot[selected_plot].scale_factor*0.01F;
   center_step = 0.0F;
   if(plot[selected_plot].ref_scale) {
      center_step = plot[selected_plot].scale_factor/(plot[selected_plot].ref_scale*10.0F);
   }
}

void move_plot_up()
{
   plot[selected_plot].user_scale = 1;
   plot[selected_plot].float_center = 0;
   plot[selected_plot].plot_center -= center_step;
}

void move_plot_down()
{
   plot[selected_plot].user_scale = 1;
   plot[selected_plot].float_center = 0;
   plot[selected_plot].plot_center += center_step;
}

void shrink_plot()
{
   plot[selected_plot].user_scale = 1;
   plot[selected_plot].float_center = 0;
   plot[selected_plot].scale_factor += scale_step;
}

void grow_plot()
{
   plot[selected_plot].user_scale = 1;
   plot[selected_plot].float_center = 0;
   plot[selected_plot].scale_factor -= scale_step;
}

void toggle_plot(int id)
{
   if((id < 0) || (id >= (NUM_PLOTS+DERIVED_PLOTS))) return;

   selected_plot = id;
   plot[selected_plot].show_plot = toggle_value(plot[selected_plot].show_plot);
   if((graph_lla == 0) && (selected_plot == THREE) && (luxor == 0)) {  //!!!! debug_plots
      if(plot[DAC].sum_yy && stat_count) {
          d_scale = sqrt(plot[TEMP].sum_yy/stat_count) / sqrt(plot[DAC].sum_yy/stat_count);
      }
      else d_scale = 0.0;
if(title_type != USER) {
   sprintf(plot_title, "DAC_scale=%f", d_scale);
   title_type = OTHER;
}
   }

   config_extra_plots();
   if(luxor && (id == AUXV)) need_redraw = 1301;
}

void show_trend_info(int plot_id)
{
   if(plot[plot_id].show_trend && (title_type != USER)) {
      sprintf(plot_title, "%s trend line: %s = (%g*t) + %g   avg_delta=%g", 
                          plot[plot_id].plot_id,
                          plot[plot_id].plot_id,
                          plot[plot_id].a1,
                          plot[plot_id].a0,
                          (plot[plot_id].sum_change/plot[plot_id].stat_count)/view_interval
      );
      title_type = OTHER;
   }
}

int change_plot_param(int c, int cmd_line)
{
    // this routine acts upon the plot control command
    if(c < 0x0100) c = tolower(c);

    if(getting_plot == (-1)) {  // changing location mode
       if(c == 'd') {
          dms = DECIMAL_FMT;
          need_redraw = 1204;
       }
       else if(c == 'e') {
          dms = ECEF_FMT;
          need_redraw = 1204;
       }
       else if(c == 'f') {
          alt_scale = "ft";
          LLA_SPAN = 10.0;        // lla plot scale in feet per division
          ANGLE_SCALE = 2.74e-6;  // degrees per foot
          angle_units = "ft";
          rebuild_lla_plot(0);
          need_redraw = 1201;
       }
       else if(c == 'g') {   // Grads
          dms = GRAD_FMT;
          need_redraw = 1208;
       }
       else if(c == 'h') {   // Ham radio Maidenhead grid square
          dms = GRIDSQ_FMT;
          need_redraw = 1202;
       }
       else if(c == 'i') {   // Mils (6400 per cicrle)
          dms = MIL_FMT;
          need_redraw = 1202;
       }
       else if(c == 'm') {
          alt_scale = "m";
          LLA_SPAN = 3.0;
          ANGLE_SCALE = ((2.74e-6)*FEET_PER_METER); // degrees per meter
          angle_units = "m";
          rebuild_lla_plot(0);
          need_redraw = 1201;
       }
       else if(c == 'n') {   // NATO (MGRS)
          dms = NATO_FMT;
          need_redraw = 1203;
       }
       else if(c == 'p') {
          plot_loc = toggle_value(plot_loc);
          if(0 && (plot_loc == 0)) {  // lat/lon/alt are private
             plot[ONE].show_plot = 0;
             plot[TWO].show_plot = 0;
             plot[THREE].show_plot = 0;
          }
       }
       else if(c == 'r') {   // radians
          dms = RADIAN_FMT;
          need_redraw = 1203;
       }
       else if(c == 's') {   // degrees minutes seconds
          dms = DMS_FMT;
          need_redraw = 1204;
       }
       else if(c == 'u') {   // UTM
          dms = UTM_FMT;
          need_redraw = 1205;
       }
       else if((c == ESC_CHAR) || (c == ' ')) {
       }
       else if(cmd_line) return 1;
       else {
          getting_plot = 0;
          sprintf(out, "Invalid location display command: %c", c);
          edit_error(out);
       }
       if(cmd_line) return 0;
    }
    else if((c == 0x0D) || (c == getting_plot)) {  // toggle plot
//!!!!!if(selected_plot == FFT) plot[selected_plot].show_plot = 1;
       toggle_plot(selected_plot);
    }
    else if(c == 'a') {  // autoscale (all plots)
       auto_scale = toggle_value(auto_scale);
       auto_center = auto_scale;
    }
    else if(c == 'c') {  // graph color
       getting_plot = 0;
       edit_buffer[0] = 0;
       sprintf(out, "Enter color for %s graph (0-15):", plot[selected_plot].plot_id);
       start_edit('g', out);
       return 1;
    }
#ifdef FFT_STUFF
    else if(c == 'f') {
       if(selected_plot != FFT) calc_fft(selected_plot);
    }
#endif
    else if(c == 'g') {  // g acts like an ignore
    }
    else if(c == 'i') {  // invert plot
       plot[selected_plot].invert_plot *= (-1.0F);
    }
    else if(c == 'l') {  // trend line
       plot[selected_plot].show_trend = toggle_value(plot[selected_plot].show_trend);
       if(plot[selected_plot].show_trend) {
          show_trend_info(selected_plot);
       }
       else if(title_type == OTHER) {
          plot_title[0] = 0;
          title_type = NONE;
       }
    }
    else if(c == '/') {
       getting_plot = 0;
       all_plots = 0;
       edit_buffer[0] = 0;
       edit_info1 =    "                            miN)   maX)   sP)an";
       start_edit('~', "Enter statistic to display: A)vg   R)ms   S)td dev   V)ar   <cr>=hide");
       return 1;
    }
    else if((c == 'm') || (c == 's') || (c == 'f')) {  // modify scale factor
       getting_plot = 0;
       edit_scale();
       return 1;
    }
    else if(c == 'x') {  // toggle scale modes
       plot[selected_plot].user_scale = toggle_value(plot[selected_plot].user_scale);
       plot[selected_plot].float_center = toggle_value(plot[selected_plot].float_center);
    }
    else if((c == 'z') || (c == 'r')) {  // zero reference line
       getting_plot = 0;
       edit_ref();
       return 1;
    }
    else if(c == '+') {  // move graph up
       move_plot_up();
    }
    else if(c == '-') {  // move graph down
       move_plot_down();
    }
    else if(c == '{') {  // make plot smaller
       shrink_plot();
    }
    else if(c == '}') {  // make plot bigger
       grow_plot();
    }
    else if(c == '=') {  // remove dift rate
       getting_plot = 0;
       edit_drift();
       return 1;
    }
//  else if((c == '&') && luxor && cal_mode) {
    else if((c == '&') && luxor) {
       getting_plot = 0;
       edit_cal();
       return 1;
    }
    else if((c == ESC_CHAR) || (c == ' ')) {
    }
    else {
       getting_plot = 0;
       sprintf(out, "Invalid plot command: %c", c);
       edit_error(out);
    }

    refresh_page();
    getting_plot = 0;
    return 0;
}


void kbd_marker(int c)
{
long val;

   // set a plot marker from the keyboard
   last_was_mark = c;
   c -= '0';
   if(mark_q_entry[c]) {  // marker was set,  go to it
      goto_mark(c);
   }
   else {  // mark current spot if it is not already marked
      for(val=1; val<MAX_MARKER; val++) {
         if(mark_q_entry[val] == last_mouse_q) {  // place is already marked
            BEEP(310);
            return;
         }
      }
      mark_q_entry[c] = last_mouse_q;
      draw_plot(1);
   }
}

int set_next_marker()
{
int val;

   // mark current mouse column with the next available marker number
   for(val=1; val<MAX_MARKER; val++) {
      if(mark_q_entry[val] == last_mouse_q) {  // place is already marked
         mark_q_entry[val] = 0;
         draw_plot(1);
         return 1;
      }
   }
   for(val=1; val<MAX_MARKER; val++) {
      if(mark_q_entry[val] == 0) {
         last_was_mark = '0'+val;
         mark_q_entry[val] = last_mouse_q;  // last marker is mouse click marker
         draw_plot(1);
         return 1;
      }
   }

   BEEP(311);
   return 0;
}

void change_fix_config(int save)
{
static int old_watch = 0;
static int old_azel = 0;
static int old_share = 0;
static int old_sigs = 0;
static int old_lla = 0;
static int saved_config = 0;

   if(save) {
      old_watch = plot_watch;
      old_azel = plot_azel;
      old_share = shared_plot;
      old_sigs = plot_signals;
      old_lla = plot_lla;
      saved_config = 1;
   }
   else if(saved_config) {
      plot_watch = old_watch;
      plot_azel = old_azel;
      shared_plot = old_share;
      plot_signals = old_sigs;
      if(show_fixes == 0) plot_lla = 0; // !!!!! old_lla;
      else plot_lla = old_lla;

      no_redraw = 1;
      config_screen(130);
      no_redraw = 0;
   }
}


void change_zoom_config(int save)
{
static int old_watch = 0;
static int old_azel = 0;
static int old_share = 0;
static int old_sigs = 0;
static int old_lla = 0;
static int saved_config = 0;

// return;
   if(save) {
      old_watch = plot_watch;
      old_azel = plot_azel;
      old_share = shared_plot;
      old_sigs = plot_signals;
      old_lla = plot_lla;
      saved_config = 1;
   }
   else if(saved_config) {
      plot_watch = old_watch;
      plot_azel = old_azel;
      shared_plot = old_share;
      plot_signals = old_sigs;
      if(show_fixes == 0) plot_lla = 0; // !!!!! old_lla;
      else plot_lla = old_lla;
   }
}


void do_fixes(int mode)
{
   if(rcvr_type == NO_RCVR) return;

   show_fixes = toggle_value(show_fixes);
   change_fix_config(1);

   if(show_fixes && (precision_survey == 0)) {  // set reference position for fix map
      precise_lat = lat;
      precise_lon = lon;
      precise_alt = alt;
      ref_lat = lat;
      ref_lon = lon;
      ref_alt = alt;
      cos_factor = cos(ref_lat);
   }
   
   if(show_fixes && (mode >= 0) && (mode <= 7)) {  // 3d fix mode
      start_3d_fixes(mode, 1);
      if(GRAPH_LLA && (graph_lla == 0)) {
         config_lla_plots(0, 0);
      }
   }
   else {  // normal overdetermined clock mode
      set_rcvr_mode(RCVR_MODE_HOLD);   
      plot_lla = 0;
      if(GRAPH_LLA && graph_lla && (keep_lla_plots == 0)) {
         config_normal_plots();
      }
   }

   request_rcvr_config(10);
   first_key = 0;
   prot_menu = 0;
   config_screen(107);
}


void dump_image()
{
   dump_type = 's';
   #ifdef GIF_FILES
      sprintf(edit_buffer, "%s.gif", unit_file_name);
      if(top_line) start_edit('w', "Enter name of .GIF or .BMP file for GRAPH image (ESC ESC to abort):");
      else         start_edit('w', "Enter name of .GIF or .BMP file for SCREEN image (ESC ESC to abort):");
   #else
      sprintf(edit_buffer, "%s.bmp", unit_file_name);
      if(top_line) start_edit('w', "Enter name of .BMP file for GRAPH image (ESC ESC to abort):");
      else         start_edit('w', "Enter name of .BMP file for SCREEN image (ESC ESC to abort):");
   #endif
}

#define HIST_BINS  40000  // 400000            // should be a multiple of 4000 (+/- 2000 msecs)
#define HIST_SCALE (HIST_BINS/4000)
int hist[HIST_BINS+1];

void set_jitter_config()
{
int i;
int ndx;
int max_bin;
double max_val;
double val;
double peak_div;
double avg;
double sdev;
double sum_y, sum_yy;
struct PLOT_Q q;

   // set up the machine to measure and display receiver message timing

   if(measure_jitter) {  // jitter measurements just enabled
      old_idle = idle_sleep;      // save current settings
      old_jit_adev = jitter_adev;
      old_poll = no_poll;

      old_plot_watch = plot_watch;
      old_plot_azel = plot_azel;
      for(i=0; i<NUM_PLOTS; i++) {  // disable all plots
         old_show_plots[i] = plot[i].show_plot;
         plot[i].show_plot = 0;
      }

      plot[MSGJIT].show_plot = 1;
      plot[MSGOFS].show_plot = 1;
      plot_watch = 0;
      plot_azel = 0;
                                  // set optimum values for jitter measurement
      idle_sleep = 0;
      jitter_adev = 1;
//    no_poll = 1;                // rrrrr messes up things like SCPI that must be polled
                                  // flush the queues
      new_queue(3);
   }
   else {  // jitter measurements disabled, restore old settings, calculate histogram
      idle_sleep = old_idle;
      jitter_adev = old_jit_adev;
      no_poll = old_poll;

      plot_watch = old_plot_watch;
      plot_azel = old_plot_azel;
      for(i=0; i<NUM_PLOTS; i++) {  // disable all plots
         plot[i].show_plot = old_show_plots[i];
      }


      // calculate histogram of message arrival times (+/- 2000 msecs, 0.1 msec res
      if(plot_q_count == 0) return;
      if(hist_file) {
         fclose(hist_file);
         hist_file = 0;
      }
      sprintf(out, "%s.jit", unit_file_name);
      hist_file = topen(out, "w");
      if(hist_file == 0) return;

      for(i=0; i<HIST_BINS; i++) hist[i] = 0;

      avg = 0.0;
      sum_y = sum_yy = 0.0;
      for(i=0; i<plot_q_count; i++) {  // calculate histogram of message offset times
         q = plot_q[i];
         if(queue_interval) val = (q.data[MSGOFS] / (double) queue_interval);
         else val = 0.0;

         sum_y += val;
         sum_yy += (val * val);

         ndx = (int) (val*HIST_SCALE+0.50);
         ndx += (HIST_BINS/2);
         if((ndx < 0) || (ndx >= HIST_BINS)) continue;
         ++hist[ndx];
      }
      if(plot_q_count) avg = sum_y / (double) plot_q_count;
      sdev = sqrt(fabs((sum_yy / (double) plot_q_count) - (avg*avg)));

      
fprintf(hist_file, "# bin   count\n");
      max_bin = 0;
      max_val = 0.0;
      peak_div = 1.0;
      for(i=0; i<HIST_BINS; i++) {  // write histogram data
         if(HIST_SCALE) val = (double) (i - (HIST_BINS/2)) / (double) HIST_SCALE;
         else val = 0.0;
         if(hist[i]) {
            fprintf(hist_file, "%.2f %d\n", val, hist[i]);
            if(hist[i] > max_bin) {  // a new peak
               max_bin = hist[i];
               max_val = val;
               peak_div = 1.0;
            }
            else if(hist[i] == max_bin) {  // broad peak, average the values
               max_val += val;
               ++peak_div;
            }
         }
      }

      sprintf(plot_title, "# msg offset time: /tsx=%.2f msec   max hits:%d points:%ld   avg:%.2f sdev:%.2f", max_val/peak_div,  max_bin, plot_q_count, avg,sdev);
      fprintf(hist_file,  "# msg offset time: /tsx=%.2f msec   max hits:%d points:%ld   avg:%.2f sdev:%.2f", max_val/peak_div,  max_bin, plot_q_count, avg,sdev);
      fclose(hist_file);
      hist_file = 0;
   }
}

void clear_all_data()
{
   new_queue(3);
   #ifdef BUFFER_LLA
      clear_lla_points();
   #endif
   #ifdef SIG_LEVELS
      clear_signals();
   #endif
   #ifdef SAT_TRAILS
      clear_sat_trails();
   #endif
   need_redraw = 5566;
}


void config_fix_display()
{
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
   if(show_fixes) start_3d_fixes(-1, 2);
   if(1 || (GRAPH_LLA && (graph_lla == 0))) {
      config_lla_plots(1, 0);
   }

   config_screen(112);
   user_fix_set = 0;
}


int kbd_a_to_o(int c)
{
u32 val;

   if(force_si_cmd) {     // !!!!! royal kludge for G C T keyboard command
      force_si_cmd = 0;
      goto si_cmd;
   }

   // Process keyboard characters a..o
   if(c == 'a') {
      if(first_key == 'c') {  // CA command - clear adev queues
         if(luxor) return help_exit(c,99);
         new_queue(1);
      }
      else if(first_key == 'f') {  // FA command - toggle ALT filter
         if(luxor) return help_exit(c,99);
         alt_filter = toggle_value(alt_filter);
         set_filter_config(pv_filter, static_filter, alt_filter, kalman_filter, marine_filter, 1);
      }
      else if(first_key == 'l') {  // LA command - open log in append mode
         if(log_file) edit_error("Log file is already open.");
         else {
            log_mode = "a";
            strcpy(edit_buffer, log_name);
            edit_info1 = "Use file extension .gpx .xml or .kml to write XML format logs.";
            start_edit('n', "Enter log file name to append data to: ");
            return 0;
         }
      }
      else if(first_key == 'p') {  // PA command - set PPS1 params
         if(luxor) return help_exit(c,99);
         if(rcvr_type == NVS_RCVR) {
            sprintf(edit_buffer, "%d", nvs_pps_width);
            start_edit(PPS1_CFG_CMD, "Enter PPS [duty_cycle <1] or [nanoseconds 50..2520500]  (ESC ESC to abort):");
         }
         else if(rcvr_type == VENUS_RCVR) {
            if((rcvr_type == VENUS_RCVR) && saw_timing_msg) return help_exit(c,99);
            if(!have_pps_freq) edit_info1 = "This receviver can only adjust the pulse width.";
            sprintf(edit_buffer, "%.0lf %.6lf", pps1_freq, pps1_duty);
            start_edit(PPS1_CFG_CMD, "Enter PPS1 [frequency]  [duty_cycle <1 or microseconds 1..100000]  (ESC ESC to abort):");
         }
         else {
            sprintf(edit_buffer, "%.0lf %.6lf", pps1_freq, pps1_duty);
            start_edit(PPS1_CFG_CMD, "Enter PPS1 [frequency]  [duty_cycle <1 or microseconds >=1]  (ESC ESC to abort):");
         }
         return 0;
      }
      else if(first_key == 's') {  // SA command - antenna signal level map
         if(luxor) return help_exit(c,99);
         edit_buffer[0] = 0;
         start_edit('Q', "A)zimuth  W)eighted azimuth  E)levation  S)ignals  D)ata  C)lear  <cr>=off");
if(plot_watch && plot_azel) {
   shared_plot = 1;
   prot_menu = 0;
}
         return 0;
      }
      else if(first_key == 't') { // TA command - set alarm clock time
         if     (script_file) edit_buffer[0] = 0;
         else if(alarm_time && alarm_date) sprintf(edit_buffer, "%02d:%02d:%02d  %02d/%02d/%04d", alarm_hh,alarm_mm,alarm_ss, alarm_month,alarm_day,alarm_year); 
         else if(alarm_time) sprintf(edit_buffer, "%02d:%02d:%02d", alarm_hh,alarm_mm,alarm_ss);
         else if(alarm_date) sprintf(edit_buffer, "%02d/%02d/%04d", alarm_month,alarm_day,alarm_year);
         else if(egg_val) {
            if((egg_val >= (24L*60L*60L)) && ((egg_val % (24L*60L*60L)) == 0)) sprintf(edit_buffer, "%ld d", egg_val/(24L*3600L));
            else if((egg_val >= (3600L)) && ((egg_val % 3600L) == 0)) sprintf(edit_buffer, "%ld h", egg_val/(3600L));
            else if((egg_val >= (60L)) && ((egg_val % 60L) == 0)) sprintf(edit_buffer, "%ld m", egg_val/(60L));
            else sprintf(edit_buffer, "%ld s", egg_val);
            if(repeat_egg) strcat(edit_buffer, " R");
         }
         else edit_buffer[0] = 0;

         edit_info1 = "Dates are in the format mm/dd/yyyy or yyyy/mm/dd";
         edit_info2 = "Intervals can be in seconds, minutes, hours or days like: 7s, 10m, 2h, 4d";
         if(edit_buffer[0]) start_edit(':', "Enter alarm clock time (and optional date) or interval or <ESC CR> to reset:");
         else               start_edit(':', "Enter alarm clock time (and optional date) or interval or <CR> to reset:");
         return 0;
      }
      else if(first_key == 'w') {  // WA command - Output all data to a log file
         strcpy(edit_buffer, "dump.log");
         sprintf(out, "Enter name of log file to write ALL %squeue info to (ESC ESC to abort):",
                       filter_log?"filtered ":"");
         start_edit('w', out);
         dump_type = 'a';
         log_mode = "w";
         return 0;
      }
      else if(first_key == 'z') {  // ZA command - zoom antenna aziumuth map
         if(rcvr_type == NO_RCVR) return sure_exit();
         change_zoom_config(6);
         zoom_screen = 'S';
         zoom_fixes = show_fixes;
         plot_signals = 2;
         config_screen(120);
      }
      else if(first_key == '&') {  // &A command - autotune osc params
         if(luxor) return help_exit(c,99);
         dac_dac = 1;
      }
      #ifdef ADEV_STUFF
         else if(first_key == 'a') {  // AA command - set adev type to ADEV
            if(luxor) return help_exit(c,99);
            ATYPE = OSC_ADEV;
            all_adevs = 0;
            force_adev_redraw();
            config_screen(108);
         }
         else if(first_key == 'g') { 
            if(luxor) {  // GA command - toggle ADEV plot
               show_prots = toggle_value(show_prots);
            }
            else {  // GA command - toggle ADEV plot
               plot_adev_data = toggle_value(plot_adev_data);
               draw_plot(1);
            }
         }
         else if(luxor) {  // A commands
            return help_exit(c,1);
         }
         else if(are_you_sure(c) != c) return 0;
      #else
         else return help_exit(c,1);
      #endif
   }
   else if(c == 'b') {
      if(first_key == 'c') {  // CB command - clear both queues
//!!!!-  pause_data = 0;
         if(luxor) return help_exit(c,99);
         new_queue(3);
      }
      else if(first_key == 'g') {  // GB command - satellite map, shared with plot area
         share_it();
      }
      else if(first_key == 'p') {  // PB command - set PPS2 params
         if(luxor) return help_exit(c,99);
         if(rcvr_type == VENUS_RCVR) {
            if(saw_timing_msg == 0) return help_exit(c,99);
            sprintf(edit_buffer, "%.0lf", pps2_freq);
            start_edit(PPS2_CFG_CMD, "Enter PPS2 frequency (max 19.2 MHz)  (ESC ESC to abort):");
         }
         else if(rcvr_type == UBX_RCVR) {
            sprintf(edit_buffer, "%.0lf %.3lf", pps2_freq, pps2_duty);
            edit_info1 = "PPS2 is limited to 1000 Hz on older model receivers";
            start_edit(PPS2_CFG_CMD, "Enter PPS2 [frequency]  [duty_cycle <1 or microseconds >=1]  (ESC ESC to abort):");
         }
         else return help_exit(c,99);
         return 0;
      }
      else if(first_key == '!') {    // !B command - send break command to the receiver
         SetDtrLine(1);
         SendBreak();
         redraw_screen();
      }
      else if(first_key == 'z') {  // ZB command - zoom watch and map
         change_zoom_config(2);
         zoom_screen = 'B';
         zoom_fixes = show_fixes;
if(rcvr_type == NO_RCVR) map_and_watch = 1;
         config_screen(109);
      }
      else if(first_key == 0) {
         if(luxor) {
            if(are_you_sure(c) != c) return 0;
         }
         else if(rcvr_type == TSIP_RCVR) {
            sprintf(edit_buffer, "K %f %f %f %f", OSC_P_GAIN, OSC_D_TC, OSC_FILTER_TC, OSC_I_TC);
            edit_info1    = "Custom OSC PID: K [proportional_gain] [derivative_tc] [filter_tc] [integrator_tc]";
            start_edit('b', "OSC CTRL PID:   W)slow  N)medium  X)fast  Y)very fast  0)off  1)on");
            return 0;
         }
         else return help_exit(c,2);
      }
      else return help_exit(c,2);
   }
   else if(c == 'c') {
      if(luxor && (first_key == 'b')) {  // BC command - constant load current mode
         sprintf(edit_buffer, "%f", 0.1F);
         start_edit(BC_CMD, "Enter desired load current in amps.  (ESC ESC to abort):"); 
         return 0;
      }
      else if(first_key == 'c') {  // CC command - clear everything
//!!!!-  pause_data = 0;
         clear_all_data();
      }
      else if(first_key == 'f') {  // FC command - coordinate filter
         sprintf(edit_buffer, "%.2f", nvs_filter);
         start_edit(NVS_FILTER_CMD, "Enter solution filtration factor (0-10)   (ESC ESC to abort):"); 
         return 0;
      }
      else if(first_key == 'g') { 
         if(luxor) {  // GC command - show battery capacity
            show_mah  = toggle_value(show_mah);
            debug_text[0] = 0;
            redraw_screen();
redraw_screen();  // gggggg
         }
         else {       // GC command - satellite constellation displays
            strcpy(edit_buffer, "P");
            edit_info2 =         "Count:   G)raph    T)able length";
            edit_info1 =         "         Include a '+' to force ascending sort or '-' for descending";
            if(have_doppler) start_edit(SORT_CMD, "Sort by: A)zimuth  D)oppler  E)levation  P)RN  S)ignal  (ESC ESC to abort)");
            else             start_edit(SORT_CMD, "Sort by: A)zimuth  E)levation  P)RN  S)ignal  (ESC ESC to abort)");
            return 0;
         }
      }
      else if(first_key == 'l') {  // LC command - toggle logging of sat constellation data
         if(luxor) return help_exit(c,99);
         log_db ^= 1;
      }
      else if(luxor && (first_key == 'p')) {  // PC command = load overcurrent
         sprintf(edit_buffer, "%f", batt_ovc);
         start_edit(PC_CMD, "Enter the battery overcurrent cutoff in amps.  (ESC ESC to abort):");
         return 0;
      }
      else if(first_key == 'p') {  // PC command - set cable delay
         edit_buffer[0] = 0;
         edit_info1 = "Example: enter 50f for 50 feet of 0.66 velocity factor coax";
         if(rcvr_type == STAR_RCVR) {
            edit_info2 = "Note: MUST use NEGATIVE values to compensate for cable delay!";
         }
         else if(rcvr_type == TSIP_RCVR) {
            edit_info2 = "Note: use NEGATIVE values to compensate for cable delay!";
         }
         start_edit('9', "Enter cable delay in ns or use #feet or #meters (ESC ESC to abort):");
         return 0;
      }
      else if(luxor && (first_key == 'w')) {  // WC command = write config data
         sprintf(edit_buffer, "LUXCFG.SCR");
         start_edit(WC_CMD, "Enter config data script file to write (ESC ESC to abort)");
         return 0;
      }
      else if(first_key == 'z') {  // ZC command - zoom clock
         change_zoom_config(3);
         zoom_screen = 'C';
         zoom_fixes = show_fixes;
         config_screen(110);
      }
      else if(first_key == '$') {  // $c command - custom screen
         new_screen(c);
         return 0;
      }
      else if(first_key == '&') { 
         if(luxor) { // &c command - toggle cal mode
            cal_mode = toggle_value(cal_mode);
            if(cal_mode) set_cal_mode(0x01);
            else         set_cal_mode(0x00);
            Sleep(100);
         }
      }
      else if(first_key == '!') {    // !C command - cold start reset
         request_cold_reset();
         redraw_screen();
      }
      else if(first_key == 0) {  // C command first char
         if(are_you_sure(c) != c) return 0;
      }
      else return help_exit(c,99);
   }
   else if(c == 'd') {  
      if(first_key == 'd') { // DD command - disable osc discipline
         if(luxor) return help_exit(c,99);
         osc_discipline = 0;
         set_discipline_mode(4);
         need_redraw = 1206;
      }
      else if(first_key == 'g') { // GD command - toggle DAC graph
         edit_plot(DAC, c);
         return 0;
      }
      else if(first_key == 'f') { // FD command - toggle display filter
         if(filter_count) strcpy(edit_buffer, "0");
         else             strcpy(edit_buffer, "10");
         start_edit('f', "Enter number of plot queue points to average (ESC ESC to abort):");
         return 0;
      }
      else if(first_key == 'l') { // LD command - delete file
         strcpy(edit_buffer, log_name);
         goto delete_file;
      }
      else if(first_key == 'p') {   // PD command - disable PPS signal
         user_pps_enable = 0;
         set_pps(user_pps_enable, pps_polarity,  delay_value, pps1_delay,  300.0, 4);
         request_pps_info();
      }
      else if(first_key == 't') { // TD command - set screen dump time
         if     (script_file) edit_buffer[0] = 0;
         else if(dump_time && dump_date) sprintf(edit_buffer, "%02d:%02d:%02d  %02d/%02d/%04d", dump_hh,dump_mm,dump_ss, dump_month,dump_day,dump_year); 
         else if(dump_time) sprintf(edit_buffer, "%02d:%02d:%02d", dump_hh,dump_mm,dump_ss);
         else if(dump_date) sprintf(edit_buffer, "%02d/%02d/%04d", dump_month,dump_day,dump_year);
         else if(dump_val) {
            if((dump_val >= (24L*60L*60L)) && ((dump_val % (24L*60L*60L)) == 0)) sprintf(edit_buffer, "%ld d", dump_val/(24L*3600L));
            else if((dump_val >= (3600L)) && ((dump_val % 3600L) == 0)) sprintf(edit_buffer, "%ld h", dump_val/(3600L));
            else if((dump_val >= (60L)) && ((dump_val % 60L) == 0)) sprintf(edit_buffer, "%ld m", dump_val/(60L));
            else sprintf(edit_buffer, "%ld s", dump_val);
         }
         else edit_buffer[0] = 0;

         edit_info1 = "Dates are in the format mm/dd/yyyy or yyyy/mm/dd";
         edit_info2 = "Intervals can be in seconds, minutes, hours or days like: 7s, 10m, 2h, 4d";
         if(edit_buffer[0]) start_edit('!', "Enter screen dump time (and optional date) or interval or <ESC CR> to reset:");
         else               start_edit('!', "Enter screen dump time (and optional date) or interval or <CR> to reset:");
         return 0;
      }
      else if(first_key == 'w') { // WD command - delete file
         edit_buffer[0] = 0;
         delete_file:
         start_edit('w', "Enter name of file to delete (ESC to abort):");
         dump_type = 'd';
         return 0;
      }
      else if(first_key == 'z') {  // ZD command - zoom antenna data map
         if(rcvr_type == NO_RCVR) return sure_exit();
         change_zoom_config(6);
         zoom_screen = 'S';
         zoom_fixes = show_fixes;
         plot_signals = 5;
         config_screen(120);
      }
      else if(first_key == '&') { // &D command - change damping factor
         if(luxor) return help_exit(c,99);
         sprintf(edit_buffer, "%f", user_damping_factor);
         start_edit('2', "Enter damping factor (ESC ESC to abort):");
         return 0;
      }
      else if(first_key == '!') {    // !D command - do receiver diagnostice
         request_self_tests();
         redraw_screen();
      }
      else {   // D command - toggle oscillator disciplining
         if(luxor) return help_exit(c, 99);
         if(are_you_sure(c) != c) return 0;
         // next call to do_kbd() will process the discipline selection character
      }
   }
   else if(c == 'e') {  
      if(first_key == 'd') { // DE command - enable osc discipline
         if(luxor) return help_exit(c,99);
         osc_discipline = 1;
         set_discipline_mode(5);
         need_redraw = 1207;
      }
      else if(first_key == 'e') { // EE command - save eeprom segments
         if(luxor) return help_exit(c,99);
         if(rcvr_type == VENUS_RCVR) {
            eeprom_save = toggle_value(eeprom_save);
         }
         else {
            save_segment(0xFF, 1);
         }
      }
      else if(first_key == 'f') {  // FE command - elevation mask
         if(luxor) return help_exit(c,99);
         sprintf(edit_buffer, "%.1f", el_mask);
         start_edit('e',  "Enter minimum acceptible satellite elevation (in degrees,  (ESC CR=best)");
         return 0;
      }
      else if(first_key == 'g') {  // GE command - toggle error graph
         plot_skip_data = toggle_value(plot_skip_data);
         draw_plot(1);
      }
      else if(first_key == 'h') {  // HE command - enter holdover
         set_discipline_mode(2);
      }
      else if(first_key == 'p') {   // PE command - enable PPS signal
         user_pps_enable = 1;
         set_pps(user_pps_enable, pps_polarity,  delay_value, pps1_delay,  300.0, 4);
         request_pps_info();
      }
      else if(first_key == 't') {  // TE command - set utc-ut delta T
         sprintf(edit_buffer, "%f", utc_delta_t()*(24.0*60.0*60.0));
         start_edit(DELTA_T_CMD,  "Enter TT-UT1 delta-T value in seconds,  (ESC ESC to abort)");
         return 0;
      }
      else if(first_key == 'z') {  // ZE command - zoom elevation map
         if(rcvr_type == NO_RCVR) return sure_exit();
         change_zoom_config(6);
         zoom_screen = 'S';
         zoom_fixes = show_fixes;
         plot_signals = 3;
         config_screen(120);
      }
      else if(luxor && (first_key == '&')) { // &e command - IR emissivity
         sprintf(edit_buffer, "%f", emis1);
         start_edit(AMPE_CMD, "Enter IR1 sensor emissivity (0.1 .. 1.0)  (ESC ESC to abort):"); 
         return 0;
      }
      else if(first_key == 0) {  // E command first char
         if(luxor) return help_exit(c,99);
         if(are_you_sure(c) != c) return 0;
      }
      else return help_exit(c,99);
   }
   else if(c == 'f') {  // filter selection
      if(first_key == '&') { // &F command - max freq offset
         if(luxor) return help_exit(c,99);
         sprintf(edit_buffer, "%f", user_max_freq_offset);
         sprintf(out, "Enter max freq offset in %s (ESC ESC to abort):", ppb_string);
         start_edit('7', out);
         return 0;
      }
      else if(luxor && (first_key == 'b')) {  // BF command - lifepo4 charge mode
         sprintf(edit_buffer, "%f", 0.1F);
         start_edit(BF_CMD, "Enter desired LiFePO4 charge current in amps.  (ESC ESC to abort):"); 
         return 0;
      }
      else if(first_key == 'f') {  // FF command - set foliage filter
         if(luxor) return help_exit(c,99);
         if     (foliage_mode == 0) strcpy(edit_buffer, "N");
         else if(foliage_mode == 2) strcpy(edit_buffer, "A");
         else                       strcpy(edit_buffer, "S");
         start_edit('o', "Foliage:  A)lways   S)ometimes   N)ever");
         return 0;
      }
      #ifdef FFT_STUFF
         else if(first_key == 'g') {   // GF command - fft plot
            edit_plot(FFT, c);
            return 0;
         }
      #endif
      #ifdef PRECISE_STUFF
         else if(first_key == 's') {  // SF command - show fixes
            if(luxor) return help_exit(c,99);
            do_fixes(0); // 2D/3D mode
         }
      #endif
      else if(first_key == 'p') {  // PF command - falling edge PPS polarity
         if(luxor) return help_exit(c,99);
         user_pps_enable = 1;
         set_pps(user_pps_enable, 1,  delay_value, pps1_delay, 300.0, 3);
         request_pps_info();
      }
      else if(first_key == 'w') {  // WF command - write filtered data to log
         filter_log = toggle_value(filter_log);
      }
      else if(first_key == '$') {  // $f command - full screen
         new_screen(c);
         return 0;
      }
      else {  // F command - toggle filter
         if(are_you_sure(c) != c) return 0;
         // next call to do_kbd() will process the filter selection character
      }
   }
   else if(c == 'g') {  
      if(first_key == 'g') {  // GG command - graph title
         strcpy(edit_buffer, plot_title);
         start_edit('H', "Enter graph description (ESC <cr> to erase):");
         return 0;
      }
      else if(first_key == 's') { // SG command - set GNSS system/systems to use
         edit_buffer[0] = 0;
         if(gnss_mask == 0) strcat(edit_buffer, "A ");
         if(gnss_mask & GPS) strcat(edit_buffer, "G ");
         if(gnss_mask & GLONASS) strcat(edit_buffer, "N ");
         if(gnss_mask & BEIDOU) strcat(edit_buffer, "B ");
         if(gnss_mask & GALILEO) strcat(edit_buffer, "L ");
         if(gnss_mask & SBAS) strcat(edit_buffer, "S ");
         if(gnss_mask & QZSS) strcat(edit_buffer, "Q ");
         if(gnss_mask & IMES) strcat(edit_buffer, "I ");
         edit_info1 = "   A=All  D=default  G=GPS  N=GLONASS  B=Beidou  L=Galileo  S=SBAS  Q=QZSS  I=IMES";
         if(edit_buffer[0]) start_edit(GNSS_CMD, "Enter characters to select GNSS systems to enable (ESC ESC to abort)");
         else               start_edit(GNSS_CMD, "Enter characters to select GNSS systems to enable (ESC to abort)");
         return 0;
      }
      else if(first_key == 't') {  // TG commmand - use GPS time
//       time_zone_set = 0;
         temp_utc_mode = 0;
         set_timing_mode(0x00);
         if(rcvr_type == NVS_RCVR) request_pps_info();
         request_timing_mode();
      }
      else if(first_key == 'w') {  // WG command - plot image dump
         invert_dump = 0;
         top_line = (MOUSE_ROW+2)*TEXT_HEIGHT;
         dump_image();
         return 0;
      }
      else if(first_key == '&') { // &G command - oscillator gain
         if(luxor) return help_exit(c,99);
         sprintf(edit_buffer, "%f", user_osc_gain);
         start_edit('3', "Enter oscillator gain in Hz/v (ESC ESC to abort):");
         return 0;
      }
      else {  // G command - select graph/display option
         if(are_you_sure(c) != c) return 0;  // select a graph to toggle
         // next call to do_kbd() will process the graph selection character
      }
   }
   else if(c == 'h') {  // toggle manual holdover mode
      if(first_key == '&') { // &H command - change max allowale dac voltage
         if(luxor) return help_exit(c,99);
         sprintf(edit_buffer, "%f", user_max_range);
         start_edit('T', "Enter maximum DAC voltage range value (ESC ESC to abort):"); 
         return 0;
      }
      else if(luxor && (first_key == 'b')) {  // BH command - high voltage lipo charge mode
         sprintf(edit_buffer, "%f %f", 0.1F, 4.30F);
         start_edit(BH_CMD, "Enter desired charge current in amps and voltage in volts.  (ESC ESC to abort):"); 
         return 0;
      }
      else if(first_key == 'g') { // GH command - toggle HOLDOVER plot
         if(luxor) return help_exit(c,99);
         plot_holdover_data = toggle_value(plot_holdover_data);
         draw_plot(1);
      }
      else if(first_key == 'h') {  // HH command - toggle holdover mode
         if(luxor) return help_exit(c,99);
         user_holdover = toggle_value(user_holdover);
         if(user_holdover) set_discipline_mode(2);
         else              set_discipline_mode(3);
      }
      else if(luxor && (first_key == 'p')) {  // PH command = battery high voltage cutoff
         sprintf(edit_buffer, "%f", batt_hvc);
         start_edit(PH_CMD, "Enter the battery high voltage cutoff in volts.  (ESC ESC to abort):");
         return 0;
      }
      else if(first_key == 's') {  // SH command - position hold mode
         set_rcvr_mode(RCVR_MODE_HOLD);
      }
      else if(first_key == 't') {  // TH command - set chime mode
         if(cuckoo) sprintf(edit_buffer, "%d", cuckoo);
         else       sprintf(edit_buffer, "4");
         if(singing_clock)    strcat(edit_buffer, "S");
         else if(ships_clock) strcat(edit_buffer, "B");
         if(cuckoo_hours)     strcat(edit_buffer, "H");
         start_edit('h', "Chime/sing at # places per hour (#S=sing  #H=chime out hours  1B=Ship's bells):");
         return 0;
      }
      else if(first_key == '$') {  // $h command - huge screen
         new_screen(c);
         return 0;
      }
      else if(first_key == '!') {    // !H command - hard factory reset
         request_factory_reset();
         redraw_screen();
      }
      #ifdef ADEV_STUFF
         else if(first_key == 'a') {  // AH command - set adev type to HDEV
            if(luxor) return help_exit(c,99);
            ATYPE = OSC_HDEV;
            all_adevs = 0;
            force_adev_redraw();
            config_screen(111);
         }
      #endif
      else if(first_key == 0) {  // H command first char - set holdover mode
         if(luxor) return help_exit(c, 99);
         if(are_you_sure(c) != c) return 0;
      }
      else return help_exit(c,99);
   }
   else if(c == 'i') {  // set queue update interval
      if(first_key == '&') { 
         if(luxor) {         // &i command - IR2 emissivity
            sprintf(edit_buffer, "%f", emis2);
            start_edit(AMPI_CMD, "Enter IR2 sensor emissivity (0.1 .. 1.0)  (ESC ESC to abort):"); 
            return 0;
         }
         else {   // &i command - initial dac voltage
            sprintf(edit_buffer, "%f", user_initial_voltage);
            start_edit('8', "Enter initial DAC voltage (ESC ESC to abort):");
            return 0;
         }
      }
      else if(luxor && (first_key == 'b')) {  // BI command - calc internal resistance
         calc_ir = IR_TIME;
         pause_data = 0;
         need_redraw = 1208;
      }
      else if(first_key == 'f') {  // FI command - toggle Motorola ionosphere filter
         if(luxor) return help_exit(c,99);
         static_filter = toggle_value(static_filter);
         set_filter_config(pv_filter, static_filter, alt_filter, kalman_filter, marine_filter, 1);
      }
      else if(first_key == 'g') { // GI command
         if(luxor) {   // toggle BATTw graph
            edit_plot(ELEVEN, c);
            return 0;
         }
         else if(rcvr_type == NO_RCVR) ;
         else {   // toggle fix map display
            show_fixes = toggle_value(show_fixes);
            config_fix_display();
         }
      }
      else if(first_key == 'l') { // LI command - set log interval
         sprintf(edit_buffer, "%ld", log_interval);
         start_edit('j', "Enter log interval in seconds: ");
         return 0;
      }
      else if(first_key == 'm') { // MI command - invert pps and temperature scale factors
         if(luxor) return help_exit(c,99);
         if(plot[PPS].invert_plot < 1.0F) val = 1;
         else                             val = 0;
         val = toggle_value((u08) val);
         if(val) plot[PPS].invert_plot = (-1.0);
         else    plot[PPS].invert_plot = (1.0);

         if(plot[TEMP].invert_plot < 1.0F) val = 1;
         else                              val = 0;
         val = toggle_value((u08) val);
         if(val) plot[TEMP].invert_plot = (-1.0);
         else    plot[TEMP].invert_plot = (1.0);
      }
      else if(first_key == 's') {  // SI command - set count of sats to display
         si_cmd:
         edit_info1 = "  Include a '+' for abbreviated sat info in two columns";
         edit_info2 = "  Include a '-' for abbreviated sat info in a single column";
         edit_info3 = "  Include a 'T' to show only Tracked satellites";
         if(sat_cols > 1) sprintf(edit_buffer, "+%d", max_sat_display);
         else             sprintf(edit_buffer, "%d", max_sat_display);
         if(tracked_only) strcat(edit_buffer, " T");
         start_edit(SI_CMD, "Enter max number of sats to display info for  (ESC ESC to abort):");
         return 0;
      }
      else if(first_key == 'w') {  // WI command - inverted video plot image dump
         invert_dump = 1;
         top_line = (MOUSE_ROW+2)*TEXT_HEIGHT;
         dump_image();
         return 0;
      }
      else if(first_key == 'z') {  // ZI command - zoom sat info
         if(rcvr_type == NO_RCVR) return sure_exit();
         change_zoom_config(6);
         zoom_screen = 'I';
         zoom_fixes = show_fixes;
         plot_signals = 5;
         config_screen(120);
      }
      else if(first_key == '$') {  // $I command - invert black/white
         invert_screen();
      }
      else if(first_key == 0) {
         edit_buffer[0] = 0;
         start_edit('i', "Enter the queue update interval in seconds: (ESC to abort):");
         return 0;
      }
      else return help_exit(c,99);
   }
   else if(c == 'j') {
      if(first_key == '&') { // &J command - jam sync threshold
         if(luxor) return help_exit(c,99);
         sprintf(edit_buffer, "%f", user_jam_sync);
         start_edit('6', "Enter jam sync threshold in ns (ESC ESC to abort):"); 
         return 0;
      }
      else if(first_key == 'f') {  // FJ command - set jamming filter
         if(luxor) return help_exit(c,99);
         if     (foliage_mode == 0) strcpy(edit_buffer, "S");
         else if(foliage_mode == 2) strcpy(edit_buffer, "N");
         else                       strcpy(edit_buffer, "N");
         start_edit('o', "Jamming:  S) enable   N) disable");
         return 0;
      }
      else if(first_key == 'g') {  
         if(luxor) {               // GJ command - toggle LEDw plot
            edit_plot(TWELVE, c);
            return 0;
         }
         else {   // GJ command - toggle el mask in azel plot
            if(plot_azel == 0) plot_el_mask = 1;
            else               plot_el_mask = toggle_value(plot_el_mask);
            plot_azel = 1;
            update_azel = 1;
            goto do_azel;
         }
      }
      else if(first_key == 'j') {  // JJ command - jam sync
         if(luxor) return help_exit(c,99);
         set_discipline_mode(0);
         BEEP(312);
      }
      else if(first_key == 't') {  // TJ command - show Julian time digital clock
         show_julian_time = toggle_value(show_julian_time);
         show_msecs = 0;
         need_redraw = 4455;
      }
      else if(first_key == 0) {   // J command first char - Jam sync
         if(luxor) return help_exit(c,99);
         if(are_you_sure(c) != c) return 0;
      }
      else return help_exit(c,99);
   }
   else if(c == 'k') {  
      if(first_key == 'f') {  // FK command - toggle KALMAN filter
         if(luxor) return help_exit(c,99);
         kalman_filter = toggle_value(kalman_filter);
         set_filter_config(pv_filter, static_filter, alt_filter, kalman_filter, marine_filter, 1);
      }
      else if(first_key == 'g') {
         if(luxor) {               // GK command - toggle driver efficency plot
            edit_plot(THIRTEEN, c);
            return 0;
         }
         else {  // GK command - satellite constellation changes
            plot_const_changes = toggle_value(plot_const_changes);
            draw_plot(1);
         }
      }
      else if(first_key == 't') {  // TK commmand - setup for receiver message jitter measurements
         measure_jitter = toggle_value(measure_jitter);
         set_jitter_config();
      }
      else if(first_key == 0) {
         if(have_temperature && (tsip_type != STARLOC_RCVR)) {
            sprintf(edit_buffer, "K %f %f %f %f", P_GAIN, D_TC, FILTER_TC, I_TC);
            edit_info2    = "Autotune:      A0)abort  A1)full  A2)no setpoint delay  A3)no stabilize delay";
            edit_info1    = "Custom PID:    K [proportional fain] [derivative_tc] [filter_tc] [integrator_tc]";
            start_edit('k', "TEMP CTRL PID: W)slow  N)medium  X)fast  Y)very fast");
            return 0;
         }
         else return help_exit(c,3);
      }
      else return help_exit(c,3);
   }
   else if(c == 'l') { 
      if(first_key == '&') { 
         if(luxor) {  // &L command - lux sensor sensitivity
            sprintf(edit_buffer, "%.0f", (float) lux1_time);
            sprintf(out, "Lux sensor gain: L)ow=%d  M)edium=%d  H)high=%d  or enter gain (%d..%d) (ESC ESC to abort):", LOW_LUX,MED_LUX,HI_LUX, MIN_LUX,MAX_LUX); 
            start_edit(AMPL_CMD, out);
            return 0;
         }
         else {  // &L command - change min dac voltage
            sprintf(edit_buffer, "%f", user_min_range);
            start_edit('t', "Enter minimum DAC voltage range value (ESC ESC to abort):"); 
            return 0;
         }
      }
      else if(luxor && (first_key == 'b')) {  // BL command - lipo charge mode
         sprintf(edit_buffer, "%f", 0.1F);
         start_edit(BL_CMD, "Enter desired LIPO charge current in amps.  (ESC ESC to abort):"); 
         return 0;
      }
      else if(first_key == 'c') {  // CL command - clear LLA fixes
         if(luxor) return help_exit(c,99);
         #ifdef BUFFER_LLA
            clear_lla_points();
            need_redraw = 1209;
         #endif
      }
      else if(first_key == 'f') {  // FL command - signal level mask
         if(luxor) return help_exit(c,99);
         sprintf(edit_buffer, "%.1f", amu_mask);
         if((res_t && (res_t != RES_T)) || (rcvr_type != TSIP_RCVR)) {
            start_edit('m',  "Enter minimum acceptible signal level (in dBc)");
         }
         else start_edit('m',  "Enter minimum acceptible signal level (in AMU)");
         return 0;
      }
      else if(first_key == 'g') {
         if(luxor) {  // GL command - color temperature
            edit_plot(FOURTEEN, c);
            return 0;
         }
         else {  // GL command - change location formatting
            edit_loc_format();
            return 0;
         }
      }
      else if(luxor && (first_key == 'p')) {  // PL command = low voltage cutoff
         sprintf(edit_buffer, "%f", batt_lvc);
         start_edit(PL_CMD, "Enter the battery low voltage cutoff in volts.  (ESC ESC to abort):");
         return 0;
      }
      else if(first_key == 's') {  // SL command - set lat/lon/alt
         if(luxor) return help_exit(c,99);
         if(check_precise_posn) {
            strcpy(edit_buffer, "Y");
            start_edit('L', "Lat/lon/alt search aborted! Save low resolution position? Y)es N)o");
         }
         else if(rcvr_type == UCCM_RCVR) {
            sprintf(edit_buffer, "%.8lf %.8lf %.3lf", lat*RAD_TO_DEG, lon*RAD_TO_DEG, alt);
            edit_info1 = "The lat/lon values can be a decimal number or like 10d20m30s";
            edit_info2 = "The altiude value is in meters unless ended with an f or '";
            start_edit('l', "Enter lat lon alt (-=S,W +=N,E  alt in meters)  (ESC ESC to abort):");
         }
         else {
            sprintf(edit_buffer, "%.8lf %.8lf %.3lf", precise_lat*RAD_TO_DEG, precise_lon*RAD_TO_DEG, precise_alt);
            edit_info1 = "The lat/lon values can be a decimal number or like 10d20m30s";
            edit_info2 = "The altiude value is in meters unless ended with an f or '";
            start_edit('l', "Enter precise lat lon alt (-=S,W +=N,E  alt in meters)  (ESC ESC to abort):");
         }
         return 0;
      }
      else if(first_key == 't') { // TL command - set log dump time
         if     (script_file) edit_buffer[0] = 0;
         else if(log_time && log_date) sprintf(edit_buffer, "%02d:%02d:%02d  %02d/%02d/%04d", log_hh,log_mm,log_ss, log_month,log_day,log_year); 
         else if(log_time) sprintf(edit_buffer, "%02d:%02d:%02d", log_hh,log_mm,log_ss);
         else if(log_date) sprintf(edit_buffer, "%02d/%02d/%04d", log_month,log_day,log_year);
         else if(log_val) {
            if((log_val >= (24L*60L*60L)) && ((log_val % (24L*60L*60L)) == 0)) sprintf(edit_buffer, "%ld d", log_val/(24L*60L*60L));
            else if((log_val >= (3600L)) && ((log_val % 3600L) == 0)) sprintf(edit_buffer, "%ld h", log_val/(3600L));
            else if((log_val >= (60L)) && ((log_val % 60L) == 0)) sprintf(edit_buffer, "%ld m", log_val/(60L));
            else sprintf(edit_buffer, "%ld s", log_val);
         }
         else edit_buffer[0] = 0;

         edit_info1 = "Dates are in the format mm/dd/yyyy or yyyy/mm/dd";
         edit_info2 = "Intervals can be in seconds, minutes, hours or days like: 7s, 10m, 2h, 4d";
         if(dump_xml) {
            if(edit_buffer[0]) start_edit('(', "Enter XML log dump time (and optional date) or interval or <ESC CR> to reset:");
            else               start_edit('(', "Enter XML log dump time (and optional date) or interval or <CR> to reset:");
         }
         else if(dump_gpx) {
            if(edit_buffer[0]) start_edit('(', "Enter GPX log dump time (and optional date) or interval or <ESC CR> to reset:");
            else               start_edit('(', "Enter GPX log dump time (and optional date) or interval or <CR> to reset:");
         }
         else {
            if(edit_buffer[0]) start_edit('(', "Enter ASCII log dump time (and optional date) or interval or <ESC CR> to reset:");
            else               start_edit('(', "Enter ADCII log dump time (and optional date) or interval or <CR> to reset:");
         }
         return 0;
      }
      else if(first_key == 'w') {  // WL command - write log
         first_key = 0;
         c = 'l';
         goto log_cmd;
      }
      else if((rcvr_type != NO_RCVR) && (first_key == 'z')) {  // ZL command - zoom lla
         config_lla_zoom(0);
      }
      else if(first_key == '$') {  // $l command - large screen
         new_screen(c);
         return 0;
      }
      else {    // L command - log stuff
         log_cmd:
         if(are_you_sure(c) != c) return 0;
         // the next keystroke will select the data to write
      }
   }
   else if(c == 'm') {  // toggle auto scaling of graphs
      if(luxor && (first_key == '&')) { // &m command - set medium sensitivity
         set_luxor_sens(69, 69);
      }
      else if(first_key == 'c') {  // CM command - clear satellite map data
         if(luxor) return help_exit(c,99);
         #ifdef SAT_TRAILS
            clear_sat_trails();
            need_redraw = 1210;
         #endif
      }
      else if(first_key == 'f') {  // FM command - movement dynamics
         if(rcvr_type == MOTO_RCVR) {
            sprintf(edit_buffer, "%d", marine_filter);
            start_edit(MARINE_CMD, "Enter marine velocity filter (10=max  100=min  ESC ESC to abort)");
         }
         else {
            if     (dynamics_code == 1) strcpy(edit_buffer, "L");
            else if(dynamics_code == 2) strcpy(edit_buffer, "S");
            else if(dynamics_code == 3) strcpy(edit_buffer, "A");
            else                        strcpy(edit_buffer, "F");
            start_edit('u',  "Receiver movement:  A)ir  F)ixed  L)and  S)ea");
         }
         return 0;
      }
      else if(first_key == 'g') {  // GM command - plot satellite map
         if(luxor) return help_exit(c,99);
         plot_azel = toggle_value(plot_azel);
         do_azel:
         if(AZEL_OK == 0) plot_azel = AZEL_OK;
//       if((WIDE_SCREEN == 0) && ((shared_plot == 0) || all_adevs)) {
//          if(plot_signals) plot_watch = 0;  //!!!!!
//          if(plot_azel || plot_signals) plot_watch = 0;
////        else          plot_watch = 1;
//       }
         if(plot_azel || plot_signals) update_azel = 1;
         config_screen(114);
      }
      #ifdef ADEV_STUFF
         else if(first_key == 'a') {  // AM command - set adev type to MDEV
            if(luxor) return help_exit(c,99);
            ATYPE = OSC_MDEV;
            all_adevs = 0;
            force_adev_redraw();
            config_screen(115);
         }
      #endif
      else if(luxor && (first_key == 'p')) {  // PM command = message watchdog timer
         sprintf(edit_buffer, "%.3f", msg_timeout);
         start_edit(PM_CMD, "Enter the Message watchdog timeout in seconds (0=disable).  (ESC ESC to abort):");
         return 0;
      }
      else if(first_key == 't') {  // TM command - show millisecond digital clock
         show_msecs = toggle_value(show_msecs);
         show_julian_time = 0;
         need_redraw = 4455;
      }
      else if(first_key == 'z') {  // ZM command - zoom map
         if(rcvr_type == NO_RCVR) return sure_exit();
         change_zoom_config(5);
         zoom_screen = 'M';
         zoom_fixes = show_fixes;
         config_screen(116);
      }
      else if(first_key == '$') {  // $M command - medium screen
         new_screen(c);
         return 0;
      }
      else if(first_key == '!')  { // !M command - restart Zodiac receiver in Motorola mode
         force_mode_change = 1;
         request_warm_reset();
         redraw_screen();
      }
      else if(luxor) {  // m command - battery pwm control (for now)
         strcpy(edit_buffer, mode_string);
         if(!isdigit(edit_buffer[1]) || (edit_buffer[0] == '.') || (edit_buffer[0] == 0)) {
            if(((edit_buffer[0] == '0') || (edit_buffer[0] == '.') || (edit_buffer[0] == 0)) && (batt_pwm == 0)) strcpy(edit_buffer, "1");
            else if(((edit_buffer[0] == '1') || (edit_buffer[0] == '.') || (edit_buffer[0] == 0)) && (batt_pwm != 0)) strcpy(edit_buffer, "0");
         }
         edit_info1 = "or enter * for double 100 msec mode change pulses";
         if(edit_buffer[0]) start_edit(DRVR_MODE, "Enter driver mode sequence: 0=OFF 1=ON  or  OFFmsecs [ONmsecs...]  (ESC ESC to abort):");
         else               start_edit(DRVR_MODE, "Enter driver mode sequence: 0=OFF 1=ON  or  OFFmsecs [ONmsecs...]  (ESC to abort):");
         return 0;
      }
      else return help_exit(c,4);
   }
   else if(c == 'n') {
      if(first_key == '&') { // &N command - change min voltage
         if(luxor) return help_exit(c,99);
         sprintf(edit_buffer, "%f", user_min_volts);
         start_edit('4', "Enter minimum EFC control voltage (ESC ESC to abort):"); 
         return 0;
      }
      else if(first_key == 's') {  // SN command - navigation (3D fix) mode
         set_rcvr_mode(RCVR_MODE_3D);
      }
      else if(first_key == 'z') {  // ZN command - no zoom
         if(zoom_screen) {
            change_zoom_config(0);
            zoom_screen = 0;
            zoom_fixes = show_fixes;
            config_screen(117);
         }
      }
      else if(first_key == '$') {  // $n command - custom screen
         new_screen(c);
         return 0;
      }
      else if(1 && (first_key == '!'))  { // !N command - restart moto receiver in NMEA mode
         if(rcvr_type == MOTO_RCVR) {     // !!!! does not work on timing receivers
            enable_moto_nmea();
         }
         else if(rcvr_type == NMEA_RCVR) {
            enable_moto_binary();
         }
         redraw_screen();
      }
      else return help_exit(c,5);
   }
   else if(c == 'o') {
      if(first_key == 'g') {  // GO command - toggle graph osc ppb data
         edit_plot(OSC, c);
         return 0;
      }
      #ifdef ADEV_STUFF
         else if(first_key == 'a') {  // AO command - graph all OSC adev types
            if(luxor) return help_exit(c,99);
            aa_val = 1;
            strcpy(edit_buffer, "G");
            start_edit('a', "Display:  A)devs only   G)raphs and all adevs   graphs and R)egular adevs");
            return 0;
         }
      #endif
      else if(luxor && (first_key == 'p')) {  // PO command = battery overcurrent
         sprintf(edit_buffer, "%f", load_ovc);
         start_edit(PO_CMD, "Enter the load overcurrent threshold in amps.  (ESC ESC to abort):");
         return 0;
      }
      else if(first_key == 'p') { // PO command - toggle osc polarity
         if(luxor) return help_exit(c,99);
         if(rcvr_type != TSIP_RCVR) {  // pps offset delay in nanoseconds
            sprintf(edit_buffer, "%.1f", pps1_delay);
            start_edit(PPS_OFS_CMD, "Enter PPS position offset in ns (ESC ESC to abort):"); 
            return 0;
         }
         else {
            user_osc_polarity = toggle_value(user_osc_polarity);
            set_osc_sense(user_osc_polarity, 1);
         }
      }
      else if(first_key == 's') { // SO command - single sat mode
         if(luxor) return help_exit(c,99);
         if((rcvr_type == TSIP_RCVR) && (rcvr_mode == 1)) sprintf(edit_buffer, "%d", 0);
         else if(single_sat) sprintf(edit_buffer, "%d", 0);
         else {
            sprintf(edit_buffer, "%d", highest_sat());
         }
         if(rcvr_type == TSIP_RCVR) {
            if(rcvr_mode == 1) start_edit('x', "Enter 0 to exit single sat mode");
            else               start_edit('x', "Enter PRN of sat for single sat mode (0=highest)");
         }
         else {
            start_edit('x', "Enter PRN of sat for single sat mode (0=enable all sats)");
         }
         return 0;
      }
      else if(first_key == 0) {  // O command
//       if(luxor) return help_exit(c,99);
         strcpy(edit_buffer, "");
         edit_info1 = "A)mu  B)ins   C)ount  D)B  E)bolt  reF)resh adevs  H)ourly erase fixes";
         edit_info2 = "J)serial log  K)fault log  L)ive FFT  M)agnify plots  N)trendline update";
         edit_info3 = "P)eak scale   Q)ueue mode  R)eset bins S)pike filter  T)rigger times  W)atch face";
         start_edit(',',  "Enter option letter and optional value (ESC to abort):");
         return 0;
      }
      else return help_exit(c,6);
   }
   else {  // help mode
      return help_exit(c,7);
   }

   return sure_exit();
}

int kbd_other(int c)
{
long val;
int i;
char p;

   // process keyboard characters that are not a..o or cursor movement
   if(c == 'p') { 
      if(luxor && (first_key == 'b')) {  // BP command - PWM resolution
         sprintf(edit_buffer, "%d", batt_pwm_res);
         start_edit(BP_CMD, "Enter PWM resolution in bits (8/9/10).  (ESC ESC to abort):"); 
         return 0;
      }
      else if(first_key == 'c') {  // CP command - clear plot queue
//!!!!-  pause_data = 0;
         new_queue(2);
      }
      else if(first_key == 'f') {  // FP command - toggle PV filter
         if(luxor) return help_exit(c,99);
         pv_filter = toggle_value(pv_filter);
         set_filter_config(pv_filter, static_filter, alt_filter, kalman_filter, marine_filter, 1);
      }
      #ifdef ADEV_STUFF
         else if(first_key == 'a') {  // AP command - graph all PPS adev types
            if(luxor) return help_exit(c,99);
            aa_val = 2;
            strcpy(edit_buffer, "G");
            start_edit('a', "Display  A)devs only   G)raphs and all adevs   graphs and R)egular adevs");
            return 0;
         }
      #endif
      else if(first_key == 'g') {  // GP command toggle PPS plot
         edit_plot(PPS, c);
         return 0;
      }
      else if(luxor && (first_key == 'p')) {  // PW command = load high voltage cutoff
         sprintf(edit_buffer, "%f", load_watts);
         start_edit(PP_CMD, "Enter the LED power cutoff in watts.  (ESC ESC to abort):");
         return 0;
      }
      #ifdef PRECISE_STUFF
         else if(first_key == 's') { // SP command - precison survey
            if(luxor) return help_exit(c,99);
            redraw_screen();
            if(precision_survey) {   // abort precision survey
               strcpy(edit_buffer, "Y");
               start_edit('A', "Precise survey aborted! Save current position? Y)es N)o");
               return 0;
            }
            else {  // start precison survey
               strcpy(edit_buffer, "48");
               sprintf(out, "Enter number of hours to do survey for (1-%d,  ESC ESC to abort) : ", SURVEY_BIN_COUNT);
               start_edit('^', out);
               return 0;
            }
         }
      #endif
      else if(first_key == 'w') {  // WP command - Write plot area data to a log file
         strcpy(edit_buffer, "dump.log");
         sprintf(out, "Enter name of log file to write %sPLOT area info to (ESC ESC to abort):",
                       filter_log?"filtered ":"");
         start_edit('w', out);
         log_mode = "w";
         dump_type = 'p';
         return 0;
      }
      else if(first_key == 'p') {  // PP command - toggle PPS polarity
         if(luxor) return help_exit(c,99);
         i = toggle_value(pps_polarity);
         user_pps_enable = 1;
         set_pps(user_pps_enable, i,  delay_value, pps1_delay, 300.0, 3);
         request_pps_info();
      }
      else if(first_key == 'z') {  // ZP command - zoom plot
         if(rcvr_type == NO_RCVR) return sure_exit();
         change_zoom_config(5);
         zoom_screen = 'P';
         zoom_fixes = show_fixes;
         config_screen(118);
      }
      else if(first_key == '!') {    // !P command - serial port parameters
         edit_info1 = "Note: this sets the system serial port config";
         edit_info2 = "      and not the receiver serial port config";
         if(parity == 1) p = 'O';
         else if(parity == 2) p = 'E';
         else p = 'N';
         sprintf(edit_buffer, "%d:%d:%c:%d", baud_rate, data_bits, p, stop_bits);
         start_edit(SERIAL_CMD, "Enter serial port settings like 9600:8:N:1   (ESC ESC to abort)");
         return 0;
      }
      else if(first_key == '&') {  // &p command - set pullin range
         if(have_pullin) sprintf(edit_buffer, "%d", pullin_range);
         else            sprintf(edit_buffer, "%d", 30);
         start_edit(PULLIN_CMD, "Enter pull-in range in PPB (10..2550, 0=UNLIMITED) (ESC ESC to abort)");
         return 0;
      }
      else if(luxor && (first_key == 0)) { // P command - luxor protection values
         if(are_you_sure(c) != c) return 0; 
         // next call to do_kbd() will process the selection character
      }
      else { // P command - time menu, select PPS menu
         if(are_you_sure(c) != c) return 0; 
         // next call to do_kbd() will process the selection character
      }
   }
   else if(c == 'q') {
      if(first_key == 'g') { // GQ command - signal level map 
         if(plot_signals == 4) plot_signals = 0;
         else                  plot_signals = 4;

         if(plot_signals == 0) {
            if(zoom_screen) {
               zoom_screen = 0; 
               change_zoom_config(0);
            }
         }
         if(plot_watch && plot_azel) {
            shared_plot = 1;
            prot_menu = 0;
         }
         config_screen(119);
//
//         edit_buffer[0] = 0;
//         start_edit('Q', "A)zimuth  W)eighted azimuth  E)levation  S)ignals  D)ata  C)lear  <cr>=off");
//if(plot_watch && plot_azel) {
//   shared_plot = 1;
//   prot_menu = 0;
//}
//         return 0;
      }
      else if(first_key == 'z') {  // ZQ command - zoom signals
         if(rcvr_type == NO_RCVR) return sure_exit();
         change_zoom_config(7);
         zoom_screen = 'S';
         zoom_fixes = show_fixes;
         plot_signals = 4;
         config_screen(120);
      }
      else return help_exit(c,8);
   }
   else if(c == 'r') { 
      if(first_key == 'g') { // GR command - redraw screen
         first_key = 0;
         if(luxor == 0) {
            debug_text[0] = 0;
            debug_text2[0] = 0;
         }

//       reset_marks();
         osc_params = 0;
         debug_text[0] = 0;  // erase debug info strings
         debug_text2[0] = 0;
         debug_text3[0] = 0;
         redraw_screen();
      }
#ifdef ADEV_STUFF
      else if(first_key == 'c') {  // CR command - reload adev queue from screen data
         if(luxor) return help_exit(c,99);
         reload_adev_queue();
      }
#endif
      else if(luxor && (first_key == 'p')) {  // PR command = reset protection fault
         strcpy(edit_buffer, "r");
         start_edit(PR_CMD, "Enter R to reset the protection fault flags.  (ESC ESC to abort):");
         return 0;
      }
      else if(luxor && (first_key == 'b')) {  // BR command - sweep PWM rate
         sprintf(edit_buffer, "%d", sweep_rate);
         start_edit(BR_CMD, "Enter PWM sweep rate in seconds per step.  (ESC ESC to abort):"); 
         return 0;
      }
      else if(first_key == 'p') {  // PR command - rising edge PPS polarity
         if(luxor) return help_exit(c,99);
         user_pps_enable = 1;
         set_pps(user_pps_enable, 0,  delay_value, pps1_delay, 300.0, 3);
         request_pps_info();
      }
      else if(first_key == 't') {  // TR command - show sun rise and set times
         if(sunrise_type == 0) sunrise_type = "Official";
         if(sunrise_type[0] == 'U') sprintf(edit_buffer, "%.3f", sunrise_horizon);
         else if(sunrise_type[0])   sprintf(edit_buffer, "%s", sunrise_type);
         else                       sprintf(edit_buffer, "Official");
         if(play_sun_song) strcat(edit_buffer, " *");
         if(realtime_sun) strcat(edit_buffer, " !");
         edit_info1 = "  A)stronomical  C)ivil  N)autical  O)fficial  P)hysical  M)oon";
         edit_info2 = "  or enter the desired sunrise/sunset horizon in degrees.";
         edit_info3 = "    Include a '*' to play the sun songs at rise/noon/set.";
         edit_info4 = "    Include a '!' to recalculate rise/settimes every second.";
         start_edit(SUN_CMD, "Enter sunrise type.  (ESC CR for none)  (ESC ESC to abort):"); 
         return 0;
      }
      else if(first_key == 'w') {  // WR command - inverted video screen dump
         invert_dump = 1;
         top_line = 0;
         dump_image();
         return 0;
      }
      else if(first_key == 'z') {  // ZR command - zoom antenna relative strenth map
         if(rcvr_type == NO_RCVR) return sure_exit();
         change_zoom_config(6);
         zoom_screen = 'S';
         zoom_fixes = show_fixes;
         plot_signals = 1;
         config_screen(120);
      }
      else if(first_key == '!') {  // !R command - set nav rate
         sprintf(edit_buffer, "%.0f", nav_rate);
         start_edit(NAV_RATE_CMD, "Enter navigation rate in Hz  (ESC ESC to abort)");
         return 0;
      }
      else if(first_key == '&') { // &R command - change time constant
         if(luxor) return help_exit(c,99);
         if(gpsdo_ref == 1) strcpy(edit_buffer, "PPS");
         else               strcpy(edit_buffer, "GPS");
         start_edit(REF_CMD, "Enter GPSDO reference source: G)ps  P)ps  (ESC ESC to abort):"); 
         return 0;
      }
      else if(first_key == 0) {  // R command read in a log file
         edit_buffer[0] = 0;
         if(luxor) {
            start_edit('r', "Enter name of .LOG .CAL .CFG .WAV or .SCRipt file to read (ESC to abort):");
         }
         else {
            edit_info1 = "The file type is determined by the file name extension.";
            edit_info2 = "Valid extensions: .LOG .XML .GPX .ADV .LLA .SIG .CAL .CFG .WAV .TIM or .SCRipt";
            start_edit('r', "Enter name of file to read (ESC to abort):");
         }
         return 0;
      }
      else return help_exit(c,99);
   }
   else if(c == 's') { 
      if(luxor && (first_key == 'b')) {  // BS command - sweep PWM value
         sprintf(edit_buffer, "%f %f", 0.0F, 1.0F);
         start_edit(BS_CMD, "Enter PWM sweep starting and ending PWM values.  (ESC ESC to abort):"); 
         return 0;
      }
      else if(first_key == 'c') {  // CS command - clear signal level data
         if(luxor) return help_exit(c,99);
         #ifdef SIG_LEVELS
            clear_signals();
            need_redraw = 1211;
         #endif
      }
      else if(first_key == 'd') { // DS command - disable osc discipline and set DAC voltage
         if(luxor) return help_exit(c,99);
         if(rcvr_type == UCCM_RCVR) {
            sprintf(edit_buffer, "%f", uccm_voltage);
            start_edit('c', "Enter DAC control value (ESC ESC to abort):");
         }
         else {
            sprintf(edit_buffer, "%f", dac_voltage);
            start_edit('c', "Enter DAC control voltage (ESC ESC to abort):");
         }
         return 0;
      }
      else if(first_key == 'f') {  // FS command - toggle STATIC filter
         if(luxor) return help_exit(c,99);
         static_filter = toggle_value(static_filter);
         set_filter_config(pv_filter, static_filter, alt_filter, kalman_filter, marine_filter, 1);
      }
      else if(first_key == 'g') { // GS command - toggle sound
////     sound_on = toggle_value(sound_on);
//       if(sound_on) BEEP(313);
         if(sound_on || beep_on) {
            sound_on = beep_on = 0;
         }
         else {
            sound_on = beep_on = 1;
            BEEP(313);
         }
         draw_plot(1);
      }
      else if(first_key == 'l') {  // LS command - stop logging
         if(log_file) {
            close_log_file();
            have_info &= (~INFO_LOGGED);
         }
         else edit_error("Log file is not open.");
      }
      else if(first_key == 's') {  // SS command - standard survey
         if(luxor) return help_exit(c,99);
         if(doing_survey) {         // survey in progress,  stop self survey
            stop_self_survey();     // stop any self survey
            redraw_screen();
         }
         else if(rcvr_type == NVS_RCVR) {
            if(do_survey) sprintf(edit_buffer, "%ld", do_survey);
            else          sprintf(edit_buffer, "%ld", SURVEY_SIZE/60);
            start_edit('s', "Enter number of minutes for self survey (20 .. 1440) (ESC ESC to abort):");
            return 0;
         }
         else if(rcvr_type == TSIP_RCVR) {
            if(do_survey) sprintf(edit_buffer, "%ld", do_survey);
            else          sprintf(edit_buffer, "%ld", SURVEY_SIZE);
            start_edit('s', "Enter number of samples for standard self survey (ESC ESC to abort):");
            return 0;
         }
         else if(rcvr_type == UBX_RCVR) {
            if(do_survey) sprintf(edit_buffer, "%ld", do_survey);
            else          sprintf(edit_buffer, "%ld", SURVEY_SIZE);
            start_edit('s', "Enter number of seconds for standard self survey (ESC ESC to abort):");
            return 0;
         }
         else if(rcvr_type == VENUS_RCVR) {
            if(do_survey) sprintf(edit_buffer, "%ld", do_survey);
            else          sprintf(edit_buffer, "%ld", SURVEY_SIZE);
            start_edit('s', "Enter number of samples for self survey (60 .. 1209600) (ESC ESC to abort):");
            return 0;
         }
         else if(rcvr_type == ZODIAC_RCVR) {
            if(do_survey) sprintf(edit_buffer, "%ld", do_survey);
            else          sprintf(edit_buffer, "%ld", SURVEY_SIZE/1000);
            start_edit('s', "Enter number of hours for standard self survey (ESC ESC to abort):");
            return 0;
         }
         else  {
            strcpy(edit_buffer, "1");
            start_edit('s', "Enter <cr> to start standard survey.  (ESC ESC to abort):");
            return 0;
         }
      }
      else if(luxor && (first_key == 'p')) {  // PS command - temp2 over temperature
         sprintf(edit_buffer, "%f", tc2_ovt);
         start_edit(PS_CMD, "Enter temp sensor 2 over-temperature cutoff (degrees C).  (ESC ESC to abort):");
         return 0;
      }
      else if(first_key == 'p') {   // PS command - toggle PPS signal
         user_pps_enable = pps_enabled ^ 1;
         set_pps(user_pps_enable, pps_polarity,  delay_value, pps1_delay,  300.0, 4);
         request_pps_info();
      }
      else if(first_key == 't') {  // TS command - set system time to receiver time
         set_system_time = toggle_value(set_system_time);
         if(set_system_time) need_time_set();
      }
      else if(first_key == 'w') {  // WS command - Dump screen to .GIF/.BMP file
         invert_dump = 0;
         top_line = 0;
         dump_image();
         return 0;
      }
      else if(first_key == 'z') {  // ZS command - zoom quality map
         if(rcvr_type == NO_RCVR) return sure_exit();
         change_zoom_config(6);
         zoom_screen = 'S';
         zoom_fixes = show_fixes;
         plot_signals = 4;
         config_screen(120);
      }
      else if(first_key == '&') { 
         if(luxor) {   // &s command - set unit serial number
            sprintf(edit_buffer, "%.4f", vref_b);
            start_edit(AMPS_CMD, "Enter the unit serial number (#.####). (ESC ESC to abort):");
            return 0;
         }
      }
      else if(first_key == '$') {  // $s command - small screen
         new_screen(c);
         return 0;
      }
      else if(first_key == '!') {  // !S command - re-init com port
         init_com();
request_rcvr_info(222);  // ggggg
      }
      else if((first_key == 0) && luxor) {
         sprintf(edit_buffer, "%f %f %f", lat*RAD_TO_DEG, lon*RAD_TO_DEG, alt);
         start_edit(S_CMD, "Enter lat lon alt: (+ for N and E,  - for S and W,  alt in meters) (ESC ESC to abort)");
         return 0;
      }
      else {  // S command - survey menu
         if(are_you_sure(c) != c) return 0;  
         // next call to do_kbd() will process the selection character
      }
   }
   else if(c == 't') { 
      if(first_key == 'f') {  // FT command - toggle Motorola troposphere filter
         if(luxor) return help_exit(c,99);
         alt_filter = toggle_value(alt_filter);
         set_filter_config(pv_filter, static_filter, alt_filter, kalman_filter, marine_filter, 1);
      }
      else if(first_key == 'g') {   // GT command - temperature graph
         edit_plot(TEMP, c);
         return 0;
      }
      #ifdef ADEV_STUFF
        else if(first_key == 'a') {  // AT command - set adev type to TDEV
           if(luxor) return help_exit(c,99);
           ATYPE = OSC_TDEV;
           all_adevs = 0;
           force_adev_redraw();
           config_screen(121);
        }
      #endif
      #ifdef TEMP_CONTROL
         else if(first_key == 't') {  // TT command - active temperature control
            sprintf(edit_buffer, "%.3f", desired_temp);
            sprintf(out, "Enter desired operating temperature in %cC: (0=OFF  ESC ESC to abort)", DEGREES);
            start_edit('*', out);
            return 0;
         }
      #endif
      else if(luxor && (first_key == 'p')) {  // PT command - temp1 over temperature
         sprintf(edit_buffer, "%f", tc1_ovt);
         start_edit(PT_CMD, "Enter temp sensor 1 over-temperature cutoff (degrees C).  (ESC ESC to abort):");
         return 0;
      }
      else if(first_key == 'p') {  // PT command - set traim threshold / enable
         if((rcvr_type == MOTO_RCVR) || (rcvr_type == ZODIAC_RCVR)) {
            sprintf(edit_buffer, "%d", traim_threshold);
            start_edit(TRAIM_CMD, "Enter traim threshold in ns (0=OFF) (ESC ESC to abort):"); 
            return 0;
         }
         else if(rcvr_type == NVS_RCVR) { 
            if(traim_mode) {
               sprintf(edit_buffer, "%d", 0);
            }
            else {
               sprintf(edit_buffer, "%d", 3);
            }
            edit_info1 = "Add 4 to disable 2D mode and/or add 8 to disable single sat mode";
            start_edit(TRAIM_CMD, "0=OFF  1=RAIM  2=FDE  3=RAIM+FDE  (ESC ESC to abort):"); 
            return 0;
         }
         else return help_exit(c,99);
      }
      else if(first_key == '$') {  // $t command - text mode screen
         new_screen(c);
         return 0;
      }
      else if(first_key == '&') { // &T command - change time constant
         if(luxor) return help_exit(c,99);
         sprintf(edit_buffer, "%f", user_time_constant);
         start_edit('1', "Enter oscillator time constant in seconds (ESC ESC to abort):"); 
         return 0;
      }
      else if(first_key == '!') {    // !T command - terminal emulator
         enable_terminal = 1;
         do_term();

         tsip_sync = 0;
         tsip_wptr = 0;
         tsip_rptr = 0;
         init_messages(10);
         redraw_screen();
         first_key = 0;
      }
      else { // T command - time menu, select UTC or GPS time etc.
         if(are_you_sure(c) != c) return 0; 
         // next call to do_kbd() will process the selection character
      }
   }
   else if(c == 'u') {  // Updates or UTC time
      if(first_key == 'g') {  // GU command - constant graph updates - we now always do this!
//       continuous_scroll = toggle_value(continuous_scroll);
//       config_screen(122);
         reset_marks();       // GU command - reset all markers
      }
      else if(luxor && (first_key == 'p')) {  // PU command - led under voltage cutoff
         sprintf(edit_buffer, "%f", load_lvc);
         start_edit(PU_CMD, "Enter LED under-voltage cutoff (in volts).  (ESC ESC to abort):");
         return 0;
      }
      else if(first_key == 'u') {  // UU command - toggle updates
         pause_data = toggle_value(pause_data);
         if(pause_data == 0) {  // we just released a pause
            if(restore_nav_rate) {
               nav_rate = saved_nav_rate;
               restore_nav_rate = 0;
            }
            end_review(0);
         }
         redraw_screen();
      }
      else if(first_key == 't') {   // TU command - time UTC
//       time_zone_set = 0;
         temp_utc_mode = 0;
         set_timing_mode(0x03);
         if(rcvr_type == NVS_RCVR) request_pps_info();
         request_timing_mode();
      }
      else if(first_key == 'z') {  // ZU command - zoom all signals
         if(rcvr_type == NO_RCVR) return sure_exit();
         change_zoom_config(26);
         zoom_screen = 'U';
         zoom_fixes = show_fixes;
         plot_signals = 4;
         config_screen(120);
      }
      else if(first_key == '$') {  // $u command - very small screen
         new_screen(c);
         return 0;
      }
      else if(first_key == '!') {  // !u command
         if((rcvr_type == SCPI_RCVR) || (rcvr_type == UCCM_RCVR) || (rcvr_type == NMEA_RCVR) || (rcvr_type == GPSD_RCVR) || (rcvr_type == STAR_RCVR) || (rcvr_type == ACRON_RCVR)) {
            strcpy(edit_buffer, last_user_cmd);;
            start_edit(USER_CMD, "Enter command to send to the receiver (ESC ESC to abort):");
            return 0;
         }
         else return help_exit(c,99);
      }
      else if(first_key == '&') {  // &U command - lumen sensor sensitivity
         if(luxor) {  
            sprintf(edit_buffer, "%.0f", (float) lux2_time);
            sprintf(out, "Lumen sensor gain: L)ow=%d  M)edium=%d  H)high=%d  or enter gain (%d..%d) (ESC ESC to abort):", LOW_LUX,MED_LUX,HI_LUX, MIN_LUX,MAX_LUX); 
            start_edit(AMPU_CMD, out);
            return 0;
         }
         else return help_exit(c,99);
      }
      else if(first_key == 0) {  // U command first char - toggle updates
         if(are_you_sure(c) != c) return 0;
      }
      else return help_exit(c,99);
   }
   else if(c == 'v') {  // set plot View time
      if(first_key == '$') {  // $v command - very large screen
         new_screen(c);
         return 0;
      }
      else if(luxor && (first_key == '&')) {  // &v command - reference voltage
         sprintf(edit_buffer, "%.4f", vref_m);
         start_edit(AMPV_CMD, "Enter the unit reference voltage (4.5 .. 6.5) (ESC ESC to abort):");
         return 0;
      }
      else if(luxor && (first_key == 'b')) {  // BV command - contsant load voltage mode
         sprintf(edit_buffer, "%f", 0.1F);
         start_edit(BV_CMD, "Enter desired load voltage in volts.  (ESC ESC to abort):"); 
         return 0;
      }
      else if(luxor && (first_key == 'p')) {  // PV command = load high voltage cutoff
         sprintf(edit_buffer, "%f", load_hvc);
         start_edit(PV_CMD, "Enter the load high voltage cutoff in volts.  (ESC ESC to abort):");
         return 0;
      }
      else if(first_key == 'g') { // GV command - show lat/lon/alt
         if(luxor) return help_exit(c,99);
         i = plot[ONE].show_plot + plot[TWO].show_plot + plot[THREE].show_plot;
         if(i >= 3) {  // disable LLA plots
            plot[ONE].show_plot = 0;
            plot[TWO].show_plot = 0;
            plot[THREE].show_plot = 0;
         }
         else { // enable LLA plots
            plot[ONE].show_plot = 1;
            plot[TWO].show_plot = 1;
            plot[THREE].show_plot = 1;
         }
         redraw_screen();
      }
      else if(first_key == 'z') {  // ZV command - zoom stuff
         if(rcvr_type == NO_RCVR) return sure_exit();
         change_zoom_config(26);
         zoom_screen = 'V';
         zoom_fixes = show_fixes;
         plot_signals = 4;
         config_screen(120);
      }
      else if(first_key == 0) {   // v command - view time
         if(view_interval == 1) {
            val = PLOT_WIDTH / HORIZ_MAJOR;
            if(val) val = (((plot_q_count+59)/60)+val-1) / val;
            else    val = 20;
//          val /= HORIZ_MAJOR;
            if(val <= 0) val = 1;
            sprintf(edit_buffer, "%ld", val);
//          sprintf(edit_buffer, "20");
            edit_info1 = "or enter a time interval to display (like 20m, 3h, 2d)";
            start_edit('v', "Enter the plot view time in minutes/division: (A=all, T=auto, ESC ESC to abort):");
         }
         else {
            strcpy(edit_buffer, "0");
            edit_info1 = "or enter a time interval to display (like 20m, 3h, 2d)";
            start_edit('v', "Enter the plot view time in minutes/division: (0=normal,  A=all,  T=auto):");
         }
         return 0;
      }
      else return help_exit(c,99);
   }
   else if(c == 'w') {  
      if(first_key == '!')  { // !W command - warm reset and self test
         request_warm_reset();
         redraw_screen();
      }
      else if(luxor && (first_key == 'b')) {  // BW command - contsant load wattage mode
         sprintf(edit_buffer, "%f", 0.1F);
         start_edit(BW_CMD, "Enter desired load power in watts.  (ESC ESC to abort):"); 
         return 0;
      }
      else if(first_key == 'g') {  // GW command - draw watch
         plot_watch = toggle_value(plot_watch);
         if((WIDE_SCREEN == 0) && ((shared_plot == 0) || all_adevs)) {
//          if(plot_watch) plot_signals = 0;
//          if(plot_watch) plot_azel = plot_signals = 0;
////        else           plot_azel = AZEL_OK;
         }
         config_screen(123);
      }
      else if(first_key == 'l') {  // LW command - open log in write mode
         if(log_file) edit_error("Log file is already open.");
         else {
            log_mode = "w";
            strcpy(edit_buffer, log_name);
            edit_info1 = "Use extension .gpx .xml or .kml to write XML format logs.";
            start_edit('n', "Enter log file name to write data to: ");
            return 0;
         }
      }
      else if(luxor && (first_key == 'p')) {  // PW command = battery power cutoff
         sprintf(edit_buffer, "%f", batt_watts);
         start_edit(PW_CMD, "Enter the battery power cutoff in watts.  (ESC ESC to abort):");
         return 0;
      }
      else if(first_key == 't') {  // TW command - 12 hour digital clock
         clock_12 ^= 1;
      }
      else if(first_key == 'z') {  // ZW command - zoom watch
         change_zoom_config(8);
         zoom_screen = 'W';
         zoom_fixes = show_fixes;
         config_screen(124);
      }
      else {  // W command - write file menu
         if(are_you_sure(c) != c) return 0;
         // the next keystroke will select the data to write
      }
   }
   else if(c == 'x') {   // toggle view_interval between minutes and hours
      if(first_key == 'f') {  // FX command - PDOP mask/switch
         if(luxor) return help_exit(c,99);
         sprintf(edit_buffer, "%.1f", pdop_mask);
         start_edit('p', "Enter PDOP mask value (switch 2D/3D mode at mask*0.75)");
         return 0;
      }
      else if(first_key == 'h') {  // HX command - exit holdover
         set_discipline_mode(3);
      }
      else if(luxor && (first_key == 'p')) {  // PX command - auxv over voltage cutoff
         sprintf(edit_buffer, "%f", auxv_hvc);
         start_edit(PX_CMD, "Enter AUXV over-voltage cutoff (in volts).  (ESC ESC to abort):");
         return 0;
      }
      else if(first_key == 's') { // SX command - exclude sat mode
         if(luxor) return help_exit(c,99);
         if(single_sat) sprintf(edit_buffer, "%d", 0);
         else {
            sprintf(edit_buffer, "%d", 0);
         }
         start_edit(SAT_IGN_CMD, "Enter PRN of sat to exclude (0=enable all sats)");
         return 0;
      }
      else if(first_key == 't') { // TX command - set exit time
         if(end_time && end_date) sprintf(edit_buffer, "%02d:%02d:%02d  %02d/%02d/%04d", end_hh,end_mm,end_ss, end_month,end_day,end_year); 
         else if(end_time) sprintf(edit_buffer, "%02d:%02d:%02d", end_hh,end_mm,end_ss);
         else if(end_date) sprintf(edit_buffer, "%02d/%02d/%04d", end_month,end_day,end_year);
         else if(exit_val) {  // exit countdown timer set
            if((exit_val >= (24L*3600L)) && ((exit_val % 24L*3600L) == 0)) sprintf(edit_buffer, "%ld d", exit_val/(24L*3600L));
            else if((exit_val >= (3600L)) && ((exit_val % 3600L) == 0)) sprintf(edit_buffer, "%ld h", exit_val/(3600L));
            else if((exit_val >= (60L)) && ((exit_val % 60L) == 0)) sprintf(edit_buffer, "%ld m", exit_val/(60L));
            else sprintf(edit_buffer, "%ld s", exit_val);
         }
         else edit_buffer[0] = 0;

         edit_info1 = "Dates are in the format mm/dd/yyyy or yyyy/mm/dd";
         edit_info2 = "Intervals can be in seconds, minutes, hours or days like: 7s, 10m, 2h, 4d";
         if(edit_buffer[0]) start_edit('/', "Enter exit time (and optional date) or interval or <ESC CR> to reset:");
         else               start_edit('/', "Enter exit time (and optional date) or interval or <CR> to reset:");
         return 0;
      }
      else if(first_key == 'w') {  // WX command - open/close debug log file
         if(debug_file) {
            edit_buffer[0] = 0;
            strcpy(edit_buffer, "Y");
            sprintf(out, "Close debug log file (Y/N)?  (ESC ESC to abort):");
         }
         else {
            strcpy(edit_buffer, "debug.log");
            sprintf(out, "Enter name of debug log file to write  (ESC ESC to abort):");
         }
         start_edit(DEBUG_LOG_CMD, out);
         return 0;
      }
      else if(first_key == 'z') {  // ZX command - zoom all maps and watch
         if(rcvr_type == NO_RCVR) return sure_exit();
         change_zoom_config(16);
         zoom_screen = 'X';
         zoom_fixes = show_fixes;
         plot_signals = 4;
         config_screen(120);
      }
      else if(first_key == '&') { // &X command - change max voltage
         if(luxor) return help_exit(c,99);
         sprintf(edit_buffer, "%f", user_max_volts);
         start_edit('5', "Enter maximum EFC control voltage (ESC ESC to abort):"); 
         return 0;
      }
      else if(first_key == '$') {  // $x command - extra large screen
         new_screen(c);
         return 0;
      }
      else if(first_key == 'g') { // GX command - show dops
         if(luxor) return help_exit(c,99);
         plot_dops = toggle_value(plot_dops);
         if(plot_dops) request_sat_list();
         redraw_screen();
      }
      else if(first_key == 0) {  // X command - 1 hr/division
view_all_data = 0;
         new_view();
      }
      else return help_exit(c,99);
   }
   else if(c == 'y') {   // toggle view_interval and set plot area to 24 (or 12) divisions
      if(first_key == ESC_CHAR) {  // ESC y command - exit program
         kbd_exit();
         return 1;  // ESC = exit
      }
      else if(first_key == 'g') { // GY command - show filters
         if(luxor) return help_exit(c,99);
         plot_filters = toggle_value(plot_filters);
         redraw_screen();
      }
      else if(first_key == 'w') {  // WY command - open/close raw receiver data log file
         if(raw_file) {
            strcpy(edit_buffer, "Y");
            sprintf(out, "Close raw receiver data log file (Y/N)?  (ESC ESC to abort):");
         }
         else if(raw_name[0]) {
            strcpy(edit_buffer, raw_name);
            sprintf(out, "Enter name of raw receiver data log file to write  (ESC ESC to abort):");
         }
         start_edit(RAW_LOG_CMD, out);
         return 0;
      }
      else if(first_key == 'z') {  // ZY command - zoom all maps
         if(rcvr_type == NO_RCVR) return sure_exit();
         change_zoom_config(26);
         zoom_screen = 'Y';
         zoom_fixes = show_fixes;
         plot_signals = 4;
         config_screen(120);
      }
      else if(first_key == 0) {  // Y command - set day plot mode
view_all_data = 0;
         if(day_plot) { day_plot = 0; view_interval = 2L;}
         else {
            day_plot = day_size;
            if     (SCREEN_WIDTH > 1280) day_plot = 26;
            else if(SCREEN_WIDTH > 1024) day_plot = 26;
            else if(SCREEN_WIDTH > 800)  day_plot = 25;
            else                         day_plot = 26;
         }
         config_screen(125);
         new_view();
      }
      else return help_exit(c,99);
   }
   else if(c == 'z') {  // toggle floating graph zero for dac and temp
      if(first_key == 'g') {  // GZ command - show big time
         plot_digital_clock = toggle_value(plot_digital_clock);
         if(ebolt) {  // adjust ebolt sat count for time clock display
            last_ebolt = (-4);
            saw_ebolt();
         }
         config_screen(126);
      }
      else if(luxor && (first_key == 'p')) {  // PZ command = load high voltage cutoff
         sprintf(edit_buffer, "%f", auxv_lvc);
         start_edit(PZ_CMD, "Enter the AUXV low voltage cutoff in volts.  (ESC ESC to abort):");
         return 0;
      }
      else if(first_key == 't') {  // TZ command - select time zone
         if(time_zone_set) {
            if     (time_zone_seconds) sprintf(edit_buffer, "%d:%02d:%02d%s", time_zone_hours, IABS(time_zone_minutes), IABS(time_zone_seconds),std_string);
            else if(time_zone_minutes) sprintf(edit_buffer, "%d:%02d%s", time_zone_hours, IABS(time_zone_minutes), std_string);
            else                       sprintf(edit_buffer, "%d%s", time_zone_hours, std_string);
            if(dst_string[0]) {
               strcat(edit_buffer, "/");
               strcat(edit_buffer, dst_string);
            }
            edit_info1 = "String can also be in Linux format (like CST6CDT)";
            start_edit('Z', "Enter time zone string (like -6:00CST/CDT)  (ESC CR to reset  ESC ESC to abort:");
         }
         else {
            edit_buffer[0] = 0;
            edit_info1 = "String can also be in Linux format (like CST6CDT)";
            start_edit('Z', "Enter time zone string (like -6:00CST/CDT)  ESC to abort:");
         }
         return 0;
      }
      else if(first_key == 'w') {  // WZ command - Output az/el signal level file
         if(luxor) return help_exit(c,99);
         sprintf(edit_buffer, "%s.sig", unit_file_name);
         sprintf(out, "Enter file to write current AZ/EL/%s info to (ESC ESC to abort):", level_type);
         start_edit('%', out);
         return 0;
      }
      else if(first_key == '$') {  // $z command - Oprah sized screen
         new_screen(c);
         return 0;
      }
      else if(first_key == '!') {    // !Z command - reset parser
         tsip_sync = 0;
         tsip_wptr = 0;
         tsip_rptr = 0;
         init_messages(110);
         redraw_screen();
      }
      else if(first_key == 0) {  // z command - zoom menu
         if(zoom_screen) {
            zoom_screen = 0;
            change_zoom_config(0);
            config_screen(127);
         }
         else {
            if(are_you_sure(c) != c) return 0; 
         }
         // the next keystroke will select the data to write
      }
      else return help_exit(c,9);
   }
   else if(c == '~') {
      if(first_key == 'g') {  // g~ command - edit color palette
         edit_buffer[0] = 0;
         edit_info1 = "Enter color number to change (0 .. 15) followed by R G B values (0..255)";
         start_edit(PALETTE_CMD, "Edit color palette entry");
         return 0;
      }
//    else return help_exit(c,99);
   }
   else if(c == '}') {  // } command - grow plot
      if(first_key == 0) {
         grow_plot();
         draw_plot(1);
      }
      else return help_exit(c,99);
   }
   else if(c == '{') {  // } command - shrink plot
      if(first_key == 0) {
         shrink_plot();
         draw_plot(1);
      }
      else return help_exit(c,99);
   }
   else if(c == '+') {  // + command 
      if(first_key == 0) {
         if(last_was_mark) {  // clear numeric marker
            last_was_mark = 0;
            val = last_q_place;
            // center point on screen
            val -= ((((long)PLOT_WIDTH/2L)*view_interval)/(long)plot_mag);
            if(last_q_place < plot_q_out) val += plot_q_size;
            zoom_review(val, 1);
            draw_plot(1);
         }
         else {
            move_plot_up();
            draw_plot(1);
         }
      }
      else if(first_key == 'g') {  // g+ command - turn on all plots
         for(i=NUM_PLOTS+DERIVED_PLOTS-1; i>=0; i--) {
            if(plot[i].show_plot == 0) toggle_plot(i);
         }
         plot_adev_data = 1;
         plot_sat_count = 1;
      }
      else return help_exit(c,99);
   }
   else if(c == '-') {  // - command
      if(first_key == 0) {
         if(last_was_mark) {  // clear numeric marker
            mark_q_entry[last_was_mark-'0'] = 0;
            last_was_mark = 0;
            draw_plot(1);
         }
         else {
            move_plot_down();
            draw_plot(1);
         }
      }
      else if(first_key == 'g') {   // g- command - hide all plots
         for(i=NUM_PLOTS+DERIVED_PLOTS-1; i>=0; i--) {
            if(plot[i].show_plot) toggle_plot(i);
         }
         plot_adev_data = 0;
         plot_sat_count = 0;
      }
      else return help_exit(c,99);
   }
   else if(c == '=') { // = command - assign next unused marker
      if(first_key == 0) {
         if(set_next_marker()) return sure_exit();
      }
      else return help_exit(c,99);
   }
   else if(c == '!') {  // ! command - reset unit, etc
      if(are_you_sure(c) != c) return 0;
      // the next keystroke will select the reset type
   }
   else if(c == '&') {  // & command - edit osc param
      if(luxor) {
         if(first_key == 'w') { // W& command - write luxor cal data script file
            strcpy(edit_buffer, "LUXCAL.SCR");
            start_edit('w', "Enter name of calibration data script file to write (ESC ESC to abort):");
            dump_type = 'c';
            return 0;
         }
         if(are_you_sure(c) != c) return 0; 
         else                     return sure_exit();
      }

      if(osc_params == 1) osc_params = 0; 
      else                osc_params = 1;   // replace sat info table with osc parameter display
      redraw_screen();
      if(are_you_sure(c) != c) {
         user_time_constant = time_constant; 
         user_damping_factor = damping_factor;
         user_osc_gain = osc_gain;
         user_min_volts = min_volts;
         user_max_volts = max_volts;
         user_min_range = min_dac_v;
         user_max_range = max_dac_v;
         user_jam_sync = jam_sync;
         user_max_freq_offset = max_freq_offset;
         user_initial_voltage = initial_voltage;
         if(process_com == 0) {
            show_satinfo();
         }
         return 0;
      }
      else if(process_com == 0) {
         show_satinfo();
      }
      else {
//----   osc_params = 0;
      }
      redraw_screen();
   }
   else if(c == '/') {  
      if(first_key == 'g') { // G/ command - set statistic for all plots
         all_plots = 1;
         edit_buffer[0] = 0;
         edit_info1 =    "                             mi)N   ma)X   sP)an";
         start_edit('~', "Enter statistic to display:  A)vg   R)ms   S)td dev   V)ar   <cr>=hide");
         return 0;
      }
      else if(first_key == 0) {
         no_auto_erase = 1;
#ifdef WINDOWS
         strcpy(edit_buffer, "/");
#else  // __linux__  __MACH__ 
         strcpy(edit_buffer, "-");
#endif
         edit_info1 = "Options must begin with a '-' or '/' and be separated by spaces.";
         start_edit('-', "Enter one or more command line options to process (ESC ESC) to abort:");
         return 0;
      }
      else return help_exit(c,99);
   }
   else if(c == '\\') {   // \ command - dump screen image
      if(first_key == 0) {
         dump_screen(invert_dump, 0, unit_file_name);
      }
      else return help_exit(c,99);
   }
   else if(c == '$') {   // $ command - set screen size
      if(are_you_sure(c) != c) return 0;
      // the next char will select the screen size
   }
   else if(c == '%') {   // % command - goto next holdover/error event
      if(first_key == 0) {
         val = 0;
         if(plot_skip_data)     val |= TIME_SKIP;
         if(plot_holdover_data) val |= HOLDOVER;
         if(plot_temp_spikes)   val |= TEMP_SPIKE;
         if(val == 0)           val |= (HOLDOVER | TIME_SKIP | TEMP_SPIKE);
         goto_event((u16) val);
      }
      else return help_exit(c,99);
   }
   else if(c == ';') {   // ; command - script file text comment to end of line
      if(script_file) skip_comment = 1;
      else return help_exit(c,10);
   }
   else if(c == '#') {   // # command - script file text comment to end of line
      if(script_file) skip_comment = 1;
      else return help_exit(c,10);
   }
   else if((c >= '0') && (c <= '9')) {
      if((first_key == 'p') && (c == '1')) {  // p1 command - set 1pps mode
         if(luxor) return help_exit(c,99);
         set_pps_mode(0x02);
         request_pps_mode();
         if(res_t) save_segment(0xFF, 2);
      }
      else if((first_key == 'p') && (c == '2')) {  // p2 command - set pp2s/100 pps mode
         if(luxor) return help_exit(c,99);
         set_pps_mode(0x82);
         request_pps_mode();
         if(res_t) save_segment(0xFF, 3);
      }
      else if((first_key == 'p') && (c == '3')) {  // p3 command - set 10000 pps mode
         if(luxor) return help_exit(c,99);
         set_pps_mode(0x83);
         request_pps_mode();
         if(res_t) save_segment(0xFF, 4);
      }
      else if(first_key == 'g') {  // G1 .. G9 commands - select auxiliary plots
         if(c == '0') val = FIRST_EXTRA_PLOT + 10 - 1;
         else         val = FIRST_EXTRA_PLOT + (c-'0') - 1;
         if((val == 0) || (val >= NUM_PLOTS+DERIVED_PLOTS)) {
            sprintf(out, "Plot %c is not supported", c);
            edit_error(out);
         }
         else {
            edit_plot((int) val, c);
            return 0;
         }
      }
      #ifdef PRECISE_STUFF
         else if(first_key == 's') {
            if(luxor) return help_exit(c,99);
            if     (c == '2') do_fixes(3);     // S2 command - 2D fixes
            else if(c == '3') do_fixes(4);     // S3 command - 3D mode
            else if(c == '4') do_fixes(2);     // S4 command - undocumented mode 2
            else              do_fixes(c-'0'); // S# command - set receiver mode to #
         }
      #endif
      else return help_exit(c,99);
   }
   else if(c == '?') {
      keyboard_cmd = 1;
      command_help("", "", help_path);
      keyboard_cmd = 0;
   }
   else {  // help mode
      return help_exit(c,11);
   }

   return sure_exit();
}


int scroll_char(int c)
{
   if((c >= '0') && (c <= '9')) return 1;
   if(c == '=') return 1;
   if(c == '-') return 1;
   if(c == '+') return 1;
   if(c == '%') return 1;

   if(c == '[') return 1;
   if(c == ']') return 1;

   if(c == '<') return 1;
   if(c == '>') return 1;

   if(c == UP_CHAR) return 1;   
   if(c == DOWN_CHAR) return 1; 
   if(c == PAGE_UP) return 1;   
   if(c == PAGE_DOWN) return 1; 
   if(c == LEFT_CHAR) return 1; 
   if(c == RIGHT_CHAR) return 1;
   if(c == HOME_CHAR) return 1; 
   if(c == END_CHAR) return 1;  

   return 0;
}

int do_kbd(int c)
{
int fix_flag;
   // This routine processes a keystroke character.
   //
   // If variable "first_key" is set, this key stroke is the second keystroke
   // of the two character command that started with character "first_key".

   if(sound_alarm) {    // any keystroke turns off alarm clock
      sound_alarm = 0;  
      return 0;
   }

   if(disable_kbd > 1) return 0;  // nothing gets by

   fix_flag = 0;
   if((zoom_screen || (un_zoom == ZOOM_ADEVS)) && (first_key == 0)) {  // any keystroke restores zoomed screen
//    if((c != 'z') && (c != '\\')) {     // unless it is one of these keys
      if((zoom_screen == 'P') && scroll_char(c)) ;
      else if(c != '\\') {     // unless it is one of these keys
         if(zoom_screen == 'L') {
            fix_flag = zoom_fixes;
         } 

         if(zoom_screen) {
            zoom_screen = 0;
            change_zoom_config(0);
         }
         if(un_zoom == ZOOM_ADEVS) {
            all_adevs = 0;
            plot_adev_data = old_plot_adevs;
            un_zoom = UN_ZOOM;
         }
         show_fixes = 0;
         if(rcvr_type != NO_RCVR) show_fixes = zoom_fixes;  // zzzzzz
if(fix_flag) {
   show_fixes = plot_lla = 1;
}
         change_fix_config(show_fixes);
         plot_lla = 0;
if(fix_flag) plot_lla = 1;
         no_redraw = 1;
         if((c == ' ') || (c == 0x0D) || (c == ESC_CHAR)) {
            no_redraw = 0;
         }
         config_screen(128);
         erase_screen();
         no_redraw = 0;

         if((c == ' ') || (c == 0x0D) || (c == ESC_CHAR)) { // don't bring up help screen
            return 0;
         }
      }
   }

   if(disable_kbd) {         // most of the keyboard is disabled
      last_was_mark = 0;
      if((first_key == 0) && (c != ESC_CHAR)) return 0;
      if((c >= 0x0100) || (c == '[') || (c == ']')) {  // cursor movement keys, etc
         c = 0;
         do_review(c);       // stop review
         return sure_exit(); // prevents spurious scroll mode header
      }
   }

   if(getting_string) { // we are currently building a parameter value string
      last_was_mark = 0;
      return build_string(c);  // add the keystroke to the line
   }
   else if(getting_plot) {  // we are in the plot control sub-menu
      if(change_plot_param(c, 0)) return 0;
   }
   else if((c >= 0x0100) || (c == '<') || (c == '>') || (c == '[') || (c == ']') || (c == '@')) {  
      // cursor movement keys, etc
      // @ command - zoom to marked queue entry (invoked from mouse click or keyboard)
      last_was_mark = 0;
      if((first_key == 0) && (text_mode == 0)) {
         if(c == '@') goto_mark(0);  // center plot on marked place 
         else         do_review(c);  // scrolling around in the plot data
      }
      else {
         if(c >= 0x0100) cmd_error(0);  //!!!!
         else            cmd_error(c);
      }
   }
   else if(c == ESC_CHAR) {  // ESC key exits program or aborts key sequence
      last_was_mark = 0;
      if(osc_params && (first_key != '&')) {  // ESC exits osc param display
         osc_params = 0;
         need_redraw = 8989;
      }
      else if(review_mode && (pause_data == 0)) {
         if(text_mode == 2) {  // displaying full screen help
            text_mode = 0;
            need_redraw = 6677;
         }
         else {
            c = 0;
            do_review(c);  // stop review
         }
      }
      else if(first_key == ESC_CHAR) {
         if(esc_esc_exit) {  // allow ESC ESC to end the program
            kbd_exit();
            return 1;
         }
      }
      else {
         if(are_you_sure(c) != c) return 0;
      }
   }
   else if((first_key == 0) && (c >= '0') && (c <= '9')) {  // plot markers
      kbd_marker(c);
   }
   else {  // normal keystroke command
      if(c < 0x0100) c = tolower(c);
      if((c != '-') && (c != '+')) last_was_mark = 0;

      // break up the keyboard processing big 'if' statement 
      // into two parts so the compiler heap does not overflow
      if((c >= 'a') && (c <= 'o')) c = kbd_a_to_o(c);
      else                         c = kbd_other(c);
      return c;
   }

   return sure_exit();
}

//
// 
//  Command line processor
//
//
void save_cmd_bytes(char *arg)
{
u16 val;
char c;
int i;
u08 saw_digit;

   val = 0x00;
   saw_digit = 0;
   for(i=1; i<=(int)strlen(arg); i++) {
      c = toupper(arg[i]);
      if((c >= '0') && (c <= '9')) {  // build hex value
         val = (val * 16) + (c-'0');
         saw_digit = 1;
         continue;
      }
      else if((c >= 'A') && (c <= 'F')) {  // build hex value
         val = (val * 16) + (c-'A'+10);
         saw_digit = 1;
         continue;
      }
      else if(saw_digit) {  // end of hex value
         if(arg[0] == '=') {  // add byte to list of bytes to send during tbolt init
            if(user_init_len < USER_CMD_LEN) {
               user_init_cmd[user_init_len++] = (u08) val;
            }
         }
         else if(arg[0] == '$') {  // add byte to list of bytes to send every second
            if(user_pps_len < USER_CMD_LEN) {
               user_pps_cmd[user_pps_len++] = (u08) val;
            }
         }

         val = 0x00;     // setup for next hex value
         saw_digit = 0;
      }

      if(c == ',') continue;   // values can be separated by spaces or commas
      else if(c == ' ') continue;
      else break;  // anything else ends the list of hex values
   }
}

u08 toggle_option(u08 val, u08 sw)
{
   if     ((sw == '1') || (sw == 'y')) return 1;
   else if((sw == '0') || (sw == 'n')) return 0;
   else return (val & 1) ^ 1;
}


void set_watch_name(char *s)
{
unsigned i, j;
int n;
char c;

   plot_watch = 1;
   config_screen(129);
   n = 0;
   j = 0;
   watch_name[j] = watch_name2[j] = 0;
   for(i=0; i<strlen(s); i++) {
      c = s[i];   
      if(c == 0) break;
      if(c == '/') {
         ++n;
         j = 0;
         continue;
      }

      if(c == '_') c = ' ';
      if(n == 0) {
         watch_name[j++] = c;
         watch_name[j] = 0;
         if(j >= (sizeof(watch_name)-1)) break;
      }
      else if(n == 1) {
         watch_name2[j++] = c;
         watch_name2[j] = 0;
         if(j >= (sizeof(watch_name2)-1)) break;
      }
   }
}


void config_utc_mode(int set_utc_ofs)
{
   // this routine configures the time mode for receivers that do not support
   // operating in GPS time mode.

   if(set_gps_mode) {
      set_utc_mode = 0;
      time_flags |= 0x0001;
      if(luxor) have_timing_mode = 1;
   }
   else if(set_utc_mode) {
      set_gps_mode = 0;
      time_flags &= (~0x0001);
      if(luxor) have_timing_mode = 1;
   }
   else {
      time_flags = 0x0000;
      if(luxor) time_flags = 0x0000;
      set_utc_mode = 1;
      set_gps_mode = 0;
   }

   // here we attempt to estimate the leapsecond count if it is needed and
   // the user did not specify one. 

   if(!user_set_utc_ofs && set_utc_ofs) { 
      get_clock_time();
      if(clk_year == 2017) utc_offset = 18;  // leapseconds change to 18 in 2017
      else if(clk_year == 2016) utc_offset = 17;
      else utc_offset = (int) (43.216 + (double)(clk_year-2061)*0.567);
      have_utc_ofs = (-1);
   }
}



void config_msg_ofs()
{
   //  This routine sets the end-of-message arrival time offset vs the timing
   //  message time values.

   if(user_set_tsx) {
   }
   else if(rcvr_type == ACRON_RCVR) { 
      time_sync_offset = (600.0);  // !!!!!! we need to measure this and set a proper value
   }
   else if(rcvr_type == GPSD_RCVR) {
      time_sync_offset = (TIME_SYNC_AVG-27.7);
   }
   else if(rcvr_type == MOTO_RCVR) {
      if(moto_chans == 12) time_sync_offset = (208.5); 
      else if(moto_chans == 8) time_sync_offset = (262.7);  // average of 8 chan units
      else time_sync_offset = (249.15);  // average of different models
   }
   else if(rcvr_type == NMEA_RCVR) {
      time_sync_offset = (222.0);  // typical NMEA value
   }
   else if(rcvr_type == NO_RCVR) { 
      time_sync_offset = (0.0);
   }
   else if(rcvr_type == NVS_RCVR) {
     time_sync_offset = 40.7;
   }
   else if(rcvr_type == SCPI_RCVR) {
      time_sync_offset = (-964.7); // Z3801:-979.8  Z3812:-949.6
   }
   else if(rcvr_type == SIRF_RCVR) {
      time_sync_offset = (417.0);
   }
   else if(rcvr_type == STAR_RCVR) {
      time_sync_offset = (215.0);
   }
   else if(rcvr_type == TSIP_RCVR) {
      if(res_t) {
         if(res_t == RES_T) time_sync_offset = (99.4);
         else if(res_t == RES_T_SMT) time_sync_offset = (393.2);
         else time_sync_offset = (246.3); // !!!!! we need actual timings
      }
      else if(tsip_type == STARLOC_RCVR) {
         time_sync_offset = (266.0);
      }
      else {
         time_sync_offset = (53.00);
      }
   }
   else if(rcvr_type == UBX_RCVR) {
      time_sync_offset = (197.3);
   }
   else if(rcvr_type == UCCM_RCVR) {
      time_sync_offset = (231.4);
   }
   else if(rcvr_type == VENUS_RCVR) {
      if(baud_rate == 9600) {
         if(saw_timing_msg) time_sync_offset = (208.3);
         else               time_sync_offset = (225.3);
      }
      else {  // 115200 baud
         if(saw_timing_msg) time_sync_offset = (153.8);
         else               time_sync_offset = (170.8);
      }
   }
   else if(rcvr_type == ZODIAC_RCVR) {
      time_sync_offset = (1238.85);  // Pico = 1232.2, Jupiter-T = 1245.5
   }
   else {
      time_sync_offset = TIME_SYNC_AVG;
   }
}


int adevs_active(int check_enable)
{
   // return true if we might have adevs to display
   //   0x01 : pps_offset available
   //   0x02 : osc_offset available
   //   0x80 : flags that bits 0x01 and 0x02 are guessed

   if(check_enable) {
      if(adev_period <= 0.0) return 0;
   }

   if(jitter_adev) return 0x03;

   if(luxor) return 0;
   if(rcvr_type == ACRON_RCVR) return 0;
   if(rcvr_type == NMEA_RCVR) return 0;
   if(rcvr_type == NO_RCVR) return 0;
   if(rcvr_type == STAR_RCVR) return 0;
   if(rcvr_type == VENUS_RCVR) return 0;
   if(!TIMING_RCVR && !GPSDO) return 0;

   if(have_pps_offset && have_osc_offset) return 0x03;
   else if(have_pps_offset) return 0x01;
   else if(have_osc_offset) return 0x02;

   return 0x83;
}


int status_second(int seconds)
{
   // check if "seconds" is within the time interval where a long status message
   // is comming in.  If so, return next possible valid second, else return 0

   if(rcvr_type == ACRON_RCVR) {
      if     (seconds == (SCPI_STATUS_SECOND+1)) return SCPI_STATUS_SECOND+4; 
      else if(seconds == (SCPI_STATUS_SECOND+2)) return SCPI_STATUS_SECOND+4; 
      else if(seconds == (SCPI_STATUS_SECOND+3)) return SCPI_STATUS_SECOND+4; 
   }
   else if(rcvr_type == SCPI_RCVR) {
      if(scpi_type == LUCENT_TYPE) {
         if     (seconds == (SCPI_STATUS_SECOND+1)) return SCPI_STATUS_SECOND+7; 
         else if(seconds == (SCPI_STATUS_SECOND+2)) return SCPI_STATUS_SECOND+7; 
         else if(seconds == (SCPI_STATUS_SECOND+3)) return SCPI_STATUS_SECOND+7; 
         else if(seconds == (SCPI_STATUS_SECOND+4)) return SCPI_STATUS_SECOND+7; 
         else if(seconds == (SCPI_STATUS_SECOND+5)) return SCPI_STATUS_SECOND+7; 
         else if(seconds == (SCPI_STATUS_SECOND+6)) return SCPI_STATUS_SECOND+7; 
      }
      else {
         if     (seconds == (SCPI_STATUS_SECOND+1)) return SCPI_STATUS_SECOND+6; 
         else if(seconds == (SCPI_STATUS_SECOND+2)) return SCPI_STATUS_SECOND+6; 
         else if(seconds == (SCPI_STATUS_SECOND+3)) return SCPI_STATUS_SECOND+6; 
         else if(seconds == (SCPI_STATUS_SECOND+4)) return SCPI_STATUS_SECOND+6; 
         else if(seconds == (SCPI_STATUS_SECOND+5)) return SCPI_STATUS_SECOND+6; 
      }
   }
   else if(rcvr_type == STAR_RCVR) {
      if(star_type == NEC_TYPE) ;  // NEC device at 115,200 baud does not skip time stamps when sending sat info
      else if(seconds == (SCPI_STATUS_SECOND+1)) return SCPI_STATUS_SECOND+5;
      else if(seconds == (SCPI_STATUS_SECOND+2)) return SCPI_STATUS_SECOND+5; 
      else if(seconds == (SCPI_STATUS_SECOND+3)) return SCPI_STATUS_SECOND+5; 
      else if(seconds == (SCPI_STATUS_SECOND+4)) return SCPI_STATUS_SECOND+5; 
   }
   else if(rcvr_type == UCCM_RCVR) {
      if     (seconds == (SCPI_STATUS_SECOND+1)) return SCPI_STATUS_SECOND+4; 
      else if(seconds == (SCPI_STATUS_SECOND+2)) return SCPI_STATUS_SECOND+4; 
      else if(seconds == (SCPI_STATUS_SECOND+3)) return SCPI_STATUS_SECOND+4; 
   }

   return 0;
}


void config_rcvr_type(int set_baud)
{
int prn;

   // setup the program to work with the specified receiver type

if(rcvr_type == LUXOR_RCVR) {
   detect_rcvr_type = 0;
   set_baud = 1;
}

   com_timeout = COM_TIMEOUT;
   if(detect_rcvr_type) ;
   else if(set_baud == 0) ;
   else if(user_set_baud == 0) {
      baud_rate = 9600;
      data_bits = 8;
      parity = 0;
      stop_bits = 1;
   }

   last_time_msec = GetMsecs();  // initialize ellapsed millisecond counter
   this_time_msec = last_time_msec + 1000.0;

   max_sat_count = 8;
   if(max_sat_count > max_sat_display) max_sat_count = max_sat_display;
   force_mode_change = 0;

   find_endian();

   user_set_rcvr = 1;
   have_gpgga = 0;
   have_gprmc = 0;
   have_gpgns = 0;
   need_msg_init = 1;
   strcpy(uccm_led_msg, "UNKNOWN STATE");

   ebolt = 0;

   pkt_end1 = pkt_end2 = (-1);
   pkt_start1 = pkt_start2 = (-1);
   packet_count = 0;
   first_msg = 1;        
   first_request = 1;

   have_antenna = 0;      // config what types of data the receiver can send
   have_osc_age = 0;
   have_saved_posn = 0;
   have_tracking = 0;
   have_sat_azel = 0;
   have_leap_info = 0;
   have_leap_days = 0;
   have_scpi_hex_leap = 0;
   have_jd_leap = 0;
   leap_days = (-1);
   guessed_leap_days = ' ';
   have_moto_Gj = 0;
   have_op_mode = 0;
   have_almanac = 0;
   have_critical_alarms = 0;
   have_eeprom = 1;
   have_gps_status = 0;
   have_gnss_mask = 0;
   default_gnss = MIXED;
   have_cable_delay = 0;
   have_rf_delay = 0;
   have_pps_delay = 0;
   have_temperature = 0;
   have_star_perf = 0;
   have_star_atdc = 0;
   have_star_input = 0;
   saw_star_time = 0;
   saw_uccm_dmode = 0;
   have_gpsdo_ref = 0;
   have_uccm_loop = 1;
   have_pullin = 0;
   got_timing_msg = 0;
   have_lifetime = 0;
   have_last_stamp = 0;
   have_nav_rate = 0;
   nav_rate = 1.0;
   have_pps_rate = 0;
   have_pps_enable = 0;
   have_pps_polarity = 0;
   pps_enabled = 0;
   have_osc_polarity = 0;
   have_pps_mode = 0;
   have_tc = 0;
   have_damp = 0;
   have_gain = 0;
   have_initv = 0;
   have_minv = 0;
   have_maxv = 0;
   have_dac_range = 0;
   have_jam_sync = 0;
   have_freq_ofs = 0;
   have_osc_params = 0;
   have_tfom = 0;
   have_ffom = 0;
   have_dac = 0;
   have_osc_offset = 0;
   have_pps_offset = 0;
   have_count = 0;
   have_sawtooth = 0;
   have_heading = 0;
   have_speed = 0;
   have_dops = 0;
   have_filter = 0;
// last_utc_ofs = 0;
   if(!user_set_utc_ofs) have_utc_ofs = 0;
   utc_offset_flag = 0;
   have_sirf_pps = 0;
   have_tow = 0;
   have_week = 0;
   rolled = 0;
   if(!user_set_rollover) rollover = 0.0;
   have_doppler = 0;
   have_phase = 0;
   have_range = 0;
   have_bias = 0;
   have_accu = 0;
   have_kalman = 1;
   saw_kalman_on = 0;
   have_traim = 0;
   have_timing_mode = 0;
   have_build = 0;
   have_pps_duty = 0;
   have_pps_freq = 0;
   use_traim_lla = 0;
   keep_lla_plots = 0;
   have_amu = 0;
   have_el_mask = 0;
   have_scpi_test = 0;
   have_rcvr_mode = 0;
   last_rcvr_mode = 0;
   first_request = 1;
   adjust_scpi_ss = 0;
   ee_write_count = 0;
   if(user_set_temp_filter) ;
   else undo_fw_temp_filter = 0;

   saw_version = 0;
   saw_mini = 0;
   saw_ntpx = 0;
   saw_nortel = 0;
   saw_icm = 0;
   saw_gpsdo = 0;
   saw_gpsd_pps = 0;
   saw_gpsd_pre = 0;
   saw_diff_change = 0;

   ubx_sw[0] = 0;
   ubx_hw[0] = 0;
   ubx_rom[0] = 0;
   ubx_fw_ver = 0.0F;
   saw_timing_msg = 0;
   saw_ubx_tp5 = 0;
   saw_ubx_tp = 0;

   nmea_type = 0;

   last_hours = last_log_hours = 99;
   leap_pending = 0;
   minor_alarms = 0;
   critical_alarms = 0;

   venus_hold_mode = 0;

   sats_enabled = 0xFFFFFFFF;
   update_disable_list(sats_enabled);
   for(prn=0; prn<=MAX_PRN; prn++) {  // extended sat PRNs
      sat[prn].disabled = 0;
      sat[prn].level_msg = 0;
      sat[prn].tracking = 0;
      max_sat_db[prn] = 0;
   }
   amu_mode = 0;
   tbolt_e = 0;
   unit_name[0] = 0;

   if(rcvr_type != TSIP_RCVR) {
      if(!user_set_dops) plot_dops = 1;
      res_t = 0;
   }

   if(luxor) unit_file_name = "luxor";
   else      unit_file_name = "tbolt";

// min_sig_db = 30;      // low sig level threshold for sig level map
// sig_level_step = 2;   // sig level map signal steo size

   min_sig_db = 20;      // low sig level threshold for sig level map
   sig_level_step = 3;   // sig level map signal steo size
   need_sunrise = 1;

   if(rcvr_type == ACRON_RCVR) { 
      unit_file_name = "acron";
      config_lla_plots(1, 1);

      if(detect_rcvr_type) ;
      else if(set_baud == 0) ;
      else if(user_set_baud == 0) {
         baud_rate = 300;
         data_bits = 8;
         parity = NO_PAR;  
         stop_bits = 2;
      }
      com_timeout = (3000.0); 

      config_utc_mode(1);
      pps_enabled = 0;

      plot_azel = 1;
      plot_signals = 0;
      shared_plot = 0;
      plot_watch = 1;
      plot_digital_clock = 1;

      plot[PPS].show_plot = 0;
      plot[DAC].show_plot = 0;
      rcvr_mode = RCVR_MODE_HOLD;
   }
   else if(rcvr_type == GPSD_RCVR) {
      unit_file_name = "gpsd";
      have_timing_mode = 1;
      config_lla_plots(1, 2);

      if(user_set_dac_plot == 0) plot[DAC].show_plot = 0;   // disable GPSDO related plots
      if(user_set_osc_plot == 0) plot[OSC].show_plot = 0;
//    if(user_set_pps_plot == 0) plot[PPS].show_plot = 0;

      have_kalman = 0;

      config_utc_mode(1);

      pkt_end1 = 0x0D;          // used to pretty-print the data stream log file
      pkt_end2 = 0x0A;
   }
   else if(rcvr_type == LUXOR_RCVR) {
      tsip_type = TBOLT_RCVR;
      luxor = 20;
      max_sats = max_sat_display = 0;
      config_sat_rows();

      if(detect_rcvr_type) {
         baud_rate = 9600;
         data_bits = 8;
         parity = ODD_PAR;  
         stop_bits = 1;
      }
      else if(set_baud == 0) ;
      else if(user_set_baud == 0) {
        baud_rate = 9600;
        data_bits = 8;
        parity = LUXOR_PAR;  
        stop_bits = 1;
      }

time_flags = 0x0000;
      config_utc_mode(1);
      plot_azel = 0;

      config_luxor_plots();
      config_screen(6578);
      pkt_end1 = DLE;
      pkt_end2 = ETX;
   }
   else if(rcvr_type == MOTO_RCVR) {
      unit_file_name = "moto";
      com_timeout = (15000.0);  // need extended com timeouts for self test command

      config_lla_plots(1, 1);

      plot[SIX].show_plot = 0;  // dop
      plot[SIX].float_center = 1;

      have_kalman = 0;
      have_temperature = 1; 
      config_rcvr_plots();

      pkt_end1 = 0x0D;          // used to pretty-print the data stream log file
      pkt_end2 = 0x0A;
   }
   else if(rcvr_type == NMEA_RCVR) {
      unit_file_name = "nmea";
      have_timing_mode = 1;
      config_lla_plots(1, 2);

      plot[SIX].show_plot = 1;   // dop
      plot[SIX].float_center = 1;

      if(user_set_dac_plot == 0) plot[DAC].show_plot = 0;   // disable GPSDO related plots
      if(user_set_osc_plot == 0) plot[OSC].show_plot = 0;
      if(user_set_pps_plot == 0) plot[PPS].show_plot = 0;

      have_kalman = 0;

      config_utc_mode(1);

      pkt_end1 = 0x0D;
      pkt_end2 = 0x0A;
   }
   else if(rcvr_type == NO_RCVR) { 
process_com = 0;
      unit_file_name = "clock";
      config_lla_plots(1, 1);

      config_utc_mode(1);
      pps_enabled = 0;

      plot_azel = 0;
      plot_watch = 1;
      plot_digital_clock = 1;
   }
   else if(rcvr_type == NVS_RCVR) {
      unit_file_name = "nvs";
      config_lla_plots(1, 1);

      if(detect_rcvr_type) ;
      else if(set_baud == 0) ;
      else if(user_set_baud == 0) {
         baud_rate = 115200;
         data_bits = 8;
         parity = ODD_PAR;  
         stop_bits = 1;
      }
      default_gnss = (GPS | GLONASS | SBAS);

      config_rcvr_plots();

      pkt_end1 = DLE;
      pkt_end2 = ETX;
   }
   else if(rcvr_type == SCPI_RCVR) {
      unit_file_name = "scpi";
      config_lla_plots(1, 1);

      if(detect_rcvr_type) ;
      else if(set_baud == 0) ;
      else if(user_set_baud == 0) {
         if(scpi_type == LUCENT_TYPE) {
            baud_rate = 19200;  // use 19200:8:N:1
            data_bits = 8;
            parity = NO_PAR;         // odd
            stop_bits = 1;
         }
         else if((scpi_type != HP_TYPE) && (scpi_type != NORTEL_TYPE)) {  // Z3801A type receivers
            baud_rate = 19200;  // use 19200:7:O:1
            data_bits = 7;
            parity = ODD_PAR;         // odd
            stop_bits = 1;
         }
      }

      com_timeout = (25000.0);  // need extended com timeouts for survey and self test commands
      if(scpi_type == NORTEL_TYPE) {
         com_timeout = 3000.0;
         config_utc_mode(0);
         level_type = "SNR";
      }
      else if(scpi_type == LUCENT_TYPE) {
         min_sig_db = 30;      // low sig level threshold for sig level map
         sig_level_step = 2;   // sig level map signal steo size
      }

      Sleep(500);
      scpi_init(0);
      saw_gpsdo = 10;

      plot[DAC].units = "%";
      plot[DAC].scale_factor = 1.0;
      plot[DAC].ref_scale = 1.0;

      plot[PPS].scale_factor = 1.0; //1000
      plot[PPS].ref_scale = 1.0;
      plot[PPS].units = "ns";

      plot[OSC].plot_id = "UNC";
      if(user_set_osc_plot == 0) plot[OSC].show_plot = 1;
      plot[OSC].ref_scale = 1.0;    //1000.0;
      plot[OSC].scale_factor = 1.0; //1000.0;
      plot[OSC].units = "us";
      ppb_string = " us";
      ppt_string = " us";

      plot_adev_data = 0;

      plot[FOUR].plot_id = "TFOM";
      plot[FOUR].units = " ";
      plot[FOUR].ref_scale = 1.0F;
      plot[FOUR].show_plot = 0;
      plot[FOUR].float_center = 0;

      plot[FIVE].plot_id = "FFOM";
      plot[FIVE].units = " ";
      plot[FIVE].ref_scale = 1.0F;
      plot[FIVE].show_plot = 0;
      plot[FIVE].float_center = 0;

      plot[SIX].show_plot = 0;

      have_kalman = 0;
   }
   else if(rcvr_type == SIRF_RCVR) {
      unit_file_name = "sirf";
      config_lla_plots(1, 2);

      plot[SIX].show_plot = 0;  // dop
      plot[SIX].float_center = 1;

      have_kalman = 0;
      config_rcvr_plots();
      pps_enabled = 1;

      pkt_end1 = 0xB0;
      pkt_end2 = 0xB3;

      ppb_string = " ns";
      ppt_string = " ns";
   }
   else if(rcvr_type == STAR_RCVR) {
      if(star_type == 'N') unit_file_name = "nec";
      else                 unit_file_name = "star";
      have_timing_mode = 1;
      config_lla_plots(1, 1);

      if(detect_rcvr_type) ;
      else if(set_baud == 0) ;
      else if(user_set_baud == 0) {
         if(star_type == NEC_TYPE) {
            baud_rate = 115200;
            data_bits = 8;
            parity = 0;  
            stop_bits = 2;
         }
         else {
            baud_rate = 9600;
            data_bits = 8;
            parity = 0;  
            stop_bits = 2;
         }
      }
      com_timeout = (4000.0); 
      star_restart_ok = 1;

      have_kalman = 0;
      have_temperature = 1;
      config_rcvr_plots();

      if(user_set_temp_plot == 0) plot[TEMP].show_plot = 1;
      if(!user_set_dac_plot) plot[DAC].show_plot = 0;
      if(!user_set_osc_plot) plot[OSC].show_plot = 0;
      if(!user_set_pps_plot) plot[PPS].show_plot = 0;

      config_utc_mode(0);

      pkt_end1 = 0x0D;
      pkt_end2 = 0x0A;
   }
   else if(rcvr_type == TSIP_RCVR) {
      config_lla_plots(1, 1);

      if(tsip_type == STARLOC_RCVR) {
         if(user_set_temp_plot == 0) plot[TEMP].show_plot = 0;
         if(user_set_osc_plot == 0) plot[OSC].show_plot = 1;
         max_sat_display = 8;
if((sat_cols > 1) && (max_sat_display < 16)) max_sat_display = 16;

         if(!user_set_short) {  // DATUM sends 0's in the clock bias, ura, etc fields
            user_set_short = 1;
            max_sats = max_sat_display;  // used to format the sat_info data
            max_sat_count = max_sat_display;
            temp_sats = max_sat_display;
         }
         config_sat_rows();

         config_utc_mode(0);
      }
      else if(res_t) ;
      else {
         min_sig_db = 30;      // low sig level threshold for sig level map
         sig_level_step = 2;   // sig level map signal steo size
         if(user_set_temp_filter) ;
         else undo_fw_temp_filter = 1;
      }
      pkt_end1 = DLE;
      pkt_end2 = ETX;
   }
   else if(rcvr_type == UBX_RCVR) {
      unit_file_name = "ublox";
      config_lla_plots(1, 1);

      min_sig_db = 16;      // low sig level threshold for sig level map
      sig_level_step = 2;   // sig level map signal steo size

      plot[SIX].show_plot = 0;  // dop
      plot[SIX].float_center = 1;

      have_kalman = 0;
      default_gnss = (GPS | GLONASS | SBAS | QZSS);
      config_rcvr_plots();
      pkt_start1 = 0xB5;
      pkt_start2 = 0x62;
   }
   else if(rcvr_type == UCCM_RCVR) {
      unit_file_name = "uccm";
      config_lla_plots(1, 1);

      if(detect_rcvr_type) ;
      else if(set_baud == 0) ;
      else if(user_set_baud == 0) {
         baud_rate = 57600;
         data_bits = 8;
         parity = 0;  
         stop_bits = 1;
      }

      com_timeout = (25000.0);  // need extended com timeouts for survey and self test commands

      Sleep(500);
      scpi_init('u');
      saw_gpsdo = 20;

      plot[DAC].units = "%";
      plot[DAC].scale_factor = 1.0;
      plot[DAC].ref_scale = 1.0;

      plot[PPS].scale_factor = 1.0; //1000
      plot[PPS].ref_scale = 1.0;
      plot[PPS].units = "ns";

      plot[OSC].float_center = 1;   // uuuuuu

//    plot[OSC].plot_id = "UNC";
//    if(user_set_osc_plot == 0) plot[OSC].show_plot = 0;
//    plot[OSC].ref_scale = 1.0;    //1000.0;
//    plot[OSC].scale_factor = 1.0; //1000.0;
//    plot[OSC].units = "us";
//    ppb_string = " us";
//    ppt_string = " us";

      if(user_set_osc_plot == 0) plot[OSC].show_plot = 1;

      plot_adev_data = 0;

      plot[FOUR].plot_id = "TFOM";
      plot[FOUR].units = " ";
      plot[FOUR].ref_scale = 1.0F;
      plot[FOUR].show_plot = 0;
      plot[FOUR].float_center = 0;

      plot[FIVE].plot_id = "FFOM";
      plot[FIVE].units = " ";
      plot[FIVE].ref_scale = 1.0F;
      plot[FIVE].show_plot = 0;
      plot[FIVE].float_center = 0;

      plot[SIX].show_plot = 0;

      have_kalman = 0;

      config_utc_mode(0);

      rcvr_mode = RCVR_MODE_UNKNOWN;
      pkt_end1 = 0x0D;
      pkt_end2 = 0x0A;
   }
   else if(rcvr_type == VENUS_RCVR) {
      unit_file_name = "venus";
      if(detect_rcvr_type) ;
      else if(set_baud == 0) ;
      else if(user_set_baud == 0) {
         baud_rate = 115200;
         data_bits = 8;
         parity = 0;  
         stop_bits = 1;
      }
      default_gnss = (GPS | BEIDOU | SBAS | QZSS);
config_rcvr_plots();

      config_lla_plots(1, 2);

      plot[SIX].show_plot = 1;   // dop
      plot[SIX].float_center = 1;

      if(user_set_dac_plot == 0) plot[DAC].show_plot = 0;   // disable GPSDO related plots
      if(user_set_osc_plot == 0) plot[OSC].show_plot = 0;
      if(user_set_pps_plot == 0) plot[PPS].show_plot = 0;

      have_kalman = 0;
      pps1_freq = 1;

      config_utc_mode(0);

      pkt_start1 = 0xA0;
      pkt_start2 = 0xA1;
      pkt_end1 = 0x0D;
      pkt_end2 = 0x0A;
   }
   else if(rcvr_type == ZODIAC_RCVR) {
      unit_file_name = "jupiter";
      com_timeout = (25000.0);  // need extended com timeouts for reset and self test commands
      config_lla_plots(1, 1);

      plot[SIX].show_plot = 0;   // dop
      plot[SIX].float_center = 1;

plot[PPS].scale_factor = 1.0; //1000
plot[PPS].ref_scale = 1.0;
plot[PPS].units = "ns";

      have_kalman = 0;
      have_temperature = 0;
      config_rcvr_plots();
      pkt_start1 = 0xFF;
      pkt_start2 = 0x81;
   }

   config_msg_ofs();

   if(rcvr_type == LUXOR_RCVR) {
      init_com();
      Sleep(200);
      reset_luxor_wdt(0x01);
      set_luxor_time();
      reset_luxor_wdt(0x01);
   }
   else if(detect_rcvr_type) ;
   else if(set_baud == 0) ;
   else if(user_set_baud == 0) {
      init_com();
   }


   if((rcvr_type == TSIP_RCVR) && (tsip_type != STARLOC_RCVR)) ;  // DATUM GPSDO has no temp sensor
   else if(user_set_temp_plot) ;
   else if(have_temperature) plot[TEMP].show_plot = 1;
   else plot[TEMP].show_plot = 0;

   sprintf(log_name, "%s.log", unit_file_name);
   sprintf(raw_name, "%s.raw", unit_file_name);

   reset_com_timer();
   return;
}

//
//
//   Command line processor
//
//

int cmd_a_to_o(char *arg) 
{
char c, d, e, f;
float scale_factor;
int i;

   d = e = f = 0;
   c = tolower(arg[1]);
   if(c) d = tolower(arg[2]);
   if(d) e = tolower(arg[3]);
   if(e) f = tolower(arg[4]);

#ifdef ADEV_STUFF
   if((c == 'a') && (d == 'j')) {  // /aj - calculate PPS adevs from message jitter
      jitter_adev = toggle_option(jitter_adev, e);
   }
   else if(c == 'a') {   // /a - set adev queue size
      not_safe = 1;
//    not_safe = 2;
//    if(keyboard_cmd) return 0;
      if(arg[2] && arg[3]) {  // get adev queue size
         sscanf(&arg[3], "%ld", &adev_q_size);                           
      }
      else {     // default adev queue sizes
         adev_q_size = (330000L);      // good for 100000 tau
      }

      if(adev_q_size <= 0) {
          adev_q_size = 1L;
          adev_period = 0.0F;
      }
      if(adev_q_size >= 50000000L) adev_q_size = 50000000L;
      user_set_adev_size = 1;

      if(keyboard_cmd) {   // we are resizing the queue
         alloc_adev();
         reset_queues(0x01);
      }
   }
   else if(c == 'b') {    // set daylight savings time mode
#else 
   if(c == 'b') {         // /b - set daylight savings time mode 
#endif
      if(d == 'r') {  // /br - set baud rate
         user_set_baud = 1;
         if((e == '=') || (e == ':')) {
            set_com_params(&arg[4]);
         }
         else baud_rate = 57600;
      }
      else if(d == 's') {  // /bs - adjust times for the solar equation-of-time
         solar_time = toggle_option(solar_time, e);
         if(solar_time) {
            strcpy(std_string, "SOLAR");
            strcpy(dst_string, "SOLAR");
         }
         else {
            std_string[0] = 0;
            dst_string[0] = 0;
         }
         if(user_set_watch_name == 0) {
            if(solar_time) set_watch_name("Solar");
            else           watch_name[0] = 0;
            label_watch_face = 1;
         }
      }
      else if(d == 't') { // /bt - toggle terminal mode
         enable_terminal = toggle_option(enable_terminal, e);
      }
      else if(((d == '=') || (d == ':')) && e) {  //  /B# or /B=#  or /B=nth,start_day,month,nth,end_day,month,hour
         if(strstr(&arg[3], ",")) {  // custom daylight savings time settings
            strncpy(custom_dst, &arg[3], sizeof(custom_dst));
            custom_dst[sizeof(custom_dst)-1] = 0;
            dst_area = CUSTOM_DST;
            user_set_dst = 1;
            not_safe = 1;
            if(keyboard_cmd) calc_dst_times(dst_list[dst_area]);
         }
         else if((e >= '0') && (e <= ('0'+DST_AREAS))) {  // standard dst area set
            dst_area = e - '0';
            user_set_dst = 1;
            not_safe = 1;
            if(keyboard_cmd) calc_dst_times(dst_list[dst_area]);
         }
         else return c;
      }
      else if((d >= '0') && (d <= ('0'+DST_AREAS))) {  // /b0 .. /b5 - standard dst area set
         dst_area = d - '0';
         user_set_dst = 1;
         not_safe = 1;
         if(keyboard_cmd) calc_dst_times(dst_list[dst_area]);
      }
      else return c;
   }
   else if(c == 'c') {    // /c - set cable delay
      ++user_set_delay;                  
      not_safe = 1;
      if(luxor && (d == 'g')) {       // cg - use green color channel for lux readings
         alt_lux1 = 0.0F;
         if((e == '=') || (e == ':')) {
            sscanf(&arg[4], "%f", &alt_lux1);
         }
      }
      else if(luxor && (d == 'h')) {  // /ch - set raw color readings mode to Hz
         show_color_hz = 1;
         show_color_uw = 0;
         show_color_pct= 0;
         config_luxor_plots();
      }
      else if(luxor && (d == 'p')) {  // /cp - set raw color readings mode in percent
         show_color_hz = 0;
         show_color_uw = 0;
         show_color_pct= 1;
         config_luxor_plots();
      }
      else if(luxor && (d == 'r')) {  // /cr - alternate CRI energy calcs
         cri_flag ^= 1;
      }
      else if(luxor && (d == 't')) {  // /ct - set cct scale factor
         if((e == '=') || (e == ':')) {
            sscanf(&arg[4], "%f", &cct_cal);
            if(cct_cal <= 0.0F) cct_cal = 1.0F;
         }
         else if(e == '1') {
            if((arg[4] == '=') || (arg[4] == ':')) {
               cct1_cal = (float) atof(&arg[5]);                           
               if(cct1_cal <= 0.0F) cct1_cal = 1.0F;
            }
            else cct_type = 1;
         }
         else if(e == '2') {
            if((arg[4] == '=') || (arg[4] == ':')) {
               cct2_cal = (float) atof(&arg[5]);                           
               if(cct2_cal <= 0.0F) cct2_cal = 1.0F;
            }
            else cct_type = 2;
         }
         else cct_type = 0;
         set_cct_id();
      }
      else if(luxor && (d == 'u')) {  // /cu - set raw color readings mode in uw/cm^2
         show_color_hz = 0;
         show_color_uw = 1;
         show_color_pct= 0;
         config_luxor_plots();
      }
      else {
         if(arg[2] && arg[3]) {
            strcpy(edit_buffer, &arg[3]);
            edit_cable_delay(0);
         }
      }
   }
#ifdef GREET_STUFF
   else if(c == 'd') {   // /d - dates (select calendar)
      if     (d == 'a') alt_calendar = AFGHAN;
      else if(d == 'b') alt_calendar = HAAB;        // Mayan 365 day cycle
      else if(d == 'c') alt_calendar = CHINESE;    
      else if(d == 'd') alt_calendar = DRUID;       // pseudo-Druid calendar
      else if(d == 'e') {  // /de - set debug level
         if((e == '=') || (e == ':')) { 
            if(arg[4]) show_debug_info = atoi(&arg[4]);
            else show_debug_info = 0;
         }
         else show_debug_info ^= 1;
         return 0;
      }
      else if(luxor && (d == 'f')) {
         tcs_color = toggle_option(tcs_color, e);
         if(tcs_color) {
            BLUE_SENS  = 1.0F; 
            GREEN_SENS = 1.0F; 
            RED_SENS   = 1.0F; 
            WHITE_SENS = ((RED_SENS+BLUE_SENS+GREEN_SENS)/3.0F);
         }
         else {
            BLUE_SENS  = 123.94F;    // 371.0F  // converts Hz to uW/cm^2
            GREEN_SENS = 144.80F;    // 386.0F
            RED_SENS   = 177.74F;    // 474.0F
            WHITE_SENS = ((RED_SENS+BLUE_SENS+GREEN_SENS)/3.0F);
         }
      }
      else if(d == 'g') alt_calendar = GREGORIAN;
      else if(d == 'h') alt_calendar = HEBREW;
      else if(d == 'i') alt_calendar = ISLAMIC;
      else if(d == 'j') alt_calendar = JULIAN;
      else if(d == 'k') alt_calendar = KURDISH;
      else if(d == 'l') {  // /dl - open debug file
         if(debug_file) {
            fclose(debug_file);
            debug_file = 0;
            debug_name[0] = 0;
         }

         if((e == '=') || (e == ':')) { 
            if(arg[4] && arg[5]) {
               open_debug_file(&arg[4]);
            }
         }
         if(debug_file == 0) need_debug_log = 1;
         return 0;
      }
      else if(d == 'm') alt_calendar = MJD;
      else if(d == 'n') alt_calendar = INDIAN;
      else if(d == 'p') alt_calendar = PERSIAN;
      else if(d == 'r') {  // /dr - open raw receiver data file
         if(raw_file) {
            fclose(raw_file);
            raw_file = 0;
         }

         if((e == '=') || (e == ':')) { 
            if(arg[4] && arg[5]) raw_file = fopen(&arg[4], "wb");
         }
         if(raw_file == 0) need_raw_file = 1;
         else log_stream |= 0x02;
         return 0;
      }
      else if(d == 's') alt_calendar = ISO;
      else if(d == 't') alt_calendar = TZOLKIN;     // Mayan 260 day cycle
      else if(d == 'u') take_a_dump ^= 1;
      else if(d == 'v') alt_calendar = BOLIVIAN;    // Bolivian 13 month calendar
      else if(d == 'x') alt_calendar = AZTEC_HAAB;  // Aztec 365 day cycle
      else if(d == 'y') alt_calendar = MAYAN;       // Mayan long count
      else if(d == 'z') alt_calendar = AZTEC;       // Aztec 260 day cycle
      else if(isdigit(d)) {   // force date for calendar testing
         sscanf(&arg[2], "%d%c%d%c%d", &force_year,&c,&force_month,&c,&force_day);
         if(keyboard_cmd) {
            pri_year = this_year = force_year;
            pri_month = force_month;
            pri_day = force_day;

            new_moon_info = 1;     // force recalculation of calendars, etc
            have_year = 0;
            last_hours = (-99);

            calc_dst_times(dst_list[dst_area]);
            dst_ofs = dst_offset();

            calc_greetings();  // calculate seasonal holidays and insert into table
            show_greetings();  // display any current greeting
         }
      }
      else if(d == 0) alt_calendar = GREGORIAN;
      else return c;

      if((e == '=') || (e == ':')) {  // calendar epoch adjustments
         if(arg[4]) {
            if     (alt_calendar == CHINESE)    sscanf(&arg[4], "%ld", &chinese_epoch);
            else if(alt_calendar == DRUID)      sscanf(&arg[4], "%ld", &druid_epoch);
            else if(alt_calendar == MAYAN)      sscanf(&arg[4], "%ld", &mayan_correlation);
            else if(alt_calendar == HAAB)       sscanf(&arg[4], "%ld", &mayan_correlation);
            else if(alt_calendar == TZOLKIN)    sscanf(&arg[4], "%ld", &mayan_correlation);
            else if(alt_calendar == AZTEC)      sscanf(&arg[4], "%ld", &aztec_epoch);
            else if(alt_calendar == AZTEC_HAAB) sscanf(&arg[4], "%ld", &aztec_epoch);
            else if(alt_calendar == BOLIVIAN)   sscanf(&arg[4], "%ld", &bolivian_epoch);
         }
      }
   }
#endif
   else if(c == 'e') {   // /e - log errors
      log_errors = toggle_option(log_errors, d);                           
   }
   else if (c == 'f') { // /f - toggle filter 
      if(d) {  // toggle filter
         if     (d == 'a') user_alt = toggle_option(user_alt, e);
         else if(d == 'g') {
            dump_gpx = toggle_option(dump_gpx, e); // /fg - dump in GPX format
            dump_xml = 0;
         }
         else if(d == 'i') user_static = toggle_option(user_static, e); // Mototola ionosphere
         else if(d == 'k') user_kalman = toggle_option(user_kalman, e);
         else if(d == 'p') user_pv = toggle_option(user_pv, e); 
         else if(d == 's') user_static = toggle_option(user_static, e);
         else if(d == 't') user_alt = toggle_option(user_alt, e);       // Motorola troposphere
         else if(d == 'u') vfx_fullscreen = toggle_option(vfx_fullscreen, e);       // WIN_VFX full screen enable
         else if(d == 'd') {  // /fd = display filter
            if((e == '=') || (e == ':')) { 
               if(arg[4]) {
                  if(strstr(&arg[4], "-")) plot[PPS].invert_plot = plot[TEMP].invert_plot = (-1.0); 
                  else if(strstr(&arg[4], "+")) plot[PPS].invert_plot = plot[TEMP].invert_plot = (1.0); 
                  filter_count = atoi(&arg[4]);
                  if(filter_count < 0) filter_count = 0 - filter_count;
               }
               else filter_count = 10;
            }
            else filter_count = 10;
            return 0;
         }
         else if(d == 'x') {
            dump_xml = toggle_option(dump_xml, e); // /fx - dump in XML format
            dump_gpx = 0;
         }
         else return d;
      }
      else {   // /f - start up in full screen mode
         not_safe = 1;
         go_fullscreen = 1;
         need_screen_init = 1;
      }
   }
   else if(c == 'g') {   // /g - toggle Graph enables
      if     (d == 'a') { plot_adev_data = toggle_option(plot_adev_data, e); user_set_adev_plot = 1; }
      else if(d == 'b') { plot_azel = AZEL_OK;  shared_plot = 1; if(plot_azel) update_azel = 1; }
      else if(d == 'c') { plot_sat_count = toggle_option(plot_sat_count, e); }
      else if(d == 'd') { plot[DAC].show_plot = toggle_option(plot[DAC].show_plot, e); user_set_dac_plot = 1; }
      else if(d == 'e') { plot_skip_data = toggle_option(plot_skip_data, e); }
      else if(d == 'h') { plot_holdover_data = toggle_option(plot_holdover_data, e); }
      else if(d == 'i') { 
         show_fixes = toggle_option(show_fixes, e);
         user_fix_set = show_fixes;
      }
      else if(d == 'j') { plot_el_mask = toggle_option(plot_el_mask, e); }
      else if(d == 'k') { plot_const_changes = toggle_option(plot_const_changes, e); }
      else if(d == 'l') { // /gl command - change location format
         if(e) {
            getting_plot = (-1);
            change_plot_param(e, 1);
            getting_plot = 0;
         }
         else plot_loc = toggle_option(plot_loc, e);
      }
      else if(d == 'm') { plot_azel = AZEL_OK;  shared_plot = 0; if(plot_azel) update_azel = 1; }
      else if(d == 'n') { no_greetings = toggle_option(no_greetings, e); }
      else if(d == 'o') { plot[OSC].show_plot = toggle_option(plot[OSC].show_plot, e); user_set_osc_plot = 1; } 
      else if(d == 'p') { plot[PPS].show_plot = toggle_option(plot[PPS].show_plot, e); user_set_pps_plot = 1; } 
      else if(d == 'q') {  // /gq - satellite signal level map
         if     (e == 'a') plot_signals = 1;
         else if(e == 'd') plot_signals = 5;
         else if(e == 'e') plot_signals = 3;
         else if(e == 'q') plot_signals = 4;
         else if(e == 's') plot_signals = 4;
         else if(e == 'w') plot_signals = 2;
         else if(e == '1') plot_signals = 4;
         else if(e == 'y') plot_signals = 4;
         else if(e == '0') plot_signals = 0;
         else if(e == 'n') plot_signals = 0;
         else if(e) return d;
         else if(plot_signals) plot_signals = 0; 
         else plot_signals = 4; 

         user_set_signals = 1;
         if(plot_azel && plot_watch) shared_plot = 1;
         return 0;
      }
      else if(d == 'r') { plot_stat_info = toggle_option(plot_stat_info, e); }
      else if(d == 's') { beep_on = toggle_option(beep_on, e); }
      else if(d == 't') { plot[TEMP].show_plot = toggle_option(plot[TEMP].show_plot, e); user_set_temp_plot = 1; }  
//    else if(d == 'u') { continuous_scroll = toggle_option(continuous_scroll, e); }
      else if(d == 'v') { 
         plot[ONE].show_plot = toggle_option(plot[ONE].show_plot, e); 
         plot[TWO].show_plot = toggle_option(plot[TWO].show_plot, e); 
         plot[THREE].show_plot = toggle_option(plot[THREE].show_plot, e); 
      }
//    else if(d == 'w') { plot_watch = toggle_option(plot_watch, e); user_set_bigtime = 1; plot_digital_clock = 0; user_set_watch_plot = 1; }
      else if(d == 'w') { plot_watch = toggle_option(plot_watch, e); user_set_bigtime = 1; user_set_watch_plot = 1; }
      else if(d == 'x') { plot_dops = toggle_option(plot_dops, e); user_set_dops = 1; }
      else if(d == 'y') { plot_filters = toggle_option(plot_filters, e); user_set_filters = 1; }
//    else if(d == 'z') { plot_digital_clock = toggle_option(plot_digital_clock, e); user_set_bigtime = 1;  plot_watch = 0; user_set_clock_plot = 1; }
      else if(d == 'z') { plot_digital_clock = toggle_option(plot_digital_clock, e); user_set_bigtime = 1;  user_set_clock_plot = 1; }
      else if(d == '0') { plot[TEN].show_plot = toggle_option(plot[TEN].show_plot, e); }
      else if((d >= '1') && (d <= '9')) {
         plot[ONE+d-'1'].show_plot = toggle_option(plot[ONE+d-'1'].show_plot, e);
      }
      else return c;
   }
   else if(c == 'h') { // /h - read config file
      if(((d == '=') || (d == ':')) && arg[3]) read_config_file(&arg[3], 0, 0);
   }
   else if(c == 'i') {   
      if((d == '0') || (d == 'o')) {  // /io - do not process any received TSIP commands
         just_read = toggle_option(just_read, e);
      }
#ifdef WINDOWS
#else // __linux__  __MACH__
      else if(d == 'd') {  // /id - set input device name
         com_port = 0;
         usb_port = 9999;
         process_com = 1;
         if(arg[4] && ((arg[3] == '=') || (arg[3] == ':'))) {
            strncpy(com_dev, &arg[4], sizeof(com_dev));  
         }
         else if(com_dev[0]) ;
         else {
            strncpy(com_dev, "/dev/heather", sizeof(com_dev));  
         }
         com_dev[sizeof(com_dev)-1] = 0;
         if(com_running) {
            init_com();
         }
         last_com_time = this_msec - com_timeout;
      }
#endif
#ifdef TCP_IP            // TCP: com_port 0 with process_com 1 means use TCP/IP connection rather than COM port 
      else if(d == 'p') { // /ip - set TCPIP address
         if(arg[4] && ((arg[3] == '=') || (arg[3] == ':'))) {
            com_port = usb_port = 0;
            process_com = 1;
            strncpy(IP_addr, &arg[4], sizeof(IP_addr));  
            IP_addr[sizeof(IP_addr)-1] = 0;

            for(i=strlen(IP_addr)-1; i>=0; i++) {
               if(IP_addr[i] == 0x0A) IP_addr[i] = 0;
               else if(IP_addr[i] == 0x0D) IP_addr[i] = 0;
               else if(IP_addr[i] == ' ') IP_addr[i] = 0;
               else if(IP_addr[i] == '\t') IP_addr[i] = 0;
               if(IP_addr[i] == 0) break;
            }
         }
         if(com_running && IP_addr[0]) {
            com_port = usb_port = 0;
            process_com = 1;
            init_com();
         }
         last_com_time = this_msec - com_timeout;
      }
#endif 
      else if(d == 'r') {  // /ir - block TSIP commands that change the unit's state
         read_only = toggle_option(read_only, e);
      }
      else if(d == 's') {  // /is - do not send data out the serial port
         no_send = toggle_option(no_send, e);
      }
      else if(d == 't') {  // /it - do not send message poll requests
         no_poll = toggle_option(no_poll, e);
         need_redraw = 4321;
      }
      else {               // /i - set plot queue update interval  
         if(arg[2] && arg[3]) {
            sscanf(&arg[3], "%ld", &queue_interval);                           
            if(queue_interval < 1) queue_interval = 0;
         }
         else {
            day_plot = 24;
            queue_interval = 1;
            interval_set = 1;
         }
      }
   }
#ifdef ADEV_STUFF
   else if(c == 'j') {  // /j - adev sample interval (not adev info display interval)
      if(d == 'p') {    // /jp - JPL wall clock mode
         jpl_clock = toggle_option(jpl_clock, e);
         if(jpl_clock) {
            change_zoom_config(9);
            zoom_screen = 'C';
            zoom_fixes = show_fixes;
            alt_calendar = ISO;
         }
         else {
            zoom_screen = 0;
            change_zoom_config(99);
            alt_calendar = 0;
         }
      }
      else {
         not_safe = 1;
         if(arg[2] && arg[3]) {
            sscanf(&arg[3], "%f", &adev_period);                           
            if(adev_period < 1.0F) adev_period = 0.0F;
         }
         else if(adev_period == 1.0F) adev_period = 10.0F;
         else adev_period = 1.0F;
         if(keyboard_cmd) {
            reset_queues(0x01);
         }
      }
   }
#endif
   else if(c == 'k') {  // /k - disable keyboard commands
      if     (d == 'b') beep_on = toggle_option(beep_on, e);  // /kb - disable beeper
      else if(d == 'c') {
         no_eeprom_writes = toggle_option(no_eeprom_writes, e);  // /kc - disable config eeprom writes
         if(no_eeprom_writes) eeprom_save = 0;
         else                 eeprom_save = 1;
      }
      else if(d == 'e') esc_esc_exit = toggle_option(esc_esc_exit, e); // /ke - allow ESC ESC to exit program
      else if(d == 'j') {  // /kj - toggle sun moon in maps
        if((e == '=') || (e == ':')) {
           no_sun_moon = atoi(&arg[4]);
        }
        else {
           if(no_sun_moon) no_sun_moon = 0;
           else            no_sun_moon = 0x03;
        }
        need_redraw = 4598;
      }
      else if(d == 'm') mouse_disabled = toggle_option(mouse_disabled, e);  // /km - disable mouse
      else if(d == 'q') disable_kbd = toggle_option(disable_kbd, e);// /kk - disable keyboard
      else if(d == 's') sound_on =  toggle_option(sound_on, e);  // /ks - disable sound files
      else if(d == 't') enable_timer = toggle_option(enable_timer, e); // /kt - disable windows timer
      else if(d == 'u') no_easter_eggs = toggle_option(no_easter_eggs, e);  // /kg - kill easter eggs
      else if(d) { // /k pid options: a d f g h i k l n o p r s t w x y z 0 9
         #ifdef TEMP_CONTROL
            if((e == '=') || (e == ':')) {
               if(edit_temp_pid_value(d, &arg[4], 0)) return c;
            }
            else {
               if(edit_temp_pid_value(d, &arg[3], 0)) return c;
            }
         #else
            return d;
         #endif
      }
      else return c;
   }
   else if(c == 'l') {  // log write interval
      if(d == 'c') {       // /lc - toggle comments in the log flag
         log_comments = toggle_option(log_comments, e);
      }
      else if(d == 'd') {  // /ld - toggle constellation/signal level data flag
         log_db = toggle_option(log_db, e);
      }
      else if(luxor && (d == 'f')) {  // /lf - show lux in footcandles
         if((e == '=') || (e == ':')) sscanf(&arg[4], "%f", &lux_scale);
         else lux_scale = 1.0F / 10.76391F;
         show_lux = 0;
         show_fc = 1;
         config_luxor_plots();
         return 0;
      }
      else if(d == 'h') {  // /lh - toggle timestamp headers in the log
         log_header = toggle_option(log_header, e);
      }
      else if(d == 'o') {  // /lo command - read old format log files
         old_log_format = toggle_option(old_log_format, e);
      }
      else if(luxor && (d == 'p')) {  // /lp - show lumens in candlepower
         if((e == '=') || (e == ':')) sscanf(&arg[4], "%f", &lum_scale);
         else lum_scale = 1.0F/(4.0F*(float)PI);
         show_lumens = 0;
         show_cp = 1;
         config_luxor_plots();
         return 0;
      }
      else if(d == 's') {  // /ls - log file value separator
         if(csv_char == ',') csv_char = '\t';
         else                csv_char = ',';
      }
//    else if((d == 'u') && (e == 'x')) {  // /lux - force luxor mode
//       luxor = toggle_option(luxor, f);
//    }
      else if(luxor && (d == 'u')) {  // /lu - show lumens in lumens
         if((e == '=') || (e == ':')) sscanf(&arg[4], "%f", &lum_scale);
         else lum_scale = 1.0F;
         show_lumens = 1;
         show_cp = 0;
         config_luxor_plots();
         return 0;
      }
      else if(luxor && (d == 'x')) {  // /lx - show lux in lux
         if((e == '=') || (e == ':')) sscanf(&arg[4], "%f", &lux_scale);
         else lux_scale = 1.0F;
         show_lux = 1;
         show_fc = 0;
         config_luxor_plots();
         return 0;
      }
      else {
         if(arg[2] && arg[3]) {
            sscanf(&arg[3], "%ld", &log_interval);                           
            if(log_interval < 1) log_interval = 0 - log_interval;
            log_file_time = log_interval+1;
         }
         user_set_log = 1;
      }
   }
   else if(c == 'm') {  // /m - multiply or set default plot scale factors
      if(d == 'a') {  // ma - 
         auto_scale = toggle_option(auto_scale, e);  // turn off auto scaling
         auto_center = auto_scale;
      }
      else if(d == 'd') {  // /md  - set dac scale factor
         plot[DAC].user_scale = 1;
         scale_factor = plot[DAC].scale_factor * 2.0F;
         plot[DAC].invert_plot = 1.0;
         if(arg[4] && ((arg[3] == '=') || (arg[3] == ':'))) {
            sscanf(&arg[4], "%f", &scale_factor);                           
            if(scale_factor <= 0.0) scale_factor *= (-1.0);
            if(strstr(&arg[4], "-")) plot[DAC].invert_plot = (-1.0);
         }
         plot[DAC].scale_factor = scale_factor;
      }
      else if(d == 'i') {  // /mi - Invert PPS and termperature plot scale factors
         if(plot[PPS].invert_plot < 1.0F) plot[PPS].invert_plot = 1.0F;
         else                             plot[PPS].invert_plot = (-1.0F);
         if(plot[TEMP].invert_plot < 1.0F) plot[TEMP].invert_plot = 1.0F;
         else                              plot[TEMP].invert_plot = (-1.0F);
      }
      else if(d == 'o') {  // /mo - osc error scale factor
         plot[OSC].user_scale = 1;
         plot[OSC].invert_plot = 1.0;
         scale_factor = (float) plot[OSC].scale_factor * 2.0F;
         if(arg[4] && ((arg[3] == '=') || (arg[3] == ':'))) {
            sscanf(&arg[4], "%f", &scale_factor);                           
            if(scale_factor <= 0.0) scale_factor *= (-1.0);
            if(strstr(&arg[4], "-")) plot[OSC].invert_plot = (-1.0);
         }
         plot[OSC].scale_factor = (OFS_SIZE) scale_factor;
      }
      else if(d == 'p') {  // /mp - pps error scale factor
         plot[PPS].user_scale = 1;
         plot[PPS].invert_plot = 1.0;
         scale_factor = (float) plot[PPS].scale_factor * 2.0F;
         if(arg[4] && ((arg[3] == '=') || (arg[3] == ':'))) {
            sscanf(&arg[4], "%f", &scale_factor);                           
            if(scale_factor <= 0.0) scale_factor *= (-1.0);
            if(strstr(&arg[4], "-")) plot[PPS].invert_plot = (-1.0);
         }
         plot[PPS].scale_factor = (OFS_SIZE) scale_factor;
      }
      else if(d == 't') {  // /mt - temperature scale factor
         plot[TEMP].user_scale = 1;
         scale_factor = plot[TEMP].scale_factor * 2.0F;
         plot[TEMP].invert_plot = 1.0;
         if(arg[4] && ((arg[3] == '=') || (arg[3] == ':'))) {
            sscanf(&arg[4], "%f", &scale_factor);                           
            if(scale_factor <= 0.0) scale_factor *= (-1.0);
            if(strstr(&arg[4], "-")) plot[TEMP].invert_plot = (-1.0);
         }
         plot[TEMP].scale_factor = scale_factor;
      }
      else if((d >= '0') && (d <= '9')) {  // /m0 .. /m9 - the extra plots
         if(d == '0') d = 10;
         else         d = d - '0' - 1;
         d += FIRST_EXTRA_PLOT;
         plot[(int) d].user_scale = 1;
         scale_factor = plot[(int) d].scale_factor * 2.0F;
         plot[(int) d].invert_plot = 1.0;
         if(arg[4] && ((arg[3] == '=') || (arg[3] == ':'))) {
            sscanf(&arg[4], "%f", &scale_factor);                           
            if(scale_factor <= 0.0) scale_factor *= (-1.0);
            if(strstr(&arg[4], "-")) plot[TEMP].invert_plot = (-1.0);
         }
         plot[(int) d].scale_factor = scale_factor;
      }
      else if(d) return c;
      else {   // /m - multiply all scale plot factors
         scale_factor = 2.0;
         if(arg[2] && arg[3]) {  // /m=
            sscanf(&arg[3], "%f", &scale_factor);                           
         }
         
         plot[TEMP].user_scale    = plot[OSC].user_scale = plot[DAC].user_scale = plot[PPS].user_scale = 1;
         plot[OSC].scale_factor  *= scale_factor;
         plot[PPS].scale_factor  *= scale_factor;
         plot[DAC].scale_factor  *= scale_factor;
         plot[TEMP].scale_factor *= scale_factor;
      }
   }
   else if(c == 'n') {  // set end time
      if(d == 'a') {
        getting_string = ':';  //  /na - alarm time
        edit_dt(&arg[4], 0);
        getting_string = 0;
      }
      else if(d == 'd') {
        getting_string = '!';  //  /nd - screen dump interval
        edit_dt(&arg[4], 0);
        getting_string = 0;
      }
      else if(d == 'l') {
        getting_string = '(';  //  /nl - log dump interval
        edit_dt(&arg[4], 0);
        getting_string = 0;
      }
      else if(d == 'r') {      // /nr - force nav rate on startup
         user_set_nav_rate = 1;
         if(((e == '=') || (e == ':')) && arg[4]) user_nav_rate = (float) atof(&arg[4]);
         else                                     user_nav_rate = 1.0F;

         if(user_nav_rate <= 0) user_nav_rate = 1;
         else if(user_nav_rate > 50) user_nav_rate = 50;
      }
      else if(d == 't') {      // /nt - try to wake up Nortel NTGxxxx unit
         ++nortel;
      }
      else if(d == 'x') {
        getting_string = '/';  //  /nx - exit time
        edit_dt(&arg[4], 0);
        getting_string = 0;
      }
      else if(d) return c;
      else {
        getting_string = '/';  //  /n - exit time
        edit_dt(&arg[3], 0);
        getting_string = 0;
      }
   }
#ifdef ADEV_STUFF
   else if(c == 'o') {   // /o - set adev type
      if     (d == 'a') { ATYPE = OSC_ADEV; all_adevs = 0; }  // /oa -
      else if(d == 'h') { ATYPE = OSC_HDEV; all_adevs = 0; }  // /oh - 
      else if(d == 'm') { ATYPE = OSC_MDEV; all_adevs = 0; }  // /om - 
      else if(d == 't') { ATYPE = OSC_TDEV; all_adevs = 0; }  // /ot - 
      else if(d == 'o') all_adevs = 1;                        // /oo - 
      else if(d == 'p') all_adevs = 2;                        // /op - 
      else return c;
   }
#endif
   else return c;

   return 0;
}

int cmd_other(char *arg)
{
char c, d, e, f;
float scale_factor;

   d = e = f = 0;
   c = tolower(arg[1]);
   if(c) d = tolower(arg[2]);
   if(d) e = tolower(arg[3]);
   if(e) f = tolower(arg[4]);

   if(c == 'p') {    // toggle signal PPS enable
      if(d == 'o'){  // /po - set lat,lon,alt
         if(((e == '=') || (e == ':')) && arg[4]) {
            strcpy(edit_buffer, &arg[4]);
            if(parse_lla(&edit_buffer[0]) == 0) {
               lat = precise_lat;
               lon = precise_lon;
               alt = precise_alt;
               ref_lat = lat;
               ref_lon = lon;
               ref_alt = alt;
               cos_factor = cos(ref_lat);
            }
            else return 1;
         }
         else return 0;
      }
      else if(d == 'd') {  // pd - disable PPS
         not_safe = 1;
         ++set_pps_polarity;
         user_pps_enable = 0;
      }
      else if(d == 'e') {  // pe - enable PPS
         not_safe = 1;
         ++set_pps_polarity;
         user_pps_enable = 1;
      }
      else if(d == 's') {  // ps - toggle PPS
         not_safe = 1;
         ++set_pps_polarity;
         user_pps_enable = toggle_option(user_pps_enable, e);
      }
      else {
         not_safe = 1;
         ++set_pps_polarity;
         user_pps_enable = toggle_option(user_pps_enable, d);
      }
   }
   else if(c == 'q') {    
      if(d == 'f')  {  // /qf - set max fft size
#ifdef FFT_STUFF
//       not_safe = 2;
//       if(keyboard_cmd) return 0;
         not_safe = 1;
         if(e == '=')      max_fft_len = (long) atof(&arg[4]);
         else if(e == ':') max_fft_len = (long) atof(&arg[4]);
         else if(e)        max_fft_len = (long) atof(&arg[3]);
         else              max_fft_len = 4096;
         if(max_fft_len < 0) max_fft_len = 0 - max_fft_len;
         max_fft_len = (1 << logg2(max_fft_len));
         if(keyboard_cmd) {  // we are resizing the FFT queue
            alloc_fft();
         }
#endif
      }
      else {  // /q - set plot queue size
//       not_safe = 2;
//       if(keyboard_cmd) return 0;
         not_safe = 1;
         if(arg[2] && arg[3]) {  // get plot queue size
            sscanf(&arg[3], "%ld", &plot_q_size);                           
            strupr(&arg[3]);
            if     (strstr(&arg[3], "W")) plot_q_size *= 3600L*24L*7L;
            else if(strstr(&arg[3], "D")) plot_q_size *= 3600L*24L;
            else if(strstr(&arg[3], "H")) plot_q_size *= 3600L;
            else if(strstr(&arg[3], "M")) plot_q_size *= 60L;
         }
         else {     // default plot queue sizes
            plot_q_size = 3600L*24L*30L;   // boost plot queue to 30 days 'o data
         }

         if(plot_q_size <= 10L) plot_q_size = 10L;
         if(plot_q_size >= (366L*24L*3600L)) plot_q_size = (366L*24L*3600L);  // 1 year max
         user_set_plot_size = 1;
         if(keyboard_cmd) {  // we are resizing the queue
            alloc_plot();
            reset_queues(0x02);
         }
      }
   }
   else if(c == 'r') {    // Read a log file into the plot and adev queues
      if(d == 'o') {      // /ro - set gps date/time rollover adjustment in seconds
         user_set_rollover = 1;
         if(((e == '=') || (e == ':')) && arg[4]) {
            rollover = atof(&arg[4]);
            if(strchr(&arg[4], '*')) {  // value /ro=2* says rollover two 1024 week cycles
               rollover *= (1024.0 * 7.0 * (24.0*60.0*60.0));
            }
         }
         else rollover = 1024.0 * 7.0 * (24.0*60.0*60.0);
      }
      else if(d == 's') { // /rs - read simulation (raw receiver data) file
         if(((e == '=') || (e == ':')) && arg[4]) {
            strcpy(sim_name, &arg[4]);
            sim_file = topen(sim_name, "rb");
            if(sim_file == 0) return 1;
         }
         else return 1;
      }
      else if(d == 't') { // /rt - Resolution-T receiver... use 9600,ODD,1
//       res_t_init = toggle_option(res_t_init, e);
         parity = 1;
         if(((e == '=') || (e == ':')) && arg[4]) {
            res_t = (u08) atof(&arg[4]);
            user_set_res_t = 1;
         }
      }
      else if(d == 'x') {  // /rx - set receiver type
         detect_rcvr_type = 0;
         last_rcvr_type = rcvr_type;
         if(e == 'a') {  // rxa - Acron Zeit WWVB clock
            rcvr_type = ACRON_RCVR;
         }
         else if(e == 'c') {   // rxc - Trimble/Symmetricom UCCM SCPI
            rcvr_type = UCCM_RCVR; 
            scpi_type = UCCM_TYPE;
            if(f == 'p') scpi_type = UCCMP_TYPE;
            scpi_init(e);

            config_utc_mode(0);
         }
         else if(e == 'd') {  // /rxd - DATUM STARLOC GPSDO
            rcvr_type = TSIP_RCVR;
            tsip_type = STARLOC_RCVR;
         }
         else if(e == 'e') {  // /rxe - NEC GPSDO (like STAR-4 at 115200 baud)
            rcvr_type = STAR_RCVR;
            star_type = NEC_TYPE;
         }
         else if(e == 'g') {  // /rxg - GPSD
            rcvr_type = GPSD_RCVR;
            strcpy(IP_addr, "localhost:2947");
            com_port = usb_port = 0;
            process_com = 1;
            init_com();
         }
         else if(e == 'j') {  // /rxj - Zodiac / Jupiter-T
            rcvr_type = ZODIAC_RCVR;
         }
         else if(e == 'k') {  // /rxk - Lucent KS-24xxx SCPI
            rcvr_type = SCPI_RCVR;
            scpi_type = LUCENT_TYPE;
            scpi_init(e);
         }
         else if(e == 'l') {  // //rxl - luxor
            rcvr_type = LUXOR_RCVR; // TSIP_RCVR;
            tsip_type = TBOLT_RCVR;
            parity = LUXOR_PAR;
            luxor = 21;
         }
         else if(e == 'm') {  // /rxm - Motorola
            rcvr_type = MOTO_RCVR;
         }
         else if(e == 'n') {  // /rxn - NMEA receivers
            rcvr_type = NMEA_RCVR;
         }
         else if(e == 'r') {  // /rxt - Trimble res-T 
            rcvr_type = TSIP_RCVR;
            tsip_type = TBOLT_RCVR;
            parity = 1;
            res_t = RES_T;
            user_set_res_t = 1;

            if(((f == '=') || (f == ':')) && arg[5]) {
               res_t = (u08) atof(&arg[5]);
               user_set_res_t = 1;
            }
//          if(res_t == RES_T_SMT) parity = 0;
            config_rcvr_plots();
config_lla_plots(1, 1);
            config_msg_ofs();
         }
         else if(e == 's') {  // /rxs - SIRF
            rcvr_type = SIRF_RCVR;
         }
         else if(e == 't') {  // /rxt - Trimble TSIP devices
            rcvr_type = TSIP_RCVR;
            tsip_type = TBOLT_RCVR;
         }
         else if(e == 'u') {  // /rxu - Ublox
            rcvr_type = UBX_RCVR;
         }
         else if(e == 'v') {  // /rxv - Venus
            rcvr_type = VENUS_RCVR;
         }
         else if(e == 'x') {  // /rxx - no receiver, clock only mode
            rcvr_type = NO_RCVR;
         }
         else if(e == 'y') {  // /rxy - Nortel SCPI mode
            rcvr_type = SCPI_RCVR;
            scpi_type = NORTEL_TYPE;
            scpi_init(e);
         }
         else if(e == 'z') {  // /rxv - SCPI
            rcvr_type = SCPI_RCVR;
            scpi_type = SCPI_TYPE;
            scpi_init(e);
         }
         else if(e == '4') {  // /rx4 - Oscilloquartz Star-4
            rcvr_type = STAR_RCVR;
            star_type = OSCILLO_TYPE;
         }
         else if(e == '5') {  // /rx5 - HP53xxx SCPI
            rcvr_type = SCPI_RCVR;
            scpi_type = HP_TYPE;
            scpi_init(e);
         }
         else if(e == '8') {   // /rx8 - NVS Binr
            rcvr_type = NVS_RCVR;
         }
         else {
            detect_rcvr_type = 1;
            if(hw_setup) auto_detect();
            if(((e == '=') || (e == ':')) && arg[5]) { // /rx=utc offset
               utc_offset = (int) atof(&arg[4]);
               have_utc_ofs = 102;
               user_set_utc_ofs = 3;
            }
            return 0;
         }


         if(res_t) ;  // /rxr=# sets res-t type, not UTC offset!!!
         else if(((f == '=') || (f == ':')) && arg[5]) { // /rx?=utc offset
            utc_offset = (int) atof(&arg[5]);
            have_utc_ofs = 102;
            user_set_utc_ofs = 3;
         }

         if((last_rcvr_type == NO_RCVR) && (rcvr_type != NO_RCVR)) { // re-enable com port
            process_com = 1;
         }

         detect_rcvr_type = 0;  // user forced the receiver type
         config_rcvr_type(1);
      }
      else { // /r - read log file
         if(arg[2] && arg[3]) {
            strcpy(read_log, &arg[3]);
         }
         else strcpy(read_log, log_name);
      }
   }
   else if((c == 's') && (d == 'i')) {   // /si - force max displayed sats
      max_sat_display = 14;
      if(((e == '=') || (e == ':')) && (arg[4])) {
         max_sat_display = atoi(&arg[4]);
      }
      else max_sat_display = 8;

      if(max_sat_display < 0) {
         max_sat_display = 0-max_sat_display;
         user_set_short = 1;
      }
      else user_set_short = 0;

      if(strchr(&arg[4], '+')) sat_cols = 2;
      else                     sat_cols = 1;
if((sat_cols > 1) && (max_sat_display < 16)) max_sat_display = 16;

      max_sats = max_sat_display;  // used to format the sat_info data
      max_sat_count = max_sat_display;
      temp_sats = max_sat_display;
      config_sat_rows();
   }
   else if((c == 's') && (d == 'r')) {   // /sr - set sunrise type
      if(((e == '=') || (e == ':')) && (arg[4])) {
         set_sunrise_type(&arg[4], 0);
      }
      else {
         sunrise_type = 0;
         need_sunrise = 0;
         play_sun_song = 0;
      }
   }
   else if((c == 's') && (d == 'd')) {   // /sd - toggle dotting of sat trails
      dot_trails = toggle_option(dot_trails, e);
      if(keyboard_cmd) need_redraw = 3489;
   }
   else if((c == 's') && (d == 's')) {   // /ss - force site position survey
      user_precision_survey = 0;
      do_survey = SURVEY_SIZE;
      survey_why = 32;
      if(arg[3] && arg[4]) sscanf(&arg[4], "%ld", &do_survey);                           
      if(do_survey < 1)  {
         do_survey = 0;
         survey_why = (-33);
      }
   }
   else if((c == 's') && (d == 't')) {   // /st - toggle satellite trails
      map_trails = toggle_option(map_trails, e);
      if(keyboard_cmd) need_redraw = 3489;
   }
#ifdef PRECISE_STUFF
   else if((c == 's') && (d == 'f')) {   // /sf - force 3D fix mode
      show_fixes = toggle_option(show_fixes, e);
      user_fix_set = show_fixes;
//    if(show_fixes) user_precision_survey = 2;
//    else           user_precision_survey = 0;
      do_survey = 0;
      survey_why = (-21);
      user_precision_survey = 0;
   }
   else if((c == 's') && (d == 'p')) {   // /sp - force precision survey
      not_safe = 1;
      user_precision_survey = 1;
      do_survey = 0;
      survey_why = (-20);
      if(arg[3]) sscanf(&arg[4], "%ld", &do_survey);                           
      if((do_survey < 1) || (do_survey > SURVEY_BIN_COUNT)) {
         do_survey = 0;
         survey_why = (-34);
      }
      else survey_why = 34;
   }
#endif
   else if(c == 't') {   // set Time reference (GPS/UTC) or temp reference (C/F)
      if(arg[3] && ((d == '=') || (d == ':') || (d == 'z'))) {  // /t or /tz - zoneinfo or /tz=zoneinfo
         set_time_zone(&arg[3]);
         if(keyboard_cmd) calc_dst_times(dst_list[dst_area]);
         return 0;
      }
      else if((d == '+') || (d == '-') || isdigit(d)) {
         set_time_zone(&arg[2]);
         if(keyboard_cmd) calc_dst_times(dst_list[dst_area]);
         return 0;
      }
      else if(d == 'a') {  // /ta - european date ordering
         show_euro_dates = toggle_option(show_euro_dates, e);
         return 0;
      }
      else if(d == 'b') {  // /tb - set watch brand name
         if((e == '=') || (e == ':')) {
            if(arg[4]) set_watch_name(&arg[4]);
            else       watch_name[0] = 0;
            label_watch_face = 1;
            user_set_watch_name = 1;
         }
         else {
            label_watch_face = toggle_option(label_watch_face, e);
         }
         return 0;
      }
      else if(d == 'c') {   // /tc - use degrees C
         DEG_SCALE = 'C';
         plot[TEMP].plot_center = NEED_CENTER;
         if(luxor) plot[TC2].plot_center = NEED_CENTER;
         return 0;
      }
      else if(d == 'd') {   // /td - use degrees Delisle
         DEG_SCALE = 'D';
         plot[TEMP].plot_center = NEED_CENTER;
         if(luxor) plot[TC2].plot_center = NEED_CENTER;
         return 0;
      }
      else if(d == 'e') {   // /te - use degrees Reaumur
         DEG_SCALE = 'E';
         plot[TEMP].plot_center = NEED_CENTER;
         if(luxor) plot[TC2].plot_center = NEED_CENTER;
         return 0;
      }
      else if(d == 'f') {   // /tf - use degrees F
         DEG_SCALE = 'F';
         plot[TEMP].plot_center = NEED_CENTER;
         if(luxor) plot[TC2].plot_center = NEED_CENTER;
         return 0;
      }
      else if(d == 'g') {   // /tg - use GPS time
         ++set_gps_mode;                  
         time_flags |= 0x0001;
         set_utc_mode = 0;
         user_set_time_mode = 'g';
         return 0;
      }
      else if(d == 'h') {  // /th - set chime mode
         strupr(arg);
         cuckoo_hours = singing_clock = 0;
         if(strstr(&arg[3], "H")) cuckoo_hours  = 1;
         if(strstr(&arg[3], "S")) singing_clock = 1;
         else if(strstr(&arg[3], "B")) ships_clock = 1;
         if(((e == '=') || (e == ':')) && (arg[4])) {
            scale_factor = (float) atof(&arg[4]);
            if(scale_factor < 0.0F) scale_factor = 0.0F - scale_factor;
            if(scale_factor > 60.0F) scale_factor = 4.0F;
            cuckoo = (u08) scale_factor;
//          if(ships_clock) cuckoo = 1;
         }
         return 0;
      }
      else if(d == 'i') {  // /ti - toggle 12/24 hour clock mode
         clock_12 = toggle_option(clock_12, e);
         return 0;
      }
      else if(d == 'j') {  // /tj - remove the smoothing the tbolt firmware does to the temperature sensor readings
         undo_fw_temp_filter = toggle_option(undo_fw_temp_filter, e);
         user_set_temp_filter = 1;
         return 0;
      }
      else if(d == 'k') {   // /tk - use degrees K
         DEG_SCALE = 'K';
         plot[TEMP].plot_center = NEED_CENTER;
         if(luxor) plot[TC2].plot_center = NEED_CENTER;
         return 0;
      }
      else if(d == 'm') {   // /tm - use meters
         alt_scale = "m";
         LLA_SPAN = 3.0;
         ANGLE_SCALE = ((2.74e-6)*FEET_PER_METER); // degrees per meter
         angle_units = "m";
         if((e == '=') || (e == ':')) {
            if(arg[4]) {
               LLA_SPAN = atof(&arg[4]);
               if(LLA_SPAN <= 0.0) LLA_SPAN = 3.0;
            }
            if(keyboard_cmd) {
               clear_lla_points();
               rebuild_lla_plot(0);
            }
         }
         return 0;
      }
      else if(d == 'n') {   // /tn - use degrees Newton
         DEG_SCALE = 'N';
         plot[TEMP].plot_center = NEED_CENTER;
         if(luxor) plot[TC2].plot_center = NEED_CENTER;
         return 0;
      }
      else if(d == 'o') {   // /to - use degrees Romer
         DEG_SCALE = 'O';
         plot[TEMP].plot_center = NEED_CENTER;
         if(luxor) plot[TC2].plot_center = NEED_CENTER;
         return 0;
      }
      else if(d == 'p') {  // /tp - show time as fractions of the day
         fraction_time = toggle_option(fraction_time, e);
         seconds_time  = 0;
         return 0;
      }
      else if(d == 'q') {  // /tq - show time as seconds of the day
         seconds_time = toggle_option(seconds_time, e);
         fraction_time = 0;
         return 0;
      }
      else if(d == 'r') {   // /tr - use degrees R
         DEG_SCALE = 'R';
         plot[TEMP].plot_center = NEED_CENTER;
         if(luxor) plot[TC2].plot_center = NEED_CENTER;
         return 0;
      }
      else if((d == 's') && (c == 'c')) {   // /tsc - don't use TSC for GetMsecs() and GetNsecs()
         if(((f == '=') || (f == ':')) && arg[5]) {
            use_tsc = (int) atof(&arg[5]);
         }
         else use_tsc ^= 1;
         return 0;
      }
      else if(d == 's') {   // /ts - set system time
         set_system_time = toggle_option(set_system_time, e);
         if(set_system_time) need_time_set();

         force_utc_time = 1;
         set_time_daily = set_time_hourly = set_time_minutely = 0;
         set_time_anytime = 0.0;
         if((e == 'm') || (e == 'h') || (e == 'd') || (e == 'a')) {  // hourly or daily or anytime millisecond is not 0
            if(set_system_time) {
               if     (e == 'd') set_time_daily = 1;
               else if(e == 'h') set_time_hourly = 1;
               else if(e == 'm') set_time_minutely = 1;
               else if(e == 'a') { // /tsa[=#msecs_drift_before_correcting
                  #ifdef WINDOWS
                     set_time_anytime = 40.0;  // default 40 milliseconds
                  #else  // __linux__  __MACH__
                     set_time_anytime = 10.0;  // default 10 milliseconds
                  #endif 
                  if(((arg[4] == '=') || (arg[4] == ':')) && arg[5]) {
                     set_time_anytime = atof(&arg[5]);
                  }
                  set_time_anytime = fabs(set_time_anytime);
               }
            }
         }
         else if(e == 'x') { // /tsx - set time message offset compensation from 1PPS
            time_sync_offset = (TIME_SYNC_AVG);  
            if(((arg[4] == '=') || (arg[4] == ':')) && arg[5]) {
               time_sync_offset = atof(&arg[5]);
            }
            user_set_tsx = 1;
         }
         else if(e == 'o') ;  // set time once
         else if(e == 'j') {  // tsj - show digital clock with Julian format
            show_julian_time = toggle_option(show_julian_time, f);
            show_msecs = 0;
         }
         else if(e == 'z') {  // tsz - show digital clock with milliseconds
            show_msecs = toggle_option(show_msecs, f);
            show_julian_time = 0;
         }
         else if(strstr(arg, "g")) force_utc_time = 0;  // set to gps time
         else if(strstr(arg, "G")) force_utc_time = 0;  // set to gps time
         else if(e) return c;
         return 0;
      }
#ifdef TEMP_CONTROL
      else if(d == 't') {  // /tt - set control temperature
         do_temp_control = 1;
         if(((e == '=') || (e == ':')) && arg[4]) {
            desired_temp = (float) atof(&arg[4]);
            if(desired_temp == 0.0F) ;
            else if(desired_temp > 50.0F) desired_temp = 40.0F;
            else if(desired_temp < 10.0F) desired_temp = 20.0F;
         }
         if(keyboard_cmd) {
            if(desired_temp) enable_temp_control();
            else             disable_temp_control();
         }
         return 0;
      }
#endif
      else if(d == 'u') {   // /tu - use UTC time
         ++set_utc_mode;                  
         set_gps_mode = 0;
         time_flags &= (~0x0001);
         user_set_time_mode = 'u';
         return 0;
      }
      else if(d == 'w') {  // /tw - Windows idle sleep time
         if(e == '=')      idle_sleep = (long) atof(&arg[4]);
         else if(e == ':') idle_sleep = (long) atof(&arg[4]);
         else if(e)        idle_sleep = (long) atof(&arg[3]);
         else              idle_sleep = DEFAULT_SLEEP;
         if(idle_sleep < 0) idle_sleep = 0 - idle_sleep;
         return 0;
      }
      else if(d == 'x') {   // /tx - use exponents on osc data
         show_euro_ppt = toggle_option(show_euro_ppt, e);
         set_osc_units();
         return 0;
      }
      else if(d == 'y') {   // /ty - round temperatures
         user_set_rounding = 1;
         if(e == '=')      round_temp = atoi(&arg[4]);
         else if(e == ':') round_temp = atoi(&arg[4]);
         else if(luxor)    round_temp = 3;
         else              round_temp = 3;
         return 0;
      }
      else if(d == '*') {   // /t* - monitor power line freq
         monitor_pl = toggle_option(monitor_pl, e);
         return 0;
      }
      else if(d == '\'') {   // /t' - use feet
         alt_scale = "ft";
         LLA_SPAN = 10.0;        // lla plot scale in feet each side of center
         ANGLE_SCALE = 2.74e-6;  // degrees per foot
         angle_units = "ft";
         if((e == '=') || (e == ':')) {
            if(arg[4]) {
               LLA_SPAN = atof(&arg[4]);
               if(LLA_SPAN <= 0.0) LLA_SPAN = 10.0;
            }
            if(keyboard_cmd) {
               clear_lla_points();
               rebuild_lla_plot(0);
            }
         }
         return 0;
      }
      else if(d == '"') {   // /t" - use d.m.s format for lat/lon
         dms = toggle_option(dms, e);
         return 0;
      }
      else return c;
   }
   else if(c == 'u') {  // set osc disciplining params or toggle queue Update mode
      if(d == 'c') {
         if(((e == '=') || (e == ':')) && arg[4]) {  // /uc - user set TT-UT1 delta T value
            user_delta_t = atof(&arg[4]);
            user_delta_t /= (24.0*60.0*60.0);
            user_set_delta_t = 1;
         }
         else user_set_delta_t = 0;
         need_sunrise = 1;
      }
      else if(d == 'd') {   // /ud - damping
         user_set_osc |= PARAM_DAMP;
         if(((e == '=') || (e == ':')) && arg[4]) cmd_damp = (float) atof(&arg[4]);
         else cmd_damp = 1.000F;
      }
      else if(d == 'j') {   // /uf - max freq error threshold
         user_set_osc |= PARAM_MAXFREQ;
         if(((e == '=') || (e == ':')) && arg[4]) cmd_maxfreq= (float) atof(&arg[4]);
         else cmd_maxfreq = 50.0F;
      }
      else if(d == 'g') {   // /ug - gain
         user_set_osc |= PARAM_GAIN;
         if(((e == '=') || (e == ':')) && arg[4]) cmd_gain = (float) atof(&arg[4]);
         else cmd_gain = 1.400F;
      }
      else if(d == 'h') {   // /uh - max allowed dac volts
         user_set_osc |= PARAM_MAXRANGE;
         if(((e == '=') || (e == ':')) && arg[4]) cmd_maxrange = (float) atof(&arg[4]);
         else cmd_minv = 5.000F;
      }
      else if(d == 'i') {   // /ui - initial dac volts
         user_set_osc |= PARAM_INITV;
         if(((e == '=') || (e == ':')) && arg[4]) cmd_initdac = (float) atof(&arg[4]);
         else cmd_initdac = 3.000F;
         initial_voltage = cmd_initdac;
      }
      else if(d == 'j') {   // /uj - jamsync threshold
         user_set_osc |= PARAM_JAMSYNC;
         if(((e == '=') || (e == ':')) && arg[4]) cmd_jamsync = (float) atof(&arg[4]);
         else cmd_jamsync = 300.0F;
      }
      else if(d == 'l') {   // /ul - min allowed dac volts
         user_set_osc |= PARAM_MINRANGE;
         if(((e == '=') || (e == ':')) && arg[4]) cmd_minrange = (float) atof(&arg[4]);
         else cmd_minv = (-5.000F);
      }
      else if(d == 'n') {   // /un - min dac volts
         user_set_osc |= PARAM_MINV;
         if(((e == '=') || (e == ':')) && arg[4]) cmd_minv = (float) atof(&arg[4]);
         else cmd_minv = 0.000F;
      }
      else if(d == 'o') {   // /uo - set UTC leapsecond offset (for receivers that don't send it)
         if(((e == '=') || (e == ':')) && arg[4]) {
            utc_offset = (int) atof(&arg[4]);
            have_utc_ofs = 104;
            user_set_utc_ofs = 5;
         }
      }
      else if(d == 'p') {   // /up - UCCM pullin range
         user_set_osc |= PARAM_PULLIN;
         if(((e == '=') || (e == ':')) && arg[4]) cmd_pullin = atoi(&arg[4]);
         else cmd_pullin = 300;
      }
      else if(d == 't') {   // /ut - time constant
         user_set_osc |= PARAM_TC;
         if(((e == '=') || (e == ':')) && arg[4]) cmd_tc = (float) atof(&arg[4]);
         else cmd_tc = 500.0F;
      }
      else if(d == 'x') {   // /ux - max dac volts
         user_set_osc |= PARAM_MAXV;
         if(((e == '=') || (e == ':')) && arg[4]) cmd_maxv = (float) atof(&arg[4]);
         else cmd_maxv = 5.000F;
      }
      else if(d == 0) {
         user_pause_data = toggle_option(user_pause_data, d);
         pause_data = user_pause_data;
      }
      else return c;
   }
   else if((c == 'v') && (d == 'f')) { // /vf - startup in full screen mode
      not_safe = 1;
      go_fullscreen = 1;
      need_screen_init = 1;
   }
   else if((c == 'v') && (d == 'i')) { // /vi - swap black and white (and tweak yellow)
      invert_screen();
   }
   else if(c == 'v') {   // /v - video screen size - Small,  Medium,  Large,  Xtra large, Huge
      not_safe = 1;
      screen_type = d;
      need_screen_init = 1;
      go_fullscreen = 0;
      user_font_size = 0;

      if(arg[3] == ':') user_font_size = 12;
      if(arg[3] && arg[4]) {
         strupr(arg);
         if     (strstr(&arg[4], "S")) user_font_size = 12;  // /vs - 
         else if(strstr(&arg[4], "N")) user_font_size = 12;  // /vn - 
         else if(strstr(&arg[4], "M")) user_font_size = 14;  // /vm -
         else if(strstr(&arg[4], "L")) user_font_size = 16;  // /vl -
         else if(strstr(&arg[4], "T")) user_font_size = 8;   // /vt - 
      }

      if(adjust_screen_options()) {
         screen_type = 'm';
         adjust_screen_options();
         return c;
      }

      if(arg[2] && arg[3] && arg[4]) {  // get video mode to use
         if(screen_type == 'c') {       // /vc - custom screen size
            custom_width = 1024;
            custom_height = 768;
            sscanf(&arg[4], "%d%c%d", &custom_width, &c, &custom_height);
         }
         else {                         // standard screen size
            sscanf(&arg[4], "%u", &user_video_mode);                     
            text_mode = 0;
            if(screen_type == 't') {
               text_mode = user_video_mode;
               if(text_mode == 0) text_mode = 2;
            }
         }
      }
   }
   else if(c == 'w') {   // name of log file to write
      not_safe = 1;
      if((d == 'w') || (d == 'l')) {  //  /ww - set log file or /wl - set log file
         log_mode = "w";
         if(e && arg[4]) strcpy(log_name, &arg[4]);
      }
      else if(d == 'a') { //  /wa - file append mode
         log_mode = "a";
         if(e && arg[4]) strcpy(log_name, &arg[4]);
      }
      else if(arg[2] && arg[3]) { //  /w - file write mode
         log_mode = "w";
         if(d) strcpy(log_name, &arg[3]);
      }
      user_set_log = 1;
   }
   else if(c == 'x') {
      if(d) {
         if((e == '=') || (e == ':')) {
            if(edit_osc_pid_value(d, &arg[4], 0)) return c;
         }
         else  {
            if(edit_osc_pid_value(d, &arg[3], 0)) return c;
         }
      }
      else return c;
   }
   else if(c == 'y') {
      not_safe = 1;
      if(arg[2] && arg[3]) {  // /y - get show interval
         strncpy(user_view_string, &arg[3], 32);
         user_view_string[32] = 0;
         user_view = 0;
         set_view = 0;
      }
      else {     // set plot window for 12 or 24 hours 
         if(day_plot == 24) day_plot = 12; 
         else day_plot = 24; 
         day_size = day_plot;
         user_view = day_plot;
         new_user_view = 1;
         ++set_view;
      }
   }
   else if(c == 'z') {  // /z - allow graphs to recenter each time the plot is redrawn
      if(d && ((arg[3] == '=') || (arg[3] == ':')) && arg[4]) {  // /zd=3 - set fixed center value for dac plot
         scale_factor = (float) atof(&arg[4]);
         if     (d == 'd') { plot[DAC].float_center  = 0;  plot[DAC].plot_center  = scale_factor; user_set_dac_float = 1; }
         else if(d == 'o') { plot[OSC].float_center  = 0;  plot[OSC].plot_center  = scale_factor/1000.0F; user_set_osc_float = 1; } 
         else if(d == 'p') { plot[PPS].float_center  = 0;  plot[PPS].plot_center  = scale_factor; user_set_pps_float = 1; } 
         else if(d == 't') { plot[TEMP].float_center = 0;  plot[TEMP].plot_center = scale_factor; } 
         else if((d >= '0') && (d <= '9')) { // !!!!!! luxor
            if(d == '0') d = 10;
            else         d = d - '0' - 1;
            d += FIRST_EXTRA_PLOT;
            plot[(int) d].float_center = 0;
            plot[(int) d].plot_center = scale_factor;
         }
         else return c;
      }
      else if(d) {  // /zd /zpt  - toggle graph floating reference mode
         if(d == 'a') {
            auto_scale = toggle_option(auto_scale, e);  // turn off auto scaling
            auto_center = auto_scale;
         }
         else if(d == 'd') plot[DAC].float_center  = toggle_option(plot[DAC].float_center, e);
         else if(d == 'o') plot[OSC].float_center  = toggle_option(plot[OSC].float_center, e); 
         else if(d == 'p') plot[PPS].float_center  = toggle_option(plot[PPS].float_center, e); 
         else if(d == 't') plot[TEMP].float_center = toggle_option(plot[TEMP].float_center, e); 
         else if((d >= '0') && (d <= '9')) { // !!!!!! luxor
            if(d == '0') d = 10;
            else         d = d - '0' - 1;
            d += FIRST_EXTRA_PLOT;
            plot[(int)d].float_center  = toggle_option(plot[(int)d].float_center, e);
         }
         else return c;
      }
      else {  // just /z - toggle dac and temp floating reference mode
         plot[TEMP].float_center = toggle_option(plot[TEMP].float_center, d); 
         plot[DAC].float_center = toggle_option(plot[TEMP].float_center, d); 
      }
   }
   else if(c == '+') {  // /+ - positive PPS polarity
      not_safe = 1;
      ++set_pps_polarity;                  
      user_pps_polarity = 0;
      user_pps_enable = 1;
   }
   else if(c == '-') {  // /- - negative PPS polarity
      not_safe = 1;
      ++set_pps_polarity;                  
      user_pps_polarity = 1;
      user_pps_enable = 1;
   }
   else if(c == '^') {  // /^ - toggle OSC signal polarity (referenced to the PPS signal)
      if(d == 'r') {    //^r
         not_safe = 1;
         ++set_osc_polarity;                  
         user_osc_polarity = 0;
      }
      else if(d == 'f') { //^f
         not_safe = 1;
         ++set_osc_polarity;                  
         user_osc_polarity = 1;
      }
      else {
         not_safe = 1;
         ++set_osc_polarity;                  
         user_osc_polarity = toggle_option(user_osc_polarity, d);
      }
   }
   else {   // not a command that we know about
      return c;
   }

   return 0;  // we did something useful
}

int option_switch(char *arg)  
{   
char c, d;

   // process a command line option parameter
   d = 0;
   c = tolower(arg[1]);
   if(c) d = tolower(arg[2]);

   if((arg[0] == '=') || (arg[0] == '$')) {  // it is a hex byte to send to the tbolt
      save_cmd_bytes(arg);
   }
   else if(c == '0') {  // toggle com port processing flag
      process_com = toggle_option(process_com, d);
      if(process_com == 0) com_port = usb_port = 0;
      last_com_time = this_msec - com_timeout;
   }
   else if(isdigit(c)) {  // com port number
      strupr(arg);
      if     (strstr(arg, "P")) lpt_port = atoi(&arg[1]);  // parallel port used for temp control
      else if(strstr(arg, "T")) lpt_port = atoi(&arg[1]);  // parallel port used for temp control 
#ifdef WINDOWS
#else // __linux__  __MACH__
      else if(strstr(arg, "U")) {  // linux ttyUSB#
         usb_port = atoi(&arg[1]); 
         com_port = 0;
      }
#endif
      else {
         com_port = atoi(&arg[1]);
         usb_port = 0;
      }
      process_com = 1;
      if(com_running) {
         init_com();
      }
      last_com_time = this_msec - com_timeout;
   }
   // we break up the command line processing big 'if' statement 
   // into two parts so the DOS compiler heap does not overflow
   else if((c >= 'a') && (c <= 'o')) {   // first half of command line options
      return cmd_a_to_o(arg);           
   }
   else {                                // all the other command line options
      return cmd_other(arg);
   }

   return 0;  // we did something useful
}

