#pragma once

void arma_save_col_double(
    const std::string& fname
  , const arma::Col<double>& col
  , const bool transpose
) {
    if (!transpose)
        col.save(fname, arma::raw_ascii);
    else
        col.t().eval().save(fname, arma::raw_ascii);
}
void arma_save_row_double(
    const std::string& fname
  , const arma::Row<double>& row
  , const bool transpose
) {
    if (!transpose)
        row.save(fname, arma::raw_ascii);
    else
        row.t().eval().save(fname, arma::raw_ascii);
}
void arma_save_mat_double(
    const std::string& fname
  , const arma::Mat<double>& mat
  , const bool transpose
) {
    if (!transpose)
        mat.save(fname, arma::raw_ascii);
    else
        mat.t().eval().save(fname, arma::raw_ascii);
}
void arma_save_cube_double(
    const std::string& fname
  , const arma::Cube<double>& cube
) {
    cube.save(fname, arma::raw_ascii);
}
void arma_save_cube_double_slice(
    const std::string& fname
  , const arma::Cube<double>& cube
  , const size_t s
  , const bool transpose
) {
    arma_save_mat_double(fname, cube.slice(s), transpose);
}

