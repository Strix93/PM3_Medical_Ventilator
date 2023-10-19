# PM3_Medical_Ventilator

The code is for a NUCLEO-F446RE board.

## Design Brief and System Requirements

As a starting point a ventilator bag will be provided (especially sized for use on young children) as well as the Nucleo Microcontroller (used in PM2) which will be used to automate the mechanical ventilator. Together with the Prusa 3D printers which are available and a small budget (100 Chf) to purchase a motor and the necessary electronics to be able to control the motor with the Nucleo-board, the challenge is to design a mechanical ventilator which can:

-	1) Enables 15 bpm (breaths per minute).
-	2) Delivers a Volume output of Vmax = 8 ml per kg weight of child (assume weight = 40 kg) =320ml
-	3) A Minimum Inhale Exhale time-ratio of 1:3 (for every second to inhale, 3 seconds are need for exhale).
-	4) Capable of delivering a maximum pressure of 400 Pa.
-	5) Makes use of a single motors or stepper motor.
-	6) Mechanically Compresses the bag according to points 1-4 using as little energy/power as possible.
-	7) Uses as little material as possible, whilst still being strong enough to operate according to points 1 – 5. 
-	8) PRUSA prints must use 100% infill (to enable accurate modelling)
-	8) To test the final design the output tube can connect to the flow measurement tool.

To simplify system design such that it is achievable in 14 weeks the system should be open loop, i.e., no sensors are required to control pressure or flow rate. To ensure that the compression of the bag provides a uniform flowrate the area of contact between the bag and the mechanical actuation system must be constant (I.e., the torque provided by the motor translates to a constant force which is applied over a constant area (or as close as possible to this, allowing for the deformation of the ventilation bag)
