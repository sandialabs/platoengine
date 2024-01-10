// PlatoSubproblemLibraryVersion(3): a stand-alone library for the kernel filter for plato.
#pragma once

#include <vector>
#include <map>
#include <cstddef>
#include "PSL_Abstract_SparseMatrix.hpp"

namespace PlatoSubproblemLibrary
{
namespace AbstractInterface
{
class MpiWrapper;
}

namespace example
{

class CompressedRowSparseMatrix : public AbstractInterface::SparseMatrix
{
public:
    CompressedRowSparseMatrix(const size_t num_rows,
                              const size_t num_columns,
                              const std::vector<size_t>& integer_row_bounds,
                              const std::vector<size_t>& columns,
                              const std::vector<double>& data);
    virtual ~CompressedRowSparseMatrix();

    size_t getNumRows() override;
    size_t getNumColumns() override;

    void matVec(const std::vector<double>& input, std::vector<double>& output, bool transpose = false) override;
    void matVecToReduced(const std::vector<double>& input, std::vector<double>& output, bool transpose = false) override;

    void rowNormalize(const std::vector<double>& rowNormalizationFactors) override;
    void columnNormalize(const std::vector<double>& columnNormalizationFactors) override;

    void getNonZeroSortedRows(std::vector<size_t>& nonZeroRows) override;
    size_t getNumNonZeroSortedRows() override;
    void getNonZeroSortedColumns(std::vector<size_t>& nonZeroColumns) override;
    size_t getNumNonZeroSortedColumns() override;

    void getRow(size_t row, std::vector<double>& data, std::vector<size_t>& columns) override;
    void setRow(size_t row, const std::vector<double>& data) override;

    std::vector<size_t> m_matrix_row_bounds;
    std::vector<size_t> m_matrix_columns;
    std::vector<double> m_matrix_data;

private:
    void internal_build_nonzero_sorted_rows_and_columns();

    size_t m_num_rows;
    size_t m_num_columns;

    bool m_built_nonzero_sorted_rows_and_columns;
    std::vector<size_t> m_nonzero_sorted_rows;
    std::vector<size_t> m_nonzero_sorted_columns;
    std::map<size_t, size_t> m_full_column_to_reduced_column;

    CompressedRowSparseMatrix(const CompressedRowSparseMatrix &);
    CompressedRowSparseMatrix operator=(const CompressedRowSparseMatrix &);
};

CompressedRowSparseMatrix* transposeCompressedRowSparseMatrix(CompressedRowSparseMatrix* input);
void sendCompressedRowSparseMatrix(AbstractInterface::MpiWrapper* mpi_wrapper, size_t rank, CompressedRowSparseMatrix* input);
CompressedRowSparseMatrix* receiveCompressedRowSparseMatrix(AbstractInterface::MpiWrapper* mpi_wrapper, size_t rank);

}

}
