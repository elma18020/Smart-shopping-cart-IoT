#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "driver/uart.h"
#include "protocol_examples_common.h"
#include "nvs.h"
#include "nvs_flash.h"
#include <esp_http_server.h>
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "driver/gpio.h"



#include "debug.h"

#include "mfrc522.h"
#include "display.h"
#include "ssd1306.h"


#define SOFT_AP_SSID "192.168.224.235"
#define SOFT_AP_PASSWORD "0987654321"

#define SOFT_AP_IP_ADDRESS_1 192
#define SOFT_AP_IP_ADDRESS_2 168
#define SOFT_AP_IP_ADDRESS_3 0
#define SOFT_AP_IP_ADDRESS_4 1

#define SOFT_AP_GW_ADDRESS_1 192
#define SOFT_AP_GW_ADDRESS_2 168
#define SOFT_AP_GW_ADDRESS_3 0
#define SOFT_AP_GW_ADDRESS_4 2

#define SOFT_AP_NM_ADDRESS_1 255
#define SOFT_AP_NM_ADDRESS_2 255
#define SOFT_AP_NM_ADDRESS_3 255
#define SOFT_AP_NM_ADDRESS_4 0

#define SERVER_PORT 80 




#define goto(x,y) printf("\033[%d;%dH", x, y)

static const char* TAG = "app";

typedef struct
{
  char nameOfArticle[25];
  char price[10];
  uint32_t id;
  uint32_t intID;
}RFID_SHOP;

uint32_t  availableID[3] = {0x21562300, 0x312F9E00, 0x813B3A00}; 
RFID_SHOP articles[3] = {{"Cokolada", "2 KM", 0x21562300, 0}, {"Cips", "3 KM", 0x312F9E00, 1}, {"Dvojni C", "2 KM", 0x813B3A00, 2}};
uint32_t finishShopping = 0xF7575C00;
uint8_t rfidToSend[4];
uint32_t cokoladaCounter = 0;
uint32_t cipsCounter = 0;
uint32_t dvojniCounter = 0;

//wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww
//NOTE:
//
//Connection to the RFID-RC522 module
//RC522			-			NODE MCU
//SDA			-			D2
//SCK			-			D5
//MOSI			-			D6
//MISO			-			D7
//3V3			-			3V3
//GND			-			GND




static httpd_handle_t server = NULL;



/* An HTTP GET handler */
esp_err_t pocetnaDodaj_get_handler(httpd_req_t *req)
{
   
   	uint8_t rf0, rf1, rf2, rf3;
	uint32_t i = 0;

     while(i < 3)
     {
        rf0 = (articles[i].id & 0xFF000000) >> 24;
        rf1 = (articles[i].id & 0x00FF0000) >> 16;
        rf2 = (articles[i].id & 0x0000FF00) >> 8;
        rf3 = (articles[i].id & 0x000000FF);
        
        if(rfidToSend[0] == rf0 && rfidToSend[1] == rf1 && rfidToSend[2] == rf2 && rfidToSend[3] == rf3 && articles[i].intID == 0)
        {
	       cokoladaCounter++;
	       break;
		}  
		else if (rfidToSend[0] == rf0 && rfidToSend[1] == rf1 && rfidToSend[2] == rf2 && rfidToSend[3] == rf3 && articles[i].intID == 1)
		{
		   cipsCounter++;
	       break;
		}
		else if (rfidToSend[0] == rf0 && rfidToSend[1] == rf1 && rfidToSend[2] == rf2 && rfidToSend[3] == rf3 && articles[i].intID == 2)
		{
		   dvojniCounter++;
	       break;
		}
	   i++;           
	 }
   
   
   char resp[1024];
    snprintf(resp, sizeof(resp),req->user_ctx);
    httpd_resp_send(req, resp, strlen(resp));
    return ESP_OK;
}

httpd_uri_t pocetnaDodaj = {
    .uri       = "/pocetnaDodaj",
    .method    = HTTP_GET,
    .handler   = pocetnaDodaj_get_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = {
			"HTTP/1.1 200 OK\r\n"
            "Content-type: text/html\r\n\r\n"
			"<head>"
			"<title>Dobro dosli u Smart Shopping Cart</title>"
			"</style><body><center>"
			"</head>"         
			"<style> div.main {"
			"font-family: Times New Roman;}"
			"</style></head>"
			"<body><div class='main'>"
			"<h1>Dobro dosli u Pametnu Korpu</h1>"
			"<br><br><br><br><br>"
			"<button onclick=\"location.href='/proizvod'\" type='button'>"
			"<style=""color:blue;""> Dodaj proizvod</button></p>"
			"<button   onclick=\"location.href='/racun'\" type='button'>"
			"Zavrsi</button></p>"
			"</div></body></html>"
			"</center></body></html>"
    },
};




