#include <pbos/dm/device.h>
#include <pbos/ki/dm/devcls.hh>

#define DM_BUILTIN_DEVICE_CLASS_MACRO(name) { DM_DEVICE_CLASS_##name, #name }

static const std::pair<kf_uuid_t, const char *> DM_DEVICE_CLASSES_TO_REGISTER[] = {
	DM_BUILTIN_DEVICE_CLASSES
};

void ki_register_device_classes() {
	km_result_t result;
	dm_device_class_t *device_class;

	for (const auto &i : DM_DEVICE_CLASSES_TO_REGISTER) {
		if (KM_FAILED(result = dm_register_device_class(&i.first, &device_class))) {
			km_panic("Error registering built-in device class: %s, result=%.08x", i.second, result);
		}
	}
}
