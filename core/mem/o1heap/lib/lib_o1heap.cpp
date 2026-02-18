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

// ReSharper disable CppDFANullDereference CppRedundantCastExpression

#include "lib_o1heap.hpp"
#include "lib_o1heap_port.hpp"
#include <limits.h>
#include <stdint.h>
#include <string.h>

namespace lib_o1heap{

// ---------------------------------------- INTERNAL DEFINITIONS ----------------------------------------


static_assert((O1HEAP_ALIGNMENT & (O1HEAP_ALIGNMENT - 1U)) == 0U, "Not a power of 2");
static_assert((FRAGMENT_SIZE_MIN & (FRAGMENT_SIZE_MIN - 1U)) == 0U, "Not a power of 2");
static_assert((FRAGMENT_SIZE_MAX & (FRAGMENT_SIZE_MAX - 1U)) == 0U, "Not a power of 2");



/// The amount of space allocated for the heap instance.
/// Its size is padded up to O1HEAP_ALIGNMENT to ensure correct alignment of the allocation arena that follows.


static_assert(INSTANCE_SIZE_PADDED >= sizeof(O1HeapInstance), "Invalid instance footprint computation");
static_assert((INSTANCE_SIZE_PADDED % O1HEAP_ALIGNMENT) == 0U, "Invalid instance footprint computation");

static constexpr size_t larger(const size_t a, const size_t b)
{
    return (a > b) ? a : b;
}

/// Undefined for zero argument.
static constexpr uint_fast8_t log2Floor(const size_t x)
{
    O1HEAP_ASSERT(x > 0);
    // NOLINTNEXTLINE redundant cast to the same type.
    return (uint_fast8_t) (((sizeof(x) * CHAR_BIT) - 1U) - ((uint_fast8_t) O1HEAP_CLZ(x)));
}

/// Raise 2 into the specified power.
/// You might be tempted to do something like (1U << power). WRONG! We humans are prone to forgetting things.
/// If you forget to cast your 1U to size_t or ULL, you may end up with undefined behavior.
static constexpr size_t pow2(const uint_fast8_t power)
{
    return ((size_t) 1U) << power;
}

/// This is equivalent to pow2(log2Ceil(x)). Undefined for x<2.
static constexpr size_t roundUpToPowerOf2(const size_t x)
{
    O1HEAP_ASSERT(x >= 2U);
    // NOLINTNEXTLINE redundant cast to the same type.
    return ((size_t) 1U) << ((sizeof(x) * CHAR_BIT) - ((uint_fast8_t) O1HEAP_CLZ(x - 1U)));
}

// ---------------------------------------- FRAGMENT HEADER ACCESSORS ----------------------------------------

#define ASSUME_ALIGNED_FRAG_PTR(x) O1HEAP_ASSUME_ALIGNED(x, sizeof(Fragment*))

Fragment * Fragment::GetNext() const{
    Fragment* const out = ASSUME_ALIGNED_FRAG_PTR(this)->header.next;
    return ASSUME_ALIGNED_FRAG_PTR(out);
}

Fragment * Fragment::GetPrev() const
{
    Fragment* const out = (Fragment*) (ASSUME_ALIGNED_FRAG_PTR(this)->header.prev_used & ~(uintptr_t) 1U);
    return ASSUME_ALIGNED_FRAG_PTR(out);
}

bool Fragment::IsUsed() const 
{
    return (ASSUME_ALIGNED_FRAG_PTR(this)->header.prev_used & (uintptr_t) 1U) != 0U;
}

size_t O1HeapInstance::fragGetSize(const Fragment* const frag) const
{
    ASSUME_ALIGNED_FRAG_PTR(frag);
    O1HEAP_ASSERT(((size_t) frag) >= (((size_t) this) + INSTANCE_SIZE_PADDED));
    ASSUME_ALIGNED_FRAG_PTR((frag)->GetPrev());

    const size_t sz = (frag->header.next != NULL) ? (size_t) (((const char*) frag->header.next) - ((const char*) frag))
                                                : (size_t) (this->arena_end - ((const char*) frag));

    O1HEAP_ASSERT(sz >= FRAGMENT_SIZE_MIN);
    O1HEAP_ASSERT(sz <= this->diagnostics.capacity);
    O1HEAP_ASSERT((sz % FRAGMENT_SIZE_MIN) == 0U);
    return sz;
}

static void fragSetNext(Fragment* const frag, Fragment* const value)
{
    // O1HEAP_ASSERT((((size_t) frag) % O1HEAP_ALIGNMENT) == 0U);
    // O1HEAP_ASSERT((((size_t) value) % O1HEAP_ALIGNMENT) == 0U);
    frag->header.next = value;
}

static void fragSetPrev(Fragment* const frag, Fragment* const value)
{
    // O1HEAP_ASSERT((((size_t) frag) % O1HEAP_ALIGNMENT) == 0U);
    // O1HEAP_ASSERT((((size_t) value) % O1HEAP_ALIGNMENT) == 0U);
    frag->header.prev_used = (frag->header.prev_used & (uintptr_t) 1U) | (uintptr_t) value;
}

static void fragSetUsed(Fragment* const frag, const bool value)
{
    // O1HEAP_ASSERT((((size_t) frag) % O1HEAP_ALIGNMENT) == 0U);
    if (value)
    {
        frag->header.prev_used |= (uintptr_t) 1U;
    }
    else
    {
        frag->header.prev_used &= ~(uintptr_t) 1U;
    }
}

// ---------------------------------------- FRAGMENT MANAGEMENT ----------------------------------------

/// Links two fragments so that their next/prev pointers point to each other; left goes before right.
static void interlink(Fragment* const left, Fragment* const right)
{
    if (O1HEAP_LIKELY(left != NULL))
    {
        fragSetNext(left, right);
    }
    if (O1HEAP_LIKELY(right != NULL))
    {
        fragSetPrev(right, left);
    }
}

/// Adds a new fragment into the appropriate bin and updates the lookup mask.
void lib_o1heap::O1HeapInstance::rebin(Fragment* const fragment, const size_t fragment_size)
{
    O1HEAP_ASSERT(fragment != NULL);
    O1HEAP_ASSERT((fragment_size % FRAGMENT_SIZE_MIN) == 0U);
    const uint_fast8_t idx = log2Floor(fragment_size / FRAGMENT_SIZE_MIN);  // Round DOWN when inserting.
    O1HEAP_ASSERT(idx < NUM_BINS_MAX, idx, NUM_BINS_MAX);
    // Add the new fragment to the beginning of the bin list.
    // I.e., each allocation will be returning the most-recently-used fragment -- good for caching.
    fragment->next_free = this->bins[idx];
    fragment->prev_free = NULL;
    if (O1HEAP_LIKELY(this->bins[idx] != NULL))
    {
        this->bins[idx]->prev_free = fragment;
    }
    this->bins[idx] = fragment;
    this->nonempty_bin_mask |= pow2(idx);
}

/// Removes the specified fragment from its bin.
void lib_o1heap::O1HeapInstance::unbin(const Fragment* const fragment, const size_t fragment_size)
{
    O1HEAP_ASSERT(fragment != NULL);
    O1HEAP_ASSERT((fragment_size % FRAGMENT_SIZE_MIN) == 0U);
    const uint_fast8_t idx = log2Floor(fragment_size / FRAGMENT_SIZE_MIN);  // Round DOWN when removing.
    O1HEAP_ASSERT(idx < NUM_BINS_MAX);
    // Remove the bin from the free fragment list.
    if (O1HEAP_LIKELY(fragment->next_free != NULL))
    {
        fragment->next_free->prev_free = fragment->prev_free;
    }
    if (O1HEAP_LIKELY(fragment->prev_free != NULL))
    {
        fragment->prev_free->next_free = fragment->next_free;
    }
    // Update the bin header.
    if (O1HEAP_LIKELY(this->bins[idx] == fragment))
    {
        O1HEAP_ASSERT(fragment->prev_free == NULL);
        this->bins[idx] = fragment->next_free;
        if (O1HEAP_LIKELY(this->bins[idx] == NULL))
        {
            this->nonempty_bin_mask &= ~pow2(idx);
        }
    }
}

// ---------------------------------------- PUBLIC API IMPLEMENTATION ----------------------------------------
O1HeapInstance* Ctor(void* const base, const size_t size)
{
    O1HeapInstance* out = NULL;
    if ((base != NULL) && ((((size_t) base) % O1HEAP_ALIGNMENT) == 0U) && (size >= O1HEAP_MIN_ARENA_SIZE))
    {
        // Allocate the core heap metadata structure in the beginning of the arena.
        O1HEAP_ASSERT(((size_t) base) % sizeof(O1HeapInstance*) == 0U);
        out                    = (O1HeapInstance*) base;
        out->nonempty_bin_mask = 0U;
        for (size_t i = 0; i < NUM_BINS_MAX; i++)
        {
            out->bins[i] = NULL;
        }

        // Limit and align the capacity.
        size_t capacity = size - INSTANCE_SIZE_PADDED;
        if (capacity > FRAGMENT_SIZE_MAX)
        {
            capacity = FRAGMENT_SIZE_MAX;
        }
        while ((capacity % FRAGMENT_SIZE_MIN) != 0)
        {
            O1HEAP_ASSERT(capacity > 0U);
            capacity--;
        }
        O1HEAP_ASSERT((capacity % FRAGMENT_SIZE_MIN) == 0);
        O1HEAP_ASSERT((capacity >= FRAGMENT_SIZE_MIN) && (capacity <= FRAGMENT_SIZE_MAX));

        // Initialize the diagnostics.
        out->diagnostics.capacity          = capacity;
        out->diagnostics.allocated         = 0U;
        out->diagnostics.peak_allocated    = 0U;
        out->diagnostics.peak_request_size = 0U;
        out->diagnostics.oom_count         = 0U;

        // Store the arena end pointer for computing size of the last fragment.
        char* const arena_start = ((char*) base) + INSTANCE_SIZE_PADDED;
        out->arena_end          = arena_start + capacity;

        // Initialize the root fragment.
        Fragment* const frag = (Fragment*) (void*) arena_start;
        O1HEAP_ASSERT((((size_t) frag) % O1HEAP_ALIGNMENT) == 0U);
        fragSetNext(frag, NULL);
        fragSetPrev(frag, NULL);
        fragSetUsed(frag, false);
        frag->next_free = NULL;
        frag->prev_free = NULL;
        O1HEAP_ASSERT(out->fragGetSize(frag) == capacity);
        out->rebin(frag, capacity);
        O1HEAP_ASSERT(out->nonempty_bin_mask != 0U);
    }

    return out;
}

void* lib_o1heap::O1HeapInstance::Allocate(const size_t amount)
{
    O1HEAP_ASSERT(this->diagnostics.capacity <= FRAGMENT_SIZE_MAX);
    void* out = NULL;

    // If the amount approaches approx. SIZE_MAX/2, an undetected integer overflow may occur.
    // To avoid that, we do not attempt allocation if the amount exceeds the hard limit.
    // We perform multiple redundant checks to account for a possible unaccounted overflow.
    if (O1HEAP_LIKELY((amount > 0U) && (amount <= (this->diagnostics.capacity - O1HEAP_ALIGNMENT))))
    {
        // Add the header size and align the allocation size to the power of 2.
        // See "Timing-Predictable Memory Allocation In Hard Real-Time Systems", Herter, page 27.
        const size_t alloc_size = roundUpToPowerOf2(amount + O1HEAP_ALIGNMENT);
        O1HEAP_ASSERT(alloc_size <= FRAGMENT_SIZE_MAX);
        O1HEAP_ASSERT(alloc_size >= FRAGMENT_SIZE_MIN);
        O1HEAP_ASSERT(alloc_size >= amount + O1HEAP_ALIGNMENT);
        O1HEAP_ASSERT((alloc_size & (alloc_size - 1U)) == 0U);  // Is power of 2.

        // Since alloc_size and FRAGMENT_SIZE_MIN are both powers of 2, the quotient is also a power of 2,
        // meaning log2Ceil == log2Floor. We use log2Floor because it is slightly faster (no x-1 before CLZ).
        const size_t alloc_bins = alloc_size / FRAGMENT_SIZE_MIN;
        O1HEAP_ASSERT((alloc_bins & (alloc_bins - 1U)) == 0U);         // Is power of 2 => log2Ceil == log2Floor.
        const uint_fast8_t optimal_bin_index = log2Floor(alloc_bins);  // Should be ceil, but in this case identical.
        O1HEAP_ASSERT(optimal_bin_index < NUM_BINS_MAX);
        const size_t candidate_bin_mask = ~(pow2(optimal_bin_index) - 1U);

        // Find the smallest non-empty bin we can use.
        const size_t suitable_bins     = this->nonempty_bin_mask & candidate_bin_mask;
        const size_t smallest_bin_mask = suitable_bins & ~(suitable_bins - 1U);  // Clear all bits but the lowest.
        if (O1HEAP_LIKELY(smallest_bin_mask != 0))
        {
            O1HEAP_ASSERT((smallest_bin_mask & (smallest_bin_mask - 1U)) == 0U);  // Is power of 2.
            const uint_fast8_t bin_index = log2Floor(smallest_bin_mask);
            O1HEAP_ASSERT(bin_index >= optimal_bin_index);
            O1HEAP_ASSERT(bin_index < NUM_BINS_MAX);

            // The bin we found shall not be empty, otherwise it's a state divergence (memory corruption?).
            Fragment* const frag = this->bins[bin_index];
            O1HEAP_ASSERT(frag != NULL);
            const size_t frag_size = this->fragGetSize(frag);
            O1HEAP_ASSERT(frag_size >= alloc_size);
            O1HEAP_ASSERT((frag_size % FRAGMENT_SIZE_MIN) == 0U);
            O1HEAP_ASSERT(!(frag->IsUsed()));
            unbin(frag, frag_size);

            // Split the fragment if it is too large.
            const size_t leftover = frag_size - alloc_size;
            O1HEAP_ASSERT(leftover < this->diagnostics.capacity);  // Overflow check.
            O1HEAP_ASSERT(leftover % FRAGMENT_SIZE_MIN == 0U);       // Alignment check.
            if (O1HEAP_LIKELY(leftover >= FRAGMENT_SIZE_MIN))
            {
                Fragment* const new_frag = (Fragment*) (void*) (((char*) frag) + alloc_size);
                fragSetUsed(new_frag, false);
                interlink(new_frag, (frag->GetNext()));
                interlink(frag, new_frag);
                O1HEAP_ASSERT(leftover == this->fragGetSize(new_frag));
                rebin(new_frag, leftover);
            }

            // Update the diagnostics.
            O1HEAP_ASSERT((this->diagnostics.allocated % FRAGMENT_SIZE_MIN) == 0U);
            this->diagnostics.allocated += alloc_size;
            O1HEAP_ASSERT(this->diagnostics.allocated <= this->diagnostics.capacity);
            this->diagnostics.peak_allocated =
                larger(this->diagnostics.peak_allocated, this->diagnostics.allocated);

            // Finalize the fragment we just allocated.
            O1HEAP_ASSERT(this->fragGetSize(frag) >= amount + O1HEAP_ALIGNMENT);
            fragSetUsed(frag, true);

            out = ((char*) frag) + O1HEAP_ALIGNMENT;
        }
    }

    // Update the diagnostics.
    this->diagnostics.peak_request_size = larger(this->diagnostics.peak_request_size, amount);
    if (O1HEAP_LIKELY((out == NULL) && (amount > 0U)))
    {
        this->diagnostics.oom_count++;
    }

    return out;
}

void lib_o1heap::O1HeapInstance::Free(void* const pointer)
{
    O1HEAP_ASSERT(this->diagnostics.capacity <= FRAGMENT_SIZE_MAX);
    if (O1HEAP_LIKELY(pointer != NULL))  // NULL pointer is a no-op.
    {
        Fragment* const frag = (Fragment*) (void*) (((char*) pointer) - O1HEAP_ALIGNMENT);

        // Check for heap corruption in debug builds.
        O1HEAP_ASSERT(((size_t) frag) % sizeof(Fragment*) == 0U);
        O1HEAP_ASSERT(((size_t) frag) >= (((size_t) this) + INSTANCE_SIZE_PADDED));
        O1HEAP_ASSERT(((size_t) frag) <=
                      (((size_t) this) + INSTANCE_SIZE_PADDED + this->diagnostics.capacity - FRAGMENT_SIZE_MIN));
        O1HEAP_ASSERT((frag->IsUsed()));  // Catch double-free
        O1HEAP_ASSERT(((size_t) (frag->GetNext())) % sizeof(Fragment*) == 0U);
        O1HEAP_ASSERT(((size_t) (frag->GetNext())) % sizeof(Fragment*) == 0U);
        const size_t frag_size = this->fragGetSize(frag);
        O1HEAP_ASSERT(frag_size >= FRAGMENT_SIZE_MIN);
        O1HEAP_ASSERT(frag_size <= this->diagnostics.capacity);
        O1HEAP_ASSERT((frag_size % FRAGMENT_SIZE_MIN) == 0U);

        // Even if we're going to drop the fragment later, mark it free anyway to prevent double-free.
        fragSetUsed(frag, false);

        // Update the diagnostics. It must be done before merging because it invalidates the fragment size information.
        O1HEAP_ASSERT(this->diagnostics.allocated >= frag_size);  // Heap corruption check.
        this->diagnostics.allocated -= frag_size;

        // Merge with siblings and insert the returned fragment into the appropriate bin and update metadata.
        Fragment* const prev       = (frag)->GetPrev();
        Fragment* const next       = (frag)->GetNext();
        const bool      join_left  = (prev != NULL) && (!(prev->IsUsed()));
        const bool      join_right = (next != NULL) && (!(next->IsUsed()));
        if (join_left && join_right)  // [ prev ][ this ][ next ] => [ ------- prev ------- ]
        {
            const size_t prev_size = this->fragGetSize(prev);
            const size_t next_size = this->fragGetSize(next);
            unbin(prev, prev_size);
            unbin(next, next_size);
            interlink(prev, (next->GetNext()));
            rebin(prev, prev_size + frag_size + next_size);
        }
        else if (join_left)  // [ prev ][ this ][ next ] => [ --- prev --- ][ next ]
        {
            const size_t prev_size = this->fragGetSize(prev);
            unbin(prev, prev_size);
            interlink(prev, next);
            rebin(prev, prev_size + frag_size);
        }
        else if (join_right)  // [ prev ][ this ][ next ] => [ prev ][ --- this --- ]
        {
            const size_t next_size = this->fragGetSize(next);
            unbin(next, next_size);
            interlink(frag, (next->GetNext()));
            rebin(frag, frag_size + next_size);
        }
        else
        {
            rebin(frag, frag_size);
        }
    }
}

void* lib_o1heap::O1HeapInstance::Reallocate(void* const pointer, const size_t new_amount)
{
    O1HEAP_ASSERT(this->diagnostics.capacity <= FRAGMENT_SIZE_MAX);

    // SPECIAL CASE: Allocation delegation.
    if (O1HEAP_UNLIKELY(pointer == NULL))
    {
        return Allocate(new_amount);  // MISRA: Early return simplifies control flow.
    }

    // SPECIAL CASE: Free delegation. This is a common implementation-defined extension in the standard realloc().
    if (O1HEAP_UNLIKELY(new_amount == 0U))
    {
        Free(pointer);
        return NULL;  // MISRA: Early return simplifies control flow.
    }

    // SPECIAL CASE: Prevent size overflow like in o1heapAllocate().
    this->diagnostics.peak_request_size = larger(this->diagnostics.peak_request_size, new_amount);
    if (O1HEAP_UNLIKELY(new_amount > (this->diagnostics.capacity - O1HEAP_ALIGNMENT)))
    {
        this->diagnostics.oom_count++;
        return NULL;  // MISRA: Early return simplifies control flow.
    }

    // NORMAL REALLOC BEHAVIORS. The edge cases have been thisd above.
    Fragment* const frag          = (Fragment*) (void*) (((char*) pointer) - O1HEAP_ALIGNMENT);
    const size_t    frag_size     = this->fragGetSize(frag);
    const size_t    old_amount    = frag_size - O1HEAP_ALIGNMENT;
    const size_t    new_frag_size = roundUpToPowerOf2(new_amount + O1HEAP_ALIGNMENT);
    O1HEAP_ASSERT((new_frag_size <= FRAGMENT_SIZE_MAX) && (new_frag_size >= FRAGMENT_SIZE_MIN));
    O1HEAP_ASSERT(new_frag_size <= this->diagnostics.capacity);
    O1HEAP_ASSERT((frag->IsUsed()));  // Catch use-after-free.

    Fragment*    prev      = (frag->GetPrev());
    Fragment*    next      = (frag->GetNext());
    const bool   prev_free = (prev != NULL) && (!(prev->IsUsed()));
    const bool   next_free = (next != NULL) && (!(next->IsUsed()));
    const size_t prev_size = prev_free ? this->fragGetSize(prev) : 0U;
    const size_t next_size = next_free ? this->fragGetSize(next) : 0U;

    // SHRINK OR SAME SIZE: new_frag_size <= frag_size. Data stays in place.
    if (O1HEAP_UNLIKELY(new_frag_size <= frag_size))
    {
        const size_t leftover = frag_size - new_frag_size;
        O1HEAP_ASSERT((leftover % FRAGMENT_SIZE_MIN) == 0U);
        if (O1HEAP_LIKELY(leftover >= FRAGMENT_SIZE_MIN))
        {
            O1HEAP_ASSERT(this->diagnostics.allocated >= leftover);
            this->diagnostics.allocated -= leftover;
            Fragment* const new_frag = (Fragment*) (void*) (((char*) frag) + new_frag_size);
            fragSetUsed(new_frag, false);
            interlink(frag, new_frag);
            if (O1HEAP_LIKELY(next_free))  // [ frag ][ new ][ next ] => [ frag ][ --- new --- ]
            {
                unbin(next, next_size);
                interlink(new_frag, (next->GetNext()));
                O1HEAP_ASSERT(this->fragGetSize(new_frag) == (leftover + next_size));
                rebin(new_frag, leftover + next_size);
            }
            else  // [ frag ][ new ][ next ]
            {
                interlink(new_frag, next);
                O1HEAP_ASSERT(this->fragGetSize(new_frag) == leftover);
                rebin(new_frag, leftover);
            }
            O1HEAP_ASSERT(this->fragGetSize(frag) == new_frag_size);
            O1HEAP_ASSERT(this->fragGetSize(new_frag) == (next_free ? (leftover + next_size) : leftover));
        }
        return pointer;  // MISRA: Early return simplifies control flow.
    }

    // EXPAND FORWARD: next is free and current+next >= new_frag_size. Data stays in place.
    if (next_free && ((frag_size + next_size) >= new_frag_size))
    {
        unbin(next, next_size);
        const size_t leftover = (frag_size + next_size) - new_frag_size;
        O1HEAP_ASSERT((leftover % FRAGMENT_SIZE_MIN) == 0U);
        if (O1HEAP_LIKELY(leftover >= FRAGMENT_SIZE_MIN))  // [ frag ][ --- next --- ] => [ --- frag --- ][ next ]
        {
            Fragment* const new_frag = (Fragment*) (void*) (((char*) frag) + new_frag_size);
            fragSetUsed(new_frag, false);
            interlink(new_frag, (next->GetNext()));
            interlink(frag, new_frag);
            rebin(new_frag, leftover);
            this->diagnostics.allocated += new_frag_size - frag_size;
        }
        else  // [ frag ][ --- next --- ] => [ --- frag --- ]
        {
            interlink(frag, (next->GetNext()));
            this->diagnostics.allocated += next_size;
        }
        this->diagnostics.peak_allocated = larger(this->diagnostics.peak_allocated, this->diagnostics.allocated);
        return pointer;  // MISRA: Early return simplifies control flow.
    }

    // EXPAND BACKWARD AND FORWARD: prev is free; next maybe free; prev+current+next >= new_frag_size.
    // Data must be moved, this is unavoidable because there is not enough space ahead.
    // Note that since the move size is not greater than the current fragment, memmove will not invalidate the next
    // fragment, but it may invalidate the current fragment.
    if (prev_free && ((prev_size + frag_size + next_size) >= new_frag_size))
    {
        unbin(prev, prev_size);
        if (next_free)
        {
            unbin(next, next_size);
        }
        void* const out = ((char*) prev) + O1HEAP_ALIGNMENT;  // Move all the way to the back before fragments updated.
        (void) memmove(out, pointer, old_amount);  // ATTENTION: Invalidates the old frag due to potential overwrite.
        fragSetUsed(prev, true);
        const size_t leftover = (prev_size + frag_size + next_size) - new_frag_size;
        O1HEAP_ASSERT((leftover % FRAGMENT_SIZE_MIN) == 0U);
        if (O1HEAP_LIKELY(leftover >= FRAGMENT_SIZE_MIN))
        {
            Fragment* const new_frag = (Fragment*) (void*) (((char*) prev) + new_frag_size);
            fragSetUsed(new_frag, false);
            interlink(new_frag, next_free ? (next->GetNext()) : next);
            interlink(prev, new_frag);  // NOLINT(readability-suspicious-call-argument)
            rebin(new_frag, leftover);
            this->diagnostics.allocated += new_frag_size - frag_size;
        }
        else
        {
            interlink(prev, next_free ? (next->GetNext()) : next);
            this->diagnostics.allocated += prev_size + next_size;
        }
        this->diagnostics.peak_allocated = larger(this->diagnostics.peak_allocated, this->diagnostics.allocated);
        return out;  // MISRA: Early return simplifies control flow.
    }

    // ALLOCATE NEW BLOCK: copy data, free old block. In-place or near-place expansion not possible.
    // This is the final resort. The normal allocate also thiss the OOM count update.
    void* const out = Allocate(new_amount);
    if (out != NULL)
    {
        (void) memcpy(out, pointer, old_amount);
        Free(pointer);
    }
    return out;
}

size_t O1HeapInstance::GetMaxAllocationSize() const
{
    // The largest allocation is smaller (up to almost two times) than the arena capacity,
    // due to the power-of-two padding and the fragment header overhead.
    return pow2(log2Floor(this->diagnostics.capacity)) - O1HEAP_ALIGNMENT;
}

bool O1HeapInstance::DoInvariantsHold() const
{
    bool valid = true;

    // Check the bin mask consistency.
    #pragma GCC unroll 8
    for (size_t i = 0; i < NUM_BINS_MAX; i++)  // Dear compiler, feel free to unroll this loop.
    {
        const bool mask_bit_set = (this->nonempty_bin_mask & pow2((uint_fast8_t) i)) != 0U;
        const bool bin_nonempty = this->bins[i] != NULL;
        valid                   = valid && (mask_bit_set == bin_nonempty);
    }

    // Create a local copy of the diagnostics struct.
    const O1HeapDiagnostics diag = this->diagnostics;

    // Capacity check.
    valid = valid && (diag.capacity <= FRAGMENT_SIZE_MAX) && (diag.capacity >= FRAGMENT_SIZE_MIN) &&
            ((diag.capacity % FRAGMENT_SIZE_MIN) == 0U);

    // Allocation info check.
    valid = valid && (diag.allocated <= diag.capacity) && ((diag.allocated % FRAGMENT_SIZE_MIN) == 0U) &&
            (diag.peak_allocated <= diag.capacity) && (diag.peak_allocated >= diag.allocated) &&
            ((diag.peak_allocated % FRAGMENT_SIZE_MIN) == 0U);

    // Peak request check
    valid = valid && ((diag.peak_request_size < diag.capacity) || (diag.oom_count > 0U));
    if (diag.peak_request_size == 0U)
    {
        valid = valid && (diag.peak_allocated == 0U) && (diag.allocated == 0U) && (diag.oom_count == 0U);
    }
    else
    {
        valid = valid &&  // Overflow on summation is possible but safe to ignore.
                (((diag.peak_request_size + O1HEAP_ALIGNMENT) <= diag.peak_allocated) || (diag.oom_count > 0U));
    }

    return valid;
}


}