#pragma once
#include <CPT/range.hpp>
#include <CPT/logger.hpp>
namespace cpt {
namespace component {
namespace axiomGT {
RANGE_NAMESPACE_SHORTCUT
class cluster_data {
public:
	// mean/variance
	// strength of both
	double m;	 	///< mean of cluster
	double ss;	 	///< variance of cluster
	double k;	 	///< strength of mean (pseudo-observations)
	double v;	 	///< strength of variance (pseudo-observations)
	double ym;	 	///< mean of cluster in other dimension
	double yss;	 	///< variance of cluster in other dimension
	double xyss;	///< covariance of cluster in both directions

	cluster_data(){}

	/** set the values */
	void Set(const double& im, 
             const double& iss, 
             const double& ik, 
             const double& iv, 
             const double& iym, 
             const double& iyss, 
             const double& ixyss){
		m    = im;
		ss   = iss;
		k    = ik;
		v    = iv;
		ym   = iym;
		yss  = iyss;
		xyss = ixyss;
	};
  	/** copy from another cluster */
	void Copy(const cluster_data& source){
		m    = source.m;
		ss   = source.ss;
		k    = source.k;
		v    = source.v;
		ym   = source.ym;
		yss  = source.yss;
		xyss = source.xyss;
	};
 	/** do a diagnostic dump */
	void Dump(){
		printf("%lf %lf %lf %lf %lf %lf %lf\n", 
                 m, ss, k, v, ym, yss, xyss);
	}
  	void Clear(){
		m    = 0.0;
		ss   = 0.0;
		k    = 0.0;
		v    = 0.0;
		ym   = 0.0;
		yss  = 0.0;
		xyss = 0.0;
  	};
private:
    friend class boost::serialization::access;
    /** Serialize the cluster data */
    void serialize(boost::archive::xml_oarchive &ar, const unsigned int& version)
    {
        // We just need to serialize each of the members.
        std::vector<double> mean0 {m, ym};
        std::vector<double> covariance0 {ss, xyss, xyss, yss};
        ar & boost::serialization::make_nvp("mean0",                 mean0);
        ar & boost::serialization::make_nvp("k0",                        k);
        ar & boost::serialization::make_nvp("covariance0",     covariance0);
        ar & boost::serialization::make_nvp("v0",                        v);
    }
    void serialize(boost::archive::xml_iarchive &ar, const unsigned int& version)
    {
        // We just need to serialize each of the members.
        std::vector<double> mean0(2);
        std::vector<double> covariance0(4);
        ar & boost::serialization::make_nvp("mean0",                 mean0);
        ar & boost::serialization::make_nvp("k0",                        k);
        ar & boost::serialization::make_nvp("covariance0",     covariance0);
        ar & boost::serialization::make_nvp("v0",                        v);
        m  = mean0[0];             ym  = mean0[1];
        ss = covariance0[0];       yss = covariance0[3];       xyss = covariance0[1];
    }
};

class probeset_distribution{
public:
	std::string  name;
	// prior
	// note: a-b for transform
	// therefore bb allele is negative
	// despite allegations to the contrary
	cluster_data aa; ///< aa allele cluster
	cluster_data ab; ///< ab genotype cluster
	cluster_data bb; ///< bb genotype cluster

	// cross-correlation terms
	// Like everything else, in pseudo-obs
	// Can be asymmetrical because of rotation between variances within clusters
	double xah,  xab,  xhb; ///< cross-correlation terms between clusters x
	double yah,  yab,  yhb; ///< cross-correlation terms between clusters y
	double xyah, xyab, xyhb; ///< cross-correlation terms between clusters x and y
	double yxah, yxab, yxhb; ///< cross-correlation terms between clusters y and x

	probeset_distribution(){}

