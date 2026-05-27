#include <not_implemented.h>

#include "../include/allocator_red_black_tree.h"

// [block_data | prev_block | next_block | trusted_mem] - occupied
// [block_data | prev_block | next_block | parent_node | left_node | right_node] - free

allocator_red_black_tree::~allocator_red_black_tree()
{
     if (!this->_trusted_memory) {return;}
    get_mutex().~mutex();
    size_t total = *reinterpret_cast<size_t*>(static_cast<std::byte*>(_trusted_memory) + sizeof(std::pmr::memory_resource*) + sizeof(fit_mode));
    (*get_parent()).deallocate(_trusted_memory, total);
    _trusted_memory = nullptr;
}

allocator_red_black_tree::allocator_red_black_tree(
    allocator_red_black_tree &&other) noexcept
{
   if (this->_trusted_memory) 
   {
        get_mutex().~mutex();
        size_t total = get_total_size();
        (*get_parent()).deallocate(_trusted_memory, total);
        this->_trusted_memory = other._trusted_memory;
        other._trusted_memory = nullptr;
    }
}

allocator_red_black_tree &allocator_red_black_tree::operator=(
    allocator_red_black_tree &&other) noexcept
{
    if (this != &other) {
        if (this->_trusted_memory) {
            get_mutex().~mutex();
            size_t total = get_total_size();
            (*get_parent()).deallocate(_trusted_memory, total);
            this->_trusted_memory = other._trusted_memory;
            other._trusted_memory = nullptr;
        }
    }
    return *this;
}

allocator_red_black_tree::allocator_red_black_tree(
        size_t space_size,
        std::pmr::memory_resource *parent_allocator,
        allocator_with_fit_mode::fit_mode allocate_fit_mode)
{
    if (parent_allocator == nullptr) {
        parent_allocator = std::pmr::get_default_resource();
    }
    this->_trusted_memory = parent_allocator->allocate(space_size + allocator_metadata_size + free_block_metadata_size);
    auto * ptr = reinterpret_cast<std::byte*>(_trusted_memory);

    get_parent() = parent_allocator;
    get_fit_mode() = allocate_fit_mode;
    get_total_size() = space_size + allocator_metadata_size + free_block_metadata_size;
    get_root() = nullptr;

    ptr = ptr + sizeof(std::pmr::memory_resource*) + sizeof(allocator_with_fit_mode::fit_mode) + sizeof(size_t);
    new (ptr) std::mutex;
    ptr += sizeof(std::mutex);
    ptr += sizeof(void*);

    block_data meta = {false, block_color::BLACK};
    *reinterpret_cast<block_data*>(ptr) = meta;
    get_prev_block(ptr) = nullptr;
    get_next_block(ptr) = nullptr;
    get_parent_node(ptr) = nullptr;
    get_left_node(ptr) = nullptr;
    get_right_node(ptr) = nullptr;
    add_block(ptr, space_size);
}

allocator_red_black_tree::allocator_red_black_tree(const allocator_red_black_tree &other) {}

allocator_red_black_tree &allocator_red_black_tree::operator=(const allocator_red_black_tree &other) {}

bool allocator_red_black_tree::do_is_equal(const std::pmr::memory_resource &other) const noexcept
{
    return dynamic_cast<const allocator_red_black_tree*>(&other) != nullptr;
}

size_t allocator_red_black_tree::get_block_size(void * block) {
    auto * ptr = reinterpret_cast<std::byte*>(block);
    auto * next = reinterpret_cast<std::byte*>(get_next_block(block));
    if (next == nullptr) {
        return reinterpret_cast<std::byte*>(end_of_memory()) - ptr;
    }
    return next - ptr;
}

