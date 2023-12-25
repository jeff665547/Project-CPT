#pragma once
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/serialization/array.hpp>
#include <armadillo>

namespace boost {
namespace serialization {

// arma::Mat<T>
//______________________________________________________________________________

template <class Archive, class T>
void save(Archive& ar, const arma::Mat<T>& obj, unsigned version)
{
    ar & make_nvp("n_rows", obj.n_rows); 
    ar & make_nvp("n_cols", obj.n_cols);
    ar & make_array(obj.memptr(), obj.n_elem);
}
template <class Archive, class T>
void load(Archive& ar, arma::Mat<T>& obj, unsigned version)
{
    size_t n_rows, n_cols;
    ar & make_nvp("n_rows", n_rows); 
    ar & make_nvp("n_cols", n_cols);
    obj.set_size(n_rows, n_cols);
    ar & make_array(obj.memptr(), obj.n_elem);
}
template <class Archive, class T>
void serialize(Archive& ar, arma::Mat<T>& obj, unsigned version)
{
    split_free(ar, obj, version);
}

// arma::Col<T>
//______________________________________________________________________________

template <class Archive, class T>
void save(Archive& ar, const arma::Col<T>& obj, unsigned version)
{
    ar & make_nvp("n_elem", obj.n_elem);
    ar & make_array(obj.memptr(), obj.n_elem);
}
template <class Archive, class T>
void load(Archive& ar, arma::Col<T>& obj, unsigned version)
{
    size_t n_elem;
    ar & make_nvp("n_elem", n_elem); 
    obj.set_size(n_elem);
    ar & make_array(obj.memptr(), obj.n_elem);
}
template <class Archive, class T>
void serialize(Archive& ar, arma::Col<T>& obj, unsigned version)
{
    split_free(ar, obj, version);
}

// arma::Row<T>
//______________________________________________________________________________

template <class Archive, class T>
void save(Archive& ar, const arma::Row<T>& obj, unsigned version)
{
    ar & make_nvp("n_elem", obj.n_elem);
    ar & make_array(obj.memptr(), obj.n_elem);
}
template <class Archive, class T>
void load(Archive& ar, arma::Row<T>& obj, unsigned version)
{
    size_t n_elem;
    ar & make_nvp("n_elem", n_elem); 
    obj.set_size(n_elem);
    ar & make_array(obj.memptr(), obj.n_elem);
}
template <class Archive, class T>
void serialize(Archive& ar, arma::Row<T>& obj, unsigned version)
{
    split_free(ar, obj, version);
}

// arma::Cube<T>
//______________________________________________________________________________

template <class Archive, class T>
void save(Archive& ar, const arma::Cube<T>& obj, unsigned version)
{
    ar & make_nvp("n_rows", obj.n_rows);
    ar & make_nvp("n_cols", obj.n_cols);
    ar & make_nvp("n_slices", obj.n_slices);
    ar & make_array(obj.memptr(), obj.n_elem);
}
template <class Archive, class T>
void load(Archive& ar, arma::Cube<T>& obj, unsigned version)
{
    size_t n_rows, n_cols, n_slices;
    ar & make_nvp("n_rows", n_rows); 
    ar & make_nvp("n_cols", n_cols); 
    ar & make_nvp("n_slices", n_slices); 
    obj.set_size(n_rows, n_cols, n_slices);
    ar & make_array(obj.memptr(), obj.n_elem);
}
template <class Archive, class T>
void serialize(Archive& ar, arma::Cube<T>& obj, unsigned version)
{
    split_free(ar, obj, version);
}

} // end of serialization
} // end of boost
