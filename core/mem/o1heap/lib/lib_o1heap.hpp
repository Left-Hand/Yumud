// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
// documentation files (the "Software"), to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
// and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions
// of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
// OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// Copyright (c) Pavel Kirienko
// Authors: Pavel Kirienko <pavel.kirienko@zubax.com>
//
// READ THE DOCUMENTATION IN README.md.

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

namespace lib_o1heap{

/// The semantic version number of this distribution.
static constexpr size_t  O1HEAP_VERSION_MAJOR = 3;

/// The guaranteed alignment depends on the platform pointer width.
static constexpr size_t O1HEAP_ALIGNMENT = (sizeof(void*) * 2U);

/// The overhead is at most O1HEAP_ALIGNMENT bytes large,
/// then follows the user data which shall keep the next fragment aligned.
static constexpr size_t FRAGMENT_SIZE_MIN = (O1HEAP_ALIGNMENT * 2U);

/// This is risky, handle with care: if the allocation amount plus per-fragment overhead exceeds 2**(b-1),
/// where b is the pointer bit width, then ceil(log2(amount)) yields b; then 2**b causes an integer overflow.
/// To avoid this, we put a hard limit on fragment size (which is amount + per-fragment overhead): 2**(b-1)
static constexpr size_t FRAGMENT_SIZE_MAX = ((SIZE_MAX >> 1U) + 1U);

/// Normally we should subtract log2(FRAGMENT_SIZE_MIN) but log2 is bulky to compute using the preprocessor only.
/// We will certainly end up with unused bins this way, but it is cheap to ignore.
static constexpr size_t NUM_BINS_MAX = (sizeof(size_t) * sizeof(char) * 8);

struct [[nodiscard]] Fragment;

/// Size is computed dynamically from (next - this) or (arena_end - this) for the last fragment.
struct [[nodiscard]] FragmentHeader
{
    Fragment* next;       ///< Next fragment in address order; NULL if this is the last fragment.
    uintptr_t prev_used;  ///< Prev pointer in upper bits, 'used' flag in bit 0.
};
static_assert(sizeof(FragmentHeader) == sizeof(void*) * 2U, "Memory layout error");
static_assert(sizeof(FragmentHeader) == O1HEAP_ALIGNMENT, "Memory layout error");

struct [[nodiscard]] Fragment
{
    FragmentHeader header;
    // Everything past the header may spill over into the allocatable space. The header survives across alloc/free.
    Fragment* next_free;  // Next free fragment in the bin; NULL in the last one.
    Fragment* prev_free;  // Same but points back; NULL in the first one.
};
static_assert(sizeof(Fragment) <= FRAGMENT_SIZE_MIN, "Memory layout error");

/// Runtime diagnostic information. This information can be used to facilitate runtime self-testing,
/// as required by certain safety-critical development guidelines.
/// If assertion checks are not disabled, the library will perform automatic runtime self-diagnostics that trigger
/// an assertion failure if a heap corruption is detected.
/// Health checks and validation can be done with o1heapDoInvariantsHold().
///
/// TODO NOTICE: Maintenance of this information takes about a dozen cycles at least, which is quite significant
/// compared to the amount of computation needed to do the actual memory management. In the future, we may add a
/// preprocessor option that disables diagnostics for the benefit of the most performance-sensitive applications.
/// If you find this feature relevant for your use case, consider opening a ticket.
struct [[nodiscard]] O1HeapDiagnostics{
    using Self = O1HeapDiagnostics;
    /// The total amount of memory available for serving allocation requests (heap size).
    /// The maximum allocation size is (capacity - O1HEAP_ALIGNMENT).
    /// This parameter does not include the overhead used up by O1HeapInstance and arena alignment.
    /// This parameter is constant.
    size_t capacity;

    /// The amount of memory that is currently allocated, including the per-fragment overhead and size alignment.
    /// For example, if the application requested a fragment of size 1 byte, the value reported here may be 32 bytes.
    size_t allocated;

    /// The maximum value of 'allocated' seen since initialization. This parameter is never decreased.
    size_t peak_allocated;

    /// The largest amount of memory that the allocator has attempted to allocate (perhaps unsuccessfully)
    /// since initialization (not including the rounding and the allocator's own per-fragment overhead,
    /// so the total is larger). This parameter is never decreased. The initial value is zero.
    size_t peak_request_size;

    /// The number of times an allocation request could not be completed due to the lack of memory or
    /// excessive fragmentation. OOM stands for "out of memory". This parameter is never decreased.
    size_t oom_count;

    static constexpr Self zero(){
        return Self{0U, 0U, 0U, 0U, 0U};
    }
} ;

/// The definition is private, so the user code can only operate on pointers. This is done to enforce encapsulation.
struct [[nodiscard]] O1HeapInstance
{
    Fragment* bins[NUM_BINS_MAX];  ///< Smallest fragments are in the bin at index 0.
    size_t    nonempty_bin_mask;   ///< Bit 1 represents a non-empty bin; bin at index 0 is for the smallest fragments.

    char* arena_end;  ///< Points past the last byte of the arena; used for computing size of the last fragment.

    O1HeapDiagnostics diagnostics;

