/*!
 * check solver interfaces
 */

#include <iostream>

#include <mpt/solver.h>

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

using namespace mpt;
using namespace mpt::solver;

int main()
{
	mtrace();
	
	dvecpar vd(7.6);
	std::cout << vd.value().fmt << ": " << vd.data() << std::endl;
	
	vd.resize(8);
	std::cout << vd.value().fmt << ": " << vd.data() << std::endl;
	
	vecpar<short> vi;
	vi[0] = 4;
	std::cout << vi.value().fmt << ": " << *vi.data().base() << std::endl;
	vi.resize(8);
	std::cout << "vi[5] = " << vi[5] << std::endl;
	
	vi[3] = 9;
	std::cout << "vi = " << vi.data() << std::endl;
	
	return 0;
}