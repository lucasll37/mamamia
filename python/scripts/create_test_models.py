#!/usr/bin/env python3

"""
Script to create simple ONNX models for testing.
Creates basic models that can be used to test the inference engine.
"""

import numpy as np
import sys
import os

def create_simple_model():
    """Create a simple linear model: y = 2*x + 1"""
    
    try:
        import onnx
        from onnx import helper, TensorProto
    except ImportError:
        print("Error: onnx package not found")
        print("Install with: pip install onnx")
        sys.exit(1)
    
    # Input tensor
    input_tensor = helper.make_tensor_value_info(
        'input',
        TensorProto.FLOAT,
        [1, 5]  # Batch size 1, 5 features
    )
    
    # Output tensor
    output_tensor = helper.make_tensor_value_info(
        'output',
        TensorProto.FLOAT,
        [1, 5]
    )
    
    # Scale: multiply by 2 (element-wise)
    scale = np.array([2.0], dtype=np.float32)
    scale_tensor = helper.make_tensor(
        'scale',
        TensorProto.FLOAT,
        [1],
        scale.tolist()
    )
    
    # Bias: add 1 (element-wise)
    bias = np.array([1.0], dtype=np.float32)
    bias_tensor = helper.make_tensor(
        'bias',
        TensorProto.FLOAT,
        [1],
        bias.tolist()
    )
    
    # Mul node: input * 2
    mul_node = helper.make_node(
        'Mul',
        inputs=['input', 'scale'],
        outputs=['scaled_output']
    )
    
    # Add node: scaled_output + 1
    add_node = helper.make_node(
        'Add',
        inputs=['scaled_output', 'bias'],
        outputs=['output']
    )
    
    # Create graph
    graph = helper.make_graph(
        [mul_node, add_node],
        'simple_linear_model',
        [input_tensor],
        [output_tensor],
        [scale_tensor, bias_tensor]
    )
    
    # Create model
    model = helper.make_model(graph, producer_name='mlinference')
    model.opset_import[0].version = 13
    
    # Check model
    onnx.checker.check_model(model)
    
    return model

def create_classification_model():
    """Create a simple classification model with softmax"""
    
    try:
        import onnx
        from onnx import helper, TensorProto
    except ImportError:
        print("Error: onnx package not found")
        print("Install with: pip install onnx")
        sys.exit(1)
    
    # Input: 4 features
    input_tensor = helper.make_tensor_value_info(
        'input',
        TensorProto.FLOAT,
        [1, 4]
    )
    
    # Output: 3 classes
    output_tensor = helper.make_tensor_value_info(
        'output',
        TensorProto.FLOAT,
        [1, 3]
    )
    
    # Weights for classification
    weights = np.random.randn(4, 3).astype(np.float32) * 0.1
    weights_tensor = helper.make_tensor(
        'weights',
        TensorProto.FLOAT,
        [4, 3],
        weights.flatten().tolist()
    )
    
    # Bias
    bias = np.zeros(3, dtype=np.float32)
    bias_tensor = helper.make_tensor(
        'bias',
        TensorProto.FLOAT,
        [3],
        bias.tolist()
    )
    
    # MatMul
    matmul_node = helper.make_node(
        'MatMul',
        inputs=['input', 'weights'],
        outputs=['matmul_output']
    )
    
    # Add bias
    add_node = helper.make_node(
        'Add',
        inputs=['matmul_output', 'bias'],
        outputs=['logits']
    )
    
    # Softmax
    softmax_node = helper.make_node(
        'Softmax',
        inputs=['logits'],
        outputs=['output'],
        axis=1
    )
    
    # Create graph
    graph = helper.make_graph(
        [matmul_node, add_node, softmax_node],
        'simple_classifier',
        [input_tensor],
        [output_tensor],
        [weights_tensor, bias_tensor]
    )
    
    # Create model
    model = helper.make_model(graph, producer_name='mlinference')
    model.opset_import[0].version = 13
    
    # Check model
    onnx.checker.check_model(model)
    
    return model

