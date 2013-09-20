// ex: ts=4 sw=4 ft=cpp et indentexpr=
/**
 * \file
 * \brief generic trie with string based key
 *
 * \author Dmitriy Kargapolov
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
#include <boost/bind.hpp>

namespace utxx {

namespace detail {

/**
 * \brief this class implements node of the trie
 * \tparam Store node store facility
 * \tparam Data node payload type
 * \tparam SArray type of collection of child nodes
 *
 * The Store and SArray types are themselv templates
 */
template <typename Store, typename Data, typename SArray>
class strie_node {
public:
    typedef typename Store::template rebind<strie_node>::other store_t;
    typedef typename store_t::pointer_t ptr_t;

    // sparse storage types
    typedef typename SArray::template rebind<ptr_t>::other sarray_t;
    typedef typename sarray_t::symbol_t symbol_t;

    strie_node() {}

    // build path adding missing nodes if needed
    strie_node *path_to_node(store_t& a_store, const char *a_key) {
        const char *l_ptr = a_key;
        symbol_t l_symbol;
        strie_node *l_node_ptr = this;
        while ((l_symbol = *l_ptr++) != 0)
            l_node_ptr = l_node_ptr->next_node(a_store, l_symbol);
        return l_node_ptr;
    }

    // store data, overwrite existing data if any
    void store(store_t& a_store, const char *a_key, const Data& a_data) {
        path_to_node(a_store, a_key)->m_data = a_data;
    }

    // update node data using provided merge-functor
    template <typename MergeFunctor, typename DataT>
    void update(store_t& a_store, const char *a_key, const DataT& a_data,
            MergeFunctor& a_merge) {
        strie_node *l_node_ptr = path_to_node(a_store, a_key);
        a_merge(l_node_ptr->m_data, a_data);
    }

    // fold through trie nodes following key components
    template <typename A, typename F>
    void fold(store_t& store, const char *key, A& acc, F proc) {
        const char *l_ptr = key;
        symbol_t l_symbol;
        strie_node *l_node_ptr = this;
        while ((l_symbol = *l_ptr++) != 0) {
            l_node_ptr = l_node_ptr->read_node(store, l_symbol);
            if (!l_node_ptr)
                break;
            if (!proc(acc, l_node_ptr->m_data, store, l_ptr))
                break;
        }
    }

    // write node to file
    template <typename T>
    T write_to_file(const store_t& a_store, std::ofstream& a_ofs)
            const {

        // write data payload, get encoded reference
        typename Data::template ext_header<T> l_data;
        m_data.write_to_file(l_data, a_store, a_ofs);

        // write children encoded, get encoded reference
        typename sarray_t::template ext_header<T> l_children;
        m_children.write_to_file(l_children,
            boost::bind(&strie_node::write_child<T>, this,
                boost::cref(a_store), _1, _2), a_ofs);

        // save offset of the node encoded
        T l_ret = boost::numeric_cast<T, std::streamoff>(a_ofs.tellp());

        // write obtained references
        l_data.write_to_file(a_ofs);
        l_children.write_to_file(a_ofs);

        // return offset of the node encoded
        return l_ret;
    }

    // should be called before destruction, can't pass a_store to destructor
    void clear(store_t& a_store) {
        m_children.foreach(
            boost::bind(&strie_node::del_child, boost::ref(a_store), _1));
    }

private:
    // used by sarray_t writer
    template <typename T>
    T write_child(const store_t& a_store, ptr_t a_child, std::ofstream& a_ofs)
            const {
        strie_node *l_ptr = a_store.
            template native_pointer<strie_node>(a_child);
        if (!l_ptr)
            throw std::invalid_argument("bad store pointer");
        return l_ptr->write_to_file<T>(a_store, a_ofs);
    }

    strie_node *read_node(store_t& a_store, symbol_t a_symbol) {
        const ptr_t *l_next_ptr = m_children.get(a_symbol);
        if (l_next_ptr == 0)
            return 0;
        strie_node *l_ptr = a_store.
            template native_pointer<strie_node>(*l_next_ptr);
        if (!l_ptr)
            throw std::invalid_argument("bad store pointer");
        return l_ptr;
    }

