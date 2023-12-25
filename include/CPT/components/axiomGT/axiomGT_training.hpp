#pragma once
#ifdef NEW_DATA_POOL
#include <CPT/engine/components/named_component.hpp>
#include <CPT/logger.hpp>
// #include <vector>
#include <CPT/utility/allele_signals_loader.hpp>
#include <CPT/engine/data_pool/shared_data_manager.hpp>
// #include <boost/range/adaptor/transformed.hpp>
// #include <boost_addon/range_eval.hpp>
// #include <boost_addon/range_sink.hpp>
#include <CPT/algorithm/probeset_rnd_choice.hpp>
// #include <Nucleona/container/vector.hpp>
// #include <boost/range/adaptor/indexed.hpp>
// #include <boost_addon/range_filter.hpp>
// #include <boost_addon/range_vector.hpp>
// #include <boost/range/adaptor/filtered.hpp>
// #include <boost_addon/range_indexed.hpp>
// #include <boost/range/irange.hpp>
// #include <boost_addon/range_vector.hpp>
// #include <boost/range/adaptor/filtered.hpp>
// #include <boost_addon/range_eval.hpp>
// #include <boost_addon/flattened.hpp>

// #include <boost_addon/range_filter.hpp>
// #include <CPT/algorithm/paralleled.hpp>
// #include <CPT/utility/gprofiler.hpp>
// #include <CPT/utility.hpp>
#include <CPT/format/special_SNP.hpp>
#include <CPT/components/axiomGT/probeset_models.hpp>
#include <CPT/components/axiomGT/binned_data.hpp>
// #include <CPT/utility/thread_task_time_reporter.hpp>

extern "C" void openblas_set_num_threads( int thread_num );

