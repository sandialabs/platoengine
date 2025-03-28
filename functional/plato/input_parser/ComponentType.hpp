#ifndef PLATO_INPUT_PARSER_COMPONENTTYPE
#define PLATO_INPUT_PARSER_COMPONENTTYPE

namespace plato::input_parser
{
/// @brief The allowable types of components. These map to general categories of objects that may be instantiated to
/// create a workflow.
enum struct ComponentType
{
    kObjective,
    kConstraint,
    kGeometry,
    kFilter,
    kProcessManager,
    kNumberOfEnumerates
};

}  // namespace plato::input_parser

#endif
