/*******************************************************************************
 * Copyright (c) 2012, 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution. 
 *
 * The Eclipse Public License is available at 
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at 
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Allan Stockdill-Mander - initial API and implementation and/or initial documentation
 *******************************************************************************/


/**
 * @file
 * SSL tests for the MQ Telemetry MQTT C client
 */

#include "MQTTClient.h"
#include <string.h>
#include <stdlib.h>

#if !defined(_WINDOWS)
	#include <sys/time.h>
  	#include <sys/socket.h>
	#include <unistd.h>
  	#include <errno.h>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#define MAXHOSTNAMELEN 256
#define EAGAIN WSAEWOULDBLOCK
#define EINTR WSAEINTR
#define EINPROGRESS WSAEINPROGRESS
#define EWOULDBLOCK WSAEWOULDBLOCK
#define ENOTCONN WSAENOTCONN
#define ECONNRESET WSAECONNRESET
#define snprintf _snprintf
#define setenv(a, b, c) _putenv_s(a, b)
#endif

#if defined(IOS)
char skeytmp[1024];
char ckeytmp[1024];
char persistenceStore[1024];
#else
char* persistenceStore = NULL;
#endif

#if 0
#include <logaX.h>   /* For general log messages                      */
#define MyLog logaLine
#else
#define LOGA_DEBUG 0
#define LOGA_INFO 1
#include <stdarg.h>
#include <time.h>
#include <sys/timeb.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

void usage()
{
	printf("Options:\n");
	printf("\t--test_no <test_no> - Run test number <test_no>\n");
	printf("\t--connection <mqtt URL> - Connect to <mqtt URL> for tests\n");
	printf("\t--haconnections \"<mqtt URLs>\" - Use \"<mqtt URLs>\" as the list of servers for HA tests (space separated)\n");
	printf("\t--client_key <key_file> - Use <key_file> as the client certificate for SSL authentication\n");
	printf("\t--client_key_pass <password> - Use <password> to access the private key in the client certificate\n");
	printf("\t--client_privatekey <file> - Client private key file if not in certificate file\n");
	printf("\t--server_key <key_file> - Use <key_file> as the trusted certificate for server\n");
	printf("\t--verbose - Enable verbose output \n");
	printf("\tserver connection URLs should be in the form; (tcp|ssl)://hostname:port\n");
	printf("\t--help - This help output\n");
	exit(-1);
}

struct Options
{
	char connection[100];
	char mutual_auth_connection[100];   /**< connection to system under test. */
	char nocert_mutual_auth_connection[100];
	char server_auth_connection[100];
	char anon_connection[100];
	char** haconnections;         	/**< connection to system under test. */
	int hacount;
	char* client_key_file;
	char* client_key_pass;
	char* server_key_file;
	char* client_private_key_file;
	int verbose;
	int test_no;
} options =
{
	"mqtt.hates.software:8883",
	"mqtt.hates.software:8883",
	"mqtt.hates.software:8883",
	"mqtt.hates.software:8883",
	"mqtt.hates.software:8883",
	NULL,
	0,
	"/etc/ssl/certs/client.hates.software.crt",
	NULL,
	"/etc/ssl/certs/ca.crt",
	NULL,
	0,
	0,
};


char* test_map[] = { "none", "2a_m"};


