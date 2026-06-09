<p align="center">
  <img src="src/qt/res/icons/photon.png" alt="Photon" width="95">
</p>

# Photon Core 0.25.2

Photon Core 0.25.2 is the Photon aux-chain port of Bitcoin Core v25.2.
It keeps Photon's Blake-family chain identity, AuxPoW merged-mining rules,
and BLAKE-256 proof-of-work while adding the Taproot-era Core codebase,
descriptor-wallet support, SQLite wallet support, ZMQ, and Linux USDT
tracepoints for hardened release builds.

## Mainnet Consensus Changes In 0.25.2

Photon 0.25.2 is intended to follow the 0.15.21 SegWit mainnet activation
and then activate the next Bitcoin-compatible script rule sets in a staged
order. Pools and miners should use the daemon-provided AuxPoW block-template
version; do not manually rewrite version bits.

| Rule set | Mainnet policy in Photon 0.25.2 |
|---|---|
| SegWit (`BIP141` / `BIP143` / `BIP147`) | Already active from 0.15.21; buried at height `2051160`. No new SegWit signaling window in 0.25.2. |
| `BIP34` coinbase height | Height activation at `2072227`; `BIP34Hash = uint256{}`. |
| `BIP65` / CLTV | Height activation at `2072227`; required for standard CLTV atomic-swap refunds. |
| `BIP66` / strict DER | Height activation at `2072227`. |
| Taproot (`BIP340` / `BIP341` / `BIP342`) | BIP9 deployment bit `2`, start `1782871200` (`2026-07-01 02:00:00 UTC`), timeout `1814407200` (`2027-07-01 02:00:00 UTC`), minimum activation height `2075587`. |

Only Taproot is a future BIP9-signaled deployment in 0.25.2. `BIP34`,
`BIP65`, `BIP66`, and buried SegWit are height rules. Photon Core computes
the correct BIP9 top bits, Taproot bit `2`, AuxPoW flag, and Photon chain-ID
bits in block templates.

## About Photon

Photon is a BlakeStream AuxPoW chain merge-mined under the Blakecoin parent
chain. It is a peer-to-peer digital currency with no central authority.

- Uses the Blake-256 hashing algorithm, 8 rounds
- Based on Bitcoin Core v25.2
- Uses AuxPoW / merged mining with chain ID `0x0002`
- Uses the autotools build system (`./autogen.sh`, `./configure`, `make`)
- Supports legacy Berkeley DB wallets and descriptor SQLite wallets
- Keeps Photon txids on single SHA-256
- Uses HASH256/double SHA-256 for witness-v0 BIP143 signing
- Keeps BIP340/BIP341/BIP342 Taproot tagged hashes byte-compatible with Bitcoin

| Network Info | Value |
|---|---|
| Algorithm | Blake-256, 8 rounds |
| Block time | 3 minutes |
| Block reward | 32,768 PHO base plus legacy difficulty-weighted increment |
| Difficulty retarget | Every 20 blocks |
| Coinbase maturity | 120 blocks |
| AuxPoW chain ID | `0x0002` |
| AuxPoW start height | `160000` on mainnet; active from genesis on testnet/regtest |
| Max supply policy | 90,000,000,000 PHO target supply policy |
| Default P2P port | `35556` |
| RPC port | `8984` |
| Mainnet genesis | `000000e79a20d718a2f2d8b98161dc6700104a22d8e9be70e8ac361ee6664b9c` |
| Mainnet Bech32 HRP | `pho` |
| Testnet Bech32 HRP | `tpho` |
| Regtest Bech32 HRP | `rpho` |

## Network Activation Notes

The SegWit activation block is height `2051160`.

Testnet is treated as a 0.25.2 feature-test/reset network. SegWit, BIP34,
BIP65/CLTV, BIP66, and Taproot are active from height `1` on testnet so
wallet, merged-mining, atomic-swap, Lightning, and Taproot behavior can be
tested without waiting on mainnet-style activation windows.

