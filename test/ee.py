arr1 = [0x01, 0x23, 0x45, 0x67,
    0x89, 0xAB, 0xCD, 0xEF,
    0xFE, 0xDC, 0xBA, 0x98,
    0x76, 0x54, 0x32, 0x10]


print(int.from_bytes(arr1,'little'))
print(int.from_bytes(arr1,'big'))