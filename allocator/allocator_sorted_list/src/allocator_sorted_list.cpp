#include <not_implemented.h>
#include "../include/allocator_sorted_list.h"
#include <iostream>

allocator_sorted_list::~allocator_sorted_list()
{
    get_mutex().~mutex();
    size_t total = *reinterpret_cast<size_t*>(static_cast<std::byte*>(_trusted_memory) + sizeof(std::pmr::memory_resource*) + sizeof(fit_mode));
    (*get_parent()).deallocate(_trusted_memory, total);
    _trusted_memory = nullptr;
}

allocator_sorted_list::allocator_sorted_list(
    allocator_sorted_list &&other) noexcept
{
    this->_trusted_memory = other._trusted_memory;
    other._trusted_memory = nullptr;
}

allocator_sorted_list &allocator_sorted_list::operator=(
    allocator_sorted_list &&other) noexcept
{
    if (this != &other) {
        if (_trusted_memory) {
            get_mutex().~mutex();
            size_t total = *reinterpret_cast<size_t*>(static_cast<std::byte*>(_trusted_memory) + sizeof(std::pmr::memory_resource*) + sizeof(fit_mode));
            (*get_parent()).deallocate(_trusted_memory, total);
            this->_trusted_memory = other._trusted_memory;
            other._trusted_memory = nullptr;
        }
    }
    return *this;
}

size_t get_block_size(void * block) {
    auto * ptr = reinterpret_cast<std::byte*>(block);
    return *reinterpret_cast<size_t*>(ptr + sizeof(void*));
}

allocator_sorted_list::allocator_sorted_list(
        size_t space_size,
        std::pmr::memory_resource *parent_allocator,
        allocator_with_fit_mode::fit_mode allocate_fit_mode)
{
    if (parent_allocator == nullptr) {
        parent_allocator = std::pmr::get_default_resource();
    }
    _trusted_memory = parent_allocator->allocate(space_size + allocator_metadata_size + block_metadata_size);
    auto * ptr = reinterpret_cast<std::byte*>(_trusted_memory);

    *reinterpret_cast<std::pmr::memory_resource**>(ptr) = parent_allocator;
    ptr += sizeof(parent_allocator);
    *reinterpret_cast<allocator_with_fit_mode::fit_mode*>(ptr) = allocate_fit_mode;
    ptr += sizeof(fit_mode);
    *reinterpret_cast<size_t*>(ptr) = space_size + allocator_metadata_size;
    ptr += sizeof(size_t);

    new (ptr) std::mutex; // placement new
    ptr += sizeof(std::mutex);

    std::byte* first_block = ptr + sizeof(void*);   // ptr уже std::byte*
    *reinterpret_cast<void**>(ptr) = first_block;   // free_list_head = first_block
    ptr += sizeof(void*);

    *reinterpret_cast<void**>(first_block) = nullptr;          // next = nullptr
    *reinterpret_cast<size_t*>(first_block + sizeof(void*)) = space_size; // size
}

