/***********************************************************
 *
 * a_timer.c
 * SYSTART GmbH
 * MIZ, 09.06.2021
 *
 ***********************************************************/
/***********************************************************
 * Includes
 ***********************************************************/
#include "d_timer.h"
#include "Serial_prints.h"

/***********************************************************
 * globale Variabeln
 ***********************************************************/
static os_timer_t Timer1;
static Timer_t upTime = 0;

/***********************************************************
 * Deklaration interner Funktionen
 ***********************************************************/
void timer1Callback(void *pArg);

/***********************************************************
 * Funktionsdefinitionen
 ***********************************************************/

void timer_init(void)
{
  os_timer_setfn(&Timer1, timer1Callback, 0);
  os_timer_arm(&Timer1, 1, true);
  upTime = 0;
}

Timer_t timer_get_time(void)
{
  return upTime;
}

void timer1Callback(void *pArg)
{
  ++upTime;
}