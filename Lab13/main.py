from flask import Flask, request, jsonify
from flask_jwt_extended import (
    JWTManager, create_access_token, jwt_required,
    get_jwt_identity, get_jwt
)
from datetime import timedelta
import json

app = Flask(__name__)
app.config["JWT_SECRET_KEY"] = "super-secret"
app.config["JWT_ACCESS_TOKEN_EXPIRES"] = timedelta(hours=1)

jwt = JWTManager(app)

users = {
    "user1": {"password": "parola1", "role": "admin"},
    "user2": {"password": "parola2", "role": "owner"},
}

jwt_store = {}

@app.route("/auth", methods=["POST"])
def login():
    data = request.get_json()
    username = data.get("username")
    password = data.get("password")

    user = users.get(username)
    if user and user["password"] == password:
        identity_json = json.dumps({"username": username, "role": user["role"]})
        access_token = create_access_token(identity=identity_json)
        jwt_store[access_token] = user["role"]
        return jsonify(access_token=access_token), 200
    return jsonify(msg="Invalid credentials"), 401


@app.route("/auth/jwtStore", methods=["GET"])
@jwt_required()
def validate_token():
    jwt_raw = request.headers.get("Authorization", "").replace("Bearer ", "")
    identity_json = get_jwt_identity()

    try:
        identity = json.loads(identity_json)
        role = identity.get("role", "guest")
    except Exception:
        return jsonify(msg="Invalid token format"), 400

    if jwt_raw in jwt_store:
        return jsonify(role=role), 200
    return jsonify(msg="Token not found"), 404


@app.route("/auth/jwtStore", methods=["DELETE"])
@jwt_required()
def logout():
    jwt_raw = request.headers.get("Authorization", "").replace("Bearer ", "")
    if jwt_raw in jwt_store:
        del jwt_store[jwt_raw]
        return jsonify(msg="Token invalidated"), 200
    return jsonify(msg="Token not found"), 404


if __name__ == "__main__":
    app.run(debug=True)