Regtest keeps override support through `-testactivationheight=` and
`-vbparams=` for activation-cycle tests. AuxPoW is available from genesis on
regtest.

## Quick Start

```bash
./build.sh --help
```

For most users, downloading a tested release artifact from GitHub Releases is
the simplest path. Use `build.sh` to build release artifacts locally.

For configuration options, see [config-help.md](config-help.md).

## Upgrade Notes

Before starting Photon Core 0.25.2 on an existing data directory, close the
older wallet cleanly and back up any wallet files first.

When syncing 0.25.2 from old 0.8/0.15.21-era chains, header presync can look slow or restart because v25 verifies low-work header chains before storing them. For trusted bootstrap only, use `-minimumchainwork=0 -connect=<trusted-node>` and remove those options after the node catches up.

`peers.dat` is only the cached P2P address database. It is safe to remove or
rename when moving between major releases, and Photon will rebuild it on the
next start. If startup fails with `Invalid or corrupt peers.dat`, remove or
rename this file:

- Windows: `%APPDATA%\Photon\peers.dat`
- Linux: `~/.photon/peers.dat`
- macOS: `~/Library/Application Support/Photon/peers.dat`

Windows PowerShell example:

```powershell
Rename-Item "$env:APPDATA\Photon\peers.dat" "peers.dat.bak"
```

Linux example:

```bash
mv ~/.photon/peers.dat ~/.photon/peers.dat.bak
```

macOS example:

```bash
mv "$HOME/Library/Application Support/Photon/peers.dat" \
   "$HOME/Library/Application Support/Photon/peers.dat.bak"
```

If the block index or chainstate database cannot be reused after an upgrade,
restart once with `-reindex` to rebuild the local block database from the
stored block files:

```bash
photond -reindex
```

Pruning is disabled by default (`-prune=0`), so a normal Photon Core node keeps
full block data. Public release nodes, explorers, pools, and bridge/watch
services should run unpruned unless they have a specific reason to discard old
block data.

For first-run testing of a new 0.25.2 build, use an isolated data directory so
the test does not touch an existing 0.15.21 wallet or chainstate:

```bash
photon-qt -datadir=/path/to/photon-25.2-test
```

## Sending Large Amounts

Photon 0.25.2 enforces a consensus money-range cap of **10,000,000,000 PHO**
(`MAX_MONEY`), applied per output and per transaction, so a single transaction
cannot move more than 10,000,000,000 PHO to one address.

To pay one address more than the cap, use the included helper, which splits the
payment into several transactions (each well under the cap) via the wallet's
`sendtoaddress`:

```bash
python3 contrib/send-big-pho.py <address> <total_PHO_amount>
# e.g. 25 billion PHO, sent as three transactions:
python3 contrib/send-big-pho.py PhotonAddressHere 25000000000
```

Options: `--chunk` (max PHO per transaction, default `9000000000`), `--datadir`,
`--delay` (seconds between transactions), and `--yes` (skip the confirmation
prompt, for non-interactive use). The helper validates the address, checks the
balance, prints each txid, and logs progress so an interrupted run resumes
without re-sending. Requires `python3` and `pip install requests`, run on the
same host as `photond`.

## Build Options

```bash
./build.sh [PLATFORM] [TARGET] [OPTIONS]

Platforms:
  --native          Build natively on this machine (Linux, macOS, or Windows)
  --appimage        Build portable Linux AppImage (requires Docker)
  --windows         Cross-compile for Windows from Linux (requires Docker)
  --macos           Cross-compile for macOS from Linux (requires Docker)

Targets:
  --daemon          Build daemon only (photond + photon-cli + photon-tx)
  --qt              Build Qt wallet only (photon-qt)
  --both            Build daemon and Qt wallet (default)

Docker options:
  --pull-docker     Pull prebuilt Docker images from Docker Hub
  --build-docker    Build Docker images locally from repo Dockerfiles
  --no-docker       For --native on Linux: build directly on the host

Other options:
  --hardened-release
                   Native Linux release profile: enable SQLite, ZMQ, and USDT
                   and fail the build if configure disables any of them
  --jobs N          Parallel make jobs
```


