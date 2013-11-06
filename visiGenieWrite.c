/*
**	Small command line utility to send data to a 4D Systems display running a VisiGenie project.
**	Allows shell scripts to easily send data to a display.
**
** 	Requires the 4D Systems ViSi-Genie-RaspPi library be already installed as shown at:
**		https://github.com/4dsystems/ViSi-Genie-RaspPi-Library
**
**	Parameters:
**		-w Write Object (not implemented yet)
**		-i Object Index
**		-s String
**		-o Object type
**		-v The value to be sent
**		-x Don't cleanly close the connection to genie, speeds things up but not great ...
**		All non-named paramters are concatenated into a single string which is used for the value if
**			the -v parameter is not set. Note that extra spaces are discarded (unless quoted).
**		The value can also be read from stdin, this is done until EOF is read, and only done if
**			-v is not defined and no non-named parameters are provided i.e. it's the last resort
**			to get the value to send.
** 		So in order of preference for getting value:
**			1 - the -v parameter
**			2 - concatenation of space seperated non-named parameters
**			3 - stdin
**
**	Example Usage:
**		Writes a string to the String3 of the display:
**			visiGenieWrtie -s -i3 -v"The string you want to write"
**		Same but without defining -v:
**			visiGenieWrtie -s -i3 The string you want to write
**		Writes 55 to Slider0 of the dispaly:
**			visiGenieWrite -oslider -o0 -v55
**		The same as above but the value is read from stdin:
**			echo 55 | visiGenieWrite -oslider -o0
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include <geniePi.h>  //the ViSi-Genie-RaspPi library

// valid types of VisiGenie objects on the display, controls which functions are used
enum validObjTypes { strObject, genericObject };

typedef struct OptionsStruct {
	int 	objIndex;
	char 	*value;
	enum 	validObjTypes objType;
	int 	objTypeConst;
	int 	cleanClose;
} OptionsStruct;

void printUsage(void)
{
	fprintf (stderr, "Options:\n");
	fprintf (stderr, " -w Write Object (not implemented yet)\n");
	fprintf (stderr, " -i Object Index\n");
	fprintf (stderr, " -s String\n");
	fprintf (stderr, " -o Object type\n");
	fprintf (stderr, " -v The value to be sent\n");
	fprintf (stderr, " -x Don't cleanly close the connection to genie, speeds things up but not great ...\n");
}

/*
**	parse the command line options
*/
int parseOptions (int argc, char **argv, OptionsStruct *opts)
{
	int c;
	int optargInt = 0;
	int index = 0;
	int paramStringLength = 0;
	
	// default values
	opts->objIndex = 0;
	opts->objType = strObject;
	opts->cleanClose = 1;
	opts->value = (char*) malloc(1);
	opts->value[0] = '\0';
	
	// read the options
	while ((c = getopt (argc, argv, "i:o:v:sx")) != -1)
	{
        switch (c)
        {
			case 'i':
				opts->objIndex = atoi(optarg);
				printf("index == %d\n", opts->objIndex);
				break;
			case 'o':
				optargInt = atoi(optarg);
				printf("objType is genericObject\n");
				opts->objType = genericObject;
				if (strcmp("0", optarg) == 0 ||
					(optargInt != 0 &&
						optargInt >= 0 &&
						optargInt <= 23))
					opts->objTypeConst = optargInt;
				else if (strcmp("dipsw", optarg) == 0)
					opts->objTypeConst = GENIE_OBJ_DIPSW;
				else if (strcmp("knob", optarg) == 0)
					opts->objTypeConst = GENIE_OBJ_KNOB;
				else if (strcmp("rockersw", optarg) == 0)
					opts->objTypeConst = GENIE_OBJ_ROCKERSW;
				else if (strcmp("rotarysw", optarg) == 0)
					opts->objTypeConst = GENIE_OBJ_ROTARYSW;
				else if (strcmp("slider", optarg) == 0)
					opts->objTypeConst = GENIE_OBJ_SLIDER;
				else if (strcmp("trackbar", optarg) == 0)
					opts->objTypeConst = GENIE_OBJ_TRACKBAR;
				else if (strcmp("winbutton", optarg) == 0)
					opts->objTypeConst = GENIE_OBJ_WINBUTTON;
				else if (strcmp("angular_meter", optarg) == 0)
					opts->objTypeConst = GENIE_OBJ_ANGULAR_METER;
				else if (strcmp("cool_gauge", optarg) == 0)
					opts->objTypeConst = GENIE_OBJ_COOL_GAUGE;
				else if (strcmp("custom_digits", optarg) == 0)
					opts->objTypeConst = GENIE_OBJ_CUSTOM_DIGITS;
				else if (strcmp("form", optarg) == 0)
					opts->objTypeConst = GENIE_OBJ_FORM;
				else if (strcmp("image", optarg) == 0)
					opts->objTypeConst = GENIE_OBJ_IMAGE;
				else if (strcmp("keyboard", optarg) == 0)
					opts->objTypeConst = GENIE_OBJ_KEYBOARD;
				else if (strcmp("led", optarg) == 0)
					opts->objTypeConst = GENIE_OBJ_LED;
				else if (strcmp("led_digits", optarg) == 0)
					opts->objTypeConst = GENIE_OBJ_LED_DIGITS;
				else if (strcmp("meter", optarg) == 0)
					opts->objTypeConst = GENIE_OBJ_METER;
				else if (strcmp("strings", optarg) == 0)
					opts->objTypeConst = GENIE_OBJ_STRINGS;
				else if (strcmp("thermometer", optarg) == 0)
					opts->objTypeConst = GENIE_OBJ_THERMOMETER;
				else if (strcmp("user_led", optarg) == 0)
					opts->objTypeConst = GENIE_OBJ_USER_LED;
				else if (strcmp("video", optarg) == 0)
					opts->objTypeConst = GENIE_OBJ_VIDEO;
				else if (strcmp("static_text", optarg) == 0)
					opts->objTypeConst = GENIE_OBJ_STATIC_TEXT;
				else if (strcmp("sound", optarg) == 0)
					opts->objTypeConst = GENIE_OBJ_SOUND;
				else if (strcmp("timer", optarg) == 0)
					opts->objTypeConst = GENIE_OBJ_TIMER;
				else 
				{
					fprintf (stderr, "Object type is not valid '%s'.\n", optarg);
					return 1;
				}
				break;
			case 's':
				printf("objType is strObject\n");
				opts->objType = strObject;
				break;
			case 'v':
				printf("before: value == '%s'\n", opts->value);
				opts->value = optarg;
				printf("after: value == '%s'\n", opts->value);
				break;
			case 'x':
				opts->cleanClose = 0;
				break;
			//case 'w':
				//break;
			case '?':
				fprintf (stderr, "Unknown option `-%c'.\n", optopt);
				printUsage();
				return 1;
			default:
				return 1;
        }
	}
	if (strlen(opts->value) == 0)
	{
		for (index = optind; index < argc; index++)
		{
			// add the length of the parameter string + a space
			paramStringLength += strlen(argv[index]) + 1;
		}
		// make value large enough, note that extra space will be lost
		opts->value = (char*) malloc(paramStringLength + 1);
		for (index = optind; index < argc; index++)
		{
			// add the length of the parameter string + a space
			if (index != optind)
				opts->value[strlen(opts->value)] = ' ';
			strncat(opts->value, argv[index], strlen(argv[index]));
		}
	}
	return 0;
}

