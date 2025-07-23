#!/usr/bin/env python3

import os
import shutil
import glob
from pathlib import Path

def clean_project():
    """
    Clean all build artifacts, generated files, and temporary files from the project.
    """
    
    # Current directory (project root)
    project_root = Path.cwd()
    
    print(f"Cleaning project at: {project_root}")
    
    # Files and patterns to remove
    cleanup_patterns = [
        # Build artifacts
        "**/*.ninja",
        "**/*.o",
        "**/Makefile",
        "**/*.plist",
        
        # Common build/cache directories
        "build/**",
        "**/obj/**",
        "**/__pycache__/**",
        "**/.cache/**",
        "**/node_modules/**",
        
        # IDE and editor files
        "**/.vscode/**",
        "**/.idea/**",
        "**/*.swp",
        "**/*.swo",
        "**/*~",
        "**/.DS_Store",
        
        # Compilation database
        "compile_commands.json",
        "build.ninja",
        
        # Log files
        "**/*.log",
        "**/*.tmp",
        
        # Other common artifacts
        "**/*.pyc",
        "**/*.pyo",
        "**/.pytest_cache/**",
    ]
    
    # Files to keep (whitelist - be careful with these)
    keep_files = [
        "Makefile.template",  # In case you have template files
        "premake5.lua",       # Keep build system configs
        "config.lua",
        "*.lua",              # Keep lua configs generally
    ]
    
    # Directories to completely avoid
    avoid_dirs = [
        ".git",
        ".svn",
        ".hg"
    ]
    
    removed_count = 0
    kept_count = 0
    
    print("\nScanning for files to remove...")
    
    for pattern in cleanup_patterns:
        matches = list(project_root.glob(pattern))
        
        for match in matches:
            # Skip if it's in a directory we should avoid
            should_avoid = False
            for avoid_dir in avoid_dirs:
                if avoid_dir in match.parts:
                    should_avoid = True
                    break
            
            if should_avoid:
                continue
            
            # Skip if it's in the keep list
            should_keep = False
            for keep_pattern in keep_files:
                if match.match(keep_pattern) or keep_pattern in str(match):
                    should_keep = True
                    break
            
            if should_keep:
                print(f"Keeping: {match.relative_to(project_root)}")
                kept_count += 1
                continue
            
            try:
                if match.is_file():
                    match.unlink()
                    print(f"Removed file: {match.relative_to(project_root)}")
                    removed_count += 1
                elif match.is_dir():
                    shutil.rmtree(match)
                    print(f"Removed directory: {match.relative_to(project_root)}")
                    removed_count += 1
            except Exception as e:
                print(f"Error removing {match}: {e}")
    
    # Clean empty directories
    print("\nRemoving empty directories...")
    empty_dirs_removed = 0
    
    # Walk from bottom to top to remove nested empty dirs
    for root, dirs, files in os.walk(project_root, topdown=False):
        for dir_name in dirs:
            dir_path = Path(root) / dir_name
            
            # Skip if it's in a directory we should avoid
            should_avoid = False
            for avoid_dir in avoid_dirs:
                if avoid_dir in dir_path.parts:
                    should_avoid = True
                    break
            
            if should_avoid:
                continue
                
            try:
                if dir_path.exists() and not any(dir_path.iterdir()):
                    dir_path.rmdir()
                    print(f"Removed empty directory: {dir_path.relative_to(project_root)}")
                    empty_dirs_removed += 1
            except Exception as e:
                # Directory might not be empty or have permission issues
                pass
    
    print(f"\nCleanup complete!")
    print(f"   Files/directories removed: {removed_count}")
    print(f"   Empty directories removed: {empty_dirs_removed}")
    print(f"   Files kept (protected): {kept_count}")

def dry_run():
    """
    Show what would be removed without actually removing anything.
    """
    project_root = Path.cwd()
    
    print(f"🔍 DRY RUN - Showing what would be removed from: {project_root}")
    
    cleanup_patterns = [
        "**/*.ninja", "**/*.o", "**/Makefile", "**/*.plist",
        "build/**", "**/obj/**", "**/__pycache__/**",
        "compile_commands.json", "build.ninja"
    ]
    
    keep_files = ["premake5.lua", "config.lua"]
    
    would_remove = []
    would_keep = []
    
    for pattern in cleanup_patterns:
        matches = list(project_root.glob(pattern))
        
        for match in matches:
            should_keep = False
            for keep_pattern in keep_files:
                if match.match(keep_pattern) or keep_pattern in str(match):
                    should_keep = True
                    break
            
            if should_keep:
                would_keep.append(match)
            else:
                would_remove.append(match)
    
    print(f"\nWould remove ({len(would_remove)} items):")
    for item in would_remove:
        print(f"   - {item.relative_to(project_root)}")
    
    print(f"\nWould keep ({len(would_keep)} items):")
    for item in would_keep:
        print(f"   - {item.relative_to(project_root)}")

if __name__ == "__main__":
    import sys
    
    print("NSE Project Cleanup Tool")
    print("=" * 40)
    
    if len(sys.argv) > 1 and sys.argv[1] == "--dry-run":
        dry_run()
    elif len(sys.argv) > 1 and sys.argv[1] == "--help":
        print("Usage:")
        print("  python cleanup.py           # Clean the project")
        print("  python cleanup.py --dry-run # Show what would be removed")
        print("  python cleanup.py --help    # Show this help")
    else:
        response = input("\nThis will permanently delete build artifacts. Continue? (y/N): ")
        if response.lower() in ['y', 'yes']:
            clean_project()
        else:
            print("Cleanup cancelled.")
