# cputemp

A tool to dynamically control CPU frequency to keep temperature below a specified target on Linux.

## Quick Start

### 1. Build the Binary
```bash
make
```
This creates the `cputemp` executable.

### 2. Find Your Sensor
```bash
./cputemp
```
This lists available temperature sensors (e.g., `coretemp`, `k10temp`).

### 3. Test (Interactive)
```bash
sudo ./cputemp --sensor <sensor_name> --temp 80 --verbose
```

### 4. Run as Daemon
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
# 1. Build the binary first (if not already built)
make

# 2. Install binary
sudo cp cputemp /usr/local/bin/

# 3. Install reset script (failsafe)
sudo cp reset_freq.sh /usr/local/bin/
sudo chmod +x /usr/local/bin/reset_freq.sh

# 4. Install service
sudo cp cputemp.service /etc/systemd/system/

# 5. Edit service file to set your sensor name
sudo nano /etc/systemd/system/cputemp.service
# Change --sensor k10temp to your sensor (e.g., --sensor coretemp)

# 6. Reload and start
sudo systemctl daemon-reload
sudo systemctl start cputemp
sudo systemctl enable cputemp
```

### Troubleshooting

If the service fails to start:

```bash
# Check service status
sudo systemctl status cputemp

# Check logs
sudo journalctl -u cputemp -n 50

# Verify binary exists
ls -l /usr/local/bin/cputemp

# Test binary manually
sudo /usr/local/bin/cputemp --sensor <your_sensor> --temp 80 --verbose

# Check sensor name is correct
/usr/local/bin/cputemp  # Lists available sensors
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

## Logs
```bash
journalctl -u cputemp.service -f
```
```bash
May 21 20:32:33 proxmox cputemp[1027]: THROTTLE: Temperature 81.000000 C exceeds target 80.000000 C, reducing frequency from 2600.000000 MHz to 2560.000000 MHz
May 21 20:32:34 proxmox cputemp[1027]: THROTTLE: Temperature 81.000000 C exceeds target 80.000000 C, reducing frequency from 2560.000000 MHz to 2540.000000 MHz
May 21 20:32:37 proxmox cputemp[1027]: THROTTLE: Temperature 81.000000 C exceeds target 80.000000 C, reducing frequency from 2620.000000 MHz to 2560.000000 MHz
May 21 20:32:38 proxmox cputemp[1027]: THROTTLE: Temperature 81.000000 C exceeds target 80.000000 C, reducing frequency from 2560.000000 MHz to 2540.000000 MHz
May 21 20:32:53 proxmox cputemp[1027]: THROTTLE: Temperature 81.000000 C exceeds target 80.000000 C, reducing frequency from 2600.000000 MHz to 2560.000000 MHz
May 21 20:33:03 proxmox cputemp[1027]: THROTTLE: Temperature 81.000000 C exceeds target 80.000000 C, reducing frequency from 2660.000000 MHz to 2600.000000 MHz
May 21 20:33:04 proxmox cputemp[1027]: THROTTLE: Temperature 82.000000 C exceeds target 80.000000 C, reducing frequency from 2600.000000 MHz to 2540.000000 MHz
May 21 20:33:05 proxmox cputemp[1027]: THROTTLE: Temperature 82.000000 C exceeds target 80.000000 C, reducing frequency from 2540.000000 MHz to 2500.000000 MHz
```


## License

Zero-Clause BSD (public domain)
