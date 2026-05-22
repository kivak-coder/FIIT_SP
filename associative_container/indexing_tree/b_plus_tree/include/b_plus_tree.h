#include <iterator>
#include <utility>
#include <vector>
#include <boost/container/static_vector.hpp>
#include <concepts>
#include <stack>
#include <pp_allocator.h>
#include <associative_container.h>
#include <not_implemented.h>
#include <initializer_list>

#ifndef SYS_PROG_B_PLUS_TREE_H
#define SYS_PROG_B_PLUS_TREE_H

template <typename tkey, typename tvalue, comparator<tkey> compare = std::less<tkey>, std::size_t t = 5>
class BP_tree final : private compare //EBCO
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

    struct bptree_node_base
    {
        bool _is_terminated;

        bptree_node_base() noexcept;
        virtual ~bptree_node_base() =default;
    };

    struct bptree_node_term : public bptree_node_base
    {
        bptree_node_term* _next;
        boost::container::static_vector<tree_data_type, maximum_keys_in_node + 1> _data;
        bptree_node_term() noexcept;
    };

    struct bptree_node_middle : public bptree_node_base
    {
        boost::container::static_vector<tkey, maximum_keys_in_node + 1> _keys;
        boost::container::static_vector<bptree_node_base*, maximum_keys_in_node + 2> _pointers;
        bptree_node_middle() noexcept;
    };

    pp_allocator<value_type> _allocator;
    bptree_node_base* _root;
    size_t _size;

    pp_allocator<value_type> get_allocator() const noexcept;

