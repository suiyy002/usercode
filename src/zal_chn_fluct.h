/***************************Copyright(c)***********************************
**                Guangzhou ZHIYUAN electronics Co.,LTD.
**
**                      http://www.zlg.cn
**
**-------------File Info---------------------------------------------------
**File Name:            zal_chn_fluct.h
**Latest modified Date:
**Latest Version:
**Description:
**
**-------------------------------------------------------------------------
**Created By:           Liao Jingsen
**Created Date:         2017-4-28
**Version:              v1.0.0
**Description:
**
**-------------------------------------------------------------------------
**Modified By:
**Modified Date:
**Version:
**Description:
**
**************************************************************************/

/**
* @addtogroup grp_zal_fluct_and_flick
* @brief 国标波动计算
* @{
*/

#ifndef __ZAL_ZalChnFluct_H__
#define __ZAL_ZalChnFluct_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t ZalInt32;
typedef float ZalFloat;
typedef uint64_t ZalInt64;
typedef void ZalVoid;

/**@brief 国标波动计算结构体 */
typedef struct{

    ZalInt32   nPhases;                    /**< @brief 相数 */
    ZalInt32   *count;                    /**< @brief 次数 */
    ZalInt32   *steadyState;            /**< @brief 当前次的持续时间 */
    ZalInt32   *steadyStateMax;            /**< @brief 当前最大值持续时间 */
    ZalInt32   *steadyStateMaxlast;        /**< @brief 上一次最大值持续时间 */
    ZalInt32   *fluct_r;                /**< @brief 频度:次/分钟 */
    ZalInt32   stdRmsCount;                /**< @brief 有效值统计次数 */
    ZalFloat   *rmsLastMax;                /**< @brief 最大有效值 */
    ZalFloat   *rmsLastMin;                /**< @brief 最小有效值 */
    ZalFloat   *stdRmsSum;                /**< @brief 有效值统计和 */
    ZalFloat   *fluct;                    /**< @brief 波动量 */
    ZalFloat   *ulast;                    /**< @brief 上一次电压值 */

    ZalFloat   *pst3_sum;                /**< @brief pst的和 */
    ZalInt32   *pst3_count;                /**< @brief pst统计次数 */
    ZalInt32   *cmp_4000_count;            /**< @brief 波动量为4000时的次数统计 */

}ZalChnFluctInfo;


/**@brief 波动计算结果 */
typedef struct{

    ZalInt32 valid;        /**< @brief 波动有效 */
    ZalFloat dc;        /**< @brief 相对的稳态电压变动 */
    ZalInt32 change;    /**< @brief 当前统计周期内的变动次数 */
    ZalInt64 start;        /**< @brief 波动统计开始时间 */
    ZalInt64 end;        /**< @brief 波动统计结束时间 */

}ZalCHNFluct;


/**
* @brief    国标波动计算信息初始化
*
* @param[out]    pFluctInfo        初始化波动相关计算信息
* @param[in]    nPhases         电压相数
*/
ZalVoid zalChnFluctInit( ZalChnFluctInfo *pFluctInfo, ZalInt32 nPhases );



/**
* @brief    使用中国标准计算波动
*
* @param[in]        iPhase            第几相
* @param[in]        pHalfRms         当前相得半波有效值
* @param[in,out]    pFluctInfo        波动相关计算信息
* @param[in]        halfRmsCounts   半波有效值次数
*/
ZalVoid zalChnFluctCalc( ZalInt32 iPhase, ZalFloat *pHalfRms,
                         ZalChnFluctInfo *pFluctInfo, ZalInt32 halfRmsCounts );



/**
* @brief    使用闪变反推波动
*
* @param[in]    pFluctInfo        波动相关计算信息
* @param[in]    pPst             短闪变值
* @param[in]    pPlt             长闪变值
* @param[in]    pVolff            电压波形因数，用于在4000cmps时对闪变进行修正的参考值
* @param[out]    pVolFluct        波动值
* @param[out]    pFluctChanges   波动频度
*/
ZalVoid zalFluctFromFlicker( ZalChnFluctInfo *pFluctInfo,
     ZalFloat *pPst, /*ZalFloat *pPlt, ZalFloat *pVolFormFactor,*/
     ZalFloat *pVolFluct, ZalFloat *pFluctChanges );



/**
* @brief 波动信息重置
* @param[in,out]  pFluctInfo    波动相关计算信息
*/
ZalVoid zalChnFluctReset( ZalChnFluctInfo *pFluctInfo );



/**
* @brief 波动计算信息析构
*
* @param[in]    pFluctInfo    释放对象
* @return                    无
*/
ZalVoid zalChnFluctDeinit( ZalChnFluctInfo *pFluctInfo);


#ifdef __cplusplus
}
#endif
#endif


/** @} */
