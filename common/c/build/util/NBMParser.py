#!/usr/bin/env python
######################################################################
##   @file        NBMParser.py
##   @defgroup    tools
##
##   Description: Simple parser to list information of NBM tiles. Please use
##                python version 2.6 or above.
##   (C) Copyright 2013 by TeleCommunications Systems, Inc.
##
##   The information contained herein is confidential, proprietary to
##   TeleCommunication Systems, Inc., and considered a trade secret as defined
##   in section 499C of the penal code of the State of California. Use of this
##   information by anyone other than authorized employees of TeleCommunication
##   Systems is granted only under a written non-disclosure agreement, expressly
##   prescribing the scope and manner of such use.
##
##
#####################################################################

# -*- coding: utf-8 -*-

import struct
import os
import sys

#### Basic types ####
def swp_bytes(X):
    "swap low and high byte of short type"
    return ((X & 0xFF) << 8) | (X >> 8)

class Half(object):
    """Half float.
    """
    def __init__(self, h):
        """Initialize a Half type.

        Arguments:
        - `h`: short int (2bytes) to represent an float.
        """
        str = struct.pack('I', self.__half_to_float(h))
        self._f32 = struct.unpack('f', str)[0]

    def __half_to_float(self, h):
        """
        """
        h = swp_bytes(h)
        s = int((h >> 15) & 0x00000001)    # sign
        e = int((h >> 10) & 0x0000001f)    # exponent
        f = int(h & 0x000003ff)            # fraction
        if e == 0:
            if f == 0:
                return int(s << 31)
            else:
                while not (f & 0x00000400):
                    f = f << 1
                    e -= 1
                e += 1
                f &= ~0x00000400
        elif e == 31:
            if f == 0:
                return int((s << 31) | 0x7f800000)
            else:
                return int((s << 31) | 0x7f800000 | (f << 13))

        e = e + (127 -15)
        f = f << 13
        return int((s << 31) | (e << 23) | f)

    def __str__(self):
        return "%.02f"%self._f32

class Half2(object):
    """2d point in x, y order
    """

    def __init__(self, x, y): ## Input x,y is little endian.
        """
        """
        self._x = Half(x)
        self._y = Half(y)

    def __str__(self):
        """
        """
        return "(%s, %s)"%(self._x, self._y)


###
class ChunkHeader(object):
    def __init__(self, tpl):
        """Chunk header.
        """
        self.name  = tpl[0] # name of chunk
        self.size  = tpl[1] # size,
        self.flags = tpl[2] # flags
        self.misc  = tpl[3] # misc, can be interpreted differently by chunks.

#### Chunks ####
class Chunk(object):
    """Default chunk.
    """

    def __init__(self):
        """
        Initialize of this chunk.
        """
        self._ok = False

    def start_parse(self, header, buffer, pos):
        """Start parse, children should overwrite this function.
        """
        print("""">>> This is default one, should be overwritten by children.\
 You can see this because this type parser is not implemented!\n<<<""")
        pass

    def ok(self):
        """Check if this chunk is parsed successfully.
        """
        return self._ok

class HeaderChunk(Chunk):
    """NBM Header Chunk
    """
    def __init__(self):
        Chunk.__init__(self)
        pass

    def start_parse(self, header, buffer, pos):
        high = header.misc >> 8
        low  = header.misc & 0xFF
        print("File Version\t: Major - %d, Minor - %d" %(high, low))
        fmt = struct.Struct("<2f")
        (x, y) = fmt.unpack_from(buffer, pos)
        print("Ref-Center\t: (%.04f, %.04f)"%( x, y))
        self._ok = True

class BitmapChunk(Chunk):
    def __init__(self):
        Chunk.__init__(self)
        pass

    def start_parse(self, header, buffer, pos):
        N = header.misc
        print("Texture Count:\t%04X\n"%(N))

        # Parse bitmaps.
        fmt = struct.Struct("<")
        for i in range(N):
            n = struct.unpack_from("<b", buffer, pos)[0]
            pos += 1

            fn = struct.unpack_from("<%ds"%n, buffer, pos)[0]
            pos += n

            s = struct.unpack_from("<I", buffer, pos)[0]
            pos += 4 + s

            print("Index: %02d, name(%d): %s, size: %d"%(i, n, fn, s))
        self._ok = True

class Material(object):
    def __init__(self):
        """
        """
        self._msg = "XXX: Not implemented."
        pass

    def parse(self, buffer, pos):
        """
        Parse an material, should be overwritten by child.
        Arguments:
        - `buffer`:
        - `pos`:
        """
        pass

    def __str__(self):
        """
        """
        return self._msg