/* An HTTP GET handler */
esp_err_t pocetnaObrisi_get_handler(httpd_req_t *req)
{
   
   	uint8_t rf0, rf1, rf2, rf3;
	uint32_t i = 0;

     while(i < 3)
     {
        rf0 = (articles[i].id & 0xFF000000) >> 24;
        rf1 = (articles[i].id & 0x00FF0000) >> 16;
        rf2 = (articles[i].id & 0x0000FF00) >> 8;
        rf3 = (articles[i].id & 0x000000FF);
        
        if(rfidToSend[0] == rf0 && rfidToSend[1] == rf1 && rfidToSend[2] == rf2 && rfidToSend[3] == rf3 && articles[i].intID == 0)
        {  
			if(cokoladaCounter > 0)
			{
	          cokoladaCounter--;
		    }
	       break;
		}  
		else if (rfidToSend[0] == rf0 && rfidToSend[1] == rf1 && rfidToSend[2] == rf2 && rfidToSend[3] == rf3 && articles[i].intID == 1)
		{
			if(cipsCounter > 0)
			{
	          cipsCounter--;
		    }
	       break;
		}
		else if (rfidToSend[0] == rf0 && rfidToSend[1] == rf1 && rfidToSend[2] == rf2 && rfidToSend[3] == rf3 && articles[i].intID == 2)
		{
		   if(dvojniCounter > 0)
			{
	          dvojniCounter--;
		    }
	       break;
		}
	   i++;           
	 }
   
   
   char resp[1024];
    snprintf(resp, sizeof(resp),req->user_ctx);
    httpd_resp_send(req, resp, strlen(resp));
    return ESP_OK;
}

httpd_uri_t pocetnaObrisi = {
    .uri       = "/pocetnaObrisi",
    .method    = HTTP_GET,
    .handler   = pocetnaObrisi_get_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = {
			"HTTP/1.1 200 OK\r\n"
            "Content-type: text/html\r\n\r\n"
			"<head>"
			"<title>Dobro dosli u Smart Shopping Cart</title>"
			"</style><body><center>"
			"</head>"         
			"<style> div.main {"
			"font-family: Times New Roman;}"
			"</style></head>"
			"<body><div class='main'>"
			"<h1>Dobro dosli u Pametnu Korpu</h1>"
			"<br><br><br><br><br>"
			"<button onclick=\"location.href='/proizvod'\" type='button'>"
			"<style=""color:blue;""> Dodaj proizvod</button></p>"
			"<button   onclick=\"location.href='/racun'\" type='button'>"
			"Zavrsi</button></p>"
			"</div></body></html>"
			"</center></body></html>"
    },
};

/* An HTTP GET handler */
esp_err_t zavrseno_get_handler(httpd_req_t *req)
{
    cokoladaCounter = 0;
    cipsCounter = 0;
    dvojniCounter = 0;
  
    char resp[1024];
    snprintf(resp, sizeof(resp),req->user_ctx);
    httpd_resp_send(req, resp, strlen(resp));
    return ESP_OK;
}

httpd_uri_t zavrseno = {
    .uri       = "/zavrseno",
    .method    = HTTP_GET,
    .handler   = zavrseno_get_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = {
			"HTTP/1.1 200 OK\r\n"
            "Content-type: text/html\r\n\r\n"
			"<head>"
			"<title>Dobro dosli u Smart Shopping Cart</title>"
			"</style><body><center>"
			"</head>"         
			"<style> div.main {"
			"font-family: Times New Roman;}"
			"</style></head>"
			"<body><div class='main'>"
			"<h1>Hvala vam na ukazanom povjerenju</h1>"
			"<br><br><br><br><br>"
			"<button onclick=\"location.href='/pocetna'\" type='button'>"
			"<style=""color:blue;""> Zapocni kupovinu</button></p>"
			"</div></body></html>"
			"</center></body></html>"
    },
};




