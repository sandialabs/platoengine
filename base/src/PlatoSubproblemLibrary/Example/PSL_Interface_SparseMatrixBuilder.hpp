// PlatoSubproblemLibraryVersion(3): a stand-alone library for the kernel filter for plato.
#pragma once

#include <cstddef>
#include <vector>

#include "PSL_Abstract_SparseMatrixBuilder.hpp"

namespace PlatoSubproblemLibrary
{
namespace AbstractInterface
{
class SparseMatrix;
}

namespace example
{

class SparseMatrixBuilder : public AbstractInterface::SparseMatrixBuilder
{
public:
    SparseMatrixBuilder(AbstractInterface::MpiWrapper* mpi_wrapper);
    ~SparseMatrixBuilder() override;

    size_t get_number_of_passes_over_all_nonzero_entries() override;
    void begin_build(size_t num_rows, size_t num_columns) override;

    bool needs_value_this_pass() override;
    void specify_nonzero(size_t row, size_t column) override;
    void specify_nonzero(size_t row, size_t column, double value) override;
    void advance_pass() override;

    AbstractInterface::SparseMatrix* end_build() override;

    void send_matrix(size_t send_rank, AbstractInterface::SparseMatrix* matrix) override;
    AbstractInterface::SparseMatrix* receive_matrix(size_t recv_rank) override;

    AbstractInterface::SparseMatrix* transpose(AbstractInterface::SparseMatrix* input) override;

protected:
    virtual void reset();

    size_t m_pass;
    size_t m_num_rows;
    size_t m_num_columns;
    std::vector<size_t> m_temporary_for_integer_row_bounds;
    std::vector<size_t> m_integer_row_bounds;
    std::vector<size_t> m_columns;
    std::vector<double> m_data;

};

}
}
