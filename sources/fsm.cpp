#include <functional>

template <typename... T>
struct FSM
{
  struct State
  {
    using type = std::function<State(T...)>;

    template <typename F, typename... U>
    constexpr State(F state_func, U... state_args)
        : func([state_func, state_args...](T... fsm_args) {
            return State{state_func(fsm_args..., state_args...)};
          })
    {
    }

    constexpr auto operator()(T... fsm_args) const { return func(fsm_args...); }
    constexpr operator bool() const { return func.operator bool(); }

  private:
    constexpr explicit State(type f) : func(f) {}

    type func;
  };

  template <typename F, typename... U>
  explicit constexpr FSM(F state_func, U... state_args)
      : current_state(state_func, state_args...)
  {
  }

  constexpr void Update(T... args)
  {
    if (current_state)
      current_state = current_state(args...);
  }

private:
  State current_state;
};

#include <chrono>
#include <iostream>
#include <string>
#include <thread>
using namespace std::chrono_literals;
using namespace std::string_literals;

using DT = std::chrono::milliseconds;
using TimedFSM = FSM<DT>;

TimedFSM::State start(DT, const std::string &, int);
TimedFSM::State countdown(DT, int counter);
TimedFSM::State wait(DT, DT, const TimedFSM::State &);

TimedFSM::State start(DT, const std::string &msg, int counter)
{
  std::cout << msg << std::endl;
  return {countdown, counter};
}

TimedFSM::State countdown(DT, int counter)
{
  if (counter > 0)
  {
    std::cout << "count " << counter << std::endl;
    return {wait, 1s, TimedFSM::State{countdown, counter - 1}};
  }
  else
  {
    return {start, "We have already been here!"s, 5};
  }
}

TimedFSM::State wait(DT dt, DT waiting_time, const TimedFSM::State &next)
{
  if (waiting_time > dt)
    return {wait, waiting_time - dt, next};
  else
    return next;
}

int main()
{
  TimedFSM fsm{start, "Hello World!"s, 10};

  while (true)
  {
    auto dt = 30ms;
    std::this_thread::sleep_for(dt);
    fsm.Update(dt);
  }
  return 0;
}