/* An HTTP GET handler */
esp_err_t pocetna_get_handler(httpd_req_t *req)
{
    char resp[1024];
    snprintf(resp, sizeof(resp),req->user_ctx);
    httpd_resp_send(req, resp, strlen(resp));
    return ESP_OK;
}


httpd_uri_t pocetna = {
    .uri       = "/pocetna",
    .method    = HTTP_GET,
    .handler   = pocetna_get_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = {
			"HTTP/1.1 200 OK\r\n"
            "Content-type: text/html\r\n\r\n"
			"<head>"
			"<title>Dobro dosli u Smart Shopping Cart</title>"
			"</style><body><center>"
			"</head>"         
			"<style> div.main {"
			"font-family: Times New Roman;}"
			"</style></head>"
			"<body><div class='main'>"
			"<h1>Dobro dosli u Pametnu Korpu</h1>"
			"<br><br><br><br><br>"
			"<button onclick=\"location.href='/proizvod'\" type='button'>"
			"<style=""color:blue;""> Dodaj proizvod</button></p>"
			"<button   onclick=\"location.href='/racun'\" type='button'>"
			"Zavrsi</button></p>"
			"</div></body></html>"
			"</center></body></html>"
    },
};

/* An HTTP POST handler */
esp_err_t proizvod_get_handler(httpd_req_t *req)
{
	uint8_t rf0, rf1, rf2, rf3;
	uint32_t i = 0;
	char nameToSend[25] = "";
	char priceToSend[10] = "";
	
     while(i < 3)
     {
        rf0 = (articles[i].id & 0xFF000000) >> 24;
        rf1 = (articles[i].id & 0x00FF0000) >> 16;
        rf2 = (articles[i].id & 0x0000FF00) >> 8;
        rf3 = (articles[i].id & 0x000000FF);
        
        if(rfidToSend[0] == rf0 && rfidToSend[1] == rf1 && rfidToSend[2] == rf2 && rfidToSend[3] == rf3 && articles[i].intID == 0)
        {
	       strcat(nameToSend, "Cokolada");
	       strcat(priceToSend, "2 KM");
	       break;
		}  
		else if (rfidToSend[0] == rf0 && rfidToSend[1] == rf1 && rfidToSend[2] == rf2 && rfidToSend[3] == rf3 && articles[i].intID == 1)
		{
		   strcat(nameToSend, "Cips");
	       strcat(priceToSend, "3 KM");
	       break;
		}
		else if (rfidToSend[0] == rf0 && rfidToSend[1] == rf1 && rfidToSend[2] == rf2 && rfidToSend[3] == rf3 && articles[i].intID == 2)
		{
		   strcat(nameToSend, "Dvojni C");
	       strcat(priceToSend, "2 KM");
	       break;
		}
	   i++;           
	 }
   
    char resp[1024];
    snprintf(resp, sizeof(resp),req->user_ctx, nameToSend, priceToSend);
    httpd_resp_send(req, resp, strlen(resp));

    return ESP_OK;
}

httpd_uri_t proizvod = 
{
    .uri       = "/proizvod",
    .method    = HTTP_GET,
    .handler   = proizvod_get_handler,
    .user_ctx  = {
        "HTTP/1.1 200 OK\r\n"
            "Content-type: text/html\r\n\r\n"
            "</head>"         
			"<style> div.main {"
			"font-family: Times New Roman;}"
			"</style></head>"
			"<body><div class='main'>"       
			"<h1>Pametna kosarica</h1>"
			"</center></body></html>"
			"<hr>"
			"<div class=""Cart-Items"">"
			"<div class=""about"">"
			"<h3 class=""title"">Ime proizvoda: %s </h3>"
			"</div>"
			"<div class=""Cart-Items"">"
			"<div class=""about"">"
			"<h3 class=""title"">Cijena: %s </h3>"
			"</div>"
			"<hr>"
			"</style><body><center>" 
			"<button onclick=\"location.href='/pocetnaDodaj'\" type='button'>"
			"Dodaj proizvod</button></p>"
			"<button onclick=\"location.href='/pocetnaObrisi'\" type='button'>"
			"Ukloni</button></p>"
			"</div></body></html>"
			"</center></body></html>"
    }
};