[[nodiscard]] void *allocator_red_black_tree::do_allocate_sm(
    size_t size)
{
    std::lock_guard<std::mutex> lock(get_mutex());
    if (size > get_total_size()) {
        throw std::bad_alloc();
    }
    size_t needed_size = size + occupied_block_metadata_size;
    fit_mode fm = get_fit_mode();
    void * block_to_alloc = nullptr;
    if (fm == allocator_with_fit_mode::fit_mode::first_fit) {
        block_to_alloc = find_first_fit(size);
    }
    if (fm == allocator_with_fit_mode::fit_mode::the_best_fit) {
        block_to_alloc = find_the_best_fit(size); 
    }
    if (fm == allocator_with_fit_mode::fit_mode::the_worst_fit) {
        block_to_alloc = find_the_worst_fit(size);
    }
    if (block_to_alloc == nullptr || get_block_size(block_to_alloc) < needed_size) {
        throw std::bad_alloc();
    }
    get_block_meta(block_to_alloc)->occupied = true;
    remove_block(block_to_alloc);

    if (get_block_size(block_to_alloc) > needed_size + free_block_metadata_size) {
        void * new_block = reinterpret_cast<std::byte*>(block_to_alloc) + occupied_block_metadata_size + size;
        void * old_next = get_next_block(block_to_alloc);
        get_next_block(block_to_alloc) = new_block;
        get_prev_block(new_block) = block_to_alloc;
        get_next_block(new_block) = old_next;
        if (old_next != nullptr) {
            get_prev_block(old_next) = new_block;
        }
        get_block_meta(new_block)->occupied = false;
        get_block_meta(new_block)->color = block_color::RED;
        get_parent_node(new_block) = nullptr;
        get_left_node(new_block) = nullptr;
        get_right_node(new_block) = nullptr;
        add_block(new_block, get_block_size(new_block));
    }

    *reinterpret_cast<void**>(reinterpret_cast<std::byte*>(block_to_alloc) + sizeof(block_data) + 2 * sizeof(void*)) = this->_trusted_memory;

    return reinterpret_cast<std::byte*>(block_to_alloc) + occupied_block_metadata_size;
}

void* allocator_red_black_tree::find_first_fit(size_t size) {
    // обход деревца
    auto * cur = get_root();
    size_t needed_size = size + occupied_block_metadata_size;
    while (cur != nullptr && get_block_size(cur) < needed_size) {
        cur = get_right_node(cur);
    }
    return cur;
}

void* allocator_red_black_tree::find_the_best_fit(size_t size) {
    auto * cur = get_root();
    void * best_fit = nullptr;
    size_t needed_size = size + occupied_block_metadata_size;
    while (cur != nullptr) {
        if (get_block_size(cur) >= needed_size) {
            best_fit = cur;
            cur = get_left_node(cur);
        } else {
            cur = get_right_node(cur);
        }
    }
    return best_fit;
}

void* allocator_red_black_tree::find_the_worst_fit(size_t size) {
    auto * cur = get_root();
    void * worst_fit = nullptr;
    while (cur != nullptr) {
        worst_fit = cur;
        cur = get_right_node(cur);
    }
    return worst_fit;
}

void allocator_red_black_tree::do_deallocate_sm(
    void *at)
{
    std::lock_guard<std::mutex> lock(get_mutex());
    if (at == nullptr) {
        throw std::exception();
    }
    void * parent_mem = *reinterpret_cast<void**>(reinterpret_cast<std::byte*>(at) - sizeof(void*));
    if (parent_mem != this->_trusted_memory) {
        throw std::exception();
    }

    std::byte * block = reinterpret_cast<std::byte*>(at) - occupied_block_metadata_size;
    if (block > this->end_of_memory() || block < reinterpret_cast<std::byte*>(this->_trusted_memory) + allocator_metadata_size) {
        throw std::exception();
    }

    get_block_meta(block)->occupied = false;
    auto * next_block = get_next_block(block);
    if (next_block != nullptr && get_block_meta(next_block)->occupied == false) {
        remove_block(next_block);
        void *next_next = get_next_block(next_block);
        get_next_block(block) = next_next;
        if (next_next != nullptr) {
            get_prev_block(next_next) = block;
        }
    }

    auto * prev_block = get_prev_block(block);
    void * new_free_block = block;
    if (prev_block != nullptr && get_block_meta(prev_block)->occupied == false) {
        remove_block(prev_block);
        new_free_block = prev_block;
        get_next_block(prev_block) = get_next_block(block);
        if (get_next_block(block) != nullptr) {
            get_prev_block(get_next_block(block)) = prev_block;
        }
    } 
    size_t new_free_size = get_block_size(new_free_block);
    add_block(new_free_block, new_free_size);
}

void allocator_red_black_tree::rotate_left(void* x) {
    void* y = get_right_node(x);
    if (y == nullptr) return;
    
    get_right_node(x) = get_left_node(y);
    if (get_left_node(y) != nullptr) {
        get_parent_node(get_left_node(y)) = x;
    }
    get_parent_node(y) = get_parent_node(x);
    
    if (get_parent_node(x) == nullptr) {
        get_root() = y;
    } else if (is_left_child(x)) {
        get_left_node(get_parent_node(x)) = y;
    } else {
        get_right_node(get_parent_node(x)) = y;
    }
    get_left_node(y) = x;
    get_parent_node(x) = y;
}

