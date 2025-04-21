#include <pthread.h>
#include <string>

#ifndef DSPP_UTILS_H
#define DSPP_UTILS_H

inline void SetCurrentThreadName(const std::string& name) {
    pthread_setname_np(pthread_self(), name.c_str());
}

// Helper functions will not be apart of the DualSense namespace - Make a util.h?
inline double getCurrentTimestamp()
{
    using namespace std::chrono;
    return duration_cast<duration<double>>(
            steady_clock::now().time_since_epoch()
    ).count();
}
#endif //DSPP_UTILS_H
