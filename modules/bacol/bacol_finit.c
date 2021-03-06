/*!
 * initialize BACOL solver data
 */

#include <stdlib.h>
#include <string.h>

#include <math.h>

#include "bacol.h"

extern void mpt_bacol_fini(MPT_SOLVER_STRUCT(bacol) *bac)
{
	mpt_solver_module_ivpset(&bac->ivp, 0);
	
	mpt_solver_module_tol_check(&bac->rtol, 0, 0, __MPT_IVP_RTOL);
	mpt_solver_module_tol_check(&bac->atol, 0, 0, __MPT_IVP_ATOL);
	
	if (bac->xy) {
		free(bac->xy);
		bac->xy = 0;
	}
	mpt_solver_module_valloc(&bac->ipar, 0, 0);
	mpt_solver_module_valloc(&bac->rpar, 0, 0);
	
	switch (bac->_backend) {
#ifdef MPT_BACOL_RADAU
	  case 'r': case 'R':
		if (bac->bd.cpar.iov_base) {
			free(bac->bd.cpar.iov_base);
			bac->bd.cpar.iov_base = 0;
		}
		bac->bd.cpar.iov_len  = 0;
#endif
		break;
	  default:;
	}
	
	bac->mflag.noinit = -1;
}

extern void mpt_bacol_init(MPT_SOLVER_STRUCT(bacol) *bac)
{
	bac->ivp.neqs = 1;
	bac->ivp.pint = 0;
	
	MPT_VECPAR_INIT(&bac->rtol, __MPT_IVP_RTOL);
	MPT_VECPAR_INIT(&bac->atol, __MPT_IVP_ATOL);
	
	bac->initstep = NAN;
	
	bac->nint     = 10;
	bac->nintmx   = MPT_BACOL_NIMAXDEF;
	bac->kcol     = 2;
	bac->_backend = 0;
	
	(void) memset(&bac->mflag, 0, sizeof(bac->mflag));
	bac->mflag.noinit = -1;
	
	
	bac->t = 0.0;
	bac->xy = 0;
	
	bac->rpar.iov_base = 0; bac->rpar.iov_len = 0;
	bac->ipar.iov_base = 0; bac->ipar.iov_len = 0;
	
	memset(&bac->bd, 0, sizeof(bac->bd));
}

