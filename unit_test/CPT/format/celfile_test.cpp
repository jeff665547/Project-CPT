// bool xxxload2 = false;
#include <iostream>
#include <Nucleona/app/cli/gtest.hpp>
#include <CPT/format/celfile.hpp>
#include <string>
using namespace std::string_literals;
TEST( celwrite, test )
{
    auto pdata = cpt::format::CELFile::load( 
        "/home/alex/data/array/GSE28111/raw_data/GSM696325_Set1_1.CEL"
        // EXAMPLE_ROOT + "/GSM2066668_206-001_CHB.CEL"s 
    );
    // auto& celdata = *dynamic_cast<const cpt::format::CELDataImpl<59, 1>* >(pdata.get());
    // EXPECT_TRUE( cpt::format::CELFile::save(
    //     "./GSM696325_Set1_1.rewrite.CEL"
    //     // EXAMPLE_ROOT + "/output/GSM2066668_206-001_CHB.rewrite.CEL"s
    //     , celdata 
    // ));
    
    // xxxload2 = true;
    // auto pdata2 = cpt::format::CELFile::load( 
    //     EXAMPLE_ROOT + "/output/GSM2066668_206-001_CHB.rewrite.CEL"s
    // );
    // TODO compare pdata & pdata2
    
    std::ofstream txt(
        "./GSM696325_Set1_1.rewrite.CEL"

        // EXAMPLE_ROOT + "/output/GSM2066668_206-001_CHB.txt"s
    );
    pdata->print_all( txt );

    // std::ofstream txt_sub( 
    //     EXAMPLE_ROOT + "/output/GSM2066668_206-001_CHB.rewrite.txt"s
    // );
    // pdata2->print_all( txt_sub );
}
