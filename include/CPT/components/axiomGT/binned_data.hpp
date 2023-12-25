#pragma once
#include <CPT/range.hpp>
namespace cpt {
namespace component {
namespace axiomGT {
RANGE_NAMESPACE_SHORTCUT
namespace mg = mlpack::gmm;
namespace cagu = ca::g_m_m_util;
class binned_data{
  public:
    template<typename T>
    void dump(const std::vector<T>& vec) {
        for(auto& e: vec) {std::cout << e << "\t";}
        std::cout << std::endl;
    }
    void dump() {
        std::cout << "length (data size): "                      << length << std::endl;
        std::cout << "NS (Total number of partition position): " << NS     << std::endl;
        std::cout << "ZS (Total bins number (data -> bins)):   " << ZS     << std::endl;
        std::cout << "nz: " << std::endl;   dump(nz);
        std::cout << "zz: " << std::endl;   dump(zz);
        std::cout << "zzx: " << std::endl;  dump(zzx);
        std::cout << "zy: " << std::endl;   dump(zy);
        std::cout << "zzy: " << std::endl;  dump(zzy);
        std::cout << "zxy: " << std::endl;  dump(zxy);
        std::cout << "nod: " << std::endl;  dump(nod);
        std::cout << "noh: " << std::endl;  dump(noh);
        std::cout << "noc: " << std::endl;  dump(noc);
        std::cout << "ec: " << std::endl;   dump(ec);
        std::cout << "ed: " << std::endl;   dump(ed);
        std::cout << "eh: " << std::endl;   dump(eh);
    }
    // length of data
	int length;  // data size
	int NS;      // data size + 1 => Total number of partition position
	int ZS;      // Total bins number (data -> bins)
	//bin summaries
	std::vector<double> nz; ///< number of things in each bin
	std::vector<double> zz; ///< sum of things in each bin
	std::vector<double> zzx; ///< sum of squares in each bin
	// bin summaries y
	std::vector<double> zy; ///< sum of y values in each bin
	std::vector<double> zzy; ///< sum of squares of y values in each bin
	// cross terms
	std::vector<double> zxy; ///< sum of x*y values in each bin
	// hints: counts of genotypes per bin
	std::vector<double> nod; ///< penalty*number of hint genotypes not d
	std::vector<double> noh; ///< penalty*number of hint genotypes not h
	std::vector<double> noc; ///< penalty*number of hint genotypes not c
	// soft genotypes by bin
	std::vector<double> ec;
	std::vector<double> ed;
	std::vector<double> eh;
};

struct second_less : public std::binary_function<std::pair<int,double>, std::pair<int,double>,bool>
{
    bool operator()(const std::pair<int,double>& x, 
                    const std::pair<int,double>& y) const
    {
        return(x.second < y.second);
    }
};
}}}
