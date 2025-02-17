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
#include "a_graph.h"

#include <stddef.h>
#include "main.h"
#include "a_ws_com.h"
#include "a_ms.h"
#include "Serial_prints.h"

/***********************************************************
 * Defines
 ***********************************************************/

/***********************************************************
 * Globale Variablen
 ***********************************************************/
// T_Kennlinie kennlinieRaw;
xyPlot_t xyPlot;
xyPlot_t* xyplot = &xyPlot; // to keep the -> operator. copied from ETS-Display
Point_t markerPos;

/***********************************************************
 * Lokale Funktionen
 ***********************************************************/
void update_numberOfQuadrants(void);
void calculate_axis_data(void);
void calculate_plot_parameters(void);
void calculate_plot_parameters(void);
void calculate_plot_data(void);
void calculate_marker_pos(void);

void send_XYParam(void);
void send_XYMarker(void);
void send_XYData(void);

/***********************************************************
 * Funktionsdefinitionen
 ***********************************************************/

/**
 * Init graph parameters and calculate plot data
 * @param void
 * @return void
 */
void init_XY_Graph(void)
{
	xyPlot.size.x = 784;
	xyPlot.size.y = 286;
	xyPlot.plotParameters.xAxisUnit = AxisUnit_U;
	xyplot->plotDataX = xyplot->plotData[AxisUnit_U];
	xyplot->plotDataY = xyplot->plotData[AxisUnit_I];

	// update_xy(GRAPH_SEND_NONE);		// initial graph calc
}

/**
 * Set origin depending on needed quadrants
 * @param void
 * @return void
 */
void update_numberOfQuadrants(void)
{
	if(xyplot->plotParameters.activeQuadrants == onlyPositive)
	{
		xyplot->originOffset.x = 8;		// % of x-axis from left end
		xyplot->originOffset.y = 0;		// % of y-axis from bottom end
	}
	else if(xyplot->plotParameters.activeQuadrants == y_negative)
	{
		xyplot->originOffset.x = 8;		// % of x-axis from left end
		xyplot->originOffset.y = 50;	// % of y-axis from bottom end
	}
	else if(xyplot->plotParameters.activeQuadrants == x_negative)
	{
		xyplot->originOffset.x = 50;
		xyplot->originOffset.y = 0;
	}
	else if(xyplot->plotParameters.activeQuadrants == xy_negative)
	{
		xyplot->originOffset.x = 50;
		xyplot->originOffset.y = 50;
	}
}

/**
 * Calculate the position and size of the axis
 * @param void
 * @return void
 */
