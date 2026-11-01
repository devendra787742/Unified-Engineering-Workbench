#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <exception>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <mutex>
#include <numeric>
#include <queue>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace wb {
struct FEAResult { std::vector<double> x, T; double max_midpoint_error{}; };

FEAResult solve_thermal(int elements, double L=1.0, double A=1e-4,
                        double k=10.0, double q=10000.0,
                        double left=100.0, double right=20.0) {
    if (elements < 1 || !(L>0) || !(A>0) || !(k>0) || !std::isfinite(q))
        throw std::invalid_argument("FEA requires N>=1, positive L/A/k and finite q");
    const int n=elements+1; const double dx=L/elements;
    std::vector<std::vector<double>> K(n,std::vector<double>(n,0.0));
    std::vector<double> F(n,0.0);
    const double ke=k*A/dx, fe=q*A*dx/2.0;
    for(int e=0;e<elements;++e){
        K[e][e]+=ke; K[e][e+1]-=ke; K[e+1][e]-=ke; K[e+1][e+1]+=ke;
        F[e]+=fe; F[e+1]+=fe;
    }
    // Eliminate prescribed endpoint temperatures; solve only interior unknowns.
    const int m=elements-1; std::vector<std::vector<double>> M(m,std::vector<double>(m));
    std::vector<double> rhs(m);
    for(int i=0;i<m;++i){ int gi=i+1; rhs[i]=F[gi]-K[gi][0]*left-K[gi][elements]*right;
        for(int j=0;j<m;++j) M[i][j]=K[gi][j+1]; }
    // Partial-pivot Gaussian elimination.
    for(int c=0;c<m;++c){ int p=c; for(int r=c+1;r<m;++r) if(std::abs(M[r][c])>std::abs(M[p][c])) p=r;
        if(std::abs(M[p][c])<1e-18) throw std::runtime_error("Singular thermal system");
        std::swap(M[p],M[c]); std::swap(rhs[p],rhs[c]);
        double d=M[c][c]; for(int j=c;j<m;++j) M[c][j]/=d; rhs[c]/=d;
        for(int r=0;r<m;++r) if(r!=c){ double f=M[r][c]; for(int j=c;j<m;++j) M[r][j]-=f*M[c][j]; rhs[r]-=f*rhs[c]; }
    }
    FEAResult out; out.x.resize(n); out.T.resize(n); out.T.front()=left; out.T.back()=right;
    for(int i=1;i<elements;++i) out.T[i]=rhs[i-1];
    for(int i=0;i<n;++i) out.x[i]=i*dx;
    for(int e=0;e<elements;++e){ double xm=(e+0.5)*dx; double tf=(out.T[e]+out.T[e+1])/2.0;
        double exact=left+(right-left)*xm/L+(q/(2.0*k))*xm*(L-xm);
        out.max_midpoint_error=std::max(out.max_midpoint_error,std::abs(tf-exact)); }
    return out;
}

void write_fea_csv(const std::string& path,const FEAResult& r){
    std::ofstream f(path); if(!f) throw std::runtime_error("Cannot write "+path);
    f<<"x_m,temperature_C\n"<<std::setprecision(12);
    for(size_t i=0;i<r.x.size();++i) f<<r.x[i]<<","<<r.T[i]<<"\n";
}

struct Case { double k,q; int elements; };
std::vector<Case> make_cases(std::size_t count,int elements,unsigned seed){
    if(elements<1 || count==0) throw std::invalid_argument("case count and mesh size must be positive");
    std::mt19937 gen(seed); std::uniform_real_distribution<double> kd(5.0,25.0),qd(1000.0,20000.0);
    std::vector<Case> v; v.reserve(count); for(size_t i=0;i<count;++i)v.push_back({kd(gen),qd(gen),elements}); return v;
}
std::vector<FEAResult> run_batch(const std::vector<Case>& cases,unsigned workers){
    std::vector<FEAResult> out(cases.size()); std::atomic<size_t> next{0};
    workers=std::max(1u,std::min<unsigned>(workers,static_cast<unsigned>(cases.size())));
    std::vector<std::thread> pool; pool.reserve(workers);
    for(unsigned w=0;w<workers;++w) pool.emplace_back([&]{ for(;;){size_t i=next.fetch_add(1); if(i>=cases.size())break;
        const auto& c=cases[i]; out[i]=solve_thermal(c.elements,1.0,1e-4,c.k,c.q,100.0,20.0); }});
    for(auto& t:pool) t.join();
    return out;
}

