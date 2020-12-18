#include "EyerMacroblock.hpp"
#include "EyerBitStream.hpp"

#include <algorithm>
#include <cstdio>
#include <cmath>
#include <iostream>

namespace Eyer
{
    EyerMacroblock::EyerMacroblock(int _mbIndex)
    {
        mbIndex = _mbIndex;
    }

    EyerMacroblock::~EyerMacroblock()
    {

    }

    int EyerMacroblock::Parse(EyerBitStream & bs, EyerSPS & _sps, EyerPPS & _pps, EyerSLICEHeader & _sliceHeader)
    {
        sps = _sps;
        pps = _pps;
        sliceHeader = _sliceHeader;

        SLICEType sliceType = sliceHeader.GetSLICEType();

        //TODO
        uint32_t mb_type = 0;
        if(!pps.entropy_coding_mode_flag){
            mb_type = bs.bs_read_ue();
        }
        else {
            // TODO ae(v)
            // mb_type = bs.bs_read_ae();
        }

        mbType = MB_TYPE::Select(sliceType, mb_type);
        if(MB_TYPE::I_PCM == mbType){
            //TODO PCM
        }
        else{
            int noSubMbPartSizeLessThan8x8Flag = 1;
            if(
                    mbType != MB_TYPE::I_NxN &&
                    mbType.MbPartPredMode() != MB_PART_PRED_MODE::Intra_16x16 &&
                    mbType.MunMbPart() == 4
                    ){
                // TODO B P Slice
            }
            else{
                uint8_t transform_size_8x8_flag = 0;
                if(pps.transform_8x8_mode_flag && mbType == MB_TYPE::I_NxN){
                    transform_size_8x8_flag = bs.bs_read_u1();
                }
                mb_pred(bs);
            }

            // EyerLog("Byte index: %d, bit offset: %d\n", bs.bits_left, bs.bits_left);

            if(mbType.MbPartPredMode() != MB_PART_PRED_MODE::Intra_16x16){
                uint32_t coded_block_pattern = bs.bs_read_me(sps.ChromaArrayType, mbType.MbPartPredMode());

                CodecBlockPatterLuma = coded_block_pattern % 16;
                CodecBlockPatterChroma = coded_block_pattern / 16;

                EyerLog("CBP: %d\n", coded_block_pattern);
                EyerLog("CodecBlockPatterLuma: %d\n", CodecBlockPatterLuma);
                EyerLog("CodecBlockPatterChroma: %d\n", CodecBlockPatterChroma);
            }

            if(CodecBlockPatterLuma > 0 || CodecBlockPatterChroma > 0 || mbType.MbPartPredMode() == MB_PART_PRED_MODE::Intra_16x16){
                int32_t mb_qp_delta = bs.bs_read_se();
                residual(bs);
            }
        }

        return 0;
    }

