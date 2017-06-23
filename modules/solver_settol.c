/*!
 * create double vectors of specified length
 */

#include <stdlib.h>
#include <string.h>

#include <sys/uio.h>

#include "meta.h"

#include "../solver.h"

extern int mpt_solver_settol(MPT_SOLVER_TYPE(dvecpar) *vec, const MPT_INTERFACE(metatype) *src, double def)
{
	MPT_INTERFACE(iterator) *it;
	struct iovec tmp;
	double *tol;
	long len;
	int ret;
	
	if (!src) {
		if ((tol = vec->base)) {
			free(tol);
			vec->base = 0;
		}
		vec->d.val = def;
		return 0;
	}
	/* values from iterator */
	if ((ret = src->_vptr->conv(src, MPT_ENUM(TypeIterator), &it)) >= 0) {
		double d;
		long reserved = 0;
		if (!ret || !it) {
			return mpt_solver_settol(vec, 0, def);
		}
		tol = 0;
		len = 0;
		while (1) {
			if ((ret = it->_vptr->get(it, 'd', &d)) < 0) {
				if (tol) {
					free(tol);
				}
				return ret;
			}
			if (!ret || (ret = it->_vptr->advance(it)) <= 0) {
				break;
			}
			if (len >= reserved) {
				double *next;
				reserved += 8;
				if (!(next = realloc(tol, reserved * sizeof(*tol)))) {
					if (tol) {
						free(tol);
					}
					return MPT_ERROR(BadOperation);
				}
				tol = next;
			}
			tol[len++] = d;
			
			if (!ret) {
				break;
			}
		}
		if (len < 2) {
			mpt_solver_settol(vec, 0, len ? d : def);
		}
		if (vec->base) {
			free(vec->base);
		}
		vec->base = tol;
		return len;
	}
	if ((ret = src->_vptr->conv(src, MPT_value_toVector('d'), &tmp)) < 0) {
		MPT_STRUCT(value) val = MPT_VALUE_INIT;
		len = 0;
		/* values from value content */
		if ((ret = src->_vptr->conv(src, MPT_ENUM(TypeValue), &val)) < 0) {
			return ret;
		}
		else if (ret) {
			if (!val.fmt) {
				return MPT_ERROR(BadValue);
			}
			/* consume matching elements */
			while (*val.fmt++ == 'd') {
				++len;
			}
		}
		tmp.iov_base = (double *) val.ptr;
		
		if ((ret = len) < 2) {
			if (len && val.ptr) {
				def = *((double *) val.ptr);
			}
			mpt_solver_settol(vec, 0, def);
			return ret;
		}
	}
	/* values from double vector */
	else {
		len = tmp.iov_len / sizeof(*tol);
		if (len < 2) {
			if (len && tmp.iov_base) {
				def = *((double *) tmp.iov_base);
			}
			return mpt_solver_settol(vec, 0, def);
		}
		ret = 0;
	}
	/* reserve new tolerance data */
	tol = vec->base;
	if (!(tol = realloc(tol, len * sizeof(*tol)))) {
		return MPT_ERROR(BadOperation);
	}
	vec->base = tol;
	vec->d.len = len * sizeof(*tol);
	
	/* fill required size with default values */
	if (!tmp.iov_base) {
		int pos;
		for (pos = 0; pos < len; ++pos) {
			tol[pos] = def;
		}
	}
	/* copy available tolerance elements */
	else {
		memcpy(tol, tmp.iov_base, len * sizeof(*tol));
	}
	return ret;
}
