#include <iterator>
#include <utility>
#include <vector>
#include <boost/container/static_vector.hpp>
#include <concepts>
#include <stack>
#include <pp_allocator.h>
#include <associative_container.h>
#include <initializer_list>
#include <not_implemented.h>

#ifndef SYS_PROG_BS_PLUS_TREE_H
#define SYS_PROG_BS_PLUS_TREE_H

template <typename tkey, typename tvalue, comparator<tkey> compare = std::less<tkey>, std::size_t t = 5>
class BSP_tree final : private compare
{
public:

    using tree_data_type = std::pair<tkey, tvalue>;
    using tree_data_type_const = std::pair<const tkey, tvalue>;
    using value_type = tree_data_type_const;

private:

    static constexpr const size_t minimum_keys_in_node = 2 * t - 1;
    static constexpr const size_t maximum_keys_in_root = 4 * t - 1;
    static constexpr const size_t maximum_keys_in_node = 3 * t - 1;


    // region comparators declaration

    inline bool compare_keys(const tkey& lhs, const tkey& rhs) const;
    inline bool compare_pairs(const tree_data_type& lhs, const tree_data_type& rhs) const;

    // endregion comparators declaration

    struct bsptree_node_base
    {
        bool _is_terminated;

        bsptree_node_base() noexcept;
        virtual ~bsptree_node_base() =default;
    };  

    struct bsptree_node_term : public bsptree_node_base
    {
        bsptree_node_term* _next;
        boost::container::static_vector<tree_data_type, maximum_keys_in_root + 1> _data;
        bsptree_node_term() noexcept;
    };

    struct bsptree_node_middle : public bsptree_node_base
    {
        boost::container::static_vector<tkey, maximum_keys_in_root + 1> _keys;
        boost::container::static_vector<bsptree_node_base*, maximum_keys_in_root + 2> _pointers;
        bsptree_node_middle() noexcept;
    };

    pp_allocator<value_type> _allocator;
    bsptree_node_base* _root;
    size_t _size;

    pp_allocator<value_type> get_allocator() const noexcept;

public:

    // region constructors declaration

