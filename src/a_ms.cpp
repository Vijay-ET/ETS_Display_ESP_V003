/***********************************************************
 *
 * * a_graph.cpp
 * SYSTART GmbH
 * MIZ, 09.07.2021
 *
 ***********************************************************/

/***********************************************************
 * Includes
 ***********************************************************/
#include "a_ms.h"
#include "Serial_prints.h"

/***********************************************************
 * Defines
 ***********************************************************/

/***********************************************************
 * Globale Variablen
 ***********************************************************/


/***********************************************************
 * Lokale Funktionen
 ***********************************************************/


/***********************************************************
 * Funktionsdefinitionen
 ***********************************************************/

/**
 * Check if Master/Slave is active
 * @param Pointer to Master/Slave variable. Participant count gets written to
 * @return bool: True if Master/Slave active
 */
bool IsMasterSlaveActive(uint8_t * ms_count)
{
	uint8_t ms_cnt = 0;

	if(NtParam.msconfig.ms_mode)
	{
		ms_cnt = 1;

		// if own number is not 0 and MS0 exists --> some other device in Master/Slave mode
		if(NtParam.msconfig.Nr !=0 && NtParam.msconfig.ms_net_info.MS0)
			ms_cnt++;
		if(NtParam.msconfig.Nr !=1 && NtParam.msconfig.ms_net_info.MS1)
			ms_cnt++;
        if(NtParam.msconfig.Nr !=2 && NtParam.msconfig.ms_net_info.MS2)
			ms_cnt++;
        if(NtParam.msconfig.Nr !=3 && NtParam.msconfig.ms_net_info.MS3)
			ms_cnt++;
        if(NtParam.msconfig.Nr !=4 && NtParam.msconfig.ms_net_info.MS4)
			ms_cnt++;
        if(NtParam.msconfig.Nr !=5 && NtParam.msconfig.ms_net_info.MS5)
			ms_cnt++;
        if(NtParam.msconfig.Nr !=6 && NtParam.msconfig.ms_net_info.MS6)
			ms_cnt++;
        if(NtParam.msconfig.Nr !=7 && NtParam.msconfig.ms_net_info.MS7)
			ms_cnt++;
        if(NtParam.msconfig.Nr !=8 && NtParam.msconfig.ms_net_info.MS8)
			ms_cnt++;
        if(NtParam.msconfig.Nr !=9 && NtParam.msconfig.ms_net_info.MS9)
			ms_cnt++;
        if(NtParam.msconfig.Nr !=10 && NtParam.msconfig.ms_net_info.MS10)
			ms_cnt++;
        if(NtParam.msconfig.Nr !=11 && NtParam.msconfig.ms_net_info.MS11)
			ms_cnt++;
        if(NtParam.msconfig.Nr !=12 && NtParam.msconfig.ms_net_info.MS12)
			ms_cnt++;
        if(NtParam.msconfig.Nr !=13 && NtParam.msconfig.ms_net_info.MS13)
			ms_cnt++;
        if(NtParam.msconfig.Nr !=14 && NtParam.msconfig.ms_net_info.MS14)
			ms_cnt++;
        if(NtParam.msconfig.Nr !=15 && NtParam.msconfig.ms_net_info.MS15)
			ms_cnt++;
	}

	if(ms_count != NULL)
		*ms_count = ms_cnt;

	return (ms_cnt >=2);
}

/**
 * Get the sum of parallel or serial Master/Slave values
 * @param 1 pointer to voltage float
 * @param 2 pointer to current float
 * @param 3 pointer to power float
 * @param 4 pointer to resistance float
 * @return void
 */
