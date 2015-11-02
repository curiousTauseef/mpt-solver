/*!
 * get vecpar properties
 */

#include <string.h>
#include <sys/uio.h>

#include "solver.h"

static int getValues(MPT_TYPE(dvecpar) *val, MPT_INTERFACE(source) *src)
{
	struct iovec tmp;
	double v1 = 0.0, v2, *dest;
	int len, pos, full;
	
	if (!src) return val->base ? val->d.len/sizeof(double) : 0;
	
	if ((len = src->_vptr->conv(src, 'd' | MPT_ENUM(TypeVector), &tmp)) >= 0) {
		size_t elem = tmp.iov_len / sizeof(double);
		if (!elem) {
			dest = mpt_vecpar_alloc((struct iovec *) val, 0, 0);
			val->d.val = 0;
		}
		else if (!(dest = mpt_vecpar_alloc((struct iovec *) val, elem, sizeof(double)))) {
			return -1;
		}
		else {
			memcpy(val->base, tmp.iov_base, val->d.len);
		}
		return elem;
	}
	if ((len = src->_vptr->conv(src, 'd', &v1)) < 0) {
		return -2;
	}
	/* single value only */
	if (!len || (pos = src->_vptr->conv(src, 'd', &v2)) <= 0) {
		dest = mpt_vecpar_alloc((struct iovec *) val, 0, 0);
		val->d.val = v1; return len ? 1 : 0;
	}
	full = (dest = val->base) ? val->d.len/sizeof(double) : 0;
	
	/* initial vector conversion */ 
	if (((pos = 2) > full)
	    && !(dest = mpt_vecpar_alloc((struct iovec *) val, 2, sizeof(double)))) {
		return -1;
	}
	dest[0] = v1;
	dest[1] = v2;
	
	/* read further data */
	while ((len = src->_vptr->conv(src, 'd', &v2)) > 0) {
		/* increase vector space */
		if (pos >= full && !(dest = mpt_vecpar_alloc((struct iovec *) val, pos+1, sizeof(double)))) {
			break;
		}
		dest[pos++] = v2;
	}
	/* shrink to required size */
	if (full > pos) {
		mpt_vecpar_alloc((struct iovec *) val, pos, sizeof(double));
	}
	return pos;
}

extern int mpt_vecpar_value(MPT_TYPE(dvecpar) *tol, MPT_STRUCT(value) *val, MPT_INTERFACE(source) *src)
{
	int len = 0;
	
	len = getValues(tol, src);
	
	if (!val) {
		return len;
	}
	if (tol->base) {
		static const char fmt[2] = { (char) (MPT_ENUM(TypeVector) | 'd') };
		val->fmt = fmt;
		val->ptr = tol;
	} else {
		static const char fmt[2] = "d";
		val->fmt = fmt;
		val->ptr = &tol->d.val;
	}
	return len;
}
