# Photon Core 0.25.2

Photon Core 0.25.2 is a release of the Photon (PHO) full node and wallet,
rebased onto the Bitcoin Core 25.2 codebase. Source and release binaries:

  https://github.com/BlueDragon747/photon

Photon is a Blake-256 (8-round) AuxPoW merge-mined coin in the BlakeStream
family. For network parameters and the full 0.25.2 consensus details, see
`README.md` ("About Photon" and "Mainnet Consensus Changes In 0.25.2").

## How to upgrade

Shut down the running wallet/node (`photon-qt` or `photond`) and wait for it to
stop completely, then replace the binaries (`photond`, `photon-qt`,
`photon-cli`, `photon-tx`, `photon-wallet`) with the 0.25.2 build. Existing
`wallet.dat` and block/chain data are kept.

## Notable changes

- Rebased onto Bitcoin Core 25.2, preserving Photon's network magic, address
  formats, AuxPoW (chain ID `0x0002`) merge-mining, subsidy, and coinbase
  maturity rules.
- Dual wallet support: legacy Berkeley DB `wallet.dat` and descriptor SQLite
  wallets.
- `MAX_MONEY` is enforced as a 10,000,000,000 PHO per-output / per-transaction
  sanity cap. To pay a single address more than the cap, use
  `contrib/send-big-pho.py`, which splits the payment across multiple
  transactions (see `README.md`, "Sending Large Amounts").

## Credits

Photon Core is built on Bitcoin Core. Thanks to the Bitcoin Core developers and
contributors, and to the Photon / BlakeStream contributors.
