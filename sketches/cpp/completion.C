
#include <string>
#include <map>

typedef std::string Name;

//multisets for size comparison
template<class T> class multiset
{
    typedef std::map<T,unsigned> Map;
    Map m_map;
public:

    //modification
    void insert (T t) {
        Map::iterator i = m_map.find(t);
        if (i == m_map.end()) m_map[t] = 1;
        else ++*i;
    }
    unsigned operator[] (T t) const {
        Map::const_iterator i = m_map.find(t);
        if (i == m_map.end()) return 0;
        else return *i;
    }

    //properties
    bool empty () const { return m_map.empty(); }
    bool operator <= (const multiset<T>& s) const { LATER }
    bool operator <  (const multiset<T>& s) const { LATER }
}

//sketchy implementation of unfailing completion
class Length
{//a linear multinomial in arbitrarily many variables
    float m_atoms;
    std::map<Name, unsigned> m_vars;
public:

    //construction
    Length (float a) : m_atoms(a) {}    //for atoms
    Length (Name n) : m_atoms(0) { m_vars.insert(n); }
    Length operator + (const Length& l) const { LATER }

    //properties
    bool operator <= (const Length& l) const {
        return m_atoms <= l.m_atoms and m_vars <= l.m_vars;
    }
    bool operator < (const Length& l) const { LATER }
    bool ground () const { return m_vars.empty(); }
}

class Term
{
    Length m_size;
public:

    //construction
    Term (Name n, float l)              { LATER } //atoms
    Term (Name n)                       { LATER } //vars
    Term operator * (const Term & t)    { LATER }  //apps

    //peoperties
    Length size () const { return m_size; }
    bool operator > (const Term& t) const { return m_size < t.m_size; }
    bool operator < (const Term& t) const { return t.m_size < m_size; }
};

class Completer
{
    typedef std::pair<Term*,Term*> Eqn;
    typedef std::set<Eqn> Eqns;                 Eqns m_E;
    typedef std::multimap<Term*,Term*> Reds;    Reds m_R;
public:
    Completer () {}

    //construction
    void equal (Term& s, Term& t) { m_E.insert(Eqn(&s,&t)); }

    //operations
    bool complete (max_size = 1000);
private:
    std::queue<Eqn> m_orient_queue;
    void orient () {
        Eqn eqn = m_orient_queue.pop();
        if (eqn.first < eqn.second) { LATER return; }
        if (eqn.first > eqn.second) { LATER return; }
    }

    void check_CP () { LATER } //critical_pairs
    //others... LATER

    //output
public:
    void print () { LATER }
};

void test ()
{
    Completer c;

    Term S("S", 2), K("K", 1), J("J", 2);
    Term x("x"), y("y"), z("z");

    //combinatory logic
    c.equal(S*x*y*z, (x*z)*(y*z));
    c.equal(K*x*y, x);
    //and ACI(L?) for J
    c.equal(J*x*x, x);
    c.equal(J*x*y, J*y*x);
    c.equal(J*x*(J*y*z), J*(J*x*y)*z);
    //c.equal(J*x*y*z, J*(x*z)*(y*z));

    c.complete(100);
    c.print();
}

int main ()
{
    test();

    return 0;
}

