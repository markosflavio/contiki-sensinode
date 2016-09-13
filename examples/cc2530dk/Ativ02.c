/*
 * Aula 03 - Lab 01 - Atividade 02 - Markos Flavio
 */
#include "contiki.h"
#include "dev/leds.h"

#include <stdio.h> /* For printf() */
/*---------------------------------------------------------------------------*/
static struct etimer et_hello;
static struct etimer et_blink;
static struct etimer et_proc3;
static uint16_t count;
static uint8_t blinks;
/*---------------------------------------------------------------------------*/
PROCESS(hello_world_process, "Hello world process");
PROCESS(blink_process, "LED blink process");
PROCESS(proc3_process, "Processo 3");
AUTOSTART_PROCESSES(&blink_process, &hello_world_process, &proc3_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(hello_world_process, ev, data)
{
	PROCESS_BEGIN();

	etimer_set(&et_hello, CLOCK_SECOND * 4);
	count = 0;

	while(1) {
		PROCESS_WAIT_EVENT();

		//Esse tipo de teste logico nao gera conflitos com outros timers?
		if(ev == PROCESS_EVENT_TIMER) {
			printf("Hello world #%u!\n", count);
			count++;
			etimer_reset(&et_hello);
		}
	}
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(blink_process, ev, data)
{
	PROCESS_BEGIN();

	leds_off(LEDS_ALL);
	etimer_set(&et_blink, 5*CLOCK_SECOND);
	PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);

	while(1) {
		etimer_set(&et_blink, 2*CLOCK_SECOND);

		PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
		leds_toggle(LEDS_YELLOW);
	}
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(proc3_process, ev, data)
{
	PROCESS_BEGIN();

	etimer_set(&et_proc3, 5*CLOCK_SECOND);

	while(1) {
		PROCESS_WAIT_EVENT();

		if(ev == PROCESS_EVENT_TIMER) {
			leds_toggle(LEDS_RED);
			etimer_reset(&et_proc3);
		}
	}
	PROCESS_END();
}