    int EyerMacroblock::mb_pred(EyerBitStream & bs){
        if(
                mbType.MbPartPredMode() == MB_PART_PRED_MODE::Intra_4x4 ||
                mbType.MbPartPredMode() == MB_PART_PRED_MODE::Intra_8x8 ||
                mbType.MbPartPredMode() == MB_PART_PRED_MODE::Intra_16x16
                ){
            if(mbType.MbPartPredMode() == MB_PART_PRED_MODE::Intra_4x4){
                uint32_t prev_intra4x4_pred_mode_flag[16];
                memset(prev_intra4x4_pred_mode_flag, 0, 16 * sizeof(uint32_t));
                uint32_t rem_intra4x4_pred_mode_flag[16];
                memset(rem_intra4x4_pred_mode_flag, 0, 16 * sizeof(uint32_t));
                for(int luma4x4BlkIdx=0; luma4x4BlkIdx<16; luma4x4BlkIdx++){
                    prev_intra4x4_pred_mode_flag[luma4x4BlkIdx] = bs.bs_read_u1();
                    if(!prev_intra4x4_pred_mode_flag[luma4x4BlkIdx]){
                        rem_intra4x4_pred_mode_flag[luma4x4BlkIdx] = bs.bs_read_u(3);
                    }
                }
            }

            if(mbType.MbPartPredMode() == MB_PART_PRED_MODE::Intra_8x8){
                uint32_t prev_intra8x8_pred_mode_flag[4];
                memset(prev_intra8x8_pred_mode_flag, 0, 4 * sizeof(uint32_t));
                uint32_t rem_intra8x8_pred_mode_flag[4];
                memset(rem_intra8x8_pred_mode_flag, 0, 4 * sizeof(uint32_t));
                for(int luma8x8BlkIdx=0; luma8x8BlkIdx<4; luma8x8BlkIdx++){
                    prev_intra8x8_pred_mode_flag[luma8x8BlkIdx] = bs.bs_read_u1();
                    if(!prev_intra8x8_pred_mode_flag[luma8x8BlkIdx]){
                        rem_intra8x8_pred_mode_flag[luma8x8BlkIdx] = bs.bs_read_u(3);
                    }
                }
            }

            if(sps.ChromaArrayType == 1 || sps.ChromaArrayType == 2){
                uint32_t intra_chrome_pred_mode = bs.bs_read_ue();
            }
        }
        else{
            // TODO B P
        }
        return 0;
    }

    int EyerMacroblock::residual(EyerBitStream & bs)
    {
        if(CodecBlockPatterLuma){
            int index8x8 = 0;
            int blockX = 0;
            int blockY = 0;
            int blockSubIndexX = 0;
            int blockSubIndexY = 0;
            for(blockY = 0; blockY < 4; blockY += 2){
                for(blockX = 0; blockX < 4; blockX += 2){
                    EyerLog("8x8 Block\n");
                    if(!pps.entropy_coding_mode_flag){
                        //CAVLC
                        for(blockSubIndexY = blockY; blockSubIndexY < blockY + 2; blockSubIndexY ++){
                            for(blockSubIndexX = blockX; blockSubIndexX < blockX + 2; blockSubIndexX ++){
                                index8x8 = 2 * (blockY / 2) + blockX / 2;
                                if(CodecBlockPatterLuma & (1 << index8x8)){
                                    EyerLog("=================Sub Block=================\n");
                                    EyerLog("有残差，Block X: %d, Y: %d, Sub X: %d, Sub Y: %d\n", blockX, blockY, blockSubIndexX, blockSubIndexY);
                                    lumaResidual[blockSubIndexY][blockSubIndexX].emptyBlock = true;

                                    get_luma_coeff(bs, blockSubIndexX, blockSubIndexY);
                                }
                                else{
                                    EyerLog("无残差，Block X: %d, Y: %d, Sub X: %d, Sub Y: %d\n", blockX, blockY, blockSubIndexX, blockSubIndexY);
                                    lumaResidual[blockSubIndexY][blockSubIndexX].emptyBlock = false;
                                }
                            }
                        }
                    }
                    else{
                        //CABAC
                    }
                }
            }
        }
        return 0;
    }

