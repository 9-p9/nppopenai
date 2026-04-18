/**
 * test_main.cpp – Entry point for the NppOpenAI unit-test executable.
 *
 * Each test_*.cpp file self-registers its tests via static initialisers;
 * run_all_tests() iterates all of them and returns 0 on full success.
 */

#include "test_framework.h"

int main()
{
    return run_all_tests();
}
