#ifndef _PBOS_IODEV_KBD_H_
#define _PBOS_IODEV_KBD_H_

#include <pbos/dm/device.h>
#include <pbos/dm/devcls.h>

PBOS_EXTERN_C_BEGIN

#define IODEV_KBD_KMOD_NAME "kbdcls"

#define IODEV_KBD_DEVICE_DIR_NAME "kbd"

enum {
	KBD_EVENT_IN_ECHO = 0x00,
	KBD_EVENT_IN_KEY_DOWN,
	KBD_EVENT_IN_KEY_UP,
	KBD_EVENT_IN_KEY_REPEAT,
	KBD_EVENT_IN_SELF_TEST_PASSED,
	KBD_EVENT_IN_SELF_TEST_FAILED,
};

enum {
	KBD_EVENT_OUT_ECHO = 0x00,
	KBD_EVENT_OUT_SET_TYPEMATIC_PARAMS,
	KBD_EVENT_OUT_GET_TYPEMATIC_PARAMS,
	KBD_EVENT_OUT_RESET_PARAMS,
	KBD_EVENT_OUT_SELF_TEST,
};

typedef uint8_t kbd_typematic_rate_t;
typedef uint8_t kbd_typematic_delay_t;

typedef uint8_t kbd_event_code_t;

typedef struct _kbd_key_event_data_t {
	uint32_t key_code;
} kbd_key_event_data_t;

typedef struct _kbd_typematic_event_data_t {
	kbd_typematic_rate_t rate;
	kbd_typematic_delay_t delay;
} kbd_typematic_event_data_t;

typedef struct _kbd_event_in_t {
	kbd_event_code_t event_code;
	union {
		kbd_key_event_data_t as_key;
		kbd_typematic_event_data_t as_typematic;
	} data;
} kbd_event_in_t;

typedef struct _kbd_event_out_t {
	kbd_event_code_t event_code;
	union {
		kbd_key_event_data_t as_key;
		kbd_typematic_event_data_t as_typematic;
	} data;
} kbd_event_out_t;

typedef struct _kbd_device_t kbd_device_t;

typedef km_result_t (*kbd_enable_op_t)(io_dispatch_context_t *dc, dm_device_t *device);
typedef km_result_t (*kbd_disable_op_t)(io_dispatch_context_t *dc, dm_device_t *device);
typedef km_result_t (*kbd_send_event_t)(io_dispatch_context_t *dc, dm_device_t *device, const kbd_event_in_t *event_in);
typedef km_result_t (*kbd_receive_event_t)(io_dispatch_context_t *dc, dm_device_t *device, const kbd_event_out_t *event_out);
typedef km_result_t (*kbd_connect_op_t)(io_dispatch_context_t *dc, dm_device_t *device);
typedef km_result_t (*kbd_disconnect_op_t)(io_dispatch_context_t *dc, dm_device_t *device);

typedef struct _kbd_device_ops_t {
	kbd_enable_op_t enable;
	kbd_disable_op_t disable;
	kbd_send_event_t send_event;
	kbd_receive_event_t receive_event;
	kbd_connect_op_t connect;
	kbd_disconnect_op_t disconnect;
} kbd_device_ops_t;

km_result_t kbd_register_device(dm_device_t *device, const kbd_device_ops_t *ops, dm_device_t **kbd_device_out);
void kbd_unregister_device(dm_device_t *kbd_device);

PBOS_EXTERN_C_END

#endif
