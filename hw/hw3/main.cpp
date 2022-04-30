#include "mbed.h"
#include "stm32l475e_iot01_accelero.h"
#include "MQTTNetwork.h"
#include "MQTTmbed.h"
#include "MQTTClient.h"
//---------------------------------------------------------------
#include "hw3.h"
#include "drivers/DigitalOut.h"
#include "erpc_simple_server.h"
#include "erpc_basic_codec.h"
#include "erpc_crc16.h"
#include "UARTTransport.h"
#include "DynamicMessageBufferFactory.h"
#include "hw3_server.h"
















bool start_pulishing = false;



WiFiInterface *wifi;
InterruptIn btn2(BUTTON1);
//InterruptIn btn3(SW3);
volatile int message_num = 0;
volatile int arrivedcount = 0;
volatile bool closed = false;

const char* topic = "Mbed";

Thread mqtt_thread(osPriorityHigh);
EventQueue mqtt_queue;
int16_t pDataXYZ[3] = {0};

Ticker ticker;

void messageArrived(MQTT::MessageData& md) {

    MQTT::Message &message = md.message;
    
    //char msg[300];
    //sprintf(msg, "Message arrived: QoS%d, retained %d, dup %d, packetID %d\r\n", message.qos, message.retained, message.dup, message.id);
   // printf(msg);
   
    ThisThread::sleep_for(100ms);
    
    char payload[300];
    sprintf(payload, "Received message: %.*s\r\n", message.payloadlen, (char*)message.payload);
    printf(payload);

    //++arrivedcount;

}



void publish_message(MQTT::Client<MQTTNetwork, Countdown>* client) {
    
    if(start_pulishing){

        MQTT::Message message;

        char buff[100];
        BSP_ACCELERO_AccGetXYZ(pDataXYZ);
        sprintf(buff, "(%4d, %4d, %4d) \n",pDataXYZ[0], pDataXYZ[1], pDataXYZ[2]);

        message.qos = MQTT::QOS0;
        message.retained = false;
        message.dup = false;
        message.payload = (void*) buff;
        message.payloadlen = strlen(buff) + 1;

        int rc = client->publish(topic, message);

        
        printf("rc = %d, %s", rc, buff);

    }

}


void close_mqtt() {
    closed = true;
}

//------------------------------------------------------------------------------------------------------

#define CONSOLE_FLOWCONTROL_RTS     1
#define CONSOLE_FLOWCONTROL_CTS     2
#define CONSOLE_FLOWCONTROL_RTSCTS  3
#define mbed_console_concat_(x) CONSOLE_FLOWCONTROL_##x
#define mbed_console_concat(x) mbed_console_concat_(x)
#define CONSOLE_FLOWCONTROL mbed_console_concat(MBED_CONF_TARGET_CONSOLE_UART_FLOW_CONTROL)

mbed::DigitalOut led1(LED1, 1);
mbed::DigitalOut led2(LED2, 1);
mbed::DigitalOut led3(LED3, 1); //blue
mbed::DigitalOut* leds[] = { &led1, &led2, &led3 };

/****** erpc declarations *******/

void start(){
    printf("start() called\n");
    start_pulishing = true;
}

void stop(){
    printf("stop() called\n");
    start_pulishing = false;
}

/** erpc infrastructure */
ep::UARTTransport uart_transport(D1, D0, 9600);
ep::DynamicMessageBufferFactory dynamic_mbf;
erpc::BasicCodecFactory basic_cf;
erpc::Crc16 crc16;
erpc::SimpleServer rpc_server;


/** LED service */
LEDBlinkService_service hw3_service;
















int main() {

    BSP_ACCELERO_Init();

    wifi = WiFiInterface::get_default_instance();
    if (!wifi) {
            printf("ERROR: No WiFiInterface found.\r\n");
            return -1;
    }

    printf("\nConnecting to %s...\r\n", MBED_CONF_APP_WIFI_SSID);
    int ret = wifi->connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD, NSAPI_SECURITY_WPA_WPA2);
    
    if (ret != 0) {
            printf("\nConnection error: %d\r\n", ret);
            return -1;
    }

    NetworkInterface* net = wifi;
    MQTTNetwork mqttNetwork(net);
    MQTT::Client<MQTTNetwork, Countdown> client(mqttNetwork);
    const char* host = "192.168.137.1";
    const int port=1883;
    printf("Connecting to TCP network...\r\n");
    printf("address is %s/%d\r\n", host, port);
    
    int rc = mqttNetwork.connect(host, port);//(host, 1883);
    if (rc != 0) {
            printf("Connection error.");
            return -1;
    }
    
    printf("Successfully connected!\r\n");
    
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 3;
    data.clientID.cstring = "Mbed";
    

    if ((rc = client.connect(data)) != 0){
            printf("Fail to connect MQTT\r\n");
    }

    //if (client.subscribe(topic, MQTT::QOS0, messageArrived) != 0){printf("Fail to subscribe\r\n");}
  
    mqtt_thread.start(callback(&mqtt_queue, &EventQueue::dispatch_forever));
    ticker.attach(mqtt_queue.event(&publish_message, &client), 100ms);

    int num = 0;
    
    while (num != 5) {
            client.yield(100);
            ++num;
    }

    /*while (1) {
            if (closed) break;
            client.yield(500);
            ThisThread::sleep_for(500ms);
    }*/

//-------------------------------------------------------------------------------------------------
   
    // Initialize the rpc server
    uart_transport.setCrc16(&crc16);


    // Set up hardware flow control, if needed
#if CONSOLE_FLOWCONTROL == CONSOLE_FLOWCONTROL_RTS
    uart_transport.set_flow_control(mbed::SerialBase::RTS, STDIO_UART_RTS, NC);
#elif CONSOLE_FLOWCONTROL == CONSOLE_FLOWCONTROL_CTS
    uart_transport.set_flow_control(mbed::SerialBase::CTS, NC, STDIO_UART_CTS);
#elif CONSOLE_FLOWCONTROL == CONSOLE_FLOWCONTROL_RTSCTS
    uart_transport.set_flow_control(mbed::SerialBase::RTSCTS, STDIO_UART_RTS, STDIO_UART_CTS);
#endif


    printf("Initializing server.\n");
    rpc_server.setTransport(&uart_transport);
    rpc_server.setCodecFactory(&basic_cf);
    rpc_server.setMessageBufferFactory(&dynamic_mbf);

    // Add the led service to the server
    printf("Adding LED server.\n");
    rpc_server.addService(&hw3_service);
    // Run the server. This should never exit
    printf("Running server.\n");
    rpc_server.run();




    return 0;
}//*/