#include "BtComposite.hpp"

namespace btree{

class BtSelector : public BtComposite {
public:
    Execution tick() override;
};


}