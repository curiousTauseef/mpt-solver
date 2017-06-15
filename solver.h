/*!
 * MPT solver interfaces
 *  generic solver, loadable from library
 */

#ifndef _MPT_SOLVER_H
#define _MPT_SOLVER_H  @INTERFACE_VERSION@

#include "object.h"

#ifdef __cplusplus
# include <stdlib.h>
#endif

struct iovec;

struct rusage;
struct timeval;

__MPT_NAMESPACE_BEGIN

MPT_INTERFACE(config);
MPT_INTERFACE(client);
MPT_INTERFACE(output);
MPT_INTERFACE(metatype);
MPT_INTERFACE(iterator);

MPT_STRUCT(dispatch);
MPT_STRUCT(array);
MPT_STRUCT(event);
MPT_STRUCT(proxy);
MPT_STRUCT(node);

#ifdef _MPT_ARRAY_H
MPT_STRUCT(solver_data)
{
#ifdef __cplusplus
	inline solver_data() : npar(0), nval(0)
	{ }
#else
# define MPT_SOLVER_DATA_INIT { MPT_ARRAY_INIT, MPT_ARRAY_INIT,  0, 0 }
#endif
	_MPT_ARRAY_TYPE(double) param;  /* parameter matrix */
	_MPT_ARRAY_TYPE(double) val;    /* input/output (matrix) data */
	
	int npar;  /* leading dimension of parameter data */
	int nval;  /* length of value matrix row */
};

MPT_STRUCT(solver_output)
{
#ifdef __cplusplus
	Reference<output> data;
	Reference<output> graphic;
	Reference<logger> info;
protected:
#else
# define MPT_SOLVER_OUTPUT_INIT { 0, 0, 0, MPT_ARRAY_INIT }
	MPT_INTERFACE(output) *_data;
	MPT_INTERFACE(output) *_graphic;
	MPT_INTERFACE(logger) *_info;
#endif
	_MPT_ARRAY_TYPE(uint8_t) _pass;  /*  process flags for data dimensions */
};
#else
MPT_STRUCT(solver_data);
MPT_STRUCT(solver_output);
#endif /* _MPT_ARRAY_H */

#ifndef __cplusplus
# ifndef __MPT_IVP_RTOL
#  define __MPT_IVP_RTOL  1e-4
# endif
# ifndef __MPT_IVP_ATOL
#  define __MPT_IVP_ATOL  1e-4
# endif
# ifndef __MPT_NLS_TOL
#  define __MPT_NLS_TOL   1e-6
# endif
# define MPT_SOLVER_STRUCT(i) struct mpt_solver_##i
# define MPT_SOLVER_TYPE(i)   MptSolver##i
# define MPT_SOLVER_ENUM(i)   MPT_SOLVER_##i
# define __MPT_SOLVER_BEGIN   __MPT_NAMESPACE_BEGIN
# define __MPT_SOLVER_END     __MPT_NAMESPACE_END
# define MPT_SOLVER(i)        struct mpt_solver_##i

# define MPT_SOLVER_NLS_STRUCT(i)   struct mpt_solver_nls_##i
# define MPT_SOLVER_NLS(i)          MptSolverNls##i
# define _MPT_SOLVER_NLS_TYPEDEF(i) MptSolverNls##i

# define MPT_SOLVER_IVP_STRUCT(i)   struct mpt_solver_ivp_##i
# define MPT_SOLVER_IVP(i)          MptSolverIvp##i
# define _MPT_SOLVER_IVP_TYPEDEF(i) MptSolverIvp##i
#else
# define MPT_SOLVER_STRUCT(i) struct i
# define MPT_SOLVER_TYPE(i)   i
# define MPT_SOLVER_ENUM(i)   i
# define __MPT_SOLVER_BEGIN   __MPT_NAMESPACE_BEGIN namespace solver {
# define __MPT_SOLVER_END     } __MPT_NAMESPACE_END
# define MPT_SOLVER(i)        class i

# define MPT_SOLVER_NLS_STRUCT(i)   struct NLS::i
# define MPT_SOLVER_NLS(i)          NLS::i
# define _MPT_SOLVER_NLS_TYPEDEF(i) i

# define MPT_SOLVER_IVP_STRUCT(i)   struct IVP::i
# define MPT_SOLVER_IVP(i)          IVP::i
# define _MPT_SOLVER_IVP_TYPEDEF(i) i

