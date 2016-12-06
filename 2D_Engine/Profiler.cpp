#include "Profiler.h"

#include <chrono>
#include <iostream>

using namespace std::chrono;

Profiler::Profiler(const std::string& name)
{
    mName = name;
    mStart = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

Profiler::~Profiler()
{
    long long deltaTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count() - mStart;
    std::cout << mName << " : " << deltaTime << " ms." << std::endl;
}
