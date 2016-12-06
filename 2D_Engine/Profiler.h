#pragma once

#include <string>

// Profiler.
class Profiler {
    public:
        // Constructor.
        Profiler(const std::string& name);

        // Destructor.
        ~Profiler();

    private:
        std::string mName;
        long long mStart;
};

#define PROFILE(name) Profiler __profileInstance(name)
