#!/bin/bash
find .. \( -name "*.m" -or -name "*.cc" -or -name "*.h" \) -exec sed -i 's/[[:space:]]*$//' {} \;
