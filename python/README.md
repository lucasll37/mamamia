# Python Environment - ML Inference System

This directory contains Python tools for creating and testing ONNX models.

## 📁 Structure

```
python/
├── .venv/                    # Virtual environment (created by make python-env)
├── models/                  # Generated ONNX models
├── scripts/                 # Python scripts
│   ├── create_test_models.py    # Create test models
│   ├── convert_pytorch.py       # Convert PyTorch models (optional)
│   ├── convert_tensorflow.py    # Convert TensorFlow models (optional)
│   └── validate_model.py        # Validate ONNX models (optional)
├── requirements.txt         # Python dependencies
└── README.md               # This file
```

## 🚀 Quick Start

### Using Makefile (Recommended)

```bash
# Setup Python environment
make python-env

# Install dependencies
make python-install

# Create test models
make create-models

# Test environment
make python-test
```

### Manual Setup

```bash
# Create virtual environment
python3.12 -m venv python/.venv

# Activate
source python/.venv/bin/activate

# Install dependencies
pip install -r python/requirements.txt

# Create models
cd python
python scripts/create_test_models.py
```

## 📦 Installed Packages

### Core Dependencies
- **onnx** - ONNX model creation and manipulation
- **numpy** - Numerical computing
- **onnxruntime** - ONNX Runtime for testing
- **protobuf** - Protocol Buffers support

### Optional Conversion Tools
Uncomment in `requirements.txt` as needed:
- **torch** - PyTorch model conversion
- **tf2onnx** - TensorFlow/Keras conversion
- **skl2onnx** - Scikit-learn conversion

## 🎨 Creating Models

### Test Models (Simple)

```bash
make create-models
```

This creates:
- `models/simple_linear.onnx` - Linear model (y = 2*x + 1)
- `models/simple_classifier.onnx` - Softmax classifier

### Convert PyTorch Model

```python
import torch

# Your model
model = YourModel()
model.eval()

# Dummy input
dummy_input = torch.randn(1, 3, 224, 224)

# Export to ONNX
torch.onnx.export(
    model,
    dummy_input,
    "python/models/your_model.onnx",
    input_names=['input'],
    output_names=['output'],
    dynamic_axes={
        'input': {0: 'batch_size'},
        'output': {0: 'batch_size'}
    }
)
```

### Convert TensorFlow Model

```python
import tf2onnx
import tensorflow as tf

# Load model
model = tf.keras.models.load_model('your_model.h5')

# Convert
spec = (tf.TensorSpec((None, 224, 224, 3), tf.float32, name="input"),)

model_proto, _ = tf2onnx.convert.from_keras(
    model,
    input_signature=spec,
    output_path="python/models/your_model.onnx"
)
```

### Convert Scikit-learn Model

```python
from skl2onnx import convert_sklearn
from skl2onnx.common.data_types import FloatTensorType

# Your trained model
model = your_sklearn_model

# Define input shape
initial_type = [('float_input', FloatTensorType([None, 4]))]

# Convert
onnx_model = convert_sklearn(
    model,
    initial_types=initial_type,
    target_opset=13
)

# Save
with open("python/models/your_model.onnx", "wb") as f:
    f.write(onnx_model.SerializeToString())
```

## 🧪 Testing Models

### With ONNX Runtime (Python)

```python
import onnxruntime as ort
import numpy as np

# Load model
session = ort.InferenceSession('python/models/simple_linear.onnx')

# Get input/output info
input_name = session.get_inputs()[0].name
output_name = session.get_outputs()[0].name

print(f"Input: {input_name}")
print(f"Output: {output_name}")

# Prepare input
x = np.array([[1.0, 2.0, 3.0, 4.0, 5.0]], dtype=np.float32)

# Run inference
result = session.run([output_name], {input_name: x})

print(f"Result: {result[0]}")
# Expected: [[3. 5. 7. 9. 11.]]
```

### With C++ Worker

```bash
# Terminal 1: Start worker
make run-worker

# Terminal 2: Test with client
make run-client
```

## 🔍 Model Validation

### Validate ONNX Model

```python
import onnx

# Load model
model = onnx.load('python/models/your_model.onnx')

# Check model
onnx.checker.check_model(model)
print("✓ Model is valid!")

# Print model info
print(f"\nModel IR version: {model.ir_version}")
print(f"Producer: {model.producer_name}")
print(f"Opset version: {model.opset_import[0].version}")

# Print inputs
print("\nInputs:")
for inp in model.graph.input:
    print(f"  - {inp.name}: {[d.dim_value for d in inp.type.tensor_type.shape.dim]}")

# Print outputs
print("\nOutputs:")
for out in model.graph.output:
    print(f"  - {out.name}: {[d.dim_value for d in out.type.tensor_type.shape.dim]}")
```

### Visualize Model (Optional)

```bash
# Install netron
pip install netron

# Visualize
netron python/models/your_model.onnx
```

## 📊 Model Performance Testing

```python
import onnxruntime as ort
import numpy as np
import time

# Load model
session = ort.InferenceSession('python/models/your_model.onnx')

# Prepare input
x = np.random.randn(1, 3, 224, 224).astype(np.float32)
input_name = session.get_inputs()[0].name

# Warmup
for _ in range(10):
    session.run(None, {input_name: x})

# Benchmark
num_runs = 100
start = time.time()
for _ in range(num_runs):
    session.run(None, {input_name: x})
end = time.time()

avg_time = (end - start) / num_runs * 1000  # ms
print(f"Average inference time: {avg_time:.2f} ms")
print(f"Throughput: {1000/avg_time:.2f} FPS")
```

## 🛠️ Useful Scripts

### List All Models

```bash
ls -lh python/models/*.onnx
```

### Clean Models

```bash
rm -rf python/models/*.onnx
```

### Clean Virtual Environment

```bash
make python-clean
```

## 🐛 Troubleshooting

### "No module named 'onnx'"

```bash
make python-install
# Or manually:
source python/.venv/bin/activate
pip install -r python/requirements.txt
```

### "Python 3.12 not found"

```bash
# Ubuntu/Debian
sudo apt-get install python3.12 python3.12-venv

# macOS
brew install python@3.12
```

### Virtual environment not activating

```bash
# Recreate environment
make python-clean
make python-env
```

### Import errors in scripts

```bash
# Make sure virtual environment is activated
source python/.venv/bin/activate

# Or use make commands (automatically uses .venv)
make create-models
```

## 📚 Resources

- [ONNX Documentation](https://onnx.ai/onnx/)
- [ONNX Runtime](https://onnxruntime.ai/)
- [PyTorch ONNX Export](https://pytorch.org/docs/stable/onnx.html)
- [TensorFlow ONNX Conversion](https://github.com/onnx/tensorflow-onnx)
- [Scikit-learn ONNX](https://github.com/onnx/sklearn-onnx)

## 💡 Tips

1. **Always use the virtual environment** for Python operations
2. **Test models in Python** before deploying to C++ worker
3. **Keep models small** for faster loading and inference
4. **Use dynamic shapes** for flexible batch sizes
5. **Validate models** before deployment