namespace solver {

#endif

enum MPT_SOLVER_ENUM(Flags)
{
	
	MPT_SOLVER_ENUM(CapableNls) = 0x0f,
	MPT_SOLVER_ENUM(NlsVector)  = 0x01,
	MPT_SOLVER_ENUM(NlsOverDet) = 0x02,
	
	MPT_SOLVER_ENUM(CapableIvp) = 0xf0,
	MPT_SOLVER_ENUM(ODE)        = 0x10,
	MPT_SOLVER_ENUM(DAE)        = 0x20,
	MPT_SOLVER_ENUM(PDE)        = 0x40,
	MPT_SOLVER_ENUM(IVP)        = 0x70,
	
	MPT_SOLVER_ENUM(Header)     = 0x1,
	MPT_SOLVER_ENUM(Status)     = 0x2,
	MPT_SOLVER_ENUM(Report)     = 0x4,
	MPT_SOLVER_ENUM(Values)     = 0x8
};

/*! generic solver interface */
#ifdef __cplusplus
class generic : public object
{
protected:
	inline ~generic() {}
public:
	enum { Type = TypeSolver };
	
	virtual int report(int what, PropertyHandler out, void *opar) = 0;
};
#else
MPT_SOLVER(generic);
MPT_INTERFACE_VPTR(solver)
{
	/* metatype operations */
	MPT_INTERFACE_VPTR(object) obj;
	/* call output function with data to report for type */
	int (*report)(MPT_SOLVER(generic) *, int , MPT_TYPE(PropertyHandler) , void *opar);
};
MPT_SOLVER(generic)
{ const MPT_INTERFACE_VPTR(solver) *_vptr; };

MPT_SOLVER_IVP_STRUCT(parameters);
#endif

/*! extension for Initial Value Problems */
#ifdef __cplusplus
struct odefcn;
struct daefcn;
struct pdefcn;

class IVP : public generic
{
protected:
	inline ~IVP() {}
public:
	
	virtual int step(double *end) = 0;
	virtual void *functions(int);
	virtual double *initstate();
	
	struct parameters;
	struct functions;
	
