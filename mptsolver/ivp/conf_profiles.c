/*!
 * MPT solver library
 *   create iterator for profiles
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include "node.h"
#include "meta.h"
#include "convert.h"
#include "output.h"

#include "values.h"

#include "solver.h"

MPT_STRUCT(iterProfile) {
	MPT_INTERFACE(metatype) _mt;
	MPT_INTERFACE(iterator) _it;
	double t;
	long pos;
	long len;
	int neqs;
};
/* iterator interface */
static int iterProfileGet(MPT_INTERFACE(iterator) *it, int type, void *dest)
{
	MPT_STRUCT(iterProfile) *p = MPT_baseaddr(iterProfile, it, _it);
	double *val = (void *) (p + 1);
	MPT_INTERFACE(metatype) **mptr = (void *) (val  + p->neqs);
	MPT_INTERFACE(iterator) **iptr = (void *) (mptr + p->neqs);
	struct iovec *vec;
	int i;
	
	if (type != MPT_type_vector('d')) {
		return MPT_ERROR(BadType);
	}
	if (p->pos >= p->len) {
		return 0;
	}
	if (!(vec = dest)) {
		return MPT_type_vector('d');
	}
	for (i = 0; i < p->neqs; ++i) {
		MPT_INTERFACE(iterator) *curr;
		int ret;
		
		if (!(curr = iptr[i])) {
			continue;
		}
		if ((ret = curr->_vptr->get(curr, 'd', val + i)) < 0) {
			return ret;
		}
		if (!ret) {
			val[i] = 0;
		}
	}
	vec->iov_base = val;
	vec->iov_len = i * sizeof(*val);
	
	return MPT_type_vector('d');
}
static int iterProfileAdvance(MPT_INTERFACE(iterator) *it)
{
	MPT_STRUCT(iterProfile) *p = MPT_baseaddr(iterProfile, it, _it);
	double *val = (void *) (p + 1);
	MPT_INTERFACE(metatype) **mptr = (void *) (val  + p->neqs);
	MPT_INTERFACE(iterator) **iptr = (void *) (mptr + p->neqs);
	long i;
	
	if (p->pos >= p->len) {
		return MPT_ERROR(BadOperation);
	}
	if (++p->pos == p->len) {
		return 0;
	}
	for (i = 0; i < p->neqs; ++i) {
		int ret;
		if ((it = iptr[i])
		 && (ret = it->_vptr->advance(it)) < 0) {
			return ret;
		}
	}
	return MPT_type_vector('d');
}
static int iterProfileReset(MPT_INTERFACE(iterator) *it)
{
	MPT_STRUCT(iterProfile) *p = MPT_baseaddr(iterProfile, it, _it);
	double *val = (void *) (p + 1);
	MPT_INTERFACE(metatype) **mptr = (void *) (val  + p->neqs);
	MPT_INTERFACE(iterator) **iptr = (void *) (mptr + p->neqs);
	long i;
	
	for (i = 0; i < p->neqs; ++i) {
		int ret;
		if ((it = iptr[i])
		 && (ret = it->_vptr->reset(it)) < 0) {
			return ret;
		}
	}
	p->pos = -1;
	return p->len + 1;
}
/* convertable interface */
static int iterProfileConv(MPT_INTERFACE(convertable) *val, int type, void *ptr)
{
	const MPT_STRUCT(iterProfile) *p = (void *) val;
	
	if (!type) {
		static const uint8_t fmt[] = { MPT_ENUM(TypeIterator), 'd', 0 };
		if (ptr) {
			*((const uint8_t **) ptr) = fmt;
			return MPT_type_vector('d');
		}
		return MPT_ENUM(TypeIterator);
	}
	if (type == 'd') {
		if (ptr) *((double *) ptr) = p->t;
		return MPT_ENUM(TypeIterator);
	}
	if (type == MPT_ENUM(TypeMetaPtr)) {
		if (ptr) *((const void **) ptr) = &p->_mt;
		return MPT_ENUM(TypeIterator);
	}
	if (type == MPT_type_pointer(MPT_ENUM(TypeIterator))) {
		if (ptr) *((const void **) ptr) = &p->_it;
		return 'd';
	}
	return MPT_ERROR(BadType);
}
/* metatype interface */
static void iterProfileUnref(MPT_INTERFACE(metatype) *mt)
{
	const MPT_STRUCT(iterProfile) *p = (void *) mt;
	double *val = (void *) (p + 1);
	MPT_INTERFACE(metatype) **mptr = (void *) (val  + p->neqs);
	int i;
	
	for (i = 0; i < p->neqs; ++i) {
		MPT_INTERFACE(metatype) *mt;
		if ((mt = mptr[i])) {
			mt->_vptr->unref(mt);
		}
	}
	free(mt);
}
static uintptr_t iterProfileRef(MPT_INTERFACE(metatype) *mt)
{
	(void) mt;
	return 0;
}
static MPT_INTERFACE(metatype) *iterProfileClone(const MPT_INTERFACE(metatype) *mt)
{
	(void) mt;
	return 0;
}