void allocator_red_black_tree::rotate_right(void* x) {
    void* y = get_left_node(x);
    if (y == nullptr) return;
    
    get_left_node(x) = get_right_node(y);
    if (get_right_node(y) != nullptr) {
        get_parent_node(get_right_node(y)) = x;
    }
    get_parent_node(y) = get_parent_node(x);
    
    if (get_parent_node(x) == nullptr) {
        get_root() = y;
    } else if (is_right_child(x)) {
        get_right_node(get_parent_node(x)) = y;
    } else {
        get_left_node(get_parent_node(x)) = y;
    }
    get_right_node(y) = x;
    get_parent_node(x) = y;
}

void allocator_red_black_tree::transplant(void* u, void* v) {
    if (get_parent_node(u) == nullptr) {
        get_root() = v;
    } else if (is_left_child(u)) {
        get_left_node(get_parent_node(u)) = v;
    } else {
        get_right_node(get_parent_node(u)) = v;
    }
    if (v != nullptr) {
        get_parent_node(v) = get_parent_node(u);
    }
}

void allocator_red_black_tree::add_block(void * block, size_t size) {
    auto * cur = get_root();
    void * parent = nullptr;
    size_t needed_size = get_block_size(block);
    
    while (cur != nullptr) {
        parent = cur;
        if (needed_size < get_block_size(cur)) {
            cur = get_left_node(cur);
        } else {
            cur = get_right_node(cur);
        }
    }
    
    get_parent_node(block) = parent;
    get_left_node(block) = nullptr;
    get_right_node(block) = nullptr;
    get_block_meta(block)->color = block_color::RED; 

    if (parent == nullptr) {
        get_root() = block;
    } else if (needed_size < get_block_size(parent)) {
        get_left_node(parent) = block;
    } else {
        get_right_node(parent) = block;
    }

    on_node_added(block);
}

void allocator_red_black_tree::on_node_added(void * block) {
    void* newNode = block;
    
    while (newNode != get_root() && get_block_meta(get_parent_node(newNode))->color == block_color::RED) {
        void* parent = get_parent_node(newNode);
        if (parent == nullptr) break; 
        void* grandparent = get_grandparent_node(newNode);
        if (grandparent == nullptr) break;  
        
        if (is_left_child(parent)) {
            void* uncle = get_right_node(grandparent);
            if (uncle != nullptr && get_block_meta(uncle)->color == block_color::RED) {
                get_block_meta(parent)->color = block_color::BLACK;
                get_block_meta(uncle)->color = block_color::BLACK;
                get_block_meta(grandparent)->color = block_color::RED;
                newNode = grandparent;
            } else {
                if (is_right_child(newNode)) {
                    newNode = parent;
                    rotate_left(newNode);
                    parent = get_parent_node(newNode);
                    if (parent == nullptr) break;  
                    grandparent = get_parent_node(parent);
                    if (grandparent == nullptr) break;  
                }
                get_block_meta(parent)->color = block_color::BLACK;
                get_block_meta(grandparent)->color = block_color::RED;
                rotate_right(grandparent);
            }
        } else {
            void* uncle = get_left_node(grandparent);
            if (uncle != nullptr && get_block_meta(uncle)->color == block_color::RED) {
                get_block_meta(parent)->color = block_color::BLACK;
                get_block_meta(uncle)->color = block_color::BLACK;
                get_block_meta(grandparent)->color = block_color::RED;
                newNode = grandparent;
            } else {
                if (is_left_child(newNode)) {
                    newNode = parent;
                    rotate_right(newNode);
                    parent = get_parent_node(newNode);
                    if (parent == nullptr) break; 
                    grandparent = get_parent_node(parent);
                    if (grandparent == nullptr) break; 
                }
                get_block_meta(parent)->color = block_color::BLACK;
                get_block_meta(grandparent)->color = block_color::RED;
                rotate_left(grandparent);
            }
        }
    }
    get_block_meta(get_root())->color = block_color::BLACK;
}

