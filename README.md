# UV-Light-Sensor

This is an algorithm designed to determine a user's risk of sunburn by using 
a photodiode and a 5V Adafruit Trinket. This was a project created for BME 50
at Tufts University, in the fall semester of 2017. The device was powered with 
a rechargeable lithium polymer battery and is to be worn as a wristband. 

The circuit amplifies the current from photodiode (as it is exposed to sunlight) 
and converts it into a voltage, which is then read by the Trinket. The Trinket then
calcualtes the original current using known equations involving the relation of
gain and voltage from the circuit. The current is then compared to a data sheet found
at https://media.digikey.com/pdf/Data%20Sheets/Photonic%20Detetectors%20Inc%20PDFs/SD012-UVA-011.pdf
which contains a linear graph relatating UV Index to current. This UV index is then weighted
against a "time until sunburn occurs", a chart which can be found here:
https://www.almanac.com/content/uv-index-chart-time-burn

The algorithm implements a linked list to keep a running list of the most 10 recent
"times until sunburn occurs", taking the average time to burn from this running
list to inform the user of risk. Risk is informed  by blinking a neopixel.

