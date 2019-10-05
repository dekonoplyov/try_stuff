#!/bin/bash

XEPHYR=$(whereis -b Xephyr | cut -f2 -d' ')
xinit ./win_man.sh -- \
    "$XEPHYR" \
        :100 \
        -ac \
        -screen 800x600 \
        -host-cursor