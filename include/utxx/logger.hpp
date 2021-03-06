//----------------------------------------------------------------------------
/// \file  logger.hpp
//----------------------------------------------------------------------------
/// \brief Logging framework
//----------------------------------------------------------------------------
// Author:  Serge Aleynikov
// Created: 2009-11-25
//----------------------------------------------------------------------------
/*
***** BEGIN LICENSE BLOCK *****

This file is part of the utxx open-source project.

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

#ifndef _UTXX_LOGGER_HPP_
#define _UTXX_LOGGER_HPP_

namespace utxx { 

class logger;

enum log_level {
      NOLOGGING      = 0
    , LEVEL_NONE     = 0
    , LEVEL_TRACE5   = 1 << 5 | 1 << 0
    , LEVEL_TRACE4   = 1 << 5 | 1 << 1
    , LEVEL_TRACE3   = 1 << 5 | 1 << 2
    , LEVEL_TRACE2   = 1 << 5 | 1 << 3
    , LEVEL_TRACE1   = 1 << 5 | 1 << 4
    , LEVEL_TRACE    = 1 << 5
    , LEVEL_DEBUG    = 1 << 6
    , LEVEL_INFO     = 1 << 7
    , LEVEL_WARNING  = 1 << 8
    , LEVEL_ERROR    = 1 << 9
    , LEVEL_FATAL    = 1 << 10
    , LEVEL_ALERT    = 1 << 11
    , LEVEL_LOG      = 1 << 12
    , LEVEL_NO_DEBUG = LEVEL_INFO | LEVEL_WARNING | LEVEL_ERROR | LEVEL_FATAL | LEVEL_ALERT | LEVEL_LOG
    , LEVEL_NO_TRACE = LEVEL_NO_DEBUG | LEVEL_DEBUG
    , LEVEL_LOG_ALL  = LEVEL_NO_TRACE | LEVEL_TRACE1 | LEVEL_TRACE2 | LEVEL_TRACE3 |
                       LEVEL_TRACE4   | LEVEL_TRACE5
};

} // namespace utxx

#include <utxx/logger/logger.hpp>

#endif // _UTXX_LOGGER_HPP_
