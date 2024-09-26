#include "PlatoKrinoAppUtils.hpp"
#include "Plato_Parser.hpp"

namespace apps::krino_app
{

template <typename T>
T getOrThrow(const Plato::InputData &aInputData, const std::string &aName, const std::string &aError)
{
    T tReturnValue;
    if (aInputData.size<std::string>(aName) == 0)
    {
        throw std::runtime_error(aError.c_str());
    }
    if constexpr (std::is_same_v<T, bool>)
    {
        tReturnValue = Plato::Get::Bool(aInputData, aName);
    }
    else if constexpr (std::is_same_v<T, int>)
    {
        tReturnValue = Plato::Get::Int(aInputData, aName);
    }
    else if constexpr (std::is_same_v<T, double>)
    {
        tReturnValue = Plato::Get::Double(aInputData, aName);
    }
    else
    {
        throw std::runtime_error(aError.c_str());
    }
    return tReturnValue;
}

LevelsetPrimitives readLevelsetInitializationData(Plato::InputData &aAppFileData)
{
    LevelsetPrimitives tPrims;
    if (!aAppFileData.empty())
    {
        for (auto tNode : aAppFileData.getByName<Plato::InputData>("Operation"))
        {
            std::string tStrName = Plato::Get::String(tNode, "Name");
            if (tStrName == "Initialize Levelsets")
            {
                auto tSpherePatternNodes = tNode.getByName<Plato::InputData>("SpherePattern");
                for (auto tCurSpherePatternNode : tSpherePatternNodes)
                {
                    createSpheresFromPattern(tCurSpherePatternNode, tPrims);
                }
                auto tSphereNodes = tNode.getByName<Plato::InputData>("Sphere");
                for (auto tCurSphereNode : tSphereNodes)
                {
                    createSphere(tCurSphereNode, tPrims);
                }
                auto tPlaneNodes = tNode.getByName<Plato::InputData>("Plane");
                for (auto tCurPlaneNode : tPlaneNodes)
                {
                    createPlane(tCurPlaneNode, tPrims);
                }
            }
        }
    }
    return tPrims;
}

void createSpheresFromPattern(const Plato::InputData &aNode, LevelsetPrimitives &aPrims)
{
    SpherePatternData tData = readSpherePatternData(aNode);
    std::vector<Sphere> tSpheres = generate_spheres(tData);
    aPrims.mSpheres.insert(aPrims.mSpheres.end(), tSpheres.begin(), tSpheres.end());
}

void createSphere(const Plato::InputData &aNode, LevelsetPrimitives &aPrims)
{
    Sphere tSphere = readSphereData(aNode);
    aPrims.mSpheres.push_back(tSphere);
}

void createPlane(const Plato::InputData &aNode, LevelsetPrimitives &aPrims)
{
    Plane tPlane = readPlaneData(aNode);
    aPrims.mPlanes.push_back(tPlane);
}

SpherePatternData readSpherePatternData(const Plato::InputData &aNode)
{
    SpherePatternData tData;
    tData.mBoundingBoxMinXYZ = {
        getOrThrow<double>(aNode, "bbox_xmin", "ERROR: Levelset definition bounding box xmin was not specified."),
        getOrThrow<double>(aNode, "bbox_ymin", "ERROR: Levelset definition bounding box ymin was not specified."),
        getOrThrow<double>(aNode, "bbox_zmin", "ERROR: Levelset definition bounding box zmin was not specified.")};
    tData.mBoundingBoxMaxXYZ = {
        getOrThrow<double>(aNode, "bbox_xmax", "ERROR: Levelset definition bounding box xmax was not specified."),
        getOrThrow<double>(aNode, "bbox_ymax", "ERROR: Levelset definition bounding box ymax was not specified."),
        getOrThrow<double>(aNode, "bbox_zmax", "ERROR: Levelset definition bounding box zmax was not specified.")};
    tData.mNumSpheres = {
        getOrThrow<int>(aNode, "num_spheres_x",
                        "ERROR: Levelset definition number of spheres in the x direction was not specified."),
        getOrThrow<int>(aNode, "num_spheres_y",
                        "ERROR: Levelset definition number of spheres in the y direction was not specified."),
        getOrThrow<int>(aNode, "num_spheres_z",
                        "ERROR: Levelset definition number of spheres in the z direction was not specified.")};
    tData.mSphereRadius =
        getOrThrow<double>(aNode, "radius", "ERROR: Levelset definition sphere radius was not specified.");
    tData.mSpheresCanOverlapBoundingBox =
        getOrThrow<bool>(aNode, "overlap_bbox", "ERROR: Levelset definition overlap param not specified.");
    return tData;
}

Sphere readSphereData(const Plato::InputData &aNode)
{
    double tCenterX =
        getOrThrow<double>(aNode, "center_x", "ERROR: Levelset definition sphere center_x was not specified.");
    double tCenterY =
        getOrThrow<double>(aNode, "center_y", "ERROR: Levelset definition sphere center_y was not specified.");
    double tCenterZ =
        getOrThrow<double>(aNode, "center_z", "ERROR: Levelset definition sphere center_z was not specified.");
    double tRadius = getOrThrow<double>(aNode, "radius", "ERROR: Levelset definition sphere radius was not specified.");
    return Sphere{tCenterX, tCenterY, tCenterZ, tRadius};
}

Plane readPlaneData(const Plato::InputData &aNode)
{
    double tNormalX =
        getOrThrow<double>(aNode, "normal_x", "ERROR: Levelset definition plane normal_x was not specified.");
    double tNormalY =
        getOrThrow<double>(aNode, "normal_y", "ERROR: Levelset definition plane normal_y was not specified.");
    double tNormalZ =
        getOrThrow<double>(aNode, "normal_z", "ERROR: Levelset definition plane normal_z was not specified.");
    double tOffset = getOrThrow<double>(aNode, "offset", "ERROR: Levelset definition plane offset was not specified.");
    return Plane{tNormalX, tNormalY, tNormalZ, tOffset};
}

}// namespace apps::krino