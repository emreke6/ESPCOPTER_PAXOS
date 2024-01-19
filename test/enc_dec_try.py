import socket
import random
from Crypto.Cipher import AES
import secrets
from Crypto.Util.Padding import pad, unpad

def encrypt_with_AES(MSG_TO_SEND):
    nonce_num = pow(2,63) + 1
    nonce_bytes = nonce_num.to_bytes((nonce_num.bit_length()+7)//8, byteorder='big')

    enc_key = pow(2,127) + 1  # 16 bytes = 128 bits

    enc_key_bytes = enc_key.to_bytes((enc_key.bit_length()+7)//8, byteorder='big')

    aes_cipher_obj = AES.new(enc_key_bytes, AES.MODE_CTR, nonce=nonce_bytes)
    MSG_STR = MSG_TO_SEND
    msg_utf8 = MSG_STR.encode('utf-8')
    cipher_bytes = aes_cipher_obj.encrypt(msg_utf8)

    all_together_msg = nonce_bytes + cipher_bytes

    return all_together_msg

def decrypt_with_aes(nonce_and_cipher_message):

    print(nonce_and_cipher_message)
    dec_key = pow(2,127) + 1  # 16 bytes = 128 bits
    dec_key_bytes = dec_key.to_bytes((dec_key.bit_length()+7)//8, byteorder='big')

    nonce_bytes = nonce_and_cipher_message[0:8]

    cipher_bytes = nonce_and_cipher_message[8:]

    aes_cipher_obj = AES.new(dec_key_bytes, AES.MODE_CTR, nonce=nonce_bytes)

    pt = aes_cipher_obj.decrypt(cipher_bytes)
    
    message_str = pt.decode("utf-8")

    print("The collected plaintext: ", message_str)


    return message_str



def encrypt_with_AES_cbc(MSG_TO_SEND):
    nonce_num = pow(2,127)
    nonce_bytes = nonce_num.to_bytes((nonce_num.bit_length()+7)//8, byteorder='big')

    print(len(nonce_bytes))

    enc_key = pow(2,127) + 1  # 16 bytes = 128 bits

    enc_key_bytes = enc_key.to_bytes((enc_key.bit_length()+7)//8, byteorder='big')

    print("enc key: ", enc_key_bytes)

    aes_cipher_obj = AES.new(enc_key_bytes, AES.MODE_CBC,iv=nonce_bytes)
    MSG_STR = MSG_TO_SEND
    msg_utf8 = MSG_STR.encode('utf-8')

    msg_utf8_padded = pad(msg_utf8, 128)

    cipher_bytes = aes_cipher_obj.encrypt(msg_utf8_padded)

    all_together_msg = nonce_bytes + cipher_bytes

    return all_together_msg

def decrypt_with_aes_cbc(nonce_and_cipher_message):

    print(nonce_and_cipher_message)
    dec_key = pow(2,127) + 1  # 16 bytes = 128 bits
    dec_key_bytes = dec_key.to_bytes((dec_key.bit_length()+7)//8, byteorder='big')

    print("dec key: ", dec_key_bytes)

    nonce_bytes = (pow(2,127)).to_bytes((pow(2,127).bit_length()+7)//8, byteorder='big')

    cipher_bytes = nonce_and_cipher_message[16:]

    aes_cipher_obj = AES.new(dec_key_bytes, AES.MODE_CBC, iv=nonce_bytes)

    #cipher_bytes_unpadded = unpad(cipher_bytes, 128)

    pt = aes_cipher_obj.decrypt(cipher_bytes)

    cipher_bytes_unpadded = unpad(pt, 128)
    
    message_str = cipher_bytes_unpadded.decode("utf-8")

    print("The collected plaintext: ", message_str)


    return message_str




if __name__ == "__main__":


    message = "EMRE"

    encr_msg = encrypt_with_AES(message)

    dec_msg = decrypt_with_aes(encr_msg)

    encrr_msg_cbc = encrypt_with_AES_cbc(message)

    dec_msg_cbc = decrypt_with_aes_cbc(encrr_msg_cbc)
