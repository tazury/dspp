#pragma once
#include <windows.h>
#include <map>

struct Resource {
    int validity;  // -1 means invalid by default
    DWORD size;    // Store the size as a DWORD, not a pointer
    void* data;    // Pointer to the resource data

    // Default constructor, initializing validity to -1
    Resource() : validity(-1), size(0), data(nullptr) {}

    // Constructor with validity value
    Resource(int valid) : validity(valid), size(0), data(nullptr) {}

    // Constructor with size and data
    Resource(int valid, DWORD resourceSize, void* resourceData)
        : validity(valid), size(resourceSize), data(resourceData) {
    }
};

 class ResourceMgr
{
public:	
 bool LoadData(int resourceID, void** data, DWORD* size) {
    HMODULE hModule = GetModuleHandle(nullptr);
    HRSRC hRes = FindResource(hModule, MAKEINTRESOURCE(resourceID), RT_FONT);
    if (!hRes) return false;

    HGLOBAL hData = LoadResource(hModule, hRes);
    if (!hData) return false;

    *size = SizeofResource(hModule, hRes);
    *data = LockResource(hData);

    if (*data != nullptr) {
        Resource res;
        res.validity = 0;
        res.data = data;
        res.size = *size;
        mData[resourceID] = res;
    }

    return (*data != nullptr);
    
}

 Resource LoadData(int resourceID) {
     HMODULE hModule = GetModuleHandle(nullptr);
     HRSRC hRes = FindResource(hModule, MAKEINTRESOURCE(resourceID), RT_FONT);  // Using RT_RCDATA instead of RT_FONT
     if (!hRes) {
         return Resource(2);  // Error: Resource not found
     }

     HGLOBAL hData = LoadResource(hModule, hRes);
     if (!hData) {
         return Resource(1);  // Error: Failed to load resource
     }

     void* data = LockResource(hData);  // Get a pointer to the resource data
     DWORD size = SizeofResource(hModule, hRes);  // Get the size of the resource data

     if (!data) {
         return Resource(3);  // Error: Data is nullptr
     }

     Resource res;
     res.validity = 0;  // Ensure that validity is set correctly
     res.data = data;   // Directly assign the pointer to the resource data
     res.size = size;   // Assign the size of the resource

     // Assuming mData is a map or a similar data structure
     mData[resourceID] = res;  // Store the resource in the map

     return mData[resourceID];  // Return the loaded resource

 }

 Resource getData(int resourceID) {
     if (mData.find(resourceID) != mData.end()) {
         return mData[resourceID];
     }
     else return Resource(1);
 }

private:
    std::map<int, Resource> mData;
};

