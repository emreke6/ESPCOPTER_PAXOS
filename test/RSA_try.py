import random

def egcd(a, b):
    x,y, u,v = 0,1, 1,0
    while a != 0:
        q, r = b//a, b%a
        m, n = x-u*q, y-v*q
        b,a, x,y, u,v = a,r, u,v, m,n
    gcd = b
    return gcd, x, y

def modinv(a, m):
    gcd, x, y = egcd(a, m)
    if gcd != 1:
        return None  # modular inverse does not exist
    else:
        return x % m


p = 17978448171644331181
q = 11785307237444331011

n = p * q

print("n: ", n)

phi_n = (p-1) * (q-1)

priv_key = 17
pub_key = modinv(priv_key, phi_n)

try_num = 41

enc_num = pow(try_num, pub_key, n)

print(enc_num)

dec_num = pow(enc_num, priv_key, n)

print(dec_num)

print("phi n: ", phi_n)


random_int = random.randint(5, n)
random_pow = random.randint(5, n)

print("random int: ", random_int)
print("random pow: ", random_pow)

print("res: ", pow(random_int, random_pow, n))




