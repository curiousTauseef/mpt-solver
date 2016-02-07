/*!
 * interface to MINPACK solver family
 */

#ifndef _MPT_MINPACK_H
#define _MPT_MINPACK_H	201405

#include "../solver.h"

#include <sys/uio.h>

__MPT_SOLVER_BEGIN

typedef void hybrd_fcn_t(int *, double *, double *, int *);
typedef void hybrj_fcn_t(int *, double *, double *, double *, int *, int *);

typedef void lmdif_fcn_t(int *, int *, double *, double *, int *);
typedef void lmder_fcn_t(int *, int *, double *, double *, double *, int *, int *);
typedef void lmstr_fcn_t(int *, int *, double *, double *, double *, int *);

enum MPT_ENUM(MinpackSolver) {
	MPT_ENUM(MinpackHybrd) = 1,
	MPT_ENUM(MinpackHybrj),
	MPT_ENUM(MinpackLmDif),
	MPT_ENUM(MinpackLmDer),
	MPT_ENUM(MinpackLmStr)
};

MPT_SOLVER_STRUCT(minpack)
{
#ifdef __cplusplus
public:
	minpack();
	~minpack();
#endif
	MPT_SOLVER_STRUCT(nlspar) nls; /* inherit nonlinear system parameter */
	
	char solv;    /* solver selection */
	char mode;    /* mode of operation */
	char nprint;  /* print iteration */
	char info;    /* status information */
	
	int maxfev;   /* max. function evaluations */
	
	int nfev,     /* function evaluations */
	    njev;     /* jacobian evaluations */
	
	int mu, ml;   /* hybrd: upper/lower bandwith */
	
	struct iovec val,   /* combined parameters and residuals */
	             diag,  /* scale factors for the variables */
	             work;  /* unified work vektor */
	
	double xtol;  /* desired relative error in solution */
	double ftol;  /* desired maximal residual */
	double gtol;  /* desired orthogonality (fcn/jaccol) */
	
	double factor;  /* initial step bound */
	double epsfcn;  /* initial forward-difference step length */
	
	union {
		hybrd_fcn_t *hd;
		hybrj_fcn_t *hj;
		lmdif_fcn_t *dif;
		lmder_fcn_t *der;
		lmstr_fcn_t *str;
	} fcn;  /* residual calculation */
	
	const MPT_SOLVER_STRUCT(nlsfcn) *ufcn;
};

__MPT_EXTDECL_BEGIN

/* simple C-Wrapper for hybr[dj]1_-call */
extern int hybrd1(hybrd_fcn_t *fcn, int n, double *x, double *fvec,
                  double tol, double *wa, int lwa);
extern int hybrj1(hybrj_fcn_t *fcn, int n, double *x, double *fvec,
                  double *fjac, int ldfjac, double tol, double *wa, int lwa);

/* simple C-Wrapper for lm{dif,der,str}1_-call */
extern int lmdif1(lmdif_fcn_t *fcn, int m, int n, double *x, double *fvec,
                  double tol, int *iwa, double *wa, int lwa);
extern int lmder1(lmder_fcn_t *fcn, int m, int n, double *x, double *fvec,
                  double *fjac, int ldfjac, double tol, int *iwa, double *wa, int lwa);
extern int lmstr1(lmstr_fcn_t *fcn, int m, int n, double *x, double *fvec,
                  double *fjac, int ldfjac, double tol, int *ipvt, double *wa, int lwa);

/* set hybrd parameter */
extern int mpt_minpack_get(const MPT_SOLVER_STRUCT(minpack) *, MPT_STRUCT(property) *);
extern int mpt_minpack_set(MPT_SOLVER_STRUCT(minpack) *, const char *, MPT_INTERFACE(metatype) *);

/* call minpack solver routine */
extern int mpt_minpack_solve(MPT_SOLVER_STRUCT(minpack) *);

/* initialize/terminate structured data */
extern void mpt_minpack_init(MPT_SOLVER_STRUCT(minpack) *);
extern void mpt_minpack_fini(MPT_SOLVER_STRUCT(minpack) *);

/* set wrapper for user functions */
extern int mpt_minpack_ufcn_hybrid(MPT_SOLVER_STRUCT(minpack) *);
extern int mpt_minpack_ufcn_lmderv(MPT_SOLVER_STRUCT(minpack) *);

/* set wrapper for user functions */
extern int mpt_minpack_prepare(MPT_SOLVER_STRUCT(minpack) *);

/* minpack status information */
extern int mpt_minpack_report(const MPT_SOLVER_STRUCT(minpack) *, int , MPT_TYPE(PropertyHandler) , void *);

/* assign minpack solver to interface */
#ifndef __cplusplus
extern MPT_SOLVER(NLS) *mpt_minpack_create(void);
#endif

__MPT_EXTDECL_END

#ifdef __cplusplus
inline minpack::minpack()
{ mpt_minpack_init(this); }
inline minpack::~minpack()
{ mpt_minpack_fini(this); }

class MinPack : public NLS, minpack
{
    public:
	MinPack() : _fcn(0)
	{ ufcn = &_fcn; }
	virtual ~MinPack()
	{ }
	uintptr_t addref()
	{
		return 0;
	}
	void unref()
	{
		delete this;
	}
	int property(struct property *pr) const
	{
		return mpt_minpack_get(this, pr);
	}
	int setProperty(const char *pr, metatype *src = 0)
	{
		return mpt_minpack_set(this, pr, src);
	}
	
	int report(int what, PropertyHandler out, void *data)
	{
		return mpt_minpack_report(this, what, out, data);
	}
	int solve()
	{
		return mpt_minpack_solve(this);
	}
	operator nlsfcn *() const
	{
		return const_cast<nlsfcn *>(&_fcn);
	}
	
	operator const nlspar *() const
	{
		return &nls;
	}
    protected:
	nlsfcn _fcn;
};
#endif

__MPT_SOLVER_END

#endif /* _MPT_MINPACK_H */

