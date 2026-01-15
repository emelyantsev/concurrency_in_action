#pragma once

#include <iostream>
#include <syncstream>
#include <chrono>

#ifndef sync_cout
#define sync_cout std::osyncstream( std::cout )
#endif

using namespace std::chrono_literals;