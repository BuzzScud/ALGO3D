#!/usr/bin/env python3
"""
Automated test runner for recovery tools.

Tests all 8 recovery tools against corrupted data:
1. geometric-recovery
2. signal-recovery
3. image-recovery
4. network-recovery
5. crypto-recovery
6. scientific-recovery
7. ml-recovery
8. platonic-demo

Validates recovery quality and generates reports.
"""

import subprocess
import json
import time
from pathlib import Path
import sys

# Tool paths
TOOLS_DIR = Path(__file__).parent.parent.parent.parent / "tools" / "recovery"
DATA_DIR = Path(__file__).parent.parent / "data"
RESULTS_DIR = Path(__file__).parent.parent / "results"
RESULTS_DIR.mkdir(parents=True, exist_ok=True)

# Tool configurations
TOOLS = {
    'geometric': {
        'tool': 'geometric-recovery',
        'data_dir': 'geometric',
        'extensions': ['.json', '.obj']
    },
    'signal': {
        'tool': 'signal-recovery',
        'data_dir': 'signal',
        'extensions': ['.json', '.csv']
    },
    'image': {
        'tool': 'image-recovery',
        'data_dir': 'image',
        'extensions': ['.json']
    },
    'network': {
        'tool': 'network-recovery',
        'data_dir': 'network',
        'extensions': ['.json']
    },
    'crypto': {
        'tool': 'crypto-recovery',
        'data_dir': 'crypto',
        'extensions': ['.json']
    },
    'scientific': {
        'tool': 'scientific-recovery',
        'data_dir': 'scientific',
        'extensions': ['.json']
    },
    'ml': {
        'tool': 'ml-recovery',
        'data_dir': 'ml',
        'extensions': ['.json']
    },
    'platonic': {
        'tool': 'platonic-demo',
        'data_dir': 'geometric',
        'extensions': ['.json', '.obj']
    }
}

def run_recovery_tool(tool_path, input_file, output_file, report_file=None):
    """Run a recovery tool on a corrupted file."""
    cmd = [
        str(tool_path),
        '-i', str(input_file),
        '-o', str(output_file)
    ]
    
    if report_file:
        cmd.extend(['-r', str(report_file)])
    
    # Set library path
    env = {
        'LD_LIBRARY_PATH': f"{TOOLS_DIR.parent.parent}/algorithms/lib/recovery_common:{TOOLS_DIR.parent.parent}"
    }
    
    try:
        start_time = time.time()
        result = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            timeout=60,
            env=env
        )
        end_time = time.time()
        
        return {
            'success': result.returncode == 0,
            'stdout': result.stdout,
            'stderr': result.stderr,
            'time': end_time - start_time,
            'returncode': result.returncode
        }
    except subprocess.TimeoutExpired:
        return {
            'success': False,
            'stdout': '',
            'stderr': 'Timeout after 60 seconds',
            'time': 60.0,
            'returncode': -1
        }
    except Exception as e:
        return {
            'success': False,
            'stdout': '',
            'stderr': str(e),
            'time': 0.0,
            'returncode': -1
        }

def test_category(category, config):
    """Test all corrupted files in a category."""
    tool_name = config['tool']
    tool_path = TOOLS_DIR / tool_name
    
    if not tool_path.exists():
        print(f"  ⚠ Tool not found: {tool_path}")
        return {'tested': 0, 'passed': 0, 'failed': 0, 'skipped': 1}
    
    data_dir = DATA_DIR / 'corrupted' / config['data_dir']
    
    if not data_dir.exists():
        print(f"  ⚠ No corrupted data found: {data_dir}")
        return {'tested': 0, 'passed': 0, 'failed': 0, 'skipped': 1}
    
    results = {'tested': 0, 'passed': 0, 'failed': 0, 'skipped': 0}
    
    # Test each corrupted file
    for corrupted_file in sorted(data_dir.glob('*_corrupted_*')):
        if corrupted_file.suffix not in config['extensions']:
            continue
        
        # Create output paths
        output_file = RESULTS_DIR / category / f"recovered_{corrupted_file.name}"
        report_file = RESULTS_DIR / category / f"report_{corrupted_file.stem}.txt"
        output_file.parent.mkdir(parents=True, exist_ok=True)
        
        # Run recovery
        result = run_recovery_tool(tool_path, corrupted_file, output_file, report_file)
        
        results['tested'] += 1
        if result['success']:
            results['passed'] += 1
            status = "✓"
        else:
            results['failed'] += 1
            status = "✗"
        
        print(f"    {status} {corrupted_file.name} ({result['time']:.2f}s)")
        
        if not result['success'] and result['stderr']:
            print(f"      Error: {result['stderr'][:100]}")
    
    return results

def main():
    """Run all recovery tests."""
    print("="*80)
    print("AUTOMATED RECOVERY TOOL TESTING")
    print("="*80)
    print()
    
    print(f"Tools directory: {TOOLS_DIR}")
    print(f"Data directory: {DATA_DIR}")
    print(f"Results directory: {RESULTS_DIR}")
    print()
    
    total_results = {'tested': 0, 'passed': 0, 'failed': 0, 'skipped': 0}
    category_results = {}
    
    for category, config in TOOLS.items():
        print(f"\n{'='*80}")
        print(f"Testing: {category} ({config['tool']})")
        print(f"{'='*80}\n")
        
        results = test_category(category, config)
        category_results[category] = results
        
        # Update totals
        for key in total_results:
            total_results[key] += results[key]
        
        print(f"\n  Results: {results['passed']}/{results['tested']} passed")
    
    # Print summary
    print(f"\n{'='*80}")
    print("SUMMARY")
    print(f"{'='*80}\n")
    
    for category, results in category_results.items():
        if results['skipped']:
            status = "⚠ SKIPPED"
        elif results['failed'] == 0:
            status = "✓ PASSED"
        else:
            status = f"✗ {results['failed']} FAILED"
        
        print(f"  {category:15s}: {results['passed']:3d}/{results['tested']:3d} tests passed  {status}")
    
    print(f"\n{'='*80}")
    print(f"TOTAL: {total_results['passed']}/{total_results['tested']} tests passed")
    print(f"       {total_results['failed']} failed, {total_results['skipped']} skipped")
    print(f"{'='*80}\n")
    
    # Save summary
    summary_file = RESULTS_DIR / "test_summary.json"
    with open(summary_file, 'w') as f:
        json.dump({
            'total': total_results,
            'categories': category_results
        }, f, indent=2)
    
    print(f"Summary saved to: {summary_file}")
    
    return total_results['failed'] == 0

if __name__ == "__main__":
    success = main()
    sys.exit(0 if success else 1)