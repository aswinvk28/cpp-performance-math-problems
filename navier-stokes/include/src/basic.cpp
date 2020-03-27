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


#include <iostream>
#include <exception>
#include <vector>
#include <cerrno>
#include <cstdint>

#include <intel/vx_samples/basic.hpp>


#ifdef __linux__
#include <sys/time.h>
#include <unistd.h>
#include <libgen.h>
#elif defined(_WIN32) || defined(WIN32)
#include <Windows.h>
#else
#include <ctime>
#endif

using std::string;


void* aligned_malloc (size_t size, size_t alignment)
{
    // a number of requirements should be met
    assert(alignment > 0);
    assert((alignment & (alignment - 1)) == 0); // test for power of 2

    if(alignment < sizeof(void*))
    {
        alignment = sizeof(void*);
    }

    assert(size >= sizeof(void*));
    assert(size/sizeof(void*)*sizeof(void*) == size);

    // allocate extra memory and convert to size_t to perform calculations
    char* orig = new char[size + alignment + sizeof(void*)];
    // calculate an aligned position in the allocated region
    // assumption: (size_t)orig does not lose lower bits
    char* aligned =
        orig + (
        (((size_t)orig + alignment + sizeof(void*)) & ~(alignment - 1)) -
        (size_t)orig
        );
    // save the original pointer to use it in aligned_free
    *((char**)aligned - 1) = orig;
    return aligned;
}


void aligned_free (void *aligned)
{
    if(!aligned)return; // behaves as delete: calling with 0 is NOP
    delete [] *((char**)aligned - 1);
}


bool is_number (const string& x)
{
    // Detection is simple: just try to represent x as an int
    try
    {
        // If x is a number, then str_to returns without an exception
        // In case when x cannot be converted to int
        // str_to rises SampleError exception (see str_to definitin)
        str_to<int>(x);

        // success: x is a number
        return true;
    }
    catch(const SampleError&)
    {
        // fail: x is not a number
        return false;
    }
}


double time_stamp ()
{
#ifdef __linux__
    {
        struct timeval t;
        if(gettimeofday(&t, 0) != 0)
        {
            throw SampleError(
                "Linux-specific time measurement counter (gettimeofday) "
                "is not available."
                );
        }
        return t.tv_sec + t.tv_usec/1e6;
    }
#elif defined(_WIN32) || defined(WIN32)
    {
        LARGE_INTEGER curclock;
        LARGE_INTEGER freq;
        if(
            !QueryPerformanceCounter(&curclock) ||
            !QueryPerformanceFrequency(&freq)
            )
        {
            throw SampleError(
                "Windows-specific time measurement counter (QueryPerformanceCounter, "
                "QueryPerformanceFrequency) is not available."
                );
        }

        return double(curclock.QuadPart)/freq.QuadPart;
    }
#else
    {
        // very low resolution
        return double(time(0));
    }
#endif
}


void destructorException ()
{
    if(std::uncaught_exception())
    {
        // don't crash an application because of double throwing
        // let the user see the original exception and suppress
        // this one instead
        std::cerr
            << "[ ERROR ] Catastrophic: another exception "
            << "was thrown and suppressed during handling of "
            << "previously started exception raising process.\n";
    }
    else
    {
        // that's OK, go up!
        throw;
    }
}


string exe_dir ()
{
    using namespace std;
    const int start_size = 1000;
    const int max_try_count = 8;

#ifdef __linux__
    {
        static string const exe = "/proc/self/exe";

        vector<char> path(start_size);
        int          count = max_try_count;  // Max number of iterations.

        for(;;)
        {
            ssize_t len = readlink(exe.c_str(), &path[0], path.size());

            if(len < 0)
            {
                throw SampleError(
                    "Cannot retrieve path to the executable: "
                    "readlink returned error code " +
                    to_str(errno) + "."
                    );
            }

            if(len < path.size())
            {
                // We got the path.
                path.resize(len);
                break;
            }

            if(count > 0)   // the buffer is too small
            {
                --count;
                // Enlarge the buffer.
                path.resize(path.size() * 2);
            }
            else
            {
                throw SampleError("Cannot retrieve path to the executable: path is too long.");
            }
        }

        return string(dirname(&path[0])) + "/";
    }
#elif defined(_WIN32) || defined(WIN32)
    {
        // Retrieving path to the executable.

        vector<char> path(start_size);
        int count = max_try_count;

        for(;;)
        {
            DWORD len = GetModuleFileNameA(NULL, &path[0], (DWORD)path.size());

            if(len == 0)
            {
                int err = GetLastError();
                throw SampleError(
                    "Getting executable path failed with error " +
                    to_str(err)
                    );
            }

            if(len < path.size())
            {
                path.resize(len);
                break;
            }

            if(count > 0)   // buffer is too small
            {
                --count;
                // Enlarge the buffer.
                path.resize(path.size() * 2);
            }
            else
            {
                throw SampleError(
                    "Cannot retrieve path to the executable: "
                    "path is too long."
                    );
            }
        }

        string exe(&path[0], path.size());

        // Splitting the path into components.

        vector<char> drv(_MAX_DRIVE);
        vector<char> dir(_MAX_DIR);
        count = max_try_count;

        for(;;)
        {
            int rc =
                _splitpath_s(
                exe.c_str(),
                &drv[0], drv.size(),
                &dir[0], dir.size(),
                NULL, 0,   // We need neither name
                NULL, 0    // nor extension
                );
            if(rc == 0)
            {
                break;
            }
            else if(rc == ERANGE)
            {
                if(count > 0)
                {
                    --count;
                    // Buffer is too small, but it is not clear which one.
                    // So we have to enlarge both.
                    drv.resize(drv.size() * 2);
                    dir.resize(dir.size() * 2);
                }
                else
                {
                    throw SampleError(
                        "Getting executable path failed: "
                        "Splitting path " + exe + " to components failed: "
                        "Buffers of " + to_str(drv.size()) + " and " +
                        to_str(dir.size()) + " bytes are still too small."
                        );
                }
            }
            else
            {
                throw SampleError(
                    "Getting executable path failed: "
                    "Splitting path " + exe +
                    " to components failed with code " + to_str(rc)
                    );
            }
        }

        // Combining components back to path.
        return string(&drv[0]) + string(&dir[0]);
    }
#else
    {
        throw SampleError(
            "There is no method to retrieve the directory path "
            "where executable is placed: unsupported platform."
            );
    }
#endif
}

