# ONNX Test Models

This directory contains simple ONNX models for testing the inference engine.

## Models

### simple_linear.onnx
- **Function**: y = 2*x + 1
- **Input**: 'input' [1, 5] float32
- **Output**: 'output' [1, 5] float32
- **Example**:
  - Input: [1, 2, 3, 4, 5]
  - Output: [3, 5, 7, 9, 11]

### simple_classifier.onnx
- **Function**: softmax(Wx + b)
- **Input**: 'input' [1, 4] float32
- **Output**: 'output' [1, 3] float32 (probabilities)
- **Example**:
  - Input: [1.0, 2.0, 3.0, 4.0]
  - Output: [0.33, 0.33, 0.34] (approximately)

## Testing

### With Python
```python
import onnxruntime as ort
import numpy as np

# Load model
session = ort.InferenceSession('models/simple_linear.onnx')

# Prepare input
x = np.array([[1.0, 2.0, 3.0, 4.0, 5.0]], dtype=np.float32)

# Run inference
result = session.run(None, {'input': x})

# Print result
print('Output:', result[0])
# Expected: [[3. 5. 7. 9. 11.]]
```

### With C++ Worker
```bash
# Terminal 1: Start worker
./build/bin/worker

# Terminal 2: Run client
./build/bin/client_example
```

## Creating Your Own Models

### PyTorch → ONNX
```python
import torch

model = YourModel()
dummy_input = torch.randn(1, 3, 224, 224)

torch.onnx.export(
    model, dummy_input, "your_model.onnx",
    input_names=['input'],
    output_names=['output']
)
```

### TensorFlow → ONNX
```python
import tf2onnx
import tensorflow as tf

model = tf.keras.models.load_model('your_model.h5')
spec = (tf.TensorSpec((None, 224, 224, 3), tf.float32),)

model_proto, _ = tf2onnx.convert.from_keras(
    model, input_signature=spec, output_path="your_model.onnx"
)
```

## Validation

To validate an ONNX model:
```python
import onnx

model = onnx.load('model.onnx')
onnx.checker.check_model(model)
print('Model is valid!')
```

## Troubleshooting

### "Model is not a valid ONNX model"
- Check opset version (should be >= 13)
- Validate with: `onnx.checker.check_model(model)`

### "Shape mismatch"
- Verify input shape matches model expectation
- Use dynamic shapes if needed

### "Operator not supported"
- Check ONNX Runtime version
- Some ops require specific execution providers
