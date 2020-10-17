#ifndef EYERLIB_EYERMP4BOX_HPP
#define EYERLIB_EYERMP4BOX_HPP

#include "EyerCore/EyerCore.hpp"
#include <string.h>

namespace Eyer
{
    enum BoxType
    {
        UNKNOW = 0,
        FTYP = 1,
        MOOV = 2,
        MVHD = 3,
        TRAK = 4,
        TKHD = 5
    };

    class EyerMP4Box {
    public:
        EyerMP4Box(const EyerBuffer & _buffer);
        virtual ~EyerMP4Box();

        int Get(EyerMP4Box * * box);


        virtual BoxType GetType();
        virtual int PrintInfo();

    protected:
        EyerBuffer buffer;
    };
}

#endif //EYERLIB_EYERMP4BOX_HPP
