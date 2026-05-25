// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2021 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <kernel/chainparams.h>

#include <chainparamsseeds.h>
#include <consensus/amount.h>
#include <consensus/merkle.h>
#include <consensus/params.h>
#include <hash.h>
#include <chainparamsbase.h>
#include <logging.h>
#include <primitives/block.h>
#include <primitives/transaction.h>
#include <script/interpreter.h>
#include <script/script.h>
#include <uint256.h>
#include <util/strencodings.h>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <limits>
#include <type_traits>

static CBlock CreateGenesisBlock(const char* pszTimestamp, const CScript& genesisOutputScript, uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

/**
 * Build the genesis block. Note that the output of its generation
 * transaction cannot be spent since it did not originally exist in the
 * database.
 *
 * CBlock(hash=000000000019d6, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=4a5e1e, nTime=1231006505, nBits=1d00ffff, nNonce=2083236893, vtx=1)
 *   CTransaction(hash=4a5e1e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
 *     CTxIn(COutPoint(000000, -1), coinbase 04ffff001d0104455468652054696d65732030332f4a616e2f32303039204368616e63656c6c6f72206f6e206272696e6b206f66207365636f6e64206261696c6f757420666f722062616e6b73)
 *     CTxOut(nValue=50.00000000, scriptPubKey=0x5F1DF16B2B704C8A578D0B)
 *   vMerkleTree: 4a5e1e
 */
static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const char* pszTimestamp = "The Times 03/Jan/2009 Chancellor on brink of second bailout for banks";
    const CScript genesisOutputScript = CScript() << ParseHex("04678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5f") << OP_CHECKSIG;
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}

static CBlock CreatePhotonGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    // Photon shares the Blakecoin coinbase scriptSig text (different
    // nTime/nNonce/reward produce a different genesis hash).
    const char* pszTimestamp = "US forces target leading al-Shabaab militant in Somalian coastal raid";
    const CScript genesisOutputScript = CScript() << ParseHex("04678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5f") << OP_CHECKSIG;
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}

/**
 * Main network on which people trade goods and services.
 */
