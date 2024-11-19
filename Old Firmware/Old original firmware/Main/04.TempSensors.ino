void ReadTemperature()
{
  /*Temperature Sensor*/
  Tsensor1.requestTemperatures(); 
  Celcius = Tsensor1.getTempCByIndex(0);
   
  //Sensor2 
  Tsensor2.requestTemperatures(); 
  Celcius2 = Tsensor2.getTempCByIndex(0);
   
}  

//Fahrenheit=Tsensor2.toFahrenheit(Celcius2); 
//Serial.print(" C2:"); Serial.print(Celcius2); Serial.print(" F2:"); Serial.println(Fahrenheit);
//ShowCoreNumber("ReadTemperature"); 
  
