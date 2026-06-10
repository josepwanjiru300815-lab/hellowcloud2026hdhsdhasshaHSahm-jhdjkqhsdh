from flask import Flask, render_template, jsonify, request

app = Flask(__name__)

state = {
    "pump":    "OFF",
    "valve":   "STOPPED",
    "temp":    0.0,
    "hum":     0.0,
    "soil1":   0.0,
    "soil2":   0.0,
    "fault1":  False,
    "fault2":  False,
    "percent": 0.0,
    "volume":  0,
}

pending_commands = {
    "pump":  None,
    "valve": None,
}

@app.route("/update", methods=["POST"])
def update():
    data = request.get_json(silent=True)
    if not data:
        return jsonify({"error": "no data"}), 400
    for key in ("pump", "valve", "temp", "hum",
                "soil1", "soil2", "fault1", "fault2",
                "percent", "volume"):
        if key in data:
            state[key] = data[key]
    cmds = {}
    if pending_commands["pump"]:
        cmds["pump"] = pending_commands["pump"]
        pending_commands["pump"] = None
    if pending_commands["valve"]:
        cmds["valve"] = pending_commands["valve"]
        pending_commands["valve"] = None
    return jsonify(cmds)

@app.route("/")
def index():
    return render_template("index.html", **state)

@app.route("/status")
def status():
    return jsonify(state)

@app.route("/pump/on")
def pump_on():
    pending_commands["pump"] = "ON"
    state["pump"] = "ON"
    return ("", 204)

@app.route("/pump/off")
def pump_off():
    pending_commands["pump"] = "OFF"
    state["pump"] = "OFF"
    return ("", 204)

@app.route("/valve/open")
def valve_open():
    pending_commands["valve"] = "OPEN"
    state["valve"] = "OPENING"
    return ("", 204)

@app.route("/valve/close")
def valve_close():
    pending_commands["valve"] = "CLOSE"
    state["valve"] = "CLOSING"
    return ("", 204)

@app.route("/valve/stop")
def valve_stop():
    pending_commands["valve"] = "STOP"
    state["valve"] = "STOPPED"
    return ("", 204)

if __name__ == "__main__":
    app.run(debug=True)
