#include "esphome.h"
class SolarImportExport : public PollingComponent {
 public:
 //User Variable
  // Input Pin of the Pulse Sensor for total power
  const int totalUsagePin = 5; 
  // Input Pin of the Pulse Sensor for import power
  const int importPin = 6; 
  // Update/Publish time in Seconds
  const int publishInterval = 60;
  // debounce in ms
  const int debounceInterval = 150;  
  // Pulses per KW
  const unsigned int pulsesPerKW = 1000;
  // Prefer input data over total data
  const bool preferInputData = true; 
  // Rising or Falling Edge
  const bool useFallingEdge = true;

  // constructors
  //Runs at the debounce freq
  SolarImportExport() : PollingComponent(debounceInterval) {}
  Sensor *solar_export_sensor = new Sensor();
  Sensor *grid_import_sensor = new Sensor();
  Sensor *total_power_consumption_sensor = new Sensor();

  //Global Variables
  bool totalUsagePinLastState = false;
  bool importPinLastState = false;
  int solarExportCounter = 0;
  int gridImportCounter = 0;
  int totalPowerConsumptionCounter = 0;
  int timeCounter = 0;

  void setup() override {
    pinMode(totalUsagePin, INPUT);
    pinMode(importPin, INPUT);
  }

  void update() override {
//*******Processing Logic*******
    //create new vars for each loop
    //these are updated on a state change either on the rising edge or falling edge of a pulse
    bool totalUsagePinFallingEdge = false;
    bool totalUsagePinRisingEdge = false;
    bool importPinFallingEdge = false;
    bool importPinRisingEdge = false;
    //Defaults pointers to Rising Edge
    bool *totalUsagePinEvent = &totalUsagePinRisingEdge;
    bool *importPinEvent = &importPinRisingEdge;

    //logic for reading each pin change event
    if (digitalRead(totalUsagePin) == LOW && totalUsagePinLastState) {
      totalUsagePinLastState = false;
      //falling edge event
      totalUsagePinFallingEdge = true;
    }
    if (digitalRead(totalUsagePin) == HIGH && !totalUsagePinLastState) {
      totalUsagePinLastState = true;
      //rising edge event
      totalUsagePinRisingEdge = true;
    }
    //power import section
    if (digitalRead(importPin) == LOW && importPinLastState) {
      importPinLastState = false;
      //falling edge event
      importPinFallingEdge = true;
    }
    if (digitalRead(importPin) == HIGH && !importPinLastState) {
      importPinLastState = true;
      //rising edge event
      importPinRisingEdge = true;
    }
// *******Mapping********
    //Changes Pointers to Falling Edge if useFallingEdge is true
    if (useFallingEdge) {
      totalUsagePinEvent = &totalUsagePinFallingEdge;
      importPinEvent = &importPinFallingEdge;
    }

// *******Output Logic********
    if (totalUsagePinEvent && importPinEvent) {
      //this is an grid import event
      gridImportCounter++;
    }
    else if (totalUsagePinEvent && !importPinEvent) {
      //this is an grid export event
      solarExportCounter++;
    }
    else if (!totalUsagePinEvent && importPinEvent) {
      //this should never occur - catch error and log output
      if (preferInputData){
        //accept as an import event
        gridImportCounter++;
      }
    }
    //counter for time & pulse count for publish logic

    //publish data and reset counters
    //converts publishInterval to ms
    if (timeCounter * debounceInterval >= publishInterval * 1000) {
      //publish data in KWh units
      solar_export_sensor -> publish_state((solarExportCounter * publishInterval) / pulsesPerKW);
      grid_import_sensor -> publish_state((gridImportCounter * publishInterval) / pulsesPerKW);
      // ######### total_power_consumption_sensor -> publish_state((totalPowerConsumptionCounter * publishInterval) / pulsesPerKW);
      //reset counters after publish
      solarExportCounter = 0;
      gridImportCounter = 0;
      totalPowerConsumptionCounter = 0;
      timeCounter = 0;
    }
    timeCounter++
  }
};