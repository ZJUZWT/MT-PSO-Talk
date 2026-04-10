#!/usr/bin/env python3

from __future__ import annotations

import argparse
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from threading import Event, Thread
from urllib.parse import parse_qs, urlparse


def first_value(values: dict[str, list[str]], key: str, default: str = "") -> str:
    items = values.get(key)
    return items[0] if items else default


def build_handler(
    *,
    outdir: Path,
    prefix: str,
    result_file: Path,
    done: Event,
):
    class CaptureHandler(BaseHTTPRequestHandler):
        def log_message(self, format: str, *args) -> None:  # noqa: A003
            return

        def end_headers(self) -> None:
            self.send_header("Access-Control-Allow-Origin", "*")
            self.send_header("Access-Control-Allow-Methods", "POST, OPTIONS")
            self.send_header(
                "Access-Control-Allow-Headers",
                "Content-Type, X-Slide-Capture-Target",
            )
            super().end_headers()

        def do_OPTIONS(self) -> None:  # noqa: N802
            self.send_response(204)
            self.end_headers()

        def do_POST(self) -> None:  # noqa: N802
            content_length = int(self.headers.get("Content-Length", "0"))
            body = self.rfile.read(content_length)
            if not body:
                self.send_response(400)
                self.end_headers()
                self.wfile.write(b"missing request body")
                return

            parsed = urlparse(self.path)
            params = parse_qs(parsed.query)
            image_path = outdir / f"{prefix}.png"
            meta_path = outdir / f"{prefix}.txt"

            image_path.write_bytes(body)
            meta_path.write_text(
                "\n".join(
                    [
                        "mode=browser-api",
                        f"scope={first_value(params, 'scope', 'page')}",
                        f"source_url={first_value(params, 'sourceUrl')}",
                        "browser_chrome_included=no",
                        f"target_id={self.headers.get('X-Slide-Capture-Target', '')}",
                        f"content_type={self.headers.get('Content-Type', 'image/png')}",
                        f"image_path={image_path}",
                    ],
                )
                + "\n",
                encoding="utf-8",
            )
            result_file.write_text(str(image_path), encoding="utf-8")

            self.send_response(200)
            self.send_header("Content-Type", "text/plain; charset=utf-8")
            self.end_headers()
            self.wfile.write(b"ok")
            done.set()

    return CaptureHandler


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Receive one browser-native SlideApp PNG capture and write it to disk.",
    )
    parser.add_argument("--outdir", required=True)
    parser.add_argument("--prefix", required=True)
    parser.add_argument("--port-file", required=True)
    parser.add_argument("--result-file", required=True)
    parser.add_argument("--timeout-seconds", type=float, default=20)
    args = parser.parse_args()

    outdir = Path(args.outdir).expanduser().resolve()
    outdir.mkdir(parents=True, exist_ok=True)
    port_file = Path(args.port_file).expanduser().resolve()
    result_file = Path(args.result_file).expanduser().resolve()
    done = Event()

    server = ThreadingHTTPServer(
        ("127.0.0.1", 0),
        build_handler(
            outdir=outdir,
            prefix=args.prefix,
            result_file=result_file,
            done=done,
        ),
    )
    server.daemon_threads = True
    port_file.write_text(str(server.server_port), encoding="utf-8")

    thread = Thread(target=server.serve_forever, kwargs={"poll_interval": 0.1})
    thread.daemon = True
    thread.start()

    try:
        if not done.wait(timeout=args.timeout_seconds):
            return 1
        return 0
    finally:
        server.shutdown()
        server.server_close()
        thread.join(timeout=1)


if __name__ == "__main__":
    raise SystemExit(main())