class RadialPinMaterial(Material):
    def __init__(self):
        Material.__init__(self)
        pass

    def parse(self, buffer, pos):
        (ub, sb, px, py, bx, by, cic, coc, cow) = struct.unpack_from(
            "<HHbbbbIIb", buffer, pos)
        self._msg = "Radius Pin --- Unselected Bitmap: %04X, selected bitmap:\
 %04X, PinOffset: (%02X, %02X), Bubble Offset: (%02X, %02X), InteriorColor: %08X, Outline Color: %08X, OutlineWidth: %02X"%(
            ub, sb, px, py, bx, by, cic, coc, cow)

class PinMaterial(Material):
    def __init__(self):
        Material.__init__(self)
        pass

    def parse(self, buffer, pos):
        (ib, px, py, bx, by) = struct.unpack_from("<Hbbbb", buffer, pos)
        self._msg = "Pin Material --- IconBitMap: %04X (%s), PinPos: (%02X,\
%02X), BubblePos: (%02X, %02X)."%(
    ib, "BMAP" if ib & 0x8000 else "IMGS", px, py, bx, by)

class StandardMaterial(Material):
    def __init__(self):
        Material.__init__(self)
        pass

    def parse(self, buffer, pos):
        (color, texture) = struct.unpack_from("<IH", buffer, pos)
        self._msg ="Standard Texture Material: Color: %08X, Texture Index: %04X (%s)"%(
            color, texture, "BMAP" if texture & 0x8000 else "IMGS")
        pass

class ShieldMaterial(Material):
    def __init__(self):
        Material.__init__(self)
        pass

    def parse(self, buffer, pos):
        (mi, ib, xo, yo, ep) = struct.unpack_from("<2H2bB", buffer, pos)
        self._msg = "Shield Material: Material Index: %04X, IconBitMap: %04X, \
 PlaceMent: (%02x, %02x), Edge Padding: %02X"%(mi, ib, xo, yo, ep)

class ShieldMaterial2(Material):
    def __init__(self):
        Material.__init__(self)
        pass

    def parse(self, buffer, pos):
        (mi, ib, xo, yo, xg, yg) = struct.unpack_from("<2H2b2B", buffer, pos)
        self._msg = "Shield Material: Material Index: %04X, IconBitMap: %04X, \
 PlaceMent: (%02x, %02x), Gap(%02X, %02X)"%(mi, ib, xo, yo, xg, yg)

class OutlineMaterial(Material):
    def __init__(self):
        Material.__init__(self)
        pass

    def parse(self, buffer, pos):
        (ic, oc, ow) = struct.unpack_from("<IIB", buffer, pos)
        self._msg = "Outline Polygon Color: Interior color: %08X, Outline\
 Color: %08X, Outline Width: %02X"%(ic, oc, ow)

class FontMaterial(Material):
    def __init__(self):
        Material.__init__(self)
        pass

    def parse(self, buffer, pos):
        (c, oc, ff, fs, so, sm, sM, cs, ow) = struct.unpack_from("<2I7B", buffer, pos)
        self._msg = "Font Material: TextColor: %08X, OutlineColor: %08X, \
Font Family: %s, Font Style: %s, OptimalSize: %02X, MinimumSize: %02X, \
Maximum Size: %02X, CharacterSpacing: %02X, OutlineWidth: %02X"%(
    c, oc, "Serif" if ff == 1 else "Sans-Serif",
    "Bold" if fs == 1 else "Normal",
    so, sm, sM, cs, ow)

class PatternLineMaterial(Material):
    def __init__(self):
        Material.__init__(self)
        pass

    def parse(self, buffer, pos):
        (onc, offc, olc, lp, w) = struct.unpack_from("<4HB", buffer, pos)
        self._msg = "Pattern Line Material: BitOn Color: %08X, \
BitOff Color: %08X, Outline Color %08X, Line Pattern: %08X, LineWidth: %02X"%(
            onc, offc, olc, lp, w)

