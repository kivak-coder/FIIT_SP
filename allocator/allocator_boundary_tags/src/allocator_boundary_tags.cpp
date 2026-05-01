#include <not_implemented.h>
#include "../include/allocator_boundary_tags.h"

// [size_t size | void* back | void* forward | void* parent_mem]

allocator_boundary_tags::~allocator_boundary_tags()
{
    get_mutex().~mutex();
    size_t total = *reinterpret_cast<size_t*>(static_cast<std::byte*>(_trusted_memory) + sizeof(std::pmr::memory_resource*) + sizeof(fit_mode));
    (*get_parent()).deallocate(_trusted_memory, total);
    _trusted_memory = nullptr;}

allocator_boundary_tags::allocator_boundary_tags(
    allocator_boundary_tags &&other) noexcept
{
    throw not_implemented("allocator_boundary_tags::allocator_boundary_tags(allocator_boundary_tags &&) noexcept", "your code should be here...");
}

allocator_boundary_tags &allocator_boundary_tags::operator=(
    allocator_boundary_tags &&other) noexcept
{
    throw not_implemented("allocator_boundary_tags &allocator_boundary_tags::operator=(allocator_boundary_tags &&) noexcept", "your code should be here...");
}

/** If parent_allocator* == nullptr you should use std::pmr::get_default_resource()
 */
allocator_boundary_tags::allocator_boundary_tags(
        size_t space_size,
        std::pmr::memory_resource *parent_allocator,
        allocator_with_fit_mode::fit_mode allocate_fit_mode)
{
    if (parent_allocator == nullptr) {
        parent_allocator = std::pmr::get_default_resource();
    }
    _trusted_memory = parent_allocator->allocate(space_size + allocator_metadata_size);
    auto * ptr = reinterpret_cast<std::byte*>(_trusted_memory);
    
    *reinterpret_cast<std::pmr::memory_resource**>(ptr) = parent_allocator;
    ptr += sizeof(parent_allocator);
    *reinterpret_cast<allocator_with_fit_mode::fit_mode*>(ptr) = allocate_fit_mode;
    ptr += sizeof(fit_mode);
    *reinterpret_cast<size_t*>(ptr) = space_size + allocator_metadata_size;
    ptr += sizeof(size_t);
    
    new (ptr) std::mutex; // placement new
    ptr += sizeof(std::mutex);
    *reinterpret_cast<void**>(ptr) = nullptr; // first_occupied = nullptr
}

size_t get_block_size(void * block) {
    return *reinterpret_cast<size_t*>(block);
}

