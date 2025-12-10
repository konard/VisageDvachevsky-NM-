#pragma once

#include <cstdlib>
#include <iostream>

namespace nm::core
{

inline void assertFailed(const char* condition, const char* message,
                         const char* file, int line)
{
    std::cerr << "Assertion failed: " << condition << "\n"
              << "Message: " << message << "\n"
              << "File: " << file << "\n"
              << "Line: " << line << std::endl;
    std::abort();
}

} // namespace nm::core

#ifdef NDEBUG
    #define NM_ASSERT(condition, message) ((void)0)
#else
    #define NM_ASSERT(condition, message) \
        do { \
            if (!(condition)) { \
                nm::core::assertFailed(#condition, message, __FILE__, __LINE__); \
            } \
        } while (false)
#endif

#define NM_ASSERT_NOT_NULL(ptr) NM_ASSERT((ptr) != nullptr, "Pointer must not be null")
