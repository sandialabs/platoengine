#include "Plato_ESP.hpp"
#include "Plato_KokkosTypes.hpp"

extern "C" {
#include "egads.h"
#include "bodyTess.h"
#include "common.h"
#include "OpenCSM.h"
// the 'undef' and 'define' below are a work around.  OpenCSM defines SUCCESS which 
// conflicts with a definition in opempi.
#undef SUCCESS
#define OCSM_SUCCESS 0

/* undocumented EGADS function */
extern int EG_loadTess(ego body, const char *name, ego *tess);
}

namespace Plato {
namespace Geometry {

template <typename ScalarType, typename ScalarVectorType>
class ESPImpl : public ESP<ScalarType,ScalarVectorType> {
public:
ESPImpl(std::string aModelFileName, std::string aTessFileName, int aParameterIndex=-1) :
    ESP<ScalarType,ScalarVectorType>(aModelFileName, aTessFileName, aParameterIndex)
{
    openContext();
    openModel();
    checkModel();
    buildGeometryAndGetBodies();
    tesselate();
}
virtual ~ESPImpl() {
    safeFreeOCSM();
    safeFreeContext();
}
private:
    using VectorType = typename ESP<ScalarType,ScalarVectorType>::VectorType;

    std::string  mActiveParameterName;
    int          mActiveParameterIndex, mBuiltTo;

    static constexpr int mSpaceDim = 3;

    ego context = nullptr;
    void *model = nullptr;
    modl_T *modelT = nullptr;

