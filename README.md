# nRF24-Esk8-Remote

This is a fork of SolidGeek's nRF24-Esk8-Remote https://github.com/SolidGeek/nRF24-Esk8-Remote with adaptations for my hardware and custom improvements.
More details to come as I test it.

### Hardware
The remote uses an Arduino Pro Mini 3v3, 128x32 OLED display, OH49E hall sensor and nRF24L01+ radio module. It's powered by a swappable 18350 lithium battery that can also be charged using the internal USB charger.

It has 3 switches:

1. Dead man switch. This needs to be held down in order to accelerate.
2. Page switch. Used to go through the settings menu and to change the information displayed on the screen.
3. Cruise control button. Hold this down to use cruise control.

### Settings
Press and hold the Dead man switch while the remote is starting to enter the settings menu. Use the Page switch button to navigate the settings. Some setting use the Cruise control button for special actions. Each setting is saved to EEPROM when you press the Page switch button.

Setting | Default | Range | Description
--- | --- | --- | --- 
Rotate Display | False | True, False | Rotate the display, so you can use the remote in the other hand. Setting takes effect after restart.
Bar shows input | True | True, False | (Not implemeted yet) If true the bar on the main display shows a graphical representation of the signal sent to the receiver. Otherwise it shows a progress depending on the current setting(e.g. main battery %, speed % of max speed). Recommended to be left on when first using the remote to understand how acceleration, endpoints and cruise control work.
Battery type | | | Not implemented yet
Battery cells | 10s | 0-12S| Not implemented yet
Throttle deadzone | 2% | 0-50% | How much of the stck travel is ignored in the center.
Throttle min | 150 | 0-1023 | Hall sensor value for minimum throttle. Move your stick to the maximum brake position and press the Cruise control button to automatically set the value.
Throttle center | 512 | 0-1023 | Hall sensor value for middle throttle. Lave your stick to the idle position and press the Cruise control button to automatically set the value.
Throttle max | 874 | 0-1023 | Hall sensor value for maximum throttle. Lave your stick to the maximum throttle position and press the Cruise control button to automatically set the value.
Brake endpoint | 100% | 0-100%| The end point of the braking value. Set it lower to have softer brakes
Thr. endpoint | 100% | 0-100% | The end point of the throttle value. Set it lower to have smaller maximum throttle.
Brake accel. | 0sec | 0-5sec | Braking acceleration time: The number of seconds it takes to reach 100% brake. Set it to as low as possible so when slamming the brakes you don't land on your face. Set to 0 to disable. USE WITH CAUTION: This makes brakes respond softly, so use small values and make sure you test that you are always able to stop in time.
Throttle accel. | 0sec | 0-5sec | Throttle acceleration time: The number of seconds it takes to reach 100% throttle. Set it so when you slam the throttle you are not thrown off the board. Set to 0 to disable.
Cruise accel. | 1sec | 0-5sec | Cruise control acceleration time: The number of seconds it takes to reach 100% throttle when using cruise control. Set it so you comfortably accelerate towards the desired cruise control value. Set to 0 to disable.
RESET ALL | | | Press the Cruise control button to reset all settings to default


### How cruise control works
Cruise control will remember a set throttle value and it will send that throttle value as long as the button is held down. Braking still works even is the cruise control button is held down. The dead man switch needs to be held down even during cruise control.

To set your cruise throttle, move the stick to the desired throttle position and then start holding down the Cruise control button. You can now release the stick and the throttle value will be maintained. If you start pressing the Cruise control button while the stick is in idle position, you will accelerate to the last value you set.

If while holding the Cruise control button, your stick throttle value is larger than the cruise throttle, the stick throttle will be used instead. This way you can temporarily give extra throttle when cruising.