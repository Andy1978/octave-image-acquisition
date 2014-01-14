#!/bin/bash
find .. \( -name "*.m" -or -name "*.c." -or -name "*.h" \) -exec sed -i 's/[[:space:]]*$$//' {} \;
