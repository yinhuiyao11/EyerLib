#ifndef EYERLIB_EYERCAVLC_HPP
#define EYERLIB_EYERCAVLC_HPP

#include "EyerBitStream.hpp"

namespace Eyer
{
    class EyerCAVLC {
    public:
        EyerCAVLC();
        ~EyerCAVLC();

        int GetCoeffNumTrailingOnes(EyerBitStream & bs, int & totleCoeff, int & trailingOnes, int nC);
    };
}

#endif //EYERLIB_EYERCAVLC_HPP
