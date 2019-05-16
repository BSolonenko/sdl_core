#ifndef SRC_COMPONENTS_APPLICATION_MANAGER_INCLUDE_APPLICATION_MANAGER_HMI_CAPABILITIES_PARSER_H_
#define SRC_COMPONENTS_APPLICATION_MANAGER_INCLUDE_APPLICATION_MANAGER_HMI_CAPABILITIES_PARSER_H_
#include <functional>
#include <string>
#include "smart_objects/enum_schema_item.h"
#include "smart_objects/smart_object.h"

namespace application_manager {

class HMICapabilitiesConverter {
 public:
  static bool ConvertDisplayCapability(
      smart_objects::SmartObject& display_capability);
  static bool ConvertWindowTypeCapabilities(
      smart_objects::SmartObject& window_type);
  static bool ConvertWindowCapability(
      smart_objects::SmartObject& window_capability);
  static bool ConvertTextField(smart_objects::SmartObject& text_field);
  static bool ConvertImageField(smart_objects::SmartObject& image_field);
  static bool ConvertButtonCapabilities(
      smart_objects::SmartObject& button_capabilities);

  /*
   * @brief Convert string to the equivalent enum value
   *
   * @return enum value that equivalent str_value
   */
  template <typename EnumType>
  static bool ConvertStringToEnumValue(smart_objects::SmartObject& str_value);

 private:
  enum Required { MANDATORY, OPTIONAL };
  static bool ArrayConvertPattern(
      smart_objects::SmartObject& obj,
      const std::string& key,
      const std::function<bool(smart_objects::SmartObject&)>& parser,
      const Required required = OPTIONAL);
};

template <typename EnumType>
bool HMICapabilitiesConverter::ConvertStringToEnumValue(
    smart_objects::SmartObject& str_value) {
  if (smart_objects::SmartType::SmartType_String != str_value.getType()) {
    return false;
  }

  EnumType enum_value;
  if (smart_objects::EnumConversionHelper<EnumType>::StringToEnum(
          str_value.asString(), &enum_value)) {
    str_value = enum_value;
  } else {
    str_value = EnumType::INVALID_ENUM;
  }
  return true;
}

}  // namespace application_manager

#endif  // SRC_COMPONENTS_APPLICATION_MANAGER_INCLUDE_APPLICATION_MANAGER_HMI_CAPABILITIES_PARSER_H_
