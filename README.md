## pachanh-new
Next generation of Hanh Linux package manager. 

pachanh-new will be POSIX C and `sh` compatible.
### Installation
> WARNING: The project is still testing. Try at your own risks
#### Dependencies
**Runtime binaries**
- `sed`
- `rm`
- `echo`
- `tee`
- `find`
- `tar`
- `gzip` (moving database archives to `xz`)
- `xz`
- `mkdir`
- `cat`
- `diff`
- `ln`
- `readlink`
- `sh` 
- `mv`
- `cp`

**Buildtime binaries**
- `install`
- A C compiler (`clang` is used by default)

**Library**: Tested with `glibc` or `musl`.
#### Compilation
- Clone this repo and `cd` to it
- Run `./install-script action=HELP` to get more infomation 
```
Usage : ./install-script action=[action] (var=val)
Action:
HELP                Print this message
CHECK               Check for commands
COMPILE             Compile source code
INSTALL             Install package
CLEAN               Clean current build
If $action is empty, it will run COMPILE by default
Build variable:
CC=<binary>         C compiler (default clang)
CFLAGS=<flags>      C compiler flags 
PREFIX=<dir>        Install prefix (default /usr/local)
DESTDIR=<dir>       Install directory (default /)
SYSROOT=<dir>       Package root directory (default /)
CONFDIR=<dir>       Package config directory (default $SYSROOT/etc)
You can create a .config file so that it will be used for compilation
```
#### Usage
Read documentation inside `doc` directory.
