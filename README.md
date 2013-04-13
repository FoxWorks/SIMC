Simulation Core
--------------------------------------------------------------------------------
SIMC contains shared source code for EVDS/IVSS/RDRS simulation libraries. It is
a required dependency of all libraries and additionally contains shared code
for Premake4 scripts.

Features
--------------------------------------------------------------------------------
 - Basic C interface to reading/writing XML files
 - Basic threading (wrap around WinAPI and pthreads)
 - Mutexes (one-entry locks)
 - Slim read-write locks (multi-reader locks, WinAPI or custom)
 - Provides precise time in seconds
 - Provides precise date as MJD
 - Provides logical processor count
 - Time delay/thread switching (wrap around WinAPI `Sleep()` and `SwitchToThread()`)
 - Linked list (SRW-lock based, thread safe for multiple readers and one writer)
 - Queue (thread safe for one reader and one writer)

Compiling
--------------------------------------------------------------------------------
Requires TinyXML for XML support. The repository must be cloned recursively to
include TinyXML as a submodule:
```
git clone --recursive https://github.com/FoxWorks/SIMC.git
```

Build files must be generated using [Premake4](http://industriousone.com/premake):
```
cd support
premake4 vs2008
```

See [Premake4 documentation](http://industriousone.com/premake-quick-start) for
more information on available options and platforms.
