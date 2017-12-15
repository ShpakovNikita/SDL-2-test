#include <iostream>
#include <vector>
#include "memory"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "engine.hxx"

namespace inst {

class instance {
   public:
    virtual int render_instance() = 0;
    instance(std::vector<float>);
    virtual ~instance();

    glm::vec3 position;

   private:
    std::vector<float> data;
};

instance* create_player(std::vector<float>,
                        std::unique_ptr<CHL::engine, void (*)(CHL::engine*)>&);
void destroy_player(instance*);
}    // namespace inst