public:

    // region constructors declaration

    explicit BP_tree(const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    explicit BP_tree(pp_allocator<value_type> alloc, const compare& comp = compare());

    template<input_iterator_for_pair<tkey, tvalue> iterator>
    explicit BP_tree(iterator begin, iterator end, const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    BP_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    // endregion constructors declaration

    // region five declaration

    BP_tree(const BP_tree& other);

    BP_tree(BP_tree&& other) noexcept;

    BP_tree& operator=(const BP_tree& other);

    BP_tree& operator=(BP_tree&& other) noexcept;

    ~BP_tree() noexcept;

    // endregion five declaration

    // region iterators declaration

    class bptree_iterator;
    class bptree_const_iterator;

    class bptree_iterator final
    {
        bptree_node_term* _node;
        size_t _index;

    public:
        using value_type = tree_data_type_const;
        using reference = value_type&;
        using pointer = value_type*;
        using iterator_category = std::forward_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = bptree_iterator;

        friend class BP_tree;
        friend class bptree_const_iterator;

        reference operator*() const noexcept;
        pointer operator->() const noexcept;

        self& operator++();
        self operator++(int);

        bool operator==(const self& other) const noexcept;
        bool operator!=(const self& other) const noexcept;

        size_t current_node_keys_count() const noexcept;
        size_t index() const noexcept;

        explicit bptree_iterator(bptree_node_term* node = nullptr, size_t index = 0);

    };

    class bptree_const_iterator final
    {
        const bptree_node_term* _node;
        size_t _index;

    public:

        using value_type = tree_data_type_const;
        using reference = const value_type&;
        using pointer = const value_type*;
        using iterator_category = std::forward_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = bptree_const_iterator;

        friend class BP_tree;
        friend class bptree_iterator;

        bptree_const_iterator(const bptree_iterator& it) noexcept;

        reference operator*() const noexcept;
        pointer operator->() const noexcept;

        self& operator++();
        self operator++(int);

        bool operator==(const self& other) const noexcept;
        bool operator!=(const self& other) const noexcept;

        size_t current_node_keys_count() const noexcept;
        size_t index() const noexcept;

        explicit bptree_const_iterator(const bptree_node_term* node = nullptr, size_t index = 0);
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

    // endregion element access declaration
    // region iterator begins declaration

    bptree_iterator begin();
    bptree_iterator end();

    bptree_const_iterator begin() const;
    bptree_const_iterator end() const;

    bptree_const_iterator cbegin() const;
    bptree_const_iterator cend() const;

    // endregion iterator begins declaration

    // region lookup declaration

    size_t size() const noexcept;
    bool empty() const noexcept;

    /*
     * Returns end() if not exist
     */

    bptree_iterator find(const tkey& key);
    bptree_const_iterator find(const tkey& key) const;

    bptree_iterator lower_bound(const tkey& key);
    bptree_const_iterator lower_bound(const tkey& key) const;

    bptree_iterator upper_bound(const tkey& key);
    bptree_const_iterator upper_bound(const tkey& key) const;

    bool contains(const tkey& key) const;

    // endregion lookup declaration

    // region modifiers declaration

    void clear() noexcept;
    void delete_subtree(bptree_node_base * node);

    

    /*
     * Does nothing if key exists, delegates to emplace.
     * Second return value is true, when inserted
     */
    std::pair<bptree_iterator, bool> insert(const tree_data_type& data);
    std::pair<bptree_iterator, bool> insert(tree_data_type&& data);
    void split_root();
    void split_leaf(std::stack<std::pair<bptree_node_middle*, size_t>>& path, bptree_node_term * leaf);
    void split_inner(std::stack<std::pair<bptree_node_middle*, size_t>>& path, bptree_node_middle * middle);

    template <typename ...Args>
    std::pair<bptree_iterator, bool> emplace(Args&&... args);

    /*
     * Updates value if key exists, delegates to emplace.
     */
    bptree_iterator insert_or_assign(const tree_data_type& data);
    bptree_iterator insert_or_assign(tree_data_type&& data);

    template <typename ...Args>
    bptree_iterator emplace_or_assign(Args&&... args);

    /*
     * Return iterator to node next ro removed or end() if key not exists
     */
    bptree_iterator erase(bptree_iterator pos);
    bptree_iterator erase(bptree_const_iterator pos);

    void handle_lack_of_keys_leaf(std::stack<std::pair<bptree_node_middle*, size_t>>& path, bptree_node_term * leaf);
    void handle_lack_of_keys_inner(std::stack<std::pair<bptree_node_middle*, size_t>>& path, bptree_node_middle * middle);

    bool try_borrow_leaf(std::stack<std::pair<bptree_node_middle*, size_t>>& path, bptree_node_term * leaf);
    bool try_borrow_inner(std::stack<std::pair<bptree_node_middle*, size_t>>& path, bptree_node_middle * node);

    void merge_leaf(std::stack<std::pair<bptree_node_middle*, size_t>>& path, bptree_node_term * leaf);
    void merge_inner(std::stack<std::pair<bptree_node_middle*, size_t>>& path, bptree_node_middle * middle);

    bptree_iterator erase(bptree_iterator beg, bptree_iterator en);
    bptree_iterator erase(bptree_const_iterator beg, bptree_const_iterator en);


    bptree_iterator erase(const tkey& key);
    void print_structure() const;


    // endregion modifiers declaration
};

template<std::input_iterator iterator, comparator<typename std::iterator_traits<iterator>::value_type::first_type> compare = std::less<typename std::iterator_traits<iterator>::value_type::first_type>,
        std::size_t t = 5, typename U>
BP_tree(iterator begin, iterator end, const compare &cmp = compare(), pp_allocator<U> = pp_allocator<U>()) -> BP_tree<typename std::iterator_traits<iterator>::value_type::first_type, typename std::iterator_traits<iterator>::value_type::second_type, compare, t>;

template<typename tkey, typename tvalue, comparator<tkey> compare = std::less<tkey>, std::size_t t = 5, typename U>
BP_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare &cmp = compare(), pp_allocator<U> = pp_allocator<U>()) -> BP_tree<tkey, tvalue, compare, t>;

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BP_tree<tkey, tvalue, compare, t>::compare_pairs(const BP_tree::tree_data_type &lhs,
                                                     const BP_tree::tree_data_type &rhs) const
{
    return compare::operator()(lhs.first, rhs.first);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::bptree_node_base::bptree_node_base() noexcept : _is_terminated(false) {}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::bptree_node_term::bptree_node_term() noexcept : bptree_node_base(), _next(nullptr)
{
    this->_is_terminated = true;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::bptree_node_middle::bptree_node_middle() noexcept {}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
pp_allocator<typename BP_tree<tkey, tvalue, compare, t>::value_type> BP_tree<tkey, tvalue, compare, t>::
get_allocator() const noexcept
{
    return this->_allocator;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator::reference BP_tree<tkey, tvalue, compare, t>::
bptree_iterator::operator*() const noexcept
{
    return reinterpret_cast<reference>(_node->_data[_index]);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator::pointer BP_tree<tkey, tvalue, compare, t>::bptree_iterator
::operator->() const noexcept
{
    return reinterpret_cast<pointer>(&_node->_data[_index]);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator::self & BP_tree<tkey, tvalue, compare, t>::bptree_iterator::
operator++()
{
    ++this->_index;
    if (_index == _node->_data.size()) {
        if (_node->_next == nullptr) {
            *this = bptree_iterator(nullptr, 0);  
        } else {
            _node = _node->_next;
            _index = 0;
        }
    }

    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator::self BP_tree<tkey, tvalue, compare, t>::bptree_iterator::
operator++(int)
{
    self temp = *this;
    ++*this;
    return temp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BP_tree<tkey, tvalue, compare, t>::bptree_iterator::operator==(const self &other) const noexcept
{
    return this->_index == other._index && this->_node == other._node;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BP_tree<tkey, tvalue, compare, t>::bptree_iterator::operator!=(const self &other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BP_tree<tkey, tvalue, compare, t>::bptree_iterator::current_node_keys_count() const noexcept
{
    return this->_node->_data.size();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BP_tree<tkey, tvalue, compare, t>::bptree_iterator::index() const noexcept
{
    return this->_index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::bptree_iterator::bptree_iterator(bptree_node_term *node, size_t index)
{
    this->_node = node;
    this->_index = index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::bptree_const_iterator(const bptree_iterator &it) noexcept
{
    this->_node = it._node;
    this->_index = it._index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::reference BP_tree<tkey, tvalue, compare, t>::
bptree_const_iterator::operator*() const noexcept
{
    return reinterpret_cast<reference>(_node->_data[_index]);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::pointer BP_tree<tkey, tvalue, compare, t>::
bptree_const_iterator::operator->() const noexcept
{
    return reinterpret_cast<pointer>(&_node->_data[_index]);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::self & BP_tree<tkey, tvalue, compare, t>::
bptree_const_iterator::operator++()
{
    ++this->_index;
    if (_index == _node->_data.size()) {
        if (_node->_next == nullptr) {
            *this = bptree_iterator(nullptr, 0);  
        } else {
            _node = _node->_next;
            _index = 0;
        }
    }

    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::self BP_tree<tkey, tvalue, compare, t>::
bptree_const_iterator::operator++(int)
{
    self temp = *this;
    ++*this;
    return temp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::operator==(const self &other) const noexcept
{
    return this->_node == other._node && this->_index == other._index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::operator!=(const self &other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::current_node_keys_count() const noexcept
{
    return this->_node->_data.size();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::index() const noexcept
{
    return this->_index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::bptree_const_iterator(const bptree_node_term *node, size_t index)
{
    this->_node = node;
    this->_index = index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue & BP_tree<tkey, tvalue, compare, t>::at(const tkey& key)
{
    bptree_iterator iter = find(key);
    if (iter == end()) {
        throw std::out_of_range("key not found");
    }
    return iter->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
const tvalue & BP_tree<tkey, tvalue, compare, t>::at(const tkey& key) const
{
    bptree_const_iterator iter = find(key);
    if (iter == end()) {
        throw std::out_of_range("key not found");
    }
    return iter->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue & BP_tree<tkey, tvalue, compare, t>::operator[](const tkey &key)
{
    bptree_iterator iter = find(key);
    if (iter == end()) {
        auto res = insert(tree_data_type(key, tvalue()));
        iter = res.first;
    }
    return iter->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue & BP_tree<tkey, tvalue, compare, t>::operator[](tkey &&key)
{
    bptree_iterator iter = this->find(key);
    if (iter == end()) {
        auto res = insert(tree_data_type(std::move(key), tvalue())); 
        iter = res.first;
    }
    return iter->second;
}


template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BP_tree<tkey, tvalue, compare, t>::compare_keys(const tkey &lhs, const tkey &rhs) const
{
    return compare::operator()(lhs, rhs);
}


template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::BP_tree(const compare& cmp, pp_allocator<value_type> alloc) : compare(cmp), _allocator(alloc), _root(nullptr), _size(0) {}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::BP_tree(pp_allocator<value_type> alloc, const compare& cmp) : BP_tree(cmp, alloc) {}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template<input_iterator_for_pair<tkey, tvalue> iterator>
BP_tree<tkey, tvalue, compare, t>::BP_tree(iterator begin, iterator end, const compare& cmp, pp_allocator<value_type> alloc) : compare(cmp), _allocator(alloc), _root(nullptr), _size(0)
{
    for (auto it = begin(); it != end(); ++it) {
        insert(*it);
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::BP_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare& cmp, pp_allocator<value_type> alloc) : compare(cmp), _allocator(alloc), _root(nullptr), _size(0)
{
    for (const auto& p : data) {
        insert(p);
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::BP_tree(const BP_tree& other) : compare(other), _allocator(other._allocator), _root(nullptr), _size(0)
{
    for (auto it = other.cbegin(); it != cend(); ++it) {
        insert(*it);
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::BP_tree(BP_tree&& other) noexcept : compare(std::move(other)), _allocator(std::move(other._allocator)), 
_root(std::exchange(other._root, nullptr)), _size(std::exchange(other._size, 0)) {}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>& BP_tree<tkey, tvalue, compare, t>::operator=(const BP_tree& other)
{
    if (this != &other) {
        BP_tree tmp(other);
        std::swap(_root, tmp._root);
        this->_size = other._size;
        std::swap(_allocator, other._allocator);
    }
    return *this;
}

// почему тут не эксчэнж?? надо сделать его?? абоб
template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>& BP_tree<tkey, tvalue, compare, t>::operator=(BP_tree&& other) noexcept
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

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::~BP_tree() noexcept
{
    clear();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::begin()
{
    if (this->_root == nullptr || this->_size == 0) {
        return end();
    }
    bptree_node_base * node = this->_root;
    while (!node->_is_terminated) {
        bptree_node_middle * middle = static_cast<bptree_node_middle*>(node);
        node = middle->_pointers[0];
    }
    auto * leaf = static_cast<bptree_node_term*>(node);
    return bptree_iterator(leaf, 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::end()
{
    return bptree_iterator(nullptr, 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator BP_tree<tkey, tvalue, compare, t>::begin() const
{
    if (this->_root == nullptr || this->_size == 0) {
        return end();
    }
    bptree_node_base * node = this->_root;
    while (!node->_is_terminated) {
        bptree_node_middle * middle = static_cast<bptree_node_middle*>(node);
        node = middle->_pointers[0];
    }
    auto * leaf = static_cast<bptree_node_term*>(node);
    return bptree_const_iterator(leaf, 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator BP_tree<tkey, tvalue, compare, t>::end() const
{
    return bptree_const_iterator(nullptr, 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator BP_tree<tkey, tvalue, compare, t>::cbegin() const
{
    if (this->_root == nullptr || this->_size == 0) {
        return cend();
    }
    bptree_node_base * node = this->_root;
    while (!node->_is_terminated) {
        bptree_node_middle * middle = static_cast<bptree_node_middle*>(node);
        node = middle->_pointers[0];
    }
    auto * leaf = static_cast<bptree_node_term*>(node);
    return bptree_const_iterator(leaf, 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator BP_tree<tkey, tvalue, compare, t>::cend() const
{
    return bptree_const_iterator(nullptr, 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BP_tree<tkey, tvalue, compare, t>::size() const noexcept
{
    return this->_size;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BP_tree<tkey, tvalue, compare, t>::empty() const noexcept
{
    return this->_size == 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::find(const tkey& key)
{
    if (this->_root == nullptr) {
        return end();
    }
    bptree_node_base * curr = this->_root;
    while (!curr->_is_terminated) {
        auto * node = static_cast<bptree_node_middle*>(curr);
        size_t i = 0;
        while (i < node->_keys.size() && !compare_keys(key, node->_keys[i])) {
            ++i;
        }
        curr = node->_pointers[i];
    }
    
    auto * NodeList = static_cast<bptree_node_term*>(curr);
    // бинарный поиск нужного ключа в узле
    int left = 0;
    int right = NodeList->_data.size() - 1;
    
    while (left < right) {
        int mid = (left + right) / 2;
        bool less = compare_keys(key, NodeList->_data[mid].first);
        bool greater = compare_keys(NodeList->_data[mid].first, key);
        if (!greater && !less) {
            return bptree_iterator(NodeList, mid);
        }
        if (less && !greater) {
            right = mid - 1;
        } else {
            left = mid + 1;
        }
    }
    if (!compare_keys(key, NodeList->_data[left].first) && !compare_keys(NodeList->_data[left].first, key)) {
        return bptree_iterator(NodeList, left);
    }
    return end(); 
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator BP_tree<tkey, tvalue, compare, t>::find(const tkey& key) const
{
    return static_cast<bptree_const_iterator>(find(key));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::lower_bound(const tkey& key)
{
    if (_root == nullptr) return end();
    bptree_iterator iter = begin();
    while (iter != end() && compare_keys(iter->first, key))   // продвигаемся, пока ключ элемента меньше key
    ++iter;
    return iter;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator BP_tree<tkey, tvalue, compare, t>::lower_bound(const tkey& key) const
{
    if (_root == nullptr) return end();
    bptree_const_iterator iter = begin();
    while (iter != end() && compare_keys(iter->first, key))   // продвигаемся, пока ключ элемента меньше key
    ++iter;
    return iter;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::upper_bound(const tkey& key)
{
    if (_root == nullptr) return end();
    bptree_iterator iter = begin();
    while (iter != end() && !compare_keys(key, iter->first))   // продвигаемся, пока ключ элемента меньше key
    ++iter;
    return iter;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator BP_tree<tkey, tvalue, compare, t>::upper_bound(const tkey& key) const
{
    if (_root == nullptr) return end();
    bptree_const_iterator iter = begin();
    while (iter != end() && !compare_keys(key, iter->first))   // продвигаемся, пока ключ элемента меньше key
    ++iter;
    return iter;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BP_tree<tkey, tvalue, compare, t>::contains(const tkey& key) const
{
    return find(key) != end();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BP_tree<tkey, tvalue, compare, t>::clear() noexcept
{
    this->delete_subtree(this->_root);
    this->_root = nullptr;
    this->_size = 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BP_tree<tkey, tvalue, compare, t>::delete_subtree(bptree_node_base * node) {
    if (node == nullptr) {return;}
    if (node->_is_terminated) {
        auto * node_term = static_cast<bptree_node_term*>(node);
        delete node_term;
    } else {
        auto * node_middle = static_cast<bptree_node_middle*>(node);
        for (bptree_node_base * child : node_middle->_pointers) {
            delete_subtree(child);
        }
        delete node_middle;
    }
}
template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
std::pair<typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator, bool>
BP_tree<tkey, tvalue, compare, t>::insert(const tree_data_type& data) {
    return emplace(data);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
std::pair<typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator, bool>
BP_tree<tkey, tvalue, compare, t>::insert(tree_data_type&& data) {
    return emplace(std::move(data));
}


template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BP_tree<tkey, tvalue, compare, t>::split_root()
{
    bptree_node_middle * new_root = new bptree_node_middle();
    bptree_node_term * right_child = new bptree_node_term();
    bptree_node_term * left_child = new bptree_node_term();
    bptree_node_term * old_root = static_cast<bptree_node_term*>(this->_root);
    
    
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
void BP_tree<tkey, tvalue, compare, t>::split_leaf(std::stack<std::pair<bptree_node_middle*, size_t>>& path, bptree_node_term * leaf) 
{
    bptree_node_term * new_child = new bptree_node_term();
    bptree_node_middle * parent = path.top().first;
    size_t child_index = path.top().second;
    path.pop();
    size_t new_parent_elem_index = t;
    auto new_parent_elem_value = leaf->_data[new_parent_elem_index].first;
    
    for (int i = new_parent_elem_index; i < leaf->_data.size(); ++i) {
        new_child->_data.push_back(leaf->_data[i]);
    }
    
    leaf->_data.resize(new_parent_elem_index);

    new_child->_next = leaf->_next;
    leaf->_next = new_child;

    parent->_keys.insert(parent->_keys.begin() + child_index, new_parent_elem_value);
    parent->_pointers.insert(parent->_pointers.begin() + child_index + 1, new_child);
    
    if (parent->_keys.size() > maximum_keys_in_node) {
        split_inner(path, parent);
    }
}


template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BP_tree<tkey, tvalue, compare, t>::split_inner(std::stack<std::pair<bptree_node_middle*, size_t>>& path, bptree_node_middle * middle) 
{
    bptree_node_middle * new_child = new bptree_node_middle();
    bptree_node_middle * parent = path.top().first;
    size_t child_index = path.top().second;
    path.pop();
    size_t new_parent_elem_index = t - 1;
    auto new_parent_elem_value = middle->_keys[new_parent_elem_index];
    
    for (int i = new_parent_elem_index; i < middle->_keys.size(); ++i) {
        new_child->_keys.push_back(middle->_keys[i]);
    }
    for (int i = new_parent_elem_index; i < middle->_pointers.size(); ++i) {
        new_child->_pointers.push_back(middle->_pointers[i]);
    }
    
    middle->_keys.resize(new_parent_elem_index);
    middle->_pointers.resize(new_parent_elem_index);
    
    parent->_keys.insert(parent->_keys.begin() + child_index, new_parent_elem_value);
    parent->_pointers.insert(parent->_pointers.begin() + child_index + 1, new_child);  
    
    if (parent->_keys.size() > maximum_keys_in_node) {
        split_inner(path, parent);
    }
}


template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template <typename ...Args>
std::pair<typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator, bool> BP_tree<tkey, tvalue, compare, t>::emplace(Args&&... args)
{
    tree_data_type data(std::forward<Args>(args)...);
    std::stack<std::pair<bptree_node_middle*, size_t>> path;
    if (this->_root == nullptr) {
        auto * leaf = new bptree_node_term();
        leaf->_data.push_back(data);
        this->_root = leaf;
        ++this->_size;
        print_structure();
        return {bptree_iterator(leaf, 0), true};
    }
    if (this->_root->_is_terminated) {
        bptree_node_term * leaf = static_cast<bptree_node_term*>(this->_root);
        size_t i = 0;
        while (i < leaf->_data.size() && compare_keys(leaf->_data[i].first, data.first)) {
            ++i;
        }
        leaf->_data.insert(leaf->_data.begin() + i, data);
        ++this->_size;
        if (leaf->_data.size() > maximum_keys_in_node) {
            split_root();
        }
        print_structure();
        return {bptree_iterator(leaf, i), true};
    } else {
        bptree_node_base * cur = this->_root;
        while (!cur->_is_terminated) {
            bptree_node_middle * middle = static_cast<bptree_node_middle*>(cur);
            size_t i = 0;
            while (i < middle->_keys.size() && compare_keys(middle->_keys[i], data.first)) {
                ++i;
            }
            path.push({middle, i});
            cur = middle->_pointers[i];
        }
        bptree_node_term * leaf = static_cast<bptree_node_term*>(cur);
        size_t i = 0;
        while (i < leaf->_data.size() && compare_keys(leaf->_data[i].first, data.first)) {
            ++i;
        }
        
        if (i < leaf->_data.size() && !compare_keys(leaf->_data[i].first, data.first) && compare_keys(leaf->_data[i].first, data.first)) {
            return {bptree_iterator(leaf, i), false};
        }
        leaf->_data.insert(leaf->_data.begin() + i, data);
        ++this->_size;
        if (leaf->_data.size() > maximum_keys_in_node) {
            split_leaf(path, leaf);
        }
        print_structure();
        return {find(data.first), true};
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator
BP_tree<tkey, tvalue, compare, t>::insert_or_assign(const tree_data_type& data) {
    bptree_iterator it = find(data.first);
    if (it != end()) {
        it->second = data.second;   
        return it;
    }
    return insert(data).first;      
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator
BP_tree<tkey, tvalue, compare, t>::insert_or_assign(tree_data_type&& data) {
    bptree_iterator it = find(data.first);
    if (it != end()) {
        it->second = std::move(data.second);   
        return it;
    }
    return insert(std::move(data)).first;      
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template <typename ...Args>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator
BP_tree<tkey, tvalue, compare, t>::emplace_or_assign(Args&&... args) {
    tree_data_type data(std::forward<Args>(args)...);
    bptree_iterator it = find(data.first);
    if (it != end()) {
        it->second = std::move(data.second);   
        return it;
    }
    return emplace(std::forward<Args>(args)...).first;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator
BP_tree<tkey, tvalue, compare, t>::erase(bptree_iterator pos) {
    if (pos == end()) return end();
    tkey key = pos->first;
    ++pos;                     
    erase(key);                 
    return pos;                 
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator
BP_tree<tkey, tvalue, compare, t>::erase(bptree_const_iterator pos) {
    if (pos == cend()) return end();
    tkey key = pos->first;
    bptree_iterator next(pos._node, pos._index);
    ++next;
    erase(key);
    return next;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator
BP_tree<tkey, tvalue, compare, t>::erase(bptree_iterator beg, bptree_iterator en) {
    while (beg != en) {
        beg = erase(beg);
    }
    return en;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator
BP_tree<tkey, tvalue, compare, t>::erase(bptree_const_iterator beg, bptree_const_iterator en) {
    bptree_iterator beg_it(const_cast<bptree_node_term*>(beg._node), beg._index);
    bptree_iterator en_it(const_cast<bptree_node_term*>(en._node), en._index);
    while (beg_it != en_it) {
        beg_it = erase(beg_it);
    }
    return en_it;
}


template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::erase(const tkey& key)
{
    std::stack<std::pair<bptree_node_middle*, size_t>> path;
    size_t index_to_remove = 0;
    tkey next_key;
    bool has_next = false;
    if (find(key) == end()) {
        return end();
    }
    bptree_node_base * cur = this->_root;
    while (!cur->_is_terminated) {
        bptree_node_middle * middle = static_cast<bptree_node_middle*>(cur);
        size_t i = 0;
        while (i < middle->_keys.size() && compare_keys(middle->_keys[i], key)) {
            ++i;
        }
        path.push({middle, i});
        cur = middle->_pointers[i];
    }
    bptree_node_term * leaf = static_cast<bptree_node_term*>(cur);
    size_t i = 0;
    while (i < leaf->_data.size() && compare_keys(leaf->_data[i].first, key)) {
        ++i;
    }
    if (i < leaf->_data.size() && !compare_keys(leaf->_data[i].first, key)) {
        index_to_remove = i;
    }
    if (index_to_remove + 1 < leaf->_data.size()) {
        next_key = leaf->_data[index_to_remove + 1].first;
        has_next = true;
    } else if (leaf->_next != nullptr && !leaf->_next->_data.empty()) {
        has_next = true;
        next_key = leaf->_next->_data[0].first;
    }
    leaf->_data.erase(leaf->_data.begin() + index_to_remove);
    --this->_size;

    if (cur == this->_root) {
        return has_next ? find(next_key) : end();
    }
    
    if (leaf->_data.size() < minimum_keys_in_node) {
        if (!path.empty()) {
            handle_lack_of_keys_leaf(path, leaf);
        }
    }
    return has_next ? find(next_key) : end();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BP_tree<tkey, tvalue, compare, t>::handle_lack_of_keys_leaf(std::stack<std::pair<bptree_node_middle*, size_t>>& path, bptree_node_term * leaf) {
    if (try_borrow_leaf(path, leaf)) {
        return;
    }
    merge_leaf(path, leaf);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BP_tree<tkey, tvalue, compare, t>::handle_lack_of_keys_inner(std::stack<std::pair<bptree_node_middle*, size_t>>& path, bptree_node_middle * middle) {
    if (try_borrow_inner(path, middle)) {
        return;
    }
    merge_inner(path, middle);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BP_tree<tkey, tvalue, compare, t>::try_borrow_leaf(std::stack<std::pair<bptree_node_middle*, size_t>>& path, bptree_node_term * leaf) {
    bptree_node_term * right_brother = nullptr;
    bptree_node_term * left_brother = nullptr;
    bptree_node_middle * parent = path.top().first;
    size_t child_index = path.top().second;

    if (child_index < parent->_keys.size()) {
        right_brother = static_cast<bptree_node_term*>(parent->_pointers[child_index + 1]);
        if (right_brother->_data.size() > minimum_keys_in_node) { // перекидываем правому брату
            leaf->_data.push_back(right_brother->_data[0]);
            right_brother->_data.erase(right_brother->_data.begin());
            parent->_keys[child_index] = right_brother->_data[0].first;
            return true;
        }
    }

    if (child_index > 0) {
        left_brother = static_cast<bptree_node_term*>(parent->_pointers[child_index - 1]);
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
bool BP_tree<tkey, tvalue, compare, t>::try_borrow_inner(std::stack<std::pair<bptree_node_middle*, size_t>>& path, bptree_node_middle * node) {
    bptree_node_middle * right_brother = nullptr;
    bptree_node_middle * left_brother = nullptr;
    bptree_node_middle * parent = path.top().first;
    size_t child_index = path.top().second;

    if (child_index < parent->_keys.size()) {
        right_brother = static_cast<bptree_node_middle*>(parent->_pointers[child_index + 1]);
        if (right_brother->_keys.size() > minimum_keys_in_node) {
            node->_keys.push_back(right_brother->_keys[0]);
            node->_pointers.push_back(right_brother->_pointers[0]);
            right_brother->_keys.erase(right_brother->_keys.begin());
            right_brother->_pointers.erase(right_brother->_pointers.begin());
            parent->_keys[child_index] = right_brother->_keys[0];
        }
    }
    
    if (child_index > 0) {
        left_brother = static_cast<bptree_node_middle*>(parent->_pointers[child_index - 1]);
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
void BP_tree<tkey, tvalue, compare, t>::merge_leaf(std::stack<std::pair<bptree_node_middle*, size_t>>& path, bptree_node_term * leaf) {
    bptree_node_middle * parent = path.top().first;
    size_t child_index = path.top().second;
    path.pop();
    std::vector<tree_data_type> merged;
    bptree_node_term * right_node = nullptr;
    bptree_node_term * left_node = nullptr;

    bptree_node_term * right_brother_to_merge = child_index < parent->_keys.size() ? static_cast<bptree_node_term*>(parent->_pointers[child_index + 1]) : nullptr; 
    bptree_node_term * left_brother_to_merge = child_index > 0 ? static_cast<bptree_node_term*>(parent->_pointers[child_index - 1]) : nullptr;

    if (right_brother_to_merge == nullptr) {
        right_node = leaf;
        left_node = left_brother_to_merge;
    } else {
        right_node = right_brother_to_merge;
        left_node = leaf;
    }

    for (auto& data : right_node->_data) {
        left_node->_data.push_back(data);
    }
    delete right_node;

    parent->_keys.erase(parent->_keys.begin() + child_index);
    parent->_pointers.erase(parent->_pointers.begin() + child_index + 1);

}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BP_tree<tkey, tvalue, compare, t>::merge_inner(std::stack<std::pair<bptree_node_middle*, size_t>>& path, bptree_node_middle * middle) {
    bptree_node_middle * parent = path.top().first;
    size_t child_index = path.top().second;
    path.pop();
    std::vector<tree_data_type> merged;
    bptree_node_middle * right_node = nullptr;
    bptree_node_middle * left_node = nullptr;

    bptree_node_middle * right_brother_to_merge = child_index < parent->_keys.size() ? static_cast<bptree_node_middle*>(parent->_pointers[child_index + 1]) : nullptr; 
    bptree_node_middle * left_brother_to_merge = child_index > 0 ? static_cast<bptree_node_middle*>(parent->_pointers[child_index - 1]) : nullptr;

    if (right_brother_to_merge == nullptr) {
        right_node = middle;
        left_node = left_brother_to_merge;
    } else {
        right_node = right_brother_to_merge;
        left_node = middle;
    }

    for (auto& key : right_node->_keys) {
        left_node->_keys.push_back(key);
    }
    for (auto& pointer : right_node->_pointers) {
        left_node->_pointers.push_back(pointer);
    }
    delete right_node;

    parent->_keys.erase(parent->_keys.begin() + child_index);
    parent->_pointers.erase(parent->_pointers.begin() + child_index + 1);
}


template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BP_tree<tkey, tvalue, compare, t>::print_structure() const
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

#endif