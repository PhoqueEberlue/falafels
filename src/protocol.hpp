#include <string>

struct packet 
{
    enum operation
    { 
        SEND_GLOBAL_MODEL, 
        SEND_LOCAL_MODEL,
        KILL_TRAINER,
    } op;

    const std::string *src;
};

std::string operation_to_string(packet::operation op);
