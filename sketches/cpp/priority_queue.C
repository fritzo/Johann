
#include <stdlib.h>
#include <iostream>
#include <queue>
#include <set>

using namespace std;

int main ()
{
    cout << "\npriority_queue: ";
    priority_queue<int> q;
    for (int i=0; i<100; ++i) {
        q.push(random() % 50);
    }
    
    while (!q.empty()) {
        cout << q.top() << " ";
        q.pop();
    }
    
    
    cout << "\n\nset: ";
    set<int> s;
    for (int i=0; i<100; ++i) {
        s.insert(random() % 50);
    }
    
    for (std::set<int>::iterator i=s.begin(); i!=s.end(); ++i) {
        cout << *i << " ";
    }


    return 0;
}

