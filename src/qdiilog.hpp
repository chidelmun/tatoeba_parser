#ifndef QDIILOG_2_0
#define QDIILOG_2_0

/** @mainpage
 * DESCRIPTION
 * -----------
 * qdiilog is a header-only library that provides facilities to log messages
 * for tracing, debugging, etc. The messages can be written on screen or on
 * file.
 *
 * SIMPLE USAGE
 * ------------
 * 5 objects are available to the user to log messages, they are:
 * - log_debug
 * - log_trace
 * - log_info
 * - log_warning
 * - log_error
 *
 * They work the same way std::cout does, for instance:
 * @code{.cpp}
 * log_warning << "foo() was passed a null pointer" << std::endl;
 * @endcode
 *
 * REDIRECTING TO A FILE
 * ---------------------
 * The objects will all log to std::cout by default, but you can change this
 * behaviour by calling setOutput(). Actually, there are 2 setOutput functions.
 * One of them is a member function that you can call on any log object, like
 * log_debug.setOutput( someOutput ), and the other one is a global function
 * that will call setOutput on every available log object.
 * Note that setOutput() takes a std::ostream reference as a parameter.
 *
 * The following snippet will redirect all output to myprogram.log
 * @code{.cpp}
 * int main()
 * {
 *   std::ofstream logstream( "myprogram.log" );
 *   setOutput( logstream );
 *
 *   int ret = foo();
 *   log_info << "foo() returned: " << ret << std::endl;
 *   return 0
 * }
 * @endcode
 *
 * Now if you want to redirect the debug level to a different file, it is also
 * possible:
 * @code{.cpp}
 * int main()
 * {
 *    // redirecting all the messages to myprogram log
 *    std::ofstream logstream( "myprogram.log" );
 *    setOutput( logstream );
 *
 *    // redirecting the debug messages to debug.log
 *    std::ofstream debug_stream( "debug.log" );
 *    log_debug.setOutput( debug_stream );
 *
 *    log_debug << "calling foo()" << std::endl; // this will go in debug.log
 *    int ret = foo();
 *    log_info << "foo() returned: " << ret << std::endl; // this goes to myprogram.log
 *
 *    return 0;
 * }
 * @endcode
 *
 * FILTERING IMPORTANT MESSAGES IN
 * -------------------------------
 *
 * You can restrict the messages output to only the important ones by calling
 * the global function setLogLevel() with one of the following parameters:
 * - Loglevel::debug
 * - Loglevel::trace
 * - Loglevel::info
 * - Loglevel::warning
 * - Loglevel::error
 * - Loglevel::disable
 *
 * If you call <c>setLogLevel( Loglevel::warning )</c>, only the error and warning
 * messages will be processed, the more detailed messages will be ignored.
 *
 * The special log level <c>Loglevel::disable</c> will disable all output. No
 * messages will be written after this has been set.
 *
 * PREPENDING YOUR MESSAGES WITH SOME CUSTOM TEXT
 * ----------------------
 * Something I like when I debug is to have every message clearly stating if
 * it is a warning, an error, or whatever. Normally, I'd prepend the warning
 * messages with some custom text, like <b>[ww]</b>, the error messages with
 * some other (say, <b>[EE]</b>), and the information messages with something
 * less visible, such as <b>[..]</b>, so that my log clearly displays what’s
 * important.
 * @verbatim
   [..] Initializing connection...
   [..] OK, connection initialized
   [..] Preparing data for transfert
   [ww] Data seems to be anormally long
   [..] Sending over data
   [..] ACK has been received
   [..] Data transfer is finished
   [EE] The server has unexpectedly closed connection
   @endverbatim
 * Here is an easy way to do that:
 * @code{.cpp}
 * log_info.setPrependText("[..] ");
 * log_warning.setPrependText("[ww] ");
 * log_error.setPrependText("[EE] ");
 * @endcode
 *
 *
 * NAMING AND NAMESPACES
 * ---------------------
 * I normally like to ditch my objects on the global namespace but some people
 * just don't. I have created a couple preprocessor directives if you want
 * to personalize the name of the objects and retrict them into a namespace. By
 * default, 5 objects are created (log_debug, log_trace, log_info, log_warning
 * and log_error), and they are thrown into the global namespace. Now if you
 * want them to be called debug, trace, info, warning and error and to belong
 * to the namespace log, what you can do is:
 * @code{.cpp}
 * #define QDIILOG_NAMESPACE log
 * #define QDIILOG_NAME_LOGGER_DEBUG debug
 * #define QDIILOG_NAME_LOGGER_TRACE trace
 * #define QDIILOG_NAME_LOGGER_INFO info
 * #define QDIILOG_NAME_LOGGER_WARNING warning
 * #define QDIILOG_NAME_LOGGER_ERROR error
 * #include "qdiilog.hpp"
 *
 * int main()
 * {
 *   log::info << "here we go!" << std::endl;
 *   return 0;
 * }
 * @endcode
 *
 * TIPS
 * ----
 * A handy feature is the possibility to disable the logging easily:
 * Instead of writing:
 * @code{.cpp}
 * if ( foo() != SUCCESS )
 * {
 *    log_warning << "problem!" << std::endl;
 * }
 * @endcode
 * You can write:
 * @code{.cpp}
 * log_warning( foo() != SUCCESS ) << "problem" << std::endl;
 * @endcode
 */