void allocator_red_black_tree::remove_block(void * z) {
    if (z == nullptr) return;
    
    void* y = z;
    void* x = nullptr;
    void* x_parent = nullptr;
    block_color y_original_color = get_block_meta(y)->color;
    
    if (get_left_node(z) == nullptr) {
        x = get_right_node(z);
        x_parent = get_parent_node(z);
        transplant(z, get_right_node(z));
    } else if (get_right_node(z) == nullptr) {
        x = get_left_node(z);
        x_parent = get_parent_node(z);
        transplant(z, get_left_node(z));
    } else {
        y = get_right_node(z);
        while (get_left_node(y) != nullptr) {
            y = get_left_node(y);
        }
        
        y_original_color = get_block_meta(y)->color;
        x = get_right_node(y);
        
        if (get_parent_node(y) == z) {
            x_parent = y;
        } else {
            x_parent = get_parent_node(y);
            transplant(y, get_right_node(y));
            get_right_node(y) = get_right_node(z);
            if (get_right_node(y) != nullptr) {
                get_parent_node(get_right_node(y)) = y;
            }
        }
        transplant(z, y);
        get_left_node(y) = get_left_node(z);
        if (get_left_node(y) != nullptr) {
            get_parent_node(get_left_node(y)) = y;
        }
        get_block_meta(y)->color = get_block_meta(z)->color;
    }
    
    if (y_original_color == block_color::BLACK) {
        on_node_removed(x, x_parent);
    }
    
    get_left_node(z) = nullptr;
    get_right_node(z) = nullptr;
    get_parent_node(z) = nullptr;
}

void allocator_red_black_tree::on_node_removed(void * x, void * x_parent) {
    while (x != get_root() && (x == nullptr || get_block_meta(x)->color == block_color::BLACK)) {
        if (x_parent == nullptr) break;  
        if (x == get_left_node(x_parent)) {
            void* sibling = get_right_node(x_parent);
            if (sibling != nullptr && get_block_meta(sibling)->color == block_color::RED) {
                get_block_meta(sibling)->color = block_color::BLACK;
                get_block_meta(x_parent)->color = block_color::RED;
                rotate_left(x_parent);
                sibling = get_right_node(x_parent);
            }
            if (sibling == nullptr) {
                x = x_parent;
                x_parent = get_parent_node(x);
                if (x_parent == nullptr) break;  
                continue;
            }
            
            bool left_is_black = (get_left_node(sibling) == nullptr || get_block_meta(get_left_node(sibling))->color == block_color::BLACK);
            bool right_is_black = (get_right_node(sibling) == nullptr || get_block_meta(get_right_node(sibling))->color == block_color::BLACK);
            
            if (left_is_black && right_is_black) {
                get_block_meta(sibling)->color = block_color::RED;
                x = x_parent;
                x_parent = get_parent_node(x);
                if (x_parent == nullptr) break;  
            } else {
                if (right_is_black) {
                    if (get_left_node(sibling) != nullptr) {
                        get_block_meta(get_left_node(sibling))->color = block_color::BLACK;
                    }
                    get_block_meta(sibling)->color = block_color::RED;
                    rotate_right(sibling);
                    sibling = get_right_node(x_parent);
                }
                get_block_meta(sibling)->color = get_block_meta(x_parent)->color;
                get_block_meta(x_parent)->color = block_color::BLACK;
                if (get_right_node(sibling) != nullptr) {
                    get_block_meta(get_right_node(sibling))->color = block_color::BLACK;
                }
                rotate_left(x_parent);
                x = get_root();
            }
        } else {
            void* sibling = get_left_node(x_parent);
            if (sibling != nullptr && get_block_meta(sibling)->color == block_color::RED) {
                get_block_meta(sibling)->color = block_color::BLACK;
                get_block_meta(x_parent)->color = block_color::RED;
                rotate_right(x_parent);
                sibling = get_left_node(x_parent);
            }
            if (sibling == nullptr) {
                x = x_parent;
                x_parent = get_parent_node(x);
                if (x_parent == nullptr) break;  
                continue;
            }
            
            bool left_is_black = (get_left_node(sibling) == nullptr || get_block_meta(get_left_node(sibling))->color == block_color::BLACK);
            bool right_is_black = (get_right_node(sibling) == nullptr || get_block_meta(get_right_node(sibling))->color == block_color::BLACK);
            
            if (left_is_black && right_is_black) {
                get_block_meta(sibling)->color = block_color::RED;
                x = x_parent;
                x_parent = get_parent_node(x);
                if (x_parent == nullptr) break;
            } else {
                if (left_is_black) {
                    if (get_right_node(sibling) != nullptr) {
                        get_block_meta(get_right_node(sibling))->color = block_color::BLACK;
                    }
                    get_block_meta(sibling)->color = block_color::RED;
                    rotate_left(sibling);
                    sibling = get_left_node(x_parent);
                }
                get_block_meta(sibling)->color = get_block_meta(x_parent)->color;
                get_block_meta(x_parent)->color = block_color::BLACK;
                if (get_left_node(sibling) != nullptr) {
                    get_block_meta(get_left_node(sibling))->color = block_color::BLACK;
                }
                rotate_right(x_parent);
                x = get_root();
            }
        }
    }
    if (x != nullptr) {
        get_block_meta(x)->color = block_color::BLACK;
    }
}

