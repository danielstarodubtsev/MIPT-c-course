#include <cassert>
#include <deque>
#include <stdexcept>
#include <vector>

#include "alena_deque.h"
// template <typename T>
// using Deque = std::deque<T>;

struct Spectator {
  static ssize_t balance;

  Spectator() {
    assert(balance >= 0 && "More destructor calls than constructor calls");
    ++balance;
  }
  Spectator(const Spectator&) {
    assert(balance >= 0 && "More destructor calls than constructor calls");
    ++balance;
  }
  Spectator(Spectator&&) {
    assert(balance >= 0 && "More destructor calls than constructor calls");
    ++balance;
  }
  ~Spectator() { --balance; }

  Spectator& operator=(const Spectator&) { return *this; }
  Spectator& operator=(Spectator&&) { return *this; };
};

ssize_t Spectator::balance = 0;

void test_stability(ssize_t size) {
  {
    Deque<Spectator> d(size);
    assert(Spectator::balance == size && "Constructors are not called in required quantity");
    d.pop_front();
    assert(Spectator::balance == size - 1 && "pop_front does not destroy object");
    d.pop_back();
    assert(Spectator::balance == size - 2 && "pop_back does not destroy object");
    d.push_front(Spectator());
    assert(Spectator::balance == size - 1 && "push_front does not construct object");
    d.push_back(Spectator());
    assert(Spectator::balance == size && "push_back does not construct object");
  }
  assert(Spectator::balance == 0 && "Destructor does not properly destroy objects");
}

struct ExplosiveSpectatorError : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

struct ExplosiveSpectator : public Spectator {
  mutable int delay;

public:
  ExplosiveSpectator()
    : ExplosiveSpectator(-1) {}

  ExplosiveSpectator(int delay)
    : Spectator(),
      delay(delay) {}

  ExplosiveSpectator(const ExplosiveSpectator& other)
    : Spectator(other),
      delay(other.delay <= 0 ? other.delay : other.delay - 1) {
    other.delay = delay;
    if (!delay) { throw ExplosiveSpectatorError("KABOOM!"); }
  }

  ExplosiveSpectator(ExplosiveSpectator&& other)
    : Spectator(other),
      delay(other.delay <= 0 ? other.delay : other.delay - 1) {
    other.delay = delay;
    if (!delay) { throw ExplosiveSpectatorError("KABOOM!"); }
  }

  ~ExplosiveSpectator() = default;

public:
  ExplosiveSpectator& operator=(const ExplosiveSpectator& other) {
    ExplosiveSpectator tmp(other);
    delay = tmp.delay;
    return *this;
  }

  ExplosiveSpectator& operator=(ExplosiveSpectator&& other) {
    ExplosiveSpectator tmp(other);
    delay = tmp.delay;
    return *this;
  }
};

void test_exception_safety(ssize_t size) {
  {
    Deque<ExplosiveSpectator> d(size);
    try {
      d.push_front(ExplosiveSpectator(0));
      assert(false && "Something went wrong...");
    } catch (const ExplosiveSpectatorError&) {
      assert(d.size() == size && Spectator::balance == size &&
             "push_front has bad exception guarantee");
    }
    try {
      d.push_back(ExplosiveSpectator(0));
      assert(false && "Something went wrong...");
    } catch (const ExplosiveSpectatorError&) {
      assert(d.size() == size && Spectator::balance == size &&
             "push_back has bad exception guarantee");
    }

    try {
      Deque<ExplosiveSpectator> copy = d;
      copy.push_back(ExplosiveSpectator());
      copy.rbegin()->delay = 0;
      assert(copy.size() == size + 1 && Spectator::balance == size * 2 + 1);

      d = copy;
      assert(false && "Something went wrong...");
    } catch (const ExplosiveSpectatorError&) {
      assert(d.size() == size && Spectator::balance == size &&
             "operator= has bad exception guarantee");
    }

    d.rbegin()->delay = 0;
    try {
      Deque<ExplosiveSpectator> copy = d;
      assert(false && "Something went wrong...");
    } catch (const ExplosiveSpectatorError&) {
      assert(Spectator::balance == size &&
             "Copy constructor does not clear everything on exception");
    }
  }
  assert(Spectator::balance == 0 && "Destructor call ruined everything");

  try {
    Deque<ExplosiveSpectator> d(size, ExplosiveSpectator(size - 1));
    assert(false && "Something went wrong...");
  } catch (const ExplosiveSpectatorError&) {
    assert(Spectator::balance == 0 && "Constructor from int and object does not clear everything");
  }
}

int main() {  
  std::vector<size_t> sizes_to_test = {
    5, 8, 10, 16, 32, 64, 128, 256, 50};  // add your chuck size here if it is not present

  for (size_t size : sizes_to_test) {
    test_stability(size);
    test_exception_safety(size);
    assert(Spectator::balance >= 0 && "More destructor calls than constructor calls");
  }
}