#!/usr/bin/env python3

import numpy as np

print("=" * 60)
print("Testing ONNX Models")
print("=" * 60)
print()

# Test the linear model
print("Linear Model (y = 2*x + 1)")
print("-" * 40)

# Input
x = np.array([[1.0, 2.0, 3.0, 4.0, 5.0]], dtype=np.float32)
print(f"Input:    {x}")

# Expected output
y_expected = 2 * x + 1
print(f"Expected: {y_expected}")
print()

# Test the classifier
print("Classifier Model")
print("-" * 40)

# Input
x_class = np.array([[1.0, 2.0, 3.0, 4.0]], dtype=np.float32)
print(f"Input: {x_class}")
print("Output: Probability distribution over 3 classes")
print("Sum should be ~1.0")
print()

print("=" * 60)
print("To test with ONNX Runtime:")
print("=" * 60)
print()
print("import onnxruntime as ort")
print()
print("# Linear model")
print("session = ort.InferenceSession('models/simple_linear.onnx')")
print("result = session.run(None, {'input': x})")
print("print('Output:', result[0])")
print()
