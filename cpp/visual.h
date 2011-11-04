#ifndef JOHANN_VISUAL_H
#define JOHANN_VISUAL_H

#include "definitions.h"

namespace Visual
{

const Logging::Logger logger("vis", Logging::DEBUG);

//tools
inline Int round (float t) { return static_cast<Int>(t+0.5f); }

//abstract images
typedef unsigned char Value;
namespace _private
{
    struct GREY  { enum { in_color = false, color_bytes = 1 }; };
    struct COLOR { enum { in_color = true,  color_bytes = 3 }; };
    template <class type>
    class Image
    {
    protected:
        const Int m_width, m_height;
        Value* m_data;
    public:
        Image (const Image<type>& other);
        Image (Int width, Int height=0) //square by default
            : m_width(width), m_height(height ? height : width),
              m_data(new(std::nothrow)
                      Value[m_width * m_height * type::color_bytes])
        { Assert (m_data, "could not allocate memory for image"); }
        ~Image () { delete[] m_data; }

        void set (Value v);
        void lighten (float t);
        bool save (string filename) const;
    };

    bool test_grey ();
    bool test_color ();
}

//concrete images
class GreyImage : public _private::Image<_private::GREY>
{
    typedef _private::Image<_private::GREY> Base;
public:
    GreyImage (const GreyImage& other);
    GreyImage (Int w, Int h=0) : Base(w,h) {}
    Value& operator() (Int x, Int y)
    {
        Assert2(0 <= x and x < m_width, " x-coord out of range: " << x);
        Assert2(0 <= y and y < m_height, "y-coord out of range: " << y);
        return m_data[x + m_width * y];
    }

    inline void set (Value v) { Base::set(v); }
    void set (Int x, Int y, Value c)
    {
        Assert2(0 <= x and x < m_width, " x-coord out of range: " << x);
        Assert2(0 <= y and y < m_height, "y-coord out of range: " << y);
        m_data[x + m_width * y] = c;
    }

    GreyImage shrink (Int factor);
};
enum Color { RED, GREEN, BLUE };
class ColorImage : public _private::Image<_private::COLOR>
{
    typedef _private::Image<_private::COLOR> Base;
public:
    ColorImage (const ColorImage& other);
    ColorImage (Int w, Int h=0) : _private::Image<_private::COLOR>(w,h) {}
    Value& operator() (Int x, Int y, Int c)
    {
        Assert2(0 <= x and x < m_width, " x-coord out of range: " << x);
        Assert2(0 <= y and y < m_height, "y-coord out of range: " << y);
        Assert2(0 <= c and c < 3, "y-coord out of range: " << y);
        return m_data[3 * (x + m_width * y) + c];
    }

    inline void set (Value v) { Base::set(v); }
    void set (Int x, Int y, Value r, Value g, Value b)
    {
        Assert2(0 <= x and x < m_width, " x-coord out of range: " << x);
        Assert2(0 <= y and y < m_height, "y-coord out of range: " << y);
        Value* pixel = m_data + 3 * (x + m_width * y);
        pixel[0] = r;
        pixel[1] = g;
        pixel[2] = b;
    }

    ColorImage shrink (Int factor);
};

}

#endif
