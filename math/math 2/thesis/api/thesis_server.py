#!/usr/bin/env python3
"""
Thesis REST API Server (Python alternative to PHP)
Serves thesis content via HTTP
"""

from http.server import HTTPServer, BaseHTTPRequestHandler
import json
import os
import re
from pathlib import Path
from urllib.parse import urlparse, parse_qs
from datetime import datetime

# Base directory
BASE_DIR = Path(__file__).parent.parent

class ThesisAPIHandler(BaseHTTPRequestHandler):
    
    def do_GET(self):
        """Handle GET requests"""
        parsed_url = urlparse(self.path)
        query_params = parse_qs(parsed_url.query)
        action = query_params.get('action', ['structure'])[0]
        
        # CORS headers
        self.send_response(200)
        self.send_header('Content-Type', 'application/json')
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, POST, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        self.end_headers()
        
        try:
            if action == 'structure':
                response = self.get_structure()
            elif action == 'content':
                path = query_params.get('path', [''])[0]
                response = self.get_content(path)
            elif action == 'search':
                query = query_params.get('query', [''])[0]
                response = self.search_content(query)
            elif action == 'stats':
                response = self.get_stats()
            else:
                response = {'success': False, 'error': 'Invalid action'}
        except Exception as e:
            response = {'success': False, 'error': str(e)}
        
        self.wfile.write(json.dumps(response, indent=2).encode())
    
    def do_OPTIONS(self):
        """Handle OPTIONS requests (CORS preflight)"""
        self.send_response(200)
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, POST, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        self.end_headers()
    
    def get_structure(self):
        """Get directory structure"""
        structure = self._scan_directory(BASE_DIR)
        return {'success': True, 'data': structure}
    
    def _scan_directory(self, path, base_path=''):
        """Recursively scan directory"""
        structure = []
        
        if not path.is_dir():
            return structure
        
        for item in sorted(path.iterdir()):
            # Skip hidden, api, and archive directories
            if item.name.startswith('.') or item.name in ['api', 'archive']:
                continue
            
            rel_path = str(item.relative_to(BASE_DIR))
            
            if item.is_dir():
                # Only include part_ and chapter_ directories
                if re.match(r'^part_\d+_', item.name) or re.match(r'^chapter_\d+_', item.name):
                    structure.append({
                        'type': 'directory',
                        'name': item.name,
                        'path': rel_path,
                        'displayName': self._format_name(item.name),
                        'children': self._scan_directory(item, rel_path)
                    })
            elif item.suffix == '.md':
                structure.append({
                    'type': 'file',
                    'name': item.name,
                    'path': rel_path,
                    'displayName': self._format_name(item.name),
                    'size': item.stat().st_size
                })
        
        return structure
    
    def _format_name(self, name):
        """Format name for display"""
        # Remove .md extension
        name = re.sub(r'\.md$', '', name)
        # Replace underscores with spaces
        name = name.replace('_', ' ')
        # Capitalize words
        name = name.title()
        # Handle special cases
        name = name.replace('Qa', 'Q&A')
        name = name.replace('Ntt', 'NTT')
        name = name.replace('Ai', 'AI')
        name = name.replace('88d', '88D')
        return name
    
    def get_content(self, path):
        """Get file content"""
        if not path:
            return {'success': False, 'error': 'Path parameter required'}
        
        full_path = BASE_DIR / path
        
        # Security check
        try:
            full_path = full_path.resolve()
            BASE_DIR.resolve()
            if not str(full_path).startswith(str(BASE_DIR.resolve())):
                return {'success': False, 'error': 'Invalid path'}
        except:
            return {'success': False, 'error': 'Invalid path'}
        
        if not full_path.exists():
            return {'success': False, 'error': 'File not found'}
        
        content = full_path.read_text(encoding='utf-8')
        stats = full_path.stat()
        
        return {
            'success': True,
            'data': {
                'content': content,
                'stats': {
                    'size': stats.st_size,
                    'modified': int(stats.st_mtime),
                    'lines': content.count('\n') + 1
                },
                'path': path
            }
        }
    
    def search_content(self, query):
        """Search content"""
        if not query:
            return {'success': False, 'error': 'Query parameter required'}
        
        results = []
        query_lower = query.lower()
        
        for md_file in BASE_DIR.rglob('*.md'):
            # Skip archive
            if 'archive' in str(md_file):
                continue
            
            rel_path = str(md_file.relative_to(BASE_DIR))
            content = md_file.read_text(encoding='utf-8')
            
            if query_lower in content.lower():
                lines = content.split('\n')
                matches = []
                
                for line_num, line in enumerate(lines):
                    if query_lower in line.lower():
                        matches.append({
                            'line': line_num + 1,
                            'text': line.strip(),
                            'context': lines[max(0, line_num-1):line_num+2]
                        })
                        
                        if len(matches) >= 3:
                            break
                
                if matches:
                    results.append({
                        'file': rel_path,
                        'displayName': self._format_name(md_file.name),
                        'matches': matches
                    })
        
        return {'success': True, 'data': results}
    
    def get_stats(self):
        """Get statistics"""
        stats = {
            'totalFiles': 0,
            'totalLines': 0,
            'totalSize': 0,
            'chapters': 0,
            'parts': 0
        }
        
        for md_file in BASE_DIR.rglob('*.md'):
            # Skip archive
            if 'archive' in str(md_file):
                continue
            
            rel_path = str(md_file.relative_to(BASE_DIR))
            
            stats['totalFiles'] += 1
            stats['totalSize'] += md_file.stat().st_size
            
            content = md_file.read_text(encoding='utf-8')
            stats['totalLines'] += content.count('\n') + 1
            
            if 'chapter_' in rel_path:
                stats['chapters'] += 1
            if 'part_' in rel_path and md_file.name == 'README.md':
                stats['parts'] += 1
        
        return {'success': True, 'data': stats}
    
    def log_message(self, format, *args):
        """Suppress default logging"""
        pass

def run_server(port=8000):
    """Run the server"""
    server_address = ('', port)
    httpd = HTTPServer(server_address, ThesisAPIHandler)
    print(f'🚀 Thesis API Server running on http://localhost:{port}')
    print(f'📖 Open http://localhost:{port}/index.html to view the thesis')
    print('Press Ctrl+C to stop')
    httpd.serve_forever()

if __name__ == '__main__':
    import sys
    port = int(sys.argv[1]) if len(sys.argv) > 1 else 8000
    run_server(port)