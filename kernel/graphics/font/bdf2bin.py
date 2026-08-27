#!/usr/bin/env python3
"""Convert an 8x16 BDF font into a 256*16 raw bitmap binary."""

from __future__ import annotations

import argparse
import sys


def parse_bdf(path: str) -> dict[int, bytes]:
    glyphs: dict[int, bytes] = {}
    encoding: int | None = None
    in_bitmap = False
    rows: list[int] = []

    with open(path, encoding="ascii", errors="replace") as f:
        for raw in f:
            line = raw.strip()
            if line.startswith("ENCODING "):
                encoding = int(line.split()[1])
                rows = []
                in_bitmap = False
            elif line == "BITMAP":
                in_bitmap = True
                rows = []
            elif line == "ENDCHAR":
                if encoding is not None and 0 <= encoding <= 255:
                    if len(rows) != 16:
                        raise SystemExit(
                            f"glyph {encoding}: expected 16 rows, got {len(rows)}"
                        )
                    glyphs[encoding] = bytes(rows)
                encoding = None
                in_bitmap = False
                rows = []
            elif in_bitmap:
                rows.append(int(line, 16) & 0xFF)
    return glyphs


def build_blob(glyphs: dict[int, bytes]) -> bytes:
    blob = bytearray(256 * 16)
    for code, data in glyphs.items():
        blob[code * 16 : code * 16 + 16] = data
    return bytes(blob)


def write_cpp(path: str, blob: bytes) -> None:
    lines = [
        '#include <cstdint>',
        '',
        'alignas(16) extern const uint8_t terminus_font[256 * 16] = {',
    ]
    for i in range(0, len(blob), 16):
        row = ', '.join(f'0x{b:02x}' for b in blob[i : i + 16])
        lines.append(f'    {row},')
    lines.append('};')
    lines.append('')
    with open(path, 'w', encoding='ascii') as f:
        f.write('\n'.join(lines))


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('bdf', help='input BDF path')
    parser.add_argument('out', help='output .bin path (4096 bytes)')
    parser.add_argument(
        '--cpp',
        help='also write a C++ array source (e.g. terminus_data.cpp)',
    )
    args = parser.parse_args()

    glyphs = parse_bdf(args.bdf)
    blob = build_blob(glyphs)
    if len(blob) != 4096:
        raise SystemExit(f'unexpected blob size: {len(blob)}')

    with open(args.out, 'wb') as f:
        f.write(blob)
    print(f'wrote {args.out}: {len(blob)} bytes, {len(glyphs)} glyphs in 0-255')

    if args.cpp:
        write_cpp(args.cpp, blob)
        print(f'wrote {args.cpp}')


if __name__ == '__main__':
    main()