std::wstring exe_dir_w ()
{
    using namespace std;
    const int start_size = 1000;
    const int max_try_count = 8;

#if defined(_WIN32) || defined(WIN32)
    {
        // Retrieving path to the executable.

        vector<wchar_t> path(start_size);
        int count = max_try_count;

        for(;;)
        {
            DWORD len = GetModuleFileNameW(NULL, &path[0], (DWORD)path.size());

            if(len == 0)
            {
                int err = GetLastError();
                throw SampleError(
                    "Getting executable path failed with error " +
                    to_str(err)
                    );
            }

            if(len < path.size())
            {
                path.resize(len);
                break;
            }

            if(count > 0)   // buffer is too small
            {
                --count;
                // Enlarge the buffer.
                path.resize(path.size() * 2);
            }
            else
            {
                throw SampleError(
                    "Cannot retrieve path to the executable: "
                    "path is too long."
                    );
            }
        }

        wstring exe(&path[0], path.size());

        // Splitting the path into components.

        vector<wchar_t> drv(_MAX_DRIVE);
        vector<wchar_t> dir(_MAX_DIR);
        count = max_try_count;

        for(;;)
        {
            int rc =
                _wsplitpath_s(
                exe.c_str(),
                &drv[0], drv.size(),
                &dir[0], dir.size(),
                NULL, 0,   // We need neither name
                NULL, 0    // nor extension
                );
            if(rc == 0)
            {
                break;
            }
            else if(rc == ERANGE)
            {
                if(count > 0)
                {
                    --count;
                    // Buffer is too small, but it is not clear which one.
                    // So we have to enlarge both.
                    drv.resize(drv.size() * 2);
                    dir.resize(dir.size() * 2);
                }
                else
                {
                    throw SampleError(
                        "Getting executable path failed: "
                        "Splitting path " + wstringToString(exe) + " to components failed: "
                        "Buffers of " + to_str(drv.size()) + " and " +
                        to_str(dir.size()) + " bytes are still too small."
                        );
                }
            }
            else
            {
                throw SampleError(
                    "Getting executable path failed: "
                    "Splitting path " + wstringToString(exe) +
                    " to components failed with code " + to_str(rc)
                    );
            }
        }

        // Combining components back to path.
        return wstring(&drv[0]) + wstring(&dir[0]);
    }
#else
    {
        throw SampleError(
            "There is no method to retrieve the directory path "
            "where executable is placed: unsupported platform."
            );
    }
#endif
}

std::wstring stringToWstring (const std::string s)
{
    return std::wstring(s.begin(), s.end());
}

#ifdef __linux__
std::string wstringToString (const std::wstring w)
{
    string tmp;
    const wchar_t* src = w.c_str();
    //Store current locale and set default locale
    CTYPELocaleHelper locale_helper;

    //Get required number of characters
    size_t count = wcsrtombs(NULL, &src, 0, NULL);
    if(count == size_t(-1))
    {
        throw SampleError(
            "Cannot convert wstring to string"
        );
    }
    std::vector<char> dst(count+1);

    //Convert wstring to multibyte representation
    size_t count_converted = wcsrtombs(&dst[0], &src, count+1, NULL);
    dst[count_converted] = '\0';
    tmp.append(&dst[0]);
    return tmp;
}
#else
std::string wstringToString (const std::wstring w)
{
    string tmp;

    const char* question_mark = "?"; //replace wide characters which don't fit in the string with "?" mark

    for(unsigned int i = 0; i < w.length(); i++)
    {
        if(w[i]>255||w[i]<0)
        {
            tmp.append(question_mark);
        }
        else
        {
            tmp += (char)w[i];
        }
    }
    return tmp;
}
#endif

size_t round_up_aligned (size_t x, size_t alignment)
{
    assert(alignment > 0);
    assert((alignment & (alignment - 1)) == 0); // test for power of 2

    size_t result = (x + alignment - 1) & ~(alignment - 1);

    assert(result >= x);
    assert(result - x < alignment);
    assert((result & (alignment - 1)) == 0);

    return result;
}