double median(std::vector<double> v){std::sort(v.begin(),v.end()); return v[v.size()/2];}
void benchmark(std::size_t count,int mesh,unsigned seed){
    auto cases=make_cases(count,mesh,seed); const unsigned hw=std::max(1u,std::thread::hardware_concurrency());
    std::cout<<"logical_threads="<<hw<<" cases="<<count<<" elements_per_case="<<mesh<<" seed="<<seed<<"\n";
    std::vector<FEAResult> reference; std::vector<double> t1;
    (void)run_batch(cases,1); // warm-up excluded from measured repetitions
    for(int rep=0;rep<5;++rep){auto a=std::chrono::steady_clock::now(); reference=run_batch(cases,1); auto b=std::chrono::steady_clock::now(); t1.push_back(std::chrono::duration<double>(b-a).count());}
    double base=median(t1); std::cout<<"workers,median_seconds,speedup,efficiency,max_abs_difference_C\n";
    for(unsigned p: {1u,2u,3u,4u}){ if(p>hw)continue; std::vector<double> times; double diff=0;
        if(p==1){ times=t1; }
        else {
            (void)run_batch(cases,p); // per-worker-count warm-up
            for(int rep=0;rep<5;++rep){auto a=std::chrono::steady_clock::now();auto r=run_batch(cases,p);auto b=std::chrono::steady_clock::now();times.push_back(std::chrono::duration<double>(b-a).count());
                for(size_t i=0;i<r.size();++i)for(size_t j=0;j<r[i].T.size();++j)diff=std::max(diff,std::abs(r[i].T[j]-reference[i].T[j]));}
        }
        double tm=median(times);std::cout<<p<<","<<tm<<","<<base/tm<<","<<(base/tm)/p<<","<<diff<<"\n";
    }
}

struct Node { long long id; double x,y,z,t; };
std::vector<Node> read_temperature_csv(const std::string& path){
    std::ifstream f(path);if(!f)throw std::runtime_error("Cannot open CSV: "+path);std::string line;
    if(!std::getline(f,line))throw std::runtime_error("Empty CSV");
    if(line!="node_id,x_m,y_m,z_m,temperature_C")throw std::runtime_error("CSV header must be node_id,x_m,y_m,z_m,temperature_C");
    std::vector<Node> nodes;while(std::getline(f,line)){if(line.empty())continue;std::stringstream ss(line);std::string s;std::array<std::string,5>a;
        for(int i=0;i<5;++i){if(!std::getline(ss,a[i],','))throw std::runtime_error("Malformed CSV row");}
        if(std::getline(ss,s,','))throw std::runtime_error("Too many CSV columns");
        try{Node n{std::stoll(a[0]),std::stod(a[1]),std::stod(a[2]),std::stod(a[3]),std::stod(a[4])};
            if(!std::isfinite(n.x)||!std::isfinite(n.y)||!std::isfinite(n.z)||!std::isfinite(n.t)) throw std::runtime_error("Non-finite CSV value");
            nodes.push_back(n);
        }catch(const std::exception&){throw std::runtime_error("Invalid numeric CSV value");}}
    if(nodes.empty()) throw std::runtime_error("CSV contains header but no data rows");
    return nodes;
}
struct Triangle{double v[9];};
std::vector<Triangle> read_ascii_stl(const std::string& path){
    std::ifstream f(path);if(!f)throw std::runtime_error("Cannot open STL: "+path);std::string line;std::vector<Triangle> tris;Triangle cur{};int count=0;
    while(std::getline(f,line)){std::stringstream ss(line);std::string word;ss>>word;if(word=="vertex"){double x,y,z;if(!(ss>>x>>y>>z)||!std::isfinite(x)||!std::isfinite(y)||!std::isfinite(z))throw std::runtime_error("Malformed/non-finite STL vertex");
        cur.v[count*3]=x;cur.v[count*3+1]=y;cur.v[count*3+2]=z;if(++count==3){tris.push_back(cur);count=0;}}}
    if(count!=0) throw std::runtime_error("STL has incomplete triangle");
    if(tris.empty()) throw std::runtime_error("No ASCII STL triangles found");
    return tris;
}

struct Device {std::string type,name,voltage,protocol;bool connected=false;};
class MockAdapter {Device d;public:explicit MockAdapter(Device x):d(std::move(x)){} Device discover()const{return d;}
 bool connect(bool simulate_timeout=false){if(simulate_timeout)throw std::runtime_error("Recoverable timeout while connecting to "+d.name);d.connected=true;return true;}
 void disconnect(){d.connected=false;}std::string telemetry()const{if(!d.connected)throw std::runtime_error("Device is disconnected");return "{\"device\":\""+d.name+"\",\"status\":\"simulated_ok\",\"sample\":42}";}};

