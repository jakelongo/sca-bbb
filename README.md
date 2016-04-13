Experiment `playground' for conducting side-channel research on
the BeagleboneBlack platform.

# Build

We are using Debian Wheezey throughout, however, any version running on
the BBB should work just the same.

First things first, get all the stuff running on the BBB. Clone the repo:

```bash
git clone https://github.com/jakelongo/sca-bbb.git
```

Pull in OpenSSL:

```bash
git submodule update --init
```

Build and install dependencies for wolfSSL:
```bash
sudo apt-get install autoconf automake libtool
```

Build wolfSSL:
```bash
./configure
make
make check
```

Patch OpenSSL:

```bash
patch -p0 < openssl.patch
```

Build OpenSSL and optionally set flags from the build table below:
```bash
cd openssl
./build [lib] [defs]
make depend
make
sudo make install_sw
```

#### OpenSSL linker and pre-processor flags:

|  lib       |  defs       |                     Notes                    |
|:----------:|:-----------:|:--------------------------------------------:|
| -lwiringPi | -DAES_TRIG  | Trigger placed around each AES enc/dec call. |


---

Contents of this repo are as follows:

* openssl/ - OpenSSL 1.0.1s @57ac73f from
  https://github.com/openssl/openssl.git

* openssl.patch - changes to the openssl source to aid in the SCA signal
  exploration phase

* generatePatch.sh - generates the openssl.patch file (only for setup purposes)