def main():
    try:
        import onnx
    except ImportError:
        print("=" * 60)
        print("ERROR: onnx package not found")
        print("=" * 60)
        print()
        print("Please install required packages:")
        print("  pip install onnx numpy")
        print()
        sys.exit(1)
    
    # Create output directory
    output_dir = 'models'
    os.makedirs(output_dir, exist_ok=True)
    
    print("=" * 60)
    print("Creating ONNX Test Models")
    print("=" * 60)
    print()
    
    # Create simple linear model
    print("Creating simple linear model (y = 2*x + 1)...")
    try:
        linear_model = create_simple_model()
        linear_path = os.path.join(output_dir, 'simple_linear.onnx')
        onnx.save(linear_model, linear_path)
        print(f"✓ Saved to: {linear_path}")
        print(f"  Input:  'input' [1, 5] float32")
        print(f"  Output: 'output' [1, 5] float32")
        print(f"  Function: y = 2*x + 1")
        print()
    except Exception as e:
        print(f"✗ Failed to create linear model: {e}")
        print()
    
    # Create classification model
    print("Creating simple classification model...")
    try:
        classifier_model = create_classification_model()
        classifier_path = os.path.join(output_dir, 'simple_classifier.onnx')
        onnx.save(classifier_model, classifier_path)
        print(f"✓ Saved to: {classifier_path}")
        print(f"  Input:  'input' [1, 4] float32")
        print(f"  Output: 'output' [1, 3] float32")
        print(f"  Function: softmax(Wx + b)")
        print()
    except Exception as e:
        print(f"✗ Failed to create classifier model: {e}")
        print()
    
    print("=" * 60)
    print("Models created successfully!")
    print("=" * 60)
    print()
    print("Test with:")
    print(f"  1. Start worker: ./build/bin/worker")
    print(f"  2. Run client:   ./build/bin/client_example")
    print()

if __name__ == '__main__':
    main()



# #!/usr/bin/env python3

# """
# Script to create simple ONNX models for testing.
# Creates basic models that can be used to test the inference engine.
# """

# import numpy as np
# import sys
# import os

# def create_simple_model():
#     """Create a simple linear model: y = 2*x + 1"""
    
#     try:
#         import onnx
#         from onnx import helper, TensorProto
#     except ImportError:
#         print("Error: onnx package not found")
#         print("Install with: pip install onnx")
#         sys.exit(1)
    
#     # Input tensor
#     input_tensor = helper.make_tensor_value_info(
#         'input',
#         TensorProto.FLOAT,
#         [1, 5]  # Batch size 1, 5 features
#     )
    
#     # Output tensor
#     output_tensor = helper.make_tensor_value_info(
#         'output',
#         TensorProto.FLOAT,
#         [1, 5]
#     )
    
#     # Weights: multiply by 2
#     weights = np.array([[2.0]], dtype=np.float32)
#     weights_tensor = helper.make_tensor(
#         'weights',
#         TensorProto.FLOAT,
#         [1, 1],
#         weights.flatten().tolist()
#     )
    
#     # Bias: add 1
#     bias = np.array([1.0], dtype=np.float32)
#     bias_tensor = helper.make_tensor(
#         'bias',
#         TensorProto.FLOAT,
#         [1],
#         bias.tolist()
#     )
    
#     # MatMul node: input * weights
#     matmul_node = helper.make_node(
#         'MatMul',
#         inputs=['input', 'weights'],
#         outputs=['matmul_output']
#     )
    
#     # Add node: matmul_output + bias
#     add_node = helper.make_node(
#         'Add',
#         inputs=['matmul_output', 'bias'],
#         outputs=['output']
#     )
    
#     # Create graph
#     graph = helper.make_graph(
#         [matmul_node, add_node],
#         'simple_linear_model',
#         [input_tensor],
#         [output_tensor],
#         [weights_tensor, bias_tensor]
#     )
    
#     # Create model
#     model = helper.make_model(graph, producer_name='mlinference')
#     model.opset_import[0].version = 13
    
#     # Check model
#     onnx.checker.check_model(model)
    
#     return model

# def create_classification_model():
#     """Create a simple classification model with softmax"""
    
#     try:
#         import onnx
#         from onnx import helper, TensorProto
#     except ImportError:
#         print("Error: onnx package not found")
#         print("Install with: pip install onnx")
#         sys.exit(1)
    
