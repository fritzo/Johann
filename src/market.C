
#include "market.h"
#include "socket_tools.h"

namespace ST = SocketTools;

namespace Market
{

void Market::clear ()
{
    Lock lock(m_problems);
    for (prob_iter i=m_problems->begin(); i!=m_problems->end(); ++i) {
        delete *i;
    }
    m_problems->clear();
}
void Market::update ()
{
    Lock lock(m_problems);
    for (prob_iter i=m_problems->begin(); i!=m_problems->end(); ++i) {
        (*i)->simplify();
    }
}

//output
void Market::write_stats (ostream& out) const
{
    Lock lock(m_problems);
    out << "problems: " << m_problems->size() << std::endl;
}

}


