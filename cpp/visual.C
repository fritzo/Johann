
#include "visual.h"

#include <cmath>
#include <cstring> //for memset
#include <cstdio> //for fopen, etc
#include <png.h>

namespace Visual
{

namespace _private
{

template<class type>
Image<type>::Image (const Image<type>& other)
    : m_width(other.m_width), m_height(other.m_height),
      m_data(new(std::nothrow)
              Value[m_width * m_height * type::color_bytes])

{
    Assert (m_data, "could not allocate memory for image");
    memcpy(m_data, other.m_data, type::color_bytes * m_width * m_height);
}



template <class type>
void Image<type>::set (Value v)
{
    Assert (m_data, "no data to set");
    std::memset(m_data, v, m_width * m_height * type::color_bytes);
}

template <class type>
bool Image<type>::save (string filename) const
{
    logger.info() << "Saving " << m_width << " x " << m_height << " image" |0;
    Logging::IndentBlock block;

    //check to open file first
    logger.debug() << "opening png file" |0;
    const string extension = ".png";
    FILE *file = fopen((filename + extension).c_str(), "wb");
    if (not file) {
        logger.warning() << "couldn't open file for writing" |0;
        return false;
    }

    //write to png file
    logger.debug() << "defining header" |0;
    png_structp p_writer = png_create_write_struct(PNG_LIBPNG_VER_STRING,
                                                   NULL, NULL, NULL);
    png_infop p_info = png_create_info_struct(p_writer);
    png_init_io(p_writer, file);
    png_set_IHDR(p_writer, p_info,
                 m_width, m_height,
                 8,                     //bit depth
                 type::in_color ? PNG_COLOR_TYPE_RGB
                                : PNG_COLOR_TYPE_GRAY,
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT);
    png_write_info(p_writer, p_info);

    //write image row-by-row
    logger.debug() << "writing data" |0;
    png_byte** rows = new png_byte*[m_height];
    for (Int y=0; y<m_height; ++y) {
        rows[y] = (png_byte*)(m_data + y * m_width * type::color_bytes);
    }
    png_write_image(p_writer, rows);
    delete[] rows;

    //finish png file
    logger.debug() << "finishing file" |0;
    png_write_end(p_writer, NULL);
    fclose(file);
    return true;
}

template <class type>
void Image<type>::lighten (float t)
{
    float p = expf(-t);
    for (Int i=0; i<m_width * m_height * type::color_bytes; ++i) {
        m_data[i] = round(powf(m_data[i] / 255.0, p) * 255.0);
    }
}

//explicit instantiations

template void Image<GREY>::lighten(float);
template void Image<COLOR>::lighten(float);
template void Image<COLOR>::set(unsigned char);

bool test_grey  ()
{
    Image<GREY> image(128);
    image.set(127);
    image.lighten(1);
    return image.save("test.gray" );
}
bool test_color ()
{
    Image<COLOR> image(128);
    image.set(127);
    image.lighten(1);
    return image.save("test.gray" );
}

}

GreyImage::GreyImage (const GreyImage& other) : Base(other) {}
ColorImage::ColorImage (const ColorImage& other) : Base(other) {}

GreyImage GreyImage::shrink (Int factor)
{
    if (factor == 0) return GreyImage(0,0);
    if (factor == 1) return *this;

    Int width  = (m_width + factor - 1) / factor;
    Int height = (m_height + factor - 1) / factor;

    Int S = factor;
    float scale = 1.0f / sqr(factor);
    GreyImage result(width,height);
    for (Int i_=0; i_<width; ++i_) {
    for (Int j_=0; j_<height; ++j_) {
        float c = 0.0f;
        for (Int _i=0; _i<S; ++_i) { Int i = S*i_+_i; if (i>=m_width) break;
        for (Int _j=0; _j<S; ++_j) { Int j = S*j_+_j; if (j>=m_height) break;
            c += operator()(i,j);
        } }
        result.set(i_,j_, round(scale * c));
    } }
    return result;
}
ColorImage ColorImage::shrink (Int factor)
{
    if (factor == 0) return ColorImage(0,0);
    if (factor == 1) return *this;

    Int width  = (m_width + factor - 1) / factor;
    Int height = (m_height + factor - 1) / factor;

    Int S = factor;
    float scale = 1.0f / sqr(factor);
    ColorImage result(width,height);
    for (Int i_=0; i_<width; ++i_) {
    for (Int j_=0; j_<height; ++j_) {
        float r = 0.0f, g = 0.0f, b = 0.0f;
        for (Int _i=0; _i<S; ++_i) { Int i = S*i_+_i; if (i>=m_width) break;
        for (Int _j=0; _j<S; ++_j) { Int j = S*j_+_j; if (j>=m_height) break;
            Value* pixel = m_data + 3 * (i + m_width * j);
            r += pixel[0];
            g += pixel[1];
            b += pixel[2];
        } }
        result.set(i_,j_, round(scale * r),
                          round(scale * g),
                          round(scale * b) );
    } }
    return result;
}

}