    int EyerMacroblock::get_luma_coeff(EyerBitStream & bs, int x, int y)
    {
        int err;

        int max_ceoff_number = 4;
        // int blockType =
        if(mbType.MbPartPredMode() == MB_PART_PRED_MODE::Intra_16x16 || mbType == MB_TYPE::I_PCM){
            // LUMA_INTRA16x16AC
        }
        else{
            // LUMA
            max_ceoff_number = 16;
        }

        int numCoeff_vlcIdx = 0, prefixLength = 0, suffixLength = 0, level_prefix = 0, level_suffix = 0;
        int levelSuffixSize = 0, levelCode = 0, i = 0;

        int numberCurrent = GetNumberCurrent(x, y); // nC
        EyerLog("nC: %d\n", numberCurrent);
        if (numberCurrent < 2){
            numCoeff_vlcIdx = 0;
        }
        else if(numberCurrent < 4){
            numCoeff_vlcIdx = 1;
        }
        else if(numberCurrent < 8){
            numCoeff_vlcIdx = 2;
        }
        else{
            numCoeff_vlcIdx = 3;
        }

        // NumberCeoff
        int numCoeff = 0;
        int trailingOnes = 0;

        int token = 0;

        GetCoeffNum_TrailingOnes(bs, numCoeff, trailingOnes, token, numCoeff_vlcIdx);

        lumaResidual[y][x].numCoeff = (uint8_t)numCoeff;


        EyerLog("Number Coeff: %d\n", numCoeff);
        // 拖尾系数的符号
        if(numCoeff != 0){
            if(trailingOnes){
                int signValue = bs.bs_read_u(trailingOnes);
                int trailingCount = trailingOnes;
                for(int coeffIdx = 0; coeffIdx < trailingOnes; coeffIdx++){
                    trailingCount--;
                    if((signValue >> trailingCount) & 1){
                        EyerLog("第 %d 个 拖尾系数符号为: %d\n", coeffIdx, -1);
                    }
                    else{
                        EyerLog("第 %d 个 拖尾系数符号为: %d\n", coeffIdx, 1);
                    }
                }
            }

            // levels
            int level = 0;
            if (numCoeff > 10 && trailingOnes < 3) {
                suffixLength = 1;
            }
            else {
                suffixLength = 0;
            }
            for (int k = 0; k <= numCoeff - 1 - trailingOnes; k++) {
                err = get_coeff_level(bs, level, k, trailingOnes, suffixLength);
                if (err < 0) {
                    return err;
                }

                if (suffixLength == 0) {
                    suffixLength = 1;
                }

                if ((abs(level) >(3 << (suffixLength - 1))) && (suffixLength < 6)) {
                    suffixLength++;
                }

                EyerLog("Level: %d\n", level);
            }

            // zerosLeft
            int totalZeros = 0;
            if (numCoeff < max_ceoff_number) {
                err = get_total_zeros(bs, totalZeros, numCoeff - 1);
                if (err < 0) {
                    return err;
                }
            }
            else {
                totalZeros = 0;
            }
            EyerLog("totalZeros: %d\n", totalZeros);


            int runBefore_vlcIdx = 0;
            i = numCoeff - 1;
            int zerosLeft = totalZeros;
            int run = 0;
            if (zerosLeft > 0 && i > 0) {
                do
                {
                    runBefore_vlcIdx = (zerosLeft - 1 < 6 ? zerosLeft - 1 : 6);
                    err = get_run_before(bs, run, runBefore_vlcIdx);
                    if (err < 0) {
                        return err;
                    }
                    zerosLeft -= run;
                    i--;
                } while (zerosLeft != 0 && i != 0);
            }
            else {
                run = 0;
            }

            EyerLog("Run: %d\n", run);
        }

        return err;
    }

