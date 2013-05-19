// ex: ts=4 sw=4 ft=cpp et indentexpr=
/**
 * \file
 * \brief s-trie in flat memory region
 *
 * \author Dmitriy Kargapolov
 * \version 1.0
 * \since 16 April 2013
 *
 */

/*
 * Copyright (C) 2013 Dmitriy Kargapolov <dmitriy.kargapolov@gmail.com>
 * Use, modification and distribution are subject to the Boost Software
 * License, Version 1.0 (See accompanying file LICENSE_1_0.txt or copy
 * at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef _UTXX_FLAT_MEM_STRIE_HPP_
#define _UTXX_FLAT_MEM_STRIE_HPP_

#include <utxx/strie.hpp>
#include <utxx/sarray.hpp>

namespace utxx {

template <typename Store, typename Data, typename SArray = utxx::sarray<> >
class flat_mem_strie {
protected:
    // data storage type
    typedef typename Store::template rebind<Data>::other data_store_t;

    // wrapper for data store pointer to implement empty() required by strie
    struct data_ptr {
        typedef typename data_store_t::pointer_t data_ptr_t;
        data_ptr_t m_data_ptr;
        bool empty() const { return m_data_ptr == data_store_t::null; }
    };

    // strie node holding data store pointer
    typedef detail::strie_node<Store, data_ptr, SArray> node_t;

    // node storage type
    typedef typename node_t::store_t node_store_t;

public:
    // "generic" store offset type
    typedef typename Store::pointer_t offset_t;

    // both data and node storages mapped to the same memory region
    flat_mem_strie(const void *a_mem, offset_t a_len, offset_t a_root)
        : m_node_store(a_mem, a_len)
        , m_data_store(a_mem, a_len)
        , m_root(root(a_root))
    {}

    node_t& root(offset_t a_root) const {
        node_t *l_ptr = m_node_store.native_pointer(a_root);
        if (l_ptr == 0)
            throw std::invalid_argument("flat_mem_strie: bad root offset");
        return *l_ptr;
    }

    // lookup function, returns pointer to data or zero
    Data* lookup(const char *a_key) {
        data_ptr *l_data_ptr = m_root.lookup(m_node_store, a_key);
        if (l_data_ptr == 0)
            return 0;
        return m_data_store.native_pointer(l_data_ptr->m_data_ptr);
    }

protected:
    node_store_t m_node_store;
    data_store_t m_data_store;
    node_t& m_root;
};

} // namespace utxx

#endif // _UTXX_FLAT_MEM_STRIE_HPP_