#     # Input: 4 features
#     input_tensor = helper.make_tensor_value_info(
#         'input',
#         TensorProto.FLOAT,
#         [1, 4]
#     )
    
#     # Output: 3 classes
#     output_tensor = helper.make_tensor_value_info(
#         'output',
#         TensorProto.FLOAT,
#         [1, 3]
#     )
    
#     # Weights for classification
#     weights = np.random.randn(4, 3).astype(np.float32) * 0.1
#     weights_tensor = helper.make_tensor(
#         'weights',
#         TensorProto.FLOAT,
#         [4, 3],
#         weights.flatten().tolist()
#     )
    
#     # Bias
#     bias = np.zeros(3, dtype=np.float32)
#     bias_tensor = helper.make_tensor(
#         'bias',
#         TensorProto.FLOAT,
#         [3],
#         bias.tolist()
#     )
    
#     # MatMul
#     matmul_node = helper.make_node(
#         'MatMul',
#         inputs=['input', 'weights'],
#         outputs=['matmul_output']
#     )
    
#     # Add bias
#     add_node = helper.make_node(
#         'Add',
#         inputs=['matmul_output', 'bias'],
#         outputs=['logits']
#     )
    
#     # Softmax
#     softmax_node = helper.make_node(
#         'Softmax',
#         inputs=['logits'],
#         outputs=['output'],
#         axis=1
#     )
    
#     # Create graph
#     graph = helper.make_graph(
#         [matmul_node, add_node, softmax_node],
#         'simple_classifier',
#         [input_tensor],
#         [output_tensor],
#         [weights_tensor, bias_tensor]
#     )
    
#     # Create model
#     model = helper.make_model(graph, producer_name='mlinference')
#     model.opset_import[0].version = 13
    
#     # Check model
#     onnx.checker.check_model(model)
    
#     return model

# def main():
#     try:
#         import onnx
#     except ImportError:
#         print("=" * 60)
#         print("ERROR: onnx package not found")
#         print("=" * 60)
#         print()
#         print("Please install required packages:")
#         print("  pip install onnx numpy")
#         print()
#         sys.exit(1)
    
#     # Create output directory
#     output_dir = 'models'
#     os.makedirs(output_dir, exist_ok=True)
    
#     print("=" * 60)
#     print("Creating ONNX Test Models")
#     print("=" * 60)
#     print()
    
#     # Create simple linear model
#     print("Creating simple linear model (y = 2*x + 1)...")
#     try:
#         linear_model = create_simple_model()
#         linear_path = os.path.join(output_dir, 'simple_linear.onnx')
#         onnx.save(linear_model, linear_path)
#         print(f"✓ Saved to: {linear_path}")
#         print(f"  Input:  'input' [1, 5] float32")
#         print(f"  Output: 'output' [1, 5] float32")
#         print(f"  Function: y = 2*x + 1")
#         print()
#     except Exception as e:
#         print(f"✗ Failed to create linear model: {e}")
#         print()
    
#     # Create classification model
#     print("Creating simple classification model...")
#     try:
#         classifier_model = create_classification_model()
#         classifier_path = os.path.join(output_dir, 'simple_classifier.onnx')
#         onnx.save(classifier_model, classifier_path)
#         print(f"✓ Saved to: {classifier_path}")
#         print(f"  Input:  'input' [1, 4] float32")
#         print(f"  Output: 'output' [1, 3] float32")
#         print(f"  Function: softmax(Wx + b)")
#         print()
#     except Exception as e:
#         print(f"✗ Failed to create classifier model: {e}")
#         print()
    
#     print("=" * 60)
#     print("Models created successfully!")
#     print("=" * 60)
#     print()
#     print("Test with:")
#     print(f"  1. Start worker: ./build/bin/worker")
#     print(f"  2. Run client:   ./build/bin/client_example")
#     print()
    
#     # Create test script
#     test_script = """#!/usr/bin/env python3

# import numpy as np

# print("=" * 60)
# print("Testing ONNX Models")
# print("=" * 60)
# print()

# # Test the linear model
# print("Linear Model (y = 2*x + 1)")
# print("-" * 40)

