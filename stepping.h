// stepping_control.hpp
#ifndef STEPPING
#define STEPPING
#include <atomic>
#include <mutex>
#include <condition_variable>

extern std::atomic<bool> stepping_enabled(false);
extern bool stepping_flag;

void setStepping(bool valor);
void setStepping_enabled(bool valor);

#endif // STEPPING_CONTROL_HPP