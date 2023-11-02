#define EXTERN extern
#include "heather.ch"

//  Thunderbolt TSIP monitor
//
//  Copyright (C) 2008-2018 Mark S. Sims - all rights reserved
//  Win32 port by John Miles, KE5FX (john@miles.io)
//
//
//  This file contains most of the user interface stuff.
//  Abandon all hope, ye mortals who dain to understand it...
//  It makes my noggin throb,  and I wrote it...
//

int all_plots;
extern char *dst_list[];
extern char szAppName[];
extern DATA_SIZE k6, k7, k8, integrator;
extern double osc_k6, osc_k7, osc_k8, osc_integrator;
extern int first_request;

#define DEFAULT_DEGLITCH_SIGMA 3.0  // standard deviation sigma to use when deglitching plots

extern char degc[];     // degrees symbols
extern char degk[];
extern char degs[];

#define GRAPH_LLA 1     // if 1,  make graphs 1,2,3 lat/lon/alt if rcvr is in 3D mode

#define NEED_NEW_QUEUES     2  // return values for string_param()
#define NEED_SCREEN_REDRAW  1


// text editing stuff
u08 first_edit;
u08 insert_mode;
u08 edit_cursor;
u08 no_auto_erase;
int edit_err_flag;
int e_row;
char mode_string[SLEN+1];     // the last driver mode change string used

int getting_plot;

u08 old_show_plots[NUM_PLOTS+DERIVED_PLOTS];  // used to save/restore plot enables when toggling jitter measurement mode
u08 old_plot_azel; 
u08 old_plot_watch; 

DATA_SIZE scale_step;
DATA_SIZE center_step;

int cal_adjust;   // used to adjust calendar by user specified "x" months or years


void set_steps(void);

// codes for identifiing edit strings (i.e. the getting_string variable)
#define ADEV_CMD         'a'
#define OSCPID_CMD       'b'
#define DACV_CMD         'c'
#define SCALE_CMD        'd'
#define ELEV_CMD         'e'
#define FILTER_CMD       'f'
#define COLOR_CMD        'g'
#define CHIME_CMD        'h'
#define QUEUE_INT_CMD    'i'
#define LOG_INT_CMD      'j'
#define TEMP_PID_CMD     'k'
#define SET_LLA_CMD      'l'
#define SET_SIGMASK_CMD  'm'
#define LOG_CMD          'n'
#define FOLIAGE_CMD      'o'
#define SET_PDOP_CMD     'p'
#define READ_CMD         'r'
#define SURVEY_CMD       's'
#define MIN_RANGE_CMD    't'
#define DYNAMICS_CMD     'u'
#define VIEW_CMD         'v'
#define WRITE_CMD        'w'
#define SINGLE_SAT_CMD   'x'

#define ABORT_SURV_CMD   'A'  // capital letters used to be reserved for ignore_blank_line() commands
#define CENTER_CMD       'C'
#define TITLE_CMD        'H'
#define ABORT_LLA_CMD    'L'
#define SIGNAL_CMD       'Q'
#define MAX_RANGE_CMD    'T'
#define SET_TZ_CMD       'Z'

#define DRIFT_CMD        '='
#define SCREEN_CMD       '$'
#define STAT_CMD         '~'
#define EXIT_CMD         '/'
#define ALARM_CMD        ':'
#define OPTION_CMD       ','
#define SWITCH_CMD       '-'
#define SCREEN_DUMP_CMD  '!'
#define LOG_DUMP_CMD     '('
#define TEMP_SET_CMD     '*'
#define WRITE_SIGS_CMD   '%'
#define PRECISE_SURV_CMD '^'

#define TC_CMD           '1'    // 1 .. 8 are oscillator disciplining parameters
#define DAMP_CMD         '2'
#define GAIN_CMD         '3'
#define MINV_CMD         '4'
#define MAXV_CMD         '5'
#define JAMSYNC_CMD      '6'
#define MAX_FOSC_CMD     '7'
#define INITV_CMD        '8'

#define CABLE_CMD        '9'

// extended codes for identifiing edit strings (getting_string variable)
#define DRVR_CMD            0x0100   // luxor driver mode change
#define PC_CMD              0x0101   // load protection overcurrent
#define PH_CMD              0x0102   // battery HVC
#define PL_CMD              0x0103   // battery LVC
#define PM_CMD              0x0104   // message watchdog timeout
#define PO_CMD              0x0105   // battery overcurrent
#define PP_CMD              0x0106   // load overwatts
#define PR_CMD              0x0107   // protection fault reset
#define PS_CMD              0x0108   // temp2 overtemp
#define PT_CMD              0x0109   // temp1 overtemp
#define PU_CMD              0x010A   // load undervoltage
#define PV_CMD              0x010B   // load overvoltage
#define PX_CMD              0x010C   // auxv overvoltage
#define PW_CMD              0x010D   // battery overwatts
#define PZ_CMD              0x010E   // auxv undervoltage
#define WC_CMD              0x010F   // write config data to file
#define S_CMD               0x0110   // set luxor lat, lon, alt
#define SCRIPT_RUN_CMD      0x0111   // run keyboard script on a schedule
#define EXEC_PGM_CMD        0x0112   // run keyboard script on a schedule

#define AMPL_CMD            0x0120   // lux sensitivity
#define AMPU_CMD            0x0121   // lumen sensitivity
#define AMPE_CMD            0x0122   // IR1 emissivity
#define AMPI_CMD            0x0123   // IR2 emissivity
#define AMPS_CMD            0x0124   // serial number
#define AMPV_CMD            0x0125   // reference voltage

#define BC_CMD              0x0130   // constant current load mode
#define BF_CMD              0x0131   // 3.60V LiFePO4 charge mode
#define BH_CMD              0x0132   // high voltage lipo charge mode
#define BL_CMD              0x0133   // 4.20V lipo charge mode
#define BP_CMD              0x0134   // battery PWM resolution (8/9/10 bit)
#define BR_CMD              0x0135   // pwm sweep rate
#define BS_CMD              0x0136   // pwm sweep
#define BV_CMD              0x0137   // constant load voltage mode
#define BW_CMD              0x0138   // constant load wattage mode

#define CAL_CMD             0x0140   // calibration constants

#define DEBUG_LOG_CMD       0x0150   // open debug log (debug_file)
#define RAW_LOG_CMD         0x0151   // open raw receiver data log (raw_file)
#define TICC_LOG_CMD        0x0152   // open raw TICC data log (ticc_file)
#define RUN_PGM_CMD         0x0153   // run a program
#define MONITOR_CMD         0x0154   // monitor port command
#define TERM_CMD            0x0155   // terminal mode command
#define VECTOR_FONT_CMD     0x0156   // enable scaled vector fonts
#define STOP_SURVEY_CMD     0x0157   // stop standard survey
#define KBD_TIMEOUT_CMD     0x0158   // keyboard idle timeout
#define SET_BAUD_CMD        0x0159   // set receiver com port baud rate
#define SET_RAW_CMD         0x015C   // set satellite raw observation rate
#define LOG_CLOSE_CMD       0x015D   // close log file
#define PRN_LOG_CMD         0x015E   // open/close sat PRN info log

#define PALETTE_CMD         0x0160   // edit color palette
#define DEGLITCH_CMD        0x0161   // set plot deglitch sigma value
#define DEGLITCH_ALL_CMD    0x0162   // set plot deglitch sigma value

#define ADEV_BIN_CMD        0x0170   // adev bin sequence
#define ADEV_HIDE_CMD       0x0171   // hide adev plots
#define ADEV_RECALC_CMD     0x0172   // recalc adev plots
#define TRIM_QUEUE_CMD      0x0173   // remove data from plot queue

#define CALC_CMD            0x0180   // MUST match value in heather.ch
#define EDIT_DEFINE_CMD     0x0181
#define SHOW_CALENDAR_CMD   0x0182

#define PROP_DELAY_CMD      0x0190   // calculate propogation delay

#define TRAIM_CMD           0x0200   // traim threshold
#define PPS_OFS_CMD         0x0201   // simple pps offset
#define ZODIAC_RESTART      0x0202   // reset Zodiac receiver into Motorola mode
#define USER_CMD            0x0203   // send user command to receiver
#define TSX_CMD             0x0204   // set time_sync_offset
#define PPS_OFS1_CMD        0x0210   // PPS1 offset delay
#define PPS_OFS2_CMD        0x0211   // PPS2 offset delay
#define PPS1_CFG_CMD        0x0212
#define PPS2_CFG_CMD        0x0213
#define REF_CMD             0x0214   // GPSDO reference source
#define ATTEN_CMD           0x0215   // attenuator
#define ANTENNA_CMD         0x0216   // antenna monitoring

#define SAT_IGN_CMD         0x0220   // ignore satellite command
#define GNSS_CMD            0x0230   // GNSS system select command
#define SI_CMD              0x0240   // sat info display count
#define PLOT_PRN_CMD        0x0241   // plot sat prn az/el/sig level
#define TRACK_PORT_CMD      0x0242   // select data to send out the TRACK_PORT

#define SORT_CMD            0x0250   // sort sat info display
#define TIDE_CMD            0x0251   // control tide/gravity plots
#define SAT_PLOT_CMD        0x0252   // control sat count plot

#define BAUD_CMD            0x0260   // set serial com port params
#define NVS_FILTER_CMD      0x0270   // NVS_RCVR solution filtration factor
#define NAV_RATE_CMD        0x0280   // navigation update rate
#define SUN_CMD             0x0290   // sunrise/sunset calculation type
#define DELTA_T_CMD         0x02A0   // set TT-UT1 delta T
#define MARINE_CMD          0x02B0   // Motorola marine velocity filter
#define PULLIN_CMD          0x02C0   // UCCM pullin-range
#define EDITOR_CMD          0x02D0   // spawn text editor program
#define SET_LLA_REF_CMD     0x02E0   // set lla scattergram reference point

#define TIME_CODE_CMD       0x02F0   // set time code format
#define UTC_OFS_CMD         0x02F1   // set default utc offset
#define REVERT_SEG_CMD      0x02F2   // revert EEPROM segment(s)
#define TRIMBLE_LLA_CMD     0x02F3   // save position on Trimble receivers using multiple single point surveys

#define BLINK_PRN_CMD       0x02F8   // select a sat to blink in the sat map

#define TICC_MODE_CMD       0x0300   // TICC mode
#define TICC_EDGE_CMD       0x0301   // TICC trigger edges
#define TICC_FUDGE_CMD      0x0302   // TICC fudge factors
#define TICC_TIME2_CMD      0x0303   // TICC time2 factors
#define TICC_SYNC_CMD       0x0304   // TICC sync mode
#define TICC_CAL_CMD        0x0305   // TICC cal periods
#define TICC_DILAT_CMD      0x0306   // TICC dilation factors
#define TICC_SPEED_CMD      0x0307   // TICC ref clock speed
#define TICC_COARSE_CMD     0x0308   // TICC coarse clock speed
#define TICC_TIMEOUT_CMD    0x0309   // TICC coarse clock speed
#define TICC_TUNE_CMD       0x030A   // TICC autotune command
#define TICC_FREQ_CMD       0x030B   // TICC input nominal freq
#define TRUE_TUNE_CMD       0x030C   // Trueposition autotune command

#define PHASE_WRAP_CMD      0x0310   // set phase wrap interval
#define TS_WRAP_CMD         0x0311   // set timestamp wrap interval

#define CS_DISP_CMD         0x0320   // set HP5071A display
#define CS_REMOTE_CMD       0x0321   // set HP5071A remote mode
#define CS_STANDBY_CMD      0x0322   // set HP5071A standby mode
#define CS_SYNC_CMD         0x0323   // set HP5071A sync mode
#define CS_TIMESET_CMD      0x0324   // set HP5071A time
#define CS_LEAP_CMD         0x0326   // set HP5071A leapsecond
#define CS_STER_CMD         0x0327   // set HP5071A frequency steering
#define CS_SLEW_CMD         0x0328   // set HP5071A clock slew

#define PRS_SAVE_CMD        0x0340   // save PRS-10 param(s) into eeprom
#define PRS_MO_CMD          0x0341   // PRS-10 magnetic offset
#define PRS_MS_CMD          0x0342   // PRS-10 magnetic switching
#define PRS_SF_CMD          0x0343   // PRS-10 freq offset
#define PRS_SP_CMD          0x0344   // PRS-10 synthesizer command
#define PRS_TO_CMD          0x0345   // PRS-10 time tag offset

#define X72_FXO_CMD         0x0380   // X72 FXO output enable
#define X72_ACMOS_ENAB_CMD  0x0381   // X72 ACMOS output enable
#define X72_SINE_CMD        0x0382   // X72 Sine output enable
#define X72_EFC_CMD         0x0383   // X72 EFC input enable
#define X72_TIC_CMD         0x0384   // X72 set TIC value
#define X72_FREQ_CMD        0x0385   // X72 set ACMOS freq divider value
#define X72_DDS_CMD         0x0386   // X72 set DDS freq tune value
#define X72_TSAVE_CMD       0x0387   // X72 save tune value in EEPROM
#define X72_TUNE_CMD        0x0388   // auto-tune X72
#define X72_HOLDOVER_CMD    0x0389   // set holdover analysis time
#define X72_OSC_CMD         0x038A   // X72 master oscillator freq

#define SRO_WIDTH_CMD       0x03A0   // PPS output pulse width
#define SRO_DELAY_CMD       0x03A1   // PPS output pulse delay
#define SRO_ALARM_CMD       0x03A2   // alarm window width
#define SRO_WINDOW_CMD      0x03A3   // tracking window width
#define SRO_RAW_CMD         0x03A4   // raw phase adjust
#define SRO_FC_CMD          0x03A5   // frequency correction
#define SRO_SY_CMD          0x03A6   // sync mode
#define SRO_TR_CMD          0x03A7   // track mode
#define SRO_FS_CMD          0x03A8   // freq save mode
#define SRO_CO_CMD          0x03A9   // fine phase comparator offset
#define SRO_GF_CMD          0x03AA   // gofast mode (only for later firmware)

#define LPFRS_FREQ_CMD      0x03B0   // frequency adjustment

#define TM4_PPS_CMD         0x03B8   // TM4 pps source

#define RT17_OFS_CMD        0x03B8   // RT17 clock offset enable

#define STAR_SET_WTR_CMD    0x03C0   // wait-to-restore time
#define STAR_CLEAR_WTR_CMD  0x03C1   // wait-to-restore time
#define STAR_SET_HBSQ_CMD   0x03C2   // hbsq time in minutes
#define STAR_TS_CMD         0x03D2   // fixup timestamp errors

#define SMOOTHING_CMD       0x03D0   // Furuno smoothing filter

#define RINEX_SITE_CMD      0x0400
#define RINEX_FORMAT_CMD    0x0401
#define RINEX_ANT_TYPE_CMD  0x0402
#define RINEX_ANT_NUM_CMD   0x0403
#define RINEX_HEIGHT_CMD    0x0404
#define MARKER_NAME_CMD     0x0405
#define MARKER_NUM_CMD      0x0406
#define RINEX_LIST_CMD      0x0407
#define RINEX_FIX_CMD       0x0408
#define RINEX_FILE_CMD      0x0410   // open RINEX file
#define RINEX_CLOSE_CMD     0x0411   // close RINEX file
#define RINEX_CPPR_CMD      0x0412   // derive L1 pseduorange from carrier phase data

#define RTK_MODE_CMD        0x0420

#define SA35_FREQ_CMD       0x0430   // frequency adjustment


void edit_plot(int id, int c);
int rpn_calc(void);

int rinex_list;


//
//
//   RPN calculator stuff
//
//

void rpn_help();

#define ITYPE u64   // logical operations data type
#define ISIZE 64    // logical operation bits (actually 54 valid bits)

#define RPN_MEM  100      // number of memory registers
#define MAX_RPN_SIZE 16   // max stack size
int RPN_SIZE = 6;         // current stack size

double rpn[MAX_RPN_SIZE+1]; // value stack
double rpn_mem[RPN_MEM+1];  // memory registers

double last_x;
int last_was_clx;
double last_rpn0;

double deg_mode = (180.0/PI);
int hex_mode;
int rpn_format = 8;
int comma_fmt;
int eng_mode;
int skip_next_rpn;
int rpn_break;

#define MAX_RPN_DEFS 100          // max number of user defined operations
char *rpn_defs[MAX_RPN_DEFS+1];   // user defined operation strings
int num_rpn_defs;                 // how many user operations have been defined

#define RPN_EXEC_DEPTH  10        // max nesting depth of user defined operations
struct RPN_EXEC_STACK {           // user operations nesting stack
   char msg[NMEA_MSG_SIZE+1];     //   commands
   int col;                       //   where we last processed
   int skip;                      //   skip operation flag
} rpn_exec_stack[RPN_EXEC_DEPTH+1];
int rpn_exec_level;               // how deep DEFINE execution is currently nested


void start_calc_zoom(int why)
{
   // start calculator in zoom display mode
   add_kbd('z');
   add_kbd('`');
}

void show_rpn_defines(int row, int col)
{
int i;

   // display user defined operations (limited by screen height)
   strcpy(out, "User DEFINEd operations:  define edit  savedefs saveall showdefs  run");
   vidstr(row++, 0, WHITE, out);

   if(num_rpn_defs) {
      for(i=0; i<num_rpn_defs; i++) {  // show the DEFINEs
         if(rpn_defs[i]) {
            sprintf(out, "(%s)", rpn_defs[i]);
            vidstr(row++,col, WHITE, out);
            if(row >= (TEXT_ROWS-1)) break;   // !!!!could use PLOT_ROW or MOUSE_ROW
         }
      }
   }
}

void rpn_help()
{
int row,col;

   // display calculator help
   col = 0;
   row = 0;

   vidstr(row++, col, WHITE, "Math:    + - * / chs inv mod abs int frac max min");
   vidstr(row++, col, WHITE, "         ctof ftoc (temperature conversion)");
   vidstr(row++, col, WHITE, "         mtof ftom (meters / feet conversion)");
   vidstr(row++, col, WHITE, "         rn (resistor noise nV/sqrt(Hz) - Y=degrees C  X=ohms)");
   vidstr(row++, col, WHITE, "Trig:    sin cos tan asin acos atan atan2 deg rad");
   vidstr(row++, col, WHITE, "         dtor rtod rtop ptor diag dist gcd bearing dms dec");
   vidstr(row++, col, WHITE, "Powers:  sqrt sqr exp powe pow10 pow **   ln log");
   vidstr(row++, col, WHITE, "Logic:   ~ & | ^  >># <<#  (#=shift count)");
   vidstr(row++, col, WHITE, "Values:  lastx pi e c k h t0 lat lon alt lla dac temp pps osc");
   vidstr(row++, col, WHITE, "         dop pdop hdop vdop gdop tdop edop xdop ydop");
   vidstr(row++, col, WHITE, "         tfom ffom cable elmask amu tc damp gain initv");
   vidstr(row++, col, WHITE, "Times:   date time utc local gps mjd gtime utime greg secs spd spw dow ti");
   vidstr(row++, col, WHITE, "         tz tzjd leap week tow epoch   rise noon set");
   vidstr(row++, col, WHITE, "Sats:    az# el# sig# doppler# range# phase#  prn#  (#=sat PRN)");
   vidstr(row++, col, WHITE, "Stack:   ex enter clx cls swap# stack# roll# down# up# drop# size# (#=count)");
   vidstr(row++, col, WHITE, "Memory:  rcl# rcl@# rcl+# rcl-# rcl*# rcl/#  (#=register num)");
   vidstr(row++, col, WHITE, "         sto# sto@# sto+# sto-# sto*# sto/#  clm clear");
   vidstr(row++, col, WHITE, "Compare: x=y x<>y x<y x<=y x>y x>=y   x=# x<># x<# x<=# x># x>=#");
   vidstr(row++, col, WHITE, "         isz dsz   nop  break  again  (#=value)");
   vidstr(row++, col, WHITE, "Format:  fix# sci# exp# hex# oct# bin# comma#  zoom  help ?  (#=decimals)");

   show_rpn_defines(row, 9);
}

int rpn_err_flag;   // flag set if an error occurs on calculator line execution

void rpn_error(char *s)
{
   // flash calculator error message on the srceen
   if(s == 0) return;
   erase_screen();
   vidstr(0,0, YELLOW, s);
   refresh_page();
   BEEP(7834);
   Sleep(1000);

   edit_buffer[0] = ' ';
   edit_buffer[1] = 0;
   rpn_err_flag = 1;
   start_calc(1234);
}

void rpn_clear(int flag)
{
int i;

   // clear stack and/or memory

   if(flag & 0x01) {  // clear stack
      for(i=0; i<RPN_SIZE; i++) {
         rpn[i] = 0.0;
      }
   }

   if(flag & 0x02) { // clear memory
      for(i=0; i<RPN_MEM; i++) {
         rpn_mem[i] = 0.0;
      }
   }

   last_x = 0.0;
   last_rpn0 = 0.0;
   last_was_clx = 0;
}

void rpn_down()
{
int i;
int count;
double val;

   // rotate stack down
   last_was_clx = 0;

   count = 1;
   if(isdigit(msg_field[4])) count = atoi(&msg_field[4]);
   if(count < 0) count = 0-count;
   count %= RPN_SIZE;

   while(count--) {
      val = rpn[0];
      for(i=0; i<RPN_SIZE-1; i++) {
         rpn[i] = rpn[i+1];
      }
      rpn[RPN_SIZE-1] = val;
   }
}

void rpn_up()
{
int i;
int count;
double val;

   // rotate stack up
   last_was_clx = 0;

   count = 1;
   if(isdigit(msg_field[2])) count = atoi(&msg_field[2]);
   if(count < 0) count = 0-count;
   count %= RPN_SIZE;

   while(count--) {
      val = rpn[RPN_SIZE-1];
      for(i=RPN_SIZE-1; i; i--) {
         rpn[i] = rpn[i-1];
      }
      rpn[0] = val;
      last_rpn0 = val;
   }
}

double rpn_top()
{
double val;
int i;

   // get stack top into last_x and drop the stack one level
   // (used to prepare for a two operand command)
   last_was_clx = 0;

   val = rpn[0];
   last_x = last_rpn0 = val;

   for(i=0; i<RPN_SIZE-1; i++) {
      rpn[i] = rpn[i+1];
   }
   return val;
}

void rpn_drop()
{
int count;
double last;

   // drop stack down
   last_was_clx = 0;

   count = 1;
   if(isdigit(msg_field[4])) count = atoi(&msg_field[4]);
   if(count < 0) count = 0-count;
   count %= RPN_SIZE;

   last = rpn[0];
   while(count--) {
      rpn_top();
   }
   last_x = last;
}

void rpn_push(double val)
{
int i;

   // push val onto stack
   if(last_was_clx == 0) {
      for(i=RPN_SIZE-1; i; i--) {
         rpn[i] = rpn[i-1];
      }
   }
   rpn[0] = val;
   last_was_clx = 0;
}

void rpn_inc(double val)
{
double last;

   // does ISZ and DSZ commands
   last = rpn[0];
   rpn[0] += val;
   if(rpn[0] == 0) skip_next_rpn = 1;
   last_x = last;
   last_was_clx = 0;
}

void rpn_stack()
{
int count;
double val;
double last;

   // push the value at a specified stack location onto the stack

   count = 1;
   if(isdigit(msg_field[5])) count = atoi(&msg_field[5]);
   if(count < 0) count = 0-count;
   count %= RPN_SIZE;

   val = rpn[count];

   last = rpn[0];
   rpn_push(val);
   last_x = last;
   last_was_clx = 0;
}

void rpn_swap()
{
int count;
double val;
double last;

   // swap X and a value on the stack

   count = 1;
   if(isdigit(msg_field[4])) count = atoi(&msg_field[4]);
   if(count < 0) count = 0-count;
   count %= RPN_SIZE;

   last = rpn[0];

   val = rpn[0];
   rpn[0] = rpn[count];
   rpn[count] = val;

   last_x = last;
   last_was_clx = 0;
}

void rpn_new(double val)
{
   // replace top of stack and lastx with new value

   last_x = rpn[0];
   last_rpn0 = val;
   rpn[0] = val;
   last_was_clx = 0;
}

void rpn_set(double val)
{
   // replace top of stack with new value

// last_x = rpn[0];
   last_rpn0 = val;
   rpn[0] = val;
   last_was_clx = 0;
}

void rpn_store()
{
int i;
int sign;

   // save top of stack into a memory location
   sign = msg_field[3];
   i = 0;
   if(isdigit(sign)) i = atoi(&msg_field[3]);
   else if(sign) i = atoi(&msg_field[4]);
   if(i < 0) i = 0 - i;

   sprintf(out, "Memory index out of range 0..%d", RPN_MEM);
   if(i >= RPN_MEM) rpn_error(out);
   else if(sign == '+') rpn_mem[i] += rpn[0];
   else if(sign == '-') rpn_mem[i] -= rpn[0];
   else if(sign == '*') rpn_mem[i] *= rpn[0];
   else if(sign == '/') rpn_mem[i] /= rpn[0];
   else if(sign == '@') {  // indirect
      i = (int) rpn_mem[i];
      i = i % RPN_MEM;
      rpn_mem[i] = rpn[0];
   }
   else rpn_mem[i] = rpn[0];
   last_was_clx = 0;
}

void rpn_recall()
{
int i;
int sign;

   // push memory location onto stack
   sign = msg_field[3];
   i = 0;
   if(isdigit(sign)) i = atoi(&msg_field[3]);
   else if(sign) i = atoi(&msg_field[4]);
   if(i < 0) i = 0 - i;

   sprintf(out, "Memory index out of range 0..%d", RPN_MEM);
   if(i >= RPN_MEM) rpn_error(out);
   else if(sign == '+') rpn_push(rpn[0]+rpn_mem[i]);
   else if(sign == '-') rpn_push(rpn[0]-rpn_mem[i]);
   else if(sign == '*') rpn_push(rpn[0]*rpn_mem[i]);
   else if(sign == '/') rpn_push(rpn[0]/rpn_mem[i]);
   else if(sign == '@') {  // indirect
      i = (int) rpn_mem[i];
      i = i % RPN_MEM;
      rpn_push(rpn_mem[i]);
   }
   else rpn_push(rpn_mem[i]);
   last_was_clx = 0;
}

void rpn_max()
{
double last;
double x0,y0;
double val;

   // maximum of x and y

   last = rpn[0];

   x0 = rpn_top();
   y0 = rpn_top();
   if(x0 > y0) val = x0;
   else        val = y0;
   rpn_push(val);

   last_x = last;
   last_was_clx = 0;
}

void rpn_min()
{
double last;
double x0,y0;
double val;

   // minimum of x and y

   last = rpn[0];

   x0 = rpn_top();
   y0 = rpn_top();
   if(x0 < y0) val = x0;
   else        val = y0;
   rpn_push(val);

   last_x = last;
   last_was_clx = 0;
}

void rpn_rtop()
{
double x, y;

   // rectangular to polar

   last_x = rpn[0];
   x = rpn[0];
   y = rpn[1];
   rpn[0] = sqrt(x*x + y*y);
   rpn[1] = atan2(y,x) * deg_mode;
   last_rpn0 = rpn[0];
   last_was_clx = 0;
}

void rpn_ptor()
{
double r, theta;

   // polar to rectangular

   last_x = rpn[0];
   r = rpn[0];
   theta = rpn[1];
   rpn[0] = r * cos(theta/deg_mode);
   rpn[1] = r * sin(theta/deg_mode);
   last_rpn0 = rpn[0];
   last_was_clx = 0;
}

void rpn_rn()
{
double temp, ohms;
double val;

   // resistor noise (in nV/sqrt(Hz))
   ohms = rpn_top();
   temp = rpn[0] - ABS_ZERO;

   val = sqrt(4.0 * KB * temp * ohms);
   rpn_set(val*1.0E9);

   last_was_clx = 0;
}

void rpn_az(int i)
{
double val;

   // push sat azimuth

   if(i == 0) {
      i = highest_sat();
      if(isdigit(msg_field[2])) {
         i = atoi(&msg_field[2]);
      }
   }
   if(i < 0) i = 0 - i;

   sprintf(out, "PRN out of range 0..%d", SUN_MOON_PRN);
   if(i > SUN_MOON_PRN) rpn_error(out);
   else if(NO_SATS || (sat[i].level_msg == 0)) {
      rpn_error("Satellite info not available");
   }
   else {
      val = sat[i].azimuth;
      if(deg_mode == 1.0) val = val * PI / 180.0;
      rpn_push(val);
   }
   last_was_clx = 0;
}

void rpn_el(int i)
{
double val;

   // push sat elevation

   if(i == 0) {
      i = highest_sat();
      if(isdigit(msg_field[2])) {
         i = atoi(&msg_field[2]);
      }
   }
   if(i < 0) i = 0 - i;

   sprintf(out, "PRN out of range 0..%d", SUN_MOON_PRN);
   if(i > SUN_MOON_PRN) rpn_error(out);
   else if(NO_SATS || (sat[i].level_msg == 0)) {
      rpn_error("Satellite info not available");
   }
   else {
      val = sat[i].elevation;
      if(deg_mode == 1.0) val = val * PI / 180.0;
      rpn_push(val);
   }
   last_was_clx = 0;
}

void rpn_sig(int i)
{
   // push sat sig level

   if(i == 0) {
      i = highest_sat();
      if(isdigit(msg_field[3])) {
         i = atoi(&msg_field[3]);
      }
   }
   if(i < 0) i = 0 - i;

   sprintf(out, "PRN out of range 0..%d", SUN_MOON_PRN);
   if(i > SUN_MOON_PRN) rpn_error(out);
   else if(NO_SATS || (sat[i].level_msg == 0)) {
      rpn_error("Satellite info not available");
   }
   else rpn_push(sat[i].sig_level);
   last_was_clx = 0;
}

void rpn_doppler(int i)
{
   // push sat doppler

   if(i == 0) {
      i = highest_sat();
      if(isdigit(msg_field[7])) {
         i = atoi(&msg_field[7]);
      }
   }
   if(i < 0) i = 0 - i;

   sprintf(out, "PRN out of range 0..%d", SUN_MOON_PRN);
   if(i > SUN_MOON_PRN) rpn_error(out);
   else if(NO_SATS || (sat[i].level_msg == 0)) {
      rpn_error("Satellite info not available");
   }
   else rpn_push(sat[i].doppler);
   last_was_clx = 0;
}

void rpn_range(int i)
{
   // push sat pseudorange

   if(i == 0) {
      i = highest_sat();
      if(isdigit(msg_field[5])) {
         i = atoi(&msg_field[5]);
      }
   }
   if(i < 0) i = 0 - i;

   sprintf(out, "PRN out of range 0..%d", SUN_MOON_PRN);
   if(i > SUN_MOON_PRN) rpn_error(out);
   else if(NO_SATS || (sat[i].level_msg == 0)) {
      rpn_error("Satellite info not available");
   }
   else rpn_push(sat[i].range);
   last_was_clx = 0;
}

void rpn_phase(int i)
{
   // push sat code or carrier phase

   if(i == 0) {
      i = highest_sat();
      if(isdigit(msg_field[5])) {
         i = atoi(&msg_field[5]);
      }
   }
   if(i < 0) i = 0 - i;

   sprintf(out, "PRN out of range 1..%d", SUN_MOON_PRN);
   if(i > SUN_MOON_PRN) rpn_error(out);
   else if(i <= 0) rpn_error(out);
   else if(NO_SATS || (sat[i].level_msg == 0)) {
      rpn_error("Satellite info not available");
   }
   else rpn_push(sat[i].code_phase);
   last_was_clx = 0;
}

void rpn_prn(int i)
{
   // push all info for a sat prn onto the stack

   if(i == 0) {
      i = highest_sat();
      if(isdigit(msg_field[3])) {
         i = atoi(&msg_field[3]);
      }
   }
   if(i < 0) i = 0 - i;

   sprintf(out, "PRN out of range 0..%d", SUN_MOON_PRN);
   if((i == 0) || (i > SUN_MOON_PRN)) rpn_error(out);
   else if(NO_SATS || (sat[i].level_msg == 0)) {
      rpn_error("Satellite info not available");
   }
   else {
      rpn_phase(i);
      rpn_range(i);
      rpn_doppler(i);
      rpn_sig(i);
      rpn_el(i);
      rpn_az(i);
   }
   last_was_clx = 0;
}


void rpn_dist()
{
double last;
double x0,y0;
double x1,y1;
double val;

   // linear distance between two points

   last = rpn[0];
   x0 = rpn_top();
   y0 = rpn_top();
   x1 = rpn_top();
   y1 = rpn_top();

   x0 -= x1;
   y0 -= y1;
   val = sqrt(x0*x0 + y0*y0);
   rpn_new(val);

   last_x = last;
   last_was_clx = 0;
}


void rpn_gcd()
{
double last;
double x0,y0;
double x1,y1;
double val;
double az;

   // great circle distance between two locations (lat,lon = R,Z and Y,X)
   // also calculate azimuth angle

   last = rpn[0];

   x0 = rpn_top() / deg_mode;
   y0 = rpn_top() / deg_mode;
   x1 = rpn_top() / deg_mode;
   y1 = rpn_top() / deg_mode;

   val = greatcircle(y0,x0, y1,x1) * 1000.0;

   az = az_angle(y1,x1, y0,x0);
   if(deg_mode == 1.0) az = az * PI / 180.0;
   rpn_push(az);
   rpn_push(val);

   last_x = last;
   last_was_clx = 0;
}


void rpn_bearing()
{
double last;
double x0,y0;
double x1,y1;
double az;

   // calculate azimuth angle between two points

   last = rpn[0];

   x0 = rpn_top() / deg_mode;
   y0 = rpn_top() / deg_mode;
   x1 = rpn_top() / deg_mode;
   y1 = rpn_top() / deg_mode;

   az = az_angle(y1,x1, y0,x0);
   if(deg_mode == 1.0) az = az * PI / 180.0;
   rpn_push(az);

   last_x = last;
   last_was_clx = 0;
}


void rpn_secs()
{
double val;
double last;

   // convert hours minutes sseconds in top 3 stack entries to seconds

   last = rpn[0];

   val = rpn_top();
   val += rpn_top() * 60.0;
   val += rpn_top() * 3600.0;
   rpn_push(val);

   last_x = last;
   last_was_clx = 0;
}


void rpn_dow()
{
double val;
double last;
int year,month,day;

   // convert year month day in top 3 stack entries to day-of-week (0..6)

   last = rpn[0];

   day = (int) rpn_top();
   month = (int) rpn_top();
   year = (int) rpn_top();

   val = day_of_week(year,month,day);
   rpn_push(val);

   last_x = last;
   last_was_clx = 0;
}


void rpn_dms()
{
double last;
double val;
double degs, mins, secs;
double sign;

   // break up X into degrees minutes seconds

   last = rpn[0];

   val = rpn_top();
   if(deg_mode == 1.0) {
      val = val * 180.0 / PI;
   }
   if(val < 0) {
      sign = (-1.0);
      val = 0.0 - val;
   }
   else sign = 1.0;

   degs = (double) (int) val;

   val = (val - degs) * 60.0;
   mins = (double) (int) val;

   val = (val - mins) * 60.0;
   secs = val;

   rpn_push(degs*sign);
   rpn_push(mins);
   rpn_push(secs);

   last_x = last;
   last_was_clx = 0;
}


void rpn_time()
{
double last;

   // put current displayed time on stack

   last = rpn[0];

   rpn_push((double) pri_hours);
   rpn_push((double) pri_minutes);
   rpn_push((double) pri_seconds + pri_frac);

   last_x = last;
   last_was_clx = 0;
}

void rpn_date()
{
double last;

   // put current displayed date on stack

   last = rpn[0];

   rpn_push((double) pri_year);
   rpn_push((double) pri_month);
   rpn_push((double) pri_day);

   last_x = last;
   last_was_clx = 0;
}


void rpn_gregorian()
{
double last;

   // convert rpn[0] julian date to gregorian date / time

   last = rpn[0];
   gregorian(0, rpn[0]);

   rpn_push((double) g_year);
   rpn_push((double) g_month);
   rpn_push((double) g_day);

   rpn_push((double) g_hours);
   rpn_push((double) g_minutes);
   rpn_push((double) g_seconds + g_frac);

   last_x = last;
   last_was_clx = 0;
}


void rpn_dec()
{
double last;
double val;
double degs, mins, secs;
double sign;

   // convert X,Y,Z degrees minutes seconds to decimal

   last = rpn[0];

   secs = rpn_top();
   mins = rpn_top();
   degs = rpn_top();
   if(degs < 0.0) {
      sign = (-1.0);
      degs = 0.0 - degs;
   }
   else sign = 1.0;

   secs = fabs(secs) * sign;
   mins = fabs(mins) * sign;
   degs *= sign;

   val = degs + (mins/60.0) + (secs/3600.0);

   if(deg_mode == 1.0) {
      val = val * PI / 180.0;
   }

   rpn_push(val);

   last_x = last;
   last_was_clx = 0;
}


void rpn_shr()
{
int count;
s32 val;

   // shift right

   if(isdigit(msg_field[2])) count = atoi(&msg_field[2]);
   else if(msg_field[2] == '-') count = atoi(&msg_field[2]);
   else count = 1;

   val = (int) rpn[0];
   if(count > 0) {
      val >>= count;
   }
   else if(count < 0) {
      count = 0-count;
      val <<= count;
   }

   last_x = rpn[0];
   rpn[0] = (double) val;
   last_was_clx = 0;
}

void rpn_shl()
{
int count;
s32 val;

   // shift left

   if(isdigit(msg_field[2])) count = atoi(&msg_field[2]);
   else if(msg_field[2] == '-') count = atoi(&msg_field[2]);
   else count = 1;

   val = (int) rpn[0];
   if(count > 0) {
      val <<= count;
   }
   else if(count < 0) {
      count = 0-count;
      val >>= count;
   }

   last_x = rpn[0];
   rpn[0] = (double) val;
   last_was_clx = 0;
}


void rpn_size()
{
int count;

   // set stack size

   count = 6;
   if(isdigit(msg_field[4])) count = atoi(&msg_field[4]);
   if((count < 4) || (count > MAX_RPN_SIZE)) {
      sprintf(out, "Stack size must be 4..%d", MAX_RPN_SIZE);
      rpn_error(out);
      return;
   }
   RPN_SIZE = count;
}


void rpn_digits(int sign)
{
int count;
int digits;

   // set output format digits

   count = IABS(rpn_format);
   if(isdigit(msg_field[3])) count = atoi(&msg_field[3]);
   else if(msg_field[3] == '-') count = atoi(&msg_field[3]);
   else if(msg_field[3] == '+') count = atoi(&msg_field[3]);
   count *= sign;

   if(hex_mode == 2) {
      digits = ISIZE;
      if(IABS(count) > ISIZE) {  // binary
         sprintf(out, "Decimal place count must be 0..%d", digits);
         rpn_error(out);
         return;
      }
   }
   else if(hex_mode == 8) {  // octal
      digits = (ISIZE+2) / 3;
      if(IABS(count) > digits) {
         sprintf(out, "Decimal place count must be 0..%d", digits);
         rpn_error(out);
         return;
      }
   }
   else if(hex_mode == 16) {  // hex
      digits = (ISIZE+3) / 4;
      if(IABS(count) > digits) {
         sprintf(out, "Decimal place count must be 0..%d", digits);
         rpn_error(out);
         return;
      }
   }
   else {  // decimal
      if(IABS(count) > 16) {
         sprintf(out, "Decimal place count must be 0..16");
         rpn_error(out);
         return;
      }
   }
   rpn_format = count;
}

int find_rpn_def(char *s)
{
int i;

   // see if user defined command has been defined

   if(s == 0) {
      return (-2);
   }

   for(i=0; i<num_rpn_defs; i++) {
      if(!strncmp(s, rpn_defs[i], strlen(s))) return i;
   }

   return (-1);
}


int rpn_define(int skip_first)
{
int i, j;
int def_col;
char *s;
int old_mode;

   // create a user define command

   if(num_rpn_defs >= MAX_RPN_DEFS) {
      edit_error("Too many DEFINE commands seen!");
      return 1;
   }
   old_mode = rpn_mode;
   rpn_mode = 1;   // make sure get_msg_field() works

   def_col = msg_col;
   rpn_break = 1;
   if(skip_first) {
      get_msg_field();
      def_col = msg_col;
   }

   if(get_msg_field()) {
      if(!isalpha(msg_field[0])) {
         edit_error("DEFINE name must start with a letter!");
         rpn_mode = old_mode;
         return 2;
      }
      if(!strcmp(msg_field, "define")) {
         edit_error("Cannot re-define the DEFINE command!");
         rpn_mode = old_mode;
         return 22;
      }

      i = find_rpn_def(msg_field);
      if(i >= 0) {  // redefine
         if(rpn_defs[i]) {      // delete old define
            free(rpn_defs[i]);
            rpn_defs[i] = 0;
         }
         if(num_rpn_defs) {
            for(j=i; j<num_rpn_defs; j++) {
               rpn_defs[j] = rpn_defs[j+1];
            }
            --num_rpn_defs;
         }
         if(nmea_msg[msg_col]) {
            goto do_define;
         }
      }
      else if(i == (-1)) {  // new define
         do_define:
         if(nmea_msg[msg_col]) {
            s = (char *) (void *) calloc(strlen(&nmea_msg[def_col])+1, sizeof(char));
            if(s) {
               strcpy(s, &nmea_msg[def_col]);
               if(s[0] == 0) strcat(s, " ");
               rpn_defs[num_rpn_defs] = s;
               ++num_rpn_defs;
            }
            else {
               edit_error("Could not allocate DEFINE memory!");
               rpn_mode = old_mode;
               return 4;
            }
         }
         else {
            edit_error("No operations specified for DEFINE!");
            rpn_mode = old_mode;
            return 7;
         }
      }
      else {
         rpn_mode = old_mode;
         return 5;  // should never happen
      }
   }
   else {
      edit_error("DEFINE name missing!");
      rpn_mode = old_mode;
      return 6;
   }

   rpn_mode = old_mode;
   return 0;
}


int read_rpn_file(char *rpn_name, int erase)
{
FILE *file;
int i;
char *s;
int old_mode;

   // read in user defined calculator functions from a file

   i = 0;
   if(rpn_name == 0) return (-1);
   s = strchr(rpn_name, '.');
   if(s == 0) {  // add .rpn extension if none given
//    strcat(rpn_name, ".rpn");
   }
   file = topen(rpn_name, "r");

   if(file) {
      old_mode = rpn_mode;
      rpn_err_flag = 0;
      while(fgets(out, sizeof out, file) != NULL) {
         msg_col = 0;
         msg_field[0] = 0;

         s = strchr(out, 0x0D);  // trim trailing CR and LF
         if(s) *s = 0;
         s = strchr(out, 0x0A);
         if(s) *s = 0;

         if(out[0] == 0) continue;
         if(out[0] == ' ') continue;
         if(out[0] == '\t') continue;

         strlwr(out);
         strcpy(edit_buffer, out);
         i = rpn_calc(); // execute the line
         if(i) {  // error in RPN file
            i = (-2);
            break;
         }
      }

      fclose(file);
      file = 0;

      rpn_mode = old_mode;
      if(erase) {  // erase calculator results from the plot area
         debug_text[0] = 0;
         debug_text2[0] = 0;
         debug_text3[0] = 0;
         debug_text4[0] = 0;
         debug_text5[0] = 0;
         debug_text6[0] = 0;
         debug_text7[0] = 0;
      }
   }
   else i = (-1);  // bad file name

   edit_buffer[0] = 0;
   nmea_msg[0] = 0;
   msg_col = 0;

   return i;
}

void rpn_edit()
{
int i;

   // edit a user defined command
   if(get_msg_field()) {
      i = find_rpn_def(msg_field);
      if(i >= 0) {
         if(zoom_screen) remove_zoom();
         rpn_mode = 0;
         show_version_header();
         add_kbd(0x0D);
         add_kbd('!'); // edit DEFINE
         add_kbd('`');
         strcpy(edit_buffer, rpn_defs[i]);
      }
      else {
         sprintf(out, "DEFINE %s not found!", msg_field);
         edit_error(out);
      }
   }
   else {
      edit_error("DEFINE name missing!");
   }
}

int rpn_nest()
{
   // save where we are in the current line

   if(rpn_exec_level < RPN_EXEC_DEPTH) {
      rpn_exec_stack[rpn_exec_level].col = msg_col;
      rpn_exec_stack[rpn_exec_level].skip = skip_next_rpn;
      strcpy(rpn_exec_stack[rpn_exec_level].msg, nmea_msg);
      skip_next_rpn = 0;
      ++rpn_exec_level;
      return 0;
   }

   return 1;
}

int rpn_unnest()
{
   // exit a user defined command string back to the previous nesting level

   if(rpn_exec_level) {
      --rpn_exec_level;
      msg_col = rpn_exec_stack[rpn_exec_level].col;
      skip_next_rpn = rpn_exec_stack[rpn_exec_level].skip;
      strcpy(nmea_msg, rpn_exec_stack[rpn_exec_level].msg);
      return 1;
   }

   return 0;
}

void rpn_save(int save_mem)
{
FILE *file;
int i;

   // save the current DEFINEs (and optionally memory) to a file

   get_msg_field();
   if(msg_field[0]) {
      file = topen(msg_field, "w");
      if(file) {
         for(i=0; i<num_rpn_defs; i++) {
            if(rpn_defs[i]) fprintf(file, "define %s\n", rpn_defs[i]);
         }

         if(save_mem) {  // also save the rpn memory and stack
            for(i=0; i<RPN_MEM; i++) { // save memory regs
               fprintf(file, "%.16g sto%d\n", rpn_mem[i], i);
            }
            for(i=RPN_SIZE-1; i>=0; i--) {  // save stack values
               fprintf(file, "%.16g\n", rpn[i]);
            }
         }
         fclose(file);
      }
      else {
         sprintf(out, "Could not write DEFINEs file: %s", msg_field);
         edit_error(out);
      }
   }
   else {
      edit_error("No file name given.");
   }
}

void rpn_run()
{
int i;

   // read and execute the commands in a RPN file

   get_msg_field();
   if(msg_field[0]) {
      i = read_rpn_file(msg_field, 0);
      if(i == (-1)) {
         sprintf(out, "Could not read RPN file: %s", msg_field);
         edit_error(out);
      }
      else if(i) {
         edit_error("Error in RPN file.");
      }
   }
   else {
      edit_error("No file name given.");
   }
}


char *rpn_fmt(int i)
{
int dp;
char *s;
char sign;
int exp;
char ee[32+1];
double val;
double thresh;
char *c;
int stack;
char buf[MAX_TEXT_COLS+1];
unsigned j;
int last_nib;
ITYPE ival;
ITYPE imask;
int inib;
int shift_count;

   // format stack entry i for screen display

   if(i < 0) i = 0-i;
   i = i % RPN_SIZE;
   stack = i;
   val = rpn[i];


   dp = rpn_format;
   if((val > BIG_NUM) || (val < (-BIG_NUM))) {  // to big to display as decimals
      if(dp > 0) dp = (0-dp);
   }

   if(1 && val && (dp > 0)) {  // display is in fixed point notation
      thresh = pow(0.1, dp);     // values below this would show as 0.0
      if(fabs(val) <= thresh) {  // ... so force exponent display
         dp = 0-dp;
      }
   }

   if     (i == 0) c = " X";
   else if(i == 1) c = " Y";
   else if(i == 2) c = " Z";
   else if(i == 3) c = " R";
   else if(i == 4) c = " S";
   else if(i == 5) c = " T";
   else if(i == 6) c = " U";
   else if(i == 7) c = " V";
   else if(i == 8) c = " W";
   else            c = "  ";

   // !!!! we need to use ITYPE here
   if(hex_mode == 16) {  // hexadecimal
      shift_count = 4;
      imask = 0x0F;
      ival = (ITYPE) val;
      i = 0;
      out[i++] = '0';
      out[i++] = 'x';
      out[i] = 0;
      goto fmt_ival;
   }
   else if(hex_mode == 8) {  // octal
      shift_count = 3;
      imask = 0x07;
      ival = (ITYPE) val;
      i = 0;
      out[i++] = '0';
      out[i++] = 'o';
      out[i] = 0;
      goto fmt_ival;
   }
   else if(hex_mode == 2) {  // binary
      shift_count = 1;
      imask = 0x01;
      ival = (ITYPE) val;
      i = 0;
      out[i++] = '0';
      out[i++] = 'b';
      out[i] = 0;

      fmt_ival:
      last_nib = 0;
      buf[0] = 0;
      for(j=0; j<ISIZE; j++) {  // format the value (in reverse order)
         inib = (int) (ival & imask);
         ival >>= shift_count;
         if(inib) {
            if(inib > 9) buf[j] = 'A'+(inib-10);
            else         buf[j] = '0'+inib;
            last_nib = j;
         }
         else buf[j] = '0';
         buf[j+1] = 0;
      }

      if(last_nib < dp) last_nib = dp;
      else ++last_nib;
      for(j=0; j<(unsigned)last_nib; j++) {  // reverse the string, maybe add commas
         out[i++] = buf[last_nib-j-1];
         if(comma_fmt && (last_nib-j-1)) {
            if(((last_nib-j-1) % 4) == 0) out[i++] = comma_fmt;
         }
         out[i] = 0;
      }

      strcat(out, c);
      return &out[0];
   }

   if(val >= 0.0) {
      s = "   ";  // aligns columns if negative value
      sign = ' ';
   }
   else {
      s = "  ";
      sign = '-';
      val = 0.0 - val;
   }
   strcpy(ee,c);


   if(dp < 0) {  // scentific / engineering format
      exp = 0;
      i = 0;
      if(val < 0.0) {
         while(val < 0.0) {
            val *= 10.0;
            --exp;
            if(++i >= 400) break;
         }
      }
      else if(val == 0.0) {
      }
      else if(1 && (val < 1.0)) {
         while(val < 1.0) {
            val *= 10.0;
            --exp;
            if(++i >= 400) break;
         }
      }
      else {
         while(val >= 10.0) {
            val /= 10.0;
            ++exp;
            if(++i >= 400) break;
         }
      }

      if(eng_mode) {  // make exponent a multiple of 3
         if(exp >= 0) {
            i = exp % 3;
            if(i == 1) { exp-=1; val*=10.0; }
            else if(i == 2) { exp-=2; val*=100.0; }
         }
         else {
            i = (0-exp) % 3;
            if(i == 1) { exp-=2; val*=100.0; }
            else if(i == 2) { exp-=1; val*=10.0; }
         }
      }

      if(exp < 0) sprintf(ee, "e-%03d%s", 0-exp, c);  // format exponent
      else        sprintf(ee, "e+%03d%s", exp, c);

      dp = 0-dp;
   }

   // format result string
   if     (dp == 0)  sprintf(out, "  %c%.0f%s",   sign,val,ee);
   else if(dp == 1)  sprintf(out, "  %c%.1f%s",   sign,val,ee);
   else if(dp == 2)  sprintf(out, "  %c%.2f%s",   sign,val,ee);
   else if(dp == 3)  sprintf(out, "  %c%.3f%s",   sign,val,ee);
   else if(dp == 4)  sprintf(out, "  %c%.4f%s",   sign,val,ee);
   else if(dp == 5)  sprintf(out, "  %c%.5f%s",   sign,val,ee);
   else if(dp == 6)  sprintf(out, "  %c%.6f%s",   sign,val,ee);
   else if(dp == 7)  sprintf(out, "  %c%.7f%s",   sign,val,ee);
   else if(dp == 8)  sprintf(out, "  %c%.8f%s",   sign,val,ee);
   else if(dp == 9)  sprintf(out, "  %c%.9f%s",   sign,val,ee);
   else if(dp == 10) sprintf(out, "  %c%.10f%s",  sign,val,ee);
   else if(dp == 11) sprintf(out, "  %c%.11f%s",  sign,val,ee);
   else if(dp == 12) sprintf(out, "  %c%.12f%s",  sign,val,ee);
   else if(dp == 13) sprintf(out, "  %c%.13f%s",  sign,val,ee);
   else if(dp == 14) sprintf(out, "  %c%.14f%s",  sign,val,ee);
   else if(dp == 15) sprintf(out, "  %c%.15f%s",  sign,val,ee);
   else if(dp == 16) sprintf(out, "  %c%.16f%s",  sign,val,ee);
   else              sprintf(out, "  %c%.8f%s",   sign,val,ee);

   s = strchr(out, '.');
   if(s && (comma_fmt == '.')) *s = ',';  // euro format

   if(comma_fmt && val) {  // optionally add commas to integer part
      buf[0] = 0;
      if(s) dp = s - &out[2] - 1;
      else  dp = strlen(out) - 2;

      if(dp > 3) {  // more than 3 digits
         j = dp % 3;
         for(i=0; i<(3+(int)j); i++) { // copy spaces, sign, and digits before the comma
            buf[i] = out[i];
            buf[i+1] = 0;
         }
         dp -= j;

         j = strlen(buf);
         if(i > 3) {
            buf[j++] = comma_fmt;  // add comma if more than 3 digits
            buf[j] = 0;
         }

         while(dp > 3) {  // copy next group of 3 digits and add comma
            dp -= 3;
            buf[j++] = out[i++];
            buf[j++] = out[i++];
            buf[j++] = out[i++];
            if(rpn_format && (dp > 0)) buf[j++] = comma_fmt;
            buf[j] = 0;
         }

         buf[j++] = out[i++];  // copy last group of 3 digits
         buf[j++] = out[i++];
         buf[j++] = out[i++];
         buf[j] = 0;

         while(out[i]) {   // copy fractional part
            buf[j++] = out[i++];
            buf[j] = 0;
         }
         strcpy(out, buf);
      }
   }

   return &out[0];
}

void rpn_space(char *s, unsigned max_len, char *fmt)
{
unsigned j;
int i;

   // space over the formatted stack value so columns align

   if(s == 0) return;
   if(fmt == 0) return;

   j = strlen(fmt);
   i = 0;
   s[i] = 0;
   while(j < max_len) {
      s[i++] = ' ';
      s[i] = 0;
      ++j;
      if(i > 60) break;
   }
   strcat(s, fmt);
}

void rpn_show()
{
unsigned max_len;
int i;
int x,y;
int scale;
int row, col;
char buf[SLEN+1];
int old_vc_scale;
int old_vc_fonts;
int oldh, oldw;

   // show current stack values
   max_len = 0;
   for(i=0; i<RPN_SIZE; i++) {  // find longest formatted string
      if(strlen(rpn_fmt(i)) > max_len) max_len = strlen(out);
   }

   // format and show the rpn stack (pretty printed)
   if(zoom_screen == '`') {
      fill_rectangle(0,0, SCREEN_WIDTH-1,PLOT_ROW-1, BLACK);
      if(show_rpn_help == 2) {
         show_rpn_defines(0, 1);
         return;
      }
      else if(show_rpn_help) {
         rpn_help();
         return;
      }

      x = (SCREEN_WIDTH*100/8) / (max_len+8);
      y = (((EDIT_ROW*TEXT_HEIGHT)*100)/16) / RPN_SIZE;
      scale = x;
      if(y < scale) scale = y;
      if(scale > 500) scale = 500;

      old_vc_fonts = use_vc_fonts;
      old_vc_scale = vc_font_scale;
      oldh = TEXT_HEIGHT;
      oldw = TEXT_WIDTH;
      use_vc_fonts = 1;
      vidstr(0,0, WHITE, " ");

      if(hex_mode == 16)       sprintf(out, "HEX%d", IABS(rpn_format));
      else if(hex_mode == 8)   sprintf(out, "OCT%d", IABS(rpn_format));
      else if(hex_mode == 2)   sprintf(out, "BIN%d", IABS(rpn_format));
      else if(eng_mode)        sprintf(out, "ENG%d", IABS(rpn_format));
      else if(rpn_format < 0)  sprintf(out, "SCI%d", IABS(rpn_format));
      else                     sprintf(out, "FIX%d", IABS(rpn_format));
      if(deg_mode == 1.0)      strcat(out,  "  RADIANS");
      else                     strcat(out,  "  DEGREES");
      vidstr(-1,0, WHITE, out);

      vc_font_scale = scale;

      col = 1;
      row = 0;
      vidstr(row,col, WHITE, " ");  // inits font scale factor

      for(i=RPN_SIZE-1; i>=0; i--) {
         rpn_space(buf, max_len, rpn_fmt(i));
         vidstr(row++,col, WHITE, buf);
      }

      vc_font_scale = old_vc_scale;
      use_vc_fonts = old_vc_fonts;
      TEXT_WIDTH = oldw;
      TEXT_HEIGHT = oldh;
   }
// else {
      rpn_space(debug_text,  max_len, rpn_fmt(0));
      rpn_space(debug_text2, max_len, rpn_fmt(1));
      rpn_space(debug_text3, max_len, rpn_fmt(2));
      rpn_space(debug_text4, max_len, rpn_fmt(3));
      if(RPN_SIZE > 4) rpn_space(debug_text5, max_len, rpn_fmt(4));
      if(RPN_SIZE > 5) rpn_space(debug_text6, max_len, rpn_fmt(5));
// }
}


int valid_hex(char *s)
{
unsigned j;
char c;

   // returns true if s is a valid hex number

   if(s == 0) return 0;
   if(strlen(s) < 1) return 0;

   for(j=0; j<strlen(s); j++) {
      c = s[j];
      if((c >= '0') && (c <= '9')) continue;
      else if((c >= 'a') && (c <= 'f')) continue;
      return 0;
   }
   return 1;
}

int valid_octal(char *s)
{
unsigned j;
char c;

   // returns true if s is a valid octal number

   if(s == 0) return 0;
   if(strlen(s) < 1) return 0;

   for(j=0; j<strlen(s); j++) {
      c = s[j];
      if((c >= '0') && (c <= '7')) continue;
      return 0;
   }
   return 1;
}

int valid_binary(char *s)
{
unsigned j;
char c;

   // returns true if s is a valid binary number

   if(s == 0) return 0;
   if(strlen(s) < 1) return 0;

   for(j=0; j<strlen(s); j++) {
      c = s[j];
      if((c == '0') || (c == '1')) continue;
      return 0;
   }
   return 1;
}

int valid_num(char *s)
{
unsigned j;
char c;
int saw_sign;
int saw_exp;
int saw_e;
int saw_dp;
int saw_digit;

   // returns true if s is a valid decimal number

   if(s == 0) return 0;
   saw_sign = 0;
   saw_dp = 0;
   saw_exp = 0;
   saw_e = 0;
   saw_digit = 0;

   for(j=0; j<strlen(s); j++) {
      c = s[j];
      if(c == '.') {
         if(saw_dp) return 0;
         if(saw_e) return 0;
         saw_dp = 1;
      }
      else if(c == 'e') {
         if(saw_e) return 0;
         saw_e = 1;
         saw_sign = 0;
         saw_digit = 0;
      }
      else if((c == '+') || (c == '-')) {
         if(saw_sign) return 0;
         if(saw_exp) return 0;
         if(saw_digit) return 0;
         saw_sign = 1;
      }
      else if((c >= '0') && (c <= '9')) {
         if(saw_e) saw_exp = c;
         saw_digit = 1;
         continue;
      }
      else return 0;
   }

   if(saw_e && (saw_exp == 0)) return 0;
   return 1;
}

double get_rpn_num()
{
double val;
unsigned i, j;
int c;

   // decode ascii value (can be integer hex or binary string) and
   // push value onto stack

   if((msg_field[0] == '0') && (msg_field[1] == 'x')) {  // hex integer
      strupr(msg_field);
      i = strlen(msg_field);
      val = 0.0;
      for(j=2; j<i; j++) {
         c = msg_field[j];
         if((c >= '0') && (c <= '9')) c = (c - '0');
         else if((c >= 'A') && (c <= 'F')) c = (c - 'A' + 10);
         val = (val * 16.0) + (double) c;
      }
   }
   else if((msg_field[0] == '0') && (msg_field[1] == 'o')) {  // octal integer
      i = strlen(msg_field);
      val = 0.0;
      for(j=2; j<i; j++) {
         c = msg_field[j] - '0';
         val = (val * 8.0) + (double) c;
      }
   }
   else if((msg_field[0] == '0') && (msg_field[1] == 'b')) {  // binary integer
      i = strlen(msg_field);
      val = 0.0;
      for(j=2; j<i; j++) {
         c = msg_field[j] - '0';
         val = (val * 2.0) + (double) c;
      }
   }
   else val = atof(msg_field);

   last_rpn0 = rpn[0];
   rpn_push(val);
   last_was_clx = 0;
   return rpn[0];
}

void rpn_comma()
{
#define COMMA_CHAR ','   // default grouping character

   // set the character to group integer part of displayed values

   if(msg_field[5])   comma_fmt = msg_field[5];  // user define char
   else if(comma_fmt) comma_fmt = 0;             // toggle comma mode
   else               comma_fmt = COMMA_CHAR;
}

int rpn_const()
{
int i;
double val;

   // perform a calculator command

   // We break the command parser into rpn_const() and rpn_math() in order
   // to get around a compiler if() chain limit.

   i = find_rpn_def(msg_field);
   if(i >= 0) {  // user defined command
      if(rpn_exec_level >= RPN_EXEC_DEPTH) {
         edit_error("DEFINE expansion nested too deep!");
         rpn_break = 1;
         rpn_exec_level = 0;
         return 0;
      }

      rpn_nest();  // save where we are in the current line

      // switch to decoding the DEFINE string
      strcpy(nmea_msg, rpn_defs[i]);
      msg_col = strlen(msg_field)+1;
      return 1;
   }

   // constants
   val = 0.0;
   if     (!strcmp(msg_field,  "lastx"))      { rpn_push(last_x); }
   else if(!strcmp(msg_field,  "last"))       { rpn_push(last_x); }
   else if(!strcmp(msg_field,  "c"))          { rpn_push(LIGHTSPEED*1000.0); }
   else if(!strcmp(msg_field,  "e"))          { rpn_push(E_VALUE); }
   else if(!strcmp(msg_field,  "h"))          { rpn_push(KH); }
   else if(!strcmp(msg_field,  "k"))          { rpn_push(KB); }
   else if(!strcmp(msg_field,  "pi"))         { rpn_push(PI); }
   else if(!strcmp(msg_field,  "\\"))         { rpn_push(PI); }
   else if(!strcmp(msg_field,  "rn"))         { rpn_rn(); }
   else if(!strcmp(msg_field,  "spd"))        { rpn_push(24.0*60.0*60.0); }
   else if(!strcmp(msg_field,  "spw"))        { rpn_push(24.0*60.0*60.0*7.0); }
   else if(!strcmp(msg_field,  "t0"))         { rpn_push(ABS_ZERO); }

   else if(!strcmp(msg_field,  "lat"))        { rpn_push(lat*deg_mode); }
   else if(!strcmp(msg_field,  "lon"))        { rpn_push(lon*deg_mode); }
   else if(!strcmp(msg_field,  "alt"))        { rpn_push(alt); }
   else if(!strcmp(msg_field,  "lla"))        { rpn_push(lat*deg_mode); rpn_push(lon*deg_mode); rpn_push(alt); }
   else if(!strcmp(msg_field,  "dac"))        { rpn_push(dac_voltage); }
   else if(!strcmp(msg_field,  "pps"))        { rpn_push(pps_offset); }
   else if(!strcmp(msg_field,  "osc"))        { rpn_push(osc_offset); }
   else if(!strcmp(msg_field,  "temp"))       { rpn_push(temperature); }

   else if(!strcmp(msg_field,  "amu"))        { rpn_push(amu_mask); }
   else if(!strcmp(msg_field,  "cable"))      { rpn_push(cable_delay*1.0E9); }
   else if(!strcmp(msg_field,  "elmask"))     { rpn_push(el_mask*PI/180.0*deg_mode); }
   else if(!strcmp(msg_field,  "ffom"))       { rpn_push(ffom); }
   else if(!strcmp(msg_field,  "leap"))       { rpn_push(utc_offset); }
   else if(!strcmp(msg_field,  "tfom"))       { rpn_push(tfom); }

   else if(!strcmp(msg_field,  "dop"))        { rpn_push(average_dop()); }
   else if(!strcmp(msg_field,  "edop"))       { rpn_push(edop); }
   else if(!strcmp(msg_field,  "gdop"))       { rpn_push(gdop); }
   else if(!strcmp(msg_field,  "hdop"))       { rpn_push(hdop); }
   else if(!strcmp(msg_field,  "pdop"))       { rpn_push(pdop); }
   else if(!strcmp(msg_field,  "tdop"))       { rpn_push(tdop); }
   else if(!strcmp(msg_field,  "vdop"))       { rpn_push(vdop); }
   else if(!strcmp(msg_field,  "xdop"))       { rpn_push(xdop); }
   else if(!strcmp(msg_field,  "ydop"))       { rpn_push(ydop); }

   else if(!strcmp(msg_field,  "damp"))       { rpn_push(damping_factor); }
   else if(!strcmp(msg_field,  "gain"))       { rpn_push(osc_gain); }
   else if(!strcmp(msg_field,  "initv"))      { rpn_push(initial_voltage); }
   else if(!strcmp(msg_field,  "tc"))         { rpn_push(time_constant); }

   else if(!strncmp(msg_field, "prn", 3))     { rpn_prn(0); }
   else if(!strncmp(msg_field, "az", 2))      { rpn_az(0); }
   else if(!strncmp(msg_field, "el", 2))      { rpn_el(0); }
   else if(!strncmp(msg_field, "sig", 3))     { rpn_sig(0); }
   else if(!strncmp(msg_field, "doppler", 7)) { rpn_doppler(0); }
   else if(!strncmp(msg_field, "range", 5))   { rpn_range(0); }
   else if(!strncmp(msg_field, "phase", 5))   { rpn_phase(0); }

   // time related constants
   else if(!strcmp(msg_field,  "time"))       { rpn_time(); }
   else if(!strcmp(msg_field,  "date"))       { rpn_date(); }
   else if(!strcmp(msg_field,  "local"))      { rpn_push(jd_local); }
   else if(!strcmp(msg_field,  "utc"))        { rpn_push(jd_utc); }
   else if(!strcmp(msg_field,  "gps"))        { rpn_push(jd_gps); }
   else if(!strcmp(msg_field,  "mjd"))        { rpn_push(jd_utc-JD_MJD); }
   else if(!strcmp(msg_field,  "tzjd"))       { rpn_push(time_zone()); }
   else if(!strcmp(msg_field,  "tz"))         { rpn_push(time_zone()*24.0); }
   else if(!strcmp(msg_field,  "tow"))        { rpn_push((double) tow); }
   else if(!strcmp(msg_field,  "week"))       { rpn_push((double) gps_week); }
   else if(!strcmp(msg_field,  "epoch"))      { rpn_push(GPS_EPOCH); }
   else if(!strcmp(msg_field,  "gtime"))      { rpn_push(jd_utc - GPS_EPOCH); }
   else if(!strcmp(msg_field,  "utime"))      { rpn_push(jd_utc - LINUX_EPOCH); }
   else if(!strcmp(msg_field,  "greg"))       { rpn_gregorian(); }
   else if(!strcmp(msg_field,  "gregorian"))  { rpn_gregorian(); }
   else if(!strcmp(msg_field,  "dow"))        { rpn_dow(); }
   else if(!strcmp(msg_field,  "ti"))         { rpn_push(jd_elapsed?((jd_utc-jd_elapsed)*24.0*60.0*60.0+0.00001):0.0); }
   else if(!strcmp(msg_field,  "rise"))       { rpn_push(sunrise_time + time_zone()); }
   else if(!strcmp(msg_field,  "noon"))       { rpn_push(solar_noon + time_zone()); }
   else if(!strcmp(msg_field,  "set"))        { rpn_push(sunset_time + time_zone()); }

   // conditionals
   else if(!strcmp(msg_field, "x=y"))       { if(rpn[0] == rpn[1]) skip_next_rpn = (2); else skip_next_rpn = 1; }
   else if(!strcmp(msg_field, "x<y"))       { if(rpn[0] < rpn[1])  skip_next_rpn = (2); else skip_next_rpn = 1; }
   else if(!strcmp(msg_field, "x>y"))       { if(rpn[0] > rpn[1])  skip_next_rpn = (2); else skip_next_rpn = 1; }
   else if(!strcmp(msg_field, "x<>y"))      { if(rpn[0] != rpn[1]) skip_next_rpn = (2); else skip_next_rpn = 1; }
   else if(!strcmp(msg_field, "x>=y"))      { if(rpn[0] >= rpn[1]) skip_next_rpn = (2); else skip_next_rpn = 1; }
   else if(!strcmp(msg_field, "x<=y"))      { if(rpn[0] <= rpn[1]) skip_next_rpn = (2); else skip_next_rpn = 1; }

   else if(!strncmp(msg_field, "x=", 2))    { val=atof(&msg_field[2]); if(rpn[0] == val) skip_next_rpn = (2); else skip_next_rpn = 1; }
   else if(!strncmp(msg_field, "x<", 2))    { val=atof(&msg_field[2]); if(rpn[0] < val)  skip_next_rpn = (2); else skip_next_rpn = 1; }
   else if(!strncmp(msg_field, "x>", 2))    { val=atof(&msg_field[2]); if(rpn[0] > val)  skip_next_rpn = (2); else skip_next_rpn = 1; }
   else if(!strncmp(msg_field, "x<>", 3))   { val=atof(&msg_field[3]); if(rpn[0] != val) skip_next_rpn = (2); else skip_next_rpn = 1; }
   else if(!strncmp(msg_field, "x>=", 3))   { val=atof(&msg_field[3]); if(rpn[0] >= val) skip_next_rpn = (2); else skip_next_rpn = 1; }
   else if(!strncmp(msg_field, "x<=", 3))   { val=atof(&msg_field[3]); if(rpn[0] <= val) skip_next_rpn = (2); else skip_next_rpn = 1; }

   else if(!strcmp(msg_field, "break"))     { skip_next_rpn = (-1); }
   else if(!strcmp(msg_field, "dsz"))       { rpn_inc(-1.0); }
   else if(!strcmp(msg_field, "isz"))       { rpn_inc(+1.0); }
   else if(!strcmp(msg_field, "nop"))       { }
   else if(!strcmp(msg_field, "again"))     { // loop back to start of line
      msg_col = 0;      // loop back to start of line
      update_pwm();     // if doing pwm temperature control
      serve_os_queue(); // so keypress check works
      if(KBHIT()) {
         nmea_msg[0] = 0;
         msg_col = 0;
         msg_field[0] = 0;
         rpn_error("Calculator loop interrupted!");
      }
   }

   else return 0;  // unknown command

   return 1;   // command processed
}

int rpn_math()
{
double val;
ITYPE ival;

   // basic math
   if     (!strcmp(msg_field, "+"))     { val=rpn_top()+rpn[0]; rpn_set(val); }
   else if(!strcmp(msg_field, "*"))     { val=rpn_top()*rpn[0]; rpn_set(val); }
   else if(!strcmp(msg_field, "-"))     { val=rpn[1]-rpn[0]; rpn_top(); rpn_set(val); }
   else if(!strcmp(msg_field, "/"))     { val=rpn[1]/rpn[0]; rpn_top(); rpn_set(val); }
   else if(!strcmp(msg_field, "mod"))   { val=rpn_top(); rpn_set(fmod(rpn[0],val)); }
   else if(!strcmp(msg_field, "chs"))   { val=last_x; rpn_new(0.0-rpn[0]); last_x=val; }
   else if(!strcmp(msg_field, "inv"))   { rpn_new(1.0/rpn[0]); }
   else if(!strcmp(msg_field, "abs"))   { rpn_new(fabs(rpn[0])); }
   else if(!strcmp(msg_field, "int"))   { rpn_new((double) (int) rpn[0]); }
   else if(!strcmp(msg_field, "frac"))  { rpn_new(rpn[0]-(double) (int) rpn[0]); }
   else if(!strcmp(msg_field, "ctof"))  { rpn_new((rpn[0]*1.8)+32.0); }
   else if(!strcmp(msg_field, "ftoc"))  { rpn_new((rpn[0]-32.0)*(5.0/9.0)); }
   else if(!strcmp(msg_field, "ftom"))  { rpn_new(rpn[0]/FEET_PER_METER); }
   else if(!strcmp(msg_field, "mtof"))  { rpn_new(rpn[0]*FEET_PER_METER); }
   else if(!strcmp(msg_field, "max"))   { rpn_max(); }
   else if(!strcmp(msg_field, "min"))   { rpn_min(); }

   // logical operations
   else if(!strcmp(msg_field, "&"))     { ival=(ITYPE)rpn[1] & (ITYPE) rpn[0]; rpn_top(); rpn_set((double) ival); }
   else if(!strcmp(msg_field, "|"))     { ival=(ITYPE)rpn[1] | (ITYPE) rpn[0]; rpn_top(); rpn_set((double) ival); }
   else if(!strcmp(msg_field, "^"))     { ival=(ITYPE)rpn[1] ^ (ITYPE) rpn[0]; rpn_top(); rpn_set((double) ival); }
   else if(!strcmp(msg_field, "~"))     { ival=(ITYPE)rpn[0]; rpn_top(); rpn_set((double) (~ival)); }
   else if(!strncmp(msg_field, ">>",2)) { rpn_shr(); }
   else if(!strncmp(msg_field, "<<",2)) { rpn_shl(); }

   // trig
   else if(!strcmp(msg_field, "deg"))   { deg_mode = 180.0/PI; }
   else if(!strcmp(msg_field, "rad"))   { deg_mode = 1.0; }
   else if(!strcmp(msg_field, "sin"))   { rpn_new(sin(rpn[0]/deg_mode)); }
   else if(!strcmp(msg_field, "cos"))   { rpn_new(cos(rpn[0]/deg_mode)); }
   else if(!strcmp(msg_field, "tan"))   { rpn_new(tan(rpn[0]/deg_mode)); }
   else if(!strcmp(msg_field, "asin"))  { rpn_new(asin(rpn[0])*deg_mode); }
   else if(!strcmp(msg_field, "acos"))  { rpn_new(acos(rpn[0])*deg_mode); }
   else if(!strcmp(msg_field, "atan"))  { rpn_new(atan(rpn[0])*deg_mode); }
   else if(!strcmp(msg_field, "atan2")) { val=rpn_top(); rpn_set(atan2(rpn[0],val)*deg_mode); }
   else if(!strcmp(msg_field, "rtod"))  { rpn_new(rpn[0]*180.0/PI); }
   else if(!strcmp(msg_field, "dtor"))  { rpn_new(rpn[0]*PI/180.0); }
   else if(!strcmp(msg_field, "rtop"))  { rpn_rtop(); } // rectangular to polar
   else if(!strcmp(msg_field, "ptor"))  { rpn_ptor(); } // to polar rectangular
   else if(!strcmp(msg_field, "diag"))  { val=rpn_top(); val=sqrt(val*val+rpn[0]*rpn[0]); rpn_set(val); }
   else if(!strcmp(msg_field, "dist"))  { rpn_dist(); }
   else if(!strcmp(msg_field, "dms"))   { rpn_dms(); }
   else if(!strcmp(msg_field, "secs"))  { rpn_secs(); }
   else if(!strcmp(msg_field, "dec"))   { rpn_dec(); }
   else if(!strcmp(msg_field, "gcd"))   { rpn_gcd(); }
   else if(!strcmp(msg_field, "bearing"))   { rpn_bearing(); }

   // logs and powers
   else if(!strcmp(msg_field, "ln"))    { rpn_new(log(rpn[0])); }
   else if(!strcmp(msg_field, "log"))   { rpn_new(log10(rpn[0])); }
   else if(!strcmp(msg_field, "sqrt"))  { rpn_new(sqrt(rpn[0])); }
   else if(!strcmp(msg_field, "sqr"))   { rpn_new(rpn[0]*rpn[0]); }
   else if(!strcmp(msg_field, "exp"))   { rpn_new(pow(E_VALUE, rpn[0])); }
   else if(!strcmp(msg_field, "powe"))  { rpn_new(pow(E_VALUE, rpn[0])); }
   else if(!strcmp(msg_field, "pow10")) { rpn_new(pow(10.0, rpn[0])); }
   else if(!strcmp(msg_field, "pow"))   { val=rpn_top(); rpn_set(pow(rpn[0],val)); }
   else if(!strcmp(msg_field, "**"))    { val=rpn_top(); rpn_set(pow(rpn[0],val)); }

   // stack manipulation
   else if(!strcmp(msg_field,  "ex"))       { val=rpn[0]; rpn[0]=rpn[1]; rpn[1]=val; }
   else if(!strcmp(msg_field,  "ent"))      { rpn_push(rpn[0]); }
   else if(!strcmp(msg_field,  "enter"))    { rpn_push(rpn[0]); }
   else if(!strcmp(msg_field,  "clx"))      { rpn[0]=0.0; last_was_clx=1; }
   else if(!strcmp(msg_field,  "cls"))      { rpn_clear(1); }
   else if(!strcmp(msg_field,  "clm"))      { rpn_clear(2); }
   else if(!strcmp(msg_field,  "clear"))    { rpn_clear(3); }
   else if(!strncmp(msg_field, "drop", 4))  { rpn_drop(); }
   else if(!strncmp(msg_field, "roll", 4))  { rpn_down(); }
   else if(!strncmp(msg_field, "down", 4))  { rpn_down(); }
   else if(!strncmp(msg_field, "up", 2))    { rpn_up(); }
   else if(!strncmp(msg_field, "sto", 3))   { rpn_store(); }
   else if(!strncmp(msg_field, "rcl", 3))   { rpn_recall(); }
   else if(!strncmp(msg_field, "stack", 5)) { rpn_stack(); }
   else if(!strncmp(msg_field, "swap", 4))  { rpn_swap(); }
   else if(!strncmp(msg_field, "size", 4))  { rpn_size(); }

   // display formatting
   else if(!strncmp(msg_field, "fix", 3))   { hex_mode = 0;  eng_mode = 0; rpn_digits(1); }
   else if(!strncmp(msg_field, "sci", 3))   { hex_mode = 0;  eng_mode = 0; rpn_digits(-1); }
   else if(!strncmp(msg_field, "eng", 3))   { hex_mode = 0;  eng_mode = 1; rpn_digits(-1); }
   else if(!strncmp(msg_field, "hex", 3))   { hex_mode = 16; rpn_digits(1); }    // show stack top as hex integer
   else if(!strncmp(msg_field, "oct", 3))   { hex_mode = 8;  rpn_digits(1); }    // show stack top as octal integer
   else if(!strncmp(msg_field, "bin", 3))   { hex_mode = 2;  rpn_digits(1); }    // show stack top as octal integer
   else if(!strncmp(msg_field, "comma", 5)) { rpn_comma(); }    // format integer part with commas

   // user defined operations
   else if(!strcmp(msg_field,  "define"))   { rpn_define(0); }
   else if(!strcmp(msg_field,  "edit"))     { rpn_edit(); }
   else if(!strcmp(msg_field,  "savedefs")) { rpn_save(0); }
   else if(!strcmp(msg_field,  "saveall"))  { rpn_save(1); }
   else if(!strcmp(msg_field,  "run"))      { rpn_run(); }

   else if(!strcmp(msg_field,  "exit"))     { shut_down(987); }

   else return 0;  // unknown command

   return 1;  // command processed
}


int rpn_calc()
{
   // rpn calculator
   if(last_touch_key == 999) {  // ? command sent from touch keyboard
      show_rpn_help = 1;
   }

   rpn_break = 0;
   if(edit_buffer[0] == 0) {  // empty line, exit calculator
      if(zoom_screen) remove_zoom();
      rpn_mode = 0;
      show_version_header();
      rpn_unnest();
      rpn_err_flag = 0;
      return 0;
   }

// rpn_err_flag = 0;
   rpn_mode = 1;
   show_version_header();
   strlwr(edit_buffer);
   strcpy(nmea_msg, edit_buffer);  // prepare to parse the line
   msg_col = 0;

if(strstr(edit_buffer, "z`")) goto poop;

   // process the line of calculator commands (separated by spaces)
   rpn_again:
   msg_field[0] = 0;
   while(get_msg_field()) {
//      if(!strcmp(msg_field, "?") == 0) {
      if(strcmp(msg_field, "?")) {
         if(last_touch_key == 999) { last_touch_key = 0; show_rpn_help = 0;}
      }

      if(rpn_break) {   // stop parsing line on any errors or "break" command
         rpn_break = 0;
         skip_next_rpn = 0;
         break;
      }

      if(skip_next_rpn > 1) {  // conditional was true skip second operand
         --skip_next_rpn;
      }
      else if(skip_next_rpn == 1) {  // skip next command
         skip_next_rpn = 0;
         continue;
      }
      else if(skip_next_rpn < 0) {  // skip rest of line
         skip_next_rpn = 0;
         break;
      }

      // see what we just got
      if((msg_field[0] == '.') && msg_field[1]) {  // floating point number
         if(valid_num(msg_field)) get_rpn_num();
         else {
            sprintf(out, "Invalid number: %s", msg_field);
            rpn_error(out);
            break;
         }
      }
      else if((msg_field[0] == '+') && msg_field[1]) { // signed number
         if(valid_num(msg_field)) get_rpn_num();
         else {
            sprintf(out, "Invalid number: %s", msg_field);
            rpn_error(out);
            break;
         }
      }
      else if((msg_field[0] == '-') && msg_field[1]) { // signed number
         if(valid_num(msg_field)) get_rpn_num();
         else {
            sprintf(out, "Invalid number: %s", msg_field);
            rpn_error(out);
            break;
         }
      }
      else if(isdigit(msg_field[0])) { // possible number
         if((msg_field[0] == '0') && (msg_field[1] == 'x')) {  // hex number
            if(valid_hex(&msg_field[2])) get_rpn_num();
            else {
               sprintf(out, "Invalid hex constant: %s", msg_field);
               rpn_error(out);
               break;
            }
         }
         else if((msg_field[0] == '0') && (msg_field[1] == 'o')) {  // octal number
            if(valid_octal(&msg_field[2])) get_rpn_num();
            else {
               sprintf(out, "Invalid octal constant: %s", msg_field);
               rpn_error(out);
               break;
            }
         }
         else if((msg_field[0] == '0') && (msg_field[1] == 'b')) {  // binary number
            if(valid_binary(&msg_field[2])) get_rpn_num();
            else {
               sprintf(out, "Invalid binary constant: %s", msg_field);
               rpn_error(out);
               break;
            }
         }
         else if(valid_num(msg_field)) {  // decimal number
            get_rpn_num();
         }
         else {
            sprintf(out, "Invalid number: %s", msg_field);
            rpn_error(out);
            break;
         }
      }
      else if(rpn_const()) ;   // calculator operations (constants, etc - rpn_const() must be called before rpn_math)
      else if(rpn_math()) ;    // calculator operations (math, etc)
      else if(!strcmp(msg_field, "?"))    { show_rpn_help = 1; }
      else if(!strcmp(msg_field, "help")) { show_rpn_help = 1; }
      else if(!strcmp(msg_field, "showdefs")) { show_rpn_help = 2; }
      else if(!strcmp(msg_field, "zoom")) {
         add_kbd(ESC_CHAR);
         start_calc_zoom(10);
      }
      else if(msg_field[0]) {
         sprintf(out, "Unknown operation: (%s)", msg_field);
         rpn_error(out);
         break;
      }
      else break;  // end of line
   }

poop:
   skip_next_rpn = 0;
   if(rpn_unnest()) {  // we got to the end of a user defined command, go back to where we were
      goto rpn_again;
   }

   rpn_show(); // show the rpn stack
   rpn_mode = (-1);

   return rpn_err_flag;
}

void zoom_calc()
{
   rpn_show();
}


//
//
//   COM port stuff
//
//

int set_serial_params(unsigned port, char *p, int open_port)
{
char *s;
int baud;
int data_bits;
int stop_bits;
int parity;

   // Set serial port parameters.
   // p is serial port parameter string
   // port is default port to config if no :P param seen in p string
   // if open_port is non-zero, com port is opened
   //
   // Returns port that was configed or negative number if error detected.
   //
   // If a serial port parameter is not specified it is not changed.

   if(p == 0) return (-1);  // null param string

   s = strstr(p, ":p");   // check for :P0 .. :P9 for forced port number
   if(s == 0) s = strstr(p, ":P");
   if(s == 0) s = strstr(p, ",p");
   if(s == 0) s = strstr(p, ",P");

   if(s && s[2]) {  // :Px value seen, override port arg
      if(isdigit(s[2])) port = (s[2]-'0');
      if     (toupper(s[2] == 'D')) port = DAC_PORT;       // :PD - external DAC port
      else if(toupper(s[2] == 'E')) port = ECHO_PORT;      // :PE - echo port
      else if(toupper(s[2] == 'F')) port = FAN_PORT;       // :PF - temperature control device port
      else if(toupper(s[2] == 'I')) port = TICC_PORT;      // :PI - TICC port
      else if(toupper(s[2] == 'K')) port = NMEA_PORT;      // :PK - echo in NMEA format port
      else if(toupper(s[2] == 'N')) port = THERMO_PORT;    // :PN - environmental sensor (thermometer) port
      else if(toupper(s[2] == 'R')) port = RCVR_PORT;      // :PR - receiver data port
      else if(toupper(s[2] == 'T')) port = TRACK_PORT;     // :PT - moon/sun position
   }

   if(port >= NUM_COM_PORTS) return (-2);  // port number out of range

   s = p;
   s = trim_whitespace(s);
   if(s == 0) return (-3);
   if(s[0] == 0) return (-3);  // null param string

   baud = com[port].baud_rate;
   data_bits = com[port].data_bits;
   stop_bits = com[port].stop_bits;
   parity = com[port].parity;

   if(isdigit(*s)) {
      baud = atoi(s);
//    if(baud >= 150) com[port].baud_rate = baud;
   }

   if     (strstr(s, ":8")) data_bits = 8;
   else if(strstr(s, ",8")) data_bits = 8;
   else if(strstr(s, ":7")) data_bits = 7;
   else if(strstr(s, ",7")) data_bits = 7;
   else if(strstr(s, ":6")) data_bits = 6;
   else if(strstr(s, ",6")) data_bits = 6;
   else if(strstr(s, ":5")) data_bits = 5;
   else if(strstr(s, ",5")) data_bits = 5;
// else                     data_bits = 8;

   if     (strstr(s, ":1")) stop_bits = 1;
   else if(strstr(s, ",1")) stop_bits = 1;
   else if(strstr(s, ":2")) stop_bits = 2;
   else if(strstr(s, ",2")) stop_bits = 2;
// else                     stop_bits = 1;

   if     (strstr(s, ":N")) parity = NO_PAR;
   else if(strstr(s, ":n")) parity = NO_PAR;
   else if(strstr(s, ":O")) parity = ODD_PAR;
   else if(strstr(s, ":0")) parity = ODD_PAR;
   else if(strstr(s, ":o")) parity = ODD_PAR;
   else if(strstr(s, ":E")) parity = EVEN_PAR;
   else if(strstr(s, ":e")) parity = EVEN_PAR;
   else if(strstr(s, ",N")) parity = NO_PAR;
   else if(strstr(s, ",n")) parity = NO_PAR;
   else if(strstr(s, ",O")) parity = ODD_PAR;
   else if(strstr(s, ",0")) parity = ODD_PAR;
   else if(strstr(s, ",o")) parity = ODD_PAR;
   else if(strstr(s, ",E")) parity = EVEN_PAR;
   else if(strstr(s, ",e")) parity = EVEN_PAR;
// else                     parity = NO_PAR;

   if(open_port == 2) {
      set_rcvr_baud(baud, data_bits, parity, stop_bits);
   }
   else {
      com[port].user_set_baud = 1;  //ppppp
      com[port].baud_rate = baud;
      com[port].data_bits = data_bits;
      com[port].parity = parity;
      com[port].stop_bits = stop_bits;
      if(open_port) init_com(port, 10);
   }
   return port;
}


int baud_list[] = {
   9600,
   19200,
   38400,
   57600,
   115200,
   0
} ;

void force_rcvr_baud(int baud)
{
int i;

   // attempt to force the receiver baud rate regardless of its current setting
   // !!!! this code currently only configures for 8,N,1 serial port params

   i = 0;
   while(baud_list[i]) {
      sprintf(out, "%d,8,N,1", baud_list[i]);  // set com port rate
      set_serial_params(RCVR_PORT, out, 1);

      sprintf(out, "%d,8,N,1", baud);          // attempt to change receiver baud rate
      set_serial_params(RCVR_PORT, out, 2);    // this will only be accepted if current receiver baud rate is correct
      ++i;
   }

   // at this point the receiver should be configured for the desired baud rate
   sprintf(out, "%d,8,N,1", baud);  // set com port rate
   set_serial_params(RCVR_PORT, out, 1);

   sprintf(out, "%d,8,N,1", baud); // attempt to change baud rate
   set_serial_params(RCVR_PORT, out, 2);
}


//
//  Keyboard processor
//


#define ALWAYS_TEXT_HELP 1  // if 1, display full screen help
                            // if 0, help can go in the plot area

void kbd_help()
{
char *s, *ems_info;
COORD help_row, help_col;
COORD row, col;
COORD help_row_1;

   // this routine displays help info about the keyboard commands
   // until a key is pressed
   //
   // NEW_RCVR

   request_version(); // so the "Press space for help" message gets updated quickly
   plot_version = 1;  // once asked for help,  switch help message to version
                      // display since the user should now know that help exists

   if(zoom_screen == 'K') {
      change_zoom_config(-100);
      cancel_zoom(10);      //zkzk
   }

   if(1 && osc_params) {   // if enabled, cancel the osc param display mode
      osc_params = 0;
      text_mode = 0;
      no_x_margin = no_y_margin = 0;
      erase_screen();
      redraw_screen();
//    do_kbd(' ');
      sure_exit();
      return;
   }

   osc_params = 0;
   erase_screen();
   redraw_screen();

   // !!! kludge: for undersized screens
   if(screen_type == 't') {   // text mode
      no_x_margin = no_y_margin = 1;
   }
   else if(ALWAYS_TEXT_HELP || (SCREEN_HEIGHT <= SHORT_SCREEN)) {  // small graphics screens
      text_mode = 2;  // say we are in text mode while help is displayed
      no_x_margin = no_y_margin = 1;
   }

   if(text_mode) {   // full screen help mode
      if(SCREEN_WIDTH >= NARROW_SCREEN) {  // there is room for a margin
         if(SCREEN_HEIGHT < TINY_HEIGHT) help_row = 0;
         else                            help_row = 1;
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

      vidstr(row,col,  HELP_COLOR, "Keyboard command help.  Press any key to continue...");
      ++row;
      ++row;
   }
   else {
      if(luxor) sprintf(out, "Luxor Power/LED Analyzer %sProgram.  Rev %s - %s %s%s", s, VERSION, date_string, __TIME__, ems_info);
      else      sprintf(out, "Lady Heather's Disciplined Oscillator %sProgram.  Rev %s - %s %s%s", s, VERSION, date_string, __TIME__, ems_info);
      vidstr(row,col,  HELP_COLOR, out);
      ++row;

      vidstr(row,col,  HELP_COLOR, "Keyboard command help.  Press any key to continue...");
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
      if(rcvr_type == CS_RCVR)       s = "d - set Standby mode / DAC voltage";
      else if(rcvr_type == PRS_RCVR) s = "d - set discipline mode / FC value";
      else                           s = "d - set osc Disciplining / DAC voltage";
      vidstr(row++, col,  HELP_COLOR, s);

      s = "e - save current config to EEPROM";
      vidstr(row++, col,  HELP_COLOR, s);
   }

   if(luxor)                        s = "f - change display Filter";
   else if(rcvr_type == CS_RCVR)    s = "f - change display Filter";
   else if(rcvr_type == TICC_RCVR)  s = "f - change display Filter";
   else                             s = "f - toggle Filter or signal/elev masks";
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
      if     (rcvr_type == CS_RCVR)     s = "j - sync PPS outputs";
      else if(timing_mode == TMODE_UTC) s = "j - Jam sync PPS to UTC time";
      else if(timing_mode == TMODE_GPS) s = "j - Jam sync PPS to GPS time";
      else if(timing_mode == TMODE_SU)  s = "j - Jam sync PPS to UTC(SU) time";
      else                              s = "j - Jam sync PPS to time";
      vidstr(row++, col,  HELP_COLOR, s);

      s = "k - temperature control PID";
      vidstr(row++, col,  HELP_COLOR, s);
   }

   s = "l - data Logging";
   vidstr(row++, col,  HELP_COLOR, s);

   if(luxor) {
      s = "m - change LED driver Mode";
      vidstr(row++, col,  HELP_COLOR, s);

#ifdef WINDOWS
      s = "n - use NOTEPAD to edit a file";
      vidstr(row++, col,  HELP_COLOR, s);
#else  // __linux__  __MACH__   __FreeBSD__
      s = "n - use NANO to edit a file";
      vidstr(row++, col,  HELP_COLOR, s);
#endif

      s = "o - Set misc options";
      vidstr(row++, col,  HELP_COLOR, s);

      s = "p - set Protection values";
      vidstr(row++, col,  HELP_COLOR, s);
   }
   else {
      if(1 || jd_obs) {
         s = "m - RINEX file commands";
         vidstr(row++, col,  HELP_COLOR, s);
      }
#ifdef WINDOWS
      s = "n - use NOTEPAD to edit a file";
      vidstr(row++, col,  HELP_COLOR, s);
#else  // __linux__  __MACH__   __FreeBSD__
      s = "n - use NANO to edit a file";
      vidstr(row++, col,  HELP_COLOR, s);
#endif

      s = "o - Set misc options";
      vidstr(row++, col,  HELP_COLOR, s);

      if     (rcvr_type == CS_RCVR)   s = "p - Configuration control";
      else if(rcvr_type == TICC_RCVR) s = "p - Configuration control";
      else                            s = "p - PPS and TRAIM control";
      vidstr(row++, col,  HELP_COLOR, s);
   }

   s = "r - Read in a file";
   vidstr(row++, col,  HELP_COLOR, s);


   help_row_1 = row;      // switch to next column
   row = help_row;
   col = help_col + 40;

   if(luxor)                       s = "s - enter position (lat lon alt)";
   else if(rcvr_type == CS_RCVR)   s = "s - enter position (lat lon alt)";
   else if(rcvr_type == TICC_RCVR) s = "s - enter position (lat lon alt)";
   else      s = "s - Survey/sats/signals/positioning";
   vidstr(row++, col,  HELP_COLOR, s);

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
   else      s = "& - oscillator discipling parameters";
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

   s = "` - RPN calculator";
   vidstr(row++, col,  HELP_COLOR, s);

   s = "? - display command line help";
   vidstr(row++, col,  HELP_COLOR, s);



   if(text_mode) {  // full screen help
      col = help_col;
      if(SCREEN_HEIGHT < TINY_HEIGHT) row = help_row_1;
      else                            row = help_row_1 + 1;
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

   if(text_mode && (SCREEN_HEIGHT < SHORT_SCREEN)) {
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

   if(SCREEN_HEIGHT < TINY_HEIGHT) ;
   else ++row;
   col = help_col;
   s = "LEFT CLICK in plot area - zoom and center plot at mouse pointer";
   vidstr(row++, col, HELP_COLOR, s);
   s = "RIGHT BUTTON (at center of plot area) - scroll plot left or right";
   vidstr(row++, col, HELP_COLOR, s);
   s = "LEFT CLICK lat/lon info, sat info, plot header, adev table, maps, clock, watch";
   vidstr(row++, col, HELP_COLOR, s);
   s = "           to zoom them to full screen.  Click again to restore screen";
   vidstr(row++, col, HELP_COLOR, s);

   if(SCREEN_HEIGHT < TINY_HEIGHT) ;
   else ++row;
   sprintf(out, "Config/log directory: %s", heather_path);
   vidstr(row++, col, GREEN, out);

   help_exit:
   refresh_page();
}


void reset_first_key(int why)
{
   if(first_key && (zoom_screen == 'K')) {
      cancel_zoom(11);
//(555)   change_zoom_config(-101);
      need_redraw = 5599;
   }

   first_key = 0;
}


void second_key(int c)
{
   // process second  keystroke of a two key command
   if(c == first_key) {  // the user confirmed a dangerous single char command
      if(c != '?') BEEP(302);
   }
   else {  // single char command not confirmed
      if(script_file) {
         erase_plot(FULL_ERASE_OK);
         sprintf(out, "Selection '%c' for command '%c' not recognized", c, first_key);
         edit_error(out);
      }
      if(text_mode) redraw_screen();
   }

   reset_first_key(20); // clear message and resume graphing
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
     draw_plot(REFRESH_SCREEN);
   }
}

char *tide_plots()
{
    if(enviro_mode() && (rcvr_type != TIDE_RCVR)) {
       return  "Enviro:  KP=pressure   KH=humidity  KT=temp1   KU=temp2   KA=all";
    }
    else if(plot_prn) {
       if(have_phase)      return  "Sat:     KX=az      KY=el      KZ=signal     KG=phase    KA=all";
       else if(have_range) return  "Sat:     KX=az      KY=el      KZ=signal     KG=range    KA=all";
       else                return  "Sat:     KX=az      KY=el      KZ=signal     KG=gravity  KA=all";
    }
    else {
       return  "Tides:   KX=lon     KY=lat     KZ=alt        KG=gravity  KA=all   KI=displacement plot";
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
   //
   // NEW_RCVR

   s1 = s2 = s3 = s4 = s5 = s6 = 0;
   two_char = 0;
   if(c != '&') {   // command is not the osc parameter command
      if(osc_params) {      // we are showing the osc parameters
         redraw_screen();
      }
   }
   else {
      show_satinfo();
   }

   if(first_key) {   // this is the second keystroke of a two char command
      if(first_key == 'a') {
         if(adevs_active(0)) last_was_adev = 3;
      }
      second_key(c);
      return c;
   }

   // This is the first keystroke of a command.
   // Either the user wants to do something dangerous... so ask the annoying question
   // or else prompt for the second keystroke of the two character commands.
show_last_mouse_x(BLACK);

   first_key = c;    // first_key is first char of the command we are doing
   if((first_key == 'a') && adevs_active(0)) last_was_adev = 2;
   else if(first_key != 'p') prot_menu = 0;
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
         show_osc_params(row+8,col);
      }
   }

esc_esc_esc = 0;
   if(c == ESC_CHAR) {
      if(disable_kbd) {
         s1 = "Keyboard is disabled.  Enter '!' to re-enable it";
      }
      else {
         s1 = "This will exit the program";
      }
esc_esc_esc = 1;
   }
#ifdef ADEV_STUFF
   else if(c == 'a') {
      if(adevs_active(0)) {
         s1 =          "All channels:   A)dev  H)dev  M)dev  T)dev  mtI)e";
         if(TICC_USED) {
            sprintf(s, "All xDEVS for:  P)%s  O)%s  C)chC  D)chD", "chA", "chB");
            s3 =       "xDEV bins:      S)equence     E)rror bars   R)ecalc";
         }
         else {
            sprintf(s, "All xDEVs for:  P)%s  O)%s", plot[PPS].plot_id, plot[OSC].plot_id);
            s3 =       "xDEV bins:      S)equence     E)rror bars   R)ecalc";
         }
         s2 = &s[0];
         s4 =          "Plots:          eX)clude plot";
         two_char = 1;
         last_was_adev = 1;
      }
      else if(rcvr_type == CS_RCVR) {
         s1 = "Adev info not supported by this device... press ESC";
         last_was_adev = 0;
         lwa = 10;
      }
      else {
         s1 = "Adev info not supported by this receiver... press ESC";
         last_was_adev = 0;
         lwa = 11;
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
         s2 = "G)litch removal from all plots   T)rim plot queue data";
         if(adevs_active(0)) s3 = "A)dev queue data";
         if(TICC_USED)       s4 = "mtI)e data";
      }
      else if(rcvr_type == ACRON_RCVR) {
         s1 = "C)lear all data                  P)lot queue data";
         s2 = "G)litch removal from all plots   T)rim plot queue data";
         if(adevs_active(0)) s3 = "A)dev queue data";
      }
      else if(rcvr_type == CS_RCVR) {
         s1 = "Clear: C)lear all data   P)lot queue data     device L)og";
         s2 = "       G)litch removal from all plots   T)rim plot queue data";
         if(adevs_active(0)) s3 = "       A)dev queue data";
         if(TICC_USED)       s4 = "       mtI)e data";
      }
      else if(rcvr_type == LPFRS_RCVR) {
         s1 = "Clear: C)lear all data                  P)lot queue data";
         s2 = "       G)litch removal from all plots   T)rim plot queue data";
         if(adevs_active(0)) s3 = "       A)dev queue data";
         if(TICC_USED)       s4 = "       mtI)e data";
      }
      else if(rcvr_type == PRS_RCVR) {
         s1 = "Clear:   C)lear all data                  P)lot queue data";
         s2 = "         G)litch removal from all plots   T)rim plot queue data";
         if(adevs_active(0)) s3 = "       A)dev queue data";
         if(TICC_USED)       s4 = "       mtI)e data";
      }
      else if(rcvr_type == RFTG_RCVR) {
         s1 = "Clear A)dev queue only   B)oth plot and adev queues   C)lear all data";
         s2 = "      L)LA fix data      satellite M)ap data          P)lot queue data";
         s3 = "      S)ignal data";
         s4 = "      R)eload adevs from displayed plot window data   E)eprom history";
         s5 = "      G)litch removal from all plots    T)rim plot queue data";
         if(TICC_USED)  s6 = "      mtI)e data";
      }
      else if(rcvr_type == RT17_RCVR) {
         s1 = "Clear: C)lear all data    S)ignal data          P)lot queuq data";
         s2 = "       L)LA fix data      satellite M)ap data";
         s3 = "       G)litch removal from all plots           T)rim plot queue data";
         if(adevs_active(0)) s4 = "       A)dev queue data";
         if(TICC_USED)       s5 = "       mtI)e data";
      }
      else if(rcvr_type == SA35_RCVR) {
         s1 = "Clear: C)lear all data                  P)lot queue data";
         s2 = "       G)litch removal from all plots   T)rim plot queue data";
         if(adevs_active(0)) s3 = "       A)dev queue data";
         if(TICC_USED)       s4 = "       mtI)e data";
      }
      else if(rcvr_type == SRO_RCVR) {
         s1 = "Clear: C)lear all data         P)lot queue data";
         s2 = "       G)litch removal from all plots   T)rim plot queue data";
         if(adevs_active(0)) s3 = "       A)dev queue data";
         if(TICC_USED)       s4 = "       mtI)e data";
      }
      else if(rcvr_type == THERMO_RCVR) {
         s1 = "Clear: C)lear all data                  P)lot queue";
         s2 = "       G)litch removal from all plots   T)rim plot queue data";
         if(adevs_active(0)) s3 = "       A)dev queue data";
         if(TICC_USED)       s4 = "       mtI)e data";
      }
      else if(rcvr_type == TICC_RCVR) {
         s1 = "Clear: A)dev queue only   B)oth plot and adev queues   C)lear all data";
         s2 = "       P)lot queue data   mtI)e queue";
         s3 = "       R)eload xDEV and MTIE from displayed plot window data";
         s4 = "       G)litch removal from all plots   T)rim plot queue data";
      }
      else if(rcvr_type == TSERVE_RCVR) {
         s1 = "Clear: C)lear all data                  P)lot queue data";
         s2 = "       G)litch removal from all plots   T)rim plot queue data";
         if(adevs_active(0)) s3 = "       A)dev queue data";
         if(TICC_USED)       s4 = "       mtI)e data";
      }
      else if(rcvr_type == Z12_RCVR) {
         s1 = "Clear: C)lear all data    S)ignal data          P)lot queuq data";
         s2 = "       L)LA fix data      satellite M)ap data";
         s3 = "       G)litch removal from all plots           T)rim plot queue data";
         if(adevs_active(0)) s4 = "       A)dev queue data";
         if(TICC_USED)       s5 = "       mtI)e data";
      }
      else {
         s1 = "Clear: A)dev queue only   B)oth plot and adev queues   C)lear all data";
         s2 = "       L)LA fix data      satellite M)ap data          P)lot queue data";
         s3 = "       S)ignal data";
         s4 = "       R)eload adevs from displayed plot window data";
         s5 = "       G)litch removal from all plots    T)rim plot queue data";
         if(TICC_USED) s6 = "       mtI)e data";
      }
      two_char = 1;
   }
   else if(c == 'd') {
      if(rcvr_type == CS_RCVR) {
         s1 = "E)nable standby   D)isable standby (normal operation)";
         s2 = "S)et osc EFC voltage (and enter standby mode)";
         two_char = 1;
      }
      else if(rcvr_type == PRS_RCVR) {
         s1 = "Oscillator discipline:  E)nable  D)isable    S)et FC voltage";
         two_char = 1;
      }
      else if(rcvr_type == SRO_RCVR) {
         s1 = "Tracking:  D)isable   E)nable";
         two_char = 1;
      }
      else if((rcvr_type == TSIP_RCVR) || (rcvr_type == UCCM_RCVR)) {
         s1 = "Oscillator discipline:  E)nable  D)isable    S)et DAC voltage";
         two_char = 1;
      }
      else if(rcvr_type == X72_RCVR) {
         if(x72_fw_discipline()) {
            s1 = "Oscillator discipline:  D)isable    E)nable HW discipline    enable S)oftware discipline";
         }
         else {
            s1 = "Software discipline:  D)isable    E)nable";
         }
         two_char = 1;
      }
      else {
         s1 = "Manual disciplining not supported by this receiver... press ESC";
      }
   }
   else if(c == 'e') {
      if(rcvr_type == CS_RCVR) {
         if(no_eeprom_writes) s1 = "EEPROM updates disabled with /kc keyboard command.";
         else {
            s1 = "This will save the current configuration into EEPROM";
            s2 = "Press E again to save the current configuration (ESC to abort)";
         }
         two_char = 1;
      }
      else if(rcvr_type == ESIP_RCVR) {
         if(no_eeprom_writes) s1 = "EEPROM updates disabled with /kc keyboard command.";
         else {
            s1 = "This will save the current configuration into EEPROM";
            s2 = "Use the !h (factory reset) command to reset to the factory state.";
            s3 = "Press E again to save the current configuration (ESC to abort)";
         }
         two_char = 1;
      }
      else if(rcvr_type == PRS_RCVR) {
         s1 = "This will save a config parameter or all config parameters into EEPROM";
         s2 = "Press E again to select the parameter(s) to save (ESC to abort)";
         two_char = 1;
      }
      else if(rcvr_type == SA35_RCVR) {
         s1 = "This will set the frequency correction save into EEPROM mode.";
         s2 = "Press E again to select the frequency correction save mode";
         s3 = "Note that the SA35 EEPROM is spec'd for 10000 total writes";
         two_char = 1;
      }
      else if(rcvr_type == SRO_RCVR) {
         s1 = "This will set the frequency correction save into EEPROM mode.";
         s2 = "Press E again to select the frequency correction save mode";
         s3 = "Note that the SRO100 EEPROM is spec'd for 10000 total writes";
         two_char = 1;
      }
      else if(rcvr_type == TRUE_RCVR) {
         if(no_eeprom_writes) s1 = "EEPROM updates disabled with /kc keyboard command.";
         else {
            s1 = "This will save the current configuration into EEPROM";
            s2 = "Press E again to save the current configiratuion (ESC to abort)";
         }
         two_char = 1;
      }
      else if((rcvr_type == TSIP_RCVR) && (tsip_type != STARLOC_TYPE)) {
         if(no_eeprom_writes) s1 = "EEPROM updates disabled with /kc keyboard command.";
         else {
            s1 = "Press E again to save the current configiratuion into EEPROM (ESC to abort)";
            if(res_t || ACU_360 || ACU_GG) ;
            else if(ACUTIME || PALISADE) {
               s2 = "Press R to revert selected segment to default values (ESC to abort)";
            }
         }
         two_char = 1;
      }
      else if(rcvr_type == UBX_RCVR) {
         if(no_eeprom_writes) s1 = "EEPROM updates disabled with /kc keyboard command.";
         else {
            s1 = "This will save the current configuration into EEPROM";
            s2 = "Press E again to save the current configiratuion (ESC to abort)";
         }
         two_char = 1;
      }
      else if((rcvr_type == VENUS_RCVR) && (lte_lite == 0)) {
         if(no_eeprom_writes) s1 = "EEPROM updates disabled with /kc keyboard command.";
         else if(eeprom_save) s1 = "Press E to DISABLE configuration change writes to EEPROM (ESC ESC to abort)";
         else                 s1 = "Press E to ENABLE configuration change writes to EEPROM (ESC ESC to abort)";
         two_char = 1;
      }
      else if(rcvr_type == X72_RCVR) {
         if(no_eeprom_writes) s1 = "EEPROM updates disabled with /kc keyboard command.";
         else {
            s1 = "This will save the current configuration into EEPROM";
            s2 = "WARNING: this device has no command to reset to the factory state";
            s3 = "Press E again to save the current configuration (ESC to abort)";
         }
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
      else if(rcvr_type == BRANDY_RCVR) {
         s1 = "Enter filter to change: D)isplay";
      }
      else if(rcvr_type == CS_RCVR) {
         s1 = "Enter filter to change: D)isplay";
      }
      else if(rcvr_type == ESIP_RCVR) {
         s1 = "Enter filter to change: D)isplay";
         s2 = "                        signal L)evel (in dBc)   E)levation mask (degrees)";
      }
      else if(rcvr_type == FURUNO_RCVR) {
         s1 = "Enter filter to change: D)isplay";
         s2 = "                        signal L)evel (in dBHz)   E)levation mask (degrees)";
         s3 = "                        X)PDOP mask   M)ovement dynamics   S)moothing index";
      }
      else if(rcvr_type == LPFRS_RCVR) {
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
      else if(rcvr_type == PRS_RCVR) {
         s1 = "Enter filter to change: D)isplay";
      }
      else if(rcvr_type == RFTG_RCVR) {
         s1 = "Enter filter to change: D)isplay";
      }
      else if(rcvr_type == RT17_RCVR) {
         s1 = "Enter filter to change: D)isplay";
      }
      else if(rcvr_type == SA35_RCVR) {
         s1 = "Enter filter to change: D)isplay";
      }
      else if(rcvr_type == SIRF_RCVR) {
         s1 = "Enter filter to change: D)isplay";
         s2 = "                        signal L)evel (in dBc)   E)levation mask (degrees)";
      }
      else if(rcvr_type == SRO_RCVR) {
         s1 = "Enter filter to change: D)isplay";
      }
      else if((rcvr_type == STAR_RCVR) && (star_type == OSA_TYPE)) {
         s1 = "Enter filter to change: D)isplay";
      }
      else if(rcvr_type == STAR_RCVR) {
         s1 = "Enter filter to change: D)isplay   T)imestamp errors";
      }
      else if(rcvr_type == THERMO_RCVR) {
         s1 = "Enter filter to change: D)isplay";
      }
      else if(rcvr_type == TICC_RCVR) {
         s1 = "Enter filter to change: D)isplay";
      }
      else if(rcvr_type == TRUE_RCVR) {
         s1 = "Enter filter to change: D)isplay";
      }
      else if(rcvr_type == TSERVE_RCVR) {
         s1 = "Enter filter to change: D)isplay";
      }
      else if(rcvr_type == TSIP_RCVR) {
         if(ACUTIME || PALISADE || ACU_360 || ACU_GG) s1 = "Enter filter to change: D)isplay";
         else if(SV6_FAMILY) s1 = "Enter filter to change: P)v  S)tatic  A)ltitude  D)isplay";
         else s1 = "Enter filter to change: P)v  S)tatic  A)ltitude  K)alman     D)isplay";

         if(tsip_type == STARLOC_TYPE) ;
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
         if(lte_lite == 0) {
            s2 = "                        signal L)evel (in dBc)   E)levation mask (degrees)";
            s3 = "                        J)amming mode";
         }
      }
      else if(rcvr_type == Z12_RCVR) {
         s1 = "Enter filter to change: D)isplay";
         s2 = "                        signal L)evel (in dBc)   E)levation mask (degrees)";
      }
      else if(rcvr_type == X72_RCVR) {
         s1 = "Enter filter to change: D)isplay";
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
         s5 = "         R)edraw   S)ound    W)atch    B)oth     G)raph title   A)prots";
         s6 = "         E)rrors   /)change statistic  ~)colors";
      }
      else if(rcvr_type == BRANDY_RCVR) {
         s1 = "Display: A)dev  B)oth map & adev tables  C)onstellation   E)rrors    F)FT  G)raph title";
         s2 = "         H)oldover  L)ocation  M)ap,no adevs   Q)uality   R)edraw    K)tide plots";
         s3 = "         S)ound     W)atch  X)dops  Y)filters  Z)clock    ~)colors   /)statistics";
         s4 = "Plot:    1)Lat      2)Lon   3)Alt   V)LLA      6)dop      7)freq trend     8)avg phase";
         s5 = "         D)ac       P)hase  O)sc    T)emperature          fI)x map   $)sats";
         s6 = tide_plots();
      }
      else if(rcvr_type == CS_RCVR) {
         s1 = "Display: E)rrors   F)FT      G)raph title  R)redraw   M)ap";
         s2 = "         S)ound    W)atch    Z)clock       ~)colors   /)statistics";
         s3 = "Plot:    P)ump     O)emult   D)ac          B)eam      C)field   T)emperature";
         s4 = "         1)rfamp1  2)rfamp2  3)gain        4)coven    5)qoven   6)DRO      $)sats";
         s5 = "         7)SAW     8)87MHz   9)uP          I)HWI      K)mspec   V)olt sum";
      }
      else if(rcvr_type == ESIP_RCVR) {
         s1 = "Display: A)dev B)oth map & adev tables C)onstellation    E)rrors   F)FT   G)raph title";
         s2 = "         H)oldover   L)ocation   M)ap,no adevs       Q)uality  R)edraw    K)tide plots";
         s3 = "         S)ound      W)atch      X)dops   Y)filters  Z)clock   ~)colors   /)statistics";
         s4 = "Plot:    1)Lat       2)Lon       3)Alt    V)LLA      6)dop     fI)x map   $)sats";
         s5 = "         P)accu      D)sawtooth";
         s6 = tide_plots();
      }
      else if(rcvr_type == FURUNO_RCVR) {
         s1 = "Display: A)dev  B)oth map & adev tables C)onstellation     E)rrors    F)FT    G)raph title";
         s2 = "         H)oldover  L)ocation  M)ap,no adevs    Q)uality   R)edraw    K)tide plots";
         s3 = "         S)ound  W)atch  X)dops    Y)filters    Z)clock    ~)colors   /)statistics";
         s4 = "Plot:    1)Latitude   2)Longitude   3)Altitude  V)LLA      6)dop      fI)x map   $)sats";
         s5 = tide_plots();
      }
      else if(rcvr_type == GPSD_RCVR) {
         s1 = "Display: A)dev  B)oth map & adev tables  C)onstellation   E)rrors      F)FT   G)raph title";
         s2 = "         H)oldover   L)ocation  M)ap,no adevs     Q)uality   R)edraw   K)tide plots";
         s3 = "         S)ound      W)atch     X)dops  Z)clock   ~)colors   /)statistics";
         s4 = "Plot:    1)Lat       2)Lon      3)Alt   V)LLA     6)dop      fI)x map  $)sats";
         s5 = "         O)bias      P)clkofs   T)emperature";
         s6 = tide_plots();
      }
      else if(rcvr_type == LPFRS_RCVR) {
         s1 = "Display: E)rrors   F)FT       G)raph title  R)redraw   M)ap         K)tide plots";
         s2 = "         S)ound    W)atch     Z)clock       ~)colors   /)statistics";
         s3 = "Plot:    P)PS      O)FC       1)EFC in      2)signal   3)photo I    4)varactor";
         s4 = "         5)lamp I  6)cell I   7)90 MHz AGC  8)FF val";
         s5 = tide_plots();
      }
      else if(rcvr_type == MOTO_RCVR) {
         s1 = "Display: A)dev B)oth map & adev tables C)onstellation    E)rrors   F)FT   G)raph title";
         s2 = "         H)oldover   L)ocation   M)ap,no adevs       Q)uality  R)edraw    K)tide plots";
         s3 = "         S)ound      W)atch      X)dops   Y)filters  Z)clock   ~)colors   /)statistics";
         s4 = "Plot:    1)Lat       2)Lon       3)Alt    V)LLA      6)dop     fI)x map   $)sats";
         s5 = "         D)sawtooth  O)bias      P)accu   T)emperature";
         s6 = tide_plots();
      }
      else if(rcvr_type == NVS_RCVR) {
         s1 = "Display: A)dev B)oth map & adev tables C)onstellation  D)sawtooth E)rrors   F)FT   G)raph title";
         s2 = "         H)oldover    L)ocation  M)ap,no adevs      Q)uality   R)edraw      K)tide plots";
         s3 = "         S)ound  e    W)atch   X)dops   Y)filters   Z)clock    ~)colors     /)statistics";
         s4 = "Plot:    1)Lat        2)Lon    3)Alt    V)LLA       6)dop      fI)x map     $)sats";
         s5 = "         D)sawtooth   O)rate   P)rgen   T)emperature";
         s6 = tide_plots();
      }
      else if(rcvr_type == NMEA_RCVR) {
         s1 = "Display: A)dev  B)oth map & adev tables C)onstellation     E)rrors    F)FT       G)raph title";
         s2 = "         H)oldover  L)ocation  M)ap,no adevs    Q)uality   R)edraw    K)tide plots";
         s3 = "         S)ound  W)atch  X)dops    Y)filters    Z)clock    ~)colors   /)statistics";
         s4 = "Plot:    1)Latitude   2)Longitude   3)Altitude  V)LLA      6)dop      fI)x map   $)sats";
         s5 = tide_plots();
      }
      else if(rcvr_type == PRS_RCVR) {
         s1 = "Display: E)rrors   F)FT       G)raph title  R)redraw   M)ap         K)tide plots";
         s2 = "         S)ound    W)atch     Z)clock       ~)colors   /)statistics";
         s3 = "Plot:    P)TT      O)FC       D)ac          T)emp";
         s4 = "         1)ocxoH   2)cellH    3)lampH       V)all      9)2nd harm   0)sig";
         s5 = "         4)pot     5)thermis  6)Power       7)I/V      8)Photo";
         s6 = tide_plots();
      }
      else if(rcvr_type == RFTG_RCVR) {
         s1 = "Display: A)dev  B)oth map & adev tables  C)onstellation   E)rrors    F)FT    G)raph title";
         s2 = "         H)oldover  L)ocation  M)ap,no adevs   Q)uality   R)edraw    K)tide plots";
         s3 = "         S)ound     W)atch  X)dops  Y)filters  Z)clock    ~)colors   /)statistics";
         if(rftg_unit == 0) {
            s4 = "Plot:    1)Lat      2)Lon   3)Alt   V)LLA      5)OscV     7)LampV    8)OcxoV";
         }
         else {
            s4 = "Plot:    1)Lat      2)Lon   3)Alt   V)LLA      5)OscV     6)AntmA    7)AntV1    8)AntV2";
         }
            s5 = "         D)ac       P)hase  T)emperature       f)Ix map   $)sats";
         s6 = tide_plots();
      }
      else if(rcvr_type == RT17_RCVR) {
         s1 = "Display: A)dev  B)oth map & adev tables  C)onstellation   E)rrors    F)FT   G)raph title";
         s2 = "         H)oldover  L)ocation  M)ap,no adevs   Q)uality   R)edraw    K)tide plots";
         s3 = "         S)ound     W)atch  X)dops  Y)filters  Z)clock    ~)colors   /)statistics";
         s4 = "Plot:    1)Lat      2)Lon   3)Alt   V)LLA      6)dop      fI)x map   $)sats";
         s5 = "         P)clk_ofs  O)freq";
         s6 = tide_plots();
      }
      else if(rcvr_type == SCPI_RCVR) {
         s1 = "Display: A)dev  B)oth map & adev tables  C)onstellation   E)rrors  F)FT       G)raph title";
         s2 = "         H)oldover    L)ocation       M)ap,no adevs   Q)uality     R)edraw    K)tide plots";
         s3 = "         S)ound       W)atch  X)dops  Y)filters       Z)clock      ~)colors   /)statistics";
         s4 = "Plot:    1)Latitude   2)Longitude     3)Altitude      V)LLA        4)tfom     5)ffom   fI)x map";
         s5 = "         O)uncert     P)PPS           D)DAC           $)sats";
         s6 = tide_plots();
      }
      else if(rcvr_type == SIRF_RCVR) {
         s1 = "Display: A)dev  B)oth map & adev tables C)onstellation   E)rrors    F)FT       G)raph title";
         s2 = "         H)oldover    L)ocation   M)ap,no adevs          Q)uality   R)edraw    K)tide plots";
         s3 = "         S)ound       W)atch      X)dops      Y)filters  Z)clock    ~)colors   /)statistics";
         s4 = "Plot:    1)Lat        2)Lon       3)Alt       V)LLA      6)dop      fI)x map   $)sats";
         s5 = "         O)cofs       P)drift";
         s6 = tide_plots();
      }
      else if(rcvr_type == SA35_RCVR) {
         s1 = "Display: E)rrors   F)FT       G)raph title  R)redraw   M)ap         K)tide plots";
         s2 = "         S)ound    W)atch     Z)clock       ~)colors   /)statistics";
         s3 = "Plot:    P)PS      O)FC       1)TEC temp    2)DC sig   3)RF volts   4)Cell I";
//       s4 = "         5)lamp I  6)cell I   7)90 MHz AGC  8)FF val";
         s4 = tide_plots();
      }
      else if(rcvr_type == SRO_RCVR) {
         s1 = "Display: E)rrors   F)FT       G)raph title  R)redraw   M)ap         K)tide plots";
         s2 = "         S)ound    W)atch     Z)clock       ~)colors   /)statistics";
         s3 = "Plot:    P)PS      O)FC       1)EFC in      2)signal   3)photo I    4)varactor";
         s4 = "         5)lamp I  6)cell I   7)loop TC     8)GG val   9)AA val";
         s6 = tide_plots();
      }
      else if(rcvr_type == STAR_RCVR) {
         s1 = "Display: A)dev   B)oth map & adev tables   C)onstellation   E)rrors    F)FT   G)raph title";
         s2 = "         H)oldover   L)ocation    M)ap,no adevs  Q)uality   R)edraw    K)tide plots";
         s3 = "         S)ound      W)atch       Y)filters      Z)clock    ~)colors   /)statistics";
         s4 = "Plot:    1)Latitude  2)Longitude  3)Altitude     V)LLA      fI)x map   $)sats";
         s5 = "         T)emperature";
         s6 = tide_plots();
      }
      else if(rcvr_type == THERMO_RCVR){
         s1 =    "Display: A)dev      E)rrors    F)FT       L)ocation     G)raph title   R)edraw";
         s2 =    "         S)ound     W)atch     Z)clock    ~)colors      /)statistics";
         s3 = "Plot:    T)emp1     V)temp1,humidity,pressure";
         s4 = tide_plots();
         if(have_adc3 || have_adc4) {
            s5 = "         P)adc3     O)adc4";
         }
      }
      else if(rcvr_type == TICC_RCVR){
         s1 =    "Display: A)dev      E)rrors    F)FT       L)ocation     G)raph title   R)edraw    K)tide plots";
         s2 =    "         S)ound     T)emp      W)atch     Z)clock       ~)colors       /)statistics";
         if(two_ticc_mode()) {
            s3 = "TI Err:  P)chA      O)chB      7)chC      8)chD         mtI)e";
            s4 = "Phase:   1)chA      2)chB      3)chC      4)chD         V)all";
         }
         else if(ticc_mode == 'L') {  // Timelab mode
            s3 = "TI Err:  P)chA      O)chB      7)chC";
            s4 = "Phase:   1)chA      2)chB      3)chC      V)all         mtI)e";
         }
         else {
            s3 = "TI Err:  P)chA      O)chB      mtI)e";
            s4 = "Phase:   1)chA      2)chB      V)all";
         }

         if(ticc_mode == 'D') {
            s5 = "Debug:   5)TIME2A   6)TIME2B   D)FUDGE_A";
            s6 = tide_plots();
         }
         else if(ticc_type == LARS_TICC) {
            s5 = "Plots:   D)ac       T)emperature";
            s6 = tide_plots();
         }
         else {
            s5 = tide_plots();
         }
      }
      else if(rcvr_type == TIDE_RCVR) {
         s1 = "Display: E)rrors    L)ocation  G)raph title  R)edraw";
         s2 = "         S)ound     W)atch     Z)clock       ~)colors    /)statistics";
         s3 = "Tides:   KX=lon     KY=lat     KZ=alt        KG=gravity  KA=all   KI=displacement plot";
         s4 = "         fI)x map (lat/lon tide displacement plot)";
         if(have_adc3 || have_adc4) {
            s5 = "         P)adc3     O)adc4";
         }
      }
      else if(rcvr_type == TM4_RCVR) {
         s1 = "Display: A)dev  B)oth map & adev tables  C)onstellation   E)rrors    F)FT   G)raph title";
         s2 = "         H)oldover  L)ocation  M)ap,no adevs   Q)uality   R)edraw    K)tide plots";
         s3 = "         S)ound     W)atch  X)dops  Y)filters  Z)clock    ~)colors   /)statistics";
         s4 = "Plot:    1)Lat      2)Lon   3)Alt   V)LLA      4)speed    5)heading  6)dop";
         s5 = "         fI)x map   $)sats";
         s6 = tide_plots();
      }
      else if(rcvr_type == TRUE_RCVR) {
         s1 = "Display: A)dev  B)oth map & adev tables   C)onstellation    E)rrors    F)FT  G)raph title";
         s2 = "         H)oldover   L)ocation  M)ap,no adevs    Q)uality   R)edraw    K)tide plots";
         s3 = "         S)ound      W)atch  X)dops  Y)filters   Z)clock    ~)colors   /)statistics";
         s4 = "Plot:    1)lat       2)lon   3)alt   V)LLA       4)tfom     5)dis_mode 8)eval  $)sats";
         s5 = "         D)sawtooth  T)emp   fI)x map";
         s6 = tide_plots();
      }
      else if(rcvr_type == TSERVE_RCVR) {
         s1 = "Display: E)rrors    L)ocation  G)raph title  R)edraw     K)tides";
         s2 = "         S)ound     W)atch     Z)clock       ~)colors    /)statistics";
         s3 = "         fI)x map (lat/lon tide displacement plot)";
         s4 = tide_plots();
      }
      else if(rcvr_type == UBX_RCVR) {
         s1 = "Display: A)dev B)oth map & adev tables C)onstellation  E)rrors   F)FT      G)raph title";
         s2 = "         H)oldover   L)ocation   M)ap,no adevs         Q)uality  R)edraw   K)tide plots";
         s3 = "         S)ound      W)atch      X)dops   Y)filters    Z)clock   ~)colors  /)statistics";
         s4 = "Plot:    1)Lat       2)Lon       3)Alt    V)LLA        fI)x map  $)sats";
         s5 = "         D)sawtooth  P)accu      O)frac   T)emperature";
         s6 = tide_plots();
      }
      else if(rcvr_type == UCCM_RCVR) {
         s1 = "Display: A)dev  B)oth map & adev tables  C)onstellation   E)rrors   F)FT   G)raph title";
         s2 = "         H)oldover  L)ocation      M)ap,no adevs   Q)uality   R)edraw      K)tide plots";
         s3 = "         S)ound  W)atch    X)dops   Y)filters      Z)clock    ~)colors     /)statistics";
         s4 = "Plot:    1)Lat   2)Lon     3)Alt    V)LLA          4)tfom     5)ffom       fI)x map";
         if(have_ant_v1 || have_ant_ma) {
            s5 = "         D)ac    O)loop    P)ps     $)sats         6)antV     7)antI";
         }
         else if(have_uccm_tcor || have_uccm_pcor) {
            s5 = "         D)ac    O)loop    P)ps     T)COR          8)PCOR     $)sats";
         }
         else {
            s5 = "         D)ac    O)loop    P)ps     $)sats";
         }
         s6 = tide_plots();
      }
      else if(rcvr_type == X72_RCVR) {
         s1 = "Display: E)rrors   F)FT       G)raph title  R)redraw   M)ap       K)tide plots";
         s2 = "         S)ound    W)atch     Z)clock       ~)colors   /)statistics";
//       s3 = "Plot:    P)PS      O)DDS      D)ac          T)emperature";
         s3 = "Plot:    P)PS      O)DDS      T)emperature";
         s4 = "         1)mvoutc  2)lvoutc   3)rvoutc      V)all      0)htrvolt";
         s5 = "         4)demavg  5)lvolts   6)MP17        7)MP5      8)pres     9)plmp";
         s6 = tide_plots();
      }
      else if(rcvr_type == ZODIAC_RCVR) {
         s1 = "Display: A)dev   B)oth map & adev tables C)onstellation    E)rrors    F)FT  G)raph title";
         s2 = "         H)oldover  L)ocation     M)ap,no adevs  Q)uality  R)edraw    K)tide plots";
         s3 = "         S)ound  W)atch   X)dops  Y)filters      Z)clock   ~)colors   /)statistics";
         s4 = "Plot:    1)Lat   2)Lon    3)Alt   V)LLA          6)dop     fI)x map   $)sats";
         s5 = "         P)PS    O)bias   T)emperature";
         s6 = tide_plots();
      }
      else if(rcvr_type == ZYFER_RCVR) {
         s1 = "Display: A)dev  B)oth map & adev tables  C)onstellation    E)rrors    F)FT  G)raph title";
         s2 = "         H)oldover  L)ocation  M)ap,no adevs    Q)uality   R)edraw    K)tide plots";
         s3 = "         S)ound     W)atch  X)dops  Y)filters   Z)clock    ~)colors   /)statistics";
         s4 = "Plot:    1)hefe     2)hete  3)hest  V)all hold  4)tfom     5)drift    6)gdop   7)tdev";
         s5 = "         D)ac       P)ps    O)sc    T)emp       fI)x map   $)sats";
         s6 = tide_plots();
      }
      else if(TIMING_RCVR) {
         s1 = "Display: A)dev B)oth map & adev tables C)onstellation     E)rrors    F)FT   G)raph title";
         s2 = "         H)oldover   L)ocation  M)ap,no adevs       Q)uality  R)edraw    K)tide plots";
         s3 = "         S)ound      W)atch     X)dops   Y)filters  Z)clock   ~)colors   /)statistics";
         s4 = "Plot:    1)Lat       2)Lon      3)Alt    V)LLA      6)dop     fI)x map   $)sats";
         if(ACU_360 || ACU_GG || ACUTIME || PALISADE) {
            s5 = "         D)sawtooth  P)bias     O)rate   T)emperature";
         }
         else if((rcvr_type == VENUS_RCVR) && have_venus_timing) {
            s5 = "         D)sawtooth";
         }
         else {
            s5 = "         D)sawtooth  P)bias     O)rate   T)emperature";
         }
         s6 = tide_plots();
      }
      else {
         s1 = "Display: A)dev  B)oth map & adev tables  C)onstellation   E)rrors    F)FT   G)raph title";
         s2 = "         H)oldover  L)ocation  M)ap,no adevs   Q)uality   R)edraw    K)tide plots";
         s3 = "         S)ound     W)atch  X)dops  Y)filters  Z)clock    ~)colors   /)statistics";
         s4 = "Plot:    1)Lat      2)Lon   3)Alt   V)LLA      6)dop      fI)x map   $)sats";
         s5 = "         D)ac       P)ps    O)sc    T)emperature";
         s6 = tide_plots();
      }
      two_char = 1;
   }
   else if(c == 'h') {
      if(rcvr_type == BRANDY_RCVR) {
         s1 = "Holdover control not supported by this receiver... press ESC";
      }
      else if(rcvr_type == PRS_RCVR) {
         s1 = "E)nable PPS locking    X)disable PPS locking";
      }
      else if(rcvr_type == RFTG_RCVR) {
         s1 = "Holdover control not supported by this receiver... press ESC";
      }
      else if(rcvr_type == TRUE_RCVR) {
         s1 = "Holdover control not supported by this receiver... press ESC";
      }
      else if((rcvr_type == X72_RCVR) && x72_fw_discipline()) {
         s1 = "E)nable PPS locking    X)disable PPS locking";
      }
      else if(rcvr_type == ZYFER_RCVR) {
         s1 = "Holdover control not supported by this receiver... press ESC";
      }
      else if(lte_lite) {
         s1 = "Holdover control not supported by this receiver... press ESC";
      }
      else if(GPSDO && (rcvr_type != TM4_RCVR)) {
                            s1 = "E)nter holdover    eX)it holdover";
//       if(user_holdover)  s2 = "Press H again to exit user Holdover mode";
//       else               s2 = "Press H again to enter user Holdover mode";
         two_char = 1;
      }
      else {
         s1 = "Holdover control not supported by this receiver... press ESC";
      }
   }
   else if(c == 'j') {
      if(rcvr_type == BRANDY_RCVR) {
         s1 = "Jam sync not supported by this receiver... press ESC";
         two_char = 1;
      }
      else if(rcvr_type == CS_RCVR) {
         s1 = "Sync PPS output to:  O)ff   F)ront panel   R)ear panel";
         two_char = 1;
      }
      else if(rcvr_type == RFTG_RCVR) {
         s1 = "Jam sync not supported by this receiver... press ESC";
         two_char = 1;
      }
      else if(rcvr_type == SRO_RCVR) {
         s1 = "This will Jam sync the PPS output to match the PPS input";
      }
      else if(rcvr_type == STAR_RCVR) {
         s1 = "Jam sync not supported by this receiver... press ESC";
         two_char = 1;
      }
      else if(rcvr_type == TRUE_RCVR) {
         s1 = "Jam sync not supported by this receiver... press ESC";
         two_char = 1;
      }
      else if(rcvr_type == UCCM_RCVR) {
         s1 = "Jam sync not supported by this receiver... press ESC";
         two_char = 1;
      }
      else if(rcvr_type == X72_RCVR) {
         s1 = "This will Jam sync the PPS output to match the PPS input";
      }
      else if(rcvr_type == ZYFER_RCVR) {
         s1 = "Jam sync not supported by this receiver... press ESC";
         two_char = 1;
      }
      else if(lte_lite) {
         s1 = "Jam sync not supported by this receiver... press ESC";
         two_char = 1;
      }
      else if(GPSDO && (rcvr_type != TM4_RCVR)) {
         if(timing_mode == TMODE_UTC)      s1 = "This will Jam sync the PPS output to UTC time";
         else if(timing_mode == TMODE_GPS) s1 = "This will Jam sync the PPS output to GPS time";
         else                              s1 = "This will Jam sync the PPS output";
      }
      else {
         s1 = "Jam sync not supported by this receiver... press ESC";
         two_char = 1;
      }
   }
   else if(c == 'l') {
      if(log_file)     s1 = "S)top logging data to file";
      else             s1 = "Log:  A)ppend to file   W)rite to file   D)elete file   I)nterval   H)ex format";
      if(NO_SATS) ;
      else if(log_db)  s2 = "      stop satellite C)onstellation data in log file";
      else             s2 = "      add satellite C)onstellation data to log file";
      two_char = 1;
   }
   else if(c == 'm') {
      s1 = "Config:    R)ate    F)ormat    X)baud       W)rite file  fI)x data errors";
      s2 = "Antenna:   A)type   H)eight    nU)mber";
      s3 = "Marker:    K)name   V)number   Z)site name";
      s4 = "v2.xx obs: M)ixed (v2.xx)";
      s5 = "v3.xx obs: G)PS     S)BAS      N)Glonass    L)Galileo    B)eidou";
      two_char = 1;
   }
   else if(c == 'p') {
      if(luxor) {
         s1 = "Battery:     H)igh voltage cutoff  L)ow voltage cutoff   W)atts   C)over current";
         s2 = "LED:         V)over voltage        U)nder voltage        P)ower   O)ver current";
         s3 = "AUXV:        X)over voltage        Z)under voltage";
         s4 = "Temperature: T)emp1 overtemp       S)temp2 overtemp";
         s5 = "Misc:        M)sg timeout          R)eset protection faults";
         prot_menu = 1;
         two_char = 1;
      }
      else if(res_t || ACU_GG || ACU_360 || ACUTIME || PALISADE) {
         s1 = "PPS output:  1)PPS mode     2)PP2S mode";
         s2 = "             R)ising edge   F)alling edge  toggle PPS edge P)olarity";
         s3 = "             D)isable PPS   E)nable PPS    toggle PP(S) enable";
         s4 = "Signals:     C)able delay";
         two_char = 1;
      }
      else if(rcvr_type == ACRON_RCVR) {
         s1 = "PPS control not supported by this receiver... press ESC";
      }
      else if(rcvr_type == BRANDY_RCVR) {
         s1 = "THE PULSE CONTROL OPTIONS AFFECT THE J10 CONNECTOR AND NOT THE PPS BNC CONNECTOR!";
         s2 = "PULSE output: R)ising edge     F)alling edge    toggle pulse edge P)olarity";
         s3 = "              D)isable PULSE   E)nable PULSE    toggle pulse enable";
         s4 = "PULSE rate:   1)PPS mode       2)PP2S mode      3)100 PPS mode";
         s5 = "Time code:    T)ime code format";
         two_char = 1;
      }
      else if(rcvr_type == CS_RCVR) {
         s1 = "Outputs:   1)freq1    2)freq2    sY)nc     O)ffset freq     slew P)PS";
         s2 = "Mode:      D)isplay   R)emote    S)tandby";
         s3 = "Time:      set T)ime (from system clock)   set L)eapsecond";
         s4 = "Misc:      C)ontinuous operation lamp reset";
         two_char = 1;
      }
      else if(rcvr_type == ESIP_RCVR) {
         s1 = "PPS output:  D)isable PPS   E)nable PPS     toggle PP(S) enable";
         s2 = "             1)PPS mode     2)PP2S mode     A)PPS width     B)GCLK";
         if(esip_pps_type) {  // GCLK mode
            s3 = "             R)ising edge   F)alling edge   toggle PPS edge P)olarity";
         }
         else {  // LEGACY mode (more accurate PPS)
            s3 = "             R)ising edge";
         }
         s4 = "Freq gen:    G)CLK          L)egacy";
         s5 = "Signals:     C)able delay";
         s6 = "TRAIM:       T)raim control";
         two_char = 1;
      }
      else if(rcvr_type == FURUNO_RCVR) {
         s1 = "PPS output:  D)isable PPS   E)nable PPS   toggle PP(S) enable";
         s2 = "Signals:     C)able delay";
         s3 = "TRAIM:       T)raim control";
         two_char = 1;
      }
      else if(rcvr_type == LPFRS_RCVR) {
         s1 = "F)requency adjust";
         two_char = 1;
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
         two_char = 1;
      }
      else if(rcvr_type == PRS_RCVR) {
         s1 = "PPS output:  H)igh=locked      L)ow=locked     on, F)iltered   on, U)nfiltered";
         s2 = "Magnetics:   S)witching        O)ffset freq";
         s3 = "Position     T)ime tag offset  D)elay offset";
         s4 = "Frequency:   A)djust freq      sY)nthesizer";
         two_char = 1;
      }
      else if(rcvr_type == RFTG_RCVR) {
         s1 = "Signals:     C)able delay      PPS O)ffset";
//!!!!!  s2 = "TRAIM:       T)raim control";
         two_char = 1;
      }
      else if(rcvr_type == RT17_RCVR) {
         s1 = "clock O)ffset observation times";
         two_char = 1;
      }
      else if(rcvr_type == SA35_RCVR) {
         s1 = "E)nable EFC in    D)isable EFC in";
         s2 = "F)requency adjust";
         two_char = 1;
      }
      else if(rcvr_type == SRO_RCVR) {
         s1 = "PPS output:  W)idth        D)elay       R)aw phase adjust";
         s2 = "Windows:     A)larm        T)racking";
         s3 = "Misc:        F)req offset  S)ync mode   tracK) mode    freq saV)e mode";
         s4 = "             set C)lock   pH)ase comparator offset     G)ofast mode";
         two_char = 1;
      }
      else if(rcvr_type == SCPI_RCVR) {
         if(scpi_type == HP_TYPE2) {
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
         if(star_type == OSA_TYPE) {
            s1 = "Signals:     C)able delay   pps D)elay   W)ait-to-restore time   R)eset WTR timers";
            s2 = "Misc:        H)bsq time";
            if(have_pps_rate)  s3 = "TOD message:  1)automatic    2)polled";
            s5 = "The TOD message should normally be set to polled";
         }
         else {
            s1 = "Signals:     C)able delay";
            s2 = "Misc:        H)bsq time     F)ix timestamp errors";
            if(have_pps_rate) s3 = "TOD output:  1)enable       2)disable";
         }
         two_char = 1;
      }
      else if((rcvr_type == THERMO_RCVR) && (enviro_sensors & ENV_EMIS)) {
         s1 = "set E)missivity";
         two_char = 1;
      }
      else if(rcvr_type == THERMO_RCVR) {
         s1 = "PPS control not supported by this receiver... press ESC";
      }
      else if(rcvr_type == TICC_RCVR) {
         if(ticc_type != TAPR_TICC) {
            s1 = "M)ode     N)ominal freq     phase W)rap interval   I)timestamp wrap interval";
            s3 = "Device does not support changing parameters.";
            s4 = "Setting the mode does not alter the counter hardware configuration";
         }
         else {
            s1 = "M)ode        E)dges           F)udge       T)ime2";
            s2 = "S)ync mode   C)al periods     D)ilation    timeO)ut";
            s3 = "R)ef clock   K)coarse clock";
            s4 = "N)ominal freq     phase W)rap interval     I)timestamp wrap interval";
         }
         two_char = 1;
      }
      else if(rcvr_type == TM4_RCVR) {
         s1 = "PPS output:  D)isable PPS   E)nable PPS    pps S)ource";
         s2 = "MUX output:  mux1) freq     mux2) freq";
         s3 = "Signals:     C)able delay   A)ntenna port monitoring";
         s4 = "Time code:   modulated T)ime code format   time port M)essage format";
         two_char = 1;
      }
      else if(rcvr_type == TRUE_RCVR) {
         s1 = "Signals:     C)able delay   B)oard delay   A)ttenuator";
         two_char = 1;
      }
      else if(rcvr_type == TSERVE_RCVR) {
         s1 = "PPS control not supported by this receiver... press ESC";
      }
      else if(SV6_FAMILY && (!ACUTIME) && (!PALISADE) && (!ACU_360) && (!ACU_GG)) {  // ACE3?
         s1 = "PPS control not supported by this receiver... press ESC";
      }
      else if(rcvr_type == TSIP_RCVR) {
         s1 = "PPS output:  R)ising edge   F)alling edge  toggle PPS edge P)olarity";
         s2 = "             D)isable PPS   E)nable PPS    toggle PP(S) enable";
         s3 = "OSC output:  toggle O)sc polarity";
         s4 = "Signals:     C)able delay";
         two_char = 1;
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
         if(lte_lite) {
            s1 = "PPS control not supported by this receiver... press ESC";
         }
         else if(saw_timing_msg && (saw_venus_raw == 0)) {  // timing receiver
            s1 = "PPS output:  B)PPS2 freq and duty cycle";
            s2 = "Signals:     C)able delay";
            s3 = " ";
            s4 = "Note: Max 100000 us PPS pulse width, max 19.2 Mhz freq";
            two_char = 1;
         }
         else if(have_pps_freq) {
            s1 = "PPS output:  A)PPS1 freq and duty cycle";
            s2 = "Signals:     C)able delay";
            s3 = " ";
            s4 = "Note: Max 100000 us PPS pulse width, max 10 Mhz freq";
            two_char = 1;
         }
         else {
            s1 = "PPS output:  A)PPS1 duty cycle";
            s2 = "Signals:     C)able delay";
            s3 = " ";
            s4 = "Note: Max 100000 us PPS pulse width. Freq fixed to 1 Hz";
            two_char = 1;
         }
      }
      else if(rcvr_type == X72_RCVR) {
         s1 = "PPS output:    D)isable PPS   E)nable PPS    toggle PP(S) enable";
         s2 = "SRVC output:   H)igh          L)ow";
         if(x72_type == SA22_TYPE) {
            s3 = "Misc outputs:  A)CMOS         S)ine";
         }
         else {
            s3 = "Misc outputs:  A)CMOS         F)XO           S)ine";
         }
         s4 = "Misc:          efC) input     set T)IC       N)ACMOS divider";
         s5 = "               Z)DDS freq offset      master O)scillator freq";
         two_char = 1;
      }
      else if(rcvr_type == Z12_RCVR) {
         s1 = "PPS control not supported by this receiver... press ESC";
      }
      else if(rcvr_type == ZODIAC_RCVR) {
         s1 = "PPS output:  1)PPS mode       2)100 PPS mode";
         s2 = "             D)isable PPS     E)nable PPS     toggle PP(S) enable";
         s4 = "Signals:     C)able delay     PPS O)ffset";
         s5 = "TRAIM:       T)raim threshold";
         two_char = 1;
      }
      else if(rcvr_type == ZYFER_RCVR) {
         s1 = "PPS output:  R)ising edge   F)alling edge  toggle PPS edge P)olarity";
         s2 = "PPS rate:    1)PPS mode     2)PP2S mode";
         s4 = "Signals:     C)able delay   PPS O)ffset    A)PPS width";
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
         if(rcvr_type == TIDE_RCVR) ;
         else if(check_precise_posn) s6 = "Enter L to abort the precise lat/lon/altitude save (ESC to ignore)";
         else if(precision_survey)   s6 = "Enter P to abort the Precison survey (ESC to ignore)";
         else if(doing_survey && STOPABLE_SURVEY) s6 = "Enter S to abort the Standard survey (ESC to ignore)";
         else if(show_fixes)         s6 = "Enter F to return to position hold mode (ESC to ignore)";

         if(rcvr_type == ACRON_RCVR) {
            s1 = "enter L)at/lon/alt   A)ntenna signal level maps";
            s2 = "sat I)nfo display";
            s3 = "signal path D)elay";
//          s4 = "Q)plot sat PRN az/el/signal level   B)link sat";
         }
         else if(rcvr_type == BRANDY_RCVR) {
            s1 = "enter fix map R)eference position";
            s2 = "sat I)nfo display      A)ntenna signal level maps";
            s3 = "signal path D)elay";
            s4 = "Q)plot sat PRN az/el/signal level   B)link sat";
         }
         else if(rcvr_type == CS_RCVR) {
            s1 = "enter L)at/lon/alt";
            s2 = "signal path D)elay";
//          s3 = "Q)plot sat PRN az/el/signal level   B)link sat";
         }
         else if(rcvr_type == ESIP_RCVR) {
            s1 = "S)tandard survey     P)recision median survey";
            s2 = "enter L)at/lon/alt   enter fix map R)eference position";
            s3 = "position H)old mode  N)3d mode    O)ne sat mode   eX)clude sat";
            s4 = "sat I)nfo display    A)ntenna signal level maps   signal path D)elay";
            s5 = "U)TC offset";
            s6 = "Q)plot sat PRN az/el/signal level    B)link sat";
         }
         else if(rcvr_type == FURUNO_RCVR) {
            s1 = "S)tandard survey     P)recision median survey";
            s2 = "enter L)at/lon/alt   enter fix map R)eference position";
            s3 = "position H)old mode  N)3d mode    O)ne sat mode   eX)clude sat";
            s4 = "sat I)nfo display    A)ntenna signal level maps   signal path D)elay";
            s5 = "Q) plot sat PRN az/el/signal level   B)link sat";
         }
         else if(rcvr_type == LPFRS_RCVR) {
            s1 = "enter L)at/lon/alt";
            s2 = "signal path D)elay";
//          s3 = "Q) plot sat PRN az/el/signal level   B)link sat";
         }
         else if(rcvr_type == MOTO_RCVR) {
            if(moto_chans < 12) {  // these don't do self-surveys
               s1 = "P)recision median survey";
            }
            else {
               s1 = "S)tandard survey     P)recision median survey";
            }
            s2 = "enter L)at/lon/alt   enter fix map R)eference position";
            s3 = "position H)old mode  N)3d mode    O)ne sat mode   eX)clude sat";
            s4 = "do 2D/3d F)ixes      3)2D fixes only   4)3D fixes";
            s5 = "sat I)nfo display    A)ntenna signal level maps   signal path D)elay";
            s6 = "Q)plot sat PRN az/el/signal level    B)link sat";
         }
         else if((rcvr_type == NMEA_RCVR) || (rcvr_type == GPSD_RCVR)) {
            s1 = "P)recision survey    enter fix map R)eference position";
            s2 = "sat I)nfo display    A)ntenna signal level maps";
            s3 = "signal path D)elay";
            s4 = "Q)plot sat PRN az/el/signal level    B)link sat";
         }
         else if(rcvr_type == NO_RCVR) {
            s2 = "enter L)at/lon/alt    enter fix map R)eference position";
            s3 = "signal path D)elay";
//          s4 = "Q)plot sat PRN az/el/signal level   B)link sat";
         }
         else if(rcvr_type == NVS_RCVR) {
            s1 = "S)tandard survey        P)recision median survey";
            s2 = "enter L)at/lon/alt      enter fix map R)eference position";
            s3 = "position H)old mode     N)3d mode    eX)clude sat";
            s4 = "select G)nss systems    sat I)nfo display";
            s5 = "A)ntenna signal maps    signal path D)elay";
            s6 = "Q)plot sat PRN az/el/signal level   B)link sat";
         }
         else if(rcvr_type == PRS_RCVR) {
            s1 = "enter L)at/lon/alt";
            s2 = "signal path D)elay";
//          s3 = "Q)plot sat PRN az/el/signal level   B)link sat";
         }
         else if(rcvr_type == RFTG_RCVR) {
            if(rftg_unit) {  // XO / GPS unit
               s1 = "enter L)at/lon/alt   enter fix map R)eference position";
               s2 = "sat I)nfo display    A)ntenna signal level maps";
               s3 = "signal path D)elay";
               s4 = "Q)plot sat PRN az/el/signal level    B)link sat";
            }
            else {  // Rb unit
               s1 = "enter L)at/lon/alt   enter fix map R)eference position";
               s2 = "signal path D)elay";
//             s3 = "Q)plot sat PRN az/el/signal level   B)link sat";
            }
         }
         else if(rcvr_type == RT17_RCVR) {
            s1 = "enter fix map R)eference position";
            s2 = "sat I)nfo display      A)ntenna signal level maps";
            s3 = "signal path D)elay";
            s4 = "Q)plot sat PRN az/el/signal level   B)link sat";
         }
         else if(rcvr_type == SA35_RCVR) {
            s1 = "enter L)at/lon/alt";
            s2 = "signal path D)elay";
//          s3 = "Q) plot sat PRN az/el/signal level   B)link sat";
         }
         else if(rcvr_type == SCPI_RCVR) {
            s1 = "S)tandard survey    (ESC to ignore)";
            s2 = "enter L)at/lon/alt   enter fix map R)eference position";
            s3 = "O)ne sat mode        eX)clude sat";
            s4 = "sat I)nfo display    A)ntenna signal level maps";
            s5 = "signal path D)elay";
            s6 = "Q)plot sat PRN az/el/signal level    B)link sat";
         }
         else if(rcvr_type == SIRF_RCVR) {
            s1 = "P)recision median survey";
            s2 = "enter fix map R)eference position";
            s3 = "sat I)nfo display    A)ntenna signal level maps";
            s4 = "signal path D)elay";
            s6 = "Q)plot sat PRN az/el/signal level    B)link sat";
         }
         else if(rcvr_type == SRO_RCVR) {
            s1 = "enter L)at/lon/alt";
            s2 = "signal path D)elay";
//          s3 = "Q)plot sat PRN az/el/signal level    B)link sat";
         }
         else if(rcvr_type == STAR_RCVR) {
            s1 = "enter fix map R)eference position";
            s2 = "sat I)nfo display";
            s3 = "A)ntenna signal level maps";
            s4 = "signal path D)elay";
            s5 = "Q)plot sat PRN az/el/signal level    B)link sat";
         }
         else if(rcvr_type == THERMO_RCVR) {
            s1 = "enter L)at/lon/alt";
            s2 = "signal path D)elay";
//          s3 = "Q)plot sat PRN az/el/signal level    B)link sat";
         }
         else if(rcvr_type == TICC_RCVR) {
            s2 = "enter L)at/lon/alt    set ticc M)ode";
            s3 = "signal path D)elay";
//          s4 = "Q)plot sat PRN az/el/signal level    B)link sat";
         }
         else if(rcvr_type == TIDE_RCVR) {
            s2 = "enter L)at/lon/alt   enter fix map R)eference position";
            s3 = "signal path D)elay";
//          s4 = "Q)plot sat PRN az/el/signal level    B)link sat";
         }
         else if(rcvr_type == TRUE_RCVR) {
            s1 = "S)tandard survey       P)recision median survey";
            s2 = "enter L)at/lon/alt     enter fix map R)eference position";
            s3 = "position H)old mode";
            s4 = "sat I)nfo display      A)ntenna signal level maps";
            s5 = "signal path D)elay";
            s6 = "Q)plot sat PRN az/el/signal level    B)link sat";
         }
         else if(rcvr_type == TSERVE_RCVR) {
            s1 = "enter L)at/lon/alt";
            s2 = "signal path D)elay";
//          s3 = "Q)plot sat PRN az/el/signal level    B)link sat";
         }
         else if((rcvr_type == TSIP_RCVR) && (tsip_type == STARLOC_TYPE)) {
            s1 = "S)tandard survey     P)recision median survey";
            s2 = "enter L)at/lon/alt   enter fix map R)eference position";
            s3 = "eX)clude satellite   sat I)nfo display";
            s4 = "A)ntenna signal level maps";
            s5 = "signal path D)elay";
            s6 = "Q)plot sat PRN az/el/signal level    B)link sat";
         }
         else if(rcvr_type == UBX_RCVR) {
            if(saw_timing_msg) s1 = "S)tandard survey       P)recision median survey";
            else               s1 = "P)recision median survey";
            s2 =                    "enter L)at/lon/alt     enter fix map R)eference position";
            s3 =                    "position H)old mode    N)avigation (3D) mode";
            s4 =                    "select G)nss systems   sat I)nfo display";
            s5 =                    "A)ntenna signal level maps          signal path D)elay";
            s6 =                    "Q)plot sat PRN az/el/signal level   B)link sat";
         }
         else if(rcvr_type == UCCM_RCVR) {
            s1 = "S)tandard survey     (ESC to ignore)";
            s2 = "enter L)at/lon/alt   enter fix map R)eference position";
            s3 = "O)ne sat mode        eX)clude sat";
            s4 = "sat I)nfo display    A)ntenna signal level maps";
            s5 = "signal path D)elay";
            s6 = "Q)plot sat PRN az/el/signal level    B)link sat";
         }
         else if(rcvr_type == VENUS_RCVR) {
            if(lte_lite) {
               s1 = "enter fix map R)eference position";
               s2 = "sat I)nfo display";
               s3 = "A)ntenna signal level maps";
               s4 = "signal path D)elay";
               s5 = "Q)plot sat PRN az/el/signal level    B)link sat";
            }
            else if(saw_venus_raw) {
               s1 = "S)tandard survey        P)recision median survey";
               s2 = "enter L)at/lon/alt      enter fix map R)eference position";
               s3 = "H)static mode           N)kinematic mode";
               s4 = "select G)nss systems    sat I)nfo display";
               s5 = "A)ntenna signal maps    signal path D)elay";
               s6 = "Q)plot sat PRN az/el/signal level   B)link sat";
            }
            else if(saw_timing_msg) {
               s1 = "S)tandard survey        P)recision median survey";
               s2 = "enter L)at/lon/alt      enter fix map R)eference position";
               s3 = "position H)old mode     N)3d navigation mode";
               s4 = "select G)nss systems    sat I)nfo display";
               s5 = "A)ntenna signal maps    signal path D)elay";
               s6 = "Q)plot sat PRN az/el/signal level   B)link sat";
            }
            else {
               s1 = "P)recision median survey";
               s2 = "enter fix map R)eference position";
               s3 = "A)ntenna signal level maps";
               s4 = "select G)nss systems      sat I)nfo display";
               s5 = "signal path D)elay";
               s6 = "Q)plot sat PRN az/el/signal level   B)link sat";
            }
         }
         else if(rcvr_type == Z12_RCVR) {
            s1 = "P)recision median survey";
            s2 = "enter L)at/lon/alt     enter fix map R)eference position";
            s3 = "sat I)nfo display      A)ntenna signal level maps";
            s4 = "signal path D)elay";
            s5 = "Q)plot sat PRN az/el/signal level      B)link sat";
         }
         else if(rcvr_type == ZODIAC_RCVR) {
            s1 = "S)tandard survey       P)recision median survey";
            s2 = "enter L)at/lon/alt     enter fix map R)eference position";
            s3 = "position H)old mode    N)avigation (3D) mode";
            s4 = "sat I)nfo display      A)ntenna signal level maps";
            s5 = "O)ne sat mode          eX)clude sat    signal path D)elay";
            s6 = "Q)plot sat PRN az/el/signal level      B)link sat";
         }
         else if(rcvr_type == ZYFER_RCVR) {
            s1 = "S)tandard survey       P)recision median survey";
            s2 = "enter L)at/lon/alt     enter fix map R)eference position";
            s3 = "position H)old mode    N)avigation (3D) mode";
            s4 = "sat I)nfo display      A)ntenna signal level maps";
            s5 = "eX)clude sat           signal path D)elay";
            s6 = "Q)plot sat PRN az/el/signal level      B)link sat";
         }
         else {
            s1 = "S)tandard survey       P)recision median survey";
            s2 = "enter L)at/lon/alt     enter fix map R)eference position";
            s3 = "position H)old mode    N)3d mode    O)ne sat mode   eX)clude sat";
            s4 = "do 2D/3d F)ixes        3)D fixes only      0)..7) set receiver mode";
            if(saw_icm || (res_t == RES_T_360)) {
               s5 = "select G)nss systems   sat I)nfo display   A)ntenna signal level maps";
            }
            else  s5 = "sat I)nfo display      A)ntenna signal maps       signal path D)elay";
            s6 = "Q)plot sat PRN az/el/signal level     B)link sat";
         }
      #else
         s1 = "S)tandard survey     (ESC to ignore)";
         s2 = "enter L)at/lon/alt   enter fix map R)eference position";
         s3 = "do 2D/3d F)ixes      3)D fixes only     0)..7) set receiver mode";
         s4 = "O)ne sat mode        eX)clude sat";
         s5 = "sat I)nfo display    A)ntenna signal maps      signal path D)elay";
         s6 = "Q)plot sat PRN az/el/signal level    B)link sat";
      #endif

      if(luxor) ;
      else two_char = 1;
   }
   else if(c == 't') {
      if(luxor) {
         s1  =   "A)larm time    cH)ime clock";
         #ifdef TEMP_CONTROL
            s2 = "time Z)one     T)emperature control     set tt-ut1 dE)lta_t";
         #else
            s2 = "time Z)one     set tt-ut1 dE)lta-t";
         #endif
         s3 =    "screen D)ump time    L)og dump time    eX)it time   scriP)t time";
      }
      else {
         if(rcvr_type == ESIP_RCVR) {
            s1 =       "Reference:  G)PS time      U)TC time      N)UTC(SU) time    set tt-ut1 dE)lta-t";
         }
         else {
            s1 =       "Reference:  G)PS time      U)TC time      set tt-ut1 dE)lta-t";
         }
         s2 =       "Timers:     A)larm         screen D)ump   L)og dump      eX)it time    scriP)t   exeC)ute";
         s3 =       "Clock:      N)ormal        M)illisecond   tW)elve hour   J)ulian       Q)modified Julian";
         s4 =       "Misc:       time Z)one     cH)ime time    sun/moon R)ise info          I)nterval";
         #ifdef TEMP_CONTROL
            if(have_temperature && (tsip_type != STARLOC_TYPE)) {
               s5 = "            S)et system time    time message O)ffset   T)emperature control";
               s6 = "            B)calendar";
            }
            else {
               s5 = "            S)et system time              time message O)ffset         B)calendar";
            }
         #else
               s5 = "            S)et system time              time message O)ffset         B)calendar";
         #endif

         if(rcvr_type == SCPI_RCVR) {
            s6 = "CHANGING BETWEEN UTC/GPS TIME WILL RESTART THE RECEIVER!";
         }
      }
      two_char = 1;
   }
   else if(c == 'u') {
      if(pause_data) s1 =    "This will resume plot/adev queue Updates";
      else           s1 =    "This will pause plot/adev queue Updates";
   }
   else if(c == 'w')  {
      if(filter_log) s1 =    "Write to file:  A)ll queue data   P)lot area    D)elete file   unF)iltered data";
      else           s1 =    "Write to file:  A)ll queue data   P)lot area    D)elete file   F)iltered data";
      if(luxor) {
         s2 =                "                S)creen dump      R)everse video screen dump   L)og";
         s4 =                "                C)onfig data      &)calibration data";
      }
      else {
         s2 =                "                S)creen dump      R)everse video screen dump   L)og";
      }
      s3 =                   "                G)raph area dump  I)nverse video graph area    Z)signale levels";
      if(rcvr_type == TICC_RCVR) {
         s4 =                "                X)debug log       Y)receiver data capture      T)raw TICC data   M)TIE data";
      }
      else if(TICC_USED) {
         s4 =                "                X)debug log       Y)receiver data capture      T)raw TICC data";
      }
      else if(jd_obs) {
         s4 =                "                X)debug log       Y)receiver data capture";
      }
      else {
         s4 =                "                X)debug log       Y)receiver data capture";
      }
      if(sim_file)      s5 = "                E)pause simulation file";
      two_char = 1;
   }
   else if(c == 'z') {
      if((zoom_screen == 0) || (zoom_screen == 'K')) {
         if(luxor) {
            s1 = "Zoom:  C)lock   W)atch    N)o data   P)lots   mO)nitor   Z)normal";
            s2 = "       T)imeout keyboard in activity          H)ex packets";
            s3 = "       `)calculator       Q)calendar";
         }
         else if(rcvr_type == CS_RCVR) {
            s1 = "Zoom:  C)lock   W)atch   B)watch with sun   P)lots   N)o data  mO)nitor";
            s2 = "       D)ata    Z)normal";
            s3 = "       T)imeout keyboard in activity        H)ex packets";
            s4 = "       `)calculator      Q)calendar";
         }
         else if(rcvr_type == LPFRS_RCVR) {
            s1 = "Zoom:  C)lock   W)atch   B)watch with sun   P)lots   N)o data  mO)nitor";
            s2 = "       D)ata    Z)normal";
            s3 = "       T)imeout keyboard in activity        H)ex packets";
            s4 = "       `)calculator      Q)calendar";
         }
         else if(rcvr_type == NO_RCVR) {
            s1 = "Zoom:  C)lock   W)atch    B)watch with sun   N)o data    mO)nitor";
            s2 = "       Z)normal";
            s3 = "       T)imeout keyboard in activity         H)ex packets";
            s4 = "       `)calculator       Q)calendar";
         }
         else if(rcvr_type == PRS_RCVR) {
            s1 = "Zoom:  C)lock   W)atch   B)watch with sun   P)lots   N)o data   mO)nitor";
            s2 = "       D)ata    Z)normal";
            s3 = "       T)imeout keyboard in activity        H)ex packets";
            s4 = "       `)calculator      Q)calendar";
         }
         else if(rcvr_type == SA35_RCVR) {
            s1 = "Zoom:  C)lock   W)atch   B)watch with sun   P)lots   N)o data  mO)nitor";
            s2 = "       D)ata    Z)normal";
            s3 = "       T)imeout keyboard in activity        H)ex packets";
            s4 = "       `)calculator      Q)calendar";
         }
         else if(rcvr_type == SRO_RCVR) {
            s1 = "Zoom:  C)lock   W)atch   B)watch with sun   P)lots   N)o data   mO)nitor";
            s2 = "       D)ata    Z)normal";
            s3 = "       T)imeout keyboard in activity        H)ex packets";
            s4 = "       `)calculator      Q)calendar";
         }
         else if(rcvr_type == RFTG_RCVR) {
            s1 = "Zoom:  C)lock   W)atch     B)watch with sun   P)lots   N)o data   mO)nitor";
            s2 = "       D)ata    Z)normal   sat I)nfo";
            s3 = "       T)imeout keyboard in activity          H)ex packets";
            s4 = "       `)calculator        Q)calendar";
         }
         else if(rcvr_type == TSERVE_RCVR) {
            s1 = "Zoom:  C)lock   W)atch   B)watch with sun   P)lots   N)o data   mO)nitor";
            s2 = "       D)ata    Z)normal";
            s3 = "       T)imeout keyboard in activity        H)ex packets";
            s4 = "       `)calculator      Q)calendar";
         }
         else {
            s1 = "Zoom:  C)lock     W)atch     M)ap       B)oth watch and map";
            s2 = "       L)LA       N)o data   P)lots     sat I)nfo       mO)nitor";
            s3 = "       S)ignals   A)zimuth   E)levation     R)elative   D)ata   U)all";
            s4 = "       X)watch/map/signals   Y)map/signals  V)watch+sats+sigs   Z)normal";
            s5 = "       T)imeout keyboard in activity        H)ex packets";
            s6 = "       `)calculator          Q)calendar";
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
      else if(rcvr_type == BRANDY_RCVR) {
         s1 = "Show frequency control parameters";
         two_char = 1;
      }
      else if(rcvr_type == ESIP_RCVR) {
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
      else if(rcvr_type == PRS_RCVR) {
         s2 = "Oscillator:   D)stability   G)ain   I)ntegrator   T)ime const";
         two_char = 1;
      }
      else if(rcvr_type == RFTG_RCVR) {
         s1 = "Show oscillator control parameters";
         two_char = 1;
      }
      else if(rcvr_type == SCPI_RCVR) {
         s1 = "Tune: A)utotune elevation mask";
         two_char = 1;
      }
      else if(rcvr_type == SIRF_RCVR) {
         s1 = "Tune: A)utotune elevation and signal level masks";
         two_char = 1;
      }
      else if(rcvr_type == SRO_RCVR) {
         s2 = "T)ime const";
         two_char = 1;
      }
      else if(rcvr_type == STAR_RCVR) {
         if(star_type != OSA_TYPE) {
            s1 = "Tune: A)utotune elevation and signal level masks";
         }
         if(have_gpsdo_ref) s2 = "Osc:  T)ime constant   R)eference source";
         else               s2 = "Osc:  T)ime constant";
         two_char = 1;
      }
      else if(rcvr_type == TICC_RCVR) {
         if(ticc_type == TAPR_TICC) {
            s1 = "Tune: A)utotune TICC FUDGE and TIME2 values";
            two_char = 1;
            osc_params = 0;
         }
         else {
            s1 = "Oscillator control not supported by this receiver... press ESC";
            osc_params = 0;
         }
      }
      else if(rcvr_type == TRUE_RCVR) {
         s1 = "Tune: A)utotune (learn) OCXO EFC parameters";
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
      else if(rcvr_type == VENUS_RCVR) {
         s1 = "Tune: A)utotune elevation and signal level masks";
         two_char = 1;
      }
      else if(rcvr_type == X72_RCVR) {
         if(x72_fw_discipline())  s1 = "Oscillator:   A)utotune DDS     D)amping   T)ime const";
         else                     s1 = "Oscillator:   A)utotune DDS     T)ime const   J)amsync thresh   H)oldover time";
         two_char = 1;
      }
      else if(rcvr_type == ZODIAC_RCVR) {
         s1 = "Tune: A)utotune elevation and signal level masks";
         two_char = 1;
      }
      else if((GPSDO || STARLOC) && (rcvr_type != ZYFER_RCVR)) {
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
         else if(rcvr_type == BRANDY_RCVR) {
            s1 = "H)ard reset to factory settings";
         }
         else if(rcvr_type == CS_RCVR) {
            s1 = "H)ard reset to factory settings";
         }
         else if(rcvr_type == ESIP_RCVR) {
            s1 = "W)arm reset   C)old reset   H)ard reset to factory settings";
         }
         else if(rcvr_type == FURUNO_RCVR) {
            s1 = "W)arm reset   C)old reset   H)ard reset to factory settings";
         }
         else if(rcvr_type == MOTO_RCVR) {
            s1 = "H)ard reset to factory settings";
         }
         else if(rcvr_type == NVS_RCVR)  {
            s1 = "W)arm reset   H)ard reset to factory settings";
         }
         else if(rcvr_type == PRS_RCVR)  {
            s1 = "W)arm reset   H)ard reset to factory settings";
         }
         else if(rcvr_type == RFTG_RCVR)  {
            s1 = "C)old reset            D)isable unit             R)e-enable unit";
         }
         else if(rcvr_type == SRO_RCVR)  {
            s1 = "C)old reset";
         }
         else if(rcvr_type == STAR_RCVR) {
            if(star_type == NEC_TYPE) s1 = 0;
            else s1 = "W)arm reset   H)ard reset to factory settings";
         }
         else if((rcvr_type == SCPI_RCVR) || (rcvr_type == UCCM_RCVR)) {
            s1 = "C)old reset   H)ard reset to factory settings";
         }
         else if(rcvr_type == TAIP_RCVR) {
            s1 = "W)arm reset   C)old reset   H)ard reset to factory settings";
         }
         else if(rcvr_type == TICC_RCVR) {
            s1 = "C)old reset     H)ard reset to factory settings";
         }
         else if(rcvr_type == TM4_RCVR) {
            s1 = "H)master reset";
         }
         else if(rcvr_type == TRUE_RCVR) {
            s1 = "W)arm reset   H)ard reset to factory settings";
         }
         else if((rcvr_type == TSIP_RCVR) || (rcvr_type == UBX_RCVR) || (rcvr_type == ZODIAC_RCVR)) {  // moto tsip ubx zodiac
            s1 = "W)arm reset   C)old reset   H)ard reset to factory settings";
         }
         else if((rcvr_type == VENUS_RCVR) && (lte_lite == 0)) {
            s1 = "W)arm reset   C)old reset   H)ard reset to factory settings";
         }
         else if(rcvr_type == Z12_RCVR) {
            s1 = "C)intern (cfg) mem     W)extern (data) mem       H)ard factory reset";
         }
         else if(rcvr_type == ZYFER_RCVR) {
            s1 = "W)arm reset   C)old reset   H)ard reset to factory settings";
         }
         else s1 = "";

         s2 =    "re-init S)erial port   set serial P)ort config   Z)reset parser";
         s3 =    "mO)nitor mode          T)erminal emulator        send B)reak";

         if(rcvr_type == ACRON_RCVR) {
            s4 = "send U)ser command to receiver";
            s5 = "E)xecute program";
         }
         else if(rcvr_type == BRANDY_RCVR) {
            s4 = "send U)ser command to receiver";
            s5 = "E)xecute program";
         }
         else if(rcvr_type == CS_RCVR) {
            s4 = "run D)iagnostics       send U)ser command to device";
            s5 = "E)xecute program       reset continuous operation L)ight";
         }
         else if(rcvr_type == ESIP_RCVR) {
            s4 = "send U)ser command     switch receiver language M)ode";
            s5 = "E)xecute program       X)set receiver baud rate";
         }
         else if(rcvr_type == FURUNO_RCVR) {
            s4 = "run D)iagnostics       send U)ser command to device";
            s5 = "E)xecute program";
         }
         else if(rcvr_type == GPSD_RCVR) {
            s4 = "send U)ser command to receiver";
            s5 = "E)xecute program";
         }
         else if(rcvr_type == LPFRS_RCVR) {
            s4 = "send U)ser command to receiver";
            s5 = "E)xecute program";
         }
         else if(rcvr_type == MOTO_RCVR) {
            s4 = "switch receiver language M)ode";
            s5 = "run receiver D)iagnostics";
            s6 = "E)xecute program";
         }
         else if(rcvr_type == NMEA_RCVR) {
            s4 = "send U)ser command to receiver   N)enable Motorola binary mode";
            s5 = "E)xecute program";
         }
         else if(rcvr_type == NO_RCVR) {
            s4 = "set clock update R)ate";
            s5 = "E)xecute program";
         }
         else if(rcvr_type == NVS_RCVR) {
            s4 = "run receiver D)iagnostics         set nav R)ate";
            s5 = "E)xecute program       X)set receiver baud rate";
         }
         else if(rcvr_type == PRS_RCVR) {
            s4 = "send U)ser command to receiver";
            s5 = "E)xecute program";
         }
         else if(rcvr_type == SA35_RCVR) {
            s4 = "send U)ser command to receiver";
            s5 = "E)xecute program";
         }
         else if(rcvr_type == SRO_RCVR) {
            s4 = "send U)ser command to receiver";
            s5 = "E)xecute program";
         }
         else if(rcvr_type == SCPI_RCVR) {
            s4 = "run receiver D)iagnostics";
            s5 = "send U)ser command to receiver";
            s6 = "E)xecute program";
         }
         else if(rcvr_type == SIRF_RCVR) {
            s4 = "switch receiver to NMEA M)ode";
            s5 = "E)xecute program";
         }
         else if(rcvr_type == STAR_RCVR) {
            s4 = "send U)ser command to receiver";
            s5 = "E)xecute program";
         }
         else if(rcvr_type == TIDE_RCVR) {
            s4 = "set clock update R)ate";
            s5 = "E)xecute program";
         }
         else if(rcvr_type == TSERVE_RCVR) {
            s4 = "send U)ser command to receiver";
            s5 = "E)xecute program";
         }
         else if(rcvr_type == TM4_RCVR) {
            s4 = "send U)ser command to receiver";
            s5 = "E)xecute program      X)set time port baud rate";
         }
         else if(rcvr_type == UBX_RCVR) {
            s4 = "set nav R)ate          switch receiver to NMEA M)ode";
            s5 = "E)xecute program       X)set receiver baud rate";
         }
         else if(rcvr_type == UCCM_RCVR) {
            if(scpi_type == UCCMP_TYPE) s4 = "run receiver D)iagnostics";
            s5 = "send U)ser command to receiver";
            s6 = "E)xecute program";
         }
         else if((rcvr_type == VENUS_RCVR) && (lte_lite == 0)) {
            s4 = "set nav R)ate          switch receiver to NMEA M)ode";
            s5 = "E)xecute program       enable rtK) mode";
         }
         else if(rcvr_type == Z12_RCVR) {
            s4 = "E)xecute program       X)set receiver baud rate";
         }
         else if(rcvr_type == ZODIAC_RCVR) {
            s4 = "switch to Motorola M)ode";
            s5 = "run receiver D)iagnostics";
            s6 = "send U)ser command to receiver    E)xecute program";
         }
         else {
            s4 = "E)xecute program";
         }
      }
      two_char = 1;
   }
   else if(c == '$') {
      s1 = "Select screen size:  T)ext         D)480x320     E)320x480     U)640x480";
      s2 = "   P)800x480         S)800x600     N)1000x540    R)1024x600    M)1024x768";
      s3 = "   J)1280x800        K)1280x960    L)1280x1024   V)1440x900    X)1680x1550";
      s4 = "   H)1920x1080       Z)2048x1536   C)ustom       F)ull";
      s5 = "   O)rotate screen   I)nvert black and white on screen         Q)scaled fonts";
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
         if(disable_kbd)       sprintf(out, "Press 'y' to exit...");
         else if(esc_esc_exit) sprintf(out, "Press 'y' or ESC to exit...");
         else                  sprintf(out, "Press 'y' to exit...");
esc_esc_esc = 1;
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
   else if(c == PAGE_UP) {  // edit last line
      strcpy(edit_buffer, last_edit_buf);
      edit_cursor = edit_ptr = strlen(edit_buffer);
      first_edit = 1;
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

         if(zoom_screen == '`') {  // cancel zoomed calculator
            rpn_mode = 0;
            remove_zoom();
         }
         else if(rpn_mode) {
            rpn_mode = (-1);
         }

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
      strcpy(last_edit_buf, edit_buffer);
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

   attr = WHITE;

   col = TEXT_COLS-SLEN-2;
   if(col < 0) col = 0;
   vidstr(e_row, EDIT_COL, WHITE, &blanks[col]);

   // kludgy,  but no-brainer way to erase old edit cursor
   col = (EDIT_COL+1+edit_cursor) * TEXT_WIDTH;
   row = e_row * TEXT_HEIGHT;
   if(zoom_screen == '`') {
      row += TEXT_HEIGHT;
      col += TEXT_WIDTH*2;
   }
   if(row < PLOT_TEXT_ROW) {  // undo Windows margin offsets
      if(no_x_margin == 0) col += TEXT_X_MARGIN; //*vc_font_scale / 100;
      if(no_y_margin == 0) row += TEXT_Y_MARGIN; //*vc_font_scale / 100;
   }
   line(0,row+TEXT_HEIGHT, SCREEN_WIDTH-1,row+TEXT_HEIGHT, BLACK);
   line(0,row+TEXT_HEIGHT+2, SCREEN_WIDTH-1,row+TEXT_HEIGHT+2, BLACK);

   sprintf(out, ">%s", edit_buffer);
   if((c == 0x0D) && (zoom_screen == '`')) {  // show empty edit line
      vidstr(e_row, EDIT_COL, WHITE, ">");
   }
   else vidstr(e_row, EDIT_COL, WHITE, out);  // show current edit line

   if(c != 0x0D) {  // draw the edit cursor under the char
      line(col,row+TEXT_HEIGHT, col+TEXT_WIDTH-1,row+TEXT_HEIGHT, PROMPT_COLOR);
      if(insert_mode) line(col,row+TEXT_HEIGHT+2, col+TEXT_WIDTH-1,row+TEXT_HEIGHT+2, PROMPT_COLOR);
   }

   if(fast_script && script_file && (script_pause == 0)) ;  // this makes script go faster,  but you can't see what is happening
   else refresh_page();

   return c;
}




int ignore_blank_line(int c)
{
   // check which getting_string commands allow blank input lines
   // this routine was blank_lines_ok()

   if(c == ABORT_SURV_CMD) return 0;   // these commands allow blank lines as input
   if(c == ABORT_LLA_CMD) return 0;
   if(c == ADEV_HIDE_CMD) return 0;
   if(c == ALARM_CMD) return 0;
   if(c == CALC_CMD) return 0;
   if(c == CENTER_CMD) return 0;
   if(c == DRIFT_CMD) return 0;
   if(c == DRVR_CMD) return 0;
   if(c == EXEC_PGM_CMD) return 0;
   if(c == EXIT_CMD) return 0;
   if(c == LOG_DUMP_CMD) return 0;
   if(c == RINEX_ANT_TYPE_CMD) return 0;
   if(c == RINEX_ANT_NUM_CMD) return 0;
   if(c == MARKER_NAME_CMD) return 0;
   if(c == MARKER_NUM_CMD) return 0;
   if(c == RINEX_LIST_CMD) return 0;
   if(c == SCREEN_DUMP_CMD) return 0;
   if(c == SCRIPT_RUN_CMD) return 0;
   if(c == SET_TZ_CMD) return 0;
   if(c == SIGNAL_CMD) return 0;
   if(c == STAT_CMD) return 0;
   if(c == SUN_CMD) return 0;
   if(c == TEMP_SET_CMD) return 0;
   if(c == TITLE_CMD) return 0;
   if(c == TRACK_PORT_CMD) return 0;

   return 1;
}

int start_edit(int c, char *prompt)
{
int i, col;
int row;

   //
   // Start building the text string for command parameter *c*
   //
   // Note that blank input lines are not processed unless allowed by the
   // function ignore_blank_line()
   // Search for function ignore_blank_line()e to add more "blank ok" commands
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
   if(edit_info5) {
      vidstr(row, EDIT_COL, PROMPT_COLOR, edit_info5);
      ++row;
      ++e_row;
      edit_info5 = 0;
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

   rpn_break = 1;
   edit_err_flag = 1;

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
   vidstr(row+2, col, PROMPT_COLOR, &blanks[TEXT_COLS-width]);
   vidstr(row+3, col, PROMPT_COLOR, &blanks[TEXT_COLS-width]);
   vidstr(row+4, col, PROMPT_COLOR, &blanks[TEXT_COLS-width]);
   vidstr(row+5, col, PROMPT_COLOR, &blanks[TEXT_COLS-width]);

   if(script_file) {
      sprintf(out, "Error in script file %s: line %d, col %d: %s",
         script_name, script_line, script_col, s);
      vidstr(row+1, col, PROMPT_COLOR, out);
      vidstr(row+2, col, PROMPT_COLOR, "Press any key to stop script....");
   }
   else {
      vidstr(row+1, col, PROMPT_COLOR, s);
      vidstr(row+2, col, PROMPT_COLOR, "Press any key to continue....");
   }

   if(rpn_mode) {
      BEEP(6566);
   }

   refresh_page();

   //!!! just waiting for a key here would be bad news because
   //    gps data would not be processed and the queue can overflow
   wait_for_key(1);

   vidstr(row+1, col, PROMPT_COLOR, "                                ");
   refresh_page();

   if(KBHIT()) x = GETCH();
   else        x = 0;
   script_fault = 1;

   if(rpn_mode) {
      edit_buffer[0] = 0;
      edit_buffer[1] = 0;
      msg_field[0] = 0;
      nmea_msg[0] = 0;
      msg_col = 0;
      start_calc(123);
      rpn_mode = (-1);
      rpn_err_flag = 1;
   }
   return x;
}



int build_string(int c)
{
u32 val;
int not_upper;
int i;

   // add a character to or edit the input string

   val = toupper(c & 0x7F);
if(getting_string == CALC_CMD) {
   i = rpn_mode;
   rpn_mode = 1;
   show_version_header();
   rpn_mode = i;
}

   // we are building a text string from the keystrokes / script input
   c = edit_string(c);     // attempt to add char to the string
   if(getting_string == SORT_CMD) {  // royal kuldge for GCC, GCG, GCZ, and GCT keyboard commands
      if(val == 'C') goto quick_cmd;  // GCC
      if(val == 'T') goto quick_cmd;  // GCT
      if(val == 'X') goto quick_cmd;  // GCX
      if(val == 'Z') goto quick_cmd;  // GCZ
      if(val == 'G') {
         getting_string = SAT_PLOT_CMD;
         goto quick_cmd;  // GCG
      }
   }
   else if(getting_string == TIDE_CMD) {  // royal kludge for GKx earth tide / gravity plot control
      goto quick_cmd;
   }

   if(getting_string < 0x0100) not_upper = !isupper(getting_string);
   else                        not_upper = 1;

   if(c == ESC_CHAR) {     // edit abandoned
      goto abandon_edit;
   }
   else if((c == 0x0D) && (edit_buffer[0] == 0) && ignore_blank_line(getting_string)) {  // blank input line
      // CR with no text in edit buffer
      abandon_edit:
      getting_string = 0;
      script_pause = 0;
      if(text_mode) redraw_screen();
      else if(rcvr_type == NO_RCVR) erase_plot(FULL_ERASE_OK);
      else if((PLOT_WIDTH/TEXT_WIDTH) <= 80) erase_plot(FULL_ERASE_OK);
   }
   else if(c == 0x0D) { // edit done, parameter available,  do the command
      quick_cmd:
      script_pause = 0;
      val = string_param();  // evaluate and act upon the text string
      getting_string = 0;

      if(val == NEED_NEW_QUEUES) new_queue(RESET_ALL_QUEUES, 10);
      else if(val == NEED_SCREEN_REDRAW) redraw_screen();
      else if(rcvr_type == NO_RCVR) erase_plot(FULL_ERASE_OK);
      else if((PLOT_WIDTH/TEXT_WIDTH) <= 80) erase_plot(FULL_ERASE_OK);
   }
   else if(getting_string == ADEV_CMD) {  // !!! kludge to make parsing command easier !!!
      goto quick_cmd;
   }
// else if(getting_string == ADEV_BIN_CMD) {  // !!! kludge to make parsing command easier !!!
//    goto quick_cmd;
// }
   else if(getting_string == SIGNAL_CMD) {  // !!! kludge to make parsing command easier !!!
      goto quick_cmd;
   }
   else if(getting_string == STAT_CMD) {  // !!! kludge to make parsing command easier !!!
      goto quick_cmd;
   }
   else return 0;  // character was added to the edit string

   return sure_exit();
}


void edit_scale()
{
DATA_SIZE val;
char *u;

   // get a plot scale factor
   if(plot[selected_plot].user_scale) val = 0.0;
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
   start_edit(SCALE_CMD, out);
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
      else if(DEG_SCALE == 'H')  u = "degrees H";
      else                       u = "degrees C";
   }
   else u = plot[selected_plot].units;
   if(u[0] == ' ') ++u;
u = "units";

   if(plot[selected_plot].drift_rate) sprintf(edit_buffer, "0");
   else if(rcvr_type == TICC_RCVR) sprintf(edit_buffer, "1");
   else sprintf(edit_buffer, "%g", plot[selected_plot].a1);

   if(dynamic_trend_line) {
      sprintf(out, "Remove linear residual to remove from %s graph (0=disable  1=emable)  (ESC ESC to abort)",
                    plot[selected_plot].plot_id);
   }
   else {

      sprintf(out, "Enter drift rate to remove from %s graph (%s/sec 0=disable)  (ESC ESC to abort)",
                    plot[selected_plot].plot_id, u);
   }
   start_edit(DRIFT_CMD, out);
}

void edit_ref()
{
DATA_SIZE val;
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
   start_edit(CENTER_CMD, out);
}


void edit_cal()
{
DATA_SIZE m, b;
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
float m, b;            // !!!! float
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


void view_all(int set_user_view)
{
double val;

   val = (double) plot_q_count;
   view_interval = 0;
   while(val > 0.0) {
      val -= (double) PLOT_WIDTH;
      ++view_interval;
   }
// view_interval = (long) (((val+PLOT_WIDTH-1) / PLOT_WIDTH) + 0.5F);
   if(view_interval <= 1L) view_interval = 1L;
   if(set_user_view) {
      user_view = view_interval;
      new_user_view = 1;
      last_was_adev = 0;
      do_review(END_CHAR, 1);
   }
}


int edit_user_view(char *s)
{
DATA_SIZE val;

   // process the new plot view time setting
   val = 0.0;
   strcpy(out, s);
   strupr(out);
   if(strstr(out, "A")) {   // view ALL data
      view_all_data = 1;
      need_view_auto = 0;
   }
   else if(strstr(out, "T")) {   // view ALL data, auto scroll
      view_all_data = 2;
//    need_view_auto = 0;
   }
   else {                   // view a subset of the data
      view_all_data = 0;
      need_view_auto = 0;
      val = (DATA_SIZE) atof(out);
      if(val < 0.0) {
         val = (DATA_SIZE) 0.0 - val;
      }
      else if(val == 0.0) {
         user_view = 0L;
         view_interval = 1L;
         return 0;
      }
   }
   val *= nav_rate;

   if(queue_interval == 0) ;
   else if(strstr(out, "W")) {
      view_interval = (long) ((((val * (DATA_SIZE) (24.0*60.0*60.0)*7.0) / (DATA_SIZE) queue_interval) / PLOT_WIDTH) + (DATA_SIZE) 0.5);
   }
   else if(strstr(out, "D")) {
      view_interval = (long) ((((val * (DATA_SIZE) (24.0*60.0*60.0)) / (DATA_SIZE) queue_interval) / PLOT_WIDTH) + (DATA_SIZE) 0.5);
   }
   else if(strstr(out, "H")) {
      view_interval = (long) ((((val * (DATA_SIZE) (60.0*60.0)) / (DATA_SIZE) queue_interval) / PLOT_WIDTH) + (DATA_SIZE) 0.5);
   }
   else if(strstr(out, "M")) {
      view_interval = (long) ((((val * (DATA_SIZE) 60.0) / (DATA_SIZE) queue_interval) / PLOT_WIDTH) + (DATA_SIZE) 0.5);
   }
   else if(strstr(out, "S")) {
      view_interval = (long) (((val / (DATA_SIZE) queue_interval) / PLOT_WIDTH) + (DATA_SIZE) 0.5);
   }
   else if(strstr(out, "T")) {
      view_all(1);
      return 0;
   }
   else if(strstr(out, "A")) {
      view_all(1);
      return 0;
   }
   else if(queue_interval) {  // interval set in minutes per division
      val /= (DATA_SIZE) queue_interval;
      val *= (DATA_SIZE) 60.0;                   // seconds/division
      val /= (DATA_SIZE) HORIZ_MAJOR;
      view_interval = (long) val;
   }

   if(view_interval <= 1L) view_interval = 1L;
   user_view = view_interval;
   new_user_view = 1;

   return 0;
}


void edit_dt(char *s,  int err_ok)
{
char x1[SLEN+1], x2[SLEN+1];
char c;
unsigned i,j;

int aa,bb,cc;
u08 s1,s2;
int dd,ee,ff;
u08 s3,s4;

long val;
u08 timer_set;
char *arg;


   // parse the alarm/exit/log/screen dump date and time variables
   // Warning: this code sucks donkey balls... sorry about that!

   if(s == 0) return;

   arg = trim_whitespace(s);
   if(arg == 0) arg = "";

   if(arg[0] == 0) {  // no string given,  clear the appropriate timer values
      alarm_wait = 0;
      if(getting_string == ALARM_CMD) {      // alarm/egg timer
         alarm_time = alarm_date = 0;
         alarm_jd = 0.0;
         egg_timer = egg_val = 0;
         repeat_egg = 0;
         reset_alarm();
         modem_alarms = 0;
      }
      else if(getting_string == EXIT_CMD) { // exit time/timer
         end_time = end_date = 0;
         exit_jd = 0.0;
         exit_timer = exit_val = 0;
         repeat_exit = 0;
      }
      else if(getting_string == SCREEN_DUMP_CMD) { // screen dump timer
         dump_time = dump_date = 0;
         dump_jd = 0.0;
         dump_timer = dump_val = 0;
         repeat_dump = 0;
      }
      else if(getting_string == SCRIPT_RUN_CMD) { // script run timer
         script_time = script_date = 0;
         script_jd = 0.0;
         script_timer = script_val = 0;
         repeat_script = 0;
      }
      else if(getting_string == EXEC_PGM_CMD) {  // script exec timer
         exec_time = exec_date = 0;
         exec_jd = 0.0;
         exec_timer = exec_val = 0;
         repeat_exec = 0;
      }
      else if(getting_string == LOG_DUMP_CMD) { // log dump timer
         log_time = log_date = 0;
         log_jd = 0.0;
         log_timer = log_val = 0;
         repeat_log = 0;
      }
      return;
   }

   strupr(arg);

   if(getting_string == SCREEN_DUMP_CMD) {
      if(strstr(arg, "O")) single_dump = 1;     // dump the screen to one file
      else                 single_dump = 0;     // dump the screen to multiple files
   }
   else if(getting_string == EXIT_CMD) {
      if(strstr(arg, "O")) single_exit = 1;     // exit program once
      else                 single_exit = 0;     // exit program peridoically
   }
   else if(getting_string == LOG_DUMP_CMD) {
      if(strstr(arg, "O")) single_log = 1;      // dump the log to one file
      else                 single_log = 0;      // dump the log multiple files
   }
   else if(getting_string == SCRIPT_RUN_CMD) {
      if(strstr(arg, "O")) single_script = 1;   // run script once
      else                 single_script = 0;   // run script multiple files
   }
   else if(getting_string == EXEC_PGM_CMD) {
      if(strstr(arg, "O")) single_exec = 1;     // run program once
      else                 single_exec = 0;     // run program multiple files
   }
   else {
      if(strstr(arg, "O")) single_alarm = 1;    // sound the alarm tone once
      else                 single_alarm = 0;    // sound the alarm until a key press

      if(strstr(arg, "A")) {
         modem_alarms = 1;    // use modem control lines to signal alarms
         reset_alarm();
      }
      else {
         modem_alarms = 0;    // dont use modem control lines
      }
   }

   // see if a countdown timer has been set
   timer_set = 0;
   val = (long) atosecs(arg);
   if(strstr(arg, "S")) {      // timer is in seconds
      timer_set = 'S';
   }
   else if(strstr(arg, "M")) { // timer is in minutes
      timer_set = 'M';
   }
   else if(strstr(arg, "H")) { // timer is in hours
      timer_set = 'H';
   }
   else if(strstr(arg, "D")) { // timer is in days
      timer_set = 'D';
   }

   if(strstr(arg, "W")) { // wait for alarm to trigger in script file
      alarm_wait = 1;
   }
   else {
      alarm_wait = 0;
   }


   if(timer_set) {  // a countdown timer has been set, is it a repeating timer?
      if(getting_string == ALARM_CMD) { // it is the egg timer
         egg_val = egg_timer = val;
         if(strstr(arg, "R")) repeat_egg = 1;
         else                 repeat_egg = 0;
      }
      else if(getting_string == EXIT_CMD) {  // it is the exit timer
         exit_val = exit_timer = val;
         if(strstr(arg, "R")) repeat_exit = 1;
         else                 repeat_exit = 0;
      }
      else if(getting_string == SCREEN_DUMP_CMD) {  // it is the screen dump timer
         dump_val = dump_timer = val;
         if(strstr(arg, "R")) repeat_dump = 1;
         else                 repeat_dump = 0;
      }
      else if(getting_string == SCRIPT_RUN_CMD) {  // it is the screen dump timer
         script_val = script_timer = val;
         if(strstr(arg, "R")) repeat_script = 1;
         else                 repeat_script = 0;
      }
      else if(getting_string == EXEC_PGM_CMD) {  // it is the screen dump timer
         exec_val = exec_timer = val;
         if(strstr(arg, "R")) repeat_exec = 1;
         else                 repeat_exec = 0;
      }
      else if(getting_string == LOG_DUMP_CMD) {  // it is the log dump timer
         log_val = log_timer = val;
         if(strstr(arg, "R")) repeat_log = 1;
         else                 repeat_log = 0;
      }

      return;
   }

   // setting an event time and/or date value
   if(err_ok) {  // processing keyboard time string, clear the current setting
      if(getting_string == SCREEN_DUMP_CMD) {
         dump_time = dump_date = 0;
         dump_hh = dump_mm = dump_ss = 0;
         dump_month = dump_day = dump_year = 0;
         dump_jd = 0.0;
      }
      else if(getting_string == LOG_DUMP_CMD) {
         log_time = log_date = 0;
         log_hh = log_mm = log_ss = 0;
         log_month = log_day = log_year = 0;
         log_jd = 0.0;
      }
      else if(getting_string == SCRIPT_RUN_CMD) {
         script_time = script_date = 0;
         script_hh = script_mm = script_ss = 0;
         script_month = script_day = script_year = 0;
         script_jd = 0.0;
      }
      else if(getting_string == EXEC_PGM_CMD) {
         exec_time = exec_date = 0;
         exec_hh = exec_mm = exec_ss = 0;
         exec_month = exec_day = exec_year = 0;
         exec_jd = 0.0;
      }
      else if(getting_string == ALARM_CMD) {
         alarm_time = alarm_date = 0;
         alarm_hh = alarm_mm = alarm_ss = 0;
         alarm_month = alarm_day = alarm_year = 0;
         alarm_jd = 0.0;
      }
      else if(getting_string == EXIT_CMD) {
         end_time = end_date = 0;
         end_hh = end_mm = end_ss = 0;
         end_month = end_day = end_year = 0;
         exit_jd = 0.0;
      }
   }

   aa = bb = cc = dd = ee = ff = 0;
   s1 = s2 = s3 = s4 = 0;
   x1[0] = 0;
   for(i=0; i<strlen(arg); i++) {  // get first param
      c = arg[i];
      if(c == ',') c = ' ';
      if(c == 0) break;

      if(c == ':') ;
      else if(c == '/') ;
      else if(isdigit(c) == 0) {
         ++i;
         break;
      }
      x1[i] = c;
      x1[i+1] = 0;
   }

   arg = trim_whitespace(&arg[i]);
   if(arg == 0) arg = "";
   x2[0] = 0;
   j = 0;
   for(j=0; j<strlen(arg); j++) {  // get second param
      c = arg[j];
      if(c == ',') c = ' ';
      if(c == 0) break;

      if(c == ':') ;
      else if(c == '/') ;
      else if(isdigit(c) == 0) {
         ++i;
         break;
      }
      x2[j] = c;
      x2[j+1] = 0;
   }

   sscanf(x1, "%d%c%d%c%d", &aa,&s1,&bb,&s2,&cc);  // decode param strings
   sscanf(x2, "%d%c%d%c%d", &dd,&s3,&ee,&s4,&ff);

   if(s1 == ':') { // first field is a time
      if(0 && status_second(cc)) {     // seconds field would occur within a long status message
         cc = status_second(cc);
      }

      if(s2 && (s2 != s1)) {
        if(err_ok) {
           edit_error("Invalid time separator character 2");
           return;
        }
      }
      else if(s3 && (s3 != '/')) {
         if(err_ok) {
            edit_error("Invalid date separator character 1");
            return;
         }
      }
      else if(s4 && (s4 != '/')) {
         if(err_ok) {
            edit_error("Invalid date separator character 2");
            return;
         }
      }
      else if(getting_string == ALARM_CMD) {  // alarm time
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
      else if(getting_string == SCREEN_DUMP_CMD) {  // screen dump time
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
      else if(getting_string == LOG_DUMP_CMD) {  // log dump time
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
      else if(getting_string == SCRIPT_RUN_CMD) {  // run script time
         script_time = 1;
         script_hh = aa;
         script_mm = bb;
         script_ss = cc;
         if(s3 == '/') {  // second field is a date
            script_date = 1;
            if(dd >= 2000) {
               script_day = ff;
               script_month = ee;
               script_year = dd;
            }
            else {
               script_month = dd;
               script_day = ee;
               if(ff < 100) ff += 2000;
               script_year = ff;
            }
         }
      }
      else if(getting_string == EXEC_PGM_CMD) {  // run program time
         exec_time = 1;
         exec_hh = aa;
         exec_mm = bb;
         exec_ss = cc;
         if(s3 == '/') {  // second field is a date
            exec_date = 1;
            if(dd >= 2000) {
               exec_day = ff;
               exec_month = ee;
               exec_year = dd;
            }
            else {
               exec_month = dd;
               exec_day = ee;
               if(ff < 100) ff += 2000;
               exec_year = ff;
            }
         }
      }
      else if(getting_string == EXIT_CMD) { // exit time
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
      if(0 && status_second(ff)) {     // seconds field would occur within a long status message
         ff = status_second(ff);
      }

      if(s2 && (s2 != s1)) {
         if(err_ok) {
            edit_error("Invalid date separator character 2");
            return;
         }
      }
      else if(s3 && (s3 != ':')) {
         if(err_ok) {
            edit_error("Invalid time separator character 1");
            return;
         }
      }
      else if(s4 && (s4 != ':')) {
         if(err_ok) {
            edit_error("Invalid time separator character 2");
            return;
         }
      }
      else if(getting_string == ALARM_CMD) {  // alarm date
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
      else if(getting_string == SCREEN_DUMP_CMD) {  // screen dump date
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
      else if(getting_string == LOG_DUMP_CMD) {  // log dump date
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
      else if(getting_string == SCRIPT_RUN_CMD) {  // script run date
         script_date = 1;
         if(aa >= 2000) {
            script_day = cc;
            script_month = bb;
            script_year = aa;
         }
         else {
            script_month = aa;
            script_day = bb;
            if(cc < 100) cc += 2000;
            script_year = cc;
         }

         if(s3 == ':') {  // second field is a time
            script_time = 1;
            script_hh = dd;
            script_mm = ee;
            script_ss = ff;
         }
      }
      else if(getting_string == EXEC_PGM_CMD) {  // program run date
         exec_date = 1;
         if(aa >= 2000) {
            exec_day = cc;
            exec_month = bb;
            exec_year = aa;
         }
         else {
            exec_month = aa;
            exec_day = bb;
            if(cc < 100) cc += 2000;
            exec_year = cc;
         }

         if(s3 == ':') {  // second field is a time
            exec_time = 1;
            exec_hh = dd;
            exec_mm = ee;
            exec_ss = ff;
         }
      }
      else if(getting_string == EXIT_CMD) { // exit date
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
      if(err_ok) {
         edit_error("No time or date specifier character seen");
      }
      return;
   }


   // set the event trigger Julian date and/or time
   if(getting_string == ALARM_CMD) {
      if(alarm_date && alarm_time) alarm_jd = jdate(alarm_year,alarm_month,alarm_day) + jtime(alarm_hh,alarm_mm,alarm_ss,0.0);
      else if(alarm_date) alarm_jd = jdate(alarm_year,alarm_month,alarm_day);
      else if(alarm_time) {
         alarm_jd = jtime(alarm_hh,alarm_mm,alarm_ss,0.0);
      }
   }
   else if(getting_string == SCREEN_DUMP_CMD) {
      if(dump_date && dump_time) dump_jd = jdate(dump_year,dump_month,dump_day) + jtime(dump_hh,dump_mm,dump_ss,0.0);
      else if(dump_date) dump_jd = jdate(dump_year,dump_month,dump_day);
      else if(dump_time) {
         dump_jd = jtime(dump_hh,dump_mm,dump_ss,0.0);
      }
   }
   else if(getting_string == LOG_DUMP_CMD) {
      if(log_date && log_time) log_jd = jdate(log_year,log_month,log_day) + jtime(log_hh,log_mm,log_ss,0.0);
      else if(log_date) log_jd = jdate(log_year,log_month,log_day);
      else if(log_time) {
         log_jd = jtime(log_hh,log_mm,log_ss,0.0);
      }
   }
   else if(getting_string == SCRIPT_RUN_CMD) {
      if(script_date && script_time) script_jd = jdate(script_year,script_month,script_day) + jtime(script_hh,script_mm,script_ss,0.0);
      else if(script_date) script_jd = jdate(script_year,script_month,script_day);
      else if(script_time) {
         script_jd = jtime(script_hh,script_mm,script_ss,0.0);
      }
   }
   else if(getting_string == EXEC_PGM_CMD) {
      if(exec_date && exec_time) exec_jd = jdate(exec_year,exec_month,exec_day) + jtime(exec_hh,exec_mm,exec_ss,0.0);
      else if(exec_date) exec_jd = jdate(exec_year,exec_month,exec_day);
      else if(exec_time) {
         exec_jd = jtime(exec_hh,exec_mm,exec_ss,0.0);
      }
   }
   else if(getting_string == EXIT_CMD) {
      if(end_date && end_time) exit_jd = jdate(end_year,end_month,end_day) + jtime(end_hh,end_mm,end_ss,0.0);
      else if(end_date) exit_jd = jdate(end_year,end_month,end_day);
      else if(end_time) {
         exit_jd = jtime(end_hh,end_mm,end_ss,0.0);
      }
   }
}


void set_alarm(char *alarm)
{
   // set the alarm clock to the values in string "alarm"
   // (with error checking disabled)

   if(alarm == 0) return;

   getting_string = ALARM_CMD;
   edit_dt(alarm, 0);
   getting_string = 0;
}


double parse_alt(char *s)
{
double val;
int adj_flag;

   // convert an altitude string to meters. The string can end in an f or '
   // to indicate feet.

   if(s == 0) return 0.0;

   strlwr(s);
   if(s[0] == '*') {
      adj_flag = 1;
      s = "0";
   }
   else if((s[0] == '+') && (s[1] == '+')) {
      adj_flag = 1;
      ++s;
   }
   else if((s[0] == '-') && (s[1] == '-')) {
      adj_flag = (-1);
      ++s;
   }
   else adj_flag = 0;

   if(strchr(s, 'a') && plot[THREE].stat_count && have_lla_queue) {
      val = plot[THREE].sum_y / plot[THREE].stat_count;
   }
   else if(strchr(s, 'a')) {
      val = alt;
   }
   else {
      val = atof(s);
   }
   if(adj_flag) {
      val += precise_alt;
   }

   if(strchr(s, 'f') || strchr(s, '\'')) {  // convert feet to meters
      val /= FEET_PER_METER;
   }
   return val;
}

double hms_val(char *s)
{
double val;
unsigned i, j, len;
double sign;

   if(s == 0) return 0.0;

   strcpy(out, s);

   val = 0.0;
   sign = 1.0;

   j = 0;
   len = strlen(out);
   for(i=0; i<len; i++) {
      if(out[i] == '-') sign *= (-1.0);

      if(toupper(out[i]) == 'W') {  // weeks
         out[i] = 0;
         val += fabs(atof(&out[j]) * 7.0*24.0*60.0*60.0);
         j = i+1;
      }
      else if(toupper(out[i]) == 'D') { // days
         out[i] = 0;
         val += fabs(atof(&out[j]) * 24.0*60.0*60.0);
         j = i+1;
      }
      else if(toupper(out[i]) == 'H') { // hours
         out[i] = 0;
         val += fabs(atof(&out[j]) * 60.0*60.0);
         j = i+1;
      }
      else if(toupper(out[i]) == 'M') { // minutes
         out[i] = 0;
         val += fabs(atof(&out[j]) * 60.0);
         j = i+1;
      }
      else if(toupper(out[i]) == 'S') { // seconds
         out[i] = 0;
         val += fabs(atof(&out[j]) * 1.0);
         j = i+1;
      }
   }

   return (val*sign);
}

double deg_min_sec(char *s)
{
double val;
unsigned i, j, len;
double sign;

   if(s == 0) return 0.0;

   strcpy(out, s);

   val = 0.0;
   sign = (1.0);

   j = 0;
   len = strlen(out);
   for(i=0; i<len; i++) {
      if(out[i] == '-') sign *= (-1.0);

      if(toupper(out[i]) == 'D') { // degrees
         out[i] = 0;
         val += fabs(atof(&out[j]) * 60.0*60.0);
         j = i+1;
      }
      else if(toupper(out[i]) == 'M') { // minutes
         out[i] = 0;
         val += fabs(atof(&out[j]) * 60.0);
         j = i+1;
      }
      else if(toupper(out[i]) == 'S') { // seconds
         out[i] = 0;
         val += fabs(atof(&out[j]) * 1.0);
         j = i+1;
      }
   }

   return (val / 3600.0 * sign);
}

double parse_coord(int lon_flag, char *s)
{
double val;
int adj_flag;

   // convert a lat/lon string to decimal degrees.  The string can either be
   // a decimal number or a string in the format 11d12m30s (no embeded spaces).
   // If a part of the dms format string is not given, it is assumed to be 0.

   if(s == 0) return 0.0;

   strlwr(s);
   if(s[0] == '*') {
      adj_flag = 1;
      s = "0";
   }
   else if((s[0] == '+') && (s[1] == '+')) {
      adj_flag = 1;
      ++s;
   }
   else if((s[0] == '-') && (s[1] == '-')) {
      adj_flag = (-1);
      ++s;
   }
   else adj_flag = 0;

   if( (strchr(s, 'a') || strchr(s, 'A')) && have_lla_queue && plot[ONE].stat_count && plot[TWO].stat_count) {
      precise_lat = plot[ONE].sum_y / plot[ONE].stat_count;
      if(lon_flag) val = plot[TWO].sum_y / plot[TWO].stat_count;
      else         val = plot[ONE].sum_y / plot[ONE].stat_count;
   }
   else if(strchr(s, 'a') || strchr(s, 'A')) {
      if(lon_flag) val = lon * 180.0 / PI;
      else         val = lat * 180.0 / PI;
   }
   else if(strchr(s, 'd') || strchr(s, 'm') || strchr(s, 's')) {  // convert feet to meters
      val = deg_min_sec(s);
   }
   else if(strchr(s, 'D') || strchr(s, 'M') || strchr(s, 'S')) {  // convert feet to meters
      val = deg_min_sec(s);
   }
   else {
      val = atof(s);
   }

   if(adj_flag) {  // add +/- offset in feet or meters to current position
      if(lon_flag) {
         if(cos_factor) val /= cos_factor;
         val = precise_lon + (val * ANGLE_SCALE * DEG_TO_RAD);
      }
      else {
         val = precise_lat + (val * ANGLE_SCALE * DEG_TO_RAD);
      }
      val = val * 180.0 / PI;
   }

   return val;
}

int parse_lla(char *s)
{
double old_lat, old_lon, old_alt;
double x_lat, x_lon, x_alt;
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

//   precise_lat = precise_lon = precise_alt = 1.0E6;
   if(strchr(s, 'A') || strchr(s, 'a')) {  // set lat/lon/alt to current average value
      if(have_lla_queue == 0) return NO_AVERAGE;
      if(plot[ONE].stat_count == (DATA_SIZE) 0) return NO_AVERAGE;
      if(plot[TWO].stat_count == (DATA_SIZE) 0) return NO_AVERAGE;
      if(plot[THREE].stat_count == (DATA_SIZE) 0) return NO_AVERAGE;
   }

   strcpy(lat_s, s);
   strupr(lat_s);
   if(strstr(lat_s, "LOC")) {
      x_lat = lat;
      x_lon = lon;
      x_alt = alt;
      n = read_default_lla();  // get location from heather.loc file
      lat = x_lat;
      lon = x_lon;
      alt = x_alt;
      if(n != LLA_OK) goto bad_loc;

      sprintf(lat_s, "%.9f", precise_lat*180.0/PI);
      sprintf(lon_s, "%.9f", precise_lon*180.0/PI);
      sprintf(alt_s, "%.9f", precise_alt);
   }
   else {
      n = sscanf(s, "%s %s %s", &lat_s[0],&lon_s[0],&alt_s[0]);
      if(n != 3) {
         bad_loc:
         precise_lat = old_lat;
         precise_lon = old_lon;
         precise_alt = old_alt;
         return LLA_MISSING;
      }
   }

   precise_lat = parse_coord(0, lat_s);
   precise_lon = parse_coord(1, lon_s);
   precise_alt = parse_alt(alt_s);
//sprintf(plot_title, "lat:%.9f lon:%.9f alt:%.9f", precise_lat,precise_lon,precise_alt);

   if((precise_lat < -90.0) || (precise_lat > 90.0)) {
      precise_lat = old_lat;
      precise_lon = old_lon;
      precise_alt = old_alt;
      return BAD_LAT;
   }
   else if((precise_lon < -180.0) || (precise_lon > 180.0)) {
      precise_lat = old_lat;
      precise_lon = old_lon;
      precise_alt = old_alt;
      return BAD_LON;
   }
   else if((precise_alt< -1000.0) || (precise_alt > 20000.0)) {
      precise_lat = old_lat;
      precise_lon = old_lon;
      precise_alt = old_alt;
      return BAD_ALT;
   }

   precise_lat /= RAD_TO_DEG;
   precise_lon /= RAD_TO_DEG;
   have_precise_lla = (-5);

   return LLA_OK;
}




struct STATION {
   char *id;
   char *lat;
   char *lon;
} station[] =
{
   { 0,0,0 },
   { "ANTHORN", "54d55m00s",  "-3d15m0s" },
   { "BBC",     "51.518409",  "-0.143591" },
   { "BPM",     "35d00m00s",  "-109d31m00s" },
   { "CHU",     "45d17m47s",  "-75d45m22s" },
   { "DCF",     "50d01m00s",  "9d00m0s" },
   { "DROIT",   "51.518409",  "-0.143591" },
   { "EBC",     "36d28m00s",  "-6d12m00s" },
   { "HBG",     "46d24m00s",  "6d15m0s" },
   { "HLA",     "36d23m15s",  "127d21m59s" },
   { "JJY",     "37d22m00s",  "140d51m00s" },
   { "KYUSHU",  "33d28m00s",  "130d11m00s" },
   { "LOL",     "34d37m00s",  "58d21m00s" },
   { "MIKES",   "60d11m00s",  "24d49m59s" },
   { "MSF",     "52d22m00s",  "-1d11m0s" },
   { "PPE",     "-22d53m45s", "-43d13m28s" },
   { "RBU",     "55d55m00s",  "38d12m0s" },
   { "RTZ",     "52d26m00s",  "103d41m0s" },
   { "RUGBY",   "52d22m00s",  "-1d11m0s" },
   { "RWM",     "56d44m01s",  "37d38m11s" },
   { "TDF",     "47d10m00s",  "2d12m00s" },
   { "WWV",     "40d40m49s",  "-105d02m27s" },
   { "WWVB",    "40d40m49s",  "-105d02m27s" },
   { "WWVH",    "21d59m26s",  "-159d46m00s" },
   { "YVTO",    "10d30m00s",  "-66d56m00s" },
   { 0,0,0 }
};

struct STATION find_station(char *s)
{
int i;

   if(s == 0) return station[0];

   i = 0;
   while(1) {
      ++i;
      if(station[i].id == 0) return station[0];
      if(strstr(station[i].id, s)) return station[i];
   }
   return station[0];
}


double iono[] = {
   0.0,
   250.0, 250.0, 275.0,  // jan feb mar
   300.0, 325.0, 350.0,  // apr may jun
   350.0, 350.0, 325.0,  // jul aug sep
   300.0, 275.0, 250.0   // oct nov dec
};

double iono_height()
{
int ndx;

   if(pri_month < 1) return 300.0;
   if(pri_month > 12) return 300.0;

   ndx = pri_month;
   if(lat < 0.0) {
      ndx = ndx + 6;
      if(ndx > 12) ndx = ndx - 12;
   }

   return iono[ndx];
}


int calc_prop_delay(char *s)
{
double old_lat, old_lon, old_alt;
struct STATION station;
char lat_s[SLEN+1];
char lon_s[SLEN+1];
char alt_s[SLEN+1];
int n;
char *comma;
int hops;
double dist;
double az;
double delay;
double x_lat,x_lon,x_alt;

   // set lat/lon/altitude
   if(s == 0) return (-1);
   strupr(s);

   old_lat = prop_lat;
   old_lon = prop_lon;
   old_alt = prop_alt;

   comma = s;
   while(comma) {  // convert commas to spaces
      comma = strchr(comma, ',');
      if(comma) *comma = ' ';
      else break;
   }

   if(strstr(s, "LOC")) {
      x_lat = lat;
      x_lon = lon;
      x_alt = alt;
      n = read_default_lla();  // get location from heather.loc file
      lat = x_lat;
      lon = x_lon;
      alt = x_alt;
      if(n != LLA_OK) goto bad_loc;

      sprintf(lat_s, "%.9f", precise_lat*180.0/PI);
      sprintf(lon_s, "%.9f", precise_lon*180.0/PI);
      sprintf(alt_s, "%.9f", 0.0);
      n = 3;
   }
   else {
      n = sscanf(s, "%s %s %s", &lat_s[0],&lon_s[0],&alt_s[0]);
      if(n == 1) {  // iono height not given, default it
         sprintf(lon_s, "%g", iono_height());
         ++n;
      }
   }

   dist = 0.0;
   if(n == 2) {  // location is a time station name, distance, or lat/lon
      strcpy(alt_s, lon_s);
      station = find_station(lat_s);
      if(station.id && station.lat && station.lon) {  // station id given as location
         strcpy(lat_s, station.lat);
         strcpy(lon_s, station.lon);
      }
      else if(strchr(lat_s, 'K')) {  // distance in km
         dist = atof(lat_s);
         strcpy(lat_s, "0.0");
         strcpy(lon_s, "0.0");
      }
      else {  // lat/lon given, fill in ionosphere height
         sprintf(alt_s, "%g", iono_height());
         ++n;
      }
   }
   else if(n != 3) {
      bad_loc:
      prop_lat = old_lat;
      prop_lon = old_lon;
      prop_alt = old_alt;
      return LLA_MISSING;
   }

   prop_lat = parse_coord(0, lat_s);
   prop_lon = parse_coord(1, lon_s);
   prop_alt = parse_alt(alt_s);
   if(prop_alt == 0.0) prop_alt = 0.001;

   if((prop_lat < -90.0) || (prop_lat > 90.0)) {
      prop_lat = old_lat;
      prop_lon = old_lon;
      prop_alt = old_alt;
      return BAD_LAT;
   }
   else if((prop_lon < -180.0) || (prop_lon > 180.0)) {
      prop_lat = old_lat;
      prop_lon = old_lon;
      prop_alt = old_alt;
      return BAD_LON;
   }
   else if((prop_alt < 0.0) || (prop_alt > 1.0E6)) {
      prop_lat = old_lat;
      prop_lon = old_lon;
      prop_alt = old_alt;
      return BAD_ALT;
   }

   prop_lat /= RAD_TO_DEG;
   prop_lon /= RAD_TO_DEG;
   have_prop_lla = (-5);

   hops = find_delay(lat,lon, prop_lat,prop_lon,prop_alt, dist, &dist,&delay);
   az = az_angle(lat,lon, prop_lat,prop_lon);

   if(dist < 1.0) {  // less than 1km away, show meters/feet
      sprintf(debug_text, "%.3f m (%.2f feet)  az=%.3f deg   prop delay %.6f seconds  ionosphere:%.0f km  hops:%d",
          dist*1000.0, dist/1.609344*5280.0, az, delay, prop_alt, hops);
   }
   else {  // show km/miles
      sprintf(debug_text, "%.3f km (%.3f miles)  az=%.3f deg   prop delay %.6f seconds  ionosphere:%.0f km  hops:%d",
          dist, dist/1.609344, az, delay, prop_alt, hops);
   }

   return LLA_OK;
}


void edit_lla_ref_cmd()
{
int err;
double old_lat, old_lon, old_alt;

   old_lat = precise_lat;
   old_lon = precise_lon;
   old_alt = precise_alt;

   precise_lat = ref_lat;
   precise_lon = ref_lon;
   precise_alt = ref_alt;

   err = parse_lla(&edit_buffer[0]);
   if     (err == BAD_LAT)     edit_error("Bad latitude value");
   else if(err == BAD_LON)     edit_error("Bad longitude value");
   else if(err == BAD_ALT)     edit_error("Bad altitude value");
   else if(err == LLA_MISSING) edit_error("Must enter three lat lon alt values (separated by spaces)");
   else if(err == NO_AVERAGE)  edit_error("No LLA data available to average");
   else if(err) ;
   else { // save current position with high accuracy
      ref_lat = precise_lat;
      ref_lon = precise_lon;
      ref_alt = precise_alt;
      user_set_ref_lla = 1;

      rebuild_lla_plot(0);
      need_redraw = 3201;
   }

   precise_lat = old_lat;
   precise_lon = old_lon;
   precise_alt = old_alt;
}

void edit_lla_cmd(int type)
{
int err;

   err = parse_lla(&edit_buffer[0]);
   if     (err == BAD_LAT)     edit_error("Bad latitude value");
   else if(err == BAD_LON)     edit_error("Bad longitude value");
   else if(err == BAD_ALT)     edit_error("Bad altitude value");
   else if(err == LLA_MISSING) edit_error("Must enter three lat lon alt values (separated by spaces)");
   else if(err == NO_AVERAGE)  edit_error("No LLA data available to average");
   else if(err) return;
   else { // save current position with high accuracy
      // !!!!! should we set ref_lat/lon/alt? gggggggg
      if(luxor) {
         set_lla(precise_lat, precise_lon, precise_alt);
      }
      else {
         #ifdef PRECISE_STUFF
            stop_precision_survey(10); // stop any precision survey
            stop_self_survey(10);      // stop any self survey
            save_precise_posn(type);   // do single point surveys until we get close enough
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
int old_width;

   // set lat/lon/altitude
   if(edit_buffer[0] == 0) return;

   freq = 1.0;
   duty = 0.50;
   if     (rcvr_type == NVS_RCVR)   sscanf(edit_buffer, "%lf", &duty);
   else if(rcvr_type == CS_RCVR)    sscanf(edit_buffer, "%lf", &freq);
   else if((rcvr_type == ESIP_RCVR) && (chan == 0)) sscanf(edit_buffer, "%lf", &duty);
   else if((rcvr_type == SS_RCVR) && (chan == 0))   sscanf(edit_buffer, "%lf", &duty);
   else if(rcvr_type == TM4_RCVR)   sscanf(edit_buffer, "%lf", &freq);
   else if(rcvr_type == ZYFER_RCVR) sscanf(edit_buffer, "%lf", &duty);
   else sscanf(edit_buffer, "%lf %lf", &freq, &duty);


   if(rcvr_type == CS_RCVR) {
   }
   else if((rcvr_type == ESIP_RCVR) && (chan == 0)) { // PPS pulse width
      if((duty < 1.0) || (duty > 500.0)) {
         edit_error("PPS width must be 1..500 msecs");
      }
      else {
         old_width = esip_pps_width;
         esip_pps_width = (int) duty;
         set_pps(user_pps_enable, pps_polarity,  cable_delay, pps1_delay,  pps_threshold, 4);
         esip_pps_width = old_width;
         request_pps_info();
      }
      return;
   }
   else if(rcvr_type == SS_RCVR) {
      old_width = (int) ss_pps_width;
      ss_pps_width = (int) (duty*1000.0);
      set_pps(user_pps_enable, pps_polarity,  cable_delay, pps1_delay,  pps_threshold, 4);
      ss_pps_width = (double) old_width;
      request_pps_info();
   }
   else if(rcvr_type == TM4_RCVR) {
   }
   else if(rcvr_type == ZYFER_RCVR) {
   }
   else if((chan == 1) && ((rcvr_type == NVS_RCVR) || ((rcvr_type == VENUS_RCVR) && !saw_timing_msg)) ) {
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
   else if(duty >= 1.0) {  // duty is a pulse width in microseconds (or nanoseconds)
      if(rcvr_type == ESIP_RCVR) duty = freq * (duty/1.0E9);  // convert nanoeconds to duty cycle
      else if(rcvr_type == NVS_RCVR) duty = freq * (duty/1.0E9);  // convert nanoeconds to duty cycle
      else duty = freq * (duty/1.0E6);  // convert microseconds to duty cycle
      if(duty >= 1.0) {
         edit_error("Pulse width too long for specified frequency");
         return;
      }
   }

   if(chan == 0) set_pps_freq(chan, freq, duty, pps_polarity, cable_delay, pps1_delay, 1);
   else          set_pps_freq(chan, freq, duty, 0,            cable_delay, pps2_delay, 1); // rising edge gggg
}

void set_not_safe()
{
   if((rcvr_type == TICC_RCVR) && (keyboard_cmd == 0)) not_safe = 3;  // keeps TICC_RCVR from being reset
   else if(POLLED_RCVR && keyboard_cmd) not_safe = 3;  // don't mess with polling process by doing an init_messages()
   else not_safe = 1;
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
         sprintf(out, "Unknown or bad command line option: %s.  Type ? for command line help.", &cmd[0]);
         edit_error(out);
         break;
      }
//    else if(not_safe == 1) {
      else if(not_safe) {      // xyzzy
         config_options();
      }
   }

   need_redraw = 1200;
}

void set_osc_units()
{
   if(rcvr_type == BRANDY_RCVR) ;
   else if(rcvr_type == PRS_RCVR) ;
   else if(rcvr_type == TSIP_RCVR) ;
   else return;

   if(show_euro_ppt) {
      ppb_string = "e-9 ";
      ppt_string = "e-12";
      if(strstr(plot[OSC].units, "ppt")) plot[OSC].units = "e-12";
      if(rcvr_type == BRANDY_RCVR) plot[SEVEN].units = "e-12";
   }
   else {
      ppb_string = " ppb";
      ppt_string = " ppt";
      if(strstr(plot[OSC].units, "e-12")) plot[OSC].units = "ppt";
      if(rcvr_type == BRANDY_RCVR) plot[SEVEN].units = "ppt";
   }
   plot[OSC].units = ppt_string;
}


void edit_option_value()
{
float val;
int i;
u08 c;

   // set a debug/test value
   val = (float) atof(&edit_buffer[1]);
   c = tolower(edit_buffer[0]);
   if(c == 'a') { // OA command - toggle AMU / dBc mode
      amu_flag ^= 0x08;
      if(SV6_FAMILY || ACE3 || PALISADE) {
         if(log_db == 0) {
            request_sig_levels();
         }
      }
      else {
//       set_io_options(0x13, 0x03, 0x01, amu_flag | BROADCAST_5A | DOPPLER_SMOOTH);  // ECEF+LLA+DBL PRECISION, ECEF+ENU vel,  UTC, no PACKET 5A, amu
         set_io_options(0x32, 0x02, 0x03, amu_flag | BROADCAST_5A | DOPPLER_SMOOTH);  // ECEF+LLA+DBL PRECISION, ECEF+ENU vel,  UTC, PACKET 5A, amu
      }
      request_io_options();
//    clear_signals();
   }
#ifdef ADEV_STUFF
   else if(c == 'b') {  // OB command - adev bin scale
      bin_scale = (int) val;
      if(bin_scale <= 0) bin_scale = 5;
      recalc_adev_info();
      force_adev_redraw(100);
   }
#endif
   else if(c == 'c') { // OC command - toggle continuous scrolling mode
      if(0) { // --- sat count plot scale
         if(edit_buffer[1]) small_sat_count = (int) val & 0x01;
         else               small_sat_count ^= 1;
         config_screen(100);
      }
      else {  // continuous plot scrolling mode
         if(edit_buffer[1]) continuous_scroll = (int) val & 0x01;
         else               continuous_scroll ^= 1;
         config_screen(9376);
      }
   }
#ifdef FFT_STUFF
   else if(c == 'd') {  // OD command - do FFT in dB
      if(edit_buffer[1]) fft_db = (int) val & 0x01;
      else               fft_db ^= 1;
      set_fft_scale();
      if((live_fft == 0) && (selected_plot != FFT)) {
         plot[FFT].show_plot = 1;
         calc_fft(selected_plot);
         if(fft_type == HIST_TYPE) dump_hist_plot();
         else                      dump_fft_plot();
      }
   }
#endif
   else if(c == 'e') {  // OE command - force Thunderbolt-E flag
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
   else if(c == 'f') {  // OF command - keep adevs fresh (by periodically forcing a recalc)
      if(edit_buffer[1]) keep_adevs_fresh = (int) val & 0x01;
      else               keep_adevs_fresh = 1;
   }
#endif
   else if(c == 'g') {  // OG command - solid earth tide options
      if(edit_buffer[1]) tide_options = (int) val;
      else               tide_options ^= 1;   // toggle low res mode
   }
   else if(c == 'h') {  // OH command - erase lla plot every hour
      if(edit_buffer[1]) erase_every_hour = (int) val & 0x01;
      else               erase_every_hour ^= 1;
   }
   else if(c == 'i') {  // OI command - signal level displays
      if(edit_buffer[1])    plot_signals = (int) val;
      else if(plot_signals) plot_signals = 0;
      else                  plot_signals = 4;

      if(zoom_screen) {
         change_zoom_config(-102);
         cancel_zoom(12);      //zkzk
      }
      zoom_fixes = show_fixes;
      if(strchr(edit_buffer, 'z')) {
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
   else if(c == 'j') {  // OJ command - just read and log serial data in hex
val = (float) atohex(&edit_buffer[1]);
      if(edit_buffer[1]) log_stream = (int) val & (LOG_HEX_STREAM | LOG_PACKET_ID | LOG_PACKET_START | LOG_SENT_DATA);
      else               log_stream ^= LOG_HEX_STREAM;
//    if(edit_buffer[1]) just_read = (int) val & 0x01;
//    else               just_read ^= 1;
//    log_stream = just_read;
   }
   else if(c == 'k') {  // OK command - flag message faults as time skips
      if(edit_buffer[1]) flag_faults = (int) val & 0x01;
      else               flag_faults ^= 1;
   }
#ifdef FFT_STUFF
   else if(c == 'l') {  // OL command - live fft
      if(edit_buffer[1]) show_live_fft = (int) val & 0x01;
      else               show_live_fft ^= 1;
      live_fft = selected_plot;
      if(show_live_fft && (plot[FFT].show_plot == 0)) toggle_plot(FFT);
   }
#endif
   else if(c == 'm') {  // OM command - plot magnification factor (for less than one sec/pixel)
      plot_mag = (int) val;
      if(plot_mag <= 1) plot_mag = 1;
   }
   else if(c == 'n') {  // ON command - dynamic trend line info display
      if(edit_buffer[1]) dynamic_trend_line = (int) val & 0x01;
      else               dynamic_trend_line ^= 1;
   }
   else if(c == 'p') {  // OP command - peak scale - dont let plot scale factors get smaller
      if(edit_buffer[1]) peak_scale = (int) val & 0x01;
      else               peak_scale ^= 1;
   }
   else if(c == 'q') {  // OQ command - skip over subsmapled queue entries using the slow, direct method
      if(edit_buffer[1]) slow_q_skips = (int) val & 0x01;
      else               slow_q_skips ^= 1;
   }
#ifdef ADEV_STUFF
   else if(c == 'r') {  // OR command - reset adev bins and recalc adevs
      recalc_adev_info();
   }
#endif
   else if(c == 's') {  // OS command - temperature spike filter 0=off  1=for temp pid  2=for pid and plot
      if(edit_buffer[1])  spike_mode = (int) val;
      else if(spike_mode) spike_mode = 0;
      else                spike_mode = 1;
   }
   else if(c == 't') {  // OT command - alarm/dump type based upon local or displayed time
      if(edit_buffer[1]) alarm_type = (int) val & 0x01;
      else               alarm_type ^= 1;
   }
   else if(c == 'u') {  // --- OU command - daylight savings time area
      if(edit_buffer[1]) dst_area = (int) val;
      else               dst_area = 0;
      if((dst_area < 0) || (dst_area > DST_AREAS)) dst_area = 1;
      calc_dst_times(this_year, dst_list[dst_area]);
      dst_ofs = dst_offset();
   }
   else if(c == 'v') {  // OV command - ADEV base value mode
      // (kludge used to increase dynamic range of single precision numbers)
      subtract_base_value = (u08) val;
      reset_queues(RESET_ALL_QUEUES, 1200);
   }
   else if(c == 'w') {  // --- OW command - watch face style
      watch_face = (u08) val;
      if(watch_face > 5) watch_face = 0;
   }
   else if(c == 'x') {  // OX command - set trend line rate display time scale
      i = (int) val;
      if(edit_buffer[1]) trend_rate_display = (i & 0x03);
      else               trend_rate_display = ((trend_rate_display & 0x01) ^ 0x01);
   }
   else if(c == 'y') {  // --- OY command - temp-dac dac scale factor
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
   else if(c == 'z') {  // OZ command - plot cursor time ref
      if(edit_buffer[1]) cursor_time_ref = (int) val & 0x03;
      else               cursor_time_ref ^= 1;
   }
   else if(c == '!') { // O! command -
   }
   else if((c == '/') || (c == '-') || (c == '$') || (c == '=') || isdigit(c)) {
      edit_option_switch();
   }
   else {
      // bugs_black_blood:
      sprintf(out, "Unknown option parameter: %c", c);
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


void dump_mtie_data()
{
FILE *file;

   if(edit_buffer[0] == 0) {
      file = topen("mtie.dat", "w");
      if(file == 0) {
         sprintf(out, "ERROR: could not write file: mtie.dat");
         edit_error(out);
         return;
      }
   }
   else {
      if(strstr(edit_buffer, ".") == 0) strcat(edit_buffer, ".dat");
      file = topen(edit_buffer, "w");
      if(file == 0) {
         sprintf(out, "ERROR: could not write file: %s", edit_buffer);
         edit_error(out);
         return;
      }
   }
   if(file == 0) return;

   dump_mtie(CHA_MTIE, file);
   dump_mtie(CHB_MTIE, file);
   dump_mtie(CHC_MTIE, file);
   dump_mtie(CHD_MTIE, file);


   fclose(file);
}

int edit_write_cmd()
{
int valid;
   // process a file write/delete command
   if(edit_buffer[0]) {
      if(dump_type == 's') {
         reset_first_key(21);
         prot_menu = 0;
         draw_plot(REFRESH_SCREEN);
         draw_maps();
         refresh_page();

         valid = 0;
         if     (strstr(edit_buffer, ".gif")) valid = 1;
         else if(strstr(edit_buffer, ".GIF")) valid = 1;
         else if(strstr(edit_buffer, ".bmp")) valid = 1;
         else if(strstr(edit_buffer, ".BMP")) valid = 1;
         if(valid == 0) {
            first_key = ' ';
            edit_error("Screen dump file type must be .gif or .bmp");
         }
         else if(dump_screen(invert_dump, top_line, edit_buffer) == 0) {
            first_key = ' ';
            edit_error("Could not write screen image file");
         }
      }
      else if(dump_type == 'c') {  // luxor cal .SCR file
         dump_cal_file();
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
      else if(dump_type == 'm') {  // MTIE data
         dump_mtie_data();
      }
      else {
         dump_log(edit_buffer, dump_type);
      }
   }
   else {
      edit_error("Missing file name");
   }

   top_line = 0;
   return 0;
}

void edit_osc_param()
{
DATA_SIZE val;

   // change an oscillator control parameter
   if(edit_buffer[0] == 0) return;
   val = (DATA_SIZE) atof(edit_buffer);

   if(getting_string == TC_CMD) {  // time constant
      val = (DATA_SIZE) fabs(val);
      user_time_constant = val;
      if(rcvr_type == PRS_RCVR) {
         set_prs_pt((int) val);
         return;
      }
      else if(rcvr_type == SRO_RCVR) {
         if((val < 0) || (val > 999999)) {
            sprintf(out, "Time constant must be 0 .. 999999");
            edit_error(out);
         }
         else {
            set_sro_tc((int) val);
         }
         return;
      }
      else if(rcvr_type == X72_RCVR) {
         if((val < (DATA_SIZE) MIN_X72_TC) || (val > (DATA_SIZE) MAX_X72_TC)) {
            sprintf(out, "Time constant must be %d .. %d", MIN_X72_TC, MAX_X72_TC);
            edit_error(out);
         }
         else {
            set_x72_tc((int) val);
         }
         return;
      }
   }
   else if(getting_string == DAMP_CMD) {  // damping factor
      val = (DATA_SIZE) fabs(val);
      user_damping_factor = val;
      if(rcvr_type == PRS_RCVR) {
         set_prs_pf((int) val);
         return;
      }
      else if(rcvr_type == X72_RCVR) {
         if((val < 0.25) || (val > 4.0)) {
            edit_error("Time constant must be 0.25 .. 4.0");
         }
         else {
            set_x72_damping((double) val);
         }
         return;
      }
   }
   else if(getting_string == GAIN_CMD) {  // osc gain
      user_osc_gain = val;
      lars_gain = val;
      if(ticc_type == LARS_TICC) osc_gain = val;
      if(com[RCVR_PORT].process_com == 0) osc_gain = val;
      if(luxor) set_luxor_sens((u08) val, (u08) val);
      else if(rcvr_type == PRS_RCVR) {
         set_prs_ga((int) val);
         return;
      }
   }
   else if(getting_string == INITV_CMD) {  // initial dac voltage
      user_initial_voltage = val;
      lars_initv = val;
      if(ticc_type == LARS_TICC) initial_voltage = val;
      if(saw_ntpx) initial_voltage = val;
      if(rcvr_type == PRS_RCVR) {
         set_prs_pi((int) val);
         return;
      }
   }
   else if(getting_string == JAMSYNC_CMD) {  // jam sync theshold
      val = (DATA_SIZE) fabs(val);
      user_jam_sync = val;
      if(rcvr_type == X72_RCVR) {
         if((val < 100.0) || (val > 1000.0)) {
            edit_error("Jamsync threshold time must be 100 .. 1000 ns");
         }
         else {
            set_x72_jamthresh((int) val);
         }
         return;
      }
   }
   else if(getting_string == MAXV_CMD) {  // max volts / luxor driver mode change
      user_max_volts = val;
   }
   else if(getting_string == MAX_FOSC_CMD) {  // max freq offset
      user_max_freq_offset = val;
   }
   else if(getting_string == MIN_RANGE_CMD) {  // min EFC range volts
      user_min_range = val;
   }
   else if(getting_string == MAX_RANGE_CMD) {  // max EFC range volts
      user_max_range = val;
   }
   else if(getting_string == MINV_CMD) {  // min volts / battery pwm level
      user_min_volts = val;
if(luxor) set_batt_pwm((u16) (val * (DATA_SIZE) 65535.0));
   }
   else if(getting_string == PULLIN_CMD) {
      user_pullin = (int) val;
      set_pullin_range(atoi(edit_buffer));
return;
   }
   else if(getting_string == X72_HOLDOVER_CMD) {
      val = (DATA_SIZE) fabs(val);
      if((val < 120) || (val > 1200)) {
         edit_error("Holdover analysis time must be 120 .. 1200");
      }
      else {
         user_holdover_time = (int) val;
         set_x72_holdover((int) val);
      }
return;
   }
   else return;   // unknown osc param

   set_discipline_params(1);
   request_all_dis_params();
}

double get_cable_val(char *s)
{
double val;

   if(s == 0) return 0.0F;

   while(s > &edit_buffer[0]) { // backup over leading white space
      --s;
      if((*s != ' ') && (*s != '\t')) break;
   }
   while(s > &edit_buffer[0]) { // backup to the value
      --s;
      if((*s == ' ') || (*s == '\t')) break;
   }
   if(s == 0) return 0.0;

   val = atof(s);
   return val;
}


void edit_cable_delay(int kbd_cmd)
{
double val;
double vf;
char *s, *sf, *squote;

   if(edit_buffer[0] == 0) return;

   vf = VELOCITY_FACTOR;  // 0.66 vp coax
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
      val *= FEET_PER_METER;
      val = (val / 983571056.0) / vf;
   }
   else if(sf) {
      val = get_cable_val(sf);
      val = (val / 983571056.0) / vf;
   }
   else if(squote) {
      val = get_cable_val(squote);
      val = (val / 983571056.0) / vf;
   }
   else {
      val = atof(edit_buffer);
      val /= 1.0E9;
   }

   delay_value = val;

   if(kbd_cmd) {
      set_pps(user_pps_enable, pps_polarity,  delay_value,  pps1_delay, pps_threshold, 1);
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
      D_TC = (DATA_SIZE) atof(s);
      calc_k_factors();
   }
   else if(c == 'F') {  // KF command - filter time constant
      FILTER_TC = (DATA_SIZE) atof(s);
      calc_k_factors();
   }
   else if(c == 'G') {  // KG command - scale factor (loop gain correction)
      k7 = (DATA_SIZE) atof(s);
      calc_k_factors();
   }
   else if(c == 'H') {  // KH command - PID debug (help) display
      pid_debug = toggle_value(pid_debug);
      if(pid_debug) osc_pid_debug = 0;
      if(kbd_cmd) clear_pid_display();
   }
   else if(c == 'I') {  // KI command - integral time constant
      I_TC = (DATA_SIZE) atof(s);
      calc_k_factors();
   }
   else if(c == 'K') {  // KK command - all major PID values
      if(kbd_cmd) enable_temp_control();
      do_temp_control = 1;
      FILTER_OFFSET = 0.0;

      #ifdef DOUBLE_DATA
         sscanf(s, "%lf%c%lf%c%lf%c%lf", &P_GAIN,&c, &D_TC,&c,  &FILTER_TC,&c, &I_TC);
      #else
         sscanf(s, "%f%c%f%c%f%c%f", &P_GAIN,&c, &D_TC,&c,  &FILTER_TC,&c, &I_TC);
      #endif

      calc_k_factors();
   }
   else if(c == 'L') {  // KL command - load distubance
      k6 = (DATA_SIZE) atof(s);
      calc_k_factors();
   }
   else if(c == 'N') {  // KN command - mark's medium values
      if(kbd_cmd) enable_temp_control();
      do_temp_control = 1;
      set_default_pid(1);
      if(0 && kbd_cmd) show_pid_values();
   }
   else if(c == 'O') {  // KO command - filter offset
      FILTER_OFFSET = (DATA_SIZE) atof(s);
      calc_k_factors();
   }
   else if(c == 'P') {  // KP command - pid gain
      P_GAIN = (DATA_SIZE) atof(s);
      calc_k_factors();
   }
   else if(c == 'R') {  // KR command - integrator reset
      k8 = (DATA_SIZE) atof(s);
      calc_k_factors();
   }
   else if(c == 'S') {  // KS command - scale factor (loop gain correction)
      k7 = (DATA_SIZE) atof(s);
      calc_k_factors();
   }
   else if(c == 'T') {  // KT command - pwm test
      #ifdef DOUBLE_DATA
         sscanf(s, "%lf%c%lf", &test_cool, &c, &test_heat);
      #else
         sscanf(s, "%f%c%f", &test_cool, &c, &test_heat);
      #endif

      if(test_heat || test_cool) {
         if(kbd_cmd) enable_temp_control();
         do_temp_control = 1;
      }
      if(++test_marker >= MAX_MARKER) test_marker = 1;
      if(test_marker < 0) test_marker = 1;
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
      KL_TUNE_STEP = (DATA_SIZE) atof(s);
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
         set_pps(user_pps_enable, pps_polarity,  delay_value, pps1_delay, pps_threshold, 0);
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



void go_full()
{
   // setup to go to a full screen mode
   not_safe = 1;
   go_fullscreen ^= 1;        // toggle full screen mode
   kill_deco = go_fullscreen; // disable X11 window decorations
   need_screen_init = 1;
}


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
   else if(screen_type == 'k') ; // plot_azel = AZEL_OK;
   else if(screen_type == 'j') ; // plot_azel = AZEL_OK;
   else if(screen_type == 'm') ; // plot_azel = 0;
   else if(screen_type == 'n') ; // plot_azel = 0;   // netbook
   else if(screen_type == 'p') ; // plot_azel = 0;   // Raspberry pi touchscreen
   else if(screen_type == 'r') ; // plot_azel = 0;   // Reduced screen size
   else if(screen_type == 's') ; // plot_azel = 0;
   else if(screen_type == 'u') ; // plot_azel = 0;
   else if(screen_type == 'c') ; // plot_azel = 0;   //!!! custom screen
   else if(screen_type == 'd') {
      if(rcvr_type == NO_RCVR) {
         plot_digital_clock = 1;
      }
   }
   else if(screen_type == 'e') {
      if(rcvr_type == NO_RCVR) {
         plot_digital_clock = 1;
      }
   }
   else if(screen_type == 'f') {
      plot_azel = 0;
      go_fullscreen = 1;
//      kill_deco = 1;
      need_screen_init = 1;
   }
   else {
      screen_type = 'm';
      text_mode = stm;
      return 1;
   }
   if(SCREEN_WIDTH < TINY_TINY_WIDTH) {  // TINY_TINY_SIZE vertical format 320x480
      plot_digital_clock = 0;
   }

   if(plot_azel || plot_signals) update_azel = 1;
   text_mode = stm;
if(text_mode == 0) no_x_margin = no_y_margin = 0;

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

   reset_first_key(22);
   prot_menu = 0;
   eofs = 1;
   adjust_screen_options();
   init_screen(9100);

   if     (rcvr_type != SCPI_RCVR);  // NEW_RCVR - mostly polled receivers here
   else if(rcvr_type != UCCM_RCVR);
   else if(rcvr_type != STAR_RCVR);
   else if(rcvr_type != TRUE_RCVR);
   else if(rcvr_type != ZYFER_RCVR);
   else init_messages(40, 0);  // zorky


   // set where the first point in the graphs will be
   last_count_y = PLOT_ROW + (PLOT_HEIGHT - VERT_MINOR);
   for(k=0; k<NUM_PLOTS+DERIVED_PLOTS; k++) {
      plot[k].last_y = PLOT_ROW+PLOT_CENTER;
   }

   redraw_screen();
}


void edit_screen_res()
{
int font_set;
int n;
char c;

   // change the screen size
   if((SCREEN_HEIGHT < TINY_TINY_HEIGHT) || (SCREEN_WIDTH < TINY_TINY_WIDTH)) {  // exit vector font mode if switching out of a TINY_TINY mode
      use_vc_fonts = 0;      // if not 0, draw fonts using vector chars
      vc_font_scale = 100;
   }

   strupr(edit_buffer);
   if(strstr(edit_buffer, "N")) return;

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

#ifdef WIN_VFX
      custom_height &= (~1);
      custom_width &= (~1);
#endif  // USE_X11

      if(custom_width < MIN_WIDTH) custom_width = MIN_WIDTH;
      if(custom_width > MAX_WIDTH) custom_width = MAX_WIDTH;
      if(custom_height < MIN_HEIGHT) custom_height = MIN_HEIGHT;
   }
   else if(screen_type == 'f') {
      custom_width = display_width;
      custom_height= display_height;
      if(custom_width < MIN_WIDTH) custom_width = MIN_WIDTH;
      if(custom_width > MAX_WIDTH) custom_width = MAX_WIDTH;
      if(custom_height < MIN_HEIGHT) custom_height = MIN_HEIGHT;
#ifdef WIN_VFX
      custom_height &= (~1);
      custom_width &= (~1);
      custom_width -= 10;
      custom_height -= 32;
#endif  // USE_X11
      screen_type = 'c';
   }

   #ifdef WINDOWS
      if(font_set == 0) {
         if(screen_type == 'c') ;
         else if(strstr(edit_buffer, "Y")) ;
         else if(strstr(edit_buffer, "I")) ;
         else if(0 && strstr(edit_buffer, "F")) ;
         else return;
      }
      if(0 && strstr(edit_buffer, "F")) initial_window_mode = VFX_TRY_FULLSCREEN;
      else initial_window_mode = VFX_WINDOW_MODE;
   #else  // __linux__  __MACH__   __FreeBSD__
   #endif

   if((user_font_size == 0) && (SCREEN_HEIGHT < SHORT_SCREEN)) font_set = 12;
   user_font_size = font_set;

   change_screen_res();
}

void new_screen(u08 c)
{
char *s;

   // setup for a new screen size
   screen_type = c;   // $ command
   if(c == 'd') {
      s="480x320";
      if(user_set_full_screen == 0) {
         go_fullscreen = 1;
         kill_deco = 1;
      }
   }
   else if(c == 'e') {
      s="320x480";
      if(user_set_full_screen == 0) {
         go_fullscreen = 1;
         kill_deco = 1;
      }
   }
   else if(c == 'p') {
      s="800x480";
      if(user_set_full_screen == 0) {
         go_fullscreen = 1;
         kill_deco = 1;
      }
   }
   else if(c == 'r') {
      s="1024x600";
      if(user_set_full_screen == 0) {
         go_fullscreen = 1;
         kill_deco = 1;
      }
   }
   else if(c == 'j') {
      s ="1280x800";
      if(user_set_full_screen == 0) {
         go_fullscreen = 1;
         kill_deco = 1;
      }
   }
   else if(c == 'u') {
      s="640x480";
   }
   else if (c == 's') {
      s="800x600";
   }
   else if(c == 'm') {
      s="1024x768";
   }
   else if(c == 'k') {
      s ="1280x960";
   }
   else if(c == 'l') {
      s="1280x1024";
   }
   else if(c == 'v') {
      s="1400x1050";
   }
   else if(c == 'x') {
      s="1680x1050";
   }
   else if(c == 'h') {
      s="1920x1080";
   }
   else if(c == 'z') {
      s="2048x1536";
   }
   else if(c == 't') {
      s="640x480 text only";
   }
   else if(c == 'n') {
      s="1000x540";
   }
   else if(c == 'c') {
      s="Custom screen size";
   }
   else if(c == 'f') {
      s="Full screen size";
      sprintf(out, "%dx%d", display_width,display_height);
      s = &out[0];
   }
   else              {
      screen_type = 'm';
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
   start_edit(SCREEN_CMD, out);
}



void update_check()
{
// if(strchr(edit_buffer, 'u') || strchr(edit_buffer, 'U')) {
   if(!strchr(edit_buffer, 'u') && !strchr(edit_buffer, 'U')) {
      pause_data = 0;    // release data pause
      dont_reset_queues = 0;
      new_queue(RESET_ALL_QUEUES, 11);  // erase the data queue
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

void set_cc_mode(int mode, DATA_SIZE volts)
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
         gregorian(0, jd);
         fprintf(debug_file, "\ncheck new moon day: %02d:%02d:%02d %02d/%02d/%04d  jd:%f\n",
            g_hours,g_minutes,g_seconds, g_month,g_day,g_year, jd);
         jd_nm = new_moon(jd);  // get this month's new moon
         gregorian(0, jd_nm-jtime(0,0,0,0.0));
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


void set_terminal_params()
{
char *s;

   // setup terminal emulattor port and hex/ascii flag

   strupr(edit_buffer);
   s = trim_whitespace(edit_buffer);
   if(s == 0) return;

   if(isdigit(s[0])) {
      term_port = atoi(s);
   }
   if(strchr(s, 'D')) term_port = DAC_PORT;
   if(strchr(s, 'E')) term_port = ECHO_PORT;
   if(strchr(s, 'F')) term_port = FAN_PORT;
   if(strchr(s, 'I')) term_port = TICC_PORT;
   if(strchr(s, 'K')) term_port = NMEA_PORT;
   if(strchr(s, 'N')) term_port = THERMO_PORT;
   if(strchr(s, 'R')) term_port = RCVR_PORT;
   if(strchr(s, 'T')) term_port = TRACK_PORT;

   if(strchr(s, 'A')) term_hex = 0;
   if(strchr(s, 'H')) term_hex = 1;
}


void set_monitor_params()
{
char *s;

   // setup monitor port and hex/ascii flag

   strupr(edit_buffer);

   s = trim_whitespace(edit_buffer);
   if(s == 0) return;

   if(isdigit(s[0])) {
      monitor_port = atoi(s);
   }
   if(strchr(s, 'D')) monitor_port = DAC_PORT;
   if(strchr(s, 'E')) monitor_port = ECHO_PORT;
   if(strchr(s, 'F')) monitor_port = FAN_PORT;
   if(strchr(s, 'I')) monitor_port = TICC_PORT;
   if(strchr(s, 'K')) monitor_port = NMEA_PORT;
   if(strchr(s, 'N')) monitor_port = THERMO_PORT;
   if(strchr(s, 'R')) monitor_port = RCVR_PORT;
   if(strchr(s, 'T')) monitor_port = TRACK_PORT;

   if(strchr(s, 'A')) monitor_hex = 0;
   if(strchr(s, 'H')) monitor_hex = 1;
}

void edit_ticc_mode()
{
int i;
unsigned port;

   if(rcvr_type == TICC_RCVR) port = RCVR_PORT;
   else if(ticc_port_open())  port = TICC_PORT;
   else {
      edit_error("No TICC device found.");
      return;
   }

   strupr(edit_buffer);
   i = edit_buffer[0];
   if     (i == 'T') ; // timestamp
   else if(i == 'I') ; // interval
   else if(i == 'P') ; // period
   else if(i == 'F') { // frequency
      if(ticc_type == TAPR_TICC) goto bad_mode;   // frequency
   }
   else if(i == 'H') { // phase
      if(ticc_type == TAPR_TICC) goto bad_mode;   // phase
   }
   else if(ticc_type != TAPR_TICC) {
      goto bad_mode;
   }
   else if(i == 'D') ;
   else if(i == 'L') ;
   else {
      bad_mode:
      sprintf(out, "Invalid TICC mode: %c", i);
      edit_error(out);
      return;
   }

   if(ticc_type != TAPR_TICC) {
      ticc_mode = i;
      have_ticc_mode = i;
      new_queue(RESET_ALL_QUEUES, 222);
   }
   else {
      sprintf(out, "M%c", i);
      set_ticc_config(port, out);
      get_ticc_config(port);
   }
}

void edit_ticc_sync()
{
int i;
unsigned port;

   if(rcvr_type == TICC_RCVR) port = RCVR_PORT;
   else if(ticc_port_open())  port = TICC_PORT;
   else {
      edit_error("No TICC device found.");
      return;
   }

   strupr(edit_buffer);  // aaaacccc
   i = edit_buffer[0];
   if     (i == 'M') ;
   else if(i == 'S') ;
   else {
      sprintf(out, "Invalid TICC sync mode: %c", i);
      edit_error(out);
   }
   sprintf(out, "Y%c", i);

   set_ticc_config(port, out);
   get_ticc_config(port);
}

void edit_ticc_fudge()
{
double val1, val2, val3, val4;
unsigned port;

   if(rcvr_type == TICC_RCVR) port = RCVR_PORT;
   else if(ticc_port_open())  port = TICC_PORT;
   else {
      edit_error("No TICC device found.");
      return;
   }

   val1 = fudge_a;  // aaaacccc
   val2 = fudge_b;
   val3 = fudge_c;
   val4 = fudge_d;
   sscanf(edit_buffer, "%lf %lf %lf %lf", &val1, &val2, &val3, &val4);

   sprintf(out, "GA%d", (int) val1);
   set_ticc_config(port, out);

   sprintf(out, "GB%d", (int) val2);
   set_ticc_config(port, out);

   get_ticc_config(port);
}

void edit_ticc_time2()
{
int val1, val2, val3, val4;
unsigned port;

   if(rcvr_type == TICC_RCVR) port = RCVR_PORT;
   else if(ticc_port_open())  port = TICC_PORT;
   else {
      edit_error("No TICC device found.");
      return;
   }

   val1 = time2_a;  // aaaacccc
   val2 = time2_b;
   val3 = time2_c;
   val4 = time2_d;
   sscanf(edit_buffer, "%d %d %d %d", &val1, &val2, &val3, &val4);

   sprintf(out, "FA%d", (int) val1);
   set_ticc_config(port, out);

   sprintf(out, "FB%d", (int) val2);
   set_ticc_config(port, out);

   get_ticc_config(port);
}

void edit_ticc_tune()
{
unsigned port;
int val;
int min_time;

   dac_dac = 0;
   min_time = 30;
   if(rcvr_type == X72_RCVR) {
      port = RCVR_PORT;
      min_time = 600;
   }
   else if(rcvr_type == TICC_RCVR) port = RCVR_PORT;
   else if(ticc_port_open())  port = TICC_PORT;
   else {
      edit_error("No TICC device found.");
      return;
   }

   val = (int) atof(edit_buffer);

   if(val >= min_time) {  // start analyzing pps drift
      ticc_tune_time = val;
      dac_dac = 1;
   }
   else if(val) {
      sprintf(out, "Autotune time must be >= %d seconds.", min_time);
      edit_error(out);
   }
   else if(dac_dac) {
      dac_dac = 0;
      edit_error("Auto tune canceled");
   }

   return;
}

void edit_ticc_dilat()
{
int val1, val2, val3, val4;
unsigned port;

   if(rcvr_type == TICC_RCVR) port = RCVR_PORT;
   else if(ticc_port_open())  port = TICC_PORT;
   else {
      edit_error("No TICC device found.");
      return;
   }

   val1 = dilat_a;  // aaaacccc
   val2 = dilat_b;
   val3 = dilat_c;
   val4 = dilat_d;
   sscanf(edit_buffer, "%d %d %d %d", &val1, &val2, &val3, &val4);

   sprintf(out, "DA%d", (int) val1);
   set_ticc_config(port, out);

   sprintf(out, "DB%d", (int) val2);
   set_ticc_config(port, out);

   get_ticc_config(port);
}

void edit_ticc_edge()
{
char e[4];
char c;
unsigned port;
unsigned i, j;

   if(rcvr_type == TICC_RCVR) port = RCVR_PORT;
   else if(ticc_port_open())  port = TICC_PORT;
   else {
      edit_error("No TICC device found.");
      return;
   }

   e[0] = edge_a;   // aaaacccc
   e[1] = edge_b;
   e[2] = edge_c;
   e[3] = edge_d;

   j = 0;
   strupr(edit_buffer);
   for(i=0; i<strlen(edit_buffer); i++) {  // get R/F chars
      c = edit_buffer[i];
      if(c == 'R') {
         e[j++] = c;
         if(j > 3) break;
      }
      else if(c == 'F') {
         e[j++] = c;
         if(j > 3) break;
      }
      else if(c == ' ') ;
      else if(c == '\t') ;
      else {
         sprintf(out, "%c is not a valid edge select", c);
         edit_error(out);
         return;
      }
   }

   if(e[0]) {
      sprintf(out, "EA %c", e[0]);
      set_ticc_config(port, out);
   }

   if(e[1]) {
      sprintf(out, "EB %c", e[1]);
      set_ticc_config(port, out);
   }

   get_ticc_config(port);
}


void edit_ticc_cal()
{
unsigned port;
int val;

   if(rcvr_type == TICC_RCVR) port = RCVR_PORT;
   else if(ticc_port_open())  port = TICC_PORT;
   else {
      edit_error("No TICC device found.");
      return;
   }

   val = (int) atof(edit_buffer);
   if     (val == 2) ;
   else if(val == 10) ;
   else if(val == 20) ;
   else if(val == 40) ;
   else {
      edit_error("Cal periods must be 2, 10, 20, or 40");
      return;
   }
   sprintf(out, "P%d", val);
   set_ticc_config(port, out);

   get_ticc_config(port);
}

void edit_ticc_timeout()
{
unsigned port;
int val;

   if(rcvr_type == TICC_RCVR) port = RCVR_PORT;
   else if(ticc_port_open())  port = TICC_PORT;
   else {
      edit_error("No TICC device found.");
      return;
   }

   val = (int) atof(edit_buffer);
   sprintf(out, "T%d", val);
   set_ticc_config(port, out);

   get_ticc_config(port);
}

void edit_ticc_speed()
{
unsigned port;
double val;

   if(rcvr_type == TICC_RCVR) port = RCVR_PORT;
   else if(ticc_port_open())  port = TICC_PORT;
   else {
      edit_error("No TICC device found.");
      return;
   }

   val = atof(edit_buffer);
   sprintf(out, "S%.6f", val);
   set_ticc_config(port, out);

   get_ticc_config(port);
}

void edit_ticc_coarse()
{
unsigned port;
double val;

   if(rcvr_type == TICC_RCVR) port = RCVR_PORT;
   else if(ticc_port_open())  port = TICC_PORT;
   else {
      edit_error("No TICC device found.");
      return;
   }

   val = atof(edit_buffer);
   sprintf(out, "C%.6f", val);
   set_ticc_config(port, out);

   get_ticc_config(port);
}


void set_nominal_freqs(double dval)
{
    // set nominal freq val and phase wrap interval for all channels.
    if(dval == 0.0) dval = NOMINAL_FREQ;

    nominal_cha_freq = dval;
    nominal_chb_freq = dval;
    nominal_chc_freq = dval;
    nominal_chd_freq = dval;
    if(nominal_cha_freq) cha_phase_wrap_interval = (1.0 / nominal_cha_freq);  // !!!!! what about chc and chd
    if(nominal_chb_freq) chb_phase_wrap_interval = (1.0 / nominal_chb_freq);
    if(nominal_chc_freq) chc_phase_wrap_interval = (1.0 / nominal_chc_freq);  // !!!!! what about chc and chd
    if(nominal_chd_freq) chc_phase_wrap_interval = (1.0 / nominal_chd_freq);
}

void update_delta_t_file()
{
FILE *file;

   // write user delta-t value to deltat.dat file

   file = fopen(DELTAT_FILE, "w");
   if(file) {
      fprintf(file, "%.6f\n", user_delta_t);
      fclose(file);
      file = 0;
   }
}

u08 more_string_params()
{
int i;
unsigned j;
int r, n, a;
int dur;
double dval;
double mjd;
int yy,mm,dd;
char c;
char *f;

   // string_param() was broken into two parts to get around a compiler
   // block nesting limit

   if((getting_string >= '1') && (getting_string <= '8')) { // oscillator disciplinging param
      edit_osc_param();
   }
   else if(getting_string == MIN_RANGE_CMD) {  // min efc dac range
      edit_osc_param();
   }
   else if(getting_string == MAX_RANGE_CMD) {  // max efc dac range
      edit_osc_param();
   }
   else if(getting_string == X72_HOLDOVER_CMD) {
      edit_osc_param();
   }
   else if(getting_string == PULLIN_CMD) {
      edit_osc_param();
   }
   else if(getting_string == CABLE_CMD) {  // cable delay
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
   else if(getting_string == DEGLITCH_CMD) {
      dval = atof(edit_buffer);
      deglitch_plot_queue(selected_plot, (DATA_SIZE) dval);
      need_redraw =2897;
   }
   else if(getting_string == DEGLITCH_ALL_CMD) {
      dval = atof(edit_buffer);
      deglitch_plot_queue(-1, (DATA_SIZE) dval);
      need_redraw =2898;
   }
   else if(getting_string == DRVR_CMD) {
      strcpy(mode_string, edit_buffer);
      set_driver_mode();
   }
   else if(getting_string == PC_CMD) {
      if(edit_buffer[0]) batt_ovc = (DATA_SIZE) atof(edit_buffer);
      set_luxor_config();
   }
   else if(getting_string == PH_CMD) {
      if(edit_buffer[0]) batt_hvc = (DATA_SIZE) atof(edit_buffer);
      set_luxor_config();
   }
   else if(getting_string == PL_CMD) {
      if(edit_buffer[0]) batt_lvc = (DATA_SIZE) atof(edit_buffer);
      set_luxor_config();
   }
   else if(getting_string == PM_CMD) {
      if(rcvr_type == TM4_RCVR) {
         i = atoi(edit_buffer);
         set_timeport_format(i);
      }
      else if(luxor) {
         if(edit_buffer[0]) msg_timeout = (DATA_SIZE) atof(edit_buffer);
         set_luxor_config();
      }
   }
   else if(getting_string == PO_CMD) {
      if(edit_buffer[0]) load_ovc = (DATA_SIZE) atof(edit_buffer);
      set_luxor_config();
   }
   else if(getting_string == PP_CMD) {
      if(edit_buffer[0]) load_watts = (DATA_SIZE) atof(edit_buffer);
      set_luxor_config();
   }
   else if(getting_string == PR_CMD) {
      if(tolower(edit_buffer[0]) == 'r') reset_luxor_wdt(0x01);
   }
   else if(getting_string == PS_CMD) {
      if(edit_buffer[0]) tc2_ovt = (DATA_SIZE) atof(edit_buffer);
      set_luxor_config();
   }
   else if(getting_string == PT_CMD) {
      if(edit_buffer[0]) tc1_ovt = (DATA_SIZE) atof(edit_buffer);
      set_luxor_config();
   }
   else if(getting_string == PU_CMD) {
      if(edit_buffer[0]) load_lvc = (DATA_SIZE) atof(edit_buffer);
      set_luxor_config();
   }
   else if(getting_string == PV_CMD) {
      if(edit_buffer[0]) load_hvc = (DATA_SIZE) atof(edit_buffer);
      set_luxor_config();
   }
   else if(getting_string == PW_CMD) {
      if(edit_buffer[0]) batt_watts = (DATA_SIZE) atof(edit_buffer);
      set_luxor_config();
   }
   else if(getting_string == PX_CMD) {
      if(edit_buffer[0]) auxv_hvc = (DATA_SIZE) atof(edit_buffer);
      set_luxor_config();
   }
   else if(getting_string == PZ_CMD) {
      if(edit_buffer[0]) auxv_lvc = (DATA_SIZE) atof(edit_buffer);
      set_luxor_config();
   }
   else if(getting_string == WC_CMD) {
      write_luxor_config();
   }
   else if(getting_string == S_CMD) {
      i = parse_lla(edit_buffer);
      if(i == LLA_MISSING)      edit_error("Missing lat/lon/alt value");
      else if(i == NO_AVERAGE)  edit_error("No LLA data available to average");
      else if(i == BAD_LAT)     edit_error("Invalid lat value.");
      else if(i == BAD_LON)     edit_error("Invalid lon value.");
      else if(i == BAD_ALT)     edit_error("Invalid alt value.");
      else if(i)                edit_error("Invalid value seen.");
      else {
         lat = precise_lat;
         lon = precise_lon;
         alt = precise_alt;
         have_precise_lla = (-6);
         #ifdef BUFFER_LLA
//lla       clear_lla_points(0);
         #endif
      }
   }
   else if(getting_string == BC_CMD) {  // constant current load
      if(edit_buffer[0]) cc_val = (DATA_SIZE) atof(edit_buffer);
      strupr(edit_buffer);
      if(strstr(edit_buffer, "S")) sweep_stop = cc_val;
      else if(strstr(edit_buffer, "R")) sweep_stop = cc_val;
      unsafe_v = (DATA_SIZE) 0.0;
      set_cc_mode(CC_AMPS, (DATA_SIZE) 0.00);
   }
   else if(getting_string == BF_CMD) {  // lifepo4 battery charge mode
      if(edit_buffer[0]) cc_val = (DATA_SIZE) atof(edit_buffer);
      unsafe_v = (DATA_SIZE) 2.0;
      if(strstr(edit_buffer, "unsafe")) unsafe_v = (DATA_SIZE) 0.0;
      if(led_v < unsafe_v) {
         edit_error("Battery voltage is too low to safely charge!");
      }
      else if(led_v > (DATA_SIZE) (3.60F-0.05F)) {
         edit_error("Battery appears to be fully charged!");
      }
      else {
         sweep_stop = 0.0F;
         set_cc_mode(CC_LIPO, 3.60F);
      }
   }
   else if(getting_string == BH_CMD) {  // high voltage lipo battery charge mode
      cc_val = 0.0;
      lipo_volts = 0.0;

      #ifdef DOUBLE_DATA
         i = sscanf(edit_buffer, "%lf %lf", &cc_val, &lipo_volts);
      #else
         i = sscanf(edit_buffer, "%f %f", &cc_val, &lipo_volts);
      #endif

      if(i != 2) {
         edit_error("ERROR: must specify charge curent and charge voltage!");
         return 0;
      }

      unsafe_v = lipo_volts * (DATA_SIZE) 0.60;
      if(strstr(edit_buffer, "unsafe")) unsafe_v = (DATA_SIZE) 0.0;
      if(led_v < unsafe_v) {
         edit_error("Battery voltage is too low to safely charge!");
      }
      else if(unsafe_v && (led_v > (lipo_volts-0.05))) {
         edit_error("Battery appears to be fully charged!");
      }
      else {
         sweep_stop = (DATA_SIZE) 0.0;
         set_cc_mode(CC_LIPO, lipo_volts);
      }
   }
   else if(getting_string == BL_CMD) {  // lipo battery charge mode
      if(edit_buffer[0]) cc_val = (DATA_SIZE) atof(edit_buffer);
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

      #ifdef DOUBLE_DATA
         sscanf(edit_buffer, "%lf %lf", &sweep_start, &sweep_end);
      #else
         sscanf(edit_buffer, "%f %f", &sweep_start, &sweep_end);
      #endif

      if(sweep_start < 0.0) sweep_start = 0.0;
      if(sweep_start > 1.0) sweep_start = 1.0;
      if(sweep_end < 0.0) sweep_end = 0.0;
      if(sweep_end > 1.0) sweep_end = 1.0;
      cc_mode = PWM_SWEEP;
      update_check();
      sweep_stop = 0.0;
      cc_state = 0;
      cc_pwm = sweep_start;
   }
   else if(getting_string == BV_CMD) {  // constant load volatge
      if(edit_buffer[0]) cc_val = (DATA_SIZE) atof(edit_buffer);
      strupr(edit_buffer);
      if(strstr(edit_buffer, "S")) sweep_stop = cc_val;
      else if(strstr(edit_buffer, "R")) sweep_stop = cc_val;
      unsafe_v = 0.0F;
      set_cc_mode(CC_VOLTS, 0.00F);
   }
   else if(getting_string == BW_CMD) {  // constant load wattage
      if(edit_buffer[0]) cc_val = (DATA_SIZE) atof(edit_buffer);
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
      if(rcvr_type == ESIP_RCVR) {
         strcpy(out, edit_buffer);
         strupr(out);
         if(strstr(out, "FORMAT,PFEC")) {
            edit_error("THIS COMMAND COULD HAVE BRICKED YOUR RECEIVER... NOT SENT!");
            return NEED_SCREEN_REDRAW;
         }
      }
      send_user_cmd(edit_buffer);
   }
   else if(getting_string == AMPE_CMD) {
      emis1 = (DATA_SIZE) atof(edit_buffer);
      if(emis1 < 0.1F) emis1 = 0.1F;
      else if(emis1 > 1.0F) emis1 = 1.0F;
      set_emissivity(emis1, emis2);
   }
   else if(getting_string == AMPI_CMD) {
      emis2 = (DATA_SIZE) atof(edit_buffer);
      if(emis2 < 0.1F) emis2 = (DATA_SIZE) 0.1;
      else if(emis2 > 1.0F) emis2 = (DATA_SIZE) 1.0;
      set_emissivity(emis1, emis2);
   }
   else if(getting_string == AMPS_CMD) {
      strupr(edit_buffer);
      if(strstr(edit_buffer, "SERIAL")) {
         vref_b = (DATA_SIZE) atof(edit_buffer);
         if(vref_b < 0.0F) vref_b = 0.0F;
         vref_b = vref_b * 10000.0F + 0.50F;
         vref_b = ((DATA_SIZE) (int) vref_b) / 10000.0F;
         set_luxor_cal();
      }
      else {
         edit_error("You did not say the magic word!");
      }
   }
   else if(getting_string == AMPV_CMD) {
      strupr(edit_buffer);
      if(strstr(edit_buffer, "VREF")) {
         vref_m = (DATA_SIZE) atof(edit_buffer);
         if(vref_m < 1.00F) vref_m = 5.0F;
         else if(vref_m > 6.50F) vref_m = 6.144F;
         set_luxor_cal();
      }
      else {
         edit_error("You did not say the magic word!");
      }
   }
   else if(getting_string == SHOW_CALENDAR_CMD) {
      cal_month = 1;
      cal_year = pri_year;
      sscanf(edit_buffer, "%d %d", &cal_year,&cal_month);
      change_zoom_config(66);
      un_zoom = 0;
      zoom_screen = 'Q';
      config_screen(1129);
      cal_adjust = 0;
   }
   else if(getting_string == CAL_CMD) {
      edit_cal_param();
   }
   else if(getting_string == CALC_CMD) {
      rpn_calc();
      if(show_rpn_help == 0) {   // toots
         if(edit_buffer[0]) {
            add_kbd(0xFF);  // force show of calc menu (null line kludge)
            add_kbd(0x08);
         }
      }
   }
   else if(getting_string == EDIT_DEFINE_CMD) {
      msg_col = 0;
      strcpy(nmea_msg, edit_buffer);
      rpn_define(0);
      start_calc_zoom(11);
   }
   else if(getting_string == CS_DISP_CMD) {
      i = atoi(edit_buffer);
      set_cs_disp(i);
   }
   else if(getting_string == CS_LEAP_CMD) {
      yy = mm = dd = 0;
      mjd = 0.0;
      dur = 61;
      sscanf(edit_buffer, "%d%c%d%c%d %d", &yy,&c,&mm,&c,&dd, &dur);
      if(yy && mm && dd) mjd = jdate(yy,mm,dd) - JD_MJD;
      if((dur < 59) || (dur > 61)) {
         edit_error("Leap second duration must be 59 .. 61");
      }
      else {
         set_cs_leap(mjd, dur);
      }
   }
   else if(getting_string == CS_REMOTE_CMD) {
      i = atoi(edit_buffer);
      user_set_remote = i;
      set_cs_remote(i);
   }
   else if(getting_string == CS_SLEW_CMD) {
      dval = atof(edit_buffer);
      set_cs_slew(dval);
   }
   else if(getting_string == CS_STANDBY_CMD) {
      i = atoi(edit_buffer);
      set_cs_standby(i);
   }
   else if(getting_string == CS_STER_CMD) {
      dval = atof(edit_buffer);
      set_cs_ster(dval);
   }
   else if(getting_string == CS_SYNC_CMD) {
      strupr(edit_buffer);
      if     (edit_buffer[0] == 'O') set_cs_sync("OFF");
      else if(edit_buffer[0] == 'F') set_cs_sync("FRON");
      else if(edit_buffer[0] == 'R') set_cs_sync("REAR");
      else edit_error("Invalid sync source specified");
   }
   else if(getting_string == CS_TIMESET_CMD) {
      strupr(edit_buffer);
      if(strchr(edit_buffer, 'U')) need_cs_timeset = 'U';
      else if(strchr(edit_buffer, 'Z')) need_cs_timeset = 'U';
      else if(strchr(edit_buffer, 'L')) need_cs_timeset = 'L';
      else if(strchr(edit_buffer, 'D')) need_cs_timeset = 'L';
      else {
         edit_error("Must specify UTC or Local");
         need_cs_timeset = 0;
      }

      if(need_cs_timeset) set_cs_remote(1);
   }
   else if(getting_string == RUN_PGM_CMD) {
      f = trim_whitespace(&edit_buffer[0]);
      if(f) strcpy(out, f);
      else  out[0] = 0;

      run_program(out, 1);
   }
   else if(getting_string == DEBUG_LOG_CMD) {
      if(debug_file) {
         strupr(edit_buffer);
         if(strchr(edit_buffer, 'Y')) {
            fclose(debug_file);
            debug_file = 0;
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
         if(dbg_flush_mode) {
            if(strchr(edit_buffer, FLUSH_CHAR) == 0) strcat(edit_buffer, "*");
         }
         open_debug_file(edit_buffer);
         if(debug_file == 0) edit_error("Could not open debug log file.");
      }
   }
   else if(getting_string == PRN_LOG_CMD) {   // PRN log file name
      if(prn_file) {
         strupr(edit_buffer);
         if(strchr(edit_buffer, 'Y')) {
            fclose(prn_file);
            prn_file = 0;
            erase_screen();
            vidstr(0,0, YELLOW, "Closing PRN data log file.");
            refresh_page();
            Sleep(1000);
            erase_screen();
            need_redraw = 4243;
         }
      }
      else if(edit_buffer[0] == 0) {
         edit_error("No file name given for sat PRN data log file.");
      }
      else {
         if(prn_flush_mode) {
            if(strchr(edit_buffer, FLUSH_CHAR) == 0) strcat(edit_buffer, "*");
         }
         strcpy(prn_name, edit_buffer);
         prn_file = open_prn_file(prn_name, "wb");
         if(prn_file == 0) edit_error("Could not open sat PRN data log file.");
      }
   }
   else if(getting_string == TRACK_PORT_CMD) {
      track_port_info = 0;
      blink_prn = 0;
      track_prn = 0;
      strupr(edit_buffer);

      if(strstr(edit_buffer, "A")) {   // send info for all visible sats
         track_port_info |= SEND_SATS;
         track_port_info &= (~SEND_PRN);
         track_prn = 0;
      }
      if(strstr(edit_buffer, "M")) {   // send moon info
         track_port_info |= SEND_MOON;
      }
      if(strstr(edit_buffer, "S")) {   // send sun info
         track_port_info |= SEND_SUN;
      }
      if(strstr(edit_buffer, "T")) {   // send time (UTC)
         track_port_info |= SEND_TIME;
      }

      if(strstr(edit_buffer, "H")) {   // send/blink highest sat
         track_port_info |= SEND_HIGHEST;
         blink_prn = BLINK_HIGHEST;
      }

      for(j=0; j<strlen(edit_buffer); j++) {  // look for PRN number
         if(isdigit(edit_buffer[j])) {
            track_port_info &= (~SEND_SATS);
            track_prn = atoi(&edit_buffer[j]);
            if((track_prn > 0) && (track_prn <= MAX_PRN)) {
               track_port_info |= SEND_PRN;
               blink_prn = track_prn;
            }
            else {
               edit_error("Invalid sat PRN number specified.");
            }
            break;
         }
      }
   }
   else if(getting_string == PRS_MO_CMD) {
      i = atoi(edit_buffer);
      if((i < 2300) || (i > 3600)) {
         edit_error("MO value must be in the range (2600 .. 3600)");
         return NEED_SCREEN_REDRAW;
      }
      else {
         set_prs_mo(i);
      }
   }
   else if(getting_string == PRS_MS_CMD) {
      strupr(edit_buffer);
      if(strchr(edit_buffer, 'N')) i = 1;
      else if(strchr(edit_buffer, 'F')) i = 0;
      else i = atoi(edit_buffer) & 0x01;
      set_prs_ms(i);
   }
   else if(getting_string == PRS_SAVE_CMD) {
      edit_buffer[2] = 0;
      strupr(edit_buffer);
      if     (!strcmp(edit_buffer, "AL"));  // all
      else if(!strcmp(edit_buffer, "LM"));
      else if(!strcmp(edit_buffer, "FC"));
      else if(!strcmp(edit_buffer, "GA"));
      else if(!strcmp(edit_buffer, "SP"));
      else if(!strcmp(edit_buffer, "MO"));
      else if(!strcmp(edit_buffer, "TO"));
      else if(!strcmp(edit_buffer, "PL"));
      else if(!strcmp(edit_buffer, "PF"));
      else if(!strcmp(edit_buffer, "PT"));
      else {
         sprintf(out, "Error: %s param cannot be saved into EEPROM", edit_buffer);
         edit_error(out);
         return NEED_SCREEN_REDRAW;
      }

      save_prs_param(edit_buffer);
   }
   else if(getting_string == PRS_SF_CMD) {
      i = atoi(edit_buffer);
      if((i < -2000) || (i > 2000)) {
         edit_error("SF value must be in the range (-2000 .. 2000)");
         return NEED_SCREEN_REDRAW;
      }
      else {
         set_prs_sf(i);
      }
   }
   else if(getting_string == PRS_SP_CMD) {
      i = sscanf(edit_buffer, "%d %d %d", &r,&n,&a);
      if(i != 3) {
         edit_error("Must specify three values (R N A)");
         return NEED_SCREEN_REDRAW;
      }
      else if((r < 1500) || (r > 8191)) {
         edit_error("R value must be in range (1500 .. 8191)");
         return NEED_SCREEN_REDRAW;
      }
      else if((n < 800) || (n > 4095)) {
         edit_error("R value must be in range (800 .. 4095)");
         return NEED_SCREEN_REDRAW;
      }
      else if((a < 0) || (a > 63)) {
         edit_error("A value must be in range (0 .. 63)");
         return NEED_SCREEN_REDRAW;
      }
      else {
         set_prs_sp(r, n, a);
      }
   }
   else if(getting_string == PRS_TO_CMD) {
      if(strchr(edit_buffer, '*')) i = (prs_to - prs_tt);  // null out current TT reading
      else i = atoi(edit_buffer);   // user set TO value
      if((i < -32767) || (i > 32768)) {
         sprintf(out, "TO value (%d) must be in the range (-32767 .. 32768)", i);
         edit_error(out);
         return NEED_SCREEN_REDRAW;
      }
      else {
         set_prs_to(i);
      }
   }
   else if(getting_string == RAW_LOG_CMD) {   // log file name
      if(raw_file) {
         strupr(edit_buffer);
         if(strchr(edit_buffer, 'Y')) {
            fclose(raw_file);
            raw_file = 0;
            erase_screen();
            vidstr(0,0, YELLOW, "Closing receiver data capture file.");
            refresh_page();
            Sleep(1000);
            erase_screen();
            need_redraw = 4243;
            log_stream &= (~LOG_RAW_STREAM);
         }
      }
      else if(edit_buffer[0] == 0) {
         edit_error("No file name given for receiver data capture file.");
      }
      else {
         if(raw_flush_mode) {
            if(strchr(edit_buffer, FLUSH_CHAR) == 0) strcat(edit_buffer, "*");
         }
         strcpy(raw_name, edit_buffer);
         raw_file = open_raw_file(raw_name, "wb");
         if(raw_file == 0) edit_error("Could not open receiver data capture file.");
         else log_stream |= LOG_RAW_STREAM;
      }
   }
   else if(getting_string == SI_CMD) {
      i = atoi(edit_buffer);
      if((i > 0) && strchr(edit_buffer, '-')) i = 0 - i;

      sat_cols = 1;
      if(strchr(edit_buffer, '+')) {  // we want a 2 column display
         sat_cols = 2;
      }
      ms_row = ms_col = 0;

      if(strchr(edit_buffer, 't'))      tracked_only = 1;
      else if(strchr(edit_buffer, 'T')) tracked_only = 1;
      else                              tracked_only = 0;

      if((i == 0) && (strchr(edit_buffer, '-') || strchr(edit_buffer, '+'))) {
         if(strchr(edit_buffer, '-')) user_set_short = 1;
         else user_set_short = 0;
      }
      else {
         max_sat_display = i;
         if(max_sat_display < 0) {
            max_sat_display = (0-max_sat_display);
            user_set_short = 1;
         }
         else user_set_short = 0;
      }

      if(sat_cols > 1) {  // two column display
         if(max_sat_display & 1) ++max_sat_display;  // round up
      }

      max_sats = max_sat_display;  // used to format the sat_info data
      max_sat_count = max_sat_display;
      temp_sats = max_sat_display;
      config_sat_rows();
      config_screen(666);
      user_set_sat_cols = 1;
   }
   else if(getting_string == TICC_LOG_CMD) {   // log file name
      if(ticc_file) {
         strupr(edit_buffer);
         if(strchr(edit_buffer, 'Y')) {
            fclose(ticc_file);
            ticc_file = 0;
            erase_screen();
            vidstr(0,0, YELLOW, "Closing TICC data capture file.");
            refresh_page();
            Sleep(1000);
            erase_screen();
            need_redraw = 4223;
// aaattt            log_stream &= (~LOG_RAW_STREAM);
         }
      }
      else if(edit_buffer[0] == 0) {
         edit_error("No file name given for TICC receiver data log file.");
      }
      else {
         strcpy(ticc_name, edit_buffer);
         ticc_file = topen(ticc_name, "wb");
         if(ticc_file == 0) edit_error("Could not open TICC data log file.");
// aaattt         else log_stream |= LOG_RAW_STREAM;
      }
   }
   else if(getting_string == TICC_CAL_CMD) {
      edit_ticc_cal();
   }
   else if(getting_string == TICC_COARSE_CMD) {
      edit_ticc_coarse();
   }
   else if(getting_string == TICC_DILAT_CMD) {
      edit_ticc_dilat();
   }
   else if(getting_string == TICC_EDGE_CMD) {
      edit_ticc_edge();
   }
   else if(getting_string == TICC_FUDGE_CMD) {
      edit_ticc_fudge();
   }
   else if(getting_string == TICC_MODE_CMD) {
      edit_ticc_mode();
   }
   else if(getting_string == TICC_SPEED_CMD) {
      edit_ticc_speed();
   }
   else if(getting_string == TICC_SYNC_CMD) {
      edit_ticc_sync();
   }
   else if(getting_string == TICC_TIMEOUT_CMD) {
      edit_ticc_timeout();
   }
   else if(getting_string == TICC_TIME2_CMD) {
      edit_ticc_time2();
   }
   else if(getting_string == TICC_TUNE_CMD) {
      edit_ticc_tune();
   }
   else if(getting_string == TICC_FREQ_CMD) {
      strupr(edit_buffer);
      dval = atof(edit_buffer);
      if(dval <= 0.0) dval = NOMINAL_FREQ;

      i=0;
      if(strchr(edit_buffer, 'A')) { i |= 0x01; nominal_cha_freq = dval; cha_phase_wrap_interval = (1.0 / dval); }
      if(strchr(edit_buffer, 'B')) { i |= 0x02; nominal_chb_freq = dval; chb_phase_wrap_interval = (1.0 / dval); }
      if(strchr(edit_buffer, 'C')) { i |= 0x04; nominal_chc_freq = dval; chc_phase_wrap_interval = (1.0 / dval); }
      if(strchr(edit_buffer, 'D')) { i |= 0x08; nominal_chd_freq = dval; chd_phase_wrap_interval = (1.0 / dval); }
      if(strchr(edit_buffer, 'P')) { i |= 0x01; nominal_cha_freq = dval; cha_phase_wrap_interval = (1.0 / dval); }
      if(strchr(edit_buffer, 'O')) { i |= 0x02; nominal_chb_freq = dval; chb_phase_wrap_interval = (1.0 / dval); }
      if(i == 0) {
         set_nominal_freqs(dval);
      }

      new_queue(RESET_ALL_QUEUES, 54);
   }
   else if(getting_string == TRUE_TUNE_CMD) { // trueposition OCXO EFC learning
      if(atoi(edit_buffer)) train_true_ocxo();
   }
   else if(getting_string == TS_WRAP_CMD) { // timestamp wrap interval
      dval = atof(edit_buffer);
      if(dval < 0.0) dval = 0.0 - dval;
      timestamp_wrap = dval;

      new_queue(RESET_ALL_QUEUES, 154);
   }
   else if(getting_string == PHASE_WRAP_CMD) {  // !!!!! support independent cha and chb values
      i = sscanf(edit_buffer, "%lf %lf %lf %lf ", &cha_phase_wrap_interval, &chb_phase_wrap_interval, &chc_phase_wrap_interval, &chd_phase_wrap_interval);

      if(i) user_set_wrap = 0;
      if(i) {
         if(cha_phase_wrap_interval < 0.0) cha_phase_wrap_interval = (1.0 / nominal_cha_freq);
         user_set_wrap |= 0x01;
      }
      if(i > 1) {
         if(chb_phase_wrap_interval < 0.0) chb_phase_wrap_interval = (1.0 / nominal_chb_freq);
         user_set_wrap |= 0x02;
      }
      if(i > 2) {
         if(chc_phase_wrap_interval < 0.0) chc_phase_wrap_interval = (1.0 / nominal_chc_freq);
         user_set_wrap |= 0x04;
      }
      if(i > 3) {
         if(chd_phase_wrap_interval < 0.0) chd_phase_wrap_interval = (1.0 / nominal_chd_freq);
         user_set_wrap |= 0x08;
      }

      if(cha_phase_wrap_interval < 0.0) cha_phase_wrap_interval *= (-1.0);
      if(chb_phase_wrap_interval < 0.0) chb_phase_wrap_interval *= (-1.0);
      if(chd_phase_wrap_interval < 0.0) chc_phase_wrap_interval *= (-1.0);
      if(chd_phase_wrap_interval < 0.0) chd_phase_wrap_interval *= (-1.0);
   }
   else if(getting_string == TIME_CODE_CMD) {
      strupr(edit_buffer);
      if     (strchr(edit_buffer, '0')) set_time_code('0');
      else if(strchr(edit_buffer, '1')) set_time_code('1');
      else if(rcvr_type == TM4_RCVR) {
         edit_error("Invalid time code format selection!");
         return NEED_SCREEN_REDRAW;
      }
      else if(strchr(edit_buffer, '2')) set_time_code('2');
      else if(strchr(edit_buffer, '3')) set_time_code('3');
      else if(strchr(edit_buffer, '4')) set_time_code('4');
      else if(strchr(edit_buffer, '5')) set_time_code('5');
      else if(strchr(edit_buffer, 'X')) set_time_code('X');
      else if(strchr(edit_buffer, 'D')) ;  // enable current code on digital output
      else {
         edit_error("Invalid time code format selection!");
         return NEED_SCREEN_REDRAW;
      }

      if(rcvr_type == TM4_RCVR) ;
      else if(strchr(edit_buffer, 'D')) {  // enable digital time code output on PPS connector
         set_time_code('D');
      }
   }
   else if(getting_string == TRAIM_CMD) {
      i = atoi(edit_buffer);
      set_traim_mode(i, 1);
   }
   else if(getting_string == SORT_CMD) {
      strupr(edit_buffer);
      if(strchr(edit_buffer, 'C')) {  // GCC command - flag changes in sats used
         plot_const_changes = toggle_value(plot_const_changes);
         draw_plot(REFRESH_SCREEN);
      }
      else if(strchr(edit_buffer, 'G')) { // GCG
         plot_sat_count = toggle_value(plot_sat_count);
         draw_plot(REFRESH_SCREEN);
      }
      else if(strchr(edit_buffer, 'T')) { // GCT
//       force_si_cmd = 1;  // royal kludge to implement G C T keyboard command
         tracked_only ^= 1;
      }
      else if(strchr(edit_buffer, 'X')) { // GCX - toggle display of raw data
         no_raw_table ^= 1;
      }
      else if(strchr(edit_buffer, 'Z')) {  // GCZ command - disable plot area (and keyboard menus!)
         no_plots ^= 1;
         user_set_plot_row = 0;
         config_screen(7777);
      }
      else {
         sort_by = 0;
         sort_ascend = 1;

         if     (strchr(edit_buffer, SORT_AZ))      { sort_by = SORT_AZ;      sort_ascend = 1; }
         else if(strchr(edit_buffer, SORT_BIAS))    { sort_by = SORT_BIAS;    sort_ascend = 1; }
         else if(strchr(edit_buffer, SORT_DOPPLER)) { sort_by = SORT_DOPPLER; sort_ascend = 1; }
         else if(strchr(edit_buffer, SORT_ELEV))    { sort_by = SORT_ELEV;    sort_ascend = 0; }
         else if(strchr(edit_buffer, SORT_PRN))     { sort_by = SORT_PRN;     sort_ascend = 1; }
         else if(strchr(edit_buffer, SORT_RANGE))   { sort_by = SORT_RANGE;   sort_ascend = 1; }
         else if(strchr(edit_buffer, SORT_SIGS))    { sort_by = SORT_SIGS;    sort_ascend = 0; }
         else if(strchr(edit_buffer, SORT_CARRIER)) { sort_by = SORT_CARRIER; sort_ascend = 1; }
         else if(strchr(edit_buffer, SORT_STATE))   { sort_by = SORT_STATE;   sort_ascend = 0; }
         else if(strchr(edit_buffer, SORT_URA))     { sort_by = SORT_URA;     sort_ascend = 1; }
         else edit_error("Not a valid sort selection.");

         if(sort_by) {
            if     (strchr(edit_buffer, '+')) sort_ascend = 1;
            else if(strchr(edit_buffer, '-')) sort_ascend = 0;
//          if((sort_by == SORT_PRN) && sort_ascend) sort_by = 0; // don't color the PRN header
         }
      }
   }
   else if(getting_string == SAT_PLOT_CMD) {
      getting_string = 0;
      first_key = '{';
      add_kbd('G');
      add_kbd('$');
   }
   else if(getting_string == TIDE_CMD) {
      strupr(edit_buffer);
      if     (strchr(edit_buffer, 'X')) tide_kbd_cmd = 'X';
      else if(strchr(edit_buffer, 'Y')) tide_kbd_cmd = 'Y';
      else if(strchr(edit_buffer, 'Z')) tide_kbd_cmd = 'Z';  // alt or sat az
      else if(strchr(edit_buffer, 'G')) tide_kbd_cmd = 'G';
      else if(strchr(edit_buffer, 'I')) tide_kbd_cmd = 'I';

      else if(strchr(edit_buffer, 'E')) tide_kbd_cmd = 'E';  // sat elevation
      else if(strchr(edit_buffer, 'S')) tide_kbd_cmd = 'S';  // sat sig level

      else if(strchr(edit_buffer, 'H')) tide_kbd_cmd = 'H';  // external humidity sensor
      else if(strchr(edit_buffer, 'P')) tide_kbd_cmd = 'P';  // external pressure sensor
      else if(strchr(edit_buffer, 'T')) tide_kbd_cmd = 'T';  // external temperature sensor
      else if(strchr(edit_buffer, 'U')) tide_kbd_cmd = 'U';  // external second temp sensor

      else if(strchr(edit_buffer, 'A')) tide_kbd_cmd = 'A';
      else if(edit_buffer[0] == 0) return NEED_SCREEN_REDRAW;
      else {
         getting_plot = 0;
         if(enviro_mode()) {
            sprintf(out, "Invalid environmental sensor display selection.");
         }
         else {
            sprintf(out, "Invalid earth tide / gravity plot display selection.");
         }
         edit_error(out);
      }
getting_string = 0;
first_key = '{';
   }
   else if(getting_string == BAUD_CMD) {
      set_serial_params(RCVR_PORT, &edit_buffer[0], 1);
      Sleep(200);
      request_rcvr_info(222);
   }
   else if(getting_string == X72_ACMOS_ENAB_CMD) {
      i = x72_creg;
      if(atoi(edit_buffer)) i &= (~0x0010);
      else                  i |= 0x0010;
      set_x72_creg(i);
   }
   else if(getting_string == X72_DDS_CMD) {
      dval = atof(edit_buffer);
      if(fabs(dval) > 1.0E-6) {
         edit_error("Maximum freq control range is +/- 1.0E-6");
      }
      else {
         set_x72_dds(dval);
      }
   }
   else if(getting_string == X72_EFC_CMD) {
      i = atoi(edit_buffer);
      set_x72_efc(i);
   }
   else if(getting_string == X72_FREQ_CMD) {
      strupr(edit_buffer);
      i = atoi(edit_buffer);
      if(i && strchr(edit_buffer, 'H')) {  // set to closest freq in Hz
         dval = (x72_osc / 2.0) / atof(edit_buffer);
         i = (int) (dval + 0.50);
      }

      if((i < 1) || (i > 65536)) {
         edit_error("Divider ratio must be (3 .. 65536)");
      }
      else {
         set_x72_acmos_freq(i);
         dval = (x72_osc / 2.0) / (double) i;
         sprintf(debug_text, "ACMOS freq set to %f Hz", dval);
      }
   }
   else if(getting_string == X72_FXO_CMD) {
      i = x72_creg;
      if(atoi(edit_buffer)) i &= (~0x0004);
      else                  i |= 0x0004;
      set_x72_creg(i);
   }
   else if(getting_string == X72_OSC_CMD) {
      dval = atof(edit_buffer);
      if(dval <= 0.0) {
         edit_error("Device master oscillator freq must be > 0.0 Hz");
      }
      else {
         x72_osc = dval;
         user_set_x72_osc = 5;
         save_x72_state();
      }
   }
   else if(getting_string == X72_SINE_CMD) {
      r = atoi(edit_buffer);
      if     (r < 1)   r = 0x0040;                     // disable
      else if(r < 50)  r = (0x0000 | 0x0000 | 0x0000); // +0  40%  -> +30, +20, +10
      else if(r < 60)  r = (0x0000 | 0x0000 | 0x0200); // +10 50%  -> +30, +20, +10
      else if(r < 70)  r = (0x0000 | 0x0100 | 0x0000); // +20 60%  -> +30, +20, +10
      else if(r < 80)  r = (0x0080 | 0x0000 | 0x0000); // +30 70%  -> +30, +20, +10
      else if(r < 90)  r = (0x0080 | 0x0000 | 0x0200); // +40 80%  -> +30, +20, +10
      else if(r < 100) r = (0x0080 | 0x0100 | 0x0000); // +50 90%  -> +30, +20, +10
      else             r = (0x0080 | 0x0100 | 0x0200); // +60 100% -> +30, +20, +10

      i = x72_creg & (~(0x0040 | 0x0080 | 0x0100 | 0x0200));
      i |= r;
      set_x72_creg(i);
   }
   else if(getting_string == X72_TIC_CMD) {
      i = atoi(edit_buffer);
      set_x72_tic(i);
   }
   else if(getting_string == X72_TUNE_CMD) {
      edit_ticc_tune();
   }
   else if(getting_string == RT17_OFS_CMD) {
      strupr(edit_buffer);
      if(strchr(edit_buffer, 'Y')) i = 1;
      else if(strchr(edit_buffer, 'N')) i = 0;
      else i = atoi(edit_buffer);
      if(i) add_clk_ofs = 1;
      else  add_clk_ofs = 0;
   }
   else if(getting_string == STAR_CLEAR_WTR_CMD) {
      i = atoi(edit_buffer);
      if(i == 1) clear_star_wtr();
   }
   else if(getting_string == STAR_SET_WTR_CMD) {
      i = atoi(edit_buffer);
      if((i < 0) || (i > 2500)) {
         edit_error("Wait-to-restore value must be (0 .. 2500)");
         return NEED_SCREEN_REDRAW;
      }
      set_star_wtr(i);
   }
   else if(getting_string == STAR_SET_HBSQ_CMD) {
      i = atoi(edit_buffer);
      if((i < 0) || (i > 2500)) {
         edit_error("HBSQ value must be (0 .. 7200) minutes");
         return NEED_SCREEN_REDRAW;
      }
      set_star_hbsq(i);
   }
   else if(getting_string == STAR_TS_CMD) {
      strupr(edit_buffer);
      if(strchr(edit_buffer, 'Y')) i = 1;
      else if(strchr(edit_buffer, 'N')) i = 0;
      else i = atoi(edit_buffer);
      if(i) fix_star_ts = 1;
      else  fix_star_ts = 0;
   }
   else if(getting_string == TM4_PPS_CMD) {
      i = atoi(edit_buffer);
      set_pps_source(i);
   }
   else if(getting_string == LPFRS_FREQ_CMD) {  // !!!!!LPFRS
      strupr(edit_buffer);
      dval = atof(edit_buffer);
      if(strchr(edit_buffer, 'H')) {  // freq change in Hz
         dval /= 10.0E6;
      }
      if(strchr(edit_buffer, '*')) {  // incremental freq change
         dval += lpfrs_freq;
      }

      if((dval < -1.28E-7) || (dval > 1.27E-7)) {
         edit_error("PPS offset must be (-1.28E-7 .. 1.27E-7) parts");
         return NEED_SCREEN_REDRAW;
      }
      set_lpfrs_freq(dval);
   }
   else if(getting_string == SA35_FREQ_CMD) {  // !!!!!SA35
      strupr(edit_buffer);
      dval = atof(edit_buffer);
      if(strchr(edit_buffer, 'H')) {  // freq change in Hz
         dval /= 10.0E6;
      }
      if(strchr(edit_buffer, '*')) {  // incremental freq change
         dval += lpfrs_freq;
      }

      if((dval < -2.00E-8) || (dval > 2.00E-8)) {
         edit_error("PPS offset must be (-2.00E-8 .. 2.00E-8) parts");
         return NEED_SCREEN_REDRAW;
      }
      set_sa35_freq(dval);
   }
   else if(getting_string == VECTOR_FONT_CMD) {
      i = atoi(edit_buffer);
      if(i == 0) {
         use_vc_fonts = 0;
         vc_font_scale = 100;
         keyboard_cmd = 1;
         init_screen(4266);
         keyboard_cmd = 0;
         user_set_font_scale = 1;
      }
      else if((i >= 50) && (i <= 500)) {
         use_vc_fonts = 1;
         vc_font_scale = i;
         keyboard_cmd = 1;
         init_screen(4267);
         keyboard_cmd = 0;
         user_set_font_scale = 1;
      }
      else {
         edit_error("Vector font scale must be 50..500 percent)");
         return NEED_SCREEN_REDRAW;
      }
   }
   else if(getting_string == GNSS_CMD) {
      strupr(edit_buffer);
      i = 0;
      if(strchr(edit_buffer, 'A')) i |= MIXED;
      if(strchr(edit_buffer, 'C')) i |= BEIDOU;
      if(strchr(edit_buffer, 'D')) i |= default_gnss;
      if(strchr(edit_buffer, 'E')) i |= GALILEO;
      if(strchr(edit_buffer, 'G')) i |= GPS;
      if(strchr(edit_buffer, 'I')) i |= IRNSS;
      if(strchr(edit_buffer, 'J')) i |= QZSS;
      if(strchr(edit_buffer, 'M')) i |= IMES;   // not a RINEX standard letter
      if(strchr(edit_buffer, 'R')) i |= GLONASS;
      if(strchr(edit_buffer, 'S')) i |= SBAS;
      if(i == 0) edit_error("No valid GNSS system selections seen.");
      else set_gnss_system(i);
   }
   else {
      if(getting_string >= 0x0100) sprintf(out, "Unknown edit string: 0x%04X", getting_string);
      else                         sprintf(out, "Unknown edit string: %c", getting_string);
      edit_error(out);
   }

   return NEED_SCREEN_REDRAW;
}

void reset_eph_flags()
{
int prn;

   for(prn=1; prn<MAX_PRN; prn++) {
      sat[prn].eph_valid = 0;
      sat[prn].eph_flag = 0;
   }
   have_range = 0;
   eph_polled = 0;
}

void erase_debug_info()
{
   debug_text[0] = 0;  // erase debug info strings
   debug_text2[0] = 0;
   debug_text3[0] = 0;
   debug_text4[0] = 0;
   debug_text5[0] = 0;
   debug_text6[0] = 0;
   debug_text7[0] = 0;
}


void show_plot_stats()
{
int old_stat;
char units[SLEN+1];
int id;
double val;

    // show all current plot statistics for a selected plot
    if(show_all_stats <= 0) return;

    id = selected_plot;
    if(id < 0) id = selected_plot = DAC;
    if(id >= (NUM_PLOTS+DERIVED_PLOTS)) id = selected_plot = DAC;

    old_stat = plot[id].show_stat;

    strcpy(units, plot[id].units);

    plot[id].show_stat = RMS;
    val = get_stat_val(id);
    sprintf(out, "%s  %.9f %s (%s)  ", stat_id, val, units, plot[id].plot_id);
    strcpy(debug_text7, out);

    plot[id].show_stat = AVG;
    val = get_stat_val(id);
    sprintf(out, "%s  %.9f %s (%s)  ", stat_id, val, units, plot[id].plot_id);
    strcpy(debug_text6, out);

    plot[id].show_stat = SDEV;
    val = get_stat_val(id);
    sprintf(out, "%s  %.9f %s (%s)  ", stat_id, val, units, plot[id].plot_id);
    strcpy(debug_text5, out);

    plot[id].show_stat = VAR;
    val = get_stat_val(id);
    sprintf(out, "%s  %.9f %s (%s)  ", stat_id, val, units, plot[id].plot_id);
    strcpy(debug_text4, out);

    plot[id].show_stat = SHOW_MIN;
    val = get_stat_val(id);
    sprintf(out, "%s  %.9f %s (%s)  ", stat_id, val, units, plot[id].plot_id);
    strcpy(debug_text3, out);

    plot[id].show_stat = SHOW_MAX;
    val = get_stat_val(id);
    sprintf(out, "%s  %.9f %s (%s)  ", stat_id, val, units, plot[id].plot_id);
    strcpy(debug_text2, out);

    plot[id].show_stat = SHOW_SPAN;
    val = get_stat_val(id);
    sprintf(out, "%s %.9f %s (%s)  ", stat_id, val, units, plot[id].plot_id);
    strcpy(debug_text, out);

    plot[id].show_stat = old_stat;
}

u08 string_param()
{
float val;   // !!!! DATA_SIZE
double dval;
u08 c;
int i, j;
unsigned k;

   // this routine evalates the string in edit_buffer
   // and sets the appropriate parameter value based upon 'getting_string'
   //
   // returns NEED_NEW_QUEUES if new queue setting needed
   // returns NEED_SCREEN_REDRAW if screen redraw needed
   //

   if(getting_string == ADEV_CMD) {  // all_adevs display mode
      strupr(edit_buffer);
      if     (edit_buffer[0] == 'A') mixed_adevs = MIXED_NONE;    // normal
      else if(edit_buffer[0] == 'G') mixed_adevs = MIXED_GRAPHS;  // graphs
      else if(edit_buffer[0] == 'R') mixed_adevs = MIXED_REGULAR; // regular
      else if(edit_buffer[0] == 'N') mixed_adevs = MIXED_REGULAR;
      plot_adev_data = 1;
      all_adevs = aa_val;
      config_screen(103);
      force_adev_redraw(101);
      need_redraw = 7645;
      if(adevs_active(0)) last_was_adev = getting_string;
   }
   else if(getting_string == ADEV_BIN_CMD) {  // adev bin sequence
      bin_scale = (int) (atof(edit_buffer)+0.50);
      if     (bin_scale == 0) ;
      else if(bin_scale == 1) ;
      else if(bin_scale == 2) ;
      else if(bin_scale == 3) ;
      else if(bin_scale == 4) ;
      else if(bin_scale == 5) ;
      else if(bin_scale == 8) ;
      else if(bin_scale == 10) ;
      else if(bin_scale == 29) ;
      else if(bin_scale == 99) ;
      else {
         bin_scale = 5;
      }

      recalc_adevs:
      first_show_bin = 0;
      max_adev_width = 0;
      max_bins_shown = 0;
      recalc_adev_info();
      force_adev_redraw(102);
      if(adevs_active(0)) last_was_adev = getting_string;
   }
   else if(getting_string == ADEV_HIDE_CMD) {
      adev_display_mask = 0xFFFF;
      strupr(edit_buffer);
      if(strchr(edit_buffer, 'A')) adev_display_mask &= (~DISPLAY_ADEV);
      if(strchr(edit_buffer, 'H')) adev_display_mask &= (~DISPLAY_HDEV);
      if(strchr(edit_buffer, 'M')) adev_display_mask &= (~DISPLAY_MDEV);
      if(strchr(edit_buffer, 'T')) adev_display_mask &= (~DISPLAY_TDEV);

//    if(strchr(edit_buffer, 'I')) adev_display_mask &= (~DISPLAY_MTIE);

      if(strchr(edit_buffer, 'P')) adev_display_mask &= (~DISPLAY_CHA);
      if(strchr(edit_buffer, 'O')) adev_display_mask &= (~DISPLAY_CHB);
      if(strchr(edit_buffer, 'C')) adev_display_mask &= (~DISPLAY_CHC);
      if(strchr(edit_buffer, 'D')) adev_display_mask &= (~DISPLAY_CHD);

      force_adev_redraw(103);
      need_redraw = 7645;
      if(adevs_active(0)) last_was_adev = getting_string;
   }
   else if(getting_string == ADEV_RECALC_CMD) {
      strupr(edit_buffer);
      if(strchr(edit_buffer, 'Y')) {
         goto recalc_adevs;
      }
   }
   else if(getting_string == ATTEN_CMD) {
      val = (float) atof(edit_buffer);
      set_atten((double) val);
   }
   else if(getting_string == ANTENNA_CMD) {
      i = atoi(edit_buffer);
      if(i) i = 1;
      set_antenna(i);
   }
#ifdef PRECISE_STUFF
   else if(getting_string == ABORT_SURV_CMD) {  // abort precise survey
      if((tolower(edit_buffer[0]) == 'y') || (tolower(edit_buffer[0]) == 'n')) {
         check_precise_posn = 0;
         plot_lla = 0;
         stop_self_survey(11);        // stop any self survey
         if(tolower(edit_buffer[0]) == 'y') {
            abort_precise_survey(10); // save current position using TSIP message
         }
         stop_precision_survey(11);
      }
   }
#endif
   else if(getting_string == STOP_SURVEY_CMD) {  // abort standard survey
      strupr(edit_buffer);
      if(strchr(edit_buffer, 'Y') || strchr(edit_buffer, '1')) {
         stop_self_survey(12);   // stop any self survey
         redraw_screen();
      }
   }
   else if(getting_string == SET_BAUD_CMD) {  // set receiver baud rate
      i = atoi(edit_buffer);
      i = set_rcvr_baud(i, com[RCVR_PORT].data_bits, com[RCVR_PORT].parity, com[RCVR_PORT].stop_bits);
      if(i == 1) {
         edit_error("Unsupported baud rate");
      }
      else if(i == 2) {
         edit_error("Changing receiver baud rate not supported for this device.");
      }
      else if(i) {
         edit_error("Unknown error");
      }
   }
   else if(getting_string == SET_RAW_CMD) {  // set raw observation data rate
      raw_msg_rate = atoi(edit_buffer);
      special_raw = 0;
      if(raw_msg_rate < 0) {
         raw_msg_rate = 0 - raw_msg_rate;
         special_raw = 1;
      }
      else if(0 && (raw_msg_rate == 0)) {
         raw_msg_rate = RAW_MSG_RATE;
      }
      reset_eph_flags();
      user_set_raw_rate = 1;
      init_messages(556,0);
   }
   else if(getting_string == KBD_TIMEOUT_CMD) {  // keyboard inactivity timeout
      strupr(edit_buffer);
      idle_timeout = atoi(edit_buffer);
      if(idle_timeout < 0) idle_timeout = 0;

      if(     strchr(edit_buffer, 'A')) idle_screen = 'A';
      else if(strchr(edit_buffer, 'B')) idle_screen = 'B';
      else if(strchr(edit_buffer, 'C')) idle_screen = 'C';
      else if(strchr(edit_buffer, 'D')) idle_screen = 'D';
      else if(strchr(edit_buffer, 'E')) idle_screen = 'E';
      else if(strchr(edit_buffer, 'I')) idle_screen = 'I';
      else if(strchr(edit_buffer, 'L')) idle_screen = 'L';
      else if(strchr(edit_buffer, 'M')) idle_screen = 'M';
      else if(strchr(edit_buffer, 'N')) idle_screen = 'N';
      else if(strchr(edit_buffer, 'O')) idle_screen = 'O';
      else if(strchr(edit_buffer, 'P')) idle_screen = 'P';
      else if(strchr(edit_buffer, 'R')) idle_screen = 'R';
      else if(strchr(edit_buffer, 'S')) idle_screen = 'S';
      else if(strchr(edit_buffer, 'U')) idle_screen = 'U';
      else if(strchr(edit_buffer, 'V')) idle_screen = 'V';
      else if(strchr(edit_buffer, 'W')) idle_screen = 'W';
      else if(strchr(edit_buffer, 'X')) idle_screen = 'X';
      else if(strchr(edit_buffer, 'Y')) idle_screen = 'Y';
      else if(strchr(edit_buffer, 'Z')) idle_screen = 'Z';
   }

#ifdef OSC_CONTROL
   else if(getting_string == OSCPID_CMD) {  // osc control PID
      if(edit_osc_pid_value(edit_buffer[0], &edit_buffer[1], 1)) {
         edit_error("Unknown osc PID command");
      }
   }
#endif
   else if(getting_string == DACV_CMD) {  // DAC control voltage
      if(edit_buffer[0] == 0) {
         if(rcvr_type == UCCM_RCVR) dval = (float) uccm_voltage;
         else if(rcvr_type == UCCM_RCVR) dval = (float) ((prs_fc1*4096) + prs_fc2);
         else dval = (float) dac_voltage;
      }
      else if(rcvr_type == PRS_RCVR) {
         i = prs_fc1;
         j = prs_fc2;
         sscanf(edit_buffer, "%d %d", &i,&j);
         dval = (float) ((i*4096) + j);
      }
      else dval = (float) atof(edit_buffer);

      if(rcvr_type == UCCM_RCVR) ;
      else if(rcvr_type == CS_RCVR) ;
      else if(rcvr_type == PRS_RCVR) ;
      else if(dval < (-10.0)) dval = (-10.0);  // !!!!! is this a proper range of limits?
      else if(dval > 10.0) dval = 10.0;

      if(rcvr_type == CS_RCVR) {
         set_discipline_mode(SET_DIS_MODE_ENABLE);  // enter standby
      }
      else {
         set_discipline_mode(SET_DIS_MODE_DISABLE);
      }
      set_dac_voltage((DATA_SIZE) dval, 10);
      osc_discipline = 0;
   }
   else if(getting_string == CENTER_CMD) { // center line zero reference
      if(edit_buffer[0]) {
         val = (float) atof(edit_buffer);
         plot[selected_plot].plot_center = (DATA_SIZE) val;
         plot[selected_plot].float_center = 0;
      }
      else plot[selected_plot].float_center = 1;
      plot[selected_plot].user_set_float = 1;
      return 0;
   }
   else if(getting_string == SCALE_CMD) {  // plot scale factor
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
   else if(getting_string == ELEV_CMD) {  // elevation angle
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
   else if(getting_string == FILTER_CMD) {  // display filter
      strupr(edit_buffer);
      disp_filter_type = 0;
      if(edit_buffer[0]) {
         filter_count = (long) atof(edit_buffer);
      }
      else filter_count = 0;
      if(strchr(edit_buffer, 'P')) disp_filter_type = 'P';  // peak filter
      else if(strchr(edit_buffer, 'B')) disp_filter_type = 'B';  // back filter
      else if(strchr(edit_buffer, 'F')) disp_filter_type = 'F';  // forward filter
      else if(strchr(edit_buffer, 'C')) disp_filter_type = 'C';  // center filter
      config_screen(104);
   }
   else if(getting_string == COLOR_CMD) {  // set plot color
      if(edit_buffer[0]) {
         val = (float) (int) atof(edit_buffer);
         if((val >= 0.0F) && (val < 16.0F)) {
            plot[selected_plot].plot_color = (int) val;
         }
      }
   }
   else if(getting_string == CHIME_CMD) {  // set chime interval
      strupr(edit_buffer);
      cuckoo_hours = singing_clock = ships_clock = tick_clock = fine_tick_clock = 0;

      if(strstr(edit_buffer, "H")) cuckoo_hours  = 1;

      if     (strstr(edit_buffer, "S")) singing_clock = 1;
      else if(strstr(edit_buffer, "W")) singing_clock = 2;  // for Big Ben / Westminster chimes
      else if(strstr(edit_buffer, "B")) ships_clock = 1;
      else if(strstr(edit_buffer, "F")) tick_clock = fine_tick_clock = 1;
      else if(strstr(edit_buffer, "T")) tick_clock = 1;

      val = (float) atof(edit_buffer);
      if(val < 0.0F)  val = 0.0F - val;
      if(val > 60.0F) val = 4.0F;
      if(tick_clock) {
         tick_clock = (u08) val;
         if(tick_clock > 3) tick_clock = 3;
         cuckoo = 0;
      }
      else cuckoo = (u08) val;
   }
   else if(getting_string == TITLE_CMD) {  // change graph titles and debug text
      if     (!strncmp(edit_buffer, "&1", 2)) strcpy(debug_text,  &edit_buffer[2]);
      else if(!strncmp(edit_buffer, "&2", 2)) strcpy(debug_text2, &edit_buffer[2]);
      else if(!strncmp(edit_buffer, "&3", 2)) strcpy(debug_text3, &edit_buffer[2]);
      else if(!strncmp(edit_buffer, "&4", 2)) strcpy(debug_text4, &edit_buffer[2]);
      else if(!strncmp(edit_buffer, "&5", 2)) strcpy(debug_text5, &edit_buffer[2]);
      else if(!strncmp(edit_buffer, "&6", 2)) strcpy(debug_text6, &edit_buffer[2]);
      else if(!strncmp(edit_buffer, "&7", 2)) strcpy(debug_text7, &edit_buffer[2]);
      else if(!strncmp(edit_buffer, "&b", 2)) {  // &b - set title bar
         strcpy(szAppName, &edit_buffer[2]);
         user_set_title_bar = 10;
         set_title_bar(10);
      }
      else {  // changing plot_title
         strcpy(plot_title, edit_buffer);
         if(plot_title[0]) title_type = USER;
         else              title_type = NONE;
      }

      sprintf(log_text, "#TITLE: %s", plot_title);
      write_log_comment(1);
      return NEED_SCREEN_REDRAW;
   }
   else if(getting_string == TRIM_QUEUE_CMD) {  // remove data from plot queue
      strupr(edit_buffer);
      i = 0;
      if(strchr(edit_buffer, 'S')) i |= TRIM_Q_START;
      if(strchr(edit_buffer, 'E')) i |= TRIM_Q_END;
      if(i) {
         trim_plot_queue(i);
         add_kbd('v');    // force display to show all queue data
         add_kbd('a');
         add_kbd(0x0D);
         need_redraw = 8278;
      }
   }
   else if(getting_string == QUEUE_INT_CMD) {  // set queue interval
      if(edit_buffer[0]) {
         val = (float) atosecs(edit_buffer);
         if(val < 1.0) {
            edit_error("Bad queue update interval");
         }
         else {   // !!!!!! luxor
            queue_interval = (long) (val+0.5F);
            plot[DAC].plot_center = last_dac_voltage = (DATA_SIZE) 1.0;
            last_temperature = (DATA_SIZE) 30.0;
            plot[TEMP].plot_center = scale_temp((double) last_temperature);
            user_set_qi = 1;
            return NEED_NEW_QUEUES;
         }
      }
   }
   else if(getting_string == LOG_INT_CMD) {  // log interval
      val = (float) atosecs(edit_buffer);
      if(val < 0.0F) val = 0.0F - val;
      if(val == 0.0F) val = 1.0F;
      log_interval = (long) val;
      sync_log_file();
      return NEED_SCREEN_REDRAW;
   }
#ifdef TEMP_CONTROL
   else if(getting_string == TEMP_PID_CMD) {  // temperature control PID
      if(edit_temp_pid_value(edit_buffer[0], &edit_buffer[1], 1)) {
         edit_error("Unknown temperature PID command");
      }
   }
#endif
   else if(getting_string == PROP_DELAY_CMD) {  // calculate propogation delay
      i = calc_prop_delay(edit_buffer);
      if     (i == BAD_LAT)     edit_error("Bad latitude value");
      else if(i == BAD_LON)     edit_error("Bad longitude value");
      else if(i == BAD_ALT)     edit_error("Bad altitude value");
      else if(i == LLA_MISSING) edit_error("Must enter three lat lon alt values (separated by spaces)");
      else if(i == NO_AVERAGE)  edit_error("No LLA data available to average");
      else if(i)                edit_error("Invalid entry");
   }
   else if(getting_string == SET_LLA_CMD) {  // enter precise lat/lon/alt
      edit_lla_cmd(0);
   }
   else if(getting_string == TRIMBLE_LLA_CMD) {  // enter precise lat/lon/alt
      if(lla_file == 0) open_lla_file(3);
      precision_survey = 1;
      trimble_save = 1;
      edit_lla_cmd(-1);   // use multiple singe point surveys
      precision_survey = 1;
      trimble_save = 1;
   }
   else if(getting_string == SET_LLA_REF_CMD) {  // enter scattergram reference lat/lon/alt
      edit_lla_ref_cmd();
   }
#ifdef PRECISE_STUFF
   else if(getting_string == ABORT_LLA_CMD) {  // abort lat/lon/alt position save search
      if((tolower(edit_buffer[0]) == 'y') || (tolower(edit_buffer[0]) == 'n')) {
         check_precise_posn = 0;
         plot_lla = 0;
         stop_self_survey(13);     // stop any self survey
         if(tolower(edit_buffer[0]) == 'y') {
            save_precise_posn(1);  // save position using single precision numbers
         }
      }
   }
#endif
   else if(getting_string == SET_SIGMASK_CMD) {  // enter AMU mask
      if(edit_buffer[0]) {
         val = (float) atof(edit_buffer);
         if(val < 0.0F) edit_error("Invalid signal level mask");
         else           set_amu_mask(val);
      }
      return 0;
   }
   else if(getting_string == LOG_CMD) {   // log file name
      if(edit_buffer[0] == 0) {
         edit_error("No file name given");
      }
      else {
         if(log_flush_mode) {
            if(strchr(edit_buffer, FLUSH_CHAR) == 0) strcat(edit_buffer, "*");
         }
         strcpy(log_name, edit_buffer);
         open_log_file(log_mode);
         log_written = 0;
//       rinex_header_written = 0;
//       rinex_obs_written = 0;
         if(log_file == 0) edit_error("Could not open log file.");
      }
   }
   else if(getting_string == LOG_CLOSE_CMD) {
      strupr(edit_buffer);
      if(log_file && (strchr(edit_buffer, 'Y') || strchr(edit_buffer, '1'))) {
         #ifdef ADEV_STUFF
            log_adevs();
         #endif
         log_stats();
         close_log_file();
log_stream = (log_stream & LOG_RAW_STREAM);
         have_info &= (~INFO_LOGGED);
      }
      return 0;
   }
   else if(getting_string == RINEX_ANT_TYPE_CMD) {  // set antenna name
//    strupr(edit_buffer);
      edit_buffer[20] = 0;
      strcpy(antenna_type, edit_buffer);
      return 0;
   }
   else if(getting_string == RINEX_ANT_NUM_CMD) {  // set antenna number
//    strupr(edit_buffer);
      edit_buffer[20] = 0;
      strcpy(antenna_number, edit_buffer);
      return 0;
   }
   else if(getting_string == MARKER_NAME_CMD) {  // set marker name
//    strupr(edit_buffer);
      edit_buffer[20] = 0;
      strcpy(marker_name, edit_buffer);
      return 0;
   }
   else if(getting_string == MARKER_NUM_CMD) {  // set marker number
//    strupr(edit_buffer);
      edit_buffer[20] = 0;
      strcpy(marker_number, edit_buffer);
      return 0;
   }
   else if(getting_string == RINEX_CLOSE_CMD) {  // close RINEX file
      strupr(edit_buffer);
      if(rinex_file && (strchr(edit_buffer, 'Y') || strchr(edit_buffer, '1'))) {
         fclose(rinex_file);
         rinex_file = 0;
         first_obs = 1;
         rinex_header_written = 0;
         rinex_obs_written = 0;
      }
      return 0;
   }
   else if(getting_string == RINEX_CPPR_CMD) {  // RINEX observation fixup mode
      strupr(edit_buffer);
      if(strchr(edit_buffer, 'Y')) i = 1;
      else if(strchr(edit_buffer, 'N')) i = 0;
      else i = atoi(edit_buffer);
      use_rinex_cppr = i;
      return 0;
   }
   else if(getting_string == RINEX_FIX_CMD) {  // RINEX observation fixup mode
      strupr(edit_buffer);
      if(strchr(edit_buffer, 'Y')) i = 1;
      else if(strchr(edit_buffer, 'N')) i = 0;
      else i = atoi(edit_buffer);
      rinex_fix = i;
      return 0;
   }
   else if(getting_string == RINEX_FILE_CMD) {   // RINEX file name
      if(jd_obs == 0) {
         edit_error("No RINEX data observations available");
      }
      else if(edit_buffer[0] == 0) {
         edit_error("No file name given");
      }
      else {
         if(rinex_flush_mode) {
            if(strchr(edit_buffer, FLUSH_CHAR) == 0) strcat(edit_buffer, "*");
         }
         strcpy(out, edit_buffer);
         open_rinex_file(out);
         rinex_header_written = 0;
         rinex_obs_written = 0;
         if(rinex_file == 0) edit_error("Could not open RINEX file.");
      }
      return 0;
   }
   else if(getting_string == RINEX_FORMAT_CMD) {  // set rinex format
      rinex_fmt = atof(edit_buffer) * 100.0;
      rinex_fmt = ((double) (int) rinex_fmt)/100.0;
      return 0;
   }
   else if(getting_string == RINEX_HEIGHT_CMD) {  // set antenna displacements
      sscanf(edit_buffer, "%lf%c%lf%c%lf", &antenna_height,&c,&antenna_ew,&c,&antenna_ns);
      return 0;
   }
   else if(getting_string == RINEX_LIST_CMD) {  // set observation list
strupr(edit_buffer);
      k = strlen(edit_buffer);
      while(k--) {
         if(edit_buffer[k] == ' ') edit_buffer[k] = 0;
         else if(edit_buffer[k] == '\t') edit_buffer[k] = 0;
         else break;
      }

      if(rinex_list == 'b') {
         strcpy(beidou_obs_list, edit_buffer);
      }
      else if(rinex_list == 'g') {
         strcpy(gps_obs_list, edit_buffer);
      }
      else if(rinex_list == 'l') {
         strcpy(galileo_obs_list, edit_buffer);
      }
      else if(rinex_list == 'm') {
         strcpy(mixed_obs_list, edit_buffer);
      }
      else if(rinex_list == 'n') {
         strcpy(glonass_obs_list, edit_buffer);
      }
      else if(rinex_list == 's') {
         strcpy(sbas_obs_list, edit_buffer);
      }
      else {
         sprintf(out, "Unknown observation list type: %c", rinex_list);
         edit_error(out);
      }
      return 0;
   }
   else if(getting_string == RTK_MODE_CMD) {
      strupr(edit_buffer);
      if(strchr(edit_buffer, 'E')) {  // enable RTCM output
         set_rtcm_mode(1);
      }
      else if(strchr(edit_buffer, 'D')) {  // disable RTCM output
         set_rtcm_mode(0);
      }

      i = (-1);
      if(strchr(edit_buffer, 'R')) i = ROVER_MODE;
      else if(strchr(edit_buffer, 'B')) i = BASE_MODE;
      else i = atoi(edit_buffer);
      if(i >= 0) {
         set_rtk_mode(i);
         Sleep(500);
         set_raw_rate();
         set_rtk_mode(i);
      }
   }
   else if(getting_string == RINEX_SITE_CMD) {  // set rinex site name
      edit_buffer[32] = 0;
      strupr(edit_buffer);
      strcpy(rinex_site, edit_buffer);
      set_rinex_site(rinex_site);
      return 0;
   }
   else if(getting_string == FOLIAGE_CMD) {  // fOliage or Jamming mode
      if((res_t && (res_t != RES_T)) || saw_icm || ACU_360 || ACU_GG || (rcvr_type == VENUS_RCVR)) {
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
   else if(getting_string == SET_PDOP_CMD) {  // pdop mask
      if(edit_buffer[0]) {
         val = (float) atof(edit_buffer);
         if(rcvr_type == FURUNO_RCVR) {
            if((val < 0.0) || (val > 10.0)) edit_error("PDOP make must be 1..10");
            else set_pdop_mask(val);
         }
         else {
            if((val < 1.0) || (val > 100.0)) edit_error("PDOP make must be 1..100");
            else set_pdop_mask(val);
         }
      }
      return 0;
   }
   else if(getting_string == SIGNAL_CMD) {
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
            change_zoom_config(-103);
            cancel_zoom(13);
         }
      }
      config_screen(105);
   }
   else if(getting_string == READ_CMD) {   // read in a log file
      c = reload_log(edit_buffer, 0);
      if(c == 0) { // adv, tim, log file
         plot_review(0L);
         #ifdef ADEV_STUFF
            force_adev_redraw(104);    // and make sure adev tables are showing
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
   else if(getting_string == SURVEY_CMD) {  // do self survey
      if(edit_buffer[0]) {
         do_survey = (long) atosecs(edit_buffer);
         survey_why = 30;

         if(do_survey < 0L) {
            edit_error("Bad self survey size");
         }
         else {
            #ifdef PRECISE_STUFF
               stop_precision_survey(12);  //medsurv
            #endif
            set_survey_params(1, 1, (long) do_survey);
            request_survey_params();  //!!!
            start_self_survey(0x00, 1);
         }
      }
   }
   else if(getting_string == DYNAMICS_CMD) {  // receiver dynamics
      if(rcvr_type == FURUNO_RCVR) {
         i = atoi(edit_buffer);
         if((i >= 1) && (i <= 3)) set_rcvr_dynamics(i);
         else edit_error("Movement dynamics index must be 1..3");
      }
      else {  // TSIP_RCVR
         if(     tolower(edit_buffer[0]) == 'a') set_rcvr_dynamics(3);
         else if(tolower(edit_buffer[0]) == 'f') set_rcvr_dynamics(4);
         else if(tolower(edit_buffer[0]) == 'l') set_rcvr_dynamics(1);
         else if(tolower(edit_buffer[0]) == 's') set_rcvr_dynamics(2);
         else edit_error("Movement dynamics must be:  A)ir  F)ixed  L)and  S)ea");
      }
      return 0;
   }
   else if(getting_string == SMOOTHING_CMD) {  // smoothing index
      i = atoi(edit_buffer);
      if((i >= 1) && (i <= 3)) set_rcvr_smoothing(i);
      else edit_error("Smoothing index must be:  1..3");
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
   else if(getting_string == VIEW_CMD) {  // set show interval
      if(edit_buffer[0]) {
         edit_user_view(&edit_buffer[0]);
      }
   }
   else if(getting_string == WRITE_CMD) {  // Write queue data to log file
      edit_write_cmd();
      return 0;
   }
   else if(getting_string == PLOT_PRN_CMD) {  // enable plotting of a sat PRN az/el/sig level
      strupr(edit_buffer);
      if(strchr(edit_buffer, 'S')) i = SUN_PRN;
      else if(strchr(edit_buffer, 'M')) i = MOON_PRN;
      i = atoi(edit_buffer);
      if(i < 0) i = 0;
      else if(i > SUN_MOON_PRN) i = 0;

      if(enviro_mode() && (rcvr_type != TIDE_RCVR)) {
         edit_error("Sat PRN plotting not available with environmental sensor");
         plot_prn = 0;
if(blink_prn == BLINK_PLOT) blink_prn = 0;
      }
      else {
         plot_prn = i;
         new_queue(RESET_ALL_QUEUES, 1515);

         if(plot_prn) config_prn_plots(1);
         else if(enviro_mode()) config_enviro_plots(1);
         else config_tide_plots(1);
blink_prn = BLINK_PLOT;  // blink the plotted prn
      }
      return 0;
   }
   else if(getting_string == SINGLE_SAT_CMD) {  // single sat mode
      if(edit_buffer[0] == 0) val = 0.0F;
      else val = (float) atof(edit_buffer);
      if(val < 0) val = 0.0F;
      else if(val > MAX_PRN) val = 0.0F;
      single_sat = ((int) val);
      set_single_sat(single_sat);
//    do_fixes(RCVR_MODE_SINGLE);
   }
   else if(getting_string == EDITOR_CMD) {  // edit a file
      text_editor();
   }
   else if(getting_string == SAT_IGN_CMD) {  // exclude sat mode
      if(edit_buffer[0] == 0) val = 0.0F;
      else val = (float) atof(edit_buffer);
      if(val < 0) val = 0.0F;
      else if(val > MAX_PRN) val = 0.0F;
      exclude_sat((int) val);
//    do_fixes(RCVR_MODE_SINGLE);
   }
   else if(getting_string == SET_TZ_CMD) {  // set time zone
      set_time_zone(edit_buffer);
      calc_dst_times(this_year, dst_list[dst_area]);
      need_redraw = 5869;
   }
#ifdef PRECISE_STUFF
   else if(getting_string == PRECISE_SURV_CMD) {  // do precision survey;
      if(edit_buffer[0] == 0) val = 48.0F;
      else val = (float) atof(edit_buffer);
      if((val < 3.0) || (val > (double) SURVEY_BIN_COUNT)) {
         sprintf(out, "Value must be between 3 and %d.  48 is good.", SURVEY_BIN_COUNT);
         edit_error(out);
      }
      else {
         do_median_survey = (long) val;
         do_survey = do_median_survey; //medsurv
         survey_why = 31;
         start_precision_survey(31);
      }
   }
#endif
#ifdef TEMP_CONTROL
   else if(getting_string == TEMP_SET_CMD) {  // set control temperature;
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
   else if(getting_string == SCREEN_CMD) {  // select a new screen resolution
      edit_screen_res();
      set_restore_size();
   }
   else if(getting_string == STAT_CMD) {  // select a plot statistic
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
            else if(c == '?') {
               show_all_stats ^= 1;
               erase_debug_info();
               break;
            }
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
         else if(c == '?') {
            show_all_stats ^= 1;
            erase_debug_info();
         }
         else if(c == ESC_CHAR) ;
         else              plot[selected_plot].show_stat = 0;
      }
      plot_stat_info = 0;
      for(c=0; c<NUM_PLOTS+DERIVED_PLOTS; c++) {
         plot_stat_info |= plot[c].show_stat;
      }
   }
   else if(getting_string == OPTION_CMD) {  // enter option/debug value
      edit_option_value();
   }
   else if(getting_string == REVERT_SEG_CMD) {
      i = atoi(edit_buffer);
      revert_segment((u08) i);
   }
   else if(getting_string == UTC_OFS_CMD) {
      i = atoi(edit_buffer);
      if((i < 0) || (i > 100)) {
         edit_error("UTC offset must be between 0 and 100 seconds");
      }
      else {
         set_rcvr_utc_ofs(i);
      }
   }
   else if(getting_string == MONITOR_CMD) {  // enter monitor port traffic mode
      set_monitor_params();
      set_monitor_mode(1);
   }
   else if(getting_string == TERM_CMD) {  // enter terminal emulator mode
      set_terminal_params();
      enable_terminal = 1;
      do_term(term_port);

      tsip_sync = 0;         // re-establish link to the receiver
      tsip_wptr = 0;
      tsip_rptr = 0;
      if(rcvr_type == X72_RCVR) init_messages(10, 0);
      else                      init_messages(10, 1);
      reset_first_key(28);
      need_redraw = 6678;
   }
   else if(getting_string == SWITCH_CMD) {  // do command line option
      edit_option_switch();
   }
   else if(getting_string == DRIFT_CMD) {  // remove drift rate from plot
      if(edit_buffer[0] == 0) val = 0.0F;
      else val = (float) atof(edit_buffer);
      plot[selected_plot].drift_rate = val;
      plot[selected_plot].show_freq = 0;
      if(val) plot[selected_plot].show_deriv = 0;
   }
   else if(getting_string == WRITE_SIGS_CMD) {  // az/el signal level data
      if(edit_buffer[0] == 0) {
         sprintf(edit_buffer, "%s.sig", unit_file_name);
      }
      dump_signals(edit_buffer);
   }
   else if(getting_string == ALARM_CMD) {  // set alarm time
      edit_dt(edit_buffer, 1);
   }
   else if(getting_string == EXIT_CMD) {   // set exit time
      edit_dt(edit_buffer, 1);
   }
   else if(getting_string == SCREEN_DUMP_CMD) {  // set screen dump time
      edit_dt(edit_buffer, 1);
   }
   else if(getting_string == LOG_DUMP_CMD) {  // set alarm or exit
      edit_dt(edit_buffer, 1);
   }
   else if(getting_string == SCRIPT_RUN_CMD) {  // run script
      edit_dt(edit_buffer, 1);
   }
   else if(getting_string == EXEC_PGM_CMD) {  // run program on a schedule
      edit_dt(edit_buffer, 1);
   }
   else if(getting_string == BLINK_PRN_CMD) {
      strupr(edit_buffer);
      if(strchr(edit_buffer, 'H')) blink_prn = BLINK_HIGHEST;
      else if(strchr(edit_buffer, 'P')) blink_prn = BLINK_PLOT;
      else if(strchr(edit_buffer, 'B')) ; // blink_prn = blink_prn;
      else blink_prn = atoi(edit_buffer);

      if(blink_prn < BLINK_PLOT) blink_prn = 0;
      if(blink_prn > SUN_MOON_PRN) blink_prn = 0;
   }
   else if(getting_string == SUN_CMD) {
      set_sunrise_type(edit_buffer, 1);
   }
   else if(getting_string == NVS_FILTER_CMD) {
      val = (float) atof(edit_buffer);
      set_filter_factor((DATA_SIZE) val);
      request_rcvr_config(3333);
   }
   else if(getting_string == NAV_RATE_CMD) {
      val = (float) atof(edit_buffer);
      set_nav_rate((DATA_SIZE) val);
      request_rcvr_config(3334);
   }
   else if(getting_string == DELTA_T_CMD) {
      strupr(edit_buffer);
      user_delta_t = atof(edit_buffer);
      if(strstr(edit_buffer, "*")) {  // update deltat.dat file with user value
         update_delta_t_file();
      }

      user_delta_t /= (24.0*60.0*60.0);  // convert seconds to days
      user_set_delta_t = 1;
      need_sunrise = 1;
   }
   else if(getting_string == TSX_CMD) {
      time_sync_offset = atof(edit_buffer);
   }
   else if(getting_string == PPS_OFS_CMD) {
      i = atoi(edit_buffer);
      if(rcvr_type == PRS_RCVR) {
         if((i < 0) || (i > 999999999)) {
            edit_error("PPS offset must be (0 .. 999999999)");
            return NEED_SCREEN_REDRAW;
         }
         set_prs_pp(i);
      }
      else set_pps(user_pps_enable, pps_polarity,  delay_value,  (double) i, pps_threshold, 2);
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
   else if(getting_string == SRO_WIDTH_CMD) {
      dval = atof(edit_buffer);
      if((dval < 0.0) || (dval > 999999.0)) {
         edit_error("PPS width must be (0 .. 999999)");
         return NEED_SCREEN_REDRAW;
      }
      set_sro_width(dval);
   }
   else if(getting_string == SRO_DELAY_CMD) {
      i = atoi(edit_buffer);
      if((i < 0) || (i > 999999999)) {
         edit_error("PPS offset must be (0 .. 999999999)");
         return NEED_SCREEN_REDRAW;
      }
      set_sro_delay((double) i);
   }
   else if(getting_string == SRO_FS_CMD) {
      i = atoi(edit_buffer);
      if((i < 0) || (i > 3)) {
         edit_error("Frequency save mode must be (0 .. 3)");
         return NEED_SCREEN_REDRAW;
      }
      set_sro_fs(i);
   }
   else if(getting_string == SRO_SY_CMD) {
      i = atoi(edit_buffer);
      if((i < 0) || (i > 3)) {
         edit_error("Sync mode must be (0 .. 3)");
         return NEED_SCREEN_REDRAW;
      }
      set_sro_sy(i);
   }
   else if(getting_string == SRO_TR_CMD) {
      i = atoi(edit_buffer);
      if((i < 0) || (i > 3)) {
         edit_error("Track mode must be (0 .. 3)");
         return NEED_SCREEN_REDRAW;
      }
      set_sro_tr(i);
   }
   else if(getting_string == SRO_RAW_CMD) {
      dval = atof(edit_buffer);
      if((dval < -17066.0) || (dval > 16933.0)) {
         edit_error("PPS offset must be (-17066 .. 16993)");
         return NEED_SCREEN_REDRAW;
      }
      set_sro_raw(dval);
   }
   else if(getting_string == SRO_FC_CMD) {
      strupr(edit_buffer);
      dval = atof(edit_buffer);
      if(strchr(edit_buffer, 'H')) {
         dval /= 10.0E6;  // convert Hz to PPT
         dval *= 1.0E12;
      }
      if((dval < -16776.0) || (dval > 16776.0)) {
         edit_error("PPS offset must be (-16776 .. 16776) ppt");
         return NEED_SCREEN_REDRAW;
      }
      set_sro_fc(dval);
   }
   else if(getting_string == SRO_CO_CMD) {
      dval = atof(edit_buffer);
      if((dval < -128.0) || (dval > 127.0)) {
         edit_error("Fine phase comparator offset must be (-128 .. 127)");
         return NEED_SCREEN_REDRAW;
      }
      set_sro_co(dval);
   }
   else if(getting_string == SRO_WINDOW_CMD) {
      dval = atof(edit_buffer);
      if((dval < 0.0) || (dval > 40000.0)) {
         edit_error("Window width must be (0 .. 40000)");
         return NEED_SCREEN_REDRAW;
      }
      set_sro_track_window(dval);
   }
   else if(getting_string == SRO_GF_CMD) {
      i = atoi(edit_buffer);
      if((i < 0) || (i > 65535)) {
         edit_error("Gofast value must be (0 .. 65535)");
         return NEED_SCREEN_REDRAW;
      }
      set_sro_gf(i);
   }
   else if(getting_string == SRO_ALARM_CMD) {
      dval = atof(edit_buffer);
      if((dval < 0.0) || (dval > 40000.0)) {
         edit_error("Window width must be (0 .. 40000)");
         return NEED_SCREEN_REDRAW;
      }
      set_sro_alarm_window(dval);
   }
   else {
      return more_string_params();
   }

   return NEED_SCREEN_REDRAW;
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
      reset_first_key(23);  // command sequence is done
first_key = 0;
      prot_menu = 0;
      draw_plot(REFRESH_SCREEN);   // replace command sub-menu with the normal plot
   }
   return 0;
}

int help_exit(int c, int reason)
{
int help_c, help_first;
   // There was an error in the keyboard command.
   // Exit the keystroke processor with a help message
   reason = 0;       // shut up compiler warning
   script_err = c;
   help_first = first_key;
   help_c = c;
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

   if(help_first == '?') ;            // invalid command
   else if(help_first && help_c) {
      BEEP(310);
      erase_screen();

      if((help_first >= 0x0100) || (help_c >= 0x0100)) {
         sprintf(out, "Unknown command");
      }
      else if((help_first < ' ') || (help_c < ' ')) {
         sprintf(out, "Unknown command");
      }
      else if(help_c == ' ') {
         sprintf(out, "Unknown command: %c <sp>", toupper(help_first));
      }
      else {
         sprintf(out, "Unknown command: %c %c", toupper(help_first), toupper(help_c));
      }
      vidstr(0,0, YELLOW, out);
      refresh_page();
      need_redraw = 3586;
      Sleep(1000);
   }

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
   reset_first_key(24);
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
   sprintf(out, " Location format:  D)ecimal   dmS)   G)rads   R)adians    (ESC to abort)");
   vidstr(EDIT_ROW+0, EDIT_COL, PROMPT_COLOR, out);
   sprintf(out, "                   mI)ls      H)am   U)TM     N)ATO       E)CEF");
   vidstr(EDIT_ROW+1, EDIT_COL, PROMPT_COLOR, out);
   sprintf(out, "                   F)eet      M)eters         P)rivate    A)utoscale LLA");
   vidstr(EDIT_ROW+2, EDIT_COL, PROMPT_COLOR, out);
   sprintf(out, "                   earth T)ide displacement");
   vidstr(EDIT_ROW+3, EDIT_COL, PROMPT_COLOR, out);

   refresh_page();
}


void edit_tide_plot()
{
   // process the earth tide / gravity plot commands

   getting_plot = (-2);
   first_key = '{';
   prot_menu = 0;

   if(text_mode) erase_screen();
   else          erase_help();

   // display prompt string for the command
   sprintf(out, " Earth tide plot:  1)lat   2)lon   3)alt   4)uGals   A)ll   (ESC to abort)");
   vidstr(EDIT_ROW+0, EDIT_COL, PROMPT_COLOR, out);

   refresh_page();
}

void edit_plot(int id, int c)
{
char *s;
char *a;
char *pid;

   // process the plot control commands

   last_selected_plot = selected_plot;
   if(id != FFT) pre_fft_plot = id;

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

   pid = plot[selected_plot].plot_id;
   sprintf(out, " %4s plot: A)utoscale %s all plots      X) toggle %s autoscale modes", pid, a, pid);
   vidstr(EDIT_ROW+0, EDIT_COL, PROMPT_COLOR, out);

   #ifdef FFT_STUFF
      if(selected_plot == FFT) sprintf(out, "            C)olor   I)nvert  S)cale factor   Z)ero ref     /)statistics");
      else if(luxor)           sprintf(out, "            C)olor   I)nvert  S)cale factor   Z)ero ref     /)statistics   &C)al");
      else                     sprintf(out, "            C)olor   I)nvert  S)cale factor   Z)ero ref     /)statistics   F)FT   H)ist");
   #else
      if(luxor) sprintf(out,                "            C)olor   I)nvert  S)cale factor   Z)ero ref     &)Cal   /)statistics");
      else      sprintf(out,                "            C)olor   I)nvert  S)cale factor   Z)ero ref     /)statistics");
   #endif
   vidstr(EDIT_ROW+1, EDIT_COL, PROMPT_COLOR, out);

   if(tie_plot(selected_plot)) {
      if(rcvr_type == PRS_RCVR) {
         sprintf(out,                       "            trend L)ine       =)remove trend  *)derivative  #) FC as freq   ~)deglitch");
      }
      else {
         sprintf(out,                       "            trend L)ine       =)remove trend  *)derivative  #) TIE as freq  ~)deglitch");
      }
   }
   else {
      sprintf(out,                          "            trend L)ine       =)remove trend  *)derivative  ~)deglitch");
   }
   vidstr(EDIT_ROW+2, EDIT_COL, PROMPT_COLOR, out);

   sprintf(out,                             "            %c or <cr>=%s", c, s);
   vidstr(EDIT_ROW+3, EDIT_COL, PROMPT_COLOR, out);

   refresh_page();
}



void set_steps()
{
   scale_step = plot[selected_plot].scale_factor*(DATA_SIZE) 0.01;
   center_step = (DATA_SIZE) 0.0;
   if(plot[selected_plot].ref_scale) {
      center_step = plot[selected_plot].scale_factor/(plot[selected_plot].ref_scale*(DATA_SIZE)10.0);
   }
}

void move_plot_up()
{
   plot[selected_plot].user_scale = 1;
   plot[selected_plot].plot_center -= center_step;
   plot[selected_plot].float_center = 0;
   plot[selected_plot].user_set_float = 1;
}

void move_plot_down()
{
   plot[selected_plot].plot_center += center_step;
   plot[selected_plot].user_scale = 1;
   plot[selected_plot].float_center = 0;
   plot[selected_plot].user_set_float = 1;
}

void shrink_plot()
{
   plot[selected_plot].user_scale = 1;
   plot[selected_plot].scale_factor += scale_step;
   plot[selected_plot].float_center = 0;
   plot[selected_plot].user_set_float = 1;
}

void grow_plot()
{
   plot[selected_plot].user_scale = 1;
   plot[selected_plot].scale_factor -= scale_step;
   plot[selected_plot].float_center = 0;
   plot[selected_plot].user_set_float = 1;
}

void toggle_plot(int id)
{
   if((id < 0) || (id >= (NUM_PLOTS+DERIVED_PLOTS))) return;
   if(id != FFT) pre_fft_plot = id;

   selected_plot = id;
   plot[selected_plot].show_plot = toggle_value(plot[selected_plot].show_plot);
   plot[selected_plot].user_set_show = 1;

if((selected_plot == FFT) && (plot[selected_plot].show_plot == 0)) { // just turned off FFT plot
   selected_plot = pre_fft_plot;
}

   if(USES_PLOT_THREE) ;
   else if((graph_lla == 0) && (selected_plot == THREE) && (luxor == 0)) {  //!!!! debug_plots
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
double val;
char *s;

   if(plot[plot_id].show_trend && (title_type != USER)) {
      if(plot[plot_id].stat_count && view_interval) {
         val = (plot[plot_id].sum_change/plot[plot_id].stat_count)/view_interval;
      }
      else val = 0.0;
      if(plot_id == OSC) {  // scale to microseconds
         if(rcvr_type == SCPI_RCVR) val *= 1000.0;
         else if(rcvr_type == UCCM_RCVR) val *= 1000.0;
      }

      sprintf(plot_title, "%s trend line: %s = (%g*t) + %g   avg_delta=%g",
                          plot[plot_id].plot_id,
                          plot[plot_id].plot_id,
                          plot[plot_id].a1,
                          plot[plot_id].a0,
                          val
      );

      if(trend_rate_display == 3) {  // per second trend
         val = plot[plot_id].a1 * (1.0);
         s = "sec";
      }
      else if(trend_rate_display == 2) {  // per minute trend
         val = plot[plot_id].a1 * (60.0);
         s = "min";
      }
      else if(trend_rate_display == 1) {
         val = plot[plot_id].a1 * (60.0*60.0);  // per hour trend
         s = "hr";
      }
      else { // per day trend
         val = plot[plot_id].a1 * (24.0*60.0*60.0);
         s = "day";
      }

      if(plot_id == OSC) {  // scale to microseconds
         if(rcvr_type == SCPI_RCVR) val *= 1000.0;
         else if(rcvr_type == UCCM_RCVR) val *= 1000.0;
      }

      if(tie_plot(plot_id)) {
         if(plot[plot_id].show_freq) {
            if(rcvr_type == X72_RCVR) {
               sprintf(out, "  (%g mHz/%s)", tie_to_freq(plot_id, val)*1.0E3, s);
            }
            else {
               sprintf(out, "  (%g Hz/%s)", tie_to_freq(plot_id, val), s);
            }
         }
         else {
            sprintf(out, "  (%g ns/%s)", val, s);
         }
      }
      else {
         sprintf(out, "  (%g %s/%s)", val, plot[plot_id].units, s);
      }
      strcat(plot_title, out);

      title_type = OTHER;
   }
}

void set_auto_scale()
{
int i;

   // enable / disable auto scaling of all plots

   for(i=0; i<NUM_PLOTS+DERIVED_PLOTS; i++) {
      if(auto_scale) {
         plot[i].user_scale = 0;
         plot[i].float_center = 1;
         plot[i].user_set_float = 1;
      }
      else {
         plot[i].user_scale = 1;
         plot[i].float_center = 0;
      }
   }
}

int change_plot_param(int c, int cmd_line)
{
    // this routine acts upon the plot control commands

    if(c < 0x0100) c = tolower(c);

    if(getting_plot == (-1)) {  // changing location mode - GL commands
       new_lla_mode = 1;
       if(c == 'a') {   // auto-scale LLA scattergram
          autoscale_lla ^= 1;
          change_lla_scale();
          rebuild_lla_plot(0);
//        plot_loc = 1;
          need_redraw = 4327;
       }
       else if(c == 'd') {  // decimal location
          dms = DECIMAL_FMT;
          plot_loc = 1;
          need_redraw = 1204;
       }
       else if(c == 'e') { // ECEF location
          dms = ECEF_FMT;
          plot_loc = 1;
          need_redraw = 1204;
       }
       else if(c == 'f') {  // scale in feet
          alt_scale = "ft";
          LLA_SPAN = last_lla_span = 10.0;  // lla plot scale in feet per division
          ANGLE_SCALE = DEG_PER_FOOT;       // degrees per foot
          angle_units = "ft";
          rebuild_lla_plot(0);
          plot_loc = 1;
          need_redraw = 1201;
       }
       else if(c == 'g') {   // Grads
          dms = GRAD_FMT;
          plot_loc = 1;
          need_redraw = 1208;
       }
       else if(c == 'h') {   // Ham radio Maidenhead grid square
          dms = GRIDSQ_FMT;
          plot_loc = 1;
          need_redraw = 1202;
       }
       else if(c == 'i') {   // Mils (6400 per cicrle)
          dms = MIL_FMT;
          plot_loc = 1;
          need_redraw = 1202;
       }
       else if(c == 'l') {
          alt_scale = "lg";
          LLA_SPAN = last_lla_span = 3.0;
          ANGLE_SCALE = ((DEG_PER_FOOT)*LG_PER_METER); // degrees per meter
          angle_units = "lg";
          rebuild_lla_plot(0);
          plot_loc = 1;
          need_redraw = 1291;
       }
       else if(c == 'm') {
          alt_scale = "m";
          LLA_SPAN = last_lla_span = 3.0;
          ANGLE_SCALE = ((DEG_PER_FOOT)*FEET_PER_METER); // degrees per meter
          angle_units = "m";
          rebuild_lla_plot(0);
          plot_loc = 1;
          need_redraw = 1201;
       }
       else if(c == 'n') {   // NATO (MGRS)
          dms = NATO_FMT;
          plot_loc = 1;
          need_redraw = 1203;
       }
       else if(c == 'p') {   // GLP - engage privacy mode
          plot_loc = toggle_value(plot_loc);
          need_redraw = 7723;
       }
       else if(c == 'r') {   // radians
          dms = RADIAN_FMT;
          plot_loc = 1;
          need_redraw = 1203;
       }
       else if(c == 's') {   // degrees minutes seconds
          dms = DMS_FMT;
          plot_loc = 1;
          need_redraw = 1204;
       }
       else if(c == 't') {   // show earth tides
          show_tides ^= 1;
          user_show_tides = 1;
          plot_loc = 1;
          need_redraw = 7724;
rebuild_lla_plot(0);  // ckckckck
       }
       else if(c == 'u') {   // UTM
          dms = UTM_FMT;
          plot_loc = 1;
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
       set_auto_scale();
    }
    else if(c == 'c') {  // graph color
       getting_plot = 0;
       edit_buffer[0] = 0;
       sprintf(edit_buffer, "%d", plot[selected_plot].plot_color);
       sprintf(out, "Enter color for %s graph (0-15):", plot[selected_plot].plot_id);
       start_edit(COLOR_CMD, out);
       return 1;
    }
#ifdef FFT_STUFF
    else if(c == 'f') {
       fft_type = FFT_TYPE;
       if(selected_plot != FFT) {
          plot[FFT].show_plot = 1;
          calc_fft(selected_plot);
          dump_fft_plot();
       }
    }
#endif
    else if(c == 'g') {  // g acts like an ignore
    }
    else if(c == 'h') {
       fft_type = HIST_TYPE;
       plot[FFT].show_plot = 1;
       calc_fft(selected_plot);
       dump_hist_plot();
    }
    else if(c == 'i') {  // invert plot
       plot[selected_plot].invert_plot *= (-1.0F);
    }
    else if(c == 'l') {  // trend line
       plot[selected_plot].show_trend = toggle_value(plot[selected_plot].show_trend);
       if(plot[selected_plot].show_trend) {
          plot[selected_plot].show_deriv = 0;    // turn off derivatice mode
          plot[selected_plot].show_freq = 0;
          plot[selected_plot].drift_rate = 0.0;  // turn off trend line mode
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
       edit_info1 =         "                             miN)   maX)   sP)an      ?)all";
       start_edit(STAT_CMD, "Select statistic to display: A)vg   R)ms   S)td dev   V)ar   <cr>=hide");
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
       plot[selected_plot].user_set_float = 1;
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
    else if(c == '*') {  // show plot derivative
       plot[selected_plot].show_deriv ^= 1;
       plot[selected_plot].show_freq = 0;
       if(plot[selected_plot].show_deriv) plot[selected_plot].drift_rate = 0.0;
    }
    else if(c == '#') {  // show frequency
       if(tie_plot(selected_plot)) {
          plot[selected_plot].show_freq ^= 1;
          plot[selected_plot].show_deriv = 0;
          if(plot[selected_plot].show_deriv) plot[selected_plot].drift_rate = 0.0;
       }
       else {
          getting_plot = 0;
          if(rcvr_type == PRS_RCVR) {
             edit_error("Can only display OSC plot as frequency");
          }
          else if(rcvr_type == TICC_RCVR) {
             edit_error("Can only display TIE plots as frequency");
          }
          else {
             edit_error("Cannot display plot as frequency");
          }
       }
    }
//  else if((c == '&') && luxor && cal_mode) {
    else if((c == '&') && luxor) {
       getting_plot = 0;
       edit_cal();
       return 1;
    }
    else if(c == '~') {  // deglitch plot
       getting_plot = 0;
       sprintf(edit_buffer, "%g", DEFAULT_DEGLITCH_SIGMA);
       sprintf(out, "Enter %s plot deglitch standard deviation sigma threshold. (ESC ESC to abort)", plot[selected_plot].plot_id);
       start_edit(DEGLITCH_CMD, out);
       return 1;
    }
    else if(c == '?') {
       show_all_stats ^= 1;
       erase_debug_info();
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
   if((c >= '0') && (c <= '9')) c -= '0';
   else if(c >= 'a') c = (c - 'a') + 10;
   else if(c >= 'A') c = (c - 'A') + 10;

   if((c < 0) || (c >= MAX_MARKER)) return;

   if(mark_q_entry[c]) {  // marker was set,  go to it
      goto_mark(c);
      hide_kbd(21);
   }
   else {  // mark current spot if it is not already marked
      for(val=1; val<MAX_MARKER; val++) {
         if(mark_q_entry[val] == last_mouse_q) {  // place is already marked
            BEEP(311);
            return;
         }
      }
      mark_q_entry[c] = last_mouse_q;
      draw_plot(REFRESH_SCREEN);
   }
}

int set_next_marker()
{
int val;

   // mark current mouse column with the next available marker number
   for(val=1; val<MAX_MARKER; val++) {
      if(mark_q_entry[val] == last_mouse_q) {  // place is already marked
         mark_q_entry[val] = 0;                // unmark it
         draw_plot(REFRESH_SCREEN);
         return 1;
      }
   }

   for(val=1; val<MAX_MARKER; val++) {
      if(mark_q_entry[val] == 0) {
         if(val < 10) last_was_mark = '0'+val;
         else         last_was_mark = 'A'+val-10;
         mark_q_entry[val] = last_mouse_q;  // last marker is mouse click marker
         draw_plot(REFRESH_SCREEN);
         return 1;
      }
   }

   BEEP(312);
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
static int save_count = 0;
static int restore_count = 0;

   if(save > 0) {
      old_watch = plot_watch;
      old_azel = plot_azel;
      old_share = shared_plot;
      old_sigs = plot_signals;
      old_lla = plot_lla;
      saved_config = save;
      ++save_count;
//sprintf(debug_text, "plot watch fix save# %d: %d: %d", save_count, save, plot_watch);
   }
   else if(saved_config) {
      ++restore_count;
//sprintf(debug_text2, "plot watch fix restore# %d: %d: pw:%d  ow:%d", restore_count, saved_config, plot_watch, old_watch);
      plot_watch = old_watch;
      plot_azel = old_azel;
      shared_plot = old_share;
      plot_signals = old_sigs;
      if(show_fixes == 0) {
         plot_lla = 0; // !!!!! old_lla;
      }
      else plot_lla = old_lla;

      no_redraw = 1;
      config_screen(130);
      no_redraw = 0;
//saved_config = 0;  // (555)
   }
}

void cancel_zoom(int why)
{
   // clear the zoomed screen flag
//sprintf(debug_text3, "zero zoom: %d  zs:%c", why, zoom_screen);
   zoom_screen = 0;
   show_rpn_help = 0;
   force_adev_redraw(105);
}

void remove_zoom()
{
   // exit a zoomed screen mode and re-config the screen
   change_zoom_config(-2);
   cancel_zoom(2);
   zoom_fixes = show_fixes;
   show_rpn_help = 0;
   config_screen(117);
   force_adev_redraw(106);
}

void change_zoom_config(int save)
{
static int old_watch = 0;
static int old_azel = 0;
static int old_share = 0;
static int old_sigs = 0;
static int old_lla = 0;
static int saved_config = 0;
static int save_count = 0;
static int restore_count = 0;

// return;
if(zoom_screen == 'K') return;

   if(save > 0) {
      ++save_count;
//sprintf(debug_text, "plot watch config save# %d: why:%d  pw:%d", save_count, save, plot_watch);
      old_watch = plot_watch;
      old_azel = plot_azel;
      old_share = shared_plot;
      old_sigs = plot_signals;
      old_lla = plot_lla;
      saved_config = save;
   }
   else if(saved_config) {
      ++restore_count;
//sprintf(debug_text2, "plot watch config restore# %d: why:%d  saved:%d %d: pw:%d  ow:%d", restore_count, save, saved_config, plot_watch, old_watch);
      plot_watch = old_watch;
      plot_azel = old_azel;
      shared_plot = old_share;
      plot_signals = old_sigs;
      if(1 && (show_fixes == 0)) {
         plot_lla = 0; // !!!!! old_lla;
      }
else if(show_fixes) plot_lla = 1;
      else plot_lla = old_lla;
//saved_config = 0;  // (555)
   }
}


void do_fixes(int mode)
{
   if(rcvr_type == NO_RCVR) return;

   show_fixes = toggle_value(show_fixes);
   change_fix_config(1);

   if(show_fixes && (precision_survey == 0)  && (trimble_save == 0)) {  // set reference position for fix map
      precise_lat = lat;
      precise_lon = lon;
      precise_alt = alt;
      have_precise_lla = (-8);
      if(user_set_ref_lla == 0) {
         ref_lat = lat;
         ref_lon = lon;
         ref_alt = alt;
      }
      cos_factor = cos(ref_lat);
      if(cos_factor == 0.0) cos_factor = 0.001;
   }

   if(show_fixes && (mode >= 0) && (mode <= 7)) {  // 3d fix mode
      start_3d_fixes(mode, 1);
      if(GRAPH_LLA && (graph_lla == 0)) {
         config_lla_plots(HIDE_LLA_ON_HOLD, KEEP_LLA_SHOW);
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
   reset_first_key(25);
   prot_menu = 0;
   config_screen(107);
}


void dump_image()
{
   dump_type = 's';
   #ifdef GIF_FILES
      sprintf(edit_buffer, "%s.gif", unit_file_name);
      if(top_line) start_edit(WRITE_CMD, "Enter name of .GIF or .BMP file for GRAPH image (ESC ESC to abort):");
      else         start_edit(WRITE_CMD, "Enter name of .GIF or .BMP file for SCREEN image (ESC ESC to abort):");
   #else
      sprintf(edit_buffer, "%s.bmp", unit_file_name);
      if(top_line) start_edit(WRITE_CMD, "Enter name of .BMP file for GRAPH image (ESC ESC to abort):");
      else         start_edit(WRITE_CMD, "Enter name of .BMP file for SCREEN image (ESC ESC to abort):");
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
      old_idle = idle_sleep;       // save current settings
      old_jit_adev = jitter_adev;
      old_poll = no_poll;

      old_plot_watch = plot_watch;
      old_plot_azel = plot_azel;
      for(i=0; i<NUM_PLOTS+DERIVED_PLOTS; i++) {  // disable all plots
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
//    no_poll = 1;                 // rrrrr messes up things like SCPI that must be polled
      new_queue(RESET_ALL_QUEUES, 63); // flush the queues
   }
   else {  // jitter measurements disabled, restore old settings, calculate histogram
      sprintf(log_text, "#");
      write_log_comment(1);
      sprintf(log_text, "#  Jitter measurement ADEVs");
      write_log_comment(1);
      sprintf(log_text, "#");
      write_log_comment(1);
      log_adevs();
      log_stats();

      idle_sleep = old_idle;
      jitter_adev = old_jit_adev;
      no_poll = old_poll;

      plot_watch = old_plot_watch;
      plot_azel = old_plot_azel;
      for(i=0; i<NUM_PLOTS+DERIVED_PLOTS; i++) {  // disable all plots
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
         if(queue_interval) val = ((double) q.data[MSGOFS] / (double) queue_interval);
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

   dont_reset_queues = 0;
   new_queue(RESET_ADEV_Q, 12);
}

void clear_all_data()
{
   dont_reset_queues = 0;
   new_queue(RESET_ALL_QUEUES, 13);

   #ifdef BUFFER_LLA
      clear_lla_points(1);
   #endif
   #ifdef SIG_LEVELS
      clear_signals();
   #endif
   #ifdef SAT_TRAILS
      clear_sat_trails();
   #endif

   if(view_all_data == 1) edit_user_view("A");
   else if(view_all_data == 2) edit_user_view("T");
   need_redraw = 5566;
}


void config_fix_display()
{
   change_fix_config(show_fixes);
   if(show_fixes && (precision_survey == 0) && (trimble_save == 0)) {  // set reference position for fix map
      precise_lat = lat;
      precise_lon = lon;
      precise_alt = alt;
      have_precise_lla = (-7);
      if(user_set_ref_lla == 0) {
         ref_lat = lat;
         ref_lon = lon;
         ref_alt = alt;
      }
      cos_factor = cos(ref_lat);
      if(cos_factor == 0.0) cos_factor = 0.001;
   }
   if(show_fixes) start_3d_fixes(-1, 2);
   if(1 || (GRAPH_LLA && (graph_lla == 0))) {
      config_lla_plots(KEEP_LLA_ON_HOLD, KEEP_LLA_SHOW);
   }

   config_screen(112);
   user_fix_set = 0;
}



int kbd_a_to_o(int c)
{
int i;
int old_type;
char *s;

   // process keyboard characters 'a' to 'o'

   if(force_si_cmd) {     // !!!!! royal kludge for G C T keyboard command
      force_si_cmd = 0;
      goto si_cmd;
   }
   else if(tide_kbd_cmd) { // !!!! royal kludge for G K x keyboard commands
      c = tide_kbd_cmd;
      tide_kbd_cmd = 0;
      first_key = 0;

//11=az  (Z)
//12=el  (E)
//13=sig level  (S)
      if     (c == 'Y') edit_plot(ELEVEN, 'Y');
      else if(c == 'X') edit_plot(TWELVE, 'X');
      else if(c == 'Z') {
         if(plot_prn) edit_plot(ELEVEN, 'Z');
         else         edit_plot(THIRTEEN, 'Z');
      }
      else if(c == 'E') edit_plot(TWELVE, 'G');
      else if(c == 'G') edit_plot(FOURTEEN, 'G');
      else if(c == 'H') edit_plot(HUMIDITY, 'G');
      else if(c == 'P') edit_plot(PRESSURE, 'G');
      else if(c == 'S') edit_plot(THIRTEEN, 'G');
      else if(c == 'T') edit_plot(TEMP1, 'G');
      else if(c == 'U') edit_plot(TEMP2, 'G');
      else if(c == 'A') {  // toggle all tide plots on/off
         if(enviro_mode()) {
            i = plot[HUMIDITY].show_plot + plot[PRESSURE].show_plot + plot[TEMP1].show_plot + plot[TEMP2].show_plot;
            if(i >= 4) {
               plot[HUMIDITY].show_plot = 0;
               plot[PRESSURE].show_plot = 0;
               plot[TEMP1].show_plot = 0;
               plot[TEMP2].show_plot = 0;
            }
            else {
               plot[HUMIDITY].show_plot = 1;
               plot[PRESSURE].show_plot = 1;
               plot[TEMP1].show_plot = 1;
               plot[TEMP2].show_plot = 1;
            }
         }
         else {
            i = plot[ELEVEN].show_plot + plot[TWELVE].show_plot + plot[THIRTEEN].show_plot + plot[FOURTEEN].show_plot;
            if(i >= 4) {
               plot[ELEVEN].show_plot = 0;
               plot[TWELVE].show_plot = 0;
               plot[THIRTEEN].show_plot = 0;
               plot[FOURTEEN].show_plot = 0;
            }
            else {
               plot[ELEVEN].show_plot = 1;
               plot[TWELVE].show_plot = 1;
               plot[THIRTEEN].show_plot = 1;
               plot[FOURTEEN].show_plot = 1;
            }
         }
      }
      else if(c == 'I') {  // toggle lat/lon tide scattergram plot
         if(zoom_screen == 'K') {
            change_zoom_config(-104);
            cancel_zoom(14);   //zkzk
         }
         show_fixes = toggle_value(show_fixes);
         show_tides = show_fixes;
         rebuild_lla_plot(0);
         user_set_ref_lla = 0;
         config_fix_display();
      }
      return 0;
   }

   // Process keyboard characters a..o
   if(c == 'a') {
      if(first_key == 'c') {  // CA command - clear adev queues
         if(luxor) return help_exit(c,99);
         dont_reset_queues = 0;
         dont_reset_phase = 1;
         new_queue(RESET_ADEV_Q, 14);
         dont_reset_phase = 0;
//       ticc_packets = 0;
      }
      else if(first_key == 'f') {  // FA command - toggle ALT filter
         if(luxor) return help_exit(c,99);
         alt_filter = toggle_value(alt_filter);
         set_filter_config(pv_filter, static_filter, alt_filter, kalman_filter, marine_filter, 1);
      }
      else if(first_key == 'l') {  // LA command - open log in append mode
         if(log_file) {
            strcpy(edit_buffer, "Y");
            sprintf(out, "Log file %s is already open.  Close it? (Y/N)  (ESC ESC to abort)", log_name);
            start_edit(LOG_CLOSE_CMD, out);
            return 0;
         }
         else {
            log_mode = "a";
            strcpy(edit_buffer, log_name);
            edit_info2 = "Append an '*' to the file name to flush file after every write";  // FLUSH_CHAR
            edit_info1 = "Use file extension .gpx .xml or .kml to write XML format logs.";
            start_edit(LOG_CMD, "Enter log file name to append data to: ");
            return 0;
         }
      }
      else if(first_key == 'm') {  // MA command - set RINEX antenna type
         strcpy(edit_buffer, antenna_type);
         start_edit(RINEX_ANT_TYPE_CMD, "Enter the antenna type (max 20 chars) (ESC ESC to abort)");
         return 0;
      }
      else if(first_key == 'p') {  // PA command - set PPS1 params
         if(luxor) return help_exit(c,99);
         if(rcvr_type == CS_RCVR) {
            sprintf(edit_buffer, "%.0lf", cs_freq1);
            edit_info1 = "Valid frequencies are 5 or 10 MHz";
            start_edit(PPS1_CFG_CMD, "Enter output 1 frequency  (ESC ESC to abort):");
         }
         else if(rcvr_type == ESIP_RCVR) {
            if(have_pps_duty) sprintf(edit_buffer, "%d", esip_pps_width);
            else              sprintf(edit_buffer, "%d", 200);
            start_edit(PPS1_CFG_CMD, "Enter PPS pulse width in msecs (1..500) (ESC ESC to abort):");
         }
         else if(rcvr_type == NVS_RCVR) {
            sprintf(edit_buffer, "%d", nvs_pps_width);
            start_edit(PPS1_CFG_CMD, "Enter PPS [duty_cycle <1] or [nanoseconds 50..2520500]  (ESC ESC to abort):");
         }
         else if(rcvr_type == PRS_RCVR) { // PA - adjust freerunning frequency
            sprintf(edit_buffer, "%d", prs_sf);
            edit_info1 = "Note: This setting is lost after power cycles or resets!";
            edit_info1 = "      Also this setting is not used when the 1PPS input is used";
            start_edit(PRS_SF_CMD, "Adjust frequency in units of 1E-12 parts (10 uHz) (-2000 .. 2000)  (ESC ESC to abort)");
            return 0;
         }
         else if(rcvr_type == SRO_RCVR) {
            edit_info1 = "Allowable values are 0 .. 34000 ns";
            edit_info2 = "Window granularity is 133.333 ns";
            sprintf(edit_buffer, "%g", sro_aw*SRO_TICK);
            start_edit(SRO_ALARM_CMD, "Enter alarm window half_width in ns (ESC ESC to abort):");
            return 0;
         }
         else if(rcvr_type == SS_RCVR) {
            if(have_pps_duty) sprintf(edit_buffer, "%d", (int) (ss_pps_width/1000.0));
            else              sprintf(edit_buffer, "%d", 1000);
            start_edit(PPS1_CFG_CMD, "Enter PPS pulse width in usec (1..65000) (ESC ESC to abort):");
         }
         else if(rcvr_type == TM4_RCVR) {
            sprintf(edit_buffer, "%d", tm4_antenna);
            start_edit(ANTENNA_CMD, "Antenna monitoring (0=disable, 1=emable)  (ESC ESC to abort):");
         }
         else if(rcvr_type == TRUE_RCVR) {
            sprintf(edit_buffer, "%d", (int) atten_val);
            start_edit(ATTEN_CMD, "Enter attenuator value (ESC ESC to abort):");
         }
         else if(rcvr_type == VENUS_RCVR) {
            if((rcvr_type == VENUS_RCVR) && saw_timing_msg) return help_exit(c,99);
            if(!have_pps_freq) edit_info1 = "This receviver can only adjust the pulse width.";
            sprintf(edit_buffer, "%.0lf %.6lf", pps1_freq, pps1_duty);
            start_edit(PPS1_CFG_CMD, "Enter PPS1 [frequency]  [duty_cycle <1 or microseconds 1..100000]  (ESC ESC to abort):");
         }
         else if(rcvr_type == X72_RCVR) { // PA - ACMOS output control
            if(x72_creg & 0x0008) sprintf(edit_buffer, "1");
            else                  sprintf(edit_buffer, "0");
            edit_info1 = " ";
            edit_info2 = "Enabling a high frequency ACMOS output can degrade the quality of the";
            edit_info3 = "sine output particularly if they are harmonically related.";
            start_edit(X72_ACMOS_ENAB_CMD, "Set ACMOS output  (1=enable 0=disable) (ESC ESC to abort)");
            return 0;
         }
         else if(rcvr_type == ZYFER_RCVR) {
            if(have_pps_duty) sprintf(edit_buffer, "%.0f", zyfer_pps_width);
            else              sprintf(edit_buffer, "%.0f", -2.0);
            edit_info1 = "Use -1 for 10 us  or -2 for 20 us";
            start_edit(PPS1_CFG_CMD, "Enter PPS pulse width in msecs (1..999) (ESC ESC to abort):");
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
         start_edit(SIGNAL_CMD, "A)zimuth  W)eighted azimuth  E)levation  S)ignals  D)ata  C)lear  <cr>=off");
         if(plot_watch && plot_azel) {
            shared_plot = 1;
            prot_menu = 0;
         }
         return 0;
      }
      else if(first_key == 't') { // TA command - set alarm clock time
         if     (script_file) edit_buffer[0] = 0;
         else if(alarm_time && alarm_date) sprintf(edit_buffer, "%02d:%02d:%02d  %04d/%02d/%02d", alarm_hh,alarm_mm,alarm_ss, alarm_year,alarm_month,alarm_day);
         else if(alarm_time) sprintf(edit_buffer, "%02d:%02d:%02d", alarm_hh,alarm_mm,alarm_ss);
         else if(alarm_date) sprintf(edit_buffer, "%04d/%02d/%02d", alarm_year,alarm_month,alarm_day);
         else if(egg_val) {
            if((egg_val >= (24L*60L*60L)) && ((egg_val % (24L*60L*60L)) == 0)) sprintf(edit_buffer, "%ld d", egg_val/(24L*3600L));
            else if((egg_val >= (3600L)) && ((egg_val % 3600L) == 0)) sprintf(edit_buffer, "%ld h", egg_val/(3600L));
            else if((egg_val >= (60L)) && ((egg_val % 60L) == 0)) sprintf(edit_buffer, "%ld m", egg_val/(60L));
            else sprintf(edit_buffer, "%ld s", egg_val);
            if(repeat_egg) strcat(edit_buffer, " R");
         }
         else edit_buffer[0] = 0;

         if(modem_alarms) strcat(edit_buffer, " A");

         edit_info1 = "Dates are in the format mm/dd/yyyy or yyyy/mm/dd";
         edit_info2 = "Intervals can be in seconds, minutes, hours or days like: 7s, 10m, 2h, 4d";
         if(edit_buffer[0]) start_edit(ALARM_CMD, "Enter alarm clock time (and optional date) or interval or <ESC CR> to reset:");
         else               start_edit(ALARM_CMD, "Enter alarm clock time (and optional date) or interval or <CR> to reset:");
         return 0;
      }
      else if(first_key == 'w') {  // WA command - Output all data to a log file
         strcpy(edit_buffer, "dump.log");
         sprintf(out, "Enter name of log file to write ALL %squeue info to (ESC ESC to abort):",
                       filter_log?"filtered ":"");
         start_edit(WRITE_CMD, out);
         dump_type = 'a';
         log_mode = "w";
         return 0;
      }
      else if(first_key == 'z') {  // ZA command - zoom antenna aziumuth map
         if(rcvr_type == NO_RCVR) return sure_exit();
         change_zoom_config(6);
         un_zoom = 0;
         zoom_screen = 'S';
         zoom_fixes = show_fixes;
         plot_signals = 2;
         config_screen(120);
      }
      else if(first_key == '&') {  // &A command - autotune osc params
         if(luxor) return help_exit(c,99);
         if(rcvr_type == TRUE_RCVR) {
            sprintf(edit_buffer, "%d", 0);
            edit_info1 = " ";
            edit_info2 = "THIS COMMAND WILL RESET THE UNIT";
            start_edit(TRUE_TUNE_CMD, "Train OCXO EFC characteristics (0=NO 1=YES)  (ESC ESC to abort)");
            return 0;
         }
         else if(rcvr_type == TICC_RCVR) {
            sprintf(edit_buffer, "%d", ticc_tune_time);
            edit_info1 = " ";
            edit_info2 = "Connect a 1PPS signal to chA and chB inputs through matched cables";
            edit_info3 = "THIS COMMAND WILL RESET THE DATA QUEUES";
            start_edit(TICC_TUNE_CMD, "Enter number of seconds to run TIME2 / FUDGE autotune  (ESC ESC to abort)");
            return 0;
         }
         else if(rcvr_type == X72_RCVR) {
            sprintf(edit_buffer, "%d", ticc_tune_time);
            edit_info1 = " ";
            edit_info2 = "Connect a 1PPS signal to device 1PPS input";
            edit_info3 = "THIS COMMAND WILL RESET THE DATA QUEUES and DDS tune word";
            start_edit(X72_TUNE_CMD, "Enter number of seconds to run DDS autotune. 0=cancel  (ESC ESC to abort)");
            return 0;
         }
         else {
            dac_dac = 1;
         }
      }
      #ifdef ADEV_STUFF
         else if(first_key == 'a') {  // AA command - set adev type to ADEV
            if(luxor) return help_exit(c,99);
            ATYPE = OSC_ADEV;
            last_atype = ATYPE;
            if(rcvr_type == TICC_RCVR) all_adevs = ALL_CHANS;
            else                       all_adevs = SINGLE_ADEVS;
            plot_adev_data = 1;
            force_adev_redraw(107);
            config_screen(108);
            if(adevs_active(0)) last_was_adev = c;
         }
         else if(first_key == 'g') {
            if(luxor) {  // GA command - toggle ADEV plot
               show_prots = toggle_value(show_prots);
            }
            else {  // GA command - toggle ADEV plot
               plot_adev_data = toggle_value(plot_adev_data);
               draw_plot(REFRESH_SCREEN);
               if(adevs_active(0)) last_was_adev = c;
               if(plot_adev_data == 0) adev_decades_shown = 0;
            }
         }
         else if(luxor) {  // A commands
            return help_exit(c,1);
         }
         else if(first_key == 0) {  // xxxxxx
            if(adevs_active(0)) {
               last_was_adev = 'A';
            }
            if(are_you_sure(c) != c) return 0;
         }
         else {
            return help_exit(c,666);
         }
      #else
         else return help_exit(c,1);
      #endif
   }
   else if(c == 'b') {
      if(first_key == 'c') {  // CB command - clear both queues
//!!!!-  pause_data = 0;
         if(luxor) return help_exit(c,99);
         dont_reset_queues = 0;
         new_queue(RESET_ALL_QUEUES, 15);
//       ticc_packets = 0;
      }
      else if(first_key == 'g') {
         if(rcvr_type == CS_RCVR) {  // GB command - beam current
            edit_plot(TEN, c);
            return 0;
         }
         else { // GB command - satellite map, shared with plot area
            share_it();
         }
      }
      else if(first_key == 'm') {  // MB command - set v3.xx Beidou observation list
         strcpy(edit_buffer, beidou_obs_list);
         rinex_list = c;
         edit_info1 = "Enter a blank line for auto-detect.";
         start_edit(RINEX_LIST_CMD, "Enter the RINEX v3.xx BEIDOU observation list (ESC ESC to abort)");
         return 0;
      }
      else if(first_key == 'p') {  // PB command - set PPS2 params
         if(luxor) return help_exit(c,99);
         if(rcvr_type == VENUS_RCVR) {
            if(saw_timing_msg == 0) return help_exit(c,99);
            sprintf(edit_buffer, "%.0lf", pps2_freq);
            start_edit(PPS2_CFG_CMD, "Enter PPS2 frequency (max 19.2 MHz)  (ESC ESC to abort):");
         }
         else if(rcvr_type == CS_RCVR) {
            sprintf(edit_buffer, "%.0lf", cs_freq2);
            edit_info1 = "Valid frequencies are 5 or 10 MHz";
            start_edit(PPS2_CFG_CMD, "Enter output 2 frequency  (ESC ESC to abort):");
         }
         else if(rcvr_type == ESIP_RCVR) {
            sprintf(edit_buffer, "%.0lf %.3lf", pps2_freq, pps2_duty);
            edit_info1 = "Allowable freq range is 4000 to 40E6 Hz,  0=disable GCLK";
            edit_info2 = "Allowabe duty cycle is 0.10 to 0.90  (default 0.50)";
            start_edit(PPS2_CFG_CMD, "Enter GCLK [frequency]  [duty_cycle <1 or nanoseconds >=1]  (ESC ESC to abort):");
         }
         else if(rcvr_type == TRUE_RCVR) {  // board delay
            sprintf(edit_buffer, "%.1f", pps1_delay*1.0E9);
            start_edit(PPS_OFS_CMD, "Enter board delay value (-32 .. 32) (ESC ESC to abort):");
         }
         else if(rcvr_type == UBX_RCVR) {
            sprintf(edit_buffer, "%.0lf %.3lf", pps2_freq, pps2_duty);
            edit_info1 = "PPS2 is limited to 1000 Hz on older model receivers";
            start_edit(PPS2_CFG_CMD, "Enter PPS2 [frequency]  [duty_cycle <1 or microseconds >=1]  (ESC ESC to abort):");
         }
         else {
            return help_exit(c,99);
         }
         return 0;
      }
      else if(first_key == 's') {  // SB command - blink a sat in the sat map
         if(blink_prn == BLINK_HIGHEST) strcpy(edit_buffer, "H");
         else if(blink_prn == BLINK_PLOT) strcpy(edit_buffer, "P");
         else sprintf(edit_buffer, "%d", blink_prn);
         edit_info1 = "H=highest sat   P=plot (SQ) prn   1..999=sat prn";
         start_edit(BLINK_PRN_CMD, "Enter satellite PRN to blink (0=none)  (ESC ESC to abort):");
         return 0;
      }
      else if(first_key == 't') {  // TB command - show calendar, user specified year
         sprintf(edit_buffer, "%d %d", pri_year, pri_month);
         edit_info1 = "For months: 1=January .. 12=December";
         start_edit(SHOW_CALENDAR_CMD, "Enter year (and optional month) to display calendar for");
         return 0;
      }
      else if(first_key == '!') {    // !B command - send break command to the receiver
         SetDtrLine(RCVR_PORT, 1);
         SendBreak(RCVR_PORT);
         redraw_screen();
      }
      else if(first_key == 'z') {  // ZB command - zoom watch and map
         change_zoom_config(2);
         un_zoom = 0;
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
barf:
            sprintf(edit_buffer, "K %f %f %f %f", OSC_P_GAIN, OSC_D_TC, OSC_FILTER_TC, OSC_I_TC);
            edit_info1    = "Custom OSC PID: K [proportional_gain] [derivative_tc] [filter_tc] [integrator_tc]";
            start_edit(OSCPID_CMD, "OSC CTRL PID:   W)slow  N)medium  X)fast  Y)very fast  0)off  1)on");
            return 0;
         }
         else if(rcvr_type == X72_RCVR) { // B command - user discipline algorithm
goto barf;
            x72_user_dis ^= 1;
            if(x72_user_dis) set_x72_discipline(3); // disable hw discipline, enable sw discipline
            else             set_x72_discipline(0); // disable all X72 disciplining
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
      #ifdef ADEV_STUFF
         else if(first_key == 'a') {  // AC command - graph all chC adev types
            if(luxor) return help_exit(c,99);
            if(ATYPE == A_MTIE) ATYPE = last_atype;
            aa_val = ALL_CHC;
            strcpy(edit_buffer, "G");
            start_edit(ADEV_CMD, "Display chC:   A)devs only   G)raphs and all adevs   graphs and R)egular adevs");
            return 0;
         }
      #endif
      else if(first_key == 'c') {  // CC command - clear everything
//!!!!-  pause_data = 0;
         clear_all_data();
//       ticc_packets = 0;
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
         else if(rcvr_type == CS_RCVR) {  // GC command - C-field
            edit_plot(ELEVEN, c);
            return 0;
         }
         else {       // GC command - satellite constellation displays
            if(sort_by) {
               sprintf(edit_buffer, "%c", sort_by);
               if(sort_ascend) strcat(edit_buffer, "+");
               else            strcat(edit_buffer, "-");
            }
            else strcpy(edit_buffer, "P+");

            out[0] = 0;
            if(have_accu) {
               if(out[0] == 0) strcat(out, "         ");
               strcat(out, "U)ra      ");
            }
            if(have_bias) {
               if(out[0] == 0) strcat(out, "         ");
               strcat(out, " B)ias    ");
            }
            if(have_state) {
               if(out[0] == 0) strcat(out, "         ");
               strcat(out, "M)state   ");
            }
            if(have_phase) {
               if(out[0] == 0) strcat(out, "         ");
               strcat(out, "W)phase   ");
            }
            if(have_range) {
               if(out[0] == 0) strcat(out, "         ");
               strcat(out, "  pseudoR)ange");
            }

            if(out[0]) {
               edit_info1 = &out[0];
               edit_info2 =         "         Include a '+' to force ascending sort or '-' for descending";
               edit_info3 =         "Count:   G)raph    T)racked sats     C)onstellation changes";
               if(have_range || have_phase) {
                  edit_info4 =         "         X)show raw measurement data";
               }
            }
            else {
               edit_info1 =         "         Include a '+' to force ascending sort or '-' for descending";
               edit_info2 =         "Count:   G)raph    T)racked sats     C)onstellation changes";
               if(have_range || have_phase) {
                  edit_info3 =         "         X)show raw measurement data";
               }
            }

            if(have_doppler) start_edit(SORT_CMD, "Sort by: A)zimuth  E)levation  P)RN  S)ignal  D)oppler  (ESC ESC to abort)");
            else             start_edit(SORT_CMD, "Sort by: A)zimuth  E)levation  P)RN  S)ignal  (ESC ESC to abort)");
            return 0;
         }
      }
      else if(first_key == 'l') {  // LC command - toggle logging of sat constellation data
         if(luxor) return help_exit(c,99);
         log_db ^= 1;
      }
      else if(first_key == 'm') {  // MC command - toggle deriving RINEX file L1 pseudorange from carrier phase data
         if(use_rinex_cppr) strcpy(edit_buffer, "Y");
         else               strcpy(edit_buffer, "N");
         start_edit(RINEX_CPPR_CMD, "Derive L1 pseudorange output from carrier phase data (Y/N).  (ESC ESC to abort):");
         return 0;
      }
      else if(luxor && (first_key == 'p')) {  // PC command = load overcurrent
         sprintf(edit_buffer, "%f", batt_ovc);
         start_edit(PC_CMD, "Enter the battery overcurrent cutoff in amps.  (ESC ESC to abort):");
         return 0;
      }
      else if((rcvr_type == CS_RCVR) && (first_key == 'p')) { // PC command - reset continuous operation lamp
         request_warm_reset();
         redraw_screen();
      }
      else if((rcvr_type == SRO_RCVR) && (first_key == 'p')) { // PC command - set clock
         set_sro_clock();
      }
      else if((rcvr_type == TICC_RCVR) && (first_key == 'p')) {  // PC command = TICC cal periods
         sprintf(edit_buffer, "%d", ticc_cal);
         edit_info1 = " ";
         edit_info2 = "THIS WILL RESET THE DATA QUEUES";
         if(ticc_type == TAPR_TICC) edit_info3 = "This command can take a few seconds...";
         start_edit(TICC_CAL_CMD, "Enter the TICC cal period count (2,10,20,40)  (ESC ESC to abort):");
         return 0;
      }
      else if((rcvr_type == X72_RCVR) && (first_key == 'p')) {  // PC command = set EFC input enable
         sprintf(edit_buffer, "%d", x72_efc);
         start_edit(X72_EFC_CMD, "Configure the EFC input (0=disable  1=enable)  (ESC ESC to abort):");
         return 0;
      }
      else if(first_key == 'p') {  // PC command - set cable delay
         edit_buffer[0] = 0;
         edit_info1 = "Example: enter 50f for 50 feet of 0.66 velocity factor coax";
         if(rcvr_type == RFTG_RCVR) {
            edit_info2 = "Note: should be at least 50 feet / 77 ns";
         }
         else if((rcvr_type == STAR_RCVR) && (star_type == OSA_TYPE)) {
            edit_info2 = "Note: MUST use POSITIVE values to compensate for cable delay!";
         }
         else if(rcvr_type == STAR_RCVR) {
            edit_info2 = "Note: MUST use NEGATIVE values to compensate for cable delay!";
         }
         else if(rcvr_type == TM4_RCVR) {
            edit_info2 = "Note: Use POSITIVE values to compensate for cable delay!";
         }
         else if(rcvr_type == TSIP_RCVR) {
            edit_info2 = "Note: use NEGATIVE values to compensate for cable delay!";
         }
         start_edit(CABLE_CMD, "Enter cable delay in ns or use #feet or #meters (ESC ESC to abort):");
         return 0;
      }
      else if(first_key == 't') { // TC command - set program run time
         if(no_exec) {
            edit_error("File execution has been disabled!");
         }
         else {
            if     (script_file) edit_buffer[0] = 0;
            else if(exec_time && exec_date) sprintf(edit_buffer, "%02d:%02d:%02d  %04d/%02d/%02d", exec_hh,exec_mm,exec_ss, exec_year,exec_month,exec_day);
            else if(exec_time) sprintf(edit_buffer, "%02d:%02d:%02d", exec_hh,exec_mm,exec_ss);
            else if(exec_date) sprintf(edit_buffer, "%04d/%02d/%02d", exec_year,exec_month,exec_day);
            else if(exec_val) {
               if((exec_val >= (24L*60L*60L)) && ((exec_val % (24L*60L*60L)) == 0)) sprintf(edit_buffer, "%ld d", exec_val/(24L*3600L));
               else if((exec_val >= (3600L)) && ((exec_val % 3600L) == 0)) sprintf(edit_buffer, "%ld h", exec_val/(3600L));
               else if((exec_val >= (60L)) && ((exec_val % 60L) == 0)) sprintf(edit_buffer, "%ld m", exec_val/(60L));
               else sprintf(edit_buffer, "%ld s", exec_val);
            }
            else edit_buffer[0] = 0;

            edit_info1 = "Dates are in the format mm/dd/yyyy or yyyy/mm/dd";
            edit_info2 = "Intervals can be in seconds, minutes, hours or days like: 7s, 10m, 2h, 4d";
            if(edit_buffer[0]) start_edit(EXEC_PGM_CMD, "Enter program run time (and optional date) or interval or <ESC CR> to reset:");
            else               start_edit(EXEC_PGM_CMD, "Enter program run time (and optional date) or interval or <CR> to reset:");
            return 0;
         }
      }
      else if(luxor && (first_key == 'w')) {  // WC command = write config data
         sprintf(edit_buffer, "LUXCFG.SCR");
         start_edit(WC_CMD, "Enter config data script file to write (ESC ESC to abort)");
         return 0;
      }
      else if(first_key == 'z') {  // ZC command - zoom clock
         change_zoom_config(3);
         un_zoom = 0;
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
         set_discipline_mode(SET_DIS_MODE_DISABLE);
         need_redraw = 1206;
      }
      #ifdef ADEV_STUFF
         else if(first_key == 'a') {  // AD command - graph all chD adev types
            if(luxor) return help_exit(c,99);
            if(ATYPE == A_MTIE) ATYPE = last_atype;
            aa_val = ALL_CHD;
            strcpy(edit_buffer, "G");
            start_edit(ADEV_CMD, "Display chD:   A)devs only   G)raphs and all adevs   graphs and R)egular adevs");
            return 0;
         }
      #endif
      else if(first_key == 'g') { // GD command - toggle DAC graph
         edit_plot(DAC, c);
         return 0;
      }
      else if(first_key == 'f') { // FD command - toggle display filter
         if(filter_count) strcpy(edit_buffer, "0");
         else if(rcvr_type == CS_RCVR) strcpy(edit_buffer, "180");
         else  strcpy(edit_buffer, "10");
         edit_info1 = "Enter -1 for \"per pixel\" filter size based upon the view interval and plot width";
         start_edit(FILTER_CMD, "Enter number of plot queue points to average (0=filter off) (ESC ESC to abort):");
         return 0;
      }
      else if(first_key == 'l') { // LD command - delete file
         strcpy(edit_buffer, log_name);
         goto delete_file;
      }
      else if(first_key == 'p') {   // PD command - disable PPS signal
         if(rcvr_type == TICC_RCVR) {  // Time dilation aaaacccc
            sprintf(edit_buffer, "%d %d", dilat_a,dilat_b);
            edit_info1 = " ";
            edit_info2 = "THIS WILL RESET THE DATA QUEUES";
            if(ticc_type == TAPR_TICC) edit_info3 = "This command can take a few seconds...";
            start_edit(TICC_DILAT_CMD, "Enter the chA and chB TIME DILATION values (ESC ESC to abort)");
            return 0;
         }
         else if(rcvr_type == CS_RCVR) { // PD - set display enable
            sprintf(edit_buffer, "%d", cs_disp ^ 1);
            start_edit(CS_DISP_CMD, "Set diaplay enable  (ESC ESC to abort)");
            return 0;
         }
         else if(rcvr_type == PRS_RCVR) {
            start_edit(PPS_OFS_CMD, "Enter PPS position offset in ns (ESC ESC to abort):");
            return 0;
         }
         else if(rcvr_type == SA35_RCVR) {
            set_sa35_efc(0);
         }
         else if(rcvr_type == SRO_RCVR) {
            edit_info1 = "Allowable values are 0 .. 999999999 ns";
            edit_info2 = "Pulse delay granularity is 133.333 ns";
            sprintf(edit_buffer, "%g", sro_de*SRO_TICK);
            start_edit(SRO_DELAY_CMD, "Enter PPS position offset in ns (ESC ESC to abort):");
            return 0;
         }
         else if((rcvr_type == STAR_RCVR) && (star_type == OSA_TYPE)) {
            start_edit(PPS_OFS_CMD, "Enter PPS position offset in ns (0 .. 999999999) (ESC ESC to abort):");
            return 0;
         }
         else {
            user_pps_enable = 0;
            set_pps(user_pps_enable, pps_polarity,  delay_value, pps1_delay,  pps_threshold, 4);
            request_pps_info();
         }
      }
      else if(first_key == 's') {  // SD command - calculate propogation delay
         if(have_prop_lla == 0) {
            prop_lat = lat;
            prop_lon = lon;
            prop_alt = iono_height();
         }

         sprintf(edit_buffer, "%.8lf %.8lf %.3lf", prop_lat*RAD_TO_DEG, prop_lon*RAD_TO_DEG, prop_alt);
         edit_info5 = "MIKES    MSF  PPE  RBU  RTZ  RUGBY  RWM  TDF  WWV  WWVB    WWVH";
         edit_info4 = "ANTHORN  BBC  BPM  CHU  DCF  EBC    HBG  HLA  JJY  KYUSHU  LOL";
         edit_info3 = "The lat/lon values can also be a station name:";
         edit_info2 = "The lat/lon values can be a decimal number or like 10d20m30s";
         edit_info1 = "The ionosphere altiude value is in km (winter:250  summer:350)";
         start_edit(PROP_DELAY_CMD, "Enter lat lon ionosphere of far station (-=S,W +=N,E)  (ESC ESC to abort):");
         return 0;
      }
      else if(first_key == 't') { // TD command - set screen dump time
         if     (script_file) edit_buffer[0] = 0;
         else if(dump_time && dump_date) sprintf(edit_buffer, "%02d:%02d:%02d  %04d/%02d/%02d", dump_hh,dump_mm,dump_ss, dump_year,dump_month,dump_day);
         else if(dump_time) sprintf(edit_buffer, "%02d:%02d:%02d", dump_hh,dump_mm,dump_ss);
         else if(dump_date) sprintf(edit_buffer, "%04d/%02d/%02d", dump_year,dump_month,dump_day);
         else if(dump_val) {
            if((dump_val >= (24L*60L*60L)) && ((dump_val % (24L*60L*60L)) == 0)) sprintf(edit_buffer, "%ld d", dump_val/(24L*3600L));
            else if((dump_val >= (3600L)) && ((dump_val % 3600L) == 0)) sprintf(edit_buffer, "%ld h", dump_val/(3600L));
            else if((dump_val >= (60L)) && ((dump_val % 60L) == 0)) sprintf(edit_buffer, "%ld m", dump_val/(60L));
            else sprintf(edit_buffer, "%ld s", dump_val);
         }
         else edit_buffer[0] = 0;

         edit_info1 = "Dates are in the format mm/dd/yyyy or yyyy/mm/dd";
         edit_info2 = "Intervals can be in seconds, minutes, hours or days like: 7s, 10m, 2h, 4d";
         if(edit_buffer[0]) start_edit(SCREEN_DUMP_CMD, "Enter screen dump time (and optional date) or interval or <ESC CR> to reset:");
         else               start_edit(SCREEN_DUMP_CMD, "Enter screen dump time (and optional date) or interval or <CR> to reset:");
         return 0;
      }
      else if(first_key == 'w') { // WD command - delete file
         edit_buffer[0] = 0;
         delete_file:
         start_edit(WRITE_CMD, "Enter name of file to delete (ESC to abort):");
         dump_type = 'd';
         return 0;
      }
      else if(first_key == 'z') {  // ZD command - zoom antenna data map
         if(rcvr_type == NO_RCVR) return sure_exit();
         change_zoom_config(6);
         un_zoom = 0;
         if(rcvr_type == BRANDY_RCVR) {
            zoom_screen = 'D';
         }
         else if(rcvr_type == CS_RCVR) {
            zoom_screen = 'D';
         }
         else if(rcvr_type == LPFRS_RCVR) {
            zoom_screen = 'D';
         }
         else if(rcvr_type == PRS_RCVR) {
            zoom_screen = 'D';
         }
         else if(rcvr_type == SA35_RCVR) {
            zoom_screen = 'D';
         }
         else if(rcvr_type == SRO_RCVR) {
            zoom_screen = 'D';
         }
         else if(rcvr_type == RFTG_RCVR) {
            zoom_screen = 'D';
         }
         else if(rcvr_type == X72_RCVR) {
            zoom_screen = 'D';
         }
         else {
            zoom_screen = 'S';
            zoom_fixes = show_fixes;
            plot_signals = 5;
         }
         config_screen(120);
      }
      else if(first_key == '$') {  // $D command - very very small 480x320 screen
         new_screen(c);
         return 0;
      }
      else if(first_key == '&') { // &D command - change damping factor
         if(luxor) return help_exit(c,99);
         if(rcvr_type == PRS_RCVR) {
            sprintf(edit_buffer, "%d", prs_pf);
            start_edit(DAMP_CMD, "Enter PLL stabiltiy factor (0 .. 4)  (ESC ESC to abort)");
         }
         else if(rcvr_type == X72_RCVR) {
//          sprintf(edit_buffer, "%f", x72_damping_val);
            sprintf(edit_buffer, "%f", user_damping_factor);
            start_edit(DAMP_CMD, "Enter damping factor (0.25 .. 4.0) (ESC ESC to abort):");
            return 0;
         }
         else {
            sprintf(edit_buffer, "%f", user_damping_factor);
            start_edit(DAMP_CMD, "Enter damping factor (ESC ESC to abort):");
            return 0;
         }
      }
      else if(first_key == '!') {    // !D command - do receiver diagnostice
         if((rcvr_type == CS_RCVR) && (cs_standby == 0)) {
            erase_screen();
            edit_error("Device must be in standby mode to run self tests");
         }
         else if(rcvr_type == RFTG_RCVR) {  // disable RFTG-m unit
            rftg_disable_cpu();
         }
         else {
            request_self_tests(-1);
            redraw_screen();
         }
      }
      else {   // D command - toggle oscillator disciplining
         if(luxor) return help_exit(c, 99);
         else if(first_key == 0) {  // xxxxxx
            if(are_you_sure(c) != c) return 0;
         }
         else return help_exit(c,666);
         // next call to do_kbd() will process the discipline selection character
      }
   }
   else if(c == 'e') {
      if(first_key == 'a') {  // AE command - toggle ADEV plot error bars
         show_error_bars = toggle_value(show_error_bars);
         draw_plot(REFRESH_SCREEN);
         if(adevs_active(0)) last_was_adev = c;
      }
      else if(first_key == 'c') {  // CE command - clear error history
         clear_eeprom_data();
      }
      else if(first_key == 'd') { // DE command - enable osc discipline
         if(luxor) return help_exit(c,99);
         osc_discipline = 1;
         set_discipline_mode(SET_DIS_MODE_ENABLE);
         need_redraw = 1207;
      }
      else if(first_key == 'e') { // EE command - save eeprom segments
         if(luxor) return help_exit(c,99);
         if(rcvr_type == PRS_RCVR) {
            strcpy(edit_buffer, last_prs_set);
            edit_info1 = "Valid params: LM FC GA SP MO TO PL PF PT";
            edit_info2 = "or enter ALL to save all params";
            start_edit(PRS_SAVE_CMD, "Enter the two letter paramter to save to EEPROM (ESC ESC to abort)");
            return 0;
         }
         else if(rcvr_type == SRO_RCVR) {  // EE command - freq save mode
            sprintf(edit_buffer, "%d", sro_fs);
            edit_info1 = "0=OFF  1=DAILY   2=INTEGRAL PART NOW    3=USER FREQ NOW";
            start_edit(SRO_FS_CMD, "Set frequency save mode  (ESC ESC to abort)");
            return 0;
         }
         else if(rcvr_type == VENUS_RCVR) {
            eeprom_save = toggle_value(eeprom_save);
         }
         else {
            save_segment(0xFF, 1);
         }
      }
      else if(first_key == 'f') {  // FE command - elevation mask
         if(luxor) return help_exit(c,99);
         sprintf(edit_buffer, "%.1f", el_mask);
         if(rcvr_type == TM4_RCVR) {
            edit_info1 = "Supported elevation masks: 5,15,20 degrees";
         }
         start_edit(ELEV_CMD,  "Enter minimum acceptible satellite elevation (in degrees,  (ESC CR=best)");
         return 0;
      }
      else if(first_key == 'g') {  // GE command - toggle error graph
         plot_skip_data = toggle_value(plot_skip_data);
         draw_plot(REFRESH_SCREEN);
      }
      else if(first_key == 'h') {  // HE command - enter holdover
         if(rcvr_type == PRS_RCVR)      set_discipline_mode(SET_DIS_MODE_NORMAL);
         else if(rcvr_type == X72_RCVR) set_discipline_mode(SET_DIS_MODE_NORMAL);
         else                           set_discipline_mode(SET_DIS_MODE_HOLDOVER);
      }
      else if(first_key == 'p') {   // PE command - enable PPS signal
         if(rcvr_type == TICC_RCVR) {  // aaaaacccc
            sprintf(edit_buffer, "%c %c", edge_a, edge_b);
            edit_info1 = " ";
            edit_info2 = "THIS WILL RESET THE DATA QUEUES";
            if(ticc_type == TAPR_TICC) edit_info3 = "This command can take a few seconds...";
            start_edit(TICC_EDGE_CMD, "Enter channel A and channel B edge selects (R/F) (ESC ESC to abort");
            return 0;
         }
         else if(rcvr_type == SA35_RCVR) {
            set_sa35_efc(1);
         }
         else if(rcvr_type == THERMO_RCVR) {  // PE command - set IR sensor emissivity
            sprintf(edit_buffer, "%.2f", emis1);
            start_edit(AMPE_CMD, "Enter IR sensor emissivity (0.1 .. 1.0)  (ESC ESC to abort):");
            return 0;
         }
         else {
            user_pps_enable = 1;
            set_pps(user_pps_enable, pps_polarity,  delay_value, pps1_delay,  pps_threshold, 4);
            request_pps_info();
         }
      }
      else if(first_key == 't') {  // TE command - set utc-ut delta T
         sprintf(edit_buffer, "%f", utc_delta_t()*(24.0*60.0*60.0));
         edit_info1 = "Follow the number with an '*' to also update the delta-t file";
         start_edit(DELTA_T_CMD,  "Enter TT-UT1 delta_T value in seconds,  (ESC ESC to abort)");
         return 0;
      }
      else if(first_key == 'z') {  // ZE command - zoom elevation map
         if(rcvr_type == NO_RCVR) return sure_exit();
         change_zoom_config(6);
         un_zoom = 0;
         zoom_screen = 'S';
         zoom_fixes = show_fixes;
         plot_signals = 3;
         config_screen(120);
      }
      else if(first_key == '$') {  // $E command - very very small 320x480 screen
         new_screen(c);
         return 0;
      }
      else if(luxor && (first_key == '&')) { // &e command - IR emissivity
         sprintf(edit_buffer, "%f", emis1);
         start_edit(AMPE_CMD, "Enter IR1 sensor emissivity (0.1 .. 1.0)  (ESC ESC to abort):");
         return 0;
      }
      else if(first_key == '!') {  // !e command - execute program
//       edit_file[0] = 0;
//       edit_file_type = 1;

         if(no_exec) {
            edit_error("File execution has been disabled!");
         }
         else {
            if((edit_file[0] == 0) || (edit_file_type != 1)) edit_file[0] = 0;
            edit_file_type = 1;
            strcpy(edit_buffer, edit_file);

            if(edit_buffer[0]) start_edit(RUN_PGM_CMD, "Enter command line of the program to run (ESC <cr> to abort)");
            else               start_edit(RUN_PGM_CMD, "Enter command line of the program to run (ESC to abort)");
            return 0;
         }
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
         start_edit(MAX_FOSC_CMD, out);
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
         start_edit(FOLIAGE_CMD, "Foliage:  A)lways   S)ometimes   N)ever");
         return 0;
      }
      #ifdef FFT_STUFF
         else if(first_key == 'g') {   // GF command - fft plot
            edit_plot(FFT, c);
            return 0;
         }
      #endif
         else if(first_key == 'j') {   // JF command
            set_cs_sync("FRONT");
         }
      else if(first_key == 'm') {   // MF command - RINEX format
         sprintf(edit_buffer, "%.2f", rinex_fmt);
         start_edit(RINEX_FORMAT_CMD, "Enter the RINEX file format to write (2.11 or 3.03).  (ESC ESC to abort)");
         return 0;
      }
      #ifdef PRECISE_STUFF
         else if(first_key == 's') {  // SF command - show fixes
            if(luxor) return help_exit(c,99);
            do_fixes(RCVR_MODE_2D_3D); // 2D/3D mode
         }
      #endif
      else if(first_key == 'p') {  // PF command - falling edge PPS polarity
         if(luxor) return help_exit(c,99);
         if(rcvr_type == LPFRS_RCVR) {
            sprintf(edit_buffer, "%g", lpfrs_freq);
            edit_info1 = "Allowable range is -1.28E-7 .. 1.27E-7 parts";
            edit_info2 = "You can use units of Hz by following the value with 'H'";
            edit_info3 = "If you follow the value with '*' the change is incremental.";
            start_edit(LPFRS_FREQ_CMD, "Set frequency offset in units of 'parts' or Hz   (ESC ESC to abort)");
            return 0;
         }
         else if(rcvr_type == PRS_RCVR) {  // PF command - PPS enabled, filtered PPS input
            user_pps_enable = 1;
            set_pps(user_pps_enable, 0,  delay_value, pps1_delay,  pps_threshold, 4);
         }
         else if(rcvr_type == SA35_RCVR) {
            sprintf(edit_buffer, "%g", sa35_freq);
            edit_info1 = "Allowable range is -2.0E-8 .. 2.0E-8 parts";
            edit_info2 = "You can use units of Hz by following the value with 'H'";
            edit_info3 = "If you follow the value with '*' the change is incremental.";
            start_edit(SA35_FREQ_CMD, "Set frequency offset in units of 'parts' or Hz   (ESC ESC to abort)");
            return 0;
         }
         else if(rcvr_type == SRO_RCVR) {
            sprintf(edit_buffer, "%g", (double) sro_fc*SRO_DDS_STEP*1.0E12);
            edit_info1 = "Allowable range is +/- 16776 ppt.";
            edit_info2 = "Resolution is 5.12E-13 Hz per step.";
            edit_info3 = "You can use units of Hz by following the value with 'H'";
            start_edit(SRO_FC_CMD, "Set frequency offset in units of PPT or Hz  (ESC ESC to abort)");
            return 0;
         }
         else if(rcvr_type == TICC_RCVR) {  // aaaacccc
            sprintf(edit_buffer, "%g %g", fudge_a,fudge_b);
            edit_info1 = " ";
            edit_info2 = "THIS WILL RESET THE DATA QUEUES";
            if(ticc_type == TAPR_TICC) edit_info3 = "This command can take a few seconds...";
            start_edit(TICC_FUDGE_CMD, "Enter the chA and chB fudge factors in ps (ESC ESC to abort)");
            return 0;
         }
         else if(rcvr_type == X72_RCVR) {
            if(x72_creg & 0x0004) sprintf(edit_buffer, "1");
            else                  sprintf(edit_buffer, "0");
            edit_info1 = "The SA22.c does not have an FXO output!";
            start_edit(X72_FXO_CMD, "Set FXO output  (1=enable 0=disable) (ESC ESC to abort)");
            return 0;
         }
         else {
            user_pps_enable = 1;
            set_pps(user_pps_enable, 1,  delay_value, pps1_delay, pps_threshold, 3);
            request_pps_info();
         }
      }
      else if(first_key == 'w') {  // WF command - write filtered data to log
         filter_log = toggle_value(filter_log);
      }
      else if(0 && (first_key == 'z')) {  // ZF command - zoom FFT watefall
         // sorry, disabled because this does nothing useful
         change_zoom_config(33);
         un_zoom = 0;
         zoom_screen = 'F';
         zoom_fixes = show_fixes;
         config_screen(133);
         fft_row = 0;
         fft_col = 0;
      }
      else if(first_key == '$') {  // $f command - full screen
         new_screen(c);
         return 0;
      }
      else {  // F command - toggle filter
         if(first_key == 0) {  // xxxxxx
            if(are_you_sure(c) != c) return 0;
         }
         else return help_exit(c,666);
         // next call to do_kbd() will process the filter selection character
      }
   }
   else if(c == 'g') {
      if(first_key == 'c') {   // CG command - remove glitched queue points
         sprintf(edit_buffer, "%g", DEFAULT_DEGLITCH_SIGMA);
         sprintf(out, "Enter deglitch standard deviation sigma threshold for all plots. (ESC ESC to abort)");
         start_edit(DEGLITCH_ALL_CMD, out);
         return 0;
      }
      else if(first_key == 'g') {  // GG command - graph title
         strcpy(edit_buffer, plot_title);
         start_edit(TITLE_CMD, "Enter graph description (ESC <cr> to erase):");
         return 0;
      }
      else if(first_key == 'm') {  // MG command - set v3.xx GPS observation list
         strcpy(edit_buffer, gps_obs_list);
         rinex_list = c;
         edit_info1 = "Enter a blank line for auto-detect.";
         start_edit(RINEX_LIST_CMD, "Enter the RINEX v3.xx GPS observation list (ESC ESC to abort)");
         return 0;
      }
      else if((first_key == 'p') && (rcvr_type == ESIP_RCVR)) { // PG command - set GCLK mode
         old_type = esip_pps_type;
         esip_pps_type = 1;
         set_pps(user_pps_enable, pps_polarity,  cable_delay, pps1_delay,  pps_threshold, 4);
         esip_pps_type = old_type;
      }
      else if((rcvr_type == SRO_RCVR) && (first_key == 'p')) {  // PG command - set gofast mode
         edit_info1 = "Allowable values are 0 .. 65535.  0=OFF  65535=ALWAYS";
         edit_info2 = "This command only available for firmware rev > 1.96";
         sprintf(edit_buffer, "%d", sro_gf);
         start_edit(SRO_GF_CMD, "Enter the GOFAST mode time in seconds.  (ESC ESC to abort):");
         return 0;
      }
      else if(first_key == 's') { // SG command - set GNSS system/systems to use
         edit_buffer[0] = 0;
         if(gnss_mask == 0)      strcat(edit_buffer, "A ");
         if(gnss_mask & GPS)     strcat(edit_buffer, "G ");
         if(gnss_mask & GLONASS) strcat(edit_buffer, "R ");
         if(gnss_mask & GALILEO) strcat(edit_buffer, "E ");
         if(gnss_mask & BEIDOU)  strcat(edit_buffer, "C ");
         if(gnss_mask & SBAS)    strcat(edit_buffer, "S ");
         if(gnss_mask & IRNSS)   strcat(edit_buffer, "I ");
         if(gnss_mask & QZSS)    strcat(edit_buffer, "J ");
         if(gnss_mask & IMES)    strcat(edit_buffer, "M ");
         edit_info1 = "   A=All  D=receiver default";
         edit_info2 = "   G=GPS  S=SBAS  R=GLONASS  E=Galileo  C=Beidou   J=QZSS  M=IMES  I=IRNSS";
         if(edit_buffer[0]) start_edit(GNSS_CMD, "Enter characters to select GNSS systems to enable (ESC ESC to abort)");
         else               start_edit(GNSS_CMD, "Enter characters to select GNSS systems to enable (ESC to abort)");
         return 0;
      }
      else if(first_key == 't') {  // TG commmand - use GPS time
//       time_zone_set = 0;
         temp_utc_mode = 0;
         set_timing_mode(TMODE_GPS, 1);
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
         if(rcvr_type == PRS_RCVR) {
            sprintf(edit_buffer, "%d", prs_ga);
            start_edit(GAIN_CMD, "Enter PLL gain (0 .. 10) (ESC ESC to abort):");
         }
         else {
            sprintf(edit_buffer, "%f", user_osc_gain);
            start_edit(GAIN_CMD, "Enter oscillator gain in Hz/v (ESC ESC to abort):");
         }
         return 0;
      }
      else {  // G command - select graph/display option
         if(first_key == 0) {  // xxxxxx
            if(are_you_sure(c) != c) return 0;
         }
         else return help_exit(c,666);
         // next call to do_kbd() will process the graph selection character
      }
   }
   else if(c == 'h') {  // toggle manual holdover mode
      if(first_key == '&') { // &H command - change max allowale dac voltage or X72 holdover analysis time
         if(luxor) return help_exit(c,99);
         else if(rcvr_type == X72_RCVR) {
            sprintf(edit_buffer, "%d", x72_holdover_val);
            start_edit(X72_HOLDOVER_CMD, "Enter holdover analysis time constant (120 .. 3600) (ESC ESC to abort):");
            return 0;
         }
         else {
            sprintf(edit_buffer, "%f", user_max_range);
            start_edit(MAX_RANGE_CMD, "Enter maximum DAC voltage range value (ESC ESC to abort):");
            return 0;
         }
      }
      else if(luxor && (first_key == 'b')) {  // BH command - high voltage lipo charge mode
         sprintf(edit_buffer, "%f %f", 0.1F, 4.30F);
         start_edit(BH_CMD, "Enter desired charge current in amps and voltage in volts.  (ESC ESC to abort):");
         return 0;
      }
      else if(first_key == 'g') { // GH command - toggle HOLDOVER plot
         if(luxor) return help_exit(c,99);
         plot_holdover_data = toggle_value(plot_holdover_data);
         draw_plot(REFRESH_SCREEN);
      }
      else if(first_key == 'h') {  // HH command - toggle holdover mode
         if(luxor) return help_exit(c,99);
         user_holdover = toggle_value(user_holdover);
         if(user_holdover) set_discipline_mode(SET_DIS_MODE_HOLDOVER);
         else              set_discipline_mode(SET_DIS_MODE_NORMAL);
      }
      else if(first_key == 'l') {  // LH command - open log in hex dump mode
         if(log_file) {
            strcpy(edit_buffer, "Y");
            sprintf(out, "Log file %s is already open.  Close it? (Y/N)  (ESC ESC to abort)", log_name);
            start_edit(LOG_CLOSE_CMD, out);
            return 0;
         }
         else {
            log_mode = "w";
            strcpy(edit_buffer, log_name);
            edit_info1 = "Append an '*' to the file name to flush file after every write";  // FLUSH_CHAR
            log_stream = (LOG_HEX_STREAM | LOG_PACKET_ID | LOG_SENT_DATA);
            if(show_debug_info) log_stream |= LOG_PACKET_START;
            start_edit(LOG_CMD, "Enter log file name to write HEX format data to: ");
            return 0;
         }
      }
      else if(first_key == 'm') {   // MH command - antenna height/ew/ns offset
         sprintf(edit_buffer, "%.3f", antenna_height);
         edit_info1 = "You can also include the east/west and north/south dispacements.";
         start_edit(RINEX_HEIGHT_CMD, "Enter the antenna height displacement in meters.  (ESC ESC to abort)");
         return 0;
      }
      else if(luxor && (first_key == 'p')) {  // PH command = battery high voltage cutoff
         sprintf(edit_buffer, "%f", batt_hvc);
         start_edit(PH_CMD, "Enter the battery high voltage cutoff in volts.  (ESC ESC to abort):");
         return 0;
      }
      else if((first_key == 'p') && (rcvr_type == PRS_RCVR)) { // PH command - set PPS output
         user_pps_enable = 0;
         set_pps(user_pps_enable, 0,  delay_value, pps1_delay,  pps_threshold, 4);
      }
      else if((first_key == 'p') && (rcvr_type == SRO_RCVR)) { // PH command - find phase comparator
         sprintf(edit_buffer, "%d", sro_co);
         start_edit(SRO_CO_CMD, "Set fine phase comparator offset in ns (-128 .. 127)  (ESC ESC to abort)");
         return 0;
      }
      else if((first_key == 'p') && (rcvr_type == STAR_RCVR)) {
         sprintf(edit_buffer, "%d", user_hbsq);
         start_edit(STAR_SET_HBSQ_CMD, "Enter the HBSQ squelch time in minutes (0 .. 7200)  (ESC ESC to abort):");
         return 0;
      }
      else if((first_key == 'p') && (rcvr_type == X72_RCVR)) { // PH command - set service polarity
         set_x72_srvc(1);
      }
      else if(first_key == 's') {  // SH command - position hold mode
         set_rcvr_mode(RCVR_MODE_HOLD);
      }
      else if(first_key == 't') {  // TH command - set chime mode
         if(cuckoo) sprintf(edit_buffer, "%d", cuckoo);
         else if(tick_clock) sprintf(edit_buffer, "%d", tick_clock);
         else sprintf(edit_buffer, "4");

         if(singing_clock == 2) strcat(edit_buffer, "W");
         else if(singing_clock) strcat(edit_buffer, "S");
         else if(ships_clock)   strcat(edit_buffer, "B");
         else if(tick_clock)    strcat(edit_buffer, "T");

if(1 && tick_clock && fine_tick_clock) {  // !!!! fine_tick_clock is now always on for tick_clock mode
   strcat(edit_buffer, "F");
}

         if(cuckoo_hours) strcat(edit_buffer, "H");

         edit_info1 = "Enter 4W for Westminster chimes / Big Ben (requires user supplied sound files)";
         edit_info2 = "Enter 1T for seconds tick  2T for minutes clock  3T for seconds/minutes clock";
         edit_info3 = "Enter 0 to disable audible clocks";
         start_edit(CHIME_CMD, "Chime/sing at # places per hour (#S=sing  #H=chime out hours  1B=Ship's bells):");
         return 0;
      }
      else if(first_key == 'z') {  // ZH command - hex packet data flow monitor mode
         monitor_port = RCVR_PORT;
         monitor_hex = 0;
         set_monitor_mode(1);
         zoom_screen = 'H';
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
            last_atype = ATYPE;
            if(rcvr_type == TICC_RCVR) all_adevs = ALL_CHANS;
            else                       all_adevs = SINGLE_ADEVS;
            plot_adev_data = 1;
            force_adev_redraw(108);
            config_screen(111);
            if(adevs_active(0)) last_was_adev = c;
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
         else if(rcvr_type == PRS_RCVR) {   // &i command - integrator value
            sprintf(edit_buffer, "%d", prs_pi);
            start_edit(INITV_CMD, "Enter PLL intergrator value (-2000 .. 2000)  (ESC ESC to abort):");
            return 0;
         }
         else {   // &i command - initial dac voltage
            sprintf(edit_buffer, "%f", user_initial_voltage);
            start_edit(INITV_CMD, "Enter initial DAC voltage (ESC ESC to abort):");
            return 0;
         }
      }
      else if(first_key == 'a') {  // AI command - set adev type to MTIE
         show_mtie:
         if(luxor) return help_exit(c,99);
         ATYPE = A_MTIE;
//       last_atype = ATYPE;
         if(rcvr_type == TICC_RCVR) all_adevs = ALL_CHANS;
         else                       all_adevs = SINGLE_ADEVS;
         plot_adev_data = 1;
         force_adev_redraw(109);
         config_screen(108);
         if(adevs_active(0)) last_was_adev = c;
      }
      else if(luxor && (first_key == 'b')) {  // BI command - calc internal resistance
         calc_ir = IR_TIME;
         pause_data = 0;
         need_redraw = 1208;
      }
      else if(first_key == 'c') {  // CI command - clear MTIE data
         new_queue(RESET_MTIE_Q, 33);
      }
      else if(first_key == 'f') {  // FI command - toggle Motorola ionosphere filter
         if(luxor) return help_exit(c,99);
         static_filter = toggle_value(static_filter);
         set_filter_config(pv_filter, static_filter, alt_filter, kalman_filter, marine_filter, 1);
      }
      else if(first_key == 'g') { // GI command
         if(luxor || (rcvr_type == CS_RCVR)) {   // toggle BATTw or HWI graph
            edit_plot(TWELVE, c);
            return 0;
         }
         else if(rcvr_type == NO_RCVR) ;
         else if(rcvr_type == TICC_RCVR) {
            goto show_mtie;
         }
//       else if(rcvr_type == ZYFER_RCVR) ;
         else {   // toggle fix map display
            if(zoom_screen == 'K') {
               change_zoom_config(-104);
               cancel_zoom(14);   //zkzk
            }
            show_fixes = toggle_value(show_fixes);
            user_set_ref_lla = 0;
            config_fix_display();
         }
      }
      else if(first_key == 'l') { // LI command - set log interval
         sprintf(edit_buffer, "%ld", log_interval);
      }
      else if(first_key == 'm') { // MI command - set RINEX fix bad data mode
         if(rinex_fix == 0) strcpy(edit_buffer, "N");
         else               strcpy(edit_buffer, "Y");
         start_edit(RINEX_FIX_CMD, "Attempt to fix bad RINEX observations? (Y=yes, N=no) (ESC ESC to abort)");
         return 0;
      }
      else if(first_key == 'p') {  // PI command - set timestamp wrap interval
         if(timestamp_wrap) sprintf(edit_buffer, "%g", timestamp_wrap);
         else               sprintf(edit_buffer, "%g", TS_WRAP_INTERVAL);
         edit_info1 = "THIS WILL RESET THE DATA QUEUES";
         edit_info2 = " ";
         start_edit(TS_WRAP_CMD, "Enter number of seconds to wrap timestamps at.  (ESC ESC to abort):");
         return 0;
      }
      else if(first_key == 's') {  // SI command - set count of sats to display
         si_cmd:
         edit_info1 = "  Include a '+' for abbreviated sat info in two columns";
         edit_info2 = "  Include a '-' for abbreviated sat info in a single column";
         edit_info3 = "  Include a 'T' to show only Tracked satellites";
         if(sat_cols > 1) sprintf(edit_buffer, "+%d", max_sat_display);
         else if(user_set_short) sprintf(edit_buffer, "%d", 0-max_sat_display);
         else sprintf(edit_buffer, "%d", max_sat_display);
         if(tracked_only) strcat(edit_buffer, " T");
         start_edit(SI_CMD, "Enter max number of sats to display info for  (ESC ESC to abort):");
         return 0;
      }
      else if(first_key == 't') {  // ti command - elapsed time interval
         show_elapsed_time = toggle_value(show_elapsed_time);
         if(show_elapsed_time && (jd_elapsed == 0.0)) jd_elapsed = jd_utc;
         need_redraw = 5976;
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
         un_zoom = 0;
         if(rcvr_type == CS_RCVR) {
            zoom_screen = 'D';
         }
         else {
            zoom_screen = 'I';
            zoom_fixes = show_fixes;
            plot_signals = 5;
         }
         config_screen(120);
      }
      else if(first_key == '$') {  // $I command - invert black/white
         invert_screen();
      }
      else if(first_key == 0) {
         edit_buffer[0] = 0;
         start_edit(QUEUE_INT_CMD, "Enter the queue update interval in seconds: (ESC to abort):");
         return 0;
      }
      else return help_exit(c,99);
   }
   else if(c == 'j') {
      if(first_key == '&') { // &J command - jam sync threshold
         if(luxor) return help_exit(c,99);
         if(rcvr_type == X72_RCVR) {
            sprintf(edit_buffer, "%g", x72_jamthresh_val);
            start_edit(JAMSYNC_CMD, "Enter jam sync threshold in ns (ESC ESC to abort):");
         }
         else {
            sprintf(edit_buffer, "%f", user_jam_sync);
            start_edit(JAMSYNC_CMD, "Enter jam sync threshold in ns (ESC ESC to abort):");
         }
         return 0;
      }
      else if(first_key == 'f') {  // FJ command - set jamming filter
         if(luxor) return help_exit(c,99);
         if     (foliage_mode == 0) strcpy(edit_buffer, "S");
         else if(foliage_mode == 2) strcpy(edit_buffer, "N");
         else                       strcpy(edit_buffer, "N");
         start_edit(FOLIAGE_CMD, "Jamming:  S) enable   N) disable");
         return 0;
      }
      else if(first_key == 'g') {
         if(luxor) { // GJ command - toggle LEDw plot
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
         set_discipline_mode(SET_DIS_MODE_JAMSYNC);
         BEEP(313);
      }
      else if(first_key == 't') {  // TJ command - show Julian time digital clock
         show_julian_time = toggle_value(show_julian_time);
         show_mjd_time = 0;
         show_unix_time = 0;
         show_gps_time = 0;
         show_msecs = 0;
         jd_elapsed = 0.0;
         show_elapsed_time = 0;
         clock_12 = 0;
         plot_digital_clock = 1;
         need_redraw = 4455;
      }
      else if(first_key == '$') {  // $j command - 1280x800 screen
         new_screen(c);
         return 0;
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
         if(luxor || (rcvr_type == CS_RCVR)) { // GK command - toggle driver efficency or mass spec plot
            edit_plot(THIRTEEN, c);
            return 0;
         }
         else if(enviro_mode()) {
            strcpy(edit_buffer, "A");
            sprintf(out, "%s  (ESC to abort)", tide_plots());
            start_edit(TIDE_CMD, out);
            return 0;
         }
         else if(plot_prn) {
            strcpy(edit_buffer, "A");
            if(have_phase)      s = "G)phase";
            else if(have_range) s = "G)range";
            else                s = "G)ravity";
            if(plot_prn == SUN_PRN) {
               sprintf(out,         "SUN:      Z)az    E)el    S)signal   %s   A)all   I)xy plot  (ESC to abort)", s);
            }
            else if(plot_prn == MOON_PRN) {
               sprintf(out,         "MOON:     Z)az    E)el    S)signal   %s   A)all   I)xy plot  (ESC to abort)", s);
            }
            else {
               sprintf(out,         "PRN %-3d:  Z)az    E)el    S)signal   %s   A)all   I)xy plot  (ESC to abort)", plot_prn, s);
            }
            start_edit(TIDE_CMD, out);
            return 0;
         }
         else {
            strcpy(edit_buffer, "A");
            start_edit(TIDE_CMD, "Earth tide plots:  X)lon   Y)lat   Z)alt   G)gravity  A)ll   I)xy plot  (ESC to abort)");
            return 0;
         }
      }
      else if(first_key == 'm') {  // MK command - set marker name
         strcpy(edit_buffer, marker_name);
         start_edit(MARKER_NAME_CMD, "Enter the marker name (max 20 chars) (ESC ESC to abort)");
         return 0;
      }
      else if((first_key == 'p') && (rcvr_type == SRO_RCVR)) {  // PK command - set track mode
         sprintf(edit_buffer, "%d", sro_sy);
         edit_info1 = "0=OFF  1=NOW  2=ALWAYS  3=NOW+ALWAYS";
         start_edit(SRO_TR_CMD, "Set tracking mode  (ESC ESC to abort)");
         return 0;
      }
      else if((first_key == 'p') && (rcvr_type == TICC_RCVR)) {  // PK command - set coarse clock
         sprintf(edit_buffer, "%f", ticc_coarse);
         edit_info1 = " ";
         edit_info2 = "THIS WILL RESET THE DATA QUEUES";
         if(ticc_type == TAPR_TICC) edit_info3 = "This command can take a few seconds...";
         start_edit(TICC_COARSE_CMD, "Enter the TICC coarse clock period in usec  (ESC ESC to abort):");
         return 0;
      }
      else if(first_key == 't') {  // TK commmand - setup for receiver message jitter measurements
         measure_jitter = toggle_value(measure_jitter);
         set_jitter_config();
      }
      else if(first_key == 'z') {  // ZK command - touch screen keyboard
         if(touch_screen) {
            zoom_screen = 'K';
            show_touch_kbd(1);
            first_key = 0;
            return 0;
         }
      }
      else if(first_key == '$') {  // $k command - 1280x960 screen
         new_screen(c);
         return 0;
      }
      else if(first_key == '!') { // !K command - init Venus RTK receiver
         if(rtk_mode == BASE_MODE) strcpy(edit_buffer, "B");
         else if(rtk_mode == ROVER_MODE) strcpy(edit_buffer, "R");
         else sprintf(edit_buffer, "%d", rtk_mode);
         edit_info1 = "This command may take several seconds to complete...";
         edit_info3 = "Also:  E=enable RTCM output   D=disable RTCM output";
         edit_info4 = "       If you enable RTCM output you must exit the program within 5 seconds!";
         start_edit(RTK_MODE_CMD, "Select RTK base / rover mode (R=Rover  B=Base)  (ESC ESC to abort)");
         return 0;
      }
      else if(first_key == 0) {
         if(have_temperature && (tsip_type != STARLOC_TYPE)) {
            sprintf(edit_buffer,     "K %f %f %f %f", P_GAIN, D_TC, FILTER_TC, I_TC);
            edit_info1 =             "Custom PID:    K [proportional fain] [derivative_tc] [filter_tc] [integrator_tc]";
            edit_info2 =             "Autotune:      A0)abort  A1)full   A2)no setpoint delay  A3)no stabilize delay";
            start_edit(TEMP_PID_CMD, "TEMP CTRL PID: W)slow    N)medium  X)fast  Y)very fast");
            return 0;
         }
         else return help_exit(c,3);
      }
      else return help_exit(c,3);
   }
   else if(c == 'l') {
      if(first_key == '&') {
         if(luxor) {  // &L command - lux sensor sensitivity
            sprintf(edit_buffer, "%.0f", (double) lux1_time);
            sprintf(out, "Lux sensor gain: L)ow=%d  M)edium=%d  H)high=%d  or enter gain (%d..%d) (ESC ESC to abort):", LOW_LUX,MED_LUX,HI_LUX, MIN_LUX,MAX_LUX);
            start_edit(AMPL_CMD, out);
            return 0;
         }
         else {  // &L command - change min dac voltage
            sprintf(edit_buffer, "%f", user_min_range);
            start_edit(MIN_RANGE_CMD, "Enter minimum DAC voltage range value (ESC ESC to abort):");
            return 0;
         }
      }
      else if(luxor && (first_key == 'b')) {  // BL command - lipo charge mode
         sprintf(edit_buffer, "%f", 0.1);
         start_edit(BL_CMD, "Enter desired LIPO charge current in amps.  (ESC ESC to abort):");
         return 0;
      }
      else if(first_key == 'c') {
         if(luxor) return help_exit(c,99);
         if(rcvr_type == CS_RCVR) {  // CL command - clear log
            clear_cs_log();
         }
         else { // CL command - clear LLA fixes
            #ifdef BUFFER_LLA
               clear_lla_points(1);
               need_redraw = 1209;
            #endif
         }
      }
      else if(first_key == 'f') {  // FL command - signal level mask
         if(luxor) return help_exit(c,99);
         sprintf(edit_buffer, "%.1f", amu_mask);
         if((res_t && (res_t != RES_T)) || (rcvr_type != TSIP_RCVR)) {
            start_edit(SET_SIGMASK_CMD,   "Enter minimum acceptible signal level (in dBc)");
         }
         else {
            start_edit(SET_SIGMASK_CMD, "Enter minimum acceptible signal level (in AMU)");
         }
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
      else if(first_key == 'm') {  // ML command - set v3.xx Galileo observation list
         strcpy(edit_buffer, galileo_obs_list);
         rinex_list = c;
         edit_info1 = "Enter a blank line for auto-detect.";
         start_edit(RINEX_LIST_CMD, "Enter the RINEX v3.xx GALILEO observation list (ESC ESC to abort)");
         return 0;
      }
      else if(luxor && (first_key == 'p')) {  // PL command = low voltage cutoff
         sprintf(edit_buffer, "%f", batt_lvc);
         start_edit(PL_CMD, "Enter the battery low voltage cutoff in volts.  (ESC ESC to abort):");
         return 0;
      }
      else if((first_key == 'p') && (rcvr_type == CS_RCVR)) {  // PL command - set leap second
         if(month > 6) sprintf(edit_buffer, "%04d/12/31 61", year);
         else          sprintf(edit_buffer, "%04d/6/30 61", year);
         start_edit(CS_LEAP_CMD, "Enter the date of the leapsecond. (0/0/0 = cancel)  (ESC ESC to abort):");
         return 0;
      }
      else if((first_key == 'p') && (rcvr_type == ESIP_RCVR)) { // PL command - set legacy GCLK mode
         old_type = esip_pps_type;
         esip_pps_type = 0;
         set_pps(user_pps_enable, 0,  cable_delay, pps1_delay,  pps_threshold, 4);
         esip_pps_type = old_type;
      }
      else if((first_key == 'p') && (rcvr_type == PRS_RCVR)) { // PL command - set PPS output
         user_pps_enable = 0;
         set_pps(user_pps_enable, 1,  delay_value, pps1_delay,  pps_threshold, 4);
      }
      else if((first_key == 'p') && (rcvr_type == X72_RCVR)) { // PL command - set service polarity
         set_x72_srvc(0);
      }
      else if(first_key == 's') {  // SL command - set lat/lon/alt
         if(luxor) return help_exit(c,99);
         if(check_precise_posn) {
            strcpy(edit_buffer, "Y");
            start_edit(ABORT_LLA_CMD, "Lat/lon/alt search aborted! Save low resolution position? Y)es N)o");
         }
         else if(rcvr_type == UCCM_RCVR) {
            sprintf(edit_buffer, "%.8lf %.8lf %.3lf", lat*RAD_TO_DEG, lon*RAD_TO_DEG, alt);
            edit_info1 = "The lat/lon values can be a decimal number or like 10d20m30s";
            edit_info2 = "The altiude value is in meters unless ended with an f or '";
            edit_info3 = "A value of 'A' says to use the current average location value.";
            edit_info4 = "Use ++ or -- as the sign of a value to move the current location";
            edit_info5 = "by the specified number of meters (or feet if in /t' mode)";
            start_edit(SET_LLA_CMD, "Enter lat lon alt (-=S,W +=N,E  alt in meters)  (ESC ESC to abort):");
         }
         else {
            if(have_precise_lla > 0) { // suggest the calculated value
               sprintf(edit_buffer, "%.8lf %.8lf %.3lf", precise_lat*RAD_TO_DEG, precise_lon*RAD_TO_DEG, precise_alt);
            }
            else {  // suggest the current receiver value
               sprintf(edit_buffer, "%.8lf %.8lf %.3lf", lat*RAD_TO_DEG, lon*RAD_TO_DEG, alt);
            }
            edit_info1 = "The lat/lon values can be a decimal number or like 10d20m30s";
            edit_info2 = "The altiude value is in meters unless ended with an f or '";
            edit_info3 = "A value of 'A' says to use the current average location value.";
            edit_info4 = "Use ++ or -- as the sign of a value to move the current location";
            edit_info5 = "by the specified number of meters (or feet if in /t' mode)";
            start_edit(SET_LLA_CMD, "Enter precise lat lon alt (-=S,W +=N,E  alt in meters)  (ESC ESC to abort):");
         }
         return 0;
      }
      else if(first_key == 't') { // TL command - set log dump time
         if     (script_file) edit_buffer[0] = 0;
         else if(log_time && log_date) sprintf(edit_buffer, "%02d:%02d:%02d  %04d/%02d/%02d", log_hh,log_mm,log_ss, log_year,log_month,log_day);
         else if(log_time) sprintf(edit_buffer, "%02d:%02d:%02d", log_hh,log_mm,log_ss);
         else if(log_date) sprintf(edit_buffer, "%04d/%02d/%02d", log_year,log_month,log_day);
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
            if(edit_buffer[0]) start_edit(LOG_DUMP_CMD, "Enter XML log dump time (and optional date) or interval or <ESC CR> to reset:");
            else               start_edit(LOG_DUMP_CMD, "Enter XML log dump time (and optional date) or interval or <CR> to reset:");
         }
         else if(dump_gpx) {
            if(edit_buffer[0]) start_edit(LOG_DUMP_CMD, "Enter GPX log dump time (and optional date) or interval or <ESC CR> to reset:");
            else               start_edit(LOG_DUMP_CMD, "Enter GPX log dump time (and optional date) or interval or <CR> to reset:");
         }
         else {
            if(edit_buffer[0]) start_edit(LOG_DUMP_CMD, "Enter ASCII log dump time (and optional date) or interval or <ESC CR> to reset:");
            else               start_edit(LOG_DUMP_CMD, "Enter ADCII log dump time (and optional date) or interval or <CR> to reset:");
         }
         return 0;
      }
      else if(first_key == 'w') {  // WL command - write log
         if(zoom_screen == 'K') {
            reset_first_key(26);
            c = 'l';
            zoom_screen = 'K';
            show_touch_kbd(555);
         }
         else {
            reset_first_key(26);
            c = 'l';
         }
         goto log_cmd;
      }
      else if((rcvr_type != NO_RCVR) && (first_key == 'z')) {  // ZL command - zoom lla
un_zoom = 0;
         config_lla_zoom(0);
      }
      else if(first_key == '$') {  // $l command - large screen
         new_screen(c);
         return 0;
      }
      else if((first_key == '!') && (rcvr_type == CS_RCVR)) { // !L command - reset continuous operation light
         request_warm_reset();
         redraw_screen();
      }
      else {    // L command - log stuff
         log_cmd:
          if(first_key == 0) {  // xxxxxx
             if(are_you_sure(c) != c) return 0;
          }
          else return help_exit(c,666);
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
         if(rcvr_type == FURUNO_RCVR) {
            sprintf(edit_buffer, "%d", dynamics_code);
            start_edit(DYNAMICS_CMD,  "Receiver movement:  1=accurate  2=average  3=frequent");
         }
         else if(rcvr_type == MOTO_RCVR) {
            sprintf(edit_buffer, "%d", marine_filter);
            start_edit(MARINE_CMD, "Enter marine velocity filter (10=max  100=min  ESC ESC to abort)");
         }
         else {
            if     (dynamics_code == 1) strcpy(edit_buffer, "L");
            else if(dynamics_code == 2) strcpy(edit_buffer, "S");
            else if(dynamics_code == 3) strcpy(edit_buffer, "A");
            else                        strcpy(edit_buffer, "F");
            start_edit(DYNAMICS_CMD,  "Receiver movement:  A)ir  F)ixed  L)and  S)ea");
         }
         return 0;
      }
      else if(first_key == 'g') {  // GM command - plot satellite map
         if(luxor) return help_exit(c,99);
         plot_azel = toggle_value(plot_azel);
         do_azel:
         if(AZEL_OK == 0) plot_azel = AZEL_OK;
         if(plot_azel || plot_signals) update_azel = 1;
         config_screen(114);
      }
      #ifdef ADEV_STUFF
         else if(first_key == 'a') {  // AM command - set adev type to MDEV
            if(luxor) return help_exit(c,99);
            ATYPE = OSC_MDEV;
            last_atype = ATYPE;
            if(rcvr_type == TICC_RCVR) all_adevs = ALL_CHANS;
            else                       all_adevs = SINGLE_ADEVS;
            plot_adev_data = 1;
            force_adev_redraw(110);
            config_screen(115);
            if(adevs_active(0)) last_was_adev = c;
         }
      #endif
      else if(first_key == 'm') {  // MM command - set v2.11 observation list
         strcpy(edit_buffer, mixed_obs_list);
         rinex_list = c;
         edit_info1 = "Enter a blank line for auto-detect.";
         start_edit(RINEX_LIST_CMD, "Enter the RINEX v2.11 observation list (ESC ESC to abort)");
         return 0;
      }
      else if(luxor && (first_key == 'p')) {  // PM command = message watchdog timer
         sprintf(edit_buffer, "%.3f", msg_timeout);
         start_edit(PM_CMD, "Enter the Message watchdog timeout in seconds (0=disable).  (ESC ESC to abort):");
         return 0;
      }
      else if((first_key == 'p') && (rcvr_type == TM4_RCVR)) {  // PM command - set time port format
         sprintf(edit_buffer, "%d", ett_time_format);
         edit_info1 = "0=Spectrum TM4     1=NTP     2=NMEA";
         start_edit(PM_CMD, "Enter the time port message format.  (ESC ESC to abort):");
         return 0;
      }
      else if((first_key == 'p') && (rcvr_type == TICC_RCVR)) {  // PM command - set TICC mode
         sprintf(edit_buffer, "%c", ticc_mode);
         if(ticc_type == TAPR_TICC) {
            edit_info1 = "T)imestamp  I)nterval  P)eriod  D)ebug  timeL)ab";
            edit_info2 = " ";
            edit_info3 = "THIS WILL RESET THE DATA QUEUES";
            edit_info4 = "This command can take a few seconds...";
         }
         else {
            edit_info1 = "T)imestamp  I)nterval  P)eriod  F)requency";
            edit_info2 = " ";
            edit_info3 = "Changing the mode does not alter the counter hardware configuration.";
            edit_info4 = "It only changes how the program interprets the counter data.";
            edit_info5 = "THIS WILL RESET THE DATA QUEUES";
         }
         start_edit(TICC_MODE_CMD, "Enter the TICC operating mode.  (ESC ESC to abort):");
         return 0;
      }
      else if(first_key == 's') {  // SM command - request GPS system message
         request_gps_system_msg();
      }
      else if(first_key == 't') {  // TM command - show millisecond digital clock
         show_msecs = toggle_value(show_msecs);
         jd_elapsed = 0.0;
         show_elapsed_time = 0;
         show_julian_time = 0;
         show_unix_time = 0;
         show_gps_time = 0;
         show_mjd_time = 0;
         clock_12 = 0;
         plot_digital_clock = 1;
         need_redraw = 4455;
      }
      else if(first_key == 'w') {  // WM command - write MTIE data
         if(TICC_USED) {
            dump_type = 'm';
            sprintf(edit_buffer, "%s", "mtie.dat");
            start_edit(WRITE_CMD, "Enter name of MTIE data file to write (ESC ESC to abort):");
            return 0;
         }
         else {
            edit_error("No MTIE data available to write.");
         }
      }
      else if(first_key == 'z') {  // ZM command - zoom map
         if(rcvr_type == NO_RCVR) return sure_exit();
         change_zoom_config(5);
         un_zoom = 0;
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
         if(edit_buffer[0]) start_edit(DRVR_CMD, "Enter driver mode sequence: 0=OFF 1=ON  or  OFFmsecs [ONmsecs...]  (ESC ESC to abort):");
         else               start_edit(DRVR_CMD, "Enter driver mode sequence: 0=OFF 1=ON  or  OFFmsecs [ONmsecs...]  (ESC to abort):");
         return 0;
      }
      else if(first_key == 0) {
         if(are_you_sure(c) != c) return 0;
      }
      else return help_exit(c,4);
   }
   else if(c == 'n') {
      if(first_key == '&') { // &N command - change min voltage
         if(luxor) return help_exit(c,99);
         sprintf(edit_buffer, "%f", user_min_volts);
         start_edit(MINV_CMD, "Enter minimum EFC control voltage (ESC ESC to abort):");
         return 0;
      }
      else if(first_key == 'm') {  // MN command - set v3.xx Glonass observation list
         strcpy(edit_buffer, glonass_obs_list);
         rinex_list = c;
         edit_info1 = "Enter a blank line for auto-detect.";
         start_edit(RINEX_LIST_CMD, "Enter the RINEX v3.xx GLONASS observation list (ESC ESC to abort)");
         return 0;
      }
      else if((first_key == 'p') && (rcvr_type == X72_RCVR)) {  // PN command - set ACMOS divider
         if(user_set_x72_acmos_freq) sprintf(edit_buffer, "%d", last_x72_acmos_freq);
         else                        sprintf(edit_buffer, "%d", 3);
         sprintf(out, "The ACMOS frequency will be %.1f / (2*N)", x72_osc);
         edit_info1 = &out[0];
         edit_info2 = "If the entered value is followed by 'h' the divider will be set to the closest";
         edit_info3 = "possible frequency in Hz (i.e. entering 4E6H will set the output to 3.75 MHz";
         if(x72_type == SA22_TYPE) {
            edit_info4 = "Changing the ACMOS freq will change the sine output freq!";
         }
         start_edit(X72_FREQ_CMD, "Enter the ACMOS frequency divider ratio (1 .. 65536)  (ESC ESC to abort):");
         return 0;
      }
      else if((first_key == 'p') && TICC_USED) {  // PN command - set nominal freq
         sprintf(edit_buffer, "%f", nominal_cha_freq);
         edit_info1 = "THIS WILL RESET THE DATA QUEUES";
         edit_info2 = " ";
         edit_info3 = "Follow the frequency value with an A/B/C and/or D to set the nominal";
         edit_info4 = "frequency of specific channels.";
         start_edit(TICC_FREQ_CMD, "Enter the nominal input frequencies in Hz.  (ESC ESC to abort):");
         return 0;
      }
      else if(first_key == 's') {  // SN command - navigation (3D fix) mode
         set_rcvr_mode(RCVR_MODE_3D);
      }
      else if(first_key == 't') {  // TN command - normal 24 hour digital clock
         show_msecs = 0;
         jd_elapsed = 0.0;
         show_elapsed_time = 0;
         show_julian_time = 0;
         show_unix_time = 0;
         show_gps_time = 0;
         show_mjd_time = 0;
         clock_12 = 0;
         plot_digital_clock = 1;
         need_redraw = 4455;
      }
      else if(first_key == 'z') {  // ZN command - cancel zoom or blank display
         if(0 && zoom_screen) {  // ZN - disable zoom
            remove_zoom();
         }
         else {  // ZN - blank display
            change_zoom_config(6);
            un_zoom = 0;
            zoom_screen = 'N';
         }
      }
      else if(first_key == 't') {  // TN command - use UTC(SU) time
if(rcvr_type != ESIP_RCVR) return help_exit(c,99);
//       time_zone_set = 0;
         temp_utc_mode = 0;
         set_timing_mode(TMODE_SU, 2);
         if(rcvr_type == NVS_RCVR) request_pps_info();
         request_timing_mode();
      }
      else if(first_key == '$') {  // $n command - custom screen
         new_screen(c);
         return 0;
      }
      else if(1 && (first_key == '!'))  { // !N command - restart moto receiver in NMEA mode
         if(rcvr_type == MOTO_RCVR) {     // !!!! does not work on timing receivers
            enable_moto_nmea();
         }
         else if(1 || (rcvr_type == NMEA_RCVR)) {
            enable_moto_binary();
         }
         redraw_screen();
      }
      else if(first_key == 0) { // N command - spawn text editor
         if(no_exec) {
            edit_error("File editing has been disabled.");
         }
         else {
            if((edit_file[0] == 0) || (edit_file_type != 0)) strcpy(edit_file, "heather.cfg");
            edit_file_type = 0;
            strcpy(edit_buffer, edit_file);
            start_edit(EDITOR_CMD, "Enter name of file to edit (ESC <cr> to abort)");
            return 0;
         }
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
            if(ATYPE == A_MTIE) ATYPE = last_atype;
            aa_val = ALL_OSC;
            strcpy(edit_buffer, "G");
            if(rcvr_type == TICC_RCVR) {
               start_edit(ADEV_CMD, "Display chB:   A)devs only   G)raphs and all adevs   graphs and R)egular adevs");
            }
            else {
               start_edit(ADEV_CMD, "Display OSC:   A)devs only   G)raphs and all adevs   graphs and R)egular adevs");
            }
            return 0;
         }
      #endif
      else if(first_key == 'j') {   // JO command
         set_cs_sync("OFF");
      }
      else if(first_key == '$') {  // $O command - rotate screen
         rotate_screen ^= 1;
         keyboard_cmd = 1;
         init_screen(8755);
         keyboard_cmd = 0;
//       return 0;
      }
      else if(luxor && (first_key == 'p')) {  // PO command = battery overcurrent
         sprintf(edit_buffer, "%f", load_ovc);
         start_edit(PO_CMD, "Enter the load overcurrent threshold in amps.  (ESC ESC to abort):");
         return 0;
      }
      else if(first_key == 'p') { // PO command - toggle osc polarity
         if(luxor) return help_exit(c,99);
         if(rcvr_type == TICC_RCVR) {  // PO command = TICC timeout command
           sprintf(edit_buffer, "%d", ticc_timeout);
           edit_info1 = " ";
           edit_info2 = "THIS WILL RESET THE DATA QUEUES";
           start_edit(TICC_TIMEOUT_CMD, "Enter the TICC timeout count  (ESC ESC to abort):");
           return 0;
         }
         else if(rcvr_type == CS_RCVR) {
            sprintf(edit_buffer, "%g", cs_ster);
            start_edit(CS_STER_CMD, "Enter the fractional frequency steering offset  (ESC ESC to abort):");
            return 0;
         }
         else if(rcvr_type == PRS_RCVR) {
            sprintf(edit_buffer, "%d", prs_mo);
            start_edit(PRS_MO_CMD, "Set magnetic offset value (2300 .. 3600)  (ESC ESC to abort)");
            return 0;
         }
         else if(rcvr_type == RT17_RCVR) {
            sprintf(edit_buffer, "%d", add_clk_ofs);
            start_edit(RT17_OFS_CMD, "Add clock offset to observation times  (0=no  1=yes) (ESC ESC to abort)");
            return 0;
         }
         else if(rcvr_type == SRO_RCVR) {
            edit_info1 = "Allowable values are 0 .. 999999999 ns";
            edit_info2 = "Pulse delay granularity is 133.333 ns";
            sprintf(edit_buffer, "%g", sro_de*SRO_TICK);
            start_edit(SRO_DELAY_CMD, "Enter PPS position offset in ns (ESC ESC to abort):");
            return 0;
         }
         else if(rcvr_type == X72_RCVR) {
            sprintf(edit_buffer, "%.1f", x72_osc);
            edit_info1 = "Osc frequemcy is usually 60.0E6 or 58.9824E6 Hz";
            edit_info2 = "This command does not change the osc frequemcy!  It just";
            edit_info3 = "tells Heather what frequemcy the device is using.";
            start_edit(X72_OSC_CMD, "Enter the device's master oscillator freq in Hz  (ESC ESC to abort)");
            return 0;
         }
         else if(rcvr_type != TSIP_RCVR) {  // pps offset delay in nanoseconds
            start_edit(PPS_OFS_CMD, "Enter PPS position offset in ns (ESC ESC to abort):");
            return 0;
         }
         else {
            user_osc_polarity = toggle_value(user_osc_polarity);
            set_osc_sense(user_osc_polarity, 1);
         }
      }
      else if(first_key == 't') {  // TO command - set message arrival offset time
         sprintf(edit_buffer, "%.2f", time_sync_offset);
         edit_info1 = " ";
         edit_info2 = "Positive values mean time messages arrive after the PPS pulse";
         edit_info3 = "Negative values mean time messages arrive before the PPS pulse";
         start_edit(TSX_CMD, "Enter time message arrival time offset in msecs (ESC ESC to abort):");
         return 0;
      }
      else if(first_key == 's') { // SO command - single sat mode
         if(luxor) return help_exit(c,99);
         if((rcvr_type == TSIP_RCVR) && (rcvr_mode == RCVR_MODE_SINGLE)) {
            sprintf(edit_buffer, "%d", 0);
         }
         else if(single_sat) {
            sprintf(edit_buffer, "%d", 0);
         }
         else {
            sprintf(edit_buffer, "%d", highest_sat());
         }
         if(rcvr_type == TSIP_RCVR) {
            if(rcvr_mode == RCVR_MODE_SINGLE) start_edit(SINGLE_SAT_CMD, "Enter 0 to exit single sat mode");
            else                              start_edit(SINGLE_SAT_CMD, "Enter PRN of sat for single sat mode (0=highest)");
         }
         else {
            start_edit(SINGLE_SAT_CMD, "Enter PRN of sat for single sat mode (0=enable all sats)");
         }
         return 0;
      }
      else if(first_key == 'z') {  // ZO command - data flow monitor mode
         monitor_port = RCVR_PORT;
         if(ASCII_RCVR) monitor_hex = 0;
         else           monitor_hex = 1;
         set_monitor_mode(1);
      }
      else if(first_key == '!') {    // !O command - monitor mode
         if     (monitor_port == DAC_PORT)       { strcpy(edit_buffer, "D"); monitor_hex = 0; }
         else if(monitor_port == ECHO_PORT)      { strcpy(edit_buffer, "E"); }
         else if(monitor_port == FAN_PORT)       { strcpy(edit_buffer, "F"); monitor_hex = 0; }
         else if(monitor_port == TICC_PORT)      { strcpy(edit_buffer, "I"); monitor_hex = 0; }
         else if(monitor_port == NMEA_PORT)      { strcpy(edit_buffer, "K"); monitor_hex = 0; }
         else if(monitor_port == THERMO_PORT)    { strcpy(edit_buffer, "N"); monitor_hex = 0; }
         else if(monitor_port == TRACK_PORT)     { strcpy(edit_buffer, "T"); monitor_hex = 0; }
         else if(monitor_port == RCVR_PORT)      { strcpy(edit_buffer, "R"); }
         else                                    { sprintf(edit_buffer, "%d", monitor_port); }

         if(monitor_hex) strcat(edit_buffer, " H");
         else            strcat(edit_buffer, " A");

         edit_info1 = "   Enter port id.  Include H for hex mode, A for ascii mode";
         edit_info2 = "   Port id # (0-9)  or";
         edit_info3 = "   R=receiver   D=DAC  E=echo  F=fan    I=TICC data   K=NMEA echo";
         edit_info4 = "   N=environmental     T=tracking info";
         edit_info5 = "   To exit monitor mode press any key";
         start_edit(MONITOR_CMD, "Monitor traffic on port  (ESC <cr> to abort)");
         return 0;
      }
      else if(first_key == 0) {  // O command - set misc options
//       if(luxor) return help_exit(c,99);
         strcpy(edit_buffer, "");
         edit_info1 = "A)mu   B)ins   C)ont   D)B   E)bolt   reF)resh adevs    G)tide options";
         edit_info2 = "H)ourly erase fixes    J)serial log   K)fault log   L)ive FFT    M)agnify plots";
         edit_info3 = "N)trendline update     P)eak scale    Q)ueue mode   R)eset bins  S)pike filter";
         edit_info4 = "T)rigger times         X)trend rate   Z)cursor time ref";
         start_edit(OPTION_CMD,   "Enter option letter and optional value (ESC to abort):");
         return 0;
      }
      else return help_exit(c,6);
   }
   else {  // help mode
      return help_exit(c,7);
   }

   return sure_exit();
}

void reset_parser()
{
   // reset serial port and message parser
   tsip_sync = 0;
   tsip_wptr = 0;
   tsip_rptr = 0;
   init_messages(110, 0);
   redraw_screen();
}

void start_calc(int why)
{
int i;

   if(deg_mode == 1.0) edit_info1 = "Calculator is in RADIANS mode";
   else                edit_info1 = "Calculator is in DEGREES mode";
   if(calc_rcvr) {
      edit_info2 =                   "To exit calculator (and program) type EXIT";
   }
   else {
      edit_info2 =                   "To exit calculator mode press ESC or <cr>";
   }
   if(zoom_screen == '`') {
      edit_info3 = "Enter ?<cr> for calculator command list.";
      if(calc_rcvr == 0) {
         edit_info4 = "When done, use GR to erase calculator results from plot area";
      }
   }
   else if(calc_rcvr == 0) {
      edit_info3 = "When done, use GR to erase calculator results from plot area";
   }


   i = rpn_mode;
   rpn_mode = (1);
   show_version_header();
   rpn_mode = i;
   show_rpn_help = 0;
   start_edit(CALC_CMD, "Enter RPN calculator commands separated by spaces. (ESC ESC to abort)");
}

int kbd_other(int c)
{
long val;
DATA_SIZE dval;
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
         new_queue(RESET_PLOT_Q, 16);
      }
      else if(first_key == 'f') {  // FP command - toggle PV filter
         if(luxor) return help_exit(c,99);
         pv_filter = toggle_value(pv_filter);
         set_filter_config(pv_filter, static_filter, alt_filter, kalman_filter, marine_filter, 1);
      }
      #ifdef ADEV_STUFF
         else if(first_key == 'a') {  // AP command - graph all PPS adev types
            if(luxor) return help_exit(c,99);
            if(ATYPE == A_MTIE) ATYPE = last_atype;
            aa_val = ALL_PPS;
            strcpy(edit_buffer, "G");
            if(rcvr_type == TICC_RCVR) {
               start_edit(ADEV_CMD, "Display chA:   A)devs only   G)raphs and all adevs   graphs and R)egular adevs");
            }
            else {
               start_edit(ADEV_CMD, "Display PPS:   A)devs only   G)raphs and all adevs   graphs and R)egular adevs");
            }
            return 0;
         }
      #endif
      else if(first_key == 'l') {  // LP command - open/close sat PRN data log file
         if(prn_file) {
            strcpy(edit_buffer, "Y");
            sprintf(out, "Close sat PRN data log file %s (Y/N)?  (ESC ESC to abort):", prn_name);
         }
         else if(prn_name[0]) {
            strcpy(edit_buffer, prn_name);
            if(prn_flush_mode) {
               if(strchr(edit_buffer, FLUSH_CHAR) == 0) strcat(edit_buffer, "*");
            }
            edit_info1 = "Append an '*' to the file name to flush file after every write"; // FLUSH_CHAR
            sprintf(out, "Enter name of sat PRN data log file to write  (ESC ESC to abort):");
         }

         start_edit(PRN_LOG_CMD, out);
         return 0;
      }
      else if(first_key == 'g') {  // GP command toggle PPS plot
         edit_plot(PPS, c);
         return 0;
      }
      else if(luxor && (first_key == 'p')) {  // PP command = load high voltage cutoff
         sprintf(edit_buffer, "%f", load_watts);
         start_edit(PP_CMD, "Enter the LED power cutoff in watts.  (ESC ESC to abort):");
         return 0;
      }
      else if((rcvr_type == CS_RCVR) && (first_key == 'p')) {  // PP command - slew PPS
         sprintf(edit_buffer, "%f", 0.0);
         start_edit(CS_SLEW_CMD, "Enter the desired 1PPS offset slew in seconds (-0.5 .. 0.5)  (ESC ESC to abort):");
         return 0;
      }
      else if(first_key == 't') { // TP command - set script run time
         if     (script_file) edit_buffer[0] = 0;
         else if(script_time && script_date) sprintf(edit_buffer, "%02d:%02d:%02d  %04d/%02d/%02d", script_hh,script_mm,script_ss, script_year,script_month,script_day);
         else if(script_time) sprintf(edit_buffer, "%02d:%02d:%02d", script_hh,script_mm,script_ss);
         else if(script_date) sprintf(edit_buffer, "%04d/%02d/%02d", script_year,script_month,script_day);
         else if(script_val) {
            if((script_val >= (24L*60L*60L)) && ((script_val % (24L*60L*60L)) == 0)) sprintf(edit_buffer, "%ld d", script_val/(24L*3600L));
            else if((script_val >= (3600L)) && ((script_val % 3600L) == 0)) sprintf(edit_buffer, "%ld h", script_val/(3600L));
            else if((script_val >= (60L)) && ((script_val % 60L) == 0)) sprintf(edit_buffer, "%ld m", script_val/(60L));
            else sprintf(edit_buffer, "%ld s", script_val);
         }
         else edit_buffer[0] = 0;

         edit_info1 = "Dates are in the format mm/dd/yyyy or yyyy/mm/dd";
         edit_info2 = "Intervals can be in seconds, minutes, hours or days like: 7s, 10m, 2h, 4d";
         if(edit_buffer[0]) start_edit(SCRIPT_RUN_CMD, "Enter timer,scr run time (and optional date) or interval or <ESC CR> to reset:");
         else               start_edit(SCRIPT_RUN_CMD, "Enter timer.scr run time (and optional date) or interval or <CR> to reset:");
         return 0;
      }
      #ifdef PRECISE_STUFF
         else if(first_key == 's') { // SP command - precison survey
            if(luxor) return help_exit(c,99);
            redraw_screen();
            if(precision_survey) {   // abort precision survey
               strcpy(edit_buffer, "Y");
               start_edit(ABORT_SURV_CMD, "Precise survey aborted! Save current position? Y)es N)o");
               return 0;
            }
            else {  // start precison survey
               strcpy(edit_buffer, "48");
               sprintf(out, "Enter number of hours to do survey for (3-%d,  ESC ESC to abort) : ", SURVEY_BIN_COUNT);
               start_edit(PRECISE_SURV_CMD, out);
               return 0;
            }
         }
      #endif
      else if(first_key == 'w') {  // WP command - Write plot area data to a log file
         strcpy(edit_buffer, "dump.log");
         sprintf(out, "Enter name of log file to write %sPLOT area info to (ESC ESC to abort):",
                       filter_log?"filtered ":"");
         start_edit(WRITE_CMD, out);
         log_mode = "w";
         dump_type = 'p';
         return 0;
      }
      else if(first_key == 'p') {  // PP command - toggle PPS polarity
         if(luxor) return help_exit(c,99);
         i = toggle_value(pps_polarity);
         user_pps_enable = 1;
         set_pps(user_pps_enable, i,  delay_value, pps1_delay, pps_threshold, 3);
         request_pps_info();
      }
      else if(first_key == 'z') {  // ZP command - zoom plot
         if(rcvr_type == NO_RCVR) return sure_exit();
         change_zoom_config(5);
         un_zoom = 0;
         zoom_screen = 'P';
         zoom_fixes = show_fixes;
         config_screen(118);
         force_adev_redraw(111);
      }
      else if(first_key == '!') {    // !P command - serial port parameters
         edit_info1 = "Note: this sets the system serial port config";
         edit_info2 = "      and not the receiver serial port config";
         if(com[RCVR_PORT].parity == 1) p = 'O';   //ppppp
         else if(com[RCVR_PORT].parity == 2) p = 'E';
         else p = 'N';
         sprintf(edit_buffer, "%d:%d:%c:%d", com[RCVR_PORT].baud_rate, com[RCVR_PORT].data_bits, p, com[RCVR_PORT].stop_bits);
         start_edit(BAUD_CMD, "Enter serial port settings like 9600:8:N:1   (ESC ESC to abort)");
         return 0;
      }
      else if(first_key == '&') {  // &p command - set pullin range
         if(have_pullin) sprintf(edit_buffer, "%d", pullin_range);
         else            sprintf(edit_buffer, "%d", 30);
         edit_info1 = "Value should be a multiple of 10";
         start_edit(PULLIN_CMD, "Enter pull-in range in PPB (10..2550, 0=UNLIMITED) (ESC ESC to abort)");
         return 0;
      }
      else if(first_key == '$') {  // $p command - Raspberry pi 800x480 screen
         new_screen(c);
         return 0;
      }
      else if(luxor && (first_key == 0)) { // P command - luxor protection values
         if(are_you_sure(c) != c) return 0;
         // next call to do_kbd() will process the selection character
      }
      else { // P command - time menu, select PPS menu
         if(first_key == 0) {  // xxxxxx
            if(rcvr_type == TICC_RCVR) {
               if(ticc_type == TAPR_TICC) {
                  if(osc_params == 0) osc_params = 1;
                  else                osc_params = 0;
               }
               redraw_screen();
            }
            if(are_you_sure(c) != c) return 0;
         }
         else return help_exit(c,666);

         // next call to do_kbd() will process the selection character
      }
   }
   else if(c == 'q') {
      if(first_key == 'g') { // GQ command - signal level map
         if(plot_signals == 4) plot_signals = 0;
         else                  plot_signals = 4;

         if(plot_signals == 0) {
            if(zoom_screen) {
               change_zoom_config(-106);
               cancel_zoom(16);     //zkzk
            }
         }
         if(plot_watch && plot_azel) {
            shared_plot = 1;
            prot_menu = 0;
         }
         config_screen(119);
      }
      else if(first_key == 's') {  // SQ command - plot single sat az/el/signal
         if(luxor) return help_exit(c,99);
         if(plot_prn == SUN_PRN) strcpy(edit_buffer, "SUN");
         else if(plot_prn == MOON_PRN) strcpy(edit_buffer, "MOON");
         else sprintf(edit_buffer, "%d", plot_prn);
         edit_info1 = "S=SUN (PRN 1000)    M=MOON (PRN 1001)";
         edit_info3 = "THIS WILL RESET THE DATA QUEUES";
         start_edit(PLOT_PRN_CMD, "Enter the sat/sun/moon PRN to plot az/el/signal (0=none) (ESC ESC to abort)");
         return 0;
      }
      else if(first_key == 't') {  // TQ command - show MJD digital clock
         show_mjd_time = toggle_value(show_mjd_time);
         show_julian_time = 0;
         show_unix_time = 0;
         show_gps_time = 0;
         show_msecs = 0;
         jd_elapsed = 0.0;
         show_elapsed_time = 0;
         clock_12 = 0;
         plot_digital_clock = 1;
         need_redraw = 4455;
      }
      else if(first_key == 'z') {  // ZQ command - zoom calendar
         change_zoom_config(66);
         un_zoom = 0;
         zoom_screen = 'Q';
         cal_year = pri_year;
         cal_month = pri_month;
         cal_day = pri_day;
         config_screen(1122);
         cal_adjust = 0;
      }
      else if(first_key == '$') {  // $q - use scaled vector fonts
         sprintf(edit_buffer, "%d", vc_font_scale);
         edit_info1 = "This command will draw characters using a scaled vector font.";
         edit_info2 = "A scale factor of 100 percent is an 8x16 font.";
         edit_info3 = "Enter 0 to use normal dot matrix fonts.";
         start_edit(VECTOR_FONT_CMD, "Enter scale factor for vector fonts in percent (50..500) (ESC ESC to abort)");
         return 0;
      }
      else return help_exit(c,8);
   }
   else if(c == 'r') {
      if(first_key == 'a') {  // AR - recalculate adevs
         strcpy(edit_buffer, "Y");
         start_edit(ADEV_RECALC_CMD, "Recalculate the ADEV tables? (Y/N)  (ESC ESC to abort)");
         return 0;
      }
      else if(first_key == 'e') { // ER command - revert EEPROM segments
         strcpy(edit_buffer, "0");
         edit_info1 = "Segment 255=ALL";
         start_edit(REVERT_SEG_CMD, "Enter eeprom segment number to revert to default values (0..255)  (ESC ESC to abort)");
         return 0;
      }
      else if(first_key == 'g') { // GR command - redraw screen
         reset_first_key(27);
         if(luxor == 0) {
            debug_text[0] = 0;
            debug_text2[0] = 0;
         }

//       reset_marks();
         osc_params = 0;
         erase_debug_info();
         leap_dumped = 0;

         first_show_bin = 0; // start adev displays from the first bin
         max_adev_width = 0;
         max_bins_shown = 0;
         force_adev_redraw(112);

         redraw_screen();
      }
#ifdef ADEV_STUFF
      else if(first_key == 'c') {  // CR command - reload adev or mtie queue from screen data
         if(luxor) return help_exit(c,99);
         reload_adev_queue(0);
      }
#endif
      else if(first_key == 'j') {   // JR command
         set_cs_sync("REAR");
      }
      else if(first_key == 'm') {  // MR command - set receiver raw observation messages rate
         sprintf(edit_buffer, "%d", raw_msg_rate);
         if((rcvr_type == TSIP_RCVR) && (res_t == RES_T)) {
            edit_info2 = "THIS RECEIVER REQUIRES A RAW RATE SETTING OF 3 SECONDS!";
         }
         start_edit(SET_RAW_CMD, "Enter raw satellite data oberrvation rate in seconds (ESC ESC to abort):");
         return 0;
      }
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
         if(rcvr_type == TICC_RCVR) {
            sprintf(edit_buffer, "%f", ticc_speed);
            edit_info1 = " ";
            edit_info2 = "THIS WILL RESET THE DATA QUEUES";
            start_edit(TICC_SPEED_CMD, "Enter the TICC reference clock in MHz  (ESC ESC to abort):");
            return 0;
         }
         else if(rcvr_type == CS_RCVR) { // PR - set remote mode
            sprintf(edit_buffer, "%d", cs_remote ^ 1);
            start_edit(CS_REMOTE_CMD, "Set remote mode  (ESC ESC to abort)");
            return 0;
         }
         else if((rcvr_type == STAR_RCVR) && (star_type == OSA_TYPE)) {
            sprintf(edit_buffer, "1");
            start_edit(STAR_CLEAR_WTR_CMD, "Enter '1' to reset the WTR timer  (ESC ESC to abort)");
            return 0;
         }
         else if(rcvr_type == SRO_RCVR) { // PR - set raw phase adjust
            sprintf(edit_buffer, "%g", sro_ra*SRO_TICK);
            edit_info1 = "Note: this values is always read back as +000";
            start_edit(SRO_RAW_CMD, "Set raw phase adjust in ns (-17066 .. 16933)  (ESC ESC to abort)");
            return 0;
         }
         else {
            user_pps_enable = 1;
            set_pps(user_pps_enable, 0,  delay_value, pps1_delay, pps_threshold, 3);
            request_pps_info();
         }
      }
      else if(first_key == 's') {  // SR command - change LLA scattergram reference
         sprintf(edit_buffer, "%.8lf %.8lf %.3lf", lat*RAD_TO_DEG, lon*RAD_TO_DEG, alt);
         edit_info1 = "The lat/lon values can be a decimal number or like 10d20m30s";
         edit_info2 = "The altiude value is in meters unless ended with an f or '";
         edit_info3 = "A value of 'A' says to use the current average location value.";
         edit_info4 = "Use ++ or -- as the sign of a value to move the reference";
         edit_info5 = "by the specified number of meters (or feet if in /t' mode)";
         start_edit(SET_LLA_REF_CMD, "Enter LLA plot reference lat lon alt (-=S,W +=N,E  alt in meters)  (ESC ESC to abort):");
         return 0;
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
         edit_info4 = "    Include a '!' to recalculate rise/settimes every second...";
         edit_info5 = "    ... (useful for for receivers that are moving).";
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
         un_zoom = 0;
         zoom_screen = 'S';
         zoom_fixes = show_fixes;
         plot_signals = 1;
         config_screen(120);
      }
      else if(first_key == '!') {  // !R command - set nav rate or enable an RFTG-m
         if(rcvr_type == RFTG_RCVR) {
            rftg_enable_cpu();
         }
         else {
            sprintf(edit_buffer, "%.0f", nav_rate);
            start_edit(NAV_RATE_CMD, "Enter navigation rate in Hz  (ESC ESC to abort)");
            return 0;
         }
      }
      else if(first_key == '$') {  // $r command - Reduced 1024x600 screen
         new_screen(c);
         return 0;
      }
      else if(first_key == '&') { // &R command - set reference input
         if(luxor) return help_exit(c,99);
         if(gpsdo_ref == 1) strcpy(edit_buffer, "PPS");
         else               strcpy(edit_buffer, "GPS");
         start_edit(REF_CMD, "Enter GPSDO reference source: G)ps  P)ps  (ESC ESC to abort):");
         return 0;
      }
      else if(first_key == 0) {  // R command read in a log file
         edit_buffer[0] = 0;
         if(luxor) {
            start_edit(READ_CMD, "Enter name of .LOG .CAL .CFG .WAV .RAW or .SCRipt file to read (ESC to abort):");
         }
         else {
            edit_info1 = "The file type is determined by the file name extension.";
            edit_info2 = "Valid extensions: .LOG .XML .GPX .ADV .LLA .PRN .SIG";
            edit_info3 = "                  .CAL .CFG .WAV .TIM .RAW .RPN or .SCRipt";
            start_edit(READ_CMD, "Enter name of file to read (ESC to abort):");
         }
         return 0;
      }
      else return help_exit(c,99);
   }
   else if(c == 's') {
      if(first_key == 'a') {  // AS command - set adev bin scaling sequence
         sprintf(edit_buffer, "%d", bin_scale);
         edit_info1 = "   1)1 per decade     2)1 per octave       3)3dB";
         edit_info2 = "   4)1-2-4 decades    5)1-2-5 decades      8)1-2-4-8 decades";
         edit_info3 = "   10)10 per decade   29)29 per decade    99)log spaced";
         start_edit(ADEV_BIN_CMD, "Select ADEV bin sequence");
         return 0;
      }
      else if(luxor && (first_key == 'b')) {  // BS command - sweep PWM value
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
         if(rcvr_type == CS_RCVR) {
            sprintf(edit_buffer, "%f", dac_voltage);
            edit_info1 = "THIS COMMAND WILL PUT THE UNIT INTO STANDBY MODE";
            start_edit(DACV_CMD, "Enter oscillator tune voltage (-100% .. +100%) (ESC ESC to abort):");
            return 0;
         }
         else if(rcvr_type == PRS_RCVR) {
            sprintf(edit_buffer, "%d,%d", prs_fc1, prs_fc2);
            edit_info1 = "FC1: (0..4095)    FC2: (1024..3072)";
            start_edit(DACV_CMD, "Enter FC control values (ESC ESC to abort):");
            return 0;
         }
         else if(rcvr_type == UCCM_RCVR) {
            sprintf(edit_buffer, "%f", uccm_voltage);
            start_edit(DACV_CMD, "Enter DAC control value (ESC ESC to abort):");
            return 0;
         }
         else if(rcvr_type == X72_RCVR) {
            set_x72_discipline(3);
         }
         else {
            sprintf(edit_buffer, "%f", dac_voltage);
            start_edit(DACV_CMD, "Enter DAC control voltage (ESC ESC to abort):");
            return 0;
         }
      }
      else if(first_key == 'f') {  // FS command - toggle STATIC or smoothing filter filter
         if(luxor) return help_exit(c,99);
         if(rcvr_type == FURUNO_RCVR) {
            sprintf(edit_buffer, "%d", smoothing_code);
            start_edit(SMOOTHING_CMD,  "Smoothing index:  1=quick  2=average  3=smoothed");
            return 0;
         }
         else {
            static_filter = toggle_value(static_filter);
            set_filter_config(pv_filter, static_filter, alt_filter, kalman_filter, marine_filter, 1);
         }
      }
      else if(first_key == 'g') { // GS command - toggle sound
////     sound_on = toggle_value(sound_on);
//       if(sound_on) BEEP(314);
         if(sound_on || beep_on) {
            sound_on = beep_on = 0;
         }
         else {
            sound_on = beep_on = 1;
            BEEP(314);
         }
         draw_plot(REFRESH_SCREEN);
      }
      else if(first_key == 'l') {  // LS command - stop logging
         if(log_file) {
            #ifdef ADEV_STUFF
               log_adevs();
            #endif
            log_stats();
            close_log_file();
            have_info &= (~INFO_LOGGED);
log_stream = 0;  // !!!!! do we want to do this?  cancel hex dump mode
         }
         else edit_error("Log file is not open.");
      }
      else if(first_key == 'm') {  // MS command - set v3.xx SBAS observation list
         strcpy(edit_buffer, sbas_obs_list);
         rinex_list = c;
         edit_info1 = "Enter a blank line for auto-detect.";
         start_edit(RINEX_LIST_CMD, "Enter the RINEX v3.xx SBAS observation list (ESC ESC to abort)");
         return 0;
      }
      else if(first_key == 's') {  // SS command - standard survey
         if(luxor) return help_exit(c,99);
         if(minor_alarms & MINOR_SURVEY) { // survey in progress,  stop self survey
            if(STOPABLE_SURVEY == 0) edit_info2 = "This receiver does not support stopping surveys, you can try anyway";
            strcpy(edit_buffer, "Y");
            start_edit(STOP_SURVEY_CMD, "Stop self survey (Y/N) (ESC ESC to abort):");
            return 0;
         }
         else if(rcvr_type == ESIP_RCVR) {
            if(do_survey) sprintf(edit_buffer, "%ld", do_survey);
            else          sprintf(edit_buffer, "%ld", 480L);
            start_edit(SURVEY_CMD, "Enter number of minutes for standard self survey (0..10800) (ESC ESC to abort):");
            return 0;
         }
         else if(rcvr_type == FURUNO_RCVR) {
            if(do_survey) sprintf(edit_buffer, "%ld", do_survey);
            else          sprintf(edit_buffer, "%ld", SURVEY_SIZE/1000);
            start_edit(SURVEY_CMD, "Enter number of hours for standard self survey (1..48) (ESC ESC to abort):");
            return 0;
         }
         else if(rcvr_type == NVS_RCVR) {
            if(do_survey) sprintf(edit_buffer, "%ld", do_survey);
            else          sprintf(edit_buffer, "%ld", SURVEY_SIZE/60);
            start_edit(SURVEY_CMD, "Enter number of minutes for self survey (20 .. 1440) (ESC ESC to abort):");
            return 0;
         }
         else if(rcvr_type == TRUE_RCVR) {
            if(do_survey) sprintf(edit_buffer, "%ld", do_survey);
            else          sprintf(edit_buffer, "%ld", SURVEY_SIZE/1000);
            start_edit(SURVEY_CMD, "Enter number of hours for standard self survey (ESC ESC to abort):");
            return 0;
         }
         else if(rcvr_type == TSIP_RCVR) {
            if(do_survey) sprintf(edit_buffer, "%ld", do_survey);
            else          sprintf(edit_buffer, "%ld", SURVEY_SIZE);
            start_edit(SURVEY_CMD, "Enter number of samples for standard self survey (ESC ESC to abort):");
            return 0;
         }
         else if(rcvr_type == UBX_RCVR) {
            if(do_survey) sprintf(edit_buffer, "%ld", do_survey);
            else          sprintf(edit_buffer, "%ld", SURVEY_SIZE);
            start_edit(SURVEY_CMD, "Enter number of seconds for standard self survey (ESC ESC to abort):");
            return 0;
         }
         else if(rcvr_type == VENUS_RCVR) {
            if(do_survey) sprintf(edit_buffer, "%ld", do_survey);
            else          sprintf(edit_buffer, "%ld", SURVEY_SIZE);
            start_edit(SURVEY_CMD, "Enter number of samples for self survey (60 .. 1209600) (ESC ESC to abort):");
            return 0;
         }
         else if(rcvr_type == ZODIAC_RCVR) {
            if(do_survey) sprintf(edit_buffer, "%ld", do_survey);
            else          sprintf(edit_buffer, "%ld", SURVEY_SIZE/1000);
            start_edit(SURVEY_CMD, "Enter number of hours for standard self survey (ESC ESC to abort):");
            return 0;
         }
         else  {
            strcpy(edit_buffer, "1");
            start_edit(SURVEY_CMD, "Enter <cr> to start standard survey.  (ESC ESC to abort):");
            return 0;
         }
      }
      else if(luxor && (first_key == 'p')) {  // PS command - temp2 over temperature
         sprintf(edit_buffer, "%f", tc2_ovt);
         start_edit(PS_CMD, "Enter temp sensor 2 over-temperature cutoff (degrees C).  (ESC ESC to abort):");
         return 0;
      }
      else if(first_key == 'p') {   // PS command - toggle PPS signal
         if(rcvr_type == TICC_RCVR) {
            sprintf(edit_buffer, "%c", 'M'); // ticc_syncmode);
            edit_info1 = "M)aster     S)lave";
            edit_info2 = " ";
            edit_info3 = "THIS WILL RESET THE DATA QUEUES";
            edit_info4 = "DO NOT SET SLAVE MODE UNLESS THE TICC IS CONNECTED TO A MASTER";
            start_edit(TICC_SYNC_CMD, "Enter the TICC sync mode.  (ESC ESC to abort):");
            return 0;
         }
         else if(rcvr_type == CS_RCVR) { // PS - set standby mode
            sprintf(edit_buffer, "%d", cs_standby ^ 1);
            start_edit(CS_STANDBY_CMD, "Set standby mode  (1=STANDBY  0=OPERATE)  (ESC ESC to abort)");
            return 0;
         }
         else if(rcvr_type == PRS_RCVR) {
            sprintf(edit_buffer, "%d", prs_ms);
            start_edit(PRS_MS_CMD, "Set magnetic switching mode (0=OFF  1=ON)  (ESC ESC to abort)");
            return 0;
         }
         else if(rcvr_type == SRO_RCVR) {
            sprintf(edit_buffer, "%d", sro_sy);
            edit_info1 = "0=OFF  1=NOW  2=ALWAYS  3=NOW+ALWAYS";
            start_edit(SRO_SY_CMD, "Set sync mode  (ESC ESC to abort)");
            return 0;
         }
         else if(rcvr_type == TM4_RCVR) {
            sprintf(edit_buffer, "%d", tm4_pps_source);
            start_edit(TM4_PPS_CMD, "Set PPS source (0..3) (ESC ESC to abort)");
            return 0;
         }
         else if(rcvr_type == X72_RCVR) {
            sprintf(edit_buffer, "%d", x72_sine_level);
            edit_info1 = "Note: most units only allow OFF or a fixed 40-50% output level";
            edit_info2 = "The SA22.c sine output is derived from the ASMOS output and might";
            edit_info3 = "not be controllable with this command.";
            start_edit(X72_SINE_CMD, "Set SINE output level (0=OFF or 40..100%) (ESC ESC to abort)");
            return 0;
         }
         else {
            if(pps_enabled) user_pps_enable = 0;
            else user_pps_enable = 1;
            set_pps(user_pps_enable, pps_polarity,  delay_value, pps1_delay,  pps_threshold, 4);
            request_pps_info();
         }
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
         un_zoom = 0;
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
         init_com(RCVR_PORT, 11);
         request_rcvr_info(223);  // ggggg
      }
      else if((first_key == 0) && luxor) {
         sprintf(edit_buffer, "%f %f %f", lat*RAD_TO_DEG, lon*RAD_TO_DEG, alt);
         start_edit(S_CMD, "Enter lat lon alt: (+ for N and E,  - for S and W,  alt in meters) (ESC ESC to abort)");
         return 0;
      }
      else {  // S command - survey menu
         if(first_key == 0) {  // xxxxxx
            if(are_you_sure(c) != c) return 0;
         }
         else return help_exit(c,666);
         // next call to do_kbd() will process the selection character
      }
   }
   else if(c == 't') {
      if(first_key == 'c') {  // CT command - trim leading entties off of plot queue
         edit_buffer[0] = 0;
         edit_info1 = "S)tart data    E)nd data";
         start_edit(TRIM_QUEUE_CMD, "Select data type(s) to remove from plot queue (ESC = abort)");
         return 0;
      }
      else if(first_key == 'f') {  // FT command - toggle Motorola troposphere filter
         if(rcvr_type == STAR_RCVR) {
            sprintf(edit_buffer, "%d", fix_star_ts);
            start_edit(STAR_TS_CMD, "Fix timestamp errors (1=YES  0=NO) (ESC ESC to abort)");
            return 0;
         }
         else {
            if(luxor) return help_exit(c,99);
            alt_filter = toggle_value(alt_filter);
            set_filter_config(pv_filter, static_filter, alt_filter, kalman_filter, marine_filter, 1);
         }
      }
      else if(first_key == 'g') {   // GT command - temperature graph
         if(rcvr_type == THERMO_RCVR) edit_plot(TEMP1, c);
         else edit_plot(TEMP, c);
         return 0;
      }
      #ifdef ADEV_STUFF
        else if(first_key == 'a') {  // AT command - set adev type to TDEV
           if(luxor) return help_exit(c,99);
           ATYPE = OSC_TDEV;
           last_atype = ATYPE;
           if(rcvr_type == TICC_RCVR) all_adevs = ALL_CHANS;
           else                       all_adevs = SINGLE_ADEVS;
           plot_adev_data = 1;
           force_adev_redraw(113);
           config_screen(121);
           if(adevs_active(0)) last_was_adev = c;
        }
      #endif
      #ifdef TEMP_CONTROL
         else if(first_key == 't') {  // TT command - active temperature control
            sprintf(edit_buffer, "%.3f", desired_temp);
            sprintf(out, "Enter desired operating temperature in %cC: (0=OFF  ESC ESC to abort)", DEGREES);
            start_edit(TEMP_SET_CMD, out);
            return 0;
         }
      #endif
      else if(luxor && (first_key == 'p')) {  // PT command - temp1 over temperature
         sprintf(edit_buffer, "%f", tc1_ovt);
         start_edit(PT_CMD, "Enter temp sensor 1 over-temperature cutoff (degrees C).  (ESC ESC to abort):");
         return 0;
      }
      else if(first_key == 'p') {  // PT command - set traim threshold / enable
         if(rcvr_type == BRANDY_RCVR) {
            sprintf(edit_buffer, "%c", brandy_code1);
            edit_info1 = "0=1kHz sine   1=IRIG-B   2=XR3/2137  3=VELA  4=NASA36";
            edit_info2 = "Include a 'D' for digital output on the PPS connector";
            start_edit(TIME_CODE_CMD, "Enter time code format to output (ESC ESC to abort):");
            return 0;
         }
         else if(rcvr_type == CS_RCVR) { // PT - set HP5071A time from system clock
            strcpy(edit_buffer, "U");
            if(0) {  // allow setting clock to local time
               edit_info1 = "";
               edit_info2 = "If the HP5071A time is set to local, Heather's clock displays will be wrong.";
               edit_info3 = "After setting local time, set Heather's time zone offset to 0";
               start_edit(CS_TIMESET_CMD, "Set HP5071A time to:  U)TC  or  L)ocal  (ESC ESC to abort)");
            }
            else {
               start_edit(CS_TIMESET_CMD, "Set HP5071A time to: U)TC  (ESC ESC to abort)");
            }
            return 0;
         }
         else if(rcvr_type == FURUNO_RCVR) {
            if(traim_mode) sprintf(edit_buffer, "%d", 1);
            else           sprintf(edit_buffer, "%d", 0);
            start_edit(TRAIM_CMD, "Enter TRAIM mode:  0=OFF  1=ON  (ESC ESC to abort):");
            return 0;
         }
         else if((rcvr_type == MOTO_RCVR) || (rcvr_type == ZODIAC_RCVR)) {
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
         else if(rcvr_type == PRS_RCVR) {
            sprintf(edit_buffer, "%d", prs_to);
            edit_info1 = "Enter '*' to set the TO value to null out the current TT value";
            start_edit(PRS_TO_CMD, "Enter time tag offset in ns (-32767 .. 32768)  (ESC ESC to abort):");
            return 0;
         }
         else if(rcvr_type == RFTG_RCVR) {
            sprintf(edit_buffer, "%d", traim_threshold);
            start_edit(TRAIM_CMD, "Enter traim 0 to tuen off or 1 to turn on (ESC ESC to abort):");
            return 0;
         }
         else if(rcvr_type == SRO_RCVR) {
            edit_info1 = "Allowable values are 0 .. 34000 ns";
            edit_info2 = "Window granularity is 133.333 ns";
            sprintf(edit_buffer, "%g", sro_tw*SRO_TICK);
            start_edit(SRO_WINDOW_CMD, "Enter tracking window half_width in ns (ESC ESC to abort):");
            return 0;
         }
         else if(rcvr_type == TICC_RCVR) {  // aaaacccc
            sprintf(edit_buffer, "%d %d", time2_a,time2_b);
            edit_info1 = " ";
            edit_info2 = "THIS WILL RESET THE DATA QUEUES";
            start_edit(TICC_TIME2_CMD, "Enter the chA and chB FIXED TIME2 (ESC ESC to abort)");
            return 0;
         }
         else if(rcvr_type == TM4_RCVR) {
            sprintf(edit_buffer, "%d", ett_code_format);
            edit_info1 = "0=IRIG-B   1=NASA36";
            start_edit(TIME_CODE_CMD, "Enter modulated time code format to output (ESC ESC to abort):");
            return 0;
         }
         else if(rcvr_type == X72_RCVR) {
            sprintf(edit_buffer, "%d", x72_pps);
            start_edit(X72_TIC_CMD, "Enter the TIC value (ESC ESC to abort):");
            return 0;
         }
         else return help_exit(c,99);
      }
      else if((first_key == 's') && (rcvr_type == TSIP_RCVR)) { // ST command - save location using single point surveys
         if(check_precise_posn) {
            strcpy(edit_buffer, "Y");
            start_edit(ABORT_LLA_CMD, "Lat/lon/alt search aborted! Save low resolution position? Y)es N)o");
         }
         else {
have_precise_lla = 0;
            if(have_precise_lla > 0) { // suggest the calculated value
               sprintf(edit_buffer, "%.8lf %.8lf %.3lf", precise_lat*RAD_TO_DEG, precise_lon*RAD_TO_DEG, precise_alt);
            }
            else {  // suggest the current receiver value
               sprintf(edit_buffer, "%.8lf %.8lf %.3lf", lat*RAD_TO_DEG, lon*RAD_TO_DEG, alt);
            }
            edit_info1 = "The lat/lon values can be a decimal number or like 10d20m30s";
            edit_info2 = "The altiude value is in meters unless ended with an f or '";
            start_edit(TRIMBLE_LLA_CMD, "Enter precise lat lon alt (-=S,W +=N,E  alt in meters)  (ESC ESC to abort):");
         }
         return 0;
      }
      else if(first_key == 'w') {  // WT command - open/close TICC data log file
         if(ticc_file) {
            strcpy(edit_buffer, "Y");
            sprintf(out, "Close TICC data log file (Y/N)?  (ESC ESC to abort):");
         }
         else if(ticc_name[0]) {
            strcpy(edit_buffer, raw_name);
            edit_info1 = "Append an '*' to the file name to flush file after every write";  // FLUSH_CHAR
            sprintf(out, "Enter name of TICC data log file to write  (ESC ESC to abort):");
         }

         start_edit(TICC_LOG_CMD, out);
         return 0;
      }
      else if(first_key == 'z') {  // ZT command - set zoom screen keyboard timeout
         sprintf(edit_buffer, "%d %c", idle_timeout, idle_screen);
         edit_info1 = " ";
         edit_info2 = "Follow the timeout value with the ZOOM type selector char to set";
         edit_info3 = "which zoomed screen to show when the keyboard idle time elapses.";
         edit_info4 = "Zoom types:  (A B C D E I L M N O P R S U V W X Y Z)";
         start_edit(KBD_TIMEOUT_CMD, "Enter the keyboard idle timeout in minutes (0=NONE) (ESC ESC to abort)");
         return 0;
      }
      else if(first_key == '$') {  // $t command - text mode screen
         new_screen(c);
         return 0;
      }
      else if(first_key == '&') { // &T command - change time constant
         if(luxor) return help_exit(c,99);
         if(rcvr_type == PRS_RCVR) {
            sprintf(edit_buffer, "%d", prs_pt);
            start_edit(TC_CMD, "Enter PLL time constant (ESC ESC to abort):");
         }
         else if(rcvr_type == SRO_RCVR) {
            sprintf(edit_buffer, "%d", (int) sro_tc);
            edit_info1 = "0=auto";
            sprintf(out, "Enter tracking loop time constant (0 .. 999999) in seconds (ESC ESC to abort):");
            start_edit(TC_CMD, out);
         }
         else if(rcvr_type == X72_RCVR) {
            sprintf(edit_buffer, "%d", (int) user_time_constant);
            sprintf(out, "Enter PLL time constant (%d .. %d) (ESC ESC to abort):", MIN_X72_TC,MAX_X72_TC);
            start_edit(TC_CMD, out);
         }
         else {
            sprintf(edit_buffer, "%f", user_time_constant);
            start_edit(TC_CMD, "Enter oscillator time constant in seconds (ESC ESC to abort):");
         }
         return 0;
      }
      else if(first_key == '!') {    // !T command - terminal emulator
         if     (term_port == DAC_PORT)       { strcpy(edit_buffer, "D"); }
         else if(term_port == ECHO_PORT)      { strcpy(edit_buffer, "E"); }
         else if(term_port == FAN_PORT)       { strcpy(edit_buffer, "F"); }
         else if(term_port == NMEA_PORT)      { strcpy(edit_buffer, "K"); term_hex = 0; }
         else if(term_port == RCVR_PORT)      { strcpy(edit_buffer, "R"); }
         else if(term_port == TRACK_PORT)     { strcpy(edit_buffer, "T"); }
         else if(term_port == THERMO_PORT)    { strcpy(edit_buffer, "N"); }
         else if(term_port == TICC_PORT)      { strcpy(edit_buffer, "I"); }
         else                                 { sprintf(edit_buffer, "%d", term_port); }
         if(term_hex) strcat(edit_buffer, " H");
         else         strcat(edit_buffer, " A");
         edit_info1 = "   Enter port id.  Include H for hex mode, A for ascii mode";
         edit_info2 = "   Port id # (0-9)  or   R=receiver  E=echo  I=TICC  K=NMEA echo  N=Environmental";
         start_edit(TERM_CMD, "Start terminal emulator on port  (ESC <cr> to abort)");
         return 0;

         enable_terminal = 1;
         do_term(term_port);

         tsip_sync = 0;
         tsip_wptr = 0;
         tsip_rptr = 0;
         if(rcvr_type == X72_RCVR) init_messages(10, 0);
         else                      init_messages(10, 1);
         reset_first_key(28);
         need_redraw = 6678;
      }
      else { // T command - time menu, select UTC or GPS time etc.
         if(first_key == 0) {  // xxxxxx
            if(are_you_sure(c) != c) return 0;
         }
         else return help_exit(c,666);
         // next call to do_kbd() will process the selection character
      }
   }
   else if(c == 'u') {  // Updates or UTC time
      if(first_key == 'g') {  // GU command - constant graph updates - we now always do this! or use OZ command
//       continuous_scroll = toggle_value(continuous_scroll);
//       config_screen(122);
         reset_marks();       // GU command - reset all markers
      }
      else if(first_key == 'm') {  // MU command - set RINEX antenna number
         strcpy(edit_buffer, antenna_number);
         edit_info1 = "Antenna number can be alphanumeric.";
         start_edit(RINEX_ANT_NUM_CMD, "Enter the antenna number (max 20 chars) (ESC ESC to abort)");
         return 0;
      }
      else if(luxor && (first_key == 'p')) {  // PU command - led under voltage cutoff
         sprintf(edit_buffer, "%f", load_lvc);
         start_edit(PU_CMD, "Enter LED under-voltage cutoff (in volts).  (ESC ESC to abort):");
         return 0;
      }
      else if((rcvr_type == PRS_RCVR) && (first_key == 'p')) {  // PU command - PPS enabled, unfiltered PPS input
         user_pps_enable = 1;
         set_pps(user_pps_enable, 1,  delay_value, pps1_delay,  pps_threshold, 4);
      }
      else if(first_key == 's') {  // SU command - set default UTC (leapsecond) offset
         sprintf(edit_buffer, "%d", utc_offset);
         edit_info1 = "This value is used until the receiver gets the value from the satellies.";
         start_edit(UTC_OFS_CMD, "Enter the default leap second (UTC) offset.  (ESC ESC to abort)");
         return 0;
      }
      else if(first_key == 'u') {  // UU command - toggle updates
         pause_data = toggle_value(pause_data);
         if(pause_data == 0) {  // we just released a pause
            dont_reset_queues = 0;

if(TICC_USED && (sim_file == 0)) {
   if((ticc_mode == 'T') || (ticc_mode == 'D')) {  // timestamp sequence was disrupted, reset adev queues
      new_queue(RESET_ADEV_Q, 50);
      new_queue(RESET_MTIE_Q, 50);
   }
}
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
         set_timing_mode(TMODE_UTC, 3);
         if(rcvr_type == NVS_RCVR) request_pps_info();
         request_timing_mode();
      }
      else if(first_key == 'w') {  // WU command - pause sim file
         if(sim_file) {  // file read from "R" keyboard command - close it
            sim_eof ^= 1;
            if(kbd_sim) {
               fclose(sim_file);
               sim_file = 0;
               kbd_sim = 0;
            }
            else if(sim_eof) sprintf(plot_title, "Reading simulation file %s paused.", sim_name);
            else             sprintf(plot_title, "Reading simulation file %s resumed.", sim_name);
            need_redraw = 7109;
         }
      }
      else if(first_key == 'z') {  // ZU command - zoom all signals
         if(rcvr_type == NO_RCVR) return sure_exit();
         change_zoom_config(26);
         un_zoom = 0;
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
         strcpy(edit_buffer, last_user_cmd);
         if(!ASCII_RCVR) edit_info1 = "(a string of hex values in ASCII separated by spaces)";
         start_edit(USER_CMD, "Enter command to send to the receiver (ESC ESC to abort):");
         return 0;
      }
      else if(first_key == '&') {  // &U command - lumen sensor sensitivity
         if(luxor) {
            sprintf(edit_buffer, "%.0f", (double) lux2_time);
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
      else if((first_key == 'p') && (rcvr_type == SRO_RCVR)) {  // PV command - freq save mode
         sprintf(edit_buffer, "%d", sro_fs);
         edit_info1 = "0=OFF  1=DAILY   2=INTEGRAL PART NOW    3=USER FREQ NOW";
         edit_info2 = "Note that the SRO100 EEPROM is spec'd for 10000 total writes";
         start_edit(SRO_FS_CMD, "Set frequency save mode  (ESC ESC to abort)");
         return 0;
      }
      else if(first_key == 'g') { // GV command - show lat/lon/alt (or phase residuals) or HP5071A power supply sum
         if(luxor) return help_exit(c,99);
         if(rcvr_type == TICC_RCVR) {
            if(two_ticc_mode()) {
               i = plot[ONE].show_plot + plot[TWO].show_plot + plot[THREE].show_plot + plot[FOUR].show_plot;
               if(i >= 4) {  // disable phase residual plots
                  plot[ONE].show_plot = 0;
                  plot[TWO].show_plot = 0;
                  plot[THREE].show_plot = 0;
                  plot[FOUR].show_plot = 0;
               }
               else { // enable phase residual plots
                  plot[ONE].show_plot = 1;
                  plot[TWO].show_plot = 1;
                  plot[THREE].show_plot = 1;
                  plot[FOUR].show_plot = 1;
               }
            }
            else {
               i = plot[ONE].show_plot + plot[TWO].show_plot + plot[THREE].show_plot + plot[FOUR].show_plot;
               if(i >= 2) {  // disable phase residual plots
                  plot[ONE].show_plot = 0;
                  plot[TWO].show_plot = 0;
                  plot[THREE].show_plot = 0;
                  plot[FOUR].show_plot = 0;
               }
               else { // enable phase residual plots
                  plot[ONE].show_plot = 1;
                  plot[TWO].show_plot = 1;
                  if(ticc_mode == 'L') plot[THREE].show_plot = 1;
                  else                 plot[THREE].show_plot = 0;
                  plot[FOUR].show_plot = 0;
               }
            }
         }
         else if(rcvr_type == CS_RCVR) {
            edit_plot(FOURTEEN, c);
            return 0;
         }
         else if(rcvr_type == THERMO_RCVR) {  // tide_kbd_cmd
//          i = plot[HUMIDITY].show_plot + plot[PRESSURE].show_plot + plot[TEMP1].show_plot + plot[TEMP2].show_plot;
//          if(i >= 4) {  // disable LLA plots
            i = plot[HUMIDITY].show_plot + plot[PRESSURE].show_plot + plot[TEMP1].show_plot;
            if(i >= 3) {  // disable enviro plots
               plot[HUMIDITY].show_plot = 0;
               plot[PRESSURE].show_plot = 0;
               plot[TEMP1].show_plot = 0;
               plot[TEMP2].show_plot = 0;
            }
            else { // enable enviro plots
               plot[HUMIDITY].show_plot = 1;
               plot[PRESSURE].show_plot = 1;
               plot[TEMP1].show_plot = 1;
               plot[TEMP2].show_plot = 0;
            }
         }
         else if(rcvr_type == TIDE_RCVR) {  // tide_kbd_cmd
            i = plot[ELEVEN].show_plot + plot[TWELVE].show_plot + plot[THIRTEEN].show_plot + plot[FOURTEEN].show_plot;
            if(i >= 4) {  // disable LLA plots
               plot[ELEVEN].show_plot = 0;
               plot[TWELVE].show_plot = 0;
               plot[THIRTEEN].show_plot = 0;
               plot[FOURTEEN].show_plot = 0;
            }
            else { // enable LLA plots
               plot[ELEVEN].show_plot = 1;
               plot[TWELVE].show_plot = 1;
               plot[THIRTEEN].show_plot = 1;
               plot[FOURTEEN].show_plot = 1;
            }
         }
         else {
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
            user_set_lla_plots = 1;
         }
         redraw_screen();
      }
      else if(first_key == 'm') {  // MV command - set marker number
         strcpy(edit_buffer, marker_number);
         edit_info1 = "The marker number can be alphanumeric.";
         start_edit(MARKER_NUM_CMD, "Enter the marker number (max 20 chars) (ESC ESC to abort)");
         return 0;
      }
      else if(first_key == 'z') {  // ZV command - zoom stuff
         if(rcvr_type == NO_RCVR) return sure_exit();
         change_zoom_config(26);
         un_zoom = 0;
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
            start_edit(VIEW_CMD, "Enter the plot view time in minutes/division: (A=all, T=auto, ESC ESC to abort):");
         }
         else {
            strcpy(edit_buffer, "0");
            edit_info1 = "or enter a time interval to display (like 20m, 3h, 2d)";
            start_edit(VIEW_CMD, "Enter the plot view time in minutes/division: (0=normal,  A=all,  T=auto):");
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
         config_screen(123);
      }
      else if(first_key == 'l') {  // LW command - open log in write mode
         if(log_file) {
            strcpy(edit_buffer, "Y");
            sprintf(out, "Log file %s is already open.  Close it? (Y/N)  (ESC ESC to abort)", log_name);
            start_edit(LOG_CLOSE_CMD, out);
            return 0;
         }
         else {
            log_mode = "w";
            strcpy(edit_buffer, log_name);
            if(0 && jd_obs) {
               edit_info3 = "Use extension .obs to write a RINEX .OBS log file";
            }
            edit_info2 = "Use extension .gpx .xml or .kml to write XML format logs.";
            edit_info1 = "Append an '*' to the file name to flush file after every write";  // FLUSH_CHAR
            start_edit(LOG_CMD, "Enter log file name to write data to: ");
            return 0;
         }
      }
      else if(first_key == 'm') {  // MW command - write RINEX file
         if(rinex_file) {
            strcpy(edit_buffer, "Y");
            sprintf(out, "RINEX file %s is already open.  Close it? (Y/N)  (ESC ESC to abort)", rinex_name);
            start_edit(RINEX_CLOSE_CMD, out);
            return 0;
         }
         else if((rcvr_type == TSIP_RCVR) && (res_t == RES_T) && (raw_msg_rate != 3)) {
            edit_error("THIS RECEIVER REQUIRES A RAW RATE SETTING OF 3 SECONDS!");
         }
         else if((rcvr_type == ESIP_RCVR) && (com[RCVR_PORT].baud_rate < 115200)) {
            edit_error("THIS RECEIVER REQUIRES A BAUD RATE >= 115200! Use !x to set the baud rate.");
         }
         else if(jd_obs == 0) {
            edit_error("No RINEX data observations available");
         }
         else {
rinex_name[0] = 0;   // always prompt with an IGS format RINEX name
            if(rinex_name[0] == 0) {
               set_rinex_name();
               strcpy(rinex_name, out);
            }
            strcpy(edit_buffer, rinex_name);
            edit_info1 = "Append an '*' to the file name to flush file after every write";  // FLUSH_CHAR
            start_edit(RINEX_FILE_CMD, "Enter RINEX file name to write observations to: ");
            return 0;
         }
      }
      else if(first_key == 'p') {  // PW command = battery power cutoff
         if(luxor) {
            sprintf(edit_buffer, "%f", batt_watts);
            start_edit(PW_CMD, "Enter the battery power cutoff in watts.  (ESC ESC to abort):");
            return 0;
         }
         else if(rcvr_type == SRO_RCVR) {
            edit_info1 = "Allowable values are 0 .. 999999 us.  0=OFF";
            edit_info2 = "Pulse width granularity is 133.333 ns";
            sprintf(edit_buffer, "%g", sro_pw*SRO_TICK/1000.0);
            start_edit(SRO_WIDTH_CMD, "Enter the PPS output pulse width in us.  (ESC ESC to abort):");
            return 0;
         }
         else if(rcvr_type == STAR_RCVR) {
            sprintf(edit_buffer, "%d", star_wtr);
            start_edit(STAR_SET_WTR_CMD, "Enter the wait-to-restore time in seconds (0 .. 2500)  (ESC ESC to abort):");
            return 0;
         }
         else if(TICC_USED) {
            sprintf(edit_buffer, "%g %g", cha_phase_wrap_interval, chb_phase_wrap_interval);
            edit_info1 = "The value specified should usually be 1.0/nominal_freq";
            edit_info2 = "A value of 0 disables phase un-wrapping";
            edit_info3 = "A value of -1 uses the current nominal freq";
            edit_info4 = " ";
            edit_info5 = "THIS WILL RESET THE DATA QUEUES";
            start_edit(PHASE_WRAP_CMD, "Enter the channel phase wrap intervals in seconds.  (ESC ESC to abort):");
            return 0;
         }
         else return help_exit(c,99);
      }
      else if(first_key == 't') {  // TW command - 12 hour digital clock
         clock_12 = toggle_value(clock_12);
         show_mjd_time = 0;
         show_julian_time = 0;
         show_unix_time = 0;
         show_gps_time = 0;
         show_msecs = 0;
         jd_elapsed = 0.0;
         show_elapsed_time = 0;
         plot_digital_clock = 1;
         need_redraw = 4455;
      }
      else if(first_key == 'z') {  // ZW command - zoom watch
         change_zoom_config(8);
         un_zoom = 0;
         zoom_screen = 'W';
         zoom_fixes = show_fixes;
         config_screen(124);
      }
      else {  // W command - write file menu
         if(first_key == 0) {  // xxxxxx
            if(are_you_sure(c) != c) return 0;
         }
         else return help_exit(c,666);
         // the next keystroke will select the data to write
      }
   }
   else if(c == 'x') {   // toggle view_interval between minutes and hours
      if(first_key == 'a') {  // AX command - exclude adev plot
         edit_buffer[0] = 0;
         if((adev_display_mask & DISPLAY_ADEV) == 0) strcat(edit_buffer, " A");
         if((adev_display_mask & DISPLAY_HDEV) == 0) strcat(edit_buffer, " H");
         if((adev_display_mask & DISPLAY_MDEV) == 0) strcat(edit_buffer, " M");
         if((adev_display_mask & DISPLAY_TDEV) == 0) strcat(edit_buffer, " T");

//       if((adev_display_mask & DISPLAY_MTIE) == 0) strcat(edit_buffer, " I");

         if((adev_display_mask & DISPLAY_CHA) == 0) strcat(edit_buffer, " P");
         if((adev_display_mask & DISPLAY_CHB) == 0) strcat(edit_buffer, " O");
         if((adev_display_mask & DISPLAY_CHC) == 0) strcat(edit_buffer, " C");
         if((adev_display_mask & DISPLAY_CHD) == 0) strcat(edit_buffer, " D");

         edit_info1 = " ";
         edit_info2 =    "xDEV plots to hide: A)dev  H)dev  M)dev  T)dev";
         if(rcvr_type == TICC_RCVR) {
            edit_info3 = "                    P)chA  O)chB  C)chC  D)chD";
         }
         else {
            edit_info3 = "                    P)PS   O)SC   C)chC  D)chD";
         }
         if(edit_buffer[0]) {  // some plots(s) are hidden
            start_edit(ADEV_HIDE_CMD, "Enter plot types to hide (ESC <cr>=none)  (ESC ESC to abort)");
         }
         else {  // nothing is hidden
            start_edit(ADEV_HIDE_CMD, "Enter plot types to hide (ESC ESC to abort)");
         }
         return 0;
      }
      else if(first_key == 'f') {  // FX command - PDOP mask/switch
         if(luxor) return help_exit(c,99);
         sprintf(edit_buffer, "%.1f", pdop_mask);
         if(rcvr_type == FURUNO_RCVR) {
            start_edit(SET_PDOP_CMD, "Enter PDOP mask 2D/3D switch value (0 .. 10)");
         }
         else {
            start_edit(SET_PDOP_CMD, "Enter PDOP mask value (switch 2D/3D mode at mask*0.75)");
         }
         return 0;
      }
      else if(first_key == 'g') { // GX command - show dops
         if(luxor) return help_exit(c,99);
         plot_dops = toggle_value(plot_dops);
//       if(plot_dops) request_sat_list();
         user_set_dops = 2;
         redraw_screen();
      }
      else if(first_key == 'h') {  // HX command - exit holdover
         if(rcvr_type == PRS_RCVR)      set_discipline_mode(SET_DIS_MODE_HOLDOVER);
         else if(rcvr_type == X72_RCVR) set_discipline_mode(SET_DIS_MODE_HOLDOVER);
         else                           set_discipline_mode(SET_DIS_MODE_NORMAL);
      }
      else if(first_key == 'm') {  // MX command - set receiver baud rate
         goto set_baud;
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
         if(end_time && end_date) sprintf(edit_buffer, "%02d:%02d:%02d  %04d/%02d/%02d", end_hh,end_mm,end_ss, end_year,end_month,end_day);
         else if(end_time) sprintf(edit_buffer, "%02d:%02d:%02d", end_hh,end_mm,end_ss);
         else if(end_date) sprintf(edit_buffer, "%04d/%02d/%02d", end_year,end_month,end_day);
         else if(exit_val) {  // exit countdown timer set
            if((exit_val >= (24L*3600L)) && ((exit_val % 24L*3600L) == 0)) sprintf(edit_buffer, "%ld d", exit_val/(24L*3600L));
            else if((exit_val >= (3600L)) && ((exit_val % 3600L) == 0)) sprintf(edit_buffer, "%ld h", exit_val/(3600L));
            else if((exit_val >= (60L)) && ((exit_val % 60L) == 0)) sprintf(edit_buffer, "%ld m", exit_val/(60L));
            else sprintf(edit_buffer, "%ld s", exit_val);
         }
         else edit_buffer[0] = 0;

         edit_info1 = "Dates are in the format mm/dd/yyyy or yyyy/mm/dd";
         edit_info2 = "Intervals can be in seconds, minutes, hours or days like: 7s, 10m, 2h, 4d";
         if(edit_buffer[0]) start_edit(EXIT_CMD, "Enter exit time (and optional date) or interval or <ESC CR> to reset:");
         else               start_edit(EXIT_CMD, "Enter exit time (and optional date) or interval or <CR> to reset:");
         return 0;
      }
      else if(first_key == 'w') {  // WX command - open/close debug log file
         if(debug_file) {
            edit_buffer[0] = 0;
            strcpy(edit_buffer, "Y");
            sprintf(out, "Close debug log file %s (Y/N)?  (ESC ESC to abort):", debug_name);
         }
         else if(debug_name[0]) {
            strcpy(edit_buffer, debug_name);
            if(dbg_flush_mode) {
               if(strchr(edit_buffer, FLUSH_CHAR) == 0) strcat(edit_buffer, "*");
            }
            edit_info1 = "Append an '*' to the file name to flush file after every write"; // FLUSH_CHAR
            sprintf(out, "Enter name of debug log file to write  (ESC ESC to abort):");
         }

         start_edit(DEBUG_LOG_CMD, out);
         return 0;
      }
      else if(first_key == 'z') {  // ZX command - zoom all maps and watch
         if(rcvr_type == NO_RCVR) return sure_exit();
         change_zoom_config(16);
         un_zoom = 0;
         zoom_screen = 'X';
         zoom_fixes = show_fixes;
         plot_signals = 4;
         config_screen(120);
      }
      else if(first_key == '&') { // &X command - change max voltage
         if(luxor) return help_exit(c,99);
         sprintf(edit_buffer, "%f", user_max_volts);
         start_edit(MAXV_CMD, "Enter maximum EFC control voltage (ESC ESC to abort):");
         return 0;
      }
      else if(first_key == '!') {  // !X command - set receiver baud rate
         set_baud:
         if(rcvr_type == TM4_RCVR) {
            if     (tm4_time_baud == 0) i = 1200;
            else if(tm4_time_baud == 1) i = 2400;
            else if(tm4_time_baud == 2) i = 4800;
            else if(tm4_time_baud == 3) i = 9600;
            else if(tm4_time_baud == 4) i = 19200;
            else if(tm4_time_baud == 5) i = 38400;
            else if(tm4_time_baud == 6) i = 57600;
            else if(tm4_time_baud == 7) i = 115200;
            else                        i = 0;
            if(i) sprintf(edit_buffer, "%d", i);
            else edit_buffer[0] = 0;
            start_edit(SET_BAUD_CMD, "Enter time port baud rate to set (ESC ESC to abort):");
         }
         else {
            sprintf(edit_buffer, "%d", com[RCVR_PORT].baud_rate);
            edit_info1 = "WARNING: if you set the receiver baud rate to its non-default value you will";
            edit_info2 = "         need to use the /br= command whenever you start Heather!";
            start_edit(SET_BAUD_CMD, "Enter receiver baud rate to set (ESC ESC to abort):");
         }
         return 0;
      }
      else if(first_key == '$') {  // $X command - extra large screen
         new_screen(c);
         return 0;
      }
      else if(first_key == 0) {  // X command - 1 hr/division
         view_all_data = 0;
         new_view();
         hide_kbd(1);
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
         user_set_dfilter = 2;
         redraw_screen();
      }
      else if(first_key == 'p') {  // PY command - sync HP5071A outputs
         if(rcvr_type == PRS_RCVR) {
            sprintf(edit_buffer, "%d %d %d", prs_sp[0], prs_sp[1], prs_sp[2]);
            start_edit(PRS_SP_CMD, "Enter the three synthesizer control values (R N A)  (ESC ESC to abort):");
            return 0;
         }
         else if(rcvr_type == CS_RCVR) {
            edit_info1 = "Allowable sources are OFF, FRONT, or REAR";
            strcpy(edit_buffer, cs_sync);
            sprintf(out, "Sync 1PPS outputs to external source  (ESC ESC to abort):");
            start_edit(CS_SYNC_CMD, out);
            return 0;
         }
      }
      else if(first_key == 'w') {  // WY command - open/close receiver data capture file
         if(raw_file) {
            strcpy(edit_buffer, "Y");
            sprintf(out, "Close receiver data capture file %s (Y/N)?  (ESC ESC to abort):", raw_name);
         }
         else if(raw_name[0]) {
            strcpy(edit_buffer, raw_name);
            if(raw_flush_mode) {
               if(strchr(edit_buffer, FLUSH_CHAR) == 0) strcat(edit_buffer, "*");
            }
            edit_info1 = "Append an '*' to the file name to flush file after every write"; // FLUSH_CHAR
            sprintf(out, "Enter name of receiver data capture file to write  (ESC ESC to abort):");
         }

         start_edit(RAW_LOG_CMD, out);
         return 0;
      }
      else if(first_key == 'z') {  // ZY command - zoom all maps
         if(rcvr_type == NO_RCVR) return sure_exit();
         change_zoom_config(26);
         un_zoom = 0;
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
            if     (SCREEN_WIDTH > WIDE_WIDTH)    day_plot = 26;
            else if(SCREEN_WIDTH > 1024)          day_plot = 26;
            else if(SCREEN_WIDTH > NARROW_SCREEN) day_plot = 25;
            else day_plot = 26;
         }
         hide_kbd(2);
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
      else if(first_key == 'm') {  // MZ command - set RINEX site name
         strcpy(edit_buffer, rinex_site);
         start_edit(RINEX_SITE_CMD, "Enter the site name. (ESC ESC to abort):");
         return 0;
      }
      else if(luxor && (first_key == 'p')) {  // PZ command = load high voltage cutoff
         sprintf(edit_buffer, "%f", auxv_lvc);
         start_edit(PZ_CMD, "Enter the AUXV low voltage cutoff in volts.  (ESC ESC to abort):");
         return 0;
      }
      else if((rcvr_type == TICC_RCVR) && (first_key == 'p')) {  // PZ command = set channel time interval zero references
//sprintf(debug_text, "offset:%.12f  %.12f  %.12f  %.12f   apref:%.12f", pps_offset,pps_offset,chc_offset,chd_offset, pps_adev_period);
         if(cha_count) cha_ref = (pps_phase/cha_count)/1.0E9;
         else          cha_ref = last_cha_interval;
         cha_ref -= pps_adev_period;

         if(chb_count) chb_ref = (osc_phase/chb_count)/1.0E9;
         else          chb_ref = last_chb_interval;
         chb_ref -= osc_adev_period;

         if(chb_count) chc_ref = (chc_phase/chc_count)/1.0E9;
         else          chc_ref = last_chc_interval;
         chc_ref -= chc_adev_period;

         if(chd_count) chd_ref = (chd_phase/chd_count)/1.0E9;
         else          chd_ref = last_chd_interval;
         chd_ref -= chd_adev_period;
new_queue(RESET_ALL_QUEUES, 55);
//sprintf(debug_text2, "zref:%.12f  %.12f  %.12f  %.12f", cha_ref,chb_ref, pps_phase/cha_count/1.0E9,osc_phase/chb_count/1.0E9);
BEEP(315);  // toots
      }
      else if((rcvr_type == X72_RCVR) && (first_key == 'p')) {  // PZ command = set X72 DDS tuning offset
         sprintf(edit_buffer, "%g", last_x72_dds);
         edit_info1 = "Tune resolution is 2.04E-12 parts";
         edit_info2 = "Note: offsets > 4.0E-8 will be done in steps of 4.0E-8 parts.  This can take";
         edit_info3 = "      several seconds to complete";
         start_edit(X72_DDS_CMD, "Enter the DDS tuning offset (+/- 1.0E-6 parts)  (ESC ESC to abort):");
         return 0;
      }
      else if(first_key == 's') {  // SZ command - select data to send out TRACK_PORT
         edit_buffer[0] = 0;
         if(track_port_info & SEND_PRN)     sprintf(edit_buffer, "%d", track_prn);
         if(track_port_info & SEND_SUN)     strcat(edit_buffer, " S");
         if(track_port_info & SEND_MOON)    strcat(edit_buffer, " M");
         if(track_port_info & SEND_SATS)    strcat(edit_buffer, " A");
         if(track_port_info & SEND_TIME)    strcat(edit_buffer, " T");
         if(track_port_info & SEND_HIGHEST) strcat(edit_buffer, " H");
         edit_info1 = "M=MOON  S=MOON  T=Time (UTC)";
         edit_info2 = "A=All sats      H=highest sat    #=single sat PRN #      (ESC CR=none)";
         start_edit(TRACK_PORT_CMD, "Select the data to send out the TRACK_PORT (if enabled) (ESC ESC to abort):");
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
            edit_info1 = "TZ string can also be in Linux format (like CST6CDT)";
            start_edit(SET_TZ_CMD, "Enter time zone string (like -6:00CST/CDT)  (ESC CR to reset  ESC ESC to abort:");
         }
         else {
            edit_buffer[0] = 0;
            edit_info1 = "String can also be in Linux format (like CST6CDT)";
            start_edit(SET_TZ_CMD, "Enter time zone string (like -6:00CST/CDT)  ESC to abort:");
         }
         return 0;
      }
      else if(first_key == 'w') {  // WZ command - Output az/el signal level file
         if(luxor) return help_exit(c,99);
         sprintf(edit_buffer, "%s.sig", unit_file_name);
         sprintf(out, "Enter file to write current AZ/EL/SIG info to (ESC ESC to abort):");
         start_edit(WRITE_SIGS_CMD, out);
         return 0;
      }
      else if(first_key == 'z') {  // ZZ command - cancel zoom
         if(zoom_screen) {
            remove_zoom();
         }
      }
      else if(first_key == '$') {  // $z command - Oprah sized screen
         new_screen(c);
         return 0;
      }
      else if(first_key == '!') {    // !Z command - reset parser
         reset_parser();
      }
      else if(first_key == 0) {  // z command - zoom menu
         if(zoom_screen == 'K') {
            show_touch_kbd(1);
            if(are_you_sure(c) != c) return 0;
         }
         else if(zoom_screen) {
            change_zoom_config(-107);
            cancel_zoom(17);     //zkzk
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
      else if(first_key == 0) {  // ~ command - fixup a glitch point in the plot queue data
         deglitch_queue_point();
      }
      else if(first_key) return help_exit(c, 99);  // xxxxxx
//    else return help_exit(c,99);
   }
   else if(c == '}') {  // } command - grow plot
      if(first_key == 0) {
         grow_plot();
         draw_plot(REFRESH_SCREEN);
         last_plot_key = c;
         hide_kbd(3);  // no_unzoom
      }
      else return help_exit(c,99);
   }
   else if(c == '{') {  // } command - shrink plot
      if(first_key == 0) {
         shrink_plot();
         draw_plot(REFRESH_SCREEN);
         last_plot_key = c;
         hide_kbd(4);  // no_unzoom
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
            zoom_review(val, REVIEW_BEEP);
            draw_plot(REFRESH_SCREEN);
            hide_kbd(27);
         }
         else {  // no_unzoom
            move_plot_up();
            draw_plot(REFRESH_SCREEN);
            last_plot_key = c;
            hide_kbd(271);
         }
      }
      else if(first_key == 'g') {
         if(1) { // g+ command - turn on all plots with changing data
            for(i=0; i<=FOURTEEN; i++) {
               plot[i].show_plot = 0;
               dval = fabs(plot[i].max_disp_val - plot[i].min_disp_val);

               if(i == TEN) {  // MSGJIT plot
                  if(rcvr_type == CS_RCVR) ;
                  else if(luxor) ;
                  else continue;
               }
               else if(i == NINE) {  // MSGOFS plot
                  if(rcvr_type == CS_RCVR) ;
                  else if(luxor) ;
                  else continue;
               }

               if(dval >= 1.0E-10) {
                  plot[i].show_plot = 1;
               }
            }
//          if(adevs_active(0)) plot_adev_data = 1;
            if(NO_SATS == 0) plot_sat_count = 1;
         }
         else { // g+ command - turn on all plots
            if(luxor) i = NUM_PLOTS+DERIVED_PLOTS-1;
            else      i = NUM_PLOTS+DERIVED_PLOTS-1;
            for(; i>=0; i--) {
               if(plot[i].show_plot == 0) toggle_plot(i);
            }
            plot_adev_data = 1;
            plot_sat_count = 1;
         }
      }
      else return help_exit(c,99);
   }
   else if(c == '-') {  // - command
      if(first_key == 0) {
         if(last_was_mark) {  // clear numeric marker
            if(last_was_mark <= '9') mark_q_entry[last_was_mark-'0'] = 0;
            else if(last_was_mark >= 'a') mark_q_entry[last_was_mark-'a'+10] = 0;
            else if(last_was_mark >= 'A') mark_q_entry[last_was_mark-'A'+10] = 0;
            last_was_mark = 0;
            draw_plot(REFRESH_SCREEN);
            hide_kbd(26);
         }
         else {  // no_unzoom
            move_plot_down();
            draw_plot(REFRESH_SCREEN);
            last_plot_key = c;
            hide_kbd(261);
         }
      }
      else if(first_key == 'g') {   // g- command - hide all plots
         if(luxor) i = NUM_PLOTS+DERIVED_PLOTS-1;
         else      i = NUM_PLOTS+DERIVED_PLOTS-1;
         for(; i>=0; i--) {
            if(plot[i].show_plot) toggle_plot(i);
         }
         plot_adev_data = 0;
         adev_decades_shown = 0;
         plot_sat_count = 0;
      }
      else return help_exit(c,99);
   }
   else if(c == '=') { // = command - assign next unused marker
      if(first_key == 0) {
         if(set_next_marker()) {
            last_plot_key = c;
            hide_kbd(22);
            return sure_exit();
         }
         hide_kbd(24);
      }
      else {
         return help_exit(c,99);
      }
   }
   else if(c == '!') {  // ! command - reset unit, etc
      if(first_key) return help_exit(c,99);  // xxxxxx

      if(are_you_sure(c) != c) return 0;
      // the next keystroke will select the reset type
   }
   else if(c == '&') {  // & command - edit osc param
      if(luxor) {
         if(first_key == 'w') { // W& command - write luxor cal data script file
            strcpy(edit_buffer, "LUXCAL.SCR");
            start_edit(WRITE_CMD, "Enter name of calibration data script file to write (ESC ESC to abort):");
            dump_type = 'c';
            return 0;
         }
         if(are_you_sure(c) != c) return 0;
         else                     return sure_exit();
      }
if(first_key && (osc_params == 0)) { // xxxxxx
   return help_exit(c, 99);
}

      if(rcvr_type == TICC_RCVR) osc_params = 0;
      else if(rcvr_type == PRS_RCVR) osc_params = 0;
      else if(rcvr_type == X72_RCVR) osc_params = 0;
      else if(osc_params == 1) osc_params = 0;
      else osc_params = 1;   // replace sat info table with osc parameter display

      redraw_screen();

      if(are_you_sure(c) != c) {
         user_time_constant = time_constant;
         user_damping_factor = damping_factor;
         user_holdover_time = x72_holdover_val;
         user_jam_sync = jam_sync;

         user_osc_gain = osc_gain;
         user_min_volts = min_volts;
         user_max_volts = max_volts;
         user_min_range = min_dac_v;
         user_max_range = max_dac_v;
         user_max_freq_offset = max_freq_offset;
         user_initial_voltage = initial_voltage;
         if(com[RCVR_PORT].process_com == 0) {
            show_satinfo();
         }
         return 0;
      }
      else if(com[RCVR_PORT].process_com == 0) {
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
         edit_info1 =         "  A)vg   R)ms   S)td dev   V)ar";
         edit_info2 =         "  mi)N   ma)X   sP)an";
         start_edit(STAT_CMD, "Select plot header statistic to display for all plots (<cr>=hide):");
         return 0;
      }
      else if(first_key == 0) {
         no_auto_erase = 1;
#ifdef WINDOWS
         strcpy(edit_buffer, "/");
#else  // __linux__  __MACH__   __FreeBSD__
         strcpy(edit_buffer, "-");
#endif
         edit_info1 = "Options must begin with a '-' or '/' and be separated by spaces.";
         start_edit(SWITCH_CMD, "Enter one or more command line options to process (ESC ESC) to abort:");
         return 0;
      }
      else return help_exit(c,99);
   }
   else if(c == '\\') {   // \ command - dump screen image
      if(first_key == 0) {   // screen dump from touch screen keyboard
         dump_screen(invert_dump, 0, unit_file_name);
      }
      else return help_exit(c,99);
   }
   else if(c == '$') {   // $ command - set screen size
      if(first_key == 'g') {  // G$ command - edit sat count plot
         edit_plot(SAT_PLOT, c);
         user_set_sat_plot = 1;
         return 0;
      }
      else if(first_key) return help_exit(c,99); // xxxxxx
      if(are_you_sure(c) != c) return 0;
      // the next char will select the screen size
   }
   else if(c == '`') {  // ` command - calculator mode
      if((first_key == 0) || (first_key == 'z')) { // Z` command - zoomed calculator
         change_zoom_config(6);
         un_zoom = 0;
         zoom_screen = '`';
         config_screen(1122);
         edit_buffer[0] = 0;
         edit_buffer[1] = 0;
         start_calc(12);

         rpn_mode = (-1);   // kludge to get menu onto the screen
         add_kbd(' ');
         add_kbd(0x0D);
      }
      else if(first_key == '!') {  // !` command - edit DEFINE
         start_edit(EDIT_DEFINE_CMD, "Edit user calculator command");
         return 0;
      }
      else if(0 && (first_key == 0)) {  // toots
         edit_buffer[0] = 0;
         edit_buffer[1] = 0;
         start_calc(11);
         return 0;
      }
      else {
         return help_exit(c,99);
      }
   }
   else if(c == '%') {   // % command - goto next holdover/error event
      if(first_key == 0) {
         val = 0;
         if(plot_skip_data)     val |= TIME_SKIP;
         if(plot_holdover_data) val |= HOLDOVER;
         if(plot_temp_spikes)   val |= TEMP_SPIKE;
         if(val == 0)           val |= (HOLDOVER | TIME_SKIP | TEMP_SPIKE);
         goto_event((u16) val);
         last_plot_key = c;
         hide_kbd(7);  // no_unzoom
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
   else if((c >= '0') && (c <= '9')) {  // digit command
      if((first_key == 'p') && (c == '1')) {  // p1 command - set 1pps mode
         if(luxor) return help_exit(c,99);
         if(rcvr_type == CS_RCVR) {
            sprintf(edit_buffer, "%.0lf", cs_freq1);
            edit_info1 = "Valid frequencies are 5 or 10 MHz";
            start_edit(PPS1_CFG_CMD, "Enter output 1 frequency  (ESC ESC to abort):");
            return 0;
         }
         else if(rcvr_type == TM4_RCVR) {
            edit_info1 = "0=10 MHz   1=5 MHz   2=1 MHz   3=100 kHz   4=10kHz   5=1kHz";
            edit_info2 = "6=baseband IRIG      7=1 PPS   8=OFF";
            sprintf(edit_buffer, "%d", tm4_code1);
            start_edit(PPS1_CFG_CMD, "Enter mux1 frequency code  (ESC ESC to abort):");
            return 0;
         }
         else {
            set_pps_mode(RATE_1PPS);
            request_pps_mode();
            if(res_t) save_segment(0xFF, 2);
         }
      }
      else if((first_key == 'p') && (c == '2')) {  // p2 command - set pp2s/100 pps mode
         if(luxor) return help_exit(c,99);
         if(rcvr_type == CS_RCVR) {
            sprintf(edit_buffer, "%.0lf", cs_freq2);
            edit_info1 = "Valid frequencies are 5 or 10 MHz";
            start_edit(PPS2_CFG_CMD, "Enter output 2 frequency  (ESC ESC to abort):");
            return 0;
         }
         else if(rcvr_type == TM4_RCVR) {
            edit_info1 = "0=10 MHz   1=mux1   2=PPS   3=option1   4=option2   5=option3";
            edit_info2 = "6=baseband IRIG     7=baseband NASA36   8=OFF";
            sprintf(edit_buffer, "%d", tm4_code2);
            start_edit(PPS2_CFG_CMD, "Enter mux2 frequency code  (ESC ESC to abort):");
            return 0;
         }
         else {
            set_pps_mode(RATE_PP2S);
            request_pps_mode();
            if(res_t) save_segment(0xFF, 3);
         }
      }
      else if((first_key == 'p') && (c == '3')) {  // p3 command - set 10000 pps mode
         if(luxor) return help_exit(c,99);
         set_pps_mode(RATE_10000PPS);
         request_pps_mode();
         if(res_t) save_segment(0xFF, 4);
      }
      else if(first_key == 'g') {  // G1 .. G9 commands - select auxiliary plots
         if(c == '0') val = FIRST_EXTRA_PLOT + 10 - 1;
         else         val = FIRST_EXTRA_PLOT + (c-'0') - 1;

         if(rcvr_type == TIDE_RCVR) {  // use tide_kbd_cmd tide plots for G1/G2/G3/G6
            if(val == ONE) val = HUMIDITY;
            else if(val == TWO) val = PRESSURE;
            else if(val == THREE) val = TEMP1;
            else if(val == SIX) val = TEMP2;
         }

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
            if     (c == '2') do_fixes(RCVR_MODE_2D);     // S2 command - 2D fixes
            else if(c == '3') do_fixes(RCVR_MODE_3D);     // S3 command - 3D mode
            else if(c == '4') do_fixes(RCVR_MODE_NO_SATS);     // S4 command - undocumented mode 2
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
   // check if 'c' is a plot area control char

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
int old_lwa;
int old_rpn;

   // This routine processes a keystroke character.
   //
   // If variable "first_key" is set, this key stroke is the second keystroke
   // of the two character command that started with character "first_key".


   if(sound_alarm) {    // any keystroke turns off alarm clock
      if(sound_alarm & ALARM_DATE) {  // date specifed, so reset alarm
         alarm_time = alarm_date = 0;
         alarm_jd = 0.0;
         egg_timer = egg_val = 0;
         repeat_egg = 0;
      }

      reset_alarm();   // reset the alarm
      sound_alarm = 0;
      return 0;
   }

   if(disable_kbd > 1) return 0;  // nothing gets by

   fix_flag = 0;
   if(plot_lla || lla_showing) fix_flag = 1;

   if((zoom_screen == 'K') || last_null_key) {  // ZOOM_KBD touchscreen keyboard active
      // oh, the crap we have to do in order to make the zoomed calculator
      // touch screen keyboard work...

      if(((c == 0x0D) || (c == ESC_CHAR)) && last_zoom_calc) {  // exit touch keyboard to zoomed calc mode
         last_zoom_calc = 0;
         change_zoom_config(-666);
         cancel_zoom(666);
         config_screen(128);
         erase_screen();
         no_redraw = 0;

         if((c == 0x0D) && (edit_buffer[0] == ' ') && (edit_buffer[1] == 0)) {
            add_kbd(0x0D);
            start_calc_zoom(12);
            last_null_key = 0;
            return 0;
         }
         else if((c == 0x0D) && (edit_buffer[0] == ' ')) {
            add_kbd(0x0D);
            add_kbd(0x0D);
            start_calc_zoom(13);
            last_null_key = 0;
            return 0;
         }
         else if(1 && (c == ESC_CHAR) && (edit_buffer[0] == ' ') && (edit_buffer[1] == 0)) {
            add_kbd(0x0D);
            zoom_screen = '`';
            show_rpn_help = 0;
         }
         else if(0 && (c == ESC_CHAR) && edit_buffer[0]) {
            add_kbd(0x0D);
            zoom_screen = '`';
            show_rpn_help = 0;
         }
         add_kbd(0x0D);
         add_kbd(0x0D);
         start_calc_zoom(14);
         last_null_key = 0;
         return 0;
      }
      else if(last_null_key) {  // keyboard exit via touching a null key
         if(edit_buffer[0] == ' ') {
            add_kbd(0x0D);
            edit_buffer[0] = 0;
            edit_buffer[1] = 0;
            zoom_screen = '`';
            show_rpn_help = 0;
         }
         start_calc_zoom(15);
         last_null_key = 0;
         return 0;
      }
      else if((c == 0x0D) && edit_err_flag) {
         edit_err_flag = 0;
      }
      last_null_key = 0;
   }
   else if(zoom_screen == '`') {  //  touchscreen keyboard active for zoomed calculator
   }
   else if((zoom_screen || (un_zoom == ZOOM_ADEVS)) && (first_key == 0)) {  // any keystroke restores zoomed screen
      if((zoom_screen == 'P') && scroll_char(c)) ;
      else if(((zoom_screen == 'H') || (zoom_screen == 'O')) && (c == ' ')) {
         pause_monitor ^= 1;
         set_term_header(monitor_port);
         show_term_screen(monitor_port);
         return 0;
      }
      else if((zoom_screen == 'Q') && isdigit(c)) {  // set adjustment value
         cal_adjust = (cal_adjust*10) + (c-'0');
         if(cal_adjust >= 10000) cal_adjust = 0;
         return 0;
      }
      else if((zoom_screen == 'Q') && (c == '+')) {  // go forward a month
         if(cal_adjust) {
            cal_year += cal_adjust / 12;
            cal_month += (cal_adjust % 12);
         }
         else ++cal_month;
         if(cal_month > 12) {
            cal_month = 1;
            ++cal_year;
         }
         cal_adjust = 0;
         return 0;
      }
      else if((zoom_screen == 'Q') && (c == '-')) {  // go back a month
         if(cal_adjust) {
            cal_year -= cal_adjust / 12;
            cal_month -= (cal_adjust % 12);
         }
         else --cal_month;
         if(cal_month < 1) {
            cal_month = 12;
            --cal_year;
         }
         cal_adjust = 0;
         return 0;
      }
      else if((zoom_screen == 'Q') && (c == '>')) {  // go forward a year
         if(cal_adjust) cal_year += cal_adjust;
         else ++cal_year;
         cal_adjust = 0;
         return 0;
      }
      else if((zoom_screen == 'Q') && (c == '<')) {  // go back a year
         if(cal_adjust) cal_year -= cal_adjust;
         else --cal_year;
         cal_adjust = 0;
         return 0;
      }
      else if(c != '\\') {     // unless it is one of these keys
         if(zoom_screen == 'L') {
            fix_flag = zoom_fixes;
         }

         if(zoom_screen) {
            change_zoom_config(-108);
            cancel_zoom(18);     //zkzk
         }
         if(un_zoom == ZOOM_ADEVS) {
            if(rcvr_type == TICC_RCVR) all_adevs = ALL_CHANS;
            else                       all_adevs = SINGLE_ADEVS;
            plot_adev_data = old_plot_adevs;
            if(plot_adev_data == 0) adev_decades_shown = 0;
            un_zoom = UN_ZOOM;
         }

         show_fixes = 0;
         if(rcvr_type != NO_RCVR) show_fixes = zoom_fixes;  // zzzzzz
         if(fix_flag) {
            show_fixes = plot_lla = 1;
         }
         change_fix_config(show_fixes);
         plot_lla = 0;
         if(fix_flag) {
            plot_lla = 1;
         }

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
      if((first_key == 0) && (c != ESC_CHAR)) {
         BEEP(317);
         return 0;
      }
      else if((first_key == ESC_CHAR) && (c == '!')) {
         disable_kbd = 0;
         return sure_exit();
      }

      if((c >= 0x0100) || (c == '[') || (c == ']')) {  // cursor movement keys, etc
         c = 0;
         do_review(c, 2);       // stop review
         return sure_exit(); // prevents spurious scroll mode header
      }
   }

   if(rpn_mode < 0) {  // keep calculator mode active
      show_version_header();
      old_rpn = rpn_mode;
      rpn_mode = 0;
      if((c == 0x0D) || (c == ESC_CHAR) || (c == '`')) {
         if(zoom_screen) remove_zoom();
         show_version_header();
         return 0;
      }

      edit_buffer[0] = c;
      edit_buffer[1] = 0;
      start_calc(13);
   }

   if(c == 0) {
   }
   else if(getting_string) { // we are currently building a parameter value string
      last_was_mark = 0;
      return build_string(c);  // add the keystroke to the line
   }
   else if(getting_plot) {  // we are in the plot control sub-menu
      if(change_plot_param(c, 0)) return 0;
   }
   else if((c >= F1_CHAR) && (c <= F10_CHAR)) {  // F1..F10 - open script file
      if(first_key) return help_exit(c, 99); // xxxxxx
      sprintf(out, "f%d.scr", c-F1_CHAR+1);
      open_script(out);
   }
   else if((c >= 0x0100) || (c == '<') || (c == '>') || (c == '[') || (c == ']') || (c == '@')) {
      // cursor movement keys, etc
      // @ command - zoom to marked queue entry (invoked from mouse click or keyboard)
      last_was_mark = 0;
      if((first_key == 0) && (text_mode == 0)) {
         if(c == '@') {
            goto_mark(0);  // center plot on marked place
            hide_kbd(23);
         }
         else do_review(c, 3);  // scrolling around in the plot data
      }
      else if(first_key) {
         return help_exit(c, 99); // xxxxxx
      }
      else {
         if(c >= 0x0100) cmd_error(0);  //!!!!
         else            cmd_error(c);
      }
      if(c < 0x0100) last_plot_key = c;
      hide_kbd(8);  // no_unzoom
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
            no_x_margin = no_y_margin = 0;
            need_redraw = 6677;
         }
         else {
            c = 0;
            old_lwa = last_was_adev;
            do_review(c, 4);  // stop review
            last_was_adev = old_lwa;
         }
      }
      else if(first_key == ESC_CHAR) {
         if(esc_esc_exit && (disable_kbd == 0)) {  // allow ESC ESC to end the program
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
      hide_kbd(20);
   }
   else {  // normal keystroke command
      if(c < 0x0100) c = tolower(c);
      if((c != '-') && (c != '+')) last_was_mark = 0;
      last_plot_key = 0;

      // break up the keyboard processing big 'if' statement
      // into two parts so the compiler heap does not overflow
      last_was_adev = 0;
      lwa = 13;
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

   // save byte string to send to the receiver

   val = 0x00;
   saw_digit = 0;
   for(i=1; i<=(int)strlen(arg); i++) {  // get and save the string of bytes
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
         if(arg[0] == '=') {  // add byte to list of bytes to send during receiver init
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
   // toggle or force an option switch value

   if     ((sw == '1') || (sw == 'y')) return 1;
   else if((sw == '0') || (sw == 'n')) return 0;
   else return (val & 1) ^ 1;
}


void set_watch_name(char *s)
{
unsigned i, j;
int n;
char c;

   // set the analog watch face name info

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
      time_flags |= TFLAGS_UTC;
      if(luxor) have_timing_mode = 1;
   }
   else if(set_utc_mode) {
      set_gps_mode = 0;
      time_flags &= (~TFLAGS_UTC);
      if(luxor) have_timing_mode = 1;
   }
   else {
      time_flags = TFLAGS_NULL;
      if(luxor) time_flags = TFLAGS_NULL;
      set_utc_mode = 1;
      set_gps_mode = 0;
   }

   // here we attempt to estimate the leapsecond count if it is needed and
   // the user or GPS receiver did not specify one.

   if(!user_set_utc_ofs && set_utc_ofs) {
      get_clock_time();
      if(clk_jd < jdate(2019,7,1)) utc_offset = 18;
      else utc_offset = (int) (43.216 + (double)(clk_year-2061)*0.567);
      have_utc_ofs = (-1);
   }
}



void config_msg_ofs()
{
   //  This routine sets the end-of-message arrival time offset vs the timing
   //  message time values.
   // NEW_RCVR

   if(user_set_tsx) {
   }
   else if(rcvr_type == ACRON_RCVR) {
      time_sync_offset = (600.0);  // !!!!!! we need to measure this and set a proper value
   }
   else if(ACUTIME && !ACU_GG && !ACU_360) {
      time_sync_offset = (67.0);  // (157.2);
   }
   else if(rcvr_type == BRANDY_RCVR) {
      time_sync_offset = (750.1);
   }
   else if(rcvr_type == CS_RCVR) {
      time_sync_offset = (330.0);
   }
   else if(rcvr_type == ESIP_RCVR) {
      time_sync_offset = (199.0);
   }
   else if(rcvr_type == FURUNO_RCVR) {
      time_sync_offset = (195.0);
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
   else if(rcvr_type == PRS_RCVR) {
     time_sync_offset = 0.0;
   }
   else if(rcvr_type == RFTG_RCVR) {
     time_sync_offset = (-663.0);
   }
   else if(rcvr_type == SCPI_RCVR) {
      time_sync_offset = (-964.7); // Z3801:-979.8  Z3812:-949.6
   }
   else if(rcvr_type == SIRF_RCVR) {
      time_sync_offset = (417.0);
   }
   else if((rcvr_type == STAR_RCVR) && (star_type == OSA_TYPE)) {
      time_sync_offset = (337.0);
   }
   else if(rcvr_type == STAR_RCVR) {
      time_sync_offset = (215.0);
   }
   else if(rcvr_type == TAIP_RCVR) {
      time_sync_offset = (-175.0);
   }
   else if(rcvr_type == THERMO_RCVR) {
      time_sync_offset = (0.0);
   }
   else if(rcvr_type == TIDE_RCVR) {
      time_sync_offset = (0.0);
   }
   else if(rcvr_type == TRUE_RCVR) {
      time_sync_offset = (32.5);
   }
   else if(rcvr_type == TSIP_RCVR) {
      if(res_t) {
         if(res_t == RES_T) time_sync_offset = (99.4);
         else if(res_t == RES_T_SMT) time_sync_offset = (393.2);
         else time_sync_offset = (246.3); // !!!!! we need actual timings
      }
      else if(tsip_type == STARLOC_TYPE) {
         time_sync_offset = (266.0);
      }
      else {  // tbolt, ACUTIME GOLD
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
      if(lte_lite) {
         time_sync_offset = 252.1;
      }
      else if(com[RCVR_PORT].baud_rate == 9600) {
         if(saw_timing_msg) time_sync_offset = (208.3);
         else               time_sync_offset = (225.3);
      }
      else {  // 115200 baud
         if(saw_timing_msg) time_sync_offset = (153.8);
         else               time_sync_offset = (170.8);
      }
   }
   else if(rcvr_type == X72_RCVR) {
     time_sync_offset = 0.0;
   }
   else if(rcvr_type == Z12_RCVR) {  // !!!!!
      time_sync_offset = 0.0; // TIME_SYNC_AVG;  // 200.0
   }
   else if(rcvr_type == ZODIAC_RCVR) {
      time_sync_offset = (1238.85);  // Pico = 1232.2, Jupiter-T = 1245.5
   }
   else if(rcvr_type == ZYFER_RCVR) {
      time_sync_offset = (65.00);  // Nanosync 380
   }
   else {
      time_sync_offset = TIME_SYNC_AVG;  // 200.0
   }
}



int status_second(int seconds)
{
   // check if "seconds" is within the time interval where a long status message
   // is comming in.  If so, return next possible valid second, else return 0
   // NEW_RCVR

   if(rcvr_type == ACRON_RCVR) {
      if     (seconds == (SCPI_STATUS_SECOND+1)) return SCPI_STATUS_SECOND+4;
      else if(seconds == (SCPI_STATUS_SECOND+2)) return SCPI_STATUS_SECOND+4;
      else if(seconds == (SCPI_STATUS_SECOND+3)) return SCPI_STATUS_SECOND+4;
   }
   else if(rcvr_type == BRANDY_RCVR) {
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

void config_tide_plots(int enable)
{
    plot[ELEVEN].plot_id = "Lat-t";
    plot[ELEVEN].units = "mm";
    plot[ELEVEN].float_center = 1;

    plot[TWELVE].plot_id = "Lon-t";
    plot[TWELVE].units = "mm";
    plot[TWELVE].float_center = 1;

    plot[THIRTEEN].plot_id = "Alt-t";
    plot[THIRTEEN].units = "mm";
    plot[THIRTEEN].float_center = 1;

    plot[FOURTEEN].plot_id = "Grav";
    plot[FOURTEEN].units = "uGal";
    plot[FOURTEEN].float_center = 1;

    plot[ELEVEN].show_plot = enable;
    plot[TWELVE].show_plot = enable;
    plot[THIRTEEN].show_plot = enable;
    plot[FOURTEEN].show_plot = enable;

    plot[ELEVEN].plot_color = ELEVEN_COLOR;
    plot[TWELVE].plot_color = TWELVE_COLOR;
    plot[THIRTEEN].plot_color = THIRTEEN_COLOR;
    plot[FOURTEEN].plot_color = FOURTEEN_COLOR;

    if((rcvr_type == THERMO_RCVR) || ((rcvr_type == TIDE_RCVR) && enviro_mode())) {
       plot[ADC3].plot_id = "ADC3";
       plot[ADC3].units = "V";
       plot[ADC3].ref_scale = (DATA_SIZE) 1.0;
       plot[ADC3].float_center = 1;

       plot[ADC4].plot_id = "ADC4";
       plot[ADC4].units = "V";
       plot[ADC4].ref_scale = (DATA_SIZE) 1.0;
       plot[ADC4].float_center = 1;
    }
}

void config_prn_plots(int enable)
{
static char degs[2];

    degs[0] = DEGREES;
    degs[1] = 0;

    plot[ELEVEN].plot_id = "AZ";
    plot[ELEVEN].units = &degs[0];
    plot[ELEVEN].float_center = 1;

    plot[TWELVE].plot_id = "EL";
    plot[TWELVE].units = &degs[0];
    plot[TWELVE].float_center = 1;

    plot[THIRTEEN].plot_id = level_type; //level_type;
    plot[THIRTEEN].units = level_type;   //level_type;
    plot[THIRTEEN].float_center = 1;

    if(have_phase) {
       plot[FOURTEEN].plot_id = "Phase";
       plot[FOURTEEN].units = "cyc";
    }
    else if(have_range) {
       plot[FOURTEEN].plot_id = "Range";
       plot[FOURTEEN].units = "m";
    }
    else {
       plot[FOURTEEN].plot_id = "Grav";
       plot[FOURTEEN].units = "uGal";
    }
    plot[FOURTEEN].float_center = 1;

    plot[ELEVEN].show_plot = enable;
    plot[TWELVE].show_plot = enable;
    plot[THIRTEEN].show_plot = enable;
    plot[FOURTEEN].show_plot = enable;

    plot[ELEVEN].plot_color = ELEVEN_COLOR;
    plot[TWELVE].plot_color = TWELVE_COLOR;
    plot[THIRTEEN].plot_color = THIRTEEN_COLOR;
    plot[FOURTEEN].plot_color = FOURTEEN_COLOR;

    if((rcvr_type == THERMO_RCVR) || ((rcvr_type == TIDE_RCVR) && enviro_mode())) {
       plot[ADC3].plot_id = "ADC3";
       plot[ADC3].units = "V";
       plot[ADC3].ref_scale = (DATA_SIZE) 1.0;
       plot[ADC3].float_center = 1;

       plot[ADC4].plot_id = "ADC4";
       plot[ADC4].units = "V";
       plot[ADC4].ref_scale = (DATA_SIZE) 1.0;
       plot[ADC4].float_center = 1;
    }
}

void config_enviro_plots(int enable)
{
if(rcvr_type == TIDE_RCVR) return;

    plot[TEMP1].plot_id = "TEMP1";     // [TC1] -> [FIVE]
    plot[TEMP1].units = degc;
    plot[TEMP1].float_center = 1;

    plot[TEMP2].plot_id = "TEMP2";     // [TC2] -> [FIVE]
    plot[TEMP2].units = degc;
    plot[TEMP2].float_center = 1;

    plot[HUMIDITY].plot_id = "RH";
    plot[HUMIDITY].units = "%";
    plot[HUMIDITY].ref_scale = (DATA_SIZE) 1.0;
    plot[HUMIDITY].float_center = 1;

    plot[PRESSURE].plot_id = "Pres";
    plot[PRESSURE].units = "mbar";
    plot[PRESSURE].ref_scale = (DATA_SIZE) 1.0;
    plot[PRESSURE].float_center = 1;

   if(enable && (rcvr_type == THERMO_RCVR)) {
      plot[HUMIDITY].show_plot = 1;
      plot[PRESSURE].show_plot = 1;
      plot[TEMP1].show_plot = 1;
      plot[TEMP2].show_plot = 1;
      plot[ADC3].show_plot = 1;
      plot[ADC4].show_plot = 1;

      plot[TEMP].show_plot = 0;
   }

   if((rcvr_type == THERMO_RCVR) || ((rcvr_type == TIDE_RCVR) && enviro_mode())) {
      plot[ADC3].plot_id = "ADC3";
      plot[ADC3].units = "V";
      plot[ADC3].ref_scale = (DATA_SIZE) 1.0;
      plot[ADC3].float_center = 1;

      plot[ADC4].plot_id = "ADC4";
      plot[ADC4].units = "V";
      plot[ADC4].ref_scale = (DATA_SIZE) 1.0;
      plot[ADC4].float_center = 1;

      plot[HUMIDITY].plot_color = MAGENTA;
      plot[PRESSURE].plot_color = WHITE;
      plot[TEMP1].plot_color = YELLOW;
      plot[TEMP2].plot_color = RED;
   }
}

void disable_adevs()
{
   // disable ADEVs for receivers that use PPS and OSC data for non-ADEVable
   // purposes

   if(user_set_adev_size) no_adev_flag = 0;
   else                   no_adev_flag = 1;
}


void config_rcvr_plots()
{
int i;

   // Configure the plots for the receiver type in use
   // note: we should not change settings the user gave on the command line
   // NEW_RCVR

   no_adev_flag = 0;

   plot[FOUR].plot_id = "Speed";
   plot[FOUR].units = "m/s";
   plot[FOUR].ref_scale = (DATA_SIZE) 1.0;
   plot[FOUR].show_plot = 0;
   plot[FOUR].float_center = 1;

   plot[FIVE].plot_id = "Course";
   plot[FIVE].units = degs;
   plot[FIVE].ref_scale = (DATA_SIZE) 1.0;
   plot[FIVE].show_plot = 0;
   plot[FIVE].float_center = 1;

   if(rcvr_type == CS_RCVR) ;
   else if(rcvr_type == PRS_RCVR) ;
   else if(luxor) ;
   else {
      plot[MSGJIT].plot_id = "MsgJit";   // timing message jitter
      plot[MSGJIT].units = "msec";
      plot[MSGJIT].ref_scale = (DATA_SIZE) 1.0;
      plot[MSGJIT].float_center = 1;
      plot[MSGJIT].show_stat = SDEV;

      plot[MSGOFS].plot_id = "MsgOfs";   // timing message offset from clock
      plot[MSGOFS].units = "msec";
      plot[MSGOFS].ref_scale = (DATA_SIZE) 1.0;
      plot[MSGOFS].float_center = 1;
      plot[MSGOFS].show_stat = AVG;      // SHOW_SPAN;

      config_tide_plots(0);
   }

   if(res_t || ACU_GG || ACU_360 || ACUTIME || PALISADE) {
      plot[DAC].plot_id = "Sawt";
      plot[DAC].units = "ns";
      plot[DAC].ref_scale = (DATA_SIZE) 1.0;
      if(user_set_dac_plot == 0) plot[DAC].show_plot = 0;
      if(user_set_dac_float == 0) plot[DAC].float_center = 1;

      if(ACU_GG || ACU_360) {
         plot[PPS].plot_id = "Bias";
         plot[PPS].units = "ns";
         plot[PPS].ref_scale = (DATA_SIZE) (1.0);
         if(user_set_pps_plot == 0) plot[PPS].show_plot = 1;
         if(user_set_pps_float == 0) plot[PPS].float_center = 1;

         plot[OSC].plot_id = "Rate";
         plot[OSC].units = "ppb";
         plot[OSC].ref_scale = (DATA_SIZE) 1.0;
         if(user_set_osc_plot == 0) plot[OSC].show_plot = 1;
         if(user_set_osc_float == 0) plot[OSC].float_center = 1;
         disable_adevs();
      }
      else {
          plot[PPS].plot_id = "Bias";
          plot[PPS].units = "us";
          plot[PPS].ref_scale = (DATA_SIZE) (1.0/1000.0);
          if(user_set_pps_plot == 0) plot[PPS].show_plot = 1;
          if(user_set_pps_float == 0) plot[PPS].float_center = 1;

          plot[OSC].plot_id = "Rate";
          plot[OSC].units = "ppb";
          plot[OSC].ref_scale = (DATA_SIZE) 1.0;
          if(user_set_osc_plot == 0) plot[OSC].show_plot = 1;
          if(user_set_osc_float == 0) plot[OSC].float_center = 1;
          disable_adevs();
      }

      if(user_set_adev_plot == 0) {
         plot_adev_data = 0;
         adev_decades_shown = 0;
      }
//    have_sawtooth = 1;
   }
   else if(rcvr_type == ACRON_RCVR) {
      plot[PPS].show_plot = 0;
      plot[DAC].show_plot = 0;
   }
   else if(rcvr_type == BRANDY_RCVR) {
      plot[PPS].plot_id = "Phase";
      disable_adevs();

      plot[DAC].units = "%";
      plot[DAC].scale_factor = 1.0;
      plot[DAC].ref_scale = (DATA_SIZE) 1.0;

      plot[SEVEN].plot_id = "Trend";
      if(show_euro_ppt) plot[SEVEN].units = "e-12";
      else              plot[SEVEN].units = "ppt";
      plot[SEVEN].scale_factor = 1.0;
      plot[SEVEN].ref_scale = (DATA_SIZE) 1.0;
      plot[SEVEN].float_center = 1;

      plot[EIGHT].plot_id = "AvgPha";
      plot[EIGHT].units = "ns";
      plot[EIGHT].scale_factor = 1.0;
      plot[EIGHT].ref_scale = (DATA_SIZE) 1.0;
      plot[EIGHT].float_center = 1;

      if(user_set_pps_float == 0) plot[PPS].float_center = 1;
      if(user_set_osc_float == 0) plot[OSC].float_center = 1;
      set_osc_units();
   }
   else if(rcvr_type == CS_RCVR) {
      plot[DAC].units = "%";
      plot[DAC].scale_factor = 1.0;
      plot[DAC].ref_scale = (DATA_SIZE) 1.0;

      plot[PPS].plot_id = "Pump";
      plot[PPS].units = "uA";
      plot[PPS].float_center = 1;

      plot[OSC].plot_id = "Emul";
      plot[OSC].units = "V";
      plot[OSC].float_center = 1;
      plot[OSC].ref_scale = (DATA_SIZE) 1.0;
      if(user_set_osc_plot == 0) plot[OSC].show_plot = 1;
      ppt_string = "V";
      disable_adevs();

      plot[ONE].plot_id = "RF1";
      plot[ONE].units = "%";

      plot[TWO].plot_id = "RF2";
      plot[TWO].units = "%";

      plot[THREE].plot_id = "GAIN";
      plot[THREE].units = "%";

      plot[FOUR].plot_id = "Coven";
      plot[FOUR].units = "V";

      plot[FIVE].plot_id = "Qoven";
      plot[FIVE].units = "V";

      plot[SIX].plot_id = "DRO";
      plot[SIX].units = "V";

      plot[SEVEN].plot_id = "SAW";
      plot[SEVEN].units = "V";

      plot[EIGHT].plot_id = "87MHz";
      plot[EIGHT].units = "V";

      plot[NINE].plot_id = "uP";
      plot[NINE].units = "V";

      plot[TEN].plot_id = "Beam";
      plot[TEN].units = "nA";

      plot[ELEVEN].plot_id = "CField";
      plot[ELEVEN].units = "mA";
      plot[ELEVEN].float_center = 1;

      plot[TWELVE].plot_id = "HWI";
      plot[TWELVE].units = "V";

      plot[THIRTEEN].plot_id = "MSpec";
      plot[THIRTEEN].units = "V";

      plot[FOURTEEN].plot_id = "PWRsum";
      plot[FOURTEEN].units = "V";

      for(i=0; i<=FOURTEEN; i++) {
         plot[i].show_stat = SHOW_SPAN;
      }

      plot_azel = 0;
      shared_plot = 0;

      round_temp = 2;
      need_screen_init = 1;
      if(user_set_temp_plot == 0) plot[TEMP].show_plot = 1;
   }
   else if(rcvr_type == ESIP_RCVR) {
//    plot[SIX].show_plot = 1;   // dop
//    plot[SIX].float_center = 1;

      plot[PPS].plot_id = "Accu";
      plot[PPS].units = "ns";
      plot[PPS].ref_scale = (DATA_SIZE) 1.0;
      if(user_set_pps_plot == 0) plot[PPS].show_plot = 1;
      if(user_set_pps_float == 0) plot[PPS].float_center = 1;

      plot[DAC].plot_id = "Sawt";
      plot[DAC].units = "ns";
      plot[DAC].ref_scale = (DATA_SIZE) 1.0;
      if(user_set_dac_plot == 0) plot[DAC].show_plot = 1;  // sawtooth

      if(user_set_osc_plot == 0) plot[OSC].show_plot = 0;
      if(user_set_temp_plot == 0) plot[TEMP].show_plot = 0;
      disable_adevs();
   }
   else if(rcvr_type == FURUNO_RCVR) {
      plot[SIX].show_plot = 1;   // dop
      plot[SIX].float_center = 1;

      if(user_set_dac_plot == 0) plot[DAC].show_plot = 0;   // disable GPSDO related plots
      if(user_set_osc_plot == 0) plot[OSC].show_plot = 0;
      if(user_set_pps_plot == 0) plot[PPS].show_plot = 0;
      if(user_set_pps_float == 0) plot[PPS].float_center = 1;
   }
   else if(rcvr_type == GPSD_RCVR) {
      plot[PPS].plot_id = "Clk";
      plot[PPS].units = "ms";
      plot[PPS].ref_scale = (DATA_SIZE) (1.0/1000000.0);
//    if(user_set_pps_plot == 0) plot[PPS].show_plot = 0;
      if(user_set_pps_float == 0) plot[PPS].float_center = 1;

      if(user_set_dac_plot == 0) plot[DAC].show_plot = 0;   // disable GPSDO related plots
      if(user_set_osc_plot == 0) plot[OSC].show_plot = 0;
      disable_adevs();
   }
   else if(rcvr_type == LPFRS_RCVR) {
      plot[PPS].plot_id = "PPS";
      plot[OSC].units = "ns";
      if(user_set_osc_plot == 0) plot[PPS].show_plot = 0;
      if(user_set_pps_float == 0) plot[PPS].float_center = 1;

      plot[OSC].plot_id = "FC";
      plot[OSC].units = "ppb";
      if(user_set_osc_plot == 0) plot[OSC].show_plot = 0;
      if(user_set_osc_float == 0) plot[OSC].float_center = 1;
      disable_adevs();

      if(user_set_dac_plot == 0) plot[DAC].show_plot = 0;

      plot[ONE].plot_id = "EFC";
      plot[ONE].units = "V";
      plot[ONE].show_plot = 1;

      plot[TWO].plot_id = "Signal";
      plot[TWO].units = "V";
      plot[TWO].show_plot = 1;

      plot[THREE].plot_id = "Photo";
      plot[THREE].units = "V";
      plot[THREE].show_plot = 1;

      plot[FOUR].plot_id = "Varactor";
      plot[FOUR].units = "V";

      plot[FIVE].plot_id = "LampI";
      plot[FIVE].units = "mA";

      plot[SIX].plot_id = "CellI";
      plot[SIX].units = "mA";

      plot[SEVEN].plot_id = "AGC";
      plot[SEVEN].units = "V";

      plot[EIGHT].plot_id = "FF";
      plot[EIGHT].units = "x";
   }
   else if(rcvr_type == MOTO_RCVR) {
      plot[DAC].plot_id = "Sawt";
      plot[DAC].units = "ns";
      plot[DAC].ref_scale = (DATA_SIZE) 1.0;
      if(user_set_dac_plot == 0) plot[DAC].show_plot = 0;
      if(user_set_dac_float == 0) plot[DAC].float_center = 1;

      plot[PPS].plot_id = "Accu";
      plot[PPS].units = "us";
      plot[PPS].ref_scale = (DATA_SIZE) (1.0/1000.0);
      if(user_set_pps_plot == 0) plot[PPS].show_plot = 1;
      if(user_set_pps_float == 0) plot[PPS].float_center = 1;

      plot[OSC].plot_id = "Cofs";
      plot[OSC].units = "ns";
      plot[OSC].ref_scale = (DATA_SIZE) 1.0;
      if(user_set_osc_plot == 0) plot[OSC].show_plot = 1;
      if(user_set_osc_float == 0) plot[OSC].float_center = 1;
      disable_adevs();

      plot[SIX].show_plot = 0;  // dop
      plot[SIX].float_center = 1;

      ppt_string = " ns";
      ppb_string = " ns";
   }
   else if(rcvr_type == NMEA_RCVR) {
      plot[SIX].show_plot = 1;   // dop
      plot[SIX].float_center = 1;

      if(user_set_dac_plot == 0) plot[DAC].show_plot = 0;   // disable GPSDO related plots
      if(user_set_osc_plot == 0) plot[OSC].show_plot = 0;
      if(user_set_pps_plot == 0) plot[PPS].show_plot = 0;
      if(user_set_pps_float == 0) plot[PPS].float_center = 1;
   }
   else if(rcvr_type == NVS_RCVR) {
      plot[DAC].plot_id = "Sawt";
      plot[DAC].units = "ns";
      plot[DAC].ref_scale = (DATA_SIZE) 1.0;
      if(user_set_dac_plot == 0) plot[DAC].show_plot = 0;
      if(user_set_dac_float == 0) plot[DAC].float_center = 1;

      plot[PPS].plot_id = "Rgen";
      plot[PPS].units = "ns/s";
      plot[PPS].ref_scale = (DATA_SIZE) 1.0;
      if(user_set_pps_plot == 0) plot[PPS].show_plot = 1;
      if(user_set_pps_float == 0) plot[PPS].float_center = 1;
      disable_adevs();
   }
   else if(rcvr_type == PRS_RCVR) {
      plot[PPS].plot_id = "TT";

      plot[OSC].plot_id = "FC";
      if(user_set_osc_plot == 0) plot[OSC].show_plot = 1;
      if(user_set_osc_float == 0) plot[OSC].float_center = 1;

      if(user_set_pps_float == 0) plot[PPS].float_center = 1;
      if(user_set_dac_plot == 0) plot[DAC].show_plot = 0;
      disable_adevs();

      plot[ONE].plot_id = "OcxoH";
      plot[ONE].units = "V";

      plot[TWO].plot_id = "CellH";
      plot[TWO].units = "V";

      plot[THREE].plot_id = "LampH";
      plot[THREE].units = "V";

      plot[FOUR].plot_id = "Pot";
      plot[FOUR].units = "V";

      plot[FIVE].plot_id = "Therm";
      plot[FIVE].units = "r";

      plot[SIX].plot_id = "Pwr";
      plot[SIX].units = "V";

      plot[SEVEN].plot_id = "I/V";
      plot[SEVEN].units = "V";

      plot[EIGHT].plot_id = "Photo";
      plot[EIGHT].units = "x";

      plot[NINE].plot_id = "HARM2";  // prs_ds1
      plot[NINE].units = "x";

      plot[TEN].plot_id = "SIG";     // prs_ds2
      plot[TEN].units = "x";
   }
   else if(rcvr_type == RFTG_RCVR) {
      plot[PPS].plot_id = "Phase";
      disable_adevs();
      if(user_set_pps_float == 0) plot[PPS].float_center = 1;

      plot[DAC].units = "V";
      plot[DAC].scale_factor = 1.0;
      plot[DAC].ref_scale = (DATA_SIZE) 1.0;

      plot[FIVE].plot_id = "OscV";
      plot[FIVE].units = "V";

      plot[SIX].plot_id = "AntI";
      plot[SIX].units = "mA";
      plot[SIX].scale_factor = 1.0;
      plot[SIX].ref_scale = (DATA_SIZE) 1.0;

      plot[SEVEN].plot_id = "AntV1";
      plot[SEVEN].units = "V";
      plot[SEVEN].scale_factor = 1.0;
      plot[SEVEN].ref_scale = (DATA_SIZE) 1.0;

      plot[EIGHT].plot_id = "AntV2";
      plot[EIGHT].units = "V";
      plot[EIGHT].scale_factor = 1.0;
      plot[EIGHT].ref_scale = (DATA_SIZE) 1.0;

      if(user_set_temp_plot == 0) plot[TEMP].show_plot = 1;
   }
   else if(rcvr_type == RT17_RCVR) {
      plot[SIX].show_plot = 0;   // dop
      plot[SIX].float_center = 1;

      plot[PPS].plot_id = "CLK";
      plot[PPS].units = "ns";

      plot[OSC].plot_id = "FREQ";
      plot[OSC].units = "Hz";

      ppt_string = " Hz";

      if(user_set_dac_plot == 0) plot[DAC].show_plot = 0;   // disable GPSDO related plots
      if(user_set_osc_plot == 0) plot[OSC].show_plot = 1;
      if(user_set_pps_plot == 0) plot[PPS].show_plot = 1;
      if(user_set_pps_float == 0) plot[PPS].float_center = 1;
   }
   else if(rcvr_type == SA35_RCVR) {
      // !!!!! placeholder
      plot[DAC].plot_id = "EFC";
      plot[OSC].units = "V";
      if(user_set_dac_plot == 0) plot[DAC].show_plot = 1;
      if(user_set_dac_float == 0) plot[DAC].float_center = 1;

//    plot[OSC].plot_id = "FC";
//    plot[OSC].units = "ppb";
//    if(user_set_osc_plot == 0) plot[OSC].show_plot = 0;
//    if(user_set_osc_float == 0) plot[OSC].float_center = 1;
      disable_adevs();

      plot[ONE].plot_id = "TEC";
      plot[ONE].units = degc;
      plot[ONE].show_plot = 1;

      plot[TWO].plot_id = "SigDC";
      plot[TWO].units = "V";
      plot[TWO].show_plot = 1;

      plot[THREE].plot_id = "RF";
      plot[THREE].units = "V";
      plot[THREE].show_plot = 1;

      plot[FOUR].plot_id = "CellI";
      plot[FOUR].units = "mA";
   }
   else if(rcvr_type == SCPI_RCVR) {
      plot[DAC].units = "%";
      plot[DAC].scale_factor = 1.0;
      plot[DAC].ref_scale = (DATA_SIZE) 1.0;

      plot[PPS].scale_factor = 1.0; //1000
      plot[PPS].ref_scale = (DATA_SIZE) 1.0;
      plot[PPS].units = "ns";
      if(user_set_pps_float == 0) plot[PPS].float_center = 1;

      plot[OSC].plot_id = "UNC";
      plot[OSC].ref_scale = (DATA_SIZE) 1.0;
      plot[OSC].scale_factor = (DATA_SIZE) 1.0;
      plot[OSC].units = "us";
      if(user_set_osc_float == 0) plot[OSC].float_center = 1;
      if(user_set_osc_plot == 0) plot[OSC].show_plot = 1;
      ppb_string = " us";
      ppt_string = " us";
      disable_adevs();

      plot_adev_data = 0;
      adev_decades_shown = 0;

      plot[FOUR].plot_id = "TFOM";
      plot[FOUR].units = " ";
      plot[FOUR].ref_scale = (DATA_SIZE) 1.0;
      plot[FOUR].show_plot = 0;
      plot[FOUR].float_center = 0;

      plot[FIVE].plot_id = "FFOM";
      plot[FIVE].units = " ";
      plot[FIVE].ref_scale = (DATA_SIZE) 1.0;
      plot[FIVE].show_plot = 0;
      plot[FIVE].float_center = 0;

      plot[SIX].show_plot = 0;
   }
   else if(rcvr_type == SIRF_RCVR) {
      plot[PPS].plot_id = "Drft";
      plot[PPS].units = "us";
      plot[PPS].ref_scale = (DATA_SIZE) (1.0/1000.0);
      if(user_set_pps_plot == 0) plot[PPS].show_plot = 1;
      if(user_set_pps_float == 0) plot[PPS].float_center = 1;

      plot[OSC].plot_id = "Cofs";
      plot[OSC].units = "ns";
      plot[OSC].ref_scale = (DATA_SIZE) 1.0;
      if(user_set_osc_plot == 0) plot[OSC].show_plot = 0;
      if(user_set_osc_float == 0) plot[OSC].float_center = 1;
      disable_adevs();

      if(user_set_dac_plot == 0) plot[DAC].show_plot = 0;

      plot[SIX].show_plot = 0;  // dop
      plot[SIX].float_center = 1;

      ppt_string = " ns";
      ppb_string = " ns";
   }
   else if(rcvr_type == SRO_RCVR) {
      plot[PPS].plot_id = "PPS";
      plot[OSC].units = "ns";
      if(user_set_pps_float == 0) plot[PPS].float_center = 1;

      plot[OSC].plot_id = "FC";
      plot[OSC].units = "ppt";
      if(user_set_osc_plot == 0) plot[OSC].show_plot = 1;
      if(user_set_osc_float == 0) plot[OSC].float_center = 1;
      disable_adevs();

      if(user_set_dac_plot == 0) plot[DAC].show_plot = 0;

      plot[ONE].plot_id = "EFC";
      plot[ONE].units = "V";

      plot[TWO].plot_id = "Signal";
      plot[TWO].units = "V";

      plot[THREE].plot_id = "Photo";
      plot[THREE].units = "V";

      plot[FOUR].plot_id = "Varactor";
      plot[FOUR].units = "V";

      plot[FIVE].plot_id = "LampI";
      plot[FIVE].units = "%";

      plot[SIX].plot_id = "CellI";
      plot[SIX].units = "%";

      plot[SEVEN].plot_id = "VT";
      plot[SEVEN].units = "s";

      plot[EIGHT].plot_id = "GG";
      plot[EIGHT].units = "x";

      plot[NINE].plot_id = "AA";   // reserved
      plot[NINE].units = "x";
   }
   else if(rcvr_type == SS_RCVR) {
      plot[PPS].plot_id = "Bias";
      plot[PPS].units = "ns";                       // "s"
      plot[PPS].ref_scale = (DATA_SIZE) (1.0/1.0);  // (1.0/1000.0)
      if(user_set_pps_plot == 0) plot[PPS].show_plot = 1;
      if(user_set_pps_float == 0) plot[PPS].float_center = 1;

      plot[OSC].plot_id = "Drft";
      plot[OSC].units = "ppb";
      plot[OSC].ref_scale = (DATA_SIZE) 1.0;
      if(user_set_osc_plot == 0) plot[OSC].show_plot = 1;
      if(user_set_osc_float == 0) plot[OSC].float_center = 1;
      disable_adevs();

      if(user_set_dac_plot == 0) plot[DAC].show_plot = 0;

      plot[SIX].show_plot = 0;  // dop
      plot[SIX].float_center = 1;

      ppt_string = " ns";
      ppb_string = " ns";
   }
   else if(rcvr_type == STAR_RCVR) {
      if(user_set_temp_plot == 0) plot[TEMP].show_plot = 1;
      if(!user_set_dac_plot) plot[DAC].show_plot = 0;
      if(!user_set_osc_plot) plot[OSC].show_plot = 0;
      if(!user_set_pps_plot) plot[PPS].show_plot = 0;
      if(user_set_pps_float == 0) plot[PPS].float_center = 1;
   }
   else if(rcvr_type == TAIP_RCVR) {
      plot[SIX].show_plot = 0;   // dop
      plot[SIX].float_center = 1;

      if(user_set_dac_plot == 0) plot[DAC].show_plot = 0;   // disable GPSDO related plots
      if(user_set_osc_plot == 0) plot[OSC].show_plot = 0;
      if(user_set_pps_plot == 0) plot[PPS].show_plot = 0;
      if(user_set_pps_float == 0) plot[PPS].float_center = 1;
   }
   else if(rcvr_type == THERMO_RCVR) {
      if(user_set_temp_plot == 0) plot[TEMP].show_plot = 1;
      if(user_set_dac_plot == 0) plot[DAC].show_plot = 0;   // disable GPSDO related plots
      if(user_set_osc_plot == 0) plot[OSC].show_plot = 0;
      if(user_set_pps_plot == 0) plot[PPS].show_plot = 0;
      if(user_set_pps_float == 0) plot[PPS].float_center = 1;

      config_enviro_plots(2);

      shared_plot = 0;  // xyzzy
      plot_signals = 0;  user_set_signals = 1;
      plot_azel = 0;
      need_screen_init = 1;
   }
   else if(rcvr_type == TICC_RCVR) {
      mixed_adevs = MIXED_GRAPHS;
      plot_adev_data = 1;
      user_set_adev_plot = 1;
      all_adevs = aa_val = ALL_PPS;

      shared_plot = 0;  // xyzzy
      plot_signals = 0;  user_set_signals = 1;
      plot_watch = 0;  user_set_watch_plot = 1;
      plot_azel = 0;
      need_screen_init = 1;

      plot[PPS].plot_id = "TIEa";
      plot[PPS].show_plot = 1;
      plot[PPS].units = "ns";
      plot[PPS].ref_scale = (DATA_SIZE) 1.0;
      plot[PPS].scale_factor = 1.0;
      plot[PPS].plot_color = PPS_ADEV_COLOR;
      if(user_set_pps_float == 0) plot[PPS].float_center = 1;

      plot[OSC].plot_id = "TIEb";
      plot[OSC].show_plot = 0;
      plot[OSC].units = "ns";
      plot[OSC].ref_scale = (DATA_SIZE) 1.0;
      plot[OSC].scale_factor = 1.0;
      plot[OSC].plot_color = OSC_ADEV_COLOR;
      if(user_set_osc_float == 0) plot[OSC].float_center = 1;

      plot[SEVEN].plot_id = "TIEc";
      plot[SEVEN].units = "ns";
      plot[SEVEN].ref_scale = (DATA_SIZE) 1.0;
      plot[SEVEN].scale_factor = 1.0;
      plot[SEVEN].plot_color = GREEN;

      plot[EIGHT].plot_id = "TIEd";
      plot[EIGHT].units = "ns";
      plot[EIGHT].ref_scale = (DATA_SIZE) 1.0;
      plot[EIGHT].scale_factor = 1.0;
      plot[EIGHT].plot_color = RED;


      plot[ONE].plot_id = "PHchA";
      plot[ONE].units = "ns";
      plot[ONE].show_plot = 0;

      plot[TWO].plot_id = "PHchB";
      plot[TWO].units = "ns";
      plot[TWO].show_plot = 0;

      plot[THREE].plot_id = "PHchC";
      plot[THREE].units = "ns";
      plot[THREE].show_plot = 0;
      plot[THREE].plot_color = DIM_GREEN;

      plot[FOUR].plot_id = "PHchD";
      plot[FOUR].units = "ns";
      plot[FOUR].show_plot = 0;
      plot[FOUR].plot_color = DIM_RED;

      plot[FIVE].plot_id = "T2_A";
      plot[FIVE].units = "";
      plot[FIVE].plot_color = CYAN;

      plot[SIX].plot_id = "T2_B";
      plot[SIX].units = "";
      plot[SIX].plot_color = YELLOW;


      if(ticc_type == LARS_TICC) {
         if(user_set_temp_plot == 0) plot[TEMP].show_plot = 1;
         have_temperature = 1;
         if(user_set_dac_plot == 0) plot[DAC].show_plot = 1;
         plot[DAC].plot_id = "DAC";
         plot[DAC].units = "";
         have_dac = 778;
      }
      else {
         plot[DAC].plot_id = "FDG_A";     // TICC debug mode values
         plot[DAC].units = "ps";
         plot[DAC].plot_color = GREEN;
         plot[DAC].show_plot = 0;
         plot[DAC].float_center = 1;
         plot[DAC].ref_scale = (DATA_SIZE) 1.0;
      }

      for(i=0; i<NUM_PLOTS+DERIVED_PLOTS; i++) {
         plot[i].show_stat = SHOW_SPAN;   // !!!! was AVG
      }

      ppt_string = " ns";
      ppb_string = " ns";
   }
   else if(rcvr_type == TIDE_RCVR) {
      if(user_set_pps_plot == 0) plot[PPS].show_plot = 0;
      if(user_set_osc_plot == 0) plot[OSC].show_plot = 0;
      if(user_set_dac_plot == 0) plot[DAC].show_plot = 0;
      if(user_set_temp_plot == 0) plot[TEMP].show_plot = 0;
      if(user_set_pps_float == 0) plot[PPS].float_center = 1;

      config_enviro_plots(0);  // uses better colors for environmental plots
   }
   else if(rcvr_type == TRUE_RCVR) {
//    plot[DAC].plot_id = "Sawt";
//    plot[DAC].units = "ns";
      plot[DAC].units = "V";
      plot[DAC].scale_factor = 1.0;
      plot[DAC].ref_scale = (DATA_SIZE) 1.0E6;
      if(user_set_dac_plot == 0) plot[DAC].show_plot = 1;

      if(user_set_dac_float == 0) plot[DAC].float_center = 1;
      if(user_set_pps_plot == 0) plot[PPS].show_plot = 0;
      if(user_set_osc_plot == 0) plot[OSC].show_plot = 0;
      if(user_set_pps_float == 0) plot[PPS].float_center = 1;

      plot[FOUR].plot_id = "TFOM";
      plot[FOUR].units = " ";
      plot[FOUR].ref_scale = (DATA_SIZE) 1.0;
      plot[FOUR].show_plot = 0;
      plot[FOUR].float_center = 0;

      plot[FIVE].plot_id = "MODE";
      plot[FIVE].units = " ";
      plot[FIVE].ref_scale = (DATA_SIZE) 1.0;
      plot[FIVE].show_plot = 0;
      plot[FIVE].float_center = 1;

      plot[EIGHT].plot_id = "EVAL";
      plot[EIGHT].units = " ";
      plot[EIGHT].ref_scale = (DATA_SIZE) 1.0;
//    plot[EIGHT].show_plot = 1;
      plot[EIGHT].float_center = 1;
   }
   else if(rcvr_type == TM4_RCVR) {
      if(user_set_temp_plot == 0) plot[TEMP].show_plot = 0;
      if(!user_set_dac_plot) plot[DAC].show_plot = 0;
      if(!user_set_osc_plot) plot[OSC].show_plot = 0;
      if(!user_set_pps_plot) plot[PPS].show_plot = 0;
      if(user_set_pps_float == 0) plot[PPS].float_center = 1;
   }
   else if(rcvr_type == TSERVE_RCVR) {
      if(user_set_temp_plot == 0) plot[TEMP].show_plot = 0;
      if(!user_set_dac_plot) plot[DAC].show_plot = 0;
      if(!user_set_osc_plot) plot[OSC].show_plot = 0;
      if(!user_set_pps_plot) plot[PPS].show_plot = 0;
      if(user_set_pps_float == 0) plot[PPS].float_center = 1;
   }
   else if(rcvr_type == UBX_RCVR) {
      plot[DAC].plot_id = "Sawt";
      plot[DAC].units = "ns";
      plot[DAC].ref_scale = (DATA_SIZE) 1.0;
      if(user_set_dac_plot == 0) plot[DAC].show_plot = 0;
      if(user_set_dac_float == 0) plot[DAC].float_center = 1;

      plot[PPS].plot_id = "Accu";
      plot[PPS].units = "ns";
      plot[PPS].ref_scale = (DATA_SIZE) 1.0;
      if(user_set_pps_plot == 0) plot[PPS].show_plot = 1;
      if(user_set_pps_float == 0) plot[PPS].float_center = 1;

      plot[OSC].plot_id = "Frac";
      plot[OSC].units = "ns";
      plot[OSC].ref_scale = (DATA_SIZE) 1.0;
      if(user_set_osc_plot == 0) plot[OSC].show_plot = 0;
      if(user_set_osc_float == 0) plot[OSC].float_center = 1;
      disable_adevs();

      plot[SIX].show_plot = 0;  // dop
      plot[SIX].float_center = 1;

      ppt_string = " ns";
      ppb_string = " ns";
   }
   else if(rcvr_type == UCCM_RCVR) {
      plot[DAC].units = "%";
      plot[DAC].scale_factor = 1.0;
      plot[DAC].ref_scale = (DATA_SIZE) 1.0;

      plot[PPS].scale_factor = 1.0; //1000
      plot[PPS].ref_scale = (DATA_SIZE) 1.0;
      plot[PPS].units = "ns";
      if(user_set_pps_float == 0) plot[PPS].float_center = 1;

      plot[SIX].plot_id = "AntI";
      plot[SIX].units = "mA";
      plot[SIX].scale_factor = 1.0;
      plot[SIX].ref_scale = (DATA_SIZE) 1.0;

      plot[SEVEN].plot_id = "AntV";
      plot[SEVEN].units = "V";
      plot[SEVEN].scale_factor = 1.0;
      plot[SEVEN].ref_scale = (DATA_SIZE) 1.0;

      plot[EIGHT].plot_id = "PCOR";
      plot[EIGHT].units = "ppt";
      plot[EIGHT].scale_factor = 1.0;
      plot[EIGHT].ref_scale = (DATA_SIZE) 1.0;

plot[OSC].plot_id = "LOOP";

      if(user_set_osc_plot == 0) plot[OSC].show_plot = 1;
      if(user_set_osc_float == 0) plot[OSC].float_center = 1;

//    plot[OSC].plot_id = "UNC";
//    if(user_set_osc_plot == 0) plot[OSC].show_plot = 0;
//    plot[OSC].ref_scale = 1.0;    //1000.0;
//    plot[OSC].scale_factor = 1.0; //1000.0;
//    plot[OSC].units = "us";
//    ppb_string = " us";
//    ppt_string = " us";

      plot_adev_data = 0;
      adev_decades_shown = 0;

      plot[FOUR].plot_id = "TFOM";
      plot[FOUR].units = " ";
      plot[FOUR].ref_scale = (DATA_SIZE) 1.0;
      plot[FOUR].show_plot = 0;
      plot[FOUR].float_center = 0;

      plot[FIVE].plot_id = "FFOM";
      plot[FIVE].units = " ";
      plot[FIVE].ref_scale = (DATA_SIZE) 1.0;
      plot[FIVE].show_plot = 0;
      plot[FIVE].float_center = 0;

      plot[SIX].show_plot = 0;
   }
   else if(rcvr_type == VENUS_RCVR) {
      if(lte_lite) {
         plot[DAC].plot_id = "DAC";
         plot[DAC].units = "%";
         if(user_set_dac_plot == 0) plot[DAC].show_plot = 1;
         if(user_set_pps_plot == 0) plot[PPS].show_plot = 1;
         if(user_set_osc_plot == 0) plot[OSC].show_plot = 1;

         plot[SIX].show_plot = 0;   // dop
         plot[SIX].float_center = 1;
         ppb_string = " ns";
         ppt_string = " ns";
      }
      else {
         plot[DAC].plot_id = "Sawt";
         plot[DAC].units = "ns";
         if(user_set_dac_plot == 0) plot[DAC].show_plot = 0;
         if(user_set_pps_plot == 0) plot[PPS].show_plot = 0;
         if(user_set_osc_plot == 0) plot[OSC].show_plot = 0;

         plot[SIX].show_plot = 1;   // dop
         plot[SIX].float_center = 1;
      }
      plot[DAC].ref_scale = (DATA_SIZE) 1.0;
      if(user_set_dac_float == 0) plot[DAC].float_center = 1;
      if(user_set_pps_float == 0) plot[PPS].float_center = 1;
   }
   else if(rcvr_type == X72_RCVR) {
      if(user_set_pps_plot == 0) plot[PPS].show_plot = 1;
      if(user_set_osc_plot == 0) plot[OSC].show_plot = 1;
      if(user_set_dac_plot == 0) plot[DAC].show_plot = 0;   // disable GPSDO related plots
      if(user_set_pps_float == 0) plot[PPS].float_center = 1;
      if(user_set_osc_float == 0) plot[OSC].float_center = 1;
      if(user_set_osc_float == 0) plot[DAC].float_center = 1;

      plot[OSC].plot_id = "DDS";
      plot[OSC].units = "parts";
      ppt_string = "parts";
      disable_adevs();

      plot[ONE].plot_id = "MVOUTC";
      plot[ONE].units = "x";

      plot[TWO].plot_id = "LVOUTC";
      plot[TWO].units = "V";

      plot[THREE].plot_id = "RVOUTC";
      plot[THREE].units = "V";

      plot[FOUR].plot_id = "DEMAVG";
      plot[FOUR].units = "x";

      plot[FIVE].plot_id = "LVOLTS";
      plot[FIVE].units = "x";

      plot[SIX].plot_id = "MP17";
      plot[SIX].units = "x";

      plot[SEVEN].plot_id = "MP5";
      plot[SEVEN].units = "x";

      plot[EIGHT].plot_id = "PRES";
      plot[EIGHT].units = "x";

      plot[NINE].plot_id = "PLMP";
      plot[NINE].units = "x";

      plot[TEN].plot_id = "HTRVOLT";
      plot[TEN].units = "V";
   }
   else if(rcvr_type == Z12_RCVR) {
      if(user_set_pps_plot == 0) plot[PPS].show_plot = 0;
      if(user_set_dac_plot == 0) plot[DAC].show_plot = 0;
      plot[SIX].plot_id = "DOP";
      plot[SIX].show_plot = 1;   // dop
      plot[SIX].float_center = 1;
   }
   else if(rcvr_type == ZODIAC_RCVR) {
      plot[DAC].show_plot = 0;

      plot[PPS].plot_id = "PPS";
      plot[PPS].units = "ns";  // "us"!!!!!
      plot[PPS].ref_scale = (DATA_SIZE) 1.0;
plot[PPS].scale_factor = 1.0; //1000
      if(user_set_pps_plot == 0) plot[PPS].show_plot = 1;
      if(user_set_pps_float == 0) plot[PPS].float_center = 1;

      plot[OSC].plot_id = "Cofs";
      plot[OSC].units = "ns";
      plot[OSC].ref_scale = (DATA_SIZE) 1.0;
      if(user_set_osc_plot == 0) plot[OSC].show_plot = 1;
      if(user_set_osc_float == 0) plot[OSC].float_center = 1;
      ppt_string = " ns";
      ppb_string = " ns";
      disable_adevs();

      plot[SIX].show_plot = 0;   // dop
      plot[SIX].float_center = 1;

   }
   else if(rcvr_type == ZYFER_RCVR) {
      plot[DAC].units = "%";
      plot[DAC].scale_factor = 1.0;
      plot[DAC].ref_scale = (DATA_SIZE) 1.0;

      if(user_set_pps_float == 0) plot[PPS].float_center = 1;

      if(user_set_osc_plot == 0) plot[OSC].show_plot = 1;
      if(user_set_osc_float == 0) plot[OSC].float_center = 1;

      if(user_set_temp_plot == 0) plot[TEMP].show_plot = 1;

plot[ONE].plot_id = "HEFE";
plot[ONE].units = "ppb";
plot[TWO].plot_id = "HETE";
plot[TWO].units = "ns";
plot[THREE].plot_id = "HEST";
plot[THREE].units = "us";

      plot[FOUR].plot_id = "TFOM";
      plot[FOUR].units = " ";
      plot[FOUR].ref_scale = (DATA_SIZE) 1.0;
      plot[FOUR].show_plot = 0;
      plot[FOUR].float_center = 0;

      plot[FIVE].plot_id = "Drft";
      plot[FIVE].units = "ppb";
      plot[FIVE].ref_scale = (DATA_SIZE) 1.0;
      plot[FIVE].show_plot = 0;
      plot[FIVE].float_center = 1;

      plot[SIX].plot_id = "GDOP";
      plot[SIX].show_plot = 0;  // dop
      plot[SIX].float_center = 1;

      plot[SEVEN].plot_id = "TDEV";
      plot[SEVEN].units = "ns";
      plot[SEVEN].ref_scale = (DATA_SIZE) 1.0;
      plot[SEVEN].show_plot = 0;
      plot[SEVEN].float_center = 1;

      plot[EIGHT].plot_id = "ESSN";
      plot[EIGHT].units = "ns";
      plot[EIGHT].ref_scale = (DATA_SIZE) 1.0;
      plot[EIGHT].show_plot = 0;
      plot[EIGHT].float_center = 1;
   }
   else {  // TSIP_RCVR:  Resolution-T, etc receiver
      plot[PPS].plot_id = "Bias";
      plot[PPS].units = "us";
      plot[PPS].ref_scale = (DATA_SIZE) (1.0/1000.0);
      if(user_set_pps_plot == 0) plot[PPS].show_plot = 1;
      if(user_set_pps_float == 0) plot[PPS].float_center = 1;

      plot[OSC].plot_id = "Rate";
      plot[OSC].units = "ppb";
      plot[OSC].ref_scale = (DATA_SIZE) 1.0;
      if(user_set_osc_plot == 0) plot[OSC].show_plot = 1;
      if(user_set_osc_float == 0) plot[OSC].float_center = 1;
      disable_adevs();

      if(SV6_FAMILY || ACE3 || PALISADE) {
         plot[OSC].plot_id = "Osc";
         plot[OSC].units = "Hz";
         plot[OSC].ref_scale = (DATA_SIZE) 1000.0;
         ppt_string = " Hz";

         if(user_set_osc_plot == 0) plot[OSC].show_plot = 1;
         if(user_set_dac_plot == 0) plot[DAC].show_plot = 0;
         if(user_set_temp_plot == 0) plot[TEMP].show_plot = 0;
         if(user_set_pps_plot == 0) plot[PPS].show_plot = 0;
      }
   }


   if(user_set_adev_plot == 0) {
      plot_adev_data = 0;
      adev_decades_shown = 0;
   }

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
return;  // we now might be using these for other stuff
    if(NO_SATS) return;
    if(rcvr_type == ZYFER_RCVR) return;
    if(rcvr_type == RFTG_RCVR) return;

    // These plots were for Warren S.'s temperature studies
    plot[ONE].plot_center = (DATA_SIZE) 0.0;
    plot[ONE].float_center = 1;
    plot[ONE].plot_id = "D\032H";
    plot[ONE].units = "mHz";

    plot[TWO].plot_center = (DATA_SIZE) 0.0;
    plot[TWO].float_center = 1;
    plot[TWO].plot_id = "DIF";
    plot[TWO].units = "x";

    plot[THREE].plot_center = (DATA_SIZE) 0.0;
    plot[THREE].float_center = 1;
    plot[THREE].plot_id = "D-T";
    plot[THREE].units = "x";

    plot[SIX].plot_center = (DATA_SIZE) (0.0);
    plot[SIX].float_center = 1;
    plot[SIX].plot_id = "[4]";
    plot[SIX].units = "x";

    graph_lla = 0;
}

void config_lla_plots(int keep, int show_lla)
{
    // config G1 G2 and G3 plots to show lat/lon/alt, G6 for DOP
    //   keep=0: disable LLA plots in position hold mode
    //   keep=1: keep LLA plot state in position hold mode
    graph_lla = 0;
    if(lte_lite) ;
    else if(rcvr_type == RFTG_RCVR) ;
    else if(rcvr_type == TIDE_RCVR) ;
    else if(NO_SATS) return;
    if(rcvr_type == ZYFER_RCVR) return;

    plot[ONE].plot_center = (DATA_SIZE) (lat * RAD_TO_DEG);
    plot[ONE].float_center = 1;
    plot[ONE].plot_id = "Lat";
    plot[ONE].units = deg_string;

    plot[TWO].plot_center = (DATA_SIZE) (lon * RAD_TO_DEG);
    plot[TWO].float_center = 1;
    plot[TWO].plot_id = "Lon";
    plot[TWO].units = deg_string;

    plot[THREE].plot_center = (DATA_SIZE) alt;
    plot[THREE].float_center = 1;
    plot[THREE].plot_id = "Alt";
    plot[THREE].units = alt_scale;

    if(show_lla == KEEP_LLA_SHOW) {  // keep current LLA show values
    }
    else if(show_lla == DISABLE_LLA_SHOW) {   // disable showing LLA plots
       plot[ONE].show_plot = 0;
       plot[TWO].show_plot = 0;
       plot[THREE].show_plot = 0;
    }
    else if(show_lla == ENABLE_LLA_SHOW) {   // enable shown LLA plots
       plot[ONE].show_plot = 1;
       plot[TWO].show_plot = 1;
       plot[THREE].show_plot = 1;
    }


    if(1 && (rcvr_type == TIDE_RCVR)) {
       plot[ONE].units = "mm";
       plot[TWO].units = "mm";
       plot[THREE].units = "mm";

       plot[SIX].plot_id = "Grav";
       plot[SIX].units = "uGal";
       plot[SIX].show_plot = 0;
    }
    else if((rcvr_type == RFTG_RCVR) || (rcvr_type == UCCM_RCVR)) {
      plot[SIX].plot_id = "AntI";
      plot[SIX].units = "mA";
      plot[SIX].scale_factor = 1.0;
      plot[SIX].ref_scale = (DATA_SIZE) 1.0;
    }
    else {
       plot[SIX].plot_center = 0.0F;
       plot[SIX].float_center = 1;
       plot[SIX].plot_id = "DOP";
       plot[SIX].units = " ";
    }

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
   time_flags = TFLAGS_UTC; // UTC
   plot_watch = 1;
   plot_adev_data = 0;      // adevs
   adev_decades_shown = 0;
   plot_sat_count = 0;      // satellite count
   adev_period = (-1.0);    // aaaapppp
   pps_adev_period = adev_period;
   osc_adev_period = adev_period;
   chc_adev_period = adev_period;
   chd_adev_period = adev_period;
   save_adev_period();

   osc_gain = user_osc_gain = (69.0);
   if(user_set_rounding == 0) {
      round_temp = 3;
   }

   for(i=0; i<NUM_PLOTS+DERIVED_PLOTS; i++) {     // initialize default plot parameters
      plot[i].show_stat = SHOW_SPAN;   // statistic to show - was AVG
   }

   plot[TEMP].plot_id = "TEMP1";
   plot[TEMP].ref_scale = (DATA_SIZE) 1.0;

   plot[BATTV].plot_id = "BATv";
   plot[BATTV].units = "V";
   plot[BATTV].ref_scale = (DATA_SIZE) 1.0;
// if(user_set_dac_plot == 0) plot[DAC].show_plot = 1;
   if(user_set_dac_float == 0) plot[BATTV].float_center = 1;

   plot[BATTI].plot_id = "BATi";
   plot[BATTI].units = "A";
   plot[BATTI].ref_scale = (DATA_SIZE) 1.0;
   if(user_set_osc_plot == 0)  plot[BATTI].show_plot = 1;
   if(user_set_osc_float == 0) plot[BATTI].float_center = 1;

   plot[LUX1].plot_id = "LUX";
   plot[LUX1].ref_scale = (DATA_SIZE) 1.0;
plot[LUX1].show_stat = SHOW_SPAN;
// if(user_set_pps_plot == 0) plot[PPS].show_plot = 1;
   if(user_set_pps_float == 0) plot[LUX1].float_center = 1;
   if(show_fc) plot[LUX1].units = "fc";
   else        plot[LUX1].units = "lux";

   plot[LUX2].plot_id = "LUM";    // !!!!!
   plot[LUX2].ref_scale = (DATA_SIZE) 1.0;
plot[LUX2].show_stat = SHOW_SPAN;
   if(user_set_pps_float == 0) plot[LUX2].float_center = 1;
   if(show_cp) plot[LUX2].units = "cp";
   else        plot[LUX2].units = "lum";

   plot[LEDV].plot_id = "LEDv";
   plot[LEDV].units = "V";
   plot[LEDV].ref_scale = (DATA_SIZE) 1.0;
   plot[LEDV].float_center = 1;

   plot[LEDI].plot_id = "LEDi";
   plot[LEDI].units = "A";
   plot[LEDI].ref_scale = (DATA_SIZE) 1.0;
   plot[LEDI].float_center = 1;

   plot[PWMHZ].plot_id = "PWM";
   plot[PWMHZ].units = "Hz";
   plot[PWMHZ].ref_scale = (DATA_SIZE) 1.0;
   plot[PWMHZ].float_center = 1;

   plot[TC2].plot_id = "TEMP2";
   plot[TC2].units = degc;
   plot[TC2].ref_scale = (DATA_SIZE) 1.0;
   plot[TC2].float_center = 1;

   plot[BLUEHZ].plot_id = "BLU";
   plot[BLUEHZ].ref_scale = (DATA_SIZE) 1.0;
   plot[BLUEHZ].float_center = 1;

   plot[GREENHZ].plot_id = "GRN";
   plot[GREENHZ].ref_scale = (DATA_SIZE) 1.0;
   plot[GREENHZ].float_center = 1;

   plot[REDHZ].plot_id = "RED";
   plot[REDHZ].ref_scale = (DATA_SIZE) 1.0;
   plot[REDHZ].float_center = 1;

   plot[WHITEHZ].plot_id = "WHT";
   plot[WHITEHZ].ref_scale = (DATA_SIZE) 1.0;
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
   plot[AUXV].ref_scale = (DATA_SIZE) 1.0;
   plot[AUXV].float_center = 1;

   plot[BATTW].plot_id = "BATw";
   plot[BATTW].units = "W";
   plot[BATTW].ref_scale = (DATA_SIZE) 1.0;
   plot[BATTW].float_center = 1;

   plot[LEDW].plot_id = "LEDw";
   plot[LEDW].units = "W";
   plot[LEDW].ref_scale = (DATA_SIZE) 1.0;
   plot[LEDW].float_center = 1;

   plot[EFF].plot_id = "Eff";
   plot[EFF].units = "%";
   plot[EFF].ref_scale = (DATA_SIZE) 1.0;
   plot[EFF].float_center = 1;

   set_cct_id();
   plot[CCT].units = degk;
   plot[CCT].ref_scale = (DATA_SIZE) 1.0;
   plot[CCT].float_center = 1;

   if(user_set_temp_filter == 0) undo_fw_temp_filter = 0;
   if(user_set_clock_plot == 0) { plot_digital_clock = 0; user_set_clock_plot = 1; }

   unit_file_name = "luxor";
   if((log_file == 0) && (user_set_log == 0)) {
      sprintf(log_name, "%s.log", unit_file_name);
   }
   config_set = 1;
}


void reset_data_flags()
{
int i;

   // reset the flags that show what data we have seen from the receiver

   have_antenna = 0;      // config what types of data the receiver can send
   have_osc_age = 0;
   have_saved_posn = 0;
   have_tracking = 0;
   have_sat_azel = 0;
   have_op_mode = 0;
   have_sec_timing = 0;
   have_almanac = 0;
   have_cable_delay = 0;
   have_rf_delay = 0;
   have_pps_delay = 0;
   have_lla_queue = 0;
   have_valid_lla = 0;
   last_lla_span = 1.0;
   have_temperature = 0;
   have_sa35_telem = 0;
   have_furuno_fix_mode = 0;
   traim_deleted = 0;
   have_pps_threshold = 0;
   have_esip_pps_type = 0;
   have_esip_status = 0;
   have_esip_fixmask = 0;
   have_pri_time = 0;
   saw_rcvr_msg = 0;
   jd_obs = 0.0;
   trimble_save = 0;
   first_obs = 1;
   eph_polled = 0;
   have_progress = 0;

   have_scpi_self_test = 0;
   have_scpi_int_power = 0;
   have_scpi_oven_power = 0;
   have_scpi_ocxo = 0;
   have_scpi_efc = 0;
   have_scpi_gps= 0;

   if(user_set_enviro_type == 0) enviro_type = DETECT_ENVIRO;
   have_temperature0 = 0;
   have_temperature1 = 0;
   have_temperature2 = 0;
   have_humidity = 0;
   have_pressure = 0;
   have_adc1 = 0;
   have_adc2 = 0;
   have_adc3 = 0;
   have_adc4 = 0;
   have_dp = 0;
   have_rem_temp = 0;
   have_ref_baro_alt = 0;
   have_lux = 0;
   have_lfs_counter = 0;
   have_lfs_emis = 0;
   enviro_sensors = 0;
   last_enviro = 0;
   next_enviro = 0;

   have_gps_status = 0;
   have_eeprom = 0;
   have_seg_info = 0;
   saw_version = 0;
   saw_timing_msg = 0;
   got_timing_msg = 0;
   roll_timing_msg = 0;
   ee_write_count = 0;
   last_eclipse = (-1);

   have_tfom = 0;
   have_ffom = 0;

   have_dac = 0;
   have_sawtooth = 0;
   have_osc_offset = 0;
   have_pps_offset = 0;
   have_chc_offset = 0;
   have_chd_offset = 0;

   have_rcvr_pps = 0;
   have_rcvr_osc = 0;
   have_count = 0;

   have_heading = 0;
   have_speed = 0;
   have_dops = 0;
   have_filter = 0;

   have_datum = 0;
   have_pdop_mask = 0;

// last_utc_ofs = 0;
   if(!user_set_utc_ofs) have_utc_ofs = 0;
   utc_offset_flag = 0;

   have_leap_info = 0;
   have_leap_days = 0;
   have_jd_leap = 0;
   have_scpi_hex_leap = 0;
   leap_days = (-1);
   guessed_leap_days = ' ';

   have_tow = 0;
   have_week = 0;
   rolled = 0;
   auto_rollover = 0.0;
   if(!user_set_rollover) rollover = 0.0;

   have_gnss_mask = 0;
   default_gnss = MIXED;

   saw_mini = 0;
   saw_ntpx = 0;
   saw_nortel = 0;
   saw_icm = 0;
   saw_gpsdo = 0;
   saw_sv6_time = 0;

   have_brandy_rate = 0;
   have_brandy_width = 0;
   have_brandy_code = 0;

   have_tm4_code = 0;
   have_tm4_ett_enable = 0;
   have_ett_time_format = 0;
   have_ett_code_format = 0;
   have_ett_polarity = 0;
   have_tm4_antenna = 0;
   have_tm4_pps_source = 0;
   have_tm4_enhanced = 0;
   have_tm4_time_baud = 0;
   have_tm4_ett_jd = 0;
   have_tm4_pop_jd = 0;
   have_tm4_pop_mode = 0;
   have_tm4_pop_polarity = 0;
   have_tm4_repeat = 0;
   have_tm4_width = 0;
   have_tm4_lock = 0;
   tm4_id[0] = 0;
   tm4_hex[0] = 0;
   tm4_obj[0] = 0;
   tm4_string[0] = 0;
   ett_jd = 0.0;
   pop_jd = 0.0;

   for(i=0; i<MAX_CS_LOGS; i++) cs_log_msg[i][0] = 0;
   have_cs_time = 0;
   have_cs_mjd = 0;
   have_cs_cbtid = 0;
   have_cs_temp = 0;
   need_cs_timeset = 0;
   have_cs_logcount = 0;
   have_cs_loginfo = 0;
   have_cs_cont = 0;
   have_cs_beam = 0;
   have_cs_cfield = 0;
   have_cs_pump = 0;
   have_cs_gain = 0;
   have_cs_rfam = 0;
   have_cs_glob = 0;
   have_cs_supply = 0;
   have_cs_coven = 0;
   have_cs_qoven = 0;
   have_cs_emul = 0;
   have_cs_hwi = 0;
   have_cs_msp = 0;
   have_cs_pll = 0;
   have_cs_volt = 0;
   have_cs_freq1 = 0;
   have_cs_freq2 = 0;
   have_cs_ster = 0;
   have_cs_sync = 0;
   have_cs_slew = 0;
   have_cs_standby = 0;
   have_cs_remote = 0;
   have_cs_disp = 0;
   have_cs_leapdur = 0;
   have_cs_leapmjd = 0;
   have_cs_leapstate = 0;

   have_gpgga = 0;
   have_gprmc = 0;
   have_gpgns = 0;
   have_gpzda = 0;
   have_gpgsv = 0;
   have_nmea_date = 0;
   nmea_type = 0;
   saw_venus_raw = 0;
   have_rtk_mode = 0;
   have_raw_rate = 0;

   saw_gpsd_pps = 0;
   saw_gpsd_pre = 0;
   saw_diff_change = 0;

   have_moto_Ba = 0;
   have_moto_Ea = 0;
   have_moto_Bg = 0;
   have_moto_Eg = 0;
   have_moto_Bk = 0;
   have_moto_Ek = 0;
   have_moto_Bn = 0;
   have_moto_En = 0;
   have_moto_Gj = 0;
   have_moto_Hr = 0;
   have_moto_range = 0;
   datum_flag = 0;
   sv6_flag = 0;

   have_prs_st = 0;
   have_prs_lm = 0;
   have_prs_lo = 0;
   have_prs_fc = 0;
   have_prs_ds = 0;
   have_prs_sf = 0;
   have_prs_ss = 0;
   have_prs_ga = 0;
   have_prs_ph = 0;
   have_prs_sp = 0;
   have_prs_ms = 0;
   have_prs_mo = 0;
   have_prs_mr = 0;
   have_prs_tt = 0;
   have_prs_ts = 0;
   have_prs_to = 0;
   have_prs_ps = 0;
   have_prs_pl = 0;
   have_prs_pt = 0;
   have_prs_pf = 0;
   have_prs_pi = 0;
   have_prs_sd = 0;
   have_prs_ad = 0;

   have_x72_creg = 0;
   have_x72_scont = 0;
   have_x72_sernum = 0;
   have_x72_pwrticks = 0;
   have_x72_lhhrs = 0;
   have_x72_lhticks = 0;
   have_x72_rhhrs = 0;
   have_x72_rhticks = 0;
   have_x72_dmp17 = 0;
   have_x72_dmp5 = 0;
   have_x72_dhtrvolt = 0;
   have_x72_plmp = 0;
   have_x72_pres = 0;
   have_x72_dlvthermc = 0;
   have_x72_drvthermc = 0;
   have_x72_dlvolt = 0;
   have_x72_dmvoutc = 0;
   have_x72_dtemplo = 0;
   have_x72_dtemphi = 0;
   have_x72_dvoltlo = 0;
   have_x72_dvolthi = 0;
   have_x72_dlvoutc = 0;
   have_x72_drvoutc = 0;
   have_x72_dmv2demavg = 0;
   have_x72_pps = 0;
   have_x72_state = 0;
   have_x72_info = 0;
   have_x72_dds_word = 0;
   have_x72_fw_dis = 0;
   have_x72_fw_dmode = 0;
   x72_state_set = 0;

   have_rftg_unit = 0;
   have_rftg_lla = 0;
   have_pll = 0;

   have_scpi_test = 0;
   scpi_type_changed = 0;
   scpi_echo_mode = 0;
   have_lifetime = 0;
   adjust_scpi_ss = 0;

   have_sirf_pps = 0;

   have_star_perf = 0;
   have_star_atdc = 0;
   have_star_wtr = 0;
   have_star_hbsq = 0;
   have_star_input = 0;
   have_star_led = 0;
   have_star_track = 0;

   have_osa_stat = 0;
   have_osa_prior = 0;
   have_osa_output = 0;
   have_osa_adm = 0;
   have_osa_aux = 0;
   saw_star_time = 0;

// have_ticc_mode = 0;
   have_ticc_fudge = 0;
   have_ticc_edges = 0;
   have_ticc_time2 = 0;
   have_ticc_dilat = 0;
   have_ticc_eeprom = 0;
   have_ticc_board = 0;
   have_ticc_cal = 0;
   have_ticc_timeout = 0;
   have_ticc_syncmode = 0;
   have_ticc_speed = 0;
   have_ticc_coarse = 0;
   last_ticc_v1 = 0.0;       // last channel readings from TICC_RCVR
   last_ticc_v2 = 0.0;
   last_ticc_v3 = 0.0;
   last_ticc_v4 = 0.0;

   have_true_eval = 0;
   have_true_debug = 0;
   have_true_pot = 0;
   have_atten = 0;
   have_true_scale = 0;

   ubx_sw[0] = 0;
   ubx_hw[0] = 0;
   for(i=0; i<UBX_ROM_INFO; i++) ubx_rom[i][0] = 0;
   ubx_fw_ver = 0.0F;
//   saw_ubx_tp5 = 0;
//   saw_ubx_tp = 0;

   saw_uccm_dmode = 0;
   have_gpsdo_ref = 0;
   have_uccm_loop = 1;
   have_uccm_tcor = 0;
   have_uccm_pcor = 0;
   have_uccm_gps_phase = 0;
   have_uccm_ext_val = 0;

   have_ant_v1 = 0;
   have_ant_v2 = 0;
   have_ant_ma = 0;
   have_pullin = 0;

   have_zyfer_drift = 0;
   have_zyfer_essn = 0;
   have_zyfer_tdev = 0;
   have_zyfer_hest = 0;
   have_zyfer_hefe = 0;
   have_zyfer_hete = 0;
   have_zyfer_hint = 0;

   have_single_prn = 0;
   have_io_options = 0;
   has_id_info = 0;
   holdover_seen = 0;
   have_last_stamp = 0;

   have_nav_rate = 0;
//   if(TICC_USED == 0) nav_rate = 1.0;

   have_critical_alarms = 0;

   have_pps_rate = 0;
   have_pps_enable = 0;
   have_pps_polarity = 0;
   have_pps_mode = 0;
   pps_enabled = 0;
   have_pps_duty = 0;
   have_pps_freq = 0;
   have_osc_polarity = 0;
   have_z12_pps = 0;
   have_z12_ext = 0;

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

   have_state = 0;
   have_doppler = 0;
   have_phase = 0;
   have_range = 0;
   have_snr = 0;
   have_l1_doppler = 0;
   have_l1_phase = 0;
   have_l1_range = 0;
   have_l1_snr = 0;
   have_l2_doppler = 0;
   have_l2_phase = 0;
   have_l2_range = 0;
   have_l2_snr = 0;
   have_l5_doppler = 0;
   have_l5_phase = 0;
   have_l5_range = 0;
   have_l5_snr = 0;
   have_l6_doppler = 0;
   have_l6_phase = 0;
   have_l6_range = 0;
   have_l6_snr = 0;
   have_l7_doppler = 0;
   have_l7_phase = 0;
   have_l7_range = 0;
   have_l7_snr = 0;
   have_l8_doppler = 0;
   have_l8_phase = 0;
   have_l8_range = 0;
   have_l8_snr = 0;

   have_accum_range = 0;
   have_bias = 0;
   have_accu = 0;
   have_ubx_rawx = 0;
   have_ubx_tmode = 0;

   have_kalman = 1;
   saw_kalman_on = 0;
   have_timing_mode = 0;
   have_rcvr_tmode = 0;
   have_amu = 0;
   have_el_mask = 0;
   have_build = 0;
   have_traim = 0;
   use_traim_lla = 0;

   have_rcvr_mode = 0;
   last_rcvr_mode = 0;
   first_request = 1;
   if(user_set_temp_filter) ;
   else undo_fw_temp_filter = 0;
}


void config_rcvr_type(int set_baud)
{
int prn;

   // setup the program to work with the specified receiver type
   // NEW_RCVR

if(rcvr_type == LUXOR_RCVR) {
   detect_rcvr_type = 0;
   set_baud = 1;
}

   plot[SAT_PLOT].user_scale = 1;
   if(user_set_sat_plot == 0) {
      if(NO_SATS) plot[SAT_PLOT].show_plot = 0;
      else        plot[SAT_PLOT].show_plot = 1;
   }

   timeout_extended = 0;

   if(detect_rcvr_type) ;
   else if(set_baud == 0) ;
   else if(com[RCVR_PORT].user_set_baud == 0) {
      com[RCVR_PORT].baud_rate = 9600;
      com[RCVR_PORT].data_bits = 8;
      com[RCVR_PORT].parity = parity_defined;
      com[RCVR_PORT].stop_bits = 1;
   }

   last_time_msec = GetMsecs();  // initialize ellapsed millisecond counter
   this_time_msec = last_time_msec + 1000.0;

   max_sat_count = 8;
   if(max_sat_count > max_sat_display) max_sat_count = max_sat_display;
   force_mode_change = 0;

   find_endian();

   user_set_rcvr = 1;
//rnx   need_msg_init = 321;
   strcpy(uccm_led_msg, "UNKNOWN STATE");

   ebolt = 0;
// lte_lite = 0;

   pkt_end1 = pkt_end2 = (-1);
   pkt_start1 = pkt_start2 = (-1);
   packet_count = 0;
   timing_seen = 0;
   ticc_packets = 0;
   first_msg = 1;
   first_request = 1;

   need_queue_reset = 1;
   dont_reset_queues = 0;

   reset_data_flags();   // reset the data seen flags, etc
   keep_lla_plots = 0;

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
   reinit_jd = 0.0;

   if(rcvr_type != TSIP_RCVR) {
      if(!user_set_dops) plot_dops = 1;
      res_t = 0;
   }

   if(luxor) unit_file_name = "luxor";
   else if(ACU_360) unit_file_name = "acutime";
   else if(ACU_GG) unit_file_name = "acutime";
   else if(ACUTIME) unit_file_name = "acutime";
   else if(PALISADE) unit_file_name = "palisade";
   else unit_file_name = "tbolt";

// min_sig_db = 30;      // low sig level threshold for sig level map
// sig_level_step = 2;   // sig level map signal step size

   min_sig_db = 20;      // low sig level threshold for sig level map
   sig_level_step = 3;   // sig level map signal step size
   need_sunrise = 1;

   if(rcvr_type == ACRON_RCVR) { 
      unit_file_name = "acron";
      config_lla_plots(KEEP_LLA_ON_HOLD, DISABLE_LLA_SHOW);

      if(detect_rcvr_type) ;
      else if(set_baud == 0) ;
      else if(com[RCVR_PORT].user_set_baud == 0) {
         com[RCVR_PORT].baud_rate = 300;
         com[RCVR_PORT].data_bits = 8;
         com[RCVR_PORT].parity = NO_PAR;
         com[RCVR_PORT].stop_bits = 2;
      }

      config_utc_mode(1);
      pps_enabled = 0;

      plot_azel = 1;
      plot_signals = 0;
      shared_plot = 0;
      plot_watch = 1;
      plot_digital_clock = 1;
      rcvr_mode = RCVR_MODE_HOLD;
   }
   else if(rcvr_type == BRANDY_RCVR) {
      unit_file_name = "brandy";
      have_timing_mode = 1;
      config_lla_plots(KEEP_LLA_ON_HOLD, DISABLE_LLA_SHOW);

      config_rcvr_plots();

      have_kalman = 0;
      brandy_pps_rate = 1000;  // assume 1000 msec PPS rate
      brandy_pps_width = 1;    // assume 1 msec PPS width

      config_utc_mode(0);

      if(detect_rcvr_type) ;
      else if(set_baud == 0) ;
      else if(com[RCVR_PORT].user_set_baud == 0) {
         com[RCVR_PORT].baud_rate = 4800;
         com[RCVR_PORT].data_bits = 8;
         com[RCVR_PORT].parity = NO_PAR;
         com[RCVR_PORT].stop_bits = 1;
      }

      pkt_end1 = 0x0D;
      pkt_end2 = 0x0A;
   }
   else if(rcvr_type == CS_RCVR) {
      unit_file_name = "cesium";
      has_id_info = 1;
      config_lla_plots(KEEP_LLA_ON_HOLD, DISABLE_LLA_SHOW);
      plot_azel = 0;
      plot_signals = 0;
      shared_plot = 0;

      have_kalman = 0;
      have_temperature = 1;
      pps_enabled = 1;

      config_utc_mode(1);
      config_rcvr_plots();

      pkt_end1 = 0x0D;          // used to pretty-print the data stream log file
      pkt_end2 = 0x0A;
   }
   else if(rcvr_type == ESIP_RCVR) {
      unit_file_name = "esip";
      have_timing_mode = 1;

      if(detect_rcvr_type) ;
      else if(set_baud == 0) ;
      else if(com[RCVR_PORT].user_set_baud == 0) {
         com[RCVR_PORT].baud_rate = 38400;
         com[RCVR_PORT].data_bits = 8;
         com[RCVR_PORT].parity = NO_PAR;
         com[RCVR_PORT].stop_bits = 1;
      }

max_sat_display = 14;
max_sats = max_sat_display;  // used to format the sat_info data
max_sat_count = max_sat_display;
temp_sats = max_sat_display;
config_sat_rows();
      config_lla_plots(KEEP_LLA_ON_HOLD, DISABLE_LLA_SHOW);

      config_rcvr_plots();

      have_kalman = 0;
      dynamics_code = 2;
      smoothing_code = 2;

      pkt_end1 = 0x0D;
      pkt_end2 = 0x0A;
   }
   else if(rcvr_type == FURUNO_RCVR) {
      unit_file_name = "furuno";
      have_timing_mode = 1;
max_sat_display = 14;
max_sats = max_sat_display;  // used to format the sat_info data
max_sat_count = max_sat_display;
temp_sats = max_sat_display;
config_sat_rows();
      config_lla_plots(KEEP_LLA_ON_HOLD, ENABLE_LLA_SHOW);

      config_rcvr_plots();

      have_kalman = 0;
      dynamics_code = 2;
      smoothing_code = 2;

      pkt_end1 = 0x0D;
      pkt_end2 = 0x0A;
   }
   else if(rcvr_type == GPSD_RCVR) {
      unit_file_name = "gpsd";
      has_id_info = 1;
      have_timing_mode = 1;
      config_lla_plots(KEEP_LLA_ON_HOLD, ENABLE_LLA_SHOW);

      have_kalman = 0;

      config_utc_mode(1);
      config_rcvr_plots();

      pkt_end1 = 0x0D;          // used to pretty-print the data stream log file
      pkt_end2 = 0x0A;
   }
   else if(rcvr_type == LUXOR_RCVR) {
      tsip_type = LUXOR_TYPE;
      luxor = 20;
      has_id_info = 1;
      max_sats = max_sat_display = 0;
      config_sat_rows();

      if(detect_rcvr_type) {
         com[RCVR_PORT].baud_rate = 9600;
         com[RCVR_PORT].data_bits = 8;
         com[RCVR_PORT].parity = ODD_PAR;
         com[RCVR_PORT].stop_bits = 1;
      }
      else if(set_baud == 0) ;
      else if(com[RCVR_PORT].user_set_baud == 0) {
        com[RCVR_PORT].baud_rate = 9600;
        com[RCVR_PORT].data_bits = 8;
        com[RCVR_PORT].parity = LUXOR_PAR;
        com[RCVR_PORT].stop_bits = 1;
      }

      time_flags = TFLAGS_NULL;
      config_utc_mode(1);
      plot_azel = 0;

      config_luxor_plots();
      config_screen(6578);
      pkt_end1 = DLE;
      pkt_end2 = ETX;
   }
   else if(rcvr_type == LPFRS_RCVR) {
      unit_file_name = "lpfrs";
      has_id_info = 1;
      if(detect_rcvr_type) ;
      else if(set_baud == 0) ;
      else if(com[RCVR_PORT].user_set_baud == 0) {
         com[RCVR_PORT].baud_rate = 1200;
         com[RCVR_PORT].data_bits = 8;
         com[RCVR_PORT].parity = NO_PAR;
         com[RCVR_PORT].stop_bits = 1;
      }
      config_lla_plots(KEEP_LLA_ON_HOLD, DISABLE_LLA_SHOW);
      plot_azel = 0;
      plot_signals = 0;
      shared_plot = 0;

      have_kalman = 0;
      have_temperature = 0;
      pps_enabled = 1;

      config_utc_mode(1);
      config_rcvr_plots();

      pkt_end1 = 0x0D;          // used to pretty-print the data stream log file
      pkt_end1 = 0x0A;
   }
   else if(rcvr_type == MOTO_RCVR) {
      unit_file_name = "moto";
      has_id_info = 1;

      config_lla_plots(KEEP_LLA_ON_HOLD, DISABLE_LLA_SHOW);

      have_kalman = 0;
//    have_temperature = 2;

      if(moto_type == VP_TYPE) {  // VP series does not do @@Bo to get UTC offset
         config_utc_mode(1);
      }
      config_rcvr_plots();

      pkt_end1 = 0x0D;          // used to pretty-print the data stream log file
      pkt_end2 = 0x0A;
   }
   else if(rcvr_type == NMEA_RCVR) {
      unit_file_name = "nmea";
      have_timing_mode = 1;
      config_lla_plots(KEEP_LLA_ON_HOLD, ENABLE_LLA_SHOW);

      config_rcvr_plots();

      have_kalman = 0;

      config_utc_mode(1);

      pkt_end1 = 0x0D;
      pkt_end2 = 0x0A;
   }
   else if(rcvr_type == NO_RCVR) { 
      if(enable_terminal == 0) {
         com[RCVR_PORT].process_com = 0;
         com[RCVR_PORT].user_disabled_com = 1;
      }
      if(calc_rcvr) unit_file_name = "calc";
      else          unit_file_name = "clock";
      config_lla_plots(KEEP_LLA_ON_HOLD, DISABLE_LLA_SHOW);

      config_utc_mode(1);
      pps_enabled = 0;

      plot_azel = 0;
      plot_watch = 1;
      plot_digital_clock = 1;
   }
   else if(rcvr_type == NVS_RCVR) {
      unit_file_name = "nvs";
      has_id_info = 1;
      config_lla_plots(KEEP_LLA_ON_HOLD, DISABLE_LLA_SHOW);

      if(detect_rcvr_type) ;
      else if(set_baud == 0) ;
      else if(com[RCVR_PORT].user_set_baud == 0) {
         com[RCVR_PORT].baud_rate = 115200;
         com[RCVR_PORT].data_bits = 8;
         com[RCVR_PORT].parity = ODD_PAR;
         com[RCVR_PORT].stop_bits = 1;
      }
      default_gnss = (GPS | GLONASS | SBAS);

      config_rcvr_plots();

      pkt_end1 = DLE;
      pkt_end2 = ETX;
   }
   else if(rcvr_type == PRS_RCVR) {
      unit_file_name = "prs10";
      has_id_info = 1;
      config_lla_plots(KEEP_LLA_ON_HOLD, DISABLE_LLA_SHOW);
      plot_azel = 0;
      plot_signals = 0;
      shared_plot = 0;

      have_kalman = 0;
      have_temperature = 3;
      pps_enabled = 1;

      config_utc_mode(1);
      config_rcvr_plots();

if(user_set_adev_period == 0) adev_period = 2.0;       // aaaapppp
if(user_set_pps_period == 0)  pps_adev_period = 2.0;
if(user_set_osc_period == 0)  osc_adev_period = 2.0;
if(user_set_chc_period == 0)  chc_adev_period = 2.0;
if(user_set_chd_period == 0)  chd_adev_period = 2.0;
save_adev_period();

      pkt_end1 = 0x0D;          // used to pretty-print the data stream log file
   }
   else if(rcvr_type == RFTG_RCVR) {
      unit_file_name = "rftg";
      has_id_info = 1;
      config_lla_plots(KEEP_LLA_ON_HOLD, DISABLE_LLA_SHOW);

      config_utc_mode(1);

      config_rcvr_plots();

      pkt_end1 = DLE;
      pkt_end2 = ETX;
   }
   else if(rcvr_type == RT17_RCVR) {
      unit_file_name = "rt17";
      have_timing_mode = 1;
      if(detect_rcvr_type) ;
      else if(set_baud == 0) ;
      else if(com[RCVR_PORT].user_set_baud == 0) {
         com[RCVR_PORT].baud_rate = 115200;
         com[RCVR_PORT].data_bits = 8;
         com[RCVR_PORT].parity = NO_PAR;
         com[RCVR_PORT].stop_bits = 1;
      }
      config_lla_plots(KEEP_LLA_ON_HOLD, DISABLE_LLA_SHOW);

      config_rcvr_plots();

      have_kalman = 0;

      config_utc_mode(1);
//have_utc_ofs = 0;

      pkt_end1 = 0x03;
   }
   else if(rcvr_type == SA35_RCVR) {
      unit_file_name = "sa3x";
      has_id_info = 1;
      if(detect_rcvr_type) ;
      else if(set_baud == 0) ;
      else if(com[RCVR_PORT].user_set_baud == 0) {
         com[RCVR_PORT].baud_rate = 57600;
         com[RCVR_PORT].data_bits = 8;
         com[RCVR_PORT].parity = NO_PAR;
         com[RCVR_PORT].stop_bits = 1;
      }
      config_lla_plots(KEEP_LLA_ON_HOLD, DISABLE_LLA_SHOW);
      plot_azel = 0;
      plot_signals = 0;
      shared_plot = 0;

      have_kalman = 0;
      have_temperature = 0;
      pps_enabled = 1;

      config_utc_mode(1);
      config_rcvr_plots();

      pkt_end1 = 0x0D;          // used to pretty-print the data stream log file
      pkt_end1 = 0x0A;
   }
   else if(rcvr_type == SCPI_RCVR) {
      unit_file_name = "scpi";
      has_id_info = 1;
      config_lla_plots(KEEP_LLA_ON_HOLD, DISABLE_LLA_SHOW);

      if(detect_rcvr_type) ;
      else if(set_baud == 0) ;
      else if(com[RCVR_PORT].user_set_baud == 0) {
         if(scpi_type == LUCENT_TYPE) {
            com[RCVR_PORT].baud_rate = 19200;  // use 19200:8:N:1
            com[RCVR_PORT].data_bits = 8;
            com[RCVR_PORT].parity = NO_PAR;         // odd
            com[RCVR_PORT].stop_bits = 1;
         }
         else if((scpi_type != HP_TYPE) && (scpi_type != HP_TYPE2) && (scpi_type != NORTEL_TYPE)) {  // Z3801A type receivers
            com[RCVR_PORT].baud_rate = 19200;  // use 19200:7:O:1
            com[RCVR_PORT].data_bits = 7;
            com[RCVR_PORT].parity = ODD_PAR;         // odd
            com[RCVR_PORT].stop_bits = 1;
         }
      }

      if(scpi_type == NORTEL_TYPE) {
         unit_file_name = "nortel";
         config_utc_mode(0);
         level_type = "SNR";
      }
      else if(scpi_type == LUCENT_TYPE) {
         unit_file_name = "lucent";
         min_sig_db = 30;      // low sig level threshold for sig level map
         sig_level_step = 2;   // sig level map signal steo size
      }
      else {
         min_sig_db = 30;      // low sig level threshold for sig level map
         sig_level_step = 2;   // sig level map signal steo size
      }

if(user_set_adev_period == 0) adev_period = 10.0;       // aaaapppp
if(user_set_pps_period == 0)  pps_adev_period = 10.0;
if(user_set_osc_period == 0)  osc_adev_period = 10.0;
if(user_set_chc_period == 0)  chc_adev_period = 10.0;
if(user_set_chd_period == 0)  chd_adev_period = 10.0;
save_adev_period();

      Sleep(500);
      scpi_init(0);
      saw_gpsdo = 10;
      config_rcvr_plots();

      // pkt_end1 is set in send_scpi_cmd() since end byte is device dependent
      have_kalman = 0;
   }
   else if(rcvr_type == SIRF_RCVR) {
      unit_file_name = "sirf";
      has_id_info = 1;
      config_lla_plots(KEEP_LLA_ON_HOLD, ENABLE_LLA_SHOW);

      have_kalman = 0;
      config_rcvr_plots();
      pps_enabled = 1;

      pkt_end1 = 0xB0;
      pkt_end2 = 0xB3;

      ppb_string = " ns";
      ppt_string = " ns";
   }
   else if(rcvr_type == SRO_RCVR) {
      unit_file_name = "sro";
      has_id_info = 1;
      if(detect_rcvr_type) ;
      else if(set_baud == 0) ;
      else if(com[RCVR_PORT].user_set_baud == 0) {
         com[RCVR_PORT].baud_rate = 9600;
         com[RCVR_PORT].data_bits = 8;
         com[RCVR_PORT].parity = NO_PAR;
         com[RCVR_PORT].stop_bits = 1;
      }
      config_lla_plots(KEEP_LLA_ON_HOLD, DISABLE_LLA_SHOW);
      plot_azel = 0;
      plot_signals = 0;
      shared_plot = 0;

      have_kalman = 0;
      have_temperature = 0;
      pps_enabled = 1;

      config_utc_mode(1);
      config_rcvr_plots();

      pkt_end1 = 0x0D;          // used to pretty-print the data stream log file
      pkt_end1 = 0x0A;
   }
   else if(rcvr_type == SS_RCVR) {  // Novatel SuperStar II
      unit_file_name = "SS_II";
      config_lla_plots(KEEP_LLA_ON_HOLD, DISABLE_LLA_SHOW);
      config_rcvr_plots();

      if(detect_rcvr_type) ;
      else if(set_baud == 0) ;
      else if(com[RCVR_PORT].user_set_baud == 0) {
         com[RCVR_PORT].baud_rate = 19200;
         com[RCVR_PORT].data_bits = 8;
         com[RCVR_PORT].parity = NO_PAR;
         com[RCVR_PORT].stop_bits = 1;
      }
pkt_end1 = 0x01;  //piss3 (actually packet start)
   }
   else if(rcvr_type == STAR_RCVR) {
      if(star_type == NEC_TYPE) unit_file_name = "nec";
      else if(star_type == OSA_TYPE) unit_file_name = "osa";
      else unit_file_name = "star";

      have_timing_mode = 1;
      config_lla_plots(KEEP_LLA_ON_HOLD, DISABLE_LLA_SHOW);

      if(detect_rcvr_type) ;
      else if(set_baud == 0) ;
      else if(com[RCVR_PORT].user_set_baud == 0) {
         if(star_type == NEC_TYPE) {
            com[RCVR_PORT].baud_rate = 115200;
            com[RCVR_PORT].data_bits = 8;
            com[RCVR_PORT].parity = NO_PAR;
            com[RCVR_PORT].stop_bits = 2;
         }
         else {
            com[RCVR_PORT].baud_rate = 9600;
            com[RCVR_PORT].data_bits = 8;
            com[RCVR_PORT].parity = NO_PAR;
            com[RCVR_PORT].stop_bits = 2;
         }
      }
      star_restart_ok = 0;

      have_kalman = 0;
      if(star_type == OSA_TYPE) {
         have_temperature = 0;
         config_rcvr_plots();
         config_utc_mode(1);
      }
      else {
         have_temperature = 4;
         has_id_info = 1;
         config_rcvr_plots();
         config_utc_mode(0);
      }


      pkt_end1 = 0x0D;
      pkt_end2 = 0x0A;
   }
   else if(rcvr_type == TAIP_RCVR) {
      unit_file_name = "taip";
      has_id_info = 1;
      have_timing_mode = 1;
      config_lla_plots(KEEP_LLA_ON_HOLD, ENABLE_LLA_SHOW);

      if(detect_rcvr_type) ;
      else if(set_baud == 0) ;
      else if(com[RCVR_PORT].user_set_baud == 0) {
         com[RCVR_PORT].baud_rate = 4800;
         com[RCVR_PORT].data_bits = 8;
         com[RCVR_PORT].parity = NO_PAR;
         com[RCVR_PORT].stop_bits = 1;
      }

      config_rcvr_plots();

      have_kalman = 0;

      config_utc_mode(0);

      pkt_end1 = '<';
   }
   else if(rcvr_type == TERM_RCVR) {
      unit_file_name = "term";
      config_lla_plots(KEEP_LLA_ON_HOLD, DISABLE_LLA_SHOW);

      config_utc_mode(1);
      pps_enabled = 0;

      plot_azel = 0;
      plot_watch = 0;
      plot_digital_clock = 0;
   }
   else if(rcvr_type == THERMO_RCVR) {
      unit_file_name = "enviro";

      if(detect_rcvr_type) ;
      else if(set_baud == 0) ;
      else if(com[RCVR_PORT].user_set_baud == 0) {
         com[RCVR_PORT].baud_rate = 115200;
         com[RCVR_PORT].data_bits = 8;
         com[RCVR_PORT].parity = NO_PAR;
         com[RCVR_PORT].stop_bits = 1;
      }

      config_rcvr_plots();

      have_kalman = 0;

      config_utc_mode(1);

      rcvr_mode = RCVR_MODE_UNKNOWN;
      pkt_end1 = 0x0D;
      pkt_end2 = 0x0A;
   }
   else if(rcvr_type == TICC_RCVR) {
      unit_file_name = "ticc";
      if(ticc_type == PICPET_TICC) unit_file_name = "picpet";
      has_id_info = 1;

      if(detect_rcvr_type) ;    // TICC_ECHO_PORT?
      else if(set_baud == 0) ;
      else if(com[RCVR_PORT].user_set_baud == 0) {
         if(ticc_type == PICPET_TICC) com[RCVR_PORT].baud_rate = 19200;
         else if(ticc_type == TAPR_TICC) com[RCVR_PORT].baud_rate = 115200;
         else if(ticc_type == LARS_TICC) com[RCVR_PORT].baud_rate = 9600;
         else com[RCVR_PORT].baud_rate = 115200;
         com[RCVR_PORT].data_bits = 8;
         com[RCVR_PORT].parity = NO_PAR;
         com[RCVR_PORT].stop_bits = 1;
      }

// aaaaahhhhh    enable_terminal = 1;
      config_rcvr_plots();

      have_kalman = 0;

      config_utc_mode(1);

      rcvr_mode = RCVR_MODE_UNKNOWN;
      pkt_end1 = 0x0D;
      pkt_end2 = 0x0A;
   }
   else if(rcvr_type == TIDE_RCVR) {
      if(enable_terminal == 0) {
         com[RCVR_PORT].process_com = 0;
         com[RCVR_PORT].user_disabled_com = 1;
      }
      unit_file_name = "gravity";  // ckckck
      config_lla_plots(KEEP_LLA_ON_HOLD, DISABLE_LLA_SHOW);

      config_rcvr_plots();
      config_tide_plots(1);
      plot[SIX].show_plot = 1;

      plot_signals = 0;
      shared_plot = 0;
      plot_azel = 0;
      if(user_show_tides == 0) show_tides = 1;
      if(user_fix_set == 0) {
         show_fixes = 1;
         user_fix_set = 1;
      }

      config_utc_mode(1);
      pps_enabled = 0;

      plot_azel = 0;
      plot_watch = 1;
      plot_digital_clock = 1;
   }
   else if(rcvr_type == TM4_RCVR) {
      unit_file_name = "spectrum";
      has_id_info = 0;

      have_timing_mode = 1;
      config_lla_plots(KEEP_LLA_ON_HOLD, ENABLE_LLA_SHOW);

      config_rcvr_plots();

      min_sig_db = 26;      // low sig level threshold for sig level map
      sig_level_step = 2;   // sig level map signal steo size

      have_kalman = 0;

      config_utc_mode(1);

      pkt_end1 = 0x0D;
      pkt_end2 = 0x0A;
   }
   else if(rcvr_type == TRUE_RCVR) {
      unit_file_name = "trueposn";
      has_id_info = 1;

      have_timing_mode = 1;
      config_lla_plots(KEEP_LLA_ON_HOLD, DISABLE_LLA_SHOW);

      config_rcvr_plots();

      min_sig_db = 26;      // low sig level threshold for sig level map
      sig_level_step = 2;   // sig level map signal steo size

      have_kalman = 0;

      config_utc_mode(0);

      pkt_end1 = 0x0D;
      pkt_end2 = 0x0A;
   }
   else if(rcvr_type == TSERVE_RCVR) {
      unit_file_name = "tserve";
      have_timing_mode = 1;
      config_lla_plots(KEEP_LLA_ON_HOLD, DISABLE_LLA_SHOW);

      config_rcvr_plots();

      have_kalman = 0;

      config_utc_mode(1);

      pkt_end1 = 0x0D;
      pkt_end2 = 0x0A;
   }
   else if(rcvr_type == TSIP_RCVR) {
      unit_file_name = "tbolt";
      has_id_info = 1;
      config_lla_plots(KEEP_LLA_ON_HOLD, DISABLE_LLA_SHOW);
      config_tide_plots(0);

      if(tsip_type == STARLOC_TYPE) {
         unit_file_name = "starloc";
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
      else if(ACU_GG || ACU_360 || ACUTIME || PALISADE) {
         unit_file_name = "acutime";
         if(detect_rcvr_type) ;
         else if(set_baud == 0) ;
         else if(com[RCVR_PORT].user_set_baud == 0) {
            if(ACU_GG) {
               com[RCVR_PORT].baud_rate = 9600;
               com[RCVR_PORT].data_bits = 8;
               com[RCVR_PORT].parity = ODD_PAR;
               com[RCVR_PORT].stop_bits = 1;
            }
            else if(ACU_360) {
               com[RCVR_PORT].baud_rate = 115200;
               com[RCVR_PORT].data_bits = 8;
               com[RCVR_PORT].parity = ODD_PAR;
               com[RCVR_PORT].stop_bits = 1;
            }
            else {
               com[RCVR_PORT].baud_rate = 9600;
               com[RCVR_PORT].data_bits = 8;
               com[RCVR_PORT].parity = ODD_PAR;
               com[RCVR_PORT].stop_bits = 1;
            }
            if(ACU_360) default_gnss = (GPS | GLONASS | BEIDOU);
            else if(ACU_GG) default_gnss = (GPS | GLONASS);
         }

         config_rcvr_plots();
      }
      else if(SV6_FAMILY || ACE3) {
         unit_file_name = "sv6";
         pps_enabled = 1;
         if(detect_rcvr_type) ;
         else if(set_baud == 0) ;
         else if(com[RCVR_PORT].user_set_baud == 0) {
            com[RCVR_PORT].baud_rate = 9600;
            com[RCVR_PORT].data_bits = 8;
            com[RCVR_PORT].parity = ODD_PAR;
            com[RCVR_PORT].stop_bits = 1;
         }

         config_lla_plots(KEEP_LLA_ON_HOLD, ENABLE_LLA_SHOW);
         config_rcvr_plots();
         config_utc_mode(0);
      }
      else if(res_t) {
         unit_file_name = "res_t";
         if(detect_rcvr_type) ;
         else if(set_baud == 0) ;
         else if(com[RCVR_PORT].user_set_baud == 0) {
            com[RCVR_PORT].baud_rate = 9600;
            com[RCVR_PORT].data_bits = 8;
            com[RCVR_PORT].parity = ODD_PAR;
            com[RCVR_PORT].stop_bits = 1;
         }
      }
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
      has_id_info = 1;
      config_lla_plots(KEEP_LLA_ON_HOLD, DISABLE_LLA_SHOW);

      min_sig_db = 16;      // low sig level threshold for sig level map
      sig_level_step = 2;   // sig level map signal steo size

      have_kalman = 0;
      default_gnss = (GPS | GLONASS | GALILEO | SBAS | QZSS);
      config_rcvr_plots();
      pkt_start1 = 0xB5;
      pkt_start2 = 0x62;
   }
   else if(rcvr_type == UCCM_RCVR) {
      unit_file_name = "uccm";
      has_id_info = 1;
      config_lla_plots(KEEP_LLA_ON_HOLD, DISABLE_LLA_SHOW);

      if(detect_rcvr_type) ;
      else if(set_baud == 0) ;
      else if(com[RCVR_PORT].user_set_baud == 0) {
         com[RCVR_PORT].baud_rate = 57600;
         com[RCVR_PORT].data_bits = 8;
         com[RCVR_PORT].parity = NO_PAR;
         com[RCVR_PORT].stop_bits = 1;
      }

if(user_set_adev_period == 0) adev_period = 2.0;       // aaaapppp
if(user_set_pps_period == 0)  pps_adev_period = 2.0;
if(user_set_osc_period == 0)  osc_adev_period = 2.0;
if(user_set_chc_period == 0)  chc_adev_period = 2.0;
if(user_set_chd_period == 0)  chd_adev_period = 2.0;
save_adev_period();

      Sleep(500);
      scpi_init('u');
      saw_gpsdo = 20;

      config_rcvr_plots();

      have_kalman = 0;

      config_utc_mode(0);

      rcvr_mode = RCVR_MODE_UNKNOWN;
      pkt_end1 = 0x0D;
      pkt_end2 = 0x0A;
   }
   else if(rcvr_type == VENUS_RCVR) {
      unit_file_name = "venus";
      has_id_info = 1;
      if(detect_rcvr_type) ;
      else if(set_baud == 0) ;
      else if(com[RCVR_PORT].user_set_baud == 0) {
         if(lte_lite) com[RCVR_PORT].baud_rate = 38400;
         else         com[RCVR_PORT].baud_rate = 115200;
         com[RCVR_PORT].data_bits = 8;
         com[RCVR_PORT].parity = NO_PAR;
         com[RCVR_PORT].stop_bits = 1;
      }
      default_gnss = (GPS | BEIDOU | SBAS | QZSS);

      if(lte_lite) config_lla_plots(KEEP_LLA_ON_HOLD, DISABLE_LLA_SHOW);
      else         config_lla_plots(KEEP_LLA_ON_HOLD, ENABLE_LLA_SHOW);
      config_rcvr_plots();

      have_kalman = 0;
      pps1_freq = 1;

      if(lte_lite && (have_sawtooth == 0)) config_utc_mode(1);
      else config_utc_mode(0);

      if(lte_lite) {
         ppb_string = " ns";
         ppt_string = " ns";
      }

      pkt_start1 = 0xA0;
      pkt_start2 = 0xA1;
      pkt_end1 = 0x0D;
      pkt_end2 = 0x0A;
   }
   else if(rcvr_type == X72_RCVR) {
      unit_file_name = "sym";
      has_id_info = 1;
      if(detect_rcvr_type) ;
      else if(set_baud == 0) ;
      else if(com[RCVR_PORT].user_set_baud == 0) {
         com[RCVR_PORT].baud_rate = 57600;
         com[RCVR_PORT].data_bits = 8;
         com[RCVR_PORT].parity = NO_PAR;
         com[RCVR_PORT].stop_bits = 1;
      }
      config_lla_plots(KEEP_LLA_ON_HOLD, DISABLE_LLA_SHOW);
      plot_azel = 0;
      plot_signals = 0;
      shared_plot = 0;

      have_kalman = 0;
      have_temperature = 5;
      pps_enabled = 1;

      config_utc_mode(1);
      config_rcvr_plots();

      pkt_end1 = 0x0D;          // used to pretty-print the data stream log file
      pkt_end1 = 0x0A;
   }
   else if(rcvr_type == Z12_RCVR) {
      unit_file_name = "ashtech";
      have_timing_mode = 1;
      config_lla_plots(KEEP_LLA_ON_HOLD, ENABLE_LLA_SHOW);

      config_rcvr_plots();

      have_kalman = 0;

      config_utc_mode(1);
have_utc_ofs = 0;

      pkt_end1 = 0x0D;
      pkt_end2 = 0x0A;
   }
   else if(rcvr_type == ZODIAC_RCVR) {
      unit_file_name = "jupiter";
      has_id_info = 1;
      config_lla_plots(KEEP_LLA_ON_HOLD, DISABLE_LLA_SHOW);

      have_kalman = 0;
      have_temperature = 0;
      config_rcvr_plots();
      pkt_start1 = 0xFF;
      pkt_start2 = 0x81;
   }
   else if(rcvr_type == ZYFER_RCVR) {
      unit_file_name = "zyfer";
      has_id_info = 1;

      if(detect_rcvr_type) ;
      else if(set_baud == 0) ;
      else if(com[RCVR_PORT].user_set_baud == 0) {
         com[RCVR_PORT].baud_rate = 19200;  // use 19200:8:N:1
         com[RCVR_PORT].data_bits = 8;
         com[RCVR_PORT].parity = NO_PAR;
         com[RCVR_PORT].stop_bits = 1;
      }

      have_timing_mode = 1;
      config_lla_plots(KEEP_LLA_ON_HOLD, DISABLE_LLA_SHOW);

      config_rcvr_plots();

      have_kalman = 0;

      config_utc_mode(0);

      pkt_end1 = 0x0D;
      pkt_end2 = 0x0A;
   }

   config_msg_ofs();

   if(rcvr_type == LUXOR_RCVR) {
      init_com(RCVR_PORT, 12);
      Sleep(200);
      reset_luxor_wdt(0x01);
      set_luxor_time();
      reset_luxor_wdt(0x01);
   }
   else if(detect_rcvr_type) ;
   else if(set_baud == 0) ;
   else if(com[RCVR_PORT].user_set_baud == 0) {
      init_com(RCVR_PORT,13);
   }


   if((rcvr_type == TSIP_RCVR) && (tsip_type != STARLOC_TYPE)) ;  // DATUM GPSDO has no temp sensor
   else if(rcvr_type == RFTG_RCVR) ;     // NEW_RCVR
   else if(rcvr_type == THERMO_RCVR) ;
   else if(rcvr_type == TRUE_RCVR) ;
   else if(rcvr_type == ZYFER_RCVR) ;
   else if(user_set_temp_plot) ;
   else if(have_temperature) plot[TEMP].show_plot = 1;
   else plot[TEMP].show_plot = 0;

   if(unit_file_name && (user_set_log == 0)) {
      if(log_file == 0) {
         if(rcvr_type == CS_RCVR) {  // HP5071A needs .xml logs
            sprintf(log_name, "%s.xml", unit_file_name);
            if(user_dump_fmt == 0) dump_xml = 'x';
         }
         else sprintf(log_name, "%s.log", unit_file_name);
      }
      if(raw_file == 0) sprintf(raw_name, "%s.raw", unit_file_name);
      if(prn_file == 0) sprintf(prn_name, "%s.prn", unit_file_name);
   }

   reset_com_timer(RCVR_PORT);
   return;
}

//
//
//   Command line processor
//
//


int config_input_device(unsigned port, char *arg)
{
int i;
char *comma;

   // __linux__  __MACH__  __FreeBSD__   ... WINDOWS can't do this

   // setup to use a com device using a com device name.
   // Returns 0 if successful,  non-zero if error.
   // Serial port baud rate info can follow the device number/name - separated by a comma

   #ifdef WINDOWS
      return 4;     // Windows does not support named input devices
   #endif

   if(arg == 0) return 1;
   if(port >= NUM_COM_PORTS) return 2;

   comma = strchr(arg, ',');
   if(comma) {     // baud rate info follows the '='
      *comma = 0;
//    port = (unsigned) set_serial_params(port, arg, 0);
      port = (unsigned) set_serial_params(port, arg, 1);
      if(port >= NUM_COM_PORTS) return 3;
   }

   com[port].com_port = 0;
   com[port].user_disabled_com = 0;
   com[port].usb_port = USE_IDEV_NAME;
   com[port].process_com = 1;

   if(arg[0]) {  // device name given
      strncpy(com[port].com_dev, &arg[0], sizeof(com[port].com_dev));

      for(i=strlen(com[port].com_dev)-1; i>=0; i++) {  // trim trailing scuzz
         if     (com[port].com_dev[i] == 0x0A) com[port].com_dev[i] = 0;
         else if(com[port].com_dev[i] == 0x0D) com[port].com_dev[i] = 0;
         else if(com[port].com_dev[i] == ' ')  com[port].com_dev[i] = 0;
         else if(com[port].com_dev[i] == '\t') com[port].com_dev[i] = 0;
         else if(com[port].com_dev[i]) break;
      }
   }
   else if(com[port].com_dev[0]) {   // use previously set device name
   }
   else {  // use default device name
      strncpy(com[port].com_dev, "/dev/heather", sizeof(com[port].com_dev));
   }

   if(com[port].com_dev[0] == 0) return 3;
   com[port].com_dev[sizeof(com[port].com_dev)-1] = 0;

   if(com[port].com_running) {
      com[port].process_com = 1;
      com[port].user_disabled_com = 0;
      init_com(port, 14);
   }

   com[port].last_com_time = this_msec - com[port].com_timeout;

   return 0;
}


int config_tcpip_device(unsigned port, char *arg)
{
int i;

   // setup to use a com device using a com device name.
   // Returns 0 if successful,  non-zero if error.

   if(arg == 0) return 1;
   if(port >= NUM_COM_PORTS) return 2;

   if(arg[0]) {  // TCP/IP addr given
      com[port].com_port = com[port].usb_port = 0;
      com[port].user_disabled_com = 0;
      com[port].process_com = 1;
      strncpy(com[port].IP_addr, &arg[0], sizeof(com[port].IP_addr)-1);
      com[port].IP_addr[sizeof(com[port].IP_addr)-1] = 0;

      for(i=strlen(com[port].IP_addr)-1; i>=0; i++) {  // trim trailing scuzz
         if     (com[port].IP_addr[i] == 0x0A) com[port].IP_addr[i] = 0;
         else if(com[port].IP_addr[i] == 0x0D) com[port].IP_addr[i] = 0;
         else if(com[port].IP_addr[i] == ' ')  com[port].IP_addr[i] = 0;
         else if(com[port].IP_addr[i] == '\t') com[port].IP_addr[i] = 0;
         else if(com[port].IP_addr[i]) break;
      }
   }
   else if(com[port].IP_addr[0]) {  // use old IP address
   }
   else {  // !!!!!! use a default IP addr
   }

   com[port].IP_addr[sizeof(com[port].IP_addr)-1] = 0;
   if(com[port].IP_addr[0] == 0) return 3;
   com[port].user_set_ip = 2;

   if(com[port].com_running && com[port].IP_addr[0]) {  // use TCPIP port
      com[port].com_port = com[port].usb_port = 0;
      com[port].user_disabled_com = 0;
      com[port].process_com = 1;
      init_com(port, 15);
   }

   com[port].last_com_time = this_msec - com[port].com_timeout;

   return 0;
}


int config_port(unsigned port, char *arg)
{
char c, d;
char out[512+1];
char *comma;

   // setup to use a com device.  Returns 0 if successful,  non-zero if error.
   //
   // If arg contains a ':', it's a TCP/IP address.
   // If arg starts with a '0' disable port processing.
   // If arg starts with a digit, it's a serial/USB port.
   // Otherwise, it's a device name  __linux__  __MACH__   __FreeBSD__
   // Serial port baud rate info can follow the device number/name - separated by a comma

   if(arg == 0) return 1;
   if(port >= NUM_COM_PORTS) return 2;
   if(port == TICC_PORT) no_adev_flag = 0;

   strcpy(out, arg);

   c = d = 0;
   c = out[0];
   if(c) d = out[1];

   comma = strchr(out, ',');
   if(comma) {
      *comma = 0;
      port = (unsigned) set_serial_params(port, comma+1, 0);
////  port = (unsigned) set_serial_params(port, comma+1, 1);
      if(port >= NUM_COM_PORTS) return 3;
   }

   if((port == FAN_PORT) && (com[FAN_PORT].user_set_baud == 0)) {
      com[FAN_PORT].baud_rate = 115200;
      com[FAN_PORT].data_bits = 8;
      com[FAN_PORT].parity = NO_PAR;
      com[FAN_PORT].stop_bits = 1;
      fan_port = FAN_PORT;
   }
   else if((port == TRACK_PORT) && (com[TRACK_PORT].user_set_baud == 0)) {
      com[THERMO_PORT].baud_rate = 9600;
      com[THERMO_PORT].data_bits = 8;
      com[THERMO_PORT].parity = NO_PAR;
      com[THERMO_PORT].stop_bits = 1;
      fan_port = THERMO_PORT;
   }
   else if((port == THERMO_PORT) && (com[THERMO_PORT].user_set_baud == 0)) {
      com[THERMO_PORT].baud_rate = 115200;
      com[THERMO_PORT].data_bits = 8;
      com[THERMO_PORT].parity = NO_PAR;
      com[THERMO_PORT].stop_bits = 1;
      fan_port = THERMO_PORT;
   }
   else if((port == TICC_PORT) && (com[TICC_PORT].user_set_baud == 0)) {
      if(ticc_type == PICPET_TICC) com[TICC_PORT].baud_rate = 19200;
      else if(ticc_type == TAPR_TICC) com[TICC_PORT].baud_rate = 115200;
      else if(ticc_type == LARS_TICC) com[TICC_PORT].baud_rate = 9600;
      else com[TICC_PORT].baud_rate = 115200;
      com[TICC_PORT].data_bits = 8;
      com[TICC_PORT].parity = NO_PAR;
      com[TICC_PORT].stop_bits = 1;
   }
   else if((port == TICC_ECHO_PORT) && (com[TICC_ECHO_PORT].user_set_baud == 0)) {
      com[TICC_ECHO_PORT].baud_rate = 115200;
      com[TICC_ECHO_PORT].data_bits = 8;
      com[TICC_ECHO_PORT].parity = NO_PAR;
      com[TICC_ECHO_PORT].stop_bits = 1;
   }

   if(port == THERMO_PORT) {
      config_enviro_plots(3);
   }

   if(strchr(out, ':')) {  // a ':' in the string means ars is a TCP/IP addr
//sprintf(debug_text, "otcpp%s", out);
      return config_tcpip_device(port, out);
   }
   else if(isdigit(c) && strchr(out, '.')) { // a dotted IP address
//sprintf(debug_text, "otcpd%s", out);
      return config_tcpip_device(port, out);
   }
   else if(c == '0') {  // toggle com port processing flag
      com[port].process_com = toggle_option(com[port].process_com, d); //lfs
com[port].process_com = 0;//lfs
com[port].port_used = (-1);//lfs
if(port == THERMO_PORT) {
   config_enviro_plots(10);
   BEEP(316);
}

      if(com[port].process_com == 0) {
         com[port].com_port = com[port].usb_port = 0;
         com[port].user_disabled_com = 1;
      }
      com[port].last_com_time = this_msec - com[port].com_timeout;
      return 0;
   }
   else if(isdigit(c)) {  // arg is a com/usb port number
//sprintf(debug_text, "oport%s", out);
      if(strstr(out, "P")) {
         lpt_port = atoi(&out[0]);  // parallel port used for temp control
         return 0;
      }
      else if(strstr(out, "T")) {
         lpt_port = atoi(&out[0]);  // parallel port used for temp control
         return 0;
      }
#ifdef WINDOWS
#else // __linux__  __MACH__  __FreeBSD__
      else if(strchr(out, 'U') || strchr(out, 'u')) {  // linux ttyUSB#
         com[port].usb_port = atoi(&out[0]);
         com[port].com_port = 0;
         com[port].user_disabled_com = 0;
      }
      else if(strchr(out, 'A') || strchr(out, 'a')) {  // linux ttyACM#
         sprintf(out, "/dev/ttyACM%d", atoi(&out[0])-1);
         config_input_device(port, out);
      }
#endif
      else {  // hardware serial port or Windows com port
         com[port].com_port = atoi(&out[0]);
         com[port].usb_port = 0;
         com[port].user_disabled_com = 0;
      }

      if(com[port].com_running) {
         com[port].process_com = 1;
         init_com(port, 16);
      }

      com[port].last_com_time = this_msec - com[port].com_timeout;
      return 0;
   }
   else {   // open port by name, not device number
      // __linux__  __MACH__   __FreeBSD__
//sprintf(debug_text, "opname:%s", out);
      return config_input_device(port, out);  //ppppp
   }

   return 4;  // should never happen
}


void set_gpsd_mode()
{
   // set the receiver type to GPSD and config the ports

   rcvr_type = GPSD_RCVR;
   com[RCVR_PORT].com_port = com[RCVR_PORT].usb_port = 0;
   if(com[RCVR_PORT].user_set_ip == 0) {
      strcpy(com[RCVR_PORT].IP_addr, "localhost:2947");
   }
   com[RCVR_PORT].user_disabled_com = 0;
   com[RCVR_PORT].process_com = 1;
   init_com(RCVR_PORT, 17);
}

char opt_cmd[512+1];
char opt_arg[512+1];

int parse_option(char *arg)
{
unsigned i;
char c;

   // break up a command line option into the command and argument fields.
   // connvert command to lower case.  Returns the char that separated the
   // command from the value, returns 0 if no value.

   for(i=0; i<sizeof(opt_cmd); i++) opt_cmd[i] = 0;
   for(i=0; i<sizeof(opt_arg); i++) opt_arg[i] = 0;
   if(arg == 0) return 0;

   for(i=0; i<sizeof(opt_cmd)-1; i++) {
      c = arg[i+1];
      if(c == 0) break;
      else if((c == '=') || (c == ':')) {  // : or = separates option from arg value
         strcpy(opt_arg, &arg[i+1+1]);
         strlwr(opt_cmd);
         return c;
      }
      else opt_cmd[i] = c;
   }

   strlwr(opt_cmd);
   return 0;
sprintf(plot_title, "cmd:(%s)  arg:(%s)", opt_cmd, opt_arg);
}


void save_adev_period()
{
   old_adev_period     = adev_period;
   old_pps_adev_period = pps_adev_period;
   old_osc_adev_period = osc_adev_period;
   old_chc_adev_period = chc_adev_period;
   old_chd_adev_period = chd_adev_period;
}

void restore_adev_period()
{
   adev_period     = old_adev_period;
   pps_adev_period = old_pps_adev_period;
   osc_adev_period = old_osc_adev_period;
   chc_adev_period = old_chc_adev_period;
   chd_adev_period = old_chd_adev_period;
}

int cmd_a_to_o(char *arg) 
{
char c, d, e, f;
unsigned port;
double val;
DATA_SIZE scale_factor;
char *s;
int i, r, g, b;
unsigned j;
char sep;
long seek_addr;

//   parse_option(arg);
//   c = opt_cmd[0];
//   d = opt_cmd[1];
//   e = opt_cmd[2];
//   f = opt_cmd[3];

   d = e = f = 0;
   c = tolower(arg[1]);
   if(c) d = tolower(arg[2]);
   if(d) e = tolower(arg[3]);
   if(e) f = tolower(arg[4]);

#ifdef ADEV_STUFF
   if((c == 'a') && (d == 'e')) {  // /ae - toggle adev error bars
      show_error_bars = toggle_option(show_error_bars, e);
   }
   else if((c == 'a') && (d == 'h')) {  // /ah - set antenna height/ew/ns displacement in meters (for RINEX output)
      if(((e == '=') || (e == ':')) && arg[4]) {
         sscanf(&arg[4], "%lf%c%lf%c%lf", &antenna_height,&c,&antenna_ew,&c,&antenna_ns);
      }
      else {
         antenna_height = 1.0;
         antenna_ew = 0.0;
         antenna_ns = 0.0;
      }
   }
   else if((c == 'a') && (d == 'j')) {  // /aj - calculate PPS adevs from message jitter
      jitter_adev = toggle_option(jitter_adev, e);
   }
   else if((c == 'a') && (d == 'n')) {  // /an - set antenna number (for RINEX output)
      if(((e == '=') || (e == ':')) && arg[4]) {
         strcpy(out, &arg[4]);
         out[20] = 0;
         for(j=0; j<strlen(out); j++) {  // replace '_' with blanks
            if(out[j] == '_') out[j] = ' ';
         }
         strcpy(antenna_number, out);
      }
      else strcpy(antenna_number, DEFAULT_NAME);
   }
   else if((c == 'a') && (d == 's')) {  // /as - set adev bin scaling sequence
      if(((e == '=') || (e == ':')) && arg[4]) {
         bin_scale = atoi(&arg[4]);
      }
      else bin_scale = 5;  // 1-2-5

      if(keyboard_cmd) {
         recalc_adev_info();
         force_adev_redraw(114);
      }
   }
   else if((c == 'a') && (d == 'k')) {  // /ak - set marker name (for RINEX output)
      if(((e == '=') || (e == ':')) && arg[4]) {
         strcpy(out, &arg[4]);
         out[20] = 0;
         for(j=0; j<strlen(out); j++) {  // replace '_' with blanks
            if(out[j] == '_') out[j] = ' ';
         }
         strcpy(marker_name, out);
      }
      else strcpy(marker_name, DEFAULT_NAME);
   }
   else if((c == 'a') && (d == 't')) {  // /at - set antenna type (for RINEX output)
      if(((e == '=') || (e == ':')) && arg[4]) {
         strcpy(out, &arg[4]);
         out[20] = 0;
         for(j=0; j<strlen(out); j++) {  // replace '_' with blanks
            if(out[j] == '_') out[j] = ' ';
         }
         strcpy(antenna_type, out);
      }
      else strcpy(antenna_type, DEFAULT_NAME);
   }
   else if((c == 'a') && (d == 'v')) {  // /ak - set marker number (for RINEX output)
      if(((e == '=') || (e == ':')) && arg[4]) {
         strcpy(out, &arg[4]);
         out[20] = 0;
         for(j=0; j<strlen(out); j++) {  // replace '_' with blanks
            if(out[j] == '_') out[j] = ' ';
         }
         strcpy(marker_number, out);
      }
      else strcpy(marker_number, DEFAULT_NAME);
   }
   else if((c == 'a') && (d == 'x') && (e == 'a')) {  // /axa - hide ADEV plots
      adev_display_mask ^= DISPLAY_ADEV;
   }
   else if((c == 'a') && (d == 'x') && (e == 'c')) {  // /axc - hide CHC xDEV plots
      adev_display_mask ^= DISPLAY_CHC;
   }
   else if((c == 'a') && (d == 'x') && (e == 'd')) {  // /axd - hide CHD xDEV plots
      adev_display_mask ^= DISPLAY_CHD;
   }
   else if((c == 'a') && (d == 'x') && (e == 'h')) {  // /axh - hide HDEV plots
      adev_display_mask ^= DISPLAY_HDEV;
   }
   else if((c == 'a') && (d == 'x') && (e == 'm')) {  // /axm - hide MDEV plots
      adev_display_mask ^= DISPLAY_MDEV;
   }
   else if((c == 'a') && (d == 'x') && (e == 'o')) {  // /axo - hide OSC/CHB xDEV plots
      adev_display_mask ^= DISPLAY_CHB;
   }
   else if((c == 'a') && (d == 'x') && (e == 'p')) {  // /axp - hide PPS/CHA xDEV plots
      adev_display_mask ^= DISPLAY_CHA;
   }
   else if((c == 'a') && (d == 'x') && (e == 't')) {  // /axt - hide TDEV plots
      adev_display_mask ^= DISPLAY_TDEV;
   }
   else if((c == 'a') && (d == 'x')) {  // /ax - unhide all adev plots
      adev_display_mask = 0xFFFF;
   }
   else if(c == 'a') {   // /a - set adev queue size
      set_not_safe();
      if(((d == '=') || (d == ':')) && arg[3]) {
         sscanf(&arg[3], "%ld", &adev_q_size);                           
      }
      else {     // default adev queue sizes
         adev_q_size = (12L*3600L*10L);      // good for 100000 tau
      }

      if(keyboard_cmd) {
         free_mtie();
//       dont_reset_queues = 0;
//       new_queue(RESET_ADEV_Q, 12);
         free_adev_queues();
      }


      if(adev_q_size <= 0) {  // aaaapppp
          // disable adevs... if you re-enable them you will need to use
          // the /j command to reset the adev period to a non-zero value!!!!!
          if(adev_period || pps_adev_period || osc_adev_period || chc_adev_period || chd_adev_period) {
             save_adev_period();
          }

          adev_q_size = 1L;
          adev_period = 0.0;
          pps_adev_period = adev_period;
          osc_adev_period = adev_period;
          chc_adev_period = adev_period;
          chd_adev_period = adev_period;
      }
      else {
         restore_adev_period();
      }

      if(adev_q_size >= 100000000L) adev_q_size = 100000000L;
      user_set_adev_size = 1;

      disable_adevs();

      if(keyboard_cmd) {   // we are resizing the queue
         alloc_adev();
         if(mtie_allocated) alloc_mtie(100);
         reload_adev_queue(0);  // !!!!! should we do this for all devices or only TICC?
         init_screen(7077);     // force screen re-config
//       redraw_screen();
//       force_adev_redraw(115);
//       redraw_screen();
      }
   }
   else if(c == 'b') {    // set daylight savings time mode
#else 
   if(c == 'b') {         // /b - set daylight savings time mode 
#endif
      if(d == 'm') { // /bm - set monitor port number and ascii/hex mode
         if((e == '=') || (e == ':')) {
            monitor_mode = 1;
            strcpy(edit_buffer, &arg[4]);
            set_monitor_params();
         }
         else {
            monitor_mode = toggle_option(monitor_mode, e);
///         monitor_port = RCVR_PORT;
         }
      }
      else if((d == 'r') && (e == 'f')) {  // /brf - force receiver baud rate (for supported devices)
         if(((f == '=') || (f == ':')) && arg[5]) {
            i = atoi(&arg[5]);
            force_rcvr_baud(i);
         }
         else {
            force_rcvr_baud(115200);
         }
      }
      else if((d == 'r') && (e == 'r')) {  // /brr - set receiver baud rate (for supported devices)
         if(((f == '=') || (f == ':')) && arg[5]) {
            i = set_serial_params(RCVR_PORT, &arg[5], 2);
         }
      }
      else if(d == 'r') {  // /br - set baud rate
         if(((e == '=') || (e == ':')) && arg[4]) {
            set_serial_params(RCVR_PORT, &arg[4], 1);
         }
         else {
            com[RCVR_PORT].user_set_baud = 1;  //ppppp
            com[RCVR_PORT].baud_rate = 57600;
         }
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
         if((e == '=') || (e == ':')) {
            enable_terminal = 1;
            strcpy(edit_buffer, &arg[4]);
            set_terminal_params();
         }
         else {
            enable_terminal = toggle_option(enable_terminal, e);
//          term_port = RCVR_PORT;
         }
      }
      else if(((d == '=') || (d == ':')) && e) {  //  /B# or /B=#  or /B=nth,start_day,month,nth,end_day,month,hour
         if(strstr(&arg[3], ",")) {  // custom daylight savings time settings
            strncpy(custom_dst, &arg[3], sizeof(custom_dst));
            custom_dst[sizeof(custom_dst)-1] = 0;
            dst_area = CUSTOM_DST;
            user_set_dst = 1;
            set_not_safe();
            if(keyboard_cmd) calc_dst_times(this_year, dst_list[dst_area]);
         }
         else if((e >= '0') && (e <= ('0'+DST_AREAS))) {  // standard dst area set
            dst_area = e - '0';
            user_set_dst = 1;
            set_not_safe();
            if(keyboard_cmd) calc_dst_times(this_year, dst_list[dst_area]);
         }
         else return c;
      }
      else if((d >= '0') && (d <= ('0'+DST_AREAS))) {  // /b0 .. /b5 - standard dst area set
         dst_area = d - '0';
         user_set_dst = 1;
         set_not_safe();
         if(keyboard_cmd) calc_dst_times(this_year, dst_list[dst_area]);
      }
      else return c;
   }
   else if(c == 'c') {    // /c - set cable delay
      if(luxor && (d == 'g')) {       // cg - use green color channel for lux readings
         alt_lux1 = 0.0F;
         if(((e == '=') || (e == ':')) && arg[4]) {
            alt_lux1 = (DATA_SIZE) atof(&arg[4]);
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
         if(((e == '=') || (e == ':')) && arg[4]) {
            cct_cal = (DATA_SIZE) atof(&arg[4]);
            if(cct_cal <= 0.0) cct_cal = 1.0;
         }
         else if(e == '1') {
            if(((f == '=') || (f == ':')) && arg[5]) {
               cct1_cal = (DATA_SIZE) atof(&arg[5]);
               if(cct1_cal <= 0.0) cct1_cal = 1.0;
            }
            else cct_type = 1;
         }
         else if(e == '2') {
            if(((f == '=') || (f == ':')) && arg[5]) {
               cct2_cal = (DATA_SIZE) atof(&arg[5]);
               if(cct2_cal <= 0.0) cct2_cal = 1.0;
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
      else if(d == 'e') {   // /ce - toggle com port error recovery
         for(i=0; i<NUM_COM_PORTS; i++) {
            com[i].com_recover = toggle_option(com[i].com_recover, e); //ppppp
         }
         // !!!!! we need to support toggling com recovery on individual ports
      }
      else if(d == 'm') {   // /cm - change color map entry
         if(((e == '=') || (e == ':')) && arg[4]) {
            i = 15;
            r = g = b = (255);
            sscanf(&arg[4], "%d%c%d%c%d%c%d", &i,&sep, &r,&sep, &g,&sep, &b);
            edit_cmap(i, r,g,b);
         }
      }
      else if(d == 'o') {   // /co - add clock offset to RT17 time stamps
         add_clk_ofs = toggle_option(add_clk_ofs, e);
      }
      else if(d == 't') {   // /ct - set user RCVR_PORT com timeout value
         if(((e == '=') || (e == ':')) && arg[4]) {  // set all port timeouts
            user_com_timeout = atof(&arg[4]);
            if(user_com_timeout < MIN_TIMEOUT) user_com_timeout = MIN_TIMEOUT;
            for(i=0; i<NUM_COM_PORTS; i++) {
               com[i].com_timeout = user_com_timeout;
               com[i].user_timeout = user_com_timeout;
            }
         }
         else if(((f == '=') || (f == ':')) && arg[5]) {  // set specifc port timeout
            user_com_timeout = atof(&arg[5]);
            if(user_com_timeout < MIN_TIMEOUT) user_com_timeout = MIN_TIMEOUT;
            i = (-1);
            e = toupper(e);
            if((e >= '0') && (e <= '9')) i = e - '0';
            else if(e == 'D') i = DAC_PORT;       // external DAC port
            else if(e == 'E') i = ECHO_PORT;      // echo port
            else if(e == 'F') i = FAN_PORT;       // temperature control device port
            else if(e == 'I') i = TICC_PORT;      // TICC port
            else if(e == 'K') i = NMEA_PORT;      // echo in NMEA format port
            else if(e == 'N') i = THERMO_PORT;    // environmental sensor (thermometer) port
            else if(e == 'R') i = RCVR_PORT;      // receiver data port
            else if(e == 'T') i = TRACK_PORT;     // moon/sun/sat position port
            if((i >= 0) && (i < NUM_COM_PORTS)) {
               com[i].com_timeout = user_com_timeout;
               com[i].user_timeout = user_com_timeout;
            }
         }
      }
      else {  // /c - set cable delay
         ++user_set_delay;
         set_not_safe();
         if(((d == '=') || (d == ':')) && arg[3]) {
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
            if(arg[4]) {
               strcpy(debug_name, &arg[4]);
               open_debug_file(debug_name);
            }
         }
         if(debug_file == 0) need_debug_log = 1;
         return 0;
      }
      else if(d == 'm') alt_calendar = MJD;
      else if(d == 'n') alt_calendar = INDIAN;
      else if(d == 'o') show_beep_reason =  toggle_option(show_beep_reason, e);  // /do - show beep reason
      else if(d == 'p') alt_calendar = PERSIAN;
      else if(d == 'q') {  // /dq - open TICC data log file
         if(ticc_file) {
            fclose(ticc_file);
            ticc_file = 0;
         }

         if((e == '=') || (e == ':')) {
            if(arg[4]) {
               strcpy(ticc_name, &arg[4]);
               ticc_file = fopen(ticc_name, "wb");
            }
         }
         if(ticc_file == 0) need_ticc_file = 1;
// aaattt         else log_stream |= LOG_RAW_STREAM;
         return 0;
      }
      else if(d == 'r') {  // /dr - open receiver data capture file
         if(raw_file) {
            fclose(raw_file);
            raw_file = 0;
         }

         if((e == '=') || (e == ':')) { 
            if(arg[4]) {
               strcpy(raw_name, &arg[4]);
               s = strchr(raw_name, FLUSH_CHAR);
               if(s) {
                  *s = 0;
                  raw_flush_mode = 1;
               }
               else raw_flush_mode = 0;
               raw_file = fopen(raw_name, "wb");
            }
         }
         if(raw_file == 0) need_raw_file = 1;
         else log_stream |= LOG_RAW_STREAM;
         return 0;
      }
      else if(d == 's') alt_calendar = ISO;
      else if(d == 't') alt_calendar = TZOLKIN;     // Mayan 260 day cycle
      else if(d == 'u') take_a_dump ^= 1;
      else if(d == 'v') alt_calendar = BOLIVIAN;    // Bolivian 13 month calendar
      else if(d == 'w') alt_calendar = ISO_WEEK;    // ISO week format
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

            calc_dst_times(this_year, dst_list[dst_area]);
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
      if(d == 'd') {  // /ed - open DAC port
         if(((e == '=') || (e == ':')) && arg[4]) {
            if(keyboard_cmd) kill_com(DAC_PORT,3001);
            config_port(DAC_PORT, &arg[4]);
         }
         else if(1) { // default to COM2 / USB1
            if(keyboard_cmd) kill_com(DAC_PORT,3002);
            config_port(DAC_PORT, "2u");
         }
      }
      else if(d == 'e') {  // /ee command - open receiver echo port
         com[ECHO_PORT].baud_rate = com[RCVR_PORT].baud_rate;  // default to echo at receiver rate
         com[ECHO_PORT].data_bits = com[RCVR_PORT].data_bits;
         com[ECHO_PORT].stop_bits = com[RCVR_PORT].stop_bits;
         com[ECHO_PORT].parity = com[RCVR_PORT].parity;
         if(((e == '=') || (e == ':')) && arg[4]) {
            if(keyboard_cmd) kill_com(ECHO_PORT,3003);
            config_port(ECHO_PORT, &arg[4]);
         }
         else { // default to COM2 / USB1
            if(keyboard_cmd) kill_com(ECHO_PORT, 3004);
            config_port(ECHO_PORT, "2u");
         }
      }
      else if(d == 'f') {  // /ef command - open temperature control device port
         com[FAN_PORT].baud_rate = com[RCVR_PORT].baud_rate;  // default to echo at receiver rate
         com[FAN_PORT].data_bits = com[RCVR_PORT].data_bits;
         com[FAN_PORT].stop_bits = com[RCVR_PORT].stop_bits;
         com[FAN_PORT].parity = com[RCVR_PORT].parity;
         fan_port = FAN_PORT;
         if(((e == '=') || (e == ':')) && arg[4]) {
            if(keyboard_cmd) kill_com(FAN_PORT,3005);
            config_port(FAN_PORT, &arg[4]);
         }
         else { // default to COM2 / USB1
            if(keyboard_cmd) kill_com(FAN_PORT,3006);
            config_port(FAN_PORT, "2u");
         }
      }
      else if(d == 'i') {  // /ei - open TICC port
         if(((e == '=') || (e == ':')) && arg[4]) {
            if(keyboard_cmd) kill_com(TICC_PORT,3007);
            config_port(TICC_PORT, &arg[4]);
         }
         else { // default to COM2 / USB1
            if(keyboard_cmd) kill_com(TICC_PORT,3008);
            config_port(TICC_PORT, "2u");
         }
         if(ticc_port_open()) {  // monitoring a GPS on the RCVR_PORT and using a TICC on the TICC_PORT
            if(user_set_ticc_type == 0) ticc_type = TAPR_TICC;  // aaattt
         }
         else {
            if(user_set_ticc_type == 0) ticc_type = TAPR_TICC;
         }
         if(user_set_ticc_mode == 0) ticc_mode = DEFAULT_TICC_MODE;  // aaattt
      }
      else if(d == 'k') {  // /ek - open NMEA format echo port
         if(((e == '=') || (e == ':')) && arg[4]) {
            if(keyboard_cmd) kill_com(NMEA_PORT,3009);
            config_port(NMEA_PORT, &arg[4]);
         }
         else { // default to COM2 / USB1
            if(keyboard_cmd) kill_com(NMEA_PORT,3010);
            config_port(NMEA_PORT, "2u");
         }
      }
      else if(d == 't') {  // /et - open moon/sun/sat location tracking port
         if(((e == '=') || (e == ':')) && arg[4]) {
            if(keyboard_cmd) kill_com(TRACK_PORT,3109);
            config_port(TRACK_PORT, &arg[4]);
         }
         else { // default to COM2 / USB1
            if(keyboard_cmd) kill_com(TRACK_PORT,3110);
            config_port(TRACK_PORT, "2u");
         }
      }
      else if(d == 'n') {  // /en - open environmental sensor (thermometer) port
         if(((e == '=') || (e == ':')) && arg[4]) {
            if(keyboard_cmd) kill_com(THERMO_PORT,3013);
            config_port(THERMO_PORT, &arg[4]);
         }
         else if(1) { // default to COM2 / USB1
            if(keyboard_cmd) kill_com(THERMO_PORT,3014);
            config_port(THERMO_PORT, "2u");
         }
         config_enviro_plots(4);
         init_enviro(THERMO_PORT);
      }
      else if(d == 'r') {  // /er - open receiver port
         if(((e == '=') || (e == ':')) && arg[4]) {
            if(keyboard_cmd) kill_com(RCVR_PORT,3011);
            config_port(RCVR_PORT, &arg[4]);
         }
         else if(1) { // default to COM2 / USB1
            if(keyboard_cmd) kill_com(RCVR_PORT,3012);
            config_port(RCVR_PORT, "1u");
         }
      }
      else if(isdigit(d)) {  // /ep0 .. /ep9 - enable user port
         port = (d - '0');
         if(keyboard_cmd) kill_com(port,3015);
         if(((f == '=') || (f == ':')) && &arg[5]) {
            config_port(port, &arg[5]);
         }
         else {
            config_port(port, "");
         }
      }
      else {
         log_errors = toggle_option(log_errors, d);
      }
   }
   else if (c == 'f') { // /f - toggle filter 
      if(d) {  // toggle filter
         if     (d == 'a') user_alt = toggle_option(user_alt, e);
         else if(d == 'g') {
            dump_gpx = toggle_option(dump_gpx, e); // /fg - dump in GPX format
            user_dump_fmt = 'g';
            dump_xml = 0;
         }
         else if(d == 'i') { user_static = toggle_option(user_static, e); user_set_filters |= STATIC_FILTER; } // Mototola ionosphere
         else if(d == 'k') { user_kalman = toggle_option(user_kalman, e); user_set_filters |= KALMAN_FILTER; }
         else if(d == 'p') { user_pv = toggle_option(user_pv, e);         user_set_filters |= PV_FILTER; }
         else if(d == 's') { user_static = toggle_option(user_static, e); user_set_filters |= STATIC_FILTER; }
         else if(d == 't') { user_alt = toggle_option(user_alt, e);       user_set_filters |= ALT_FILTER; }       // Motorola troposphere
         else if(d == 'u') {  // /fu - full screen enable
            #ifdef WIN_VFX
               vfx_fullscreen = toggle_option(vfx_fullscreen, e); // allow full screen
            #else  //  USE_X11
               if(1 && keyboard_cmd) {  // piss
                  need_screen_init = 99999;
                  set_not_safe();
               }
               else go_full();  // do full screen
            #endif
user_set_full_screen = 1;  // piss
         }
         else if(d == 'd') {  // /fd = display filter
            disp_filter_type = 0;
            if((e == '=') || (e == ':')) { 
               if(arg[4]) {
                  filter_count = atoi(&arg[4]);
                  if(strchr(&arg[4], 'P')) disp_filter_type = 'P';  // peak filter
                  else if(strchr(&arg[4], 'p')) disp_filter_type = 'P';  // peak filter
               }
               else filter_count = 10;
            }
            else filter_count = 10;
            return 0;
         }
         else if(d == 'x') {
            dump_xml = toggle_option(dump_xml, e); // /fx - dump in XML format
            user_dump_fmt = 'x';
            dump_gpx = 0;
         }
         else return d;
      }
      else {   // /f - start up in full screen mode
         go_full();
      }
   }
   else if(c == 'g') {   // /g - toggle Graph enables
      if(d == 'a') {
         plot_adev_data = toggle_option(plot_adev_data, e); user_set_adev_plot = 1;
         if(plot_adev_data == 0) adev_decades_shown = 0;
      }
      else if(d == 'b') {
         plot_azel = AZEL_OK;
         shared_plot = toggle_option(shared_plot,e);
         if(plot_azel) update_azel = 1;
         if(keyboard_cmd) {
            init_screen(5123);
         }
      }
      else if(d == 'c') { plot_sat_count = toggle_option(plot_sat_count, e); }
      else if(d == 'd') { plot[DAC].show_plot = toggle_option(plot[DAC].show_plot, e); user_set_dac_plot = 1; }
      else if(d == 'e') { plot_skip_data = toggle_option(plot_skip_data, e); }
      else if(d == 'h') { plot_holdover_data = toggle_option(plot_holdover_data, e); }
      else if(d == 'i') {   // /gi
         if(rcvr_type == TICC_RCVR) { // /gi for TICC_RCVR shows MTIE
            ATYPE = A_MTIE;
            all_adevs = ALL_CHANS;
         }
         else {
            show_fixes = toggle_option(show_fixes, e);
            user_fix_set = show_fixes;
         }
      }
      else if(d == 'j') { plot_el_mask = toggle_option(plot_el_mask, e); }
      else if(d == 'k') {
         if     (e == 'h') { plot[HUMIDITY].show_plot = toggle_option(plot[HUMIDITY].show_plot, f);  }
         else if(e == 'p') { plot[PRESSURE].show_plot = toggle_option(plot[PRESSURE].show_plot, f);  }
         else if(e == 't') { plot[TEMP1].show_plot = toggle_option(plot[TEMP1].show_plot, f);  }
         else if(e == 'u') { plot[TEMP2].show_plot = toggle_option(plot[TEMP2].show_plot, f);  }
         else if(e == 'y') { plot[ELEVEN].show_plot = toggle_option(plot[ELEVEN].show_plot, f); }
         else if(e == 'x') { plot[TWELVE].show_plot = toggle_option(plot[TWELVE].show_plot, f); }
         else if(e == 'z') { plot[THIRTEEN].show_plot = toggle_option(plot[THIRTEEN].show_plot, f); }
         else if(e == 'g') { plot[FOURTEEN].show_plot = toggle_option(plot[FOURTEEN].show_plot, f); }
         else              plot_const_changes = toggle_option(plot_const_changes, e);
      }
      else if(d == 'l') { // /gl command - change location format
         if(e) {
            getting_plot = (-1);
            change_plot_param(e, 1);
            getting_plot = 0;
         }
         else {
            plot_loc = toggle_option(plot_loc, e);
            need_redraw = 7723;
         }
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
      else if(d == 'y') { plot_filters = toggle_option(plot_filters, e); user_set_dfilter = 1; }
//    else if(d == 'z') { plot_digital_clock = toggle_option(plot_digital_clock, e); user_set_bigtime = 1;  plot_watch = 0; user_set_clock_plot = 1; }
      else if(d == 'z') { plot_digital_clock = toggle_option(plot_digital_clock, e); user_set_bigtime = 1;  user_set_clock_plot = 1; }
      else if(d == '0') { plot[TEN].show_plot = toggle_option(plot[TEN].show_plot, e); }
      else if(d == '$') { plot[SAT_PLOT].show_plot = toggle_option(plot[SAT_PLOT].show_plot, e); user_set_sat_plot = 1; }
      else if((d >= '1') && (d <= '9')) {
         plot[ONE+d-'1'].show_plot = toggle_option(plot[ONE+d-'1'].show_plot, e);
      }
      else return c;
   }
   else if(c == 'h' && d == 'd' ) {
       // This is processed elsewhere since it needs to be done
       // before we get to this point.
       // see option_handle_hd in heather.cpp
   }
   else if(c == 'h') { // /h - show help info (same as /?, but more Linux friendly)
      if(keyboard_cmd == 0) {
         exit_flag = 1;
      }
      return c;
   }
   else if(c == 'i') {   
      if((d == '0') || (d == 'o')) {  // /io - do not process any received TSIP commands
         just_read = toggle_option(just_read, e);
      }
#ifdef WINDOWS
#else // __linux__  __MACH__  __FreeBSD__
      else if(d == 'd') {  // /id - set input device name
         if(arg[4] && ((e == '=') || (e == ':'))) {
            config_input_device(RCVR_PORT, &arg[4]);
         }
         else {
            config_input_device(RCVR_PORT, "");
         }
      }
#endif
#ifdef TCP_IP    // TCP: com_port 0 with process_com 1 means use TCP/IP connection rather than COM port
      else if(d == 'p') { // /ip - set TCPIP address
         if(arg[4] && ((e == '=') || (e == ':'))) {
            config_tcpip_device(RCVR_PORT, &arg[4]);
         }
         else {
            config_tcpip_device(RCVR_PORT, "");
         }
      }
#endif 
      else if(d == 'f') {   // /if?  - set nominal freq
         if(arg[4] && ((e == '=') || (e == ':'))) {
            val = atof(&arg[4]);
            if(val <= 0.0) val = NOMINAL_FREQ;
         }
         else if(arg[5] && ((f == '=') || (f == ':'))) {
            val = atof(&arg[5]);
            if(val <= 0.0) val = NOMINAL_FREQ;
         }
         else val = NOMINAL_FREQ;

         if     (e == 'a') { nominal_cha_freq = val; cha_phase_wrap_interval = (1.0 / nominal_cha_freq); }
         else if(e == 'b') { nominal_chb_freq = val; chb_phase_wrap_interval = (1.0 / nominal_chb_freq); }
         else if(e == 'c') { nominal_chc_freq = val; chc_phase_wrap_interval = (1.0 / nominal_chc_freq); }
         else if(e == 'd') { nominal_chd_freq = val; chd_phase_wrap_interval = (1.0 / nominal_chc_freq); }
         else if(e == 'p') { nominal_cha_freq = val; cha_phase_wrap_interval = (1.0 / nominal_cha_freq); }
         else if(e == 'o') { nominal_chb_freq = val; chb_phase_wrap_interval = (1.0 / nominal_chb_freq); }
         else if((e == '=') || (e == ':')) {
            set_nominal_freqs(val);
         }
         else return e;
      }
      else if(d == 'g') {  // /ig=gpsd_device_name
         if(arg[4] && ((e == '=') || (e == ':'))) {
            strcpy(gpsd_device, &arg[4]);
         }
         else gpsd_device[0] = 0;
         set_gpsd_mode();
      }
      else if(d == 'm') {     // /im? - set ticc mode (and nominal freqs / phase wrap intervals)
         if(e) e = toupper(e);
         if     (e == 'T') ;  // timestamp
         else if(e == 'I') ;  // interval
         else if(e == 'P') ;  // period
         else if(e == 'L') ;  // Timelab
         else if(e == 'D') ;  // debug
         else if(e == 'F') ;  // frequency
         else return c;       // invalid mode

         ticc_mode = e;
         user_set_ticc_mode = 1;
         have_ticc_mode = 1;

         if(((f == '=') || (f == ':')) && arg[5]) {  // user specifed nominal freq
            val = atof(&arg[5]);
            if(val <= 0.0) val = NOMINAL_FREQ;
         }
         else val = NOMINAL_FREQ;
         set_nominal_freqs(val);
      }
      else if(d == 'n') { // /in - open input file
         if(((e == '=') || (e == ':')) && arg[4]) {
            strcpy(in_name, &arg[4]);

            s = strchr(in_name, ',');
            if(s) {  // seek offset set
               *s = 0;
               seek_addr = (long) atoi(s+1);
            }
            else seek_addr = 0;

            in_file = topen(in_name, "rb");
            if(in_file == 0) return 1;
//          sim_file_read |= 0x01;

            fseek(in_file, seek_addr, SEEK_SET);
//          sim_eof = 0;
         }
         else return 1;
      }
      else if(d == 'r') {  // /ir - block commands that change the unit's state
         read_only = toggle_option(read_only, e);
      }
      else if(d == 's') {  // /is - do not send data out the serial port
         no_send = toggle_option(no_send, e);
      }
      else if((d == 't') && (e == 'h')) {  // /ith - force HP53xxx time interval counter
         ticc_type = HP_TICC;
         user_set_ticc_type = HP_TICC;
      }
      else if((d == 't') && (e == 'i')) {  // /itc - force generic time interval counter
         ticc_type = COUNTER_TICC;
         user_set_ticc_type = COUNTER_TICC;
      }
      else if((d == 't') && (e == 'l')) {  // /itl - force LARS GPSDO TICC
         ticc_type = LARS_TICC;
         user_set_ticc_type = LARS_TICC;
//lfs         plot[PPS].plot_id = "PPSL";
         if(rcvr_type == TICC_RCVR) {
            config_rcvr_plots();
         }
         if(user_set_ticc_mode == 0) {
            ticc_mode = 'I';   // time interval
            have_ticc_mode = 'I';
            user_set_ticc_mode = 6;
         }
      }
      else if((d == 't') && (e == 'p')) {  // /itp - force PICPET time interval counter
         ticc_type = PICPET_TICC;
         user_set_ticc_type = PICPET_TICC;
         if(user_set_ticc_mode == 0) {
            ticc_mode = 'T';
            have_ticc_mode = 'T';
            user_set_ticc_mode = 5;
         }
      }
      else if((d == 't') && (e == 't')) {  // /itt - force TAPR ticc
         ticc_type = TAPR_TICC;
         user_set_ticc_type = TAPR_TICC;
      }
      else if(d == 'x') {  // /ix - do not send message poll requests
         no_poll = toggle_option(no_poll, e);
         need_redraw = 4321;
      }
      else {               // /i - set plot queue update interval  
         if(((d == '=') || (d == ':')) && arg[3]) {
            queue_interval = (long) atosecs(&arg[3]);
            user_set_qi = 1;
            if(queue_interval < 1) queue_interval = 0;
         }
         else {
            day_plot = 24;
            queue_interval = 1;
            user_set_qi = 1;
            interval_set = 1;
         }
      }
   }
#ifdef ADEV_STUFF
   else if(c == 'j') {
      if(d == 'w') {    // /jw - JPL wall clock mode
         jpl_clock = toggle_option(jpl_clock, e);
         if(jpl_clock) {
            change_zoom_config(9);
            zoom_screen = 'C';
            zoom_fixes = show_fixes;
            alt_calendar = ISO;
         }
         else {
            change_zoom_config(-109);
            cancel_zoom(19);     //zkzk
            alt_calendar = 0;
         }
      }
      else if((d == 'a') || (d == 'p')) {  // /jp or /ja - set PPS / chA adev period
         set_not_safe();
         if(keyboard_cmd) {
            dont_reset_phase = 1;
            reset_queues(RESET_ADEV_Q, 1201);
            reset_queues(RESET_MTIE_Q, 1202);
            dont_reset_phase = 0;
         }
         if(((e == '=') || (e == ':')) && arg[4]) {
            sscanf(&arg[4], "%lf", &pps_adev_period);
            if(pps_adev_period < 0.0) pps_adev_period = 0.0;
         }
         else if(pps_adev_period == 1.0) pps_adev_period = 10.0;
         else pps_adev_period = 1.0;
         save_adev_period();
         user_set_pps_period = 1;
      }
      else if((d == 'b') || (d == 'o')) {  // /jo or /jb - set OSC / chB adev period
         set_not_safe();
         if(keyboard_cmd) {
            dont_reset_phase = 1;
            reset_queues(RESET_ADEV_Q, 1203);
            reset_queues(RESET_MTIE_Q, 1204);
            dont_reset_phase = 0;
         }
         if(((e == '=') || (e == ':')) && arg[4]) {
            sscanf(&arg[4], "%lf", &osc_adev_period);
            if(osc_adev_period < 0.0) osc_adev_period = 0.0;
         }
         else if(osc_adev_period == 1.0) osc_adev_period = 10.0;
         else osc_adev_period = 1.0;
         save_adev_period();
         user_set_osc_period = 1;
      }
      else if(d == 'c') {  // /jc - set chC adev period
         set_not_safe();
         if(keyboard_cmd) {
            dont_reset_phase = 1;
            reset_queues(RESET_ADEV_Q, 1205);
            reset_queues(RESET_MTIE_Q, 1206);
            dont_reset_phase = 0;
         }
         if(((e == '=') || (e == ':')) && arg[4]) {
            sscanf(&arg[4], "%lf", &chc_adev_period);
            if(chc_adev_period < 0.0) chc_adev_period = 0.0;
         }
         else if(chc_adev_period == 1.0) chc_adev_period = 10.0;
         else chc_adev_period = 1.0;
         save_adev_period();
         user_set_chc_period = 1;
      }
      else if(d == 'd') {  // /jd - set chD / chA adev period
         set_not_safe();
         if(keyboard_cmd) {
            dont_reset_phase = 1;
            reset_queues(RESET_ADEV_Q, 1207);
            reset_queues(RESET_MTIE_Q, 1208);
            dont_reset_phase = 0;
         }
         if(((e == '=') || (e == ':')) && arg[4]) {
            sscanf(&arg[4], "%lf", &chd_adev_period);
            if(chd_adev_period < 0.0) chd_adev_period = 0.0;
         }
         else if(chd_adev_period == 1.0) chd_adev_period = 10.0;
         else chd_adev_period = 1.0;
         save_adev_period();
         user_set_chd_period = 1;
      }
      else { // /j - adev sample period (which is not adev info display interval)
         set_not_safe();
         if(((d == '=') || (d == ':')) && arg[3]) {
            adev_period = (DATA_SIZE) atof(&arg[3]);
            if(adev_period < 0.0) adev_period = 0.0;
         }
         else if(adev_period == 1.0) adev_period = 10.0;
         else adev_period = 1.0;

         pps_adev_period = adev_period;  // aaaapppp
         osc_adev_period = adev_period;
         chc_adev_period = adev_period;
         chd_adev_period = adev_period;
         save_adev_period();

         user_set_adev_period = 1;
         user_set_pps_period = 1;
         user_set_osc_period = 1;
         user_set_chc_period = 1;
         user_set_chd_period = 1;

         if(keyboard_cmd) {
            reset_queues(RESET_ADEV_Q, 1209);
            reset_queues(RESET_MTIE_Q, 1210);
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
        if(((e == '=') || (e == ':')) && arg[4]) {
           no_sun_moon = atoi(&arg[4]);
        }
        else {
           if(no_sun_moon) no_sun_moon = 0;
           else            no_sun_moon = 0x03;
        }
        need_redraw = 4598;
      }
      else if(d == 'm') mouse_disabled = toggle_option(mouse_disabled, e);  // /km - disable mouse
      else if(d == 'q') disable_kbd = toggle_option(disable_kbd, e);// /kq - disable keyboard
      else if(d == 's') sound_on =  toggle_option(sound_on, e);  // /ks - disable sound files
      else if(d == 't') enable_timer = toggle_option(enable_timer, e); // /kt - disable windows timer
      else if(d == 'u') no_easter_eggs = toggle_option(no_easter_eggs, e);  // /ku - kill easter eggs
      else if(d == 'v') touch_screen = toggle_option(touch_screen, e);  // /kv - enable touch screen
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
         if(((e == '=') || (e == ':')) && arg[4]) lux_scale = (DATA_SIZE) atof(&arg[4]);
         else lux_scale = (DATA_SIZE) (1.0 / 10.76391);
         show_lux = 0;
         show_fc = 1;
         config_luxor_plots();
         return 0;
      }
      else if(d == 'h') {  // /lh - toggle timestamp headers in the log
         log_header = toggle_option(log_header, e);
      }
      else if(d == 'i') {  // /li - toggle wait-for-receiver-id before logging data value
         no_log_id_wait = toggle_option(no_log_id_wait, e);
      }
      else if(d == 'o') {  // /lo command - read old format log files
         old_log_format = toggle_option(old_log_format, e);
      }
      else if(luxor && (d == 'p')) {  // /lp - show lumens in candlepower
         if(((e == '=') || (e == ':')) && arg[4]) lum_scale = (DATA_SIZE) atof(&arg[4]);
         else lum_scale = (DATA_SIZE) (1.0/(4.0*PI));
         show_lumens = 0;
         show_cp = 1;
         config_luxor_plots();
         return 0;
      }
      else if(d == 's') {  // /ls - log file value separator
         if(((e == '=') || (e == ':')) && arg[4]) {
            csv_char = arg[4];
         }
         else {
            if(csv_char == ',') csv_char = '\t';
            else                csv_char = ',';
         }
      }
//    else if((d == 'u') && (e == 'x')) {  // /lux - force luxor mode
//       luxor = toggle_option(luxor, f);
//    }
      else if(luxor && (d == 'u')) {  // /lu - show lumens in lumens
         if(((e == '=') || (e == ':')) && arg[4]) lum_scale = (DATA_SIZE) atof(&arg[4]);
         else lum_scale = (DATA_SIZE) 1.0;
         show_lumens = 1;
         show_cp = 0;
         config_luxor_plots();
         return 0;
      }
      else if(luxor && (d == 'x')) {  // /lx - show lux in lux
         if((e == '=') || (e == ':')) lux_scale = (DATA_SIZE) atof(&arg[4]);
         else lux_scale = (DATA_SIZE) 1.0;
         show_lux = 1;
         show_fc = 0;
         config_luxor_plots();
         return 0;
      }
      else if(d == 't') {  // /lx - toggle hex packet stream log
         if(log_stream) log_stream = 0;
         else log_stream = (LOG_HEX_STREAM | LOG_PACKET_ID | LOG_PACKET_START | LOG_SENT_DATA);
      }
      else {  // /l - set log interva;
         if(((d == '=') || (d == ':')) && arg[3]) {
            log_interval = (long) atosecs(&arg[3]);
            if(log_interval < 1) log_interval = 0 - log_interval;
            log_file_time = log_interval+1;
         }
//       user_set_log |= 0x01;
      }
   }
   else if(c == 'm') {  // /m - multiply or set default plot scale factors
      if(d == 'a') {  // ma - 
         auto_scale = toggle_option(auto_scale, e);  // turn off auto scaling
         auto_center = auto_scale;
         set_auto_scale();
      }
      else if(d == 'b') {  // /mb  - toggle mouse single button mode (PI 480x320 touchscreen)
         crap_mouse = toggle_option(crap_mouse, e);
         user_set_crap_mouse = 1;
      }
      else if(d == 'd') {  // /md  - set dac scale factor
         plot[DAC].user_scale = 1;
         scale_factor = plot[DAC].scale_factor * (DATA_SIZE) 2.0;
         plot[DAC].invert_plot = 1.0;
         if(arg[4] && ((arg[3] == '=') || (arg[3] == ':'))) {
            scale_factor = (DATA_SIZE) atof(&arg[4]);
            if(scale_factor <= 0.0) scale_factor *= (-1.0);
            if(strstr(&arg[4], "-")) plot[DAC].invert_plot = (-1.0);
         }
         plot[DAC].scale_factor = scale_factor;
      }
      else if(d == 'i') {  // /mi - Invert PPS and termperature plot scale factors
         if(plot[PPS].invert_plot < 1.0F)  plot[PPS].invert_plot = 1.0F;
         else                              plot[PPS].invert_plot = (-1.0F);
         if(plot[TEMP].invert_plot < 1.0F) plot[TEMP].invert_plot = 1.0F;
         else                              plot[TEMP].invert_plot = (-1.0F);
      }
      else if(d == 'o') {  // /mo - osc error scale factor
         plot[OSC].user_scale = 1;
         plot[OSC].invert_plot = 1.0;
         scale_factor = (DATA_SIZE) plot[OSC].scale_factor * (DATA_SIZE) 2.0;
         if(arg[4] && ((arg[3] == '=') || (arg[3] == ':'))) {
            scale_factor = (DATA_SIZE) atof(&arg[4]);
            if(scale_factor <= 0.0) scale_factor *= (-1.0);
            if(strstr(&arg[4], "-")) plot[OSC].invert_plot = (-1.0);
         }
         plot[OSC].scale_factor = scale_factor;
      }
      else if(d == 'p') {  // /mp - pps error scale factor
         plot[PPS].user_scale = 1;
         plot[PPS].invert_plot = 1.0;
         scale_factor = (DATA_SIZE) plot[PPS].scale_factor * (DATA_SIZE) 2.0;
         if(arg[4] && ((arg[3] == '=') || (arg[3] == ':'))) {
            scale_factor = (DATA_SIZE) atof(&arg[4]);
            if(scale_factor <= 0.0) scale_factor *= (-1.0);
            if(strstr(&arg[4], "-")) plot[PPS].invert_plot = (-1.0);
         }
         plot[PPS].scale_factor = scale_factor;
      }
      else if(d == 't') {  // /mt - temperature scale factor
         plot[TEMP].user_scale = 1;
         scale_factor = plot[TEMP].scale_factor * (DATA_SIZE) 2.0;
         plot[TEMP].invert_plot = 1.0;
         if(arg[4] && ((arg[3] == '=') || (arg[3] == ':'))) {
            scale_factor = (DATA_SIZE) atof(&arg[4]);
            if(scale_factor <= 0.0) scale_factor *= (-1.0);
            if(strstr(&arg[4], "-")) plot[TEMP].invert_plot = (-1.0);
         }
         plot[TEMP].scale_factor = scale_factor;
      }
      else if(d == 'w') {  //  /mw - open RINEX file or set RINEX file name
         if(rinex_file) {
            fclose(rinex_file);
            rinex_file = 0;
            rinex_header_written = 0;
            rinex_name[0] = 0;
         }

         if((e == '=') || (e == ':')) {  // name given
            if(arg[4]) {
               strcpy(out, &arg[4]);
            }
            else {  // use IGS format name
               set_rinex_name();
            }
            if(out[0]) open_rinex_file(out);
         }
         else { // no name given, use IGS format name
            set_rinex_name();
            if(out[0]) open_rinex_file(out);
         }
         if(rinex_file == 0) need_rinex_file = 1;
         return 0;
      }
      else if((d >= '0') && (d <= '9')) {  // /m0 .. /m9 - the extra plots
         if(d == '0') d = 10;
         else         d = d - '0' - 1;
         d += FIRST_EXTRA_PLOT;
         plot[(int) d].user_scale = 1;
         scale_factor = plot[(int) d].scale_factor * (DATA_SIZE) 2.0;
         plot[(int) d].invert_plot = 1.0;
         if(arg[4] && ((arg[3] == '=') || (arg[3] == ':'))) {
            scale_factor = (DATA_SIZE) atof(&arg[4]);
            if(scale_factor <= 0.0) scale_factor *= (-1.0);
            if(strstr(&arg[4], "-")) plot[TEMP].invert_plot = (-1.0);
         }
         plot[(int) d].scale_factor = scale_factor;
      }
      else if((d == '=') || (d == ':') || (d == 0)) {   // /m - multiply all scale plot factors
         scale_factor = 2.0;
         if(d && arg[3]) {
            scale_factor = (DATA_SIZE) atof(&arg[3]);
         }
         
         plot[TEMP].user_scale    = plot[OSC].user_scale = plot[DAC].user_scale = plot[PPS].user_scale = 1;
         plot[OSC].scale_factor  *= scale_factor;
         plot[PPS].scale_factor  *= scale_factor;
         plot[DAC].scale_factor  *= scale_factor;
         plot[TEMP].scale_factor *= scale_factor;
      }
      else if(d) return c;
   }
   else if(c == 'n') {  // set end time
      if(d == 'a') {
        getting_string = ALARM_CMD;  //  /na - alarm time
        strcpy(out, &arg[4]);
        edit_dt(out, 0);
        getting_string = 0;
      }
      else if(d == 'c') {
        getting_string = EXEC_PGM_CMD;  //  /nc - program run interval
        strcpy(out, &arg[4]);
        edit_dt(out, 0);
        getting_string = 0;
      }
      else if(d == 'd') {
        getting_string = SCREEN_DUMP_CMD;  //  /nd - screen dump interval
        strcpy(out, &arg[4]);
        edit_dt(out, 0);
        getting_string = 0;
      }
      else if(d == 'e') {  // /ne - disable file execution commands
         ++no_exec;
      }
      else if(d == 'f') {  // /nf - toggle fast script mode
         fast_script = toggle_option(fast_script, e);
      }
      else if(d == 'l') {
        getting_string = LOG_DUMP_CMD;  //  /nl - log dump interval
        strcpy(out, &arg[4]);
        edit_dt(out, 0);
        getting_string = 0;
      }
      else if(d == 'p') {
        getting_string = SCRIPT_RUN_CMD;  //  /np - run script interval
        strcpy(out, &arg[4]);
        edit_dt(out, 0);
        getting_string = 0;
      }
      else if(d == 'r') {      // /nr - force nav rate on startup
         user_set_nav_rate = 1;
         if(((e == '=') || (e == ':')) && arg[4]) user_nav_rate = (DATA_SIZE) atof(&arg[4]);
         else                                     user_nav_rate = (DATA_SIZE) 1.0;

         if(user_nav_rate <= (DATA_SIZE) 0) user_nav_rate = (DATA_SIZE) 1;
         else if(user_nav_rate > (DATA_SIZE) 60) user_nav_rate = (DATA_SIZE) 60;
      }
      else if(d == 't') {      // /nt - try to wake up Nortel NTGxxxx unit
         ++nortel;
      }
      else if(d == 'x') {
        getting_string = EXIT_CMD;  //  /nx - exit time
        strcpy(out, &arg[4]);
        edit_dt(out, 0);
        getting_string = 0;
      }
      else if(d) return c;
      else {
        getting_string = EXIT_CMD;  //  /n - exit time
        strcpy(out, &arg[3]);
        edit_dt(out, 0);
        getting_string = 0;
      }
   }
#ifdef ADEV_STUFF
   else if(c == 'o') {   // /o - set adev type
      if     (d == 'a') { ATYPE = last_atype = OSC_ADEV; all_adevs = SINGLE_ADEVS; }  // /oa -
      else if(d == 'c') all_adevs = ALL_CHC;                             // /oc -
      else if(d == 'd') all_adevs = ALL_CHD;                             // /oc -
      else if(d == 'h') { ATYPE = last_atype = OSC_HDEV; all_adevs = SINGLE_ADEVS; }  // /oh -
      else if(d == 'i') { ATYPE = A_MTIE;   all_adevs = SINGLE_ADEVS; }  // /oi -
      else if(d == 'm') { ATYPE = last_atype = OSC_MDEV; all_adevs = SINGLE_ADEVS; }  // /om -
      else if(d == 't') { ATYPE = last_atype = OSC_TDEV; all_adevs = SINGLE_ADEVS; }  // /ot -
      else if(d == 'o') all_adevs = ALL_OSC;                             // /oo -
      else if(d == 'p') all_adevs = ALL_PPS;                             // /op -
      else return c;
      if(rcvr_type == TICC_RCVR) {
         if(all_adevs == SINGLE_ADEVS) {
            all_adevs = ALL_CHANS;
         }
      }
   }
#endif
   else return c;

   return 0;
}

int cmd_other(char *arg)
{
char c, d, e, f;
DATA_SIZE scale_factor;
int i;
char *s;
long seek_addr;

// parse_option(arg);
// c = opt_cmd[0];
// d = opt_cmd[1];
// e = opt_cmd[2];
// f = opt_cmd[3];

   d = e = f = 0;
   c = tolower(arg[1]);
   if(c) d = tolower(arg[2]);
   if(d) e = tolower(arg[3]);
   if(e) f = tolower(arg[4]);

   if(c == 'p') {    // toggle signal PPS enable
      if(d == 'o'){  // /po - set lat,lon,alt
         if(((e == '=') || (e == ':')) && arg[4]) {
            strcpy(edit_buffer, &arg[4]);
            if(parse_lla(&edit_buffer[0]) == LLA_OK) {
               lat = precise_lat;
               lon = precise_lon;
               alt = precise_alt;
               ref_lat = lat;
               ref_lon = lon;
               ref_alt = alt;
               cos_factor = cos(ref_lat);
               if(cos_factor == 0.0) cos_factor = 0.001;
            }
            else return 1;
         }
         else return 0;
      }
      else if(d == 'd') {  // /pd - disable PPS
         set_not_safe();
         ++set_pps_polarity;
         user_pps_enable = 0;
      }
      else if(d == 'e') {  // /pe - enable PPS
         set_not_safe();
         ++set_pps_polarity;
         user_pps_enable = 1;
      }
      else if((d == 'w') && ((e == 'a') || (e == 'p'))) {  // /pwa /pwp - set cha wrap interval
         if(((f == '=') || (f == ':')) && arg[5]) {
            cha_phase_wrap_interval = atof(&arg[5]);
         }
         else cha_phase_wrap_interval = (1.0 / nominal_cha_freq);
         user_set_wrap |= 0x01;
      }
      else if(d == 'r') {  // /pr = force plot row
         i = 0;
         if(((e == '=') || (e == ':')) && arg[4]) {
            i = atoi(&arg[4]);
         }
         if(i > 0) {
            PLOT_ROW = i;
            user_set_plot_row = 1;
         }
      }
      else if(d == 's') {  // /ps - toggle PPS
         set_not_safe();
         ++set_pps_polarity;
         user_pps_enable = toggle_option(user_pps_enable, e);
      }
      else if(d == 't') {  // /pt - set program title bar text
         if(((e == '=') || (e == ':')) && arg[4]) {
            strcpy(szAppName, &arg[4]);
         }
         else {  // no name specified, use unit_file_name
            szAppName[0] = 0;
         }
         user_set_title_bar = 11;
         if(keyboard_cmd) {
            set_title_bar(11);
         }
      }
      else if(d == 'l') {  // /pl - set power line freq
         if(((f == '=') || (f == ':')) && arg[5]) {
            plm_freq = atoi(&arg[5]);
         }
         else plm_freq = 60;
         set_nominal_freqs((double) plm_freq);
         plm_cycles = 0;
      }
      else if((d == 'w') && ((e == 'b') || (e == 'o'))) {  // /pwb /pwo - set chb wrap interval
         if(((f == '=') || (f == ':')) && arg[5]) {
            chb_phase_wrap_interval = atof(&arg[5]);
         }
         else chb_phase_wrap_interval = (1.0 / nominal_chb_freq);
         user_set_wrap |= 0x02;
      }
      else if((d == 'w') && (e == 'c')) {  // /pwc - set chc wrap interval
         if(((f == '=') || (f == ':')) && arg[5]) {
            chc_phase_wrap_interval = atof(&arg[5]);
         }
         else chc_phase_wrap_interval = (1.0 / nominal_chc_freq);
         user_set_wrap |= 0x04;
      }
      else if((d == 'w') && (e == 'd')) {  // /pwd - set chd wrap interval
         if(((f == '=') || (f == ':')) && arg[5]) {
            chd_phase_wrap_interval = atof(&arg[5]);
         }
         else chd_phase_wrap_interval = (1.0 / nominal_chd_freq);
         user_set_wrap |= 0x08;
      }
      else if(d == 'w') {  // /pw - set phase wrap interval for all channels
         if(((e == '=') || (e == ':')) && arg[4]) {
            cha_phase_wrap_interval = chb_phase_wrap_interval = atof(&arg[4]);
         }
         else {
            cha_phase_wrap_interval = (1.0 / nominal_cha_freq);
            chb_phase_wrap_interval = (1.0 / nominal_chb_freq);
         }
         user_set_wrap = 0x03;
      }
      else {
         set_not_safe();
         ++set_pps_polarity;
         user_pps_enable = toggle_option(user_pps_enable, d);
      }
   }
   else if(c == 'q') {    
      if(d == 'f')  {  // /qf - set max fft size
#ifdef FFT_STUFF
         set_not_safe();
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
         set_not_safe();
         if(((d == '=') || (d == ':')) && arg[3]) {
            sscanf(&arg[3], "%ld", &plot_q_size);
            strupr(&arg[3]);
            if     (strstr(&arg[3], "W")) plot_q_size *= 60L*60L*24L*7L;
            else if(strstr(&arg[3], "D")) plot_q_size *= 60L*60L*24L;
            else if(strstr(&arg[3], "H")) plot_q_size *= 60L*60L;
            else if(strstr(&arg[3], "M")) plot_q_size *= 60L;
         }
         else {     // default plot queue sizes
            plot_q_size = 60L*60L*24L*30L;   // boost plot queue to 30 days 'o data
         }

         if(plot_q_size <= 10L) plot_q_size = 10L;
         if(plot_q_size >= (366L*24L*60L*60L)) plot_q_size = (366L*24L*60L*60L);  // 1 year max
         user_set_plot_size = 1;
         if(keyboard_cmd) {  // we are resizing the queue
            alloc_plot();
            reset_queues(RESET_PLOT_Q, 1220);
         }
      }
   }
   else if(c == 'r') {    // Read a log file into the plot and adev queues
      if(d == 'b') {
         show_beep_reason =  toggle_option(show_beep_reason, e);  // /rb - show beep reason
      }
      else if(d == 'i') { // /ri - read aux TICC simulation (raw TICC data) file
         if(((e == '=') || (e == ':')) && arg[4]) {
            strcpy(ticc_sim_name, &arg[4]);

            s = strchr(ticc_sim_name, ',');
            if(s) {  // seek offset set
               *s = 0;
               seek_addr = (long) atoi(s+1);
            }
            else seek_addr = 0;

            ticc_sim_file = topen(ticc_sim_name, "rb");
            if(ticc_sim_file == 0) return 1;
            ticc_sim_file_read |= 0x01;
            ticc_sim_eof = 0;

            fseek(ticc_sim_file, seek_addr, SEEK_SET);
            ticc_sim_eof = 0;
         }
         else return 1;
      }
      else if(d == 'f') {  // /rf - set RINEX file format
         if(((e == '=') || (e == ':')) && arg[4]) {  // RINEX v2.11
            rinex_fmt = atof(&arg[4]);
         }
         else if(rinex_fmt >= 3.0) rinex_fmt = 2.11;
         else                      rinex_fmt = 3.03;
      }
      else if(d == 'm') {  // /rm and /rm? - set RINEX output measurement types
         if(((e == '=') || (e == ':')) && arg[4]) { // /rm RINEX v2.11
            strcpy(mixed_obs_list, &arg[4]);
            strupr(mixed_obs_list);
         }
         else if(e == 'b') { // /rmb RINEX v3.xx
            if(((f == '=') || (f == ':')) && arg[5]) {
               strcpy(beidou_obs_list, &arg[5]);
               strupr(beidou_obs_list);
            }
            else {
               strcpy(mixed_obs_list, "C1C,L1C,D1C,S1C");
            }
         }
         else if(e == 'g') { // /rmg RINEX v3.xx  GPS
            if(((f == '=') || (f == ':')) && arg[5]) {
               strcpy(gps_obs_list, &arg[5]);
               strupr(gps_obs_list);
            }
            else {
               strcpy(mixed_obs_list, "C1C,L1C,D1C,S1C");
            }
         }
         else if(e == 'l') { // /rml RINEX v3.xx  GALILEO
            if(((f == '=') || (f == ':')) && arg[5]) {
               strcpy(galileo_obs_list, &arg[5]);
               strupr(galileo_obs_list);
            }
            else {
               strcpy(mixed_obs_list, "C1C,L1C,D1C,S1C");
            }
         }
         else if(e == 'n') { // /rmn RINEX v3.xx GLONASS
            if(((f == '=') || (f == ':')) && arg[5]) {
               strcpy(glonass_obs_list, &arg[5]);
               strupr(glonass_obs_list);
            }
            else {
               strcpy(mixed_obs_list, "C1C,L1C,D1C,S1C");
            }
         }
         else if(e == 's') { // /rms RINEX v3.xx SBAS
            if(((f == '=') || (f == ':')) && arg[5]) {
               strcpy(sbas_obs_list, &arg[5]);
               strupr(sbas_obs_list);
            }
            else {
               strcpy(mixed_obs_list, "C1C,L1C,D1C,S1C");
            }
         }
         else {  // standard single freq L1 measurements
            strcpy(mixed_obs_list, "C1,L1,D1,S1");
         }
      }
      else if(d == 'o') {      // /ro - set gps date/time rollover adjustment in seconds
         user_set_rollover = 1;
         if(((e == '=') || (e == ':')) && arg[4]) {
            rollover = atof(&arg[4]);
            if(strchr(&arg[4], '*')) {  // value /ro=2* says rollover two 1024 week cycles
               rollover *= (1024.0 * 7.0 * (24.0*60.0*60.0));
            }
            else if(strchr(&arg[4], 'w') || strchr(&arg[4], 'd') || strchr(&arg[4], 'h') || strchr(&arg[4], 'm') || strchr(&arg[4], 's')) {  // rollover adjust in hms format
               rollover = hms_val(&arg[4]);
            }
            else if(strchr(&arg[4], 'W') || strchr(&arg[4], 'D') || strchr(&arg[4], 'H') || strchr(&arg[4], 'M') || strchr(&arg[4], 'S')) {  // rollover adjust in hms format
               rollover = hms_val(&arg[4]);
            }
         }
         else if(((e == '+') || (e == ':')) && arg[4]) {  // adjust rollover forwrad
            if(strchr(&arg[4], 'w') || strchr(&arg[4], 'd') || strchr(&arg[4], 'h') || strchr(&arg[4], 'm') || strchr(&arg[4], 's')) {  // rollover adjust in hms format
               rollover += hms_val(&arg[4]);
            }
            else if(strchr(&arg[4], 'W') || strchr(&arg[4], 'D') || strchr(&arg[4], 'H') || strchr(&arg[4], 'M') || strchr(&arg[4], 'S')) {  // rollover adjust in hms format
               rollover += hms_val(&arg[4]);
            }
            else {
               rollover += atof(&arg[4]);
            }
         }
         else if(((e == '-') || (e == ':')) && arg[4]) {  // adjust rollover backwards
            if(strchr(&arg[4], 'w') || strchr(&arg[4], 'd') || strchr(&arg[4], 'h') || strchr(&arg[4], 'm') || strchr(&arg[4], 's')) {  // rollover adjust in hms format
               rollover -= hms_val(&arg[4]);
            }
            else if(strchr(&arg[4], 'W') || strchr(&arg[4], 'D') || strchr(&arg[4], 'H') || strchr(&arg[4], 'M') || strchr(&arg[4], 'S')) {  // rollover adjust in hms format
               rollover -= hms_val(&arg[4]);
            }
            else {
               rollover -= atof(&arg[4]);
            }
         }
         else rollover = 1024.0 * 7.0 * (24.0*60.0*60.0);
      }
      else if(d == 'r') {  // /rr - set reveiver raw satellie observation data output rate (for rtklib/RINEX) messages
         special_raw = 0;
         if(((e == '=') || (e == ':')) && arg[4]) {
            raw_msg_rate = atoi(&arg[4]);
            if(raw_msg_rate < 0) {
               raw_msg_rate = 0 - raw_msg_rate;
               special_raw = 1;
            }
         }
         else {
            if(raw_msg_rate) raw_msg_rate = 0;
            else             raw_msg_rate = RAW_MSG_RATE;
         }
         reset_eph_flags();
         user_set_raw_rate = 1;
         if(keyboard_cmd) {
            init_messages(555,0);
         }
      }
      else if(d == 's') { // /rs - read simulation (raw receiver data) file
         if(((e == '=') || (e == ':')) && arg[4]) {
            strcpy(sim_name, &arg[4]);

            s = strchr(sim_name, ',');
            if(s) {  // seek offset set
               *s = 0;
               seek_addr = (long) atoi(s+1);
            }
            else seek_addr = 0;

            sim_file = topen(sim_name, "rb");
            if(sim_file == 0) return 1;
            sim_file_read |= 0x01;

            fseek(sim_file, seek_addr, SEEK_SET);
            sim_eof = 0;
         }
         else return 1;
      }
      else if(d == 't') { // /rt - Resolution-T receiver... use 9600,ODD,1
//       res_t_init = toggle_option(res_t_init, e);
         com[RCVR_PORT].parity = ODD_PAR;
         parity_defined = com[RCVR_PORT].parity;
         if(((e == '=') || (e == ':')) && arg[4]) {
            res_t = (u08) atof(&arg[4]);
            if(res_t == RES_T_ICM) saw_icm = 2;
            user_set_res_t = 1;
            if(com[RCVR_PORT].user_set_baud == 0) {
               com[RCVR_PORT].user_set_baud = 1;
               if(res_t == RES_T_360) com[RCVR_PORT].baud_rate = 115200;
               else if(res_t == RES_T_ICM) com[RCVR_PORT].baud_rate = 115200;
               else com[RCVR_PORT].baud_rate = 9600;
               com[RCVR_PORT].data_bits = 8;
               com[RCVR_PORT].parity = ODD_PAR;
               com[RCVR_PORT].stop_bits = 1;
               parity_defined = com[RCVR_PORT].parity;
            }
         }
      }
      else if(d == 'x') {  // /rx - set receiver type
// NEW_RCVR
         detect_rcvr_type = 0;
         last_rcvr_type = rcvr_type;
         tsip_type = TBOLT_TYPE;
         buffered_term = 0;

         if(e == 'a') {  // /rxa - Acron Zeit WWVB clock
            if(f == 'i') {  // /rxai - Trimble ACE-III
               rcvr_type = TSIP_RCVR;
               tsip_type = ACE3_TYPE;
            }
            else if(f == 'g') { // /rxag - Trimble Acutime GG multi-GNSS
               rcvr_type = TSIP_RCVR;
               tsip_type = ACUTIME_TYPE;
               acu_type = 'G';
            }
            else if(f == 't') { // /rxat - Trimble Acutime 1000/2000/Gold
               rcvr_type = TSIP_RCVR;
               tsip_type = ACUTIME_TYPE;
               acu_type = '1';
            }
            else if(f == '3') { // /rxa3 - Trimble Acutime 360 multi-GNSS
               rcvr_type = TSIP_RCVR;
               tsip_type = ACUTIME_TYPE;
               acu_type = '3';
            }
            else {
               rcvr_type = ACRON_RCVR;
            }
         }
         else if(e == 'b') { // /rxb - Brandywine GPS4
            rcvr_type = BRANDY_RCVR;
         }
         else if((e == 'c') && (f == 'a')) {   // /rxca calculator only mode
            rcvr_type = NO_RCVR;
            show_tides = 0;
            show_fixes = 0;
            calc_rcvr = 1;
         }
         else if(e == 'c') {   // /rxc /rxcp /rxcs /rxcu - Trimble/Symmetricom/Samsung UCCM SCPI
            rcvr_type = UCCM_RCVR; 
            scpi_type = UCCM_TYPE;
            if(f == 'p') {
               scpi_type = UCCMP_TYPE;
               user_set_scpi_type = 1;
            }
            else if(f == 's') {
               scpi_type = SAMSUNG_TYPE;
               user_set_scpi_type = 1;
            }
            else if(f == 'u') {
               scpi_type = UCCM_TYPE;
               user_set_scpi_type = 1;
            }
            scpi_init(e);

            config_utc_mode(0);
         }
         else if(e == 'd') {  // /rxd - DATUM STARLOC GPSDO
            rcvr_type = TSIP_RCVR;
            tsip_type = STARLOC_TYPE;
         }
         else if((e == 'e') && (f == 'n')) { // /rxen - environmenal sensors
            rcvr_type = THERMO_RCVR;
            fan_port = RCVR_PORT;
         }
         else if((e == 'e') && (f == 's')) { // /rxes - Furuno eSIP GPS
            rcvr_type = ESIP_RCVR;
         }
         else if(e == 'e') {  // /rxe - NEC GPSDO (like STAR-4 at 115200 baud)
            rcvr_type = STAR_RCVR;
            star_type = NEC_TYPE;
         }
         else if((e == 'f') && (f == 'u')) { // /rxfu - Furuno PFEC GPS
            rcvr_type = FURUNO_RCVR;
         }
         else if(e == 'f') {  // /rxf - Lucent RFTG-m
            rcvr_type = RFTG_RCVR;
         }
         else if(e == 'g') {  // /rxg - GPSD
            set_gpsd_mode();
         }
         else if(e == 'h') {  // /rxh - HP-5071A cesium
            rcvr_type = CS_RCVR;
            cs_type = HP5071;
         }
         else if(e == 'i') {  // /rxi - TAPR TICC
            rcvr_type = TICC_RCVR;
            if(user_set_ticc_type == 0) ticc_type = TAPR_TICC;
            if(user_set_ticc_mode == 0) ticc_mode = DEFAULT_TICC_MODE;    // aaattt
         }
         else if(e == 'j') {  // /rxj - Zodiac / Jupiter-T
            rcvr_type = ZODIAC_RCVR;
         }
         else if(e == 'k') {  // /rxk - Lucent KS-24xxx SCPI
            rcvr_type = SCPI_RCVR;
            scpi_type = LUCENT_TYPE;
            user_set_scpi_type = 1;
            scpi_init(e);
         }
         else if((e == 'l') && (f == 'p')) { // /rxlp - Spectratime LPFRS rubidium
            rcvr_type = LPFRS_RCVR;
         }
         else if(e == 'l') {  // //rxl - Jackson Labs LTE Lite
            rcvr_type = VENUS_RCVR;
            if(com[RCVR_PORT].user_set_baud == 0) {
               com[RCVR_PORT].user_set_baud = 1;
               com[RCVR_PORT].baud_rate = 38400;
               com[RCVR_PORT].data_bits = 8;
               com[RCVR_PORT].parity = NO_PAR;
               com[RCVR_PORT].stop_bits = 1;
               parity_defined = com[RCVR_PORT].parity;
            }
         }
         else if(e == 'm') {  // /rxm - Motorola
            rcvr_type = MOTO_RCVR;
            moto_type = 0;
         }
         else if(e == 'n') {  // /rxn - NMEA receivers
            rcvr_type = NMEA_RCVR;
         }
         else if(e == 'o') {  // /rxo - TruePosition GPSDO
            rcvr_type = TRUE_RCVR;
         }
         else if((e == 'p') && (f == 'p')) { // /rxpp - TVB's PICPET timestamping counter
            rcvr_type = TICC_RCVR;

            if(user_set_ticc_type == 0) {
               ticc_type = PICPET_TICC;
               user_set_ticc_type = ticc_type;
            }
            if(user_set_ticc_mode == 0) {
               ticc_mode = 'T';    // time stamp mode
               user_set_ticc_mode = 'T';
               have_ticc_mode = 2;
            }
         }
         else if((e == 'p') && (f == 'a')) { // /rxpa - Trimble Palisade
            rcvr_type = TSIP_RCVR;
            tsip_type = PALISADE_TYPE;
         }
         else if((e == 'p') && (f == 'r')) { // /rxpr - PRS-10 rubidium
            rcvr_type = PRS_RCVR;
         }
         else if(e == 'p') {  // /rxp - Trimble TAIP
            rcvr_type = TAIP_RCVR;
         }
         else if(e == 'q') {  // //rxq - luxor
            rcvr_type = LUXOR_RCVR; // TSIP_RCVR;
            tsip_type = LUXOR_TYPE;
            com[RCVR_PORT].parity = LUXOR_PAR;
            parity_defined = com[RCVR_PORT].parity;
            luxor = 21;
         }
         else if(e == 'r') {  // /rxr - Trimble res-T
            rcvr_type = TSIP_RCVR;
            tsip_type = TBOLT_TYPE;
            res_t = RES_T;
            user_set_res_t = 1;

            if(((f == '=') || (f == ':')) && arg[5]) {
               res_t = (u08) atof(&arg[5]);
               user_set_res_t = 1;
            }
//          if(res_t == RES_T_SMT) com[RCVR_PORT].parity = NO_PAR;

            if(com[RCVR_PORT].user_set_baud == 0) {
               com[RCVR_PORT].user_set_baud = 1;
               if(res_t == RES_T_360) com[RCVR_PORT].baud_rate = 115200;
               else if(res_t == RES_T_ICM) com[RCVR_PORT].baud_rate = 115200;
               else com[RCVR_PORT].baud_rate = 9600;
               com[RCVR_PORT].data_bits = 8;
               com[RCVR_PORT].parity = ODD_PAR;
               com[RCVR_PORT].stop_bits = 1;
               parity_defined = com[RCVR_PORT].parity;
            }

            config_rcvr_plots();
            config_lla_plots(KEEP_LLA_ON_HOLD, DISABLE_LLA_SHOW);
            config_msg_ofs();
         }
         else if((e == 's') && (f == 'a')) { // /rxsa - SA22 rubidium at 60 MHz
            rcvr_type = X72_RCVR;
            x72_type = SA22_TYPE;
            have_x72_type = 1;
            x72_osc = X72_OSC;
            user_set_x72_osc = 3;
         }
         else if((e == 's') && (f == 'b')) { // /rxsb - SA22 rubidium at 58982400.0 Hz
            rcvr_type = X72_RCVR;
            x72_type = SA22_TYPE;
            have_x72_type = 1;
            x72_osc = X72_OSC2;
            user_set_x72_osc = 4;
         }
         else if((e == 's') && (f == 's')) { // /rxss - Novatel Superstar II
            rcvr_type = SS_RCVR;
         }
         else if((e == 's') && (f == 'y')) { // /rxsy - X72 or SA22 or X99 rubidium
            rcvr_type = X72_RCVR;
            x72_type = X72_TYPE;
            have_x72_type = 0;
            user_set_x72_osc = 1;
         }
         else if((e == 's') && (f == 'r')) { // /rxsr - Spectratime SRO100 rubidium
            rcvr_type = SRO_RCVR;
         }
         else if(e == 's') {  // /rxs - SIRF
            rcvr_type = SIRF_RCVR;
         }
         else if((e == 't') && ((f == 'e') || (f == 't') || (f == 'b'))) { // /rxte or /rxtt  or /rxtb - terminal emulator
            if(f == 'b') buffered_term = 1;
            rcvr_type = TERM_RCVR;
            enable_terminal = 1;

            if(user_set_plot_size == 0) {
               plot_q_size = 10L;
            }
            if(user_set_adev_size == 0) {
               adev_q_size = 1L;
               adev_period = 0.0;
               pps_adev_period = adev_period;
               osc_adev_period = adev_period;
               chc_adev_period = adev_period;
               chd_adev_period = adev_period;
            }

            if(((arg[5] == '=') || (arg[5] == ':')) && arg[6]) {
               strcpy(edit_buffer, &arg[6]);
               if(keyboard_cmd) kill_com(RCVR_PORT,3016);
               config_port(RCVR_PORT, &arg[6]);
//             set_serial_params(RCVR_PORT, &arg[6], 1);
            }
         }
         else if((e == 't') && (f == 'm')) { // /rxtm - Spectum TM4 GPSDO
            rcvr_type = TM4_RCVR;
         }
         else if((e == 't') && (f == 's')) { // /rxts - TymServe 2000
            rcvr_type = TSERVE_RCVR;
         }
         else if(e == 't') {  // /rxt - Trimble TSIP devices
            rcvr_type = TSIP_RCVR;
            tsip_type = TBOLT_TYPE;
         }
         else if((e == 'u') && (f == 't')) {  // /rxut - Ublox timing receiver (with TP5 messages)
            rcvr_type = UBX_RCVR;
            saw_ubx_tp5 = 0x80;
            saw_ubx_tp = 0x00;
         }
         else if(e == 'u') {  // /rxu - Ublox
            rcvr_type = UBX_RCVR;
            saw_ubx_tp5 = 0x00;
            saw_ubx_tp = 0x00;
         }
         else if((e == 'v') && (f == 'b')) { // /rxvb - Venus RTK receivers, base mode
            rcvr_type = VENUS_RCVR;
            venus_type = VENUS_RTK;
            rtk_mode = BASE_MODE;
            have_venus_timing = 0;
         }
         else if((e == 'v') && (f == 'p')) { // /rxvp - Motorola VP receiver
            rcvr_type = MOTO_RCVR;
            moto_type = VP_TYPE;
         }
         else if((e == 'v') && (f == 'r')) { // /rxvr - Venus RTK receivers, rover mode
            rcvr_type = VENUS_RCVR;
            venus_type = VENUS_RTK;
            rtk_mode = ROVER_MODE;
            have_venus_timing = 0;
         }
         else if((e == 'v') && (f == 't')) { // /rxvt - Venus timing receivers
            rcvr_type = VENUS_RCVR;
            venus_type = VENUS_TIMING;
            have_venus_timing = 0x80;
         }
         else if(e == 'v') {  // /rxv - Venus
            rcvr_type = VENUS_RCVR;
            venus_type = VENUS_TYPE;
            have_venus_timing = 0;
         }
         else if((e == 'x') && (f == '7')) { // /rxx7 - X72 rubidium
            rcvr_type = X72_RCVR;
            x72_type = X72_TYPE;
            have_x72_type = 1;
         }
         else if((e == 'x') && (f == '9')) { // /rxx9 - X99 rubidium
            rcvr_type = X72_RCVR;
            x72_type = X99_TYPE;
            have_x72_type = 1;
         }
         else if(e == 'x') {  // /rxx - no receiver, clock only mode
            rcvr_type = NO_RCVR;
            show_tides = 0;
            show_fixes = 0;
            calc_rcvr = 0;
         }
         else if(e == 'y') {  // /rxy - Nortel SCPI mode
            rcvr_type = SCPI_RCVR;
            scpi_type = NORTEL_TYPE;
            user_set_scpi_type = 1;
            scpi_init(e);
         }
         else if(e == 'z') {  // /rxz - SCPI (Z3801, etc)
            rcvr_type = SCPI_RCVR;
            scpi_type = SCPI_TYPE;
            user_set_scpi_type = 1;
            scpi_init(e);
         }
         else if(e == '0') {  // /rx0 - no receiver, plots shown for earth tides / gravity offset
            rcvr_type = TIDE_RCVR;
         }
         else if((e == '1') && (f == '2')) {  // /rx12 - Ahstech Z12 receiver
            rcvr_type = Z12_RCVR;
         }
         else if((e == '1') && (f == '7')) { // /rx17 - Trimble RT17 receiver
            rcvr_type = RT17_RCVR;
         }
         else if(e == '1') {  // /rx1 - HP53xxx counter (9600 baud counters)
            rcvr_type = TICC_RCVR;
            ticc_type = HP_TICC;
         }
         else if((e == '3') && (f == '5')) { // /rx35 - Symmetricom SA35.m rubidium
            rcvr_type = SA35_RCVR;
         }
         else if(e == '3') {  // /rx3 - Zyfer Nanosync 380
            rcvr_type = ZYFER_RCVR;
         }
         else if((e == '4') && (f == '5')) { // /rx45 - Oscilloquartz OSA-453x
            rcvr_type = STAR_RCVR;
            star_type = OSA_TYPE;
            buffered_term = 1;
         }
         else if(e == '4') {  // /rx4 - Oscilloquartz Star-4
            rcvr_type = STAR_RCVR;
            star_type = STAR4_TYPE;
         }
         else if(e == '5') {  // /rx5 - HP58xxx SCPI   - !!!!! what about HP_TYPE2
            rcvr_type = SCPI_RCVR;
            scpi_type = HP_TYPE;
            user_set_scpi_type = 1;
            scpi_init(e);
         }
         else if(e == '6') {  // /rx6 - Trimble SV6, etc
            rcvr_type = TSIP_RCVR;
            tsip_type = SV6_TYPE;
            pps_enabled = 1;
         }
         else if(e == '8') {   // /rx8 - NVS BINR messages
            rcvr_type = NVS_RCVR;
         }
         else {
            detect_rcvr_type = 1;
            if(hw_setup) auto_detect(0);
            if(((e == '=') || (e == ':')) && arg[4]) { // /rx=utc offset
               utc_offset = (int) atof(&arg[4]);
               have_utc_ofs = 102;
               user_set_utc_ofs = 3;
            }
            return 0;
         }


         if(res_t) ;  // /rx?=# sets res-t type, not UTC offset!!!
         else if(rcvr_type == TERM_RCVR) ;
         else if(((f == '=') || (f == ':')) && arg[5]) { // /rx?=utc offset
            utc_offset = (int) atof(&arg[5]);
            have_utc_ofs = 102;
            user_set_utc_ofs = 3;
         }
         else if(f) {  // support for two character receiver types /rx??=#
            f = arg[5];
            if(((f == '=') || (f == ':')) && arg[6]) { // /rx??=utc offset
               utc_offset = (int) atof(&arg[6]);
               have_utc_ofs = 102;
               user_set_utc_ofs = 3;
            }
         }

         if((last_rcvr_type == NO_RCVR) && (rcvr_type != NO_RCVR)) { // re-enable com port
            com[RCVR_PORT].process_com = 1;
         }

         detect_rcvr_type = 0;  // user forced the receiver type
         config_rcvr_type(1);
         if(keyboard_cmd) need_msg_init = 1000;
      }
      else { // /r - read log file
         if(((d == '=') || (d == ':')) && arg[3]) {
            strcpy(read_log, &arg[3]);
         }
         else strcpy(read_log, log_name);
      }
   }
   else if((c == 's') && (d == 'i')) {   // /si - force max displayed sats
      max_sat_display = 14;
      user_set_sat_cols = 1;
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
      ms_row = ms_col = 0;
if((sat_cols > 1) && (max_sat_display < 16)) max_sat_display = 16;

      max_sats = max_sat_display;  // used to format the sat_info data
      max_sat_count = max_sat_display;
      temp_sats = max_sat_display;
      config_sat_rows();
   }
#if USE_SDL
   else if((c == 's') && (d == 'c')) {   // /sc - set SDL window scaling
       sdl_scaling = 1;
   }
#endif
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
   else if((c == 's') && (d == 'o')) {   // /so - set satellite outline shape
      if(((e == '=') || (e == ':')) && (arg[4])) {
         fancy_sats = atoi(&arg[4]);
         if(fancy_sats < ROUND_SATS) fancy_sats = ROUND_SATS;
         else if(fancy_sats > RECT_LINES) fancy_sats = RECT_LINES;
      }
      else if(fancy_sats) fancy_sats = 0;
      else fancy_sats = RECT_WINGS;
   }
   else if((c == 's') && (d == 's')) {   // /ss - force site position survey
      user_precision_survey = 0;  //medsurv
      do_survey = SURVEY_SIZE;
      survey_why = 32;
      if(((e == '=') || (e == ':')) && arg[4]) sscanf(&arg[4], "%ld", &do_survey);
      if(do_survey < 1)  {
         do_survey = 0;
         survey_why = (-33);
      }
   }
   else if((c == 's') && (d == 't')) {   // /st - toggle satellite trails
      map_trails = toggle_option(map_trails, e);
      if(keyboard_cmd) need_redraw = 3489;
   }
   else if((c == 's') && (d == 'w')) {   // /sw - simulation file message processing delay
      if(e == '=')      sim_delay = (long) atof(&arg[4]);
      else if(e == ':') sim_delay = (long) atof(&arg[4]);
      else if(e)        sim_delay = (long) atof(&arg[3]);
      else              sim_delay = 100;
      if(sim_delay < 0) sim_delay = 0 - sim_delay;
      return 0;
   }
#ifdef PRECISE_STUFF
   else if((c == 's') && (d == 'f')) {   // /sf - force 3D fix mode
      show_fixes = toggle_option(show_fixes, e);
      user_fix_set = show_fixes;
//    if(show_fixes) user_precision_survey = 2;
//    else           user_precision_survey = 0;
      do_survey = do_median_survey = 0;
      survey_why = (-21);
      user_precision_survey = 0;
   }
   else if((c == 's') && (d == 'p')) {   // /sp - force precision survey
      set_not_safe();
      user_precision_survey = 1; // medsurv
      do_survey = 0;
      do_median_survey = 0;
      survey_why = (-20);
      if(arg[3]) sscanf(&arg[4], "%ld", &do_median_survey);
      if((do_median_survey < 1) || (do_median_survey > SURVEY_BIN_COUNT)) {
         do_median_survey = 0;
         survey_why = (-34);
      }
      else survey_why = 34;
      do_survey = do_median_survey;  // medsurv
   }
#endif
   else if(c == 't') {   // set Time reference (GPS/UTC) or temp reference (C/F)
      if(arg[3] && ((d == '=') || (d == ':') || (d == 'z'))) {  // /t or /tz - zoneinfo or /tz=zoneinfo
         set_time_zone(&arg[3]);
         if(keyboard_cmd) calc_dst_times(this_year, dst_list[dst_area]);
         return 0;
      }
      else if((d == '+') || (d == '-') || isdigit(d)) {
         set_time_zone(&arg[2]);
         if(keyboard_cmd) calc_dst_times(this_year, dst_list[dst_area]);
         return 0;
      }
      else if(d == 'a') {  // /ta - european date ordering
         show_euro_dates = toggle_option(show_euro_dates, e);
         show_iso_dates = 0;
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
         else if(rcvr_type == THERMO_RCVR) plot[TC2].plot_center = NEED_CENTER;
         return 0;
      }
      else if(d == 'd') {   // /td - use degrees Delisle
         DEG_SCALE = 'D';
         plot[TEMP].plot_center = NEED_CENTER;
         if(luxor) plot[TC2].plot_center = NEED_CENTER;
         else if(rcvr_type == THERMO_RCVR) plot[TC2].plot_center = NEED_CENTER;
         return 0;
      }
      else if(d == 'e') {   // /te - use degrees Reaumur
         DEG_SCALE = 'E';
         plot[TEMP].plot_center = NEED_CENTER;
         if(luxor) plot[TC2].plot_center = NEED_CENTER;
         else if(rcvr_type == THERMO_RCVR) plot[TC2].plot_center = NEED_CENTER;
         return 0;
      }
      else if(d == 'f') {   // /tf - use degrees F
         DEG_SCALE = 'F';
         plot[TEMP].plot_center = NEED_CENTER;
         if(luxor) plot[TC2].plot_center = NEED_CENTER;
         else if(rcvr_type == THERMO_RCVR) plot[TC2].plot_center = NEED_CENTER;
         return 0;
      }
      else if(d == 'g') {   // /tg - use GPS time
         ++set_gps_mode;                  
         time_flags |= TFLAGS_UTC;  // forces a time switch to GPS
         set_utc_mode = 0;
         user_set_time_mode = 'g';
         return 0;
      }
      else if(d == 'h') {  // /th - set chime mode
         strupr(arg);
         cuckoo_hours = singing_clock = tick_clock = ships_clock = fine_tick_clock = 0;
         if(strstr(&arg[3], "H")) cuckoo_hours = 1;

         if     (strstr(&arg[3], "S")) singing_clock = 1;
         else if(strstr(&arg[3], "W")) singing_clock = 2;   // Westminster chimes mode
         else if(strstr(&arg[3], "B")) ships_clock = 1;
         else if(strstr(&arg[3], "F")) fine_tick_clock = tick_clock = 1;
         else if(strstr(&arg[3], "T")) tick_clock = 1;

         if(((e == '=') || (e == ':')) && (arg[4])) {
            i = atoi(&arg[4]);
            if(i < 0) i = 0 - i;
            if(i > 60) i = 4;
            if(tick_clock) tick_clock = (u08) i;
            else cuckoo = (u08) i;
         }
         if(tick_clock) cuckoo = 0;
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
         else if(rcvr_type == THERMO_RCVR) plot[TC2].plot_center = NEED_CENTER;
         return 0;
      }
      else if((d == 'l') && (e == 'g'))  {   // /tlg - use linguini
         alt_scale = "lg";
         LLA_SPAN = last_lla_span = 3.0;
         ANGLE_SCALE = ((DEG_PER_FOOT)*LG_PER_METER); // degrees per meter
         angle_units = "lg";
         if((f == '=') || (f == ':')) {
            if(arg[5]) {
               LLA_SPAN = atof(&arg[5]);
               if(LLA_SPAN <= 0.0) LLA_SPAN = 3.0;
               last_lla_span = LLA_SPAN;
            }
            if(keyboard_cmd) {
               rebuild_lla_plot(0);
            }
         }
         return 0;
      }
      else if(d == 'l') {  // /tl - ISO (yyyy.mm.dd) date ordering
         show_iso_dates = toggle_option(show_iso_dates, e);
         show_euro_dates = 0;
         return 0;
      }
      else if(d == 'm') {   // /tm - use meters
         alt_scale = "m";
         LLA_SPAN = last_lla_span = 3.0;
         ANGLE_SCALE = ((DEG_PER_FOOT)*FEET_PER_METER); // degrees per meter
         angle_units = "m";
         if((e == '=') || (e == ':')) {
            if(arg[4]) {
               LLA_SPAN = atof(&arg[4]);
               if(LLA_SPAN <= 0.0) LLA_SPAN = 3.0;
               last_lla_span = LLA_SPAN;
            }
            if(keyboard_cmd) {
               rebuild_lla_plot(0);
            }
         }
         return 0;
      }
      else if(d == 'n') {   // /tn - use degrees Newton
         DEG_SCALE = 'N';
         plot[TEMP].plot_center = NEED_CENTER;
         if(luxor) plot[TC2].plot_center = NEED_CENTER;
         else if(rcvr_type == THERMO_RCVR) plot[TC2].plot_center = NEED_CENTER;
         return 0;
      }
      else if(d == 'o') {   // /to - use degrees Romer
         DEG_SCALE = 'O';
         plot[TEMP].plot_center = NEED_CENTER;
         if(luxor) plot[TC2].plot_center = NEED_CENTER;
         else if(rcvr_type == THERMO_RCVR) plot[TC2].plot_center = NEED_CENTER;
         return 0;
      }
      else if((d == 'p') && (e == 'h')) {  // /tph - show temperature as (Paris) Hiltons
         DEG_SCALE = 'H';
         plot[TEMP].plot_center = NEED_CENTER;
         if(luxor) plot[TC2].plot_center = NEED_CENTER;
         else if(rcvr_type == THERMO_RCVR) plot[TC2].plot_center = NEED_CENTER;
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
         else if(rcvr_type == THERMO_RCVR) plot[TC2].plot_center = NEED_CENTER;
         return 0;
      }
      else if((d == 's') && (e == 'c')) {   // /tsc - don't use TSC for GetMsecs() and GetNsecs()
         if(((f == '=') || (f == ':')) && arg[5]) {
            use_tsc = (int) atof(&arg[5]);
         }
         else use_tsc ^= 1;
         return 0;
      }
      else if((d == 's') && (e == 'g')) {   // /tsg - show digital clock as GPS epoch time
         show_gps_time = toggle_option(show_gps_time, f);
         show_unix_time = 0;
         show_mjd_time = 0;
         show_julian_time = 0;
         show_msecs = 0;
         jd_elapsed = 0.0;
         show_elapsed_time = 0;
         clock_12 = 0;
         return 0;
      }
      else if((d == 's') && (e == 'j')) {   // /tsj - show digital clock in Julian format
         show_julian_time = toggle_option(show_julian_time, f);
         show_mjd_time = 0;
         show_unix_time = 0;
         show_gps_time = 0;
         show_msecs = 0;
         jd_elapsed = 0.0;
         show_elapsed_time = 0;
         clock_12 = 0;
         return 0;
      }
      else if((d == 's') && (e == 'k')) {   // /tsk - show digital clock in Modified Julian format
         show_mjd_time = toggle_option(show_mjd_time, f);
         show_julian_time = 0;
         show_unix_time = 0;
         show_gps_time = 0;
         show_msecs = 0;
         jd_elapsed = 0.0;
         show_elapsed_time = 0;
         clock_12 = 0;
         return 0;
      }
      else if((d == 's') && (e == 'n')) {   // /tsn - normal 24 hour digital clock
         show_mjd_time = toggle_option(show_mjd_time, f);
         show_julian_time = 0;
         show_unix_time = 0;
         show_gps_time = 0;
         show_msecs = 0;
         jd_elapsed = 0.0;
         show_elapsed_time = 0;
         clock_12 = 0;
         return 0;
      }
      else if((d == 's') && (e == 'u')) {   // /tsu - show digital clock as Unix (32-bit) time
         show_unix_time = toggle_option(show_unix_time, f);
         show_gps_time = 0;
         show_mjd_time = 0;
         show_julian_time = 0;
         show_msecs = 0;
         jd_elapsed = 0.0;
         show_elapsed_time = 0;
         clock_12 = 0;
         return 0;
      }
      else if((d == 's') && (e == 'w')) {   // /tsw - stopwatch clock
         elapsed_time_set = 0;
         jd_elapsed = 0.0;
         show_elapsed_time = 1;
      }
      else if((d == 's') && (e == 'x')) {   // /tsx - don't use TSC for GetMsecs() and GetNsecs()
         time_sync_offset = (TIME_SYNC_AVG);
         if(((f == '=') || (f == ':')) && arg[5]) {
            time_sync_offset = atof(&arg[5]);
         }
         user_set_tsx = 1;
         return 0;
      }
      else if((d == 's') && (e == 'z')) {   // /tsz - show digital clock with millisecond format
         show_msecs = toggle_option(show_msecs, f);
         jd_elapsed = 0.0;
         show_elapsed_time = 0;
         show_julian_time = 0;
         show_mjd_time = 0;
         show_unix_time = 0;
         show_gps_time = 0;
         clock_12 = 0;
         return 0;
      }
      else if(d == 's') {   // /ts /tsa /tsh /tsd /tso - set system time
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
                  #else  // __linux__  __MACH__  __FreeBSD__
                     set_time_anytime = 10.0;  // default 10 milliseconds
                  #endif 
                  if(((arg[4] == '=') || (arg[4] == ':')) && arg[5]) {
                     set_time_anytime = atof(&arg[5]);
                  }
                  set_time_anytime = fabs(set_time_anytime);
               }
            }
         }
         else if(e == 'o') ;  // set time once
         else if(strstr(arg, "g")) force_utc_time = 0;  // set to gps time
         else if(strstr(arg, "G")) force_utc_time = 0;  // set to gps time
         else if(e) return c;
         return 0;
      }
#ifdef TEMP_CONTROL
      else if(d == 't') {  // /tt - set control temperature
         do_temp_control = 1;
         if(((e == '=') || (e == ':')) && arg[4]) {
            desired_temp = (DATA_SIZE) atof(&arg[4]);
            if(desired_temp == 0.0) ;
            else if(desired_temp > 50.0) desired_temp = 40.0;
            else if(desired_temp < 10.0) desired_temp = 20.0;
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
         time_flags &= (~TFLAGS_UTC);  // forces a time switch to UTC
         user_set_time_mode = 'u';
         return 0;
      }
      else if((d == 'w') && (e == 'i')) {  // /twi - set timestamp wrap interval in seconds
         if(((f == '=') || (f == ':')) && arg[5]) {
            timestamp_wrap = atof(&arg[5]);
         }
         else timestamp_wrap = 0.0;
      }
      else if(d == 'w') {  // /tw - Windows idle sleep time
         if(e == '=')      idle_sleep = (long) atof(&arg[4]);
         else if(e == ':') idle_sleep = (long) atof(&arg[4]);
         else if(e)        idle_sleep = (long) atof(&arg[3]);
         else              idle_sleep = IDLE_SLEEP;
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
         LLA_SPAN = last_lla_span = 10.0;        // lla plot scale in feet each side of center
         ANGLE_SCALE = DEG_PER_FOOT;  // degrees per foot
         angle_units = "ft";
         if((e == '=') || (e == ':')) {
            if(arg[4]) {
               LLA_SPAN = atof(&arg[4]);
               if(LLA_SPAN <= 0.0) LLA_SPAN = 10.0;
               last_lla_span = LLA_SPAN;
            }
            if(keyboard_cmd) {
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
            if(strstr(&arg[4], "*")) {  // update deltat.dat file with user value
               update_delta_t_file();
            }
            user_delta_t /= (24.0*60.0*60.0);
            user_set_delta_t = 1;
         }
         else user_set_delta_t = 0;
         need_sunrise = 1;
      }
      else if(d == 'd') {   // /ud - damping
         user_set_osc_param |= PARAM_DAMP;
         if(((e == '=') || (e == ':')) && arg[4]) cmd_damp = (DATA_SIZE) atof(&arg[4]);
         else cmd_damp = (DATA_SIZE) 1.000;
      }
      else if(d == 'f') {   // /uf - max freq error threshold
         user_set_osc_param |= PARAM_MAXFREQ;
         if(((e == '=') || (e == ':')) && arg[4]) cmd_maxfreq= (DATA_SIZE) atof(&arg[4]);
         else cmd_maxfreq = (DATA_SIZE) 50.0;
      }
      else if(d == 'g') {   // /ug - gain
         user_set_osc_param |= PARAM_GAIN;
         if(((e == '=') || (e == ':')) && arg[4]) cmd_gain = (DATA_SIZE) atof(&arg[4]);
         else cmd_gain = (DATA_SIZE) 1.400;
         lars_gain = cmd_gain;
      }
      else if(d == 'h') {   // /uh - max allowed dac volts
         if(rcvr_type == X72_RCVR) {
            user_set_osc_param |= PARAM_HOLDOVER;
            if(((e == '=') || (e == ':')) && arg[4]) cmd_holdover = atoi(&arg[4]);
            else cmd_holdover = 120;
         }
         else {
            user_set_osc_param |= PARAM_MAXRANGE;
            if(((e == '=') || (e == ':')) && arg[4]) cmd_maxrange = (DATA_SIZE) atof(&arg[4]);
            else cmd_minv = (DATA_SIZE) 5.000;
         }
      }
      else if(d == 'i') {   // /ui - initial dac volts
         user_set_osc_param |= PARAM_INITV;
         if(((e == '=') || (e == ':')) && arg[4]) cmd_initdac = (DATA_SIZE) atof(&arg[4]);
         else cmd_initdac = (DATA_SIZE) 3.000;
         initial_voltage = cmd_initdac;
         lars_initv = cmd_initdac;
      }
      else if(d == 'j') {   // /uj - jamsync threshold
         user_set_osc_param |= PARAM_JAMSYNC;
         if(((e == '=') || (e == ':')) && arg[4]) cmd_jamsync = (DATA_SIZE) atof(&arg[4]);
         else cmd_jamsync = (DATA_SIZE) 300.0;
      }
      else if(d == 'l') {   // /ul - min allowed dac volts
         user_set_osc_param |= PARAM_MINRANGE;
         if(((e == '=') || (e == ':')) && arg[4]) cmd_minrange = (DATA_SIZE) atof(&arg[4]);
         else cmd_minv = (DATA_SIZE) (-5.000);
      }
      else if(d == 'n') {   // /un - min dac volts
         user_set_osc_param |= PARAM_MINV;
         if(((e == '=') || (e == ':')) && arg[4]) cmd_minv = (DATA_SIZE) atof(&arg[4]);
         else cmd_minv = (DATA_SIZE) 0.000;
      }
      else if(d == 'o') {   // /uo - set UTC leapsecond offset (for receivers that don't send it)
         if(((e == '=') || (e == ':')) && arg[4]) {
            utc_offset = (int) atof(&arg[4]);
            have_utc_ofs = 104;
            user_set_utc_ofs = 5;
         }
      }
      else if(d == 'p') {   // /up - UCCM pullin range
         user_set_osc_param |= PARAM_PULLIN;
         if(((e == '=') || (e == ':')) && arg[4]) cmd_pullin = atoi(&arg[4]);
         else cmd_pullin = 300;
      }
      else if(d == 't') {   // /ut - time constant
         user_set_osc_param |= PARAM_TC;
         if(((e == '=') || (e == ':')) && arg[4]) cmd_tc = (DATA_SIZE) atof(&arg[4]);
         else cmd_tc = 500.0;
      }
      else if(d == 'x') {   // /ux - max dac volts
         user_set_osc_param |= PARAM_MAXV;
         if(((e == '=') || (e == ':')) && arg[4]) cmd_maxv = (DATA_SIZE) atof(&arg[4]);
         else cmd_maxv = 5.000;
      }
      else if(d == 0) {
         user_pause_data = toggle_option(user_pause_data, d);
         pause_data = user_pause_data;
      }
      else return c;
   }
   else if((c == 'v') && (d == 'a')) { // /va - startup in View Auto mode
      need_view_auto = toggle_option(need_view_auto, e);
   }
   else if((c == 'v') && (d == 'f')) { // /vf - startup in full screen mode
      set_not_safe();
      go_fullscreen = 1;
      need_screen_init = 1;
   }
   else if((c == 'v') && (d == 'q')) { // /vq - use scaled vector fonts
      if(((e == '=') || (e == ':')) && arg[4]) {
         use_vc_fonts = 1;
         vc_font_scale = atoi(&arg[4]);
         if(vc_font_scale == 0) {
            use_vc_fonts = 0;
            vc_font_scale = 100;
         }
         else if(vc_font_scale < 50) {
            vc_font_scale = 50;
         }
         else if(vc_font_scale > 500) {
            vc_font_scale = 500;
         }
      }
      else if(rotate_screen) {
         use_vc_fonts = 1;
         vc_font_scale = 75;
      }
      else {
         use_vc_fonts = 1;
         vc_font_scale = 150;
      }
      user_set_font_scale = 1;
      if(keyboard_cmd) {
         init_screen(4242);
      }
   }
   else if((c == 'v') && (d == 'i')) { // /vi - swap black and white (and tweak yellow)
      invert_screen();
   }
   else if((c == 'v') && (d == 'o')) { // /vo - rotate screen
      rotate_screen = toggle_option(rotate_screen, e);
      if(keyboard_cmd) {
         init_screen(8756);
      }
   }
   else if(c == 'v') {   // /v - video screen size - Small,  Medium,  Large,  Xtra large, Huge
//    not_safe = 3;       // so TICC inits properly if /v# on command line
      set_not_safe();
      screen_type = d;
      need_screen_init = 1;
      go_fullscreen = 0;
      kill_deco = 0;
      user_font_size = 0;

      if(keyboard_cmd) {
         if((SCREEN_HEIGHT < TINY_TINY_HEIGHT) || (SCREEN_WIDTH < TINY_TINY_WIDTH)) {  // exit vector font mode if switching out of a TINY_TINY mode
            use_vc_fonts = 0;      // if not 0, draw fonts using vector chars
            vc_font_scale = 100;
         }
      }
      if((keyboard_cmd == 0) && (screen_type == 'd')) {
         if(rotate_screen) vc_font_scale = 60;
         else if(user_set_font_scale == 0) {
            if(plot_digital_clock) vc_font_scale = 60;
            else                   vc_font_scale = 75;
         }
         else vc_font_scale = 75;
      }
      else if((keyboard_cmd == 0) && (screen_type == 'e')) {
         if(rotate_screen) vc_font_scale = 60;
         else              vc_font_scale = 60;
      }


      if(arg[3] == ':') user_font_size = 12;
      if(arg[3] && arg[4]) {  // select font size
         strupr(arg);
         if     (strstr(&arg[4], "S")) user_font_size = 12;
         else if(strstr(&arg[4], "N")) user_font_size = 12;
         else if(strstr(&arg[4], "M")) user_font_size = 14;
         else if(strstr(&arg[4], "L")) user_font_size = 16;
         else if(strstr(&arg[4], "T")) user_font_size = 8;
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

            if(custom_width < MIN_WIDTH) custom_width = MIN_WIDTH;
            if(custom_width > MAX_WIDTH) custom_width = MAX_WIDTH;
            if(custom_height < MIN_HEIGHT) custom_height = MIN_HEIGHT;

            #ifdef WIN_VFX
               custom_height &= (~1);
               custom_width &= (~1);
            #endif  // USE_X11
         }
         else {                         // standard screen size
            return c;
         }
      }
   }
   else if(c == 'w') {   // name of log file to write
      set_not_safe();
      if(d == 'f') {  //  /wf - set watch face type
         if(((e == '=') || (e == ':')) && arg[4]) {
            watch_face = (u08) atof(&arg[4]);
            if(watch_face > 5) watch_face = 0;
         }
         else watch_face = 0;
      }
      else if((d == 'w') || (d == 'l')) {  //  /ww  or /wl - set log file name
         log_mode = "w";
         if(((e == '=') || (e == ':')) && arg[4]) {
            strcpy(log_name, &arg[4]);
         }
         user_set_log |= 0x02;
      }
      else if(d == 'a') { //  /wa - file append mode
         log_mode = "a";
         if(((e == '=') || (e == ':')) && arg[4]) {
            strcpy(log_name, &arg[4]);
         }
         user_set_log |= 0x04;
      }
      else if(d == 'h') { // /wh - toggle ASCII hex dump log mode
         if(log_stream) log_stream = 0;
         else           log_stream = (LOG_HEX_STREAM | LOG_PACKET_ID | LOG_PACKET_START | LOG_SENT_DATA);
      }
      else if(d == 'p') { // /wp - automatic dump file name prefix string
         if(((e == '=') || (e == ':')) && arg[4]) {
            strcpy(dump_prefix, &arg[4]);
         }
//       user_set_log |= 0x08;
      }
      else if(d == 'q') { // /wq - toggle log flush mode
         log_flush_mode = toggle_option(log_flush_mode, e);  // toggle log flush mode
      }
      else if(d == 't') { // /wt - watch face trapazoidal hands (1=filled, 2=hollow)
         if(((e == '=') || (e == ':')) && arg[4]) {
            fancy_hands = atoi(&arg[4]);
         }
         else if(fancy_hands) fancy_hands = 0;
         else fancy_hands = 1;
      }
      else if(arg[2] && arg[3]) { // /w - file write mode
         log_mode = "w";
         if(d) strcpy(log_name, &arg[3]);
         user_set_log |= 0x10;
      }
   }
   else if(c == 'x') {  // x?= set osc pid value
      if(d) {
         if(((e == '=') || (e == ':')) && arg[4]) {
            if(edit_osc_pid_value(d, &arg[4], 0)) return c;
         }
         else  {
            if(edit_osc_pid_value(d, &arg[3], 0)) return c;
         }
      }
      else return c;
   }
   else if(c == 'y') {
      set_not_safe();
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
      if((d == 'k') && e && ((f == '=') || (f == ':')) && arg[5]) {  // /zkx=3 - set fixed center value for plots
         scale_factor = (DATA_SIZE) atof(&arg[5]);
         if     (e == 'h') { plot[HUMIDITY].float_center = 0;  plot[HUMIDITY].plot_center = scale_factor; }
         else if(e == 'p') { plot[PRESSURE].float_center = 0;  plot[PRESSURE].plot_center = scale_factor; }
         else if(e == 't') { plot[TEMP1].float_center = 0;     plot[TEMP1].plot_center    = scale_factor; }
         else if(e == 'u') { plot[TEMP2].float_center = 0;     plot[TEMP2].plot_center    = scale_factor; }
         else if(e == 'y') { plot[ELEVEN].float_center = 0;    plot[ELEVEN].plot_center   = scale_factor; }
         else if(e == 'x') { plot[TWELVE].float_center = 0;    plot[TWELVE].plot_center   = scale_factor; }
         else if(e == 'z') { plot[THIRTEEN].float_center = 0;  plot[THIRTEEN].plot_center = scale_factor; }
         else if(e == 'g') { plot[FOURTEEN].float_center = 0;  plot[FOURTEEN].plot_center = scale_factor; }
         else return c;
      }
      else if(d && ((e == '=') || (e == ':')) && arg[4]) {  // /zd=3 - set fixed center value for plots
         scale_factor = (DATA_SIZE) atof(&arg[4]);
         if     (d == 'd') { plot[DAC].float_center  = 0;  plot[DAC].plot_center  = scale_factor; user_set_dac_float = 1; }
         else if(d == 'o') { plot[OSC].float_center  = 0;  plot[OSC].plot_center  = scale_factor/(DATA_SIZE) 1000.0; user_set_osc_float = 1; }
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
      else if(d) {  // /zd /zpt etc  - toggle graph floating reference mode
         if(d == 'a') {
            auto_scale = toggle_option(auto_scale, e);  // turn off auto scaling
            auto_center = auto_scale;
            set_auto_scale();
         }
         else if(d == 'd') plot[DAC].float_center  = toggle_option(plot[DAC].float_center, e);
         else if(d == 'k') {
            if     (e == 'h') { plot[HUMIDITY].float_center = toggle_option(plot[HUMIDITY].float_center, f);  }
            else if(e == 'p') { plot[PRESSURE].float_center = toggle_option(plot[PRESSURE].float_center, f);  }
            else if(e == 't') { plot[TEMP1].float_center = toggle_option(plot[TEMP1].float_center, f);  }
            else if(e == 'u') { plot[TEMP2].float_center = toggle_option(plot[TEMP2].float_center, f);  }
            else if(e == 'y') { plot[ELEVEN].float_center = toggle_option(plot[ELEVEN].float_center, f); }
            else if(e == 'x') { plot[TWELVE].float_center = toggle_option(plot[TWELVE].float_center, f); }
            else if(e == 'z') { plot[THIRTEEN].float_center = toggle_option(plot[THIRTEEN].float_center, f); }
            else if(e == 'g') { plot[FOURTEEN].float_center = toggle_option(plot[FOURTEEN].float_center, f); }
            else return c;
         }
         else if(d == 'o') plot[OSC].float_center  = toggle_option(plot[OSC].float_center, e); 
         else if(d == 'p') plot[PPS].float_center  = toggle_option(plot[PPS].float_center, e); 
         else if(d == 't') plot[TEMP].float_center = toggle_option(plot[TEMP].float_center, e); 
         else if((d == 's') && e) {  // /zs? - force zoom screen mode (has problems!)
            add_kbd('z');
            add_kbd(e);
         }
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
      set_not_safe();
      ++set_pps_polarity;                  
      user_pps_polarity = 0;
      user_pps_enable = 1;
   }
   else if(c == '-') {  // /- - negative PPS polarity
      set_not_safe();
      ++set_pps_polarity;                  
      user_pps_polarity = 1;
      user_pps_enable = 1;
   }
   else if(c == '^') {  // /^ - toggle OSC signal polarity (referenced to the PPS signal)
      if(d == 'r') {    //^r
         set_not_safe();
         ++set_osc_polarity;                  
         user_osc_polarity = 0;
      }
      else if(d == 'f') { //^f
         set_not_safe();
         ++set_osc_polarity;                  
         user_osc_polarity = 1;
      }
      else {
         set_not_safe();
         ++set_osc_polarity;                  
         user_osc_polarity = toggle_option(user_osc_polarity, d);
      }
   }
   else if(c == '?') {  // help menu
      if(keyboard_cmd == 0) {
         exit_flag = 1;
      }
      return c;
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
   else if(isdigit(c)) {  // /digit - rcvr port number or dotted IP address
      if(keyboard_cmd) kill_com(RCVR_PORT,3017);
      config_port(RCVR_PORT, &arg[1]);
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
