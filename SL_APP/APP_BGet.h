#ifndef __APP_BGET_H
#define __APP_BGET_H
#include "APP_Include.h"
typedef long bufsize;
/* Buffer allocation size quantum:
	 all buffers allocated are a
	 multiple of this size.  This
	 MUST be a power of two. */
#define SizeQuant   4              
/* Define this symbol to enable the
	 bpoold() function which dumps the
	 buffers in a buffer pool. */
#define BufDump     1              
#undef BufDump
/* Define this symbol to enable the
	 bpoolv() function for validating
	 a buffer pool. */
#define BufValid    1              
#undef BufValid
/* Define this symbol to enable the
	 bufdump() function which allows
	 dumping the contents of an allocated
	 or free buffer. */
#define DumpData    1              
#undef DumpData
/* Define this symbol to enable the
	 bstats() function which calculates
	 the total free space in the buffer
	 pool, the largest available
	 buffer, and the total space
	 currently allocated. */
#define BufStats    1              
/* Wipe free buffers to a guaranteed
	 pattern of garbage to trip up
	 miscreants who attempt to use
	 pointers into released buffers. */
#define FreeWipe    1              
#undef FreeWipe
/* Use a best fit algorithm when
	 searching for space for an
	 allocation request.  This uses
	 memory more efficiently, but
	 allocation will be much slower. */
#define BestFit     1              
/* Define this symbol to enable the
	 bectl() function for automatic
	 pool space control.  */
#define BECtl       1              
#undef BECtl
typedef enum
{
	MEM_0 = 0x00,
	MEM_N,
}APP_BGet_Name_e;
typedef struct
{
	APP_BGet_Name_e MemName;
	uint8_t *MemAddress;
	uint32_t Size;
#ifdef BufStats
  bufsize totalloc;  /* Total space currently allocated */
	long numget;
	long numrel;   /* Number of bget() and brel() calls */
#ifdef BECtl
	long numpblk;  /* Number of pool blocks */
	long numpget;
	long numprel;  /* Number of block gets and rels */
	long numdget;
	long numdrel;  /* Number of direct gets and rels */
#endif /* BECtl */
#endif /* BufStats */
#ifdef BECtl
/* Automatic expansion block management functions */
	int (*compfcn)(bufsize sizereq, int sequence);
	void *(*acqfcn)(bufsize size);
	void (*relfcn)(void *buf);
	bufsize exp_incr;          /* Expansion block size */
	bufsize pool_len;          /* 0: no bpool calls have been made
                     -1: not all pool blocks are
                         the same size
                     >0: (common) block size for all
                         bpool calls made so far
                      */
#endif
}APP_BGet_Control_t;
extern void APP_BGet_Init(void);
extern bufsize APP_BGet_TotalSize(APP_BGet_Name_e mem_name);
extern bufsize APP_BGet_AllocSize(APP_BGet_Name_e mem_name);

extern void *APP_BGet(APP_BGet_Name_e mem_name,bufsize requested_size);
extern void *APP_BGetz(APP_BGet_Name_e mem_name,bufsize size);
extern void *APP_BGetr(APP_BGet_Name_e mem_name,void *buf,bufsize size);
extern void APP_BRel(APP_BGet_Name_e mem_name,void *buf);


extern void APP_BEctl(int (*compact)(bufsize sizereq, int sequence),
		       void *(*acquire)(bufsize size),
		       void (*release)(void *buf), bufsize pool_incr);
extern void APP_Bufdump(void *buf);
extern void APP_BPoold(void *pool, int dumpalloc, int dumpfree);
extern int	APP_BPoolv(void *pool);


#endif
