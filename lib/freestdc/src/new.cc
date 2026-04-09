#include <new>

void operator delete(void *, size_t) noexcept {
}