// ___________________________  INCLUDES  ____________________________________

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <assert.h>

// ___________________________ PREPROCESSOR __________________________________

// let the user defines his own namespace
#ifdef QDIILOG_NAMESPACE
#   define QDIILOG_NS_START namespace QDIILOG_NAMESPACE {
#   define QDIILOG_NS_END   }
#else
#   define QDIILOG_NS_START
#   define QDIILOG_NS_END
#endif

// let the user defines his object names
#ifndef QDIILOG_NAME_LOGGER_DEBUG
#   define QDIILOG_NAME_LOGGER_DEBUG log_debug
#endif
#ifndef QDIILOG_NAME_LOGGER_TRACE
#   define QDIILOG_NAME_LOGGER_TRACE log_trace
#endif
#ifndef QDIILOG_NAME_LOGGER_INFO
#   define QDIILOG_NAME_LOGGER_INFO log_info
#endif
#ifndef QDIILOG_NAME_LOGGER_WARNING
#   define QDIILOG_NAME_LOGGER_WARNING log_warning
#endif
#ifndef QDIILOG_NAME_LOGGER_ERROR
#   define QDIILOG_NAME_LOGGER_ERROR log_error
#endif

QDIILOG_NS_START

// -------------------------------------------------------------------------- //

/**@brief The different granularity of logging */
enum class Loglevel
{
    debug,
    trace,
    info,
    warning,
    error,

    disable
};
// -------------------------------------------------------------------------- //

/**@struct UserFilter
 * @private
 * @internal
 */
template<typename T>
struct UserFilter
{
    /**@private */
    static Loglevel level;
};

// -------------------------------------------------------------------------- //

/*
 * @private
 * @internal
 */
template<typename T>
Loglevel UserFilter<T>::level;

// -------------------------------------------------------------------------- //

inline
void setLogLevel( Loglevel _level )
{
    UserFilter<int>::level = _level;
}

// -------------------------------------------------------------------------- //

/**@struct Logger
 * @brief Logs messages
 * @author qdii
 *
 * Normally, users should not have to instantiate this class and should rather
 * use the already existing ones:
 *  - log_debug
 *  - log_trace
 *  - log_info
 *  - log_warning
 *  - log_error
 *
 * The normal usage of a class is similar to std::cout. For instance, if I
 * wanted to log the return value of a function, I could type.
 * @code{.cpp}
 * int main()
 * {
 *     const int ret = foo();
 *     log_debug << "the return value of foo() is: " << ret << std::endl;
 * }
 * @endcode
 */
