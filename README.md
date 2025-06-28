# arduino-led-kontroller

#### Simple LED controller for Arduino Uno R3

- Uses Arduino Uno R3 to drive an RGB led / led strip connected to PWM pins `9, 10, 11`.
- Program only runs the led strip while a remote control signal is sensed HIGH in Digital pin `2`.
- Toggling switch state advances the LED effect program.
- (Remote control signal can be simulated by connecting a positive line through a series resistor to pin 2 via normally-closed switch.)
