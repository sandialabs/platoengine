// PlatoSubproblemLibraryVersion(8): a stand-alone library for the kernel filter for plato.
#pragma once

#include "PSL_Abstract_NearestNeighborSearcher.hpp"

#include <vector>

namespace PlatoSubproblemLibrary
{
class PointCloud;
class Point;

class BruteForceNearestNeighbor : public AbstractInterface::NearestNeighborSearcher
{
public:
    BruteForceNearestNeighbor();
    ~BruteForceNearestNeighbor() override;

    // build searcher
    void build(PlatoSubproblemLibrary::PointCloud* answer_points) override;
    // find nearest neighbor
    size_t get_neighbor(PlatoSubproblemLibrary::Point* query_point) override;

protected:

    PlatoSubproblemLibrary::PointCloud* m_answer_points;
};

}
