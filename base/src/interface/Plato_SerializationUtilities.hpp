#ifndef PLATO_SERIALIZATIONUTILITIES_H
#define PLATO_SERIALIZATIONUTILITIES_H

#include "Plato_Interface.hpp"
#include "Plato_SerializationLoadSave.hpp"

namespace Plato{

template<typename T>
void registerApplicationWithInterfaceLoadFromXML(
    Interface& aInterface, 
    T* aApplication, 
    const XMLFileName& aFileName, 
    const XMLNodeName& aNodeName)
{
    aInterface.tryFCatchInterfaceExceptions(
        [aApplication, &aFileName, &aNodeName](){
            Plato::loadFromXML(*aApplication, aFileName, aNodeName);
        });
    aInterface.registerApplicationOnlyInitializeMPI(aApplication);
}

}

#endif