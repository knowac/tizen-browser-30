/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef BROWSER_ASSERT_HPP_INCLUDED
#define BROWSER_ASSERT_HPP_INCLUDED 1

#ifndef NDEBUG
/*! \brief Run-time assertion tester.
 *
 * This macro has meaning only in debug build.
 *
 * \param condition - condition to be tested.
 * \post Normal progam flow is continued only if condition is met, otherwise EFailedAssertion exception is thrown.
 */
#   define M_ASSERT( condition ) do { if ( ! ( condition ) ) tizen_browser::debug::failed_assert( __FILE__, __LINE__, __PRETTY_FUNCTION__, #condition ); } while ( 0 )
#else /* #ifndef NDEBUG */
#   define M_ASSERT( c ) /**/
#endif /* #else #ifndef NDEBUG */

namespace tizen_browser
{
namespace debug
{

/*! \brief Failed assertion exception.
 *
 * In \e DEBUG build failuers in assertions does not abort the
 * process, insead the throw instance of EFailedAssertion.
 */
class EFailedAssertion
{
    char const * _what;
public:
    EFailedAssertion(char const * const what_) : _what(what_) {}
    EFailedAssertion(EFailedAssertion const & fa) : _what(fa._what) {}
    EFailedAssertion & operator = (EFailedAssertion const & fa) {
        if (&fa != this) {
            EFailedAssertion n(fa);
            swap(n);
        }
        return (*this);
    }
    char const * what(void) const {
        return (_what);
    }
private:
    void swap(EFailedAssertion &);
};

void failed_assert(char const * const, int, char const * const, char const * const) __attribute__((__noreturn__));

} /* end namespace debug */
} /* end namespace tizen_browser */

#endif /* #ifndef BROWSER_ASSERT_HPP_INCLUDED */

