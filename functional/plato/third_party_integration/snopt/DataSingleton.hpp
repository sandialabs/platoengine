#ifndef PLATO_THIRD_PARTY_INTEGRATION_SNOPT_DATASINGLETON
#define PLATO_THIRD_PARTY_INTEGRATION_SNOPT_DATASINGLETON

#include <optional>

namespace plato::third_party_integration::snopt
{
/// @brief A singleton for storing static data.
///
/// The purpose of this class is to provide access to data that snopt needs in the
/// free functions used for evaluating an objective and its gradient.
///
/// @tparam Tag This template parameter may be used to disambiguate DataSingleton instances that might have the same
/// type.
template <typename T, typename Tag = T>
class DataSingleton
{
   public:
    [[nodiscard]] static auto instance() -> DataSingleton &;

    [[nodiscard]] auto data() -> std::optional<T> &;
    [[nodiscard]] auto data() const -> const std::optional<T> &;

    void reset();

   private:
    DataSingleton() = default;
    DataSingleton &operator=(const DataSingleton &) = delete;
    DataSingleton &operator=(DataSingleton &&) = delete;
    DataSingleton(const DataSingleton &) = delete;
    DataSingleton(DataSingleton &&) = delete;

    std::optional<T> mData;
};

template <typename T, typename Tag>
auto DataSingleton<T, Tag>::instance() -> DataSingleton<T, Tag> &
{
    static DataSingleton sInstance;
    return sInstance;
}

template <typename T, typename Tag>
auto DataSingleton<T, Tag>::data() -> std::optional<T> &
{
    return mData;
}

template <typename T, typename Tag>
auto DataSingleton<T, Tag>::data() const -> const std::optional<T> &
{
    return mData;
}

template <typename T, typename Tag>
void DataSingleton<T, Tag>::reset()
{
    mData = std::nullopt;
}

}  // namespace plato::third_party_integration::snopt

#endif
