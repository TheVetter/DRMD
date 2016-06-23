#include "drmd.h"
#include "Python.h"


// typedef enum { TRUE, FALSE } Bool;
// typedef enum { MOVE_STEPPER, READ_UV, UI } State;

static void       cleanAndExit();
static void       configInput(uint8_t, uint8_t);
static void       configOutput(uint8_t, uint8_t);
static int64_t    getTimestampNs();
static int        initialize();
// static void       interrupt(int);
// static State      machine(State);
static uint8_t    readPin(uint8_t);
static void       sleepNs(long);
static int 		  moveStepperToTarget(int)
static int        readUV();

// static State      stateUI();
static void       writeToPin(uint8_t, uint8_t);

// static Bool       sigint_set = FALSE;
// static State      state = UI;
static uint8_t    buffer[2];
static uint16_t   voltage;
static int        stepper_position = 0; // In microsteps, not mm
// static int        stepper_target = 0; // In microsteps, not mm
static uint16_t   stepper_rpm = 100;

#define MIN_RPM         50
#define MAX_RPM         800
#define USTEP_PER_REV   6400 // Number of microsteps per revolution

// int main() {
  // if (initialize()) return 1; // exit if there was an error initializing

  // // while (1) {
    // // if (sigint_set == TRUE) {
      // // state = UI;
      // // printf("\n");

      // sigint_set = FALSE;
    // }

    // state = machine(state);
  // }

  // return 0;
// }

static void cleanAndExit() {
  configInput(PDN1, PULLDOWN);
  configInput(PDN3, PULLDOWN);

  configInput(nENBL, NOPULL);
  configInput(nSLEEP, NOPULL);
  configInput(nRESET, NOPULL);
  configInput(DECAY, NOPULL);
  configInput(STEP, NOPULL);
  configInput(DIR, NOPULL);

  configInput(MODE0, NOPULL);
  configInput(MODE1, NOPULL);
  configInput(MODE2, NOPULL);

  configInput(nHOME, NOPULL);

  // exit(0);
}

static void configInput(uint8_t pin, uint8_t pullmode) {
  bcm2835_gpio_fsel(pin, INPUT);
  bcm2835_gpio_set_pud(pin, pullmode);
}

static void configOutput(uint8_t pin, uint8_t logiclevel) {
  bcm2835_gpio_fsel(pin, OUTPUT);
  writeToPin(pin, logiclevel);
}

static int64_t getTimestampNs() {
  struct timespec currenttime;
  clock_gettime(CLOCK_MONOTONIC, &currenttime);
  return currenttime.tv_sec * 1e9 + currenttime.tv_nsec;
}

static void writeToPin(uint8_t pin, uint8_t logiclevel) {
  bcm2835_gpio_write(pin, logiclevel);
}

static PyObject* cleanUp(PyObject* self){
	cleanAndExit();
	Py_RETURN_NONE;
}

static PyMethodDef module_methods[] = {
    { "moveStepper", (PyCFunction)moveStepper, METH_VARARGS, "move the Stepper motor in mm" },
    { "getUVsample", (PyCFunction)getUVSample, METH_NOARGS, "get a sample from the uv PD" },
    { "cleanUp", (PyCFunction)cleanUp, METH_NOARGS, "kill running stuff" },
	{ "initDevice", (PyCFunction)initDevice, METH_NOARGS, "initialize stuff" },
    { NULL, NULL, 0, NULL }
};



PyMODINIT_FUNC initDevice() {
  
  Py_InitModule3(initializeDevice, module_methods, "docstring...");
}

static PyObject* initDevice(PyObject* self){
	
	if(initialize()!= 0){
		Py_BuildValue("i", 1)// deveice init properly 
	}
	
	Py_BuildValue("i", 0)// deveice didn't init properly
}