void calculate_axis_data(void)
{
	float originOffsetX = (float)xyplot->originOffset.x / 100;
	float originOffsetY = 1 - ((float)xyplot->originOffset.y / 100);

	/* --- Size of Axes --- */
	xyplot->axisParameters.xAxisSize.x = xyplot->size.x - X_AXIS_GAP_LEFT - X_AXIS_GAP_RIGHT - ARROW_LENGTH;
	xyplot->axisParameters.xAxisSize.y = AXIS_WIDTH;

	xyplot->axisParameters.yAxisSize.x = AXIS_WIDTH;
	xyplot->axisParameters.yAxisSize.y = xyplot->size.y - Y_AXIS_GAP_TOP - Y_AXIS_GAP_BOTTOM - ARROW_LENGTH;

	/* --- Start of Axes --- */
	xyplot->axisParameters.xAxisStart.x = X_AXIS_GAP_LEFT;
	xyplot->axisParameters.yAxisStart.y = Y_AXIS_GAP_TOP + ARROW_LENGTH;

	xyplot->axisParameters.origin.x = xyplot->axisParameters.xAxisStart.x + (originOffsetX * xyplot->axisParameters.xAxisSize.x);
	xyplot->axisParameters.origin.y = xyplot->axisParameters.yAxisStart.y + (originOffsetY * xyplot->axisParameters.yAxisSize.y);

	xyplot->axisParameters.xAxisStart.y = xyplot->axisParameters.origin.y - (AXIS_WIDTH / 2);
	xyplot->axisParameters.yAxisStart.x = xyplot->axisParameters.origin.x - (AXIS_WIDTH / 2);

	/* --- Start of Arrow --- */
	xyplot->axisParameters.xAxisArrowStart.x = xyplot->axisParameters.xAxisStart.x + xyplot->axisParameters.xAxisSize.x;
	xyplot->axisParameters.xAxisArrowStart.y = xyplot->axisParameters.origin.y;

	xyplot->axisParameters.yAxisArrowStart.x = xyplot->axisParameters.origin.x;
	xyplot->axisParameters.yAxisArrowStart.y = xyplot->axisParameters.yAxisStart.y;

	/* --- Axes Title --- */
	xyplot->axisParameters.xAxisTitleLoc.x = xyplot->axisParameters.xAxisArrowStart.x + X_AXIS_TITLE_OFFSETX;
	xyplot->axisParameters.xAxisTitleLoc.y = xyplot->axisParameters.xAxisArrowStart.y + X_AXIS_TITLE_OFFSETY;

	xyplot->axisParameters.xAxisTitleSize.x = X_AXIS_TITLE_SIZEX;
	xyplot->axisParameters.xAxisTitleSize.y = X_AXIS_TITLE_SIZEY;

	xyplot->axisParameters.yAxisTitleLoc.x = xyplot->axisParameters.yAxisArrowStart.x + Y_AXIS_TITLE_OFFSETX;
	xyplot->axisParameters.yAxisTitleLoc.y = xyplot->axisParameters.yAxisArrowStart.y + Y_AXIS_TITLE_OFFSETY;

	xyplot->axisParameters.yAxisTitleSize.x = Y_AXIS_TITLE_SIZEX;
	xyplot->axisParameters.yAxisTitleSize.y = Y_AXIS_TITLE_SIZEY;

	/* --- Maximum Pixel of Axes --- */
	xyplot->axisParameters.AxisLengthPositive.x = xyplot->axisParameters.xAxisArrowStart.x - xyplot->axisParameters.origin.x;
	xyplot->axisParameters.AxisLengthPositive.y = xyplot->axisParameters.origin.y - xyplot->axisParameters.yAxisArrowStart.y;
}

/**
 * Calculate the maximum display values and the voltage/current scale
 * @param void
 * @return void
 */
void calculate_plot_parameters(void)
{
	// Calculate max positions
	if(xyplot->plotParameters.xAxisUnit == AxisUnit_I)
	{
		xyplot->plotParameters.posMaxI = (xyplot->axisParameters.AxisLengthPositive.x * X_AXIS_MAX_LIMIT) / 100;	// relative pos to origin
		xyplot->plotParameters.posMaxU = (xyplot->axisParameters.AxisLengthPositive.y * Y_AXIS_MAX_LIMIT) / 100;	// relative pos to origin
	}
	else
	{
		xyplot->plotParameters.posMaxU = (xyplot->axisParameters.AxisLengthPositive.x * X_AXIS_MAX_LIMIT) / 100;	// relative pos to origin
		xyplot->plotParameters.posMaxI = (xyplot->axisParameters.AxisLengthPositive.y * Y_AXIS_MAX_LIMIT) / 100;	// relative pos to origin
	}

	xyplot->plotParameters.mode = status.mode;

	if((xyplot->plotParameters.mode == VIP) && ((xyplot->plotParameters.totalSetI * xyplot->plotParameters.totalSetU) <= xyplot->plotParameters.totalSetP))
	{
		xyplot->plotParameters.mode = VI;		// P in UIP is set so high, that it act like UI mode
	}

	// Scale U
	if(xyplot->plotParameters.mode == PVSIM || xyplot->plotParameters.mode == USER)
	{
		xyplot->plotParameters.scaleU = (float)xyplot->plotParameters.posMaxU / NtParam.kennlinie.MaxU;
	}
	else if(xyplot->plotParameters.totalSetU == 0)
	{
		xyplot->plotParameters.posMaxU = 0;
		xyplot->plotParameters.scaleU = 0;
	}
	else
	{
		xyplot->plotParameters.scaleU = (float)xyplot->plotParameters.posMaxU / xyplot->plotParameters.totalSetU;
	}

	// Scale I
	if (xyplot->plotParameters.mode == PVSIM || xyplot->plotParameters.mode == USER)
	{
		xyplot->plotParameters.scaleI = (float)xyplot->plotParameters.posMaxI / NtParam.kennlinie.MaxI;
	}
	else if (xyplot->plotParameters.totalSetI==0)
	{
		xyplot->plotParameters.posMaxI = 0;
		xyplot->plotParameters.scaleI = 0;
	}
	else
	{
		xyplot->plotParameters.scaleI = (float)xyplot->plotParameters.posMaxI / xyplot->plotParameters.totalSetI;
	}
}

