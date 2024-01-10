/*
 * Plato_XTK_Local_Op_Load_From_Hdf5.hpp
 *
 *  Created on: Jul 26, 2019
 *      Author: doble
 */

#ifndef BASE_SRC_XTK_PLATO_XTK_LOCAL_OP_LOAD_FROM_HDF5_HPP_
#define BASE_SRC_XTK_PLATO_XTK_LOCAL_OP_LOAD_FROM_HDF5_HPP_


#include "Plato_XTK_Local_Op.hpp"
#include "Plato_XTK_Application.hpp"

namespace PlatoXTK
{
class Load_From_HDF5: public Local_Op
{
public:
    /******************************************************************************/
    Load_From_HDF5(Plato::XTKApplication* aXTKApp,
                          Plato::InputData &     aNode);
    /******************************************************************************/
    ~Load_From_HDF5();
    /******************************************************************************/
    void operator()();
    /******************************************************************************/
    std::string op_name() const;
    /******************************************************************************/

//    virtual void getArguments(std::vector<Plato::LocalArg>& localArgs)=0;
protected:
    std::string                         mHdfFileName;
    std::vector<std::string>            mSharedDataName;
    std::vector<Plato::data::layout_t > mFieldLayout;
};

}


#endif /* BASE_SRC_XTK_PLATO_XTK_LOCAL_OP_LOAD_FROM_HDF5_HPP_ */
