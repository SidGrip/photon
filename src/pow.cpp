// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2022 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <pow.h>

#include <arith_uint256.h>
#include <chain.h>
#include <logging.h>
#include <primitives/block.h>
#include <uint256.h>

#include <limits>

unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params)
{
    assert(pindexLast != nullptr);
    unsigned int nProofOfWorkLimit = UintToArith256(params.powLimit).GetCompact();

    // Only change once per difficulty adjustment interval
    if ((pindexLast->nHeight+1) % params.DifficultyAdjustmentInterval() != 0)
    {
        if (params.fPowAllowMinDifficultyBlocks)
        {
            // Special difficulty rule for testnet:
            // If the new block's timestamp is more than 2* 10 minutes
            // then allow mining of a min-difficulty block.
            if (pblock->GetBlockTime() > pindexLast->GetBlockTime() + params.nPowTargetSpacing*2)
                return nProofOfWorkLimit;

            // Regtest disables retargeting. Once a block has moved the chain
            // to minimum difficulty, keep that difficulty instead of walking
            // back to the genesis block's inherited Blakecoin target.
            if (params.fPowNoRetargeting)
                return pindexLast->nBits;
            else
            {
                // Return the last non-special-min-difficulty-rules-block
                const CBlockIndex* pindex = pindexLast;
                while (pindex->pprev && pindex->nHeight % params.DifficultyAdjustmentInterval() != 0 && pindex->nBits == nProofOfWorkLimit)
                    pindex = pindex->pprev;
                return pindex->nBits;
            }
        }
        return pindexLast->nBits;
    }

    // Photon preserves the 0.15.21/0.8 retarget lookback rule.
    // After height 150000, use the full interval lookback when calculating the
    // actual timespan, matching Photon-0.15.21 src/pow.cpp.
    const int height = pindexLast->nHeight + 1;
    int blockstogoback = params.DifficultyAdjustmentInterval() - 1;
    if (height >= 150000 && height != params.DifficultyAdjustmentInterval()) {
        blockstogoback = params.DifficultyAdjustmentInterval();
    }

    int nHeightFirst = pindexLast->nHeight - blockstogoback;
    assert(nHeightFirst >= 0);
    const CBlockIndex* pindexFirst = pindexLast->GetAncestor(nHeightFirst);
    assert(pindexFirst);

    return CalculateNextWorkRequired(pindexLast, pindexFirst->GetBlockTime(), params);
}

unsigned int CalculateNextWorkRequired(const CBlockIndex* pindexLast, int64_t nFirstBlockTime, const Consensus::Params& params)
{
    if (params.fPowNoRetargeting)
        return pindexLast->nBits;

    // BEGIN BLAKECOIN: Blakecoin difficulty adjustment limits (ported from
    // 0.15.21 src/pow.cpp:63-87 per user directive "all of the blakecoin
    // consesus should be found in the previous version 15.21", 2026-04-23).
    // Bitcoin uses symmetric 4x / 0.25x bounds; Blakecoin uses
    //   - 15% max difficulty increase pre-height-3500
    //   - 3%  max difficulty increase post-height-3500 (only when blocks
    //     mined too fast: nActualTimespan < nTargetTimespan/4)
    //   - 50% max difficulty decrease
    // This is consensus-critical: using Bitcoin's looser bounds would admit
    // blocks an 0.15.21 network would reject, causing a chain split.
    int64_t nActualTimespan = pindexLast->GetBlockTime() - nFirstBlockTime;
    int64_t nTargetTimespan = params.nPowTargetTimespan;

    int64_t nMinActualTimespan = nTargetTimespan * 100 / 115;       // Up 15% max
    int64_t nMinActualTimespan3Pct = nTargetTimespan * 100 / 103;   // Up 3% max
    int64_t nMaxActualTimespan = nTargetTimespan * 2;               // Down 50% max

    if (nActualTimespan < nTargetTimespan / 4 && pindexLast->nHeight >= 3500)
    {
        nActualTimespan = nMinActualTimespan3Pct;
    }
    else if (nActualTimespan < nMinActualTimespan && pindexLast->nHeight < 3500)
    {
        nActualTimespan = nMinActualTimespan;
    }

    if (nActualTimespan > nMaxActualTimespan)
        nActualTimespan = nMaxActualTimespan;
    // END BLAKECOIN

    // Retarget
    const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);
    arith_uint256 bnNew;
    bnNew.SetCompact(pindexLast->nBits);
    bnNew *= nActualTimespan;
    bnNew /= nTargetTimespan;

    if (bnNew > bnPowLimit)
        bnNew = bnPowLimit;

    return bnNew.GetCompact();
}

