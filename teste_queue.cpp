#include <iostream>
using namespace std;

struct Queue {
  int front, back;
  int queue[10];

  // Métodos da struct
  void start() {
    front = -1;
    back = -1;
  }

  bool full() {
    return ((back + 1) % 10 == front);
  }

  bool empty() {
    return (front == -1);
  }

  void enqueue(int x) {
    if (full()) dequeue();

    if (empty()) {
      front = back = 0;
      queue[back] = x;
      cout << x << " Added" << endl;
    } else {
      back = (back + 1) % 10;
      queue[back] = x;
      cout << x << " Added" << endl;
    }
  }

  void dequeue() {
    if (empty()) {
      cout << "The queue is empty" << endl;
    } else if (front == back) {
      cout << queue[front] << " Removed" << endl;
      front = -1;
      back = -1;
    } else {
      cout << queue[front] << " Removed" << endl;
      front = (front + 1) % 10;
    }
  }

  int get_last() {
    if (empty()) {
      cout << "The queue is empty" << endl;
      return -1;
    }
    return queue[back];
  }

  void display() {
    if (empty()) {
      cout << "The queue is empty" << endl;
      return;
    }

    cout << "Queue: ";
    int i = front;
    while (true) {
      cout << queue[i] << " ";
      if (i == back) break;
      i = (i + 1) % 10;
    }
    cout << endl;
  }
};

int main() {
  Queue q;
  q.start();
  q.enqueue(1);
  q.enqueue(2);
  q.enqueue(3);
  q.enqueue(4);
  q.enqueue(5);
  q.enqueue(6);
  q.enqueue(7);
  q.enqueue(8);
  q.enqueue(9);
  q.display();
  cout << "Last: " << q.get_last() << endl;
  q.enqueue(10);
  q.display();
  cout << "Last: " << q.get_last() << endl;
  q.enqueue(11);
  q.display();
  cout << "Last: " << q.get_last() << endl;
  q.enqueue(12);
  q.display();
  cout << "Last: " << q.get_last() << endl;
  q.enqueue(13);
  q.display();
  cout << "Last: " << q.get_last() << endl;

  return 0;
}
