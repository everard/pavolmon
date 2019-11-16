# DESCRIPTION
This repository contains the source code for a simple stand-alone zero-config PulseAudio input/output sound volume monitor.
This program has _very_ minimal RAM/CPU footprint and might come in handy if you are using [i3blocks](https://github.com/vivien/i3blocks).

# COMPILATION/INSTALLATION
Run `make` in your command line to compile.

Note: A C11-capable compiler is required; PulseAudio development headers must be installed.

The compiled binary can be copied to the `/usr/local/bin/` directory using the following command.

```
sudo make install
```

The installed binary can be removed from the `/usr/local/bin/` directory using the following command.

```
sudo make uninstall
```

# USAGE

## Command line
The program's help message can be viewed using the `pavolmon -h` command.

To start monitoring the sound volume, run:

```
pavolmon
```

To use custom labels for speaker and microphone, run (as an example):

```
pavolmon -f "VOL: " "VOL[MUTED]: " "MIC: " "MIC[MUTED]: "
```

Another example (if your terminal supports Unicode):

```
pavolmon -f "🔊 " "🔊ₓ" "🎤 " "🎤ₓ"
```

## i3blocks
Add the following block to the [i3blocks](https://github.com/vivien/i3blocks) config file:

```
[pavolmon]
command=pavolmon -f "🔊 " "🔊ₓ" "🎤 " "🎤ₓ"
interval=persist
```

# LICENSE
Copyright Nezametdinov E. Ildus 2019.

Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
