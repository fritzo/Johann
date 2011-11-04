
//sketch of intersection iterator
Class IntersectionIterator
{
private:
    InorderIterator iter1, iter2;
public:
    IntersectionIterator(root1, root2);
    Pos dereference();
    void increment();
};
IntersectionIterator::IntersectionIterator(root1, root2)
    : iter1(root1),
      iter2(root2)
{
    iter1.begin();
    iter2.begin();
}
Pos IntersectionIterator::dereference()
{
    return *iter1;
}
IntersectionIterator::increment()
{
    while (!iter1.finished() && !iter2.finished()) {
        switch(Hcmp(*iter1,*iter2)) {
            case 1:  ++iter2; break;
            case -1: ++iter1; break;
            case 0:  return;
        }
    }
    iter1.end();
    iter2.end();
}

//sketchs of fast merge operation
class InorderPopper : public Iterator
{
public:
    InorderPopper(root);
    void begin();
    bool finished();
};
class InorderPusher : public Iterator
{
public:
    InorderPusher(root);
    void begin_at(Pos iter);
};
void merge(Pos rep_root, Pos dep_root)
{//fast merge for simple binary search trees
    InorderPusher iter1(rep_root);
    InorderPopper iter2(dep_root);
    iter2.begin();
    iter1.begin_at(*iter2);
    while (!iter2.finished()) {
        iter1.push(iter2.pop());
        iter1.advance_to(iter2);
    }
}
/*
Note: one should be able to move over entire sections of the dep tree at a
time, though this will bring little speedup.  
Note: it's better to pop only from leaves of the dep tree.  this suggests a
depth-first recursive algorithm, where sub-branches of the heap are
successively merged.
*/
void merge(Pos rep_root, Pos dep_root)
{//fast merge using heap structure
    ...
}
/*
Note: the relative efficiency of the above merging functions depends on
the average density of sub-heaps within the heap.  This, in turn, depends on
both thermal/connectedness properties of the db stats and the axiomatic
system on which the db is based.
*/


