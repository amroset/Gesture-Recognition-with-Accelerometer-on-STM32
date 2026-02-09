This project is part of the Machine Learning on Microcontrollers course I attended at ETH. The purpose was to show how
gesture recognition techniques based on neural networks can be implemented on a STM32U5 Microcontroller, which uses an ARM Cortex-M edge AI platform.

Some important information on the files provided:

-there is one training script for each of the trained models: Kaggle, Hybrid, CNN and TCN.
-the quantization_aware_prune script is used to apply quantization aware training and unstructured pruning. I had some version compatibilities issues with other libraries so I found it easier to have a specific script for that.
-the read_data script is the one I used to record sequences from the accelerometer to create my own datasets.
-the demo script is used to train the demo model.
-the inference_real_time script is used to test the inference capabilities of the microcontroller in real time.
-the 'Gesture' folder is the STMCube project folder. It contains all the files needed to run the code on the board.
-main.c contains all the initializations and preprocessing needed to record data from the accelerometer.

Please refer to "requirements.txt" to have a list of the installed packages.
