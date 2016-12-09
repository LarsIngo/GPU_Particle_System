#pragma once

#include <string>

// Profiler.
class Profiler {
    public:
        // Constructor.
        Profiler(const std::string& name, bool dump);

        // Destructor.
        ~Profiler();

    private:
        std::string mName;
        long long mStart;
        bool mDump;
};

#define PROFILE(name, dump) Profiler __profileInstance(name, dump)
