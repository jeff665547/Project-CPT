#pragma once
#include <string>
#include <CPT/application/ui_img_index/option.hpp>
namespace cpt {
namespace application {
namespace ui_img_index {

struct Indexer
{
    virtual void operator() ( 
        const Parameters& param
    ) = 0;
};

}}}
