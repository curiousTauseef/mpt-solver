/*!
 * check/adapt memory sizes and integrator parameters.
 */

#include <string.h>
#include <stdlib.h>

#include "limex.h"

extern int mpt_limex_prepare(MPT_SOLVER_STRUCT(limex) *data)
{
	int neqs, pdim;
	double *v;
	int *ipos;
	
	pdim = data->ivp.pint + 1;
	neqs = data->ivp.neqs * pdim;
	
	if (data->iopt[7] < 0) data->iopt[7] = data->ivp.neqs;
	if (data->iopt[8] < 0) data->iopt[8] = data->ivp.neqs;
	
	/* vector tolerances in sufficent dimension */
	if (data->ivp.neqs < 2 || (!data->rtol._base && !data->atol._base)) {
		pdim = 0;
	}
	if (mpt_solver_module_tol_check(&data->atol, neqs, pdim, __MPT_IVP_ATOL) < 0) {
		return MPT_ERROR(BadOperation);
	}
	if (mpt_solver_module_tol_check(&data->rtol, neqs, pdim, __MPT_IVP_ATOL) < 0) {
		return MPT_ERROR(BadOperation);
	}
	if (!(ipos = data->ipos)) {
		size_t len = sizeof(*ipos) * neqs;
		if (!(ipos = malloc(len))) {
			return MPT_ERROR(BadOperation);
		}
		data->ipos = memset(ipos, 0, len);
	}
	if (!(v = data->y)) {
		size_t len = sizeof(*v) * neqs;
		if (!(v = malloc(len))) {
			return MPT_ERROR(BadOperation);
		}
		data->y = memset(v, 0, len);
	}
	if (!(v = data->ys)) {
		size_t len = sizeof(*v) * neqs;
		if (!(v = malloc(len))) {
			return MPT_ERROR(BadOperation);
		}
		data->ys = memset(v, 0, len);
		
		/* determine consistent initstate */
		data->iopt[5] = 1;
	}
	/* reset counter on reinitialisation */
	for (pdim = 0; pdim < 6; pdim++) data->iopt[23+pdim] = 0;
	
	data->iopt[15] = 0;
	
	return neqs;
}