	void Copy(const probeset_distribution& s)
	{
		aa.Copy(s.aa);
		ab.Copy(s.ab);
		bb.Copy(s.bb);
		xah  = s.xah;
		xab  = s.xab;
		xhb  = s.xhb;
		yah  = s.yah;
		yab  = s.yab;
		yhb  = s.yhb;
		xyah = s.xyah;
		xyab = s.xyab;
		xyhb = s.xyhb;
		yxah = s.yxah;
		yxab = s.yxab;
		yxhb = s.yxhb;
	}
  	void Clear_Cross() {
		xah  = 0.0;
		xab  = 0.0;
		xhb  = 0.0;
		yah  = 0.0;
		yab  = 0.0;
		yhb  = 0.0;
		xyah = 0.0;
		xyab = 0.0;
		xyhb = 0.0;
		yxah = 0.0;
		yxab = 0.0;
		yxhb = 0.0;
  	};
	void Clear() {
		aa.Clear();
		ab.Clear();
		bb.Clear();
		Clear_Cross();
	};
	void Dump() {
		std::cout << "Probeset Name: " << name << std::endl;
		std::cout << "Genotype Info (meanX, varX, nObsMean, nObsVar, meanY, varY, covarXY)" << std::endl;
		std::cout << "AA genotype cluster: " << std::endl;
		aa.Dump();
		std::cout << "AB genotype cluster: " << std::endl;
		ab.Dump();
		std::cout << "BB genotype cluster: " << std::endl;
		bb.Dump();
		std::cout << "cross-correlation terms (cov(AA, AB), cov(AA, BB), cov(AB, BB)): " << std::endl;
		std::cout << "cross-correlation terms between clusters x axis." << std::endl;
		std::cout << xah << "\t" << xab << "\t" << xhb << std::endl;
		std::cout << "cross-correlation terms between clusters y axis." << std::endl;
		std::cout << yah << "\t" << yab << "\t" << yhb << std::endl;
		std::cout << "cross-correlation terms between clusters x axis and y axis." << std::endl;
		std::cout << xyah << "\t" << xyab << "\t" << xyhb << std::endl;
		std::cout << "cross-correlation terms between clusters y axis and x axis." << std::endl;
		std::cout << yxah << "\t" << yxab << "\t" << yxhb << std::endl;
	}
private:
    friend class boost::serialization::access;
    /** Serialize the snp_distribution */
    void serialize(boost::archive::xml_oarchive &ar, const unsigned int& version)
    {
        // We just need to serialize each of the members.
        std::vector<double> btwclusterscovariance0 {xah,  xab,  xhb,
                                                    yah,  yab,  yhb,
                                                    xyah, xyab, xyhb,
                                                    yxah, yxab, yxhb};
		ar & boost::serialization::make_nvp("ProbesetName",     name);
        ar & boost::serialization::make_nvp("distBB",           bb  );
        ar & boost::serialization::make_nvp("distAB",           ab  );
        ar & boost::serialization::make_nvp("distAA",           aa  );
        ar & boost::serialization::make_nvp("btwclusterscovariance0", btwclusterscovariance0);
    }
    void serialize(boost::archive::xml_iarchive &ar, const unsigned int& version)
    {
        // We just need to serialize each of the members.
        std::vector<double> btwclusterscovariance0(12);
        ar & boost::serialization::make_nvp("ProbesetName",     name);
		ar & boost::serialization::make_nvp("distBB",           bb  );
        ar & boost::serialization::make_nvp("distAB",           ab  );
        ar & boost::serialization::make_nvp("distAA",           aa  );
        ar & boost::serialization::make_nvp("btwclusterscovariance0", btwclusterscovariance0);
        xah  = btwclusterscovariance0[0];  xab  = btwclusterscovariance0[1];   xhb  = btwclusterscovariance0[2];
        yah  = btwclusterscovariance0[3];  yab  = btwclusterscovariance0[4];   yhb  = btwclusterscovariance0[5];
        xyah = btwclusterscovariance0[6];  xyab = btwclusterscovariance0[7];   xyhb = btwclusterscovariance0[8];
        yxah = btwclusterscovariance0[9];  yxab = btwclusterscovariance0[10];  yxhb = btwclusterscovariance0[11];
    }	
};

class probeset_param{
public:

	probeset_distribution prior;
	probeset_distribution posterior;
	
