// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2021 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_CONSENSUS_AMOUNT_H
#define BITCOIN_CONSENSUS_AMOUNT_H

#include <cstdint>

/** Amount in satoshis (Can be negative) */
typedef int64_t CAmount;

/** The amount of satoshis in one PHO. */
static constexpr CAmount COIN = 100000000;

/** No amount larger than this (in satoshi) is valid.
 *
 * Photon's money-range sanity ceiling (per-output and per-transaction-sum),
 * consensus-critical. Held at 10,000,000,000 coins, which is below INT64_MAX/2
 * so the wallet's change-target math (payment_value * 2 in GenerateChangeTarget)
 * cannot overflow.
 *
 * CONSENSUS CAVEAT: this value must match the live network's cap and must be
 * >= every confirmed on-chain single output and per-transaction output-sum, or
 * nodes will reject those blocks ("bad-txns-vout-toolarge" /
 * "bad-txns-txouttotal-toolarge") and fork. Lowering it relative to the live
 * 0.15.21 network (90,000,000,000) is only safe as a coordinated upgrade after
 * verifying the largest confirmed output/tx-sum on-chain. See repos/maxmoney-qc.md.
 * */
static constexpr CAmount MAX_MONEY = 10000000000LL * COIN;
inline bool MoneyRange(const CAmount& nValue) { return (nValue >= 0 && nValue <= MAX_MONEY); }

#endif // BITCOIN_CONSENSUS_AMOUNT_H
