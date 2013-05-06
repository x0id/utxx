// ex: ts=4 sw=4 ft=cpp et indentexpr=
/**
 * \file
 * \brief Test cases for classes and functions in the strie.hpp
 *
 * \author Dmitriy Kargapolov <dmitriy dot kargapolov at gmail dot com>
 * \since 01 Apr 2013
 *
 * Copyright (C) 2012 Dmitriy Kargapolov <dmitriy.kargapolov@gmail.com>
 *
 * Use, modification and distribution are subject to the Boost Software
 * License, Version 1.0 (See accompanying file LICENSE_1_0.txt or copy
 * at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <config.h>

#include <utxx/strie.hpp>
#include <utxx/memstat_alloc.hpp>

#include <boost/test/unit_test.hpp>
#include <boost/foreach.hpp>

#if defined HAVE_BOOST_CHRONO
#include <boost/chrono/system_clocks.hpp>
#endif

#include <boost/unordered_map.hpp>
#include <map>

namespace strie_test {

#if defined HAVE_BOOST_CHRONO
typedef boost::chrono::high_resolution_clock clock;
typedef clock::time_point time_point;
typedef clock::duration duration;
using boost::chrono::nanoseconds;
using boost::chrono::microseconds;
using boost::chrono::milliseconds;
using boost::chrono::duration_cast;
#endif

template<int N>
struct memstat {
    static void inc(size_t n) { cnt += n; }
    static void dec(size_t n) { cnt -= n; }
    static size_t cnt;
};

template<int N>
size_t memstat<N>::cnt = 0;

struct f0 {

    // memory counters
    enum { cKey, cData, cTabData, cMap, cStore, cTrie };

    typedef memstat_alloc<char, memstat<cKey> > key_alloc;
    typedef std::basic_string<char, std::char_traits<char>, key_alloc> key_t;

    typedef memstat_alloc<char, memstat<cData> > data_alloc;
    typedef std::basic_string<char, std::char_traits<char>, data_alloc> data_t;

    typedef memstat_alloc<char, memstat<cTabData> > tab_data_alloc;
    typedef std::basic_string<char, std::char_traits<char>, tab_data_alloc>
        tab_data_t;

    typedef std::pair<const key_t, tab_data_t> pair_t;
    typedef memstat_alloc<pair_t, memstat<cMap> > map_alloc;

    typedef std::map<key_t, tab_data_t, std::less<key_t>, map_alloc> map_t;

    typedef boost::unordered_map<key_t, tab_data_t, boost::hash<key_t>,
                                std::equal_to<key_t>, map_alloc> tab_t;
    typedef typename tab_t::const_iterator tab_it_t;

    // symbol-to-index mapping with compression factor of 1
    typedef utxx::idxmap<1> idxmap_t;

    f0() {
        idxmap_t::init();
    }

    class store;
    typedef memstat_alloc<char, memstat<cTrie> > trie_alloc;
    typedef utxx::strie<store, data_t, idxmap_t, trie_alloc> trie_t;

    // simplest store implementation
    class store {
    public:
        typedef memstat_alloc<trie_t, memstat<cStore> > alloc_t;
        store() : cnt(0) {}
        typedef trie_t* pointer_t;
        static const pointer_t null;
        pointer_t allocate() {
            ++cnt;
            pointer_t l_ptr = m_allocator.allocate(1);
            return new (l_ptr) trie_t;
        }
        void deallocate(pointer_t a_ptr) {
            a_ptr->~trie_t();
            m_allocator.deallocate(a_ptr, 1);
        }
        trie_t *native_pointer(pointer_t a_ptr) const {
            return a_ptr;
        }
        alloc_t m_allocator;
        int cnt;
    };

    static const char *makenum(int *cnt) {
        static char buf[11];
        int n = 5 + rand() % 5;
        for (int i=0; i<n; ++i)
            buf[i] = '0' + rand() % 10;
        if (cnt) *cnt += n;
        buf[n] = 0;
        return &buf[0];
    }

};
const f0::store::pointer_t f0::store::null = 0;

BOOST_AUTO_TEST_SUITE( test_strie )

BOOST_FIXTURE_TEST_CASE( write_read_test, f0 )
{
    { // start objects' life

    store l_store;
    trie_t l_data;
    tab_t l_tab;

    int l_total = 1000000;
    int l_cnt = 0;

    memstat<cData>::cnt = 0;
    memstat<cStore>::cnt = 0;
    memstat<cTrie>::cnt = 0;
    memstat<cKey>::cnt = 0;
    memstat<cTabData>::cnt = 0;
    memstat<cMap>::cnt = 0;

    srand(1);

    for (int i=0; i<l_total; ++i) {
        const char *l_num = makenum(&l_cnt);
        // insert data into s-trie
        l_data.store(l_store, l_num, data_t(l_num));
        // insert data into unordered map (hash-table)
        l_tab.insert(pair_t(l_num, l_num));
    }

    size_t l_trie_cnt =
        memstat<cData>::cnt + memstat<cStore>::cnt + memstat<cTrie>::cnt;
    size_t l_htab_cnt =
        memstat<cKey>::cnt + memstat<cTabData>::cnt + memstat<cMap>::cnt;
    BOOST_TEST_MESSAGE(
        "\n      unique objects count: " << l_tab.size() <<
        "\ntrie:  total nodes created: " << l_store.cnt <<
        "\ntrie: num of chars in keys: " << l_cnt <<
        "\n" <<
        "\ntrie: data bytes allocated: " << memstat<cData>::cnt <<
        "\ntrie: node bytes allocated: " << memstat<cStore>::cnt <<
        "\ntrie: nptr bytes allocated: " << memstat<cTrie>::cnt <<
        "\ntrie: total byte allocated: " << l_trie_cnt <<
        "\ntrie:     bytes per object: " << l_trie_cnt / l_tab.size() <<
        "\n" <<
        "\nhtab:  key bytes allocated: " << memstat<cKey>::cnt <<
        "\nhtab: data bytes allocated: " << memstat<cTabData>::cnt <<
        "\nhtab:  tab bytes allocated: " << memstat<cMap>::cnt <<
        "\nhtab: total byte allocated: " << l_htab_cnt <<
        "\nhtab:     bytes per object: " << l_htab_cnt / l_tab.size() <<
        "\n"
    );

    // looking for random matches
    srand(123);
    int l_found = 0, l_exact = 0;
    for (int i=0; i<l_total; ++i) {
        const char *l_num = makenum(0);
        data_t *l_data_ptr = l_data.lookup(l_store, l_num);
        if (l_data_ptr) {
            // full or substring match only
            BOOST_REQUIRE_EQUAL(0,
                strncmp(l_num, l_data_ptr->c_str(), l_data_ptr->length()));
            ++l_found;
            if (!strcmp(l_num, l_data_ptr->c_str()))
                ++l_exact;
        }
    }
    BOOST_TEST_MESSAGE( "from " << l_total << " found: " << l_found
        << ", exact: " << l_exact );

    // compare full strings matches to hash table
    BOOST_FOREACH(pair_t& p, l_tab) {
        data_t *l_data_ptr = l_data.lookup(l_store, p.first.c_str());
        BOOST_REQUIRE(l_data_ptr != 0);
        BOOST_REQUIRE_EQUAL(0, strcmp(p.second.c_str(), l_data_ptr->c_str()));
    }

    // clear store-based data before object destructed
    // (object does not keep store reference)
    l_data.clear(l_store);

    } // end of all objects life

    // make sure all memory released
    BOOST_REQUIRE_EQUAL(0, memstat<cData>::cnt );
    BOOST_REQUIRE_EQUAL(0, memstat<cStore>::cnt );
    BOOST_REQUIRE_EQUAL(0, memstat<cTrie>::cnt );
    BOOST_REQUIRE_EQUAL(0, memstat<cKey>::cnt );
    BOOST_REQUIRE_EQUAL(0, memstat<cTabData>::cnt );
    BOOST_REQUIRE_EQUAL(0, memstat<cMap>::cnt );
}

#if defined HAVE_BOOST_CHRONO_

BOOST_FIXTURE_TEST_CASE( chrono_test, f0 )
{
    store l_store;
    trie_t l_data;
    int l_total = 1000000;
    int l_cnt = 0;

    time_point tp1 = clock::now();
    for (int i=0; i<l_total; ++i) makenum(&l_cnt);
    duration d1 = clock::now() - tp1;

    srand(1);
    l_cnt = 0;

    map_t l_map;
    time_point tp2 = clock::now();
    for (int i=0; i<l_total; ++i) {
        const char *l_num = makenum(&l_cnt);
        l_map.insert(pair_t(l_num, l_num));
    }
    duration d2 = (clock::now() - tp2 - d1) / l_total;

    BOOST_TEST_MESSAGE( "map insert time "
        << duration_cast<nanoseconds>(d2).count() << " ns" );

    BOOST_TEST_MESSAGE( " key bytes allocated: " << memstat<cKey>::cnt );
    BOOST_TEST_MESSAGE( "data bytes allocated: " << memstat<cData>::cnt );
    BOOST_TEST_MESSAGE( " map bytes allocated: " << memstat<cMap>::cnt );
    BOOST_TEST_MESSAGE( "unique objects count: " << l_map.size() );
    BOOST_TEST_MESSAGE( "total byte allocated: " << memstat<cKey>::cnt +
        memstat<cData>::cnt + memstat<cMap>::cnt );
    BOOST_TEST_MESSAGE( "    bytes per object: " << (memstat<cKey>::cnt +
        memstat<cData>::cnt + memstat<cMap>::cnt) / l_map.size() );

    srand(1);
    l_cnt = 0;
    memstat<cKey>::cnt = 0;
    memstat<cData>::cnt = 0;
    memstat<cMap>::cnt = 0;

    tab_t l_tab;
    time_point tp2a = clock::now();
    for (int i=0; i<l_total; ++i) {
        const char *l_num = makenum(&l_cnt);
        l_tab.insert(pair_t(l_num, l_num));
    }
    duration d2a = (clock::now() - tp2a - d1) / l_total;

    BOOST_TEST_MESSAGE( "tab insert time "
        << duration_cast<nanoseconds>(d2a).count() << " ns" );

    BOOST_TEST_MESSAGE( " key bytes allocated: " << memstat<cKey>::cnt );
    BOOST_TEST_MESSAGE( "data bytes allocated: " << memstat<cData>::cnt );
    BOOST_TEST_MESSAGE( " tab bytes allocated: " << memstat<cMap>::cnt );
    BOOST_TEST_MESSAGE( "unique objects count: " << l_tab.size() );
    BOOST_TEST_MESSAGE( "total byte allocated: " << memstat<cKey>::cnt +
        memstat<cData>::cnt + memstat<cMap>::cnt );
    BOOST_TEST_MESSAGE( "    bytes per object: " << (memstat<cKey>::cnt +
        memstat<cData>::cnt + memstat<cMap>::cnt) / l_tab.size() );

    srand(1);
    l_cnt = 0;
    memstat<cKey>::cnt = 0;
    memstat<cData>::cnt = 0;

    time_point tp3 = clock::now();
    for (int i=0; i<l_total; ++i) {
        const char *l_num = makenum(&l_cnt);
        l_data.store(l_store, l_num, data_t(l_num));
    }
    duration d3 = (clock::now() - tp3 - d1) / l_total;

    BOOST_TEST_MESSAGE( "trie insert time "
        << duration_cast<nanoseconds>(d3).count() << " ns" );
    BOOST_TEST_MESSAGE( l_store.cnt << " nodes allocated for " << l_cnt
        << " symbols" );
    BOOST_TEST_MESSAGE( "in strings allocated: " << memstat<cData>::cnt );
    BOOST_TEST_MESSAGE( "in store   allocated: " << memstat<cStore>::cnt );
    BOOST_TEST_MESSAGE( "in strie   allocated: " << memstat<cTrie>::cnt );
    BOOST_TEST_MESSAGE( "total byte allocated: " << memstat<cData>::cnt +
        memstat<cStore>::cnt + memstat<cTrie>::cnt );
    BOOST_TEST_MESSAGE( "    bytes per object: " << (memstat<cData>::cnt +
        memstat<cStore>::cnt + memstat<cTrie>::cnt) / l_tab.size() );

    srand(123);
    int l_found = 0, l_exact = 0;
    time_point tp4 = clock::now();
    for (int i=0; i<l_total; ++i)
        l_data.lookup(l_store, makenum(0));
    duration d4 = (clock::now() - tp4 - d1) / l_total;
    BOOST_TEST_MESSAGE( "trie lookup time "
        << duration_cast<nanoseconds>(d4).count() << " ns" );

    srand(123);
    time_point tp5 = clock::now();
    for (int i=0; i<l_total; ++i) {
        const char *l_num = makenum(0);
        // key_t l_num = makenum(0);
        l_map.find(l_num);
    }
    duration d5 = (clock::now() - tp5 - d1) / l_total;
    BOOST_TEST_MESSAGE( "map lookup time "
        << duration_cast<nanoseconds>(d5).count() << " ns" );

    srand(123);
    time_point tp6 = clock::now();
    for (int i=0; i<l_total; ++i) {
        const char *l_num = makenum(0);
        // key_t l_num = makenum(0);
        l_tab.find(l_num);
    }
    duration d6 = (clock::now() - tp6 - d1) / l_total;
    BOOST_TEST_MESSAGE( "tab lookup time "
        << duration_cast<nanoseconds>(d6).count() << " ns" );

    srand(123);
    time_point tp7 = clock::now();
    tab_it_t e = l_tab.end();
    for (int i=0; i<l_total; ++i) {
        const char *l_num = makenum(0);
        size_t n = strlen(l_num);
        while (n > 0) {
            key_t l_key(l_num, n);
            tab_it_t it = l_tab.find(l_num);
            if (it != e)
                break;
            --n;
        }
    }
    duration d7 = (clock::now() - tp7 - d1) / l_total;
    BOOST_TEST_MESSAGE( "tab lookup time "
        << duration_cast<nanoseconds>(d7).count() << " ns" );
}

#endif // HAVE_BOOST_CHRONO

BOOST_AUTO_TEST_SUITE_END()

} // namespace strie_test
