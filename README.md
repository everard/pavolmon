# DESCRIPTION
This repository contains the source code for a simple stand-alone zero-config PulseAudio input/output sound volume monitor.
This program might come in handy if you are using something similar to i3blocks.

# COMPILATION
Simply run `make` in your command line.

Note: A C++17-capable compiler is required; PulseAudio development headers must be installed.

# INSTALLATION [OPTIONAL]
Run the following command to copy the compiled binary to your /usr/local/bin/ directory.

```
sudo make install
```

# UNINSTALLATION
To remove the compiled binary from your system run the following command.

```
sudo make uninstall
```

# USAGE
To show the help message, run:

```
pavolmon -h
```

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

# LICENSE
Copyright Nezametdinov E. Ildus 2019.

Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

