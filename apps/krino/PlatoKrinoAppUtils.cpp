#include "PlatoKrinoAppUtils.hpp"
#include "Plato_Parser.hpp"
#include "plato/third_party_integration/krino/SphereBuilder.hpp"
#include <Teuchos_CommandLineProcessor.hpp>

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
    tData.mSphereSpacing = getOrThrow<int>(aNode, "sphere_spacing",
                        "ERROR: Levelset definition sphere spacing was not specified.");
    tData.mSphereRadius =
        getOrThrow<double>(aNode, "radius", "ERROR: Levelset definition sphere radius was not specified.");
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

bool parse_command_line_options(int aArgc, char *aArgv[], CommandLineOptions &aCommandLineOptions)
{
    Teuchos::CommandLineProcessor tCLP;
    tCLP.setDocString("plato_krino_main options:");

    tCLP.setOption("bg_mesh", &(aCommandLineOptions.mBackgroundMeshName),
                   "Name of static background mesh that will hold levelset values.");
    tCLP.setOption("cut_mesh", &(aCommandLineOptions.mCutMeshName),
                   "Name of mesh that is cut out of the background mesh using levelset values.");
    tCLP.setOption("krino_operations_file", &(aCommandLineOptions.mKrinoOperationsFileName),
                   "Name of plato/krino operations file.");
    tCLP.setOption("field_mesh_name", &(aCommandLineOptions.mFieldMeshName),
                   "Name of mesh containing field for initializing levelset values.");
    tCLP.setOption("field_name", &(aCommandLineOptions.mFieldName),
                   "Name of field from which levelset values will be initialized.");
    tCLP.setOption("time_step", &(aCommandLineOptions.mFieldDataTimeStep),
                   "Time step from which to initialize levelset values.");
    tCLP.setOption("execute_initial_mesh", "", &(aCommandLineOptions.mExecuteInitialMesh),
                   "Specifies whether to create an initial cut mesh.");
    tCLP.setOption("include_void_region", "", &(aCommandLineOptions.mIncludeVoidRegion),
                   "Specifies whether to include the void regions in the output mesh.");

    Teuchos::CommandLineProcessor::EParseCommandLineReturn parseReturn =
        Teuchos::CommandLineProcessor::PARSE_SUCCESSFUL;
    try
    {
        parseReturn = tCLP.parse(aArgc, aArgv);
    }
    catch (std::exception &exc)
    {
        std::cout << "Failed to parse the command line arguments." << std::endl;
        return false;
    }

    if (parseReturn == Teuchos::CommandLineProcessor::PARSE_SUCCESSFUL) return true;

    return false;
}

}// namespace apps::krino