    int EyerMacroblock::GetCoeffNum_TrailingOnes(EyerBitStream & bs, int & totleCoeff, int & trailingOnes, int & token, int numCoeffIndex)
    {
        int ret = 0;

        int * lengthTable = nullptr;
        int * codeTable = nullptr;

        int coeffTokenTable_Length[3][4][17] =
        {
            {   // 0702
                { 1, 6, 8, 9,10,11,13,13,13,14,14,15,15,16,16,16,16},
                { 0, 2, 6, 8, 9,10,11,13,13,14,14,15,15,15,16,16,16},
                { 0, 0, 3, 7, 8, 9,10,11,13,13,14,14,15,15,16,16,16},
                { 0, 0, 0, 5, 6, 7, 8, 9,10,11,13,14,14,15,15,16,16},
            },
            {
                { 2, 6, 6, 7, 8, 8, 9,11,11,12,12,12,13,13,13,14,14},
                { 0, 2, 5, 6, 6, 7, 8, 9,11,11,12,12,13,13,14,14,14},
                { 0, 0, 3, 6, 6, 7, 8, 9,11,11,12,12,13,13,13,14,14},
                { 0, 0, 0, 4, 4, 5, 6, 6, 7, 9,11,11,12,13,13,13,14},
            },
            {
                { 4, 6, 6, 6, 7, 7, 7, 7, 8, 8, 9, 9, 9,10,10,10,10},
                { 0, 4, 5, 5, 5, 5, 6, 6, 7, 8, 8, 9, 9, 9,10,10,10},
                { 0, 0, 4, 5, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9,10,10,10},
                { 0, 0, 0, 4, 4, 4, 4, 4, 5, 6, 7, 8, 8, 9,10,10,10},
            },
        };

        int coeffTokenTable_Code[3][4][17] =
        {
            {
                { 1, 5, 7, 7, 7, 7,15,11, 8,15,11,15,11,15,11, 7,4},
                { 0, 1, 4, 6, 6, 6, 6,14,10,14,10,14,10, 1,14,10,6},
                { 0, 0, 1, 5, 5, 5, 5, 5,13, 9,13, 9,13, 9,13, 9,5},
                { 0, 0, 0, 3, 3, 4, 4, 4, 4, 4,12,12, 8,12, 8,12,8},
            },
            {
                { 3,11, 7, 7, 7, 4, 7,15,11,15,11, 8,15,11, 7, 9,7},
                { 0, 2, 7,10, 6, 6, 6, 6,14,10,14,10,14,10,11, 8,6},
                { 0, 0, 3, 9, 5, 5, 5, 5,13, 9,13, 9,13, 9, 6,10,5},
                { 0, 0, 0, 5, 4, 6, 8, 4, 4, 4,12, 8,12,12, 8, 1,4},
            },
            {
                {15,15,11, 8,15,11, 9, 8,15,11,15,11, 8,13, 9, 5,1},
                { 0,14,15,12,10, 8,14,10,14,14,10,14,10, 7,12, 8,4},
                { 0, 0,13,14,11, 9,13, 9,13,10,13, 9,13, 9,11, 7,3},
                { 0, 0, 0,12,11,10, 9, 8,13,12,12,12, 8,12,10, 6,2},
            },
        };

        if(numCoeffIndex < 3){
            lengthTable = &coeffTokenTable_Length[numCoeffIndex][0][0];
            codeTable = &coeffTokenTable_Code[numCoeffIndex][0][0];

            search_for_value_in_2D_table(bs, totleCoeff, trailingOnes, token, lengthTable, codeTable, 17, 4);
        }
        if(numCoeffIndex == 3){
            uint32_t code = bs.bs_read_u(6);
            // EyerLog("Code: %d\n", code);

            uint32_t b = code & 3;
            uint32_t a = code >> 2;

            if (!a && b == 3) {
                b = 0;
            }
            else{
                a++;
            }

            // EyerLog("a: %d, b: %d\n", a, b);

            totleCoeff = a;
            trailingOnes = b;
        }

        return ret;
    }

    int EyerMacroblock::GetNumberCurrent(int x, int y)
    {
        int nC = 0;
        int topIndex = 0;
        int leftIndex = 0;
        int topNum = 0;
        int leftNum = 0;

        bool available_top = false;
        bool available_left = false;

        GetNeighborAvailable(available_top, available_left, topIndex, leftIndex, x, y);

        if(!available_left && !available_top){
            nC = 0;
        }
        else{
            if(available_left){
                leftNum = GetLeftNeighborCoeffNum(leftIndex, x, y);
            }
            if(available_top){
                topNum = GetTopNeighborCoeffNum(topIndex, x, y);
            }

            nC = topNum + leftNum;
            if(available_left && available_top){
                nC++;
                nC = nC >> 1;
            }
            // EyerLog("nC top: %d, left: %d, nC: %d\n", topNum, leftNum, nC);
        }

        return nC;
    }

    int EyerMacroblock::GetTopNeighborCoeffNum(int topIdx, int blockX, int blockY)
    {
        int nzCeoff = 0;
        int target_idx_y = 0;
        if(topIdx == mbIndex){
            target_idx_y = blockY - 1;
            nzCeoff = lumaResidual[target_idx_y][blockX].numCoeff;
        }

        return nzCeoff;
    }

