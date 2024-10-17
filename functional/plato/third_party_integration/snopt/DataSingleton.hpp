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

    /// @brief Const access to the instance held by this singleton.
    [[nodiscard]] static auto constInstance() -> const DataSingleton &;

    /// @brief Returns `true` if the data member is set.
    [[nodiscard]] auto hasData() const -> bool;

    [[nodiscard]] auto data() -> std::optional<T> &;

    /// @brief Returns the data held by this singleton.
    /// @pre hasData must return `true`.
    [[nodiscard]] auto data() const -> const T &;

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
auto DataSingleton<T, Tag>::constInstance() -> const DataSingleton<T, Tag> &
{
    return instance();
}

template <typename T, typename Tag>
auto DataSingleton<T, Tag>::hasData() const -> bool
{
    return mData.has_value();
}

template <typename T, typename Tag>
auto DataSingleton<T, Tag>::data() -> std::optional<T> &
{
    return mData;
}

template <typename T, typename Tag>
auto DataSingleton<T, Tag>::data() const -> const T &
{
    assert(mData);
    return mData.value();  // NOLINT
}

template <typename T, typename Tag>
void DataSingleton<T, Tag>::reset()
{
    mData = std::nullopt;
}

}  // namespace plato::third_party_integration::snopt

#endif
