// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2013-2026 The Blakecoin Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"
#include "consensus/merkle.h"

#include "tinyformat.h"
#include "util.h"
#include "utilstrencodings.h"

#include <assert.h>
#include <limits>

#include "chainparamsseeds.h"

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
 * Blakecoin Genesis Block:
 * CBlock(hash=0000000f14c5..., ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=4a5e1e, nTime=1372066561, nBits=1d00ffff, nNonce=421575, vtx=1)
 *   CTransaction(hash=4a5e1e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
 *     CTxIn(COutPoint(000000, -1), coinbase 04ffff001d010445...)
 *     CTxOut(nValue=50.00000000, scriptPubKey=...)
 *   vMerkleTree: 4a5e1e
 */
static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    // Photon legacy genesis block parameters.
    const char* pszTimestamp = "US forces target leading al-Shabaab militant in Somalian coastal raid";
    const CScript genesisOutputScript = CScript() << ParseHex("04678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5f") << OP_CHECKSIG;
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}

void CChainParams::UpdateVersionBitsParameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
{
    consensus.vDeployments[d].nStartTime = nStartTime;
    consensus.vDeployments[d].nTimeout = nTimeout;
}

/**
 * Main network
 */
/**
 * What makes a good checkpoint block?
 * + Is surrounded by blocks with reasonable timestamps
 *   (no blocks before with a timestamp after, none after with
 *    timestamp before)
 * + Contains no strange transactions
 */

class CMainParams : public CChainParams {
public:
    CMainParams() {
        strNetworkID = "main";
        // BEGIN BLAKECOIN: Blakecoin uses dynamic subsidy, not halving
        // Subsidy formula: 25 + sqrt(difficulty * height) BLC
        consensus.nSubsidyHalvingInterval = std::numeric_limits<int>::max(); // No halving
        // END BLAKECOIN
        // BEGIN BLAKECOIN: Set BIP heights to disable version checks for historical blocks
        // Blakecoin uses different block versioning - disable these checks
        consensus.BIP34Height = 100000000; // Disabled - far in future
        consensus.BIP34Hash = uint256S("0x000000000000024b89b42a942fe0d9fea3bb44ab7bd1b19115dd6a759c0808b8");
        consensus.BIP65Height = 100000000; // Disabled - far in future
        consensus.BIP66Height = 100000000; // Disabled - far in future
        // END BLAKECOIN
        // BEGIN BLAKECOIN: Proof of work limit: difficulty 1 target from nBits 0x1e00ffff
        // Target = 0x00ffff * 256^27 = 000000ffff000000000000000000000000000000000000000000000000000000
        consensus.powLimit = uint256S("000000ffff000000000000000000000000000000000000000000000000000000");
        // Difficulty adjustment: every 20 blocks (1 hour with 3-minute blocks)
        consensus.nPowTargetTimespan = 20 * 3 * 60;       // 1 hour (20 blocks * 3 minutes)
        consensus.nPowTargetSpacing = 3 * 60;             // 3 minutes (Blakecoin)
        // END BLAKECOIN
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.fStrictChainId = true;
        consensus.nAuxpowChainId = 0x0002;
        // Preserve Photon's legacy nominal AuxPoW activation height. Historical
        // bootstrap data can carry AuxPoW-bearing blocks earlier than this, and
        // the legacy 0.8.x code tolerated those during sync by only enforcing
        // chain-ID rules once the nominal boundary was reached.
        consensus.nAuxpowStartHeight = 160000;
        // BEGIN BLAKECOIN: Rule change threshold for 20-block interval
        consensus.nRuleChangeActivationThreshold = 19; // 95% of 20
        consensus.nMinerConfirmationWindow = 20; // nPowTargetTimespan / nPowTargetSpacing
        // END BLAKECOIN
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = Consensus::BIP9Deployment::ALWAYS_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;

        // Mainnet SegWit signaling starts on May 11, 2026 00:00:00 UTC and
        // times out on May 11, 2027 00:00:00 UTC.
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 1778457600;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 1809993600;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x0000000000000000000000000000000000000000000000000000000000000000");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x0000000000000000000000000000000000000000000000000000000000000000");

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0xf9;
        pchMessageStart[1] = 0xbc;
        pchMessageStart[2] = 0xb4;
        pchMessageStart[3] = 0xd2;
        nDefaultPort = 35556;
        nPruneAfterHeight = 100000;

        genesis = CreateGenesisBlock(1392688072, 5992999, 503382015, 112, 32768 * COIN);
        consensus.hashGenesisBlock = uint256S("0x000000e79a20d718a2f2d8b98161dc6700104a22d8e9be70e8ac361ee6664b9c");
        assert(genesis.hashMerkleRoot == uint256S("0x251e462b7d8b2e92e74651186fbbc66ac715cf9c160212efb02642232207112d"));

        // BlakeStream ecosystem DNS seeds — shared across all 6 coins
        vSeeds.emplace_back("blakestream.io", "seed.blakestream.io", false);
        vSeeds.emplace_back("blakecoin.org", "seed.blakecoin.org", false);

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,26);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,7);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,128);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x88, 0xB2, 0x1E};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x88, 0xAD, 0xE4};
        bech32_hrp = "pho";

        vFixedSeeds.clear();

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;

        checkpointData = (CCheckpointData) {
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
            }
        };

        chainTxData = ChainTxData{
            1646497409,
            2739713,
            3000.0 / (24 * 60 * 60)
        };
    }
};

