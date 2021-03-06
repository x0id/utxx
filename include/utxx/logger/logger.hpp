//----------------------------------------------------------------------------
/// \file   logger.hpp
/// \author Serge Aleynikov
//----------------------------------------------------------------------------
/// \brief Light-weight logging of messages to configured targets.
///
/// The client supports pluggable back-ends that can write messages
/// synchronously and asyncronously to different targets.  Unsafe printf() like 
/// interface of the logger (vs. stream-based) is chosen primarily for 
/// simplisity and speed. 
/// The idea behind the logger is hargely based on the asynchronous logging 
/// project called LAMA in the jungerl.sf.net repository.
///
/// The following back-end logger plugins are currently implemented:
///  * console writer
///  * file writer
///  * asynchronous file writer
///  * syslog writer
//----------------------------------------------------------------------------
// Copyright (C) 2003-2009 Serge Aleynikov <saleyn@gmail.com>
// Created: 2009-11-25
//----------------------------------------------------------------------------
/*
***** BEGIN LICENSE BLOCK *****

This file is part of the utxx open-source project.

Copyright (C) 2009 Serge Aleynikov <saleyn@gmail.com>

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
#ifndef _UTXX_MAIN_LOGGER_HPP_
#define _UTXX_MAIN_LOGGER_HPP_

#include <stdarg.h>
#include <stdio.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/exceptions.hpp>
#include <boost/function.hpp>
#include <boost/scope_exit.hpp>
//#include <utxx/singleton.hpp>
#include <boost/thread/mutex.hpp>
#include <utxx/logger/logger_impl.hpp>
#ifndef _MSC_VER
#   include <utxx/synch.hpp>
#   include <utxx/high_res_timer.hpp>
#   include <utxx/timestamp.hpp>
#endif

namespace utxx { 

#ifdef _MSC_VER

#define LOG_TRACE5(FmtArgs)  
#define LOG_TRACE4(FmtArgs)
#define LOG_TRACE3(FmtArgs)
#define LOG_TRACE2(FmtArgs)
#define LOG_TRACE1(FmtArgs)
#define LOG_DEBUG(FmtArgs)      printf FmtArgs;
#define LOG_INFO(FmtArgs)       printf FmtArgs;    
#define LOG_WARNING(FmtArgs)    printf FmtArgs;
#define LOG_ERROR(FmtArgs)      printf FmtArgs;
#define LOG_FATAL(FmtArgs)      printf FmtArgs;
#define LOG_ALERT(FmtArgs)      printf FmtArgs;

#else

/// In all <LOG_*> macros <FmtArgs> are parameter lists with signature of
/// the <printf> function: <(const char* fmt, ...)>
#ifndef UTXX_SKIP_LOG_MACROS
typedef log_msg_info _lim;
#define LOG_TRACE5(FmtArgs)  do { \
    utxx::_lim(utxx::logger::instance(), utxx::LEVEL_TRACE5 , \
        __FILE__, __LINE__).log FmtArgs; } while(0)
#define LOG_TRACE4(FmtArgs)  do { \
    utxx::_lim(utxx::logger::instance(), utxx::LEVEL_TRACE4 , \
        __FILE__, __LINE__).log FmtArgs; } while(0)
#define LOG_TRACE3(FmtArgs)  do { \
    utxx::_lim(utxx::logger::instance(), utxx::LEVEL_TRACE3 , \
        __FILE__, __LINE__).log FmtArgs; } while(0)
#define LOG_TRACE2(FmtArgs)  do { \
    utxx::_lim(utxx::logger::instance(), utxx::LEVEL_TRACE2 , \
        __FILE__, __LINE__).log FmtArgs; } while(0)
#define LOG_TRACE1(FmtArgs)  do { \
    utxx::_lim(utxx::logger::instance(), utxx::LEVEL_TRACE1 , \
        __FILE__, __LINE__).log FmtArgs; } while(0)
#define LOG_DEBUG(FmtArgs)   do { \
    utxx::_lim(utxx::logger::instance(), utxx::LEVEL_DEBUG  , \
        __FILE__, __LINE__).log FmtArgs; } while(0)
#define LOG_INFO(FmtArgs)    do { \
    utxx::_lim(utxx::logger::instance(), utxx::LEVEL_INFO   , \
        __FILE__, __LINE__).log FmtArgs; } while(0)
#define LOG_WARNING(FmtArgs) do { \
    utxx::_lim(utxx::logger::instance(), utxx::LEVEL_WARNING, \
        __FILE__, __LINE__).log FmtArgs; } while(0)
#define LOG_ERROR(FmtArgs)   do { \
    utxx::_lim(utxx::logger::instance(), utxx::LEVEL_ERROR  , \
        __FILE__, __LINE__).log FmtArgs; } while(0)
#define LOG_FATAL(FmtArgs)   do { \
    utxx::_lim(utxx::logger::instance(), utxx::LEVEL_FATAL  , \
        __FILE__, __LINE__).log FmtArgs; } while(0)
#define LOG_ALERT(FmtArgs)   do { \
    utxx::_lim(utxx::logger::instance(), utxx::LEVEL_ALERT  , \
        __FILE__, __LINE__).log FmtArgs; } while(0)
#endif

#endif

/// Logging class that supports pluggable back-ends responsible for handling
/// log messages. Examples of backends are implemented in the logger_impl_console,
/// logger_impl_file, logger_impl_async_file classes.
class logger : boost::noncopyable {
public:
    static const char* log_level_to_str(log_level level);
    static std::string log_levels_to_str(int a_levels);
    /// Convert a <level> to the slot number in the <m_sig_msg> array
    static int         level_to_signal_slot(log_level level) throw(badarg_error);
    /// Convert a <level> to the slot number in the <m_sig_msg> array
    static log_level   signal_slot_to_level(int slot) throw(badarg_error);

    typedef std::vector<logger_impl*>      implementations_vector;

    typedef logger_impl::on_msg_delegate_t on_msg_delegate_t;
    typedef logger_impl::on_bin_delegate_t on_bin_delegate_t;

private:
    event_source<on_msg_delegate_t> m_sig_msg[logger_impl::NLEVELS];
    event_source<on_bin_delegate_t> m_sig_bin;
    unsigned int                    m_level_filter;
    implementations_vector          m_implementations;
    stamp_type                      m_timestamp_type;
    char                            m_src_location[256];
    bool                            m_show_location;
    bool                            m_show_ident;
    std::string                     m_ident;

    boost::function<void (const char* reason)> m_error;

    /// @return <true> if log <level> is enabled.
    bool is_enabled(log_level level) const {
        return (m_level_filter & (unsigned int)level) != 0;
    }

    void set_timestamp(char* buf, time_t seconds) const;

public:
    static logger& instance() {
        return singleton<logger>::instance();
    }

    enum defaults {
        MAX_MESSAGE_SIZE = 512
    };

    logger() 
        : m_level_filter(LEVEL_NO_DEBUG), m_timestamp_type(TIME)
        , m_show_location(true), m_show_ident(false)
    {}
    ~logger() { finalize(); }

    enum init_file_type { INFO_FILE, JSON_FILE, XML_FILE };

    /// Call to initialize the logger by reading configuration from file.
    /// Supported file formats: {info, json, xml}. See boost/property_tree
    /// library for more information about format details.
    void init(const char* filename, init_file_type type = INFO_FILE);

    /// Call to initialize the logger from a configuration container.
    void init(const variant_tree& config);

    /// Called on destruction/reinitialization of the logger.
    void finalize();

    /// Set program identifier to be used in the log output.
    void set_ident(const char* ident) { m_ident = ident; }

    /// Set filter mask to allow only selected log_levels to be included
    /// in the log.
    void set_level_filter(log_level a_level);

    /// Set filter mask to allow log_levels above or equal to \a a_level
    /// to be included in the log.
    void set_min_level_filter(log_level a_level);

    /// Set an error handler delegate to fire when there is an error
    /// instead of throwing run-time exceptions.  Note that the handler
    /// may be called from different threads so it has to be thread-safe.
    void set_error_handler(boost::function<void (const char*)>& eh) {
        m_error = eh;
    }

    /// Set the timestamp type to use in log files.
    /// @param ts the timestamp type.
    void timestamp_type(stamp_type ts) { m_timestamp_type = ts; }

    /// @return format type of timestamp written to log
    stamp_type timestamp_type() const { return m_timestamp_type; }

    /// @return true if ident display is enabled by default.
    bool show_ident()          const { return m_show_ident; }
    /// @return true if source location display is enabled by default.
    bool show_location()       const { return m_show_location; }

    /// Get program identifier to be used in the log output.
    const std::string& ident() const { return m_ident; }

    /// To be called by <logger_impl> child to register a delegate to be
    /// invoked on a call to LOG_*() macros.
    void add_msg_logger(log_level level,
            event_binder<on_msg_delegate_t>& binder, on_msg_delegate_t subscriber);
    /// To be called by <logger_impl> child to register a delegate to be
    /// invoked on a call to logger::log(msg, size).
    void add_bin_logger(event_binder<on_bin_delegate_t>& binder, on_bin_delegate_t subscriber);

    /// Converts a string (e.g. "DEBUG | INFO | WARNING") sizeof(m_timestamp)-1to a bitmask of
    /// corresponding levels.  This method is used for configuration parsing
    static int parse_log_levels(const std::string& levels) throw(std::runtime_error);
    /// String representation of log levels enabled by default.  Used in config
    /// parsing.
    static const char* default_log_levels; 

    /// @return vector of active back-end logging implementations
    const implementations_vector&  implementations() const;

    /// Dump internal settings
    std::ostream& dump(std::ostream& out) const;

    /// Signal info/warning/error/fatal/alert level message to registered
    /// implementations.  Use the provided <LOG_*> macros instead of calling it directly.
    /// @param a_level is the log level.
    /// @param filename is the content of __FILE__ variable.
    /// @param line is the content of __LINE__ variable.
    /// @param fmt is the format string passed to <sprintf()>
    /// @param args is the list of optional arguments passed to <args>
    template <int N>
    static void log(logger& a_logger, log_level a_level,
        const char (&filename)[N], size_t line,
        const char* fmt, va_list args)
    {
        log_msg_info info(a_logger, a_level, filename, line);
        a_logger.log(info, fmt, args);
    }

    /// Signal info/warning/error/fatal/alert level message to registered
    /// implementations.  Use the provided <LOG_*> macros instead of calling it directly.
    /// @param info is an object containing log level, and msg source location.
    /// @param fmt is the format string passed to <sprintf()>
    /// @param args is the list of optional arguments passed to <args>
    void log(const log_msg_info& info, const char* fmt, va_list args) {
        if (is_enabled(info.level()))
            try {
                timestamp l_ts;
                l_ts.update();
                m_sig_msg[level_to_signal_slot(info.level())](
                    on_msg_delegate_t::invoker_type(info, l_ts, fmt, args));
            } catch (std::runtime_error& e) {
                if (m_error)
                    m_error(e.what());
                else
                    throw;
            }
    }

    /// Signal binary message to registered implementations using <LEVEL_LOG> level.
    /// @param buf is message buffer.
    /// @param size is the size of the message.
    void log(const char* buf, size_t size) {
        if (likely(is_enabled(LEVEL_LOG)))
            try {
                m_sig_bin(on_bin_delegate_t::invoker_type(buf, size));
            } catch (std::runtime_error& e) {
                if (m_error)
                    m_error(e.what());
                else
                    throw;
            }
    }
};

} // namespace utxx

#include <utxx/logger/logger_impl.ipp> // Logger implementation

#endif  // _UTXX_MAIN_LOGGER_HPP_