template< Loglevel Level >
struct Logger : public std::ostream
{
    /**@brief Construct a logger
     * @private
     * @param[in] _muted Forbids the logger from outputting anything
     * @param[in] _decorated Whether the logger can decorate user messages
     * @warning There no reason why you would want to create a logger,
     *          you should use already existing objects: log_debug,
     *          log_trace, log_info, log_warning, log_error
     */
    explicit Logger( bool _muted = false, bool _decorated = true );

    /**@brief Construct a copy of a Logger
     * @param[in] _copy The other Logger you want to construct from
     * @warning There no reason why you would want to create a logger,
     *          you should use already existing objects: log_debug,
     *          log_trace, log_info, log_warning, log_error         */
    Logger( const Logger & _copy );

    /**@brief Assign a logger to another
     * @private */
    Logger & operator=( const Logger & _copy );

    /**@brief Destruct a Logger */
    virtual ~Logger();

    /**@brief Writes the user message in the stream after customizations
     * @private
     * @return A logger */
    virtual Logger treat( const std::string & _userMessage );

    /**@brief Checks if the logger is muted
     * @private
     * @return True if it is, false if it is not
     *
     * A muted logger does not output anything. Normal loggers should not be
     * muted.
     */
    bool isMuted() const;

    /**@brief Checks if the logger is decorated
     * @private
     * @return true if it is, false if it is not
     *
     * A decorated logger will append decoration to the user message.
     * @see Logger::prepend( const std::string &)
     */
    bool isDecorated() const;

    /**@brief Prepends all the user messages with some text.
     * @param[in] _text The text to add before the log message.
     *
     * This function lets you add a custom text before any message logged.
     * For instance, if you want all warning messages to be preceded by
     * <c>WARNING: </c>, you could call
     * @code{.cpp}log_warning.setPrependText("WARNING: "); @endcode
     * This way, whenever you’ll type a warning message, it will be preceded
     * by your text. For instance,
     * @code{.cpp}log_warning << "something odd happened\n"; @endcode
     * will write <c>WARNING: something odd happened</c>
     * @brief Adds a custom text before the message to log.
     * @param[in] _text The text to add before the log message.
     * @warning Calling setPrependText many times cancels any previously
     *          set text. */
    void prepend( const std::string & _text );

    /**@brief Changes the output of the logger
     * @param[in] _newOutput The output that will replace the former one
     *
     * Changing the output lets you decide where you want that stream to
     * output your messages. This could be a file (using std::ofstream) or
     * the console (via std::cout or std::cerr for instance) */
    void setOutput( std::ostream & _newOutput );

    /**
     * <c>operator()</c> has been rewritten to provide a handy way to log
     * under condition. This is particularly handy when you want to warn the
     * user that something went wrong without writing if and else cases.
     * Consider this example:
     * @code{.cpp}
     * if (ret != SUCCESS)
     * {
     *    log_warning << "something odd happenned\n";
     * }
     * else
     * {
     *    log_info << "entering foo()\n";
     * }
     * @endcode
     * The previous snippet can easily be rewritten:
     * @code{.cpp}
     * log_warning(ret != SUCCESS) << "something odd happenned\n";
     * log_info(ret == SUCCESS) << "entering foo()\n";
     * @endcode
     * @brief Provides a simple way to disable/enable logging
     * @return *this
     * @param[in] _condition The logging will be enabled only if
     *            _condition is true */
    Logger operator()( bool _condition );

    /**@brief Checks whether the level of details is sufficient *
     * @private
     * @return true if it is, false if it is not
     */
    bool isGranularityOk() const;

public:
    /**@brief Changes the output of all the loggers of this level
     * @private */
    static void setAllOutputs( std::ostream & _newOutput );

