#include "EyerSLICE.hpp"

#include "EyerAVC_VLC.hpp"
#include "EyerNALU.hpp"

namespace Eyer
{
    NaluType EyerSLICE::GetNalType()
    {
        return NaluType::NALU_TYPE_SLICE;
    }

    EyerSLICE::EyerSLICE(EyerNALU & _nalu) : EyerSLICEBase(_nalu)
    {

    }

    EyerSLICE::~EyerSLICE()
    {

    }
}