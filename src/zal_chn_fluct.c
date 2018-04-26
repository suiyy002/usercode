/***************************Copyright(c)***********************************
**                Guangzhou ZHIYUAN electronics Co.,LTD.
**
**                      http://www.zlg.cn
**
**-------------File Info---------------------------------------------------
**File Name:            zal_chn_fluct.c
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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "zal_chn_fluct.h"


ZalVoid fluctStdRmsInput( ZalInt32 phase, ZalFloat halfRms, ZalChnFluctInfo *pFluctInfo )
{
    pFluctInfo->stdRmsSum[ phase ] += halfRms;

    if( phase == 0 ){
        pFluctInfo->stdRmsCount++;
    }
}


ZalFloat fluctStdRmsGet( ZalInt32 phase, ZalChnFluctInfo *pFluctInfo )
{
    return pFluctInfo->stdRmsSum[ phase ]/pFluctInfo->stdRmsCount;
}


ZalVoid fluctStdRmsReset( ZalChnFluctInfo *pFluctInfo )
{
    ZalInt32 i;

    for(i=0; i<pFluctInfo->nPhases; i++)
    {
        pFluctInfo->stdRmsSum[i] = 0.0f;
    }

    pFluctInfo->stdRmsCount = 0;
}


/************************************************************************/
/* ������Ϣ����                                                         */
/************************************************************************/
ZalVoid zalChnFluctReset( ZalChnFluctInfo *pFluctInfo )
{
    ZalInt32 i = 0;

    for (i=0; i<pFluctInfo->nPhases; i++)
    {
        pFluctInfo->fluct[i]              = 0.0f;
        pFluctInfo->count[i]              = 0;
        pFluctInfo->fluct_r[i]              = 0;
        pFluctInfo->steadyStateMaxlast[i] = 0;

        if(pFluctInfo->pst3_count[i] == 12)
        {
            pFluctInfo->pst3_sum[i]            = 0.0f;
            pFluctInfo->pst3_count[i]       = 0;
            pFluctInfo->cmp_4000_count[i]   = 0;
        }
    }

    fluctStdRmsReset(pFluctInfo);
}


/************************************************************************/
/* ���겨��������ʼ��                                                   */
/************************************************************************/
ZalVoid zalChnFluctInit( ZalChnFluctInfo *pFluctInfo, ZalInt32 nPhases )
{
    ZalInt32 i;

    pFluctInfo->nPhases = nPhases;

    pFluctInfo->count                = malloc(sizeof(ZalInt32)*nPhases);
    pFluctInfo->steadyState            = malloc(sizeof(ZalInt32)*nPhases);
    pFluctInfo->steadyStateMax        = malloc(sizeof(ZalInt32)*nPhases);
    pFluctInfo->steadyStateMaxlast    = malloc(sizeof(ZalInt32)*nPhases);
    pFluctInfo->fluct_r                = malloc(sizeof(ZalInt32)*nPhases);
    pFluctInfo->fluct                = malloc(sizeof(ZalFloat)*nPhases);
    pFluctInfo->rmsLastMax            = malloc(sizeof(ZalFloat)*nPhases);
    pFluctInfo->rmsLastMin            = malloc(sizeof(ZalFloat)*nPhases);
    pFluctInfo->stdRmsSum            = malloc(sizeof(ZalFloat)*nPhases);
    pFluctInfo->ulast                = malloc(sizeof(ZalFloat)*nPhases);
    pFluctInfo->pst3_sum            = malloc(sizeof(ZalFloat)*nPhases);
    pFluctInfo->pst3_count            = malloc(sizeof(ZalFloat)*nPhases);
    pFluctInfo->cmp_4000_count        = malloc(sizeof(ZalFloat)*nPhases);

    for (i=0; i<nPhases; i++)
    {
        pFluctInfo->fluct[i]                = 0.0f;
        pFluctInfo->count[i]                = 0;
        pFluctInfo->fluct_r[i]                = 0;
        pFluctInfo->steadyStateMaxlast[i]    = 0;
        pFluctInfo->pst3_sum[i]                = 0.0f;
        pFluctInfo->pst3_count[i]            = 0;
        pFluctInfo->cmp_4000_count[i]        = 0;
    }

    fluctStdRmsReset(pFluctInfo);
}