void getopts(int argc, char** argv)
{
	int count = 1;
	
	while (count < argc)
	{
		if (strcmp(argv[count], "--help") == 0)
			usage();
		else if (strcmp(argv[count], "--test_no") == 0)
		{
			if (++count < argc)
			{
				int i;

				for (i = 1; i < ARRAY_SIZE(test_map); ++i)
				{
					if (strcmp(argv[count], test_map[i]) == 0)
					{
						options.test_no = i;
						break;
					}
				}
				if (options.test_no == 0)
					options.test_no = atoi(argv[count]);
				
			}
			else
				usage();
		}
		else if (strcmp(argv[count], "--hostname") == 0)
		{
			if (++count < argc)
			{
				sprintf(options.connection, "ssl://%s:8883", argv[count]);
				printf("Setting connection to %s\n", options.connection);
			}
			else
				usage();
		}
		else if (strcmp(argv[count], "--haconnections") == 0)
		{
			if (++count < argc)
			{
				char* tok = strtok(argv[count], " ");
				options.hacount = 0;
				options.haconnections = malloc(sizeof(char*) * 5);
				while (tok)
				{
					options.haconnections[options.hacount] = malloc(strlen(tok) + 1);
					strcpy(options.haconnections[options.hacount], tok);
					options.hacount++;
					tok = strtok(NULL, " ");
				}
			}
			else
				usage();
		}
		else if (strcmp(argv[count], "--client_key") == 0)
		{
			if (++count < argc)
        		{
#if defined(IOS)
        		        strcat(ckeytmp, getenv("HOME"));
        		        strcat(ckeytmp, argv[count]);
        		        options.client_key_file = ckeytmp;
#else
				options.client_key_file = argv[count];
#endif
        		}
			else
				usage();
		}
		else if (strcmp(argv[count], "--client_privatekey") == 0)
		{
			if (++count < argc)
        		{
#if defined(IOS)
        		        strcat(ckeytmp, getenv("HOME"));
        		        strcat(ckeytmp, argv[count]);
        		        options.client_private_key_file = ckeytmp;
#else
				options.client_private_key_file = argv[count];
#endif
        		}
			else
				usage();
		}
		else if (strcmp(argv[count], "--client_key_pass") == 0)
		{
			if (++count < argc)
				options.client_key_pass = argv[count];
			else
				usage();
		}
		else if (strcmp(argv[count], "--server_key") == 0)
		{
			if (++count < argc)
			{
#if defined(IOS)
		                strcat(skeytmp, getenv("HOME"));
		                strcat(skeytmp, argv[count]);
		                options.server_key_file = skeytmp;
#else
		                options.server_key_file = argv[count];
#endif
			}
			else
				usage();
		}
		else if (strcmp(argv[count], "--verbose") == 0)
		{
			options.verbose = 1;
			//TODO
			printf("\nSetting verbose on\n");
		}
		count++;
	}
#if defined(IOS)
	strcpy(persistenceStore, getenv("HOME"));
	strcat(persistenceStore, "/Library/Caches");
#endif
}

void MyLog(int LOGA_level, char* format, ...)
{
	static char msg_buf[2048];
	va_list args;
	struct timeb ts;

	struct tm *timeinfo;

	if (LOGA_level == LOGA_DEBUG && options.verbose == 0)
	  return;
	
	ftime(&ts);
	timeinfo = localtime(&ts.time);
	strftime(msg_buf, 80, "%Y%m%d %H%M%S", timeinfo);

	sprintf(&msg_buf[strlen(msg_buf)], ".%.3hu ", ts.millitm);

	va_start(args, format);
	vsnprintf(&msg_buf[strlen(msg_buf)], sizeof(msg_buf) - strlen(msg_buf), format, args);
	va_end(args);

	printf("%s\n", msg_buf);
	fflush(stdout);
}
#endif


#if defined(WIN32) || defined(_WINDOWS)
#define mqsleep(A) Sleep(1000*A)
#define START_TIME_TYPE DWORD
static DWORD start_time = 0;
START_TIME_TYPE start_clock(void)
{
	return GetTickCount();
}
#elif defined(AIX)
#define mqsleep sleep
#define START_TIME_TYPE struct timespec
START_TIME_TYPE start_clock(void)
{
	static struct timespec start;
	clock_gettime(CLOCK_REALTIME, &start);
	return start;
}
#else
#define mqsleep sleep
#define START_TIME_TYPE struct timeval
/* TODO - unused - remove? static struct timeval start_time; */
START_TIME_TYPE start_clock(void)
{
	struct timeval start_time;
	gettimeofday(&start_time, NULL);
	return start_time;
}
#endif


