#pragma once
#include <CPT/exception.hpp>
namespace cpt {
namespace exception {

struct UnknownSNPForm : public cpt::Exception
{
    using Base = cpt::Exception;
    using Base::Base;
};

}}
