#!/usr/bin/env python3

#
# Copyright (C) 2019 Inria
#               2019 FU Berlin
#
# This file is subject to the terms and conditions of the GNU Lesser
# General Public License v2.1. See the file LICENSE in the top level
# directory for more details.
#

import argparse
import json
import os
import uuid

from collections import OrderedDict


def str2int(x):
    if x.startswith("0x"):
        return int(x, 16)
    else:
        return int(x)


def parse_arguments():
    parser = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('--urlroot', '-u', help='',
                        default="coap://example.org")
    parser.add_argument('--seqnr', '-s', default=0,
                        help='Sequence number of the manifest')
    parser.add_argument('--output', '-o', default="out.json",
                        help='Manifest output binary file path')
    parser.add_argument('--uuid-vendor', '-V', default="riot-os.org",
                        help='Manifest vendor uuid')
    parser.add_argument('--uuid-class', '-C', default="native64",
                        help='Manifest class uuid')
    parser.add_argument('slotfiles', nargs="+",
                        help='The list of slot file paths')
    parser.add_argument('--ecies', '-ec', metavar='FILE', type=argparse.FileType('r'), help='ECIES info file path')
    parser.add_argument('--manifest', '-mf', metavar='FILE', type=argparse.FileType('r'), help='manifest metadata file path')
    return parser.parse_args()


def main(args):
    uuid_vendor = uuid.uuid5(uuid.NAMESPACE_DNS, args.uuid_vendor)
    uuid_class = uuid.uuid5(uuid_vendor, args.uuid_class)
    template = {}

    template["manifest-version"] = int(1)


    if args.manifest:
        data = {}
        with args.manifest as file:
            for line in file:
                if '=' in line:
                    key, value = line.split('=', 1)
                    data[key.strip()] = value.strip()
        template["manifest-sequence-number"] = data["SEQUENCE_NUMBER"]
    else:
        template["manifest-sequence-number"] = int(args.seqnr)


    images = []
    for filename_offset in args.slotfiles:
        comp_name = ["00"]
        split = filename_offset.split(":", maxsplit=2)
        if len(split) == 1:
            filename, offset = split[0], 0
        elif len(split) == 2:
            filename, offset = split[0], str2int(split[1])
        else:
            filename, offset, comp_name = split[0], str2int(split[1]), split[2].split(":")

        images.append((filename, offset, comp_name))

    template["components"] = []

    for slot, image in enumerate(images):
        filename, offset, comp_name = image

        uri = os.path.join(args.urlroot, os.path.basename(filename))

        component = {
            "install-id": comp_name,
            "vendor-id": uuid_vendor.hex,
            "class-id": uuid_class.hex,
            "file": filename,
            "uri": uri,
            "bootable": False,
        }

        if args.ecies:
            m = json.loads(args.ecies.read(), object_pairs_hook=OrderedDict)
            component.update({
                "session-key": m['session-key'],
                "ephemeral-public-key": os.path.join(args.urlroot, m['ephemeral-public-key']),
                "salt": m['salt']
            })

        if offset:
            component.update({"offset": offset})

        template["components"].append(component)

    with open(args.output, 'w') as f:
        json.dump(template, f, indent=4)


if __name__ == "__main__":
    _args = parse_arguments()
    main(_args)
