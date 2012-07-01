#ifndef JOHANN_NODES_H
#define JOHANN_NODES_H

#include "node_heap.h"


//================================ All nodes ================================
namespace Nodes
{

//ob nodes
//WARNING: the REP field must be maintained even after death;
//         thus REP cannot be used as the is_used_field (which is set to 0).
const Int _BOOL_PROPS_ = 0x2;
class ObSignature
{
public:
    typedef ObSignature MySig;
    enum { size_in_bytes = 32, is_used_field = _BOOL_PROPS_};
    typedef Heap_::Node_   <MySig> Node;
    typedef Heap_::Heap_   <MySig> Heap;
    typedef Heap_::Pos_    <MySig> Pos;
    typedef Heap_::Name_   <MySig> Name;
    typedef Heap_::Handle_ <MySig> Handle;
};

//application nodes
const Int _LR_ = 0x1;
class AppSignature
{
public:
    typedef AppSignature MySig;
    enum { size_in_bytes = 32, is_used_field = _LR_};
    typedef Heap_::Node_   <MySig> Node;
    typedef Heap_::Heap_   <MySig> Heap;
    typedef Heap_::Pos_    <MySig> Pos;
    typedef Heap_::Name_   <MySig> Name;
    typedef Heap_::Handle_ <MySig> Handle;
};

//composition nodes
class CompSignature
{
public:
    typedef CompSignature MySig;
    enum { size_in_bytes = 32, is_used_field = _LR_};
    typedef Heap_::Node_   <MySig> Node;
    typedef Heap_::Heap_   <MySig> Heap;
    typedef Heap_::Pos_    <MySig> Pos;
    typedef Heap_::Name_   <MySig> Name;
    typedef Heap_::Handle_ <MySig> Handle;
};

//join nodes
class JoinSignature
{
public:
    typedef JoinSignature MySig;
    enum { size_in_bytes = 32, is_used_field = _LR_};
    typedef Heap_::Node_   <MySig> Node;
    typedef Heap_::Heap_   <MySig> Heap;
    typedef Heap_::Pos_    <MySig> Pos;
    typedef Heap_::Name_   <MySig> Name;
    typedef Heap_::Handle_ <MySig> Handle;
};

//readable typedefs
typedef Nodes::ObSignature::Heap   ObHeap;
typedef Nodes::ObSignature::Node&  ObRef;
typedef Nodes::ObSignature::Pos    Ob;
typedef Nodes::ObSignature::Name   ObName;
typedef Nodes::ObSignature::Handle ObHdl;

typedef Nodes::AppSignature::Heap   AppHeap;
typedef Nodes::AppSignature::Node&  AppRef;
typedef Nodes::AppSignature::Pos    App;
typedef Nodes::AppSignature::Name   AppName; //not used
typedef Nodes::AppSignature::Handle AppHdl;  //not used

typedef Nodes::CompSignature::Heap   CompHeap;
typedef Nodes::CompSignature::Node&  CompRef;
typedef Nodes::CompSignature::Pos    Comp;
typedef Nodes::CompSignature::Name   CompName; //not used
typedef Nodes::CompSignature::Handle CompHdl;  //not used

typedef Nodes::JoinSignature::Heap   JoinHeap;
typedef Nodes::JoinSignature::Node&  JoinRef;
typedef Nodes::JoinSignature::Pos    Join;
typedef Nodes::JoinSignature::Name   JoinName; //not used
typedef Nodes::JoinSignature::Handle JoinHdl;  //not used

}

//public declarations
using Nodes::Ob;
using Nodes::ObName;
using Nodes::ObHdl;

using Nodes::App;
using Nodes::Comp;
using Nodes::Join;

#endif

