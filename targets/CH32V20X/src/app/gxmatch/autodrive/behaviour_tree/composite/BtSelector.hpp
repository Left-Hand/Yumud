#include "BtComposite.hpp"

namespace btree{

class BtSelector : public BtComposite {
public:
    Execution execute() override;
};


}