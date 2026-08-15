import http.client
import json
import socket
import subprocess
import sys
import time


def require(condition, message):
    if not condition:
        raise AssertionError(message)


def find_port():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        sock.bind(("127.0.0.1", 0))
        return sock.getsockname()[1]


def request(port, method, path, body=None):
    connection = http.client.HTTPConnection("127.0.0.1", port, timeout=5)
    headers = {}
    payload = None

    if body is not None:
        payload = body.encode("utf-8")
        headers["Content-Type"] = "application/json"

    connection.request(method, path, payload, headers)
    response = connection.getresponse()
    text = response.read().decode("utf-8")
    connection.close()
    return response.status, text


def request_json(port, method, path, data):
    return request(port, method, path, json.dumps(data))


def wait_for_server(port):
    deadline = time.time() + 10

    while time.time() < deadline:
        try:
            status, _ = request(port, "GET", "/health")

            if status == 200:
                return
        except OSError:
            time.sleep(0.1)

    raise RuntimeError("Server did not start.")


def main():
    executable = sys.argv[1]
    port = find_port()
    server = subprocess.Popen(
        [executable, str(port)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )

    try:
        wait_for_server(port)

        status, _ = request(port, "GET", "/health")
        require(status == 200, "GET /health should return 200")

        valid_code = "int main() { int values[2]; return values[4]; }"
        status, body = request_json(
            port,
            "POST",
            "/api/review",
            {"language": "cpp", "code": valid_code},
        )
        require(status == 200, "Valid C++ review should return 200")
        require("totalIssues" in json.loads(body), "Response should include totalIssues")

        status, _ = request_json(
            port,
            "POST",
            "/api/review",
            {"language": "cpp", "code": ""},
        )
        require(status == 400, "Empty code should return 400")

        status, _ = request_json(
            port,
            "POST",
            "/api/review",
            {"language": "cpp"},
        )
        require(status == 400, "Missing code should return 400")

        status, _ = request_json(
            port,
            "POST",
            "/api/review",
            {"language": "python", "code": "print('hello')"},
        )
        require(status == 400, "Unsupported language should return 400")

        status, _ = request(port, "POST", "/api/review", "{")
        require(status == 400, "Malformed JSON should return 400")
    finally:
        server.terminate()

        try:
            server.wait(timeout=5)
        except subprocess.TimeoutExpired:
            server.kill()
            server.wait(timeout=5)


if __name__ == "__main__":
    main()
