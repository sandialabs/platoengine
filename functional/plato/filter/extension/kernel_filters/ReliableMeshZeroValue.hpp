#ifndef PLATO_FILTER_EXTENSION_KERNELFILTERS_RELIABLEMESHZEROVALUE
#define PLATO_FILTER_EXTENSION_KERNELFILTERS_RELIABLEMESHZEROVALUE

namespace plato::filter::extension::kernel_filters
{

/// @brief Return the value for what can reliably considered close enough to zero to be zero in exodus mesh files.
[[nodiscard]] constexpr auto reliable_mesh_zero_value() -> double
{
    constexpr auto tReliableZeroInMesh = 1e-6;
    return tReliableZeroInMesh;
}

}  // namespace plato::filter::extension::kernel_filters

#endif
