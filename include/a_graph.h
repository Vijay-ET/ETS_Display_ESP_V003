/***********************************************************
 *
 * a_graph.h
 * SYSTART GmbH
 * MIZ, 09.07.2021
 *
 ***********************************************************/

#ifndef A_GRAPH_H_
#define A_GRAPH_H_

 /***********************************************************
 * Includes
 ***********************************************************/
#include "a_com.h"

 /***********************************************************
 * Defines
 ***********************************************************/
#define ARROW_LENGTH				7
#define AXIS_WIDTH					3

#define X_AXIS_GAP_LEFT				10
#define X_AXIS_GAP_RIGHT			10
#define X_AXIS_TITLE_OFFSETX		-14
#define X_AXIS_TITLE_OFFSETY		-6
#define X_AXIS_TITLE_SIZEX			12
#define X_AXIS_TITLE_SIZEY			24

#define Y_AXIS_GAP_TOP				13
#define Y_AXIS_GAP_BOTTOM			13
#define Y_AXIS_TITLE_OFFSETX		10
#define Y_AXIS_TITLE_OFFSETY		6
#define Y_AXIS_TITLE_SIZEX			12
#define Y_AXIS_TITLE_SIZEY			24

#define PLOT_DATA_SIZE				257	// 256 bytes for pvSim + endbyte at I axis

#define X_AXIS_MAX_LIMIT			90	// max printed value should be at 90% of axis length
#define Y_AXIS_MAX_LIMIT			90	// max printed value should be at 90% of axis length

#define QUADRATIC_PLOT_DATA_SIZE	61


#define GRAPH_SEND_NONE				0
#define GRAPH_SEND_AXIS				1
#define GRAPH_SEND_PLOT				2
#define GRAPH_SEND_MARKER			4
#define GRAPH_SEND_ALL				7

/***********************************************************
 * Typ Definitionen
 ***********************************************************/

typedef enum{
	notDefined,
	onlyPositive,
	x_negative,
	y_negative,
	xy_negative
}ActiveQuadrats_e;

typedef enum{
	AxisUnit_I,
	AxisUnit_U
}AxisUnit_e;

typedef struct{
	int16_t x;
	int16_t y;
}Point_t;	// V: 4 Bytes

typedef struct {
	AxisUnit_e xAxisUnit;
	ActiveQuadrats_e activeQuadrants;	// onlyPositive, i_negative, ...
	uint16_t mode;

	float totalSetU;
	float totalSetI;
	float totalSetP;
	float totalSetR;
	int16_t posMaxU;
	int16_t posMaxI;
	float scaleU;
	float scaleI;
}XYPlotParameters_t;

typedef struct {
	Point_t xAxisStart;			// start location of x-axis
	Point_t xAxisSize;			// size of x-axis
	Point_t xAxisArrowStart;	// start point of x-axis arrow
	Point_t xAxisTitleLoc;		// position of x-axis title
	Point_t xAxisTitleSize;		// size of x-axis title

	Point_t yAxisStart;			// start location of x-axis
	Point_t yAxisSize;			// size of x-axis
	Point_t yAxisArrowStart;	// start point of x-axis arrow
	Point_t yAxisTitleLoc;		// position of x-axis title
	Point_t yAxisTitleSize;		// size of x-axis title

	Point_t origin;				// Coordinates of the plot origin
	Point_t AxisLengthPositive;	// maximum positive pixel length (0 to arrow)
}XYAxisParameters_t;

typedef struct{
	Point_t location;			// location of chartarea
	Point_t size;				// size of chartarea
	Point_t originOffset;		// offset form start of x and y axis (% of axis lenght)
	// Textfeld_t axis_x_title;
	// Textfeld_t axis_y_title;

	// color_t bgcolor; 			// color of chartarea
	// color_t fgcolor; 			// color of axes
	// color_t plotColor; 			// color of plot

	int16_t *plotDataX;
	int16_t *plotDataY;
	int16_t plotData[2][PLOT_DATA_SIZE];

	XYAxisParameters_t axisParameters;
	XYPlotParameters_t plotParameters;
}xyPlot_t;

/***********************************************************
 * Exported Variables
 ***********************************************************/

 /***********************************************************
 * Funktionsdeklarationen
************************************************************/
void init_XY_Graph(void);
void update_xy(uint8_t toSend);

#endif /* A_GRAPH_H_ */
