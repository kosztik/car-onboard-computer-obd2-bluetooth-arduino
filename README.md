# Car onboard computer with arduino



<p>
  <ul>
    <li>
  <img src="https://i.ibb.co/f1jsPtN/s-l500.jpg" width="150px"><a href="https://www.ebay.com/itm/AT24C02-Data-Storage-Module-EEPROM-Memory-Module-2-7-5-5V-I2C-IIC-Interface/232487881849?ssPageName=STRK%3AMEBIDX%3AIT&_trksid=p2057872.m2749.l2649">AT24C02 Data Storage Module EEPROM Memory Module 2.7-5.5V I2C IIC Interface</a><br/>
      <p>https://stackoverflow.com/questions/47823243/can-read-write-eeprom-address-however-can-only-read-write-first-memory-page/47829869#47829869</p>
      <p>
      https://arduino.stackexchange.com/questions/34244/storing-data-on-at24c02-eeprom-chips-with-arduino
      </p>
    </li>
<li><img src="https://i.ibb.co/9Ts5Mb0/p.jpg" ><a href="https://www.ebay.com/itm/Keyes-Electronic-Block-AD-Analog-Keyboard-Keypad-Module-for-Arduino/123719143889?ssPageName=STRK%3AMEBIDX%3AIT&_trksid=p2057872.m2749.l2649">Arduino AD keyboard</a><br />
  
  
  https://forum.arduino.cc/index.php?topic=436540.0</li>
  <li>
    <p><a href="https://i.ibb.co/9yT680Q/s-l1600.jpg" target="_blank"><img src="https://i.ibb.co/9yT680Q/s-l1600.jpg" width="150"></a><a href="https://www.ebay.com/itm/30ft-Wireless-Bluetooth-RF-Transceiver-Module-serial-RS232-TTL-HC-05-for-arduino/310540196588?ssPageName=STRK%3AMEBIDX%3AIT&_trksid=p2057872.m2749.l2649">HC-05 Bluetooth module</a> </p>
    <p>Enter command mode: high on KEY pinout. Very important to set communication speed to 38400bps because command mode talk in this speed. Arduino and serial monitor still on 9600bps</p>
<p><img src="https://i.ibb.co/74qpCJR/Bluetooth-HC-05-with-Arduino-bb.png"><br /><a href="https://www.instructables.com/id/Setting-Up-Bluetooth-HC-05-With-Arduino/">https://www.instructables.com/id/Setting-Up-Bluetooth-HC-05-With-Arduino/</a></p>
  <p>
  For some reason this bluetooth module can't worked well for me. The pair was success, but the link always failed. So I switched this version of bluetooth HC-05 module:<br />
    <img src="https://i.ibb.co/V3w7N9P/1258-20-1-300x300.jpg"><br />
    It has a small button to enter AT mode. In AT mode it needs just a next few command:
AT+INIT<br />
AT+PAIR=12,34,56789c72,20 (,20 means 20 second timeout)<br />
AT+BIND=12,34,56789c72<br />
AT+LINK=12,34,56789c72<br />
    note: capital letters must be use!!!<br />
    After switched back from AT mode, and typed ATI command, the ELM was answer me, so it succeeded first!
    
    
  </p>
  </li>
  <li>
    <p>I2C serial interface 1602 16x2 Character LCD</p>
    <p>Address: 0x27</p>
    <p>used library: LiquidCrystal I2C by Frank de Brabander version 1.1.2</p>
  </li>
</ul>
    </p>
