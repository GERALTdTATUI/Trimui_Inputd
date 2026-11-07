# Trimui Input Daemon

User-space input daemon for the Trimui family (Smart Pro, Smart Pro S, Brick). It reads the device-specific GPIO/serial inputs, publishes a unified uinput gamepad, and supports rumble on the supported A133-based hardware.

## Features

- Auto-detects Smart Pro, Smart Pro S, or Brick at startup.
- Maps physical buttons, triggers, sticks, and hat to a single virtual gamepad.
- Rumble for A133-based units through the GPIO 227 and Smart Pro S PWM-vibrator backends.
- Upstream joystick calibration files with per-axis min/max/center values and normalized deadzone support.
- Learned calibration fallback when no upstream calibration file is available.
- Support for button remap functions using configuration files.

## Building

The repo ships with a cross-compilation container. From the project root:

```bash
docker compose up --build
```

If you are using an x86 host, you might need to run this first:

```bash
docker run --privileged --rm tonistiigi/binfmt --install arm64
```

If you prefer a native toolchain, install `gcc`, `make`, and standard headers for your aarch64 rootfs, then run:

```bash
make clean && make
```

The native build outputs to `build/trimui_inputd/bin/trimui_inputd`.

## Running

- Requires access to `/dev/uinput`, serial ports (`/dev/ttyS3/4` or `/dev/ttyAS5/7`), `/dev/mem` (Brick GPIO), and `/sys/class/gpio` (A133 rumble).
- Run the built binary as root (or with the needed capabilities):

```bash
sudo ./build/trimui_inputd/bin/trimui_inputd
```

The daemon auto-detects the connected Trimui variant and starts polling.

## Configuration

- Smart Pro button remapping reads `/mnt/UDISK/trimui_inputd.conf`; set `TRIMUI_INPUTD_CONFIG` to use another path. Lines use `CONTROL=CODE`, with `A`, `B`, `X`, `Y`, `L1`, `R1`, `L2`, `R2` `SELECT`, `START`, and `MODE` as controls. Supported key codes are `BTN_EAST`, `BTN_SOUTH`, `BTN_NORTH`, `BTN_WEST`, `BTN_TL`, `BTN_TR`, `BTN_TL2`, `BTN_TR2`, `BTN_SELECT`, `BTN_START`, `BTN_MODE`, `BTN_THUMBL`, and `BTN_THUMBR`. Defaults use the Western layout (A=South, B=East, X=West, Y=North). On Trimui Smart PRO `L2` and `R2` remain analog-style trigger axes, so remapping configs differ from the rest as `L2` and `R2` codes represent axis codes `ABS_Z` and `ABS_RZ` instead of button codes `BTN_TL2`, `BTN_TR2`, this means that `L2` and `R2`, right now, can just be mapped to each other, not being able to act as the other buttons.

As previous said, a config mapping file can be set on a default or custom location, a normal config file would look like this example:

```ini
# Western defaults are already active; these lines are optional.
A=BTN_SOUTH
B=BTN_EAST
X=BTN_WEST
Y=BTN_NORTH
L1=BTN_TL
R1=BTN_TR
L2=BTN_TL2 # Only Trimui Brick and Smart PRO S accept this mappings
Rr=BTN_TR2 # Only Trimui Brick and Smart PRO S accept this mappings

# Swap Select and Mode.
SELECT=BTN_SELECT
START=BTN_START
MODE=BTN_MODE

#All the buttons above can be mapped into any permutation of themselves

# Swap the two trigger axes, if desired.
L2=ABS_Z  # Only Trimui Smart PRO sees L2 as axis, and cant just be mapped to R2
R2=ABS_RZ # Only Trimui Smart PRO sees R2 as axis, and cant just be mapped to L2

```

- Alternative remapping function using `/tmp/trimui_inputd/swap_ab` and `/tmp/trimui_inputd/swap_xy`: In orther to improve compatibility with previous Trimui Smart PRO FW versions, that creates files into `/tmp/trimui_inputd/` folder to use as variables, a functon to monitor the existance of a `swap_ab` file were added to remap the A and B inputs. Also a new function to invert X and Y were added, wich monitors the existance of a `swap_xy` file. The main remap config file on `/mnt/UDISK/trimui_inputd.conf` sets a global remap configuration which is set at the startup of the daemon and will be fixed until deamon gets restarted. The `/tmp/trimui_inputd/swap_*` file are dinamically monitored and it's changes take affect without the need to restart the daemon. Also, the `swap_*` files inverts the logic set by the default config file, so if a inverted A/B remap were set on `/mnt/UDISK/trimui_inputd.conf`, creating a `swap_ab` file would revert the configuration.
- `BRICK_ACTIVE_LOW=0` forces Brick buttons to be treated as active-high (defaults to active-low).
- Joystick calibration lookup checks the first available file in this order: `/etc/trimui/joypad*.config`, `/userdata/system/joypad*.config` (Batocera), then `/mnt/UDISK/joypad*.config` (stock BSP). The concrete filenames are `joypad.config` for the left stick and `joypad_right.config` for the right stick. Override the paths with `TRIMUI_JOYPAD_CONFIG` and `TRIMUI_JOYPAD_RIGHT_CONFIG`. Each file uses `x_min`, `x_max`, `y_min`, `y_max`, `x_zero`, `y_zero`, and a normalized `deadzone` (for example `0.10`). Missing files retain the learned calibration fallback.
- Rumble uses GPIO 227 on the A133 backend and the PWM-vibrator backend on Smart Pro S.

## Repo layout

- `src/devices/`: Device-specific logic (Smart Pro, Smart Pro S, Brick) and shared rumble core.
- `src/drivers/`: Low-level helpers (serial, GPIO, sunxi GPIO mmap, rumble drivers).
- `src/gamepad/`: uinput wrapper.
- `src/calibration/`: Upstream and learned stick calibration helpers.