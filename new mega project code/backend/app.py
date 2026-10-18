import os
import sys
import json
import time
import sqlite3
import hashlib
import base64
import secrets
import asyncio
import threading
import subprocess
from datetime import datetime
from functools import wraps
from flask import Flask, request, jsonify, render_template, send_from_directory, Response

# ==============================================================================
# CONFIGURATION & GLOBAL CONSTANTS
# ==============================================================================
BASE_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DATA_DIR = os.path.join(BASE_DIR, "data")
BIN_DIR = os.path.join(BASE_DIR, "native", "bin")
STATIC_DIR = os.path.join(BASE_DIR, "frontend")
DB_PATH = os.path.join(DATA_DIR, "master_portal.db")

os.makedirs(DATA_DIR, exist_ok=True)
os.makedirs(BIN_DIR, exist_ok=True)

app = Flask(__name__, static_folder=STATIC_DIR)
app.config['SECRET_KEY'] = secrets.token_hex(32)

# ==============================================================================
# SECURITY & ENCRYPTION ENGINE
# ==============================================================================
class SecurityManager:
    """Handles hashing, simple obfuscation, and credential protection."""
    @staticmethod
    def hash_password(password: str) -> str:
        salt = secrets.token_hex(16)
        pwd_hash = hashlib.pbkdf2_hmac('sha256', password.encode('utf-8'), salt.encode('utf-8'), 100000)
        return f"{salt}${pwd_hash.hex()}"

    @staticmethod
    def verify_password(stored_password: str, provided_password: str) -> bool:
        salt, pwd_hash = stored_password.split('$')
        reconstructed_hash = hashlib.pbkdf2_hmac(
            'sha256', 
            provided_password.encode('utf-8'), 
            salt.encode('utf-8'), 
            100000
        )
        return reconstructed_hash.hex() == pwd_hash

    @staticmethod
    def encode_token(data: str) -> str:
        return base64.b64encode(data.encode('utf-8')).decode('utf-8')

    @staticmethod
    def decode_token(token: str) -> str:
        return base64.b64decode(token.encode('utf-8')).decode('utf-8')

# ==============================================================================
# DATABASE ORM & INITIALIZATION
# ==============================================================================
class DatabaseController:
    """SQLite ORM for managing user profiles, services, and system logs."""
    def __init__(self, db_path: str):
        self.db_path = db_path
        self._init_db()

    def get_connection(self):
        conn = sqlite3.connect(self.db_path)
        conn.row_factory = sqlite3.Row
        return conn

    def _init_db(self):
        with self.get_connection() as conn:
            cursor = conn.cursor()
            # Profiles Table
            cursor.execute('''
                CREATE TABLE IF NOT EXISTS profiles (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    username TEXT UNIQUE NOT NULL,
                    full_name TEXT NOT NULL,
                    title TEXT NOT NULL,
                    bio TEXT,
                    business_email TEXT,
                    personal_email TEXT,
                    microsoft_account TEXT,
                    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
                )
            ''')
            # Accounts & Links
            cursor.execute('''
                CREATE TABLE IF NOT EXISTS accounts (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    platform_name TEXT NOT NULL,
                    handle TEXT NOT NULL,
                    url TEXT NOT NULL,
                    category TEXT NOT NULL,
                    is_verified INTEGER DEFAULT 1
                )
            ''')
            # System Execution Logs
            cursor.execute('''
                CREATE TABLE IF NOT EXISTS sys_logs (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    binary_name TEXT NOT NULL,
                    execution_time REAL NOT NULL,
                    status_code INTEGER NOT NULL,
                    output_summary TEXT,
                    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP
                )
            ''')
            conn.commit()
            self._seed_default_data()

    def _seed_default_data(self):
        with self.get_connection() as conn:
            cursor = conn.cursor()
            cursor.execute("SELECT COUNT(*) FROM profiles")
            if cursor.fetchone()[0] == 0:
                cursor.execute('''
                    INSERT INTO profiles (username, full_name, title, bio, business_email, personal_email, microsoft_account)
                    VALUES (?, ?, ?, ?, ?, ?, ?)
                ''', (
                    "ashraya",
                    "Ashraya Gyawali",
                    "IT Media Head | Developer & Web Designer",
                    "Engineering multi-language systems across Python, C++, C, and Modern Web Platforms.",
                    "business@ashrayagyawali.com",
                    "ashraya.dev@gmail.com",
                    "ashraya@outlook.com"
                ))
                
                accounts_data = [
                    ("YouTube", "VoltCutz Editz", "https://youtube.com", "Content"),
                    ("Instagram", "@voltcutz.editz", "https://instagram.com", "Social"),
                    ("Facebook", "Ashraya Gyawali", "https://facebook.com", "Social"),
                    ("GitHub", "AshrayaGyawali", "https://github.com", "Developer"),
                    ("Discord", "VoltCutz#0001", "https://discord.com", "Community")
                ]
                cursor.executemany('''
                    INSERT INTO accounts (platform_name, handle, url, category)
                    VALUES (?, ?, ?, ?)
                ''', accounts_data)
                conn.commit()

    def get_profile(self):
        with self.get_connection() as conn:
            cursor = conn.cursor()
            cursor.execute("SELECT * FROM profiles LIMIT 1")
            row = cursor.fetchone()
            return dict(row) if row else {}

    def get_accounts(self):
        with self.get_connection() as conn:
            cursor = conn.cursor()
            cursor.execute("SELECT * FROM accounts")
            return [dict(row) for row in cursor.fetchall()]

    def log_execution(self, binary_name, exec_time, status_code, output):
        with self.get_connection() as conn:
            cursor = conn.cursor()
            cursor.execute('''
                INSERT INTO sys_logs (binary_name, execution_time, status_code, output_summary)
                VALUES (?, ?, ?, ?)
            ''', (binary_name, exec_time, status_code, output[:500]))
            conn.commit()

    def get_logs(self):
        with self.get_connection() as conn:
            cursor = conn.cursor()
            cursor.execute("SELECT * FROM sys_logs ORDER BY timestamp DESC LIMIT 20")
            return [dict(row) for row in cursor.fetchall()]

