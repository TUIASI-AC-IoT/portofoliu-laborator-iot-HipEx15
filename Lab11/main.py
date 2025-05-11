from flask import Flask, request, jsonify
import os
import uuid

app = Flask(__name__)
BASE_DIR = 'files'

if not os.path.exists(BASE_DIR):
    os.makedirs(BASE_DIR)


@app.route('/files', methods=['GET'])
def list_files():
    return jsonify(os.listdir(BASE_DIR))


@app.route('/files/<filename>', methods=['GET'])
def get_file(filename):
    filepath = os.path.join(BASE_DIR, filename)
    if not os.path.exists(filepath):
        return jsonify({'error': 'File not found'}), 404
    with open(filepath, 'r') as f:
        content = f.read()
    return jsonify({'filename': filename, 'content': content})


@app.route('/files/<filename>', methods=['PUT'])
def put_file(filename):
    data = request.get_json()
    content = data.get('content', '')
    filepath = os.path.join(BASE_DIR, filename)

    # Creăm sau modificăm
    with open(filepath, 'w') as f:
        f.write(content)

    if os.path.exists(filepath):
        return jsonify({'message': f'File {filename} created or updated'}), 200
    else:
        return jsonify({'message': f'File {filename} created'}), 201


@app.route('/files', methods=['POST'])
def post_file():
    data = request.get_json()
    content = data.get('content', '')
    filename = f"{uuid.uuid4().hex}.txt"
    filepath = os.path.join(BASE_DIR, filename)

    with open(filepath, 'w') as f:
        f.write(content)

    return jsonify({'message': 'File created', 'filename': filename}), 201


@app.route('/files/<filename>', methods=['DELETE'])
def delete_file(filename):
    filepath = os.path.join(BASE_DIR, filename)
    if not os.path.exists(filepath):
        return jsonify({'error': 'File not found'}), 404
    os.remove(filepath)
    return jsonify({'message': f'File {filename} deleted'}), 200


if __name__ == '__main__':
    app.run(debug=True)