class CMainParams : public CChainParams {
public:
    CMainParams() {
        strNetworkID = CBaseChainParams::MAIN;
        consensus.signet_blocks = false;
        consensus.signet_challenge.clear();
        consensus.nSubsidyHalvingInterval = std::numeric_limits<int>::max();
        // Photon-0.25.2 post-SegWit cleanup group. These are intentional
        // future-height mainnet activations after the verified 0.15.21 SegWit
        // activation at height 2051160.
        consensus.BIP34Height = 2072227;
        consensus.BIP34Hash = uint256{};
        consensus.BIP65Height = 2072227;
        consensus.BIP66Height = 2072227;
        // TODO(blakestream-25.2-activation): CSV (BIP68/112/113) — atomic-swap timeout
        // primitive. ALWAYS_ACTIVE from genesis on Blakestream family per
        // coin-source-of-truth.md "Common rules". Do NOT change.
        consensus.CSVHeight = 1;
        // Photon SegWit is inherited from the 0.15.21 mainnet activation.
        // 0.25.2 buries this height and does not re-signal SegWit.
        consensus.SegwitHeight = 2051160;
        // Suppress historical pre-burial unknown bit 4 warnings from old
        // AuxPoW/version reuse. Future warnings remain enabled from SegWit on.
        consensus.MinBIP9WarningHeight = 2051160;
        consensus.powLimit = uint256S("000000ffff000000000000000000000000000000000000000000000000000000");
        consensus.nPowTargetTimespan = 20 * 3 * 60;
        consensus.nPowTargetSpacing = 3 * 60;
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 19;
        consensus.nMinerConfirmationWindow = 20;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].min_activation_height = 0;
        // Taproot bundle BIP340/BIP341/BIP342
        // (Schnorr + Taproot key/script commitments + Tapscript). This is what enables
        // PTLC-style atomic swaps and MuSig2 cross-chain DEX paths. ALL THE C++
        // MACHINERY IS ALREADY PRESENT in this codebase (verified 2026-04-25):
        // Schnorr verify, key-path, script-path, OP_CHECKSIGADD, TaggedHash byte-
        // identity to upstream Bitcoin Core. Activation values are assigned below.
        //
        // Taproot follows the 0.25.2 cleanup group in a separate BIP9 window.
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].bit = 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nStartTime = 1782871200;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nTimeout = 1814407200;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].min_activation_height = 2075587;

        consensus.nMinimumChainWork = uint256S("0x000000000000000000000000000000000000000000000087ac0e7d8c0ad5852a");
        consensus.defaultAssumeValid = uint256S("0x6e1317a14c360e184284e91f202300859386864d6e630bbad363dbb3ab159f37");

        // Photon AuxPoW chain identity (consumed by Phase 2 AuxPoW core).
        // mainnet: strict chain-ID, AuxPoW activates at historical height 160000.
        consensus.fStrictChainId = true;
        consensus.nAuxpowChainId = 0x0002;
        consensus.nAuxpowStartHeight = 160000;

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         *
         * Photon mainnet differs from Blakecoin in byte 2 (0xbc vs 0xbe).
         */
        pchMessageStart[0] = 0xf9;
        pchMessageStart[1] = 0xbc;
        pchMessageStart[2] = 0xb4;
        pchMessageStart[3] = 0xd2;
        nDefaultPort = 35556;
        nPruneAfterHeight = 100000;
        m_assumed_blockchain_size = 6;
        m_assumed_chain_state_size = 1;

        genesis = CreatePhotonGenesisBlock(1392688072, 5992999, 503382015, 112, 32768 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x000000e79a20d718a2f2d8b98161dc6700104a22d8e9be70e8ac361ee6664b9c"));
        assert(genesis.hashMerkleRoot == uint256S("0x251e462b7d8b2e92e74651186fbbc66ac715cf9c160212efb02642232207112d"));

        // Shared BlakeStream aux-coin DNS seeds.
        vSeeds.emplace_back("seed.blakestream.io");
        vSeeds.emplace_back("seed.blakecoin.org");

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,26);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,7);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,128);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x88, 0xB2, 0x1E};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x88, 0xAD, 0xE4};

        bech32_hrp = "pho";
        vFixedSeeds.clear();

        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        m_is_test_chain = false;
        m_is_mockable_chain = false;

        // Photon mainnet checkpoints, lifted from Photon-0.15.21 and extended
        // from the live 0.25.2 mainnet chain.
        checkpointData = {
            {
                {0,       uint256S("0x000000e79a20d718a2f2d8b98161dc6700104a22d8e9be70e8ac361ee6664b9c")},
                {4684,    uint256S("0x000000000000abb156fb3ba96155f880a8bd41ab3ca744cf45c91c51c7a7c42d")},
                {5050,    uint256S("0x0000000000040709146699c80907b2aae30bb37c4ca99c21c9efe04f82966959")},
                {15200,   uint256S("0x0000000000187d893af37ebcbac6aafa9483291af9248e3f4976c38606950811")},
                {27205,   uint256S("0x00000000001b387301466a4a91aebc2c037556b8a76dcac227ca2796873b08e2")},
                {31300,   uint256S("0x00000000000668d829d8f12684ff0c63b6e7f84bdff988f7ef5831c116fe8615")},
                {49009,   uint256S("0x05e6756348da0e27b1547f530160c1b7aff1955751b3dbe1cef80ee715f0bd0b")},
                {75020,   uint256S("0x07676917780a8b0054f5c0f7006b5d3037f6d68b40af63e4c119e48d96ee90c6")},
                {101017,  uint256S("0x04a75ebd63e3e6cf4ade7958d8978bfaab7d905b081d337aa556b113c0c5c89d")},
                {143027,  uint256S("0x0f9beea1816ff9053a15433373ecce12694ed8521d752d7205c33d8619f6a15e")},
                {180023,  uint256S("0x0b98a2b2358aab5dad42f1d8271da92c3624f00dc6af8ed996f7fae6991a92d2")},
                {220000,  uint256S("0x82640e0dcc90320fea369a3c86dff88173047f309cceb48898eeec46d577917a")},
                {239000,  uint256S("0x228019408d89f524736fa2656ad41979be9f0918b4412415f6f1e704e33e435f")},
                {297000,  uint256S("0xc189ea7f602a4fea8012e35befe137bbd653630868133c7336c2d89501abcc07")},
                {545000,  uint256S("0xfea8243a8fb5034daf47e445caa4e091d61a1bca199b807848c7379d7548a836")},
                {630003,  uint256S("0x5c95cdc3ef47864f195583cc2af80824636f2c728856c8dcc5fbb3d722fc4350")},
                {897332,  uint256S("0x8d51f94fe5e285a0e15bd5cd670522934ac37c115767781bc918867b8a2b85cd")},
                {1346300, uint256S("0x240136ffd659dde451087e10082075844157ee102b08c469c72f4983421d56be")},
                {2032461, uint256S("0xd58aac5395970447777021d84469cc060998732480db912167efdd43a0da2139")},
                {2051160, uint256S("0x56292d0b02685325f6b57c8e96b31033f218eb9bd761f894580b7b91c34c23c1")},
                {2053000, uint256S("0x1a6bcade2ab7d6d9350631f7fe8e9ba3c050c447389f3370bbef69346901ad69")},
                {2055000, uint256S("0x6e1317a14c360e184284e91f202300859386864d6e630bbad363dbb3ab159f37")},
                {2057000, uint256S("0xc49ecbd3f18c1570aacde8e38fccbb27ebb32c2f10dec0766562b936604071aa")},
            }
        };

        m_assumeutxo_data = MapAssumeutxo{
            {
                2057519,
                {
                    AssumeutxoHash{uint256S("0xb76486b1888d365bedb9ed3009fcb1da7184c196e54fe71bd4a74a13030a61a6")},
                    3566180,
                },
            },
        };

        chainTxData = ChainTxData{
            .nTime    = 1779179072,
            .nTxCount = 3563534,
            .dTxRate  = 0.005678206870959294,
        };
    }
};

