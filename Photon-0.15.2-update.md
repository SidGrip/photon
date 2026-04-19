# Photon 0.15.2 Update — Source of Truth

## Overview

Port Photon (PHO) from its current 0.8.9.8 codebase to Bitcoin Core 0.15.2, following the same approach used for the Blakecoin 0.15.2 update (`Blakecoin-0.15.2-update`).

**Reference codebase:** `../Blakecoin-0.15.2-update/` — the completed Blakecoin port to 0.15.2
**Original codebase:** `../Photon/` — current 0.8.9.8 source with all coin-specific parameters

---

## QC Status

- This file is only safe to implement from after applying the QC corrections in this section.
- The current 0.15.2 tree builds and passes `test/functional/wallet-basic-smoke.py` on regtest with no sends, no funding, and no mining.
- Verified from `../Photon/src`: main/test P2P and RPC ports, `pchMessageStart`, address prefixes, message header, and chain ID `0x0002`.
- Corrected from source and live replay: Photon keeps the legacy nominal AuxPow start height `160000` from `../Photon/src/main.cpp`, and the 0.15.2 port must preserve the legacy behavior that tolerates historical pre-start AuxPoW-bearing blocks during sync.
- Corrected from source: secret-key prefixes are `128` mainnet and `239` testnet, not `TBD`.
- Corrected from source: the reward formula is not `32768 + sqrt(height * difficulty) * COIN`. Legacy Photon adds `int64(sqrt(dDiff * nHeight))` raw satoshis to `32768 * COIN`, and that exact `nBits`-based formula is now ported into `src/validation.cpp`.
- Corrected from source: Photon already has a populated checkpoint table in `../Photon/src/checkpoints.cpp`; do not treat checkpoints as missing work.
- Corrected in this tree: mainnet checkpoints, chain transaction metadata, and historical seed nodes are now sourced from the original Photon repo instead of inherited Blakecoin donor defaults.
- The shared AuxPow framework is now integrated: `src/auxpow.{h,cpp}`, `src/primitives/pureheader.{h,cpp}`, AuxPow-aware block/header serialization, disk index persistence, AuxPow-aware PoW validation, and chain-ID-aware block template versions.
- Mainnet keeps strict chain-ID enforcement with chain ID `0x0002` and the legacy nominal AuxPow start height `160000`; testnet and regtest use chain ID `0x0002` with start height `0` and strict chain ID disabled for local QA.
- Sync note: preserved bootstrap data shows AuxPoW-flagged historical blocks around height `32050`, but those are now treated as tolerated pre-start history rather than proof that the nominal activation height changed.
- Current live daemon QA on the build server now accepts deep historical Photon bootstrap blocks again after restoring the legacy-compatible rule. Keep monitoring until the full bootstrap replay and peer catch-up complete before calling Photon fully green.
- Verified after the fan-out: `make -C src -j4 blakecoind blakecoin-cli` succeeds and `test/functional/test_runner.py --jobs=1 wallet-basic-smoke.py` passes on regtest.
- Historical validation compatibility is now preserved: the legacy Photon 0.8 tree only enforced the BIP30 overwrite rule in `!pindex->phashBlock` contexts, so the 0.15.2 port must not silently restore Bitcoin Core's broader duplicate-txid rejection path.
- Merged-mining RPC direction is now fixed for this port: primary RPCs are `createauxblock <address>` plus `submitauxblock <hash> <auxpow>`, with `getauxblock` kept only as a compatibility wrapper for older pool software. `getworkaux` is intentionally out of scope unless a real dependency is later proven.
- Keep this repo on a strict no-send / no-mine mainnet rule until the final production pool and Electrium carry-back staging are complete.

### BlakeStream Seed And AuxPoW RPC Policy

