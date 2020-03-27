/*
        Copyright 2019 Intel Corporation.
        This software and the related documents are Intel copyrighted materials,
        and your use of them is governed by the express license under which they
        were provided to you (End User License Agreement for the Intel(R) Software
        Development Products (Version May 2017)). Unless the License provides
        otherwise, you may not use, modify, copy, publish, distribute, disclose or
        transmit this software or the related documents without Intel's prior
        written permission.

        This software and the related documents are provided as is, with no
        express or implied warranties, other than those that are expressly
        stated in the License.
*/

#ifndef _INTEL_OPENVX_SAMPLE_BASIC_HPP_
#define _INTEL_OPENVX_SAMPLE_BASIC_HPP_


#include <cstdlib>
#include <cassert>
#include <string>
#include <stdexcept>
#include <sstream>
#include <typeinfo>
#include <cstdlib>
#include <algorithm>
#include <iomanip>
#include <exception>
#include <iostream>

using std::cerr;
using std::string;


// Base class for all exception in samples
class SampleError : public std::runtime_error
{
public:
    SampleError (const string& msg) :
        std::runtime_error(msg)
    {
    }
};


// Allocates piece of aligned memory
// alignment should be a power of 2
// Out of memory situation is reported by throwing std::bad_alloc exception
void* aligned_malloc (size_t size, size_t alignment);

// Deallocates memory allocated by aligned_malloc
void aligned_free (void *aligned);


// Represent a given value as a string and enclose in quotes
template <typename T>
string inquotes (const T& x, const char* q = "\"")
{
    std::ostringstream ostr;
    ostr << q << x << q;
    return ostr.str();
}

template <typename T>
std::wstring inquotes_w (const T& x, const wchar_t* q = L"\"")
{
    std::wostringstream ostr;
    ostr << q << x << q;
    return ostr.str();
}


template <typename T>
void _str_to (const string& s, T& res)
{
    std::istringstream ss(s);
    ss >> res;

    if (!ss || (ss.get(), ss))
    {
        throw SampleError(
            "Cannot interpret string " + inquotes(s) +
            " as object of type " + inquotes(typeid(T).name())
        );
    }
}


inline void _str_to (const string& s, string& res)
{
    res = s;
}


// Convert from a string to a value of a given type.
// T should have operator>> defined to be read from stream.
template <typename T>
T str_to (const string& s)
{
    T res;
    _str_to(s, res);
    return res;
}


// Convert from a value of a given type to string with optional formatting.
// T should have operator<< defined to be written to stream.
template <typename T>
string to_str (const T x, std::streamsize width = 0, char fill = ' ', int precision = -1)
{
    using namespace std;
    ostringstream os;
    if( precision >= 0 )
        os << setprecision(precision) << fixed;
    os << setw(width) << setfill(fill) << x;
    if(!os)
    {
        throw SampleError("Cannot represent object as a string");
    }
    return os.str();
}


// Detect if x is string representation of int value.
bool is_number (const string& x);


// Return one random number uniformally distributed in
// range [0,1] by std::rand.
// T should be a floatting point type
template <typename T>
T rand_uniform_01 ()
{
    return T(std::rand())/RAND_MAX;
}


// Fill array of a given size with random numbers
// uniformally distributed in range of [0,1] by std::rand.
// T should be a floatting point type
template <typename T>
void fill_rand_uniform_01 (T* buffer, size_t size)
{
    std::generate_n(buffer, size, rand_uniform_01<T>);
}


// Returns random index in range 0..n-1
inline size_t rand_index (size_t n)
{
    return static_cast<size_t>(std::rand()/((double)RAND_MAX + 1)*n);
}


// Returns current system time accurate enough for performance measurements
// In seconds.
double time_stamp ();

// Follows safe procedure when exception in destructor is thrown.
void destructorException ();


// Query for several frequently used device/kernel capabilities

// Returns directory path of current executable.
std::string exe_dir ();
std::wstring exe_dir_w ();


// Convers string to wstring
std::wstring stringToWstring (const std::string s);

// Convers wstring to string
std::string wstringToString (const std::wstring s);


// Full path creation helper macros for string and wstring
#define FULL_PATH_A(name) (::exe_dir()+std::string(name)).c_str()
#define FULL_PATH_W(name) (::exe_dir_w()+std::wstring(L##name)).c_str()



#ifdef UNICODE
#define FULL_PATH FULL_PATH_W
#else
#define FULL_PATH FULL_PATH_A
#endif

// Helper structure to hold CTYPE locale.
// Default CTYPE locale at the program startup is "C".
// It can be changed to the system default. Previous locale is stored and will be restored at object
// destruction time.
struct CTYPELocaleHelper
{
    CTYPELocaleHelper()
    {
        //Get current locale
        const char* tmp_locale = setlocale(LC_CTYPE, NULL);
        if(tmp_locale == NULL)
        {
            cerr
            << "[ WARNING ] Cannot retrieve current CTYPE locale. Non-ASCII file paths will not work.\n";
            return;
        }

        //Store current locale
        old_locale.append(tmp_locale);

        //Set system default locale
        tmp_locale = setlocale(LC_CTYPE, "");
        if(tmp_locale == NULL)
        {
            cerr
            << "[ WARNING ] Cannot set system default CTYPE locale. Non-ASCII file paths will not work.\n";
        }
    }


    ~CTYPELocaleHelper ()
    {
        //Restore locale
        const char* tmp_locale = setlocale(LC_CTYPE, old_locale.c_str());
        if(tmp_locale == NULL)
        {
            cerr
            << "[ WARNING ] Cannot restore CTYPE locale.\n";
        }
    }

private:
    // Old locale storage.
    std::string old_locale;
};
// Rounds up a given number x to be dividable by alignement
// Alignment should be a power of two
size_t round_up_aligned (size_t x, size_t alignment);


#endif  // end of include guard
