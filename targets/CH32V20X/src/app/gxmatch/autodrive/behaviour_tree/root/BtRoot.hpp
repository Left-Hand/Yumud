#pragma once

#include "../../behaviour_tree/composite/BtComposite.hpp"

namespace ymd::btree{

class BtRoot:public BtComposite{
    BtRoot():BtComposite("root"){;}
public:

};

}