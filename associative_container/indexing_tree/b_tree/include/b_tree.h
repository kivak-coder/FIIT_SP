#ifndef SYS_PROG_B_TREE_H
#define SYS_PROG_B_TREE_H

#include <iterator>
#include <utility>
#include <boost/container/static_vector.hpp>
#include <stack>
#include <pp_allocator.h>
#include <associative_container.h>
#include <not_implemented.h>
#include <initializer_list>

template <typename tkey, typename tvalue, comparator<tkey> compare = std::less<tkey>, std::size_t t = 5>
class B_tree final : private compare // EBCO
{
public:

    using tree_data_type = std::pair<tkey, tvalue>;
    using tree_data_type_const = std::pair<const tkey, tvalue>;
    using value_type = tree_data_type_const;

private:

    static constexpr const size_t minimum_keys_in_node = t - 1;
    static constexpr const size_t maximum_keys_in_node = 2 * t - 1;

    // region comparators declaration

    inline bool compare_keys(const tkey& lhs, const tkey& rhs) const;
    inline bool compare_pairs(const tree_data_type& lhs, const tree_data_type& rhs) const;

    // endregion comparators declaration


    struct btree_node
    {
        boost::container::static_vector<tree_data_type, maximum_keys_in_node + 1> _keys;
        boost::container::static_vector<btree_node*, maximum_keys_in_node + 2> _pointers;
        btree_node() noexcept;
    };

    pp_allocator<value_type> _allocator;
    btree_node* _root;
    size_t _size;

    pp_allocator<value_type> get_allocator() const noexcept;
    static void print_node(std::ostream& os, const btree_node* node, int depth, const std::string& indent, bool last);

public:

    // region constructors declaration

