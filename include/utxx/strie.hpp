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

#include <fstream>
#include <stdexcept>
#include <boost/numeric/conversion/cast.hpp>

namespace utxx {

namespace detail {

template <typename Store, typename Data, typename SArray>
class strie_node {
public:
    typedef typename Store::template rebind<strie_node>::other store_t;

private:
    typedef typename store_t::pointer_t ptr_t;

    // sparse storage types
    typedef typename SArray::template rebind<ptr_t>::other sarray_t;
    typedef typename sarray_t::symbol_t symbol_t;
    typedef typename sarray_t::pos_t pos_t;

    Data m_data;
    sarray_t m_children;

public:
    strie_node() {}

    void store(store_t& a_store, const char *a_key, const Data& a_data) {
        const char *l_ptr = a_key;
        symbol_t l_symbol;
        strie_node *l_node_ptr = this;
        while ((l_symbol = *l_ptr++) != 0)
            l_node_ptr = l_node_ptr->next_node(a_store, l_symbol);
        l_node_ptr->data(a_data);
    }

    Data* lookup(store_t& a_store, const char *a_key) {
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
        typedef typename sarray_t::iterator it_t;
        for (it_t it=m_children.begin(), e = m_children.end(); it != e; ++it) {
            strie_node *l_ptr = a_store.native_pointer(*it);
            if (!l_ptr)
                throw std::invalid_argument("bad store pointer");
            l_ptr->clear(a_store);
            a_store.deallocate(*it); // this calls child's destructor
        }
    }

    template <typename OffsetType>
    struct enc_node {
        typedef OffsetType offset_t;
        typedef typename sarray_t::mask_t mask_t;
        enum { capacity = sarray_t::capacity };

        offset_t m_data;
          mask_t m_mask;
        offset_t m_children[capacity];

        static size_t size_of(unsigned n) {
            if (n > capacity)
                throw std::out_of_range("invalid number of node children");
            return sizeof(enc_node) - (capacity - n) * sizeof(offset_t);
        }

        offset_t write_to_file(unsigned n, store_t&, std::ofstream& a_ofs) {
            offset_t l_ret = boost::numeric_cast<offset_t, long>(a_ofs.tellp());
            a_ofs.write((const char *)this, size_of(n));
            return l_ret;
        }
    };

    template <typename T>
    T write_to_file(store_t& a_store, std::ofstream& a_ofs) {
        typedef typename sarray_t::iterator it_t;
        enc_node<T> l_node;
        // write data, fill data offset
        l_node.m_data = m_data.write_to_file(a_store, a_ofs);
        // fill mask
        l_node.m_mask = m_children.mask();
        // write children, fill offsets
        unsigned i = 0;
        for (it_t it=m_children.begin(), e = m_children.end(); it != e; ++it) {
            if (i == sarray_t::capacity)
                throw std::out_of_range("number of children");
            strie_node *l_ptr = a_store.native_pointer(*it);
            if (!l_ptr)
                throw std::invalid_argument("bad store pointer");
            l_node.m_children[i] = l_ptr->write_to_file<T>(a_store, a_ofs);
            ++i;
        }
        // write myself - adjust m_children to i elements
        return l_node.write_to_file(i, a_store, a_ofs);
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
        pos_t l_pos;
        ptr_t l_next;
        if (m_children.find(a_symbol, l_pos)) {
            l_next = m_children.at(l_pos);
            if (l_next == store_t::null)
                throw std::runtime_error("null pointer retrieved");
        } else {
            l_next = new_child(a_store);
            m_children.insert(l_pos, l_next);
        }
        strie_node *l_ptr = a_store.native_pointer(l_next);
        if (!l_ptr)
            throw std::invalid_argument("bad store pointer");
        return l_ptr;
    }

    void data(const Data& a_data) {
        m_data = a_data;
    }

    Data& data() {
        return m_data;
    }

public:
    const Data& data() const {
        return m_data;
    }

private:
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
        pos_t l_pos;
        if (m_children.find(a_symbol, l_pos))
            return m_children.at(l_pos);
        else
            return store_t::null;
    }
};

};

// namespace detail

template <typename Store, typename Data, typename SArray>
class strie {
public:
    typedef detail::strie_node<Store, Data, SArray> node_t;
    typedef typename node_t::store_t store_t;

    ~strie() {
        m_root.clear(m_store);
    }

    const store_t& store() const { return m_store; }
    const node_t& root() const { return m_root; }

    void store(const char *a_key, const Data& a_data) {
        m_root.store(m_store, a_key, a_data);
    }

    Data* lookup(const char *a_key) {
        return m_root.lookup(m_store, a_key);
    }

    // RAII-wrapper for std::ofstream
    // can't just rely on std::ofstream destructor due to enabled exceptions
    // which could be thrown while in std::ofstream destructor...
    class ofile {
        std::ofstream m_ofs;
    public:
        ofile(const char *a_fname) {
            m_ofs.exceptions(std::ofstream::failbit | std::ofstream::badbit);
            m_ofs.open(a_fname, std::ofstream::out |
                std::ofstream::binary | std::ofstream::trunc);
        }
        ~ofile() {
            try {
                m_ofs.close();
            } catch (...) {
            }
        }
        std::ofstream& ofs() { return m_ofs; }
    };

    template <typename T>
    struct enc_trie {
        T m_root;
        void write_to_file(store_t& a_store, std::ofstream& a_ofs) {
            a_ofs.write((const char *)this, sizeof(*this));
        }
    };

    template <typename T>
    void write_to_file(const char *a_fname) {
        ofile l_file(a_fname);
        enc_trie<T> l_trie;
        // write nodes
        l_trie.m_root = m_root.write_to_file<T>(m_store, l_file.ofs());
        // write trie
        l_trie.write_to_file(m_store, l_file.ofs());
    }

protected:
    store_t m_store;
    node_t m_root;
};

} // namespace utxx

#endif // _UTXX_STRIE_HPP_
