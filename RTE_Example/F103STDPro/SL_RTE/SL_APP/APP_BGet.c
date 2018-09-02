#include "APP_BGet.h"
 /* Type for size arguments to memxxx()functions such as memcmp(). */
#define MemSize     int  
/* Queue links */
struct qlinks {
    struct bfhead *flink;          /* Forward link */
    struct bfhead *blink;          /* Backward link */
};
/* Header in allocated and free buffers */
struct bhead {
    bufsize prevfree;              /* Relative link back to previous free buffer in memory or 0 if previous buffer is allocated.    */
    bufsize bsize;              /* Buffer size: positive if free,  negative if allocated. */
};
#define BH(p)    ((struct bhead *) (p))
/*  Header in directly allocated buffers (by acqfcn) */
struct bdhead {
    bufsize tsize;              /* Total size, including overhead */
    struct bhead bh;              /* Common header */
};
#define BDH(p)    ((struct bdhead *) (p))
/* Header in free buffers */
struct bfhead {
    struct bhead bh;              /* Common allocated/free header */
    struct qlinks ql;              /* Links on free list */
};
#define BFH(p)    ((struct bfhead *) (p))
/* End sentinel: value placed in bsize field of dummy block delimiting
 end of pool block.  The most negative number which will  fit  in  a
 bufsize, defined in a way that the compiler will accept. */
#define ESent    ((bufsize) (-(((1L << (sizeof(bufsize) * 8 - 2)) - 1) * 2) - 2))
/*  Minimum allocation quantum: */
#define QLSize    (sizeof(struct qlinks))
#define SizeQ    ((SizeQuant > QLSize) ? SizeQuant : QLSize)
#define V   (void)              /* To denote unwanted returned values */
	