    explicit B_tree(const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    explicit B_tree(pp_allocator<value_type> alloc, const compare& comp = compare());

    template<input_iterator_for_pair<tkey, tvalue> iterator>
    explicit B_tree(iterator begin, iterator end, const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    B_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    // endregion constructors declaration

    // region five declaration
    btree_node * copy_subtree(const btree_node* node);

    B_tree(const B_tree& other);

    B_tree(B_tree&& other) noexcept;

    B_tree& operator=(const B_tree& other);

    B_tree& operator=(B_tree&& other) noexcept;

    ~B_tree() noexcept;

    // endregion five declaration

    // region iterators declaration

    class btree_iterator;
    class btree_reverse_iterator;
    class btree_const_iterator;
    class btree_const_reverse_iterator;

    class btree_iterator final
    {
        std::stack<std::pair<btree_node**, size_t>> _path;
        size_t _index;

    public:
        using value_type = tree_data_type_const;
        using reference = value_type&;
        using pointer = value_type*;
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = btree_iterator;

        friend class B_tree;
        friend class btree_reverse_iterator;
        friend class btree_const_iterator;
        friend class btree_const_reverse_iterator;

        reference operator*() const noexcept;
        pointer operator->() const noexcept;

        self& operator++();
        self operator++(int);

        self& operator--();
        self operator--(int);

        bool operator==(const self& other) const noexcept;
        bool operator!=(const self& other) const noexcept;

        size_t depth() const noexcept;
        size_t current_node_keys_count() const noexcept;
        bool is_terminate_node() const noexcept;
        size_t index() const noexcept;
        void print_iterator() const noexcept;


        explicit btree_iterator(const std::stack<std::pair<btree_node**, size_t>>& path = std::stack<std::pair<btree_node**, size_t>>(), size_t index = 0);

    };

    class btree_const_iterator final
    {
        std::stack<std::pair<btree_node* const*, size_t>> _path;
        size_t _index;

    public:

        using value_type = tree_data_type_const;
        using reference = const value_type&;
        using pointer = const value_type*;
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = btree_const_iterator;

        friend class B_tree;
        friend class btree_reverse_iterator;
        friend class btree_iterator;
        friend class btree_const_reverse_iterator;

        btree_const_iterator(const btree_iterator& it) noexcept;

        reference operator*() const noexcept;
        pointer operator->() const noexcept;

        self& operator++();
        self operator++(int);

        self& operator--();
        self operator--(int);

        bool operator==(const self& other) const noexcept;
        bool operator!=(const self& other) const noexcept;

        size_t depth() const noexcept;
        size_t current_node_keys_count() const noexcept;
        bool is_terminate_node() const noexcept;
        size_t index() const noexcept;
        void print_iterator() const noexcept;

        explicit btree_const_iterator(const std::stack<std::pair<btree_node* const*, size_t>>& path = std::stack<std::pair<btree_node* const*, size_t>>(), size_t index = 0);
    };

    class btree_reverse_iterator final
    {
        std::stack<std::pair<btree_node**, size_t>> _path;
        size_t _index;

    public:

        using value_type = tree_data_type_const;
        using reference = value_type&;
        using pointer = value_type*;
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = btree_reverse_iterator;

        friend class B_tree;
        friend class btree_iterator;
        friend class btree_const_iterator;
        friend class btree_const_reverse_iterator;

        btree_reverse_iterator(const btree_iterator& it) noexcept;
        operator btree_iterator() const noexcept;

        reference operator*() const noexcept;
        pointer operator->() const noexcept;

        self& operator++();
        self operator++(int);

        self& operator--();
        self operator--(int);

        bool operator==(const self& other) const noexcept;
        bool operator!=(const self& other) const noexcept;

        size_t depth() const noexcept;
        size_t current_node_keys_count() const noexcept;
        bool is_terminate_node() const noexcept;
        size_t index() const noexcept;

        explicit btree_reverse_iterator(const std::stack<std::pair<btree_node**, size_t>>& path = std::stack<std::pair<btree_node**, size_t>>(), size_t index = 0);
    };

    class btree_const_reverse_iterator final
    {
        std::stack<std::pair<btree_node* const*, size_t>> _path;
        size_t _index;

    public:

        using value_type = tree_data_type_const;
        using reference = const value_type&;
        using pointer = const value_type*;
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = btree_const_reverse_iterator;

        friend class B_tree;
        friend class btree_reverse_iterator;
        friend class btree_const_iterator;
        friend class btree_iterator;

        btree_const_reverse_iterator(const btree_reverse_iterator& it) noexcept;
        operator btree_const_iterator() const noexcept;

        reference operator*() const noexcept;
        pointer operator->() const noexcept;

        self& operator++();
        self operator++(int);

        self& operator--();
        self operator--(int);

        bool operator==(const self& other) const noexcept;
        bool operator!=(const self& other) const noexcept;

        size_t depth() const noexcept;
        size_t current_node_keys_count() const noexcept;
        bool is_terminate_node() const noexcept;
        size_t index() const noexcept;

        explicit btree_const_reverse_iterator(const std::stack<std::pair<btree_node* const*, size_t>>& path = std::stack<std::pair<btree_node* const*, size_t>>(), size_t index = 0);
    };

    friend class btree_iterator;
    friend class btree_const_iterator;
    friend class btree_reverse_iterator;
    friend class btree_const_reverse_iterator;

    // endregion iterators declaration

    // region element access declaration

    /*
     * Returns a reference to the mapped value of the element with specified key. If no such element exists, an exception of type std::out_of_range is thrown.
     */
    tvalue& at(const tkey&);
    const tvalue& at(const tkey&) const;

    /*
     * If key not exists, makes default initialization of value
     */
    tvalue& operator[](const tkey& key);
    tvalue& operator[](tkey&& key);

    // endregion element access declaration
    // region iterator begins declaration

    btree_iterator begin();
    btree_iterator end();

    btree_const_iterator begin() const;
    btree_const_iterator end() const;

    btree_const_iterator cbegin() const;
    btree_const_iterator cend() const;

    btree_reverse_iterator rbegin();
    btree_reverse_iterator rend();

    btree_const_reverse_iterator rbegin() const;
    btree_const_reverse_iterator rend() const;

    btree_const_reverse_iterator crbegin() const;
    btree_const_reverse_iterator crend() const;

    // endregion iterator begins declaration

    // region lookup declaration

    size_t size() const noexcept;
    bool empty() const noexcept;

    /*
     * Returns end() if not exist
     */

    btree_iterator find(const tkey& key);
    btree_const_iterator find(const tkey& key) const;

    btree_iterator lower_bound(const tkey& key);
    btree_const_iterator lower_bound(const tkey& key) const;

    btree_iterator upper_bound(const tkey& key);
    btree_const_iterator upper_bound(const tkey& key) const;

    bool contains(const tkey& key) const;

    // endregion lookup declaration

    // region modifiers declaration

    void clear() noexcept;
    void delete_subtree(btree_node * node);

    void split_node(btree_node * node, size_t i);
    void split_root();
    /*
     * Does nothing if key exists, delegates to emplace.
     * Second return value is true, when inserted
     */
    std::pair<btree_iterator, bool> insert(const tree_data_type& data);
    std::pair<btree_iterator, bool> insert(tree_data_type&& data);

    template <typename ...Args>
    std::pair<btree_iterator, bool> emplace(Args&&... args);

    /*
     * Updates value if key exists, delegates to emplace.
     */
    btree_iterator insert_or_assign(const tree_data_type& data);
    btree_iterator insert_or_assign(tree_data_type&& data);

    template <typename ...Args>
    btree_iterator emplace_or_assign(Args&&... args);

    /*
     * Return iterator to node next ro removed or end() if key not exists
     */

    void remove_from_leaf(std::stack<std::pair<btree_node **, size_t>>& path, size_t idx);
    void borrow_from_left(btree_node* parent, size_t child_idx);
    void borrow_from_right(btree_node* parent, size_t child_idx);
    void merge_nodes(btree_node* parent, size_t left_idx);

    btree_iterator erase(btree_iterator pos);
    btree_iterator erase(btree_const_iterator pos);

    btree_iterator erase(btree_iterator beg, btree_iterator en);
    btree_iterator erase(btree_const_iterator beg, btree_const_iterator en);


    btree_iterator erase(const tkey& key);

    void print_structure(std::ostream& os = std::cout) const;


    // endregion modifiers declaration
};

template<std::input_iterator iterator, comparator<typename std::iterator_traits<iterator>::value_type::first_type> compare = std::less<typename std::iterator_traits<iterator>::value_type::first_type>,
        std::size_t t = 5, typename U>
B_tree(iterator begin, iterator end, const compare &cmp = compare(), pp_allocator<U> = pp_allocator<U>()) -> B_tree<typename std::iterator_traits<iterator>::value_type::first_type, typename std::iterator_traits<iterator>::value_type::second_type, compare, t>;

template<typename tkey, typename tvalue, comparator<tkey> compare = std::less<tkey>, std::size_t t = 5, typename U>
B_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare &cmp = compare(), pp_allocator<U> = pp_allocator<U>()) -> B_tree<tkey, tvalue, compare, t>;

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::compare_pairs(const B_tree::tree_data_type &lhs,
                                                     const B_tree::tree_data_type &rhs) const
{
    return compare::operator()(lhs, rhs);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::compare_keys(const tkey &lhs, const tkey &rhs) const
{
    return compare::operator()(lhs, rhs);
}


template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_node::btree_node() noexcept {}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
pp_allocator<typename B_tree<tkey, tvalue, compare, t>::value_type> B_tree<tkey, tvalue, compare, t>::get_allocator() const noexcept
{
    return this->_allocator;
}

// region constructors implementation

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::B_tree(
        const compare& cmp,
        pp_allocator<value_type> alloc) : compare(cmp), _allocator(alloc), _root(nullptr), _size(0) {}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::B_tree(
        pp_allocator<value_type> alloc,
        const compare& comp) : compare(comp), _allocator(alloc), _root(nullptr), _size(0) {}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template<input_iterator_for_pair<tkey, tvalue> iterator>
B_tree<tkey, tvalue, compare, t>::B_tree(
        iterator begin,
        iterator end,
        const compare& cmp,
        pp_allocator<value_type> alloc) : compare(cmp), _allocator(alloc), _root(nullptr), _size(0)
{
    for (auto iter = begin; iter != end; ++iter) {
        insert(*iter);
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::B_tree(
        std::initializer_list<std::pair<tkey, tvalue>> data,
        const compare& cmp,
        pp_allocator<value_type> alloc) : compare(cmp), _allocator(alloc), _root(nullptr), _size(0)
{
    for (auto pair : data) {
        insert(pair);
    }
}

// endregion constructors implementation

// region five implementation

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::~B_tree() noexcept
{
    clear();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_node*
B_tree<tkey, tvalue, compare, t>::copy_subtree(const btree_node* node) {
    if (!node) {
        return nullptr;
    }
    btree_node * new_node = new btree_node();
    new_node->_keys = node->_keys;
    for (const auto& child : node->_pointers) {
        new_node->_pointers.push_back(copy_subtree(child));
    }
    return new_node;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::B_tree(const B_tree& other) : compare(other), _allocator(other._allocator), _root(nullptr), _size(other._size)
{
    if (other._root) {
        this->_root = copy_subtree(other._root);
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>& B_tree<tkey, tvalue, compare, t>::operator=(const B_tree& other)
{
    if (this != &other) {
        B_tree tmp(other);
        std::swap(this->_root, tmp._root);
        std::swap(this->_size, tmp._size);
        std::swap(this->_allocator, tmp._allocator);
    }
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::B_tree(B_tree&& other) noexcept : compare(std::move(other)), _allocator(std::move(other._allocator)), _root(other._root), _size(other._size)
{
    other._root = nullptr;
    other._size = 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>& B_tree<tkey, tvalue, compare, t>::operator=(B_tree&& other) noexcept
{
    if (this != &other) {
        clear();
        static_cast<compare&>(*this) = std::move(static_cast<compare&>(other));
        _allocator = std::move(other._allocator);
        this->_root = other._root;
        this->_size = other._size;

        other._root = nullptr;
        other._size = 0;
    }
    return *this;
}

// endregion five implementation

// region iterators implementation

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_iterator::btree_iterator(
        const std::stack<std::pair<btree_node**, size_t>>& path, size_t index) : _index(index), _path(path) {}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator::reference
B_tree<tkey, tvalue, compare, t>::btree_iterator::operator*() const noexcept
{
    auto node = *(this->_path.top().first);
    return reinterpret_cast<reference>(node->_keys[_index]);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator::pointer
B_tree<tkey, tvalue, compare, t>::btree_iterator::operator->() const noexcept
{
    auto node = *(this->_path.top().first);
    return reinterpret_cast<pointer>(&node->_keys[_index]);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator&
B_tree<tkey, tvalue, compare, t>::btree_iterator::operator++()
{
    btree_node * node = *this->_path.top().first;
    size_t index = this->_index;
    this->print_iterator();

    if (!node->_pointers.empty()) { // спускаемся к листу
        this->_path.push({&node->_pointers[index + 1], index + 1});
        node = node->_pointers[index + 1];
        while (!node->_pointers.empty()) {
            this->_path.push({&node->_pointers[0], 0});
            node = node->_pointers[0];
        }
        this->_index = 0;
        return *this;
    } else {
        if (index + 1 < node->_keys.size()) { // идем по всем элементам листа
            ++this->_index;
            this->print_iterator();
            return *this;
        }
    }

    while (!this->_path.empty()) { // поднримаемся
        size_t child_index = this->_path.top().second;
        this->_path.pop();
        if (this->_path.empty()) {
            break;
        }

        btree_node * parent = *this->_path.top().first;
        if (child_index < parent->_keys.size()) {
            this->_index = child_index;
            this->print_iterator();
            return *this;
        }
    }
    this->_path = {};
    this->_index = 0;
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::btree_iterator::operator++(int)
{
    self tmp = *this;
    ++(*this);
    return tmp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator&
B_tree<tkey, tvalue, compare, t>::btree_iterator::operator--()
{
    if (this->_path.empty()) {
        return *this;
    }
    btree_node * node = *this->_path.top().first;
    size_t index = this->_index;
    this->print_iterator();

    if (!node->_pointers.empty()) {
        this->_path.push({&node->_pointers[index], index});
        node = node->_pointers[index];
        while (!node->_pointers.empty()) {
            this->_path.push({&node->_pointers[node->_pointers.size() - 1], node->_pointers.size() - 1});
            node = node->_pointers[node->_pointers.size() - 1];
        }
        this->_index = node->_keys.size() - 1;
        return *this;
    } 
    if (index > 0) {
        --this->_index;
        return *this;
    }

    while (!this->_path.empty()) {
        size_t child_index = this->_path.top().second;
        this->_path.pop();
        if (this->_path.empty()) {
            break;
        }

        btree_node * parent = *this->_path.top().first;
        if (child_index > 0) {
            this->_index = child_index - 1;
            this->print_iterator();
            return *this;
        }
    }
    this->_path = {};
    this->_index = 0;
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::btree_iterator::operator--(int)
{
    self tmp = *this;
    --(*this);
    return tmp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_iterator::operator==(const self& other) const noexcept
{
    return (this->_path == other._path && this->_index == other._index);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_iterator::operator!=(const self& other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_iterator::depth() const noexcept
{
    return this->_path.size() - 1;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_iterator::current_node_keys_count() const noexcept
{
    btree_node * node = *(this->_path.top()).first;
    return node->_keys.size();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_iterator::is_terminate_node() const noexcept
{
    btree_node * node = *(this->_path.top()).first;
    return node->_pointers.empty();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_iterator::index() const noexcept
{
    return this->_index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::btree_const_iterator(
        const std::stack<std::pair<btree_node* const*, size_t>>& path, size_t index) : _path(path), _index(index) {}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::btree_const_iterator(
    const btree_iterator& it) noexcept {
    auto const_path = it._path; 
    std::vector<std::pair<btree_node* const*, size_t>> tmp;
    while (!const_path.empty()) {
        tmp.push_back(const_path.top());
        const_path.pop();
    }
    for (auto it_vec = tmp.rbegin(); it_vec != tmp.rend(); ++it_vec) {
        _path.push({const_cast<btree_node**>(it_vec->first), it_vec->second});
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator::reference
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator*() const noexcept
{
    auto node = *(this->_path.top().first);
    return reinterpret_cast<reference>(node->_keys[_index]);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator::pointer
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator->() const noexcept
{
    auto node = *(this->_path.top().first);
    return reinterpret_cast<pointer>(&node->_keys[_index]);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator&
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator++()
{
    const btree_node * node = *this->_path.top().first;
    size_t index = this->_index;
    this->print_iterator();

    if (!node->_pointers.empty()) {
        this->_path.push({&node->_pointers[index + 1], index + 1});
        node = node->_pointers[index + 1];
        while (!node->_pointers.empty()) {
            this->_path.push({&node->_pointers[0], 0});
            node = node->_pointers[0];
        }
        this->_index = 0;
        return *this;
    } else {
        if (index + 1 < node->_keys.size()) {
            ++this->_index;
            this->print_iterator();
            return *this;
        }
    }

    while (!this->_path.empty()) {
        size_t child_index = this->_path.top().second;
        this->_path.pop();
        if (this->_path.empty()) {
            break;
        }

        const btree_node * parent = *this->_path.top().first;
        if (child_index < parent->_keys.size()) {
            this->_index = child_index;
            this->print_iterator();
            return *this;
        }
    }
    this->_path = {};
    this->_index = 0;
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator++(int)
{
    self tmp = *this;
    ++(*this);
    return tmp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator&
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator--()
{
    if (this->_path.empty()) {
        return *this;
    }
    const btree_node * node = *this->_path.top().first;
    size_t index = this->_index;
    this->print_iterator();

    if (!node->_pointers.empty()) {
        this->_path.push({&node->_pointers[index], index});
        node = node->_pointers[index];
        while (!node->_pointers.empty()) {
            this->_path.push({&node->_pointers[node->_pointers.size() - 1], node->_pointers.size() - 1});
            node = node->_pointers[node->_pointers.size() - 1];
        }
        this->_index = node->_keys.size() - 1;
        return *this;
    } 
    if (index > 0) {
        --this->_index;
        return *this;
    }

    while (!this->_path.empty()) {
        size_t child_index = this->_path.top().second;
        this->_path.pop();
        if (this->_path.empty()) {
            break;
        }

        btree_node * parent = *this->_path.top().first;
        if (child_index > 0) {
            this->_index = child_index - 1;
            this->print_iterator();
            return *this;
        }
    }
    this->_path = {};
    this->_index = 0;
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator--(int)
{
   self tmp = *this;
   --(*this);
   return tmp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator==(const self& other) const noexcept
{
    return (this->_path == other._path && this->_index == other._index);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator!=(const self& other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_const_iterator::depth() const noexcept
{
    return this->_path.size() - 1;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_const_iterator::current_node_keys_count() const noexcept
{
    const btree_node * node = *(this->_path.top()).first;
    return node->_keys.size();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_const_iterator::is_terminate_node() const noexcept
{
    const btree_node * node = *(this->_path.top()).first;
    return (node->_pointers.empty());
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_const_iterator::index() const noexcept
{
    return this->_index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::btree_reverse_iterator(
        const std::stack<std::pair<btree_node**, size_t>>& path, size_t index) : _path(path), _index(index) {}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::btree_reverse_iterator(
        const btree_iterator& it) noexcept : _path(it._path), _index(it._index) {}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator B_tree<tkey, tvalue, compare, t>::btree_iterator() const noexcept
{
    return btree_iterator(this->_path, this->_index);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::reference
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator*() const noexcept
{
    auto node = *(this->_path.top().first);
    return reinterpret_cast<reference>(node->_keys[_index]);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::pointer
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator->() const noexcept
{
    auto node = *(this->_path.top().first);
    return reinterpret_cast<pointer>(&node->_keys[_index]);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator&
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator++()
{
    btree_iterator iter = static_cast<btree_iterator>(*this);
    --iter;
    *this = btree_reverse_iterator(iter);
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator++(int)
{
   self tmp = *this;
   ++(*this);
   return tmp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator&
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator--()
{
    btree_iterator iter = static_cast<btree_iterator>(*this);
    ++iter;
    *this = btree_reverse_iterator(iter);
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator--(int)
{
    self tmp = *this;
    --(*this);
    return tmp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator==(const self& other) const noexcept
{
    return (this->_path == other._path && this->_index == other._index);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator!=(const self& other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::depth() const noexcept
{
    return this->_path.size() - 1;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::current_node_keys_count() const noexcept
{
    btree_node * node = *(this->_path.top()).first;
    return node->_keys.size();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::is_terminate_node() const noexcept
{
    btree_node * node = *(this->_path.top()).first;
    return node->_pointers.empty();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::index() const noexcept
{
    return this->_index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::btree_const_reverse_iterator(
        const std::stack<std::pair<btree_node* const*, size_t>>& path, size_t index) : _path(path), _index(index) {}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::btree_const_reverse_iterator(
        const btree_reverse_iterator& it) noexcept : _path(it._path), _index(it._index) {}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator B_tree<tkey, tvalue, compare, t>::btree_const_iterator() const noexcept
{
    return btree_const_iterator(this->_path, this->_index);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::reference
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator*() const noexcept
{
    auto node = *(this->_path.top().first);
    return reinterpret_cast<reference>(node->_keys[_index]);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::pointer
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator->() const noexcept
{
    auto node = *(this->_path.top().first);
    return reinterpret_cast<pointer>(&node->_keys[_index]);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator&
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator++()
{
    btree_const_iterator iter = static_cast<btree_const_iterator>(*this);
    --iter;
    *this = btree_const_reverse_iterator(iter);
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator++(int)
{
    self tmp = *this;
    ++(*this);
    return tmp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator&
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator--()
{
    btree_const_iterator iter = static_cast<btree_const_iterator>(*this);
    ++iter;
    *this = btree_const_reverse_iterator(iter);
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator--(int)
{
    self tmp = *this;
    --(*this);
    return tmp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator==(const self& other) const noexcept
{
    return (this->_path == other._path && this->_index == other._index);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator!=(const self& other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::depth() const noexcept
{
    return this->_path.size() - 1; 
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::current_node_keys_count() const noexcept
{
    const btree_node * node = *(this->_path.top().first);
    return node->_keys.size();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::is_terminate_node() const noexcept
{
    const btree_node* node = *(this->_path.top().first);
    return node->_pointers.empty();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::index() const noexcept
{
    return this->_index;
}

// endregion iterators implementation

// region element access implementation

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue& B_tree<tkey, tvalue, compare, t>::at(const tkey& key)
{
    btree_iterator iter = this->find(key);
    if (iter == end()) {
        throw std::out_of_range("key not found");   
    }
    return iter->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
const tvalue& B_tree<tkey, tvalue, compare, t>::at(const tkey& key) const
{
    btree_const_iterator iter = this->find(key);
    if (iter == cend()) {
        throw std::out_of_range("key not found");
    }
    return iter->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue& B_tree<tkey, tvalue, compare, t>::operator[](const tkey& key)
{
    btree_iterator iter = this->find(key);
    if (iter == end()) {
        iter = insert(tree_data_type(key, tvalue())).first; 
    }
    return iter.second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue& B_tree<tkey, tvalue, compare, t>::operator[](tkey&& key)
{
    btree_const_iterator iter = this->find(key);
    if (iter == cend()) {
        iter = insert(tree_data_type(key, tvalue())).first;
    }
    return iter.second;
}

// endregion element access implementation

// region iterator begins implementation

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator B_tree<tkey, tvalue, compare, t>::begin()
{
    if (this->_root == nullptr) {
        return end();
    }

    btree_node * node = this->_root;
    std::stack<std::pair<btree_node**, size_t>> path;

    path.push({&this->_root, 0});

    while (!node->_pointers.empty() && node->_pointers[0] != nullptr) {
        path.push({&node->_pointers[0], 0});
        node = node->_pointers[0];
    }
    return btree_iterator(path, 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator B_tree<tkey, tvalue, compare, t>::end()
{
    return btree_iterator();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator B_tree<tkey, tvalue, compare, t>::begin() const
{
    if (this->_root == nullptr) {
        return cend();
    }
    const btree_node * node = this->_root;
    std::stack<std::pair<btree_node* const*, size_t>> path;
    path.push({&this->_root, 0});

    while (!node->_pointers.empty() && node->_pointers[0] != nullptr) {
        path.push({&node->_pointers[0], 0});
        node = node->_pointers[0];
    }
    return btree_const_iterator(path, 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator B_tree<tkey, tvalue, compare, t>::end() const
{
    return btree_const_iterator();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator B_tree<tkey, tvalue, compare, t>::cbegin() const
{
    return begin();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator B_tree<tkey, tvalue, compare, t>::cend() const
{
    return end();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator B_tree<tkey, tvalue, compare, t>::rbegin()
{
    if (this->_root == nullptr) {
        return rend();
    }

    btree_node * node = this->_root;
    std::stack<std::pair<btree_node**, size_t>> path;

    path.push({&this->_root, 0});

    while (!node->_pointers.empty() && node->_pointers[node->_pointers.size() - 1] != nullptr) {
        path.push({node->_pointers[node->_pointers.size() - 1], node->_pointers.size() - 1});
        node = node->_pointers[node->_pointers.size() - 1];
    }
    return btree_reverse_iterator(path, node->_pointers.size() - 1);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator B_tree<tkey, tvalue, compare, t>::rend()
{
    return btree_reverse_iterator();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator B_tree<tkey, tvalue, compare, t>::rbegin() const
{
    if (this->_root == nullptr) {
        return rend();
    }

    const btree_node * node = this->_root;
    std::stack<std::pair<btree_node**, size_t>> path;

    path.push({&this->_root, 0});

    while (!node->_pointers.empty() && node->_pointers[node->_pointers.size() - 1] != nullptr) {
        path.push({node->_pointers[node->_pointers.size() - 1], node->_pointers.size() - 1});
        node = node->_pointers[node->_pointers.size() - 1];
    }
    return btree_const_reverse_iterator(path, node->_pointers.size() - 2);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator B_tree<tkey, tvalue, compare, t>::rend() const
{
    return btree_const_reverse_iterator();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator B_tree<tkey, tvalue, compare, t>::crbegin() const
{
    rbegin();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator B_tree<tkey, tvalue, compare, t>::crend() const
{
    rend();
}

// endregion iterator begins implementation

// region lookup implementation

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::size() const noexcept
{
    return this->_size;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::empty() const noexcept
{
    return this->_size == 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator B_tree<tkey, tvalue, compare, t>::find(const tkey& key)
{
    if (this->_root == nullptr) {
        return end();
    }

    std::stack<std::pair<btree_node**, size_t>> path;
    path.push({&this->_root, 0});

    btree_node * node = this->_root; // начинаем поиск с корня
    while (node != nullptr) {

        size_t i = 0;
        while (i < node->_keys.size() && compare_keys(node->_keys[i].first, key)) { // шлепаем по узлу и ищем нужный промежуток
            ++i;
        }

        if (i < node->_keys.size() && !compare_keys(key, node->_keys[i].first)) { // граница промежутка подошла - узел найден
            return btree_iterator(path, i);
        }

        if (node->_pointers.empty() || node->_pointers[i] == nullptr) { // мы ничо не нашли и мы в листе => финита ля комедия
            return end();
        }

        path.push({&node->_pointers[i], i}); 
        node = node->_pointers[i]; // продолжаем поиск, идем к детям (I + 1 МБ??)
    }
    return end();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator B_tree<tkey, tvalue, compare, t>::find(const tkey& key) const
{
    if (this->_root == nullptr) {
        return cend();
    }

    std::stack<std::pair<btree_node**, size_t>> path;
    path.push({&_root, 0});

    const btree_node * node = this->_root;

    while (node != nullptr) {
        size_t i = 0;
        while (i < node->_keys.size() && compare_keys(node->_keys[i].first, key)) {
            ++i;
        }

        if (i < node->_keys.size() && !compare_keys(key, node->_keys[i].first)) { // мб убрать условие про i < size?
            return btree_const_iterator({path, i});
        }
        if (node->_pointers.empty() || node->_pointers[i] == nullptr) {
            return cend();
        }
        path.push({&node->_pointers[i], i});
        node = node->_pointers[i];
    }
    return cend();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator B_tree<tkey, tvalue, compare, t>::lower_bound(const tkey& key)
{
    if (this->_root == nullptr) {
        return end();
    }
    std::stack<std::pair<btree_node**, size_t>> path;
    btree_node * node = this->_root;
    path.push({&this->_root, 0});

    while (node != nullptr) {
        size_t i = 0;
        while (i < node->_keys.size() && !compare_keys(key, node->_keys[i].first)) {
            ++i;
        }
        if (i < node->_keys.size()) {
            if (!node->_pointers.empty() && node->_pointers[i] != nullptr) {
                path.push({&node->_pointers[i], i});
                node = node->_pointers[i];
            } else {
                return btree_iterator(path, i);
            }
        } else {
            if (!node->_pointers.empty() && node->_pointers[i] != nullptr) {
                path.push({&node->_pointers[i], i});
                node = node->_pointers[i];
            } else {
                return end();
            }
        }
    }
    return end();
}


template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator B_tree<tkey, tvalue, compare, t>::lower_bound(const tkey& key) const
{
    if (this->_root == nullptr) {
        return cend();
    }

    std::stack<std::pair<btree_node* const*, size_t>> path;
    const btree_node * node = this->_root;
    path.push({&this->_root, 0});

    while (node != nullptr) {
        size_t i = 0;
        while (i < node->_keys.size() && !compare_keys(key, node->_keys[i].first)) {
            ++i;
        }
        if (i < node->_keys.size()) {
            if (!node->_pointers.empty() && node->_pointers != nullptr) {
                path.push({node->_pointers[i], i});
                node = node->_pointers[i];
            } else {
                return btree_const_iterator(path, i);
            }
        } else {
            if (!node->_pointers.empty() && node->_pointers != nullptr) {
                path.push({node->_pointers[i], i});
                node = node->_pointers[i];
            } else {
                return cend();
            }
        }
    }
    return cend();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator B_tree<tkey, tvalue, compare, t>::upper_bound(const tkey& key)
{
    if (this->_root == nullptr) {
        return end();
    }

    std::stack<std::pair<btree_node**, size_t>> path;
    btree_node * node = this->_root;
    path.push({&this->_root, 0});

    while (node != nullptr) {
        size_t i = 0;
        while (i < node->_keys.size() && compare_keys(node->_keys[i].first, key)) {
            ++i;
        }
        if (i < node->_keys.size()) {
            if (!node->_pointers.empty() && node->_pointers[i] != nullptr) {
                path.push({&node->_pointers[i], i});
                node = node->_pointers[i];
            } else {
                return btree_iterator(path, i);
            }
        } else {
            if (!node->_pointers.empty() && node->_pointers[i] != nullptr) {
                path.push({&node->_pointers[i], i});
                node = node->_pointers[i];
            } else {
                return end();
            }
        }
    }
    return end();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator B_tree<tkey, tvalue, compare, t>::upper_bound(const tkey& key) const
{
    if (this->_root == nullptr) {
        return cend();
    }

    std::stack<std::pair<btree_node* const*, size_t>> path;
    const btree_node * node = this->_root;
    path.push({&this->_root, 0});

    while (node != nullptr) {
        size_t i = 0;
        while (i < node->_keys.size() && compare_keys(node->_keys[i].first, key)) {
            ++i;
        }
        if (i < node->_keys.size()) {
            if (!node->_pointers.empty() && node->_pointers[i] != nullptr) {
                path.push({&node->_pointers[i], i});
                node = node->_pointers[i];
            } else {
                return btree_const_iterator(path, i);
            }
        } else {
            if (!node->_pointers.empty() && node->_pointers[i] != nullptr) {
                path.push({&node->_pointers[i], i});
                node = node->_pointers[i];
            } else {
                return cend();
            }
        }
    }
    return cend();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::contains(const tkey& key) const
{
    return find(key) != end();
}

// endregion lookup implementation

// region modifiers implementation

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void B_tree<tkey, tvalue, compare, t>::clear() noexcept
{
    this->delete_subtree(this->_root);
    this->_root = nullptr;
    this->_size = 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void B_tree<tkey, tvalue, compare, t>::delete_subtree(btree_node * node) {
    if (node == nullptr) {return;}
    for (btree_node * child : node->_pointers) {
        delete_subtree(child);
    }
    delete node;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void B_tree<tkey, tvalue, compare, t>::split_node(btree_node * parent, size_t i) {
    btree_node * child = parent->_pointers[i]; // узел, который мы делим
    btree_node * new_child = new btree_node();

    size_t new_parent_elem_index = t - 1;
    auto new_parent_elem_value = child->_keys[new_parent_elem_index];

    for (int j = t; j < child->_keys.size(); ++j) {
        new_child->_keys.push_back(child->_keys[j]);
    }
    for (int j = t; j < child->_pointers.size(); ++j) {
        new_child->_pointers.push_back(child->_pointers[j]); // мб так не сработает хз
    }

    child->_keys.resize(new_parent_elem_index);

    if (!child->_pointers.empty()) {
        child->_pointers.resize(new_parent_elem_index + 1);
    }

    parent->_keys.insert(parent->_keys.begin() + i, new_parent_elem_value);
    parent->_pointers.insert(parent->_pointers.begin() + i + 1, new_child);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void B_tree<tkey, tvalue, compare, t>::split_root() {
    // можно было бы выделить память для двух потомков и нового корня явно, но это требует бОльших затрат, чем такой способ
    // тут второй дитенок формируется "автоматически" из остатка корня

    btree_node * new_root = new btree_node();
    btree_node * old_root = this->_root;
    btree_node * child = new btree_node();

    size_t new_root_index = t;
    auto new_root_value = old_root->_keys[new_root_index];

    for (size_t i = t + 1; i < old_root->_keys.size(); ++i) { // берем вторую половину, потому что resize в векторе усекает конец, значит его и надо сохранить
        child->_keys.push_back(old_root->_keys[i]);
    }
    
    for (size_t i = t + 1; i < old_root->_pointers.size(); ++i) {
        child->_pointers.push_back(old_root->_pointers[i]);
    }

    old_root->_keys.resize(new_root_index);
    if (!old_root->_pointers.empty()) {
        old_root->_pointers.resize(new_root_index + 1);
    }

    new_root->_keys.push_back(new_root_value);
    new_root->_pointers.push_back(old_root);
    new_root->_pointers.push_back(child);
    this->_root = new_root;
}


template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
std::pair<typename B_tree<tkey, tvalue, compare, t>::btree_iterator, bool>
B_tree<tkey, tvalue, compare, t>::insert(const tree_data_type& data)
{
    std::stack<std::pair<btree_node**, size_t>> path;
    
    if (this->_root == nullptr) {
        btree_node * node = new btree_node(); 
        node->_keys.push_back(data);
        this->_root = node;
        this->_size++;
        path.push({&this->_root, 0});
        this->print_structure();
        return {btree_iterator{path, 0}, true};
    }
     
    if (this->_root->_keys.size() == maximum_keys_in_node) { // нужен сплит
        split_root();
    }
    path.push({&this->_root, 0});
    btree_node * node = this->_root;
    while (node != nullptr) {
        size_t i = 0;
        while (i < node->_keys.size() && compare_keys(node->_keys[i].first, data.first)) { // ищем первый элемент > key
            ++i;
        }
    
        if (i < node->_keys.size() && !compare_keys(node->_keys[i].first, data.first) && !compare_keys(data.first, node->_keys[i].first)) { // равны, ключ уже есть
            return {btree_iterator{path, i}, false};
        }
    
        if (node->_pointers.empty() || node->_pointers[i] == nullptr) { // нашли лист для вставки. вставляем очев
            node->_keys.insert(node->_keys.begin() + i, data);
            ++this->_size;
            this->print_structure();
            return {btree_iterator{path, i}, true};
        } else {
            btree_node * child = node->_pointers[i];
            if (child->_keys.size() == maximum_keys_in_node) {
                split_node(node, i);
                i = 0;
                while (i < node->_keys.size() && compare_keys(node->_keys[i].first, data.first)) {
                    ++i;
                }
                if (i < node->_keys.size() && !compare_keys(node->_keys[i].first, data.first) && !compare_keys(data.first, node->_keys[i].first)) {
                    return {btree_iterator{path, i}, false};
                }
            }
            size_t child_index;
            if (i < node->_keys.size() && compare_keys(data.first, node->_keys[i].first)) {
                child_index = i;
            } else if (i < node->_keys.size()) {
                child_index = i + 1;
            } else {
                child_index = i;
            }
            path.push({&node->_pointers[child_index], child_index});
            node = node->_pointers[child_index]; 
        }
    }
    return {end(), 0};
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
std::pair<typename B_tree<tkey, tvalue, compare, t>::btree_iterator, bool>
B_tree<tkey, tvalue, compare, t>::insert(tree_data_type&& data)
{
    this->print_structure();
    std::stack<std::pair<btree_node**, size_t>> path;
    
    if (this->_root == nullptr) {
        btree_node * node = new btree_node(); 
        node->_keys.push_back(std::move(data));
        this->_root = node;
        this->_size++;
        path.push({&this->_root, 0});
        this->print_structure();
        return {btree_iterator{path, 0}, true};
    }
     
    if (this->_root->_keys.size() == maximum_keys_in_node) { // нужен сплит
        split_root();
    }
    path.push({&this->_root, 0});
    btree_node * node = this->_root;
    while (node != nullptr) {
        size_t i = 0;
        while (i < node->_keys.size() && compare_keys(node->_keys[i].first, data.first)) { // ищем первый элемент > key
            ++i;
        }
    
        if (i < node->_keys.size() && !compare_keys(node->_keys[i].first, data.first) && !compare_keys(data.first, node->_keys[i].first)) { // равны, ключ уже есть
            return {btree_iterator{path, i}, false};
        }
    
        if (node->_pointers.empty() || node->_pointers[i] == nullptr) { // нашли лист для вставки. вставляем очев
            node->_keys.insert(node->_keys.begin() + i, std::move(data));
            ++this->_size;
            this->print_structure();
            return {btree_iterator{path, i}, true};
        } else {
            btree_node * child = node->_pointers[i];
            if (child->_keys.size() == maximum_keys_in_node) {
                split_node(node, i);
                i = 0;
                while (i < node->_keys.size() && compare_keys(node->_keys[i].first, data.first)) {
                    ++i;
                }
                if (i < node->_keys.size() && !compare_keys(node->_keys[i].first, data.first) && !compare_keys(data.first, node->_keys[i].first)) {
                    return {btree_iterator{path, i}, false};
                }
            }
            size_t child_index;
            if (i < node->_keys.size() && compare_keys(data.first, node->_keys[i].first)) {
                child_index = i;
            } else if (i < node->_keys.size()) {
                child_index = i + 1;
            } else {
                child_index = i;
            }
            path.push({&node->_pointers[child_index], child_index});
            node = node->_pointers[child_index]; 
        }
    }
    return {end(), 0};
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template<typename... Args>
std::pair<typename B_tree<tkey, tvalue, compare, t>::btree_iterator, bool>
B_tree<tkey, tvalue, compare, t>::emplace(Args&&... args)
{
    return insert(tree_data_type(std::forward<Args>(args)...));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::insert_or_assign(const tree_data_type& data)
{
    auto iter = find(data.first);
    if (iter != end()) {
        iter->second = data.second;
        return iter;
    } 
    return insert(data).first;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::insert_or_assign(tree_data_type&& data)
{
    auto iter = find(data.first);
    if (iter != end()) {
        iter.second = std::move(data.second);
        return iter;
    }
    return insert(std::move(data)).first;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template<typename... Args>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::emplace_or_assign(Args&&... args)
{
    tree_data_type data(std::forward<Args>(args)...);
    auto iter = find(data.first);
    if (iter != end()) {
        iter.second = std::move(data.second);
        return iter;
    }
    return emplace(std::move(data)).first;

}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::erase(btree_iterator pos)
{
    if (pos == end()) {
        return end();
    }
    auto path = pos._path;
    size_t index = pos._index;

    btree_node * node = *path.top().first;
    tkey key_to_remove = node->_keys[index].first;

    if (node->_pointers.empty()) {
        remove_from_leaf(path, index);
    } else {
        auto left_child = node->_pointers[index];
        auto left_path = path;
        left_path.push({&node->_pointers[index], index});
        btree_node * cur = left_child;
        while (!cur->_pointers.empty()) {
            left_path.push({&cur->_pointers[cur->_pointers.size() - 1], cur->_pointers.size() - 1});
            cur = cur->_pointers[cur->_pointers.size() - 1];
        }

        size_t last_index = cur->_keys.size() - 1;
        node->_keys[index] = cur->_keys[last_index];
        remove_from_leaf(left_path, last_index);
    }
    return find(key_to_remove);

}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void B_tree<tkey, tvalue, compare, t>::remove_from_leaf(std::stack<std::pair<btree_node **, size_t>>& path, size_t index) {

    btree_node * node = *path.top().first;
    size_t parent_child_index = path.top().second;


    node->_keys.erase(node->_keys.begin() + index);
    --this->_size;

    while (path.size() > 1 && node->_keys.size() < minimum_keys_in_node) {
        path.pop();

        btree_node * parent = *path.top().first;
        size_t child_index = path.top().second;


        if (child_index > 0) {
            btree_node * left_brat = parent->_pointers[child_index - 1];
            if (left_brat->_keys.size() > minimum_keys_in_node) {
                borrow_from_left(parent, child_index);
                return;
            }
        }

        if (child_index + 1 < parent->_pointers.size()) {
            btree_node * right_brat = parent->_pointers[child_index + 1];
            if (right_brat->_keys.size() > minimum_keys_in_node) {
                borrow_from_right(parent, child_index);
                return;
            }
        }

        if (child_index > 0) {
            merge_nodes(parent, child_index - 1);
            node = parent;
        } else {
            merge_nodes(parent, child_index);
            node = parent;
        }
    }
    if (this->_root->_keys.empty() && !this->_root->_pointers.empty()) {
        btree_node * old_root = this->_root;
        this->_root = this->_root->_pointers[0];
        delete old_root;
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void B_tree<tkey, tvalue, compare, t>::borrow_from_left(btree_node* parent, size_t child_index) {
    btree_node * left = parent->_pointers[child_index - 1];
    btree_node * node = parent->_pointers[child_index];

    node->_keys.insert(node->_keys.begin(), parent->_keys[child_index - 1]);
    parent->_keys[child_index - 1] = left->_keys.back();
    left->_keys.pop_back();

    if (!left->_pointers.empty()) {
        node->_pointers.insert(node->_pointers.begin(), left->_pointers.back());
        left->_pointers.pop_back();
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void B_tree<tkey, tvalue, compare, t>::borrow_from_right(btree_node* parent, size_t child_index) {
    btree_node * node = parent->_pointers[child_index];
    btree_node * right = parent->_pointers[child_index + 1];

    node->_keys.push_back(parent->_keys[child_index]);

    parent->_keys[child_index] = right->_keys.front();
    right->_keys.erase(right->_keys.begin());

    if (!right->_pointers.empty()) {
        node->_pointers.push_back(right->_pointers.front());
        right->_pointers.erase(right->_pointers.begin());
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void B_tree<tkey, tvalue, compare, t>::merge_nodes(btree_node* parent, size_t left_index) {
    btree_node * left = parent->_pointers[left_index];
    btree_node * right = parent->_pointers[left_index + 1];

    left->_keys.push_back(parent->_keys[left_index]);
    for (auto& child : right->_pointers) {
        left->_pointers.push_back(child);
    }

    for (auto& data : right->_keys) {
        left->_keys.push_back(data);
    }
    delete right;

    parent->_keys.erase(parent->_keys.begin() + left_index);
    parent->_pointers.erase(parent->_pointers.begin() + left_index + 1);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::erase(btree_const_iterator pos)
{
   return erase(btree_iterator(pos));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::erase(btree_iterator beg, btree_iterator en)
{
    while (beg != en) {
        beg = erase(beg);
    }
    return en;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::erase(btree_const_iterator beg, btree_const_iterator en)
{
    return erase(btree_iterator(beg), btree_iterator(en));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::erase(const tkey& key)
{
    auto iter = find(key);
    if (iter == end()) {
        return end();
    }
    return erase(iter);
}

// endregion modifiers implementation

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool compare_pairs(const typename B_tree<tkey, tvalue, compare, t>::tree_data_type &lhs,
                   const typename B_tree<tkey, tvalue, compare, t>::tree_data_type &rhs)
{
    return compare::operator()(lhs, rhs);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool compare_keys(const tkey &lhs, const tkey &rhs)
{
    return compare::operator()(lhs, rhs);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void B_tree<tkey, tvalue, compare, t>::print_structure(std::ostream& os) const {
    if (_root == nullptr) {
        os << "Empty tree\n";
        return;
    }
    print_node(os, _root, 0, "", true);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void B_tree<tkey, tvalue, compare, t>::print_node(std::ostream& os, const btree_node* node, int depth, const std::string& indent, bool last) {
    // Рисуем отступ и префикс (└── или ├──)
    os << indent;
    if (depth > 0) {
        os << (last ? "└── " : "├── ");
    }

    // Выводим ключи узла
    os << "[";
    for (size_t i = 0; i < node->_keys.size(); ++i) {
        os << node->_keys[i].first;
        if (i + 1 < node->_keys.size()) os << ", ";
    }
    os << "]";

    // Если лист, помечаем
    if (node->_pointers.empty()) os << " (leaf)";

    // Для информации выводим глубину
    os << "  depth=" << depth << "\n";

    // Если есть дети, рекурсивно обходим их
    if (!node->_pointers.empty()) {
        // Строим новый отступ для детей
        std::string new_indent = indent;
        if (depth > 0) {
            // Если родитель был последним, то добавляем пробелы, иначе вертикальную линию
            new_indent += last ? "    " : "│   ";
        } else {
            // Для корня просто добавляем отступ
            new_indent += "    ";
        }

        for (size_t i = 0; i < node->_pointers.size(); ++i) {
            bool last_child = (i == node->_pointers.size() - 1);
            print_node(os, node->_pointers[i], depth + 1, new_indent, last_child);
        }
    }
}


template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void B_tree<tkey, tvalue, compare, t>::btree_const_iterator::print_iterator() const noexcept {
    std::cout << "const_iterator: depth=" << depth()
              << ", index=" << index()
              << ", current=(" << (*this)->first << ", " << (*this)->second << ")"
              << ", stack_size=" << (depth() + 1)
              << std::endl;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void B_tree<tkey, tvalue, compare, t>::btree_iterator::print_iterator() const noexcept {
    std::cout << "const_iterator: depth=" << depth()
              << ", index=" << index()
              << ", current=(" << (*this)->first << ", " << (*this)->second << ")"
              << ", stack_size=" << (depth() + 1)
              << std::endl;
}

#endif
