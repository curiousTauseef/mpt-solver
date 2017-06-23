/*!
 * interface to dVODE solver
 */

#ifndef _MPT_VODE_H
#define _MPT_VODE_H  @INTERFACE_VERSION@

#include "../solver.h"

#include <sys/uio.h>

__MPT_SOLVER_BEGIN

typedef void vode_fcn_t(int *, double *, double *, double *, double *, int *);
typedef void vode_jac_t(int *, double *, double *, int *, int *, double *, int *, double *, int *);

MPT_SOLVER_STRUCT(vode)
{
#ifdef __cplusplus
public:
	~vode();
	vode();
#endif
	MPT_SOLVER_IVP_STRUCT(parameters) ivp; /* inherit IVP parameter */
	
	double t;         /* reference time */
	
	MPT_SOLVER_TYPE(dvecpar) rtol, atol;  /* tolerances */
	
	double *y;        /* state data */
	
	char meth, miter, /* method flags */
	     jsv,         /* save jacobian iteration */
	     iopt;        /* flag for optional input */
	
	short istate;     /* dvode state */
	short itask;      /* type of integration step */
	
	struct iovec rwork, /* vode real work vector */
	             iwork; /* vode integer work vector */
	
	double *rpar;     /* real parameters */
	int    *ipar;     /* integer parameters */
	
	vode_fcn_t *fcn;  /* calculate right-hand side */
	vode_jac_t *jac;  /* user supplied jacobian */
};

__MPT_EXTDECL_BEGIN

/* c definition for vode fortran call */
extern void dvode_(vode_fcn_t *, int *, double *, double *, double *, int *, double *, double *,
                   int *, int *, int *, double *, int *, int *, int *,
                   vode_jac_t *, int *, double *, int *);

/* execute next step on supplied vode instance */
extern int mpt_vode_step(MPT_SOLVER_STRUCT(vode) *, double);

/* set vode parameter */
extern int mpt_vode_set(MPT_SOLVER_STRUCT(vode) *, const char *, const MPT_INTERFACE(metatype) *);
extern int mpt_vode_get(const MPT_SOLVER_STRUCT(vode) *, MPT_STRUCT(property) *);

/* validate settings and working space for use */
extern int mpt_vode_prepare(MPT_SOLVER_STRUCT(vode) *);

/* initialize/clear vode integrator descriptor */
extern void mpt_vode_init(MPT_SOLVER_STRUCT(vode) *);
extern void mpt_vode_fini(MPT_SOLVER_STRUCT(vode) *);
/* set wrapper for user functions */
extern int mpt_vode_ufcn(MPT_SOLVER_STRUCT(vode) *, const MPT_SOLVER_IVP_STRUCT(functions) *);

/* vode status information */
extern int mpt_vode_report(const MPT_SOLVER_STRUCT(vode) *, int , MPT_TYPE(PropertyHandler) , void *);

/* setup generic solver to use vode */
#ifndef __cplusplus
extern MPT_SOLVER(IVP) *mpt_vode_create();
#endif

__MPT_EXTDECL_END

#ifdef __cplusplus
inline vode::vode()
{ mpt_vode_init(this); }
inline vode::~vode()
{ mpt_vode_fini(this); }

class Vode : public IVP, vode
{
public:
	inline Vode()
	{
		_fcn.dae.param = &ivp;
	}
	virtual ~Vode()
	{ }
	void unref() __MPT_OVERRIDE
	{
		delete this;
	}
	int property(struct property *pr) const __MPT_OVERRIDE
	{
		return mpt_vode_get(this, pr);
	}
	int setProperty(const char *pr, const metatype *src = 0) __MPT_OVERRIDE
	{
		return mpt_vode_set(this, pr, src);
	}
	int report(int what, PropertyHandler out, void *opar) __MPT_OVERRIDE
	{
		return mpt_vode_report(this, what, out, opar);
	}
	int step(double *end) __MPT_OVERRIDE
	{
		if (!end) {
			int ret;
			if (!vode::fcn && _fcn.dae.fcn && (ret = mpt_vode_ufcn(this, &_fcn)) < 0) {
				return ret;
			}
			return mpt_vode_prepare(this);
		}
		int ret = mpt_vode_step(this, *end);
		*end = t;
		return ret;
	}
	void *functions(int type) __MPT_OVERRIDE
	{
		return (type == DAE) ? 0 : _fcn.select(type, ivp);
	}
protected:
	struct functions _fcn;
};
#endif

__MPT_SOLVER_END

#endif /* _MPT_VODE_H */

