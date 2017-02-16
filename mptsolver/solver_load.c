
#include <string.h>

#include "client.h"

#include "meta.h"

#include "solver.h"

/*!
 * \ingroup mptSolver
 * \brief solver proxy access
 * 
 * Load (or convert) solver via proxy element.
 * Track references via proxy data.
 * 
 * \param pr   proxy data
 * \param conf solver alias/symbol to mpt_library_bind
 * 
 * \return untracked solver interface instance
 */
/* load solver of specific type */
extern MPT_SOLVER(generic) *mpt_solver_load(MPT_STRUCT(proxy) *pr, int match, const char *conf, MPT_INTERFACE(logger) *log)
{
	MPT_SOLVER(generic) *sol;
	MPT_INTERFACE(metatype) *mt;
	const char *name;
	uintptr_t h;
	int mode;
	
	if (!conf) {
		if (!(mt = pr->_ref)) {
			if (log) {
				mpt_log(log, __func__, MPT_LOG(Error), "%s",
			                MPT_tr("no existing solver"));
			}
			return 0;
		}
		if (mt->_vptr->conv(mt, MPT_ENUM(TypeSolver), &sol) < 0) {
			if (log) {
				mpt_log(log, __func__, MPT_LOG(Error), "%s: %s",
			                MPT_tr("no solver in proxy instance"), conf);
			}
			return 0;
		}
		name = mpt_object_typename((void *) sol);
		mode = sol->_vptr->obj.property((void *) sol, 0);
		if (mode < 0) {
			if (log) {
				mpt_log(log, __func__, MPT_LOG(Error), "%s: %s",
			                MPT_tr("no valid solver"), name ? name : "");
			}
			return 0;
		}
		if (match && !(mode & match)) {
			if (log) {
				mpt_log(log, __func__, MPT_LOG(Error), "%s: 0x%02x",
			                MPT_tr("solver no IVP type"), mode);
			}
			return 0;
		}
		return sol;
	}
	if (!*conf) {
		if (log) {
			mpt_log(log, __func__, MPT_LOG(Error), "%s",
		                MPT_tr("no solver description"));
		}
		return 0;
	}
	if (!strchr(conf, '@')) {
		if (!(name = mpt_solver_alias(conf))) {
			if (log) {
				mpt_log(log, __func__, MPT_LOG(Error), "%s: %s",
			                MPT_tr("bad solver description"), conf);
			}
			return 0;
		}
		conf = name;
	}
	if (pr->_hash
	    && pr->_hash == (h = mpt_hash(conf, strlen(conf)))) {
		mt = pr->_ref;
	}
	else {
		const char *lpath = 0;
		if ((mt = mpt_config_get(0, "mpt.prefix.lib", '.', 0))) {
			mt->_vptr->conv(mt, 's', &lpath);
		}
		if (!(mt = mpt_library_bind(MPT_ENUM(TypeSolver), conf, lpath, log))) {
			return 0;
		}
	}
	if (mt->_vptr->conv(mt, MPT_ENUM(TypeSolver), &sol) < 0) {
		if (log) {
			mpt_log(log, __func__, MPT_LOG(Error), "%s: %s",
		                MPT_tr("no solver in proxy instance"), conf);
		}
		return 0;
	}
	if (sol) {
		MPT_STRUCT(property) prop;
		MPT_INTERFACE(metatype) *pre;
		
		prop.name = "";
		prop.desc = 0;
		
		if ((mode = sol->_vptr->obj.property((void *) sol, &prop)) < 0) {
			mode = MPT_ERROR(BadOperation);
			mt->_vptr->ref.unref((void *) mt);
			return 0;
		}
		if (match && !(mode & match)) {
			if (log) {
				mpt_log(log, __func__, MPT_LOG(Error), "%s (0x%x <> 0x%x): %s",
			                MPT_tr("incompatible solver type"), mode, match, conf);
			}
			mt->_vptr->ref.unref((void *) mt);
			return 0;
		}
		pre = pr->_ref;
		if (mt != pre) {
			if (pre) {
				pre->_vptr->ref.unref((void *) pre);
			}
			pr->_hash = mpt_hash(conf, strlen(conf));
			pr->_ref = mt;
		}
		return sol;
	}
	if (mt != pr->_ref) {
		mt->_vptr->ref.unref((void *) mt);
	}
	if (log) {
		mpt_log(log, __func__, MPT_LOG(Error), "%s: %s",
	                MPT_tr("bad solver instance pointer"), conf);
	}
	return 0;
}
