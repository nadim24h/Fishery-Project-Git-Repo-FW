//Task1code:
void Task1code( void * pvParameters ) {
  //Serial.print("Task1 running on core ");
  //Serial.println(xPortGetCoreID());
  for (;;) {
    ReadTemperature();
    delay(6000);
  }
}

//Task2code:
void Task2code( void * pvParameters ) {
  //Serial.print("Task2 running on core ");
  //Serial.println(xPortGetCoreID());
  for (;;) {
    IRReceiveFunction();
    delay(1500);
  }
}

//Task3code:
void Task3code( void * pvParameters ) {
  //Serial.print("Task3 running on core ");
  //Serial.println(xPortGetCoreID());
  for (;;) {
    if (ConnectionMode == 1)
    {
      if (WiFi.status() != WL_CONNECTED)
      {
        Portal.handleClient();
      } else
      {
        WifiFunctions();
        delay(10000);
      }
    } else if (ConnectionMode == 2)
    {
      GSMConnect();
      delay(12000);
    } else
    {
      delay(2000);
    }
    delay(1000);
  }
}


void InitTask1()
{
  //Temperature Sensor read
  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
    Task1code,   /* Task function. */
    "Task1",     /* name of task. */
    10000,       /* Stack size of task */
    NULL,        /* parameter of the task */
    1,           /* priority of the task */
    &Task1,      /* Task handle to keep track of created task */
    0);          /* pin task to core 0 */
  delay(500);

}
void InitTask2()
{ //Infrared
  //create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
    Task2code,   /* Task function. */
    "Task2",     /* name of task. */
    10000,       /* Stack size of task */
    NULL,        /* parameter of the task */
    1,           /* priority of the task */
    &Task2,      /* Task handle to keep track of created task */
    1);          /* pin task to core 1 */
  delay(500);

}

void InitTask3()
{ //Connectivity
  xTaskCreatePinnedToCore(
    Task3code,   /* Task function. */
    "Task3",     /* name of task. */
    10000,       /* Stack size of task */
    NULL,        /* parameter of the task */
    1,           /* priority of the task */
    &Task3,      /* Task handle to keep track of created task */
    1);          /* pin task to core 1 */
  delay(500);
}