<!-- BEGIN electrium-build -->
### Electrium Wallet

Build the Photon ([Electrium](https://github.com/SidGrip/Blakestream-Electrium)) wallet by
choosing a target (linux/windows build in an **amd64** container, so any amd64 Docker host — Linux,
Windows, or an Intel Mac — can build either; only the macOS app needs a Mac):

```bash
./build-electrum.sh linux      # Linux AppImage    (amd64 Docker host)
./build-electrum.sh windows    # Windows .exe      (amd64 Docker host)
./build-electrum.sh macos      # macOS .dmg/.app   (on a Mac)
./build-electrum.sh all        # everything buildable on this host
```

Artifacts land in `outputs/Electrium/PHO/`, named `Electrium-PHO-<version>`.

The wallet builder is the shared multicoin repo
**[SidGrip/Blakestream-Electrium](https://github.com/SidGrip/Blakestream-Electrium)** — it also builds
all six BlakeStream wallets at once (`build-single-wallets.sh`) and the ElectrumX **server** Docker
image (`build-electrumx.sh`). `build-electrum.sh` auto-clones it when no local checkout is found.
<!-- END electrium-build -->

## Platform Build Instructions

### Native Linux

```bash
./build.sh --native --both --no-docker
```

- Supported validation lanes: Ubuntu 20.04, 22.04, 24.04, and 26.04
- Public Linux release lane: Ubuntu 26.04
- Ubuntu 20.04 remains a test/compatibility lane and is not planned as a public
  prebuilt release artifact for 0.25.2
- Native Linux outputs are written under `outputs/Ubuntu-XX/`
- Berkeley DB 4.8 is bootstrapped into the repo cache for legacy wallet
  compatibility
- Dual-wallet builds enable both Berkeley DB and SQLite

Recommended hardened Ubuntu 26 release build:

```bash
DOCKER_NATIVE=sidgrip/native-base:26.04 \
  ./build.sh --native --both --build-docker --hardened-release --jobs 5
```

The hardened Linux release profile requires:

- `USE_BDB=true`
- `USE_SQLITE=true`
- `ENABLE_ZMQ=true`
- `ENABLE_USDT_TRACEPOINTS=true`

USDT runtime attach validation is Linux/eBPF-specific. macOS and Windows builds
do not fail release acceptance because they do not expose the Linux USDT
backend.

### Native Linux With Docker

Use `--pull-docker` to pull prebuilt images from Docker Hub, or
`--build-docker` to build the selected base image locally from the Dockerfiles
in `docker/`.

```bash
./build.sh --native --both --pull-docker
./build.sh --native --qt --pull-docker
./build.sh --native --daemon --pull-docker
DOCKER_NATIVE=sidgrip/native-base:26.04 \
  ./build.sh --native --both --build-docker --hardened-release --jobs 5
```

### AppImage

```bash
./build.sh --appimage --pull-docker
```

- Uses `sidgrip/appimage-base:22.04`
- Produces a portable Linux AppImage under `outputs/AppImage/`
- Intended for Ubuntu 22.04 and newer
- If the host lacks FUSE support, launch with `--appimage-extract-and-run`

### Windows

```bash
./build.sh --windows --both --pull-docker
```

- Windows release artifacts come from the MXE cross-compile container
- Uses `sidgrip/mxe-base:latest`
- Outputs are written under `outputs/Windows/`
- Native Windows builds are diagnostic only and are not the release lane because
  they do not package the same bundled DLL/runtime layout

### macOS

```bash
./build.sh --native --both --no-docker
```

- Official macOS release artifacts are built natively on macOS
- Native macOS builds use Homebrew dependencies
- Berkeley DB 4.8, SQLite, and ZMQ should be enabled
- USDT tracing is Linux/eBPF-specific and is not a macOS release gate

## Output Structure

```text
outputs/
├── AppImage/
│   ├── Photon-0.25.2-x86_64.AppImage
│   ├── README.md
│   └── build-info.txt
├── Macosx/
│   ├── Photon-Qt.app
│   ├── photon-cli-0.25.2
│   ├── photon-qt-0.25.2
│   ├── photon-tx-0.25.2
│   ├── photon-util-0.25.2
│   ├── photon-wallet-0.25.2
│   ├── photond-0.25.2
│   └── build-info.txt
├── Ubuntu-20/
│   ├── README.md
│   ├── photon-256.png
│   ├── photon-cli
│   ├── photon.conf
│   ├── photon.desktop
│   ├── photon-qt
│   ├── photon-tx
│   ├── photon-util
│   ├── photon-wallet
│   ├── photond
│   ├── build-info.txt
│   └── install-deps.sh
├── Ubuntu-22/
├── Ubuntu-24/
├── Ubuntu-26/
└── Windows/
    ├── photon-cli-0.25.2.exe
    ├── photon-qt-0.25.2.exe
    ├── photon-tx-0.25.2.exe
    ├── photon-util-0.25.2.exe
    ├── photon-wallet-0.25.2.exe
    ├── photond-0.25.2.exe
    └── build-info.txt
```

For Ubuntu native builds, the current host's final wallet files land in
`outputs/Ubuntu-20/`, `outputs/Ubuntu-22/`, `outputs/Ubuntu-24/`, or
`outputs/Ubuntu-26/` depending on the detected Ubuntu release. Each Ubuntu
folder gets its own `install-deps.sh`, `README.md`, `build-info.txt`, and
`photon.conf`.

For Windows cross-builds from Linux, the output bundle lands in
`outputs/Windows/` and contains versioned `.exe` binaries plus
`build-info.txt`. For native macOS builds, `Photon-Qt.app` and the versioned
daemon tools land in `outputs/Macosx/`.

## Docker Images

When using `--pull-docker`, the build script uses these prebuilt images:

| Image | Purpose |
|---|---|
| `sidgrip/native-base:20.04` | Native Linux Ubuntu 20.04 build |
| `sidgrip/native-base:22.04` | Native Linux Ubuntu 22.04 build |
| `sidgrip/native-base:24.04` | Native Linux Ubuntu 24.04 build; default native Docker image |
| `sidgrip/native-base:26.04` | Native Linux Ubuntu 26.04 build and hardened release lane |
| `sidgrip/appimage-base:22.04` | Ubuntu 22.04+ AppImage build |
| `sidgrip/mxe-base:latest` | Windows cross-compile |
| `sidgrip/osxcross-base:sdk-26.2` | macOS cross-compile |

## AuxPoW Mining RPCs

Photon exposes the AuxPoW mining RPCs used by merged-mining pools:

- `createauxblock <address>`
- `submitauxblock <hash> <auxpow>`
- `getauxblock` with no arguments, using `-auxpowmineraddress=<address>`
- `getauxblock <hash> <auxpow>` submit mode

AuxPoW templates include the daemon-computed `version` and `versionHex` fields.
Pools should verify those fields, but should not rewrite them. During Taproot
`started` and `locked_in`, the daemon sets bit `2`; after Taproot is active,
miners do not need to keep signaling the bit.

## Multi-Coin Builder

For coordinated BlakeStream-family wallet builds, see the
[Blakestream Installer](https://github.com/SidGrip/Blakestream-Installer).

## License

Photon Core is released under the terms of the MIT license. See
[COPYING](COPYING) for more information or see
https://opensource.org/licenses/MIT.
