
#include <mpt/message.h>
#include <mpt/client.h>

#include <mpt/solver.h>

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

int main()
{
	struct mpt_proxy pr = MPT_PROXY_INIT;
	struct mpt_solver_generic *sol;
	
	mtrace();
	
	if (!(sol = mpt_solver_load(&pr, MPT_SOLVER_ENUM(CapableIvp), "sundials_cvode_create@libmpt_sundials.so.1"))) {
		return 1;
	}
	return 0;
}
