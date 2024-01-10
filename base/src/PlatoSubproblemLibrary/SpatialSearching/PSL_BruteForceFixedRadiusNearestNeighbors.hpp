// PlatoSubproblemLibraryVersion(7): a stand-alone library for the kernel filter for plato.
#pragma once

#include "PSL_Abstract_FixedRadiusNearestNeighborsSearcher.hpp"

#include <cstddef>
#include <vector>

namespace PlatoSubproblemLibrary
{
class PointCloud;
class Point;

class BruteForceFixedRadiusNearestNeighbors : public AbstractInterface::FixedRadiusNearestNeighborsSearcher
{
public:
    BruteForceFixedRadiusNearestNeighbors();

    // build searcher
    void build(PlatoSubproblemLibrary::PointCloud* answer_points, double radius) override;
    // find neighbors within radius
    void get_neighbors(PlatoSubproblemLibrary::Point* query_point,
                               std::vector<size_t>& neighbors_buffer,
                               size_t& num_neighbors) override;

protected:
    PlatoSubproblemLibrary::PointCloud* m_answer_points;
    double m_radius;

};

}
