#include "BtComposite.hpp"

namespace ymd::btree{

class BtSequence: public BtComposite {
public:
    Execution tick() override;
};


}