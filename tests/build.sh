#!/bin/bash -e

cp ../image-acquisition-*.tar.gz .

## Debian bullseye
## GNU Octave Version: 6.2.0 (hg id: 5cfdf97c5350)
docker build -f Dockerfile_debian_bullseye -t debian_bullseye .

## Debian bookworm
## GNU Octave Version: 7.3.0 (hg id: 2c037ce00450)
docker build -f Dockerfile_debian_bookworm -t debian_bookworm .

## Debian bookworm and default branch (aka development)
## GNU Octave Version: 10.0.0 (hg id: b2816ce46619) as of 2024-09-14, this will change
docker build -f Dockerfile_debian_bookworm_default_branch -t debian_bookworm_default .

## Fedora 31
## GNU Octave Version: 5.1.0 (hg id: d05d6eebde10)
docker build -f Dockerfile_fedora31 -t fedora31 .

## Fedora 33
## GNU Octave Version: 5.2.0 (hg id: eb46a9f47164)
docker build -f Dockerfile_fedora33 -t fedora33 .

## Fedora 38
## GNU Octave Version: 7.3.0 (hg id: 2c037ce00450)
docker build -f Dockerfile_fedora38 -t fedora38 .

## Ubuntu 20.04, EOL Apr 2025
## The installed kernel is video-node-centric
## See https://www.kernel.org/doc/html/v6.1/userspace-api/media/v4l/open.html
## which describes the media controller API for newer kernels
## GNU Octave Version: 5.2.0 (hg id: eb46a9f47164)
docker build -f Dockerfile_ubuntu_20_04 -t ubuntu_20_04 .

## Ubuntu 22.04
## GNU Octave Version: 6.4.0 (hg id: 8d7671609955)
docker build -f Dockerfile_ubuntu_22_04 -t ubuntu_22_04 .

## Ubuntu 23.04
## GNU Octave Version: 7.3.0 (hg id: 2c037ce00450)
docker build -f Dockerfile_ubuntu_23_04 -t ubuntu_23_04 .

## Ubuntu 24.04
## GNU Octave Version: 8.4.0 (hg id: 78c13a2594f3)
docker build -f Dockerfile_ubuntu_24_04 -t ubuntu_24_04 .

rm -f image-acquisition-*.tar.gz


