/*
  CrazyAra, a deep learning chess variant engine
  Copyright (C) 2018       Johannes Czech, Moritz Willig, Alena Beyer
  Copyright (C) 2019-2020  Johannes Czech

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License fåor more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

/*
 * @file: boardstate.h
 * Created on 13.07.2020
 * @author: queensgambit
 */

#include "boardstate.h"
#include "domain/crazyhouse/inputrepresentation.h"

BoardState::BoardState() : State(),
    states(StateListPtr(new std::deque<StateInfo>(0)))
{
}

BoardState::BoardState(const BoardState &b) :
    State(),
    board(b.board),
    states(StateListPtr(new std::deque<StateInfo>(0)))
{
}

vector<Action> BoardState::legal_actions() const
{
    vector<Action> legalMoves;
    // generate the legal moves and save them in the list
    for (const ExtMove& move : MoveList<LEGAL>(board)) {
        legalMoves.push_back(Action(move));
    }
    return legalMoves;
}

State &BoardState::set(const string &fenStr, bool isChess960, int variant)
{
    states = StateListPtr(new std::deque<StateInfo>(1));
    variant = UCI::variant_from_name(Options["UCI_Variant"]);
    board.set(fenStr, isChess960, Variant(variant), &states->back(), nullptr);
    return *this;
}

void BoardState::get_state_planes(bool normalize, float *inputPlanes) const
{
    board_to_planes(&board, board.number_repetitions(), normalize, inputPlanes);
}

unsigned int BoardState::steps_from_null() const
{
    return board.plies_from_null();
}

bool BoardState::is_chess960() const
{
    return board.is_chess960();
}

string BoardState::fen() const
{
    return board.fen();
}

void BoardState::do_action(Action action)
{
    states->emplace_back();
    board.do_move(Move(action), states->back());
}

unsigned int BoardState::number_repetitions() const
{
    return board.number_repetitions();
}

int BoardState::side_to_move() const
{
    return board.side_to_move();
}

Key BoardState::hash_key() const
{
    return board.hash_key();
}

void BoardState::flip()
{
    board.flip();
}

Action BoardState::uci_to_action(string& uciStr) const
{
    return Action(UCI::to_move(board, uciStr));
}

string BoardState::action_to_san(Action action, const vector<Action>& legalActions) const
{
    return pgn_move(Move(action), this->is_chess960(), board, legalActions);
}

TerminalType BoardState::is_terminal(size_t numberLegalMoves, bool inCheck) const
{
    if (numberLegalMoves == 0) {
#ifdef ANTI
        if (board.is_anti()) {
            // a stalmate is a win in antichess
            return TERMINAL_WIN;
        }
#endif
        // test if we have a check-mate
        if (inCheck) {
            return TERMINAL_LOSS;
        }
        // we reached a stalmate
        return TERMINAL_DRAW;
    }
#ifdef ANTI
    if (board.is_anti()) {
        if (board.is_anti_win()) {
            return TERMINAL_WIN;
        }
        if (board.is_anti_loss()) {
            return TERMINAL_LOSS;
        }
    }
#endif
    if (board.can_claim_3fold_repetition() || board.is_50_move_rule_draw() || board.draw_by_insufficient_material()) {
        // reached 3-fold-repetition or 50 moves rule draw or insufficient material
        return TERMINAL_DRAW;
    }

    // normal game position
    return TERMINAL_NONE;
}

bool BoardState::gives_check(Action action) const
{
    return board.gives_check(Move(action));
}

unique_ptr<State> BoardState::clone() const
{
    return make_unique<BoardState>(*this);
}

void BoardState::print(ostream &os) const
{
    os << board;
}