	// hints
	int     comvar;                 ///< do we use common variances for all clusters?
	double  lambda;                 ///< control mixing of variances if common
	int     callmethod;             ///< do we use raw labels or posterior distributions?
	int     hardshell;              ///< do we stop cluster centers from being too close?
	double  shellbarrier;           ///< how close can cluster centers be?
	int     bins;                   ///< use quick method
	int     copynumber;             ///< only one copy, for example, XY snps
	int     hints;                  ///< use hinted genotypes as reference
	double  contradictionpenalty;   ///< penalty for contradicting a hint
	int     hok;                    ///< allow hints to be flipped in genotype 0,1,2<->2,1,0
	int     mix;                    ///< turn on mixture distribution penalty
	double  bic;                    ///< bic penalty level bic*nparam*log(n)
	double  CSepPen;                ///< penalty for FLD separation too low
	double  CSepThr;                ///< stop penalizing using Geman-McClure
	double  wobble;                 ///< how much do we allow clusters to shift from prior
	double  copyqc;                 ///< check for possible outlier "size" values
	int     copytype;               ///< how to handle copy number outlier detection
	int     clustertype;            ///< do I handle multiple cluster dimensions
	double  ocean;                  ///< uniform density to compare weird data points against
	double  inflatePRA;             ///< increase variance by uncertainty in mean location when making calls
	double  IsoHetY;                ///< ensure Het > average Hom Y by at least epsilon
    double  MS;                     ///< threshold for no-calls

	// safetyparameter
	double SafetyFrequency;         ///< ensure avoid taking log of zero frequencies

	probeset_param(){}

	/** initialize to some useful defaults */
	void Initialize()
	{
		prior.aa.Set(.66,  .005,  4, 10, 9, 0.1, 0);
		prior.ab.Set(  0,   .01, .2, 10, 9, 0.1, 0);
		prior.bb.Set(-.66, .005,  4, 10, 9, 0.1, 0);
		prior.xah  = prior.xab  = prior.xhb  = 0;
		prior.yah  = prior.yab  = prior.yhb  = 0;
		prior.xyah = prior.xyab = prior.xyhb = 0;
		prior.yxah = prior.yxab = prior.yxhb = 0;
		posterior.Copy(prior);

		comvar                   = 1;
		lambda                   = 1;
		callmethod               = 0;       // labeling
		hardshell                = 2;
		shellbarrier             = 0.05;
		bins                     = 0;       // no bins used
		copynumber               = 2;       // default two copies
		hints                    = 0;
		contradictionpenalty     = 0;       // no penalty, contradiction (to genohints) penalty, (only used when genohints is provided)
		hok                      = 0;       // can homs be flipped in hints? e.g. AA -> BB or BB -> AA, (only used when genohints is provided)
		mix                      = 0;       // no mixture penalty
		bic                      = 0;       // no bic turned on
		wobble                   = 0.00001; // max prior 100,000 data points
		copyqc                   = 0;       // no test for copy qc
		copytype                 = 0;       // standard copy qc method
		clustertype              = 1;       // standard 1-d clustering only
		CSepPen                  = 0;
		CSepThr                  = 16;
		ocean                    = 0;       // similar to copyqc, turn this off normally
		inflatePRA               = 0;       // no increase in uncertainty
		IsoHetY                  = 0;       // nothing here
        MS                       = 0.2;
		SafetyFrequency          = 1;       // at least one count per genotype safety
	};

	void SetAPTGenericPrior()
	{
		prior.name = "GENERIC";
		prior.aa.Set( 2, 0.06, 0.2,  1, 10.5, 0.3, 0);
		prior.ab.Set( 0, 0.06, 0.3, 10, 11.0, 0.3, 0);
		prior.bb.Set(-2, 0.06, 0.2,  1, 10.5, 0.3, 0);
		prior.xah  =  prior.xab  =  prior.xhb  =  -0.1;
		prior.yah  =  prior.yhb  =  -0.05;
        prior.yab  =  -0.1;
		prior.xyah =  prior.xyab =  prior.xyhb =  0;
		prior.yxah =  prior.yxab =  prior.yxhb =  0;
		posterior.Copy(prior);
	}

	void InitializeWithRecommendedParams()
	{
		comvar                   = 1;
		lambda                   = 1.0;
		callmethod               = 1;
		hardshell                = 3;
		shellbarrier             = 0.75;
		bins                     = 100;
		copynumber               = 2;       // default two copies
		hints                    = 0;
		contradictionpenalty     = 16;      // contradiction (to genohints) penalty, (only used when genohints is provided)
		hok                      = 0;       // can homs be flipped in hints? e.g. AA -> BB or BB -> AA, (only used when genohints is provided)
		mix                      = 1;
		bic                      = 2;       // no bic turned on
		wobble                   = 0.05;    // max prior 100,000 data points
		copyqc                   = 0.00000; // no test for copy qc
		copytype                 = -1;
		clustertype              = 2;
		CSepPen                  = 0.1;
		CSepThr                  = 4;
		ocean                    = 0.00001; // similar to copyqc, turn this off normally
		inflatePRA               = 0;       // no increase in uncertainty
		IsoHetY                  = 0;       // nothing here
        MS                       = 0.15;
		SafetyFrequency          = 1;       // at least one count per genotype safety
	}

