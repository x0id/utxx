//----------------------------------------------------------------------------
/// \file  test_pcap.cpp
//----------------------------------------------------------------------------
/// \brief Test cases for PCAP file format parser.
//----------------------------------------------------------------------------
// Copyright (c) 2010 Serge Aleynikov <saleyn@gmail.com>
// Created: 2010-09-30
//----------------------------------------------------------------------------
/*
***** BEGIN LICENSE BLOCK *****

This file may be included in different open-source projects

Copyright (C) 2010 Serge Aleynikov <saleyn@gmail.com>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

***** END LICENSE BLOCK *****
*/

#include <boost/test/unit_test.hpp>
#include <utxx/pcap.hpp>
#include <utxx/verbosity.hpp>

using namespace utxx;

static const uint8_t s_buffer[] = {
    212,195,178,161,2,0,4,0,0,0,0,0,0,0,0,0,255,255,0,0,1,0,0,0,213,0,212,76,132,
    54,5,0,77,0,0,0,77,0,0,0,1,0,94,54,12,119,0,28,35,123,225,201,8,0,69,0,0,63,
    63,219,64,0,27,17,102,130,206,200,244,218,233,54,12,119,181,2,103,109,0,43,
    27,209,48,48,48,48,48,53,51,56,54,66,0,0,0,0,0,95,210,7,0,1,0,13,68,35,188,
    252,101,0,0,0,0,0,85,96,134,213,0,212,76,208,54,5,0,99,0,0,0,99,0,0,0,1,0,94,
    54,12,32,0,28,35,123,225,201,8,0,69,0,0,85,204,187,64,0,27,17,218,76,206,200,
    244,112,233,54,12,32,233,148,103,116,0,65,171,255,48,48,48,48,48,53,51,56,54,
    66,16,192,19,0,1,0,39,0,51,50,55,55,55,53,57,57,85,83,32,32,32,32,32,49,48,
    48,48,84,76,84,32,32,32,32,32,32,32,57,56,49,57,48,48,78,83,68,81,213,0,212,
    76,206,71,5,0,177,2,0,0,177,2,0,0,1,0,94,72,79,25,0,28,35,123,225,201,8,0,69,
    0,2,163,0,0,64,0,17,17,100,122,159,125,42,113,233,200,79,25,188,64,238,97,2,
    143,3,67,1,69,66,70,79,32,65,32,32,48,48,49,49,53,49,54,53,48,75,57,54,65,53,
    57,53,84,76,84,32,32,32,32,32,32,32,32,32,32,80,32,32,48,32,32,32,32,65,65,
    65,82,32,32,66,48,48,48,48,48,48,48,48,57,56,48,54,48,48,48,48,48,48,50,66,
    48,48,48,48,48,48,48,48,57,56,49,57,48,48,48,48,48,49,55,32,32,32,32,32,32,
    32,32,32,48,50,31,69,66,70,79,32,65,32,32,48,48,49,49,53,49,54,53,49,75,57,
    54,65,53,57,54,84,76,84,32,32,32,32,32,32,32,32,32,32,80,32,32,48,32,32,32,
    32,65,65,65,82,32,32,66,48,48,48,48,48,48,48,48,57,56,48,54,48,48,48,48,48,
    48,50,66,48,48,48,48,48,48,48,48,57,56,49,57,48,48,48,48,48,48,52,32,32,32,
    32,32,32,32,32,32,48,50,31,69,66,70,79,32,65,32,32,48,48,49,49,53,49,54,53,
    50,80,57,54,65,53,57,57,84,66,84,32,32,32,32,32,32,32,32,32,32,80,32,32,48,
    32,32,32,32,65,65,65,82,32,32,68,48,48,48,48,48,48,51,53,49,55,48,48,48,48,
    48,48,48,51,52,68,48,48,48,48,48,48,51,53,50,52,48,48,48,48,48,48,48,48,52,
    32,32,32,32,32,32,32,32,32,49,50,31,69,66,70,79,32,65,32,32,48,48,49,49,53,
    49,54,53,51,80,57,54,65,53,57,57,84,76,84,32,32,32,32,32,32,32,32,32,32,80,
    32,32,48,32,32,32,32,65,65,65,82,32,32,68,48,48,48,48,48,48,57,56,49,48,48,
    48,48,48,48,48,48,48,49,68,48,48,48,48,48,48,57,56,49,53,48,48,48,48,48,48,
    48,49,51,32,32,32,32,32,32,32,32,32,54,50,80,68,48,48,57,56,49,48,48,48,48,
    48,49,32,90,66,48,48,48,48,57,56,49,53,48,49,51,32,31,69,66,70,79,32,65,32,
    32,48,48,49,49,53,49,54,53,52,74,57,54,65,54,48,48,84,76,84,32,32,32,32,32,
    32,32,32,32,32,80,32,32,48,32,32,32,32,65,65,65,82,32,32,66,48,48,48,48,48,
    48,48,48,57,56,48,54,48,48,48,48,48,48,50,66,48,48,48,48,48,48,48,48,57,56,
    50,48,48,48,48,48,48,48,54,32,32,32,32,32,32,32,32,32,48,50,31,69,66,70,79,
    32,65,32,32,48,48,49,49,53,49,54,53,53,75,57,54,65,54,48,48,84,76,84,32,32,
    32,32,32,32,32,32,32,32,80,32,32,48,32,32,32,32,65,65,65,82,32,32,66,48,48,
    48,48,48,48,48,48,57,56,48,54,48,48,48,48,48,48,50,66,48,48,48,48,48,48,48,
    48,57,56,50,48,48,48,48,48,48,49,57,32,32,32,32,32,32,32,32,32,48,50,3
};

BOOST_AUTO_TEST_CASE( test_pcap_reader )
{
    BOOST_STATIC_ASSERT(sizeof(pcap::file_header)   == 24);
    BOOST_STATIC_ASSERT(sizeof(pcap::packet_header) == 16);

    pcap reader;

    const char* p = reinterpret_cast<const char*>(s_buffer);
    BOOST_REQUIRE_EQUAL(0,   reader.read_file_header(p, sizeof(s_buffer)));
    BOOST_REQUIRE_EQUAL(77,  reader.read_packet_header(p, sizeof(s_buffer)));

    BOOST_REQUIRE_EQUAL(2u,     reader.header().version_major);
    BOOST_REQUIRE_EQUAL(4u,     reader.header().version_minor);
    BOOST_REQUIRE_EQUAL(0,      reader.header().thiszone);
    BOOST_REQUIRE_EQUAL(0u,     reader.header().sigfigs);
    BOOST_REQUIRE_EQUAL(65535u, reader.header().snaplen);
    BOOST_REQUIRE_EQUAL(1u,     reader.header().network);

    BOOST_REQUIRE_EQUAL(1288962261u, reader.packet().ts_sec);
    BOOST_REQUIRE_EQUAL(341636u,     reader.packet().ts_usec);
    BOOST_REQUIRE_EQUAL(77u,         reader.packet().incl_len);
    BOOST_REQUIRE_EQUAL(77u,         reader.packet().orig_len);

    while ( (p - reinterpret_cast<const char*>(s_buffer)) < sizeof(s_buffer) ) {
        size_t n = reader.packet().incl_len;
        p += n;
        if (verbosity::level() >= VERBOSE_DEBUG)
            std::cout << "  Got packet len " << n << std::endl;
        int rc = reader.read_packet_header(p, sizeof(s_buffer));
        BOOST_REQUIRE(rc == (int)reader.packet().incl_len);
    }
}
