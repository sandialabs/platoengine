#pragma once

#include "PlatoPythonOperation.hpp"
#include "Plato_InputData.hpp"

#include <memory>

class PythonOperationFactory
{
public:
    PythonOperationFactory() = default;

    std::unique_ptr<PlatoPythonOperation>
    create(const Plato::InputData& aOperationNode);

};