/*
** 	pushToDisplay
**	
**	Pushes the given data to the appropriate object described by the options provided.
** 	opts - OptionsStruct with details about the object to be written
**	val - char* to be sent, for object types this will be converted into a uint
*/
int pushToDisplay (OptionsStruct opts, char *val)
{
	int result = 0;
	printf("pushToDisplay: opts->objType == %d\n", opts.objType);
	printf("pushToDisplay: opts->value == %s\n", opts.value);
	if (opts.objType == strObject)
	{
		printf("writing string %d with '%s'\n", opts.objIndex,  val);
		result = genieWriteStr(opts.objIndex, val);
	} else {
		unsigned int valInt = 0;
		valInt = atoi(val);
		if (strcmp("0", val) == 0 ||
			valInt != 0)
		{
			printf("writing object %d type %d with '%s'\n", opts.objIndex, opts.objTypeConst,  val);
			result = genieWriteObj(opts.objTypeConst, opts.objIndex, valInt);
		}
		else 
			result =  1;
	}
	return result;
}

int main(int argc, char **argv)
{
	OptionsStruct opts; // struct for options
	int optStatus = 0; //
  
	// Get the parameters:
	optStatus = parseOptions(argc, argv, &opts);
	
	// if errors reading options then exit with non 0 status
	if (optStatus != 0)
	{
		return 1;
	}
	
	//open the Raspberry Pi's onboard serial port, baud rate is 115200
	//make sure that the display module has the same baud rate
	genieSetup("/dev/ttyAMA0", 115200);  
	
	// If a value is provided send that to the display and exit
	if (strlen(opts.value) > 0)
	{
		pushToDisplay(opts, opts.value);
	// If no value provided on command line the use stdin
	// Need to fix this for non string objects
	} else {
		int continueLoop = 1;
		while (continueLoop)
		{
			char *line = NULL;
			size_t size;
			size_t chars;
			chars = getline(&line, &size, stdin);
			if (chars == -1) {
				continueLoop = 0; // get out of the loop
			} else {
				// remove the newline character
				if (line[chars - 1] == '\n') 
				{
					line[chars - 1] = '\0';
					--chars;
				}
				// push the line to the display
				pushToDisplay(opts, line);
			}
		}
	}
	if (opts.cleanClose == 1)
	{
		genieClose();
	}
	return 0;
}

