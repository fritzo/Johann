
#include "definitions.h"
#include "priority_queue.h"

using namespace std;
using namespace nonstd;

namespace Testing
{

Logging::Logger logger("test");

typedef priority_queue<Int> PQ;

void push_print (PQ& pq, Int item)
{
    logger.debug() << "pushing " << item |0;
    pq.push(item);
}

Int pop_print (PQ& pq)
{
    Int item = pq.pop();
    logger.debug() << "popping " << item |0;
    return item;
}

}

using Testing::push_print;
using Testing::pop_print;

int main ()
{
    Logging::switch_to_log("test.log");
    Logging::title("Running Priority Queue Test");

    priority_queue<Int> data;

    push_print(data, 2);
    push_print(data, 1);
    push_print(data, 3);
    push_print(data, 2);
    while (data) {
        pop_print(data);
    }
    data.clear();

    return 0;
}

