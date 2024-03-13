#include "protocol.hpp"

// It would be interesting to generate this function with a generative macro
std::string operation_to_string(packet::operation op)
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