#if defined(WIN32)
long elapsed(START_TIME_TYPE start_time)
{
	return GetTickCount() - start_time;
}
#elif defined(AIX)
#define assert(a)
long elapsed(struct timespec start)
{
	struct timespec now, res;

	clock_gettime(CLOCK_REALTIME, &now);
	ntimersub(now, start, res);
	return (res.tv_sec)*1000L + (res.tv_nsec)/1000000L;
}
#else
long elapsed(START_TIME_TYPE start_time)
{
	struct timeval now, res;

	gettimeofday(&now, NULL);
	timersub(&now, &start_time, &res);
	return (res.tv_sec)*1000 + (res.tv_usec)/1000;
}
#endif


#define assert(a, b, c, d) myassert(__FILE__, __LINE__, a, b, c, d)
#define assert1(a, b, c, d, e) myassert(__FILE__, __LINE__, a, b, c, d, e)


int tests = 0;
int failures = 0;
FILE* xml;
START_TIME_TYPE global_start_time;
char output[3000];
char* cur_output = output;


void write_test_result()
{
	long duration = elapsed(global_start_time);

	fprintf(xml, " time=\"%ld.%.3ld\" >\n", duration / 1000, duration % 1000); 
	if (cur_output != output)
	{
		fprintf(xml, "%s", output);
		cur_output = output;	
	}
	fprintf(xml, "</testcase>\n");
}


int myassert(char* filename, int lineno, char* description, int value, char* format, ...)
{
	++tests;
	if (!value)
	{
		va_list args;

		++failures;
		printf("Assertion failed, file %s, line %d, description: %s\n", filename, lineno, description);

		va_start(args, format);
		vprintf(format, args);
		va_end(args);

		cur_output += sprintf(cur_output, "<failure type=\"%s\">file %s, line %d </failure>\n", 
                        description, filename, lineno);
	}
	else
		MyLog(LOGA_DEBUG, "Assertion succeeded, file %s, line %d, description: %s", filename, lineno, description);  
	return value;
}


/*********************************************************************

Test: single-threaded client

*********************************************************************/
void singleThread_sendAndReceive(MQTTClient* c, int qos, char* test_topic)
{
	MQTTClient_deliveryToken dt;
	MQTTClient_message pubmsg = MQTTClient_message_initializer;
	MQTTClient_message* m = NULL;
	char* topicName = NULL;
	int topicLen;
	int i = 0;
	int iterations = 50;
	int rc;

	MyLog(LOGA_DEBUG, "%d messages at QoS %d", iterations, qos);
	pubmsg.payload = "a much longer message that we can shorten to the extent that we need to payload up to 11";
	pubmsg.payloadlen = 11;
	pubmsg.qos = qos;
	pubmsg.retained = 0;

	for (i = 0; i< iterations; ++i)
	{
		if (i % 10 == 0)
			rc = MQTTClient_publish(c, test_topic, pubmsg.payloadlen, pubmsg.payload, pubmsg.qos, pubmsg.retained, NULL);
		else
			rc = MQTTClient_publishMessage(c, test_topic, &pubmsg, &dt);
		assert("Good rc from publish", rc == MQTTCLIENT_SUCCESS, "rc was %d", rc);

		if (qos > 0)
		{
			rc = MQTTClient_waitForCompletion(c, dt, 20000L);
			assert("Good rc from waitforCompletion", rc == MQTTCLIENT_SUCCESS, "rc was %d", rc);
		}

		rc = MQTTClient_receive(c, &topicName, &topicLen, &m, 10000);
		assert("Good rc from receive", rc == MQTTCLIENT_SUCCESS, "rc was %d", rc);
		if (topicName)
		{
			MyLog(LOGA_DEBUG, "Message received on topic %s is %.*s", topicName, m->payloadlen, (char*)(m->payload));
			if (pubmsg.payloadlen != m->payloadlen ||
					memcmp(m->payload, pubmsg.payload, m->payloadlen) != 0)
			{
				failures++;
				MyLog(LOGA_INFO, "Error: wrong data - received lengths %d %d", pubmsg.payloadlen, m->payloadlen);
				break;
			}
			MQTTClient_free(topicName);
			MQTTClient_freeMessage(&m);
		}
		else
			printf("No message received within timeout period\n");
	}

	/* receive any outstanding messages */
	MQTTClient_receive(c, &topicName, &topicLen, &m, 1000);
	while (topicName)
	{
		printf("Message received on topic %s is %.*s.\n", topicName, m->payloadlen, (char*)(m->payload));
		MQTTClient_free(topicName);
		MQTTClient_freeMessage(&m);
		MQTTClient_receive(c, &topicName, &topicLen, &m, 1000);
	}
}