[[nodiscard]] void *allocator_sorted_list::do_allocate_sm(
    size_t size)
{
    if (size == 0) {
        return nullptr;
    }
    std::lock_guard<std::mutex> lock(get_mutex()); 
    std::byte * ptr_cur = reinterpret_cast<std::byte*>(get_first_free_block()); // проверяемый блок
    std::byte * ptr_prev = nullptr; // предыдущий блок
    size_t worst_size = 0;
    size_t best_size = std::numeric_limits<size_t>::max();

    std::byte * first_fit_place = nullptr; std::byte * prev_first_fit = nullptr;
    std::byte * best_fit_place = nullptr; std::byte * prev_best_fit = nullptr;
    std::byte * worst_fit_place = nullptr; std::byte * prev_worst_fit = nullptr;
    std::byte * allocated_block = nullptr;

    while (ptr_cur != nullptr) {
        size_t size_block = get_block_size(ptr_cur);
        std::byte * next_free_block = read_next_block(ptr_cur); // next FREE block!

        if (get_fit_mode() == allocator_with_fit_mode::fit_mode::first_fit) {
            if (size_block - block_metadata_size >= size) {
                first_fit_place = ptr_cur;
                prev_first_fit = ptr_prev;
                break;
            }
        } 
        if (get_fit_mode() == allocator_with_fit_mode::fit_mode::the_best_fit) {
            if (size_block < best_size && size_block - block_metadata_size >= size) {
                best_size = size_block;
                best_fit_place = ptr_cur;
                prev_best_fit = ptr_prev;
            }
        }
        if (get_fit_mode() == allocator_with_fit_mode::fit_mode::the_worst_fit) {
            if (size_block > worst_size && size_block - block_metadata_size >= size) {
                worst_size = size_block;
                worst_fit_place = ptr_cur;
                prev_worst_fit = ptr_prev;
            }
        }

        ptr_prev = ptr_cur;
        ptr_cur = read_next_block(ptr_cur);
        // идем на след блок
    }
    if (get_fit_mode() == allocator_with_fit_mode::fit_mode::first_fit) {
        if (first_fit_place == nullptr) {
            throw std::bad_alloc();
        }
        allocated_block = allocate_and_merge(prev_first_fit, first_fit_place, size);
    }
    if (get_fit_mode() == allocator_with_fit_mode::fit_mode::the_worst_fit) {
        if (worst_fit_place == nullptr) {
            throw std::bad_alloc();
        }
        allocated_block = allocate_and_merge(prev_worst_fit, worst_fit_place, size);
    }
    if (get_fit_mode() == allocator_with_fit_mode::fit_mode::the_best_fit) {
        if (best_fit_place == nullptr) {
            throw std::bad_alloc();
        }
        allocated_block = allocate_and_merge(prev_best_fit, best_fit_place, size);
    }

    return reinterpret_cast<void*>(allocated_block + block_metadata_size);
}


std::byte * allocator_sorted_list::allocate_and_merge(std::byte * ptr_prev, std::byte * ptr_cur, size_t size) { 
    
    std::byte * allocated_block = ptr_cur;
    size_t size_block = get_block_size(ptr_cur);
    std::byte * next_free_block = read_next_block(ptr_cur);

    if (size_block - block_metadata_size - size > block_metadata_size + 1) { // можно отрезать лишний кусок и вставить в список свободных
        std::byte * new_block = ptr_cur + block_metadata_size + size; // на этом месте надо заполнить мета (это новый свободный)
        *reinterpret_cast<void**>(new_block) = next_free_block;
        *reinterpret_cast<size_t*>(new_block + sizeof(void*)) = size_block - size - block_metadata_size;
        *reinterpret_cast<size_t*>(ptr_cur + sizeof(void*)) = size + block_metadata_size;

        if (ptr_prev == nullptr) { // мы в первом блоке
            set_new_first_free_block(new_block);
        } else {
            *reinterpret_cast<void**>(ptr_prev) = new_block;
        }

    } else {
        if (ptr_prev == nullptr) {
            set_new_first_free_block(next_free_block);
        } else {
            *reinterpret_cast<void**>(ptr_prev) = next_free_block;
        }
    }
    return allocated_block;
}

allocator_sorted_list::allocator_sorted_list(const allocator_sorted_list &other) {}

allocator_sorted_list &allocator_sorted_list::operator=(const allocator_sorted_list &other) {}

bool allocator_sorted_list::do_is_equal(const std::pmr::memory_resource &other) const noexcept
{
    return dynamic_cast<const allocator_sorted_list*>(&other) != nullptr;
}

