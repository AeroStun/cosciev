# cosciev

_COroutines with nvSCIEVent_.

C++20 coroutines sample implementation based on the NvSciEvent library.

See `demo.cxx` for a small example of cooperative concurrency.

## Dependencies

Besides the C++ standard library, _cosciev_ only relies on the NvSciEvent portion of nvsci,
Nvidia's software communication interface first developed for their automotive stack (DriveOS).

You can obtain and install nvsci using the [first-party x86_64 Debian package](https://repo.download.nvidia.com/jetson/x86_64/jammy/pool/main/n/nvsci/nvsci_7.0.0.0_amd64.deb).
