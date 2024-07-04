# -*- coding: utf-8 -*-
# @Date    : 2023-12-16 09:10:26
# @Author  : Starreeze

from __future__ import annotations
import sys, os, logging, urllib.parse
from http.server import BaseHTTPRequestHandler, HTTPServer

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.dirname(SCRIPT_DIR))
import args
from nlu.weather import answer
from nlu.llama import init_llama


class RequestHandler(BaseHTTPRequestHandler):
    def do_POST(self):
        content_length = int(self.headers["Content-Length"])
        post_data = self.rfile.read(content_length)
        data = urllib.parse.parse_qs(post_data.decode())
        response = ""
        if "query" in data:
            response = answer(data["query"][0])
        self.respond_with_message(response)
        return

    def respond_with_message(self, message: str):
        self.send_response(200)
        self.send_header("Content-type", "text/html")
        self.end_headers()
        self.wfile.write(message.encode())


def run():
    logging.info("loading model...")
    init_llama()
    logging.info("Starting server...")
    server_address = (args.server_ip, args.server_port)
    httpd = HTTPServer(server_address, RequestHandler)
    logging.info("Running server...")
    httpd.serve_forever()


if __name__ == "__main__":
    run()
