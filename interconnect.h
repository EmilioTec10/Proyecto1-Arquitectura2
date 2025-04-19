#ifndef INTERCONNECT_H
#define INTERCONNECT_H

#include <queue>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include <cstdint>
#include <map>
#include <atomic>
#include <iostream>
#include <fstream>
#include <chrono>
#include <fstream>

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
    uint32_t ADDR;
    int SIZE;
    int CACHE_LINE;
    uint32_t DATA;
    int NUM_OF_CACHE_LINES;
    int START_CACHE_LINE;
    int QoS;
};

struct InvalidationTracker {
    int expected_acks = 0;
    int received_acks = 0;
    int source_pe = -1; // PE que originó el broadcast
    int qos = 0;
};



extern std::map<int, InvalidationTracker> invalidation_map;
extern int next_inv_id;

extern std::unordered_map<std::string, std::chrono::steady_clock::time_point> enqueue_times;


class Interconnect {
public:
    Interconnect(bool useQoS = false);
    void enqueueMessage(const Message& msg);
    void processMessages();
    void attachMemory(Memory* mem);
    void registerPE(int id, PE* pe);
    void requestStop();   // para decirle que ya puede terminar
    void waitForFinish(); // para esperar a que termine naturalmente

private:
    std::queue<Message> fifo_queue;
    std::atomic<bool> stop_requested;

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
