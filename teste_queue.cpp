#include <iostream>
using namespace std;

struct Queue {
  int front, back;
  int queue[10];
};

// Protótipos das funções
void start(Queue &queue);
bool full(Queue &queue);
bool empty(Queue &queue);
void enqueue(Queue &queue, int x);
void dequeue(Queue &queue);
void display(Queue &queue);

void start(Queue &queue){
  queue.front = -1;
  queue.back = -1;
}

bool full(Queue &queue){
  return ( (queue.back + 1) % 10 == queue.front);
}

bool empty(Queue &queue){
  return (queue.front == -1);
}

void enqueue(Queue &queue, int x){
    if(full(queue)) dequeue(queue);

    if(empty(queue)){
        queue.front = queue.back = 0;
        queue.queue[queue.back] = x;
        cout << x << " Added" << endl;
    }
    else{
        queue.back = (queue.back + 1) % 10;
        queue.queue[queue.back] = x;
        cout << x << " Added" << endl;
    }
}

void dequeue(Queue &queue){
  if(empty(queue)) 
    cout << "The queue is empty" << endl;
  else if(queue.front == queue.back){
    cout << queue.queue[queue.front] << " Removed" << endl;
    queue.front = -1;
    queue.back = -1;
  }
  else{
    cout << queue.queue[queue.front] << " Removed" << endl;
    queue.front = (queue.front + 1) % 10;
  }
}

void get_last(Queue &queue){
    cout << queue.queue[queue.back] << "\n";
}

void display(Queue &queue) {
  if (empty(queue)) {
    cout << "The queue is empty" << endl;
    return;
  }

  cout << "Queue: ";

  int i = queue.front;
  while (true) {
    cout << queue.queue[i] << " ";
    if (i == queue.back)
      break;
    i = (i + 1) % 10;
  }

  cout << endl;
}


int main() {
  Queue q;
  start(q);
  enqueue(q, 1);
  enqueue(q, 2);
  enqueue(q, 3);
  enqueue(q, 4);
  enqueue(q, 5);
  enqueue(q, 6);
  enqueue(q, 7);
  enqueue(q, 8);
  enqueue(q, 9);
  display(q);
  get_last(q);
  enqueue(q, 10);
  display(q);
  get_last(q);
  enqueue(q, 11);
  display(q);
  get_last(q);
  enqueue(q, 12);
  display(q);
  get_last(q);
  enqueue(q, 13);
  display(q);
  get_last(q);

  return 0;
}
