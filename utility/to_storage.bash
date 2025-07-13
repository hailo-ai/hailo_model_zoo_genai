#!/bin/bash

readonly storage=$1
shift

sha256sum $@ | while read -r digest file; do
  echo ${file} ${digest}
  ln -s $(realpath --no-symlinks ${file}) ${storage}/sha256_${digest}
done
