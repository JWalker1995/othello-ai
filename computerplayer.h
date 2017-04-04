#ifndef COMPUTERPLAYER_H
#define COMPUTERPLAYER_H

#include "boardstate.h"

class ComputerPlayer {
public:
    struct Move {
        unsigned int position;
        float score;
    };

    static bool has_move(BoardState board) {
        // Returns true if there is a possible move by the current player

        BitBoard8x8 playable_cells = board.calc_playable_cells();
        while (playable_cells) {
            unsigned int move = BitBoard8x8Util::pop_bit(playable_cells);
            if (!board.calc_branch(move).is_flag()) {
                return true;
            }
        }
        return false;
    }

    static unsigned int get_move_with_timeout(BoardState board, std::chrono::seconds timeout) {
        // Uses iterative deepening to search for a move, not deepening the search past timeout seconds

        std::chrono::high_resolution_clock::time_point timeout_point = std::chrono::high_resolution_clock::now() + timeout;

        for (unsigned int depth = 2; true; depth++) {
            std::cout << "Calculating move with depth " << depth << "... " << std::flush;

            Move move = get_move(board, depth);
            std::cout << BoardState::get_x(move.position) << "," << BoardState::get_y(move.position) << " = " << move.score << std::endl;

            if (depth >= 20 || std::chrono::high_resolution_clock::now() > timeout_point) {
                return move.position;
            }
        }
    }

    static Move get_move(BoardState board, unsigned int depth) {
        // Root search node
        // Returns the move position

        assert(depth > 0);
        depth--;

        Move best;
        best.position = 0;
        best.score = -1.0e9f;

        BitBoard8x8 playable_cells = board.calc_playable_cells();
        assert(playable_cells);

        do {
            unsigned int move = BitBoard8x8Util::pop_bit(playable_cells);
            BoardState new_board = board.calc_branch(move);
            if (new_board.is_flag()) {
                continue;
            }

            float branch_score = get_enemy_score(new_board, depth);
            if (branch_score > best.score) {
                best.position = move;
                best.score = branch_score;
            }
        } while (playable_cells);

        return best;
    }

private:
    static float get_my_score(BoardState board, unsigned int depth) {
        // Non-root search node
        // Does not return the move position

#ifdef PAUSE_AT_NODE
        std::cout << board.to_string('s', 'e');
#endif
        if (depth == 0) {
#ifdef PAUSE_AT_NODE
            std::cout << "score = " << board.calc_score() << std::endl;
            char a; std::cin >> a;
#endif
            return board.calc_score();
        }

#ifdef PAUSE_AT_NODE
        std::cout << "branching..." << std::endl;
        char a; std::cin >> a;
#endif

        depth--;

        float max_score = -1.0e6f;

        BitBoard8x8 playable_cells = board.calc_playable_cells();
        if (!playable_cells) {
            // If there are no playable cells, the game is over and the score is whoever had the most pieces
            return board.calc_piece_delta() * 1.0e3f;
        }

        do {
            // Otherwise try each move
            unsigned int move = BitBoard8x8Util::pop_bit(playable_cells);
            BoardState new_board = board.calc_branch(move);
            if (new_board.is_flag()) {
                continue;
            }

            float branch_score = get_enemy_score(new_board, depth);
            if (branch_score > max_score) {
                max_score = branch_score;
            }
        } while (playable_cells);

#ifdef PAUSE_AT_NODE
        std::cout << "exiting branch..." << std::endl;
#endif

        return max_score;
    }

    static float get_enemy_score(BoardState board, unsigned int depth) {
        // The enemy score is calculated easily: By returning the negative score of the flipped board
        board.swap_players();
        return -get_my_score(board, depth);
    }
};

#endif // COMPUTERPLAYER_H