/**
 * Calculate plot points (red line) depending on mode
 * @param void
 * @return void
 */
void calculate_plot_data(void)
{
	uint16_t n = 0;
	memset(&xyplot->plotData, 0x80, sizeof (xyplot->plotData)); // ervery not used element will be 0x8080 = -32640 -> should never be a coordinate

	if(xyplot->plotParameters.mode == VI)
	{
		if(!NtParam.devparams.Uneg)		// no negative voltages (U+, I+)
		{
			xyplot->plotData[AxisUnit_I][n] = xyplot->plotParameters.posMaxI;
			xyplot->plotData[AxisUnit_U][n] = 0;
			n++;

			xyplot->plotData[AxisUnit_I][n] = xyplot->plotParameters.posMaxI;
			xyplot->plotData[AxisUnit_U][n] = xyplot->plotParameters.posMaxU;
			n++;

			xyplot->plotData[AxisUnit_I][n] = 0;
			xyplot->plotData[AxisUnit_U][n] = xyplot->plotParameters.posMaxU;
			n++;

			if(NtParam.devparams.Ineg)		// no negative voltages, but negative current (U+, I+, I-)
			{
				xyplot->plotData[AxisUnit_I][n] = -xyplot->plotParameters.posMaxI;
				xyplot->plotData[AxisUnit_U][n] = xyplot->plotParameters.posMaxU;
				n++;

				xyplot->plotData[AxisUnit_I][n] = -xyplot->plotParameters.posMaxI;
				xyplot->plotData[AxisUnit_U][n] = 0;
				n++;
			}
		}
		else											// negative voltages (U+, U-, I+)
		{
			xyplot->plotData[AxisUnit_I][n] = 0;
			xyplot->plotData[AxisUnit_U][n] = -xyplot->plotParameters.posMaxU;
			n++;

			xyplot->plotData[AxisUnit_I][n] = xyplot->plotParameters.posMaxI;
			xyplot->plotData[AxisUnit_U][n] = -xyplot->plotParameters.posMaxU;
			n++;

			xyplot->plotData[AxisUnit_I][n] = xyplot->plotParameters.posMaxI;
			xyplot->plotData[AxisUnit_U][n] = xyplot->plotParameters.posMaxU;
			n++;

			xyplot->plotData[AxisUnit_I][n] = 0;
			xyplot->plotData[AxisUnit_U][n] = xyplot->plotParameters.posMaxU;
			n++;

			if(NtParam.devparams.Ineg)		// negative voltages and negative current (U+, U-, I+, i-)
			{
				xyplot->plotData[AxisUnit_I][n] = -xyplot->plotParameters.posMaxI;
				xyplot->plotData[AxisUnit_U][n] = xyplot->plotParameters.posMaxU;
				n++;

				xyplot->plotData[AxisUnit_I][n] = -xyplot->plotParameters.posMaxI;
				xyplot->plotData[AxisUnit_U][n] = -xyplot->plotParameters.posMaxU;
				n++;

				xyplot->plotData[AxisUnit_I][n] = 0;
				xyplot->plotData[AxisUnit_U][n] = -xyplot->plotParameters.posMaxU;
				n++;
			}
		}
	} // END UI-Mode
	else if(xyplot->plotParameters.mode == VIP)
	{
		int16_t cornerPos1I, cornerPos1U, cornerPos2I, cornerPos2U;
		int16_t diffCornerPosI;
		uint8_t quadrticCurvePtQty = QUADRATIC_PLOT_DATA_SIZE;
		uint8_t quadrticCurveStepQty = quadrticCurvePtQty - 1;
		float stepSize;
		int16_t quadraticCurve[2][QUADRATIC_PLOT_DATA_SIZE];

		// Corner Pos on Umax line
		cornerPos1I = xyplot->plotParameters.scaleI * (xyplot->plotParameters.totalSetP / xyplot->plotParameters.totalSetU);
		cornerPos1U = xyplot->plotParameters.posMaxU;

		// Corner Pos on Imax line
		cornerPos2I = xyplot->plotParameters.posMaxI;
		cornerPos2U = xyplot->plotParameters.scaleU * (xyplot->plotParameters.totalSetP / xyplot->plotParameters.totalSetI);

		diffCornerPosI = cornerPos2I - cornerPos1I;					// pixel distance between corner Points (of I)
		stepSize = (float)diffCornerPosI / quadrticCurveStepQty;	// step size in pixels

		for(uint8_t i = 0; i < (quadrticCurvePtQty - 1); i++)
		{
			int16_t posI, posU;
			float I, U;

			posI = cornerPos1I + (i * stepSize);
			I = posI / xyplot->plotParameters.scaleI;		// real I on next I-coordinate
			U = xyplot->plotParameters.totalSetP / I;		// real U; calculated with real I and setP
			posU = U * xyplot->plotParameters.scaleU;		// U-coordinate
			if(posU > cornerPos1U)
				posU = cornerPos1U;
			if(posI <= 0)										// Fehler in Display (hinzufügen) <----------------
				posU = cornerPos1U;

			quadraticCurve[AxisUnit_I][i] = posI;
			quadraticCurve[AxisUnit_U][i] = posU;
		}
		// Set last point to exact corner position
		quadraticCurve[AxisUnit_I][quadrticCurvePtQty - 1] = cornerPos2I;
		quadraticCurve[AxisUnit_U][quadrticCurvePtQty - 1] = cornerPos2U;

		if(!NtParam.devparams.Uneg)		// no negative voltages (U+, I+)
		{
			xyplot->plotData[AxisUnit_I][n] = xyplot->plotParameters.posMaxI;
			xyplot->plotData[AxisUnit_U][n] = 0;
			n++;

			for(uint8_t i = quadrticCurvePtQty; i > 0; i--)
			{
				xyplot->plotData[AxisUnit_I][n] = quadraticCurve[AxisUnit_I][i - 1];
				xyplot->plotData[AxisUnit_U][n] = quadraticCurve[AxisUnit_U][i - 1];
				n++;
			}

			xyplot->plotData[AxisUnit_I][n] = 0;
			xyplot->plotData[AxisUnit_U][n] = xyplot->plotParameters.posMaxU;
			n++;

			if(NtParam.devparams.Ineg)		// no negative voltages, but negative current (U+, I+, I-)
			{
				for(uint8_t i = 0; i < quadrticCurvePtQty; i++)
				{
					xyplot->plotData[AxisUnit_I][n] = -quadraticCurve[AxisUnit_I][i];
					xyplot->plotData[AxisUnit_U][n] = quadraticCurve[AxisUnit_U][i];
					n++;
				}

				xyplot->plotData[AxisUnit_I][n] = -xyplot->plotParameters.posMaxI;
				xyplot->plotData[AxisUnit_U][n] = 0;
				n++;
			}
		}
		else											// negative voltages (U+, U-, I+)
		{
			xyplot->plotData[AxisUnit_I][n] = 0;
			xyplot->plotData[AxisUnit_U][n] = -xyplot->plotParameters.posMaxU;			// Fehler in Display (-) <----------------
			n++;

			for(uint8_t i = 0; i < quadrticCurvePtQty; i++)
			{
				xyplot->plotData[AxisUnit_I][n] = quadraticCurve[AxisUnit_I][i];
				xyplot->plotData[AxisUnit_U][n] = -quadraticCurve[AxisUnit_U][i];
				n++;
			}

			for(uint8_t i = quadrticCurvePtQty; i > 0; i--)
			{
				xyplot->plotData[AxisUnit_I][n] = quadraticCurve[AxisUnit_I][i - 1];
				xyplot->plotData[AxisUnit_U][n] = quadraticCurve[AxisUnit_U][i - 1];
				n++;
			}

			xyplot->plotData[AxisUnit_I][n] = 0;
			xyplot->plotData[AxisUnit_U][n] = xyplot->plotParameters.posMaxU;
			n++;

			if(NtParam.devparams.Ineg)		// negative voltages and negative current (U+, U-, I+, I-)
			{
				for(uint8_t i = 0; i < quadrticCurvePtQty; i++)
				{
					xyplot->plotData[AxisUnit_I][n] = -quadraticCurve[AxisUnit_I][i];
					xyplot->plotData[AxisUnit_U][n] = quadraticCurve[AxisUnit_U][i];
					n++;
				}

				for(uint8_t i = quadrticCurvePtQty; i > 0; i--)
				{
					xyplot->plotData[AxisUnit_I][n] = -quadraticCurve[AxisUnit_I][i - 1];
					xyplot->plotData[AxisUnit_U][n] = -quadraticCurve[AxisUnit_U][i - 1];
					n++;
				}

				xyplot->plotData[AxisUnit_I][n] = 0;
				xyplot->plotData[AxisUnit_U][n] = -xyplot->plotParameters.posMaxU;		// Fehler in Display (-) <----------------
				n++;
			}
		}
	} // END UIP-Mode
	else if (xyplot->plotParameters.mode == VIR)
	{
		int16_t cornerPosU, cornerPosI;

		cornerPosI = xyplot->plotParameters.posMaxI;
		cornerPosU = xyplot->plotParameters.scaleU * (xyplot->plotParameters.totalSetU - xyplot->plotParameters.totalSetR * xyplot->plotParameters.totalSetI);
		if(cornerPosU < 0)
			cornerPosU = 0;

		if(!NtParam.devparams.Uneg)		// no negative voltages (U+, I+)
		{
			xyplot->plotData[AxisUnit_I][n] = xyplot->plotParameters.posMaxI;
			xyplot->plotData[AxisUnit_U][n] = 0;
			n++;

			xyplot->plotData[AxisUnit_I][n] = cornerPosI;
			xyplot->plotData[AxisUnit_U][n] = cornerPosU;
			n++;

			xyplot->plotData[AxisUnit_I][n] = 0;
			xyplot->plotData[AxisUnit_U][n] = xyplot->plotParameters.posMaxU;
			n++;

			if(NtParam.devparams.Ineg)		// no negative voltages, but negative current (U+, I+, I-)
			{
				xyplot->plotData[AxisUnit_I][n] = -cornerPosI;
				xyplot->plotData[AxisUnit_U][n] = cornerPosU;
				n++;

				xyplot->plotData[AxisUnit_I][n] = -xyplot->plotParameters.posMaxI;
				xyplot->plotData[AxisUnit_U][n] = 0;
				n++;
			}
		}
		else											// negative voltages (U+, U-, I+)
		{
			xyplot->plotData[AxisUnit_I][n] = 0;
			xyplot->plotData[AxisUnit_U][n] = -xyplot->plotParameters.posMaxU;
			n++;

			xyplot->plotData[AxisUnit_I][n] = cornerPosI;
			xyplot->plotData[AxisUnit_U][n] = -cornerPosU;
			n++;

			xyplot->plotData[AxisUnit_I][n] = cornerPosI;
			xyplot->plotData[AxisUnit_U][n] = cornerPosU;
			n++;

			xyplot->plotData[AxisUnit_I][n] = 0;
			xyplot->plotData[AxisUnit_U][n] = xyplot->plotParameters.posMaxU;
			n++;

			if(NtParam.devparams.Ineg)		// negative voltages and negative current (U+, U-, I+, I-)
			{
				xyplot->plotData[AxisUnit_I][n] = -cornerPosI;
				xyplot->plotData[AxisUnit_U][n] = cornerPosU;
				n++;

				xyplot->plotData[AxisUnit_I][n] = -cornerPosI;
				xyplot->plotData[AxisUnit_U][n] = -cornerPosU;
				n++;

				xyplot->plotData[AxisUnit_I][n] = 0;
				xyplot->plotData[AxisUnit_U][n] = -xyplot->plotParameters.posMaxU;
				n++;
			}
		}
	} // END UIR-Mode
	else if (xyplot->plotParameters.mode == PVSIM || xyplot->plotParameters.mode == USER)
	{
		float pvSimStepI = (float)xyplot->plotParameters.posMaxI / 255;
		float pvSimScaleU = (float)xyplot->plotParameters.posMaxU / 255;

		for(int i = 0; i < 256; i++)
		{
			xyplot->plotData[AxisUnit_I][i] = pvSimStepI * i;
			xyplot->plotData[AxisUnit_U][i] = NtParam.kennlinie.Kennlinie[i] * pvSimScaleU;
		}
		xyplot->plotData[AxisUnit_I][256] = xyplot->plotParameters.posMaxI;
		xyplot->plotData[AxisUnit_U][256] = 0;
	}
	// ws_Send_Text("xyplot->plotParameters.posMaxU = " + (String)xyplot->plotParameters.posMaxU);
	// ws_Send_Text("pvSimScaleU" + (String)pvSimScaleU);
	// ws_Send_Text("NtParam.kennlinie.Kennlinie[0] = " + (String)NtParam.kennlinie.Kennlinie[0]);
}