    int EyerMacroblock::GetLeftNeighborCoeffNum(int leftIdx, int blockX, int blockY)
    {
        int nzCeoff = 0;
        int target_idx_x = 0;
        if(leftIdx == mbIndex){
            target_idx_x = blockX - 1;
            nzCeoff = lumaResidual[blockY][target_idx_x].numCoeff;
        }

        return nzCeoff;
    }

    int EyerMacroblock::GetNeighborAvailable(bool & available_top, bool & available_left, int & topIndex, int & leftIndex, int blockIndexX, int blockIndexY){

        int width_in_mb = sps.pic_width_in_mbs_minus1 + 1;
        int height_in_mb = sps.pic_height_in_map_units_minus1 + 1;

        bool left_egde_mb = (mbIndex % width_in_mb == 0);
        bool top_egde_mb = (mbIndex < width_in_mb);

        if(!top_egde_mb){
            available_top = true;
            topIndex = mbIndex - width_in_mb;
        }
        else{
            if(blockIndexY == 0){
                available_top = false;
            }
            else{
                available_top = true;
                topIndex = mbIndex;
            }
        }

        if(!left_egde_mb){
            available_left = true;
            leftIndex = mbIndex - 1;
        }
        else{
            if(blockIndexX == 0){
                available_left = false;
            }
            else{
                available_left = true;
                leftIndex = mbIndex;
            }
        }

        return 0;
    }

    int EyerMacroblock::search_for_value_in_2D_table(EyerBitStream & bs, int &value1, int &value2, int &code, int *lengthTable, int *codeTable, int tableWidth, int tableHeight)
    {
        for(int yIdx = 0; yIdx < tableHeight; yIdx++) {
            for(int xIdx = 0; xIdx < tableWidth; xIdx++) {
                int codeLen = lengthTable[xIdx];
                if (codeLen == 0) {
                    continue;
                }

                code = codeTable[xIdx];

                // EyerLog("Miao!!!!!!! : %d, code: %d\n", bs.bs_peek_u(codeLen), code);

                if(bs.bs_peek_u(codeLen) == code){
                    value1 = xIdx;
                    value2 = yIdx;
                    bs.bs_skip_u(codeLen);

                    EyerLog("value1: %d, value2: %d\n", xIdx, yIdx);
                    goto END;
                }
            }
            lengthTable += tableWidth;
            codeTable += tableWidth;
        }

        EyerLog("Not Found Error\n");
    END:
        return 0;
    }


    int EyerMacroblock::get_coeff_level(EyerBitStream & bs, int &level, int levelIdx, int trailingOnes, int suffixLength)
    {
        int prefixLength = 0, level_prefix = 0, level_suffix = 0;
        int levelSuffixSize = 0, levelCode = 0, i = 0;

        while (!bs.bs_read_u1()) {
            level_prefix++;
        }
        prefixLength = level_prefix + 1;
        if (level_prefix == 14 && suffixLength == 0) {
            levelSuffixSize = 4;
        }
        else if (level_prefix == 15) {
            levelSuffixSize = level_prefix - 3;
        }
        else {
            levelSuffixSize = suffixLength;
        }
        if (levelSuffixSize > 0) {
            // level_suffix = Get_uint_code_num(m_pSODB, m_bypeOffset, m_bitOffset, levelSuffixSize);
            level_suffix = bs.bs_read_u(levelSuffixSize);
        }
        else {
            level_suffix = 0;
        }
        levelCode = (std::min(15, level_prefix) << suffixLength) + level_suffix;
        if (level_prefix >= 15 && suffixLength == 0) {
            levelCode += 15;
        }
        if (level_prefix >= 16) {
            levelCode += (1 << (level_prefix - 3)) - 4096;
        }
        if (levelIdx == 0 && trailingOnes < 3) {
            levelCode += 2;
        }

        if (levelCode % 2 == 0) {
            level = (levelCode + 2) >> 1;
        }
        else {
            level = (-levelCode - 1) >> 1;
        }

        return 0;
    }


