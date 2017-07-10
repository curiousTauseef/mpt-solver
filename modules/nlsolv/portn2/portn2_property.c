/*!
 * set PORT N2 solver parameter
 */

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <limits.h>
#include <float.h>

#include "meta.h"

#include "version.h"

#include "portn2.h"

#include "module_functions.h"

/*!
 * \ingroup mptNlSolvPortN2
 * \brief set N2 property
 * 
 * Query property of Port N2 solver
 * 
 * \param n2   N2 data
 * \param name name of property to change
 * \param src  data source to change property
 * 
 * \retval <0   failure
 * \retval >=0  used values
 */
extern int mpt_portn2_set(MPT_SOLVER_STRUCT(portn2) *n2, const char *name, const MPT_INTERFACE(metatype) *src)
{
	if (!name) {
		MPT_INTERFACE(iterator) *it;
		double *dst;
		int len;
		
		if (n2->nls.nval <= 0) {
			return MPT_ERROR(BadArgument);
		}
		len = n2->bnd ? 3 * n2->nls.nval : n2->nls.nval;
		
		if (!(dst = mpt_solver_valloc(&n2->pv, len, sizeof(double)))) {
			return MPT_ERROR(BadOperation);
		}
		if (!src) {
			memset(dst, 0, n2->nls.nval * sizeof(double));
			return 0;
		}
		if (src->_vptr->conv(src, MPT_ENUM(TypeIterator), &it) > 0) {
			return MPT_SOLVER_MODULE_FCN(data_set)(dst, n2->nls.nval, 0, it);
		}
		return MPT_ERROR(BadType);
	}
	if (!*name) {
		MPT_NLS_STRUCT(parameters) nls = MPT_NLSPAR_INIT;
		int ret = 0;
		if (src && (ret = mpt_solver_nlsset(&nls, src)) < 0) {
			return ret;
		}
		mpt_portn2_fini(n2);
		mpt_portn2_init(n2);
		n2->nls = nls;
		return ret;
	}
	return MPT_ERROR(BadArgument);
}

/*!
 * \ingroup mptNlSolvPortN2
 * \brief set N2 property
 * 
 * Query property of Port N2 solver
 * 
 * \param n2   N2 data
 * \param prop solver property
 * \param src  data source to change property
 * 
 * \retval 0   default value
 * \retval <0  failure
 * \retval >0  changed property
 */
extern int mpt_portn2_get(const MPT_SOLVER_STRUCT(portn2) *data, MPT_STRUCT(property) *prop)
{
	const char *name;
	intptr_t pos = -1, id;
	
	if (!prop) {
		return MPT_ENUM(TypeSolver);
	}
	if (!(name = prop->name)) {
		if ((pos = (intptr_t) prop->desc) < 0) {
			return MPT_ERROR(BadArgument);
		}
	}
	else if (!*name) {
		prop->name = "portn2"; prop->desc = "solver for overdetermined nonlinear equotations";
		prop->val.fmt = "ii"; prop->val.ptr = &data->nls;
		return MPT_SOLVER_ENUM(NlsVector) | MPT_SOLVER_ENUM(NlsOverdet);
	}
	
	if (name && !strcasecmp(name, "version")) {
		static const char version[] = BUILD_VERSION"\0";
		prop->name = "version"; prop->desc = "solver release information";
		prop->val.fmt = 0; prop->val.ptr = version;
		return 0;
	}
	(void) id;
	return MPT_ERROR(BadArgument);
}
