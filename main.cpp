#include "prec.h"
#include "parser.h"
#include "selecter.h"
#include "filter_regex.h"
#include "filter_language.h"
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/cmdline.hpp>
#include <boost/program_options/parsers.hpp>
#include <iostream>

namespace po = boost::program_options;

NAMESPACE_START

cout_warning * gs_coutWarning;

NAMESPACE_END
USING_NAMESPACE

int addRegularExpressionFilter( const std::string & _regex, Filter * & filter_ );

int main( int argc, char * argv[] )
{
    // initializing outputs
    cout_warning warning_output;
    gs_coutWarning = & warning_output;

    // command line parameters
    po::options_description desc( "Allowed options" );
    desc.add_options()
    ( "help,h", "produce help message" )
    ( "compulsory,c", po::value<std::string>(), "The characters that should appear in the sentence" )
    ( "optional,o", po::value<std::string>(), "The characters that may compose the sentence" )
    ( "line-numbers,n", "Display the indexes of the lines" )
    ( "language,l", po::value<std::string>(), "Restrict the languages to a given one" )
    ( "display-ids,i", "Displays the sentence ids" )
    ( "separator,s", po::value<char>(), "Changes the separator characters, default is '\\t'" )
    ( "regex,r", po::value<std::string>(), "A regular expression that the sentence should match entirely" )
    ( "verbose,v", "Displays warnings" )
    ;

    po::variables_map vm;
    po::store( po::parse_command_line( argc, argv, desc ), vm );
    po::notify( vm );

    if( vm.count( "help" ) )
    {
        std::cout << desc << "\n";
        return 1;
    }

    if( !vm.count( "verbose" ) )
        VERIFY_EQ( gs_coutWarning->mute(), SUCCESS );

    // parse the file
    const std::string filename( "sentences.csv" );
    parser tatoeba_parser( filename );

    dataset allSentences;
    VERIFY_EQ( tatoeba_parser.setOutput( allSentences ),    SUCCESS );

    // ###### SET UP THE FILTERS #####
    selecter sel;
    std::vector< Filter * > filtersToDelete;

    // LANGUAGE FILTER
    if( vm.count( "language" ) )
    {
        FilterLanguage * const filter = new FilterLanguage( std::move( vm["language"].as<std::string>() ) );

        if( filter )
        {
            filtersToDelete.push_back( filter );
            VERIFY_EQ( sel.addFilter( *filter ), SUCCESS );
        }
        else
        {
            ERR << "Out of memory\n";
            return 1;
        }
    }

    // OPTIONAL CHARACTERS
    if( vm.count( "optional" ) )
    {
        Filter * filter = nullptr;

        if( addRegularExpressionFilter( std::string( "[" + vm["optional"].as<std::string>() + "]*" ), filter ) == SUCCESS )
        {
            ASSERT( filter != nullptr );
            filtersToDelete.push_back( filter );
            VERIFY_EQ( sel.addFilter( *filter ), SUCCESS );
        }
        else
            return 1;
    }

    // MANDATORY CHARACTERS
    if( vm.count( "compulsory" ) )
    {
        Filter * filter = nullptr;

        if( addRegularExpressionFilter( std::string( ".*[" + vm["compulsory"].as<std::string>() + "]+.*" ), filter ) == SUCCESS )
        {
            ASSERT( filter != nullptr );
            filtersToDelete.push_back( filter );
            VERIFY_EQ( sel.addFilter( *filter ), SUCCESS );
        }
        else
            return 1;
    }


    // USER DEFINED REGEX
    if( vm.count( "regex" ) )
    {
        Filter * filter = nullptr;

        if( addRegularExpressionFilter( std::string( vm["regex"].as<std::string>() ), filter ) == SUCCESS )
        {
            ASSERT( filter != nullptr );
            filtersToDelete.push_back( filter );
            VERIFY_EQ( sel.addFilter( *filter ), SUCCESS );
        }
        else
            return 1;
    }

    // START PARSING
    const int parseSuccess = tatoeba_parser.start();

    if( parseSuccess == -1 )
        ERR << "Unable to open \"sentences.csv\"\n";
    else
    {
        WARN << "OK, parsed " << allSentences.size() << " sentences\n";

        // ##### The actual parsing ####
        ux lineNumber = 0;
        const char separator = vm.count( "separator" ) ? vm["separator"].as<char>() : '\t';

        for( auto sentence : allSentences )
        {
            const int matchResult = sel.matches( sentence );

            switch( matchResult )
            {
            case SUCCESS:
                if( vm.count( "line-numbers" ) )
                    std::cout << lineNumber++ << separator;

                if( vm.count( "display-ids" ) )
                    std::cout << sentence.getId() << separator;

                std::cout << sentence << "\n";
                break;

            case INTERNAL_ERROR:
                WARN << "[**] Error while parsing sentence: " << sentence << "\n";
                break;

            default:
                break;
            }
        }
    }
    // clean up
    for( auto filter : filtersToDelete )
    {
        delete filter;
    }

    return 0;
}

int addRegularExpressionFilter( const std::string & _regex, Filter *& filter_ )
{
    int ret = SUCCESS;

    FilterRegex * const regex = new FilterRegex( _regex );

    if( regex != nullptr )
    {
        if( !regex->isRegexValid() )
        {
            ERR << "Invalid regular expression\n";
            ret = INVALID_ARG;
        }
        else
            filter_ = regex;
    }
    else
    {
        ERR << "Out of memory\n";
        ret = OUT_OF_MEMORY;
    }

    return ret;
}