/************************************************************************/
/* ���겨������                                                         */
/************************************************************************/
ZalVoid zalChnFluctCalc( ZalInt32 iPhase, ZalFloat *pHalfRms, ZalChnFluctInfo *pFluctInfo, ZalInt32 halfRmsCounts )
{
    ZalInt32 i;
    ZalFloat t;

    for(i=0; i<halfRmsCounts; i++)
    {

        fluctStdRmsInput(iPhase, pHalfRms[i], pFluctInfo);

        // ��ʼֵ
        if ( pFluctInfo->count[ iPhase ] == 0 )
        {
            pFluctInfo->rmsLastMax[ iPhase ] = pFluctInfo->rmsLastMin[ iPhase ] = pHalfRms[i];
            ++pFluctInfo->count[ iPhase ];
            pFluctInfo->steadyStateMaxlast[ iPhase ] = pFluctInfo->steadyStateMax[ iPhase ];
            pFluctInfo->steadyStateMax[ iPhase ] = 0;
            pFluctInfo->ulast[ iPhase ] = pHalfRms[i];
            return ;
        }

        t = pHalfRms[i] - pFluctInfo->ulast[ iPhase ];

        if ( t<0 )
        {
            t = -t;
        }
        t /= pHalfRms[i];

        if ( t<0.002 )
        {
            if ( pFluctInfo->steadyState[ iPhase ] == 10 )    /* 100ms ��ƽ����һ�� */
            {
                pFluctInfo->fluct_r[ iPhase ]++;
            }

            pFluctInfo->steadyState[ iPhase ]++;

        } else {
            if ( pFluctInfo->steadyState[ iPhase ] > pFluctInfo->steadyStateMax[ iPhase ] )
            {
                pFluctInfo->steadyStateMax[ iPhase ] = pFluctInfo->steadyState[ iPhase ];
            }
            pFluctInfo->steadyState[ iPhase ] = 0;
        }

        // ��¼������Сֵ
        if( pHalfRms[i] > pFluctInfo->rmsLastMax[ iPhase ] )
        {
            pFluctInfo->rmsLastMax[ iPhase ] = pHalfRms[i];
        }
        if( pHalfRms[i] < pFluctInfo->rmsLastMin[ iPhase ] )
        {
            pFluctInfo->rmsLastMin[ iPhase ] = pHalfRms[i];
        }

        // �����ӳ�һ������ֵ
        if( ++pFluctInfo->count[ iPhase ] >= 25*4*120 )
        {
            ZalFloat rms   = fluctStdRmsGet( iPhase, pFluctInfo );
            ZalFloat fluct = (pFluctInfo->rmsLastMax[ iPhase ] - pFluctInfo->rmsLastMin[ iPhase ]) *100.0f /rms;

            if ( fluct > 80 )
            {
                fluct = -1;
            } else if ( rms < 1.0f )
            {
                // ��ѹС��1Vʱ����0.
                fluct = 0.0f;
            } else if (  rms < 120  )
            {
                if ( fluct < 0.7 )
                {
                    fluct *= 0.4/0.47;
                } else if ( t < 1.3 )
                {
                    fluct *= 0.402/0.42;
                }
            }

            if( fluct > pFluctInfo->fluct[ iPhase ] )
            {
                pFluctInfo->fluct[ iPhase ] = fluct;
            }

            pFluctInfo->count[ iPhase ] = 0;

            if( iPhase == (pFluctInfo->nPhases-1) )
            {
                fluctStdRmsReset( pFluctInfo );
            }

        }
    }

}


