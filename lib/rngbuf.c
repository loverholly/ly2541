#include <stdlib.h>
#include <string.h>
#include "rngbuf.h"

/*******************************************************************************
*
* rngbuf_check_valid - check the rngbuf_id is valid
*
* This routine creates a ring buffer of size <nbytes>, and initializes
* it.  Memory for the buffer is allocated from the system memory partition.
*
* RETURNS
* true--the param is valid; false- the param is invalid.
*/
static bool rngbuf_check_valid(rngbuf_handle_t rngbuf_id)
{
	if (rngbuf_id == NULL || rngbuf_id->buf == NULL)
		return false;

	return true;
}

/*******************************************************************************
*
* rngbuf_create - create an empty ring buffer
*
* This routine creates a ring buffer of size <nbytes>, and initializes
* it.  Memory for the buffer is allocated from the system memory partition.
*
* RETURNS
* The ID of the ring buffer, or NULL if memory cannot be allocated.
*/

rngbuf_handle_t rngbuf_create(int nbytes)
{
	char *buffer;
	rngbuf_handle_t rngbuf_id = malloc(sizeof(rngbuf_t));

	if (rngbuf_id == NULL)
		return (NULL);

	/* bump number of bytes requested because ring buffer algorithm
	 * always leaves at least one empty byte in buffer */
	buffer = malloc((unsigned)++nbytes);

	if (buffer == NULL) {
		free(rngbuf_id);
		return (NULL);
	}

	rngbuf_id->size = nbytes;
	rngbuf_id->buf  = buffer;

	rngbuf_flush(rngbuf_id);

	return (rngbuf_id);
}
/*******************************************************************************
*
* rngbuf_delete - delete a ring buffer
*
* This routine deletes a specified ring buffer.
* Any data currently in the buffer will be lost.
*
* RETURNS: N/A
*/

void rngbuf_delete(rngbuf_handle_t rngbuf_id)
{
	if (rngbuf_check_valid(rngbuf_id) == false)
		return;

	free(rngbuf_id->buf);
	free(rngbuf_id);
	rngbuf_id->buf = NULL;
}
/*******************************************************************************
*
* rngbuf_flush - make a ring buffer empty
*
* This routine initializes a specified ring buffer to be empty.
* Any data currently in the buffer will be lost.
*
* RETURNS: N/A
*/

void rngbuf_flush(rngbuf_handle_t rngbuf_id)
{
	rngbuf_id->wptr = 0;
	rngbuf_id->rptr = 0;
}
/*******************************************************************************
*
* rngbuf_get - get characters from a ring buffer
*
* This routine copies bytes from the ring buffer <rngbuf_id> into <buffer>.
* It copies as many bytes as are available in the ring, up to <maxbytes>.
* The bytes copied will be removed from the ring.
*
* RETURNS:
* The number of bytes actually received from the ring buffer;
* it may be zero if the ring buffer is empty at the time of the call.
*/

int rngbuf_get(rngbuf_handle_t rngbuf_id, char *buffer, int maxbytes)
{
	if (rngbuf_check_valid(rngbuf_id) == false)
		return -1;

	int bytesgot = 0;
	int wptr = rngbuf_id->wptr;
	int bytes2;
	int rptr = 0;

	if (wptr >= rngbuf_id->rptr) {
		/* wptr has not wrapped around */
		bytesgot = min(maxbytes, wptr - rngbuf_id->rptr);
		bcopy(&rngbuf_id->buf[rngbuf_id->rptr], buffer, bytesgot);
		rngbuf_id->rptr += bytesgot;
	} else {
		/* wptr has wrapped around.  Grab chars up to the end of the
		 * buffer, then wrap around if we need to. */

		bytesgot = min(maxbytes, rngbuf_id->size - rngbuf_id->rptr);
		bcopy(&rngbuf_id->buf[rngbuf_id->rptr], buffer, bytesgot);
		rptr = rngbuf_id->rptr + bytesgot;

		/* If rptr is equal to size, we've read the entire buffer,
		 * and need to wrap now.  If bytesgot < maxbytes, copy some more chars
		 * in now. */
		if (rptr == rngbuf_id->size) {
			bytes2 = min(maxbytes - bytesgot, wptr);
			bcopy(rngbuf_id->buf, buffer + bytesgot, bytes2);
			rngbuf_id->rptr = bytes2;
			bytesgot += bytes2;
		} else {
			rngbuf_id->rptr = rptr;
		}
	}

	return (bytesgot);
}
/*******************************************************************************
*
* rngBufPut - put bytes into a ring buffer
*
* This routine puts bytes from <buffer> into ring buffer <rngbuf_id>.  The
* specified number of bytes will be put into the ring, up to the number of
* bytes available in the ring.
*
* INTERNAL
* Always leaves at least one byte empty between wptr and rptr, to
* eliminate ambiguities which could otherwise occur when the two pointers
* are equal.
*
* RETURNS:
* The number of bytes actually put into the ring buffer;
* it may be less than number requested, even zero,
* if there is insufficient room in the ring buffer at the time of the call.
*/

