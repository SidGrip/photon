// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2013-2026 The Blakecoin Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_PRIMITIVES_PUREHEADER_H
#define BITCOIN_PRIMITIVES_PUREHEADER_H

#include "serialize.h"
#include "uint256.h"

/**
 * A block header without auxpow information.
 *
 * This breaks the dependency cycle between the child block header and the
 * auxpow payload, which itself carries a parent block header.
 */
class CPureBlockHeader
{
public:
    static const int BLOCK_VERSION_DEFAULT = (1 << 4);
    static const int VERSION_AUXPOW = (1 << 8);
    static const int VERSION_CHAIN_START = (1 << 16);
    // AuxPoW only needs a small chain-id field. Keep it bounded so the
    // versionbits top pattern can coexist with AuxPoW chain IDs.
    static const int VERSION_CHAIN_MASK = (0xFF << 16);

    int nVersion;
    uint256 hashPrevBlock;
    uint256 hashMerkleRoot;
    unsigned int nTime;
    unsigned int nBits;
    unsigned int nNonce;

    CPureBlockHeader()
    {
        SetNull();
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(this->nVersion);
        READWRITE(hashPrevBlock);
        READWRITE(hashMerkleRoot);
        READWRITE(nTime);
        READWRITE(nBits);
        READWRITE(nNonce);
    }

    void SetNull()
    {
        nVersion = BLOCK_VERSION_DEFAULT;
        hashPrevBlock.SetNull();
        hashMerkleRoot.SetNull();
        nTime = 0;
        nBits = 0;
        nNonce = 0;
    }

    bool IsNull() const
    {
        return (nBits == 0);
    }

    uint256 GetHash() const;
    uint256 GetPoWHash() const;

    int64_t GetBlockTime() const
    {
        return (int64_t)nTime;
    }

    inline int GetBaseVersion() const
    {
        return GetBaseVersion(nVersion);
    }

    static inline int GetBaseVersion(int version)
    {
        return version & ~VERSION_AUXPOW & ~VERSION_CHAIN_MASK;
    }

    void SetBaseVersion(int baseVersion, int chainId)
    {
        const int modifiers = nVersion & VERSION_AUXPOW;
        nVersion = (baseVersion & ~VERSION_AUXPOW & ~VERSION_CHAIN_MASK) |
                   modifiers |
                   ((chainId & 0xFF) * VERSION_CHAIN_START);
    }

    inline int GetChainId() const
    {
        return (nVersion & VERSION_CHAIN_MASK) / VERSION_CHAIN_START;
    }

    inline bool IsAuxpow() const
    {
        return nVersion & VERSION_AUXPOW;
    }

    inline void SetAuxpowFlag(bool auxpow)
    {
        if (auxpow) {
            nVersion |= VERSION_AUXPOW;
        } else {
            nVersion &= ~VERSION_AUXPOW;
        }
    }
};

#endif // BITCOIN_PRIMITIVES_PUREHEADER_H