// Check that on difficulty adjustments, the new difficulty does not increase
// or decrease beyond the permitted limits.
bool PermittedDifficultyTransition(const Consensus::Params& params, int64_t height, uint32_t old_nbits, uint32_t new_nbits)
{
    if (params.fPowAllowMinDifficultyBlocks) return true;

    if (height % params.DifficultyAdjustmentInterval() == 0) {
        // BEGIN BLAKECOIN: match the valid retarget envelope from
        // CalculateNextWorkRequired without rejecting blocks that 0.15.21
        // would accept. Pre-height-3500, Blakecoin caps difficulty increases
        // at 15%. Post-height-3500, the special 3% clamp only triggers when
        // blocks are faster than nPowTargetTimespan / 4, so the broadest
        // still-valid lower envelope remains the quarter-timespan boundary.
        int64_t smallest_timespan = params.nPowTargetTimespan / 4;
        if (height <= 3500) {
            smallest_timespan = params.nPowTargetTimespan * 100 / 115;
        }
        int64_t largest_timespan = params.nPowTargetTimespan * 2;
        // END BLAKECOIN

        const arith_uint256 pow_limit = UintToArith256(params.powLimit);
        arith_uint256 observed_new_target;
        observed_new_target.SetCompact(new_nbits);

        // Calculate the largest difficulty value possible:
        arith_uint256 largest_difficulty_target;
        largest_difficulty_target.SetCompact(old_nbits);
        largest_difficulty_target *= largest_timespan;
        largest_difficulty_target /= params.nPowTargetTimespan;

        if (largest_difficulty_target > pow_limit) {
            largest_difficulty_target = pow_limit;
        }

        // Round and then compare this new calculated value to what is
        // observed.
        arith_uint256 maximum_new_target;
        maximum_new_target.SetCompact(largest_difficulty_target.GetCompact());
        if (maximum_new_target < observed_new_target) return false;

        // Calculate the smallest difficulty value possible:
        arith_uint256 smallest_difficulty_target;
        smallest_difficulty_target.SetCompact(old_nbits);
        smallest_difficulty_target *= smallest_timespan;
        smallest_difficulty_target /= params.nPowTargetTimespan;

        if (smallest_difficulty_target > pow_limit) {
            smallest_difficulty_target = pow_limit;
        }

        // Round and then compare this new calculated value to what is
        // observed.
        arith_uint256 minimum_new_target;
        minimum_new_target.SetCompact(smallest_difficulty_target.GetCompact());
        if (minimum_new_target > observed_new_target) return false;
    } else if (old_nbits != new_nbits) {
        return false;
    }
    return true;
}

bool CheckProofOfWork(uint256 hash, unsigned int nBits, const Consensus::Params& params)
{
    bool fNegative;
    bool fOverflow;
    arith_uint256 bnTarget;

    bnTarget.SetCompact(nBits, &fNegative, &fOverflow);

    // Check range
    if (fNegative || bnTarget == 0 || fOverflow || bnTarget > UintToArith256(params.powLimit))
        return false;

    // Check proof of work matches claimed amount
    if (UintToArith256(hash) > bnTarget)
        return false;

    return true;
}

bool CheckAuxPowProofOfWork(const CBlockHeader& block, const Consensus::Params& params, int nHeight)
{
    const bool auxpow_active = nHeight >= params.nAuxpowStartHeight;

    if (auxpow_active && params.fStrictChainId && nHeight != std::numeric_limits<int>::max() && block.GetChainId() != params.nAuxpowChainId) {
        return error("%s: block does not have our chain ID", __func__);
    }

    if (!block.auxpow) {
        if (block.IsAuxpow()) {
            return error("%s: no auxpow on block with auxpow version", __func__);
        }

        if (!CheckProofOfWork(block.GetPoWHash(), block.nBits, params)) {
            return error("%s: non-AUX proof of work failed", __func__);
        }

        return true;
    }

    if (!block.IsAuxpow()) {
        return error("%s: auxpow on block with non-auxpow version", __func__);
    }

    if (!auxpow_active) {
        return true;
    }

    if (!CheckProofOfWork(block.auxpow->GetParentBlockPoWHash(), block.nBits, params)) {
        return error("%s: AUX proof of work failed", __func__);
    }

    if (!block.auxpow->Check(block.GetHash(), block.GetChainId(), params)) {
        return error("%s: AUX POW is not valid", __func__);
    }

    return true;
}