- BlakeStream DNS seeds (`seed.blakestream.io`, `seed.blakecoin.org`) are shared across all six coins and serve nodes for ALL coins. A single seed lookup returns peer IPs regardless of which coin is asking; coin separation happens at the wire-protocol layer via each coin's unique `pchMessageStart` and default port.
- The production direction for this repo is a modern merged-mining RPC surface: `createauxblock` to build the child-chain template and `submitauxblock` to submit the solved AuxPoW payload.
- `createauxblock` is address-driven on purpose so a pool can choose the child-chain payout script explicitly instead of depending on wallet mining state inside the daemon.
- `getauxblock` remains only as a compatibility mode for older merged-mining software. It should map onto the same block-template / block-submit flow rather than preserving a separate legacy implementation path.
- `getworkaux` is not part of the planned 0.15.2 target. We are not reviving `getwork`-era RPC unless a live pool or deployment proves it is still required.
- The same 2 DNS seeds (`seed.blakestream.io`, `seed.blakecoin.org`) are used by all six coins. This matches the Blakecoin 0.15.2 reference repo exactly.

## AuxPoW Start And Completed Work

| Network | Chain ID | Nominal AuxPoW Start | Observed Pre-Start AuxPoW Evidence In Current QA | Exact Time/Date Status | 0.15.2 Port Rule |
|---------|----------|----------------------|-----------------------------------------------|------------------------|------------------|
| Mainnet | `0x0002` | `160000` | Preserved bootstrap history switches from non-AuxPoW at `32049` to AuxPoW-bearing history at `32050` | Exact activation time remains archival-only; compatibility rule is already proven from preserved replay | Keep `160000` as the nominal legacy value, but tolerate earlier historical AuxPoW-bearing blocks during bootstrap / IBD |
| Testnet | `0x0002` | `0` | N/A | Local QA only | AuxPoW enabled for local QA; strict chain ID disabled |
| Regtest | `0x0002` | `0` | N/A | Local QA only | AuxPoW enabled for local QA; strict chain ID disabled |

Interpretation note:
Photon is the clearest example of why the docs now separate nominal start height from observed historical evidence. `160000` is the configured legacy boundary from the original source. The observed AuxPoW-bearing history around `32050` is real preserved chain data, but it is not being treated as proof that the nominal legacy activation height changed.

- Completed in this repo:
  - Integrated the shared AuxPoW framework with `src/auxpow.{h,cpp}` and `src/primitives/pureheader.{h,cpp}`.
  - Ported AuxPoW-aware block/header serialization, disk index persistence, block version handling, and PoW validation.
  - Restored Photon's nominal legacy mainnet start height to `160000` after confirming the old `32050` observation was historical pre-start AuxPoW history, not a real activation override.
  - Removed the modern `early-auxpow-block` reject so historical bootstrap replay can accept the early AuxPoW-bearing chain history while chain-ID enforcement still starts from the nominal boundary.
  - Preserved the legacy relaxed BIP30 behavior needed for historical sync compatibility.
  - Implemented the modern merged-mining RPC direction: `createauxblock <address>` and `submitauxblock <hash> <auxpow>`, with `getauxblock` retained only as a compatibility wrapper.
- Operational rule:
  - Keep the strict no-send / no-mine mainnet rule in place while final production pool and Electrium carry-back staging continue.

### Wire Checksum Policy

- Photon should preserve the legacy `Hashblake` P2P message checksum behavior for current network interoperability.
- Do not normalize Photon to Blakecoin's temporary non-`Hashblake` handshake exception.
- Keep Blakecoin documented as the one current exception; Photon stays on `Hashblake` before go-live unless a fresh compatibility review says otherwise.

---

## Coin Identity

| Parameter | Value |
|-----------|-------|
| Coin Name | Photon |
| Ticker | PHO |
| Algorithm | Blake-256 (8 rounds) |
| Merge Mining | Yes (AuxPow — first Blake-256 merge-mined coin) |
| Base Version (current) | 0.8.9.8 (forked from Blakecoin/Bitcoin 0.8.5) |
| Target Version | 0.15.2 |

---

## Chain Parameters to Preserve

### Network

