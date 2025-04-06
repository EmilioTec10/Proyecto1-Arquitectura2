#ifndef INTERCONNECT_H
#define INTERCONNECT_H

#include <queue>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include <cstdint>

class PE;
class Memory;

enum class MessageType {
    WRITE_MEM,
    READ_MEM,
    BROADCAST_INVALIDATE,
    INV_ACK,
    INV_COMPLETE,
    READ_RESP,
    WRITE_RESP
};

struct Message {
    MessageType type;
    int SRC;
    int DEST;
    int ADDR;
    int SIZE;
    int CACHE_LINE;
    uint32_t DATA;
    int NUM_OF_CACHE_LINES;
    int START_CACHE_LINE;
    int QoS;
};

class Interconnect {
public:
    Interconnect(bool useQoS = false);
    void enqueueMessage(const Message& msg);
    void processMessages();
    void attachMemory(Memory* mem);
    void registerPE(int id, PE* pe);

private:
    std::queue<Message> fifo_queue;

    struct QoSComparator {
        bool operator()(const std::pair<int, Message>& a, const std::pair<int, Message>& b) {
            return a.first < b.first;
        }
    };

    std::priority_queue<std::pair<int, Message>,
        std::vector<std::pair<int, Message>>,
        QoSComparator> qos_queue;

    std::mutex queue_mutex;
    std::condition_variable queue_cv;
    bool useQoS;

    Memory* memory;
    std::unordered_map<int, PE*> pe_map;

    Message getNextMessage();
    void handleMessage(const Message& msg);
};

#endif
