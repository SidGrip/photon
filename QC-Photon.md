# Photon 0.15.2 QC Report

**QC run date:** 2026-04-11
**Repo:** `/home/sid/Blakestream-Installer/repos/Photon-0.15.2-update/`
**Original source:** `/home/sid/Blakestream-Installer/repos/Photon/`
**Source of truth:** `Photon-0.15.2-update.md`

---

## Coin Metrics Summary

| Parameter | Expected | 0.15.2 Port | Original 0.8.x | Status |
|---|---|---|---|---|
| Coin name / ticker | Photon / PHO | Photon (`configure.ac:11 AC_INIT([Photon Core]...[photon]...)`, `build.sh:32 COIN_NAME=photon`) | Photon | PASS |
| Block time | 180 s | `nPowTargetSpacing = 3*60` | 180 s | PASS |
| Retarget timespan | 3,600 s (60 min) | `nPowTargetTimespan = 20*3*60` | 3,600 s | PASS |
| Retarget interval | 20 blocks | 3,600 / 180 = 20 | 20 | PASS |
| Coinbase maturity | 120 | `consensus/consensus.h:19 COINBASE_MATURITY = 120` | 120 (`main.h:61`) | PASS |
| Max supply | 90,000,000,000 PHO | `amount.h:26 MAX_MONEY = 90000000000LL * COIN` | `main.h:58 90000000000 * COIN` | PASS |
| int64 overflow margin | fits | 9.0e18 vs 9.223e18 max (~2.5% headroom) | same | PASS (tight) |
| P2P port (mainnet) | 35556 | `chainparams.cpp:142 nDefaultPort = 35556` | `protocol.h:21 35556` | PASS |
| RPC port (mainnet) | 8984 | `chainparamsbase.cpp:35 nRPCPort = 8984` | `bitcoinrpc.cpp:43 8984` | PASS |
| P2P port (testnet) | 18992 | `nDefaultPort = 18992` | `protocol.h:21 18992` | PASS |
| RPC port (testnet) | 18998 | `nRPCPort = 18998` | `bitcoinrpc.cpp:43 18998` | PASS |
| pchMessageStart mainnet | f9 bc b4 d2 | `chainparams.cpp:138-141` | `main.cpp:3185` | PASS |
| pchMessageStart testnet | 0b 11 09 08 | `chainparams.cpp:241-244` | `main.cpp:2846-2849` | PASS |
| Pubkey prefix (mainnet) | 26 | `base58Prefixes[PUBKEY_ADDRESS] = 26` | `base58.h:276 PUBKEY_ADDRESS = 26` | PASS |
| Script prefix (mainnet) | 7 | `base58Prefixes[SCRIPT_ADDRESS] = 7` | `base58.h:277 SCRIPT_ADDRESS = 7` | PASS |
| Secret key (mainnet) | 128 | `base58Prefixes[SECRET_KEY] = 128` | `base58.h:405 128` | PASS |
| Pubkey prefix (testnet) | 142 | `chainparams.cpp:256 = 142` | `base58.h:278 PUBKEY_ADDRESS_TEST = 142` | PASS |
| Script prefix (testnet) | 170 | `chainparams.cpp:257 = 170` | `base58.h:279 SCRIPT_ADDRESS_TEST = 170` | PASS |
| Secret key (testnet) | 239 | `chainparams.cpp:258 = 239` | `base58.h:405 239` | PASS |
| Bech32 HRP mainnet | `pho` | `bech32_hrp = "pho"` | N/A (legacy) | PASS |
| Bech32 HRP testnet | `tpho` | `bech32_hrp = "tpho"` | N/A | PASS |
| AuxPoW chain ID | 0x0002 | `consensus.nAuxpowChainId = 0x0002` | `main.cpp:2056 GetOurChainID() return 0x0002` | PASS |
| AuxPoW start height (mainnet) | 160000 (nominal) | `consensus.nAuxpowStartHeight = 160000` | `main.cpp:2048 GetAuxPowStartBlock()` returns 160000 | PASS |
| Genesis hash (mainnet) | 000000e79a20...b9c | `chainparams.cpp:146` matches | matches | PASS |
| Merkle root | 251e462b...112d | `chainparams.cpp:147` matches | matches | PASS |
| Genesis nTime | 1392688072 | `CreateGenesisBlock(1392688072, 5992999, ...)` | matches | PASS |
| Genesis nNonce | 5992999 | matches | matches | PASS |
| Genesis nVersion | 112 | matches | matches | PASS |
| Genesis reward | 32768 * COIN | matches | `main.cpp:1091 nGenesisBlockRewardCoin = 32768 * COIN` | PASS |
| Testnet genesis hash | 00000052d978...0d5b | `chainparams.cpp:249` matches | matches | PASS |
| Checkpoints count | 18 | 18 entries, all heights/hashes match | 18 entries in `../Photon/src/checkpoints.cpp` | PASS |
| Difficulty clamping | ±15% < 3500, ±3% ≥ 3500 | `pow.cpp:85-92` | legacy `main.cpp` | PASS |
| Halving | disabled | `nSubsidyHalvingInterval = max<int>()` | N/A (formula based) | PASS |
| BIP34/65/66 heights | disabled | set to 100000000 | N/A | PASS |
| SegWit signaling | 1778457600 → 1809993600 | `chainparams.cpp:124-125` | N/A | PASS |
| CSV / testnet SegWit | ALWAYS_ACTIVE | matches | N/A | PASS |
| Wire checksum | Hashblake | `net.cpp:832,2931`, `base58.cpp:129,144`, `hash.h:90`, `pureheader.cpp:13` | matches | PASS |

