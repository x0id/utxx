// ex: ts=4 sw=4 ft=cpp et indentexpr=
/**
 * \file
 * \brief strie write-to-file demo
 *
 * \author Dmitriy Kargapolov
 * \since 14 November 2013
 *
 */

/*
 * Copyright (C) 2013 Dmitriy Kargapolov <dmitriy.kargapolov@gmail.com>
 * Use, modification and distribution are subject to the Boost Software
 * License, Version 1.0 (See accompanying file LICENSE_1_0.txt or copy
 * at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <utxx/simple_node_store.hpp>
#include <utxx/svector.hpp>
#include <utxx/sarray.hpp>
#include <utxx/pnode.hpp>
#include <utxx/ptrie.hpp>

#include "string_codec.hpp"
#include "file_store.hpp"

// payload type
typedef std::string data_t;

// trie node type
typedef utxx::pnode<utxx::simple_node_store<>, data_t, utxx::svector<> > node_t;

// trie type
typedef utxx::ptrie<node_t> trie_t;

template<typename AddrType>
struct MyTraits {
    typedef AddrType addr_type;
    typedef file_store<addr_type> store_type;
    typedef typename string_codec<addr_type>::writer data_encoder;
    typedef typename utxx::sarray<addr_type>::encoder coll_encoder;
    typedef trie_t::encoder<addr_type> trie_encoder;
};

// offset type in external data representation
typedef MyTraits<uint32_t> store_traits;

int main() {
    trie_t trie;

    // store some data
    trie.store("123", "three");
    trie.store("1234", "four");
    trie.store("12345", "five");

    // write (export) trie to the file in external format
    store_traits::store_type store("trie.bin");
    trie.store_trie<store_traits>(store);

    return 0;
}