| Parameter | Mainnet | Testnet |
|-----------|---------|---------|
| P2P Port | 35556 | 18992 |
| RPC Port | 8984 | 18998 |
| pchMessageStart | 0xf9, 0xbc, 0xb4, 0xd2 | 0x0b, 0x11, 0x09, 0x08 |

### Address Prefixes

| Type | Mainnet | Testnet |
|------|---------|---------|
| Pubkey Address | 26 (0x1A) | 142 (0x8E) |
| Script Address | 7 (0x07) | 170 (0xAA) |
| Secret Key | 128 (0x80) | 239 (0xEF) |
| Bech32 HRP | `pho` | `tpho` |

### Block Parameters

| Parameter | Value |
|-----------|-------|
| Block Time | 180 seconds (3 minutes) |
| Target Timespan | 3,600 seconds (60 minutes) |
| Retarget Interval | 20 blocks |
| Coinbase Maturity | 120 blocks |
| Max Supply | 90,000,000,000 PHO (90 billion) |
| COIN | 100,000,000 satoshis |
| PoW Limit | ~uint256(0) >> 24 |

### Block Reward (CRITICAL — Dynamic Formula)

Photon uses a **dynamic reward formula** based on block height AND difficulty:

```
reward = 32768 * COIN + int64(sqrt(dDiff * height))
```

- Base reward: 32,768 PHO per block
- Additional reward scales with `sqrt(dDiff * height)` raw satoshis
- `dDiff` comes from compact `nBits` relative to the network pow limit
- Genesis block reward: 32,768 PHO

`src/validation.cpp` now uses the legacy Photon formula directly. AuxPow validation still needs the remaining port work before mainnet mining is considered complete.

### Difficulty Adjustment (CUSTOM)

- Max ±15% change per retarget until block 3,500
- Max ±3% change per retarget after block 3,500
- Same clamping logic as Lithium (both forked from same base)

### Genesis Block

| Parameter | Value |
|-----------|-------|
| Hash | `0x000000e79a20d718a2f2d8b98161dc6700104a22d8e9be70e8ac361ee6664b9c` |
| Merkle Root | `0x251e462b7d8b2e92e74651186fbbc66ac715cf9c160212efb02642232207112d` |
| nTime | 1392688072 (Feb 17, 2014) |
| nNonce | 5,992,999 |
| nVersion | 112 |
| nBits | bnProofOfWorkLimit |
| Coinbase Message | "US forces target leading al-Shabaab militant in Somalian coastal raid" |

### Testnet Genesis Block

| Parameter | Value |
|-----------|-------|
| Hash | `0x00000052d978f26d698e0c4dbce9f8139a69f2fbda37715149146776aeb70d5b` |
| nTime | 1392351202 |
| nNonce | 4,335,147 |

### DNS Seeds

- photoncc.com
- 72.23.74.166
- 62.219.234.143
- 78.26.209.208
- 77.121.61.203

### AuxPow Configuration

| Parameter | Value |
|-----------|-------|
| Chain ID | 0x0002 |
| Mainnet AuxPow Start Height | 160000 |
| AuxPow Block Version Flag | 1 << 8 (0x0100) |
| Chain Start Multiplier | 1 << 16 (0x10000) |

- Historical replay note: preserved bootstrap data switches from non-AuxPow block version `0x20010` at height `32049` to AuxPow version `0x20110` at height `32050`, so the 0.15.2 port preserves `160000` as the nominal activation height while still tolerating that earlier historical AuxPoW-bearing history during sync.

---

## What Needs to Be Done

### Phase 1: Copy & Rebrand the Blakecoin 0.15.2 Base

1. **Copy** the entire `Blakecoin-0.15.2-update` codebase into this directory
2. **Rename** all Blakecoin references → Photon:
   - Binary names: `photond`, `photon-qt`, `photon-cli`, `photon-tx`
   - Config file: `photon.conf`, config dir `~/.photon/`
   - URI scheme: `photon://`
   - Desktop entry, icons, window titles
   - `configure.ac`: package name, version
   - Window title: "Photon - Wallet"
   - Tray tooltip: "Photon client"
   - Signed message header: "Photon Signed Message:\n"

