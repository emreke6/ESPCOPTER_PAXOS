#include <Arduino.h>
#include "bignumber.h"
#include <iostream>
#include "string.h"

using namespace std;

// BigNumber p = 17978448171644331181ULL;
// BigNumber q = 11785307237444331011ULL;
BigNumber public_modulus = BigNumber("211881535355297736509781884103072553991");
BigNumber drone_1_pub = BigNumber("124636197267822197929422428643519936353");
BigNumber drone_2_pub = 23;
BigNumber drone_3_pub = 29;

BigNumber drone_1_pri = 17;

// // Custom 128-bit unsigned integer type
// typedef struct {
//   uint64_t low;
//   uint64_t high;
// } uint128_t;

// #define RSA_BIT_SIZE 64

// uint128_t* RSA_N;
// uint128_t* RSA_PHI;

// BigNumber mod(const BigNumber& b, const BigNumber& a) {
//     return b - (b / a) * a;
// }

// BigNumber egcd(BigNumber a, BigNumber b, BigNumber& x, BigNumber& y) {
//     x = 0; 
//     y = 1;
//     BigNumber u = 1, v = 0;

//     while (!(a == 0)) {
//         BigNumber q = b / a;
//         BigNumber r = mod(b, a);
//         BigNumber m = x - u * q;
//         BigNumber n = y - v * q;
//         b = a;
//         a = r;
//         x = u;
//         y = v;
//         u = m;
//         v = n;
//     }

//     return b;
// }

// BigNumber modinv(BigNumber a, BigNumber b) {
//     BigNumber x = 0, y = 0;
//     BigNumber gcd = egcd(a, b, x, y);
//     if (!(gcd == 1)) {
//         return 0;
//     }
//     return mod(x, b) < 0 ? mod(x, b) + b : mod(x, b);
// }

// void mult64to128(uint64_t op1, uint64_t op2, uint64_t *hi, uint64_t *lo)
// {
//     uint64_t u1 = (op1 & 0xffffffff);
//     uint64_t v1 = (op2 & 0xffffffff);
//     uint64_t t = (u1 * v1);
//     uint64_t w3 = (t & 0xffffffff);
//     uint64_t k = (t >> 32);

//     op1 >>= 32;
//     t = (op1 * v1) + k;
//     k = (t & 0xffffffff);
//     uint64_t w1 = (t >> 32);

//     op2 >>= 32;
//     t = (u1 * op2) + k;
//     k = (t >> 32);

//     *hi = (op1 * op2) + w1 + k;
//     *lo = (t << 32) + w3;
// }

// // Function to calculate the modular exponentiation (base^exp % mod)
// uint128_t mod_exp(uint128_t base, uint128_t exp, uint128_t mod)
// {
//   uint128_t result;
//   result.low = 1;
//   result.high = 0;
//   base.low = base.low % mod.low;

//   while (exp.low > 0 || exp.high > 0)
//   {
//     if (exp.low % 2 == 1)
//     {
//       result.low = (result.low * base.low) % mod.low;
//       result.high = (result.high * base.low + result.low * base.high + result.high * base.low) % mod.low;
//     }

//     exp.low = exp.low >> 1;
//     exp.high = exp.high >> 1;

//     uint64_t temp = (base.low * base.high) % mod.low;
//     base.low = (base.low * base.low) % mod.low;
//     base.high = ((temp + temp) * base.high) % mod.low;
//   }

//   return result;
// }

// // Function to perform the extended Euclidean algorithm
// uint64_t extended_gcd(uint64_t a, uint64_t b, uint64_t *x, uint64_t *y)
// {
//   if (a == 0)
//   {
//     *x = 0;
//     *y = 1;
//     return b;
//   }

//   uint64_t x1, y1;
//   uint64_t gcd = extended_gcd(b % a, a, &x1, &y1);

//   *x = y1 - (b / a) * x1;
//   *y = x1;

//   return gcd;
// }

// // Function to calculate the modular inverse
// uint64_t mod_inverse(uint64_t a, uint64_t m)
// {
//   uint64_t x, y;
//   uint64_t gcd = extended_gcd(a, m, &x, &y);

//   if (gcd != 1)
//     return -1; // modular inverse does not exist

//   return (x % m + m) % m;
// }


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

  BigNumber::begin ();  // initialize library

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
