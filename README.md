## pachanh-new
Next generation of Hanh Linux package manager. 

pachanh-new will be POSIX C and `sh` compatible.
### Installation
#### Dependencies
**Runtime binaries**
- `sed`
- `rm`
- `echo`
- `tee`
- `find`
- `tar`
- `xz`
- `mkdir`
- `cat`
- `diff`
- `ln`
- `readlink`
- `sh` 
- `mv`
- `cp`
- `type` (`posh` may not work properly, better disable build flavor)

**Buildtime binaries**
- `install`
- A C compiler (`clang` is used by default)
- A linker (`-fuse-ld=<linker>`, depends on compiler support)

**Library**: Tested with `glibc` or `musl`, should work with any libc that provides POSIX C headers
#### Compilation
- Clone this repo and `cd` to it
- Run `./install-script action=HELP` to get more infomation 
```
Usage : ./install-script action=[action] (var=val)
Actions:
HELP                Print this message
CHECK               Check for commands
COMPILE             Compile source code
INSTALL             Install package
CLEAN               Clean current build
INSTALL_MIRROR      Install available mirrors
If $action is empty, it will run COMPILE by default
Build variables:
CC=<binary>         C compiler (default clang)
LD=<binary>         Linker (default ld.lld)
CFLAGS=<flags>      C compiler flags
PREFIX=<dir>        Install prefix (default /usr/local)
DESTDIR=<dir>       Install directory (default /)
SYSROOT=<dir>       Package root directory (default /)
CONFDIR=<dir>       Package config directory (default $SYSROOT/etc)
Options:
keepcfg=<val>       Keep old config
verbose=<val>       Verbose
You can create a .config file so that it will be used for compilation
```

