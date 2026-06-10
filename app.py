from flask import Flask, render_template, jsonify
import random

app = Flask(__name__)

# ──────────────────────────────────────────────
# Mock state  (replace with real ESP32 polling
# if your device has a public IP / tunnel)
# ──────────────────────────────────────────────
state = {
    "pump":    "OFF",
    "valve":   "STOPPED",
    "temp":    27.4,
    "hum":     58.2,
    "soil1":   62.0,
    "soil2":   45.0,
    "fault1":  False,
    "fault2":  False,
    "percent": 73.0,
    "volume":  730,
}


@app.route("/")
def index():
    return render_template("index.html", **state)


@app.route("/status")
def status():
    """
    In a real deployment you could forward this request to your ESP32:
        import requests
        r = requests.get("http://<ESP32-IP>/status", timeout=2)
        return r.content, r.status_code, {"Content-Type": "application/json"}
    For now we return the mock state with slight random drift so the
    dashboard visually updates.
    """
    state["temp"]    = round(state["temp"]    + random.uniform(-0.2, 0.2), 1)
    state["hum"]     = round(state["hum"]     + random.uniform(-0.5, 0.5), 1)
    state["soil1"]   = round(min(100, max(0, state["soil1"] + random.uniform(-1, 1))), 1)
    state["soil2"]   = round(min(100, max(0, state["soil2"] + random.uniform(-1, 1))), 1)
    state["percent"] = round(min(100, max(0, state["percent"] + random.uniform(-0.5, 0.5))), 1)
    state["volume"]  = int(state["percent"] / 100 * 1000)
    return jsonify(state)


# ── pump controls ──────────────────────────────
@app.route("/pump/on")
def pump_on():
    state["pump"] = "ON"
    return ("", 204)


@app.route("/pump/off")
def pump_off():
    state["pump"] = "OFF"
    return ("", 204)


# ── valve controls ─────────────────────────────
@app.route("/valve/open")
def valve_open():
    state["valve"] = "OPENED"
    return ("", 204)


@app.route("/valve/close")
def valve_close():
    state["valve"] = "CLOSED"
    return ("", 204)


@app.route("/valve/stop")
def valve_stop():
    state["valve"] = "STOPPED"
    return ("", 204)


if __name__ == "__main__":
    app.run(debug=True)
