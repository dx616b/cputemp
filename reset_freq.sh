#!/bin/bash
# Failsafe script to reset CPU frequency to maximum
# Used by systemd ExecStopPost to ensure frequency is restored even if process crashes

# Find the maximum frequency
MAX_FREQ=$(cat /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq 2>/dev/null)

if [ -z "$MAX_FREQ" ]; then
    # Try to find any CPU with frequency info
    for cpu in /sys/devices/system/cpu/cpu*/cpufreq/cpuinfo_max_freq; do
        if [ -r "$cpu" ]; then
            MAX_FREQ=$(cat "$cpu")
            break
        fi
    done
fi

if [ -z "$MAX_FREQ" ]; then
    echo "Error: Could not determine maximum CPU frequency" >&2
    exit 1
fi

# Set frequency for all CPU cores
# Both cpuinfo_max_freq and scaling_max_freq use kHz, so no conversion needed
CORES_RESET=0
for cpu in /sys/devices/system/cpu/cpu*/cpufreq/scaling_max_freq; do
    if [ -w "$cpu" ]; then
        if echo "$MAX_FREQ" > "$cpu" 2>/dev/null; then
            CORES_RESET=$((CORES_RESET + 1))
        fi
    fi
done

if [ $CORES_RESET -eq 0 ]; then
    echo "Error: Could not write to any CPU frequency files (need root access)" >&2
    exit 1
fi

# Log the action (if logger is available and we're running as root)
if command -v logger >/dev/null 2>&1 && [ "$EUID" -eq 0 ]; then
    logger -t cputemp "Failsafe: Reset CPU frequency to maximum ($MAX_FREQ kHz) on $CORES_RESET cores"
else
    echo "Reset CPU frequency to maximum ($MAX_FREQ kHz) on $CORES_RESET cores"
fi
