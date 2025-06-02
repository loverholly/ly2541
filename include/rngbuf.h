#ifndef __RNGBUF_H__
#define __RNGBUF_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "types.h"

typedef struct {	/* rngbuf - ring buffer */
	volatile int wptr;		/* offset from start of buffer where to write next */
	volatile int rptr;	/* offset from start of buffer where to read next */
	int size;	/* size of ring in bytes */
	char *buf;		/* pointer to start of buffer */
} rngbuf_t;

typedef rngbuf_t *rngbuf_handle_t;


/*
 * The following macros are designed to do various operations on
 * the RING object.  By using them, users can avoid having to know
 * the structure of RING.  However they are intended to be very
 * efficient and this is why they are macros in the first place.
 * In general the parameters to them should be register variables
 * for maximum efficiency.
 */

/*******************************************************************************
*
* RNG_ELEM_GET - get one character from a ring buffer
*
* This macro gets a single character from the specified ring buffer.
* Must supply temporary variable (register int) 'fromP'.
*
* RETURNS: 1 if there was a char in the buffer to return, 0 otherwise
*
* NOMANUAL
*/

#define RNG_ELEM_GET(rngbuf_id, char, rptr)		\
	(						\
	                        rptr = (rngbuf_id)->rptr,			\
	                        ((rngbuf_id)->wptr == rptr) ?		\
	                        0 					\
	                        :						\
	                        (					\
	                            *char = (rngbuf_id)->buf[rptr],		\
	                            (rngbuf_id)->rptr = ((++rptr == (rngbuf_id)->size) ? 0 : rptr), \
	                            1					\
	                        )					\
	)

/*******************************************************************************
*
* RNG_ELEM_PUT - put one character into a ring buffer
*
* This macro puts a single character into the specified ring buffer.
* Must supply temporary variable (register int) 'toP'.
*
* RETURNS: 1 if there was room in the buffer for the char, 0 otherwise
*
* NOMANUAL
*/

#define RNG_ELEM_PUT(rngbuf_id, char, wptr)		\
	(						\
	                        wptr = (rngbuf_id)->wptr,			\
	                        (wptr == (rngbuf_id)->rptr - 1) ?		\
	                        0 					\
	                        :						\
	                        (					\
	                            (wptr == (rngbuf_id)->size - 1) ?	\
	                            (					\
	                                ((rngbuf_id)->rptr == 0) ?		\
	                                0				\
	                                :					\
	                                (				\
	                                    (rngbuf_id)->buf[wptr] = char,	\
	                                    (rngbuf_id)->wptr = 0,		\
	                                    1				\
	                                )				\
	                            )					\
	                            :					\
	                            (					\
	                                (rngbuf_id)->buf[wptr] = char,		\
	                                (rngbuf_id)->wptr++,			\
	                                1					\
	                            )					\
	                        )					\
	)

bool 	rngbuf_is_empty(rngbuf_handle_t rngbuf_id);
bool 	rngbuf_is_full(rngbuf_handle_t rngbuf_id);
rngbuf_handle_t rngbuf_create(int nbytes);
int 	rngbuf_get(rngbuf_handle_t rngId, char *buffer, int maxbytes);
int 	rngbuf_put(rngbuf_handle_t rngId, char *buffer, int nbytes);
int 	rngbuf_free_bytes(rngbuf_handle_t rngbuf_id);
int 	rngbuf_n_bytes(rngbuf_handle_t rngbuf_id);
void 	rngbuf_delete(rngbuf_handle_t rngbuf_id);
void 	rngbuf_flush(rngbuf_handle_t rngbuf_id);
void 	rngbuf_put_ahead(rngbuf_handle_t rngbuf_id, char byte, int offset);
void 	rngbuf_move_ahead(rngbuf_handle_t rngbuf_id, int n);

#ifdef __cplusplus
}
#endif

#endif	/* __RNGBUF_H__ */
