/*!
 * create client for solving nonlinear systems.
 */

#include <sys/uio.h>

#include "message.h"
#include "array.h"

#include "values.h"
#include "output.h"

#include "solver.h"

static void outputSize(MPT_STRUCT(output) *out, int nres)
{
	MPT_STRUCT(msgtype) hdr;
	uint32_t val;
	uint8_t fmt;
	int len;
	
	hdr.cmd = MPT_MESGTYPE(ValueFmt);
	hdr.arg = 0;
	
	/* push header data indicator */
	if ((len = out->_vptr->push(out, sizeof(hdr), &hdr)) < 0) {
		return;
	}
	/* append residual size */
	fmt = MPT_message_value(Unsigned, val);
	val = nres;
	out->_vptr->push(out, sizeof(fmt), &fmt);
	out->_vptr->push(out, sizeof(val), &val);
	/* terminate message */
	out->_vptr->push(out, 0, 0);
}

static void outputParam(MPT_STRUCT(output) *out, int state, const double *par, int np)
{
	struct {
		MPT_STRUCT(msgtype) mt;
		MPT_STRUCT(msgbind) bnd;
	} hdr;
	int i;
	
	hdr.mt.cmd = MPT_MESGTYPE(ValueRaw);
	hdr.mt.arg = (int8_t) MPT_message_value(Float, *par);
	
	/* indicate special data */
	hdr.bnd.dim = state;
	hdr.bnd.state = 0;
	
	/* push header */
	if (out->_vptr->push(out, sizeof(hdr), &hdr) < 0) {
		return;
	}
	/* append values */
	for (i = 0; i < np; ++i) {
		if (out->_vptr->push(out, sizeof(*par), par++) < 0) {
			out->_vptr->push(out, 1, 0);
		}
	}
	/* terminate message */
	out->_vptr->push(out, 0, 0);
}
static void outputValues(MPT_STRUCT(output) *out, int state, int dim, int len, const double *val, int ld)
{
	struct {
		MPT_STRUCT(msgtype) mt;
		MPT_STRUCT(msgbind) bnd;
	} hdr;
	
	hdr.mt.cmd = MPT_MESGTYPE(ValueRaw);
	hdr.mt.arg = (int8_t) MPT_message_value(Float, *val);
	
	hdr.bnd.dim = dim & 0xff;
	hdr.bnd.state = state & 0xff;
	
	out->_vptr->push(out, sizeof(hdr), &hdr);
	mpt_output_values(out, len, val, ld);
	out->_vptr->push(out, 0, 0);
}

/*!
 * \ingroup mptSolver
 * \brief NLS data output
 * 
 * Push data state message to output.
 * 
 * \param out    nonlinear solver descriptor
 * \param state  state of nonlinear solver data 
 * \param val    nonlinear solver parameters (and residuals)
 * \param dat    solver data for residial dimension mask
 * 
 * \return message push result
 */
extern int mpt_solver_output_nls(const MPT_STRUCT(solver_output) *out, int state, const MPT_STRUCT(value) *val, const MPT_STRUCT(solver_data) *sd)
{
	const MPT_STRUCT(buffer) *buf;
	const uint8_t *pass;
	const char *fmt;
	const struct iovec *vec;
	const double *res, *par;
	size_t passlen;
	int nr, np;
	
	if (!val) {
		return MPT_ERROR(BadArgument);
	}
	if (!(fmt = val->fmt)
	    || fmt[0] != MPT_value_toVector('d')) {
		return MPT_ERROR(BadType);
	}
	if (!(vec = val->ptr)) {
		return MPT_ERROR(BadValue);
	}
	if (!out->_data && !out->_graphic) {
		return 0;
	}
	par = vec->iov_base;
	np  = vec->iov_len / sizeof (*par);
	
	res = 0;
	if (fmt[1] == MPT_value_toVector('d')) {
		res = vec[1].iov_base;
		nr  = vec[1].iov_len / sizeof(*res);
	}
	else if (fmt[1] == 'd') {
		res = (void *) (vec + 1);
		nr  = 1;
	}
	/* output parameters */
	if (par &&
	    (state & (MPT_DATASTATE(Init) | MPT_DATASTATE(Step) | MPT_DATASTATE(Fini)))) {
		if (out->_data) {
			outputParam(out->_data, state, par, np);
		}
		if (out->_graphic
		    && out->_graphic != out->_data) {
			/* pass state via dimension */
			outputParam(out->_graphic, state, par, np);
		}
	}
	if (!res) {
		return 1;
	}
	if ((buf = out->_pass._buf)) {
		pass = (void *) (buf + 1);
		passlen = buf->_used / sizeof(uint8_t);
	} else {
		pass = 0;
		passlen = 0;
	}
	/* output residuals */
	if (out->_graphic
	    && state & MPT_DATASTATE(Step)) {
		if (!pass || (passlen && (pass[0] & state))) {
			outputValues(out->_graphic, state, 0, nr, res, 1);
		}
	}
	if (out->_data
	    && state & MPT_DATASTATE(Fini)) {
		outputSize(out->_data, nr);
		mpt_output_solver_history(out->_data, res, nr, 0, 0);
	}
	/* output user data */
	if (out->_graphic
	    && (state & MPT_DATASTATE(Init))
	    && sd
	    && (buf = sd->val._buf)
	    && (np = buf->_used / sizeof(double))) {
		const double *val = (double *) (buf + 1);
		int i, nv, add;
		if ((nv = sd->nval) < 0) {
			np = 0;
		}
		else if (nv) {
			np /= nv;
		} else {
			nv = 1;
		}
		add = 0;
		for (i = 0; i < np; ++i) {
			size_t pos = i + 1;
			if (!pass || (pos < passlen && pass[pos] & state)) {
				outputValues(out->_graphic, state, pos, np, val + i, nv);
				++add;
			}
		}
		return add;
	}
	return 0;
}