/**
 * Testnet (v3)
 */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        strNetworkID = "test";
        // Keep testnet aligned with Blakecoin mainnet instead of inherited Bitcoin defaults.
        consensus.nSubsidyHalvingInterval = std::numeric_limits<int>::max(); // No halving
        consensus.BIP34Height = 100000000; // Disabled - far in future
        consensus.BIP34Hash = uint256();
        consensus.BIP65Height = 100000000; // Disabled - far in future
        consensus.BIP66Height = 100000000; // Disabled - far in future
        consensus.powLimit = uint256S("000000ffff000000000000000000000000000000000000000000000000000000");
        consensus.nPowTargetTimespan = 20 * 3 * 60; // 1 hour (20 blocks * 3 minutes)
        consensus.nPowTargetSpacing = 3 * 60;             // 3 minutes (Blakecoin)
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = false;
        consensus.fStrictChainId = false;
        consensus.nAuxpowChainId = 0x0002;
        consensus.nAuxpowStartHeight = 0;
        consensus.nRuleChangeActivationThreshold = 15; // 75% of 20
        consensus.nMinerConfirmationWindow = 20;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = Consensus::BIP9Deployment::ALWAYS_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;

        // Deployment of SegWit (BIP141, BIP143, and BIP147)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = Consensus::BIP9Deployment::ALWAYS_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x0000000000000000000000000000000000000000000000000000000000000000");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x0000000000000000000000000000000000000000000000000000000000000000");

        pchMessageStart[0] = 0x0b;
        pchMessageStart[1] = 0x11;
        pchMessageStart[2] = 0x09;
        pchMessageStart[3] = 0x08;
        nDefaultPort = 18992;
        nPruneAfterHeight = 1000;

        genesis = CreateGenesisBlock(1392351202, 4335147, 503382015, 112, 32768 * COIN);
        consensus.hashGenesisBlock = uint256S("0x00000052d978f26d698e0c4dbce9f8139a69f2fbda37715149146776aeb70d5b");
        assert(genesis.hashMerkleRoot == uint256S("0x251e462b7d8b2e92e74651186fbbc66ac715cf9c160212efb02642232207112d"));

        vFixedSeeds.clear();
        vSeeds.clear();
        // Testnet seeds to be added when available

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,142);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,170);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};
        bech32_hrp = "tpho";

        vFixedSeeds.clear();

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;

        checkpointData = (CCheckpointData) {
            {
                {0, uint256S("0x00000052d978f26d698e0c4dbce9f8139a69f2fbda37715149146776aeb70d5b")},
            }
        };

        chainTxData = ChainTxData{
            // Data as of testnet genesis
            1392351202,
            1,
            0.01
        };

    }
};

/**
 * Regression test
 */
class CRegTestParams : public CChainParams {
public:
    CRegTestParams() {
        strNetworkID = "regtest";
        consensus.nSubsidyHalvingInterval = 150;
        consensus.BIP34Height = 100000000; // BIP34 has not activated on regtest (far in the future so block v1 are not rejected in tests)
        consensus.BIP34Hash = uint256();
        consensus.BIP65Height = 1351; // BIP65 activated on regtest (Used in rpc activation tests)
        consensus.BIP66Height = 1251; // BIP66 activated on regtest (Used in rpc activation tests)
        consensus.powLimit = uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        // Keep regtest useful for local testing, but use Blakecoin's 3-minute cadence.
        consensus.nPowTargetTimespan = 20 * 3 * 60; // 1 hour
        consensus.nPowTargetSpacing = 3 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = true;
        consensus.fStrictChainId = false;
        consensus.nAuxpowChainId = 0x0002;
        consensus.nAuxpowStartHeight = 0;
        consensus.nRuleChangeActivationThreshold = 108; // 75% for testchains
        consensus.nMinerConfirmationWindow = 144; // Faster than normal for regtest (144 instead of 2016)
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = Consensus::BIP9Deployment::ALWAYS_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = Consensus::BIP9Deployment::ALWAYS_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00");

        pchMessageStart[0] = 0xfa;
        pchMessageStart[1] = 0xbf;
        pchMessageStart[2] = 0xb5;
        pchMessageStart[3] = 0xda;
        nDefaultPort = 18444;
        nPruneAfterHeight = 1000;

        genesis = CreateGenesisBlock(1392688072, 5992999, 503382015, 112, 32768 * COIN);
        consensus.hashGenesisBlock = uint256S("0x000000e79a20d718a2f2d8b98161dc6700104a22d8e9be70e8ac361ee6664b9c");
        assert(genesis.hashMerkleRoot == uint256S("0x251e462b7d8b2e92e74651186fbbc66ac715cf9c160212efb02642232207112d"));

        vFixedSeeds.clear(); //!< Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();      //!< Regtest mode doesn't have any DNS seeds.

        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;

        checkpointData = (CCheckpointData){
            {
                {0, uint256S("0x000000e79a20d718a2f2d8b98161dc6700104a22d8e9be70e8ac361ee6664b9c")}
            }
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

static std::unique_ptr<CChainParams> globalChainParams;

const CChainParams &Params() {
    assert(globalChainParams);
    return *globalChainParams;
}

std::unique_ptr<CChainParams> CreateChainParams(const std::string& chain)
{
    if (chain == CBaseChainParams::MAIN)
        return std::unique_ptr<CChainParams>(new CMainParams());
    else if (chain == CBaseChainParams::TESTNET)
        return std::unique_ptr<CChainParams>(new CTestNetParams());
    else if (chain == CBaseChainParams::REGTEST)
        return std::unique_ptr<CChainParams>(new CRegTestParams());
    throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string& network)
{
    SelectBaseParams(network);
    globalChainParams = CreateChainParams(network);
}

void UpdateVersionBitsParameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
{
    globalChainParams->UpdateVersionBitsParameters(d, nStartTime, nTimeout);
}
