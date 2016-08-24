#! /usr/bin/env python

import sys
import traceback


def convert(f):
    """Convert mega_info_creator output into an address map."""
    map = {}
    for line in f:
        symbol = line.strip().split('\t')
        if len(symbol) != 3:
            continue

        _, name, addr = symbol
        if addr in map and name != map[addr]:
            sys.stderr.write("Collision at {}: {} != {}\n".format(
                addr, name, map[addr]
            ))
        else:
            map[addr] = name
    return map


if __name__ == "__main__":
    if len(sys.argv) > 1:
        files = sys.argv[1:]
    else:
        files = [sys.stdin]

    for f in files:
        try:
            if isinstance(f, str):
                with open(f, 'r') as fin:
                    map = convert(fin)
            else:
                map = convert(f)
            for addr, name in map.items():
                print("{}\t{}".format(addr, name))
        except:
            traceback.print_exc()