    /**@brief Prepends the same text to all loggers of this level
     * @param[in] _text The _text to prepend
     * @private */
    static void prependAll( const std::string & _text );

private:
    static const unsigned MUTED         = 0x00000001;
    static const unsigned DECORATED     = 0x00000002;

    std::vector<bool> m_flags;



private:
    struct Decoration
    {
        Decoration() : prependText() {}
        std::string prependText;
    };

    Decoration * m_decoration;

private:
    // this section permits configuring things that apply to all the loggers
    static std::vector<Logger *> * m_allLoggers;
    static void registerMe( Logger & _logger )
    {
        if( !m_allLoggers )
            m_allLoggers = new std::vector<Logger *>();

        m_allLoggers->push_back( &_logger );
    }

    static void unregisterMe( Logger & _logger )
    {
        m_allLoggers->erase(
            std::find(
                m_allLoggers->begin(),
                m_allLoggers->end(),
                &_logger
            ),
            m_allLoggers->end()
        );

        if( m_allLoggers->empty() )
        {
            delete m_allLoggers;
            m_allLoggers = nullptr;
        }
    }
};

// -------------------------------------------------------------------------- //

template<Loglevel Level>
Logger<Level>::Logger( bool _muted, bool _decorated )
    :std::basic_ios<char>()
    ,std::basic_ostream<char>()
    ,m_flags()
    ,m_decoration( nullptr )
{
    m_flags.reserve( 3 );
    m_flags[MUTED] = _muted;
    m_flags[DECORATED] = _decorated;

    registerMe( *this );
}

// -------------------------------------------------------------------------- //

template<Loglevel Level>
Logger<Level>::Logger( const Logger & _copy )
    :std::basic_ios<char>()
    ,std::basic_ostream<char>()
    ,m_flags( _copy.m_flags )
    ,m_decoration( nullptr )
{
    setOutput( const_cast<Logger &>( _copy ) );

    registerMe( *this );
}

// -------------------------------------------------------------------------- //

template<Loglevel Level>
std::vector<Logger<Level>*>* Logger<Level>::m_allLoggers;

// -------------------------------------------------------------------------- //

template<Loglevel Level> inline
bool Logger<Level>::isMuted() const
{
    return m_flags[MUTED];
}

// -------------------------------------------------------------------------- //

template<Loglevel Level> inline
bool Logger<Level>::isDecorated() const
{
    return m_flags[DECORATED];
}

// -------------------------------------------------------------------------- //

template<Loglevel Level>
Logger<Level> Logger<Level>::treat( const std::string & _userMessage )
{
    if( !isMuted() && isGranularityOk() )
    {
        std::string fullMessage;

        if( isDecorated() && m_decoration )
        {
            fullMessage += m_decoration->prependText;
        }

        fullMessage += _userMessage;

        static_cast<std::ostream &>( *this ) << fullMessage;
    }

    Logger returnedLogger( isMuted(), false );
    returnedLogger.setOutput( *this );

    return returnedLogger;
}



// -------------------------------------------------------------------------- //

template<Loglevel Level>
void Logger<Level>::prepend( const std::string & _text )
{
    if( !m_decoration )
        m_decoration = new Logger::Decoration();

    if( m_decoration )
        m_decoration->prependText = _text;
}

// -------------------------------------------------------------------------- //

template<Loglevel Level>
Logger<Level> & Logger<Level>::operator=( const Logger<Level> & _copy )
{
    if( this == &_copy )
        return *this;

    Decoration * const newDecoration =
        _copy.m_decoration ? new Decoration : nullptr;

    try
    {
        m_flags.reserve( _copy.m_flags.size() );
    }
    catch( std::bad_alloc & e )
    {
        delete newDecoration;
        throw;
    }

    if( newDecoration )
    {
        try
        {
            newDecoration->prependText.reserve(
                _copy.m_decoration->prependText.size()
            );
        }
        catch( std::bad_alloc & e )
        {
            delete newDecoration;
            throw;
        }
    }

    // if we made it to this point, then all memory has been allocated. yippi
    // we can proceed to the copy itself
    if( newDecoration )
        newDecoration->prependText = _copy.m_decoration->prependText;

    m_flags = _copy.m_flags;

    delete m_decoration;
    m_decoration = newDecoration;

    /**@todo Transférer le output aussi */
    setOutput( const_cast<Logger &>( _copy ) );

    return *this;
}

