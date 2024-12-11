#include "BtComposite.hpp"

namespace btree{

class BtParallel: public BtComposite {
public:
    Execution execute() override;
};


}