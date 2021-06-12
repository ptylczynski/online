# Collabora Online docker

This directory contains everything needed to compile a
working docker container with Collabora Online.

Docker image can be built from packages or from source code.

### Examples:

1. Build latest CODE based on Ubuntu 18.04 LTS

  ```bash
  cd from-packages
  docker build --no-cache -t mydomain/code -f Ubuntu .
  ```

2. Build Collabora Online 6.4

  ```bash
  cd from-packages
  docker build --no-cache --build-arg type=cool --build-arg secret_key=<....> -t mydomain/cool -f Ubuntu .
  ```

3. Build Collabora Online 4.2 based on RHEL8

  ```bash
  cd from-packages
  docker build --no-cache --build-arg version=4.2 --build-arg type=cool --build-arg secret_key=<....> -t mydomain/cool -f RHEL8 .
  ```
  *Get your secret URL key from https://support.collaboraoffice.com/ (Collabora Partners/Customers)*

4. Build Collabora Online 6.4 license key enabled version

   ```bash
   cd from-packages
   docker build --no-cache --build-arg type=key -t mydomain/cool -f Ubuntu .
   ```

5. Build Collabora Online from master branch (from source code)

   ```bash
   cd from-source
   ./build.sh
   ```

**Check build.sh for more build options!**