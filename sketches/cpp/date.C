

#include <sys/time.h>
#include <string>
#include <iostream>

std::string date (bool hour=false)
{
    const size_t size = 20; //fits e.g. 2007-05-17-11-28
    static char buff[size];

    time_t t = time(NULL);
    tm T;
    gmtime_r (&t,&T);
    if (hour) strftime(buff,size, "%Y-%m-%d-%H-%M", &T);
    else      strftime(buff,size, "%Y-%m-%d", &T);
    return buff;
}
int main ()
{
    std::cout << date() << std::endl;

    return 0;
}

