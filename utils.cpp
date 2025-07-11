#include <iostream>
using namespace std;

struct Queue{
    int front, size;
    int queue[10];
};

bool empty(Queue &queue){
    return(queue.size == 0);
}

bool full(Queue &queue){
    return(queue.size == 10);
}

void start(Queue &queue){
    queue.size = queue.front = 0;
}

void enqueue(Queue &queue, int valor){
    if(full(queue)){
        cout << "The queue is full" << endl;
    }
    else if(empty(queue)){
        queue.front = 0;
        queue.queue[(queue.size + queue.front) % 10] = valor;
        queue.size ++;
        cout << valor << " Added" << endl;
    }
    else{
        queue.queue[(queue.size + queue.front) % 10] = valor;
        queue.size ++;
        cout << valor << " Added" << endl;
    }
}

void dequeue(Queue &queue){
    if(empty(queue)) cout << "The queue is empty" << endl;
    else{
        cout << queue.queue[queue.front] << " Removed" << endl; 
        queue.front = (queue.front + 1) % 10;
        queue.size --;
    }
}

void display(Queue &queue){
    if(full(queue)) cout << "The queue is full" << endl;
    else if(empty(queue)) cout << "The queue is empty" << endl;
    else{
        int j = queue.front;
        cout << "Queue: ";
        for(int i = 0; i < queue.size; i++){
            cout << queue.queue[(i + j) % 10] << " ";
        }
        cout << endl;
    }
}

int main(){
    Queue q;
    start(q);
    enqueue(q, 1);
    enqueue(q, 2);
    enqueue(q, 3);
    display(q);
    dequeue(q);
    display(q);
}
