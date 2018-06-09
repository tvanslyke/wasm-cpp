#ifndef VM_OP_ALLOC_MEMORY_RESOURCE_H
#define VM_OP_ALLOC_MEMORY_RESOURCE_H

#include <ciso646>

#if defined(__has_include)
# if __has_include(<memory_resource>)
#  include <memory_resource>
namespace wasm { namespace pmr = ::std::pmr; }
# elif __has_include(<experimental/memory_resource>)
# include <experimental/memory_resource>
namespace wasm { namespace pmr = ::std::experimental::pmr; }
# else
#  error "<memory_resource> or <experimental/memory_resource> must be available." 
# endif /* __has_include(<memory_resource>) */
#endif /* defined(__has_include) */

#endif /* VM_OP_ALLOC_MEMORY_RESOURCE_H */