//内存池(4字节对齐)
__align(4) uint8_t RAM_0[MEM_BASE_SIZE];//内部SRAM内存池
//__align(4) uint8_t APP_EXMEM_BaseAddress[MEMEX_MAX_SIZE] __attribute__((at(0X68000000)));					//外部SRAM内存池
static APP_BGet_Control_t APP_BGet_ControlHandle[MEM_N] = 
{
	{MEM_0,RAM_0,MEM_BASE_SIZE}
};
static struct bfhead freelist[MEM_N] = {     /* List of free buffers */
	{
		{0, 0},
		{&freelist[MEM_0], &freelist[MEM_0]}
	}
};
/*  BPOOL  --  Add a region of memory to the buffer pool.  */
void bpool(APP_BGet_Name_e mem_name,void *buf, bufsize len)
{
	struct bfhead *b = BFH(buf);
	struct bhead *bn;
#ifdef SizeQuant
  len &= ~(SizeQuant - 1);
#endif
#ifdef BECtl
	if (APP_BGet_ControlHandle[mem_name].pool_len == 0) {
		APP_BGet_ControlHandle[mem_name].pool_len = len;
	} else if (len != APP_BGet_ControlHandle[mem_name].pool_len) {
		APP_BGet_ControlHandle[mem_name].pool_len = -1;
	}
#ifdef BufStats
	APP_BGet_ControlHandle[mem_name].numpget++;                  /* Number of block acquisitions */
	APP_BGet_ControlHandle[mem_name].numpblk++;                  /* Number of blocks total */
	APP_AssertParam(APP_BGet_ControlHandle[mem_name].numpblk == 
		APP_BGet_ControlHandle[mem_name].numpget - APP_BGet_ControlHandle[mem_name].numprel);
#endif /* BufStats */
#endif /* BECtl */
	/* Since the block is initially occupied by a single free  buffer,
		 it  had    better    not  be  (much) larger than the largest buffer
		 whose size we can store in bhead.bsize. */
	APP_AssertParam(len - sizeof(struct bhead) <= -((bufsize) ESent + 1));
	/* Clear  the  backpointer at  the start of the block to indicate that
		 there  is  no  free  block  prior  to  this   one.    That   blocks
		 recombination when the first block in memory is released. */
  b->bh.prevfree = 0;
	/* Chain the new block to the free list. */
	APP_AssertParam(freelist[mem_name].ql.blink->ql.flink == &freelist[mem_name]);
	APP_AssertParam(freelist[mem_name].ql.flink->ql.blink == &freelist[mem_name]);
	b->ql.flink = &freelist[mem_name];
	b->ql.blink = freelist[mem_name].ql.blink;
	freelist[mem_name].ql.blink = b;
	b->ql.blink->ql.flink = b;
	/* Create a dummy allocated buffer at the end of the pool.    This dummy
		 buffer is seen when a buffer at the end of the pool is released and
		 blocks  recombination  of  the last buffer with the dummy buffer at
		 the end.  The length in the dummy buffer  is  set  to  the  largest
		 negative  number  to  denote  the  end  of  the pool for diagnostic
		 routines (this specific value is  not  counted  on  by  the  actual
		 allocation and release functions). */
	len -= sizeof(struct bhead);
	b->bh.bsize = (bufsize) len;
#ifdef FreeWipe
	V memset(((char *) b) + sizeof(struct bfhead), 0x55,
			 (MemSize) (len - sizeof(struct bfhead)));
#endif
	bn = BH(((char *) b) + len);
	bn->prevfree = (bufsize) len;
	/* Definition of ESent assumes two's complement! */
	APP_AssertParam((~0) == -1);
	bn->bsize = ESent;
}
/*  BGET  --  Allocate a buffer.  */
void *APP_BGet(APP_BGet_Name_e mem_name,bufsize requested_size)
{
	bufsize size = requested_size;
	struct bfhead *b;
#ifdef BestFit
	struct bfhead *best;
#endif
	void *buf;
#ifdef BECtl
	int compactseq = 0;
#endif
	APP_AssertParam(size > 0);

	if (size < SizeQ) {           /* Need at least room for the */
	size = SizeQ;              /*    queue links.  */
	}
#ifdef SizeQuant
	#if SizeQuant > 1
	size = (size + (SizeQuant - 1)) & (~(SizeQuant - 1));
	#endif
#endif
  size += sizeof(struct bhead);     /* Add overhead in allocated buffer to size required. */
#ifdef BECtl
	/* If a compact function was provided in the call to bectl(), wrap
		 a loop around the allocation process  to  allow    compaction  to
		 intervene in case we don't find a suitable buffer in the chain. */

	while (1) {
#endif
	b = freelist[mem_name].ql.flink;
#ifdef BestFit
	best = &freelist[mem_name];
#endif
	/* Scan the free list searching for the first buffer big enough
		 to hold the requested size buffer. */
#ifdef BestFit
	while (b != &freelist[mem_name]) {
		if (b->bh.bsize >= size) {
			if ((best == &freelist[mem_name]) || (b->bh.bsize < best->bh.bsize)) {
					best = b;
			}
		}
		b = b->ql.flink;          /* Link to next buffer */
	}
	b = best;
#endif /* BestFit */
	while (b != &freelist[mem_name]) {
		if ((bufsize) b->bh.bsize >= size) {
			/* Buffer  is big enough to satisfy  the request.  Allocate it
				 to the caller.  We must decide whether the buffer is  large
				 enough  to  split  into  the part given to the caller and a
				 free buffer that remains on the free list, or  whether  the
				 entire  buffer  should  be  removed    from the free list and
				 given to the caller in its entirety.   We  only  split  the
				 buffer if enough room remains for a header plus the minimum
				 quantum of allocation. */
			if ((b->bh.bsize - size) > (SizeQ + (sizeof(struct bhead)))) {
				struct bhead *ba, *bn;
				ba = BH(((char *) b) + (b->bh.bsize - size));
				bn = BH(((char *) ba) + size);
				APP_AssertParam(bn->prevfree == b->bh.bsize);
				/* Subtract size from length of free block. */
				b->bh.bsize -= size;
				/* Link allocated buffer to the previous free buffer. */
				ba->prevfree = b->bh.bsize;
				/* Plug negative size into user buffer. */
				ba->bsize = -(bufsize) size;
				/* Mark buffer after this one not preceded by free block. */
				bn->prevfree = 0;
#ifdef BufStats
				APP_BGet_ControlHandle[mem_name].totalloc += size;
				APP_BGet_ControlHandle[mem_name].numget++;          /* Increment number of bget() calls */
#endif
				buf = (void *) ((((char *) ba) + sizeof(struct bhead)));
				return buf;
			} else {
				struct bhead *ba;
				ba = BH(((char *) b) + b->bh.bsize);
				APP_AssertParam(ba->prevfree == b->bh.bsize);
				/* The buffer isn't big enough to split.  Give  the  whole
					 shebang to the caller and remove it from the free list. */
				APP_AssertParam(b->ql.blink->ql.flink == b);
				APP_AssertParam(b->ql.flink->ql.blink == b);
				b->ql.blink->ql.flink = b->ql.flink;
				b->ql.flink->ql.blink = b->ql.blink;
#ifdef BufStats
				APP_BGet_ControlHandle[mem_name].totalloc += b->bh.bsize;
				APP_BGet_ControlHandle[mem_name].numget++;          /* Increment number of bget() calls */
#endif
				/* Negate size to mark buffer allocated. */
				b->bh.bsize = -(b->bh.bsize);

				/* Zero the back pointer in the next buffer in memory
					 to indicate that this buffer is allocated. */
				ba->prevfree = 0;

				/* Give user buffer starting at queue links. */
				buf =  (void *) &(b->ql);
				return buf;
			}
		}
		b = b->ql.flink;          /* Link to next buffer */
	}
#ifdef BECtl
		/* We failed to find a buffer.  If there's a compact  function
	 defined,  notify  it  of the size requested.  If it returns
	 TRUE, try the allocation again. */
    if ((APP_BGet_ControlHandle[mem_name].compfcn == NULL) || (!(*APP_BGet_ControlHandle[mem_name].compfcn)(size, ++compactseq))) {
        break;
    }
    }
    /* No buffer available with requested size free. */
    /* Don't give up yet -- look in the reserve supply. */
    if (APP_BGet_ControlHandle[mem_name].acqfcn != NULL) {
    if (size > APP_BGet_ControlHandle[mem_name].exp_incr - sizeof(struct bhead)) {

        /* Request    is  too  large    to  fit in a single expansion
           block.  Try to satisy it by a direct buffer acquisition. */

        struct bdhead *bdh;
        size += sizeof(struct bdhead) - sizeof(struct bhead);
        if ((bdh = BDH((*APP_BGet_ControlHandle[mem_name].acqfcn)((bufsize) size))) != NULL) {

					/*  Mark the buffer special by setting the size field
							of its header to zero.  */
					bdh->bh.bsize = 0;
					bdh->bh.prevfree = 0;
					bdh->tsize = size;
#ifdef BufStats
					APP_BGet_ControlHandle[mem_name].totalloc += size;
					APP_BGet_ControlHandle[mem_name].numget++;          /* Increment number of bget() calls */
					APP_BGet_ControlHandle[mem_name].numdget++;          /* Direct bget() call count */
#endif
					buf =  (void *) (bdh + 1);
					return buf;
        }
			} else {
				/*    Try to obtain a new expansion block */
				void *newpool;
				if ((newpool = (*APP_BGet_ControlHandle[mem_name].acqfcn)((bufsize) APP_BGet_ControlHandle[mem_name].exp_incr)) != NULL) {
					bpool(mem_name,newpool, APP_BGet_ControlHandle[mem_name].exp_incr);
					buf =  bget(mem_name,requested_size);  /* This can't, I say, can't
									 get into a loop. */
					return buf;
				}
			}
    }
    /*    Still no buffer available */
#endif /* BECtl */
	return NULL;
}
/*  BGETZ  --  Allocate a buffer and clear its contents to zero.  We clear
           the  entire  contents  of  the buffer to zero, not just the
           region requested by the caller. */
