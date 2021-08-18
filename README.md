# PowerLED-timer
It's a simple Power LED lamp controller with timer and two steps brightness 50% and 100%.
Made on request of my wife, I decided to build instead of buying quite pricey Red Light Therapy (RLT) lamp.
<p><a href="https://www.medicalnewstoday.com/articles/325884">
More info about red light therapy</a></p>
  
There are two options in code LowLED and HighLED, for 12x3W and 18x3W PowerLEDs.

3D print files x12:
https://www.thingiverse.com/thing:4932931

3D print files x18:
https://www.thingiverse.com/thing:4932926

<b>Warning
  
To solder LEDs on PCB you will need reflow oven or hot air instead of soldering iron, due to LED low soldering temperature (max 260C), higher temps like 300C can damage LED structure. It's quite dificult to solder on aluminum PCB, so effects are much better using reflow oven.</b>


BOM:
<ul>
<li><a href="https://en.maritex.com.pl/product/show/131448.html">
12x 3W Crimson Power LED PK2N-3LME-HSD</a></li>

<li><a href="https://en.maritex.com.pl/product/show/44614.html">
90mm diameter MCPCB aluminium, 12 x CREE XPC, XPE, XPG </a></li>

<li><a href="https://en.maritex.com.pl/product/show/53441.html">
LENS for 12 x LED High Power CREE</a>, need to cut in pieces</li>

<li><a href="https://www.aliexpress.com/item/32810995244.html">
130cm Universal Arm Tablet Stand Holder</a></li>

<li>Aluminium radiator 78x80x35mm, need to cut it in to a bit smaller than LED pcb</li>

<li>60x60x10mm fan 24V 1.68W</li>


<li>4-Digit LED 0.56" TM1637 display</li>
<li>KY-040 rotary encoder</li>
<li>Arduino board: uno, nano, pro micro, and probably others with enough amount of digital ports</li>
<li>Step-down module LM2596 3,2V-35V 3A</li>
<li>LM1117 module</li>
<li>5.5mm x 2.1mm DC Female Chassis Power Socket</li>
<li>KCD1-104 4-PIN rocker switch 21x15mm</li>
<li>29V 1000mA power supply</li>
<li>Some 3W resistors</li>
<li>2x 12V mini relay</li>
<li>BC547 NPN transistor, or any replacement</li>
<li>2x IRLR024 N mosfet transistor, or any replacement</li>
<li>5.1V Zener diode</li>
<li>1N4148 diode</li>
</ul>
  
![Image1](https://github.com/polihedron/PowerLED-timer/blob/main/img/1.jpg)

![Image2](https://github.com/polihedron/PowerLED-timer/blob/main/img/2.jpg)