	inline operator struct odefcn *();
	inline operator struct daefcn *();
	inline operator struct pdefcn *();
#endif
/* ODE solver functions */
typedef int (*_MPT_SOLVER_IVP_TYPEDEF(Fcn))(void *upar, double t, const double *y, double *f);
typedef int (*_MPT_SOLVER_IVP_TYPEDEF(Jac))(void *upar, double t, const double *y, double *jac, int ldjac);
/* extension for DAE solvers */
typedef int (*_MPT_SOLVER_IVP_TYPEDEF(Mas))(void *upar, double t, const double *y, double *band, int *ir, int *ic);
/* extensions for PDE solvers */
typedef int (*_MPT_SOLVER_IVP_TYPEDEF(Rside))(void *upar, double t, const double *y, double *f, double x, double *dx, double *dy);
typedef int (*_MPT_SOLVER_IVP_TYPEDEF(Pde))(void *upar, double t, const double *y, double *f, const MPT_SOLVER_IVP_STRUCT(parameters) *, const double *x, _MPT_SOLVER_IVP_TYPEDEF(Rside));
#ifdef __cplusplus
};
inline void *IVP::functions(int)
{ return 0; }
inline double *IVP::initstate()
{ return 0; }
#endif

/*! generic IVP functions */
MPT_SOLVER_STRUCT(odefcn)
{
#ifdef __cplusplus
	enum { Type = ODE };
	
	inline odefcn(IVP::Fcn f, IVP::Jac j = 0) : fcn(f), param(0), jac(j)
	{ }
#endif
	MPT_SOLVER_IVP(Fcn) fcn;   /* unified right side calculation */
	void               *param; /* user parameter for handler functions */
	MPT_SOLVER_IVP(Jac) jac;   /* (full/banded) jacobi matrix */
};
MPT_SOLVER_STRUCT(daefcn)
{
#ifdef __cplusplus
	enum { Type = DAE };
	
	inline daefcn(IVP::Fcn f, IVP::Mas m, IVP::Jac j = 0) : fcn(f), param(0), jac(j), mas(m)
	{ }
#endif
	MPT_SOLVER_IVP(Fcn)  fcn;   /* unified right side calculation */
	void                *param; /* user parameter for handler functions */
	MPT_SOLVER_IVP(Jac)  jac;   /* (full/banded) jacobi matrix */
	MPT_SOLVER_IVP(Mas)  mas;   /* (sparse) mass matrix */
};
MPT_SOLVER_STRUCT(pdefcn)
{
#ifdef __cplusplus
	enum { Type = PDE };
	
	inline pdefcn(IVP::Pde f, IVP::Rside r = 0) : rside(r), grid(0), fcn(f), param(0)
	{ }
#endif
	MPT_SOLVER_IVP(Rside) rside; /* per node right side */
	double               *grid;  /* solver grid data */
	MPT_SOLVER_IVP(Pde)   fcn;   /* right side function */
	void                 *param; /* user parameter for handler functions */
};

/*! general IVP parameter */
MPT_SOLVER_IVP_STRUCT(parameters)
{
#ifdef __cplusplus
	inline parameters() : neqs(0), pint(0)
	{ }
#else
# define MPT_IVPPAR_INIT(d)  ((d)->neqs = 0, (d)->pint = 0)
#endif
	int32_t neqs,  /* number of equotations */
	        pint;  /* number pde intervals */
};
/*! initial value problem functions */
MPT_SOLVER_IVP_STRUCT(functions)
{
#ifdef __cplusplus
	enum { Type = mpt::solver::IVP };
	
	inline functions(void *par = 0) : rside(0), grid(0), dae(0, 0)
	{
		dae.param = par;
	}
	void *select(int type, const struct parameters &par)
	{
		switch (type) {
		  case odefcn::Type: if (par.pint) return 0; dae.mas = 0; return &dae;
		  case daefcn::Type: return par.pint ? 0 : &dae;
		  case pdefcn::Type: if (!par.pint) return 0; dae.jac = 0; dae.mas = 0;
		  case Type: return this;
		  default: return 0;
		}
	}
#else
# define MPT_IVPFCN_INIT(d)  ((MPT_SOLVER_IVP_STRUCT(functions) *) memset(d, 0, sizeof(MPT_SOLVER_IVP_STRUCT(functions))))
#endif
	MPT_SOLVER_IVP(Rside)     rside; /* right side function */
	double                   *grid;  /* solver grid data */
	MPT_SOLVER_STRUCT(daefcn) dae;
};

#ifdef __cplusplus
inline IVP::operator struct odefcn *()
{ return reinterpret_cast<odefcn *>(functions(odefcn::Type)); }
inline IVP::operator struct daefcn *()
{ return reinterpret_cast<daefcn *>(functions(daefcn::Type)); }
inline IVP::operator struct pdefcn *()
{ return reinterpret_cast<pdefcn *>(functions(pdefcn::Type)); }

/*! extension for NonLinear Systems */
class NLS : public generic
{
protected:
	inline ~NLS() {}
    public:
	struct parameters;
	struct functions;
	
	virtual int solve() = 0;
	
	virtual operator functions *();
	
#endif
typedef int (*_MPT_SOLVER_NLS_TYPEDEF(Fcn))(void *rpar, const double *p, double *res, const int *lres);
typedef int (*_MPT_SOLVER_NLS_TYPEDEF(Jac))(void *jpar, const double *p, double *jac, const int *ld, const double *res);
typedef int (*_MPT_SOLVER_NLS_TYPEDEF(Out))(void *opar, const MPT_STRUCT(value) *val);
#ifdef __cplusplus
};
inline NLS::operator NLS::functions *()
{ return 0; }
#else
MPT_SOLVER(NLS);
MPT_INTERFACE_VPTR(solver_nls)
{
	MPT_INTERFACE_VPTR(solver) gen;
	int (*solve)(MPT_SOLVER(NLS) *s);
	MPT_SOLVER_NLS_STRUCT(functions) *(*functions)(MPT_SOLVER(NLS) *);
};
MPT_SOLVER(NLS)
{ const MPT_INTERFACE_VPTR(solver_nls) *_vptr; };
#endif

/*! general nonlinear system parameter */
MPT_SOLVER_NLS_STRUCT(parameters)
{
#ifdef __cplusplus
	inline parameters() : nval(0), nres(0)
	{ }
#else
# define MPT_NLSPAR_INIT(d)  ((d)->nres = (d)->nval = 0)
#endif
	int32_t nval,  /* parameters to optimize */
	        nres;  /* number of residuals */
};
/*! generic nonlinear system functions */
MPT_SOLVER_NLS_STRUCT(functions)
{
#ifdef __cplusplus
	inline functions(Fcn f, void *u = 0) : res(f), jac(0), rpar(u), jpar(0), out(0), opar(0)
	{ }
#else
# define MPT_NLSFCN_INIT(d)  ((MPT_SOLVER_NLS_STRUCT(functions) *) memset(d, 0, sizeof(MPT_SOLVER_NLS_STRUCT(functions))))
#endif
	MPT_SOLVER_NLS(Fcn) res;  /* calculate residuals */
	MPT_SOLVER_NLS(Jac) jac;  /* calculate residual jacobian */
	void *rpar, *jpar;        /* residual/jacobian function parameter */
	
	MPT_SOLVER_NLS(Out) out;  /* output current parameter/residuals */
	void *opar;               /* output function parameter */
};

#ifdef __cplusplus
template <typename T>
struct vecpar
{
	typedef T* iterator;
	
