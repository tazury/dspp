/*
* THIS IS DESIGNED FOR CLION with Ubuntu 24.04.2! If that isn't your platform, please check if there are any others!
* It is recommanded to build from source if you have an older version but note the gcc version.
 *
* Title: DS++ Utilities
* Author: Munashe Dirwayi [wanomaniac]
* Purpose: "I created an utility file because GCC was complaining about repeated functions.
           This is the main way of accessing most of the library."

* Compatibility: Designed in C++ 20 | C Standard 17 [gcc (Ubuntu 13.3.0-6ubuntu2~24.04) 13.3.0]

* Updates -
          ** UPDATES WILL BE HERE **

* Known issues -
          ** BUGS WILL BE REPORTED HERE **
*/
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
