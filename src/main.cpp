#include <Arduino.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define RSA_BIT_SIZE 16

// Function to calculate the modular exponentiation (base^exp % mod)
uint16_t mod_exp(uint16_t base, uint16_t exp, uint16_t mod)
{
  uint16_t result = 1;
  base = base % mod;

  while (exp > 0)
  {
    if (exp % 2 == 1)
      result = (result * base) % mod;

    exp = exp >> 1;
    base = (base * base) % mod;
  }
  return result;
}

// Function to perform the extended Euclidean algorithm
int extended_gcd(int a, int b, int *x, int *y)
{
  if (a == 0)
  {
    *x = 0;
    *y = 1;
    return b;
  }

  int x1, y1;
  int gcd = extended_gcd(b % a, a, &x1, &y1);

  *x = y1 - (b / a) * x1;
  *y = x1;

  return gcd;
}

// Function to calculate the modular inverse
int mod_inverse(int a, int m)
{
  int x, y;
  int gcd = extended_gcd(a, m, &x, &y);

  if (gcd != 1)
    return -1; // modular inverse does not exist

  return (x % m + m) % m;
}

void generate_key_pair(uint16_t *public_key, uint16_t *private_key, uint16_t *modulus)
{
  // Select two large prime numbers
  uint16_t p = 61;
  uint16_t q = 53;

  *modulus = p * q;
  uint16_t phi = (p - 1) * (q - 1);

  // Select public key (e) such that 1 < e < phi(n) and gcd(e, phi(n)) = 1
  uint16_t public_key_e = 17;

  // Calculate private key (d) using modular inverse of e mod phi(n)
  uint16_t private_key_d = mod_inverse(public_key_e, phi);

  *public_key = public_key_e;
  *private_key = private_key_d;
}

void encrypt(uint16_t plaintext, uint16_t public_key, uint16_t modulus, uint16_t *ciphertext)
{
  *ciphertext = mod_exp(plaintext, public_key, modulus);
}

void decrypt(uint16_t ciphertext, uint16_t private_key, uint16_t modulus, uint16_t *decrypted_text)
{
  *decrypted_text = mod_exp(ciphertext, private_key, modulus);
}

void setup()
{
  Serial.begin(115200);

  uint16_t public_key, private_key, modulus;
  generate_key_pair(&public_key, &private_key, &modulus);

  Serial.println("Public Key: " + String(public_key));
  Serial.println("Private Key: " + String(private_key));
  Serial.println("Modulus: " + String(modulus));

  uint16_t plaintext = 42;
  Serial.println("Original Text: " + String(plaintext));

  uint16_t ciphertext;
  encrypt(plaintext, public_key, modulus, &ciphertext);
  Serial.println("Encrypted Text: " + String(ciphertext));

  uint16_t decrypted_text;
  decrypt(ciphertext, private_key, modulus, &decrypted_text);
  Serial.println("Decrypted Text: " + String(decrypted_text));
}

void loop()
{
  // Nothing to do here
}
