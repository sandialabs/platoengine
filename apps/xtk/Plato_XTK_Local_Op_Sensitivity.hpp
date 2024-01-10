/*
 * Plato_XTK_Local_Op_Sensivitiy.hpp
 *
 *  Created on: Jul 3, 2019
 *      Author: doble
 */

#ifndef BASE_SRC_XTK_PLATO_XTK_LOCAL_OP_SENSITIVITY_HPP_
#define BASE_SRC_XTK_PLATO_XTK_LOCAL_OP_SENSITIVITY_HPP_

#include "Plato_XTK_Local_Op.hpp"
#include "Plato_XTK_Application.hpp"

namespace PlatoXTK
{
class Sensitivity: public Local_Op
{
public:
    /******************************************************************************/
    Sensitivity(Plato::XTKApplication* aXTKApp,
                Plato::InputData &     aNode);
    /******************************************************************************/
    ~Sensitivity();
    /******************************************************************************/
    void operator()();
    /******************************************************************************/
    std::string op_name() const;
    /******************************************************************************/


//    virtual void getArguments(std::vector<Plato::LocalArg>& localArgs)=0;
protected:
    bool mSparse;
    Plato::data::layout_t  mOutputLayout;

    std::string mGradientX;
    std::string mGradientY;
    std::string mGradientZ;
    std::string mOutputSharedDataName;

    int mOutputStartFieldIndex;
    int mOutputEndFieldIndex;
};

}




#endif /* BASE_SRC_XTK_PLATO_XTK_LOCAL_OP_SENSITIVITY_HPP_ */
