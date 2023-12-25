================================================================================

All the components need to be designed for single or multi channel data and the maxium channel number may could be 5.

All the components must be careful to overwrite the data in the datapool, because another component may still need that data.

At the end of each component, it must make a report of the data for log or somthig.

================================================================================

Component list:

( at least 28 components )

    0. CenFile Reader

    1. Sample QC
        1 Sample Dish QC
        2 Sample Call Rate QC
        3 Sample Plates QC

        A
    2. Probe Filter
        1 Kill List
        2 Mismatch
        3 GC

    3. Quantile Normalization
        1 Mean
        2 Average

    4. Allele Summarization
        1 AvgDiff             - Average measurement PM and MM Probe
        2 Dabg                - Detected Above BackGround analysis
        3 IterPlier           - Iteratively Calling of PLIER
        4 Median              - Compute Median
        5 Median Polish       - Median Polish
        6 Plier               - Probe Logarithmic Error Intensity Estimate
        7 SEA                 - Simplified Expression Analysis
        8 Log Scale Transform - This is in effect generalized log without the log transformation

    5. Space Transformation
        1 MvA - Minus vs Average
        2 RvT - R vs Theta (polar)
        3 CES - Contrast Extremes Stretch
        4 CCS - Contrast Centers Stretch

    6. Genotyping Method
        1 DM Call
        2 BRLMM Call
        3 BRLMMP Call
        4 BirdSeedv1 Call
        5 BirdSeedv2 Call

    7. SNP QC
        Different types of SNP Polisher

    8. Reporter 

================================================================================

Component of BRLMMP Call:

    Setup stuff
        Intensity analysis martix
        Copy Number info

    Haploid snps for gender caller
        Compute gender sample covariate by EM

    Setup stuff
        Read snp prior map
        HaploidSnps
        GendersSnps
        Sample covariate inbreeding penalty

    Normalization of covariates from sample file
        Fit EM with SampleContrast and SampleStrength

    Assign sample copy number for each SNP
        Separate male and female

    Get and construct prior for each subset

    Bayes Update
        Initialize and setup bins
        Computing the relative likelihood of the data
        Turn into approximate posterior distribution
        Make calls based on mean/variance/confidence

    Computing standardized distance

================================================================================

Workflow of BRLMMP in APT code:

    set variable here
        setup seed and DM stuff 
        setup genotyping stuff
        setup gender stuff
        setup inbred sample covariate stuff
    
    get the layout of probesets on the chip
        get kill-list
        get mismatch
        get GC
        get Pm Allele Match
    
    make a temporary map of probeset ids for QCing probeset-ids list
        to run all probeset or only run for snpqc probeset
    
    set the quantification method
        call and set different method here
            BRLMMP - read snp prior map 
    
    load up marker by sample specific cn info - readCopyNumberFile
    setup chrx-snp or special-snps separately
    pick snp for prior or not
    pick snp for normalizaiont or not
    read genders or inbreedin gcovariates sample
    setup DM call BRLMM only
    setup intensity analysis martix
    setup haploid snps vector for gender caller
    
    compute gender sample covariate by internal data alone, by EM?
    initial DM calls, BRLMM only
    
    setup covariates depending between different method
    
    analysis setup
        learning prior for gender or inbred
        setup for spcific method
            BRLMMP - set SpecialSnps or HaploidSnps
            BRLMMP - set GendersSnps
            BRLMMP - add sample covariate inbreeding penalty
            BRLMMP - set normalization based on covariates for each sample
    
    do analysis
        setup for spcific method
            BRLMMP - check is everything ready
            BRLMMP - fill probeset into A_allele and B_allele
            BRLMMP - A allele summarize
            BRLMMP - B allele summarize
            BRLMMP - transform the data 
            BRLMMP - loadInitialCallsForSnp
        compute estimate for spcific method
            BRLMMP - setup things for calling
                BRLMMP - snp specific prior
                BRLMMP - calls and confidences
                BRLMMP - distances for each genotype
                BRLMMP - look up male and female copy
                BRLMMP - assign sample copy numbers for this snp ( male and female )
            BRLMMP - do subsets by copy number
                BRLMMP - assign subset to current copy number
                BRLMMP - getPrior
                    BRLMMP - getSequentialPrior
                        BRLMMP - QuantLabelZ__SnpPriorFromStrings, construct prior from strings
                    BRLMMP - search for the snp prior
                BRLMMP - bayes_label, takes data and turns it into genotypes with no seeds needed
                    BRLMMP - setup posterior by copy prior
                    BRLMMP - apply wobble dor cluster shifts
                    BRLMMP - initialize and setup bins
                    BRLMMP - computing the relative likelihood of the data
                    BRLMMP - turn into approximate "posterior" distribution
                    BRLMMP - make calls based on mean/variance/confidence
                    BRLMMP - make calls only by posterior distribution
                    BRLMMP - qc check for weird value
                    BRLMMP - computer posterior qc
                    BRLMMP - modify posterior confidences qc
                BRLMMP - transfer data to calls
                BRLMMP - computing standardized distance

================================================================================
