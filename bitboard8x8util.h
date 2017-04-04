#ifndef BITBOARD8X8UTIL_H
#define BITBOARD8X8UTIL_H

#include <assert.h>

#include "bitboard8x8.h"

class BitBoard8x8Util {
public:
    static BitBoard8x8 dilate_8(BitBoard8x8 board) {
        // . . .
        // . X .
        // . . .

        // Shift board up 1 cell
        board |= board << 8;
        // . X .
        // . X .
        // . . .

        // Shift board right 1 cell
        board |= board >> 1;
        // . X X
        // . X X
        // . . .

        // Shift board down 1 cell
        board |= board >> 8;
        // . X X
        // . X X
        // . X X

        // Shift board left 1 cell
        board |= board << 1;
        // X X X
        // X X X
        // X X X

        return board;
    }

    static bool get_bit(BitBoard8x8 board, unsigned int pos) {
        return (board >> pos) & 1;
    }

    static unsigned int pop_bit(BitBoard8x8 &board) {
        assert(board != 0);

        unsigned int pos = __builtin_ctzll(board);
        board ^= static_cast<BitBoard8x8>(1) << pos;
        return pos;
    }

    static unsigned int calc_population(BitBoard8x8 board) {
        return __builtin_popcountll(board);
    }
};

#endif // BITBOARD8X8UTIL_H