/*********************************************************************

Test: multi-threaded client using callbacks

*********************************************************************/
volatile int multiThread_arrivedcount = 0;
int multiThread_deliveryCompleted = 0;
MQTTClient_message multiThread_pubmsg = MQTTClient_message_initializer;

void multiThread_deliveryComplete(void* context, MQTTClient_deliveryToken dt)
{
	++multiThread_deliveryCompleted;
}

int multiThread_messageArrived(void* context, char* topicName, int topicLen, MQTTClient_message* m)
{
	++multiThread_arrivedcount;
	MyLog(LOGA_DEBUG, "Callback: %d message received on topic %s is %.*s.",
					multiThread_arrivedcount, topicName, m->payloadlen, (char*)(m->payload));
	if (multiThread_pubmsg.payloadlen != m->payloadlen ||
					memcmp(m->payload, multiThread_pubmsg.payload, m->payloadlen) != 0)
	{
		failures++;
		MyLog(LOGA_INFO, "Error: wrong data received lengths %d %d\n", multiThread_pubmsg.payloadlen, m->payloadlen);
	}
	MQTTClient_free(topicName);
	MQTTClient_freeMessage(&m);
	return 1;
}


void multiThread_sendAndReceive(MQTTClient* c, int qos, char* test_topic)
{
	MQTTClient_deliveryToken dt;
	int i = 0;
	int iterations = 50;
	int rc = 0;
	int wait_seconds = 0;

	multiThread_deliveryCompleted = 0;
	multiThread_arrivedcount = 0;

	MyLog(LOGA_DEBUG, "%d messages at QoS %d", iterations, qos);
	multiThread_pubmsg.payload = "a much longer message that we can shorten to the extent that we need to";
	multiThread_pubmsg.payloadlen = 27;
	multiThread_pubmsg.qos = qos;
	multiThread_pubmsg.retained = 0;

	for (i = 1; i <= iterations; ++i)
	{
		if (i % 10 == 0)
			rc = MQTTClient_publish(c, test_topic, multiThread_pubmsg.payloadlen, multiThread_pubmsg.payload, 
                   multiThread_pubmsg.qos, multiThread_pubmsg.retained, NULL);
		else
			rc = MQTTClient_publishMessage(c, test_topic, &multiThread_pubmsg, &dt);
		assert("Good rc from publish", rc == MQTTCLIENT_SUCCESS, "rc was %d", rc);

		#if defined(WIN32)
			Sleep(100);
		#else
			usleep(100000L);
		#endif

		wait_seconds = 20;
		while ((multiThread_arrivedcount < i) && (wait_seconds-- > 0))
		{
			MyLog(LOGA_DEBUG, "Arrived %d count %d", multiThread_arrivedcount, i);
			#if defined(WIN32)
				Sleep(1000);
			#else
				usleep(1000000L);
			#endif
		}
		assert("Message Arrived", wait_seconds > 0, 
				"Time out waiting for message %d\n", i );
	}
	if (qos > 0)
	{
		/* MQ Telemetry can send a message to a subscriber before the server has
		   completed the QoS 2 handshake with the publisher. For QoS 1 and 2,
		   allow time for the final delivery complete callback before checking
		   that all expected callbacks have been made */ 
		wait_seconds = 10;
		while ((multiThread_deliveryCompleted < iterations) && (wait_seconds-- > 0))
		{
			MyLog(LOGA_DEBUG, "Delivery Completed %d count %d", multiThread_deliveryCompleted, i);
			#if defined(WIN32)
				Sleep(1000);
			#else
				usleep(1000000L);
			#endif
		}
		assert("All Deliveries Complete", wait_seconds > 0, 
			   "Number of deliveryCompleted callbacks was %d\n", 
			   multiThread_deliveryCompleted);
	}
}

