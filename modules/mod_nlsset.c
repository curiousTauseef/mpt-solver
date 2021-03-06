/*!
 * MPT solver module helper function
 *   set NLS parameter and residual count
 */

#include <stdlib.h>

#include "../solver.h"

#include "meta.h"

extern int mpt_solver_module_value_nls(MPT_STRUCT(value) *val, const MPT_NLS_STRUCT(parameters) *par)
{
	static const uint8_t fmt[] = "ii";
	val->fmt = fmt;
	val->ptr = par;
	return par && (par->nval != 1 || par->nres) ? 1 : 0;
}

extern int mpt_solver_module_nlsset(MPT_NLS_STRUCT(parameters) *nls, MPT_INTERFACE(convertable) *src)
{
	MPT_STRUCT(consumable) val;
	int32_t nv = 1, nr = 0;
	int ret;
	
	if (!src) {
		nls->nval = nv;
		nls->nres = nr;
		return 0;
	}
	if ((ret = mpt_consumable_setup(&val, src)) < 0) {
		if ((ret = src->_vptr->convert(src, 'i', &nv)) < 0) {
			return ret;
		}
		ret = 0;
	}
	if (nv < 0) {
		return MPT_ERROR(BadValue);
	}
	if (!ret) {
		nls->nval = nv;
		nls->nres = nr;
		return 0;
	}
	if ((ret = mpt_consume_int(&val, &nv)) < 0) {
		return ret;
	}
	if (!ret) {
		nls->nval = nv;
		nls->nres = nr;
		return 0;
	}
	if (nv < 0) {
		return MPT_ERROR(BadValue);
	}
	if ((ret = mpt_consume_int(&val, &nr)) < 0) {
		return ret;
	}
	if (!ret) {
		nls->nval = nv;
		nls->nres = nr;
		return 1;
	}
	if (nr < 0) {
		return MPT_ERROR(BadValue);
	}
	nls->nval = nv;
	nls->nres = nr;
	return 2;
}
