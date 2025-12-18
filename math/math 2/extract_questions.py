import re
import sys

def extract_questions(filename):
    """Extract all questions from a markdown file."""
    questions = []
    
    try:
        with open(filename, 'r', encoding='utf-8') as f:
            lines = f.readlines()
            
        for i, line in enumerate(lines, 1):
            # Look for questions (lines ending with ?)
            if '?' in line:
                # Skip code lines (ternary operators)
                if '? ' in line and ':' in line and '==' in line:
                    continue
                    
                # Extract actual questions
                # Pattern 1: **Question?**
                match = re.search(r'\*\*([^*]+\?)\*\*', line)
                if match:
                    questions.append({
                        'line': i,
                        'question': match.group(1).strip(),
                        'context': line.strip(),
                        'file': filename
                    })
                    continue
                
                # Pattern 2: Question at end of line
                if line.strip().endswith('?'):
                    # Skip if it's a code line
                    if not any(x in line for x in ['==', '!=', '?:', 'return']):
                        questions.append({
                            'line': i,
                            'question': line.strip(),
                            'context': line.strip(),
                            'file': filename
                        })
                        continue
                
                # Pattern 3: Question in middle of text
                matches = re.findall(r'([A-Z][^.!?]*\?)', line)
                for match in matches:
                    if not any(x in match for x in ['==', '!=', '?:']):
                        questions.append({
                            'line': i,
                            'question': match.strip(),
                            'context': line.strip(),
                            'file': filename
                        })
    
    except Exception as e:
        print(f"Error reading {filename}: {e}", file=sys.stderr)
    
    return questions

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Usage: python extract_questions.py <file1> [file2] ...")
        sys.exit(1)
    
    all_questions = []
    for filename in sys.argv[1:]:
        questions = extract_questions(filename)
        all_questions.extend(questions)
    
    # Print results
    print(f"# QUESTIONS CATALOG\n")
    print(f"Total questions found: {len(all_questions)}\n")
    
    # Group by file
    by_file = {}
    for q in all_questions:
        if q['file'] not in by_file:
            by_file[q['file']] = []
        by_file[q['file']].append(q)
    
    for filename, questions in by_file.items():
        print(f"\n## {filename} ({len(questions)} questions)\n")
        for q in questions:
            print(f"**Line {q['line']}:** {q['question']}")
            print()