namespace cpt {
namespace component {
namespace axiomGT {
namespace cf = cpt::format;
namespace cu = cpt::utility;
namespace ced = cpt::engine::data_pool;
namespace ba = boost::adaptors;
namespace ca = cpt::algorithm;
namespace md = mlpack::distribution;
namespace mg = mlpack::gmm;
namespace bt = boost;
namespace cam = ca::matrix_range;

namespace axiomGT_training_detail {
    using AxiomGTTrainingBase = engine::NamedComponent;
}
namespace com_ = cpt::engine::data_pool::component_object_manager;
class ProbesetTraining 
: public axiomGT_training_detail::AxiomGTTrainingBase
{
    static constexpr auto& tlog   = cpt::logout6;
    using GMM                     = mlpack::gmm::GMM;
    using Base                    = axiomGT_training_detail::AxiomGTTrainingBase;
    using DataPoolType            = typename Base::DataPoolType;
    using RndIds                  = typename ca::ProbesetRndChoice::RndIds;
    using GMean                   = arma::vec;
    using GMMean                  = std::vector<GMean>;
    using SizeRow                 = arma::Row<std::size_t>;
    using SpecialSNPsTable        = cf::SpecialSNPsHelper::SpecialSNPsTable;
  public:
    static constexpr auto& logger = cpt::logout5;
  private:

    std::vector<std::string>                    probeset_names                         ;
    std::vector<std::string>                    sample_names                           ;

    /* input */
    com_::ICPObjPtr<std::string>                axiomGT_prefix_                        ;
    com_::ICPObjPtr<std::string>                probeset_priors_ipath_                 ;  // axiomGT_probeset_priors

    /* output */
    com_::ICPObjPtr<std::string>                probeset_posteriors_opath_             ;
    bool                                        is_posterior_                          ;

  public:
    std::size_t                                 select_probeset_num                    ;
    // ced::Shared<GMM>                            grand_model                            ;
    // com_::ICPObjPtr<GMM>                        grand_model_                           ;
    com_::ICPObjPtr<
        cf::Cube<double>
    >                                           probeset_cube_                         ;
    com_::ICPObjPtr<
        cf::Cube< double > 
    >                                           specialsnps_info_cube_                 ;    
    com_::ICPObjPtr<
        cf::Cube< double > 
    >                                           sample_specialsnps_cn_cube_            ;
    com_::ICPObjPtr<
        cf::Cube< double > 
    >                                           sample_genohints_cube_                 ;
    std::size_t                                 thread_num_                            ;
    std::set<std::string>                       dbg_view_list                          ;
    std::string                                 archive                                ;
    std::string                                 fitting_algo_name                      ;
    bpt::ptree                                  algo_params                            ;
    cpt::component::axiomGT::probeset_param     analysis_params                        ;

    template< class... T >
    ProbesetTraining(
          const DataPoolType& data_pool
        , const T&... o
    )
    : Base          ( data_pool, o... )
    {}

    /**
     * tunes up the prior to allow for cluster shifts
     *
     */
    void apply_wobble(cpt::component::axiomGT::probeset_param& sp)
    {
        // apply "wobble" to account for possible cluster shifts
        // i.e. even if infinite amount of prior data
        // this experimental set could be different
        //
        // // change to "f" here to preserve prior relative frequency after changing k
        //
        // note that this is currently "undone" in the actual 1-D seeding routine to preserve continuity of behavior
        // flag will be installed to allow better behavior
        
        double tmp;
        tmp = 1 / sp.wobble;
        if (tmp < sp.prior.aa.k)
            sp.prior.aa.k = tmp;
        if (tmp < sp.prior.ab.k)
            sp.prior.ab.k = tmp;
        if (tmp < sp.prior.bb.k)
            sp.prior.bb.k = tmp;
        // note that the posterior will reflect the "lower" preknowledge
        // of the prior data
    }

    int setup_bins(cpt::component::axiomGT::binned_data&               zall, 
                   const std::vector<double>&                          z,
                   const std::vector<double>&                          w,
                   const int&                                          bins,
                   const std::vector<double>&                          zeroInbred,
                   const std::vector<int>&                             zerohints,
                   const int&                                          hok,
                   const double&                                       Cpenalty)
    {
        auto& zlength = zall.length;

        double delta, boundary;
        int ti;

        delta = (z[zlength-1] - z[0]) / (bins + 1);

        zall.zz[0]  = 0;
        zall.zzx[0] = 0;
        zall.zy[0]  = 0;
        zall.zzy[0] = 0;
        zall.zxy[0] = 0;
        zall.nz[0]  = 0;
        zall.nod[0] = 0;
        zall.noh[0] = 0;    
        zall.noc[0] = 0;
        ti = 0;
        boundary = z[0] - 1;
        for (int i = 0; i < zlength; i++)
        {
            // when do we go to the next thing?
            // when a) when point outside domain
            // b) bins is turned off
            // c) when fewer points than bins
            if (z[i] > boundary || bins == 0 || zlength < bins)
            {
                ti++;
                boundary = z[i] + delta;
                // initialize the new bin
                zall.nz[ti]  = 0;
                zall.zz[ti]  = 0;
                zall.zzx[ti] = 0;
                zall.zy[ti]  = 0;
                zall.zzy[ti] = 0;
                zall.zxy[ti] = 0;
                // hints
                zall.nod[ti] = 0;    
                zall.noh[ti] = 0;
                zall.noc[ti] = 0;
            }
            // add current point to bins
            zall.nz[ti]  += 1;
            zall.zz[ti]  += z[i];
            zall.zzx[ti] += z[i] * z[i];
            zall.zy[ti]  += w[i];
            zall.zzy[ti] += w[i] * w[i];
            zall.zxy[ti] += z[i] * w[i];
            // count contradictions
            // if no information, no contradictions
            if (zerohints[i] == 0)
            {
                if (hok < 1)
                    zall.nod[ti] += Cpenalty;
                zall.noh[ti] += Cpenalty;
            }
            if (zerohints[i] == 1)
            {
                zall.nod[ti] += Cpenalty;
                zall.noc[ti] += Cpenalty;
            }
            if (zerohints[i] == 2)
            {
                zall.noh[ti] += Cpenalty;
                if (hok < 1)
                    zall.noc[ti] += Cpenalty;
            }
            // bin ti contains item [i] which has a bias for or against hets by being inbred.
            zall.noh[ti] += zeroInbred[i];
        }
        
        // ti indicates last point here
        // how much did we actually use
        return(ti + 1);
    }

    void initialize_bins(cpt::component::axiomGT::binned_data&               zall, 
                         const std::vector<double>&                          x, 
                         const std::vector<double>&                          y, 
                         const int&                                          numbins, 
                         const std::vector<double>&                          SubsetInbred, 
                         const std::vector<int>&                             genohints, 
                         const int&                                          hok, 
                         const double&                                       CP)
    {
        zall.length = x.size();
        zall.NS = zall.length + 1;

        std::vector<double> z, w;
        std::vector<int> zerohints;
        std::vector<double> zeroInbred;
        // local hints always null unless assigned otherwise
        zerohints.assign(zall.length, -1);
        zeroInbred.assign(zall.length, 0);
        /// store sorted data
        z.resize(zall.length);
        w.resize(zall.length); // y values stored in sorted x order
        ///
        zall.nz.resize(zall.NS);
        zall.zz.resize(zall.NS);
        zall.zzx.resize(zall.NS);
        // y terms
        zall.zy.resize(zall.NS);
        zall.zzy.resize(zall.NS);
        zall.zxy.resize(zall.NS);
        // hints
        zall.nod.resize(zall.NS);
        zall.noh.resize(zall.NS);
        zall.noc.resize(zall.NS);

        std::vector<std::pair<int, double>> data;
        // Make index for fetching sorting data.
        for(int i = 0; i < zall.length; i++) {
            std::pair<int, double> p;
            p.first = i;
            p.second = x[i];
            data.push_back(p);
        }

        std::sort(data.begin(), data.end(), [](const auto& x, const auto& y) {return (x.second < y.second);});
        // sort data accroding to the x value in the increasing order.
        for (int i = 0; i < zall.length; i++)
        {
            z[i] = x[data[i].first];
            w[i] = y[data[i].first];
            if (genohints.size() > 0)
                zerohints[i] = genohints.at(data[i].first);
            else
                zerohints[i] = -1; //no hint here
            if (SubsetInbred.size() > 0)
                zeroInbred[i] = SubsetInbred.at(data[i].first);
        }
        zall.ZS = setup_bins(zall, z, w, numbins, zeroInbred, zerohints, hok, CP);
        zall.ec.resize(zall.ZS);
        zall.ed.resize(zall.ZS);
        zall.eh.resize(zall.ZS);
    }
    /**
     * cumulative sum of source vector using STL
     * 
     * @param final - destination for cumulative sum
     * @param source - source of individual values
     * @param length - length of data to sum over [carried over]
     */
    void vcumsum(std::vector<double>&       final, 
                const std::vector<double>& source, 
                const int&                 length)
    {
        final.resize(length);
        final[0] = source[0];
        for (int i = 1; i < length; i++) {
            final[i] = source[i] + final[i-1];
        }
    }

    /**
     * takes a cumulative sum and makes it cumulate from the other end
     *
     * @param final - output, reverse cumulative sum
     * @param source - input, forward direction cumulative sum
     * @param length - length of data (not needed for STL)
     */
    void vreversecumsum(std::vector<double>&       final, 
                        const std::vector<double>& source, 
                        const int&                 length)
    {
        double tmp;
        final.resize(length);
        tmp = source[length - 1];
        for (int i = 0; i < length; i++) {
            final[i] = tmp - source[i];
        }
    }

    /**
     * hash indexer
     * 
     * @param i - first index
     * @param j - second index
     * @param length - size of side of array
     */
    inline int hax(const int& i, 
                   const int& j, 
                   const int& length)
    {
        return(i * length + j);
    }
    /**
     *  setup the matrixes for doing individual BRLMM steps
     *
     * @param m - prior mean vector
     * @param Minv - inverse matrix of covariances (pseudo-observations) for means
     * @param Sinv - inverse matrix of variances of observations within genotypes
     * @param snp_param - parameters of algorithm
     */
    void setupBRLMM(std::vector<double>&                               m, 
                    std::vector<double>&                               Minv, 
                    std::vector<double>&                               Sinv, 
                    const cpt::component::axiomGT::probeset_param&     sp)
    {
                m.assign(3, 0);
                m[0] = sp.prior.aa.m;
                m[1] = sp.prior.ab.m;
                m[2] = sp.prior.bb.m;
                // only work with Minv
                //inversethree(Minv,M);
                Minv.assign(9, 0);
                // set up allowed variation in cluster center
                Minv[0] = sp.prior.aa.k / sp.prior.aa.ss;
                Minv[4] = sp.prior.ab.k / sp.prior.ab.ss;
                Minv[8] = sp.prior.bb.k / sp.prior.bb.ss;
                // handle cross-terms here
                Minv[1] = sp.prior.xah / sqrt(sp.prior.aa.ss * sp.prior.ab.ss);
                Minv[2] = sp.prior.xab / sqrt(sp.prior.aa.ss * sp.prior.bb.ss);
                Minv[3] = Minv[1];
                Minv[5] = sp.prior.xhb / sqrt(sp.prior.ab.ss * sp.prior.bb.ss);
                Minv[6] = Minv[2];
                Minv[7] = Minv[5];
                
                // only work with Sinv;
                //inversethree(Sinv,S);
                Sinv.assign(9, 0);
                Sinv[0] = 1 / sp.prior.aa.ss;
                Sinv[4] = 1 / sp.prior.ab.ss;
                Sinv[8] = 1 / sp.prior.bb.ss;
    }

    /**
     * This penalizes unusual frequency distributions
     * 
     * @param a - first cluster freq
     * @param b - second cluster freq
     * @param c - third cluster freq
     * @param oa - number of observations
     * @param ob - number of observations
     * @param oc - number of observations
     * @param lambda - parameterized safety value to avoid zero frequency
     */
    double mixture_penalty(const double& a,
                           const double& b, 
                           const double& c,
                           const double& oa, 
                           const double& ob, 
                           const double& oc,
                           const double& lambda)
    {
        double total, entropy;
        // [Major feature] This part computes mixture penalty (the N*entropy part) => prefer not to generate a new cluster."
        // penalty for mixing fraction
        // want to avoid splitting clusters
        // so 20/0/0 is good, 7/7/7 less good
        // i.e. need more evidence to provoke a new cluster
        //     => the smaller the entropy is, the better.
        total = a + b + c + 3 * lambda;
        entropy = oa * log((a + lambda) / total) + ob * log((b + lambda) / total) + oc * log((c + lambda) / total);
        entropy = -1 * entropy;
        return(entropy);
    }

    /**
     * inverts 3x3 matrix
     * specialty code, avoid calling external routines
     *
     * @param w - output inverse of matrix
     * @param v - input matrix
     */
    void inversethree(std::vector<double>&       w, 
                      const std::vector<double>& v)
    {
        w.resize(9);
        double det;
        // v = 9 element matrix 11,12,...33
        w[0] = v[4]*v[8] - v[5]*v[7];
        w[1] = v[2]*v[7] - v[1]*v[8];
        w[2] = v[1]*v[5] - v[2]*v[4];
        w[3] = v[5]*v[6] - v[3]*v[8];
        w[4] = v[0]*v[8] - v[2]*v[6];
        w[5] = v[2]*v[3] - v[0]*v[5];
        w[6] = v[3]*v[7] - v[4]*v[6];
        w[7] = v[1]*v[6] - v[0]*v[7];
        w[8] = v[0]*v[4] - v[1]*v[3];
        
        det = v[0]*w[0] + v[3]*w[1] + v[6]*w[2];
        for (int i = 0; i < 9; i++)
            w[i] /= det;
    }

    /**
     * sum two vectors
     *
     * @param z - output
     * @param a - input
     * @param b - input
     */
    void sumthree(std::vector<double>&       z, 
                  const std::vector<double>& a, 
                  const std::vector<double>& b)
    {
        z.resize(a.size());
        for (int i = 0; i < a.size(); i++)
            z[i] = a[i] + b[i];
    }


    /**
     * multiply two 3x3 matrices
     *
     * @param z - output
     * @param a - input 3x3 matrix (as 9 length vector)
     * @param b - input 3x3 matrix (as 9 length vector)
     */
    void timesthree(std::vector<double>&       z, 
                    const std::vector<double>& a, 
                    const std::vector<double>& b)
    {
        z.resize(9);
        z[0] = a[0]*b[0] + a[1]*b[3] + a[2]*b[6];
        z[1] = a[0]*b[1] + a[1]*b[4] + a[2]*b[7];
        z[2] = a[0]*b[2] + a[1]*b[5] + a[2]*b[8];
        z[3] = a[3]*b[0] + a[4]*b[3] + a[5]*b[6];
        z[4] = a[3]*b[1] + a[4]*b[4] + a[5]*b[7];
        z[5] = a[3]*b[2] + a[4]*b[5] + a[5]*b[8];
        z[6] = a[6]*b[0] + a[7]*b[3] + a[8]*b[6];
        z[7] = a[6]*b[1] + a[7]*b[4] + a[8]*b[7];
        z[8] = a[6]*b[2] + a[7]*b[5] + a[8]*b[8];
    }

    /**
     * multiply a 1x3 vector by a 3x3 matrix
     *
     * @param z - output
     * @param a - 3x3 matrix (as 9 length vector)
     * @param b - 1x3 vector (as 3 length vector)
     */
    void timesone(std::vector<double>&       z, 
                  const std::vector<double>& a, 
                  const std::vector<double>& b)
    {
        z.resize(3);
        z[0] = a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
        z[1] = a[3]*b[0] + a[4]*b[1] + a[5]*b[2];
        z[2] = a[6]*b[0] + a[7]*b[1] + a[8]*b[2];
    }

    void force_isotonic(double&       mb, 
                        double&       mh, 
                        double&       ma, 
                        const double& wb, 
                        const double& wh, 
                        const double& wa,
                        const double& delta)
    {
        double gamma, tmp;

        // [minor feature] This is Isotonic Regression for adjusting cluster centers.
        // This forces the posterior cluster means to be separated by at least a specified distance (delta).
        // forces means to be isotonic, separated by at least delta (equal distance)
        // moves low weight centers most  // minor delta adjustment.
        gamma = delta * (wb - wa) / (wb + wh + wa);   // use gamma to balance the following final tmp expected to be the mass center.
        // shift means towards center of mass by appropriate values
        mb += delta - gamma;
        mh += 0 - gamma;
        ma += -1 * delta - gamma;
        // Pool Adjacent Violators
        // possible that mb<=mh<=ma not true
        if (mb > mh)
        {
            tmp = (wb*mb + wh*mh) / (wb + wh);  // new mean 
            mb  = tmp;
            mh  = tmp;
        }
        if (mh > ma)
        {
            tmp = (wh*mh + wa*ma) / (wh + wa);  // new mean
            mh  = tmp;
            ma  = tmp;
            if (mb > mh)
            {
                tmp = (wb*mb + wh*mh + wa*ma) / (wb + wh + wa);  // new mean 
                mb  = tmp;
                mh  = tmp;
                ma  = tmp;
            }
        }
        // now mb<=mh<=ma is true and average data hasn't changed
        // now we make sure that they are separated by delta
        mb -= delta - gamma;
        mh -= 0 - gamma;
        ma -= -1 * delta - gamma;
        // now mb+delta<=mh, mh+delta<ma
    }

    /**
     * bayesian variance - prior + observed + shift in means
     *
     * @param v - prior pseudo-observations
     * @param pv - prior variance
     * @param ddx - sum of squares of observations
     * @param cc - sum of observations
     * @param nc - number of observations
     * @param k - prior pseudo-observations for mean
     * @param mcc - observed mean
     * @param pm - prior location for mean
     */
    double b_var(const double& v, 
                 const double& pv, 
                 const double& ddx, 
                 const double& cc, 
                 const double& nc, 
                 const double& k, 
                 const double& mcc, 
                 const double& pm)
    {
        double tmp;
        // variance is prior
        tmp = v * pv;
        // plus observed ssq sum(x^2)-sum(x)^2/n
        tmp += ddx - cc*cc / (nc + 0.0001);
        // plus change in means squared
        tmp += (k / (k + nc)) * nc * (mcc - pm) * (mcc - pm);
        // divided by effective number of observations
        tmp = tmp / (v + nc);
        return (tmp);
    }


    /**
     * gaussian likelihood, in convenient running sums form
     *
     * @param xcx - sum of squares of observations
     * @param lc  - posterior mean
     * @param xc - sum of observations
     * @param yc - number of observations
     * @param dvc - posterior variance
     */
    double l_posterior(const double& xcx, 
                       const double& lc, 
                       const double& xc, 
                       const double& yc, 
                       const double& dvc)
    {
        double tmp;
        tmp = (xcx - 2*lc*xc + lc*lc * yc) / dvc + yc*log(dvc);
        return(tmp);
    }

    /**
     * gaussian likelihood, for just one piece of data
     * 
     * @param m - mean
     * @param x - observed value
     * @param v - variance
     */
    double l_normal(const double& m, const double& x, const double& v)
    {
        return ((m - x) * (m - x) / v + log(v));
    }

    /**
     * cheap inverse-gamma likelihood, for one piece of data
     * 
     * @param av - prior expected variance
     * @param dv - observed variance
     * @param v - pseudo-observations for prior
     */
    double l_inverse(const double& av, 
                     const double& dv, 
                     const double& v)
    {
        return(av / dv + (v + 1) * log(dv));
    }

    /**
     * mask out a portion of an array
     *
     * @param final - array to be masked
     * @param length - size of margin of array
     * @param val - value to be placed in masked area
     */
    void vmask(std::vector<double>& final, 
               const int&           length, 
               const double&        val)
    {
        for (int i = 0; i < length; i++)
            for (int j = 0; j < i; j++)
                final[hax(i, j, length)] = val;
        
    }

    /**
     *  convert log-likelihood of all labelings into relative probability
     * 
     * @param q - matrix to be converted
     * @param ZS - edge size of matrix
     */
    void qtoeq(std::vector<double>& q, const int& ZS)
    {
        double qmin;
        int tin;

    // compute minimum of useful values
        // so as to rescale log-likelihood to relative levels
        qmin = q[0];
        for (int i = 0; i < ZS; i++)
            for (int j = i; j < ZS; j++)
            {
                tin = hax(i, j, ZS);
                if (qmin > q[tin])
                    qmin = q[tin];
            }
        for (int i =0; i < q.size(); i++)
        {
            q[i] = qmin - q[i];  // log scale minus => probe scale division
            q[i] = exp(q[i]);  // the relative probability of other kinds of partition way with respect to Max posterior partition way.
        }
        // mask out useless values
        vmask(q, ZS, 0);
    }

    /**
     * sum up the source array along one axis
     *
     * @param final - output, marginal sums of array
     * @param source - input, all the individual values
     * @param length - dimension of array
     * @param flag - which margin to sum to
     */
    void apply_sum(std::vector<double>&       final, 
                   const std::vector<double>& source, 
                   const int&                 length, 
                   const int&                 flag)
    {
        double sum;

        for (int i = 0; i < length; i++)
        {
            sum=0;
            for (int j = 0; j < length; j++)
            {
                if (flag)
                    sum += source[hax(i, j, length)];
                else
                    sum += source[hax(j, i, length)];
            }
            final[i] = sum;
        }
    }

    /**
     * turn the overall relative probability of labelings into relative probability of individual genotypes
     *
     * @param ec - relative probability of c = a allele dominant
     * @param eh - relative probability of h = ab allele about equal
     * @param ed - relative probability of d = b allele dominant
     * @param ZS - marginal size of matrix
     * @param q  - matrix of relative probability of labelings
     */
    void eqtoproblabel(std::vector<double>&       ec, 
                       std::vector<double>&       eh, 
                       std::vector<double>&       ed, 
                       const int&                 ZS, 
                       const std::vector<double>& q)
    {
        int i;
        double total;
        total = 0; 
        for (int i = 0; i < q.size(); i++)
            total += q[i];
        
        std::vector<double> eqt;
        eqt.resize(ZS);
        apply_sum(eqt, q, ZS, 0);  // colsum: sum different i, given same j, eqt store different j, sum i  => eqt: sum of all possible partition posterior given specified j (specified AA cluster points) / max_posterior_P
        vcumsum(ec, eqt, ZS);      // 
        // entry i is in ed[l] iff i>l
        // do cumsum to get i<=l, then subtract
        apply_sum(eqt, q, ZS, 1);  // rowsum  => eqt: sum of all possible partition posterior given specified i (specified BB cluster points) / max_posterior_P
        vcumsum(ed, eqt, ZS);
        
        for (i = 0; i < ZS - 1; i++)
        {
            ed[i] = total - ed[i];
            // entry i is in eh[l] iff not in a or b
            eh[i] = total - ed[i] - ec[i];
            // and therefore the sum is always total
            //sm     = total;
            ed[i] /= total;
            ec[i] /= total;
            eh[i] /= total;
        }
    }
    void integratebrlmmoverlabelings(cpt::component::axiomGT::binned_data& zall, 
                                     const cpt::component::axiomGT::probeset_param& sp)
    {
        auto& ec = zall.ec;
        auto& eh = zall.eh;
        auto& ed = zall.ed;
        int   ZS = zall.ZS;

        int ZSS;
        int NS;
        int i, j;
        // allocate memory for these puppies
        // over allocate for most of these, but temporary
        ZSS = ZS*ZS;

        // likelihood of data, given 
        std::vector<double> q;
        q.resize(ZSS);
        q.assign(ZSS,0 );
        std::vector<double> ddx;
        std::vector<double> ccx;
        std::vector<double> nd;
        std::vector<double> nc;
        std::vector<double> dd;
        std::vector<double> cc;
        ddx.resize(ZS);
        ccx.resize(ZS);
        nd.resize(ZS);
        nc.resize(ZS);
        dd.resize(ZS);
        cc.resize(ZS);

        std::vector<double> dnod;
        std::vector<double> dnoh;
        std::vector<double> dnoc, cnoc;
        dnod.resize(ZS);
        //hnod.resize(ZS);
        //cnod.resize(ZS);
        dnoh.resize(ZS);
        //hnoh.resize(ZS);
        //cnoh.resize(ZS);
        dnoc.resize(ZS);
        //hnoc.resize(ZS);
        cnoc.resize(ZS);

        // count up the number of contradictions
        // cumulative contradictions (Cpenalty, CP, 16/each)
        vcumsum(dnod, zall.nod, ZS);
        vcumsum(dnoh, zall.noh, ZS);
        vcumsum(dnoc, zall.noc, ZS);
        //vreversecumsum(cnod,dnod,ZS);
        //vreversecumsum(cnoh,dnoh,ZS);
        vreversecumsum(cnoc, dnoc, ZS);

        // cumulative number - weight for bins
        vcumsum(nd, zall.nz, ZS);
        NS = (int)floor(nd[ZS-1] + 0.00001); // number of points total
        vreversecumsum(nc, nd, ZS);
        vcumsum(dd, zall.zz, ZS);
        vreversecumsum(cc, dd, ZS);    
        vcumsum(ddx, zall.zzx, ZS);
        vreversecumsum(ccx, ddx, ZS);

        int tin;

        // temporaries
        double ycij,  xcij,  xcxij;
        double ydij,  xdij,  xdxij;
        double yhij,  xhij,  xhxij;
        double ldij,  lcij,  lhij;
        double dvdij, dvcij, dvhij;

        // setup brlmm bits that don't change
        // speed
        std::vector<double> m;
        std::vector<double> Minv;
        std::vector<double> Sinv;
        setupBRLMM(m, Minv, Sinv, sp);
        std::vector<double> Tz;
        std::vector<double> Zt;
        std::vector<double> Xinv;
        std::vector<double> Xt;
        std::vector<double> Mz;
        // vectors that change
        // sum of x
        std::vector<double> Nv;
        // number of data points
        std::vector<double> N;
        Nv.resize(3);
        N.assign(9, 0);

        if (sp.copynumber < 2)
        {
            // no hets
            for (i = 0; i < ZS; i++)
            {
                for (j = i + 1; j < ZS; j++)
                {
                    tin = hax(i,j,ZS);
                    q[tin] += NS * 1000;  // No penalty only when 2 homo clusters cases
                }
            }
        }
        if (sp.hints == 1)
        {
            // penalty for contradiction of reference
            for (i = 0; i < ZS; i++)
            {
                for (j = i; j < ZS; j++)
                {
                    tin = hax(i,j,ZS);
                    q[tin] += (dnod[i] + cnoc[j] + (dnoh[j] - dnoh[i]));
                }
            }

        }
        for (i = 0; i < ZS; i++)
        {
            for (j = i; j < ZS; j++)
            {
                tin = hax(i, j, ZS);
                q[tin] += mixture_penalty(nc[j], nd[i], nd[j]-nd[i],
                                        nc[j], nd[i], nd[j]-nd[i],
                                        sp.SafetyFrequency);
            }
        }
        // [Major feature] This part computes BIC penalty (the k*log(n) part).
        // if useful to do some bic correction
        // bic * log(n) - empirical correction
        for (i = 0; i < ZS; i++)
        {
            // 1. subtract off once for 2 cluster
            // 2. [0,0],[0,ZS-1],[ZS-1,ZS-1] occur twice
            //    so subtract off 2 times for 1 cluster
            tin = hax(i, i, ZS);
                q[tin] -= sp.bic * log((double) NS);
            tin = hax(0, i, ZS);
                q[tin] -= sp.bic * log((double) NS);
            tin = hax(i, ZS-1, ZS);
                q[tin] -= sp.bic * log((double) NS);
            
            // The following is for 3 clusters cases, thus 3*2*log(N), where 2 represents mean and variance, 3 means three clusters.
            for (j = i; j < ZS; j++)
            {
                tin = hax(i, j, ZS);
                // add 3 times for everyone
                q[tin] += 3 * sp.bic * log((double) NS);
            }
        }
        // likelihood of data under posterior
        // (x-m)^2 = x^2-2*x*m+m^2
        // running sums sum(x^2)-2*(sum(x)*m+sum(1)*m^2
        // big loop: try all reasonable labelings
        // bb^i->ab^(j-i)->aa^(n-j+1)
        // log likelihood means the log likelihood (probability) under this kind of partition.
        // log prior means the log probability of this kind of partition
        // log marginal means the log probability of the current data without any specified partition
        //     = log (sum all possible partitions of (likelihood*prior))
        // log posterior means the log probability of this kind of partition given observed data.
        //     = log ( likelihood * prior / marginal ) = log likelihood + log prior - log marginal
        // Total: (1+(N+1)) * (N+1) / 2 partition ways
        for (int i = 0; i < ZS; i++) 
        {
            for(int j = i; j < ZS; j++) 
            {
                tin   = hax(i, j, ZS); //  tin = (i * ZS) + j
                // initialize global matrix
                // c  (genotype AA cluster, right most)
                ycij  = nc[j];         //  cumulative sample size in this cluster
                xcij  = cc[j];         //  cumulative SIGMA_bin X   (contrast) value in this cluster
                xcxij = ccx[j];        //  cumulative SIGMA_bin X^2 (contrast) value in this cluster
                // d  (genotype BB cluster, left most)
                ydij  = nd[i];
                xdij  = dd[i];
                xdxij = ddx[i];
                // h  (genotype AB cluster, middle)
                yhij  = nd[j]  - nd[i];
                xhij  = dd[j]  - dd[i];
                xhxij = ddx[j] - ddx[i];


                // brlmm here
                // setup centers
                Nv[0] = xcij;
                Nv[1] = xhij;
                Nv[2] = xdij;
                // number of data points
                N[0]  = ycij;
                N[4]  = yhij;
                N[8]  = ydij;

                // Mean update
                // do some inversions
                // Quick transform: In paper: N = Tz (the following)
                //                            K = Minv (the following)
                timesthree(Tz, N, Sinv); // Mat_Tz     = Mat_N x Mat_Sinv          3x3   =>   n/((s_0)^2)
                sumthree(Zt, Minv, Tz);  // Mat_Zt     = Mat_Minv  +  Mat_Tz       3x3   =>   (n+k_0)/((s_0)^2), rho_xy/(s_x0*s_y0)
                inversethree(Xinv, Zt);  // Mat_Xinv   = Mat_Zt^(-1)               3x3   

                                                                                        // the 1/ss in the Sinv would get cancelled both in the numerator and demonator after combination,
                                                                                        // Nv is sum s which means N*average.
                timesone(Tz, Sinv, Nv);   // Vec_Tz     = Mat_Sinv x Vec_Nv         3x1  i.e. Assigning tentative genotype observations to clusters (N x Sinv)
                                                                                        //      * the observed mean locations (Nv / N)  => N is cancelled.
                timesone(Zt, Minv, m);    // Vec_Zt     = Mat_Minv x Vec_m          3x1  i.e. prior precision matrix * prior mean locations
                sumthree(Xt, Tz,  Zt);    // Vec_Xt     = Vec_Tz + Vec_Zt           3x1
                
                timesone(Mz, Xinv, Xt);   // Vec_Mz     = Mat_Xinv x Vec_Xt         3x1  i.e. Mz is the final updated mean vector.
                
                // figure out the triple mean/variance combo
                ldij = Mz[2];   // BB genotype
                lhij = Mz[1];   // AB genotype
                lcij = Mz[0];   // AA genotype
                // force means apart by shellbarrier
                if (sp.hardshell == 3)   //  3
                    force_isotonic(ldij, lhij, lcij,
                                ydij + sp.prior.bb.k, 
                                yhij + sp.prior.ab.k,
                                ydij + sp.prior.aa.k, 
                                sp.shellbarrier);
                // compute variance given means
                // within-cluster variance + prior + distance shifted
                // Variance update
                // [minor feature] Update the posteriror variance.
                dvcij = b_var(sp.prior.aa.v, sp.prior.aa.ss, xcxij, xcij, ycij, sp.prior.aa.k, lcij, sp.prior.aa.m);
                dvdij = b_var(sp.prior.bb.v, sp.prior.bb.ss, xdxij, xdij, ydij, sp.prior.bb.k, ldij, sp.prior.bb.m);
                dvhij = b_var(sp.prior.ab.v, sp.prior.ab.ss, xhxij, xhij, yhij, sp.prior.ab.k, lhij, sp.prior.ab.m);
                
                // fix up common variance which shrinks the variance of different within-cluster to be similar.
                if (sp.comvar == 1)       //  1
                {
                    // [minor feature] allow the variances to be different from one genotype cluster to another.
                    // This makes the within-cluster variances to be similar and improves the behavior of clusters with 
                    // few data points.
                    double tc, td, th;
                    double tnc, tnd, tnh;
                    double tl, tlm;
                    tnd   = ydij + sp.prior.bb.v;  // number of data points + prior obs of variance.
                    tnc   = ycij + sp.prior.aa.v;  // number of data points + prior obs of variance.
                    tnh   = yhij + sp.prior.ab.v;  // number of data points + prior obs of variance.
                    td    = dvdij * tnd;
                    tc    = dvcij * tnc;
                    th    = dvhij * tnh;
                    // cross-talk between variances 0 = independent, 1 = common variance
                    tl    = sp.lambda;
                    tlm   = 3 - 2 * sp.lambda;
                    dvdij = (tlm*td + tl*tc  + tl*th)  / (tlm*tnd + tl*tnc  + tl*tnh);
                    dvcij = (tl*td  + tlm*tc + tl*th)  / (tl*tnd  + tlm*tnc + tl*tnh);
                    dvhij = (tl*td  + tl*tc  + tlm*th) / (tl*tnd  + tl*tnc  + tlm*tnh);

                    q[tin] += l_posterior(xdxij, ldij, xdij, ydij, dvdij);
                    q[tin] += l_posterior(xhxij, lhij, xhij, yhij, dvhij);
                    q[tin] += l_posterior(xcxij, lcij, xcij, ycij, dvcij);

                    // likelihood of mean under prior           // log posterior parameters (mean) likelihood
                    q[tin] += l_normal(sp.prior.bb.m, ldij, 1 / Minv[8]);
                    q[tin] += l_normal(sp.prior.ab.m, lhij, 1 / Minv[4]);
                    q[tin] += l_normal(sp.prior.aa.m, lcij, 1 / Minv[0]);

                    // likelihood of variance under prior       // log posterior parameters (variance) likelihood
                    q[tin] += l_inverse(sp.prior.bb.ss, dvdij, sp.prior.bb.v);
                    q[tin] += l_inverse(sp.prior.ab.ss, dvhij, sp.prior.ab.v);
                    q[tin] += l_inverse(sp.prior.aa.ss, dvcij, sp.prior.aa.v);

                    // half
                    q[tin] /= 2;

                    // avoid cluster splitting by ad-hoc penalty for FLD too small
                    // rather ad-hoc favor for having FLD large!
                    // [Major feature] This part computes the penalty (FLD + Geman-McClure)
                    // (prefer well-sperarated clusters case).
                    double flddh, fldhc, flddc;
                    // compute FLD for cluster separation
                    flddh = (ldij - lhij) * (ldij - lhij) / (dvdij + dvhij);
                    fldhc = (lhij - lcij) * (lhij - lcij) / (dvhij + dvcij);
                    flddc = (ldij - lcij) * (ldij - lcij) / (dvdij + dvcij);
                    // threshold FLD to avoid too much repulsion  (**)
                    // [minor feature] Smoothly threshold: Geman-McClure transformation: 
                    // B = F/(1+F/z), where z is a tuning parameter that sets the threshold 
                    // stopping penalizing using Geman-McClure, F is a given scaled separation.
                    flddh = flddh / (1 + flddh / sp.CSepThr);
                    fldhc = fldhc / (1 + fldhc / sp.CSepThr);
                    // might have only homs, so keep this
                    flddc = flddc / (1 + flddc / (2 * sp.CSepThr));
                    // apply data points at risk
                    flddh *= (ydij + yhij);
                    fldhc *= (yhij + ycij);
                    flddc *= (ydij + ycij);
                    // favor! [larger FLD better, up to a point]
                    q[tin] -= sp.CSepPen * (flddh + fldhc + flddc);     // inverse sign => - add to likelihood
                }
            }
        }
        qtoeq(q, ZS);
        eqtoproblabel(ec, eh, ed, ZS, q);  // sum each relative likelihood from the perspective of each genotype.
    }
    
    /**
     * generate sums of data based on weighted cluster membership
     * 
     * @param n - effective number of data points 
     * @param m - effective sum of data points
     * @param s - effective sum of squares of data points
     * @param zall - binned contrast data for points
     * @param wi - weight of bins
     * @param ZS - number of bins
     */
    void compute_two_binned_cluster(std::vector<double>&                                six, 
                                    const cpt::component::axiomGT::binned_data&         zall, 
                                    const std::vector<double>&                          wi, 
                                    const int&                                          ZS)
    {
        for (int j = 0; j < 6; j++)
            six[j] = 0;
        /*
            n=0;
            m=0;
            s=0;
            my=0;
            sy=0;
            xy=0;
        */
        
        for (int i=0; i<ZS-1; i++)
        {          //   Statistics * Weight => Eeffective Statistics
            six[0] +=  zall.nz[i+1]  * wi[i];  // sample size n
            six[1] +=  zall.zz[i+1]  * wi[i];  // sum X
            six[2] +=  zall.zzx[i+1] * wi[i];  // sum X^2
            six[3] +=  zall.zy[i+1]  * wi[i];  // sum Y
            six[4] +=  zall.zzy[i+1] * wi[i];  // sum Y^2
            six[5] +=  zall.zxy[i+1] * wi[i];  // sum X*Y
        }
    }

    void bayes_two_variance(
        cpt::component::axiomGT::cluster_data&              post, 
        const std::vector<double>&                          sixc, 
        const cpt::component::axiomGT::cluster_data&        prior)
    {
        post.v = prior.v + sixc[0];

        post.ss = prior.v * prior.ss;  // prior
        post.ss += sixc[2] - sixc[1]*sixc[1] / (sixc[0] + 0.001); // observed ssq, avoid dividebyzero
        post.ss += ((prior.k * sixc[0]) / (prior.k + sixc[0])) * (post.m - prior.m) * (post.m - prior.m);  
        post.ss /= post.v;

        post.yss = prior.v * prior.yss; // prior
        post.yss += sixc[4] - sixc[3]*sixc[3] / (sixc[0] + 0.001);
        post.yss += ((prior.k * sixc[0]) / (prior.k + sixc[0])) * (post.ym - prior.ym) * (post.ym - prior.ym);  
        post.yss /= post.v;

        //cout << prior.v*prior.yss << "\t" << sixc[4]-sixc[3]*sixc[3]/(sixc[0]+0.001) << "\t" << ((prior.k*sixc[0])/(prior.k+sixc[0])) * (post.ym-prior.ym)*(post.ym-prior.ym) << endl;

        post.xyss = prior.v * prior.xyss; // prior
        post.xyss += sixc[5] - sixc[1]*sixc[3] / (sixc[0] + 0.001);
        post.xyss += ((prior.k * sixc[0]) / (prior.k + sixc[0])) * (post.ym - prior.ym) * (post.m - prior.m);
        post.xyss /= post.v;
    }

    void shrink_cluster_variance(
        cpt::component::axiomGT::cluster_data&                 out, 
        const cpt::component::axiomGT::cluster_data&           a, 
        const cpt::component::axiomGT::cluster_data&           b, 
        const cpt::component::axiomGT::cluster_data&           c, 
        const double&                                          lambda)
    {
        double main, cross;
        
        main = 3 - 2*lambda;
        cross = lambda;

        out.ss   = (main*a.ss*a.v  + cross*b.ss*b.v  + cross*c.ss*c.v)  / (main*a.v + cross*b.v + cross*c.v);
        out.yss  = (main*a.yss*a.v + cross*b.yss*b.v + cross*c.yss*c.v) / (main*a.v + cross*b.v + cross*c.v);
        out.xyss = sqrt(out.ss * out.yss) * a.xyss / (sqrt(a.ss * a.yss));  // covariance shrinkage, same correlation
        // what should degree of confidence in variance be???
    }

    void bayes_two_shrink_variance(cpt::component::axiomGT::probeset_param& sp)
    {
        // keep the correlation the same within each cluster
        // move the variances towards each other?
        // analogue of 1-d common variance
        
        cpt::component::axiomGT::probeset_distribution tmp;
        tmp.Copy(sp.posterior);

        shrink_cluster_variance(tmp.aa, sp.posterior.aa, sp.posterior.ab, sp.posterior.bb, sp.lambda);
        shrink_cluster_variance(tmp.ab, sp.posterior.ab, sp.posterior.bb, sp.posterior.aa, sp.lambda);
        shrink_cluster_variance(tmp.bb, sp.posterior.bb, sp.posterior.aa, sp.posterior.ab, sp.lambda);
        
        sp.posterior.Copy(tmp); // copy shrunk values
    }

    void labels_two_posterior(cpt::component::axiomGT::probeset_param& sp, 
                              const cpt::component::axiomGT::binned_data& zall)
    {
        const auto& ZS = zall.ZS;
        const auto& ec = zall.ec;
        const auto& eh = zall.eh;
        const auto& ed = zall.ed;

        // Means = (K+N)^-1 (K*u+N*X)
        // Fix isotonic X dimension means
        // K<- K+N
        // V1<-V + SSQ + [kn/(k+n)] * (u-Means)^2
        // Vshrink = common var
        std::vector<double> sixd,sixc,sixh;
        sixd.resize(6);
        sixc.resize(6);
        sixh.resize(6);
        // compute basic stats, raw mean/sum-square/number of entries
        // N, sum X, sum X^2, sum Y, sum Y^2, sumXY
        // for each genotype assignment mixed fraction
        compute_two_binned_cluster(sixd, zall, ed, ZS);
        compute_two_binned_cluster(sixc, zall, ec, ZS);
        compute_two_binned_cluster(sixh, zall, eh, ZS);

        // set up the N matrix
        arma::mat N(6, 6, arma::fill::zeros);  // effective number of observations
        N(0, 0) = sixc[0];
        N(1, 1) = sixc[0];
        N(2, 2) = sixh[0];
        N(3, 3) = sixh[0];
        N(4, 4) = sixd[0];
        N(5, 5) = sixd[0];

        arma::mat K(6, 6, arma::fill::zeros);  // prior pseudo-observations
        K(0, 0) = sp.prior.aa.k;
        K(1, 1) = sp.prior.aa.k;
        K(2, 2) = sp.prior.ab.k;
        K(3, 3) = sp.prior.ab.k;
        K(4, 4) = sp.prior.bb.k;
        K(5, 5) = sp.prior.bb.k;
        // fill in off-diagonal elements for x to x
        K(0, 2) = sp.prior.xah;
        K(2, 0) = sp.prior.xah;
        K(2, 4) = sp.prior.xhb;
        K(4, 2) = sp.prior.xhb;
        K(0, 4) = sp.prior.xab;
        K(4, 0) = sp.prior.xab;
        // fill in off diagonal elements for y to y
        K(1, 3) = sp.prior.yah;
        K(3, 1) = sp.prior.yah;
        K(3, 5) = sp.prior.yhb;
        K(5, 3) = sp.prior.yhb;
        K(1, 5) = sp.prior.yab;
        K(5, 1) = sp.prior.yab;

        arma::mat Left = K + N;
        Left = arma::inv(Left);

        arma::vec Mu(6);  // prior mean
        Mu(0) = sp.prior.aa.m;
        Mu(1) = sp.prior.aa.ym;
        Mu(2) = sp.prior.ab.m;
        Mu(3) = sp.prior.ab.ym;
        Mu(4) = sp.prior.bb.m;
        Mu(5) = sp.prior.bb.ym;

        arma::mat KMu = K * Mu;

        arma::vec Nv(6);
        Nv(0) = sixc[1];
        Nv(1) = sixc[3];
        Nv(2) = sixh[1];
        Nv(3) = sixh[3];
        Nv(4) = sixd[1];
        Nv(5) = sixd[3];

        arma::mat Right = KMu + Nv;
        arma::mat Final = Left * Right;
        
        // update means
        sp.posterior.aa.m  = Final(0,0);
        sp.posterior.aa.ym = Final(1,0);
        sp.posterior.ab.m  = Final(2,0);
        sp.posterior.ab.ym = Final(3,0);
        sp.posterior.bb.m  = Final(4,0);
        sp.posterior.bb.ym = Final(5,0);

        // update posterior strength
        sp.posterior.aa.k = sixc[0] + sp.prior.aa.k;
        sp.posterior.ab.k = sixh[0] + sp.prior.ab.k;
        sp.posterior.bb.k = sixd[0] + sp.prior.bb.k;

        force_isotonic(sp.posterior.bb.m, sp.posterior.ab.m, sp.posterior.aa.m,
                    sp.posterior.bb.k, sp.posterior.ab.k, sp.posterior.aa.k, 
                    sp.shellbarrier);

        // update posterior frequency
        // Omit currently
        // now I have the means computed!!!
        // on to compute variances
        // Technically, these are variances computed from the non-isotonic means
        bayes_two_variance(sp.posterior.aa, sixc, sp.prior.aa);
        bayes_two_variance(sp.posterior.ab, sixh, sp.prior.ab);
        bayes_two_variance(sp.posterior.bb, sixd, sp.prior.bb);
        // how do I shrink variances in 2-D correctly?
        bayes_two_shrink_variance(sp);

        // cluster covariances in pseudo-obs
        sp.posterior.xah = sp.prior.xah;
        sp.posterior.xhb = sp.prior.xhb;
        sp.posterior.xab = sp.prior.xab;
        sp.posterior.yah = sp.prior.yah;
        sp.posterior.yhb = sp.prior.yhb;
        sp.posterior.yab = sp.prior.yab;
    }

    void create_data_model(
        mlpack::gmm::GMM&                                       data_model,
        const cpt::component::axiomGT::probeset_distribution&   p,
        const int&                                              freqflag,
        const double&                                           inflatePRA,
        const int&                                              copynumber
    )
    {
        double up;
        int cluster_number = copynumber + 1;
        arma::vec weights(3, arma::fill::ones);
        weights /= cluster_number;
        double adj_k = p.ab.k;
        if(copynumber < 2) {
            weights(1) = adj_k = 0.000001;
        }
        if(freqflag) 
        {
            double total_obs = p.bb.k + adj_k + p.aa.k;
            weights(0) = p.bb.k / total_obs;  // BB genotype cluster
            weights(1) =  adj_k / total_obs;  // AB genotype cluster
            weights(2) = p.aa.k / total_obs;  // AA genotype cluster
        }

        std::vector<mlpack::distribution::GaussianDistribution> dists;
        std::vector<const cpt::component::axiomGT::cluster_data *> clusters {&p.bb, &p.ab, &p.aa};  // BB, AB, AA genotype clusters
        for(const auto& cl: clusters) 
        {
            up = 1 + inflatePRA/cl->k;
            arma::vec mean(2, arma::fill::zeros);
            arma::mat covariance(2, 2, arma::fill::zeros);

            mean(0) = cl-> m;
            mean(1) = cl->ym;

            covariance(0, 0) = up * cl->ss;
            covariance(1, 1) = up * cl->yss;
            covariance(0, 1) = covariance(1, 0) = up * cl->xyss;

            dists.emplace_back(mlpack::distribution::GaussianDistribution(mean, covariance));
        }
        data_model = mlpack::gmm::GMM(dists, weights);
    }
    void set_mdl_invalidate(
        std::map<int, ced::AxiomGTModel>& dbmodel,
        const std::string&                ps_name,
        const int&                        spec_copy_number
    )
    {
        dbmodel[spec_copy_number] = { ps_name, mg::GMM{}, false };
    }
    template< typename KEY_TYPE, typename VALUE_TYPE >
    void check_key_exist( 
        const std::map< KEY_TYPE, VALUE_TYPE >& container, 
        const KEY_TYPE& key,
        const std::string& ps_name
    )
    {
        if( auto it = container.find( key ); it == container.end() )
            throw std::runtime_error("Probeset "
                                      + ps_name 
                                      + " cannot find the model for the copynumber: " 
                                      + std::to_string(key) + ".\n");
    }    

    template<class T>
    void params_config( const cf::Json<T>& algo_json )
    {
        analysis_params.wobble                = algo_json.template get_optional<double>("wobble.content")              .value_or(0.05);

        analysis_params.bins                  = algo_json.template get_optional< int > ("bins.content")                .value_or(100);
        analysis_params.hok                   = algo_json.template get_optional< int > ("hok.content")                 .value_or( 0 );
        analysis_params.contradictionpenalty  = algo_json.template get_optional<double>("contradictionpenalty.content").value_or(16) ;

        // analysis_params.hints                 = 0;
        analysis_params.SafetyFrequency       = algo_json.template get_optional<double>("SafetyFrequency.content")     .value_or( 1 );
        analysis_params.bic                   = algo_json.template get_optional<double>("bic.content")                 .value_or( 2 );
        analysis_params.hardshell             = algo_json.template get_optional< int > ("hardshell.content")           .value_or( 3 );
        analysis_params.comvar                = algo_json.template get_optional< int > ("comvar.content")              .value_or( 1 );
        analysis_params.CSepThr               = algo_json.template get_optional<double>("CSepThr.content")             .value_or( 4 );
        analysis_params.CSepPen               = algo_json.template get_optional<double>("CSepPen.content")             .value_or(0.1);

        analysis_params.shellbarrier          = algo_json.template get_optional<double>("shellbarrier.content")        .value_or(0.75);
        analysis_params.lambda                = algo_json.template get_optional<double>("lambda.content")              .value_or(1.0);

        analysis_params.mix                   = algo_json.template get_optional< int > ("mix.content")                 .value_or( 1 );
        analysis_params.inflatePRA            = algo_json.template get_optional<double>("inflatePRA.content")          .value_or( 0 );

        // analysis_params.ocean                 = algo_json.template get_optional<double>("callocean.content")           .value_or(0.00001);

    }
    virtual void config_parameters( const bpt::ptree& p ) override
    {
        /* TODO require cube */

        auto&  db ( this->mut_data_pool() );

        auto json ( cf::make_json( p ) );

        params_config( cf::make_json( p ) );

        db.ocean         = json.get_optional<double>("callocean.content")        .value_or( 0.00001 );
        db.maxconfidence = json.get_optional<double>("callmaxconfidence.content").value_or(  1.0  );

        axiomGT_prefix_ = com_::require_w< std::string > (
            json, "axiomGT_prefix", com_::make_ref_parameter("output_dir")
        );
        probeset_priors_ipath_ = com_::require_w< std::string > (
            json, "probeset_priors",     com_::make_ref_parameter("probeset_priors")
        );
        probeset_posteriors_opath_ = com_::require_w< std::string > (
            json, "probeset_posteriors", com_::make_ref_parameter("probeset_posteriors")
        );

        select_probeset_num = json
            .get_optional<std::size_t>("select_probeset_num.content")
            .value_or(0);

        probeset_cube_ = com_::require_w             < cf::Cube<double> > ( 
              json
            , "probeset_cube"         
            ,  com_::make_ref_parameter("probeset_cube")  
        );
        specialsnps_info_cube_      = com_::require_w< cpt::format::Cube<double> > (
              json
            , "specialsnps_info_cube"
            ,  com_::make_ref_parameter("specialsnps_info_cube")
        );
        sample_specialsnps_cn_cube_ = com_::require_w< cf::Cube<double> > (
              json
            , "sample_special_snps_copynumbers_cube"
            ,  com_::make_ref_parameter("sample_special_snps_copynumbers_cube")
        );
        sample_genohints_cube_      = com_::require_w< cf::Cube<double> > (
              json
            , "sample_genotype_hints_cube"
            ,  com_::make_ref_parameter("sample_genotype_hints_cube")
        );

        archive       = json.get_optional<std::string>( "archive.content"       ).value_or("");
        cpt::dbg << "archive : " << archive << std::endl;
        is_posterior_ = json.get_optional< bool >     ( "is_posterior.content"  ).value_or( true );
        thread_num_   = json.get_optional<std::size_t>( "thread_num.content"    ).value_or(16);
        auto dvlp     = json.get_optional<std::string>( "dbg_view_list.content" );
        if ( dvlp )
        {
            cpt::dbg << "dbg_view_list : " << dvlp.get() << std::endl;
            for ( auto&& line : std::ifstream(dvlp.get()) | ca::getlined() )
            {
                dbg_view_list.emplace(line);
            }
        }
    }
    void check_opath(std::string& opath, const std::string& val) 
    {
        if( opath.empty() ) opath = val;
    }
    void models_initialize()
    {
        auto& db ( this->mut_data_pool() );
        probeset_priors_ipath_      ->  initialize_or( "" );
        auto& probeset_priors_ipath  =  probeset_priors_ipath_->get();

        db.axiom_gt_aux.initialize_by_copynumber_categories(
            db.data_likelihood, 
            { { 1, ced::AxiomGTModel() }, { 2, ced::AxiomGTModel() } } 
        );
        db.axiom_gt_aux.setup_NIW_bayes_model_basics( probeset_priors_ipath, probeset_names );
    }    
    virtual void initialize() override
    {
        /* TODO load cube       */
        auto& db( this->mut_data_pool() );
        axiomGT_prefix_             ->  initialize();
        probeset_priors_ipath_      ->  initialize_or( "" );

        probeset_posteriors_opath_  ->  initialize_or(  axiomGT_prefix_ -> get() + "probeset_posteriors.mdl" );
        check_opath( probeset_posteriors_opath_->get(), axiomGT_prefix_ -> get() + "probeset_posteriors.mdl" );

        probeset_cube_              ->  initialize();
        specialsnps_info_cube_      ->  initialize();
        sample_specialsnps_cn_cube_ ->  initialize();
        sample_genohints_cube_      ->  initialize();



        auto& tspecialsnps_info_cube      = specialsnps_info_cube_      -> get();
        auto& tsample_specialsnps_cn_cube = sample_specialsnps_cn_cube_ -> get();
        auto& tsample_genohints_cube      = sample_genohints_cube_      -> get();

        auto& probeset_cube = probeset_cube_                           -> get();
        probeset_names      = probeset_cube .z_axis.get_labels();
        sample_names        = probeset_cube .y_axis.get_labels();
        auto& specialsnps_info_cube      = specialsnps_info_cube_      -> get();
        db.axiom_gt_aux.set_copynumber_categories_for_each_probeset(
            specialsnps_info_cube, 
            probeset_names
        );
        auto& sample_specialsnps_cn_cube = sample_specialsnps_cn_cube_ -> get();
        db.axiom_gt_aux.set_copynumbers_from_sample_special_snps(
            sample_specialsnps_cn_cube, 
            probeset_names, 
            sample_names
        );
        auto& sample_genohints_cube      = sample_genohints_cube_      -> get();
        analysis_params.hints = db.axiom_gt_aux.set_genohints(
            sample_genohints_cube, 
            probeset_names, 
            sample_names
        );

        models_initialize();
        db.thread_pool->resize_pool( thread_num_ );
    }
    virtual void NOINLINE start() override
    {
        // NPTimer timer("axiomGT_training");
        cu::GProfiler profiler ( "axiomGT_training.prof" );

        auto& db ( this->mut_data_pool() );
        auto& probeset_cube = probeset_cube_->get();

        const auto& cube      ( probeset_cube  );
        const auto& ps_names  ( probeset_names );
        const ca::ProbesetRndChoice prc{};
        const auto cands ( prc.get_rnd_vec( probeset_cube, select_probeset_num ) );
        std::vector<std::tuple<std::size_t, std::vector<std::size_t>>> missing_cluster;

        /* monitor setting */
        constexpr static auto component_tag = "AxiomGT Training";
        auto& monitor = db.monitor();
        monitor.set_monitor( component_tag, 3 );
        monitor.log( component_tag, "Start ... " );

        std::size_t ps_load_for_each_thread = std::ceil( static_cast<double>(cands.size()) / 20 /  db.thread_pool->get_thread_num() );

        monitor.log( component_tag, "Model Training ... " );
        monitor.set_monitor( "axiomGT_training::model_train", 1 , true );

        cands 
        | ba::transformed(
            [
                this
                , &db
                , &cube
                , &ps_names
                , &monitor

            ]( auto mid )
            {
                monitor.thread_monitor_start( "axiomGT_training::model_train" );
                auto& dbmodel      = db.data_likelihood[mid];
                auto& dbprior      = db.axiom_gt_aux.NIW_priors[mid];
                auto& dbposterior  = db.axiom_gt_aux.NIW_posteriors[mid];

                auto& dbcnfromspec = db.axiom_gt_aux.copynumber_categories[mid];
                auto& dbcopynumber = db.axiom_gt_aux.copynumbers;
                auto& dbgenohints  = db.axiom_gt_aux.genohints;
                
                auto& allele_sample ( cube.slice(mid) );
                auto& ps_name      = ps_names[mid];

                const arma::irowvec& mid_copynumbers = dbcopynumber.row(mid);
                const arma::irowvec& mid_genohints   = dbgenohints .row(mid);
                
                for(const auto& spec_copy_number: dbcnfromspec)
                {
                    if( spec_copy_number == 0 )
                    {
                        arma::uvec s_ids_by_spec_cn = arma::find( mid_copynumbers == spec_copy_number );                        
                        this->set_mdl_invalidate( dbmodel, ps_name, spec_copy_number );
                        continue;
                    }
                    check_key_exist( dbprior,     spec_copy_number, ps_name );
                    check_key_exist( dbposterior, spec_copy_number, ps_name );
                    check_key_exist( dbmodel,     spec_copy_number, ps_name );
                    arma::uvec s_ids_by_spec_cn = arma::find( mid_copynumbers == spec_copy_number );

                    const arma::mat&    xydata    = allele_sample.cols( s_ids_by_spec_cn );
                    std::vector<double>  x        = arma::conv_to<std::vector<double>>::from( xydata.row(0) );
                    std::vector<double>  y        = arma::conv_to<std::vector<double>>::from( xydata.row(1) );
                    std::vector< int >  genohints = arma::conv_to<std::vector< int >> ::from(
                        mid_genohints.elem( s_ids_by_spec_cn ) 
                    );
                    std::vector<double> SubsetInbred(x.size(), 0.0); // Inbreeding status, currently deprecated.


                    cpt::component::axiomGT::probeset_param sp;
                    sp.InitializeWithRecommendedParams();
                    sp.prior      = dbprior    [ spec_copy_number ];
                    sp.posterior  = dbposterior[ spec_copy_number ];
                    sp.SetProbesetName( ps_name );
                    sp.UpdateAnalysisParams( this->analysis_params );
                    sp.copynumber = spec_copy_number;

                    if( s_ids_by_spec_cn.size() != 0 )
                    {
                        this->apply_wobble(sp);


                        cpt::component::axiomGT::binned_data zall;
                        this->initialize_bins(zall, x, y, sp.bins, SubsetInbred, genohints, sp.hok, sp.contradictionpenalty);

                        this->integratebrlmmoverlabelings(zall, sp);

                        this->labels_two_posterior(sp, zall);
                    };     

                    mg::GMM data_model;
                    this->create_data_model(data_model, sp.posterior, sp.mix, sp.inflatePRA, sp.copynumber);
                    
                    dbmodel    [spec_copy_number] = { ps_name, data_model, true };
                    dbposterior[spec_copy_number] = sp.posterior;
                }

                monitor.thread_monitor_end( "axiomGT_training::model_train" );

                return 0;
            }
        )
        | ca::parallel_eval(
            *(db.thread_pool)
            , ps_load_for_each_thread
        );


        // monitor.set_monitor("Missing Cluster Imputation", missing_cluster.size() +2);
        // monitor.log("Missing Cluster Imputation", "Start");

        logger << "total missing : "                  << missing_cluster.size() << std::endl;
        logger << "total valid model ( phase 1 ): "   << valid_model_num() << std::endl;
        logger << "total invalid model ( phase 1 ): " << invalid_model_num() << std::endl;

        monitor.log( "axiomGT_training::model_train", "Done", true );
        monitor.log( component_tag, "Complete!!!" );
    }
    std::size_t valid_model_num()
    {
        auto& db = this->mut_data_pool();
        std::size_t sum(0);
        for ( const auto& probeset_info : db.data_likelihood )
        {
            bool v = true;
            for ( const auto& info: probeset_info )
                v and info.second.validate;
            sum += ( v ? 1 : 0 );
        }
        return sum;
    }
    std::size_t invalid_model_num()
    {
        auto& db = this->mut_data_pool();
        std::size_t sum(0);
        for ( const auto& probeset_info : db.data_likelihood )
        {
            bool v = true;
            for ( const auto& info: probeset_info )
                v and info.second.validate;
            sum += ( v ? 0 : 1 );
        }
        return sum;
    }
    virtual void finish() override
    {
        auto& db = this->mut_data_pool();
        db.save_axiomGT_models(db);
        auto& node = db.require_output( "data_likelihood_models" );
        db.add_output_path( 
              node
            , db.get_axiomGT_models_path(db)
        );
        // if(is_posterior_) db.axiom_gt_aux.save_NIW_posteriors(probeset_posteriors_opath, probeset_names);
        if( is_posterior_ )
        {
            auto& probeset_posteriors_opath = probeset_posteriors_opath_ -> get();
            db.axiom_gt_aux.save_NIW_posteriors( probeset_posteriors_opath );
            auto& node = db.require_output( "NIW_posterior_models" );
            // auto  path = probeset_posteriors_opath;
            db.add_output_path( node, probeset_posteriors_opath );
        }

        // db.release( grand_model );
        // db.release( probeset_idx );
    }
    template<class OS = decltype(logger)&>
    void show_mean_info( const GMMean& g_m_mean, OS&& os = logger) const 
    {
        os << "   GMM component num : " << g_m_mean.size() << std::endl;
        os << "   GMM means : " << std::endl;
        for ( auto&& mean : g_m_mean )
        {
            os << "       " << mean[0] << ", " << mean[1] << std::endl;
        }
    }
    template<class OS = decltype(logger)& >
    void show_ob ( const arma::mat& ob, OS&& os = logger) const 
    {
        for ( auto& col : ob | glue(ca::col_range()) )
        {
            os << "(" << col[0] << ", " 
                << col[1] << ")" << std::endl;
        }
    }
    template<class OS = decltype(logger)& >
    void show_model_info( const GMM& gmm, OS&& os = logger ) const 
    {
        os << "gmm model info" << std::endl;
        gmm
            | ca::mdl_components()
            | glue( ::range_indexed(0) )
            | glue( ba::transformed ( 
                [&os] ( auto&& comp_i )
                {
                    os << "   sub_component " << comp_i.index() << std::endl;
                    os << "   mean : " 
                        << comp_i.value().Mean()[0] 
                        << ", "  << comp_i.value().Mean()[1] << std::endl;
                    os << "   cov : " << std::endl;
                    os << comp_i.value().Covariance();
                    os << std::endl;
                    return comp_i.value();
                }
            ))
            | ::endp
            ;
    }
};
}}}
#else
#endif 