### Phase 2: Apply Coin-Specific Parameters

3. **`src/chainparams.cpp`** — Replace ALL chain parameters:
   - Genesis block (hash, merkle root, nTime=1392688072, nNonce=5992999, coinbase)
   - Network ports (P2P: 35556, RPC: 8984)
   - Message start bytes (0xf9, 0xbc, 0xb4, 0xd2)
   - Address prefixes (pubkey: 26, script: 7) — NOTE: same pubkey prefix as Blakecoin!
   - Block timing (180s block time, 60-min retarget, 20-block interval) — same as Blakecoin
   - **Disable halving interval** (set to max) — reward is formula-based and no longer uses the donor halving schedule
   - Checkpoints
   - Bech32 HRP
   - DNS seeds + fixed seed IPs

4. **`src/amount.h`** — MAX_MONEY = 90,000,000,000 * COIN (90 billion PHO)
   - **OVERFLOW CHECK**: 90B * 100M = 9e18. int64_t max = ~9.2e18. This is TIGHT but fits. Verify carefully!

5. **`src/validation.cpp`** — Block reward logic (**CUSTOM — DYNAMIC FORMULA**):
   ```
   CAmount reward = 32768 * COIN;
   reward += int64(sqrt(dDiff * height));  // dDiff is derived from compact nBits vs powLimit
   ```
   - Matches the legacy `../Photon/src/main.cpp` `GetBlockValue()` formula
   - The difficulty input comes from compact `nBits`, not a floating-point helper

6. **`src/pow.cpp`** — Custom difficulty adjustment:
   - Port the ±15% / ±3% clamping logic
   - Threshold at block 3,500

7. **`src/consensus/consensus.h`** — COINBASE_MATURITY = 120

8. **`src/qt/`** — Update all GUI branding for Photon

### Phase 3: AuxPow / Merge Mining (CRITICAL)

9. **AuxPow integration** — Photon was the FIRST Blake-256 merge-mined coin
   - Chain ID `0x0002` and legacy nominal mainnet activation height `160000` are wired into the shared 0.15.2 AuxPow framework
   - `src/auxpow.{h,cpp}` and `src/primitives/pureheader.{h,cpp}` are integrated, with AuxPow-aware block/header serialization, disk index persistence, PoW validation, and block template versions
   - The modern merged-mining RPC path is now implemented and no-send regtest-smoke verified for `createauxblock <address>` plus compatibility `getauxblock`
   - This remains consensus-critical: direct `createauxblock` plus `submitauxblock` acceptance is already proven in isolated QA, and the former production pool / Electrium carry-back staging gate is now green

### Phase 4: Build System

10. **`build.sh`** — Update all variables:
    - COIN_NAME: "photon"
    - DAEMON_NAME: "photond"
    - QT_NAME: "photon-qt"
    - CLI_NAME: "photon-cli"
    - TX_NAME: "photon-tx"
    - VERSION: "0.15.2"
    - RPC_PORT: 8984
    - P2P_PORT: 35556

11. **Docker configs** — Same Docker images as Blakecoin 0.15.2

### Phase 5: SegWit Activation

12. **Mainnet SegWit rollout** — Mainnet versionbits signaling starts on May 11, 2026 00:00:00 UTC (`1778457600`) and times out on May 11, 2027 00:00:00 UTC (`1809993600`).
13. **Activation semantics** — May 11, 2026 is the signaling start date, not guaranteed same-day activation. Actual mainnet SegWit enforcement still depends on miner signaling and BIP9 lock-in.
14. **CSV / test networks** — CSV stays `ALWAYS_ACTIVE`, and testnet/regtest keep `ALWAYS_ACTIVE` SegWit for controlled QA and wallet validation.
15. **BIP34/65/66** — Disable version checks

---

## Key Differences from Blakecoin

