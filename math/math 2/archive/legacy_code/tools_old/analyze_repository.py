#!/usr/bin/env python3
"""
Comprehensive Repository File Analyzer
Generates detailed inventory of all files in the repository
"""

import os
import sys
from pathlib import Path
from collections import defaultdict
import subprocess

def get_file_size(filepath):
    """Get file size in bytes"""
    try:
        return os.path.getsize(filepath)
    except:
        return 0

def get_file_lines(filepath):
    """Get number of lines in text file"""
    try:
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            return sum(1 for _ in f)
    except:
        return 0

def is_git_tracked(filepath):
    """Check if file is tracked by git"""
    try:
        result = subprocess.run(
            ['git', 'ls-files', '--error-unmatch', filepath],
            capture_output=True,
            text=True,
            cwd='/workspace'
        )
        return result.returncode == 0
    except:
        return False

def analyze_repository():
    """Analyze all files in repository"""
    
    # Exclude patterns
    exclude_dirs = {'.git', 'build', 'output', '__pycache__', '.vscode', '.idea'}
    
    # File categories
    categories = defaultdict(list)
    
    # Statistics
    stats = {
        'total_files': 0,
        'total_size': 0,
        'tracked_files': 0,
        'untracked_files': 0,
        'by_extension': defaultdict(int),
        'by_category': defaultdict(int)
    }
    
    print("Scanning repository...")
    
    # Walk through all files
    for root, dirs, files in os.walk('/workspace'):
        # Remove excluded directories
        dirs[:] = [d for d in dirs if d not in exclude_dirs]
        
        for filename in files:
            filepath = os.path.join(root, filename)
            relpath = os.path.relpath(filepath, '/workspace')
            
            # Get file info
            size = get_file_size(filepath)
            ext = Path(filename).suffix.lower()
            tracked = is_git_tracked(relpath)
            
            # Update statistics
            stats['total_files'] += 1
            stats['total_size'] += size
            stats['by_extension'][ext if ext else 'no_extension'] += 1
            
            if tracked:
                stats['tracked_files'] += 1
            else:
                stats['untracked_files'] += 1
            
            # Categorize file
            category = categorize_file(filename, ext)
            stats['by_category'][category] += 1
            
            # Store file info
            file_info = {
                'path': relpath,
                'size': size,
                'ext': ext,
                'tracked': tracked,
                'category': category
            }
            
            # Add line count for text files
            if category in ['source_c', 'source_h', 'source_py', 'source_sh', 'documentation']:
                file_info['lines'] = get_file_lines(filepath)
            
            categories[category].append(file_info)
    
    return categories, stats

def categorize_file(filename, ext):
    """Categorize file by extension and name"""
    
    # Source code
    if ext == '.c':
        return 'source_c'
    elif ext == '.h':
        return 'source_h'
    elif ext == '.py':
        return 'source_py'
    elif ext == '.sh':
        return 'source_sh'
    
    # Documentation
    elif ext == '.md':
        return 'documentation'
    
    # Data files
    elif ext == '.txt':
        return 'data_txt'
    elif ext == '.log':
        return 'data_log'
    elif ext == '.bin':
        return 'data_bin'
    elif ext == '.cllm':
        return 'data_cllm'
    
    # Compiled files (should not exist)
    elif ext == '.o':
        return 'compiled_object'
    elif ext == '.so':
        return 'compiled_shared'
    elif ext == '.a':
        return 'compiled_static'
    
    # Images
    elif ext in ['.png', '.jpg', '.jpeg', '.gif', '.bmp']:
        return 'image'
    
    # Archives
    elif ext in ['.zip', '.tar', '.gz', '.bz2']:
        return 'archive'
    
    # Build files
    elif filename in ['Makefile', 'makefile']:
        return 'build_system'
    elif ext == '.mk':
        return 'build_system'
    
    # Config files
    elif filename in ['.gitignore', '.gitattributes']:
        return 'config'
    
    # Other
    else:
        return 'other'

def format_size(size):
    """Format size in human-readable format"""
    for unit in ['B', 'KB', 'MB', 'GB']:
        if size < 1024.0:
            return f"{size:.1f} {unit}"
        size /= 1024.0
    return f"{size:.1f} TB"