void *APP_BGetz(APP_BGet_Name_e mem_name,bufsize size)
{
	char *buf = (char *) APP_BGet(mem_name,size);
	if (buf != NULL) {
		struct bhead *b;
		bufsize rsize;
		b = BH(buf - sizeof(struct bhead));
		rsize = -(b->bsize);
		if (rsize == 0) {
			struct bdhead *bd;

			bd = BDH(buf - sizeof(struct bdhead));
			rsize = bd->tsize - sizeof(struct bdhead);
		} else {
			rsize -= sizeof(struct bhead);
		}
		APP_AssertParam(rsize >= size);
		V memset(buf, 0, (MemSize) rsize);
	}
	return ((void *) buf);
}
/*  BREL  --  Release a buffer.  */
void APP_BRel(APP_BGet_Name_e mem_name,void *buf)
{
	struct bfhead *b, *bn;

	b = BFH(((char *) buf) - sizeof(struct bhead));
#ifdef BufStats
	APP_BGet_ControlHandle[mem_name].numrel++;                  /* Increment number of brel() calls */
#endif
	APP_AssertParam(buf != NULL);
#ifdef BECtl
	if (b->bh.bsize == 0) {          /* Directly-acquired buffer? */
		struct bdhead *bdh;

		bdh = BDH(((char *) buf) - sizeof(struct bdhead));
		APP_AssertParam(b->bh.prevfree == 0);
#ifdef BufStats
		APP_BGet_ControlHandle[mem_name].totalloc -= bdh->tsize;
		APP_AssertParam(APP_BGet_ControlHandle[mem_name].totalloc >= 0);
		APP_BGet_ControlHandle[mem_name].numdrel++;              /* Number of direct releases */
#endif /* BufStats */
#ifdef FreeWipe
		V memset((char *) buf, 0x55,
				 (MemSize) (bdh->tsize - sizeof(struct bdhead)));
#endif /* FreeWipe */
		APP_AssertParam(APP_BGet_ControlHandle[mem_name].relfcn != NULL);
		(*APP_BGet_ControlHandle[mem_name].relfcn)((void *) bdh);      /* Release it directly. */
		return;
	}
#endif /* BECtl */
	/* Buffer size must be negative, indicating that the buffer is
		 allocated. */
	if (b->bh.bsize >= 0) {
		bn = NULL;
	}
	APP_AssertParam(b->bh.bsize < 0);
	/*    Back pointer in next buffer must be zero, indicating the
	same thing: */
	APP_AssertParam(BH((char *) b - b->bh.bsize)->prevfree == 0);
#ifdef BufStats
	APP_BGet_ControlHandle[mem_name].totalloc += b->bh.bsize;
	APP_AssertParam(APP_BGet_ControlHandle[mem_name].totalloc >= 0);
#endif
	/* If the back link is nonzero, the previous buffer is free.  */
	if (b->bh.prevfree != 0) {
    /* The previous buffer is free.  Consolidate this buffer  with    it
       by  adding  the  length  of    this  buffer  to the previous free
       buffer.  Note that we subtract the size  in    the  buffer  being
           released,  since  it's  negative to indicate that the buffer is
       allocated. */
    register bufsize size = b->bh.bsize;
        /* Make the previous buffer the one we're working on. */
    APP_AssertParam(BH((char *) b - b->bh.prevfree)->bsize == b->bh.prevfree);
    b = BFH(((char *) b) - b->bh.prevfree);
    b->bh.bsize -= size;
	} 
	else {
		/* The previous buffer isn't allocated.  Insert this buffer
	 on the free list as an isolated free block. */
    APP_AssertParam(freelist[mem_name].ql.blink->ql.flink == &freelist[mem_name]);
    APP_AssertParam(freelist[mem_name].ql.flink->ql.blink == &freelist[mem_name]);
    b->ql.flink = &freelist[mem_name];
    b->ql.blink = freelist[mem_name].ql.blink;
    freelist[mem_name].ql.blink = b;
    b->ql.blink->ql.flink = b;
    b->bh.bsize = -b->bh.bsize;
	}
	/* Now we look at the next buffer in memory, located by advancing from
		 the  start  of  this  buffer  by its size, to see if that buffer is
		 free.  If it is, we combine  this  buffer  with    the  next  one    in
		 memory, dechaining the second buffer from the free list. */
	bn =  BFH(((char *) b) + b->bh.bsize);
	if (bn->bh.bsize > 0) {
    /* The buffer is free.    Remove it from the free list and add
       its size to that of our buffer. */
    APP_AssertParam(BH((char *) bn + bn->bh.bsize)->prevfree == bn->bh.bsize);
    APP_AssertParam(bn->ql.blink->ql.flink == bn);
    APP_AssertParam(bn->ql.flink->ql.blink == bn);
    bn->ql.blink->ql.flink = bn->ql.flink;
    bn->ql.flink->ql.blink = bn->ql.blink;
    b->bh.bsize += bn->bh.bsize;
    /* Finally,  advance  to   the    buffer    that   follows    the  newly
       consolidated free block.  We must set its  backpointer  to  the
       head  of  the  consolidated free block.  We know the next block
       must be an allocated block because the process of recombination
       guarantees  that  two  free    blocks will never be contiguous in
       memory.  */
    bn = BFH(((char *) b) + b->bh.bsize);
	}
#ifdef FreeWipe
	V memset(((char *) b) + sizeof(struct bfhead), 0x55,
			(MemSize) (b->bh.bsize - sizeof(struct bfhead)));
#endif
	APP_AssertParam(bn->bh.bsize < 0);
	/* The next buffer is allocated.  Set the backpointer in it  to  point
		 to this buffer; the previous free buffer in memory. */
	bn->bh.prevfree = b->bh.bsize;
#ifdef BECtl
	/*    If  a  block-release function is defined, and this free buffer
	constitutes the entire block, release it.  Note that  pool_len
	is  defined  in  such a way that the test will fail unless all
	pool blocks are the same size.    */

	if (APP_BGet_ControlHandle[mem_name].relfcn != NULL &&
	((bufsize) b->bh.bsize) == (APP_BGet_ControlHandle[mem_name].pool_len - sizeof(struct bhead))) {
		APP_AssertParam(b->bh.prevfree == 0);
		APP_AssertParam(BH((char *) b + b->bh.bsize)->bsize == ESent);
		APP_AssertParam(BH((char *) b + b->bh.bsize)->prevfree == b->bh.bsize);
		/*  Unlink the buffer from the free list  */
		b->ql.blink->ql.flink = b->ql.flink;
		b->ql.flink->ql.blink = b->ql.blink;

		(*APP_BGet_ControlHandle[mem_name].relfcn)(b);
#ifdef BufStats
		APP_BGet_ControlHandle[mem_name].numprel++;              /* Nr of expansion block releases */
		APP_BGet_ControlHandle[mem_name].numpblk--;              /* Total number of blocks */
		APP_AssertParam(APP_BGet_ControlHandle[mem_name].numpblk == APP_BGet_ControlHandle[mem_name].numpget - APP_BGet_ControlHandle[mem_name].numprel);
#endif /* BufStats */
	}
#endif /* BECtl */
}
/*  BGETR  --  Reallocate a buffer.  This is a minimal implementation,
           simply in terms of brel()  and  bget().     It  could  be
           enhanced to allow the buffer to grow into adjacent free
           blocks and to avoid moving data unnecessarily.  */
