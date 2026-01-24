# cputemp

A tool to dynamically control CPU frequency to keep temperature below a specified target on Linux.

## Quick Start

### Build
```bash
make
```

### Find Your Sensor
```bash
./cputemp
```
This lists available temperature sensors (e.g., `coretemp`, `k10temp`).

### Run (Interactive)
```bash
sudo ./cputemp --sensor <sensor_name> --temp 80 --verbose
```

### Run as Daemon
```bash
sudo ./cputemp --daemon /run/cputemp.pid --sensor <sensor_name> --temp 80
```

### Stop Daemon
```bash
sudo ./cputemp --kill-daemon /run/cputemp.pid
```

## Systemd Service

### Install
```bash
# Install binary
sudo cp cputemp /usr/local/bin/

# Install reset script (failsafe)
sudo cp reset_freq.sh /usr/local/bin/
sudo chmod +x /usr/local/bin/reset_freq.sh

# Install service
sudo cp cputemp.service /etc/systemd/system/

# Edit service file to set your sensor name
sudo nano /etc/systemd/system/cputemp.service

# Reload and start
sudo systemctl daemon-reload
sudo systemctl start cputemp
sudo systemctl enable cputemp
```

### Service Management
```bash
sudo systemctl start cputemp    # Start
sudo systemctl stop cputemp     # Stop
sudo systemctl restart cputemp  # Restart
sudo systemctl status cputemp   # Status
```

### View Logs
```bash
# All logs
sudo journalctl -u cputemp

# Follow logs
sudo journalctl -u cputemp -f

# Throttling events only
sudo journalctl -u cputemp | grep THROTTLE
```

## Options

- `--sensor <name>` - Temperature sensor name (required)
- `--temp <degrees>` - Target temperature in Celsius (default: 80)
- `--period <seconds>` - Check interval (default: 1.0)
- `--daemon <pidfile>` - Run as daemon
- `--kill-daemon <pidfile>` - Stop daemon
- `--verbose` - Verbose output

## Features

- **Automatic throttling**: Reduces CPU frequency when temperature exceeds target
- **Logging**: Throttling events logged to systemd journal
- **Failsafe**: Frequency automatically restored to maximum on exit
- **Systemd integration**: Full systemd service support

## Requirements

- Linux with cpufreq support
- Root access (for sysfs operations)
- C++17 compiler (g++ or clang++)

## Notes

- Some CPUs may need kernel parameters (e.g., `amd_pstate=passive` for AMD)
- Frequency is controlled via `/sys/devices/system/cpu/cpu*/cpufreq/scaling_max_freq`
- Throttling events are always logged, even without `--verbose`

## License

Zero-Clause BSD (public domain)