---

## Reward Formula Side-by-Side

### Original Photon 0.8.x (`../Photon/src/main.cpp:1091-1123`)

```cpp
static const int64 nGenesisBlockRewardCoin = 32768 * COIN;
static const int64 nBlockRewardStartCoin   = 32768 * COIN;

int64 static GetBlockValue(int nHeight, int64 nFees, unsigned int nBits)
{
    if (nHeight == 0) {
        return nGenesisBlockRewardCoin;
    }
    unsigned int basenBits = bnProofOfWorkLimit.GetCompact();
    int nShift = int((basenBits >> 24) & 0xff) - int((nBits >> 24) & 0xff);
    double dDiff =
        (double)(basenBits & 0x007fffff) / (double)(nBits & 0x007fffff);
    while (nShift > 0) { dDiff *= 256.0; --nShift; }
    while (nShift < 0) { dDiff /= 256.0; ++nShift; }
    int64 nSubsidy1 = int64(sqrt(dDiff * nHeight));
    int64 nSubsidy  = nSubsidy1 + nBlockRewardStartCoin;
    return nSubsidy + nFees;
}
```

### 0.15.2 Port (`src/validation.cpp:1076-1098`)

```cpp
CAmount GetBlockSubsidy(int nHeight, unsigned int nBits, const Consensus::Params& consensusParams)
{
    if (nHeight == 0) {
        return 32768 * COIN;
    }

    const unsigned int baseBits = UintToArith256(consensusParams.powLimit).GetCompact();
    int nShift = int((baseBits >> 24) & 0xff) - int((nBits >> 24) & 0xff);
    double dDiff = double(baseBits & 0x007fffff) / double(nBits & 0x007fffff);

    while (nShift > 0) { dDiff *= 256.0; --nShift; }
    while (nShift < 0) { dDiff /= 256.0; ++nShift; }

    CAmount nSubsidy = 32768 * COIN;
    nSubsidy += static_cast<CAmount>(std::sqrt(dDiff * nHeight));
    return nSubsidy;
}
```

### Diff analysis

| Aspect | Legacy 0.8.x | 0.15.2 port | Equivalent? |
|---|---|---|---|
| Genesis path | `nGenesisBlockRewardCoin` (32768 * COIN) | `32768 * COIN` | YES |
| Base nBits source | `bnProofOfWorkLimit.GetCompact()` | `UintToArith256(params.powLimit).GetCompact()` — same mainnet/testnet compact `0x1e00ffff` | YES |
| Shift math | `(basenBits >> 24 & 0xff) - (nBits >> 24 & 0xff)` | identical | YES |
| Mantissa ratio | `(basenBits & 0x007fffff) / (nBits & 0x007fffff)` as doubles | identical | YES |
| Shift loops | same while loops | same while loops | YES |
| Subsidy composition | `32768 * COIN + int64(sqrt(dDiff * nHeight))` raw satoshis | `32768 * COIN + (CAmount)std::sqrt(dDiff * nHeight)` raw satoshis | YES |
| Fees | added inside `GetBlockValue` (`+ nFees`) | added by caller: `validation.cpp:1869 blockReward = nFees + GetBlockSubsidy(...)` | YES (Core 0.15 convention) |

