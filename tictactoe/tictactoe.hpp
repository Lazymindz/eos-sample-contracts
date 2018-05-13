/**
* @file
* @copyright - MIT
*/

// Import all the necessary libraries
#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>

using namespace eosio;
namespace tictactoe {
  static const account_name games_account = N(games);
  static const account_name code_account = N(tictactoe);

  /**
  * @brief Data Structure to hold the information on game
  */
  static const uint32_t board_len = 9;

  struct game {
    game() {}
    game(account_name challenger, account_name host) :
    challenger(challenger),
    host(host),
    turn(host) {
      // initialise a board
      initialise_board();
    }
    account_name challenger;
    account_name host;
    account_name turn; // will be either of host, challenger;
    account_name winner = N(none); //none/draw/host/challenger will be the values
    uint8_t board[9];

    //initialise board with empty values
    void initialise_board() {
      for (uint8_t i=0; i < board_len; i++) {
        board[i] = 0;
      }
    }

    //reset a game
    void reset_game(){
      initialise_board();
      turn = host;
      winner = N(none);
    }

    auto primary_key() const { return challenger;}

    EOSLIB_SERIALIZE(game, (challenger)(host)(turn)(winner)(board))
  };

  /**
  * @brief Action to create a new game
  */
  struct create {
    account_name challenger;
    account_name host;

    EOSLIB_SERIALIZE(create, (challenger)(host))
  };

  /**
  * @brief Action to create a restart a game
  */
  struct restart {
    account_name challenger;
    account_name host;
    account_name by;

    EOSLIB_SERIALIZE(restart, (challenger)(host)(by))
  };

  /**
  * @brief Action to close a game
  */
  struct close {
    account_name challenger;
    account_name host;

    EOSLIB_SERIALIZE(close, (challenger)(host))
  };

  /**
  * @brief strucutre for placing a movement step in the game
  */
  struct movement {
    uint32_t row;
    uint32_t column;

    EOSLIB_SERIALIZE(movement, (row)(column))
  };

  /**
  * @brief Action to make a move in the game
  */
  struct move{
    account_name challenger;
    account_name host;
    account_name by; // details of the account that is making the movement
    movement mvt;

    EOSLIB_SERIALIZE(move, (challenger)(host)(by)(mvt))
  };

  /**
  * @brief table definition, to store games and their state
  */
  typedef eosio::multi_index<games_account, game> games;

}
