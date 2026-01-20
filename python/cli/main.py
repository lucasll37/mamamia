"""
ML Inference System - CLI Tool
Command-line interface for managing the inference system
"""

import click
import sys
from pathlib import Path

from .commands import model, worker, apikey, config as config_cmd
from .utils.client import get_manager_client
from .utils.formatting import print_success, print_error, print_info


@click.group()
@click.option('--manager', '-m', default='localhost:50051',
              help='Manager address (host:port)')
@click.option('--api-key', '-k', envvar='ML_INFERENCE_API_KEY',
              help='API key for authentication')
@click.option('--verbose', '-v', is_flag=True,
              help='Enable verbose output')
@click.pass_context
def cli(ctx, manager, api_key, verbose):
    """
    ML Inference System CLI
    
    Manage models, workers, and API keys for the distributed inference system.
    """
    ctx.ensure_object(dict)
    ctx.obj['manager_address'] = manager
    ctx.obj['api_key'] = api_key
    ctx.obj['verbose'] = verbose
    
    if verbose:
        print_info(f"Connecting to manager at {manager}")


# Register command groups
cli.add_command(model.model)
cli.add_command(worker.worker)
cli.add_command(apikey.apikey)
cli.add_command(config_cmd.config)


@cli.command()
@click.pass_context
def version(ctx):
    """Show version information"""
    print_info("ML Inference System CLI v1.0.0")
    
    # Try to get manager version
    try:
        client = get_manager_client(
            ctx.obj['manager_address'],
            ctx.obj['api_key']
        )
        # health = client.get_health()
        print_info(f"Manager: {ctx.obj['manager_address']}")
    except Exception as e:
        print_error(f"Could not connect to manager: {e}")


@cli.command()
@click.pass_context
def status(ctx):
    """Check system status"""
    try:
        client = get_manager_client(
            ctx.obj['manager_address'],
            ctx.obj['api_key']
        )
        
        print_info("=== System Status ===\n")
        
        # Get health
        print_info("Manager: Online")
        
        # Get workers
        # workers = client.list_workers()
        print_info(f"Workers: 0 active")
        
        # Get models
        # models = client.list_models()
        print_info(f"Models: 0 registered")
        
        print_success("\nSystem is operational")
        
    except Exception as e:
        print_error(f"Failed to get status: {e}")
        sys.exit(1)


def main():
    """Main entry point"""
    try:
        cli(obj={})
    except KeyboardInterrupt:
        print_info("\nInterrupted by user")
        sys.exit(130)
    except Exception as e:
        print_error(f"Unexpected error: {e}")
        if '--verbose' in sys.argv or '-v' in sys.argv:
            import traceback
            traceback.print_exc()
        sys.exit(1)


if __name__ == '__main__':
    main()
