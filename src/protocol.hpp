#include <cstdio>
#include <string>
#include <xbt/log.h>

struct packet 
{
    enum operation
    { 
        SEND_GLOBAL_MODEL, 
        SEND_LOCAL_MODEL,
        KILL_TRAINER,
    } op;

    const std::string src;
};

// It would be interesting to generate this function with a generative macro
static const char* operation_to_str(packet::operation op)
{
    switch (op) {
        case packet::operation::SEND_GLOBAL_MODEL:
            return "SEND_GLOBAL_MODEL";
        case packet::operation::SEND_LOCAL_MODEL:
            return "SEND_LOCAL_MODEL";
        case packet::operation::KILL_TRAINER:
            return "KILL_TRAINER";
    }
}