/**
 * calculate the position of the marker
 * @param void
 * @return void
 */
void calculate_marker_pos(void)
{
	float val_mstotal_u = monitor.v;
	float val_mstotal_i = monitor.i;

	int16_t markerPosU, markerPosI;
	int16_t markerPosX, markerPosY;

	// calculate marker position
	markerPosU = val_mstotal_u * xyplot->plotParameters.scaleU;
	markerPosI = val_mstotal_i * xyplot->plotParameters.scaleI;

	if(xyplot->plotParameters.xAxisUnit == AxisUnit_I)
	{
		markerPosX = xyplot->axisParameters.origin.x + markerPosI;
		markerPosY = xyplot->axisParameters.origin.y - markerPosU;
	}
	else
	{
		markerPosX = xyplot->axisParameters.origin.x + markerPosU;
		markerPosY = xyplot->axisParameters.origin.y - markerPosI;
	}

	if(markerPosX > xyplot->axisParameters.xAxisArrowStart.x)
		markerPosX = xyplot->axisParameters.xAxisArrowStart.x;
	else if(markerPosX < xyplot->axisParameters.xAxisStart.x)
		markerPosX = xyplot->axisParameters.xAxisStart.x;

	if(markerPosY < xyplot->axisParameters.yAxisArrowStart.y)
		markerPosY = xyplot->axisParameters.yAxisArrowStart.y;
	else if(markerPosY > (xyplot->axisParameters.yAxisStart.y + xyplot->axisParameters.yAxisSize.y))
		markerPosY = xyplot->axisParameters.yAxisStart.y + xyplot->axisParameters.yAxisSize.y;

	markerPos.x = markerPosX;
	markerPos.y = markerPosY;
}


