// BigNumber test: powers
#include "BigNumber.h"
#include "Arduino.h"

void aaaa ()
{
  Serial.begin (115200);
  Serial.println ();
  BigNumber::begin ();  // initialize library

  Serial.println ("--- powers of 2 ---");
  
  BigNumber a = BigNumber("166615559182980197783476703033582575179");
  BigNumber pow2 = BigNumber("172126872553704262343918980322200971030");
  BigNumber modulus = BigNumber("211881535355297736509781884103072553991");

  for (int i = 1; i <= 300; i++)
  {
    Serial.print ("2^");
    Serial.print (i);
    Serial.print (" = ");
    BigNumber p = a.powMod(pow2, modulus);
    Serial.println (p);
  }  // end of for loop

  Serial.println ("--- powers of 3 ---");
  
  a = 3;

  for (int i = 1; i <= 300; i++)
  {
    Serial.print ("3^");
    Serial.print (i);
    Serial.print (" = ");
    BigNumber p = a.pow (i);
    Serial.println (p);
  }  // end of for loop

}  // end of setup