[[nodiscard]] void *allocator_boundary_tags::do_allocate_sm(
    size_t size)
{
    // свободный блок определяется как разность между двумя занятыми, поэтому будем двигаться по занятым 
    std::lock_guard<std::mutex> lock(get_mutex());
    std::byte * ptr_cur = reinterpret_cast<std::byte*>(get_first_occupied_block()); // первый занятый
    std::byte * free_start = reinterpret_cast<std::byte*>(_trusted_memory) + allocator_metadata_size;
    std::byte * ptr_prev = nullptr; // предыдущий занятый
    size_t worst_size = 0;
    size_t best_size = std::numeric_limits<size_t>::max();
    size_t first_size = 0;

    std::byte * first_fit_place = nullptr; std::byte * prev_first_fit = nullptr;
    std::byte * best_fit_place = nullptr; std::byte * prev_best_fit = nullptr;
    std::byte * worst_fit_place = nullptr; std::byte * prev_worst_fit = nullptr;
    std::byte * allocated_block = nullptr;
    fit_mode fm = get_fit_mode();
    
    if (ptr_cur == nullptr) { // еще не было выделено ни одного блока => надо выделить первый
        if (reinterpret_cast<std::byte*>(end_of_memory()) - free_start - occupied_block_metadata_size > size) {
            std::byte * new_block = free_start;
            *reinterpret_cast<size_t*>(new_block) = size;
            *reinterpret_cast<void**>(new_block + sizeof(size_t)) = nullptr;
            *reinterpret_cast<void**>(new_block + occupied_block_metadata_size - 2 * sizeof(void*)) = nullptr;
            *reinterpret_cast<void**>(new_block + occupied_block_metadata_size - sizeof(void*)) = this->_trusted_memory;
            *reinterpret_cast<void**>(reinterpret_cast<std::byte*>(_trusted_memory) + allocator_metadata_size - sizeof(void*)) = new_block;
            return new_block + occupied_block_metadata_size;
        } else {
            throw std::bad_alloc(); // не хватило даже полного пространства нашего блока
        }
    }
    
    while (ptr_cur != nullptr) {
        std::byte* free_begin;
        if (ptr_prev == nullptr) {
            free_begin = reinterpret_cast<std::byte*>(_trusted_memory) + allocator_metadata_size;
        } else {
            free_begin = ptr_prev + occupied_block_metadata_size + get_block_size(ptr_prev);
        }

        size_t size_free_block = ptr_cur - free_begin;
        
        if (fm == allocator_with_fit_mode::fit_mode::first_fit) {
            if (size_free_block >= size + occupied_block_metadata_size) {
                first_fit_place = free_begin;
                prev_first_fit = ptr_prev;
                first_size = size_free_block - occupied_block_metadata_size;
                break;
            }
        }
        if (fm == allocator_with_fit_mode::fit_mode::the_best_fit) {
            if (size_free_block - occupied_block_metadata_size < best_size && size_free_block >= size + occupied_block_metadata_size) {
                best_fit_place = free_begin;
                best_size = size_free_block - occupied_block_metadata_size;
                prev_best_fit = ptr_prev;
            }
        }
        if (fm == allocator_with_fit_mode::fit_mode::the_worst_fit) {
            if (size_free_block - occupied_block_metadata_size > worst_size && size_free_block >= size + occupied_block_metadata_size) {
                worst_fit_place = free_begin;
                worst_size = size_free_block - occupied_block_metadata_size;
                prev_worst_fit = ptr_prev;
            }
        }
        ptr_prev = ptr_cur;
        ptr_cur = reinterpret_cast<std::byte*>(get_next_occupied_block(ptr_cur));
    }

    size_t tail_size = reinterpret_cast<std::byte*>(end_of_memory()) - ptr_prev - occupied_block_metadata_size - get_block_size(ptr_prev); // самый последний свободный блок, надо учесть!
    if (tail_size >= size + occupied_block_metadata_size) {
        if (fm == allocator_with_fit_mode::fit_mode::first_fit && first_fit_place == nullptr) {
            first_fit_place = ptr_prev + get_block_size(ptr_prev) + occupied_block_metadata_size;
            first_size = tail_size - occupied_block_metadata_size;
            prev_first_fit = ptr_prev;
        }
        if (fm == allocator_with_fit_mode::fit_mode::the_best_fit && tail_size - occupied_block_metadata_size < best_size) {
            best_fit_place = ptr_prev + get_block_size(ptr_prev) + occupied_block_metadata_size;
            best_size = tail_size - occupied_block_metadata_size;
            prev_best_fit = ptr_prev;
        }
        if (fm == allocator_with_fit_mode::fit_mode::the_worst_fit && tail_size - occupied_block_metadata_size > worst_size) {
            worst_fit_place = ptr_prev + get_block_size(ptr_prev) + occupied_block_metadata_size;
            worst_size = tail_size - occupied_block_metadata_size;
            prev_worst_fit = ptr_prev;
        }
    }

    if (fm == allocator_with_fit_mode::fit_mode::first_fit) {
        if (first_fit_place == nullptr) {
            throw std::bad_alloc();
        }
        if (first_size - size < occupied_block_metadata_size) {
            size = first_size;
        }
        allocated_block = allocate_and_fill_meta(prev_first_fit, first_fit_place, size); 
    }

    if (fm == allocator_with_fit_mode::fit_mode::the_best_fit) {
        if (best_fit_place == nullptr) {
            throw std::bad_alloc();
        }
        if (best_size - size < occupied_block_metadata_size) {
            size = best_size;
        }
        allocated_block = allocate_and_fill_meta(prev_best_fit, best_fit_place, size);
    }

    if (fm == allocator_with_fit_mode::fit_mode::the_worst_fit) {
        if (worst_fit_place == nullptr) {
            throw std::bad_alloc();
        }
        if (worst_size - size < occupied_block_metadata_size) {
            size = worst_size;
        }
        allocated_block = allocate_and_fill_meta(prev_worst_fit, worst_fit_place, size);
    }
    return reinterpret_cast<void*>(allocated_block + occupied_block_metadata_size);

}