db = DatabaseController(DB_PATH)

# ==============================================================================
# SUBPROCESS & NATIVE BINARY EXECUTOR
# ==============================================================================
class NativeExecutor:
    """Manages secure spawning and streaming of compiled C and C++ executables."""
    
    @staticmethod
    def run_binary(binary_filename: str):
        if sys.platform.startswith('win'):
            if not binary_filename.endswith('.exe'):
                binary_filename += '.exe'
                
        binary_path = os.path.join(BIN_DIR, binary_filename)
        
        if not os.path.exists(binary_path):
            return {
                "success": False,
                "error": f"Binary '{binary_filename}' not found at path: {binary_path}. Please compile native engines first.",
                "output": ""
            }

        start_time = time.time()
        try:
            process = subprocess.run(
                [binary_path],
                capture_output=True,
                text=True,
                timeout=15,
                check=True
            )
            elapsed = time.time() - start_time
            output = process.stdout
            
            db.log_execution(binary_filename, elapsed, process.returncode, output)
            
            return {
                "success": True,
                "execution_time_seconds": round(elapsed, 4),
                "return_code": process.returncode,
                "output": output
            }
        except subprocess.TimeoutExpired:
            return {"success": False, "error": "Execution timed out (15s limit).", "output": ""}
        except subprocess.CalledProcessError as e:
            return {"success": False, "error": f"Process crashed with exit code {e.returncode}.", "output": e.stderr}
        except Exception as e:
            return {"success": False, "error": str(e), "output": ""}

# ==============================================================================
# REST API ENDPOINTS
# ==============================================================================
@app.route('/')
def index():
    return send_from_directory(STATIC_DIR, 'index.html')

@app.route('/<path:filename>')
def static_files(filename):
    return send_from_directory(STATIC_DIR, filename)

@app.route('/api/profile', methods=['GET'])
def api_get_profile():
    profile = db.get_profile()
    accounts = db.get_accounts()
    return jsonify({
        "status": "online",
        "timestamp": datetime.now().isoformat(),
        "profile": profile,
        "accounts": accounts
    })

@app.route('/api/execute/c', methods=['POST', 'GET'])
def execute_c():
    result = NativeExecutor.run_binary("sys_check.exe")
    return jsonify(result)

@app.route('/api/execute/cpp', methods=['POST', 'GET'])
def execute_cpp():
    result = NativeExecutor.run_binary("engine.exe")
    return jsonify(result)

@app.route('/api/logs', methods=['GET'])
def api_get_logs():
    return jsonify({"logs": db.get_logs()})

@app.route('/api/system/metrics', methods=['GET'])
def system_metrics():
    """Generates mock low-level system diagnostic metrics for UI polling."""
    return jsonify({
        "cpu_usage_percent": round(15.4 + (time.time() % 5), 2),
        "ram_usage_mb": 4096 + int((time.time() * 10) % 512),
        "active_threads": threading.active_count(),
        "uptime_seconds": int(time.time() - app_start_time)
    })

# ==============================================================================
# SERVER INITIALIZATION
# ==============================================================================
if __name__ == '__main__':
    app_start_time = time.time()
    print("==========================================================")
    print("  ASHRAYA GYAWALI MASTER SYSTEM - PYTHON FLASK BACKEND    ")
    print("==========================================================")
    print(f" [*] Database Path : {DB_PATH}")
    print(f" [*] Binaries Dir  : {BIN_DIR}")
    print(f" [*] Server Status : http://127.0.0.1:5000")
    print("==========================================================")
    app.run(host='127.0.0.1', port=5000, debug=True)