int rngbuf_put(rngbuf_handle_t rngbuf_id, char *buffer, int nbytes)
{
	if (rngbuf_check_valid(rngbuf_id) == false)
		return -1;

	int bytesput = 0;
	int rptr = rngbuf_id->rptr;
	int bytes2;
	int wptr = 0;

	if (rptr > rngbuf_id->wptr) {
		/* rptr is ahead of wptr.  We can fill up to two bytes
		 * before it */
		bytesput = min(nbytes, rptr - rngbuf_id->wptr - 1);
		bcopy(buffer, &rngbuf_id->buf[rngbuf_id->wptr], bytesput);
		rngbuf_id->wptr += bytesput;
	} else if (rptr == 0) {
		/* rptr is at the beginning of the buffer.  We can fill till
		 * the next-to-last element */
		bytesput = min(nbytes, rngbuf_id->size - rngbuf_id->wptr - 1);
		bcopy(buffer, &rngbuf_id->buf[rngbuf_id->wptr], bytesput);
		rngbuf_id->wptr += bytesput;
	} else {
		/* rptr has wrapped around, and its not 0, so we can fill
		 * at least to the end of the ring buffer.  Do so, then see if
		 * we need to wrap and put more at the beginning of the buffer. */
		bytesput = min(nbytes, rngbuf_id->size - rngbuf_id->wptr);
		bcopy(buffer, &rngbuf_id->buf[rngbuf_id->wptr], bytesput);
		wptr = rngbuf_id->wptr + bytesput;

		if (wptr == rngbuf_id->size) {
			/* We need to wrap, and perhaps put some more chars */
			bytes2 = min(nbytes - bytesput, rptr - 1);
			bcopy(buffer + bytesput, rngbuf_id->buf, bytes2);
			rngbuf_id->wptr = bytes2;
			bytesput += bytes2;
		} else {
			rngbuf_id->wptr = wptr;
		}
	}

	return (bytesput);
}
/*******************************************************************************
*
* rngbuf_is_empty - test if a ring buffer is empty
*
* This routine determines if a specified ring buffer is empty.
*
* RETURNS:
* TRUE if empty, FALSE if not.
*/

bool rngbuf_is_empty(rngbuf_handle_t rngbuf_id)
{
	if (rngbuf_check_valid(rngbuf_id) == false)
		return false;

	return (rngbuf_id->wptr == rngbuf_id->rptr);
}
/*******************************************************************************
*
* rngbuf_is_full - test if a ring buffer is full (no more room)
*
* This routine determines if a specified ring buffer is completely full.
*
* RETURNS:
* TRUE if full, FALSE if not.
*/

bool rngbuf_is_full(rngbuf_handle_t rngbuf_id)
{
	if (rngbuf_check_valid(rngbuf_id) == false)
		return false;

	int n = rngbuf_id->wptr - rngbuf_id->rptr + 1;
	return ((n == 0) || (n == rngbuf_id->size));
}

/*******************************************************************************
*
* rngbuf_free_bytes - determine the number of free bytes in a ring buffer
*
* This routine determines the number of bytes currently unused in a specified
* ring buffer.
*
* RETURNS: The number of unused bytes in the ring buffer.
*/

int rngbuf_free_bytes(rngbuf_handle_t rngbuf_id)
{
	if (rngbuf_check_valid(rngbuf_id) == false)
		return -1;

	int n = rngbuf_id->rptr - rngbuf_id->wptr - 1;
	if (n < 0)
		n += rngbuf_id->size;

	return (n);
}
/*******************************************************************************
*
* rngbuf_n_bytes - determine the number of bytes in a ring buffer
*
* This routine determines the number of bytes currently in a specified
* ring buffer.
*
* RETURNS: The number of bytes filled in the ring buffer.
*/

int rngbuf_n_bytes(rngbuf_handle_t rngbuf_id)
{
	if (rngbuf_check_valid(rngbuf_id) == false)
		return -1;

	int n = rngbuf_id->wptr - rngbuf_id->rptr;
	if (n < 0)
		n += rngbuf_id->size;

	return (n);
}
/*******************************************************************************
*
* rngbuf_put_ahead - put a byte ahead in a ring buffer without moving ring pointers
*
* This routine writes a byte into the ring, but does not move the ring buffer
* pointers.  Thus the byte will not yet be available to rngbuf_get() calls.
* The byte is written <offset> bytes ahead of the next input location in the
* ring.  Thus, an offset of 0 puts the byte in the same position as would
* RNG_ELEM_PUT would put a byte, except that the input pointer is not updated.
*
* Bytes written ahead in the ring buffer with this routine can be made available
* all at once by subsequently moving the ring buffer pointers with the routine
* rngMoveAhead().
*
* Before calling rngbuf_put_ahead(), the caller must verify that at least
* <offset> + 1 bytes are available in the ring buffer.
*
* RETURNS: N/A
*/

void rngbuf_put_ahead(rngbuf_handle_t rngbuf_id, char byte, int offset)
{
	if (rngbuf_check_valid(rngbuf_id) == false)
		return;

	int n = rngbuf_id->wptr + offset;

	if (n >= rngbuf_id->size)
		n -= rngbuf_id->size;

	*(rngbuf_id->buf + n) = byte;
}
/*******************************************************************************
*
* rngbuf_move_ahead - advance a ring pointer by <n> bytes
*
* This routine advances the ring buffer input pointer by <n> bytes.  This makes
* <n> bytes available in the ring buffer, after having been written ahead in
* the ring buffer with rngbuf_put_ahead().
*
* RETURNS: N/A
*/

void rngbuf_move_ahead(rngbuf_handle_t rngbuf_id, int n)
{
	if (rngbuf_check_valid(rngbuf_id) == false)
		return;

	n += rngbuf_id->wptr;

	if (n >= rngbuf_id->size)
		n -= rngbuf_id->size;

	rngbuf_id->wptr = n;
}
