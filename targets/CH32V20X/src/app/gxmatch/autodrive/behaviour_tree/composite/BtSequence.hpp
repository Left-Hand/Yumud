#include "BtComposite.hpp"

namespace btree{

class BtSequence: public BtComposite {
public:
    Execution tick() override;
};


}