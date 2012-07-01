
#include "nodes.h"

using Nodes::ObHeap;
using Nodes::AppHeap;
using Nodes::AppHdl;
using Nodes::CompHeap;
using Nodes::CompHdl;
using Nodes::JoinHeap;
using Nodes::JoinHdl;

namespace Nodes
{

//definitions for static variables
template<> ObHeap   Ob::s_heap   = ObHeap();
template<> AppHeap  App::s_heap  = AppHeap();
template<> CompHeap Comp::s_heap = CompHeap();
template<> JoinHeap Join::s_heap = JoinHeap();

template<> Int ObName::s_numNames   = 0;
template<> Int AppName::s_numNames  = 0; //not used
template<> Int CompName::s_numNames = 0; //not used
template<> Int JoinName::s_numNames = 0; //not used

template<> ObHdl::DictType   ObHdl::s_dict   = ObHdl::DictType();
template<> AppHdl::DictType  AppHdl::s_dict  = AppHdl::DictType(); //not used
template<> CompHdl::DictType CompHdl::s_dict = CompHdl::DictType(); //not used
template<> JoinHdl::DictType JoinHdl::s_dict = JoinHdl::DictType(); //not used

}

