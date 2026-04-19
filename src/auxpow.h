// Copyright (c) 2011 Vince Durham
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_AUXPOW_H
#define BITCOIN_AUXPOW_H

#include "consensus/params.h"
#include "primitives/pureheader.h"
#include "primitives/transaction.h"
#include "serialize.h"
#include "uint256.h"

#include <vector>

class CBlockHeader;

static const unsigned char pchMergedMiningHeader[] = { 0xfa, 0xbe, 'm', 'm' };

class CAuxPow
{
private:
    CTransactionRef coinbaseTx;
    std::vector<uint256> vMerkleBranch;

    static uint256 CheckMerkleBranch(uint256 hash, const std::vector<uint256>& vMerkleBranch, int nIndex);

public:
    std::vector<uint256> vChainMerkleBranch;
    unsigned int nChainIndex;
    CPureBlockHeader parentBlock;

    explicit CAuxPow(CTransactionRef txIn)
        : coinbaseTx(std::move(txIn)), nChainIndex(0)
    {
    }

    CAuxPow()
        : nChainIndex(0)
    {
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        uint256 hashBlock;
        int nIndex = 0;

        READWRITE(coinbaseTx);
        READWRITE(hashBlock);
        READWRITE(vMerkleBranch);
        READWRITE(nIndex);
        READWRITE(vChainMerkleBranch);
        READWRITE(nChainIndex);
        READWRITE(parentBlock);
    }

    bool Check(const uint256& hashAuxBlock, int nChainId, const Consensus::Params& params) const;

    uint256 GetParentBlockPoWHash() const
    {
        return parentBlock.GetPoWHash();
    }

    const CTransactionRef& GetCoinbaseTx() const
    {
        return coinbaseTx;
    }

    const std::vector<uint256>& GetCoinbaseMerkleBranch() const
    {
        return vMerkleBranch;
    }

    static int GetExpectedIndex(uint32_t nNonce, int nChainId, unsigned h);
    static void InitAuxPow(CBlockHeader& header);
};

#endif // BITCOIN_AUXPOW_H
