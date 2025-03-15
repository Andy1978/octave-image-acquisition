#!/bin/bash
find .. \( -name "*.c" -or -name "*.cpp" -or -name "*.h" \) -exec astyle --style=gnu -s2 -n {} \;
