# nRF24-Esk8-Remote

This is a fork of SolidGeek's nRF24-Esk8-Remote https://github.com/SolidGeek/nRF24-Esk8-Remote with adaptations for my hardware and custom improvements.
More details to come as I test it.

### Hardware
The remote uses an Arduino Pro Mini 3v3, 128x32 OLED display, OH49E hall sensor and nRF24L01+ radio module. It's powered by a swappable 18350 lithium battery that can also be charged using the internal USB charger.

The receiver uses an Arduino Pro Mini 5V and nRF24L01+ radio module. It has a voltage divider to measure voltages from 0 to 51v.

It has 3 switches:

1. Dead man switch. This needs to be held down in order to accelerate.
2. Page switch. Used to go through the settings menu and to change the information displayed on the screen.
3. Cruise control button. Hold this down to use cruise control.

### Settings
Press and hold the Dead man switch while the remote is starting to enter the settings menu. Use the Page switch button to navigate the settings. Some setting use the Cruise control button for special actions. Each setting is saved to EEPROM when you press the Page switch button.

Setting | Default | Range | Description
--- | --- | --- | --- 
Rotate Display | False | True, False | Rotate the display, so you can use the remote in the other hand. Setting takes effect after restart.
Battery type | | | The battery type that is connected to the receiver voltage sensor. Used for calculating the battery level in the "BATTERY" page.
Battery cells | 10s | 0-12S| The cell count of the battery that is connected to the receiver voltage sensor. Used for calculating the battery level in the "BATTERY" page.
Throttle deadzone | 2% | 0-50% | How much of the stck travel is ignored in the center.
Throttle min | 150 | 0-1023 | Hall sensor value for minimum throttle. Move your stick to the maximum brake position and press the Cruise control button to automatically set the value.
Throttle center | 512 | 0-1023 | Hall sensor value for middle throttle. Lave your stick to the idle position and press the Cruise control button to automatically set the value.
Throttle max | 874 | 0-1023 | Hall sensor value for maximum throttle. Lave your stick to the maximum throttle position and press the Cruise control button to automatically set the value.
Brake endpoint | 100% | 0-100%| The end point of the braking value. Set it lower to have softer brakes
Thr. endpoint | 100% | 0-100% | The end point of the throttle value. Set it lower to have smaller maximum throttle.
Brake accel. | 0sec | 0-5sec | Braking acceleration time: The number of seconds it takes to reach 100% brake. Set it to as low as possible so when slamming the brakes you don't land on your face. Set to 0 to disable. USE WITH CAUTION: This makes brakes respond softly, so use small values and make sure you test that you are always able to stop in time.
Throttle accel. | 0sec | 0-5sec | Throttle acceleration time: The number of seconds it takes to reach 100% throttle. Set it so when you slam the throttle you are not thrown off the board. Set to 0 to disable.
Cruise accel. | 1sec | 0-5sec | Cruise control acceleration time: The number of seconds it takes to reach 100% throttle when using cruise control. Set it so you comfortably accelerate towards the desired cruise control value. Set to 0 to disable.
Voltage cal. sen. | 1 | 0-2 | Voltage calibration multiplier for the receiver sensor: Multiplies the voltage of the receiver sensor with this value, to calibrate it.
Voltage cal. rem. | 1 | 0-2 | Voltage calibration multiplier for the remote battery: Multiplies the voltage of the remote battery sensor with this value, to calibrate it.
Batery range | 10KM | 0-99KM | The typical range you get on a battery, used to calculate the remaining range on the "DISTANCE" page.
Radio Channel | 108 | 0-125 | The radio frequency used for communication. Change to avoid interference when using multiple remotes nearby.
RESET ALL | | | Press the Cruise control button to reset all settings to default


### How cruise control works
Cruise control will remember a set throttle value and it will send that throttle value as long as the button is held down. Braking still works even is the cruise control button is held down. The dead man switch needs to be held down even during cruise control.

To set your cruise throttle, move the stick to the desired throttle position and then start holding down the Cruise control button. You can now release the stick and the throttle value will be maintained. If you start pressing the Cruise control button while the stick is in idle position, you will accelerate to the last value you set.

If while holding the Cruise control button, your stick throttle value is larger than the cruise throttle, the stick throttle will be used instead. This way you can temporarily give extra throttle when cruising.

### Pages
Battery: Shows the voltage of the battery connected to the receiver sensor. The progress bar shows the percentage of the battery level. The percentage is calculated based on the typical discharge curve of the battery type selected in the settings. Remember to also set the correct number of battery cells.

Distance: Shows the remaining range of the battery. This is just a convenience calculation, simply calculated by multiplying your battery level with the battery range from the settings.

Output: Shows the signal sent to the receiver. Recommended to be left on when first using the remote to understand how acceleration, endpoints and cruise control work.

### How binding works
To avoid conflicts with other remotes, the transmitter and receiver have to be binded. The remote implements autobinding, meaning the transmitter will automatically connect and bind to the first unconnected receiver it detects. The binding code is generated automatically when the remote is first started or when the radio channel setting is changed. To avoid further interference when using multiple remotes, you can use the radio channel setting to change the radio frequency used.

### Links
More details:
https://www.electric-skateboard.builders/t/twinsens-3d-printed-nrf-remote/50482

Schematics:
https://easyeda.com/Twinsen/NRF_Remote

Parts list:
https://docs.google.com/spreadsheets/d/1Ayte5Xm0up_AQHfdTZghDGnGC5o7WZZo4H4hzZ2LCAw/edit?usp=sharing

3D files:
https://www.thingiverse.com/thing:2843488