void allocator_sorted_list::do_deallocate_sm(
    void *at)
{
    if (at == nullptr) {
        throw std::bad_alloc();
    }
    std::byte * mem_start = reinterpret_cast<std::byte*>(_trusted_memory) + allocator_metadata_size;
    std::byte * mem_end = reinterpret_cast<std::byte*>(_trusted_memory) + *get_size_total();
    std::lock_guard<std::mutex> lock(get_mutex());
    std::byte * block = reinterpret_cast<std::byte*>(static_cast<std::byte*>(at) - block_metadata_size);

    if (block < mem_start || block >= mem_end) {
        throw std::bad_alloc();
    }
    insert_free_block(block);
    size_t size = get_block_size(block);
    std::byte * next_block = block + size;
    std::byte * next_free_block = *reinterpret_cast<std::byte**>(block);
    if (next_block == next_free_block) {
        size_t size_next = get_block_size(next_block);
        *reinterpret_cast<void**>(block) = *reinterpret_cast<void**>(next_block);
        *reinterpret_cast<size_t*>(block + sizeof(void*)) = size + size_next;
    }
    std::byte * curr = reinterpret_cast<std::byte*>(get_first_free_block());
    std::byte * prev = nullptr;
    while (curr != nullptr && curr < block) {
        prev = curr;
        curr = static_cast<std::byte*>(*reinterpret_cast<void**>(curr));    
    } 
    if (prev != nullptr) {
        size_t size_prev = get_block_size(prev);
        if (prev + size_prev + block_metadata_size == block) {
            *reinterpret_cast<void**>(prev) = *reinterpret_cast<void**>(block); 
            *reinterpret_cast<size_t*>(prev + sizeof(void*)) = size_prev + *reinterpret_cast<size_t*>(block + sizeof(void*));
        }
    }
}


inline void allocator_sorted_list::set_fit_mode(
    allocator_with_fit_mode::fit_mode mode)
{
    std::lock_guard<std::mutex> lock(get_mutex());
    fit_mode& fm = get_fit_mode();
    fm = mode;
}

std::vector<allocator_test_utils::block_info> allocator_sorted_list::get_blocks_info() const noexcept
{
    std::lock_guard<std::mutex> lock(get_mutex()); 
    return get_blocks_info_inner();
}

std::vector<allocator_test_utils::block_info> allocator_sorted_list::get_blocks_info_inner() const
{
    std::vector<allocator_test_utils::block_info> info_vector;
    for (auto it = begin(); it != end(); ++it) {
        info_vector.push_back({it.size(), it.occupied()});
    }
    return info_vector;
}

allocator_sorted_list::sorted_free_iterator allocator_sorted_list::free_begin() const noexcept
{
    return sorted_free_iterator(_trusted_memory);
}

allocator_sorted_list::sorted_free_iterator allocator_sorted_list::free_end() const noexcept
{
    return sorted_free_iterator();
}

allocator_sorted_list::sorted_iterator allocator_sorted_list::begin() const noexcept
{
    return sorted_iterator(_trusted_memory);
}

allocator_sorted_list::sorted_iterator allocator_sorted_list::end() const noexcept
{
    return sorted_iterator();
}

bool allocator_sorted_list::sorted_free_iterator::operator==(
        const allocator_sorted_list::sorted_free_iterator & other) const noexcept
{
    return this->_free_ptr == other._free_ptr;
}

bool allocator_sorted_list::sorted_free_iterator::operator!=(
        const allocator_sorted_list::sorted_free_iterator &other) const noexcept
{
    return !(*this == other);
}

allocator_sorted_list::sorted_free_iterator &allocator_sorted_list::sorted_free_iterator::operator++() & noexcept
{
    this->_free_ptr = reinterpret_cast<std::byte*>(*reinterpret_cast<void**>(this->_free_ptr));
    return *this;
}

allocator_sorted_list::sorted_free_iterator allocator_sorted_list::sorted_free_iterator::operator++(int n)
{
    sorted_free_iterator tmp(*this);
    ++(*this);
    return tmp;
}

size_t allocator_sorted_list::sorted_free_iterator::size() const noexcept
{
    return get_block_size(_free_ptr);
}

void *allocator_sorted_list::sorted_free_iterator::operator*() const noexcept
{
    return this->_free_ptr; 
}

allocator_sorted_list::sorted_free_iterator::sorted_free_iterator() : _free_ptr(nullptr) {}

allocator_sorted_list::sorted_free_iterator::sorted_free_iterator(void *trusted)

{
    this->_free_ptr = *reinterpret_cast<void**>(static_cast<std::byte*>(trusted) + allocator_metadata_size - sizeof(void*));
}

bool allocator_sorted_list::sorted_iterator::operator==(const allocator_sorted_list::sorted_iterator & other) const noexcept
{
    return (this->_current_ptr == other._current_ptr && this->_trusted_memory == other._trusted_memory);
}

bool allocator_sorted_list::sorted_iterator::operator!=(const allocator_sorted_list::sorted_iterator &other) const noexcept
{
    return !(*this == other);
}

