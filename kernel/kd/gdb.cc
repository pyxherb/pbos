#include <pbos/kh/kd/gdb.h>
#include <pbos/kf/misc.h>
#include <pbos/ki/kd/gdb.h>
#include <pbos/kfxx/scope_guard.hh>
#include "../generated/config.hh"

PBOS_EXTERN_C_BEGIN

PBOS_PRIVATE void *ki_gdb_read_pmem_mapping_vaddr = nullptr;
PBOS_PRIVATE ps::mutex_t ki_gdb_read_pmem_mapping_mutex;

PBOS_API int kd_gdb_hex_to_digit(char digit) {
	switch (digit) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			return digit - '0';
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
			return digit - 'a' + 10;
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
			return digit - 'A' + 10;
		default:
			break;
	}
	return -1;
}

PBOS_API char kd_digit_to_gdb_hex(int num) {
	switch (num) {
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
			return (char)num + '0';
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
			return (char)num - 10 + 'a';
		default:
			km_panic("%s only accepts digit in [0, 16)", __func__);
	}
	PBOS_UNREACHABLE();
}

PBOS_API uint8_t kd_compute_gdb_checksum(const char *data, size_t len) {
	uint8_t sum = 0;
	while (len--)
		sum += (uint8_t)*data++;
	return sum;
}

PBOS_PRIVATE kd_gdb_stub_backend_t ki_kd_current_gdb_stub_backend;

PBOS_API km_result_t kd_set_gdb_stub_backend(kd_gdb_stub_backend_t *backend) {
	KM_RETURN_IF_FAILED(backend->init());
	kfxx::scope_guard deinit_backend_guard([backend]() noexcept {
		backend->deinit();
	});

	backend->flush();

	deinit_backend_guard.release();

	ki_kd_current_gdb_stub_backend.deinit();
	memcpy(&ki_kd_current_gdb_stub_backend, backend, sizeof(*backend));
	return KM_RESULT_OK;
}

PBOS_PRIVATE void ki_kd_send_packet(const char *data, size_t len) {
	uint8_t checksum = kd_compute_gdb_checksum(data, len);
	char c;

	ki_kd_current_gdb_stub_backend.send("$", 1);

	ki_kd_current_gdb_stub_backend.send(data, len);

	ki_kd_current_gdb_stub_backend.send("#", 1);

	c = kd_digit_to_gdb_hex(checksum >> 4);
	ki_kd_current_gdb_stub_backend.send(&c, 1);

	c = kd_digit_to_gdb_hex(checksum & 0xf);
	ki_kd_current_gdb_stub_backend.send(&c, 1);

	ki_kd_current_gdb_stub_backend.flush();
}

PBOS_PRIVATE int ki_kd_recv_packet(char *buffer, size_t len) {
	bool started = false;
	size_t off = 0;
	uint8_t computed_checksum = 0, received_checksum = 0, checksum_nibble = 0;
	bool in_escape = false, in_checksum = false;
	for (;;) {
		int c = ki_kd_current_gdb_stub_backend.recv_char();
		if(c < 0)
			return -1;

		if (!started) {
			switch (c) {
				case '$':
					started = true;
					off = 0;
					in_escape = false;
					in_checksum = false;
					computed_checksum = 0;
				case '+':
					// May be ACK of the last packet, we ignore here.
					continue;
				case '-':
					// NAK, you may request a retransfer here.
					continue;
				default:
					continue;
			}
		} else {
			if (in_escape) {
				c ^= 0x20;
				in_escape = false;
			} else {
				switch (c) {
					case '}':
						in_escape = true;
						continue;
					case '#':
						in_checksum = true;
						received_checksum = 0;
						checksum_nibble = 0;
					default:
						break;
				}
			}

			if(!in_checksum) {
				if(off < len - 1) {
					buffer[off++] = (char)c;
					computed_checksum += (char)c;
				} else {
					// Overlengthed packet, restart to receive.
					started = false;
					continue;
				}
			} else {
				int val = kd_gdb_hex_to_digit((char)c);
                if (val < 0) {
					// Invalid checksum character, discard the packet.
                    started = 0;
                    continue;
                }
                if (checksum_nibble == 0) {
                    computed_checksum = (uint8_t)(val << 4);
                    checksum_nibble = 1;
                } else {
                    received_checksum |= (uint8_t)val;
					// Checksum received.
                    if (computed_checksum == received_checksum) {
                        buffer[off] = '\0';
                        ki_kd_send_ack();
						return KM_RESULT_OK;
                    } else {
						// Checksum error, send NAK and then wait for retransfer.
                        ki_kd_send_nak();
                        started = false;
                    }
                }
			}
		}
	}
}

PBOS_PRIVATE void ki_kd_send_ack() {
	ki_kd_current_gdb_stub_backend.send("+", 1);
	ki_kd_current_gdb_stub_backend.flush();
}

PBOS_PRIVATE void ki_kd_send_nak() {
	ki_kd_current_gdb_stub_backend.send("-", 1);
	ki_kd_current_gdb_stub_backend.flush();
}

PBOS_NORETURN PBOS_PRIVATE void ki_enter_gdb_stub(void *registers) {
	char packet_buffer[KI_GDB_STUB_MAX_PACKET_SIZE];

	{
		char stop_packet[] = "Sstopped";
		ki_kd_send_packet(stop_packet, sizeof(stop_packet) - 1);
	}

	for(;;) {
		int len = ki_kd_recv_packet(packet_buffer, sizeof(packet_buffer));
		if(len < 0)
			// Error receiving the packet, wait for retransfer.
			continue;
		if(!len)
			ki_kd_send_packet("", 0);
		kh_handle_gdb_packet(packet_buffer, (size_t)len, registers);
	}
}

PBOS_EXTERN_C_END
