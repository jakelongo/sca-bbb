Experiment `playground' for conducting side-channel research on
the BeagleboneBlack platform.

# Build

We are using Debian Wheezey throughout, however, any version running on
the BBB should work just the same.

First things first, get all the stuff running on the BBB. Clone the repo:

```bash
git clone https://github.com/jakelongo/sca-bbb.git
```

Initialise all target implementation submodules:
```bash
git submodule update --init
```

## OpenSSL target

Patch OpenSSL with optionsl build flags below:

```bash
patch -p0 < openssl.patch
```

Build OpenSSL and optionally set flags from the build table below:
```bash
cd openssl
./config [lib] [defs]
make depend
make
sudo make install_sw
```

#### openSSL linker and pre-processor flags:

|  lib       |  defs       |                     Notes                    |
|:----------:|:-----------:|:--------------------------------------------:|
|            | -DAES_TRIG  | Trigger placed around each AES enc/dec call. |


## wolfSSL target

Build and install dependencies for wolfSSL:
```bash
sudo apt-get install autoconf automake libtool
```

Patch wolfSSL with optionsl build flags below:
```bash
patch -p0 < wolfssl.patch
```

Build wolfSSL:
```bash
./configure CPPFLAGS=[defs] LDFLAGS=[lib]
make
make check
sudo make install
```

#### wolfSSL linker and pre-processor flags:

|  lib       |  defs       |                     Notes                    |
|:----------:|:-----------:|:--------------------------------------------:|
|            | -DAES_TRIG  | Trigger placed around each AES enc/dec call. |

---

Contents of this repo are as follows:

* openssl/ - OpenSSL 1.0.1s @57ac73f from
  https://github.com/openssl/openssl.git

* wolfSSL/ - wolfSSL @master from
  https://github.com/wolfssl/wolfssl.git

* trigger/ - trigger implementation source code

* openssl.patch - changes to the openssl source to aid in the SCA signal
  exploration phase

* wolf.patch - changes to the wolfssl source to aid in the SCA signal
  exploration phase

* generatePatch.sh - generates the openssl.patch and wolfssl.patch files (only for completeness)

* exportGPIO.sh - GPIO export script to allow for fast GPIO switching
