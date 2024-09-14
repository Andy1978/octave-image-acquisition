#!/bin/bash -e

OCMD='pkg load image-acquisition; system ("head -n2 /etc/os-release"); ver; test @videoinput/videoinput;test @videoinput/get;test @videoinput/set;test @videoinput/getsnapshot;test imaqhwinfo;'
echo "octave command: ${OCMD}"
echo

do_run () {
  # Info: If you want to play in an interactive shell, just add --persist before --eval
  docker run --device /dev/video0 -it "$1" octave -q --eval "$OCMD"
  echo "---"
}

do_run debian_bullseye
do_run debian_bookworm
do_run debian_bookworm_default
do_run fedora31
do_run fedora33
do_run fedora38
do_run ubuntu_20_04
do_run ubuntu_22_04
do_run ubuntu_23_04
do_run ubuntu_24_04
