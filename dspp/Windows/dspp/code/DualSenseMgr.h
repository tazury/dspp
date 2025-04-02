/*
* THIS IS DESIGNED FOR Visual Studio 2022 (v143) with the Windows SDK 10.0! If that isn't your platform, please check if there are any others!

* Title: Sony Playstation©️ 5 DualSense Manager
* Author: Munashe Dirwayi [wanomaniac]
* Purpose: "This header file is where the manager file is located, this will manage how you will discover controllers.
            You can discover controllers by either using the threaded option if you wanted to keep it in the background.
            Or you can just wait until an amount of controllers has been added. 

            This manager will allow you to listen to multiple controllers via Input Listeners by specific player or all 
            players. Making it much easier to control.

* Compatibility: Designed in C++ 20 | C Standard 17

* Updates -
          ** UPDATES WILL BE HERE **

* Known issues -
          ** BUGS WILL BE REPORTED HERE **
*/
#pragma once
#include "DualSense.h"
#include "DualSenseInput.h"
#include <map>
#include <functional>
#include <thread>
#include <mutex>
#include <unordered_set>
#include <chrono> // MSVC broke chrono so we will have to include it ourselves.

/// <summary>
/// This is the first thing you'll use in this library, this controls the IO of your controllers and the connection of the controllers
/// </summary>
class DualSenseMgr {
public:
    ~DualSenseMgr();
public: // Structs - Discoveries
    /// <summary>
    /// This is what gets sent to your Discovery Listener once a DualSense has been detected.
    /// </summary>
    struct CtrlIncomingInfo {
        bool reconnection;
        int playerNumber; // The player number of the controller. It can be used as an ID
        long long timestamp; // When the controller was detected in ms
        DualSense::CtrlInfo* cInfo; // The controller's specific information
    };
    /// <summary>
    /// This is configuration for discovering a DualSense controller
    /// </summary>
    struct DiscoverSettings {
        int minPlayers = 1; // Minimum players before finishing.
        int maxPlayers = 8; // Maximum players before finishing.
        long long timeout = -1; // The amount of miliseconds added to the time that would be considered too long for you.
    };
    /// <summary>
    /// The format of Discovery Listeners
    /// </summary>
    using DiscoveryListener = std::function<bool(const CtrlIncomingInfo&)>;
private: // Private Variables - Discoveries
    std::atomic<bool> justStop = false;
    std::thread discoveryThread;
    DiscoveryListener dlistener;
    std::vector<std::thread> cThreads;
    std::mutex dlistenersMutex;
public: // Public Functions - Discoveries
    /// <summary>
    /// Creates an seperate thread for discovering controllers
    /// </summary>
    /// <param name="settings">: Your configuration</param>
    void discoverInBackground(DiscoverSettings settings);
    /// <summary>
    /// Creates an seperate thread for discovering controllers. When an controller is detected, it will send the controller for approval
    /// with your DiscoveryListener. If the listener says yes, it becomes apart of the controller list, if the listener says no, that controller will be gone.
    /// </summary>
    /// <param name="settings">: Your configuration</param>
    /// <param name="listener">: A DiscoveryListener format function for approving controllers</param>
    void discoverInBackground(DiscoverSettings settings, DiscoveryListener listener);
    /// <summary>
    /// It will discover controllers on the same thread as the function. So depending on your configuration, it make take some time before the function finishes and your code continues
    /// </summary>
    /// <param name="settings">: Your configuration</param>
    /// <returns>Boolean</returns>
    bool discoverNow(DiscoverSettings settings);

private:
    bool notifyDListeners(CtrlIncomingInfo info);
    bool discoverHelper(DualSenseMgr::DiscoverSettings settings, bool isThreaded);
    void connectionThread(DualSense::DualSense* ds);
public: 
    /// <summary>
    /// A get function for the controllers map
    /// </summary>
    /// <returns>A map - std::map(int, DualSense*)</returns>
    std::map<int, DualSense::DualSense*> getControllers();
    /// <summary>
    /// Gets the controller from the Player ID requested
    /// Player IDs start from 1, do not put 0 in traditional C++
    /// </summary>
    DualSense::DualSense* getController(int id);
    /// <summary>
    /// A get function for the amount of players connected.
    /// </summary>
    /// <returns>Integer</returns>
    int getAmoOfPlayers();
private:
	std::map<int, DualSense::DualSense*> controllers; // Your controllers that are currently running
    std::unordered_set<std::string> seenDevicePaths;
};