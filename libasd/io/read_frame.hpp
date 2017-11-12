#ifndef LIBASD_READ_FRAME_H
#define LIBASD_READ_FRAME_H
#include <libasd/io/read_binary_as.hpp>
#include <libasd/io/frame_header.hpp>
#include <libasd/io/frame_data.hpp>
#include <libasd/io/frame.hpp>

namespace asd
{
namespace detail
{

template<typename sourceT>
void read_frame_header_impl(FrameHeader& fh, sourceT& source)
{
    fh.number        = read_binary_as<std::int32_t>(source);
    fh.max_data      = read_binary_as<std::int16_t>(source);
    fh.min_data      = read_binary_as<std::int16_t>(source);
    fh.x_offset      = read_binary_as<std::int16_t>(source);
    fh.y_offset      = read_binary_as<std::int16_t>(source);
    fh.x_tilt        = read_binary_as<float       >(source);
    fh.y_tilt        = read_binary_as<float       >(source);
    fh.is_stimulated = read_binary_as<bool        >(source);
    fh._booked1      = read_binary_as<std::int8_t >(source);
    fh._booked2      = read_binary_as<std::int16_t>(source);
    fh._booked3      = read_binary_as<std::int32_t>(source);
    fh._booked4      = read_binary_as<std::int32_t>(source);
    return;
}

template<typename contT>
void read_frame_data_impl(FrameData<contT>& fd, const char*& ptr,
                          const std::size_t x, const std::size_t y)
{
    fd.x_pixel = x; fd.y_pixel = y;

    std::size_t total_size = x * y;
    ::asd::container::resize(fd.data, total_size);

    const std::int16_t* init = reinterpret_cast<const std::int16_t*>(ptr);
    const std::int16_t* last = reinterpret_cast<const std::int16_t*>(ptr);
    last += total_size;

    std::copy(init, last, fd.data.begin());

    ptr += total_size;
    return;
}

template<typename contT>
void read_frame_data_impl(FrameData<contT>& fd, std::istream& is,
                          const std::size_t x, const std::size_t y)
{
    fd.x_pixel = x; fd.y_pixel = y;
    std::size_t total_size = x * y;
    ::asd::container::resize(fd.data, total_size);
    is.read(reinterpret_cast<char*>(fd.data.data()),
            total_size * sizeof(std::int16_t));
    return;
}

template<typename channelT, typename contT>
struct read_frame_impl
{
    template<typename sourceT>
    static Frame<channelT, contT>
    invoke(sourceT& source, const std::size_t x, const std::size_t y)
    {
        Frame<channelT, contT> f;
        read_frame_header_impl(f.header, source);
        for(std::size_t i=0; i<f.data.size(); ++i) // for each channel
        {
            read_frame_data_impl(f.data[i], source, x, y);
        }
        return f;
    }
};

template<typename contT>
struct read_frame_impl<channel<1>, contT>
{
    template<typename sourceT>
    static Frame<channel<1>, contT>
    invoke(sourceT& source, const std::size_t x, const std::size_t y)
    {
        Frame<channel<1>, contT> f;
        read_frame_header_impl(f.header, source);
        read_frame_data_impl(f.data, source, x, y);
        return f;
    }
};

} // detail

inline FrameHeader read_frame_header(const char* ptr)
{
    FrameHeader fh;
    detail::read_frame_header_impl(fh, ptr);
    return fh;
}
inline FrameHeader read_frame_header(std::istream& is)
{
    FrameHeader fh;
    detail::read_frame_header_impl(fh, is);
    return fh;
}

template<typename contT = container::vec>
FrameData<contT>
read_frame_data(const char* ptr, std::size_t x, std::size_t y)
{
    FrameData<contT> fd;
    detail::read_frame_data_impl<contT>(fd, ptr, x, y);
    return fd;
}
template<typename contT = container::vec>
FrameData<contT>
read_frame_data(std::istream& is, std::size_t x, std::size_t y)
{
    FrameData<contT> fd;
    detail::read_frame_data_impl<contT>(fd, is, x, y);
    return fd;
}

template<typename chT = channel<1>, typename contT = container::vec>
Frame<chT, contT> read_frame(const char* ptr, std::size_t x, std::size_t y)
{
    return detail::read_frame_impl<chT, contT>::invoke(ptr, x, y);
}
template<typename chT = channel<1>, typename contT = container::vec>
Frame<chT, contT> read_frame(std::istream& is, std::size_t x, std::size_t y)
{
    return detail::read_frame_impl<chT, contT>::invoke(is, x, y);
}

}// asd
#endif// LIBASD_READ_FRAME_H
