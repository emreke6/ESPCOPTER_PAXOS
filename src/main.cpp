#include <Arduino.h>


// Custom 128-bit unsigned integer type
typedef struct {
  uint64_t low;
  uint64_t high;
} uint128_t;

#define RSA_BIT_SIZE 64

void mult64to128(uint64_t op1, uint64_t op2, uint64_t *hi, uint64_t *lo)
{
    uint64_t u1 = (op1 & 0xffffffff);
    uint64_t v1 = (op2 & 0xffffffff);
    uint64_t t = (u1 * v1);
    uint64_t w3 = (t & 0xffffffff);
    uint64_t k = (t >> 32);

    op1 >>= 32;
    t = (op1 * v1) + k;
    k = (t & 0xffffffff);
    uint64_t w1 = (t >> 32);

    op2 >>= 32;
    t = (u1 * op2) + k;
    k = (t >> 32);

    *hi = (op1 * op2) + w1 + k;
    *lo = (t << 32) + w3;
}

// Function to calculate the modular exponentiation (base^exp % mod)
uint128_t mod_exp(uint128_t base, uint128_t exp, uint128_t mod)
{
  uint128_t result;
  result.low = 1;
  result.high = 0;
  base.low = base.low % mod.low;

  while (exp.low > 0 || exp.high > 0)
  {
    if (exp.low % 2 == 1)
    {
      result.low = (result.low * base.low) % mod.low;
      result.high = (result.high * base.low + result.low * base.high + result.high * base.low) % mod.low;
    }

    exp.low = exp.low >> 1;
    exp.high = exp.high >> 1;

    uint64_t temp = (base.low * base.high) % mod.low;
    base.low = (base.low * base.low) % mod.low;
    base.high = ((temp + temp) * base.high) % mod.low;
  }

  return result;
}

// Function to perform the extended Euclidean algorithm
uint64_t extended_gcd(uint64_t a, uint64_t b, uint64_t *x, uint64_t *y)
{
  if (a == 0)
  {
    *x = 0;
    *y = 1;
    return b;
  }

  uint64_t x1, y1;
  uint64_t gcd = extended_gcd(b % a, a, &x1, &y1);

  *x = y1 - (b / a) * x1;
  *y = x1;

  return gcd;
}

// Function to calculate the modular inverse
uint64_t mod_inverse(uint64_t a, uint64_t m)
{
  uint64_t x, y;
  uint64_t gcd = extended_gcd(a, m, &x, &y);

  if (gcd != 1)
    return -1; // modular inverse does not exist

  return (x % m + m) % m;
}

void generate_key_pair(uint64_t *public_key, uint64_t *private_key, uint64_t *modulus_high,  uint64_t *modulus_low)
{
  // Select two large prime numbers
  uint64_t p = 17978448171644331181ULL;
  uint64_t q = 11785307237444331011ULL;

  //modulus->low = p * q;
  //modulus->high = 0;

  mult64to128(p, q, (uint64_t*)modulus_high, (uint64_t*)modulus_low);

  Serial.println("Modulus high");
  Serial.println(*modulus_high);

  Serial.println("Modulus low");
  Serial.println(*modulus_low);


  uint64_t phi = (p - 1) * (q - 1);

  // Select public key (e) such that 1 < e < phi(n) and gcd(e, phi(n)) = 1
  uint64_t public_key_e = 17;

  // Calculate private key (d) using modular inverse of e mod phi(n)
  uint64_t private_key_d = mod_inverse(public_key_e, phi);

  *public_key = public_key_e;
  *private_key = private_key_d;
}

void encrypt(uint64_t plaintext, uint64_t public_key, uint128_t modulus, uint64_t *ciphertext)
{
  uint128_t base, exp, mod;
  base.low = plaintext;
  base.high = 0;
  exp.low = public_key;
  exp.high = 0;
  mod = modulus;

  uint128_t result = mod_exp(base, exp, mod);
  *ciphertext = result.low;
}

void decrypt(uint64_t ciphertext, uint64_t private_key, uint128_t modulus, uint64_t *decrypted_text)
{
  uint128_t base, exp, mod;
  base.low = ciphertext;
  base.high = 0;
  exp.low = private_key;
  exp.high = 0;
  mod = modulus;

  uint128_t result = mod_exp(base, exp, mod);
  *decrypted_text = result.low;
}

void setup()
{
  Serial.begin(115200);

  uint64_t public_key, private_key;
  uint64_t modulus_low, modulus_high;
  generate_key_pair(&public_key, &private_key, &modulus_low, &modulus_high);

  // Serial.print("Public Key: ");
  // Serial.println(public_key);
  // Serial.print("Private Key: ");
  // Serial.println(private_key);

  // uint64_t plaintext = 42;
  // Serial.print("Original Text: ");
  // Serial.println(plaintext);

  // uint64_t ciphertext;
  // encrypt(plaintext, public_key, modulus, &ciphertext);
  // Serial.print("Encrypted Text: ");
  // Serial.println(ciphertext);

  // uint64_t decrypted_text;
  // decrypt(ciphertext, private_key, modulus, &decrypted_text);
  // Serial.print("Decrypted Text: ");
  // Serial.println(decrypted_text);
}

void loop()
{
  // Nothing to do here
}