/**
 * Testnet (v3): public test network which is reset from time to time.
 */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        strNetworkID = CBaseChainParams::TESTNET;
        consensus.signet_blocks = false;
        consensus.signet_challenge.clear();
        consensus.nSubsidyHalvingInterval = std::numeric_limits<int>::max();
        // Photon 25.2 testnet is a feature-test/reset network: keep SegWit,
        // CLTV, strict-DER, BIP34 coinbase height, and Taproot available from
        // height 1 so merged-mining, atomic-swap, and Taproot QA are valid on
        // default testnet. If an old 0.15.21 testnet history must be preserved,
        // replace these height-1 values with explicit future testnet activations.
        consensus.BIP34Height = 1;
        consensus.BIP34Hash = uint256{};
        consensus.BIP65Height = 1;
        consensus.BIP66Height = 1;
        // TODO(blakestream-25.2-activation): CSV (BIP68/112/113) ALWAYS_ACTIVE on
        // Blakestream family — atomic-swap timeout primitive. Do NOT change.
        consensus.CSVHeight = 1;
        // TODO(blakestream-25.2-activation): testnet SegWit ALWAYS_ACTIVE from height
        // 1 so testnet AuxPoW + atomic-swap regression coverage works without
        // waiting on the 0.15.21 mainnet activation. Do NOT change.
        consensus.SegwitHeight = 1;
        consensus.MinBIP9WarningHeight = 0;
        consensus.powLimit = uint256S("000000ffff000000000000000000000000000000000000000000000000000000");
        consensus.nPowTargetTimespan = 20 * 3 * 60;
        consensus.nPowTargetSpacing = 3 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 15;
        consensus.nMinerConfirmationWindow = 20;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].min_activation_height = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].bit = 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nStartTime = Consensus::BIP9Deployment::ALWAYS_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].min_activation_height = 0;

        consensus.nMinimumChainWork = uint256{};
        consensus.defaultAssumeValid = uint256{};

        // Photon AuxPoW: testnet does NOT enforce strict chain-ID, and AuxPoW
        // is acceptable from genesis (no historical pre-AuxPoW height).
        consensus.fStrictChainId = false;
        consensus.nAuxpowChainId = 0x0002;
        consensus.nAuxpowStartHeight = 0;

        pchMessageStart[0] = 0x0b;
        pchMessageStart[1] = 0x11;
        pchMessageStart[2] = 0x09;
        pchMessageStart[3] = 0x08;
        nDefaultPort = 18992;
        nPruneAfterHeight = 1000;
        m_assumed_blockchain_size = 0;
        m_assumed_chain_state_size = 0;

        // Photon testnet has its OWN distinct genesis (different nTime/nNonce
        // from mainnet/regtest). Lifted verbatim from Photon-0.15.21
        // chainparams.cpp:248-250.
        genesis = CreatePhotonGenesisBlock(1392351202, 4335147, 503382015, 112, 32768 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x00000052d978f26d698e0c4dbce9f8139a69f2fbda37715149146776aeb70d5b"));
        assert(genesis.hashMerkleRoot == uint256S("0x251e462b7d8b2e92e74651186fbbc66ac715cf9c160212efb02642232207112d"));

        vFixedSeeds.clear();
        vSeeds.clear();
        // Photon testnet seeds to be added when available.

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,142);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,170);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        bech32_hrp = "tpho";

        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        m_is_test_chain = true;
        m_is_mockable_chain = false;

        checkpointData = {
            {
                {0, uint256S("0x00000052d978f26d698e0c4dbce9f8139a69f2fbda37715149146776aeb70d5b")},
            }
        };

        m_assumeutxo_data = MapAssumeutxo{
            // TODO to be specified in a future patch.
        };

        chainTxData = ChainTxData{
            .nTime    = 1392351202,
            .nTxCount = 1,
            .dTxRate  = 0.01,
        };
    }
};