void allocator_red_black_tree::set_fit_mode(allocator_with_fit_mode::fit_mode mode)
{
    std::lock_guard<std::mutex> lock(get_mutex());
    fit_mode& fm = get_fit_mode();
    fm = mode;
}
std::vector<allocator_test_utils::block_info> allocator_red_black_tree::get_blocks_info() const
{
    std::lock_guard<std::mutex> lock(get_mutex());
    return get_blocks_info_inner();
}

std::vector<allocator_test_utils::block_info> allocator_red_black_tree::get_blocks_info_inner() const
{
    std::vector<allocator_test_utils::block_info> result;

    for (auto it = begin(); it != end(); ++it) {
        result.push_back({it.size(), it.occupied()});
    }
    return result;
}


allocator_red_black_tree::rb_iterator allocator_red_black_tree::begin() const noexcept
{
    return rb_iterator(this->_trusted_memory);
}

allocator_red_black_tree::rb_iterator allocator_red_black_tree::end() const noexcept
{
    return rb_iterator();
}


bool allocator_red_black_tree::rb_iterator::operator==(const allocator_red_black_tree::rb_iterator &other) const noexcept
{
    if (this->_block_ptr == nullptr && other._block_ptr == nullptr) {
        return true;
    }
    return this->_block_ptr == other._block_ptr && this->_trusted == other._trusted;
}

bool allocator_red_black_tree::rb_iterator::operator!=(const allocator_red_black_tree::rb_iterator &other) const noexcept
{
    return !(*this == other);
}

allocator_red_black_tree::rb_iterator &allocator_red_black_tree::rb_iterator::operator++() & noexcept
{
    auto * ptr = reinterpret_cast<std::byte*>(this->_block_ptr);
    this->_block_ptr = *reinterpret_cast<void**>(ptr +  sizeof(block_data) + sizeof(void*));
    return *this;
}

allocator_red_black_tree::rb_iterator allocator_red_black_tree::rb_iterator::operator++(int n)
{
    rb_iterator tmp(*this);
    ++(*this);
    return tmp;
}

size_t allocator_red_black_tree::rb_iterator::size() const noexcept
{
    auto * block = reinterpret_cast<std::byte*>(this->_block_ptr);
    size_t offset = sizeof(block_data) + sizeof(void*);
    auto * next = reinterpret_cast<std::byte*>(block + offset);
    if (next == nullptr) {
        auto * trusted = reinterpret_cast<std::byte*>(this->_trusted);
        size_t offset = sizeof(allocator_dbg_helper*) + sizeof(fit_mode);
        size_t total_size = *reinterpret_cast<size_t*>(trusted + offset);
        auto * end_of_mem = reinterpret_cast<std::byte*>(this->_trusted) + total_size;
        return end_of_mem - block;
    }
    return next - block;
}

void *allocator_red_black_tree::rb_iterator::operator*() const noexcept
{
    return this->_block_ptr;
}

allocator_red_black_tree::rb_iterator::rb_iterator() : _trusted(nullptr), _block_ptr(nullptr) {}

allocator_red_black_tree::rb_iterator::rb_iterator(void *trusted) : _trusted(trusted), _block_ptr(nullptr) {}

bool allocator_red_black_tree::rb_iterator::occupied() const noexcept
{
    auto * metadata = reinterpret_cast<block_data*>(this->_block_ptr);
    return metadata->occupied;
}

std::pmr::memory_resource*& allocator_red_black_tree::get_parent() const {
    return *reinterpret_cast<std::pmr::memory_resource**>(_trusted_memory);
}