# # Input
# x = np.array([[1.0, 2.0, 3.0, 4.0, 5.0]], dtype=np.float32)
# print(f"Input:    {x}")

# # Expected output
# y_expected = 2 * x + 1
# print(f"Expected: {y_expected}")
# print()

# # Test the classifier
# print("Classifier Model")
# print("-" * 40)

# # Input
# x_class = np.array([[1.0, 2.0, 3.0, 4.0]], dtype=np.float32)
# print(f"Input: {x_class}")
# print("Output: Probability distribution over 3 classes")
# print("Sum should be ~1.0")
# print()

# print("=" * 60)
# print("To test with ONNX Runtime:")
# print("=" * 60)
# print()
# print("import onnxruntime as ort")
# print()
# print("# Linear model")
# print("session = ort.InferenceSession('models/simple_linear.onnx')")
# print("result = session.run(None, {'input': x})")
# print("print('Output:', result[0])")
# print()
# """
    
#     test_path = os.path.join(output_dir, 'test_models.py')
#     with open(test_path, 'w') as f:
#         f.write(test_script)
#     os.chmod(test_path, 0o755)
#     print(f"Test script saved to: {test_path}")
#     print()
    
#     # Create README
#     readme_content = """# ONNX Test Models

# This directory contains simple ONNX models for testing the inference engine.

# ## Models

# ### simple_linear.onnx
# - **Function**: y = 2*x + 1
# - **Input**: 'input' [1, 5] float32
# - **Output**: 'output' [1, 5] float32
# - **Example**:
#   - Input: [1, 2, 3, 4, 5]
#   - Output: [3, 5, 7, 9, 11]

# ### simple_classifier.onnx
# - **Function**: softmax(Wx + b)
# - **Input**: 'input' [1, 4] float32
# - **Output**: 'output' [1, 3] float32 (probabilities)
# - **Example**:
#   - Input: [1.0, 2.0, 3.0, 4.0]
#   - Output: [0.33, 0.33, 0.34] (approximately)

# ## Testing

# ### With Python
# ```python
# import onnxruntime as ort
# import numpy as np

# # Load model
# session = ort.InferenceSession('models/simple_linear.onnx')

# # Prepare input
# x = np.array([[1.0, 2.0, 3.0, 4.0, 5.0]], dtype=np.float32)

# # Run inference
# result = session.run(None, {'input': x})

# # Print result
# print('Output:', result[0])
# # Expected: [[3. 5. 7. 9. 11.]]
# ```

# ### With C++ Worker
# ```bash
# # Terminal 1: Start worker
# ./build/bin/worker

# # Terminal 2: Run client
# ./build/bin/client_example
# ```

# ## Creating Your Own Models

# ### PyTorch → ONNX
# ```python
# import torch

# model = YourModel()
# dummy_input = torch.randn(1, 3, 224, 224)

# torch.onnx.export(
#     model, dummy_input, "your_model.onnx",
#     input_names=['input'],
#     output_names=['output']
# )
# ```

# ### TensorFlow → ONNX
# ```python
# import tf2onnx
# import tensorflow as tf

# model = tf.keras.models.load_model('your_model.h5')
# spec = (tf.TensorSpec((None, 224, 224, 3), tf.float32),)

# model_proto, _ = tf2onnx.convert.from_keras(
#     model, input_signature=spec, output_path="your_model.onnx"
# )
# ```

# ## Validation

# To validate an ONNX model:
# ```python
# import onnx

# model = onnx.load('model.onnx')
# onnx.checker.check_model(model)
# print('Model is valid!')
# ```

# ## Troubleshooting

# ### "Model is not a valid ONNX model"
# - Check opset version (should be >= 13)
# - Validate with: `onnx.checker.check_model(model)`

# ### "Shape mismatch"
# - Verify input shape matches model expectation
# - Use dynamic shapes if needed

# ### "Operator not supported"
# - Check ONNX Runtime version
# - Some ops require specific execution providers
# """
    
#     readme_path = os.path.join(output_dir, 'README.md')
#     with open(readme_path, 'w') as f:
#         f.write(readme_content)
#     print(f"README saved to: {readme_path}")
#     print()

# if __name__ == '__main__':
#     main()