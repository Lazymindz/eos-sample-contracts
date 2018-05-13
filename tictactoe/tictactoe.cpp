#include "tictactoe.hpp"

using namespace eosio;
namespace tictactoe{
  struct impl {

    /**
    * @param create - action to be applied
    */
    void on(const create& c){
      require_auth(c.host);
      eosio_assert(c.challenger != c.host, "Challenger shouldn't be same as host");

      //Check if the game exists already
      games existing_host_games(code_account, c.host);
      auto itr = existing_host_games.find(c.challenger);
      eosio_assert(itr==existing_host_games.end(),"game already exists");

      existing_host_games.emplace(c.host, [&](auto& g){
        g.challenger=c.challenger;
        g.host = c.host;
        g.turn = c.host;
      });
    }

    /**
    * @brief Apply restart action
    * @param restart - action to be applied
    */
    void on(const restart& r){
      require_auth(r.by);

      //check if game exists;
      games existing_host_games(code_account, r.host);
      auto itr = existing_host_games.find(r.challenger);
      eosio_assert(itr != existing_host_games.end(),"Game doesn't exist");

      //Check if the game belongs to action sender.
      eosio_assert(r.by == itr->host || r.by == itr->challenger,"Not your game!");

      //Reset game
      existing_host_games.modify(itr, itr->host, [](auto& g){
        g.reset_game();
      });
    }

    /**
    * @brief Apply close action
    * @param close - action to be applied
    */
    void on(const close& c){
      require_auth(c.host);

      //Check if game exists;
      games existing_host_games(code_account, c.host);
      auto itr = existing_host_games.find(c.challenger);
      eosio_assert(itr != existing_host_games.end(), "No Game to Close!");

      //Remove itr
      existing_host_games.erase(itr);
    }

    /**
    * @brief check if the cell is empty;
    * @param cell - value of the cell should be either 0, 1, or 2;
    * @return true if the cell is empty;
    */
    bool is_empty_cell(const uint8_t& cell){
      return cell == 0;
    }

    /**
    * @brief check if the movement id valid
    * @detail Movement is considered valid if done on a Cell that is 0 and within boundary;
    * @param mvt : It is the movement made by the player
    * @param game: Game on which the movement is carried out
    * @return : Return true is the movement is valid
    */
    bool is_valid_movement(const movement& mvt, const game& game_for_movement){
      uint32_t movement_location = mvt.row * 3 + mvt.column;
      bool is_valid = movement_location < board_len && is_empty_cell(game_for_movement.board[movement_location]);
      return is_valid;
    }

    /**
    * @brief Get Winner of the game
    * @detail winner is the one who made first aligned movement(3)
    * @param game for which winner needs to be determined
    * @returns Account name of the Winner or none or draw
    */
    account_name get_winner(const game& current_game){
      if((current_game.board[0] == current_game.board[4] && current_game.board[4] == current_game.board[8]) ||
      (current_game.board[1] == current_game.board[4] && current_game.board[4] == current_game.board[7]) ||
      (current_game.board[2] == current_game.board[4] && current_game.board[4] == current_game.board[6]) ||
      (current_game.board[3] == current_game.board[4] && current_game.board[4] == current_game.board[5])) {
      //  - | - | x    x | - | -    - | - | -    - | x | -
      //  - | x | -    - | x | -    x | x | x    - | x | -
      //  x | - | -    - | - | x    - | - | -    - | x | -
      if (current_game.board[4] == 1) {
         return current_game.host;
      } else if (current_game.board[4] == 2) {
         return current_game.challenger;
      }
   } else if ((current_game.board[0] == current_game.board[1] && current_game.board[1] == current_game.board[2]) ||
              (current_game.board[0] == current_game.board[3] && current_game.board[3] == current_game.board[6])) {
      //  x | x | x       x | - | -
      //  - | - | -       x | - | -
      //  - | - | -       x | - | -
      if (current_game.board[0] == 1) {
         return current_game.host;
      } else if (current_game.board[0] == 2) {
         return current_game.challenger;
      }
   } else if ((current_game.board[2] == current_game.board[5] && current_game.board[5] == current_game.board[8]) ||
              (current_game.board[6] == current_game.board[7] && current_game.board[7] == current_game.board[8])) {
      //  - | - | -       - | - | x
      //  - | - | -       - | - | x
      //  x | x | x       - | - | x
      if (current_game.board[8] == 1) {
         return current_game.host;
      } else if (current_game.board[8] == 2) {
         return current_game.challenger;
      }
      } else {
      bool is_board_full = true;
      for (uint8_t i = 0; i < board_len; i++) {
         if (is_empty_cell(current_game.board[i])) {
            is_board_full = false;
            break;
         }
      }
      if (is_board_full) {
         return N(draw);
      }
    }
      return N(none);
    }

    /**
    * @brief Apply move action
    * @param move - action to be applied
    */
    void on(const move& m){
      require_auth(m.by);

      //Check if the game exists;
      games existing_host_games(code_account, m.host);
      auto itr = existing_host_games.find(m.challenger);
      eosio_assert(itr != existing_host_games.end(), "No game exists!");

      //Check the game has not ended
      eosio_assert(itr->winner == N(none), "the game has ended");

      //Check the game belongs to the sender;
      eosio_assert(m.by == itr->host || m.by == itr->challenger, "Not your game");

      //Check if the turn of the player is correct
      eosio_assert(m.by == itr->turn, "Not your turn. Wait for the other player");

      //Check if the movement is valid;
      eosio_assert(is_valid_movement(m.mvt, *itr),"Not a valid movement!");

      //Fill the cell value; 1 for Host and 2 for challenger;
      const auto cell_value = itr->turn == itr->host ? 1 : 2;
      const auto turn = itr->turn == itr->host ? itr->challenger : itr->host;

      existing_host_games.modify(itr, itr->host, [&]( auto& g){
        g.board[m.mvt.row * 3 + m.mvt.column] = cell_value;
        g.turn = turn;
        //Check and update if there is a winner;
        g.winner = get_winner(g);
      });
    }

    /// The apply method implements the dispatch of events to this contract
    void apply(uint64_t receiver, uint64_t code, uint64_t action){
      if (code == code_account){
        if (action == N(create)){
          impl::on(eosio::unpack_action_data<tictactoe::create>());
        }
        else if (action == N(restart)){
          impl::on(eosio::unpack_action_data<tictactoe::restart>());
        }
        else if (action == N(close)){
          impl::on(eosio::unpack_action_data<tictactoe::close>());
        }
        else if (action == N(move)){
          impl::on(eosio::unpack_action_data<tictactoe::move>());
        }
      }
    }
  };
}

/**
* The apply() method muct have C calling convention for Blockchain to lookup and
* call these methods.
*/
extern "C" {

  using namespace tictactoe;
  /// The apply method implements the dispatch of events for this contract
  void apply( uint64_t receiver, uint64_t code, uint64_t action){
    impl().apply(receiver, code, action);
  }

} //extern C
