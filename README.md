# Descriptions

The payload of the Cleaner is injected into the host process, and it kills the child processes created by the host after the process is stopped.

## Usage

### Parameters

- Arg1: Command line to execute. (The full path of the executable file is required)
- Arg2: The full path of the dynamic library to be injected.

### Example

> e.g. Adobe Photoshop creates some child processes on startup and does not clean them up on exit. (So, Fuck you Adobe!)

The simple way is to create a shortcut which execute the following command to replace the original one.

```shell
Launcher.exe "C:\Adobe\Adobe Photoshop 2021\Photoshop.exe" "Payload.dll"
```