| Aspect | Blakecoin | Photon |
|--------|-----------|--------|
| Block Time | 180s (3 min) | 180s (3 min) — SAME |
| Retarget Interval | 20 blocks | 20 blocks — SAME |
| Retarget Timespan | 1 hour | 1 hour — SAME |
| Reward Model | Dynamic (25 + sqrt) | Dynamic (32768 * COIN + nBits sqrt) |
| Max Supply | 21M | 90 billion |
| Coinbase Maturity | ??? | 120 blocks |
| P2P Port | 8773 | 35556 |
| RPC Port | 8772 | 8984 |
| Pubkey Address | 26 | 26 — SAME |
| Merge Mining | No (in donor Blakecoin 0.15.2) | Yes (AuxPow, chain ID `0x0002`) |
| Difficulty Clamping | Standard | ±15% then ±3% |
| Genesis Date | Oct 2013 | Feb 2014 |

Note: Photon shares the SAME pubkey address prefix (26) as Blakecoin. This means Photon addresses look identical to Blakecoin addresses. This is by design (both are Blake-family coins).

---

## Potential Issues & Gotchas

1. **MAX_MONEY overflow risk** — 90 billion * 100 million = 9×10^18. int64_t max ≈ 9.2×10^18. This BARELY fits. Any arithmetic on MAX_MONEY must be checked for overflow. Consider whether intermediate calculations could overflow.
2. **Dynamic reward formula** — The `sqrt(dDiff * height)` formula is unusual, but the port now follows the legacy compact `nBits` math from `../Photon/src/main.cpp`.
3. **AuxPow with Chain ID** — Photon has a specific chain ID (`0x0002`) for merge mining. Historical bootstrap replay shows AuxPoW-bearing blocks around `32050`, so the port must preserve the legacy rule that tolerates pre-start AuxPoW history instead of replacing the original nominal `160000` boundary.
4. **Same address prefix as Blakecoin** — Pubkey address 26 is shared. Bech32 HRP (`pho`) will differentiate SegWit addresses.
5. **Custom difficulty clamping** — Same ±15%/±3% logic as Lithium. Must be ported to `pow.cpp`.
6. **Shared genesis coinbase message** — Same message as Blakecoin ("US forces target...") but different nTime/nNonce produce a different genesis hash.
7. **P2P port 35556** — Unusual high port number. Ensure firewall documentation mentions this.
8. **Historical BIP30 rule** — The original Photon chain relaxed BIP30 during normal block connection (`!pindex->phashBlock` only). Keep that legacy behavior unless a full historical replay proves the broader 0.15.2 rule is safe.

---

## Build & Test Plan

1. Build native Linux first
2. Verify genesis block hash matches
3. Test address generation (prefix 26 — same as Blakecoin, but different network magic prevents cross-network issues)
4. Verify block reward formula produces correct values at various heights
5. Test RPC on port 8984
6. Build AppImage, Windows, macOS
7. Preserve the archived historical replay notes and do not reintroduce the rejected `early-auxpow-block` rule before any mainnet activity
8. Verify MAX_MONEY doesn't cause overflow issues

---

## Verified Snapshot

- Native Linux rebuild succeeded for `blakecoind` and `blakecoin-cli`.
- Fresh regtest no-send smoke passed for `getnewaddress`, `createauxblock <address>`, and compatibility `getauxblock`.
- Verified AuxPow template `chainid` returned as `2` on fresh regtest, matching `consensus.nAuxpowChainId`.
- Verified from `~/.photon/bootstrap.dat.old`: height `32049` remains non-AuxPow and height `32050` is the first observed AuxPoW-flagged historical mainnet block (`0x20110`).
- Verified from `../Photon/src/main.cpp`: legacy Photon keeps BIP30 relaxed with `bool fEnforceBIP30 = !pindex->phashBlock;`, uses chain ID `0x0002`, and keeps the nominal AuxPoW start height at `160000`. The 0.15.2 port now preserves that historical validation behavior while tolerating early AuxPoW-bearing history during sync.
- Direct `createauxblock` plus `submitauxblock` acceptance is now proven in isolated QA. The production carry-back staging that used to block release is now green.

