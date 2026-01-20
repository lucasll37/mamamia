"""
ML Inference System - API Gateway
FastAPI REST API for easy access to the inference system
"""

from fastapi import FastAPI, HTTPException, Depends, status
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import JSONResponse
from contextlib import asynccontextmanager
import uvicorn
from typing import Optional

from .config import settings
from .routes import inference, models, workers, health
from .middleware.auth import validate_api_key
from .utils.grpc_client import ManagerGRPCClient

# Global manager client
manager_client: Optional[ManagerGRPCClient] = None


@asynccontextmanager
async def lifespan(app: FastAPI):
    """Startup and shutdown events"""
    global manager_client
    
    # Startup
    print(f"Starting API Gateway on {settings.HOST}:{settings.PORT}")
    print(f"Connecting to Manager at {settings.MANAGER_ADDRESS}:{settings.MANAGER_PORT}")
    
    manager_client = ManagerGRPCClient(
        address=settings.MANAGER_ADDRESS,
        port=settings.MANAGER_PORT
    )
    
    if not manager_client.connect():
        raise RuntimeError("Failed to connect to Manager node")
    
    print("Successfully connected to Manager")
    
    yield
    
    # Shutdown
    print("Shutting down API Gateway")
    if manager_client:
        manager_client.close()


# Create FastAPI app
app = FastAPI(
    title="ML Inference System API",
    description="REST API Gateway for distributed ML inference",
    version="1.0.0",
    lifespan=lifespan
)

# CORS middleware
if settings.ENABLE_CORS:
    app.add_middleware(
        CORSMiddleware,
        allow_origins=settings.CORS_ORIGINS,
        allow_credentials=True,
        allow_methods=["*"],
        allow_headers=["*"],
    )


# Exception handlers
@app.exception_handler(HTTPException)
async def http_exception_handler(request, exc):
    return JSONResponse(
        status_code=exc.status_code,
        content={
            "error": exc.detail,
            "status_code": exc.status_code
        }
    )


@app.exception_handler(Exception)
async def general_exception_handler(request, exc):
    return JSONResponse(
        status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
        content={
            "error": "Internal server error",
            "detail": str(exc)
        }
    )


# Include routers
app.include_router(
    health.router,
    tags=["health"]
)

app.include_router(
    inference.router,
    prefix="/api/v1/inference",
    tags=["inference"]
)

app.include_router(
    models.router,
    prefix="/api/v1/models",
    tags=["models"],
    dependencies=[Depends(validate_api_key)]
)

app.include_router(
    workers.router,
    prefix="/api/v1/workers",
    tags=["workers"],
    dependencies=[Depends(validate_api_key)]
)


@app.get("/")
async def root():
    """Root endpoint"""
    return {
        "service": "ML Inference System API Gateway",
        "version": "1.0.0",
        "status": "online",
        "docs": "/docs",
        "manager": f"{settings.MANAGER_ADDRESS}:{settings.MANAGER_PORT}"
    }


def get_manager_client() -> ManagerGRPCClient:
    """Dependency to get manager client"""
    if manager_client is None:
        raise HTTPException(
            status_code=status.HTTP_503_SERVICE_UNAVAILABLE,
            detail="Manager client not initialized"
        )
    return manager_client


def main():
    """Run the API Gateway"""
    uvicorn.run(
        "api_gateway.main:app",
        host=settings.HOST,
        port=settings.PORT,
        workers=settings.WORKERS,
        reload=settings.DEBUG,
        log_level=settings.LOG_LEVEL.lower()
    )


if __name__ == "__main__":
    main()
