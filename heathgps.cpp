#define EXTERN extern
#include "heather.ch"
//
// Thunderbolt TSIP monitor
//
// Copyright (C) 2008-2016 Mark S. Sims
// Win32 port by John Miles, KE5FX (jmiles@pop.net)
// 
// This file contains the receiver message parsers and GPS receiver control
// code.  It also displays the basic GPS data.
//
//
// minor alarms
//    0x0001 - osc age alarm
//    0x0002 - antenna open
//    0x0004 - antenna open
//    0x0006 - antenna no power
//    0x0008 - no tracked sats
//    0x0010 - external osc control / undisciplined
//    0x0020 - survey mode
//    0x0040 - no saved position
//    0x0080 - leapsecond pending
//    0x0100 - calibration mode   
//    0x0200 - bad saved position
//    0x0400 - EEPROM
//    0x0800 - no almanac
//    0x1000 - PPS2 skipped
//
// critical alarms
//    0x0001 - ROM fail
//    0x0002 - RAM fail
//    0x0004 - power supply fail
//    0x0008 - fpga fail / gps fail
//    0x0010 - ocxo fail / rtc

#define LEAP_THRESH (366/2)  // used to determine valid day count untill leaspsecond

#define ROLL_THRESH 15 
int roll_filter;        // used to filter out transient clock errors

int last_survey;        // used to detect changes in the survey mode

u16 early_end;          // flag set if receiver message ended early
int last_msg;           // the previous TSIP message type

int skip_starloc_time;  // stuff used to filter out bogus STARLOC_RCVR time stamps
int this_pri_tow;
int last_pri_tow;

int old_hh;
int old_min;
int old_sec;

int pt_hh;
int pt_min;
int pt_sec;
int pt_tow;

int st_hh;
int st_min;
int st_sec;


u08 rev_month;          // ebolt info
u08 rev_day;
u16 rev_year;
u08 rev_hour;
u32 ebolt_serno;


unsigned long wakeup_tsip_msg;  // flag set if any tsip message has been received

int tsip_send_count;
u08 try_nortel;       // if flag set, try nortel format osc messages
u08 waking;           // flag set if in wakeup mode
u08 luxor_time_set;   // flag set if luxor time message has been sent
#define LUXOR_ID 0xDC // luxor TSIP packets start with this code


#define UBX_NAV       0x0102
#define UBX_STATUS    0x0103
#define UBX_DOP       0x0104
#define UBX_SPEED     0x0112
#define UBX_GPS       0x0120
#define UBX_UTC       0x0121
#define UBX_CLOCK     0x0122
#define UBX_SVN       0x0130
#define UBX_RAW       0x0210
#define UBX_SFRB      0x0220
#define UBX_NAK       0x0500
#define UBX_ACK       0x0501
#define UBX_DATUM     0x0606
#define UBX_PPS       0x0607
#define UBX_NAV_RATE  0x0608
#define UBX_EEPROM    0x0609
#define UBX_CFG_ANT   0x0613
#define UBX_CFG_TMODE 0x061D 
#define UBX_NAVX5     0x0623 
#define UBX_CFG       0x0624
#define UBX_TP5       0x0631
#define UBX_GNSS      0x063E
#define UBX_VER       0x0A04
#define UBX_MON       0x0A09
#define UBX_LEAP      0x0B02
#define UBX_TP        0x0D01
#define UBX_SVIN      0x0D04 

#define UBX_RAW_RATE  0  // rate to send RAW and SFRB messages (if 0, polled by standard message poll loop)

u08 ubx_txa, ubx_txb; // calculates ubx transmit checksum bytes to send
u08 ubx_rxa, ubx_rxb; // received ubx message calculated checksum bytes
u08 ubx_msg_rxa, ubx_msg_rxb; // checksum bytes from the received ubx message

u08 ubx_rx_class;     // received ubx message class byte
u08 ubx_rx_id;        // received ubx message id byte
u16 ubx_msg_id;
u32 ubx_rx_len;       // length of ubx message payload remaining to be received
u32 ubx_msg_len;
u08 ubx_msg_cka;      // checksum bytes from the received ubx message
u08 ubx_msg_ckb;

double ecef_x;        // used in coversion to/from ecef and lat/lon/alt
double ecef_y;
double ecef_z;
double elat;
double elon;
double ealt;
u32 ubx_svar;         // survey variance



u08 nmea_vfy_cksum;   // NMEA message checksum stuff
u08 nmea_msg_cksum;
u08 nmea_tx_cksum;


u08 moto_vfy_cksum;   // MOTOROLA message checksum and parsing stuff
u08 moto_msg_cksum;
u08 moto_tx_cksum;
int moto_msg_ptr;
int moto_msg_len;
char id_tag[6+1];     // moto vehicle id tag field



// messages from receiver
#define VENUS_SBAS_MSG    0x62
#define VENUS_QZSS_MSG    0x62
#define VENUS_TIME_MSG    0x64 
#define VENUS_JAMMING_MSG 0x64
#define VENUS_BOOT_MSG    0x64
#define VENUS_GNSS_MSG    0x64
#define VENUS_PPS_MSG     0x65
#define VENUS_VERSION_MSG 0x80
#define VENUS_ACK_MSG     0x83
#define VENUS_NAK_MSG     0x84
#define VENUS_RATE_MSG    0x86
#define VENUS_NAV_MSG     0xA8
#define VENUS_MASK_MSG    0xB0
#define VENUS_CABLE_MSG   0xBB
#define VENUS_SURVEY_MSG  0xC2

// message id, message subcode (-1 if none)
#define QUERY_VENUS_VERSION  0x02,0x01
#define QUERY_VENUS_RATE     0x10,-1
#define QUERY_VENUS_MASKS    0x2F,-1
#define QUERY_VENUS_SURVEY   0x44,-1
#define QUERY_VENUS_CABLE    0x46,-1
#define QUERY_VENUS_SBAS     0x62,0x02  // has sub-code
#define QUERY_VENUS_QZSS     0x62,0x04  // has sub-code
#define QUERY_VENUS_BOOT     0x64,0x01  // has sub-code
#define QUERY_VENUS_JAMMING  0x64,0x07  // has sub-code
#define QUERY_VENUS_GNSS     0x64,0x1A  // has sub-code
#define QUERY_VENUS_LEAP     0x64,0x20  // has sub-code
#define QUERY_VENUS_WIDTH    0x65,0x02  // has sub-code
#define QUERY_VENUS_FREQ     0x65,0x04  // has sub-code

// message id, message len
#define SET_VENUS_RESTART    0x01,15
#define VENUS_FACTORY_RESET  0x04,2
#define SET_VENUS_BINARY     0x09,3
#define SET_VENUS_FW         0x0B,0
#define SET_VENUS_POWER      0x0C,3
#define SET_VENUS_NAV_RATE   0x0E,3
#define SET_VENUS_OUT_RATE   0x11,3
#define SET_VENUS_MASKS      0x2B,5
#define SET_VENUS_PINNING    0x39,3
#define SET_VENUS_CABLE      0x45,6
#define SET_VENUS_SURVEY     0x54,31
#define SET_VENUS_QZSS       0x62,5  // has sub-code
#define SET_VENUS_SBAS       0x62,9  // has sub-code
#define SET_VENUS_JAMMING    0x64,4  // has sub-code
#define SET_VENUS_GNSS       0x64,5  // has sub-code
#define SET_VENUS_NAV_MODE   0x64,5  // has sub-code
#define SET_VENUS_FREQ       0x65,7  // has sub-code
#define SET_VENUS_WIDTH      0x65,7  // has sub-code

#define VENUS_SAVE eeprom_save  // 0=SRAM  1=SRAM and flash
#define VENUS_RATE 1            // nav update rate

u08 venus_tx_cksum;   // Venus message checksum and parsing stuff 
u08 venus_vfy_cksum;
u32 venus_rx_len;
u32 venus_msg_len;
u08 venus_msg_id;
int venus_nmea;       // used to process mixed Venus binary and NMEA messages



#define SIRF_TRACK_MSG        4
#define SIRF_SW_VER_MSG       6
#define SIRF_CLOCK_MSG        7
#define SIRF_VIS_MSG         13
#define SIRF_NAV_PARAM_MSG   19
#define SIRF_NAV_MSG         41
#define SIRF_PPS_MSG         52

#define SET_SIRF_NMEA       129
#define GET_SIRF_SW_VER     132
#define SET_SIRF_EL_MASK    139
#define SET_SIRF_AMU_MASK   140
#define SET_STATIC_MODE     143
#define GET_SIRF_NAV_PARAMS 152

u16 sirf_rx_cksum;    // calculated checksum of the received sirf message
u16 sirf_msg_cksum;   // checksum in the received message
u16 sirf_tx_cksum;
u16 sirf_msg_id;
u32 sirf_rx_len;      // length of sirf message payload remaining to be received
u32 sirf_msg_len;



#define send_nvs_start(x) send_tsip_start(x)
#define NVS_GLONASS_PRN 64

u16 nvs_rx_crc;
u16 nvs_msg_crc;
u16 nvs_tx_crc;

int nvs_test;      // receiver test results
double nvs_msecs;  // milliseconds value from time packet


#define ZOD_POSN_MSG         1000
#define ZOD_SNR_MSG          1002
#define ZOD_VIS_MSG          1003
#define ZOD_BEST_MSG         1008  // best user data
#define ZOD_ID_MSG           1011
#define ZOD_SETTINGS_MSG     1012
#define ZOD_RAM_MSG          1050  // ram status
#define ZOD_TRAIM_CONFIG_MSG 1055
#define ZOD_TRAIM_STATUS_MSG 1056
#define ZOD_ACCEL_STATUS_MSG 1092  // hardware accelerator status
#define ZOD_DIAG_MSG         1100
#define ZOD_MARK_MSG         1108
#define ZOD_TEMP_MSG         1110
#define ZOD_POWER_MSG        1117
#define ZOD_EE_WRITE_MSG     1135
#define ZOD_EE_STATUS_MSG    1136
#define ZOD_ACCEL_DATA_MSG   1191  // hardware accelerator data

#define ZOD_SET_EL_MASK      1212
#define ZOD_SET_SAT_MASK     1213
#define ZOD_SET_PLATFORM     1220
#define ZOD_SET_NAV_MODE     1221
#define ZOD_SET_CONFIG       1255
#define ZOD_SET_ACCEL        1292  // enable hardware accelerator
#define ZOD_SET_POWER        1317
#define ZOD_REQ_DIAGS        1300
#define ZOD_REQ_RESTART      1303
#define ZOD_SET_PROTOCOL     1331
#define ZOD_FW_UPDATE        1380

u16 zodiac_tx_cnt;    // used to keep track of even/odd bytes being sent
u16 zodiac_tx_cksum;  // checksum of message payload being sent
u16 zodiac_vfy_cksum; // checksum word from the message
u16 zodiac_rx_cksum;  // calculated checksum of the received message
u16 zodiac_rx_len;    // count of bytes being read from a message payload
u16 zodiac_hdr[4];    // msg id,  msg len, msg flags, msg checksum
u16 zod_seq_num;      // sequence number of sent messages
u16 zod_nav_flags;
u16 zodiac_low_byte;


#define SCPI_TIME_MSG       999
#define SCPI_NULL_MSG       1
#define SCPI_CLS_MSG        2
#define SCPI_TIMEFMT_MSG    3
#define SCPI_ELEV_MSG       10
#define SCPI_EFC_MSG        11
#define SCPI_VIS_MSG        12
#define SCPI_TRACK_MSG      13
#define SCPI_GET_CABLE_MSG  14
#define SCPI_UTC_OFS_MSG    15
#define SCPI_TINT_MSG       16
#define SCPI_STATUS_MSG     17
#define SCPI_PROGRESS_MSG   18
#define SCPI_SURVEY_MSG     19
#define SCPI_RESET_MSG      20
#define SCPI_POSN_MSG       21
#define SCPI_HOLDOVER_MSG   22
#define SCPI_UTC_MSG        23
#define SCPI_TUNC_MSG       24
#define SCPI_TEST_MSG       25
#define SCPI_HARDWARE_MSG   26
#define SCPI_LEAPTIME_MSG   27
#define SCPI_ID_MSG         28
#define SCPI_POS_MSG        29  // stop self survey
#define SCPI_IGNORE_MSG     30
#define SCPI_IGN_COUNT_MSG  31
#define SCPI_INCLUDE_MSG    32
#define SCPI_JAMSYNC_MSG    33
#define SCPI_EDGE_MSG       34
#define SCPI_LIFE_MSG       35
#define SCPI_POWER_MSG      36
#define SCPI_USER_MSG       37
#define SCPI_SET_CABLE_MSG  38
#define SCPI_FFOM_MSG       50  // NORTEL_TYPE
#define SCPI_ANTENNA_MSG    51  // NORTEL_TYPE 
#define SCPI_OPERATION_MSG  52  // NORTEL_TYPE 

#define UCCM_RATE_MSG       100
#define UCCM_STATUS_MSG     101
#define UCCM_LOOP_MSG       102
#define UCCM_EFC_MSG        103
#define UCCM_SETDAC_MSG     104
#define UCCM_LED_MSG        105
#define UCCM_OUTP_MSG       106
#define UCCM_STATE_MSG      107
#define UCCM_SET_PULLIN_MSG 108
#define UCCM_GET_PULLIN_MSG 109


 // scpi_cmd() message codes for STAR 4 receivers
#define STAR_POS_MSG        299  // position and time
#define STAR_ALARM_MSG      200
#define STAR_ALMASK_MSG     201
#define STAR_CONF_MSG       202
#define STAR_INP_TYPE_MSG   203
#define STAR_HBSQ_MSG       204
#define STAR_VIS_MSG        205
#define STAR_TRACK_MSG      206
#define STAR_GPS_TIME_MSG   207
#define STAR_INV_MSG        208
#define STAR_ANGLE_MSG      209
#define STAR_STATE_MSG      210
#define STAR_STATUS_MSG     211
#define STAR_TEMP_MSG       212
#define STAR_TOD_MSG        213
#define STAR_TYPE_MSG       214
#define STAR_ATDC_MSG       215
#define STAR_PERF_MSG       216

#define STAR_TAU_MSG        250  // command output only messages
#define STAR_RESTART_MSG    251
#define STAR_HOLD_MSG       252
#define STAR_CABLE_MSG      253

#define queue_star_cmd(a, b) queue_scpi_cmd(a, b)

int star_line;    // number of lines in multi-line response
int star_msg;     // multi-line message type

int star_tod;            // STAR-4 TOD state
int star_hold_perf;
int star_atdc_on;
int star_atdc_time;
int user_hbsq = (-1);    // HBSQ (holdover squelch time)
int current_hbsq = (-1); // time remaining before squelch activated


char this_acron_cmd;     // the ACRON_RCVR command we just sent
int need_acron_sync;     // flag set if reception attempt is requested




#define SCPI_Q_SIZE 100

struct SCPI_Q {        // queue of pending SCPI messages to send
   char msg[128];
   int  id;
} scpi_q[SCPI_Q_SIZE];

int scpi_in;           // SCPI message send queue pointers
int scpi_out;
int scpi_q_entries;
int scpi_req;          // used to cycle through the info request messages we send each second
unsigned scpi_seq;

int scpi_status;       // the section of the SCPI status screen we are processing
int scpi_life;         // SCPI reveiver lifetime (in hours)

int no_uccm_ref;
int uccm_loop;
int loop_fmt;


extern char *months[];
extern char *dst_list[];


void primary_timing(int get_tsip);
void secondary_timing(int get_tsip);
void request_misc_msg();
void show_filter_status();
void show_operation_info(int why);
void show_lla(int why);
void show_cable_delay();
void update_plot_data();
u08 tsip_byte();

void send_moto_start(char *s);
void send_moto_end();
void send_nmea_end(int do_cksum);
void request_moto_leap(int rate);

void set_ubx_rate(int id, int rate);
void set_ubx_nav_rate(float rate, int align);
void set_ubx_antenna(u08 mode);
void set_ubx_config(int mode, double slat,double slon,double salt);
void set_ubx_amu(float amu);
void set_ubx_pps(int chan, u08 pps_rate, u08 pps_enable,  u08 pps_polarity,  double delay, double pps_delay, int save);
void request_ubx_msg(int id);
void request_ubx_tp5(int chan);

void query_venus_msg(int id, int subid);
void send_venus_start(int id, int len);
void send_venus_end();
void send_venus_save(int allow_eeprom);

void send_zod_start(u16 id, u16 len, u16 flags);
void send_zod_end();
void query_zod_msg(int msg);
void send_zod_seq();
void set_zod_power(int power);
void set_zod_config(int mode, double slat,double slon,double salt, u32 traim, int why);
void set_zod_nav_mode(int flags);
void set_zod_sat_mask(u32 val);

void send_scpi_cmd(char *s, int id);
void queue_scpi_cmd(char *s, int id);
int send_queued_scpi(int why);
void poll_next_scpi();
void poll_next_uccm();

void send_sirf_start(u16 sirf_cmd, int len);
void send_sirf_end();
void query_sirf_msg(int msg);

void send_nmea_string(char *s, int comma);
void send_nmea_cmd(char *s);
void drive_nmea_screen(int system, int why);

void enable_gpsd();

void parse_uccm_loop();

void decode_nvs_message();

void send_acron_cmd(char *s);
void init_acron_time(void);


void utc_to_gps();
void gps_to_utc();
void adjust_rcvr_time(double seconds);

void show_test_warning();
void show_serial_info();
void primary_misc();
void show_param_values();
void update_gps_screen(int why);


//
//
//   Low level serial port handlers
//
//


void debug_stream(unsigned c)
{
static char buf[16];
int i;

   // This routine dumps the serial port data stream to a file

   if((log_stream & 0x02) && raw_file) {  // recording raw receiver data
      fprintf(raw_file, "%c", c);
   }

   if((log_stream & 0x01) == 0) return;
   if(log_file == 0) return;

   // logging hex data to log_file
   if(1 && (rcvr_type == TSIP_RCVR)) {
      ++kol;
      if(last_was_dle) {  
         if(c == DLE) {
            if(kol >= 16) {
               fprintf(log_file, "\n      ");
               kol = (-1);
            }
            fprintf(log_file, "%02X:%02X ", c,c);
         }
         else if(c == ETX) {
            fprintf(log_file, "%02X:%02X ", DLE,(unsigned) (c&0xFF));
         }
         else {
            fprintf(log_file, "\n%02X:%02X", DLE,(unsigned) (c&0xFF));
            if(c == 0x8F) fprintf(log_file, "-");
            else          fprintf(log_file, " ");
            kol = (-1);
         }
         last_was_dle = 0;
      }
      else if(c == DLE) last_was_dle = 1;
      else {
         if(kol >= 16) {
            fprintf(log_file, "\n      ");
            kol = 0;
         }
         fprintf(log_file, "%02X ", (unsigned) (c&0xFF));
      }
   }
   else {
      if((last_was_dle == 2) || (kol >= 16)) {
         for(i=kol; i<16; i++) {
            if((i % 4) == 0) fprintf(log_file, " ");
            fprintf(log_file, "   ");
         }

         for(i=0; i<kol; i++) {
            if((i % 4) == 0) fprintf(log_file, " ");
            if((buf[i] >= ' ') && (buf[i] <= 0x7E)) fprintf(log_file, "%c", buf[i]);
            else fprintf(log_file, ".");
         }
         fprintf(log_file, "\n");
         if(last_was_dle == 2) {
            fprintf(log_file, "\n");
            last_was_dle = 0;
         }

         kol = 0;
      }
      if((kol % 4) == 0) fprintf(log_file, " ");
//    if((kol % 8) == 0) fprintf(log_file, " ");
      fprintf(log_file, "%02X ", (unsigned) (c&0xFF));
      buf[kol] = c;
      ++kol;

      if(last_was_dle) {  // used to pretty print some packet boundaries
         if(c == (unsigned) pkt_end2) last_was_dle = 2;
         else last_was_dle = 0;
      }
      else if(c == (unsigned) pkt_end1) last_was_dle = 1;
   }
}

int get_com_char()
{
u08 c;
int i;

   // Get the next byte from the com device or simulation file
   //
   // Note that if a simulation file is being used that a GPS receiver must
   // be connected and feeding some sort of data stream.  The data is ignored,
   // but is used to pace the data feed rate to Lady Heather.

   c = get_serial_char();
   if(rcvr_type == ACRON_RCVR) c &= 0x7F;
   debug_stream(c);

   if(sim_file) {  // replace GPS receiver data with simulation file data
      i = fread(&c, 1, 1, sim_file);
      if(i <= 0) {
         return 0;  // end of file
      }
   }
   return c;
}


void drain_com_data()
{
   // drain any data from the serial input buffer

   if(sim_file) return;
   if(rcvr_type == NO_RCVR) return;

   Sleep(250);
   while(SERIAL_DATA_AVAILABLE()) {
      update_pwm();
      if((this_msec-last_com_time) > 1000.0) { 
         break;
      }
      get_serial_char();
   }
}

void get_sync_time()
{
int bits;

   // get the approximate time the receiver sent the start bit of the message
   // (this assumes no delay through the operating system buffers)

   if(baud_rate == 0) return;

   bits = 1 + data_bits + stop_bits;
   if(parity) ++bits;
   msg_sync_msec = 0.0 - ((double) bits / (double) baud_rate);

   msg_sync_msec += GetMsecs();
}


void check_utc_ofs(int redraw)
{
   if(have_utc_ofs) return;
   else if((pri_year > 2016) && (utc_offset < 18)) return;
   else if(utc_offset < MIN_UTC_OFFSET) return;

   if((redraw > 0) && (have_utc_ofs == 0)) need_redraw = 2000+redraw;
   if(redraw < 0) have_utc_ofs = (0 - redraw);
   else           have_utc_ofs = redraw;
}

//
//
//   Get basic binary items from the serial port data stream
//
//   Although they are labeled tsip_ routines they are used for all
//   receiver type.
//
//


void get_unkn_message()
{
u08 c;

   // This routine reads and throws away messages from unknown receiver types.

   if(SERIAL_DATA_AVAILABLE() == 0) {
      check_com_timer();
      return;
   }
   else {
      reset_com_timer();
   }

   c = get_com_char();
   if(rcv_error) {      // parity/framing/overrun errors
      rcv_error = 0;
   }

   tsip_wptr = 0;
   tsip_sync = 0;
}



unsigned get_next_tsip()
{
   early_end = 0;
   if(tsip_rptr >= tsip_wptr) {  // no more message to read
      return 0x2000;     // !!!!t
   }

   return tsip_buf[tsip_rptr++];
}

unsigned get_next_tsip_byte()
{
u16 c;

   if(1 && early_end) {  // we are skipping message data
      if(log_comments && (log_stream & 0x01) && log_file) {
         fprintf(log_file, "[%04X]", early_end);
      }
      return early_end;
   }

   c = get_next_tsip();  // get next data stream element
   if(c & 0xFF00) {      // we saw something that was not expected
      if(log_comments && (log_stream & 0x01) && log_file) {
         fprintf(log_file, "<%04X>", c);
      }
      early_end = c;
   }
   return c;
}


u08 tsip_byte()   
{
u16 c;

   c = get_next_tsip_byte();
   if(c & 0xFF00) tsip_error |= 0x08;
   return (u08) (c & 0xFF);
}

u16 tsip_word()   /* get next two byte (word) field from binary message */
{
u08 i;
u08 word[2];
u16 c;

   for(i=0; i<2; i++) {
      c = get_next_tsip_byte();
      if(c & 0xFF00) tsip_error |= 0x10; 
      if(ENDIAN) word[i] = (u08) c;
      else       word[1-i] = (u08) c;
   }
   return * ((u16 *) (void *) &word[0]);
}

u32 tsip_dword()   /* get next dword field from binary message */
{
u08 i;
u08 word[4];
u16 c;

   for(i=0; i<4; i++) {
      c = get_next_tsip_byte();
      if(c & 0xFF00) tsip_error |= 0x20;
      if(ENDIAN) word[i] = (u08) c;
      else       word[3-i] = (u08) c;
   }
   return * ((u32 *) (void *) &word[0]);
}

u48 tsip_tword()   /* get next tword field (48 bit) from binary message */
{
u08 i;
u08 word[3];
u16 c;

   for(i=0; i<3; i++) {
      c = get_next_tsip_byte();
      if(c & 0xFF00) tsip_error |= 0x20;
      if(ENDIAN) word[i] = (u08) c;
      else       word[2-i] = (u08) c;
   }
   return * ((u48 *) (void *) &word[0]);
}

float tsip_single()   /* get next four byte floating number from binary message */
{
u08 i;
u08 word[4];
u16 c;

   for(i=0; i<4; i++) {
      c = get_next_tsip_byte();
      if(c & 0xFF00) tsip_error |= 0x40;
      if(ENDIAN) word[i] = (u08) c;
      else       word[3-i] = (u08) c;
   }
   return * ((float *) (void *) &word[0]);
}

double tsip_double()   /* get next eight byte floating number from binary message */
{
u08 i;
u08 word[8];
u16 c;

   for(i=0; i<8; i++) {
      c = get_next_tsip_byte();
      if(c & 0xFF00) tsip_error |= 0x80;
      if(ENDIAN) word[i] = (u08) c;
      else       word[7-i] = (u08) c;
   }
   return * ((double *) (void *) &word[0]);
}


double tsip_fp80()  
{
u08 i;
u08 x[10];
u08 d[8];
u16 c;
int exponent;
#define u64 LONG_LONG
LONG_LONG mantissa;

   // get FP80 10 byte floating number from binary message (in Intel byte order!)
   // and return it as a standard double 

   for(i=0; i<10; i++) {  // get 10 bytes from the receiver (always in intel byte order)
      c = get_next_tsip_byte();
      if(c & 0xFF00) tsip_error |= 0x80;
      x[i] = (u08) c;
   }

   for(i=0; i<8; i++) d[i] = 0;

   exponent = (((x[9] << 8) | x[8]) & 0x7FFF);
   mantissa =
       ((u64)x[7] << 56) | ((u64)x[6] << 48) | ((u64)x[5] << 40) | ((u64)x[4] << 32) | 
       ((u64)x[3] << 24) | ((u64)x[2] << 16) | ((u64)x[1] << 8) | (u64)x[0];

   d[7] = x[9] & 0x80; // Set sign

   if((exponent == 0x7FFF) || (exponent == 0)) { // Infinite, NaN or denormal 
      if(exponent == 0x7FFF) {  // Infinite or NaN 
         d[7] |= 0x7F;
         d[6] = 0xF0;
      }
      else { // It's denormal. It cannot be represented as double. Translate as zero.
         return 0.0;
      }
   }
   else {  // Normal number
      exponent = exponent - 0x3FFF + 0x03FF; // exponent for double precision.

      if(exponent <= -52) { // too small to represent. Translate as (signed) zero.
         return 0.0;
      }
      else if(exponent < 0) {  // Denormal, exponent bits are already zero here.
      }
      else if(exponent >= 0x7FF) { // Too large to represent. Translate as infinite.
         d[7] |= 0x7F;
         d[6] = 0xF0;
         goto got_it;
      }
      else { // Representable number 
         d[7] |= (exponent & 0x7F0) >> 4;
         d[6] |= (exponent & 0xF) << 4;
      }
   }

   // Translate mantissa

   mantissa >>= 11;

   if(exponent < 0) { // Denormal, further shifting is required here.
      mantissa >>= (-exponent + 1);
   }

   d[0]  = (u08) ((mantissa >> 0) & 0xFF);  // break down the mantissa
   d[1]  = (u08) ((mantissa >> 8) & 0xFF);
   d[2]  = (u08) ((mantissa >> 16) & 0xFF);
   d[3]  = (u08) ((mantissa >> 24) & 0xFF);
   d[4]  = (u08) ((mantissa >> 32) & 0xFF);
   d[5]  = (u08) ((mantissa >> 40) & 0xFF);
   d[6] |= (u08) ((mantissa >> 48) & 0x0F);

   got_it:
   if(!ENDIAN) {  // need to reverse the result byte order
      i = d[0];  d[0] = d[7];  d[7] = i;
      i = d[1];  d[1] = d[6];  d[6] = i;
      i = d[2];  d[2] = d[5];  d[5] = i;
      i = d[3];  d[3] = d[4];  d[4] = i;
   }

   return * ((double *) (void *) &d[0]);
}



u08 tsip_end(u08 report_err)
{
   // handle the end of a TSIP message and check the error flags

   if(tsip_rptr != tsip_wptr) {  // we are not at the end of the message
      tsip_error |= 0x1000;  // !!!!t
   }
   if(tsip_error) {
      ++bad_packets;
      return 1;
   }
   tsip_rptr = tsip_wptr = 0;
   return 0;
}


//
//
//   Send basic items items out the serial port
//
//

u16 nvs_crc_table[256] = {   // calculate the CRC for a NVS message
   0x0000,0x1021,0x2042,0x3063,0x4084,0x50A5,0x60C6,0x70E7,
   0x8108,0x9129,0xA14A,0xB16B,0xC18C,0xD1AD,0xE1CE,0xF1EF,
   0x1231,0x0210,0x3273,0x2252,0x52B5,0x4294,0x72F7,0x62D6,
   0x9339,0x8318,0xB37B,0xA35A,0xD3BD,0xC39C,0xF3FF,0xE3DE,
   0x2462,0x3443,0x0420,0x1401,0x64E6,0x74C7,0x44A4,0x5485,
   0xA56A,0xB54B,0x8528,0x9509,0xE5EE,0xF5CF,0xC5AC,0xD58D,
   0x3653,0x2672,0x1611,0x0630,0x76D7,0x66F6,0x5695,0x46B4,
   0xB75B,0xA77A,0x9719,0x8738,0xF7DF,0xE7FE,0xD79D,0xC7BC,
   0x48C4,0x58E5,0x6886,0x78A7,0x0840,0x1861,0x2802,0x3823,
   0xC9CC,0xD9ED,0xE98E,0xF9AF,0x8948,0x9969,0xA90A,0xB92B,
   0x5AF5,0x4AD4,0x7AB7,0x6A96,0x1A71,0x0A50,0x3A33,0x2A12,
   0xDBFD,0xCBDC,0xFBBF,0xEB9E,0x9B79,0x8B58,0xBB3B,0xAB1A,
   0x6CA6,0x7C87,0x4CE4,0x5CC5,0x2C22,0x3C03,0x0C60,0x1C41,
   0xEDAE,0xFD8F,0xCDEC,0xDDCD,0xAD2A,0xBD0B,0x8D68,0x9D49,
   0x7E97,0x6EB6,0x5ED5,0x4EF4,0x3E13,0x2E32,0x1E51,0x0E70,
   0xFF9F,0xEFBE,0xDFDD,0xCFFC,0xBF1B,0xAF3A,0x9F59,0x8F78,
   0x9188,0x81A9,0xB1CA,0xA1EB,0xD10C,0xC12D,0xF14E,0xE16F,
   0x1080,0x00A1,0x30C2,0x20E3,0x5004,0x4025,0x7046,0x6067,
   0x83B9,0x9398,0xA3FB,0xB3DA,0xC33D,0xD31C,0xE37F,0xF35E,
   0x02B1,0x1290,0x22F3,0x32D2,0x4235,0x5214,0x6277,0x7256,
   0xB5EA,0xA5CB,0x95A8,0x8589,0xF56E,0xE54F,0xD52C,0xC50D,
   0x34E2,0x24C3,0x14A0,0x0481,0x7466,0x6447,0x5424,0x4405,
   0xA7DB,0xB7FA,0x8799,0x97B8,0xE75F,0xF77E,0xC71D,0xD73C,
   0x26D3,0x36F2,0x0691,0x16B0,0x6657,0x7676,0x4615,0x5634,
   0xD94C,0xC96D,0xF90E,0xE92F,0x99C8,0x89E9,0xB98A,0xA9AB,
   0x5844,0x4865,0x7806,0x6827,0x18C0,0x08E1,0x3882,0x28A3,
   0xCB7D,0xDB5C,0xEB3F,0xFB1E,0x8BF9,0x9BD8,0xABBB,0xBB9A,
   0x4A75,0x5A54,0x6A37,0x7A16,0x0AF1,0x1AD0,0x2AB3,0x3A92,
   0xFD2E,0xED0F,0xDD6C,0xCD4D,0xBDAA,0xAD8B,0x9DE8,0x8DC9,
   0x7C26,0x6C07,0x5C64,0x4C45,0x3CA2,0x2C83,0x1CE0,0x0CC1,
   0xEF1F,0xFF3E,0xCF5D,0xDF7C,0xAF9B,0xBFBA,0x8FD9,0x9FF8,
   0x6E17,0x7E36,0x4E55,0x5E74,0x2E93,0x3EB2,0x0ED1,0x1EF0
};

u16 calc_nvs_crc(u16 crc, u08 c)
{
u16 cval;

   cval = ((crc >> 8) ^ c) & 0xFF;

   crc = (crc << 8) ^ nvs_crc_table[cval]; // new CRC
   return crc & 0xFFFF;
}


void send_byte(u08 val)
{
   sendout(val);  
   ++tsip_send_count;

   nmea_tx_cksum ^= val;  // we always update these since we may need to send
   moto_tx_cksum ^= val;  // ... messaages in these formats to wake up other receivers
   venus_tx_cksum ^= val;
   sirf_tx_cksum += val;

   if(rcvr_type == TSIP_RCVR) {
      if(val == DLE) {  // we are sending a TSIP message
         sendout(DLE);  // DLE character must be sent twice
         ++tsip_send_count;
      }
   }
   else if(rcvr_type == NVS_RCVR) {
      nvs_tx_crc = calc_nvs_crc(nvs_tx_crc, val);
      if(val == DLE) {  // we are sending a TSIP message
         nvs_tx_crc = calc_nvs_crc(nvs_tx_crc, DLE);
         sendout(DLE);  // DLE character must be sent twice
         ++tsip_send_count;
      }
   }
   else if(rcvr_type == UBX_RCVR) { // sending ublox ubx message
      ubx_txa += val;    // add byte to message checksums
      ubx_txb += ubx_txa;
   }
   else if(rcvr_type == ZODIAC_RCVR) {
      if(zodiac_tx_cnt & 1) zodiac_tx_cksum += ( (((u16) val) * 256) + zodiac_low_byte );
      else zodiac_low_byte = (u16) val;
      ++zodiac_tx_cnt;
   }
}


void send_word(u16 val)
{
u08 *s;
int i;

   s = (u08 *) (void *) &val;
   for(i=0; i<2; i++) {
      if(ENDIAN) send_byte(s[i]);
      else       send_byte(s[1-i]);
   }
}

void send_dword(u32 val)
{
u08 *s;
int i;

   s = (u08 *) (void *) &val;
   for(i=0; i<4; i++) {
      if(ENDIAN) send_byte(s[i]);
      else       send_byte(s[3-i]);
   }
}

void send_tword(u48 val)
{
u08 *s;
int i;

   s = (u08 *) (void *) &val;
   for(i=0; i<3; i++) {
      if(ENDIAN) send_byte(s[i]);
      else       send_byte(s[2-i]);
   }
}

void send_single(float val)
{
u08 *s;
int i;

   s = (u08 *) (void *) &val;
   for(i=0; i<4; i++) {
      if(ENDIAN) send_byte(s[i]);
      else       send_byte(s[3-i]);
   }
}

void send_double(double val)
{
u08 *s;
int i;

   s = (u08 *) (void *) &val;
   for(i=0; i<8; i++) {
      if(ENDIAN) send_byte(s[i]);
      else       send_byte(s[7-i]);
   }
}

void send_fp80(FP80 val)
{
u08 *s;
int i;

// !!!!!!!!!!!! this does not work on machines that don't support native
//              80-bit floating point formats.  We need to do format conversions.
//              before this can be useful.  Note that Heather currently does
//              not ever call this routine... it is just a placeholder for
//              future functionality.

   s = (u08 *) (void *) &val;
   for(i=0; i<10; i++) {
      if(ENDIAN) send_byte(s[i]);
      else       send_byte(s[9-i]);
   }
}


//
//
//   General support routines
//
//


void primary_misc()
{
   //  This routine is called once a time stamp message has been received.
   //  It handles time conversions, event handling, etc.
   //

   if(time_flags & 0x0001) {  // receiver time is in UTC
      jd_gps = jd_utc + jtime(0,0,utc_offset,0.0);
   }
   else {  // receiver time is in GPS - convert GPS time to UTC time
      jd_utc -= jtime(0,0,utc_offset,0.0);
   }

   jd_tt = jd_gps + utc_delta_t() - jtime(0,0,0,((double) utc_offset)+TSX_TWEAK);  // TT is 51.184 seconds ahead of GPS

   jd_local = jd_utc + time_zone() + jtime(0,0,0,TSX_TWEAK);  // local time (UTC adjusted for time zone)
   gregorian(jd_local);
   this_year = g_year;  // the current year in the local time zone

   have_time = 1;
   if(have_year != year) {    // it's the first valid year we have seen or year has changed
      have_year = year;
      init_dsm();             // tweek month tables for possible leap year
      calc_dst_times(dst_list[dst_area]);  // find daylight savings change times
      calc_dst_times(dst_list[dst_area]);  // do it again to get things right if starting up near DST switch time
   }                                       // and recalculate the seasons, etc

   ticker ^= 1;               // used to flash the alarm on the screen
   adjust_tz(10);             // tweak pri_xxxx time variables for time zone and daylight savings time

   if(hours != last_hours) {
      dst_ofs = dst_offset();
      jd_local = jd_utc + time_zone() + jtime(0,0,0,TSX_TWEAK);
      last_hours = hours;
   }

   have_sun_el = 0;
   if((lat != 0.0) || (lon != 0.0)) {
      sun_el = sun_posn(jd_utc, 0);   // calculate sun position
      have_sun_el = 1;
      log_sun_posn();

      moon_posn(jd_tt);               // calculate moon position
      if(need_posns > 0) --need_posns;
   }

   if(realtime_sun || need_sunrise || ((pri_minutes == 00) && (pri_seconds == SUN_SECOND))) {  // update sunrise time
      if(have_utc_ofs) {
         if(do_moonrise) calc_moonrise();
         else            calc_sunrise(0.0, 20);
         if((lat != 0.0) || (lon != 0.0)) {
            need_sunrise = 0;
         }
      }
   }

   moon_info(jd_utc);  // calculate moon phase, etc

   #ifdef SAT_TRAILS
      update_sat_trails();            // update sat az/el position array
   #endif


// SetDtrLine(1);
   if(set_time_minutely) {  // we do it a xx:xx:06 local time every minute
      if(pri_seconds == SYNC_SECOND) {
         need_time_set();
      }
   }
   else if(set_time_hourly) {  // we do it a xx:05:06 local time every hour
      if((pri_minutes == SYNC_MINUTE) && (pri_seconds == SYNC_SECOND)) {
         need_time_set();
      }
   }
   else if(set_time_daily) {  // we do it a 4:05:06 local time every day
      if((pri_hours == SYNC_HOUR) && (pri_minutes == SYNC_MINUTE) && (pri_seconds == SYNC_SECOND)) {
         need_time_set();
      }
   }

   set_cpu_clock();           // set system clock to receiver time

   #ifdef GREET_STUFF
      if(new_moon_info || (pri_day != last_day)) {   // calculate new moon info for the month
         if(new_moon_info || (pri_seconds == MOON_STUFF)) {
            calc_moons(10);
            last_day = pri_day;
            new_moon_info = 0;
         }
      }
   #endif

   get_delta_t();             // try to get delta_t from external file deltat.dat
   check_end_times();         // exit program at preset time
   silly_clocks();            // do alarm clock and cuckoo clock

   show_satinfo();            // show satellite info
   draw_maps();               // draw maps and analog watch

   if(time_flags != last_time_flags) {     // redo big clock if time settings have changed
      sync_log_file();  // time reference changed,  note it in the log file
      last_time_flags = time_flags;
   }

   if(set_gps_mode || set_utc_mode) {
      if(luxor) {
         if(set_gps_mode) time_flags |= 0x0001;
         else if(set_utc_mode) time_flags &= (~0x0001);
      }

      if(time_flags & 0x0001) {  // we are in UTC mode
         if(set_gps_mode) set_timing_mode(0x00);
         set_utc_mode = set_gps_mode = 0;
      }
      else { // we are in GPS mode
         if(set_utc_mode) set_timing_mode(0x03);
         set_utc_mode = set_gps_mode = 0;
      }
   }

   show_time_info();          // draw primary timing data
   if(configed_mode == 5) {   // secondary timing not available in DGPS mode
      refresh_page();
   }

   if(user_fix_set) {  // command line option enabled fix map display
      config_fix_display();
   }
// SetDtrLine(0);
}


void adjust_rcvr_time(double delta)
{
double jd;

   // adjust the raw receiver clock reading by "delta" seconds
   //
   // NOTE! This routine uses the pri_xxxx time variables.  At the points where
   //       it is called, these variables contain the receiver time (UTC or GPS)
   //       and NOT the local time!

   jd = jdate(pri_year,pri_month,pri_day);
   jd += jtime(pri_hours,pri_minutes,pri_seconds, delta+pri_frac);
   gregorian(jd);  // convert julian date to gregorian

   pri_year = year = g_year;  // update date/time variables to the corrected values
   pri_month = month = g_month;
   pri_day = day = g_day;

   pri_hours = hours = g_hours;
   pri_minutes = minutes = g_minutes;
   pri_seconds = seconds = g_seconds;
   pri_frac = raw_frac = g_frac;
}


void utc_to_gps()
{
   // convert receiver UTC time to GPS time by adding the utc_offset
   adjust_rcvr_time(0.0 + (double) utc_offset);
   time_flags &= (~0x0001);
   have_timing_mode = 1;
}

void gps_to_utc()
{
   // convert receiver GPS time to UTC time by subtracting the utc_offset
   adjust_rcvr_time(0.0 - (double) utc_offset);
   time_flags |= (0x0001);
   have_timing_mode = 1;
}


#define WGS84_A          6378137.0
#define WGS84_E          0.08181919084296430236105472696748
#define eccSquared       (WGS84_E * WGS84_E)
#define eccPrimeSquared  (eccSquared / (1.0 - eccSquared))
#define dScaleFactor     0.9996


void lla_to_ecef(double lat, double lon, double alt)
{   
double clat;
double slat;
double clon;
double slon;
double N;

   // convert lat/lon/alt values (in radians/meters) to ECEF coordinates

   clat = cos(lat);
   slat = sin(lat);
   clon = cos(lon);
   slon = sin(lon);

   N = WGS84_A / sqrt(1.0 - WGS84_E * WGS84_E * slat * slat);

   ecef_x = (N + alt) * clat * clon;
   ecef_y = (N + alt) * clat * slon;
   ecef_z = (N * (1.0 - WGS84_E * WGS84_E) + alt) * slat;
// sprintf(plot_title, "ecef: %f %f %f", ecef_x,ecef_y,ecef_z);
}


void ecef_to_lla(double x, double y, double z)
{
  // convert ECEF coordinate to geocentric lat/lon/alt in radians/meters

  double b = sqrt( (WGS84_A*WGS84_A) * (1.0-(WGS84_E*WGS84_E)) );
  double bsq = (b * b);
  double ep = sqrt( (WGS84_A*WGS84_A - bsq) / bsq);
  double p = sqrt( (x*x) + (y*y) );
  double th = atan2(WGS84_A*z, b*p);

  elon = atan2(y,x);
  elat = atan2( (z + (ep*ep)*b*(sin(th)*sin(th)*sin(th)) ), (p - (WGS84_E*WGS84_E)*WGS84_A*(cos(th)*cos(th)*cos(th))) );

  double N = WGS84_A / ( sqrt(1.0-(WGS84_E*WGS84_E)*(sin(elat)*sin(elat))) );
  ealt = (p / cos(elat)) - N;

  // mod lon to 0-2pi
  elon = fmod(elon,(2.0*PI));
  // correction for altitude near poles left out.  // zork
}

void calc_jd_leap(int why)
{
double days;

   // calculate the Julian date of the next leapsecond and how many days
   // until the event

   days = jd_leap - jd_utc;
   if(days < 0.0) have_jd_leap = have_leap_days = 0;
   else if(days >= ((double) LEAP_THRESH)) have_jd_leap = have_leap_days = 0;
   else have_jd_leap = have_leap_days = why;

   leap_days = (int) days;

//gregorian(jd_leap);
//sprintf(plot_title, "jd_leap %d:  have:%d  %04d/%02d/%02d %02d:%02d:%02d   days:%f", 
//why, have_jd_leap, g_year,g_month,g_day, g_hours,g_minutes,g_seconds, days);
}


void calc_leap_days(double wn_lsf, unsigned short dn, int why)
{
double jd;

   jd = GPS_EPOCH + (wn_lsf * 7.0) + (dn) - jtime(0,0,0,1.0); // Julian day of leapsecond
if(wn_lsf < 1024) jd += rollover / (1024.0 * 7.0 * 24.0*60.0*60.0);
   jd_leap = jd;
   calc_jd_leap(1);
   return;
}


void guess_leap_days()
{
double today;
double days;

   // If receiver has no day of leapsecond info, we attempt to guess it
   // assuming the leapsecond occurs on 30 Jun or 31 Dec.

//sprintf(plot_title, "hld:%d  hli:%d  alarms:%04X", have_leap_days,have_leap_info,minor_alarms & 0x0080);
   if((minor_alarms & 0x0080) == 0) return;  // no leap pending flag
   if(have_leap_days) return;  // receiver provided the leap day

   today = jdate(year,month,day);
   if(month >= 7) {
      days = jdate(year,12,31);
   }
   else {
      days = jdate(year,6,30);
   }
   jd_leap = days + jtime(23,59,59, 0.0);
   have_jd_leap = 2;
//   calc_jd_leap(2);

   days = days-today;
   if(days < 180.0) leap_days = (int) days;
   else leap_days = (-1);
   guessed_leap_days = '?';
}


void record_sig_levels(int prn)
{
   // check the signal level for sat PRN "prn", update the sats' max signal
   // level,  and update the signal level map info

   if(prn < 0) return;
   if(prn > MAX_PRN) return;
   if(lat == 0.0) return;
   if(lon == 0.0) return;

#ifdef SIG_LEVELS
   if(sat[prn].level_msg != 0) { 
      if(sat[prn].sig_level > max_sat_db[prn]) {
         max_sat_db[prn] = sat[prn].sig_level;
      }
      if(sat[prn].sig_level > max_sig_level) {
         max_sig_level = sat[prn].sig_level;
      }

      if(pause_data) ;
      else if(reading_signals == 0) {
         log_signal(sat[prn].azimuth+0.5F, sat[prn].elevation+0.5F, sat[prn].sig_level, amu_mode);
      }
   }
#endif
}

void set_sat_el(int prn, float el)
{
   // record the current elevation of sat PRN "prn" and flag how its elevation
   // if changing

   if(prn < 0) return;
   if(prn > MAX_PRN) return;

   if(sat[prn].elevation == 0.0) sat[prn].el_dir = ' ';
   else if(el == 0.0) sat[prn].el_dir = ' ';
   else if(el > sat[prn].elevation) sat[prn].el_dir = UP_ARROW;
   else if(el < sat[prn].elevation) sat[prn].el_dir = DOWN_ARROW;

   sat[prn].elevation = el;
}


float set_el_level(void)
{
float level;

   // set the satellite elevation level mask to the elevation where the
   // signal levels begin to fall off rapidly.

   #ifdef SIG_LEVELS
      level = (float) good_el_level();
   #else
      level = GOOD_EL_LEVEL;
   #endif

   set_el_mask(level);
   return level;
}


void reset_sat_tracking()
{
int prn;

   // reset all sat tracking flags

   for(prn=0; prn<=MAX_PRN; prn++) {
      sat[prn].tracking = 0;
      sat[prn].level_msg = 0;
   }
}

void reset_sat_health()
{
int prn;

   // reset all satellite health flags to unknown

   for(prn=0; prn<=MAX_PRN; prn++) { 
      sat[prn].health_flag = 0;
   }
}

void update_disable_list(u32 val)
{
int i;

   // transfer data from a 32 bit bitmask of enabled sats to the sat disabled flags

   for(i=1; i<=32; i++) {
      if(val & (1 << (i-1))) sat[i].disabled = 0;
      else                   sat[i].disabled = 1;
   }
}


void saw_ebolt()
{
   // This routine is called if message that is only supported
   // on the Thunderbolt-E is received or when the received satellite count 
   // changes.

   temp_sats = max_sat_count;

   if((small_font == 1) && (SCREEN_HEIGHT < 600) && (text_mode == 0)) {
      temp_sats = 8;
      eofs = 1;
   }
   else if((TEXT_HEIGHT <= 12) || (SCREEN_HEIGHT >= MEDIUM_HEIGHT)) {
      if(rcvr_type != TSIP_RCVR) ;
      else temp_sats = 12;
      eofs = 1;
   }
   else {
      if(rcvr_type != TSIP_RCVR) ;
      else if(plot_stat_info) temp_sats = 11;
      else if(res_t == RES_T) temp_sats = 12;
      else if(res_t)          temp_sats = 14;
      else                    temp_sats = 12;
      eofs = 0;
   }

   // there is no room to show all the sats if the big clock is on
   if(0 && plot_digital_clock && (time_col >= (TEXT_COLS/2))) { 
      temp_sats = 8; //// what if sat_info is in plot area?
   }

   ebolt = 1;
   if(ebolt != last_ebolt) {
      max_sats = temp_sats;
if(res_t == RES_T) max_sats = 12;
else if(res_t) max_sats = 14;
if(max_sats > max_sat_display) max_sats = max_sat_display;
config_sat_rows();
      last_ebolt = ebolt;
config_screen(200);
      erase_screen();
      need_redraw = 2000;
   }
}


void config_sat_rows()
{
   // calculate how many rows on the screen the sat info display will use

   if(sat_cols <= 0) return;

   sat_rows = max_sat_display / sat_cols;
   if(max_sat_display & 1) ++sat_rows;
}

void config_sat_count(int sat_count)
{
   // configure the display for the number of satellite channels

   if(first_key) return;  // delay screen size update until not in a menu
// sprintf(debug_text3, "csc:%d   msd:%d  msc:%d", sat_count, max_sat_display, max_sat_count);
   if(sat_count > max_sat_display) {
      sat_count = max_sat_display;
      have_count = 30;
   }

   if(sat_count > max_sat_count) {
//ppppif(SCREEN_HEIGHT <= 600) plot_digital_clock = 0;
      max_sat_count = sat_count;
      last_ebolt = (-1);
      saw_ebolt();  // !!!! we should do this better  zzzzz
   }

   config_sat_rows();
}


u08 this_const[MAX_PRN+1];
u08 last_const[MAX_PRN+1];

void find_sat_changes()
{
int prn, j;
int sats_changed;

   // see if the satellite constellation has changed
   // Also converts sat_count from number of visible sats to tracked sats.

   for(prn=1; prn<=MAX_PRN; prn++) {  // save old tracking list
      last_const[prn] = this_const[prn];
      this_const[prn] = 0;
   }

   j = 0;
   sat_count = track_count = vis_count = 0;
   sats_changed = 0;
   for(prn=1; prn<=MAX_PRN; prn++) { 
      if(sat[prn].level_msg == 0x00) continue;
//    if(++j > max_sats) break;

      if(sat[prn].tracking > 0) {
         this_const[prn] = 1;
         ++sats_changed;
         ++sat_count;
         ++track_count;
         ++vis_count;
         have_count = 100;
      }
      else if(sat[prn].tracking < 0) {
         ++track_count;
      }
   }

   if(sats_changed) {
      if(sat_count > SAT_COUNT) new_const = (SAT_COUNT | CONST_CHANGE);
      else                      new_const = (sat_count | CONST_CHANGE);
   }
}



void fmt_fp(int v1,int v2,int v3,int v4)
{
unsigned char v[4];

   // debug routine for show showing four integers as a floating point value

   v[3] = v1;
   v[2] = v2;
   v[1] = v3;
   v[0] = v4;
   sprintf(plot_title, "%02X:%02X:%02X:%02X -> %f", v1,v2,v3,v4, *((float *) (void *) &v));
}


void calc_msg_ofs()
{
double rcvr_jd;

   rcvr_jd = jd_utc;
   rcvr_jd *= (24.0*60.0*60.0*1000.0);  // convert Julian to milliseconds

   get_clock_time();   // the system clock time when the message arrived
   clk_jd *= (24.0*60.0*60.0*1000.0);
   if(!fake_time_stamp) msg_ofs = clk_jd - rcvr_jd;  // offset from message time to time the message arrived

   if(measure_jitter) {   // rrrrrr
      sprintf(plot_title, "rcvr-clock: %f  rcvr:%f  clk:%f  /tsx=%f", msg_ofs, fmod(rcvr_jd,10000.0), fmod(clk_jd,10000.0), time_sync_offset);
   }
}



//
//
//   Trimble TSIP protocol handling stuff
//
//


void send_tsip_start(u08 id)
{
   sendout(DLE);
   nvs_tx_crc = 0;
   send_byte(id);
   tsip_send_count = 2;
}

void send_tsip_end()
{
   sendout(DLE);
   eom_flag = 1;   // sending last byte of a TSIP message
   sendout(ETX);
   tsip_send_count += 2;
   Sleep(20); 
}

void send_nvs_end()
{
u16 crc;
if(1) {  // send CRC
   crc = nvs_tx_crc;
   sendout(DLE);
   sendout(0xFF);
   sendout(crc & 0xFF);
   sendout((crc/256) & 0xFF);
}

   sendout(DLE);
   eom_flag = 1;   // sending last byte of a TSIP message to an NVS receiver
   sendout(ETX);
   tsip_send_count += 2;
   Sleep(50);
}

void send_user_cmd(char *s)
{
unsigned i;
int val;
int p;

   // send a user specified (ASCII) command to a receiver
   if(s == 0) return;

   strcpy(last_user_cmd, s);
   if(rcvr_type == ACRON_RCVR) {
      send_acron_cmd(s);
   }
   else if(rcvr_type == GPSD_RCVR) { // send ascii string
      send_nmea_string(s, 0);
//    send_byte(0x0D);
      eom_flag = 1;
      send_byte(0x0A);
   }
   else if(rcvr_type == NMEA_RCVR) {
      if(s[0] == '$') send_nmea_cmd(&s[1]);
      else            send_nmea_cmd(&s[0]);
   }
   else if(rcvr_type == SCPI_RCVR) {
      queue_scpi_cmd(s, SCPI_USER_MSG);
   }
   else if(rcvr_type == UCCM_RCVR) {
      queue_scpi_cmd(s, SCPI_USER_MSG);
   }
   else if(0) {  // send hex string
      strupr(s);
      p = 0;
      val = 0;
      for(i=0; i<strlen(s); i++) {
         if((s[i] >= '0') && (s[i] <= '9')) {
            val = (val*16) + (s[i] - '0');
            p = 1;
         }
         else if((s[i] >= 'A') && (s[i] <= 'F')) {
            val = (val*16) + (s[i] - 'A' + 10);
            p = 1;
         }
         else if(p) {
eom_flag = 1;
            send_byte(val & 0xFF);
            val = 0;
            p = 0;
         }
      }
      if(p) {
         eom_flag = 1;
         send_byte(val & 0xFF);
         val = 0;
         p = 0;
      }
   }
   else { // send ascii string
      send_nmea_string(s, 0);
      send_byte(0x0D);
      eom_flag = 1;
      send_byte(0x0A);
   }
}




void send_ubx_start(u16 ubx_cmd, int len)
{
   // start sending a ublox message
   sendout(0xB5);         // send message start code
   sendout(0x62);
   ubx_txa = ubx_txb = 0; // init transmit checksum bytes

   send_byte((ubx_cmd >> 8) & 0xFF);  // send message class
   send_byte(ubx_cmd & 0xFF);         // send message identifier

   send_byte((len % 256) & 0xFF);     // send message length
   send_byte((len/256) & 0xFF);
}

void send_ubx_end()
{
u08 a,b;

   // end the ublox message
   a = ubx_txa;  // transmitted message checksum
   b = ubx_txb;
   sendout(a);   // send checksum
   eom_flag = 1;
   sendout(b);
Sleep(50);
}



void send_nmea_start()
{
   sendout('$');
   nmea_tx_cksum = 0;
}

void send_nmea_string(char *s, int comma)
{
int i, j;

   j = strlen(s);
   for(i=0; i<j; i++) send_byte((u08) s[i]);
   if(comma) send_byte((u08) ',');
}

void send_nmea_end(int do_cksum)
{
char cksum[10];

   sprintf(cksum, "%02X", nmea_tx_cksum);

   if(do_cksum) {
      sendout('*');
      sendout(cksum[0]);
      sendout(cksum[1]);
   }
   sendout(0x0D);
   eom_flag = 1;
   sendout(0x0A);
Sleep(50);
}

void send_nmea_cmd(char *s)
{
   send_nmea_start();
   send_nmea_string(s, 0);
   if(strchr(s, '*')) send_nmea_end(0);
   else               send_nmea_end(1);
}


//
//
//   Message request and parameter setting messages
//
//

void request_fw_ver()
{
   if(luxor) return;
   if(no_poll) return;

   if(rcvr_type == TSIP_RCVR) {
      send_tsip_start(0x1C);   //!!! ThunderBolt-E, Resolution only
      send_byte(0x01);
      send_tsip_end();
   }
}

void request_unk_ver()
{
   if(luxor) return;
   if(no_poll) return;
   if(rcvr_type != TSIP_RCVR) return;

   send_tsip_start(0x1C);   //!!! ??? receiver only
   send_byte(0x02);
   send_tsip_end();
}

void request_hw_ver()
{
   if(luxor) return;
   if(no_poll) return;

   if(rcvr_type == TSIP_RCVR) {
      send_tsip_start(0x1C);   //!!! ThunderBolt-E, Resolution only
      send_byte(0x03);
      send_tsip_end();
   }
}

void request_c2()
{
   if(luxor) return;
   if(no_poll) return;
   if(rcvr_type != TSIP_RCVR) return;

   send_tsip_start(0xC2);   //!!! ??? receiver
   send_tsip_end();
}

void request_7A_00()
{
   if(luxor) return;
   if(no_poll) return;
   if(rcvr_type != TSIP_RCVR) return;

   send_tsip_start(0x7A);   //!!! ??? receiver
   send_byte(0x00);
   send_tsip_end();
}

void request_rcvr_health()
{
   if(luxor) return;
   if(no_poll) return;
   if(rcvr_type != TSIP_RCVR) return;

   send_tsip_start(0x26);   //!!! ThunderBolt-E only
   send_tsip_end();
}

void request_utc_info()
{
   if(luxor) return;
   if(no_poll) return;

   if(rcvr_type == TSIP_RCVR) {
      if(saw_version == 0) return;
      if(res_t == RES_T) return;   // this message can hose-up this receiver for 3 seconds

      send_tsip_start(0x38);       // get UTC (leapsecond) info from sat data
      send_byte(0x01);
      send_byte(0x05);
      send_byte(0x00);
      send_tsip_end();
   }
   else if(rcvr_type == MOTO_RCVR) {
      request_moto_leap(0);
   }
   else if(rcvr_type == NVS_RCVR) {
      send_nvs_start(0x2B);
      send_nvs_end();
   }
   else if(rcvr_type == UBX_RCVR) {  // get leapsecond info
      request_ubx_msg(UBX_LEAP);
   }
}


void write_all_nvs()
{
   if(read_only) return;
   if(luxor) return;

   if(rcvr_type == TSIP_RCVR) {
      send_tsip_start(0x8E);   //!!! ThunderBolt-E only
      send_byte(0x26);
      send_tsip_end();
   }
}


void restart_gpsdo()
{
   // re-awaken a GPSDO after a reset
   if(saw_nortel || saw_ntpx || (rcvr_type == MOTO_RCVR) || (rcvr_type == NVS_RCVR)) {
      vidstr(EDIT_ROW+4, EDIT_COL, PROMPT_COLOR, "There is a 20 second reset delay...");
      refresh_page();
      Sleep(20000);
      SetDtrLine(1);
      if(NO_SCPI_BREAK && (rcvr_type == SCPI_RCVR)) ; else 
      SendBreak();
      Sleep(2000);
   }
}

void restart_zodiac_rcvr()
{
   // re-init ZODIAC receiver message format after a restart
   if(force_mode_change) {     // leave the receiver in Motorola mode
      rcvr_type = MOTO_RCVR;
      config_rcvr_type(0);
      force_mode_change = 0;
   }
   else {
      send_moto_start("Wb");  // set Jupiter-T receiver protocol to Zodiac mode
      send_byte(1);
      send_moto_end();
      Sleep(1000);

      tsip_sync = 0;
      tsip_wptr = 0;
      tsip_rptr = 0;
      init_messages(70);
   }

   Sleep(250);
}



#define UBX_EE_IO  0x0001
#define UBX_EE_MSG 0x0002
#define UBX_EE_INF 0x0004
#define UBX_EE_NAV 0x0008
#define UBX_EE_TP  0x0010
#define UBX_EE_INV 0x0000   // 0x0200  not supported on LEA-5
#define UBX_EE_ANT 0x0400

#define UBX_EE_ALL (UBX_EE_IO | UBX_EE_MSG | UBX_EE_INF | UBX_EE_NAV | UBX_EE_TP | UBX_EE_INV | UBX_EE_ANT)

void update_ubx_config(u32 clear, u32 save, u32 load)
{
#define EE_MSG_SIZE 12

   send_ubx_start(UBX_EEPROM, EE_MSG_SIZE);
   send_dword(clear);   // reset params from permanent config
   send_dword(save);    // save params to EEPROM
   send_dword(load);    // load config from EEPROM

   if(EE_MSG_SIZE == 13) {  // storage device select
      send_byte(0x17);      // all devices
   }
   send_ubx_end();

   if(save) {
      BEEP(200);
      ++ee_write_count;
      Sleep(1000);
   }
}

void venus_restart(int mode)
{
s16 val;

   send_venus_start(SET_VENUS_RESTART);
   send_byte(mode);
   send_word(pri_year);
   send_byte(pri_month);
   send_byte(pri_day);
   send_byte(pri_hours);
   send_byte(pri_minutes);
   send_byte(pri_seconds);

   val = (s16) (lat * 180.0 / PI * 100.0);
   send_word(val);

   val = (s16) (lon * 180.0 / PI * 100.0);
   send_word(val);

   val = (s16) alt;
   send_word(val);

   send_venus_end();

   Sleep(5000);
   init_messages(50);
}


void reset_leap_flags()
{
   leaped = 0;
   leap_pending = 0;
   minor_alarms &= (~0x0080);
}

void enable_moto_nmea()
{
//send_moto_start("Eq"); // testing ASCII position message
//send_byte(0);
//send_moto_end();
//return;

   // put Motorola receiver into NMEA mode
   send_moto_start("Ci");
   send_byte(1);
   send_moto_end();
   Sleep(500);

   baud_rate = 4800;
   data_bits = 8;
   parity = NO_PAR;
   stop_bits = 1;
   init_com();
   Sleep(100);

   rcvr_type = NMEA_RCVR;
   send_nmea_cmd("$PMOTG,GGA,0001");
   send_nmea_cmd("$PMOTG,GLL,0001");
   send_nmea_cmd("$PMOTG,GSA,0001");
   send_nmea_cmd("$PMOTG,GSV,0001");
   send_nmea_cmd("$PMOTG,RMC,0001");
   send_nmea_cmd("$PMOTG,VTG,0001");
   send_nmea_cmd("$PMOTG,ZDA,0001");

// auto_detect();
   config_rcvr_type(0);
}

void enable_moto_binary()
{
   // put Motorola receiver into BINARY mode
   rcvr_type = NMEA_RCVR;
   send_nmea_cmd("$PMOTG,FOR,0");
   Sleep(500);

   baud_rate = 9600;
   data_bits = 8;
   parity = NO_PAR;
   stop_bits = 1;
   init_com();
   Sleep(100);

   rcvr_type = MOTO_RCVR;
   auto_detect();
}


void request_cold_reset()
{
   if(read_only) return;
   if(no_poll) return;

   reset_leap_flags();

   if(luxor) {
      send_tsip_start(LUXOR_ID);
      send_byte(0x10);
      send_tsip_end();
      Sleep(100);

      set_luxor_time();
   }
   else if(rcvr_type == ACRON_RCVR) {
      need_acron_sync = 'h';
   }
   else if(rcvr_type == NVS_RCVR) {
      send_nvs_start(0x01);
      send_byte(0x00); 
      send_byte(0x01); 
      send_byte(0x21); 
      send_byte(0x01); 
      send_byte(0x00); 
      send_byte(0x00); 
      send_nvs_end();

      Sleep(5000);
      init_messages(5445);
   }
   else if(rcvr_type == SCPI_RCVR) {
// !!! should adjust com_timeout
//    queue_scpi_cmd(":SYST:PON", SCPI_POWER_MSG);   // no NORTEL_TYPE
      queue_scpi_cmd("*TST?", SCPI_TEST_MSG);  // causes a system reset
   }
   else if(rcvr_type == UCCM_RCVR) {
// !!! should adjust com_timeout
      queue_scpi_cmd("SYST:PON", SCPI_POWER_MSG);
      queue_scpi_cmd("SYST:STAT?", UCCM_STATUS_MSG);
   }
   else if(rcvr_type == STAR_RCVR) {
      queue_star_cmd("RESTART(C);", STAR_RESTART_MSG);
   }
   else if(rcvr_type == TSIP_RCVR) {
      send_tsip_start(0x1E);
      send_byte('K');
      send_tsip_end();
      restart_gpsdo();
   }
   else if(rcvr_type == UBX_RCVR) {
      update_ubx_config(0x0000, 0x0000, UBX_EE_ALL);
      Sleep(1000);
      restart_gpsdo();
      init_messages(20);
      rcvr_reset = 1;
   }
   else if(rcvr_type == VENUS_RCVR) {
      venus_restart(3);
   }
   else if(rcvr_type == ZODIAC_RCVR) {
      send_zod_start(ZOD_REQ_RESTART, 8, 0);
      send_zod_seq();
      send_word(0x8003);
      send_zod_end();

      Sleep(5000);
      restart_zodiac_rcvr();
   }
}

void request_warm_reset()
{
   if(read_only) return;
   if(no_poll) return;

   reset_leap_flags();

   if(luxor) {
      set_luxor_time();
   }
   else if(rcvr_type == ACRON_RCVR) {
      need_acron_sync = 'h';
   }
   else if(rcvr_type == MOTO_RCVR) {
      if(force_mode_change) {
         send_moto_start("Wb");  // set Resolution-T TEP receiver protocol to TSIP mode
         send_moto_end();
         Sleep(1000);

         tsip_sync = 0;
         tsip_wptr = 0;
         tsip_rptr = 0;
         rcvr_type = TSIP_RCVR;
auto_detect();
//         init_messages(70);
      }
   }
   else if(rcvr_type == NVS_RCVR) {
      send_nvs_start(0x01);
      send_byte(0x00); 
      send_byte(0x01); 
      send_byte(0x21); 
      send_byte(0x01); 
      send_byte(0x00); 
      send_byte(0x01); 
      send_nvs_end();

      Sleep(5000);
      init_messages(5454);
   }
   else if(rcvr_type == SIRF_RCVR) {
      if(force_mode_change) {
         send_sirf_start(SET_SIRF_NMEA, 23+1);  // set to NMEA protocol
         send_byte(2);    // don't change debug state
         send_byte(1); send_byte(1);  // GGA
         send_byte(0); send_byte(1);  // GLL
         send_byte(1); send_byte(1);  // GSA
         send_byte(5); send_byte(1);  // GSV
         send_byte(1); send_byte(1);  // RMC
         send_byte(0); send_byte(1);  // VTG
         send_byte(0); send_byte(1);  // MSS
         send_byte(0); send_byte(1);  // EPE
         send_byte(0); send_byte(1);  // ZDA
         send_byte(0); send_byte(1);  // rsvd
         send_word(baud_rate);  // 9600 bps
         send_sirf_end();
         Sleep(4000);

         tsip_sync = 0;
         tsip_wptr = 0;
         tsip_rptr = 0;
         rcvr_type = NMEA_RCVR;
auto_detect();
      }
   }
   else if(rcvr_type == STAR_RCVR) {
      queue_star_cmd("RESTART(W);", STAR_RESTART_MSG);
   }
   else if(rcvr_type == TSIP_RCVR) {
      send_tsip_start(0x25);
      send_tsip_end();
      restart_gpsdo();
   }
   else if(rcvr_type == UBX_RCVR) {
      update_ubx_config(0x0000, 0x0000, UBX_EE_ALL);  // load config from eeprom

      if(force_mode_change) {
         Sleep(500);
         rcvr_type = NMEA_RCVR;
         sprintf(out, "PUBX,41,1,0003,0002,%d,0", baud_rate);  // 9600 bps ublox NMEA output mode (port 1)
         send_nmea_cmd(out);
//       sprintf(out, "PUBX,41,2,0003,0002,%d,0", baud_rate);  // 9600 bps ublox NMEA output mode (port 2)
//       send_nmea_cmd(out);
         Sleep(500);
         force_mode_change = 0;
         auto_detect();
      }

      init_messages(21);
      rcvr_reset = 1;
   }
   else if(rcvr_type == VENUS_RCVR) {
      if(force_mode_change) {
         set_venus_mode(1);
         Sleep(1000);

         tsip_sync = 0;
         tsip_wptr = 0;
         tsip_rptr = 0;
         rcvr_type = NMEA_RCVR;
auto_detect();
      }
      else {
         venus_restart(2);
      }
   }
   else if(rcvr_type == ZODIAC_RCVR) {
      send_zod_start(ZOD_REQ_RESTART, 8, 0);
      send_zod_seq();
      send_word(0x0001);
      send_zod_end();

      Sleep(5000);
      restart_zodiac_rcvr();
   }
}

void request_factory_reset()
{
   if(read_only) return;
   if(no_poll) return;

   reset_leap_flags();

   if(luxor) {
      send_tsip_start(LUXOR_ID);   // erase EEPROM config
      send_byte(0x08);
      send_tsip_end();
      Sleep(100);

      request_cold_reset();
   }
   else if(rcvr_type == ACRON_RCVR) {
      need_acron_sync = 'h';
   }
   else if(rcvr_type == MOTO_RCVR) {
      send_moto_start("Cf");
      send_moto_end();
      rcvr_reset = 1;
//    restart_gpsdo();
//    Sleep(2000);
   }
   else if(rcvr_type == NVS_RCVR) {
      request_cold_reset();
   }
   else if(rcvr_type == SCPI_RCVR) {
// !!! should adjust com_timeout
      if(scpi_type == NORTEL_TYPE) queue_scpi_cmd("SYST:PRESET", SCPI_RESET_MSG);
      else                         queue_scpi_cmd(":SYST:PRES", SCPI_RESET_MSG);
   }
   else if(rcvr_type == STAR_RCVR) {
      queue_star_cmd("RESTART(C);", STAR_RESTART_MSG);
   }
   else if(rcvr_type == UCCM_RCVR) {
// !!! should adjust com_timeout
      queue_scpi_cmd("SYST:PRES", SCPI_RESET_MSG);
      queue_scpi_cmd("SYST:STAT?", UCCM_STATUS_MSG);
   }
   else if(rcvr_type == TSIP_RCVR) {
      send_tsip_start(0x1E);
      send_byte('F');
      send_tsip_end();
   }
   else if(rcvr_type == UBX_RCVR) {
      update_ubx_config(UBX_EE_ALL, 0x0000, UBX_EE_ALL);  // reset eeprom then re-load config from eeprom
      rcvr_reset = 1;
      Sleep(1000);
      init_messages(22);
   }
   else if(rcvr_type == VENUS_RCVR) {
      send_venus_start(VENUS_FACTORY_RESET);
      send_byte(1);
      send_venus_end();
      Sleep(5000);
      init_messages(22);
   }
   else if(rcvr_type == ZODIAC_RCVR) {
      send_zod_start(ZOD_REQ_RESTART, 8, 0);
      send_zod_seq();
      send_word(0x8007);
      send_zod_end();

      Sleep(10000);
      restart_zodiac_rcvr();
   }
}


void request_manuf_params()
{
   if(luxor) return;
   if(no_poll) return;
   if(rcvr_type != TSIP_RCVR) return;

   send_tsip_start(0x8E);
   send_byte(0x41);
   send_tsip_end();
}

void request_prodn_params()
{
   if(luxor) return;
   if(no_poll) return;
   if(rcvr_type != TSIP_RCVR) return;

   send_tsip_start(0x8E);
   send_byte(0x42);
   send_tsip_end();
}

void request_version()
{
   if(luxor) return;
   if(no_poll) return;

   if(rcvr_type == GPSD_RCVR) {
      send_user_cmd("?DEVICES;");
      send_user_cmd("?VERSION;");
   }
   else if(rcvr_type == MOTO_RCVR) {
      if(!saw_version) {  // this message is rather long, only get the info once
         send_moto_start("Cj");  // request receiver ID string
         send_moto_end();
      }
   }
   else if(rcvr_type == NVS_RCVR) {
      send_nvs_start(0x1B);
      send_nvs_end();
   }
   else if(rcvr_type == SIRF_RCVR) {
      query_sirf_msg(GET_SIRF_SW_VER);
   }
   else if(rcvr_type == TSIP_RCVR) {
      send_tsip_start(0x1F);
      if(0) {  // requests (undocumented) extended receiver configuration info
         send_byte(0x01);
         send_byte(0x00);
      }
      send_tsip_end();

      request_manuf_params();
      request_prodn_params();
      request_fw_ver();      // !!! testing ThunderBolt-E, Resolution messages
      request_hw_ver();
   }
   else if(rcvr_type == UBX_RCVR) {
       request_ubx_msg(UBX_VER);
   }
   else if(rcvr_type == VENUS_RCVR) {
      query_venus_msg(QUERY_VENUS_VERSION);
   }
   else if(rcvr_type == ZODIAC_RCVR) {
      query_zod_msg(ZOD_ID_MSG);
      query_zod_msg(ZOD_DIAG_MSG);
   }
}

void request_self_tests(void)
{
   if(luxor) return;
   if(no_poll) return;
   if(read_only) return;

// !!! should adjust com_timeout
   if(rcvr_type == MOTO_RCVR) {
      send_moto_start("Ia");
      send_moto_end();

      send_moto_start("Fa");
      send_moto_end();

      send_moto_start("Ca");
      send_moto_end();
      show_test_warning();
   }
   else if(rcvr_type == NVS_RCVR) {
      send_nvs_start(0x11);
      send_byte(0x00);
      send_nvs_end();
   }
   else if(rcvr_type == SCPI_RCVR) { 
      queue_scpi_cmd("*TST?", SCPI_TEST_MSG);  // NORTEL_TYPE?
      show_test_warning();
   }
   else if(rcvr_type == UCCM_RCVR) {  // not for UCCM? only UCCMP? uuuuuu
      queue_scpi_cmd("*TST?", SCPI_TEST_MSG);
   }
   else if(rcvr_type == ZODIAC_RCVR) {
      send_zod_start(ZOD_REQ_DIAGS, 8, 0);
      send_zod_seq();
      send_word(0);
      send_zod_end();
      show_test_warning();

      Sleep(10000);
      restart_zodiac_rcvr();
   }
}

void request_almanac(u08 prn)
{
   if(luxor) return;
   if(no_poll) return;

   if(rcvr_type == NVS_RCVR) { 
      if(prn < NVS_GLONASS_PRN) {  // GPS
         send_nvs_start(0x20);
         send_byte(0x01);
         send_byte(prn);
         send_nvs_end();
      }
      else { // GLONASS
         send_nvs_start(0x20);
         send_byte(0x02);
         send_byte(prn-NVS_GLONASS_PRN);
         send_nvs_end();
      }
   }
   else if(rcvr_type == TSIP_RCVR) {
      send_tsip_start(0x20);
      send_byte(prn);
      send_tsip_end();
   }
}

void request_sat_list()
{
   if(luxor) return;
   if(no_poll) return;

   if(rcvr_type == MOTO_RCVR) {
      send_moto_start("Bb");  // sat visibility data
      send_byte(0);
      send_moto_end();
   }
   else if(rcvr_type == NVS_RCVR) {
      send_nvs_start(0x24);
      send_nvs_end();
   }
   else if(rcvr_type == TSIP_RCVR) {
      send_tsip_start(0x24);
      send_tsip_end();
   }
   else if(rcvr_type == UBX_RCVR) {
      request_ubx_msg(UBX_DOP);
   }
   else if(rcvr_type == ZODIAC_RCVR) {
      query_zod_msg(ZOD_SNR_MSG);
      query_zod_msg(ZOD_VIS_MSG);
//query_zod_msg(ZOD_TRAIM_CONFIG_MSG);  //zork
   }
}

void request_gps_time()
{
   if(luxor) return;
   if(no_poll) return;

   if(rcvr_type == TSIP_RCVR) {
      send_tsip_start(0x21);
      send_tsip_end();
   }
}

void request_sig_levels()
{
   if(luxor) return;
   if(no_poll) return;

   if(rcvr_type == TSIP_RCVR) {
      send_tsip_start(0x27);
      send_tsip_end();
   }
   else if(rcvr_type == UBX_RCVR) {
      request_ubx_msg(UBX_SVN); // sat info
      request_ubx_msg(UBX_RAW); // doppler, etc
   }
}

void request_alm_health()
{
   if(no_poll) return;
   if(res_t) return;
   if(luxor) return;

   if(rcvr_type == TSIP_RCVR) {
      send_tsip_start(0x29);
      send_tsip_end();
   }
}

void set_xyz(float x, float y, float z)
{
   if(read_only) return;
   if(luxor) return;

   if(rcvr_type == TSIP_RCVR) {
      send_tsip_start(0x31);
      send_single(x);
      send_single(y);
      send_single(z);
      send_tsip_end();
   }
}

void set_ref_input(int ref)
{
   // ref=0: GPS
   // ref=1: AUX PPS input

   if(read_only) return;
   if(luxor) return;

   if(rcvr_type == STAR_RCVR) {
      if     (ref == 0) queue_star_cmd("INPUT_TYPE(1)=GPS;", STAR_INP_TYPE_MSG);       // gps input
      else if(ref == 1) queue_star_cmd("INPUT_TYPE(1)=AUX;", STAR_INP_TYPE_MSG);  // pps input
   }
   request_rcvr_config(7788);
}


void set_venus_survey(int mode, double slat,double slon,double salt)
{
   send_venus_start(SET_VENUS_SURVEY);
   send_byte(mode);       // 0=timing pvt mode  1=do survey  3=posn hold mode
   send_dword(do_survey);
   send_dword(100);
   send_double(slat*180.0/PI);  // lla only used for position hold mode
   send_double(slon*180.0/PI);
   send_single((float)salt);
   send_venus_save(1);
}

void set_filter_factor(float val)
{
   if(read_only) return;

   if(rcvr_type == NVS_RCVR) {
      send_nvs_start(0x0D);
      send_byte(0x04);
      send_single(val);
      send_nvs_end();
   }
}


void set_lla(double slat, double slon, double salt)
{
S32 v1,v2,v3;
char c;
double x;
int deg,mins;
double secs;
char lats[256];
char lons[256];

// sprintf(debug_text2, "set lla:%.9lf %.9lf %.9lf", slat*180.0/PI, slon*180.0/PI, salt);

   if(read_only) return;

   need_posns = 1;
   need_sunrise = 1;
   if(luxor) {
      lat = slat;
      lon = slon;
      alt = salt;
   }
   else if((rcvr_type == NO_RCVR) || (rcvr_type == ACRON_RCVR)) {
      lat = slat;
      lon = slon;
      alt = salt;
   }
   else if(rcvr_type == NVS_RCVR) {
      set_rcvr_mode(RCVR_MODE_HOLD);  // position hold mode

//    send_nvs_start(0x1D);  // could be 0x0F 0x03 
//    send_byte(0x07);
      send_nvs_start(0x0F);  // could be 0x1D 0x07 
      send_byte(0x03);
      send_double(slat);
      send_double(slon);
      send_double(salt);
      send_nvs_end();
   }
   else if(rcvr_type == MOTO_RCVR) {
      v1 = (S32) ((slat * 180.0 / PI) * 3600000.0); // radians to milliarcseconds
      v2 = (S32) ((slon * 180.0 / PI) * 3600000.0); // radians to milliarcseconds
      v3 = (S32) (salt * 100.0);  // convert meters to cm

      if(0) {
         set_rcvr_mode(RCVR_MODE_HOLD);  // position hold mode

         send_moto_start("Ad");  // !!!! (moto_chans != 12)
         send_dword(v1);
         send_moto_end();
         send_moto_start("Ae");
         send_dword(v2);
         send_moto_end();
         send_moto_start("Af");
         send_dword(v3);
         send_byte(0);
         send_moto_end();
      }

      set_rcvr_mode(RCVR_MODE_3D);  // enter 3D mode so we can set position hold lla
      Sleep(2000);

      send_moto_start("As");   // set LLA
      send_dword(v1);
      send_dword(v2);
      send_dword(v3);
      send_byte(0);
      send_moto_end();

      Sleep(1000);
      set_rcvr_mode(RCVR_MODE_HOLD);  // position hold mode
   }
   else if((rcvr_type == SCPI_RCVR) || (rcvr_type == UCCM_RCVR)) {
      x = slat * 180.0 / PI;
      if(x >= 0.0) c = 'N';
      else {
         c = 'S';
         x = 0.0 - x;
      }

      deg = (int) x;

      x -= (double) deg; 
      x *= 60.0;         // x = decimal degrees
      mins = (int) x;

      x -= (double) mins; 
      secs = (x * 60);   // x = seconds
      sprintf(lats, "%c,%d,%d,%f,", c,deg,mins,secs);


      x = slon * 180.0 / PI;
      if(x >= 0.0) c = 'E';
      else {
         c = 'W';
         x = 0.0 - x;
      }

      deg = (int) x;

      x -= (double) deg; 
      x *= 60.0;         // x = decimal degrees
      mins = (int) x;

      x -= (double) mins; 
      secs = (x * 60);   // x = seconds
      sprintf(lons, "%c,%d,%d,%f,%f", c,deg,mins,secs,salt);

      if(rcvr_type == UCCM_RCVR) {
         sprintf(out, "GPS:POS %s%s", lats,lons);
         queue_scpi_cmd(out, SCPI_NULL_MSG);  // !!!!!
         Sleep(1000);
         queue_scpi_cmd("SYST:STAT?", UCCM_STATUS_MSG);
      }
      else if(scpi_type == NORTEL_TYPE) {
         sprintf(out, "GPS:POSITION %s%s", lats,lons);
         queue_scpi_cmd(out, SCPI_NULL_MSG);  // !!!!!
      }
      else {  // !!! should adjust com_timeout
         sprintf(out, ":PTIM:GPS:POS %s%s", lats,lons);
         queue_scpi_cmd(out, SCPI_NULL_MSG);  // !!!!!
      }
   }
   else if(rcvr_type == TSIP_RCVR) {
      send_tsip_start(0x32);
      send_single((float) slat);
      send_single((float) slon);
      send_single((float) salt);
      send_tsip_end();
   }
   else if(rcvr_type == UBX_RCVR) {
      set_ubx_config(2, slat,slon,salt);
   }
   else if(rcvr_type == VENUS_RCVR) {
      set_venus_survey(2, slat,slon,salt);
   }
   else if(rcvr_type == ZODIAC_RCVR) {
      set_zod_config(6, slat,slon,salt, traim_threshold, 1);
   }
}

void request_io_options()
{
   if(luxor) return;
   if(no_poll) return;

   if(rcvr_type == TSIP_RCVR) {
      send_tsip_start(0x35);
      send_tsip_end();
   }
}

void set_io_options(u08 posn, u08 vel, u08 timing, u08 aux)
{
   if(read_only) return;
   if(luxor) return;

   if(rcvr_type == TSIP_RCVR) {
      send_tsip_start(0x35);
      send_byte(posn);
      send_byte(vel);
      send_byte(timing);
      send_byte(aux);
      send_tsip_end();
   }
}

void request_last_posn()
{
   if(luxor) return;
   if(no_poll) return;

   if(rcvr_type == TSIP_RCVR) {
      send_tsip_start(0x37);
      send_tsip_end();
   }
}

void request_system_data(u08 mode, u08 prn)
{
   if(luxor) return;
   if(no_poll) return;
   if(rcvr_type != TSIP_RCVR) return;
   if(res_t == RES_T) return;   // this message can hose-up this receiver for 3 seconds

   send_tsip_start(0x38);
   send_byte(0x01);
   send_byte(mode);
   send_byte(prn);
   send_tsip_end();
}



void set_health_config(u08 mode, u08 prn)
{
   // mode 1: enable sat
   // mode 2: disable sat
   // mode 3: get enable/disable status of all sats
   // mode 4: set sat healthy
   // mode 5: set sat unhealthy
   // mode 6: get health of all sats

   if(read_only) return;
   if(luxor) return;
   if((res_t && (res_t != RES_T))) return;
   if(rcvr_type != TSIP_RCVR) return;

   send_tsip_start(0x39);
   send_byte(mode);
   send_byte(prn);
   send_tsip_end();
}


void request_sat_health()
{
   if(luxor) return;
   if(no_poll) return;

   if(rcvr_type == MOTO_RCVR) {
      send_moto_start("Am");
      send_byte(0xFF);
      send_dword(0);
      send_moto_end();
   }
   else if(rcvr_type == NVS_RCVR) {
      send_nvs_start(0x35);
      send_nvs_end();
   }
   else if(rcvr_type == TSIP_RCVR) {
      set_health_config(3, 0x00);  // request enable/disable status of all sats
      set_health_config(6, 0x00);  // request heed/ignore status of all sats
   }
   else if(rcvr_type == ZODIAC_RCVR) {
      query_zod_msg(ZOD_SETTINGS_MSG);
   }
}

void set_single_sat(u08 prn)
{
u32 val;

   if(read_only) return;
   if(luxor) return;
   if(prn > 32) return;

   if(rcvr_type == MOTO_RCVR) {
      single_sat_prn = prn;
      send_moto_start("Am");
      send_byte(0);
      if(prn) {
         val = 1 << (prn-1);
         val = (~val);
      }
      else {
         val = 0;
      }
      send_dword(val);
      send_moto_end();

      reset_sat_tracking();
      request_sat_health();
   }
   else if((rcvr_type == SCPI_RCVR) && (scpi_type == NORTEL_TYPE)) { 
      single_sat_prn = prn;
      if(prn) {
         queue_scpi_cmd("GPS:SAT:TRAC:IGNORE 0", SCPI_IGNORE_MSG);
         sprintf(out,"GPS:SAT:TRAC:INCLUDE %d", prn);
         queue_scpi_cmd(out,  SCPI_INCLUDE_MSG);
      }
      else {
         queue_scpi_cmd("GPS:SAT:TRAC:INCLUDE 1,2,3,4,5,6,7,8,9,10,11,12,13", SCPI_INCLUDE_MSG);
         queue_scpi_cmd("GPS:SAT:TRAC:INCLUDE 14,15,16,17,18,19,20,21,22,23", SCPI_INCLUDE_MSG);
         queue_scpi_cmd("GPS:SAT:TRAC:INCLUDE 24,25,26,27,28,29,30,31,32", SCPI_INCLUDE_MSG);
         queue_scpi_cmd("SYNC:IMMEDIATE", SCPI_JAMSYNC_MSG);
      }
      queue_scpi_cmd(":GPS:SAT:TRAC:IGN?", SCPI_IGNORE_MSG);
   }
   else if(rcvr_type == SCPI_RCVR) {  // no NORTEL_TYPE
      single_sat_prn = prn;
      if(prn) {
         queue_scpi_cmd(":GPS:SAT:TRAC:IGN:ALL", SCPI_IGNORE_MSG);
         sprintf(out,":GPS:SAT:TRAC:INCL %d", prn);
         queue_scpi_cmd(out,  SCPI_INCLUDE_MSG);
      }
      else {
         queue_scpi_cmd(":GPS:SAT:TRAC:IGN:NONE", SCPI_IGNORE_MSG);
         queue_scpi_cmd(":GPS:SAT:TRAC:INCL:ALL", SCPI_INCLUDE_MSG);
         queue_scpi_cmd(":SYNC:HOLD:REC:INIT", SCPI_HOLDOVER_MSG);    //pppp? ggggggg
      }
      queue_scpi_cmd(":GPS:SAT:TRAC:IGN?", SCPI_IGNORE_MSG);
   }
   else if(rcvr_type == UCCM_RCVR) {
      single_sat_prn = prn;
      if(prn) {
         queue_scpi_cmd("GPS:SAT:TRAC:IGN:ALL", SCPI_IGNORE_MSG);
         sprintf(out,"GPS:SAT:TRAC:INCL %d", prn);
         queue_scpi_cmd(out,  SCPI_INCLUDE_MSG);
      }
      else {
         queue_scpi_cmd("GPS:SAT:TRAC:INCL:ALL", SCPI_INCLUDE_MSG);
// queue_scpi_cmd("SYNC:HOLD:REC:INIT", SCPI_HOLDOVER_MSG);    //pppp? ggggggg
      }
      queue_scpi_cmd("GPS:SAT:TRAC:IGN?", SCPI_IGNORE_MSG);
   }
   else if(rcvr_type == TSIP_RCVR) {
      single_sat_prn = prn;
      send_tsip_start(0x34);
      send_byte(prn);
      send_tsip_end();
   }
   else if(rcvr_type == ZODIAC_RCVR) {
      single_sat_prn = prn;
      if(prn) {
         val = 1 << (prn-1);
      }
      else {
         val = 0xFFFFFFFF;
      }

      set_zod_sat_mask(val);
   }

}

void exclude_sat(u08 prn)
{
u32 val;
int i;

   if(read_only) return;
   if(luxor) return;

   if(rcvr_type == MOTO_RCVR) {
      if(prn > 32) return;
      send_moto_start("Am");
      send_byte(0);
      if(prn) {
         val = 1 << (prn-1);     // val = bitmask of sats to exclude
         val |= (~sats_enabled);
      }
      else {
         val = 0;
      }
      send_dword(val);
      send_moto_end();

      reset_sat_tracking();
      request_sat_health();
   }
   else if(rcvr_type == NVS_RCVR) {
      if(prn) {  // exclude sat
         send_nvs_start(0x12);
         if(prn >= NVS_GLONASS_PRN) {
            send_byte(0x02);  // GLONASS
            send_byte(prn-NVS_GLONASS_PRN);
         }
         else {
            send_byte(0x01);  // GPS
            send_byte(prn);
         }
         send_byte(0x02);  // disable
         send_nvs_end();
      }
      else {  // enable all excluded sats
         for(i=1; i<=32; i++) {  // GPS sats
            if(sat[i].disabled) {
               send_nvs_start(0x12);
               send_byte(0x01);  // GPS
               send_byte(i);
               send_byte(0x01);  // enable
               send_nvs_end();
            }
         }
         for(i=NVS_GLONASS_PRN+1; i<=NVS_GLONASS_PRN+24; i++) { // GLONASS sats
            if(sat[i].disabled) {
               send_nvs_start(0x12);
               send_byte(0x02);  // GLONASS
               send_byte(i-NVS_GLONASS_PRN);
               send_byte(0x01);  // enable
               send_nvs_end();
            }
         }
      }
   }
   else if(rcvr_type == SCPI_RCVR) {
      if(prn > 32) return;
      if(prn) {
         if(scpi_type == NORTEL_TYPE) sprintf(out, "GPS:SAT:TRAC:IGNORE %d", prn);
         else                         sprintf(out, ":GPS:SAT:TRAC:IGN %d", prn);
         queue_scpi_cmd(out, SCPI_IGNORE_MSG);
      }
      else if(scpi_type == NORTEL_TYPE) {
         queue_scpi_cmd("GPS:SAT:TRAC:INCLUDE 1,2,3,4,5,6,7,8,9,10,11,12,13", SCPI_INCLUDE_MSG);
         queue_scpi_cmd("GPS:SAT:TRAC:INCLUDE 14,15,16,17,18,19,20,21,22,23", SCPI_INCLUDE_MSG);
         queue_scpi_cmd("GPS:SAT:TRAC:INCLUDE 24,25,26,27,28,29,30,31,32", SCPI_INCLUDE_MSG);
         queue_scpi_cmd("SYNC:IMMEDIATE", SCPI_JAMSYNC_MSG);
      }
      else {
         queue_scpi_cmd(":GPS:SAT:TRAC:IGN:NONE", SCPI_IGNORE_MSG);
         queue_scpi_cmd(":GPS:SAT:TRAC:INCL:ALL", SCPI_INCLUDE_MSG);
         queue_scpi_cmd(":SYNC:HOLD:REC:INIT", SCPI_HOLDOVER_MSG);
      }

      if(scpi_type == NORTEL_TYPE) {
         queue_scpi_cmd("GPS:SAT:TRAC:IGNORE?", SCPI_IGNORE_MSG);
         queue_scpi_cmd("GPS:SAT:TRAC:INCLUDE?", SCPI_INCLUDE_MSG);
      }
      else queue_scpi_cmd(":GPS:SAT:TRAC:IGN?", SCPI_IGNORE_MSG);
   }
   else if(rcvr_type == UCCM_RCVR) {
      if(prn > 32) return;
      if(prn) {
         queue_scpi_cmd("GPS:SAT:TRAC:INCL:ALL", SCPI_INCLUDE_MSG);
         sprintf(out, "GPS:SAT:TRAC:IGN %d", prn);
         queue_scpi_cmd(out, SCPI_IGNORE_MSG);
      }
      else {
         queue_scpi_cmd("GPS:SAT:TRAC:INCL:ALL", SCPI_INCLUDE_MSG);
// uuuu  queue_scpi_cmd("SYNC:HOLD:REC:INIT", SCPI_HOLDOVER_MSG);
      }
      queue_scpi_cmd("GPS:SAT:TRAC:IGN?", SCPI_IGNORE_MSG);
   }
   else if(rcvr_type == TSIP_RCVR) {
      if(prn) set_health_config(2, prn);
      else    set_health_config(1, 0);
      request_sat_health();  // request enable/disable and health status of all sats
   }
   else if(rcvr_type == ZODIAC_RCVR) {
      if(prn > 32) return;
      if(prn) {
         val = 1 << (prn-1);   // val is bitmap of sats to exclude
         val = (~val);         // not it's bitmap of sats to include
         val &= sats_enabled;
      }
      else {
         val = 0xFFFFFFFF;
      }

      set_zod_sat_mask(val);
   }
}



void request_last_raw(u08 prn)
{
   if(luxor) return;
   if(no_poll) return;

   if(rcvr_type == TSIP_RCVR) {
      send_tsip_start(0x3A);
      send_byte(prn);
      send_tsip_end();
   }
}

void request_eph_status(u08 prn)
{
   if(res_t) return;
   if(luxor) return;
   if(no_poll) return;
   if(rcvr_type != TSIP_RCVR) return;

   send_tsip_start(0x3B);
   send_byte(prn);
   send_tsip_end();
}

void request_sat_status(u08 prn)
{
   if(luxor) return;
   if(no_poll) return;
   if(rcvr_type != TSIP_RCVR) return;

   send_tsip_start(0x3C);
   send_byte(prn);
   send_tsip_end();
}

void request_eeprom_status()
{
   if(luxor) return;
   if(no_poll) return;
   if(rcvr_type != TSIP_RCVR) return;
   if(res_t) return;          // !!! Thunderbolt only
   if(saw_ntpx) return;

   send_tsip_start(0x3F);
   send_byte(0x11);
   send_tsip_end();
}


void request_filter_config()
{
   if(luxor) return;
   if(no_poll) return;

   if(rcvr_type == TSIP_RCVR) {
      send_tsip_start(0x70);
      send_tsip_end();
   }
   else if(rcvr_type == MOTO_RCVR) {
      send_moto_start("AN");  // marine (velocity) filter
      send_byte(0xFF);
      send_moto_end();

      send_moto_start("AQ");  // position filter
      send_byte(0xFF);
      send_moto_end();

      send_moto_start("Aq");  // ionosphere corrections
      send_byte(0xFF);
      send_moto_end();

      send_moto_start("Ar");  // fix type
      send_byte(0xFF);
      send_moto_end();
   }
}

void set_filter_config(u08 pv, u08 stat, u08 alt, u08 kalman, u08 marine, int save)
{
u08 val;

   if(read_only) return;
   if(luxor) return;

   if(rcvr_type == TSIP_RCVR) {
      send_tsip_start(0x70);
      send_byte(pv);
      send_byte(stat);
      send_byte(alt);
      send_byte(kalman); // rsvd on ThunderBolt,  kalman on ThunderBolt-E
      send_tsip_end();
      if(0 && save) save_segment(255, 20);
   }
   else if(1 && (rcvr_type == MOTO_RCVR)) { // marine velocity filter - !!!! code in heathui.cpp will need to be changed to do this
      if((marine >= 10) && (marine <= 100)) {
         send_moto_start("AN");
         send_byte(marine);
         send_moto_end();
      }

      send_moto_start("AQ");
      send_byte(pv);
      send_moto_end();

      val = 0x00;
      if(stat) val |= 0x01;
      if(alt) val |= 0x02;
      send_moto_start("Aq");
      send_byte(val);
      send_moto_end();
   }
}


void request_rcvr_config(int why)
{
   if(luxor) return;
   if(no_poll) return;

   if(rcvr_type == MOTO_RCVR) {
      send_moto_start("Ag"); // elevation mask
      send_byte(0xFF);
      send_moto_end();

      send_moto_start("At"); // position hold mode
      send_byte(0xFF);
      send_moto_end();
   }
   else if(rcvr_type == NVS_RCVR) {
      send_nvs_start(0x0D);  // sig level / elevation masks
      send_byte(0x03);
      send_nvs_end();

      send_nvs_start(0x0D);  // solution filtration factor
      send_byte(0x04);
      send_nvs_end();

      send_nvs_start(0x11);  // self test results
      send_byte(0x00);
      send_nvs_end();

      send_nvs_start(0x1D);  // survey mode info
      send_byte(0x00);
      send_nvs_end();

      send_nvs_start(0xD7);  // get nav rate (1 Hz)
      send_byte(0x02);
      send_nvs_end();
   }
   else if(rcvr_type == SIRF_RCVR) {
      query_sirf_msg(GET_SIRF_NAV_PARAMS);
   }
   else if(rcvr_type == STAR_RCVR) {
      if(star_line) return;  // dont do this in the middle of a multi-line response
      queue_star_cmd("CONF;", STAR_CONF_MSG);
//    queue_star_cmd("MASK_ANGLE;", STAR_ANGLE_MSG);
if(debug_file) fprintf(debug_file, "request_rcvr_config(%d)\n", why);
//    queue_star_cmd("INPUT_TYPE(1);", STAR_INP_TYPE_MSG);  // !!! this command is not valid
   }
   else if(rcvr_type == TSIP_RCVR) {
      send_tsip_start(0xBB); // receiver config packet
      send_byte(0x00);
      send_tsip_end();
   }
   else if(rcvr_type == UBX_RCVR) {
      request_ubx_msg(UBX_MON);
      request_ubx_msg(UBX_CFG);
      request_ubx_msg(UBX_CFG_ANT);
      request_ubx_msg(UBX_NAVX5);
      request_ubx_msg(UBX_NAV_RATE);
   }
   else if(rcvr_type == UCCM_RCVR) {
      queue_scpi_cmd("OUTP:STAT?", UCCM_STATE_MSG);
      queue_scpi_cmd("PULLINRANGE?", UCCM_GET_PULLIN_MSG);
   }
   else if(rcvr_type == VENUS_RCVR) {
      query_venus_msg(QUERY_VENUS_MASKS);    // get el/sig mask values
      query_venus_msg(QUERY_VENUS_BOOT);     // get boot status
      query_venus_msg(QUERY_VENUS_JAMMING);  // get jamming mode
      query_venus_msg(QUERY_VENUS_CABLE);    // get cable delay
      query_venus_msg(QUERY_VENUS_LEAP);     // get UTC offset
      query_venus_msg(QUERY_VENUS_RATE);     // get nav rate
   }
   else if(rcvr_type == ZODIAC_RCVR) {
      query_zod_msg(ZOD_SETTINGS_MSG);
   }

   request_gnss_system();
}


void set_config(u08 mode, u08 dynamics, float elev, float amu, float pdop_mask, float pdop_switch, u08 foliage)
{
int i;
int el;
int flag;

   //!!! values of FF and -1.0 say to not change that value
   if(luxor) return;
   if(read_only == 0) {     // it's OK to change the receiver settings
      if(rcvr_type == MOTO_RCVR) {
         if(elev >= 0.0) {
            el = (int) (((double) elev * 180.0 / PI) + 0.50);
            if(el > 89) el = 89;
            else if(el< 0) el = 0;
            send_moto_start("Ag");
            send_byte(el);
            send_moto_end();
         }

         if(mode != 0xFF) { // set receiver operating mode   zzzzz
            if(mode == 7) {  // position hold mode
               send_moto_start("Gd");
               send_byte(1);           // position hold
               send_moto_end();

               send_moto_start("At");
               send_byte(1);           // position hold
               send_moto_end();

               send_moto_start("Av");
               send_byte(1);           // altitude hold
               send_moto_end();
            }
            else if(mode == 3) {
               send_moto_start("Gd");
               send_byte(2);           // 2D mode
               send_moto_end();
            }
            else {      // 3d positioning mode
               send_moto_start("Gd");
               send_byte(0);           // 3D mode
               send_moto_end();

               send_moto_start("At");
               send_byte(0);           // 3D mode
               send_moto_end();

               send_moto_start("Av");
               send_byte(0);           // altitude hold
               send_moto_end();
            }
         }
      }
      else if(rcvr_type == NVS_RCVR) {
         if((elev >= 0.0F) || (amu >= 0.0F)) {
            if(elev < 0.0F) elev = el_mask;
            else            elev = (elev * 180.0F / (float) PI) + 0.50F;
            if(elev < 0.0F) elev = 0.0F;
            else if(elev > 90.0F) elev = 90.0F;

            if(amu < 0.0F) amu = amu_mask;
            send_nvs_start(0x0D);
            send_byte(0x03); // set elev and snr mask
            send_byte((u08) elev);
            send_byte((u08) amu);
            send_word(0x0000);    // leave position RMS error filter (meters) alone
            send_nvs_end();
         }

         if(mode != 0xFF) {  // 
            send_nvs_start(0x1D);
            send_byte(0);
            if(mode == 7) {  // posn hold
               send_byte(1);
            }
            else {  // 3D mode
               send_byte(0);
            }
            send_nvs_end();
         }
      }
      else if(rcvr_type == SCPI_RCVR) {
         if(elev >= 0.0) {
            el = (int) (((double) elev * 180.0 / PI) + 0.50);
            if(el > 90) el = 89;
            else if(el < 0) el = 0;

            if(scpi_type == NORTEL_TYPE) {
               sprintf(out, "GPS:SAT:TRAC:EMANGLE %d", el);
               queue_scpi_cmd(out, SCPI_ELEV_MSG);  
               queue_scpi_cmd("GPS:SAT:TRAC:EMANGLE?", SCPI_ELEV_MSG);
            }
            else {
               sprintf(out, ":PTIM:GPS:EMAN %d", el);
               queue_scpi_cmd(out, SCPI_ELEV_MSG);  
               queue_scpi_cmd(":PTIM:GPS:EMAN?", SCPI_ELEV_MSG);
            }
         }
      }
      else if(rcvr_type == SIRF_RCVR) {
         if(elev >= 0.0) {
            el = (int) (((double) elev * 180.0 / PI) + 0.50);
            send_sirf_start(SET_SIRF_EL_MASK, 4+1);
            send_word((int) (el * 10.0F));
            send_word((int) (el * 10.0F));
            send_sirf_end();
         }

         if(amu >= 0.0) {
            send_sirf_start(SET_SIRF_AMU_MASK, 2+1);
            send_byte((int) amu);
            send_byte((int) amu);
            send_sirf_end();
         }
      }
      else if(rcvr_type == STAR_RCVR) {
         if(elev >= 0.0) {
            el = (int) (((double) elev * 180.0 / PI) + 0.50);
            if(el > 90) el = 90;
            else if(el < 5) el = 5;

            sprintf(out, "MASK_ANGLE=%d;", el);
            queue_star_cmd(out, STAR_ANGLE_MSG);
         }
      }
      else if(rcvr_type == UCCM_RCVR) {
         if(elev >= 0.0) {
            el = (int) (((double) elev * 180.0 / PI) + 0.50);
            if(el > 90) el = 89;
            else if(el < 0) el = 0;

            sprintf(out, "GPS:SAT:TRAC:EMAN %d", el);
            queue_scpi_cmd(out, SCPI_ELEV_MSG);  
            queue_scpi_cmd("GPS:SAT:TRAC:EMAN?", SCPI_ELEV_MSG);
         }
      }
      else if(rcvr_type == TSIP_RCVR) {
         send_tsip_start(0xBB);
         send_byte(0x00);
         send_byte(mode);          // 2:  4=full posn  7=overdetermined clock
         send_byte(0xFF);          // 3:  ignored
         send_byte(dynamics);      // 4:  4=stationary dynamics  
         send_byte(0xFF);          // 5:  ignored
         send_single(elev);        // 6:  0.175F = 10 degree mask angle
         send_single(amu);         // 10: 4.0 amu mask
         send_single(pdop_mask);   // 14: 8.0 pdop mask
         send_single(pdop_switch); // 18: 6.0 pdop switch
         send_byte(0xFF) ;         // 22: 0xFF ignored
         send_byte(foliage) ;      // 23: 1 = some foliage //  saw_icm: anti-jam mode0=disabled 1=enabled
         for(i=24; i<=40; i++) {
            // byte 28: sat type mask for OD clock mode for RES360, ICM
            //          0x01 GPS
            //          0x02 GLONASS
            //          0x08 Beidou
            //          0x10 Galileo
            //          0x20 QZSS 
            flag = 0xFF;
            if((i == 28) && (saw_icm || (res_t == RES_T_360))) {  // !!!!! should set this gnss_mask
               if(have_gnss_mask) {
                  flag = 0x00;
                  if(gnss_mask & GPS) flag |= 0x01;
                  if(gnss_mask & GLONASS) flag |= 0x02;
                  if(gnss_mask & BEIDOU) flag |= 0x08;
                  if(gnss_mask & GALILEO) flag |= 0x10;
                  if(gnss_mask & QZSS) flag |= 0x20;
                  if(flag == 0) flag = 0xFF;
               }
            }
            send_byte(flag);
         }

         send_tsip_end();
      }
      else if(rcvr_type == UBX_RCVR) {
         if(elev >= 0.0) {
            el = (int) (((double) elev * 180.0 / PI) + 0.50);
            if(el > 90) el = 89;
            else if(el < 0) el = 0;

            send_ubx_start(UBX_CFG, 36);
            send_word(0x0002);  // update elevation field
            for(i=2; i<12; i++) send_byte(0);
            send_byte(el);
            for(i=13; i<36; i++) send_byte(0);
            send_ubx_end();
         }

         if(mode != 0xFF) {  // zorky
            if(mode == 7) {  // posn hold
               set_ubx_config(2, lat,lon,alt);  // hold:current posn
            }
            else {  // 3D mode
               set_ubx_config(0, lat,lon,alt);
            }
         }

         if(amu >= 0.0) {
            set_ubx_amu(amu);
            request_ubx_msg(UBX_NAVX5);
         }

         request_ubx_msg(UBX_CFG);
      }
      else if(rcvr_type == VENUS_RCVR) {
         if((elev >= 0.0F) || (amu >= 0.0F)) {
            if(elev < 0.0F) elev = el_mask;
            else            elev = (elev * 180.0F / (float) PI) + 0.50F;
            if(elev < 3.0F) elev = 3.0F;
            else if(elev > 85.0F) elev = 85.0F;

            if(amu < 0.0F) amu = amu_mask;

            send_venus_start(SET_VENUS_MASKS);
            send_byte(1);
            send_byte((u08) elev);
            send_byte((u08) amu);
            send_venus_save(1);
         }

         if(foliage != 0xFF) {
            send_venus_start(SET_VENUS_JAMMING);
            send_byte(6);
            send_byte(foliage);
            send_venus_save(1);
         }

         if(mode != 0xFF) {  // zorky
            if(mode == 7) {  // posn hold
               set_venus_survey(2, lat,lon,alt);  // hold:current posn
            }
            else {  // 3D mode
               set_venus_survey(0, lat,lon,alt);
            }
         }

         query_venus_msg(QUERY_VENUS_SURVEY);
      }
      else if(rcvr_type == ZODIAC_RCVR) {  
         if(elev >= 0.0) {
            elev = elev * 1000.0F;
            send_zod_start(ZOD_SET_EL_MASK, 8, 0);
            send_zod_seq();
            send_word((u16) elev);
            send_zod_end();
         }

         if(mode != 0xFF) {
            if(mode == 7) {  // posn hold
               set_zod_config(5, 0.0,0.0,0.0, traim_threshold, 2);  // hold:current posn
            }
            else {  // 3D mode
               set_zod_config(1, 0.0,0.0,0.0, traim_threshold, 3);   // normal nav mode
            }
         }
      }
   }

   have_rcvr_mode = 0;
   request_filter_config();
   request_sat_list();
   request_rcvr_config(1);

   if(mode != 0xFF) {
      if(mode == 7) fix_mode = 0;
      else          fix_mode = 1;
      configed_mode = mode;
   }
   else if(read_only == 0) {
      save_segment(3, 21);    // save receiver config
   }
}

void request_serial_config(u08 port)
{
   if(luxor) return;
   if(no_poll) return;
   if(rcvr_type != TSIP_RCVR) return;

   send_tsip_start(0xBC);
   send_byte(port);
   send_tsip_end();
}

void set_com_protocol(u08 in_prot, u08 out_prot)
{
   if(read_only) return;
   if(rcvr_type != TSIP_RCVR) return;

   send_tsip_start(0xBC);
   send_byte(0xFF);   // current com port (0=main com)
   send_byte(7);      // 9600 baud
   send_byte(7);      // 9600 baud
   send_byte(3);      // 8-bit
   send_byte(0);      // no parity
   send_byte(0);      // 1 stop bit
   send_byte(0);      // rsvd (flow control)
   send_byte(in_prot);   // 2=tsip 4=nmea
   send_byte(out_prot);  // 
   send_byte(0);      // rsvd
   send_tsip_end();
}

void set_serial_config()
{
   //!!!
   if(luxor) return;
}


void request_datum()
{
   if(luxor) return;
   if(no_poll) return;

   if(rcvr_type == MOTO_RCVR) {
      send_moto_start("Ao");
      send_byte(0xFF);
      send_moto_end();
   }
   else if(rcvr_type == TSIP_RCVR) {
      send_tsip_start(0x8E);
      send_byte(0x15);
      send_tsip_end();
   }
   else if(rcvr_type == UBX_RCVR) {
      request_ubx_msg(UBX_DATUM);
   }
}

void revert_segment(u08 segment)
{
   if(read_only) return;
   if(luxor) return;
   if(rcvr_type != TSIP_RCVR) return;

   send_tsip_start(0x8E);
   send_byte(0x45);
   send_byte(segment);
   send_tsip_end();
}


void request_osc_sense()
{
   if(luxor) return;
   if(no_poll) return;
   if(res_t) return;
   if(rcvr_type != TSIP_RCVR) return;

   send_tsip_start(0x8E);  // not available on ThunderBolt-E or early ThunderBolts
   send_byte(0xA1);
   send_tsip_end();
}


void request_pps_info()
{
   if(luxor) return;
   if(no_poll) return;

   if(rcvr_type == MOTO_RCVR) {
      send_moto_start("Az");     // antenna cable delay
      send_dword(0xFFFFFFFF);
      send_moto_end();

      send_moto_start("Ay");     // pps delay
      send_dword(0xFFFFFFFF);
      send_moto_end();

      send_moto_start("Gc");
      send_byte(0xFF);
      send_moto_end();
   }
   else if(rcvr_type == NVS_RCVR) {
      send_nvs_start(0xD7);  // get pps info
      send_byte(0x05);
      send_nvs_end();

      send_nvs_start(0xD7);  // get cable delay
      send_byte(0x06);
      send_nvs_end();

      send_nvs_start(0xD7);  // get traim mode, 2D OK, single sat OK
      send_byte(0x07);
      send_nvs_end();
   }
   else if(rcvr_type == TSIP_RCVR) {
      send_tsip_start(0x8E);
      send_byte(0x4A);
      send_tsip_end();

      request_osc_sense();
   }
   else if(rcvr_type == UBX_RCVR) {
      if(!saw_ubx_tp5) request_ubx_msg(UBX_PPS);
      request_ubx_tp5(0);
      request_ubx_tp5(1);
   }
   else if(rcvr_type == VENUS_RCVR) {
      query_venus_msg(QUERY_VENUS_WIDTH);
      query_venus_msg(QUERY_VENUS_FREQ);
   }
}

void set_pps(u08 pps_enable,  u08 pps_polarity,  double delay, double pps_delay, float threshold, int save)
{
int val;
int i;
u08 flags;
double dly;

   if(read_only) return;
   if(luxor) return;

   if(rcvr_type == MOTO_RCVR) {  // !!! moto_chans
      val = (int) (delay*1.0E9);
      send_moto_start("Az");
      send_dword(val);
      send_moto_end();

      send_moto_start("Ay");
      send_dword((S32) pps_delay);
      send_moto_end();

      send_moto_start("Gc");
      send_byte(pps_enable);
      send_moto_end();

      send_moto_start("Bn");  // 6 chan traim control
      send_byte(1);
      if(traim_threshold) {
         send_byte(1);
         send_word(traim_threshold);
      }
      else {
         send_byte(0);
         send_word(1000);
      }
      send_byte(pps_enable);
      for(i=0; i<10; i++) send_byte(0);
      send_moto_end();

      send_moto_start("En");  // 8 chan traim control
      send_byte(1);
      if(traim_threshold) {
         send_byte(1);
         send_word(traim_threshold);
      }
      else {
         send_byte(0);
         send_word(1000);
      }
      send_byte(pps_enable);
      for(i=0; i<10; i++) send_byte(0);
      send_moto_end();
   }
   else if(rcvr_type == NVS_RCVR) { 
      send_nvs_start(0xD7);
      send_byte(0x06);
      send_double(delay * 1.0E6);   
      send_nvs_end();

      flags = 0x02;    // software PPS every second
      if(pps_polarity == 0)   flags |= 0x08;  // rising edge (direct PPS)
      if(time_flags & 0x0001) flags |= 0x30;  // sync PPS to UTC time
      else                    flags |= 0x10;  // sync PPS to GPS time
      send_nvs_start(0xD7); // qqqqqqqqq !!!!! can also sync to GLONASS or UTC-SU
      send_byte(0x05);
      send_byte(flags);     // PPS config byte
      send_byte(1);         // keep internal time scale aligned
      if(pps_enable) {
         if(nvs_pps_width) send_dword(nvs_pps_width); // set pps width in nanoseconds
         else if(last_nvs_pps_width) send_dword(last_nvs_pps_width); // set pps width in nanoseconds
         else send_dword(NVS_PPS_WIDTH); // set pps width in nanoseconds
      }
      else send_dword(0); // set pps width in nanoseconds
      send_nvs_end();

      Sleep(200);
      request_pps_info();
   }
   else if(rcvr_type == SCPI_RCVR) {
      if(scpi_type == NORTEL_TYPE) {
         sprintf(out, "GPS:REF:ADELAY %.9f", delay);
         queue_scpi_cmd(out, SCPI_SET_CABLE_MSG);
         queue_scpi_cmd("GPS:REF:ADELAY?", SCPI_GET_CABLE_MSG);
      }
      else {
         sprintf(out, ":PTIM:GPS:ADEL %.9f", delay);
         queue_scpi_cmd(out, SCPI_SET_CABLE_MSG);
         queue_scpi_cmd("PTIM:GPS:ADEL?", SCPI_GET_CABLE_MSG);
      }

      if(scpi_type == HP_TYPE) {
         if(pps_polarity) {  // falling edge  Z3816 HP5xxxxx
            queue_scpi_cmd(":PTIM:PPS:EDGE FALL", SCPI_EDGE_MSG);
         }
         else {  // rising edge
            queue_scpi_cmd(":PTIM:PPS:EDGE RIS", SCPI_EDGE_MSG);
         }
         queue_scpi_cmd(":PTIM:PPS:EDGE?", SCPI_EDGE_MSG);  // HP_TYPE only
      }

      // !!!! Z3816 / HP5xxxx can set pps rate
      // :PULSE:CONT:PER seconds
      // :PULSE:CONT:STAT (0 | 1)
   }
   else if(rcvr_type == STAR_RCVR) {
      if(delay > 0.0) delay = 0.0 - delay;  // STAR values must always be negative
      sprintf(out, "PPS_CABLE_DELAY=%d;", (int) (delay*1.0E9));
      queue_star_cmd(out, STAR_CABLE_MSG);
      request_rcvr_config(4965);
   }
   else if(rcvr_type == UCCM_RCVR) {   // can't change polarity
      sprintf(out, "GPS:REF:ADEL %.9f", delay);
      queue_scpi_cmd(out, SCPI_SET_CABLE_MSG);
      queue_scpi_cmd("GPS:REF:ADEL?", SCPI_GET_CABLE_MSG);

      if(pps_enable) queue_scpi_cmd("OUTP:ACT:ENAB", UCCM_OUTP_MSG);
      else           queue_scpi_cmd("OUTP:ACT:DIS", UCCM_OUTP_MSG);
      queue_scpi_cmd("OUTP:STAT?", UCCM_STATE_MSG);

      // !!!! Z3816 / HP5xxxx can set pps rate
      // :PULSE:CONT:PER seconds
      // :PULSE:CONT:STAT (0 | 1)
   }
   else if(rcvr_type == TSIP_RCVR) {
      send_tsip_start(0x8E);
      send_byte(0x4A);
      send_byte(pps_enable);
      send_byte(0x00);
      send_byte(pps_polarity);
      send_double(delay);
      send_single(threshold);
      send_tsip_end();

      if(save) save_segment(6, 22);  // save timing config in EEPROM
   }
   else if(rcvr_type == UBX_RCVR) {  // !!!! zorky TP5
      set_ubx_pps(0, pps_rate, pps_enable, pps_polarity, delay, pps_delay, save);
   }
   else if(rcvr_type == VENUS_RCVR) {
      send_venus_start(SET_VENUS_CABLE);
      send_dword((S32) (delay*1.0E11));
      send_venus_save(1);
   }
   else if(rcvr_type == ZODIAC_RCVR) {
      dly = cable_delay;
      cable_delay = delay;
      set_zod_config(0, 0.0,0.0,0.0, traim_threshold, 4);  // cable delay
      cable_delay = dly;
   }
}


void request_pps_mode()
{
   if(read_only) return;    // !!!Resolution-T, RES360 only
   if(no_poll) return;
   if(luxor) return;
   if(saw_ntpx) return;

   if(rcvr_type == MOTO_RCVR) {
      send_moto_start("Gc");
      send_byte(0xFF);
      send_moto_end();

      send_moto_start("AP");   // get pps rate
      send_byte(0xFF);
      send_moto_end();
   }
   else if(rcvr_type == TSIP_RCVR) {
      send_tsip_start(0x8E);
      send_byte(0x4E);
      send_tsip_end();
   }
   else if(rcvr_type == UBX_RCVR) {
      if(saw_ubx_tp5) request_ubx_tp5(0);
      else            request_ubx_msg(UBX_PPS);
   }
}

void request_pps_width()
{
   if(read_only) return;    // !!!Which receivers can do this?
   if(no_poll) return;
   if(luxor) return;

   if(rcvr_type == NVS_RCVR) {
      send_nvs_start(0xD7);
      send_byte(0x05);
      send_nvs_end();
   }
   else if(rcvr_type == TSIP_RCVR) {
      send_tsip_start(0x8E);
      send_byte(0x4F);
      send_tsip_end();
   }
   else if(rcvr_type == UBX_RCVR) {
      if(saw_ubx_tp5) request_ubx_tp5(0);
      else            request_ubx_msg(UBX_PPS);
   }
}

void set_pps_mode(int mode)
{
int old_rate;

   // mode 0x02 - 1PPS
   // mode 0x82 - PP2S / 100 pps mode (Motorola)
   // mode 0x83 - 10000 PPS mode

   if(read_only) return;    // !!! Resolution-T only
   if(luxor) return;

   if(rcvr_type == MOTO_RCVR) {
//sprintf(debug_text, "set pps mode:%d  oldr:%d  oldm:%d", mode, pps_rate,pps_mode);
      send_moto_start("AP");
      if(mode == 0x82) send_byte(1);  // 100 pps
      else             send_byte(0);  // 1 PPS
      send_moto_end();
   }
   else if(rcvr_type == STAR_RCVR) {  // !!!!! we use this to toggle the TOD output
      if(mode == 0x82) {  // disable TOD message
         queue_star_cmd("TOD_STATE=0;", STAR_TOD_MSG);
      }
      else {
         queue_star_cmd("TOD_STATE=1;", STAR_TOD_MSG);
      }
      queue_star_cmd("TOD_STATE;", STAR_TOD_MSG);
   }
   else if(rcvr_type == TSIP_RCVR) {
      send_tsip_start(0x8E);
      send_byte(0x4E);
      mode &= 0xFE;
      send_byte(mode);
      send_tsip_end();
   }
   else if(rcvr_type == UBX_RCVR) {
       old_rate = pps_rate;
       pps_rate = mode;
       set_pps(user_pps_enable, pps_polarity,  delay_value, pps1_delay,  300.0, 10);
       pps_rate = old_rate;
       have_pps_rate = 1;
   }
   else if(rcvr_type == UCCM_RCVR) {
      if(mode == 0x82) queue_scpi_cmd("OUTP:TP:SEL PP2S", UCCM_RATE_MSG);
      else             queue_scpi_cmd("OUTP:TP:SEL PP1S", UCCM_RATE_MSG);
      queue_scpi_cmd("OUTP:TP:SEL?", UCCM_RATE_MSG);
   }
}


void save_segment(u08 segment, int why)
{
   // save config info into EEPROM
   //     3 = receiver config
   //     6 = timing config
   //     7 = survey config
   //     9 = discipline config
   //   255 = all info

   if(read_only) return;
   if(luxor) return;
   if(no_eeprom_writes) return;

   if(rcvr_type == TSIP_RCVR) {
      if(res_t) {
         if(segment == 7) start_self_survey(1, 3);  // save survey params
         else             write_all_nvs();          // save current config
      }
      else {
         send_tsip_start(0x8E);
         send_byte(0x4C);
         send_byte(segment);
         send_tsip_end();
      }
      ++ee_write_count;
      BEEP(201);   // !!! if you hear lots of beeps,  you may be wearing out the EEPROM
   }
   else if(rcvr_type == UBX_RCVR) {
      if     (segment == 3)   update_ubx_config(0x0000, UBX_EE_NAV, UBX_EE_NAV);
      else if(segment == 6)   update_ubx_config(0x0000, UBX_EE_TP, UBX_EE_TP);
      else if(segment == 255) update_ubx_config(0x0000, UBX_EE_ALL, UBX_EE_ALL);
   }
}


void request_dac_voltage()
{
   if(luxor) return;
   if(no_poll) return;
   if(res_t) return;
   if(rcvr_type != TSIP_RCVR) return;

   send_tsip_start(0x8E);
   send_byte(0xA0);
   send_tsip_end();
}

void set_dac_voltage(float volts, int why)
{
int val;

   if(read_only) return;
   if(luxor) return;

   if(rcvr_type == TSIP_RCVR) {
      send_tsip_start(0x8E);
      send_byte(0xA0);
      send_byte(0x00);
      send_single(volts);
      send_tsip_end();
   }
   else if(rcvr_type == UCCM_RCVR) {
      val = (int) volts;
      sprintf(out, "DIAG:ROSC:EFC:DATA %x", val);
      queue_scpi_cmd(out, UCCM_SETDAC_MSG);
      queue_scpi_cmd("DIAG:ROSC:EFC:DATA?", UCCM_EFC_MSG);
      discipline_mode = 3;  // manual control
//sprintf(plot_title, "Set dac to %f   why:%d", volts,why);
if(debug_file) fprintf(debug_file, "Set dac to %f   why:%d\n", volts,why);
   }
}

void set_dac_value(u32 value)
{
   if(read_only) return;
   if(luxor) return;
   if(rcvr_type != TSIP_RCVR) return;

   send_tsip_start(0x8E);
   send_byte(0xA0);
   send_byte(0x01);
   send_dword(value);
   send_tsip_end();
}


void set_osc_sense(u08 mode, int save)
{
   if(read_only) return;
   if(luxor) return;
   if(rcvr_type != TSIP_RCVR) return;

   send_tsip_start(0x8E);  // not available on ThunderBolt-E or early ThunderBolts
   send_byte(0xA1);
   send_byte(mode);
   send_tsip_end();

   if(save) save_segment(6, 24);       // save timing config in EEPROM
}


void request_traim_info()
{
   if(luxor) return;
   if(no_poll) return;

   if(rcvr_type == MOTO_RCVR) {
      send_moto_start("Ge");  // 12 chan traim data
      send_byte(0xFF);
      send_moto_end();
      send_moto_start("Gf");  // 12 chan traim data
      send_word(0xFFFF);
      send_moto_end();
   }
}

void request_timing_mode()
{
   if(luxor) return;
   if(no_poll) return;

   if(rcvr_type == MOTO_RCVR) {
      send_moto_start("Aw");  // get GPS/UTC mode
      send_byte(0xFF);
      send_moto_end();

      send_moto_start("Bo");  // get UTC offset command
      send_byte(0);
      send_moto_end();
   }
   else if(rcvr_type == NVS_RCVR) {
      send_nvs_start(0x1E);  // !!!! gets leapsecond info, timing mode comes from request_pps_info()
      send_nvs_end();
   }
   else if(rcvr_type == TSIP_RCVR) {
      send_tsip_start(0x8E);
      send_byte(0xA2);
      send_tsip_end();
   }

   request_traim_info();
}

void set_timing_mode(u08 mode)
{
int old_tf;
   // set the receiver to report time in either GPS or UTC
   // !!!!! should we always do manual_time if user set the UTC offset?

   if(read_only) return;

   if(rcvr_type == ACRON_RCVR) {
      goto manual_time;
   }
   else if(rcvr_type == LUXOR_RCVR) {
      goto manual_time;
   }
   else if(rcvr_type == MOTO_RCVR) {
      send_moto_start("Aw");
      if(mode) send_byte(1);
      else     send_byte(0);
      send_moto_end();
   }
   else if(rcvr_type == NMEA_RCVR) {
      manual_time:     // receiver does not support output in GPS time, we do the conversion (if utc_offset is available)

      if(mode == 0) {  // GPS time
         time_flags &= (~0x0001);
         timing_mode = 0x00;
         have_timing_mode = 1;
      }
      else {  // UTC time
         time_flags |= (0x0001);
         timing_mode = 0x03;
         have_timing_mode = 1;
      }
   }
   else if(rcvr_type == NVS_RCVR) {
      old_tf = time_flags;
      if(mode == 0) {  // GPS time
         time_flags &= (~0x0001);
      }
      else {  // UTC time
         time_flags |= (0x0001);
      }
      set_pps(user_pps_enable, pps_polarity,  delay_value, pps1_delay,  300.0, 44);
      time_flags = old_tf;
   }
   else if(rcvr_type == NO_RCVR) {
      goto manual_time;
   }
   else if(rcvr_type == SCPI_RCVR) {
      if(scpi_type == NORTEL_TYPE) {
         goto manual_time;
      }

      if(mode == 0) queue_scpi_cmd(":DIAG:GPS:UTC 0", SCPI_UTC_MSG);
      else          queue_scpi_cmd(":DIAG:GPS:UTC 1", SCPI_UTC_MSG);
      queue_scpi_cmd("*TST?", SCPI_TEST_MSG);  // cause a system reset
      show_test_warning();
//    queue_scpi_cmd(":DIAG:GPS:UTC?", SCPI_UTC_MSG);
   }
   else if(rcvr_type == SIRF_RCVR) {
      goto manual_time;
   }
   else if(rcvr_type == STAR_RCVR) {
      goto manual_time;
   }
   else if((rcvr_type == TSIP_RCVR) && (tsip_type == STARLOC_RCVR)) {
      send_tsip_start(0x8E);
      send_byte(0xA2);
      send_byte(0x00);  // always set GPS mode, DATA_RCVR always reports it is in GPS mode
      send_tsip_end();
      goto manual_time;
   }
   else if(rcvr_type == TSIP_RCVR) {
      send_tsip_start(0x8E);
      send_byte(0xA2);
      send_byte(mode);
      send_tsip_end();
   }
   else if(rcvr_type == UBX_RCVR) {
      goto manual_time;
   }
   else if(rcvr_type == UCCM_RCVR) {
      goto manual_time;
   }
   else if(rcvr_type == VENUS_RCVR) {
      goto manual_time;
   }
   else if(rcvr_type == ZODIAC_RCVR) {
      if(mode == 0) {  // GPS time
         time_flags &= (~0x0001);
         timing_mode = 0x00;
         have_timing_mode = 1;
      }
      else {  // utc time
         time_flags |= (0x0001);
         timing_mode = 0x03;
         have_timing_mode = 1;
      }
      set_zod_config(0, 0.0,0.0,0.0, traim_threshold, 5);  // timing mode alignment
   }
}


void set_traim_mode(int thresh, int save)
{
int old_thresh;

   if(luxor) return;

   if(rcvr_type == MOTO_RCVR) {
      if(thresh <= 0) {  // 12-channel disable traim
         send_moto_start("Ge");
         send_byte(0);
         send_moto_end();
      }
      else {  // 12-channel enable traim
         send_moto_start("Gf");
         send_word(thresh);
         send_moto_end();

         send_moto_start("Ge");
         send_byte(1);
         send_moto_end();
      }

      // 6/8 channel traim control
      old_thresh = traim_threshold;
      traim_threshold = thresh;
      set_pps(pps_mode, pps_polarity, delay_value, pps1_delay, 300.0, save);
      traim_threshold = old_thresh;
set_pps_mode(pps_rate);
   }
   else if(rcvr_type == NVS_RCVR) { 
      send_nvs_start(0xD7);
      send_byte(0x07);
      send_word(thresh);
      send_nvs_end();

      request_pps_info();
   }
   else if(rcvr_type == ZODIAC_RCVR) {
      if(thresh < 0) thresh = 0;
      set_zod_config(0, 0.0,0.0,0.0, thresh, 6);
   }
}


void set_discipline_mode(u08 mode)
{
   if(read_only) return;
   if(luxor) return;

   // mode 0: jam sync
   // mode 1: enter recovery
   // mode 2: manual holdover
   // mode 3: exit manual holdover
   // mode 4: disable disciplining
   // mode 5: enable disciplining

   if(rcvr_type == STAR_RCVR) {
      if(mode == 2) queue_star_cmd("MODE=H;", STAR_HOLD_MSG);
      else if(mode == 3) queue_star_cmd("MODE=A;", STAR_HOLD_MSG);
   }
   else if(rcvr_type == SCPI_RCVR) {  // 2=exit holdover  3=force holdover
      z_rcvr:
if(debug_file) fprintf(debug_file, "set_dmode(%d)\n", mode);
// !!! should set com_timeout
      if(scpi_type == NORTEL_TYPE) {
         if     (mode == 3) queue_scpi_cmd("ROSC:HOLD:REC:INIT", SCPI_HOLDOVER_MSG);
         else if(mode == 2) queue_scpi_cmd("ROSC:HOLD:INIT", SCPI_HOLDOVER_MSG);
         else if(mode == 0) queue_scpi_cmd("SYNC:IMMEDIATE", SCPI_JAMSYNC_MSG);
//       else if(mode == 4) ; // zzzzz disable disciplining
//       else if(mode == 5) ; // zzzzz enable disciplining
         queue_scpi_cmd("ROSC:HOLD:DUR?", SCPI_HOLDOVER_MSG);
      }
      else {
         if     (mode == 3) queue_scpi_cmd(":SYNC:HOLD:REC:INIT", SCPI_HOLDOVER_MSG);
         else if(mode == 2) queue_scpi_cmd(":SYNC:HOLD:INIT", SCPI_HOLDOVER_MSG);
         else if(mode == 0) queue_scpi_cmd(":SYNC:IMM", SCPI_JAMSYNC_MSG);
//       else if(mode == 4) ; // zzzzz disable disciplining
//       else if(mode == 5) ; // zzzzz enable disciplining
         queue_scpi_cmd(":ROSC:HOLD:DUR?", SCPI_HOLDOVER_MSG);
      }
   }
   else if(rcvr_type == UCCM_RCVR) {
if(scpi_type == UCCMP_TYPE) goto z_rcvr;  //kkkkk
      if((mode == 2) || (mode == 4)) {  // disable disciplining at current EFC
         set_dac_voltage(uccm_voltage, 20);
      }
      else if((mode == 1) || (mode == 3) || (mode == 5)) {  // enable disciplining
         queue_scpi_cmd("DIAG:ROSC:EFC:DATA GPS", UCCM_SETDAC_MSG);
         queue_scpi_cmd("DIAG:ROSC:EFC:DATA?", UCCM_EFC_MSG);
         discipline_mode = 4;  // recovery
      }
   }
   else if(rcvr_type == TSIP_RCVR) {
      send_tsip_start(0x8E);
      send_byte(0xA3);
      send_byte(mode);
      send_tsip_end();
   }

   if(mode == 0) {
      osc_integral = 0.0;
      have_osc = 0;
   }
}


void exit_test_mode()
{
   // this is a set_() type function
   if(read_only) return;
   if(luxor) return;
   if(rcvr_type != TSIP_RCVR) return;

   send_tsip_start(0x8E);
   send_byte(0xA4);
   send_byte(0x00);
   send_tsip_end();
}

void set_test_mode()
{
   //!!!
   if(read_only) return;
   if(luxor) return;
}


void request_packet_mask()
{
   if(luxor) return;
   if(luxor) return;
   if(rcvr_type != TSIP_RCVR) return;

   send_tsip_start(0x8E);
   send_byte(0xA5);
   send_tsip_end();
}

void set_venus_mode(int mode)
{
   // mode 1: NMEA output (accepts binary input) 2:binary output

   send_venus_start(SET_VENUS_BINARY);
   send_byte(mode);
   send_venus_save(1);
}


void set_nav_rate(float hz)
{
   if(hz <= 0.0) hz = 1.0;

   if(rcvr_type == NO_RCVR) {
      if(!pause_data) nav_rate = hz;
   }
   else if(rcvr_type == NVS_RCVR) {
      send_nvs_start(0xD7); // set nav rate (1 Hz)
      send_byte(0x02);
      send_byte((u08) hz);
      send_nvs_end();
   }
   else if(rcvr_type == UBX_RCVR) {
      if(time_flags & 0x0001) {       // sync to UTC time
         set_ubx_nav_rate(1000.0F/hz, 1); // 1Hz, align to UTC
      }
      else {
         set_ubx_nav_rate(1000.0F/hz, 0); // 1Hz, align to GPS
      }
   }
   else if(rcvr_type == VENUS_RCVR) {
      send_venus_start(SET_VENUS_OUT_RATE);
      send_byte(1);  // 1 Hz
      send_venus_save(1);
      Sleep(1000);

      // doing this causes the receiver to effectively warm start
      send_venus_start(SET_VENUS_NAV_RATE);
      send_byte((u08) hz);  // Hz
      send_venus_save(1);
   }
}


void init_receiver(int why)
{
int i;
u16 mask1, mask2;

   // initialze GPS receiver to send periodic messages

   reset_sat_health();

   if(read_only) return;
   if(luxor) return;

   if(rcvr_type == ACRON_RCVR) {
      this_acron_cmd = 0;
      if(0) {  // initialize receiver time zone, etc
         init_acron_time();
         Sleep(500);
      }
      send_acron_cmd("o");
   }
   else if(rcvr_type == GPSD_RCVR) {  // gggg - send ?WATCH cmd here
      enable_gpsd();
   }
   else if(rcvr_type == MOTO_RCVR) {
      send_moto_start("Wb");  // set Jupiter-T to Motorola mode
      send_byte(0);
      send_moto_end();

      Sleep(250);
      if(1 && debug_file) {  // !!!! gggg
         send_moto_start("Cj");  // request receiver ID string
         send_moto_end();
      }

      send_moto_start("Cg");  // enable 6/8 chan reciever to do fixes
      send_byte(1);
      send_moto_end();

      send_moto_start("Ba");  // 6 chan position data
      send_byte(1);
      send_moto_end();

      send_moto_start("Bk");  // 6 chan extended position data 
      send_byte(1);
      send_moto_end();

      send_moto_start("Ea");  // 8 chan position data
      send_byte(1);
      send_moto_end();

      send_moto_start("Ek");  // 8 chan extended position data
      send_byte(1);
      send_moto_end();

      send_moto_start("Ha");  // 12 chan position data
      send_byte(1);
      send_moto_end();


      send_moto_start("Hr");  // differential data every 10 seconds
      send_byte(10);
      send_byte('+');
      send_byte('+');
      send_byte('+');
      send_byte('+');
      send_byte('+');
      send_byte('+');
      send_moto_end();


      send_moto_start("Bn");  // 6 chan traim status
      send_byte(1);
//      send_byte(0);
//      send_word(1000);
      send_byte(0xFF);  // values should be rejected
      send_word(0xFFFF);
      send_byte(0xFF);
      for(i=0; i<10; i++) send_byte(0xFF);
      send_moto_end();

      send_moto_start("En");  // 8 chan traim status
      send_byte(1);
//      send_byte(0);
//      send_word(1000);
      send_byte(0xFF); // values should be rejected
      send_word(0xFFFF);
      send_byte(0xFF);
      for(i=0; i<10; i++) send_byte(0xFF);
      send_moto_end();

      send_moto_start("Hn");  // 12 chan traim
      send_byte(1);
      send_moto_end();

      request_pps_mode();
   }
   else if(rcvr_type == NMEA_RCVR) {
      sprintf(out, "PUBX,41,1,0003,0002,%d,0", baud_rate);  // 9600 bps ublox NMEA output mode (port 1)
      send_nmea_cmd(out);
//    sprintf(out, "PUBX,41,2,0003,0002,%d,0", baud_rate);  // 9600 bps ublox NMEA output mode (port 2)
//    send_nmea_cmd(out);
   }
   else if(rcvr_type == NVS_RCVR) {
      send_nvs_start(0xB2);
      send_word(0x0004);    // no CRC, ellisoid altitude, geodetic coords
//    send_word(0x0006);    // CRC, ellisoid altitude, geodetic coords
      send_nvs_end();

//    set_nav_rate(1.0);      // 1Hz nav solutions

      send_nvs_start(0x27); // 0x88 PVT nav message every nav solution
      send_byte(0x01);
      send_nvs_end();

      send_nvs_start(0xF4); // 0xF5 raw data every second
      send_byte(10);
      send_nvs_end();

      send_nvs_start(0x1F); // 0x72 time message every second
      send_byte(0x01);
      send_nvs_end();

      send_nvs_start(0x13); // 0x41 course/speed every second
      send_byte(0x01);
      send_nvs_end();

      send_nvs_start(0x24); // 0x54 sky view message every 5 seconds
      send_byte(0x05);
      send_nvs_end();

      send_nvs_start(0x31); // 0x61 DOPs message every 10 seconds
      send_byte(10);
      send_nvs_end();

      send_nvs_start(0x12); // get excluded sats
      send_nvs_end();
   }
   else if(rcvr_type == SCPI_RCVR) {
      Sleep(100);
      scpi_req = 0;
      send_scpi_cmd("", SCPI_NULL_MSG);
Sleep(100);
      queue_scpi_cmd("*CLS", SCPI_CLS_MSG);
      if(scpi_type == HP_TYPE) {  // gggg
         queue_scpi_cmd(":SYST:COM:SER1:FDUP 0", SCPI_NULL_MSG); // for Z3816 HP5xxxx (could be :FDUP OFF)
         queue_scpi_cmd("PTIME:TCOD:CONT 0", SCPI_NULL_MSG);     // for Z3816 HP5xxxx
         queue_scpi_cmd(":SYST:COM:SER1:PACE NONE", SCPI_NULL_MSG);   // for Z3816 HP5xxxx
         queue_scpi_cmd(":FORM:CONT:DATA ASC", SCPI_NULL_MSG);   // ascii data  Z3816 HP5xxxx 
         queue_scpi_cmd(":PTIM:TZON 0,0", SCPI_NULL_MSG);        // GMT time zone zone (Heather does time zone conversions)
      }
//    queue_scpi_cmd(":PTIM:TCOD:FORM F1", SCPI_TIMEFMT_MSG);  // hex time format
      queue_scpi_cmd(":PTIM:TCOD:FORM F2", SCPI_TIMEFMT_MSG);  // decimal time format
      queue_scpi_cmd(":PTIM:GPS:POS:SURV:STAT?", SCPI_SURVEY_MSG);

      poll_next_scpi();
   }
   else if(rcvr_type == SIRF_RCVR) {
//    send_nmea_cmd("PSRF100,0,9600,8,1,0");  // 9600 bps switch SIRF to binary mode
      sprintf(out, "PSRF100,0,%d,%d,1,0", baud_rate, data_bits);  // 9600 bps switch SIRF to binary mode
      send_nmea_cmd(out);
      Sleep(500);
      send_sirf_start(SET_STATIC_MODE, 2);  // disable static nav mode
      send_byte(0);
      send_sirf_end();
   }
   else if(rcvr_type == STAR_RCVR) {
      // When a STAR receiver is powered up, it won't send time messages
      // until a RESTART command is sent!!!  Here, we kick it in its
      // sorry ass.
      if(restart_count > 5) {  // send warm reset after 5 consecutive loss of data restart attempts
         saw_star_time = 0;
         star_restart_ok = 1;
      }
      if(star_restart_ok && (saw_star_time == 0) && (why == 0)) {  // loss of com data
         star_restart_ok = 0;   // avoid RESTART(W) cascade
         queue_star_cmd("RESTART(W);", STAR_RESTART_MSG);
      }

      sat_count = 0;
      have_time = 0;

      queue_star_cmd("INFO_GPS_POS;", STAR_POS_MSG);    // request time and location message
      queue_star_cmd("ALARM_MASK=N;", STAR_ALMASK_MSG); // enable all alarms
      queue_star_cmd("TOD_STATE;", STAR_TOD_MSG);       // get TOD output state

      request_rcvr_config(7766);  // CONF; and elevation mask
      if(user_set_osc) {
         have_osc_params = user_set_osc;
         update_osc_params();
      }
      poll_next_scpi();
   }
   else if(rcvr_type == TSIP_RCVR) {
      if(tsip_type == STARLOC_RCVR) {  // !!!! does not accept standard TSIP packet mask values
         mask1 = 0xFFFF;
         mask2 = 0xFFFF;
      }
      else {
         mask1 = 0x0055;  // broadcast primary and supplemntal timing packets
         mask2 = 0x0000;  // also broadcast satellite solutions, gps system data,  and ephemeris data
      }
      send_tsip_start(0x8E);
      send_byte(0xA5);
      send_word(mask1);
      send_word(mask2);
      send_tsip_end();

      set_io_options(0x13, 0x03, 0x01, 0x08);  // ECEF+LLA+DBL PRECISION, ECEF+ENU vel,  UTC, no PACKET 5A, dBc
//    set_io_options(0x13, 0x03, 0x01, 0x09);  // ECEF+LLA+DBL PRECISION, ECEF+ENU vel,  UTC, PACKET 5A
//    set_io_options(0x13, 0x03, 0x01, 0x00);  // ECEF+LLA+DBL PRECISION, ECEF+ENU vel,  UTC, no PACKET 5A, AMU

      if(0 && user_set_osc) {  // set osc params from command line values
         update_osc_params();
      }
   }
   else if(rcvr_type == UBX_RCVR) {
      rcvr_type = NMEA_RCVR;
      sprintf(out, "PUBX,41,1,0003,0001,%d,0", baud_rate);  // 9600 bps ublox binary output mode (port 1)
      send_nmea_cmd(out);
//    sprintf(out, "PUBX,41,2,0003,0001,%d,0", baud_rate);  // 9600 bps ublox binary output mode (port 2)
//    send_nmea_cmd(out);
      rcvr_type = UBX_RCVR;
Sleep(1000);

      set_ubx_rate(UBX_NAV, 1);     // nav solution
      set_ubx_rate(UBX_UTC, 1);     // time solution
      set_ubx_rate(UBX_SPEED,1);    // course / speed
      set_ubx_rate(UBX_GPS, 1);     // gps-utc offset
      set_ubx_rate(UBX_STATUS, 1);  // operating mode
      set_ubx_rate(UBX_CLOCK, 1);   // operating mode
      set_ubx_rate(UBX_TP, 1);      // time pulse (sawtooth)

//    set_nav_rate(1.0);  // 1 Hz nav rate
//    set_ubx_rate(UBX_SVN, 5);    // sat info
      if(UBX_RAW_RATE) {  // automatically send RAW and SFRB messages
         set_ubx_rate(UBX_RAW,  UBX_RAW_RATE);    // RAW info
         set_ubx_rate(UBX_SFRB, UBX_RAW_RATE);    // SFRB info
      }
set_ubx_rate(UBX_SFRB, 0);         // SFRB info

      set_ubx_antenna(0x1F);       // make sure open circuit monitoring is on
   }
   else if(rcvr_type == UCCM_RCVR) {
      Sleep(100);
      scpi_req = 0;
      send_scpi_cmd("", SCPI_NULL_MSG);
      queue_scpi_cmd("TOD EN", SCPI_NULL_MSG); 
      queue_scpi_cmd("SYNC:REF:DISABLE LINK", SCPI_NULL_MSG);
      queue_scpi_cmd("SYNC:REF:ENABLE GPS", SCPI_NULL_MSG);
      queue_scpi_cmd("REF:TYPE GPS", SCPI_NULL_MSG);
      queue_scpi_cmd("OUTP:STAT?", UCCM_STATE_MSG);
      queue_scpi_cmd("LED:GPSL?", UCCM_LED_MSG);
///    queue_scpi_cmd("OUTP:TP:SEL PP1S", UCCM_RATE_MSG);
//!!!! queue_scpi_cmd("DIAG:ROSC:EFC:DATA GPS", UCCM_SETDAC_MSG);  // make sure gps ref is enabled, but this causes a recovery state
      queue_scpi_cmd(":GPS:POS:SURV:STAT?", SCPI_SURVEY_MSG);  // kkkkkk
      if(user_set_osc) {
         have_osc_params = user_set_osc;
         update_osc_params();
      }
      queue_scpi_cmd("SYST:STAT?", UCCM_STATUS_MSG);

      poll_next_uccm();
   }
   else if(rcvr_type == VENUS_RCVR) {
      set_venus_mode(1);  // NMEA (mostly) output, NMEA/binary in

      send_venus_start(SET_VENUS_PINNING);
      send_byte(2);   // diable pinning
      send_venus_save(0+1);

      send_venus_start(SET_VENUS_POWER);
      send_byte(0);   // normal power
      send_venus_save(0+1);

      send_venus_start(SET_VENUS_NAV_MODE);
      send_byte(0x17);
      send_byte(0);     // auto mode
      send_venus_save(0+1);

//    set_nav_rate(1.0);  // 1Hz nav rate
   }
   else if(rcvr_type == ZODIAC_RCVR) {
      send_moto_start("Wb");  // set Jupiter-T to Zodiac mode
      send_byte(1);
      send_moto_end();
      Sleep(250);

      query_zod_msg(ZOD_EE_STATUS_MSG);
//    set_zod_power(0);
      set_zod_nav_mode(0x000F);
   }
}

void request_gnss_system()
{
   // request the current GNSS system selections
   if(no_poll) return;
   if(luxor) return;

   if(rcvr_type == NVS_RCVR) {
      send_nvs_start(0x0D);  // gnss system info
      send_byte(0x02);
      send_nvs_end();
   }
   else if(rcvr_type == TSIP_RCVR) {
      if(saw_icm || (res_t == RES_T_360)) {
         request_rcvr_config(666);
      }
   }
   else if(rcvr_type == UBX_RCVR) {
      request_ubx_msg(UBX_GNSS);
   }
   else if(rcvr_type == VENUS_RCVR) {
      query_venus_msg(QUERY_VENUS_GNSS);
      query_venus_msg(QUERY_VENUS_SBAS);
      query_venus_msg(QUERY_VENUS_QZSS);
   }
}

#define NUM_UBX_SYS 7
void set_ubx_gnss(int system)
{
int id;
int min_chans;
int max_chans;
int min_chan_count;
int max_chan_count;
u32 flags;

   send_ubx_start(UBX_GNSS, 4+NUM_UBX_SYS*8);
   send_byte(0);
   send_byte(32);
   send_byte(0xFF);
   send_byte(NUM_UBX_SYS);

   min_chan_count = max_chan_count = 0;
   for(id=0; id<NUM_UBX_SYS; id++) {
      flags = 0x01010000;
      min_chans = 0;
      max_chans = 0;
      if(id == 0) {       // GPS
         if(system & GPS) flags |= 0x0001;
         min_chans = 8;
         max_chans = 16;
      }
      else if(id == 1) {  // SBAS
         if(system & SBAS) flags |= 0x0001;
         min_chans = 1;
         max_chans = 3;
      }
      else if(id == 2) {  // GALILEO
         if(system & GALILEO) flags |= 0x0001;
         min_chans = 4;
         max_chans = 8;
      }
      else if(id == 3) {  // Beidou
         if(system & BEIDOU) flags |= 0x0001;
         min_chans = 8;
         max_chans = 16;
      }
      else if(id == 4) {  // IMES
         if(system & IMES) flags |= 0x0001;
         flags |= 0x00020000;
         min_chans = 0;
         max_chans = 8;
      }
      else if(id == 5) {  // QZSS
         if(system & QZSS) flags |= 0x0001;
         flags |= 0x00040000;
         min_chans = 0;
         max_chans = 3;
      }
      else if(id == 6) {  // GLONASS
         if(system & GLONASS) flags |= 0x0001;
         min_chans = 8;
         max_chans = 14;
      }

      min_chan_count += min_chans;
      max_chan_count += max_chans;

      send_byte(id);
      send_byte(min_chans);
      send_byte(max_chans);
      send_byte(0); // reserved
      send_dword(flags);
   }

   send_ubx_end();
}

void set_gnss_system(int system)
{
u08 val;
int old_gnss_mask;
int old_have_gnss;

   // set the GNSS system/systems to use
   if(luxor) return;
   if(read_only) return;

   if(rcvr_type == NVS_RCVR) {
      system &= (~BEIDOU);
      send_nvs_start(0x0D);
      send_byte(0x02);
      val = 0;
      if((system & GPS) && (system & GLONASS)) val = 0;
      else if(system & GPS) val = 1;
      else if(system & GLONASS) val = 2;
      else if(system & GALILEO) val = 3;
      if(system & GALILEO) ;            // can't do SBAS
      else if(system & SBAS) val += 10; // include SBAS 
      send_byte(val);
      send_nvs_end();
   }
   else if(rcvr_type == UBX_RCVR) {
      set_ubx_gnss(system);
   }
   else if(rcvr_type == VENUS_RCVR) {
      send_venus_start(SET_VENUS_GNSS);
      send_byte(0x19);
      send_word((u16) (system & (GPS | GLONASS | BEIDOU | GALILEO)));
      send_venus_save(1);

      send_venus_start(SET_VENUS_QZSS);
      send_byte(0x03);
      if(system & QZSS) send_byte(1);
      else              send_byte(0);
      send_byte(1);       // channels
      send_venus_save(1);

if(1) {
      send_venus_start(SET_VENUS_SBAS);
      send_byte(0x01);
      if(system & SBAS) send_byte(1);
      else              send_byte(0);
      send_byte(2);     // auto select mode
      send_byte(8);     // URA
      send_byte(1);     // apply corrections
      send_byte(2);     // channels
      send_byte(0x0F);  // WAAS + EGNOS + MSAS + GAGAN
      send_venus_save(1);
}
   }
   else if(rcvr_type == TSIP_RCVR) {
      if(saw_icm || (res_t == RES_T_360)) {
         old_gnss_mask = gnss_mask;
         old_have_gnss = have_gnss_mask;
         have_gnss_mask = 666;

         set_config(0xFF, 0xFF, -1.0F, -1.0F, -1.0F, -1.0F, 0xFF);

         gnss_mask = old_gnss_mask;
         have_gnss_mask = old_have_gnss;
      }
   }
   request_gnss_system();
}


void request_discipline_params(u08 type)
{
   if(luxor) return;
   if(no_poll) return;
   if(res_t) return;

   if(rcvr_type == STAR_RCVR) {
      if(type == 0x00) {
         request_rcvr_config(5577);
      }
   }
   else if(rcvr_type == TSIP_RCVR) {
      send_tsip_start(0x8E);
      send_byte(0xA8);
      send_byte(type);
      send_tsip_end();
   }
}

void request_all_dis_params()
{
   if(luxor) return;
   if(no_poll) return;

   request_discipline_params(0x00); // time constant
   request_discipline_params(0x01); // gain / DAC range
   request_discipline_params(0x02); // sync / freq thresholds
   request_discipline_params(0x03); // initial DAC voltage
   request_discipline_params(0x04); // undocumented - allowable DAC range?
}



void set_pullin_range(int i)
{
   if(rcvr_type == UCCM_RCVR) {
      sprintf(out, "PULLINRANGE %d", i/10);
      queue_scpi_cmd(out, UCCM_SET_PULLIN_MSG);
      queue_scpi_cmd("PULLINRANGE?", UCCM_GET_PULLIN_MSG);
   }
}


void set_discipline_params(int save)
{
   if(read_only) return;
   if(luxor) return;


   if(rcvr_type == STAR_RCVR) {
      if(have_osc_params & PARAM_TC) {
         sprintf(out, "TAU=%d;", (int) user_time_constant);
         queue_star_cmd(out, STAR_TAU_MSG);
         request_rcvr_config(4565);
      }
   }
   else if(rcvr_type == TSIP_RCVR) {
      if(have_osc_params & (PARAM_TC | PARAM_DAMP)) { 
         send_tsip_start(0x8E);
         send_byte(0xA8);
         send_byte(0x00);
         send_single(user_time_constant);
         send_single(user_damping_factor);
         send_tsip_end();
      }

      if(have_osc_params & (PARAM_GAIN | PARAM_MINV | PARAM_MAXV)) { 
         send_tsip_start(0x8E);
         send_byte(0xA8);
         send_byte(0x01);
         send_single(user_osc_gain);
         send_single(user_min_volts);
         send_single(user_max_volts);
         send_tsip_end();
      }

      if(have_osc_params & (PARAM_JAMSYNC | PARAM_MAXFREQ)) { 
         send_tsip_start(0x8E);
         send_byte(0xA8);
         send_byte(0x02);
         send_single(user_jam_sync);
         send_single(user_max_freq_offset);
         send_tsip_end();
      }

      if(have_osc_params & PARAM_INITV) { 
         send_tsip_start(0x8E);
         send_byte(0xA8);
         send_byte(0x03);
         send_single(user_initial_voltage);
         send_tsip_end();
      }

      if(have_osc_params & (PARAM_MINRANGE | PARAM_MAXRANGE)) { // !!!! undocumented - allowable dac range
         send_tsip_start(0x8E);
         send_byte(0xA8);
         send_byte(0x04);
         send_single(user_min_range);
         send_single(user_max_range);
         send_tsip_end();
      }

      if(save) {
         if((have_osc_params & PARAM_TBOLT) == PARAM_TBOLT) {  // !!!!! 0x0F: we have all osc params available
            save_segment(9, 25);
         }
         else if(saw_ntpx && ((have_osc_params & PARAM_NTPX) == PARAM_NTPX)) {
            save_segment(9, 26);
         }
      }
   }
   else if(rcvr_type == UCCM_RCVR) {
      if(have_osc_params & PARAM_PULLIN) {
         set_pullin_range(user_pullin);
      }
   }
}


void request_survey_params()
{
   if(luxor) return;
   if(no_poll) return;

   if(rcvr_type == MOTO_RCVR) {
      send_moto_start("Gd");
      send_byte(0xFF);
      send_moto_end();
   }
   else if(rcvr_type == TSIP_RCVR) {
      send_tsip_start(0x8E);
      send_byte(0xA9);
      send_tsip_end();
   }
   else if(rcvr_type == UBX_RCVR) {  // zorky
      request_ubx_msg(UBX_CFG_TMODE);
      request_ubx_msg(UBX_SVIN);
   }
   else if(rcvr_type == VENUS_RCVR) {
      query_venus_msg(QUERY_VENUS_SURVEY);
   }
}

void set_survey_params(u08 enable_survey,  u08 save_survey, u32 survey_len)
{
   if(read_only) return;
   if(luxor) return;

   if(rcvr_type == MOTO_RCVR) {  //!!! moto_chans
      send_moto_start("Gd");
      if(enable_survey) send_byte(0x03);
      else              send_byte(0x00);
      send_moto_end();
   }
   else if(rcvr_type == NVS_RCVR) {
      if(survey_len > 1440) survey_len = 1440;
      else if(survey_len < 20) survey_len = 20;
      send_nvs_start(0x1D);
      send_byte(0x06);
      send_word(survey_len);
      send_nvs_end();
   }
   else if(rcvr_type == TSIP_RCVR) {
      send_tsip_start(0x8E);
      send_byte(0xA9);
      send_byte(enable_survey);
      send_byte(save_survey);
      send_dword(survey_len);
      send_dword(0x0000);
      send_tsip_end();
   }
   else if(rcvr_type == UBX_RCVR) {  // zorky
   }
}

void start_self_survey(u08 val, int why)
{
   // val=0 - start survey
   // val=1 - save position (Tbolt-E)
   // val=2 - delete position (Tbolt-E)
   if(read_only) return;
   if(luxor) return;
stop_precision_survey();

   if(rcvr_type == MOTO_RCVR) {  //!!! moto_chans
      send_moto_start("Gd");
      send_byte(0x03);
      send_moto_end();
   }
   else if(rcvr_type == NVS_RCVR) {
      send_nvs_start(0x1D);
      send_byte(0x00);
      send_byte(0x02);
      send_nvs_end();
   }
   else if(rcvr_type == SCPI_RCVR) {
// !!! should adjust com_timeout
      if(scpi_type == NORTEL_TYPE) {
         queue_scpi_cmd("GPS:POS:SURVEY:STAT ONCE", SCPI_SURVEY_MSG);
         queue_scpi_cmd("GPS:POS:SURVEY:PROGRESS?", SCPI_PROGRESS_MSG);
      }
      else {
         queue_scpi_cmd(":PTIM:GPS:POS:SURV:STAT ONCE", SCPI_SURVEY_MSG);
         queue_scpi_cmd(":PTIM:GPS:POS:SURV:STAT?", SCPI_SURVEY_MSG);
      }
if(debug_file) fprintf(debug_file, "### start survey: why=%d  do_survey=%ld\n", why, do_survey);
if(debug_file) fflush(debug_file);
do_survey = 0;
survey_why = (-11);
   }
   else if(rcvr_type == TSIP_RCVR) {
      send_tsip_start(0x8E);
      send_byte(0xA6);
      send_byte(val);
      send_tsip_end();
   }
   else if(rcvr_type == UBX_RCVR) {  // zorky
      set_ubx_config(1, lat,lon,alt);  // go to survey mode
   }
   else if(rcvr_type == UCCM_RCVR) {
if(scpi_type == UCCMP_TYPE) {  // kkkkkk
      queue_scpi_cmd("GPS:POS:SURV:STAT ONCE", SCPI_SURVEY_MSG);
if(debug_file) fprintf(debug_file, "### start survey: why=%d  do_survey=%ld\n", why, do_survey);
if(debug_file) fflush(debug_file);
do_survey = 0;
survey_why = (-12);
      queue_scpi_cmd(":GPS:POS:SURV:STAT?", SCPI_SURVEY_MSG);
}
else {
      queue_scpi_cmd("SYST:PRES", SCPI_RESET_MSG);  // use factory reset to start survey mode
      queue_scpi_cmd("SYST:STAT?", UCCM_STATUS_MSG);
}
   }
   else if(rcvr_type == VENUS_RCVR) { // gggg
      set_venus_survey(1, lat,lon,alt);  // start self survey
   }
   else if(rcvr_type == ZODIAC_RCVR) {
      set_zod_config(3, 0.0,0.0,0.0, traim_threshold, 7);
   }

   if(val == 0) {
      surveying = 1;
   }
// plot_lla = 1;
// show_fixes = 1;
   if(lat || lon || alt) { // gggggggg
      precise_lat = lat;
      precise_lon = lon;
      precise_alt = alt;

      #ifdef BUFFER_LLA
         clear_lla_points();
      #endif
   }
   request_survey_params();
}

void stop_self_survey()
{
// set_survey_params(0, 0, 0L);
// start_self_survey(0, 5);
   if(read_only) return;
   if(luxor) return;

   if(rcvr_type == MOTO_RCVR) {
      set_rcvr_mode(RCVR_MODE_HOLD);   // position hold mode
   }
   else if(rcvr_type == NVS_RCVR) {
      send_nvs_start(0x1D);
      send_byte(0x00);
      send_byte(0x01);
      send_nvs_end();
   }
   else if(rcvr_type == SCPI_RCVR) { 
if(debug_file) fprintf(debug_file, "### stop survey\n");
if(debug_file) fflush(debug_file);
// !!! should adjust com_timeout
//    queue_scpi_cmd(":GPS:POS LAST", SCPI_POS_MSG);   // use old saved position
      if(scpi_type == NORTEL_TYPE) {
         set_lla(lat,lon,alt);
         queue_scpi_cmd("GPS:POS:SURVEY:PROGRESS?", SCPI_PROGRESS_MSG);
      }
      else {
         queue_scpi_cmd(":GPS:POS SURV", SCPI_POS_MSG);   // use position calculated so far
         queue_scpi_cmd(":PTIM:GPS:POS:SURV:STAT?", SCPI_SURVEY_MSG);
      }
   }
   else if(rcvr_type == TSIP_RCVR) {
      set_rcvr_mode(RCVR_MODE_HOLD);
   }
   else if(rcvr_type == UBX_RCVR) {  // zorky // go to position hold at current LLA
      set_ubx_config(2, lat,lon,alt);  
   }
   else if(rcvr_type == UCCM_RCVR) {  // go to position hold at current LLA
      set_lla(lat,lon,alt);
   }
   else if(rcvr_type == VENUS_RCVR) { 
      set_venus_survey(2, lat,lon,alt); // go to position hold at current LLA 
   }
   else if(rcvr_type == ZODIAC_RCVR) {
      set_zod_config(5, 0.0,0.0,0.0, traim_threshold, 8);
   }
}


void request_primary_timing()
{
   if(luxor) return;
   if(no_poll) return;
   if(rcvr_type != TSIP_RCVR) return;

   send_tsip_start(0x8E);
   send_byte(0xAB);
   send_byte(0x02);
   send_tsip_end();
}

void request_secondary_timing()
{
   if(luxor) return;
   if(no_poll) return;
   if(rcvr_type != TSIP_RCVR) return;

   send_tsip_start(0x8E);
   send_byte(0xAC);
   send_byte(0x02);
   send_tsip_end();
}

void request_ae_packet(u08 subcode)
{
   if(luxor) return;
   if(no_poll) return;
   if(rcvr_type != TSIP_RCVR) return;

   send_tsip_start(0x8E);
   send_byte(0xAE);
   send_byte(subcode);
   send_tsip_end();
}


//
//
//   Incoming TSIP message handlers
//
//


void ebolt_version()
{   // ThunderBolt-E only
u08 rsvd1;
u08 major;
u08 minor;
u08 build;
u08 len;
int i;

// printf("Packet 1C (Tbolt-E version):\n");
   rev_month = rev_day = rev_hour = 0;
   rev_year = 0;

   subcode = tsip_byte();
   if(subcode == 0x81) {  // firmware version
      rsvd1 = tsip_byte();
      major = tsip_byte();
      minor = tsip_byte();
      build = tsip_byte();
      rev_month = tsip_byte();
      rev_day = tsip_byte();
      rev_year = tsip_word();
      len = tsip_byte();
  
      out[0] = 0;
      for(i=0; i<len; i++) {
         if(i >= MAX_TEXT_COLS) {
            tsip_byte();
            continue;
         }
         out[i] = tsip_byte();
         out[i+1] = 0;
      }
  
      strupr(out);
   }
   else if(subcode == 0x83) {  // hardware version
      ebolt_serno = tsip_dword();
      rev_day = tsip_byte();
      rev_month = tsip_byte();
      rev_year = tsip_word();
      rev_hour = tsip_byte();
      hw_code = tsip_word();
      len = tsip_byte();

      for(i=0; i<UNIT_LEN; i++) unit_name[i] = ' ';
      unit_name[UNIT_LEN] = 0;

      out[0] = 0;
      for(i=0; i<len; i++) {   // get hardware id
         if(i >= MAX_TEXT_COLS) {
            tsip_byte();
            continue;
         }
         out[i] = tsip_byte();  // string can have: ICM, RES SMT, RES360
         out[i+1] = 0;
         if(i < UNIT_LEN) {
            unit_name[i] = out[i];
         }
      }

      strupr(out);
      if(res_t == 0) {
         if(strstr(out, "ICM")) { // !!!!!! ICM RES SMT
            saw_icm = 1;
            saw_gpsdo = 2;
         }
         else if(strstr(out, "RESOLUTION")) {
            if(user_set_res_t) ;
            else if(strstr(out, "SMT")) res_t = RES_T_SMT;
            else                        res_t = RES_T;
            config_msg_ofs();
            config_rcvr_plots();
         }
         else if(strstr(out, "RES SMT")) {
            if(!user_set_res_t) res_t = RES_T_RES;
            config_msg_ofs();
            config_rcvr_plots();
         }
         else if(strstr(out, "RES360")) {
            if(!user_set_res_t) res_t = RES_T_360;
            config_msg_ofs();
            config_rcvr_plots();
         }
      }
      if(mini_t == 0) {
         if(strstr(out, "MINI")) {
            mini_t = 1;
            saw_mini = 1;
         }
      }
      strncpy(unit_name, out, UNIT_LEN);
      unit_name[UNIT_LEN-1] = 0;
   }
   else {  // unknown
      //!!! process packet contents
   }

   tsip_end(1);   
   if(tsip_error) return;

   saw_ebolt();
   tbolt_e |= 0x08;
   saw_version |= 0x02;

   if(1) {  //!!!!!
      if(rev_month > 12) return;
      if(rev_month == 0) return;
      if(rev_day > 31) return;
      if(rev_day == 0) return;
      if(rev_year < 1990) return;
      if(rev_year > 2100) return;
      if(rev_hour > 24) return;
   }

   if(text_mode && first_key) return;
   if(zoom_screen) return;
   if(luxor) return;
if(just_read) return;

   if(0 && (subcode == 0x83)) {  // !!! disabled since hw_code clashes with sn_prefix
      show_ebolt_info();
   }
}


void rcvr_config()
{
u08 rcvr_mode;
u08 rsvd1;
u08 rsvd2;
u08 rsvd3;
u08 rsvd4[17];
u08 sat_types;
u08 i;
float el_val;
float amu_val;

// printf("Packet BB (Receiver config):\n");

   subcode = tsip_byte();
   rcvr_mode = tsip_byte();
   rsvd1 = tsip_byte();
   dynamics_code = tsip_byte();
   rsvd2 = tsip_byte();
   el_val = tsip_single();
   amu_val = tsip_single();
   pdop_mask = tsip_single();
   pdop_switch = tsip_single();
   rsvd3 = tsip_byte();
   foliage_mode = tsip_byte();  // saw_icm: anti-jam mode

   // byte 28: sat type mask for OD clock mode saw_icm
   //          0x01 GPS
   //          0x02 GLONASS
   //          0x08 Beidou
   //          0x10 Galileo
   //          0x20 QZSS 
   for(i=24; i<=40; i++) {   
      rsvd4[i-24] = tsip_byte();
      if((i == 28) && (saw_icm || (res_t == RES_T_360))) { // saw_icm: byte 28 = OD clock mode sat type bit mask
         sat_types = rsvd4[i-24];
         if(sat_types == 0) ;
         else if(sat_types == 0xFF) ;
         else {
            gnss_mask = 0;
            if(sat_types & 0x01) gnss_mask |= GPS;
            if(sat_types & 0x02) gnss_mask |= GLONASS;
            if(sat_types & 0x08) gnss_mask |= BEIDOU;
            if(sat_types & 0x10) gnss_mask |= GALILEO;
            if(sat_types & 0x20) gnss_mask |= QZSS;
            if(gnss_mask == 0) {
               gnss_mask = MIXED;
               have_gnss_mask = 0;
            }
            else have_gnss_mask = 4;
         }
      }
   }
   tsip_end(1);

   // stupid filtering required for STARLOC II devices
   el_val = (float) (int) ((el_val*RAD_TO_DEG) + 0.50F);
   if(tsip_type == STARLOC_RCVR) ;
   else if((el_val >= 0.0) && (el_val <= 90.0)) {
      el_mask = el_val;
      have_el_mask = 1;
   }

   if(tsip_type == STARLOC_RCVR) ;
   else if((amu_val >= 0.0) && (amu_val < 60.0)) {
      amu_mask = amu_val;
      have_amu = 1;
   }
   if(tsip_error) return;
}

void get_gps_time()
{
// printf("Packet 41 - GPS time\n");
float gps_tow;
u16 gps_week;
float gps_utc_offset;

   gps_tow = tsip_single();
   gps_week = tsip_word();
   gps_utc_offset = tsip_single();
   tsip_end(1);

// check_utc_ofs(1);
   if(tsip_error) return;
}

void single_ecef_fix()
{
float fix_x,fix_y,fix_z;
float time_of_fix;

// printf("Packet 42 (XYZ ECEF):\n");
   fix_x = tsip_single();
   fix_y = tsip_single();
   fix_z = tsip_single();
   time_of_fix = tsip_single();
   tsip_end(1);
   if(tsip_error) return;
}


void get_pps_mode()
{
// printf("Packet 8F 4E (pps rate):\n");
   pps_rate = tsip_byte();
   have_pps_rate = 2;
   tsip_end(1);
   if(tsip_error) return;
}

void velocity_fix()
{
float x_vel,y_vel,z_vel;
float bias_rate;
float time_of_fix;

// printf("Packet 43 (XYZ ECEF velocity):\n");

   x_vel = tsip_single();
   y_vel = tsip_single();
   z_vel = tsip_single();
   bias_rate = tsip_single();
   time_of_fix = tsip_single();
   tsip_end(1);
   if(tsip_error) return;
}


void version_info()
{
// printf("Packet 45 (Software version):\n");

   ap_major = tsip_byte();
   ap_minor = tsip_byte();
   ap_month = tsip_byte();
   ap_day = tsip_byte();
   ap_year = tsip_byte();
   if(ap_year > 80) ap_year += 1900;
   else             ap_year += 2000;

   core_major = tsip_byte();
   core_minor = tsip_byte();
   core_month = tsip_byte();
   core_day = tsip_byte();
   core_year = tsip_byte();
   if(core_year > 80) core_year += 1900;
   else               core_year += 2000;

   tsip_end(1);
   if(tsip_error) return;

   saw_version |= 0x01;

   if(ap_month > 12) return;
   if(ap_month == 0) return;
   if(ap_day > 31) return;
   if(ap_day == 0) return;
   if(core_month > 12) return;
   if(core_month == 0) return;
   if(core_day > 31) return;
   if(core_day == 0) return;
   if(ap_year < 1980) return;
   if(ap_year > 2080) return;
   if(core_year < 1980) return;
   if(core_year > 2080) return;

   have_info |= VERSION_INFO;

   if(text_mode && first_key) return;
   if(zoom_screen) return;
   if(luxor) return;
if(just_read) return;

   show_version_header();
   show_unit_info();
}

void ebolt_health1()
{
u08 sv_fix;
u08 antenna_fault;

// printf("Packet 46 (Tbolt-E health):\n");

   sv_fix = tsip_byte();
   antenna_fault = tsip_byte();
   tsip_end(1);
   if(tsip_error) return;

   saw_version |= 0x04;
   saw_ebolt();
   tbolt_e |= 0x01;
}


void sig_levels()
{
u08 count;
int i;
int prn;
float sig_level;

// printf("Packet 47 (Signal levels):\n");

   for(i=1; i<=MAX_PRN; i++) {  // tttttt
      sat[i].level_msg = 0x00;
////  sat[i].tracking = 0x00;
      if(1 || log_db) sat[i].sig_level = 0;
   }

   count = tsip_byte();
   for(i=0; i<count; i++) {
      prn = tsip_byte();
      sig_level = tsip_single();
      if(prn > MAX_PRN) prn = 0;  // put any bogus data in unused entry in array
      sat[prn].sig_level = sig_level;
      sat[prn].level_msg = 0x47;
   }
   tsip_end(1);
   if(tsip_error) return;
}


void get_alm_health()
{
int i;

// printf("Packet 49 (Almanac health page):\n");
   for(i=1; i<=MAX_PRN; i++) {
//    sat[i].health_flag = tsip_byte();
      if(tsip_byte()) sat[i].health_flag = 1;  // unhealthy
      else            sat[i].health_flag = 0;  // healthy
   }
   tsip_end(1);
   if(tsip_error) return;
}

void single_lla_fix()
{
float lat, lon, alt;
float clock_bias;
float time_of_fix;

// printf("Packet 4A (LLA fix):\n");
   lat = tsip_single();
   lon = tsip_single();
   alt = tsip_single();
   clock_bias = tsip_single();
   time_of_fix = tsip_single();
   tsip_end(1);
   if(tsip_error) return;

// show_lla(100);
}

void ebolt_health2()
{
u08 id;
u08 rtc;
u08 superpackets;

// printf("Packet 4B (Tbolt-E health):\n");

   id = tsip_byte();
   rtc = tsip_byte();
   superpackets = tsip_byte();
   tsip_end(1);
   if(tsip_error) return;

   saw_version |= 0x08;
   saw_ebolt();
   tbolt_e |= 0x02;
}

void io_options()
{
u08 posn;
u08 vel;
u08 timing;
u08 aux;

// printf("Packet 55 (I/O options):\n");

   posn = tsip_byte();
   vel = tsip_byte();
   timing = tsip_byte();
   aux = tsip_byte();
   if(aux & 0x08) {
      level_type = "dBc";
      amu_mode = 0;
   }
   else {
      level_type = "AMU";
      amu_mode = 1;
   }
   tsip_end(1);
   if(tsip_error) return;
}


void enu_velocity_fix()
{
float x_vel,y_vel,z_vel;
float bias_rate;
float time_of_fix;

// printf("Packet 56 (ENU velocity):\n");

   x_vel = tsip_single();
   y_vel = tsip_single();
   z_vel = tsip_single();
   bias_rate = tsip_single();
   time_of_fix = tsip_single();
   tsip_end(1);
   if(tsip_error) return;
}


void last_fix_info()
{
u08 source_of_fix;
u08 tracking_mode;
float time_of_fix;
u16 week_of_fix;

// printf("Packet 57 (last fix info):\n");

   source_of_fix = tsip_byte();
   tracking_mode = tsip_byte();
   time_of_fix = tsip_single();
   week_of_fix = tsip_word();
   tsip_end(1);
   if(tsip_error) return;
}


void packet_58()
{
u08 op;
u08 type;
int prn;
u08 len;
u08 i;

double a0;
float a1;
short delta_ls;
float tot;
double wn_lsf;
unsigned short wnt;
unsigned short dn;
short delta_lsf;
int wraps;

// printf("Packet 58 (GPS system data):\n");
   op = tsip_byte();
   type = tsip_byte();
   prn = tsip_byte();
   len = tsip_byte();

   if(type == 0x05) {  // UTC data
      for(i=4; i<=16; i++) {
         tsip_byte();
      }                      
      a0 = tsip_double();
      a1 = tsip_single();
      delta_ls = tsip_word();
      tot = tsip_single();
      wnt = tsip_word();
      wn_lsf = (double) (short) tsip_word();
      wraps = gps_week / 1024;
      wn_lsf += (wraps * 1024);
//      while(wn_lsf < (double) gps_week) {  // adjust for week rollovers
//         wn_lsf += 1024.0;
//      }
      dn = tsip_word();
      delta_lsf = tsip_word();

      calc_leap_days(wn_lsf, dn, 10);

//sprintf(plot_title, "dls:%d  wnlsf:%g  dn:%d  dlsf:%d   wks:%d  days:%d  %d/%d/%d", 
//delta_ls, wn_lsf, dn, delta_lsf, (int) wn_lsf-gps_week, leap_days, g_year,g_month,g_day);
   }
   else {
      for(i=0; i<len; i++) {   //!!! we should probably process this data
         tsip_byte();
      }
   }

   tsip_end(1);
   if(tsip_error) return;
}

void sat_health()
{
u08 op;
u08 vals[1+32];
u08 i;
u32 val;

// printf("Packet 59 (Sat disable/health):\n");

   op = tsip_byte();
   if(op == 3) {  // sat selection enabled / disabled
      val = 0;
      for(i=1; i<=32; i++) {
         sat[i].disabled = tsip_byte();
         if(!sat[i].disabled) val |= 1 << (i-1);
      }
      sats_enabled = val;
   }
   else if(op == 6) {  // sat health enabled / disabled
      for(i=1; i<=32; i++) sat[i].forced_healthy = tsip_byte();
   }
   else if((op == 0) && (tsip_type == STARLOC_RCVR)) { // bogus packet!
      for(i=1; i<=31; i++) vals[i] = tsip_byte();  // note: 00 op plus 31 bytes of zeroes
   }
   else {  // unknown type field
      for(i=1; i<=32; i++) vals[i] = tsip_byte();
   }
   tsip_end(1);
   if(tsip_error) return;
}

void raw_data()
{
int prn;

// printf("Packet 5A (Raw measurement data):\n");

   prn = tsip_byte();
   if(prn > MAX_PRN) prn = 0;  // !!!! max_sat_check - put any bogus data in unused entry in array

   sat[prn].sample_len = tsip_single();
   if(1 || (log_db == 0)) sat[prn].sig_level = tsip_single();
   else tsip_single();
   sat[prn].level_msg = 0x5A;
   sat[prn].code_phase = tsip_single();
   sat[prn].doppler = tsip_single();
   sat[prn].raw_time = tsip_double();
   tsip_end(1);

   if(sat[prn].doppler != 0.0F) {
      if(have_doppler == 0) need_redraw = 2002;
      have_doppler = 1;
   }

   if(sat[prn].code_phase != 0.0F) {
      if(have_phase == 0) need_redraw = 2003;
      have_phase = 1;
   }

   if(tsip_error) return;
}

void eph_status()
{
int prn;

// printf("Packet 5B (Sat ephemeris status):\n");

   prn = tsip_byte();
   if((prn == 0) && (tsip_type == STARLOC_RCVR)) {  // bogus message
      for(prn=1; prn<=15; prn++) tsip_byte();
   }
   else {
      if(prn > MAX_PRN) prn = 0;  // !!!! max_sat_check - put any bogus data in unused entry in array
      sat[prn].eph_time = tsip_single();
      sat[prn].eph_health = tsip_byte();
      sat[prn].iode = tsip_byte();
      sat[prn].toe = tsip_single();
      sat[prn].fit_flag = tsip_byte();
      sat[prn].sv_accuracy = tsip_single();
      tsip_end(1);

      if(have_accu == 0) need_redraw = 2004;
      have_accu = 1;
   }
   if(tsip_error) return;
}

void sat_tracking(int msg_fmt)
{
int prn;
float el;

// printf("Packet 5C (Sat tracking):\n");
// also packet 5d

   have_sat_azel = 1;
   prn = tsip_byte();
   if(prn > MAX_PRN) prn = 0; // !!!! max_sat_check -  put any bogus data in unused entry in array
   else if(prn < 1) prn = 0;  // !!!! max_sat_check -  put any bogus data in unused entry in array

   sat[prn].chan = tsip_byte();
   sat[prn].chan >>= 3;
   sat[prn].acq_flag = tsip_byte();
   sat[prn].eph_flag = tsip_byte();
   if(1 || (log_db == 0)) sat[prn].sig_level = tsip_single();
   else            tsip_single();
   sat[prn].level_msg = 0x5C;
   sat[prn].time_of_week = tsip_single();

   el = tsip_single() * (float) RAD_TO_DEG;
   set_sat_el(prn, el);

   sat[prn].azimuth = tsip_single() * (float) RAD_TO_DEG;
//sprintf(plot_title, "trk msg %02X prn:%02d  az:%p el:%p", msg_fmt, prn, sat[prn].azimuth,sat[prn].elevation);
   sat[prn].age = tsip_byte();
   sat[prn].msec = tsip_byte();
   sat[prn].bad_flag = tsip_byte();
   sat[prn].collecting = tsip_byte();
   if(msg_fmt == 0x5D) {  // RES 360 message 0x5D
      sat[prn].how_used = tsip_byte();
      // sv_type for icm receivers:
      //   0=GPS
      //   1=Glonass
      //   2=Beidou?
      //   3=Galileo?
      //   4=SBAS
      //   5=QZSS
      sat[prn].sv_type = tsip_byte();
   }
   tsip_end(1);
   if(tsip_error) {
      sat[prn].level_msg = 0x00;
      return;
   }

   record_sig_levels(prn);
}


void eeprom_status()
{
u08 flag;
u16 ee_status;

// printf("Packet 5F (EEPROM status):\n");

   flag = tsip_byte();
   if(flag == 0x11) {
      ee_status = tsip_word();
   }

   tsip_end(1);
   if(tsip_error) return;
}


void sat_list(int flag)
{
u08 dimension;
u08 count;
int i;
int prn;

// printf("Packet 6D (Satellite list):\n");  // also 6C

   dimension = count = tsip_byte();
   dimension &= 0x07;
   count >>= 4;

   have_dops = 0;
   pdop = tsip_single();
   if(pdop > 20.0) pdop = 20.0;

   hdop = tsip_single();
   if(hdop > 20.0) hdop = 20.0;

   vdop = tsip_single();
   if(vdop > 20.0) vdop = 20.0;

   tdop = tsip_single();
   if(tdop > 20.0) tdop = 20.0;

   have_dops |= (PDOP | HDOP | VDOP | TDOP);

   for(i=1; i<=MAX_PRN; i++) {  // reset current tracking flags
      sat[i].tracking = 0;
   }

   if(flag) {
      count = tsip_byte();
   }

   for(i=0; i<count; i++) {
      prn = tsip_byte();
      if(prn & 0x80) {  // prn tracked,  but not used
         prn |= (~0xFF);
      }
      if(prn > MAX_PRN) prn = 0;  // put any bogus data in unused entry in array
      else if(prn < (-MAX_PRN)) prn = 0;

      if(prn < 0) {
         sat[0-prn].tracking = (-1); // prn;
      }
      else {  // prn is used in solution
         sat[prn].tracking = (1); // prn;
      }
   }
   tsip_end(1);
   if(tsip_error) return;
}


void filter_config()
{
// printf("Packet 70 (Filter config):\n");

   pv_filter = tsip_byte();
   static_filter = tsip_byte();
   alt_filter = tsip_byte();
   kalman_filter = tsip_byte();
   have_filter = (PV_FILTER | STATIC_FILTER | ALT_FILTER | KALMAN_FILTER);
   tsip_end(1);
   if(tsip_error) return;

   saw_version |= 0x80;
if(just_read) return;

   saw_kalman_on |= kalman_filter;
}


void ecef_fix()
{
double clock_bias;
double fix_x,fix_y,fix_z;
float time_of_fix;

// printf("Packet 83 (XYZ ECEF):\n");

   fix_x = tsip_double();
   fix_y = tsip_double();
   fix_z = tsip_double();
   clock_bias = tsip_double();
   time_of_fix = tsip_single();
   tsip_end(1);
   if(tsip_error) return;
}


void lla_fix()
{
double lat, lon, alt;
double clock_bias;
float time_of_fix;

// printf("Packet 84 (LLA fix):\n");

   lat = tsip_double();
   lon = tsip_double();
   alt = tsip_double();
   #ifdef PRECISE_STUFF
      precise_check();
   #endif
   clock_bias = tsip_double();
   time_of_fix = tsip_single();
   tsip_end(1);
   if(tsip_error) return;
// show_lla(200);
}

void datums()
{
int index;
double dx;
double dy;
double dz;
double a_axis;
double ecc;

// printf("Packet 8F.15 (Datums):\n");

   index = tsip_word();
   dx = tsip_double();
   dy = tsip_double();
   dz = tsip_double();
   a_axis = tsip_double();
   ecc = tsip_double();
   tsip_end(1);
   if(tsip_error) return;
}


void fix_manuf_params()
{
   if(luxor) return;
   if(rcvr_type != TSIP_RCVR) return;

   send_tsip_start(0x8E);
   send_byte(0x41);
   send_word(0x1234);           //prefix
   send_dword(0x56789ABCL);     // sn
   send_byte(10);               // yr
   send_byte(1);    // mo
   send_byte(2);    //day
   send_byte(3);    // hr
   send_single(0.0F);    // build offset
   send_word(0);    // test code
   send_tsip_end();

   save_segment(0xFF, 27);
}

void manuf_params()
{
u16 test_code;
int sum;

// printf("Packet 8F.41 (Manufacturing Params):\n");

   sn_prefix = tsip_word();
   serial_num = tsip_dword();
   build_year = sum = tsip_byte();
   if(build_year > 80) build_year += 1900;
   else                build_year += 2000;
   build_month = tsip_byte();
   build_day = tsip_byte();
   build_hour = tsip_byte();
   build_offset = tsip_single();
   test_code = tsip_word();
   tsip_end(1);
   if(tsip_error) return;

   sum += sn_prefix + serial_num + test_code + build_month + build_day + build_hour;  // check for zeroed build params
   build_ok = ((build_month <= 12) && (build_month != 0) && 
               (build_day <= 31) && (build_day != 0) &&
               (build_year >= 1980) && (build_year <= 2080) &&
               (build_hour <= 24));

   have_build |= 0x01;
   if(build_ok) have_build |= 0x02;


   if((sum == 0) || build_ok) {
      show_manuf_params();
   }

   if(text_mode && first_key) return;
   if(zoom_screen) return;
   if(luxor) return;
   if(just_read) return;

   show_serial_info();
}

void prodn_params()
{
u16 rsvd2;
u16 rsvd1;

// printf("Packet 8F.42 (Production Params):\n");

   prodn_options = tsip_byte();
   prodn_extn = tsip_byte();
   case_prefix = tsip_word();
   case_sn = tsip_dword();
   prodn_num = tsip_dword();
   rsvd1 = tsip_word();
   machine_id = tsip_word();
   rsvd2 = tsip_word();
   tsip_end(1);
   if(tsip_error) return;

   have_info |= PRODN_PARAMS;
}

void pps_settings()
{
u08 pps_rsvd;
float bias_threshold;

// printf("Packet 8F.4A (PPS settings):\n");

   pps_enabled = tsip_byte();
   have_pps_enable = 1;
   pps_rsvd = tsip_byte();
   pps_polarity = tsip_byte();
   have_pps_polarity = 1;
   cable_delay = tsip_double();
   bias_threshold = tsip_single();
   tsip_end(1);

// if(have_cable_delay == 0) need_redraw = 2005;
   have_cable_delay = 1;
   if(tsip_error) return;

   if(text_mode && first_key) return;
   if(zoom_screen) return;
   if(luxor) return;
if(just_read) return;

   saw_version |= 0x40;
   show_cable_delay();
}


void dac_values()
{
u32 dac_value;
float dac_voltage;
u08 dac_res;
u08 dac_format;
float dac_min, dac_max;

// printf("Packet 8F.A0 (DAC values):\n");

   dac_value = tsip_dword();
   dac_voltage = tsip_single();
   dac_res = tsip_byte();
   dac_format = tsip_byte();
   dac_min = tsip_single();
   dac_max = tsip_single();
   tsip_end(1);
   if(tsip_error) return;
}


void osc_sense()
{  // not avilable on ThunderBolt-E or early ThunderBolts
// printf("Packet 8F.A1 (10 MHz sense):\n");

   osc_polarity = tsip_byte();
   have_osc_polarity = 1;
   tsip_end(1);
   if(tsip_error) return;
}


void get_timing_mode()
{
// printf("Packet 8F.A2 (Timing mode):\n");

   timing_mode = tsip_byte();
   have_timing_mode = 1;
   tsip_end(1);
   if(tsip_error) return;
}


void packet_mask()
{
u16 mask1;
u16 mask2;

// printf("Packet 8F.A5 (Packet mask):\n");

   mask1 = tsip_word();
   mask2 = tsip_word();
   tsip_end(1);
   if(tsip_error) return;
}


void sat_solutions()
{   //not available on ThunderBolt-E
u08 format;
u32 time_of_week;
float clock_bias;
float clock_bias_rate;
float sat_bias;
int prn;
int i;

// printf("Packet 8F.A7 (Satellite solutions):\n");
   if(tsip_type == STARLOC_RCVR) {  // !!!! who knows what garbage this device craps out?
      // some of these are 9 bytes long, some are 13... we just ignore this message
      return;
   }

   format = tsip_byte();
   time_of_week = tsip_dword();

   if(format == 0) {     // floating point
      clock_bias = tsip_single();
      clock_bias_rate = tsip_single();
   }
   else if(format == 1) {   // integer values
      clock_bias = (float) (int) tsip_word();
      clock_bias *= 100.0e-12F;
      clock_bias_rate = (float) (int) tsip_word();
      clock_bias_rate *= 1.0e-12F;
   }
   else {  // invalid format
      tsip_end(1);
      if(tsip_error) return;
      return;
   }

   for(i=1; i<=MAX_PRN; i++) {  // reset current bias flags
      sat[i].last_bias_msg = 0;
   }

   for(i=0; i<8; i++) {  // get bias info from all satellites
      prn = tsip_byte();
      if(prn > MAX_PRN) prn = 0;  // put any bogus data in unused entry in array
      if(format == 0) {
         sat_bias = tsip_single();
      }
      else {
         sat_bias = (float) (int) tsip_word();
         sat_bias *= 100.0e-12F;
      }

      sat[prn].time_of_fix = (float) time_of_week;
      sat[prn].sat_bias = sat_bias;
      sat[prn].last_bias_msg = 1;

      if(have_bias == 0) need_redraw = 2006;
      have_bias = 1;
   }
   tsip_end(1);
   if(tsip_error) return;
}


void update_osc_params()
{
   user_time_constant = time_constant;           // get current values
   user_damping_factor = damping_factor;
   user_osc_gain = osc_gain;
   user_pullin = pullin_range;

   user_initial_voltage = initial_voltage;
   user_min_volts = min_volts;
   user_max_volts = max_volts;
   user_min_range = min_dac_v;
   user_max_range = max_dac_v;

   user_jam_sync = jam_sync;
   user_max_freq_offset = max_freq_offset;

   // update current values with user set values from command line
   if(user_set_osc & PARAM_TC)        user_time_constant = cmd_tc;
   if(user_set_osc & PARAM_DAMP)      user_damping_factor = cmd_damp;
   if(user_set_osc & PARAM_GAIN)      user_osc_gain = cmd_gain;
   if(user_set_osc & PARAM_PULLIN)    user_pullin = cmd_pullin;

   if(user_set_osc & PARAM_INITV)     user_initial_voltage = cmd_initdac;
   if(user_set_osc & PARAM_MINV)      user_min_volts = cmd_minv;
   if(user_set_osc & PARAM_MAXV)      user_max_volts = cmd_maxv;

   if(user_set_osc & PARAM_JAMSYNC)   user_jam_sync = cmd_jamsync;
   if(user_set_osc & PARAM_MAXFREQ)   user_max_freq_offset = cmd_maxfreq;

   if(user_set_osc & PARAM_MINRANGE)  user_min_range = cmd_minrange;
   if(user_set_osc & PARAM_MAXRANGE)  user_max_range = cmd_maxrange;
   user_set_osc = 0;

   set_discipline_params(0);
}

void discipline_params()
{
u08 type;
float v1,v2,v3;
float v4,v5,v6,v7;
int osc_ok;

// printf("Packet 8F.A8 (Discipline params):\n");

   type = tsip_byte();

   if(type == 0) {
      v1 = tsip_single();
      v2 = tsip_single();
      if(saw_ntpx) {
         v3 = tsip_single();
         v4 = tsip_single();
         v5 = tsip_single();
//sprintf(plot_title, "v: %f %f %f %f %f      ", v1,v2,v3,v4,v5);
      } 
      else if(try_nortel & 0x01) {
         v3 = tsip_single();
         v4 = tsip_single();
         v5 = tsip_single();
         v6 = tsip_single();
         v7 = tsip_single();
      }
      tsip_end(1);

      if(tsip_error) {
         try_nortel ^= 0x01;
         request_discipline_params(0x00);
         return;
      }

      time_constant = v1;
      have_tc = 0x01;

      damping_factor = v2;
      have_damp = 1;
      have_osc_params |= (PARAM_TC | PARAM_DAMP);

      if(saw_ntpx) {
         osc_gain = v3;
         min_volts = v4;
         max_volts = v5;
         have_minv = have_maxv = 1;
         have_osc_params |= (PARAM_GAIN | PARAM_MINV | PARAM_MAXV);
      }
      else if(try_nortel & 0x01) {
         osc_gain = v3;
         min_volts = v4;
         max_volts = v5;
         have_minv = have_maxv = 1;
         have_osc_params |= (PARAM_GAIN | PARAM_MINV | PARAM_MAXV);

         jam_sync = v6;
         have_jam_sync = 1;
         max_freq_offset = v7;
         have_freq_ofs = 1;
         have_osc_params |= (PARAM_JAMSYNC | PARAM_MAXFREQ);

         saw_nortel |= 0x01;
         saw_gpsdo = 4;
         if(user_set_temp_filter == 0) undo_fw_temp_filter = 0;
      }
   }
   else if(type == 1) {
      v1 = tsip_single();
      v2 = tsip_single();
      v3 = tsip_single();
      if(try_nortel & 0x02) {
         v4 = tsip_single();
         v5 = tsip_single();
      }
      tsip_end(1);
      if(tsip_error) {
         try_nortel ^= 0x02;
         request_discipline_params(0x01);
         return;
      }
      if(pause_data) return;   // in case reading log file,  we don't want to change the OSC_GAIN
                               // since the user may be calculating osc parameters
      gain_color = WHITE;
      osc_gain = v1;
      have_gain = 1;
      min_volts = v2;
      max_volts = v3;
      have_minv = have_maxv = 1;
      have_osc_params |= (PARAM_GAIN | PARAM_MINV | PARAM_MAXV);

      if(try_nortel & 0x02) {
         jam_sync = v4;
         have_jam_sync = 1;
         max_freq_offset = v5;
         have_freq_ofs = 1;
         saw_nortel |= 0x02;
         saw_gpsdo = 5;
         have_osc_params |= (PARAM_JAMSYNC | PARAM_MAXFREQ);
         if(user_set_temp_filter == 0) undo_fw_temp_filter = 0;
      }
   }
   else if(saw_ntpx) {  // packets 2,3,4 have no data
      tsip_end(1);
      if(tsip_error) return;
   }
   else if(type == 2) {
      v1 = tsip_single();
      v2 = tsip_single();
      tsip_end(1);
      if(tsip_error) return;

      jam_sync = v1;
      have_jam_sync = 1;
      max_freq_offset = v2;
      have_freq_ofs = 1;
      have_osc_params |= (PARAM_JAMSYNC | PARAM_MAXFREQ);
   }
   else if(type == 3) {
      v1 = tsip_single();
      tsip_end(1);
      if(tsip_error) return;

      initial_voltage = v1;
      have_initv = 1;
      have_osc_params |= (PARAM_INITV);
   }
   else if(type == 4) {     // undocumented - allowable dac range?
      v1 = tsip_single();
      v2 = tsip_single();
      tsip_end(1);
      if(tsip_error) return;

      min_dac_v = v1;
      max_dac_v = v2;
      have_dac_range = 1;
      have_osc_params |= (PARAM_MINRANGE | PARAM_MAXRANGE);
   }
   else {
      tsip_end(1);
      if(tsip_error) return;
   }

   if((have_osc_params & PARAM_TBOLT) == PARAM_TBOLT) {  // !!!!! 0x0F: we have all osc params available
      osc_ok = 1;
   }
   else if(saw_ntpx && ((have_osc_params & PARAM_NTPX) == PARAM_NTPX)) {
      osc_ok = 1;
   }
   else osc_ok = 0;


   if(1 && user_set_osc && osc_ok) {  // set osc params from command line values
      update_osc_params();
   }
}

void survey_params()
{
u08 survey_flag;
u32 rsvd;

// printf("Packet 8F.A9 (Survey params):\n");

   survey_flag = tsip_byte();
   survey_save = tsip_byte();
   survey_length = tsip_dword();
   rsvd = tsip_dword();
   tsip_end(1);
   if(tsip_error) return;
}

void primary_timing(int get_tsip)
{
int i;
double jd;
double rolls;

   // process the receiver primary timing message.  For TSIP receivers, gets
   // the timing message.

   skip_starloc_time = 0;
   old_hh = pt_hh;
   old_min = pt_min;
   old_sec = pt_sec;
   last_pri_tow = pt_tow;
   ++got_timing_msg;

// printf("Packet 8F.AB (Primary timing):\n");
   if(get_tsip) {
      pri_tow = this_pri_tow = pt_tow = tsip_dword();
      gps_week = tsip_word();

      i = (int) (s16) tsip_word();
      if(!user_set_utc_ofs && (i >= 0) && (i < 64)) {    // filter bogus values
         utc_offset = i;
         if(utc_offset != 0) utc_offset_flag = utc_offset;
         if(tsip_type == STARLOC_RCVR) {
            if(utc_offset_flag && (utc_offset == 0)) {
               utc_offset = utc_offset_flag;  // sometimes sends random 0 utc_offset value
            }
            tsip_byte();  // STARLOC_RCVR always sends bogus 0 time_flags value
         }
         else time_flags = tsip_byte();
      }
      else time_flags = tsip_byte();

      pri_seconds = pt_sec = tsip_byte();
      pri_minutes = pt_min = tsip_byte();
      pri_hours = pt_hh = tsip_byte();
      pri_day = tsip_byte();
      pri_month = tsip_byte();
      pri_year = tsip_word();
      pri_frac = 0.0;

      tsip_end(1);

      if(have_week == 0) need_redraw = 2007;
      have_week = 1;

      if(have_tow == 0) need_redraw = 2008;
      have_tow = 1;

      check_utc_ofs(2);
   }

   if(have_utc_ofs) time_flags &= (~0x0008);
   else             time_flags |= 0x0008;

   last_time_msec = this_time_msec;  // used to measure timing message jitter
   this_time_msec = GetMsecs();

   if(tsip_type == STARLOC_RCVR) {
      if(time_flags & 0x0001) {  // stupid DATUM receiver can output UTC time
         gps_to_utc();           // but always says it is GPS time.  We always run
      }                          // those turds in GPS mode and convert to UTC
   }

   if(pri_seconds == 60) leap_time = 1;
   else                  leap_time = 0;

   if(force_day) {
      pri_day = force_day;
      pri_month = force_month;
      pri_year = force_year;
   }

   if(user_set_rollover) ;               // user forced a rollover offset
   else if(rolled) ;                     // rollover has already been set
   else if(time_flags & 0x0008) ;        // GPS time is not valid
   else if(!have_utc_ofs) ;              // no UTC offset seen yet
   else if(pri_year < 1980) ;            // should never happen
   else if(pri_year < ROLLOVER_YEAR) {   // GPS year looks bogus
      if(roll_filter < ROLL_THRESH) {    // we need a few consecutive bogo years
         ++roll_filter;
      }
      else {
         get_clock_time();  // figure out how many times we rolled over from the system clock
         jd = jdate(pri_year,pri_month,pri_day) + jtime(pri_hours,pri_minutes,pri_seconds, pri_frac);
         rolls = (clk_jd -jd) / (19.0*365.0);  // 19.0 is (1024/52) rounded down (to allow for some system clock error)
//sprintf(plot_title, "clkjd:%.9f rjd:%.9f  diff:%.9f rolls:%f", clk_jd, jd, clk_jd-jd, rolls);
         rolls = (double) (int) rolls;
         if(rolls <= 1.0) rolls = 1.0;
         rollover = rolls * (1024.0 * 7.0 * 24.0*60.0*60.0);  // rollover one 1024 week epoch

//       setup_calendars(10);
      }
   }
   else roll_filter = 0;

   if(rollover || user_set_rollover) {  // adjust GPS receiver time by xxx seconds
// sprintf(debug_text, "rover:%f  rolled:%d  pyear:%d", rollover, user_set_rollover, pri_year);
      adjust_rcvr_time(rollover);
      if(rolled == 0) rolled = 1;
   }

   if(log_db) request_sig_levels();

   if(rcvr_type != TSIP_RCVR) ;  // ggggg
   else if(user_pps_len) {  // send string of user specified bytes each time a timing message is received
      for(i=0; i<user_pps_len; i++) sendout(user_pps_cmd[i]);
   }

   if(rcvr_type == SCPI_RCVR) ;
   else if(rcvr_type == STAR_RCVR) ;
   else if(rcvr_type == UCCM_RCVR) ;
   else if(tsip_error) {
      goto pt_exit; 
   }

   if(just_read) goto pt_exit; 

   if(1) {   // filter bogus time values
      i = 0;

//    if(pri_tow >= (60L*60L*24L*7L))   { write_log_error("tow", pri_tow);         ++i; }
      if(pri_seconds > 61)              { write_log_error("seconds", pri_seconds); ++i; } 
      if(pri_minutes > 60)              { write_log_error("minutes", pri_minutes); ++i; } 
      if(pri_hours > 24)                { write_log_error("hours", pri_hours);     ++i; } // accomodate weird leap seconds
      if(pri_day == 0)                  { write_log_error("day", pri_day);         ++i; } 
      if(pri_day > 31)                  { write_log_error("day", pri_day);         ++i; } 
      if(pri_month == 0)                { write_log_error("month", pri_month);     ++i; } 
      if(pri_month > 12)                { write_log_error("month", pri_month);     ++i; } 
      if(pri_year < 1980)               { write_log_error("year", pri_year);       ++i; } 
      if(pri_year > 2100)               { write_log_error("year", pri_year);       ++i; } 

      if(i) {
         goto pt_exit;
      }
   }

   // the time message seems to have valid time
   seconds = pri_seconds;     // save GPS/UTC time and date in global variables
   minutes = pri_minutes;
   hours = pri_hours;
   day = pri_day;
   month = pri_month;
   year = pri_year;
   raw_frac = pri_frac;

   tow = pri_tow;  
   this_tow = tow;
   survey_tow = tow;

   jd_utc = jd_gps = jdate(year,month,day) + jtime(hours,minutes,seconds,raw_frac);
   if(seconds == 60) leap_sixty = 1;  // used to restore xx:xx:60 leapsecond value
   else              leap_sixty = 0;  // ... after Julian -> Gregorian conversion

   calc_msg_ofs();

   if(no_poll) show_version_header();

   primary_misc();

   if(leap_time && leap_dump) {  // do screen dump if leap-second seen
      refresh_page();
      leap_time = 0;
      dump_screen(0, 0, "leap_sec");
   }

   pt_exit:
   // every time we see the 8F.AB message, 
   // ... we also request a different minor message
   request_misc_msg();  
   return;
}

float last_xt;
float x_temp;

int last_ma;

void secondary_timing(int get_tsip)
{
u08 spare1, spare2;
u32 dac_value;
u08 spare3[8];
int i;
int color;
u08 spare_data;
u08 survey_err;
float val;
double x_pps;
double x_osc;
long   x_val;
float  x_dac;
double x_lat;
double x_lon;
double x_alt;
int seq_err;

   // process the secondary timing message from the receiver (if any) and
   // validate the various receiver data values.  For TSIP devices, this routine
   // gets the secondary timing message from the device.

// printf("Packet 8F.AC (Secondary timing):\n");

   if(tsip_type == STARLOC_RCVR) {   // this turd can send secondary timing packets
      seq_err = this_pri_tow-(last_pri_tow+1);
      if(seq_err < (-1000)) seq_err += (7*24*60*60);  // week wrap
      if(seq_err == (-2)) return;  // skipped time stamp
      if(skip_starloc_time) return;  // without a preceeding primary timing packet
      skip_starloc_time = 1;
   }

   color = 0;

   if(get_tsip) {
      rcvr_mode = tsip_byte();
      discipline_mode = tsip_byte();
      survey_progress = tsip_byte();
      holdover = tsip_dword();
      critical_alarms = tsip_word();
      last_ma = minor_alarms;
      minor_alarms = tsip_word();
//if(last_ma != minor_alarms) sprintf(debug_text, "ma:%04X  last_ma:%04X", minor_alarms, last_ma); // rrrrr
      gps_status = tsip_byte();
      discipline = tsip_byte();
      spare1 = tsip_byte();
      spare2 = tsip_byte();
      x_pps = (double) tsip_single();
      x_osc = (double) tsip_single();
      x_val = tsip_dword();
      x_dac = tsip_single();
      if(res_t) have_sawtooth = 1;
      else have_dac = 1;
//      last_temp_val = temperature;
      last_xt = x_temp;
      x_temp= tsip_single();
      if(last_xt == 0.0F) last_xt = x_temp;
//      if(last_temp_val == 0.0F) last_temp_val = temperature;
      x_lat = tsip_double();
      x_lon = tsip_double();
      x_alt = tsip_double();

      pps_quant = tsip_single();  // res_t
      if(res_t) x_dac = pps_quant;

      spare_data = 0;  // flag set if any non-zero spare data fields 
      for(i=0; i<4; i++) {
         spare3[i] = tsip_byte();
         if(spare3[i]) ++spare_data;
      }

      tsip_end(1);

      if(!have_critical_alarms) need_redraw = 7701;

      have_antenna = 1;
      have_osc_age = 1;
      have_osc_offset = 1;
      have_pps_offset = 1;
      have_saved_posn = 1;
      have_tracking = 1;
      have_leap_info = 1;
      have_op_mode = 1;
      have_almanac = 1;
      have_critical_alarms = 1;
      have_gps_status = 1;
      have_temperature = 1;

      if(just_read) return;
      if(tsip_error) return;
   }
   else {
      x_pps = pps_offset;
      x_osc = osc_offset;
      x_val = 0;
      x_dac = dac_voltage;
      last_xt = x_temp;
      x_temp = temperature;
   }

   pps_offset = x_pps;
   osc_offset = x_osc;
   dac_value = x_val;
   dac_voltage = x_dac;
   last_temp_val = temperature;
// temperature = x_temp;
if(undo_fw_temp_filter && (luxor == 0) && (rcvr_type != UCCM_RCVR)) {
   temperature = (SENSOR_TC*x_temp) - ((SENSOR_TC-1.0F)*last_xt);  // unaverage the firmware reported temperature
   if((temperature < -100.0) || (temperature > 100.0F)) temperature = x_temp;  // filter problem
}
else {
   temperature = x_temp;
}
   if(last_temp_val == 0.0F) last_temp_val = temperature;
   if(get_tsip) {
      lat = x_lat;
      lon = x_lon;
      alt = x_alt;
   }

   if(plot[TEMP].plot_center == NEED_CENTER) plot[TEMP].plot_center = scale_temp(temperature);
   if(plot[TC2].plot_center == NEED_CENTER) plot[TC2].plot_center = scale_temp(tc2);
   if(plot[BATTV].plot_center  == NEED_CENTER) plot[BATTV].plot_center  = dac_voltage;
   if(luxor && (plot[LUX1].plot_center == NEED_CENTER)) plot[LUX1].plot_center = (float) (pps_offset * lux_scale);
   if(luxor && (plot[LUX2].plot_center == NEED_CENTER)) plot[LUX2].plot_center = (float) (pps_offset * lum_scale);

   if(survey_progress == 100) surveying = 0;
   if(doing_survey != last_survey) {  // self survey changed
      need_redraw = 2010;
   }
   last_survey = doing_survey;

   #ifdef PRECISE_STUFF
      precise_check();
   #endif

   if(crude_temp) {   // simulate the crude temperature sensor for temp ctrl pid testing
      val = temperature - (float) (int) temperature;
      val -= 0.75F;
      if((val >= 0.02) || (val < (-0.02))) {  // allow some noise around the switch point
         temperature = ((float) (int) temperature) + 0.75F;
      }
   }

   if(tsip_error) {  // don't pollute log with potentially bad data
      pps_offset = last_pps_offset;
      osc_offset = last_osc_offset;
      dac_voltage = last_dac_voltage;
      temperature = last_temp_val;
   }
   else {  // filter out bogus values
      if(TIMING_RCVR) ;
      else if(rcvr_type == UCCM_RCVR) ;  // kkkkkk
      else if(res_t == 0) {
         #ifdef __386__
            if(ABS(dac_voltage) <= 1.0E-4F) dac_voltage = 0.0F;
            if(ABS(temperature) <= 1.0E-4F) temperature = 0.0F;
         #endif

         if(luxor == 0) {
            if(dac_voltage == 0.0F) dac_voltage = last_dac_voltage;
            if(temperature == 0.0F) temperature = last_temp_val;
            if(pps_offset == 0.0F)  pps_offset = last_pps_offset;
            if(osc_offset == 0.0F)  osc_offset = last_osc_offset;
            if((rcvr_type == SCPI_RCVR) || (rcvr_type == UCCM_RCVR)) {
               if((dac_voltage < (-100.0F)) || (dac_voltage > 100.0F))  dac_voltage = last_dac_voltage;
            }
            else if((dac_voltage < (-10.0F)) || (dac_voltage > 10.0F))  dac_voltage = last_dac_voltage;
            if((temperature < (-55.0F)) || (temperature > 100.0F)) temperature = last_temp_val;
         }
      }

//    if((pps_offset < (-100.0)) || (pps_offset > 100.0))  pps_offset = last_pps_offset;
//    if((osc_offset < (-100.0)) || (osc_offset > 100.0))  osc_offset = last_osc_offset;
      have_rcvr_mode = 1;

      #ifdef PRECISE_STUFF
         update_precise_survey();
         if(rcvr_mode != last_rcvr_mode) plot_lla_axes(20);
         last_rcvr_mode = rcvr_mode;
      #endif
   }

   if(saw_ntpx && (initial_voltage == 0.0) && ((user_set_osc & 0x08) == 0x00)) {
      initial_voltage = dac_voltage;  // set initial dac voltage to the current dac value
      have_initv = 1;
   }

   if(have_time && (have_osc == 0)) {  // initialize the value to the current PPS offset
      have_osc = 1;
      osc_integral = (((double) pps_offset) * 1.0E-9);
   }
   osc_integral += (osc_offset * 1.0E-9);


   filter_spikes();    // filter out false temperature sensor spikes
   #ifdef TEMP_CONTROL
      control_temp();
   #else
      temp_dir = ' ';
   #endif

   #ifdef OSC_CONTROL
      control_osc();
   #endif

   survey_err = 0;
   if(survey_progress > 100) {
      survey_progress = 100;
      survey_err = 1;
   }

#ifdef ADEV_STUFF
    if((adev_period > 0.0F) && (pause_data == 0)) {
      if(++adev_time >= (int) (adev_period+0.5F)) {  // add this data point to adev data queue
         if(jitter_adev) add_adev_point(msg_ofs*1.0E6, (this_time_msec-last_time_msec)*1.0E6);   // rrrrr
         else            add_adev_point(osc_offset, pps_offset); 
//       add_adev_point(osc_integral*1.0E9/100.0, pps_offset);   
         adev_time = 0;
      }
   }
#endif // ADEV_STUFF

   if((tsip_type == STARLOC_RCVR) && (pt_hh == st_hh) && (pt_min == st_min) && (pt_sec == st_sec)) {
      // ignore duplicate time stamp
   }
   else {
      if(reading_log == 0) {
         time_check(0, 1.0F, year,month,day, hours,minutes,seconds,raw_frac);  // check for skips in the time
      }
      if(++log_file_time >= log_interval) {
         write_log_readings(log_file, -1L);
         log_file_time = 0;
      }
   }
   st_hh = pt_hh;
   st_min = pt_min;
   st_sec = pt_sec;

   if(tsip_error) return;
   if(survey_err) return;

   last_pps_offset = pps_offset;
   last_osc_offset = osc_offset;
   last_dac_voltage = dac_voltage;
   last_temperature = temperature;

   // round these to multiples of the graph scale factor
   last_temperature *= plot[TEMP].ref_scale;
   last_temperature = (long) ((last_temperature / plot[TEMP].scale_factor)) * plot[TEMP].scale_factor;
   last_temperature /= plot[TEMP].ref_scale;

   last_dac_voltage *= plot[DAC].ref_scale; 
   last_dac_voltage = (long) ((last_dac_voltage / plot[DAC].scale_factor)) * plot[DAC].scale_factor;
   last_dac_voltage /= plot[DAC].ref_scale; 

   if(have_alarms == 0) {
      have_alarms = 1;
      last_critical = critical_alarms;
      last_minor = minor_alarms;
   }

#ifdef ADEV_STUFF
   if(pause_data == 0) {  // redraw adev curves and tables every 10 seconds
      update_adev_display(ATYPE);  
   }
#endif // ADEV_STUFF

   show_serial_info();
   show_version_header();
   show_param_values();      // update alarm and data value displays

   write_log_changes();      // write important state changes to log file

   if((last_utc_ofs == 0) && have_utc_ofs) {  // we just got the UTC offset
      last_utc_ofs = have_utc_ofs;            
      if(log_loaded == 0) clear_all_data();   // assume previous data was bogus, clear it
   }

   update_plot_data();       // add latest info to the plot queue and update the screen
   if(dynamic_trend_info) {  // update trend line info plot title dynamically
      show_trend_info(selected_plot);
   }
}


void ae_packet()
{
u08 subcode;      // packet subcode
u16 zero;         // ???
u08 leds;         // led status
u08 hold;         // holdover state
u08 pll;          // pll state
u08 dis;          // disciplining mode
u08 flash;        // firmware code
u08 b1,b2;        // ???
u08 log_count;    // number of events in the log
u16 op_status;    // operation status
u16 hw_status;    // hardware status
float pps_val;    // related to pps error (in ns)
float osc_val;    // related to osc error (in ppb)
int i;
unsigned char buf[32];

   // Unknown packet sent by Nortel receiver.  Also mentioned as being
   // sent by the Palisade,  but in a different format.

// for(i=0; i<9+8; i++) tsip_byte();
   subcode = tsip_byte();
   if(subcode == 0x01) {
      zero  = tsip_word();
      leds  = tsip_byte();
      ffom  = tsip_byte();
      have_ffom = 1;
      tfom  = tsip_byte();
      have_tfom = 1;
      hold  = tsip_byte();

      if(try_ntpx == 0) {
         pll   = tsip_byte();
         dis   = tsip_byte();
         flash = tsip_byte();

         b1 = tsip_byte();    // 0
         b2 = tsip_byte();    // 1
         log_count = tsip_byte();  // error count
         op_status = tsip_word(); 
         hw_status = tsip_word(); 

         pps_val = tsip_single();  // PPS val (ns)
         osc_val = tsip_single();  // OSC val (ppb)
      }
//    sprintf(plot_title, "led:%02X  ffom:%02X tfom:%02X  ho:%02X  pll:%02X  dis:%02X  flash:%02X  b:%02X,%02X  log:%02X  op:%04X hw:%04X  %12g %12g", 
//      leds, ffom,tfom,  hold,pll,dis,flash, b1,b2, log_count, op_status,hw_status, pps_val,osc_val);
   }
   else if(1) {
//    plot_title[0] = 0;
      for(i=0; i<9+8+8; i++) {
         buf[i] = tsip_byte();
//       sprintf(out, "%02X ", (unsigned) buf[i]);
//       strcat(plot_title, out);
      }
   }

   tsip_end(1);
   if(try_ntpx && (tsip_error == 0) && (zero == 0x0001)) {
      saw_ntpx = 1;
      saw_gpsdo = 3;
      if(user_set_temp_filter == 0) undo_fw_temp_filter = 0;
   }
   if(tsip_error && (zero == 0x0001)) { // error seen, maybe it's an NTPX gpsdo?
      try_ntpx ^= 1;
      if(try_ntpx == 0) {
         saw_ntpx = 0;
         saw_gpsdo = 0;
      }
   }
//sprintf(plot_title, "ntpx: %d  zero:%04X  saw_ntpx=%d  notel=%d  oscp=%04X", 
//try_ntpx, zero, saw_ntpx, saw_nortel, have_osc_params);
}

//
//
//  Luxor LED / Power analyzer routines
//
//

// unit_status bit definitions
#define RESET_BIT      0x80000000L
#define CAL_MODE_BIT   0x40000000L
#define ZEROING_BIT    0x20000000L
#define WDT_BIT        0x10000000L
#define RUN_TIMEOUT    0x08000000L
#define CONFIG_ERR     0x04000000L
#define CAL_CRC_ERR    0x02000000L
#define COM_ERR_BIT    0x01000000L

#define AUXV_HVC_BIT   0x00800000L
#define AUXV_LVC_BIT   0x00400000L
#define LOAD_WATTS_BIT 0x00200000L
#define BATT_WATTS_BIT 0x00100000L

#define LUX2_OVFL_BIT  0x00080000L
#define LUX2_CHIP_BIT  0x00040000L
#define LUX1_OVFL_BIT  0x00020000L
#define LUX1_CHIP_BIT  0x00010000L

#define TEMP2_BAD_BIT  0x00008000L
#define TEMP2_OVT_BIT  0x00004000L
#define TEMP2_MASK     0xFFFFCFFFL
#define TEMP2_IR       0x00001000L
#define TEMP2_TC       0x00002000L
#define TEMP2_AT       0x00003000L

#define TEMP1_BAD_BIT  0x00000800L
#define TEMP1_OVT_BIT  0x00000400L
#define TEMP1_MASK     0xFFFFFCFFL
#define TEMP1_IR       0x00000100L
#define TEMP1_TC       0x00000200L
#define TEMP1_AT       0x00000300L

#define LOAD_HVC_BIT   0x00000080L
#define LOAD_LVC_BIT   0x00000040L
#define LOAD_OVC_BIT   0x00000020L
#define CFG_CRC_ERR    0x00000010L

#define BATT_HVC_BIT   0x00000008L
#define BATT_LVC_BIT   0x00000004L
#define BATT_OVC_BIT   0x00000002L
#define BATTERY_ON     0x00000001L

#define FAULT_BITS    (BATT_OVC_BIT | BATT_LVC_BIT | BATT_HVC_BIT | BATT_WATTS_BIT | LOAD_OVC_BIT | LOAD_HVC_BIT | LOAD_LVC_BIT | LOAD_WATTS_BIT | AUXV_HVC_BIT | AUXV_LVC_BIT | TEMP1_OVT_BIT | TEMP1_BAD_BIT | TEMP2_OVT_BIT | TEMP2_BAD_BIT | WDT_BIT | COM_ERR_BIT | RUN_TIMEOUT | CONFIG_ERR | CAL_CRC_ERR | CFG_CRC_ERR)


u08 luxor_fault()
{
int row, col;
int color;

   if(1 && (unit_status & RESET_BIT)) {
      have_time = 0;
      sound_alarm = 0;
//    fault_seen = 0;
   }

   if((unit_status & FAULT_BITS) == 0) {
      fault_seen = 0;
      if(unit_status & RESET_BIT) ;
      else if(unit_status & CAL_MODE_BIT) ;
      else if(unit_status & ZEROING_BIT) ;
      else if(prot_menu || show_prots) ;
      else {
         return 0;
      }
   }
   else if((unit_status & FAULT_BITS) && (fault_seen == 0)) {
      if(unit_status & RESET_BIT) ;
      else if(unit_status & ZEROING_BIT) ;
      else {
         fault_seen = 1;
         if((unit_status & FAULT_BITS) == COM_ERR_BIT) ;
         else if((unit_status & FAULT_BITS) == WDT_BIT) ;
         else if((unit_status & FAULT_BITS) == (WDT_BIT | COM_ERR_BIT)) ;
         else if(have_time) sound_alarm = 1;
      }
   }

   erase_watch();
   row = (aclock_y - ACLOCK_SIZE/2 + (TEXT_HEIGHT-1)) / TEXT_HEIGHT;
   col = (aclock_x - ACLOCK_SIZE/2 + (TEXT_WIDTH-1)) / TEXT_WIDTH;
   if(aclock_y < PLOT_ROW) {
      col -= 5;
      if(row) --row;
   }

   if(prot_menu || show_prots) {
      if(unit_status & FAULT_BITS) color = RED;
      else                         color = WHITE;
      vidstr(row++, col, color, "Protections:");

      if(unit_status & BATT_LVC_BIT) color = RED;
      else                           color = WHITE;
      sprintf(out, "Batt LVC: %8.3f V", batt_lvc);
      vidstr(row++, col, color, out);

      if(unit_status & BATT_HVC_BIT) color = RED;
      else                           color = WHITE;
      sprintf(out, "Batt HVC: %8.3f V", batt_hvc);
      vidstr(row++, col, color, out);

      if(unit_status & BATT_OVC_BIT) color = RED;
      else                           color = WHITE;
      sprintf(out, "Batt OVC: %8.3f A", batt_ovc);
      vidstr(row++, col, color, out);

      if(unit_status & BATT_WATTS_BIT) color = RED;
      else                             color = WHITE;
      sprintf(out, "Batt OVW: %8.3f W", batt_watts);
      vidstr(row++, col, color, out);

      if(unit_status & LOAD_LVC_BIT) color = RED;
      else                           color = WHITE;
      sprintf(out, "LED LVC:  %8.3f V", load_lvc);
      vidstr(row++, col, color, out);

      if(unit_status & LOAD_HVC_BIT) color = RED;
      else                           color = WHITE;
      sprintf(out, "LED HVC:  %8.3f V", load_hvc);
      vidstr(row++, col, color, out);

      if(unit_status & LOAD_OVC_BIT) color = RED;
      else                           color = WHITE;
      sprintf(out, "LED OVC:  %8.3f A", load_ovc);
      vidstr(row++, col, color, out);

      if(unit_status & LOAD_WATTS_BIT) color = RED;
      else                             color = WHITE;
      sprintf(out, "LED OVW:  %8.3f W", load_watts);
      vidstr(row++, col, color, out);

      if(unit_status & AUXV_LVC_BIT) color = RED;
      else                           color = WHITE;
      sprintf(out, "AUXV LVC: %8.3f V", auxv_lvc);
      vidstr(row++, col, color, out);

      if(unit_status & AUXV_HVC_BIT) color = RED;
      else                           color = WHITE;
      sprintf(out, "AUXV HVC: %8.3f V", auxv_hvc);
      vidstr(row++, col, color, out);


      if(unit_status & TEMP1_OVT_BIT) color = RED;
      else                            color = WHITE;
      sprintf(out, "TEMP1:    %8.3f%c%c", tc1_ovt, DEGREES, DEG_SCALE);
      vidstr(row++, col, color, out);

      if(unit_status & TEMP2_OVT_BIT) color = RED;
      else                            color = WHITE;
      sprintf(out, "TEMP2:    %8.3f%c%c", tc2_ovt, DEGREES, DEG_SCALE);
      vidstr(row++, col, color, out);

      if(unit_status & WDT_BIT) color = RED;
      else                      color = WHITE;
      sprintf(out, "Timeout:  %8.3f S", msg_timeout);
      vidstr(row++, col, color, out);
   }
   else {
      vidstr(row++, col, RED, "Abnormal status:");
      if(unit_status & RESET_BIT)      vidstr(row++, col, RED, "Unit reset"); 
      if(unit_status & CAL_MODE_BIT)   vidstr(row++, col, RED, "Cal mode"); 
      if(unit_status & CAL_CRC_ERR)    vidstr(row++, col, RED, "Cal memory bad"); 
      if(unit_status & CFG_CRC_ERR)    vidstr(row++, col, RED, "Config memory bad"); 
      if(unit_status & CONFIG_ERR)     vidstr(row++, col, RED, "Config changed"); 
      if(unit_status & ZEROING_BIT) {
         vidstr(row++, col, RED, "Zeroing sensors"); 
      }
      else {
         if(unit_status & BATT_OVC_BIT)   vidstr(row++, col, RED, "Battery over current");
         if(unit_status & BATT_WATTS_BIT) vidstr(row++, col, RED, "Battery over watts");
         if(unit_status & LOAD_OVC_BIT)   vidstr(row++, col, RED, "LED over current");
         if(unit_status & LOAD_WATTS_BIT) vidstr(row++, col, RED, "LED over watts");
      }
      if(unit_status & BATT_LVC_BIT)   vidstr(row++, col, RED, "Battery low voltage");
      if(unit_status & BATT_HVC_BIT)   vidstr(row++, col, RED, "Battery high voltage");
      if(unit_status & LOAD_LVC_BIT)   vidstr(row++, col, RED, "LED low voltage");
      if(unit_status & LOAD_HVC_BIT)   vidstr(row++, col, RED, "LED high voltage");
      if(unit_status & AUXV_LVC_BIT)   vidstr(row++, col, RED, "AUXV low voltage");
      if(unit_status & AUXV_HVC_BIT)   vidstr(row++, col, RED, "AUXV high voltage");
      if(unit_status & TEMP1_OVT_BIT)  vidstr(row++, col, RED, "Temp1 over-temp");
      if(unit_status & TEMP2_OVT_BIT)  vidstr(row++, col, RED, "Temp2 over-temp");
      if(unit_status & TEMP1_BAD_BIT)  vidstr(row++, col, RED, "Temp1 sensor fault");
      if(unit_status & TEMP2_BAD_BIT)  vidstr(row++, col, RED, "Temp2 sensor fault");
      if(unit_status & LUX1_OVFL_BIT)  vidstr(row++, col, RED, "Lux overflow");
      if(unit_status & LUX2_OVFL_BIT)  vidstr(row++, col, RED, "Lumens overflow");
      if(unit_status & COM_ERR_BIT)    vidstr(row++, col, RED, "Serial data error");
      if(unit_status & WDT_BIT)        vidstr(row++, col, RED, "Message timeout");
      if(unit_status & RUN_TIMEOUT)    vidstr(row++, col, RED, "Timer shutdown");
   }

   return 1;
}

struct CCT_TABLE {
   float ratio;
   float k;
} cctt[] = {
   { 0.000F, 200000.0F},
   { 0.440F, 63675.0F},     // 390 nm UV !!!!!
   { 0.823F, 20000.0F}, // dummy
   { 0.858F, 15000.0F}, // Chinese 3W
   { 0.894F, 13000.0F}, // Chinese 3W
   { 1.013F, 10000.0F}, // Chinese 3W
   { 1.156F, 8000.0F},  // Chinese 3W
   { 1.360F, 6500.0F},  // XML T6 UF2100
   { 1.550F, 5700.0F},  // luminus
   { 1.770F, 5200.0F},  // Bridgelux C8000
   { 1.890F, 4950.0F},  // SkyRayKing NW on high
   { 2.295F, 4250.0F},  // Nichia 219
   { 2.482F, 4000.0F},  // Philips capsule
   { 3.023F, 3350.0F},  // Bridgelux
   { 3.367F, 3050.0F},  // LSG MR16l
   { 3.458F, 3000.0F},  // philips/lsg/syl PAR20/Syl PAR16
   { 3.752F, 2850.0F},  // Bridgelux W0804
   { 4.077F, 2700.0F},  // LSG PAR30  SYL PAR16
   { 4.500F, 2500.0F}   // dummy value !!!!!
};

#define NUM_CCT (sizeof(cctt) / sizeof(struct CCT_TABLE))

float cie_x, cie_y, cie_z;
float cie_sum;

void calc_cie(float red_uw, float green_uw,  float blue_uw)
{
   if(tcs_color) {  // TCS3414
      cie_x = (-0.14282F*red_uw) + (1.54924F*green_uw) + (-0.95641F*blue_uw); // TCS3414
      cie_y = (-0.32466F*red_uw) + (1.57837F*green_uw) + (-0.73191F*blue_uw);
      cie_z = (-0.68202F*red_uw) + (0.77073F*green_uw) + (+0.56332F*blue_uw);
   }
   else {           // TCS3210
      cie_x = (0.4287F*red_uw) + (0.4489F*green_uw)  + (0.0493F*blue_uw);
      cie_y = (0.1450F*red_uw) + (0.9623F*green_uw)  + (-0.1167F*blue_uw);
      cie_z = (0.0539F*red_uw) + (-0.4059F*green_uw) + (1.4191F*blue_uw);
   }
   cie_sum = cie_x + cie_y + cie_z;
}

float calc_cct(int type, int undo_scale, float red, float green, float blue)
{
int i;
float val;
float dif;
float pct;
float red_uw, green_uw, blue_uw;
float ccx, ccy, ccz;
float n;
float cct;

   if(undo_scale) {        // red, green and blue came from get_plot_q() values
      if(show_color_uw) {  // we need to undo the scale factor that was applied to those values 
         red *= RED_SENS;     // blue
         green *= GREEN_SENS; // green
         blue *= BLUE_SENS;   // red
      }
      else if(show_color_pct) {
         // !!!!!!! we cant properly undo those values
         red_uw = red * (red + green + blue);
         green_uw = green * (red + green + blue);
         blue_uw = blue * (red + green + blue);
         red = red_uw;
         green = green_uw;
         blue = blue_uw;
      }
   }
if(0 && cct_dbg) sprintf(plot_title, "ty=%d r=%f g=%f b=%f", type, red, green, blue);

   if(type == 0) {
      if(blue == 0.0F) return 0.0F;
      val = (red / blue) * cct_cal;
val = rb_m * val + rb_b;

      for(i=0; i<(int) NUM_CCT; i++) {
         if(val < cctt[i].ratio) {
            if(i == 0) return 99999.999F;

            dif = cctt[i].ratio - cctt[i-1].ratio;
            if(dif == 0.0F) return 0.0F;
            pct = (val - cctt[i-1].ratio) / dif;

            dif = cctt[i].k - cctt[i-1].k;
// sprintf(plot_title, "red=%f  rob=%f  val=%f  hz=%f  i=%d  pct=%f  dif=%f", red, r_over_b, val, red_hz/blue_hz, i, pct, dif);
            dif *= pct;
            return cctt[i-1].k + dif;
         }
      }
   }
   else {
      red_uw = red / RED_SENS;
      green_uw = green / GREEN_SENS;
      blue_uw = blue / BLUE_SENS;

      calc_cie(red_uw, green_uw, blue_uw);
      if(cie_sum) {
         ccx = cie_x / cie_sum;
         ccy = cie_y / cie_sum;
         ccz = cie_z / cie_sum;
         if(type == 1) {
            n = (ccx - 0.3320F) / (ccy - 0.1858F);
            cct = (-449.0F*n*n*n) + (3525.0F*n*n) - (6823.3F*n) + 5520.33F;
            if((cct < 0.0F) || (cct > 100000.0F)) cct = 0.0F;
            cct /= cct1_cal;  // apply bogus correction factor
            return cct;
         }
         else if(type == 2) {
            n = (ccx - 0.3366F) / (ccy - 0.1735F);
            cct = (float) ((-949.86315F) + 6253.80338F*exp(-n/0.91259F) + 28.70599F*exp(-n/0.20039) + 0.00004F*exp(-n/0.07125F));
            if((cct < 0.0F) || (cct > 100000.0F)) cct = 0.0F;
            cct /= cct2_cal;  // apply bogus correction factor
            return cct;
         }
      }
      else return 0.0F;
   }

   return 1.0F;
}


#define HH 6.626068E-34
#define CC 299792458.0
#define KK 1.3806503E-23

double plank(double t, double y)
{
double f;

// This routine used Plank's equation to calculate the energy at wavelength
// y (in namometers) of a black body at temperature t (in degrees K)

   y *= 1.0E-9;     // convert wavelenth to meters
   f = (2.0*PI*HH*CC*CC) /
       ((y*y*y*y*y) * (exp((HH*CC) / (KK*y*t)) - 1.0));
   f /= 1.0E6;      // normalize to standard units
//printf("T=%f  y=%g  f=%g\n", t,y,f);
   return f;

}


float calc_cri(float cct)
{
double bb_r,bb_g,bb_b,d,sum;  // black body values
double R,G,B;                 // sensor values
double y;
double err;

   d = 0.0;

   R = red_uw;
   G = green_uw;
   B = blue_uw;
   sum = R + G + B;
   if(sum == 0.0) return 0.0F;

   R /= (float) sum;
   G /= (float) sum;
   B /= (float) sum;

   // calculate the black body energy emitted over the color sensor bands
   if(cri_flag) {
      bb_r = plank(cct, 621.0);  // 640
      bb_g = plank(cct, 500.0);  // 524
      bb_b = plank(cct, 490.0);  // 470
   }
   else if(1) {
      bb_b  = plank(cct, 420.0) * 0.30;
      bb_b += plank(cct, 430.0) * 0.40;
      bb_b += plank(cct, 440.0) * 0.42;
      bb_b += plank(cct, 450.0) * 0.45;
      bb_b += plank(cct, 460.0) * 0.48;
      bb_b += plank(cct, 470.0) * 0.46;
      bb_b += plank(cct, 480.0) * 0.44;
      bb_b += plank(cct, 490.0) * 0.40;
      bb_b += plank(cct, 500.0) * 0.35;
      bb_b += plank(cct, 510.0) * 0.30;
      bb_b += plank(cct, 520.0) * 0.25;
      bb_b += plank(cct, 530.0) * 0.15;
      bb_b += plank(cct, 540.0) * 0.14;
      bb_b += plank(cct, 550.0) * 0.13;
      bb_b += plank(cct, 560.0) * 0.08;
      bb_b /= 15.0;

      bb_g  = plank(cct, 420.0) * 0.05;
      bb_g += plank(cct, 420.0) * 0.06;
      bb_g += plank(cct, 440.0) * 0.07;
      bb_g += plank(cct, 450.0) * 0.08;
      bb_g += plank(cct, 460.0) * 0.10;
      bb_g += plank(cct, 470.0) * 0.18;
      bb_g += plank(cct, 480.0) * 0.20;
      bb_g += plank(cct, 490.0) * 0.23;
      bb_g += plank(cct, 500.0) * 0.28;
      bb_g += plank(cct, 510.0) * 0.32;
      bb_g += plank(cct, 520.0) * 0.38;
      bb_g += plank(cct, 530.0) * 0.50;
      bb_g += plank(cct, 540.0) * 0.55;
      bb_g += plank(cct, 550.0) * 0.54;
      bb_g += plank(cct, 560.0) * 0.50;
      bb_g += plank(cct, 570.0) * 0.40;
      bb_g += plank(cct, 580.0) * 0.39;
      bb_g += plank(cct, 590.0) * 0.20;
      bb_g += plank(cct, 600.0) * 0.15;
      bb_g += plank(cct, 610.0) * 0.06;
      bb_g /= 20.0;

      bb_r  = plank(cct, 510.0) * 0.03;
      bb_r += plank(cct, 520.0) * 0.05;
      bb_r += plank(cct, 530.0) * 0.08;
      bb_r += plank(cct, 540.0) * 0.10;
      bb_r += plank(cct, 550.0) * 0.17;
      bb_r += plank(cct, 560.0) * 0.17;
      bb_r += plank(cct, 570.0) * 0.20;
      bb_r += plank(cct, 580.0) * 0.40;
      bb_r += plank(cct, 590.0) * 0.50;
      bb_r += plank(cct, 600.0) * 0.60;
      bb_r += plank(cct, 610.0) * 0.65;
      bb_r += plank(cct, 620.0) * 0.70;
      bb_r += plank(cct, 630.0) * 0.73;
      bb_r += plank(cct, 640.0) * 0.80;
      bb_r += plank(cct, 650.0) * 0.82;
      bb_r += plank(cct, 660.0) * 0.85;
      bb_r += plank(cct, 670.0) * 0.85;
      bb_r += plank(cct, 680.0) * 0.90;
      bb_r += plank(cct, 690.0) * 0.93;
      bb_r += plank(cct, 700.0) * 0.95;
      bb_r /= 20.0;    
   }
   else {
      bb_r = bb_g = bb_b = 0.0;
      for(y=400.0; y<=520.0; y+=10.0) {
         bb_b += plank(cct, y);
         d += 1.0;
      }
      bb_b /= d;

      for(y=480.0; y<=600.0; y+=10.0) {
         bb_g += plank(cct, y);
         d += 1.0;
      }
      bb_g /= d;

      for(y=550.0; y<=770.0; y+=10.0) {
         bb_r += plank(cct, y);
         d += 1.0;
      }
      bb_r /= d;
   }
   sum = bb_r + bb_g + bb_b;  // normalize black body energy to 0 .. 1
   bb_r /= sum;
   bb_g /= sum;
   bb_b /= sum;


   err = sqrt((((R-bb_r)*(R-bb_r)) + ((G-bb_g)*(G-bb_g)) + ((B-bb_b)*(B-bb_b)))/3.0);
   err = (1.0-err) * (1.0-err);

   return (float)err*100.0F;
}


#define SAT_COLOR_HZ 700000.0F      // color sensor saturation threshold
#define MAX_COLOR_HZ 500000.0F      // color sensor saturation warning threshold
#define MIN_COLOR_HZ 100.0F         // color sensor too low threshold
#define RED_OVFL   0x01
#define GREEN_OVFL 0x02
#define BLUE_OVFL  0x04
#define WHITE_OVFL 0x08
#define RED_LOW    0x10
#define GREEN_LOW  0x20
#define BLUE_LOW   0x40
#define WHITE_LOW  0x80
#define RED_SAT    0x100
#define GREEN_SAT  0x200
#define BLUE_SAT   0x400
#define WHITE_SAT  0x800

u08 last_cal_mode;


void measure_ir()
{
   // this routine measures the battery internal resistance by checking
   // how the battery voltage changes when loaded and unloaded

   if(calc_ir >= IR_TIME) {        // turn on load
      set_batt_pwm(0xFFFF);
      --calc_ir;
      sprintf(plot_title, "Calculating battery internal resistance: %2d", calc_ir);
   }
   else if(calc_ir == 3) {   // get loaded readings
      ir_v = batt_v;
      ir_i = batt_i;
      --calc_ir;
      set_batt_pwm(0);
      sprintf(plot_title, "Calculating battery internal resistance: %2d", calc_ir);
   }
   else if(calc_ir) {
      --calc_ir;
      if(calc_ir) {     // get unloaded readings
         sprintf(plot_title, "Calculating battery internal resistance: %2d", calc_ir);
      }
      else {
         ir_v = fabs(batt_v-ir_v);
         if(ir_i) {
            ir_v /= ir_i;
            sprintf(plot_title, "Battery internal resistance: %.2f milliohms at %.3f amps", ir_v*1000.0F, ir_i);
         }
         else {
            sprintf(plot_title, "No load detected on the battery");
         }
      }
   }
}



#define TERM_TIME 60  // seconds
int term_timer;       // times amount of time that the charge current is less than the shutoff threshold


void end_sweep()
{
   cc_mode = 0;
   sweep_stop = 0.0F;
   sweep_val = 0.0F;
   cc_state = 0;
   if(update_stop) {
      pause_data = 1;
      view_all();
      need_redraw = 2011;
   }
   update_stop = 0;
}

void constant_load()
{
float delta;
u16 pwm;

   if(cc_mode == 0) return;
   if(cc_state < 5) ++cc_state;
   if(sweep_rate <= 0) {
      sweep_rate = sweep_tick = 1;
   }

   if(cc_state == 1) {  // starting up
      if(cc_mode == PWM_SWEEP) {
         cc_pwm = sweep_val = sweep_start;
         goto charged;
      }
      else if((cc_mode == CC_LIPO) && (led_v < unsafe_v)) {  // potentially unsafe cell - don't charge
         goto stop_charge;
      }
      term_timer = TERM_TIME;
      if(sweep_stop) cc_pwm = 0.0F;
      else           cc_pwm = PWM_STEP;
   }
   else if(cc_mode == PWM_SWEEP) {   // sweeping PWM value between limits
      if(sweep_start > sweep_end) {  // down sweep
         if(--sweep_tick <= 0) {
            sweep_tick = sweep_rate;
            sweep_val -= PWM_STEP;
            if(sweep_val < 0.0F) end_sweep();
         }
         if(sweep_val < sweep_end) {
            end_sweep();
         }
      }
      else {                         // up sweep;
         if(--sweep_tick <= 0) {
            sweep_tick = sweep_rate;
            sweep_val += PWM_STEP;
            if(sweep_val > 1.0F) end_sweep();
         }
         if(sweep_val > sweep_end) {
            end_sweep();
         }
      }
      cc_pwm = sweep_val;
      goto charged;
   }
   else {
      if((cc_mode == CC_LIPO) && (led_v > (lipo_volts-0.05F))) {  // constant V mode
//       if(led_v > (lipo_volts+0.05F)) goto stop_charge;
         if(led_v > (lipo_volts+0.10F)) goto stop_charge;
         if(led_i < (cc_val/20.0F)) {  // constant I mode - charge done at charge_rate/20
            if(--term_timer < 0) {
               stop_charge:
               end_sweep();
               cc_pwm = 0.0F;
               goto charged;
            }
         }
         else term_timer = TERM_TIME;

         if(led_i < cc_val) {
            if((led_v > lipo_volts) && cc_pwm) {
               cc_pwm -= PWM_STEP;
               if(cc_pwm < PWM_STEP) cc_pwm = PWM_STEP;
            }
            goto charged;
         }
      }

      if((cc_mode == CC_VOLTS) && (led_v > 0.050F)) {  // must have at least 50mv load voltage
         if(sweep_stop) {
            if(led_v > sweep_stop) {
               end_sweep();
               cc_pwm = 0.0F;
            }
            else if(--sweep_tick <= 0) {
               sweep_tick = sweep_rate;
               cc_pwm += PWM_STEP;
               if(cc_pwm > 1.0F) {
                  end_sweep();
                  cc_pwm = 0.0F;
               }
            }
         }
         else {
            delta = (cc_val - led_v);
            delta /= led_v;
            if(delta < 0.0F) delta *= 0.50F;
            else             delta *= 0.20F;
            cc_pwm += (delta * cc_pwm);
         }
      }
      else if((cc_mode == CC_WATTS) && (led_w > 0.050F)) {  // must have at least 50mw load power
         if(sweep_stop) {
            if(led_w > sweep_stop) {
               end_sweep();
               cc_pwm = 0.0F;
            }
            else if(--sweep_tick <= 0) {
               sweep_tick = sweep_rate;
               cc_pwm += PWM_STEP;
               if(cc_pwm > 1.0F) {
                  end_sweep();
                  cc_pwm = 0.0F;
               }
            }
         }
         else {
            delta = (cc_val - led_w);
            delta /= led_w;
            if(delta < 0.0F) delta *= 0.25F;  // power is a squared function, reduce gain
            else             delta *= 0.04F;
            cc_pwm += (delta * cc_pwm);
         }
      }
      else if((cc_mode > 1) && (led_i > 0.050F)) {  // must have at least 50mA load current
         if(sweep_stop) {
            if(led_i > sweep_stop) {
               end_sweep();
               cc_pwm = 0.0F;
            }
            else if(--sweep_tick <= 0) {
               sweep_tick = sweep_rate;
               cc_pwm += PWM_STEP;
               if(cc_pwm > 1.0F) {
                  end_sweep();
                  cc_pwm = 0.0F;
               }
            }
         }
         else {
            delta = (cc_val - led_i);
            delta /= led_i;
            if(delta < 0.0F) delta *= 0.50F;
            else             delta *= 0.20F;
            cc_pwm += (delta * cc_pwm);
         }
      }
      else {
         if(sweep_stop) {
            if(--sweep_tick <= 0) {
               sweep_tick = sweep_rate;
               cc_pwm += PWM_STEP;
               if(cc_pwm > 1.0F) {
                  end_sweep();
                  cc_pwm = 0.0F;
               }
            }
         }
         else {
            cc_pwm += PWM_STEP;
         }
      }
   }

   charged:
   if(cc_pwm > 1.0)        cc_pwm = 1.0F;
   else if(cc_pwm <= 0.0F) cc_pwm = 0.0F;

   pwm = (u16) (cc_pwm * 65535.0F);
   set_batt_pwm(pwm);

   if(cc_mode == CC_LIPO) {
      sprintf(plot_title, " LiPo Charge %.3fA to %.3fV: led_i=%.3fA  pwm=%f %04X %04X", cc_val,lipo_volts, led_i, cc_pwm,pwm,pwm>>6);
   }
   else if(cc_mode == CC_AMPS) {
      sprintf(plot_title, " CC Load %.3fA: led_i=%.3fA  pwm=%f %04X %04X", cc_val, led_i, cc_pwm,pwm,pwm>>6);
   }
   else if(cc_mode == CC_VOLTS) {
      sprintf(plot_title, " CV Load %.3fV: led_v=%.3fV  pwm=%f %04X %04X", cc_val, led_v, cc_pwm,pwm,pwm>>6);
   }
   else if(cc_mode == CC_WATTS) {
      sprintf(plot_title, " CW Load %.3fW: led_v=%.3fV  led_i=%.3fA  led_w=%.3fW  pwm=%f %04X %04X", cc_val, led_v, led_i, led_w, cc_pwm,pwm,pwm>>6);
   }
   else if(cc_mode == PWM_SWEEP) {
      sprintf(plot_title, " PWM sweep from %.3f to %.3f:  pwm=%f %04X %04X", sweep_start, sweep_end, cc_pwm,pwm,pwm>>6);
   }
}


u32 last_unit_status;

float ldbg1, ldbg2, ldbg3, ldbg4;


void request_luxor_ver()
{
   if(luxor == 0) return;
   if(no_poll) return;

   send_tsip_start(LUXOR_ID); 
   send_byte(0x00);
   send_tsip_end();
}

void luxor_packet()
{
u08 subcode;    // packet subcode
double val;
int i;
unsigned char buf[32];
int batt_volts, led_volts;
int batt_amps, led_amps;
int row;
int col;
int color;
int color_ovfl;
float color_sum;
float ccx, ccy, ccz;
float cct1, cct2, cct3, cct4;

   // Luxor power/light analyzer
   subcode = tsip_byte();
   if(subcode == 0x00) {        // get version info
      luxor_hw_ver = tsip_byte();
      luxor_hw_rev = tsip_byte();
      luxor_sn_prefix = sn_prefix = tsip_byte();
      luxor_serial_num = serial_num = tsip_word();

      batt_volts = tsip_word();
      batt_amps = (int) (s16) tsip_word();
      led_volts = tsip_word();
      led_amps = (int) (s16) tsip_word();
      tsip_end(1);

      have_info = (MANUF_PARAMS | PRODN_PARAMS | VERSION_INFO);

      if(text_mode && first_key) return;
      if(zoom_screen) return;

      if(cal_mode) color = RED;
      else         color = WHITE;

      sprintf(
         out, "BAT:%3dV  %c%dA", 
         batt_volts, (batt_amps<0)?PLUS_MINUS:'+', (batt_amps<0)?0-batt_amps:batt_amps
      );
      vidstr(VER_ROW+1, VER_COL, color, out);
      sprintf(
         out, "LED:%3dV  %c%dA", 
         led_volts,  (led_amps<0)?PLUS_MINUS:'+',  (led_amps<0)?0-led_amps:led_amps
      );
      vidstr(VER_ROW+2, VER_COL, color, out);
      sprintf(out, "VER: %d.%02d", luxor_hw_ver,luxor_hw_rev);
      vidstr(VER_ROW+3, VER_COL, color, out);

      show_serial_info();
      show_version_header();
   }
   else if(subcode == 0x01) {   // get readings
      if(0 && (luxor == 0)) {
         luxor = 10;
         rcvr_type = LUXOR_RCVR;
         config_rcvr_type(1);
//       reset_luxor_wdt(0x01);
//       config_luxor_plots();
         config_screen(201);
         request_luxor_ver();
         set_luxor_time();
      }
      ++tow;
      time_color = DST_TIME_COLOR;

      unit_status = tsip_dword();
      batt_pwm = tsip_word();

      pri_frac = raw_frac = 0.0;
      pri_seconds = seconds = tsip_byte();
      pri_minutes = minutes = tsip_byte();
      pri_hours = hours = tsip_byte();
      pri_day = day = tsip_byte();
      pri_month = month = tsip_byte();
      pri_year = year = tsip_byte() + 2000;

      tow = fake_tow(jd_utc) % (7L*24L*60L*60L);
//    this_tow = tow;

      batt_v = dac_voltage = tsip_single();
      have_dac = 1;
      batt_i = tsip_single(); 

      led_pos = tsip_single(); 
      led_neg = tsip_single(); 
      led_v = led_pos - led_neg;
      led_i = tsip_single(); 
      adc2 = tsip_single();

      tc1 = temperature = tsip_single(); 
      tc2 = tsip_single(); 
      have_temperature = 2;

      pwm_hz = tsip_single();

      lux1 = tsip_single(); 
      lux1_time = tsip_byte();

      lux2 = tsip_single(); 
      lux2_time = tsip_byte();

      red_hz = tsip_single();
      blue_hz = tsip_single();
      white_hz = tsip_single();
      green_hz = tsip_single();
      r_over_b = tsip_single();

      cal_mode = tsip_byte();
      ldbg1 = tsip_single();
      ldbg2 = tsip_single();
      ldbg3 = tsip_single();
      ldbg4 = tsip_single();

      tsip_end(1);


      if(show_debug_info) {
         sprintf(plot_title, "dbg1=%f dbg2=%f dbg3=%f dbg4=%f", ldbg1,ldbg2,ldbg3,ldbg4);
      }

      reset_luxor_wdt(0);

      if(unit_status != last_unit_status) {  // make sure fault display gets erased/redrawn
         last_unit_status = unit_status;
         need_redraw = 2012;
      }

      if(unit_status & FAULT_BITS) {
         end_sweep();
      }
      if(calc_ir) {     // battery internal resistance
         measure_ir();
      }
      constant_load();

      if(unit_status & RESET_BIT) {
         set_luxor_time();
         request_luxor_ver();
      }
      else if(cal_mode != last_cal_mode) request_luxor_ver();
      last_cal_mode = cal_mode;


      color_ovfl = 0;
      if(red_hz > SAT_COLOR_HZ)        color_ovfl |= RED_SAT;
      else if(red_hz > MAX_COLOR_HZ)   color_ovfl |= RED_OVFL;
      else if(red_hz < MIN_COLOR_HZ)   color_ovfl |= RED_LOW;

      if(green_hz > SAT_COLOR_HZ)      color_ovfl |= GREEN_SAT;
      else if(green_hz > MAX_COLOR_HZ) color_ovfl |= GREEN_OVFL;
      else if(green_hz < MIN_COLOR_HZ) color_ovfl |= GREEN_LOW;

      if(blue_hz > SAT_COLOR_HZ)       color_ovfl |= BLUE_SAT;
      else if(blue_hz > MAX_COLOR_HZ)  color_ovfl |= BLUE_OVFL;
      else if(blue_hz < MIN_COLOR_HZ)  color_ovfl |= BLUE_LOW;

      if(white_hz > SAT_COLOR_HZ)      color_ovfl |= WHITE_SAT;
      else if(white_hz > MAX_COLOR_HZ) color_ovfl |= WHITE_OVFL;
      else if(white_hz < MIN_COLOR_HZ) color_ovfl |= WHITE_LOW;

      cct  = calc_cct(0, 0, red_hz, green_hz, blue_hz);
      cct1 = calc_cct(1, 0, red_hz, green_hz, blue_hz);
      cct2 = calc_cct(2, 0, red_hz, green_hz, blue_hz);

      cct3 = (cct + cct1 + cct2) / 3.0F;
      cct4 = sqrt((cct*cct + cct1*cct1 + cct2*cct2) / 3.0F);
cct3 = cct4;

      red_uw = red_hz / RED_SENS;
      green_uw = green_hz / GREEN_SENS;
      blue_uw = blue_hz / BLUE_SENS;
      white_uw = white_hz / WHITE_SENS;
if(alt_lux1) lux1 = green_uw * alt_lux1;

      calc_cie(red_uw, green_uw, blue_uw);

      if(cie_sum) {
         ccx = cie_x / cie_sum;
         ccy = cie_y / cie_sum;
         ccz = cie_z / cie_sum;
      }
      else {
         ccx = ccy = ccz = 0.0F;
         cct = cct1 = cct2 = 0.0F;
      }


      pps_offset = (double) lux1; 
      have_pps_offset = 2;
      osc_offset = (double) batt_i; 
      have_osc_offset = 2;

      update_gps_screen(5555);

      batt_w = (batt_v*batt_i);
      led_w = (led_v*led_i);

      if(text_mode && first_key) return;
      if(zoom_screen) return;

      if(1 && (SCREEN_HEIGHT > 600)) row = 1;
      else                           row = 0;
      col = 30;

      if(cal_mode) unit_status |= CAL_MODE_BIT;
      sprintf(out, "Status: %08lX", (unsigned long) unit_status);
      if(unit_status & FAULT_BITS)      color = RED;
      else if(unit_status & BATTERY_ON) color = WHITE;
      else                              color = YELLOW;
      vidstr(row++, col, color, out);
      ++row;


      color_sum = (red_uw + green_uw + blue_uw) / 100.0F;
      if(color_sum == 0.0F) {
         sprintf(out, "Red:    none       ");
         vidstr(row++, col, GREY, out);
         sprintf(out, "Green:  none       ");
         vidstr(row++, col, GREY, out);
         sprintf(out, "Blue:   none       ");
         vidstr(row++, col, GREY, out);
         sprintf(out, "White:  none       ");
         vidstr(row++, col, GREY, out);
      }
      else if(show_color_hz) {
         sprintf(out, "Red:    %8.0f Hz", red_hz);
         if(color_ovfl & RED_OVFL)     color = YELLOW;
         else if(color_ovfl & RED_SAT) color = RED;
         else if(color_ovfl & RED_LOW) color = BLUE;
         else                          color = WHITE;
         vidstr(row++, col, color, out);

         sprintf(out, "Green:  %8.0f Hz", green_hz);
         if(color_ovfl & GREEN_OVFL)     color = YELLOW;
         else if(color_ovfl & GREEN_SAT) color = RED;
         else if(color_ovfl & GREEN_LOW) color = BLUE;
         else                            color = WHITE;
         vidstr(row++, col, color, out);

         sprintf(out, "Blue:   %8.0f Hz", blue_hz);
         if(color_ovfl & BLUE_OVFL)     color = YELLOW;
         else if(color_ovfl & BLUE_SAT) color = RED;
         else if(color_ovfl & BLUE_LOW) color = BLUE;
         else                           color = WHITE;
         vidstr(row++, col, color, out);

         sprintf(out, "White:  %8.0f Hz", white_hz);
         if(color_ovfl & WHITE_OVFL)     color = YELLOW;
         else if(color_ovfl & WHITE_SAT) color = RED;
         else if(color_ovfl & WHITE_LOW) color = BLUE;
         else                            color = WHITE;
         vidstr(row++, col, color, out);
      }
      else if(show_color_pct) {
         sprintf(out, "Red:    %8.3f %%", red_uw/color_sum);
         if(color_ovfl & RED_OVFL)     color = YELLOW;
         else if(color_ovfl & RED_SAT) color = RED; 
         else if(color_ovfl & RED_LOW) color = BLUE;
         else                          color = WHITE;
         vidstr(row++, col, color, out);

         sprintf(out, "Green:  %8.3f %%", green_uw/color_sum);
         if(color_ovfl & GREEN_OVFL)     color = YELLOW;
         else if(color_ovfl & GREEN_SAT) color = RED;
         else if(color_ovfl & GREEN_LOW) color = BLUE;
         else                            color = WHITE;
         vidstr(row++, col, color, out);

         sprintf(out, "Blue:   %8.3f %%", blue_uw/color_sum);
         if(color_ovfl & BLUE_OVFL)     color = YELLOW;
         else if(color_ovfl & BLUE_SAT) color = RED;
         else if(color_ovfl & BLUE_LOW) color = BLUE;
         else                           color = WHITE;
         vidstr(row++, col, color, out);

         sprintf(out, "White:  %8.3f %%", white_uw/color_sum);
         if(color_ovfl & WHITE_OVFL)     color = YELLOW;
         else if(color_ovfl & WHITE_SAT) color = RED;
         else if(color_ovfl & WHITE_LOW) color = BLUE;
         else                            color = WHITE;
         vidstr(row++, col, color, out);
      }
      else if(show_color_uw) {
         sprintf(out, "Red:   %9.3f uW/cm^2", red_hz/RED_SENS);
         if(color_ovfl & RED_OVFL)     color = YELLOW;
         else if(color_ovfl & RED_SAT) color = RED;
         else if(color_ovfl & RED_LOW) color = BLUE;
         else                          color = WHITE;
         vidstr(row++, col, color, out);

         sprintf(out, "Green: %9.3f uW/cm^2", green_hz/GREEN_SENS);
         if(color_ovfl & GREEN_OVFL)     color = YELLOW;
         else if(color_ovfl & GREEN_SAT) color = RED;
         else if(color_ovfl & GREEN_LOW) color = BLUE;
         else                            color = WHITE;
         vidstr(row++, col, color, out);

         sprintf(out, "Blue:  %9.3f uW/cm^2", blue_hz/BLUE_SENS);
         if(color_ovfl & BLUE_OVFL)     color = YELLOW;
         else if(color_ovfl & BLUE_SAT) color = RED;
         else if(color_ovfl & BLUE_LOW) color = BLUE;
         else                           color = WHITE;
         vidstr(row++, col, color, out);

         sprintf(out, "White: %9.3f uW/cm^2", white_hz/WHITE_SENS);
         if(color_ovfl & WHITE_OVFL)     color = YELLOW;
         else if(color_ovfl & WHITE_SAT) color = RED;
         else if(color_ovfl & WHITE_LOW) color = BLUE;
         else                            color = WHITE;
         vidstr(row++, col, color, out);
      }

      if(color_sum == 0.0F)                        color = GREY;
      else if(color_ovfl & (RED_SAT | BLUE_SAT))   color = RED;
      else if(color_ovfl & (RED_OVFL | BLUE_OVFL)) color = YELLOW;
      else if(color_ovfl & (RED_LOW | BLUE_LOW))   color = BLUE;
      else                                         color = WHITE;
      if(1) sprintf(out, "R/B:   %9.3f   ", r_over_b);
//    if(blue_hz) sprintf(out, "R/B:   %9.3f   ", red_hz/blue_hz);
      else        sprintf(out, "R/B:               ");
      vidstr(row++, col, color, out);
      sprintf(out, "CCT:   %9.3f %cK  ", cct, DEGREES);
      vidstr(row++, col, color, out);

      ++row;
      if(color_sum == 0.0F)                                     color = GREY;
      else if(color_ovfl & (RED_SAT | GREEN_SAT | BLUE_SAT))    color = RED;
      else if(color_ovfl & (RED_OVFL | GREEN_OVFL | BLUE_OVFL)) color = YELLOW;
      else if(color_ovfl & (RED_LOW | GREEN_LOW | BLUE_LOW))    color = BLUE;
      else                                                      color = WHITE;
//    sprintf(out, "ciex: %.4f  ", cie_x);
//    vidstr(row++, col, color, out); 
//    sprintf(out, "ciey: %.4f  ", cie_y);
//    vidstr(row++, col, color, out); 
//    sprintf(out, "ciez: %.4f  ", cie_z);
//    vidstr(row++, col, color, out); 
      sprintf(out, "ciex: %.4f  ", ccx);
      vidstr(row++, col, color, out); 
      sprintf(out, "ciey: %.4f  ", ccy);
      vidstr(row++, col, color, out); 
      sprintf(out, "ciez: %.4f  ", ccz);
      vidstr(row++, col, color, out); 
      ++row;

      sprintf(out, "cct1: %.3f %cK  ", cct1, DEGREES);
      vidstr(row++, col, color, out); 
      sprintf(out, "cct2: %.3f %cK  ", cct2, DEGREES);
      vidstr(row++, col, color, out); 
      sprintf(out, "cct3: %.3f %cK  ", cct3, DEGREES);
      vidstr(row++, col, color, out); 
//    sprintf(out, "cct4: %.3f %cK  ", cct4, DEGREES);
//    vidstr(row++, col, color, out); 
      sprintf(out, "bogoCRI:%6.2f  ", calc_cri(cct));
      vidstr(row++, col, color, out); 


      col = 0;
      if(1 && (SCREEN_HEIGHT > 600)) row = 1;
      else                           row = 0;

      if(unit_status & (BATT_HVC_BIT | BATT_LVC_BIT)) color = RED;
//    else if((batt_v > 0.003F) && (batt_v < 0.300F)) color = BLUE;
      else if(unit_status & BATTERY_ON)               color = WHITE;
      else                                            color = YELLOW;
      sprintf(out, "Batt V: %8.3f V", batt_v);
      vidstr(row++, col, color, out);

      if(unit_status & BATT_OVC_BIT)    color = RED;
      else if(unit_status & BATTERY_ON) color = WHITE;
      else                              color = YELLOW;
      sprintf(out, "Batt I: %8.3f A", batt_i);
      vidstr(row++, col, color, out);

      if(unit_status & BATTERY_ON) {
         if(batt_pwm != 0xFFFF) color = GREY;
         else                   color = WHITE;
      }
      else color = YELLOW;
      sprintf(out, "Batt W: %8.3f W", batt_w);
      vidstr(row++, col, color, out);


      if(plot[AUXV].show_plot) {
         if(unit_status & AUXV_HVC_BIT)      color = RED;
         else if(unit_status & AUXV_LVC_BIT) color = RED;
         else                                color = WHITE;
         sprintf(out, "Aux V:  %8.3f V", adc2);
         vidstr(row++, col, color, out);
      }
      else if(SCREEN_HEIGHT >= 600) ++row;

      if(unit_status & (LOAD_HVC_BIT | LOAD_LVC_BIT))   color = RED;
//    else if((led_pos > 0.003F) && (led_pos < 0.300F)) color = BLUE;
//    else if((led_neg > 0.003F) && (led_neg < 0.300F)) color = BLUE;
      else if(unit_status & BATTERY_ON)                 color = WHITE;
      else                                              color = YELLOW;
      sprintf(out, "Led +:  %8.3f V", led_pos);
      vidstr(row++, col, color, out);
      sprintf(out, "Led -:  %8.3f V", led_neg);
      vidstr(row++, col, color, out);
      sprintf(out, "Led Vf: %8.3f V", fabs(led_v));
      vidstr(row++, col, color, out);

      if(unit_status & LOAD_OVC_BIT)    color = RED;
      else if(unit_status & BATTERY_ON) color = WHITE;
      else                              color = YELLOW;
      sprintf(out, "Led I:  %8.3f A", led_i);
      vidstr(row++, col, color, out);

      sprintf(out, "Led W:  %8.3f W", led_w);
      if(unit_status & BATTERY_ON) color = WHITE;
      else                         color = YELLOW;
      vidstr(row++, col, color, out);

      sprintf(out, "Eff:             ");
      if(batt_w) {
         val = fabs(led_w/batt_w*100.0);  // !!!!! fabs?
         if(batt_pwm != 0xFFFF) color = GREY;
         if(val < 120.0) sprintf(out, "Eff:    %8.3f %%", val);
         else            sprintf(out, "Eff:         ??? %%");
      }
      vidstr(row++, col, color, out);


      if(SCREEN_HEIGHT >= 600) ++row;
      if((unit_status & (TEMP1_IR | TEMP1_TC | TEMP1_AT)) == 0) {   // no chip seen
         color = GREY;
         sprintf(out, "Temp1:      NONE    ");
      }
      else {                            // thermocouple chip installed
         if(unit_status & TEMP1_OVT_BIT)      color = RED;     // over temp
         else if(unit_status & TEMP1_BAD_BIT) color = YELLOW;  // no thermocouple seen
         else                                 color = WHITE;
         sprintf(out, "Temp1:  %s", fmt_temp(tc1));
      }
      vidstr(row++, col, color, out);


      if((unit_status & (TEMP2_IR | TEMP2_TC | TEMP2_AT)) == 0) {   // no chip seen
         color = GREY;
         sprintf(out, "Temp2:      NONE    ");
      }
      else {
         if(unit_status & TEMP2_OVT_BIT)      color = RED;     // over temp
         else if(unit_status & TEMP2_BAD_BIT) color = YELLOW;  // no thermocouple seen
         else                                 color = WHITE;
         sprintf(out, "Temp2:  %s", fmt_temp(tc2));
      }
      vidstr(row++, col, color, out);




      if(SCREEN_HEIGHT >= 600) ++row;
      if(unit_status & LUX1_CHIP_BIT) {
//val = ((white_hz/243.56F) + ((red_hz+green_hz+blue_hz)/229.8F)) * (1.10F/2.0F);
         if(show_fc) sprintf(out, "Lux:    %8.3f fc  ", lux1*lux_scale);
         else        sprintf(out, "Lux:    %8.3f lux ", lux1*lux_scale);
         if(unit_status & LUX1_OVFL_BIT) color = RED;   
         else                            color = WHITE;
      }
      else {
         color = GREY;
         sprintf(out, "Lux:        NONE");
      }
      vidstr(row++, col, color, out);

      if(unit_status & LUX2_CHIP_BIT) {
         if(show_cp) sprintf(out, "Lum:    %8.3f cp  ", lux2*lum_scale);
         else        sprintf(out, "Lum:    %8.3f lum ", lux2*lum_scale);
         if(unit_status & LUX2_OVFL_BIT) color = RED;   
         else                            color = WHITE;
      }
      else {
         color = GREY;
         sprintf(out, "Lum:        NONE");
      }
      vidstr(row++, col, color, out);

      color = WHITE;
      sprintf(out, "PWM:    %8.3f Hz  ", pwm_hz);
      vidstr(row++, col, color, out);
   }
   else if(subcode == 0x07) {
      get_luxor_config();
   }
   else if(subcode == 0x10) {
      get_luxor_cal();
   }
   else if(1) {
      plot_title[0] = 0;
      for(i=0; i<9+8+8; i++) {
         buf[i] = tsip_byte();
         sprintf(out, "%02X ", (unsigned) buf[i]);
         strcat(plot_title, out);
      }
      tsip_end(1);
   }
   else {
      tsip_end(1);
   }
}

void set_luxor_time()
{
#ifdef WINDOWS 
SYSTEMTIME t;

   if(luxor == 0) return;

   GetSystemTime(&t);

   send_tsip_start(LUXOR_ID); 
   send_byte(0x02);
   send_byte((u08) t.wSecond);
   send_byte((u08) t.wMinute);
   send_byte((u08) t.wHour);
   send_byte((u08) t.wDay);
   send_byte((u08) t.wMonth);
   send_byte((u08) (t.wYear-2000));
   send_tsip_end();
#else  // __linux__  __MACH__
   time_t rawtime;
   struct tm *tt;
   if(luxor == 0) return;

   time(&rawtime);
   tt = gmtime(&rawtime); // get GMT time
   if(tt) {
      send_tsip_start(LUXOR_ID); 
      send_byte(0x02);
      send_byte((u08) tt->tm_sec);
      send_byte((u08) tt->tm_min);
      send_byte((u08) tt->tm_hour);
      send_byte((u08) tt->tm_mday);
      send_byte((u08) tt->tm_mon+1);
      send_byte((u08) (tt->tm_year%100));
      send_tsip_end();
   }
#endif

   luxor_time_set = 1;
}

void set_luxor_sens(u08 lux1, u08 lux2)
{
   if(luxor == 0) return;

   send_tsip_start(LUXOR_ID); 
   send_byte(0x03);
   send_byte(lux1);
   send_byte(lux2);
   send_tsip_end();
}

void set_batt_pwm(u16 pwm)
{
   if(luxor == 0) return;

   send_tsip_start(LUXOR_ID); 
   send_byte(0x06);
   send_word(pwm);
   send_tsip_end();

   if(pwm == 0) {    // cancel alarm timers
      alarm_time = alarm_date = 0;
      egg_timer = 0;
      repeat_egg = 0;
   }
}


void set_pwm_step()
{
   if(batt_pwm_res == 8) PWM_STEP = (256.0F / 65536.0F);
   else if(batt_pwm_res == 9) PWM_STEP = (128.0F / 65536.0F);
   else { 
      PWM_STEP = (64.0F / 65536.0F);
      batt_pwm_res = 10;
   }
}

void set_luxor_config()
{
   if(luxor == 0) return;

   send_tsip_start(LUXOR_ID); 
   send_byte(0x07);
   send_single(batt_lvc);
   send_single(batt_hvc);
   send_single(batt_ovc);
   send_single(batt_watts);
   send_single(load_lvc);
   send_single(load_hvc);
   send_single(load_ovc);
   send_single(load_watts);
   send_single(auxv_lvc);
   send_single(auxv_hvc);
   send_single(tc1_ovt);
   send_single(tc2_ovt);
   send_single(msg_timeout);

   send_single((float) lux1_time);
   send_single((float) lux2_time);
   send_single((float) emis1);
   send_single((float) emis2);

   send_byte(batt_pwm_res);
   send_tsip_end();

   set_pwm_step();

   Sleep(500);   // !!!!!! allow time for message to be processed
}

void get_luxor_config()
{
   if(luxor == 0) return;

   batt_lvc = tsip_single();
   batt_hvc = tsip_single(); 
   batt_ovc = tsip_single(); 
   batt_watts = tsip_single();
   load_lvc = tsip_single(); 
   load_hvc = tsip_single(); 
   load_ovc = tsip_single(); 
   load_watts = tsip_single();
   auxv_lvc = tsip_single(); 
   auxv_hvc = tsip_single(); 
   tc1_ovt = tsip_single(); 
   tc2_ovt = tsip_single(); 
   msg_timeout = tsip_single();

   lux1_time = (u08) tsip_single();
   lux2_time = (u08) tsip_single();
   emis1 = tsip_single(); 
   emis2 = tsip_single(); 

   batt_pwm_res = tsip_byte();
   set_pwm_step();
   tsip_end(1);
}

void set_luxor_cal()
{
   if(luxor == 0) return;

   send_tsip_start(LUXOR_ID); 
   send_byte(0x11);
   send_single(vref_m);
   send_single(vref_b);
   send_single(temp1_m);
   send_single(temp1_b);
   send_single(temp2_m);
   send_single(temp2_b);
   send_single(vcal_m);
   send_single(vcal_b);
   send_single(batti_m);
   send_single(batti_b);
   send_single(ledi_m);
   send_single(ledi_b);
   send_single(lux1_m);
   send_single(lux1_b);
   send_single(lux2_m);
   send_single(lux2_b);
   send_single(adc2_m);
   send_single(adc2_b);
   send_single(rb_m);
   send_single(rb_b);
   send_tsip_end();

   Sleep(500);   // !!!!!! allow time for message to be processed
}

void get_luxor_cal()
{
   if(luxor == 0) return;

   vref_m = tsip_single();
   vref_b = tsip_single();
   temp1_m = tsip_single();
   temp1_b = tsip_single();
   temp2_m = tsip_single();
   temp2_b = tsip_single();
   vcal_m = tsip_single();
   vcal_b = tsip_single();
   batti_m = tsip_single();
   batti_b = tsip_single();
   ledi_m = tsip_single();
   ledi_b = tsip_single();
   lux1_m = tsip_single();
   lux1_b = tsip_single();
   lux2_m = tsip_single();
   lux2_b = tsip_single();
   adc2_m = tsip_single();
   adc2_b = tsip_single();
   rb_m = tsip_single();
   rb_b = tsip_single();

   tsip_end(1);
}


void set_luxor_runtime(u32 seconds)
{
   if(luxor == 0) return;

   send_tsip_start(LUXOR_ID); 
   send_byte(0x0A);
   send_dword(seconds);
   send_tsip_end();
}

void set_luxor_delay(u16 msecs)
{
   if(luxor == 0) return;

   send_tsip_start(LUXOR_ID); 
   send_byte(0x0B);
   send_word(msecs);
   send_tsip_end();
}


#define CLICK_TIME 500


void set_driver_mode()
{
int i, j;
int len;
int count;
float vals[128];
float val;

   // This routine sets the LED driver mode by pulsing the battery on and off.
   // It is also used to switch the battery on, off, or PWM it.

   end_sweep();
   count = 0;
   i = 0;
   len = strlen(edit_buffer);

   while(i < len) {   // get a list of numbers on the line
      for(j=i; j<len; j++) {   // find next number field
         if(edit_buffer[j] == 0) goto no_num;
         if(edit_buffer[j] == 0x0A) goto no_num;
         if(edit_buffer[j] == 0x0D) goto no_num;
         if(edit_buffer[j] == '.') goto got_num;
         if(isdigit(edit_buffer[j])) goto got_num;
         if(edit_buffer[j] == '*') {  // '*' is a double 100 msec pulse
            if(count >= 125) break;
            vals[count++] = 100;     // off
            vals[count++] = 100;     // on
            vals[count++] = 100;     // off
            vals[count++] = 100;     // on
         }
      }
      break;

      got_num:
      i = j;
      val = 0;
      sscanf(&edit_buffer[i], "%f", &val);
      vals[count] = val;
      ++count;
      if(count >= 128) break;

      while(i < len) {  // skip over number
         ++i;
         if(edit_buffer[i] == 0) goto no_num;
         if(edit_buffer[i] == 0x0A) goto no_num;
         if(edit_buffer[i] == 0x0D) goto no_num;
         if(edit_buffer[i] == '.') continue;
         if(edit_buffer[i] < '0') break;
         if(edit_buffer[i] > '9') break;
         if(edit_buffer[i] == '*') break;
      }
   }

   no_num:
   if(count == 0) {  // no numbers on the line, toggle battery state
      if(batt_pwm) {
         set_batt_pwm(0);
      }
      else {
update_check();
         set_batt_pwm(65535);
      }
//    set_luxor_delay(CLICK_TIME);
      Sleep(CLICK_TIME);
   }
   else if((count == 1) && (vals[0] <= 1.0)) {  // single number
      if(vals[0] == 0.0F) {
         set_batt_pwm(0);
      }
      else {
update_check();
         if(vals[0] == 1.0F) set_batt_pwm(65535);
         else set_batt_pwm((u16) (val*65536.0F));
      }
   }
   else {  // pulse the battery
      for(i=0; i<count; i++) {
         if(i & 1) {  // an ON time
            set_batt_pwm(65535);
         }
         else {           // an OFF time
            set_batt_pwm(0);
         }
//       set_luxor_delay((u16) vals[i]);
         Sleep((DWORD) vals[i]);
         set_batt_pwm(65535);
      }
   }
}

void set_emissivity(float em1, float em2)
{
   if(luxor == 0) return;

   send_tsip_start(LUXOR_ID); 
   send_byte(0x04);
   send_single(em1);
   send_single(em2);
   send_tsip_end();
}

void set_cal_mode(u08 cal_mode)
{
   if(luxor == 0) return;

   send_tsip_start(LUXOR_ID); 
   send_byte(0x05);
   send_byte(cal_mode);
   send_tsip_end();
}

void reset_luxor_wdt(u08 flag)
{
   if(luxor == 0) return;

   send_tsip_start(LUXOR_ID); 
   send_byte(0x09);
   send_byte(flag);
   send_tsip_end();
}

//
//
//  TSIP receiver message dispatch stuff
//
//


void request_misc_msg()
{
   // This routine requests various minor status messages
   // It requests a different message each time it is called.

   if(first_request) {
////  request_rcvr_info(200);
      first_request = 0;
      saw_version = 0;
      req_num = 0;
   }

   if(rcvr_type == SCPI_RCVR) return; // these receivers have their own message polling
   if(rcvr_type == STAR_RCVR) return;
   if(rcvr_type == UCCM_RCVR) return;

   ++req_num;
   if     (req_num ==  1) request_version();
   else if(req_num ==  2) request_rcvr_config(2); 
   else if(req_num ==  3) request_timing_mode();
   else if(req_num ==  4) request_pps_mode();
   else if(req_num ==  5) request_pps_info();
   else if(req_num ==  6) request_survey_params();
   else if(req_num ==  7) request_sat_list();
   else if(req_num ==  8) {
      if(log_db == 0) request_sig_levels();
   }
   else if(req_num ==  9) request_sat_health(); 
   else if(req_num == 10) request_datum(); 
   else if(req_num == 11) request_utc_info();  // this message can hose up some res_t receivers
   else if(req_num == 12) request_filter_config();    // tsip only
   else if(rcvr_type == TSIP_RCVR) {
      if     (req_num == 13) request_last_raw(0x00);     // tsip only
      else if(req_num == 14) request_all_dis_params();   // tsip only
      else if(req_num == 15) request_eeprom_status();    // tsip only
      else if(req_num == 16) request_dac_voltage();      // tsip only
      else if(req_num == 17) request_sat_status(0x00);   // tsip only
      else if(req_num == 18) request_eph_status(0x00);   // tsip only
      else if(req_num == 19) request_alm_health();       // tsip only
      else if(req_num == 20) request_io_options();       // tsip only
      else if(req_num == 21) request_packet_mask();      // tsip only
      else if(req_num == 22) request_last_posn();        // tsip only
      else if(saw_version == 0) {   // no response to version message requests
         parity = (parity+1) % 3;   // try different parity
         init_com();
         need_redraw = 2013; // ggggg
         req_num = 0;
      }
      else req_num = 0;
   }
   else req_num = 0;

   #ifdef SAT_TRAILS
      // here we try to get position info for untracked sats,  but tbolts
      // don't seem to want to give up the goods on untracked sats... but
      // we ask anyway
      if(++status_prn > MAX_PRN) status_prn = 1; // !!!! max_sat_check
      request_sat_status(status_prn);
   #endif
}

void timing_msg()
{
   subcode = tsip_byte();

   if     (subcode == 0x15) datums();
   else if(subcode == 0x41) manuf_params();
   else if(subcode == 0x42) prodn_params();
   else if(subcode == 0x4A) pps_settings();
   else if(subcode == 0x4E) get_pps_mode();
   else if(subcode == 0xA0) dac_values();
   else if(subcode == 0xA1) osc_sense();

   else if(subcode == 0xA2) get_timing_mode();
   else if(subcode == 0xA5) packet_mask();
   else if(subcode == 0xA7) sat_solutions();   // not on ThunderBolt-E
   else if(subcode == 0xA8) discipline_params();
   else if(subcode == 0xA9) survey_params();
   else if(subcode == 0xAB) primary_timing(1);
   else if(subcode == 0xAC) secondary_timing(1);
   else if(subcode == 0xAE) ae_packet();
   else                     unknown_msg(0x8F00 | subcode);
}


void unknown_msg(u16 msg_id)
{
   if(msg_id == 0x13) {
      msg_id = get_next_tsip();
      if(log_comments && (log_stream & 0x01) && log_file) {
         fprintf(log_file, "#!! Message rejected: %02X\n      ", msg_id);
         kol = (-1);
      }
   }
   else {
      if(log_comments && (log_stream & 0x01) && log_file) {
         fprintf(log_file, "#!! Unknown message: id %02X:\n      ", msg_id);
         kol = (-1);
      }
   }

   tsip_rptr = tsip_wptr = 0;  // flush the tsip buffer
}


void wakeup_nortel()
{
int row, col;
unsigned c;

   // try to wake up the Nortel NTGS/NTPX/NTBW units

   if(sim_file) return;
   try_nortel = 0;
   saw_nortel = 0;
   if(nortel != 1) return;  

   try_nortel = 0xFF;
   wakeup_tsip_msg = 0;
   waking = 1;

   col = 0;
   row = 1;
   while(wakeup_tsip_msg == 0) {
      vidstr(row,col, RED, "*");
      refresh_page();
      ++col;

      SetDtrLine(1);
      if(NO_SCPI_BREAK && (rcvr_type == SCPI_RCVR)) ; else 
      SendBreak();

      request_sig_levels();           // 0x27
      request_rcvr_health();          // 0x26
      request_sat_list();             // 0x24
      request_sat_status(0x00);       // 0x3C
      request_gps_time();             // 0x21
      request_io_options();           // 0x35
//    request_hw_ver();               // 1C 03
//    request_unk_ver();              // 1C 02
      request_version();              // 1F
      request_rcvr_config(4);         // BB 00
      request_filter_config();        // 70
      set_health_config(0x03, 0x00);  // 39 03 00
      request_c2();                   // C2
      request_7A_00();                // 7A 00
      request_datum();                // 8E 15
      Sleep(500);

      if(1 && process_com) {
         loopy:
         abort_wakeup();
         if(SERIAL_DATA_AVAILABLE()) {
            while(SERIAL_DATA_AVAILABLE()) {
               c = get_com_char();
               while(c == DLE) {
                  if(SERIAL_DATA_AVAILABLE() == 0) {
                     Sleep(100);
                  }
                  if(SERIAL_DATA_AVAILABLE() == 0) {
                     goto loopy;
                  }
                  c = get_com_char();
                  if(c == ETX) goto etx_seen;
                  abort_wakeup();
               }
               abort_wakeup();
            }
            Sleep(100);
            goto loopy;
         }
         abort_wakeup();
         continue;
      }

      etx_seen:
      get_pending_gps();
      abort_wakeup();
   }

   waking = 0;
// set_pps_mode(0x02);  // 1pps mode
// request_pps_mode();
}


void start_msg_decode(int redraw)
{
   // prepare to decode a receiver message

   msg_fault = 0x00;
   tsip_rptr = 0;

   first_msg = 0;
   last_msg = msg_id;
   ++wakeup_tsip_msg; // we have seen a message
   tsip_error = 0;    // this flag gets set if we see a start or ETX when we wanted normal data
   early_end = 0;
   ++packet_count;
   if(packet_count == 1L) {
      if(redraw) need_redraw = 3333;  // get rid of any "no serial..." message
   }
}



void decode_tsip_message()
{
   msg_fault = 0x00;
   tsip_rptr = 0;
   msg_id = get_next_tsip();

   subcode = 0x00;
   first_msg = 0;
   last_msg = msg_id;
   ++wakeup_tsip_msg; // we have seen a TSIP message
   tsip_error = 0;    // this flag gets set if we see a start or ETX when we wanted normal data
   early_end = 0;
   ++packet_count;
   if(packet_count == 1L) {
      need_redraw = 3344;  // get rid of any "no serial..." message
   }

   msg_id &= 0xFF;
   if     (msg_id == 0x13) unknown_msg(0x13);
   else if(msg_id == 0x1C) ebolt_version();
   else if(msg_id == 0x41) get_gps_time();    // ntpx, RES 360
   else if(msg_id == 0x42) single_ecef_fix();
   else if(msg_id == 0x43) velocity_fix();
   else if(msg_id == 0x45) version_info();
   else if(msg_id == 0x46) ebolt_health1();
   else if(msg_id == 0x47) sig_levels();
   else if(msg_id == 0x49) get_alm_health();
   else if(msg_id == 0x4A) single_lla_fix(); 
   else if(msg_id == 0x4B) ebolt_health2();
   else if(msg_id == 0x55) io_options();
   else if(msg_id == 0x56) enu_velocity_fix();
   else if(msg_id == 0x57) last_fix_info();
   else if(msg_id == 0x58) packet_58();
   else if(msg_id == 0x59) sat_health();
   else if(msg_id == 0x5A) raw_data();
   else if(msg_id == 0x5B) eph_status();
   else if(msg_id == 0x5C) sat_tracking(0x5C);
   else if(msg_id == 0x5D) sat_tracking(0x5D); // res 360 receiver
   else if(msg_id == 0x5F) eeprom_status();
   else if(msg_id == 0x6C) sat_list(1);        // res 360 receiver
   else if(msg_id == 0x6D) sat_list(0);
   else if(msg_id == 0x70) filter_config();
   else if(msg_id == 0x83) ecef_fix();
   else if(msg_id == 0x84) lla_fix();
   else if(msg_id == 0x8F) timing_msg();
   else if(msg_id == 0xBB) rcvr_config();
   else if(msg_id == LUXOR_ID) luxor_packet();
   else                        unknown_msg(msg_id);

   if(com_error) {    // we had a com timeout so were skipping serial reads
      com_error = 0;  // we can stop searching since data is now comming in
   }
}


void get_tsip_message()
{
u08 c;

   // this routine buffers up an incoming TSIP message and then parses it
   // when it is complete.

   if(SERIAL_DATA_AVAILABLE() == 0) {
      check_com_timer();
      return;
   }
   else {
      reset_com_timer();
   }

   c = get_com_char();
   if(rcv_error) {      // parity/framing/overrun errors
      rcv_error = 0;
//!!!!!qqqq    goto rst_msg;
   }

   if(tsip_sync == 0) {         // syncing to start of message, search for a DLE
      if(c == DLE) {
         tsip_sync = 1;
         get_sync_time();
      }
      tsip_wptr = 0;
      tsip_rptr = 0;
      return;
   }
   else if(tsip_sync == 1) {    // DLE had been seen, now checking next byte
      if(c == DLE) {            // DLE DLE is a 0x10 data byte
         goto rst_msg;
      }
      else if(c == ETX) {       // DLE ETX is end-of-message
         goto rst_msg;          // ... should never happen here
      }
      else {                    // DLE xx is message start
         tsip_sync = 2;         // ... so accumulate the message
         if(tsip_wptr < MAX_TSIP) {
            tsip_buf[tsip_wptr++] = c;
         }
         else {                 // buffer overlow
            tsip_error |= 0x8000; // !!!!t
            goto rst_msg;
         }
      }
   }
   else if(tsip_sync == 2) {    // buffer up the message
      if(c == DLE) tsip_sync = 3;
      else if(tsip_wptr < MAX_TSIP) {
         tsip_buf[tsip_wptr++] = c;
      }
      else {
         tsip_error |= 0x8000;   // !!!!t
      }
   }
   else if(tsip_sync == 3) {   // last char was a DLE
      if(c == ETX) {           // DLE ETX is end-of-message
         decode_tsip_message(); // so process the buffered message

         rst_msg:
         tsip_wptr = 0;
         tsip_sync = 0;
      }
      else {                   // DLE DLE is a DLE data byte and DLE xx is message ID
         if(tsip_wptr < MAX_TSIP) {  // so add it to the message buffer
            tsip_buf[tsip_wptr++] = c;
         }
         else {
            tsip_error |= 0x8000;
         }
         tsip_sync = 2;
      }
   }
   else {     // should never happen
      goto rst_msg;
   }
}



//
//
//   NMEA receiver stuff
//
//


char nmea_msg[NMEA_MSG_SIZE+1];     // nmea message buffer
char nmea_field[NMEA_MSG_SIZE+1];   // fields extracted from message buffer come here
int nmea_col;           // used to get chars from the nmea buffer to the field buffer

int last_was_gsv;

int get_nmea_field()
{
char c;
int i;

   // exract the next comma separated field from a message (this routine is
   // used by several different receiver types besides NMEA)

   nmea_field[0] = 0;
   if(nmea_msg[nmea_col] == 0) return 0;  // at end-of-message

   i = 0;
   while(nmea_col < (int)sizeof(nmea_msg)) {   // get next field from the nmea message
      c = nmea_msg[nmea_col];
      if(c == 0) {  // reached end-of-message
         break;
      }
      else if(c == ',') { // end of comma delimted field
         ++nmea_col;
         break;
      }
      else if((rcvr_type == STAR_RCVR) && (c == '=')) {
         ++nmea_col;
         break;
      }
      else {        // add char to extracted field
         nmea_field[i++] = c;
         nmea_field[i] = 0;
         ++nmea_col;
      }
   }

   return nmea_field[0];
}


void parse_gpgsa(int system)
{
int mode_flag;
int fix_status;
int prn;
int i;

   if((system & system_mask) == 0) return;

   last_was_gsv = 0;
   if(get_nmea_field()) mode_flag = nmea_field[0];

   if(get_nmea_field()) {
      fix_status = atoi(nmea_field);
      if     (fix_status == 1) rcvr_mode = RCVR_MODE_2D;  
      else if(fix_status == 2) rcvr_mode = RCVR_MODE_ACQUIRE;
      else if(fix_status == 3) rcvr_mode = RCVR_MODE_3D;
      else                     rcvr_mode = RCVR_MODE_UNKNOWN;
      have_tracking = 1;
      if(rcvr_mode == RCVR_MODE_NO_SATS) minor_alarms |= 0x0008;
      else                               minor_alarms &= (~0x0008);
      have_rcvr_mode = 2;
   }

   for(i=1; i<=12; i++) {
      if(get_nmea_field()) {
         prn = atoi(nmea_field);
         if((prn >= 1) && (prn <= MAX_PRN)) { 
            sat[prn].tracking = prn;
            sat[prn].level_msg = 2;
         }
      }
   }

   have_dops = 0;
   if(get_nmea_field()) {
      pdop = (float) atof(nmea_field);
      have_dops |= PDOP;
   }
   if(get_nmea_field()) {
      hdop = (float) atof(nmea_field);
      have_dops |= HDOP;
   }
   if(get_nmea_field()) {
      vdop = (float) atof(nmea_field);
      have_dops |= VDOP;
   }
}

void parse_gpgsv(int system)
{
int num_sents;
int sent_num;
int sat_count;
int prn;
int az, el;
int snr;
int i;

   if((system & system_mask) == 0) return;

   if(get_nmea_field()) {
      num_sents = atoi(nmea_field);
   }
   else return;

   if(get_nmea_field()) {
      sent_num = atoi(nmea_field);
   }
   else return;

   if(get_nmea_field()) {
      sat_count = atoi(nmea_field);
      have_count = 1;
   }

//   if(sent_num == 1) {  // reset sat tracking info 
   if(last_was_gsv == 0) {  // reset sat tracking info 
      reset_sat_tracking();
   }

   last_was_gsv = 1;

   for(i=0; i<4; i++) {
      if(get_nmea_field()) prn = atoi(nmea_field);
      else continue;

      if(get_nmea_field()) el = atoi(nmea_field);
      else continue;

      if(get_nmea_field()) az = atoi(nmea_field);
      else continue;

      if(get_nmea_field()) snr = atoi(nmea_field);
      else {
         snr = (0);
//       continue;
      }

      if((prn >= 1) && (prn <= MAX_PRN)) { 
         sat[prn].level_msg = 1;
         sat[prn].azimuth = (float) az;
         set_sat_el(prn, (float) el);
         sat[prn].sig_level = (float) snr;
         if(snr > 0) sat[prn].tracking = prn;
         else        sat[prn].tracking = (-1);
         have_sat_azel = 2;

         record_sig_levels(prn);
      }
   }

   if(sent_num == num_sents) {
      level_type = "SNR";
   }

   config_sat_count(sat_count);
}

void get_nmea_lat()
{
double deg;

   if(get_nmea_field()) {
      deg = atof(nmea_field);
      lat = (double) (int) (deg / 100.0);
      deg = deg - (lat * 100.0);
      deg /= 60.0;
      lat += deg;
      lat *= (PI / 180.0);
   }
   if(get_nmea_field()) {
      if(nmea_field[0] == 'S') lat = 0.0 - lat;
   }
}

void get_nmea_lon()
{
double deg;

   if(get_nmea_field()) {
      deg = atof(nmea_field);
      lon = (double) (int) (deg / 100.0);
      deg = deg - (lon * 100.0);
      deg /= 60.0;
      lon += deg;
      lon *= (PI / 180.0);
   }
   if(get_nmea_field()) {
      if(nmea_field[0] == 'W') lon = 0.0 - lon;
   }
}

void get_nmea_alt()
{
   if(get_nmea_field()) {  //altitude
      alt = atoi(nmea_field);
      get_nmea_field();    // meters or feet
      if(nmea_field[0] == 'F') alt = alt * FEET_PER_METER;
   }
   else get_nmea_field();  // meters or feet
}

void get_nmea_time()
{
double ftime;
int time;

   if(get_nmea_field()) {  // time
      ftime = atof(nmea_field);  // time field can be floating point
      time = (int) ftime;
      pri_hours = hours = (time / 10000);
      pri_minutes = minutes = ((time / 100) % 100);
      pri_seconds = seconds = ((time / 1) % 100);
      pri_frac = raw_frac = ftime - (double) time;
//vvvv   time_flags |= 0x0001;  // UTC based time
      if(timing_mode == 0) {  // convert utc time to gps time
         utc_to_gps();
      }
   }
}

void get_nmea_date()
{
int date;

   if(get_nmea_field()) {
      date = atoi(nmea_field);
        pri_day = day = (date / 10000);
        pri_month = month = ((date / 100) % 100);
        pri_year = ((date / 1) % 100);
        pri_year += 2000;
        year = pri_year;
   }
}

void parse_gpgns(int system)
{
double old_lat, old_lon, old_alt;
int old_hours;
int old_minutes;
int old_seconds;
int old_year;
int old_month;
int old_day;
double old_frac;

int old_pri_hours;
int old_pri_minutes;
int old_pri_seconds;
int old_pri_year;
int old_pri_month;
int old_pri_day;
double old_pri_frac;

   have_gpgns = system;
   last_was_gsv = 0;

   old_lat = lat;
   old_lon = lon;
   old_alt = alt;

   old_pri_hours = pri_hours;
   old_pri_minutes = pri_minutes;
   old_pri_seconds = pri_seconds;
   old_pri_year = pri_year;
   old_pri_month = pri_month;
   old_pri_day = pri_day;
   old_pri_frac = pri_frac;

   old_hours = hours;
   old_minutes = minutes;
   old_seconds = seconds;
   old_year = year;
   old_month = month;
   old_day = day;
   old_frac = raw_frac;

   get_nmea_time();
   get_nmea_lat();
   get_nmea_lon();

   if(get_nmea_field()) {  // mode
      if(strstr(nmea_field, "N")) {  // invalid fix
         reject:
         lat = old_lat;
         lon = old_lon;
         alt = old_alt;

         pri_hours = old_pri_hours;   
         pri_minutes = old_pri_minutes; 
         pri_seconds = old_pri_seconds; 
         pri_year = old_pri_year;    
         pri_month = old_pri_month;   
         pri_day = old_pri_day;     
         pri_frac = old_pri_frac;    

         hours = old_hours;   
         minutes = old_minutes; 
         seconds = old_seconds; 
         year = old_year;    
         month = old_month;   
         day = old_day;     
         raw_frac = old_frac;
         return;
      }
   }
   else goto reject;

   if(get_nmea_field()) {  // sat count
      sat_count = atoi(nmea_field);
      have_count = 2;
   }

   get_nmea_field();  // HDROP

   if(get_nmea_field()) alt = atoi(nmea_field);

   config_sat_count(sat_count);
   drive_nmea_screen(system, 1);
}


void parse_gprmc(int system)
{
   if((system & system_mask) == 0) return;

   last_was_gsv = 0;
   have_gprmc = system;

   get_nmea_time();
   get_nmea_field();  // active flag A=good  V=invalid fix

   if(nmea_field[0] == 'A') {
      get_nmea_lat();
      get_nmea_lon();

      if(get_nmea_field()) {  // speed (km/hr)
         speed = atof(nmea_field) * 0.514444;
         have_speed = 4;
      }
      if(get_nmea_field()) {  // course angle
         heading = atof(nmea_field);
         have_heading = 4;
      }

      get_nmea_date();
   }

   drive_nmea_screen(system, 2);
}


void parse_gpgga(int system)
{
int fix;

   if((system & system_mask) == 0) return;

   last_was_gsv = 0;
   have_gpgga = system;

   get_nmea_time();  // was get_nmea_field();
   get_nmea_lat();
   get_nmea_lon();

   get_nmea_field();  // fix status   zzzzz - verify fix is good
   if(nmea_field[0]) fix = atoi(nmea_field);
   else              fix = 0;

   if(get_nmea_field()) { // sat count
      sat_count = atoi(nmea_field);
      have_count = 3;
   }

   if(get_nmea_field()) { // hdop
      hdop = (float) atof(nmea_field);
      have_dops |= HDOP;
   }

   get_nmea_alt();

   get_nmea_field();  // geiod height
   get_nmea_field();  // dgps time
   get_nmea_field();  // dgps station

   config_sat_count(sat_count);

   drive_nmea_screen(system, 3);
}

void parse_gpzda(int system)
{
   if((system & system_mask) == 0) return;

   have_gpzda = system;

   last_was_gsv = 0;

   get_nmea_time();
   if(get_nmea_field()) pri_day = day = atoi(nmea_field);
   if(get_nmea_field()) pri_month = month = atoi(nmea_field);
   if(get_nmea_field()) pri_year = year = atoi(nmea_field);

   get_nmea_field();  // time zone info

   drive_nmea_screen(system, 4);
}

void config_venus_timing()
{
   if((saw_timing_msg == 0) || (have_sawtooth == 0)) {
      if(rcvr_type != VENUS_RCVR) {
         rcvr_type = VENUS_RCVR;
         config_rcvr_type(0);
      }
      else {
         config_rcvr_plots();
         config_msg_ofs();
      }

      plot[ONE].show_plot = 0;
      plot[TWO].show_plot = 0;
      plot[THREE].show_plot = 0;
      plot[SIX].show_plot = 0;
      plot[DAC].show_plot = 1;
   }
}

void parse_psti(int system)
{
int id;
int mode;
int survey_len;
float sawtooth;
float sdev1;
float sdev2;

   // handle the Venus timing receiver NMEA message
   // !!!!!! should we set rcvr_type to VENUS_RCVR?

   nmea_type = VENUS_NMEA;
   if((system & system_mask) == 0) return;

   if(get_nmea_field()) {
      id = atoi(nmea_field);
      if(id != 0) return;

      config_venus_timing();
      saw_timing_msg |= 0x08;

      if(get_nmea_field()) {
         mode = atoi(nmea_field);

         if(mode == 0) { // PVT mode
            minor_alarms &= (~0x0020);
            venus_hold_mode = 0;
            rcvr_mode = RCVR_MODE_3D;
         }
         else if(mode == 1) {  // surveying
            minor_alarms |= (0x0020);
            venus_hold_mode = 0;
            rcvr_mode = RCVR_MODE_SURVEY;
         }
         else if(mode == 2) {  // position hold
            minor_alarms &= (~0x0020);
            venus_hold_mode = 1;
            rcvr_mode = RCVR_MODE_HOLD;
         }
      }

      if(get_nmea_field()) {
         survey_len = atoi(nmea_field);
      }

      if(get_nmea_field()) {
         sawtooth = (float) atof(nmea_field);
         have_sawtooth = 1;
         dac_voltage = sawtooth;
      }

      if(get_nmea_field()) {
         sdev1 = (float) atof(nmea_field);
      }

      if(get_nmea_field()) {
         sdev2 = (float) atof(nmea_field);
      }
   }

   return;
}


void parse_pgrmf(int system)
{
int i;
int fix_status;

   // handle the Garmin PGRMF GPS system data NMEA message
return;  // !!!!! disabled until it can be tested

   nmea_type = GARMIN_NMEA;
   if((system & system_mask) == 0) return;

   if(get_nmea_field()) {  // 1 - GPS week
      gps_week = atoi(nmea_field);
      if(have_week == 0) need_redraw = 3354;
      have_week = 199;
   }

   if(get_nmea_field()) {  // 2 - GPS TOW
      pri_tow = atoi(nmea_field);
      tow = pri_tow;  
      this_tow = tow;
      survey_tow = tow;

      if(have_tow == 0) need_redraw = 3029;
      have_tow = 55;
   }

   get_nmea_date();    // 3 - date: ddmmyy
   get_nmea_time();    // 4 - time: hhmmss

   if(get_nmea_field()) {  // 5 - leap second count
      i = get_nmea_field();
      if(!user_set_utc_ofs) utc_offset = i;
      check_utc_ofs(33);
   }

   get_nmea_lat();         // 6 and 7
   get_nmea_lon();         // 8 and 9


   if(get_nmea_field()) {  // 10 - mode - M)anual  A)automatic
   }

   if(get_nmea_field()) {  // 11 - fix type: 0=none  1=2D  2=3D
      if(1) {  // !!!! do we want to get this from GPGSA
         fix_status = atoi(nmea_field);
         if     (fix_status == 0) rcvr_mode = RCVR_MODE_NO_SATS;  
         else if(fix_status == 1) rcvr_mode = RCVR_MODE_2D;  
         else if(fix_status == 2) rcvr_mode = RCVR_MODE_3D;
         else                     rcvr_mode = RCVR_MODE_UNKNOWN;
         have_tracking = 1;
         if(rcvr_mode == RCVR_MODE_NO_SATS) minor_alarms |= 0x0008;
         else                               minor_alarms &= (~0x0008);
         have_rcvr_mode = 2;
      }
   }

   if(get_nmea_field()) {  // 12 - speed (km/hr)
      speed = atof(nmea_field) * 0.514444;  // meters per second
      have_speed = 20;
   }

   if(get_nmea_field()) {  // 13 - heading 0..359
      heading = atof(nmea_field);
      have_heading = 20;
   }

   if(get_nmea_field()) {  // 14 - PDOP
      pdop = (float) atof(nmea_field);
      have_dops |= PDOP;
   }

   if(get_nmea_field()) {  // 15 - TDOP
      tdop = (float) atof(nmea_field);
      have_dops |= TDOP;
   }
}


void parse_pgrmt(int system)
{
   // handle the Garmin PGRMT sensor status NMEA message
return;  // !!!!! disabled until it can be tested

   nmea_type = GARMIN_NMEA;
   if((system & system_mask) == 0) return;

   critical_alarms = 0x0000;
   minor_alarms = 0x0000;
   have_critical_alarms = 555;

   if(get_nmea_field()) {  // 1 - receiver id
   }

   if(get_nmea_field()) {  // 2 - ROM check
      if(strchr(nmea_field, 'F')) critical_alarms |= 0x0001;
   }

   if(get_nmea_field()) {  // 3 - Rcvr check
   }

   if(get_nmea_field()) {  // 4 - BBRAM check
      if(strchr(nmea_field, 'L')) minor_alarms |= 0x0400;
      have_eeprom = 555;
   }

   if(get_nmea_field()) {  // 5 - RTC check
      if(strchr(nmea_field, 'L')) critical_alarms |= 0x0010;
   }

   if(get_nmea_field()) {  // 6 - OSC check
      if(strchr(nmea_field, 'F')) minor_alarms |= 0x0001;
      have_osc_age = 555;
   }

   if(get_nmea_field()) {  // 7 - data collection
   }

   if(get_nmea_field()) {  // 8 - temperature
      temperature = (float) atof(nmea_field);
      have_temperature = 555;
   }

   if(get_nmea_field()) {  // 9 - config data OK
      if(strchr(nmea_field, 'L')) minor_alarms |= 0x0400;
      have_eeprom = 555;
   }
}



void update_gps_screen(int why)
{
double jd0;

   // this routine handles the primary_timing() function for non-TSIP receivers
   // It drives the main screen update cycle

   // once a Venus receiver has NMEA messages enabled, it stops sending the
   // NAV message, so we need to fake the tow and gps week numbers from the
   // NMEA date/time
   if((rcvr_type == VENUS_RCVR) || (rcvr_type == STAR_RCVR) || (rcvr_type == ACRON_RCVR)) {
      tow = fake_tow(jd_utc);
      if(timing_mode) tow += utc_offset;
      tow %= (7L*24L*60L*60L);
      this_tow = survey_tow = pri_tow = tow;
      have_tow = 99;

      jd0 = jd_utc;
      if(timing_mode) jd0 += jtime(0,0,utc_offset,0.0);  // we are in UTC time, convert to GPS time

      jd0 -= GPS_EPOCH;
      gps_week = (int) (jd0 / 7.0);
      if(have_week == 0) need_redraw = 2054;
      have_week = 99;
//sprintf(plot_title, "fake week:%d  jd0:%f  tmode:%d  ofs:%d", gps_week,jd0, timing_mode, utc_offset);
   }
   else if(!have_tow) {
      tow = this_tow = survey_tow = pri_tow = fake_tow(jd_utc) % (7L*24L*60L*60L);
   }

   if((rcvr_type != SCPI_RCVR) && (rcvr_type != UCCM_RCVR)) {
      have_info |= MISC_INFO;
   }

   primary_timing(0);     // show time and lla info

   secondary_timing(0);   // validate receiver time, etc values and update screen
}


void drive_nmea_screen(int system, int why)
{
static int last_hours = 0;
static int last_minutes = 0;
static int last_seconds = 0;
static int last_month = 0;
static int last_day = 0;
static int last_year = 0;
static double last_frac;
static int last_why;

   if     (hours != last_hours) ;      // filter out duplicate time stamps since
   else if(minutes != last_minutes) ;  // ... NMEA sends the time in several 
   else if(seconds != last_seconds) ;  // ... different messages and different
   else if(month != last_month) ;      // ... receivers send different sets
   else if(day != last_day) ;          // ... of messages that may have the
   else if(year != last_year) ;        // ... same time stamps.
   else if((nav_rate != 1.0) && (last_frac != raw_frac)) ;
   else return;

   last_hours = hours;
   last_minutes = minutes;
   last_seconds = seconds;
   last_month = month;
   last_day = day;
   last_year = year;
   last_frac = raw_frac;

   update_gps_screen(system);

//sprintf(plot_title, "dns last:%d this:%d", last_why,why);
   last_why = why;
}



void decode_nmea_msg()
{
char c;
int i;
static int row = 0;

   venus_nmea = 0;
   start_msg_decode(1);

   nmea_msg[0] = 0;
   for(i=0; i<tsip_wptr; i++) {  // copy message from tsip buffer to nmea buffer
      c = tsip_byte();
      nmea_msg[i] = c;
      nmea_msg[i+1] = 0;
      if(i >= ((int)sizeof(nmea_msg)-12)) break;
   }

   nmea_col = 0;     // index of next char to get from nmea buffer
   get_nmea_field(); // get the NMEA message type 

   if     (!strcmp(nmea_field, "GPGSV")) parse_gpgsv(GPS);
   else if(!strcmp(nmea_field, "BDGSV")) parse_gpgsv(BEIDOU);
   else if(!strcmp(nmea_field, "GBGSV")) parse_gpgsv(BEIDOU);
   else if(!strcmp(nmea_field, "GLGSV")) parse_gpgsv(GLONASS);
   else if(!strcmp(nmea_field, "GAGSV")) parse_gpgsv(GALILEO);
   else if(!strcmp(nmea_field, "GNGSV")) parse_gpgsv(MIXED);

   else if(!strcmp(nmea_field, "GPGSA")) parse_gpgsa(GPS);
   else if(!strcmp(nmea_field, "BDGSA")) parse_gpgsa(BEIDOU);
   else if(!strcmp(nmea_field, "GBGSA")) parse_gpgsa(BEIDOU);
   else if(!strcmp(nmea_field, "GLGSA")) parse_gpgsa(GLONASS);
   else if(!strcmp(nmea_field, "GAGSA")) parse_gpgsa(GALILEO);
   else if(!strcmp(nmea_field, "GNGSA")) parse_gpgsa(MIXED);

   else if(!strcmp(nmea_field, "GPGGA")) parse_gpgga(GPS);
   else if(!strcmp(nmea_field, "BDGGA")) parse_gpgga(BEIDOU);
   else if(!strcmp(nmea_field, "GBGGA")) parse_gpgga(BEIDOU);
   else if(!strcmp(nmea_field, "GLGGA")) parse_gpgga(GLONASS);
   else if(!strcmp(nmea_field, "GAGGA")) parse_gpgga(GALILEO);
   else if(!strcmp(nmea_field, "GNGGA")) parse_gpgga(MIXED);

   else if(!strcmp(nmea_field, "GPRMC")) parse_gprmc(GPS);
   else if(!strcmp(nmea_field, "BDRCM")) parse_gprmc(BEIDOU);
   else if(!strcmp(nmea_field, "GBRMC")) parse_gprmc(BEIDOU);
   else if(!strcmp(nmea_field, "GLRMC")) parse_gprmc(GLONASS);
   else if(!strcmp(nmea_field, "GARMC")) parse_gprmc(GALILEO);
   else if(!strcmp(nmea_field, "GNRMC")) parse_gprmc(MIXED);

   else if(!strcmp(nmea_field, "GPZDA")) parse_gpzda(GPS);
   else if(!strcmp(nmea_field, "BDZDA")) parse_gpzda(BEIDOU);
   else if(!strcmp(nmea_field, "GBZDA")) parse_gpzda(BEIDOU);
   else if(!strcmp(nmea_field, "GLZDA")) parse_gpzda(GLONASS);
   else if(!strcmp(nmea_field, "GAZDA")) parse_gpzda(GALILEO);
   else if(!strcmp(nmea_field, "GNZDA")) parse_gpzda(MIXED);

// else if(!strcmp(nmea_field, "GPGNS")) parse_gpgns(GPS);
// else if(!strcmp(nmea_field, "BDGNS")) parse_gpgns(BEIDOU);
// else if(!strcmp(nmea_field, "GBGNS")) parse_gpgns(BEIDOU);
// else if(!strcmp(nmea_field, "GLGNS")) parse_gpgns(GLONASS);
// else if(!strcmp(nmea_field, "GAGNS")) parse_gpgns(GALILEO);
// else if(!strcmp(nmea_field, "GNGNS")) parse_gpgns(MIXED);

   else if(!strcmp(nmea_field, "PGRMT")) parse_pgrmt(MIXED);   // Garmin GPS sensor status
   else if(!strcmp(nmea_field, "PGRMF")) parse_pgrmf(MIXED);   // Garmin GPS system data
   else if(!strcmp(nmea_field, "PSTI"))  parse_psti(MIXED);

   else last_was_gsv = 0;
   return;
}


void get_nmea_message()
{
u08 c;

   // This routine buffers up an incoming NMEA message.  When the end of the
   // message is seen, the message is parsed and decoded with decode_nmea_msg()

   if(SERIAL_DATA_AVAILABLE() == 0) {
      check_com_timer();
      return;
   }
   else {
      reset_com_timer();
   }

   c = get_com_char();
   if(rcv_error) {      // parity/framing/overrun errors
      rcv_error = 0;
//!!!!!qqqq    goto rst_msg;
   }

   if(tsip_sync == 0) {    // syncing to start of message, search for a '$'
      if(c == '$') {
         tsip_sync = 1;
         get_sync_time();
      }
      tsip_wptr = 0;
      tsip_rptr = 0;
      nmea_vfy_cksum = 0;  // the calculated message checksum
      nmea_msg_cksum = 0;  // the checksum in the message
      return;
   }
   else if(tsip_sync == 1) { // '$' has been seen, now build the message
      if((c == 0x0D) || (c == 0x0A)) goto nmea_end;  // message has no checksum

      if(c == '*') tsip_sync = 2;  // message checksum follows
      else if(tsip_wptr < MAX_TSIP) {   // add char to the message buffer
         nmea_vfy_cksum ^= c;
         tsip_buf[tsip_wptr++] = c;
      }
      else {
         tsip_error |= 0x8000;
         goto rst_msg;
      }
   }
   else if(tsip_sync == 2) {  // getting message checksum chars
      if((c == 0x0D) || (c == 0x0A)) goto nmea_end;

      if     ((c >= '0') && (c <= '9')) nmea_msg_cksum = (nmea_msg_cksum << 4) | (c-'0');
      else if((c >= 'A') && (c <= 'F')) nmea_msg_cksum = (nmea_msg_cksum << 4) | ((c-'A')+10);
      else goto rst_msg;
   }
   else if(tsip_sync == 3) {  // end of message
      nmea_end:
      if((tsip_sync == 2) && (nmea_vfy_cksum != nmea_msg_cksum)) { // checksum error
         tsip_error |= 0x8000;
         goto rst_msg;
      }
      decode_nmea_msg(); // message checksum matches, process the message
      tsip_wptr = 0;     // prepare for next message
      tsip_sync = 0;
      venus_nmea = 0;
   }
   else {     // should never happen, prepare for next message
      rst_msg:
      tsip_wptr = 0;
      tsip_sync = 0;
      nmea_vfy_cksum = nmea_msg_cksum = 0; // init received message checksum
      venus_nmea = 0;
   }
}

//
//
//   Oscilloquartz STAR4 receiver stuff (also NEC GPSDO)
//
//

void poll_next_star(int why)
{
int test;

   // request an item from a STAR receiver.  Requests are alternated between
   // a psoition/time message and some receiver parameter.  The parameter requests
   // alternate between one of three important parameters and then the lesser
   // important ones.

   star_line = 0;
   star_msg = 0;

   test = 1;
   if(test && (scpi_in != scpi_out)) {  // we have a special message request queued
      send_queued_scpi(1);
   }
   else if(scpi_msg_id == STAR_POS_MSG) { // request next misc message every time
      if((test == 0) && (scpi_in != scpi_out)) {  // we have a special message request queued
         send_queued_scpi(1);
      }
      if(scpi_req < 0) scpi_req = 0;

      if(1 && ((scpi_seq % 3) == 0)) send_scpi_cmd("TEMPERATURE;",  STAR_TEMP_MSG); 
      else {
         if     (scpi_req == 0)  send_scpi_cmd("GPS_TIME;",         STAR_GPS_TIME_MSG);
         else if(scpi_req == 1)  send_scpi_cmd("MASK_ANGLE;",       STAR_ANGLE_MSG);
         else if(scpi_req == 2)  send_scpi_cmd("INV;",              STAR_INV_MSG);
         else if(scpi_req == 3)  send_scpi_cmd("CONF;",             STAR_CONF_MSG);

         else if(scpi_req == 4)  send_scpi_cmd("ALARM;",            STAR_ALARM_MSG);
         else if(scpi_req == 5)  send_scpi_cmd("HBSQ;",             STAR_HBSQ_MSG);
         else if(scpi_req == 6)  send_scpi_cmd("ATDC_STATUS;",      STAR_ATDC_MSG);
         else if(scpi_req == 7)  send_scpi_cmd("HOLD_PERF_STATUS;", STAR_PERF_MSG);

         else if(scpi_req == 8)  send_scpi_cmd("OUTPUT_STATE;",     STAR_STATE_MSG);   // !!! multi-line response 
         else if(scpi_req == 9)  send_scpi_cmd("STATUS;",           STAR_STATUS_MSG);
         else if(scpi_req == 10) send_scpi_cmd("TOD_STATE;",        STAR_TOD_MSG);
         else if(scpi_req == 11) send_scpi_cmd("TYPE;",             STAR_TYPE_MSG);
         else if(scpi_req == 12) send_scpi_cmd("ALARM_MASK;",       STAR_ALMASK_MSG);
         else if(scpi_req == 13) send_scpi_cmd("INPUT_TYPE(1);",    STAR_INP_TYPE_MSG);

         ++scpi_req;
         if(scpi_req > 13) scpi_req = 0;
      }

      ++scpi_seq;
   }
   else {
      send_scpi_cmd("INFO_GPS_POS;", STAR_POS_MSG);  // request time and location message
   }
}


void parse_star_alarm()
{
int i;
int count;

   // device alarms and status

   minor_alarms &= (~0x0006);
   minor_alarms &= (~0x0008);
   minor_alarms &= (~0x0020);
   minor_alarms &= (~0x1000);
   time_flags &= (~0x0004);
   have_antenna = 222;

   critical_alarms &= (~0x0008);
   critical_alarms &= (~0x0010);
   if(have_critical_alarms == 0) need_redraw = 222;
   have_critical_alarms = 222;

   count = 0;
   while(get_nmea_field()) {
      if(++count > 12) break;

      if(isdigit(nmea_field[0])) {
         i = atoi(nmea_field);
         if(i == 1) {       // warmup
            discipline_mode = 10;
         }
         else if(i == 2) {  // holdover
            discipline_mode = 2;
         }
         else if(i == 3) {  // tracked fast
            discipline_mode = 5;
         }
         else if(i == 4) {  // ocxo fail
            critical_alarms |= 0x0010;
         }
         else if(i == 5) {  // squelched
            discipline_mode = 4;
         }
         else if(i == 6) {  // gps timing error
//          minor_alarms |= 0x1000;
            time_flags |= 0x0004;
         }
         else if(i == 7) {  // gps fail
            critical_alarms |= 0x0008;
         }
         else if(i == 8) {  // antenna fault
            minor_alarms |= 0x0006;
         }
         else if(i == 9) {  // not surveyed
            minor_alarms |= 0x0020;
         }
         else if(i == 10) { // temperature fault
         }
         else if(i == 11) { // missing aux PPS input
         }
      }
   }
}


void parse_star_conf()   
{
   // device configuration info

   if(get_nmea_field()) {  // user time constant
      time_constant = (float) atof(nmea_field);
      have_osc_params |= PARAM_TC;
      have_tc |= 0x01;
   }

   if(get_nmea_field()) {  // real pll time constant
      real_time_constant = (float) atof(nmea_field);
      have_tc |= 0x02;
   }

   if(get_nmea_field()) {  // user mode (A)utomatic or (H)oldover
      if(nmea_field[0] == 'H') rcvr_mode = RCVR_MODE_PROP;       // user holdover
      else if(nmea_field[0] == 'A') {
         if(minor_alarms & 0x0020) rcvr_mode = RCVR_MODE_3D;
         else rcvr_mode = RCVR_MODE_HOLD;  // position hold
      }
      else rcvr_mode = RCVR_MODE_UNKNOWN;
   }

   if(get_nmea_field()) {  // time zone - !!!! we don't do this
   }

   if(get_nmea_field()) {  // cable delay
      cable_delay = atof(nmea_field) / 1.0E9;
      have_cable_delay = 22;
   }
}


void parse_star_hbsq()  
{
   // output squelch control

   user_hbsq = current_hbsq = (-1);

   if(get_nmea_field()) {  // cable delay
      user_hbsq = atoi(nmea_field);
   }
   if(get_nmea_field()) {  // cable delay
      current_hbsq = atoi(nmea_field);
   }
}


void parse_star_gps_time()   
{
int ls;
int dd,mm,yy;
int hrs,mins,secs;
char c[2];

   // gps time / leapsecond info

   if(get_nmea_field()) {  // GPS week
      gps_week = atoi(nmea_field);
      if(gps_week == 0) return;   // bogus 0 values will be in the message
      if(have_week == 0) need_redraw = 4354;
      have_week = 299;
   }

   if(get_nmea_field()) {  // GPS TOW
      if(0) {
         pri_tow = atoi(nmea_field);
         tow = pri_tow;  
         this_tow = tow;
         survey_tow = tow;

         if(have_tow == 0) need_redraw = 4029;
         have_tow = 59;
      }
   }

   dd = mm = yy = 0;
   hrs = mins = secs = 0;
   if(get_nmea_field()) { // leap dd/mm/yy
      sscanf(nmea_field, "%d%c%d%c%d", &dd,&c[0],&mm,&c[0],&yy);
   }
   if(get_nmea_field()) { // leap hh/mm/ss
      sscanf(nmea_field, "%d%c%d%c%d", &hrs,&c[0],&mins,&c[0],&secs);
   }

   if(get_nmea_field()) { // leap pending
      if(gps_week) {  // if week is 0, the message was invalid
         ls = atoi(nmea_field);
         if(ls) {
            minor_alarms |= 0x0080;

            if(time_flags & 0x0008) {
               jd_leap = 0.0;
               have_jd_leap = 0;
               have_leap_days = 0;
            }
            else if(have_time && (yy || dd || mm || hrs || mins || secs)) {
               if(yy && (yy < 2000)) yy += 2000;
               jd_leap = jdate(yy,mm,dd) + jtime(hrs,mins,secs,0.0) - jtime(0,0,1,0.0);
               calc_jd_leap(5);
            }
         }
         else {
            minor_alarms &= (~0x0080);
            jd_leap = 0.0;
            have_jd_leap = 0;
            have_leap_days = 0;
         }
         have_leap_info = 32;
      }
   }

   if(get_nmea_field()) {  // leap second count
      ls = atoi(nmea_field);
      if(!user_set_utc_ofs) utc_offset = ls;
      check_utc_ofs(39);
   }
}

// #define FIX_STAR_TIME 0
#define FIX_STAR_TIME 3

void parse_star_pos()    
{
int dd,mm,yy;
int hrs,mins,secs;
int ff;
char s[2];
char dir[2];
static int last_sec = (-99);
static double last_jd = 0.0;
double this_jd;
double delta_jd;
static int fix_count = 0;

   // position and time info

   saw_star_time = 1;
   restart_count = 0;
   if(get_nmea_field()) {  // lat
      strupr(nmea_field);
      sscanf(nmea_field, "%d%c%d%c%d%c%c", &dd,&s[0],&mm,&s[0],&ff,&s[0],&dir[0]);
      lat = (double) dd;
      lat += ((double) mm)/60.0;
      lat += ((double) ff)/(60.0*10000.0);
      if(dir[0] == 'S') lat *= (-1.0);
      lat = lat * PI /180.0;
   }

   if(get_nmea_field()) {  // lon
      strupr(nmea_field);
      sscanf(nmea_field, "%d%c%d%c%d%c%c", &dd,&s[0],&mm,&s[0],&ff,&s[0],&dir[0]);
      lon = (double) dd;
      lon += ((double) mm)/60.0;
      lon += ((double) ff)/(60.0*10000.0);
      if(dir[0] == 'W') lon *= (-1.0);
      lon = lon * PI /180.0;
   }

   if(get_nmea_field()) {  // alt
      alt = atof(nmea_field);
   }


   if(get_nmea_field()) { // dd/mm/yyyy
      sscanf(nmea_field, "%d%c%d%c%d", &dd,&s[0],&mm,&s[0],&yy);
   }
   else return;
   if(get_nmea_field()) { // hrs/mins/secs
      sscanf(nmea_field, "%d%c%d%c%d", &hrs,&s[0],&mins,&s[0],&secs);
   }
   else return;

   // the Star 4 occasionally produces skipped time stamps, we try to fix
   // them, by backing this one up by a second
   this_jd = jdate(yy,mm,dd) + jtime(hrs,mins,secs, 0.0);
   delta_jd = this_jd - (last_jd + jtime(0,0,2, 0.0));
if(debug_file) fprintf(debug_file, "thisjd:%f  lastjd:%f  delta:%.12f (%f)\n", this_jd,last_jd, delta_jd, delta_jd*3600.0*24.0);
// if(FIX_STAR_TIME && have_sat_azel && (secs != 35) && (fabs(delta_jd) <= 0.0000001)) {            // time stamp skip detected
   if(FIX_STAR_TIME && (secs != 35) && (fabs(delta_jd) <= 0.0000001)) {            // time stamp skip detected
      this_jd -= jtime(0,0,1, 0.0); // ... back this time up a second
      ++fix_count;

      gregorian(this_jd);
      yy = g_year;
      mm = g_month;
      dd = g_day;
      hrs = g_hours;
      mins = g_minutes;
      secs = g_seconds;
if(debug_file) fprintf(debug_file, "!! time stamp skip fixed! %02d:%02d:%02d\n", hrs,mins,secs);
   }

   last_jd = this_jd;

   if((star_type == NEC_TYPE) && (secs == last_sec)) {
      dup_star_time = 1;
   }
   else if(FIX_STAR_TIME || (last_sec != secs)) {
      last_sec = secs;

      pri_year = yy;
      pri_month = mm;
      pri_day = dd;
      pri_hours = hrs;
      pri_minutes = mins;
      pri_seconds = secs;
      pri_frac = 0.0;

      if(timing_mode == 0) {     // convert utc time to gps time
         utc_to_gps();
      }
      else {
         adjust_rcvr_time(0.0);  // incorporarte possibly negative fractional second into time variables
      }

      update_gps_screen(2233);

      if(pri_seconds == SCPI_STATUS_SECOND) {
         queue_scpi_cmd("INFO_VIS_SAT;",   STAR_VIS_MSG);     // !!! multi-line response
         queue_scpi_cmd("INFO_TRACK_SAT;", STAR_TRACK_MSG);   // !!! multi-line response
      }
   }
}


void parse_star_track()    
{
int prn;

   // tracked sat info

   if(star_line) {
      --star_line;
      get_nmea_field();      // eat the line number
      if(get_nmea_field()) { // sat PRN
         prn = atoi(nmea_field);
         if((prn >= 1) && (prn <= MAX_PRN)) {
            sat[prn].tracking = prn;
         }
         if(strchr(nmea_field, ';')) {  // end of list
            star_line = 0;
            star_msg = 0;
         }
      }
      else {
         star_line = star_msg = 0;
      }
   }
   else if(get_nmea_field()) {
      star_msg = STAR_TRACK_MSG;
      star_line = atoi(nmea_field);
      for(prn=0; prn<=MAX_PRN; prn++) {
         if(sat[prn].visible) sat[prn].tracking = (-1);
         else                 sat[prn].tracking = (0);
      }
      if(star_line == 0) {
         star_msg = 0;
      }
   }
}


void parse_star_vis()  
{
int prn;
int el;
int az;
int snr;
int health;

   // visible sat info

   if(star_line) {
      --star_line;
      get_nmea_field();      // eat the line number
      prn = el = az = snr = health = (-999);

      if(get_nmea_field()) prn = atoi(nmea_field);
      if(get_nmea_field()) el = atoi(nmea_field);
      if(get_nmea_field()) az = atoi(nmea_field);
      if(get_nmea_field()) snr = atoi(nmea_field);
      if(get_nmea_field()) {
         health = atoi(nmea_field);
         if(strchr(nmea_field, ';')) {  // end of list
            star_line = 0;
            star_msg = 0;
         }
      }

if(debug_file) fprintf(debug_file, "  star vis msg:%d  line:%d  prn:%d el:%d az:%d snr:%d health:%d\n", 
star_msg,star_line, prn,el,az,snr,health);

      if((prn >= 1) && (prn <= MAX_PRN)) {
         sat[prn].azimuth = (float) az;
         set_sat_el(prn, (float) el);
         sat[prn].sig_level = (float) snr;
         sat[prn].level_msg = 52;
         sat[prn].visible = 1;
//       if(snr > 0) sat[prn].tracking = prn;
//       else        sat[prn].tracking = (-1);
         sat[prn].health_flag = health;  // 0=no_alm, 1=unhealthy, 2=healthy
         have_sat_azel = 2;

         record_sig_levels(prn);
         level_type = "SNR";
      }
   }
   else if(get_nmea_field()) {
      star_msg = STAR_VIS_MSG;
      star_line = atoi(nmea_field);
if(debug_file) fprintf(debug_file, "star vis msg:%d  line:%d\n", star_msg,star_line);

      sat_count = star_line;
      config_sat_count(sat_count);
      have_count = 40;

      for(prn=0; prn<=MAX_PRN; prn++) { 
         sat[prn].visible = 0;
         sat[prn].level_msg = 0;
      }

      if(star_line == 0) {
         star_line = 0;
         star_msg = 0;
      }
   }
}


void parse_star_state()  
{
   // process the multi-line output state message

   if(star_line) {
      --star_line;
      get_nmea_field();      // eat the line number
      if(get_nmea_field()) {
         // !!!!!! get and process the info
         if(!strcmp(nmea_field, "10M_S")) {      // sinewave output
            if(get_nmea_field()) {
               if(strstr(nmea_field, "OK")) {   
               }
               else {
                  discipline_mode = 4;
               }
               if(strchr(nmea_field, ';')) {  // end of list
                  star_line = 0;
                  star_msg = 0;
               }
            }
         }
         else if(!strcmp(nmea_field, "1PPS")) {  // 1PPS
            pps_enabled = 1;
            if(get_nmea_field()) {
               if(strstr(nmea_field, "OK")) {   
               }
               else {
                  discipline_mode = 4;
               }
               if(strchr(nmea_field, ';')) {  // end of list
                  star_line = 0;
                  star_msg = 0;
               }
            }
         }
         else if(!strcmp(nmea_field, "10M_L")) { // logic level output
            if(get_nmea_field()) {
               if(strstr(nmea_field, "OK")) {   
               }
               else {
                  discipline_mode = 4;
               }
               if(strchr(nmea_field, ';')) {  // end of list
                  star_line = 0;
                  star_msg = 0;
               }
            }
         }
      }
   }
   else if(get_nmea_field()) {
      star_msg = STAR_STATE_MSG;
      star_line = atoi(nmea_field);
   }
}


void parse_star_status() 
{
int led;
int gps;
int mode;

   // unit status

   led = (-1);
   gps = 0;
   mode = 0;

   if(get_nmea_field()) {
      led = atoi(nmea_field);
   }
   if(get_nmea_field()) {
      gps = nmea_field[0];
      if(gps == 'A') {   // GPS alarm
         critical_alarms |= 0x0008;
         have_critical_alarms = 1;
      }
      else if(gps == 'O') {   // GPS ok
         critical_alarms &= (~0x0008);
         have_critical_alarms = 1;
      }
   }
   if(get_nmea_field()) {
      mode = nmea_field[0];
      if     (mode == 'I') discipline_mode = 1;
      else if(mode == 'F') discipline_mode = 5;
      else if(mode == 'H') discipline_mode = 2;
      else if(mode == 'W') discipline_mode = 10;
      else if(mode == 'S') discipline_mode = 4;
      else if(mode == 'T') discipline_mode = 0;
      else                 discipline_mode = mode;
   }
}


void parse_star_perf()
{
   // holdover performance

   if(get_nmea_field()) {
      star_hold_perf = atoi(nmea_field);  // 0=disable  1=enable
      have_star_perf = 1;
   }
}

void parse_star_atdc()
{
   // atdc (automatic temperature and drift compensation) status

   if(get_nmea_field()) {
      star_atdc_on = atoi(nmea_field);  // 0=atdc active  1=atdc in progress
      have_star_atdc |= 0x01;
   }
   if(get_nmea_field()) {
      star_atdc_time = atoi(nmea_field); 
      have_star_atdc |= 0x02;
   }
}


void parse_star_temp()   
{
   // device temperature

   if(get_nmea_field()) {
      temperature = (float) atof(nmea_field);
      have_temperature = 55;
   } 
}


void parse_star_tod()    
{
   // time-of-day output enable/disable (not on NEC GPSDO)

   if(get_nmea_field()) {
      if(strstr(nmea_field, "OK")) ;
      else {
         star_tod = atoi(nmea_field);  // 0=disable  1=enabke
         have_pps_rate = 88;
      }
   }
}


void parse_star_type()   
{
   // module type info

   star_family[0] = star_variant[0] = 0;

   if(get_nmea_field()) {
      nmea_field[12] = 0;
      strcpy(star_family, nmea_field);
   }

   if(get_nmea_field()) {
      nmea_field[12] = 0;
      strcpy(star_variant, nmea_field);
   }
}


void parse_star_inv()    
{
   // hardware version info

   if(get_nmea_field()) {  // a: name of module (12 chars)
      nmea_field[12] = 0;
      if(star_module[0] == 0) need_redraw = 5478;  // just got the inventory
      strcpy(star_module, nmea_field);
      if(strstr(nmea_field, "NEC")) star_type = NEC_TYPE;
      else star_type = OSCILLO_TYPE;
   }

   if(get_nmea_field()) {  // b: article number (6 chars) 
      nmea_field[6] = 0;
      strcpy(star_article, nmea_field);
   }

   if(get_nmea_field()) {  // c: serial number (6 chars) 
      nmea_field[6] = 0;
      strcpy(star_sn, nmea_field);
   }

   if(get_nmea_field()) {  // d: hardware version (2 chars) 
      nmea_field[2] = 0;
      strcpy(star_hw_ver, nmea_field);
   }

   if(get_nmea_field()) {  // e: firmware article (6 chars) 
      nmea_field[6] = 0;
      strcpy(star_fw_article, nmea_field);
   }

   if(get_nmea_field()) {  // f: fw version
      nmea_field[12] = 0;
      strcpy(star_fw, nmea_field);
   }

   if(get_nmea_field()) {  // g: date of test (ddmmyyyy)
      nmea_field[8] = 0;
      strcpy(star_test_date, nmea_field);
   }

   if(get_nmea_field()) {  // h: test system version (4 chars) 
      nmea_field[4] = 0;
      strcpy(star_test_version, nmea_field);
   }

   if(get_nmea_field()) {  // i: osc type (10 chars) 
      nmea_field[10] = 0;
      strcpy(star_osc, nmea_field);
   }

   if(get_nmea_field()) {  // j: fpga version
      nmea_field[12] = 0;
      strcpy(star_fpga, nmea_field);
   }
}


void parse_star_elmask()
{
float el_val;

   // antenna elevation mask

   if(get_nmea_field()) {
      el_val = (float) atof(nmea_field);
      if((el_val >= 0.0) && (el_val <= 90.0)) {
         el_mask = el_val;
         have_el_mask = 1;
      }
   }
}


void parse_star_inp_type() 
{
   // input ref type: GPS or PPS (not on Star-4 ATDC devices)

   if(get_nmea_field()) {
      if(strstr(nmea_field, "GPS")) {
         gpsdo_ref = 0;
         have_gpsdo_ref = 1;
      }
      else if(strstr(nmea_field, "AUX")) {
         gpsdo_ref = 1;
         have_gpsdo_ref = 1;
      }
   }
}


void parse_star_alarm_mask()  
{
}


void parse_star_mode()    
{
   // should never happen
}


void parse_star_ok()    
{
   star_line = 0;
   star_msg = 0;
}


void parse_star_error()    
{
   star_line = 0;
   star_msg = 0;
}



void decode_star_msg()
{
char c;
int i;
static int row = 0;


   start_msg_decode(0);

   nmea_msg[0] = 0;
   for(i=0; i<tsip_wptr; i++) {  // copy message from tsip buffer to nmea buffer
      c = tsip_byte();
      nmea_msg[i] = c;
      nmea_msg[i+1] = 0;
      if(i >= ((int)sizeof(nmea_msg)-12)) break;
   }

   tsip_wptr = tsip_rptr = 0;
   tsip_sync = 0;


   // get the response from the reciver and pass it to (HOPEFULLY) the routine
   // that expects it...

   strupr(nmea_msg);
   nmea_col = 0;     // index of next char to get from nmea buffer

if(debug_file) fprintf(debug_file, "Decode STAR msg %d (m:%d l:%d):[%s]\n", scpi_msg_id, star_msg, star_line, nmea_msg);

   if(star_msg && star_line) {  // getting multi-line response
      if     (star_msg == STAR_VIS_MSG) parse_star_vis();
      else if(star_msg == STAR_TRACK_MSG) parse_star_track();
      else if(star_msg == STAR_STATE_MSG) parse_star_state();
      else {
         star_line = star_msg = 0;
      }
      if(star_line == 0) {
         poll_next_star(1);
      }
      return;
   }
   else {
      star_line = star_msg = 0;
   }

   if(get_nmea_field()) {  // get the STAR message type and parse it
      if     (!strcmp(nmea_field, "ALARM"))            parse_star_alarm();
      else if(!strcmp(nmea_field, "ALARM_MASK"))       parse_star_alarm_mask();
      else if(!strcmp(nmea_field, "CONF"))             parse_star_conf();
      else if(!strcmp(nmea_field, "GPS_TIME"))         parse_star_gps_time();
      else if(!strcmp(nmea_field, "INPUT_TYPE(1)"))    parse_star_inp_type();
      else if(!strcmp(nmea_field, "HBSQ"))             parse_star_hbsq();
      else if(!strcmp(nmea_field, "INFO_GPS"))         parse_star_pos();
      else if(!strcmp(nmea_field, "INV"))              parse_star_inv();
      else if(!strcmp(nmea_field, "STATUS"))           parse_star_status();
      else if(!strcmp(nmea_field, "TEMPERATURE"))      parse_star_temp();
      else if(!strcmp(nmea_field, "TOD_STATE"))        parse_star_tod();
      else if(!strcmp(nmea_field, "TYPE"))             parse_star_type();
      else if(!strcmp(nmea_field, "MASK_ANGLE"))       parse_star_elmask(); 
      else if(!strcmp(nmea_field, "ATDC_STATUS"))      parse_star_atdc(); 
      else if(!strcmp(nmea_field, "HOLD_PERF_STATUS")) parse_star_perf(); 

      else if(!strcmp(nmea_field, "INFO_VIS_SAT")) {    // multi-line response
         parse_star_vis();
         return;
      }
      else if(!strcmp(nmea_field, "INFO_TRACK_SAT"))  { // multi-line response
         parse_star_track();
         return;
      }
      else if(!strcmp(nmea_field, "OUTPUT_STATE"))    { // multi-line response
         parse_star_state();
         return;
      }

      else if(!strcmp(nmea_field, "MODE"))             parse_star_mode();   // !!!!!
      else if(!strcmp(nmea_field, "PARAM_ERROR"))      parse_star_error();  // !!!!!
      else if(!strcmp(nmea_field, "SYNTAX_ERROR"))     parse_star_error();  // !!!!!
      else if(!strcmp(nmea_field, "UNKNOWN_CMD"))      parse_star_error();  // !!!!!
      else if((nmea_field[0] == 'O') && (nmea_field[1] == 'K')) {
         parse_star_ok(); 
      }
   }

   poll_next_star(2);
   return;
}

void get_star_message()
{
u08 c;

   // This routine buffers up an incoming STAR4 message.  When the end of the
   // message is seen, the message is parsed and decoded with decode_star_msg()

   if(SERIAL_DATA_AVAILABLE() == 0) {
      check_com_timer();
      return;
   }
   else {
      reset_com_timer();
   }

   c = get_com_char();
   if(rcv_error) {      // parity/framing/overrun errors
      rcv_error = 0;
//!!!!!qqqq    goto rst_msg;
   }

   if(tsip_sync == 0) {    // syncing to start of message
      tsip_wptr = 0;
      tsip_rptr = 0;
      if((c == 0x0A) || (c == 0x0D)) return;

      tsip_sync = 1;
      get_sync_time();
      goto star_msg;
   }
   else if(tsip_sync == 1) { // '$' has been seen, now build the message
      star_msg:
//    if((c == 0x0D) || (c == 0x0A)) goto star_end;  // message has no checksum
      if(c == 0x0D) return; // message has no checksum
      if(c == 0x0A) goto star_end;  // message has no checksum

      if(tsip_wptr < (MAX_TSIP-1)) {   // add char to the message buffer
         tsip_buf[tsip_wptr++] = c;
         tsip_buf[tsip_wptr] = 0;
      }
      else {
         tsip_error |= 0x8000;
         goto rst_msg;
      }
   }
   else if(tsip_sync == 3) {  // end of message
      star_end:
      decode_star_msg(); // process the message
      tsip_wptr = 0;     // prepare for next message
      tsip_sync = 0;
   }
   else {     // should never happen, prepare for next message
      rst_msg:
      tsip_wptr = 0;
      tsip_sync = 0;
   }
}



//
//
//  Ublox receiver stuff
//
//

void set_ubx_rate(int id, int rate)
{
   // set periodic output rate for a UBX message

   send_ubx_start(0x0601, 3);
   send_byte((id >> 8) & 0xFF);
   send_byte(id & 0xFF);
   send_byte(rate);
   send_ubx_end();
}

void request_ubx_msg(int id)
{
   send_ubx_start(id, 0);
   send_ubx_end();
}


void parse_ubx_nak()
{
int id;

   id = tsip_word();
}

void parse_ubx_ack()
{
int id;

   id = tsip_word();
}

void parse_ubx_nav()
{
S32 msec;

   // get lat/lon/alt

   msec = tsip_dword();
   lon = (double) (S32) tsip_dword();
   lat = (double) (S32) tsip_dword();
   alt = (double) (S32) tsip_dword();
   tsip_dword();
   tsip_dword();
   tsip_dword();

   lat = (lat / 1.0E7);
   lon = (lon / 1.0E7);
   alt = alt / 1000.0;

   lat = lat * PI / 180.0;
   lon = lon * PI / 180.0;
}

void parse_ubx_status()
{
int mode;

   // get receiver operating mode

   tsip_dword();  // tow
   mode = tsip_byte();
   tsip_byte();
   tsip_byte();
   tsip_byte();
   tsip_double();

   if     (mode == 0) rcvr_mode = RCVR_MODE_ACQUIRE;
   else if(mode == 1) rcvr_mode = RCVR_MODE_UNKNOWN;  // dead reckoning
   else if(mode == 2) rcvr_mode = RCVR_MODE_2D;
   else if(mode == 3) rcvr_mode = RCVR_MODE_3D;
   else if(mode == 4) rcvr_mode = RCVR_MODE_3D;       // gps+dead reckonong
   else if(mode == 5) rcvr_mode = RCVR_MODE_HOLD;     // (time only fix)
   else               rcvr_mode = RCVR_MODE_UNKNOWN;
   have_rcvr_mode = 3;

   have_tracking = 1;
   if     (rcvr_mode == RCVR_MODE_NO_SATS) minor_alarms |= 0x0008;
   else if(rcvr_mode == RCVR_MODE_2D_3D)   minor_alarms |= 0x0008;
   else                                    minor_alarms &= (~0x0008);
}

void parse_ubx_clock()
{
S32 bias;
S32 drift;
u32 accu;
u32 freq;

   // get clock bias info  !!!! we need to do something with this

   tsip_dword();  // tow
   bias = tsip_dword();
   drift = tsip_dword();
   accu = tsip_dword();
   freq = tsip_dword();
}

void parse_ubx_dop()
{
   // get DOP info

   tsip_dword();  // tow
   have_dops = 0;
   gdop = ((float) tsip_word()) / 100.0F;   // pdop
   pdop = ((float) tsip_word()) / 100.0F;   // pdop
   tdop = ((float) tsip_word()) / 100.0F;   // tdop
   vdop = ((float) tsip_word()) / 100.0F;   // tdop
   hdop = ((float) tsip_word()) / 100.0F;   // tdop
   ndop = ((float) tsip_word()) / 100.0F;   // tdop
   edop = ((float) tsip_word()) / 100.0F;   // tdop
   have_dops |= (GDOP | PDOP | TDOP | VDOP | NDOP | EDOP);
}

void parse_ubx_speed()
{
   // get course/speed info

   tsip_dword();  // tow
   tsip_dword();  
   tsip_dword();  
   tsip_dword();  
   speed = ((double) tsip_dword()) / 100.0;  // 3d speed
   tsip_dword();   // 2D speed
   heading = ((double) (S32) tsip_dword()) / 1.0E5;
   if(heading < 0.0) heading += 360.0;
   tsip_dword();  // speed accuracy
   tsip_dword();  // course accu

   have_speed = 15;
   have_heading = 15;
}

void parse_ubx_utc()
{
u32 gps_msec;
double accu;

   // get utc time

   gps_msec = tsip_dword();
   pri_tow = tow = this_tow = (gps_msec / 1000);
   if(have_tow == 0) need_redraw = 2014;
   have_tow = 2;
   survey_tow = this_tow;
   accu = (double) tsip_dword();  // time error estimate in nanoseconds

   pri_frac = (double) (int) tsip_dword();  // !!!!!! can be negative
   pri_frac /= 1.0E9;
   raw_frac = pri_frac;

   pri_year = year = tsip_word();
   pri_month = month = tsip_byte();
   pri_day = day = tsip_byte();
   pri_hours = hours = tsip_byte();
   pri_minutes = minutes = tsip_byte();
   pri_seconds = seconds = tsip_byte();
   tsip_byte();  // validity flags

//if(log_file) fprintf(log_file, "  %02d:%02d:%02d.%f  %04d/%02d/%02d  tow:%d\n", 
//pri_hours,pri_minutes,pri_seconds, pri_frac, pri_year,pri_month,pri_day, tow); // zorky

   if(accu == (double) (u32) 0xFFFFFFFF) accu = 0.0;  // filter bogus power-up value
   pps_offset = (accu);
   have_pps_offset = 3;

   osc_offset = pri_frac*1.0E9;
   have_osc_offset = 3;

   time_flags |= 0x0001;  // UTC based time
   have_timing_mode = 1;

   if(timing_mode == 0) {     // convert utc time to gps time
      utc_to_gps();
   }
   else {
      adjust_rcvr_time(0.0);  // incorporarte possibly negative fractional second into time variables
   }

   update_gps_screen(1000);
}

void parse_ubx_gps()
{
int i;
   // get gps-utc offset info

   tsip_dword();
   tsip_dword();
   gps_week = tsip_word();
   i = (int) (s08) tsip_byte();
   if(!user_set_utc_ofs) utc_offset = i;
   tsip_byte();
   tsip_word();

   check_utc_ofs(3);

   if(have_week == 0) need_redraw = 2016;
   have_week = 2;
}

void parse_ubx_leap()
{
int i;
double a0, a1;
int tow;
int wnt;
int ls;
int wnf;
int dn;
int lsf;

   // calculate pending leapsecond info

   tsip_dword();
   a0 = tsip_double();
   a1 = tsip_double();
   tow = tsip_dword();
// pri_tow = this_tow = tow;
// if(have_tow == 0) need_redraw = 2017;
// have_tow = 3;
// survey_tow = tow;

   wnt = tsip_word();
   ls = tsip_word();
   wnf = tsip_word();
   dn = tsip_word();
   lsf = tsip_word();

   minor_alarms &= (~0x0080);
   if(ls != lsf) {
      minor_alarms |= 0x0080;
   }
   have_leap_info = 2;
   calc_leap_days((double) wnf, dn, 11);

//sprintf(debug_text2, "ls:%d lsf:%d  dn:%d wnf:%d wnt:%d  ldays:%d", ls,lsf, dn,wnf,wnt, leap_days);

   for(i=34; i<72; i++) tsip_byte();
}

void parse_ubx_svn()
{
int chans;
int prn;
int flag;
int qual;
int sig;
int az;
int el;

   // get satellite position and signal info

   tsip_dword();  // tow
   chans = tsip_byte();
   have_count = 50;
   tsip_byte();
   tsip_word();

   reset_sat_tracking();

   sat_count = 0;
   while(chans--) {
      tsip_byte();       // channel number
      prn = tsip_byte();
      flag = tsip_byte();
      qual = tsip_byte();
      sig = tsip_byte();
//sig += (sig/2);   // Ublox reports dBc levels that are around 50% less than most receivers
      el = tsip_byte();
      az = tsip_word();
      tsip_dword();      // pseudo range

      if((az == 0.0) || (el == 0.0)) continue;

      if((prn >= 1) && (prn <= MAX_PRN)) {  
         sat[prn].azimuth = (float) az;
         set_sat_el(prn, (float) el);
         sat[prn].sig_level = (float) sig;
         sat[prn].level_msg = 3;
         sat[prn].sv_accuracy = 0.0F;
         have_sat_azel = 3;

         if(flag & 0x01) sat[prn].tracking = 1;
         else sat[prn].tracking = (-1);

         record_sig_levels(prn);
         ++sat_count;
         have_count = 20;
      }
   }

   level_type = "dBc";
   config_sat_count(sat_count); 
}

void parse_ubx_raw()
{
int sats;
double carrier;
double range;
float doppler;
int prn;
int snr;
int lol;

   // get doppler and code phase data (only on special receivers)

   tsip_dword(); // tow
   tsip_word();  // week
   sats = tsip_byte();

   while(sats--) {
      tsip_byte();
      carrier = tsip_double();
      range = tsip_double();
      doppler = tsip_single();
      if(doppler != 0.0F) {
         if(have_doppler == 0) need_redraw = 2018;
         have_doppler = 1;
      }
      prn = tsip_byte();  // !!!! SVN?
      snr = tsip_byte();
      lol = tsip_byte();

      if((prn >= 1) && (prn <= MAX_PRN)) { 
         sat[prn].doppler = doppler;
//       sat[prn].sig_level = (float) snr;
//       sat[prn].level_msg = 4;
         sat[prn].code_phase = (float) carrier;  // !!!! zzzzz
         record_sig_levels(prn);

         if(carrier != 0.0F) {
            if(have_phase == 0) need_redraw = 2019;
            have_phase = 3;
         }
      }
   }
}

void parse_ubx_sfrb()
{
}

void parse_ubx_mon()
{
int i;
u08 ant;
u08 ant_pwr;

   // used to get antenna power monitor info

   for(i=0; i<20; i++) tsip_byte();
   ant = tsip_byte();
   ant_pwr = tsip_byte();
   for(i=22; i<68; i++) tsip_byte(); 

   minor_alarms &= (~0x0006);
   if     (ant == 2);                             // ok
   else if(ant == 3)     minor_alarms |= 0x0004;  // short
   else if(ant == 4)     minor_alarms |= 0x0002;  // open
   else if(ant_pwr == 0) minor_alarms |= 0x0006;  // no power
   have_antenna = 1;
//sprintf(plot_title, "ant state:%02X  pwr:%02X", ant, ant_pwr);
}

void parse_ubx_ant()
{
u16 a1, a2;

   a1 = tsip_word();
   a2 = tsip_word();
//sprintf(debug_text2, "ant cfg:%04X %04X", a1,a2);
}


void set_ubx_config(int mode, double slat,double slon,double salt)
{
   lla_to_ecef(slat, slon, salt);

// sprintf(debug_text2, "X:%.10f  y:%.10f  z:%.10f", ecef_x,ecef_y,ecef_z);
   send_ubx_start(UBX_CFG_TMODE, 28);
   send_dword((u32) mode);
   send_dword((S32) (ecef_x*100.0));
   send_dword((S32) (ecef_y*100.0));
   send_dword((S32) (ecef_z*100.0));
   if(mode == 1) {
      send_dword(50000);       // fixed posn variance
      send_dword(do_survey);   // min survey time
      send_dword(0xFFFFFFFF);  // survey stop required variance (mm^2)
   }                           // ... we set to max value so survey is based only on time
   else {
      send_dword(0);   // fixed posn variance
      send_dword(0);   // min survey time
      send_dword(0);   // survey stop required variance (mm^2)
   }
   send_ubx_end();

   request_ubx_msg(UBX_CFG_TMODE);
}

void parse_ubx_tmode()
{
u32 mode;
u32 slen;

   saw_timing_msg |= 0x02;

   mode = tsip_dword();
   tsip_dword();
   tsip_dword();
   tsip_dword();
   tsip_dword();   // fixed posn variance
   slen = tsip_dword();   // min survey time
   tsip_dword();   // survey stop required variance (mm^2)

   if(mode == 1) {
      minor_alarms |= 0x0020;
      do_survey = (long) slen;
      survey_why = 10;
   }
   else {
      minor_alarms &= (~0x0020);
      if(precision_survey == 0) {
         do_survey = 0;
         survey_why = (-10);
      }
   }
}

void parse_ubx_svin()
{
u32 dur;        // current duration of survey
double ecef_x;
double ecef_y;
double ecef_z;
u32 obs;        // number of observations used
u08 valid;
u08 active;
u32 rsvd;

   saw_timing_msg |= 0x01;

   dur = tsip_dword();  // dur
   ecef_x = (double) (S32) tsip_dword();
   ecef_y = (double) (S32) tsip_dword();
   ecef_z = (double) (S32) tsip_dword();
   ubx_svar = tsip_dword();  // var
   obs = tsip_dword();   // observation count
   valid = tsip_byte();
   active = tsip_byte(); 
   rsvd  = tsip_dword();

   if(do_survey) survey_progress = (int) ((100 * dur) / do_survey);  // !!!! use obs instead of dur?
   else          survey_progress = 0;
//sprintf(plot_title, "obs:%d  dur:%d  svar:%d  progress:%d  act:%d  valid:%d", 
//obs,dur,ubx_svar,survey_progress, active,valid);  // zorky
}

void set_ubx_antenna(u08 mode)
{ 
   send_ubx_start(UBX_CFG_ANT, 4);
   send_word(mode);
   send_word(0x0000);  // dont re-config pins - LEA-5T has a98b
   send_ubx_end();
}

void parse_ubx_navx5()
{
   have_amu = 1;

   tsip_word();      // msg version
   tsip_word(); // update amu mask 
   tsip_dword();
   tsip_byte();      // rsvd
   tsip_byte();      // rsvd
   tsip_byte();      // min sv's
   tsip_byte();      // max sv'ss
   amu_mask = (float) tsip_byte();
   tsip_byte();      // rsvd
   tsip_byte();      // initial fix
   tsip_byte();      // rsvd
   tsip_byte();      // rsvd
   tsip_byte();      // rsvd
   tsip_word();      // rollover week
   tsip_dword();     // rsvd
   tsip_dword();     // rsvd
   tsip_dword();     // rsvd
   tsip_dword();     // rsvd
   tsip_dword();     // rsvd
}

void set_ubx_amu(float amu)
{
   send_ubx_start(UBX_NAVX5, 40);

   send_word(0);      // msg version
   send_word(0x0008); // only update amu mask 
   send_dword(0);
   send_byte(0);      // rsvd
   send_byte(0);      // rsvd
   send_byte(1);      // min sv
   send_byte(16);     // max sv
   send_byte((s08) amu);
   send_byte(0);      // rsvd
   send_byte(0);      // initial fix
   send_byte(0);      // rsvd
   send_byte(0);      // rsvd
   send_byte(0);      // rsvd
   send_word(0);      // rollover week
   send_dword(0);     // rsvd
   send_dword(0);     // rsvd
   send_dword(0);     // rsvd
   send_dword(0);     // rsvd
   send_dword(0);     // rsvd

   send_ubx_end();
}


void parse_ubx_ver()
{
int i;

   // note: docs say the message is 70+ bytes long, we only see 40 bytes
   // on a NEO-6M

   saw_version |= 0x100;

   for(i=0; i<30; i++) {  // sw version
      ubx_sw[i] = tsip_byte();
   }
ubx_sw[21] = 0;  // trim it so info fits on screen
   if(ubx_sw[0]) ubx_fw_ver = (float) atof(ubx_sw);
   else          ubx_fw_ver = (float) atof(ubx_sw);

   for(i=0; i<10; i++) {  // hw vers
      ubx_hw[i] = tsip_byte();
   }
   ubx_hw[9] = 0;

   for(i=0; i<30; i++) {   // rom version (not on NEO-6M)
      ubx_rom[i] = tsip_byte();
   }
   ubx_rom[29] = 0;

// show_version_info();
}

void parse_ubx_datum()
{
u16 datum;

   datum = tsip_word();
   // !!!! ignore the rest of the message for now
}

void parse_ubx_cfg()
{
int fix;  // 1=2D  2=3D  3=2D/3D
int el;
int i;

   tsip_word();
   tsip_byte();
   fix = tsip_byte();
   for(i=4; i<12; i++) tsip_byte();
   el = tsip_byte();
   // ignore bytes 13..35

   el_mask = (float) el;
   have_el_mask = 1;
}

void set_ubx_nav_rate(float rate, int align)
{
   send_ubx_start(UBX_NAV_RATE, 6);
   send_word((u16) rate);
   send_word(1);
   send_word(align);
   send_ubx_end();
}


void parse_ubx_pps()
{
u08 status;
u08 flags;
u08 rsvd;
u32 rate;
u32 width;

   // get timepulse1 info
   saw_ubx_tp = 1;

   rate = tsip_dword();  // interval in usec
   if     (rate == 1000000) pps_rate = 2;     // 1pps
   else if(rate == 10000)   pps_rate = 0x82;  // 100 pps
   else if(rate == 1000)    pps_rate = 0x83;  // 1000 pps
   else                     pps_rate = 0;
   have_pps_rate = 3;
//sprintf(debug_text, "ubxpps: rate:%d", rate);

   width = tsip_dword();  // width in us

   status = tsip_byte();
   if(status == 0) pps_enabled = 0;
   else {
      pps_enabled = 1;
      if(status & 0x80) pps_polarity = 1; // falling
      else              pps_polarity = 0; // rising
      have_pps_polarity = 2;
   }
   have_pps_enable = 2;

   tsip_byte();   // align 0=UTC 1=GPS 2=LOCAL
   flags = tsip_byte();  // 1=always on, 0=when tracking
   rsvd = tsip_byte();   // rsvd

   cable_delay = (double) (s16) tsip_word();
   cable_delay /= 1.0E9;
// if(have_cable_delay == 0) need_redraw = 2020;
   have_cable_delay = 1;

   tsip_word();   // rf group delay
   pps1_delay = (double) (S32) tsip_dword();
   have_pps_delay |= 0x01;

   show_cable_delay();
// sprintf(debug_text2, "UBXPPS: rate:%d  width:%d  ofs:%f  mode:%d flags:%d rcvd:%02X", rate,width, pps1_delay, status,flags,rsvd);
}


void request_ubx_tp5(int chan)
{
   send_ubx_start(UBX_TP5, 1);
   send_byte(chan);
   send_ubx_end();
}

void parse_ubx_tp5()
{
u08 chan;
u32 freq;
u32 lfreq;
u32 duty;
u32 lduty;
S32 dly;
u32 flags;

   chan = tsip_byte();     // output number (0 / 1)
   tsip_byte();            // rsvd
   tsip_word();            // rsvd

   cable_delay = (double) (s16) tsip_word();
   cable_delay /= 1.0E9;
// if(have_cable_delay == 0) need_redraw = 2021;
   have_cable_delay = 1;

   tsip_word();            // signed RF delay(ns)
   freq = tsip_dword();    // freq / period
   lfreq = tsip_dword();   // gps locked freq/period
   duty = tsip_dword();    // width / duty cycle
   lduty = tsip_dword();   // gps locked width / duty cycle
   dly = tsip_dword();     // time pulse delay (ns)
   flags = tsip_dword();   // flags

   if(chan) {  // pps2
      saw_ubx_tp5 |= 2;
      pps2_freq = freq;
      pps2_duty = ((double) duty) / (double) (u32) 0xFFFFFFFF;
      pps2_delay = (double) dly;
      pps2_flags = flags;
      have_pps_duty |= 0x02;
      have_pps_freq |= 0x02;
      have_pps_delay |= 0x02;
   }
   else {  // pps1
      saw_ubx_tp5 |= 1;
      pps1_freq = (double) freq;
      pps1_duty = ((double) duty) / (double) (u32) 0xFFFFFFFF;
      pps1_delay = (double) dly;
      pps1_flags = flags;
      have_pps_duty |= 0x01;
      have_pps_freq |= 0x01;
      have_pps_delay |= 0x01;

      if(pps1_flags & 0x0001) pps_enabled = 1;
      else                    pps_enabled = 0;
      have_pps_enable = 3;

      if(pps1_flags & 0x0040) pps_polarity = 0;
      else                    pps_polarity = 1;
      have_pps_polarity = 3;

      if(pps1_freq == 100) pps_rate = 0x82;
      else if(pps1_freq == 1000) pps_rate = 0x83;
      else if(pps1_freq == 1) pps_rate = 2;
      else pps_rate = 0;  // user defined rate
      have_pps_rate = 4;
//sprintf(debug_text2, "ubxtp5: rate:%d freq:%g duty:%g  flags:%04X  enbl:%d pol:%d", pps_rate, pps1_freq, pps1_duty, pps1_flags, pps_enabled, pps_polarity);
   }

   show_cable_delay();

//if(chan == 0) { // dddd
//   sprintf(plot_title, "f:%d  lf:%d  d:%d  ld:%d  dly:%d  flags:%08X", freq,lfreq,duty,lduty, dly, flags);
//}
//else {
//   sprintf(debug_text, "f:%d  lf:%d  d:%d  ld:%d  dly:%d  flags:%08X", freq,lfreq,duty,lduty, dly, flags);
//}
}


void set_ubx_pps(int chan, u08 pps_rate, u08 pps_enable,  u08 pps_polarity,  double delay, double pps_delay, int save)
{
   if(saw_ubx_tp5) {
      if(pps_enable == 0) {
//sprintf(debug_text2, "pps disable:%02X  tp5:%02X", pps_rate, saw_ubx_tp5); 
         set_pps_freq(chan, 0.0, 0.10, pps_polarity, delay, pps_delay, save);
      }
      else {
         if(pps_rate == 0x82) {  // 100 hz
            set_pps_freq(chan, 100.0, 0.10, pps_polarity, delay, pps_delay, save);
         }
         else if(pps_rate == 0x83) { // 1000 Hz
            set_pps_freq(chan, 1000.0, 0.10, pps_polarity, delay, pps_delay, save);
         }
         else {
            set_pps_freq(chan, 1.0, 0.10, pps_polarity, delay, pps_delay, save);
         }
//sprintf(debug_text2, "pps enable:%02X  tp5:%02X", pps_rate, saw_ubx_tp5);
      }
   }
   else {
      send_ubx_start(UBX_PPS, 20);

      if(pps_rate == 0x82) {
         send_dword(10000);    // 100 pps
         send_dword(100);      // 100 us width
      }
      else if(pps_rate == 0x83) {
         send_dword(1000);     // 1000 pps
         send_dword(500);      // 500 us width
      }
      else {
         send_dword(1000000);  // 1 sec
         send_dword(10000);    // 10 msec width
      }

      if(pps_enable == 0)   send_byte(0);    // pulse off
      else if(pps_polarity) send_byte(0xFF); // neg pulse
      else                  send_byte(1);    // pos pulse

      if(time_flags & 0x0001) send_byte(0);  // align to UTC
      else                    send_byte(1);  // align to GPS

      send_byte(1);  // always on

      send_byte(0);  // rsvd

      send_word((s16) (delay*1.0E9));
      send_word(0);  // rf delay
      send_dword((S32) pps_delay);

      send_ubx_end();
   }


   if(save) save_segment(6, 28);  // save PPS timing config in EEPROM

   if(saw_ubx_tp5) request_ubx_tp5(chan);
   else if(chan == 0) request_ubx_msg(UBX_PPS);
}


void set_pps_freq(int chan, double freq, double duty, int polarity, double cable_delay, double pps_delay, int save)
{
u32 flags;
u32 val;

   if(rcvr_type == NVS_RCVR) {
      if(chan != 0) return;
      if(freq <= 0.0) return;
      freq = 1.0;  // can only do 1pps

      val = nvs_pps_width;
      nvs_pps_width = (u32) ((1.0E9 / freq) * duty);
//sprintf(debug_text, "nvs set width: %d ns", nvs_pps_width);
      set_pps(user_pps_enable, polarity,  cable_delay, pps1_delay,  300.0, 45);
      nvs_pps_width = val;
   }
   else if(rcvr_type == UBX_RCVR) {
      if(saw_ubx_tp5 == 0) return;

      send_ubx_start(UBX_TP5, 32);

      send_byte(chan); // output number (0 / 1)
      send_byte(0);    // rsvd
      send_word(0);    // rsvd
      send_word((s16) (cable_delay*1.0E9));
      send_word(0);    // rf delay

      flags = 0x0001;     // enable PPS
      if(freq <= 0.0) {
         flags = 0x0000;  // disable PPS
         freq = 1.0;
      }

      if(duty <= 0.0) duty = 0.500;
      else if(duty >= 100.0) duty = 0.500;
      val = (u32) (duty * (double) 0xFFFFFFFFU);

      send_dword((u32) freq); // freq / period
      send_dword((u32) freq); // gps locked freq/period

      send_dword(val); // ratio / duty cycle
      send_dword(val); // gps locked ratio / duty cycle

      send_dword((S32) pps_delay);

      flags |= 0x0001;  // !!!! pulse active
      flags |= 0x0002;  // sync to GPS
      flags |= 0x0004;  // !!!! enable alt values if not locked to GPS

      flags |= 0x0008;  // 1=freq mode         0=rate mode
//    flags |= 0x0010;  // 1=pulse width mode  0=duty cycle mode
      flags |= 0x0020;  // !!!! align pulse to TOW
      if(polarity == 0) flags |= 0x0040;   // flag bit: 1=rising edge  0=falling edge
      if((time_flags & 0x0001) == 0) flags |= 0x0080;  // sync to GPS time

// !!!!! bits 8/9/10/11 are what GNSS time system to use !!!!!
//       0=UTC 1=GPS 2=GLONASS 3=BEIDOU 4=GALILEO
// !!!!! bits 12/13/14 are how to sync time

      send_dword(flags);    // flags

      send_ubx_end();

// sprintf(plot_title, "SETPPS %d: %lf %lf flags:%08X", chan, freq, duty, flags); 

      if(save) save_segment(6, 29);

      if(saw_ubx_tp5) request_ubx_tp5(chan);
      else            request_ubx_msg(UBX_PPS);
   }
   else if(rcvr_type == VENUS_RCVR) {
//    if(chan != 0) return;
      if(freq <= 0.0) return;

      send_venus_start(SET_VENUS_FREQ);
      send_byte(3);
      send_dword((u32) freq);
      send_venus_save(1);

      if(duty > 0.0) {
         val = (u32) ((1.0E6 / freq) * duty);
         send_venus_start(SET_VENUS_WIDTH);
         send_byte(1);
         send_dword(val);
         send_venus_save(1);
      }
   }
}

void parse_ubx_tp()
{
u32 tow;
u32 subms;
S32 quant;
u16 week;
u08 flags;
u08 rsvd;

   tow = tsip_dword();
   subms = tsip_dword();
   quant = (S32) tsip_dword();
   week = tsip_word();
   flags = tsip_byte();
   rsvd = tsip_byte();

   have_sawtooth = 1;
   dac_voltage = ((float) quant) / 1000.0F;
}


void parse_ubx_rate()
{
int rate;

   rate = tsip_word();
   tsip_word();
   tsip_word();

   if(rate > 0) {
      if(!pause_data) {
         nav_rate = 1000.0F / (float) rate;
         have_nav_rate = 1;
      }
   }
}

void parse_ubx_gnss()
{
int hw_chans;
int use_chans;
int blocks;
int id;
int min_chans;
int max_chans;
u32 flags;

   tsip_byte();  // msg version
   hw_chans = tsip_byte();
   use_chans = tsip_byte();
   blocks = tsip_byte();

   if(blocks == 0) return;

   gnss_mask = 0x0000;
   have_gnss_mask = 1;
   while(blocks--) {
      id = tsip_byte();
      min_chans = tsip_byte();
      max_chans = tsip_byte();
      tsip_byte();  // reserved
      flags = tsip_dword();

      if(id == 0) {       // GPS
         if(flags & 0x0001) gnss_mask |= GPS;
      }
      else if(id == 1) {  // SBAS
         if(flags & 0x0001) gnss_mask |= SBAS;
      }
      else if(id == 2) {  // GALILEO
         if(flags & 0x0001) gnss_mask |= GALILEO;
      }
      else if(id == 3) {  // Beidou
         if(flags & 0x0001) gnss_mask |= BEIDOU;
      }
      else if(id == 4) {  // IMES
         if(flags & 0x0001) gnss_mask |= IMES;
      }
      else if(id == 5) {  // QZSS
         if(flags & 0x0001) gnss_mask |= QZSS;
      }
      else if(id == 6) {  // GLONASS
         if(flags & 0x0001) gnss_mask |= GLONASS;
      }

//if(debug_file) {
//   fprintf(debug_file, "gnss blk:%d  hw:%d use:%d  id:%d min:%d max:%d flags:%08X  mask:%04X\n", 
//   blocks, hw_chans, use_chans, id,min_chans,max_chans,flags, gnss_mask);
//}
   }
}


void ignore_ubx_msg()
{
}

void decode_ubx_msg()
{
   // Here we decode and parse Ublox UBX format messages

   start_msg_decode(1);

   if     (ubx_msg_id == UBX_NAV)       parse_ubx_nav();
   else if(ubx_msg_id == UBX_STATUS)    parse_ubx_status();
   else if(ubx_msg_id == UBX_CLOCK)     parse_ubx_clock();
   else if(ubx_msg_id == UBX_DOP)       parse_ubx_dop();
   else if(ubx_msg_id == UBX_SPEED)     parse_ubx_speed();
   else if(ubx_msg_id == UBX_UTC)       parse_ubx_utc();
   else if(ubx_msg_id == UBX_GPS)       parse_ubx_gps();
   else if(ubx_msg_id == UBX_SVN)       parse_ubx_svn();
   else if(ubx_msg_id == UBX_RAW)       parse_ubx_raw();
   else if(ubx_msg_id == UBX_SFRB)      parse_ubx_sfrb();
   else if(ubx_msg_id == UBX_PPS)       parse_ubx_pps();
   else if(ubx_msg_id == UBX_TP)        parse_ubx_tp();
   else if(ubx_msg_id == UBX_TP5)       parse_ubx_tp5();
   else if(ubx_msg_id == UBX_GNSS)      parse_ubx_gnss();
   else if(ubx_msg_id == UBX_CFG)       parse_ubx_cfg();
   else if(ubx_msg_id == UBX_MON)       parse_ubx_mon();
   else if(ubx_msg_id == UBX_VER)       parse_ubx_ver();
   else if(ubx_msg_id == UBX_LEAP)      parse_ubx_leap();
   else if(ubx_msg_id == UBX_DATUM)     parse_ubx_datum();
   else if(ubx_msg_id == UBX_NAK)       parse_ubx_nak();
   else if(ubx_msg_id == UBX_ACK)       parse_ubx_ack();
   else if(ubx_msg_id == UBX_CFG_ANT)   parse_ubx_ant();
   else if(ubx_msg_id == UBX_NAVX5)     parse_ubx_navx5();
   else if(ubx_msg_id == UBX_CFG_TMODE) parse_ubx_tmode();
   else if(ubx_msg_id == UBX_NAV_RATE)  parse_ubx_rate();
   else if(ubx_msg_id == UBX_SVIN)      parse_ubx_svin();
   else                                 ignore_ubx_msg();
}


void get_ubx_message()
{
u08 c;

   // This routine buffers up an incoming UBX message.  When the end of the
   // message is seen, the message is parsed and decoded with decode_ubx_msg()

   if(SERIAL_DATA_AVAILABLE() == 0) {
      check_com_timer();
      return;
   }
   else {
      reset_com_timer();
   }

   c = get_com_char();
   if(rcv_error) {      // parity/framing/overrun errors
      rcv_error = 0;
//!!!!!qqqq    goto rst_msg;
   }


   if(tsip_sync == 0) {         // syncing to start of message, search for a 0xB5
      if(c == 0xB5) {
         tsip_sync = 1;
         get_sync_time();
      }
      tsip_wptr = 0;
      tsip_rptr = 0;
      return;
   }
   else if(tsip_sync == 1) { // 0xB5 has been seen, now checking next byte
      if(c != 0x62) goto rst_msg;
      ++tsip_sync;           // 0xB5 0x62 seen... so accumulate the message
      ubx_rxa = ubx_rxb = 0; // init received message checksum
   }
   else if(tsip_sync == 2) {    // get ubx message class
      ubx_rx_class = c;
      ubx_msg_id = (c & 0xFF);
      ubx_rxa += c;
      ubx_rxb += ubx_rxa;
      ++tsip_sync;
   }
   else if(tsip_sync == 3) {    // get ubx message id
      ubx_rx_id = c;
      ubx_msg_id = (ubx_msg_id * 256) + (c & 0xFF);
      ubx_rxa += c;
      ubx_rxb += ubx_rxa;
      ++tsip_sync;
   }
   else if(tsip_sync == 4) {    // low byte of payload length
      ubx_rx_len = c;
      ubx_rxa += c;
      ubx_rxb += ubx_rxa;
      ++tsip_sync;
   }
   else if(tsip_sync == 5) {    // high byte of payload length
      ubx_rx_len += (((u32) c) * 256);
      ubx_msg_len = ubx_rx_len;
      ubx_rxa += c;
      ubx_rxb += ubx_rxa;
      ++tsip_sync;
      if(ubx_rx_len == 0) ++tsip_sync;  // message has no payload
   }
   else if(tsip_sync == 6) {   // get ubx message payload
      if(tsip_wptr < MAX_TSIP) {  // so add it to the message buffer
         tsip_buf[tsip_wptr++] = c;
         ubx_rxa += c;
         ubx_rxb += ubx_rxa;
         --ubx_rx_len;
         if(ubx_rx_len == 0) ++tsip_sync;  // message payload processed, now get checksums
      }
      else {
         tsip_error |= 0x8000;
         goto rst_msg;
      }
   }
   else if(tsip_sync == 7) {  // first byte of message checksum
      ++tsip_sync;
      ubx_msg_rxa = c;
   }
   else if(tsip_sync == 8) {  // last byte of message checksum, end of ubx message
      tsip_sync = 0;
      ubx_msg_rxb = c;
      if((ubx_msg_rxa != ubx_rxa) || (ubx_msg_rxb != ubx_rxb)) { // checksum error
         tsip_error |= 0x8000;
         goto rst_msg;
      }

      decode_ubx_msg();  // message checksum matches, process the message
   }
   else {     // should never happen
      rst_msg:
      tsip_wptr = 0;
      tsip_sync = 0;
      ubx_rxa = ubx_rxb = 0; // init received message checksum
   }
}


//
//
//  Sirf binary protocol stuff
//
//

void send_sirf_start(u16 sirf_cmd, int len)
{
   // start sending a ublox message
   sendout(0xA0);     // send message start code
   sendout(0xA2);

   send_byte((len / 256) & 0xFF);  // send message length
   send_byte((len % 256) & 0xFF); 

   sirf_tx_cksum = 0; // init transmit checksum bytes

   send_byte(sirf_cmd & 0xFF);  // send message id
}


void send_sirf_end()
{
u16 cksum;

   // end the ublox message
   cksum = sirf_tx_cksum;
   cksum &= 0x7FFF;
   sendout(cksum / 256);   // send checksum
   eom_flag = 1;
   sendout((cksum % 256) & 0xFF);

   sendout(0xB0);
   eom_flag = 1;
   sendout(0xB3);

Sleep(100); // rate limit send messages gggg
}

void query_sirf_msg(int msg)
{
   send_sirf_start(msg, 2);
   send_byte(0);
   send_sirf_end();
}


void parse_sirf_sw_ver()
{
int i;
int len;

   sirf_sw_id[0] = 0;
   if(sirf_msg_len > 80) len = 80-2;
   else len = sirf_msg_len-2;

   for(i=0; i<len; i++) {
      sirf_sw_id[i] = tsip_byte();
      sirf_sw_id[i+1] = 0;
   }
}

void parse_sirf_track()
{
int chans;
int prn;
float az,el;
float cno;
u16 state;

   gps_week = tsip_word();
   pri_tow = this_tow = tsip_dword() / 100; 
   chans = tsip_byte();

   reset_sat_tracking();

   sat_count = 0;
   while(chans--) {
      prn = tsip_byte();
      az = ((float) tsip_byte() * 3.0F / 2.0F);
      el = ((float) tsip_byte() / 2.0F);
      state = tsip_word();
      cno  = ((float) tsip_byte());  // cno1
      cno += ((float) tsip_byte());  // cno2
      cno += ((float) tsip_byte());  // cno3
      cno += ((float) tsip_byte());  // cno4
      cno += ((float) tsip_byte());  // cno5
      cno += ((float) tsip_byte());  // cno6
      cno += ((float) tsip_byte());  // cno7
      cno += ((float) tsip_byte());  // cno8
      cno += ((float) tsip_byte());  // cno9
      cno += ((float) tsip_byte());  // cno10
      cno /= 10.0F;

      if((prn >= 1) && (prn <= MAX_PRN)) {
         sat[prn].azimuth = az;
         set_sat_el(prn, el);
         sat[prn].sig_level = cno;
         sat[prn].level_msg = 24;

         have_sat_azel = 4;

         if(state & 0x0001) sat[prn].tracking = prn;
         else               sat[prn].tracking = (-1);

         sat[prn].code_phase = (float) state;
         if(have_phase == 0) need_redraw = 2022;
         have_phase = 4;

         record_sig_levels(prn);
         ++sat_count;
         have_count = 21;
      }
   }
config_sat_count(sat_count);


   if(have_week == 0) need_redraw = 2023;
   have_week = 3;

   if(have_tow == 0) need_redraw = 2024;
   have_tow = 3;

   level_type = "dBc";
}

void parse_sirf_vis()
{
u08 sats;
int prn;
float az, el;

return; // bbbb - this causes sat info to toggle between yellow and green, best to not do it

   reset_sat_tracking();

   sats = tsip_byte();
   while(sats--) {
      prn = tsip_byte();
      az = ((float) tsip_byte() * 3.0F / 2.0F);
      el = ((float) tsip_byte() / 2.0F);

      if((prn >= 1) && (prn <= MAX_PRN)) {
         sat[prn].azimuth = az;
         set_sat_el(prn, el);
         have_sat_azel = 5;
      }
   }
}


void parse_sirf_nav()
{
u16 valid;
u16 nav_type;
u32 sats;
u16 secs;
u32 utc_tow;

   valid = tsip_word();
   nav_type = tsip_word();

   gps_week = tsip_word();
   pri_tow = this_tow = tsip_dword();
   pri_tow /= 1000;  // gggg use msecs?

   pri_year = year = tsip_word();
   pri_month = month = tsip_byte();
   pri_day = day = tsip_byte();
   pri_hours = hours = tsip_byte();
   pri_minutes = minutes = tsip_byte();
   secs = tsip_word();

   pri_frac = raw_frac = ((double) (secs % 1000)) / 1000.0;
   pri_seconds = seconds = (secs / 1000);


   if(have_sirf_pps == 0) {  // fake the UTC offset from tow values since some receivers do not do the 52 message
      utc_tow = pri_hours * 24*60*60;
      utc_tow += pri_minutes * 60;
      utc_tow += pri_seconds;
      utc_tow %= 60;

      if(!user_set_utc_ofs) {
         utc_offset = (pri_tow % 60) - utc_tow;
         if(utc_offset < 0) utc_offset += 60;
         utc_offset %= 60;
         check_utc_ofs(4);
      }
//sprintf(plot_title, "utc_tow:%d  tow:%d  delta:%d  valid:%04X  nav:%04X", utc_tow,pri_tow%60, utc_offset, valid,nav_type);
   }

   time_flags |= 0x0001;   // UTC based time
   have_timing_mode = 1;

   if(timing_mode == 0) {  // convert utc time to gps time
      utc_to_gps();
   }

   sats = tsip_dword();
   lat = ((float) (S32) tsip_dword()) * 1.0E-7 * PI / 180.0;
   lon = ((float) (S32) tsip_dword()) * 1.0E-7 * PI / 180.0;
   alt = ((float) (S32) tsip_dword()) * 1.0E-2;

   tsip_dword(); // msl
   tsip_byte();  // datum
   speed = ((double)tsip_word()) / 100.0;    // speed
   heading = ((double) tsip_word()) / 100.0; // course
   tsip_word();  // mag var
   tsip_word();  // climb
   tsip_word();  // heading rate
   tsip_dword(); // horiz err
   tsip_dword(); // vert err
   tsip_dword(); // time err
   tsip_word();  // horiz velocity err
   tsip_dword(); // clock bias
   tsip_dword(); // bias err
   tsip_dword(); // clock drift
   tsip_dword(); // drift err
   tsip_dword(); // distance
   tsip_word();  // distance err
   tsip_word();  // heading err
   tsip_byte();  // sat count
   hdop = ((float) (u08) tsip_byte()) / 5.0F;
   have_dops |= HDOP;
   tsip_byte();  // info

   have_heading = 1;
   have_speed = 1;

   nav_type &= 0x07;
   if((nav_type == 4) || (nav_type == 6)) rcvr_mode = RCVR_MODE_3D;
   else if((nav_type == 3) || (nav_type == 5)) rcvr_mode = RCVR_MODE_2D;
   else rcvr_mode = RCVR_MODE_ACQUIRE;

   if(have_week == 0) need_redraw = 2026;
   have_week = 4;

   if(have_tow == 0) need_redraw = 2027;
   have_tow = 4;

   update_gps_screen(1001);
}

void parse_sirf_pps()
{
double utc_frac;
u08 status;
int i;

   // WARNING: not all receivers output this message

   have_sirf_pps = 1;

   tsip_byte();  // hh
   tsip_byte();  // mm
   tsip_byte();  // ss
   tsip_byte();  // day
   tsip_byte();  // month
   tsip_word();  // year
   i = (int) (s16) tsip_word();  // utc offset
   utc_frac = ((double) (S32) tsip_dword()) * 1.0E-9;
   status = tsip_byte();  // bit0:valid time  bit1:utc time  bit2:utc offset info valid

   if(!user_set_utc_ofs) {
      if(status & 0x04) {    // UTC offset valid
         utc_offset = i;
         check_utc_ofs(5);
      }
   }

   if(status & 0x01) {  // time is valid
   }

   if(status & 0x02) {  // UTC time 
   }
}

void parse_sirf_clock()
{
u32 drift;
u32 bias;
u32 time;

   tsip_word();  // gps week
   tsip_dword(); // tow
   tsip_byte();  // sats
   drift = tsip_dword(); // clock drift in Hz
   bias = tsip_dword();  // clock bias in nanosecs
   time = tsip_dword();  // GPS time in msecs

   if(drift) pps_offset = 1.0 / (double) drift;  // convert Hz to nanoseconds
   else pps_offset = 0.0;
   pps_offset *= 1.0E9;
   have_pps_offset = 20;

   osc_offset = ((double) bias) / 1.0E6;
   have_osc_offset = 20;
}

void parse_sirf_nav_params()
{
int sub_id;
u08 tsm;
u08 snm;

   sub_id = tsip_byte();

   tsip_byte();  // rsvd
   tsip_byte();
   tsip_byte();

   tsip_byte();  // alt hold mode
   tsip_byte();  // alt hold source
   tsip_word();  // alt source input

   tsip_byte();  // degraded mode
   tsip_byte();  // degraded timeout
   tsip_byte();  // DR timeout
   tsm = tsip_byte();  // track smooth mode
   snm = tsip_byte();  // static navigation
   tsip_byte();  // 3SV least squares
   tsip_dword(); // rsvd
   tsip_byte();  // DOP mask mode

   el_mask = ((float) (s16) tsip_word()) / 10.0F;
   have_el_mask = 1;

   amu_mask = (float) tsip_byte();  // power mask
   have_amu = 1;

   tsip_dword(); // rsvd
   // ... gggg lots more info follows

//sprintf(debug_text2, "tsm:%02X  snm:%02X", tsm,snm);
}

void decode_sirf_msg()
{
u08 id;

   // process the received Sirf message
   start_msg_decode(1);

   id = tsip_byte();
// if(debug_file) fprintf(debug_file, "decode sirf:%02X (%d)  len:%d\n", id,id, sirf_msg_len);

   if     (id == SIRF_TRACK_MSG)     parse_sirf_track();
   if     (id == SIRF_SW_VER_MSG)    parse_sirf_sw_ver();
   else if(id == SIRF_CLOCK_MSG)     parse_sirf_clock();
   else if(id == SIRF_VIS_MSG)       parse_sirf_vis();
   else if(id == SIRF_NAV_PARAM_MSG) parse_sirf_nav_params();
   else if(id == SIRF_NAV_MSG)       parse_sirf_nav();
   else if(id == SIRF_PPS_MSG)       parse_sirf_pps();
}


void get_sirf_message()
{
u08 c;

   // This routine buffers up an incoming UBX message.  When the end of the
   // message is seen, the message is parsed and decoded with parse_ubx_msg()

   if(SERIAL_DATA_AVAILABLE() == 0) {
      check_com_timer();
      return;
   }
   else {
      reset_com_timer();
   }

   c = get_com_char();
   if(rcv_error) {      // parity/framing/overrun errors
      rcv_error = 0;
//!!!!!qqqq    goto rst_msg;
   }


   if(tsip_sync == 0) {      // syncing to start of message, search for a 0xA0
      if(c == 0xA0) {
         tsip_sync = 1;
         get_sync_time();
      }
      tsip_wptr = 0;
      tsip_rptr = 0;
      return;
   }
   else if(tsip_sync == 1) { // 0xA0 has been seen, now checking next byte
      if(c != 0xA2) goto rst_msg;
      ++tsip_sync;           // 0xA2 seen... so accumulate the message
      sirf_rx_cksum = 0;     // init received message checksum
   }
   else if(tsip_sync == 2) { // high byte of payload length
      sirf_rx_len = ((u32) c);
      sirf_msg_len = sirf_rx_len;
      ++tsip_sync;
   }
   else if(tsip_sync == 3) {    // low byte of payload length
      sirf_rx_len *= 256;
      sirf_rx_len += (((u32) c) & 0xFF);
      sirf_msg_len = sirf_rx_len;
      ++tsip_sync;
      if(sirf_rx_len == 0) ++tsip_sync;  // message has no payload
   }
   else if(tsip_sync == 4) {   // get sirf message payload
      if(tsip_wptr < MAX_TSIP) {  // so add it to the message buffer
         sirf_rx_cksum += c;
         tsip_buf[tsip_wptr++] = c;
         --sirf_rx_len;
         if((int) sirf_rx_len <= 0) ++tsip_sync;  // message payload processed, now get checksums
      }
      else {
         tsip_error |= 0x8000;
         goto rst_msg;
      }
   }
   else if(tsip_sync == 5) {  // first (high) byte of message checksum
      sirf_msg_cksum = c;
      ++tsip_sync;
   }
   else if(tsip_sync == 6) {  // last byte of message checksum, end of sirf message
      tsip_sync = 0;
      sirf_msg_cksum = (sirf_msg_cksum * 256) + (((u16) c) & 0xFF);
      sirf_msg_cksum &= 0x7FFF;
      sirf_rx_cksum &= 0x7FFF;

      if(sirf_msg_cksum != sirf_rx_cksum) { // checksum error
         tsip_error |= 0x8000;
         goto rst_msg;
      }
      decode_sirf_msg();  // message checksum matches, process the message
   }
   else {     // should never happen
      rst_msg:
      tsip_wptr = 0;
      tsip_sync = 0;
      sirf_rx_cksum = 0; // init received message checksum
   }
}


//
//
//   Venus receiver stuff
//
//

void send_venus_start(int id, int len)
{
   // send a Venus binary message start header
   if(id == 0x08) return;  // protect against possible brickage (firmware download)

   sendout(0xA0);
   sendout(0xA1);
   send_word(len);
   venus_tx_cksum = 0;
   send_byte(id);
}

void send_venus_end()
{
   venus_tx_cksum &= 0xFF;
   sendout(venus_tx_cksum);
   sendout(0x0D);
   eom_flag = 1;
   sendout(0x0A);
Sleep(20);   // slightly rate limit sent messages
}

void send_venus_save(int allow_eeprom)
{
   if(VENUS_SAVE && allow_eeprom && (no_eeprom_writes == 0)) {
      send_byte(1);  // update SRAM and EEPROM
BEEP(201);  // gggg
      ++ee_write_count;
   }
   else {
      send_byte(0);           // update SRAM
   }
   send_venus_end();
}

void query_venus_msg(int id, int sub_id)
{
   if(sub_id >= 0) {
      send_venus_start(id, 2);
      send_byte(sub_id);
   }
   else {
      send_venus_start(id, 1);
   }
   send_venus_end();
}


void parse_venus_nav()
{
double jd;
u08 fix_mode;

   fix_mode = tsip_byte();  // fix mode
   if(minor_alarms & 0x0020) rcvr_mode = RCVR_MODE_SURVEY;
   else if(venus_hold_mode)  rcvr_mode = RCVR_MODE_HOLD;
   else if(fix_mode == 0)    rcvr_mode = RCVR_MODE_ACQUIRE;
   else if(fix_mode == 1)    rcvr_mode = RCVR_MODE_2D;
   else if(fix_mode == 2)    rcvr_mode = RCVR_MODE_3D;
   else if(fix_mode == 3)    rcvr_mode = RCVR_MODE_3D;
   else                      rcvr_mode = RCVR_MODE_UNKNOWN;

   tsip_byte();  // sat count
   gps_week = tsip_word();  // week
   pri_tow = this_tow = tow = tsip_dword(); // tow

   lat = ((double) (S32) tsip_dword()) * 1.0E-7 * PI / 180.0; // lat
   lon = ((double) (S32) tsip_dword()) * 1.0E-7 * PI / 180.0; // lon
   alt = ((double) (S32) tsip_dword()) / 100.0; // lon
   tsip_dword(); // msl alt

   gdop = ((float) (s16) tsip_word()) / 100.0F;  // GDOP
   pdop = ((float) (s16) tsip_word()) / 100.0F;  // PDOP
   hdop = ((float) (s16) tsip_word()) / 100.0F;  // HDOP
   vdop = ((float) (s16) tsip_word()) / 100.0F;  // VDOP
   tdop = ((float) (s16) tsip_word()) / 100.0F;  // TDOP
   have_dops |= (GDOP | PDOP | HDOP | VDOP | TDOP);

   ecef_x = tsip_dword(); // ecefx
   ecef_y = tsip_dword(); // ecefy
   ecef_z = tsip_dword(); // ecefz
   tsip_dword(); // ecefvx
   tsip_dword(); // ecefvy
   tsip_dword(); // ecefvz

   if(have_tow == 0) need_redraw = 2029;
   have_tow = 5;

   if(have_week == 0) need_redraw = 2030;
   have_week = 5;

   jd = (double) (u32) pri_tow;
   jd /= 100.0;
   jd += 0.01;  // adjust for possible roundoff

   if(timing_mode) {  // we want UTC time - convert gps to utc
      jd -= (double) utc_offset;
   }
   
   jd /= (24.0*60.0*60.0);
   jd += GPS_EPOCH;
   jd += ((double) gps_week) * 7.0;
   gregorian(jd);

   pri_tow /= 100;
   this_tow /= 100;

   pri_year = year = g_year;
   pri_month = month = g_month;
   pri_day = day = g_day;
   pri_hours = hours = g_hours;
   pri_minutes = minutes = g_minutes;
   pri_seconds = seconds = g_seconds;
   pri_frac = raw_frac = g_frac;

   query_venus_msg(QUERY_VENUS_LEAP);  // get UTC offset

   level_type = "dBc";
   update_gps_screen(1002); 
}

void parse_venus_masks()
{
u08 flags;
u08 el;
u08 cnr;

   flags = tsip_byte();
   el = tsip_byte();
   cnr = tsip_byte();

   if((flags == 1) || (flags == 2)) {
      el_mask = (float) el;
      have_el_mask = 1;
   }

   if((flags == 1) || (flags == 3)) {
      amu_mask = (float) cnr;
      have_amu = 1;
   }
}

void parse_venus_cable()
{
   cable_delay = (float) (S32) tsip_dword();
   cable_delay *= 1.0E-11;

// if(have_cable_delay == 0) need_redraw = 2031;
   have_cable_delay = 1;

   show_cable_delay();
}

void parse_venus_survey()
{
u08 mode;
u32 len;
u32 sdev;
double slat,slon,salt;
u08 rtime;
u32 rlen;

   config_venus_timing();
   saw_timing_msg |= 0x04;
   
   mode = tsip_byte();   // timing mode
   len = tsip_dword();   // survey length
   sdev = tsip_dword();  // std dev
   slat = tsip_double(); // saved lat
   slon = tsip_double(); // saved lon
   salt = (double) tsip_single(); // saved alt
   rtime = tsip_byte();  // run-time timing mode
   rlen = tsip_dword();  // run-time survey length

   if(do_survey > 0) survey_progress = 100 - ((100 * rlen) / do_survey);
   else              survey_progress = (-1);
//sprintf(debug_text, "mode:%d len:%d  sdev:%d  lla:%f %f %f   rtime:%d  rlen:%d  prog:%d",
//mode, len, sdev, slat,slon,salt, rtime, rlen, survey_progress);

   minor_alarms &= (~0x0020);
   venus_hold_mode = 0;
   if(rlen == 0) ;  // survey completed
   else if(rtime == 0) { // PVT mode
      rcvr_mode = RCVR_MODE_3D;
   }
   else if(rtime == 1) {  // surveying
      minor_alarms |= (0x0020);
      rcvr_mode = RCVR_MODE_SURVEY;
   }
   else if(rtime == 2) {  // position hold
      venus_hold_mode = 1;
      rcvr_mode = RCVR_MODE_HOLD;
   }
}

void parse_venus_pps()
{
u08 sub_code;
u32 freq;
u32 width;

pps_enabled = pps_mode = 1; // zzzzz
   have_pps_enable = 4;
   have_pps_mode = 4;
   sub_code = tsip_byte();
   freq = width = 0;

   if(sub_code == 0x80) {   // width (us) 1..100000us
      width = tsip_dword();
      if(saw_timing_msg) {
         pps2_duty = 0.50;
         have_pps_duty |= 0x02;
      }
      else {
         if(pps1_freq) {
            pps1_duty = (((double) (width)) / 1.0E6) /  (1.0 / pps1_freq);
            have_pps_duty |= 0x01;
         }
      }
   }
   else if(sub_code == 0x81) {  // freq (not all firmware does this)
      freq = tsip_dword();
      if(saw_timing_msg) {
         pps2_freq = (double) freq;
         have_pps_freq |= 0x02;
      }
      else {
//!!!!   pps1_freq = (double) freq;
         pps1_freq = 1.0;        // only does 1PPS, can't set the freq
         have_pps_freq |= 0x01;
      }
   }
//sprintf(debug_text, "freq %d:%d  width %d:%d  duty:%f", have_pps_freq,freq,  have_pps_duty,width,pps1_duty); // qqqqqqqq
}


void parse_venus_sbas()
{
u08 sub_code;
u08 flag;
u08 qzss_chans;
u08 sbas_mode;
u08 sbas_ura;
u08 sbas_correct;
u08 sbas_chans;
u08 sbas_mask;

   // message type 0x62 responses

   sub_code = tsip_byte();
   flag = tsip_byte();
   if(sub_code == 0x80) {  // SBAS status
      if(flag) gnss_mask |= SBAS;
      else     gnss_mask &= (~SBAS);
      sbas_mode = tsip_byte();
      sbas_ura = tsip_byte();
      sbas_correct = tsip_byte();
      sbas_chans = tsip_byte();
      sbas_mask = tsip_byte();
//sprintf(plot_title, "sbas flag:%02X mode:%02X ura:%02X correct:%02X chans:%d mask:%d",
//flag, sbas_mode, sbas_ura, sbas_correct, sbas_chans, sbas_mask);
   }
   else if(sub_code == 0x81) {  // QZSS status
      if(flag) gnss_mask |= QZSS;
      else     gnss_mask &= (~QZSS);

      qzss_chans = tsip_byte();
//sprintf(debug_text, "qzss flag:%02X chans:%d", flag,qzss_chans);
   }
}

void parse_venus_status()
{
u08 sub_code;
int def_ofs;
int cur_ofs;
int flags;
u08 jam_status;
u32 tow;
u08 flash_status;
u08 flash_type;

   // message type 0x64 responses

   sub_code = tsip_byte();
   if(sub_code == 0x80) {  // boot status
      flash_status = tsip_byte();
      flash_type = tsip_byte();
      if(flash_status != 0) {
         critical_alarms |= 0x0001;
      }
      else { 
         critical_alarms &= (~0x0001);
      }
      if(!have_critical_alarms) need_redraw = 7702;
      have_critical_alarms = 2;
   }
   else if(sub_code == 0x83) {  // jamming mode
      foliage_mode = tsip_byte();
      jam_status = tsip_byte();
   }
   else if(sub_code == 0x8C) {  // GNSS system selection mask
      gnss_mask = tsip_word();
      have_gnss_mask = 1;
//sprintf(plot_title, "gnss mask:%04X", gnss_mask); // ggggg
   }
   else if(sub_code == 0x8E) {  // utc offset
      tow = tsip_dword(); // tow in msecs
      tsip_dword();       // tow fraction
      tsip_word();        // week
      def_ofs = (int) (s08) tsip_byte();  // gps offset default
      cur_ofs = (int) (s08) tsip_byte();  // gps offset current
      flags = tsip_byte();

      if(!user_set_utc_ofs && (flags & 0x04)) {
         utc_offset = cur_ofs;
         check_utc_ofs(6);
      }
   }
}

void parse_venus_version()
{
u08 sw_type;
u32 kernel;
u32 odm;
u32 rev;

   sw_type = tsip_byte();
   kernel = tsip_dword();
   odm = tsip_dword();
   rev = tsip_dword();

   sprintf(venus_kern, "%04d.%02d.%02d", (kernel>>16), (kernel>>8)&0xFF, kernel&0xFF);
   sprintf(venus_odm,  "Ver:  %04d.%02d.%02d", (odm>>16), (odm>>8)&0xFF, odm&0xFF);
   sprintf(venus_rev,  "Rev:  %04d.%02d.%02d", (rev>>16), (rev>>8)&0xFF, rev&0xFF);
//sprintf(plot_title, "k:%08X  odm:%08X  rev:%08X", kernel,odm,rev);
}

void parse_venus_ack()
{
u08 ack;

   ack = tsip_byte();
}

void parse_venus_nak()
{
u08 nak;

   nak = tsip_byte();
}

void parse_venus_rate()
{
float rate;

   rate = (float) tsip_byte();
   if(!pause_data) {
      nav_rate = rate;
      have_nav_rate = 2;
   }
}


void decode_venus_msg()
{
u08 id;

   // process the received Venus message
   start_msg_decode(1);

   id = tsip_byte();
// if(debug_file) fprintf(debug_file, "decode venus:%02X (%d)  len:%d\n", id,id, venus_msg_len);

   if     (id == VENUS_NAV_MSG)      parse_venus_nav();
   else if(id == VENUS_MASK_MSG)     parse_venus_masks();
   else if(id == VENUS_CABLE_MSG)    parse_venus_cable();
   else if(id == VENUS_SURVEY_MSG)   parse_venus_survey();
   else if(id == VENUS_PPS_MSG)      parse_venus_pps();
   else if(id == VENUS_BOOT_MSG)     parse_venus_status();
   else if(id == VENUS_TIME_MSG)     parse_venus_status();
   else if(id == VENUS_JAMMING_MSG)  parse_venus_status();
   else if(id == VENUS_GNSS_MSG)     parse_venus_sbas();
   else if(id == VENUS_SBAS_MSG)     parse_venus_sbas();
   else if(id == VENUS_QZSS_MSG)     parse_venus_status();
   else if(id == VENUS_VERSION_MSG)  parse_venus_version();
   else if(id == VENUS_ACK_MSG)      parse_venus_ack();
   else if(id == VENUS_NAK_MSG)      parse_venus_nak();
   else if(id == VENUS_RATE_MSG)     parse_venus_rate();
}


void get_venus_message()
{
u08 c;

   // This routine buffers up an incoming Venus message.  When the end of the
   // message is seen, the message is parsed and decoded with decode_venus_msg()

if(venus_nmea) {   // vvvvv - mixed NMEA and Venus binary messages
   get_nmea_message();
   return;
}

   if(SERIAL_DATA_AVAILABLE() == 0) {
      check_com_timer();
      return;
   }
   else {
      reset_com_timer();
   }

   c = get_com_char();
   if(rcv_error) {      // parity/framing/overrun errors
      rcv_error = 0;
//!!!!!qqqq    goto rst_msg;
   }


   if(tsip_sync == 0) {         // syncing to start of message, search for a 0xA1
      if(c == 0xA0) {
         tsip_sync = 1;
         get_sync_time();
         venus_nmea = 0;
      }
else if(c == '$') {  // vvvvvv - mixed Venus binary and NMEA messages
   tsip_sync = 1;
   get_sync_time();
   venus_nmea = 1;
}
      nmea_vfy_cksum = 0;  // the calculated message checksum
      nmea_msg_cksum = 0;  // the checksum in the message
      tsip_wptr = 0;
      tsip_rptr = 0;
      return;
   }
   else if(tsip_sync == 1) { // 0xA1 has been seen, now checking next byte
      if(c != 0xA1) goto rst_msg;
      ++tsip_sync;           // 0xA0 0xA1 seen... so accumulate the message
      venus_vfy_cksum = 0;
   }
   else if(tsip_sync == 2) {    // payload len high byte
      venus_rx_len = ((u32) c);
      venus_msg_len = venus_rx_len;
      ++tsip_sync;
   }
   else if(tsip_sync == 3) {    // payload len low byte
      venus_rx_len *= 256;
      venus_rx_len += (((u32) c) & 0xFF);
      venus_msg_len = venus_rx_len;
      ++tsip_sync;
      if(venus_rx_len == 0) ++tsip_sync;
   }
   else if(tsip_sync == 4) {   // get Venus message payload
      if(tsip_wptr < MAX_TSIP) {  // so add it to the message buffer
         venus_vfy_cksum ^= c;
         tsip_buf[tsip_wptr++] = c;
         --venus_rx_len;
         if((int) venus_rx_len <= 0) ++tsip_sync;  // message payload processed, now get checksums
      }
      else {
         tsip_error |= 0x8000;
         goto rst_msg;
      }
   }
   else if(tsip_sync == 5) {  // message checksum, end of venus message
      tsip_sync = 0;
venus_msg_id = tsip_buf[0];
// fprintf(debug_file, "venus msg %02X  len:%02X  vchk:%02X  c=%02X\n", venus_msg_id, venus_msg_len, venus_vfy_cksum, c);
      venus_vfy_cksum ^= c;
      if(venus_vfy_cksum) { // checksum error
         tsip_error |= 0x8000;
         goto rst_msg;
      }

      decode_venus_msg();  // message checksum matches, process the message
   }
   else {     // should never happen
      rst_msg:
      tsip_wptr = 0;
      tsip_sync = 0;
   }
}


//
//
//  Zodiac receiver binary message stuff
//
//

void send_zod_start(u16 id, u16 len, u16 flags)
{
u16 cksum;

   if(id == ZOD_FW_UPDATE) return;  // we don't allow this - might brick things

   len -= 6;
   cksum = (0x81FF + id + len + flags) & 0xFFFF;
   if(cksum != 0x8000) cksum = (0 - cksum) & 0xFFFF;

   zod_seq_num = (zod_seq_num + 1) & 0x7FFF;

   send_word(0x81FF);
   send_word(id);
   send_word(len);
   send_word(flags);
   send_word(cksum);

   zodiac_tx_cksum = 0;
   zodiac_tx_cnt = 0;
}

void send_zod_end()
{
   if(zodiac_tx_cksum != 0x8000) zodiac_tx_cksum = (0 - zodiac_tx_cksum) & 0xFFFF;

   sendout(zodiac_tx_cksum % 256);
   eom_flag = 1;
   sendout(zodiac_tx_cksum / 256);
Sleep(50);
}

void send_zod_seq()
{
   send_word(zod_seq_num);
}

void query_zod_msg(int msg)
{
   send_zod_start(msg, 6, 0x0800);
   send_zod_end();
}

void get_zod_seq(int sat_meas)
{
int ts;
int seq;
int sat_seq;

   ts = tsip_dword();  // timestamp
   seq = tsip_word();  // seq
   if(sat_meas) {      // satellite measurement sequence number
      sat_seq = tsip_word();
   }
}

void parse_zod_temp()
{
u16 t0;
s16 tref;
s16 tfilt;
s16 s0;
float dt;

   tfilt = 0;     // !!!! current temp sensor reading - but no messages
                  //      return it... thus, this code is USELESS !!!!

   get_zod_seq(0);

   tsip_word();

   tsip_word();
   tsip_word();
   tsip_word();
   tsip_word();
   tsip_word();

   tsip_word();
   tsip_word();

   tref = tsip_word();
   t0 = tsip_word();
   s0 = tsip_word();

   tsip_word();
   tsip_word();

   dt = ((float) s0) / (65536.0F*100.0F); // s0 = deg/count
   dt *= ((float) (tfilt-t0));             // t0 = counts, tfilt=current sensor reading
   temperature = (((float) tref) * 0.01F) - dt;  // !!!! but no way to get tfilt !!!!
   have_temperature = 3;
}


void parse_zod_snr()
{
int i;
int flags;
int prn;
int snr;

   get_zod_seq(1);
   
   gps_week = tsip_word();
   if(have_week == 0) need_redraw = 2033;
   have_week = 6;

   pri_tow = tow = this_tow = tsip_dword();
   if(have_tow == 0) need_redraw = 2034;
   have_tow = 6;
   survey_tow = tow;

   tsip_dword(); // epoch nanoseconds

   reset_sat_tracking();

   for(i=0; i<12; i++) {
      flags = tsip_word();
      prn = tsip_word();
      snr = tsip_word();
      if((prn >= 1) && (prn <= MAX_PRN)) { 
         sat[prn].sig_level = (float) snr;
         sat[prn].level_msg = 9;
         if(flags & 0x0001) sat[prn].tracking = 1;
         else               sat[prn].tracking = (-1);
         record_sig_levels(prn);
      }
   }

   level_type = "SNR";
}

void parse_zod_vis()
{
int i;
int prn;
double a, e;

   get_zod_seq(0);

   gdop = ((float) tsip_word()) / 100.0F;
   pdop = ((float) tsip_word()) / 100.0F;
   hdop = ((float) tsip_word()) / 100.0F;
   vdop = ((float) tsip_word()) / 100.0F;
   tdop = ((float) tsip_word()) / 100.0F;
   have_dops = (GDOP | PDOP | HDOP | VDOP | TDOP);

   i = tsip_word();
   sat_count = i;
   have_count = 4;
   config_sat_count(i);

   if(i > 12) i = 12;
   else if(i < 0) i = 0;
   while(i--) {
      prn = tsip_word();
      a = (((double) (s16) tsip_word()) * 1.0E-4 * 180.0 / PI);
      if(a < 0.0) a += 360.0;
      e = (((double) (s16) tsip_word()) * 1.0E-4 * 180.0 / PI);

      if((prn >= 1) && (prn <= MAX_PRN)) { 
         sat[prn].azimuth = (float) a;
         set_sat_el(prn, (float) e);
         have_sat_azel = 6;
      }
   }
}


void parse_zod_mark()
{
double secs;
int flags;

   get_zod_seq(0);
   tsip_word();  // rsvd
   tsip_word(); 
   tsip_word(); 
   tsip_word(); 
   tsip_word(); 

   pri_tow = tow = this_tow = tsip_dword();
   if(have_tow == 0) need_redraw = 2035;
   have_tow = 7;
   survey_tow = tow;

   secs  = (double) tsip_word();
   secs += ((double) tsip_dword()) / 1.0E9;;
   if(!user_set_utc_ofs) {
      utc_offset = (int) (secs + 0.50);
      check_utc_ofs(7);
   }

   flags = tsip_word();

   minor_alarms &= (~0x1000);
//sprintf(debug_text2, "mark flags:%04X", flags);
   if((flags & 0x0001) == 0) minor_alarms |= 0x1000;  // time mark invalid
   if(flags & 0x0008) minor_alarms |= 0x1000;  // traim alarm

}

void parse_zod_traim_config()
{
S32 val;
u16 mode;
S32 slat;
S32 slon;
S32 salt;
u16 flag;

   if(minor_alarms & 0x0020) use_traim_lla = 1;
   else if(rcvr_mode == RCVR_MODE_HOLD) use_traim_lla = 0;
   else if(1) use_traim_lla = 0;
   else if(precision_survey) use_traim_lla = 2;
   else                      use_traim_lla = 3;
//use_traim_lla = 0;

   get_zod_seq(0);
   mode = tsip_word(); // rcvr mode
   tsip_word();        // power up in posn hold mode

   val = tsip_dword();
   cable_delay = ((float) val) / 1.0E9;
// if(have_cable_delay == 0) need_redraw = 2037;
   have_cable_delay = 1;

   survey_length = tsip_word();  // survey hours

   slat = tsip_dword();
   slon = tsip_dword();
   salt = tsip_dword();

   tsip_word();  // align to UTC

   tsip_word();  // traim alarm due to any time fault

   flag = tsip_word();  // traim disabled
   traim_threshold = tsip_word()*50;
   if(flag) {
      traim_mode = 0;
      traim_threshold = 0;
   }
   else traim_mode = 1;
   have_traim = 1;

   tsip_dword();  // reserved

   if(use_traim_lla) {
      lat = ((double) (S32) slat) * 1.0E-8;
      lon = ((double) (S32) slon) * 1.0E-8;
      alt = ((double) (S32) salt) * 1.0E-2;
//sprintf(debug_text2, "lla from traim");
   }

   show_cable_delay();
//sprintf(debug_text, "zod config %d: mode %d  traim threshold: %d  slat:%d", use_traim_lla, mode, traim_threshold, slat);
}

void parse_zod_traim_status()
{
u16 mode;
u16 survey;
u32 surv_count;
u32 surv_dur;
u16 hold_status;
u16 pps_status;
u16 traim_alarm;
u16 traim_disable;
u16 t_status;
u32 excluded;

   get_zod_seq(0);

   mode = tsip_word();         //0=survey  1=posn hold   2=nav
   survey = tsip_word(); 
   surv_count = tsip_dword(); 
   surv_dur = tsip_dword(); 
   hold_status = tsip_word(); 
   pps_status = tsip_word(); 
   traim_alarm = tsip_word(); 

   traim_disable = tsip_word();
   t_status = tsip_word();
   excluded = tsip_dword();

   pps_offset = (((double) tsip_dword()) * 0.10);
   have_pps_offset = 4;

   minor_alarms &= (~0x0020);
   if(mode == 0) {  // survey mode
      minor_alarms |= 0x0020;
      rcvr_mode = RCVR_MODE_SURVEY;
   }
   else if(mode == 1) {  // position hold  zorky
      rcvr_mode = RCVR_MODE_HOLD;
   }
   else if(mode == 2) {  // standard nav
      rcvr_mode = RCVR_MODE_3D;
   }
   else rcvr_mode = RCVR_MODE_UNKNOWN; 

   if(pps_status & 0x02) {  // gps time
      time_flags &= (~0x0001);  // GPS based time 
      timing_mode = 0x00;
      have_timing_mode = 1;
   }
   else {    // utc time
      time_flags |= 0x0001;     // UTC based time
      timing_mode = 0x03;
      have_timing_mode = 1;
   }

   if(t_status == 3) traim_status = 3;
   else              traim_status = 0;
   // !!!! note pps status bit 1 is traim alarm flag - the documentation says
   //      1 is traim error... this appears to be wrong... 0=error !!!!
//sprintf(plot_title, "rcvr mode:%d  zod mode:%d  traim_disable:%d  t_status:%d  pps_status:%04X  tflags:%02X  exclude:%08X", rcvr_mode, mode,traim_disable,traim_status,pps_status, time_flags, excluded);
}

void parse_zod_posn()
{
u16 flag1;
u16 type;
u16 used;
u16 flag2;
S32 val;

   pps_enabled = pps_mode = 1;
   have_pps_enable = 5;
   have_pps_mode = 5;

   get_zod_seq(1);

   flag1 = tsip_word();
   type = tsip_word();
   used = tsip_word();
   flag2 = tsip_word();

   gps_week = tsip_word();
   if(have_week == 0) need_redraw = 2038;
   have_week = 7;

   pri_tow = tow = this_tow = tsip_dword();
   if(have_tow == 0) need_redraw = 2039;
   have_tow = 7;
   survey_tow = tow;
   tsip_dword();  // tow nsecs

   pri_day = day = tsip_word();
   pri_month = month = tsip_word();
   pri_year = year = tsip_word();
   pri_hours = hours = tsip_word();
   pri_minutes = minutes = tsip_word();
   pri_seconds = seconds = tsip_word();
   pri_frac = raw_frac = ((double) tsip_dword()) * 1.0E-9;


   if(timing_mode == 0) {  // convert utc time to gps time
      utc_to_gps();
   }

   if(use_traim_lla == 0) {
      lat = ((double) (S32) tsip_dword()) * 1.0E-8;
      lon = ((double) (S32) tsip_dword()) * 1.0E-8;
      alt = ((double) (S32) tsip_dword()) * 1.0E-2;
   }
   else {
      tsip_dword();
      tsip_dword();
      tsip_dword();
   }

   tsip_word();   // geoid separation
   speed = ((double) tsip_dword()) / 100.0;  // speed
   heading = (((double)tsip_word()) / 1000.0) * 180.0 / PI; // course
   tsip_word();   // mvar 
   tsip_word();   // climb
   tsip_word();   // datum
   tsip_dword();  // horiz err
   tsip_dword();  // vert err
   tsip_dword();  // time err
   tsip_word();   // hv err
   val = tsip_dword();  // clock bias in meters
   tsip_dword();  // bias sdev
   tsip_dword();  // clock drift
   tsip_dword();  // clock drift sdev

   have_heading = 5;
   have_speed = 5;

   osc_offset = ((double) val) * 1.0E-2;  // clock bias (in meters)
   have_osc_offset = 4;
osc_offset /= 299792458.0;  // convert meters to secs
osc_offset *= 1.0E9;        // convert secs to ns

// query_zod_msg(ZOD_TEMP_MSG);  // this does no good, current temp sensor reading is not sent by any message
   query_zod_msg(ZOD_TRAIM_CONFIG_MSG);  // to get dynamic lat/lon/alt values 
   update_gps_screen(1003);
}

void parse_zod_ee_written()
{
u16 flags;

   get_zod_seq(0);
   flags = tsip_word();
   BEEP(203); // gggg
   ++ee_write_count;
//sprintf(debug_text, "ee_write:%04X  count:%d", flags,ee_write_count); 
}

void parse_zod_ee_status()
{
u16 flags;
u32 alm_fail;
u32 fail;
u32 alm_status;
u32 status;

   get_zod_seq(0);
   flags = tsip_word();
   alm_fail = tsip_dword();
   fail = tsip_dword();
   alm_status = tsip_dword();
   status = tsip_dword();

   if(flags & 0x01) have_eeprom = 0;
   else             have_eeprom = 1;
}

void parse_zod_power()
{
u16 flags;

   get_zod_seq(0);
   flags = tsip_word();
}

void parse_zod_id()
{
int i;

   get_zod_seq(0);

   for(i=0; i<20; i++) {
      zod_chans[i] = tsip_byte();
      zod_chans[i+1] = 0;
   }

   for(i=0; i<20; i++) {
      zod_sw[i] = tsip_byte();
      zod_sw[i+1] = 0;
   }

   for(i=0; i<20; i++) {
      zod_date[i] = tsip_byte();
      zod_date[i+1] = 0;
   }

   for(i=0; i<20; i++) {
      zod_opt[i] = tsip_byte();
      zod_opt[i+1] = 0;

   }

   for(i=0; i<20; i++) {
      zod_rsvd[i] = tsip_byte();
      zod_rsvd[i+1] = 0;
   }

// show_version_info();
}

void parse_zod_settings()
{
u16 snr_threshold;
u16 elev;
u16 platform;

   get_zod_seq(0);

   zod_nav_flags = tsip_word();
   snr_threshold = ((zod_nav_flags >> 9) & 0x7F);

   tsip_word();        // col start timeout
   tsip_word();        // dgps timeout

   elev = tsip_word();
   el_mask = (float) elev;
   el_mask += 0.005F;
   el_mask = el_mask * (float) ((180.0/PI) / 1000.0);
   have_el_mask = 1;

   sats_enabled = tsip_dword();  // selected sats bit mask
   tsip_word();        // solution flags
   tsip_word();        // required sat count
   tsip_dword();       // required horiz err
   tsip_dword();       // required vert err
   platform = tsip_word();

   update_disable_list(sats_enabled);
}

void parse_zod_diags()
{
int ram;
int rom;
int ee;
int dp_ram;
int dsp;
int rtc;

   get_zod_seq(0);
   rom = tsip_word();
   ram = tsip_word();
   ee  = tsip_word();
   dp_ram = tsip_word();
   dsp = tsip_word();
   rtc = tsip_word();

   tsip_word();   // serial port stats
   tsip_word();
   tsip_word();
   tsip_word();

   critical_alarms = 0;
   if(have_eeprom == 0) ee = 0;
   minor_alarms &= (~0x0400);

   if(rom) critical_alarms |= 0x0001;  // ROM
   if(ram | dp_ram) critical_alarms |= 0x0002;  // RAM
   if(rtc) critical_alarms |= 0x0010;  // 1KHz / DCXO
   if(dsp) critical_alarms |= 0x0008;  // FPGA
   if(ee) {
      minor_alarms |= 0x0400;      // EEPROM
      have_eeprom = 1;
   }
   if(!have_critical_alarms) need_redraw = 7703;
   have_critical_alarms = 3;
}


void set_zod_sat_mask(u32 val)
{
   send_zod_start(ZOD_SET_SAT_MASK, 10, 0);
   send_zod_seq();
   send_dword(val);
   send_word(0);     // don't write to EEPROM
   send_zod_end();

   query_zod_msg(ZOD_SETTINGS_MSG);
}


void set_zod_nav_mode(int flags)
{
   send_zod_start(ZOD_SET_NAV_MODE, 15, 0);
   send_zod_seq();
   send_word(flags);
   send_word(0);
   send_word(0);
   send_word(0);
   send_word(0);
   send_word(0);
   send_word(0);
   send_word(0);
   send_zod_end();
}

void set_zod_protocol(int prot)
{
   if(prot < 0) return;
   if(prot > 1) return;

   send_zod_start(ZOD_SET_PROTOCOL, 9, 0);
   send_zod_seq();
   send_word(prot);
   send_zod_end();

   Sleep(250);
   if     (prot == 0) rcvr_type = ZODIAC_RCVR;
   else if(prot == 1) rcvr_type = NMEA_RCVR;
   config_rcvr_type(0);
   init_messages(60);
}

void set_zod_power(int power)
{
   send_zod_start(ZOD_SET_POWER, 8, 0);
   send_zod_seq();
   send_word(power);
   send_zod_end();
   Sleep(100);

   query_zod_msg(ZOD_POWER_MSG);
}

void set_zod_platform(int platform)
{
   send_zod_start(ZOD_SET_PLATFORM, 8, 0);
   send_zod_seq();
   send_word(platform);
   send_zod_end();
   Sleep(100);

   query_zod_msg(ZOD_SETTINGS_MSG);
}

void set_zod_config(int mode, double slat,double slon,double salt, u32 traim, int why)
{
S32 val;
u16 pps_suppress;

//sprintf(debug_text2, "set zod config: %d", why);

   send_zod_start(ZOD_SET_CONFIG, 24, 0);
   send_zod_seq();
   send_word(mode);
   send_word(1);     // power up in posn hold mode
   send_dword((S32) (cable_delay*1.0E9));
   send_word((u16) do_survey);  // hours

   val = (S32) (slat * 1.0E8);    // lat lon alt
   send_dword(val);
   val = (S32) (slon * 1.0E8);
   send_dword(val);
   val = (S32) (salt * 1.0E2);
   send_dword(val);

   pps_suppress = 0;  // !!!! zorky set to 1 to suppress PPS pulses if traim error

   if(time_flags & 0x0001) send_word(0x0000 | pps_suppress);  // align to UTC
   else                    send_word(0x0002 | pps_suppress);  // align to GPS

   send_word(0x0007);  // traim alarm due to any time fault

   if(traim == 0) send_word(1);  // traim disabled
   else           send_word(0);  // traim enabled
   send_word(traim/50);

   send_dword(0);  // reserved

   send_zod_end();

Sleep(200);  // zorky
query_zod_msg(ZOD_TRAIM_CONFIG_MSG);
}


void decode_zod_message()
{
u16 id;

   id = zodiac_hdr[0];

   if     (id == ZOD_POSN_MSG)         parse_zod_posn();
   else if(id == ZOD_SNR_MSG)          parse_zod_snr();
   else if(id == ZOD_VIS_MSG)          parse_zod_vis();
   else if(id == ZOD_TRAIM_STATUS_MSG) parse_zod_traim_status();
   else if(id == ZOD_TRAIM_CONFIG_MSG) parse_zod_traim_config();
   else if(id == ZOD_DIAG_MSG)         parse_zod_diags();
   else if(id == ZOD_MARK_MSG)         parse_zod_mark();
   else if(id == ZOD_TEMP_MSG)         parse_zod_temp();
   else if(id == ZOD_EE_WRITE_MSG)     parse_zod_ee_written();
   else if(id == ZOD_EE_STATUS_MSG)    parse_zod_ee_status();
   else if(id == ZOD_POWER_MSG)        parse_zod_power();
   else if(id == ZOD_SETTINGS_MSG)     parse_zod_settings();
   else if(id == ZOD_ID_MSG)           parse_zod_id();
// else sprintf(debug_text, "unk zod msg:%d", id);
}

void get_zod_message()
{
static int hdr_ptr = 0;
u08 c;
static u16 cksum;
static u16 byte_count;
static int row=3;

   // This routine buffers up an incoming Venus message.  When the end of the
   // message is seen, the message is parsed and decoded with parse_zodiac_msg()

   if(SERIAL_DATA_AVAILABLE() == 0) {
      check_com_timer();
      return;
   }
   else {
      reset_com_timer();
   }

   c = get_com_char();
   if(rcv_error) {      // parity/framing/overrun errors
      rcv_error = 0;
   }


//if(tsip_sync == 0) fprintf(debug_file, "\n");
//if(debug_file) fprintf(debug_file, "%02X ", c);

   if(tsip_sync == 0) {         // syncing to start of message, search for a 0xFF
      if(c == 0xFF) {
         tsip_sync = 1;
         get_sync_time();
      }

      tsip_wptr = 0;
      tsip_rptr = 0;
      zodiac_rx_len = 0;
      zodiac_rx_cksum = 0;
      cksum = 0;
      byte_count = 0;
      zodiac_vfy_cksum = 0;
      hdr_ptr = 0;
      zodiac_hdr[0] = zodiac_hdr[1] = zodiac_hdr[2] = zodiac_hdr[3] = 0;
      return;
   }
   else if(tsip_sync == 1) { // 0xFF has been seen, now checking next byte
      if(c != 0x81) goto rst_msg;
      ++tsip_sync;           // 0xFF 0x81 seen... so get the message header
   }
   else if(tsip_sync == 2) { // header packet
      if(hdr_ptr >= 8) goto rst_msg;   // should never happen

      if(hdr_ptr & 1) {  // high byte
         zodiac_hdr[hdr_ptr>>1] += (((u16) c) * 256);
         zodiac_hdr[hdr_ptr>>1] &= 0xFFFF;
      }
      else {  // low byte
         zodiac_hdr[hdr_ptr>>1] = ((u16) c) & 0xFF;
      }

      if(++hdr_ptr >= 8) {  // end of header
         cksum = (0x81FF + zodiac_hdr[0] + zodiac_hdr[1] + zodiac_hdr[2]) & 0xFFFF;   
         if(cksum != 0x8000) cksum = (0 - cksum) & 0xFFFF;
         if(cksum != zodiac_hdr[3]) goto rst_msg;  // bad checksum;
         zodiac_rx_len = (zodiac_hdr[1]*2);
         if(zodiac_rx_len == 0) goto got_it;  // message has no payload, go parse it
         else {
            ++tsip_sync;
            cksum = 0;
//if(debug_file) fprintf(debug_file, "  [%d]  ", byte_count);
         }
      }
   }
   else if(tsip_sync == 3) {   // get Zodiac message payload
      if(tsip_wptr < MAX_TSIP) {  // so add it to the message buffer
         tsip_buf[tsip_wptr++] = c;
         if(byte_count & 1) zodiac_rx_cksum += ((((u16) c) * 256) + cksum);
         else               cksum = (u16) c;    // low byte of word
         ++byte_count;
         --zodiac_rx_len;
         if(zodiac_rx_len == 0) ++tsip_sync;  // message payload processed, now get checksums
      }
      else {
         tsip_error |= 0x8000;
         goto rst_msg;
      }
   }
   else if(tsip_sync == 4) {  // get message checksum low byte
      zodiac_vfy_cksum = (u16) c;   // low byte of message checksum
      ++tsip_sync;
   }
   else if(tsip_sync == 5) {  // message checksum, end of venus message
      zodiac_vfy_cksum += ((u16) c) * 256;

      if(zodiac_rx_cksum != 0x8000) zodiac_rx_cksum = (0 - zodiac_rx_cksum) & 0xFFFF;
//sprintf(out, "id:%-4d  len:%-4d  flags:%04X  vfy:%04X  rx:%04X  bc:%-4d  dif:%04X", 
//zodiac_hdr[0], zodiac_hdr[1], zodiac_hdr[2], zodiac_vfy_cksum, zodiac_rx_cksum, byte_count, (zodiac_rx_cksum-zodiac_vfy_cksum)&0xFFFF);
//vidstr(row,3, YELLOW, out);
//if(row++>30) row = 0;
//refresh_page();
//if(debug_file) fprintf(debug_file, "  (v:%04X calc:%04X)\n", zodiac_vfy_cksum, zodiac_rx_cksum);
      if(zodiac_rx_cksum != zodiac_vfy_cksum) { // checksum error
         tsip_error |= 0x8000;
         goto rst_msg;
      }

      got_it:
      tsip_sync = 0;
      decode_zod_message();  // message checksum matches, process the message
   }
   else {     // should never happen
      rst_msg:
      tsip_wptr = 0;
      tsip_sync = 0;
   }
}




//
//
//  Motorola receiver binary message stuff
//
//


void send_moto_start(char *s)
{
   // send a Motorola binary message start header
   if(s == 0) return;

   sendout('@');
   sendout('@');
   moto_tx_cksum = 0;
   send_byte(s[0]);
   send_byte(s[1]);
}

void send_moto_end()
{
   // send Motorola binary message checksum and end-of-packet
   sendout(moto_tx_cksum);
   sendout(0x0D);
   eom_flag = 1;
   sendout(0x0A);
Sleep(50);
}

void request_moto_leap(int rate)
{
   send_moto_start("Gj");
   send_moto_end();

   send_moto_start("Bj");
   send_byte(rate);
   send_moto_end();
}


void get_moto_lla()
{
   // get lat/lon/alt fields from a motorola binary message

   lat = ((double) (S32) tsip_dword());  // filtered or unfiltered posn
   lon = ((double) (S32) tsip_dword());
   alt = ((double) (S32) tsip_dword());  // gps altitude

   lat = (lat / 3600000.0) * PI / 180.0; // milliarcseconds to radians
   lon = (lon / 3600000.0) * PI / 180.0;
   alt /= 100.0;  // convert cm to meters
}

void get_moto_time(int set_time)
{
   if(set_time) {
      pri_month = month = tsip_byte();
      pri_day = day = tsip_byte();
      pri_year = year = tsip_word();
      pri_hours = hours = tsip_byte();
      pri_minutes = minutes = tsip_byte();
      pri_seconds = seconds = tsip_byte();
      pri_frac = raw_frac = 0.0;
   }
   else {
      tsip_byte();
      tsip_byte();
      tsip_word();
      tsip_byte();
      tsip_byte();
      tsip_byte();
   }
}

void get_moto_tz()
{
   tsip_byte();  // local time zone offset
   tsip_byte();  // hours
   tsip_byte();  // minutes
}


void set_survey_status(int flag)
{
   if(flag) minor_alarms |= 0x0020;    
   else     minor_alarms &= (~0x0020);
}

void decode_posn_mode()
{
   // convert position mode word into tbolt equivalents

   set_survey_status(0);

   if     (posn_mode == 0) rcvr_mode = RCVR_MODE_3D;
   else if(posn_mode == 1) rcvr_mode = RCVR_MODE_HOLD;
   else if(posn_mode == 2) rcvr_mode = RCVR_MODE_2D;
   else if(posn_mode == 3) set_survey_status(1);
   have_rcvr_mode = 4;
}


void decode_moto_status(int r_status)
{
int ant;
int mode;

   // convert status word into tbolt equivalents

   // Bit fields:
   //    0x0006 = antenna status
   //    0x0008 = no sats
   //    0x0010 = survey mode
   //    0x0020 = posn lock
   //    0x0040 = differential mode
   //    0x0080 = cold start
   //    0x0100 - unfiltered GPS
   //    0x0200 - fast acquisition mode
   //    0x0400 - narrow band filter
   //    0xE000 = rcvr mode
   //             7=3d  6=2d  5=propogeate 4=posn hold  3=acquiring 2=bad geom

   ant = (r_status >> 1) & 0x03;

   minor_alarms &= (~0x0006);  // antenna state
   if     (ant == 3) minor_alarms |= 0x0006;  // unbiased
   else if(ant == 2) minor_alarms |= 0x0002;  // open
   else if(ant == 1) minor_alarms |= 0x0004;  // short
   have_antenna = 1;

   if(r_status & 0x0008) {
      minor_alarms |= 0x0008;    // no sats
      gps_status = 8;
   }
   else { 
      minor_alarms &= (~0x0008);
      gps_status = 0;
   }
   have_gps_status = 1;
   have_tracking = 1;

   set_survey_status(r_status & 0x0010);

   if(r_status & 0x0080) minor_alarms |= 0x0800;   // cold start/no almanac
   else                  minor_alarms &= (~0x0800);
   have_almanac = 1;

   //  mode: 7=3d  6=2d  5=propogate 4=posn hold  3=acquiring 2=bad geom
   mode = (r_status >> 13) & 0x07;  

   if(mode == 3) discipline_mode = 4;
   else          discipline_mode = 0;

   if     (mode == 7) rcvr_mode = RCVR_MODE_3D;
   else if(mode == 6) rcvr_mode = RCVR_MODE_2D;
   else if(mode == 5) rcvr_mode = RCVR_MODE_PROP;
   else if(mode == 4) rcvr_mode = RCVR_MODE_HOLD;
   else if(mode == 3) rcvr_mode = RCVR_MODE_ACQUIRE;
   else if(mode == 2) rcvr_mode = RCVR_MODE_BAD_GEOM;
   else               rcvr_mode = RCVR_MODE_UNKNOWN;
   have_rcvr_mode = 5;
}


int get_moto_status()
{
int r_status;

   r_status = tsip_word();
   decode_moto_status(r_status);
   return r_status;
}


void get_moto_tag()
{
   id_tag[0] = tsip_byte();  // id string
   id_tag[1] = tsip_byte();
   id_tag[2] = tsip_byte();
   id_tag[3] = tsip_byte();
   id_tag[4] = tsip_byte();
   id_tag[5] = tsip_byte();
   id_tag[6] = 0;
}


void parse_moto_Ad()
{
   lat = ((double) (S32) tsip_dword());  // lat
   lat = (lat / 3600000.0) * PI / 180.0; // milliarcseconds to radians
}

void parse_moto_Ae()
{
   lon = ((double) (S32) tsip_dword());  // lon
   lon = (lon / 3600000.0) * PI / 180.0; // milliarcseconds to radians
}

void parse_moto_Af()
{
   alt = ((double) (S32) tsip_dword());  // GPS height in cm
   tsip_dword();                         // MSL height in cm
   alt = (alt / 100.0); // cm to meters
}

void parse_moto_Ag()
{
   // elevation mask
   el_mask = tsip_byte();
   have_el_mask = 1;
}

void parse_moto_Am()
{
u32 val;

   tsip_byte();
   val = tsip_dword();  // sat ignore bitmask

   val = (~val);
   sats_enabled = val;
   update_disable_list(sats_enabled);

}

void parse_moto_AN()
{
u08 v_filter;

   // marine velocity filter
   v_filter = tsip_byte();
   marine_filter = v_filter;
   if((v_filter >= 10) && (v_filter <= 100)) have_filter |= MARINE_FILTER;
   else                                      have_filter &= (~MARINE_FILTER);
}

void parse_moto_AP()
{
   // PPS rate (1 / 100 PPS)
   pps_rate = tsip_byte();
   have_pps_rate = 5;
}

void parse_moto_Aq()
{
u08 iono;

   // ionosphere corrections
   iono = tsip_byte();  // 0x01=iono, 0x02:tropo
   if(iono & 0x01) static_filter = 1;
   else            static_filter = 0;
   have_filter |= STATIC_FILTER;

   // troposphere corrections
   if(iono & 0x02) alt_filter = 1;
   else            alt_filter = 0;
   have_filter |= ALT_FILTER;
//sprintf(plot_title, "iono:%02x", iono);
}

void parse_moto_AQ()
{
u08 v_filter;

   // position filter
   v_filter = tsip_byte();
   pv_filter = v_filter;
   have_filter |= PV_FILTER;
//sprintf(debug_text, "Pfilter:%02x", pv_filter);
}

void parse_moto_Ar()
{
u08 fix_type;

   // fix type
   fix_type = tsip_byte();  // 0=best 4, 1=all in view
//sprintf(debug_text2, "fix type:%02x", fix_type);
}

void parse_moto_At()
{
   // position hold status
   posn_mode = tsip_byte();  // 0=3D, 1=posn hold
   decode_posn_mode();
}

void parse_moto_Ay()
{
   // pps delay
   pps1_delay = (double) (S32) tsip_dword();
   have_pps_delay |= 0x01;
   show_cable_delay();
}

void parse_moto_Aw()
{
u08 x;

   // GPS (0) UTC (1) select
   x = tsip_byte();  
   if(x == 0) {
      time_flags &= (~0x0001);
      timing_mode = 0x00;
      have_timing_mode = 1;
   }
   else {
      time_flags |= (0x0001);
      timing_mode = 0x03;
      have_timing_mode = 1;
   }
}

void parse_moto_Az()
{
S32 val;

   // cable delay
   val = tsip_dword();
   cable_delay = ((double) val) / 1.0E9;
// if(have_cable_delay == 0) need_redraw = 2040;
   have_cable_delay = 1;
   show_cable_delay();
}


void parse_moto_Wb()
{
   // message used to put Jupiter-T into Zodiac mode
//BEEP(204); //gggggg
}

void parse_moto_Bb()
{
int i;
int nsats;
int prn;
s16 doppler;
int el;
int az;
int health;

   nsats = tsip_byte();
   for(i=0; i<12; i++) {
      prn     = tsip_byte();
      doppler = tsip_word();
      el      = tsip_byte();
      az      = tsip_word();
      health  = tsip_byte();

      if((prn >= 1) && (prn <= MAX_PRN)) { 
         sat[prn].azimuth = (float) az;
         set_sat_el(prn, (float) el);
         sat[prn].doppler = (float) doppler;
         sat[prn].health_flag = health;  // 0=healthy  1=unhealthy
         have_sat_azel = 7;
         if(sat[prn].doppler != 0.0F) {
            if(have_doppler == 0) need_redraw = 2041;
            have_doppler = 1;
         }
      }
   }
}

void parse_moto_Bd()
{
u08 valid;
u08 week;
u08 time;
u08 sv_count;
u32 sv_mask;

   // almanac status

   valid = tsip_byte();
   week = tsip_byte();
   time = tsip_byte();
   sv_count = tsip_byte();
   sv_mask = tsip_dword();

   tsip_byte();  // reserved bytes
   tsip_byte();
   tsip_byte();
   tsip_byte();
   tsip_byte();
   tsip_byte();
   tsip_byte();
   tsip_byte();

   if(valid) minor_alarms &= (~0x0800);
   else      minor_alarms |= 0x0800;
   have_almanac = 1;
}

void parse_moto_Bj()
{
int leap;

   // get leapscond pending info
   if(have_moto_Gj) return;  // M12 messes this up, always reports no leap

   leap = tsip_byte();
   if(leap) {
      minor_alarms |= 0x0080;
   }
   else { 
      minor_alarms &= (~0x0080);
   }
   have_leap_info = 3;
}

void parse_moto_Bo()
{
int i;

   // gps/utc time offset
   i = (int) (s08) tsip_byte(); 

   if(!user_set_utc_ofs) {
      utc_offset = i;
      check_utc_ofs(8);
   }
}


void parse_moto_Ca()
{
u16 status;

   status = tsip_word();     // 6 channel self test

   critical_alarms = 0;
   minor_alarms &= (~0x0400);

   if(status & 0x0280) critical_alarms |= 0x0001;  // ROM
   if(status & 0x0500) critical_alarms |= 0x0002;  // RAM
   if(status & 0x1040) critical_alarms |= 0x0010;  // 1KHz / DCXO
   if(status & 0xE03F) critical_alarms |= 0x0008;  // FPGA
   if(status & 0x0800) minor_alarms    |= 0x0400;  // EEPROM
   have_eeprom = 1;
   if(!have_critical_alarms) need_redraw = 7704;
   have_critical_alarms = 4;

   init_messages(24);
}

void parse_moto_Cg()
{
   moto_active = tsip_byte();     // 0=idle 1=active
}


                                    
void parse_moto_Cj()
{
int c;
int i;

   // Motorola receiver id message:

   // <cr><lf>
   // COPYRIGHT 1991-199X MOTOROLA INC.<cr><lf> 
   // SFTW P/N # XXXXXXXXXXXXXXX<cr><lf> 
   // SOFTWARE VER # XXXXXXXXXXX<cr><lf> 
   // SOFTWARE REV # XXXXXXXXXXX<cr><lf> 
   // SOFTWARE DATE  XXXXXXXXXXX<cr><lf> 
   // MODEL #    XXXXXXXXXXXXXXX<cr><lf> 
   // HDWR P/N # XXXXXXXXXXXXXXX<cr><lf> 
   // SERIAL #   XXXXXXXXXXXXXXX<cr><lf> 
   // MANUFACTUR DATE XXXXXXXXXX<cr><lf> 
   // OPTIONS LIST    XXXXXXXXXX<cr><lf> 

   i = 0;
   moto_id_lines = 0;
   moto_id[moto_id_lines][0] = 0;

   while(moto_msg_len > 0) {  // get the lines from the device id message
      c = tsip_byte();

      if(c == 0x00) ;
      else if(c == 0x0D) ;
      else if(c == 0x0A) {
         ++moto_id_lines;
         if(moto_id_lines >= MAX_ID_LINES) break;
         i = 0;
         moto_id[moto_id_lines][0] = 0;
      }
      else if(i <= 128) {
         moto_id[moto_id_lines][i++] = c;
         moto_id[moto_id_lines][i] = 0;
      }
      --moto_msg_len;
   }
   if(1 || moto_id_lines) saw_version |= 0x1000;

// show_version_info();

if(debug_file) {
   for(i=0; i<moto_id_lines; i++) fprintf(debug_file, "[%s]\n", &moto_id[i][0]);
}
}

void parse_moto_Ea(int num_chans)
{
int i;
int prn;
int mode;
int sig;
int s1;
int s2;
int dop;
S32 frac;

   if(num_chans != moto_chans) {
      moto_chans = num_chans;
      config_msg_ofs();
   }

   get_moto_time(1);   //+0
   frac = (S32) tsip_dword();    //+7 !!!! fractional time in nsec 
   pri_frac = raw_frac = ((double) frac) / 1.0E9;

   get_moto_lla();     //+11
   tsip_dword();       //+23 msl altitude

   speed = ((double) tsip_word()) / 100.0;   //+27 3d speed
   heading = ((double) tsip_word() / 10.0);  //+29 heading

   have_speed = 6;
   have_heading = 6;

   dop = tsip_word();  //+30
   i = tsip_byte();    //+32

   have_dops = 0;
   if(i) {
      hdop = ((float) dop) / 10.0F;
      have_dops |= HDOP;
   }
   else {
      pdop = ((float) dop) / 10.0F;
      have_dops |= PDOP;
   }

   sat_count = tsip_byte();    //+33 visible
   have_count = 5;
   track_count = tsip_byte();  //+34 tracked

   reset_sat_tracking();

   sat_count = 0;
   for(i=0; i<num_chans; i++) {  //+35
      prn = tsip_byte();
      mode = tsip_byte();
      sig = tsip_byte();
      s1 = tsip_byte();
      if((prn >= 1) && (prn <= MAX_PRN)) { 
         sat[prn].sig_level = (float) sig;
         sat[prn].level_msg = 12;
         sat[prn].sv_accuracy = 0.0F;
         sat[prn].code_phase = (float) mode;
         if(mode == 8) sat[prn].tracking = 1;
         else          sat[prn].tracking = (-1);

         if(sat[prn].code_phase != 0.0F) {
            if(have_phase == 0) need_redraw = 2043;
            have_phase = 2;
         }

         record_sig_levels(prn);
         ++sat_count;
         have_count = 22;
      }
   }
   config_sat_count(sat_count);

   s2 = tsip_byte();

   level_type = "SNR";
   have_temperature = 0;
   plot[TEMP].show_plot = 0;

   request_moto_leap(0);
   update_gps_screen(1003);
}

void parse_moto_Ek(int num_chans)
{
u16 dop;
int i;
int bias;
int osc;

   if(num_chans != moto_chans) {
      moto_chans = num_chans;
      config_msg_ofs();
   }

   have_dops = 0;
   dop = tsip_word();
   gdop = ((float) dop) / 10.0F;
   dop = tsip_word();
   pdop = ((float) dop) / 10.0F;
   dop = tsip_word();
   hdop = ((float) dop) / 10.0F;
   dop = tsip_word();
   vdop = ((float) dop) / 10.0F;
   dop = tsip_word();
   tdop = ((float) dop) / 10.0F;
   have_dops |= (GDOP | PDOP | HDOP | VDOP | TDOP);

   tsip_word();  // mag variation
   tsip_word();  // north velocity
   tsip_word();  // east velocity
   tsip_word();  // up velocity

   tsip_word();  // differential age
   tsip_dword(); // ecef coords
   tsip_dword();
   tsip_dword();   

   for(i=0; i<num_chans; i++) {
      tsip_byte();  // r
   }

   for(i=0; i<10; i++) {
      tsip_word();  // a1 elements
   }

   bias = (int) (s16) tsip_word();  // clock bias
   osc_offset = bias;  // !!!! zzzzz
   have_osc_offset = 5;
   osc = tsip_word();  // osc offset
}

void parse_moto_En(int num_chans)
{
int i;
S32 sawtooth;
s16 accu;

   if(num_chans != moto_chans) {
      moto_chans = num_chans;
      config_msg_ofs();
   }

   tsip_byte();  // output rate
   traim_mode = tsip_byte();
   have_traim = 1;
   traim_threshold = tsip_word();
   pps_enabled = pps_mode = tsip_byte();  // 0=off, 1=always on,  2=on when tracking, 3=on when traim met
   have_pps_enable = 6;
   have_pps_mode = 6;

   tsip_byte();  // pps rate   // !!!!! have_pps_rate
   tsip_byte();
   tsip_byte();

   tsip_byte();  // next pps time
   tsip_byte();
   tsip_word();
   tsip_byte();
   tsip_byte();
   tsip_byte();

   tsip_byte();  // pps status (=0off, 1=enabled)
   tsip_byte();  // 0=sync to UTC  1=sync to GPS
   traim_status = tsip_byte();
   tsip_byte();

   accu = tsip_word();     // time accuracy
   sawtooth = tsip_byte(); // sawtooth
   if(sawtooth & 0x80) sawtooth |= (~(S32)0xFF);
   have_sawtooth = 1;

   dac_voltage = (float) sawtooth;
   pps_offset = (double) accu;
   have_pps_offset = 5;

   for(i=0; i<num_chans; i++) {
      tsip_byte();  // prn
      tsip_dword(); // time fraction
   }
//sprintf(debug_text2, "En: thresh:%d tmode:%d tstat:%d  pps:%d  saw:%d  rate:%d   ", 
//traim_threshold,traim_mode,traim_status,pps_mode,sawtooth,pps_rate);
}

void parse_moto_Eq()
{
   // ascii position message
}

void parse_moto_Fa()
{
u16 status;

   status = tsip_word();     // 8 channel self test

   critical_alarms = 0;
   minor_alarms &= (~0x0400);

   if(status & 0x0600) critical_alarms |= 0x0001;  // ROM
   if(status & 0x1800) critical_alarms |= 0x0002;  // RAM
   if(status & 0x4100) critical_alarms |= 0x0010;  // 1KHz / DCXO
   if(status & 0x80FF) critical_alarms |= 0x0008;  // FPGA
   if(status & 0x2000) minor_alarms    |= 0x0400;  // EEPROM
   have_eeprom = 1;
   if(!have_critical_alarms) need_redraw = 7705;
   have_critical_alarms = 5;

   init_messages(25);
}

void parse_moto_Ga()
{
   get_moto_lla();
   tsip_byte();     // height type
}

void parse_moto_Gb()
{
   get_moto_time(1);
   get_moto_tz();
}

void parse_moto_Gc()
{
   pps_enabled = pps_mode = tsip_byte();  // 0=off, 1=always on,  2=on when tracking, 3=on when traim met
   have_pps_enable = 7;
   have_pps_mode = 7;
}

void parse_moto_Gd()
{
   posn_mode = tsip_byte();  // 0=3D, 1=posn hold,  2=2D, 3=timing auto survey
   decode_posn_mode();
}

void parse_moto_Ge()
{
   traim_mode = tsip_byte();  // 0=disabled  1=enabled
}

void parse_moto_Gf()
{
   traim_threshold = tsip_word();  // in 100nS increments
}


void parse_moto_Gj()
{
u08 present, future;
u16 leap_year;
u08 leap_month;
u16 leap_day;
u08 leap_hour;
u08 leap_min;
u08 leap_sec;
double jd;

   // leapsecond info
   present = tsip_byte();
   future = tsip_byte();
   leap_year = tsip_word();
   leap_month = tsip_byte();
   leap_day = tsip_byte();

   tsip_byte();
   tsip_dword();

   leap_hour = tsip_byte();
   leap_min = tsip_byte();
   leap_sec = tsip_byte(); 

   if(present != future) {    // leapsecond pending
      minor_alarms |= (0x0080);
      jd = jdate(leap_year, leap_month, leap_day) - 1.0;  // -1 since moto says 2017/1/1
      jd_leap = jd + jtime(23,59,59,0.0);
      have_jd_leap = 3;
      calc_jd_leap(3);

//      jd -= jdate(year, month, day);  // days until leap occurs
//      leap_days = ((int) jd);
//      have_leap_days = 0;
//      if(leap_days >= LEAP_THRESH) leap_days = (-1);
//      else have_leap_days = 2;
//sprintf(plot_title, "pres:%d fut:%d %04d/%02d/%02d   days:%d", present,future,leap_year,leap_month,leap_day, leap_days);
   }
   else {
      minor_alarms &= (~0x0080);
   }

   have_moto_Gj = 1;
   have_leap_info = 4;
}

void parse_moto_Gk()
{
   get_moto_tag();
}


void parse_moto_Ha(int long_msg)
{
int i;
int prn;
int mode;
int sig;
int iode;
int status;
int r_status;
int dop;
int bias;
S32 ofs;
int utc;
int acc;
float t;
S32 frac;

//max_sats = 12;
//ebolt = 1;
   if(moto_chans != 12) {
      moto_chans = 12;
      config_msg_ofs();
   }

   get_moto_time(1);
   frac = (S32) tsip_dword();  // !!!! fractional time in nsec 
   pri_frac = raw_frac = ((double) frac) / 1.0E9;

   get_moto_lla();
   tsip_dword();  // msl altitude

   if(long_msg) {
      tsip_dword();  // unfiltered posn
      tsip_dword();
      tsip_dword();
      tsip_dword();  // msl altitude
   }

   speed = ((double) tsip_word()) / 100.0;   // 3d speed
   tsip_word();                              // 2d speed
   heading = ((double) tsip_word()) / 10.0;  // heading

   have_heading = 2;
   have_speed = 2;

   dop = tsip_word();
   have_dops = 0;

   sat_count = tsip_byte();    // visible
   track_count = tsip_byte();  // tracked
   have_count = 6;

   if(long_msg) {
      reset_sat_tracking();
      sat_count = 0;
      for(i=0; i<12; i++) {
         prn = tsip_byte();
         mode = tsip_byte();
         sig = tsip_byte();
         iode = tsip_byte();
         status = tsip_word();
         if((prn >= 1) && (prn <= MAX_PRN)) { 
            sat[prn].sig_level = (float) sig;
            sat[prn].level_msg = 13;

            sat[prn].code_phase = (float) mode;

            acc = (status & 0x000F);
            if     (acc == 0)  sat[prn].sv_accuracy = 2.4F;
            else if(acc == 1)  sat[prn].sv_accuracy = 3.4F;
            else if(acc == 2)  sat[prn].sv_accuracy = 4.85F;
            else if(acc == 3)  sat[prn].sv_accuracy = 6.85F;
            else if(acc == 4)  sat[prn].sv_accuracy = 9.65F;
            else if(acc == 5)  sat[prn].sv_accuracy = 13.65F;
            else if(acc == 6)  sat[prn].sv_accuracy = 24.00F;
            else if(acc == 7)  sat[prn].sv_accuracy = 48.00F;
            else if(acc == 8)  sat[prn].sv_accuracy = 96.00F;
            else if(acc == 9)  sat[prn].sv_accuracy = 192.00F;
            else if(acc == 10) sat[prn].sv_accuracy = 384.00F;
            else if(acc == 11) sat[prn].sv_accuracy = 768.00F;
            else               sat[prn].sv_accuracy = (-1.00F);

            if(sat[prn].code_phase != 0.0F) {
               if(have_phase == 0) need_redraw = 2044;
               have_phase = 2;
            }

            if(have_accu == 0) need_redraw = 2045;
            have_accu = 1;

            if(status & 0x0880) sat[prn].tracking = 1;
            else                sat[prn].tracking = (-1);

            record_sig_levels(prn);
            ++sat_count;
            have_count = 23;
         }
      }
      config_sat_count(sat_count);
   }

   r_status = get_moto_status();

   have_dops = 0;
   if((r_status & 0xE000) == 0xE000) {
      pdop = ((float) dop) / 10.0F;  // 3D fix
      have_dops |= PDOP;
   }
   else if((r_status & 0xE000) == 0x6000) {
      hdop = ((float) dop) / 10.0F;  // 2D fix
      have_dops |= HDOP;
   }
   else {
      hdop = pdop = ((float) dop) / 10.0F;  // 2D fix
      have_dops |= (HDOP | PDOP);  // !!!!
   }

   tsip_word();  // reserved

   if(long_msg) {
      bias = (int) (s16) tsip_word();  // nanoseconds
      ofs = tsip_dword();
      osc_offset = bias;  // !!!! zzzzz
      have_osc_offset = 6;
      t = ((float) (s16) tsip_word()) / 2.0F;  // temperature
      temperature = t;
      have_temperature = 4;

      utc = tsip_byte();  // time mode 
      if(utc & 0x80) time_flags |= 0x0001;     // utc time
      else           time_flags &= (~0x0001);  // gps time
      if(utc & 0x40) time_flags &= (~0x0008);  // utc offset avail
      else           time_flags |= (0x0008);

      if(!user_set_utc_ofs) {
         utc_offset = utc & 0x1F;
         check_utc_ofs(9);
      }

      get_moto_tz();
   }

   get_moto_tag();

//sprintf(plot_title, "r_status:%04X  bias:%d  ofs:%d  t:%f  utc:%02X  tag:%s  tmode:%d thresh:%d", 
//r_status, bias, ofs, t, utc, id_tag, traim_mode,traim_threshold);

   level_type = "SNR";

   update_gps_screen(1005);
}

void parse_moto_Hn()
{
int i;
S32 sawtooth;
s16 accu;
int traim_flag;
int prn;
u32 frac;

   if(moto_chans != 12) {
      moto_chans = 12;
      config_msg_ofs();
   }

   tsip_byte();   // pulse
   tsip_byte();   // synced to
   traim_status = tsip_byte();  // timing solution
   traim_flag = tsip_byte();    // traim status
   have_traim = 1;
//sprintf(debug_text2, "traim status:%d", traim_status);
   tsip_dword();  // Traim removed sats vector

   accu = tsip_word();     // time accuracy
   sawtooth = tsip_byte(); // sawtooth
   if(sawtooth & 0x80) sawtooth |= (~(S32)0xFF);
   have_sawtooth = 1;

   dac_voltage = (float) sawtooth;
   pps_offset = (double) accu;
   have_pps_offset = 6;

   for(i=0; i<12; i++) {
      prn = tsip_byte();  // prn
      frac = tsip_dword(); // time fraction
      if(frac >= 1000000000) frac = 0;

      if(0 && (prn >= 1) && (prn <= MAX_PRN)) {  // pppp
         sat[prn].sat_bias = (float) (((double) frac) * 1.0E-9);
         sat[prn].last_bias_msg = 2;

         if(frac > 0) {
            if(have_bias == 0) need_redraw = 2047;
            have_bias = 2;
         }
      }
   }
//sprintf(debug_text2, "Hn: thresh:%d tmode:%d tstat:%d flag:%d   pps:%d  saw:%d  rate:%d  accu:%d ", 
//traim_threshold,traim_mode,traim_status,traim_flag,   pps_mode,sawtooth,pps_rate,accu);
//sprintf(debug_text, "baud:%d db:%d par:%d stop:%d", baud_rate,data_bits,parity,stop_bits);
}


void parse_moto_Hr()
{
int i;

   if(moto_chans != 12) {
      moto_chans = 12;
      config_msg_ofs();
   }

   get_moto_time(0);                  

   gps_week = tsip_word();  // these are what we really want from this message         
   if(have_week == 0) need_redraw = 2048;
   have_week = 8;
   pri_tow = tow = this_tow = tsip_dword();               
   if(have_tow == 0) need_redraw = 2049;
   have_tow = 8;
   survey_tow = tow;

   get_moto_lla();    // lat/lon/alt               
   speed = tsip_word();       // speed          
   heading = tsip_word();       // heading        
   get_moto_status();                
   tsip_word();       // dop            
   tsip_byte();       // dop type       

   tsip_byte();       // vis count      
   tsip_word();       // fix vector     

   have_speed = 3;
   have_heading = 3;
                                     
   for(i=0; i<12; i++) {
      tsip_byte();    // prn
      tsip_byte();    // iode
      tsip_dword();   // pseudorange
      tsip_dword();   // range rate
   }
                                     
   get_moto_tag();    // id tag         
}

void parse_moto_Ia()
{
u08 stat1;
u16 stat2;
u32 status;

   stat1 = tsip_byte();
   stat2 = tsip_word();

   status = stat1;
   status <<= 16;
   status |= stat2; 

   critical_alarms = 0;
   minor_alarms &= (~0x0400);

   if(status & 0x020000) critical_alarms |= 0x0001;  // ROM
   if(status & 0x040000) critical_alarms |= 0x0002;  // RAM
   if(status & 0x012000) critical_alarms |= 0x0010;  // 1KHz / DCXO
   if(status & 0x000FFF) critical_alarms |= 0x0008;  // FPGA
   if(status & 0x305000) critical_alarms |= 0x0008;  // FPGA
   if(status & 0x000000) minor_alarms    |= 0x0400;  // EEPROM
   have_eeprom = 1;
   if(!have_critical_alarms) need_redraw = 7706;
   have_critical_alarms = 6;

   init_messages(26);
}

void parse_moto_Sz()
{
   // power up self test failed

   tsip_byte();

   critical_alarms = 0xFFFF;  // 0x1F;
   if(!have_critical_alarms) need_redraw = 7707;
   have_critical_alarms = 7;
   minor_alarms = 0xFFFF;
}


void skip_moto_msg()
{
   while(moto_msg_len > 0) {
      tsip_byte();
      --moto_msg_len;
   }
}


int find_moto_cmd(int cmd1, int cmd2)
{
   // !!!!! lookup Motorola binary command and return message length
   if     ((cmd1 == 'A') && (cmd2 == '@')) return (-1);

   else if((cmd1 == 'A') && (cmd2 == 'a')) return (10-7);
   else if((cmd1 == 'A') && (cmd2 == 'A')) return (8-7);
   else if((cmd1 == 'A') && (cmd2 == 'b')) return (10-7);
   else if((cmd1 == 'A') && (cmd2 == 'B')) return (8-7);
   else if((cmd1 == 'A') && (cmd2 == 'c')) return (11-7);
   else if((cmd1 == 'A') && (cmd2 == 'C')) return (9-7);
   else if((cmd1 == 'A') && (cmd2 == 'd')) return 11-7;
   else if((cmd1 == 'A') && (cmd2 == 'D')) return (9-7);
   else if((cmd1 == 'A') && (cmd2 == 'e')) return 11-7;
   else if((cmd1 == 'A') && (cmd2 == 'E')) return (8-7);
   else if((cmd1 == 'A') && (cmd2 == 'f')) return 15-7;
   else if((cmd1 == 'A') && (cmd2 == 'g')) return 8-7;
   else if((cmd1 == 'A') && (cmd2 == 'h')) return (8-7);
   else if((cmd1 == 'A') && (cmd2 == 'i')) return (9-7);
   else if((cmd1 == 'A') && (cmd2 == 'j')) return (8-7);
   else if((cmd1 == 'A') && (cmd2 == 'k')) return (9-7);
   else if((cmd1 == 'A') && (cmd2 == 'l')) return (9-7);
   else if((cmd1 == 'A') && (cmd2 == 'm')) return 12-7;
   else if((cmd1 == 'A') && (cmd2 == 'M')) return 11-7;
   else if((cmd1 == 'A') && (cmd2 == 'n')) return (8-7);
   else if((cmd1 == 'A') && (cmd2 == 'N')) return 8-7;
   else if((cmd1 == 'A') && (cmd2 == 'o')) return 25-7;
   else if((cmd1 == 'A') && (cmd2 == 'O')) return 8-7;
   else if((cmd1 == 'A') && (cmd2 == 'p')) return 25-7;
   else if((cmd1 == 'A') && (cmd2 == 'P')) return 8-7;
   else if((cmd1 == 'A') && (cmd2 == 'q')) return 8-7;
   else if((cmd1 == 'A') && (cmd2 == 'Q')) return 8-7;
   else if((cmd1 == 'A') && (cmd2 == 'r')) return (8-7);
   else if((cmd1 == 'A') && (cmd2 == 's')) return (20-7);
   else if((cmd1 == 'A') && (cmd2 == 't')) return (8-7);
   else if((cmd1 == 'A') && (cmd2 == 'u')) return (12-7);
   else if((cmd1 == 'A') && (cmd2 == 'v')) return (8-7);
   else if((cmd1 == 'A') && (cmd2 == 'w')) return 8-7;
   else if((cmd1 == 'A') && (cmd2 == 'x')) return (9-7);
   else if((cmd1 == 'A') && (cmd2 == 'y')) return 11-7;
   else if((cmd1 == 'A') && (cmd2 == 'z')) return 11-7;

   else if((cmd1 == 'B') && (cmd2 == 'a')) return 68-7;
   else if((cmd1 == 'B') && (cmd2 == 'b')) return 92-7;
   else if((cmd1 == 'B') && (cmd2 == 'c')) return 82-7;
   else if((cmd1 == 'B') && (cmd2 == 'd')) return 23-7;
   else if((cmd1 == 'B') && (cmd2 == 'e')) return 33-7;
   else if((cmd1 == 'B') && (cmd2 == 'f')) return 80-7;
   else if((cmd1 == 'B') && (cmd2 == 'g')) return 122-7;
   else if((cmd1 == 'B') && (cmd2 == 'h')) return 52-7;
   else if((cmd1 == 'B') && (cmd2 == 'i')) return 80-7;
   else if((cmd1 == 'B') && (cmd2 == 'j')) return 8-7;
   else if((cmd1 == 'B') && (cmd2 == 'k')) return 69-7;
   else if((cmd1 == 'B') && (cmd2 == 'l')) return 41-7;
   else if((cmd1 == 'B') && (cmd2 == 'n')) return 59-7;
   else if((cmd1 == 'B') && (cmd2 == 'o')) return 8-7;
   else if((cmd1 == 'B') && (cmd2 == 'p')) return 29-7;

   else if((cmd1 == 'C') && (cmd2 == 'a')) return 9-7;
   else if((cmd1 == 'C') && (cmd2 == 'b')) return 9-7;
   else if((cmd1 == 'C') && (cmd2 == 'd')) return 171-7;
   else if((cmd1 == 'C') && (cmd2 == 'e')) return 7-7;
   else if((cmd1 == 'C') && (cmd2 == 'f')) return 7-7;
   else if((cmd1 == 'C') && (cmd2 == 'g')) return 8-7;
   else if((cmd1 == 'C') && (cmd2 == 'h')) return 9-7;
   else if((cmd1 == 'C') && (cmd2 == 'i')) return 8-7;   // actually msg has no response
   else if((cmd1 == 'C') && (cmd2 == 'j')) return 294-7;
   else if((cmd1 == 'C') && (cmd2 == 'k')) return 7-7;
   else if((cmd1 == 'C') && (cmd2 == 'o')) return 29-7;

   else if((cmd1 == 'E') && (cmd2 == 'a')) return 76-7;
   else if((cmd1 == 'E') && (cmd2 == 'c')) return 82-7;
   else if((cmd1 == 'E') && (cmd2 == 'g')) return 158-7;
   else if((cmd1 == 'E') && (cmd2 == 'k')) return 71-7;
   else if((cmd1 == 'E') && (cmd2 == 'n')) return 69-7;
   else if((cmd1 == 'E') && (cmd2 == 'q')) return 96-9;  // !!!! BASTARD format

   else if((cmd1 == 'F') && (cmd2 == 'a')) return 9-7;
   else if((cmd1 == 'F') && (cmd2 == 'd')) return 189-7;

   else if((cmd1 == 'G') && (cmd2 == 'a')) return 20-7;
   else if((cmd1 == 'G') && (cmd2 == 'b')) return 17-7;
   else if((cmd1 == 'G') && (cmd2 == 'c')) return 8-7;
   else if((cmd1 == 'G') && (cmd2 == 'd')) return 8-7;
   else if((cmd1 == 'G') && (cmd2 == 'e')) return 8-7;
   else if((cmd1 == 'G') && (cmd2 == 'f')) return 9-7;
   else if((cmd1 == 'G') && (cmd2 == 'j')) return 21-7;
   else if((cmd1 == 'G') && (cmd2 == 'k')) return 13-7;

   else if((cmd1 == 'H') && (cmd2 == 'a')) return 154-7;
   else if((cmd1 == 'H') && (cmd2 == 'b')) return 54-7;
   else if((cmd1 == 'H') && (cmd2 == 'n')) return 78-7;
   else if((cmd1 == 'H') && (cmd2 == 'r')) return 170-7;

   else if((cmd1 == 'I') && (cmd2 == 'a')) return 10-7;

   else if((cmd1 == 'S') && (cmd2 == 'z')) return 8-7;

   else if((cmd1 == 'W') && (cmd2 == 'b')) return 8-7;

   return (-1);
}

void decode_moto_msg(int cmd1, int cmd2)
{
   start_msg_decode(1);  // ggggg
   // parse and decode Motorola binary message
   if     ((cmd1 == 'A') && (cmd2 == 'A')) skip_moto_msg();   // sat positions

   else if((cmd1 == 'A') && (cmd2 == 'a')) skip_moto_msg();   // time
   else if((cmd1 == 'A') && (cmd2 == 'A')) skip_moto_msg();   // ephemeris hold correction
   else if((cmd1 == 'A') && (cmd2 == 'b')) skip_moto_msg();   // GMT correction
   else if((cmd1 == 'A') && (cmd2 == 'B')) skip_moto_msg();   // application type select
   else if((cmd1 == 'A') && (cmd2 == 'c')) skip_moto_msg();   // date
   else if((cmd1 == 'A') && (cmd2 == 'C')) skip_moto_msg();   // 2D to 0D dop threshold
   else if((cmd1 == 'A') && (cmd2 == 'd')) parse_moto_Ad();   // lat
   else if((cmd1 == 'A') && (cmd2 == 'D')) skip_moto_msg();   // correction thresholds
   else if((cmd1 == 'A') && (cmd2 == 'e')) parse_moto_Ae();   // lon
   else if((cmd1 == 'A') && (cmd2 == 'E')) skip_moto_msg();   // output align
   else if((cmd1 == 'A') && (cmd2 == 'f')) parse_moto_Af();   // alt
   else if((cmd1 == 'A') && (cmd2 == 'g')) parse_moto_Ag();   // elevation mask
   else if((cmd1 == 'A') && (cmd2 == 'h')) skip_moto_msg();   // satellite select options
   else if((cmd1 == 'A') && (cmd2 == 'i')) skip_moto_msg();   // manual satellite select
   else if((cmd1 == 'A') && (cmd2 == 'j')) skip_moto_msg();   // DOP type
   else if((cmd1 == 'A') && (cmd2 == 'J')) skip_moto_msg();   // differential timeout select
   else if((cmd1 == 'A') && (cmd2 == 'k')) skip_moto_msg();   // DOP hysteresis
   else if((cmd1 == 'A') && (cmd2 == 'l')) skip_moto_msg();   // 3D to 2D dop threshold
   else if((cmd1 == 'A') && (cmd2 == 'm')) parse_moto_Am();   // sat ignore list
   else if((cmd1 == 'A') && (cmd2 == 'M')) skip_moto_msg();   // 
   else if((cmd1 == 'A') && (cmd2 == 'n')) skip_moto_msg();   // almanac update
   else if((cmd1 == 'A') && (cmd2 == 'N')) parse_moto_AN();   // velocity filer
   else if((cmd1 == 'A') && (cmd2 == 'o')) skip_moto_msg();   // datum
   else if((cmd1 == 'A') && (cmd2 == 'O')) skip_moto_msg();   // RTCM baud rate
   else if((cmd1 == 'A') && (cmd2 == 'p')) skip_moto_msg();   // user datum
   else if((cmd1 == 'A') && (cmd2 == 'P')) parse_moto_AP();   // PPS rate
   else if((cmd1 == 'A') && (cmd2 == 'q')) parse_moto_Aq();   // ionospheic correction enabled
   else if((cmd1 == 'A') && (cmd2 == 'Q')) parse_moto_AQ();   // position filter
   else if((cmd1 == 'A') && (cmd2 == 'r')) parse_moto_Ar();   // position fix algorithm type
   else if((cmd1 == 'A') && (cmd2 == 's')) skip_moto_msg();   // position hold position
   else if((cmd1 == 'A') && (cmd2 == 't')) parse_moto_At();   // position hold select
   else if((cmd1 == 'A') && (cmd2 == 'u')) skip_moto_msg();   // altitude hold height
   else if((cmd1 == 'A') && (cmd2 == 'v')) skip_moto_msg();   // altitude hold select
   else if((cmd1 == 'A') && (cmd2 == 'w')) parse_moto_Aw();   // gps/utc select
   else if((cmd1 == 'A') && (cmd2 == 'x')) skip_moto_msg();   // measurement epcho offser
   else if((cmd1 == 'A') && (cmd2 == 'y')) parse_moto_Ay();   // pps offset delay
   else if((cmd1 == 'A') && (cmd2 == 'z')) parse_moto_Az();   // cable delay (for 1PPS)

   else if((cmd1 == 'B') && (cmd2 == 'a')) parse_moto_Ea(6);  // 6 chan positioning
   else if((cmd1 == 'B') && (cmd2 == 'b')) parse_moto_Bb();   // sat positions
   else if((cmd1 == 'B') && (cmd2 == 'c')) skip_moto_msg();   // 6 chan dops
   else if((cmd1 == 'B') && (cmd2 == 'd')) parse_moto_Bd();   // almanac status
   else if((cmd1 == 'B') && (cmd2 == 'e')) skip_moto_msg();   // almanac data
   else if((cmd1 == 'B') && (cmd2 == 'f')) skip_moto_msg();   // ephermeris data
   else if((cmd1 == 'B') && (cmd2 == 'g')) skip_moto_msg();   // 6 chan range
   else if((cmd1 == 'B') && (cmd2 == 'h')) skip_moto_msg();   // pseudorange data
   else if((cmd1 == 'B') && (cmd2 == 'i')) skip_moto_msg();   // ephemeris data
   else if((cmd1 == 'B') && (cmd2 == 'j')) parse_moto_Bj();   // leapsecond pending
   else if((cmd1 == 'B') && (cmd2 == 'k')) parse_moto_Ek(6);  // 6 chan position
   else if((cmd1 == 'B') && (cmd2 == 'l')) skip_moto_msg();   // broadcast data data
   else if((cmd1 == 'B') && (cmd2 == 'n')) parse_moto_En(6);  // 6 chan traim
   else if((cmd1 == 'B') && (cmd2 == 'o')) parse_moto_Bo();   // UTC offset from GPS time
   else if((cmd1 == 'B') && (cmd2 == 'p')) skip_moto_msg();   // ionosphere data

   else if((cmd1 == 'C') && (cmd2 == 'a')) parse_moto_Ca();   // 6-channel self test
   else if((cmd1 == 'C') && (cmd2 == 'b')) skip_moto_msg();   // almanac input response
   else if((cmd1 == 'C') && (cmd2 == 'd')) skip_moto_msg();   // alert planning
   else if((cmd1 == 'C') && (cmd2 == 'e')) skip_moto_msg();   // pseudorange correction
   else if((cmd1 == 'C') && (cmd2 == 'f')) skip_moto_msg();   // receiver set to defaults
   else if((cmd1 == 'C') && (cmd2 == 'g')) parse_moto_Cg();   // idle/fix mode
   else if((cmd1 == 'C') && (cmd2 == 'h')) skip_moto_msg();   // almanac ack
   else if((cmd1 == 'C') && (cmd2 == 'i')) skip_moto_msg();   // switch I/O format
   else if((cmd1 == 'C') && (cmd2 == 'j')) parse_moto_Cj();   // receiver id
   else if((cmd1 == 'C') && (cmd2 == 'k')) skip_moto_msg();   // psudorange ack
   else if((cmd1 == 'C') && (cmd2 == 'o')) skip_moto_msg();   // ionosphere data

   else if((cmd1 == 'E') && (cmd2 == 'a')) parse_moto_Ea(8);  // 8 chan positioning
   else if((cmd1 == 'E') && (cmd2 == 'c')) skip_moto_msg();   // 8 chan dops
   else if((cmd1 == 'E') && (cmd2 == 'g')) skip_moto_msg();   // 8 chan range
   else if((cmd1 == 'E') && (cmd2 == 'k')) parse_moto_Ek(8);  // 8 chan position
   else if((cmd1 == 'E') && (cmd2 == 'n')) parse_moto_En(8);  // 8 chan traim

   else if((cmd1 == 'E') && (cmd2 == 'q')) parse_moto_Eq();   // !!!! BASTARD FORMAT ascii position

   else if((cmd1 == 'F') && (cmd2 == 'a')) parse_moto_Fa();   // 8-channel self test
   else if((cmd1 == 'F') && (cmd2 == 'd')) skip_moto_msg();   // alert planning

   else if((cmd1 == 'G') && (cmd2 == 'a')) parse_moto_Ga();   // lat/lon/alt
   else if((cmd1 == 'G') && (cmd2 == 'b')) parse_moto_Gb();   // date and time
   else if((cmd1 == 'G') && (cmd2 == 'c')) parse_moto_Gc();   // PPS status
   else if((cmd1 == 'G') && (cmd2 == 'd')) parse_moto_Gd();   // positioning mode
   else if((cmd1 == 'G') && (cmd2 == 'e')) parse_moto_Ge();   // traim mode
   else if((cmd1 == 'G') && (cmd2 == 'f')) parse_moto_Gf();   // traim alarm threshold
   else if((cmd1 == 'G') && (cmd2 == 'j')) parse_moto_Gj();   // leapsecond info
   else if((cmd1 == 'G') && (cmd2 == 'k')) parse_moto_Gk();   // vehicle id tag

   else if((cmd1 == 'H') && (cmd2 == 'a')) parse_moto_Ha(1);  // 
   else if((cmd1 == 'H') && (cmd2 == 'b')) parse_moto_Ha(0);  // 
   else if((cmd1 == 'H') && (cmd2 == 'n')) parse_moto_Hn();   // !!!! traim
   else if((cmd1 == 'H') && (cmd2 == 'r')) parse_moto_Hr();   // inverse differential

   else if((cmd1 == 'I') && (cmd2 == 'a')) parse_moto_Ia();   // 12-channel self test

   else if((cmd1 == 'S') && (cmd2 == 'z')) parse_moto_Sz();   // power-up test failure
   else if((cmd1 == 'W') && (cmd2 == 'b')) parse_moto_Wb();   // Jupiter-T message format switch
}


void get_moto_message()
{
u08 c;
static int cmd1 = 0;
static int cmd2 = 0;
static int eq_cksum = 0; // ascii Eq message checksum

   // this routine buffers up an incoming Motorola binary message

   if(SERIAL_DATA_AVAILABLE() == 0) {
      check_com_timer();
      return;
   }
   else {
      reset_com_timer();
   }

   c = get_com_char();
   if(rcv_error) {      // parity/framing/overrun errors
      rcv_error = 0;
//!!!!!qqqq    goto rst_msg;
   }

   if(tsip_sync == 0) {         // syncing to start of message, search for a @
      if(c == '@') {
         tsip_sync = 1;
         get_sync_time();
      }
      tsip_wptr = 0;
      tsip_rptr = 0;
      moto_msg_len = moto_msg_ptr = 0;
      moto_vfy_cksum = 0;
      return;
   }
   else if(tsip_sync == 1) {    // @ had been seen, now check next byte for '@'
      if(c != '@') goto rst_msg;
      ++tsip_sync;
   }
   else if(tsip_sync == 2) {    // get first byte of command
      if((c >= 'A') && (c <= 'Z')) {
         cmd1 = c;
         moto_vfy_cksum ^= c;
         ++tsip_sync;
      }
      else goto rst_msg;
   }
   else if(tsip_sync == 3) {   // next byte of command
      if((tolower(c) >= 'a') && (tolower(c) <= 'z')) {
         cmd2 = c;
         moto_vfy_cksum ^= c;
         moto_msg_len = moto_msg_ptr = find_moto_cmd(cmd1, cmd2);
         if(moto_msg_len < 0) goto rst_msg;       // unknown command
         else if(moto_msg_len == 0) ++tsip_sync;  // no payload bytes
      }
      else goto rst_msg;
      ++tsip_sync;
   }
   else if(tsip_sync == 4) {   // buffer up message bytes
      if(tsip_wptr < MAX_TSIP) {   // add char to the message buffer
         moto_vfy_cksum ^= c;
         tsip_buf[tsip_wptr++] = c;
      }
      else {
         tsip_error |= 0x8000;
         goto rst_msg;
      }
      if(--moto_msg_ptr <= 0) ++tsip_sync;  // end of message payload
   }
   else if(tsip_sync == 5) {   // get checksum and verify, parse payload
      if((cmd1 == 'E') && (cmd2 == 'q')) {  // BASTARD ASCII message format
         eq_cksum = (c - '0');
         ++tsip_sync;
      }
      else {
         if(c != moto_vfy_cksum) goto rst_msg;
         decode_moto_msg(cmd1, cmd2);
         tsip_sync = 0;
      }
   }
   else if(tsip_sync == 6) {  // second byte of ascii checksum
      eq_cksum = (eq_cksum*10) + (c - '0');
      ++tsip_sync;
   }
   else if(tsip_sync == 7) {  // final byte of ascii checksum
      eq_cksum = (eq_cksum*10) + (c - '0');
//sprintf(plot_title, "c:%c eq:%d vfy:%d", c, eq_cksum, moto_vfy_cksum);
      if(eq_cksum != moto_vfy_cksum) goto rst_msg;
      decode_moto_msg(cmd1, cmd2);
      tsip_sync = 0;
   }
   else {
      rst_msg:
      tsip_wptr = 0;
      tsip_sync = 0;
   }
}




//
//
//  SCPI receiver stuff
//
//

int get_scpi_field(int width)
{
char c;
int i;

   nmea_field[0] = 0;
   if(width <= 0) return 0;
   if(nmea_msg[nmea_col] == 0) return 0;  // at end-of-message

   i = 0;
   while(width--) {   // get next field from the nmea message
      c = nmea_msg[nmea_col];
      if(c == 0) {  // reached end-of-message
         break;
      }

      if(i < ((int)sizeof(nmea_field)-2)) {
         nmea_field[i++] = c;
         nmea_field[i] = 0;
         ++nmea_col;
      }
   }

   return nmea_field[0];
}


int sent_uccm_status;

int fake_missing_second(double delta)
{
double jd;
double this_t,last_t;
static double last_jd = 0.0;
int old_yy;
int old_mm;
int old_dd;
int old_hh;
int old_min;
int old_sec;
double old_frac;

   // this routine generates a fake time message every other second in order
   // to support telecom receivers that work on 2 second intervals.
   // Returns 1 if call would cause a duplicated time stamp, else returns 0.

   old_yy = year;      // used to fake missing odd time time stamps
   old_mm = month;
   old_dd = day;
   old_hh = hours;
   old_min = minutes;
   old_sec = seconds;
   old_frac = raw_frac;

   jd = jdate(year,month,day);
   jd += jtime(hours,minutes,seconds, 0.000);  // !!!! raw_frac?
   if(jd == last_jd) return 1;

   last_jd = jd;

   gregorian(jd - (delta / (24.0*60.0*60.0)));  // back up delta seconds

   pri_year = year = g_year;
   pri_month = month = g_month;
   pri_day = day = g_day;
   pri_hours = hours = g_hours;
   pri_minutes = minutes = g_minutes;
   pri_seconds = seconds = g_seconds;
   pri_frac = raw_frac = g_frac;

   this_t = this_time_msec;
   last_t = last_time_msec;

   fake_time_stamp = 1;
   update_gps_screen(554);
//Sleep(500);
   fake_time_stamp = 0;

   if(pri_seconds == SCPI_STATUS_SECOND) {
     if(rcvr_type == UCCM_RCVR) queue_scpi_cmd("SYST:STAT?", UCCM_STATUS_MSG);
     else                       queue_scpi_cmd(":SYST:STAT?", SCPI_STATUS_MSG);
     sent_uccm_status = 1;
   }

   this_time_msec = this_t + (1000.0 * delta);  // fake the timestamp of the faked time stamp message
   last_time_msec = last_t + (1000.0 * delta);

   pri_year = year = old_yy;        // restore real time stamp
   pri_month = month = old_mm;
   pri_day = day = old_dd;
   pri_hours = hours = old_hh;
   pri_minutes = minutes = old_min;
   pri_seconds = seconds = old_sec;
   pri_frac = raw_frac = old_frac;
   return 0;
}


void parse_scpi_time()
{
u32 val;
double jd;

pps_enabled = pps_mode = 1; // zzzzz
   have_pps_enable = 8;
   have_pps_mode = 8;
   sent_uccm_status = 0;

   nmea_col = 0;
   get_scpi_field(3);  // get " T#"
if(debug_file) fprintf(debug_file, "scpi time:%s\n", nmea_field);

   if(!strcmp(nmea_field, " T1")) {  // hex format time
      get_scpi_field(2);
      if(strcmp(nmea_field, "#H")) return;
      get_scpi_field(8);
      sscanf(nmea_field, "%x", &val);
if(debug_file) fprintf(debug_file, "time val:%08X(%u)", val,val);

      jd = (double) (u32) val;
      jd /= (24.0*60.0*60.0);
      jd += GPS_EPOCH;
      
      gregorian(jd); 

      pri_year = year = g_year;
      pri_month = month = g_month;
      pri_day = day = g_day;
      pri_hours = hours = g_hours;
      pri_minutes = minutes = g_minutes;
      pri_seconds = seconds = g_seconds;
      pri_frac = raw_frac = g_frac;

      if((scpi_type == NORTEL_TYPE) && (time_flags & 0x0001)) {
         gps_to_utc();
      }

      goto time_info;
   }
   else if(!strcmp(nmea_field, " T2")) {
      get_scpi_field(4);
      pri_year = year = atoi(nmea_field);
      get_scpi_field(2);
      pri_month = month = atoi(nmea_field);
      get_scpi_field(2);
      pri_day = day = atoi(nmea_field);

      get_scpi_field(2);
      pri_hours = hours = atoi(nmea_field);
      get_scpi_field(2);
      pri_minutes = minutes = atoi(nmea_field);
      get_scpi_field(2);
      pri_seconds = seconds = atoi(nmea_field);
      pri_frac = raw_frac = 0.0;

      time_info:
      if(get_scpi_field(1)) {  // TFOM
         tfom = atoi(nmea_field);
         have_tfom = 2;
      }
      if(get_scpi_field(1)) {  // FFOM
         ffom = atoi(nmea_field);
         have_ffom = 2;
      }

      get_scpi_field(1);  // leap pending
      if(nmea_field[0] == '+') {
         minor_alarms |= (0x0080);
         have_leap_info = 5;
      }
      else if(nmea_field[0] == '-') {
         minor_alarms |= (0x0080);
         have_leap_info = 6;
      }
      else if(nmea_field[0] == '0') {
         minor_alarms &= (~0x0080);
         have_leap_info = 7;
      }

      get_scpi_field(1);  // request for service

      get_scpi_field(1);  // time validity
      if(nmea_field[0] == '1') {       // time invalid
         time_flags |= 0x0004;
      }
      else if(nmea_field[0] == '0') {  // time valid
         time_flags &= (~0x0004);
      }

      get_scpi_field(2);  // checksum
if(debug_file) fprintf(debug_file, "time code: %04d/%02d/%02d %02d:%02d:%02d\n", pri_year,pri_month,pri_day, pri_hours,pri_minutes,pri_seconds);

      fake_time_stamp = 0;
      if(scpi_type == NORTEL_TYPE) {  // fake every other time stamp
         if(fake_missing_second(1.0)) return;
      }

      update_gps_screen(1006);
      if(sent_uccm_status) ;
      else if(pri_seconds == SCPI_STATUS_SECOND) {
         if(rcvr_type == UCCM_RCVR) queue_scpi_cmd("SYST:STAT?", UCCM_STATUS_MSG);
         else                       queue_scpi_cmd(":SYST:STAT?", SCPI_STATUS_MSG);
      }
   }
   else {  // message requests/responses out of sync?
BEEP(205);  // gggg
if(debug_file) fprintf(debug_file, "Message sync error (%s): %s\n", nmea_field, nmea_msg);
      update_gps_screen(1007);
      scpi_in = scpi_out = 0;
      scpi_q_entries = 0; 
      init_messages(50);
   }
}


void parse_scpi_vis()
{
int prn;
int sat_count;

   sat_count = 0;
   for(prn=0; prn<=MAX_PRN; prn++) {
      sat[prn].visible = 0;
   }

   nmea_col = 0;
   while(get_nmea_field()) {
      prn = atoi(nmea_field);
      if((prn >= 1) && (prn <= MAX_PRN)) {  // nnnnnnn
         sat[prn].visible = (-1);
         ++sat_count;
         have_count = 40;
      }
   }

   config_sat_count(sat_count);
}


void parse_scpi_track()
{
int prn;

   for(prn=0; prn<=MAX_PRN; prn++) { 
      sat[prn].tracking = sat[prn].visible;
      if(sat[prn].visible) sat[prn].level_msg = 14;
      else                 sat[prn].level_msg = 0;
   }

   nmea_col = 0;
   while(get_nmea_field()) {
      prn = atoi(nmea_field);
      if((prn >= 1) && (prn <= MAX_PRN)) {  // nnnnnnn
         sat[prn].tracking = (1);
      }
   }
}

void parse_scpi_efc()
{
   nmea_col = 0;
   if(get_nmea_field()) {
      dac_voltage = (float) atof(nmea_field);
      have_dac = 1;
   }
}

void parse_uccm_efc()
{
u32 val;

//kkkkkkuuuuuu
   nmea_col = 0;
   if(strstr(nmea_msg, "TO GPS")) {  // recovering from manual mode
      discipline_mode = 4;  // recovery
   }
   else if(strstr(nmea_msg, "RANGE")) {  // out of range
   }
   else if(get_nmea_field() > 2) {
      if(discipline_mode == 4) discipline_mode = 0;
      sscanf(&nmea_field[2], "%x", &val);
      uccm_voltage = (float) val;
   }
// sprintf(debug_text, "uccm voltage:%f  msg:%s", uccm_voltage, nmea_msg); 
}

void parse_uccm_led()
{
int i;

   if(scpi_type == UCCMP_TYPE) { //kkkkkuuuuuu
      if     (strstr(nmea_msg, "0")) strcpy(uccm_led_msg, "Initializing   ");
      else if(strstr(nmea_msg, "1")) strcpy(uccm_led_msg, "Normal         ");
      else                           strcpy(uccm_led_msg, "Unknown status ");
   }
   else {
      nmea_msg[16] = 0;
      strcpy(uccm_led_msg, nmea_msg);
      for(i=strlen(uccm_led_msg); i<16; i++) uccm_led_msg[i] = ' ';
      uccm_led_msg[16] = 0;
   }
}

void parse_scpi_cable()
{
   nmea_col = 0;
   if(get_nmea_field()) {
      cable_delay = atof(nmea_field);
//    if(have_cable_delay == 0) need_redraw = 2050;
      have_cable_delay = 1;
      show_cable_delay();
   }
}

void parse_scpi_elev()
{
   nmea_col = 0;
   if(get_nmea_field()) {
      el_mask = (float) atof(nmea_field);
      have_el_mask = 1;
   }
}

void parse_scpi_utc_ofs()
{
   nmea_col = 0;
   if(!user_set_utc_ofs && get_nmea_field()) {
      utc_offset = (int) atof(nmea_field);
      check_utc_ofs(10);
   }
}

void parse_scpi_leaptime()
{
char *s;
u32 val;
int yy,mm,dd;
double leap_day;

   // calculates the date of the next leapsecond event

   val = 0;

   nmea_col = 0;
   if(get_nmea_field()) { 
      strupr(nmea_field);
      s = strstr(nmea_field, "#H");
      if(scpi_type == NORTEL_TYPE) {
         s = "";
         sscanf(&nmea_field[0], "%x", &val);
         goto hex_time;
      }
      else if(s) {  // hex format
         if(s[2] == 0) return;
         sscanf(s+2, "%x", &val);

         hex_time:
         leap_day = (double) (u32) val;
         leap_day /= (24.0*60.0*60.0);
         leap_day += GPS_EPOCH;
         leap_day -= jtime(0,0,utc_offset, 0.0);  // convert GPS to UTC
         leap_day = ((double) (int) leap_day) + 0.50;  // removes time fraction
         leap_day -= jtime(0,0,0, 1.0);  // gets time of leap to 23:59:59
         have_scpi_hex_leap = 1;
//gregorian(leap_day);
//sprintf(debug_text2, "hexday:%f utcofs:%d  %04d/%02d/%02d %02d:%02d:%02d", leap_day, utc_offset, g_year,g_month,g_day, g_hours,g_minutes,g_seconds);
      }
      else if(0 && have_scpi_hex_leap) {  // we have leap date from the hex format message
         return;
      }
      else { // yyyy mm dd format  // this message can be wrong on devcices that send both formats
          yy = atoi(nmea_field);   // ... might say 30 Sep if leap pending flag
          if(yy < 2016) return;    // ... was broadcast in July
          if(yy > 2100) return;

          if(get_nmea_field() == 0) return;
          mm = atoi(nmea_field);
          if(mm < 1) return;
          if(mm > 12) return;

          if(get_nmea_field() == 0) return;
          dd = atoi(nmea_field);
          if(dd < 1) return;
          if(dd > 31) return;

          leap_day = jdate(yy, mm, dd) + jtime(23,59,59,0.0);
          if(dd == 1) leap_day -= 1.0;
          sprintf(out, "%04d/%02d/%02d", yy,mm,dd);
//sprintf(debug_text3, "lday2:%f  %s", leap_day, out);
          s = &out[0];
      }

      leap_day += (double) (rolled * 1024*7);
      jd_leap = leap_day;
      calc_jd_leap(4);
///have_leap_days = 30;
///gregorian(leap_day);
//sprintf(plot_title, "rolled:%d  lday:%04d/%02d/%02d  %02d:%02d:%02d  today:%04d/%02d/%02d", rolled, g_year,g_month,g_day, g_hours,g_minutes,g_seconds, year,month,day);
///      today = jdate(year, month, day);
///      leap_days = (int) (leap_day - today);
///      have_leap_days = 3;
///sprintf(debug_text, "leap_day:%f today:%f leap_days:%d  rolled:%d", leap_day,today,leap_days, rolled);

if(debug_file) fprintf(debug_file, "### leaptime:%08X(%u)  leap_day:%f  leap_days:%d  s:%s", val, val, jd_leap, leap_days, s);
   }
}


void parse_scpi_id()
{
int i;

   nmea_col = 0;
   if(get_nmea_field()) {  // manufacturer
      if(nmea_field[0] == ' ') {
         sprintf(scpi_mfg_id, "Mfg: %-18s", &nmea_field[0]);
         strncpy(scpi_mfg, &nmea_field[1], 20);
      }
      else {
         sprintf(scpi_mfg_id, "Mfg:  %-18s", nmea_field);
         strncpy(scpi_mfg, nmea_field, 20);
      }
      scpi_mfg[20] = 0;
      scpi_mfg_id[24] = 0;
   }

   if(get_nmea_field()) {  // model
      for(i=0; i<UNIT_LEN; i++) unit_name[i] = ' ';
      unit_name[UNIT_LEN] = 0;
      for(i=0; i<UNIT_LEN; i++) {
         if(nmea_field[i] == 0) break;
         unit_name[i] = nmea_field[i];
         scpi_model[i] = nmea_field[i];
         scpi_model[i+1] = 0;
      }
   }

   scpi_serno[0] = 0;
   if(get_nmea_field()) {  // serial number
      if     (scpi_type == UCCMP_TYPE) sprintf(scpi_serno, "Ser:  %s", &nmea_field[5]);
      else if(rcvr_type == UCCM_RCVR)  sprintf(scpi_serno, "Ser:  %s", &nmea_field[3]);
      else                             sprintf(scpi_serno, "Ser:  %s", nmea_field);
      strncpy(scpi_sn, nmea_field, 22);
      scpi_sn[22] = 0;
      scpi_serno[23] = 0;
   }

   scpi_fw[0] = 0;
   if(get_nmea_field()) {  // fw rev
      strncpy(scpi_fw, nmea_field, 22);
      scpi_fw[22] = 0;
      strcat(scpi_serno, " ");
      strcat(scpi_serno, scpi_fw);
      scpi_serno[23] = 0;
   }

   if(scpi_type == NORTEL_TYPE) {
      scpi_serno[0] = 0;
      if(get_nmea_field()) {  // CPC - serial number
         if     (scpi_type == UCCMP_TYPE) sprintf(scpi_serno, "Ser:  %s", &nmea_field[5]);
         else if(rcvr_type == UCCM_RCVR)  sprintf(scpi_serno, "Ser:  %s", &nmea_field[3]);
         else                             sprintf(scpi_serno, "Ser:  %s", nmea_field);
         strncpy(scpi_sn, nmea_field, 22);
         scpi_sn[22] = 0;

         strcat(scpi_serno, " ");
         strcat(scpi_serno, scpi_fw);
         scpi_serno[23] = 0;
      }

      if(get_nmea_field()) {  // PEC - model
         for(i=0; i<UNIT_LEN; i++) unit_name[i] = ' ';
         unit_name[UNIT_LEN] = 0;
         for(i=0; i<UNIT_LEN; i++) {
            if(nmea_field[i] == 0) break;
            unit_name[i] = nmea_field[i];
            scpi_model[i] = nmea_field[i];
            scpi_model[i+1] = 0;
         }
      }

      if(get_nmea_field()) {  // hardware version
         nmea_field[2] = 0;
         strcat(scpi_serno, "-");
         strcat(scpi_serno, nmea_field);
         scpi_serno[23] = 0;
      }
   }

   if(have_info == 0) have_info = (MANUF_PARAMS | PRODN_PARAMS | VERSION_INFO);

if(debug_file) fprintf(debug_file, "### id:%s  have_info:%02X\n", nmea_msg, have_info);

// show_version_info();
}

void parse_scpi_tint()
{
   nmea_col = 0;
   if(get_nmea_field()) {
      if(scpi_type == NORTEL_TYPE) pps_offset = (float) atof(nmea_field)*1.0;
      else                         pps_offset = (float) atof(nmea_field)*1.0E9;
      have_pps_offset = 7;
   }
}

void parse_scpi_progress()
{
   nmea_col = 0;
   if(get_nmea_field()) {
      survey_progress = (int) atof(nmea_field);
      if(scpi_type == NORTEL_TYPE) {
         if(survey_progress != 100) minor_alarms |= 0x0020;
         else                       minor_alarms &= (~0x0020);
      }
   }
}

void parse_scpi_survey()
{
   nmea_col = 0;
// !!! should restore com_timeout
   if(get_nmea_field()) {
      strupr(nmea_field);
      if(strstr(nmea_field, "ONCE"))   minor_alarms |= 0x0020;
      else if(strstr(nmea_field, "1")) minor_alarms |= 0x0020;
      else                             minor_alarms &= (~0x0020);
   }
}

int have_scpi_hold;

void parse_scpi_holdover()
{
int state;

   nmea_col = 0;
   if(get_nmea_field()) {
      holdover = (int) atof(nmea_field);
   }

   if(get_nmea_field()) {
      state = atoi(nmea_field);
      if(state) {  // in holdover mode
         discipline_mode = 3;
         if(!have_scpi_hold) user_holdover = 1;
if(scpi_type != UCCMP_TYPE) {  // kkkkkk
   rcvr_mode = RCVR_MODE_PROP;
   have_rcvr_mode = 10;
}
      }
      else {   // not in holdover
         discipline_mode = 0;
         if(!have_scpi_hold) user_holdover = 0;
if(scpi_type != UCCMP_TYPE) {  // kkkkkk
   rcvr_mode = RCVR_MODE_HOLD;
   have_rcvr_mode = 10;
}
      }
      have_scpi_hold = 1;
   }
// !!! should restore com_timeout
if(debug_file) fprintf(debug_file, "### holdover:%d  dmode:%d  user:%d\n", holdover, discipline_mode, user_holdover);
}

void parse_scpi_jamsync()
{
}

void parse_scpi_utc()
{
int state;

   nmea_col = 0;
   if(get_nmea_field()) {
      state = (int) atof(nmea_field);
      if(state == 0) {  // GPS time
         time_flags &= (~0x0001);
         timing_mode = 0x00;
         have_timing_mode = 1;
      }
      else {  // utc time
         time_flags |= (0x0001);
         timing_mode = 0x03;
         have_timing_mode = 1;
      }
   }
}

void parse_scpi_tunc()
{
   nmea_col = 0;
   if(get_nmea_field()) {
      osc_offset = (float) atof(nmea_msg);
      have_osc_offset = 7;
osc_offset *= 1000.0;
   }
}

void parse_scpi_edge()
{
   nmea_col = 0;
   if(get_nmea_field()) {
      strupr(nmea_field);
      if     (strstr(nmea_field, "RIS")) {
         pps_polarity = 0;
         have_pps_polarity = 12;
      }
      else if(strstr(nmea_field, "FAL")) {
         pps_polarity = 1;
         have_pps_polarity = 12;
      }
   }
}

void parse_scpi_life()
{
   nmea_col = 0;
   if(get_nmea_field()) {
      scpi_life = atoi(nmea_field) * 3;
      have_lifetime = 1;
   }
}

void parse_scpi_user()
{
static int msg_num = 0;
int i;

   ++msg_num;

   i = (int) sizeof(plot_title)-20;
   if(i < (int) sizeof(nmea_msg)) nmea_msg[i] = 0;

   sprintf(plot_title, "response %d:%s", msg_num, nmea_msg);
   need_redraw = 543;  // why is this needed for X11?
}

void parse_scpi_test()
{
// !!! should restore com_timeout
   nmea_col = 0;
   if(get_nmea_field()) {
      scpi_test = atoi(nmea_msg);
      have_scpi_test = 1;
if(debug_file) fprintf(debug_file, "### self test:%04X\n", scpi_test);
   }
}

void parse_scpi_hardware()
{
int status;

   nmea_col = 0;
   if(get_nmea_field()) {
      status = atoi(nmea_msg);
if(debug_file) fprintf(debug_file, "### hardware status:%04X\n", status);
      if(scpi_type == NORTEL_TYPE) {
        // 0x0001 - self test fail
        // 0x0002 - power fail
        // 0x0004 - GPS 1PPS fail
        // 0x0008 - OCXO fail
        // 0x0010 - EFC fail
        // 0x0020 - PLL fail
        // 0x0040 - EEPROM fail
        // 0x0080 - RAM fail
        // 0x0100 - FPGA fail
        // 0x0200 - antenna RF signal too high
        // 0x0400 - antenna RF signal too low
        // 0x0800 - GPS fail (TRAIM failure)
        // 0x1000 - PDOP fail
        if(status & 0x0002) critical_alarms |= 0x0004;      // power
        else                critical_alarms &= (~0x0004);
        if(status & 0x0008) critical_alarms |= 0x0010;      // ocxo
        else                critical_alarms &= (~0x0010);
        if(status & 0x0080) critical_alarms |= 0x0002;      // RAM
        else                critical_alarms &= (~0x0002);
        if(status & 0x01A5) critical_alarms |= 0x0008;      // various hardware
        else                critical_alarms &= (~0x0008);
        if(!have_critical_alarms) need_redraw = 7701;
        have_critical_alarms = 555;

        if(status & 0x0010) minor_alarms |= 0x0001;      // EFC limit
        else                minor_alarms &= (~0x0001);
        have_osc_age = 1;

        if(status & 0x0040) minor_alarms |= 0x0400;      // eeprom
        else                minor_alarms &= (~0x0400);
        have_eeprom = 1;
      }
      else {
         critical_alarms = 0x0000;
         if(status & 0x003E) critical_alarms |= 0x0004;  // power
         if(status & 0x0300) critical_alarms |= 0x0008;  // GPS
         if(status & 0x1000) critical_alarms |= 0x0010;  // ref failure

         if(status & 0x00C0) minor_alarms |= 0x0001;     // EFC limit
         else                minor_alarms &= (~0x0001);  // EFC limit

         if(status & 0x0800) minor_alarms |= 0x0400;     // EEPROM fault
         else                minor_alarms &= (~0x0400);  // EEPROM ok
         have_eeprom = 1;

         if(!have_critical_alarms) need_redraw = 7708;
         have_critical_alarms = 8;
      }
   }
}

void parse_scpi_operation()
{
int status;

   if(scpi_type != NORTEL_TYPE) return;

   // 0x0002 - locked
   // 0x0004 - holdover/unlocked modes
   // 0x0008 - position hold
   // 0x0010 - sats tracked
   // 0x0020 - hardware status reg set
   // 0x0040 - diagnostic log almost full
   // 0x0080 - over temp
   // 0x0100 - under temp
   // 0x0200 - antenna fault

   nmea_col = 0;
   if(get_nmea_field()) {
      status = atoi(nmea_msg);
if(debug_file) fprintf(debug_file, "### operation status:%04X\n", status);
      if(status & 0x0080) {       // over temperature alarm
         temperature = 71.0F;
         have_temperature = 1;
      }
      else if(status & 0x0100) {  // under temperature alarm
         temperature = (-1.0F);
         have_temperature = 1;
      }
      else {
         temperature = 30.0F;     // normal temperature
         have_temperature = 1;
      }

      if(status & 0x0010) minor_alarms &= (~0x0008);  // sat tracking OK
      else                minor_alarms |= 0x0008;
      have_tracking = 1;
   }
}

void parse_scpi_pos()
{
}

void parse_scpi_ignore()
{
u32 val;
int count;
int prn;

   count = 0;
   val = 0;
   nmea_col = 0;
   while(get_nmea_field()) {
      prn = atoi(nmea_field);
      if((prn >= 1) && (prn <= MAX_PRN)) { // nnnnnnn
////  if((prn >= 1) && (prn <= 32)) { 
         val |= 1 << (prn-1);
         ++count;
      }
   }

   if(count == 31) single_sat_prn = count; // !!!! kludge
   else            single_sat_prn = 0;

   val = (~val);
   sats_enabled = val;
   update_disable_list(sats_enabled);
}

void parse_scpi_ign_count()
{
   nmea_col = 0;
   if(get_nmea_field()) {
      single_sat_prn = atoi(nmea_field);
   }
}

void parse_scpi_include()
{
u32 val;
int count;
int prn;

   if(scpi_type != NORTEL_TYPE) return;

   count = 0;
   val = 0;
   nmea_col = 0;
   while(get_nmea_field()) {
      prn = atoi(nmea_field);
      if((prn >= 1) && (prn <= 32)) { // nnnnnnn
         val |= 1 << (prn-1);
         ++count;
      }
   }

   if(count == 1) single_sat_prn = count; // !!!! kludge
   else           single_sat_prn = 0;

   sats_enabled = val;
   update_disable_list(sats_enabled);
}

void parse_scpi_posn()
{
double sign;

   sign = 1.0;
   nmea_col = 0;
   if(get_nmea_field()) {
      if(nmea_field[0] == 'S') sign = (-1.0);
   }
   if(get_nmea_field()) {
      lat = atof(nmea_field);
   }
   if(get_nmea_field()) {
      lat += atof(nmea_field) / 60.0;
   }
   if(get_nmea_field()) {
      lat += atof(nmea_field) / 3600.0;
   }
   lat *= sign;

   sign = 1.0;
   if(get_nmea_field()) {
      if(nmea_field[0] == 'W') sign = (-1.0);
   }
   if(get_nmea_field()) {
      lon = atof(nmea_field);
   }
   if(get_nmea_field()) {
      lon += atof(nmea_field) / 60.0;
   }
   if(get_nmea_field()) {
      lon += atof(nmea_field) / 3600.0;
   }
   lon *= sign;

   if(get_nmea_field()) {
      alt = atof(nmea_field);
   }
if(debug_file) fprintf(debug_file, "###lla: %g %g %g\n", lat,lon,alt);
   lat = lat * PI / 180.0;
   lon = lon * PI / 180.0;
}

void parse_scpi_reset()
{
// !!! should restore com_timeout
}

void parse_scpi_power()
{
}

void parse_scpi_antenna()
{
   minor_alarms &= (~0x0006);
   if(strstr(nmea_msg, "OK")) ;
   else if(strstr(nmea_msg, "SHORT")) minor_alarms |= 0x0004;
   else if(strstr(nmea_msg, "OPEN"))  minor_alarms |= 0x0002; 
   else                               minor_alarms |= 0x0006; 
   have_antenna = 1;
}


void parse_scpi_ffom()
{
}

void parse_uccm_rate()
{
   nmea_col = 0;
   if(get_nmea_field()) {
      if(strstr(nmea_field, "PP1S")) {
         pps_rate = 2;
         have_pps_rate = 6;
//pps_enabled = 1;
      }
      else if(strstr(nmea_field, "PP2S")) {
         pps_rate = 0x82;
         have_pps_rate = 6;
//pps_enabled = 1;
      }
//    have_pps_enable = 10;
   }
}

void parse_uccm_state()
{
   nmea_col = 0;
   if(get_nmea_field()) {
      if(strstr(nmea_field, "BLOCK")) {
         pps_enabled = 0;
      }
      else if(1 || strstr(nmea_field, "ACTIVE")) {  // ACTIVE, NORMAL UNLOCK
         pps_enabled = 1;
      }
      else if(strstr(nmea_field, "NORMAL")) {  // ACTIVE, NORMAL UNLOCK
         pps_enabled = 1;
      }
      else if(strstr(nmea_field, "UNLOCK")) {
         pps_enabled = 1;
      }
      have_pps_enable = 10;
   }
}


void parse_uccm_pullin()
{
char *s;

   nmea_col = 0;
   s = strchr(nmea_msg, '[');
   if(s) {
      if(*s+1) {
         if(strstr(s+1, "UNLIMITED")) pullin_range = 0;
         else                         pullin_range = atoi(s+1);
         have_pullin = 1;
      }
   }
}


char scpi_status_line[128+1];
int scpi_col;
int scpi_tracked, scpi_not_tracked;

void parse_scpi_status()
{
//if(debug_file) fprintf(debug_file, "%s\n", nmea_msg);

   // decode the HIDEIOUS multi-line SCPI STATUS message (that takes three
   // seconds for the receiver to send.

   scpi_status = 1;
   saw_uccm_dmode = 0;
   scpi_tracked = scpi_not_tracked = 0;
   scpi_col = 0;
   scpi_status_line[0] = 0;
   no_uccm_ref = 0;
}


int send_queued_scpi(int why)
{
int id;

   // send the next message in the request queue to the receiver

   if(scpi_in == scpi_out) return 0;
   id = scpi_q[scpi_out].id;

if(debug_file) fprintf(debug_file, "SEND Q %d: sqe:%d  old_id=%d: %s  why:%d\n", scpi_out, scpi_q_entries, scpi_msg_id, scpi_q[scpi_out].msg, why);

   send_scpi_cmd(scpi_q[scpi_out].msg, scpi_q[scpi_out].id);


   if(++scpi_out >= SCPI_Q_SIZE) scpi_out = 0;
   --scpi_q_entries;

   if(rcvr_type == STAR_RCVR) ;
   else if((id == SCPI_TEST_MSG) && (scpi_type != UCCMP_TYPE)) {
      Sleep(20000);
      auto_detect();
//    init_messages(77);
   }
   return 1;
}


void queue_scpi_cmd(char *s, int id)
{
static int last_id = 0;
   // add a command to a queue of messages to send to the receiver.  SCPI 
   // receivers are REALLY dumb and not designed for talking to computer
   // programs.  Their responses to messages have NO indication of what
   // request the response is from.  If responses get out of sync with
   // requests, you a SCREWED!
   if((id == STAR_CONF_MSG) && (last_id == STAR_CONF_MSG)) {
      return;
   }
   last_id = id;


   strcpy(scpi_q[scpi_in].msg, s);
   scpi_q[scpi_in].id = id;
   ++scpi_q_entries;
   if(++scpi_in >= SCPI_Q_SIZE) scpi_in = 0;

if(debug_file) fprintf(debug_file,   "\nQ SCPI id:%d  sqe:%d  in:%d out:%d: %s\n", id, scpi_q_entries, scpi_in,scpi_out, s);
}



void poll_next_scpi()
{
int test;

   // request an item from a SCPI receiver.  Requests are alternated between
   // a time message and some receiver parameter.  The parameter requests are 
   // alternated between one of three important parameters and then the lesser
   // important ones.

   test = 1;
   if(test && (scpi_in != scpi_out)) {  // we have a special message request queued
      send_queued_scpi(1);
   }
   else if(scpi_msg_id == SCPI_TIME_MSG) { // request next misc message every time
      if((test == 0) && (scpi_in != scpi_out)) {  // we have a special message request queued
         send_queued_scpi(1);
      }
      else if(scpi_type == NORTEL_TYPE) {
         if(scpi_req < 0) scpi_req = 0;

         if((scpi_seq % 5) == 0) send_scpi_cmd("PTIME:INTERVAL?",       SCPI_TINT_MSG);    
         else if((scpi_seq % 5) == 2) send_scpi_cmd("ROSC:CONT?",       SCPI_EFC_MSG);
         else {
            if     (scpi_req ==  0) send_scpi_cmd("PTIME:LEAPSECOND?",           SCPI_LEAPTIME_MSG);
            else if(scpi_req ==  1) send_scpi_cmd("PTIME:ACC:LEAPSECOND?",       SCPI_UTC_OFS_MSG);
            else if(scpi_req ==  2) send_scpi_cmd("*IDN?",                       SCPI_ID_MSG);
            else if(scpi_req ==  3) send_scpi_cmd("GPS:POSITION?",               SCPI_POSN_MSG);
            else if(scpi_req ==  4) send_scpi_cmd("GPS:REF:ADELAY?",             SCPI_GET_CABLE_MSG);
            else if(scpi_req ==  5) send_scpi_cmd("GPS:SAT:TRAC:EMANGLE?",       SCPI_ELEV_MSG);
            else if(scpi_req ==  6) send_scpi_cmd("ANTENNA:CONDITION?",          SCPI_ANTENNA_MSG);
            else if(scpi_req ==  7) send_scpi_cmd("STATUS:OPERATION:CONDITION?", SCPI_OPERATION_MSG);
            else if(scpi_req ==  8) send_scpi_cmd("STATUS:HARDWARE:CONDITION?",  SCPI_HARDWARE_MSG);
            else if(scpi_req ==  9) send_scpi_cmd("GPS:POS:SURVEY:PROGRESS?",    SCPI_PROGRESS_MSG);
            else if(scpi_req == 10) send_scpi_cmd("GPS:SAT:TRACKING?",           SCPI_TRACK_MSG);
            else if(scpi_req == 11) send_scpi_cmd("GPS:SAT:VIS:PRED?",           SCPI_VIS_MSG);
            else if(scpi_req == 12) send_scpi_cmd("ROSC:HOLD:DUR?",              SCPI_HOLDOVER_MSG);
            else if(scpi_req == 13) send_scpi_cmd("GPS:SAT:TRAC:IGNORE?",        SCPI_IGNORE_MSG);
            else if(scpi_req == 14) send_scpi_cmd("GPS:SAT:TRAC:INCLUDE?",       SCPI_INCLUDE_MSG);
            else if(scpi_req == 15) send_scpi_cmd("DIAG:LIFETIME:COUNT?",        SCPI_LIFE_MSG);
            else if(scpi_req == 16) send_scpi_cmd("SYNC:FFOM?",                  SCPI_FFOM_MSG);

            if(++scpi_req > 16) scpi_req = 0;
         }
         ++scpi_seq;
      }
      else {
//       queue_scpi_cmd(":DIAG:ROSC:EFC?", SCPI_EFC_MSG);     // we want these as often as possible
//       queue_scpi_cmd(":PTIM:TINT?",     SCPI_TINT_MSG);
//       queue_scpi_cmd(":ROSC:HOLD:TUNC:PRED?",    SCPI_TUNC_MSG);

         if(scpi_req < 0) scpi_req = 0;

         if(     (scpi_seq % 10) == 0) send_scpi_cmd(":ROSC:HOLD:TUNC:PRED?", SCPI_TUNC_MSG);
         else if((scpi_seq % 10) == 1) send_scpi_cmd(":PTIM:TINT?",           SCPI_TINT_MSG);
         else if((scpi_seq % 10) == 2) send_scpi_cmd(":DIAG:ROSC:EFC?",       SCPI_EFC_MSG);
         else {
            if     (scpi_req == 0)  send_scpi_cmd(":GPS:POS:ACT?",            SCPI_POSN_MSG);
            else if(scpi_req == 1)  send_scpi_cmd("*IDN?",                    SCPI_ID_MSG);
            else if(scpi_req == 2)  send_scpi_cmd(":PTIM:GPS:SAT:VIS:PRED?",  SCPI_VIS_MSG);
            else if(scpi_req == 3)  send_scpi_cmd(":PTIM:GPS:SAT:TRAC?",      SCPI_TRACK_MSG);
            else if(scpi_req == 4)  send_scpi_cmd(":GPS:SAT:TRAC:IGN?"      , SCPI_IGNORE_MSG);
            else if(scpi_req == 5)  send_scpi_cmd(":STATUS:OPER:HARD:COND?",  SCPI_HARDWARE_MSG);
            else if(scpi_req == 6)  send_scpi_cmd(":ROSC:HOLD:DUR?",          SCPI_HOLDOVER_MSG);
            else if(scpi_req == 7)  send_scpi_cmd(":DIAG:GPS:UTC?",           SCPI_UTC_MSG);

            else if(scpi_req == 8)  send_scpi_cmd(":PTIM:GPS:EMAN?",          SCPI_ELEV_MSG);
            else if(scpi_req == 9)  send_scpi_cmd(":PTIM:GPS:ADEL?",          SCPI_GET_CABLE_MSG);
            else if(scpi_req == 10) send_scpi_cmd(":PTIM:LEAP:ACC?",          SCPI_UTC_OFS_MSG);
            else if(scpi_req == 11) send_scpi_cmd(":PTIM:GPS:POS:SURV:STAT?", SCPI_SURVEY_MSG);
            else if(scpi_req == 12) send_scpi_cmd(":PTIM:GPS:POS:SURV:PROG?", SCPI_PROGRESS_MSG);
            else if(scpi_req == 13) send_scpi_cmd(":PTIM:LEAP:GPST?",         SCPI_LEAPTIME_MSG);
            else if(scpi_req == 14) send_scpi_cmd(":PTIM:LEAP:DATE?",         SCPI_LEAPTIME_MSG);
            else if(scpi_req == 15) send_scpi_cmd(":PTIM:PPS:EDGE?",          SCPI_EDGE_MSG);
            else if(scpi_req == 16) send_scpi_cmd(":DIAG:LIF:COUNT?",         SCPI_LIFE_MSG);

            if(++scpi_req > 16) scpi_req = 0;
         }
         ++scpi_seq;
      }
   }
   else {
      send_scpi_cmd(":PTIME:TCODE?", SCPI_TIME_MSG);  // request time message
   }
}


void send_scpi_cmd(char *s, int id)
{
int i, j;

   j = strlen(s);
   for(i=0; i<j; i++) {
      send_byte((u08) s[i]);
   }

   if(rcvr_type == STAR_RCVR) {
      send_byte(0x0D);
      star_line = 0;
      star_msg = 0;
   }

   eom_flag = 1;
   if(rcvr_type == STAR_RCVR) {
      send_byte(0x0A);
   }
   else if(scpi_type == NORTEL_TYPE) {
      send_byte(0x0D);
   }
   else if(rcvr_type == UCCM_RCVR) {
      send_byte(0x0D);
   }
   else {
      send_byte(0x0A);
   }

   scpi_msg_id = id;

   if(rcvr_type == STAR_RCVR) ;
   else if(id == SCPI_SET_CABLE_MSG) {  // setting cable delay ties up receiver for several seconds
      Sleep(5000);
   }

   Sleep(200);   // allow time for receiver to digest the command
if(debug_file) fprintf(debug_file,   "\nSEND SCPI id:%d  sqe:%d  in:%d out:%d: %s\n", id, scpi_q_entries, scpi_in,scpi_out, s);
}

void decode_scpi_msg()
{
char c;
int i;

   start_msg_decode(0);

   nmea_msg[0] = 0;
   for(i=0; i<tsip_wptr; i++) {  // copy message from tsip buffer to nmea buffer
      c = tsip_byte();
      nmea_msg[i] = c;
      nmea_msg[i+1] = 0;
      if(i >= ((int)sizeof(nmea_msg)-12)) break;
   }

   tsip_wptr = tsip_rptr = 0;
   tsip_sync = 0;


  // get the response from the reciver and pass it to (HOPEFULLY) the routine
  // that expects it...

  strupr(nmea_msg);

if(debug_file) fprintf(debug_file, "Decode msg %d:[%s]\n", scpi_msg_id, nmea_msg);

   if((nmea_msg[0] == ' ') && (nmea_msg[1] == 'E') && (nmea_msg[2] == '-')) {  // message returned error
      queue_scpi_cmd("*CLS", SCPI_CLS_MSG);  // reset the error
      poll_next_scpi();
      return;
   }
   else if((nmea_msg[0] == ' ') && (nmea_msg[1] == 'S') && (nmea_msg[2] == 'C') && (nmea_msg[3] == 'P')) { // message with no data to return
      poll_next_scpi();
      return;
   }

   if     (scpi_msg_id == SCPI_TIME_MSG)      parse_scpi_time();
   else if(scpi_msg_id == SCPI_VIS_MSG)       parse_scpi_vis();
   else if(scpi_msg_id == SCPI_TRACK_MSG)     parse_scpi_track();
   else if(scpi_msg_id == SCPI_EFC_MSG)       parse_scpi_efc();
   else if(scpi_msg_id == SCPI_ELEV_MSG)      parse_scpi_elev();
   else if(scpi_msg_id == SCPI_GET_CABLE_MSG) parse_scpi_cable();
   else if(scpi_msg_id == SCPI_SET_CABLE_MSG) parse_scpi_cable();
   else if(scpi_msg_id == SCPI_UTC_OFS_MSG)   parse_scpi_utc_ofs();
   else if(scpi_msg_id == SCPI_TINT_MSG)      parse_scpi_tint();
   else if(scpi_msg_id == SCPI_STATUS_MSG)    parse_scpi_status();
   else if(scpi_msg_id == SCPI_PROGRESS_MSG)  parse_scpi_progress();
   else if(scpi_msg_id == SCPI_SURVEY_MSG)    parse_scpi_survey();
   else if(scpi_msg_id == SCPI_RESET_MSG)     parse_scpi_reset();
   else if(scpi_msg_id == SCPI_POSN_MSG)      parse_scpi_posn();
   else if(scpi_msg_id == SCPI_POS_MSG)       parse_scpi_pos();
   else if(scpi_msg_id == SCPI_HOLDOVER_MSG)  parse_scpi_holdover();
   else if(scpi_msg_id == SCPI_UTC_MSG)       parse_scpi_utc();
   else if(scpi_msg_id == SCPI_TUNC_MSG)      parse_scpi_tunc();
   else if(scpi_msg_id == SCPI_TEST_MSG)      parse_scpi_test();
   else if(scpi_msg_id == SCPI_HARDWARE_MSG)  parse_scpi_hardware();
   else if(scpi_msg_id == SCPI_LEAPTIME_MSG)  parse_scpi_leaptime();
   else if(scpi_msg_id == SCPI_ID_MSG)        parse_scpi_id();
   else if(scpi_msg_id == SCPI_IGNORE_MSG)    parse_scpi_ignore();
   else if(scpi_msg_id == SCPI_IGN_COUNT_MSG) parse_scpi_ign_count();
   else if(scpi_msg_id == SCPI_INCLUDE_MSG)   parse_scpi_include();
   else if(scpi_msg_id == SCPI_JAMSYNC_MSG)   parse_scpi_jamsync();
   else if(scpi_msg_id == SCPI_EDGE_MSG)      parse_scpi_edge();
   else if(scpi_msg_id == SCPI_LIFE_MSG)      parse_scpi_life();
   else if(scpi_msg_id == SCPI_POWER_MSG)     parse_scpi_power();
   else if(scpi_msg_id == SCPI_USER_MSG)      parse_scpi_user();
   else if(scpi_msg_id == SCPI_ANTENNA_MSG)   parse_scpi_antenna();
   else if(scpi_msg_id == SCPI_FFOM_MSG)      parse_scpi_ffom();
   else if(scpi_msg_id == SCPI_OPERATION_MSG) parse_scpi_operation();
//   else if(scpi_msg_id == UCCM_RATE_MSG)      parse_uccm_rate();

   poll_next_scpi();
}


int xxyyzz;

void parse_scpi_azel()
{
int n;
int prn, aaa,eee,sss;
char *s;

   // process the SYST:STAT? az/el info line

   if(no_uccm_ref) {
      gps_status = 0x08;
      have_gps_status = 1;
      minor_alarms |= 0x0008;
      have_tracking = 1;
   }
   else if(rcvr_type == UCCM_RCVR) {
      have_gps_status = 0;
      minor_alarms &= (~0x0008);
      have_tracking = 1;

      if(strstr(scpi_status_line, "MODE")) {
         s = strstr(scpi_status_line, "SURVEY");
         if(s) {
            rcvr_mode = RCVR_MODE_SURVEY;
            minor_alarms |= (0x0020);
            sscanf(s+9, "%d", &survey_progress);
         }
         else if(strstr(scpi_status_line, "HOLD")) {
            rcvr_mode = RCVR_MODE_HOLD;
            minor_alarms &= (~0x0020);
         }
      }
   }

   while(1) {  // convert '*' (attempting to track) to ' '
      s = strchr(scpi_status_line, '*');
      if(s == 0) break;
      *s = ' ';
   }
   scpi_status_line[17] = 0; // break line between TRACKED and NOT-TRACKED areas

   prn = aaa = eee = sss = (-1);
   n = sscanf(scpi_status_line, "%d %d %d %d", &prn, &eee,&aaa,&sss);
   if((n == 4) && (prn >= 1) && (prn <= MAX_PRN) && (aaa >= 0) && (eee >= 0) && (sss >= 0)) {  // nnnnnnn - 32
      sat[prn].azimuth = (float) aaa;
      set_sat_el(prn, (float) eee);
      have_sat_azel = 10;

      if(adjust_scpi_ss) {  // scale sig strength level to 0..50
         if(sss > 30) {
            sss = sss - 30;
            sss = 30 + (sss/10);
         }
         level_type = "SIG";
      }
      else level_type = "C/N";

      sat[prn].sig_level = ((float) sss);
      sat[prn].level_msg = 15;
      ++scpi_tracked;
if(rcvr_type == UCCM_RCVR) sat[prn].tracking = 1; // uuuuuu
      record_sig_levels(prn);
   }

   if(rcvr_type == UCCM_RCVR) {  // get not-tracked info
      prn = aaa = eee = (-1);
      sss = 0;
      n = sscanf(&scpi_status_line[17+1], "%d %d %d", &prn, &eee,&aaa);
      if((n == 3) && (prn >= 1) && (prn <= MAX_PRN) && (aaa >= 0) && (eee >= 0)) { // nnnnnnn - 32
         sat[prn].azimuth = (float) aaa;
         set_sat_el(prn, (float) eee);
         have_sat_azel = 11;

         if(adjust_scpi_ss) {  // scale sig strength level to 0..50
            if(sss > 30) {
               sss = sss - 30;
               sss = 30 + (sss/10);
            }
            level_type = "SIG";
         }
         else level_type = "C/N";

         sat[prn].sig_level = ((float) sss);
         sat[prn].level_msg = 16;
         sat[prn].tracking = (-1); // uuuuuu
         ++scpi_not_tracked;
         record_sig_levels(prn);
      }
   }
}

void get_scpi_status(u08 c)
{
   if(c == 0x0D) return;

   if(c == 0x0A) {  // end-of-line, process it
if(debug_file) fprintf(debug_file, "%d: %s\n", scpi_status, scpi_status_line);

      strupr(scpi_status_line);
      if(strstr(scpi_status_line, "SELF TEST")) {
         scpi_status = 0;
if(debug_file) fflush(debug_file);
      }
      else if(strstr(scpi_status_line, "PRN") && strstr(scpi_status_line, "EL")) {  // tracking info follows
         if(strstr(scpi_status_line, "SS")) adjust_scpi_ss = 1;
         else                               adjust_scpi_ss = 0;
         scpi_status = 2;
         if(rcvr_type == UCCM_RCVR) {
            reset_sat_tracking();
         }
      }
      else if(strstr(scpi_status_line, "ELEV") && strstr(scpi_status_line, "MASK")) {  // end of tracking info
         scpi_status = 4; // skip lines until we see the "SELF TEST" or "------------" end-of-status line
      }
      if(scpi_status == 2) {
         parse_scpi_azel();
      }

      scpi_col = 0;
      scpi_status_line[0] = 0;
   }
   else {
      if(scpi_col < ((int)sizeof(scpi_status_line)-2)) {
         scpi_status_line[scpi_col++] = c;
         scpi_status_line[scpi_col] = 0;
      }
   }
}

void get_scpi_message()
{
u08 c;
static int row=0;

   // This routine buffers up an incoming SCPI message.  When the end of the
   // message is seen, the message is parsed and decoded with decode_scpi_msg()

   if(SERIAL_DATA_AVAILABLE() == 0) {
      check_com_timer();
      return;
   }
   else {
      reset_com_timer();
   }


   c = get_com_char();
   if(scpi_status) {   // we are reading the SYST:STAT? results
      get_scpi_status(c);
      return;
   }

   if(rcv_error) {      // parity/framing/overrun errors
      rcv_error = 0;
//!!!!!qqqq    goto rst_msg;
   }

   if(tsip_sync == 0) {    // syncing to start of message, search for a '$'
      if(c == '>') {
         tsip_sync = 1;
         get_sync_time();
      }
      tsip_wptr = 0;
      tsip_rptr = 0;
      return;
   }
   else if(tsip_sync == 1) { // '>' has been seen, now build the message
      if(c == 0x0D) goto scpi_end; 
      else if(c == 0x0A) goto scpi_end; 
      else if(c == '>')  {  // null response to SCPI command
         decode_scpi_msg();
         tsip_sync = 1; 
         tsip_rptr = 0;
         tsip_wptr = 0;
         return;
      }

      if(tsip_wptr < MAX_TSIP) {  // so add it to the message buffer
         tsip_buf[tsip_wptr++] = c;
      }
   }
   else {
      scpi_end:
      decode_scpi_msg();
      tsip_sync = 0;
   }
}

//
//
//   UCCM receiver stuff
//
//

void parse_uccm_loop()
{
   // prepare to decode the multi-line UCCM LOOP status message 

   uccm_loop = 1;
   loop_fmt = 0;   // assume Trimble format
   scpi_col = 0;
   scpi_status_line[0] = 0;
}


void poll_next_uccm()
{
int test;

   // request an item from a UCCM receiver.  Requests are alternated between
   // a time message and some receiver parameter.  The parameter requests are 
   // alternated between one of three important parameters and then the lesser
   // important ones.

   test = 0;
   if((test == 0) && (scpi_in != scpi_out)) {  // we have a special message request queued
      send_queued_scpi(1);
   }
   else {
      if(scpi_req < 0) scpi_req = 0;

      if(     (scpi_seq % 3) == 0) send_scpi_cmd("SYNC:TINT?",       SCPI_TINT_MSG);
//qqqqelse if((scpi_seq % 3) == 1) send_scpi_cmd("DIAG:LOOP?",       UCCM_LOOP_MSG); // ggggggg
      else {
         if     (scpi_req == 0)  send_scpi_cmd("DIAG:ROSC:EFC:REL?",  SCPI_EFC_MSG);
         else if(scpi_req == 1)  send_scpi_cmd("DIAG:ROSC:EFC:DATA?", UCCM_EFC_MSG);
         else if(scpi_req == 2)  send_scpi_cmd("GPS:POS?",            SCPI_POSN_MSG);
         else if(scpi_req == 3)  send_scpi_cmd("*IDN?",               SCPI_ID_MSG);
         else if(scpi_req == 4)  send_scpi_cmd("GPS:SAT:TRAC:IGN?",   SCPI_IGNORE_MSG);
         else if(scpi_req == 5)  send_scpi_cmd("GPS:SAT:TRAC:EMAN?",  SCPI_ELEV_MSG);
         else if(scpi_req == 6)  send_scpi_cmd("GPS:REF:ADEL?",       SCPI_GET_CABLE_MSG);
         else if(scpi_req == 7)  send_scpi_cmd("OUTP:TP:SEL?",        UCCM_RATE_MSG);
         else if(scpi_req == 8)  send_scpi_cmd("OUTP:STAT?",          UCCM_STATE_MSG);
         else if(scpi_req == 9)  send_scpi_cmd("LED:GPSL?",           UCCM_LED_MSG);

         if(scpi_type != UCCMP_TYPE) {  // kkkkkk
            if(++scpi_req > 9) scpi_req = 0;
         }
         else {
            if     (scpi_req == 9)  send_scpi_cmd(":ROSC:HOLD:DUR?",     SCPI_HOLDOVER_MSG); //kkkkkk
            else if(scpi_req == 10) send_scpi_cmd(":GPS:POS:SURV:STAT?", SCPI_SURVEY_MSG);   //kkkkkk
            else if(scpi_req == 11) send_scpi_cmd(":GPS:POS:SURV:PROG?", SCPI_PROGRESS_MSG); //kkkkkk
            if(++scpi_req > 11) scpi_req = 0;
         }

      }
      ++scpi_seq;
   }
}

void parse_uccm_time(int why)
{
int vals[50];
int i,j;
double jd, jd0;
char *s;

   j = 0;
   sent_uccm_status = 0;
   s = strstr(nmea_msg, "C5 ");

if(debug_file) fprintf(debug_file, "uccm time %d loop:%d  len:%d: [%s]\n", why, have_uccm_loop, (int) strlen(nmea_msg), nmea_msg);
//if(raw_file) fprintf(raw_file, "uccm time %d len:%d: [%s]\n", why, (int) strlen(nmea_msg), nmea_msg);
   if(s == 0) return;


   for(i=0; i<131; i+=3) {  // get the values from the time line
      sscanf(&s[i], "%02X", &vals[j]);
      j++;
   }

   if(!user_set_utc_ofs && vals[32]) {  // leap second offset from UTC
      utc_offset = vals[32];
      check_utc_ofs(-111);
   }

   // vals[33]: 40=PPS validity?  41:phase settling  50:pps invalid?
   //           60:stable  62:stable, leap pending?
   // on power up: 41 -> 43 -> 63 -> 60/62 (62=leap pending?) Trimble
   // on power up: 41 -> 43 -> 60 -> 62 (62=leap pending?)    Trimble UCCM-P
   if(1) {
      have_leap_info = 8;
      if(vals[33] & 0x02) {  // leap pending flag?
         minor_alarms |= 0x0080;
      }
      else {
         minor_alarms &= (~0x0080);
      }
   }


   // vals[34]: 04=normal 0C=antenna open/shorted  06=normal?
   // on power up: 00 -> 04
   // disconnect antenna: 04 -> 0C
   // reconnect antenna   0C -> 04


   // vals[35]: 8F=FFOM >0/settling/no antenna    85:FFOM 0,locked? - Symmetricom UCCMP
   // on power up: 8F -> 85           Symmetricom
   // disconnect antenna: 85 -> 8F    Symmetricom
   // reconnect antenna:  8F -> 85    Symmetricom
   //
   // vals[35]: 41=power up  4F=FFOM >0/settling/no antenna    45:FFOM 0,locked? - Trimble
   // on power up: 4F -> 45           Trimble UCCM-P
   // on power up: 41 -> 4F -> 45     Trimble UCCM
   // on antenna disconnect 45 -> 4F  Trimble
   // on antenna connect    4F -> 45 


   // vals[36]: 40=have date?/normal  50/60:date invalid?/no antenna
   // power up 50 -> 40                   Symmetricom 
   // disconnect antenna: 40 -> 50 -> 60  Symmetricom 
   // reconnect antenna:  60 -> 50 -> 40  Symmetricom 
   //
   // vals[36]: 80=have date?/normal  90:date invalid?/no antenna
   // power up 90 -> 80                   Trimble 
   // disconnect antenna: 80 -> 90        Trimble UCCM-P
   // reconnect antenna:  90 -> 80        Trimble UCCM-P

   jd = jd0 = (double) ((vals[27] * (256*256*256)) + (vals[28]*(256*256)) + (vals[29]*(256)) + vals[30]);

   if(timing_mode) {  // we want UTC time - convert GPS to UTC
      jd -= (double) utc_offset;
   }

   jd /= (24.0*60.0*60.0);
   jd += GPS_EPOCH;

   gregorian(jd);

   pri_year = year = g_year;
   pri_month = month = g_month;
   pri_day = day = g_day;
   pri_hours = hours = g_hours;
   pri_minutes = minutes = g_minutes;
   pri_seconds = seconds = g_seconds;
   pri_frac = raw_frac = g_frac;

   fake_time_stamp = 0;
   if(1) {  // !!!!! fake an odd second time message
      gps_week = (int) ((jd0-1.0) / (7.0*24.0*60.0*60.0));
////  if(have_week == 0) need_redraw = 2053;  // ggggg
      have_week = 9;

////  tow = pri_tow = this_tow = survey_tow = (int) (jd - ((double) gps_week) * (7.0*24.0*60.0*60.0));
      tow = pri_tow = this_tow = survey_tow = (int) ((jd0-1.0) - ((double) gps_week) * (7.0*24.0*60.0*60.0));
      have_tow = 9;

      fake_missing_second(1.0);
   }

   gps_week = (int) (jd0 / (7.0*24.0*60.0*60.0));
   if(have_week == 0) need_redraw = 2054;
   have_week = 9;

//   tow = pri_tow = this_tow = survey_tow = (int) (jd - ((double) gps_week) * (7.0*24.0*60.0*60.0));
   tow = pri_tow = this_tow = survey_tow = (int) (jd0 - ((double) gps_week) * (7.0*24.0*60.0*60.0));
   have_tow = 9;

//if(log_file) fprintf(log_file, "uccm1 jd:%f  week:%d  tow:%d  %04d/%02d/%02d  %02d:%02d:%02d  %02X:%02X:%02X:%02X\n", 
//jd0,gps_week,tow,year,month,day,hours,minutes,seconds, vals[27],vals[28],vals[29],vals[30]);  //kkkkk
//if(raw_file) fprintf(raw_file, "uccm1 jd:%f  week:%d  tow:%d  %04d/%02d/%02d  %02d:%02d:%02d  %02X:%02X:%02X:%02X\n", 
//jd0,gps_week,tow,year,month,day,hours,minutes,seconds, vals[27],vals[28],vals[29],vals[30]);  //kkkkk

   update_gps_screen(555);
   if(sent_uccm_status) ;
   else if(pri_seconds == SCPI_STATUS_SECOND) {
      queue_scpi_cmd("SYST:STAT?", UCCM_STATUS_MSG);
   }
}

int uccm_time_line(int why)
{

   // this routine handles the case where the Symmetricom units send a 
   // time code packet in the middle of another message's response
   
   if(scpi_status_line[0] != 'C') return 0;  // line is not a time code line
   if(scpi_status_line[1] != '5') return 0;

   strcpy(nmea_msg, scpi_status_line);  // prepare to decode the time code line
   parse_uccm_time(why);                   // decode it

   scpi_col = 0;              // prepare to get the next line
   scpi_status_line[0] = 0;

poll_next_uccm();  //kkkkkk
   return 1;
}

int uccm_msg_id = 0;

void decode_uccm_msg(int why)
{
char c;
int i;

   start_msg_decode(0);

   nmea_msg[0] = 0;
   for(i=0; i<tsip_wptr; i++) {  // copy message from tsip buffer to nmea buffer
      c = tsip_byte();
      nmea_msg[i] = c;
      nmea_msg[i+1] = 0;
      if(i >= ((int)sizeof(nmea_msg)-12)) break;
   }

   tsip_wptr = tsip_rptr = 0;
   tsip_sync = 0;

   // get the response from the receiver and pass it to (HOPEFULLY) the routine
   // that expects it...

   strupr(nmea_msg);

if(debug_file) fprintf(debug_file, "Decode uccm msg %d:%s\n", uccm_msg_id, nmea_msg);

   if(strstr(nmea_msg, "UCCM-P")) {  // UCCM error response?
      uccm_msg_id = 0;
      return;
   }
   else if(strstr(nmea_msg, "UCCM")) {  // UCCM error response?
      uccm_msg_id = 0;
      return;
   }
   else if(strstr(nmea_msg, "COMMAND COMPLETE")) {
      uccm_msg_id = 0;
      return;
   }
   else if(strstr(nmea_msg, "COMMAND ERROR")) {
      uccm_msg_id = 0;
      return;
   }
   else if(strstr(nmea_msg, "UNDEFINED HEADER")) {
      uccm_msg_id = 0;
      return;
   }
   else if(strstr(nmea_msg, "INVALID PARAMETER")) {
      uccm_msg_id = 0;
      return;
   }
   else if(strstr(nmea_msg, "CORRUPT")) {  // "Data corrupt or stale" uccmp
      uccm_msg_id = 0;
      return;
   }
   else if((nmea_msg[0] == 'C') && (nmea_msg[1] == '5')) {  // time string 132 bytes (44 values) long
      parse_uccm_time(123);

      uccm_time:
if(have_uccm_loop) {
   send_scpi_cmd("DIAG:LOOP?",       UCCM_LOOP_MSG);  // qqqqq gggggggg
   poll_next_uccm();
}
if(1 || (scpi_type != UCCMP_TYPE)) {  //kkkkkkkkkkkkk
poll_next_uccm();
poll_next_uccm();
poll_next_uccm();
//poll_next_uccm();
//poll_next_uccm();
//poll_next_uccm();
}
      return;
   }
   else if(uccm_msg_id == 0) {
if(debug_file) fprintf(debug_file, "response to:%s (status msg:%d) is next\n", nmea_msg, scpi_status);
      if(strstr(nmea_msg, "GPS:POS?")) {
         uccm_msg_id = SCPI_POSN_MSG;
      }
      else if(strstr(nmea_msg, "SYNC:TINT?")) {
         uccm_msg_id = SCPI_TINT_MSG;
      }
      else if(strstr(nmea_msg, "DIAG:ROSC:EFC:REL?")) {
         uccm_msg_id = SCPI_EFC_MSG;
      }
      else if(strstr(nmea_msg, "LED:GPSL?")) {
         uccm_msg_id = UCCM_LED_MSG;
      }
      else if(strstr(nmea_msg, "*TST?")) {
         uccm_msg_id = SCPI_TEST_MSG;
      }
      else if(strstr(nmea_msg, ":GPS:POS:SURV:STAT?")) {
         uccm_msg_id = SCPI_SURVEY_MSG;
      }
      else if(strstr(nmea_msg, ":GPS:POS:SURV:PROG?")) {
         uccm_msg_id = SCPI_PROGRESS_MSG;
      }
      else if(strstr(nmea_msg, ":GPS:POS:SURV:STAT ONCE")) {
         uccm_msg_id = SCPI_SURVEY_MSG;
      }
      else if(strstr(nmea_msg, "ROSC:HOLD:DUR?")) {
         uccm_msg_id = SCPI_HOLDOVER_MSG;
      }
      else if(strstr(nmea_msg, "SYNC:IMM")) {
         uccm_msg_id = SCPI_JAMSYNC_MSG;
      }
      else if(strstr(nmea_msg, "DIAG:ROSC:EFC:DATA?")) {
         uccm_msg_id = UCCM_EFC_MSG;
      }
      else if(strstr(nmea_msg, "DIAG:LOOP?")) {
         uccm_msg_id = SCPI_EFC_MSG;
         parse_uccm_loop();
      }
      else if(strstr(nmea_msg, "GPS:SAT:TRAC:EMAN?")) {
         uccm_msg_id = SCPI_ELEV_MSG;
      }
      else if(strstr(nmea_msg, "GPS:REF:ADEL?")) {
         uccm_msg_id = SCPI_GET_CABLE_MSG;
      }
      else if(strstr(nmea_msg, "GPS:REF:ADEL ")) {
         uccm_msg_id = SCPI_SET_CABLE_MSG;
      }
      else if(strstr(nmea_msg, "PULLINRANGE?")) {
         uccm_msg_id = UCCM_GET_PULLIN_MSG;
      }
      else if(strstr(nmea_msg, "PULLINRANGE")) {
         uccm_msg_id = UCCM_SET_PULLIN_MSG;
      }
      else if(strstr(nmea_msg, "OUTP:TP:SEL?")) {
         uccm_msg_id = UCCM_RATE_MSG;
      }
      else if(strstr(nmea_msg, "OUTP:STAT?")) {
         uccm_msg_id = UCCM_STATE_MSG;
      }
      else if(strstr(nmea_msg, "*IDN?")) {
         uccm_msg_id = SCPI_ID_MSG;
      }
      else if(strstr(nmea_msg, "SYST:STAT?")) {
         uccm_msg_id = UCCM_STATUS_MSG;
         parse_scpi_status();
      }
      else if(strstr(nmea_msg, "C5")) {  //kkkkkkkkk
         uccm_msg_id = 0;
         parse_uccm_time(144);
         goto uccm_time;
      }
      return;
   }

if(debug_file) fprintf(debug_file, "uccm %d response:%s\n", uccm_msg_id, nmea_msg);
   if(uccm_msg_id == SCPI_POSN_MSG) {
      parse_scpi_posn();
   }
   else if(uccm_msg_id == SCPI_TINT_MSG) {
      parse_scpi_tint();
   }
   else if(uccm_msg_id == SCPI_EFC_MSG) {
      parse_scpi_efc();
   }
   else if(uccm_msg_id == UCCM_LED_MSG) {
      parse_uccm_led();
   }
   else if(uccm_msg_id == UCCM_EFC_MSG) {
      parse_uccm_efc();
   }
   else if(uccm_msg_id == SCPI_TEST_MSG) {
      parse_scpi_test();
   }
   else if(uccm_msg_id == SCPI_ELEV_MSG) {
      parse_scpi_elev();
   }
   else if(uccm_msg_id == SCPI_GET_CABLE_MSG) {
      parse_scpi_cable();
   }
   else if(uccm_msg_id == SCPI_SET_CABLE_MSG) {
      parse_scpi_cable();
   }
   else if(uccm_msg_id == SCPI_ID_MSG) {
      parse_scpi_id();
   }
   else if(uccm_msg_id == SCPI_SURVEY_MSG) {
      parse_scpi_survey();
   }
   else if(uccm_msg_id == SCPI_PROGRESS_MSG) {
      parse_scpi_progress();
   }
   else if(uccm_msg_id == SCPI_JAMSYNC_MSG) {
      parse_scpi_jamsync();
   }
   else if(uccm_msg_id == SCPI_HOLDOVER_MSG) {
      parse_scpi_holdover();
   }
   else if(uccm_msg_id == UCCM_RATE_MSG) {
      parse_uccm_rate();
   }
   else if(uccm_msg_id == UCCM_STATE_MSG) {
      parse_uccm_state();
   }
   else if(uccm_msg_id == UCCM_GET_PULLIN_MSG) {
      parse_uccm_pullin();
   }
   else if(uccm_msg_id == SCPI_STATUS_MSG) {
   }
   else if(uccm_msg_id == UCCM_LOOP_MSG) {
   }

   uccm_msg_id = 0;
}


void get_uccm_status(u08 c)
{
char *s;

   if(c == 0x0A) return;
   if(c == 0x00) return;

   if(c == 0x0D) {  // end-of-line,  process it
if(debug_file) fprintf(debug_file, "get uccm status:%d: %s\n", scpi_status, scpi_status_line);
      strupr(scpi_status_line);
      if(uccm_time_line(1)) return;  // time code message in middle of the status message

      if(strstr(scpi_status_line, "SETTLING")) {
         discipline_mode = 10;
         saw_uccm_dmode = 1;
      }
      if(strstr(scpi_status_line, "WARMUP")) {
         discipline_mode = 10;
         saw_uccm_dmode = 2;
      }

      if(strstr(scpi_status_line, "COMMAND COMPLETE")) {
//          scpi_status = 0;
if(debug_file) fflush(debug_file);
      }
      else if(strstr(scpi_status_line, "TFOM")) {
         s = strstr(scpi_status_line, "TFOM");
         if(s) {
            sscanf(s+4, "%d", &tfom);
            have_tfom = 3;
         }
         s = strstr(scpi_status_line, "FFOM");
         if(s) {
            sscanf(s+4, "%d", &ffom);
            have_ffom = 3;
         }
      }
      else if(strstr(scpi_status_line, "NO REF")) {
         no_uccm_ref = 1;
         rcvr_mode = RCVR_MODE_ACQUIRE;  // !!!! RCVR_MODE_NO_SATS
      }
      else if(strstr(scpi_status_line, "WAIT FOR GPS")) {
         no_uccm_ref = 1;
         rcvr_mode = RCVR_MODE_ACQUIRE;
      }
      else if(strstr(scpi_status_line, "PRN") && strstr(scpi_status_line, "EL")) {  // tracking info follows
         if(strstr(scpi_status_line, "SS")) adjust_scpi_ss = 1;
         else                               adjust_scpi_ss = 0;
         scpi_status = 2;
         if(rcvr_type == UCCM_RCVR) {
            reset_sat_tracking();
         }
      }
      else if(strstr(scpi_status_line, "ELEV") && strstr(scpi_status_line, "MASK")) {  // end of tracking info
         scpi_status = 4; // skip lines until we see the COMMAND COMPLETE
         have_count = 44;
         sat_count = scpi_tracked + scpi_not_tracked;
         config_sat_count(sat_count);
      }
      else if((scpi_status == 4) && strstr(scpi_status_line, "------")) {
         scpi_status = 0;
      }
      if(scpi_status == 2) {  // we are in the az/el area, decode it
         parse_scpi_azel();
      }

      scpi_col = 0;
      scpi_status_line[0] = 0;
   }
   else {  // accumulate the status line
      if(scpi_col < ((int)sizeof(scpi_status_line)-2)) {
         scpi_status_line[scpi_col++] = c;
         scpi_status_line[scpi_col] = 0;
      }
// if(debug_file) fprintf(debug_file, "sline col:%d c:%d: stat:%d [%s]\n", scpi_col, c, scpi_status, scpi_status_line);
   }
}

#define NUM_DIFFS 30
double fdifs[NUM_DIFFS+2];

void decode_uccm_temp()
{
char *s;

   s = strstr(scpi_status_line, "=");   //kkkkk
   if(s) {
      temperature = (float) atof(s+1);
temperature *= 1.0E12F;
//    have_temperature = 1;
   }
}

void decode_uccm_loop()
{
float dac_link;
float dac_avg;
float dac_gps;    // same as dac_link?
float freq_corr;  // always 0.0?
float link_off;   // changes
float freq_diff;  // changes
float freq_frac;  // 0 during recovery
char *s;

if(debug_file) fprintf(debug_file, "decode_uccm_loop fmt:%d: %s\n", loop_fmt, scpi_status_line);

   freq_diff = 999.0;

   if(loop_fmt != 0) {  // Symmetricom
      s = strstr(scpi_status_line, "=");
      if(s) {
         sscanf(s+1, "%f", &freq_diff);
//freq_diff /= 1000.0;  // kkkkkk
//ppt_string = " ppb";
      }
   }
   else {  // Trimble
      sscanf(scpi_status_line, "%g %g %g %g %g %g %g", 
         &dac_link, &dac_avg, &dac_gps, &freq_corr, &link_off, &freq_diff, &freq_frac);

      if(saw_uccm_dmode && (discipline_mode == 10)) ;
      else if((discipline_mode == 0) && (freq_corr != 0.0F)) {
         discipline_mode = 3;
      }
      else if(discipline_mode && (freq_corr == 0.0F)) {
         discipline_mode = 0;
      }
   }

   if(freq_diff < (-2.00E-7)) ;      // bogus value -2.79E-7 shows up occasionally on Trimble
   else if(freq_diff > (2.00E-7)) ;  // bogus value shows up occasionally on Trimble
   else {
      osc_offset = (double) (freq_diff*1.0E9);
      have_osc_offset = 10;
   }

//sprintf(plot_title, "%g %g %g %g %g %g %g", 
//dac_link, dac_avg, dac_gps, freq_corr, link_off, freq_diff, freq_frac);
}

void get_uccm_loop(u08 c)
{
   if(c == 0x0A) return;
   if(c == 0x00) return;

   if(c == 0x0D) {  // end-of-line,  process it
if(debug_file) fprintf(debug_file, "get_uccm_loop %d: %s\n", uccm_loop, scpi_status_line);
      strupr(scpi_status_line);
      if(uccm_time_line(2)) return;  // time code message in middle of the loop message

      if(strstr(scpi_status_line, "COMMAND COMPLETE")) {
         uccm_loop = 0;
if(debug_file) fflush(debug_file);
      }
      else if(strstr(scpi_status_line, "DAC")) {  // loop info header line
         uccm_loop = 2;
      }
      else if(strstr(scpi_status_line, "----")) {  // Symmetricom loop info header line
         uccm_loop = 2;
         loop_fmt = 1;
         if(scpi_type != UCCMP_TYPE) plot[TEMP].show_plot = 1;
         scpi_type = UCCMP_TYPE;
         config_msg_ofs();
      }
      else if(uccm_loop == 2) { // skip the header line(s)
         if(0 || strstr(scpi_status_line, "SERIAL NUMBER")) { // we got syst:stat
            have_uccm_loop = 0;
            uccm_loop = 4;
         }
         else if(loop_fmt != 0) {  // Symmetricom
            if(strstr(scpi_status_line, "FREQ COR")) {  // the payload line we want
               decode_uccm_loop();
            }
            else if(strstr(scpi_status_line, "TEMP COR")) {  // the payload line we want
               decode_uccm_temp();
            }
         }
         else uccm_loop = 3;    // next line has the goodies
      }
      else if(uccm_loop == 4) {  // skip to Command Complete
      }

      if(uccm_loop == 3) {  // we are in the info data area, decode it
         decode_uccm_loop();
      }

      scpi_col = 0;
      scpi_status_line[0] = 0;
   }
   else {  // accumulate the status line
      if(scpi_col < ((int)sizeof(scpi_status_line)-2)) {
         scpi_status_line[scpi_col++] = c;
         scpi_status_line[scpi_col] = 0;
      }
// if(debug_file) fprintf(debug_file, "info_line col:%d c:%d: stat:%d [%s]\n", scpi_col, c, uccm_loop, scpi_status_line);
   }
}



void get_uccm_message()
{
u08 c;
static int row=0;

   // This routine buffers up an incoming UCCM message.  When the end of the
   // message is seen, the message is parsed and decoded with decode_uccm_msg()

   if(SERIAL_DATA_AVAILABLE() == 0) {
      check_com_timer();
      return;
   }
   else {
      reset_com_timer();
   }

   c = get_com_char();
   if(scpi_status) {    // we are reading the SYST:STAT? results
      get_uccm_status(c);
      return;
   }
   else if(uccm_loop) {  // we are reading the UCCM LOOP results
      get_uccm_loop(c);
      return;
   }

   if(rcv_error) {      // parity/framing/overrun errors
      rcv_error = 0;
//!!!!!qqqq    goto rst_msg;
   }

   if(tsip_sync == 0) {    // syncing to start of message, search for a '>'
      tsip_wptr = 0;
      tsip_rptr = 0;
      if(c == '>') {
         tsip_sync = 1;
         get_sync_time();
      }
      else if(c == 'C') {
         tsip_sync = 2; // maybe the time field?
         get_sync_time();
      }
      else {
         tsip_sync = 1;
         get_sync_time();
      }
      return;
   }
   else if(tsip_sync == 1) { // '>' has been seen, now build the message
      if(c == 0x0D) goto uccm_end; 
      else if(c == 0x0A) goto uccm_end; 
      else if(c == '>')  {  // null response to UCCM command
         decode_uccm_msg(1);
         tsip_sync = 1; 
         tsip_rptr = 0;
         tsip_wptr = 0;
         return;
      }

      if(tsip_wptr < MAX_TSIP) {  // so add it to the message buffer
         tsip_buf[tsip_wptr++] = c;
      }
   }
   else if(tsip_sync == 2) {  // C5 timecode message
      if(c == '5') {
         tsip_buf[tsip_wptr++] = 'C';
         tsip_buf[tsip_wptr++] = '5';
         tsip_sync = 1;
      }
      else tsip_sync = 0;
   }
   else {
      uccm_end:
      decode_uccm_msg(2);
      tsip_sync = 0;
   }
}


//
//
//  Acron Zeit receiver stuff
//
//

void send_acron_byte(u08 c)
{
   // send string to the serial port...  send each char to the serial port
   // wait for ACRON_RCVR to echo input char, eat it, then wait 
   // at least 10 msecs
//if(debug_file) fprintf(debug_file, "send:%02X\n", c);

   eom_flag = 1;
   sendout(c);

   while(SERIAL_DATA_AVAILABLE() == 0) {  // wait for char to echo
      check_com_timer();
      if(com_data_lost) return;
   }

   c = get_com_char();  // eat that pesky echoed character
//if(debug_file) fprintf(debug_file, "eat:%02X\n", c);
   Sleep(20);
}

void send_acron_cmd(char *s)
{
int i, j;

   if(s == 0) return;
   
   j = strlen(s);
   if(j) this_acron_cmd = toupper(s[j-1]);
   else  this_acron_cmd = 0;
//if(debug_file) fprintf(debug_file, "send acron cmd:[%s]  len:%d  cmd:%c\n", s, j, this_acron_cmd);

   for(i=0; i<j; i++) {
      send_acron_byte((u08) s[i]);
   }

   send_acron_byte((u08) 0x0D);

//if(debug_file) fprintf(debug_file, "this_acron_cmd: %c\n", this_acron_cmd);

   if(FAKE_ACRON_RCVR) {
      if(this_acron_cmd == 'O') {
         get_clock_time();   // get system clock time in UTC
         sprintf(out, "%02d%02d%02d6%02d%02d%02d47\r", clk_hours,clk_minutes,clk_seconds,
         clk_day,clk_month,clk_year-2000);
         strcpy((char *) &rcvr_buf[0], (char *) out);
         next_serial_byte = 0;
         rcvr_byte_count = 16;
//if(debug_file) fprintf(debug_file, "stuff serial buf:%c  avail:%d  s:%d\n", this_acron_cmd, SERIAL_DATA_AVAILABLE(), rcvr_buf);
      }
      else if(this_acron_cmd == 'G') {
         sprintf(out, "1%c\r", '0'+(pri_minutes%6));
         strcpy((char *) &rcvr_buf[0], (char *) out);
         next_serial_byte = 0;
         rcvr_byte_count = 3;
//if(debug_file) fprintf(debug_file, "stuff serial buf:%c  avail:%d  s:%d\n", this_acron_cmd, SERIAL_DATA_AVAILABLE(), rcvr_buf);
      }
      else if(this_acron_cmd == 'H') {  // receiver sync request
      }
   }
}


void init_acron_time()
{
   // set UTC mode, leap year flag
   // clear leap second and dst flags

   get_clock_time();
   if(leap_year(clk_year)) {
      send_acron_cmd("88c");
   }
   else {
      send_acron_cmd("80c");
   }
}


void parse_acron_time()
{
int dow;
int leap;
int status;
static int last_sec = 99;

   nmea_col = 0;

   get_scpi_field(2);
   hours = pri_hours = atoi(nmea_field);
   get_scpi_field(2);
   minutes = pri_minutes = atoi(nmea_field);
   get_scpi_field(2);
   seconds = pri_seconds = atoi(nmea_field);
   pri_frac = 0.0;

   get_scpi_field(1);
   dow = atoi(nmea_field);

   get_scpi_field(2);
   day = pri_day = atoi(nmea_field);
   get_scpi_field(2);
   month = pri_month = atoi(nmea_field);
   get_scpi_field(2);
   year = pri_year = atoi(nmea_field) + 2000;

if(debug_file) fprintf(debug_file, "acron time(%s) -> %02d/%02d/%02d %02d:%02d:%02d\n", nmea_msg, year,month,day,hours,minutes,seconds);

   get_scpi_field(1);
   leap = atoi(nmea_field);

   if(leap & 0x04) {  // leap pending
      minor_alarms |= 0x0080;
   }
   else {
      minor_alarms &= (~0x0080);
   }
   have_leap_info = 33;

   get_scpi_field(1);
   status = atoi(nmea_field);

   if(timing_mode == 0) {     // convert utc time to gps time
      utc_to_gps();
   }
   else {
      adjust_rcvr_time(0.0);  // incorporarte possibly negative fractional second into time variables
   }

   if(FAKE_ACRON_RCVR && (seconds == last_sec)) {
   }
   else {
      last_sec = seconds;
      update_gps_screen(1556);
   }
if(debug_file) {
   gregorian(jd_local);
if(debug_file) fprintf(debug_file, "local: %02d/%02d/%02d %02d:%02d:%02d %f\n", g_year,g_month,g_day,g_hours,g_minutes,g_seconds, g_frac);
}
}


void parse_acron_atime()
{
int doy;
int tz;
double jd;
int last_tick = 999;
   // asynchronous time request (12 byte response)
if(debug_file) fprintf(debug_file, "acron atime:%s\n", nmea_msg);

   nmea_col = 0;

   get_scpi_field(2);
   hours = pri_hours = atoi(nmea_field);
   get_scpi_field(2);
   minutes = pri_minutes = atoi(nmea_field);
   get_scpi_field(2);
   seconds = pri_seconds = atoi(nmea_field);

   get_scpi_field(3);       // day of year  (does this start a 1?)
   doy = atoi(nmea_field);

   get_scpi_field(2);
   year = pri_year = atoi(nmea_field) + 2000;

   get_scpi_field(1);
   tz = atoi(nmea_field);

return;

   jd = jdate(year, 1, 1) + (double) (doy) - 1.0;
   jd += jtime(hours,minutes,seconds, 0.0);

   gregorian(jd);  // convert julian date to gregorian

   pri_year = year = g_year;  // update date/time variables to the corrected values
   pri_month = month = g_month;
   pri_day = day = g_day;

   pri_hours = hours = g_hours;
   pri_minutes = minutes = g_minutes;
   pri_seconds = seconds = g_seconds;
   pri_frac = raw_frac = g_frac;

   if(timing_mode == 0) {     // convert utc time to gps time
      utc_to_gps();
   }
   else {
      adjust_rcvr_time(0.0);  // incorporarte possibly negative fractional second into time variables
   }
   
   if(last_tick != seconds) {
      last_tick = seconds;
      update_gps_screen(1555);
   }
}


double bearing(double lat2,double lon2, double lat1,double lon1)
{
double dLon;
double x,y;
double brng;

   lat1 *= (PI/180.0);  // convert to radians
   lat2 *= (PI/180.0);
   lon1 *= (PI/180.0);  // convert to radians
   lon2 *= (PI/180.0);
   dLon = (lon2 - lon1);

   y = sin(dLon) * cos(lat2);
   x = cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(dLon);

   brng = atan2(y, x) * 180.0/PI;
   if(brng < 0.0) brng += 360.0;
// brng = fmod(brng+360.0, 360.0);
// brng = 360.0 - brng; // count degrees counter-clockwise - remove to make clockwise

   return brng;
}


void parse_acron_status()
{
int status;
int quality;
int prn;
double el;
double angle;

if(debug_file) fprintf(debug_file, "acron status:%s\n", nmea_msg);
   if(nmea_msg[0] == 0) return;

   status = nmea_msg[0];
   if(status & 0x01) {  // reception attempt in progress
   }
   if(status & 0x02) {  // port P51
   }
   
   level_type = "SNR";
   if(nmea_msg[1]) {    // signal quality
      quality = nmea_msg[1];
      quality &= 0x07;  // 5=good  <3=very poor  0=between reception trys

      if     (quality == 0) quality = 0;
      else if(quality == 1) quality = 30;
      else if(quality == 2) quality = 35;
      else if(quality == 3) quality = 40;
      else if(quality == 4) quality = 45;
      else if(quality >= 5) quality = 50;
      if(quality > 50) quality = 50;

      if(lat || lon) {  // az/el is bearing to Ft Collins WWVB site
         angle = bearing(WWVB_LAT,WWVB_LON, lat*180.0/PI,lon*180.0/PI);
         el = 1.0;
      }
      else {
         angle = 1.0;
         el = 89.0;
      }

      prn = 1;    // fake single sat at az=1 el=89
      sat[prn].level_msg = 111;
      sat[prn].azimuth = (float) angle;
      set_sat_el(prn, (float) el);
      sat[prn].sig_level = (float) quality;
      if(quality > 0) sat[prn].tracking = prn;
      else            sat[prn].tracking = (-1);
      have_sat_azel = 2;

      record_sig_levels(prn);

      sat_count = 1;
      config_sat_count(sat_count);
if(debug_file) fprintf(debug_file, "status:%d quality:%d\n", quality, status);
   }
}


void poll_next_acron()
{
   if(1 && (pri_seconds == SCPI_STATUS_SECOND)) { // request status
      if(need_acron_sync) {
         need_acron_sync = 0;
         send_acron_cmd("h");  // user wants a reception attempt
         Sleep(500);
      }

      send_acron_cmd("g");   // get receiver status
      ++pri_seconds;
      ++seconds;
   }
   else if(0 && (this_acron_cmd == 'G')) {
      send_acron_cmd("i");  // async world time / time zone info
   }
   else send_acron_cmd("o");  // time
}


void decode_acron_msg()
{
char c;
int i;

   start_msg_decode(0);

   nmea_msg[0] = 0;
   for(i=0; i<tsip_wptr; i++) {  // copy message from tsip buffer to nmea buffer
      c = tsip_byte();
      nmea_msg[i] = c;
      nmea_msg[i+1] = 0;
      if(i >= ((int)sizeof(nmea_msg)-12)) break;
   }

   tsip_wptr = tsip_rptr = 0;
   tsip_sync = 0;


  // get the response from the reciver and pass it to (HOPEFULLY) the routine
  // that expects it...

  strupr(nmea_msg);

if(debug_file) fprintf(debug_file, "Decode msg %c:[%s]\n", this_acron_cmd, nmea_msg);

   if     (this_acron_cmd == 'O') parse_acron_time();   // parse answer to last command we sent
   else if(this_acron_cmd == 'G') parse_acron_status(); // status
   else if(this_acron_cmd == 'I') parse_acron_atime();  // asynchronous time request (has time zone setting)

   poll_next_acron();
}


void get_acron_message()
{
u08 c;

   // This routine buffers up an incoming ACRON ZEIT message.  When the end of the
   // message is seen, the message is parsed and decoded with decode_acron_msg()

   if(SERIAL_DATA_AVAILABLE() == 0) {
      check_com_timer();
      return;
   }
   else {
      reset_com_timer();
   }

   c = get_com_char();
   c &= 0x7F;           // remove parity bit

   if(rcv_error) {      // parity/framing/overrun errors
      rcv_error = 0;
//!!!!!qqqq    goto rst_msg;
   }

   if(tsip_sync == 0) {    // syncing to start of message
      tsip_sync = 1;
      get_sync_time();
      tsip_wptr = 0;
      tsip_rptr = 0;
      goto get_acron;
   }
   else if(tsip_sync == 1) { // '$' has been seen, now build the message
      get_acron:
      if(c == 0x0D) goto acron_end;  // message has no checksum

      if(tsip_wptr < MAX_TSIP) {   // add char to the message buffer
         tsip_buf[tsip_wptr++] = c;
      }
      else {
         tsip_error |= 0x8000;
         goto rst_msg;
      }
   }
   else if(tsip_sync == 3) {  // end of message
      acron_end:
      decode_acron_msg(); // process the message
      tsip_wptr = 0;      // prepare for next message
      tsip_sync = 0;
   }
   else {     // should never happen, prepare for next message
      rst_msg:
      tsip_wptr = 0;
      tsip_sync = 0;
   }
}




//
//
//   GPSD receiver stuff
//
//


char json_delim;
char gpsd_cmd[NMEA_MSG_SIZE+2];


void enable_gpsd()
{
   sprintf(out, "?WATCH={\"enable\":true,\"json\":true,\"pps\":true}");
   send_nmea_string(out, 0);
//    send_byte(0x0D);
   eom_flag = 1;
   send_byte(0x0A);
}


char *get_json_field(char *s)
{
int i;
char c;
int quote;

   // gets the next item from a JSON message

   i = 0;
   nmea_field[i] = 0;
   quote = 0;
   json_delim = 0;
   if(s == 0) return 0;

  
if(*s == '{') {
   ++s;
   nmea_field[0] = 0;
   return s;
}
if(*s == '[') {
   ++s;
   nmea_field[0] = 0;
   return s;
}
  

   while(i < (NMEA_MSG_SIZE-2)) {
      c = *s++;
      if(c == 0) return 0;

      if(quote == 0) {
         if(c == ' ') continue;    // ignore white space
         if(c == '\t') continue;
         if(c == 0x0D) continue;
         if(c == 0x0A) continue;
         if(c == '[') {
//if(*s != '{') continue;
//            continue;
         }
         if(c == ']') continue;
         if(c == '{') continue;
         if(c == '}') continue;

         if(c == ':') {
            json_delim = c;
            break;
         }
         else if(c == '}') {
            json_delim = c;
            break;
         }
         else if(c == ',') {
            json_delim = c;
            break;
         }
         else if(c == '[') {
            json_delim = c;
            break;
         }
else if(c == '{') {
   json_delim = c;
   break;
}
         else if(c == ']') {
            json_delim = c;
            break;
         }
      }

      nmea_field[i++] = c;  // add char to field
      nmea_field[i] = 0;

      if(c == '"') {
         if(quote) {    // end of quoted string
            quote = 0;
         }
         else {         // start of quoted string
            quote = 1;
         }
      }
   }

   if(s) strupr(s);
   return s;
}

void log_gpsd_cmd()
{
   // write debug/trace info to the debug log file

   if(debug_file == 0) return;  

   fprintf(debug_file, "%s  ", gpsd_cmd);
   fprintf(debug_file, "%s\n", nmea_field);
}

char time_field[32];
int time_ptr;

void get_time_field(int n)
{
int i;

   // get a fixed length sub-field from a sting - used to decode the date/time
   // string.

   i = 0;
   while(n--) {
      time_field[i++] = nmea_field[time_ptr++];
      time_field[i] = 0;
   }
}

void parse_gpsd_tpv(char *s)
{
int i;
int old_yy;
int old_mm;
int old_dd;
int old_hh;
int old_min;
int old_sec;
double old_frac;

   // decode the TPV message - used to get the time, lat, lon, and altitude
   // info.  This message is used to drive the display... without it nothing
   // gets done.


   old_yy = year;      // used to skip duplicate time stamps
   old_mm = month;
   old_dd = day;
   old_hh = hours;
   old_min = minutes;
   old_sec = seconds;
   old_frac = raw_frac;

   while(s) {
      s = get_json_field(s);
      strcpy(gpsd_cmd, nmea_field);
      if(gpsd_cmd[0] == 0) continue; 

      s = get_json_field(s);
log_gpsd_cmd();

      if(!strcmp(gpsd_cmd, "\"TIME\"")) {
         time_ptr = 1;

         get_time_field(4);
         year = atoi(time_field);
         get_time_field(1);

         get_time_field(2);
         month = atoi(time_field);
         get_time_field(1);

         get_time_field(2);
         day = atoi(time_field);
         get_time_field(1);        // eat the 'T'

         get_time_field(2);
         hours = atoi(time_field);
         get_time_field(1);        // eat the ':'

         get_time_field(2);
         minutes = atoi(time_field);
         get_time_field(1);        // eat the ':'

         get_time_field(2);
         seconds = atoi(time_field);
         get_time_field(1);        // eat the 'Z'

         get_time_field(4);
         raw_frac = atof(time_field);
         get_time_field(1);        // eat the ':'

         pri_year = year;
         pri_month = month;
         pri_day = day;
         pri_hours = hours;
         pri_minutes = minutes;
         pri_seconds = seconds;
         pri_frac = raw_frac;

//gggg
if(debug_file) fprintf(debug_file, "nema_field:%s\n", nmea_field);
if(debug_file) fprintf(debug_file, "time:%04d-%02d-%02d  %02d:%02d:%02d %f\n", 
year,month,day, hours,minutes,seconds,raw_frac);

         time_flags |= 0x0001;  // UTC based time
      }
      else if(!strcmp(gpsd_cmd, "\"LAT\"")) {
         lat = atof(nmea_field) * PI / 180.0;
      }
      else if(!strcmp(gpsd_cmd, "\"LON\"")) {
         lon = atof(nmea_field) * PI / 180.0;
      }
      else if(!strcmp(gpsd_cmd, "\"ALT\"")) {
         alt = atof(nmea_field);
      }
      else if(!strcmp(gpsd_cmd, "\"TRACK\"")) {
         heading = atof(nmea_field);
         have_heading = 20;
      }
      else if(!strcmp(gpsd_cmd, "\"SPEED\"")) {
         speed = atof(nmea_field);
         have_speed = 20;
      }
      else if(!strcmp(gpsd_cmd, "\"MODE\"")) {
         i = atoi(nmea_field);
         if     (i == 3) rcvr_mode = RCVR_MODE_3D;
         else if(i == 2) rcvr_mode = RCVR_MODE_2D;
         else if(i == 1) rcvr_mode = RCVR_MODE_ACQUIRE;
         else            rcvr_mode = RCVR_MODE_UNKNOWN;
      }
   }


   if(old_yy != year) ;            // gpsd can generate duplicate time stamps
   else if(old_mm != month) ;
   else if(old_dd != day) ;
   else if(old_hh != hours) ;
   else if(old_min != minutes) ;
   else if(old_sec != seconds) ;
   else return;

   update_gps_screen(1008);
}

void parse_gpsd_sky(char *s)
{
int saw_sats;
int prn;

   // decode the SKY message - used to display satellite position and signal
   // level data

   have_dops = 0;
   prn = 0;
   saw_sats = 0;
   level_type = "SNR";

   while(s) {
      s = get_json_field(s);
      strcpy(gpsd_cmd, nmea_field);
      if(gpsd_cmd[0] == 0) continue; 

      s = get_json_field(s);
log_gpsd_cmd();

      if(!strcmp(gpsd_cmd, "\"GDOP\"")) {
         gdop = (float) atof(nmea_field);
         have_dops |= GDOP;
      }
      else if(!strcmp(gpsd_cmd, "\"TDOP\"")) {
         tdop = (float) atof(nmea_field);
         have_dops |= TDOP;
      }
      else if(!strcmp(gpsd_cmd, "\"HDOP\"")) {
         hdop = (float) atof(nmea_field);
         have_dops |= HDOP;
      }
      else if(!strcmp(gpsd_cmd, "\"VDOP\"")) {
         hdop = (float) atof(nmea_field);
         have_dops |= HDOP;
      }
      else if(!strcmp(gpsd_cmd, "\"PDOP\"")) {
         pdop = (float) atof(nmea_field);
         have_dops |= PDOP;
      }
      else if(!strcmp(gpsd_cmd, "\"SATELLITES\"")) {
         saw_sats = 0;
         reset_sat_tracking();
      }
      else if(!strcmp(gpsd_cmd, "\"PRN\"")) {
         prn = atoi(nmea_field);
         if(prn <= 0) prn = 0;
         else if(prn > MAX_PRN) prn = 0; 
         else ++saw_sats;
      }
      else if(!strcmp(gpsd_cmd, "\"EL\"")) {   
         set_sat_el(prn, (float) atof(nmea_field));
         have_sat_azel = 21;
      }
      else if(!strcmp(gpsd_cmd, "\"AZ\"")) {
         sat[prn].azimuth = (float) atoi(nmea_field);
         have_sat_azel = 20;
      }
      else if(!strcmp(gpsd_cmd, "\"SS\"")) {
         sat[prn].sig_level = (float) atof(nmea_field);
         sat[prn].level_msg = 55;
         record_sig_levels(prn);
      }
      else if(!strcmp(gpsd_cmd, "\"USED\"")) {
         if(!strcmp(nmea_field, "TRUE")) sat[prn].tracking = 1;
         else                            sat[prn].tracking = (-1);
      }
   }

   sat_count = saw_sats;
   have_count = 7;
if(debug_file) fprintf(debug_file, "sat count:%d\n", sat_count);
   config_sat_count(saw_sats);
}

void show_gpsd_msg(char *s)
{
   // used for debug logging
   while(s) {
      s = get_json_field(s);
      strcpy(gpsd_cmd, nmea_field);
      if(gpsd_cmd[0] == 0) continue; 

      s = get_json_field(s);
log_gpsd_cmd();
   }
}

void parse_gpsd_pps(char *s)
{
double clock_sec, clock_nsec;
double real_sec, real_nsec;
double precision;

   // decode the PPS message - used to get info for the PPS plot

   saw_gpsd_pps = 1;
   pps_enabled = 1;
   have_pps_enable = 11;

   precision = 0;  // NTP precision
   clock_sec = real_sec = 0.0;
   clock_nsec = real_nsec = 0.0;

   while(s) {
      s = get_json_field(s);
      strcpy(gpsd_cmd, nmea_field);
      if(gpsd_cmd[0] == 0) continue; 

      s = get_json_field(s);
log_gpsd_cmd();

      if(!strcmp(gpsd_cmd, "\"CLOCK_SEC\"")) {
         clock_sec = atof(nmea_field);
      }
      else if(!strcmp(gpsd_cmd, "\"CLOCK_NSEC\"")) { 
         clock_nsec = atof(nmea_field)*1.0E-9;
      }
      else if(!strcmp(gpsd_cmd, "\"REAL_SEC\"")) { 
         real_sec = atof(nmea_field);
      }
      else if(!strcmp(gpsd_cmd, "\"REAL_NSEC\"")) { 
         real_nsec = atof(nmea_field)*1.0E-9;
      }
      else if(!strcmp(gpsd_cmd, "\"PRECISION\"")) { // ntp style power of two seconds
         saw_gpsd_pre = 1;
         precision = atof(nmea_field);
         precision = pow(2.0, precision);
      }
   }

   pps_offset = (clock_sec - real_sec);
   have_pps_offset = 8;
   pps_offset += (clock_nsec - real_nsec);
   pps_offset *= 1.0E9;
if(debug_file) fprintf(debug_file, "pps offset:%.10f", pps_offset);

}

void parse_gpsd_osc(char *s)
{
int saw_run;
int running;
int saw_ref;
int ref;
int saw_dis;
int dis;
int saw_pps;
double pps;

   // decode the OSC message - used to display GPSDO related info

   saw_run = saw_ref = saw_dis = saw_pps = 0;
   running = ref = dis = 0;
   pps = 0.0;
   saw_gpsdo = 1;
   while(s) {
      s = get_json_field(s);
      strcpy(gpsd_cmd, nmea_field);
      if(gpsd_cmd[0] == 0) continue; 

      s = get_json_field(s);
log_gpsd_cmd();

      if(!strcmp(gpsd_cmd, "\"RUNNING\"")) {  // warming up
         if(!strcmp(nmea_field, "TRUE")) {
            running = 1;
         }
         else {
            running = 0;
         }
         saw_run = 1;
      }
      else if(!strcmp(gpsd_cmd, "\"REFERENCE\"")) {   // pps active
         if(!strcmp(nmea_field, "TRUE")) ref = 1;
         else                            ref = 0;
         saw_ref = 1;
      }
      else if(!strcmp(gpsd_cmd, "\"DISCIPLINED\"")) { // being disciplined
         if(!strcmp(nmea_field, "TRUE")) dis = 1;
         else                            dis = 0;
         saw_dis = 1;
      }
      else if(!strcmp(gpsd_cmd, "\"DELTA\"")) {  // ns of err
         pps = atof(nmea_field);
         osc_offset = pps;
         have_osc_offset = 8;
         saw_pps = 1;
      }
   }

   if(saw_run && (running == 0))  discipline_mode = 1; // warming up
   else if(saw_dis && (dis == 0)) discipline_mode = 3; // PPS OFF -> holdover
   else if(saw_ref && (ref == 0)) discipline_mode = 4; // PPS OFF -> holdover
   else                           discipline_mode = 0;
}

void parse_gpsd_version(char *s)
{
int i;

   // decode the VERSION message - used to display the version info

   saw_version |= 0x0200;
   while(s) {
      s = get_json_field(s);
      strcpy(gpsd_cmd, nmea_field);
      if(gpsd_cmd[0] == 0) continue; 

      s = get_json_field(s);
log_gpsd_cmd();

      if(!strcmp(gpsd_cmd, "\"PROTO_MAJOR\"")) {
         gpsd_major = atoi(nmea_field);
      }
      else if(!strcmp(gpsd_cmd, "\"PROTO_MINOR\"")) {
         gpsd_minor = atoi(nmea_field);
      }
      else if(!strcmp(gpsd_cmd, "\"RELEASE\"")) {
         nmea_field[17] = 0;
         gpsd_release[0] = 0;
         i = strlen(nmea_field);
         if(i) {
            strcpy(gpsd_release, &nmea_field[1]);
            i = strlen(gpsd_release);
            if(i) gpsd_release[i-1] = 0;
         }
      }
   }
}

void parse_gpsd_devices(char *s)
{
int i;

   // decode the DEVICES message - we just use the driver name field

   saw_version |= 0x0400;
   while(s) {
      s = get_json_field(s);
      strcpy(gpsd_cmd, nmea_field);
      if(gpsd_cmd[0] == 0) continue; 

      s = get_json_field(s);
log_gpsd_cmd();

      if(!strcmp(gpsd_cmd, "\"DRIVER\"")) {
         i = strlen(nmea_field);
if(debug_file) fprintf(debug_file, "driver:%s -> ", nmea_field);
         if(i) nmea_field[i-1] = 0;
         nmea_field[17] = 0;
if(debug_file) fprintf(debug_file, "%s\n", nmea_field);
         show_gpsd_driver();
      }
   }
}


void parse_gpsd_att(char *s)
{
   // decode the ATT message - we just use the temperature field

   while(s) {
      s = get_json_field(s);
      strcpy(gpsd_cmd, nmea_field);
      if(gpsd_cmd[0] == 0) continue; 

      s = get_json_field(s);
log_gpsd_cmd();

      if(!strcmp(gpsd_cmd, "\"TEMPERATURE\"")) {
         temperature = (float) atof(nmea_field);
         have_temperature = 5;
      }
   }
}


void parse_gpsd_rtcm2(char *s)
{
   // decode the RTCM2 message - we just use the leapsecs field

   while(s) {
      s = get_json_field(s);
      strcpy(gpsd_cmd, nmea_field);
      if(gpsd_cmd[0] == 0) continue; 

      s = get_json_field(s);
log_gpsd_cmd();

      if(!user_set_utc_ofs && !strcmp(gpsd_cmd, "\"LEAPSECS\"")) {
         utc_offset = atoi(nmea_field);
         check_utc_ofs(11);
      }
      else if(!strcmp(gpsd_cmd, "\"WEEK\"")) {
         gps_week = atoi(nmea_field); 
         if(have_week == 0) need_redraw = 2056;
         have_week = 10;
      }
   }
}


void parse_gpsd_watch(char *s)
{
   // decode the WATCH message - we just thow it away
   while(s) {
      s = get_json_field(s);
      strcpy(gpsd_cmd, nmea_field);
      if(gpsd_cmd[0] == 0) continue; 

      s = get_json_field(s);
log_gpsd_cmd();
   }
}


void decode_gpsd_msg()
{
char *s;

   // this routine looks at the GPSD message header and dispatches a call to
   // decode the data in the message and act on it.
   start_msg_decode(1); // gggggg
   
   s = strstr((char *) tsip_buf, "\"class\":");
   if(s == 0) return;

   s = get_json_field(s);     // get the class keyword
   if(json_delim != ':') return;  // no class
   s = get_json_field(s);     // get the message type

   strupr(nmea_field);
if(debug_file) fprintf(debug_file, "\n\nMessage: %s (%c)\n\n", nmea_field, json_delim);
   if     (!strcmp(nmea_field, "\"TPV\""))     parse_gpsd_tpv(s);
   else if(!strcmp(nmea_field, "\"SKY\""))     parse_gpsd_sky(s);
   else if(!strcmp(nmea_field, "\"PPS\""))     parse_gpsd_pps(s);
   else if(!strcmp(nmea_field, "\"TOFF\""))    parse_gpsd_pps(s);
   else if(!strcmp(nmea_field, "\"OSC\""))     parse_gpsd_osc(s);
   else if(!strcmp(nmea_field, "\"ATT\""))     parse_gpsd_att(s);
   else if(!strcmp(nmea_field, "\"RTCM2\""))   parse_gpsd_rtcm2(s);
   else if(!strcmp(nmea_field, "\"VERSION\"")) parse_gpsd_version(s);
   else if(!strcmp(nmea_field, "\"DEVICES\"")) parse_gpsd_devices(s);
   else if(!strcmp(nmea_field, "\"DEVICE\""))  parse_gpsd_devices(s);
   else if(!strcmp(nmea_field, "\"WATCH\""))   parse_gpsd_watch(s);
   else                                        show_gpsd_msg(s);
}


#define MAX_GPSD_LEN 1536

void get_gpsd_message()
{
u08 c;
static int paren_level = 0;

   // This routine buffers up an incoming GPSD JSON message.  When the end of the
   // message is seen, the message is parsed and decoded with decode_gpsd_msg()

   if(SERIAL_DATA_AVAILABLE() == 0) {
      check_com_timer();
      return;
   }
   else {
      reset_com_timer();
   }

   c = get_com_char();
   if(rcv_error) {      // parity/framing/overrun errors
      rcv_error = 0;
//!!!!!qqqq    goto rst_msg;
   }

   if(tsip_sync == 0) {    // syncing to start of message, search for a '{'
      tsip_rptr = 0;
      tsip_wptr = 0;
      tsip_buf[tsip_wptr] = 0;
      if(c == '{') {
         get_sync_time();
         ++tsip_sync;
         tsip_buf[tsip_wptr++] = (u08) c;
         tsip_buf[tsip_wptr] = 0;
         paren_level = 1;
      }
      return;
   }
   else if(tsip_sync == 1) { // '"' has been seen
      if(c == '"') ++tsip_sync; 
      else goto rst_msg;
      tsip_buf[tsip_wptr++] = (u08) c;
      tsip_buf[tsip_wptr] = 0;
   }
   else if(tsip_sync == 2) { // 'c' has been seen
      if(c == 'c') ++tsip_sync; 
      else goto rst_msg;
      tsip_buf[tsip_wptr++] = (u08) c;
      tsip_buf[tsip_wptr] = 0;
   }
   else if(tsip_sync == 3) { // 'l' has been seen
      if(c == 'l') ++tsip_sync; 
      else goto rst_msg;
      tsip_buf[tsip_wptr++] = (u08) c;
      tsip_buf[tsip_wptr] = 0;
   }
   else if(tsip_sync == 4) { // 'a' has been seen
      if(c == 'a') ++tsip_sync; 
      else goto rst_msg;
      tsip_buf[tsip_wptr++] = (u08) c;
      tsip_buf[tsip_wptr] = 0;
   }
   else if(tsip_sync == 5) { // 's' has been seen
      if(c == 's') ++tsip_sync; 
      else goto rst_msg;
      tsip_buf[tsip_wptr++] = (u08) c;
      tsip_buf[tsip_wptr] = 0;
   }
   else if(tsip_sync == 6) { // 's' has been seen - time to start building the full message
      if(c == 's') ++tsip_sync; 
      else goto rst_msg;
      tsip_buf[tsip_wptr++] = (u08) c;
      tsip_buf[tsip_wptr] = 0;
   }
   else if(tsip_sync == 7) {  // adding chars to the message
      if(tsip_wptr > MAX_GPSD_LEN) goto rst_msg; // message too long (max JSON is 1536 chars)
      if((c == 0x0D) || (c == 0x0A)) {
         decode_gpsd_msg();
         tsip_wptr = 0;     // prepare for next message
         tsip_sync = 0;
      }
      else {
         tsip_buf[tsip_wptr++] = (u08) c;
         tsip_buf[tsip_wptr] = 0;
      }
   }
   else {     // should never happen, prepare for next message
      rst_msg:
      tsip_wptr = 0;
      tsip_sync = 0;
      paren_level = 0;
   }
}


//
//
//   Information display routines
//
//


void show_cable_delay()
{
double val;

   if(zoom_screen) return;
   if(text_mode && first_key) return;
   if(!have_cable_delay) return;

   val = cable_delay*1.0E9;

   if     (val >= 100000000.0) sprintf(out, "Dly:%11.0f ns", val);
   else if(val >= 10000000.0)  sprintf(out, "Dly:%11.1f ns", val);
   else if(val >= 1000000.0)   sprintf(out, "Dly:%11.2f ns", val);
   else if(val >= 100000.0)    sprintf(out, "Dly:%11.3f ns", val);
   else if(val >= 10000.0)     sprintf(out, "Dly:%11.4f ns", val);
   else if(val >= 1000.0)      sprintf(out, "Dly:%11.5f ns", val);
   else                        sprintf(out, "Dly:%11.6f ns", val);
   vidstr(VAL_ROW+5, VAL_COL, WHITE, out);
}

void show_test_warning()
{
   erase_screen();
   vidstr(0,0, YELLOW, "Operation in progress... this may take a while");
   refresh_page();
   Sleep(3000);
   erase_screen();
   refresh_page();
}


void show_sun_azel(int row, int col)
{
   sprintf(out, "Sun az:  %9.5f%c", sun_az, DEGREES);
   vidstr(row+0, col, WHITE, out);
   sprintf(out, "Sun el:  %9.5f%c", sun_el, DEGREES);
   vidstr(row+1, col, WHITE, out);
}

void show_moon_azel(int row, int col)
{
//
// MoonPhase = moon_info(jd_utc);
// MoonPhase = moon_phase(jd_jtc);

   sprintf(out, "Moon az: %9.5f%c", moon_az, DEGREES);
   vidstr(row+0, col, WHITE, out);
   sprintf(out, "Moon el: %9.5f%c", moon_el, DEGREES);
   vidstr(row+1, col, WHITE, out);
   sprintf(out, "Phase:   %9.5f%%", (MoonPhase*100.0));
   vidstr(row+2, col, WHITE, out);
}

void show_sun_rise_set(int row, int col)
{
char c1, c2, c3;
int color;
double mage;

   if(sunrise_type == 0) return;
   if(all_adevs) return;

   if(do_moonrise || (sunrise_type[0] == 0)) {
      c1 = c2 = c3 = ' ';
   }
   else c1 = c2 = c3 = tolower(sunrise_type[0]);

   if(sun_file && play_sun_song) {
      c2 = SONG_CHAR;
   }
   if(noon_file && play_sun_song) {
      c3 = SONG_CHAR;
   }

   if(have_sun_times & 0x01) color = WHITE;
   else                      color = GREY;
   if(do_moonrise) sprintf(out, "Moonrise: %02d:%02d:%02d%c", rise_hh,rise_mm,rise_ss,c2);
   else            sprintf(out, "Sunrise:  %02d:%02d:%02d%c", rise_hh,rise_mm,rise_ss,c2);
   vidstr(row+0, col, color, out);

   if(have_sun_times & 0x02) color = WHITE;
   else                      color = GREY;
   if(do_moonrise) sprintf(out, "Transit:  %02d:%02d:%02d%c", noon_hh,noon_mm,noon_ss,c3);
   else            sprintf(out, "Sun noon: %02d:%02d:%02d%c", noon_hh,noon_mm,noon_ss,c3);
   vidstr(row+1, col, color, out);

   if(have_sun_times & 0x04) color = WHITE;
   else                      color = GREY;
   if(do_moonrise) sprintf(out, "Moon set: %02d:%02d:%02d%c", set_hh,set_mm,set_ss,c1);
   else            sprintf(out, "Sunset:   %02d:%02d:%02d%c", set_hh,set_mm,set_ss,c1);
   vidstr(row+2, col, color, out);

   if(do_moonrise) {
//    mage = MoonAge;
      if(new_moon_jd && (jd_utc >= new_moon_jd)) mage = (jd_utc - new_moon_jd);
      else if(MoonAge) mage = MoonAge;
      else mage = moon_age(jd_utc);
      sprintf(out, "Moon age: %8.5fd", mage); // moon_age(jd_utc));
   }
   else {
      eot(jd_utc); // uses NOAA algorithm, but seems a second or so off of NREL's numbers (which I don't fully trust, but included a fudge factor to make them match)
      sprintf(out, "EqTime:  %9.5fm", eot_ofs);
   }
   vidstr(row+3, col, WHITE, out);
}


void show_sun_moon(int row, int col)
{
   if(all_adevs) return;
   if(luxor) {
      sun_el = sun_posn(jd_utc, 0);
      have_sun_el = 1;
      log_sun_posn();

      if(do_moonrise) calc_moonrise();
      else            calc_sunrise(0.0, 21);
      --row;
   }
   else if(rcvr_type == NO_RCVR) ++row;
// else if(SCREEN_HEIGHT >= 600) ; // ++row;
// if(rcvr_type == TSIP_RCVR) col += 42;  // gggg

   show_sun_azel(row+1, col);
   if(text_mode || (SCREEN_HEIGHT >= MEDIUM_HEIGHT)) {
      show_moon_azel(row+4-1, col);
show_sun_rise_set(row+3+3, col);
   }
   else if(1 || luxor || (SCREEN_HEIGHT >= 600) || (rcvr_type == NO_RCVR)) {
      show_moon_azel(row+3, col);
show_sun_rise_set(row+3+3, col);
   }
}


#ifdef SIG_LEVELS
void show_max_signals()
{
int row,col;
int prn;
char c[32];
float min, max;
float sum;
float count;

   if(all_adevs) return;
   if(rcvr_type == NO_RCVR) return;
   if(rcvr_type == ACRON_RCVR) return;

   row = DIS_ROW+0+eofs+5;
   col = DIS_COL;
   sprintf(out, "Satellite max %s", level_type);
   vidstr(row, col, WHITE, out); 
   ++row;
   strcpy(out, " 01: ");
   sum = count = 0.0F;
   max = 0.0F;
   min = 9999.0F;
//   for(prn=1; prn<=MAX_PRN; prn++) {  // !!!! max_sat_check (32)
   for(prn=1; prn<=32; prn++) {  // !!!! max_sat_check (32)
      if(max_sat_db[prn]) {
         sprintf(c, "%2d ", (int) (max_sat_db[prn]+0.5F));
         sum += max_sat_db[prn];
         if(max_sat_db[prn] > max) max = max_sat_db[prn];
         if(max_sat_db[prn] < min) min = max_sat_db[prn];
         count += 1.0F;
      }
      else strcpy(c,".. ");

      strcat(out, c);
      if((prn%5) == 0) { 
         vidstr(row, col, WHITE, out); 
         sprintf(out, " %02d: ", prn+1);
         ++row;
      }
   }
   vidstr(row, col, WHITE, out); 

//   if(SCREEN_HEIGHT > (480.0+TEXT_HEIGHT)) ++row;
//   if(SCREEN_HEIGHT >= MEDIUM_HEIGHT) ++row;
   if(min >= 9999.0F) min = 0.0F;
   if(count) {
      sum = (sum / count) + 0.50F;
   }
   else sum = 0.0F;
   min += 0.50;
   max += 0.50;

   sprintf(out, " avg: %2d    max: %2d", (int) sum, (int) max);
   ++row;
   vidstr(row, col, WHITE, out); 

if(0 && !digital_clock_shown) show_sun_moon(row,col);
}
#endif // SIG_LEVELS


void show_osc_params(int row, int col)
{
double dac;
double drift;
double tempco;
char c[10];
int prn;
int color;

   prn = 0;
   c[0] = 0;

   if(all_adevs) {
      if(SCREEN_WIDTH > 800)   col += 3;
      else if(small_font == 1) col -= 3;
   }

   if(rcvr_type == STAR_RCVR) {
      if(have_tc & 0x01) color = WHITE;
      else               color = GREY; 
      sprintf(out, "User time constant: %.3f secs    ", time_constant);
      vidstr(row++, col, color, out);

      if(have_tc & 0x02) color = WHITE;
      else               color = GREY; 
      sprintf(out, "Real time constant: %.3f secs    ", real_time_constant);
      vidstr(row++, col, color, out);

      if(have_gpsdo_ref & 0x02) color = WHITE;
      else                      color = GREY; 
      if(gpsdo_ref) sprintf(out, "GPSDO reference:    AUX PPS    ");
      else          sprintf(out, "GPSDO reference:    GPS        ");
      vidstr(row++, col, color, out);
   }
   else if(rcvr_type == TSIP_RCVR) {
      if(have_jam_sync) color = WHITE;
      else color = GREY;
      if(jam_sync < 10.0)       sprintf(out, "Jam sync threshold:   %.3f ns    ", jam_sync);
      else if(jam_sync < 100.0) sprintf(out, "Jam sync threshold:  %.3f ns    ", jam_sync);
      else                      sprintf(out, "Jam sync threshold: %.3f ns    ", jam_sync);
      if(saw_ntpx) vidstr(row++, col, GREY,  out);
      else         vidstr(row++, col, color, out);

      if(have_freq_ofs) color = WHITE;
      else color = GREY;
      if(max_freq_offset < 10.0) sprintf(out, "Max freq offset:      %.3f%s    ", max_freq_offset, ppb_string);
      else                       sprintf(out, "Max freq offset:     %.3f%s    ", max_freq_offset, ppb_string);
      if(saw_ntpx) vidstr(row++, col, GREY,  out);
      else         vidstr(row++, col, color, out);

      if(have_tc) color = WHITE;
      else        color = GREY; 
      sprintf(out, "Time constant:      %.3f secs    ", time_constant);
      vidstr(row++, col, color, out);

      if(have_damp) color = WHITE;
      else          color = GREY; 
      sprintf(out, "Damping factor:       %.3f    ", damping_factor);
      vidstr(row++, col, color, out);

      if(gain_color == YELLOW) color = gain_color;
      else if(have_gain) color = WHITE;
      else color = GREY; 
      sprintf(out, "Osc gain:            %+.6f Hz/V    ", osc_gain);
      vidstr(row++, col, color, out);

      if(have_initv) color = WHITE;
      else           color = GREY; 
      sprintf(out, "Initial DAC voltage: %+.6f V    ", initial_voltage);
      if(saw_ntpx) vidstr(row++, col, GREY,  out);
      else         vidstr(row++, col, color, out);

      if(have_minv) color = WHITE;
      else          color = GREY; 
      sprintf(out, "Min EFC voltage:     %+.6f V    ", min_volts);
      vidstr(row++, col, color, out);

      if(have_maxv) color = WHITE;
      else          color = GREY; 
      sprintf(out, "Max EFC voltage:     %+.6f V    ", max_volts);
      vidstr(row++, col, color, out);

      if(have_dac_range) {
         sprintf(out, "EFC DAC range:       %+.2f to %+.2f V ", min_dac_v, max_dac_v);
         if(saw_ntpx) vidstr(row++, col, GREY,  out);
         else         vidstr(row++, col, WHITE, out);
      }


   
      // Calculate oscillator drift rate per day based upon the dac
      // voltage change over the plot display interval.  This assumes
      // that the temperature is stable.
      dac = plot[DAC].a1;
      drift = (dac * (double) osc_gain) * 24.0*3600.0;  // Hz per day
      sprintf(out, "Osc drift rate: %14.6e parts/day   ", drift/10.0E6F);
      vidstr(row+1, col, WHITE, out);

      // Calculate oscillator freq change with temperature over the plot
      // display interval.  This assumes that the temperature has changed
      // during the display interval.
      tempco = plot[TEMP].a1;  // degrees/sec
      if(tempco) {
         tempco = plot[OSC].a1 / tempco;  // (ppb/sec) divided by (degrees/sec)
         tempco /= (-1.0E9);              // convert ppb/degree C into parts/C
         sprintf(out, "Osc tempco:     %14.6e parts/%cC   ", tempco, DEGREES);
         vidstr(row+2, col, WHITE, out);
      }
   }
   else if(rcvr_type == UCCM_RCVR) {
      if(have_pullin) color = WHITE;
      else            color = GREY; 
      sprintf(out, "Pullin range:       %3d PPB      ", pullin_range);
      vidstr(row++, col, color, out);
   }

#ifdef SIG_LEVELS
   show_max_signals();
#endif

   if(0) {  // alternate osc parameter calculations
      if(view_interval == 0) return;
      if(plot_mag == 0) return;
      if(plot[DAC].stat_count == 0.0) return;
      if(plot[OSC].stat_count == 0.0) return;
      ++row;

      dac = plot[DAC].sum_change / plot[DAC].stat_count / view_interval;
      dac /= plot_mag;
      drift = (dac * (double) osc_gain) * 24.0*3600.0;  // Hz per day
      sprintf(out, "Osc drift rate: %14.6e parts/day   ", drift/10.0E6F);
      vidstr(row++, col, WHITE, out);

      tempco = plot[TEMP].sum_change / plot[TEMP].stat_count / view_interval;
      tempco /= plot_mag;
      if(tempco) {
         tempco = (plot[OSC].sum_change/plot[OSC].stat_count/view_interval) / tempco;  // (ppb/sec) divided by (degrees/sec)
         tempco /= (-1.0E9);              // convert ppb/degree C into parts/C
         sprintf(out, "Osc tempco:     %14.6e parts/%cC   ", tempco, DEGREES);
         vidstr(row++, col, WHITE, out);
      }
   }

   return;
}

int highest_sat()
{
float el;
int prn;
int i;

   // return the PRN of the highest elevation satellite

   prn = 0;
   el = 0.0;
   for(i=1; i<=MAX_PRN; i++) { 
      if(sat[i].tracking > 0) {
         if(sat[i].elevation > el) {
            el = sat[i].elevation;
            prn = i;
         }
      }
   }
   return prn;
}

struct SAT_SORT {
   int prn;
   double val;
} sat_sort[MAX_PRN+1];


void sort_sat_vals()
{
int i, j;
int k;
int m;
double val;

   // Creates a list of sat info values (sig levels, az, or el) sorted in 
   // in descending order along with their PRNs.  This list can be used
   // to display the sat info in sorted order

   if     (sort_by == SORT_SIGS) ;
   else if(sort_by == SORT_ELEV) ;
   else if(sort_by == SORT_AZ) ;
   else if(sort_by == SORT_DOPPLER) ;
   else if(sort_by == SORT_PRN) ;
   else return;

   j = 0;

   for(i=1; i<=MAX_PRN; i++) {
      if(sat[i].level_msg == 0) continue;  // sat prn not tracked

      if     (sort_by == SORT_SIGS)    val = sat[i].sig_level;  // the sat's signal level
      else if(sort_by == SORT_ELEV)    val = sat[i].elevation;  // the sat's elevation
      else if(sort_by == SORT_AZ)      val = sat[i].azimuth;    // the sat's azimuth
      else if(sort_by == SORT_DOPPLER) val = sat[i].doppler;    // the sat's doppler
      else if(sort_by == SORT_PRN)     val = (float) i;         // the sat's PRN
      else val = (double) i;

      if(j == 0) {  // first entry
         sat_sort[j].prn = i;
         sat_sort[j].val = val;
         ++j;
         continue;
      }
      else if(sort_ascend) {  // sort in ascending order
         if(val >= sat_sort[j-1].val) {  // insert at end of list
            sat_sort[j].prn = i;
            sat_sort[j].val = val;
            ++j;
            continue;
         }
      }
      else {  // sort in descending order
         if(val <= sat_sort[j-1].val) {  // insert at end of list
            sat_sort[j].prn = i;
            sat_sort[j].val = val;
            ++j;
            continue;
         }
      }

      for(k=0; k<j; k++) {  // see where this value lies in the list so far
         if(sort_ascend) {
            if(val > sat_sort[k].val) continue;  // it's greater than the current value
         }
         else {
            if(val < sat_sort[k].val) continue;  // it's less than the current value
         }

         for(m=j; m>k; m--) {  // move lower value entries down one position
            sat_sort[m].prn = sat_sort[m-1].prn;
            sat_sort[m].val = sat_sort[m-1].val;
         }

         sat_sort[k].prn = i;    // insert new value in the list
         sat_sort[k].val = val;
         ++j;
         break;
      }
   }
}


void show_satinfo()
{
int i, j;
float az;
float el;
u08 el_dir;
double dop;
double phase;
float sig;
float clock;
float acc;
COORD row, col;
u08 time_exit;
char fmt[256];
char el_string[10];
char bias_string[20];
int len, max_len;
u08 info_under_azel;
int show_short_info;
int datum_crapum;
int crap_count;
int prn;
int old_row, old_col;
int col2;
int top_row;
int track_count;
int col_switch;
int rows_avail;

#define COL2 30  // where to show the second row of sat info

   // This routine draws the satellite info table.  It also calls the
   // digital clock display routine (since on small screens the digital clock
   // takes the place of the sat info display).  It also controls the alarm
   // clock operation.
   last_sat_row = SAT_ROW+3;

   if(text_mode && first_key) {
      find_sat_changes();
      return;
   }
   
   time_exit = 0;
   print_using = 0;
   fmt[0] = 0;

   if(all_adevs && (zoom_screen != 'I')) {   // screen is showing all the adev tables
      if(text_mode) {
         col = 0;
         row = TEXT_ROWS - 8;
      }
      else {
         if(SCREEN_WIDTH >= 1280) col = CRIT_COL + 19;
         else                     col = CRIT_COL + 12;
         row = 0;
      }
   }
   else {    // normal screen mode
      col = SAT_COL;
      row = SAT_ROW+eofs;
      if(text_mode) ++row;
      else if((TEXT_HEIGHT <= 14) || (SCREEN_HEIGHT >= MEDIUM_HEIGHT)) ++row;
   }

   if(sound_alarm) {
      if(ticker & 0x01)      time_color = ALARM_COLOR;
      else if(dst_ofs)       time_color = DST_TIME_COLOR;
      else if(time_zone_set) time_color = DST_TIME_COLOR;  // !!! std_time_color is rather dim for the big clock
      else                   time_color = WHITE;
   }

   // if sats are not in use show the osc control info
// if(osc_params || (osc_discipline == 0)) {  
   if(zoom_screen == 'I') ;
   else if(osc_params && (all_adevs == 0) && (zoom_screen == 0)) {
      show_osc_params(row, col);
      find_sat_changes();
      return;
   }

   info_under_azel = 0;
   if(SHARED_AZEL && (AZEL_BOTTOM+(TEXT_HEIGHT*(8+1)) < SCREEN_HEIGHT)) {  
      if(SCREEN_HEIGHT <= MEDIUM_HEIGHT) {
         info_under_azel = 1;
      }
      else if(plot_watch && plot_digital_clock && ebolt) info_under_azel = 1;
      else if(all_adevs) info_under_azel = 1;
   }

   digital_clock_shown = 0;
   #ifdef DIGITAL_CLOCK
      if(zoom_screen == 'I') ;
      else if((plot_digital_clock && ((all_adevs == 0) || (SCREEN_WIDTH > 800))) || (zoom_screen == 'C')) {
         time_exit = show_digital_clock(row, col);
         if((time_exit == 0) && (all_adevs == 0)) info_under_azel = 0;

         // there is room to sneak in the sat info table below the az/el map
         if(info_under_azel) {  
            row = AZEL_BOTTOM/TEXT_HEIGHT;
            col = (AZEL_COL/TEXT_WIDTH) + 5; 
            if(ebolt) ;        
            else if(all_adevs) col -= 2;
         }
         else if(time_exit) { // no room on screen for sat info and digital clock
            find_sat_changes();
            return;
         }
      }
   #endif  // DIGITAL_CLOCK

   if(zoom_screen == 'I') {
      erase_screen();
      show_time_info();
      if((lat != 0.0) || (lon != 0.0)) {
         show_sun_moon(VAL_ROW-1,VAL_COL);
      }
      format_lla(lat,lon,alt, VAL_ROW,VAL_COL+22);
      row = 10;
   }
   else if(zoom_screen) {
      find_sat_changes();
      return;
   }

   if(luxor) return;
   if(rcvr_type == ACRON_RCVR) goto warn;
   if(rcvr_type == NO_RCVR) {
      if((lat != 0.0) || (lon != 0.0)) {
         show_sun_moon(VAL_ROW-2,VAL_COL);
      }
      else {
         warn:
         i = 0;
         if((lat != 0.0) || (lon != 0.0)) ;
         else {
            vidstr(VAL_ROW+i,VAL_COL, RED, "No location set.  ");
            ++i;
            vidstr(VAL_ROW+i,VAL_COL, RED, "Use the SL command");
            ++i;
            vidstr(VAL_ROW+i,VAL_COL, RED, "to set it.        ");
            ++i;
            if(rcvr_type == NO_RCVR) ++i;
         }

         if(have_utc_ofs < 0) {    // using estimated value
            vidstr(VAL_ROW+i,VAL_COL, RED, "UTC leapsecond   ");
            ++i;
            vidstr(VAL_ROW+i,VAL_COL, RED, "offset guessed.  ");
            ++i;
            vidstr(VAL_ROW+i,VAL_COL, RED, "Use the /uo=#    ");
            ++i;
            vidstr(VAL_ROW+i,VAL_COL, RED, "command to set it");
         }
      }
      if(rcvr_type == ACRON_RCVR) ;
      else return;
   }
   if(rcvr_type == ACRON_RCVR) {  // !!!! should we handle this as NO_RCVR?
//    return;
   }

   if(zoom_screen == 'I') ;
   else if(all_adevs) {  // if no room to show info,  just check for sat changes
      if(text_mode) {
         find_sat_changes();
         return;
      }

      if(first_key && (row >= PLOT_TEXT_ROW)) {
         find_sat_changes();
         return;
      }

      if((digital_clock_shown || ((row+max_sats+1) >= all_adev_row))) {
         if(info_under_azel && ((plot_lla == 0) || (all_adevs == 0))) {  
            row = AZEL_BOTTOM/TEXT_HEIGHT;
            col = (AZEL_COL/TEXT_WIDTH) + 5; 
            if(ebolt) ;        
            else if(all_adevs) col -= 2;
         }
         else {
            find_sat_changes();
            return;
         }
      }
   }

   // print the sat info table header
   if(max_sat_display == 0) return;

   top_row = row;
   rows_avail = (MOUSE_ROW-top_row-1);

   track_count = 0;
   for(prn=1; prn<=MAX_PRN; prn++) {
      if(sat[prn].level_msg) ++track_count;
   }

   if(zoom_screen == 'I') {
      col_switch = 0;
   }
   else if(track_count > 8) {
      col_switch = ((max_sat_display+1) / 2);
if(rows_avail < col_switch) col_switch = rows_avail;
      if(col_switch < 8) col_switch = 8;
   }
   else col_switch = 8;
//sprintf(plot_title, "sat cols:%d  rows:%d  max_sats:%d  msd:%d  tracked:%d  sw:%d  avail:%d top:%d mouse:%d", 
//sat_cols, sat_rows,max_sats,max_sat_display, track_count, col_switch,rows_avail,top_row,MOUSE_ROW);

   show_short_info = user_set_short;
   if(all_adevs) show_short_info = 1;
   if(sat_cols > 1) show_short_info = 2;
   if(zoom_screen == 'I') show_short_info = 0;

   // build the sat info title string
   i = col;
   col2 = col+COL2;

   sprintf(out, "PRN   ");
   if(sort_by == 'P') vidstr(row,col, GREEN, out);
   else               vidstr(row,col, WHITE, out);
   if(sat_cols > 1) {
      if(sort_by == 'P') vidstr(row,col+COL2, GREEN, out);
      else               vidstr(row,col+COL2, WHITE, out);
   }
   col += strlen(out);

   sprintf(out, "%cAZ   ", DEGREES);
   if(sort_by == 'A') vidstr(row,col, GREEN, out);
   else               vidstr(row,col, WHITE, out);
   if(sat_cols > 1) {
      if(sort_by == 'A') vidstr(row,col+COL2, GREEN, out);
      else               vidstr(row,col+COL2, WHITE, out);
   }
   col += strlen(out);

   sprintf(out, "%cEL    ", DEGREES);
   if(sort_by == 'E') vidstr(row,col, GREEN, out);
   else               vidstr(row,col, WHITE, out);
   if(sat_cols > 1) {
      if(sort_by == 'E') vidstr(row,col+COL2, GREEN, out);
      else               vidstr(row,col+COL2, WHITE, out);
   }
   col += strlen(out);

   sprintf(out, "%s  ", level_type);
   if(sort_by == 'S') vidstr(row,col, GREEN, out);
   else               vidstr(row,col, WHITE, out);
   if(sat_cols > 1) {
      if(sort_by == 'S') vidstr(row,col+COL2, GREEN, out);
      else               vidstr(row,col+COL2, WHITE, out);
   }
   col += strlen(out);
   last_sat_row = row;

   if(have_doppler && (sat_cols <= 1)) {
      sprintf(out, " DOPPLER ");
      if(sort_by == 'D') vidstr(row,col, GREEN, out);
      else               vidstr(row,col, WHITE, out);
      if(sat_cols > 1) {
         if(sort_by == 'D') vidstr(row,col+COL2, GREEN, out);
         else               vidstr(row,col+COL2, WHITE, out);
      }
      col += strlen(out);
   }
   out[0] = 0;

   if(show_short_info == 0) {
      if     (have_phase == 2) strcat(out, "  STATE   ");
      else if(have_phase == 4) strcat(out, " STATE   ");
      else if(have_phase)      strcat(out, "   PHASE  ");

      if(have_range)           strcat(out, "PSEUDORANGE ");
      else if(have_bias)       strcat(out, " CLOCK BIAS ");

      if(have_accu)       strcat(out, " URA(m)");
   }
   else if(have_phase == 4) {
      strcat(out, "STATE ");
   }

   vidstr(row, col, WHITE, out);
   col += strlen(out);
   top_row = row;

   ++row;

   if(col < 40) {   // we don't have much info to display
      show_short_info = 1;  // ... so allow max sig levels to show
   }
   col = i;


   // now draw the sat info table
   j = 0;
   max_len = 0;
   datum_crapum = 0;
   crap_count = 0;

   sort_sat_vals();

   old_row = row;        // erase the sat info display
   old_col = col;
   if(zoom_screen == 'I') ;
   else if(sat_cols > 1) {
      i = max_sat_display;
      if(i < (8*2)) i = (8*2);
      for(j=1; j<=i; j++) {  // show spaces for unused satellites
         if(all_adevs && (j > 8)) break;  // only room for 8 sats here

         vidstr(row, col, WHITE, &blanks[TEXT_COLS-COL2]);
         if(small_font == 1) vidstr(row, col, WHITE, "  ..");
         else                vidstr(row, col, WHITE, " ..");
         ++row;

         if(j == col_switch) {
            col = col2;
            row = top_row+1;
         }
         if((col >= col2) && (row > (top_row+rows_avail))) break;
      }
   }
   else {
      for(j=1; j <= max_sats; j++) {  // show spaces for unused satellites
         if(all_adevs && (j > 8)) break;  // only room for 8 sats here
         if((col < (TEXT_COLS/2)) && (row >= MOUSE_ROW)) break;

         vidstr(row, col, WHITE, &blanks[TEXT_COLS-FILTER_COL+1]);
         if(small_font == 1) vidstr(row, col, WHITE, "  ..");
         else                vidstr(row, col, WHITE, " ..");
         ++row;
      }
   }

   col = old_col;
   row = old_row;
   j = 0;

   for(prn=1; prn<=MAX_PRN; prn++) {
      if(sat[prn].level_msg == 0x00) continue;
      if(zoom_screen == 'I') ;
      else if(tracked_only && (sat[prn].tracking <= 0)) continue;

      if     (sort_by == SORT_ELEV)    i = sat_sort[j].prn;
      else if(sort_by == SORT_SIGS)    i = sat_sort[j].prn;
      else if(sort_by == SORT_AZ)      i = sat_sort[j].prn;
      else if(sort_by == SORT_PRN)     i = sat_sort[j].prn;
      else if(sort_by == SORT_DOPPLER) i = sat_sort[j].prn;
      else                             i = prn;

      if(zoom_screen == 'I') {
         ++j;
      }
      else if(sat_cols > 1) {
         if(++j > max_sat_display) break;
         if(all_adevs && (j > 8)) break;  // only room for 8 sats here
      } 
      else {
         if(++j > max_sats) break;
         if(all_adevs && (j > 8)) break;  // only room for 8 sats here
         if((col < (TEXT_COLS/2)) && (row >= MOUSE_ROW)) break;
      }

      // filter out potentially bogus data
      az = sat[i].azimuth;
      if(az != 0.0) ++datum_crapum;
      if((az < 0.0) || (az > 360.0)) az = 0.0;

      el = sat[i].elevation;
      if(el != 0.0) ++datum_crapum;
      if((el < 0.0) || (el > 90.0)) el = 0.0;

      el_dir = sat[i].el_dir;
      if(el_dir == 0) el_dir = ' ';
      else if(el == 0.0) el_dir = ' ';

      dop = sat[i].doppler;
      if((dop <= -10000.0) || (dop >= 10000.0)) dop = 0.0;

      phase = sat[i].code_phase;
      sig = sat[i].sig_level;
      if(rcvr_type == SCPI_RCVR) {
         if(sig > 255.0F) sig = 0.0F;
         if(sig <= (0.0F)) sig = 0.0F;
      }
      else {
         if(sig > 100.0F) sig = 0.0F;
         if(sig <= (-100.0F)) sig = 0.0F;
         if(sig < 0.0F) sig = 0.0F - sig;
      }

      clock = sat[i].sat_bias;

      acc = sat[i].sv_accuracy;

      out[0] = 0;

      if(el < 10.0F) sprintf(el_string, " %-3.1f", el);
      else           sprintf(el_string, "%-4.1f", el);
      if(el_string[3] == ' ') el_string[3] = el_dir;
      else                    el_string[4] = el_dir;
      el_string[5] = 0;

      if(have_range)        sprintf(bias_string, "%11.7f", sat[i].range);
      else if(clock == 0.0) strcpy(bias_string,  " 0.000e+000");
      else                  sprintf(bias_string, "%#11.4g", clock);

      
      // show the sat info columns
      if(i > 99) sprintf(out, "%03d  %5.1f  %s %5.1f", i, az, el_string, sig);
      else       sprintf(out, " %02d  %5.1f  %s %5.1f", i, az, el_string, sig);

//sprintf(fmt, "{%d} ", sat[i].level_msg);  // nnnnnnn
//strcat(out, fmt);

      if(have_doppler && (sat_cols <= 1)) {
         sprintf(fmt, " %8.2f", dop);
         strcat(out, fmt);
      }

      if(show_short_info == 0) {
         if(have_phase == 4) {
            sprintf(fmt, "   %04X  ", (int) phase);
            strcat(out, fmt);
         }
         else if(have_phase == 2) {
            if     (phase == 0) strcpy(fmt, "  SEARCH  ");
            else if(phase == 1) strcpy(fmt, "  CODE ACQ");
            else if(phase == 2) strcpy(fmt, "  SET AGC ");
            else if(phase == 3) strcpy(fmt, "  FREQ ACQ");
            else if(phase == 4) strcpy(fmt, "  BIT SYNC");
            else if(phase == 5) strcpy(fmt, "  MSG SYNC");
            else if(phase == 6) strcpy(fmt, "  TIME ACQ");
            else if(phase == 7) strcpy(fmt, "  EPH ACQ ");
            else if(phase == 8) strcpy(fmt, "  TRACKING");
            else                strcpy(fmt, "  UNKNOWN ");
            strcat(out, fmt);
         }
         else if(have_phase) {
            if     (phase <= (-1000000.0F)) sprintf(fmt, " %9ld", (long)phase);
            else if(phase <= (-100000.0F))  sprintf(fmt, " %9.1f", phase);
            else if(phase >= (10000000.0F)) sprintf(fmt, " %9ld", (long)phase);
            else if(phase >= (1000000.0F))  sprintf(fmt, " %9.1f", phase);
            else                            sprintf(fmt, " %9.2f", phase);
            strcat(out, fmt);
         }

         if(have_bias || have_range) {
            sprintf(fmt, " %s", bias_string);
            strcat(out, fmt);
         }

         if(have_accu) {
            if(acc <= 0.0)           sprintf(fmt, "   UNKN");
            else if(acc >= 10000.0F) sprintf(fmt, "  LARGE");
            else if(acc >= 1000.0F)  sprintf(fmt, "  %5.0f", acc);
            else if(acc >= 100.0F)   sprintf(fmt, "  %5.1f", acc);
            else                     sprintf(fmt, "  %5.2f", acc);
            strcat(out, fmt);
         }
      }
      else if(1 && (have_phase == 4)) {  // SIRF tracking flags
         sprintf(fmt, "  %04X  ", (int) phase);
         strcat(out, fmt);
      }

      if((sat_cols > 1) && (j == (col_switch+1))) {
         col = col2;
         row = top_row+1;
      }

      if(sat[i].disabled) {
         vidstr(row++, col, BLUE, out);
      }
      else if(sat[i].health_flag == 1) {  // unhealthy sat
         vidstr(row++, col, RED, out);
      }
      else if(sat[i].tracking <= 0) {  
         vidstr(row++, col, YELLOW, out);
      }
      else {
         vidstr(row++, col, GREEN, out);
         ++crap_count;
      }
      last_sat_row = (row+1);

      len = strlen(out);
      if(len > max_len) max_len = len;
   }

   if((rcvr_type == TSIP_RCVR) && (tsip_type == 0)) {  // all 0 az el values on tracked sats is what a STARLOC_RCVR does... BASTARDS!
      if(have_sat_azel && (datum_crapum == 0) && (crap_count > 2)) {
         tsip_type = STARLOC_RCVR;
         config_msg_ofs();
         have_el_mask = have_amu = 0;
      }
   }

   find_sat_changes();

   print_using = 0;
   blank_underscore = 0;

#ifdef SIG_LEVELS
   if(show_short_info && (sat_cols <= 1)) {
      if(1 || digital_clock_shown) {
//       show_sun_moon(DIS_ROW+0+eofs+5, DIS_COL);
         show_sun_moon(DIS_ROW+0+eofs+4, DIS_COL);
      }
      else {
         show_max_signals();
      }
   }
#endif
}

char *maidenhead_square(double Lat, double Long)
{
static char mls[16];
int i;
int Ext;
Ext = 2;

   // calculate the Maidenhead grid square
   //
   // Based on code by Yves Goergen, http://unclassified.software/source/maidenheadlocator
   // which was based upon code by Dirk Koopman, G1TLH

   Lat = Lat * 180.0 / PI;
   Long = Long * 180.0 / PI;

   i = 0;
   mls[i] = 0;

   Lat += 90.0;
   Long += 180.0;

   mls[i++] = (char) ('A' + (int) (Long / 20.0));
   mls[i++] = (char) ('A' + (int) (Lat / 10.0));

   Long = fmod(Long, 20.0);
   if(Long < 0.0) Long += 20.0;
   Lat = fmod(Lat, 10.0);
   if(Lat < 0.0) Lat += 10.0;

   mls[i++] = (char) ('0' + (int)(Long / 2.0));
   mls[i++] = (char) ('0' + (int)(Lat / 1.0));
   Long = fmod(Long, 2.0);
   if(Long < 0.0) Long += 2.0;
   Lat = fmod(Lat, 1.0);
   if(Lat < 0.0) Lat += 1.0;

   mls[i++] = (char) ('a' + (int)(Long * 12.0));
   mls[i++] = (char) ('a' + (int)(Lat * 24.0));
   Long = fmod(Long, 1.0 / 12.0);
   if(Long < 0) Long += 1.0/12.0;
   Lat = fmod(Lat, 1.0 / 24.0);
   if(Lat < 0.0) Lat += 1.0/24.0;
   mls[i] = 0;

   if(Ext >= 1) {
      mls[i++] = ' ';
      mls[i++] = (char) ('0' + (int)(Long * 120.0));
      mls[i++] = (char) ('0' + (int)(Lat * 240.0));
      Long = fmod(Long, 1.0/120.0);
      if(Long < 0.0) Long += 1.0 / 120.0;
      Lat = fmod(Lat, 1.0 / 240.0);
      if(Lat < 0.0) Lat += 1.0 / 240.0;
      mls[i] = 0;
   }

   if(Ext >= 2) {
      mls[i++] = ' ';
      mls[i++] = (char) ('A' + (int)(Long * 120.0 * 24.0));
      mls[i++] = (char) ('A' + (int)(Lat * 240.0 * 24.0));
      Long = fmod(Long, 1.0 / 120.0 / 24.0);
      if(Long < 0) Long += 1.0 / 120.0 / 24.0;
      Lat = fmod(Lat, 1.0 / 240.0 / 24.0);
      if(Lat < 0) Lat += 1.0 / 240.0 / 24.0;
      mls[i] = 0;
   }

   return &mls[0];
}


//  UTM conversion code based upon code written by Chuck Gantz- chuck.gantz@globalstar.com
double UTMNorthing;
double UTMEasting;
int UTM_Zone;
char utm_zone_string[32];
static char mgrs_square[3];

int utm_lon_zone(double dLat, double dLon)
{
int zone;

   // longitude must be between -180.00 .. 179.9
   if((dLon < -180.0) || (dLon > 180.0)) return (0); // implies failure: 0 is an invalid utm zone

   zone = (int)((180.0 + dLon) / 6.0 + 1.0);

   if((dLat >= 56.0) && (dLat < 64.0)) { // sw Norge (ie, zone 32V)
     if((dLon >= 3.0) && (dLon < 12.0)) zone = 32;
   }
   else if((dLat >= 72.0 && dLat < 84.0)) {  // special zones for Svalbard
     if     ((dLon >= 0.0)  && (dLon < 9.0))  zone = 31;
     else if((dLon >= 9.0)  && (dLon < 21.0)) zone = 33;
     else if((dLon >= 21.0) && (dLon < 33.0)) zone = 35;
     else if((dLon >= 33.0) && (dLon < 42.0)) zone = 37;
   }

   return zone;
}


char utm_lat_zone(double lat, double lon)
{
   if(lat >= 84.0) { // north polar
      if(lon >= 0.0)                        return 'Z'; // east
      else                                  return 'Y'; // west
   }
   else if((84.0 >= lat) && (lat >= 72.0))  return 'X';
   else if((72.0 > lat)  && (lat >= 64.0))  return 'W';
   else if((64.0 > lat)  && (lat >= 56.0))  return 'V';
   else if((56.0 > lat)  && (lat >= 48.0))  return 'U';
   else if((48.0 > lat)  && (lat >= 40.0))  return 'T';
   else if((40.0 > lat)  && (lat >= 32.0))  return 'S';
   else if((32.0 > lat)  && (lat >= 24.0))  return 'R';
   else if((24.0 > lat)  && (lat >= 16.0))  return 'Q';
   else if((16.0 > lat)  && (lat >= 8.0))   return 'P';
   else if((8.0 > lat)   && (lat >= 0.0))   return 'N';
   else if((0.0 > lat)   && (lat >= -8.0))  return 'M';
   else if((-8.0 > lat)  && (lat >= -16.0)) return 'L';
   else if((-16.0 > lat) && (lat >= -24.0)) return 'K';
   else if((-24.0 > lat) && (lat >= -32.0)) return 'J';
   else if((-32.0 > lat) && (lat >= -40.0)) return 'H';
   else if((-40.0 > lat) && (lat >= -48.0)) return 'G';
   else if((-48.0 > lat) && (lat >= -56.0)) return 'F';
   else if((-56.0 > lat) && (lat >= -64.0)) return 'E';
   else if((-64.0 > lat) && (lat >= -72.0)) return 'D';
   else if((-72.0 > lat) && (lat >= -80.0)) return 'C';
   else if (lat <= (-80.0)) { // south polar
      if(lon >= 0.0)                        return 'B'; // east
      else                                  return 'A'; // west
   }
   else                                     return 'Z';
}

void calc_mgrs_square(int set, int row, int col) 
{
char *l1, *l2;

   //
   // Retrieve the Square Identification (two-character letter code), for the
   // given row, column and set identifier (set refers to the zone set:
   // zones 1-6 have a unique set of square identifiers; these identifiers are
   // repeated for zones 7-12, etc.)
   // See p. 10 of the "United States National Grid" white paper for a diagram
   // of the zone sets.
   // 

   if((set < 0) || (row < 0) || (row > 19) || (col < 0) || (col > 7)) {
      strcpy(mgrs_square, "??");
      return;
   }

   // handle case of last row
   if(row == 0) row = 19;
   else         row -= 1;

   // handle case of last column
   if(col == 0)  col = 7;
   else col -= 1;

   set = (set % 6);
   if(set == 0) set = 6;

   switch (set) {
      case 1:
          l1 = "ABCDEFGH";              // column ids
          l2 = "ABCDEFGHJKLMNPQRSTUV";  // row ids
          break;

      case 2:
          l1 = "JKLMNPQR";
          l2 = "FGHJKLMNPQRSTUVABCDE";
          break;

      case 3:
          l1 = "STUVWXYZ";
          l2 = "ABCDEFGHJKLMNPQRSTUV";
          break;

      case 4:
          l1 = "ABCDEFGH";
          l2 = "FGHJKLMNPQRSTUVABCDE";
          break;

      case 5:
          l1 = "JKLMNPQR";
          l2 = "ABCDEFGHJKLMNPQRSTUV";
          break;

      case 6:
          l1 = "STUVWXYZ";
          l2 = "FGHJKLMNPQRSTUVABCDE";
          break;

      default:
          l1 = "????????";
          l2 = "????????????????????";
          break;
   }

   mgrs_square[0] = l1[col];
   mgrs_square[1] = l2[row];
   mgrs_square[2] = 0;
}


#define DMS_TO_DEC(d,m,s) ((d+(m/60.0)+(s/3600.0))*PI/180.0)

void lla_to_ups(double lat, double lon)
{
double c0;
double tz;
double R;
double dlat;

   // convert lat/lon to Universal Polar Stereographic coords

   dlat = lat;

   lat = fabs(lat);  // !!!!!!! is this correct?

   c0 = (1.0-WGS84_E) / (1.0+WGS84_E);
   c0 = pow(c0, WGS84_E/2.0);
   c0 *= ((2.0*WGS84_A) / sqrt(1.0-eccSquared));

   tz = (1.0 + WGS84_E*sin(lat)) / (1.0 - WGS84_E*sin(lat));
   tz = pow(tz, WGS84_E/2.0);
   tz *= tan(PI/4.0 - lat/2.0);

   R = 0.994 * c0 * tz;

   if(dlat >= 0.0) UTMNorthing = 2000000.0 - R*cos(lon);
   else            UTMNorthing = 2000000.0 + R*cos(lon);

   UTMEasting = 2000000.0 + R*sin(lon);
//sprintf(plot_title, "c0:%f tz:%f R:%f N:%f E:%f", c0,tz,R, UTMNorthing,UTMEasting);
}


int lla_to_utm(double dLatRad, double dLonRad)
{
double dLat;
double dLon;
double dLonWork;
double dCentralMeridian_Rad;
double N;
double T;
double K;
double A;
double M;
int row,col;

   // convert radians to degrees
   dLat = dLatRad * 180.0/PI;
   dLon = dLonRad * 180.0/PI;

   // use dLonWork to make sure the longitude is between -180.00 .. 179.9
   dLonWork = (dLon + 180.0) - ((int)((dLon + 180.0) / 360.0)) * 360.0 - 180.0; // -180.00 .. 179.9;
   // convert degrees to radians
   dLonRad = dLonWork * PI / 180.0;

   // set the UTM zone string
   UTM_Zone = utm_lon_zone(dLat, dLonWork);
   sprintf(utm_zone_string, "%02d%c", UTM_Zone, utm_lat_zone(dLat, dLon));

   if((dLat >= 84.0) || (dLat <= (-80.0))) {  // polar regions
      lla_to_ups(dLatRad, dLonRad);
   }
   else { // normal regions
      // set central meridian
      dCentralMeridian_Rad = ((183.0 - (6.0 * (double)UTM_Zone)) * -1.0) * PI / 180.0;

      N = WGS84_A / sqrt(1.0 - eccSquared * sin(dLatRad) * sin(dLatRad));
      T = tan(dLatRad) * tan(dLatRad);
      K = eccPrimeSquared * cos(dLatRad) * cos(dLatRad);
      A = cos(dLatRad) * (dLonRad - dCentralMeridian_Rad);
      M = WGS84_A * ((1.0 - eccSquared / 4.0 - 3.0 * eccSquared * eccSquared / 64.0 - 5.0 * eccSquared * eccSquared * eccSquared / 256.0) * dLatRad
          - (3.0 * eccSquared / 8.0 + 3.0 * eccSquared * eccSquared / 32.0 + 45.0 * eccSquared * eccSquared * eccSquared / 1024.0) * sin(2.0 * dLatRad)
          + (15.0 * eccSquared * eccSquared / 256.0 + 45.0 * eccSquared * eccSquared * eccSquared / 1024.0) * sin(4.0 * dLatRad)
          - (35.0 * eccSquared * eccSquared * eccSquared / 3072.0) * sin(6.0 * dLatRad));

      UTMEasting = (double)(dScaleFactor * N * (A + (1.0 - T + K) * A * A * A / 6.0
                 + (5.0 - 18.0 * T + T * T + 72.0 * K - 58.0 * eccPrimeSquared) * A * A * A * A * A / 120.0) + 500000.0);

      UTMNorthing = (double)(dScaleFactor * (M + N * tan(dLatRad) * (A * A / 2.0 + (5.0 - T + 9.0 * K + 4 * K * K) * A * A * A * A / 24.0
                  + (61.0 - 58.0 * T + T * T + 600.0 * K - 330.0 * eccPrimeSquared) * A * A * A * A * A * A / 720.0)));
      if(dLat < 0.0) UTMNorthing += 10000000.0; // 10000000 meter offset for southern hemisphere
   }

   col = (0);
   row = (0);
   strcpy(mgrs_square, "??");

   if(dms == NATO_FMT) {
      row = 1;
      while(UTMNorthing >= 100000.0) {
         UTMNorthing -= 100000.0;
         ++row;
      }
      row = (row % 20);
      UTMNorthing = (double) (int) UTMNorthing;

      col = 0;
      while(UTMEasting >= 100000.0) {
         UTMEasting -= 100000.0;
         ++col;
      }
      col = (col % 8);
      UTMEasting  = (double) (int) UTMEasting;

      calc_mgrs_square(UTM_Zone, row,col);
   }
//sprintf(plot_title, "utm: E:%f(%d) N:%f(%d)  %s  mgrs:%s", UTMEasting,col, UTMNorthing,row, utm_zone_string, mgrs_square);

   return (0);
}


void format_lla(double lat, double lon,  double alt,  int row, int col)
{
u08 color;
char s[4];
double x;
int deg, min;
double sec;
char *lat_s, *lon_s, *alt_s;

// test data for UPS projection
//lat = DMS_TO_DEC(84.0, 17.0, 14.024);
//lon = 0.0-DMS_TO_DEC(132.0, 14.0, 52.761);

//lat = DMS_TO_DEC(73.0, 0.0, 0.0);
//lon = DMS_TO_DEC(44.0, 0.0, 0.0);

//lat = -DMS_TO_DEC(87.0, 17.0, 14.400);
//lon = DMS_TO_DEC(132.0, 14.0, 52.303);
// lon *= (-1.0);


   if(luxor) return;
   if(text_mode && first_key) return;
   if(zoom_screen == 'I') ;
   else if(zoom_screen == 'L') ;
   else if(zoom_screen) return;

   if((SCREEN_WIDTH <= 800) && (col > (TEXT_COLS/2))) {
      lat_s = "Lat:";
      lon_s = "Lon:";
      alt_s = "Alt:";
   }
   else {
      lat_s = "Lat: ";
      lon_s = "Lon: ";
      alt_s = "Alt: ";
   }

   color = WHITE;
   if(minor_alarms & 0x0200) color = YELLOW;
   else if(minor_alarms & 0x0020) {  // self-survey in progress
      color = YELLOW;
   }

   if(lat < 0.0) s[0] = s[2] = 'S';
   else          s[0] = s[2] = 'N';

   if(dms == DMS_FMT) s[1] = 0;
   else if((dms == UTM_FMT) || (dms == NATO_FMT)) {
      lla_to_utm(lat, lon);
   }
   else if(dms == GRAD_FMT) {
      s[0] = 'g';
      s[1] = ' ';
      s[3] = 0;
   }
   else if(dms == MIL_FMT) {
      s[0] = 'i';
      s[1] = ' ';
      s[3] = 0;
   }
   else if(dms == RADIAN_FMT) {
      s[0] = 'r';
      s[1] = ' ';
      s[3] = 0;
   }
   else {
      s[0] = DEGREES;
      s[1] = ' ';
      s[3] = 0;
   }

   if(dms == ECEF_FMT) {
      lla_to_ecef(lat, lon, alt);
      lat_s = "X:";
      lon_s = "Y:";
      alt_s = "Z:";
   }

   if(dms == RADIAN_FMT) x = DABS(lat);
   else if(dms == GRAD_FMT) x = DABS(lat*200.0 / PI);
   else if(dms == MIL_FMT) x = DABS(lat*3200.0 / PI);
   else if(dms == ECEF_FMT) x = ecef_x;
   else x = DABS(lat*RAD_TO_DEG);

   if(plot_loc == 0) sprintf(out, "%s (private)    ", lon_s); // (double) (int) x;
   else if(dms == GRIDSQ_FMT) {
      sprintf(out, "MLS: %s", maidenhead_square(lat, lon));
   }
   else if(dms == UTM_FMT) {
      sprintf(out, "%s:%13.4lf E", utm_zone_string, UTMEasting);
   }
   else if(dms == NATO_FMT) {
      sprintf(out, "%s %s      %05d E", utm_zone_string, mgrs_square, (int) UTMEasting);
   }
   else if(dms == DMS_FMT) {
      deg = (int) x;
      min = (int) (60.0 * ((double) x - (double) deg));
      sec = ((x - (double) deg) - ((double) min / 60.0)) * 3600.0;
      sprintf(out, "%s%2d%c%02d'%06.3f\"%s", lat_s, deg, DEGREES, min, sec, s);
   }
   else if(dms == ECEF_FMT) {
      if(x >= 0.0) sprintf(out, "%s %15.6lf  ", lat_s, x);
      else         sprintf(out, "%s%16.6lf  ", lat_s, x);
   }
   else if(dms == MIL_FMT) {
      sprintf(out, "%s%11.6lf%s", lat_s, x, s);
   }
   else sprintf(out, "%s%11.7lf%s", lat_s, x, s);
   vidstr(row+0, col, color, out);

   if(dms == DMS_FMT) {
      if(lon < 0.0) s[0] = 'W';
      else          s[0] = 'E';
   }
   else {
      if(lon < 0.0) s[2] = 'W';
      else          s[2] = 'E';
   }

   if(dms == RADIAN_FMT) x = DABS(lon);
   else if(dms == GRAD_FMT) x = DABS(lon*200.0 / PI);
   else if(dms == MIL_FMT) x = DABS(lon*3200.0 / PI);
   else if(dms == ECEF_FMT) x = ecef_y;
   else x = DABS(lon*RAD_TO_DEG);

   if(plot_loc == 0) sprintf(out, "%s (private)    ", lon_s); // (double) (int) x;
   else if(dms == GRIDSQ_FMT) {
      out[0] = 0;
   }
   else if(dms == UTM_FMT) {
      if((lat >= (84.0*PI/180)) || (lat <= (-80.0*PI/180.0))) {
         sprintf(out, "%s %13.4lf N", "UPS", UTMNorthing);
      }
      else {
         sprintf(out, "%s %13.4lf N", "UTM", UTMNorthing);
      }
   }
   else if(dms == NATO_FMT) {
      sprintf(out, "%s        %05d N", "MGRS", (int) UTMNorthing);
   }
   else if(dms == DMS_FMT) {
      deg = (int) x;
      min = (int) (60.0 * ((double) x - (double) deg));
      sec = ((x - (double) deg) - ((double) min / 60.0)) * 3600.0;
      if(deg >= 100) {
         lon_s = "Lon:";
         sprintf(out, "%s%3d%c%02d'%06.3f\"%s", lon_s, deg, DEGREES, min, sec, s);
      }
      else {
         sprintf(out, "%s%2d%c%02d'%06.3f\"%s", lon_s, deg, DEGREES, min, sec, s);
      }
   }
   else if(dms == ECEF_FMT) {
      if(x >= 0.0) sprintf(out, "%s %15.6lf  ", lon_s, x);
      else         sprintf(out, "%s%16.6lf  ", lon_s, x);
   }
   else if(dms == MIL_FMT) {
      sprintf(out, "%s%11.6lf%s", lon_s, x, s);
   }
   else sprintf(out, "%s%11.7lf%s", lon_s, x, s);
   vidstr(row+1, col, color, out);

   x = alt;
   if(alt_scale[0] == 'f') x *= (100.0/2.54/12.0); // convert meters to feet
   if(0 && (plot_loc == 0)) sprintf(out, "%s (private)      ", alt_s);  // (double) (int) x;
   else if(dms == UTM_FMT) {
      if(x >= 1000.0) {
         if(alt_scale[0] == 'f') sprintf(out, "%s%12.4lf'   ", alt_s, x);
         else                    sprintf(out, "%s%12.4lf m ",  alt_s, x);
      }
      else {
         if(alt_scale[0] == 'f') sprintf(out, "%s%12.4lf'   ", alt_s, x);
         else                    sprintf(out, "%s%12.4lf m ",  alt_s, x);
      }
   }
   else if(dms == ECEF_FMT) {
      if(x >= 0.0) sprintf(out, "%s %15.6lf  ", alt_s, ecef_z);
      else         sprintf(out, "%s%16.6lf  ", alt_s, ecef_z);
   }
   else {
      if(x >= 1000.0) {
         if(alt_scale[0] == 'f') sprintf(out, "%s%11.4lf'   ", alt_s, x);
         else                    sprintf(out, "%s%12.5lf m ",  alt_s, x);
      }
      else {
         if(alt_scale[0] == 'f') sprintf(out, "%s%11.7lf'   ", alt_s, x);
         else                    sprintf(out, "%s%12.8lf m ",  alt_s, x);
      }
   }
   if(dms == GRIDSQ_FMT) --row;
   vidstr(row+2, col, color, out);
}

void show_lla(int why)
{
   if(text_mode && first_key) return;
   if(zoom_screen) return;
   if(luxor) return;
   if(all_adevs && (all_adev_row < (ALL_ROW+5))) return;

   if(lat < (0.0-PI)) return;    // filter out any bogosity
   if(lat > PI) return;
   if(lon < (0.0-PI)) return;
   if(lon > PI) return;
// if(alt > 10000.0) return;
// if(alt < (0.0-1000.0)) return;

   // get a default precise position from the last self-surveyed coordinates
   if((have_initial_posn == 0) && (lla_log == 0) && (lat || lon || alt)) {  
      precise_lat = lat;
      precise_lon = lon;
      precise_alt = alt;

      #ifdef BUFFER_LLA
         clear_lla_points();
      #endif

      cos_factor = cos(lat);
      if(rcvr_mode == RCVR_MODE_HOLD) have_initial_posn = 1;
have_initial_posn = 1;  // ggggggggggg
   }
   
   #ifdef PRECISE_STUFF
      if(lla_file && (gps_status == 0) && have_time && (precision_survey || show_fixes)) {
         fprintf(lla_file, "%-6lu %d %13.8lf  %13.8lf  %8.3lf\n", 
         (unsigned long) tow, gps_status, lat*RAD_TO_DEG, lon*RAD_TO_DEG, alt);
      }

      // user requested a precision survey from the command line
      if(have_initial_posn && have_time && user_precision_survey) {
         if(user_precision_survey == 1) {  // start precise survey
            user_precision_survey = 0;
            start_precision_survey();
         }
         else {  // start 3D fix mode
            user_precision_survey = 0;
            start_3d_fixes(4, 20);
            config_screen(2202);
         }
      }
   #endif  // PRECISE_STUFF

   format_lla(lat, lon, alt,  POSN_ROW+1+eofs, POSN_COL);
}


void show_filter_status()
{
int row;
int rows_avail;
int rows_needed;
int show_dops;
int dop_count;
int show_filters;
int filter_count;
int color;
int do_dop;
int i;
char *s;

   // This routine draws the filter settings,  dilution of precision,  
   // and elevation/signal level masks

   if(text_mode && first_key) return;
   if(zoom_screen) return;
   if(luxor) return;
   if(all_adevs && (SCREEN_WIDTH < 1280)) return;
   if(rcvr_type == ACRON_RCVR) return;
   if(rcvr_type == NO_RCVR) return;

   dop_count = 0;
   if(have_dops & TDOP) ++dop_count;
   if(have_dops & HDOP) ++dop_count;
   if(have_dops & VDOP) ++dop_count;
   if(have_dops & PDOP) ++dop_count;
   if(rcvr_type == SCPI_RCVR) dop_count = 2;       // tfom and ffom treated as DOPs
   else if(rcvr_type == UCCM_RCVR) dop_count = 2;  // tfom and ffom treated as DOPs 

   show_dops = 0;
   do_dop = 0;
   if(dop_count) {
      if(plot_dops) do_dop = 1;
//    else if(doing_survey) do_dop = 2;
//    else if(rcvr_type == SCPI_RCVR) do_dop = 3;
//    else if((rcvr_mode != 7) && (rcvr_mode != 1) && have_rcvr_mode) do_dop = 4;
   }

   filter_count = 0;
   if(have_filter & PV_FILTER)     ++filter_count;
   if(have_filter & ALT_FILTER)    ++filter_count;
   if(have_filter & STATIC_FILTER) ++filter_count;
   if(have_filter & KALMAN_FILTER) ++filter_count;
   if(plot_filters == 0) filter_count = 0;

   rows_needed = dop_count + filter_count;
   if(have_amu) ++rows_needed;
   if(have_el_mask) ++rows_needed;
   ++rows_needed;  // for PPS info

   row = INFO_ROW;                          // 16
   if(rcvr_type == UBX_RCVR) row -= 1; // 15
   else if(rcvr_type == VENUS_RCVR) row -= 8;   // 13
   else if(rcvr_type == ZODIAC_RCVR) row -= 3;   // 13
   else if(res_t) row += 1;

   if(0 && (rcvr_type == VENUS_RCVR)) row = CRIT_ROW+2;  // ggggggg
   else if(!have_critical_alarms) row = CRIT_ROW+6;

   for(i=row; i<=view_row; i++) {  // erase old info
      vidstr(i,INFO_COL, WHITE, "             ");
   }

   rows_avail = (view_row - row) + 1;
//sprintf(debug_text, "rows avail:%d  rows_needed:%d  dopc:%d  fc:%d  view:%d  row:%d", 
//rows_avail, rows_needed, dop_count, filter_count, view_row, row); 

if(0 && (rcvr_type == TSIP_RCVR) && (rows_avail > rows_needed) && !res_t) {
   ++row;
   --rows_avail;
}

   if((all_adevs == 0) || (SCREEN_WIDTH >= 1280)) {
      if(rows_avail > rows_needed) { // pretty printing
         ++row;
         --rows_avail;
      }

      if(pps_enabled) {
         if((rcvr_type == UBX_RCVR) && (pps_rate == 0x82)) {
            if(pps_polarity) sprintf(out, "100PPS: ON %c", DOWN_ARROW);  //falling
            else             sprintf(out, "100PPS: ON %c", UP_ARROW);    //rising
            vidstr(row, INFO_COL, GREEN, out);
         }
         else if((rcvr_type == UBX_RCVR) && (pps_rate == 0x83)) {
            if(pps_polarity) sprintf(out, "1kPPS: ON %c ", DOWN_ARROW);  //falling
            else             sprintf(out, "1kPPS: ON %c ", UP_ARROW);    //rising
            vidstr(row, INFO_COL, GREEN, out);
         }
         else if((rcvr_type == UBX_RCVR) && (pps_rate == 0)) {
            if(pps_polarity) sprintf(out, "USRPPS: ON %c", DOWN_ARROW);  //falling
            else             sprintf(out, "USRPPS: ON %c", UP_ARROW);    //rising
            vidstr(row, INFO_COL, GREEN, out);
            vidstr(row, INFO_COL, GREEN, out);
         }
         else if((rcvr_type == MOTO_RCVR) && pps_rate) {
            if(pps_polarity) sprintf(out, "100PPS: ON %c", DOWN_ARROW);  //falling
            else             sprintf(out, "100PPS: ON %c", UP_ARROW);    //rising
            vidstr(row, INFO_COL, GREEN, out);
         }
         else if(pps_rate == 0x82) {
            if(pps_polarity) sprintf(out, "PP2S: ON %c  ", DOWN_ARROW);  //falling
            else             sprintf(out, "PP2S: ON %c  ", UP_ARROW);    //rising
            vidstr(row, INFO_COL, GREEN, out);
         }
         else if((rcvr_type == STAR_RCVR) || (rcvr_type == ZODIAC_RCVR)) {
            if(minor_alarms & 0x1000) {
               sprintf(out, "PPS: ERROR  "); 
            }
            else if(pps_polarity) sprintf(out, "PPS: ON %c   ", DOWN_ARROW);  //falling
            else                  sprintf(out, "PPS: ON %c   ", UP_ARROW);    //rising
            if(minor_alarms & 0x1000) vidstr(row, INFO_COL, RED, out);   // traim failure
            else                      vidstr(row, INFO_COL, GREEN, out);

            if(have_pps_rate) {
               ++row;
               if(star_tod == 1) vidstr(row, INFO_COL, GREEN,  "TOD: ON     ");
               else if(star_tod) vidstr(row, INFO_COL, RED,    "TOD: UNKN   ");
               else              vidstr(row, INFO_COL, YELLOW, "TOD: OFF    ");
            }
         }
         else {
            if(pps_polarity) sprintf(out, "PPS: ON %c   ", DOWN_ARROW);  //falling
            else             sprintf(out, "PPS: ON %c   ", UP_ARROW);    //rising
            vidstr(row, INFO_COL, GREEN, out);
         }
      }
      else if((rcvr_type != NMEA_RCVR) && (rcvr_type != GPSD_RCVR) && (rcvr_type != NO_RCVR) && (rcvr_type != ACRON_RCVR)) {
         sprintf(out, "PPS: OFF    ");
         vidstr(row, INFO_COL, YELLOW, out);
      }
      ++row;
      --rows_avail;
   }

   if(do_dop) { 
      if(rows_avail > rows_needed) {  // pretty printing
         ++row;
         --rows_avail;
      }
      if(rows_avail >= dop_count) {
         show_dops = row;
      }
   }

   if(show_dops && ((rcvr_type == SCPI_RCVR) || (rcvr_type == UCCM_RCVR))) {
      if(rows_avail > 0) {
         if(tfom <= 3) color = GREEN;
         else if(tfom <= 6) color = YELLOW;
         else color = RED;
         sprintf(out, "TFOM:    %d", tfom);
         vidstr(row, FILTER_COL, color, out);
         ++row;
         --rows_avail;
      }

      if(rows_avail > 0) {
         if(ffom == 0) color = GREEN;
         else if(ffom >= 3) color = RED;
         else color = YELLOW;
         if     (ffom == 3) sprintf(out, "PLL:  INIT");
         else if(ffom == 2) sprintf(out, "PLL:UNLOCK");
         else if(ffom == 1) sprintf(out, "PLL:SETTLE");
         else if(ffom == 0) sprintf(out, "PLL:  LOCK");
         else               sprintf(out, "FFOM:%5d", ffom);
         vidstr(row, FILTER_COL, color, out);
         ++row;
         --rows_avail;
      }
   }
   else if(show_dops) {
      if(rows_avail > 0) {
         sprintf(out, "PDOP: %5.2f  ", pdop);
         if((rcvr_type == UBX_RCVR) && (rcvr_mode == RCVR_MODE_HOLD)) ;
         else if(have_dops & PDOP) {
            if(pdop <= 0.0F) vidstr(row, FILTER_COL, GREY,      out);
            else             vidstr(row, FILTER_COL, DOP_COLOR, out);
            ++row;
            --rows_avail;
         }
      }

      if(rows_avail > 0) {
         sprintf(out, "HDOP: %5.2f  ", hdop);
         if((rcvr_type == UBX_RCVR) && (rcvr_mode == RCVR_MODE_HOLD)) ;
         else if(have_dops & HDOP) {
            if(hdop <= 0.0F) vidstr(row, FILTER_COL, GREY,      out);
            else             vidstr(row, FILTER_COL, DOP_COLOR, out);
            ++row;
            --rows_avail;
         }
      }

      if(rows_avail > 0) {
         sprintf(out, "VDOP: %5.2f  ", vdop);
         if((rcvr_type == UBX_RCVR) && (rcvr_mode == RCVR_MODE_HOLD)) ;
         else if(have_dops & VDOP) {
            if(vdop <= 0.0F) vidstr(row, FILTER_COL, GREY,      out);
            else             vidstr(row, FILTER_COL, DOP_COLOR, out);
            ++row;
            --rows_avail;
         }
      }

      if(rows_avail > 0) {
         sprintf(out, "TDOP: %5.2f  ", tdop);
         if(have_dops & TDOP) {
            if(tdop <= 0.0F) vidstr(row, FILTER_COL, GREY,      out);
            else             vidstr(row, FILTER_COL, DOP_COLOR, out);
            ++row;
            --rows_avail;
         }
      }

      if((rcvr_type == MOTO_RCVR) || (rcvr_type == UBX_RCVR) || (rcvr_type == VENUS_RCVR)) {
         if(rows_avail > 0) {
            sprintf(out, "GDOP: %5.2f  ", gdop);
            if((rcvr_type == UBX_RCVR) && (rcvr_mode == RCVR_MODE_HOLD)) ;
            else if(have_dops & GDOP) {
               if(gdop <= 0.0F) vidstr(row, FILTER_COL, GREY,      out);
               else             vidstr(row, FILTER_COL, DOP_COLOR, out);
               ++row;
               --rows_avail;
            }
         }

         if(rows_avail > 0) {
            sprintf(out, "NDOP: %5.2f  ", ndop);
            if((rcvr_type == UBX_RCVR) && (rcvr_mode == RCVR_MODE_HOLD)) ;
            else if(have_dops & NDOP) {
               if(ndop <= 0.0F) vidstr(row, FILTER_COL, GREY,      out);
               else             vidstr(row, FILTER_COL, DOP_COLOR, out);
               ++row;
               --rows_avail;
            }
         }

         if(rows_avail > 0) {
            sprintf(out, "EDOP: %5.2f  ", edop);
            if(have_dops & EDOP) {
               if(edop <= 0.0F) vidstr(row, FILTER_COL, GREY,      out);
               else             vidstr(row, FILTER_COL, DOP_COLOR, out);
               ++row;
               --rows_avail;
            }
         }
      }

      // !!!! GDOP NDOP EDOP
   }

   if(filter_count == 0) show_filters = 0;
   else if(rcvr_type == SCPI_RCVR) show_filters = 0;
   else if(rcvr_type == UCCM_RCVR) show_filters = 0;
   else if(rcvr_type == ZODIAC_RCVR) show_filters = row;
   else if(rows_avail > rows_needed) {  // pretty printing
      ++row;
      --rows_avail;
      show_filters = row;
   }
   else if(rows_avail >= filter_count) show_filters = row;
   else show_filters = 0;

   if(rcvr_type == GPSD_RCVR) return;
   else if(rcvr_type == NMEA_RCVR) return;
   else if(show_filters) {
      if(have_filter & PV_FILTER) {
         sprintf(out, "PV:      %s", pv_filter?"ON ":"OFF");
         vidstr(row, FILTER_COL, pv_filter?WHITE:YELLOW, out);
         ++row;
         --rows_avail;
      }

      if(have_filter & STATIC_FILTER) {
         if(rcvr_type == MOTO_RCVR) sprintf(out, "IONO:    %s", static_filter?"ON ":"OFF");
         else                       sprintf(out, "STATIC:  %s", static_filter?"ON ":"OFF");
         vidstr(row, FILTER_COL, static_filter?WHITE:YELLOW, out);
         ++row;
         --rows_avail;
      }

      if(have_filter & ALT_FILTER) {
         if(rcvr_type == MOTO_RCVR) sprintf(out, "TROPO:   %s", alt_filter?"ON ":"OFF"); 
         else                       sprintf(out, "ALTITUDE:%s", alt_filter?"ON ":"OFF"); 
         vidstr(row, FILTER_COL, alt_filter?WHITE:YELLOW, out);
         ++row;
         --rows_avail;
      }

      if(have_filter & KALMAN_FILTER) {
         if(ebolt || saw_kalman_on) {
            sprintf(out, "KALMAN:  %s", kalman_filter?"ON ":"OFF"); 
            vidstr(row, FILTER_COL, kalman_filter?WHITE:YELLOW, out);
         }
         else {
            sprintf(out, "KALMAN:  %s", kalman_filter?"ON ":"N/A"); 
            vidstr(row, FILTER_COL, kalman_filter?YELLOW:GREY, out);
         }
         ++row;
         --rows_avail;
      }

      if(have_filter & MARINE_FILTER) {  // Motorola marine velocity filter
         printf(out, "MARINE:  %3d", marine_filter);
         vidstr(row, FILTER_COL, pv_filter?WHITE:YELLOW, out);
         ++row;
         --rows_avail;
      }
   }

   if(rows_avail > 2) { // pretty printing
      ++row; 
      --rows_avail; 
   }

   if(rows_avail >= 1) {
      if(have_el_mask) {
         sprintf(out, "EL:  %6.2f%c", el_mask, DEGREES);
         if(rcvr_type == SCPI_RCVR) ++show_filters;
         else if(rcvr_type == UCCM_RCVR) ++show_filters;
         vidstr(row, FILTER_COL, WHITE, out);
         ++row;
         rows_avail -= 1;
      }
   }

   if(rows_avail >= 1) {
      if(have_amu) {
         if((res_t && (res_t != RES_T)) || (rcvr_type != TSIP_RCVR)) {
            if(level_type == 0) s = "SNR";
            else if(level_type[0] == 0) s = "SNR";
            else if(level_type[0] == ' ') s = "SNR";
            else s = level_type;
            sprintf(out, "%s: %6.2f", s, amu_mask);
         }
         else if(1 || amu_mode) {
            sprintf(out, "AMU: %6.2f", amu_mask);
         }
         else {
            sprintf(out, "dBc: %6.2f", amu_mask);
         }
         vidstr(row, FILTER_COL, WHITE, out);
         ++row;
         --rows_avail;
      }
   }
}


void show_unit_info()
{
   if(unit_name[0] == 0) {
      sprintf(out, "App: %2d.%-2d  %02d %s %02d", 
         ap_major, ap_minor,  ap_day, months[ap_month], ap_year);   //!!! docs say 1900
      vidstr(VER_ROW+1, VER_COL, WHITE, out);
   }

   sprintf(out, "GPS: %2d.%-2d  %02d %s %02d", 
      core_major, core_minor,  core_day, months[core_month], core_year);  //!!! docs say 1900
   vidstr(VER_ROW+2, VER_COL, WHITE, out);
}

void show_gpsd_driver()
{
   if(zoom_screen) return;

   sprintf(gpsd_driver, "Drv:  %s", &nmea_field[1]);
   vidstr(VER_ROW+2, VER_COL, WHITE, gpsd_driver);
}


void show_manuf_params()
{
   have_info |= MANUF_PARAMS;

   if(text_mode && first_key) return;
   if(zoom_screen) return;
   if(luxor) return;
   if(just_read) return;

   if(build_ok) {
      sprintf(out, "Mfg: %02d:00  %02d %s %04d", 
         build_hour, build_day, months[build_month], build_year);
      vidstr(VER_ROW+3, VER_COL, WHITE, out);
   }
}


void show_ebolt_info()
{
   sprintf(out, "Mfg: %02d:00  %02d %s %04d", 
      rev_hour, rev_day, months[rev_month], rev_year);
   vidstr(VER_ROW+3, VER_COL, WHITE, out);

#ifdef WINDOWS
   if(com_port >= 10)      sprintf(out, "Ser: %2u.%-8lu COM:%d",  hw_code, (unsigned long) ebolt_serno, com_port);
   else if (com_port)      sprintf(out, "Ser: %2u.%-8lu  COM:%d", hw_code, (unsigned long) ebolt_serno, com_port);
#else   // __linux__  __MACH__
   if(com_port >= 10)      sprintf(out, "Ser: %2u.%-8lu TTY:%d",  hw_code, (unsigned long) ebolt_serno, com_port-1);
   else if (com_port)      sprintf(out, "Ser: %2u.%-8lu  TTY:%d", hw_code, (unsigned long) ebolt_serno, com_port-1);
#endif
   else if(usb_port >= 10) sprintf(out, "Ser: %2u.%-8lu USB:%d",  hw_code, (unsigned long) ebolt_serno, usb_port-1);
   else if(usb_port)       sprintf(out, "Ser: %2u.%-8lu  USB:%d", hw_code, (unsigned long) ebolt_serno, usb_port-1);
#ifdef TCP_IP 
   else                    sprintf(out," IP: %s", IP_addr); // TCP 
#endif
   vidstr(VER_ROW+4, VER_COL, WHITE, out);
}


void show_version_header()
{
   if(zoom_screen) return;
   if(text_mode && first_key) return;

   if(measure_jitter) {
      if(no_poll) vidstr(VER_ROW+0, VER_COL, RED, "Jitter meas - no polling");
      else        vidstr(VER_ROW+0, VER_COL, RED, "Jitter measurement mode ");
   }
   else if(read_only) vidstr(VER_ROW+0, VER_COL, BLUE, "Read-only mode set      ");
   else if(no_send)   vidstr(VER_ROW+0, VER_COL, BLUE, "COM output disabled     ");
   else if(no_poll)   vidstr(VER_ROW+0, VER_COL, BLUE, "Rcvr requests disabled  ");
   else if(just_read) vidstr(VER_ROW+0, VER_COL, BLUE, "Rcvr decode disabled    ");
   else if(disable_kbd > 1) {
      if(script_file) {
         vidstr(VER_ROW+0, VER_COL, BLUE,    "Script file error        "); 
      }
      else {
         vidstr(VER_ROW+0, VER_COL, BLUE, "Keyboard disabled        "); 
      }
   }
   else if(disable_kbd) vidstr(VER_ROW+0, VER_COL, BLUE, "Press ESC Y to exit    ");
   else if(plot_version) {
       sprintf(out, "V%s - %s        ", VERSION, date_string);
       vidstr(VER_ROW+0, VER_COL, BLUE, out);
   }
   else vidstr(VER_ROW+0, VER_COL, BLUE, "Press SPACE for help    ");

// show_version_info();
}


void show_version_info()
{
char *s;

   if(zoom_screen) return;
   if(text_mode && first_key) return;
   if(luxor) return;

   if(rcvr_type == MOTO_RCVR) {
      if(moto_id[6][0]) {
         sprintf(out, "HW:   %-17s", &moto_id[6][11]);
         vidstr(VER_ROW+2, VER_COL, WHITE, out);
      }

      if(strstr(moto_id[1], "Trimble") && moto_id[3][0]) {
         sprintf(out, "SW:   %-17s", &moto_id[3][15]);
         vidstr(VER_ROW+3, VER_COL, WHITE, out);
      }
      else {
         if(moto_id[7][0]) {
            sprintf(out, "PN:   %-17s", &moto_id[7][11]);
            vidstr(VER_ROW+3, VER_COL, WHITE, out);
         }
      }
   }
   else if(rcvr_type == NVS_RCVR) {
      if(nvs_id[0]) {
         vidstr(VER_ROW+2, VER_COL, WHITE, nvs_id);
      }
      if(nvs_sn) {
         sprintf(out, "SN: %u", nvs_sn);
         vidstr(VER_ROW+3, VER_COL, WHITE, out);
      }
   }
   else if(rcvr_type == SIRF_RCVR) {
      s = strchr(sirf_sw_id, '_');
      if(s == 0) s = strchr(sirf_sw_id, ' '); 
      if(s) *s = 0;

      sprintf(out, "SW:   %-17s", sirf_sw_id);
      vidstr(VER_ROW+2, VER_COL, WHITE, out);
   }
   else if((rcvr_type == SCPI_RCVR) || (rcvr_type == UCCM_RCVR)) {
      if(scpi_mfg_id[0]) {
         vidstr(VER_ROW+2, VER_COL, WHITE, scpi_mfg_id);
      }
      if(scpi_serno[0]) {
         vidstr(VER_ROW+3, VER_COL, WHITE, scpi_serno);
      }
   }
   else if(rcvr_type == SIRF_RCVR) {
      s = strchr(sirf_sw_id, '_');
      if(s == 0) s = strchr(sirf_sw_id, ' '); 
      if(s) *s = 0;

      sprintf(out, "SW:   %-17s", sirf_sw_id);
      vidstr(VER_ROW+2, VER_COL, WHITE, out);
   }
   else if(rcvr_type == STAR_RCVR) {
      if(star_fw[0]) {
         sprintf(out, "FW:   %s", star_fw);
         vidstr(VER_ROW+2, VER_COL, WHITE, out);
      }
      if(star_osc[0]) {
         sprintf(out, "OSC:  %s", star_osc);
         vidstr(VER_ROW+3, VER_COL, WHITE, out);
      }
   }
   else if(rcvr_type == UBX_RCVR) {
      if(ubx_sw[0]) {
         sprintf(out, "FW:   %s", ubx_sw);
         vidstr(VER_ROW+2, VER_COL, WHITE, out);
      }

      if(ubx_hw[0]) {
         sprintf(out, "HW:   %s", ubx_hw);
         vidstr(VER_ROW+3, VER_COL, WHITE, out);
      }
   }
   else if(rcvr_type == VENUS_RCVR) {
      if(venus_odm[0]) {
         vidstr(VER_ROW+2, VER_COL, WHITE, venus_odm);
      }
      if(venus_rev[0]) {
         vidstr(VER_ROW+3, VER_COL, WHITE, venus_rev);
      }
   }
   else if(rcvr_type == ZODIAC_RCVR) {
      if(zod_sw[0] || zod_date[0]) {
         sprintf(out, "SW:   %-6s %-8s  ", zod_sw, zod_date);
         vidstr(VER_ROW+2, VER_COL, WHITE, out);
      }

      if(zod_opt[0]) {
         sprintf(out, "HW:   %-17s", zod_opt);
         vidstr(VER_ROW+3, VER_COL, WHITE, out);
      }
   }
}

void show_serial_info()
{
   if(zoom_screen) return;
   if(text_mode && first_key) return;

   if(luxor) {
      if(usb_port == 9999)     sprintf(out, "Dev: %s", com_dev); 
      else if(usb_port == 999) sprintf(out, "Dev: %s", com_dev); 
      else if(usb_port)        sprintf(out, "USB: %d", usb_port-1);
#ifdef WINDOWS
      else if(com_port)        sprintf(out, "COM: %d", com_port);
#else   // __linux__  __MACH__
      else if(com_port)        sprintf(out, "TTY: %d", com_port-1);
#endif
#ifdef TCP_IP 
      else         sprintf(out," IP: %s", IP_addr); // TCP, no room for sernum
#else
      else         sprintf(out," COM: none");
#endif
      if(strlen(out) > 16) strcpy(&out[16-3],"...");   // don't hit 'Power' label
      vidstr(VER_ROW+5, VER_COL, WHITE, out);

      sprintf(out, "SER: %u.%04lu  ", sn_prefix, (unsigned long) serial_num);
   }
   else if(sn_prefix && serial_num) {
      if(usb_port == 9999)     sprintf(out, "Dev: %-17s", com_dev); // user specified device
      else if(usb_port == 999) sprintf(out, "Dev: %-17s", com_dev); // symlink to /dev/heather
      else if(usb_port >= 10)  sprintf(out, "Ser: %2u.%-8lu USB:%d",  sn_prefix, (unsigned long) serial_num, usb_port-1);
      else if(usb_port)        sprintf(out, "Ser: %2u.%-8lu  USB:%d", sn_prefix, (unsigned long) serial_num, usb_port-1);
#ifdef WINDOWS
      else if(com_port >= 10)  sprintf(out, "Ser: %2u.%-8lu COM:%d",  sn_prefix, (unsigned long) serial_num, com_port);
      else if(com_port)        sprintf(out, "Ser: %2u.%-8lu  COM:%d", sn_prefix, (unsigned long) serial_num, com_port);
#else   // __linux__  __MACH__
      else if(com_port >= 10)  sprintf(out, "Ser: %2u.%-8lu TTY:%d",  sn_prefix, (unsigned long) serial_num, com_port-1);
      else if(com_port)        sprintf(out, "Ser: %2u.%-8lu  TTY:%d", sn_prefix, (unsigned long) serial_num, com_port-1);
#endif
#ifdef TCP_IP 
      else                    sprintf(out," IP: %s", IP_addr); // TCP, no room for sernum
#else
      else                    sprintf(out, "Ser: %2u.%-8lu  COM:none", sn_prefix, (unsigned long) serial_num);
#endif
      if(strlen(out) > 27) strcpy(&out[27-3],"...");   // don't hit 'Power' label
   }
   else {
      if(usb_port == 9999)     sprintf(out, "DEV: %-17s", com_dev); // user specified device
      else if(usb_port == 999) sprintf(out, "DEV: %-17s", com_dev); // symlink to /dev/heather
      else if(usb_port >= 10)  sprintf(out, "USB: %-2d               ", usb_port-1);
      else if(usb_port)        sprintf(out, "USB: %-2d               ", usb_port-1);
#ifdef WINDOWS
      else if(com_port >= 10)  sprintf(out, "COM: %-2d               ", com_port);
      else if(com_port)        sprintf(out, "COM: %-2d               ", com_port);
#else   // __linux__  __MACH__
      else if(com_port >= 10)  sprintf(out, "TTY: %-2d               ", com_port-1);
      else if(com_port)        sprintf(out, "TTY: %-2d               ", com_port-1);
#endif
#ifdef TCP_IP 
      else                    sprintf(out," IP: %s", IP_addr); // TCP, no room for sernum
#else
      else                    sprintf(out, "COM:none              ");
#endif
      if(strlen(out) > 27) strcpy(&out[27-3],"...");   // don't hit 'Power' label
   }
   vidstr(VER_ROW+4, VER_COL, WHITE, out);
}


unsigned long last_pl;

void show_time_info()
{
int color;
u08 alarm_state;
long sec;
int row;
int zoomed;

   zoomed = zoom_screen;
   if(1 && (!SMALL_SCREEN) && ((zoom_screen == 'S') || (zoom_screen == 'L') || (zoom_screen == 'M'))) zoomed = 0;
   if(zoom_screen == 'I') zoomed = 0;

   if((text_mode && first_key) || zoomed || luxor) { // screen in use for help/edit info
      time_set_char = ' ';
      return;
   }

   if(alarm_time || alarm_date || egg_timer) alarm_state = ALARM_CHAR;
   else if(dump_time || dump_date || dump_timer || log_time || log_date || log_timer) {
      if(single_dump || single_log) alarm_state = '!';         //!!!!! need separate flags for screen and log dumps
      else                          alarm_state = DUMP_CHAR;
   }
   else if(cuckoo) {
      if(singing_clock) alarm_state = SONG_CHAR;
      else if(ships_clock) alarm_state = '#'; // SONG_CHAR;
      else alarm_state = CHIME_CHAR;
   }
   else alarm_state = ' ';

   if(time_set_char != ' ') {
      sprintf(out, "Time synced %c  ", alarm_state);
      vidstr(TIME_ROW+0, TIME_COL, GREEN, out);
   }
   else if(time_flags & 0x0004) {
      sprintf(out, "Time invalid %c ", alarm_state);
      vidstr(TIME_ROW+0, TIME_COL, RED, out);
      time_color = RED;
   }
   else if((time_flags & 0x0008) || !have_utc_ofs) {
      sprintf(out, "NO UTC offset %c", alarm_state);
      if(time_flags & 0x0001) vidstr(TIME_ROW+0, TIME_COL, RED, out);
      else                    vidstr(TIME_ROW+0, TIME_COL, YELLOW, out);
      time_color = YELLOW;
   }
   else if((time_flags & 0x0030) && saw_icm) {
      if((time_flags & 0x0030) == 0x0010) sprintf(out, "GLONASS TIME %c ", alarm_state);
      if((time_flags & 0x0030) == 0x0020) sprintf(out, "BEIDOU  TIME %c ", alarm_state);
      if((time_flags & 0x0030) == 0x0030) sprintf(out, "GALILEO TIME %c ", alarm_state);
   }
   else if(time_flags & 0x0010) {
      sprintf(out, "USER set time %c", alarm_state);
      vidstr(TIME_ROW+0, TIME_COL, YELLOW, out);
      time_color = YELLOW;
   }
   else if(time_flags & 0x0001) {
      sprintf(out, "UTC time OK %c  ", alarm_state);
      if(have_timing_mode) vidstr(TIME_ROW+0, TIME_COL, GREEN,  out);
      else                 vidstr(TIME_ROW+0, TIME_COL, GREY,   out);
      if(dst_ofs)            time_color = DST_TIME_COLOR;
      else if(time_zone_set) time_color = DST_TIME_COLOR;  // !!! std_time_color is rather dim for the big clock
      else                   time_color = WHITE;
   }
   else {
      sprintf(out, "GPS time OK %c  ", alarm_state);
      if(have_timing_mode) vidstr(TIME_ROW+0, TIME_COL, GREEN,  out);
      else                 vidstr(TIME_ROW+0, TIME_COL, GREY,   out);
      if(dst_ofs)            time_color = DST_TIME_COLOR;
      else if(time_zone_set) time_color = DST_TIME_COLOR;  // !!! std_time_color is rather dim for the big clock
      else                   time_color = WHITE;
   }

   if(time_color != last_time_color) {  // redraw big clock if time status has changed
      last_time_color = time_color;
   }

   if((nav_rate != 1.0) && (raw_frac != 0.0)) ;
   else if(seconds > 59) write_log_leapsecond();
   else if((minutes == 0) && (seconds == 0) && (last_second == 0)) write_log_leapsecond();
   else if((minutes == 59) && (seconds == 59) && (last_second == 59)) write_log_leapsecond();
   last_second = seconds;

   if((last_utc_offset != (-9999)) && (utc_offset != last_utc_offset)) {
      write_log_utc(utc_offset);
   }
   last_utc_offset = utc_offset;


   color = WHITE;
   if(sound_alarm && (ticker & 0x01)) color = RED;

   if(seconds_time || fraction_time) {
      sec  = (long) pri_hours * 3600L;
      sec += (long) pri_minutes * 60L;
      sec += (long) pri_seconds;
      if(fraction_time) {
         sec *= 1000L;
         sec /= 864L;
         sprintf(out, "%s: .%05ld", tz_info(), sec);
      }
      else {
         sprintf(out, "%s:  %05ld", tz_info(), sec);
      }
      if(dst_ofs)            vidstr(TIME_ROW+2,TIME_COL, DST_TIME_COLOR, out);
      else if(time_zone_set) vidstr(TIME_ROW+2,TIME_COL, STD_TIME_COLOR, out);
      else                   vidstr(TIME_ROW+2,TIME_COL, WHITE, out);

      strcpy(out, fmt_date(0));
      vidstr(TIME_ROW+1,TIME_COL, color, out);
   }
   else {
      sprintf(out, "%02d:%02d:%02d %s", pri_hours,pri_minutes,pri_seconds, tz_info());
      if(dst_ofs)            vidstr(TIME_ROW+1,TIME_COL, DST_TIME_COLOR, out);
      else if(time_zone_set) vidstr(TIME_ROW+1,TIME_COL, STD_TIME_COLOR, out);
      else                   vidstr(TIME_ROW+1,TIME_COL, WHITE, out);

      strcpy(out, fmt_date(0));
      if(rolled && (color == WHITE)) {
         vidstr(TIME_ROW+2,TIME_COL, YELLOW, out);
      }
      else {
         vidstr(TIME_ROW+2,TIME_COL, color, out);
      }
   }
   

   row = TIME_ROW+3;
   if(have_week) {
      if(0 && (rolled || user_set_rollover)) sprintf(out, "Week: %5uro", (unsigned) gps_week);
      else                            sprintf(out, "Week: %5u  ", (unsigned) gps_week);
//sprintf(out, "%5lu", pl_counter-last_pl);
//last_pl = pl_counter;
      if(rolled)         vidstr(row,TIME_COL, color, out);
      else if(rollover)  vidstr(row,TIME_COL, color, out);
      else               vidstr(row,TIME_COL, color, out);
      ++row;
   }

   if(have_tow) {
      sprintf(out, "TOW:%7lu  ", (unsigned long) tow);
      vidstr(row,TIME_COL, color, out);
      ++row;
   }

   if(have_utc_ofs) {
      sprintf(out, "UTC ofs:%3d  ", utc_offset);
      if(color == RED) ;
      else if(user_set_utc_ofs) color = YELLOW;   // using user value
      else if(have_utc_ofs < 0) color = RED;      // using estimated value
      vidstr(row,TIME_COL, color, out);           // using receiver value
      ++row;
   }

   time_set_char = ' ';
   return;
}

void show_survey_info()
{
int color;

   if(luxor) return;

   if(minor_alarms & 0x0020) {   // self survey is active
      vidstr(SURVEY_ROW+0+eofs, SURVEY_COL, RED,    "Self Survey:  ");
      color = YELLOW;
   }
   else if(precision_survey) { 
      vidstr(SURVEY_ROW+0+eofs, SURVEY_COL, RED,    "Median Survey:");
      color = YELLOW;
   }
   else if(rcvr_type == GPSD_RCVR) {
   }
   else if(rcvr_type == NMEA_RCVR) {
   }
   else if(rcvr_type == SIRF_RCVR) {
   }
   else {  // not in survey mode,  grey out this info on the screen
      if(TIMING_RCVR || (rcvr_type == SCPI_RCVR) || (rcvr_type == UCCM_RCVR)) {  // show survey info
         color = GREY;
         vidstr(SURVEY_ROW+0+eofs, SURVEY_COL, color,  "Survey data:  ");
      }
      else {   // show osc info
        if(have_tc) color = WHITE;
        else        color = GREY; 
        sprintf(out, "TC:   %.1f sec", time_constant);
        vidstr(SURVEY_ROW+0+eofs, SURVEY_COL, color, out);

        if(have_damp) color = WHITE;
        else          color = GREY; 
        sprintf(out, "DAMP: %.3f", damping_factor);
        vidstr(SURVEY_ROW+1+eofs, SURVEY_COL, color, out);

        if(gain_color == YELLOW) color = gain_color;
        else if(have_gain) color = WHITE;
        else color = GREY; 
        if(osc_gain < 0.0F)       sprintf(out, "GAIN:%.3f Hz/V ", osc_gain);
        else if(osc_gain > 10.0F) sprintf(out, "GAIN:%.3f Hz/V ", osc_gain);
        else                      sprintf(out, "GAIN: %.3f Hz/V ", osc_gain);
        vidstr(SURVEY_ROW+2+eofs, SURVEY_COL, color, out);

        if(have_initv) color = WHITE;
        else           color = GREY; 
        sprintf(out, "INIT: %.3f V", initial_voltage);
        if(saw_ntpx) vidstr(SURVEY_ROW+3+eofs, SURVEY_COL, GREY,  out);
        else         vidstr(SURVEY_ROW+3+eofs, SURVEY_COL, color, out);
        return;
      }
   }

   if((survey_length >= 0) && (survey_length <= 100000L)) {
      if(precision_survey)            sprintf(out, "Samples:%5lu", (unsigned long) precision_samples);  
      else if(rcvr_type == TSIP_RCVR) sprintf(out, "Samples:%5d", survey_length);
      else if(rcvr_type == ZODIAC_RCVR) {
         if(survey_length < 10) sprintf(out, "%d hour%s      ", survey_length, (survey_length==1)?" ":"s");
         else                   sprintf(out, "%d hour%s     ", survey_length, (survey_length==1)?" ":"s");
      }
      else if(minor_alarms & 0x0020)    sprintf(out, "In progress  ");
      else                              sprintf(out, "             ");
      vidstr(SURVEY_ROW+1+eofs, SURVEY_COL, color, out);
   }

   if((survey_progress >= 0) && (survey_progress <= 100)) {
      if(precision_survey) {
         if(PRECISE_SURVEY_HOURS) {
            sprintf(out, "Progress:%3ld%%", (survey_minutes*99L)/((long)PRECISE_SURVEY_HOURS*60L)+1L);
         }
         else sprintf(out, "             ");
         vidstr(SURVEY_ROW+2+eofs, SURVEY_COL, color, out);
      }
      else {
         if     (rcvr_type == SCPI_RCVR)  sprintf(out, "Progress:%3d%%", survey_progress);
         else if(rcvr_type == UCCM_RCVR)  sprintf(out, "Progress:%3d%%", survey_progress);
         else if(rcvr_type == TSIP_RCVR)  sprintf(out, "Progress:%3d%%", survey_progress);
         else if(rcvr_type == UBX_RCVR)   sprintf(out, "Progress:%3d%%", survey_progress);
         else if(rcvr_type == VENUS_RCVR) sprintf(out, "Progress:%3d%%", survey_progress);
         else sprintf(out, "             ");
         vidstr(SURVEY_ROW+2+eofs, SURVEY_COL, color, out);

         if(rcvr_type == UBX_RCVR)  {
            sprintf(out, "VAR: %-9d", ubx_svar);
            vidstr(SURVEY_ROW+3+eofs, SURVEY_COL, color, out);
         }
      }
   }

   if(precision_survey || check_precise_posn || survey_done) {
   }
   else if(rcvr_type == TSIP_RCVR) {
      if(survey_save) vidstr(SURVEY_ROW+3+eofs, SURVEY_COL, color, "Save position");
      else            vidstr(SURVEY_ROW+3+eofs, SURVEY_COL, color, "Dont save pos");
   }
}

void show_minor_alarms()
{
int bad_mask;
char s[80];
int row;
int i;
double jd;

   if(text_mode && first_key) return;
   if(zoom_screen) return;
   if(luxor) return;
   if(rcvr_type == NO_RCVR) return;

   row = MINOR_ROW;
if(!have_critical_alarms) row = CRIT_ROW;

   for(i=row; i<=MINOR_ROW+10; i++) {  // clear out old info
      vidstr(i,MINOR_COL, WHITE, "             ");
   }

   // minor alarm info
   if(!have_eeprom) ; 
   else if(!have_critical_alarms) ;
   else if(rcvr_type == NVS_RCVR) ;
   else if(rcvr_type == STAR_RCVR) ;
   else if(rcvr_type == VENUS_RCVR) ;
   else {
      if(minor_alarms & 0x0400)  vidstr(row, MINOR_COL, RED,        "EEPROM BAD   ");
      else                       vidstr(row, MINOR_COL, GREEN,      "EEPROM  OK   ");
      ++row;
   }

   if(!have_antenna) vidstr(row, MINOR_COL, GREY, "Antenna OK    ");
   else if(rcvr_type == STAR_RCVR) {
      if(minor_alarms & 0x0006) vidstr(row, MINOR_COL, RED,  "Antenna alarm "); 
      else                      vidstr(row, MINOR_COL, GREEN,"Antenna OK    "); 
   }
   else if((minor_alarms & 0x0002) && (minor_alarms & 0x0004)) {
      vidstr(row, MINOR_COL, YELLOW,"Antenna no pwr"); 
   }
   else if(minor_alarms & 0x0002)  vidstr(row, MINOR_COL, YELLOW,"Antenna open  ");
   else if(minor_alarms & 0x0004)  vidstr(row, MINOR_COL, RED,   "Antenna short ");
   else                            vidstr(row, MINOR_COL, GREEN, "Antenna OK    ");
   ++row;

   if(have_almanac) {
      if(minor_alarms & 0x0800)  vidstr(row, MINOR_COL, YELLOW,     "No almanac   ");
                                 vidstr(row, MINOR_COL, GREEN,      "Almanac OK   ");
      ++row;
   }

   if(minor_alarms & 0x0010)  {
      if(TIMING_RCVR)         vidstr(row, MINOR_COL, RED,           "Unk alarm 0x10");
      else if(osc_control_on) vidstr(row, MINOR_COL, OSC_PID_COLOR, "OSC PID CTRL  ");
      else                    vidstr(row, MINOR_COL, RED,           "Undisciplined ");
      ++row;
   }
   else if(rcvr_type == MOTO_RCVR) {
      if(have_traim) {
         sprintf(out, "TRAIM:%5dns  ", traim_threshold);
         if(traim_mode == 0)        vidstr(row, MINOR_COL, GREEN,     "TRAIM   OFF    ");  
         else if(traim_status == 0) vidstr(row, MINOR_COL, GREEN,     out); 
         else if(traim_status == 1) vidstr(row, MINOR_COL, RED,       "TRAIM   ALARM  "); 
         else  {
            sprintf(out, "TRAIM   Unk:%-3d", traim_status);
            vidstr(row, MINOR_COL, YELLOW, out);
         }
         ++row;
      }
   }
   else if(rcvr_type == NVS_RCVR) {
      if(have_traim) {
         if     (traim_mode == 0) vidstr(row, MINOR_COL, GREEN,     "RAIM    OFF    ");  
         else if(traim_mode == 1) vidstr(row, MINOR_COL, GREEN,     "RAIM    ON     ");  
         else if(traim_mode == 2) vidstr(row, MINOR_COL, GREEN,     "RAIM    FDE    ");  
         else if(traim_mode == 3) vidstr(row, MINOR_COL, GREEN,     "RAIM    ON+FDE ");  
         ++row;
      }
   }
   else if(rcvr_type == ZODIAC_RCVR) {
      if(have_traim) {
         sprintf(out, "TRAIM:%5dns  ", traim_threshold);
         if     (traim_mode == 0)   vidstr(row, MINOR_COL, GREEN,     "TRAIM   OFF    ");  
         else if(traim_status == 0) vidstr(row, MINOR_COL, GREEN, out);
         else if(traim_status == 1) vidstr(row, MINOR_COL, GREEN,     "TRAIM   DETECT ");
         else if(traim_status == 2) vidstr(row, MINOR_COL, GREEN,     "TRAIM   ISOLATE");
         else  {
            sprintf(out, "TRAIM   Unk:%-3d", traim_status);
            vidstr(row, MINOR_COL, YELLOW, out);
         }
         ++row;
      }
   }
   else if(rcvr_type == STAR_RCVR) ;
   else if(rcvr_type == UCCM_RCVR) ;
   else if(GPSDO) {
      vidstr(row, MINOR_COL, GREEN,      "Discipline OK ");
      ++row;
   }

   if(have_saved_posn) {
      if(minor_alarms & 0x0200) vidstr(row, MINOR_COL, RED,        "Saved posn BAD");
      else                      vidstr(row, MINOR_COL, GREEN,      "Saved posn OK ");
      ++row;
   }

   if(have_tracking) {
      if(minor_alarms & 0x0008) vidstr(row, MINOR_COL, RED,        "No sats usable");
      else                      vidstr(row, MINOR_COL, GREEN,      "Tracking sats ");
      ++row;
   }

   if(have_saved_posn) {
      if(minor_alarms & 0x0040) vidstr(row, MINOR_COL, RED,        "No saved posn ");
      else                      vidstr(row, MINOR_COL, GREEN,      "Position saved");
      ++row;
   }

if(rcvr_type == ZODIAC_RCVR) return;
         
   if(have_leap_info) {
      guessed_leap_days = ' ';
      if(!have_leap_days) guess_leap_days();

      if(leap_days >= LEAP_THRESH) sprintf(s, "LEAP: Pending!");
      else if(leap_days >= 100) {
         if(guessed_leap_days == ' ') sprintf(s, "LEAP: %3u days", leap_days);
         else                         sprintf(s, "LEAP:%3u days%c", leap_days, guessed_leap_days);
      }
      else if(leap_days >= 10)  sprintf(s, "LEAP: %2u days%c", leap_days, guessed_leap_days);
      else if(leap_days >= 4)   sprintf(s, "LEAP: %1u days%c ", leap_days, guessed_leap_days);
      else if(leap_days >= 0) {
         jd = (jd_leap + jtime(0,0,0,1.0) - jd_utc) * 24.0;
         if(jd > 100) {  // should never happen (except STAR_RCVR with loss of signal)
            sprintf(s, "LEAP: PENDING!");
         }
         else if(have_jd_leap && (jd >= 0.0)) {
            if(jd >= 1.0) sprintf(s, "LEAP:%3d hours", (int) (jd+0.99));
            else {
               jd *= 60.0;
               if(jd >= 1.0) sprintf(s, "LEAP: %2d mins!", (int) (jd+0.99));
               else {
                  jd *= 60.0;
                  sprintf(s, "LEAP: %2d secs!", (int) (jd+0.99));
               }
            }
         }
         else {
            sprintf(s, "LEAP: TODAY!  ");
         }
      }
      else sprintf(s, "LEAP: PENDING!");

      if(minor_alarms & 0x0080)  vidstr(row, MINOR_COL, YELLOW,     s);
      else                       vidstr(row, MINOR_COL, GREEN,      "No leap pend  ");
      ++row;
   }

if(rcvr_type == UBX_RCVR) return;

   if(have_op_mode) {
      if(minor_alarms & 0x0100) vidstr(row, MINOR_COL, RED,        "Cal mode set  ");
      else                      vidstr(row, MINOR_COL, GREEN,      "Normal op mode");
      ++row;
   }


   if(have_osc_age) {
      if(res_t) {
         if(minor_alarms & 0x0001)  vidstr(row, MINOR_COL, RED,     "Unk alarm 0x01");
         else                       vidstr(row, MINOR_COL, GREY,    "              ");
      }
      else if((rcvr_type == NMEA_RCVR) && (nmea_type == GARMIN_NMEA)) {  // Garmin
         if(minor_alarms & 0x0001)  vidstr(row, MINOR_COL, RED,     "OSC drifting  ");
         else                       vidstr(row, MINOR_COL, GREEN,   "OSC stable    ");
      }
      else {
         if(minor_alarms & 0x0001)  vidstr(row, MINOR_COL, RED,     "OSC age alarm ");
         else                       vidstr(row, MINOR_COL, GREEN,   "Normal OSC age");
      }
      ++row;
   }

   if(1 || (SCREEN_HEIGHT >= MEDIUM_HEIGHT) || (small_font && (SCREEN_HEIGHT > 600))) { // windows uses small font,  so this fits in 800x600 mode
      if(res_t) {
         if(pps_rate & 0x80) {
            if(minor_alarms & 0x1000)  vidstr(row, MINOR_COL, GREEN,      "PP2S: skipped  ");
            else                       vidstr(row, MINOR_COL, GREEN,      "PP2S: generated");
         }
         else {
            if(minor_alarms & 0x1000)  vidstr(row, MINOR_COL, YELLOW,     "PPS: skipped   ");
            else                       vidstr(row, MINOR_COL, GREEN,      "PPS: generated ");
         }
         bad_mask = 0xE000;
      }
      else if(rcvr_type == ZODIAC_RCVR) bad_mask = 0xE000;
      else bad_mask = 0xF000;

      if(minor_alarms & bad_mask)  {
         sprintf(out, "Alarm?: %04X  ",  minor_alarms&0xF000);
         vidstr(row, MINOR_COL, RED,   out);
      }
      else {
///!!!!  vidstr(row, MINOR_COL, WHITE,   "              ");
      }
      ++row;
   }
}

void show_operation_info(int why)
{
int color;
int i;

   if(text_mode && first_key) return;
   if(zoom_screen) return;
   if(luxor) return;
// if(rcvr_type == ACRON_RCVR) return;
   if(rcvr_type == NO_RCVR) return;

   // receiver mode and position
   i = 0;
   if((rcvr_type == SIRF_RCVR) && (!saw_timing_msg)) color = GREEN;
   else if((rcvr_type == UBX_RCVR) && (!saw_timing_msg)) color = GREEN;
   else if((rcvr_type == VENUS_RCVR) && (!saw_timing_msg)) color = GREEN;
   else if(GPSDO || TIMING_RCVR) color = YELLOW;
   else color = GREEN;

   if(rcvr_type == ZODIAC_RCVR) {
      if(single_sat_prn)       vidstr(POSN_ROW+0+eofs,POSN_COL, YELLOW, "Single sat mode          ");
      else if(rcvr_mode == 4)  vidstr(POSN_ROW+0+eofs,POSN_COL, GREEN,  "Navigation mode          ");
      else if(rcvr_mode == 7)  vidstr(POSN_ROW+0+eofs,POSN_COL, GREEN,  "Position hold mode       ");
      else if(rcvr_mode == 11) vidstr(POSN_ROW+0+eofs,POSN_COL, YELLOW, "Survey mode              ");
      else {
         sprintf(out, "Receiver mode ?%02X?  ", rcvr_mode);
         vidstr(POSN_ROW+0+eofs,POSN_COL, RED, out);
      }
   }
   else if(single_sat_prn && (rcvr_type != TSIP_RCVR)) vidstr(POSN_ROW+0+eofs,POSN_COL, YELLOW, "Single sat mode          ");
   else if(rcvr_mode == 0)  vidstr(POSN_ROW+0+eofs,POSN_COL, YELLOW, "2D/3D positioning        ");
   else if(rcvr_mode == 1)  vidstr(POSN_ROW+0+eofs,POSN_COL, YELLOW, "Single satellite         ");
   else if(rcvr_mode == 2)  vidstr(POSN_ROW+0+eofs,POSN_COL, YELLOW, "No sats usable           ");
   else if(rcvr_mode == 3)  vidstr(POSN_ROW+0+eofs,POSN_COL, YELLOW, "2D positioning           ");
   else if(rcvr_mode == 4)  vidstr(POSN_ROW+0+eofs,POSN_COL, color,  "3D positioning           ");
   else if(rcvr_mode == 5)  vidstr(POSN_ROW+0+eofs,POSN_COL, YELLOW, "DGPS reference           ");
   else if(rcvr_mode == 6)  vidstr(POSN_ROW+0+eofs,POSN_COL, YELLOW, "2D clock hold            ");
   else if(rcvr_mode == 7)  {
      if     (rcvr_type == ACRON_RCVR)  vidstr(POSN_ROW+0+eofs,POSN_COL, GREEN,  "Position hold mode  ");
      else if(rcvr_type == MOTO_RCVR)   vidstr(POSN_ROW+0+eofs,POSN_COL, GREEN,  "Position hold mode  ");
      else if(rcvr_type == NVS_RCVR)    vidstr(POSN_ROW+0+eofs,POSN_COL, GREEN,  "Position hold mode  ");
      else if(rcvr_type == SCPI_RCVR)   vidstr(POSN_ROW+0+eofs,POSN_COL, GREEN,  "Position hold mode  ");
      else if(rcvr_type == STAR_RCVR)   vidstr(POSN_ROW+0+eofs,POSN_COL, GREEN,  "Position hold mode  ");
      else if(rcvr_type == UCCM_RCVR)   vidstr(POSN_ROW+0+eofs,POSN_COL, GREEN,  "Position hold mode  ");
      else if(rcvr_type == UBX_RCVR)    vidstr(POSN_ROW+0+eofs,POSN_COL, GREEN,  "Position hold mode  ");
      else if(rcvr_type == VENUS_RCVR)  vidstr(POSN_ROW+0+eofs,POSN_COL, GREEN,  "Position hold mode  ");
      else if(rcvr_type == ZODIAC_RCVR) vidstr(POSN_ROW+0+eofs,POSN_COL, GREEN,  "Position hold mode  ");
      else                              vidstr(POSN_ROW+0+eofs,POSN_COL, GREEN,  "Overdetermined clock");
   }
   else if(rcvr_mode == RCVR_MODE_PROP)  {
      if(rcvr_type == SCPI_RCVR) {
         vidstr(POSN_ROW+0+eofs,POSN_COL, YELLOW, "Holdover mode       ");
      }
      else if(rcvr_type == UCCM_RCVR) { 
         vidstr(POSN_ROW+0+eofs,POSN_COL, YELLOW, "Holdover mode       ");
      }
      else if(rcvr_type == STAR_RCVR) { 
         vidstr(POSN_ROW+0+eofs,POSN_COL, YELLOW, "Holdover mode       ");
      }
      else {
         vidstr(POSN_ROW+0+eofs,POSN_COL, YELLOW, "Propogate mode      ");
      }
   }
   else if(rcvr_mode == RCVR_MODE_ACQUIRE)  vidstr(POSN_ROW+0+eofs,POSN_COL, YELLOW, "Acquiring sats      ");
   else if(rcvr_mode == RCVR_MODE_BAD_GEOM) vidstr(POSN_ROW+0+eofs,POSN_COL, RED,    "Bad sat geometry    ");
   else if(rcvr_mode == RCVR_MODE_SURVEY)   vidstr(POSN_ROW+0+eofs,POSN_COL, YELLOW, "Survey mode         ");
   else if(rcvr_mode == RCVR_MODE_UNKNOWN) {
      sprintf(out, "Unknown mode: %02X    ", rcvr_mode);
      vidstr(POSN_ROW+0+eofs,POSN_COL, YELLOW, out);
   }
   else {
      sprintf(out, "Receiver mode ?%02X?  ", rcvr_mode);
      vidstr(POSN_ROW+0+eofs,POSN_COL, RED, out);
   }

   if(rcvr_mode == RCVR_MODE_HOLD) fix_mode = 0;
   else                            fix_mode = 1;

   show_lla(why);        // lat/lon/alt info
   show_survey_info();   // self survey info
if(rcvr_type == NMEA_RCVR) return;
if(rcvr_type == ACRON_RCVR) return;
if((rcvr_type == GPSD_RCVR) && (!saw_gpsdo)) return; // gggg

   // Oscillator disciplining info
   if(sats_enabled != 0xFFFFFFFF)             vidstr(DIS_ROW+0+eofs, DIS_COL, YELLOW, "Sats excluded   ");
   else if(GPSDO && (rcvr_type != UCCM_RCVR)) vidstr(DIS_ROW+0+eofs, DIS_COL, WHITE,  "Discipline mode:");
   else                                       vidstr(DIS_ROW+0+eofs, DIS_COL, WHITE,  "Operation mode: ");

   if(discipline_mode == 0) {
      if((rcvr_type == UCCM_RCVR) && uccm_led_msg[0]) { //kkkkk
         if     (strstr(uccm_led_msg, "Normal")) color = GREEN;
         else if(strstr(uccm_led_msg, "LOCKED")) color = GREEN;
         else                                    color = YELLOW;
         vidstr(DIS_ROW+1+eofs, DIS_COL, color,  uccm_led_msg);
      }
      else vidstr(DIS_ROW+1+eofs, DIS_COL, GREEN,  "Normal          ");
   }
   else if(discipline_mode == 1) vidstr(DIS_ROW+1+eofs, DIS_COL, YELLOW, "Power-up        ");
   else if(discipline_mode == 2) vidstr(DIS_ROW+1+eofs, DIS_COL, RED,    "Auto holdover   ");  // this could be a YELLOW alert
   else if(discipline_mode == 3) {
      if     (rcvr_type == SCPI_RCVR) vidstr(DIS_ROW+1+eofs, DIS_COL, YELLOW, "Holdover mode   ");
      else if(rcvr_type == UCCM_RCVR) {
         if(scpi_type == UCCMP_TYPE)  vidstr(DIS_ROW+1+eofs, DIS_COL, YELLOW, "Holdover mode   ");  // kkkkkk uuuuu
         else                         vidstr(DIS_ROW+1+eofs, DIS_COL, YELLOW, "Manual EFC mode ");  // kkkkkk uuuuu
      }
      else if(rcvr_type == GPSD_RCVR) vidstr(DIS_ROW+1+eofs, DIS_COL, YELLOW, "Discipline off  ");
      else                            vidstr(DIS_ROW+1+eofs, DIS_COL, YELLOW, "Manual holdover ");
   }
   else if(discipline_mode == 4) {
      if(rcvr_type == GPSD_RCVR)      vidstr(DIS_ROW+1+eofs, DIS_COL, YELLOW, "No PPS reference");
      else if(rcvr_type == STAR_RCVR) vidstr(DIS_ROW+1+eofs, DIS_COL, YELLOW, "Output squelched");
      else                            vidstr(DIS_ROW+1+eofs, DIS_COL, YELLOW, "Recovery        ");
   }
   else if(discipline_mode == 5) vidstr(DIS_ROW+1+eofs, DIS_COL, YELLOW, "Fast recovery   ");
   else if(discipline_mode == 6) {
      if(osc_control_on) vidstr(DIS_ROW+1+eofs, DIS_COL, OSC_PID_COLOR,  "OSC PID ENABLED ");
      else               vidstr(DIS_ROW+1+eofs, DIS_COL, RED,            "Disabled        ");
   }
   else if(discipline_mode == 10) {
      if(saw_uccm_dmode == 2) vidstr(DIS_ROW+1+eofs, DIS_COL, YELLOW, "Warming up      ");
      else if(rcvr_type == STAR_RCVR) vidstr(DIS_ROW+1+eofs, DIS_COL, YELLOW, "Warming up      ");
      else vidstr(DIS_ROW+1+eofs, DIS_COL, YELLOW, "Settling        ");
   }
   else {
      if(res_t) sprintf(out, "Operation ?%02X?  ", discipline_mode);
      else      sprintf(out, "Discipline ?%02X? ", discipline_mode);
      vidstr(DIS_ROW+1+eofs, DIS_COL, YELLOW, out);
   }

if(rcvr_type == GPSD_RCVR) return;

   color = GREEN;
   if((rcvr_type == UCCM_RCVR) && (scpi_type == UCCMP_TYPE)) {  //kkkkkk
      if(holdover >= 1000000) {
         sprintf(out, "Holdover:%lu secs            ", (unsigned long) holdover);
      }
      else {
         sprintf(out, "Holdover: %lu secs            ", (unsigned long) holdover);
      }
   }
   else if(have_star_atdc) {
      if(star_atdc_on) {
        if(star_atdc_time) {
           sprintf(out, "ATDC: in %d secs              ", star_atdc_time);
           color = YELLOW;
        }
        else {
           sprintf(out, "ATDC: NOT READY               ");
           color = RED;
        }
      }
      else sprintf(out, "ATDC: ACTIVE                  ");
   }
   else if(GPSDO && (rcvr_type != STAR_RCVR) && (rcvr_type != UCCM_RCVR)) {
      if(holdover >= 1000000) {
         sprintf(out, "Holdover:%lu secs            ", (unsigned long) holdover);
      }
      else {
         sprintf(out, "Holdover: %lu secs            ", (unsigned long) holdover);
      }
   }
   else strcpy(out, blanks);

   out[20] = 0;
   if((discipline_mode == 2) || (discipline_mode == 3)) {
      color = RED;
      holdover_seen = 1;
   }
   else if(holdover_seen) color = YELLOW;

   vidstr(DIS_ROW+2+eofs, DIS_COL, color, out);

   if(have_star_perf) {
      if(star_hold_perf) vidstr(DIS_ROW+3+eofs, DIS_COL, GREEN,  "Holdover perf GOOD ");
      else               vidstr(DIS_ROW+3+eofs, DIS_COL, YELLOW, "Holdover perf POOR ");
   }
   else if(!have_gps_status) {
      if(have_lifetime) {
         sprintf(out, "Life:%7d hrs", scpi_life);
         vidstr(DIS_ROW+3+eofs, DIS_COL, GREEN, out); 
      }
      else vidstr(DIS_ROW+3+eofs, DIS_COL, GREY,   "No status avail "); 
   }
   else if(gps_status == 0x00) vidstr(DIS_ROW+3+eofs, DIS_COL, GREEN,  "Doing fixes     ");
   else if(gps_status == 0x01) vidstr(DIS_ROW+3+eofs, DIS_COL, RED,    "No GPS time     ");
   else if(gps_status == 0x03) vidstr(DIS_ROW+3+eofs, DIS_COL, RED,    "PDOP too high   ");
   else if(gps_status == 0x08) vidstr(DIS_ROW+3+eofs, DIS_COL, RED,    "No usable sats  ");
   else if(gps_status == 0x09) vidstr(DIS_ROW+3+eofs, DIS_COL, RED,    "1 usable sat    ");
   else if(gps_status == 0x0A) vidstr(DIS_ROW+3+eofs, DIS_COL, RED,    "2 usable sats   ");
   else if(gps_status == 0x0B) vidstr(DIS_ROW+3+eofs, DIS_COL, RED,    "3 usable sats   ");
   else if(gps_status == 0x0C) vidstr(DIS_ROW+3+eofs, DIS_COL, RED,    "sat unusable    ");
   else if(gps_status == 0x10) vidstr(DIS_ROW+3+eofs, DIS_COL, RED,    "TRAIM rejected  ");
   else {
      sprintf(out, "GPS status:?%02X? ", gps_status);
      vidstr(DIS_ROW+3+eofs, DIS_COL, YELLOW, out);
   }

}

void show_critical_alarms()
{
int row;

   if(have_scpi_test) {
      if(scpi_test) {
         sprintf(out, "FAIL:%5d", scpi_test);
         vidstr(CRIT_ROW+3, CRIT_COL, RED, out); 
      }
      else vidstr(CRIT_ROW+3, CRIT_COL, GREEN, "TEST:   OK"); 
      return;
   }

   if(!have_critical_alarms) return;

   for(row=CRIT_ROW; row<=CRIT_ROW+4; row++) {
      vidstr(row,CRIT_COL, WHITE, "          ");
   }


   if(rcvr_type == NVS_RCVR) {
      vidstr(CRIT_ROW+2, CRIT_COL, GREEN, "GPS:    OK");
      vidstr(CRIT_ROW+3, CRIT_COL, GREEN, "GLO:    OK");
      if(critical_alarms & 0x0008)  {
         if(nvs_test & 0x10) vidstr(CRIT_ROW+2, CRIT_COL, RED,   "GPS:   BAD");
         if(nvs_test & 0x20) vidstr(CRIT_ROW+3, CRIT_COL, RED,   "GLO:   BAD");
      }
      return;
   }
   else if(rcvr_type == STAR_RCVR) {
      if(critical_alarms & 0x0008)  {
         vidstr(CRIT_ROW+2, CRIT_COL, RED,   "GPS:   BAD");
      }
      else {
         vidstr(CRIT_ROW+2, CRIT_COL, GREEN, "GPS:    OK");
      }

      if(critical_alarms & 0x0010)  vidstr(CRIT_ROW+3, CRIT_COL, RED,   "OSC:   BAD");
      else                          vidstr(CRIT_ROW+3, CRIT_COL, GREEN, "OSC:    OK");
      return;
   }
   else if(rcvr_type == VENUS_RCVR) {
      if(critical_alarms & 0x0001)  vidstr(CRIT_ROW+0, CRIT_COL, RED,   "FLASH: BAD");
      else if(no_eeprom_writes || (eeprom_save == 0))     vidstr(CRIT_ROW+0, CRIT_COL, GREEN, "FLASH:  OK");
      else vidstr(CRIT_ROW+0, CRIT_COL, GREEN, "FLASH: WRT");
      return;
   }
   else {
      if(critical_alarms & 0x0001)  vidstr(CRIT_ROW+0, CRIT_COL, RED,   "ROM:   BAD");
      else                          vidstr(CRIT_ROW+0, CRIT_COL, GREEN, "ROM:    OK");

      if(critical_alarms & 0x0002)  vidstr(CRIT_ROW+1, CRIT_COL, RED,   "RAM:   BAD");
      else                          vidstr(CRIT_ROW+1, CRIT_COL, GREEN, "RAM:    OK");
   }


   if(rcvr_type == ZODIAC_RCVR) {
      if(critical_alarms & 0x0010)  vidstr(CRIT_ROW+2, CRIT_COL, RED,   "RTC:   BAD");
      else                          vidstr(CRIT_ROW+2, CRIT_COL, GREEN, "RTC:    OK");
   }
   else if((rcvr_type == NMEA_RCVR) && (nmea_type == GARMIN_NMEA)) {  // Garmin
      if(critical_alarms & 0x0010)  vidstr(CRIT_ROW+2, CRIT_COL, RED,   "RTC:   BAD");
      else                          vidstr(CRIT_ROW+2, CRIT_COL, GREEN, "RTC:    OK");
   }
   else {
      if(critical_alarms & 0x0010)  vidstr(CRIT_ROW+2, CRIT_COL, RED,   "OSC:   BAD");
      else                          vidstr(CRIT_ROW+2, CRIT_COL, GREEN, "OSC:    OK");
   }

   if((rcvr_type == SCPI_RCVR) && (scpi_type != NORTEL_TYPE)) {
      if(critical_alarms & 0x0008)  vidstr(CRIT_ROW+3, CRIT_COL, RED,   "GPS:   BAD");
      else                          vidstr(CRIT_ROW+3, CRIT_COL, GREEN, "GPS:    OK");
   }
   else if(rcvr_type == UCCM_RCVR) { 
      if(critical_alarms & 0x0008)  vidstr(CRIT_ROW+3, CRIT_COL, RED,   "GPS:   BAD");
      else                          vidstr(CRIT_ROW+3, CRIT_COL, GREEN, "GPS:    OK");
   }
   else if(rcvr_type == ZODIAC_RCVR) {
      if(critical_alarms & 0x0008)  vidstr(CRIT_ROW+3, CRIT_COL, RED,   "DSP:   BAD");
      else                          vidstr(CRIT_ROW+3, CRIT_COL, GREEN, "DSP:    OK");
   }
   else {
      if(critical_alarms & 0x0008)  vidstr(CRIT_ROW+3, CRIT_COL, RED,   "FPGA:  BAD");
      else                          vidstr(CRIT_ROW+3, CRIT_COL, GREEN, "FPGA:   OK");
   }

   if(critical_alarms & 0x0004)     vidstr(CRIT_ROW+4, CRIT_COL, RED,   "Power: BAD");
   else                             vidstr(CRIT_ROW+4, CRIT_COL, GREEN, "Power:  OK");
}


void show_rcvr_rate()
{
   if(have_nav_rate && (nav_rate != 1)) {
      if(nav_rate < 10.0) sprintf(out, "Rcvr rate:%3.1f Hz", nav_rate);
      else                sprintf(out, "Rcvr rate:%3d Hz", (int) nav_rate);
      vidstr(VAL_ROW+0, VAL_COL, GREEN,  out);
   }
   else {
      vidstr(VAL_ROW+0, VAL_COL, GREEN,  "Receiver data   ");
   }
}


void show_param_values()
{
u08 c;
float val;

   if(text_mode && first_key) return;
   if(zoom_screen == 'I') {
      show_lla(543);
   }
   if(zoom_screen) return;
   if(luxor) return;

   if((all_adevs == 0) || ((TEXT_HEIGHT <= 12) && (SCREEN_WIDTH >= 1280))) {
      show_operation_info(1);
      show_minor_alarms();
      show_filter_status();
   }
   else if(all_adevs) {
      if(SCREEN_WIDTH >= 1280) {
         show_minor_alarms();
         show_filter_status();
      }
      if(all_adev_row >= (ALL_ROW+5)) show_operation_info(2);
   }

   show_cable_delay();

   show_version_info();

   show_critical_alarms();

   if(rcvr_type == NO_RCVR) {
      vidstr(VER_ROW+1, VER_COL, WHITE,  "Dev:  System clock     "); 
      return;
   }
   else if(rcvr_type == ACRON_RCVR) {
      vidstr(VER_ROW+1, VER_COL, WHITE,  "Dev:  Acron Zeit       "); 
      return;
   }
   else if(unit_name[0]) {
      sprintf(out, "Dev:  %s", unit_name);
      vidstr(VER_ROW+1, VER_COL, WHITE,  out);  
      show_rcvr_rate();
   }
   else if(res_t) {
      vidstr(VER_ROW+1, VER_COL, WHITE,  "Dev:  Resolution-T     "); 
      show_rcvr_rate();
   }
   else if(saw_ntpx) {
      vidstr(VER_ROW+1, VER_COL, WHITE,  "Dev:  Nortel NTPX      "); 
      show_rcvr_rate();
   }
   else if(saw_nortel) {
      vidstr(VER_ROW+1, VER_COL, WHITE,  "Dev:  Nortel NTGS/NTBW "); 
      show_rcvr_rate();
   }
   else if(rcvr_type == GPSD_RCVR) {  
      if(gpsd_release[0]) {
         sprintf(out, "Rel:  GPSD %-12s", gpsd_release);
         vidstr(VER_ROW+1, VER_COL, WHITE,  out);
      }
      else if(gpsd_major || gpsd_minor) {
         sprintf(out, "Dev:  GPSD prot %d.%02d   ", gpsd_major,gpsd_minor);
         vidstr(VER_ROW+1, VER_COL, WHITE,  out);
      }
      else {
         vidstr(VER_ROW+1, VER_COL, WHITE,  "Dev:  GPSD interface   ");
      }
      show_rcvr_rate();

      if(gpsd_driver[0]) vidstr(VER_ROW+2, VER_COL, WHITE, gpsd_driver);
   }
   else if(rcvr_type == MOTO_RCVR) {
      sprintf(out, "Dev:  Motorola %2d chan ", moto_chans);
      vidstr(VER_ROW+1, VER_COL, WHITE,  out);
      show_rcvr_rate();
   }
   else if(rcvr_type == NMEA_RCVR) {
      vidstr(VER_ROW+1, VER_COL, WHITE,  "Dev:  NMEA GPS Receiver");
      show_rcvr_rate();
   }
   else if(rcvr_type == NVS_RCVR) {
      sprintf(out, "Dev:  NVS %2d channel   ", nvs_chans);
      vidstr(VER_ROW+1, VER_COL, WHITE,  out);
      show_rcvr_rate();
   }
   else if(rcvr_type == SCPI_RCVR) {
      vidstr(VER_ROW+1, VER_COL, WHITE,  "Dev:  SCPI receiver    ");
      show_rcvr_rate();
   }
   else if(rcvr_type == SIRF_RCVR) {
      vidstr(VER_ROW+1, VER_COL, WHITE,  "Dev:  SIRF receiver    ");
      show_rcvr_rate();
   }
   else if(rcvr_type == STAR_RCVR) {
      if(star_module[0]) {
         sprintf(out, "Dev:  %s", star_module);
         vidstr(VER_ROW+1, VER_COL, WHITE,  out);
      }
      else {
         vidstr(VER_ROW+1, VER_COL, WHITE,  "Dev:  STAR receiver    ");
      }
      show_rcvr_rate();
   }
   else if(rcvr_type == UCCM_RCVR) {
      vidstr(VER_ROW+1, VER_COL, WHITE,  "Dev:  UCCM receiver    ");
      show_rcvr_rate();
   }
   else if(rcvr_type == UBX_RCVR) {
      if(saw_timing_msg) vidstr(VER_ROW+1, VER_COL, WHITE,  "Dev:  Ublox timing     ");
      else               vidstr(VER_ROW+1, VER_COL, WHITE,  "Dev:  Ublox receiver   ");
      show_rcvr_rate();
   }
   else if(rcvr_type == VENUS_RCVR) {
      if(saw_timing_msg) {
         if(venus_kern[0]) sprintf(out, "Dev:  Venus-T %s", venus_kern);
         else              sprintf(out, "%s", "Dev:  Venus timing rcvr");
      }
      else {
         if(venus_kern[0]) sprintf(out, "Dev:  Venus %s ", venus_kern);
         else              sprintf(out, "%s", "Dev:  Venus receiver   ");
      }
      vidstr(VER_ROW+1, VER_COL, WHITE,  out);
      show_rcvr_rate();
   }
   else if(rcvr_type == ZODIAC_RCVR) {
      if(zod_chans[0]) sprintf(out, "Dev:  Jupiter %2s chan    ", zod_chans);
      else             sprintf(out, "Dev:  Jupiter receiver   ");
      vidstr(VER_ROW+1, VER_COL, WHITE,  out);
      show_rcvr_rate();
   }
   else if(discipline == 0)  vidstr(VAL_ROW+0, VAL_COL, GREEN,  "Phase locked    ");
   else if(discipline == 1)  vidstr(VAL_ROW+0, VAL_COL, YELLOW, "Warming up      ");
   else if(discipline == 2)  vidstr(VAL_ROW+0, VAL_COL, YELLOW, "Freq locking    ");
   else if(discipline == 3)  vidstr(VAL_ROW+0, VAL_COL, YELLOW, "Placing PPS     ");
   else if(discipline == 4)  vidstr(VAL_ROW+0, VAL_COL, YELLOW, "Filter init     ");
   else if(discipline == 5)  vidstr(VAL_ROW+0, VAL_COL, YELLOW, "OCXO comp       ");
   else if(discipline == 6) {
      if(osc_control_on) vidstr(VAL_ROW+0, VAL_COL, OSC_PID_COLOR, "Discipline OSC PID   ");
      else               vidstr(VAL_ROW+0, VAL_COL, RED,           "Discipline OFF       ");
   }
   else if(discipline == 8) vidstr(VAL_ROW+0, VAL_COL, YELLOW, "Recovery mode   ");
   else {
      sprintf(out, "Discipline: %02X  ", discipline);
      vidstr(VAL_ROW+0, VAL_COL, RED, out);
   }

   if((rcvr_type == NMEA_RCVR) || (rcvr_type == GPSD_RCVR) || (rcvr_type == STAR_RCVR)) {
      sprintf(out, "                    ");
   }
   else if(TIMING_RCVR) {
      if(have_sawtooth) {
         if((dac_voltage >= 1000.0) || (dac_voltage <= -100.0)) {
            sprintf(out, "Sawt:%10.5f ns  ", dac_voltage);  // pps_quant);
         }
         else {
            sprintf(out, "Sawt:%10.6f ns  ", dac_voltage);  // pps_quant);
         }
      }
      else sprintf(out, "                    ");
   }
   else if(rcvr_type == TSIP_RCVR) {
      sprintf(out, "DAC:%11.6f %s   ", dac_voltage, "V");
   }
   else {
      sprintf(out, "DAC:%11.6f %s   ", dac_voltage, plot[DAC].units);
   }

   if     (critical_alarms & 0x0010) vidstr(VAL_ROW+2, VAL_COL, RED,    out);
   else if(minor_alarms & 0x0001)    vidstr(VAL_ROW+2, VAL_COL, YELLOW, out);
   else if(osc_control_on)           vidstr(VAL_ROW+2, VAL_COL, OSC_PID_COLOR, out);
   else                              vidstr(VAL_ROW+2, VAL_COL, WHITE,  out);

   if((rcvr_type == UCCM_RCVR) && (scpi_type == UCCMP_TYPE)) {
      val = (float) temperature;
      if((val >= 100000000.0F) || (val <= (-10000000.0F))) {
         sprintf(out, "%s%10.0f%s   ", "Tcor:",val, ppt_string);
      }
      else if((val >= 10000000.0F) || (val <= (-1000000.0F))) {
         sprintf(out, "%s%10.1f%s   ", "Tcor:",val, ppt_string);
      }
      else if((val >= 1000000.0F) || (val <= (-100000.0F))) {
         sprintf(out, "%s%10.2f%s   ", "Tcor:",val, ppt_string);
      }
      else if((val >= 100000.0F) || (val <= (-10000.0F))) {
         sprintf(out, "%s%10.3f%s   ", "Tcor:",val, ppt_string);
      }
      else if((val >= 10000.0F) || (val <= (-1000.0F))) {
         sprintf(out, "%s%10.4f%s   ", "Tcor:",val, ppt_string);
      }
      else if((val >= 1000.0F) || (val <= (-100.0F))) {
         sprintf(out, "%s%10.5f%s   ", "Tcor:",val, ppt_string);
      }
      else {
         sprintf(out, "%s%10.6f%s   ", "Tcor:",val, ppt_string);
      }
      vidstr(VAL_ROW+1, VAL_COL, WHITE, out);
   }
   else if(!have_temperature) vidstr(VAL_ROW+1, VAL_COL, GREY, "Temp: no sensor");
   else if((temperature > (-100.0)) && (temperature < 100.0)) {
      if((rcvr_type == SCPI_RCVR) && (scpi_type == NORTEL_TYPE)) {
         if(temperature > 70.0F)     sprintf(out, "Temp: >70C     ");
         else if(temperature < 0.0F) sprintf(out, "Temp: <0C      ");
         else                        sprintf(out, "Temp: OK       ");
      }
      else sprintf(out, "Temp: %s%c   ", fmt_temp(temperature), temp_dir);

      if((temperature < 0.0F) || (temperature >= 60.0F))  {
         vidstr(VAL_ROW+1, VAL_COL, RED, out);
      }
      else if((temperature < 5.0F) || (temperature >= 55.0F)) {
         vidstr(VAL_ROW+1, VAL_COL, YELLOW, out);
      }
      else if(!tbolt_e && ((temperature < 10.0F) || (temperature >= 50.0F))) {
         vidstr(VAL_ROW+1, VAL_COL, YELLOW, out);
      }
      else {
         vidstr(VAL_ROW+1, VAL_COL, WHITE, out);
      }
   }
   else {
      sprintf(out, "Temp: %s%c   ", fmt_temp(temperature), temp_dir);
      vidstr(VAL_ROW+1, VAL_COL, RED, out);
   }


   sprintf(out, "                      ");
   if((rcvr_type == GPSD_RCVR) && !saw_gpsd_pps) ;
   else if(rcvr_type == NMEA_RCVR) ;
   else if(rcvr_type == STAR_RCVR) ;
   else if(rcvr_type == VENUS_RCVR) ;
   else if(luxor) {
      sprintf(out, "PPS%c%11.6f V     ", pps_polarity?DOWN_ARROW:UP_ARROW, (float) pps_offset);
   }
   else if(TIMING_RCVR) {
      if(rcvr_type == MOTO_RCVR) {
         val = (float) (pps_offset / 1000.0);
         if     (val >= 1000000000.0F) sprintf(out, "Accu:%10.0f us    ", val);
         else if(val >= 100000000.0F)  sprintf(out, "Accu:%10.0f us    ", val);
         else if(val >= 10000000.0F)   sprintf(out, "Accu:%10.1f us    ", val);
         else if(val >= 1000000.0F)    sprintf(out, "Accu:%10.2f us    ", val);
         else if(val >= 100000.0F)     sprintf(out, "Accu:%10.3f us    ", val);
         else if(val >= 10000.0F)      sprintf(out, "Accu:%10.4f us    ", val);
         else if(val >= 1000.0F)       sprintf(out, "Accu:%10.5f us    ", val);
         else if(val >= 0.0F)          sprintf(out, "Accu:%10.6f us    ", val);
         else if(val <= -1000000.0F)   sprintf(out, "Accu:%10.0f us    ", val);
         else if(val <= -100000.0F)    sprintf(out, "Accu:%10.1f us    ", val);
         else if(val <= -10000.0F)     sprintf(out, "Accu:%10.2f us    ", val);
         else if(val <= -1000.0F)      sprintf(out, "Accu:%10.3f us    ", val);
         else if(val <= -100.0F)       sprintf(out, "Accu!%10.4f us    ", val);
         else                          sprintf(out, "Accu:%10.6f us    ", val);
      }
      else if(rcvr_type == NVS_RCVR) {
         val = (float) (pps_offset / 1.0);
         if     (val >= 1000000000.0F) sprintf(out, "Rgen:%10.0f ns/s  ", val);
         else if(val >= 100000000.0F)  sprintf(out, "Rgen:%10.0f ns/s  ", val);
         else if(val >= 10000000.0F)   sprintf(out, "Rgen:%10.1f ns/s  ", val);
         else if(val >= 1000000.0F)    sprintf(out, "Rgen:%10.2f ns/s  ", val);
         else if(val >= 100000.0F)     sprintf(out, "Rgen:%10.3f ns/s  ", val);
         else if(val >= 10000.0F)      sprintf(out, "Rgen:%10.4f ns/s  ", val);
         else if(val >= 1000.0F)       sprintf(out, "Rgen:%10.5f ns/s  ", val);
         else if(val >= 0.0F)          sprintf(out, "Rgen:%10.6f ns/s  ", val);
         else if(val <= -1000000.0F)   sprintf(out, "Rgen:%10.0f ns/s  ", val);
         else if(val <= -100000.0F)    sprintf(out, "Rgen:%10.1f ns/s  ", val);
         else if(val <= -10000.0F)     sprintf(out, "Rgen:%10.2f ns/s  ", val);
         else if(val <= -1000.0F)      sprintf(out, "Rgen:%10.3f ns/s  ", val);
         else if(val <= -100.0F)       sprintf(out, "Rgen:%10.4f ns/s  ", val);
         else                          sprintf(out, "Rgen:%10.6f ns/s  ", val);
      }
      else if(rcvr_type == SIRF_RCVR) {
         val = (float) (pps_offset / 1000.0);
         if     (val >= 1000000000.0F) sprintf(out, "Drft:%10.0f us    ", val);
         else if(val >= 100000000.0F)  sprintf(out, "Drft:%10.0f us    ", val);
         else if(val >= 10000000.0F)   sprintf(out, "Drft:%10.1f us    ", val);
         else if(val >= 1000000.0F)    sprintf(out, "Drft:%10.2f us    ", val);
         else if(val >= 100000.0F)     sprintf(out, "Drft:%10.2f us    ", val);
         else if(val >= 10000.0F)      sprintf(out, "Drft:%10.4f us    ", val);
         else if(val >= 1000.0F)       sprintf(out, "Drft:%10.4f us    ", val);
         else                          sprintf(out, "Drft:%10.6f us    ", val);
      }
      else if(rcvr_type == UBX_RCVR) {
         val = (float) (pps_offset);
         if     (val >= 1000000000.0F) sprintf(out, "Accu:%10.0f ns    ", val);
         else if(val >= 100000000.0F)  sprintf(out, "Accu:%10.0f ns    ", val);
         else if(val >= 10000000.0F)   sprintf(out, "Accu:%10.1f ns    ", val);
         else if(val >= 1000000.0F)    sprintf(out, "Accu:%10.2f ns    ", val);
         else if(val >= 100000.0F)     sprintf(out, "Accu:%10.3f ns    ", val);
         else if(val >= 10000.0F)      sprintf(out, "Accu:%10.4f ns    ", val);
         else if(val >= 1000.0F)       sprintf(out, "Accu:%10.5f ns    ", val);
         else                          sprintf(out, "Accu:%10.6f ns    ", val);
      }
      else if(rcvr_type == ZODIAC_RCVR) {
         c = ':';
         goto fmt_pps;
      }             
      else {
         val = (float) (pps_offset / 1000.0);
         if     (val >= 1000000000.0F) sprintf(out, "Bias:%10.0f us    ", val);
         else if(val >= 100000000.0F)  sprintf(out, "Bias:%10.0f us    ", val);
         else if(val >= 10000000.0F)   sprintf(out, "Bias:%10.1f us    ", val);
         else if(val >= 1000000.0F)    sprintf(out, "Bias:%10.2f us    ", val);
         else if(val >= 100000.0F)     sprintf(out, "Bias:%10.3f us    ", val);
         else if(val >= 10000.0F)      sprintf(out, "Bias:%10.4f us    ", val);
         else if(val >= 1000.0F)       sprintf(out, "Bias:%10.5f us    ", val);
         else                          sprintf(out, "Bias:%10.6f us    ", val);
      }
   }
   else {
//    sprintf(out, "PPS%c%11.6f ns    ", pps_polarity?DOWN_ARROW:UP_ARROW, (float) pps_offset);
      if(pps_polarity) c = DOWN_ARROW;
      else             c = UP_ARROW;

      fmt_pps:
      val = (float) pps_offset;
      if     (val >= 10000000000.0F)   sprintf(out, "PPS%c >10 seconds      ", c);
      else if(val >= 1000000000.0F)    sprintf(out, "PPS%c%11.0f ns    ", c, val);
      else if(val >= 100000000.0F)     sprintf(out, "PPS%c%11.1f ns    ", c, val);
      else if(val >= 10000000.0F)      sprintf(out, "PPS%c%11.2f ns    ", c, val);
      else if(val >= 1000000.0F)       sprintf(out, "PPS%c%11.3f ns    ", c, val);
      else if(val >= 100000.0F)        sprintf(out, "PPS%c%11.4f ns    ", c, val);
      else if(val >= 10000.0F)         sprintf(out, "PPS%c%11.5f ns    ", c, val);
      else if(val >= 0.0F)             sprintf(out, "PPS%c%11.6f ns    ", c, val);
      else if(val <= -100000000.0F)    sprintf(out, "PPS%c%11.0f ns    ", c, val);
      else if(val <= -10000000.0F)     sprintf(out, "PPS%c%11.1f ns    ", c, val);
      else if(val <= -1000000.0F)      sprintf(out, "PPS%c%11.2f ns    ", c, val);
      else if(val <= -100000.0F)       sprintf(out, "PPS%c%11.3f ns    ", c, val);
      else if(val <= -10000.0F)        sprintf(out, "PPS%c%11.4f ns    ", c, val);
      else if(val <= -1000.0F)         sprintf(out, "PPS%c%11.5f ns    ", c, val);
      else                             sprintf(out, "PPS%c%11.6f ns    ", c, val);
   }
   if(pps_enabled) vidstr(VAL_ROW+3, VAL_COL, WHITE, out);
   else            vidstr(VAL_ROW+3, VAL_COL, YELLOW, out);


   if(ebolt || res_t)    c = ':';
   else if(osc_polarity) c = DOWN_ARROW;
   else                  c = UP_ARROW;

   sprintf(out, "                      ");
   if(rcvr_type == GPSD_RCVR) ;
   else if(rcvr_type == NMEA_RCVR) ;
   else if(rcvr_type == STAR_RCVR) ;
   else if(rcvr_type == VENUS_RCVR) ;
   else if(luxor) {
      val = (float) osc_offset;
      if((val >= 10000.0F) || (val <= (-1000.0F))) {
         sprintf(out, "Osc%c%11.4f%s   ", c, val, "V  ");
      }
      else if((val >= 1000.0F) || (val <= (-100.0F))) {
         sprintf(out, "Osc%c%11.5f%s   ", c, val, "V  ");
      }
      else {
         sprintf(out, "Osc%c%11.6f%s   ", c, val, "V  ");
      }
   }
   else if(rcvr_type == NVS_RCVR) {
   }
   else if((rcvr_type == SCPI_RCVR) && (scpi_type == NORTEL_TYPE)) {
   }
   else if(rcvr_type == SCPI_RCVR) {
      val = (float) osc_offset * 1000.0F;
      c = ':';
      if((val >= 10000.0F) || (val <= (-1000.0F))) {
         sprintf(out, "%s%c%11.4f%s   ", plot[OSC].plot_id, c, val, " us");
      }
      else if((val >= 1000.0F) || (val <= (-100.0F))) {
         sprintf(out, "%s%c%11.5f%s   ", plot[OSC].plot_id, c, val, " us");
      }
      else {
         sprintf(out, "%s%c%11.6f%s   ", plot[OSC].plot_id, c, val, " us");
      }
   }
   else if(rcvr_type == UBX_RCVR) {
      val = (float) osc_offset;
      c = ':';
      if((val >= 10000.0F) || (val <= (-1000.0F))) {
         sprintf(out, "%s%c%10.0f%s   ", plot[OSC].plot_id, c, val, " ns");
      }
      else if((val >= 1000.0F) || (val <= (-100.0F))) {
         sprintf(out, "%s%c%10.1f%s   ", plot[OSC].plot_id, c, val, " ns");
      }
      else {
         sprintf(out, "%s%c%10.2f%s   ", plot[OSC].plot_id, c, val, " ns");
      }
   }
   else if(rcvr_type == ZODIAC_RCVR) {  // clock bias
      val = (float) osc_offset;
      c = ':';
      if((val >= 10000.0F) || (val <= (-1000.0F))) {
         sprintf(out, "%s%c%10.4f%s   ", plot[OSC].plot_id, c, val, " ns");
      }
      else if((val >= 1000.0F) || (val <= (-100.0F))) {
         sprintf(out, "%s%c%10.5f%s   ", plot[OSC].plot_id, c, val, " ns");
      }
      else {
         sprintf(out, "%s%c%10.6f%s   ", plot[OSC].plot_id, c, val, " ns");
      }
   }
   else if(TIMING_RCVR) {
      val = (float) osc_offset;
      if(rcvr_type == MOTO_RCVR) ;
      else if(rcvr_type == SIRF_RCVR) ;
      else if(show_euro_ppt) ppt_string = "e-9 ";
      else ppt_string = " ppb";

      if((val >= 100000000.0F) || (val <= (-10000000.0F))) {
         sprintf(out, "%s%c%10.0f%s   ", plot[OSC].plot_id, c, val, ppt_string);
      }
      else if((val >= 10000000.0F) || (val <= (-1000000.0F))) {
         sprintf(out, "%s%c%10.1f%s   ", plot[OSC].plot_id, c, val, ppt_string);
      }
      else if((val >= 1000000.0F) || (val <= (-100000.0F))) {
         sprintf(out, "%s%c%10.2f%s   ", plot[OSC].plot_id, c, val, ppt_string);
      }
      else if((val >= 100000.0F) || (val <= (-10000.0F))) {
         sprintf(out, "%s%c%10.3f%s   ", plot[OSC].plot_id, c, val, ppt_string);
      }
      else if((val >= 10000.0F) || (val <= (-1000.0F))) {
         sprintf(out, "%s%c%10.4f%s   ", plot[OSC].plot_id, c, val, ppt_string);
      }
      else if((val >= 1000.0F) || (val <= (-100.0F))) {
         sprintf(out, "%s%c%10.5f%s   ", plot[OSC].plot_id, c, val, ppt_string);
      }
      else {
         sprintf(out, "%s%c%10.6f%s   ", plot[OSC].plot_id, c, val, ppt_string);
      }
   }
   else {
      val = (float) osc_offset * 1000.0F;
      if((val >= 100000000.0F) || (val <= (-10000000.0F))) {
         sprintf(out, "%s%c%11.0f%s   ", plot[OSC].plot_id, c, val, ppt_string);
      }
      else if((val >= 10000000.0F) || (val <= (-1000000.0F))) {
         sprintf(out, "%s%c%11.1f%s   ", plot[OSC].plot_id, c, val, ppt_string);
      }
      else if((val >= 1000000.0F) || (val <= (-100000.0F))) {
         sprintf(out, "%s%c%11.2f%s   ", plot[OSC].plot_id, c, val, ppt_string);
      }
      else if((val >= 100000.0F) || (val <= (-10000.0F))) {
         sprintf(out, "%s%c%11.3f%s   ", plot[OSC].plot_id, c, val, ppt_string);
      }
      else if((val >= 10000.0F) || (val <= (-1000.0F))) {
         sprintf(out, "%s%c%11.4f%s   ", plot[OSC].plot_id, c, val, ppt_string);
      }
      else if((val >= 1000.0F) || (val <= (-100.0F))) {
         sprintf(out, "%s%c%11.5f%s   ", plot[OSC].plot_id, c, val, ppt_string);
      }
      else {
         sprintf(out, "%s%c%11.6f%s   ", plot[OSC].plot_id, c, val, ppt_string);
      }
   }
   vidstr(VAL_ROW+4, VAL_COL, WHITE, out);
}


void update_plot_data()
{
   // add the latest data to the plot queue and update the screen
   if(have_time && (pause_data == 0)) {
      if(continuous_scroll) update_plot(2);
      else if((all_adevs == 0) || mixed_adevs) update_plot(1);
      else if(all_adevs) update_plot(1);
      else update_plot(0);

      if(first_sample) {
         if(rcvr_type == SCPI_RCVR) ;
         else if(rcvr_type == UCCM_RCVR) ;
         else {
            if(discipline_mode == 6) osc_discipline = 0;
            else                     osc_discipline = 1;

            if(discipline_mode == 3) user_holdover = 1;
            else                     user_holdover = 0;
         }

         if((all_adevs == 0) || mixed_adevs) draw_plot(1);

         first_sample = 0;
      }
   }
   else {
      refresh_page();
   }
}



//
//
//  System clock only device
//
//


void get_norcvr_message()
{
static int last_tick = (9999);
int this_tick;
double ss;

   // system clock driven receiverless mode handler

   get_clock_time();   // get system clock time in UTC

   year = pri_year = clk_year;
   month = pri_month = clk_month;
   day = pri_day = clk_day;

   hours = pri_hours = clk_hours;
   minutes = pri_minutes = clk_minutes;
   seconds = pri_seconds = clk_seconds;
   pri_frac = raw_frac = clk_frac;

   ss = (((double) seconds) + pri_frac) * nav_rate;
   this_tick = (int) ss;

   if(timing_mode == 0) {     // convert utc time to gps time
      utc_to_gps();
   }
   else {
      adjust_rcvr_time(0.0);  // incorporarte possibly negative fractional second into time variables
   }

   if(last_tick != this_tick) {
      last_tick = this_tick;
      update_gps_screen(555);
   }
}


//
//
//  NVS receiver stuff
//
//

void parse_nvs_speed()
{
   heading = tsip_single();
   speed = tsip_single() * 0.2777778; // km/hr -> m/sec
   tsip_dword();  // time stamp

   have_heading = 12;
   have_speed = 12;
}

void parse_nvs_test()
{
u08 subcode;
   // 0x43 packet

   subcode = tsip_byte();
   if(subcode == 2) {
      nvs_test = tsip_byte();
      tsip_dword();

      minor_alarms &= (~0x0006);
      if((nvs_test & 0xC0) == 0xC0) minor_alarms |= 0x0004; // short
      else if((nvs_test & 0xC0) == 0x80) minor_alarms |= 0x0002; // open
      else if((nvs_test & 0xC0) == 0x00) ; // OK
      else minor_alarms |= 0x0006;; // unknown
      have_antenna = 1;

      if(!have_critical_alarms) need_redraw = 7709;
      have_critical_alarms = 9;

      if(nvs_test & 0x30) {  // gps(0x10) / glonass(0x20) failure
         critical_alarms |= 0x0008;
      }
      else {
         critical_alarms &= (~0x0008);
      }
   }
}


void parse_nvs_date()
{
u32 tow;
u08 dd,mm;
u16 yy;
u08 tz_hh,tz_mm;
double delta;

   // 0x46 packet

   tow = tsip_dword();
   dd = tsip_byte();
   mm = tsip_byte();
   yy = tsip_word();
   tz_hh = tsip_byte();
   tz_mm = tsip_byte();

   delta = nvs_msecs - ((double)tow*1000.0);

//sprintf(debug_text2, "%04d/%02d/%02d  tz:%02d:%02d  tow:%d  msecs:%f  delta:%f", 
//yy,mm,dd, tz_hh,tz_mm, tow,nvs_msecs, delta);
}

void parse_nvs_disable()
{
int i;
u08 sys;
u08 prn;
u08 flag;

   // 0x47 packet

   for(i=0; i<32+24; i++) {
      sys = tsip_byte();
      prn = tsip_byte();
      flag = tsip_byte();

      if(sys == 1) ;
      else if(sys == 2) prn += NVS_GLONASS_PRN;
      else continue;

      if(flag == 1) ;     // enabled
      else if(flag == 2); // disabled
      else continue;

      if((prn >= 1) && (prn <= MAX_PRN)) {
         flag -= 1;
         sat[prn].disabled = flag;
         if(prn <= 32) {  // gps sats
            if(flag) sats_enabled &= (~(1 << (prn-1)));
            else     sats_enabled |= (1 << (prn-1));
         }
      }
   }
}

void parse_nvs_iono()
{
float a0,a1,a2,a3;
float b0,b1,b2,b3;
u08 reliability;

   // 0x4A packet - ionosphere data
   a0 = tsip_single();
   a1 = tsip_single();
   a2 = tsip_single();
   a3 = tsip_single();
   b0 = tsip_single();
   b1 = tsip_single();
   b2 = tsip_single();
   b3 = tsip_single();
   reliability = tsip_byte();
}

void parse_nvs_gnss_timescale()
{
double a0, a1;
int tow;
int wnt;
int ls;
int wnf;
int dn;
int lsf;
int this_dn;
int wraps;

   // 0x4B packet - calculate pending leapsecond info
   a1 = tsip_double();
   a0 = tsip_double();
   tow = tsip_dword();

   wnt = tsip_word();
   ls = (int) (s16) tsip_word();
   wnf = tsip_word();  // gps week
   dn = tsip_word();
   lsf = (int) (s16) tsip_word();

   this_dn = tow / (24*60*60);
   wraps = gps_week / 1024;
///sprintf(debug_text, "gps_week:%d wraps:%d wnt:%d ls:%d wnf:%d dn:%d lsf:%d this_dn:%d", 
///gps_week, wraps, wnt, ls, wnf, dn, lsf, this_dn);
// wnf += (wraps * 1024);

   // we ignore the rest of this message

   minor_alarms &= (~0x0080);
   leap_days = (-1);
   have_leap_days = 0;
   if(ls != lsf) {
      minor_alarms |= 0x0080;
      calc_leap_days(wnf+(gps_week/256)*256, dn, 555);
      have_leap_info = 22;
   }
   have_leap_info = 22;
}

void parse_nvs_config()
{
u08 nav_sys;

   // 0x51 packet

   nav_sys = tsip_byte();
   tsip_byte();    // rsvd
   tsip_byte();    // coord sys
   el_mask = (float) (s08) tsip_byte();
   amu_mask = (float) tsip_byte();
   tsip_word();    // max rms error
   nvs_filter = tsip_single();  // filtration degree

   have_el_mask = 1;
   have_amu = 1;

   // nav system:
   //  0=GPS+GLONASS
   //  1=GPS
   //  2=GLONASS
   //  3=Galileo
   // 10=GPS+GLONASS+SBAS
   // 11=GPS+SBAS
   // 12=GLONASS+SBAS
   have_gnss_mask = 3;
   if     (nav_sys == 0)  gnss_mask = (GPS | GLONASS);
   else if(nav_sys == 1)  gnss_mask = (GPS);
   else if(nav_sys == 2)  gnss_mask = (GLONASS);
   else if(nav_sys == 3)  gnss_mask = (GALILEO);
   else if(nav_sys == 10) gnss_mask = (GPS | GLONASS | SBAS);
   else if(nav_sys == 11) gnss_mask = (GPS | SBAS);
   else if(nav_sys == 12) gnss_mask = (GLONASS | SBAS);
   else if(nav_sys == 13) gnss_mask = (GALILEO | SBAS);
   else have_gnss_mask = 0;
// sprintf(plot_title, "nav sys:%d", nav_sys); //qqqqqqqqq
}


void parse_nvs_sats()
{
u08 sat_sys;
int prn;
u08 slot;
u08 el;
s16 az;
u08 snr;

   // 0x52 packet

   for(prn=1; prn<=MAX_PRN; prn++) {  // reset tracking info
      sat[prn].level_msg = 0;
      sat[prn].tracking = 0;
   }

   sat_count = 0;
   while(tsip_rptr < (tsip_wptr-0)) {
      sat_sys = tsip_byte();
      prn = (int) tsip_byte();
      slot = tsip_byte();
      el = tsip_byte();
      az = (s16) tsip_word();
      snr = tsip_byte();

      if(sat_sys == 2) prn += NVS_GLONASS_PRN;

      if((prn >= 1) && (prn <= MAX_PRN)) {
         sat[prn].level_msg = 77;
         if(snr) sat[prn].tracking = 1;
         else    sat[prn].tracking = (-1);
         sat[prn].sig_level = (float) snr;
         sat[prn].azimuth = (float) az;
         set_sat_el(prn, (float) el);
         record_sig_levels(prn);
         ++sat_count;
         have_count = 222;
      }
   }

   level_type = "SNR";
   config_sat_count(sat_count); 
}


void parse_nvs_mode()
{
u08 mode;
double delay;
int avg_time;

   // 0x55 packet

   mode = tsip_byte();
   delay = tsip_double();
   avg_time = (int) tsip_word();

   if(mode == 0) {
      if(nvs_traim & 0x04) rcvr_mode = RCVR_MODE_3D;
      else                 rcvr_mode = RCVR_MODE_2D_3D;
   }
   else if(mode == 1) rcvr_mode = RCVR_MODE_HOLD;
   else if(mode == 2) rcvr_mode = RCVR_MODE_SURVEY;
   else               rcvr_mode = RCVR_MODE_UNKNOWN;

   if(mode == 2) {
      minor_alarms |= 0x0020;
      do_survey = avg_time;
      survey_why = 33;
   }
   else {
      minor_alarms &= (~0x0020);
      if(precision_survey == 0) {
         do_survey = 0;
         survey_why = (-333);
      }
   }
}


void parse_nvs_dops()
{
int gps_count;
int glonass_count;

   // 0x60 packet

   gps_count = tsip_byte();
   glonass_count = tsip_byte();
   hdop = tsip_single();
   vdop = tsip_single();
   have_dops |= (HDOP | VDOP);
}


void parse_nvs_pvtdops()
{
   // 0x61 packet

   hdop = tsip_single();
   vdop = tsip_single();
   tdop = tsip_single();
   // we ignore the rest of this message
   have_dops |= (HDOP | VDOP | TDOP);
}


void parse_nvs_version()
{
int i;

   // 0x70 packet

   nvs_chans = tsip_byte();
   for(i=0; i<21; i++) {  // version string
      nvs_id[i] = tsip_byte();
      nvs_id[i+1] = 0;
   }
   nvs_sn = tsip_dword();

   for(i=0; i<21; i++) {  // version string
      nvs_id2[i] = tsip_byte();
      nvs_id2[i+1] = 0;
   }
   nvs_sn2 = tsip_dword();

   for(i=0; i<21; i++) {  // version string
      nvs_id3[i] = tsip_byte();
      nvs_id3[i+1] = 0;
   }
   nvs_sn3 = tsip_dword();

   saw_version |= 0x2000;

   // we ignore the rest of this message
}



void parse_nvs_time()
{
double msecs;
u08 scale;
int week;
int i;
double gen_dev;
double time_dev;

   // 0x72 packet

   nvs_msecs = msecs = tsip_fp80(); //  - 67108864.0;  // qqqqqqqqqqqqqq - 67....
   week = (int) (s16) tsip_word();
   scale = tsip_byte();
   gen_dev = tsip_double();   // ref gen deviation
   time_dev = tsip_double();  // sawtooth
   i = (int) (s16) tsip_word();
   if(!user_set_utc_ofs) {
      utc_offset = i;
      check_utc_ofs(12);
   }

   tsip_byte();
   tsip_word();

   dac_voltage = (float) time_dev;
   have_sawtooth = 6666;
}


void parse_nvs_survey()
{
u08 mode;
double delay;
int avg_time;

   // 0x73 packet

   mode = tsip_byte();
   delay = tsip_double();
   avg_time = (int) tsip_word();

   if(mode == 0) {
      if(nvs_traim & 0x04) rcvr_mode = RCVR_MODE_3D;
      else                 rcvr_mode = RCVR_MODE_2D_3D;
   }
   else if(mode == 1) rcvr_mode = RCVR_MODE_HOLD;
   else if(mode == 2) rcvr_mode = RCVR_MODE_SURVEY;
   else               rcvr_mode = RCVR_MODE_UNKNOWN;

   if(mode == 2) {
      minor_alarms |= 0x0020;
      do_survey = avg_time;
      survey_why = 33;
   }
   else {
      minor_alarms &= (~0x0020);
      if(precision_survey == 0) {
         do_survey = 0;
         survey_why = (-334);
      }
   }
}



void parse_nvs_timescale()
{
double scale_shift;
u08 flags;
int gps_flags;
int glo_flags;

   // !!!! qqqqq do we need anything from here?
   // 0x74 packet
   scale_shift = tsip_fp80();
   tsip_fp80();
   tsip_fp80();
   tsip_fp80();
   tsip_fp80();
   flags = tsip_byte();

   gps_flags = glo_flags = 0x0000;
   if(!have_gnss_mask) {
      gps_flags |= 0x0004;
      glo_flags |= 0x0004;
   }

   if(time_flags & 0x0001) {  // UTC time
      if(gnss_mask & GPS) {
         if((flags & 0x04) == 0) gps_flags |= 0x0004;
      }
      if(gnss_mask & GLONASS) {
         if((flags & 0x08) == 0) glo_flags |= 0x0004;
      }
   }
   else { // GPS time
      if(gnss_mask & GPS) {
         if((flags & 0x01) == 0) gps_flags |= 0x0004;
      }
      if(gnss_mask & GLONASS) {
         if((flags & 0x02) == 0) glo_flags |= 0x0004;
      }
   }
   if((gnss_mask & GPS) && (gnss_mask & GLONASS)) {  // see if we have valid time from at least one system
      if(gps_flags == 0) glo_flags = 0x0000;  // we have GPS time
      if(glo_flags == 0) gps_flags = 0x0000;  // we have GLONASS time
   }


   time_flags &= (~0x0004);
   time_flags |= (gps_flags | glo_flags);
//sprintf(plot_title, "shift:%.10f  flags:%02X  tflags:%04X  gnss:%04X  have_utc:%d", 
//scale_shift, flags, time_flags, gnss_mask, have_utc_ofs);
}


void parse_nvs_pvt()
{
u08 status;
double msecs;
int week;
double lat_speed;
double lon_speed;
double alt_speed;
double gen_dev;
double jd;

   // 0x88 packet

   lat = tsip_double();
   lon = tsip_double();
   alt = tsip_double();

   tsip_single(); // RMS posn err
   msecs = tsip_fp80();       // time stamp msecs
   week = tsip_word();        // gps week
   lat_speed = tsip_double(); // lat speed
   lon_speed = tsip_double(); // lon speed
   alt_speed = tsip_double(); // alt speed
   gen_dev = (double) tsip_single(); // ref gen deviation (msecs/sec)
   status = tsip_byte();  // !!!! has 2D flag qqqqqq

   pps_offset = gen_dev * 1.0E6; 
   have_pps_offset = 2;

   if(week >= 0) {
      gps_week = week + 1024;
      if(have_week == 0) need_redraw = 2007;
      have_week = 222;
   }

   pri_tow = tow = this_tow = (int) (msecs / 1000.0);
   if(have_tow == 0) need_redraw = 2008;
   have_tow = 222;


   jd = msecs / 1000.0;
   
   jd /= (24.0*60.0*60.0);
   jd += jdate(1999,8,22);
   jd += (week * 7.0);

   gregorian(jd);

   pri_year = year = g_year;
   pri_month = month = g_month;
   pri_day = day = g_day;
   pri_hours = hours = g_hours;
   pri_minutes = minutes = g_minutes;
   pri_seconds = seconds = g_seconds;
   pri_frac = raw_frac = g_frac;

   if(timing_mode == 0) {  // we want UTC time - convert gps time to utc
      utc_to_gps();
   }

   update_gps_screen(1222); 
}


void parse_nvs_used()
{
   // !!!! qqqqq do we need anything from here?
   // 0x93 packet
}


void parse_nvs_rcvr_config()
{
u08 sub_code;
u16 flags;
u08 keep;
u08 scale;
float rate;

   // 0xE7 packet

   sub_code = tsip_byte();
   if(sub_code == 0x02) {
      rate = (float) tsip_byte();
      if(!pause_data) {
         nav_rate = rate;
         have_nav_rate = 3;
      }
   }
   else if(sub_code == 0x04) {  // solution filtration factor
      nvs_filter = tsip_single();
   }
   else if(sub_code == 0x05) {  // pps info
      flags = tsip_byte();
      keep = tsip_byte();
      nvs_pps_width = tsip_dword();

      if(flags & 0x08) pps_polarity = 0; // positive
      else             pps_polarity = 1; // inverted
      have_pps_polarity = 1;
      have_pps_freq = 0x01;

      if(nvs_pps_width) {
         last_nvs_pps_width = nvs_pps_width;
         pps_enabled = 1;
      }
      else pps_enabled = 0;
      have_pps_enable = 20;

      scale = flags & 0xF0;
      if((scale == 0x10) || (scale == 0x20)) {  // GPS/GLONASS time
         time_flags &= (~0x0001);
         timing_mode = 0x00;
         have_timing_mode = 1;
      }
      else if((scale == 0x30) || (scale == 0x40)) {  // UTC
         time_flags |= (0x0001);
         timing_mode = 0x03;
         have_timing_mode = 1;
      }
//sprintf(plot_title, "pps flags:%02X  keep:%02X  width:%d", flags,keep,nvs_pps_width);  // qqqqqqqqqqqqqq
   }
   else if(sub_code == 0x06) {  // cable delay
      cable_delay = tsip_double() / 1.0E6;
//    if(have_cable_delay == 0) need_redraw = 2237;
      have_cable_delay = 1;
   }
   else if(sub_code == 0x07) {  // TRAIM mode, 2D OK, single sat OK
      flags = tsip_word();
      nvs_traim = flags;
      traim_mode = flags & 0x03;
      have_traim = 1;
   }
}


void parse_nvs_raw()
{
double tow;
int week;
double gps_shift;
double glonass_shift;
int corr;

u08 type;
u08 prn;
u08 glonass_carrier;
u08 snr;
double phase;
double range;
double doppler;
u08 flags;
u08 rsvd;

   // 0xF5 packet

   tow = tsip_double();
   week = tsip_word();
   gps_shift = tsip_double();
   glonass_shift = tsip_double();
   corr = (int) (s08) tsip_byte();

   while(tsip_rptr < tsip_wptr) {
      type = tsip_byte();
      prn = tsip_byte();
      glonass_carrier = tsip_byte();
      snr = tsip_byte();
      phase = tsip_double();
      range = tsip_double();
      doppler = tsip_double();
      flags = tsip_byte();
      rsvd = tsip_byte();

      if(type == 0x01) prn += NVS_GLONASS_PRN;
      else if(type == 0x02) prn += 0;
      else {
         continue;
      }

      sat[prn].code_phase = phase;
      sat[prn].doppler = doppler;
      sat[prn].range = range;
      have_doppler = 1;
      have_phase = 1;
      have_range = 1;
   }
}

void parse_nvs_ephem()
{
   // 0xF5 packet  // !!!! requested by 0xF4, we need to parse this out
}


void decode_nvs_message()
{
u08 id;

   msg_fault = 0x00;
   tsip_rptr = 0;
   id = get_next_tsip();

   subcode = 0x00;
   first_msg = 0;
   last_msg = msg_id;
   ++wakeup_tsip_msg; // we have seen a TSIP message
   tsip_error = 0;    // this flag gets set if we see a start or ETX when we wanted normal data
   early_end = 0;
   ++packet_count;
   if(packet_count == 1L) {
      need_redraw = 4433;  // get rid of any "no serial..." message
   }

   if     (id == 0x41) parse_nvs_speed();
   else if(id == 0x43) parse_nvs_test();
   else if(id == 0x46) parse_nvs_date();
   else if(id == 0x47) parse_nvs_disable();
   else if(id == 0x4A) parse_nvs_iono();
   else if(id == 0x4B) parse_nvs_gnss_timescale();
   else if(id == 0x51) parse_nvs_config();
   else if(id == 0x52) parse_nvs_sats();
   else if(id == 0x55) parse_nvs_mode();
   else if(id == 0x60) parse_nvs_dops();
   else if(id == 0x61) parse_nvs_pvtdops();
   else if(id == 0x70) parse_nvs_version();
   else if(id == 0x72) parse_nvs_time();
   else if(id == 0x73) parse_nvs_survey();
   else if(id == 0x74) parse_nvs_timescale();
   else if(id == 0x88) parse_nvs_pvt();
   else if(id == 0x93) parse_nvs_used();
   else if(id == 0xE7) parse_nvs_rcvr_config();
   else if(id == 0xF5) parse_nvs_raw();
   else if(id == 0xF7) parse_nvs_ephem();
}


void get_nvs_message()
{
u08 c;

   // this routine buffers up an incoming TSIP message and then parses it
   // when it is complete.

   if(SERIAL_DATA_AVAILABLE() == 0) {
      check_com_timer();
      return;
   }
   else {
      reset_com_timer();
   }

   c = get_com_char();
   if(rcv_error) {      // parity/framing/overrun errors
      rcv_error = 0;
//!!!!!qqqq    goto rst_msg;
   }

   if(tsip_sync == 0) {         // syncing to start of message, search for a DLE
      if(c == DLE) {
         tsip_sync = 1;
         get_sync_time();
      }
      tsip_wptr = 0;
      tsip_rptr = 0;
      nvs_rx_crc = 0;
      return;
   }
   else if(tsip_sync == 1) {    // DLE had been seen, now checking next byte
      if(c == DLE) {            // DLE DLE is a 0x10 data byte
         goto rst_msg;
      }
      else if(c == ETX) {       // DLE ETX is end-of-message
         goto rst_msg;          // ... should never happen here
      }
      else {                    // DLE xx is message start
         tsip_sync = 2;         // ... so accumulate the message
         if(tsip_wptr < MAX_TSIP) {
            tsip_buf[tsip_wptr++] = c;
            nvs_rx_crc = calc_nvs_crc(nvs_rx_crc, c);
         }
         else {                 // buffer overlow
            tsip_error |= 0x8000; // !!!!t
            goto rst_msg;
         }
      }
   }
   else if(tsip_sync == 2) {    // buffer up the message
      if(c == DLE) tsip_sync = 3;
      else if(tsip_wptr < MAX_TSIP) {
         nvs_rx_crc = calc_nvs_crc(nvs_rx_crc, c);
         tsip_buf[tsip_wptr++] = c;
      }
      else {
         tsip_error |= 0x8000;   // !!!!t
      }
   }
   else if(tsip_sync == 3) {   // last char was a DLE
      if(c == ETX) {           // DLE ETX is end-of-message
         decode_nvs_message(); // so process the buffered message

         rst_msg:
         tsip_wptr = 0;
         tsip_sync = 0;
      }
      else if(c == 0xFF) {   // DLE 0xFF is start of CRC
         tsip_sync = 4;
      }
      else {                   // DLE DLE is a DLE data byte and DLE xx is message ID
         nvs_rx_crc = calc_nvs_crc(nvs_rx_crc, DLE);
         nvs_rx_crc = calc_nvs_crc(nvs_rx_crc, c);
         if(tsip_wptr < MAX_TSIP) {  // so add it to the message buffer
            tsip_buf[tsip_wptr++] = c;
         }
         else {
            tsip_error |= 0x8000;
         }
         tsip_sync = 2;
      }
   }
   else if(tsip_sync == 4) {  // first byte of message CRC
      nvs_msg_crc = c;
      ++tsip_sync;
   }
   else if(tsip_sync == 5) {  // second byte of message CRC
      nvs_msg_crc = (c * 256) + nvs_msg_crc;
      if(nvs_msg_crc == nvs_rx_crc) {
         decode_nvs_message();
      }
      else {
         tsip_error |= 0x8000;
      }
      goto rst_msg;
   }
   else {     // should never happen
      goto rst_msg;
   }
}


//
//
//   Receiver type detection routines
//
//

void request_rcvr_info(int why)
{
   if(no_poll) return;

   request_version();
   request_luxor_ver();
   if(luxor_time_set == 0) set_luxor_time();

   request_utc_info();

   request_pps_info();
   request_pps_mode();
   request_timing_mode();

   request_filter_config();
   request_rcvr_config(why);
   request_survey_params();

   if(rcvr_type != STAR_RCVR) {  // request_rcvr_config() did this
      request_all_dis_params();
   }

   request_sat_list();
}


void init_messages(int why)
{
int i;

   // initialize the receiver to send periodic messages and request some
   // various configuration info.

if(debug_file) fprintf(debug_file, "### init_messages(%d)\n", why);
   if(sim_file) {
      sprintf(plot_title, "Using simulation input file: %s", sim_name);
      title_type = USER;
   }

   need_msg_init = 0;
   wakeup_nortel();
   if(luxor) {
      request_luxor_ver();
      if(luxor_time_set == 0) set_luxor_time();
   }


   init_receiver(why);   // setup the receiver

   if(user_set_nav_rate) {
      set_nav_rate(user_nav_rate);
   }

   if(user_set_delay || set_pps_polarity || user_set_traim) {   // set cable delay and pps values
      set_pps(user_pps_enable, user_pps_polarity,  delay_value, pps1_delay, 300.0, 0);
      if(user_set_traim) set_traim_mode(traim_threshold, 0);
   }


   if(set_osc_polarity) {
      set_osc_sense(user_osc_polarity, 0);
   }

   set_filter_config(user_pv, user_static, user_alt, user_kalman, user_marine, 0);

   request_rcvr_info(201);   // get various receiver status messages
   request_rcvr_health();

   request_sat_list();       // get satellite info
   request_last_raw(0x00);
   request_sat_status(0x00);
   request_eph_status(0x00);

   if(log_db == 0) {
      request_sig_levels();  // get signal quality info
   }

   if(do_survey && (survey_why > 0) && (user_precision_survey == 0)) {
      set_survey_params(1, 1, do_survey);
      request_survey_params();
      start_self_survey(0x00, 6);
      survey_why = (-999);
   }


   if(user_init_len) {  // send string of user specified bytes after the standard init commands
      for(i=0; i<user_init_len; i++) sendout(user_init_cmd[i]);
   }

   request_primary_timing();    // get time of day
   request_secondary_timing();
   request_all_dis_params();
}



void get_rcvr_message()
{
   // get bytes from serial port and build up a receiver message.  When message
   // is complete, parse it and to what it says.

   if(need_msg_init) {
      init_messages(27);
   }

   if     (rcvr_type == ACRON_RCVR)  get_acron_message();
   else if(rcvr_type == GPSD_RCVR)   get_gpsd_message();
   else if(rcvr_type == LUXOR_RCVR)  get_tsip_message();
   else if(rcvr_type == MOTO_RCVR)   get_moto_message();
   else if(rcvr_type == NMEA_RCVR)   get_nmea_message();
   else if(rcvr_type == NO_RCVR)     get_norcvr_message();
   else if(rcvr_type == NVS_RCVR)    get_nvs_message();    // NVS format is basically TSIP
   else if(rcvr_type == SCPI_RCVR)   get_scpi_message(); 
   else if(rcvr_type == UCCM_RCVR)   get_uccm_message();
   else if(rcvr_type == SIRF_RCVR)   get_sirf_message(); 
   else if(rcvr_type == STAR_RCVR)   get_star_message(); 
   else if(rcvr_type == TSIP_RCVR)   get_tsip_message();
   else if(rcvr_type == UBX_RCVR)    get_ubx_message();
   else if(rcvr_type == VENUS_RCVR)  get_venus_message();
   else if(rcvr_type == ZODIAC_RCVR) get_zod_message();
   else                              get_unkn_message();
}


void scpi_init(int type)
{
   // placeholder routine for possible special init stuff for SCPI devices
}


void wakey_wakey(int force_sirf)
{
   if(NO_SCPI_BREAK && (rcvr_type == SCPI_RCVR)) ; else 
   SendBreak();

   enable_gpsd();  // gggg

   scpi_init(0);

   send_byte(0x0D);        // for SCPI wakeup
   eom_flag = 1;
   send_byte(0x0A);
   Sleep(250);
//drain_com_data();
if(force_sirf) send_nmea_cmd("PSRF100,0,9600,8,1,0");  // switch SIRF to binary mode

   send_byte(0x0D);
   eom_flag = 1;
   send_byte(0x0A);
   Sleep(250);
// drain_com_data();


// send_tsip_start(0x8E);  // init TSIP receiver to send packets
// send_byte(0xA5);
// send_byte(0x00);
// send_byte(0x55);
// send_byte(0x00);
// send_byte(0x00);
// send_tsip_end();
   if(baud_rate == 115200) {  // wake up NVS_RCVR
      send_nvs_start(0x1F);   // 0x72 time message every second
      send_byte((u08) nav_rate);
      send_nvs_end();
   }

   send_moto_start("Bb");     // request Motorola sat visibility data
   send_byte(0);
   send_moto_end();

   send_moto_start("Wb");
   send_byte(1);              // for Jupiter-T: 0=MOTOROLA mode,  1=Jupiter-T mode
   send_moto_end();

   send_tsip_end();
   send_tsip_end();
   send_tsip_start(0x3C);     // request sat info
   send_byte(0x00);
   send_tsip_end();

   drain_com_data();          // drain serial port data buffer

   reset_com_timer();
}


int nvs_pkt(int id)
{
   // return true if id is a NVS receiver specific TSIP packet
   if     (id == 0x51) return 0;
   else if(id == 0x52) return 0;                 
   else if(id == 0x60) return 1;                 
   else if(id == 0x61) return 1;  //             
   else if(id == 0x72) return 1;  // every second
   else if(id == 0x73) return 0;                 
   else if(id == 0x74) return 0;                 
   else if(id == 0x88) return 1;  // every nav solution
   else if(id == 0x93) return 0;                 
   else if(id == 0xE7) return 0;                 
   else if(id == 0xF5) return 1;  // every second
   else if(id == 0xF7) return 0;                 

   return 0;
}


int auto_detect()
{
int i, j, k;
int crlf;
int lf;
int gpsd_flag;
int luxor_flag;
int moto_flag;
int nmea_flag;
int nvs_flag;
int scpi_flag;
int sirf_flag;
int star_flag;
int tsip_flag;
int ubx_flag;
int uccm_flag;
int venus_flag;
int zodiac_flag;
int count;
int type;
double old_timeout;
double start_time;
int c1,c2,c3;
int row, col;
char *s;
int speed_change;
int config_baud;
int c;
int long_pause;

   // This routine attemps to determin the type of receiver the serial
   // port is connected to.

   rcvr_type = TSIP_RCVR;  // if NO_RCVR, com port read routine always returns 0
   old_timeout = com_timeout;
   long_pause = 0;
   if(process_com == 0) {  // no com port open
      type = rcvr_type = (-1);   // this forces NO_RCVR mode with warning message
      long_pause = 1;
      goto no_com;
   }

   detect_rcvr_type = 1;
   detecting = 1;
   speed_change = 0;
   if(user_set_baud == 0) {
      baud_rate = 9600;
      data_bits = 8;
      parity = NO_PAR;
      init_com();
   }

   re_speed:
   old_timeout = com_timeout;

   j = sizeof(tsip_buf)-1;
   if(j > (512-1)) j = 512;
if((baud_rate == 115200) && (sizeof(tsip_buf) > 770)) j = 768;  // for better NVS detection


   com_timeout = 1500.0;   // if nothing received after 1500 msecs, abort
   reset_com_timer();
   tsip_rptr = tsip_wptr = 0;

   erase_screen();
   if     (parity == NO_PAR)   c = 'N';
   else if(parity == ODD_PAR)  c = 'O';
   else if(parity == EVEN_PAR) c = 'E';
   else                        c = '?';
   sprintf(out, "Detecting receiver type at %d:%d:%c:%d ...", baud_rate,data_bits,c,stop_bits);
   vidstr(0,0, GREEN, out);
   refresh_page();


   if(1) {  // send some commands to try to get receiver to send something
      wakey_wakey(0);
   }

   i = 0;
   row = 999;
   col = 999;

   start_time = this_msec = GetMsecs();
   while(i < j) {  // get a sample of the data stream
      while(SERIAL_DATA_AVAILABLE() == 0) {
         vidstr(1,1, YELLOW, "Press ESC to exit.");

         sprintf(out, "last:%.0f start:%.0f  this:%.0f  cto:%.0f", start_time, last_com_time, this_msec, com_timeout);
         vidstr(2,1,GREEN, out);

         refresh_page();
         this_msec = GetMsecs();
         if((this_msec - start_time) >= (com_timeout*2.0)) goto analyze;

         abort_wakeup();  // if ESC pressed, exit
      }

      this_msec = GetMsecs();
      if((this_msec - start_time) >= (com_timeout*2.0)) break;

      c1 = get_com_char();
      c1 &= 0xFF;
      tsip_buf[i++] = c1;
      tsip_buf[i] = 0;

      if(++col >= 32) {  // show data we are getting
         refresh_page();
         col = 0;
         if(++row > 32) row = 5;
      }
      sprintf(out, " %02X", c1&0xFF);
      vidstr(row,col*3,YELLOW, out);
   }

   // look at the data we got and see if we can find data that is 
   // semi-unique to the receiver types
   analyze:         
   reset_com_timer();
   refresh_page();

   crlf = 0;
   lf = 0;
   gpsd_flag = 0;
   luxor_flag = 0;
   moto_flag = 0;
   nmea_flag = 0;
   nvs_flag = 0;
   scpi_flag = 0;
   sirf_flag = 0;
   star_flag = 0;
   tsip_flag = 0;
   ubx_flag = 0;
   uccm_flag = 0;
   venus_flag = 0;
   zodiac_flag = 0;

   for(k=0; k<i; k++) {
      c1 = tsip_buf[k+0]; c1 &= 0xFF;
      c2 = tsip_buf[k+1]; c2 &= 0xFF; 
      c3 = tsip_buf[k+2]; c3 &= 0xFF; 

      if((c1 == '"')  && (c2 == ':'))  ++gpsd_flag;
      if((c1 == '$')  && (c2 == 'G'))  ++nmea_flag;
      if((c1 == '@')  && (c2 == '@'))  ++moto_flag;
      if((c1 == ETX)  && (c2 == DLE) && nvs_pkt(c3)) ++nvs_flag;
      if((c1 == ETX)  && (c2 == DLE) && (c3 == LUXOR_ID)) ++luxor_flag;
      if((c1 == DLE)  && (c2 == ETX) && (c3 == DLE))  ++tsip_flag;
//    if((c1 == DLE)  && (c2 == 0x8F)) ++tsip_flag;  // timing message group
      if((c1 == 0xB5) && (c2 == 0x62)) ++ubx_flag;
      if((c1 == 0xA0) && (c2 == 0xA2)) ++sirf_flag;
      if((c1 == 0xB0) && (c2 == 0xB3)) ++sirf_flag;
      if((c1 == 0xA0) && (c2 == 0xA1)) ++venus_flag;
      if((c1 == 0xFF) && (c2 == 0x81)) ++zodiac_flag;
      if((c1 == 0x0D) && (c2 == 0x0A)) ++crlf;
      if(c1 == 0x0A) ++lf;

      if(     (c1 == 'T')  && (c2 == '2') && (c3 == '2'))   ++scpi_flag;
      else if((c1 == 's')  && (c2 == 'c') && (c3 == 'p'))   ++scpi_flag;
      else if((c1 == 'E')  && (c2 == '-') && (isdigit(c3))) ++scpi_flag;
      else if((c1 == 'U')  && (c2 == 'C') && (c3 == 'C'))   ++uccm_flag;  // UCCM-P
      else if((c1 == 'C')  && (c2 == '5') && (c3 == ' '))   ++uccm_flag;  // UCCM-P
      else if((c1 == 'c')  && (c2 == '5') && (c3 == ' '))   ++uccm_flag;  // UCCM-P
      else if((c1 == ' ')  && (c2 == 'C') && (c3 == 'A'))   ++uccm_flag;  // UCCM-P
      else if((c1 == ' ')  && (c2 == 'c') && (c3 == 'a'))   ++uccm_flag;  // UCCM-P
      else if((c1 == 'U')  && (c2 == 'n') && (c3 == 'd'))   ++uccm_flag;  // UCCM-P
   }
   tsip_buf[128] = 0;
   if(strstr((char *) &tsip_buf[0], "SYNTAX_ERROR;")) ++star_flag;

   sirf_flag /= 2;  // these types have symmetrical indicators
   gpsd_flag /= 2;

   if(crlf == 0) {  // these type have messages that end in CR LF
      gpsd_flag = 0;
      moto_flag = 0;
      nmea_flag = 0;
//    scpi_flag = 0;
      uccm_flag = 0;
      venus_flag = 0;
      star_flag = 0;
   }

   if(uccm_flag && (tsip_flag < 2)) tsip_flag = 0;
   if(nvs_flag && (tsip_flag < 2)) nvs_flag = 0;
   if(baud_rate != 115200) nvs_flag = 0;

   count = 0;
   type = 0;
   // find the most commonly seen receiver pattern match count
   if(gpsd_flag > count)   { type = GPSD_RCVR;  count = gpsd_flag;  }
   if(moto_flag > count)   { type = MOTO_RCVR;  count = moto_flag;  }
   if(nmea_flag > count)   { type = NMEA_RCVR;  count = nmea_flag;  }
   if(scpi_flag > count)   { type = SCPI_RCVR;  count = scpi_flag;  }
   if(tsip_flag > count)   { type = TSIP_RCVR;  count = tsip_flag;  }
   if(ubx_flag  > count)   { type = UBX_RCVR;   count = ubx_flag;   }
   if(uccm_flag > count)   { type = UCCM_RCVR;  count = uccm_flag;  }
   if(sirf_flag > count)   { type = SIRF_RCVR;  count = sirf_flag;  }
   if(star_flag > count)   { type = STAR_RCVR;  count = star_flag;  }
   if(venus_flag > count)  { type = VENUS_RCVR; count = venus_flag; }
   if(zodiac_flag > count) { type = ZODIAC_RCVR; count = zodiac_flag; }


   if(count == 0) {  // no recognizeable receiver type, assume NO_RCVR
      if((user_set_baud == 0) && (speed_change < 3)) {  // try different serial port config
         ++speed_change;

         data_bits = 8;
         parity = NO_PAR;
         stop_bits = 1;

         if(speed_change == 1) {
            baud_rate = 115200;
         }
         else if(speed_change == 2) {
            baud_rate = 57600;
         }
         else if(speed_change == 3) {
            baud_rate = 19200;
            data_bits = 7;
            parity = ODD_PAR;
         }
         init_com();
// BEEP(206);
         goto re_speed;
      }
      type = NO_RCVR;
   }

   no_com:
   config_baud = 0;
   if((type == TSIP_RCVR) && nvs_flag) {
      type = NVS_RCVR;
      config_baud = 1;  // we need to do ODD parity
      detect_rcvr_type = 0;  // so config_rcvr_type will set baud
   }
   else if((type == TSIP_RCVR) && (luxor_flag > 1) && (baud_rate == 9600)) {
      type = LUXOR_RCVR;
      luxor = 11;
      config_baud = 1;  // we need to do ODD parity
      parity = ODD_PAR;
      detect_rcvr_type = 0;  // so config_rcvr_type will set baud
   }

   rcvr_type = type;
   config_rcvr_type(config_baud);
   if(config_baud) detect_rcvr_type = 1;

//sprintf(plot_title, "rcvr:%d  max:%d  tsip:%d  nmea:%d  gpsd:%d  moto:%d  ubx:%d  sirf:%d  scpi:%d  zod:%d  lost:%d", 
//type,count, tsip_flag,nmea_flag,gpsd_flag, moto_flag,ubx_flag,sirf_flag, scpi_flag, zodiac_flag, com_data_lost);
//Sleep(3000);

   com_timeout = old_timeout;
   reset_com_timer();
   erase_screen();

   if     (rcvr_type == ACRON_RCVR)  s = "Acron receiver";
   else if(rcvr_type == GPSD_RCVR)   s = "GPSD receiver";
   else if(rcvr_type == LUXOR_RCVR)  s = "Luxor analyzer";
   else if(rcvr_type == NMEA_RCVR)   s = "NMEA receiver";
   else if(rcvr_type == NO_RCVR)     s = "No receiver";
   else if(rcvr_type == NVS_RCVR)    s = "NVS receiver";
   else if(rcvr_type == MOTO_RCVR)   s = "Motorola receiver";
   else if(rcvr_type == SCPI_RCVR)   s = "SCPI receiver";
   else if(rcvr_type == SIRF_RCVR)   s = "Sirf receiver";
   else if(rcvr_type == STAR_RCVR) {
      if(baud_rate == 115200) {
         s = "NEC receiver";
         star_type = NEC_TYPE;
      }
      else {
         s = "STAR-4 receiver";
         star_type = OSCILLO_TYPE;
      }
   }
   else if(rcvr_type == TSIP_RCVR)   s = "TSIP receiver";
   else if(rcvr_type == UBX_RCVR)    s = "Ublox receiver";
   else if(rcvr_type == UCCM_RCVR)   s = "UCCM receiver";
   else if(rcvr_type == VENUS_RCVR)  s = "Venus receiver";
   else if(rcvr_type == ZODIAC_RCVR) s = "Jupiter receiver";
   else if(rcvr_type == (-1)) {
      s = "No COM port open";
      goto bad_port;
   }
   else {
      sprintf(out, "Unknown type:%d", type);
      s = &out[0];

      bad_port:
      rcvr_type = NO_RCVR;
      config_rcvr_type(config_baud);
      config_msg_ofs();
   }

   vchar_string(0,0,CYAN, s);
   refresh_page();
   if(long_pause) Sleep(2000);  // no com port, delay longer so it's visible
   else           Sleep(1000);

   erase_screen();
   refresh_page();
   detecting = 0;
   return rcvr_type;
}

