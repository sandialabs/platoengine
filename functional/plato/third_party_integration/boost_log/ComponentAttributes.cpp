#include "plato/third_party_integration/boost_log/ComponentAttributes.hpp"

#include <boost/log/expressions.hpp>

#include "plato/components/ComponentTypeColors.hpp"
#include "plato/components/ComponentTypeStream.hpp"
#include "plato/third_party_integration/boost_log/FormattingUtilities.hpp"
#include "plato/utilities/Colorize.hpp"
#include "plato/utilities/StringUtilities.hpp"

namespace plato::third_party_integration::boost_log
{
[[nodiscard]] auto ComponentTypeAndNameAttribute::formatter(const FormattingStyle aFormattingStyle)
    -> boost::log::formatter
{
    namespace ble = boost::log::expressions;

    using StreamType = boost::log::formatter::stream_type;
    const auto tFormatter = [aFormattingStyle](const boost::log::record_view& aRecord, StreamType& aStream)
    {
        if (const auto tComponentAndName = boost::log::extract<typename ComponentTypeAndNameAttribute::AttributeType>(
                ComponentTypeAndNameAttribute::name().data(), aRecord))
        {
            const auto tColorFormat =
                color_code(components::component_text_color(tComponentAndName.get().mComponentType), aFormattingStyle);
            const auto tComponentAndNameAsString =
                utilities::concatenate(components::to_string(tComponentAndName.get().mComponentType), ':',
                                       tComponentAndName.get().mComponentName);

            aStream << "[" << tColorFormat << tComponentAndNameAsString
                    << color_code(utilities::TextColor::kDefault, aFormattingStyle) << "] ";
        }
    };

    return boost::log::formatter{tFormatter};
}

auto operator<<(std::ostream& aStream, const ComponentTypeAndName& aComponentTypeAndName) -> std::ostream&
{
    const auto tComponentAndNameAsString = utilities::concatenate(
        components::to_string(aComponentTypeAndName.mComponentType), ':', aComponentTypeAndName.mComponentName);
    aStream << utilities::colorize(tComponentAndNameAsString,
                                   components::component_text_color(aComponentTypeAndName.mComponentType));
    return aStream;
}
}  // namespace plato::third_party_integration::boost_log
