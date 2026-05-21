// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2022 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <consensus/amount.h>
#include <policy/feerate.h>
#include <tinyformat.h>

#include <cmath>
#include <limits>

namespace {
CAmount ScaleFeeRatePerK(const CAmount& nFeePaid, int64_t num_bytes)
{
    const CAmount quotient{nFeePaid / num_bytes};
    const CAmount remainder{nFeePaid % num_bytes};
    const CAmount limit_max{std::numeric_limits<CAmount>::max()};
    const CAmount limit_min{std::numeric_limits<CAmount>::min()};

    if (quotient > limit_max / 1000) return limit_max;
    if (quotient < limit_min / 1000) return limit_min;

    const CAmount base{quotient * 1000};
    const CAmount tail{(remainder * 1000) / num_bytes};
    if (tail > 0 && base > limit_max - tail) return limit_max;
    if (tail < 0 && base < limit_min - tail) return limit_min;
    return base + tail;
}
} // namespace

CFeeRate::CFeeRate(const CAmount& nFeePaid, uint32_t num_bytes)
{
    const int64_t nSize{num_bytes};

    if (nSize > 0) {
        nSatoshisPerK = ScaleFeeRatePerK(nFeePaid, nSize);
    } else {
        nSatoshisPerK = 0;
    }
}

CAmount CFeeRate::GetFee(uint32_t num_bytes) const
{
    const int64_t nSize{num_bytes};

    // Be explicit that we're converting from a double to int64_t (CAmount) here.
    // We've previously had issues with the silent double->int64_t conversion.
    CAmount nFee{static_cast<CAmount>(std::ceil(nSatoshisPerK * nSize / 1000.0))};

    if (nFee == 0 && nSize != 0) {
        if (nSatoshisPerK > 0) nFee = CAmount(1);
        if (nSatoshisPerK < 0) nFee = CAmount(-1);
    }

    return nFee;
}

std::string CFeeRate::ToString(const FeeEstimateMode& fee_estimate_mode) const
{
    switch (fee_estimate_mode) {
    case FeeEstimateMode::SAT_VB: return strprintf("%d.%03d %s/vB", nSatoshisPerK / 1000, nSatoshisPerK % 1000, CURRENCY_ATOM);
    default:                      return strprintf("%d.%08d %s/kvB", nSatoshisPerK / COIN, nSatoshisPerK % COIN, CURRENCY_UNIT);
    }
}
