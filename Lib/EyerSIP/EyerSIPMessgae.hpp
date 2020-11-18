#ifndef EYERLIB_EYERSIPMESSGAE_HPP
#define EYERLIB_EYERSIPMESSGAE_HPP

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "EyerCore/EyerCore.hpp"
#include "EyerSIPFrom.hpp"

namespace Eyer
{
    class EyerSIPMessgaePrivate;

    class EyerSIPMessgae {
    public:
        EyerSIPMessgae();
        ~EyerSIPMessgae();

        int Parse(const uint8_t * buf, size_t length);

        EyerString & GetMethod();

        int GetFrom(EyerSIPFrom & from);
        int GetTo(EyerSIPFrom & to);
    private:
        EyerSIPMessgaePrivate * impl = nullptr;
    };
}

#endif //EYERLIB_EYERSIPMESSGAE_HPP