allocator_sorted_list::sorted_iterator &allocator_sorted_list::sorted_iterator::operator++() & noexcept
{
    int offset = get_block_size(this->_current_ptr);
    this->_current_ptr = static_cast<std::byte*>(this->_current_ptr) + offset;
    return *this;
}

allocator_sorted_list::sorted_iterator allocator_sorted_list::sorted_iterator::operator++(int n)
{
    sorted_iterator tmp(*this);
    ++(*this);
    return tmp;
}

size_t allocator_sorted_list::sorted_iterator::size() const noexcept
{
    return get_block_size(_current_ptr);
}

void *allocator_sorted_list::sorted_iterator::operator*() const noexcept
{
    return _current_ptr;
}

allocator_sorted_list::sorted_iterator::sorted_iterator() : _free_ptr(nullptr), _current_ptr(nullptr), _trusted_memory(nullptr) {}

allocator_sorted_list::sorted_iterator::sorted_iterator(void *trusted)
{
    auto * ptr = reinterpret_cast<std::byte*>(trusted);
    _trusted_memory = ptr;
    _current_ptr = ptr + allocator_metadata_size;
    _free_ptr = *reinterpret_cast<void**>(ptr + allocator_metadata_size - sizeof(void*));
}

bool allocator_sorted_list::sorted_iterator::occupied() const noexcept
{
    std::byte * free_blocks = reinterpret_cast<std::byte*>(this->_free_ptr); 
    while (free_blocks != nullptr) {
        if (this->_current_ptr == free_blocks) {
            return false;
        }
        free_blocks = reinterpret_cast<std::byte*>((*reinterpret_cast<void**>(free_blocks)));
    }
    return true;
}

std::pmr::memory_resource*& allocator_sorted_list::get_parent() {
    return *reinterpret_cast<std::pmr::memory_resource**>(_trusted_memory);
}

std::mutex& allocator_sorted_list::get_mutex() const {
    auto * ptr = reinterpret_cast<std::byte*> (_trusted_memory); 
    size_t offset = sizeof(std::pmr::memory_resource*) + sizeof(allocator_with_fit_mode::fit_mode) + sizeof(size_t);
    return *reinterpret_cast<std::mutex*> (ptr + offset); 
}

allocator_with_fit_mode::fit_mode& allocator_sorted_list::get_fit_mode() {
    auto * ptr = reinterpret_cast<std::byte*>(_trusted_memory);
    return *reinterpret_cast<allocator_with_fit_mode::fit_mode*>(ptr + sizeof(std::pmr::memory_resource*));  
}

size_t * allocator_sorted_list::get_size_total() {
    auto * ptr = reinterpret_cast<std::byte*>(_trusted_memory);
    return reinterpret_cast<size_t*>(ptr + sizeof(std::pmr::memory_resource*) + sizeof(fit_mode));  
}

void * allocator_sorted_list::get_first_free_block() {
    auto * ptr = reinterpret_cast<std::byte*>(_trusted_memory);
    return *reinterpret_cast<void**>(ptr + allocator_metadata_size - sizeof(void*));
}

std::byte* allocator_sorted_list::read_next_block(void *block) {
    return reinterpret_cast<std::byte*>((*reinterpret_cast<void**>(block)));
}

void allocator_sorted_list::set_new_first_free_block(void * block) {
    auto *ptr = reinterpret_cast<std::byte *>(_trusted_memory) +
                sizeof(std::pmr::memory_resource *) + sizeof(fit_mode) +
                sizeof(size_t) + sizeof(std::mutex);
    *reinterpret_cast<void **>(ptr) = block;
}

void allocator_sorted_list::insert_free_block(void * block) {
    std::byte* byte_block = reinterpret_cast<std::byte*>(block); 
    std::byte * curr = reinterpret_cast<std::byte*>(get_first_free_block());
    std::byte * prev = nullptr;

    while (curr != nullptr && curr < byte_block) {
        prev = curr;
        curr = read_next_block(curr);
    }

    *reinterpret_cast<std::byte**>(byte_block) = curr;
    if (prev == nullptr) {
        set_new_first_free_block(byte_block);
    } else {
        *reinterpret_cast<void**>(prev) = byte_block; 
    }
}

