#include <pbos/kf/misc.h>
#include <pbos/kh/kd/gdb.h>
#include <hal/x86_64/proc.hh>

PBOS_EXTERN_C_BEGIN

const char *hali_hex_to_addr(const char *ptr, uintptr_t *int_out) {
	int num_chars = 0;
	int hex_val;

	*int_out = 0;

	while (*ptr) {
		hex_val = kd_gdb_hex_to_digit(*ptr);
		if (hex_val >= 0) {
			*int_out = (*int_out << 4) | hex_val;
			num_chars++;
		} else
			break;

		++ptr;
	}

	return ptr;
}

PBOS_PRIVATE void kh_handle_gdb_packet(const char *packet, size_t len, void *registers) {
	switch (packet[0]) {
		case 'g': {
			hali_gdb_registers_t *gdb_regs = (hali_gdb_registers_t *)registers;

			const uint8_t *raw_regs = (const uint8_t *)gdb_regs;

			char buffer[sizeof(hali_gdb_registers_t) * 2 + 1];

			for (size_t i = 0; i < sizeof(hali_gdb_registers_t); i++) {
				buffer[2 * i] = kd_digit_to_gdb_hex(raw_regs[i] >> 4);
				buffer[2 * i + 1] = kd_digit_to_gdb_hex(raw_regs[i] & 0xf);
			}
			buffer[sizeof(hali_gdb_registers_t) * 2] = '\0';

			ki_kd_send_packet(buffer, sizeof(buffer));
			break;
		}
		case 'm': {
			const char *ptr = &packet[1];
			uintptr_t addr;

			char response[PAGESIZE + sizeof("E01")];
			size_t response_size = 0;
			memset(response, 0, sizeof(response));

			if ((ptr = hali_hex_to_addr(ptr, &addr))) {
				if (*(ptr++) == ',') {
					uintptr_t length;
					if (hali_hex_to_addr(ptr, &length) && (length <= PAGESIZE)) {
						ps::mutex_guard g(ki_gdb_read_pmem_mapping_mutex);

						mm_page_access_t access_begin, access_end;
						mm_getmap(mm_get_cur_context(), (void *)addr, &access_begin);
						mm_getmap(mm_get_cur_context(), (void *)(addr + PAGESIZE - 1), &access_end);

						if ((access_begin & access_end) & MM_PAGE_READ) {
							memcpy(response, (void *)addr, length);
							strcpy(&response[PAGESIZE], "E01");
							response_size = sizeof(response);
						}
					}
				}
			}

			ki_kd_send_packet(response, response_size);

			break;
		}
		case 'k':
			km_panic("The debugger killed the kernel");
		default:
			ki_kd_send_packet("", 0);
	}
}

PBOS_EXTERN_C_END
