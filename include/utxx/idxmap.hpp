// ex: ts=4 sw=4 ft=cpp et indentexpr=
/**
 * \file
 * \brief symbol-to-index mapping
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

#ifndef _UTXX_IDXMAP_HPP_
#define _UTXX_IDXMAP_HPP_

#include <stdint.h>
#include <stdexcept>

namespace utxx {

// special table to map symbol and mask to index
// in context of simple trie implementation
template <int Pack>
class idxmap {
    static const int NElem = 1024 / Pack * 10;

public:
    typedef int8_t index_t;
    typedef char symbol_t;
    typedef uint16_t mask_t;

    static void init();
    static void index(mask_t a_mask, symbol_t a_symbol, mask_t& a_ret_mask,
        index_t& a_ret_index);

private:
    static index_t m_maps[NElem];
};

template <int P>
typename idxmap<P>::index_t idxmap<P>::m_maps[NElem];

template <>
void idxmap<1>::init() {
    for (mask_t i=0, sm=1u; i<10; ++i, sm<<=1) {
        for (mask_t mask=0; mask<1024; ++mask) {
            // find number of i-th 1 in the mask
            index_t idx = 0;
            for (mask_t m = 1u; m != sm; m <<= 1)
                if ((m & mask) != 0) ++idx;
            m_maps[mask | (i << 10)] = idx;
        }
    }
}

template <>
void idxmap<2>::init() {
    for (int i=0; i<10; ++i) {
        mask_t ii = i << 10;
        for (mask_t k=0; k<1024; ++k) {
            // find number of i-th 1 in the k
            index_t idx = 0;
            if ((k & (1 << i)) != 0)
                for (mask_t msk=1, j=0; j<=i; ++j, msk<<=1)
                    if ((msk & k) != 0) ++idx;
            idx = idx ? idx - 1 : 0x0f;
            mask_t m = k | ii;
            index_t& msk = m_maps[m >> 1];
            if ((m & 1) != 0)
                msk = (msk & 0x0f) | (idx << 4);
            else
                msk = (msk & 0xf0) | idx;
        }
    }
}

template<>
void idxmap<1>::index(mask_t a_mask, symbol_t a_symbol, mask_t& a_ret_mask,
    index_t& a_ret_index)
{
    if (a_mask > 1023)
        throw std::invalid_argument("bad mask");
    int i = a_symbol - '0';
    if (i < 0 || i > 9)
        throw std::invalid_argument("bad symbol");
    a_ret_mask = 1 << i;
    a_ret_index = m_maps[a_mask | (i << 10)];
}

/* template<> typename idxmap<2>::index_t
idxmap<2>::index(mask_t a_mask, symbol_t a_symbol) {
    if (a_mask > 1023)
        return -1;
    int i = a_symbol - '0';
    if (i < 0 || i > 9)
        return -1;
    mask_t m = a_mask | (i << 10);
    if ((m & 1) != 0)
        return (m_maps[m >> 1] & 0xf0) >> 4;
    else
        return m_maps[m >> 1] & 0x0f;
} */

} // namespace utxx

#endif // _UTXX_IDXMAP_HPP_
