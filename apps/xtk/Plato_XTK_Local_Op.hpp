/*
 * Plato_XTK_Local_Op.hpp
 *
 *  Created on: Jul 3, 2019
 *      Author: doble
 */

#ifndef BASE_SRC_XTK_PLATO_XTK_LOCAL_OP_HPP_
#define BASE_SRC_XTK_PLATO_XTK_LOCAL_OP_HPP_

#include <string>

namespace Plato
{
class XTKApplication;
}

namespace PlatoXTK
{

class Local_Op
{
public:
    Local_Op(Plato::XTKApplication* p) : mXTKApp(p) {}
    virtual ~Local_Op(){}
    virtual void operator()()=0;
    virtual std::string op_name() const = 0;
protected:
    Plato::XTKApplication* mXTKApp;
};

}
#endif /* BASE_SRC_XTK_PLATO_XTK_LOCAL_OP_HPP_ */