std::byte * allocator_boundary_tags::allocate_and_fill_meta(std::byte * ptr_prev, std::byte * ptr_cur, size_t size) {
    // надо заполнить мета нового занятого. размер и парент - очевидно
    // back - ptr_prev, forward = ptr_prev.next
    std::byte * new_block = ptr_cur;
    *reinterpret_cast<size_t*>(new_block) = size;
    if (ptr_prev == nullptr) {
        *reinterpret_cast<void**>(new_block + sizeof(size_t)) = nullptr;         
        *reinterpret_cast<void**>(new_block + sizeof(size_t) + sizeof(void*)) = nullptr; 
        *reinterpret_cast<void**>(new_block + sizeof(size_t) + 2*sizeof(void*)) = this->_trusted_memory;

        auto * first_ptr = reinterpret_cast<std::byte*>(_trusted_memory) + allocator_metadata_size - sizeof(void*);
        *reinterpret_cast<void**>(first_ptr) = new_block;
        return new_block;
    }
    std::byte * next_occupied = reinterpret_cast<std::byte*>(get_next_occupied_block(ptr_prev));
    *reinterpret_cast<void**>(new_block + sizeof(size_t)) = ptr_prev;
    *reinterpret_cast<void**>(new_block + sizeof(size_t) + sizeof(void*)) = next_occupied;
    *reinterpret_cast<void**>(new_block + sizeof(size_t) + 2 * sizeof(void*)) = this->_trusted_memory;
    // у предыдущего надо обновить следующего - наш ptr_cur
    // у следующего надо обновить предыдущего - наш ptr_cur
    if (ptr_prev != nullptr) {
        *reinterpret_cast<void**>(ptr_prev + sizeof(size_t) + sizeof(void*)) = new_block;
    }
    if (next_occupied != nullptr) {
        *reinterpret_cast<void**>(next_occupied + sizeof(size_t)) = new_block;
    }
    return new_block;
}

void allocator_boundary_tags::do_deallocate_sm(
    void *at)
{
    std::lock_guard<std::mutex> lock(get_mutex());
    if (at == nullptr) {
        throw std::exception();
    }
    // ВАЖНО: проверить, принадлежит ли удаляемый кусок нашей памяти
    std::byte * block = reinterpret_cast<std::byte*>(at) - occupied_block_metadata_size;
    if (block > this->end_of_memory() || block < reinterpret_cast<std::byte*>(this->_trusted_memory) + allocator_metadata_size) {
        throw std::exception();
    }
    std::byte * prev_occupied = reinterpret_cast<std::byte*>(get_prev_occupied_block(block));
    std::byte * next_occupied = reinterpret_cast<std::byte*>(get_next_occupied_block(block));
    if (next_occupied == nullptr && prev_occupied == nullptr) { // мы - первый и единственный занятый
        *reinterpret_cast<void**>(reinterpret_cast<std::byte*>(_trusted_memory) + allocator_metadata_size - sizeof(void*)) = nullptr;
    } else if (next_occupied == nullptr && prev_occupied != nullptr) { // мы - последний занятый
        *reinterpret_cast<void**>(prev_occupied + sizeof(size_t) + sizeof(void*)) = nullptr;
    } else if (next_occupied != nullptr && prev_occupied == nullptr) { // мы - первый занятый
        *reinterpret_cast<void**>(reinterpret_cast<std::byte*>(_trusted_memory) + allocator_metadata_size - sizeof(void*)) = next_occupied;
        *reinterpret_cast<void**>(next_occupied + sizeof(size_t)) = nullptr;

    } else {
        *reinterpret_cast<void**>(prev_occupied + sizeof(size_t) + sizeof(void*)) = next_occupied;
        *reinterpret_cast<void**>(next_occupied + sizeof(size_t)) = prev_occupied;
    }
}

inline void allocator_boundary_tags::set_fit_mode(
    allocator_with_fit_mode::fit_mode mode)
{
    std::lock_guard<std::mutex> lock(get_mutex());
    fit_mode& fm = get_fit_mode();
    fm = mode;
}