esp_err_t racun_get_handler(httpd_req_t *req)
{
 
    uint32_t ukupnaCijena = (2 * cokoladaCounter) + (3 * cipsCounter) + (2 * dvojniCounter);
    
    char resp[1024];
    snprintf(resp, sizeof(resp),req->user_ctx,
            cokoladaCounter,
            cipsCounter,dvojniCounter,ukupnaCijena);
    httpd_resp_send(req, resp, strlen(resp));

    return ESP_OK;
}

httpd_uri_t racun = {
    .uri       = "/racun",
    .method    = HTTP_GET,
    .handler   = racun_get_handler,
    .user_ctx  = {
        "HTTP/1.1 200 OK\r\n"
            "Content-type: text/html\r\n\r\n"
            "<html><head><title>E Cart using IoT</title></head>" 
			"</style><body><center>"    
			"<h1>Kupovina zavrsena </h1>"
			"<hr>"
			"<br><br>"
			"<table border=""1"" style=""width: 70%; background: #ffe6e6; color: grey;" ">"
			"<thead>"
			"<tr>"
			"<th scope=""col""><br>PROIZVOD<br><br></th>"
			"<th><br>CIJENA<br><br></th>"
			"<th><br>KOLICINA<br><br></th>"
			"</tr>"
			"</thead>"
			"<tbody>"
			"<tr>"   
			"<th><br>Cokolada<br><br></th>"
			"<td style=""text-align:center""><br> 2 KM<br><br></td>"
			"<td style=""text-align:center""> <br>%d<br><br></td>"
			"</tr>"
			"<tr>"    
			"<th><br>Cips<br><br></th>"
			"<td style=""text-align:center""><br>3 KM<br><br></td>"
			"<td style=""text-align:center""><br>%d<br><br></td>"
			"</tr>"
			"</tbody>"  
			"<tfoot>"
			"<tr>"   
			"<th><br>Dvojni C<br><br></th>"
			"<td style=""text-align:center""><br>2 KM<br><br></td>"
			"<td style=""text-align:center""><br>%d<br><br></td>"
			"</tr>"
			"</tfoot>"
			"</table>"
			"<hr>"
			"<th><h3>RACUN:</th><td>"       
			"<td><th>%d KM</th></h3></tr><tr>"     
			"<hr>"
			"<br><br>"
			"<button onclick=\"location.href='/zavrseno'\" type='button'>"
			"PLATI</button></p>"
			"</center></body></html>"
    }
};


httpd_handle_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    printDEBUG(DSYS, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK)
    {
        // Set URI handlers
        printDEBUG(DSYS, "Registering URI handlers");
        httpd_register_uri_handler(server, &pocetna);
        httpd_register_uri_handler(server, &proizvod);
        httpd_register_uri_handler(server, &racun);
        httpd_register_uri_handler(server, &pocetnaObrisi);
         httpd_register_uri_handler(server, &zavrseno);
         httpd_register_uri_handler(server, &pocetnaDodaj);
        return server;
    }

    printDEBUG(DSYS, "Error starting server!");
    return NULL;
}

void stop_webserver()
{
    // Stop the httpd server
    httpd_stop(server);
    //     
}


static esp_err_t wifiEventHandler(void* userParameter, system_event_t *event)
{
    switch(event->event_id)
    {
        case (SYSTEM_EVENT_AP_STACONNECTED):
        {
            printDEBUG(DSYS," Start http server\n");
            start_webserver();
            break;
        }
        case (SYSTEM_EVENT_AP_STADISCONNECTED):
        {
            printDEBUG(DSYS," Stop http server\n");
            stop_webserver();
            break;
        }
        default:
        {

            break;
        }
    }
    return ESP_OK;
}


