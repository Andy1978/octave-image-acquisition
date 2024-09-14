#!/bin/bash -e

cp ../image-acquisition-*.tar.gz .

## Debian bullseye
docker build -f Dockerfile_debian_bullseye -t debian_bullseye .

## Debian bookworm
docker build -f Dockerfile_debian_bookworm -t debian_bookworm .

## Debian bookworm and default branch (aka development)
docker build -f Dockerfile_debian_bookworm_default_branch -t debian_bookworm_default .

## Fedora 31
docker build -f Dockerfile_fedora31 -t fedora31 .

## Fedora 33
docker build -f Dockerfile_fedora33 -t fedora33 .

## Fedora 38
docker build -f Dockerfile_fedora38 -t fedora38 .

## Ubuntu 20.04, EOL Apr 2025
## The installed kernel is video-node-centric
## See https://www.kernel.org/doc/html/v6.1/userspace-api/media/v4l/open.html
## which describes the media controller API for newer kernels
docker build -f Dockerfile_ubuntu_20_04 -t ubuntu_20_04 .

## Ubuntu 22.04
docker build -f Dockerfile_ubuntu_22_04 -t ubuntu_22_04 .

## Ubuntu 23.04
docker build -f Dockerfile_ubuntu_23_04 -t ubuntu_23_04 .

## Ubuntu 24.04
docker build -f Dockerfile_ubuntu_24_04 -t ubuntu_24_04 .

rm -f image-acquisition-*.tar.gz