std::mutex& allocator_red_black_tree::get_mutex() const {
    auto * ptr = reinterpret_cast<std::byte*> (_trusted_memory); 
    size_t offset = sizeof(std::pmr::memory_resource*) + sizeof(fit_mode) + sizeof(size_t);
    return *reinterpret_cast<std::mutex*> (ptr + offset); 
}

allocator_with_fit_mode::fit_mode& allocator_red_black_tree::get_fit_mode() const {
    auto * ptr = reinterpret_cast<std::byte*>(_trusted_memory);
    size_t offset = sizeof(std::pmr::memory_resource*);
    return *reinterpret_cast<allocator_with_fit_mode::fit_mode*>(ptr + offset);  
}

void*& allocator_red_black_tree::get_root() const {
    auto * ptr = reinterpret_cast<std::byte*>(_trusted_memory);
    size_t offset = sizeof(std::pmr::memory_resource*) + sizeof(fit_mode) + sizeof(size_t) + sizeof(std::mutex);
    return *reinterpret_cast<void**>(ptr + offset);
}

size_t& allocator_red_black_tree::get_total_size() const {
    auto * ptr = reinterpret_cast<std::byte*>(_trusted_memory);
    size_t offset = sizeof(std::pmr::memory_resource*) + sizeof(fit_mode);
    return *reinterpret_cast<size_t*>(ptr + offset);
}

allocator_red_black_tree::block_data* allocator_red_black_tree::get_block_meta(void * block) const {
    if (block == nullptr) {
        return nullptr;
    }
    return reinterpret_cast<block_data*>(block);;
}

void*& allocator_red_black_tree::get_next_block(void * block) const {
    static void * abob = nullptr;
    if (block == nullptr) {
        return abob;
    }
    auto * ptr = reinterpret_cast<std::byte*>(block);
    size_t offset = sizeof(block_data) + sizeof(void*);
    return *reinterpret_cast<void**>(ptr + offset);
}

void*& allocator_red_black_tree::get_prev_block(void * block) const {
    static void * abob = nullptr;
    if (block == nullptr) {
        return abob;
    }
    auto * ptr = reinterpret_cast<std::byte*>(block);
    size_t offset = sizeof(block_data);
    return *reinterpret_cast<void**>(ptr + offset);
}

void*& allocator_red_black_tree::get_parent_node(void * block) const {
    static void * abob = nullptr;
    if (block == nullptr) {
        return abob;
    }
    auto * ptr = reinterpret_cast<std::byte*>(block);
    size_t offset = sizeof(block_data) + 2 * sizeof(void*);
    return *reinterpret_cast<void**>(ptr + offset);
}

void*& allocator_red_black_tree::get_sibling_node(void * block) const {
    static void * abob = nullptr;
    if (block == nullptr) {
        return abob;
    }
    void * parent = get_parent_node(block);
    if (get_left_node(parent) == block) {
        return get_right_node(parent);
    } else {
        return get_left_node(parent);
    }
}

void*& allocator_red_black_tree::get_uncle_node(void * block) const {
    void * parent = get_parent_node(block);
    return get_sibling_node(parent);
}

void *& allocator_red_black_tree::get_grandparent_node(void * block) const {
    void * parent = get_parent_node(block);
    return get_parent_node(parent);
}

bool allocator_red_black_tree::is_left_child(void * block) const {
    void * parent = get_parent_node(block);
    return (parent != nullptr && get_left_node(parent) == block);
}

bool allocator_red_black_tree::is_right_child(void * block) const {
    void * parent = get_parent_node(block);
    return (parent != nullptr && get_right_node(parent) == block);
}

void*& allocator_red_black_tree::get_left_node(void * block) const{
    static void * abob = nullptr;
    if (block == nullptr) {
        return abob;
    }
    auto * ptr = reinterpret_cast<std::byte*>(block);
    size_t offset = sizeof(block_data) + 3 * sizeof(void*);
    return *reinterpret_cast<void**>(ptr + offset);
}

void*& allocator_red_black_tree::get_right_node(void * block) const {
    static void * abob = nullptr;
    if (block == nullptr) {
        return abob;
    }
    auto * ptr = reinterpret_cast<std::byte*>(block);
    size_t offset = sizeof(block_data) + 4 * sizeof(void*);
    return *reinterpret_cast<void**>(ptr + offset);
}


void* allocator_red_black_tree::end_of_memory() const {
    return reinterpret_cast<std::byte*>(_trusted_memory) + get_total_size();
}
