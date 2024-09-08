
#ifndef SPC_MAC_H_
#define SPC_MAC_H_

namespace cen {

namespace macutils {
    #include <string>
    #include <CoreFoundation/CoreFoundation.h>

    std::string GetResourcePath(const std::string& resource) {
        CFBundleRef bundle = CFBundleGetMainBundle();
        CFURLRef resourceURL = CFBundleCopyResourceURL(bundle, CFStringCreateWithCString(nullptr, resource.c_str(), kCFStringEncodingUTF8), nullptr, nullptr);
        
        char path[PATH_MAX];
        CFURLGetFileSystemRepresentation(resourceURL, true, (UInt8*)path, PATH_MAX);
        CFRelease(resourceURL);

        return std::string(path);
    }
}

}

#endif // SPC_MAC_H_