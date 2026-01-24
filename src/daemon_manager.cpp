#include "daemon_manager.hpp"
#include <iostream>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <thread>
#include <chrono>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

// Global pointer for signal handler cleanup
static DaemonManager* g_daemonManager = nullptr;
static void (*g_freqResetHandler)(void) = nullptr;

extern "C" {
    static void signalHandler(int sig) {
        if (g_daemonManager) {
            g_daemonManager->cleanup();
        }
        if (g_freqResetHandler) {
            g_freqResetHandler();
        }
        std::exit(0);
    }
}

DaemonManager::DaemonManager() : pidFileOpen_(false) {
    g_daemonManager = this;
}

DaemonManager::~DaemonManager() {
    cleanup();
    g_daemonManager = nullptr;
}

void DaemonManager::cleanup() {
    if (pidFileOpen_ && !pidFilePath_.empty()) {
        unlink(pidFilePath_.c_str());
        pidFileOpen_ = false;
        pidFilePath_.clear();
    }
}

pid_t DaemonManager::readPidFromFile(const std::string& pidFile) {
    std::ifstream file(pidFile);
    if (!file.is_open()) {
        return 0;
    }
    
    pid_t pid = 0;
    file >> pid;
    
    // Check if process is still running
    if (pid > 0 && kill(pid, 0) == 0) {
        return pid;
    }
    
    return 0;
}

bool DaemonManager::writePidToFile(const std::string& pidFile, pid_t pid) {
    std::ofstream file(pidFile, std::ios::trunc);
    if (!file.is_open()) {
        return false;
    }
    
    file << pid << std::endl;
    return file.good();
}

bool DaemonManager::pidFileExists(const std::string& pidFile) {
    std::ifstream file(pidFile);
    return file.good();
}

bool DaemonManager::openPidFile(const std::string& pidFile, bool killExisting) {
    pid_t otherpid = 0;
    
    // Try to read existing PID file
    if (pidFileExists(pidFile)) {
        otherpid = readPidFromFile(pidFile);
        if (otherpid > 0) {
            if (killExisting) {
                if (kill(otherpid, SIGTERM) == 0) {
                    // Wait for process to exit
                    for (int i = 0; i < 10; i++) {
                        if (kill(otherpid, 0) != 0) {
                            break; // Process is gone
                        }
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    }
                } else {
                    std::cerr << "Could not kill already running daemon (PID:" << otherpid << ")" << std::endl;
                    return false;
                }
            } else {
                std::cerr << "Could not kill already running daemon (PID:" << otherpid << ")" << std::endl;
                return false;
            }
        }
    }
    
    // Try to create/lock PID file using O_CREAT | O_EXCL
    for (int i = 0; i < 10 && !pidFileOpen_; i++) {
        int fd = open(pidFile.c_str(), O_CREAT | O_EXCL | O_WRONLY, 0600);
        if (fd >= 0) {
            close(fd);
            pidFilePath_ = pidFile;
            pidFileOpen_ = true;
            break;
        } else if (errno == EEXIST) {
            // File exists, check if process is still running
            otherpid = readPidFromFile(pidFile);
            if (otherpid > 0 && kill(otherpid, 0) == 0) {
                if (killExisting) {
                    kill(otherpid, SIGTERM);
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                } else {
                    std::cerr << "Could not kill already running daemon (PID:" << otherpid << ")" << std::endl;
                    return false;
                }
            } else {
                // Stale PID file, remove it
                unlink(pidFile.c_str());
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        } else {
            std::cerr << "Could not open pid file: " << strerror(errno) << std::endl;
            return false;
        }
    }
    
    if (!pidFileOpen_) {
        std::cerr << "Could not open pid file after retries" << std::endl;
        return false;
    }
    
    return true;
}

void DaemonManager::closePidFile() {
    if (pidFileOpen_) {
        unlink(pidFilePath_.c_str());
        pidFileOpen_ = false;
        pidFilePath_.clear();
    }
}

bool DaemonManager::killDaemon(const std::string& pidFile) {
    pid_t otherpid = readPidFromFile(pidFile);
    
    if (otherpid > 0) {
        if (kill(otherpid, SIGTERM) == 0) {
            unlink(pidFile.c_str());
            return true;
        }
    } else {
        // No running process, just remove stale file
        unlink(pidFile.c_str());
        return true;
    }
    
    return false;
}

bool DaemonManager::daemonize() {
    if (::daemon(0, 0) == -1) {
        std::cerr << "daemon(0, 0) failed" << std::endl;
        cleanup();
        return false;
    }
    return true;
}

void DaemonManager::writePid() {
    if (pidFileOpen_ && !pidFilePath_.empty()) {
        writePidToFile(pidFilePath_, getpid());
    }
}

void DaemonManager::setupSignalHandler(void (*resetFreqCallback)(void)) {
    // Store frequency reset handler
    g_freqResetHandler = resetFreqCallback;
    
    // Register signal handlers for graceful shutdown
    signal(SIGTERM, signalHandler);  // systemd stop
    signal(SIGINT, signalHandler);   // Ctrl+C
    signal(SIGHUP, signalHandler);   // Hangup
    signal(SIGQUIT, signalHandler);  // Quit
}