/**
 * Signet: test network with an additional consensus parameter (see BIP325).
 */
class SigNetParams : public CChainParams {
public:
    explicit SigNetParams(const SigNetOptions& options)
    {
        std::vector<uint8_t> bin;
        vSeeds.clear();

        if (!options.challenge) {
            // Photon signet defaults to a local/private developer network.
            // Keep the default challenge trivial and ship no global seeds,
            // assumevalid, or chainwork so we do not point at Bitcoin signet.
            bin = ParseHex("51");
            consensus.nMinimumChainWork = uint256{};
            consensus.defaultAssumeValid = uint256{};
            m_assumed_blockchain_size = 0;
            m_assumed_chain_state_size = 0;
            chainTxData = ChainTxData{0, 0, 0};
        } else {
            bin = *options.challenge;
            consensus.nMinimumChainWork = uint256{};
            consensus.defaultAssumeValid = uint256{};
            m_assumed_blockchain_size = 0;
            m_assumed_chain_state_size = 0;
            chainTxData = ChainTxData{
                0,
                0,
                0,
            };
            LogPrintf("Signet with challenge %s\n", HexStr(bin));
        }

        if (options.seeds) {
            vSeeds = *options.seeds;
        }

        strNetworkID = CBaseChainParams::SIGNET;
        consensus.signet_blocks = true;
        consensus.signet_challenge.assign(bin.begin(), bin.end());
        consensus.nSubsidyHalvingInterval = std::numeric_limits<int>::max();
        consensus.BIP34Height = 1;
        consensus.BIP34Hash = uint256{};
        consensus.BIP65Height = 1;
        consensus.BIP66Height = 1;
        consensus.CSVHeight = 1;
        consensus.SegwitHeight = 1;
        consensus.nPowTargetTimespan = 20 * 3 * 60;
        consensus.nPowTargetSpacing = 3 * 60;
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 19;
        consensus.nMinerConfirmationWindow = 20;
        consensus.MinBIP9WarningHeight = 0;
        consensus.powLimit = uint256S("00000377ae000000000000000000000000000000000000000000000000000000");
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = Consensus::BIP9Deployment::NEVER_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].min_activation_height = 0; // No activation delay