void *APP_BGetr(APP_BGet_Name_e mem_name,void *buf,bufsize size)
{
	void *nbuf;
	bufsize osize;              /* Old size of buffer */
	struct bhead *b;

	if ((nbuf = APP_BGet(mem_name,size)) == NULL) { /* Acquire new buffer */
		return NULL;
	}
	if (buf == NULL) {
		return nbuf;
	}
	b = BH(((char *) buf) - sizeof(struct bhead));
	osize = -b->bsize;
#ifdef BECtl
	if (osize == 0) {
	/*  Buffer acquired directly through acqfcn. */
	struct bdhead *bd;

	bd = BDH(((char *) buf) - sizeof(struct bdhead));
	osize = bd->tsize - sizeof(struct bdhead);
	} else
#endif
	osize -= sizeof(struct bhead);
	APP_AssertParam(osize > 0);
	V memcpy((char *) nbuf, (char *) buf, /* Copy the data */
			 (MemSize) ((size < osize) ? size : osize));
	APP_BRel(mem_name,buf);
	return nbuf;
}
#ifdef BECtl
/*  BECTL  --  Establish automatic pool expansion control  */
void bectl(
  int (*compact) _((bufsize sizereq, int sequence)),
  void *(*acquire) _((bufsize size)),
  void (*release) _((void *buf)),
  bufsize pool_incr)
{
    compfcn = compact;
    acqfcn = acquire;
    relfcn = release;
    exp_incr = pool_incr;
}
#endif
#ifdef DumpData