	void InitializeWithGenericPrior()
	{
		SetAPTGenericPrior();
		InitializeWithRecommendedParams();
	}

	void SetProbesetName( const std::string& ps_name )
	{		
		prior.name     = ps_name;
		posterior.name = ps_name;
	}

 	/** copy parameters */
	void copy(const probeset_param& s)
	{
		// copy prior
		prior.Copy(s.prior);

		// copy posterior
		posterior.Copy(s.posterior);

		// copy algorithm tweaks/parameters
		comvar               = s.comvar;
		lambda               = s.lambda;
		hardshell            = s.hardshell;
		shellbarrier         = s.shellbarrier;
		callmethod           = s.callmethod;
		copynumber           = s.copynumber;
		bins                 = s.bins;
		hints                = s.hints;
		contradictionpenalty = s.contradictionpenalty;
		hok                  = s.hok;
		mix                  = s.mix;
		bic                  = s.bic;
		wobble               = s.wobble;
		copyqc               = s.copyqc;
		copytype             = s.copytype;
		clustertype          = s.clustertype;
		CSepPen              = s.CSepPen;
		CSepThr              = s.CSepThr;
		ocean                = s.ocean;
		inflatePRA           = s.inflatePRA;
		IsoHetY              = s.IsoHetY;
        MS                   = s.MS;
		SafetyFrequency      = s.SafetyFrequency;
	}
	/** get only the prior value from some source*/
	void GetPrior(const probeset_param& s)
	{
		// just copy the prior data over
		// keep the current methods
		prior.Copy(s.prior);
	}
	/** transfer prior directly to posterior */
	void Prior2Posterior()
	{
		posterior.Copy(prior);
	}

	void UpdateAnalysisParams(const probeset_param& s)
	{
		// overall used analysis params in this aptaxiomGT training process
		wobble               = s.wobble;

        bins 				 = s.bins;
        hok				     = s.hok;
        contradictionpenalty = s.contradictionpenalty;

		hints                = s.hints;
        SafetyFrequency 	 = s.SafetyFrequency;
        bic 				 = s.bic;
        hardshell 			 = s.hardshell;
        shellbarrier 		 = s.shellbarrier;
        comvar 				 = s.comvar;
        CSepThr 			 = s.CSepThr;
        CSepPen 			 = s.CSepPen;

        lambda 				 = s.lambda;

        mix 				 = s.mix;
        inflatePRA 			 = s.inflatePRA;

		ocean                = s.ocean;
	}
    // void setDocValues(SelfDoc& doc) {
    //     // prior strength
    //     doc.setOptValue("KX",   std::to_string(prior.aa.k));
    //     doc.setOptValue("KH",   std::to_string(prior.ab.k));
    //     doc.setOptValue("V",    std::to_string(prior.ab.v));

    //     // prior centers
    //     doc.setOptValue("BBM",  std::to_string(prior.bb.m));
    //     doc.setOptValue("ABM",  std::to_string(prior.ab.m));
    //     doc.setOptValue("AAM",  std::to_string(prior.aa.m));

	//     doc.setOptValue("BBY",  std::to_string(prior.bb.ym));
	//     doc.setOptValue("ABY",  std::to_string(prior.ab.ym));
	//     doc.setOptValue("AAY",  std::to_string(prior.aa.ym));

    //     // prior variances
    //     doc.setOptValue("AAV",  std::to_string(prior.aa.ss));
    //     doc.setOptValue("BBV",  std::to_string(prior.bb.ss));
    //     doc.setOptValue("ABV",  std::to_string(prior.ab.ss));

	//     doc.setOptValue("AAYV", std::to_string(prior.aa.yss));
	//     doc.setOptValue("BBYV", std::to_string(prior.bb.yss));
	//     doc.setOptValue("ABYV", std::to_string(prior.ab.yss));