std::vector<allocator_test_utils::block_info> allocator_boundary_tags::get_blocks_info() const
{
    std::lock_guard<std::mutex> lock(get_mutex());
    return get_blocks_info_inner();
}

allocator_boundary_tags::boundary_iterator allocator_boundary_tags::begin() const noexcept
{
    return boundary_iterator(_trusted_memory);
}

allocator_boundary_tags::boundary_iterator allocator_boundary_tags::end() const noexcept
{
    return boundary_iterator();
}

std::vector<allocator_test_utils::block_info> allocator_boundary_tags::get_blocks_info_inner() const
{
    std::vector<allocator_test_utils::block_info> info;
    std::byte* start = reinterpret_cast<std::byte*>(_trusted_memory) + allocator_metadata_size;
    std::byte* end = reinterpret_cast<std::byte*>(end_of_memory());
    
    // Получаем первый занятый блок
    std::byte* first_occupied = reinterpret_cast<std::byte*>(get_first_occupied_block());
    
    std::byte* current = start;
    std::byte* curr_occupied = first_occupied;
    
    while (current < end) {
        bool occupied = (curr_occupied != nullptr && curr_occupied == current);
        size_t block_size;
        
        if (occupied) {
            // Занятый блок – читаем размер из заголовка
            block_size = get_block_size(current) + occupied_block_metadata_size;
            // Переходим к следующему занятому блоку
            curr_occupied = reinterpret_cast<std::byte*>(get_next_occupied_block(curr_occupied));
        } else {
            // Свободный блок – его размер определяется расстоянием до следующего занятого
            std::byte* next_occupied = curr_occupied; // следующий занятый (или nullptr)
            if (next_occupied == nullptr) {
                block_size = end - current;
            } else {
                block_size = next_occupied - current;
            }
        }
        
        // Добавляем информацию о блоке (размер + флаг занятости)
        info.push_back({block_size, occupied});
        
        // Перемещаемся к следующему блоку
        current += block_size;
    }
    
    return info;
}

allocator_boundary_tags::allocator_boundary_tags(const allocator_boundary_tags &other)
{
    throw not_implemented("allocator_boundary_tags::allocator_boundary_tags(const allocator_boundary_tags &other)", "your code should be here...");
}

allocator_boundary_tags &allocator_boundary_tags::operator=(const allocator_boundary_tags &other)
{
    throw not_implemented("allocator_boundary_tags &allocator_boundary_tags::operator=(const allocator_boundary_tags &other)", "your code should be here...");
}

bool allocator_boundary_tags::do_is_equal(const std::pmr::memory_resource &other) const noexcept
{
    //
}

bool allocator_boundary_tags::boundary_iterator::operator==(
        const allocator_boundary_tags::boundary_iterator &other) const noexcept
{
    return this->_trusted_memory == other._trusted_memory && this->_occupied == other._occupied && this->_occupied == other._occupied;
}

bool allocator_boundary_tags::boundary_iterator::operator!=(
        const allocator_boundary_tags::boundary_iterator & other) const noexcept
{
    return !(*this == other);
}

allocator_boundary_tags::boundary_iterator &allocator_boundary_tags::boundary_iterator::operator++() & noexcept
{
    if (_occupied) {
        size_t block_size = get_block_size(_occupied_ptr);
        _occupied = next_is_free(_occupied_ptr);
        _occupied_ptr = reinterpret_cast<std::byte*>(_occupied_ptr) + occupied_block_metadata_size + block_size;
    } else {
        // а че делать если блок свободный
    }
}

allocator_boundary_tags::boundary_iterator &allocator_boundary_tags::boundary_iterator::operator--() & noexcept
{
    throw not_implemented("allocator_boundary_tags::boundary_iterator &allocator_boundary_tags::boundary_iterator::operator--() & noexcept", "your code should be here...");
}

allocator_boundary_tags::boundary_iterator allocator_boundary_tags::boundary_iterator::operator++(int n)
{
    boundary_iterator tmp(*this); // как это работает то???????
    ++(*this);
    return tmp;
}