/*!
 * \ingroup mptSolver
 * \brief configure parameter data
 * 
 * Get profile data from configuration element.
 * 
 * \param dat  solver data descriptor
 * \param neqs number of profiles
 * \param t    initial time
 * \param conf profile configuration node
 * \param out  error log descriptor
 * 
 * \return iterator containing profile segments
 */
extern MPT_INTERFACE(metatype) *mpt_conf_profiles(const MPT_STRUCT(solver_data) *dat, double t, const MPT_STRUCT(node) *conf, MPT_INTERFACE(logger) *out)
{
	static const MPT_INTERFACE_VPTR(iterator) iterProfileIter = {
		iterProfileGet, iterProfileAdvance, iterProfileReset
	};
	static const MPT_INTERFACE_VPTR(metatype) iterProfileMeta = {
		{ iterProfileConv }, iterProfileUnref, iterProfileRef, iterProfileClone
	};
	MPT_STRUCT(iterProfile) *ip;
	MPT_INTERFACE(metatype) **mptr;
	MPT_INTERFACE(iterator) **iptr;
	const MPT_STRUCT(type_traits) *info;
	const MPT_STRUCT(buffer) *buf;
	const MPT_STRUCT(node) *prof;
	const char *desc;
	double *val;
	size_t len;
	int i, neqs;
	
	if (!(buf = dat->val._buf)
	    || !(len = buf->_used / sizeof(*val))) {
		mpt_log(out, __func__, MPT_LOG(Error), "%s",
		        MPT_tr("empty buffer"));
		return 0;
	}
	if (!(info = buf->_typeinfo)
	    || (i = info->type) != 'd'
	    || info->size != sizeof(double)) {
		mpt_log(out, __func__, MPT_LOG(Error), "%s (%d)",
		        MPT_tr("bad grid content"), i);
		return 0;
	}
	if ((neqs = dat->nval) < 0) {
		if (!conf) {
			neqs = 0;
		}
		else if (!(prof = conf->children)) {
			neqs = 256;
		} else {
			neqs = 0;
			while (prof) {
				++neqs;
				prof = prof->next;
			}
		}
	}
	i = sizeof(*mptr) + sizeof(*iptr) + sizeof(*val);
	if (!(ip = malloc(sizeof(*ip) + neqs * i))) {
		mpt_log(out, __func__, MPT_LOG(Error), "%s: (len = %i)",
		        MPT_tr("failed to create iterator"), neqs);
		return 0;
	}
	ip->_mt._vptr = &iterProfileMeta;
	ip->_it._vptr = &iterProfileIter;
	ip->t = t;
	ip->pos = 0;
	ip->len = len;
	
	if (!conf) {
		ip->neqs = 0;
		return &ip->_mt;
	}
	val = memset(ip + 1, 0, neqs * sizeof(*val));
	
	/* no source iterator descriptions */
	if (!(prof = conf->children)) {
		i = 0;
		/* default zero profiles */
		if (!(desc = mpt_node_data(conf, 0))) {
			return &ip->_mt;
		}
		/* static initial values for components */
		while (i < neqs) {
			ssize_t len;
			if ((len = mpt_cdouble(val + i, desc, 0)) < 0) {
				if (out) mpt_log(out, __func__, MPT_LOG(Error), "%s (%d): %s",
				                 MPT_tr("bad profile constant"), i, desc);
				iterProfileUnref((void *) ip);
				errno = EINVAL;
				return 0;
			}
			if (!len) {
				if (!(neqs = i)) {
					ip->neqs = 0;
					return &ip->_mt;
				}
				break;
			}
			desc += len;
			++i;
		}
	}
	ip->neqs = neqs;
	mptr = memset(val  + neqs, 0, neqs * sizeof(*mptr));
	iptr = memset(mptr + neqs, 0, neqs * sizeof(*iptr));
	i = 0;
	while (prof && i++ < neqs) {
		MPT_INTERFACE(metatype) *mt;
		if (!(desc = mpt_node_data(prof, 0))) {
			if (out) mpt_log(out, __func__, MPT_LOG(Info), "%s: %d",
			                 MPT_tr("no profile description"), i);
			continue;
		}
		/* require valid iterator element */
		if (!(mt = mpt_iterator_profile(&dat->val, desc))) {
			if (out) {
				mpt_log(out, __func__, MPT_LOG(Warning), "%s (%d): %s",
				        MPT_tr("bad profile"), i, desc);
			}
			iterProfileUnref((void *) ip);
			errno = EINVAL;
			return 0;
		}
		*mptr++ = mt;
		/* skip bad iterator implementations */
		if (MPT_metatype_convert(mt, MPT_type_pointer(MPT_ENUM(TypeIterator)), iptr++) < 0) {
			if (out) {
				mpt_log(out, __func__, MPT_LOG(Warning), "%s (%d): %s",
				        MPT_tr("invalid profile iterator"), i, desc);
			}
		}
		prof = prof->next;
	}
	return &ip->_mt;
}

