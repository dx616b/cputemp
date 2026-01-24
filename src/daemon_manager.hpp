#ifndef DAEMON_MANAGER_HPP
#define DAEMON_MANAGER_HPP

#include <string>
#include <signal.h>
#include <fstream>

class DaemonManager {
public:
    DaemonManager();
    ~DaemonManager();
    
    // Non-copyable
    DaemonManager(const DaemonManager&) = delete;
    DaemonManager& operator=(const DaemonManager&) = delete;
    
    // Open PID file and optionally kill existing daemon
    // Returns true on success
    bool openPidFile(const std::string& pidFile, bool killExisting);
    
    // Close PID file without removing it (for reopening later)
    void closePidFile();
    
    // Kill existing daemon (used by --kill-daemon option)
    // Returns true if daemon was killed or didn't exist
    bool killDaemon(const std::string& pidFile);
    
    // Daemonize the process
    // Returns true on success
    bool daemonize();
    
    // Write PID to file
    void writePid();
    
    // Setup signal handler with frequency reset callback
    void setupSignalHandler(void (*resetFreqCallback)(void));
    
    // Cleanup (public for signal handler)
    void cleanup();
    
private:
    std::string pidFilePath_;
    bool pidFileOpen_;
    pid_t readPidFromFile(const std::string& pidFile);
    bool writePidToFile(const std::string& pidFile, pid_t pid);
    bool pidFileExists(const std::string& pidFile);
};

#endif // DAEMON_MANAGER_HPP