allocator_boundary_tags::boundary_iterator allocator_boundary_tags::boundary_iterator::operator--(int n)
{
    boundary_iterator tmp(*this);
    --(*this);
    return tmp;
}

size_t allocator_boundary_tags::boundary_iterator::size() const noexcept
{
    return get_block_size(this->_occupied_ptr);
}

bool allocator_boundary_tags::boundary_iterator::occupied() const noexcept
{
    return this->_occupied;
}

void* allocator_boundary_tags::boundary_iterator::operator*() const noexcept
{
    return this->_occupied_ptr;
}

allocator_boundary_tags::boundary_iterator::boundary_iterator() : _trusted_memory(nullptr), _occupied_ptr(nullptr), _occupied(false) {}

allocator_boundary_tags::boundary_iterator::boundary_iterator(void *trusted) : _trusted_memory(trusted)
{
    this->_occupied_ptr = *reinterpret_cast<void**>(reinterpret_cast<std::byte*>(trusted) + allocator_metadata_size);
}

void *allocator_boundary_tags::boundary_iterator::get_ptr() const noexcept
{
    throw not_implemented("void *allocator_boundary_tags::boundary_iterator::get_ptr() const noexcept", "your code should be here...");
}

std::pmr::memory_resource*& allocator_boundary_tags::get_parent() const {
    return *reinterpret_cast<std::pmr::memory_resource**>(_trusted_memory);
}

std::mutex& allocator_boundary_tags::get_mutex() const {
    auto * ptr = reinterpret_cast<std::byte*> (_trusted_memory); 
    size_t offset = sizeof(std::pmr::memory_resource*) + sizeof(allocator_with_fit_mode::fit_mode) + sizeof(size_t);
    return *reinterpret_cast<std::mutex*> (ptr + offset); 
}

allocator_with_fit_mode::fit_mode& allocator_boundary_tags::get_fit_mode() const {
    auto * ptr = reinterpret_cast<std::byte*>(_trusted_memory);
    return *reinterpret_cast<allocator_with_fit_mode::fit_mode*>(ptr + sizeof(std::pmr::memory_resource*));  
}

size_t allocator_boundary_tags::get_size_total() const {
    auto * ptr = reinterpret_cast<std::byte*>(_trusted_memory);
    return *reinterpret_cast<size_t*>(ptr + sizeof(std::pmr::memory_resource*) + sizeof(fit_mode));  
}

void * allocator_boundary_tags::get_first_occupied_block() const {
    auto * ptr = reinterpret_cast<std::byte*>(_trusted_memory);
    return *reinterpret_cast<void**>(ptr + sizeof(std::pmr::memory_resource*) + sizeof(fit_mode) + sizeof(size_t) + sizeof(std::mutex));  
}

void * allocator_boundary_tags::get_next_occupied_block(void * block) {
    auto * ptr = reinterpret_cast<std::byte*>(block);
    return *reinterpret_cast<void**>(ptr + sizeof(size_t) + sizeof(void*));
}

void * allocator_boundary_tags::get_prev_occupied_block(void * block) const {
    auto * ptr = reinterpret_cast<std::byte*>(block);
    if (ptr == static_cast<std::byte*>(_trusted_memory) + allocator_metadata_size) { // самый первый блок - предыдущего нет
        return nullptr;
    } 
    return *reinterpret_cast<void**>(ptr + sizeof(size_t));
}

bool allocator_boundary_tags::next_is_free(void * block) { // нужна проверка не вышли ли мы за пределы памяти!
    auto * ptr = reinterpret_cast<std::byte*>(block);
    if (get_next_occupied_block(block) == ptr + occupied_block_metadata_size + get_block_size(block)) { // следующий тоже занят
        return false;
    }
    return true;
}

void * allocator_boundary_tags::get_next_free_block(void * block) {
    auto * ptr = reinterpret_cast<std::byte*>(block);
    while (!next_is_free(ptr)) { // пока все последующие блоки заняты
        ptr = ptr + occupied_block_metadata_size + get_block_size(ptr); // а если за пределы памяти вышли!!!!!!
    }
    return ptr;
}

void * allocator_boundary_tags::end_of_memory() const {
    return reinterpret_cast<std::byte*>(_trusted_memory) + get_size_total();
}