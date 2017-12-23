#include <iostream>

class instance {
   public:
    int render_instance();

   private:
};

instance* create_instance();
void destroy_instance(instance*);