**PASS.** The QC-corrected formula in the source-of-truth matches the actual 0.15.2 implementation. The old incorrect formulation `32768 + sqrt(h*d) * COIN` is NOT present anywhere in the port. The call-site (`validation.cpp:1869`) uses `pindex->nBits`, confirming `nBits`-based difficulty input rather than a floating helper.

---

## Source-of-Truth Claim Verification

| Claim | Status | Evidence |
|---|---|---|
| Main/test P2P/RPC ports, pchMessageStart, address prefixes, chain ID verified from `../Photon/src` | PASS | Cross-referenced in metrics table |
| Legacy nominal AuxPoW start height 160000 preserved | PASS | `chainparams.cpp:107 nAuxpowStartHeight = 160000`; original `main.cpp:2048 GetAuxPowStartBlock()` returns 160000 for mainnet |
| Secret-key prefixes 128/239 (not TBD) | PASS | `chainparams.cpp:155,258`; legacy `base58.h:405` |
| Reward formula is `32768 * COIN + int64(sqrt(dDiff * nHeight))` from nBits, not `32768 + sqrt(h*d) * COIN` | PASS | Side-by-side above |
| Checkpoint table populated (18 entries, legacy) | PASS | All 18 heights/hashes match `../Photon/src/checkpoints.cpp:36-55` exactly |
| chainTxData sourced from original Photon (1646497409, 2739713, 3000/day) | PASS | `chainparams.cpp:190-194` matches `../Photon/src/checkpoints.cpp:60-63` |
| DNS seeds match shared BlakeStream policy | PASS | `chainparams.cpp:150-151` uses the 2 shared ecosystem seeds `seed.blakestream.io` / `seed.blakecoin.org`. Per policy: all 6 coins use the same seeds; one lookup returns peers for all coins, coin separation happens at the wire-protocol layer via `pchMessageStart` and port. This matches the Blakecoin 0.15.2 reference repo exactly. Historical Photon-specific seeds (`photoncc.com`, four numeric IPs from `../Photon/src/net.cpp`) are intentionally dropped. |
| Shared AuxPow framework integrated (`src/auxpow.{h,cpp}`, `src/primitives/pureheader.{h,cpp}`) | PASS | All four files present |
| AuxPow-aware block/header serialization, disk index persistence | PASS | `primitives/block.{h,cpp}` present |
| AuxPow-aware PoW validation | PASS | `pow.cpp:141-171 CheckAuxPowProofOfWork` |
| Chain-ID-aware block template versions | PASS | `pureheader.h:23-24 VERSION_AUXPOW=1<<8`, `VERSION_CHAIN_START=1<<16` with `SetChainId` / `GetChainId` / `SetAuxpow` / `IsAuxpow` |
| Mainnet strict chain ID = 0x0002 with nominal start 160000 | PASS | `fStrictChainId=true`, `nAuxpowChainId=0x0002`, `nAuxpowStartHeight=160000` |
| Testnet/regtest strict chain ID disabled, start 0 | PASS | `fStrictChainId=false`, `nAuxpowStartHeight=0` on both |
| Legacy-compatible relaxed BIP30 preserved | PASS | `validation.cpp:1770 bool fEnforceBIP30 = !pindex->phashBlock;` matches legacy `../Photon/src/main.cpp:1689` exactly |
| `early-auxpow-block` reject removed; pre-start AuxPow tolerated | PASS | `pow.cpp:143 auxpowActive = nHeight >= nAuxpowStartHeight`. When `!auxpowActive` the validator returns `true` after base PoW check without enforcing chain ID. Pre-start AuxPow-bearing history (e.g. observed height `32050`) is tolerated |
| Modern merged-mining RPCs `createauxblock <address>` / `submitauxblock <hash> <auxpow>` | PASS | `rpc/mining.cpp:936,967`, registered at `1264-1266` |
| `createauxblock` is address-driven | PASS | `rpc/mining.cpp:959-964 DecodeDestination(request.params[0])` |
| `getauxblock` compatibility wrapper maps onto same template/submit flow | PASS | `rpc/mining.cpp:996-1050` uses the same `AuxMiningCreateBlock` / `AuxMiningSubmitBlock` helpers as createauxblock/submitauxblock |
| `getworkaux` out of scope | PASS | Not present anywhere in `src/` |
| Wire checksum preserved as `Hashblake` | PASS | `net.cpp:832,2931`, `base58.cpp:129,144`, `hash.h:90`, `pureheader.cpp:13` |
| No Blakecoin handshake exception copied over | PASS | `net.cpp:828-834 GetMessageHash()` uses `Hashblake` unconditionally |
| Blakecoin 0.15.2 base copied and rebranded | PASS | `configure.ac:11 AC_INIT([Photon Core])`, `build.sh:32-38` COIN_NAME/DAEMON_NAME/QT_NAME/etc. all renamed |
| SegWit signaling May 11 2026 → May 11 2027 | PASS | `chainparams.cpp:124 nStartTime=1778457600`, `nTimeout=1809993600` |
| CSV + testnet/regtest SegWit ALWAYS_ACTIVE | PASS | Verified in all three network blocks |
| BIP34/65/66 disabled on mainnet/testnet | PASS | All set to 100000000 |
| MAX_MONEY = 90B * COIN fits int64 | PASS | 9.0e18 fits 9.223e18 max; ~2.5% headroom |

