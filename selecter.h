#ifndef SELECTER_H
#define SELECTER_H

#include "sentence.h"
#include <string>
#include <unicode/regex.h>

NAMESPACE_START

/**@struct selecter
 * @brief Tells if a sentence match a set of criterions.
 *
 * Users should basically set their criterions using the different member
 * functions, then run matches(), passing it the right sentence. */
struct selecter
{
    /**@brief Construct a selecter */
    selecter();

    /**@brief Destructs a selecter */
    ~selecter();

    /**@brief Set the list of characters which must be present in the sentence
     *
     * If a sentence does contain none of the characters passed here, then
     * the matches() member function will return false when called on it.
     *
     * @return SUCCESS on success, INVALID_ARG if _characters is null.
     * @param[in] _characters A null-terminated set of characters that have to
     *            be present in the sentence.
     * @pre _characters != nullptr */
    int setMustContainCharacters( const char * _characters );

    /**@brief Set the list of characters which must be present in the sentence
     *
     * If a sentence does contain none of the characters passed here, then
     * the matches() member function will return false when called on it.
     *
     * @return SUCCESS on success;
     * @param[in] _characters A string containing the characters that have to
     *            be present in the sentence.
     * @pre _characters != nullptr */
    int setMustContainCharacters( const std::string & _characters );

    /**@brief Set the list of characters that can appear in the sentence
     *
     * Determines which characters are allowed to appear in the sentence.
     * @param[in] _characters If nullptr is passed, any characters are allowed.
     *            if a null-terminated char array is passed, a sentence that
     *            contains any other character will be discarded.
     * @return SUCCESS on success, INVALID_ARG if _characters is null.
     * */
    int canContainCharacters( const char * _characters );

    /**@brief Checks whether the sentence matches the country code
     * @param[in] _countryCode The country code to match 
     * @return SUCCESS on success */
    int hasCountryCode( const char _countryCode[5] );

    /**@brief Checks whether the sentence matches a given regular expression
     * @param[in] _regex The regular expression
     * @return SUCCESS on success, INVALID_ARG if the regular expression is not valid, OUT_OF_MEMORY if there is no memory */
    int matchRegularExpression( const std::string & _regex );

    /**@brief The string to check
     * @return SUCCESS if the string matches, DOES_NOT_MATCH if it does not, INTERNAL_ERROR on error */
    int matches( const sentence & _stringToCheck );

private:
    UnicodeString   m_compulsoryCharacters; // characters that HAVE to be in the sentence
    UnicodeString * m_allowedCharacters; //characters that may appear in the sentence
    char            m_hasCountryCode[5]; //the country code to matchmo
    RegexMatcher  * m_regularExpression;
};

// __________________________________________________________________________ //

inline
int selecter::setMustContainCharacters( const std::string & _characters )
{
    return setMustContainCharacters( _characters.c_str() );
}

NAMESPACE_END

#endif //SELECTER_H
