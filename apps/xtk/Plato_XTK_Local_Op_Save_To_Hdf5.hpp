/*
 * Plato_XTK_Local_Op_Save_To_Hdf5.hpp
 *
 *  Created on: Jul 26, 2019
 *      Author: doble
 */

#ifndef BASE_SRC_XTK_PLATO_XTK_LOCAL_OP_SAVE_TO_HDF5_HPP_
#define BASE_SRC_XTK_PLATO_XTK_LOCAL_OP_SAVE_TO_HDF5_HPP_


#include "Plato_XTK_Local_Op.hpp"
#include "Plato_XTK_Application.hpp"

namespace PlatoXTK
{
class Save_To_HDF5: public Local_Op
{
public:
    /******************************************************************************/
    Save_To_HDF5(Plato::XTKApplication* aXTKApp,
                          Plato::InputData &     aNode);
    /******************************************************************************/
    ~Save_To_HDF5();
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


#endif /* BASE_SRC_XTK_PLATO_XTK_LOCALSAVE_TOFROM_HDF5_HPP_ */