void GetMonitorTotalValues(float * u, float * i, float * p, float * r)
{
	float val_mstotal_u = 0, val_mstotal_i = 0, val_mstotal_p = 0, val_mstotal_r = 0;
	float val_mstotal_r_1 = 0;

	if(NtParam.msconfig.ms_mode == MS_Serial)
	{
		if(NtParam.msconfig.ms_net_info.MS0)
		{
			val_mstotal_u += NtParam.msmonitor[0].U;
			val_mstotal_r += NtParam.msmonitor[0].R;
			val_mstotal_p += NtParam.msmonitor[0].P;
		}
		if(NtParam.msconfig.ms_net_info.MS1)
		{
			val_mstotal_u += NtParam.msmonitor[1].U;
			val_mstotal_r += NtParam.msmonitor[1].R;
			val_mstotal_p += NtParam.msmonitor[1].P;
		}
		if(NtParam.msconfig.ms_net_info.MS2)
		{
			val_mstotal_u += NtParam.msmonitor[2].U;
			val_mstotal_r += NtParam.msmonitor[2].R;
			val_mstotal_p += NtParam.msmonitor[2].P;
		}
		if(NtParam.msconfig.ms_net_info.MS3)
		{
			val_mstotal_u += NtParam.msmonitor[3].U;
			val_mstotal_r += NtParam.msmonitor[3].R;
			val_mstotal_p += NtParam.msmonitor[3].P;
		}
		if(NtParam.msconfig.ms_net_info.MS4)
		{
			val_mstotal_u += NtParam.msmonitor[4].U;
			val_mstotal_r += NtParam.msmonitor[4].R;
			val_mstotal_p += NtParam.msmonitor[4].P;
		}
		if(NtParam.msconfig.ms_net_info.MS5)
		{
			val_mstotal_u += NtParam.msmonitor[5].U;
			val_mstotal_r += NtParam.msmonitor[5].R;
			val_mstotal_p += NtParam.msmonitor[5].P;
		}
		if(NtParam.msconfig.ms_net_info.MS6)
		{
			val_mstotal_u += NtParam.msmonitor[6].U;
			val_mstotal_r += NtParam.msmonitor[6].R;
			val_mstotal_p += NtParam.msmonitor[6].P;
		}
		if(NtParam.msconfig.ms_net_info.MS7)
		{
			val_mstotal_u += NtParam.msmonitor[7].U;
			val_mstotal_r += NtParam.msmonitor[7].R;
			val_mstotal_p += NtParam.msmonitor[7].P;
		}
		if(NtParam.msconfig.ms_net_info.MS8)
		{
			val_mstotal_u += NtParam.msmonitor[8].U;
			val_mstotal_r += NtParam.msmonitor[8].R;
			val_mstotal_p += NtParam.msmonitor[8].P;
		}
		if(NtParam.msconfig.ms_net_info.MS9)
		{
			val_mstotal_u += NtParam.msmonitor[9].U;
			val_mstotal_r += NtParam.msmonitor[9].R;
			val_mstotal_p += NtParam.msmonitor[9].P;
		}
		if(NtParam.msconfig.ms_net_info.MS10)
		{
			val_mstotal_u += NtParam.msmonitor[10].U;
			val_mstotal_r += NtParam.msmonitor[10].R;
			val_mstotal_p += NtParam.msmonitor[10].P;
		}
		if(NtParam.msconfig.ms_net_info.MS11)
		{
			val_mstotal_u += NtParam.msmonitor[11].U;
			val_mstotal_r += NtParam.msmonitor[11].R;
			val_mstotal_p += NtParam.msmonitor[11].P;
		}
		if(NtParam.msconfig.ms_net_info.MS12)
		{
			val_mstotal_u += NtParam.msmonitor[12].U;
			val_mstotal_r += NtParam.msmonitor[12].R;
			val_mstotal_p += NtParam.msmonitor[12].P;
		}
		if(NtParam.msconfig.ms_net_info.MS13)
		{
			val_mstotal_u += NtParam.msmonitor[13].U;
			val_mstotal_r += NtParam.msmonitor[13].R;
			val_mstotal_p += NtParam.msmonitor[13].P;
		}
		if(NtParam.msconfig.ms_net_info.MS14)
		{
			val_mstotal_u += NtParam.msmonitor[14].U;
			val_mstotal_r += NtParam.msmonitor[14].R;
			val_mstotal_p += NtParam.msmonitor[14].P;
		}

		val_mstotal_i = NtParam.monitor.I;

	}
	else if(NtParam.msconfig.ms_mode == MS_Parallel)
	{

		if(NtParam.msconfig.ms_net_info.MS0)
		{
			val_mstotal_i += NtParam.msmonitor[0].I;
			val_mstotal_r_1 += 1/NtParam.msmonitor[0].R;
			val_mstotal_p += NtParam.msmonitor[0].P;
		}
		if(NtParam.msconfig.ms_net_info.MS1)
		{
			val_mstotal_i += NtParam.msmonitor[1].I;
			val_mstotal_r_1 += 1/NtParam.msmonitor[1].R;
			val_mstotal_p += NtParam.msmonitor[1].P;
		}
		if(NtParam.msconfig.ms_net_info.MS2)
		{
			val_mstotal_i += NtParam.msmonitor[2].I;
			val_mstotal_r_1 += 1/NtParam.msmonitor[2].R;
			val_mstotal_p += NtParam.msmonitor[2].P;
		}
		if(NtParam.msconfig.ms_net_info.MS3)
		{
			val_mstotal_i += NtParam.msmonitor[3].I;
			val_mstotal_r_1 += 1/NtParam.msmonitor[3].R;
			val_mstotal_p += NtParam.msmonitor[3].P;
		}
		if(NtParam.msconfig.ms_net_info.MS4)
		{
			val_mstotal_i += NtParam.msmonitor[4].I;
			val_mstotal_r_1 += 1/NtParam.msmonitor[4].R;
			val_mstotal_p += NtParam.msmonitor[4].P;
		}
		if(NtParam.msconfig.ms_net_info.MS5)
		{
			val_mstotal_i += NtParam.msmonitor[5].I;
			val_mstotal_r_1 += 1/NtParam.msmonitor[5].R;
			val_mstotal_p += NtParam.msmonitor[5].P;
		}
		if(NtParam.msconfig.ms_net_info.MS6)
		{
			val_mstotal_i += NtParam.msmonitor[6].I;
			val_mstotal_r_1 += 1/NtParam.msmonitor[6].R;
			val_mstotal_p += NtParam.msmonitor[6].P;
		}
		if(NtParam.msconfig.ms_net_info.MS7)
		{
			val_mstotal_i += NtParam.msmonitor[7].I;
			val_mstotal_r_1 += 1/NtParam.msmonitor[7].R;
			val_mstotal_p += NtParam.msmonitor[7].P;
		}
		if(NtParam.msconfig.ms_net_info.MS8)
		{
			val_mstotal_i += NtParam.msmonitor[8].I;
			val_mstotal_r_1 += 1/NtParam.msmonitor[8].R;
			val_mstotal_p += NtParam.msmonitor[8].P;
		}
		if(NtParam.msconfig.ms_net_info.MS9)
		{
			val_mstotal_i += NtParam.msmonitor[9].I;
			val_mstotal_r_1 += 1/NtParam.msmonitor[9].R;
			val_mstotal_p += NtParam.msmonitor[9].P;
		}
		if(NtParam.msconfig.ms_net_info.MS10)
		{
			val_mstotal_i += NtParam.msmonitor[10].I;
			val_mstotal_r_1 += 1/NtParam.msmonitor[10].R;
			val_mstotal_p += NtParam.msmonitor[10].P;
		}
		if(NtParam.msconfig.ms_net_info.MS15)
		{
			val_mstotal_i += NtParam.msmonitor[11].I;
			val_mstotal_r_1 += 1/NtParam.msmonitor[11].R;
			val_mstotal_p += NtParam.msmonitor[11].P;
		}
		if(NtParam.msconfig.ms_net_info.MS15)
		{
			val_mstotal_i += NtParam.msmonitor[12].I;
			val_mstotal_r_1 += 1/NtParam.msmonitor[12].R;
			val_mstotal_p += NtParam.msmonitor[12].P;
		}
		if(NtParam.msconfig.ms_net_info.MS15)
		{
			val_mstotal_i += NtParam.msmonitor[13].I;
			val_mstotal_r_1 += 1/NtParam.msmonitor[13].R;
			val_mstotal_p += NtParam.msmonitor[13].P;
		}
		if(NtParam.msconfig.ms_net_info.MS15)
		{
			val_mstotal_i += NtParam.msmonitor[14].I;
			val_mstotal_r_1 += 1/NtParam.msmonitor[14].R;
			val_mstotal_p += NtParam.msmonitor[14].P;
		}
		if(NtParam.msconfig.ms_net_info.MS15)
		{
			val_mstotal_i += NtParam.msmonitor[15].I;
			val_mstotal_r_1 += 1/NtParam.msmonitor[15].R;
			val_mstotal_p += NtParam.msmonitor[15].P;
		}

		val_mstotal_u = NtParam.monitor.U;
		val_mstotal_r = 1/val_mstotal_r_1;
	}
	else //MS_Independant?--------------------------------------------------------------------
	{
		val_mstotal_u = NtParam.monitor.U;
		val_mstotal_i = NtParam.monitor.I;
		val_mstotal_p = NtParam.monitor.P;
		val_mstotal_r = NtParam.monitor.R;
	}


	//müssen diese Werte noch zurückgegeben werden?--------------------------------------------------
	*u = val_mstotal_u;
	*i = val_mstotal_i;
	*p = val_mstotal_p;
	*r = val_mstotal_r;

}