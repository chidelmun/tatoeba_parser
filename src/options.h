#ifndef TATOPARSER_OPTIONS_H
#define TATOPARSER_OPTIONS_H

#include <iostream>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include "filter.h"

NAMESPACE_START

struct dataset;
struct linkset;
struct tagset;

/**@struct userOptions
 * @brief Treats the user-passed parameters (--help and such)
 */
struct userOptions
{
    /**@brief Constructs an userOptions object */
    userOptions();

    /**@brief Treat the command line arguments argc and argv, populate objects. */
    void treatCommandLine( int argc, char * argv[] );

    /**@brief Populate a list of filters according to the user-specified parameters
     * @param[in] _dataset The list of sentences
     * @param[in] _linkset The list of links
     * @param[in] _tagset The list of tags
     * @param[out] allFilters_ The filter list that will be filled in by the call */
    void getFilters( dataset & _dataset, linkset & _linkset, tagset & _tagset,
                        FilterVector & allFilters_);

    /**@brief Checks if any argument the user specified needs the links.csv to be parsed */
    bool isItNecessaryToParseLinksFile() const;

    /**@brief Checks if any argument the user specified needs the tags.csv to be parsed */
    bool isItNecessaryToParseTagFile() const;

    /**@brief Has -v been specified? */
    bool isVerbose() const;

    /**@brief Was --help set? */
    bool isHelpRequested() const;

    /**@brief Was --version set? */
    bool isVersionRequested() const;

    /**@brief Should we enumerate lines (like the -n option of 'cat')? */
    bool displayLineNumbers() const;

    /**@brief Should we display the id of the matching sentences? */
    bool displayIds() const;

    /**@brief This is debug, only the parsing is done */
    bool justParse() const;

public:
    void printHelp();
    void printVersion();

    std::string getCsvPath() const;

private:
    boost::program_options::options_description m_desc;
    boost::program_options::variables_map       m_vm;
};

inline
bool userOptions::isItNecessaryToParseTagFile() const
{
    return m_vm.count( "has-tag" );
}

// -------------------------------------------------------------------------- //

inline
bool userOptions::isItNecessaryToParseLinksFile() const
{
    return  m_vm.count( "is-linked-to" ) ||
            m_vm.count( "translation-regex" ) ||
            m_vm.count( "is-translatable-in" );
}

// -------------------------------------------------------------------------- //

inline
bool userOptions::isVerbose() const
{
    return m_vm.count( "verbose" );
}

// -------------------------------------------------------------------------- //

inline
bool userOptions::isHelpRequested() const
{
    return m_vm.count( "help" );
}

// -------------------------------------------------------------------------- //

inline
bool userOptions::displayLineNumbers() const
{
    return m_vm.count( "display-line-numbers" );
}

// -------------------------------------------------------------------------- //

inline
bool userOptions::displayIds() const
{
    return m_vm.count( "display-ids" );
}

// -------------------------------------------------------------------------- //

inline
bool userOptions::isVersionRequested() const
{
    return m_vm.count( "version" );
}

// -------------------------------------------------------------------------- //

inline
void userOptions::printHelp()
{
    std::cout << "Usage: " << PACKAGE_NAME << " <OPTION>\nWhere OPTION can be any of:\n";
    std::cout << m_desc << std::endl;
}

// -------------------------------------------------------------------------- //

inline
std::string userOptions::getCsvPath() const
{
    return m_vm.count( "csv-path" ) ? m_vm["csv-path"].as<std::string>() : ".";
}

// -------------------------------------------------------------------------- //

inline
bool userOptions::justParse() const
{
    return m_vm.count( "just-parse" );
}

// -------------------------------------------------------------------------- //

NAMESPACE_END
#endif //TATOPARSER_OPTIONS_H