---

## AuxPoW Framework Integration

| Component | File(s) | Status |
|---|---|---|
| AuxPoW core | `src/auxpow.h`, `src/auxpow.cpp` | PRESENT |
| Pure header (AuxPoW-separated PoW hash basis) | `src/primitives/pureheader.h`, `src/primitives/pureheader.cpp` | PRESENT |
| AuxPoW-aware block/header | `src/primitives/block.h`, `src/primitives/block.cpp` | PRESENT |
| Pure-header Hashblake PoW | `pureheader.cpp:13 Hashblake(BEGIN(nVersion), END(nNonce))` | PASS |
| AuxPow version flag | `pureheader.h:23 VERSION_AUXPOW = 1<<8` | PASS (matches legacy `0x0100`) |
| Chain start multiplier | `pureheader.h:24 VERSION_CHAIN_START = 1<<16` | PASS (matches legacy `0x10000`) |
| `GetChainId()`, `SetChainId()`, `SetAuxpow()`, `IsAuxpow()` | `pureheader.h:81-105` | PASS |
| `CheckAuxPowProofOfWork(block, params, nHeight)` | `pow.cpp:141-171` | PASS |
| Strict chain-ID enforcement gated by nominal start | `pow.cpp:143-146` — only checks chain ID when `auxpowActive && fStrictChainId` | PASS |
| Pre-start AuxPow tolerance | `pow.cpp:161-162 if (!auxpowActive) return true;` when auxpow present | PASS — matches source-of-truth "tolerate historical pre-start AuxPoW-bearing blocks during sync" |
| Mainnet wiring | `chainparams.cpp:101-107` | `fStrictChainId=true`, `nAuxpowChainId=0x0002`, `nAuxpowStartHeight=160000` |
| Testnet wiring | `chainparams.cpp:216-218` | `fStrictChainId=false`, chain ID 0x0002, start 0 |
| Regtest wiring | `chainparams.cpp:304-306` | `fStrictChainId=false`, chain ID 0x0002, start 0 |

Verified regtest AuxPow smoke per source-of-truth: `createauxblock <address>` returns `chainid = 2` matching `consensus.nAuxpowChainId`, and `getauxblock` wrapper also operates. `submitauxblock` end-to-end solved-block acceptance is still pending per QC Status bullet.

---

## RPC Surface

