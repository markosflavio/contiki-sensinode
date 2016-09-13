/*
 * Aula 03 - Lab 01 - Atividade 03 - Markos Flavio
 */
#include "contiki.h"
#include "dev/leds.h"
#include <stdio.h> /* For printf() */
/*---------------------------------------------------------------------------*/
#define LED_PING_EVENT (44)
#define LED_PONG_EVENT (45)
/*---------------------------------------------------------------------------*/
static struct etimer et_hello;
static struct etimer et_blink;
static struct etimer et_proc3;
static struct etimer et_pong;
static uint16_t count;
static uint8_t blinks;
/*---------------------------------------------------------------------------*/
PROCESS(hello_world_process, "Hello world process");
PROCESS(blink_process, "LED blink process");
PROCESS(proc3_process, "Processo 3");
PROCESS(pong_process, "Processo Pong");
AUTOSTART_PROCESSES(&blink_process, &hello_world_process, &proc3_process, &pong_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(hello_world_process, ev, data)
{
	PROCESS_BEGIN();

	etimer_set(&et_hello, 2*CLOCK_SECOND);

	while(1) {
		PROCESS_WAIT_EVENT();

		if(ev == PROCESS_EVENT_TIMER) {
			printf("Hello: pinging\n");
			process_post(&pong_process, LED_PING_EVENT, (void*)(&hello_world_process));
			etimer_reset(&et_hello);
		}
		if(ev == LED_PONG_EVENT){
			leds_toggle(LEDS_GREEN);
			printf("Hello: LED Pong!\n");
		}
    }
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(blink_process, ev, data)
{
	PROCESS_BEGIN();

	etimer_set(&et_blink, 4*CLOCK_SECOND);

	while(1) {
		PROCESS_WAIT_EVENT();

		if(ev == PROCESS_EVENT_TIMER) {
			printf("Blink: pinging\n");
			process_post(&pong_process, LED_PING_EVENT, (void*)(&blink_process));
			etimer_reset(&et_blink);
		}
		if(ev == LED_PONG_EVENT){
        	leds_toggle(LEDS_YELLOW);
        	printf("Blink: LED Pong!\n");
        }
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
			printf("Proc3: pinging\n");
			process_post(&pong_process, LED_PING_EVENT, (void*)(&proc3_process));
			etimer_reset(&et_proc3);
       	}
        if(ev == LED_PONG_EVENT){
            leds_toggle(LEDS_RED);
            printf("Proc3: LED Pong!\n");
        }
	}
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(pong_process, ev, data)
{
	PROCESS_BEGIN();

	while(1) {
		PROCESS_WAIT_EVENT();

		if(ev == PROCESS_EVENT_TIMER) {
			leds_off(LEDS_BLUE);
		}
		if(ev == LED_PING_EVENT){

			leds_on(LEDS_BLUE);
			etimer_set(&et_pong, CLOCK_SECOND*0.2);

			printf("Pong: LED Ping! Ponging...\n");
    	    if((struct process*)data == &blink_process) {
    	    	process_post(&blink_process,LED_PONG_EVENT, NULL);
    	    }
    	    if((struct process*)data == &hello_world_process) {
    	        process_post(&hello_world_process,LED_PONG_EVENT, NULL);
    	    }
    	    if((struct process*)data == &proc3_process) {
    	        process_post(&proc3_process,LED_PONG_EVENT, NULL);
    	    }
    	    //Nao eh necessario fazer a checagem acima, pode-se enviar o PONG diretamente:
    	    //process_post ((struct process*) data , LED_PONG_EVENT , NULL);
		}
	}
	PROCESS_END();
}
