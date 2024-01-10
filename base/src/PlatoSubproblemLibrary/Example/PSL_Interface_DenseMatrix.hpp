// PlatoSubproblemLibraryVersion(3): a stand-alone library for the kernel filter for plato.
#pragma once

#include <vector>
#include <cstddef>

#include "PSL_Abstract_DenseMatrix.hpp"

namespace PlatoSubproblemLibrary
{
namespace AbstractInterface
{
class GlobalUtilities;
class DenseMatrixBuilder;
}

namespace example
{

class Interface_DenseMatrix : public AbstractInterface::DenseMatrix
{
public:
    Interface_DenseMatrix(AbstractInterface::GlobalUtilities* global,
                          AbstractInterface::DenseMatrixBuilder* builder);
    ~Interface_DenseMatrix() override;

    void receive(double** data, size_t num_rows, size_t num_columns);

    size_t get_num_rows() override;
    size_t get_num_columns() override;
    double get_value(size_t row, size_t column) override;
    void set_value(size_t row, size_t column, double value) override;

    void matvec(const std::vector<double>& in, std::vector<double>& out, bool transpose) override;
    void fill(double alpha) override;
    void fill_by_row_major(const std::vector<double>& in) override;
    void set_to_identity() override;
    void copy(AbstractInterface::DenseMatrix* source) override;
    void aXpY(double alpha, AbstractInterface::DenseMatrix* X) override;
    void matrix_matrix_product(double alpha,
                                       AbstractInterface::DenseMatrix* X, bool X_transpose,
                                       AbstractInterface::DenseMatrix* Y, bool Y_transpose) override;
    void scale(double alpha) override;
    double dot(AbstractInterface::DenseMatrix* other) override;
    void get_row(const int& row_index, std::vector<double>& row) override;
    void get_column(const int& column_index, std::vector<double>& column) override;
    void get_diagonal(std::vector<double>& diagonal) override;
    void permute_columns(const std::vector<int>& permutation) override;
    void scale_column(const int& column_index, const double& scale) override;

    double** m_data;

protected:

    size_t m_num_rows;
    size_t m_num_columns;
};

}
}
