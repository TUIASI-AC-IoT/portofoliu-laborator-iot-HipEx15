from flask import Flask, jsonify, request
import random

app = Flask(__name__)

sensors = [
    {"id": "0", "state": "none", "config": None},
    {"id": "1", "state": "pending", "config": "modified"}
]

# === GET: Printeaza lista cu toti senzorii ===
@app.route('/sensors', methods=['GET'])
def get_all_sensors():
    for sensor in sensors:
        sensor['temp'] = round(random.uniform(10.0, 100.0), 2)
    return jsonify(sensors)

# === GET: Printeaza senzorul cu id-ul X ===
@app.route('/sensors/<sensorId>', methods=['GET'])
def get_sensor_by_id(sensorId):
    for sensor in sensors:
        if sensor['id'] == sensorId:
            sensor['temp'] = round(random.uniform(10.0, 100.0), 2)
            return jsonify(sensor)
    return jsonify({'error': 'Sensor not found'}), 404

# === GET: Printeaza temperatura senzorului cu id-ul X ===
@app.route('/sensors/<sensorId>/temp', methods=['GET'])
def get_temp_by_sensor(sensorId):
    for sensor in sensors:
        if sensor['id'] == sensorId:
            temp = round(random.uniform(10.0, 100.0), 2)
            sensor['temp'] = temp
            return jsonify({'temp': temp})
    return jsonify({'error': 'Sensor not found'}), 404

# === GET: Printeaza starea senzorului cu id-ul X ===
@app.route('/sensors/<sensorId>/state', methods=['GET'])
def get_state(sensorId):
    for sensor in sensors:
        if sensor['id'] == sensorId:
            return jsonify({'state': sensor['state']})
    return jsonify({'error': 'Sensor not found'}), 404

# === GET: Configurarea senzorului cu id-ul X ===
@app.route('/sensors/<sensorId>/config', methods=['GET'])
def get_config(sensorId):
    for sensor in sensors:
        if sensor['id'] == sensorId:
            if sensor['config'] is None:
                return jsonify({'message': 'No configuration file exists'}), 404
            return jsonify({'config': sensor['config']})
    return jsonify({'error': 'Sensor not found'}), 404

# === POST: Crearea configuratie pentru senzorul cu id-ul X ===
@app.route('/sensors/<sensorId>/config', methods=['POST'])
def create_config(sensorId):
    for sensor in sensors:
        if sensor['id'] == sensorId:
            if sensor['config'] is not None:
                return jsonify({'error': 'Configuration already exists'}), 409
            data = request.get_json()
            if not data or 'config' not in data:
                return jsonify({'error': 'Missing config data'}), 400
            sensor['config'] = data['config']
            return jsonify({'message': 'Configuration created', 'config': sensor['config']}), 201
    return jsonify({'error': 'Sensor not found'}), 404

# === PUT: Reconfigurare senzorului cu id-ul X ===
@app.route('/sensors/<sensorId>/config', methods=['PUT'])
def update_config(sensorId):
    for sensor in sensors:
        if sensor['id'] == sensorId:
            if sensor['config'] is None:
                return jsonify({'error': 'Configuration does not exist'}), 409
            data = request.get_json()
            if not data or 'config' not in data:
                return jsonify({'error': 'Missing config data'}), 400
            sensor['config'] = data['config']
            return jsonify({'message': 'Configuration updated', 'config': sensor['config']}), 200
    return jsonify({'error': 'Sensor not found'}), 404

# === DELETE: Stergerea configuratiei senzorului cu id-ul X ===
@app.route('/sensors/<sensorId>/config', methods=['DELETE'])
def delete_config(sensorId):
    for sensor in sensors:
        if sensor['id'] == sensorId:
            sensor['config'] = None
            return jsonify({'message': 'Configuration deleted'}), 200
    return jsonify({'error': 'Sensor not found'}), 404

if __name__ == '__main__':
    app.run(debug=True)
