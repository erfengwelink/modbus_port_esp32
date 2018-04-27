#include "app_ac_dev.h"
#include "app_modbus.h"
#include "app_console.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mkjson.h"


int mkjson_demo( )
{
	char *json = mkjson( MKJSON_OBJ, 13,
		's', "mystr", "hello world!",
		'i', "myinteger", 42,
		'I', "longlong", 784ll,
		'd', "double", 1.4481,
		'D', "longdbl", 1.22l,
		'e', "exponential", 3e62,
		'E', "exponentialL", 3e104l,	
		'b', "boolean", 1,
		'n', "nullvalue",
		'r', "alsonull", NULL,
		'j', "object", mkjson( MKJSON_OBJ, 1, 
			'i', "something", 45 
			),
		'j', "array", mkjson( MKJSON_ARR, 3, 
			'i', 123,
			's', "another string!",
			'b', 0 ),
		'j', "empty", mkjson( MKJSON_OBJ, 0 )
		);
	
	//Just print it and then free
	assert( json != NULL );
	printf( "%s\n", json );
	free( json );
	return 0;
}

void app_main()
{

    xTaskCreate(modebus_task,"modebus_task",1024*80,NULL,5,NULL);

    mkjson_demo( );
    app_console_init();
    app_console_run();
}