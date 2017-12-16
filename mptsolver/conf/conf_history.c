/*!
 * read output parameter from configuration list.
 */

#include <stdio.h>
#include <string.h>

#include <sys/un.h>
#include <netinet/in.h>

#include "node.h"
#include "message.h"

#include "solver.h"

/*!
 * \ingroup mptOutput
 * \brief set history parameters
 * 
 * get history parameters from configuration list.
 * 
 * \param out   output descriptor
 * \param conf  configuration list
 * 
 * \return combined result of history configuration
 */
extern int mpt_conf_history(MPT_INTERFACE(object) *out, const MPT_STRUCT(node) *conf)
{
	static const int flags = MPT_ENUM(TraverseLeafs) | MPT_ENUM(TraverseChange) | MPT_ENUM(TraverseDefault);
	const char *data;
	int ret;
	
	if (!out) {
		return 0;
	}
	if (!conf) {
		out->_vptr->setProperty(out, 0, 0);
		return 0;
	}
	/* process output config */
	data = mpt_node_data(conf, 0);
	conf = conf->children;
	if (!conf) {
		if ((ret = mpt_object_set_string(out, 0, data, 0)) < 0) {
			return ret;
		}
		return 1;
	}
	ret = 0;
	do {
		/* apply options with non-empty name */
		if (mpt_object_set_node(out, conf, flags) >= 0) {
			++ret;
		}
	} while ((conf = conf->next));
	return ret;
}
