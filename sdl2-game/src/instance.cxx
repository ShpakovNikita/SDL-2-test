#include "headers/instance.hxx"

namespace inst {

instance::instance(std::vector<float> coords) {
    data = coords;
}

instance::~instance() {}

class player : public instance {
   private:
   public:
    player(std::vector<float> data,
           std::unique_ptr<CHL::engine, void (*)(CHL::engine*)>& eng)
        : instance(data) {}

    int render_instance() final { return 1; }
};

bool player_exist = false;

instance* create_player(
    std::vector<float> data,
    std::unique_ptr<CHL::engine, void (*)(CHL::engine*)>& e) {
    if (player_exist) {
        throw std::runtime_error("player already exist");
    }

    instance* inst = new player(data, e);
    player_exist = true;
    return inst;
}

void destroy_player(instance* i) {
    if (player_exist == false) {
        throw std::runtime_error("player is not created");
    }
    if (nullptr == i) {
        throw std::runtime_error("player is null");
    }
    delete i;
}

}    // namespace inst
