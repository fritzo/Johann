

template<int T> int evaluate (int x);

template<> int evaluate<1> (int x) { return x; }
template<> int evaluate<2> (int x) { return -x; }

int main ()
{
    int x = 12;
    int y1 = evaluate<1>(x);
    int y2 = evaluate<2>(x);

    return 0;
}

