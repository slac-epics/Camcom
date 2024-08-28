/*

Abs: CAMCOM data structures on Linux

Name: camcom_data.h

Auth: 26-Jan-2010  R. Chestnut

*/

#ifndef camcom_data_h

extern short current_packet;


#define CTLW_F16      0x00100000
#define CTLW_F8       0x00080000
#define CTLW_F4       0x00040000
#define CTLW_F2       0x00020000
#define CTLW_F1       0x00010000
#define CTLW_C8       0x00008000
#define CTLW_C4       0x00004000
#define CTLW_C2       0x00002000
#define CTLW_C1       0x00001000
#define CTLW_M16      0x00000800
#define CTLW_M8       0x00000400
#define CTLW_M4       0x00000200
#define CTLW_M2       0x00000100
#define CTLW_M1       0x00000080
#define CTLW_SPARE    0x00000040
#define CTLW_P8       0x00000020
#define CTLW_RE_PACK  0x00000010
#define CTLW_A8       0x00000008
#define CTLW_A4       0x00000004
#define CTLW_A2       0x00000002
#define CTLW_A1       0x00000001

#define CTLW_CRATE_MASK                CTLW_C8 | CTLW_C4 | CTLW_C2 | CTLW_C1
#define CTLW_MODULE_MASK    CTLW_M16 | CTLW_M8 | CTLW_M4 | CTLW_M2 | CTLW_M1
#define CTLW_SUBADDR_MASK              CTLW_A8 | CTLW_A4 | CTLW_A2 | CTLW_A1
#define CTLW_CRATE_SHIFT    -12
#define CTLW_MODULE_SHIFT   -7
#define CTLW_FUNCTION_SHIFT -16
#define CTLW_FUNCTION_MASK CTLW_F16 | CTLW_F8 | CTLW_F4 | CTLW_F2 | CTLW_F1             
#define CTLW_P24      0x04000000
#define CTLW_XM1      0x40000000
#define CTLW_XM2      0x20000000
#define CTLW_QM1      0x10000000
#define CTLW_QM2      0x08000000
#define CTLW_ILQ      0x01000000
#define CTLW_SC       0x00800000
#define CTLW_SM       0x00400000
#define CTLW_SA       0x00200000
#define STAT_X        0x00020000
#define STAT_Q        0x00010000

/*
 * ---------------------------------------------------------------
 * camac control block structure
 *     - one of these is at the start of every camac package
 *     - note that the timing job uses spare2 and spare3.
 */

/* CNTBLK_LEN is 8 shorts */
typedef struct
{
    unsigned short key;         /* the literal "FR" */
    unsigned short nops;        /* max number of packets to follow */
    unsigned short iop;         /* actual number of packets */
    unsigned short tbytes;      /* approx. max bytecount for this package */
    unsigned short spare1;
    unsigned short bit_summary; /* either P8 or RE_PACK, see ctlw? */
    unsigned short spare2;
    unsigned short spare3;
} camac_ctlb_ts;


/*
 * ---------------------------------------------------------------
 * camac status/data area layout
 *     - used to send and receive data from camac I/O
 *     - used to receive I/O completion status from camac operation
 *     - the stat field is set using the above STAT_* bits.
 *     - that data field can be arbitrarily long (in bytes).
 */
typedef struct
{
    unsigned int  stat; 
    unsigned short  data[40];
} camac_statdata_ts;

/*
 * ---------------------------------------------------------------
 *     - the measurement task needs one of these with a 4-byte field - 
 *       for easier looking at in the debugger
 */
 
typedef struct
{
    unsigned int  stat;
    unsigned int  data;
} camac_longdata_ts;


/*
 * ---------------------------------------------------------------
 * camac packet structure
 *     - an arbitrary number of these may be present in a camac package
 *     - the ctlw field is set using the above CTLW_* bits.
 */
/* MBCD_LEN is 6 shorts */
typedef struct
{
  unsigned int                ctlw;
  /*
    camac_statdata_ts *statdata;
  */
  int                         statdata;
  unsigned short              wc_max;    /* max number of words in this pkt */
  unsigned short              cic;       /* completion interrupt code, must be 0 */
} camac_pkt_ts;

/*
 * ---------------------------------------------------------------
 * camac package structure as sent to CAMGO
 *    - only one control_block per package.
 *    - as many packets as camac operations to be submitted.
 *    - as many emasks as there are packets, following the last packet
 *      in the package, but that's not shown here because I didn't want
 *      to hardware in a number of packets after which the emasks came.
 */
typedef struct
{
    camac_ctlb_ts  control_block;
    camac_pkt_ts   packet[2];
    unsigned int emask;
} camac_pkg_ts;

#define CAMBLK_P ((camac_pkg_ts *) *camblk_tok_p)

#define NOPS 10

#define DATA_START_OFFSET 256

#define STAT_DATA_LENGTH 128

#ifdef CAMCOM_DATA_DEFS
#define CAMCOM_EXTERN
#else
#define CAMCOM_EXTERN extern
#endif

CAMCOM_EXTERN short out_buffer_len;

CAMCOM_EXTERN int packet_data[NOPS][128];

CAMCOM_EXTERN short packet_wcmax[NOPS];

CAMCOM_EXTERN short packet_read[NOPS];

CAMCOM_EXTERN short packet_write[NOPS];

CAMCOM_EXTERN short packet_pack8[NOPS];

CAMCOM_EXTERN short packet_32bitdata[NOPS];

CAMCOM_EXTERN short packet_bcnt[NOPS];

typedef union {__attribute__((packed)) int l; short w[2];} ctlw_t;

CAMCOM_EXTERN ctlw_t packet_ctlw[NOPS];

CAMCOM_EXTERN camac_pkg_ts *camblk_tok_p;

CAMCOM_EXTERN camac_statdata_ts *camac_statdata_p;

CAMCOM_EXTERN int calling_param_values[39];

CAMCOM_EXTERN char* calling_param_string[39];

#undef CAMCOM_EXTERN

#define camcom_data_h
#endif
