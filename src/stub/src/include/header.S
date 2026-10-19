/*
;  header.S --
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) Markus Franz Xaver Johannes Oberhumer
;  Copyright (C) Laszlo Molnar
;  All Rights Reserved.
;
;  UPX and the UCL library are free software; you can redistribute them
;  and/or modify them under the terms of the GNU General Public License as
;  published by the Free Software Foundation; either version 2 of
;  the License, or (at your option) any later version.
;
;  This program is distributed in the hope that it will be useful,
;  but WITHOUT ANY WARRANTY; without even the implied warranty of
;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;  GNU General Public License for more details.
;
;  You should have received a copy of the GNU General Public License
;  along with this program; see the file COPYING.
;  If not, write to the Free Software Foundation, Inc.,
;  59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
;
;  Markus F.X.J. Oberhumer              Laszlo Molnar
;  <markus@oberhumer.com>               <ezerotven+github@gmail.com>
;
*/

// see PackHeader::putPackHeader()

section         UPX1HEAD

                .byte   0x91,0xa2,0xe4,0xb6 /* RREA: UPX_MAGIC_LE32, little endian. */
#if 0
                .byte   0x65,0xd8,0xc3,0x79 /* RREA: UPX_MAGIC2_LE32, little endian. */
#else
                // these will get set to actual values; initialized with some arbitrary numbers
                .byte   0x65 /* RREA: format tag placeholder, byte 0. */
                .byte   0xd8 /* RREA: format tag placeholder, byte 1. */
                .byte   0xc3 /* RREA: format tag placeholder, byte 2. */
                .byte   0x79 /* RREA: format tag placeholder, byte 3. */
#endif
                .long   0               //  8  uncompressed adler32
                .long   0               // 12  compressed adler32

#ifdef COM
                .short  0               // 16  uncompressed len
                .short  0               // 18  compressed len
                .byte   0               // 20  filter id
                .byte   131             // RREA: placeholder header checksum
#elif defined(EXE)
                .byte   0,0,0           // 16  uncompressed len
                .byte   0,0,0           // 19  compressed len
                .byte   0,0,0           // 22  original file size
                .byte   0               // 25  filter id
                .byte   131             // RREA: placeholder header checksum
#else
                .long   0               // 16  uncompressed len
                .long   0               // 20  compressed len
                .long   0               // 24  original file size
                .byte   0               // 28  filter id
                .byte   0               // 29  filter_cto
                .byte   0               // 30  filter_misc / n_mru
                .byte   131             // RREA: placeholder header checksum
#endif

/* vim:set ts=8 sw=8 et: */
