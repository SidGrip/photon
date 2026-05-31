#!/usr/bin/env python3
"""
send-big-pho.py - Photon (PHO) big-send helper for 0.25.2.

Photon 0.25.2 enforces MAX_MONEY = 10,000,000,000 PHO as a per-output AND
per-transaction-sum consensus ceiling, so a single transaction cannot move more
than that. To pay one address more than the cap, this splits the payment into
several transactions, each well under the cap (default 9,000,000,000 PHO), using
the wallet's own sendtoaddress (which handles coin selection, change and fee).

Design notes:
  * Amounts are handled as fixed-point 8-decimal STRINGS, never floats, so large
    values keep full satoshi precision (a float cannot represent e.g.
    653945377.70662856 exactly).
  * Each sent chunk is appended to a per-(address,total) log file, and a re-run
    subtracts what was already sent, so an interrupted run resumes without
    re-sending confirmed chunks.
  * Chunks are <= 9,000,000,000 PHO, far below INT64_MAX/2, so the wallet's
    change-target math cannot overflow.

Requirements: python3 and `pip install requests`. Run on the same host as
photond. RPC credentials are read from <datadir>/photon.conf
(rpcuser/rpcpassword) or the <datadir>/.cookie file. Default datadir ~/.photon.

Usage:
    python3 send-big-pho.py <address> <total_PHO> [--chunk 9000000000]
                            [--datadir DIR] [--delay 2] [--yes]

Caveat: if a transaction is broadcast but the script is killed before it writes
the log, a re-run could re-send that one chunk. After an interrupted run, check
the log file and your wallet (listtransactions) before re-running.
"""
import argparse
import json
import os
import sys
import time
from decimal import Decimal, ROUND_DOWN, InvalidOperation

SAT = Decimal("0.00000001")
CAP = Decimal("10000000000")           # Photon 0.25.2 consensus MAX_MONEY (PHO)
DEFAULT_CHUNK = Decimal("9000000000")  # safe margin under the cap
DEFAULT_RPCPORT = "8984"               # Photon mainnet RPC port


def fixed8(d):
    return f"{d.quantize(SAT, rounding=ROUND_DOWN):.8f}"


def load_auth(datadir):
    d = os.path.expanduser(datadir or "~/.photon")
    user = pw = port = None
    conf = os.path.join(d, "photon.conf")
    if os.path.exists(conf):
        with open(conf) as f:
            for line in f:
                line = line.strip()
                if not line or line.startswith("#") or "=" not in line:
                    continue
                k, v = (x.strip() for x in line.split("=", 1))
                if k == "rpcuser":
                    user = v
                elif k == "rpcpassword":
                    pw = v
                elif k == "rpcport":
                    port = v
    if not (user and pw):
        cookie = os.path.join(d, ".cookie")
        if os.path.exists(cookie):
            user, pw = open(cookie).read().strip().split(":", 1)
    if not (user and pw):
        sys.exit(f"could not find RPC credentials in {conf} or {os.path.join(d, '.cookie')}")
    return user, pw, port or DEFAULT_RPCPORT


def rpc(session, url, auth, method, params=None):
    payload = {"jsonrpc": "1.0", "id": "send-big-pho", "method": method, "params": params or []}
    r = session.post(url, auth=auth, data=json.dumps(payload), timeout=120)
    try:
        out = r.json()
    except ValueError:
        sys.exit(f"non-JSON RPC reply ({r.status_code}): {r.text[:200]}")
    if out.get("error"):
        sys.exit(f"RPC {method} failed: {out['error']}")
    return out["result"]


def main():
    ap = argparse.ArgumentParser(
        description="Photon big-send helper: split a large payment into <=cap transactions.")
    ap.add_argument("address", help="destination PHO address")
    ap.add_argument("amount", help="total PHO to send (may exceed the 10,000,000,000 per-tx cap)")
    ap.add_argument("--chunk", default=str(DEFAULT_CHUNK), help="max PHO per transaction (default 9000000000)")
    ap.add_argument("--datadir", default=None, help="Photon datadir (default ~/.photon)")
    ap.add_argument("--delay", type=int, default=2, help="seconds to wait between transactions")
    ap.add_argument("--yes", action="store_true", help="do not prompt for confirmation")
    a = ap.parse_args()

    try:
        import requests
    except ImportError:
        sys.exit("missing dependency: pip install requests")

    try:
        target = Decimal(a.amount)
        chunk = Decimal(a.chunk)
    except InvalidOperation:
        sys.exit("amount and --chunk must be decimal numbers")
    if target <= 0 or chunk <= 0:
        sys.exit("amount and --chunk must be > 0")
    if chunk >= CAP:
        sys.exit(f"--chunk must be < {fixed8(CAP)} (the consensus cap); default is {fixed8(DEFAULT_CHUNK)}")

    user, pw, port = load_auth(a.datadir)
    url = f"http://127.0.0.1:{port}"
    auth = (user, pw)
    s = requests.Session()

    info = rpc(s, url, auth, "validateaddress", [a.address])
    if not info.get("isvalid"):
        sys.exit(f"invalid PHO address: {a.address}")

    # progress log keyed by destination + target so a re-run resumes (no double-send)
    safe_addr = "".join(c if c.isalnum() else "_" for c in a.address)
    log_path = os.path.abspath(f"bigsend_{safe_addr}_{fixed8(target)}.json")
    sent = json.load(open(log_path)) if os.path.exists(log_path) else []
    already = sum((Decimal(x["amount"]) for x in sent), Decimal(0))
    remaining = target - already
    if remaining <= 0:
        print(f"Target {fixed8(target)} PHO already sent to {a.address} "
              f"({len(sent)} tx, log {log_path}).")
        return

    bal = Decimal(str(rpc(s, url, auth, "getbalance")))
    if bal < remaining:
        sys.exit(f"insufficient balance: have {fixed8(bal)} PHO, need ~{fixed8(remaining)} (+fees)")

    n = int((remaining + chunk - SAT) // chunk)
    print(f"Send {fixed8(remaining)} PHO to {a.address} in {n} transaction(s) "
          f"of <= {fixed8(chunk)} PHO each.")
    if already > 0:
        print(f"Resuming: {fixed8(already)} PHO already sent in {len(sent)} tx.")
    if not a.yes and input("Proceed? [y/N] ").strip().lower() not in ("y", "yes"):
        sys.exit("aborted")

    while remaining > 0:
        this = min(remaining, chunk)
        txid = rpc(s, url, auth, "sendtoaddress", [a.address, fixed8(this)])
        print(f"  sent {fixed8(this)} PHO  ->  {txid}")
        sent.append({"txid": txid, "amount": fixed8(this)})
        with open(log_path, "w") as f:
            json.dump(sent, f, indent=2)
        remaining -= this
        if remaining > 0:
            time.sleep(a.delay)

    print(f"Done: {len(sent)} transaction(s) total. Log: {log_path}")


if __name__ == "__main__":
    main()
