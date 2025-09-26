# MES
Mobile and Embedded Security

## Build

Prerequisites:
```bash
sudo apt update
sudo apt install -y g++ gcc cmake make gcovr pkg-config
```


### Release Binaries

```bash
mkdir release
cd release
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j
```

### Debug Binaries

Optimizations turned off, debugging symbols are included

```bash
mkdir debug
cd debug
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j
```

## Run the Applications

```bash
./release/KeyMgmnt/KeyMgmnt
./release/FWMgmnt/FWMgmnt
```

## Execute the Unit Tests

Either `ctest --output-on-failure` or run a test binary, e.g. `./debug/Libraries/DemoLib/DemoLibTest`.

### Generate a Test Coverage Report

FIXME: Does not work properly yet

Build the debug binary, then run

```bash
cmake --build debug --target coverage --config Debug
```

The coverage report is available in `./debug/DemoLib/coverage.html`

## Develop on remote node using local VS Code

VS Code supports to connect to a remote node, and to execute as if VS code was running on that remote machine, including compilation, debugging, code editing,...

- Preconditions: SSH server executing on the remote host, using ssh keys for authentication
- Install "Remote Development" extension pack from the VS Code extensions menu.
- Open Command Pallette via Ctrl-Shift-P
- Select "Remote-SSH: Connect to Host...", add `user@node` in the popup window, `user` being your account on the remote machine and `node` being the name or IP of the remote machine.
- A new window opens up, setting up VS Code for the remote node, this takes a few minutes
- Select "Linux" as platform
- You will be asked if you trust the remote node
- After the installation is done, open the project folder (VS Code will display the folders on the remote machine).
- When compiling, debugging, it may be necessary to (re-) install some of the extensions, C++, CMake, ... "Install in SSH: <node>" to have it available.
- As a shortcut, switch to the "Remote Explorer" View in VS Code, select "Remotes (Tunnels/SSH)" in the top combo box, use the "+" icon to add a new remote host to connect to, and add the ssh command, e.g. `ssh uer@node`. The configuration is stored and can be actrivated in the future from the Remotes View.

See also:
- https://code.visualstudio.com/docs/remote/ssh
- https://learn.microsoft.com/en-us/training/modules/develop-on-remote-machine/