/**
 * calculate the plot data. Call when a value has changed
 * @param Info, which info to send via ws
 * @return void
 */
void update_xy(uint8_t toSend)
{
	uint8_t ms_multiplier = 0;
	bool msactive = IsMasterSlaveActive(&ms_multiplier);
	ActiveQuadrats_e neededQuadrants;

	/* --- calculate setU --- */
	if (msactive && NtParam.msconfig.ms_mode == MS_Serial)
		xyplot->plotParameters.totalSetU = ms_multiplier * preset.v;
	else
		xyplot->plotParameters.totalSetU = preset.v;
	// setI
	if (msactive && NtParam.msconfig.ms_mode == MS_Parallel)
		xyplot->plotParameters.totalSetI = ms_multiplier * preset.i;
	else
		xyplot->plotParameters.totalSetI = preset.i;
	//set P
	if (msactive && (NtParam.msconfig.ms_mode == MS_Serial || NtParam.msconfig.ms_mode == MS_Parallel))
		xyplot->plotParameters.totalSetP = ms_multiplier * preset.p;
	else
		xyplot->plotParameters.totalSetP = preset.p;
	//setRi
	if (msactive && (NtParam.msconfig.ms_mode == MS_Serial))
		xyplot->plotParameters.totalSetR = ms_multiplier * preset.r;
	else if(msactive && NtParam.msconfig.ms_mode == MS_Parallel)
		xyplot->plotParameters.totalSetR = preset.r / (float)ms_multiplier;
	else
		xyplot->plotParameters.totalSetR = preset.r;

	/* --- Get needed quadrants --- */
	if (NtParam.devparams.Ineg && NtParam.devparams.Uneg)
		neededQuadrants = xy_negative;
	else if (NtParam.devparams.Ineg)
	{
		if(xyplot->plotParameters.xAxisUnit == AxisUnit_I)			// X = I, Y = U
			neededQuadrants = x_negative;
		else														// X = U, Y = I
			neededQuadrants = y_negative;
	}
	else if (NtParam.devparams.Uneg)
	{
		if(xyplot->plotParameters.xAxisUnit == AxisUnit_I)			// X = I, Y = U
			neededQuadrants = y_negative;
		else														// X = U, Y = I
			neededQuadrants = x_negative;
	}
	else
		neededQuadrants = onlyPositive;

	// Quadrants update needed? if yes, calculate axis data
	if(xyplot->plotParameters.activeQuadrants != neededQuadrants)
	{
		xyplot->plotParameters.activeQuadrants = neededQuadrants;
		update_numberOfQuadrants();
		calculate_axis_data();
	}

	calculate_plot_parameters();	// calculate max positions and scale of U and I
	calculate_plot_data();			// calculate plot points
	calculate_marker_pos();			// calculate and set marker position


	// if(toSend & GRAPH_SEND_AXIS)
	// 	send_XYParam();
	// if(toSend & GRAPH_SEND_PLOT){}
	// 	send_XYData();
	// if(toSend & GRAPH_SEND_MARKER)
	// 	send_XYMarker();
	// if(toSend & GRAPH_SEND_ALL){
	// 	send_XYParam();
	// 	send_XYMarker();
	// 	send_XYData();
	// }
}

