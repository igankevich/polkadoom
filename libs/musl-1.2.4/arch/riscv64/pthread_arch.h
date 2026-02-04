uintptr_t __get_tp();

// We use dummy TLS.
//#define TLS_ABOVE_TP

#define GAP_ABOVE_TP 0

#define DTP_OFFSET 0x800

#define MC_PC __gregs[0]
