#define main workbench_cli_main
#include "../src/workbench.cpp"
#undef main
#include <cassert>
int main(){
 auto a=wb::solve_thermal(10),b=wb::solve_thermal(20),c=wb::solve_thermal(40),d=wb::solve_thermal(80);
 assert(std::abs(a.max_midpoint_error-1.25)<1e-8);
 assert(std::abs(b.max_midpoint_error-0.3125)<1e-8);
 assert(std::abs(c.max_midpoint_error-0.078125)<1e-8);
 assert(std::abs(d.max_midpoint_error-0.01953125)<1e-8);
 assert(a.T.front()==100 && a.T.back()==20);
 auto cases=wb::make_cases(8,10,123);auto s=wb::run_batch(cases,1),p=wb::run_batch(cases,4);
 for(size_t i=0;i<s.size();++i)for(size_t j=0;j<s[i].T.size();++j)assert(std::abs(s[i].T[j]-p[i].T[j])<=1e-8);
 auto r=wb::backprop();assert(std::abs(r.h-0.46211715726000974)<1e-12);
 auto fd=wb::finite_difference_gradients();assert(std::abs(fd[0]-r.dw)<1e-5 && std::abs(fd[1]-r.db)<1e-5 && std::abs(fd[2]-r.dv)<1e-5 && std::abs(fd[3]-r.dc)<1e-5);
 assert(r.dw<0 && r.db<0 && r.dv<0 && r.dc<0);
 bool rejected=false;try{wb::solve_thermal(0);}catch(...){rejected=true;}assert(rejected);
 return 0;
}
