
#include "definitions.h"
#include "fifo_queue.h"

using namespace std;
using namespace nonstd;

namespace Testing
{

Logging::Logger logger("test", Logging::DEBUG);

typedef fifo_queue<Int> PQ;

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
void merge_print (PQ& pq, Int dep, Int rep)
{
    logger.debug() << "merging " << dep << " := " << rep |0;
    pq.merge(dep,rep);
}

}

using Testing::push_print;
using Testing::pop_print;
using Testing::merge_print;

int main ()
{
    Logging::switch_to_log("test.log");
    Logging::title("Running FIFO Queue Test");

    fifo_queue<Int> data;

    push_print(data, 2);
    push_print(data, 1);
    push_print(data, 3);
    push_print(data, 2);
    push_print(data, 4);
    merge_print(data, 3,4);
    while (data) {
        pop_print(data);
    }

    return 0;
}