    /// Obtains the maximum theoretically possible allocation size for this heap instance.
    /// This is useful when implementing std::allocator_traits<Alloc>::max_size.
    size_t o1heapGetMaxAllocationSize() const ;

    /// Performs a basic sanity check on the heap.
    /// This function can be used as a weak but fast method of heap corruption detection.
    /// If the handle pointer is NULL, the behavior is undefined.
    /// The time complexity is constant.
    /// The return value is truth if the heap looks valid, falsity otherwise.
    bool o1heapDoInvariantsHold() const ;

    size_t fragGetSize(const Fragment* const frag) const;


    /// The semantics follows malloc() with additional guarantees the full list of which is provided below.
    ///
    /// If the allocation request is served successfully, a pointer to the newly allocated memory fragment is returned.
    /// The returned pointer is guaranteed to be aligned at O1HEAP_ALIGNMENT.
    ///
    /// If the allocation request cannot be served due to the lack of memory or its excessive fragmentation,
    /// a NULL pointer is returned.
    ///
    /// The function is executed in constant time.
    /// The allocated memory is NOT zero-filled (because zero-filling is a variable-complexity operation).
    void* o1heapAllocate(const size_t amount);

    /// The semantics follows free() with additional guarantees the full list of which is provided below.
    ///
    /// If the pointer does not point to a previously allocated block and is not NULL, the behavior is undefined.
    /// Builds where assertion checks are enabled may trigger an assertion failure for some invalid inputs.
    ///
    /// The function is executed in constant time.
    void o1heapFree(void* const pointer);

    /// Similar to the standard realloc() with a few improvements. Given a previously allocated fragment and a new size,
    /// attempts to resize the fragment.
    ///
    /// - If the pointer is NULL, acts as o1heapAllocate(). The complexity is constant.
    ///
    /// - If the new_amount is zero, acts as o1heapFree() (n.b.: in realloc() this case is implementation-defined).
    ///   The result should be ignored. The complexity is constant.
    ///
    /// - If the new_amount is not greater than the old fragment size, the same memory pointer is always returned;
    ///   the data is not moved and the fragment is shrunk in place. The complexity is constant.
    ///
    /// - If the new_amount is greater than the old fragment but there is enough free space after the fragment
    ///   to expand in-place, the fragment is expanded in place and the same pointer is returned; the data is not moved.
    ///   The complexity is constant. The contents of the expanded memory is undefined.
    ///
    /// - If the new_amount is greater than the old fragment and there is not enough free space after the fragment to
    ///   expand in-place, but there is a suitable free space elsewhere, the data is moved and the new pointer is returned.
    ///   The complexity is LINEAR (sic!) of the size of the old fragment, as it needs to be memmove()d to the new location.
    ///   The library attempts to relocate the data in a nearby or recently used area to improve cache locality.
    ///   The new pointer is returned. The new fragment may overlap the original one.
    ///
    /// - Otherwise, there is not enough memory to expand the fragment as requested. The old fragment remains valid as-is,
    ///   and NULL is returned to indicate failure. The complexity is constant.
    ///
    /// To summarize, the only linear-complexity case is when the new_amount is larger and there is not enough free space
    /// following this fragment, necessitating moving the data to a new place; the library avoids this if at all possible.
    /// Every other case is constant-complexity as the data is not moved.
    void* o1heapReallocate(void* const pointer, const size_t new_amount);
private:
    void rebin(Fragment* const fragment, const size_t fragment_size);
    void unbin(const Fragment* const fragment, const size_t fragment_size);

    friend O1HeapInstance* o1heapInit(void* const base, const size_t size);
};

static constexpr size_t INSTANCE_SIZE_PADDED = ((sizeof(O1HeapInstance) + O1HEAP_ALIGNMENT - 1U) & ~(O1HEAP_ALIGNMENT - 1U));


/// o1heapInit() will fail unless the arena size is at least this large.
/// This value depends only on the machine architecture.
/// The other reason to fail is if the arena pointer is not aligned at O1HEAP_ALIGNMENT.
static constexpr size_t o1heapMinArenaSize = INSTANCE_SIZE_PADDED + FRAGMENT_SIZE_MIN;

/// The arena base pointer shall be aligned at O1HEAP_ALIGNMENT, otherwise NULL is returned.
///
/// The total heap capacity cannot exceed approx. (SIZE_MAX/2). If the arena size allows for a larger heap,
/// the excess will be silently truncated away (no error). This is not a realistic use case because a typical
/// application is unlikely to be able to dedicate that much of the address space for the heap.
///
/// The function initializes a new heap instance allocated in the provided arena, taking some of its space for its
/// own needs (normally about 40..600 bytes depending on the architecture, but this parameter is not characterized).
/// A pointer to the newly initialized instance is returned.
///
/// The function fails and returns NULL iff:
/// - The provided space is less than o1heapMinArenaSize.
/// - The base pointer is not aligned at O1HEAP_ALIGNMENT.
/// - The base pointer is NULL.
///
/// An initialized instance does not hold any resources except for the arena memory.
/// Therefore, if the instance is no longer needed, it can be discarded without any de-initialization procedures.
///
/// The heap is not thread-safe; external synchronization may be required.
O1HeapInstance* o1heapInit(void* const base, const size_t size);


}
