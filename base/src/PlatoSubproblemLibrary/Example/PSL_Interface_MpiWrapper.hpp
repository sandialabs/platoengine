// PlatoSubproblemLibraryVersion(6): a stand-alone library for the kernel filter for plato.
#pragma once

#include "PSL_Abstract_MpiWrapper.hpp"

#include <vector>
#include <cstddef>
#include <mpi.h>

namespace PlatoSubproblemLibrary
{
namespace AbstractInterface
{
class GlobalUtilities;
}

namespace example
{

class Interface_MpiWrapper : public AbstractInterface::MpiWrapper
{
public:
    Interface_MpiWrapper(AbstractInterface::GlobalUtilities* utilities, MPI_Comm* comm);
    ~Interface_MpiWrapper() override;

    size_t get_rank() override;
    size_t get_size() override;
    double get_time() override;

    void send(size_t target_rank, std::vector<int>& send_vector) override;
    void send(size_t target_rank, std::vector<float>& send_vector) override;
    void send(size_t target_rank, std::vector<double>& send_vector) override;

    void receive(size_t source_rank, std::vector<int>& send_vector) override;
    void receive(size_t source_rank, std::vector<float>& send_vector) override;
    void receive(size_t source_rank, std::vector<double>& send_vector) override;

    void all_gather(std::vector<int>& local_portion, std::vector<int>& global_portion) override;
    void all_gather(std::vector<float>& local_portion, std::vector<float>& global_portion) override;
    void all_gather(std::vector<double>& local_portion, std::vector<double>& global_portion) override;

    void all_reduce_min(std::vector<int>& local_contribution, std::vector<int>& global) override;
    void all_reduce_min(std::vector<float>& local_contribution, std::vector<float>& global) override;
    void all_reduce_min(std::vector<double>& local_contribution, std::vector<double>& global) override;

    void all_reduce_sum(std::vector<int>& local_contribution, std::vector<int>& global_result) override;
    void all_reduce_sum(std::vector<float>& local_contribution, std::vector<float>& global_result) override;
    void all_reduce_sum(std::vector<double>& local_contribution, std::vector<double>& global_result) override;

    void all_reduce_max(std::vector<int>& local_contribution, std::vector<int>& global) override;
    void all_reduce_max(std::vector<float>& local_contribution, std::vector<float>& global) override;
    void all_reduce_max(std::vector<double>& local_contribution, std::vector<double>& global) override;

    void broadcast(size_t source_rank, std::vector<int>& broadcast_vector) override;
    void broadcast(size_t source_rank, std::vector<float>& broadcast_vector) override;
    void broadcast(size_t source_rank, std::vector<double>& broadcast_vector) override;

protected:
    MPI_Comm* m_comm;

};

}
}