/*  BUFDUMP  --  Dump the data in a buffer.  This is called with the  user
         data pointer, and backs up to the buffer header.  It will
         dump either a free block or an allocated one.    */

void bufdump(
  void *buf)
{
    struct bfhead *b;
    unsigned char *bdump;
    bufsize bdlen;

    b = BFH(((char *) buf) - sizeof(struct bhead));
    ASSERT(b->bh.bsize != 0);
    if (b->bh.bsize < 0) {
    bdump = (unsigned char *) buf;
    bdlen = (-b->bh.bsize) - sizeof(struct bhead);
    } else {
    bdump = (unsigned char *) (((char *) b) + sizeof(struct bfhead));
    bdlen = b->bh.bsize - sizeof(struct bfhead);
    }

    while (bdlen > 0) {
    int i, dupes = 0;
    bufsize l = bdlen;
    char bhex[50], bascii[20];

    if (l > 16) {
        l = 16;
    }

    for (i = 0; i < l; i++) {
            V sprintf(bhex + i * 3, "%02X ", bdump[i]);
            bascii[i] = isprint(bdump[i]) ? bdump[i] : ' ';
    }
    bascii[i] = 0;
        V printf("%-48s   %s\n", bhex, bascii);
    bdump += l;
    bdlen -= l;
    while ((bdlen > 16) && (memcmp((char *) (bdump - 16),
                       (char *) bdump, 16) == 0)) {
        dupes++;
        bdump += 16;
        bdlen -= 16;
    }
    if (dupes > 1) {
        V printf(
                "     (%d lines [%d bytes] identical to above line skipped)\n",
        dupes, dupes * 16);
    } else if (dupes == 1) {
        bdump -= 16;
        bdlen += 16;
    }
    }
}
#endif

