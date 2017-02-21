/*!
 *  define IVP user init parameters
 */
#ifndef MPT_INCLUDE
# define MPT_INCLUDE(h) <mpt/h>
#endif
#include MPT_INCLUDE(array.h)  /* make solver data accessable */
#include MPT_INCLUDE(solver.h)
#include "solver_run.h"
extern int user_init(MPT_SOLVER(IVP) *, MPT_STRUCT(solver_data) *, MPT_INTERFACE(logger) *);