static int initialize() {
  if (!bcm2835_init()) {
    printf("Error: bcm2835_init failed. Must run as root.\n");
    return 1;
  }

  bcm2835_i2c_begin();
  bcm2835_i2c_set_baudrate(100000);

  bcm2835_i2c_setSlaveAddress(0b00010100); // Select UV LED ADC
  buffer[0] = 0b10100000; // Code to enable continuous calibration on ADC
  bcm2835_i2c_write(buffer, 1);

  configOutput(PDN1, LOW); // UV LED off
  configOutput(PDN3, LOW); // Peristaltic pump off

  configOutput(nENBL, HIGH); // Disable output drivers
  configOutput(nSLEEP, HIGH); // Enable internal logic
  configOutput(nRESET, HIGH); // Remove reset condition
  configOutput(DECAY, HIGH); // Mixed decay
  configOutput(STEP, LOW); // Ready to provide rising edge for step
  configOutput(DIR, LOW); // Forward direction

  // Use 1/32 microstepping
  configOutput(MODE0, HIGH);
  configOutput(MODE1, HIGH);
  configOutput(MODE2, HIGH);

  configInput(nHOME, PULLUP);

  // signal(SIGINT, interrupt); // Set interrupt(int) to handle Ctrl+c events

  return 0;
}

// // static void interrupt(int signo) {
  // // if (signo == SIGINT) {
    // // if (state == UI) {
      // // printf("\n");
      // // cleanAndExit();
    // // } else if (state == MOVE_STEPPER) {      
      // // configOutput(nENBL, HIGH); // Disable output drivers
    // // }
    // // sigint_set = TRUE;
  // // }
// }

// static State machine(State state) {
  // switch (state) {
    // case MOVE_STEPPER:
      // return stateMoveStepper();
    // case READ_UV:
      // return stateReadUV();
    // case UI:
      // return stateUI();
  // }
// }

static uint8_t readPin(uint8_t pin) {
  return bcm2835_gpio_lev(pin);
}

/* Only works for nanosecond values between 0 and 1 second, non inclusive */
static void sleepNs(long nanoseconds) {
  nanosleep((const struct timespec[]){{0, nanoseconds}}, NULL);
}

static PyObject* moveStepper(PyObject* self, PyObject* args){
	int target;
	int atTarget = 0; // 0 not at target, 1 at target 
	if(!PyArg_ParseTuple(args, "i", &target)){
		return NULL;
	}
	
	int relTarget = checkTargetPosition(target); //converted target 
	
	if(!relTarget==0){
		atTarget = moveStepperToTarget(relTarget);
	}
	
	return Py_BuildValue("i", atTarget);
}	



/* Max Resolution: .01 mm, Range: [-100 mm, 100 mm] */
int checkTargetPosition(int distance_mm) {
  if (distance_mm < -100 || distance_mm > 100) { // check target
    printf("Error: Distance must be between -100.00 mm and 100.00 mm inclusive.\n");
    return 0;
  }

  // Convert to a whole number of .01 mm steps and then multiply by the
  // number of microsteps necessary to move .01 mm (64 microsteps).
  int num_steps = ((int)(distance_mm * 100)) * 64;
  
  if (num_steps != 0) {
    return stepper_position + num_steps; // target 
     
  } else {
    printf("Error: Maximum resolution is 0.01 mm.\n");
  }

  return 0;//should never reach 
}


//toggle led state 
static void setLED(int state){
	if(state != 0){
		writeToPin(PDN1, HIGH); // turn led on 
	
	}else{
		writeToPin(PDN1, LOW);// turn led off 
	}
	
	
}




//lets python get sample 
static PyObject* getUVSample(PyObject* self){
	
	return Py_BuildValue("i", readUV());
}

//get reading from photodiode 
int readUV() { 
  int count = 0;
  int sum = 0;
  
  setLED(1);

  bcm2835_i2c_setSlaveAddress(0b00010100); // Select UV PD ADC
	while(count < 20){ 
	
	  switch (bcm2835_i2c_read(buffer, 2)) {
		case BCM2835_I2C_REASON_OK:
		  voltage = buffer[0] << 8 | buffer[1];
		  sum += voltage;
		  count++;
		  break;
		case BCM2835_I2C_REASON_ERROR_NACK:
		case BCM2835_I2C_REASON_ERROR_CLKT:
		case BCM2835_I2C_REASON_ERROR_DATA:
		  break;
	  } 
	}
  // if (count >= 20) {			//average 20 samples 
   // count = 0;
   // sum = 0;
   return sum / count; //65536 * 100
  

  
}


