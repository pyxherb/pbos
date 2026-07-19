#include <pbos/ki/io/context.hh>

_io_context_t::_io_context_t(kfxx::allocator_t *allocator): registered_isrs(allocator) {
}
