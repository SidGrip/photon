# Photon Mainnet Carry-Back Audit — 2026-04-18

## Summary

- Source compared:
  `/home/sid/Blakestream-Installer/repos/Photon-0.15.2-update/src`
- Devnet copy compared:
  `/home/sid/Blakestream-Devnet/coins/Photon/src`
- Result:
  no new Photon mainnet wallet, consensus, or RPC promotion was discovered in the devnet copy. The meaningful carry-back work for this cycle lives in Eloipool and Electrium, not in the devnet coin copy.

## Diff Classification

### Devnet-Only Differences

- `src/chainparams.cpp`
- `src/chainparamsbase.cpp`
- `src/chainparamsbase.h`

Reason:
- these files reflect devnet chain selection, local datadir/default-network behavior, and devnet-facing network identity

### Build Cleanup Only

- generated `Makefile.in` removal in the devnet copy
- `src/secp256k1/Makefile.am` build-tree cleanup

Reason:
- build-tree cleanup does not represent a new Photon protocol, wallet, or RPC change to promote into the mainnet core line

## Carry-Back Decision

- Keep this repo as the canonical Photon mainnet core line.
- Do not port devnet network identity, datadir, or activation shortcuts into this repo.
- Track pool carry-back separately in the mainnet Eloipool repo.
- Track wallet sync/signing carry-back separately in the Electrium repo.
- Keep the documented AuxPoW mainnet start height and scheduled SegWit rollout in this repo as the chain source of truth.
