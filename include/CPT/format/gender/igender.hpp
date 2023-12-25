#pragma once
#include <CPT/range.hpp>

namespace cpt {
namespace format {
namespace gender {

struct IGender
{   
    enum GenderType { Female, Male };
    bool valid( const std::string& gender_str, GenderType& gender )
    {
        if( (gender_str == "1") || (gender_str == "male") || (gender_str == "Male") )
            gender = GenderType::Male;
        else if( (gender_str == "0") || (gender_str == "female") || (gender_str == "Female") )
            gender = GenderType::Female;
        else
            return false;
        return true;
    }
    virtual std::vector<std::string> get_sample_names() const = 0;
    virtual std::vector<GenderType> get_genders() const = 0;

  private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const GenderType& g, const unsigned int& version)
    {
        ar & boost::serialization::make_nvp("Gender", g);
    }
};

} // end of namespace gender
} // end of namespace format
} // end of namespace cpt