/**
 * Send axis data and axis title location via websocket
 * @param void
 * @return void
 */
void send_XYParam(void)
{
    xyPlotData.graphParam.xAxisStart.x = xyplot->axisParameters.xAxisStart.x;
    xyPlotData.graphParam.xAxisStart.y = xyplot->axisParameters.xAxisStart.y;
    xyPlotData.graphParam.xAxisSize = xyplot->axisParameters.xAxisSize.x;
    xyPlotData.graphParam.xTitleLoc.x = xyplot->axisParameters.xAxisTitleLoc.x;
    xyPlotData.graphParam.xTitleLoc.y = xyplot->axisParameters.xAxisTitleLoc.y;

    xyPlotData.graphParam.yAxisStart.x = xyplot->axisParameters.yAxisStart.x;
    xyPlotData.graphParam.yAxisStart.y = xyplot->axisParameters.yAxisStart.y;
    xyPlotData.graphParam.yAxisSize    = xyplot->axisParameters.yAxisSize.y;
    xyPlotData.graphParam.yTitleLoc.x = xyplot->axisParameters.yAxisTitleLoc.x;
    xyPlotData.graphParam.yTitleLoc.y = xyplot->axisParameters.yAxisTitleLoc.y;

    queue_WS_MSG(XY_AXIS);
}

/**
 * Send marker position via websocket
 * @param void
 * @return void
 */
void send_XYMarker(void)
{
    xyPlotData.markerPos.x = markerPos.x;
    xyPlotData.markerPos.y = markerPos.y;

    queue_WS_MSG(XY_MPOS);
}

/**
 * Send plot data (red line) via websocket
 * @param void
 * @return void
 */
void send_XYData(void)
{
    int i;
    
    for(i = 0; i < PLOT_DATA_SIZE; ++i)
	{	
		if(xyplot->plotDataX[i] != (int16_t)0x8080)		// 0x8080 == -32640 -> should never be a coordinate -> empty field
		{
            xyPlotData.plotData[i].x = xyplot->axisParameters.origin.x + xyplot->plotDataX[i];
            xyPlotData.plotData[i].y = xyplot->axisParameters.origin.y - xyplot->plotDataY[i];
		}
		else
		{
			break;
		}
	}

    xyPlotData.plotDataSize = i;
    queue_WS_MSG(XY_PDATA);
}
