#include "../../../../dml/include/i_common.hpp"

class SCommon : ICommon
{
public:
    SCommon() {}
    ~SCommon() {};
    double get_time();
    void kill_processes();
};