        // Keep Taproot always active on signet so developer coverage matches
        // regtest/testnet, while mainnet activation policy waits on
        // Photon-0.15.21 mainnet SegWit activation results.
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].bit = 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nStartTime = Consensus::BIP9Deployment::ALWAYS_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].min_activation_height = 0; // No activation delay

        // message start is defined as the first 4 bytes of the sha256d of the block script
        HashWriter h{};
        h << consensus.signet_challenge;
        uint256 hash = h.GetHash();
        memcpy(pchMessageStart, hash.begin(), 4);

        nDefaultPort = 38733;
        nPruneAfterHeight = 1000;

        // Photon signet: defaults to the testnet genesis params (Photon-0.15.21
        // never shipped mainnet signet; signet is a Bitcoin-Core-25.2 inherited
        // facility used here for developer experimentation only).
        consensus.fStrictChainId = false;
        consensus.nAuxpowChainId = 0x0002;
        consensus.nAuxpowStartHeight = 0;
        genesis = CreatePhotonGenesisBlock(1392351202, 4335147, 503382015, 112, 32768 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x00000052d978f26d698e0c4dbce9f8139a69f2fbda37715149146776aeb70d5b"));
        assert(genesis.hashMerkleRoot == uint256S("0x251e462b7d8b2e92e74651186fbbc66ac715cf9c160212efb02642232207112d"));

        vFixedSeeds.clear();

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,142);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,170);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        bech32_hrp = "tpho";

        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        m_is_test_chain = true;
        m_is_mockable_chain = false;
    }
};

/**
 * Regression test: intended for private networks only. Has minimal difficulty to ensure that
 * blocks can be found instantly.
 */
