# M365 Dashboard replacement

## Description

Replacement for BLE m365 scooter dashboard using ESP32 chip.

## Build

Clone official esp-idf sdk repo and set `IDF_PATH` environment variable
to point it.

```bash
    git clone --recursive https://github.com/espressif/esp-idf.git ~/esp-idf

    cd ~/esp-idf && ./install.sh && . ./export.sh
```

Build app and watch the info using `make`.

```bash
    make flash monitor
```

