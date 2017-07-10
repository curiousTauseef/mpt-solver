/*!
 * print extended information for mebdfi.
 */

#include <stdio.h>
#include <string.h>

#include "mebdfi.h"

#include "daesolv_modfcn.h"

extern int mpt_mebdfi_report(const MPT_SOLVER_STRUCT(mebdfi) *me, int show, MPT_TYPE(PropertyHandler) out, void *usr)
{
	MPT_STRUCT(property) pr;
	int line = 0, *iwk = me->iwork.iov_base;
	double *rwk = me->rwork.iov_base;
	
	if (show & MPT_SOLVER_ENUM(Header)) {
	pr.name = "jacobian";
	pr.desc = MPT_tr("method for jacobian");
	if (me->jbnd) {
		struct { const char *t; int32_t mu, ml; } val;
		val.t = (me->jac && !me->jnum) ? "banded(user)" : "banded";
		val.t = MPT_tr(val.t);
		val.ml = me->mbnd[0]; val.mu = me->mbnd[0];
		pr.val.fmt = "sii";
		pr.val.ptr = &val;
		out(usr, &pr);
	} else {
		pr.val.fmt  = 0;
		pr.val.ptr = (me->jac && !me->jnum) ? "full(user)" : "full";
		pr.val.ptr = MPT_tr(pr.val.ptr);
		out(usr, &pr);
	}
	++line;
	}
	
	if (show & MPT_SOLVER_ENUM(Values)) {
	MPT_SOLVER_MODULE_FCN(ivp_values)(&me->ivp, me->t, me->y, MPT_tr("MEBDFI solver state"), out, usr);
	}
	
	if (show & MPT_SOLVER_ENUM(Status)) {
	pr.name = "t";
	pr.desc = MPT_tr("value of independent variable");
	pr.val.fmt = "d";
	pr.val.ptr = &me->t;
	out(usr, &pr);
	++line;
	}
	
	if (show & (MPT_SOLVER_ENUM(Status) | MPT_SOLVER_ENUM(Report)) && iwk) {
	pr.name = "n";
	pr.desc = MPT_tr("integration steps");
	pr.val.fmt = "i";
	pr.val.ptr = &iwk[4];
	out(usr, &pr);
	++line;
	}
	
	if (show & MPT_SOLVER_ENUM(Status) && rwk) {
	pr.name = "h";
	pr.desc = MPT_tr("current step size");
	pr.val.fmt = "d";
	pr.val.ptr = &rwk[1];
	out(usr, &pr);
	++line;
	}
	
	if (show & MPT_SOLVER_ENUM(Report) && iwk) {
	
	if (iwk[5]) {
	pr.name = "nfail";
	pr.desc = MPT_tr("failed steps");
	pr.val.fmt = "i";
	pr.val.ptr = &iwk[5];
	out(usr, &pr);
	++line;
	}
	
	pr.name = "feval";
	pr.desc = MPT_tr("function evaluations");
	pr.val.fmt = "i";
	pr.val.ptr = &iwk[6];
	out(usr, &pr);
	++line;
	
	pr.name = "jeval";
	pr.desc = MPT_tr("jacobian evaluations");
	pr.val.fmt = "i";
	pr.val.ptr = &iwk[7];
	out(usr, &pr);
	++line;
	
	pr.name = "ludec";
	pr.desc = MPT_tr("LU decompositions");
	pr.val.fmt = "i";
	pr.val.ptr = &iwk[8];
	out(usr, &pr);
	++line;
	
	pr.name = "bsol";
	pr.desc = MPT_tr("backsolves");
	pr.val.fmt = "i";
	pr.val.ptr = &iwk[9];
	out(usr, &pr);
	++line;
	
	pr.name = "mform";
	pr.desc = MPT_tr("coeff. matrix form.");
	pr.val.fmt = "i";
	pr.val.ptr = &iwk[10];
	out(usr, &pr);
	++line;
	}
	
	return line;
}