    int EyerMacroblock::get_total_zeros(EyerBitStream & bs, int & totalZeros, int totalZeros_vlcIdx)
    {
        int totalZerosTable_Length[15][16] =
            {
                { 1,3,3,4,4,5,5,6,6,7,7,8,8,9,9,9 },
                { 3,3,3,3,3,4,4,4,4,5,5,6,6,6,6 },
                { 4,3,3,3,4,4,3,3,4,5,5,6,5,6 },
                { 5,3,4,4,3,3,3,4,3,4,5,5,5 },
                { 4,4,4,3,3,3,3,3,4,5,4,5 },
                { 6,5,3,3,3,3,3,3,4,3,6 },
                { 6,5,3,3,3,2,3,4,3,6 },
                { 6,4,5,3,2,2,3,3,6 },
                { 6,6,4,2,2,3,2,5 },
                { 5,5,3,2,2,2,4 },
                { 4,4,3,3,1,3 },
                { 4,4,2,1,3 },
                { 3,3,1,2 },
                { 2,2,1 },
                { 1,1 }
            };

        int totalZerosTable_Code[15][16] =
            {
                { 1,3,2,3,2,3,2,3,2,3,2,3,2,3,2,1 },
                { 7,6,5,4,3,5,4,3,2,3,2,3,2,1,0 },
                { 5,7,6,5,4,3,4,3,2,3,2,1,1,0 },
                { 3,7,5,4,6,5,4,3,3,2,2,1,0 },
                { 5,4,3,7,6,5,4,3,2,1,1,0 },
                { 1,1,7,6,5,4,3,2,1,1,0 },
                { 1,1,5,4,3,3,2,1,1,0 },
                { 1,1,1,3,3,2,2,1,0 },
                { 1,0,1,3,2,1,1,1, },
                { 1,0,1,3,2,1,1, },
                { 0,1,1,2,1,3 },
                { 0,1,1,1,1 },
                { 0,1,1,1 },
                { 0,1,1 },
                { 0,1 }
            };

        int err = 0;
        int idx2 = 0;
        int idx1 = 0;
        int *lengthTable = &totalZerosTable_Length[totalZeros_vlcIdx][0];
        int *codeTable = &totalZerosTable_Code[totalZeros_vlcIdx][0];
        err = search_for_value_in_2D_table(bs, totalZeros, idx1, idx2, lengthTable, codeTable, 16, 1);
        if (err < 0) {
            return err;
        }

        return 0;
    }


    int EyerMacroblock::get_run_before(EyerBitStream & bs, int & runBefore, int runBefore_vlcIdx)
    {
        int runBeforeTable_Length[15][16] =
            {
                { 1,1 },
                { 1,2,2 },
                { 2,2,2,2 },
                { 2,2,2,3,3 },
                { 2,2,3,3,3,3 },
                { 2,3,3,3,3,3,3 },
                { 3,3,3,3,3,3,3,4,5,6,7,8,9,10,11 }
            };

        int runBeforeTable_Code[15][16] =
            {
                { 1,0 },
                { 1,1,0 },
                { 3,2,1,0 },
                { 3,2,1,1,0 },
                { 3,2,3,2,1,0 },
                { 3,0,1,3,2,5,4 },
                { 7,6,5,4,3,2,1,1,1,1,1,1,1,1,1 }
            };


        int idx1 = 0;
        int idx2 = 0, err = 0;
        int *lengthTable = &runBeforeTable_Length[runBefore_vlcIdx][0];
        int *codeTable = &runBeforeTable_Code[runBefore_vlcIdx][0];
        err = search_for_value_in_2D_table(bs, runBefore, idx1, idx2, lengthTable, codeTable, 16, 1);
        if (err < 0)
        {
            return err;
        }

        return 0;
    }
}