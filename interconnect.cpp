#include <iostream>
#include <queue>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <map>
#include <cstdint>
#include <atomic>

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

struct QoSComparator {
    bool operator()(const std::pair<int, Message>& a, const std::pair<int, Message>& b) {
        // Queremos que mayor prioridad (mayor valor) vaya primero
        return a.first < b.first;
    }
};


class Interconnect {
    public:
        Interconnect(bool useQoS = false) : useQoS(useQoS) {}
    
        void enqueueMessage(const Message& msg) {
            std::lock_guard<std::mutex> lock(queue_mutex);
            if (useQoS) {
                qos_queue.emplace(msg.QoS, msg);
            } else {
                fifo_queue.push(msg);
            }
            queue_cv.notify_one();
        }
    
        // Debe llamarse en un hilo que esté constantemente procesando mensajes
        void processMessages() {
            while (true) {
                Message msg = getNextMessage();
                handleMessage(msg);
            }
        }
    
    private:
        std::queue<Message> fifo_queue;
        std::priority_queue<std::pair<int, Message>, std::vector<std::pair<int, Message>>, QoSComparator> qos_queue;
        std::mutex queue_mutex;
        std::condition_variable queue_cv;
        bool useQoS;
    
        Message getNextMessage() {
            std::unique_lock<std::mutex> lock(queue_mutex);
            queue_cv.wait(lock, [this]() {
                return !fifo_queue.empty() || !qos_queue.empty();
            });
    
            if (useQoS && !qos_queue.empty()) {
                auto top = qos_queue.top();
                qos_queue.pop();
                return top.second;
            } else {
                Message msg = fifo_queue.front();
                fifo_queue.pop();
                return msg;
            }
        }
    
        void handleMessage(const Message& msg) {
            switch (msg.type) {
                case MessageType::WRITE_MEM:
                    std::cout << "[IC] WRITE_MEM from PE" << msg.SRC << " at address " << msg.ADDR << "\n";
                    break;
    
                case MessageType::READ_MEM:
                    std::cout << "[IC] READ_MEM from PE" << msg.SRC << " at address " << msg.ADDR << "\n";
                    break;
    
                case MessageType::BROADCAST_INVALIDATE:
                    std::cout << "[IC] BROADCAST_INVALIDATE from PE" << msg.SRC << ", line " << msg.CACHE_LINE << "\n";
                    break;
    
                case MessageType::INV_ACK:
                    std::cout << "[IC] INV_ACK from PE" << msg.SRC << "\n";
                    break;
    
                case MessageType::INV_COMPLETE:
                    std::cout << "[IC] INV_COMPLETE to PE" << msg.DEST << "\n";
                    break;
    
                case MessageType::READ_RESP:
                    std::cout << "[IC] READ_RESP to PE" << msg.DEST << "\n";
                    break;
    
                case MessageType::WRITE_RESP:
                    std::cout << "[IC] WRITE_RESP to PE" << msg.DEST << ", status = " << msg.DATA << "\n";
                    break;
            }
        }
};