---

## File Reference

| What | Where |
|------|-------|
| Reference (completed) | `../Blakecoin-0.15.2-update/` |
| Original coin source | `../Photon/` |
| Original params | `../Photon/src/main.cpp` |
| Original build script | `../Photon/build.sh` |
| Qt project file | `../Photon/Photon-qt.pro` |
| AuxPow code | `../Photon/src/main.cpp` (BLOCK_VERSION_AUXPOW) |
| DNS seeds | `../Photon/src/net.cpp` |

## SegWit Activation Test

- Functional test: `test/functional/segwit-activation-smoke.py`
- Build-server wrapper: `/home/sid/Blakestream-Installer/qa/runtime/run-segwit-activation-suite.sh`
- Direct command used by the wrapper:

```bash
BITCOIND=/path/to/photond BITCOINCLI=/path/to/photon-cli \
python3 ./test/functional/segwit-activation-smoke.py \
  --srcdir="$(pwd)/src" \
  --tmpdir="<artifact_root>/photon/<timestamp>/tmpdir" \
  --nocleanup \
  --loglevel=DEBUG \
  --tracerpc
```

- Expected regtest Bech32 prefix: `rpho1`
- Review artifacts:
  `summary.json`, `state-defined.json`, `state-started.json`, `state-locked_in.json`, `state-active.json`, `address-sanity.json`, `combined-regtest.log`, `tmpdir/test_framework.log`, `tmpdir/node*/regtest/debug.log`
- Successful all-six build-server run:
  `/home/sid/Blakestream-Installer/outputs/segwit-activation/20260412T083423Z/run-summary.md`
- Coin artifact directory:
  `/home/sid/Blakestream-Installer/outputs/segwit-activation/20260412T083423Z/photon`
- Harness note:
  the final witness proposal builder now takes the coinbase amount directly from `getblocktemplate()["coinbasevalue"]`, and the Photon miner/validator reward path was corrected to stay tied to the block's compact `nBits` difficulty.
- Safety rule:
  regtest only for activation validation; do not mine or send transactions on mainnet while rollout QA is still in progress.

## AuxPoW Testnet Merged-Mining Verification

- Final successful container-built run:
  `/home/sid/Blakestream-Installer/outputs/auxpow-testnet/20260413T003341Z/run-summary.md`
- Wrapper command:
  `bash /home/sid/Blakestream-Installer/qa/auxpow-testnet/run-auxpow-testnet-suite.sh`
- Parent chain:
  Blakecoin testnet only, fully isolated from public peers.
- Live proof result:
  Photon accepted `2` merged-mined child blocks in the 4-child batch and `1` in the 5-child full run.
- Direct RPC cross-check:
  `createauxblock` plus `submitauxblock` accepted on a fresh Photon testnet pair. Artifact:
  `/home/sid/Blakestream-Installer/outputs/auxpow-testnet/20260413T003341Z/photon/rpc-crosscheck.json`
- QC note:
  the full harness used the collision-free proxy sizing rule discovered during QA, which keeps Photon from sharing an aux merkle slot with another child chain in the multi-child phases.
- Safety rule:
  testnet only for merged-mining QA; do not mine or send transactions on mainnet while AuxPoW rollout validation is still in progress.

## Devnet/Testnet Validation Outcomes

- SegWit activation validation passed on isolated regtest. See:
  `/home/sid/Blakestream-Installer/outputs/segwit-activation/20260412T083423Z/photon`
- AuxPoW merged-mining validation passed on isolated testnet, including direct `createauxblock` plus `submitauxblock` acceptance. See:
  `/home/sid/Blakestream-Installer/outputs/auxpow-testnet/20260413T003341Z/photon`
- Mainnet carry-back audit for the devnet copy lives in:
  `mainnet-carryback-audit-2026-04-18.md`
- Audit result:
  the diff between this repo and the devnet `coins/Photon` copy stayed limited to devnet `chainparams*` and build cleanup. No new Photon mainnet wallet, consensus, or RPC carry-back was identified from the devnet copy itself.