def print_report(categories, stats):
    """Print comprehensive report"""
    
    print("\n" + "="*80)
    print("COMPREHENSIVE REPOSITORY ANALYSIS")
    print("="*80)
    
    # Overall statistics
    print("\n### OVERALL STATISTICS")
    print(f"Total Files: {stats['total_files']:,}")
    print(f"Total Size: {format_size(stats['total_size'])}")
    print(f"Git Tracked: {stats['tracked_files']:,}")
    print(f"Untracked: {stats['untracked_files']:,}")
    
    # By category
    print("\n### FILES BY CATEGORY")
    for category, count in sorted(stats['by_category'].items(), key=lambda x: x[1], reverse=True):
        print(f"{category:30s}: {count:5d} files")
    
    # By extension
    print("\n### FILES BY EXTENSION (Top 20)")
    for ext, count in sorted(stats['by_extension'].items(), key=lambda x: x[1], reverse=True)[:20]:
        print(f"{ext:20s}: {count:5d} files")
    
    # Source code details
    print("\n### SOURCE CODE DETAILS")
    
    for cat in ['source_c', 'source_h', 'source_py', 'source_sh']:
        if cat in categories:
            files = categories[cat]
            total_lines = sum(f.get('lines', 0) for f in files)
            print(f"\n{cat.upper()}:")
            print(f"  Files: {len(files)}")
            print(f"  Total Lines: {total_lines:,}")
            print(f"  Avg Lines/File: {total_lines//len(files) if files else 0}")
    
    # Documentation details
    if 'documentation' in categories:
        files = categories['documentation']
        total_lines = sum(f.get('lines', 0) for f in files)
        print(f"\nDOCUMENTATION:")
        print(f"  Files: {len(files)}")
        print(f"  Total Lines: {total_lines:,}")
    
    # Compiled files (should be zero)
    compiled_cats = ['compiled_object', 'compiled_shared', 'compiled_static']
    compiled_count = sum(stats['by_category'][cat] for cat in compiled_cats)
    if compiled_count > 0:
        print(f"\n⚠️  WARNING: {compiled_count} compiled files found (should be 0)")
    
    # Untracked files
    if stats['untracked_files'] > 0:
        print(f"\n### UNTRACKED FILES: {stats['untracked_files']}")
        print("(These files exist but are not in git)")

def save_detailed_inventory(categories, stats):
    """Save detailed inventory to file"""
    
    output_file = '/workspace/REPOSITORY_INVENTORY.txt'
    
    with open(output_file, 'w') as f:
        f.write("="*80 + "\n")
        f.write("COMPREHENSIVE REPOSITORY INVENTORY\n")
        f.write("="*80 + "\n\n")
        
        # Statistics
        f.write("### STATISTICS\n")
        f.write(f"Total Files: {stats['total_files']:,}\n")
        f.write(f"Total Size: {format_size(stats['total_size'])}\n")
        f.write(f"Git Tracked: {stats['tracked_files']:,}\n")
        f.write(f"Untracked: {stats['untracked_files']:,}\n\n")
        
        # Detailed file listing by category
        for category in sorted(categories.keys()):
            files = categories[category]
            f.write(f"\n{'='*80}\n")
            f.write(f"CATEGORY: {category.upper()} ({len(files)} files)\n")
            f.write(f"{'='*80}\n\n")
            
            # Sort by size (largest first)
            files_sorted = sorted(files, key=lambda x: x['size'], reverse=True)
            
            for file_info in files_sorted:
                f.write(f"File: {file_info['path']}\n")
                f.write(f"  Size: {format_size(file_info['size'])}\n")
                f.write(f"  Tracked: {'Yes' if file_info['tracked'] else 'No'}\n")
                if 'lines' in file_info:
                    f.write(f"  Lines: {file_info['lines']:,}\n")
                f.write("\n")
    
    print(f"\nDetailed inventory saved to: {output_file}")

if __name__ == '__main__':
    categories, stats = analyze_repository()
    print_report(categories, stats)
    save_detailed_inventory(categories, stats)
    
    print("\n" + "="*80)
    print("Analysis complete!")
    print("="*80)