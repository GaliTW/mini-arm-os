#include <stddef.h>
#include "malloc.h"
#include "os.h"

typedef long Align;

union header {
	struct {
		union header *ptr;
		unsigned int size;
	} s;
	Align x;
};

typedef union header Header;

static unsigned char heaps[MAX_HEAPS];
static unsigned char *program_break = heaps;

static Header base; /* empty list to get started */
static Header *freep = NULL; /* start of free list */

static void *sbrk(unsigned int nbytes)
{
	if (program_break + nbytes >= heaps
	    && program_break + nbytes < heaps + MAX_HEAPS) {
		unsigned char *previous_pb = program_break;
		program_break += nbytes;
		return (void *) previous_pb;
	}
	return (void *) - 1;
}

void *malloc(unsigned int nbytes)
{
	if (nbytes <= 0)
		return NULL;
	Header *p, *prevp;

	unsigned int nunits = (nbytes - 1) / sizeof(Header) + 2;

	if (freep == NULL) {
		base.s.ptr = freep = &base;
		base.s.size = 0;
	}

	for (prevp = freep, p = freep->s.ptr; p != freep; prevp = p, p = p->s.ptr) {
		if (p->s.size > nunits) {
			p->s.size -= nunits;
			p += p->s.size;
			p->s.size = nunits;
			return (void *)(p + 1);
		}

		if (p->s.size == nunits) {
			prevp->s.ptr = p->s.ptr;
			return (void *)(p + 1);
		}
	}

	p = (Header *) sbrk(nunits * sizeof(Header));
	if ((void *) p == (void *) - 1)
		return NULL;

	p->s.ptr = NULL;
	p->s.size = nunits;
	return (void *)(p + 1);
}

void free(void *ap)
{
	unsigned char insert = 1;
	Header *p, *prevp;
	Header *bp = (Header *) ap - 1;
	if ((void *)bp < (void *)heaps || (void *)bp >= (void *)program_break)
		return ;

	for (prevp = freep, p = freep->s.ptr; p != freep; prevp = p, p = p->s.ptr) {
		if (p > bp)
			break;
	}

	if (bp + bp->s.size == p) {
		bp->s.size += p->s.size;
		bp->s.ptr = p->s.ptr;
		prevp->s.ptr = bp;
		insert = 0;
	}

	if (prevp + prevp->s.size == bp) {
		prevp->s.size += bp->s.size;
		if (!insert)
			prevp->s.ptr = bp->s.ptr;
		insert = 0;
	}

	if (insert) {
		prevp->s.ptr = bp;
		bp->s.ptr = p;
	}
}