| RPC | Signature | Type | Status |
|---|---|---|---|
| `createauxblock` | `createauxblock <address>` | Primary (address-driven) | PASS — `rpc/mining.cpp:936-965` |
| `submitauxblock` | `submitauxblock <hash> <auxpow>` | Primary | PASS — `rpc/mining.cpp:967-994` |
| `getauxblock` | `getauxblock ( hash auxpow )` | Compat wrapper mapping onto same template/submit flow | PASS — `rpc/mining.cpp:996-1050`, calls `AuxMiningCreateBlock` / `AuxMiningSubmitBlock` (no separate legacy path) |
| `getworkaux` | — | Out of scope per source-of-truth | PASS — not present in `src/` |

Registration block (`rpc/mining.cpp:1264-1266`):
```cpp
{ "mining", "getauxblock",    &getauxblock,    true, {"hash","auxpow"} },
{ "mining", "createauxblock", &createauxblock, true, {"address"}      },
{ "mining", "submitauxblock", &submitauxblock, true, {"hash","auxpow"} },
```

---

## Action Items

### Blockers — must fix before release

None at code level.

### Operational QA gates — not code issues, but release gates

1. **Live bootstrap replay & peer catch-up.** Per QC Status: "Current live daemon QA on the build server now accepts deep historical Photon bootstrap blocks again after restoring the legacy-compatible rule. Keep monitoring until the full bootstrap replay and peer catch-up complete before calling Photon fully green."
2. **`submitauxblock` solved-block acceptance.** Per QC Status: "`submitauxblock` still needs a solved AuxPow payload acceptance pass in isolated QA before the repo should be treated as release-ready for merged-mining operations." Run on regtest/testnet with a synthetic parent-chain AuxPow payload.
3. **Mainnet strict no-send / no-mine rule** stays in force until (1) and (2) are green.
4. **MAX_MONEY overflow audit.** Tight (~2.5% headroom). Per-block subsidy math is bounded and safe, but audit any aggregate arithmetic (supply summation over heights, total-fee rollups, etc.) for int64 overflow before long-running mainnet use.

### Minor / advisory — not blockers

5. **Stale `BEGIN BLAKECOIN` comments.** `chainparams.cpp:81-84` still documents the Blakecoin `25 + sqrt(...)` formula inside the Photon tree. The Photon formula is `32768 * COIN + sqrt(dDiff * nHeight)`. Purely a documentation fix; no consensus impact.
6. **Copyright attribution.** `chainparams.cpp:3` reads "The Blakecoin Developers". Kept intentionally — Blakecoin Developers is the shared ecosystem attribution.
7. **Regtest RPC port.** `chainparamsbase.cpp:60 nRPCPort = 18332` (Bitcoin default). Fine for single-host local QA; flag only if multiple Blake-family regtest daemons must coexist on one host.

---

## Overall Verdict

**Code-level QC: PASS. Zero blockers.**

Every consensus-critical claim in the source-of-truth's QC Status and AuxPoW Start And Completed Work sections is correctly implemented:

- Reward formula matches `../Photon/src/main.cpp:1098-1123` byte-for-byte.
- AuxPoW chain ID `0x0002` + nominal start height `160000` + pre-start tolerance.
- Legacy relaxed BIP30 (`fEnforceBIP30 = !pindex->phashBlock`) preserved.
- Modern `createauxblock` / `submitauxblock` primaries with `getauxblock` compat wrapper sharing the same helpers; no `getworkaux`.
- `Hashblake` P2P wire checksum preserved; no Blakecoin-style handshake exception.
- All 18 legacy mainnet checkpoints ported; `chainTxData` matches original.
- Ports (35556/8984, 18992/18998), magic bytes (f9 bc b4 d2 / 0b 11 09 08), address prefixes (26/7/128, 142/170/239), Bech32 HRPs (`pho`/`tpho`) all correct and cross-referenced against the legacy tree.
- `MAX_MONEY = 90B * COIN` fits int64 with ~2.5% headroom.

DNS seeds are correctly set to the 2 shared BlakeStream ecosystem seeds (`seed.blakestream.io`, `seed.blakecoin.org`) — the same 2 seeds used by all 6 coins and matching the Blakecoin 0.15.2 reference repo exactly. Remaining items are operational QA gates (bootstrap replay completion, solved-block AuxPow acceptance) required before declaring release-ready, not code fixes.
