#include "application_manager/hmi_capabilities_converter.h"
#include "application_manager/smart_object_keys.h"
#include "interfaces/HMI_API.h"

namespace application_manager {

bool HMICapabilitiesConverter::ConvertDisplayCapability(
    smart_objects::SmartObject& display_capability) {
  if (!ArrayConvertPattern(display_capability,
                           strings::window_type_supported,
                           &ConvertWindowTypeCapabilities)) {
    return false;
  }

  if (!ArrayConvertPattern(display_capability,
                           strings::window_capabilities,
                           &ConvertWindowCapability)) {
    return false;
  }
  return true;
}

bool HMICapabilitiesConverter::ConvertWindowTypeCapabilities(
    smart_objects::SmartObject& window_type) {
  if (window_type.keyExists(strings::type)) {
    return ConvertStringToEnumValue<hmi_apis::Common_WindowType::eType>(
        window_type[strings::type]);
  }
  return false;
}

bool HMICapabilitiesConverter::ConvertWindowCapability(
    smart_objects::SmartObject& window_capability) {
  if (!ArrayConvertPattern(
          window_capability, strings::text_fields, &ConvertTextField)) {
    return false;
  }

  if (!ArrayConvertPattern(
          window_capability, strings::image_fields, &ConvertImageField)) {
    return false;
  }

  if (!ArrayConvertPattern(
          window_capability,
          strings::image_type,
          ConvertStringToEnumValue<hmi_apis::Common_ImageType::eType>)) {
    return false;
  }

  if (!ArrayConvertPattern(window_capability,
                           strings::button_capabilities,
                           &ConvertButtonCapabilities)) {
    return false;
  }

  return true;
}

bool HMICapabilitiesConverter::ConvertTextField(
    smart_objects::SmartObject& text_field) {
  if (!text_field.keyExists(strings::name) ||
      !ConvertStringToEnumValue<hmi_apis::Common_TextFieldName::eType>(
          text_field[strings::name])) {
    return false;
  }
  if (!text_field.keyExists(strings::character_set) ||
      !ConvertStringToEnumValue<hmi_apis::Common_CharacterSet::eType>(
          text_field[strings::character_set])) {
    return false;
  }
  return true;
}

bool HMICapabilitiesConverter::ConvertImageField(
    smart_objects::SmartObject& image_field) {
  if (!image_field.keyExists(strings::name) ||
      !ConvertStringToEnumValue<hmi_apis::Common_ImageFieldName::eType>(
          image_field[strings::name])) {
    return false;
  }

  if (!ArrayConvertPattern(
          image_field,
          strings::image_type_supported,
          ConvertStringToEnumValue<hmi_apis::Common_FileType::eType>,
          Required::MANDATORY)) {
    return false;
  }
  return true;
}

bool HMICapabilitiesConverter::ConvertButtonCapabilities(
    smart_objects::SmartObject& button_capabilities) {
  if (!button_capabilities.keyExists(strings::name)) {
    return false;
  }
  return ConvertStringToEnumValue<hmi_apis::Common_ButtonName::eType>(
      button_capabilities[strings::name]);
}

bool HMICapabilitiesConverter::ArrayConvertPattern(
    smart_objects::SmartObject& obj,
    const std::string& key,
    const std::function<bool(smart_objects::SmartObject&)>& parser,
    const Required required) {
  if (!obj.keyExists(key)) {
    return OPTIONAL == required;
  }
  auto array = obj[key].asArray();
  if (!array) {
    return false;
  }
  for (auto& item : *array) {
    if (!parser(item)) {
      return false;
    }
  }
  return true;
}

}  // namespace application_manager
