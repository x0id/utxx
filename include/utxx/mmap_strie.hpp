// ex: ts=4 sw=4 ft=cpp et indentexpr=
/**
 * \file
 * \brief s-trie in memory mapped region
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

#ifndef _UTXX_MMAP_STRIE_HPP_
#define _UTXX_MMAP_STRIE_HPP_

#include <utxx/flat_mem_strie.hpp>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>

namespace utxx {

namespace { namespace bip = boost::interprocess; }

template <typename Store, typename Data, typename SArray = utxx::sarray<> >
class mmap_strie {
protected:
    bip::file_mapping m_fmap;
    bip::mapped_region m_reg;
    void  *m_addr; // address of memory region
    size_t m_size; // size of memory region
    typedef flat_mem_strie<Store, Data, SArray> trie_t;
    typedef typename trie_t::offset_t offset_t;
    trie_t m_trie;

    // provide root node offset - make it virtual?
    virtual size_t root() {
        return *(offset_t*)((char *)m_addr + (m_size - sizeof(offset_t)));
    }

public:
    mmap_strie(const char *fname/*, bool a_verbose*/)
        : m_fmap(fname, bip::read_only)
        , m_reg(m_fmap, bip::read_only)
        , m_addr(m_reg.get_address())
        , m_size(m_reg.get_size())
        , m_trie(m_addr, m_size, root())
    {}

    // fold through trie nodes following key components
    template <typename A, typename F>
    void fold(const char *key, A& acc, F proc) {
        m_trie.fold(key, acc, proc);
    }

    // lookup data by key, prefix matching only
    template <typename F>
    Data* lookup(const char *a_key, F is_empty) {
        return m_trie.lookup(a_key, is_empty);
    }

    // lookup data by key, prefix matching only, default "data empty" functor
    Data* lookup(const char *a_key) {
        return m_trie.lookup(a_key);
    }

    // lookup data by key, prefix matching only, simple "data empty" functor
    Data* lookup_simple(const char *a_key) {
        return m_trie.lookup_simple(a_key);
    }

    // lookup data by key, exact matching allowed
    template <typename F>
    Data* lookup_exact(const char *a_key, F is_empty) {
        return m_trie.lookup_exact(a_key, is_empty);
    }

    // lookup data by key, exact matching allowed, default "data empty" functor
    Data* lookup_exact(const char *a_key) {
        return m_trie.lookup_exact(a_key);
    }
};

} // namespace utxx

#endif // _UTXX_MMAP_STRIE_HPP_
