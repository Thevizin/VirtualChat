#include <iostream>
#include <string>
#include <queue>
using namespace std;

const int MAX_MESSAGES = 100;


class MessagesBlock{
    struct Message{
        int size;
        int type;
        string destination;
        string envoy;
    };

    struct Queue{
        int front, size;
        int queue[MAX_MESSAGES];

        bool empty(Queue &queue){
            return(queue.size == 0);
        }

        bool full(Queue &queue){
            return(queue.size == MAX_MESSAGES);
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
                queue.queue[(queue.size + queue.front) % MAX_MESSAGES] = valor;
                queue.size ++;
                cout << valor << " Added" << endl;
            }
            else{
                queue.queue[(queue.size + queue.front) % MAX_MESSAGES] = valor;
                queue.size ++;
            }
        }

        void dequeue(Queue &queue){
            if(empty(queue)) cout << "The queue is empty" << endl;
            else{
                cout << queue.queue[queue.front] << " Removed" << endl; 
                queue.front = (queue.front + 1) % MAX_MESSAGES;
                queue.size --;
            }
        }

    };
    
    // FYI: when we consider the type variable, we are considering the type of the message
    // It can be: A name definition (0), a standard message (1), a file message (2) or a instruction message (3)

    Message GlobalMessages[MAX_MESSAGES];


    public:



    Message returnLastMessage(string dest, string env){
        string message;
        for 

    }

};

MessagesBlock GlobalMessages[MAX_MESSAGES]


void handle_messages(MessagesBlock block){}




int main(){
    cout << "ola";
}