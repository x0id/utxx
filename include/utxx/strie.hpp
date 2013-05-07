// ex: ts=4 sw=4 ft=cpp et indentexpr=
/**
 * \file
 * \brief symbol-based trie
 *
 * \author Dmitriy Kargapolov
 * \version 1.0
 * \since 01 April 2013
 *
 */

/*
 * Copyright (C) 2013 Dmitriy Kargapolov <dmitriy.kargapolov@gmail.com>
 * Use, modification and distribution are subject to the Boost Software
 * License, Version 1.0 (See accompanying file LICENSE_1_0.txt or copy
 * at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef _UTXX_STRIE_HPP_
#define _UTXX_STRIE_HPP_

#include <vector>
#include <stdexcept>

namespace utxx {

namespace detail {

template <typename Store, typename Data, typename Map, typename Alloc>
class strie_node {
public:
    typedef typename Store::template rebind<strie_node>::other store_t;

private:
    typedef typename store_t::pointer_t ptr_t;
    typedef Data data_t;
    typedef Map idxmap_t;

    typedef typename idxmap_t::index_t index_t;
    typedef typename idxmap_t::symbol_t symbol_t;
    typedef typename idxmap_t::mask_t mask_t;

    typedef std::vector<ptr_t, Alloc> array_t;
    typedef typename array_t::size_type asize_t;
    typedef typename array_t::iterator it_t;

public:
    strie_node() : m_mask(0) {}

    void store(store_t& a_store, const char *a_key, const data_t& a_data) {
        const char *l_ptr = a_key;
        symbol_t l_symbol;
        strie_node *l_node_ptr = this;
        while ((l_symbol = *l_ptr++) != 0)
            l_node_ptr = l_node_ptr->next_node(a_store, l_symbol);
        l_node_ptr->set_data(a_data);
    }

    data_t* lookup(store_t& a_store, const char *a_key) {
        const char *l_ptr = a_key;
        symbol_t l_symbol;
        strie_node *l_node_ptr = this;
        strie_node *l_save_node_ptr = 0;
        while ((l_symbol = *l_ptr++) != 0) {
            l_node_ptr = l_node_ptr->read_node(a_store, l_symbol);
            if (!l_node_ptr)
                break;
            if (l_node_ptr->is_data_node())
                l_save_node_ptr = l_node_ptr;
        }
        return l_save_node_ptr ? &l_save_node_ptr->m_data : 0;
    }

    // should be called before destruction, can't pass a_store to destructor
    void clear(store_t& a_store) {
        for (it_t it=m_children.begin(), e = m_children.end(); it != e; ++it) {
            strie_node *l_ptr = a_store.native_pointer(*it);
            if (!l_ptr)
                throw std::invalid_argument("bad store pointer");
            l_ptr->clear(a_store);
            a_store.deallocate(*it); // this calls child's destructor
        }
    }

private:
    strie_node *read_node(store_t& a_store, symbol_t a_symbol) {
        ptr_t l_next = get_next(a_symbol);
        if (l_next == store_t::null)
            return 0;
        strie_node *l_ptr = a_store.native_pointer(l_next);
        if (!l_ptr)
            throw std::invalid_argument("bad store pointer");
        return l_ptr;
    }

    strie_node *next_node(store_t& a_store, symbol_t a_symbol) {
        mask_t l_mask;
        index_t l_index;
        m_map.index(m_mask, a_symbol, l_mask, l_index);
        ptr_t l_next;
        if ((l_mask & m_mask) == 0) {
            l_next = new_child(a_store);
            m_children.insert(m_children.begin() + l_index, l_next);
            m_mask |= l_mask;
        } else {
            if (l_index < 0 || (asize_t)l_index >= m_children.size())
                throw std::runtime_error("bad index returned");
            l_next = m_children[l_index];
            if (l_next == store_t::null)
                throw std::runtime_error("null pointer retrieved");
        }
        strie_node *l_ptr = a_store.native_pointer(l_next);
        if (!l_ptr)
            throw std::invalid_argument("bad store pointer");
        return l_ptr;
    }

    void set_data(const data_t& a_data) {
        m_data = a_data;
    }

    data_t& get_data() {
        return m_data;
    }

    const data_t& get_data() const {
        return m_data;
    }

    bool is_data_node() const {
        return !m_data.empty();
    }

    ptr_t new_child(store_t& a_store) {
        ptr_t l_ptr = a_store.allocate();
        if (l_ptr == store_t::null)
            throw std::runtime_error("store allocation error");
        return l_ptr;
    }

    // input - symbol, output - pointer to the next node or NULL
    ptr_t get_next(symbol_t a_symbol) {
        mask_t l_mask;
        index_t l_index;
        m_map.index(m_mask, a_symbol, l_mask, l_index);
        if ((l_mask & m_mask) == 0)
            return store_t::null;
        if (l_index < 0 || (asize_t)l_index >= m_children.size())
            throw std::runtime_error("bad index returned");
        return m_children[l_index];
    }

    data_t m_data;
    mask_t m_mask;
    array_t m_children;
    static idxmap_t m_map;
};

template <typename Store, typename Data, typename Map, typename Alloc>
Map strie_node<Store, Data, Map, Alloc>::m_map;

};

// namespace detail

template <typename Store, typename Data, typename Map,
                          typename Alloc = std::allocator<char> >
class strie {
public:
    typedef detail::strie_node<Store, Data, Map, Alloc> node_t;
    typedef typename node_t::store_t store_t;

    ~strie() {
        m_root.clear(m_store);
    }

    void store(const char *a_key, const Data& a_data) {
        m_root.store(m_store, a_key, a_data);
    }

    Data* lookup(const char *a_key) {
        return m_root.lookup(m_store, a_key);
    }

protected:
    store_t m_store;
    node_t m_root;
};

} // namespace utxx

#endif // _UTXX_STRIE_HPP_
