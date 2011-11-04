#include <iostream>
#include <set>

class Derived;
class Base
{
private:
    static int id;
public:
    int m_id;
    std::set<Derived*> descendents;
    Base () : m_id(id++), descendents()
    { std::cout << "defining Base instance #" << m_id << "\n"; }
    virtual void addDescendent(Derived* descendent)
    { descendents.insert(descendent); }
};
int Base::id(0);

class Derived : public Base
{
public:
    Base* parent;
    Derived (Base* _parent) : Base (), parent(_parent)
    { parent->addDescendent(this); }
    virtual void addDescendent(Derived* descendent)
    {
        descendents.insert(descendent);
        parent->addDescendent(descendent);
    }
};

int main ()
{
    Base base1;
    Derived derived1(&base1),
            derived2(&base1),
            derived3(&derived1),
            derived4(&derived1),
            derived5(&derived3);
}