static void launchSoftAp()
{
    ESP_ERROR_CHECK(nvs_flash_init());
    tcpip_adapter_init();
    ESP_ERROR_CHECK(tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP));
    tcpip_adapter_ip_info_t ipAddressInfo;
    memset(&ipAddressInfo, 0, sizeof(ipAddressInfo));
    IP4_ADDR(
            &ipAddressInfo.ip,
            SOFT_AP_IP_ADDRESS_1,
            SOFT_AP_IP_ADDRESS_2,
            SOFT_AP_IP_ADDRESS_3,
            SOFT_AP_IP_ADDRESS_4);
    IP4_ADDR(
            &ipAddressInfo.gw,
            SOFT_AP_GW_ADDRESS_1,
            SOFT_AP_GW_ADDRESS_2,
            SOFT_AP_GW_ADDRESS_3,
            SOFT_AP_GW_ADDRESS_4);
    IP4_ADDR(
            &ipAddressInfo.netmask,
            SOFT_AP_NM_ADDRESS_1,
            SOFT_AP_NM_ADDRESS_2,
            SOFT_AP_NM_ADDRESS_3,
            SOFT_AP_NM_ADDRESS_4);
    ESP_ERROR_CHECK(tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_AP, &ipAddressInfo));
    ESP_ERROR_CHECK(tcpip_adapter_dhcps_start(TCPIP_ADAPTER_IF_AP));
    ESP_ERROR_CHECK(esp_event_loop_init(wifiEventHandler, NULL));
    wifi_init_config_t wifiConfiguration = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifiConfiguration));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    wifi_config_t apConfiguration =
    {
        .ap = {
            .ssid = SOFT_AP_SSID,
            .password = SOFT_AP_PASSWORD,
            .ssid_len = 0,
            //.channel = default,
            .authmode = WIFI_AUTH_WPA2_PSK,
            .ssid_hidden = 0,
            .max_connection = 1,
            .beacon_interval = 150,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &apConfiguration));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_start());
}





static void i2c_master_init()
{
   i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = SDA_PIN;
    conf.sda_pullup_en = 1;
    conf.scl_io_num = SCL_PIN;
    conf.scl_pullup_en = 1;
    conf.clk_stretch_tick = 1000000;

i2c_driver_install(I2C_MODE_MASTER, conf.mode);
i2c_param_config(I2C_MODE_MASTER, &conf);
	
}

static void ssd1306_init() {
	esp_err_t espRc;

	i2c_cmd_handle_t cmd = i2c_cmd_link_create();

	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);

	i2c_master_write_byte(cmd, OLED_CMD_SET_CHARGE_PUMP, true);
	i2c_master_write_byte(cmd, 0x14, true);

	i2c_master_write_byte(cmd, OLED_CMD_SET_SEGMENT_REMAP, true); // reverse left-right mapping
	i2c_master_write_byte(cmd, OLED_CMD_SET_COM_SCAN_MODE, true); // reverse up-bottom mapping

	i2c_master_write_byte(cmd, OLED_CMD_DISPLAY_ON, true);
	i2c_master_stop(cmd);

	espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
	if (espRc == ESP_OK) {
		printf("OLED configured successfully");
	} else {
		printf("OLED configuration failed. code: 0x%.2X", espRc);
	}
	i2c_cmd_link_delete(cmd);
}



static void ssd1306_display_clear() {
	i2c_cmd_handle_t cmd;

	uint8_t zero[128]={0};
	for (uint8_t i = 0; i < 8; i++) {
		cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_SINGLE, true);
		i2c_master_write_byte(cmd, 0xB0 | i, true);

		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
		i2c_master_write(cmd, zero, 128, true);
		i2c_master_stop(cmd);
		i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
		i2c_cmd_link_delete(cmd);
	}

	//vTaskDelete(NULL);
}

static void ssd1306_display_text(char* ime) {
	char *text = ime;
	uint8_t text_len = strlen(text);

	i2c_cmd_handle_t cmd;

	uint8_t cur_page = 0;

	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);

	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);
	i2c_master_write_byte(cmd, 0x00, true); // reset column
	i2c_master_write_byte(cmd, 0x10, true);
	i2c_master_write_byte(cmd, 0xB0 | cur_page, true); // reset page

	i2c_master_stop(cmd);
	i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);

	for (uint8_t i = 0; i < text_len; i++) {
		if (text[i] == '\n') {
			cmd = i2c_cmd_link_create();
			i2c_master_start(cmd);
			i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);

			i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);
			i2c_master_write_byte(cmd, 0x00, true); // reset column
			i2c_master_write_byte(cmd, 0x10, true);
			i2c_master_write_byte(cmd, 0xB0 | ++cur_page, true); // increment page

			i2c_master_stop(cmd);
			i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
			i2c_cmd_link_delete(cmd);
		} else {
			cmd = i2c_cmd_link_create();
			i2c_master_start(cmd);
			i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);

			i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
			i2c_master_write(cmd, font8x8_basic_tr[(uint8_t)text[i]], 8, true);

			i2c_master_stop(cmd);
			i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
			i2c_cmd_link_delete(cmd);
		}
	}

	//vTaskDelete(NULL);
}

