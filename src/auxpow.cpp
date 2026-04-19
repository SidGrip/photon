// Copyright (c) 2011 Vince Durham
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "auxpow.h"

#include "compat/endian.h"
#include "consensus/merkle.h"
#include "hash.h"
#include "primitives/block.h"
#include "script/script.h"
#include "util.h"

#include <algorithm>
#include <cstring>

bool CAuxPow::Check(const uint256& hashAuxBlock, int nChainId, const Consensus::Params& params) const
{
    if (params.fStrictChainId && parentBlock.GetChainId() == nChainId)
        return error("Aux POW parent has our chain ID");

    if (vChainMerkleBranch.size() > 30)
        return error("Aux POW chain merkle branch too long");

    const uint256 nRootHash = CheckMerkleBranch(hashAuxBlock, vChainMerkleBranch, nChainIndex);
    std::vector<unsigned char> vchRootHash(nRootHash.begin(), nRootHash.end());
    std::reverse(vchRootHash.begin(), vchRootHash.end());

    if (CheckMerkleBranch(coinbaseTx->GetHash(), vMerkleBranch, 0) != parentBlock.hashMerkleRoot)
        return error("Aux POW merkle root incorrect");

    if (coinbaseTx->vin.empty())
        return error("Aux POW coinbase has no inputs");

    const CScript script = coinbaseTx->vin[0].scriptSig;
    CScript::const_iterator pcHead =
        std::search(script.begin(), script.end(), UBEGIN(pchMergedMiningHeader), UEND(pchMergedMiningHeader));
    CScript::const_iterator pc =
        std::search(script.begin(), script.end(), vchRootHash.begin(), vchRootHash.end());

    if (pc == script.end())
        return error("Aux POW missing chain merkle root in parent coinbase");

    if (pcHead != script.end()) {
        if (script.end() != std::search(pcHead + 1, script.end(), UBEGIN(pchMergedMiningHeader), UEND(pchMergedMiningHeader)))
            return error("Multiple merged mining headers in coinbase");
        if (pcHead + sizeof(pchMergedMiningHeader) != pc)
            return error("Merged mining header is not just before chain merkle root");
    } else {
        if (pc - script.begin() > 20)
            return error("Aux POW chain merkle root must start in the first 20 bytes of the parent coinbase");
    }

    pc += vchRootHash.size();
    if (script.end() - pc < 8)
        return error("Aux POW missing chain merkle tree size and nonce in parent coinbase");

    uint32_t nSize;
    memcpy(&nSize, &pc[0], 4);
    nSize = le32toh(nSize);
    const unsigned merkleHeight = vChainMerkleBranch.size();
    if (nSize != (1u << merkleHeight))
        return error("Aux POW merkle branch size does not match parent coinbase");

    uint32_t nNonce;
    memcpy(&nNonce, &pc[4], 4);
    nNonce = le32toh(nNonce);
    if (nChainIndex != (unsigned int)GetExpectedIndex(nNonce, nChainId, merkleHeight))
        return error("Aux POW wrong index");

    return true;
}

int CAuxPow::GetExpectedIndex(uint32_t nNonce, int nChainId, unsigned h)
{
    uint32_t rand = nNonce;
    rand = rand * 1103515245 + 12345;
    rand += nChainId;
    rand = rand * 1103515245 + 12345;

    return rand % (1u << h);
}

uint256 CAuxPow::CheckMerkleBranch(uint256 hash, const std::vector<uint256>& vMerkleBranch, int nIndex)
{
    if (nIndex == -1)
        return uint256();

    for (const uint256& branchHash : vMerkleBranch) {
        if (nIndex & 1) {
            hash = Hash4(BEGIN(branchHash), END(branchHash), BEGIN(hash), END(hash));
        } else {
            hash = Hash4(BEGIN(hash), END(hash), BEGIN(branchHash), END(branchHash));
        }
        nIndex >>= 1;
    }

    return hash;
}

void CAuxPow::InitAuxPow(CBlockHeader& header)
{
    header.SetAuxpowFlag(true);

    const uint256 blockHash = header.GetHash();
    std::vector<unsigned char> inputData(blockHash.begin(), blockHash.end());
    std::reverse(inputData.begin(), inputData.end());
    inputData.push_back(1);
    inputData.insert(inputData.end(), 7, 0);

    CMutableTransaction coinbase;
    coinbase.vin.resize(1);
    coinbase.vin[0].prevout.SetNull();
    coinbase.vin[0].scriptSig = (CScript() << inputData);
    CTransactionRef coinbaseRef = MakeTransactionRef(std::move(coinbase));

    CBlock parent;
    parent.nVersion = 1;
    parent.vtx.resize(1);
    parent.vtx[0] = coinbaseRef;
    parent.hashMerkleRoot = BlockMerkleRoot(parent);

    header.SetAuxpow(new CAuxPow(coinbaseRef));
    header.auxpow->nChainIndex = 0;
    header.auxpow->parentBlock = parent;
}
