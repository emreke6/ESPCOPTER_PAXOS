from cryptography.hazmat.primitives import serialization, hashes
from cryptography.hazmat.primitives.asymmetric import rsa, padding
from cryptography.hazmat.backends import default_backend

def generate_key_pair(bits=3072):
    private_key = rsa.generate_private_key(
        public_exponent=65537,
        key_size=bits,
        backend=default_backend()
    )
    public_key = private_key.public_key()
    return private_key, public_key

def encrypt(message, public_key):
    ciphertext = public_key.encrypt(
        message.encode('utf-8'),
        padding.PKCS1v15()
    )
    return ciphertext

def decrypt(ciphertext, private_key):
    plaintext = private_key.decrypt(
        ciphertext,
        padding.PKCS1v15()
    )
    return plaintext.decode('utf-8')


rsa_key_file = open("RSA_keys", 'wb')

def save_key_to_file(key, filename):
    rsa_key_file.write(key)

# Example usage
private_key, public_key = generate_key_pair()

message = "Hello, RSA Encryption and Decryption!"

# Encryption
ciphertext = encrypt(message, public_key)
print("Encrypted Message:", ciphertext.hex())

# Decryption
decrypted_message = decrypt(ciphertext, private_key)
print("Decrypted Message:", decrypted_message)

# Save private key to PEM file
private_pem = private_key.private_bytes(
    encoding=serialization.Encoding.PEM,
    format=serialization.PrivateFormat.TraditionalOpenSSL,
    encryption_algorithm=serialization.NoEncryption()
)
save_key_to_file(private_pem, "RSA_keys.txt")

# Save public key to PEM file
public_pem = public_key.public_bytes(
    encoding=serialization.Encoding.PEM,
    format=serialization.PublicFormat.SubjectPublicKeyInfo
)
save_key_to_file(public_pem, "RSA_keys.txt")

print("Private and public keys saved to 'RSA_keys.txt'")
