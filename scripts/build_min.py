#!/usr/bin/env python3
"""
Ultra-minimal build script for NSE project (Ninja version)
"""

import subprocess
import sys
import time
from pathlib import Path

CONFIGS = ["Debug", "Release", "Shipping", "Strict"]

def main():
    # Get number of cores
    cores = input("Cores [auto]: ").strip()
    if cores and cores.isdigit():
        make_cmd = ["ninja", f"-j{cores}"]
    else:
        make_cmd = ["ninja"]
    
    # Ask about bear
    use_bear = input("Use bear for compile_commands.json? [y/N]: ").strip().lower()
    if use_bear in ['y', 'yes']:
        make_cmd = ["bear", "--"] + make_cmd
    
    # Change to project root
    try:
        import os
        os.chdir(Path(__file__).parent.parent)
    except:
        pass
    
    print("Building all configs...")
    
    config_start = time.time()
    try:
        result = subprocess.run(
            make_cmd + CONFIGS,
            capture_output=True, text=True, check=False
        )
        success = result.returncode == 0
    except:
        success = False
    
    duration = time.time() - config_start
    
    if success:
        print(f"All configs built successfully! ({duration:.1f}s)")
        return 0
    else:
        print(f"Build failed! ({duration:.1f}s)")
        if result.stderr:
            print(f"Error: {result.stderr.strip()}")
        return 1

if __name__ == "__main__":
    try:
        sys.exit(main())
    except KeyboardInterrupt:
        print("\nInterrupted")
        sys.exit(130)
