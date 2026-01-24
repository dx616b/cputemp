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

if [ -n "$MAX_FREQ" ]; then
    # Convert from kHz to the format sysfs expects (it's already in kHz)
    # Set frequency for all CPU cores
    for cpu in /sys/devices/system/cpu/cpu*/cpufreq/scaling_max_freq; do
        if [ -w "$cpu" ]; then
            echo "$MAX_FREQ" > "$cpu" 2>/dev/null
        fi
    done
    logger -t cputemp "Failsafe: Reset CPU frequency to maximum ($MAX_FREQ kHz)"
fi