class CRegTestParams : public CChainParams
{
public:
    explicit CRegTestParams(const RegTestOptions& opts)
    {
        strNetworkID =  CBaseChainParams::REGTEST;
        consensus.signet_blocks = false;
        consensus.signet_challenge.clear();
        consensus.nSubsidyHalvingInterval = 150;
        consensus.BIP34Height = 100000000;
        consensus.BIP34Hash = uint256{};
        consensus.BIP65Height = 1351;
        consensus.BIP66Height = 1251;
        consensus.CSVHeight = 1;
        consensus.SegwitHeight = 0;
        consensus.MinBIP9WarningHeight = 0;
        consensus.powLimit = uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 20 * 3 * 60;
        consensus.nPowTargetSpacing = 3 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = true;
        consensus.nRuleChangeActivationThreshold = 108;
        consensus.nMinerConfirmationWindow = 144;

        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].min_activation_height = 0; // No activation delay

        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].bit = 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nStartTime = Consensus::BIP9Deployment::ALWAYS_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].min_activation_height = 0; // No activation delay

        consensus.nMinimumChainWork = uint256{};
        consensus.defaultAssumeValid = uint256{};

        // Photon regtest: AuxPoW machinery available from genesis, no
        // strict chain-ID enforcement (mirrors Photon-0.15.21).
        consensus.fStrictChainId = false;
        consensus.nAuxpowChainId = 0x0002;
        consensus.nAuxpowStartHeight = 0;

        pchMessageStart[0] = 0xfa;
        pchMessageStart[1] = 0xbf;
        pchMessageStart[2] = 0xb5;
        pchMessageStart[3] = 0xda;
        nDefaultPort = 18444;
        nPruneAfterHeight = opts.fastprune ? 100 : 1000;
        m_assumed_blockchain_size = 0;
        m_assumed_chain_state_size = 0;

        for (const auto& [dep, height] : opts.activation_heights) {
            switch (dep) {
            case Consensus::BuriedDeployment::DEPLOYMENT_SEGWIT:
                consensus.SegwitHeight = int{height};
                break;
            case Consensus::BuriedDeployment::DEPLOYMENT_HEIGHTINCB:
                consensus.BIP34Height = int{height};
                break;
            case Consensus::BuriedDeployment::DEPLOYMENT_DERSIG:
                consensus.BIP66Height = int{height};
                break;
            case Consensus::BuriedDeployment::DEPLOYMENT_CLTV:
                consensus.BIP65Height = int{height};
                break;
            case Consensus::BuriedDeployment::DEPLOYMENT_CSV:
                consensus.CSVHeight = int{height};
                break;
            }
        }

        for (const auto& [deployment_pos, version_bits_params] : opts.version_bits_parameters) {
            consensus.vDeployments[deployment_pos].nStartTime = version_bits_params.start_time;
            consensus.vDeployments[deployment_pos].nTimeout = version_bits_params.timeout;
            consensus.vDeployments[deployment_pos].min_activation_height = version_bits_params.min_activation_height;
        }

        // Photon regtest reuses the Photon mainnet genesis parameters
        // (Photon-0.15.21 chainparams.cpp:332-334). Testnet has its own
        // distinct genesis and is NOT shared with regtest/mainnet.
        genesis = CreatePhotonGenesisBlock(1392688072, 5992999, 503382015, 112, 32768 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x000000e79a20d718a2f2d8b98161dc6700104a22d8e9be70e8ac361ee6664b9c"));
        assert(genesis.hashMerkleRoot == uint256S("0x251e462b7d8b2e92e74651186fbbc66ac715cf9c160212efb02642232207112d"));

        vFixedSeeds.clear(); //!< Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();
        vSeeds.emplace_back("dummySeed.invalid.");

        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        m_is_test_chain = true;
        m_is_mockable_chain = true;

        checkpointData = {
            {
                {0, uint256S("0x000000e79a20d718a2f2d8b98161dc6700104a22d8e9be70e8ac361ee6664b9c")},
            }
        };

        // Photon regtest assumeutxo snapshot at height 130. The donor
        // Blakecoin-25.2 hash (`75e404e4…`) does not apply: Photon regtest
        // has a different genesis, so the height-130 chainstate (txoutset)
        // hash is different. Captured by running the regtest 130-block
        // TestChain100Setup sequence and dumping the resulting snapshot's
        // `txoutset_hash`.
        m_assumeutxo_data = MapAssumeutxo{
            {
                130,
                {
                    AssumeutxoHash{uint256S("0x2657be9892a9c85dc17b94beb739acaf74e92b777d6e46910e807451b75af428")},
                    131,
                },
            },
        };

        chainTxData = ChainTxData{
            0,
            0,
            0
        };

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,26);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,7);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,128);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x88, 0xB2, 0x1E};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x88, 0xAD, 0xE4};

        bech32_hrp = "rpho";
    }
};

std::unique_ptr<const CChainParams> CChainParams::SigNet(const SigNetOptions& options)
{
    return std::make_unique<const SigNetParams>(options);
}

std::unique_ptr<const CChainParams> CChainParams::RegTest(const RegTestOptions& options)
{
    return std::make_unique<const CRegTestParams>(options);
}

std::unique_ptr<const CChainParams> CChainParams::Main()
{
    return std::make_unique<const CMainParams>();
}

std::unique_ptr<const CChainParams> CChainParams::TestNet()
{
    return std::make_unique<const CTestNetParams>();
}
