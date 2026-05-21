Photon Core
=============

Setup
---------------------
Photon Core is the Photon full node and wallet software. It downloads and, by default, stores the full Photon chain history. Depending on the speed of your computer and network connection, the synchronization process can take time to complete.

To download Photon Core, visit the Photon project repository.

Running
---------------------
The following are some helpful notes on how to run Photon Core on your native platform.

### Unix

Unpack the files into a directory and run:

- `bin/photon-qt` (GUI) or
- `bin/photond` (headless)

### Windows

Unpack the files into a directory, and then run photon-qt.exe.

### macOS

Drag Photon Core to your applications folder, and then run Photon Core.

### Need Help?

* See the Photon project repository for help and more information.

Building
---------------------
The following are developer notes on how to build Photon Core on your native platform. They are not complete guides, but include notes on the necessary libraries, compile flags, etc.

- [Dependencies](dependencies.md)
- [macOS Build Notes](build-osx.md)
- [Unix Build Notes](build-unix.md)
- [Windows Build Notes](build-windows.md)
- [FreeBSD Build Notes](build-freebsd.md)
- [OpenBSD Build Notes](build-openbsd.md)
- [NetBSD Build Notes](build-netbsd.md)
- [Android Build Notes](build-android.md)

Development
---------------------
The Photon repo's [root README](/README.md) contains relevant information on the development process and automated testing.

- [Developer Notes](developer-notes.md)
- [Productivity Notes](productivity.md)
- [Release Process](release-process.md)
- [Source Code Documentation (External Link)](https://doxygen.bitcoincore.org/)
- [Translation Process](translation_process.md)
- [Translation Strings Policy](translation_strings_policy.md)
- [JSON-RPC Interface](JSON-RPC-interface.md)
- [Unauthenticated REST Interface](REST-interface.md)
- [Shared Libraries](shared-libraries.md)
- [BIPS](bips.md)
- [Dnsseed Policy](dnsseed-policy.md)
- [Benchmarking](benchmarking.md)
- [Internal Design Docs](design/)

### Resources
* Discuss project-specific development through the Photon project channels.

### Miscellaneous
- [Assets Attribution](assets-attribution.md)
- [photon.conf Configuration File](photon-conf.md)
- [CJDNS Support](cjdns.md)
- [Files](files.md)
- [Fuzz-testing](fuzzing.md)
- [I2P Support](i2p.md)
- [Init Scripts (systemd/upstart/openrc)](init.md)
- [Managing Wallets](managing-wallets.md)
- [Multisig Tutorial](multisig-tutorial.md)
- [P2P bad ports definition and list](p2p-bad-ports.md)
- [PSBT support](psbt.md)
- [Reduce Memory](reduce-memory.md)
- [Reduce Traffic](reduce-traffic.md)
- [Tor Support](tor.md)
- [Transaction Relay Policy](policy/README.md)
- [ZMQ](zmq.md)

License
---------------------
Distributed under the [MIT software license](/COPYING).
