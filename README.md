# SALC
An Attiny85 based virtual computer(interpeter) highly inspired from "Duo Tiny".  
I used a 16x2 lcd display, 4 buttons for input, a buzzer and for program storage a **24c32 EEPROM**.  
As I'm writing this you can make this for under **260₺**.  
I uploaded programs onto the EEPROM using an Arduino uno. While I was testing, my arduino was not available so I uploaded the code onto the attiny85 using an esp8266.  
  
The first prototype:  
<img src="https://github.com/QwertyR0/SALC/blob/main/prototype.jpg" width="315" height="420">  

SALC uses the programming language I constructed: "SAL" 
And obviously it is inspired from Duo tiny's programming language.

But unlike DTPL, its commands are seperated with spaces and its arguments are seperated with dots. You can quite literally change any value in the source code to make it act however you want.  
  
I set commands to be 3 bytes long because I was concerened about memory but as I said, just edit the parameters for the command array if you want to have longer command names.  
  
It was very entertaining for me to work on this project, it also made me learn a lot.  
  
More about the SALC is on the way!
