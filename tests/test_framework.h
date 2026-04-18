/**
 * test_framework.h – Minimal unit-test assertion helpers.
 *
 * Usage:
 *   TEST_SUITE("Suite name")
 *   TEST_CASE("test description") { ASSERT_EQ(1+1, 2); }
 *   TEST_CASE("another test") { ASSERT_TRUE(someCondition()); }
 *
 * Call run_all_tests() from main() to execute everything.
 * The program returns 0 on success, 1 if any test failed.
 */

#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <cstdlib>

// ---- Internal registry ----

struct TestCase
{
    std::string suite;
    std::string name;
    std::function<void()> fn;
};

inline std::vector<TestCase> &_test_registry()
{
    static std::vector<TestCase> v;
    return v;
}

inline std::string &_current_suite()
{
    static std::string s;
    return s;
}

// ---- Macros for declaring suites and test cases ----

// Helper macros for token-pasting with __COUNTER__ (ensures globally unique names)
#define _NPP_CONCAT_IMPL(a, b) a##b
#define _NPP_CONCAT(a, b) _NPP_CONCAT_IMPL(a, b)

// Two-level trick: __COUNTER__ is expanded once, then id is reused consistently
#define TEST_SUITE(name) \
    _NPP_TEST_SUITE_IMPL(_NPP_CONCAT(_NPP_Suite_, __COUNTER__), name)

#define _NPP_TEST_SUITE_IMPL(id, name)                                             \
    namespace { struct id { id() { ::_current_suite() = (name); } }                \
                _NPP_CONCAT(_npp_suite_inst_, id); }

#define TEST_CASE(desc) \
    _NPP_TEST_CASE_IMPL(_NPP_CONCAT(_npp_test_fn_, __COUNTER__), desc)

#define _NPP_TEST_CASE_IMPL(id, desc)                                              \
    static void id();                                                              \
    namespace {                                                                    \
        struct _NPP_CONCAT(_NPP_Reg_, id) {                                        \
            _NPP_CONCAT(_NPP_Reg_, id)() {                                         \
                ::_test_registry().push_back({::_current_suite(), (desc), id});    \
            }                                                                      \
        } _NPP_CONCAT(_npp_reg_, id);                                              \
    }                                                                              \
    static void id()

// ---- Assertions ----

#define ASSERT_TRUE(expr) \
    do { if (!(expr)) { \
        std::cerr << "  FAIL: " << __FILE__ << ":" << __LINE__ \
                  << "  ASSERT_TRUE(" #expr ")\n"; \
        throw std::string("assertion failed"); \
    } } while(0)

#define ASSERT_FALSE(expr)  ASSERT_TRUE(!(expr))

#define ASSERT_EQ(a, b) \
    do { auto _a = (a); auto _b = (b); \
         if (!(_a == _b)) { \
             std::cerr << "  FAIL: " << __FILE__ << ":" << __LINE__ \
                       << "  ASSERT_EQ  left=["  << _a << "]  right=[" << _b << "]\n"; \
             throw std::string("assertion failed"); \
         } } while(0)

#define ASSERT_NE(a, b) \
    do { auto _a = (a); auto _b = (b); \
         if (_a == _b) { \
             std::cerr << "  FAIL: " << __FILE__ << ":" << __LINE__ \
                       << "  ASSERT_NE  both=[" << _a << "]\n"; \
             throw std::string("assertion failed"); \
         } } while(0)

#define ASSERT_CONTAINS(str, sub) \
    do { if ((str).find(sub) == std::string::npos) { \
        std::cerr << "  FAIL: " << __FILE__ << ":" << __LINE__ \
                  << "  ASSERT_CONTAINS  string=[" << (str) \
                  << "]  sub=[" << (sub) << "]\n"; \
        throw std::string("assertion failed"); \
    } } while(0)

#define ASSERT_NOT_CONTAINS(str, sub) \
    do { if ((str).find(sub) != std::string::npos) { \
        std::cerr << "  FAIL: " << __FILE__ << ":" << __LINE__ \
                  << "  ASSERT_NOT_CONTAINS  string=[" << (str) \
                  << "]  sub=[" << (sub) << "]\n"; \
        throw std::string("assertion failed"); \
    } } while(0)

// ---- Runner ----

inline int run_all_tests()
{
    int passed = 0, failed = 0;
    std::string currentSuite;

    for (auto &tc : _test_registry()) {
        if (tc.suite != currentSuite) {
            currentSuite = tc.suite;
            std::cout << "\n[" << currentSuite << "]\n";
        }
        std::cout << "  " << tc.name << " ... ";
        try {
            tc.fn();
            std::cout << "PASS\n";
            ++passed;
        } catch (const std::string &msg) {
            std::cout << "FAIL  (" << msg << ")\n";
            ++failed;
        } catch (const std::exception &e) {
            std::cout << "FAIL  (exception: " << e.what() << ")\n";
            ++failed;
        } catch (...) {
            std::cout << "FAIL  (unknown exception)\n";
            ++failed;
        }
    }

    std::cout << "\n==================================================\n";
    std::cout << "Results: " << passed << " passed, " << failed << " failed\n";
    return failed == 0 ? 0 : 1;
}