	//     doc.setOptValue("AAXY", std::to_string(prior.aa.xyss));
	//     doc.setOptValue("ABXY", std::to_string(prior.ab.xyss));
	//     doc.setOptValue("BBXY", std::to_string(prior.bb.xyss));

	//     //Cryptic between-cluster terms
    //     doc.setOptValue("KXX",  std::to_string(prior.xab));
    //     doc.setOptValue("KAH",  std::to_string(prior.xah));
    //     doc.setOptValue("KHB",  std::to_string(prior.xhb));
 	//     // Y versions
	//     doc.setOptValue("KYAB", std::to_string(prior.yab));
	//     doc.setOptValue("KYAH", std::to_string(prior.yah));
	//     doc.setOptValue("KYHB", std::to_string(prior.yhb));

	//     // other parameters
    //     doc.setOptValue("COMVAR",       std::to_string(comvar));
    //     doc.setOptValue("HARD",         std::to_string(hardshell));
    //     doc.setOptValue("SB",           std::to_string(shellbarrier));
    //     doc.setOptValue("CM",           std::to_string(callmethod));
    //     doc.setOptValue("bins",         std::to_string(bins));
    //     doc.setOptValue("hints",        std::to_string(hints));
    //     doc.setOptValue("CP",           std::to_string(contradictionpenalty));
	//     doc.setOptValue("Hok",          std::to_string(hok));
    //     doc.setOptValue("mix",          std::to_string(mix));
    //     doc.setOptValue("bic",          std::to_string(bic));
	//     doc.setOptValue("CSepPen",      std::to_string(CSepPen));
	//     doc.setOptValue("CSepThr",      std::to_string(CSepThr));
    //     doc.setOptValue("lambda",       std::to_string(lambda));
    //     doc.setOptValue("wobble",       std::to_string(wobble));
    //     doc.setOptValue("copyqc",       std::to_string(copyqc));
	//     doc.setOptValue("copytype",     std::to_string(copytype));
	//     doc.setOptValue("clustertype",  std::to_string(clustertype));
	//     doc.setOptValue("ocean",        std::to_string(ocean));
	//     doc.setOptValue("inflatePRA",   std::to_string(inflatePRA));
	//     doc.setOptValue("IsoHetY",      std::to_string(IsoHetY));
    //     doc.setOptValue("MS",           std::to_string(MS));
    // }
    void dump_model() {
        std::cout << std::endl << "===========> Prior Info <===========" << std::endl;
        prior.Dump();
        std::cout << std::endl << "=========> Posterior Info <=========" << std::endl;
        posterior.Dump();
    }

	void dump_params() {
		std::cout << "comvar: " 				<< comvar 				<< std::endl;
		std::cout << "lambda: " 				<< lambda 				<< std::endl;
		std::cout << "callmethod: " 			<< callmethod 			<< std::endl;
		std::cout << "hardshell: " 				<< hardshell 			<< std::endl;
		std::cout << "shellbarrier: " 			<< shellbarrier 		<< std::endl;
		std::cout << "bins: " 					<< bins 				<< std::endl;
		std::cout << "copynumber: " 			<< copynumber 			<< std::endl;
		std::cout << "hints: " 					<< hints 				<< std::endl;
		std::cout << "contradictionpenalty: " 	<< contradictionpenalty << std::endl;
		std::cout << "hok: " 					<< hok 					<< std::endl;
		std::cout << "mix: " 					<< mix 					<< std::endl;
		std::cout << "bic: " 					<< bic 					<< std::endl;
		std::cout << "CSepPen: " 				<< CSepPen 				<< std::endl;
		std::cout << "CSepThr: " 				<< CSepThr 				<< std::endl;
		std::cout << "wobble: " 				<< wobble 				<< std::endl;
		std::cout << "copyqc: " 				<< copyqc 				<< std::endl;
		std::cout << "copytype: " 				<< copytype 			<< std::endl;
		std::cout << "clustertype: " 			<< clustertype 			<< std::endl;
		std::cout << "ocean: " 					<< ocean 				<< std::endl;
		std::cout << "inflatePRA: " 			<< inflatePRA 			<< std::endl;
		std::cout << "IsoHetY: " 				<< IsoHetY 				<< std::endl;
		std::cout << "MS: " 					<< MS 					<< std::endl;
		std::cout << "SafetyFrequency: " 		<< SafetyFrequency 		<< std::endl;
	}
};
}}}