## Mainnet Carry-Back Decisions

- SegWit rollout remains scheduled, not forced active.
- Mainnet AuxPoW start height remains `160000` as the chain source of truth.
- Do not port devnet network identity, datadir, test shortcuts, or activation shortcuts back into this repo.
- Pool/runtime carry-back work is tracked in the mainnet Eloipool repo.
- Electrium sync and signing carry-back work is tracked in the Electrium repo.
- Mainnet pool integration now depends on the proven multi-miner aux-child payout path in Eloipool, not the old single active mining-key QA shortcut.

## Staging Hygiene

- Keep the intentional autotools and build-system layer in staging for this repo:
  `Makefile.am`, `Makefile.in`, `aclocal.m4`, `autogen.sh`, `configure*`, `build-aux/*`, and `depends/*`.
- Trim generated build junk before review or promotion:
  `.libs/`, `.deps/`, `autom4te.cache/`, `*.o`, `*.lo`, `*.la`, `config.log`, `config.status`, and similar transient outputs.
- April 19, 2026 staging pass explicitly removed staged libtool and univalue build artifacts while preserving the intentional autotools carry-back set.

## Not Carried Back From Devnet

- `src/chainparams.cpp`, `src/chainparamsbase.cpp`, `src/chainparamsbase.h`
- Any private-testnet `BIP65Height = 1`, `ALWAYS_ACTIVE`, devnet ports, message starts, datadirs, or local-only harness shortcuts
- Pool UI, merged-mine proxy, Electrium, ElectrumX, and builder/runtime scripts

## Pool / Electrium Dependencies

- Mainnet merged-mining now depends on the modern `createauxblock` plus `submitauxblock` direction and the proven multi-miner aux payout model in Eloipool.
- Electrium compatibility now depends on full AuxPoW header support and Blake-family single-SHA signing compatibility.
- Per-coin overlays and branding stay in the Electrium repo and are not folded back into this C++ core tree.

## Safety Rule

- Do not mine on mainnet while carry-back staging is in progress.
- Do not send transactions on mainnet while carry-back staging is in progress.
- Use isolated regtest, testnet, or staging environments until rollout QA is complete.

## April 18, 2026 Devnet Validation Snapshot

- Shared BlakeStream devnet run `20260418T195508Z` proved concurrent multi-miner AuxPoW against the live pool with two mining keys active in the same session.
- Live pooled merged-mined Photon child block proof is green:
  height `415` accepted with `tx_count = 2`.
- This means the live pool/proxy path is no longer limited to coinbase-only Photon child blocks once mempool transactions are present.

## Mainnet Carry-Back Snapshot

- Keep Photon chain identity, Hashblake requirement, AuxPoW start rules, and scheduled SegWit rollout exactly as already documented in this repo.
- Promote only the proven external dependencies:
  mainnet pool multi-miner mining-key payout plumbing and Electrium full AuxPoW-header plus single-SHA signing compatibility.
- Do not carry back any devnet ports, datadirs, private-testnet activation shortcuts, or runtime wrapper behavior into mainnet chain params.

## April 19, 2026 Broader Electrium Staging Closure

- Broader staged packaged-client proof is now green at:
  `/home/sid/Blakestream-Devnet/outputs/electrium-staging/20260419T053030Z/run-summary.md`
- Photon's packaged Electrium client connected successfully against the staged
  local ElectrumX backend on `127.0.0.1:55001`.
- This run also exposed and closed a shared aux-core startup bug in
  `src/validation.cpp`: height-less disk rereads could treat genesis-like
  headers as regular AuxPoW blocks and fail with a false
  `non-AUX proof of work failed` reject.
- The fix now treats
  `block.GetHash() == consensusParams.hashGenesisBlock || block.hashPrevBlock.IsNull()`
  as genesis-like in the disk-reread path, which keeps standalone staged
  backends honest without relaxing real chained AuxPoW validation.