	inline vecpar(T const &val) : base(0)
	{ d.val = val; }
	inline vecpar() : base(0)
	{ d.val = T(); }
	inline ~vecpar()
	{ if (base) resize(0); }
	
	inline iterator begin() const
	{ return base ? base : const_cast<T *>(&d.val); }
	
	 inline iterator end() const
	{ return base ? base + d.len/sizeof(*base) : const_cast<T *>(&d.val) + 1; }
	
	inline Slice<const T> data() const
	{
		if (base) return Slice<const T>(base, d.len / sizeof(T));
		return Slice<const T>(&d.val, 1);
	}
	inline int type()
	{ return typeIdentifier<T>(); }
	
	bool resize(size_t elem) {
		T *t = base;
		size_t l = t ? d.len/sizeof(*base) : 0;
		
		if (l == elem) return true; /* noop */
		
		/* save/destruct vector data */
		if (!elem && t) d.val = t[0]; /* vector>scalar transition */
		for (size_t i = elem; i < l; ++i) t[i].~T();
		
		/* free/extend memory */
		if (!elem) { free(t); t = 0; }
		else if (!(t = static_cast<T *>(realloc(t, elem * sizeof(*base))))) return false;
		
		/* initialize new elements */
		for (T &val = base ? t[l - 1] : d.val; l < elem; ++l) t[l] = val;
		
		/* save new data */
		if (elem) {
			if (!base) d.val.~T(); /* scalar>vector transition */
			d.len = elem * sizeof(*base);
		}
		base = t;
		
		return true;
	}
	T &operator [](int pos)
	{
		static T def;
		int len = base ? d.len / sizeof(T) : 1;
		if (pos < 0) pos += len;
		if (!pos && !base) return d.val;
		return (pos >= 0 && pos < len) ? base[pos] : def;
	}
	struct value value() const
	{
		if (base) {
			static const char fmt[2] = { static_cast<char>(vectorIdentifier<T>()), 0 };
			return ::mpt::value(fmt, this);
		} else {
			static const char fmt[2] = { static_cast<char>(typeIdentifier<T>()), 0 };
			return ::mpt::value(fmt, &d.val);
		}
	}
protected:
	T *base;
	union {
		size_t len;
		T val;
	} d;
};

# if __cplusplus >= 201103L
using dvecpar = vecpar<double>;
using ivecpar = vecpar<int>;
# else
typedef vecpar<double> dvecpar;
typedef vecpar<int> ivecpar;
# endif
#else
# define MPT_VECPAR_INIT(p,v)  ((p)->base = 0, (p)->d.val = (v))
typedef struct
{
	double *base;
	union { size_t len; double val; } d;
} MPT_SOLVER_TYPE(dvecpar);
typedef struct
{
	int *base;
	union { size_t len; int val; } d;
} MPT_SOLVER_TYPE(ivecpar);

MPT_SOLVER(IVP);
MPT_INTERFACE_VPTR(solver_ivp)
{
	MPT_INTERFACE_VPTR(solver) gen;
	int (*step)(MPT_SOLVER(IVP) *, double *end);
	void *(*functions)(MPT_SOLVER(IVP) *, int);
	double *(*initstate)(MPT_SOLVER(IVP) *);
};
MPT_SOLVER(IVP)
{ const MPT_INTERFACE_VPTR(solver_ivp) *_vptr; };
#endif

__MPT_EXTDECL_BEGIN


/* create interface to solver */
extern MPT_INTERFACE(client) *mpt_client_ivp(MPT_INTERFACE(output) *, int (*)(MPT_SOLVER(IVP) *, MPT_STRUCT(solver_data) *, MPT_INTERFACE(logger) *));
extern MPT_INTERFACE(client) *mpt_client_nls(MPT_INTERFACE(output) *, int (*)(MPT_SOLVER(NLS) *, MPT_STRUCT(solver_data) *, MPT_INTERFACE(logger) *));


/* initialize IVP solver states */
extern int mpt_init_ivp(MPT_SOLVER(IVP) *, double , int , const double *, MPT_INTERFACE(logger) *__MPT_DEFPAR(logger::defaultInstance()));

extern MPT_SOLVER_STRUCT(daefcn) *mpt_init_dae(MPT_SOLVER(IVP) *, const MPT_STRUCT(array) *, MPT_INTERFACE(logger) *__MPT_DEFPAR(logger::defaultInstance()));
extern MPT_SOLVER_STRUCT(odefcn) *mpt_init_ode(MPT_SOLVER(IVP) *, const MPT_STRUCT(array) *, MPT_INTERFACE(logger) *__MPT_DEFPAR(logger::defaultInstance()));
extern MPT_SOLVER_STRUCT(pdefcn) *mpt_init_pde(MPT_SOLVER(IVP) *, int , int , MPT_INTERFACE(logger) *__MPT_DEFPAR(logger::defaultInstance()));

/* initialize NLS solver states */
extern MPT_SOLVER_NLS_STRUCT(functions) *mpt_init_nls(MPT_SOLVER(NLS) *, const MPT_STRUCT(solver_data) *, MPT_INTERFACE(logger) *__MPT_DEFPAR(logger::defaultInstance()));


/* register events on notifier */
extern int mpt_solver_events(MPT_STRUCT(dispatch) *, MPT_INTERFACE(client) *);

/* set solver (user data and solver parameter) config files */
extern int mpt_solver_config(MPT_INTERFACE(config) *, MPT_STRUCT(event) *);
/* read files, initialize and prepare solver, set default handler */
extern int mpt_solver_start(MPT_INTERFACE(client) *, MPT_STRUCT(event) *);
/* set solver command line arguments */
extern int mpt_solver_args(MPT_INTERFACE(config) *, char * const [], int __MPT_DEFPAR(-1));

/* read files to configuration */
extern int mpt_solver_read(MPT_STRUCT(node) *, MPT_INTERFACE(iterator) *, MPT_INTERFACE(logger) *);

/* initialize solver memory */
extern void mpt_solver_data_fini(MPT_STRUCT(solver_data) *);
extern void mpt_solver_data_clear(MPT_STRUCT(solver_data) *);

/* copy state to solver data */
extern int mpt_solver_data_nls(MPT_STRUCT(solver_data) *, const MPT_STRUCT(value) *);

/* add usage time difference */
extern void mpt_timeradd_sys(struct timeval *, const struct rusage *, const struct rusage *);
extern void mpt_timeradd_usr(struct timeval *, const struct rusage *, const struct rusage *);


/* set problem specific parameters */
extern int mpt_conf_nls(MPT_STRUCT(solver_data) *, const MPT_STRUCT(node) *, MPT_INTERFACE(logger) *__MPT_DEFPAR(logger::defaultInstance()));
extern int mpt_conf_pde(MPT_STRUCT(solver_data) *, const MPT_STRUCT(node) *, MPT_INTERFACE(logger) *__MPT_DEFPAR(logger::defaultInstance()));
extern int mpt_conf_ode(MPT_STRUCT(solver_data) *, double , const MPT_STRUCT(node) *, MPT_INTERFACE(logger) *__MPT_DEFPAR(logger::defaultInstance()));

/* configure graphic output and bindings */
extern int mpt_conf_graphic(MPT_INTERFACE(output) *, const MPT_STRUCT(node) *);
/* configure history output and format */
extern int mpt_conf_history(MPT_INTERFACE(output) *, const MPT_STRUCT(node) *);

/* create profile data */
extern int mpt_conf_profiles(int , double *, int , const MPT_STRUCT(node) *, const double *, MPT_INTERFACE(logger) *);
extern int mpt_conf_profile (int , double *, int , const MPT_STRUCT(node) *, const double *);

/* append user data */
extern int mpt_conf_param(MPT_STRUCT(array) *, const MPT_STRUCT(node) *, int);
/* set grid data */
extern int mpt_conf_grid(MPT_STRUCT(array) *, const MPT_STRUCT(node) *);


/* get/initialize solver data parameters */
extern double *mpt_solver_data_grid (MPT_STRUCT(solver_data) *);
extern double *mpt_solver_data_param(MPT_STRUCT(solver_data) *);

/* execute specific IVP solver step */
extern int mpt_steps_ode(MPT_SOLVER(IVP) *, MPT_INTERFACE(iterator) *, MPT_STRUCT(solver_data) *, MPT_INTERFACE(logger) *__MPT_DEFPAR(logger::defaultInstance()));

/* inner node residuals with central differences */
extern int mpt_residuals_cdiff(void *, double , const double *, double *, const MPT_SOLVER_IVP_STRUCT(parameters) *, const double *, MPT_SOLVER_IVP(Rside));

/* generate library description form short form */
extern const char *mpt_solver_alias(const char *);
/* load solver of specific type */
extern MPT_SOLVER(generic) *mpt_solver_load(MPT_STRUCT(proxy) *, int , const char *, MPT_INTERFACE(logger) *);

/* set solver parameter */
extern int  mpt_solver_pset (MPT_INTERFACE(object) *, const MPT_STRUCT(node) *, int , MPT_INTERFACE(logger) *__MPT_DEFPAR(logger::defaultInstance()));
extern void mpt_solver_param(MPT_INTERFACE(object) *, const MPT_STRUCT(node) *, MPT_INTERFACE(metatype) *, MPT_INTERFACE(logger) *__MPT_DEFPAR(logger::defaultInstance()));

/* generic solver output */
extern int mpt_solver_info  (MPT_SOLVER(generic) *, MPT_INTERFACE(logger) *__MPT_DEFPAR(logger::defaultInstance()));
extern int mpt_solver_status(MPT_SOLVER(generic) *, MPT_INTERFACE(logger) *__MPT_DEFPAR(logger::defaultInstance()), int (*)(void *, const MPT_STRUCT(value) *) __MPT_DEFPAR(0), void *__MPT_DEFPAR(0));
extern int mpt_solver_report(MPT_SOLVER(generic) *, MPT_INTERFACE(logger) *__MPT_DEFPAR(logger::defaultInstance()));
/* final solver statistics */
extern void mpt_solver_statistics(MPT_SOLVER(generic) *, MPT_INTERFACE(logger) *, const struct timeval *, const struct timeval *);


/* output for solvers */
extern int mpt_solver_output_nls(const MPT_STRUCT(solver_output) *, int , const MPT_STRUCT(value) *, const MPT_STRUCT(solver_data) *);
extern int mpt_solver_output_pde(const MPT_STRUCT(solver_output) *, int , const MPT_STRUCT(value) *, const MPT_STRUCT(solver_data) *);
extern int mpt_solver_output_ode(const MPT_STRUCT(solver_output) *, int , const MPT_STRUCT(solver_data) *);
extern int mpt_output_solver_data(MPT_INTERFACE(output) *, int , int , int , const double *, int);
extern int mpt_output_ivp_header(MPT_INTERFACE(output) *, int , int , const double *);
extern int mpt_output_solver_history(MPT_INTERFACE(output) *, const double *, int , const double *, int);

/* solver module data management */
extern void *mpt_vecpar_alloc(struct iovec *, size_t len, size_t size);

extern int mpt_vecpar_settol(MPT_SOLVER_TYPE(dvecpar) *, MPT_INTERFACE(metatype) *, double def);
extern int mpt_vecpar_cktol(MPT_SOLVER_TYPE(dvecpar) *, int len, int repeat, double def);

extern int mpt_vecpar_get(const MPT_SOLVER_TYPE(dvecpar) *, MPT_STRUCT(value) *);
extern int mpt_vecpar_set(double **, int , MPT_INTERFACE(metatype) *);

extern int mpt_ivppar_set(MPT_SOLVER_IVP_STRUCT(parameters) *, MPT_INTERFACE(metatype) *);
extern int mpt_nlspar_set(MPT_SOLVER_NLS_STRUCT(parameters) *, MPT_INTERFACE(metatype) *);


__MPT_EXTDECL_END

#ifdef __cplusplus
} /* namespace solver */
#endif

__MPT_NAMESPACE_END

#endif /* _MPT_SOLVER_H */