    void cleanUpAndThrow(std::string aError);
    void tesselate();
    ScalarType computeSensitivity(VectorType& aDXDp);
    void buildGeometryAndGetBodies();
    void activateParameterInModel(const std::string& aParameterName);
    void openContext();
    void openModel();
    void checkModel();
    void safeFreeOCSM();
    void safeFreeContext();
};


template <typename ScalarType, typename ScalarVectorType>
void ESPImpl<ScalarType,ScalarVectorType>::tesselate()
{
    int tCntr=0;
    std::string tTessBaseName("");
    std::size_t tPos = this->mTessFileName.find(".eto");
    tTessBaseName = this->mTessFileName.substr(0, tPos);

    int tNumActiveBodies=0;
    for (int ibody=1; ibody<=modelT->nbody; ibody++)
    {
        if (modelT->body[ibody].onstack == 1)
            tNumActiveBodies++;
    }

    /* store the tessellation object in OpenCSM */
    for (int ibody=1; ibody<=modelT->nbody; ibody++)
    {
        if (modelT->body[ibody].onstack != 1) continue;
        if (modelT->body[ibody].botype == OCSM_NULL_BODY) continue;
        ego tBody = modelT->body[ibody].ebody;
        if (modelT->body[ibody].etess != NULL)
        {
            EG_deleteObject(modelT->body[ibody].etess);
        }

        std::string tCurTessName = this->mTessFileName;
        if(tNumActiveBodies > 1)
        {
            tCurTessName = tTessBaseName + "_" + std::to_string(tCntr++) + ".eto";
        }
       
        auto tStatus = EG_loadTess(tBody, (char*)tCurTessName.c_str(), &modelT->body[ibody].etess);
        if (tStatus != EGADS_SUCCESS)
        {
            cleanUpAndThrow("EG_loadTess failed.");
        }
    }
    int tNumParams = this->mParameterNames.size();
    for(int iParam=0; iParam<tNumParams; iParam++)
    {
        activateParameterInModel(this->mParameterNames[iParam]);
        computeSensitivity(this->mSensitivity[iParam]);
    }
}

template <typename ScalarType, typename ScalarVectorType>
ScalarType ESPImpl<ScalarType,ScalarVectorType>::computeSensitivity(VectorType& aDXDp)
{
    ScalarType tSensitivity(0.0);

    aDXDp.clear();

    /* clear all then set the parameter & tell OpenCSM */
    ocsmSetVelD(model, 0,     0,    0,    0.0);
    ocsmSetVelD(model, this->mActiveParameterIndex, /*irow=*/ 1, /*icol=*/ 1, 1.0);
    int tBuildTo = 0;
    int tNbody   = 0;
    ocsmBuild(model, tBuildTo, &this->mBuiltTo, &tNbody, NULL);
    
    /* retrieve the sensitivities for the active bodies */
    int tNface, tNedge, tNvert, tNtri;
    int *tTris(nullptr);
    ScalarType *tCoords(nullptr);
    verTags *tVtags;
    for (int ibody = 1; ibody <= modelT->nbody; ibody++)
    {
        if (modelT->body[ibody].onstack != 1) continue;
        if (modelT->body[ibody].botype  == OCSM_NULL_BODY) continue;
        auto tStatus = bodyTess(modelT->body[ibody].etess, &tNface, &tNedge, &tNvert, &tCoords, &tVtags, &tNtri, &tTris);
        if (tStatus != EGADS_SUCCESS)
        {
           cleanUpAndThrow("bodyTess failed.");
        }

        VectorType tCurDXDpVector = VectorType(tNvert*this->mSpaceDim);

        const ScalarType *tPcsens;
    
        /* fill in the Nodes */
        for (int j=0; j<tNvert; j++)
        {
            if (tVtags[j].ptype != 0) continue;
            tStatus = ocsmGetTessVel(model, ibody, OCSM_NODE, tVtags[j].pindex, &tPcsens);
            if (tStatus != EGADS_SUCCESS)
            {
                EG_free(tTris);
                EG_free(tVtags);
                EG_free(tCoords);
                std::stringstream ss;
                ss << "ocsmGetTessVel Parameter " << this->mActiveParameterName << " vert " << j+1 << " failed: " 
                    << tStatus << " (Node = " << tVtags[j].pindex << ")!";
                cleanUpAndThrow(ss.str());
            }
            tCurDXDpVector[3*j  ] = tPcsens[0];
            tCurDXDpVector[3*j+1] = tPcsens[1];
            tCurDXDpVector[3*j+2] = tPcsens[2];
        }
    
        /* next do all of the edges */
        int tIndex;
        for (int j=1; j<=tNedge; j++)
        {
            tStatus = ocsmGetTessVel(model, ibody, OCSM_EDGE, j, &tPcsens);
            if (tStatus != EGADS_SUCCESS)
            {
                EG_free(tTris);
                EG_free(tVtags);
                EG_free(tCoords);
                std::stringstream ss;
                ss << " ocsmGetTessVel Parameter " << this->mActiveParameterName << " Edge " << j << " failed: " << tStatus;
                cleanUpAndThrow(ss.str());
            }
            for (int k = 0; k < tNvert; k++)
            {
                if ((tVtags[k].ptype > 0) && (tVtags[k].pindex == j)) {
                    tIndex       = tVtags[k].ptype - 1;
                    tCurDXDpVector[3*k  ] = tPcsens[3*tIndex  ];
                    tCurDXDpVector[3*k+1] = tPcsens[3*tIndex+1];
                    tCurDXDpVector[3*k+2] = tPcsens[3*tIndex+2];
                }
            }
        }
            
        /* do all of the faces */
        for (int j=1; j<=tNface; j++) {
            tStatus = ocsmGetTessVel(model, ibody, OCSM_FACE, j, &tPcsens);
            if (tStatus != EGADS_SUCCESS) {
            EG_free(tTris);
            EG_free(tVtags);
            EG_free(tCoords);
            std::stringstream ss;
            ss << " ocsmGetTessVel Parameter " << this->mActiveParameterName << " Face " << j << "failed: " << tStatus;
            cleanUpAndThrow(ss.str());
            }
            for (int k=0; k<tNvert; k++)
            if ((tVtags[k].ptype < 0) && (tVtags[k].pindex == j)) {
                tIndex       = -tVtags[k].ptype - 1;
                tCurDXDpVector[3*k  ] = tPcsens[3*tIndex  ];
                tCurDXDpVector[3*k+1] = tPcsens[3*tIndex+1];
                tCurDXDpVector[3*k+2] = tPcsens[3*tIndex+2];
            }
        }
    
        EG_free(tTris);
        EG_free(tVtags);
        EG_free(tCoords);
        
        aDXDp.insert(aDXDp.end(), tCurDXDpVector.begin(), tCurDXDpVector.end());

    }
    return tSensitivity;
}

template <typename ScalarType, typename ScalarVectorType>
void ESPImpl<ScalarType,ScalarVectorType>::buildGeometryAndGetBodies()
{
    int tBuildTo = 0; /* all */
    int tNbody   = 0;
    auto tStatus = ocsmBuild(model, tBuildTo, &this->mBuiltTo, &tNbody, NULL);
    EG_deleteObject(context); context = nullptr;
    if (tStatus != OCSM_SUCCESS)
    {
        cleanUpAndThrow("ocsmBuild failed");
    }
    tNbody = 0;
    for (int ibody=1; ibody<=modelT->nbody; ibody++)
    {
        if (modelT->body[ibody].onstack != 1) continue;
        if (modelT->body[ibody].botype  == OCSM_NULL_BODY) continue;
        tNbody++;
    }
    if (tNbody <= 0)
    {
        cleanUpAndThrow("No bodies found.");
    }
/*
    if (tNbody != 1) {
        cleanUpAndThrow(" ERROR: ETO option only works for a single body!");
    }
*/
}

template <typename ScalarType, typename ScalarVectorType>
void ESPImpl<ScalarType,ScalarVectorType>::activateParameterInModel(const std::string& aParameterName)
{
    int tNumRows(1), tNumCols(1); // no matrix variables allowed currently
    int tType(0); // not sure what this variable is for
    char tParameterName[129];
    for (int j=0; j<modelT->npmtr; j++)
    {
        auto tStatus = ocsmGetPmtr(model, j+1, &tType, &tNumRows, &tNumCols, tParameterName);
        if (tStatus != OCSM_SUCCESS) {
            cleanUpAndThrow("ocsmGetPmtr failed.");
        }
        if (strcmp(aParameterName.c_str(), tParameterName) == 0) {
            this->mActiveParameterIndex = j+1;
            this->mActiveParameterName = aParameterName;
            break;
        }
    }
    if (this->mActiveParameterIndex == 0)
    {
        std::stringstream ss;
        ss << "Parameter not found: " << aParameterName;
        cleanUpAndThrow(ss.str());
    }
}

template <typename ScalarType, typename ScalarVectorType>
void ESPImpl<ScalarType,ScalarVectorType>::openContext()
{
    auto tStatus = EG_open(&context);
    if (tStatus != EGADS_SUCCESS)
    {
        cleanUpAndThrow("EGADS failed to Open");
    }
}

template <typename ScalarType, typename ScalarVectorType>
void ESPImpl<ScalarType,ScalarVectorType>::openModel()
{
    auto tStatus = ocsmLoad((char*)this->mModelFileName.c_str(), &model);
    if (tStatus < OCSM_SUCCESS)
    {
        cleanUpAndThrow("ocsmLoad failed");
    }
    modelT = (modl_T*)model;
    modelT->context = context;
}

template <typename ScalarType, typename ScalarVectorType>
void ESPImpl<ScalarType,ScalarVectorType>::checkModel()
{
    auto tStatus = ocsmCheck(model);
    if (tStatus < OCSM_SUCCESS)
    {
        cleanUpAndThrow("ocsmCheck failed");
    }
}

template <typename ScalarType, typename ScalarVectorType>
void ESPImpl<ScalarType,ScalarVectorType>::safeFreeOCSM()
{
    if (model != nullptr)
    {
        ocsmFree(model);
        ocsmFree(nullptr);
    }
}

template <typename ScalarType, typename ScalarVectorType>
void ESPImpl<ScalarType,ScalarVectorType>::safeFreeContext()
{
    if (context != nullptr)
    {
        EG_close(context);
    }
}

template <typename ScalarType, typename ScalarVectorType>
void ESPImpl<ScalarType,ScalarVectorType>::cleanUpAndThrow(std::string aError)
{
    safeFreeOCSM();
    safeFreeContext();
    this->throwWithError(aError);
}

template class ESPImpl<double,std::vector<double>>;
template class ESPImpl<double,Plato::ScalarVectorT<double>::HostMirror>;

extern "C"
ESP<double,std::vector<double>>* createESP(std::string aModelFileName, std::string aTessFileName, int aParameterIndex)
{
    return new ESPImpl<double,std::vector<double>>(aModelFileName,aTessFileName,aParameterIndex);
}

extern "C"
void destroyESP(ESP<double,std::vector<double>>* esp)
{
    delete esp;
}

} // end namespace Geometry
} // end namespace Plato
