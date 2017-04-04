// Compilation:
// g++ -O4 -ffast-math -g -std=c++11 main.cpp -o proj_3

// Running:
// ./proj_3

#include <iostream>
#include <chrono>

#include "computerplayer.h"

int main() {
    enum class Player {Human, Computer};
    Player turn;

    std::cout << "Do you want to go first [y/n]? " << std::flush;
    std::string resp;
    std::cin >> resp;
    if (resp == "y" || resp == "Y") {
        turn = Player::Human;
    } else if (resp == "n" || resp == "N") {
        turn = Player::Computer;
    } else {
        std::cerr << "Invalid input" << std::endl;
        return 1;
    }

    std::cout << "You are X" << std::endl;

    BoardState board;
    board.init();

    std::chrono::seconds move_timeout(5);

    while (ComputerPlayer::has_move(board)) {
        unsigned int move;

        if (turn == Player::Human) {
            // If it's the player's turn to move

            std::cout << board.to_string('X', 'O') << std::endl;
            std::cout << "Enter your move in x,y notation: " << std::flush;

            std::string input;
            std::cin >> input;

            if (input.size() != 3 || input[0] < '0' || input[0] > '5' || input[1] != ',' || input[2] < '0' || input[2] > '5') {
                std::cerr << "Invalid input format" << std::endl;
                continue;
            }

            unsigned int x = input[0] - '0';
            unsigned int y = input[2] - '0';
            move = BoardState::get_index(x, y);
        } else if (turn == Player::Computer) {
            // If it's the computer's turn to move

            std::cout << board.to_string('O', 'X') << std::endl;
            move = ComputerPlayer::get_move_with_timeout(board, move_timeout);
        }

        if (!BitBoard8x8Util::get_bit(board.calc_playable_cells(), move)) {
            std::cerr << "Invalid move!" << std::endl;
            continue;
        }

        BoardState new_board = board.calc_branch(move);
        if (new_board.is_flag()) {
            std::cerr << "Invalid move!" << std::endl;
            continue;
        }

        board = new_board;
        board.swap_players();
        turn = (turn == Player::Human) ? Player::Computer : Player::Human;
    }

    if (turn == Player::Computer) {
        board.swap_players();
    }

    // Print board
    std::cout << board.to_string('X', 'O') << std::endl;

    signed int piece_delta = board.calc_piece_delta();
    if (piece_delta < 0) {
        std::cout << "You lost by " << -piece_delta << " pieces!" << std::endl;
    } else if (piece_delta > 0) {
        std::cout << "You won by " << piece_delta << " pieces!" << std::endl;
    } else {
        std::cout << "You tied!" << std::endl;
    }

    return 0;
}
