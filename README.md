# LiniTunes
Acronym for : LiniTunes is not iTunes

The aim of this project is to give Linux users (maybe Windows and macOS as well) a GUI app to manage their iDevice.
After that, the goal is to add the ability to listen to their music with Apple Music (maybe)

This will use lots of open-source project, and since I'm new to GUI app on Linux, I need to learn.
I use Qt6+QML for this project.
If you want to contribute, you can open an issue, a request, a suggestion, or submit a pull request.

This project is in its early developments. Everything could change at any time given.

Looks like this for now :
<img width="982" height="690" alt="Capture d’écran du 2026-07-07 16-06-45" src="https://github.com/user-attachments/assets/53443185-29ca-4914-867f-9976d5e1d8a2" />

This project is under the GPLv3 licence.

## Linux Wi-Fi sync

Linux AppImage builds bundle `netmuxd` so Wi-Fi device discovery can work even when it is not installed system-wide.

At startup, LiniTunes uses an existing netmuxd at `127.0.0.1:27015` when available. Otherwise, it starts the bundled helper in shim mode when system `usbmuxd` is available, or standalone mode when it is not. Standalone mode stores pairing records in `~/.local/share/LiniTunes/lockdown`.

If standalone mode cannot access USB devices, your system may need udev/USB permissions for Apple devices. Advanced overrides:

- `LINITUNES_NETMUXD_ADDRESS=host:port` uses an external netmuxd and skips the bundled helper.
- `LINITUNES_DISABLE_BUNDLED_NETMUXD=1` disables the bundled helper.
- `LINITUNES_NETMUXD_HELPER=/path/to/netmuxd` uses a custom helper binary.

## Known issues

- Storage sync in network/Wi-Fi mode is painfully slow

## Thanks to 
 - [@nikias](https://github.com/nikias) for [libimobiledevice](https://github.com/libimobiledevice/libimobiledevice)
 - [@jkcoxson](https://github.com/jkcoxson) for [idevice](https://github.com/jkcoxson/idevice)
 - [@cj123](https://github.com/cj123) for [ipsw.me](https://ipsw.me/) and its API
 - Apple for iOS, iPadOS, etc
