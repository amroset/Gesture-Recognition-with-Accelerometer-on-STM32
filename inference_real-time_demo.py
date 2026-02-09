# Copyright 2021 ETH Zurich and University of Bologna.
# Licensed under the Apache License, Version 2.0, see https://www.apache.org/licenses/LICENSE-2.0 for details.
# SPDX-License-Identifier: Apache-2.0
#
# Author: Viviane Potocnik <vivianep@iis.ee.ethz.ch> (ETH Zurich) 

import matplotlib.pyplot as plt
import numpy as np
import serial
import sys
import time
import tkinter as tk
from PIL import Image, ImageTk

if __name__ == '__main__':
    #x_test = np.load('x_test_gestures.npy')
    #y_test = np.load('y_test_gestures.npy').squeeze()
   
    #classes = ['0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '10',
              #'11', '12', '13', '14', '15', '16', '17', '18', '19']
        
    #classes = ['right', 'left', 'front', 'back', 'counterclock', 'clockwise', 'A-shape', 'V-shape']
    classes = ['right', 'left', 'front', 'back', 'counterclock', 'clockwise', 'A-shape']

    #print(f'Loaded x with shape: {x_test.shape}')
    #print(f'Loaded y with shape: {y_test.shape}')

    ser = serial.Serial(port='/dev/ttyACM0', baudrate=115200, timeout=3)
    # flush the serial port
    ser.flush()
    ser.flushInput()
    ser.flushOutput()

    correct_count = 0
    # get how many predictions we iterated over
    num_pred = 0
    flag = 0
    flag2 = 0
    
    #for x, y in zip(x_test[:test_len], y_test[:test_len]):
    while(1):
        num_pred += 1
        #class_idx = np.argmax(y)
        #ser.write(x.tobytes())
        time.sleep(1)
        seq = ser.read(32*3)
        seq = np.frombuffer(seq, dtype=np.uint8)
        time.sleep(1)
        if(seq.any()):
            print("Sequence recorded by the accelerometer on the MCUs: \n {}".format(seq))
            flag = 1
        if(flag):            
            pred = ser.read(7)
            pred = np.frombuffer(pred, dtype=np.uint8)
            if(pred.any()):
                if(max(pred) >= 150):
                    print(f'Prediction (from MCU):{classes[np.argmax(pred)]}')
                else:
                   print(f'Unsure: {classes[np.argmax(pred)]}')
            flag = 0
        #if np.argmax(pred) == class_idx:
            #print("correct prediction!")
            #correct_count += 1

        """root = tk.Tk()
        root.title('Real-time Inference')

        # Prediction label
        pred_text = f'Prediction: {classes[np.argmax(pred)]}'
        pred_label = tk.Label(root, text=pred_text, font=('Helvetica', 18, 'bold'),
                              bg=color, fg='white')
        pred_label.grid(row=1, column=0, padx=(10, 5), pady=10, sticky='nsew')

        # Target label
        target_text = f'Target: {classes[class_idx]}'
        target_label = tk.Label(root, text=target_text, font=('Helvetica', 18, 'bold'),
                                bg=color, fg='white')
        target_label.grid(row=1, column=1, padx=(5, 10), pady=10, sticky='nsew')

        # Accuracy label
        accuracy = (correct_count / num_pred) * 100
        accuracy_text = f'Accuracy: {accuracy:.2f}%'
        accuracy_label = tk.Label(root, text=accuracy_text, font=('Helvetica', 18, 'bold'),
                                  bg=yellow, fg=darkgrey)
        accuracy_label.grid(row=2, column=0, padx=(10, 10), pady=10, columnspan=2, sticky='nsew')

        # Functions to handle button clicks
        def next_image():
            root.quit()
            root.destroy()

        def quit_program():
            root.quit()
            root.destroy()
            sys.exit()

        # Next button
        next_button = tk.Button(root, text='Next', command=next_image,
                                font=('Helvetica', 18, 'bold'), bg=next_color, fg='white',
                                activebackground=next_hover_color, activeforeground='white')
        next_button.grid(row=3, column=0, padx=(10, 5), pady=10, columnspan=2, sticky='nsew')

        # Quit button
        quit_button = tk.Button(root, text='Quit', command=quit_program,
                                font=('Helvetica', 18, 'bold'), bg=next_color, fg='white',
                                activebackground=quit_color, activeforeground='white')
        quit_button.grid(row=4, column=0, padx=(10, 5), pady=10, columnspan=2, sticky='nsew')

        root.mainloop()"""