// return true if at target 
static int moveStepperToTarget(int stepper_target) {
	int64_t startTime_ns = getTimestampNs();
	
	int64_t step_interval_ns = 60e9 / ((int64_t) stepper_rpm * USTEP_PER_REV);
  
  	while(stepper_position != stepper_target){
	
		int64_t currenttime_ns = getTimestampNs();
	
		int64_t elapsed_ns = currenttime_ns - startTime_ns;

		if (elapsed_ns >= step_interval_ns) {
		
			if (stepper_target - stepper_position > 0) {
				
				if (readPin(DIR) != 0){
					writeToPin(DIR, LOW); // set direction
					stepper_position++;
				}
			
			} else {
				
				if (readPin(DIR) != 1){
					writeToPin(DIR, HIGH); // set direction
					stepper_position--;
				}
			}

			writeToPin(STEP, HIGH); // run stepper motor
			sleepNs(5000); // 5 microseconds
			writeToPin(STEP, LOW);// stop stepper 

			startTime_ns = currenttime_ns;
		}
    //printf("(position = %d, elapsed_ns = %lld, nHOME = %d)\n", stepper_position, elapsed_ns, readPin(nHOME));
	}

    if (stepper_position == stepper_target) {
		configOutput(nENBL, HIGH); // Disable output drivers
		return 1;
	}
	else{
		return moveStepperToTarget(stepper_target);
	}
}

// static State stateUI() {
  // static char command[100];

  // printf("> ");
  // fgets(command, 100, stdin);

  // if (strncmp(command, "exit", 4) == 0) {
    // cleanAndExit();
  // } else if (strncmp(command, "read uv", 7) == 0) {
    // printf("Press (ctrl+c) to stop.\n");
    // return READ_UV;
  // } else if (strncmp(command, "move", 4) == 0) 
  
  // {
    // float distance_mm = 0;
    
    // if (command[4] != '\0' && command[5] != '\0') {
      // if (sscanf(&command[5], "%f", &distance_mm)) {
        // if (setTargetPosition(distance_mm)) return UI;

        // writeToPin(nENBL, LOW); // Enable output drivers
        // printf("Moving %d micrometers... Press (ctrl+c) to stop.\n", (stepper_target-stepper_position)*10/64);
        // sleepNs(1e6); // Allow drivers time to enable
        // return MOVE_STEPPER;
      // } else {
        // printf("Error: Distance must be an integer.\n");
      // }
    // } else {
      // printf("Error: Must provide an integer distance X to move (move stepper X).\n");
    // }
  // } else if (strncmp(command, "stepper rpm", 11) == 0) {
    // int rpm = 0;
    
    // if (command[11] != '\0' && command[12] != '\0') {
      // if (sscanf(&command[12], "%d", &rpm)) {
        // if (rpm >= MIN_RPM && rpm <= MAX_RPM) {
          // stepper_rpm = (uint16_t) rpm;
          // printf("Stepper speed successfully set to %d rpm.\n", stepper_rpm);
        // } else {
          // printf("Error: rpm must be between %d and %d inclusive.\n", MIN_RPM, MAX_RPM);
        // }
      // } else {
        // printf("Error: Third parameter must be an integer.\n");
      // }
    // } else {
      // printf("Error: Must provide an integer rpm X (stepper rpm X).\n");
    // }
  // } else if (strncmp(command, "pump on", 7) == 0) {
    // writeToPin(PDN3, HIGH);
    // printf("Peristaltic pump successfully activated.\n");
  // } else if (strncmp(command, "pump off", 8) == 0) {
    // writeToPin(PDN3, LOW);
    // printf("Peristaltic pump successfully deactivated.\n");
  // } else if (strncmp(command, "uv on", 5) == 0) {
    // writeToPin(PDN1, HIGH);
    // printf("UV LED successfully activated.\n");
  // } else if (strncmp(command, "uv off", 6) == 0) {
    // writeToPin(PDN1, LOW);
    // printf("UV LED successfully deactivated.\n");
  // } else {
    // printf("Error: Invalid command.\n");
  // }

  // return UI;
// }


