#include <not_implemented.h>
#include <cstddef>
#include "../include/allocator_buddies_system.h"

allocator_buddies_system::~allocator_buddies_system()
{
}

allocator_buddies_system::allocator_buddies_system(
    allocator_buddies_system &&other) noexcept
{
    throw not_implemented("allocator_buddies_system::allocator_buddies_system(allocator_buddies_system &&) noexcept", "your code should be here...");
}

allocator_buddies_system &allocator_buddies_system::operator=(
    allocator_buddies_system &&other) noexcept
{
    throw not_implemented("allocator_buddies_system &allocator_buddies_system::operator=(allocator_buddies_system &&) noexcept", "your code should be here...");
}

allocator_buddies_system::allocator_buddies_system(
        size_t space_size,
        std::pmr::memory_resource *parent_allocator,
        allocator_with_fit_mode::fit_mode allocate_fit_mode)
{
    throw not_implemented("allocator_buddies_system::allocator_buddies_system(size_t,std::pmr::memory_resource *,allocator_with_fit_mode::fit_mode)", "your code should be here...");
}

[[nodiscard]] void *allocator_buddies_system::do_allocate_sm(
    size_t size)
{
    throw not_implemented("[[nodiscard]] void *allocator_buddies_system::do_allocate_sm(size_t)", "your code should be here...");
}

void allocator_buddies_system::do_deallocate_sm(void *at)
{
    throw not_implemented("void allocator_buddies_system::do_deallocate_sm(void *)", "your code should be here...");
}

allocator_buddies_system::allocator_buddies_system(const allocator_buddies_system &other)
{
    throw not_implemented("allocator_buddies_system::allocator_buddies_system(const allocator_buddies_system &other)", "your code should be here...");
}

allocator_buddies_system &allocator_buddies_system::operator=(const allocator_buddies_system &other)
{
    throw not_implemented("allocator_buddies_system &allocator_buddies_system::operator=(const allocator_buddies_system &other)", "your code should be here...");
}

bool allocator_buddies_system::do_is_equal(const std::pmr::memory_resource &other) const noexcept
{
    throw not_implemented("bool allocator_buddies_system::do_is_equal(const std::pmr::memory_resource &other) const noexcept", "your code should be here...");
}

inline void allocator_buddies_system::set_fit_mode(
    allocator_with_fit_mode::fit_mode mode)
{
}


std::vector<allocator_test_utils::block_info> allocator_buddies_system::get_blocks_info() const noexcept
{
    // + lock mutex
    get_blocks_info_inner();
}

std::vector<allocator_test_utils::block_info> allocator_buddies_system::get_blocks_info_inner() const
{
    throw not_implemented("std::vector<allocator_test_utils::block_info> allocator_buddies_system::get_blocks_info_inner() const", "your code should be here...");
}

allocator_buddies_system::buddy_iterator allocator_buddies_system::begin() const noexcept
{
    return buddy_iterator(this->_trusted_memory);
}

allocator_buddies_system::buddy_iterator allocator_buddies_system::end() const noexcept
{
    return buddy_iterator(nullptr);
}

bool allocator_buddies_system::buddy_iterator::operator==(const allocator_buddies_system::buddy_iterator &other) const noexcept
{
    if (this->_block == nullptr && other._block == nullptr) {
        return true;
    }
    return this->_block == other._block;
}

bool allocator_buddies_system::buddy_iterator::operator!=(const allocator_buddies_system::buddy_iterator &other) const noexcept
{
    return !(*this == other);
}

allocator_buddies_system::buddy_iterator &allocator_buddies_system::buddy_iterator::operator++() & noexcept
{
    if (this->_block != nullptr) {
        auto * metadata = reinterpret_cast<block_metadata*>(this->_block);
        size_t block_size = 1ULL << metadata->size;
        this->_block = reinterpret_cast<void*>(reinterpret_cast<std::byte*>(this->_block) + block_size);
    }
    return *this;
}

allocator_buddies_system::buddy_iterator allocator_buddies_system::buddy_iterator::operator++(int n)
{
    buddy_iterator tmp(*this);
    ++(*this);
    return tmp;
}

size_t allocator_buddies_system::buddy_iterator::size() const noexcept
{
    auto * metadata = reinterpret_cast<block_metadata*>(this->_block);
    return metadata->size;
}

bool allocator_buddies_system::buddy_iterator::occupied() const noexcept
{
    auto * metadata = reinterpret_cast<block_metadata*>(this->_block);
    return metadata->occupied;
}

void *allocator_buddies_system::buddy_iterator::operator*() const noexcept
{
    return this->_block;
}

allocator_buddies_system::buddy_iterator::buddy_iterator(void *start) : _block(start) {}

allocator_buddies_system::buddy_iterator::buddy_iterator() : _block(nullptr) {}