/*********************************************************************
Test2a: Mutual SSL Authentication - Certificates in place on client and server - multi threaded
*********************************************************************/

int test2a_m(struct Options options)
{
	char* testname = "test2a_m";
	char* test_topic = "C client test2a_m";
	int subsqos = 2;
	/* TODO - usused - remove ? MQTTClient_deliveryToken* dt = NULL; */
	MQTTClient c;
	MQTTClient_connectOptions opts = MQTTClient_connectOptions_initializer;
	MQTTClient_willOptions wopts = MQTTClient_willOptions_initializer;
	MQTTClient_SSLOptions sslopts = MQTTClient_SSLOptions_initializer;
	int rc = 0;

	failures = 0;
	MyLog(LOGA_INFO, "Starting test 2a_m - Mutual SSL authentication - multi-threaded client using callbacks");
	fprintf(xml, "<testcase classname=\"test3\" name=\"test 2a_m\"");
	global_start_time = start_clock();

	rc = MQTTClient_create(&c, options.mutual_auth_connection, "test2a_m", MQTTCLIENT_PERSISTENCE_DEFAULT, persistenceStore);
	if (!(assert("good rc from create", rc == MQTTCLIENT_SUCCESS, "rc was %d\n", rc)))
		goto exit;

	opts.keepAliveInterval = 20;
	opts.cleansession = 1;
	opts.username = NULL;
	opts.password = NULL;
	if (options.haconnections != NULL)
	{
		opts.serverURIs = options.haconnections;
		opts.serverURIcount = options.hacount;
	}

	opts.ssl = &sslopts;
        opts.ssl->enabledCipherSuites = "ECDHE-RSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-RSA-AES128-SHA256:ECDHE-RSA-AES128-SHA:ECDHE-RSA-AES256-SHA384:ECDHE-RSA-AES256-SHA:ECDHE-RSA-RC4-SHA:AES128-GCM-SHA256:AES256-GCM-SHA384:AES128-SHA256:AES128-SHA:AES256-SHA256:AES256-SHA:RC4-SHA:!aNULL:!eNULL:!LOW:!3DES:!MD5:!EXP:!PSK:!SRP:!DSS:!EDH";
	// opts.ssl->enableServerCertAuth = 0;

	if (options.server_key_file) 
	        MyLog(LOGA_INFO, "Using server key file %s\n",options.server_key_file);
		opts.ssl->trustStore = options.server_key_file; /*file of certificates trusted by client*/
	MyLog(LOGA_INFO, "Using client key file %s\n",options.client_key_file);
	opts.ssl->keyStore = options.client_key_file;  /*file of certificate for client to present to server*/
	if (options.client_key_pass) 
                MyLog(LOGA_INFO, "Using password %s\n",options.client_key_pass);
		opts.ssl->privateKeyPassword = options.client_key_pass;
	if (options.client_private_key_file) 
                MyLog(LOGA_INFO, "Using client private key file: %s\n",options.client_private_key_file);
		opts.ssl->privateKey = options.client_private_key_file;

                MyLog(LOGA_INFO, "Using cipher suites: %s\n",opts.ssl->enabledCipherSuites);

	rc = MQTTClient_setCallbacks(c, NULL, NULL, multiThread_messageArrived, multiThread_deliveryComplete);
	if (!(assert("Good rc from setCallbacks", rc == MQTTCLIENT_SUCCESS, "rc was %d", rc)))
		goto exit;

	MyLog(LOGA_DEBUG, "Connecting");

	rc = MQTTClient_connect(c, &opts);
	if (!(assert("Good rc from connect", rc == MQTTCLIENT_SUCCESS, "rc was %d", rc)))
		goto exit;

	rc = MQTTClient_subscribe(c, test_topic, subsqos);
	if (!(assert("Good rc from subscribe", rc == MQTTCLIENT_SUCCESS, "rc was %d", rc)))
		goto exit;

	multiThread_sendAndReceive(c, 0, test_topic);
	multiThread_sendAndReceive(c, 1, test_topic);
	multiThread_sendAndReceive(c, 2, test_topic);

	MyLog(LOGA_DEBUG, "Stopping");

	rc = MQTTClient_unsubscribe(c, test_topic);
	if (!(assert("Unsubscribe successful", rc == MQTTCLIENT_SUCCESS, "rc was %d", rc)))
		goto exit;
	rc = MQTTClient_disconnect(c, 0);
	if (!(assert("Disconnect successful", rc == MQTTCLIENT_SUCCESS, "rc was %d", rc)))
		goto exit;

exit:
	MQTTClient_destroy(&c);
	MyLog(LOGA_INFO, "%s: test %s. %d tests run, %d failures.",
			(failures == 0) ? "passed" : "failed", testname, tests, failures);
	write_test_result();
	return failures;
}


