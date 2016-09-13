/*
 * Aula 03 - Lab 01 - Atividade 01 - Markos Flavio
 * Author: George Oikonomou - <oikonomou@users.sourceforge.net>
 * Alterado para a disciplina de Rede de Sensores Sem Fio
 */
#include "contiki.h"
#include "dev/leds.h"

#include <stdio.h> /* For printf() */
/*---------------------------------------------------------------------------*/
static struct etimer et_hello;
static struct etimer et_blink;
static uint16_t count;
static uint8_t blinks;
/*---------------------------------------------------------------------------*/
PROCESS(hello_world_process, "Hello world process");
PROCESS(blink_process, "LED blink process");
AUTOSTART_PROCESSES(&blink_process);
/*---------------------------------------------------------------------------*/
//O processo abaixo nunca eh inicializado:
PROCESS_THREAD(hello_world_process, ev, data)
{
	PROCESS_BEGIN();

	etimer_set(&et_hello, CLOCK_SECOND * 4);
	count = 0;

	while(1) {
		PROCESS_WAIT_EVENT(); // Eu espero por qualquer evento.

		if(ev == PROCESS_EVENT_TIMER) {
			printf("Hello world #%u!\n", count);
			count++;

			etimer_reset(&et_hello);
			//Mesmo apos o reset, o timer ainda gerara' evento quando ocorrer 4 segundos.
		}
		/*No caso desse loop qualquer evento que ocorrer vai "acordar" o processo, e a checagem
		do if sera realizada (alguns cilcos de clock passarao). Isso ocorre para qualquer evento ativado,
		entretanto apenas eventos do tipo TIMER eh relevante. Uma checagem como a abaixo previne esse tipo de
		fenomeno:
		PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
		Ou mais especificamente:
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et3));	*/
	}
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(blink_process, ev, data)
{
	PROCESS_BEGIN();

	blinks = 0;
	printf("LEDS_ALL = %d\n", LEDS_ALL);

	leds_on(LEDS_ALL);
	etimer_set(&et_blink, 5*CLOCK_SECOND);
	PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);

	while(1) {
		//Esse timer poderia ficar anteriormente ao 'while'. Porem seria necessario reseta-lo a cada iteracao.
		etimer_set(&et_blink, CLOCK_SECOND);

		PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
		leds_off(LEDS_ALL);
		leds_on(blinks & LEDS_ALL);
		blinks++;
		printf("Blink... (state %X)\n", leds_get());
	}
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