struct BP {double h,yhat,loss,dw,db,dv,dc,wn,bn,vn,cn;};
BP backprop(){const double x=1,y=2,w=.5,b=0,v=1,c=0,lr=.1;double h=std::tanh(w*x+b),yh=v*h+c,e=yh-y,loss=.5*e*e;
 double dc=e,dv=e*h,dz=e*v*(1-h*h),dw=dz*x,db=dz;
 return {h,yh,loss,dw,db,dv,dc,w-lr*dw,b-lr*db,v-lr*dv,c-lr*dc};}
double loss_fn(double w,double b,double v,double c){double h=std::tanh(w+b);double e=v*h+c-2;return .5*e*e;}
std::array<double,4> finite_difference_gradients(double eps=1e-6){std::array<double,4> g{};std::array<double,4> p{.5,0,1,0};for(int i=0;i<4;++i){auto a=p,b=p;a[i]+=eps;b[i]-=eps;g[i]=(loss_fn(a[0],a[1],a[2],a[3])-loss_fn(b[0],b[1],b[2],b[3]))/(2*eps);}return g;}
void backprop_csv(const std::string& path){std::ofstream f(path);f<<"step,loss\n"<<std::setprecision(12);double w=.5,b=0,v=1,c=0,lr=.1;
 for(int i=0;i<=100;++i){double h=std::tanh(w+b),e=v*h+c-2;f<<i<<","<<.5*e*e<<"\n";double dc=e,dv=e*h,dz=e*v*(1-h*h),dw=dz,db=dz;w-=lr*dw;b-=lr*db;v-=lr*dv;c-=lr*dc;}}
void usage(){std::cout<<"Unified Engineering Workbench (C++17)\nCommands:\n  fea <N> <csv>\n  batch <case_count> <mesh_elements> <seed>\n  import-csv <file>\n  import-stl <file>\n  hardware\n  backprop <loss_csv>\n";}
}
int main(int argc,char**argv){try{if(argc<2){wb::usage();return 0;}std::string cmd=argv[1];
 if(cmd=="fea"&&argc==4){int n=std::stoi(argv[2]);auto r=wb::solve_thermal(n);wb::write_fea_csv(argv[3],r);std::cout<<"N="<<n<<" max_midpoint_error_C="<<r.max_midpoint_error<<" output="<<argv[3]<<"\n";}
 else if(cmd=="batch"&&argc==5)wb::benchmark(std::stoull(argv[2]),std::stoi(argv[3]),static_cast<unsigned>(std::stoul(argv[4])));
 else if(cmd=="import-csv"&&argc==3){auto n=wb::read_temperature_csv(argv[2]);std::cout<<"Validated "<<n.size()<<" temperature nodes\n";}
 else if(cmd=="import-stl"&&argc==3){auto t=wb::read_ascii_stl(argv[2]);std::cout<<"Validated "<<t.size()<<" ASCII STL triangles\n";}
 else if(cmd=="hardware"){std::vector<wb::MockAdapter> ds;ds.emplace_back(wb::Device{"MCU","STM32 Nucleo mock","3.3 V","USB CDC"});ds.emplace_back(wb::Device{"FPGA","Basys 3 mock","3.3 V I/O","USB-JTAG"});ds.emplace_back(wb::Device{"JTAG","USB-Blaster-compatible mock","target-dependent","JTAG"});for(auto&d:ds){auto x=d.discover();std::cout<<x.type<<" | "<<x.name<<" | "<<x.voltage<<" | "<<x.protocol<<"\n";try{d.connect(x.type=="FPGA");std::cout<<d.telemetry()<<"\n";}catch(const std::exception&e){std::cout<<"RECOVERABLE: "<<e.what()<<"\n";d.disconnect();}}}
 else if(cmd=="backprop"&&argc==3){auto r=wb::backprop();wb::backprop_csv(argv[2]);auto fd=wb::finite_difference_gradients();std::cout<<std::setprecision(12)<<"h="<<r.h<<" yhat="<<r.yhat<<" loss="<<r.loss<<"\n"<<"gradients dw="<<r.dw<<" db="<<r.db<<" dv="<<r.dv<<" dc="<<r.dc<<"\n"<<"finite_difference dw="<<fd[0]<<" db="<<fd[1]<<" dv="<<fd[2]<<" dc="<<fd[3]<<"\n"<<"updated w="<<r.wn<<" b="<<r.bn<<" v="<<r.vn<<" c="<<r.cn<<"\n";}
 else{wb::usage();return 2;}return 0;}catch(const std::exception&e){std::cerr<<"ERROR: "<<e.what()<<"\n";return 1;}}
