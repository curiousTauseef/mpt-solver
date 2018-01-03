
#include <string.h>
#include <ctype.h>

#include "client.h"
#include "config.h"

#include "meta.h"

#include "solver.h"

/*!
 * \ingroup mptSolver
 * \brief solver proxy assign
 * 
 * Load (or convert) reference in proxy element.
 * Track/Replace reference in proxy structure.
 * 
 * \param ref    proxy metatype pointer reference
 * \param match  flags required for solver
 * \param conf   solver alias/symbol to bind
 * \param info   log/error output descriptor
 * 
 * \return solver interface (tracked by proxy reference)
 */
extern MPT_SOLVER(interface) *mpt_solver_load(MPT_INTERFACE(metatype) **ref, int match, const char *conf, MPT_INTERFACE(logger) *log)
{
	MPT_INTERFACE(metatype) *mt;
	MPT_SOLVER(interface) *sol;
	const char *old;
	int me;
	
	if ((me = mpt_solver_typeid()) < 0) {
		if (log) {
			mpt_log(log, __func__, MPT_LOG(Critical), "%s",
		                MPT_tr("no registration for solver interface"));
		}
		return 0;
	}
	mt = *ref;
	if (!conf) {
		if (!mt) {
			if (log) {
				mpt_log(log, __func__, MPT_LOG(Error), "%s",
			                MPT_tr("no existing solver"));
			}
			return 0;
		}
		return mpt_solver_conv(mt, match, log);
	}
	if (!*conf) {
		if (!(conf = mpt_solver_alias(0))) {
			if (log) {
				mpt_log(log, __func__, MPT_LOG(Error), "%s",
				        MPT_tr("no solver description"));
			}
			return 0;
		}
	}
	else if (!strchr(conf, '@')) {
		const char *name = conf;
		char tmp[256];
		size_t pos = 0;
		
		conf = 0;
		while (1) {
			const char *sym;
			int val;
			if (!(val = name[pos])) {
				if (!(sym = mpt_solver_alias(name))) {
					if (log) {
						mpt_log(log, __func__, MPT_LOG(Error), "%s (%d)",
						        MPT_tr("no valid solver alias"), conf);
					}
					return 0;
				}
				conf = sym;
				break;
			}
			if (!isspace(val)) {
				if (pos < sizeof(tmp)) {
					tmp[pos++] = val;
					continue;
				}
				++pos;
				while ((val = name[pos])) {
					if (isspace(val)) {
						break;
					}
					++pos;
				}
				val = pos;
				if (log) {
					mpt_log(log, __func__, MPT_LOG(Error), "%s (%d)",
					        MPT_tr("solver description element exceeds buffer"), val);
				}
				name += pos;
				pos = 0;
			}
			if (!pos) {
				++name;
				continue;
			}
			tmp[pos++] = 0;
			if ((sym = mpt_solver_alias(tmp))) {
				conf = sym;
				break;
			}
			if (log) {
				mpt_log(log, __func__, MPT_LOG(Error), "%s: %s",
				        MPT_tr("loader alias unknown"), tmp);
			}
			name += pos;
			pos = 0;
		}
	}
	/* identical to current instance */
	if (mt
	    && (old = mpt_meta_data(mt, 0))
	    && !strcmp(conf, old)
	    && (sol = mpt_solver_conv(mt, match, log))) {
		return sol;
	}
	/* change library symbol */
	else {
		const MPT_INTERFACE(metatype) *cfg;
		MPT_INTERFACE(metatype) *next;
		const char *lpath = 0;
		
		if ((cfg = mpt_config_get(0, "mpt.prefix.lib", '.', 0))) {
			cfg->_vptr->conv(cfg, 's', &lpath);
		}
		if (!(next = mpt_library_meta(me, conf, lpath, log))) {
			return 0;
		}
		if (!(sol = mpt_solver_conv(next, match, log))) {
			next->_vptr->ref.unref((void *) next);
			return 0;
		}
		if (mt) {
			mt->_vptr->ref.unref((void *) mt);
		}
		*ref = next;
		
		return sol;
	}
}
