#!/bin/bash

set -e
brew install \
  cmake \
  docbook-xsl \
  wxwidgets \
  fftw wavpack \
  gettext \
  imagemagick \
  jack \
  pkg-config \
  yaml-cpp
brew link gettext --force

# A workaround of https://gitlab.kitware.com/cmake/cmake/-/issues/23826
for F in $(grep -l '(, weak)\?'  /usr/local/Cellar/cmake/*/share/cmake/Modules/GetPrerequisites.cmake); do
  echo Patching $F...
  # OSx sed does not support the -i option
  sed -e 's/(, weak)\?/(, (weak|reexport))?/' $F >${F}.new
  mv ${F}.new $F
done
