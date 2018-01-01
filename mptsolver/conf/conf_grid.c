/*!
 * get grid start and dimension.
 */

#include <errno.h>

#include "array.h"
#include "meta.h"

#include "values.h"

/*!
 * \ingroup mptValues
 * \brief create grid data
 * 
 * Create grid values from configuration.
 * 
 * \param grid  target array
 * \param conf  configuration node for grid
 * 
 * \return size of grid
 */
extern int mpt_conf_grid(MPT_STRUCT(array) *grid, const MPT_INTERFACE(metatype) *conf)
{
	MPT_INTERFACE(iterator) *it;
	MPT_INTERFACE(metatype) *mt;
	MPT_STRUCT(buffer) *buf;
	double *dest;
	const char *desc;
	size_t old, len;
	int ret, pts;
	
	if ((buf = grid->_buf)) {
		old = buf->_used;
	} else {
		old = 0;
	}
	it = 0;
	mt = 0;
	if (conf) {
		/* use existing iterator */
		if ((ret = conf->_vptr->conv(conf, MPT_ENUM(TypeIterator), &it)) > 0) {
			if (!it) {
				return MPT_ERROR(BadValue);
			}
		}
		/* use default grid settings */
		else if (!(desc = mpt_meta_data(conf, 0))) {
			if (!(mt = mpt_iterator_linear(10, 0, 1))) {
				return MPT_ERROR(BadOperation);
			}
			return MPT_ERROR(BadType);
		}
		/* make iterator from description */
		else if (!(mt = mpt_iterator_create(desc))) {
			return MPT_ERROR(BadValue);
		}
	}
	if (mt) {
		if ((ret = mt->_vptr->conv(mt, MPT_ENUM(TypeIterator), &it)) < 0
		    || !it) {
			mt->_vptr->ref.unref((void *) mt);
			return MPT_ERROR(BadType);
		}
	}
	pts = 0;
	len = 0;
	ret = -1;
	while (1) {
		/* increase buffer size */
		if (!len) {
			if (!(dest = mpt_values_prepare(grid, len = 16))) {
				if (buf) {
					buf->_used = old;
				}
				return MPT_ERROR(BadOperation);
			}
			buf = grid->_buf;
		}
		/* get iterator data */
		if ((ret = it->_vptr->get(it, 'd', dest)) < 0) {
			if (mt) {
				mt->_vptr->ref.unref((void *) mt);
			}
			buf->_used = old;
			return ret;
		}
		if (!ret) {
			break;
		}
		/* advance position and iterator */
		--len;
		++pts;
		++dest;
		if ((ret = it->_vptr->advance(it)) < 0) {
			if (mt) {
				mt->_vptr->ref.unref((void *) mt);
			}
			buf->_used = old;
			return ret;
		}
		if (!ret) {
			break;
		}
	}
	if (mt) {
		mt->_vptr->ref.unref((void *) mt);
	} else {
		it->_vptr->reset(it);
	}
	/* crop unused trailing buffer data */
	if (buf && len) {
		buf->_used -= len * sizeof(double);
	}
	return pts;
}

