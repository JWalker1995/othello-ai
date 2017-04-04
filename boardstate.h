#ifndef BOARDSTATE_H
#define BOARDSTATE_H

#include "bitboard8x8.h"
#include "bitboard8x8util.h"

// I use a bit-board for the implementation
// https://en.wikipedia.org/wiki/Bitboard

class BoardState {
public:
    void init() {
        self =  0x0000001008000000;
        enemy = 0x0000000810000000;
    }

    static BoardState make_flag() {
        BoardState res;
        res.self = 1;
        return res;
    }

    bool is_flag() const {
        return self == 1;
    }

    void assert_valid() const {
        assert(!(self & enemy));
        assert(!(self & ~board_mask));
        assert(!(enemy & ~board_mask));
    }

    signed int calc_piece_delta() const {
        signed int score = 0;
        score += BitBoard8x8Util::calc_population(self);
        score -= BitBoard8x8Util::calc_population(enemy);
        return score;
    }

    BitBoard8x8 calc_playable_cells() const {
        return BitBoard8x8Util::dilate_8(enemy) & board_mask & ~(self | enemy);
    }

    BoardState calc_branch(unsigned int move) const {
        assert((self & move) == 0);
        assert((enemy & move) == 0);

        // Copy board
        BoardState res;
        res.self = self;
        res.enemy = enemy;

        // Play piece
        res.self |= static_cast<BitBoard8x8>(1) << move;

        // We can only play where we actually flip pieces, so we need to keep track
        bool flipped = false;

        // Flip enemy pieces in each direction
        flipped |= res.flip_in_ray_towards_lsb<9>(move);
        flipped |= res.flip_in_ray_towards_lsb<8>(move);
        flipped |= res.flip_in_ray_towards_lsb<7>(move);
        flipped |= res.flip_in_ray_towards_lsb<1>(move);
        flipped |= res.flip_in_ray_towards_msb<1>(move);
        flipped |= res.flip_in_ray_towards_msb<7>(move);
        flipped |= res.flip_in_ray_towards_msb<8>(move);
        flipped |= res.flip_in_ray_towards_msb<9>(move);

        if (!flipped) {
            return BoardState::make_flag();
        }

        // Return new board state
        return res;
    }

    float calc_score() const {
        unsigned int self_pop = BitBoard8x8Util::calc_population(self);
        unsigned int enemy_pop = BitBoard8x8Util::calc_population(enemy);
        float empty_ratio = (36 - self_pop - enemy_pop) / 36.0f;

        float score = 0.0f;

        // Add the piece delta to the score
        score += self_pop;
        score -= enemy_pop;

        // Edge pieces are worth 3x a normal piece,
        // But as we get closer to the end of the game, we become more focused on the raw piece count
        score += BitBoard8x8Util::calc_population(self & edge_mask) * 2 * empty_ratio;
        score -= BitBoard8x8Util::calc_population(enemy & edge_mask) * 2 * empty_ratio;

        // Corner pieces are worth 7x a normal piece,
        // But as we get closer to the end of the game, we become more focused on the raw piece count
        score += BitBoard8x8Util::calc_population(self & corner_mask) * 4 * empty_ratio;
        score -= BitBoard8x8Util::calc_population(enemy & corner_mask) * 4 * empty_ratio;

        return score;
    }

    void swap_players() {
        std::swap(self, enemy);
    }

    static unsigned int get_index(unsigned int x, unsigned int y) {
        assert(x < 6);
        assert(y < 6);
        return (x + 1) + (y + 1) * 8;
    }
    static unsigned int get_x(unsigned int index) {
        return (index % 8) - 1;
    }
    static unsigned int get_y(unsigned int index) {
        return (index / 8) - 1;
    }

    std::string to_string(char self_char, char enemy_char) const {
        assert_valid();

        std::string res;
        res += "  0 1 2 3 4 5\n";
        for (unsigned int i = 1; i < 7; i++) {
            res += std::to_string(i - 1);
            for (unsigned int j = 1; j < 7; j++) {
                unsigned int index = get_index(j - 1, i - 1);

                res += ' ';
                if (BitBoard8x8Util::get_bit(self, index)) {
                    res += self_char;
                } else if (BitBoard8x8Util::get_bit(enemy, index)) {
                    res += enemy_char;
                } else {
                    res += '.';
                }
            }
            res += '\n';
        }
        return res;
    }

private:
    BitBoard8x8 self;
    BitBoard8x8 enemy;

    template <unsigned int dir_shift>
    bool flip_in_ray_towards_lsb(unsigned int play) {
        // Create ray template

        static constexpr BitBoard8x8 ray_template_1 = static_cast<BitBoard8x8>(1) << (64 - dir_shift);
        // X . . . . . . . . .

        static constexpr BitBoard8x8 ray_template_2 = ray_template_1 | (ray_template_1 >> (dir_shift * 1));
        // X X . . . . . . . .

        static constexpr BitBoard8x8 ray_template_3 = ray_template_2 | (ray_template_2 >> (dir_shift * 2));
        // X X X X . . . . . .

        static constexpr BitBoard8x8 ray_template_4 = ray_template_3 | (ray_template_3 >> (dir_shift * 4));
        // X X X X X X X X . .

        // Create actual ray
        BitBoard8x8 ray = ray_template_4 >> (64 - play);

        // Find intersection of ray and our pieces
        BitBoard8x8 intersect = ray & self;

        // Dilate that intersection by 7 cells to the left
        intersect |= intersect >> (dir_shift * 1);
        intersect |= intersect >> (dir_shift * 2);
        intersect |= intersect >> (dir_shift * 4);

        // Compute the range of cells to flip by removing the dilation
        BitBoard8x8 flip = ray & ~intersect;

        // If all of those cells are enemy, flip them
        if (flip && (flip & ~enemy) == 0) {
            self ^= flip;
            enemy ^= flip;
            return true;
        } else {
            return false;
        }
    }

    template <unsigned int dir_shift>
    bool flip_in_ray_towards_msb(unsigned int play) {
        // Create ray template

        static constexpr BitBoard8x8 ray_template_1 = static_cast<BitBoard8x8>(1) << dir_shift;
        // . . . . . . . . X .

        static constexpr BitBoard8x8 ray_template_2 = ray_template_1 | (ray_template_1 << (dir_shift * 1));
        // . . . . . . . X X .

        static constexpr BitBoard8x8 ray_template_3 = ray_template_2 | (ray_template_2 << (dir_shift * 2));
        // . . . . . X X X X .

        static constexpr BitBoard8x8 ray_template_4 = ray_template_3 | (ray_template_3 << (dir_shift * 4));
        // . X X X X X X X X .

        // Create actual ray
        BitBoard8x8 ray = ray_template_4 << play;

        // Find intersection of ray and our pieces
        BitBoard8x8 intersect = ray & self;

        // Dilate that intersection by 7 cells to the left
        intersect |= intersect << (dir_shift * 1);
        intersect |= intersect << (dir_shift * 2);
        intersect |= intersect << (dir_shift * 4);

        // Compute the range of cells to flip by removing the dilation
        BitBoard8x8 flip = ray & ~intersect;

        // If all of those cells are enemy, flip them
        if (flip && (flip & ~enemy) == 0) {
            self ^= flip;
            enemy ^= flip;
            return true;
        } else {
            return false;
        }
    }

    static constexpr BitBoard8x8 board_mask =   0x007E7E7E7E7E7E00u;
    static constexpr BitBoard8x8 edge_mask =    0x007E424242427E00u;
    static constexpr BitBoard8x8 corner_mask =  0x0042000000004200u;
};

#endif // BOARDSTATE_H