/************************************************************************/
/* ʹ�����䷴�Ʋ���                                                     */
/************************************************************************/
ZalVoid zalFluctFromFlicker( ZalChnFluctInfo *pFluctInfo, ZalFloat *pPst, /*ZalFloat *pPlt,*/
                             /*ZalFloat *pVolff,*/ ZalFloat *pVolFluct, ZalFloat *pFluctChanges )
{
    ZalInt32 i;
    ZalFloat pst;
    ZalFloat t;

    for(i=0; i<pFluctInfo->nPhases; i++)
    {
        pst = pPst[i];
        t   =  pFluctInfo->fluct[i] / pst;

        pFluctInfo->pst3_count[i]++;

#if 0
        if ( t < 0.3 && pFluctInfo->steadyStateMaxlast[i] < 2 )
        {
            pVolFluct[i] =  pFluctInfo->fluct[i] * ( 1.0 - 0.12);
        } else if ( t > 1.5 && t < 3.3 && pFluctInfo->steadyStateMaxlast[i] < 5 && pFluctInfo->steadyStateMaxlast[i] != 0 ) // 4000
        {
            pVolFluct[i] = 2.4*pst;
            pFluctInfo->fluct_r[i] = 4000;
        } else if ( t < 1.0 && pFluctInfo->steadyStateMaxlast[i] < 15  && pFluctInfo->steadyStateMaxlast[i] != 0 ) // 1620
        {
            pVolFluct[i] = 0.402*pst;
            pFluctInfo->fluct_r[i] = 1620;
        } else if ( (t<1.2) && (pFluctInfo->steadyStateMaxlast[i]<100)  && pFluctInfo->steadyStateMaxlast[i] != 0 ) // 110 ��̬����0.2�� С�� 1��
        {
            pVolFluct[i] = 0.725*pst;
            pFluctInfo->fluct_r[i] = 110;
        } else if ( t < 1.3 && (pFluctInfo->steadyStateMaxlast[i]<800)  && pFluctInfo->steadyStateMaxlast[i] != 0 ) // 39 ��̬ʱ�䳬�� 1�� С��8��
        {
            pVolFluct[i] = 0.906*pst;
            pFluctInfo->fluct_r[i] = 39;
        } else if ( t < 1.9 && pFluctInfo->steadyStateMaxlast[i]< 2500  && pFluctInfo->steadyStateMaxlast[i] != 0 ) // 7  ��̬����8��С��25��
        {
            pVolFluct[i] = 1.459*pst;
            pFluctInfo->fluct_r[i] = 7;
        } else if ( t < 3 && pFluctInfo->steadyStateMaxlast[i]<4000  && pFluctInfo->steadyStateMaxlast[i] != 0 ) // 2������̬����25�룬С��40�룬��Ϊ��2
        {
            pVolFluct[i] = 2.211*pst;
            pFluctInfo->fluct_r[i] = 2;
        } else if ( t < 3.60 && pFluctInfo->steadyStateMaxlast[i]>5000  && pFluctInfo->steadyStateMaxlast[i] != 0 ) // 1 ��̬ʱ�䳬�� 50 ��
        {
            pVolFluct[i] = 2.724*pst;
            pFluctInfo->fluct_r[i] = 1;
        } else {
            pVolFluct[i] =  pFluctInfo->fluct[i] * ( 1.0 - 0.12);
            pFluctInfo->fluct_r[i] /= 10; /* 10�����ڵĴ���,�������ɣ���/���� */
        }

#endif

        if ( t > 1.5 && t < 3.3 && pFluctInfo->steadyStateMaxlast[i] < 5 ) // 4000
        {
            /** @bief ��������˵��
            *
            * ��4000Ƶ�ȵ��Ĳ��ν����⣬����ѹ��Чֵ�����Ƶ�ѹ�Ĳ�ֵ���Ų�������������������
            * �����������У�����ʹ�õ�ѹ��Чֵ��Ϊ�����ı��Ƶ�ѹ������Чֵƫ��ʱ��PST�ļ�������ƫС����֮PST������
            * ��ʹ�ò��������������������Ĺ�ϵ���������ļ�����������������
            */

            // �Զ�����ֵ����������pst����7.5����Ӧ��������ֵԼΪ8.0��
//            if( pPst[i] > 7.5 )
//            {
//                pPst[i] = pPst[i] + (50.8439*pVolff[i] - 56.5747);
//            }

            // ���ڳ���������ֻ����4000Ƶ�Ƚ�������
            pFluctInfo->pst3_sum[i]  += pPst[i] * pPst[i] * pPst[i];
//            pFluctInfo->cmp_4000_count[i]++;

//            if( pFluctInfo->cmp_4000_count[i] == 12 )
//            {
//                pPlt[i] = powf( pFluctInfo->pst3_sum[i]/pFluctInfo->pst3_count[i], 1/3.0f);
//            }

            pVolFluct[ i ] = 2.343 * pPst[i];
            pFluctInfo->fluct_r[i] = 4000 * 10;

        }else {
            pVolFluct[i] =  pFluctInfo->fluct[i];
            pFluctInfo->fluct_r[i] *= 2; /* 10�����ڵĴ��� */
        }

        //  ��ѹ�䶯����
        pFluctChanges[i]        = (ZalFloat) pFluctInfo->fluct_r[i];

        pFluctInfo->fluct_r[i]  = 0;

        pFluctInfo->steadyStateMaxlast[i] = 0;

    }

}


/************************************************************************/
/* ���겨����������                                                     */
/************************************************************************/
ZalVoid zalChnFluctDeinit( ZalChnFluctInfo *pFluctInfo)
{
    free(pFluctInfo->count);
    free(pFluctInfo->steadyState);
    free(pFluctInfo->steadyStateMax);
    free(pFluctInfo->steadyStateMaxlast);
    free(pFluctInfo->fluct_r);
    free(pFluctInfo->fluct);
    free(pFluctInfo->rmsLastMax);
    free(pFluctInfo->rmsLastMin);
    free(pFluctInfo->stdRmsSum);
    free(pFluctInfo->ulast);
    free( pFluctInfo->pst3_sum );
    free( pFluctInfo->pst3_count );
    free( pFluctInfo->cmp_4000_count );
}