    explicit BSP_tree(const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    explicit BSP_tree(pp_allocator<value_type> alloc, const compare& comp = compare());

    template<input_iterator_for_pair<tkey, tvalue> iterator>
    explicit BSP_tree(iterator begin, iterator end, const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    BSP_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    // endregion constructors declaration

    // region five declaration

    BSP_tree(const BSP_tree& other);

    BSP_tree(BSP_tree&& other) noexcept;

    BSP_tree& operator=(const BSP_tree& other);

    BSP_tree& operator=(BSP_tree&& other) noexcept;

    ~BSP_tree() noexcept;

    // endregion five declaration

    // region iterators declaration

    class bsptree_iterator;
    class bsptree_const_iterator;

    class bsptree_iterator final
    {
        bsptree_node_term* _node;
        size_t _index;

    public:
        using value_type = tree_data_type_const;
        using reference = value_type&;
        using pointer = value_type*;
        using iterator_category = std::forward_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = bsptree_iterator;

        friend class BSP_tree;
        friend class bsptree_const_iterator;

        reference operator*() const noexcept;
        pointer operator->() const noexcept;

        self& operator++();
        self operator++(int);

        bool operator==(const self& other) const noexcept;
        bool operator!=(const self& other) const noexcept;

        size_t current_node_keys_count() const noexcept;
        size_t index() const noexcept;

        explicit bsptree_iterator(bsptree_node_term* node = nullptr, size_t index = 0);
    };

    class bsptree_const_iterator final
    {
        const bsptree_node_term* _node;
        size_t _index;

    public:

        using value_type = tree_data_type_const;
        using reference = const value_type&;
        using pointer = const value_type*;
        using iterator_category = std::forward_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = bsptree_const_iterator;

        friend class BSP_tree;
        friend class bsptree_iterator;

        bsptree_const_iterator(const bsptree_iterator& it) noexcept;

        reference operator*() const noexcept;
        pointer operator->() const noexcept;

        self& operator++();
        self operator++(int);

        bool operator==(const self& other) const noexcept;
        bool operator!=(const self& other) const noexcept;

        size_t current_node_keys_count() const noexcept;
        size_t index() const noexcept;

        explicit bsptree_const_iterator(const bsptree_node_term* node = nullptr, size_t index = 0);
    };

    friend class btree_iterator;
    friend class btree_const_iterator;

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

    void print_iterator(const bsptree_iterator& it) const;
    void print_structure() const;

    // endregion element access declaration
    // region iterator begins declaration

    bsptree_iterator begin();
    bsptree_iterator end();

    bsptree_const_iterator begin() const;
    bsptree_const_iterator end() const;

    bsptree_const_iterator cbegin() const;
    bsptree_const_iterator cend() const;

    // endregion iterator begins declaration

    // region lookup declaration

    size_t size() const noexcept;
    bool empty() const noexcept;

    /*
     * Returns end() if not exist
     */

    bsptree_iterator find(const tkey& key);
    bsptree_const_iterator find(const tkey& key) const;

    bsptree_iterator lower_bound(const tkey& key);
    bsptree_const_iterator lower_bound(const tkey& key) const;

    bsptree_iterator upper_bound(const tkey& key);
    bsptree_const_iterator upper_bound(const tkey& key) const;

    bool contains(const tkey& key) const;

    // endregion lookup declaration

    // region modifiers declaration
    void delete_subtree(bsptree_node_base * node);
    void clear() noexcept;

    /*
     * Does nothing if key exists, delegates to emplace.
     * Second return value is true, when inserted
     */
    void split_root();
    void handle_leaf_overflow(std::stack<std::pair<bsptree_node_middle*, size_t>>& path, bsptree_node_term * leaf);
    void handle_inner_overflow(std::stack<std::pair<bsptree_node_middle*, size_t>>& path, bsptree_node_middle * node);

    bool try_redistribute_leaf(std::stack<std::pair<bsptree_node_middle*, size_t>>& path, bsptree_node_term * leaf);
    bool try_redistribute_inner(std::stack<std::pair<bsptree_node_middle*, size_t>>& path, bsptree_node_middle * node);

    void split_leaf_2_to_3(std::stack<std::pair<bsptree_node_middle*, size_t>>& path, bsptree_node_term * leaf);
    void split_inner_2_to_3(std::stack<std::pair<bsptree_node_middle*, size_t>>& path, bsptree_node_middle * node);


    std::pair<bsptree_iterator, bool> insert(const tree_data_type& data);
    std::pair<bsptree_iterator, bool> insert(tree_data_type&& data);

    template <typename ...Args>
    std::pair<bsptree_iterator, bool> emplace(Args&&... args);

    /*
     * Updates value if key exists, delegates to emplace.
     */
    bsptree_iterator insert_or_assign(const tree_data_type& data);
    bsptree_iterator insert_or_assign(tree_data_type&& data);

    template <typename ...Args>
    bsptree_iterator emplace_or_assign(Args&&... args);

    /*
     * Return iterator to node next ro removed or end() if key not exists
     */

    void handle_lack_of_keys_leaf(std::stack<std::pair<bsptree_node_middle*, size_t>>& path, bsptree_node_term * leaf);
    void handle_lack_of_keys_inner(std::stack<std::pair<bsptree_node_middle*, size_t>>& path, bsptree_node_middle * middle);

    bool try_borrow_leaf(std::stack<std::pair<bsptree_node_middle*, size_t>>& path, bsptree_node_term * leaf);
    bool try_borrow_inner(std::stack<std::pair<bsptree_node_middle*, size_t>>& path, bsptree_node_middle * node);

    void merge_leaf_3_to_2(std::stack<std::pair<bsptree_node_middle*, size_t>>& path, bsptree_node_term * leaf);
    void merge_inner_3_to_2(std::stack<std::pair<bsptree_node_middle*, size_t>>& path, bsptree_node_middle * node);



    bsptree_iterator erase(bsptree_iterator pos);
    bsptree_iterator erase(bsptree_const_iterator pos);

    bsptree_iterator erase(bsptree_iterator beg, bsptree_iterator en);
    bsptree_iterator erase(bsptree_const_iterator beg, bsptree_const_iterator en);


    bsptree_iterator erase(const tkey& key);

    // endregion modifiers declaration
};

template<std::input_iterator iterator, comparator<typename std::iterator_traits<iterator>::value_type::first_type> compare = std::less<typename std::iterator_traits<iterator>::value_type::first_type>,
        std::size_t t = 5, typename U>
BSP_tree(iterator begin, iterator end, const compare &cmp = compare(), pp_allocator<U> = pp_allocator<U>()) -> BSP_tree<typename std::iterator_traits<iterator>::value_type::first_type, typename std::iterator_traits<iterator>::value_type::second_type, compare, t>;

template<typename tkey, typename tvalue, comparator<tkey> compare = std::less<tkey>, std::size_t t = 5, typename U>
BSP_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare &cmp = compare(), pp_allocator<U> = pp_allocator<U>()) -> BSP_tree<tkey, tvalue, compare, t>;

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BSP_tree<tkey, tvalue, compare, t>::compare_pairs(const BSP_tree::tree_data_type &lhs,
                                                      const BSP_tree::tree_data_type &rhs) const
{
    return compare::operator()(lhs.first, rhs.first);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BSP_tree<tkey, tvalue, compare, t>::compare_keys(const tkey &lhs, const tkey &rhs) const
{
    return compare::operator()(lhs, rhs);
}

// region bsptree_node_base implementation

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::bsptree_node_base::bsptree_node_base() noexcept : _is_terminated(false) {}


template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::bsptree_node_term::bsptree_node_term() noexcept : bsptree_node_base(), _next(nullptr) {
    this->_is_terminated = true;

}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::bsptree_node_middle::bsptree_node_middle() noexcept {}
// region BSP_tree constructor implementations

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
pp_allocator<typename BSP_tree<tkey, tvalue, compare, t>::value_type> BSP_tree<tkey, tvalue, compare, t>::
get_allocator() const noexcept
{
    return this->_allocator;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::bsptree_const_iterator(const bsptree_node_term *node,
    size_t index) : _node(node), _index(index) {}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::BSP_tree(const compare& cmp, pp_allocator<value_type> alloc) : compare(cmp), _allocator(alloc), _root(nullptr), _size(0) {}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::BSP_tree(pp_allocator<value_type> alloc, const compare& cmp) : BSP_tree(cmp, alloc) {}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template<input_iterator_for_pair<tkey, tvalue> iterator>
BSP_tree<tkey, tvalue, compare, t>::BSP_tree(iterator begin, iterator end, const compare& cmp, pp_allocator<value_type> alloc) : compare(cmp), _allocator(alloc), _root(nullptr), _size(0)
{
    for (auto it = begin(); it != end(); ++it) {
        insert(*it);
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::BSP_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare& cmp, pp_allocator<value_type> alloc) : compare(cmp), _allocator(alloc), _root(nullptr), _size(0)
{
    for (const auto& p : data) {
        insert(p);
    }
}

// endregion BSP_tree constructor implementations

// region BSP_tree copy and move constructors

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::BSP_tree(const BSP_tree& other) : compare(other), _allocator(other._allocator), _root(nullptr), _size(0)
{
    for (auto it = other.cbegin(); it != other.cend(); ++it) {
        insert(*it);
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::BSP_tree(BSP_tree&& other) noexcept : compare(std::move(other)), _allocator(std::move(other._allocator)),
      _root(std::exchange(other._root, nullptr)), _size(std::exchange(other._size, 0)) {}

// endregion BSP_tree copy and move constructors

// region BSP_tree copy and move assignment operators

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>& BSP_tree<tkey, tvalue, compare, t>::operator=(const BSP_tree& other)
{
    if (this != &other) {
        BSP_tree tmp(other);
        std::swap(_root, tmp._root);
        std::swap(_size, tmp._size);
        std::swap(_allocator, tmp._allocator);
    }
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>& BSP_tree<tkey, tvalue, compare, t>::operator=(BSP_tree&& other) noexcept
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

// endregion BSP_tree copy and move assignment operators

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::~BSP_tree() noexcept
{
    clear();
}

// region BSP_tree iterators implementations

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::bsptree_iterator(bsptree_node_term* node, size_t index) : _node(node), _index(index) {}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::reference BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::operator*() const noexcept
{
    return reinterpret_cast<reference>(_node->_data[_index]);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::pointer BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::operator->() const noexcept
{
    return reinterpret_cast<pointer>(&_node->_data[_index]);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator& BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::operator++()
{
    ++this->_index;
    if (_index == _node->_data.size()) {
        if (_node->_next == nullptr) {
            *this = bsptree_iterator(nullptr, 0);  
        } else {
            _node = _node->_next;
            _index = 0;
        }
    }

    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::operator++(int)
{
    self temp = *this;
    ++*this;
    return temp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::operator==(const self& other) const noexcept
{
    return this->_node == other._node && this->_index == other._index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::operator!=(const self& other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::current_node_keys_count() const noexcept
{
    return this->_node->_data.size();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::index() const noexcept
{
    return this->_index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::bsptree_const_iterator(const bsptree_iterator& it) noexcept
{
    this->_index = it._index;
    this->_node = it._node;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::reference BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::operator*() const noexcept
{
    return reinterpret_cast<reference>(_node->_data[_index]);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::pointer BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::operator->() const noexcept
{
    return reinterpret_cast<pointer>(&_node->_data[_index]);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator& BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::operator++()
{
    ++this->_index;
    if (_index == _node->_data.size()) {
        if (_node->_next == nullptr) {
            *this = bsptree_iterator(nullptr, 0);  
        } else {
            _node = _node->_next;
            _index = 0;
        }
    }

    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::operator++(int)
{
    self temp = *this;
    ++*this;
    return temp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::operator==(const self& other) const noexcept
{
    return (this->_node == other._node && this->_index == other._index);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::operator!=(const self& other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::current_node_keys_count() const noexcept
{
    return this->_node->_data.size();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::index() const noexcept
{
    return this->_index;
}

// endregion BSP_tree iterators implementations

// region BSP_tree element access implementations

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue& BSP_tree<tkey, tvalue, compare, t>::at(const tkey& key)
{
    bsptree_iterator iter = this->find(key);
    if (iter == end()) {
        throw std::out_of_range("key not found");   
    }
    return iter->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
const tvalue& BSP_tree<tkey, tvalue, compare, t>::at(const tkey& key) const
{
    bsptree_const_iterator iter = this->find(key);
    if (iter == end()) {
        throw std::out_of_range("key not found");   
    }
    return iter->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue& BSP_tree<tkey, tvalue, compare, t>::operator[](const tkey& key)
{
    bsptree_iterator iter = this->find(key);
    if (iter == end()) {
        auto res = insert(tree_data_type(key, tvalue())); 
        iter = res.first;
    }
    return iter->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue& BSP_tree<tkey, tvalue, compare, t>::operator[](tkey&& key)
{
    bsptree_iterator iter = this->find(key);
    if (iter == end()) {
        auto res = insert(tree_data_type(std::move(key), tvalue())); 
        iter = res.first;
    }
    return iter->second;
}

// endregion BSP_tree element access implementations

// region BSP_tree iterator begins implementations

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::begin()
{
    if (_root == nullptr || _size == 0) {
        return end();
    }

    bsptree_node_base * node = this->_root;
    while(!node->_is_terminated) {
        auto * middle = static_cast<bsptree_node_middle*>(node);
        node = middle->_pointers[0];
    }

    auto leaf = static_cast<bsptree_node_term*>(node);
    return bsptree_iterator(leaf, 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::end()
{
    return bsptree_iterator(nullptr, 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator BSP_tree<tkey, tvalue, compare, t>::begin() const
{
    if (_root == nullptr || _size == 0) {
        return end();
    }

    bsptree_node_base * node = this->_root;
    while(!node->_is_terminated) {
        auto * middle = static_cast<bsptree_node_middle*>(node);
        node = middle->_pointers[0];
    }

    auto leaf = static_cast<bsptree_node_term*>(node);
    return bsptree_const_iterator(leaf, 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator BSP_tree<tkey, tvalue, compare, t>::end() const
{
    return bsptree_const_iterator(nullptr, 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator BSP_tree<tkey, tvalue, compare, t>::cbegin() const
{
    if (_root == nullptr || _size == 0) {
        return cend();
    }

    bsptree_node_base * node = this->_root;
    while(!node->_is_terminated) {
        auto * middle = static_cast<bsptree_node_middle*>(node);
        node = middle->_pointers[0];
    }

    auto leaf = static_cast<bsptree_node_term*>(node);
    return bsptree_const_iterator(leaf, 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator BSP_tree<tkey, tvalue, compare, t>::cend() const
{
    return bsptree_const_iterator(nullptr, 0);
}

// endregion BSP_tree iterator begins implementations

// region BSP_tree lookup implementations

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BSP_tree<tkey, tvalue, compare, t>::size() const noexcept
{
    return this->_size;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BSP_tree<tkey, tvalue, compare, t>::empty() const noexcept
{
    return this->_size == 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::find(const tkey& key)
{
    if (this->_root == nullptr) {
        return end();
    }
    bsptree_node_base * curr = this->_root;
    while (!curr->_is_terminated) {
        auto * node = static_cast<bsptree_node_middle*>(curr);
        size_t i = 0;
        while (i < node->_keys.size() && compare_keys(node->_keys[i], key)) {
            ++i;
        }
        curr = node->_pointers[i];
    }

    auto * NodeList = static_cast<bsptree_node_term*>(curr);
    // бинарный поиск нужного ключа в узле
    int left = 0;
    int right = NodeList->_data.size() - 1;

    while (left < right) {
        int mid = (left + right) / 2;
        bool less = compare_keys(key, NodeList->_data[mid].first);
        bool greater = compare_keys(NodeList->_data[mid].first, key);
        if (!greater && !less) {
            return bsptree_iterator(NodeList, mid);
        }
        if (less && !greater) {
            right = mid - 1;
        } else {
            left = mid + 1;
        }
    }
    if (!compare_keys(key, NodeList->_data[left].first) && !compare_keys(NodeList->_data[left].first, key)) {
        return bsptree_iterator(NodeList, left);
    }
    return end(); 
}


template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator BSP_tree<tkey, tvalue, compare, t>::find(const tkey& key) const
{
    return static_cast<bsptree_const_iterator>(find(key));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::lower_bound(const tkey& key)
{
    if (_root == nullptr) return end();
    bsptree_iterator iter = begin();
    while (iter != end() && compare_keys(iter->first, key))   // продвигаемся, пока ключ элемента меньше key
        ++iter;
    return iter;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator BSP_tree<tkey, tvalue, compare, t>::lower_bound(const tkey& key) const
{
    if (_root == nullptr) return end();
    bsptree_const_iterator iter = begin();
    while (iter != end() && compare_keys(iter->first, key))   // продвигаемся, пока ключ элемента меньше key
        ++iter;
    return iter;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::upper_bound(const tkey& key)
{
    if (_root == nullptr) return end();
    bsptree_iterator iter = begin();
    while (iter != end() && !compare_keys(key, iter->first))   // продвигаемся, пока ключ элемента меньше key
        ++iter;
    return iter;
}


template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator BSP_tree<tkey, tvalue, compare, t>::upper_bound(const tkey& key) const
{
    if (_root == nullptr) return end();
    bsptree_const_iterator iter = begin();
    while (iter != end() && !compare_keys(key, iter->first))   // продвигаемся, пока ключ элемента меньше key
        ++iter;
    return iter;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BSP_tree<tkey, tvalue, compare, t>::contains(const tkey& key) const
{
    return find(key) != end();
}

// endregion BSP_tree lookup implementations

// region BSP_tree modifiers implementations

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::clear() noexcept
{
    this->delete_subtree(this->_root);
    this->_root = nullptr;
    this->_size = 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::delete_subtree(bsptree_node_base * node) {
    if (node == nullptr) {return;}
    if (node->_is_terminated) {
        auto * node_term = static_cast<bsptree_node_term*>(node);
        delete node_term;
    } else {
        auto * node_middle = static_cast<bsptree_node_middle*>(node);
        for (bsptree_node_base * child : node_middle->_pointers) {
            delete_subtree(child);
        }
        delete node_middle;
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::split_root() {

    bsptree_node_middle * new_root = new bsptree_node_middle();
    bsptree_node_term * right_child = new bsptree_node_term();
    bsptree_node_term * left_child = new bsptree_node_term();
    bsptree_node_term * old_root = static_cast<bsptree_node_term*>(this->_root);


    size_t new_root_index = old_root->_data.size() / 2;
    auto new_root_value = old_root->_data[new_root_index].first;

    for (size_t i = 0; i < new_root_index; ++i) { 
        left_child->_data.push_back(old_root->_data[i]);
    }

    for (size_t i = new_root_index; i < old_root->_data.size(); ++i) { 
        right_child->_data.push_back(old_root->_data[i]);
    }

    left_child->_next = right_child;
    right_child->_next = nullptr;

    new_root->_keys.push_back(new_root_value);
    new_root->_pointers.push_back(left_child);
    new_root->_pointers.push_back(right_child);
    delete old_root;
    this->_root = new_root;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
std::pair<typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator, bool> BSP_tree<tkey, tvalue, compare, t>::insert(const tree_data_type& data)
{
    // добавить return!
    std::stack<std::pair<bsptree_node_middle*, size_t>> path;

    if (this->_root == nullptr) {
        auto * leaf = new bsptree_node_term();
        leaf->_data.push_back(data);
        this->_root = leaf;
        ++this->_size;
        return {bsptree_iterator(leaf, 0), true};
        
    }
    if (this->_root->_is_terminated) {
        std::cout << "insert in root" << std::endl;
        bsptree_node_term * leaf = static_cast<bsptree_node_term*>(this->_root);
        size_t i = 0;
        while (i < leaf->_data.size() && compare_keys(leaf->_data[i].first, data.first)) {
            ++i;
        }
        leaf->_data.insert(leaf->_data.begin() + i, data);
        ++this->_size;
        if (leaf->_data.size() > maximum_keys_in_root) {
            std::cout << "root is full" << std::endl;
            split_root();
            print_structure();
            return {begin(), true}; 
        }
        return {bsptree_iterator(leaf, i), true};
    } else {
        bsptree_node_base * curr = this->_root;
        while (!curr->_is_terminated) {
            auto * middle = static_cast<bsptree_node_middle*>(curr);
            size_t i = 0;
            while (i < middle->_keys.size() && compare_keys(middle->_keys[i], data.first)) {
                ++i;
            }
            path.push({middle, i});
            curr = middle->_pointers[i];
        }
        bsptree_node_term * leaf = static_cast<bsptree_node_term*>(curr);
        size_t i = 0;
        while (i < leaf->_data.size() && compare_keys(leaf->_data[i].first, data.first)) {
            ++i;
        }
    
        if (i < leaf->_data.size() && !compare_keys(leaf->_data[i].first, data.first) && !compare_keys(data.first, leaf->_data[i].first)) { // равны, ключ уже есть
            return {bsptree_iterator(leaf, i), false};
        }
        leaf->_data.insert(leaf->_data.begin() + i, data);
        ++this->_size;
        if (leaf->_data.size() > maximum_keys_in_node) {
            std::cout << leaf->_data.size() << maximum_keys_in_node << std::endl;
            handle_leaf_overflow(path, leaf);
        }  
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::handle_leaf_overflow(std::stack<std::pair<bsptree_node_middle*, size_t>>& path, bsptree_node_term * leaf) {
    if (try_redistribute_leaf(path, leaf)) {
        return;
    }
    split_leaf_2_to_3(path, leaf);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::handle_inner_overflow(std::stack<std::pair<bsptree_node_middle*, size_t>>& path, bsptree_node_middle * node) {
    if (try_redistribute_inner(path, node)) {
        return;
    }
    split_inner_2_to_3(path, node);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BSP_tree<tkey, tvalue, compare, t>::try_redistribute_leaf(std::stack<std::pair<bsptree_node_middle*, size_t>>& path, bsptree_node_term * leaf) {
    bsptree_node_middle * parent = path.top().first;
    size_t child_index = path.top().second;

    bsptree_node_term * right_brother = child_index < parent->_keys.size() ? static_cast<bsptree_node_term*>(parent->_pointers[child_index + 1]) : nullptr; 
    bsptree_node_term * left_brother = child_index > 0 ? static_cast<bsptree_node_term*>(parent->_pointers[child_index - 1]) : nullptr;

    if (right_brother != nullptr && right_brother->_data.size() < maximum_keys_in_node) { // перекидываем правому брату
        right_brother->_data.insert(right_brother->_data.begin(), leaf->_data[leaf->_data.size() - 1]);
        leaf->_data.pop_back();
        parent->_keys[child_index] = right_brother->_data[0].first;
        return true;
    }

    if (left_brother != nullptr && left_brother->_data.size() < maximum_keys_in_node) {
        left_brother->_data.push_back(leaf->_data[0]);
        leaf->_data.erase(leaf->_data.begin());
        parent->_keys[child_index - 1] = leaf->_data[0].first;
        return true;
    }
    return false;    
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BSP_tree<tkey, tvalue, compare, t>::try_redistribute_inner(std::stack<std::pair<bsptree_node_middle*, size_t>>& path, bsptree_node_middle * node) {
    bsptree_node_middle * parent = path.top().first;
    size_t child_index = path.top().second;
    bsptree_node_middle * right_brother = child_index < parent->_keys.size() ? static_cast<bsptree_node_middle*>(parent->_pointers[child_index + 1]) : nullptr; 
    bsptree_node_middle * left_brother = child_index > 0 ? static_cast<bsptree_node_middle*>(parent->_pointers[child_index - 1]) : nullptr;

    if (right_brother != nullptr && right_brother->_keys.size() < maximum_keys_in_node) {
        right_brother->_keys.insert(right_brother->_keys.begin(), node->_keys[node->_keys.size() - 1]);
        node->_keys.pop_back();
        node->_pointers.pop_back();
        right_brother->_pointers.insert(right_brother->_pointers.begin(), node->_pointers[node->_pointers.size() - 1]);
        parent->_keys[child_index] = right_brother->_keys[0];
    }
    
    if (left_brother != nullptr && left_brother->_keys.size() < maximum_keys_in_node) {
        left_brother->_keys.push_back(node->_keys[0]);
        node->_keys.erase(node->_keys.begin());
        node->_pointers.erase(node->_pointers.begin());
        left_brother->_pointers.push_back(node->_pointers[0]);
        parent->_keys[child_index - 1] = node->_keys[0];
        return true;
    }
    return false;
}


template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::split_leaf_2_to_3(std::stack<std::pair<bsptree_node_middle*, size_t>>& path, bsptree_node_term * leaf) {
    bsptree_node_term * new_leaf = new bsptree_node_term();
    bsptree_node_middle * parent = path.top().first;
    size_t child_index = path.top().second;
    path.pop();
    bsptree_node_term * right_brother_to_merge = child_index < parent->_keys.size() ? static_cast<bsptree_node_term*>(parent->_pointers[child_index + 1]) : nullptr; 
    bsptree_node_term * left_brother_to_merge = child_index > 0 ? static_cast<bsptree_node_term*>(parent->_pointers[child_index - 1]) : nullptr;
    std::vector<tree_data_type> merged;
    bsptree_node_term * right_node = nullptr;
    bsptree_node_term * left_node = nullptr;

    if (right_brother_to_merge == nullptr) {
        left_node = left_brother_to_merge; // взяли левого брата
        right_node = leaf;
    } else {
        left_node = leaf;
        right_node = right_brother_to_merge;
    }

    merged.insert(merged.end(), left_node->_data.begin(), left_node->_data.end());
    merged.insert(merged.end(), right_node->_data.begin(), right_node->_data.end());

    size_t overall_size = merged.size();
    size_t border1 = overall_size / 3;
    size_t border2 = 2 * overall_size / 3;

    left_node->_data.clear();
    right_node->_data.clear();

    for (size_t i = 0; i < border1; ++i) {
        left_node->_data.push_back(merged[i]);
    }

    for (size_t i = border1; i < border2; ++i) {
        new_leaf->_data.push_back(merged[i]);
    }

    for (size_t i = border2; i < overall_size; ++i) {
        right_node->_data.push_back(merged[i]);
    }

    bsptree_node_term * old_next = right_node->_next;
    left_node->_next = new_leaf;
    new_leaf->_next = right_node;
    right_node->_next = old_next;

    parent->_keys.erase(parent->_keys.begin() + (child_index - 1));
    parent->_pointers.erase(parent->_pointers.begin() + child_index);

    parent->_keys.insert(parent->_keys.begin() + (child_index - 1), new_leaf->_data[0].first);
    parent->_keys.insert(parent->_keys.begin() + child_index, right_node->_data[0].first);

    parent->_pointers.insert(parent->_pointers.begin() + child_index, new_leaf);
    parent->_pointers.insert(parent->_pointers.begin() + child_index + 1, right_node);

    if (parent->_keys.size() > maximum_keys_in_node) {
        handle_inner_overflow(path, parent);
    }
}


template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::split_inner_2_to_3(std::stack<std::pair<bsptree_node_middle*, size_t>>& path, bsptree_node_middle * node) {
    bsptree_node_middle * new_leaf = new bsptree_node_middle();
    bsptree_node_middle * parent = path.top().first;
    size_t child_index = path.top().second;
    path.pop();
    std::vector<tkey> merged_keys;
    std::vector<bsptree_node_base*> merged_pointers;
    bsptree_node_middle * right_node = nullptr;
    bsptree_node_middle * left_node = nullptr;
    // немного переписать логику, чтобы была консистентна
    if (child_index + 1 > parent->_keys.size()) { 
        left_node = static_cast<bsptree_node_middle*>(parent->_pointers[child_index - 1]); // взяли левого брата
        right_node = node;
    } else {
        left_node = node;
        right_node = static_cast<bsptree_node_middle*>(parent->_pointers[child_index + 1]);
    }

    merged_keys.insert(merged_keys.end(), left_node->_keys.begin(), left_node->_keys.end());
    merged_pointers.insert(merged_pointers.end(), left_node->_pointers.begin(), left_node->_pointers.end());

    merged_keys.insert(merged_keys.end(), right_node->_keys.begin(), right_node->_keys.end());
    merged_pointers.insert(merged_pointers.end(), right_node->_pointers.begin(), right_node->_pointers.end());

    size_t overall_size = merged_keys.size();
    size_t border1 = overall_size / 3;
    size_t border2 = 2 * overall_size / 3;

    left_node->_keys.clear();
    right_node->_keys.clear();
    left_node->_pointers.clear();
    right_node->_pointers.clear();

    for (size_t i = 0; i < border1; ++i) {
        left_node->_keys.push_back(merged_keys[i]);
    }
    for (size_t i = 0; i < border1 + 1; ++i) {
        left_node->_pointers.push_back(merged_pointers[i]);
    }

    for (size_t i = border1; i < border2; ++i) {
        new_leaf->_keys.push_back(merged_keys[i]);
    }
    for (size_t i = border1 + 1; i < border2 + 1; ++i) {
        new_leaf->_pointers.push_back(merged_pointers[i]);
    }

    for (size_t i = border2; i < overall_size; ++i) {
        right_node->_keys.push_back(merged_keys[i]);
    }
    for (size_t i = border2 + 1; i < overall_size + 1; ++i) {
        right_node->_pointers.push_back(merged_pointers[i]);
    }

    parent->_keys.erase(parent->_keys.begin() + (child_index - 1));
    parent->_pointers.erase(parent->_pointers.begin() + child_index);

    parent->_keys.insert(parent->_keys.begin() + (child_index - 1), new_leaf->_keys[0]);
    parent->_keys.insert(parent->_keys.begin() + child_index, right_node->_keys[0]);

    parent->_pointers.insert(parent->_pointers.begin() + child_index, new_leaf);
    parent->_pointers.insert(parent->_pointers.begin() + child_index + 1, right_node);

    if (parent->_keys.size() > maximum_keys_in_node) {
        handle_inner_overflow(path, parent);
    }
}



template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
std::pair<typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator, bool> BSP_tree<tkey, tvalue, compare, t>::insert(tree_data_type&& data)
{
    std::stack<std::pair<bsptree_node_middle*, size_t>> path;

    if (this->_root == nullptr) {
        auto * leaf = new bsptree_node_term();
        leaf->_data.push_back(data);
        this->_root = leaf;
        ++this->_size;
        print_structure();
        return {bsptree_iterator(leaf, 0), true};
    }
    if (this->_root->_is_terminated) {
        bsptree_node_term * leaf = static_cast<bsptree_node_term*>(this->_root);
        size_t i = 0;
        while (i < leaf->_data.size() && compare_keys(leaf->_data[i].first, data.first)) {
            ++i;
        }
        leaf->_data.insert(leaf->_data.begin() + i, data);
        ++this->_size;
        if (leaf->_data.size() > maximum_keys_in_root) {
            split_root();
            print_structure();
            return {begin(), true}; 
        }
        return {bsptree_iterator(leaf, i), true};
    } else {
        bsptree_node_base * curr = this->_root;
        while (!curr->_is_terminated) {
            auto * middle = static_cast<bsptree_node_middle*>(curr);
            size_t i = 0;
            while (i < middle->_keys.size() && compare_keys(middle->_keys[i], data.first)) {
                ++i;
            }
            path.push({middle, i});
            curr = middle->_pointers[i];

        }
        bsptree_node_term * leaf = static_cast<bsptree_node_term*>(curr);
        size_t i = 0;
        while (i < leaf->_data.size() && compare_keys(leaf->_data[i].first, data.first)) {
            ++i;
        }
    
        if (i < leaf->_data.size() && !compare_keys(leaf->_data[i].first, data.first) && !compare_keys(data.first, leaf->_data[i].first)) { // равны, ключ уже есть
            return {bsptree_iterator(leaf, i), false};
        }
        leaf->_data.insert(leaf->_data.begin() + i, data);
        ++this->_size;
        if (leaf->_data.size() > maximum_keys_in_node) {
            std::cout << leaf->_data.size() << maximum_keys_in_node << std::endl;
            handle_leaf_overflow(path, leaf);
        }  
        return {find(data.first), true};
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template<typename ...Args>
std::pair<typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator, bool> BSP_tree<tkey, tvalue, compare, t>::emplace(Args&&... args)
{
    return insert(tree_data_type(std::forward<Args>(args)...));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::insert_or_assign(const tree_data_type& data)
{
    throw not_implemented("template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t> typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::insert_or_assign(const tree_data_type& data)", "your code should be here...");
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::insert_or_assign(tree_data_type&& data)
{
    throw not_implemented("template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t> typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::insert_or_assign(tree_data_type&& data)", "your code should be here...");
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template<typename ...Args>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::emplace_or_assign(Args&&... args)
{
    throw not_implemented("template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t> template<typename ...Args> typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::emplace_or_assign(Args&&... args)", "your code should be here...");
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::erase(bsptree_iterator pos)
{
    if (pos == end()) {
        return end();
    }

    // тут еще чота!!!!!!!!!
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::erase(bsptree_const_iterator pos)
{
    if (pos == end()) {
        return end();
    } else {
        return bsptree_iterator(nullptr, 0);
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::erase(bsptree_iterator beg, bsptree_iterator en)
{
    if (en != beg) {
        beg = erase(beg);
    }
    return en;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::erase(bsptree_const_iterator beg, bsptree_const_iterator en)
{
    if (en != beg) {
        beg = erase(beg);
    }
    return bsptree_iterator(nullptr, 0); // ПОПРАВИТЬ
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::erase(const tkey& key)
{
    print_structure();
    std::stack<std::pair<bsptree_node_middle*, size_t>> path;
    size_t index_to_remove = 0;
    if (find(key) == end()) {
        return end();
    }
    bsptree_node_base * cur = this->_root;
    while (!cur->_is_terminated) {
        auto * node = static_cast<bsptree_node_middle*>(cur);
        size_t i = 0;
        while (i < node->_keys.size() && compare_keys(node->_keys[i], key)) { // заменить на функцию
            ++i;
        }
        path.push({node, i});
        cur = node->_pointers[i];
    }
    size_t i = 0;
    bsptree_node_term * node_term = static_cast<bsptree_node_term*>(cur);
    while (i < node_term->_data.size() && compare_keys(node_term->_data[i].first, key)) {
        ++i;
    }
    if (i < node_term->_data.size() && !compare_keys(node_term->_data[i].first, key) && !compare_keys(key, node_term->_data[i].first)) {
        index_to_remove = i;
    }
    tkey next_key;
    bool has_next = false;
    if (index_to_remove + 1 < node_term->_data.size()) {
        next_key = node_term->_data[index_to_remove + 1].first;
        has_next = true;
    } else if (node_term->_next != nullptr && !node_term->_next->_data.empty()) {
        next_key = node_term->_next->_data[0].first;
        has_next = true;
    }
    node_term->_data.erase(node_term->_data.begin() + index_to_remove);
    --this->_size;

    if (cur == this->_root) {
        return has_next ? find(next_key) : end();
    }

    if (node_term->_data.size() < minimum_keys_in_node) {
        if (!path.empty()) {
            handle_lack_of_keys_leaf(path, node_term);
        }
    }
    print_structure();
    if (has_next) {
        return find(next_key);
    } else {
        return end();
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::handle_lack_of_keys_leaf(std::stack<std::pair<bsptree_node_middle*, size_t>>& path, bsptree_node_term * leaf) {
    if (try_borrow_leaf(path, leaf)) {
        return;
    }
    merge_leaf_3_to_2(path, leaf);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::handle_lack_of_keys_inner(std::stack<std::pair<bsptree_node_middle*, size_t>>& path, bsptree_node_middle * middle) {
    if (try_borrow_inner(path, middle)) {
        return;
    }
    merge_inner_3_to_2(path, middle);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BSP_tree<tkey, tvalue, compare, t>::try_borrow_leaf(std::stack<std::pair<bsptree_node_middle*, size_t>>& path, bsptree_node_term * leaf) {
    bsptree_node_term * right_brother = leaf->_next;
    bsptree_node_term * left_brother = nullptr;
    bsptree_node_middle * parent = path.top().first;
    size_t child_index = path.top().second;

    if (right_brother != nullptr && right_brother->_data.size() > minimum_keys_in_node) { // перекидываем правому брату
        leaf->_data.push_back(right_brother->_data[0]);
        right_brother->_data.erase(right_brother->_data.begin());
        parent->_keys[child_index] = right_brother->_data[0].first;
        return true;
    }

    if (child_index > 0) {
        left_brother = static_cast<bsptree_node_term*>(parent->_pointers[child_index - 1]);
        if (left_brother->_data.size() > minimum_keys_in_node) { 
            leaf->_data.insert(leaf->_data.begin(), left_brother->_data[left_brother->_data.size() - 1]);
            left_brother->_data.pop_back();
            parent->_keys[child_index - 1] = leaf->_data[0].first;
            return true;
        }
    }
    return false;    
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BSP_tree<tkey, tvalue, compare, t>::try_borrow_inner(std::stack<std::pair<bsptree_node_middle*, size_t>>& path, bsptree_node_middle * node) {
    bsptree_node_middle * right_brother = nullptr;
    bsptree_node_middle * left_brother = nullptr;
    bsptree_node_middle * parent = path.top().first;
    size_t child_index = path.top().second;

    if (child_index < parent->_keys.size()) {
        right_brother = static_cast<bsptree_node_middle*>(parent->_pointers[child_index + 1]);
        if (right_brother->_keys.size() > minimum_keys_in_node) {
            node->_keys.push_back(right_brother->_keys[0]);
            node->_pointers.push_back(right_brother->_pointers[0]);
            right_brother->_keys.erase(right_brother->_keys.begin());
            right_brother->_pointers.erase(right_brother->_pointers.begin());
            parent->_keys[child_index] = right_brother->_keys[0];
        }
    }
    
    if (child_index > 0) {
        left_brother = static_cast<bsptree_node_middle*>(parent->_pointers[child_index - 1]);
        if (left_brother->_keys.size() > minimum_keys_in_node) {
            node->_keys.insert(node->_keys.begin(), left_brother->_keys[node->_keys.size() - 1]);
            node->_pointers.insert(node->_pointers.begin(), left_brother->_pointers[node->_pointers.size() - 1]);
            left_brother->_keys.pop_back();
            left_brother->_pointers.pop_back();
            parent->_keys[child_index - 1] = node->_keys[0];
            return true;
        }
    }
    return false;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::merge_leaf_3_to_2(std::stack<std::pair<bsptree_node_middle*, size_t>>& path, bsptree_node_term * leaf) {
    bsptree_node_middle * parent = path.top().first;
    size_t child_index = path.top().second;
    path.pop();
    std::vector<tree_data_type> merged;
    bsptree_node_term * right_node = nullptr;
    bsptree_node_term * central_node = nullptr;
    bsptree_node_term * left_node = nullptr;

    if (parent->_keys.size() == 1) { // отец == корень (мб добавить проверку на это?)
        bsptree_node_term * new_root = new bsptree_node_term();
        left_node = static_cast<bsptree_node_term*>(parent->_pointers[0]);
        right_node = static_cast<bsptree_node_term*>(parent->_pointers[1]);
        merged.insert(merged.end(), left_node->_data.begin(), left_node->_data.end());
        merged.insert(merged.end(), right_node->_data.begin(), right_node->_data.end());
        new_root->_data.insert(new_root->_data.end(), merged.begin(), merged.end());
        this->_root = new_root;
        delete left_node;
        delete right_node;
        delete parent;
        return;
    }
    bsptree_node_term * right_brother_to_merge = child_index < parent->_keys.size() ? static_cast<bsptree_node_term*>(parent->_pointers[child_index + 1]) : nullptr; 
    bsptree_node_term * left_brother_to_merge = child_index > 0 ? static_cast<bsptree_node_term*>(parent->_pointers[child_index - 1]) : nullptr;
    size_t central_index = 0;

    if (right_brother_to_merge == nullptr && left_brother_to_merge != nullptr) { // берем предыдущего и предыдущего для предыдущего
        right_node = leaf;
        central_node = left_brother_to_merge;
        left_node = static_cast<bsptree_node_term*>(parent->_pointers[child_index - 2]);
        central_index = child_index - 1;
    } else if (right_brother_to_merge != nullptr && left_brother_to_merge == nullptr) { // берем следующего и следующего для следующего
        left_node = leaf;
        central_node= right_brother_to_merge;
        right_node = static_cast<bsptree_node_term*>(parent->_pointers[child_index + 2]);
        central_index = child_index + 1;
    } else {
        right_node = right_brother_to_merge;
        central_node = leaf;
        left_node = left_brother_to_merge;
        central_index = child_index;
    }

    merged.insert(merged.end(), left_node->_data.begin(), left_node->_data.end());
    merged.insert(merged.end(), central_node->_data.begin(), central_node->_data.end());
    merged.insert(merged.end(), right_node->_data.begin(), right_node->_data.end());

    size_t overall_size = merged.size();
    size_t border = overall_size / 2;

    left_node->_data.clear();
    right_node->_data.clear();
    central_node->_data.clear();
    
    for (size_t i = 0; i < border; ++i) {
        left_node->_data.push_back(merged[i]);
    }
    
    for (size_t i = border; i < overall_size; ++i) {
        right_node->_data.push_back(merged[i]);
    }
    
    bsptree_node_term * old_next = right_node->_next;
    left_node->_next = right_node;
    right_node->_next = old_next;
    delete central_node;
    
    parent->_keys.erase(parent->_keys.begin() + (central_index - 1));
    parent->_pointers.erase(parent->_pointers.begin() + central_index);
    parent->_keys.erase(parent->_keys.begin() + (central_index - 1));
    parent->_pointers.erase(parent->_pointers.begin() + central_index);

    parent->_keys.insert(parent->_keys.begin() + (central_index - 1), right_node->_data[0].first);
    parent->_pointers.insert(parent->_pointers.begin() + central_index, right_node);

    if (parent->_keys.size() < minimum_keys_in_node) {
        handle_lack_of_keys_inner(path, parent);
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::merge_inner_3_to_2(std::stack<std::pair<bsptree_node_middle*, size_t>>& path, bsptree_node_middle * middle) {
    bsptree_node_middle * parent = path.top().first;
    size_t child_index = path.top().second;
    path.pop();
    std::vector<tkey> merged_keys;
    std::vector<bsptree_node_base*> merged_pointers;

    bsptree_node_middle * right_node = nullptr;
    bsptree_node_middle * central_node = nullptr;
    bsptree_node_middle * left_node = nullptr;


    bsptree_node_middle * right_brother_to_merge = child_index < parent->_keys.size() ? static_cast<bsptree_node_middle*>(parent->_pointers[child_index + 1]) : nullptr; 
    bsptree_node_middle * left_brother_to_merge = child_index > 0 ? static_cast<bsptree_node_middle*>(parent->_pointers[child_index - 1]) : nullptr;
    size_t central_index = 0;

    if (right_brother_to_merge == nullptr && left_brother_to_merge != nullptr) { // берем предыдущего и предыдущего для предыдущего
        right_node = middle;
        central_node = left_brother_to_merge;
        left_node = static_cast<bsptree_node_middle*>(parent->_pointers[child_index - 2]);
        central_index = child_index - 1;
    } else if (right_brother_to_merge != nullptr && left_brother_to_merge == nullptr) { // берем следующего и следующего для следующего
        left_node = middle;
        central_node= right_brother_to_merge;
        right_node = static_cast<bsptree_node_middle*>(parent->_pointers[child_index + 2]);
        central_index = child_index + 1;
    } else {
        right_node = right_brother_to_merge;
        central_node = middle;
        left_node = left_brother_to_merge;
        central_index = child_index;
    }

    merged_keys.insert(merged_keys.end(), left_node->_keys.begin(), left_node->_keys.end());
    merged_keys.insert(merged_keys.end(), central_node->_keys.begin(), central_node->_keys.end());
    merged_keys.insert(merged_keys.end(), right_node->_keys.begin(), right_node->_keys.end());
    merged_pointers.insert(merged_pointers.end(), right_node->_pointers.begin(), right_node->_pointers.end());
    merged_pointers.insert(merged_pointers.end(), central_node->_pointers.begin(), central_node->_pointers.end());
    merged_pointers.insert(merged_pointers.end(), left_node->_pointers.begin(), left_node->_pointers.end());

    size_t overall_size = merged_keys.size();
    size_t border = overall_size / 2;

    // left_node->_data.clear();
    // right_node->_data.clear();
    // central_node->_data.clear();
    
    for (size_t i = 0; i < border; ++i) {
        left_node->_keys.push_back(merged_keys[i]);
    }
    for (size_t i = border; i < overall_size; ++i) {
        right_node->_keys.push_back(merged_keys[i]);
    }
    
    for (size_t i = 0; i < border; ++i) {
        left_node->_pointers.push_back(merged_pointers[i]);
    }
    for (size_t i = border; i < overall_size; ++i) {
        right_node->_pointers.push_back(merged_pointers[i]);
    }
    
    delete central_node;
    
    parent->_keys.erase(parent->_keys.begin() + (central_index - 1));
    parent->_pointers.erase(parent->_pointers.begin() + central_index);
    parent->_keys.erase(parent->_keys.begin() + (central_index - 1));
    parent->_pointers.erase(parent->_pointers.begin() + central_index);

    parent->_keys.insert(parent->_keys.begin() + (central_index - 1), right_node->_keys[0]);
    parent->_pointers.insert(parent->_pointers.begin() + central_index, right_node);

    if (parent->_keys.size() < minimum_keys_in_node) {
        handle_lack_of_keys_inner(path, parent);
    }
}


// endregion BSP_tree modifiers implementations

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::print_iterator(const bsptree_iterator& it) const
{
    if (it._node == nullptr) {
        std::cerr << "Iterator: end()" << std::endl;
        return;
    }
    std::cerr << "Iterator: node=" << it._node
              << " index=" << it._index;
    if (it._index < it._node->_data.size())
        std::cerr << " key=" << it._node->_data[it._index].first
                  << " value=" << it._node->_data[it._index].second;
    std::cerr << std::endl;
}


template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::print_structure() const
{
    if (_root == nullptr) {
        std::cout << "Empty tree" << std::endl;
        return;
    }
    std::cout << "Tree elements (" << size() << "):\n";
    for (auto it = begin(); it != end(); ++it) {
        std::cout << "[" << it.index() << "] " << it->first << " -> " << it->second << std::endl;
    }
}

#include <string>   // если используете std::string в тесте
template class BSP_tree<int, std::string>;
template class BSP_tree<int, int>;

#endif