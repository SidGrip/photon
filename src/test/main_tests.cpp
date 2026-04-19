// Copyright (c) 2014-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"
#include "arith_uint256.h"
#include "validation.h"
#include "net.h"

#include "test/test_bitcoin.h"

#include <cmath>

#include <boost/signals2/signal.hpp>
#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_SUITE(main_tests, TestingSetup)

static CAmount PhotonBlockSubsidyExpected(int nHeight, unsigned int nBits, const Consensus::Params& consensusParams)
{
    if (nHeight == 0) {
        return 32768 * COIN;
    }

    const unsigned int baseBits = UintToArith256(consensusParams.powLimit).GetCompact();
    int nShift = int((baseBits >> 24) & 0xff) - int((nBits >> 24) & 0xff);
    double dDiff = double(baseBits & 0x007fffff) / double(nBits & 0x007fffff);
    while (nShift > 0) {
        dDiff *= 256.0;
        --nShift;
    }
    while (nShift < 0) {
        dDiff /= 256.0;
        ++nShift;
    }

    return 32768 * COIN + static_cast<CAmount>(std::sqrt(dDiff * nHeight));
}

BOOST_AUTO_TEST_CASE(block_subsidy_test)
{
    const auto chainParams = CreateChainParams(CBaseChainParams::MAIN);
    const Consensus::Params& consensusParams = chainParams->GetConsensus();
    const unsigned int baseBits = UintToArith256(consensusParams.powLimit).GetCompact();
    const unsigned int harderBits = baseBits - 0x01000000;

    BOOST_CHECK_EQUAL(GetBlockSubsidy(0, baseBits, consensusParams), 32768 * COIN);
    BOOST_CHECK_EQUAL(GetBlockSubsidy(1, baseBits, consensusParams), PhotonBlockSubsidyExpected(1, baseBits, consensusParams));
    BOOST_CHECK_EQUAL(GetBlockSubsidy(4, baseBits, consensusParams), PhotonBlockSubsidyExpected(4, baseBits, consensusParams));
    BOOST_CHECK_EQUAL(GetBlockSubsidy(9, baseBits, consensusParams), PhotonBlockSubsidyExpected(9, baseBits, consensusParams));
    BOOST_CHECK(GetBlockSubsidy(1, harderBits, consensusParams) > GetBlockSubsidy(1, baseBits, consensusParams));
}

BOOST_AUTO_TEST_CASE(subsidy_limit_test)
{
    const auto chainParams = CreateChainParams(CBaseChainParams::MAIN);
    const Consensus::Params& consensusParams = chainParams->GetConsensus();
    const unsigned int baseBits = UintToArith256(consensusParams.powLimit).GetCompact();

    for (int nHeight : {0, 1, 4, 9, 1000, 100000}) {
        CAmount nSubsidy = GetBlockSubsidy(nHeight, baseBits, consensusParams);
        BOOST_CHECK_EQUAL(nSubsidy, PhotonBlockSubsidyExpected(nHeight, baseBits, consensusParams));
        BOOST_CHECK(MoneyRange(nSubsidy));
    }
}

bool ReturnFalse() { return false; }
bool ReturnTrue() { return true; }

BOOST_AUTO_TEST_CASE(test_combiner_all)
{
    boost::signals2::signal<bool (), CombinerAll> Test;
    BOOST_CHECK(Test());
    Test.connect(&ReturnFalse);
    BOOST_CHECK(!Test());
    Test.connect(&ReturnTrue);
    BOOST_CHECK(!Test());
    Test.disconnect(&ReturnFalse);
    BOOST_CHECK(Test());
    Test.disconnect(&ReturnTrue);
    BOOST_CHECK(Test());
}
BOOST_AUTO_TEST_SUITE_END()
