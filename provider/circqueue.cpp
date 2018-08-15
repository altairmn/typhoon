#include <iostream>
#include <cstdlib>

class CircularQueue {
  private:
    int size, *q;
    bool empty, full;
  public:
    int len, start, end;
    CircularQueue(int _size) : len(0), size(_size), empty(true), full(false), end(0), start(0) {
      q = new int[size];
    }
    ~CircularQueue() {
      delete[] q;
    }

    bool pop();
    bool push(int);
    int top();
};

bool CircularQueue::pop() {
  if (start == end) return false;
  start = (start + 1) % size;
  len--;
  return true;
}

bool CircularQueue::push(int num) {
  if ( (end + 1) % size == start) {
    return false; 
  }
  q[end] = num;
  end = (end + 1) % size;
  len++;
  return true;
}

int CircularQueue::top() {
  return q[start];
}

int main() {
  CircularQueue q(10);
  int steps = 100, num;
  bool push, res;
  for (int i = 0; i < steps; ++i) {
    push = rand() % 2;
    if (push) {
      num = rand() % 1000;
      res = q.push(num);
      if (res == true) printf("[->%d]; len: %d; start: %d; end: %d\n", num, q.len, q.start, q.end);
      else printf("[FULL]. len: %d; start: %d; end: %d\n", q.len, q.start, q.end);
    } else {
      num = q.top();
      res = q.pop();
      if (res == false) printf("[EMPTY] len: %d; start: %d; end: %d\n", q.len, q.start, q.end);
      else printf("[<-%d]; len: %d; start: %d; end: %d\n", num, q.len, q.start, q.end);
    }
  }

  return 0;

}