// -------------------------------------------------------------------------- //

template<Loglevel Level>
Logger<Level>::~Logger()
{
    delete m_decoration;
    unregisterMe( *this );
}

// -------------------------------------------------------------------------- //

// hack to catch std::endl;
typedef std::basic_ostream<char, std::char_traits<char> > CoutType;
typedef CoutType & ( *StandardEndLine )( CoutType & );

template<Loglevel Level>
Logger<Level> && operator<<( Logger<Level> & _logger, StandardEndLine _endl )
{
    return std::move( _logger ) << _endl;
}

template<Loglevel Level>
Logger<Level> && operator<<( Logger<Level> && _logger, StandardEndLine _endl )
{
    if( !_logger.isMuted() && _logger.isGranularityOk() )
    {
        _endl( _logger );
    }

    return std::move( _logger );
}

// -------------------------------------------------------------------------- //

/**@brief Stream out an user message
 * @param[in] _logger The logger which will take care of the user message
 * @param[in] _message The message of the user.
 * @return A logger (which might be different  */
template<Loglevel Level, typename UserMessage>
Logger<Level> operator<<( Logger<Level> & _logger, const UserMessage & _message )
{
    return std::move( _logger ) << _message;
}

// -------------------------------------------------------------------------- //

/**@brief Stream out an user message
 * @param[in] _logger The logger which will take care of the user message
 * @param[in] _message The message of the user.
 * @return A logger (which might be different  */
template<Loglevel Level, typename UserMessage>
Logger<Level> operator<<( Logger<Level> && _logger, const UserMessage & _message )
{
    if( !_logger.isMuted() )
    {
        std::ostringstream istr;
        istr << _message;
        return _logger.treat( istr.str() );
    }

    return _logger;
}

// -------------------------------------------------------------------------- //

/**@brief Changes the output of all the loggers of this level */
template<Loglevel Level>
void Logger<Level>::setAllOutputs( std::ostream & _newOutput )
{
    const auto end = m_allLoggers->end();

    for( typename std::vector<Logger *>::iterator logger = m_allLoggers->begin();
            logger != end; ++logger )
    {
        ( *logger )->setOutput( _newOutput );
    }
}

// -------------------------------------------------------------------------- //

/**@brief Changes the output of all the loggers of this level */
template<Loglevel Level>
void Logger<Level>::prependAll( const std::string & _text )
{
    const auto end = m_allLoggers->end();

    for( typename std::vector<Logger *>::iterator logger = m_allLoggers->begin();
            logger != end; ++logger )
    {
        ( *logger )->prepend( _text );
    }
}

// -------------------------------------------------------------------------- //

template<Loglevel Level>
void Logger<Level>::setOutput( std::ostream & _newOutput )
{
    //assert( _newOutput.rdbuf() );
    std::ostream::rdbuf( _newOutput.rdbuf() );
}

// -------------------------------------------------------------------------- //

template<Loglevel Level>
bool Logger<Level>::isGranularityOk() const
{
    bool doesLevelAllowLogging = false;

    switch( Level )
    {
    case Loglevel::error:
        doesLevelAllowLogging |= ( UserFilter<int>::level == Loglevel::error );

    case Loglevel::warning:
        doesLevelAllowLogging |= ( UserFilter<int>::level == Loglevel::warning );

    case Loglevel::info:
        doesLevelAllowLogging |= ( UserFilter<int>::level == Loglevel::info );

    case Loglevel::trace:
        doesLevelAllowLogging |= ( UserFilter<int>::level == Loglevel::trace );

    case Loglevel::debug:
        doesLevelAllowLogging |= ( UserFilter<int>::level == Loglevel::debug );
        break;

    case Loglevel::disable:
        doesLevelAllowLogging = false;
        break;
    }

    return doesLevelAllowLogging;
}

