# sudo apt-get install python3-pip
# python3 -m pip install pyserial
# sudo apt-get install python3-matplotlib

import serial
import numpy as np
from PIL import Image
import matplotlib.pyplot as plt

# === Initialize Serial Port ===
ser = serial.Serial('/dev/tty.usbmodem2101', timeout=1)  # Adjust port as needed
print('Opening port:', ser.name)

# === Image Buffers ===
reds = np.zeros((60, 80), dtype=np.uint8)
greens = np.zeros((60, 80), dtype=np.uint8)
blues = np.zeros((60, 80), dtype=np.uint8)

# === Command Loop ===
while True:
    selection = input('\nENTER COMMAND ("c" to capture, "q" to quit): ')
    if selection == 'q':
        print('Exiting client.')
        ser.close()
        break

    if selection == 'c':
        ser.reset_input_buffer()  # Clear junk from previous data

        # Send 'c' to microcontroller
        ser.write(b'c\n')

        # Collect 4800 lines of image data
        count = 0
        while count < 4800:
            dat_str = ser.read_until(b'\n')
            try:
                parts = dat_str.decode().strip().split()
                if len(parts) != 4:
                    print(f"Skipping invalid line {count}: {dat_str}")
                    continue
                i, r, g, b = map(int, parts)
                row = i // 80
                col = i % 80
                reds[row][col] = r
                greens[row][col] = g
                blues[row][col] = b
                count += 1
            except Exception as e:
                print(f"Error at line {count}: {e}, raw: {dat_str}")
                continue

        # Convert and show image
        rgb_array = np.stack((reds, greens, blues), axis=-1)
        image = Image.fromarray(rgb_array)
        plt.imshow(image)
        plt.axis("off")
        plt.title("Captured Image")
        plt.show()
    else:
        print("Invalid command. Use 'c' to capture or 'q' to quit.")