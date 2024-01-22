#include <Arduino.h>
#include "bignumber.h"
#include <iostream>
#include "string.h"

using namespace std;

BigNumber public_modulus = BigNumber("211881535355297736509781884103072553991");
BigNumber drone_1_pub = BigNumber("124636197267822197929422428643519936353");
BigNumber drone_2_pub = 23;
BigNumber drone_3_pub = 29;

BigNumber drone_1_pri = 17;

BigNumber encrypt(BigNumber plaintext, BigNumber public_key, BigNumber modulus)
{
  return plaintext.powMod(public_key, modulus);
}

BigNumber decrypt(BigNumber ciphertext, BigNumber private_key, BigNumber modulus)
{
  return ciphertext.powMod(private_key, modulus);
}

void setup()
{
  Serial.begin(115200);

  uint64_t plaintext = 42;
  Serial.print("Original Text: ");
  Serial.println(plaintext);

  BigNumber ctext = encrypt(plaintext, drone_1_pub, public_modulus);
  Serial.print("Encrypted Text: ");
  Serial.println(ctext);

  BigNumber ptext = decrypt(ctext, drone_1_pri, public_modulus);
  Serial.print("Decrypted Text: ");
  Serial.println(ptext);
}

void loop()
{
  // Nothing to do here
}
