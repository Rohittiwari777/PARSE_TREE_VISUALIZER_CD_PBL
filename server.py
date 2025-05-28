from flask import Flask, request, jsonify, send_from_directory
import subprocess
import os

app = Flask(__name__)

# Serve static files
@app.route('/')
def index():
    return send_from_directory('.', 'index.html')

@app.route('/<path:path>')
def serve_static(path):
    return send_from_directory('.', path)

# Save code to input.cpp
@app.route('/save-code', methods=['POST'])
def save_code():
    try:
        code = request.json.get('code')
        with open('input.cpp', 'w') as f:
            f.write(code)
        return jsonify({'success': True})
    except Exception as e:
        return jsonify({'error': str(e)}), 500

# Run the parser
@app.route('/run-parser', methods=['POST'])
def run_parser():
    try:
        # Compile and run the parser
        subprocess.run(['g++', 'parse.cpp', '-o', 'parser'], check=True)
        subprocess.run(['./parser'], check=True)
        return jsonify({'success': True})
    except subprocess.CalledProcessError as e:
        return jsonify({'error': f'Parser error: {str(e)}'}), 500
    except Exception as e:
        return jsonify({'error': str(e)}), 500

if __name__ == '__main__':
    app.run(port=3000, debug=True) 
