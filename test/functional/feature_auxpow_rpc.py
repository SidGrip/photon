#!/usr/bin/env python3
# Copyright (c) 2026 The Blakecoin Developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""Test Photon AuxPoW mining RPCs."""

from test_framework.auxpow import build_auxpow, solve_auxpow_submit
from test_framework.blocktools import COINBASE_MATURITY
from test_framework.test_framework import BitcoinTestFramework
from test_framework.util import (
    assert_equal,
    assert_raises_rpc_error,
    assert_greater_than,
)


class AuxpowRPCTest(BitcoinTestFramework):
    def add_options(self, parser):
        self.add_wallet_options(parser)

    def set_test_params(self):
        self.num_nodes = 1
        self.setup_clean_chain = True
        self.extra_args = [[
            "-testactivationheight=segwit@130",
            "-vbparams=taproot:0:999999999999:0",
        ]]

    def skip_test_if_missing_module(self):
        self.skip_if_no_wallet()

    def node_args(self, *args):
        return self.extra_args[0] + list(args)

    def assert_auxpow_version_fields(self, work):
        assert_equal(isinstance(work["version"], int), True)
        assert_equal(work["versionHex"], f"{work['version'] & 0xffffffff:08x}")
        assert_equal(work["version"] & 0x20000000, 0x20000000)
        assert_equal(work["version"] & (1 << 8), 1 << 8)
        assert_equal((work["version"] >> 16) & 0xff, 2)

    def assert_taproot_signaling_if_required(self, node, work):
        chaininfo = node.getblockchaininfo()
        if "softforks" in chaininfo:
            taproot = chaininfo["softforks"]["taproot"]
        else:
            taproot = node.getdeploymentinfo()["deployments"]["taproot"]
        bip9_status = taproot.get("bip9", {}).get("status") or taproot.get("status")
        if bip9_status in ("started", "locked_in"):
            assert_equal(work["version"] & (1 << 2), 1 << 2)

    def mine_auxpow_block(self, node, address):
        work = node.createauxblock(address)
        self.assert_auxpow_version_fields(work)
        self.assert_taproot_signaling_if_required(node, work)
        solve_auxpow_submit(node, work["hash"], node.submitauxblock)
        assert_equal(node.getbestblockhash(), work["hash"])
        return work

    def mine_auxpow_blocks(self, node, address, count):
        work = None
        for _ in range(count):
            work = self.mine_auxpow_block(node, address)
        return work

    def run_test(self):
        node = self.nodes[0]
        address = node.getnewaddress("", "legacy")

        self.log.info("createauxblock returns Photon AuxPoW work")
        start_height = node.getblockcount()
        work = node.createauxblock(address)
        assert_equal(work["chainid"], 2)
        assert_equal(work["height"], start_height + 1)
        assert_equal(work["previousblockhash"], node.getbestblockhash())
        assert_equal(len(work["hash"]), 64)
        assert_equal(len(work["target"]), 64)
        self.assert_auxpow_version_fields(work)
        self.assert_taproot_signaling_if_required(node, work)

        self.log.info("getauxblock without configured payout address is rejected")
        assert_raises_rpc_error(
            -8,
            "getauxblock without parameters requires -auxpowmineraddress",
            node.getauxblock,
        )

        self.log.info("getauxblock no-arg mode uses -auxpowmineraddress")
        self.restart_node(0, extra_args=self.node_args(f"-auxpowmineraddress={address}"))
        node = self.nodes[0]
        work = node.getauxblock()
        assert_equal(work["chainid"], 2)
        assert_equal(work["height"], start_height + 1)
        self.assert_auxpow_version_fields(work)
        self.assert_taproot_signaling_if_required(node, work)

        self.log.info("malformed AuxPoW payload is rejected")
        assert_raises_rpc_error(-22, "AuxPoW decode failed", node.submitauxblock, work["hash"], "00")
        assert_raises_rpc_error(-22, "trailing data", node.submitauxblock, work["hash"], build_auxpow(work["hash"], 0) + "00")
        assert_raises_rpc_error(-8, "block hash unknown", node.submitauxblock, "0" * 64, build_auxpow(work["hash"], 0))

        self.log.info("submitauxblock accepts a valid regtest AuxPoW payload")
        accepted = False
        solved_auxpow = None
        for nonce in range(128):
            auxpow = build_auxpow(work["hash"], nonce)
            if node.submitauxblock(work["hash"], auxpow):
                accepted = True
                solved_auxpow = auxpow
                break
        assert_equal(accepted, True)
        assert_equal(node.getblockcount(), start_height + 1)
        assert_equal(node.getbestblockhash(), work["hash"])

        self.log.info("getauxblock submit mode reports duplicates as false")
        assert_equal(node.getauxblock(work["hash"], solved_auxpow), False)

        self.log.info("restart preserves the AuxPoW block from blk*.dat")
        self.restart_node(0, extra_args=self.node_args())
        node = self.nodes[0]
        assert_equal(node.getblockcount(), start_height + 1)
        assert_equal(node.getbestblockhash(), work["hash"])
        block = node.getblock(work["hash"])
        assert_equal(block["hash"], work["hash"])
        assert_equal(block["height"], start_height + 1)
        assert_equal(block["confirmations"], 1)

        self.log.info("-reindex reloads the AuxPoW block from blk*.dat")
        self.restart_node(0, extra_args=self.node_args("-reindex"))
        node = self.nodes[0]
        assert_equal(node.getblockcount(), start_height + 1)
        assert_equal(node.getbestblockhash(), work["hash"])
        block = node.getblock(work["hash"])
        assert_equal(block["hash"], work["hash"])
        assert_equal(block["height"], start_height + 1)

        self.log.info("AuxPoW templates before SegWit activation do not mine witness spends")
        pre_segwit_work = self.mine_auxpow_block(node, address)
        pre_segwit_block = node.getblock(pre_segwit_work["hash"], 2)
        assert_equal(pre_segwit_work["height"] < 130, True)
        # Bitcoin Core 0.25-style block assembly may include the reserved
        # witness commitment output before SegWit is active. The consensus
        # safety property here is that no non-coinbase witness spend is mined
        # through the AuxPoW RPC path before activation.
        assert_equal(
            any("txinwitness" in vin for tx in pre_segwit_block["tx"][1:] for vin in tx["vin"]),
            False,
        )

        self.log.info("Mine mature AuxPoW balance")
        self.mine_auxpow_blocks(node, address, COINBASE_MATURITY + 1)
        assert_greater_than(node.getbalance(), 0)

        self.log.info("Fund a native witness output before SegWit activation")
        witness_address = node.getnewaddress("", "bech32")
        fund_txid = node.sendtoaddress(witness_address, 1)
        fund_work = self.mine_auxpow_block(node, address)
        assert_equal(fund_work["height"] < 130, True)
        assert_equal(fund_txid in node.getblock(fund_work["hash"])["tx"], True)

        self.log.info("Advance to the last pre-SegWit block")
        while node.getblockcount() < 129:
            self.mine_auxpow_block(node, address)
        assert_equal(node.getblockcount(), 129)

        self.log.info("Spend the witness output and mine it through the AuxPoW RPC path")
        witness_utxos = node.listunspent(1, 9999999, [witness_address])
        assert_equal(len(witness_utxos), 1)
        witness_outpoint = {"txid": witness_utxos[0]["txid"], "vout": witness_utxos[0]["vout"]}
        other_outpoints = [
            {"txid": utxo["txid"], "vout": utxo["vout"]}
            for utxo in node.listunspent()
            if {"txid": utxo["txid"], "vout": utxo["vout"]} != witness_outpoint
        ]
        node.lockunspent(False, other_outpoints)
        spend_txid = node.sendtoaddress(address, 0.5)
        node.lockunspent(True, other_outpoints)
        assert_equal(spend_txid in node.getrawmempool(), True)

        segwit_work = self.mine_auxpow_block(node, address)
        assert_equal(segwit_work["height"], 130)
        segwit_block = node.getblock(segwit_work["hash"], 2)
        block_txids = [tx["txid"] for tx in segwit_block["tx"]]
        assert_equal(spend_txid in block_txids, True)
        witness_spend = next(tx for tx in segwit_block["tx"] if tx["txid"] == spend_txid)
        assert_equal("txinwitness" in witness_spend["vin"][0], True)
        assert_equal(
            any(
                vout["scriptPubKey"]["hex"].startswith("6a24aa21a9ed")
                for vout in segwit_block["tx"][0]["vout"]
            ),
            True,
        )


if __name__ == "__main__":
    AuxpowRPCTest().main()
