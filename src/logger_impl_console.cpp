//----------------------------------------------------------------------------
/// \file  logger_impl_console.cpp
//----------------------------------------------------------------------------
/// \brief Back-end plugin implementating console message writer for the
/// <tt>logger</tt> class.
//----------------------------------------------------------------------------
// Copyright (c) 2011 Serge Aleynikov <saleyn@gmail.com>
// Created: 2009-11-25
//----------------------------------------------------------------------------
/*
***** BEGIN LICENSE BLOCK *****

This file is part of the utxx open-source project.

Copyright (C) 2011 Serge Aleynikov <saleyn@gmail.com>

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
#include <utxx/logger/logger_impl_console.hpp>
#include <iostream>
#include <stdio.h>

namespace utxx {

static logger_impl_mgr::impl_callback_t f = &logger_impl_console::create;
static logger_impl_mgr::registrar reg("console", f);

std::ostream& logger_impl_console::dump(std::ostream& out,
    const std::string& a_prefix) const
{
    out << a_prefix << "logger." << name() << '\n'
        << a_prefix << "    stdout_levels  = " << logger::log_levels_to_str(m_stdout_levels) << '\n'
        << a_prefix << "    stderr_levels  = " << logger::log_levels_to_str(m_stderr_levels) << '\n'
        << a_prefix << "    show_location  = " << (m_show_location ? "true" : "false") << '\n'
        << a_prefix << "    show_indent    = " << (m_show_ident    ? "true" : "false") << '\n';
    return out;
}

bool logger_impl_console::init(const variant_tree& a_config)
    throw(badarg_error, io_error)
{
    using boost::property_tree::ptree;
    BOOST_ASSERT(this->m_log_mgr);

    ptree::const_assoc_iterator it;
    std::string s = a_config.get("logger.console.stdout_levels", "");
    m_stdout_levels = !s.empty() ? logger::parse_log_levels(s) : s_def_stdout_levels;

    s = a_config.get("logger.console.stderr_levels", "");
    m_stderr_levels = !s.empty() ? logger::parse_log_levels(s) : s_def_stderr_levels;

    m_show_location =
        a_config.get<bool>("logger.console.show_location", this->m_log_mgr->show_location());
    m_show_ident =
        a_config.get<bool>("logger.console.show_ident", this->m_log_mgr->show_ident());

    int all_levels = m_stdout_levels | m_stderr_levels;
    for(int lvl = 0; lvl < logger_impl::NLEVELS; ++lvl) {
        log_level level = logger::signal_slot_to_level(lvl);
        if ((all_levels & static_cast<int>(level)) != 0)
            this->m_log_mgr->add_msg_logger(level, msg_binder[lvl],
                on_msg_delegate_t::from_method<logger_impl_console, &logger_impl_console::log_msg>(this));
    }
    return true;
}

void logger_impl_console::log_msg(
    const log_msg_info& info, const timestamp& a_tv, const char* fmt, va_list args)
    throw(std::runtime_error)
{
    char buf[logger::MAX_MESSAGE_SIZE];

    // int len =
    logger_impl::format_message(buf, sizeof(buf), false,
                m_show_ident, m_show_location, a_tv,
                info, fmt, args);

    if (info.level() & m_stdout_levels)
        std::cout << buf << std::endl;
    else if (info.level() & m_stderr_levels)
        std::cerr << buf << std::endl;
}

} // namespace utxx