static void rfid_task()
{
	char ime[15] = "Proizvod:    ";
    char cijena[15] = "    ";
    char naziv[5] = "  ";
    char pric[15] = "Cijena:";
    char razmak[5] ="\n";
	char kupovina[30] = "\n\n\n    Kupovina";
    char zavrsena[15] = "    zavrsena";
	

	uint32_t finish0, finish1, finish2, finish3;
      
        while(1)
	   {
		
		char imeProizvoda[200]="";  
		char gotovo[200]="";
		uint8_t CardID[5];
		if (MFRC522_Check(CardID) == MI_OK)
		{   
			rfidToSend[0] = CardID[0];
			rfidToSend[1] = CardID[1];
			rfidToSend[2] = CardID[2];
			rfidToSend[3] = CardID[3];
			
 		    uint32_t i = 0;
                    uint8_t rfid0, rfid1, rfid2, rfid3;
                    finish0 = (finishShopping & 0xFF000000) >> 24;
                    finish1 = (finishShopping & 0x00FF0000) >> 16;
                    finish2 = (finishShopping & 0x0000FF00) >> 8;
                    finish3 = (finishShopping & 0x000000FF);
                    if(CardID[0] == finish0 && CardID[1] == finish1 && CardID[2] == finish2 && CardID[3] == finish3)
                    {
						
						
					  uint32_t ukupnaCijena = (2 * cokoladaCounter) + (3 * cipsCounter) + (2 * dvojniCounter);
					  	
                      printf("\e[?25l");
					  printf("\e[2J");
					  goto(1,1);
					  printf("\t         Smart Shooping Cart   \t\n");
					  printf("\t------------------------------------\n");
					  printf("\t|        Kupovina zavrsena!        |\n");
					  printf("\t------------------------------------\n");
					  printf("\t|      Ukupna cijena:      KM      |\n");
					  goto(5,31);
					  printf("%d", ukupnaCijena);
					  goto(6,1);
                      printf("\t------------------------------------\n");
                      
                       strcat(gotovo, kupovina);
                       strcat(gotovo, razmak);
                       strcat(gotovo, zavrsena);
                     
                       ssd1306_display_clear();
                       ssd1306_display_text(gotovo);   
                      
		    }
                    while(i < 3)
                    {
                      rfid0 = (articles[i].id & 0xFF000000) >> 24;
                      rfid1 = (articles[i].id & 0x00FF0000) >> 16;
                      rfid2 = (articles[i].id & 0x0000FF00) >> 8;
                      rfid3 = (articles[i].id & 0x000000FF);
                     
                      if(CardID[0] == rfid0 && CardID[1] == rfid1 && CardID[2] == rfid2 && CardID[3] == rfid3)
                      {
                        printf("\e[?25l");
						printf("\e[2J");
						goto(1,1);
							printf("\t         Smart Shooping Cart   \t\n");
						printf("\t------------------------------------\n");
						printf("\t|     Article Name    | Price [KM] |\n");
						printf("\t------------------------------------\n");
						printf("\t|                     |            |\n");
						printf("\t|                     |            |\n");
						goto(6,11);
						printf("%s", articles[i].nameOfArticle);  
						goto(6,35);
						printf("%s", articles[i].price);
						goto(7,1);
						printf("\t|                     |            |\n");
						printf("\t------------------------------------\n");
							
					  
						
						strcat(imeProizvoda, ime);
						strcat(imeProizvoda, razmak);
						strcat(imeProizvoda, naziv);
						strcat(imeProizvoda, articles[i].nameOfArticle);
						strcat(imeProizvoda, razmak);
						strcat(imeProizvoda, razmak);
						strcat(imeProizvoda, pric);
						strcat(imeProizvoda, razmak);
						strcat(imeProizvoda, cijena);
						strcat(imeProizvoda, articles[i].price);			
						
			 
						ssd1306_display_clear();
						ssd1306_display_text(imeProizvoda);      
						
						break;   
					 }
                     
                     ++i;
                    }
                }
			
		vTaskDelay(10/portTICK_PERIOD_MS); 
  }
}



void app_main()
{
    initDEBUG("", '5', 921600, "DSA - Debug example");
    i2c_master_init();
    ssd1306_init();
    MFRC522_Init();
    launchSoftAp();
 
	
    ssd1306_display_clear();
    xTaskCreate(rfid_task, "rfid_task", 1024, NULL, 10, NULL);
}

