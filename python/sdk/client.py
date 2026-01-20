"""
ML Inference System - Python Client SDK
High-level Python client for easy inference requests
"""

import grpc
import numpy as np
from typing import Dict, List, Optional, Union, Any
from dataclasses import dataclass
import time

# Import generated protobuf files
# These would be generated from the .proto files
# from generated import manager_pb2, manager_pb2_grpc, worker_pb2, worker_pb2_grpc, common_pb2


@dataclass
class ModelInfo:
    """Model information"""
    model_id: str
    name: str
    version: str
    description: str
    tags: List[str]
    stage: str
    input_schema: Dict[str, Any]
    output_schema: Dict[str, Any]


@dataclass
class InferenceResult:
    """Inference result"""
    request_id: str
    outputs: Dict[str, np.ndarray]
    inference_time_ms: float
    total_time_ms: float
    worker_id: str
    success: bool
    error_message: Optional[str] = None


class InferenceClient:
    """
    High-level client for ML inference
    
    Usage:
        client = InferenceClient(manager_address="localhost:50051")
        
        # Simple inference
        result = client.predict(
            model_id="my-model",
            inputs={"input": np.array([[1.0, 2.0, 3.0]])}
        )
        
        # Batch inference
        results = client.batch_predict(
            model_id="my-model",
            batch_inputs=[
                {"input": np.array([[1.0, 2.0, 3.0]])},
                {"input": np.array([[4.0, 5.0, 6.0]])}
            ]
        )
    """
    
    def __init__(
        self,
        manager_address: str,
        api_key: Optional[str] = None,
        timeout: int = 30,
        use_tls: bool = False,
        tls_cert_path: Optional[str] = None
    ):
        """
        Initialize inference client
        
        Args:
            manager_address: Manager node address (host:port)
            api_key: API key for authentication
            timeout: Default timeout for requests in seconds
            use_tls: Whether to use TLS
            tls_cert_path: Path to TLS certificate
        """
        self.manager_address = manager_address
        self.api_key = api_key
        self.timeout = timeout
        
        # Create gRPC channel to manager
        if use_tls:
            if tls_cert_path:
                with open(tls_cert_path, 'rb') as f:
                    credentials = grpc.ssl_channel_credentials(f.read())
            else:
                credentials = grpc.ssl_channel_credentials()
            self.manager_channel = grpc.secure_channel(manager_address, credentials)
        else:
            self.manager_channel = grpc.insecure_channel(manager_address)
        
        # Create stub (would use generated pb2_grpc)
        # self.manager_stub = manager_pb2_grpc.ManagerServiceStub(self.manager_channel)
        
        # Worker connections cache
        self.worker_channels: Dict[str, grpc.Channel] = {}
        self.worker_stubs: Dict[str, Any] = {}
    
    def predict(
        self,
        model_id: str,
        inputs: Dict[str, np.ndarray],
        model_version: str = "latest",
        use_gpu: bool = True,
        timeout: Optional[int] = None
    ) -> InferenceResult:
        """
        Perform single inference
        
        Args:
            model_id: Model identifier
            inputs: Dictionary of input tensors {name: numpy_array}
            model_version: Model version (default: "latest")
            use_gpu: Whether to use GPU for inference
            timeout: Request timeout in seconds
            
        Returns:
            InferenceResult object
        """
        start_time = time.time()
        
        # Step 1: Get worker from manager
        worker_info = self._get_worker(model_id, model_version)
        
        # Step 2: Connect to worker
        worker_stub = self._get_worker_stub(worker_info)
        
        # Step 3: Perform inference directly on worker
        request_id = self._generate_request_id()
        
        # Convert numpy arrays to tensors (would use protobuf)
        tensor_inputs = self._numpy_to_tensors(inputs)
        
        # Make prediction request
        # predict_request = worker_pb2.PredictRequest(
        #     request_id=request_id,
        #     model_id=model_id,
        #     model_version=model_version,
        #     inputs=tensor_inputs,
        #     session_token=worker_info.session_token,
        #     use_gpu=use_gpu,
        #     timeout_ms=(timeout or self.timeout) * 1000
        # )
        
        # response = worker_stub.Predict(predict_request, timeout=timeout or self.timeout)
        
        # For now, return mock result
        # In real implementation, this would parse the protobuf response
        total_time = (time.time() - start_time) * 1000
        
        return InferenceResult(
            request_id=request_id,
            outputs={},  # Would parse from response
            inference_time_ms=0,  # response.inference_time_ms
            total_time_ms=total_time,
            worker_id=worker_info['worker_id'],
            success=True
        )
    
    def batch_predict(
        self,
        model_id: str,
        batch_inputs: List[Dict[str, np.ndarray]],
        model_version: str = "latest",
        use_gpu: bool = True,
        timeout: Optional[int] = None
    ) -> List[InferenceResult]:
        """
        Perform batch inference
        
        Args:
            model_id: Model identifier
            batch_inputs: List of input dictionaries
            model_version: Model version
            use_gpu: Whether to use GPU
            timeout: Request timeout
            
        Returns:
            List of InferenceResult objects
        """
        results = []
        for inputs in batch_inputs:
            result = self.predict(
                model_id=model_id,
                inputs=inputs,
                model_version=model_version,
                use_gpu=use_gpu,
                timeout=timeout
            )
            results.append(result)
        return results
    
    def get_model_info(self, model_id: str, version: str = "latest") -> ModelInfo:
        """
        Get model information
        
        Args:
            model_id: Model identifier
            version: Model version
            
        Returns:
            ModelInfo object
        """
        # Would call manager gRPC GetModel
        # request = manager_pb2.GetModelRequest(
        #     model_id=model_id,
        #     version=version
        # )
        # response = self.manager_stub.GetModel(request)
        
        # Mock return
        return ModelInfo(
            model_id=model_id,
            name=model_id,
            version=version,
            description="",
            tags=[],
            stage="prod",
            input_schema={},
            output_schema={}
        )
    
    def list_models(
        self,
        stage: Optional[str] = None,
        tags: Optional[List[str]] = None
    ) -> List[ModelInfo]:
        """
        List available models
        
        Args:
            stage: Filter by stage (dev/staging/prod)
            tags: Filter by tags
            
        Returns:
            List of ModelInfo objects
        """
        # Would call manager gRPC ListModels
        return []
    
    def close(self):
        """Close all connections"""
        if self.manager_channel:
            self.manager_channel.close()
        
        for channel in self.worker_channels.values():
            channel.close()
    
    def __enter__(self):
        """Context manager entry"""
        return self
    
    def __exit__(self, exc_type, exc_val, exc_tb):
        """Context manager exit"""
        self.close()
    
    # Private helper methods
    
    def _get_worker(self, model_id: str, model_version: str) -> Dict[str, Any]:
        """Get worker assignment from manager"""
        # Would call manager gRPC GetWorkerForInference
        # request = manager_pb2.GetWorkerRequest(
        #     model_id=model_id,
        #     model_version=model_version,
        #     api_key=self.api_key
        # )
        # response = self.manager_stub.GetWorkerForInference(request)
        
        # Mock return
        return {
            'worker_id': 'worker-1',
            'worker_address': 'localhost',
            'worker_port': 50052,
            'session_token': 'mock-token'
        }
    
    def _get_worker_stub(self, worker_info: Dict[str, Any]) -> Any:
        """Get or create worker stub"""
        worker_address = f"{worker_info['worker_address']}:{worker_info['worker_port']}"
        
        if worker_address not in self.worker_stubs:
            channel = grpc.insecure_channel(worker_address)
            # stub = worker_pb2_grpc.WorkerServiceStub(channel)
            
            self.worker_channels[worker_address] = channel
            # self.worker_stubs[worker_address] = stub
        
        return self.worker_stubs.get(worker_address)
    
    def _numpy_to_tensors(self, inputs: Dict[str, np.ndarray]) -> List[Any]:
        """Convert numpy arrays to protobuf tensors"""
        # Would convert to common_pb2.Tensor
        return []
    
    def _generate_request_id(self) -> str:
        """Generate unique request ID"""
        import uuid
        return str(uuid.uuid4())


# Convenience function
def create_client(
    manager_address: str = "localhost:50051",
    api_key: Optional[str] = None,
    **kwargs
) -> InferenceClient:
    """
    Create an inference client
    
    Args:
        manager_address: Manager node address
        api_key: API key for authentication
        **kwargs: Additional arguments for InferenceClient
        
    Returns:
        InferenceClient instance
    """
    return InferenceClient(
        manager_address=manager_address,
        api_key=api_key,
        **kwargs
    )
