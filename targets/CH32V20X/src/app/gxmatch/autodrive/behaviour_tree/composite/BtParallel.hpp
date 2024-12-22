#include "BtComposite.hpp"

namespace ymd::btree{

class BtParallel: public BtComposite {
public:
    Execution tick() override;
};


}