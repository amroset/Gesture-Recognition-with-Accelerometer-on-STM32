# Gesture Recognition with Accelerometer on STM32

This project explores **hand gesture recognition on microcontrollers** using **deep learning models optimized for real-time inference**.  
The main goal is to identify architectures that offer the **best trade-off between accuracy, model size, and latency**, making them suitable for deployment on **resource-constrained STM32 microcontrollers**.

The work combines **model comparison**, **model compression**, and the implementation of a **real-time embedded demo** based on accelerometer data.

---

## Project Overview

The project is structured around four main phases:

1. **Dataset analysis**  
   Understanding the properties and limitations of the available gesture dataset.

2. **Model selection and comparison**  
   Implementing and evaluating architectures proposed in the literature.

3. **Model optimization**  
   Reducing model size and inference time through architectural design and quantization.

4. **Real-time embedded demo**  
   Deploying the final solution on an STM32 board with live sensor input.

---

## Dataset

- **20 gestures**
- **8 users**
- **3,251 recorded sequences**
- Sequence length between **10 and 51 time steps**
- 3-axis accelerometer data `(x, y, z)`

The dataset originates from research by the **Technology of Vision (TeV)** unit at **Fondazione Bruno Kessler**, collected using a Sony smartwatch.

---

## Baseline Models

### Baseline 1 – Bidirectional LSTM (Kaggle solution)

- Architecture: Bidirectional LSTM + Dense layers
- Accuracy: **99.5%**
- Flash size: **~41 KB**
- Latency: **~81 ms** @ 80 MHz

Despite its high accuracy, the recurrent nature of the model results in relatively high inference latency, making it less suitable for real-time microcontroller deployment.

---

### Baseline 2 – Hybrid LSTM + CNN

Inspired by more recent literature, a **hybrid architecture** was implemented:

- LSTM layers extract temporal features
- CNN layers extract spatial features
- Hyperparameters optimized using **Bayesian optimization (Optuna)**

Results:
- Accuracy: **99.7%**
- Flash size: **~35 KB**
- Latency: **~32 ms**

Although faster than the pure LSTM approach, the LSTM component still dominates inference time.

---

## Moving Beyond LSTMs

Recent research suggests that **convolutional networks can outperform recurrent networks** for sequence modeling tasks, while being faster and easier to optimize for embedded systems.

### Temporal Convolutional Networks (TCN)

- Single-layer TCN with dilations `[1, 2, 4, 8]`
- Accuracy: **99.5%**
- Model size: **~110 KB**

While effective, the model size is too large for the target microcontroller.

---

## Final Model Architectures

### Simple CNN

- Two `Conv2D` layers with kernel size `(3 × 3)`
- Extracts:
  - inter-axis dependencies
  - short-term temporal features
- Padding strategy inspired by TCNs

This model achieves high accuracy but introduces higher latency.

---

### Semi-TCN (Conv2D + Conv1D)

To reduce computational cost:

- One `Conv2D` layer
- One `Conv1D` layer with **causal padding**

This architecture significantly improves inference speed while maintaining high accuracy.

A clear **accuracy–latency trade-off** emerges across all tested models.

---

## Model Compression

The following techniques were applied to both CNN and TCN-based models:

- **Post-Training Quantization (PTQ)**
- **Quantization-Aware Training (QAT)**
- **Unstructured pruning + QAT**

### Key results

- Quantization introduces **virtually no accuracy loss**
- In some cases, QAT **improves accuracy**
- Both **model size and latency are significantly reduced**
- Quantized CNNs achieve one of the **best overall trade-offs**

---

## Real-Time Embedded Demo

### Hardware

- **STM32 IOT01A2**
- **LSM6DSL 3-axis accelerometer**

---

### Data Preprocessing

- Moving-average low-pass filtering during acquisition
- Data augmentation before training:
  - scaling
  - noise injection
  - rotation
- Standardization (mean subtraction acts as a high-pass filter)
- Input quantization using the same scale and zero-point as the model

---

### Gesture Segmentation

Instead of requiring user interaction:

- Threshold-based segmentation
- Recording starts when acceleration exceeds **1.2 g**
- Fixed window of **32 samples** `(x, y, z)`
- Inference performed immediately after acquisition

---

## Custom Demo Dataset

- **7 gestures**
- Approximately **150 repetitions per gesture**
- Single-user dataset
- Fixed sequence length of **32 samples**

While not suitable for generalization, this dataset is sufficient to validate the **complete embedded inference pipeline**.

---

## Conclusions

- **CNN-based models** are better suited than LSTM or hybrid models for real-time gesture recognition on microcontrollers
- `Conv2D` layers improve accuracy but increase latency
- Quantization is highly effective and often improves the accuracy–latency trade-off
- Carefully designed CNN architectures enable **fast, accurate, and memory-efficient inference** on STM32 devices

---

## Author

**Andrea Massimo Rosetti**


Some important information on the files provided:

-there is one training script for each of the trained models: Kaggle, Hybrid, CNN and TCN.
-the quantization_aware_prune script is used to apply quantization aware training and unstructured pruning. I had some version compatibilities issues with other libraries so I found it easier to have a specific script for that.
-the read_data script is the one I used to record sequences from the accelerometer to create my own datasets.
-the demo script is used to train the demo model.
-the inference_real_time script is used to test the inference capabilities of the microcontroller in real time.
-the 'Gesture' folder is the STMCube project folder. It contains all the files needed to run the code on the board.
-main.c contains all the initializations and preprocessing needed to record data from the accelerometer.

Please refer to "requirements.txt" to have a list of the installed packages.