#ifdef BufDump

/*  BPOOLD  --    Dump a buffer pool.  The buffer headers are always listed.
        If DUMPALLOC is nonzero, the contents of allocated buffers
        are  dumped.   If  DUMPFREE  is  nonzero,  free blocks are
        dumped as well.  If FreeWipe  checking    is  enabled,  free
        blocks    which  have  been clobbered will always be dumped. */

void bpoold(
  void *buf,
  int dumpalloc, int dumpfree)
{
    struct bfhead *b = BFH(buf);

    while (b->bh.bsize != ESent) {
    bufsize bs = b->bh.bsize;

    if (bs < 0) {
        bs = -bs;
            V printf("Allocated buffer: size %6ld bytes.\n", (long) bs);
        if (dumpalloc) {
        bufdump((void *) (((char *) b) + sizeof(struct bhead)));
        }
    } else {
            char *lerr = "";

        ASSERT(bs > 0);
        if ((b->ql.blink->ql.flink != b) ||
        (b->ql.flink->ql.blink != b)) {
                lerr = "  (Bad free list links)";
        }
            V printf("Free block:       size %6ld bytes.%s\n",
        (long) bs, lerr);
#ifdef FreeWipe
        lerr = ((char *) b) + sizeof(struct bfhead);
        if ((bs > sizeof(struct bfhead)) && ((*lerr != 0x55) ||
        (memcmp(lerr, lerr + 1,
          (MemSize) (bs - (sizeof(struct bfhead) + 1))) != 0))) {
        V printf(
                    "(Contents of above free block have been overstored.)\n");
        bufdump((void *) (((char *) b) + sizeof(struct bhead)));
        } else
#endif
        if (dumpfree) {
        bufdump((void *) (((char *) b) + sizeof(struct bhead)));
        }
    }
    b = BFH(((char *) b) + bs);
    }
}
#endif /* BufDump */

#ifdef BufValid

/*  BPOOLV  --  Validate a buffer pool.  If NDEBUG isn't defined,
        any error generates an assertion failure.  */

int bpoolv(
  void *buf)
{
    struct bfhead *b = BFH(buf);

    while (b->bh.bsize != ESent) {
    bufsize bs = b->bh.bsize;

    if (bs < 0) {
        bs = -bs;
    } else {
            char *lerr = "";

        ASSERT(bs > 0);
        if (bs <= 0) {
        return 0;
        }
        if ((b->ql.blink->ql.flink != b) ||
        (b->ql.flink->ql.blink != b)) {
                V printf("Free block: size %6ld bytes.  (Bad free list links)\n",
             (long) bs);
        ASSERT(0);
        return 0;
        }
#ifdef FreeWipe
        lerr = ((char *) b) + sizeof(struct bfhead);
        if ((bs > sizeof(struct bfhead)) && ((*lerr != 0x55) ||
        (memcmp(lerr, lerr + 1,
          (MemSize) (bs - (sizeof(struct bfhead) + 1))) != 0))) {
        V printf(
                    "(Contents of above free block have been overstored.)\n");
        bufdump((void *) (((char *) b) + sizeof(struct bhead)));
        ASSERT(0);
        return 0;
        }
#endif
    }
    b = BFH(((char *) b) + bs);
    }
    return 1;
}
#endif /* BufValid */
void APP_BGet_Init(void)
{
	for(APP_BGet_Name_e i = (APP_BGet_Name_e)0; i<(MEM_N); i++)
	{
		bpool(i,APP_BGet_ControlHandle[i].MemAddress,APP_BGet_ControlHandle[i].Size);
	}
}
bufsize APP_BGet_TotalSize(APP_BGet_Name_e mem_name)
{
	return APP_BGet_ControlHandle[mem_name].Size;
}
bufsize APP_BGet_AllocSize(APP_BGet_Name_e mem_name)
{
	return APP_BGet_ControlHandle[mem_name].totalloc;
}