// -------------------------------------------------------------------------- //

template<Loglevel Level>
Logger<Level> Logger<Level>::operator()( bool _condition )
{
    Logger<Level> logger( !_condition, true );

    if( _condition )
    {
        logger = *this;
    }

    return logger;
}

// -------------------------------------------------------------------------- //

inline
void setOutput( std::ostream & _newOutput )
{
    Logger<Loglevel::debug>::setAllOutputs( _newOutput );
    Logger<Loglevel::trace>::setAllOutputs( _newOutput );
    Logger<Loglevel::info>::setAllOutputs( _newOutput );
    Logger<Loglevel::warning>::setAllOutputs( _newOutput );
    Logger<Loglevel::error>::setAllOutputs( _newOutput );
}

// -------------------------------------------------------------------------- //

static
Logger<Loglevel::debug> QDIILOG_NAME_LOGGER_DEBUG ;

static
Logger<Loglevel::trace> QDIILOG_NAME_LOGGER_TRACE ;

static
Logger<Loglevel::info> QDIILOG_NAME_LOGGER_INFO ;

static
Logger<Loglevel::warning> QDIILOG_NAME_LOGGER_WARNING ;

static
Logger<Loglevel::error> QDIILOG_NAME_LOGGER_ERROR ;

// -------------------------------------------------------------------------- //

inline
void setPrependTextQdiiFlavour()
{
    QDIILOG_NAME_LOGGER_DEBUG   .prependAll( "" );
    QDIILOG_NAME_LOGGER_TRACE   .prependAll( "" );
    QDIILOG_NAME_LOGGER_INFO    .prependAll( "[..] " );
    QDIILOG_NAME_LOGGER_WARNING .prependAll( "[ww] " );
    QDIILOG_NAME_LOGGER_ERROR   .prependAll( "[EE] " );
}

// -------------------------------------------------------------------------- //

enum BashColor
{
    NONE = 0,
    BLACK, RED, GREEN,
    YELLOW, BLUE, MAGENTA,
    CYAN, WHITE
};

// -------------------------------------------------------------------------- //

static
std::string setBashColor( BashColor _foreground = NONE,
                          BashColor _background = NONE,
                          bool _bold = false )
{
    std::ostringstream transform;
    transform << ( _bold ? "\033[1m" : "\033[0m" );

    if( _foreground != NONE || _background != NONE )
    {
        transform << "\033[";

        if( _foreground != NONE )
        {
            transform << 29 + _foreground;

            if( _background ) transform << ";";
        }

        if( _background != NONE )
        {
            transform << 39 + _background;
        }

        transform << "m";
    }

    return transform.str();
}

// -------------------------------------------------------------------------- //

inline
void setPrependedTextQdiiFlavourBashColors()
{
    QDIILOG_NAME_LOGGER_DEBUG   .prependAll( setBashColor( NONE, NONE, false ) );
    QDIILOG_NAME_LOGGER_TRACE   .prependAll( setBashColor( NONE, NONE, false ) );
    QDIILOG_NAME_LOGGER_INFO    .prependAll( setBashColor( NONE, NONE, false ) + "[..] " );
    QDIILOG_NAME_LOGGER_WARNING .prependAll( std::string( "[" ) + setBashColor( GREEN ) + "ww" + setBashColor( NONE ) + "] " );
    QDIILOG_NAME_LOGGER_ERROR   .prependAll( std::string( "[" ) + setBashColor( RED ) + "EE" + setBashColor( NONE ) + "]" + setBashColor( NONE, NONE, true ) + " " );
}

QDIILOG_NS_END

#endif //QDILOG_2_0