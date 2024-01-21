import sympy

def generate_64bit_primes():
    while True:
        prime1 = sympy.randprime(2**(64-1), 2**64)
        prime2 = sympy.randprime(2**(64-1), 2**64)
        
        # Ensure the generated primes are different
        if prime1 != prime2:
            return prime1, prime2

if __name__ == "__main__":
    prime1, prime2 = generate_64bit_primes()
    print("64-bit Prime 1:", prime1)
    print("64-bit Prime 2:", prime2)
