/*
 * This file is part of the Contiki operating system.
 *
 */

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"

#include <string.h>
#include "dev/leds.h"
#include "dev/button-sensor.h"
#include "debug.h"

#define DEBUG DEBUG_PRINT
#include "net/uip-debug.h"

#define SEND_INTERVAL		10 * CLOCK_SECOND
#define MAX_PAYLOAD_LEN		40

#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_UDP_BUF  ((struct uip_udp_hdr *)&uip_buf[uip_l2_l3_hdr_len])

static char buf[MAX_PAYLOAD_LEN];

/* Our destinations and udp conns. One link-local and one global */
#define LOCAL_CONN_PORT 8802
static struct uip_udp_conn *l_conn;
#if UIP_CONF_ROUTER
#define GLOBAL_CONN_PORT 8802
static struct uip_udp_conn *g_conn;
#endif
#define LED_TOGGLE_REQUEST (0x79)
#define LED_SET_STATE (0x7A)
#define LED_GET_STATE (0x7B)
#define LED_STATE (0x7C)

/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "UDP client process");
#if BUTTON_SENSOR_ON
PROCESS_NAME(ping6_process);
AUTOSTART_PROCESSES(&udp_client_process, &ping6_process);
#else
AUTOSTART_PROCESSES(&udp_client_process);
#endif
/*---------------------------------------------------------------------------*/
static void tcpip_handler ( void )
{
	char i=0;
	#define SEND_ECHO (0xBA)
	if(uip_newdata()) // verifica se novos dados foram recebidos
	{
		char * dados = (( char *) uip_appdata ); // este buffer eh pardao do contiki
		PRINTF("Recebidos %d bytes \n" , uip_datalen ());
		switch (dados[0])
		{
		case LED_GET_STATE:
		{
			uip_ipaddr_copy (& g_conn -> ripaddr , & UIP_IP_BUF -> srcipaddr );
			g_conn -> rport = UIP_UDP_BUF -> destport ;
			memset(buf, 0, MAX_PAYLOAD_LEN);
			buf[0] = LED_STATE;
			buf[1] = leds_get();
			uip_udp_packet_send (g_conn, buf, 2);
			break;
		}
		case LED_SET_STATE:
		{
			leds_off(LEDS_ALL);
			leds_on(dados[1]); //leds_set nao funcionou.
			//Enviando o novo estado dos leds ao gateway..
			uip_ipaddr_copy (& g_conn -> ripaddr , & UIP_IP_BUF -> srcipaddr );
			g_conn -> rport = UIP_UDP_BUF -> destport ;
			memset(buf, 0, MAX_PAYLOAD_LEN);
			buf[0] = LED_STATE;
			buf[1] = leds_get();
			uip_udp_packet_send (g_conn, buf, 2);
			break;
		}
		case SEND_ECHO:
		{
			uip_ipaddr_copy (& g_conn -> ripaddr , & UIP_IP_BUF -> srcipaddr );
			g_conn -> rport = UIP_UDP_BUF -> destport ;
			uip_udp_packet_send (g_conn , dados , uip_datalen ());
			PRINTF("Enviando eco para [");
			PRINT6ADDR (& g_conn -> ripaddr );
			PRINTF("]:% u\n", UIP_HTONS (g_conn -> rport ));
			break ;
		}
		default:
		{
			PRINTF("Comando Invalido :");
			for (i=0;i<uip_datalen ();i++)
			{
				PRINTF("0x%02X" ,dados [i]);
			}
			PRINTF("\n");
			break ;
		}
		}
	}
	return;

}
/*---------------------------------------------------------------------------*/
static void
timeout_handler(void)
{
	memset(buf, 0, MAX_PAYLOAD_LEN); //zera o buffer global
	/*
	   * memset preenche uma quantidade de uma determinada área da memória com um dado valor. Em outras palavras,
	   * inicializa algum objeto (variável, estrutura, etc).
	   * Exemplo:
	   * struct mystruct s;
	   * memset(&s, 0, sizeof(s));
	   * O exemplo acima preencherá a estrutura 's' do tipo mystruct com o caractere nulo.
	*/

  	//Verifica se conhece o IP global do ROOT:
  	if(uip_ds6_get_global(ADDR_PREFERRED) == NULL) {
  		PRINTF ("O No ainda nao tem um IP global valido! \n");
  	}else{
  		// Imprime uma mensagem indicando o IP e a porta de destino.
  		PRINTF("Cliente para [");
  	  	PRINT6ADDR(&g_conn->ripaddr);
  	  	PRINTF("]:%u\n", UIP_HTONS(g_conn->rport));

  		buf[0] = LED_TOGGLE_REQUEST;
  		uip_udp_packet_send(g_conn, buf, sizeof(LED_TOGGLE_REQUEST));
  		//Como o tamanho de LED_TOGGLE_REQUEST eh um byte, no terceiro argumento poderiamos colocar 1 apenas.
  	}
}
/*---------------------------------------------------------------------------*/
static void
print_local_addresses(void)
{
	int i;
	uint8_t state;
	PRINTF("Nodes's IPv6 addresses:\n");
	for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
		state = uip_ds6_if.addr_list[i].state;
		if(uip_ds6_if.addr_list[i].isused && (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
			PRINTF("  \n");
			PRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
			if(state == ADDR_TENTATIVE) {
				uip_ds6_if.addr_list[i].state = ADDR_PREFERRED;
			}
		}
	}
}

PROCESS_THREAD(udp_client_process, ev, data)
{
	static struct etimer et;
	uip_ipaddr_t ipaddr;

	PROCESS_BEGIN();
	PRINTF("UDP client process started\n");

	// ****** CONEXAO LOCAL ****** // (Inicio do IP = fe80)
	uip_ip6addr(&ipaddr, 0xfe80, 0, 0, 0, 0x0212, 0x4b00, 0x04e9, 0x043b);
	/* new connection with remote host */
	l_conn = udp_new(&ipaddr, UIP_HTONS(LOCAL_CONN_PORT), NULL);
	if(!l_conn) {
		PRINTF("udp_new l_conn error.\n");
	}
	udp_bind(l_conn, UIP_HTONS(LOCAL_CONN_PORT));

	PRINTF("Link-Local connection with ");
	PRINT6ADDR(&l_conn->ripaddr);
	PRINTF("local/remote port %u/%u\n",
         UIP_HTONS(l_conn->lport), UIP_HTONS(l_conn->rport));

	// ****** CONEXAO GLOBAL ****** //
	uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0x0212, 0x4b00, 0x04e9, 0x043b);
	g_conn = udp_new(&ipaddr, UIP_HTONS(GLOBAL_CONN_PORT), NULL);
	if(!g_conn) {
		PRINTF("udp_new g_conn error.\n");
	}
	udp_bind(g_conn, UIP_HTONS(GLOBAL_CONN_PORT));

	print_local_addresses();

	PRINTF("\nGlobal connection with ");
	PRINT6ADDR(&g_conn->ripaddr);
	PRINTF("local/remote port %u/%u\n",
         UIP_HTONS(g_conn->lport), UIP_HTONS(g_conn->rport));


	etimer_set(&et, SEND_INTERVAL); // 10 sec
	while(1) {
		PROCESS_WAIT_EVENT();
		if(etimer_expired(&et)) {
			timeout_handler();
			etimer_restart(&et);
		}else if(ev == tcpip_event) {
			tcpip_handler();
		}
	}
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