    strie_node *next_node(store_t& a_store, symbol_t a_symbol) {
        ptr_t& l_next = m_children.ensure(a_symbol,
            boost::bind(&strie_node::new_child, boost::ref(a_store)));
        strie_node *l_ptr = a_store.
            template native_pointer<strie_node>(l_next);
        if (!l_ptr)
            throw std::invalid_argument("bad store pointer");
        return l_ptr;
    }

    static ptr_t new_child(store_t& a_store) {
        ptr_t l_ptr = a_store.template allocate<strie_node>();
        if (l_ptr == store_t::null)
            throw std::runtime_error("store allocation error");
        return l_ptr;
    }

public:
    // child clean up - used by clear()
    static void del_child(store_t& a_store, ptr_t a_child) {
        strie_node *l_ptr = a_store.
            template native_pointer<strie_node>(a_child);
        if (!l_ptr) return;
        // recursive call to child's children
        l_ptr->clear(a_store);
        // this calls child's destructor
        a_store.template deallocate<strie_node>(a_child);
    }

    Data     m_data;     ///< node payload
    sarray_t m_children; ///< collection of child nodes
};

}

// namespace detail

namespace {

// conditional destroy
template<bool> struct node_destructor {
    template<typename T>
    static inline void destroy(typename T::ptr_t, typename T::store_t&);
};

template<> template<typename T> void
node_destructor<false>::destroy(typename T::ptr_t, typename T::store_t&) {
}

template<> template<typename T> void
node_destructor<true>::destroy(typename T::ptr_t n, typename T::store_t& s) {
    T::del_child(s, n);
}

}

template <typename Store, typename Data, typename SArray>
class strie {
public:
    typedef detail::strie_node<Store, Data, SArray> node_t;
    typedef typename node_t::store_t store_t;
    typedef typename store_t::pointer_t ptr_t;

    strie() : m_root(make_root()) {}

    strie(ptr_t a_root) : m_root(get_root(a_root)) {}

    strie(store_t& a_store)
        : m_store(a_store) , m_root(make_root())
    {}

    strie(store_t& a_store, ptr_t& a_root)
        : m_store(a_store), m_root(get_root(a_root))
    {}

    ~strie() {
        node_destructor<store_t::dynamic>::
            template destroy<node_t>(m_root_ptr, m_store);
    }

    const store_t& store() const { return m_store; }

    // store data, overwrite existing data if any
    void store(const char *a_key, const Data& a_data) {
        m_root.store(m_store, a_key, a_data);
    }

    // update node data using provided merge-functor
    template <typename MergeF, typename DataT>
    void update(const char *a_key, const DataT& a_data, MergeF& a_merge) {
        m_root.update(m_store, a_key, a_data, a_merge);
    }

    // fold through trie nodes following key components
    template <typename A, typename F>
    void fold(const char *key, A& acc, F proc) {
        m_root.fold(m_store, key, acc, proc);
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

    // default trie header
    template <typename T>
    struct enc_trie {
        T m_root;
        void write_to_file(store_t& a_store, std::ofstream& a_ofs) {
            a_ofs.write((const char *)this, sizeof(*this));
        }
    };

    // write trie to file
    template <typename T>
    void write_to_file(const char *a_fname) {
        ofile l_file(a_fname);
        char l_magic = 'A';
        l_file.ofs().write(&l_magic, sizeof(l_magic));
        enc_trie<T> l_trie;
        // write nodes
        l_trie.m_root = m_root.write_to_file<T>(m_store, l_file.ofs());
        // write trie
        l_trie.write_to_file(m_store, l_file.ofs());
    }

    // aux method for custom writers
    template <typename T>
    T write_root_node(std::ofstream& a_ofs) {
        return m_root.write_to_file<T>(m_store, a_ofs);
    }

protected:
    node_t& get_root(ptr_t a_ptr) {
        m_root_ptr = a_ptr;
        if (m_root_ptr == store_t::null)
            throw std::runtime_error("bad store pointer");
        node_t *l_ptr = m_store.
            template native_pointer<node_t>(a_ptr);
        if (l_ptr == NULL)
            throw std::runtime_error("bad store pointer");
        return *l_ptr;
    }

    node_t& make_root() {
        return get_root(m_store.template allocate<node_t>());
    }

    store_t m_store;
    ptr_t m_root_ptr;
    node_t& m_root;
};

} // namespace utxx

#endif // _UTXX_STRIE_HPP_