class MaterialChunk(Chunk):
    def __init__(self):
        Chunk.__init__(self)
        pass

    def start_parse(self, header, buffer, pos):
        N = header.misc
        print("Material Counts: \t%d"%N)
        fmt1 = struct.Struct("<bbH")
        for i in range(N):
            (t, s, bi) = fmt1.unpack_from(buffer, pos)
            pos += fmt1.size
            print("Index: %03d, type: %02X, size: %04X, backup_index: %04X"%
                  (i, t, s, bi))

            # Parse each material.
            m = {
                0x00 : StandardMaterial,
                0x02 : OutlineMaterial,
                0x04 : FontMaterial,
                0x05 : PatternLineMaterial,
                0x07 : ShieldMaterial,
                0x0B : PinMaterial,
                0x0C : OutlineMaterial,
                0x0D : OutlineMaterial,
                0x0E : ShieldMaterial2,
                0x0F : RadialPinMaterial
            }.get(t, Material)()
            m.parse(buffer, pos)
            print("\t%s"%m)
            pos += s
        self._ok = True

class LayersChunk(Chunk):
    def __init__(self):
        Chunk.__init__(self)
        pass

    def start_parse(self, header, buffer, pos):
        N = header.misc
        fmt = struct.Struct("<HffIb")
        print("Total Layers: %d,"%N)
        for i in range(N):
            (t, nv, fv, p, al) = fmt.unpack_from(buffer, pos)
            pos += fmt.size
            print("Layer type: %04X(%s),\tVisibility: (%.04F -- %.04F), ChunkFilePosition: %08X, Associate Layer: %02X"%(
                t, {
                    0:  "Associated Layer",
                    1:  "Road Network",
                    2:  "Area Polygon",
                    3:  "Un-Texture Building",
                    4:  "Un-Texture Landmark Building",
                    5:  "3D Landmark Building",
                    7:  "Radius Pin Layer"
                }.get(t, "Unknown"), nv, fv, p, al
            ))

class TextChunk(Chunk):
    def __init__(self):
        Chunk.__init__(self)
        pass

    def start_parse(self, header, buffer, pos):
        N = header.misc
        fmt = struct.Struct("<Hbb")
        print("Total Texts: %d,"%N)
        for i in range(N):
            (mi, hb, lb) = fmt.unpack_from(buffer, pos)
            pos += fmt.size
            total = hb * 256 + lb
            txt = struct.unpack_from("<%ds"%total, buffer, pos)
            pos += total
            print("(%02X): MaterialIndex: %04X, Length: %02X, String: %s"%(
                i, mi, total, txt))

class GPinChunk(Chunk):
    def __init__(self):
        Chunk.__init__(self)
        pass

    def start_parse(self, header, buffer, pos):
        N = header.misc
        fmt = struct.Struct("<5H")
        print("Total Points: %d,"%N)
        for i in range(N):
            (mi, x, y,pi, r) = fmt.unpack_from(buffer, pos)
            pos += fmt.size
            print("(%04X): MaterialIndex: %04X, Location: %s, \
 PinID: %04X, Radius: %04X"%(
     i, mi, Half2(x, y), pi, r))

#### Parser ####
class NBMParser(object):
    """Generic Header shared by all kinds of Chunks...
    """
    def __init__(self, path):
        """
        """
        self.__fmt = struct.Struct('<4sIHH')
        self.__path = path
        if not os.access(path, os.R_OK):
            print("File %s is not readable!\n", path)
            sys.exit(1)
        self.__contents = open(path, 'rb').read()

    def run(self):
        """Start parsing.
        """
        print("\n********** Parsing file: %s **********"%self.__path)
        length = len(self.__contents)
        pos = 0
        while pos < length:
            header = ChunkHeader(self.__fmt.unpack_from(self.__contents, pos))
            print("\n>>>>>>>>>> Parsing chunk: %s <<<<<<<<"%header.name)
            print("StartPos: %08X, ChunkSize: %08X, Flags: %08X\n" %(
                pos, header.size, header.flags))

            pos += self.__fmt.size
            chunk = {
                "NBMF" : HeaderChunk,
                "BMAP" : BitmapChunk,
                "MTRL" : MaterialChunk,
                "LAYS" : LayersChunk,
                "TEXT" : TextChunk,
                "GPIN" : GPinChunk
                # Add your own chunk parsers here.
            }.get(header.name, Chunk)()
            chunk.start_parse(header, self.__contents, pos)

            pos += header.size - 4
            pos += 0 if pos % 4 == 0 else 4 - pos % 4 # align to 4 bytes.

    def parse(self, buffer):
        """Start parsing..

        Arguments:
        - `buffer`:
        """
        pass

if __name__ == '__main__':
    if len(sys.argv) <= 1:
        print ("Usage: %s NBMFile\n" %sys.argv[0])
        sys.exit(1)

    for ele in sys.argv[1:]:
        parser = NBMParser(ele)
        parser.run()
