#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/singleton.hpp>

using namespace eosio;
using namespace std;

class deep : public contract{
  using contract::contract;

  public:
    deep(account_name self) :
    contract(self){}

    void setapp(account_name application){
      require_auth(_self);
      require_auth(application);

      uint64_t fingerprint = 10;
      eosio_assert(!configs(application, fingerprint).exists(), "configuration exists already");
      configs(application, fingerprint).set(application);
    }

  private:
    typedef uint64_t id;

    struct config {
      account_name application;

      EOSLIB_SERIALIZE(config, (application))
    };

    typedef singleton<N(deep), config> configs;

};

EOSIO_ABI(deep,(setapp))
