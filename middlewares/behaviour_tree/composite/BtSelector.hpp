#include "BtComposite.hpp"

namespace ymd::btree{

class BtSelector : public BtComposite {
public:
    Execution tick() override;
};


}