typedef struct
{
	char* clientID;					/**< the string id of the client */
	char* username;					/**< MQTT v3.1 user name */
	char* password;					/**< MQTT v3.1 password */
	unsigned int cleansession : 1;			/**< MQTT clean session flag */
	unsigned int connected : 1;			/**< whether it is currently connected */
	unsigned int good : 1; 			/**< if we have an error on the socket we turn this off */
	unsigned int ping_outstanding : 1;
	unsigned int connect_state : 2;
	int socket;
	int msgID;
	int keepAliveInterval;
	int retryInterval;
	int maxInflightMessages;
	time_t lastContact;
	void* will;
	void* inboundMsgs;
	void* outboundMsgs;				/**< in flight */
	void* messageQueue;
	void* phandle;  /* the persistence handle */
	MQTTClient_persistence* persistence; /* a persistence implementation */
	int connectOptionsVersion;
} Clients;

typedef struct
{
	char* serverURI;
	Clients* c;
	MQTTClient_connectionLost* cl;
	MQTTClient_messageArrived* ma;
	MQTTClient_deliveryComplete* dc;
	void* context;

	int connect_sem;
	int rc; /* getsockopt return code in connect */
	int connack_sem;
	int suback_sem;
	int unsuback_sem;
	void* pack;
} MQTTClients;


int main(int argc, char** argv)
{
	int* numtests = &tests;
	int rc = 0;
 	int (*tests[])() = {NULL, test2a_m,};
	//MQTTClient_nameValue* info;

	xml = fopen("TEST-test3.xml", "w");
	fprintf(xml, "<testsuite name=\"test3\" tests=\"%d\">\n", (int)(ARRAY_SIZE(tests) - 1));
    
	setenv("MQTT_C_CLIENT_TRACE", "ON", 1);
	setenv("MQTT_C_CLIENT_TRACE_LEVEL", "ERROR", 1);
	getopts(argc, argv);
 	if (options.test_no == 0)
	{ /* run all the tests */
		for (options.test_no = 1; options.test_no < ARRAY_SIZE(tests); ++options.test_no)
			rc += tests[options.test_no](options); /* return number of failures.  0 = test succeeded */
	}
	else
		rc = tests[options.test_no](options); /* run just the selected test */
    	
	MyLog(LOGA_INFO, "Total tests run: %d", *numtests);
	if (rc == 0)
		MyLog(LOGA_INFO, "verdict pass");
	else
		MyLog(LOGA_INFO, "verdict fail");

	fprintf(xml, "</testsuite>\n");
	fclose(xml);
	
	return rc;
}

